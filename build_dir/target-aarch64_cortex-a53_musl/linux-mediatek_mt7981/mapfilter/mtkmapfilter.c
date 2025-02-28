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
#include <linux/ip.h>
#include <../net/8021q/vlan.h>

#include "mtkmapfilter.h"
#include "dual_bh.h"
#include "load_balance.h"
#include "traffic_separation.h"
#include "service_prioritization.h"
#include "debug.h"
MODULE_LICENSE("Dual BSD/GPL");

#define VERSION "v3.0.1.2"

/* drop default specific u32 ip addr */
#define SPECIFIC_IP_ADDR1 (htonl(0x08080808))			/* IP 8.8.8.8 */
#define SPECIFIC_IP_ADDR2 (htonl(0xD043DEDE))			/* IP 208.67.222.222 */

struct map_nt_entry {
	struct list_head list;
	unsigned char mac[6];
	unsigned char dev_addr[6];
	unsigned long updated;
	char intf[IFNAMSIZ + 1];
};

#define MAP_NETLINK 26
#define MAX_ENTRY_CNT 256
#define MAX_MSGSIZE 1024
#define NT_TIMEOUT (600 * HZ) /*10mins*/
#define TIMER_EXCUTE_PERIOD (5*HZ)
#define FDB_TIMEOUT (15*HZ)
#define DROP_TIMEOUT (180 * HZ)

#define CMDU_HLEN	8
#define TOPOLOGY_DISCOVERY	0x0000
#define VENDOR_SPECIFIC		0x0004

#define END_OF_TLV_TYPE 0

struct list_head nt[HASH_TABLE_SIZE];
spinlock_t nt_lock;
struct sock *nl_sk;
struct timer_list nt_timer;
struct timer_list dp_timer;
struct drop_specific_dest_ip_status drop_ip_status;
struct drop_specific_dest_ip_addr drop_ip_addr;
int nt_cnt;
unsigned char almac[ETH_ALEN];
unsigned char mtk_oui[3] = {0x00, 0x0C, 0xE7};

struct hlist_head		dev_hash[HASH_TABLE_SIZE];

spinlock_t dev_lock;

char *dev_type2str(unsigned char type)
{
	switch(type) {
		case AP:
			return "AP";
		case APCLI:
			return "APCLI";
		case ETH:
			return "ETH";
		default:
			return "UNKNOWN";
	}
}

char *primary_type2str(unsigned char primary)
{
	switch(primary) {
		case INF_UNKNOWN:
			return "INF_NORMAL";
		case INF_PRIMARY:
			return "INF_PRIMARY";
		case INF_NONPRIMARY:
			return "INF_NONPRIMARY";
		default:
			return "UNKNOWN";
	}
}

bool eth_addr_equal(unsigned char *addr1, unsigned char *addr2)
{
	unsigned char ret = 0;
	ret =	((addr1[0] ^ addr2[0]) | (addr1[1] ^ addr2[1]) | (addr1[2] ^ addr2[2]) |
			(addr1[3] ^ addr2[3]) | (addr1[4] ^ addr2[4]) | (addr1[5] ^ addr2[5]));
	if (ret == 0) {
		return 1;
	}
	return 0;
}

bool is_zero_mac(unsigned char *mac)
{
	unsigned char zero_mac[6] = {0x00};

	return eth_addr_equal(mac, zero_mac);
}

void hex_dump_all(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
	unsigned char *pt;
	int x;

	pt = pSrcBufVA;
	pr_info("%s: %p, len = %d\n",str,  pSrcBufVA, SrcBufLen);
	for (x=0; x<SrcBufLen; x++)
	{
		if (x % 16 == 0)
			pr_info("0x%04x : ", x);
		pr_info("%02x ", ((unsigned char)pt[x]));
		if (x%16 == 15) pr_info("\n");
	}
	pr_info("\n");
}

unsigned short get_1905_message_type(unsigned char *msg)
{
	unsigned char *pos = NULL;
	unsigned short mtype = 0;

	pos = msg;
	/*jump to message_type*/
	pos += 2;
	memcpy(&mtype, pos, 2);
	mtype = htons(mtype);

	return mtype;
}

int get_cmdu_tlv_length(unsigned char *buf)
{
    unsigned char *temp_buf = buf;
    unsigned short length = 0;

    temp_buf += 1;

    length = (*temp_buf);
    length = (length << 8) & 0xFF00;
    length = length |(*(temp_buf+1));

    return (length+3);
}

unsigned char is_vendor_specific_topology_discovery(struct sk_buff *skb)
{
	/*1905 internal using vendor specific tlv format
	   type			1 byte			0x11
	   length			2 bytes			0x, 0x
	   oui			3 bytes			0x00, 0x0C, 0xE7
	   function type	1 byte			0xff
	   suboui			3 bytes			0x00, 0x0C, 0xE7
	   sub func type	1 byte			0x
	*/
	unsigned char *temp_buf = skb->data;
	unsigned char charactor_value[5] = {0xFF, 0x00, 0x0C, 0xE7, 0x00};
	int left_len = 0;

	left_len = (int)skb->len;
	left_len -= CMDU_HLEN;

	if (left_len < 11)
		return 0;

	temp_buf += CMDU_HLEN + 6;

	if ( !memcmp(temp_buf, charactor_value, sizeof(charactor_value)))
		return 1;
	else
		return 0;
}

int add_discovery_inf_vstlv(struct sk_buff *skb, unsigned char *rmac)
{
	unsigned char *temp_buf = skb->data;
	int length =0, left_len = 0;
	int offset = 0;

	/*vslen = tlvType(1 octet) + tlvLength(2 octets) + OUI(3 octets) +
	* subType(1 octet) + subLength(2 octets) + mac(6 octets)
	*/
	unsigned char vsinfo[15] = {0};

	/*build inf vs tlv*/
	vsinfo[0] = 11; /*Vendor-specific TLV value*/
	vsinfo[1] = 0x00;
	vsinfo[2] = 0x0c;
	memcpy(&vsinfo[3], mtk_oui, 3);
	vsinfo[6] = 0x00; /*subTyle inf mac*/
	vsinfo[7] = 0x00;
	vsinfo[8] = 0x06;
	memcpy(&vsinfo[9], rmac, 6);

	/*1905 cmdu header*/
	left_len = (int)skb->len;
	left_len -= CMDU_HLEN;
	temp_buf += CMDU_HLEN;
	offset += CMDU_HLEN;
	while (left_len > 0) {
        if (*temp_buf == END_OF_TLV_TYPE) {
            break;
        } else {
            /*ignore extra tlv*/
            length = get_cmdu_tlv_length(temp_buf);
            temp_buf += length;
			left_len -= length;
			offset += length;
        }
    }

	if (left_len < 0) {
		pr_info("%s error discovery msg\n", __func__);
		return -1;
	}

	/*vsinfo(15 octets) + End of message TLV(3 octets)*/
	if ((left_len + skb_tailroom(skb)) < 18) {
		/*do skb expand*/
		pr_info("%s not enough room for vstlv & end tlv\n", __func__);
		if (0 == pskb_expand_head(skb, 0, 15, GFP_ATOMIC)) {
			pr_info("%s expand room success\n", __func__);
			temp_buf = skb->data + offset;
			memcpy(temp_buf, vsinfo, 15);
			temp_buf += 15;
			memset(temp_buf, 0, 3);
			temp_buf += 3;
			skb_put(skb, 15);
		} else {
			pr_info("%s expand room fail drop!\n", __func__);
			return -1;
		}
	} else {
		if (left_len >= 18) {
			memcpy(temp_buf, vsinfo, 15);
			temp_buf += 15;
			memset(temp_buf, 0, 3);
		} else {
			memcpy(temp_buf, vsinfo, 15);
			temp_buf += 15;
			memset(temp_buf, 0, 3);
			temp_buf += 3;
			skb_put(skb, 15);
		}
	}

	return 0;
}

void nt_timeout_func(
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,14,0))
	struct timer_list *timer
#else
	unsigned long arg
#endif
)
{
	int i;
	struct map_nt_entry *pos, *n;

	spin_lock_bh(&nt_lock);
	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		list_for_each_entry_safe(pos, n, &nt[i], list) {
			if (time_after(jiffies, pos->updated+NT_TIMEOUT)) {
				pr_info("timeout! free" MAC2STR "\n", PRINT_MAC(pos->mac));
				list_del(&pos->list);
				nt_cnt--;
				kfree(pos);
			}
		}
	}
	spin_unlock_bh(&nt_lock);
	mod_timer(&nt_timer, jiffies + TIMER_EXCUTE_PERIOD);

	return;
}

void dp_timeout_func(
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,14,0))
	struct timer_list *timer
#else
	unsigned long arg
#endif
)
{
	/* stop dropping the specific ip addr*/
	drop_ip_status.drop_flag = 0;
	pr_info("mapfilter:drop IP addr timeout! stop dropping IP addr.");

	return;
}

int nt_get_intf(char *addr)
{
	int hash_idx = MAC_ADDR_HASH_INDEX(addr);
	struct map_nt_entry *pos;
	spin_lock_bh(&nt_lock);
	list_for_each_entry(pos, &nt[hash_idx], list) {
		if (eth_addr_equal(pos->mac, addr)) {
			memcpy(addr, pos->dev_addr, ETH_ALEN);
			spin_unlock_bh(&nt_lock);
			return 0;
		}
	}
	spin_unlock_bh(&nt_lock);
	memset(addr, 0 , ETH_ALEN);
	return -1;
}

unsigned int map_basic_hook_fn_pre_rout(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	void *priv, struct sk_buff *skb, const struct nf_hook_state *state
#else
	unsigned int hooknum, struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *)
#endif
)
{
	const struct net_device *indev = NULL;
	struct ethhdr *hdr = eth_hdr(skb);
	int hash_idx = MAC_ADDR_HASH_INDEX(hdr->h_source);
	struct map_nt_entry *pos;
	unsigned short mtype = 0;
	int ret = 0;

	/*step 1: if it's not a 1905 packet, ignore it.*/
	if (likely(skb->protocol != htons(0x893A)))
		return NF_ACCEPT;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	indev = state->in;
#else
	indev = in;
#endif

	/*drop 1905 packets*/
	if (!memcmp(hdr->h_source, almac, ETH_ALEN))
		return NF_DROP;

	/*only handle skb without nonlinear memory*/
	if (likely(skb->data_len == 0)) {
		/*step 1.1: check if the pkts is a discovery message
		* need add vendor specific tlv specify receiving net device mac
		*/
		mtype = get_1905_message_type(skb->data);
		if (mtype == TOPOLOGY_DISCOVERY) {
			ret = add_discovery_inf_vstlv(skb, indev->dev_addr);
			if (ret < 0) {
				pr_info("%s drop error msg\n", __func__);
				return NF_DROP;
			}
		}
	}

	spin_lock_bh(&nt_lock);
	/*step 2: if the source address was add to the table, update it.*/
	list_for_each_entry(pos, &nt[hash_idx], list) {
		if (eth_addr_equal(pos->mac, hdr->h_source)) {
			strncpy(pos->intf, indev->name, IFNAMSIZ);
			memcpy(pos->dev_addr, indev->dev_addr, 6);
			pos->updated = jiffies;
			break;
		}
	}

	/*step 3: if the source address has not been added to the table.*/
	if (&pos->list == &nt[hash_idx] && nt_cnt <= MAX_ENTRY_CNT) {
		/*add a new entry to table.*/
		pos = kmalloc(sizeof(struct map_nt_entry), GFP_ATOMIC);
		if (pos == NULL)
			goto out;

		memset(pos, 0, sizeof(struct map_nt_entry));
		memcpy(pos->mac, hdr->h_source, 6);
		memcpy(pos->dev_addr, indev->dev_addr, 6);
		strncpy(pos->intf, indev->name, IFNAMSIZ);
		pos->updated = jiffies;

		pr_info("alloc new entry for " MAC2STR ", interface:%s\n",
			PRINT_MAC(hdr->h_source), indev->name);
		pr_info("recv intf mac " MAC2STR "\n", PRINT_MAC(indev->dev_addr));
		list_add_tail(&pos->list, &nt[hash_idx]);
		nt_cnt++;

	}
out:
	spin_unlock_bh(&nt_lock);

	return NF_ACCEPT;
}

unsigned int map_auto_role_select_hook_fn_pre_rout(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	void *priv, struct sk_buff *skb, const struct nf_hook_state *state
#else
	unsigned int hooknum, struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *)
#endif
)
{
	int ret = NF_ACCEPT;
	unsigned int dest_ip;
	const struct iphdr *iph;

	/* add: drop the ping packets of specific IP addr && specific size to avoid two controller
	 * in auto-role
	 */
	if (unlikely(drop_ip_status.drop_flag == 1)) {
		if (skb->protocol == htons(ETH_P_IP)) {
			iph = ip_hdr(skb);
			dest_ip = iph->daddr;
			if (iph->protocol == IP_PROTOCOL_ICMP &&
				ntohs(iph->tot_len) == PING_SIZE &&
				(drop_ip_addr.addr1 == dest_ip || drop_ip_addr.addr2 == dest_ip)) {
				pr_info("mapfilter: drop the packet of specific IP addr: %08x, size = %d \n",
					dest_ip, ntohs(iph->tot_len));
				ret = NF_DROP;
			}
		}
	}
	
	return ret;
}

unsigned int map_basic_hook_fn_forward(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	void *priv, struct sk_buff *skb, const struct nf_hook_state *state
#else
	unsigned int hooknum, struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *)
#endif
)
{
	struct ethhdr *hdr = eth_hdr(skb);

	/*step 1: drop 1905 neighbor multicast packet*/
	if (unlikely(skb->protocol == htons(0x893A))) {
		if ((hdr->h_dest[0]&1) || !memcmp(hdr->h_dest, almac, ETH_ALEN))
			return NF_DROP;
	}

	return NF_ACCEPT;
}

void send_msg(char *msg, int pid)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	int len = NLMSG_SPACE(MAX_MSGSIZE);

	if (!msg || !nl_sk)
		return;

	skb = alloc_skb(len, GFP_KERNEL);
	if (!skb) {
		pr_info("send_msg:alloc_skb error\n");
		return;
	}
	nlh = nlmsg_put(skb, 0, 0, 0, MAX_MSGSIZE, 0);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0))
	NETLINK_CB(skb).pid = 0;
#else
	NETLINK_CB(skb).portid = 0;
#endif

	NETLINK_CB(skb).dst_group = 0;
	memcpy(NLMSG_DATA(nlh), msg, 6);
	netlink_unicast(nl_sk, skb, pid, MSG_DONTWAIT);
}

struct net_device *mt_dev_get_by_name(const char *name)
{
	struct net_device *dev;

	dev = dev_get_by_name(&init_net, name);

	if (dev)
		dev_put(dev);

	return dev;
}

struct map_net_device *get_map_net_dev_by_mac(unsigned char *mac)
{
	struct map_net_device *dev = NULL;
	struct hlist_head *head = &dev_hash[MAC_ADDR_HASH_INDEX(mac)];

	hlist_for_each_entry_rcu(dev, head, hlist) {
		if (ether_addr_equal(dev->inf.mac, mac))
			break;
	}

	return dev;
}

void free_map_net_device(struct rcu_head *head)
{
	struct map_net_device *dev = container_of(head, struct map_net_device, rcu);
	pr_info("----->free map device rcu %p\n", head);
	kfree(dev);
}

struct map_net_device *create_map_net_device(struct local_interface *inf,
	struct net_device *dev, struct net_device *dest_dev, unsigned char primary)
{
	struct map_net_device *new_dev = NULL;

	new_dev = kmalloc(sizeof(struct map_net_device), GFP_ATOMIC);
	if (!new_dev) {
		pr_info("alloc new map_net_device fail\n");
		return NULL;
	}
	memset(new_dev, 0, sizeof(struct map_net_device));
	new_dev->dev = dev;
	new_dev->dest_dev = dest_dev;
	new_dev->primary_interface = primary;

	INIT_HLIST_NODE(&new_dev->hlist);

	memcpy(&new_dev->inf, inf, sizeof(struct local_interface));
	if (dev && dev->dev_addr)
		memcpy(new_dev->inf.mac, dev->dev_addr, ETH_ALEN);

	return new_dev;
}

int clear_map_net_device(void)
{
	int i = 0;
	struct hlist_head *head = NULL;
	struct map_net_device *dev = NULL;
	struct hlist_node *n = NULL;
	struct hlist_head clear_list;

	INIT_HLIST_HEAD(&clear_list);

	spin_lock_bh(&dev_lock);
	for(i = 0; i < HASH_TABLE_SIZE; i++) {
		head = &dev_hash[i];
		hlist_for_each_entry_safe(dev, n, head, hlist) {
			hlist_del_rcu(&dev->hlist);
			hlist_add_head_rcu(&dev->hlist, &clear_list);
		}
	}
	spin_unlock_bh(&dev_lock);

	hlist_for_each_entry_safe(dev, n, &clear_list, hlist) {
		hlist_del_rcu(&dev->hlist);
		synchronize_rcu();
		free_map_net_device(&dev->rcu);
	}
	return 0;
}

int update_map_net_device(struct local_itfs *dev_cfg)
{
	int i = 0;
	struct map_net_device *dev = NULL, *new_dev = NULL;
	struct net_device *net_dev = NULL;
	struct hlist_head *head = NULL;

	spin_lock_bh(&dev_lock);
	for (i = 0; i < dev_cfg->num; i++) {
		net_dev = mt_dev_get_by_name(dev_cfg->inf[i].name);
		if (!net_dev || (net_dev && !net_dev->dev_addr))
			continue;

		dev = get_map_net_dev_by_mac(net_dev->dev_addr);

		new_dev = create_map_net_device(&dev_cfg->inf[i], net_dev, NULL, INF_UNKNOWN);

		if (dev) {
			hlist_replace_rcu(&dev->hlist, &new_dev->hlist);
			call_rcu(&dev->rcu, free_map_net_device);
		} else {
			head = &dev_hash[MAC_ADDR_HASH_INDEX(new_dev->inf.mac)];
			hlist_add_head_rcu(&new_dev->hlist, head);
		}
	}
	spin_unlock_bh(&dev_lock);

	return 0;
}

int update_map_wan_tag_flag(unsigned char *mac, unsigned char status)
{
	struct map_net_device *dev = NULL;

	spin_lock_bh(&dev_lock);
	dev = get_map_net_dev_by_mac(mac);
	if (dev) {
		dev->wan_tag = status;
		pr_info("set wan(%02X:%02X:%02X:%02X:%02X:%02X) to %s vlan tag check successfully\n",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], (status == 1) ? "enable" : "disable");
	} else {
		pr_info("fails to find wan dev by mac(%02X:%02X:%02X:%02X:%02X:%02X)\n",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	spin_unlock_bh(&dev_lock);

	return 0;
}

int set_primary_map_net_device(struct local_interface *inf, unsigned char primary)
{
	struct hlist_head *head = NULL;
	struct map_net_device *dev = NULL, *dev_exist = NULL, *dev_new = NULL;
	int i = 0;


	spin_lock_bh(&dev_lock);

	dev = get_map_net_dev_by_mac(inf->mac);

	if (!dev) {
		pr_info("fails to set primary interface(%s)\n", inf->name);
		goto fail;
	}

	if (dev->inf.dev_type == AP || dev->inf.dev_type == ETH) {
		pr_info("cannot assign AP/ETH as (non)primary interface(%s)\n", inf->name);
		goto fail;
	}

	dev_new = create_map_net_device(&dev->inf, dev->dev, dev->dest_dev, primary);
	if (!dev_new)
		goto fail;

	hlist_replace_rcu(&dev->hlist, &dev_new->hlist);
	call_rcu(&dev->rcu, free_map_net_device);

	if (primary == INF_PRIMARY) {
	/*set other APCLI interfaces to non-primary interface*/
		for (i = 0; i < HASH_TABLE_SIZE; i++) {
			head = &dev_hash[i];

			hlist_for_each_entry_rcu(dev_exist, head, hlist) {
				if (dev_exist->inf.dev_type == APCLI &&
					memcmp(dev_new->inf.mac, dev_exist->inf.mac, ETH_ALEN))
					dev_exist->primary_interface = INF_NONPRIMARY;
			}
		}
	}

	spin_unlock_bh(&dev_lock);
	return 0;
fail:
	spin_unlock_bh(&dev_lock);
	return -1;
}

int set_uplink_path(struct local_interface *in, struct local_interface *out)
{
	struct map_net_device *dev_in = NULL, *dev_out = NULL;

	rcu_read_lock();
	dev_in = get_map_net_dev_by_mac(in->mac);

	if (!dev_in || ((dev_in) && dev_in->inf.dev_type == APCLI)) {
		goto fail;
	}
	if (out) {
		dev_out = get_map_net_dev_by_mac(out->mac);

		if (!dev_out || ((dev_out) && dev_out->inf.dev_type == AP)) {
			goto fail;
		}

		if (dev_in == dev_out) {
			goto fail;
		}

		dev_in->dest_dev = dev_out->dev;
	} else
		dev_in->dest_dev = NULL;

	rcu_read_unlock();
	return 0;
fail:
	rcu_read_unlock();
	return -1;
}

void show_mapfilter_version(void)
{
	pr_info("Current mapfilter version %s\n", VERSION);
}

void send_broadcast_nlmsg(unsigned char *msg, int msg_len)
{
	struct sk_buff *skb_out;
	struct nlmsghdr *nlh;
	int res;

	skb_out = nlmsg_new(msg_len, GFP_ATOMIC);

	if (!skb_out) {
		pr_info("Failed to allocate new skb\n");
		return;
	}
	nlh = nlmsg_put(skb_out, 0, 1, NLMSG_DONE, msg_len, 0);
	if (!nlh) {
		pr_info("error! nlh is NULL");
		nlmsg_free(skb_out);
		return;
	}
	//NETLINK_CB(skb_out).dst_group = 1; /* Multicast to group 1, 1<<0 */
	memcpy(nlmsg_data(nlh), msg, msg_len);

	res = nlmsg_multicast(nl_sk, skb_out, 0, 1, 0); /*group 1*/
	if (res < 0) {
		pr_info("Error while sending bak to user, err id: %d\n", res);
	}
	return;
}

int handle_map_command(struct map_netlink_message *cmd)
{
	int ret = 0;

	switch(cmd->type) {
		case UPDATE_MAP_NET_DEVICE:
			ret = update_map_net_device((struct local_itfs*)(cmd->event));
			break;
		case SET_PRIMARY_INTERFACE:
			{
				struct primary_itf_setting *psetting = (struct primary_itf_setting*)(cmd->event);
				ret = set_primary_map_net_device(&psetting->inf, psetting->primary);
			}
			break;
		case SET_UPLINK_PATH_ENTRY:
			{
				struct local_interface *in = NULL, *out = NULL;
				struct up_link_path_setting *psetting = (struct up_link_path_setting*)(cmd->event);

				in = &psetting->in;
				if (!is_zero_mac(psetting->out.mac))
					out = &psetting->out;

				pr_info("SET_UPLINK_PATH_ENTRY, in(" MAC2STR ")", PRINT_MAC(in->mac));
				if (out)
					pr_info("-->out(" MAC2STR ")\n", PRINT_MAC(out->mac));

				ret = set_uplink_path(in, out);
			}
			break;
		case DUMP_DEBUG_INFO:
			break;
		case UPDATE_APCLI_LINK_STATUS:
			{
				struct apcli_link_status *pstatus = (struct apcli_link_status*)(cmd->event);

				update_apcli_link_status(pstatus);
			}
			break;
		case UPDATE_CHANNEL_UTILIZATION:
			{
				struct radio_utilization *util = (struct radio_utilization*)(cmd->event);
				struct map_net_device *dev = NULL;
				unsigned char found = 0;
				int i = 0;

				rcu_read_lock();
				for (i = 0; i < HASH_TABLE_SIZE; i++) {
					hlist_for_each_entry_rcu(dev, &dev_hash[i], hlist) {
						if (dev->inf.dev_type == APCLI && dev->inf.band == util->band) {
							found = 1;
							break;
						}
					}

					if (found)
						break;
				}
				if (!dev) {
					pr_info("%s error apcli interface not found for band(%d)\n",
						__func__, util->band);
					rcu_read_unlock();
					break;
				}
				set_radio_channel_utilization(util, dev->dev);
				rcu_read_unlock();
			}
			break;
		case DYNAMIC_LOAD_BALANCE:
			{
				enable_load_balance(cmd->event[0]);
			}
			break;
		case SET_TRAFFIC_SEPARATION_DEFAULT_8021Q:
			{
				struct ts_default_8021q *default_8021q = (struct ts_default_8021q*)(cmd->event);

				handle_ts_default_8021q(default_8021q->primary_vid, default_8021q->default_pcp);
			}
			break;
		case SET_TRAFFIC_SEPARATION_POLICY:
			{
				struct ts_policy *policy = (struct ts_policy *)(cmd->event);

				handle_ts_policy(policy);
			}
			break;
		case SET_DROP_SPECIFIC_IP_PACKETS_STATUS:
			{
				struct drop_specific_dest_ip_status *set_drop_ip_status = (struct drop_specific_dest_ip_status *)(cmd->event);

				drop_ip_status.drop_flag = set_drop_ip_status->drop_flag;
				if (set_drop_ip_status->drop_flag == 1) {
					mod_timer(&dp_timer, jiffies + DROP_TIMEOUT);
					pr_info("mapfilter: Start drop the specific ip packets now !!\n ");
				} else {
					pr_info("mapfilter: Stop drop the specific ip packets now !!\n ");
					del_timer_sync(&dp_timer);
				}
			}
			break;
		case SET_TRANSPARENT_VID:
			{
				struct transparent_vids *trans_vids = (struct transparent_vids*)(cmd->event);

				handle_transparent_vlan(trans_vids);
			}
			break;
		case UPDATE_CLIENT_VID:
			{
				struct client_vid *client = (struct client_vid*)(cmd->event);

				handle_client_vid(client);
			}
			break;
		case TRAFFIC_SEPARATION_ONOFF:
			{
				enable_ts(cmd->event[0]);
			}
			break;
		case WAN_TAG_FLAG_ON_DEV:
			ret = update_map_wan_tag_flag(cmd->event, cmd->event[6]);
			break;
		case SHOW_VERSION:
			show_mapfilter_version();
			break;
		case SERVICE_PRIORITIZATION_RULE:
			sp_add_rule((struct sp_rules_config *)cmd->event);
			break;
		case SERVICE_PRIORITIZATION_DSCP_MAPPING_TBL:
			{
				struct sp_dscp_mapping_tbl_config *tbl = NULL;

				tbl = (struct sp_dscp_mapping_tbl_config *)cmd->event;
				sp_set_dscp_mapping_table(tbl->dptbl);
			}
			break;
		case SERVICE_PRIORITIZATION_CLEAR_RULE:
			{
				sp_clear_rule();
				sp_learn_hash_clear();
			}
			break;
		case SERVICE_PRIORITIZATION_REMOVE_ONE_RULE:
			{
				struct sp_rule *r = NULL;

				r = (struct sp_rule*)cmd->event;

				sp_rm_one_rule(r);
			}
			break;
		default:
			pr_info("unknown map command type(%02x)\n", cmd->type);
			break;
	}

	return ret;
}

void recv_nlmsg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = nlmsg_hdr(skb);
	struct map_netlink_message *msg = NULL;

	if (nlh->nlmsg_len < NLMSG_HDRLEN || skb->len < nlh->nlmsg_len)
		return;

	msg = (struct map_netlink_message *)NLMSG_DATA(nlh);
	handle_map_command(msg);
}
#if (!(LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0)))
struct netlink_kernel_cfg nl_kernel_cfg = {
	.groups = 0,
	.flags = 0,
	.input = recv_nlmsg,
	.cb_mutex = NULL,
	.bind = NULL,
};
#endif

static struct nf_hook_ops map_basic_ops[] = {
	{
		.hook		= map_basic_hook_fn_pre_rout,
		.pf		= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_PRE_ROUTING,
		.priority	= NF_BR_PRI_BRNF,
		//.owner		= THIS_MODULE,
	},
	{
		.hook		= map_basic_hook_fn_forward,
		.pf		= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_FORWARD,
		.priority	= NF_BR_PRI_BRNF,
		//.owner		= THIS_MODULE,
	},
	{
		.hook		= map_auto_role_select_hook_fn_pre_rout,
		.pf		= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_PRE_ROUTING,
		.priority	= NF_BR_PRI_BRNF,
		//.owner		= THIS_MODULE,
	}
};

#define MAP_SO_BASE 1905
#define MAP_GET_MAC_BY_SRC MAP_SO_BASE
#define MAP_SET_ALMAC MAP_SO_BASE
#define MAP_SO_MAX (MAP_SO_BASE + 1)

static int do_map_set_ctl(struct sock *sk, int cmd,
		void __user *user, unsigned int len)
{
	int ret = 0;

	pr_info("do_map_set_ctl==>cmd(%d)\n", cmd);
	switch (cmd) {
	case MAP_SET_ALMAC:
		if (copy_from_user(almac, user, ETH_ALEN) != 0) {
			ret = -EFAULT;
			pr_info("do_map_set_ctl==>copy_from_user fail\n");
			break;
		}
		pr_info("do_map_set_ctl==>almac(" MAC2STR ")\n", PRINT_MAC(almac));

		sp_update_own_almac(almac);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int do_map_get_ctl(struct sock *sk, int cmd, void __user *user, int *len)
{
	int ret = 0;

	switch (cmd) {
	case MAP_GET_MAC_BY_SRC:
	{
		char addr[ETH_ALEN] = {0};

		if (*len < ETH_ALEN) {
			ret = -EINVAL;
			break;
		}

		if (copy_from_user(addr, user, ETH_ALEN) != 0) {
			ret = EFAULT;
			break;
		}

		nt_get_intf(addr);
		if (copy_to_user(user, addr, ETH_ALEN) != 0)
			ret = -EFAULT;
		break;
	}
	default:
		ret = -EINVAL;
	}

	return ret;
}
static struct nf_sockopt_ops map_sockopts = {
	.pf		= PF_INET,
	.set_optmin	= MAP_SO_BASE,
	.set_optmax	= MAP_SO_MAX,
	.set		= do_map_set_ctl,
#ifdef CONFIG_COMPAT
	.compat_set	= NULL,
#endif
	.get_optmin	= MAP_SO_BASE,
	.get_optmax	= MAP_SO_MAX,
	.get		= do_map_get_ctl,
#ifdef CONFIG_COMPAT
	.compat_get	= NULL,
#endif
	.owner		= THIS_MODULE,
};

static ssize_t map_nt_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int i, len = 0, total_len = 0, buf_len = PAGE_SIZE;
	struct map_nt_entry *pos;

	spin_lock_bh(&nt_lock);
	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		if (list_empty(&nt[i]))
			continue;
		list_for_each_entry(pos, &nt[i], list) {
			len = snprintf(buf + total_len, buf_len, 
				"idx: %d\t" MAC2STR " is at %s\n", i, PRINT_MAC(pos->mac),
				pos->intf);

			if (len < 0)
				goto end;

			if (len >= buf_len) {
				total_len += buf_len - 1;
				goto end;
			}

			total_len += len;
			buf_len -= total_len;

			if (buf_len <= 1)
				goto end;
		}
	}	
end:
	spin_unlock_bh(&nt_lock);
	return total_len;
}

static ssize_t map_nt_cnt_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", nt_cnt);
}

static ssize_t map_device_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int len = 0, total_len = 0;
	struct map_net_device *dev = NULL;
	struct hlist_head *head = NULL;
	int i = 0, count = 0;

	len = snprintf(buf + total_len, PAGE_SIZE - total_len,
		"[dump map device info]\n");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;
	
	len = snprintf(buf + total_len, PAGE_SIZE - total_len,
		"NAME\tMAC\tTYPE\tBAND\tWAN_TAG\n");
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		head = &dev_hash[i];
		hlist_for_each_entry_rcu(dev, head, hlist) {
			len = snprintf(buf + total_len, PAGE_SIZE - total_len,
				"\t[%d]\n\t\t%s\t" MAC2STR "\t%s\t%02x\t%d", count,
				dev->inf.name, PRINT_MAC(dev->inf.mac),
				dev_type2str(dev->inf.dev_type), dev->inf.band, dev->wan_tag);
			if (snprintf_error(PAGE_SIZE - total_len, len))
				goto end;

			total_len += len;
			
			if (dev->dev && dev->dest_dev) {
				len = snprintf(buf + total_len, PAGE_SIZE - total_len,
					"\t[uplink path] %s--->%s", dev->dev->name, dev->dest_dev->name);
				if (snprintf_error(PAGE_SIZE - total_len, len))
					goto end;

				total_len += len;
			}

			len = snprintf(buf + total_len, PAGE_SIZE - total_len,
				"\t[multicast primary] %s\n", primary_type2str(dev->primary_interface));
			if (snprintf_error(PAGE_SIZE - total_len, len))
				goto end;

			total_len += len;
			count++;
		}
	}
end:
	return total_len;
}

static ssize_t mapfilter_version_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int len = 0, total_len = 0;

	len = snprintf(buf + total_len, PAGE_SIZE - total_len,
		"mapfilter version %s\n", VERSION);
	if (snprintf_error(PAGE_SIZE - total_len, len))
		goto end;

	total_len += len;
end:
	return total_len;
}

static struct kobj_attribute map_sysfs_nt_show =
		__ATTR(nt_show, S_IRUGO, map_nt_show, NULL);
static struct kobj_attribute map_sysfs_nt_cnt_show =
		__ATTR(nt_cnt_show, S_IRUGO, map_nt_cnt_show, NULL);
static struct kobj_attribute map_sysfs_device_show =
		__ATTR(map_devices_show, S_IRUGO, map_device_show, NULL);
static struct kobj_attribute map_sysfs_version =
		__ATTR(version, S_IRUGO, mapfilter_version_show, NULL);

static struct attribute *map_sysfs[] = {
	&map_sysfs_nt_show.attr,
	&map_sysfs_nt_cnt_show.attr,
	&map_sysfs_device_show.attr,
	&map_sysfs_version.attr,
	NULL,
};
static struct attribute_group map_attr_group = {
	.attrs = map_sysfs,
};
struct kobject *map_kobj;

static int __init map_init(void)
{
	int ret, i;

	pr_info("Current mapfilter version %s\n", VERSION);

	spin_lock_init(&nt_lock);
	spin_lock_init(&dev_lock);

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		INIT_LIST_HEAD(&nt[i]);
		INIT_HLIST_HEAD(&dev_hash[i]);
	}

	drop_ip_status.drop_flag = 1;
	drop_ip_addr.addr1 = SPECIFIC_IP_ADDR1;
	drop_ip_addr.addr2 = SPECIFIC_IP_ADDR2;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22))
	nl_sk = netlink_kernel_create(MAP_NETLINK, 0, recv_nlmsg, THIS_MODULE);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
	nl_sk = netlink_kernel_create(MAP_NETLINK, 0, recv_nlmsg, NULL, THIS_MODULE);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
	nl_sk = netlink_kernel_create(&init_net, MAP_NETLINK, 0, recv_nlmsg, NULL, THIS_MODULE);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0))
	nl_sk = netlink_kernel_create(&init_net, MAP_NETLINK, THIS_MODULE, &nl_kernel_cfg);
#else
	nl_sk = netlink_kernel_create(&init_net, MAP_NETLINK, &nl_kernel_cfg);
#endif
	if (!nl_sk) {
		pr_info("create netlink socket error.\n");
		ret = -EFAULT;
		goto error2;
	}

	ret = nf_register_sockopt(&map_sockopts);
	if (ret < 0)
		goto error3;

	map_kobj = kobject_create_and_add("mapfilter", NULL);
	if (!map_kobj) {
		ret = -EFAULT;
		goto error4;
	}

	ret = sysfs_create_group(map_kobj, &map_attr_group);
	if (ret)
		goto error5;

	service_prioritization_init(map_kobj);
	ts_init(map_kobj);
	load_balance_init(map_kobj);
	dual_bh_init();

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	ret = nf_register_hooks(&map_basic_ops[0], ARRAY_SIZE(map_basic_ops));
#else
	ret = nf_register_net_hooks(&init_net, &map_basic_ops[0], ARRAY_SIZE(map_basic_ops));
#endif
	if (ret < 0) {
		pr_info("register nf hook fail, ret = %d\n", ret);
	}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,14,0))
	timer_setup(&nt_timer, nt_timeout_func, 0);
	timer_setup(&dp_timer, dp_timeout_func, 0);
#else
	init_timer(&nt_timer);
	nt_timer.function = nt_timeout_func;
	init_timer(&dp_timer);
	dp_timer.function = dp_timeout_func;
#endif
	nt_timer.expires = jiffies + TIMER_EXCUTE_PERIOD;
	add_timer(&nt_timer);
	dp_timer.expires = jiffies + DROP_TIMEOUT;
	add_timer(&dp_timer);

	return ret;
error5:
	kobject_put(map_kobj);
error4:
	nf_unregister_sockopt(&map_sockopts);
error3:
	sock_release(nl_sk->sk_socket);
error2:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	nf_unregister_hooks(&map_basic_ops[0], ARRAY_SIZE(map_basic_ops));
#else
	nf_unregister_net_hooks(&init_net, &map_basic_ops[0], ARRAY_SIZE(map_basic_ops));
#endif
	return ret;
}

static void __exit map_exit(void)
{
	int i;
	struct map_nt_entry *pos, *n;

	del_timer_sync(&nt_timer);
	del_timer_sync(&dp_timer);
	service_prioritization_deinit();
	ts_deinit();
	load_balance_deinit();
	dual_bh_deinit();

	sysfs_remove_group(map_kobj, &map_attr_group);
	kobject_put(map_kobj);

	nf_unregister_sockopt(&map_sockopts);

	if (nl_sk != NULL)
		sock_release(nl_sk->sk_socket);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	nf_unregister_hooks(&map_basic_ops[0], ARRAY_SIZE(map_basic_ops));
#else
	nf_unregister_net_hooks(&init_net, &map_basic_ops[0], ARRAY_SIZE(map_basic_ops));
#endif

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		list_for_each_entry_safe(pos, n, &nt[i], list) {
			pr_info("exit! free " MAC2STR "\n", PRINT_MAC(pos->mac));
			list_del(&pos->list);
			nt_cnt--;
			kfree(pos);
		}
	}

	clear_map_net_device();

	pr_info("module exit\n");
	return;
}

module_init(map_init);
module_exit(map_exit);
