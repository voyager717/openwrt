#ifndef _SERVICE_PRIORITIZATION_H_
#define _SERVICE_PRIORITIZATION_H_
/*
 * service_prioritization.h, it's used to support MAP R3 Service Prioritization
 *
 * Author: Zheng Zhou <Zheng.Zhou@mediatek.com>
 */
#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

enum {
	RULE_INDEX_IDX = 1,
	RULE_NAME_IDX,
	RULE_AL_MAC_IDX,
	RULE_SSID_IDX,
	RULE_MAC_IDX,
	RULE_TCPIP_PRO_IDX,
	RULE_TCPIP_DST_IP_IDX,
	RULE_TCPIP_SRC_IP_IDX,
	RULE_TCPIP_DST_PORT_IDX,
	RULE_TCPIP_SRC_PORT_IDX,
	RULE_OUTPUT
};

#define STANDARD_SP_RULE(r) (!((r)->mask ^ (RULE_AL_MAC)))
#define SP_RULE_ALWAYS_MATCH BIT(7)

enum {
	RULE_TYPE_STATIC = 0,
	RULE_TYPE_DYNAMIC
};

#define IP_V4 0x04
#define IP_V6 0x06

#define RULE_AL_MAC BIT(0)
#define RULE_SSID	BIT(1)
#define RULE_MAC	BIT(2)
#define RULE_TCPIP_PRO	BIT(3)
#define RULE_TCPIP_DST_IP	BIT(4)
#define RULE_TCPIP_SRC_IP	BIT(5)
#define RULE_TCPIP_DST_PORT	BIT(6)
#define RULE_TCPIP_SRC_PORT	BIT(7)

enum rule_action {
	REMOVE_RULE = 0,
	ADD_RULE	
};

struct ip_proto {
	char proto_name[16];
	unsigned char value;
};

struct GNU_PACKED tcpip_5_tuple {
	unsigned char protocol;
	union {
		unsigned int v4_addr;
		unsigned char v6_addr[16];
	} src_addr;
	union {
		unsigned int v4_addr;
		unsigned char v6_addr[16];
	} dst_addr;
	unsigned short dst_port;
	unsigned short src_port;
	unsigned char ip_ver;
};

#define RULE_NAME_SIZE 32
#define SSID_MAX_LEN 32

struct GNU_PACKED sp_rule {
	unsigned char index;
	unsigned char name_len;
	char rule_name[RULE_NAME_SIZE + 1];
	unsigned short mask;
	unsigned char rule_type;
	unsigned char al_mac[ETH_ALEN];
	unsigned char ssid_len;
	char ssid[SSID_MAX_LEN + 1];
	unsigned char client_mac[ETH_ALEN];
	struct tcpip_5_tuple tcpip;
	unsigned char rule_output;
	unsigned char rule_identifier[4]; /*indentified in standard rule tlv*/
};


struct GNU_PACKED sp_rules_config {
	unsigned char number;
	struct sp_rule rules[0];
};

struct GNU_PACKED sp_dscp_mapping_tbl_config {
	unsigned char dptbl[64];
};

struct sp_rule_list {
	struct hlist_head head;
	spinlock_t lock;
};

struct sp_rule_element {
	struct hlist_node entry;	
	struct rcu_head rcu;
	struct sp_rule rule;
	unsigned long match_count;
};

struct ip_5_tuple {
	unsigned char protocol;
	union {
		unsigned int v4_addr;
		unsigned char v6_addr[16];
	} src_addr;
	union {
		unsigned int v4_addr;
		unsigned char v6_addr[16];
	} dst_addr;
	unsigned short dst_port;
	unsigned short src_port;
	unsigned char ip_ver;
};

struct tcpip_session_element {
	struct hlist_node entry;
	struct rcu_head rcu;
	struct ip_5_tuple tuple;
	unsigned char pcp;
	unsigned long match_count;
	unsigned char correspond_rule_idx;
	unsigned long age;
};

struct tcpip_hash_head {
	struct hlist_head head;
	spinlock_t hash_lock;
};

struct GNU_PACKED sp_learn_complete_event
{
	unsigned char up;
	unsigned char ip_ver;
	unsigned char mask;
	union {
		unsigned int v4_addr;
		unsigned char v6_addr[16];
	} src_addr;
	union {
		unsigned int v4_addr;
		unsigned char v6_addr[16];
	} dst_addr;
	unsigned short pkt_src_port;
	unsigned short pkt_dst_port;
};

struct learn_comp_element {
	struct hlist_node entry;
	struct rcu_head rcu;
	struct sp_learn_complete_event learn_event;
	unsigned long age;
	unsigned char report;
	unsigned int learn_count;
};

struct learn_complete_hash_head {
	struct hlist_head head;
	spinlock_t hash_lock;
};

#define SP_TCPIP_HASH_SIZE 1238
#define SP_LEARN_HASH_SIZE 1238

struct sp_conf {
	unsigned char enable;
	unsigned char dynamic_rule_enable;
	unsigned char dynamic_prio;
	unsigned char quick_match;
};

struct sp_context {
	struct sp_conf conf;
	struct sp_rule_list static_list;
	struct sp_rule_list dynamic_list;
	struct sp_dscp_mapping_tbl_config dptbl;
	struct tcpip_hash_head hash[SP_TCPIP_HASH_SIZE];
	struct learn_complete_hash_head learn_hash[SP_LEARN_HASH_SIZE];
	struct timer_list sp_period;
	unsigned char own_al_mac[ETH_ALEN];
	struct kobject *sp_obj;
};

enum peer_type {
	NORMAL_STA = 0,
	MAP_R1,
	MAP_R2,
	MAP_R3
};

#define SKB_CB_OFFSET 10

int service_prioritization_init(struct kobject *parent_obj);
int service_prioritization_deinit(void);
int sp_add_rule(struct sp_rules_config *rule_config);
int sp_clear_rule(void);
int sp_rm_one_rule(struct sp_rule *rule);
int sp_set_dscp_mapping_table( unsigned char *tbl);
int sp_learn_hash_clear(void);
int sp_update_own_almac(unsigned char *almac);
#endif
