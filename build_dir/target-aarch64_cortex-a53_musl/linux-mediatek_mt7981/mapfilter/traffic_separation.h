#ifndef _TRAFFIC_SEPARATION_H_
#define _TRAFFIC_SEPARATION_H_
#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

#define STATION_JOIN 1
#define STATION_LEAVE 0

struct GNU_PACKED ssid_2_vid_mapping {
	char ssid[32];
	unsigned short vlan_id;
};

struct GNU_PACKED ts_policy {
	unsigned char num;
	struct ssid_2_vid_mapping ssid_2_vid[0];
};

struct GNU_PACKED ts_default_8021q {
	unsigned short primary_vid;
	unsigned char default_pcp;
};

struct GNU_PACKED transparent_vids {
	unsigned char num;
	unsigned short vids[0];
};

struct GNU_PACKED client_vid {
	unsigned char client_mac[ETH_ALEN];
	unsigned short vid;
	unsigned char status;
	unsigned char ssid_len;
	char ssid[33];
	unsigned char al_mac[ETH_ALEN];
};

struct client_db {
	struct hlist_node hlist;
	struct rcu_head rcu;
	unsigned char mac[ETH_ALEN];
	unsigned short vid;
	unsigned char ssid_len;
	char ssid[33];
	unsigned char al_mac[ETH_ALEN];
};

void ts_init(struct kobject *parent_obj);
void ts_deinit(void);
unsigned char handle_ts_default_8021q(unsigned short primary_vid, unsigned char default_pcp);
unsigned char handle_ts_policy(struct ts_policy *policy);
unsigned char handle_transparent_vlan(struct transparent_vids *tvids);
unsigned char handle_client_vid(struct client_vid *client);
void dump_ts_info(void);
struct client_db *get_client_db(unsigned char mac[]);
void enable_ts(unsigned char enable);
#endif
