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

#ifndef P1905_DATABASE_H
#define P1905_DATABASE_H

#include <sys/queue.h>

#include "interface.h"
#include "data_def.h"
#include "os.h"
#include "wifi_utils.h"
#include "list.h"

#define ITF_NUM		56  //tri-band wifi + 4 eth + 1 wan
#define MAX_IF_NAME	20

#define BRIDGE_NUM      1
#define MAX_BSS_NUM 16
#define MAX_MID_QUEUE 10

/*define vendor specific data structure*/
struct p1905_vs_info
{
    unsigned char al_mac[ETH_ALEN];
};

typedef struct
{
    unsigned char target[ETH_ALEN];
    unsigned char type;

}link_metrics_rep;

typedef struct
{
    unsigned char target[ETH_ALEN];
    unsigned char type;

}link_metrics_query;

/*data structure of channel selection*/

struct GNU_PACKED config_bss_info
{
	unsigned char ifname[IFNAMSIZ];
	unsigned char mac[ETH_ALEN];
	unsigned char config_status; /*1 for success config; 0 for fail config*/
	unsigned char priority;
#ifdef MAP_R2
	WSC_CONFIG config;
#endif
};

struct GNU_PACKED operating_class
{
	unsigned char class_number;
	unsigned char max_tx_pwr;
	unsigned char non_operch_num;
	unsigned char non_operch_list[MAX_CH_NUM];
	struct dl_list entry;
};

struct GNU_PACKED radio_basic_capability
{
	unsigned char max_bss_num;
	unsigned char op_class_num;
	/*element is struct operating_class*/
	struct dl_list op_class_list;
};

/*this struct is used to store radio info;
  *the maximun number of radio is 3, include 2g, 5gl, 5gh
  */
struct GNU_PACKED radio_info
{
	unsigned char identifier[ETH_ALEN];
	unsigned char band;
	unsigned char dev_type;
	unsigned char bss_number;
	unsigned char trrigerd_autoconf;
	struct config_bss_info bss[MAX_BSS_NUM];
	unsigned char teared_down;             /*whether is it teared down by controller*/
	unsigned char config_status;
	struct radio_basic_capability basic_cap;
};

struct GNU_PACKED controller_info
{
	unsigned char local_ifname[IFNAMSIZ];    //local interface name that recv the controller
	unsigned char almac[ETH_ALEN];
	unsigned char mac_2g[ETH_ALEN];
	unsigned char mac_5g[ETH_ALEN];
	unsigned char supported_role_registrar;
	unsigned char supported_freq;
	unsigned int recv_ifid;
	unsigned char profile;
	unsigned char kibmib;
};

struct ap_ht_cap_db
{
	unsigned char identifier[ETH_ALEN];
	unsigned char tx_stream;
	unsigned char rx_stream;
	unsigned char sgi_20;
	unsigned char sgi_40;
	unsigned char ht_40;
	SLIST_ENTRY(ap_ht_cap_db) ap_ht_cap_entry;
};

struct ap_vht_cap_db
{
	unsigned char identifier[ETH_ALEN];
	unsigned short vht_tx_mcs;
	unsigned short vht_rx_mcs;
	unsigned char tx_stream;
	unsigned char rx_stream;
	unsigned char sgi_80;
	unsigned char sgi_160;
	unsigned char vht_8080;
	unsigned char vht_160;
	unsigned char su_beamformer;
	unsigned char mu_beamformer;
	SLIST_ENTRY(ap_vht_cap_db) ap_vht_cap_entry;
};

struct ap_he_cap_db
{
	unsigned char identifier[ETH_ALEN];
	unsigned char he_mcs_len;
	unsigned char he_mcs[12];
	unsigned char tx_stream;
	unsigned char rx_stream;
	unsigned char he_8080;
	unsigned char he_160;
	unsigned char su_bf_cap;
	unsigned char mu_bf_cap;
	unsigned char ul_mu_mimo_cap;
	unsigned char ul_mu_mimo_ofdma_cap;
	unsigned char dl_mu_mimo_ofdma_cap;
	unsigned char ul_ofdma_cap;
	unsigned char dl_ofdma_cap;
#ifdef MAP_R3
	unsigned char agent_role;
	unsigned char su_beamformee_status;
	unsigned char beamformee_sts_less80;
	unsigned char beamformee_sts_more80;
	unsigned char max_user_dl_tx_mu_mimo;
	unsigned char max_user_ul_rx_mu_mimo;
	unsigned char max_user_dl_tx_ofdma;
	unsigned char max_user_ul_rx_ofdma;
	unsigned char rts_status;
	unsigned char mu_rts_status;
	unsigned char m_bssid_status;
	unsigned char mu_edca_status;
	unsigned char twt_requester_status;
	unsigned char twt_responder_status;
#endif
	SLIST_ENTRY(ap_he_cap_db) ap_he_cap_entry;
};

#ifdef MAP_R3
struct ap_wf6_cap_db
{
	unsigned char he_mcs_len;
	unsigned char he_mcs[12];
	unsigned char tx_stream;
	unsigned char rx_stream;
	unsigned char he_8080;
	unsigned char he_160;
	unsigned char su_bf_cap;
	unsigned char mu_bf_cap;
	unsigned char ul_mu_mimo_cap;
	unsigned char ul_mu_mimo_ofdma_cap;
	unsigned char dl_mu_mimo_ofdma_cap;
	unsigned char ul_ofdma_cap;
	unsigned char dl_ofdma_cap;
	unsigned char agent_role;
	unsigned char su_beamformee_status;
	unsigned char beamformee_sts_less80;
	unsigned char beamformee_sts_more80;
	unsigned char max_user_dl_tx_mu_mimo;
	unsigned char max_user_ul_rx_mu_mimo;
	unsigned char max_user_dl_tx_ofdma;
	unsigned char max_user_ul_rx_ofdma;
	unsigned char rts_status;
	unsigned char mu_rts_status;
	unsigned char m_bssid_status;
	unsigned char mu_edca_status;
	unsigned char twt_requester_status;
	unsigned char twt_responder_status;
};

struct ap_wf6_role_db {
	unsigned char identifier[ETH_ALEN];
	unsigned char role_supp;
#ifdef MAP_R4_SPT
	unsigned char sr_mode;
#endif
	struct ap_wf6_cap_db wf6_role[2];
	SLIST_ENTRY(ap_wf6_role_db) ap_wf6_role_entry;
};
#endif /*MAP_R3*/

struct GNU_PACKED op_bss_db
{
	unsigned char bssid[ETH_ALEN];
	unsigned char ssid_len;
	unsigned char ssid[MAX_SSID_LEN];
	SLIST_ENTRY(op_bss_db) op_bss_entry;
};

struct GNU_PACKED operational_bss_db
{
	unsigned char identifier[ETH_ALEN];
	unsigned char oper_bss_num;
	unsigned char band;
	SLIST_ENTRY(operational_bss_db) oper_bss_entry;
	SLIST_HEAD(list_head_op_bss, op_bss_db) op_bss_head;
};

struct GNU_PACKED clients_db
{
	unsigned char mac[ETH_ALEN];
	unsigned int time;
	SLIST_ENTRY(clients_db) clients_entry;
	unsigned short frame_len;
	unsigned char frame[0];
};

struct GNU_PACKED associated_clients_db
{
	unsigned char bssid[ETH_ALEN];
	unsigned short assco_clients_num;
	SLIST_ENTRY(associated_clients_db) assoc_clients_entry;
	SLIST_HEAD(list_head_clients, clients_db) clients_head;
};

struct GNU_PACKED prefer_info_db
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char perference;
	unsigned char reason;
	SLIST_ENTRY(prefer_info_db) prefer_info_entry;
};

struct GNU_PACKED ch_prefer_db
{
	unsigned char identifier[ETH_ALEN];
	signed char tx_power_limit;
	unsigned char op_class_num;
	SLIST_ENTRY(ch_prefer_db) ch_prefer_entry;
	SLIST_HEAD(list_head_prefer_info, prefer_info_db) prefer_info_head;
};

struct GNU_PACKED restrict_db
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char min_fre_sep[MAX_CH_NUM];
	SLIST_ENTRY(restrict_db) restrict_entry;
};

struct GNU_PACKED oper_restrict_db
{
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class_num;
	SLIST_ENTRY(oper_restrict_db) oper_restrict_entry;
	SLIST_HEAD(list_head_restrict, restrict_db) restrict_head;
};

struct GNU_PACKED tx_power_limit_db
{
	unsigned char identifier[ETH_ALEN];
	signed char tx_power_limit;
	SLIST_ENTRY(tx_power_limit_db) tx_power_limit_entry;
};


enum MAP_PROFILE {
	MAP_PROFILE_REVD = 0,
	MAP_PROFILE_R1 = 1,
	MAP_PROFILE_R2,
	MAP_PROFILE_R3,
};
#ifdef MAP_R2
struct GNU_PACKED cac_opcap_db
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM]; //MAX of 5G Channels
	SLIST_ENTRY(cac_opcap_db) cac_opcap_entry;
};


struct GNU_PACKED cac_cap_db
{
	unsigned char cac_mode;
	unsigned char cac_interval[3];
	unsigned char op_class_num;
	SLIST_ENTRY(cac_cap_db) cac_cap_entry;
	SLIST_HEAD(list_head_cac_opcap, cac_opcap_db) cac_opcap_head;
};


struct GNU_PACKED cac_capability_db
{
	unsigned char identifier[ETH_ALEN];
	unsigned char cac_type_num;
	SLIST_ENTRY(cac_capability_db) cac_capab_entry;
	SLIST_HEAD(list_head_cac_cap, cac_cap_db) cac_capab_head;
};

struct GNU_PACKED radio_cac_capability_db
{
	unsigned char country_code[2];
	unsigned char radio_num;
	SLIST_HEAD(list_head_radio_cac_cap, cac_capability_db) radio_cac_capab_head;
};


struct GNU_PACKED ap_ext_cap_db
{
	unsigned char bssid[ETH_ALEN];
	unsigned int uc_tx;
	unsigned int uc_rx;
	unsigned int mc_tx;
	unsigned int mc_rx;
	unsigned int bc_tx;
	unsigned int bc_rx;
	SLIST_ENTRY(ap_ext_cap_db) ap_ext_cap_entry;
};


struct GNU_PACKED radio_metrics_db {
	unsigned char identifier[ETH_ALEN];
	unsigned char noise;
	unsigned char transmit;
	unsigned char receive_self;
	unsigned char receive_other;
	SLIST_ENTRY(radio_metrics_db) radio_metrics_entry;
};

struct GNU_PACKED radio_identifier_db {
	unsigned char identifier[ETH_ALEN];
	SLIST_ENTRY(radio_identifier_db) radio_identifier_entry;
};


#endif // #ifdef MAP_R2




/**
  *@support_radio: one byte bitmap to specify the support  radio; 1: 2.4g; 2: 5gl; 4: 5gh;
  */
struct GNU_PACKED ap_capability_db
{
	struct ap_capability ap_cap;
	//list to store ap ht capability for each radio
	SLIST_HEAD(list_head_ht_capability, ap_ht_cap_db) ap_ht_cap_head;
	//list to store ap vht capability for each radio
	SLIST_HEAD(list_head_vht_capability, ap_vht_cap_db) ap_vht_cap_head;

	SLIST_HEAD(list_head_he_capability, ap_he_cap_db) ap_he_cap_head;
	SLIST_HEAD(list_head_oper_bss, operational_bss_db) oper_bss_head;
	SLIST_HEAD(list_head_assoc_clients, associated_clients_db) assoc_clients_head;
	SLIST_HEAD(list_head_ch_prefer, ch_prefer_db) ch_prefer_head;
	SLIST_HEAD(list_head_oper_restrict, oper_restrict_db) oper_restrict_head;

#ifdef MAP_R2
	struct ap_r2_capability ap_r2_cap;
	//list to store ap extended capability for each bss
	SLIST_HEAD(list_head_ap_ext_capability, ap_ext_cap_db) ap_ext_cap_head;
	/*channel scan feature*/
	SLIST_HEAD(list_head_ch_scan_cap, channel_scan_cap_db) ch_scan_cap_head;
	/* DFS CAC */
	struct radio_cac_capability_db radio_cac_cap;
	//SLIST_HEAD(list_head_cac_capab, cac_capability_db) cac_capab_head;
#endif // #ifdef MAP_R2
#ifdef MAP_R3
	SLIST_HEAD(list_head_wf6_capability, ap_wf6_role_db) ap_wf6_cap_head;
#endif /*MAP_R3*/
};

struct GNU_PACKED sta_db {
	unsigned char mac[ETH_ALEN];
	SLIST_ENTRY(sta_db) sta_entry;
};

struct GNU_PACKED radio_policy_db {
	unsigned char identifier[ETH_ALEN];
	unsigned char steer_policy;
	unsigned char ch_util_thres;
	unsigned char rssi_thres;
	SLIST_ENTRY(radio_policy_db) radio_policy_entry;
};

struct GNU_PACKED steer_policy {
	unsigned char local_disallow_count;
	SLIST_HEAD(list_head_local_steer, sta_db) local_disallow_head;
	unsigned char btm_disallow_count;
	SLIST_HEAD(list_head_btm_steer, sta_db) btm_disallow_head;
	unsigned char radios;
	SLIST_HEAD(list_head_radio_policy, radio_policy_db) radio_policy_head;
};

struct GNU_PACKED metric_policy_db {
	unsigned char identifier[ETH_ALEN];
	unsigned char rssi_thres;
	unsigned char hysteresis_margin;
	unsigned char ch_util_thres;
	unsigned char sta_stats_inclusion;
	unsigned char sta_metrics_inclusion;
#ifdef MAP_R3
	unsigned char assoc_wf6_inclusion;
#endif
	SLIST_ENTRY(metric_policy_db) policy_entry;
};

struct GNU_PACKED metrics_policy {
	unsigned char report_interval;
	unsigned char radio_num;
	SLIST_HEAD(list_head_metric_policy, metric_policy_db) policy_head;
};

struct GNU_PACKED control_policy {
	unsigned char policy_cnt;
	SLIST_HEAD(list_head_control_policy, control_policy_db) policy_head;
};
struct GNU_PACKED control_policy_db {
	unsigned char bssid[ETH_ALEN];
	unsigned char assoc_control;
	unsigned short valid_period;
	unsigned char sta_list_count;
	SLIST_HEAD(list_head_sta_mac, sta_db) sta_head;
	SLIST_ENTRY(control_policy_db) policy_entry;
};

#ifdef MAP_R2
typedef unsigned char UINT8;
typedef unsigned short UINT16;

#define MAX_SSID_LEN	33
#define MAC_ADDR_LEN	6

enum map_nlmsg {
	NLMSG_GET_MAC_BY_SRC = 1,
	NLMSG_SET_ALMAC,
	NLMSG_DEFAULT_SETTING,
	NLMSG_SSID_TO_VID,
	NLMSG_SP_RULE,
	NLMSG_EXCEPT_INT,
	NLMSG_PERMMIT_MAC,
	NLMSG_STA_TO_SSID,
	NLMSG_ETH_INTF,
	NLMSG_DSCP,
	NLMSG_PEER_TYPE,
	NLMSG_PEER_DEV_INFO,
	NLMSG_CLEANUP_POLICY,
	NLMSG_DEV_VERSION
};

enum map_device_type {
	DEV_TYPE_UNKNOWN=0,
	DEV_TYPE_R1,
	DEV_TYPE_R2,
	DEV_TYPE_R3,
	DEV_TYPE_CLIENT
};

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned char UCHAR;

#define MSG_HEADER_SIZE 4

struct nlmsg_cmd {
	UINT8	cmd;
	UINT8	num;
	UINT16  bufsize;	/*size of buff*/
	UCHAR	buf[0];
};

struct ssid_to_vlan {
	UINT8 ssid[MAX_SSID_LEN];
	UINT16 vlanid;
};

struct default_setting {
	UINT16 primary_vid;
	UINT16 max_rule_num;
	UINT16 max_adv_rule_num;
	UINT16 max_dest_num;
	UINT8 default_pcp;
};

struct intf_type_info {
	UCHAR mac[MAC_ADDR_LEN];
	UINT8 type;
};

struct GNU_PACKED def_8021q_setting {
	unsigned short primary_vid;
	struct dft {
		unsigned char  Reserved:5;
		unsigned char  PCP:3;
	}dft;
	unsigned char updated;
};

struct GNU_PACKED traffic_separation_db {
	unsigned char  SSID_len;
	unsigned char  *SSID; /* lenofSSID octets */
	unsigned short  vid; /* 2 octets */
	SLIST_ENTRY(traffic_separation_db) policy_entry;
};

struct GNU_PACKED traffics_policy {
	unsigned char SSID_num;
	SLIST_HEAD(list_head_traffic_policy, traffic_separation_db) policy_head;
	unsigned char updated;
};

struct GNU_PACKED dest_mac_list {
	unsigned char  dest_addr[ETH_ALEN];
	SLIST_ENTRY(dest_mac_list) dmac_entry;
};

struct GNU_PACKED pf_bssid_list {
	unsigned char  bssid[ETH_ALEN];
	unsigned char  n; /* permitted dest address */
	SLIST_HEAD(list_head_dest_addr, dest_mac_list) dmac_head;
	SLIST_ENTRY(pf_bssid_list) bssid_entry;
};

struct GNU_PACKED pfiltering_policy {
	unsigned char  k; /* Number of BSSIDs.*/
	SLIST_HEAD(list_head_pfiltering_policy, pf_bssid_list) bssid_head;
};

struct GNU_PACKED unsuccess_assoc_policy {
	/*
	**	0: Do not report unsuccessful association attempts
	**	1: Report unsuccessful association attempts
	*/
	unsigned char report_switch;
	/* Maximum rate for reporting unsuccessful association attempts
	** (in attempts per minute)
	*/
	unsigned int report_rate;
};

struct GNU_PACKED ethernet_config_db {
	unsigned char  intfid[ETH_ALEN]; /* 6 ctets */
	struct eth {
		unsigned char  Reserved:6;
		unsigned char  type:2;
	}eth;
	SLIST_ENTRY(ethernet_config_db) policy_entry;
};

struct GNU_PACKED ethernets_policy {
	unsigned char inf_num;
	SLIST_HEAD(list_head_ethernet_policy, ethernet_config_db) policy_head;
};
#define MAX_NUM_TRANSPARENT_VID 128
struct GNU_PACKED transparent_vlan_setting {
	unsigned char num;
	unsigned short transparent_vids[MAX_NUM_TRANSPARENT_VID];
	unsigned char updated;
};
#endif // #ifdef MAP_R2

struct GNU_PACKED policy_config {
	struct steer_policy spolicy;
	struct metrics_policy mpolicy;
#ifdef MAP_R2
	struct traffics_policy tpolicy;
	struct def_8021q_setting setting;
	struct transparent_vlan_setting trans_vlan;
	struct ethernets_policy epolicy;
	struct pfiltering_policy fpolicy;
	struct unsuccess_assoc_policy unsuccess_assoc_policy;
#endif // #ifdef MAP_R2
};

struct GNU_PACKED bss_db {
	unsigned char bssid[ETH_ALEN];
	SLIST_ENTRY(bss_db) bss_entry;
};

struct GNU_PACKED esp_db {
	unsigned char ac;
	unsigned char format;
	unsigned char ba_win_size;
	unsigned char e_air_time_fraction;
	unsigned char ppdu_dur_target;
	SLIST_ENTRY(esp_db) esp_entry;
};

struct GNU_PACKED mrsp_db {
	unsigned char bssid[ETH_ALEN];
	unsigned char ch_util;
	unsigned short assoc_sta_cnt;
	SLIST_ENTRY(mrsp_db) mrsp_entry;
	unsigned char esp_cnt;
	SLIST_HEAD(list_head_esp, esp_db) esp_head;
};

struct GNU_PACKED stats_db {
	unsigned char mac[ETH_ALEN];
	unsigned int bytes_sent;
	unsigned int bytes_received;
	unsigned int packets_sent;
	unsigned int packets_received;
	unsigned int tx_packets_errors;
	unsigned int rx_packets_errors;
	unsigned int retransmission_count;
	SLIST_ENTRY(stats_db) stats_entry;
};

struct GNU_PACKED traffic_stats_db {
	unsigned char identifier[ETH_ALEN];
	SLIST_ENTRY(traffic_stats_db) traffic_stats_entry;
	unsigned char sta_cnt;
	SLIST_HEAD(list_head_stats, stats_db) stats_head;
};

struct GNU_PACKED metrics_db {
	unsigned char mac[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	unsigned int time_delta;
	unsigned int erate_downlink;
	unsigned int erate_uplink;
	unsigned char rssi_uplink;
	SLIST_ENTRY(metrics_db) metrics_entry;
};

struct GNU_PACKED link_metrics_db {
	unsigned char identifier[ETH_ALEN];
	SLIST_ENTRY(link_metrics_db) link_metrics_entry;
	unsigned char sta_cnt;
	SLIST_HEAD(list_head_metrics, metrics_db) metrics_head;
};

#ifdef MAP_R2
struct GNU_PACKED extended_metrics_db {
	unsigned char bssid[ETH_ALEN];
	unsigned int last_data_ul_rate;
	unsigned int last_data_dl_rate;
	unsigned int utilization_rx;
	unsigned int utilization_tx;
	SLIST_ENTRY(extended_metrics_db) metrics_entry;
};

struct GNU_PACKED sta_extended_metrics_db {
	unsigned char sta_mac[ETH_ALEN];
	unsigned char extended_metric_cnt;
	SLIST_ENTRY(sta_extended_metrics_db) sta_extended_metrics_entry;
	SLIST_HEAD(list_head_extended_metrics, extended_metrics_db) extended_metrics_head;
};
#endif // #ifdef MAP_R2

#ifdef MAP_R3
struct GNU_PACKED bh_sta_conf_db {
	unsigned char net_role;
	unsigned char akm;
	unsigned char ch_num;
	unsigned char ch[MAX_CH_NUM];
};
#endif // #ifdef MAP_R3

struct GNU_PACKED unlink_metrics_db {
	unsigned char mac[ETH_ALEN];
	unsigned char ch;
	unsigned int time_delta;
	unsigned char uplink_rssi;
	SLIST_ENTRY(unlink_metrics_db) unlink_metrics_entry;
};

struct GNU_PACKED unlink_metrics_info {
	unsigned char oper_class;
	unsigned char sta_num;
	SLIST_HEAD(list_head_unlink_metrics, unlink_metrics_db) unlink_metrics_head;
};

struct _1905_link_stat {
	struct tx_link_stat_rsp tx_link_stat;
	struct rx_link_stat_rsp rx_link_stat;
};

struct GNU_PACKED metrics_info{
	unsigned char metrics_query_cnt;
	SLIST_HEAD(list_head_metrics_query, bss_db) metrics_query_head;
	unsigned char metrics_rsp_cnt;
	SLIST_HEAD(list_head_metrics_rsp, mrsp_db) metrics_rsp_head;
	SLIST_HEAD(list_head_traffic_stats, traffic_stats_db) traffic_stats_head;
	SLIST_HEAD(list_head_link_metrics, link_metrics_db) link_metrics_head;
	unsigned char assoc_sta[ETH_ALEN];
	struct metrics_db assoc_sta_link_metrics;
	struct unlink_metrics_query *unlink_query;
	struct unlink_metrics_info unlink_info;
	struct beacon_metrics_query *bcn_query;
	struct beacon_metrics_rsp *bcn_rsp;
#ifdef MAP_R2
	unsigned int metric_collection_interval;
	// radio metric db
	SLIST_HEAD(list_head_radio_metrics, radio_metrics_db) radio_metrics_head;
	SLIST_HEAD(list_head_assoc_sta_extended_link_metrics, sta_extended_metrics_db) assoc_sta_extended_link_metrics_head;
	SLIST_HEAD(list_head_radio_identifier, radio_identifier_db) radio_identifier_head;
#endif // #ifdef MAP_R2
};

struct GNU_PACKED sta_info
{
	struct map_client_association_event sassoc_evt;
	struct client_info cinfo;
	struct client_capa_rep *pcli_rep;
	struct cli_steer_btm_event sbtm_evt;
};

struct GNU_PACKED ch_stat_info
{
	unsigned char identifier[ETH_ALEN];
	unsigned char code;
};
struct GNU_PACKED channel_status
{
	unsigned char ch_rsp_num;
	struct ch_stat_info info[0];
};

struct GNU_PACKED tx_metric_db {
	unsigned char mac[ETH_ALEN];
	unsigned char nmac[ETH_ALEN];
	unsigned short intf_type;
	unsigned char bridge_flag;
	unsigned int error_packet;
	unsigned int tx_packets;
	unsigned short mac_tpcap;
	unsigned short linkavl;
	unsigned short phyrate;
	SLIST_ENTRY(tx_metric_db) tx_metric_entry;
};

struct GNU_PACKED tx_link_metric_db {
	unsigned char almac[ETH_ALEN];
	unsigned char nalmac[ETH_ALEN];
	SLIST_ENTRY(tx_link_metric_db) tx_link_metric_entry;
	SLIST_HEAD(list_head_tx_metric, tx_metric_db) tx_metric_head;
};

struct GNU_PACKED rx_metric_db {
	unsigned char mac[ETH_ALEN];
	unsigned char nmac[ETH_ALEN];
	unsigned short intf_type;
	unsigned int error_packet;
	unsigned int rx_packets;
	unsigned char rssi;
	SLIST_ENTRY(rx_metric_db) rx_metric_entry;
};

struct GNU_PACKED rx_link_metric_db {
	unsigned char almac[ETH_ALEN];
	unsigned char nalmac[ETH_ALEN];
	SLIST_ENTRY(rx_link_metric_db) rx_link_metric_entry;
	SLIST_HEAD(list_head_rx_metric, rx_metric_db) rx_metric_head;
};


#ifdef MAP_R3
struct GNU_PACKED radio_r3_basic
{
	unsigned char bss_num[MAX_RADIO_NUM];
	unsigned char band_cap_r3[MAX_RADIO_NUM];
	unsigned char identifier[MAX_RADIO_NUM][ETH_ALEN];
};
#endif // #endif MAP_R3

struct agent_radio_config_context {
	unsigned char config_status;
	unsigned short m2_mid;
};
struct GNU_PACKED agent_radio_info {
	unsigned char identifier[ETH_ALEN];
	/*
	 * #define BAND_INVALID_CAP 0x00
	 * #define BAND_2G_CAP 0x01
	 * #define BAND_5GL_CAP  0x02
	 * #define BAND_5GH_CAP  0x04
	 * #define BAND_5G_CAP  0x06
	 * #define BAND_6G_CAP  0x10
	 */
	unsigned char band;
	struct agent_radio_config_context conf_ctx;
	unsigned char max_bss_num;
	unsigned char op_class_num;
	unsigned char doing_wsc;
};

struct GNU_PACKED agent_list_db
{
	unsigned char almac[ETH_ALEN];
	unsigned char profile;
	unsigned char radio_num;
	struct agent_radio_info ra_info[MAX_RADIO_NUM];
#ifdef MAP_R2
	struct ap_r2_capability r2_cap;
	SLIST_HEAD(list_head_ts_cap_agent, ts_cap_db) ts_cap_head;
#endif
#ifdef MAP_R3
	struct bh_sta_conf_db bh_sta_conf[MAX_RADIO_NUM];
#endif

	SLIST_HEAD(list_head_ch_prefer_agent, ch_prefer_db) ch_prefer_head;
	SLIST_HEAD(list_head_oper_restrict_agent, oper_restrict_db) oper_restrict_head;
	SLIST_HEAD(list_head_metrics_rsp_agent, mrsp_db) metrics_rsp_head;
	SLIST_HEAD(list_head_tx_metrics_agent, tx_link_metric_db) tx_metrics_head;
	SLIST_HEAD(list_head_rx_metrics_agent, rx_link_metric_db) rx_metrics_head;
	SLIST_ENTRY(agent_list_db) agent_entry;
	unsigned char wts_syn_done;
};

#define TX_MUL	BIT(0)
#define RX_MUL	BIT(1)
#define TX_UNI	BIT(2)
#define RX_UNI	BIT(3)
/*
trx_config:
bit 0-allow/disallow to tx multicast
bit 1-allow/disallow to rx multicast
bit 2-allow/disallow to tx unicast
bit 3-allow/disallow to rx unicast
*/
/*below is the data structure of local information*/
struct GNU_PACKED p1905_interface
{
    unsigned char mac_addr[ETH_ALEN];
	unsigned char if_name[IFNAMSIZ];
    unsigned short media_type;
	unsigned char is_wifi_ap;
	unsigned char is_wifi_sta;
	unsigned char identifier[ETH_ALEN];
	unsigned char is_radio_identifier;
    unsigned char *vs_info;
    unsigned char vs_info_length;
	unsigned char config_priority;
	unsigned char trx_config;
	unsigned char is_veth;
	unsigned char is_wan;
	unsigned char channel_freq;
	unsigned char channel_bw;
};

struct GNU_PACKED bridge_capabiltiy
{
   unsigned char interface_num;
   unsigned char *itf_mac_list;
};

struct GNU_PACKED non_p1905_neighbor_info
{
    unsigned char itf_mac_addr[ETH_ALEN];
	unsigned char port_index;
    LIST_ENTRY(non_p1905_neighbor_info) non_p1905nbr_entry;
};

struct GNU_PACKED non_p1905_neighbor
{
    unsigned char local_mac_addr[ETH_ALEN];
    unsigned short br_port_no;
    struct non_p1905_neighbor_info info;
    LIST_HEAD(list_head_nonp1905nbr, non_p1905_neighbor_info) non_p1905nbr_head;
};

struct GNU_PACKED p1905_neighbor_info
{
    unsigned char al_mac_addr[ETH_ALEN];
    unsigned char ieee802_1_bridge;
    LIST_ENTRY(p1905_neighbor_info) p1905nbr_entry;
};

struct GNU_PACKED p1905_neighbor
{
    unsigned char local_mac_addr[ETH_ALEN];
    struct p1905_neighbor_info info;
    LIST_HEAD(list_head_p1905nbr, p1905_neighbor_info) p1905nbr_head;
};

/*below is data structure for received topology message*/
struct GNU_PACKED topology_discovery_db
{
    unsigned char al_mac[6];
    unsigned char itf_mac[6];
    unsigned char receive_itf_mac[6];
    int time_to_live;
	int eth_port;
    LIST_ENTRY(topology_discovery_db) tpddb_entry;
};

struct GNU_PACKED non_p1905_neighbor_device_list_db
{
    unsigned char non_p1905_device_interface_mac[6];
    SLIST_ENTRY(non_p1905_neighbor_device_list_db) non_p1905_nbrdb_entry;
};

struct GNU_PACKED p1905_neighbor_device_db
{
    unsigned char p1905_neighbor_al_mac[6];
    unsigned char ieee_802_1_bridge_exist;
    SLIST_ENTRY(p1905_neighbor_device_db) p1905_nbrdb_entry;
};

struct GNU_PACKED device_info_db
{
    unsigned char mac_addr[6];
    unsigned short media_type;
    unsigned char vs_info_len;
    unsigned char *vs_info;
    struct p1905_neighbor_device_db p1905_dev;
    struct non_p1905_neighbor_device_list_db non_p1905_dev;
    SLIST_ENTRY(device_info_db) devinfo_entry;
    SLIST_HEAD(list_head_p1905nbrdb, p1905_neighbor_device_db) p1905_nbrdb_head;
    SLIST_HEAD(list_head_nonp1905nbrdb, non_p1905_neighbor_device_list_db) non_p1905_nbrdb_head;
};

struct GNU_PACKED device_bridge_capability_db
{
    unsigned char interface_amount;
    unsigned char *interface_mac_tuple;
    LIST_ENTRY(device_bridge_capability_db) brcap_entry;
};

struct GNU_PACKED topology_response_db
{
	unsigned char al_mac_addr[6];
	unsigned char support_service;
	unsigned char kibmib_byte_counter;
	unsigned int recv_ifid;
	struct device_info_db    dev_info_entry;
	struct device_bridge_capability_db    bridge_capability_entry;
	struct os_time last_seen;
	int eth_port;
	unsigned char valid;
	SLIST_ENTRY(topology_response_db) tprdb_entry;
	SLIST_HEAD(list_head_devinfo, device_info_db) devinfo_head;
	LIST_HEAD(list_head_brcap, device_bridge_capability_db) brcap_head;
	enum MAP_PROFILE profile;
};

/*data structure of topology database*/
struct GNU_PACKED p1905_topology_db
{
    struct topology_discovery_db dcv_entry;
    struct topology_response_db  drp_entry;
    SLIST_HEAD(list_head_tprdb, topology_response_db) tprdb_head;
    LIST_HEAD(list_head_tpddb, topology_discovery_db)tpddb_head;
};


struct GNU_PACKED topology_device_db
{
    unsigned char aldev_mac[6];
	unsigned short mid_list[MAX_MID_QUEUE];
	unsigned short mid_set;
	unsigned short recv_mid;
	int search_live_time;
    int time_to_live;
    SLIST_ENTRY(topology_device_db) tpdev_entry;
};

struct GNU_PACKED p1905_topodevice_db
{
	struct topology_device_db  dev_entry;
    SLIST_HEAD(list_head_tpdevdb, topology_device_db) tpdevdb_head;
};

struct GNU_PACKED neighbor_list_db
{
	unsigned char nalmac[ETH_ALEN];
	unsigned char nitf_mac[ETH_ALEN];
	SLIST_ENTRY(neighbor_list_db) neighbor_entry;
};

struct GNU_PACKED err_sta_mac_db
{
	unsigned char mac_addr[ETH_ALEN];
	SLIST_ENTRY(err_sta_mac_db) err_sta_mac_entry;
};


struct GNU_PACKED err_sta_db
{
	int err_sta_cnt;
	SLIST_HEAD(list_head_err_sta, err_sta_mac_db) err_sta_head;
};


#ifdef MAP_R2
/*channel scan feature*/
struct GNU_PACKED scan_opcap_db
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	SLIST_ENTRY(scan_opcap_db) scan_opcap_entry;
};

struct GNU_PACKED channel_scan_cap_db
{
	unsigned char identifier[ETH_ALEN];
	unsigned char boot_only;
	unsigned char scan_impact;
	unsigned int min_scan_interval;
	unsigned char op_class_num;
	SLIST_ENTRY(channel_scan_cap_db) ch_scan_cap_entry;
	SLIST_HEAD(list_head_scan_opcap, scan_opcap_db) scan_opcap_head;
};
struct GNU_PACKED ts_cap_db
{
	unsigned char identifier[ETH_ALEN];
	unsigned char ts_combined_fh;
	unsigned char ts_combined_bh;
	SLIST_ENTRY(ts_cap_db) ts_cap_entry;
};
#endif
#endif /*P1905_DATABASE_H*/
