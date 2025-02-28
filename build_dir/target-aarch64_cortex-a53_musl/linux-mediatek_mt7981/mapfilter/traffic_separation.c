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
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/if_vlan.h>

#include "traffic_separation.h"
#include "mtkmapfilter.h"
#include "debug.h"

#define MAX_VLAN_NUM 48
#define INVALID_VLAN_ID		4095
#define MAX_NUM_TRANSPARENT_VLAN 128

#ifndef BIT
#define BIT(x) (1U << (x))
#endif

struct client_context {
	struct hlist_head client_head;
	spinlock_t hash_lock;
};

struct transparent_vlan {
	unsigned char transparent_enabled;
	unsigned int bitmap_transparent_vids[128];
};

struct traffic_separation_config {
	unsigned char enable;
	struct kobject *ts_obj;
	unsigned short primary_vid;
	unsigned char default_pcp;
	unsigned char pvid_num;
	unsigned int bitmap_pvid[128];
	struct transparent_vlan trans_vlan;
	struct client_context clients[HASH_TABLE_SIZE];
};
unsigned short size_of_int = sizeof(int) * 8;

struct traffic_separation_config ts_config;

static ssize_t map_ts_debug_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf);

static struct kobj_attribute map_sysfs_ts_debug =
		__ATTR(ts_debug_show, S_IRUGO, map_ts_debug_show, NULL);

static struct attribute *ts_sysfs[] = {
	&map_sysfs_ts_debug.attr,
	NULL,
};
static struct attribute_group ts_attr_group = {
	.attrs = ts_sysfs,
};

struct client_db *create_client_db(unsigned char mac[], unsigned short vid,
	unsigned char ssid_len, char *ssid, unsigned char *al_mac)
{
	struct client_db *client = NULL;

	client = (struct client_db*)kmalloc(sizeof(struct client_db), GFP_ATOMIC);

	if (client) {
		memset(client, 0, sizeof(struct client_db));
		memcpy(client->mac, mac, ETH_ALEN);
		client->vid = vid;
		if (ssid_len > 32)
			ssid_len = 32;
		client->ssid_len = ssid_len;
		strncpy(client->ssid, ssid, ssid_len);
		memcpy(client->al_mac, al_mac, ETH_ALEN);
	}

	return client;
 }

struct client_db *get_client_db(unsigned char mac[])
{
	struct client_db *client = NULL;
	unsigned char hash_idx = MAC_ADDR_HASH_INDEX(mac);
	struct hlist_head *head = &ts_config.clients[hash_idx].client_head;

	hlist_for_each_entry_rcu(client, head, hlist) {
		if (!memcmp(client->mac, mac, ETH_ALEN))
			break;
	}

	return client;
}
static void free_client_db(struct rcu_head *head);

int update_client_db(unsigned char mac[], unsigned short vid,
	unsigned char ssid_len, char *ssid, unsigned char *al_mac)
{
	struct client_db *client = NULL, *client_new = NULL;
	int ret = 0;
	unsigned char update = 0, free = 1;
	unsigned char hash_idx = MAC_ADDR_HASH_INDEX(mac);
	struct hlist_head *head = &ts_config.clients[hash_idx].client_head;

	if (ssid_len > 32)
		goto end;

	rcu_read_lock();
	client = get_client_db(mac);
	if (!client)
		goto end;

	if (client->vid != vid || client->ssid_len != ssid_len ||
		strncmp(client->ssid, ssid, ssid_len) || memcmp(client->al_mac, al_mac, ETH_ALEN)) {
		update = 1;
	}
	rcu_read_unlock();

	if (update == 1) {
		client_new = create_client_db(mac, vid, ssid_len, ssid, al_mac);

		if (!client_new)
			goto end;

		hlist_for_each_entry_rcu(client, head, hlist) {
			if (!memcmp(client->mac, mac, ETH_ALEN)) {
				hlist_replace_rcu(&client->hlist, &client_new->hlist);
				free = 0;
				call_rcu(&client->rcu, free_client_db);
			}
		}

		if (free)
			kfree(client_new);
	}

end:
	return ret;
}

static void free_client_db(struct rcu_head *head)
{
	struct client_db *dev = container_of(head, struct client_db, rcu);
	pr_info("----->free client db rcu %p,%p\n", head, dev);
	kfree(dev);
}

void remove_client_db(unsigned char mac[])
{
	struct client_db *client = NULL;
	unsigned char hash_idx = MAC_ADDR_HASH_INDEX(mac);
	struct client_context *head = &ts_config.clients[hash_idx];

	spin_lock_bh(&head->hash_lock);
	hlist_for_each_entry_rcu(client, &head->client_head, hlist) {
		if (!memcmp(client->mac, mac, ETH_ALEN)) {
			hlist_del_rcu(&client->hlist);
			call_rcu(&client->rcu, free_client_db);
			break;
		}
	}
	spin_unlock_bh(&head->hash_lock);
}

void clear_client_db(void)
{
	struct client_db *client = NULL;
	struct hlist_node *n;
	struct client_context *head = NULL;
	int i = 0;
	struct hlist_head clear_list;

	INIT_HLIST_HEAD(&clear_list);

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		head = &ts_config.clients[i];

		spin_lock_bh(&head->hash_lock);
		hlist_for_each_entry_safe(client, n, &head->client_head, hlist) {
			hlist_del_rcu(&client->hlist);
			hlist_add_head_rcu(&client->hlist, &clear_list);
		}
		spin_unlock_bh(&head->hash_lock);
	}

	hlist_for_each_entry_safe(client, n, &clear_list, hlist) {
		hlist_del_rcu(&client->hlist);
		synchronize_rcu();
		kfree(client);
	}
}

unsigned char is_ts_enable(void)
{
	return ts_config.pvid_num;
}

unsigned char handle_ts_default_8021q(unsigned short primary_vid, unsigned char default_pcp)
{
	ts_config.primary_vid = primary_vid;
	ts_config.default_pcp = default_pcp;
	pr_info("handle_ts_default_8021q vid=%d, pcp=%d\n", primary_vid, default_pcp);
	return 0;
}

unsigned char handle_ts_policy(struct ts_policy *policy)
{
	unsigned char i = 0, index = 0, offset = 0;

	if (policy->num > MAX_VLAN_NUM) {
		pr_info("invalid traffic separtion policy number=%d\n", policy->num);
		ts_config.pvid_num = 0;
		return -1;
	}

	memset(ts_config.bitmap_pvid, 0, sizeof(ts_config.bitmap_pvid));
	ts_config.pvid_num = policy->num;

	pr_info("ts policy number=%d\n", ts_config.pvid_num);
	for (i = 0; i < policy->num; i++) {
		if (policy->ssid_2_vid[i].vlan_id < 1 || policy->ssid_2_vid[i].vlan_id >= INVALID_VLAN_ID)
			continue;
		index = policy->ssid_2_vid[i].vlan_id / size_of_int;
		offset = policy->ssid_2_vid[i].vlan_id % size_of_int;
		ts_config.bitmap_pvid[index] |= BIT(offset);
		pr_info("ts policy(%d) vid=%d ([%d]|1<<%d)=%d\n", i,
			policy->ssid_2_vid[i].vlan_id, index, offset, ts_config.bitmap_pvid[index]);
	}

	return 0;
}

unsigned char handle_transparent_vlan(struct transparent_vids *tvids)
{
	unsigned char i = 0, index = 0, offset = 0;

	memset(ts_config.trans_vlan.bitmap_transparent_vids, 0,
		sizeof(ts_config.trans_vlan.bitmap_transparent_vids));

	pr_info("transparent_vid_num=%d\n", tvids->num);

	ts_config.trans_vlan.transparent_enabled = tvids->num;
	memset(ts_config.trans_vlan.bitmap_transparent_vids, 0,
		sizeof(ts_config.trans_vlan.bitmap_transparent_vids));

	for (i = 0; i < tvids->num; i++) {
		if (tvids->vids[i] < 1 || tvids->vids[i] >= INVALID_VLAN_ID)
			continue;
		index = tvids->vids[i] / size_of_int;
		offset = tvids->vids[i] % size_of_int;
		ts_config.trans_vlan.bitmap_transparent_vids[index] |= BIT(offset);
		pr_info("transparent vids[%d] vid=%d ([%d]|1<<%d)=%d\n", i, tvids->vids[i],
			index, offset, ts_config.trans_vlan.bitmap_transparent_vids[index]);
	}

	return 0;
}

unsigned char handle_client_vid(struct client_vid *client)
{
	unsigned char hash_idx = MAC_ADDR_HASH_INDEX(client->client_mac);
	struct client_context *client_head = NULL;
	struct client_db *cli_db = NULL;

	/* mark it since other information in client db is useful for us
	if (client->vid < 1 || client->vid >= 4095) {
		printk("client vid invlaid(%d)\n", client->vid);
		return 0;
	}
	*/
	client_head = &ts_config.clients[hash_idx];

	if (client->status == STATION_JOIN) {
		rcu_read_lock();
		cli_db = get_client_db(client->client_mac);
		if (!cli_db) {
			rcu_read_unlock();
			cli_db = create_client_db(client->client_mac, client->vid,
				client->ssid_len, client->ssid, client->al_mac);
			spin_lock_bh(&client_head->hash_lock);
			hlist_add_head_rcu(&cli_db->hlist, &client_head->client_head);
			spin_unlock_bh(&client_head->hash_lock);  
		} else {
			rcu_read_unlock();
			update_client_db(client->client_mac, client->vid,
				client->ssid_len, client->ssid, client->al_mac);
		}
	} else if(client->status == STATION_LEAVE) {
		remove_client_db(client->client_mac);
	}

	return 0;
}



unsigned int is_vid_in_policy(unsigned short vid)
{
	return ts_config.bitmap_pvid[vid / size_of_int] & BIT(vid % size_of_int);
}

unsigned char is_transparent_vlan_on(void)
{
	return ts_config.trans_vlan.transparent_enabled;
}

unsigned int is_transparent_vlan(unsigned short vid)
{
	return ts_config.trans_vlan.bitmap_transparent_vids[vid / size_of_int] & BIT(vid % size_of_int);
}

#define VLAN_VID_MASK		0x0fff /* VLAN Identifier */
#define vlan_tx_tag_get_id(__skb)	((__skb)->vlan_tci & VLAN_VID_MASK)

unsigned int RtmpOsCsumAdd(unsigned int csum, unsigned int addend)
{
	unsigned int res = csum;
	res += addend;
	return res + (res < addend);
}

void RtmpOsSkbPullRcsum(struct sk_buff *skb, unsigned int len)
{
	if (len > skb->len)
		return;

	skb_pull(skb, len);
	if (skb->ip_summed == CHECKSUM_COMPLETE)
		skb->csum = RtmpOsCsumAdd(skb->csum, ~csum_partial(skb->data, len, 0));
	else if (skb->ip_summed == CHECKSUM_PARTIAL &&
		 (skb->csum_start - (skb->data - skb->head)) < 0)
		skb->ip_summed = CHECKSUM_NONE;
}

static inline void remove_vlan_tag(struct sk_buff *skb)
{
	unsigned short VLAN_LEN = 4;
	unsigned char extra_field_offset = 2 * ETH_ALEN;

	memmove(skb->data + VLAN_LEN,
		skb->data, extra_field_offset);
	RtmpOsSkbPullRcsum(skb, 4);
	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);
	skb_reset_transport_header(skb);
	skb_reset_mac_len(skb);
}
unsigned char add_vlan_tag(struct sk_buff *skb, unsigned short vlan_id, unsigned char vlan_pcp);

unsigned int ts_tx_process(struct sk_buff *skb, unsigned char wan_tag)
{
	unsigned short vid = 0;
	unsigned char vlan_in_header = 0;

	/*firstly, check the transparent vlan*/
	if (is_transparent_vlan_on()) {
		if (skb->vlan_proto == htons(ETH_P_8021Q) && skb->vlan_tci) {
			vid = vlan_tx_tag_get_id(skb);
			if (is_transparent_vlan(vid))
				return NF_ACCEPT;
		}
	}

	/*then, check normal traffic separation configuration*/
	if (!is_ts_enable())
		return NF_ACCEPT;

	/*if vlan info is in skb->vlan_tci*/
	if (skb->vlan_proto == htons(ETH_P_8021Q) && skb->vlan_tci) {
		vid = vlan_tx_tag_get_id(skb);
	} else {
	/*if vlan info is in ethernet header*/
		if (skb->protocol ==  htons(ETH_P_8021Q)) {
			struct vlan_hdr *vhdr;
			unsigned short vlan_tci;

			vhdr = (struct vlan_hdr *) skb->data;
			vlan_tci = ntohs(vhdr->h_vlan_TCI);
			vid = vlan_tci & VLAN_VID_MASK;
			vlan_in_header = 1;
		}
	}

	if (vid) {
		if (vid == ts_config.primary_vid || wan_tag) {
			skb->vlan_tci = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
			__vlan_hwaccel_clear_tag(skb);
#endif
		}
		/*whether need to check if the vid is inlcued in the traffic policy???*/
	}

	return NF_ACCEPT;
}

unsigned int ts_rx_process(struct sk_buff *skb, unsigned char wan_tag)
{
	unsigned short vid = 0;
	struct ethhdr *hdr;
	struct client_db *cli_db = NULL;

	/*firstly, check the transparent vlan*/
	if (is_transparent_vlan_on()) {
		if (skb->vlan_proto == htons(ETH_P_8021Q) && skb->vlan_tci) {
			vid = vlan_tx_tag_get_id(skb);
			
			if (is_transparent_vlan(vid))
				return NF_ACCEPT;
		}
	}

	/*then, check normal traffic separation configuration*/
	if (!is_ts_enable())
		return NF_ACCEPT;

	/*skb with vlan tag ?? how about ETH_P_8021AD*/
	if (skb->vlan_proto == htons(ETH_P_8021Q) && skb->vlan_tci) {
		vid = vlan_tx_tag_get_id(skb);
		if (vid == ts_config.primary_vid || !is_vid_in_policy(vid))
			return NF_DROP;
		return NF_ACCEPT;
	}

	if (wan_tag) {
		hdr = eth_hdr(skb);
		rcu_read_lock();
		cli_db = get_client_db(hdr->h_dest);
		if (!cli_db) {
			rcu_read_unlock();
			goto primary_tag;
		}

		vid = cli_db->vid;
		rcu_read_unlock();

		if (vid >= 3 && vid <= 4094) {
			/*need add secondary vid for wireless sta on wan interface*/
			if (vid != ts_config.primary_vid) {
				__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vid);
				goto end;
			}
		}
	}

primary_tag:
	if (ts_config.primary_vid != INVALID_VLAN_ID) {
		vid = ((ts_config.default_pcp & 0x7) << 13)  | (ts_config.primary_vid & 0x0FFF);
		__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vid);
	}

end:
	return NF_ACCEPT;
}

unsigned int ts_local_in_process(struct sk_buff *skb)
{
	unsigned short vid = 0;

	/*firstly, check the transparent vlan*/
	if (is_transparent_vlan_on()) {
		if (skb->vlan_proto == htons(ETH_P_8021Q) && skb->vlan_tci) {
			vid = vlan_tx_tag_get_id(skb);
			
			if (is_transparent_vlan(vid))
				return NF_ACCEPT;
		}
	}

	if (!is_ts_enable())
		return NF_ACCEPT;

	if (skb->vlan_proto == htons(ETH_P_8021Q) && skb->vlan_tci) {
//		printk("ts_local_in_process: remove vlan tag vid=%d\n", vid);
		skb->vlan_tci = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
		__vlan_hwaccel_clear_tag(skb);
#endif
	}

	return NF_ACCEPT;
}

unsigned char add_vlan_tag(struct sk_buff *skb, unsigned short vlan_id, unsigned char vlan_pcp)
{
	unsigned char vlan_tci = 0;

	vlan_tci |= 0x0fff & vlan_id;
	vlan_tci |= vlan_pcp << 13;

	skb = vlan_insert_tag(skb, htons(ETH_P_8021Q), vlan_tci);
	if (skb) {
		skb->protocol = htons(ETH_P_8021Q);
		skb->vlan_tci = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
		__vlan_hwaccel_clear_tag(skb);
#endif
		return 0;
	} else {
		return 1;
	}
}

unsigned int ts_local_out_process(struct sk_buff *skb, unsigned char out_type)
{
	unsigned short vid = 0;
	struct ethhdr *hdr;
	struct client_db *cli_db = NULL;
	
	/*firstly, check the transparent vlan*/
	if (is_transparent_vlan_on()) {
		if (skb->vlan_proto == htons(ETH_P_8021Q) && skb->vlan_tci) {
			vid = vlan_tx_tag_get_id(skb);
			
			if (is_transparent_vlan(vid))
				goto end;
		}
	}
	
	if (!is_ts_enable())
		goto end;
	hdr = eth_hdr(skb);

	rcu_read_lock();
	cli_db = get_client_db(hdr->h_dest);
	if (!cli_db) {
		rcu_read_unlock();
		goto end;
	}

	vid = cli_db->vid;
	rcu_read_unlock();

	if (vid >= 1 && vid <= 4094) {
		/*if out device is ethernet and its vlan id equals to primary vlan id, do not add vlan tag for it*/
		if (out_type == ETH && vid == ts_config.primary_vid)
			goto end;
		__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vid);
#if 0
		/*if out device is ethernet and its vlan id equals to primary vlan id, do not add vlan tag for it*/
		if (out_type == ETH && vid == ts_config.primary_vid)
			goto end;

		/*if it is already 802.1q header*/
		if (skb->protocol == htons(ETH_P_8021Q)) {
			struct vlan_hdr *vhdr;
			unsigned short vlan_tci = 0;

			vhdr = (struct vlan_hdr *) skb->data;
			vlan_tci |= 0x0fff & vid;
			vhdr->h_vlan_TCI = htons(vlan_tci);
		} else {
		/*if it is not 802.1q header, insert the vlan header to packet*/
			skb_push(skb, ETH_HLEN);
			add_vlan_tag(skb, vid, 0);
			skb_pull(skb, ETH_HLEN);
		}
#endif
	}
end:
	return NF_ACCEPT;
}

unsigned int ts_ip_pre_routing_process(struct sk_buff *skb)
{
	unsigned short vid = 0;
	unsigned char *dst = NULL;
	struct client_db *cli_db = NULL;

	if (!is_ts_enable())
		goto end;

	skb_set_network_header(skb, 0);
	skb_push(skb, ETH_HLEN);
	dst = skb->data;
	skb_pull(skb, ETH_HLEN);

	rcu_read_lock();
	cli_db = get_client_db(dst);
	if (!cli_db) {
		rcu_read_unlock();
		goto end;
	}

	vid = cli_db->vid;
	rcu_read_unlock();

	if (vid >= 1 && vid <= 4094) {
		__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vid);
	}
end:
	return NF_ACCEPT;
}

static ssize_t map_ts_debug_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int len = 0, total_len = 0;
	unsigned short i = 0;
	struct client_context *head = NULL;
	struct client_db * client = NULL;

	len = snprintf(buf, PAGE_SIZE, "traffic separation config information\n");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;

	len = snprintf(buf + total_len, PAGE_SIZE, "ts switch %s\n", ts_config.enable ? "on" : "off");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;

	len = snprintf(buf + total_len, PAGE_SIZE, "ts %s\n", is_ts_enable() ? "enabled" : "disabled");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;

	len = snprintf(buf + total_len, PAGE_SIZE, "primary vlan id=%d, primary pcp=%d\n",
		ts_config.primary_vid, ts_config.default_pcp);
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;

	for (i = 1; i < INVALID_VLAN_ID; i++) {
		if (is_vid_in_policy(i)) {
			len = snprintf(buf + total_len, PAGE_SIZE, "ts policy vids=%d\n", i);
			if (snprintf_error(PAGE_SIZE - total_len, len))
				goto end;
			total_len += len;
		}
	}

	for (i = 1; i < INVALID_VLAN_ID; i++) {
		if (is_transparent_vlan(i)) {
			len = snprintf(buf + total_len, PAGE_SIZE, "transparent vids=%d\n", i);
			if (snprintf_error(PAGE_SIZE - total_len, len))
				goto end;
			total_len += len;
		}
	}

	rcu_read_lock();
	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		head = &ts_config.clients[i];
		hlist_for_each_entry_rcu(client, &head->client_head, hlist) {
			len = snprintf(buf + total_len, PAGE_SIZE,
				"mac(" MAC2STR ") vid=%d ssid=%s ssid_len=%d AL_MAC("MAC2STR")\n", PRINT_MAC(client->mac),
				client->vid, client->ssid, client->ssid_len, PRINT_MAC(client->al_mac));
			if (snprintf_error(PAGE_SIZE - total_len, len)) {
				rcu_read_unlock();
				goto end;
			}
			total_len += len;
		}
	}
	rcu_read_unlock();
end:
	len = total_len;
	return len;
}

void enable_ts(unsigned char enable)
{
	ts_config.enable = enable;
	pr_info("traffic separation is %s\n", enable ? "on" : "off");
}

unsigned int ts_hook_fn_pre_rout(
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
	unsigned char wan_tag;
	struct vlan_dev_priv *vlan = NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	indev = state->in;
#else
	indev = in;
#endif

	rcu_read_lock();
	in_map_dev = get_map_net_dev_by_mac(indev->dev_addr);
	if (!in_map_dev) {
		rcu_read_unlock();
		if (ts_config.enable && is_transparent_vlan_on()) {
			if (indev->priv_flags & IFF_802_1Q_VLAN) {
				vlan = (struct vlan_dev_priv *)netdev_priv(indev);
				if (vlan && is_transparent_vlan(vlan->vlan_id)) {
					__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vlan->vlan_id);
					return NF_ACCEPT;
				}
			}
		}
		ret = NF_ACCEPT;
		goto end;
	}
	device_type = in_map_dev->inf.dev_type;
	wan_tag = in_map_dev->wan_tag;
	rcu_read_unlock();

	/*step d, traffic separation for ETH Mixed interface*/
	if (ts_config.enable) {
		if (device_type == ETH)
			ret = ts_rx_process(skb, wan_tag);
	}

end:
	return ret;
}

unsigned int ts_hook_fn_local_in(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	void *priv,	struct sk_buff *skb, const struct nf_hook_state *state
#else
	unsigned int hooknum, struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *)
#endif
)
{
	const struct net_device *indev = NULL;
	unsigned int ret = NF_ACCEPT;

	if (ts_config.enable) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
		indev = state->in;
#else
		indev = in;
#endif
		ret = ts_local_in_process(skb);
	}

	return ret;
}

unsigned int ts_hook_fn_local_out(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	void *priv, struct sk_buff *skb, const struct nf_hook_state *state
#else
	unsigned int hooknum, struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *)
#endif
)
{
	const struct net_device *outdev = NULL;
	struct map_net_device *out_map_dev = NULL;
	unsigned int ret = NF_ACCEPT;
	unsigned char device_type;

	if (ts_config.enable) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	outdev = state->out;
#else
	outdev = out;
#endif

		rcu_read_lock();
		out_map_dev = get_map_net_dev_by_mac(outdev->dev_addr);
		if (!out_map_dev) {
	//		pr_info("%s-should not happen(out_map_dev " MAC2STR " not found)\n", __func__, PRINT_MAC(outdev->dev_addr));
			rcu_read_unlock();
			return ret;
		}

		device_type = out_map_dev->inf.dev_type;
		rcu_read_unlock();

		ret = ts_local_out_process(skb, device_type);
	}

	return ret;
}

unsigned int ts_hook_fn_local_out_post_routing(
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
	unsigned int ret = NF_ACCEPT;
	unsigned char device_type, primary_interface;
	unsigned char wan_tag = 0;
	struct vlan_dev_priv *vlan = NULL;
	unsigned short vid = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	outdev = state->out;
#else
	outdev = out;
#endif

	hdr = eth_hdr(skb);
	if (!ts_config.enable)
		goto end;

	rcu_read_lock();

	out_map_dev = get_map_net_dev_by_mac(outdev->dev_addr);
	if (!out_map_dev) {
		rcu_read_unlock();
		if (is_transparent_vlan_on()) {
			if (skb->vlan_proto == htons(ETH_P_8021Q) && skb->vlan_tci) {
				vid = skb->vlan_tci & VLAN_VID_MASK;
				if (is_transparent_vlan(vid)) {
					if (outdev->priv_flags & IFF_802_1Q_VLAN) {
						vlan = (struct vlan_dev_priv *)netdev_priv(outdev);
						if (vlan && vlan->vlan_id == vid) {
							skb->vlan_tci = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
							__vlan_hwaccel_clear_tag(skb);
#endif
						}
					}
					return NF_ACCEPT;
				}
			}
		}
		goto end;
	}

	device_type = out_map_dev->inf.dev_type;
	primary_interface = out_map_dev->primary_interface;
	wan_tag = out_map_dev->wan_tag;
	rcu_read_unlock();

	if (device_type == ETH)
		ret = ts_tx_process(skb, wan_tag);

end:
	return ret;
}

unsigned int ts_hook_fn_ip_pre_routing(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	void *priv, struct sk_buff *skb, const struct nf_hook_state *state
#else
	unsigned int hooknum, struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *)
#endif
)
{
	unsigned int ret = NF_ACCEPT;

	if (ts_config.enable) {
		ret = ts_ip_pre_routing_process(skb);
	}

	return ret;
}

static struct nf_hook_ops ts_ops[] = {
	{
		/*pre-rourting hook for multi backhaul*/
		.hook		= ts_hook_fn_pre_rout,
		.pf		= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_PRE_ROUTING,
		.priority	= NF_BR_PRI_BRNF,
		//.owner		= THIS_MODULE,
	},
	{
		/*hook for mapr2 to strip vlan tag for local-in packet.*/
		.hook		= ts_hook_fn_local_in,
		.pf		= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_LOCAL_IN,
		.priority	= NF_BR_PRI_BRNF,
		//.owner		= THIS_MODULE,
	},
	{
		/*fowarding hook for local out*/
		.hook		= ts_hook_fn_local_out,
		.pf		= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_LOCAL_OUT,
		.priority	= NF_BR_PRI_FILTER_OTHER,
	},
	{
		.hook 		= ts_hook_fn_local_out_post_routing,
		.pf			= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_POST_ROUTING,
		.priority	= NF_BR_PRI_FILTER_OTHER,
	},
	{
		.hook 		= ts_hook_fn_ip_pre_routing,
		.pf			= NFPROTO_IPV4,
		.hooknum 	= NF_INET_PRE_ROUTING,
		.priority 	= NF_IP_PRI_FIRST,
	},
};

void ts_init(struct kobject *parent_obj)
{
	int i = 0, ret = 0;

	memset(&ts_config, 0, sizeof(struct traffic_separation_config));

	ts_config.ts_obj = kobject_create_and_add("ts", parent_obj);
	if (!ts_config.ts_obj) {
		pr_info("kobject for ts create failure\n");
		return;
	}

	ret = sysfs_create_group(ts_config.ts_obj, &ts_attr_group);
	if (ret) {
		pr_info("sysfs for ts create failure\n");
		return;
	}

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		INIT_HLIST_HEAD(&ts_config.clients[i].client_head);
		spin_lock_init(&ts_config.clients[i].hash_lock);
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	ret = nf_register_hooks(&ts_ops[0], ARRAY_SIZE(ts_ops));
#else
	ret = nf_register_net_hooks(&init_net, &ts_ops[0], ARRAY_SIZE(ts_ops));
#endif
	if (ret < 0) {
		pr_info("register nf hook fail, ret = %d\n", ret);
	}
}

void ts_deinit(void)
{
	sysfs_remove_group(ts_config.ts_obj, &ts_attr_group);
	kobject_put(ts_config.ts_obj);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	nf_unregister_hooks(&ts_ops[0], ARRAY_SIZE(ts_ops));
#else
	nf_unregister_net_hooks(&init_net, &ts_ops[0], ARRAY_SIZE(ts_ops));
#endif

	clear_client_db();
	return;
}
