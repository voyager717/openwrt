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

#ifndef MAP_H
#define MAP_H

#include "interface.h"
#include "data_def.h"
#include "p1905_managerd.h"
#include "multi_ap.h"

#define wapp_utils_success 0
#define wapp_utils_error (-1)

#define IEEE802_11_band_2P4G 0x00
#define IEEE802_11_band_5GL  0x01
#define IEEE802_11_band_5GH  0x02

#define BAND_INVALID_CAP 0x00
#define BAND_2G_CAP 0x01
#define BAND_5GL_CAP  0x02
#define BAND_5GH_CAP  0x04
#define BAND_5G_CAP  0x06
#define BAND_6G_CAP  0x10

#define DETECT_NEIGHBOR_TIME 10
#define RECV_CONFIG_RSP_TIME 3
#define RECV_WSC_M1_TIME 5
#define RECV_WSC_M2_TIME 5
#define RECV_WSC_M2_ACK_TIME 5
#define M1_RETRY_CNT 2
#define ONE_RADIO_CONFIG_TIME_MAX (M1_RETRY_CNT * RECV_WSC_M2_TIME)

#define SERVICE_CONTROLLER 0x00
#define SERVICE_AGENT 0x01
#define SERVICE_KIBMIB_BYTE_COUNTER 0x02

#define BYTE_COUNT_UNIT_BYTES 0x00
#define BYTE_COUNT_UNIT_KBYTES 0x01
#define BYTE_COUNT_UNIT_MBYTES 0x02

/*Multi-AP tlv type*/
#define SUPPORTED_SERVICE_TLV_TYPE 0x80
#define SEARCHED_SERVICE_TLV_TYPE 0x81
#define AP_RADIO_IDENTIFIER_TYPE 0x82
#define AP_OPERATIONAL_BSS_TYPE 0x83
#define AP_ASSOCIATED_CLIENTS_TYPE 0x84
#define AP_RADIO_BASIC_CAPABILITY_TYPE 0x85
#define AP_HT_CAPABILITY_TYPE 0x86
#define AP_VHT_CAPABILITY_TYPE 0x87
#define AP_HE_CAPABILITY_TYPE 0x88
#define STEERING_POLICY_TYPE 0x89
#define METRIC_REPORTING_POLICY_TYPE 0x8A
#define CH_PREFERENCE_TYPE 0x8B
#define RADIO_OPERATION_RESTRICTION_TYPE 0x8C
#define TRANSMIT_POWER_LIMIT_TYPE 0x8D
#define CH_SELECTION_RESPONSE_TYPE 0x8E
#define OPERATING_CHANNEL_REPORT_TYPE 0x8F

#define CLIENT_INFO_TYPE 0x90
#define CLIENT_CAPABILITY_REPORT_TYPE 0x91
#define CLIENT_ASSOCIATION_EVENT_TYPE 0x92
#define AP_METRICS_QUERY_TYPE 0x93
#define AP_METRICS_TYPE 0x94
#define STA_MAC_ADDRESS_TYPE 0x95
#define ASSOC_STA_LINK_METRICS_TYPE 0x96
#define UNASSOC_STA_LINK_METRICS_QUERY_TYPE 0x97
#define UNASSOC_STA_LINK_METRICS_RSP_TYPE 0x98
#define BEACON_METRICS_QUERY_TYPE 0x99
#define BEACON_METRICS_RESPONSE_TYPE 0x9A
#define STEERING_REQUEST_TYPE 0x9B
#define STEERING_BTM_REPORT_TYPE 0x9C
#define CLI_ASSOC_CONTROL_REQUEST_TYPE 0x9D
#define BACKHAUL_STEERING_REQUEST_TYPE 0x9E
#define BACKHAUL_STEERING_RESPONSE_TYPE 0x9F
#define HIGH_LAYER_DATA_TYPE 0xA0
#define AP_CAPABILITY_TYPE 0xA1
#define ASSOC_STA_TRAFFIC_STATS_TYPE 0xA2
#define ERROR_CODE_TYPE 0xA3

//#ifdef MAP_R2

#define CHANNEL_SCAN_REPORTING_POLICY_TYPE 0xA4
#define CHANNEL_SCAN_CAPABILITY_TYPE 0xA5
#define CHANNEL_SCAN_REQUEST_TYPE 0xA6
#define CHANNEL_SCAN_RESULT_TYPE 0xA7
#define TIMESTAMP_TYPE 0xA8
#define _1905_LAYER_SECURITY_CAPABILITY_TYPE 0xA9
#define GROUP_INTEGRITY_KEY_TYPE 0xAA
#define MIC_TYPE 0xAB
#define ENCRYPTED_TYPE 0xAC
#define CAC_REQUEST_TYPE 0xAD
#define CAC_TERMINATION_TYPE 0xAE
#define CAC_COMPLETION_REPORT_TYPE 0xAF
#define CAC_STATUS_REQUEST_TYPE 0xB0
#define CAC_STATUS_REPORT_TYPE 0xB1
#define CAC_CAPABILITIES_TYPE 0xB2
#define MULTI_AP_VERSION_TYPE 0xB3
#define R2_AP_CAPABILITY_TYPE 0xB4
#define DEFAULT_8021Q_SETTING_TYPE 0xB5
#define TRAFFIC_SEPARATION_POLICY_TYPE 0xB6
#define PACKET_FILTERING_POLICY_TYPE 0xB7
#define ETHERNET_CONFIGURATION_POLICY_TYPE 0xB8
#define SERVICE_PRIORITIZATION_TULE_TYPE 0xB9
#define DSCP_MAPPING_TABLE_TYPE 0xBA
#define SERVICE_PRIORITIZATION_INTERFACE_EXCEPTION_TYPE 0xBB
#define R2_ERROR_CODE_TYPE 0xBC
#define AP_OPERATIONAL_BACKHAUL_BSS_TYPE 0xBD
#define AP_RADIO_ADVANCED_CAPABILITIES_TYPE 0xBE
#define ASSOCIATION_STATUS_NOTIFICATION_TYPE 0xBF
//below value should be confirmed by the formal released MAP R2 spec
#define SOURCE_INFO_TYPE 0xC0
#define TUNNELED_MESSAGE_TYPE 0xC1
#define TUNNELED_TYPE 0xC2
#define R2_STEERING_REQUEST_TYPE 0xC3
#define UNSUCCESSFUL_ASSOCIATION_POLICY_TYPE 0xC4
#define METRIC_COLLECTION_INTERVAL_TYPE 0xC5
#define RADIO_METRIC_TYPE 0xC6
#define AP_EXTENDED_METRIC_TYPE 0xC7
#define ASSOCIATED_STA_EXTENDED_LINK_METRIC_TYPE 0xC8
#define ASSOCIATED_STATUS_CODE_TYPE 0xC9
#define REASON_CODE_TYPE 0xCA
//#define BH_STA_RADIA_CAPABILITY_TYPE 0xCB
#define BACKHAUL_STA_RADIA_CAPABILITY_TYPE 0xCB
//#endif // #ifdef MAP_R2
//#ifdef MAP_R3
#define AKM_SUITE_CAPABILITY_TYPE 0xCC
#define PROXIED_ENCAP_DPP_MESSAGE_TYPE 0xCD
#define _1905_ENCAP_EAPOL_MESSAGE_TYPE 0xCE
#define DPP_BOOTSTRAP_URI_NOTIFY_TYPE 0xCF
#define DIRECT_ENCAP_DPP_MESSAGE_TYPE 0xD1
#define DPP_CCE_INDICATION_TYPE 0xD2
#define DPP_CHIRP_VALUE_TYPE 0xD3
#define AGENT_LIST_TLV_TYPE 0xD5
#define BSS_CONFIG_REQUEST_TYPE 0xBB
#define BSS_CONFIG_RESPONSE_TYPE 0xBD
#define BSS_CONFIG_REPORT_TYPE 0xB7
#define R3_DE_INVENTORY_TLV_TYPE 0xD4
#define SPATIAL_REUSE_REQ_TYPE 0xD8
#define SPATIAL_REUSE_CONFIG_RESP_TYPE 0xDA
#define SPATIAL_REUSE_REPORT_TYPE 0xD9
#define CONTROLLER_CAPABILITY_TYPE 0XDD

typedef enum {
	DPP_AUTH_REQUEST = 0,
	DPP_AUTH_RESPONSE,
	DPP_AUTH_CONFIRM,
	DPP_DISCOVERY_REQUEST = 5,
	DPP_DISCOVERY_RESPONSE,
	DPP_PKEX_EXCHANGE_REQUEST,
	DPP_PKEX_EXCHANGE_RESPONSE,
	DPP_PKEX_COMMIT_REVEAL_REQUEST,
	DPP_PKEX_COMMIT_REVEAL_RESPONSE,
} DPP_ACTION_TYPE;
//#endif //MAP_R3

/*Multi-AP tlv length*/
#define SUPPORTED_SERVICE_LENGTH 2
#define SUPPORTED_SERVICE2_LENGTH 3
#define SEARCHED_SERVICE_LENGTH 2
#define AP_RADIO_IDENTIFIER_LENGTH 6
#define AP_CAPABILITY_LENGTH 1
#define AP_HT_CAPABILITY_LENGTH 7
#define AP_VHT_CAPABILITY_LENGTH 12
#define CLIENT_ASSOCIATION_EVENT_LENGTH 13
#define CLIENT_INFO_LENGTH 12
#define TRANSMIT_POWER_LIMIT_LENGTH 7
#define ERROR_CODE_TLV_LENGTH 7


enum internal_vendor_subtype{
	internal_vendor_discovery_message_subtype = 0,
	internal_vendor_notification_message_subtype = 1,
	transparent_vlan_message_subtype,
	internal_vendor_wts_content_message_subtype,
	service_prioritization_vendor_rule_message_subtype
};

typedef enum {
	MAP_BH_ETH,
	MAP_BH_WIFI,
	MAP_BH_UNKNOWN,
} MAP_BH_TYPE;

struct wfa_subelements_attr
{
	unsigned char attribute;
	unsigned char attribute_length;
	unsigned char attribute_value[0];
};

struct operational_bss {
	struct dl_list entry;
	unsigned char mac[ETH_ALEN];
	char ssid[33];
	unsigned char almac[ETH_ALEN];
#ifdef MAP_R2
	unsigned short vid;
#endif
};

struct assoc_client {
	struct dl_list entry;
	unsigned char mac[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	unsigned short vlan_id;
	unsigned char disassoc;
	unsigned char ssid_len;
	char ssid[MAX_SSID_LEN];
	unsigned char al_mac[ETH_ALEN];
	unsigned char apply_2_mapfilter;
};

#ifdef MAP_R2
#define INVALID_VLAN_ID 4095
#define INVALID_PCP 8

struct GNU_PACKED ap_radio_advan_cap {
	unsigned char radioid[6];
	unsigned char flag;
};
#endif

#ifndef BIT
#define BIT(x) (1U << (x))
#endif

#define BIT_BH_STA BIT(7)
#define BIT_BH_BSS BIT(6)
#define BIT_FH_BSS BIT(5)
#define BIT_TEAR_DOWN BIT(4)

unsigned short ap_autoconfiguration_search_message(
    unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short append_searched_role_tlv(unsigned char *pkt);
unsigned short append_autoconfig_freq_band_tlv(unsigned char *pkt, unsigned char band);
int parse_ap_radio_identifier_tlv(struct p1905_managerd_ctx *ctx,
			OUT unsigned char *radio_identifier
#ifdef MAP_R2
			, unsigned char need_store
#endif
			, unsigned short len, unsigned char *val);
int triger_autoconfiguration(struct p1905_managerd_ctx *ctx);
int set_radio_autoconf_trriger(struct p1905_managerd_ctx *ctx, unsigned char radio_id, unsigned char set_value);
int set_radio_autoconf_prepare(struct p1905_managerd_ctx* ctx, unsigned int band, unsigned char set_value);
void auto_configuration_done(struct p1905_managerd_ctx *ctx);
unsigned short channel_selection_request_message(unsigned char *buf, struct agent_list_db *agent, struct p1905_managerd_ctx *ctx);
struct agent_list_db *insert_agent_info(struct p1905_managerd_ctx *ctx, unsigned char *almac);
int delete_agent_ch_prefer_info(struct p1905_managerd_ctx *ctx, struct agent_list_db *agent);
int parse_channel_preference_report_message(struct p1905_managerd_ctx *ctx, struct agent_list_db *agent,
	unsigned char *buf, unsigned int left_tlv_len);
int free_all_the_agents_info(struct list_head_agent *agent_head);
unsigned short combined_infrastructure_metrics_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx, unsigned char *dalmac);
int delete_agent_ap_metrics_info(struct list_head_metrics_rsp_agent *metrics_head);
int delete_agent_tx_link_metrics_info(struct list_head_tx_metrics_agent *tx_metrics_head);
int delete_agent_rx_link_metrics_info(struct list_head_rx_metrics_agent *rx_metrics_head);
int parse_ap_metrics_response_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
int parse_link_metrics_response_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);

unsigned short ap_autoconfiguration_wsc_message(
        unsigned char *buf, struct p1905_managerd_ctx *ctx, unsigned char *al_mac, unsigned char wsc_type);
unsigned short append_WSC_tlv(unsigned char *pkt, struct p1905_managerd_ctx *ctx,
	unsigned char wsc_type, WSC_CONFIG *config_data, unsigned char wfa_vendor_extension);
unsigned short ap_capability_report_message(
    unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short cli_capability_query_message(
        unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short ap_metrics_query_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);
unsigned short associated_sta_link_metrics_query_message(
	unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short unassociated_sta_link_metrics_query_message(
	unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short beacon_metrics_query_message(
	unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short client_steering_request_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);
unsigned short backhaul_steering_request_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);
unsigned short cli_capability_report_message(
    unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short channel_selection_response_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);
unsigned short channel_preference_report_message(
    unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short operating_channel_report_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);
unsigned short client_steering_btm_report_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);
unsigned short client_steering_completed_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);
unsigned short map_policy_config_request_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx, unsigned char *al_mac);
unsigned short client_association_control_request_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);
unsigned short ap_metrics_response_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);
unsigned short associated_sta_link_metrics_response_message(
	unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short unassociated_sta_link_metrics_response_message(
	unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short beacon_metrics_response_message(
	unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short backhaul_steering_response_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);
int high_layer_data_message(unsigned char *buf, struct p1905_managerd_ctx *ctx);
int _1905_ack_message(unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short ap_capability_query_message(unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short channel_preference_query_message(unsigned char *buf, struct p1905_managerd_ctx *ctx);
void find_agent_info(struct p1905_managerd_ctx *ctx, unsigned char *almac, struct agent_list_db **agent);
struct agent_radio_info *find_agent_radio_info(struct agent_list_db *agent, unsigned char *id);
int update_agent_radio_info(struct agent_list_db *agent, struct agent_radio_info *r);
void set_agent_wsc_doing_radio(struct agent_list_db *agent, unsigned char *id);
struct agent_radio_info *find_agent_wsc_doing_radio(struct agent_list_db *agent);
void clear_agent_all_radio_config_stat(struct agent_list_db *agent);
void config_agent_all_radio_config_stat(struct agent_list_db *agent);
void set_agent_radio_config_stat(struct agent_list_db *agent,
	unsigned char *radio_id, unsigned char state);
int get_agent_all_radio_config_state(struct agent_list_db *agent);
struct radio_info *get_rainfo_by_id(struct p1905_managerd_ctx *ctx, unsigned char* identifier);

int dev_send_1905_msg(unsigned char *buf, struct p1905_managerd_ctx *ctx);

unsigned short append_supported_service_tlv(unsigned char *pkt, unsigned char service);
unsigned short append_controller_cap_tlv(unsigned char *pkt, unsigned char kibmib);
int append_operational_bss_tlv(unsigned char *pkt,
	struct list_head_oper_bss *opbss_head);
int append_associated_clients_tlv(unsigned char *pkt,
	struct list_head_assoc_clients *asscli_head, unsigned int cnt);
int append_client_assoc_event_tlv(unsigned char *pkt,
	struct map_client_association_event *assoc_evt);
unsigned short append_searched_service_tlv(unsigned char *pkt);
unsigned short append_ap_radio_basic_capability_tlv(struct p1905_managerd_ctx* ctx, unsigned char *pkt,
	struct radio_info *r);
int append_cli_info_tlv(unsigned char *pkt, struct client_info *info);
int append_cli_capability_report_tlv(struct p1905_managerd_ctx* ctx, unsigned char *pkt, int *error_code);
unsigned short append_ap_radio_identifier_tlv(unsigned char *pkt, unsigned char *identifier);
unsigned short append_ap_metrics_tlv(unsigned char *pkt, struct mrsp_db *mrsp);
unsigned short append_tx_link_metric_tlv(unsigned char *pkt,
	struct tx_link_metric_db *tx_link_metric);
unsigned short append_rx_link_metric_tlv(unsigned char *pkt,
	struct rx_link_metric_db *tx_link_metric);


/***parse tlv***/
int parse_client_assoc_event_tlv(struct p1905_managerd_ctx *ctx, unsigned char *mac,
	unsigned char *bssid, unsigned char *event, unsigned char *al_mac, unsigned short len,
	unsigned char *value, struct dl_list *clients_table);
int parse_supported_service_tlv(unsigned char *service, unsigned short len, unsigned char *value);
int parse_client_info_tlv(struct p1905_managerd_ctx *ctx, unsigned char *bssid, unsigned char *sta_mac,
			unsigned short len, unsigned char *value);
int parse_channel_selection_request_message(struct p1905_managerd_ctx *ctx,
							unsigned char *buf, unsigned int left_tlv_len);
int parse_client_steering_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len, struct err_sta_db *err_sta);
int parse_map_policy_config_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
int parse_cli_assoc_control_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
int parse_ap_metrics_query_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len, OUT unsigned char *radio_identifier);
int parse_associated_sta_link_metrics_query_message(struct p1905_managerd_ctx *ctx,
		unsigned char *buf, unsigned int left_tlv_len);
int parse_unassociated_sta_link_metrics_query_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned int left_tlv_len);
int parse_associated_sta_link_metrics_rsp_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
int parse_beacon_metrics_query_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned int left_tlv_len);
int parse_backhaul_steering_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
int parse_ap_radio_basic_cap_tlv(struct p1905_managerd_ctx *ctx, struct agent_radio_info *rinfo,
					unsigned short len, unsigned char *value);
/***delete tlv***/
int delete_all_ch_prefer_info(struct list_head_ch_prefer *ch_prefer_head);
int delete_exist_steering_policy(struct steer_policy *spolicy);
int delete_exist_metrics_policy(struct p1905_managerd_ctx *ctx, struct metrics_policy *mpolicy);
int delete_exist_unlink_metrics_rsp(struct unlink_metrics_info *unlink_metrics);
int delete_exist_radio_basic_capability(struct p1905_managerd_ctx *ctx, unsigned char* identifier);
int delete_exist_operational_bss(struct p1905_managerd_ctx *ctx, unsigned char *identifier);
int delete_exist_ch_prefer_info(struct p1905_managerd_ctx *ctx, unsigned char *identifier);
int delete_exist_restriction_info(struct p1905_managerd_ctx *ctx, unsigned char *identifier);
int delete_exist_metrics_info(struct p1905_managerd_ctx *ctx, unsigned char *bssid);
int delete_exist_traffic_stats_info(struct p1905_managerd_ctx *ctx, unsigned char *identifier);
int delete_exist_link_metrics_info(struct p1905_managerd_ctx *ctx, unsigned char *identifier);


int insert_new_operational_bss(struct p1905_managerd_ctx *ctx, struct oper_bss_cap *opcap);
int insert_new_channel_prefer_info(struct p1905_managerd_ctx *ctx, struct ch_prefer *prefer);
int insert_new_restriction_info(struct p1905_managerd_ctx *ctx, struct restriction *op_restrict);
int insert_new_metrics_info(struct p1905_managerd_ctx *ctx, struct ap_metrics_info *minfo);
int insert_new_traffic_stats_info(struct p1905_managerd_ctx *ctx, struct sta_traffic_stats *traffic_stats);
int insert_new_link_metrics_info(struct p1905_managerd_ctx *ctx, struct sta_link_metrics *metrics_info);

void update_radio_info(struct p1905_managerd_ctx* ctx, struct wps_get_config* info);
void update_channel_setting(struct p1905_managerd_ctx *ctx, unsigned char *almac,
	unsigned short mid);
unsigned char check_invalid_channel(unsigned char op_class, unsigned char ch_num, unsigned char *ch_list);
int update_ap_ht_cap(struct p1905_managerd_ctx* ctx, struct ap_ht_capability *pcap);
int update_ap_vht_cap(struct p1905_managerd_ctx* ctx, struct ap_vht_capability *pcap);
int update_ap_he_cap(struct p1905_managerd_ctx* ctx, struct ap_he_capability *pcap);
int update_one_sta_link_metrics_info(struct p1905_managerd_ctx *ctx, struct link_metrics *metrics);
int update_unlink_metrics_rsp(struct unlink_metrics_info *unlink_metrics_ctx, struct unlink_metrics_rsp *unlink_metrics);
int update_assoc_sta_info(struct p1905_managerd_ctx *ctx, struct map_client_association_event *cinfo);
int update_channel_report_info(struct p1905_managerd_ctx *ctx, struct channel_report *rep, unsigned char len);
int process_1905_request(struct p1905_managerd_ctx* ctx, struct _1905_cmdu_request* request);
int dev_send_1905(struct p1905_managerd_ctx* ctx, struct _1905_cmdu_request* request);

unsigned short append_send_tlv(unsigned char *pkt, struct p1905_managerd_ctx *ctx);

unsigned short append_send_tlv_relayed(unsigned char *pkt, struct p1905_managerd_ctx *ctx);

int reset_stored_tlv(struct p1905_managerd_ctx* ctx);
int store_revd_tlvs(struct p1905_managerd_ctx* ctx, unsigned char *revd_buf, unsigned short len);
int fill_send_tlv(struct p1905_managerd_ctx* ctx, unsigned char* buf, unsigned short len);
int reset_send_tlv(struct p1905_managerd_ctx *ctx);

int read_bss_conf_and_renew(struct p1905_managerd_ctx *ctx, unsigned char local_only);
int read_bss_conf_and_renew_v2(struct p1905_managerd_ctx *ctx, unsigned char local_only);


int _1905_read_dev_send_1905(struct p1905_managerd_ctx* ctx,
	char* name, unsigned char *almac, unsigned short* _type, unsigned short *tlv_len, unsigned char *pay_load);
int parse_combined_infra_metrics_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
int parse_backhaul_steering_rsp_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
void show_1905_mapfilter_version(struct p1905_managerd_ctx* ctx);
void send_1905_raw_data(struct p1905_managerd_ctx* ctx, char *file);
void read_1905_bss_config(struct p1905_managerd_ctx* ctx, char *file);
int map_event_handler(struct p1905_managerd_ctx *ctx, char *buf, int len, unsigned char type,
	unsigned char *reply, int *reply_len);
#ifdef SUPPORT_CMDU_RELIABLE
void cmdu_reliable_send(struct p1905_managerd_ctx *ctx, unsigned short msg_type,
		unsigned short mid, unsigned int ifidx);
#endif
int change_role_dynamic(struct p1905_managerd_ctx* ctx, unsigned char role);

int get_local_device_info(struct p1905_managerd_ctx* ctx, unsigned char *buf,
	unsigned int buf_len, unsigned short *len);

int parse_ap_capability_report_message(struct p1905_managerd_ctx *ctx,
		unsigned char *almac, unsigned char *buf, unsigned int left_tlv_len,
		OUT struct agent_radio_info *r, OUT unsigned char *radio_cnt);

int parse_channel_selection_rsp_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);

int parse_operating_channel_report_message(struct p1905_managerd_ctx *ctx,
		unsigned char *buf, unsigned int left_tlv_len);

int parse_client_capability_report_message(struct p1905_managerd_ctx *ctx,
		unsigned char *buf, unsigned int left_tlv_len);

int parse_unassoc_sta_link_metrics_response_message(struct p1905_managerd_ctx *ctx,
		unsigned char *buf, unsigned int left_tlv_len);

int parse_client_steering_btm_report_message(struct p1905_managerd_ctx *ctx,
		unsigned char *buf, unsigned int left_tlv_len);

int parse_beacon_metrics_response_message(struct p1905_managerd_ctx *ctx,
		unsigned char *buf, unsigned int left_tlv_len);

int parse_higher_layer_data_message(struct p1905_managerd_ctx *ctx,
		unsigned char *buf, unsigned int left_tlv_len);

int set_bss_config(struct p1905_managerd_ctx *ctx, enum BSS_CONFIG_OPERATION operation,
	struct bss_config_info* info);

void build_ap_metric_query_tlv(struct p1905_managerd_ctx *ctx);

void report_own_topology_rsp(struct p1905_managerd_ctx *ctx,
	unsigned char *al_mac, struct bridge_capabiltiy *br_cap_list,
	struct p1905_neighbor *p1905_dev, struct non_p1905_neighbor *non_p1905_dev,
	unsigned char service, struct list_head_oper_bss *opbss_head,
    struct list_head_assoc_clients *asscli_head, unsigned int cnt);


int get_bandcap(struct p1905_managerd_ctx *ctx, unsigned char opclass,
	unsigned char non_opch_num, unsigned char *non_opch);

unsigned char determin_band_config(struct p1905_managerd_ctx *ctx, unsigned char *almac,
	unsigned char band_cap);

WIFI_UTILS_STATUS config_bss_by_band(struct p1905_managerd_ctx *ctx, unsigned char band,
	WSC_CONFIG *wsc, unsigned char *wfa_vendor_extension);

void init_radio_info_by_intf(struct p1905_managerd_ctx* ctx, struct p1905_interface *itf);
void metrics_report_timeout(void *eloop_data, void *user_ctx);
unsigned short create_vs_info_for_specific_discovery(unsigned char *vs_info);
void discovery_at_interface_link_up(void *eloop_data, void *user_ctx);
void attach_action(void *eloop_data, void *user_ctx);
unsigned short create_vs_info_for_specific_notification(
	struct p1905_managerd_ctx *ctx, unsigned char *vs_info);
void reset_controller_connect_ifname(struct p1905_managerd_ctx *ctx,
	unsigned char *link_down_ifname);
int check_add_error_code_tlv(struct p1905_managerd_ctx *ctx, unsigned char *pkt,
	unsigned char max_sta_cnt, unsigned short msgtype);
int append_map_version_tlv(unsigned char *pkt, unsigned char version);

#ifdef MAP_R2
/*channel scan feature*/
unsigned short channel_scan_request_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx);
unsigned short channel_scan_report_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx);
int parse_channel_scan_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
int parse_channel_scan_report_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
unsigned short append_channel_scan_capability_tlv(unsigned char *pkt,
	struct list_head_ch_scan_cap *shead);
unsigned short append_cac_capability_tlv(
        unsigned char *pkt, struct radio_cac_capability_db *cachead);

int delete_exist_channel_scan_capability(struct p1905_managerd_ctx *ctx,
	unsigned char* identifier);
int delete_exist_cac_capability(
	struct p1905_managerd_ctx *ctx, unsigned char* identifier);

int insert_new_channel_scan_capability(struct p1905_managerd_ctx *ctx,
	struct scan_capability_lib *scap);
int insert_new_cac_capability(
	struct p1905_managerd_ctx *ctx, struct cac_capability_lib *cac_capab_in);


int delete_exist_traffic_policy(struct p1905_managerd_ctx *ctx,
	struct traffics_policy *tpolicy);
int parse_traffic_separation_policy_tlv(
	struct traffics_policy *policy, unsigned short len, unsigned char *val);
int parse_default_802_1q_setting_tlv(
	struct def_8021q_setting *setting,
	unsigned short len, unsigned char *val);
int delete_exist_eth_config_policy(struct p1905_managerd_ctx *ctx,
	struct ethernets_policy *epolicy);
int parse_eth_config_policy_tlv(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);
int delete_exist_pfilter_policy(struct p1905_managerd_ctx *ctx,
	struct pfiltering_policy *fpolicy);
int parse_packet_filtering_policy_tlv(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);

void map_r2_set_all_policy(void *eloop_ctx, void *timeout_ctx);

int parse_tunneled_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
int parse_assoc_status_notification_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
unsigned short tunneled_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx);
unsigned short assoc_status_notification_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx);

int parse_cac_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);

int parse_cac_terminate_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);

unsigned short cac_request_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx);

unsigned short cac_terminate_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx);

unsigned short client_disassciation_stats_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx);
unsigned short backhual_sta_cap_query_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx);
unsigned short backhual_sta_cap_report_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx);

int parse_client_disassciation_stats_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
int parse_unsuccess_assoc_policy_tlv(unsigned char *buf, struct p1905_managerd_ctx *ctx);
int parse_backhaul_sta_cap_report_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
int parse_failed_association_message(struct p1905_managerd_ctx *ctx, unsigned char *buf);
int parse_failed_connection_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);

int update_r2_ap_capability(
	struct p1905_managerd_ctx *ctx, struct ap_r2_capability *ap_r2_cap);

int insert_new_ap_extended_capability(
	struct p1905_managerd_ctx *ctx, struct ap_extended_metrics_lib *metric);
int insert_new_radio_metric(
	struct p1905_managerd_ctx *ctx, struct radio_metrics_lib *metric);
int insert_assoc_sta_extended_link_metric(
	struct p1905_managerd_ctx *ctx, struct sta_extended_metrics_lib *metric);

void cmd_set_ts_policy(struct p1905_managerd_ctx *ctx, char* buf);
void cmd_set_ts_pvlan(struct p1905_managerd_ctx *ctx, char* buf);
void cmd_set_ts_policy_done(struct p1905_managerd_ctx *ctx);
void cmd_set_ts_policy_clear(struct p1905_managerd_ctx *ctx);
void map_r2_notify_ts_config(void *eloop_ctx, void *timeout_ctx);
void traffic_separation_init(struct p1905_managerd_ctx *ctx);
void ts_traffic_separation_deinit(struct p1905_managerd_ctx *ctx);
int update_traffic_separation_policy(struct p1905_managerd_ctx *ctx,
	struct set_config_bss_info bss_info[], unsigned char bss_num);
void map_notify_transparent_vlan_setting(void *eloop_ctx, void *timeout_ctx);
void show_all_assoc_clients(struct p1905_managerd_ctx *ctx);
void show_all_operational_bss(struct p1905_managerd_ctx *ctx);
void update_operation_bss_vlan(struct p1905_managerd_ctx *ctx,struct dl_list *bss_head,
	struct traffics_policy *policy, unsigned char *al_mac);
void maintain_all_assoc_clients(struct p1905_managerd_ctx *ctx);
struct assoc_client *get_assoc_cli_by_mac(struct p1905_managerd_ctx *ctx, unsigned char mac[]);
struct assoc_client *create_assoc_cli(struct p1905_managerd_ctx *ctx, unsigned char mac[],
	unsigned char *al_mac);
int parse_assoc_clients_tlv(struct p1905_managerd_ctx *ctx,
	unsigned char *al_mac, unsigned short len, unsigned char *value, struct dl_list *clients_table);
struct operational_bss *get_bss_by_bssid(struct p1905_managerd_ctx *ctx,
	unsigned char *mac);
struct operational_bss *create_oper_bss(struct p1905_managerd_ctx *ctx,
	unsigned char *mac, char *ssid, unsigned char ssid_len, unsigned char *almac, struct dl_list *bss_list);
int parse_ap_operational_bss_tlv(struct p1905_managerd_ctx *ctx,
		unsigned char *almac, unsigned short len, unsigned char *value, struct dl_list *bss_list);
unsigned short get_vlan_by_ssid_from_ts_policy(char *ssid, unsigned char ssid_len,
	struct traffics_policy *policy);
int append_default_8021Q_tlv(struct p1905_managerd_ctx *ctx,
	unsigned char *pkt, unsigned char *al_mac,
	unsigned char bss_num, struct set_config_bss_info *bss_info);
int append_traffic_separation_tlv(struct p1905_managerd_ctx *ctx,
	unsigned char *pkt, unsigned char *al_mac,
	unsigned char bss_num, struct set_config_bss_info *bss_info);
int append_transparent_vlan_tlv(struct p1905_managerd_ctx *ctx, unsigned char *pkt);
int append_r2_cap_tlv(unsigned char *pkt, struct ap_r2_capability *r2_cap);
int append_metric_collection_interval_tlv(unsigned char *pkt, unsigned int interval);
int append_ap_radio_advan_tlv(unsigned char *pkt, struct ap_radio_advan_cap *radio_adv_capab);
unsigned short failed_connection_message(unsigned char *buf, struct p1905_managerd_ctx *ctx);
void add_new_radio_identifier(struct p1905_managerd_ctx *ctx, unsigned char *identifier);
void delete_exist_radio_identifier(struct p1905_managerd_ctx *ctx);
unsigned short append_backhaul_sta_radio_cap_tlv(unsigned char *pkt,
    unsigned char *rid, unsigned char *itf_mac);
int parse_cac_status_report_type_tlv(struct p1905_managerd_ctx *ctx, unsigned char *buf);
int parse_ap_radio_advanced_capability_tlv(struct p1905_managerd_ctx *ctx,
		unsigned char *almac, struct ts_cap_db **ts_cap, unsigned short len, unsigned char *value);
int parse_r2_ap_capability_tlv(struct ap_r2_capability *r2_cap, unsigned char *almac,
				  unsigned short len, unsigned char *value);
#endif // MAP_R2



#ifdef MAP_R3
#define BIT_BH_STA BIT(7)
#define BIT_BH_BSS BIT(6)
#define BIT_FH_BSS BIT(5)
#define BIT_TEAR_DOWN BIT(4)
#define BIT_PROFILE1_DISALLOW BIT(3)
#define BIT_PROFILE2_DISALLOW BIT(2)


unsigned short bss_configuration_response_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx, unsigned char *al_mac);
int parse_1905_bss_configuration_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len, unsigned char *al_mac, OUT struct agent_radio_info *r,
	OUT unsigned char *radio_cnt);
int parse_bss_configuration_response_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len, unsigned char *al_mac);
unsigned short bss_configuration_result_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx, unsigned char *al_mac);
unsigned short agent_list_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx, unsigned char *al_mac);
unsigned short chirp_notification_message(
	unsigned char *buf, struct p1905_managerd_ctx *ctx);
int init_akm_capability(struct p1905_managerd_ctx *ctx);
void deinit_akm_capability(struct p1905_managerd_ctx *ctx);
int append_bss_configuration_request_tlv(unsigned char *pkt, struct p1905_managerd_ctx* ctx);
int parse_bss_configuration_response_tlv(
	unsigned char *buf, unsigned short len, struct r3_onboarding_info *r3_ctx);
void r3_set_config_state(struct p1905_managerd_ctx *ctx,  unsigned char state);
void r3_config_sm_step(void *eloop_ctx, void *timeout_ctx);
void r3_set_config(void *eloop_ctx, void *timeout_ctx);
int append_bss_config_report_tlv(unsigned char *pkt, struct p1905_managerd_ctx* ctx);
void wait_send_bss_configuration_result(void *eloop_ctx, void *timeout_ctx);
void cmd_dev_start_buffer();
void cmd_dev_stop_buffer();
void cmd_dev_get_frame_info();

int update_ap_wf6_cap(struct p1905_managerd_ctx* ctx, struct ap_wf6_cap_roles *pcap);
unsigned short _1905_rekey_request_message(unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short _1905_decryption_failure_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned char *al_mac);
unsigned short _1905_encap_dpp_message(unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short dpp_cce_indication_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);
unsigned short bss_configuration_request_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);

unsigned short bss_reconfiguration_trigger_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);



int append_dpp_chirp_value_tlv
(
	unsigned char *pkt,
	struct chirp_tlv_info *chirp
);

int append_1905_layer_security_tlv(unsigned char *pkt);
int append_1905_encap_eapol_tlv(unsigned char *pkt, struct p1905_managerd_ctx* ctx);
int append_bootstrap_uri_notification_tlv(unsigned char *pkt, struct p1905_managerd_ctx* ctx);
int append_dpp_cce_indication_tlv(unsigned char *pkt, struct p1905_managerd_ctx* ctx);
int append_akm_suite_cap_tlv(unsigned char *pkt, struct p1905_managerd_ctx* ctx);
int append_bh_sta_radio_cap_tlv(unsigned char *pkt, struct p1905_managerd_ctx* ctx);

unsigned short direct_encap_dpp_message(unsigned char *buf, struct p1905_managerd_ctx *ctx);
unsigned short proxied_encap_dpp_message(unsigned char *buf, struct p1905_managerd_ctx *ctx);

unsigned short _1905_encap_eapol_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx);

unsigned short dpp_bootstrap_uri_notification_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx);

unsigned short dpp_bootstrap_uri_query_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx);


int parse_proxied_encap_dpp_message(
	struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len,
	unsigned char *al_mac,
	unsigned char *frame_type,
	unsigned short *frame_offset,
	unsigned short *frame_len);

int parse_1905_encap_eapol_message(
	struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len,
	unsigned short *eapol_offset, unsigned short *eapol_len);

int parse_dpp_bootstraping_uri_notification_message(
	struct p1905_managerd_ctx *ctx,
	unsigned char *buf,
	unsigned int left_tlv_len);
int parse_dpp_chirp_value_tlv(
	unsigned char *buf, unsigned short len, struct chirp_tlv_info *chirp);
int parse_chirp_notification_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len, unsigned char *al_mac);

int parse_direct_encap_dpp_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned int left_tlv_len,
	unsigned char *frame_type, unsigned short *frame_offset, unsigned short *frame_len,
	unsigned char *al_mac);
int parse_dpp_cce_indication_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
int parse_1905_rekey_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len);
int parse_1905_layer_security_capability_tlv(struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned char *onboardingProto, unsigned char *msgIntAlg, unsigned char *msgEncAlg);
int parse_1905_decryption_failure_message(struct p1905_managerd_ctx *ctx,
			unsigned char *buf, unsigned int left_tlv_len, unsigned char *al_mac);
unsigned short append_ap_wf6_capability_tlv(unsigned char *pkt, struct ap_wf6_role_db *wf6_cap);
int parse_ap_wf6_capability_tlv(struct p1905_managerd_ctx *ctx, unsigned char *buf);


int parse_agent_list_message(unsigned char *buf, unsigned int left_tlv_len);
int parse_service_prioritization_request_message(struct p1905_managerd_ctx *ctx, unsigned char *buf);
void periodic_send_autoconfig_search(void *eloop_ctx, void *timeout_ctx);
void cmd_send_bss_reconfiguration_trigger(struct p1905_managerd_ctx *ctx, char *buf);
void add_chirp_hash_list(struct p1905_managerd_ctx *ctx,
	struct chirp_tlv_info *chirp_tlv, unsigned char *almac);
void report_akm_suit_cap_to_mapd(void *eloop_ctx, void *timeout_ctx);
void report_1905_secure_cap_to_mapd(void *eloop_ctx, void *timeout_ctx);
void report_sp_standard_rule_to_mapd(struct p1905_managerd_ctx *ctx);
void report_sp_standard_rule_to_mapd_timeout(void *eloop_ctx, void *timeout_ctx);


#endif // #endif MAP_R3

#ifdef MAP_R3_DE

unsigned short append_dev_inven_tlv(unsigned char *pkt, struct dev_inven *de);
int update_ap_dev_inven(struct p1905_managerd_ctx* ctx, struct dev_inven *de);
#endif

int init_wireless_interface(struct p1905_managerd_ctx *ctx,
	struct interface_info_list_hdr *info);

int common_info_init(struct p1905_managerd_ctx *ctx);

int write_almac_to_1905_cfg_file(char *cfg_file_name, unsigned char *almac);
void mark_valid_topo_rsp_node(struct p1905_managerd_ctx *ctx);
void update_topology_info(struct p1905_managerd_ctx *ctx,
	struct topology_response_db *tpgr);
struct topology_response_db *find_topology_rsp_by_almac(struct p1905_managerd_ctx *ctx,
	unsigned char *almac);

unsigned short create_vs_info_for_wts_content(unsigned char *vs_info);

unsigned short create_vendor_specific_wts_content_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);
int delete_exist_steer_cntrl_db(struct p1905_managerd_ctx *ctx);
int check_C_and_A_concurrent_by_topology_rsp(struct p1905_managerd_ctx *ctx, struct topology_response_db *tpgr_db);
int check_C_and_A_concurrent__by_discovery(struct p1905_managerd_ctx *ctx, unsigned char *al_mac_addr, unsigned char *mac_addr);
void set_trx_config_for_bss(struct p1905_managerd_ctx*ctx, unsigned char map_vendor_extension, unsigned char *bss_mac);
int dm_init(struct p1905_managerd_ctx *ctx);
int store_dm_buffer(IN struct p1905_managerd_ctx *ctx, IN unsigned char *almac, IN unsigned char event,
	IN unsigned short mid, IN char *if_name);
int pop_dm_buffer(IN struct p1905_managerd_ctx *ctx, IN unsigned char event, OUT unsigned char *almac,
	OUT unsigned short *mid, OUT char *if_name);
int clear_expired_dm_buffer(IN struct p1905_managerd_ctx *ctx);
int dm_deinit(struct p1905_managerd_ctx *ctx);
unsigned char find_band_by_opclass(char *opclass);
int cont_handle_agent_ap_radio_basic_capability(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	struct agent_radio_info *r, unsigned char radio_cnt);
#ifdef MAP_R3
int cont_handle_bss_configuration_request(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	struct agent_radio_info *r, unsigned char radio_cnt);
#endif
const char *band_2_string(unsigned char band);
#endif /*MAP_H*/

