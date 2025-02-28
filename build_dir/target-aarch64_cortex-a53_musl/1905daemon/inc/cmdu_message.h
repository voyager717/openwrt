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

#ifndef CMDU_MESSAGE_H
#define CMDU_MESSAGE_H

#include <linux/if_ether.h>
#include "cmdu_tlv.h"

#define MIN_TLVS_LENGTH    38   /*46 - 8(cmdu header size) */
#define MAX_TLVS_LENGTH    1492     /*1500 -8(cmdu header size)*/
#define CMDU_HLEN          8

/*p1905.1 message version*/
#define MESSAGE_VERSION 0


/*p1905.1 message type*/
#define TOPOLOGY_DISCOVERY      0x0000
#define TOPOLOGY_NOTIFICATION   0x0001
#define TOPOLOGY_QUERY          0x0002
#define TOPOLOGY_RESPONSE       0x0003
#define VENDOR_SPECIFIC         0x0004
#define LINK_METRICS_QUERY      0x0005
#define LINK_METRICS_RESPONSE   0x0006
#define AP_AUTOCONFIG_SEARCH    0x0007
#define AP_AUTOCONFIG_RESPONSE  0x0008
#define AP_AUTOCONFIG_WSC       0x0009
#define AP_AUTOCONFIG_RENEW     0x000A
#define P1905_PB_EVENT_NOTIFY   0x000B
#define P1905_PB_JOIN_NOTIFY    0x000C

#define SCLIENT_CAPABILITY_QUERY 0x000D
#define ICHANNEL_SELECTION_REQUEST 0x000E
#define Ack_1905 0x8000
#define AP_CAPABILITY_QUERY 0x8001
#define AP_CAPABILITY_REPORT 0x8002
#define MAP_POLICY_CONFIG_REQUEST 0x8003
#define CHANNEL_PREFERENCE_QUERY 0x8004
#define CHANNEL_PREFERENCE_REPORT 0x8005
#define CHANNEL_SELECTION_REQUEST 0x8006
#define CHANNEL_SELECTION_RESPONSE 0x8007
#define OPERATING_CHANNEL_REPORT 0x8008
#define CLIENT_CAPABILITY_QUERY 0x8009
#define CLIENT_CAPABILITY_REPORT 0x800A
#define AP_LINK_METRICS_QUERY 0x800B
#define AP_LINK_METRICS_RESPONSE 0x800C
#define ASSOC_STA_LINK_METRICS_QUERY 0x800D
#define ASSOC_STA_LINK_METRICS_RESPONSE 0x800E
#define UNASSOC_STA_LINK_METRICS_QUERY 0x800F
#define UNASSOC_STA_LINK_METRICS_RESPONSE 0x8010
#define BEACON_METRICS_QUERY 0x8011
#define BEACON_METRICS_RESPONSE 0x8012
#define COMBINED_INFRASTRUCTURE_METRICS 0x8013
#define CLIENT_STEERING_REQUEST 0x8014
#define CLIENT_STEERING_BTM_REPORT 0x8015
#define CLIENT_ASSOC_CONTROL_REQUEST 0x8016
#define CLIENT_STEERING_COMPLETED 0x8017
#define HIGHER_LAYER_DATA_MESSAGE 0x8018
#define BACKHAUL_STEERING_REQUEST 0x8019
#define BACKHAUL_STEERING_RESPONSE 0x801A
#define CHANNEL_SCAN_REQUEST 0x801B
#define CHANNEL_SCAN_REPORT 0x801C
//#define _1905_GROUP_INTEGRITY_KEY 0x801D
#define DPP_CCE_INDICATION_MESSAGE 0x801D
#define _1905_REKEY_REQUEST 0x801E
#define _1905_DECRYPTION_FAILURE 0x801F
#define CAC_REQUEST 0x8020
#define CAC_TERMINATION 0x8021
#define CLIENT_DISASSOCIATION_STATS 0x8022
#define SERVICE_PRIORITIZATION_REQUEST 0x8023
#define ERROR_RESPONSE 0x8024
#define ASSOCIATION_STATUS_NOTIFICATION 0x8025
#define TUNNELED_MESSAGE 0x8026
#define BACKHAUL_STA_CAP_QUERY_MESSAGE 0x8027
#define BACKHAUL_STA_CAP_REPORT_MESSAGE 0x8028
#define PROXIED_ENCAP_DPP_MESSAGE 0x8029
#define DIRECT_ENCAP_DPP_MESSAGE 0x802A
#define BSS_RECONFIGURATION_TRIGGER_MESSAGE 0x802B
#define BSS_CONFIGURATION_REQUEST_MESSAGE 0x802C
#define BSS_CONFIGURATION_RESPONSE_MESSAGE 0x802D
#define BSS_CONFIGURATION_RESULT_MESSAGE 0x802E
#define CHIRP_NOTIFICATION_MESSAGE 0x802F
#define _1905_ENCAP_EAPOL 0x8030
#define DPP_BOOTSTRAPING_URI_NOTIFICATION 0x8031
#define DPP_BOOTSTRAPING_URI_QUERY 0x8032
/*
	no FAILED_ASSOCIATION_MESSAGE any more from spec
	replaced by FAILED_CONNECTION_MESSAGE
*/
//#define FAILED_ASSOCIATION_MESSAGE 0x8033
#define FAILED_CONNECTION_MESSAGE 0x8033
#define DPP_URI_NOTIFICATION_MESSAGE 0x8034
#define AGENT_LIST_MESSAGE 0x8035

#define DEV_SEND_1905_REQUEST 0x9000


/*
 * WARNING: cmdu parser will check msg type!!!
 * If you want to add a new message type here, please add the new message type to
 * "parse_cmdu_message" function.
 */

typedef enum
{
    e_topology_discovery = 1,
    e_topology_notification,
    e_topology_query,
    e_topology_response,
    e_vendor_specific,
    e_link_metric_query,
    e_link_metric_response,
    e_push_button_event_notification,
    e_push_button_join_notification,//for PLC
    e_push_button_join_notification_wifi,//for wifi
    e_ap_autoconfiguration_search,
    e_ap_autoconfiguration_wsc_m1,
    e_ap_autoconfiguration_response,
    e_ap_autoconfiguration_wsc_m2,
    e_ap_autoconfiguration_renew,
	e_1905_ack = 0x8000,
	e_ap_capability_query,
	e_ap_capability_report,
	e_multi_ap_policy_config_request,
	e_channel_preference_query,
	e_channel_preference_report,
	e_channel_selection_request,
	e_channel_selection_response,
	e_operating_channel_report,
	e_cli_capability_query,
	e_cli_capability_report,
	e_ap_metrics_query,
	e_ap_metrics_response,
	e_associated_sta_link_metrics_query,
	e_associated_sta_link_metrics_response,
	e_unassociated_sta_link_metrics_query,
	e_unassociated_sta_link_metrics_response,
	e_beacon_metrics_query,
	e_beacon_metrics_response,
	e_combined_infrastructure_metrics,
	e_client_steering_request,
	e_client_steering_btm_report,
	e_client_association_control_request,
	e_steering_completed,
	e_higher_layer_data,
	e_backhaul_steering_request,
	e_backhaul_steering_response,
#ifdef MAP_R2
	/*channel scan feature*/
	e_channel_scan_request,
	e_channel_scan_report,
//	e_1905_group_integrity_key,
	e_1905_rekey_request = 0x801e,
	e_1905_decryption_failure,
	e_cac_request,
	e_cac_termination,
	e_client_disassociation_stats,
	e_service_prioritization_request,
	e_service_prioritization_request_4_learning_comp,
	e_error_response,
	e_association_status_notification,
	e_tunneled_message,
	e_failed_connection_message,
	e_backhaul_sta_capability_query,
	e_backhaul_sta_capability_report,
	e_backhual_sta_cap_query_message,
	e_backhual_sta_cap_report_message,
#endif // #ifdef MAP_R2
#ifdef MAP_R3
	e_proxied_encap_dpp,
	e_direct_encap_dpp,
	e_1905_encap_eapol,
	e_dpp_bootstrapping_uri_notification,
	e_dpp_bootstrapping_uri_query,
	e_dpp_cce_indication,
	e_chirp_notification,
	e_bss_configuration_request,
	e_bss_configuration_response,
	e_bss_configuration_result,
	e_bss_reconfiguration_trigger,
	e_ptk_rekey_request,
	e_decryption_failure,
	e_agent_list,
#endif // #ifdef MAP_R3
	e_dev_send_1905_request = 0x9000,
	e_topology_discovery_with_vendor_ie,
	e_vendor_specific_topology_discovery,
	e_topology_notification_with_vendor_ie,
	e_vendor_specific_wts_content,
}msgtype;

typedef struct
{
    unsigned char message_version;
    unsigned char reserved_field_0;
    __be16 message_type;
    __be16 message_id;
    unsigned char fragment_id;
#ifdef RT_BIG_ENDIAN
	unsigned char last_fragment_indicator:1;
	unsigned char relay_indicator:1;
	unsigned char reserve_field_1:6;
#else
    unsigned char reserve_field_1:6;
    unsigned char relay_indicator:1;
    unsigned char last_fragment_indicator:1;
#endif
} __attribute__ ((__packed__)) cmdu_message_header;

unsigned char *get_tlv_buffer(struct p1905_managerd_ctx *ctx);

unsigned short create_vendor_specific_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx);
unsigned short create_link_metrics_query_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned char *target_al_mac, unsigned char type);
unsigned short create_link_metrics_response_message(unsigned char *buf,
	unsigned char *target_al_mac, unsigned char type,
    unsigned char *local_al_mac, struct p1905_interface *itf_list,
    struct p1905_neighbor *devlist, struct list_head_tpddb *tpd_head,
    struct p1905_managerd_ctx *ctx);
unsigned short push_button_event_notification_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned char *al_mac, struct p1905_interface *itf_list);

unsigned short push_button_join_notification_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned char *local_mac, unsigned char *al_mac,
	push_button_param *pbc_p, unsigned char is_plc,
	unsigned char role);

unsigned short create_topology_discovery_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned char *al_mac, unsigned char *itf_mac,
	unsigned short vs_len, unsigned char *vs_info);

unsigned short create_vendor_specific_topology_discovery_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned char *al_mac, unsigned char *itf_mac,
	unsigned short vs_len, unsigned char *vs_info);

unsigned short create_topology_query_message(unsigned char *buf,  struct p1905_managerd_ctx *ctx);

unsigned short create_topology_response_message(unsigned char *buf,
    unsigned char *al_mac, struct p1905_managerd_ctx *ctx,
    struct bridge_capabiltiy *br_cap_list, struct p1905_neighbor *p1905_dev,
    struct non_p1905_neighbor *non_p1905_dev, struct list_head_tprdb *tpgr_head,
    unsigned char service, struct list_head_oper_bss *opbss_head,
    struct list_head_assoc_clients *asscli_head, unsigned int cnt);

unsigned short create_topology_notification_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf,
    unsigned char *al_mac, struct map_client_association_event *assoc_evt,
    unsigned char notifier, unsigned short vs_len, unsigned char *vs_info);
void delete_non_p1905_neighbor_dev_info(struct non_p1905_neighbor *non_p1905_dev);
unsigned short ap_autoconfiguration_response_message(
        unsigned char *buf, unsigned char *dmac, struct p1905_managerd_ctx *ctx);
unsigned short ap_autoconfiguration_renew_message(
        unsigned char *buf, struct p1905_managerd_ctx *ctx);
void delete_exist_topology_response_database(struct p1905_managerd_ctx *ctx,
	unsigned char *al_mac);
int update_p1905_neighbor_dev_info(struct p1905_managerd_ctx *ctx,
    unsigned char *al_mac, unsigned char *neighbor_itf_mac,
    unsigned char *itf_mac_addr, unsigned char *notify);
void detect_neighbor_existence(struct p1905_managerd_ctx *ctx,
	unsigned char *remote_almac, unsigned char *sta_mac);
void recv_neighbor_disc_timeout(void *eloop_data, void *user_ctx);
void check_neighbor_discovery(struct p1905_managerd_ctx *ctx,
	unsigned char *al_mac, unsigned char *itf_mac);

#endif /* CMDU_MESSAGE_H */
