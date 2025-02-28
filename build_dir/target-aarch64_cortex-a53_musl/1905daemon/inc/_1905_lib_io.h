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

#ifndef _1905_LIB_IO_H
#define _1905_LIB_IO_H
#include "p1905_managerd.h"

int _1905_set_wireless_setting(struct p1905_managerd_ctx* ctx);

int _1905_wait_parse_spec_event(struct p1905_managerd_ctx* ctx, unsigned char* buf, int len, unsigned char event_type, long sec, long usec);

int _1905_event_notify(struct p1905_managerd_ctx *ctx, unsigned short event_id,
	unsigned char *peer_al_mac, unsigned short mid);

void _1905_set_steering_setting(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_set_client_assoc_ctrl(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_set_map_policy_config(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

int _1905_notify_ap_metrics_query(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned short mid
#ifdef MAP_R2
, unsigned char periodic
#endif
);

int _1905_notify_bh_steering_req(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

int _1905_notify_assoc_sta_link_metrics_query(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned short mid);

int _1905_notify_link_metrics_query(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned short mid);

void _1905_notify_ap_metrics_response(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

int _1905_notify_unassoc_sta_metrics_query(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

int _1905_set_radio_tear_down(struct p1905_managerd_ctx* ctx, unsigned char* radio_identifier);

void _1905_notify_beacon_metrics_query(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_controller_found(struct p1905_managerd_ctx* ctx);

int _1905_notify_channel_selection_req(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned short mid);

int _1905_notify_channel_preference_query(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned short mid);

void _1905_notify_channel_preference_report(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_client_steering_btm_report(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_steering_complete(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_link_metrics_response(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_assoc_sta_link_metric_rsp(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_unassoc_sta_link_metric_rsp(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_bcn_metric_rsp(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_bh_steering_rsp(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_combined_infra_metrics(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_vendor_specific_message(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_topology_rsp_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac, unsigned char* local_if_mac);

void _1905_notify_renew_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac, unsigned char renew_band);

void _1905_notify_autoconfig_search_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_autoconfig_rsp_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_topology_notification_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_raw_data(struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned short len);

void _1905_notify_ap_capability_report_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_ch_selection_rsp_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_operating_channel_report_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_client_capability_report_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

void _1905_notify_higher_layer_data_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

int _1905_update_bss_info_per_radio(struct p1905_managerd_ctx *ctx, struct radio_info *rinfo);

void _1905_notify_combined_infrastructure_metrics_query(struct p1905_managerd_ctx* ctx,
	unsigned char* al_mac, unsigned char *peer_almac);

void _1905_notify_switch_status(struct p1905_managerd_ctx* ctx, unsigned char status);
int _1905_notify_error_code_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);
int _1905_flash_out_config(struct p1905_managerd_ctx* ctx);

#ifdef MAP_R2
/*channel scan feature*/
int _1905_notify_ch_scan_req(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

int _1905_notify_ch_scan_rep(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

int _1905_notify_tunneled_msg(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

int _1905_notify_assoc_status_notification_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

int _1905_notify_cac_request_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

int _1905_notify_cac_terminate_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);

int _1905_notify_client_disassociation_stats_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);
int _1905_notify_backhaul_sta_cap_report_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);
void _1905_notify_ts_setting(struct p1905_managerd_ctx *ctx, unsigned char num,
	struct ssid_2_vid_mapping *mapping);
void _1905_notify_transparent_vlan_setting(struct p1905_managerd_ctx *ctx);
int _1905_notify_failed_connection_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);
void map_r2_notify_ts_cap(struct p1905_managerd_ctx *ctx, unsigned char *almac,
	struct ts_cap_db *ts_cap);
#endif // #ifdef MAP_R2

#ifdef MAP_R3
void map_notify_standard_sp_rules(struct p1905_managerd_ctx *ctx,
	unsigned char *almac, unsigned char *buf, unsigned short len);
void map_notify_akm_suit_cap(struct p1905_managerd_ctx *ctx, unsigned char *almac,
	unsigned char *buf, unsigned short len);
void map_notify_1905_secure_cap(struct p1905_managerd_ctx *ctx, unsigned char *almac,
	unsigned char *buf, unsigned short len);
/* dpp */
int _1905_notify_direct_encap_dpp_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);
int _1905_notify_proxied_encap_dpp_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);
int _1905_notify_dpp_bootstraping_uri_notification_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);
int _1905_notify_dpp_cce_indication_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);
int _1905_notify_dpp_bootstraping_uri_query_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);
int _1905_notify_chirp_notification_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac);
void _1905_notify_autoconfig_search(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned char peer_profile, unsigned char recv_if_type);
void _1905_notify_1905_security(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned char security_1905);
void _1905_notify_onboarding_result(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned char result, unsigned char *hash, unsigned short len);

#endif // #ifdef MAP_R3
int _1905_send_sync_recv_buf_size_rsp(struct p1905_managerd_ctx* ctx, unsigned short change_len, unsigned char rsp);

#endif
