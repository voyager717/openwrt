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

/* this header file is uesed to define the message between wapp and 1905 deamon
 */

#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <linux/if_ether.h>
#include <net/if.h>

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

#ifdef MAP_6E_SUPPORT
#define MAX_CH_NUM 59
#else
#define MAX_CH_NUM 13
#endif
#define MAX_ELEMNT_NUM 4
#define MAX_SSID_LEN 33

/*MAPD LIB CMD*/
enum {
	LIB_MAP_MIN_CMD=0x0,
	/*add new mapcmd below this line*/

	LIB_MAP_BH_READY=0x01,
	LIB_WPS_CONFIG_STATUS,// not use
	LIB_RADIO_BASIC_CAP,
	LIB_CHANNLE_PREFERENCE,
	LIB_RADIO_OPERATION_RESTRICTION,
	LIB_DISCOVERY,// not use
	LIB_CLIENT_NOTIFICATION,
	LIB_AP_CAPABILITY,
	LIB_AP_HT_CAPABILITY,
	LIB_AP_VHT_CAPABILITY,
	LIB_AP_HE_CAPABILITY,
	LIB_AP_OP_BSS=0x0C,//not use
	LIB_ASSOC_CLI,// not use
	LIB_CHN_SEL_RSP,// not use
	LIB_TOPOQUERY,// not use
	LIB_OPERBSS_REPORT,
	LIB_CLI_CAPABILITY_REPORT,
	LIB_1905_CMDU_REQUEST,
	LIB_CLI_STEER_BTM_REPORT,
	LIB_STEERING_COMPLETED,
	LIB_1905_READ_BSS_CONF_REQUEST,  /* for wts controller bss_info ready case */
	LIB_1905_READ_BSS_CONF_AND_RENEW,
	LIB_AP_METRICS_INFO,
	LIB_GET_WSC_CONF, //not use
	LIB_1905_READ_1905_TLV_REQUEST, /* for wts send 1905 data ready case */
	LIB_BACKHAUL_STEER_RSP,
	LIB_ALL_ASSOC_STA_TRAFFIC_STATS,
	LIB_ONE_ASSOC_STA_TRAFFIC_STATS,// not use
	LIB_ALL_ASSOC_STA_LINK_METRICS,
	LIB_ONE_ASSOC_STA_LINK_METRICS,
	LIB_TX_LINK_STATISTICS,
	LIB_RX_LINK_STATISTICS,
	LIB_UNASSOC_STA_LINK_METRICS,
	LIB_OPERATING_CHANNEL_REPORT,
	LIB_BEACON_METRICS_REPORT,
	LIB_1905_REQ,
	LIB_AP_LINK_METRIC_REQ, ////not used
	LIB_OPERATING_CHANNEL_INFO,   //not used
	LIB_AP_CAPABLILTY_QUERY=0x61,// not use
	LIB_CLI_CAPABLILTY_QUERY,// not use
	LIB_CH_SELECTION_REQUEST,// not use
	LIB_CLI_STEER_REQUEST,// not use
	LIB_CH_PREFER_QUERY,// not use
	LIB_POLICY_CONFIG_REQUEST,// not use
	LIB_CLI_ASSOC_CNTRL_REQUEST,// not use
	LIB_OP_CHN_RPT=0x8F,// not use
	LIB_STA_RSSI,// not use
	LIB_STA_STAT,// not use
	LIB_NAC_INFO,// not use
	LIB_STA_BSSLOAD,// not use
	LIB_TXPWR_CHANGE,// not use
	LIB_CSA_INFO,// not use
	LIB_APCLI_UPLINK_RSSI,// not use
	LIB_BSS_STAT_CHANGE,// not use
	LIB_BSS_LOAD_CROSSING,// not use
	LIB_APCLI_ASSOC_STAT_CHANGE,// not use
	LIB_STA_CNNCT_REJ_INFO,// not use
	LIB_UPDATE_PROBE_INFO,// not use
	LIB_WIRELESS_INF_INFO,// not use
	LIB_SCAN_RESULT,// not use
	LIB_SCAN_DONE,// not use
	LIB_MAP_VEND_IE_CHANGED,// not use
	LIB_ALL_ASSOC_TP_METRICS,// not use
	LIB_AIR_MONITOR_REPORT,// not use
	LIB_MAP_BH_CONFIG,// not use
#ifdef MAP_R2
	LIB_CHANNEL_SCAN_CAPAB,  //MAP R2
	LIB_CHANNEL_SCAN_REPORT, // not use
	LIB_ASSOC_STATUS_NOTIFICATION,// not use
	LIB_TUNNELED_MESSAGE,// not use
	LIB_CAC_CAPAB,
	LIB_CAC_COMPLETION_REPORT,// not use
	LIB_R2_AP_CAP,
	LIB_METRIC_REP_INTERVAL_CAP,
	LIB_AP_EXTENDED_METRICS_INFO,
	LIB_RADIO_METRICS_INFO,
	LIB_ASSOC_STA_EXTENDED_LINK_METRICS,
#endif // #ifdef MAP_R2
#ifdef MAP_R3
	LIB_SET_1905_CONNECTOR,
	LIB_SET_BSS_CONNECTOR,
	LIB_SET_CHIRP_VALUE,

	LIB_AP_WF6_CAPABILITY,
	LIB_SET_GTK_REKEY_INTERVAL,
	LIB_SET_R3_ONBOARDING_TYPE,
#endif /*MAP_R3*/
#ifdef MAP_R3_DE
	LIB_DEV_INVEN_TLV = 0xf6,
#endif
	LIB_WIRELESS_INTERFACE_INFO,
	LIB_CLEAR_SWITCH_TABLE,
	LIB_SYNC_RECV_BUF_SIZE,
	LIB_SYNC_RECV_BUF_SIZE_RSP,
	/*add new map LIB cmd before this line*/
	LIB_MAP_MAX_EVENT,
};

/*wapp user req command*/
enum {
	SET_REGITER_EVENT = 0x01,

	WAPP_USER_MIN_CMD = 0x80,
	/*add wapp user cmd below this line*/

	WAPP_USER_GET_CLI_CAPABILITY_REPORT = 0x81,
	WAPP_USER_SET_WIRELESS_SETTING,
	WAPP_USER_GET_RADIO_BASIC_CAP,
	WAPP_USER_GET_AP_CAPABILITY,
	WAPP_USER_GET_AP_HT_CAPABILITY,
	WAPP_USER_GET_AP_VHT_CAPABILITY,
	WAPP_USER_GET_SUPPORTED_SERVICE,
	WAPP_USER_GET_SEARCHED_SERVICE,
	WAPP_USER_GET_ASSOCIATED_CLIENT,
	WAPP_USER_GET_RA_OP_RESTRICTION,
	WAPP_USER_GET_CHANNEL_PREFERENCE,
	WAPP_USER_SET_CHANNEL_SETTING,
	WAPP_USER_GET_OPERATIONAL_BSS,
	WAPP_USER_SET_STEERING_SETTING,
	WAPP_USER_SET_ASSOC_CNTRL_SETTING,
	WAPP_USER_SET_LOCAL_STEER_DISALLOW_STA,
	WAPP_USER_SET_BTM_STEER_DISALLOW_STA,
	WAPP_USER_SET_RADIO_CONTROL_POLICY,
	WAPP_USER_GET_AP_METRICS_INFO,
	WAPP_USER_MAP_CONTROLLER_FOUND,
	WAPP_USER_SET_BACKHAUL_STEER,
	WAPP_USER_SET_METIRCS_POLICY,
	WAPP_USER_GET_ASSOC_STA_TRAFFIC_STATS,
	WAPP_USER_GET_ONE_ASSOC_STA_TRAFFIC_STATS,
	WAPP_USER_GET_ASSOC_STA_LINK_METRICS,
	WAPP_USER_GET_ONE_ASSOC_STA_LINK_METRICS,
	WAPP_USER_GET_TX_LINK_STATISTICS,
	WAPP_USER_GET_RX_LINK_STATISTICS,
	WAPP_USER_GET_UNASSOC_STA_LINK_METRICS,
	WAPP_USER_SET_RADIO_TEARED_DOWN,
	WAPP_USER_SET_BEACON_METRICS_QRY,
	WAPP_USER_SET_TOPOLOGY_INFO,
	WAPP_USER_SET_RADIO_RENEW,
	WAPP_USER_GET_OPERATING_CHANNEL_INFO,
	WAPP_USER_GET_AP_HE_CAPABILITY,
	WAPP_USER_FLUSH_ACL,
	WAPP_USER_GET_BSSLOAD,
	WAPP_USER_GET_RSSI_REQ,
	WAPP_USER_SET_WHPROBE_REQ,
	WAPP_USER_SET_NAC_REQ,
	WAPP_USER_SET_DEAUTH_STA,
	WAPP_USER_SET_VENDOR_IE,
	WAPP_USER_GET_APCLI_RSSI_REQ,
	WAPP_USER_GET_WIRELESS_INF_INFO,
	WAPP_USER_SET_ADDITIONAL_BH_ASSOC,
	WAPP_USER_ISSUE_SCAN_REQ,
	WAPP_USER_GET_SCAN_RESULT,
	WAPP_USER_SEND_NULL_FRAMES,
	WAPP_USER_GET_ALL_ASSOC_TP_METRICS,
	WAPP_USER_SET_AIR_MONITOR_REQUEST,

	WAPP_USER_SET_ENROLLEE_BH,
	WAPP_USER_SET_BSS_ROLE,
	WAPP_USER_TRIGGER_WPS,
	WAPP_USER_ISSUE_APCLI_DISCONNECT,
	WAPP_USER_SET_BH_WIRELESS_SETTING,
	WAPP_USER_SET_BH_PRIORITY,
	WAPP_USER_SET_BH_CONNECT_REQUEST,
	WAPP_USER_GET_BH_WIRELESS_SETTING,
	WAPP_USER_SET_TX_POWER_PERCENTAGE,
	WAPP_USER_SET_OFF_CH_SCAN_REQ,
	WAPP_USER_SET_NET_OPT_SCAN_REQ,
	WAPP_NEGOTIATE_ROLE,
	WAPP_UPDATE_MAP_DEVICE_ROLE,
	WAPP_USER_SET_AVOID_SCAN_CAC,
	WAPP_USER_GET_RSSI_OFFSETS,
	WAPP_USER_SET_TRAFFIC_SEPARATION_BH_SETTING,
	WAPP_USER_SET_TRAFFIC_SEPARATION_FH_SETTING,
	/*add wapp user cmd above this line*/
	WAPP_USER_MAX_CMD,
};

typedef enum {
	SHOW_1905_REQ,
	SET_1905_DBG_LV_REQ,
} WAPP_1905_REQ_ID;

typedef enum {
	SYNC,
	ASYNC,
} CMD_EVENT_TYPE;

struct GNU_PACKED cmd_to_wapp
{
	unsigned short type;
	unsigned char role;             /*0-indicate this message is from agent, 1-indicate this message is from controller*/
	unsigned short length;
	unsigned char band;
	unsigned char bssAddr[6];
	unsigned char staAddr[6];
	unsigned char body[0];
};

struct GNU_PACKED msg
{
	unsigned short type;
	unsigned short mid;
	unsigned short length;
	unsigned char buffer[0];
};

struct _map_cmd_event
{
	unsigned short cmd;
	unsigned short event;
	CMD_EVENT_TYPE type;
	unsigned char (*match_func)(struct msg*, struct cmd_to_wapp* );
};


struct GNU_PACKED wapp_1905_req
{
	unsigned char  id;
	unsigned short value;
};

typedef struct GNU_PACKED _inf_info{
	unsigned char ifname[IFNAMSIZ];
	unsigned char mac_addr[ETH_ALEN];
} inf_info;
/**
  *@type: backhaul type, 0-eth; 1-wifi
  *@ifname: backhaul inf name (with link ready)
  *@mac_addr: mac addr of the backhaul interface
  */
struct GNU_PACKED bh_link_info
{
	unsigned char type;
	unsigned char ifname[IFNAMSIZ]; /*  */
	unsigned char mac_addr[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	unsigned char trigger_autconf;
};
/**
  *@type: channel info,
  *@ifname: backhaul inf name (with link ready)
  *@mac_addr: mac addr of the backhaul interface
  */
struct GNU_PACKED channel_bw_info
{
	unsigned char iface_addr[ETH_ALEN];
	unsigned char channel_bw;
	unsigned char channel_num;
};

/**
  *@identifier: radio unique identifier of the radio requesting config settings
  *@band: 0x00-2.4g 0x01-5g
  *@dev_type: 1-wifi ap; 2-wifi sta
  *@num_of_inf: number of interface under this radio for getting conf
  *@inf_info: ifname, e.g ra0/ra1/apcli0; mac_addr: mac addr of this interface
  */

struct GNU_PACKED wps_get_config
{
	unsigned char identifier[ETH_ALEN];
	unsigned char band;
	unsigned char dev_type;
	unsigned char num_of_inf;
	inf_info      inf_data[0];
};
/**
  *@identifier: radio unique identifier of the radio for which capabilities are reported
  *@op_class:
  *@channel:
  *@power:
  */
struct GNU_PACKED ch_config_info
{
	unsigned char identifier[ETH_ALEN];
	unsigned char ifname[IFNAMSIZ];
	unsigned char op_class;
	unsigned char channel;
	signed char power;
};

struct GNU_PACKED channel_setting
{
	unsigned char almac[ETH_ALEN];
	unsigned char ch_set_num;
	struct ch_config_info chinfo[0];
};

struct GNU_PACKED ch_rep_info {
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class;
	unsigned char channel;
	signed char tx_power;
};

#ifdef MAP_R4_SPT
struct GNU_PACKED ap_spt_reuse_resp {
	unsigned char identifier[ETH_ALEN];
	unsigned char partial_bss_color;
	unsigned char bss_color;
	unsigned char hesiga_spa_reuse_val_allowed;
	unsigned char srg_info_valid;
	unsigned char nonsrg_offset_valid;
	unsigned char psr_disallowed;
	unsigned char nonsrg_obsspd_max_offset;
	unsigned char srg_obsspd_min_offset;
	unsigned char srg_obsspd_max_offset;
	unsigned char srg_bss_color_bitmap[8];
	unsigned char srg_partial_bssid_bitmap[8];
	unsigned char ngh_bss_color_in_bitmap[8];
};

struct GNU_PACKED spt_reuse_report {
	unsigned char spt_rep_num;
	struct ap_spt_reuse_resp spt_reuse_report[0];
};
#endif

struct GNU_PACKED channel_report
{
	unsigned char ch_rep_num;
	struct ch_rep_info info[0];
};

struct GNU_PACKED tx_power_percentage_setting {
	unsigned char bandIdx;
	unsigned char tx_power_percentage;
};

/**
  * this structure belongs to struct ap_radio_basic_cap
  * @op_class: specifying the global operating class in which the subsequent channel list is valid
  * @max_tx_pwr: maximum transmit power EIRP that this radio is capable of transmitting in the
  * current regulatory domain for the operating class
  * @non_operch_num: number of statically non-operable channels in the operating class
  * @non_operch_list: statically non-operable channels in the operating class
  */
struct GNU_PACKED radio_basic_cap
{
	unsigned char op_class;
	unsigned char max_tx_pwr;
	unsigned char non_operch_num;
	unsigned char non_operch_list[MAX_CH_NUM];
};
/**
  * @identifier: radio unique identifier of the radio for which capabilities are reported
  * @max_bss_num: maximum number of bss supported by this radio
  * @op_class_num: operation class number
  * @opcap: a pointer to struct radio_basic_cap
  */
struct GNU_PACKED ap_radio_basic_cap
{
	unsigned char identifier[ETH_ALEN];
	unsigned char max_bss_num;
	unsigned char band;
	int wireless_mode;
	unsigned char op_class_num;
	struct radio_basic_cap opcap[0];
};

/**
  * @bssid: MAC address of local interface(equal to BSSID) operating on the radio
  */
struct GNU_PACKED op_bss_cap
{
	unsigned char bssid[ETH_ALEN];
	unsigned char ssid_len;
	unsigned char ssid[MAX_SSID_LEN];
	unsigned char map_vendor_extension;
	unsigned int auth_mode;
	unsigned int enc_type;
	unsigned char key_len;
	unsigned char key[64 + 1];
};
/**
  * @identifier: radio unique identifier of a radio
  * @identifier: the band of the radio
  * @oper_bss_num: number of bss(802.11 local interfaces) currently operating on the radio
  */
struct GNU_PACKED oper_bss_cap
{
	unsigned char identifier[ETH_ALEN];
	unsigned char oper_bss_num;
	unsigned char band;
	struct op_bss_cap cap[0];
};

#define BIT_BH_STA BIT(7)
#define BIT_BH_BSS BIT(6)
#define BIT_FH_BSS BIT(5)
#define BIT_TEAR_DOWN BIT(4)

#define OPER_BSS_CAP_LEN 8
/**
  * @ifname: the radio interface name to be config
  * @num: number of wireless setting, can be 1~16
  * @Ssid: the ssid of the bss
  * @AuthMode:
  * @EncrypType:
  * @WPAKey:
  * @map_vendor_extension: map vendor extentsion in M2, 1 byte
  */
struct GNU_PACKED wireless_setting {
	unsigned char mac_addr[ETH_ALEN];
	unsigned char	Ssid[32 + 1];
	unsigned short  AuthMode;
	unsigned short	EncrypType;
	unsigned char	WPAKey[64 + 1];
	unsigned char map_vendor_extension;    /*store MAP controller's Muiti-AP Vendor Extesion value in M2*/
	unsigned char hidden_ssid;
#ifdef MAP_R3
	/*
	** cred_len is for BH, ext_cred_len is for FH, which support both FH and BH
	** cred_len is for BH or FH which support only FH or BH
	*/
	unsigned short cred_len;
	unsigned short ext_cred_len;
	unsigned char cred[0];
#endif
};

struct GNU_PACKED wsc_config{
	unsigned char num;
	struct wireless_setting setting[0];
};


/**
  * @sta_report_on_cop(0 or 1): cap of unassociated sta link metrics reporting on the channels
  * its BSSs arecurrently operating on
  * @sta_report_not_cop(0 or 1): cap of unassociated sta link metrics reporting on the channels
  * its BSSs are not currently operating on
  * @rssi_steer(0 or 1): cap of Agent-initiated RSSI-based Steering
  */
struct GNU_PACKED ap_capability
{
	unsigned char sta_report_on_cop;
	unsigned char sta_report_not_cop;
	unsigned char rssi_steer;
};


/**
  * @identifier: radio unique identifier of the radio for witch HT capabilities are reported
  * @tx_stream: maximum number of supported Tx spatial streams
  * @rx_stream: maximum number of supported Rx spatial streams
  * @sgi_20: short GI support for 20 MHz
  * @sgi_40: short GI support for 40 MHz
  * @ht_40: HT support for 40MHz
  */
struct GNU_PACKED ap_ht_capability
{
	unsigned char identifier[ETH_ALEN];
	unsigned char tx_stream;
	unsigned char rx_stream;
	unsigned char sgi_20;
	unsigned char sgi_40;
	unsigned char ht_40;
	unsigned char band;
};



/**
  * @identifier: radio unique identifier of the radio for witch HT capabilities are reported
  * @vht_tx_mcs: supported VHT Tx MCS; set to Tx VHT MCS Map field
  * @vht_rx_mcs: supported VHT Rx MCS; set to Rx VHT MCS Map field
  * @tx_stream: maximum number of supported Tx spatial streams
  * @rx_stream: maximum number of supported Rx spatial streams
  * @sgi_80: short GI support for 80 MHz
  * @sgi_160: short GI support for 160 MHz and 80+80MHz
  * @vht_8080: VHT support for 80+80MHz
  * @vht_160: VHT support for 160MHz
  * @su_beamformer: SU Beamformer capable
  * @mu_beamformer: MU Beamformer capable
  */
struct GNU_PACKED ap_vht_capability
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
	unsigned char band;
};

struct GNU_PACKED ap_he_capability
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
#endif //MAP_R3
};


/**
  * this structure belongs to struct ch_prefer
  * @op_class: specifying the global operating class in which the subsequent channel list is valid
  * @ch_num: valid channel num belongs to op_class
  * @ch_list: channel list belongs to op_class
  * @perference: indicate a preference value for the channels in the channel list
  * @reason: indicate reason for the preference
  */
struct GNU_PACKED prefer_info
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char perference;
	unsigned char reason;
};
/**
  * @identifier: radio unique identifier of the radio for witch channel preferences are reported
  * @op_class_num: operation class number
  * @opinfo: a pointer to struct prefer_info
  */
struct GNU_PACKED ch_prefer
{
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class_num;
	struct prefer_info opinfo[0];
};
/**
  * this structure belongs to struct restriction
  * @op_class: specifying the global operating class in which the subsequent channel list is valid
  * @ch_num: valid channel num belongs to op_class
  * @ch_list: channel list belongs to op_class
  * @fre_separation: the minimum frequency separation(in multiples of 10 MHz) that this radio
  * would require when operationg on the above channel number between the center frequency of
  * that channel and the center operating requency of another radio(operating simultaneous TX/RX)
  * of the agent
  */
struct GNU_PACKED restrict_info
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char fre_separation[MAX_CH_NUM];
};
/**
  * @identifier: radio unique identifier of the radio for witch radio operation restrictions are reported
  * @op_class_num: operation class number
  * @opinfo: a pointer to struct restrict_info
  */
struct GNU_PACKED restriction
{
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class_num;
	struct restrict_info opinfo[0];
};

/**
  * @sta_mac: the MAC address of the client
  * @bssid: the BSSID of the BSS operated by the MAP agent for which the event has occurred
  * @assoc_evt: 1 for client has joined the BSS; 0 for client has left the BSS
  * @assoc_time: the time of the 802.11 client's last association to this Multi-AP device. unit jiffies.
  * @assoc_req: association request frame body
  */
struct GNU_PACKED map_client_association_event
{
	unsigned char sta_mac[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	unsigned char assoc_evt;
	unsigned int assoc_time;
	unsigned char is_APCLI;
	unsigned short reason;
	unsigned short assoc_req_len;
	unsigned char assoc_req[0];
};

/*In case of AX nss can be different for different supported BWs.
So map requires NSS of all the supported BWs as in the HE cap info*/
struct GNU_PACKED map_he_nss{
        unsigned short nss_80:2;
        unsigned short nss_160:2;
        unsigned short nss_8080:2;
};

struct GNU_PACKED map_priv_cli_cap
{
#ifdef RT_BIG_ENDIAN
	unsigned short reserve:6;
	unsigned short mbo_capable:1;
	unsigned short rrm_capable:1;
	unsigned short btm_capable:1;
	unsigned short nss:2;
	unsigned short phy_mode:3;
	unsigned short bw:2;
	struct map_he_nss nss_he;
#else
	unsigned short bw:2;
	unsigned short phy_mode:3;
	unsigned short nss:2;
	unsigned short btm_capable:1;
	unsigned short rrm_capable:1;
	unsigned short mbo_capable:1;
	unsigned short reserve:6;
	struct map_he_nss nss_he;
#endif
};

struct GNU_PACKED client_association_event
{
	struct map_priv_cli_cap cli_caps;
	struct map_client_association_event map_assoc_evt;
};

/**
  * @bssid: the BSSID of a BSS
  * @sta_mac: the MAC address of the client
  */
struct GNU_PACKED client_info
{
	unsigned char bssid[ETH_ALEN];
	unsigned char sta_mac[ETH_ALEN];
};
/**
  * @result: result code for the client capability report message
  * @length: the length of frame body
  * @body: the frame body of the most recently received (re)association request frame
  * from this client. if result code is not equal to 0, this field is omitted
  */
struct GNU_PACKED client_capa_rep
{
	unsigned char result;
	unsigned short length;
	unsigned char body[0];
};


/**
  * @dest_al_mac: dest al mac
  * @type: 1905 frame type, e.g High layer Data Message 0x8018
  * @len: the length frame body, for High layer Data Message, it must not be 0,
  	       for other type of 1905 cmdu(e.g topology discovery, there's no frame content, it may be 0)
  * @body: frame body
  */
struct GNU_PACKED _1905_cmdu_request
{
	unsigned char dest_al_mac[ETH_ALEN];
	unsigned short type;
	unsigned short len;
	unsigned char body[0];
};

/**
  * @target_bssid: indicates a target BSSID for steering. wildcard BSSID is represented by ff:ff:ff:ff:ff:ff
  * @op_class: target BSS operating class
  * @channel: target BSS channel number for channel on which the target BSS is transmitting beacon frames
  * @sta_mac: sta mac address for which the steering request applies. if sta_count of struct steer_request
  * is 0, then this field is invalid
  */
struct GNU_PACKED target_bssid_info {
	unsigned char target_bssid[ETH_ALEN];
	unsigned char op_class;
	unsigned char channel;
	unsigned char sta_mac[ETH_ALEN];
};

/**
  * @assoc_bssid: unique identifier of the source BSS for whirch the steering request applies
  * (i.e. BSS that the STAs specified in the request are currently associated with)
  * @request_mode: 0: request is a steering opportunity; 1: request is a steering mandate to
  * trigger steering for specific client STA(s)
  * @btm_disassoc_immi: BTM Disassociation imminent bit
  * @btm_abridged: BTM Abridged bit
  * @steer_window: steering opportunity window. time period in seconds(from reception of the
  * steering request message)for whitch the request is valid.
  * @btm_disassoc_timer: BTM Disassociation timer. time period in TUs for disassociation timer
  * in BTM request
  * @sta_count: sta list count. k=0: steering request applies to all associated STAs in the BSS per
  * policy setting; k>0: steering request applies to specific STAs specific by STA MAC address(es)
  * @target_bssid_count: target BSSID list count. only valid when request_mode is set to 1.
  * m=1: the same target BSSID is indicated for all specified STAs;m=k: an individual target BSSID
  * is indicated for each apecified STA(in same order)
  * @steer_info: contains k sta mac address and m target bssid info
  */
struct GNU_PACKED steer_request {
	unsigned char assoc_bssid[ETH_ALEN];
	unsigned char request_mode;
	unsigned char btm_disassoc_immi;
	unsigned char btm_abridged;
	unsigned short steer_window;
	unsigned short btm_disassoc_timer;
	unsigned char sta_count;
	unsigned char target_bssid_count;
	struct target_bssid_info info[0];
};


/**
  * @bssid: unique identifier of the source BSS for which the steering BTM report applies
  * @sta_mac: sta mac address for which the steering BTM report applies
  * @status: indicates the value of the BTM status code as reported by the STA in the BTM response
  * @tbssid: indicates the value of the target BSSID field(if present)in the BTM response received
  * from the STA. Note: this indicates the BSSID that the STA intends to roam to, whicg may not
  * align with the target BSSID specified in the BTM request.
  */
struct GNU_PACKED cli_steer_btm_event {
	unsigned char bssid[ETH_ALEN];
	unsigned char sta_mac[ETH_ALEN];
	unsigned char status;
	unsigned char tbssid[ETH_ALEN];
};

/**
  * @bssid: unique identifier of the BSS for which the client blocking request applies
  * @assoc_control: indicates if the request is to block or unblock the indicated STAs from associating
  * @valid_period: time period in seconds(from reception of the client association control request message)
  * for which a blocking request is valid
  * @sta_list_count: indicateing one or more STAs  for which the client association control request applies
  * @sta_mac: sta mac address for which he client association control request applies
  */
struct GNU_PACKED cli_assoc_control {
	unsigned char bssid[ETH_ALEN];
	unsigned char assoc_control;
	unsigned short valid_period;
	unsigned char sta_list_count;
	unsigned char sta_mac[0];
};

struct GNU_PACKED local_disallow_sta_head {
	unsigned char sta_cnt;
	unsigned char sta_list[0];
};
struct GNU_PACKED btm_disallow_sta_head {
	unsigned char sta_cnt;
	unsigned char sta_list[0];
};
struct GNU_PACKED radio_policy_head {
	unsigned char identifier[ETH_ALEN];
	unsigned char policy;
	unsigned char ch_ultil_thres;
	unsigned char rssi_thres;
};
struct GNU_PACKED radio_policy {
	unsigned char radio_cnt;
	struct radio_policy_head radio[0];
};

struct GNU_PACKED metric_policy_head {
	unsigned char identifier[ETH_ALEN];
	unsigned char rssi_thres;
	unsigned char hysteresis_margin;
	unsigned char ch_util_thres;
	unsigned char sta_stats_inclusion;
	unsigned char sta_metrics_inclusion;
#ifdef MAP_R3
	unsigned char assoc_wf6_inclusion;
#endif
};
struct GNU_PACKED metric_policy {
	unsigned char report_interval;
	unsigned char policy_cnt;
	struct metric_policy_head policy[0];
};


/**
  * @backhaul_mac: the mac address of associated backhaul station operated by the multi-ap agent
  * @target_bssid: the bssid of the target BSS
  * @oper_class: operating class
  * @channel: channel number on which beacon frames are being transmitted by the target BSS
  */
struct GNU_PACKED backhaul_steer_request {
	unsigned char backhaul_mac[ETH_ALEN];
	unsigned char target_bssid[ETH_ALEN];
	unsigned char oper_class;
	unsigned char channel;
};

/**
  * @backhaul_mac: the mac address of associated backhaul station operated by the multi-ap agent
  * @target_bssid: the bssid of the target BSS
  * @status: status code
  */
struct GNU_PACKED backhaul_steer_rsp {
	unsigned char backhaul_mac[ETH_ALEN];
	unsigned char target_bssid[ETH_ALEN];
	unsigned char status;
	unsigned char error;
};

/*link metric collection*/
struct GNU_PACKED esp_info {
	unsigned char ac;
	unsigned char format;
	unsigned char ba_win_size;
	unsigned char e_air_time_fraction;
	unsigned char ppdu_dur_target;
};

/**
  * @bssid: bssid of a bss operated by the multi-ap agent for which the metrics are reported
  * @ch_util: channel utilization as mesured by the radio operating the bss
  * @assoc_sta_cnt: indicates the total number of STAs currently associated with the BSS
  * @esp: estimated service parameters information field.
  */
struct GNU_PACKED ap_metrics_info {
	unsigned char bssid[ETH_ALEN];
	unsigned char ch_util;
	unsigned short assoc_sta_cnt;
	unsigned char valid_esp_count;
	struct esp_info esp[0];
};

/**
  * @mac: MAC address of the associated STA
  * @bytes_sent: raw counter of the number of bytes sent to the associated STA
  * @bytes_received: raw counter of number of bytes received from the associated STA
  * @packets_sent: raw counter of the number of packets successfully sent to the associated STA
  * @packets_received: raw counter of the number of packets received from the associated STA
  * during the measurement window
  * @tx_packets_errors: raw counter of the number of packets which could not be transmitted to
  * the associated STA due to errors
  * @rx_packets_errors: raw counter of the number of packets which were received in error from
  * the associated STA
  * @retransmission_count: raw counter of the number of packets sent with the retry flag set to
  * the associated STA
  */
struct GNU_PACKED stat_info {
	unsigned char mac[ETH_ALEN];
	unsigned int bytes_sent;
	unsigned int bytes_received;
	unsigned int packets_sent;
	unsigned int packets_received;
	unsigned int tx_packets_errors;
	unsigned int rx_packets_errors;
	unsigned int retransmission_count;
	unsigned char is_APCLI;
};

/**
  * @identifier: radio unique identifier of the radio for which sta are associated
  * @sta_cnt: the total number of sta which traffic stats are reported
  * @stats: sta traffic stats info
  */
struct GNU_PACKED sta_traffic_stats {
	unsigned char identifier[ETH_ALEN];
	unsigned char sta_cnt;
	struct stat_info stats[0];
};

/**
  * @mac: MAC address of the associated STA
  * @bssid: BSSID of the BSS for which the STA is associated
  * @time_delta: The time delta in ms between the time at which the earliest measurement that
  * contributed to the data rate estimates were made, and the time at which this report was sent
  * @erate_downlink: Estimated MAC Data Rate in downlink (in Mb/s)
  * @erate_uplink: Estimated MAC Data Rate in uplink (in Mb/s)
  * @rssi_uplink: Measured uplink RSSI for STA (dBm)
  */
struct GNU_PACKED link_metrics {
	unsigned char mac[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	unsigned int time_delta;
	unsigned int erate_downlink;
	unsigned int erate_uplink;
	unsigned char rssi_uplink;
	unsigned char is_APCLI;
};

/**
  * @identifier: radio unique identifier of the radio for which sta are associated
  * @sta_cnt: the total number of sta which link metrics are reported
  * @info: sta link metrics info
  */
struct GNU_PACKED sta_link_metrics {
	unsigned char identifier[ETH_ALEN];
	unsigned char sta_cnt;
	struct link_metrics info[0];
};

/**
  * a struct to query tx or rx link metrics info of 1905
  * @media_type: wifi or eth interface
  * @local_if: MAC address of an interface in the receiving IEEE 1905.1 AL, which connects to
  * an interface in the neighbor IEEE 1905.1 AL
  * @neighbor_if: MAC address of an IEEE 1905.1 interface in a neighbor IEEE 1905.1 device
  * which connects to an IEEE 1905.1 interface in the receiving IEEE 1905.1 device
  */
struct GNU_PACKED link_stat_query {
	unsigned short int media_type;
	unsigned char local_if[ETH_ALEN];
	unsigned char neighbor_if[ETH_ALEN];
};

/**
  * @pkt_errs: wifi or eth interface
  * @local_if: Estimated number of lost packets on the transmit side of the link during the
  * measurement period
  * @tx_pkts: Estimated number of packets transmitted by the Transmitter of the link on
  * the same measurement period
  * @mac_tp_cap: The maximum MAC throughput of the Link estimated at the transmitter
  * and expressed in Mb/s
  * @link_avail: The estimated average percentage of time that the link is available for data
  * transmission
  * @phyrate: If the media type of the link is IEEE 802.3, then IEEE 1901 or MoCA 1.1
  * This value is the PHY rate estimated at the transmitter of the link expressed in Mb/s;
  * otherwise, it is set to 0xFFFF.
  */
struct GNU_PACKED tx_link_stat_rsp {
	unsigned int pkt_errs;
	unsigned int tx_pkts;
	unsigned short mac_tp_cap;
	unsigned short link_avail;
	unsigned short phyrate;
};

/**
  * @pkt_errs: wifi or eth interface
  * @local_if: Estimated number of lost packets on the receive side of the link during the
  * measurement period
  * @rx_pkts: Estimated number of packets transmitted by the Transmitter of the link on
  * the same measurement period
  * @rssi: If the media type of the link is IEEE 802.11, then this value is the estimated RSSI
  * in dB at the receive side of the Link expressed in dB; otherwise, it is set to 0xFF.
  */
struct GNU_PACKED rx_link_stat_rsp {
	unsigned int pkt_errs;
	unsigned int rx_pkts;
	unsigned char rssi;
};

struct GNU_PACKED unlink_metrics_query {
	unsigned char oper_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char sta_num;
	unsigned char sta_list[0];
};

struct GNU_PACKED unlink_rsp_sta {
	unsigned char mac[ETH_ALEN];
	unsigned char ch;
	unsigned int time_delta;
	signed char uplink_rssi;
};

struct GNU_PACKED unlink_metrics_rsp {
	unsigned char oper_class;
	unsigned char sta_num;
	struct unlink_rsp_sta info[0];
};

struct GNU_PACKED ap_chn_rpt {
	unsigned char ch_rpt_len;
	unsigned char oper_class;
	unsigned char ch_list[MAX_CH_NUM];
};

struct GNU_PACKED beacon_metrics_query {
	unsigned char sta_mac[ETH_ALEN];
	unsigned char oper_class;
	unsigned char ch;
	unsigned char bssid[ETH_ALEN];
	unsigned char rpt_detail_val;
	unsigned char ssid_len;
	unsigned char ssid[33];
	unsigned char elemnt_num;
	unsigned char elemnt_list[MAX_ELEMNT_NUM];
	unsigned char ap_ch_rpt_num;
	struct ap_chn_rpt rpt[0];
};


struct GNU_PACKED beacon_metrics_rsp {
	unsigned char sta_mac[ETH_ALEN];
	unsigned char reserved;
	unsigned char bcn_rpt_num;
	unsigned short rpt_len;
	unsigned char rpt[0];
};

struct GNU_PACKED topo_info {
	unsigned char almac[ETH_ALEN];
	unsigned char bssid_num;
	unsigned char bssid[0];
};

struct GNU_PACKED csa_info_rsp {
	unsigned char ruid[ETH_ALEN];
	unsigned char new_ch;
};

struct GNU_PACKED interface_info {
	unsigned char if_name[IFNAMSIZ];
	unsigned char if_role[6]; /* wiap,wista */
	unsigned char if_ch;      /* channel */
#ifdef MAP_6E_SUPPORT
	unsigned char if_opclass;
#endif
	unsigned char if_phymode[3];  /* n,ac */
	unsigned char if_mac_addr[ETH_ALEN];
	unsigned char identifier[ETH_ALEN];/*belong to which radio*/
};

struct GNU_PACKED interface_info_list_hdr {
	unsigned char interface_count;
	struct interface_info if_info[0];
};

struct GNU_PACKED bh_assoc_wireless_setting {
	unsigned char   bh_mac_addr[ETH_ALEN];
	unsigned char	target_ssid[33];
	unsigned char 	target_bssid[ETH_ALEN];
	unsigned short  auth_mode;
	unsigned short	encryp_type;
	unsigned char	wpa_key[65];
	unsigned char   target_channel;
};

typedef struct GNU_PACKED wapp_sta_activity_status {
	unsigned char status;
	unsigned char sta_mac[ETH_ALEN];
} wapp_sta_status_info;

struct GNU_PACKED map_vendor_ie
{
	unsigned char type;
	unsigned char subtype;
	unsigned char root_distance;
	unsigned char connectivity_to_controller;
	unsigned short uplink_rate;
	unsigned char uplink_bssid[ETH_ALEN];
	unsigned char bssid_5g[ETH_ALEN];
	unsigned char bssid_2g[ETH_ALEN];
};

struct GNU_PACKED bss_info {
	unsigned char Bssid[ETH_ALEN];
	unsigned char Channel;
	unsigned char CentralChannel;   /*Store the wide-band central channel for 40MHz.  .used in 40MHz AP. Or this is the same as Channel. */
	char Rssi;
	char MinSNR;
	unsigned char Privacy;                  /* Indicate security function ON/OFF. Don't mess up with auth mode. */
	unsigned char SsidLen;
	char Ssid[33];
	char AuthMode;
	unsigned char map_vendor_ie_found;
	struct map_vendor_ie map_info;
};

struct GNU_PACKED wapp_scan_info {
	unsigned char more_bss;
	unsigned char bss_count;
	struct bss_info bss[0];
};

struct GNU_PACKED tp_metrics {
	unsigned char mac[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	unsigned int tx_tp;
	unsigned int rx_tp;
};

struct GNU_PACKED sta_tp_metrics {
	unsigned char identifier[ETH_ALEN];
	unsigned char sta_cnt;
	struct tp_metrics info[0];
};

struct GNU_PACKED bssload_info {
	unsigned short sta_cnt;
	unsigned char ch_util;
	unsigned short AvalAdmCap;
};

struct GNU_PACKED CONNECT_FAILURE_REASON {
	unsigned char	connect_stage;
	unsigned short	reason;
} ;

struct GNU_PACKED sta_cnnct_rej_info {
	unsigned int interface_index;
	unsigned char sta_mac[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	struct CONNECT_FAILURE_REASON cnnct_fail;
};

struct GNU_PACKED bssload_crossing_info {
	unsigned int interface_index;
	unsigned char bssload_high_thrd;
	unsigned char bssload_low_thrd;
	unsigned char bssload;
};

struct GNU_PACKED bss_state_info {
	unsigned int interface_index;
	unsigned char bss_state;
};

struct GNU_PACKED apcli_association_info {
	unsigned int interface_index;
	unsigned char apcli_assoc_state;
	char rssi;
};

#define PREQ_IE_LEN 200
struct GNU_PACKED probe {
	unsigned char mac_addr[ETH_ALEN];
	unsigned char channel;
	char rssi;
	unsigned char preq_len;
	unsigned char preq[PREQ_IE_LEN];
};

#ifdef MAP_R2
#define MAX_VLAN_NUM 	48
#define VLAN_N_VID		4095

struct GNU_PACKED ts_fh_config {
	unsigned char itf_mac[ETH_ALEN];
	unsigned short vid;
};

struct GNU_PACKED ts_fh_bss_setting {
	unsigned char itf_num;
	struct ts_fh_config fh_configs[0];
};

struct GNU_PACKED ts_common_setting {
	unsigned short primary_vid;
	unsigned char primary_pcp;
	unsigned char policy_vid_num;
	unsigned short policy_vids[MAX_VLAN_NUM];
};

struct GNU_PACKED ts_setting {
	struct ts_common_setting common_setting;
	struct ts_fh_bss_setting fh_bss_setting;
};

struct GNU_PACKED trans_vlan_config {
	unsigned char trans_vid_num;
	unsigned short vids[128];
};

struct GNU_PACKED trans_vlan_setting {
	struct trans_vlan_config trans_vlan_configs;
	unsigned char apply_itf_num;
	unsigned char apply_itf_mac[0];
};

struct GNU_PACKED ap_r2_capability {
	unsigned char max_total_num_sp_rules;
	unsigned char reserved1;

	unsigned char rsv_bit0_2:3;
	unsigned char ts_flag:1;
	unsigned char dpp_flag:1;
	unsigned char sp_flag:1;
	unsigned char byte_counter_units:2; /*0: bytes, 1: kibibytes (KiB), 2: mebibytes (MiB), 3: reserved*/

	unsigned char max_total_num_vid;
};

/**
  * @target_bssid: indicates a target BSSID for steering. wildcard BSSID is represented by ff:ff:ff:ff:ff:ff
  * @op_class: target BSS operating class
  * @channel: target BSS channel number for channel on which the target BSS is transmitting beacon frames
  * @sta_mac: sta mac address for which the steering request applies. if sta_count of struct steer_request
  * is 0, then this field is invalid
  */
struct GNU_PACKED target_bssid_info_r2 {
	unsigned char target_bssid[ETH_ALEN];
	unsigned char op_class;
	unsigned char channel;
	unsigned char reason_code;
	unsigned char sta_mac[ETH_ALEN];
};

/**
  * @assoc_bssid: unique identifier of the source BSS for whirch the steering request applies
  * (i.e. BSS that the STAs specified in the request are currently associated with)
  * @request_mode: 0: request is a steering opportunity; 1: request is a steering mandate to
  * trigger steering for specific client STA(s)
  * @btm_disassoc_immi: BTM Disassociation imminent bit
  * @btm_abridged: BTM Abridged bit
  * @steer_window: steering opportunity window. time period in seconds(from reception of the
  * steering request message)for whitch the request is valid.
  * @btm_disassoc_timer: BTM Disassociation timer. time period in TUs for disassociation timer
  * in BTM request
  * @sta_count: sta list count. k=0: steering request applies to all associated STAs in the BSS per
  * policy setting; k>0: steering request applies to specific STAs specific by STA MAC address(es)
  * @target_bssid_count: target BSSID list count. only valid when request_mode is set to 1.
  * m=1: the same target BSSID is indicated for all specified STAs;m=k: an individual target BSSID
  * is indicated for each apecified STA(in same order)
  * @steer_info: contains k sta mac address and m target bssid info
  */
struct GNU_PACKED steer_request_r2 {
	unsigned char assoc_bssid[ETH_ALEN];
	unsigned char request_mode;
	unsigned char btm_disassoc_immi;
	unsigned char btm_abridged;
	unsigned short steer_window;
	unsigned short btm_disassoc_timer;
	unsigned char sta_count;
	unsigned char target_bssid_count;
	struct target_bssid_info_r2 info[0];
};
#endif // #ifdef MAP_R2


#ifdef MAP_R3_DE
#define DE_MAX_LEN 64
#define MAX_RUID 3
struct GNU_PACKED dev_inven_ruid {
	unsigned char chip_ven_len;
	char identifier[ETH_ALEN];
	char chip_ven[DE_MAX_LEN];
};

struct GNU_PACKED dev_inven {
	unsigned char ser_num_len;
	unsigned char sw_ver_len;
	unsigned char exec_env_len;
	unsigned char num_radio;
	char ser_num[DE_MAX_LEN];
	char sw_ver[DE_MAX_LEN];
	char exec_env[DE_MAX_LEN];
	struct dev_inven_ruid ruid[MAX_RUID];
};
#endif //MAP_R3_DE

#ifdef MAP_R3
struct GNU_PACKED ap_wf6_capability
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

struct GNU_PACKED ap_wf6_cap_roles {
	unsigned char identifier[ETH_ALEN];
	unsigned char role_supp;
#ifdef MAP_R4_SPT
	unsigned char sr_mode;
#endif
	struct ap_wf6_capability wf6_role[2];
};
#endif /*MAP_R3*/

#ifndef MAP_R$_SPT
struct GNU_PACKED ap_spt_reuse_req {
	unsigned char identifier[ETH_ALEN];
	unsigned char bss_color;
	unsigned char hesiga_spa_reuse_val_allowed;
	unsigned char srg_info_valid;
	unsigned char nonsrg_offset_valid;
	unsigned char psr_disallowed;
	unsigned char nonsrg_obsspd_max_offset;
	unsigned char srg_obsspd_min_offset;
	unsigned char srg_obsspd_max_offset;
	unsigned char srg_bss_color_bitmap[8];
	unsigned char srg_partial_bssid_bitmap[8];
};
#endif

#endif /*INTERFACE_H*/

