/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef P1905_MANAGERD_H
#define P1905_MANAGERD_H

#include <linux/if_packet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <sys/un.h>
#include "p1905_database.h"
//#include "../../linux/gpio.h"
#ifdef SUPPORT_WIFI
#include "wifi_utils.h"
#include "wsc_attr_tlv.h"
#include "p1905_ap_autoconfig.h"
#endif
#include "_1905_interface.h"
#include "_1905_lib_internal.h"
#include "os.h"
#include "stack.h"
#include "ethernet_layer.h"
#include "mapfilter_if.h"
#include "list.h"

#define VERSION_1905 "v3.0.1.2"
#define CONTROLLER 0
#define AGENT 1
#define UNKNOWN 0xff

#define WAPP_SERVER_NAME    "wapp_server"

/* use this compiler option to enable update bridge fdb for AL mac learning
 * set 1: enable
 * set 0: disbale
 */
#define UPDATE_BRIDGE_FDB_ENABLE 0//1

/** Define packet max length */
#define MAX_PKT_LEN     20480
#define PKT_LEN_100K 102400

/*define time_to_live of topology discovery database*/
#define TOPOLOGY_DISCOVERY_TTL 300 //70 seconds
#define TOPOLOGY_DEVICE_TTL 300 //2min
#define SEARCH_TIME_TTL 20 //60 seconds


/*for vendor specific*/
#define OUI_SPIDCOM     "\x00\x0c\x43"
#define OUI_LEN 3

/* for push button join notify, we must record the current stations
 * information to compare. although in PLC there can be 256 stations, but
 * we use 32 to reduce memory usage.
 */
#define MAX_STATIONS 32

#define P1905_CONF_PATH "/etc/p1905.conf"


#ifndef MAC2STR
#ifdef CONFIG_MASK_MACADDR
#define MAC2STR(a) (a)[0], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:**:**:%02x:%02x:%02x"
#define PRINT_MAC(a) a[0],a[3],a[4],a[5]
#else
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define PRINT_MAC(a) a[0],a[1],a[2],a[3],a[4],a[5]
#endif
#endif



/* HPAV station information, we use this structure to maintain
 * the information gotton from MME(CC_DISCOVER_LIST_CNF)
 */

#define MAX_SET_BSS_INFO_NUM 58
#define MAX_RADIO_NUM 3

/*Interface Media Type(spec 6.4.7)*/
#define IEEE802_3_GROUP   		0x0000
#define IEEE802_11_GROUP 		0x0100
#define IEEE802_11B_24G_GROUP  	0x0100
#define IEEE802_11G_24G_GROUP  	0x0101
#define IEEE802_11A_5G_GROUP  	0x0102
#define IEEE802_11N_24G_GROUP  	0x0103
#define IEEE802_11N_5G_GROUP  	0x0104
#define IEEE802_11AC_5G_GROUP  	0x0105
#define IEEE802_11AX_GROUP  	0x0108
#define IEEE1901_GROUP    		0x0200
#define MOCA_GROUP        		0x0300

#define HASH_TABLE_SIZE 256
#define MAC_ADDR_HASH(addr) (addr[0]^addr[1]^addr[2]^addr[3]^addr[4]^addr[5])
#define MAC_ADDR_HASH_INDEX(addr) (MAC_ADDR_HASH(addr) & (HASH_TABLE_SIZE - 1))

typedef struct
{
    unsigned char station_num;
    unsigned char station_mac_list[MAX_STATIONS][ETH_ALEN];
    unsigned char new_station[ETH_ALEN];
} hpav_info;

#ifdef SUPPORT_WIFI
typedef struct
{
    unsigned char no_need_start_pbc;
    unsigned char station_num;
    unsigned char station_mac_list[WIFI_MAX_STATION_NUM][ETH_ALEN];
    unsigned char new_station[ETH_ALEN];

} iee802_11_info;
#endif

/* for 1905.1 push button join notification use. */
typedef struct
{
    /*store the al id from push button event notification*/
    unsigned char al_id[ETH_ALEN];
    /*strore the message id from push button event notification*/
    unsigned short mid;
    /*is_sc: 1 ==> simple connect on-going*/
    unsigned char is_sc;
    /*store the information of PLC*/
    hpav_info info;

#ifdef SUPPORT_WIFI
    /*store the information of WIFI*/
    iee802_11_info wifi_info;
#endif
} push_button_param;

struct auto_config_info {
	signed char radio_index;
	unsigned char config_number;
	WSC_CONFIG config_data[MAX_BSS_NUM];
} ;

struct set_config_bss_info{
	unsigned char mac[ETH_ALEN];
	char oper_class[4];
	/* #define BAND_2G_CAP 0x01
	 * #define BAND_5GL_CAP 0x02
	 * #define BAND_5GH_CAP 0x04
	 * #define BAND_6G_CAP  0x10
	 */
	char band_support;
	char ssid[MAX_SSID_LEN];
	unsigned short authmode;
	unsigned short encryptype;
	char key[65];
	unsigned char wfa_vendor_extension;
	unsigned char hidden_ssid;
	unsigned short pvid;
	unsigned char pcp;
	unsigned char ssid_len;
	unsigned short vid;
};

struct _config_buf_ctrl{
	unsigned char *itf_name;
	unsigned char *config_buff;
	int len;
	struct dl_list list;
};

struct contro_conn_port {
	unsigned char is_set;
	int port_num;
	unsigned char filter_almac[ETH_ALEN];
};

#ifdef MAP_R3
typedef enum
{
	no_r3_autoconfig = 0,
	wait_4_send_bss_autoconfig_req,
	wait_4_recv_bss_autoconfig_resp,
	wait_4_set_r3_config,
	r3_autoconfig_done,
} r3_enrolle_config_stage;


struct r3_config_data {
	unsigned char radio_identifier[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	WSC_CONFIG config_data;
};


enum r3_onboarding_stage {
	R3_ONBOARDING_STAGE_INVALID = 0,
	R3_ONBOARDING_STAGE_PEER_DISCOVERY = 1,
	R3_ONBOARDING_STAGE_4_WAY_HS = 2,
	R3_ONBOARDING_STAGE_BSS_CONFIG = 3,
	R3_ONBOARDING_STAGE_DONE = 4,
};

struct r3_onboarding_info {
	unsigned char active;
	unsigned char peer_profile;
	unsigned char peer_service_type;
	unsigned char bh_type;
	unsigned char peer_almac[ETH_ALEN];
	r3_enrolle_config_stage r3_config_state;
	unsigned char total_config_num;
	unsigned char bss_req_retry_cnt;
	unsigned char bss_renew;
	unsigned char onboarding_stage;
	struct r3_config_data bss_config_data[MAX_SET_BSS_INFO_NUM];
};

struct r3_reconfig_info {
	unsigned char reconfig_state;
	unsigned char reconfig_cnt;
};


struct hash_value_item {
    unsigned short hashLen;
    unsigned char hashValue[32];
    unsigned char almac[ETH_ALEN];
    unsigned char mac_apcli[ETH_ALEN];
    struct dl_list member;
};

struct bss_connector_item {
	unsigned char mac_apcli[ETH_ALEN];
	unsigned char almac[ETH_ALEN];
	unsigned short bh_connector_len;
	unsigned short fh_connector_len;
	char *bh_connector;
	char *fh_connector;
	struct dl_list member;
};

struct r3_dpp_information {
	unsigned char initialized;
	/* onboarding type while wifi case,
	 * 0-dpp onboarding, other-PBC onboarding
	*/
	unsigned char onboarding_type;
	unsigned short connector_len;
	unsigned short netAccessKey_len;
	unsigned short CsignKey_len;
	unsigned int netAccessKey_expiry;
	/* hash value for each agent, first one is for self */
	struct dl_list hash_value_list;
	/* hash value for each agent */
	struct dl_list bss_connector_list;
	char *connector;
	unsigned char *netAccessKey;
	unsigned char *CsignKey;

	unsigned char pmkid[16];
	unsigned char pmk[64];
	unsigned int pmk_len;
};

/* packets store related structure */

struct frame_list_struct{
	unsigned int mid;
	unsigned short len;
	unsigned char *data;
	struct dl_list data_list;
};

struct frame_buff_struct{
    unsigned short msg_type;
    unsigned short msg_cnt;
    /* msg data list for the msg type */
    struct dl_list frame_data_list;
    /* msg type list */
    struct dl_list frame_info_list;
};

#endif // MAP_R3

struct delayed_message_buf {
	struct dl_list entry;
	unsigned char al_mac[ETH_ALEN];
	unsigned char wait_for_event;
	unsigned short mid;
	char if_name[IFNAMSIZ];
	struct os_time time;
};

struct delayed_message_context {
	struct dl_list buf_list_head;
};

struct renew_context {
	unsigned char trigger_renew;
	/*for agent use*/
	unsigned char is_renew_ongoing;
};

#define FIRST_VITUAL_ITF 0             /*for MAP agent and contriller concurrent*/
#define MAX_FILE_PATH_LENGTH 64


struct buf_info {
	unsigned char *buf;
	unsigned int buf_len;
};

/** Context structure */
struct p1905_managerd_ctx {
	unsigned char role;
	unsigned char br_name[IFNAMSIZ];
	unsigned char al_inf[IFNAMSIZ];
	char map_cfg_file[MAX_FILE_PATH_LENGTH];
	char wts_bss_cfg_file[MAX_FILE_PATH_LENGTH];
	int sock_br;                       /*bridge socket*/
	int ctrl_sock;
    char br_addr[ETH_ALEN];
	int sock_inf_recv_1905[ITF_NUM];   /*recv socket array*/
	int sock_netlink;				   /*netlink socket to receive the netlink message*/
	struct nl_sock *genl_sock;
	int sock_inf_trx_test;             /*this socket is used to tx/rx packet, it is just for self test*/
	int fd_trx_test;                   /*to record peer fd*/
	int sock_internal;				   /*internal socket to receive the command from worker task*/
//	int sock_wapp;                     /*this socket is used to send request or recv event from wapp*/
//	int fd_wapp;                       /*to record peed wapp fd*/
	struct sockaddr_un sock_addr;
//    int sock_br0;   /* br0 socket */
    int sock_lldp[ITF_NUM];  /* lldp socket*/
    /*push button use*/
    int gpio_fd;
	int discovery_cnt;
	struct _1905_interface_ctrl _1905_ctrl;

    /*for golden node use*/
    int uart2_fd;
    unsigned char is_GN;
    unsigned char is_GCAP01_forward_packet;
    unsigned char is_wait_gcap05_data;
    unsigned char is_need_send_gcap05_data;
    unsigned char *gcap05_data_buffer;
    int gcap05_data_len;
    unsigned char gcap05_timer_cnt;
    uint8_t gcap05_src_mac_addr[ETH_ALEN];
    int sock_gcap05;
    struct sockaddr_ll gcap05_sll;
    unsigned char is_GCAP01_forward_tx_packet;
    unsigned char is_GCAP01_forward_rx_packet;
    unsigned char GCAP01_choose_interface[6];

    /*for sniffer use*/
    int sock_eth0;
    unsigned char sniffer_enable;

    /*for debug dump*/
    unsigned char dump_info;

    struct sockaddr_ll lldp_sll;

    /*for sniffer use*/
//    struct sockaddr_ll eth0_sll;

    unsigned char p1905_al_mac_addr[ETH_ALEN];

#ifdef SUPPORT_WIFI
//    uint8_t wifi0_mac_addr[ETH_ALEN];
    ap_config_para ap_config;
    unsigned char uuid[16];
    unsigned char is_ap_config_by_eth;
    unsigned char *last_rx_data;
	unsigned short last_rx_buf_len;
    unsigned short last_rx_length;
    unsigned char *last_tx_data;
	unsigned short last_tx_buf_len;
    unsigned short last_tx_length;
    unsigned char *current_rx_data;
    unsigned short current_rx_length;
    unsigned char is_authenticator_exist_in_M2;
    unsigned char is_in_encrypt_settings;
    unsigned char get_config_attr_kind;
    enrolle_config_stage enrolle_state;
	renew_bss_state renew_state;
    unsigned short autoconfig_search_mid;
	unsigned char controller_search_cnt;
	controller_search_stage controller_search_state;
	WSC_CONFIG ap_config_data;

	unsigned char four_addr_support;		/*support 4 address, usually used for WiFi interfaces*/
#endif
	unsigned char MAP_Cer;			/*for MAP Certification*/
    /*push button use*/
//    union gpio_info sc_it;
    push_button_param pbc_param;
    int push_button_event_notify_by_msg;
    int push_button_trigger_by_GN;

    unsigned short mid;
	unsigned short dev_send_1905_mid;
    unsigned char need_relay;

    /*for link metric query/response use*/
    link_metrics_query link_metric_query_para;
    link_metrics_rep link_metric_response_para;

	/*contain the cmdu matirial to make the corresponding cmdu*/
	unsigned char send_tlv[MAX_PKT_LEN];
	unsigned short send_tlv_len;
	unsigned long send_tlv_cookie;

	struct os_time own_topo_rsp_update_time; 						   /*last own topology response update time*/

	unsigned int recent_cmdu_rx_if_number;
	unsigned int itf_number;
    struct p1905_interface itf[ITF_NUM];                       /*first itf is the virtual interface*/
    struct bridge_capabiltiy br_cap[BRIDGE_NUM];
    struct non_p1905_neighbor non_p1905_neighbor_dev[ITF_NUM];
    struct p1905_neighbor p1905_neighbor_dev[ITF_NUM];
    struct p1905_topology_db topology_entry;
	struct p1905_topodevice_db topodev_entry;
	struct leaf *root_leaf;									/*root leaf of the topology tree*/
	unsigned int cnt;
	unsigned char radio_number;
	char bss_priority_str[MAX_RADIO_NUM * (IFNAMSIZ + 1) * MAX_BSS_NUM];
	struct radio_info rinfo[MAX_RADIO_NUM];
	unsigned char service;
	unsigned char byte_count_unit;
	unsigned char authenticated;
	struct auto_config_info  current_autoconfig_info;
	struct controller_info cinfo;
	struct ap_capability_db ap_cap_entry;
	struct sta_info sinfo;
	unsigned char sta_notifier;
	struct channel_setting *ch_set;
	struct channel_status *ch_stat;
	struct channel_report *ch_rep;
	/*map policy configuration*/
	struct policy_config map_policy;
	/*client assciation control of steering request*/
	struct control_policy steer_cntrl;
	/*backhaul steering request & response*/
	struct backhaul_steer_request bsteer_req;
	struct backhaul_steer_rsp bsteer_rsp;
	/*client steering request*/
	unsigned int steer_req_len;
	struct steer_request *cli_steer_req;
#ifdef MAP_R2
	/* R2 client steering request*/
	unsigned int steer_req_len_r2;
	struct steer_request_r2 *cli_steer_req_r2;
#endif
	/*ap metrics info*/
	struct metrics_info metric_entry;
	/*store 1905 link metrics info */
	struct _1905_link_stat link_stat;
	unsigned char *pcmdu_tx_buf;         /*pointer to the DEV_SEND_1905  tx content from wapp, e.g high layer data*/
	unsigned short cmdu_tx_buf_len;      /* DEV_SEND_1905 length*/
	unsigned short cmdu_tx_msg_type;     /*type of DEV_SEND_1905*/
	SLIST_HEAD(list_head_agent, agent_list_db) agent_head;
	/*for MAP controller*/
	unsigned int bss_config_bitmap[4];
	unsigned char bss_config_num;
	struct set_config_bss_info bss_config[MAX_SET_BSS_INFO_NUM];
	unsigned char peer_search_band;
	unsigned char backtrace;
	unsigned char core_dump;

	struct buf_info trx_buf;
	struct buf_info reassemble_buf;
	struct buf_info tlv_buf;
	/*to store the recevied tlvs*/
	struct buf_info revd_tlv;
#ifdef MAP_R3
	struct buf_info encr_buf;
#endif
	/*for controller use*/
	stack topo_stack;/*store the pre-order travel result of topology tree*/
	struct renew_context renew_ctx;
	struct _config_buf_ctrl config_buffer;
	/*used for PON*/
	struct contro_conn_port conn_port;
	SLIST_HEAD(list_head_neighbor, neighbor_list_db) query_neighbor_head;
	unsigned char map_version; /*1:DEV_TYPE_R1, 2:DEV_TYPE_R2*/
#ifdef MAP_R2
	struct dl_list clients_table[HASH_TABLE_SIZE];
	struct dl_list bss_head;
#endif // // #ifdef MAP_R2
#ifdef MAP_R3
	struct r3_dpp_information r3_dpp;

	/* struct wpa_parameter */
	void *wpa;
    unsigned int gtk_rekey_interval;
    unsigned int decrypt_fail_threshold;

	struct akm_suite_cap bsta_akm;
	struct akm_suite_cap fh_akm;
	struct akm_suite_cap bh_akm;
	struct r3_onboarding_info r3_oboard_ctx;
	struct r3_reconfig_info r3_reconfig;

	/* store msg for ucc check */
	unsigned int frame_buff_switch;
	/* msg type list head */
	struct dl_list frame_buff_head;

	char dpp_keys_file[MAX_FILE_PATH_LENGTH];
#endif // #ifdef MAP_R3
#ifdef MAP_R3_DE

	struct dev_inven de;
#endif //MAP_R3_DE
	unsigned char switch_polling;
#ifdef MAP_R3_SP
	/*sp rule lists*/
	struct dl_list sp_rule_static_list;
	struct dl_list sp_rule_dynamic_list;
	struct dl_list sp_rule_del_list;
	struct dl_list sp_rule_learn_complete_list;
	/*sp dscp mapping table*/
	struct sp_dscp_pcp_mapping_table sp_dp_table;

	/*sp contex when send sp request*/
	struct sp_request_send_config sp_req_conf;
#endif
#ifdef SINGLE_BAND_SUPPORT
	unsigned char wifi_present;
#endif
	struct delayed_message_context dm_ctx;
#ifdef MAP_R4_SPT
	struct spt_reuse_report *spt_report;
#endif
};

int ap_autoconfig_init(struct p1905_managerd_ctx *ctx);
void multi_ap_controller_search_sm(struct p1905_managerd_ctx* ctx);
void ap_autoconfig_enrolle_sm(struct p1905_managerd_ctx *ctx);
void controller_renew_bss_sm(struct p1905_managerd_ctx *ctx);

void manage_cmd_process(struct p1905_managerd_ctx *ctx, struct manage_cmd *cmd);
int common_process(struct p1905_managerd_ctx *ctx, struct buf_info *buf);
#ifdef MAP_R2
int update_mapfilter_wan_tag(struct p1905_managerd_ctx *ctx, unsigned char status);
#endif
#ifdef DISABLE_SWITCH_POLLING
void genl_netlink_event_process(int sock, void *eloop_ctx, void *sock_ctx);
#endif
void mapfilter_event_process(int sock, void *eloop_ctx, void *sock_ctx);
#ifdef MAP_R3
void buf_huge(struct buf_info *buf, unsigned int pkt_len,	unsigned char bcopy);
void tm_reset_buf_size(void *eloop_data, void *user_ctx);
#endif
#endif /* P1905_MANAGERD_H */
