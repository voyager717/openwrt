/*
 * service_prioritization.c, it's used to support MAP R3 Service Prioritization
 *
 * Author: Zheng Zhou <Zheng.Zhou@mediatek.com>
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_bridge.h>
#include <linux/netfilter_ipv4.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>
#include <net/netlink.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/kobject.h>
#include <linux/version.h>
#include <linux/jhash.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#include "traffic_separation.h"
#include "service_prioritization.h"
#include "mtkmapfilter.h"
#include "debug.h"

#define SP_PERIOD	(5 * HZ)
#define SP_AGE_OUT		(20 * HZ)

#define PRINT_IP_RAW(ip) ((ip) & 0x000000ff), (((ip) & 0x0000ff00) >> 8), (((ip) & 0x00ff0000) >> 16), (((ip) & 0xff000000) >> 24)
#define PRINT_MAC_RAW(a) a[0],a[1],a[2],a[3],a[4],a[5]
#define MACSTR_RAW "%02x:%02x:%02x:%02x:%02x:%02x"
#define IPSTR_RAW "%d.%d.%d.%d"
#define RULE_ID_RAW "%02x%02x%02x%02x"
#define PRINT_RULE_ID_RAW(id) (id)[0], (id)[1], (id)[2], (id)[3]

static inline unsigned int ip_hdrlen(const struct sk_buff *skb)
{
	return ip_hdr(skb)->ihl * 4;
}
int sp_notify(unsigned char up, unsigned char mask, unsigned char version, unsigned char *saddr,
	unsigned char *daddr, unsigned short src_port, unsigned short dst_port);
int sp_notify_wrap(struct sk_buff *skb, unsigned char up, unsigned char mask);

struct sp_context sp_ctx;
int dump_sp_debug_info(struct sp_context *ctx, char *buf);

static ssize_t map_sp_debug_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int len = dump_sp_debug_info(&sp_ctx, buf);

	if (len < 0)
		len = 0;

	return len;
}

static ssize_t map_sp_config_get(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "sp_enable=%u, sp_dynamic_enable=%u,sp_dynamic_prio=%u, sp_quick_match=%u\n",
		(unsigned int)sp_ctx.conf.enable,
		(unsigned int)sp_ctx.conf.dynamic_rule_enable,
		(unsigned int)sp_ctx.conf.dynamic_prio, (unsigned int)sp_ctx.conf.quick_match);
}

ssize_t map_sp_config_set(struct kobject *kobj, struct kobj_attribute *attr,
	const char *buf, size_t count)
{
	sscanf(buf, "%hhu %hhu %hhu %hhu\n", &sp_ctx.conf.enable,
		&sp_ctx.conf.dynamic_rule_enable,
		&sp_ctx.conf.dynamic_prio, &sp_ctx.conf.quick_match);
	return count;
}

static struct kobj_attribute map_sysfs_sp_debug =
		__ATTR(sp_debug_show, S_IRUGO, map_sp_debug_show, NULL);
static struct kobj_attribute map_sysfs_sp_config =
		__ATTR(sp_config, S_IRUGO | S_IWUSR, map_sp_config_get, map_sp_config_set);

static struct attribute *sp_sysfs[] = {
	&map_sysfs_sp_debug.attr,
	&map_sysfs_sp_config.attr,
	NULL,
};
static struct attribute_group sp_attr_group = {
	.attrs = sp_sysfs,
};

/*
 *sp free rule function
 *internal using in service_prioritization.c
 */
static void free_sp_rule(struct rcu_head *head)
{
	struct sp_rule_element *r = container_of(head, struct sp_rule_element, rcu);
	kfree(r);
}

/*
 *[internal rule list operation]
 *insert the rule element to rule list
 */
int sp_rule_insert(struct hlist_head *list, struct sp_rule_element *rule)
{
	int found = 0, same = 0, ret = 0;
	struct sp_rule_element *r = NULL;
	struct hlist_node *n = NULL;

	if (rule->rule.rule_type == RULE_TYPE_STATIC) {
		hlist_for_each_entry_safe(r, n, list, entry) {
			if (!memcmp(r->rule.rule_identifier, rule->rule.rule_identifier, 4)) {
				same = 1;
				break;
			}

			if (r->rule.index > rule->rule.index) {
				found = 1;
				break;
			}
		}
	}

	if (same) {
		hlist_replace_rcu(&r->entry, &rule->entry);
		call_rcu(&r->rcu, free_sp_rule);
	} else {
		if (found) {
			hlist_add_before_rcu(&rule->entry, &r->entry);
		} else {
			hlist_add_tail_rcu(&rule->entry, list);
		}
	}

	return ret;
}

/*
 *[internal rule list operation]
 *clear all rule elements from rule list
 */
int sp_rule_clear(struct hlist_head *list)
{
	int ret = 0;
	struct sp_rule_element *r = NULL;
	struct hlist_node *n = NULL;

	hlist_for_each_entry_safe(r, n, list, entry) {
		hlist_del_rcu(&r->entry);
		call_rcu(&r->rcu, free_sp_rule);
	}

	return ret;
}

/*
 *[internal rule list operation]
 *remove one sp rule from rule list
 */
int sp_rule_del_one(struct hlist_head *list, struct sp_rule *rule)
{
	int ret = 0;
	struct sp_rule_element *r = NULL;
	struct hlist_node *n = NULL;

	hlist_for_each_entry_safe(r, n, list, entry) {
		if (!memcmp(r->rule.rule_identifier, rule->rule_identifier, 4)) {
			hlist_del_rcu(&r->entry);
			call_rcu(&r->rcu, free_sp_rule);
			break;
		}
	}

	return ret;
}

/*
 *sp create rule element function
 *internal using in service_prioritization.c
 */
struct sp_rule_element *create_rule_element(struct sp_rule *rule)
{
	struct sp_rule_element *r = NULL;

	r = (struct sp_rule_element*)kmalloc(sizeof(struct sp_rule_element), GFP_KERNEL);

	if (r) {
		memset(r, 0, sizeof(struct sp_rule_element));
		r->rule = *rule;
	}

	return r;
}

/*
 *sp add rules function
 *exported to other file for using
 */
int sp_add_rule(struct sp_rules_config *rule_config)
{
	struct sp_context *ctx = &sp_ctx;
	int ret = 0, i = 0;
	struct sp_rule_list *tlist = NULL;
	struct sp_rule_element *r = NULL;

	for (i = 0; i < rule_config->number; i++) {
		tlist = rule_config->rules[i].rule_type == RULE_TYPE_STATIC ?
			&ctx->static_list : &ctx->dynamic_list;

		r = create_rule_element(&rule_config->rules[i]);
		if (r) {
			spin_lock_bh(&tlist->lock);
			ret = sp_rule_insert(&tlist->head, r);
			spin_unlock_bh(&tlist->lock);
		}
	}

	return ret;
}

/*
 *sp clear rules function
 *exported to other file for using
 */
int sp_clear_rule(void)
{
	struct sp_context *ctx = &sp_ctx;
	int ret = 0;

	spin_lock_bh(&ctx->static_list.lock);
	sp_rule_clear(&ctx->static_list.head);
	spin_unlock_bh(&ctx->static_list.lock);

	spin_lock_bh(&ctx->dynamic_list.lock);
	sp_rule_clear(&ctx->dynamic_list.head);
	spin_unlock_bh(&ctx->dynamic_list.lock);

	return ret;
}

/*
 *sp remove one rule function
 *exported to other file for using
 */
int sp_rm_one_rule(struct sp_rule *rule)
{
	struct sp_context *ctx = &sp_ctx;
	int ret = 0;
	struct sp_rule_list *tlist = NULL;

	tlist = rule->rule_type == RULE_TYPE_STATIC ?
			&ctx->static_list : &ctx->dynamic_list;

	spin_lock_bh(&tlist->lock);
	ret = sp_rule_del_one(&tlist->head, rule);
	spin_unlock_bh(&tlist->lock);

	return ret;
}

/*
 *sp set dscp mapping table function
 *exported to other file for using
 */
int sp_set_dscp_mapping_table(unsigned char *tbl)
{
	struct sp_context *ctx = &sp_ctx;
	int ret = 0;

	memcpy(ctx->dptbl.dptbl, tbl, 64);

	return ret;
}

/*
 *sp set sp config function
 *exported to other file for using
 */
int sp_set_conf(struct sp_context *ctx, unsigned char sp_enable,
	unsigned char dynamic_enable, unsigned char dynamic_prio, unsigned char quick_match)
{
	ctx->conf.enable = sp_enable;
	ctx->conf.dynamic_rule_enable = dynamic_enable;
	ctx->conf.dynamic_prio = dynamic_prio;
	ctx->conf.quick_match = quick_match;

	return 0;
}

#define VLAN_VID_MASK		0x0fff /* VLAN Identifier */
#define vlan_tag_get_id(__skb)	((__skb)->vlan_tci & VLAN_VID_MASK)

/*
 *sp modify dscp field in IP header
 *internal using in service_prioritization.c
 */
int modify_dscp_and_pcp_field(struct sk_buff *skb, unsigned char pcp)
{
	int ret = 0;
	struct ipv6hdr *iph_v6 = NULL; /*struct ipv6hdr *hdr = ipv6_hdr(skb);*/
	struct iphdr *iph_v4 = NULL;	/*iph = ip_hdr(skb);*/
	unsigned short vid = 0;

	if (skb->protocol == htons(ETH_P_IP)) {
		iph_v4 = ip_hdr(skb);
		iph_v4->check = 0;
		iph_v4->tos &= 0x1F;
		iph_v4->tos |= (pcp << 5);
		iph_v4->check = ip_fast_csum(skb_network_header(skb), iph_v4->ihl);
	} else if (skb->protocol == htons(ETH_P_IPV6)) {
		iph_v6 = ipv6_hdr(skb);
		iph_v6->priority |= (pcp << 1);
	}

	/*Modify pcp value in vlan header*/
	if (skb->vlan_proto == htons(ETH_P_8021Q) && skb->vlan_tci) {
		vid = vlan_tag_get_id(skb);
		vid = ((pcp & 0x07) << 13) | vid;
		__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vid);
	}

	return ret;
}

/*
 *sp free session function
 *internal using in service_prioritization.c
 */
static void free_session(struct rcu_head *head)
{
	struct tcpip_session_element *session = container_of(head, struct tcpip_session_element, rcu);
	kfree(session);
}

static void free_learn_complete_element(struct rcu_head *head);
void send_broadcast_nlmsg(unsigned char *msg, int msg_len);
#define LEARN_COUNT_THRESHOLD 20
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,14,0))
void sp_period(struct timer_list *timer)
#else
void sp_period(unsigned long _data)
#endif
{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,14,0))
	struct sp_context *ctx = (struct sp_context *)container_of(timer, struct sp_context, sp_period);
#else
	struct sp_context *ctx = (struct sp_context *)_data;
#endif
	int i = 0;
	struct tcpip_session_element *session = NULL;
	struct learn_comp_element *learn_comp = NULL;
	/*printk("%s %d\n", __func__, __LINE__);*/

	/*remove the expired session from hash table*/
	for (i = 0; i < SP_TCPIP_HASH_SIZE; i++) {
		spin_lock_bh(&ctx->hash[i].hash_lock);
		hlist_for_each_entry_rcu(session, &ctx->hash[i].head, entry) {
			if (time_before_eq(session->age + SP_AGE_OUT, jiffies)) {
					hlist_del_rcu(&session->entry);
					call_rcu(&session->rcu, free_session);
					break;	/*only remove one session once*/
			}
		}
		spin_unlock_bh(&ctx->hash[i].hash_lock);
	}

	for (i = 0; i < SP_LEARN_HASH_SIZE; i++) {
		spin_lock_bh(&ctx->learn_hash[i].hash_lock);
		hlist_for_each_entry_rcu(learn_comp, &ctx->learn_hash[i].head, entry) {
			if (time_before_eq(learn_comp->age + SP_AGE_OUT, jiffies)) {
				/*remove the expired learn complete event from hash table*/
				hlist_del_rcu(&learn_comp->entry);
				call_rcu(&learn_comp->rcu, free_learn_complete_element);
				break;	/*only remove one session once*/
			} else if (!learn_comp->report && learn_comp->learn_count > LEARN_COUNT_THRESHOLD) {
				/*report learn_complete event to 1905daemon*/
				send_broadcast_nlmsg((unsigned char *)&learn_comp->learn_event, sizeof(struct sp_learn_complete_event));
				learn_comp->report = 1;
			}
		}
		spin_unlock_bh(&ctx->learn_hash[i].hash_lock);
	}

	/*printk("%s %d\n", __func__, __LINE__);*/
	mod_timer(&ctx->sp_period, jiffies + SP_PERIOD);
}


/*
 *sp tcphash computing
 *internal using in service_prioritization.c
 */
unsigned int sp_tcpip_hash(const u32 saddr, const u16 sport,
				  const u32 daddr, const u16 dport)
{
	return jhash_3words(saddr, daddr, (u32)sport << 16 | (u32)dport, 16);
}

unsigned int sp_tcp_ip_hash_idx(const u8 *saddr,
	const u16 sport, const u8 *daddr, const u16 dport, unsigned char ip_ver)
{
	unsigned int h_idx = 0, src_addr = 0, dst_addr = 0;

	if (ip_ver == IP_V4) {
		src_addr = *((unsigned int *)saddr);
		dst_addr = *((unsigned int *)daddr);
	} else if (ip_ver == IP_V6) {
		src_addr = *((unsigned int *)saddr) ^ *((unsigned int *)(saddr + 4))
			^ *((unsigned int *)(saddr + 8)) ^ *((unsigned int *)(saddr + 12));
		dst_addr = *((unsigned int *)daddr) ^ *((unsigned int *)(daddr + 4))
			^ *((unsigned int *)(daddr + 8)) ^ *((unsigned int *)(daddr + 12));
	}

	h_idx = sp_tcpip_hash(src_addr, sport, dst_addr, dport) & (SP_TCPIP_HASH_SIZE - 1) ;

	return h_idx;
}

/*
 *sp create tcpip session
 *internal using in service_prioritization.c
 */
struct tcpip_session_element *sp_create_session(const u8 *saddr, const u16 sport,
				  const u8 *daddr, const u16 dport, unsigned char ip_ver)
{
	struct tcpip_session_element *session = NULL;

	session = kmalloc(sizeof(struct tcpip_session_element), GFP_ATOMIC);

	if (session) {
		memset(session, 0, sizeof(struct tcpip_session_element));
		session->tuple.src_port = sport;
		session->tuple.dst_port = dport;
		if (ip_ver == IP_V4) {
			session->tuple.src_addr.v4_addr = *((unsigned int *)saddr);
			session->tuple.dst_addr.v4_addr = *((unsigned int *)daddr);
		} else if (ip_ver == IP_V6) {
			memcpy(session->tuple.src_addr.v6_addr, saddr, 16);
			memcpy(session->tuple.dst_addr.v6_addr, daddr, 16);
		}
		session->pcp = 0;
		session->age = jiffies;
	}

	return session;
}

/*
 *sp get tcpip session
 *internal using in service_prioritization.c
 */
struct tcpip_session_element *sp_get_session(struct tcpip_hash_head *hash, const u8 *saddr,
	const u16 sport, const u8 *daddr, const u16 dport, unsigned char ip_ver)
{
	struct tcpip_session_element *session = NULL;
	unsigned int h_idx = 0;
#if 0
	int xx = 0;
#endif

	h_idx = sp_tcp_ip_hash_idx(saddr, sport, daddr, dport, ip_ver);

	rcu_read_lock();
	hlist_for_each_entry_rcu(session, &hash[h_idx].head, entry) {
		/*
		printk("%s %d "IPSTR_RAW " " IPSTR_RAW " %d %d""\n", __func__, __LINE__,
			PRINT_IP_RAW(session->tuple.src_addr.v4_addr),
			PRINT_IP_RAW(session->tuple.dst_addr.v4_addr),
			session->tuple.src_port, session->tuple.dst_port);
		*/
#if 0
		if (xx++ >= 100000) {
			printk("!!!!!!!!!!!!!!!!!!!%s %d\n", __func__, __LINE__);
		}
#endif
		if (session->tuple.src_port == sport && session->tuple.dst_port == dport) {
			if (ip_ver == IP_V4) {
				if (session->tuple.src_addr.v4_addr == *((unsigned int*)saddr) &&
					session->tuple.dst_addr.v4_addr == *((unsigned int*)daddr))
					break;
			} else if (ip_ver == IP_V6) {
				if (!memcmp(session->tuple.src_addr.v6_addr, saddr, 16) &&
					!memcmp(session->tuple.dst_addr.v6_addr, daddr, 16))
					break;
			}
		}
	}
	rcu_read_unlock();

	return session;
}

int get_tuple_from_skb(struct sk_buff *skb, unsigned char data_dir,
	struct ip_5_tuple *tuple, unsigned char *dscp)
{
	int ret = 0;

	struct ipv6hdr *iph_v6 = NULL; /*struct ipv6hdr *hdr = ipv6_hdr(skb);*/
	struct iphdr *iph_v4 = NULL;	/*iph = ip_hdr(skb);*/
	struct tcphdr *tcph = NULL;
	struct udphdr *udph = NULL;
	unsigned int ip_len = 0;

	memset(tuple, 0, sizeof(struct ip_5_tuple));

	if (skb->protocol == htons(ETH_P_IP)) {
		iph_v4 = ip_hdr(skb);
		tuple->src_addr.v4_addr = data_dir == DIR_UPSTREAM ? iph_v4->saddr : iph_v4->daddr;
		tuple->dst_addr.v4_addr = data_dir == DIR_UPSTREAM ? iph_v4->daddr : iph_v4->saddr;
		tuple->ip_ver = IP_V4;
		tuple->protocol = iph_v4->protocol;
		*dscp = ((iph_v4->tos & 0xfc) >> 2);
		/*IPv4 header length is flexible*/
		ip_len = ip_hdrlen(skb);
		skb_set_transport_header(skb, ip_len);
	} else if (skb->protocol == htons(ETH_P_IPV6)) {
		iph_v6 = ipv6_hdr(skb);
		memcpy(tuple->src_addr.v6_addr,	data_dir == DIR_UPSTREAM ?
			iph_v6->saddr.in6_u.u6_addr8 : iph_v6->daddr.in6_u.u6_addr8, 16);
		memcpy(tuple->dst_addr.v6_addr,	data_dir == DIR_UPSTREAM ?
			iph_v6->daddr.in6_u.u6_addr8 : iph_v6->saddr.in6_u.u6_addr8, 16);
		tuple->ip_ver = IP_V6;
		tuple->protocol = iph_v6->nexthdr;
		*dscp = (iph_v6->priority << 2) | ((iph_v6->flow_lbl[0] & 0xc0) >> 6);
		/*IPv6 header length always equals to 40 bytes*/
		ip_len = sizeof(struct ipv6hdr);
		skb_set_transport_header(skb, ip_len);
	}

	if (tuple->protocol == IPPROTO_TCP) {
		tcph = tcp_hdr(skb);
		tuple->src_port = ntohs(data_dir == DIR_UPSTREAM ? tcph->source : tcph->dest);
		tuple->dst_port = ntohs(data_dir == DIR_UPSTREAM ? tcph->dest : tcph->source);
	} else if (tuple->protocol == IPPROTO_UDP) {
		udph = udp_hdr(skb);
		tuple->src_port = ntohs(data_dir == DIR_UPSTREAM ? udph->source: udph->dest);
		tuple->dst_port = ntohs(data_dir == DIR_UPSTREAM ? udph->dest : udph->source);
	} else {
		/*only notify TCP/UDP session*/
		ret = -1;
	}

	return ret;
}

/*
 *sp rule quick match function
 *internal using in service_prioritization.c
 */
int rule_quick_match(struct sp_context *ctx, struct sk_buff *skb, unsigned char *pcp,
	unsigned char data_dir, struct ip_5_tuple *tuple)
{
	struct tcpip_session_element *session = NULL;
	int ret = 0;

	rcu_read_lock();
	if (tuple->protocol == IPPROTO_TCP || tuple->protocol == IPPROTO_UDP) {
		session = sp_get_session(&ctx->hash[0], (unsigned char *)&tuple->src_addr, tuple->src_port,
			(unsigned char *)&tuple->dst_addr, tuple->dst_port, tuple->ip_ver);

		if (session) {
			*pcp = session->pcp;
			session->age = jiffies;
			session->match_count++;
			ret = 1;
		}
	}
	rcu_read_unlock();

	return ret;
}

/*
 *sp rule slow match function
 *internal using in service_prioritization.c
 */
int rule_slow_match(struct sp_context *ctx, struct sp_rule *r, struct sk_buff *skb, unsigned char *pcp,
	unsigned char data_dir, struct ip_5_tuple *tuple, unsigned char dscp)
{
	int ret = 0;
	struct ethhdr *hdr = NULL;
	struct client_db *client = NULL;

	hdr = eth_hdr(skb);

	/*slow matching*/
	/*1. compare al mac*/
	if (r->mask & RULE_AL_MAC) {
		if (r->rule_output == 0x08) {
			/*dscp table looking up*/
			if (dscp >= 64)
				goto fail;

			*pcp = ctx->dptbl.dptbl[dscp];
			goto suc;
		} else if (r->rule_output <= 7) {
			if (memcmp(ctx->own_al_mac, r->al_mac, ETH_ALEN))
				goto fail;
		}
	}

	/*2. compare ssid*/
	if (r->mask & RULE_SSID) {
		client = client ? client : get_client_db(data_dir == DIR_UPSTREAM ? hdr->h_source : hdr->h_dest);
		if (!client)
			goto fail;

		if (r->ssid_len != client->ssid_len ||
			strncmp(r->ssid, client->ssid, r->ssid_len))
			goto fail;
	}

	/*3. compare client mac*/
	if (r->mask & RULE_MAC) {
		client = client ? client : get_client_db(data_dir == DIR_UPSTREAM ? hdr->h_source : hdr->h_dest);
		if (!client)
			goto fail;

		if (memcmp(r->client_mac, client->mac, ETH_ALEN))
			goto fail;
	}

	/*4. compare protocol in IP header*/
	if (r->mask & RULE_TCPIP_PRO) {
		if (tuple->protocol != r->tcpip.protocol)
			goto fail;
	}

	/*5. compare TCPIP 4 tuples*/
	if (r->mask & RULE_TCPIP_DST_IP || r->mask & RULE_TCPIP_DST_PORT ||
		r->mask & RULE_TCPIP_SRC_IP || r->mask & RULE_TCPIP_SRC_PORT) {
		if (tuple->ip_ver == IP_V4) {
			/*ip v4*/
			if (r->mask & RULE_TCPIP_DST_IP) {
				if (tuple->dst_addr.v4_addr != r->tcpip.dst_addr.v4_addr)
					goto fail;
			}

			if (r->mask & RULE_TCPIP_SRC_IP) {
				if (tuple->src_addr.v4_addr != r->tcpip.src_addr.v4_addr)
					goto fail;
			}
		} else {
			/*ip v6*/
			if (r->mask & RULE_TCPIP_DST_IP) {
				if (memcmp(tuple->dst_addr.v6_addr, r->tcpip.dst_addr.v6_addr, 16))
					goto fail;
			}

			if (r->mask & RULE_TCPIP_SRC_IP) {
				if (memcmp(tuple->src_addr.v6_addr, r->tcpip.src_addr.v6_addr, 16))
					goto fail;
			}
		}

		if (r->mask & RULE_TCPIP_DST_PORT) {
			if (tuple->dst_port != r->tcpip.dst_port)
				goto fail;
		}

		if (r->mask & RULE_TCPIP_SRC_PORT) {
			if (tuple->src_port != r->tcpip.src_port)
				goto fail;
		}
	}

	*pcp = r->rule_output;

suc:
	/*successfully matching*/
	ret = 1;
fail:
	return ret;
}

/*
 *sp matching function
 *internal using in service_prioritization.c
 */
int sp_matching(struct sp_context *ctx, struct hlist_head *list,
	struct sk_buff *skb, unsigned char dir)
{
	int match = 0;
	struct sp_rule_element *r = NULL;
	struct tcpip_session_element *new_session = NULL;
	unsigned int new_hidx = 0;
	unsigned char pcp = 0, dscp_value = 0;
	struct ip_5_tuple tuple;
#if 0
	int xx = 0;
#endif

	/*1. get tcpip tuple*/
	get_tuple_from_skb(skb, dir, &tuple, &dscp_value);

	/*2. do quick match firstly*/
	if (ctx->conf.quick_match) {
		if (rule_quick_match(ctx, skb, &pcp, dir, &tuple)) {
			modify_dscp_and_pcp_field(skb, pcp);
			goto end;
		}
	}

	/*3. then do slow match if neccessary*/
	rcu_read_lock();
	hlist_for_each_entry_rcu(r, list, entry) {
#if 0
		if (xx++ >= 100000) {
			printk("!!!!!!!!!!!!!!!!!!!%s %d\n", __func__, __LINE__);
		}
#endif
		if (rule_slow_match(ctx, &r->rule, skb, &pcp, dir, &tuple, dscp_value)) {
			/*pcp------>dscp*/
			r->match_count++;
			modify_dscp_and_pcp_field(skb, pcp);
			match = 1;
			break;
		}
	}
	rcu_read_unlock();

	if (ctx->conf.quick_match) {
		/*add new session to existing hash table when successfully slow matching*/
		if (match/* && dir == DIR_UPSTREAM*/) {
			if (tuple.protocol == IPPROTO_TCP || tuple.protocol == IPPROTO_UDP) {
				new_session = sp_create_session((unsigned char *)&tuple.src_addr, tuple.src_port,
					(unsigned char *)&tuple.dst_addr, tuple.dst_port, tuple.ip_ver);

				if (new_session) {
					new_session->pcp = pcp;
					new_session->correspond_rule_idx = r->rule.index;

					new_hidx = sp_tcp_ip_hash_idx((unsigned char *)&tuple.src_addr, tuple.src_port,
						(unsigned char *)&tuple.dst_addr, tuple.dst_port, tuple.ip_ver);

					spin_lock_bh(&ctx->hash[new_hidx].hash_lock);
					hlist_add_head_rcu(&new_session->entry, &ctx->hash[new_hidx].head);
					spin_unlock_bh(&ctx->hash[new_hidx].hash_lock);
				}
			}
		}
	}
end:
	return match;
}

/*
 *sp main function
 *exported to other file for using
 */
int sp_function(struct sp_context *ctx, struct sk_buff *skb, unsigned char dir)
{
	int ret = 0;
	unsigned char up = 0, peer_type = 0;
	struct hlist_head *list_1st = NULL, *list_2nd = NULL;

	if (skb->protocol != htons(ETH_P_IP) &&
		skb->protocol != htons(ETH_P_IPV6))
		goto end;

	up = skb->cb[SKB_CB_OFFSET] & 0x0F;
	peer_type = (skb->cb[SKB_CB_OFFSET] & 0xF0) >> 4;
	/*1. check peer type*/
	if (peer_type == MAP_R3)
		goto end;

	/*2. check if up equals to 0*/
	if (up) {
		/*call sp notifier*/
		modify_dscp_and_pcp_field(skb, up);
		if (ctx->conf.dynamic_rule_enable) {
			if (dir == DIR_UPSTREAM)
				sp_notify_wrap(skb, up, 0x0F);
		}
		ret = 1;
	} else {
		if (ctx->conf.dynamic_rule_enable) {
			if (!ctx->conf.dynamic_prio) {
				list_1st = &ctx->static_list.head;
				list_2nd = &ctx->dynamic_list.head;
			} else {
				list_1st = &ctx->dynamic_list.head;
				list_2nd = &ctx->static_list.head;
			}
		} else {
			list_1st = &ctx->static_list.head;
			list_2nd = NULL;
		}

		if (list_1st && sp_matching(ctx, list_1st, skb, dir))
				ret = 1;
		else if (list_2nd && sp_matching(ctx, list_2nd, skb, dir))
				ret = 1;

	}
end:
	return ret;
}

/*
 *sp debug function
 *exported to other file for using
 */
int dump_sp_debug_info(struct sp_context *ctx, char *buf)
{
	int len = 0, total_len = 0, i = 0;
	struct sp_rule_element *r = NULL;
	struct tcpip_session_element *session = NULL;
	struct learn_comp_element *learn_comp = NULL;

	/*0.basic information*/
	len = snprintf(buf, PAGE_SIZE, "[dump sp debug information]\n");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;

	len = snprintf(buf + total_len, PAGE_SIZE - total_len,
		"[Dynamic Rule]:%s\n", ctx->conf.dynamic_rule_enable ? "ENABLE" : "DISABLE");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;
	len = snprintf(buf + total_len, PAGE_SIZE - total_len,
		"[Dynamic Rule Priority]:%s\n", ctx->conf.dynamic_prio ? "ENABLE" : "DISABLE");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	/*1.dump static rule list*/
	total_len += len;
	len = snprintf(buf + total_len, PAGE_SIZE - total_len,
		"[Static Rule list]:\n");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;
	len = snprintf(buf + total_len, PAGE_SIZE - total_len,
		"index hit_cnt name\trule_ID mask al_mac\tssid\tclient_mac\tprotocol"
		" dst_IP\tsrc_IP\tdst_port src_port output\n");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;

	rcu_read_lock();
	spin_lock_bh(&ctx->static_list.lock);
	hlist_for_each_entry_rcu(r, &ctx->static_list.head, entry) {
		len = snprintf(buf + total_len, PAGE_SIZE - total_len, "%d\t%ld\t%s\t"RULE_ID_RAW" %04x "MACSTR_RAW"\t%s\t"
			MACSTR_RAW"\t%d %d.%d.%d.%d %d.%d.%d.%d %d %d %d\n",
			r->rule.index, r->match_count, r->rule.rule_name, PRINT_RULE_ID_RAW(r->rule.rule_identifier),r->rule.mask,
			PRINT_MAC_RAW(r->rule.al_mac), r->rule.ssid, PRINT_MAC_RAW(r->rule.client_mac),
			r->rule.tcpip.protocol, PRINT_IP_RAW(r->rule.tcpip.dst_addr.v4_addr),
			PRINT_IP_RAW(r->rule.tcpip.src_addr.v4_addr), r->rule.tcpip.dst_port,
			r->rule.tcpip.src_port, r->rule.rule_output);

		if (snprintf_error(PAGE_SIZE - total_len, len)) {
			spin_unlock_bh(&ctx->static_list.lock);
			rcu_read_unlock();
			goto end;
		}

		total_len += len;
	}
	spin_unlock_bh(&ctx->static_list.lock);
	rcu_read_unlock();

	/*2.dump dynamic rule list*/
	total_len += len;
	len = snprintf(buf + total_len, PAGE_SIZE - total_len,
		"[Dynamic Rule list]:\n");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;
	len = snprintf(buf + total_len, PAGE_SIZE - total_len,
		"index hit_cnt name\trule_ID mask al_mac\tssid\tclient_mac\tprotocol"
		" dst_IP\tsrc_IP\tdst_port src_port output\n");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;

	spin_lock_bh(&ctx->dynamic_list.lock);
	rcu_read_lock();
	hlist_for_each_entry_rcu(r, &ctx->dynamic_list.head, entry) {
		len = snprintf(buf + total_len, PAGE_SIZE - total_len, "%d\t%ld\t%s\t"RULE_ID_RAW" %04x "MACSTR_RAW"\t%s\t"
			MACSTR_RAW"\t%d %d.%d.%d.%d %d.%d.%d.%d %d %d %d\n",
			r->rule.index, r->match_count, r->rule.rule_name, PRINT_RULE_ID_RAW(r->rule.rule_identifier),r->rule.mask,
			PRINT_MAC_RAW(r->rule.al_mac), r->rule.ssid, PRINT_MAC_RAW(r->rule.client_mac),
			r->rule.tcpip.protocol, PRINT_IP_RAW(r->rule.tcpip.dst_addr.v4_addr),
			PRINT_IP_RAW(r->rule.tcpip.src_addr.v4_addr), r->rule.tcpip.dst_port,
			r->rule.tcpip.src_port, r->rule.rule_output);

		if (snprintf_error(PAGE_SIZE - total_len, len)) {
			spin_unlock_bh(&ctx->dynamic_list.lock);
			rcu_read_unlock();
			goto end;
		}

		total_len += len;
	}
	rcu_read_unlock();
	spin_unlock_bh(&ctx->dynamic_list.lock);

	/*3.dump session list*/
	total_len += len;
	len = snprintf(buf + total_len, PAGE_SIZE - total_len,
		"[Session hash table]:\n");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;
	len = snprintf(buf + total_len, PAGE_SIZE - total_len,
		"[hash] [dst IP]\t[src IP]\t\t[dst port]\t[src port]\t[output]\t[age]\t[match_cnt]\t[rule_index]\n");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;

	rcu_read_lock();
	for (i = 0; i < SP_TCPIP_HASH_SIZE; i++) {
		spin_lock_bh(&ctx->hash[i].hash_lock);
		hlist_for_each_entry_rcu(session, &ctx->hash[i].head, entry) {
			len = snprintf(buf + total_len, PAGE_SIZE - total_len, "%d "IPSTR_RAW"\t"
				IPSTR_RAW"\t%d\t%d\t%d\t%ld\t%ld\t%d\n", i, PRINT_IP_RAW(session->tuple.dst_addr.v4_addr),
				PRINT_IP_RAW(session->tuple.src_addr.v4_addr), session->tuple.dst_port,
				session->tuple.src_port, session->pcp, session->age, session->match_count,
				session->correspond_rule_idx);

			if (snprintf_error(PAGE_SIZE - total_len, len)) {
				spin_lock_bh(&ctx->hash[i].hash_lock);
				rcu_read_unlock();
				goto end;
			}

			total_len += len;
		}
		spin_unlock_bh(&ctx->hash[i].hash_lock);
	}
	rcu_read_unlock();

	/*4. dump learn complete session list*/
	len = snprintf(buf + total_len, PAGE_SIZE - total_len,
		"[hash] [report] [mask] [ip_ver] [dst IP]\t[src IP]\t\t[dst port]\t[src port]\t[up]\t[age]\t\n");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;
	rcu_read_lock();
	for (i = 0; i < SP_LEARN_HASH_SIZE; i++) {
		spin_lock_bh(&ctx->learn_hash[i].hash_lock);
		hlist_for_each_entry_rcu(learn_comp, &ctx->learn_hash[i].head, entry) {
			len = snprintf(buf + total_len, PAGE_SIZE - total_len, "%d\t%d\t%02x\t%02x\t"IPSTR_RAW"\t"
				IPSTR_RAW"\t%d\t%d\t%d\t%ld\n", i, learn_comp->report, learn_comp->learn_event.mask,
				learn_comp->learn_event.ip_ver, PRINT_IP_RAW(learn_comp->learn_event.dst_addr.v4_addr),
				PRINT_IP_RAW(learn_comp->learn_event.src_addr.v4_addr), learn_comp->learn_event.pkt_dst_port,
				learn_comp->learn_event.pkt_src_port, learn_comp->learn_event.up,
				learn_comp->age);

			if (snprintf_error(PAGE_SIZE - total_len, len)) {
				spin_lock_bh(&ctx->learn_hash[i].hash_lock);
				rcu_read_unlock();
				goto end;
			}

			total_len += len;
		}
		spin_unlock_bh(&ctx->learn_hash[i].hash_lock);
	}
	rcu_read_unlock();

	/*4. dump dscp pcp mapping table*/
	len = snprintf(buf + total_len, PAGE_SIZE - total_len, "[DSCP PCP MAPPING TBL]\n");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;
	for (i = 1; i <= 64; i++) {
		len = snprintf(buf + total_len, PAGE_SIZE - total_len, "0x%02x ", ctx->dptbl.dptbl[i - 1]);
		if (snprintf_error(PAGE_SIZE - total_len, len))
			goto end;

		total_len += len;

		if ((i % 16) == 0) {
			len = snprintf(buf + total_len, PAGE_SIZE - total_len, "\n");
			if (snprintf_error(PAGE_SIZE - total_len, len))
				goto end;

			total_len += len;
		}
	}

end:
	len = total_len;
	return len;
}

unsigned int sp_learn_event_hash_idx(const u8 *saddr,
	const u16 sport, const u8 *daddr, const u16 dport, unsigned char ip_ver)
{
	unsigned int h_idx = 0, src_addr = 0, dst_addr = 0;

	if (ip_ver == IP_V4) {
		src_addr = *((unsigned int *)saddr);
		dst_addr = *((unsigned int *)daddr);
	} else if (ip_ver == IP_V6) {
		src_addr = *((unsigned int *)saddr) ^ *((unsigned int *)(saddr + 4))
			^ *((unsigned int *)(saddr + 8)) ^ *((unsigned int *)(saddr + 12));
		dst_addr = *((unsigned int *)daddr) ^ *((unsigned int *)(daddr + 4))
			^ *((unsigned int *)(daddr + 8)) ^ *((unsigned int *)(daddr + 12));
	}

	h_idx = sp_tcpip_hash(src_addr, sport, dst_addr, dport) & (SP_LEARN_HASH_SIZE - 1) ;

	return h_idx;
}

/*
 *sp get tcpip session
 *internal using in service_prioritization.c
 */
struct learn_comp_element *sp_get_learn_complete_event(struct learn_complete_hash_head *hash, const u8 *saddr,
	const u16 sport, const u8 *daddr, const u16 dport, unsigned char ip_ver)
{
	struct learn_comp_element *comp = NULL;
	unsigned int h_idx = 0;
#if 0
	int xx = 0;
#endif

	h_idx = sp_learn_event_hash_idx(saddr, sport, daddr, dport, ip_ver);

	rcu_read_lock();
	hlist_for_each_entry_rcu(comp, &hash[h_idx].head, entry) {
		/*
		printk("%s %d "IPSTR_RAW " " IPSTR_RAW " %d %d""\n", __func__, __LINE__,
			PRINT_IP_RAW(session->tuple.src_addr.v4_addr),
			PRINT_IP_RAW(session->tuple.dst_addr.v4_addr),
			session->tuple.src_port, session->tuple.dst_port);
		*/
#if 0
		if (xx++ >= 100000) {
			printk("!!!!!!!!!!!!!!!!!!!%s %d\n", __func__, __LINE__);
		}
#endif
		if (comp->learn_event.pkt_src_port == sport && comp->learn_event.pkt_dst_port == dport) {
			if (ip_ver == IP_V4) {
				if (comp->learn_event.src_addr.v4_addr == *((unsigned int*)saddr) &&
					comp->learn_event.dst_addr.v4_addr == *((unsigned int*)daddr))
					break;
			} else if (ip_ver == IP_V6) {
				if (!memcmp(comp->learn_event.src_addr.v6_addr, saddr, 16) &&
					!memcmp(comp->learn_event.dst_addr.v6_addr, daddr, 16))
					break;
			}
		}
	}
	rcu_read_unlock();

	return comp;
}

/*
 *sp create learn complete element
 *internal using in service_prioritization.c
 */
struct learn_comp_element *sp_create_learn_complete_element(struct sp_learn_complete_event *event)
{
	struct learn_comp_element *comp = NULL;

	comp = kmalloc(sizeof(struct learn_comp_element), GFP_ATOMIC);

	if (comp) {
		memset(comp, 0, sizeof(struct learn_comp_element));
		comp->learn_event = *event;
		comp->age = jiffies;
		comp->report = 0;
		comp->learn_count = 0;
	}

	return comp;
}

/*
 *sp free session function
 *internal using in service_prioritization.c
 */
static void free_learn_complete_element(struct rcu_head *head)
{
	struct learn_comp_element *comp = container_of(head, struct learn_comp_element, rcu);
	pr_info("free learn_complete\n");
	kfree(comp);
}
/*
 *sp notify
 *internal using and will be exported to other module for using(e.g. mtqos) in service_prioritization.c
 */
int sp_notify(unsigned char up, unsigned char mask, unsigned char version, unsigned char *saddr,
	unsigned char *daddr, unsigned short src_port, unsigned short dst_port)
{
	struct sp_learn_complete_event event;
	struct learn_comp_element *learn_comp = NULL;
	struct sp_context *ctx = &sp_ctx;
	unsigned int new_hidx = 0;

	if (up == 0 || up > 7)
		return -1;

	memset(&event, 0, sizeof(event));

	event.up = up;
	event.ip_ver = version;
	event.mask = mask;

	/*soure address*/
	if (mask & BIT(0)) {
		if (!saddr)
			return -1;
		if (version == 0x04)
			event.src_addr.v4_addr = *((unsigned int*)saddr);
		else if (version == 0x06)
			memcpy(event.src_addr.v6_addr, saddr, 16);
	}
	/*destination address*/
	if (mask & BIT(1)) {
		if (!daddr)
			return -1;
		if (version == 0x04)
			event.dst_addr.v4_addr = *((unsigned int*)daddr);
		else if (version == 0x06)
			memcpy(event.dst_addr.v6_addr, daddr, 16);
	}
	/*soure port*/
	if (mask & BIT(2)) {
		event.pkt_src_port = src_port;
	}
	/*soure port*/
	if (mask & BIT(3)) {
		event.pkt_dst_port = dst_port;
	}

	rcu_read_lock();
	learn_comp = sp_get_learn_complete_event(ctx->learn_hash, (unsigned char *)&event.src_addr,
		event.pkt_src_port, (unsigned char *)&event.dst_addr, event.pkt_dst_port, event.ip_ver);

	/*it means already having learned this session*/
	if (learn_comp) {
		learn_comp->age = jiffies;
		learn_comp->learn_count++;
		rcu_read_unlock();
		return 0;
	}
	rcu_read_unlock();

	learn_comp = sp_create_learn_complete_element(&event);

	if (!learn_comp)
		return -1;

	new_hidx = sp_learn_event_hash_idx((unsigned char *)&event.src_addr,
		event.pkt_src_port, (unsigned char *)&event.dst_addr, event.pkt_dst_port, event.ip_ver);

	spin_lock_bh(&ctx->learn_hash[new_hidx].hash_lock);
	hlist_add_head_rcu(&learn_comp->entry, &ctx->learn_hash[new_hidx].head);
	spin_unlock_bh(&ctx->learn_hash[new_hidx].hash_lock);

	return 0;
}

/*
 *sp notify wrap function
 *internal using in service_prioritization.c
 */
int sp_notify_wrap(struct sk_buff *skb, unsigned char up, unsigned char mask)
{
	int ret = 0;
	unsigned char dscp_value = 0;
	struct ip_5_tuple tuple;
#if 0
	int xx = 0;
#endif

	/*1. get tcpip tuple*/
	ret = get_tuple_from_skb(skb, DIR_UPSTREAM, &tuple, &dscp_value);

	if (ret == 0)
		sp_notify(up, mask, tuple.ip_ver, (unsigned char *)&tuple.src_addr,
			(unsigned char *)&tuple.dst_addr, tuple.src_port, tuple.dst_port);

	return ret;
}

int sp_learn_hash_clear(void)
{
	struct sp_context *ctx = &sp_ctx;
	int ret = 0, i = 0;
	struct hlist_node *n = NULL;
	struct learn_comp_element *learn_comp = NULL;

	for (i = 0; i < SP_LEARN_HASH_SIZE; i++) {
		spin_lock_bh(&ctx->learn_hash[i].hash_lock);
		hlist_for_each_entry_safe(learn_comp, n, &ctx->learn_hash[i].head, entry) {
			hlist_del_rcu(&learn_comp->entry);
			call_rcu(&learn_comp->rcu, free_learn_complete_element);
		}
		spin_unlock_bh(&ctx->learn_hash[i].hash_lock);
	}
	return ret;
}

int sp_update_own_almac(unsigned char *almac)
{
	struct sp_context *ctx = &sp_ctx;

	memcpy(ctx->own_al_mac, almac, ETH_ALEN);
	pr_info("sp update own almac to" MAC2STR "\n", PRINT_MAC(ctx->own_al_mac));

	return 0;
}

unsigned int sp_hook_fn_pre_rout(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	void *priv, struct sk_buff *skb, const struct nf_hook_state *state
#else
	unsigned int hooknum, struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *)
#endif
)
{
	const struct net_device *indev = NULL;
	struct map_net_device *in_map_dev = NULL;
	unsigned char device_type;
	int ret = NF_ACCEPT;
	int match = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	indev = state->in;
#else
	indev = in;
#endif

	if (!sp_ctx.conf.enable)
		goto end;

	rcu_read_lock();
	in_map_dev = get_map_net_dev_by_mac(indev->dev_addr);
	if (!in_map_dev) {
		rcu_read_unlock();
		ret = NF_ACCEPT;
		goto end;
	}
	device_type = in_map_dev->inf.dev_type;
	rcu_read_unlock();

	/*step e: service prioritization process
	 * Purpose: 1. change pcp value in vlan header if existed
	 *			2. change dscp filed in IP header
	 */

	if (device_type == AP)
		sp_function(&sp_ctx, skb, DIR_UPSTREAM);
	else if (device_type == APCLI)
		sp_function(&sp_ctx, skb, DIR_DOWNSTREAM);
	else if (device_type == ETH) {
		/*Since we don't know the data direction when in_dev equals to ETH,
		 *we should do sp_function twice for both data direction at worst case.
		 */
		match = sp_function(&sp_ctx, skb, DIR_UPSTREAM);
		if (!match)
			sp_function(&sp_ctx, skb, DIR_DOWNSTREAM);
	}

end:
	return ret;
}

unsigned int sp_hook_fn_local_out(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	void *priv, struct sk_buff *skb, const struct nf_hook_state *state
#else
	unsigned int hooknum, struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *)
#endif
)
{
	if (sp_ctx.conf.enable)
		sp_function(&sp_ctx, skb, DIR_DOWNSTREAM);

	return NF_ACCEPT;
}

unsigned int sp_hook_fn_ip_pre_routing(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	void *priv, struct sk_buff *skb, const struct nf_hook_state *state
#else
	unsigned int hooknum, struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *)
#endif
)
{
	unsigned int ret = NF_ACCEPT;
	struct map_net_device *in_map_dev = NULL;
	const struct net_device *indev = NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	indev = state->in;
#else
	indev = in;
#endif

	if (!sp_ctx.conf.enable)
		return ret;

	rcu_read_lock();
	in_map_dev = get_map_net_dev_by_mac(indev->dev_addr);
	rcu_read_unlock();

	/*Only process the packet that is from WAN ethernet*/
	if (!in_map_dev)
		sp_function(&sp_ctx, skb, DIR_DOWNSTREAM);

	return ret;
}

static struct nf_hook_ops sp_ops[] = {
	{
		/*pre-rourting hook for multi backhaul*/
		.hook		= sp_hook_fn_pre_rout,
		.pf		= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_PRE_ROUTING,
		.priority	= NF_BR_PRI_BRNF,
		//.owner		= THIS_MODULE,
	},
	{
		/*fowarding hook for local out*/
		.hook		= sp_hook_fn_local_out,
		.pf		= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_LOCAL_OUT,
		.priority	= NF_BR_PRI_FILTER_OTHER,
	},
	{
		.hook 		= sp_hook_fn_ip_pre_routing,
		.pf			= NFPROTO_IPV4,
		.hooknum 	= NF_INET_PRE_ROUTING,
		.priority 	= NF_IP_PRI_FIRST,
	},
};

/*
 *sp initialization function
 *exported to other file for using
 */
int service_prioritization_init(struct kobject *parent_obj)
{
	int ret = 0, i = 0;
	struct sp_context *ctx = &sp_ctx;

	memset(ctx, 0, sizeof(struct sp_context));

	ctx->sp_obj = kobject_create_and_add("sp", parent_obj);
	if (!ctx->sp_obj) {
		pr_info("kobject for sp create failure\n");
		return -1;
	}

	ret = sysfs_create_group(ctx->sp_obj, &sp_attr_group);
	if (ret) {
		pr_info("sysfs for sp create failure\n");
		return -1;
	}

	INIT_HLIST_HEAD(&ctx->static_list.head);
	spin_lock_init(&ctx->static_list.lock);

	INIT_HLIST_HEAD(&ctx->dynamic_list.head);
	spin_lock_init(&ctx->dynamic_list.lock);

	for (i = 0; i < SP_TCPIP_HASH_SIZE; i++) {
		INIT_HLIST_HEAD(&ctx->hash[i].head);
		spin_lock_init(&ctx->hash[i].hash_lock);

		INIT_HLIST_HEAD(&ctx->learn_hash[i].head);
		spin_lock_init(&ctx->learn_hash[i].hash_lock);
	}

	ret = sp_set_conf(ctx, 1, 0, 0, 1);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,14,0))
	timer_setup(&ctx->sp_period, sp_period, 0);
#else
	init_timer(&ctx->sp_period);
	ctx->sp_period.function = sp_period;
	ctx->sp_period.data = (unsigned long)ctx;
#endif
	ctx->sp_period.expires = jiffies + SP_PERIOD;
	add_timer(&ctx->sp_period);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	ret = nf_register_hooks(&sp_ops[0], ARRAY_SIZE(sp_ops));
#else
	ret = nf_register_net_hooks(&init_net, &sp_ops[0], ARRAY_SIZE(sp_ops));
#endif
	if (ret < 0) {
		pr_info("register nf hook fail, ret = %d\n", ret);
	}

	return ret;
}

/*
 *sp de-initialization function
 *exported to other file for using
 */
int service_prioritization_deinit(void)
{
	struct sp_context *ctx = &sp_ctx;
	int ret = 0, i = 0;
	struct sp_rule_element *r = NULL;
	struct hlist_node *n = NULL;
	struct tcpip_session_element *session = NULL;
	struct learn_comp_element *learn_comp = NULL;

	sysfs_remove_group(ctx->sp_obj, &sp_attr_group);
	kobject_put(ctx->sp_obj);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	nf_unregister_hooks(&sp_ops[0], ARRAY_SIZE(sp_ops));
#else
	nf_unregister_net_hooks(&init_net, &sp_ops[0], ARRAY_SIZE(sp_ops));
#endif

	del_timer_sync(&ctx->sp_period);

	spin_lock_bh(&ctx->static_list.lock);
	hlist_for_each_entry_safe(r, n, &ctx->static_list.head, entry) {
		hlist_del_rcu(&r->entry);
		kfree(r);
	}
	spin_unlock_bh(&ctx->static_list.lock);

	spin_lock_bh(&ctx->dynamic_list.lock);
	hlist_for_each_entry_safe(r, n, &ctx->dynamic_list.head, entry) {
		hlist_del_rcu(&r->entry);
		kfree(r);
	}
	spin_unlock_bh(&ctx->dynamic_list.lock);

	for (i = 0; i < SP_TCPIP_HASH_SIZE; i++) {
		spin_lock_bh(&ctx->hash[i].hash_lock);
		hlist_for_each_entry_safe(session, n, &ctx->hash[i].head, entry) {
			hlist_del_rcu(&session->entry);
			kfree(session);
		}
		spin_unlock_bh(&ctx->hash[i].hash_lock);
	}

	for (i = 0; i < SP_LEARN_HASH_SIZE; i++) {
		spin_lock_bh(&ctx->learn_hash[i].hash_lock);
		hlist_for_each_entry_safe(learn_comp, n, &ctx->learn_hash[i].head, entry) {
			hlist_del_rcu(&learn_comp->entry);
			kfree(learn_comp);
		}
		spin_unlock_bh(&ctx->learn_hash[i].hash_lock);
	}

	return ret;
}

