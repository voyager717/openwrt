#ifndef _LOAD_BALANCE_H_
#define _LOAD_BALANCE_H_
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

#include "mtkmapfilter.h"
#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

#define SESSION_HASH_TABLE_SIZE 512

struct radio_context {
	struct hlist_head radio_list;
	spinlock_t hash_lock;
};

struct wireless_radio {
	struct hlist_node hlist;
	struct rcu_head rcu;
	unsigned char radio_id[ETH_ALEN];
	unsigned char band;
	unsigned char util;
	unsigned char up_link_status;
	struct net_device *up_dev;
};

struct session_context {
	struct hlist_head session_hash;
	spinlock_t hash_lock;
};

struct session {
	struct hlist_node hlist;
	struct rcu_head rcu;
	struct net_device *dest_dev;
	unsigned char src_mac[ETH_ALEN];
	unsigned char band;
	u32	ip_saddr;	/*upstream direction*/
	u32	ip_daddr;
	u16	tcp_source;
	u16	tcp_dest;
	unsigned long updated;
	unsigned int len;
	unsigned int total_len;
	unsigned int speed;
};

struct GNU_PACKED radio_utilization {
	unsigned char radio_id[ETH_ALEN];
	unsigned char band;
	unsigned char util;
};

void ap_load_balance_process(struct sk_buff *skb);
void apcli_load_balance_process(struct sk_buff *skb);
void ap_load_balance_learning(struct sk_buff *skb, struct net_device *in);
void apcli_load_balance_learning(struct sk_buff *skb);
int set_radio_channel_utilization(struct radio_utilization *util, struct net_device *up_dev);
void load_balance_init(struct kobject *parent_obj);
void load_balance_deinit(void);
void radio_link_change_update_apcli_session(unsigned char band, unsigned char link_status);
void update_apcli_link_status(struct apcli_link_status *status);
void enable_load_balance(unsigned char enable);
#endif
