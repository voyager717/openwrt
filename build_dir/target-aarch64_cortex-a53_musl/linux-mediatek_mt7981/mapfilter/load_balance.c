#include <linux/init.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_bridge.h>
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
#include <linux/tcp.h>
#include <linux/udp.h>

#include "load_balance.h"
#include "mtkmapfilter.h"
#include "debug.h"
struct radio_context radio_ctx;

/*
       ---------                            ------------------------
      |	ra0  	| 	<----------	|apcli0				ra0	|	<----------
      |         	|				|						|
      |	rai0	|	<----------	|apclii0				rai0	|	<----------
	--------				 ------------------------
  [ap_session_ctx]	      		[apcli_session_ctx]		 [ap_session_ctx]
*/

struct session_context apcli_session_ctx[SESSION_HASH_TABLE_SIZE];
struct session_context ap_session_ctx[SESSION_HASH_TABLE_SIZE];

#define HASH_MASK 0x000001FF

#define OVERLOAD_THRESHOLD_5G	80
#define LOW_THRESHOLD_5G		75

#define OVERLOAD_THRESHOLD_24G	80

/*assume 24g band's maxium tp is 180Mbps*/
#define _24G_MAX_THROUGHPUT		180

#define _24G 0x01
#define _5GL 0x02
#define _5GH 0x04
#define _5G	0x06
struct timer_list period_steer;
#define PERIOD_TIMEER (5 * HZ)
#define SHORT_PERIOD_TIMER (2 * HZ)
#define SESSION_TIMEOUT (30 * HZ)

unsigned char dynamic_load_balance = 0;
struct kobject *load_balance_obj = NULL;

static ssize_t map_load_balance_debug_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf);

static struct kobj_attribute map_sysfs_lb_debug =
		__ATTR(lb_debug_show, S_IRUGO, map_load_balance_debug_show, NULL);

static struct attribute *lb_sysfs[] = {
	&map_sysfs_lb_debug.attr,
	NULL,
};
static struct attribute_group lb_attr_group = {
	.attrs = lb_sysfs,
};
unsigned int load_balance_hook_fn_pre_rout(
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
	unsigned char device_type, primary_interface;
	unsigned char data_dir = DIR_UNKNOWN;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	indev = state->in;
#else
	indev = in;
#endif

	if (!dynamic_load_balance)
		return NF_ACCEPT;

	rcu_read_lock();
	in_map_dev = get_map_net_dev_by_mac(indev->dev_addr);
	if (!in_map_dev) {
		rcu_read_unlock();
		return NF_ACCEPT;
	}
	device_type = in_map_dev->inf.dev_type;
	primary_interface = in_map_dev->primary_interface;
	rcu_read_unlock();

	/*step c: learning for dynamic loading balance*/
	if (dynamic_load_balance) {
		if (device_type == AP)
			data_dir = DIR_UPSTREAM;
		else if (device_type == APCLI)
			data_dir = DIR_DOWNSTREAM;

		if (data_dir == DIR_UPSTREAM)
			ap_load_balance_learning(skb, (struct net_device*)indev);
		else if (data_dir == DIR_DOWNSTREAM)
			apcli_load_balance_learning(skb);
	}

	return NF_ACCEPT;
}

unsigned int load_balance_hook_fn_forward(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	void *priv, struct sk_buff *skb, const struct nf_hook_state *state
#else
	unsigned int hooknum, struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *)
#endif
)
{
	struct ethhdr *hdr;
	const struct net_device *indev = NULL, *outdev = NULL;
	struct map_net_device *in_map_dev = NULL, *out_map_dev = NULL;
	unsigned char data_dir;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	indev = state->in;
	outdev = state->out;
#else
	indev = in;
	outdev = out;
#endif
	hdr = eth_hdr(skb);

	if (!dynamic_load_balance)
		return NF_ACCEPT;

	if (indev && indev->dev_addr && outdev && outdev->dev_addr) {
		rcu_read_lock();
		out_map_dev = get_map_net_dev_by_mac(outdev->dev_addr);
		if (!out_map_dev) {
//			pr_info("%s-should not happen(out_map_dev " MAC2STR " not found)\n", __func__, PRINT_MAC(outdev->dev_addr));
			goto accept;
		}

		data_dir = out_map_dev->inf.dev_type == APCLI ? DIR_UPSTREAM : DIR_UNKNOWN;

		in_map_dev = get_map_net_dev_by_mac(indev->dev_addr);

		if (!in_map_dev) {
//			pr_info("%s-should not happen(in_map_dev " MAC2STR " not found)\n", __func__, PRINT_MAC(indev->dev_addr));
			goto accept;
		}

		if (in_map_dev->inf.dev_type != AP && out_map_dev->inf.dev_type == AP)
			data_dir = DIR_DOWNSTREAM;

		/*2.1 check the upstream data fisrt*/
		if (data_dir == DIR_UPSTREAM) {
			if (!(is_multicast_ether_addr(hdr->h_dest) || is_broadcast_ether_addr(hdr->h_dest))) {
				rcu_read_unlock();
				apcli_load_balance_process(skb);
			}
		/*2.2 check the non-upstream data*/
		} else if (data_dir == DIR_DOWNSTREAM) {
			if (!is_multicast_ether_addr(hdr->h_dest) && !is_broadcast_ether_addr(hdr->h_dest)) {
				rcu_read_unlock();
				ap_load_balance_process(skb);
			}
		}
	}

accept:
	rcu_read_unlock();
	return NF_ACCEPT;
}


unsigned int load_balance_hook_fn_local_out(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	void *priv, struct sk_buff *skb, const struct nf_hook_state *state
#else
	unsigned int hooknum, struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *)
#endif
)
{
	struct ethhdr *hdr;
	const struct net_device *outdev = NULL;
	struct map_net_device *out_map_dev = NULL;
	unsigned char data_dir;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	outdev = state->out;
#else
	outdev = out;
#endif

	hdr = eth_hdr(skb);
	if (!dynamic_load_balance)
		goto accept;

	if (outdev && outdev->dev_addr) {
		rcu_read_lock();
		out_map_dev = get_map_net_dev_by_mac(outdev->dev_addr);
		if (!out_map_dev) {
			rcu_read_unlock();
			goto accept;
		}

		data_dir = out_map_dev->inf.dev_type == AP ? DIR_DOWNSTREAM: DIR_UNKNOWN;
		rcu_read_unlock();

		if (data_dir == DIR_DOWNSTREAM) {
                  	/* only ucast pkt need load balance */
			if (!(hdr->h_dest[0] & 0x01))
				ap_load_balance_process(skb);
		}
	}

accept:
	return NF_ACCEPT;
}

static struct nf_hook_ops load_balance_ops[] = {
	{
		.hook		= load_balance_hook_fn_pre_rout,
		.pf		= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_PRE_ROUTING,
		.priority	= NF_BR_PRI_BRNF,
		//.owner		= THIS_MODULE,
	},
	{
		/*fowarding hook for multi backhaul*/
		.hook		= load_balance_hook_fn_forward,
		.pf		= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_FORWARD,
		.priority	= NF_BR_PRI_BRNF,
		//.owner		= THIS_MODULE,
	},
	{
		/*fowarding hook for local out*/
		.hook		= load_balance_hook_fn_local_out,
		.pf		= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_LOCAL_OUT,
		.priority	= NF_BR_PRI_FILTER_OTHER,
	},
};

struct session *get_most_heavy_session(struct session_context *ctx, unsigned char band)
{
	int i = 0;
	unsigned int max_len = 0;
	struct session *psession = NULL, *psession_heavy = NULL;

	for (i = 0; i < SESSION_HASH_TABLE_SIZE; i++) {
		hlist_for_each_entry_rcu(psession, &ctx[i].session_hash, hlist) {
			if ((psession->band & band) && psession->len > max_len) {
				psession_heavy = psession;
				max_len = psession_heavy->len;
			}
		}
	}

	return psession_heavy;
}

struct session *get_less_heavy_session(struct session_context *ctx, unsigned char band)
{
	int i = 0;
	unsigned int min_len = (~0U);
	struct session *psession = NULL, *psession_lessheavy = NULL;

	for (i = 0; i < SESSION_HASH_TABLE_SIZE; i++) {
		hlist_for_each_entry_rcu(psession, &ctx[i].session_hash, hlist) {
			if ((psession->band & band) && psession->len < min_len && psession->len > 0) {
				psession_lessheavy = psession;
				min_len = psession_lessheavy->len;
			}
		}
	}

	return psession_lessheavy;
}

unsigned char is_overloading(unsigned char util, unsigned char band)
{
	unsigned char threshold = band & _5G ? OVERLOAD_THRESHOLD_5G : OVERLOAD_THRESHOLD_24G;

	return (unsigned char)(util > threshold);
}

unsigned char is_lessloading(unsigned char util)
{	
	return (unsigned char)(util < LOW_THRESHOLD_5G);
}

static void free_session(struct rcu_head *head);

struct wireless_radio *get_overloading_radio(unsigned char band)
{
	struct wireless_radio *radio = NULL;

	hlist_for_each_entry_rcu(radio, &radio_ctx.radio_list, hlist) {
		if (radio->up_link_status) {
			if (is_overloading(radio->util, band) && (radio->band & band))
				break;
		}
	}

	return radio;
}

struct wireless_radio *get_non_overloading_radio(unsigned char band)
{
	struct wireless_radio *radio = NULL;

	hlist_for_each_entry_rcu(radio, &radio_ctx.radio_list, hlist) {
		if (radio->up_link_status) {
			if (!is_overloading(radio->util, band) && (radio->band & band))
				break;
		}
	}

	return radio;
}

void distribute_new_session(struct session *psession);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,14,0))
void apcli_load_balance_period(struct timer_list *timer)
#else
void apcli_load_balance_period(unsigned long _data)
#endif
{
	struct wireless_radio *radio = NULL, *radio_overld = NULL, *radio_nonld = NULL;
	struct session *psession = NULL;
	struct session_context *ctx = apcli_session_ctx;
	struct hlist_node *n;
	unsigned long peroid = PERIOD_TIMEER;
	int i = 0;

	rcu_read_lock();
	/*rule 1, check if 5G channel utilization is less that the Low Threshold*/
	hlist_for_each_entry_rcu(radio, &radio_ctx.radio_list, hlist) {
		if (radio->up_link_status && radio->band & _5G) {
			if (is_lessloading(radio->util))
				break;
		} 
	}
	
	if (radio) {
		psession = get_most_heavy_session(ctx, _24G);
		if (psession && radio->up_dev) {
			psession->band = radio->band;
			psession->dest_dev = radio->up_dev;
/*
			printk("[2.4G->5G] %d.%d.%d.%d[%d]-->%d.%d.%d.%d[%d] dest_dev=%s total_len=%u len_period=%u, updated=%lu\n", 
				PRINT_IP(psession->ip_saddr), ntohs(psession->tcp_source),
				PRINT_IP(psession->ip_daddr), ntohs(psession->tcp_dest), 
				psession->dest_dev ? psession->dest_dev->name : "N/A",
				psession->total_len, psession->len, psession->updated);
*/
		}
		goto end;
	}
	
	/*rule 2, check if one band is overloaded and another one is non-overloaded*/
	radio_overld = get_overloading_radio(_24G);
	
	if (radio_overld) {
		radio_nonld = get_non_overloading_radio(_5G);
	} else {
		radio_overld = get_overloading_radio(_5G);

		if (radio_overld) {
			radio_nonld = get_non_overloading_radio(radio_overld->band == _5GH ? _5GL : _5GH);
			if (!radio_nonld)
				radio_nonld = get_non_overloading_radio(_24G);
		}
	}

	if (radio_overld && radio_nonld) {
		psession = get_less_heavy_session(ctx, radio_overld->band);
		if (psession && radio_nonld->up_dev) {
			/*to solve issues: steer from 5G to 2G, to check if its throughput will decrease a lot*/
			if ((radio_nonld->band & _24G) && (radio_overld->band & _5G)) {
				unsigned int tp_left_capa_24g = 0;
				unsigned char cu_threshold_24g = OVERLOAD_THRESHOLD_24G;
				
				psession->speed = (psession->len * 8) / 1024 / 1024 /5;
				tp_left_capa_24g = 
					(cu_threshold_24g - radio_nonld->util) * _24G_MAX_THROUGHPUT / 100;

				if (psession->speed >= tp_left_capa_24g)
					goto end;
			}
			
			psession->band = radio_nonld->band;
			psession->dest_dev = radio_nonld->up_dev;
/*
			printk("[%s]-%d->[%s]-%d\n", radio_overld->up_dev->name, radio_overld->util,
				radio_nonld->up_dev->name, radio_nonld->util);
			printk("changing %d.%d.%d.%d[%d]-->%d.%d.%d.%d[%d] dest_dev=%s total_len=%u len_period=%u, updated=%lu\n", 
				PRINT_IP(psession->ip_saddr), ntohs(psession->tcp_source),
				PRINT_IP(psession->ip_daddr), ntohs(psession->tcp_dest), 
				psession->dest_dev ? psession->dest_dev->name : "N/A",
				psession->total_len, psession->len, psession->updated);
*/
		}
	}
	
end:
	rcu_read_unlock();

	for (i = 0; i < SESSION_HASH_TABLE_SIZE; i++) {
		spin_lock_bh(&apcli_session_ctx[i].hash_lock);
		hlist_for_each_entry_safe(psession, n, &apcli_session_ctx[i].session_hash, hlist) {
			psession->speed = (psession->len * 8) / 1024 / 1024 /5;
			psession->len = 0;
			if (time_before_eq(psession->updated + SESSION_TIMEOUT, jiffies)) {
					hlist_del_rcu(&psession->hlist);
					call_rcu(&psession->rcu, free_session);
			} else {
				if (!psession->dest_dev)
					distribute_new_session(psession);
			}
		}
		spin_unlock_bh(&apcli_session_ctx[i].hash_lock);

		spin_lock_bh(&ap_session_ctx[i].hash_lock);
		hlist_for_each_entry_safe(psession, n, &ap_session_ctx[i].session_hash, hlist) {
			psession->speed = (psession->len * 8) / 1024 / 1024 /5;
			psession->len = 0;
			if (time_before_eq(psession->updated + SESSION_TIMEOUT, jiffies)) {
					hlist_del_rcu(&psession->hlist);
					call_rcu(&psession->rcu, free_session);
			}
		}
		spin_unlock_bh(&ap_session_ctx[i].hash_lock);
	}

	mod_timer(&period_steer, jiffies + peroid);
}

void load_balance_init(struct kobject *parent_obj)
{
	int i = 0, ret = 0;

	load_balance_obj = kobject_create_and_add("load_balance", parent_obj);
	if (!load_balance_obj) {
		pr_info("kobject for load balance create failure\n");
		return;
	}
	ret = sysfs_create_group(load_balance_obj, &lb_attr_group);
	if (ret) {
		pr_info("sysfs for load_balance create failure\n");
		return;
	}

	INIT_HLIST_HEAD(&radio_ctx.radio_list);
	spin_lock_init(&radio_ctx.hash_lock);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	ret = nf_register_hooks(&load_balance_ops[0], ARRAY_SIZE(load_balance_ops));
#else
	ret = nf_register_net_hooks(&init_net, &load_balance_ops[0], ARRAY_SIZE(load_balance_ops));
#endif
	if (ret < 0) {
		pr_info("register nf hook fail, ret = %d\n", ret);
		return;
	}

	for (i = 0; i < SESSION_HASH_TABLE_SIZE; i++) {
		INIT_HLIST_HEAD(&apcli_session_ctx[i].session_hash);
		spin_lock_init(&apcli_session_ctx[i].hash_lock);
		INIT_HLIST_HEAD(&ap_session_ctx[i].session_hash);
		spin_lock_init(&ap_session_ctx[i].hash_lock);
	}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,14,0))
	timer_setup(&period_steer, apcli_load_balance_period, 0);
#else
	init_timer(&period_steer);
	period_steer.function = apcli_load_balance_period;
#endif
	period_steer.expires = jiffies + PERIOD_TIMEER;
	add_timer(&period_steer);
}

void load_balance_deinit(void)
{
	struct wireless_radio *radio = NULL;
	struct session *psession = NULL;
	struct hlist_node *n;
	int i = 0;

	kobject_put(load_balance_obj);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	nf_unregister_hooks(&load_balance_ops[0], ARRAY_SIZE(load_balance_ops));
#else
	nf_unregister_net_hooks(&init_net, &load_balance_ops[0], ARRAY_SIZE(load_balance_ops));
#endif

	spin_lock_bh(&radio_ctx.hash_lock);
	hlist_for_each_entry_safe(radio, n, &radio_ctx.radio_list, hlist) {
		hlist_del_rcu(&radio->hlist);
		kfree(radio);
	}
	spin_unlock_bh(&radio_ctx.hash_lock);

	for (i = 0; i < SESSION_HASH_TABLE_SIZE; i++) {
		spin_lock_bh(&apcli_session_ctx[i].hash_lock);
		hlist_for_each_entry_safe(psession, n, &apcli_session_ctx[i].session_hash, hlist) {
			hlist_del_rcu(&psession->hlist);
			kfree(psession);
		}
		spin_unlock_bh(&apcli_session_ctx[i].hash_lock);
		spin_lock_bh(&ap_session_ctx[i].hash_lock);
		hlist_for_each_entry_safe(psession, n, &ap_session_ctx[i].session_hash, hlist) {
			hlist_del_rcu(&psession->hlist);
			kfree(psession);
		}
		spin_unlock_bh(&ap_session_ctx[i].hash_lock);
	}

	del_timer_sync(&period_steer);
}

struct wireless_radio *get_radio(unsigned char *radio_id)
{
	struct wireless_radio *radio = NULL;
	
	hlist_for_each_entry_rcu(radio, &radio_ctx.radio_list, hlist) {
		if (ether_addr_equal(radio->radio_id, radio_id))
			return radio;
	}

	return radio;
}

struct wireless_radio *create_radio(struct radio_utilization *util, struct net_device *dev)
{
	struct wireless_radio *radio = NULL;

	radio = kmalloc(sizeof(struct wireless_radio), GFP_ATOMIC);

	if (radio) {
		memset(radio, 0, sizeof(struct wireless_radio));
		radio->band = util->band;
		radio->util = util->util;
		radio->up_dev = dev;
		memcpy(radio->radio_id, util->radio_id, ETH_ALEN);
	}

	return radio;
}

/*this API should be protected by spin_lock*/
int set_radio_channel_utilization(struct radio_utilization *util, struct net_device *up_dev)
{
	struct wireless_radio *radio = NULL;

	if (!up_dev)
		return 0;
	
	spin_lock_bh(&radio_ctx.hash_lock);
	radio = get_radio(util->radio_id);

	if (!radio) {
		radio = create_radio(util, up_dev);
		hlist_add_head_rcu(&radio->hlist, &radio_ctx.radio_list);
	}
	else
		radio->util = (radio->util + 3 * (unsigned short)(util->util)) >> 2;

	spin_unlock_bh(&radio_ctx.hash_lock);
//	printk("%s %s utilization=%d\n", __func__, up_dev->name, util->util);
	return 0;
}
struct session *create_session(u32 sip, u16 sport, u32 dip, u16 dport, unsigned int len);

void radio_link_change_update_apcli_session(unsigned char band, unsigned char link_status)
{
	struct wireless_radio *radio = NULL;
	struct session *psession = NULL, *psession_new = NULL;
	struct net_device *dev = NULL;
	struct hlist_node *n;
	int i = 0;
	struct session_context *ctx = apcli_session_ctx;
	
	rcu_read_lock();
	hlist_for_each_entry_rcu (radio, &radio_ctx.radio_list, hlist) {
		if (radio->band == band) {
			break;
		}
	}

	if (!radio)
		goto end;

	dev = radio->up_dev;

	if (!dev)
		goto end;

	/*link status not change, do nothing*/
	if (radio->up_link_status == link_status)
		goto end;

	radio->up_link_status = link_status;
	
	/*if link down, reset dest dev of all sessions belong to this band*/
	if (link_status == 0) {
		rcu_read_unlock();
		for (i = 0; i < SESSION_HASH_TABLE_SIZE; i++) {
			spin_lock_bh(&ctx[i].hash_lock);
			hlist_for_each_entry_safe(psession, n, &ctx[i].session_hash, hlist) {
				if (psession->dest_dev == dev) {
					psession_new = create_session(psession->ip_saddr, psession->tcp_source,
						psession->tcp_source, psession->tcp_dest, psession->total_len);
					if (psession_new) {
						psession_new->len = psession->len;
						hlist_replace_rcu(&psession->hlist, &psession_new->hlist);
						call_rcu(&psession->rcu, free_session);
					}
				}
			}
			spin_unlock_bh(&ctx[i].hash_lock);
		}
	}
		

end:
	rcu_read_unlock();
	return;
}


u32 get_stream_hash(const u32 saddr, const u16 sport,
				  const u32 daddr, const u16 dport)
{
	return jhash_3words(saddr, daddr, (u32)sport << 16 | (u32)dport, 16) & HASH_MASK ;
}

/*this API should be locked by RCU lock*/
struct session *get_session(struct session_context *ctx, u32 hash, u32 sip, u16 sport, 
	u32 dip, u16 dport)
{
	struct session *psession = NULL;
	
	hlist_for_each_entry_rcu(psession, &ctx[hash].session_hash, hlist) {
		if (sip == psession->ip_saddr && sport == psession->tcp_source
			&& dip == psession->ip_daddr && dport == psession->tcp_dest)
			return psession;
	}

	return psession;
}

struct session *create_session(u32 sip, u16 sport, u32 dip, u16 dport, unsigned int len)
{
	struct session *psession = NULL;

	psession = kmalloc(sizeof(struct session), GFP_ATOMIC);
	if (psession) {
		psession->ip_saddr = sip;
		psession->ip_daddr = dip;
		psession->tcp_source = sport;
		psession->tcp_dest = dport;
		psession->updated = jiffies;
		psession->len = 0;
		psession->total_len = len;
		psession->dest_dev = NULL;
/*
		printk("create_session %d.%d.%d.%d[%d]-->%d.%d.%d.%d[%d] dest_dev=%s stotal_len=%d len_period=%d, updated=%lu\n", 
			PRINT_IP(psession->ip_saddr), ntohs(psession->tcp_source),
			PRINT_IP(psession->ip_daddr), ntohs(psession->tcp_dest), 
			psession->dest_dev ? psession->dest_dev->name : "N/A",
			psession->total_len, psession->len, psession->updated);
*/
	}

	return psession;
}

static void free_session(struct rcu_head *head)
{
	struct session *psession = container_of(head, struct session, rcu);
/*
	printk("free_session %d.%d.%d.%d[%d]-->%d.%d.%d.%d[%d] dest_dev=%s stotal_len=%d len_period=%d, updated=%ld\n", 
		PRINT_IP(psession->ip_saddr), ntohs(psession->tcp_source),
		PRINT_IP(psession->ip_daddr), ntohs(psession->tcp_dest), 
		psession->dest_dev ? psession->dest_dev->name : "N/A",
		psession->total_len, psession->len, psession->updated);
*/
	kfree(psession);
}

void distribute_new_session(struct session *psession)
{
	struct wireless_radio *radio = NULL, *radio_temp = NULL;
	hlist_for_each_entry_rcu(radio, &radio_ctx.radio_list, hlist) {
		if (radio->up_link_status) {
			if (radio->band & _5G) {
				if (is_overloading(radio->util, radio->band))
					radio_temp = radio_temp == NULL ? radio : radio_temp;
				else {
					radio_temp = radio;
					break;
				}
			} else if (radio->band & _24G) {
				if (!is_overloading(radio->util, radio->band))
					radio_temp = radio;
			}
		}
	}

	if (radio_temp) {
/*
		printk("distribute this session to %s\n", radio_temp->up_dev->name);
*/
		psession->dest_dev = radio_temp->up_dev;
		psession->band = radio_temp->band;
	}
}

u32 parse_skb_2_iptcp(struct sk_buff *skb, u32 *sip, u16 *sport, u32 *dip,
	u16 *dport, unsigned char direction)
{
	const struct iphdr *iph;
	const struct tcphdr *th;
#if 0
	const struct udphdr *uh = NULL;
#endif

	u32 hash_idx = SESSION_HASH_TABLE_SIZE;

	iph = ip_hdr(skb);

	*sip = direction == DIR_UPSTREAM ? iph->saddr : iph->daddr;
	*dip = direction == DIR_UPSTREAM ? iph->daddr : iph->saddr;
	
	skb_set_transport_header(skb, sizeof(struct iphdr));

	if (iph->protocol == IPPROTO_TCP) {
		th = tcp_hdr(skb);
		*sport = direction == DIR_UPSTREAM ? th->source : th->dest;
		*dport = direction == DIR_UPSTREAM ? th->dest: th->source;
	}
	/*do not support udp sesssion, because udp data length can be 8192 bytes,
	   so it will be divided into many fragments by IP layer, and it is hard to calculate
	   the udp length for missing of udp header*/
#if 0
	else if (iph->protocol == IPPROTO_UDP) {
		uh = udp_hdr(skb);
		*sport = direction == DIR_UPSTREAM ? uh->source : uh->dest;
		*dport = direction == DIR_UPSTREAM ? uh->dest: uh->source;
	}
#endif
	else
		return hash_idx;

	hash_idx = get_stream_hash(*sip, *sport, *dip, *dport);

	return hash_idx;
}

/*call it in pre-routing, it is for learning*/
void apcli_load_balance_learning(struct sk_buff *skb)
{
	u32 sip = 0, dip = 0, hash_idx = 0;
	u16 sport = 0, dport = 0;
	struct session *psession = NULL;
	struct session_context *ctx = apcli_session_ctx;
	
	if (skb->protocol != htons(ETH_P_IP))
		return;
	
	hash_idx = parse_skb_2_iptcp(skb, &sip, &sport, &dip, &dport, DIR_DOWNSTREAM);

	if (hash_idx >= SESSION_HASH_TABLE_SIZE)
		return;

	rcu_read_lock();
	psession = get_session(ctx, hash_idx, sip, sport, dip, dport);

	if (psession) {
		psession->updated = jiffies;
		psession->len += skb->len;
		psession->total_len += skb->len;
		rcu_read_unlock();
	} else {
		rcu_read_unlock();
		
		if (skb->len <= 128)
			return;
		
		psession = create_session(sip, sport, dip, dport, skb->len);
		
		if (psession) {	
			spin_lock_bh(&ctx[hash_idx].hash_lock);
			hlist_add_head_rcu(&psession->hlist, &ctx[hash_idx].session_hash);
			spin_unlock_bh(&ctx[hash_idx].hash_lock);
		}
		else
			return;
	}
	
}

/*call it in pre-routing, it is for learning*/
void ap_load_balance_learning(struct sk_buff *skb, struct net_device *in)
{
	u32 sip = 0, dip = 0, hash_idx = 0;
	u16 sport = 0, dport = 0;
	struct session *psession = NULL;
	struct ethhdr *hdr = eth_hdr(skb);
	struct session_context *ctx = ap_session_ctx;
	int i = 0;
	
	if (skb->protocol != htons(ETH_P_IP))
		return;
	
	hash_idx = parse_skb_2_iptcp(skb, &sip, &sport, &dip, &dport, DIR_UPSTREAM);

	if (hash_idx >= SESSION_HASH_TABLE_SIZE)
		return;
	
	rcu_read_lock();
	psession = get_session(ctx, hash_idx, sip, sport, dip, dport);


	if (psession) {
		psession->updated = jiffies;
		psession->len += skb->len;
		psession->total_len += skb->len;

		psession->dest_dev = in;	/*in worst case, the dow_dev is incorrect*/
		rcu_read_unlock();
	} else {
		rcu_read_unlock();
		
		if (skb->len <= 128)
			return;
		
		psession = create_session(sip, sport, dip, dport, skb->len);
		
		if (psession) {	
			psession->dest_dev = in;
			memcpy(psession->src_mac, hdr->h_source, ETH_ALEN);
			spin_lock_bh(&ctx[hash_idx].hash_lock);
			hlist_add_head_rcu(&psession->hlist, &ctx[hash_idx].session_hash);
			spin_unlock_bh(&ctx[hash_idx].hash_lock);
		}
		else
			return;
	}

	rcu_read_lock();
	/*if ap interface receives a multicast packet, should update the dest dev of all down stream session,
	    because our wifi driver would delete the peer a4 entry when tx multicast packet*/
	if ((is_multicast_ether_addr(hdr->h_dest) || is_broadcast_ether_addr(hdr->h_dest))) {
		for (i = 0; i < SESSION_HASH_TABLE_SIZE; i++) {
			hlist_for_each_entry_rcu(psession, &ctx[i].session_hash, hlist) {
			if (!memcmp(psession->src_mac, hdr->h_source, ETH_ALEN))
				psession->dest_dev = in;
			}
		}
	}
	rcu_read_unlock();
	
}

/*call it in forwarding hook func*/
void apcli_load_balance_process(struct sk_buff *skb)
{
	u32 sip = 0, dip = 0, hash_idx = 0;
	u16 sport = 0, dport = 0;
	struct session *psession = NULL;
	struct session_context *ctx = apcli_session_ctx;

	if (skb->protocol != htons(ETH_P_IP))
		return;

	hash_idx = parse_skb_2_iptcp(skb, &sip, &sport, &dip, &dport, DIR_UPSTREAM);
	
	if (hash_idx >= SESSION_HASH_TABLE_SIZE)
		return;

	rcu_read_lock();
	psession = get_session(ctx, hash_idx, sip, sport, dip, dport);


	if (psession) {
		/*only modify the upstream direction out device*/
		if (psession->dest_dev)
			skb->dev = psession->dest_dev;
		
		psession->updated = jiffies;
		psession->len += skb->len;
		psession->total_len += skb->len;
		rcu_read_unlock();
	} else {
		rcu_read_unlock();

		/*do not care about packet with length less than 128 bytes*/
		if (skb->len <= 128)
			return;
		
		psession = create_session(sip, sport, dip, dport, skb->len);
		
		if (psession) {
			rcu_read_lock();
			distribute_new_session(psession);
			if (psession->dest_dev)
				skb->dev = psession->dest_dev;
			
			rcu_read_unlock();
			spin_lock_bh(&ctx[hash_idx].hash_lock);
			hlist_add_head_rcu(&psession->hlist, &ctx[hash_idx].session_hash);
			spin_unlock_bh(&ctx[hash_idx].hash_lock);
		}
		else
			return;
	}
}

/*call it in forwarding hook func*/
void ap_load_balance_process(struct sk_buff *skb)
{
	u32 sip = 0, dip = 0, hash_idx = 0;
	u16 sport = 0, dport = 0;
	struct session *psession = NULL;
	struct session_context *ctx = ap_session_ctx;
	
	if (skb->protocol != htons(ETH_P_IP))
		return;

	hash_idx = parse_skb_2_iptcp(skb, &sip, &sport, &dip, &dport, DIR_DOWNSTREAM);
	
	if (hash_idx >= SESSION_HASH_TABLE_SIZE)
		return;

	rcu_read_lock();
	psession = get_session(ctx, hash_idx, sip, sport, dip, dport);


	if (psession) {

		if (psession->dest_dev)
			skb->dev = psession->dest_dev;
		
		psession->updated = jiffies;
		psession->len += skb->len;
		psession->total_len += skb->len;
		rcu_read_unlock();
	} else {
		rcu_read_unlock();

		/*do not care about packet with length less than 128 bytes*/
		if (skb->len <= 128)
			return;
		
		psession = create_session(sip, sport, dip, dport, skb->len);
		if (psession) {
			spin_lock_bh(&ctx[hash_idx].hash_lock);
			hlist_add_head_rcu(&psession->hlist, &ctx[hash_idx].session_hash);
			spin_unlock_bh(&ctx[hash_idx].hash_lock);
		}
		else
			return;
	}
}

static ssize_t map_load_balance_debug_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int len = 0, total_len = 0;
	struct wireless_radio *radio = NULL;
	struct session *psession = NULL;
	int i = 0;

	rcu_read_lock();
	hlist_for_each_entry_rcu(radio, &radio_ctx.radio_list, hlist) {
		len = snprintf(buf + total_len, PAGE_SIZE,
			"\tradio id:" MAC2STR ", band=%02x, util=%d, uplink=%d, up_dev=%s\n",
			PRINT_MAC(radio->radio_id), radio->band, radio->util,
			radio->up_link_status, radio->up_dev->name);

		if (snprintf_error(PAGE_SIZE - total_len, len))
			goto end;

		total_len += len;
	}

	for (i = 0; i < SESSION_HASH_TABLE_SIZE; i++) {
		hlist_for_each_entry_rcu(psession, &apcli_session_ctx[i].session_hash, hlist) {
			len = snprintf(buf + total_len, PAGE_SIZE,
				"\tAPCLI " IP2STR "[%d]-->" IP2STR "[%d] dest_dev=%s total_len=%u len_period=%u, updated=%lu, speed=%uMbps\n",
				PRINT_IP(psession->ip_saddr), ntohs(psession->tcp_source),
				PRINT_IP(psession->ip_daddr), ntohs(psession->tcp_dest),
				psession->dest_dev ? psession->dest_dev->name : "N/A",
				psession->total_len, psession->len, psession->updated, psession->speed);

			if (snprintf_error(PAGE_SIZE - total_len, len))
				goto end;

			total_len += len;
		}
	}

	for (i = 0; i < SESSION_HASH_TABLE_SIZE; i++) {
		hlist_for_each_entry_rcu(psession, &ap_session_ctx[i].session_hash, hlist) {
			len = snprintf(buf + total_len, PAGE_SIZE,
				"\tAP " IP2STR "[%d]-->" IP2STR "[%d] dest_dev=%s total_len=%u len_period=%u, updated=%lu, speed=%uMbps\n",
				PRINT_IP(psession->ip_saddr), ntohs(psession->tcp_source),
				PRINT_IP(psession->ip_daddr), ntohs(psession->tcp_dest),
				psession->dest_dev ? psession->dest_dev->name : "N/A",
				psession->total_len, psession->len, psession->updated, psession->speed);

			if (snprintf_error(PAGE_SIZE - total_len, len))
				goto end;

			total_len += len;
		}
	}
end:
	rcu_read_unlock();
	return total_len;
}

void update_apcli_link_status(struct apcli_link_status *status)
{
	struct map_net_device *dev = NULL;
	unsigned char band, link_status;
	rcu_read_lock();
	dev = get_map_net_dev_by_mac(status->in.mac);
	if (!dev) {
		pr_info("%s error interface " MAC2STR " not found\n",
			__func__, PRINT_MAC(status->in.mac));
		goto end;
	}
	if (dev->inf.dev_type != APCLI) {
		pr_info("%s error %s is not apcli interface\n", __func__, dev->inf.name);
		goto end;
	}
	band = dev->inf.band;
	link_status = status->link_status;
	rcu_read_unlock();

	pr_info("update_apcli_link_status %s link %s\n", dev->inf.name, link_status ? "up" : "down");
	radio_link_change_update_apcli_session(band, link_status);

	return;
end:
	rcu_read_unlock();
	return;
}

void enable_load_balance(unsigned char enable)
{
	dynamic_load_balance = enable;
	pr_info("dynamic load balance is %s\n", enable ? "on" : "off");
}
