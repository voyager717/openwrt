#ifndef _MTKMAPFILTER_H_
#define _MTKMAPFILTER_H_
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

#define HASH_TABLE_SIZE 256
#define MAC_ADDR_HASH(addr) (addr[0]^addr[1]^addr[2]^addr[3]^addr[4]^addr[5])
#define MAC_ADDR_HASH_INDEX(addr) (MAC_ADDR_HASH(addr) & (HASH_TABLE_SIZE - 1))

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

enum DEVICE_TYPE {
	AP = 0,
	APCLI,
	ETH,
};

#define INF_UNKNOWN		0x00
#define INF_PRIMARY		0x01
#define INF_NONPRIMARY	0x02

#define _24G 0x01
#define _5GL 0x02
#define _5GH 0x04
#define _5G	0x06

#define DIR_UNKNOWN		0x00
#define DIR_UPSTREAM 	0x01
#define DIR_DOWNSTREAM	0x02

enum MAP_NETLINK_EVENT_TYPE {
	UPDATE_MAP_NET_DEVICE = 0,
	SET_PRIMARY_INTERFACE,
	SET_UPLINK_PATH_ENTRY,
	DUMP_DEBUG_INFO,
	UPDATE_APCLI_LINK_STATUS,
	UPDATE_CHANNEL_UTILIZATION,
	DYNAMIC_LOAD_BALANCE,
	SET_DROP_SPECIFIC_IP_PACKETS_STATUS,
	SET_TRAFFIC_SEPARATION_DEFAULT_8021Q,
	SET_TRAFFIC_SEPARATION_POLICY,
	SET_TRANSPARENT_VID,
	UPDATE_CLIENT_VID,
	TRAFFIC_SEPARATION_ONOFF,
	WAN_TAG_FLAG_ON_DEV,
	SHOW_VERSION,	
	SERVICE_PRIORITIZATION_RULE,
	SERVICE_PRIORITIZATION_DSCP_MAPPING_TBL,
	SERVICE_PRIORITIZATION_CLEAR_RULE,
	SERVICE_PRIORITIZATION_REMOVE_ONE_RULE
};

#define IP_PROTOCOL_ICMP 0x01
#define PING_SIZE 116 /*ping -s88 8.8.8.8, PING_SIZE = ping size + headerlen(28)*/

struct GNU_PACKED map_netlink_message {
	unsigned char type;
	unsigned short len;
	unsigned char event[0];
};

struct GNU_PACKED local_interface {
	char name[IFNAMSIZ];
	unsigned char mac[ETH_ALEN];
	enum DEVICE_TYPE dev_type;
	unsigned char band;
};
struct  map_net_device {
	struct hlist_node hlist;
	struct rcu_head rcu;
	struct local_interface inf;
	struct net_device *dev;
	struct net_device *dest_dev;
	unsigned char primary_interface;
	unsigned char wan_tag;
};

struct GNU_PACKED local_itfs {
	unsigned char num;
	struct local_interface inf[0];
};

struct GNU_PACKED primary_itf_setting {
	struct local_interface inf;
	unsigned char primary;
};

struct GNU_PACKED up_link_path_setting {
	struct local_interface in;
	struct local_interface out;
};

struct GNU_PACKED apcli_link_status {
	struct local_interface in;
	unsigned char link_status;
};

struct GNU_PACKED drop_specific_dest_ip_status {
	unsigned char drop_flag;
};

/* reserved: user can change the drop_specific_dest_ip_addr */
struct GNU_PACKED drop_specific_dest_ip_addr {
	unsigned int addr1;
	unsigned int addr2;
};
void send_broadcast_nlmsg(unsigned char *msg, int msg_len);
struct map_net_device *get_map_net_dev_by_mac(unsigned char *mac);
struct map_net_device *create_map_net_device(struct local_interface *inf,
	struct net_device *dev, struct net_device *dest_dev, unsigned char primary);
void free_map_net_device(struct rcu_head *head);
#endif
