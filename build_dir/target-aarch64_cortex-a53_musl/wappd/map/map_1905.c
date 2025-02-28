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

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "driver_wext.h"
#include "wapp_cmm.h"
#include "wapp_cmm_type.h"
#include "interface.h"
#include "wps.h"
#include "dhcp_ctl.h"
#ifdef DPP_SUPPORT
#include "dpp_wdev.h"
#endif /*DPP_SUPPORT*/
#include "debug.h"
#include <sys/socket.h>

/*agent 1905 server socket name*/
#define _1905_SERVER_NAME   "1905_server"
/*controller 1905 server socket name*/
#define _1905_SERVER_NAME_CONTROLLER	"1905_server_controller"
#define WAPP_SERVER_NAME    "/tmp/wapp_server"

//Message buffer len from 1905
#define MAX_MSG_BUF_LEN 1524
static unsigned short air_mnt_map;
unsigned short prev_1905_msg = 0;
u8 prev_req_radio_id[ETH_ALEN] = {0};

struct vht_ch_layout {
	UCHAR ch_low_bnd;
	UCHAR ch_up_bnd;
	UCHAR cent_freq_idx;
};

static struct vht_ch_layout vht_ch_80M[] = {
	{36, 48, 42},
	{52, 64, 58},
	{100, 112, 106},
	{116, 128, 122},
	{132, 144, 138},
	{149, 161, 155},
	{165, 177, 171},
	{0, 0, 0},
};

void map_config_state_check(void *eloop_data, void *user_ctx);
#ifdef MAP_R2
int map_config_unsuccessful_assoc_policy_msg(
	struct wifi_app *wapp, char *msg_buf);
#endif
unsigned char __get_primary_channel(unsigned char channel) {
	int i, ch_size;
	struct vht_ch_layout *vht;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	ch_size = sizeof(vht_ch_80M) / sizeof(struct vht_ch_layout);
	for (i = 0; i < ch_size; i++) {
		vht = &vht_ch_80M[i];
		if (vht->cent_freq_idx == channel) {
			return vht->ch_low_bnd;
		}
	}
	return channel;
}

#ifdef ACL_CTRL
/* ACL command to driver */
void map_acl_system_cmd(
	struct wifi_app *wapp, struct wapp_dev *wdev, unsigned char *sta_addr,
	ACL_CMD_TYPE type)
{

#if NL80211_SUPPORT
	u8 Enable = 1;
	u8 AccessPolicy = 0;
#else
	char cmd[MAX_CMD_MSG_LEN] = {0};
	os_memset(cmd, 0, MAX_CMD_MSG_LEN);
	int ret;
#endif

	DBGPRINT(RT_DEBUG_OFF, "%s ACL type = %d\n", __func__, type);
	switch (type) {
#if NL80211_SUPPORT
		case ACL_ADD:
			wapp_set_ACLAddEntry(wapp, (const char *)wdev->ifname,
					(char*)sta_addr, MAC_ADDR_LEN);
			break;
		case ACL_DEL:
			wapp_set_ACLDelEntry(wapp, (const char *)wdev->ifname,
					(char*)sta_addr, MAC_ADDR_LEN);
			break;
		case ACL_FLUSH:
			wapp_set_ACLClearAll(wapp, (const char *)wdev->ifname,
					(char*)&Enable, 1);
			break;
		case ACL_POLICY_0:
			wapp_set_AccessPolicy(wapp, (const char *)wdev->ifname,
					(char*)&AccessPolicy, 1);
			break;
		case ACL_POLICY_1:
			AccessPolicy = 1;
			wapp_set_AccessPolicy(wapp, (const char *)wdev->ifname,
					(char*)&AccessPolicy, 1);
			break;
		case ACL_POLICY_2:
			AccessPolicy = 2;
			wapp_set_AccessPolicy(wapp, (const char *)wdev->ifname,
					(char*)&AccessPolicy, 1);
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, "NL80211: %s Unknown Type\n", __func__);
			break;
#else
		case ACL_ADD:
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ACLAddEntry=%02x:%02x:%02x:%02x:%02x:%02x;", wdev->ifname, PRINT_MAC(sta_addr));
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			break;
		case ACL_DEL:
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ACLDelEntry=%02x:%02x:%02x:%02x:%02x:%02x;", wdev->ifname, PRINT_MAC(sta_addr));
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			break;
		case ACL_FLUSH:
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ACLClearAll=1;", wdev->ifname);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			break;
		case ACL_POLICY_0:
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set AccessPolicy=0;", wdev->ifname);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			break;
		case ACL_POLICY_1:
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set AccessPolicy=1;", wdev->ifname);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			break;
		case ACL_POLICY_2:
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set AccessPolicy=2;", wdev->ifname);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, "%s Unknown Type\n", __func__);
			break;
#endif
	}
}

/* Blacklist command to driver */
void map_blacklist_system_cmd(
	struct wapp_dev *wdev, unsigned char *sta_addr, BLACKLIST_CMD_TYPE type)
{
	char cmd[MAX_CMD_MSG_LEN] = {0};
	int ret = -1;
	DBGPRINT(RT_DEBUG_OFF, "%s BL type = %d\n", __func__, type);
	switch (type) {
#if NL80211_SUPPORT
		case BLOCK:
			wapp_set_BlAdd(wapp, (const char *)wdev->ifname,
					(char*)sta_addr, MAC_ADDR_LEN);
			break;
		case UNBLOCK:
			wapp_set_BlDel(wapp, (const char *)wdev->ifname,
					(char*)sta_addr, MAC_ADDR_LEN);
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, "NL80211: %s Unknown Type\n", __func__);
			break;
#else
		case BLOCK:
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set BlAdd=%02x:%02x:%02x:%02x:%02x:%02x;", wdev->ifname, PRINT_MAC(sta_addr));
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			break;
		case UNBLOCK:
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set BlDel=%02x:%02x:%02x:%02x:%02x:%02x;", wdev->ifname, PRINT_MAC(sta_addr));
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, "%s Unknown Type\n", __func__);
			break;
#endif
	}
	os_memset(cmd, 0, MAX_CMD_MSG_LEN);
}
#else
/* ACL command to driver */
void map_acl_system_cmd(
	struct wifi_app *wapp, struct wapp_dev *wdev, unsigned char *sta_addr,
	ACL_CMD_TYPE type)
{
#if NL80211_SUPPORT
	u8 Enable = 1;
	u8 AccessPolicy = 2;
#else
	char cmd[MAX_CMD_MSG_LEN] = {0};
	os_memset(cmd, 0, MAX_CMD_MSG_LEN);
	int ret = -1;
#endif

	DBGPRINT(RT_DEBUG_OFF, "%s ACL type = %d\n", __func__, type);
	switch (type) {
#if NL80211_SUPPORT
		case ACL_ADD:
			wapp_set_AccessPolicy(wapp, (const char *)wdev->ifname,
					(char*)&AccessPolicy, 1);
			wapp_set_ACLAddEntry(wapp, (const char *)wdev->ifname,
					(char*)sta_addr, MAC_ADDR_LEN);
			break;
		case ACL_DEL:
			wapp_set_ACLDelEntry(wapp, (const char *)wdev->ifname,
					(char*)sta_addr, MAC_ADDR_LEN);
			break;
		case ACL_FLUSH:
			wapp_set_ACLClearAll(wapp, (const char *)wdev->ifname,
					(char*)&Enable, 1);
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, "NL80211: %s Unknown Type\n", __func__);
			break;
#else
		case ACL_ADD:
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set AccessPolicy=2;", wdev->ifname);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			os_memset(cmd, 0, MAX_CMD_MSG_LEN);
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ACLAddEntry=%02x:%02x:%02x:%02x:%02x:%02x;", wdev->ifname, PRINT_MAC(sta_addr));
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			break;
		case ACL_DEL:
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ACLDelEntry=%02x:%02x:%02x:%02x:%02x:%02x;", wdev->ifname, PRINT_MAC(sta_addr));
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			break;
		case ACL_FLUSH:
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ACLClearAll=1;", wdev->ifname);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, "%s Unknown Type\n", __func__);
			break;
#endif
	}
}
#endif /*ACL_CTRL*/
static unsigned char get_air_mnt_idx(void)
{
	unsigned char i = 0;

	for (i = 0; i < 16; i++) {
		if ((air_mnt_map & BIT(i)) == 0) {
			air_mnt_map |= BIT(i);
			return i;
		}
	}
	return 0xFF;
}
/* Air monitor command to driver */
void map_set_air_mnt_cmd(
#if NL80211_SUPPORT
	struct wifi_app *wapp, struct wapp_dev *wdev, unsigned char *sta_addr, BOOLEAN mnt_en, unsigned char mnt_rule
#else
	struct wifi_app *wapp, struct wapp_dev *wdev, unsigned char *sta_addr, BOOLEAN mnt_en, unsigned char mnt_rule
#endif /* NL80211_SUPPORT */
)
{
#if NL80211_SUPPORT
	char *rule;
#else
	char cmd[MAX_CMD_MSG_LEN] = {0};
	char *rule;
	int ret;
#endif /* NL80211_SUPPORT */
	unsigned char idx = 0;
	DBGPRINT(RT_DEBUG_OFF, "%s Air Mnt enable = %d, rule = %d\n", __func__, mnt_en, mnt_rule);

	if (!mnt_en || air_mnt_map == 0) {
#if NL80211_SUPPORT
	wapp_set_mnt_en(wapp, (const char *)wdev->ifname,
				(char *)&mnt_en,
				(size_t)sizeof(mnt_en));
#else
	ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set mnt_en=%d;", wdev->ifname, mnt_en);
	if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
	os_memset(cmd, 0, MAX_CMD_MSG_LEN);
#endif /* NL80211_SUPPORT */
	}

	if (!mnt_en)
		return;

	switch(mnt_rule) {
		case 0:
			rule="0:0:0";
			break;
		case 1:
			rule="0:0:1";
			break;
		case 2:
			rule="0:1:0";
			break;
		case 3:
			rule="0:1:1";
			break;
		case 4:
			rule="1:0:0";
			break;
		case 5:
			rule="1:0:1";
			break;
		case 6:
			rule="1:1:0";
			break;
		case 7:
			rule="1:1:1";
			break;
		default:
			DBGPRINT(RT_DEBUG_OFF, "%s Air Mnt not support rule = %d\n", __func__, mnt_rule);
			rule="1:1:0";
			break;
	}

	if (air_mnt_map == 0) {
#if NL80211_SUPPORT
	wapp_set_mnt_rule(wapp, (const char *)wdev->ifname,
					(char *)rule,
					(size_t)sizeof(rule));

	/* After setting the sta mac, air monitor will be enabled automatically*/
	wapp_set_mnt_sta0(wapp, (const char *)wdev->ifname,
					(char *)sta_addr,
					MAC_ADDR_LEN);
#else
	if (wapp->air_mnt_max_pkt) {
		ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set mnt_max_pkt_app=%d;", wdev->ifname, wapp->air_mnt_max_pkt);
		if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		if (system(cmd) != 0)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
	}

	ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set mnt_rule=%s;", wdev->ifname, rule);
	if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
	os_memset(cmd, 0, MAX_CMD_MSG_LEN);
	}

	idx = get_air_mnt_idx();
	/* After setting the sta mac, air monitor will be enabled automatically*/
	ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set mnt_sta%d=%02x:%02x:%02x:%02x:%02x:%02x;",
								wdev->ifname, idx, PRINT_MAC(sta_addr));
	if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
	os_memset(cmd, 0, MAX_CMD_MSG_LEN);
#endif /* NL80211_SUPPORT */
}

char *MapMsgTypeToString(unsigned char MsgType)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	switch (MsgType) {
		case WAPP_WPS_CONFIG_STATUS:
			return "WAPP_WPS_CONFIG_STATUS";
		case WAPP_USER_GET_RADIO_BASIC_CAP:
			return "WAPP_USER_GET_RADIO_BASIC_CAP";
		case WAPP_USER_GET_AP_CAPABILITY:
			return "WAPP_USER_GET_AP_CAPABILITY";
		case WAPP_USER_GET_AP_HT_CAPABILITY:
			return "WAPP_USER_GET_AP_HT_CAPABILITY";
		case WAPP_USER_GET_AP_VHT_CAPABILITY:
			return "WAPP_USER_GET_AP_VHT_CAPABILITY";
		case WAPP_USER_GET_AP_HE_CAPABILITY:
			return "WAPP_USER_GET_AP_HE_CAPABILITY";
		case WAPP_USER_GET_ASSOCIATED_CLIENT:
			return "WAPP_USER_GET_ASSOCIATED_CLIENT";
		case WAPP_USER_GET_RA_OP_RESTRICTION:
			return "WAPP_USER_GET_RA_OP_RESTRICTION";
		case WAPP_USER_GET_CHANNEL_PREFERENCE:
			return "WAPP_USER_GET_CHANNEL_PREFERENCE";
		case WAPP_USER_SET_CHANNEL_SETTING:
			return "WAPP_USER_SET_CHANNEL_SETTING";
		case WAPP_USER_GET_OPERATIONAL_BSS:
			return "WAPP_USER_GET_OPERATIONAL_BSS";
		case WAPP_USER_SET_WIRELESS_SETTING:
			return "WAPP_USER_SET_WIRELESS_SETTING";
		case WAPP_USER_SET_STEERING_SETTING:
			return "WAPP_USER_SET_STEERING_SETTING";
		case WAPP_USER_MAP_CONTROLLER_FOUND:
			return "WAPP_USER_MAP_CONTROLLER_FOUND";
		case WAPP_USER_SET_LOCAL_STEER_DISALLOW_STA:
			return "WAPP_USER_SET_LOCAL_STEER_DISALLOW_STA";
		case WAPP_USER_SET_BTM_STEER_DISALLOW_STA:
			return "WAPP_USER_SET_BTM_STEER_DISALLOW_STA";
		case WAPP_USER_SET_RADIO_CONTROL_POLICY:
			return "WAPP_USER_SET_RADIO_CONTROL_POLICY";
		case WAPP_USER_SET_ASSOC_CNTRL_SETTING:
			return "WAPP_USER_SET_ASSOC_CNTRL_SETTING";
		case WAPP_USER_SET_BACKHAUL_STEER:
			return "WAPP_USER_SET_BACKHAUL_STEER";
		case WAPP_USER_GET_AP_METRICS_INFO:
			return "WAPP_USER_GET_AP_METRICS_INFO";
		case WAPP_USER_GET_ASSOC_STA_TRAFFIC_STATS:
			return "WAPP_USER_GET_ASSOC_STA_TRAFFIC_STATS";
		case WAPP_USER_GET_ONE_ASSOC_STA_TRAFFIC_STATS:
			return "WAPP_USER_GET_ONE_ASSOC_STA_TRAFFIC_STATS";
		case WAPP_USER_GET_ASSOC_STA_LINK_METRICS:
			return "WAPP_USER_GET_ASSOC_STA_LINK_METRICS";
		case WAPP_USER_GET_ALL_ASSOC_TP_METRICS:
			return "WAPP_USER_GET_ALL_ASSOC_TP_METRICS";
		case WAPP_USER_GET_ONE_ASSOC_STA_LINK_METRICS:
			return "WAPP_USER_GET_ONE_ASSOC_STA_LINK_METRICS";
		case WAPP_USER_GET_RX_LINK_STATISTICS:
			return "WAPP_USER_GET_RX_LINK_STATISTICS";
		case WAPP_USER_GET_TX_LINK_STATISTICS:
			return "WAPP_USER_GET_TX_LINK_STATISTICS";
		case WAPP_USER_GET_UNASSOC_STA_LINK_METRICS:
			return "WAPP_USER_GET_UNASSOC_STA_LINK_METRICS";
		case WAPP_USER_SET_METIRCS_POLICY:
			return "WAPP_USER_SET_METIRCS_POLICY";
		case WAPP_USER_SET_BEACON_METRICS_QRY:
			return "WAPP_USER_SET_BEACON_METRICS_QRY";
		case WAPP_USER_SET_RADIO_TEARED_DOWN:
			return "WAPP_USER_SET_RADIO_TEARED_DOWN";
		case WAPP_USER_GET_OPERATING_CHANNEL_INFO:
			return "WAPP_USER_GET_OPERATING_CHANNEL_INFO";
		case WAPP_USER_FLUSH_ACL:
			return "WAPP_USER_FLUSH_ACL";
		case WAPP_USER_GET_BSSLOAD:
			return "WAPP_USER_GET_BSSLOAD";
		case WAPP_USER_GET_RSSI_REQ:
			return "WAPP_USER_GET_RSSI_REQ";
		case WAPP_USER_SET_WHPROBE_REQ:
			return "WAPP_USER_SET_WHPROBE_REQ";
		case WAPP_USER_SET_NAC_REQ:
			return "WAPP_USER_SET_NAC_REQ";
#ifdef MAP_SUPPORT
		case WAPP_USER_SET_AIR_MONITOR_REQUEST:
			return "WAPP_USER_SET_AIR_MONITOR_REQUEST";
#endif
		case WAPP_USER_GET_APCLI_RSSI_REQ:
			return "WAPP_USER_GET_APCLI_RSSI_REQ";
		case WAPP_USER_GET_BRIDGE_IP_REQUEST:
			return "WAPP_USER_GET_BRIDGE_IP_REQUEST";
		case WAPP_USER_SET_TX_POWER_PERCENTAGE:
			return "WAPP_USER_SET_TX_POWER_PERCENTAGE";
#ifdef ACL_CTRL
		case WAPP_USER_SET_ACL_CNTRL_SETTING:
			return "WAPP_USER_SET_ACL_CNTRL_SETTING";
#endif
#ifdef MAP_R3_WF6
		case WAPP_USER_GET_ASSOC_WIFI6_STA_STATUS:
			return "WAPP_USER_GET_ASSOC_WIFI6_STA_STATUS";
		case WAPP_USER_GET_DEV_INVEN_TLV:
			return "WAPP_USER_GET_DEV_INVEN_TLV";
		case WAPP_USER_GET_AP_WF6_CAPABILITY:
			return "WAPP_USER_GET_AP_WF6_CAPABILITY";

#endif
#ifdef MAP_R4_SPT
		case WAPP_USER_GET_AP_SPT_REUSE:
			return "WAPP_USER_GET_AP_SPT_REUSE";
#endif

		default:
			return "Unknown Type";
	}
}

int map_check_buf_incr_ctrl_pending(int sock, struct timeval *tv)
{
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(sock, &rfds);
	select(sock + 1, &rfds, NULL, NULL, tv);
	return FD_ISSET(sock, &rfds);
}

int map_check_buf_incr_recv(int sock, char *reply, size_t *reply_len)
{
	int res;

	res = recv(sock, reply, *reply_len, 0);
	if (res < 0)
		return res;
	*reply_len = res;
	return 0;
}

int map_1905_send(struct wifi_app* wapp, char* buffer_send, int len_send)
{
	struct evt *map_event;
	struct cmd_to_wapp *cmd = NULL;
	int pkt_len = 0;
	char recv_buf[50] = {0};
	struct timeval tv;
	size_t len = 19;
	u16 length = 0;
	char *status = NULL;

	if(wapp->map && (len_send > wapp->map->send_buffer)) {
		pkt_len = sizeof(struct evt) + sizeof(length);
		length = len_send;
		map_event = (struct evt *)recv_buf;
		map_event->type = WAPP_SEND_BUFFER_INCR;
		map_event->length = sizeof(length);
		os_memcpy(&map_event->buffer[0], &length, map_event->length);
		wapp_iface_send(wapp, (char* )recv_buf, pkt_len, "mapDemo_wapp_App");

		os_memset(recv_buf,0,50);
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		/* Select() wait for confirmation from mapd on buffer increase */
		if(map_check_buf_incr_ctrl_pending(wapp->infcli_ctrl.sock, &tv)) {
			if(map_check_buf_incr_recv(wapp->infcli_ctrl.sock, recv_buf, &len) < 0) {
				DBGPRINT(RT_DEBUG_ERROR, "%s socket read failed!!!!! \n", __func__);
				return -1;
			}

			cmd = (struct cmd_to_wapp *)recv_buf;
			status= (char*)cmd->body;
			if ((cmd->type == WAPP_SET_SEND_BUFFER_INCR )&& (*status == 1)) {
				wapp_iface_send(wapp, (char* )buffer_send, len_send, "mapDemo_wapp_App");
				wapp->map->send_buffer = len_send;
				DBGPRINT(RT_DEBUG_ERROR, "%s Send as Buffer increase success for size %d !!!!! \n", __func__,len_send);
			} else {
				DBGPRINT(RT_DEBUG_ERROR, "%s different event received Message:%d \n", __func__,cmd->type);
			}
		} else {
				map_event = (struct evt*)buffer_send;
				DBGPRINT(RT_DEBUG_ERROR, "%s buffer increase confirmation timeout for evtid:%d,len:%d \n", __func__
					,map_event->type,map_event->length);
		}

	}else
		wapp_iface_send(wapp, (char* )buffer_send, len_send, "mapDemo_wapp_App");

	return 0;
}

int map_1905_send_controller(struct wifi_app* wapp, char* buffer_send, int len_send)
{
	wapp_iface_send(wapp, (char* )buffer_send, len_send, "mapDemo_wapp_App");
	return 0;
}

unsigned char get_centre_freq_ch(struct wifi_app *wapp, unsigned char channel, unsigned char op)
{
	if (op == 129) {
		if (channel >= 36 && channel <= 64)
			return 50;
		else if (channel >= 100 && channel <= 128)
			return 114;
		else if (channel >= 149 && channel <= 177)
			return 163;
	}
#ifdef MAP_6E_SUPPORT
	else if (op == 133) {/* BW80 case */
		if (channel > 1 && channel <= 13)
			return 7;
		else if (channel >= 17 && channel <= 29)
			return 23;
		else if (channel >= 33 && channel <= 45)
			return 39;
		else if (channel >= 49 && channel <= 61)
			return 55;
		else if (channel >= 65 && channel <= 77)
			return 71;
		else if (channel >= 81 && channel <= 93)
			return 87;
		else if (channel >= 97 && channel <= 109)
			return 103;
		else if (channel >= 113 && channel <= 125)
			return 119;
		else if (channel >= 129 && channel <= 141)
			return 135;
		else if (channel >= 145 && channel <= 157)
			return 151;
		else if (channel >= 161 && channel <= 173)
			return 167;
		else if (channel >= 177 && channel <= 189)
			return 183;
		else if (channel >= 193 && channel <= 205)
			return 199;
		else if (channel >= 209 && channel <= 221)
			return 215;
	} else if (op == 134) {/* BW160 case */
		if (channel >= 1 && channel <= 29)
			return 15;
		else if (channel >= 33 && channel <= 61)
			return 47;
		else if (channel >= 65 && channel <= 93)
			return 79;
		else if (channel >= 97 && channel <= 125)
			return 111;
		else if (channel >= 129 && channel <= 157)
			return 143;
		else if (channel >= 161 && channel <= 189)
			return 175;
		else if (channel >= 193 && channel <= 221)
			return 207;
	}
#endif
#ifdef MAP_320BW
	else if (op == 137) {
		if (channel >= 1 && channel <= 61) {
			if ((!wapp->HE_EXTCHA) || channel <= 29)
				return 31;
			else
				return 63;
		} else if (channel >= 33 && channel <= 93) {
			if (!wapp->HE_EXTCHA)
				return 63;
			else
				return 95;
		} else if (channel >= 65 && channel <= 125) {
			if (!wapp->HE_EXTCHA)
				return 95;
			else
				return 127;
		} else if (channel >= 97 && channel <= 157) {
			if (!wapp->HE_EXTCHA)
				return 127;
			else
				return 159;
		} else if (channel >= 129 && channel <= 189) {
			if (!wapp->HE_EXTCHA)
				return 159;
			else
				return 191;
		} else if (channel >= 161 && channel <= 221) {
			return 191;
		}
	}
#endif
	else {
		if (channel >= 36 && channel <= 48)
			return 42;
		else if (channel >= 52 && channel <= 64)
			return 58;
		else if (channel >= 100 && channel <= 112)
			return 106;
		else if (channel >= 116 && channel <= 128)
			return 122;
		else if (channel >= 132 && channel <= 144)
			return 138;
		else if (channel >= 149 && channel <= 161)
			return 155;
		else if (channel >= 165 && channel <= 177)
			return 171;
	}
        return 0;
}

int map_operating_channel_echo_msg(
	struct map_info *map, struct channel_setting *setting, char *buf, int* buf_len)
{
	struct evt *wapp_event;
	struct channel_report *ch_rpt = NULL;
	struct ch_rep_info *rep_info = NULL;
	int send_pkt_len = 0;
	int i = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_OPERATING_CHANNEL_REPORT;
	ch_rpt = (struct channel_report *)wapp_event->buffer;

	ch_rpt->ch_rep_num = setting->ch_set_num;
	rep_info = (struct ch_rep_info *)ch_rpt->info;

	for (i = 0; i < setting->ch_set_num; i++) {
		memcpy(rep_info->identifier, setting->chinfo[i].identifier, ETH_ALEN);
		rep_info->op_class = setting->chinfo[i].op_class;
		rep_info->channel = setting->chinfo[i].channel;
		/*test code*/
		rep_info->tx_power = setting->chinfo[i].power;
		rep_info++;
	}

	wapp_event->length = sizeof(struct channel_report) + (setting->ch_set_num * sizeof(struct ch_rep_info));
	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;
	*buf_len = send_pkt_len;

	return 0;
}

u8 get_20M_opclass_from_central_ch(struct wapp_dev *wdev, u8 cent_ch, u8 op_ch)
{
	struct ap_dev *ap = NULL;
#ifdef MAP_6E_SUPPORT
	struct _wdev_op_class_info_ext *op_class = NULL;
#else
	wdev_op_class_info *op_class = NULL;
#endif
	int i, j;

	if (!wdev)
		return 0;

	ap = (struct ap_dev *)wdev->p_dev;
	op_class = &ap->op_class;

#ifdef MAP_6E_SUPPORT
	for (i = 0; i < op_class->num_of_op_class; i++) {
		for (j = 0; j < op_class->opClassInfoExt[i].num_of_ch; j++) {
			if (op_class->opClassInfoExt[i].ch_list[j] == op_ch &&
				(op_class->opClassInfoExt[i].op_class != 128 && op_class->opClassInfoExt[i].op_class != 129
				&& op_class->opClassInfoExt[i].op_class != 133 && op_class->opClassInfoExt[i].op_class != 134
#ifdef MAP_320BW
				&& op_class->opClassInfoExt[i].op_class != 137
#endif
				)) {
				return op_class->opClassInfoExt[i].op_class;
			}
		}
	}
#else
	for (i = 0; i < op_class->num_of_op_class; i++) {
		for (j = 0; j < op_class->opClassInfo[i].num_of_ch; j++) {
			if (op_class->opClassInfo[i].ch_list[j] == op_ch &&
				(op_class->opClassInfo[i].op_class != 128 && op_class->opClassInfo[i].op_class != 129)) {
				return op_class->opClassInfo[i].op_class;
			}
		}
	}
#endif

	return 0;
}


int map_operating_channel_info(
	struct wifi_app *wapp)
{
	char *buf = NULL;
	int len = 0;
	struct channel_report *ch_rpt = NULL;
	struct ch_rep_info *rep_info = NULL;
	struct wapp_radio *ra = NULL;
	int i = 0, num_of_ra = 0;
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	unsigned char radio_id[MAC_ADDR_LEN] = {0};
#ifdef MAP_R4_SPT
	struct ap_spt_reuse_resp *spt_rep_info = NULL;
	struct wapp_mesh_sr_info *spt_reuse_info = NULL;
#endif

	DBGPRINT(RT_DEBUG_ERROR, "%s\n", __func__);

	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		ra = &wapp->radio[i];
		if (ra->op_ch != 0)
		{
			wdev = NULL;
			os_memset(radio_id, 0, MAC_ADDR_LEN);
			MAP_GET_RADIO_IDNFER(ra, radio_id);
			wdev = wapp_dev_list_lookup_by_radio(wapp, (char *) radio_id);
			if (wdev) {
				ap = (struct ap_dev *) wdev->p_dev;
				if (ap->ch_info.op_class == 128 || ap->ch_info.op_class == 129
#ifdef MAP_6E_SUPPORT
					|| ap->ch_info.op_class == 133 || ap->ch_info.op_class == 134
#ifdef MAP_320BW
					|| ap->ch_info.op_class == 137
#endif
#endif
				) {
					num_of_ra += 2;
				} else {
					num_of_ra += 1;
				}
			} else {
				DBGPRINT(RT_DEBUG_ERROR, RED("%s, no wdev match this radio\n"), __func__);
			}
		}
	}
	len = sizeof(struct channel_report) + (num_of_ra * sizeof(struct ch_rep_info));
	buf = os_zalloc(len);

	if (buf == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, RED("%s, alloc memeory fail\n"), __func__);
		return WAPP_RESOURCE_ALLOC_FAIL;
	}

	ch_rpt = (struct channel_report *) buf;
	rep_info = ch_rpt->info;

	ch_rpt->ch_rep_num = num_of_ra;

#ifdef MAP_R4_SPT
	spt_rep_info = ch_rpt->spt_reuse;
	ch_rpt->spt_rep_num = 0;
#endif

	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		ra = &wapp->radio[i];
		if (ra->op_ch != 0)
		{
			wdev = NULL;
			MAP_GET_RADIO_IDNFER(ra, rep_info->identifier);
#ifdef MAP_R4_SPT
			MAP_GET_RADIO_IDNFER(ra, spt_rep_info->identifier);
#endif
			wdev = wapp_dev_list_lookup_by_radio(wapp, (char *) rep_info->identifier);
			if (wdev) {
				ap = (struct ap_dev *) wdev->p_dev;
				rep_info->op_class = ap->ch_info.op_class;
				rep_info->tx_power = ap->pwr.tx_pwr;
#ifdef MAP_R4_SPT
				spt_reuse_info = &ap->sr_info;

				if (spt_reuse_info->sr_mode == 1) {
					spt_rep_info->partial_bss_color = 0;
					spt_rep_info->bss_color = 8;
					spt_rep_info->hesiga_spa_reuse_val_allowed = 0;
					spt_rep_info->srg_info_valid = 1;
					spt_rep_info->nonsrg_offset_valid = 0;
					spt_rep_info->psr_disallowed = 0;
					spt_rep_info->nonsrg_obsspd_max_offset = 110;
					spt_rep_info->srg_obsspd_min_offset = 0;
					spt_rep_info->srg_obsspd_max_offset = 75;
					os_memcpy(spt_rep_info->srg_bss_color_bitmap, &ap->sr_info.bm_info.color_31_0, 8);
					os_memcpy(spt_rep_info->srg_partial_bssid_bitmap, &ap->sr_info.bm_info.bssid_31_0, 8);
					os_memset(spt_rep_info->ngh_bss_color_in_bitmap, 0, 8);
					DBGPRINT_RAW(RT_DEBUG_TRACE, GRN("[wapp bm]")
								"0x%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
						spt_rep_info->srg_bss_color_bitmap[0],
						spt_rep_info->srg_bss_color_bitmap[1],
						spt_rep_info->srg_bss_color_bitmap[2],
						spt_rep_info->srg_bss_color_bitmap[3],
						spt_rep_info->srg_bss_color_bitmap[4],
						spt_rep_info->srg_bss_color_bitmap[5],
						spt_rep_info->srg_bss_color_bitmap[6],
						spt_rep_info->srg_bss_color_bitmap[7]);
					spt_rep_info++;

					ch_rpt->spt_rep_num++;
				}
#endif
			} else {
				DBGPRINT(RT_DEBUG_ERROR, RED("%s, no wdev match this radio\n"), __func__);
			}
			if (rep_info->op_class == 128 || rep_info->op_class == 129
#ifdef MAP_6E_SUPPORT
					|| rep_info->op_class == 133 || rep_info->op_class == 134
#ifdef MAP_320BW
					|| ap->ch_info.op_class == 137
#endif
#endif
				) {
				os_memcpy(radio_id, rep_info->identifier, MAC_ADDR_LEN);
				rep_info->channel = get_centre_freq_ch(wapp, ra->op_ch, rep_info->op_class);
				rep_info++;
				rep_info->op_class = get_20M_opclass_from_central_ch(wdev, rep_info->channel, ra->op_ch); //ap->ch_info.op_class;//20M opclass
				rep_info->tx_power = ap->pwr.tx_pwr;
				rep_info->channel = ra->op_ch; //primary channel
				os_memcpy(rep_info->identifier, radio_id, MAC_ADDR_LEN);//radio id
				rep_info++;
			} else {
				rep_info->channel = ra->op_ch;
				rep_info++;
			}
		}
	}

	if(wdev) {
		ch_rpt->AutoChannelSkipListNum = wapp->AutoChannelSkipListNum;
		for (i = 0; i < wapp->AutoChannelSkipListNum; i++) {
			ch_rpt->AutoChannelSkipList[i] = wapp->AutoChannelSkipList[i];
		}
	}

	wapp_send_1905_msg(wapp, WAPP_OPERATING_CHANNEL_INFO, len, buf);
	os_memset(buf, 0, len);
	os_free(buf);

	return MAP_SUCCESS;
}

#ifdef MAP_R4_SPT
void map_send_ch_select_req_info(
	struct wifi_app *wapp, struct wapp_dev *wdev)
{
	char *buf = NULL;
	int len = 0;
	struct ap_dev *ap = NULL;
	struct wapp_mesh_sr_info *sr_info = NULL;
	struct ap_spt_reuse_req *map_spt_reuse;

	len = sizeof(struct ap_spt_reuse_req);
	buf = os_zalloc(len);

	if (buf == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, RED("%s, alloc memeory fail\n"), __func__);
		return;
	}

	map_spt_reuse = (struct ap_spt_reuse_req *)buf;
	ap = (struct ap_dev *)wdev->p_dev;
	if (ap == NULL) {
		os_free(buf);
		return;
	}

	sr_info = &ap->sr_info;

	if (sr_info->sr_mode == 0) {
		os_free(buf);
		return;
	}

	if (wdev->radio) {
		MAP_GET_RADIO_IDNFER(wdev->radio, map_spt_reuse->identifier);
	}

	map_spt_reuse->bss_color = 8;
	map_spt_reuse->hesiga_spa_reuse_val_allowed = 1;
	map_spt_reuse->srg_info_valid = 1;
	map_spt_reuse->nonsrg_offset_valid = 0;
	map_spt_reuse->psr_disallowed = 0;
	map_spt_reuse->nonsrg_obsspd_max_offset = 75;
	map_spt_reuse->srg_obsspd_min_offset = 0;
	map_spt_reuse->srg_obsspd_max_offset = 110;
	os_memcpy(map_spt_reuse->srg_bss_color_bitmap, (unsigned
		char*)&sr_info->bm_info.color_31_0, SRG_BITMAP_SIZE);
	os_memcpy(map_spt_reuse->srg_partial_bssid_bitmap, (unsigned
		char*)&sr_info->bm_info.bssid_31_0, SRG_BITMAP_SIZE);

	if(wapp_send_1905_msg(wapp, WAPP_CH_SELECTION_INFO, len, buf) < 0){
		DBGPRINT(RT_DEBUG_OFF, "send to map fail");
	}
	os_free(buf);
}

int mapd_receive_uplink_traffic_status(
	struct wifi_app *wapp, char *msg_buf, char *evt_buf, int* buf_len)
{
	struct wapp_dev *wdev = NULL;
	struct uplink_traffic_status *traffic_status = (struct uplink_traffic_status *)msg_buf;

	wdev = wapp_dev_list_lookup_by_band_and_type(wapp, traffic_status->band, WAPP_DEV_TYPE_AP);

	if(!wdev){
		DBGPRINT(RT_DEBUG_ERROR, "Not finding Wdev for band %d\n",traffic_status->band);
		return MAP_ERROR;
	}
	return wapp_set_uplink_traffic_status(wapp, wdev, traffic_status->status);
}

int mapd_receive_sr_topo_info(
	struct wifi_app *wapp, char *msg_buf, char *evt_buf, int* buf_len)
{
	struct wapp_dev *wdev = NULL;
	struct srg_topology_info *sr_topo_info = NULL;
	sr_topo_info = (struct srg_topology_info *)msg_buf;
	struct wapp_mesh_sr_topology sr_topo_update;
	int i;

	for (i = 0; i < sr_topo_info->srg_remote_info_cnt; i++) {
		wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)sr_topo_info->srg_info[i].identifier);

		if(wdev){
			sr_topo_update.map_dev_count = sr_topo_info->map_dev_cnt;
			sr_topo_update.map_dev_sr_support_mode = sr_topo_info->map_dev_sr_mode;
			sr_topo_update.self_role = sr_topo_info->self_mode;
			os_memcpy(sr_topo_update.map_remote_al_mac, sr_topo_info->map_remote_almac, ETH_ALEN);
			os_memcpy(sr_topo_update.map_remote_fh_bssid,
						sr_topo_info->srg_info[i].map_remote_FH_Bssid, ETH_ALEN);
			os_memcpy(sr_topo_update.map_remote_bh_mac, sr_topo_info->map_remote_bh_mac, ETH_ALEN);
			sr_topo_update.ssid_len = sr_topo_info->srg_info[i].ssid_len;
			os_memcpy(sr_topo_update.ssid, sr_topo_info->srg_info[i].ssid,
						sr_topo_info->srg_info[i].ssid_len);
			wapp_set_sr_topo_info(wapp, wdev, &sr_topo_update);
		}
	}
	return MAP_SUCCESS;
}

int mapd_receive_operating_sr_rpt(
	struct wifi_app *wapp, char *msg_buf, char *evt_buf, int* buf_len)
{
	struct wapp_dev *wdev = NULL;
	struct ap_spt_reuse *operaing_sr_rpt= NULL;
	operaing_sr_rpt = (struct ap_spt_reuse *)msg_buf;
	struct ap_spt_reuse_req*sr_bm_radio = (struct ap_spt_reuse_req*)operaing_sr_rpt->spt_reuse_req;
	int i;

	for (i = 0; i < operaing_sr_rpt->spt_req_cnt; i++) {
		wdev = wapp_dev_list_lookup_by_radio(wapp, (char *) sr_bm_radio->identifier);
		if(!wdev)
			continue;
		DBGPRINT(RT_DEBUG_ERROR, GRN("[wapp bm]")"index %d "
							"0x%2x:%2x:%2x:%2x:%2x:%2x:%2x:%2x\n",wdev->ifindex,
					sr_bm_radio->srg_bss_color_bitmap[0],
					sr_bm_radio->srg_bss_color_bitmap[1],
					sr_bm_radio->srg_bss_color_bitmap[2],
					sr_bm_radio->srg_bss_color_bitmap[3],
					sr_bm_radio->srg_bss_color_bitmap[4],
					sr_bm_radio->srg_bss_color_bitmap[5],
					sr_bm_radio->srg_bss_color_bitmap[6],
					sr_bm_radio->srg_bss_color_bitmap[7]);
		wapp_set_sr_bitmap(wapp, wdev, sr_bm_radio->srg_bss_color_bitmap,
							sr_bm_radio->srg_partial_bssid_bitmap);
		sr_bm_radio++;
	}
	return MAP_SUCCESS;
}

int mapd_recieve_spt_reuse_req(
	struct wifi_app *wapp, char *msg_buf, char *evt_buf, int* buf_len)
{
	struct wapp_dev *wdev = NULL;
	struct ap_spt_reuse *spt_req = NULL;
	struct ap_dev *ap = NULL;
	spt_req = (struct ap_spt_reuse *)msg_buf;
	struct ap_spt_reuse_req *spt_reuse_radio = (struct ap_spt_reuse_req *)spt_req->spt_reuse_req;
	int i;
	struct dl_list *dev_list;
	unsigned char wdev_identifier[ETH_ALEN];
	char cmd[MAX_CMD_MSG_LEN] = {0};
	int idx = 0;
	int ret = 0;
	struct wapp_mesh_sr_info *sr_info = NULL;

	for (i = 0; i < spt_req->spt_req_cnt; i++)
	{
		if (wapp->map->MapMode == 4) {
			dev_list = &wapp->dev_list;
			idx = 0;

			dl_list_for_each(wdev, dev_list, struct wapp_dev, list)
			{
				MAP_GET_RADIO_IDNFER(wdev->radio, wdev_identifier);

				DBGPRINT(RT_DEBUG_TRACE, RED("spt: %02x:%02x:%02x:%02x:%02x:%02x\n"),
							PRINT_MAC(spt_reuse_radio->identifier));
				DBGPRINT(RT_DEBUG_TRACE, GRN("wev %02x:%02x:%02x:%02x:%02x:%02x\n"),
							PRINT_MAC(wdev_identifier));
				if (!os_memcmp(wdev_identifier, (char *)spt_reuse_radio->identifier, ETH_ALEN)) {
					ap = (struct ap_dev *) wdev->p_dev;
					os_memcpy((unsigned char *)&ap->sr_info.bm_info.color_31_0,
							&spt_reuse_radio->srg_bss_color_bitmap, SRG_BITMAP_SIZE);
					os_memcpy((unsigned char *)&ap->sr_info.bm_info.bssid_31_0,
							&spt_reuse_radio->srg_partial_bssid_bitmap, SRG_BITMAP_SIZE);
					wapp_set_sr_bitmap(wapp, wdev, spt_reuse_radio->srg_bss_color_bitmap,
							spt_reuse_radio->srg_partial_bssid_bitmap);
					ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set color_dbg=%d:6:%d;",
							wdev->ifname, idx++, (int) spt_reuse_radio->bss_color);
					if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
						DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
					DBGPRINT(RT_DEBUG_TRACE, "%s\n", cmd);
				}
			}
		} else {
			wdev = wapp_dev_list_lookup_by_radio(wapp, (char *) spt_reuse_radio->identifier);
			if (!wdev)
				continue;
			ap = (struct ap_dev *)wdev->p_dev;
			if (ap == NULL)
				continue;
			sr_info = &ap->sr_info;
			if (sr_info->sr_mode == 0)
				continue;

			wapp_set_sr_bitmap(wapp, wdev, spt_reuse_radio->srg_bss_color_bitmap,
								spt_reuse_radio->srg_partial_bssid_bitmap);
			spt_reuse_radio++;
		}
	}
	return 0;
}
#endif

unsigned int extract_and_set_channel(struct wifi_app *wapp, struct wapp_dev *wdev,
		struct ch_config_info *ch_info, struct channel_setting *setting, u8 radio_band)
{
	unsigned char channel;
	unsigned int primary_ch_80M = 0;
	u8 bw = BW_20;
	int i = 0, ret;
	char cmd[MAX_CMD_MSG_LEN] = {0};

	if (ch_info->channel) {
#ifndef MAP_6E_SUPPORT
		if ((ch_info->channel > 14 && radio_band != RADIO_24G) ||
			(ch_info->channel <= 14 && radio_band == RADIO_24G))
#else
		if ((IS_OP_CLASS_5G(ch_info->op_class) &&
			(radio_band == RADIO_5G || radio_band == RADIO_5GL || radio_band == RADIO_5GH)) ||
			(IS_OP_CLASS_24G(ch_info->op_class) && radio_band == RADIO_24G) ||
			(IS_OP_CLASS_6G(ch_info->op_class) && radio_band == RADIO_6G))
#endif
		{
			channel = ch_info->channel;
			if (((ch_info->channel > 14) && ((ch_info->op_class == 128)
				|| (ch_info->op_class == 129)))
#ifdef MAP_6E_SUPPORT
				|| (IS_MAP_CH_6G(ch_info->channel) && ((ch_info->op_class == 133)
				|| (ch_info->op_class == 134)
#ifdef MAP_320BW
				|| (ch_info->op_class == 137)
#endif
			))
#endif
			) {
				/* Get actual primary channel provided in 2nd chinfo */
				if (i < (setting->ch_set_num - 1)) {
					DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"channel: %u, I+ch:%u, I+op: %u\n", ch_info->channel,
						setting->chinfo[i+1].channel, setting->chinfo[i+1].op_class);
					if (ch_info->channel == get_centre_freq_ch(wapp, setting->chinfo[i+1].channel,
						ch_info->op_class)) {
						channel = setting->chinfo[i+1].channel;
						primary_ch_80M = 1;
						DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"%s: Primay CH %d center CH %d Op: %u\n",
							__func__, channel, setting->chinfo[i].channel, setting->chinfo[i+1].op_class);
					}
				}
			}
			if (channel != wdev->radio->op_ch) {
#ifdef MAP_R2
				if (ch_info->reason_code & DFS_CH_CLEAR_INDICATION)
					wdev->cac_not_required = 1;
				else
					wdev->cac_not_required = 0;
#endif
				if (WMODE_CAP_AX(wdev->wireless_mode)) {
					bw = chan_mon_get_vht_bw_from_op_class(ch_info->op_class);
#if NL80211_SUPPORT
					wapp_set_vhtbw(wapp, (const char *)wdev->ifname,
										(char *)&bw,
										(size_t)sizeof(bw));
#else
					ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set VhtBw=%d;", wdev->ifname, bw);
					if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
						DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
					DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);
#endif /* NL80211_SUPPORT */

					bw = chan_mon_get_ht_bw_from_op_class(ch_info->op_class);
#if NL80211_SUPPORT
					wapp_set_htbw(wapp, (const char *)wdev->ifname,
										(char *)&bw,
										(size_t)sizeof(bw));
#else
					ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set HtBw=%d;", wdev->ifname, bw);
					if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
						DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
					DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);
#endif /* NL80211_SUPPORT */
				} else if (WMODE_CAP_AC(wdev->wireless_mode)) {
					bw = chan_mon_get_vht_bw_from_op_class(ch_info->op_class);
#if NL80211_SUPPORT
					wapp_set_vhtbw(wapp, (const char *)wdev->ifname,
										(char *)&bw,
										(size_t)sizeof(bw));
#else
					ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set VhtBw=%d;", wdev->ifname, bw);
					if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
						DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
					DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);
#endif /* NL80211_SUPPORT */
				} else {
					bw = chan_mon_get_ht_bw_from_op_class(ch_info->op_class);
#if NL80211_SUPPORT
					wapp_set_htbw(wapp, (const char *)wdev->ifname,
										(char *)&bw,
										(size_t)sizeof(bw));
#else
					ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set HtBw=%d;", wdev->ifname, bw);
					if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
						DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
					DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);
#endif /* NL80211_SUPPORT */
				}

				wdev_set_ch(wapp, wdev, channel, ch_info->op_class);
				DBGPRINT(RT_DEBUG_OFF,
								"Config ifname = %s, channel = %d\n", ch_info->ifname, ch_info->channel);
			} else {
				DBGPRINT(RT_DEBUG_OFF,
							"same channel with wdev->radio (%d), dont bother to swtich\n", channel);
			}
		} else {
			DBGPRINT(RT_DEBUG_ERROR, "wrong ch:%d to radio "MACSTR"\n",
						ch_info->channel, MAC2STR(ch_info->identifier));
		}

		/* set pwr limit */
		if (ch_info->power)
			wdev_ap_set_txpwr_limit(wapp, wdev, ch_info->power, ch_info->op_class);
	}

	return primary_ch_80M;

}

int map_config_channel_setting_msg(
	struct wifi_app *wapp, char *msg_buf, char *evt_buf, int* buf_len)
{
	struct wapp_dev *wdev = NULL;
	struct ch_config_info *ch_info = NULL;
	struct channel_setting *setting = NULL;
	int i = 0;
	u8 radio_band = 0;
	unsigned char wdev_identifier[ETH_ALEN];
	struct dl_list *dev_list;
	unsigned int primary_ch_80M = 0;

	setting = (struct channel_setting *)msg_buf;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	// TODO: This channel report is not real, it should take from driver after channel settings.
	/*send channel selection report*/
	map_operating_channel_echo_msg(wapp->map, setting, evt_buf, buf_len);

	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"ch_num: %u\n", setting->ch_set_num);

	while (i < setting->ch_set_num) {
		ch_info = &setting->chinfo[i];
		dev_list = &wapp->dev_list;
		if (!dl_list_empty(dev_list)){
			dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
				if(wdev->radio) {
					radio_band = 0;
					MAP_GET_RADIO_IDNFER(wdev->radio, wdev_identifier);
					if (wdev->radio->radio_band)
						radio_band = *wdev->radio->radio_band;
					if(!os_memcmp(wdev_identifier, (char *)ch_info->identifier, ETH_ALEN) &&
						(wdev->dev_type == WAPP_DEV_TYPE_AP)) {
#ifdef MAP_R3
						if (wapp->map->MapMode == 4 && (wdev->is_configured != TRUE))
							continue;
#endif
						primary_ch_80M = extract_and_set_channel(wapp, wdev, ch_info, setting, radio_band);
					}
				}
			}
		}
		if(primary_ch_80M) {
			i++;
			primary_ch_80M = 0;
		}
		i++;
	}
	map_operating_channel_info(wapp);
	return MAP_SUCCESS;
}

int map_config_tx_power_percentage_msg(
	struct wifi_app *wapp, char *msg_buf, char *evt_buf, int* buf_len)
{
	struct tx_power_percentage_setting *tx_power_setting = NULL;
	struct dl_list *dev_list;
	struct wapp_dev *wdev = NULL;
	u8 radio_band = 0;

	tx_power_setting = (struct tx_power_percentage_setting *)msg_buf;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(tx_power_setting->tx_power_percentage > 100) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid tx_power percentage!\n");
		return MAP_ERROR;
	}

	if (!((tx_power_setting->bandIdx == BAND_24G) || (tx_power_setting->bandIdx == BAND_5G)
#ifdef MAP_6E_SUPPORT
		|| (tx_power_setting->bandIdx == BAND_6G)
#endif
		)) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid band %d!\n",tx_power_setting->bandIdx);
		return MAP_ERROR;
	}

	dev_list = &wapp->dev_list;
	if (!dl_list_empty(dev_list)) {
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
			if(wdev->radio) {
				radio_band = 0;
				if (wdev->radio->radio_band)
					radio_band = *wdev->radio->radio_band;
				else
					continue;
				if (radio_band == RADIO_24G)
					radio_band = BAND_24G;
				else if((radio_band == RADIO_5GL) || (radio_band == RADIO_5GH) || (radio_band == RADIO_5G))
					radio_band = BAND_5G;
#ifdef MAP_6E_SUPPORT
				else if (radio_band == RADIO_6G)
					radio_band = BAND_6G;
#endif
				if(radio_band == tx_power_setting->bandIdx) {
					DBGPRINT(RT_DEBUG_TRACE," Setting tx power percentage for %s\n", wdev->ifname);
					wapp_set_tx_power_percentage(wapp, wdev, (u8)tx_power_setting->tx_power_percentage);
					break;
				}
			}
		}
	}
	return MAP_SUCCESS;
}


/* WPA3 To Work with MAP_R1*/
#ifdef MAP_SUPPORT
static void wapp_get_auth_string(short authmode, char *auth_str) {

	int ret;
#ifdef MAP_R3
	if ((authmode & WPS_AUTH_SAE) && (authmode & WPS_AUTH_WPA2PSK)
		&& (authmode & AUTH_DPP_ONLY))
		os_strlcpy(auth_str, "DPPWPA3PSKWPA2PSK", sizeof("DPPWPA3PSKWPA2PSK"));
	else if ((authmode & WPS_AUTH_SAE) && (authmode & AUTH_DPP_ONLY))
		os_strlcpy(auth_str, "DPPWPA3PSK", sizeof("DPPWPA3PSK"));
	else if ((authmode & WPS_AUTH_WPA2PSK) && (authmode & AUTH_DPP_ONLY))
		os_strlcpy(auth_str, "DPPWPA2PSK", sizeof("DPPWPA2PSK"));
	else
#endif /* MAP_R3 */
	if ((authmode & WPS_AUTH_SAE) && (authmode & WPS_AUTH_WPA2PSK)) {
		ret = snprintf(auth_str, 20, "WPA2PSKWPA3PSK");
		if (os_snprintf_error(20, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	} else if (authmode & WPS_AUTH_SAE) {
		ret = snprintf(auth_str, 20, "WPA3PSK");
		if (os_snprintf_error(20, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	} else if ((authmode & WPS_AUTH_WPA2PSK) && (authmode & WPS_AUTH_WPAPSK)) {
		ret = snprintf(auth_str, 20, "WPAPSKWPA2PSK");
		if (os_snprintf_error(20, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	} else if (authmode & WPS_AUTH_WPA2PSK) {
		ret = snprintf(auth_str, 20, "WPA2PSK");
		if (os_snprintf_error(20, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	} else if (authmode & WPS_AUTH_WPA2) {
		ret = snprintf(auth_str, 20, "WPA2");
		if (os_snprintf_error(20, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	} else if (authmode & WPS_AUTH_WPA) {
		ret = snprintf(auth_str, 20, "WPA");
		if (os_snprintf_error(20, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	} else if (authmode & WPS_AUTH_SHARED) {
		ret = snprintf(auth_str, 20, "SHARED");
		if (os_snprintf_error(20, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	} else if (authmode & WPS_AUTH_WPAPSK) {
		ret = snprintf(auth_str, 20, "WPAPSK");
		if (os_snprintf_error(20, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	} else if (authmode & WPS_AUTH_WPAPSK) {
		ret = snprintf(auth_str, 20, "WPAPSK");
		if (os_snprintf_error(20, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	} else if (authmode & WPS_AUTH_OPEN) {
		ret = snprintf(auth_str, 20, "OPEN");
		if (os_snprintf_error(20, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	}
}

int fill_sec_info(struct sec_info *sec, struct wireless_setting *pconf)
{
	char auth_str[20] = {0};
	char encryp_str[][10] = {
		{"NONE"},
		{"WEP"},
		{"TKIP"},
		{"AES"},
		{"TKIPAES"}
	};
	int j = 0;
	unsigned short authmode = pconf->AuthMode;
	unsigned short encryptype = pconf->EncrypType;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if ((authmode < WPS_AUTH_OPEN || authmode > (WPS_AUTH_SAE|WPS_AUTH_WPA2PSK
#ifdef MAP_R3
		|AUTH_DPP_ONLY
#endif /* MAP_R3 */
	)) ||
		(encryptype < WPS_ENCR_NONE || encryptype > WPS_ENCR_AES)) {

		if ((authmode == WPS_AUTH_MIXED || authmode == WPS_AUTH_WPA2PSK ||
			authmode == WPS_AUTH_WPAPSK) && encryptype == WPS_ENCR_AESTKIP)
		{
			DBGPRINT(RT_DEBUG_TRACE, "Mixed mode security for "MACSTR"\n", MAC2STR(pconf->mac_addr));
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, "invalid sec_info for "MACSTR"\n", MAC2STR(pconf->mac_addr));
			return MAP_ERROR;
		}

	}

	wapp_get_auth_string(authmode, auth_str);
	memcpy(sec->auth, auth_str, strlen(auth_str));

	if (encryptype == WPS_ENCR_AESTKIP)
	{
		j = 4;
	}
	else {

		while (encryptype) {
			encryptype = encryptype >> 1;
			j++;
		}
	j--;
	}


	memcpy(sec->encryp, encryp_str[j], strlen(encryp_str[j]));
	memcpy(sec->psphr, pconf->WPAKey, sizeof(pconf->WPAKey));

	return MAP_SUCCESS;
}

#else

int fill_sec_info(struct sec_info *sec, struct wireless_setting *pconf)
{
	char *auth_str[] = {
		"OPEN",
		"WPAPSK",
		"SHARED",
		"WPA",
		"WPA2",
		"WPA2PSK",
		"WPAPSKWPA2PSK"
	};
	char *encryp_str[] = {
		"NONE",
		"WEP",
		"TKIP",
		"AES",
		"TKIPAES"
	};
	int i = 0, j = 0;
	unsigned short authmode = pconf->AuthMode;
	unsigned short encryptype = pconf->EncrypType;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if ((authmode < WPS_AUTH_OPEN || authmode > WPS_AUTH_WPA2PSK) ||
		(encryptype < WPS_ENCR_NONE || encryptype > WPS_ENCR_AES)) {

		if ((authmode == WPS_AUTH_MIXED || authmode == WPS_AUTH_WPA2PSK ||
			authmode == WPS_AUTH_WPAPSK) && encryptype == WPS_ENCR_AESTKIP)
		{
			DBGPRINT(RT_DEBUG_TRACE, "%s, Mixed mode security\n", __func__);
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, "%s, invalid sec_info\n", __func__);
			return MAP_ERROR;
		}
	}

	if (authmode == WPS_AUTH_MIXED)
	{
		i = 6;
	}
	else
	{
		while (authmode) {
			authmode = authmode >> 1;
			i++;
		}
		i--;
	}

	memcpy(sec->auth, auth_str[i], strlen(auth_str[i]));
	DBGPRINT(RT_DEBUG_OFF, "%s, auth(%d) = %s len=%d\n", __func__, i, sec->auth, (UINT32)sizeof(auth_str[i]));

	if (encryptype == WPS_ENCR_AESTKIP)
	{
		j = 4;
	}
	else
	{
		while (encryptype) {
			encryptype = encryptype >> 1;
			j++;
		}
		j--;
	}

	memcpy(sec->encryp, encryp_str[j], strlen(encryp_str[j]));
	DBGPRINT(RT_DEBUG_OFF, "%s, encryptype(%d) = %s\n", __func__, j, sec->encryp);
	memcpy(sec->psphr, pconf->WPAKey, sizeof(pconf->WPAKey));

	return MAP_SUCCESS;
}
#endif
int map_receive_bssload_query_msg(
	struct wifi_app *wapp, unsigned char *bss_addr, char *msg_buf)
{
	struct wapp_dev *wdev = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bss_addr, WAPP_DEV_TYPE_AP);
	if (wdev)
		wapp_query_bssload(wapp, wdev);
	else
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);

	return MAP_SUCCESS;
}

int map_receive_he_cap_query_msg(
	struct wifi_app *wapp, unsigned char *bss_addr, char *evt_buf, int* len_buf)
{
	struct wapp_dev *wdev = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bss_addr, WAPP_DEV_TYPE_AP);
	if (wdev)
		wapp_query_he_cap(wapp, wdev);
	else
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);

	//*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}

int map_receive_set_vendor_ie(
	struct wifi_app *wapp, unsigned char *bss_addr, char *msg_buf)
{
	int ret = 0;
	struct wapp_dev *wdev = NULL;
	int vender_ie_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bss_addr, WAPP_DEV_TYPE_AP);
	if (wdev) {
		vender_ie_len = msg_buf[1] + 2; // Tag + length
		ret = wapp_set_ie(wapp, wdev->ifname, msg_buf, vender_ie_len);
	} else
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);

	return ret;
}

int map_receive_apcli_rssi_query_msg(
	struct wifi_app *wapp, unsigned char *bss_addr, unsigned char *apcli_addr)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bss_addr, WAPP_DEV_TYPE_AP);
	if (wdev)
		wapp_query_apcli_rssi(wapp, wdev, apcli_addr);
	else
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);

	return MAP_SUCCESS;
}
#ifdef MAP_SUPPORT

unsigned char air_monitor_entry_check(struct wifi_app *wapp,
	unsigned char *sta_mac, unsigned char channel)
{
	struct sta_mnt_stat *sta_stat = NULL;
	struct wapp_dev *wdev = NULL;
	unsigned char wdev_found = 0;

	/*check if entry is present in existing list*/
	if (!dl_list_empty(&wapp->sta_mntr_list)){
		dl_list_for_each(sta_stat, &wapp->sta_mntr_list, struct sta_mnt_stat, list) {
			if((!os_memcmp(sta_mac,sta_stat->sta_mac,MAC_ADDR_LEN))
				&& (sta_stat->Channel == channel)) {
				sta_stat->mnt_reference++;
				DBGPRINT(RT_DEBUG_OFF, "%s STA Entry found\n", __func__);
				return 0;
			}
		}
	}

	/*Search for wdev and channel match*/
	dl_list_for_each(wdev, &wapp->dev_list, struct wapp_dev, list){
		if (wdev->radio && wdev->radio->op_ch == channel &&
			wdev->dev_type == WAPP_DEV_TYPE_AP) {
			wdev_found = TRUE;
			break;
		}
	}
#ifdef WIFI_MD_COEX_SUPPORT
	if (wdev_found) {
		if (!wdev->radio->operatable) {
			DBGPRINT(RT_DEBUG_OFF, "%s Not operatable on current radio\n", __func__);
			return -110;
		}
	}
#endif
	if(wdev_found) {
		/*create New Entry*/
		sta_stat = (struct sta_mnt_stat *)os_zalloc(sizeof(struct sta_mnt_stat));
		if(sta_stat == NULL) {
			DBGPRINT(RT_DEBUG_OFF, "%s Memory Alloc Fail\n", __func__);
			return -110;
		}
		DBGPRINT(RT_DEBUG_ERROR, "%s STA Entry Add"MACSTR"\n", __func__, MAC2STR(sta_mac));
		os_memcpy(sta_stat->sta_mac, sta_mac, MAC_ADDR_LEN);
		sta_stat->Channel = channel;
#if NL80211_SUPPORT
		map_set_air_mnt_cmd(wapp, wdev, sta_mac, 1, 6);
#else
		map_set_air_mnt_cmd(wapp, wdev, sta_mac, 1, 6);
#endif /* NL80211_SUPPORT */
		sta_stat->mnt_state = MONITOR_ONGOING;
		sta_stat->mnt_reference++;
		dl_list_add_tail(&wapp->sta_mntr_list, &sta_stat->list);
	} else {
		DBGPRINT(RT_DEBUG_OFF, "%s Channel mismatch wdev not found\n", __func__);
		return -110;
	}
	return 0;
}


void send_air_monitor_response(struct wifi_app *wapp, struct air_monitor_query_rsp  *metrics_rsp)
{
	char *buf = NULL;
	unsigned int send_pkt_len = 0, i;

	send_pkt_len = sizeof (struct unlink_metrics_rsp) +
		(metrics_rsp->unlink_metric.sta_num * sizeof(struct unlink_rsp_sta));
	buf = os_zalloc(send_pkt_len);
	if(buf == NULL) {
		DBGPRINT(RT_DEBUG_OFF, "%s Memory Alloc Fail Can't Send NAC Response\n",
			__func__);
		return;
	}
	metrics_rsp->bitmap |= BIT(AIR_MONITOR_RESPONSE_DONE_BIT);
	os_memcpy(buf,&metrics_rsp->unlink_metric,send_pkt_len);
	for (i = 0; i < metrics_rsp->unlink_metric.sta_num; i++)
		DBGPRINT(RT_DEBUG_ERROR, "%s Air mon Response sta [%d] ["MACSTR"] RSSI %d\n", __func__, i,
					MAC2STR(metrics_rsp->unlink_metric.info[i].mac), metrics_rsp->unlink_metric.info[i].uplink_rssi);

	wapp_send_1905_msg(wapp, WAPP_AIR_MONITOR_REPORT, send_pkt_len, buf);
	os_free(buf);
}
void bh_steering_ready_timeout(void *eloop_data, void *user_ctx)
{
	struct wifi_app *wapp = (struct wifi_app *)eloop_data;
	struct wapp_dev *wdev = (struct wapp_dev *)user_ctx;
#if NL80211_SUPPORT
	char bssid[] = "00:00:00:00:00:00";
#else
	char cmd[MAX_CMD_MSG_LEN] = {0};
	int ret;
#endif

	if(wapp->map && wapp->map->bh_link_ready && wdev) {
		DBGPRINT(RT_DEBUG_OFF, "%s BH link ready\n", __func__);
#if NL80211_SUPPORT
		wapp_set_bssid(wapp, (const char *)wdev->ifname,
				(char *)bssid, MAC_ADDR_LEN);
#else
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
		ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ApCliBssid=00:00:00:00:00:00;", wdev->ifname);
		if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		DBGPRINT(RT_DEBUG_ERROR," Send Command: %s\n",cmd);
		if (system(cmd) == -1)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif
		return;
	}
	DBGPRINT(RT_DEBUG_INFO, "%s bh steering timeout scheduled\n", __func__);
	eloop_register_timeout(1, 0, bh_steering_ready_timeout, wapp, wdev);
}
static void update_rssi_for_timeout(struct wifi_app *wapp)
{
	struct air_monitor_query_rsp  *metrics_rsp, *metric_rsp_tmp = NULL;
	struct sta_mnt_stat *sta_stat = NULL, *sta_stat_tmp = NULL;
	struct unlink_rsp_sta *unlink_rsp;
	int i = 0;

	if (!dl_list_empty(&wapp->sta_mntr_list)) {
		dl_list_for_each_safe(sta_stat, sta_stat_tmp, &wapp->sta_mntr_list, struct sta_mnt_stat, list) {
		if (!dl_list_empty(&wapp->air_monitor_query_list)) {
			dl_list_for_each_safe(metrics_rsp, metric_rsp_tmp, &wapp->air_monitor_query_list,
				struct air_monitor_query_rsp, list) {
			for (i = 0; i < metrics_rsp->unlink_metric.sta_num; i++) {
				unlink_rsp = &metrics_rsp->unlink_metric.info[i];
				if (!os_memcmp(sta_stat->sta_mac, unlink_rsp->mac, MAC_ADDR_LEN) &&
					(sta_stat->mnt_cnt > 0)) {
					if (wapp->air_mnt_max_pkt == 0)
						unlink_rsp->uplink_rssi = (signed char)((sta_stat->avg_rssi)/
											(signed char)(sta_stat->mnt_cnt));
					else
						unlink_rsp->uplink_rssi = (signed char)sta_stat->avg_rssi;
					DBGPRINT(RT_DEBUG_OFF, "Idx:%d STA:"MACSTR" RSSI:%d pkt cnt:%d\n",
					i, MAC2STR(unlink_rsp->mac), unlink_rsp->uplink_rssi, sta_stat->mnt_cnt);
				}
			}
			}
		}
		}
	}
}

void air_monitor_query_timeout(void *eloop_data, void *user_ctx)
{
	struct wifi_app *wapp = (struct wifi_app *)eloop_data;
	struct wapp_dev *wdev, *wdev_tmp;
	struct air_monitor_query_rsp  *metrics_rsp = (struct air_monitor_query_rsp *)user_ctx;
	wapp_mnt_info mnt_info[MAX_NUM_OF_MONITOR_STA];
	u8 channel, i;
	size_t length = MAX_NUM_OF_MONITOR_STA * sizeof(wapp_mnt_info);
	struct sta_mnt_stat *sta = NULL;

	if (wapp->air_mnt_max_pkt) {
		sta = dl_list_first(&wapp->sta_mntr_list, struct sta_mnt_stat, list);
		if (sta == NULL)
			goto clean;
		channel = sta->Channel;

		DBGPRINT(RT_DEBUG_ERROR, "%s Air Monitor Query Timeout\n",
			__func__);

		dl_list_for_each_safe(wdev, wdev_tmp, &wapp->dev_list, struct wapp_dev, list) {
			if (wdev->radio && wdev->radio->op_ch == channel &&
				wdev->dev_type == WAPP_DEV_TYPE_AP) {
				break;
			}
		}
		wapp_get_air_mnt_results(wapp, wdev->ifname, (char *) mnt_info, length);
		for (i = 0; i < MAX_NUM_OF_MONITOR_STA; i++)
			update_sta_rssi(wapp, wdev, &mnt_info[i]);
	}
clean:
	update_rssi_for_timeout(wapp);
	/*Send Response to 1905 Device*/
	send_air_monitor_response(wapp, metrics_rsp);
	/*Clear NAC Request List*/
	clear_air_monitor_req_list(wapp);
	clear_monitor_list(wapp);
}
void map_wps_timeout(void *eloop_data, void *user_ctx)
{
	struct wifi_app *wapp = (struct wifi_app *)eloop_data;
	wapp_device_status *device_status = (wapp_device_status *)user_ctx;
	unsigned int need_send_status = FALSE;
	DBGPRINT(RT_DEBUG_ERROR, "%s WPS trigger timeout\n",
		__func__);

	if((device_status->status_bhsta != STATUS_BHSTA_WPS_TRIGGERED)
		&& (device_status->status_fhbss != STATUS_FHBSS_WPS_TRIGGERED))
		return;

	wapp->wps_on_controller_cli = 0;
	if (device_status->status_bhsta == STATUS_BHSTA_WPS_TRIGGERED) {
		//wapp->map->ctrler_found = 0;
		device_status->status_bhsta = STATUS_BHSTA_WPS_FAILED;
		stop_con_cli_wps(wapp, NULL);
		need_send_status = TRUE;
	} else if (device_status->status_fhbss == STATUS_FHBSS_WPS_TRIGGERED){
		device_status->status_fhbss = STATUS_FHBSS_WPS_FAILED;
		need_send_status = TRUE;
		wapp->map->WPS_Fh_Fail=1;
	}

	if(wapp->wsc_save_bh_profile){
		wsc_apcli_config_msg apcli_config_msg;
		wapp->wsc_save_bh_profile = FALSE;
		save_map_parameters(wapp, "BhProfile0Valid", "1", NON_DRIVER_PARAM);
		device_status->status_bhsta = STATUS_BHSTA_UNCONFIGURED;
		apcli_config_msg.profile_count = WSC_APCLI_CONFIG_MSG_USE_SAVED_PROFILE;
		wapp_send_1905_msg(wapp, WAPP_MAP_BH_CONFIG, sizeof(apcli_config_msg), (void *)&apcli_config_msg);
	}

	if(need_send_status) {
		wapp_send_1905_msg(
			wapp,
			WAPP_DEVICE_STATUS,
			sizeof(wapp_device_status),
			(char *)device_status);
		}
}

unsigned char check_for_monitor_complettion(struct air_monitor_query_rsp  *metrics_rsp)
{
	int i;

	/*Check if need to wait for Results*/
	for(i=0 ; i < metrics_rsp->unlink_metric.sta_num; i++) {
		if(!(metrics_rsp->bitmap & BIT(i)))
			break;
	}
	if(i == metrics_rsp->unlink_metric.sta_num)
		return 1;
	 else
		return 0;

}
int update_sta_rssi(struct wifi_app *wapp, struct wapp_dev *wdev, wapp_mnt_info *mnt_info)
{
	struct sta_mnt_stat *sta_stat = NULL;

	if (!dl_list_empty(&wapp->sta_mntr_list)){
		dl_list_for_each(sta_stat, &wapp->sta_mntr_list, struct sta_mnt_stat, list) {
			if((!os_memcmp(mnt_info->sta_addr,sta_stat->sta_mac,MAC_ADDR_LEN))
				&& (sta_stat->Channel == wdev->radio->op_ch)) {
				sta_stat->avg_rssi += mnt_info->rssi;
				sta_stat->mnt_cnt += wapp->air_mnt_max_pkt ? wapp->air_mnt_max_pkt : 1;
				DBGPRINT(RT_DEBUG_OFF, "STA:%02x:%02x:%02x:%02x:%02x:%02x Running Avg Rssi:%d\n",
					PRINT_MAC(sta_stat->sta_mac), sta_stat->avg_rssi);
				return 0;
			}
		}
	}
	return -1;
}

void clear_monitor_list(struct wifi_app * wapp)
{
	struct sta_mnt_stat *sta_stat = NULL, *sta_stat_tmp;
	struct wapp_dev *wdev = NULL, *wdev_tmp;
	unsigned char wdev_found = 0;
	struct dl_list *dev_list = &wapp->dev_list;
	if (!dl_list_empty(&wapp->sta_mntr_list)){
		dl_list_for_each_safe(sta_stat, sta_stat_tmp, &wapp->sta_mntr_list, struct sta_mnt_stat, list) {
		if (sta_stat->mnt_reference <= 0) {
			if (!dl_list_empty(dev_list)) {
				dl_list_for_each_safe(wdev, wdev_tmp, &wapp->dev_list, struct wapp_dev, list) {
					if (wdev->radio && wdev->radio->op_ch == sta_stat->Channel &&
						wdev->dev_type == WAPP_DEV_TYPE_AP) {
						wdev_found = TRUE;
						break;
					}
				}
			}
			dl_list_del(&sta_stat->list);
			os_free(sta_stat);
		}
		}
		if (dl_list_empty(&wapp->sta_mntr_list) && wdev_found)
			map_set_air_mnt_cmd(wapp, wdev, NULL, 0, 6);
	}
}

void clear_air_monitor_req_list(struct wifi_app * wapp)
{
	struct air_monitor_query_rsp *metrics_rsp = NULL, *metrics_rsp_tmp;
	struct unlink_rsp_sta *info;
	struct sta_mnt_stat *sta_stat = NULL, *sta_stat_tmp;
	unsigned int i=0;

	if (!dl_list_empty(&wapp->sta_mntr_list)){
		dl_list_for_each_safe(metrics_rsp, metrics_rsp_tmp, &wapp->air_monitor_query_list,
								struct air_monitor_query_rsp, list) {
		if (metrics_rsp->bitmap & BIT(AIR_MONITOR_RESPONSE_DONE_BIT)) {
			/*Clear STA reference Here*/
			for (i = 0 ; i < metrics_rsp->unlink_metric.sta_num ; i++) {
				info = &metrics_rsp->unlink_metric.info[i];
				if (!dl_list_empty(&wapp->sta_mntr_list)) {
					dl_list_for_each_safe(sta_stat, sta_stat_tmp, &wapp->sta_mntr_list, struct sta_mnt_stat, list) {
					if ((!os_memcmp(info->mac, sta_stat->sta_mac, MAC_ADDR_LEN))
					&& (sta_stat->Channel == info->ch)) {
					if (sta_stat->mnt_reference > 0)
						sta_stat->mnt_reference--;
					}
					}
				}
			}
			dl_list_del(&metrics_rsp->list);
			os_free(metrics_rsp);
		}
		}
	}
	wapp->air_mnt_max_pkt = 0;
	air_mnt_map = 0;
}

void send_air_monitor_response_check(struct wifi_app *wapp, wapp_mnt_info *mnt_info)
{
	struct air_monitor_query_rsp  *metrics_rsp, *metrics_rsp_tmp;
	struct sta_mnt_stat *sta_stat = NULL, *sta_stat_tmp;
	struct unlink_rsp_sta *unlink_rsp;
	int i = 0;

	if (!dl_list_empty(&wapp->sta_mntr_list)){
		/*sta List Parse*/
		dl_list_for_each_safe(sta_stat, sta_stat_tmp, &wapp->sta_mntr_list, struct sta_mnt_stat, list) {
		DBGPRINT(RT_DEBUG_TRACE, "%s Packet count %d\n", __func__, sta_stat->mnt_cnt);
		if (!dl_list_empty(&wapp->air_monitor_query_list)) {
			dl_list_for_each_safe(metrics_rsp, metrics_rsp_tmp, &wapp->air_monitor_query_list,
								struct air_monitor_query_rsp, list) {
			for (i = 0; i < metrics_rsp->unlink_metric.sta_num; i++) {
				unlink_rsp = &metrics_rsp->unlink_metric.info[i];
				if (!os_memcmp(sta_stat->sta_mac, unlink_rsp->mac, MAC_ADDR_LEN)) {
					if (sta_stat->mnt_cnt == MONITOR_PACKET_COUNT || (wapp->air_mnt_max_pkt
					&& sta_stat->mnt_cnt == wapp->air_mnt_max_pkt)) {
					metrics_rsp->bitmap |= BIT(i);
					DBGPRINT(RT_DEBUG_ERROR, "%s Monitor Result For Ind %d reference %d\n",
						__func__, i, sta_stat->mnt_reference);
					unlink_rsp->ch = sta_stat->Channel;
					if (wapp->air_mnt_max_pkt == 0)
						unlink_rsp->uplink_rssi = (signed char)((sta_stat->avg_rssi)/
											(signed char)(sta_stat->mnt_cnt));
					else
						unlink_rsp->uplink_rssi = (signed char)sta_stat->avg_rssi;
					/* air mnt req complete remove referance*/
					sta_stat->mnt_reference--;
					}
				}
			}
			}
		}
		}
	}

	/*Send Response if Monitor Done*/
	if (!dl_list_empty(&wapp->air_monitor_query_list)){
		dl_list_for_each_safe(metrics_rsp, metrics_rsp_tmp, &wapp->air_monitor_query_list, struct air_monitor_query_rsp, list) {
			if(check_for_monitor_complettion(metrics_rsp)) {
				/*send response from here only*/
				eloop_cancel_timeout(air_monitor_query_timeout, wapp, metrics_rsp);
				send_air_monitor_response(wapp, metrics_rsp);
				clear_air_monitor_req_list(wapp);
			}
		}
	}

	/*Clear NAC Request List*/
	/*clear_air_monitor_req_list(wapp);*/
}

int map_receive_air_monitor_request(
	struct wifi_app *wapp, unsigned char *bss_addr, unsigned char *sta_addr, char *msg_buf)
{
	unsigned char ret;
	struct unlink_metrics_query *query;
	struct unlink_rsp_sta *uplink_rsp;
	struct air_monitor_query_rsp *metrics_rsp= NULL;

	int i, j;

	query = (struct unlink_metrics_query *)msg_buf;

	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);

	DBGPRINT(RT_DEBUG_TRACE, "%s  bss_addr "MACSTR"\n"
			, __func__, MAC2STR(bss_addr));
	metrics_rsp = (struct air_monitor_query_rsp *)os_zalloc(sizeof(struct air_monitor_query_rsp) +
		(query->sta_num * sizeof(struct unlink_rsp_sta)));
	if(metrics_rsp == NULL) {
		DBGPRINT(RT_DEBUG_OFF, "%s Memory Alloc Fail\n", __func__);
		return MAP_ERROR;
	}

	/*Add it later once blocking of this query is removed*/
//	os_memcpy(metrics_rsp->almac,bss_addr,MAC_ADDR_LEN);
	metrics_rsp->unlink_metric.sta_num = 0;
	metrics_rsp->unlink_metric.oper_class = query->oper_class;

	for(i = 0 ; i < query->ch_num ; i ++) {

		for( j = 0 ; j < query->sta_num ; j++){
			uplink_rsp = &metrics_rsp->unlink_metric.info[metrics_rsp->unlink_metric.sta_num];
			uplink_rsp->ch = query->ch_list[i];
			os_memcpy(uplink_rsp->mac, &query->sta_list[j*MAC_ADDR_LEN], MAC_ADDR_LEN);
			/*Trigger Air Monitor if needed*/
			uplink_rsp->uplink_rssi = -110;
			DBGPRINT(RT_DEBUG_ERROR, "%s STA Entry Add"MACSTR"\n", __func__, MAC2STR(&query->sta_list[j*MAC_ADDR_LEN]));
			ret = air_monitor_entry_check(wapp, &query->sta_list[j* MAC_ADDR_LEN], query->ch_list[i]);

			if(ret) {
				DBGPRINT(RT_DEBUG_OFF, "%s Failed fill Default RSSI\n", __func__);
				uplink_rsp->uplink_rssi = ret;
				metrics_rsp->bitmap |= BIT(metrics_rsp->unlink_metric.sta_num);
			}

			metrics_rsp->unlink_metric.sta_num++;
		}
	}

	if(check_for_monitor_complettion(metrics_rsp)) {
		/*send response from here only*/
		send_air_monitor_response(wapp, metrics_rsp);
		os_free(metrics_rsp);
		return MAP_SUCCESS;
	}
	dl_list_add_tail(&wapp->air_monitor_query_list, &metrics_rsp->list);
	eloop_register_timeout(0, AIR_MONITOR_QUERY_TIMEOUT, air_monitor_query_timeout, wapp, metrics_rsp);
	return MAP_SUCCESS;
}

void air_monitor_packet_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	DBGPRINT(RT_DEBUG_ERROR, "%s\n", __func__);

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_OFF, "%s wdev not found if idx %d \n", __func__,ifindex);
		return;
	}
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		if(update_sta_rssi(wapp, wdev, &event_data->mnt_info) == 0) {
			send_air_monitor_response_check(wapp, &event_data->mnt_info);
			clear_monitor_list(wapp);
		} else {
			DBGPRINT(RT_DEBUG_TRACE, "%s  Drop Packet\n", __func__);
		}
	}
}

#ifdef MAP_R3
void wapp_dpp_check_conn_wrapper(void *eloop_ctx,
						   void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct wapp_dev *wdev = NULL;
	int ssid_cmp_len = 0;
	struct backhaul_connect_request *bh = timeout_ctx;

	if(bh == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null bh\n", __func__);
		return;
	}

	if (wapp->map->ch_change_done == 0) {
		eloop_register_timeout(1, 0, wapp_dpp_check_conn_wrapper, wapp, bh);
		return;
	}
	wapp->map->ch_change_done = 0;
	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bh->backhaul_mac, WAPP_DEV_TYPE_STA);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		goto fail;
	}

	if (!wdev->config) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: Config not found in wdev\n", __func__);
		goto fail;
	}
	if (!wdev->config->ssid) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: ssid  not found in config in wdev\n", __func__);
		goto fail;
	}

	ssid_cmp_len = (wdev->config->ssid_len >
		os_strlen((const char *)bh->target_ssid)) ?
		wdev->config->ssid_len :
		os_strlen((const char *)bh->target_ssid);
	if (os_memcmp(wdev->config->ssid, bh->target_ssid, ssid_cmp_len) != 0) {
		DBGPRINT(RT_DEBUG_ERROR, "%s wdev SSID and bh SSID are not same\n", __func__);
		/* Modify wdev SSID if not same with bh SSID */
		os_free(wdev->config->ssid);
		wdev->config->ssid = NULL;
		wdev->config->ssid_len = os_strlen((const char *)bh->target_ssid);
		wdev->config->ssid = os_zalloc(wdev->config->ssid_len + 1);
		if (!wdev->config->ssid) {
			DBGPRINT(RT_DEBUG_ERROR, "%s SSID malloc failed\n", __func__);
			goto fail;
		}
		os_memcpy(wdev->config->ssid, bh->target_ssid, wdev->config->ssid_len);
		wdev->config->ssid[wdev->config->ssid_len] = '\0';
		DBGPRINT(RT_DEBUG_WARN, "Modified wdev SSID:%s\n", wdev->config->ssid);
		/* Modify ssid in dpp cfg for reboot case */
		dpp_delete_parameter_from_file(wapp, "_ssid");
		dpp_save_config(wapp, "_ssid", (const char *)wdev->config->ssid, NULL);
	}

	DBGPRINT(RT_DEBUG_ERROR, "%s\n", __func__);
	if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		if (wapp_dpp_check_connect(wapp, wdev, (char *)bh->target_bssid, bh->channel) <= 0) {
			goto fail;
		}
	}
fail:
	if(bh) {
		os_free(bh);
		bh = NULL;
	}
}
void wapp_dpp_auth_rx_wrapper(void *eloop_ctx,
						   void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct wapp_dev *wdev = NULL;
	struct dpp_authentication *auth = timeout_ctx;
	if (!auth)
		return;
	wdev = auth->wdev;
	if (!wdev)
		return;
	if (wapp->map->ch_change_done == 0) {
		eloop_register_timeout(1, 0, wapp_dpp_auth_rx_wrapper, wapp, auth);
		return;
	}
	wapp->map->ch_change_done = 0;
	DBGPRINT(RT_DEBUG_ERROR, "%s\n", __func__);
	DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX "DPP: Authentication Request received");
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR
			" chan=%u type=%d", MAC2STR(auth->peer_mac_addr), auth->curr_chan,
			DPP_PA_AUTHENTICATION_RESP);
	auth->current_state = DPP_STATE_AUTH_CONF_WAITING;
	wapp_drv_send_action(wapp, wdev, auth->curr_chan, 0,
			auth->peer_mac_addr, wpabuf_head(auth->resp_msg),
			wpabuf_len(auth->resp_msg));
	if(!wapp->dpp->dpp_qr_mutual)
	{
		eloop_cancel_timeout(wapp_dpp_auth_conf_wait_timeout, wapp, auth);
		eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_auth_conf_wait_timeout, wapp, auth);
	}
}

#endif /* MAP_R3 */

#endif

int map_receive_nac_query_msg(
	struct wifi_app *wapp, unsigned char *bss_addr, unsigned char *sta_addr, char *msg_buf)
{
	struct wapp_dev *wdev = NULL;
	BOOLEAN mnt_en;
	unsigned char mnt_rule;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bss_addr, WAPP_DEV_TYPE_AP);
	if (wdev) {
		mnt_en = msg_buf[0];
		mnt_rule = msg_buf[1];
#if NL80211_SUPPORT
		map_set_air_mnt_cmd(wapp, wdev, sta_addr, mnt_en, mnt_rule);
#else
		map_set_air_mnt_cmd(wapp, wdev, sta_addr, mnt_en, mnt_rule);
#endif /* NL80211_SUPPORT */
	} else
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);

	return MAP_SUCCESS;
}

int map_receive_rssi_query_msg(
	struct wifi_app *wapp, unsigned char *bss_addr, unsigned char *sta_addr)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_sta *sta = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bss_addr, WAPP_DEV_TYPE_AP);
	if (wdev) {
		sta = wdev_ap_client_list_lookup_for_all_bss(wapp, sta_addr);
		if (sta)
			wapp_query_sta_rssi(wapp, wdev, sta_addr);
		else
			DBGPRINT(RT_DEBUG_ERROR, "%s no such sta\n", __func__);
	} else
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);

	return MAP_SUCCESS;
}

void map_get_scan_result(void *eloop_ctx, void *timeout_ctx)
{
	struct wifi_app *wapp = (struct wifi_app *)eloop_ctx;
	struct wapp_dev *wdev = (struct wapp_dev *)timeout_ctx;

	if(wapp->scan_wdev) {
		eloop_cancel_timeout(map_get_scan_result, wapp, wapp->scan_wdev);
		wapp->scan_wdev = NULL;
	}

	if (wdev->scan_cookie)
		wapp_query_scan_result(wapp, wdev, 0);
}

int map_receive_scan_request(
	struct wifi_app *wapp, unsigned char *bss_addr, char *msg_buf)
{
	struct wapp_dev *wdev = NULL;
	int sec = 10;
	//struct wapp_radio *radio;
	struct scan_BH_ssids *scan_ssids=NULL;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bss_addr, WAPP_DEV_TYPE_STA);
	if (wdev) {
		eloop_cancel_timeout(bh_steering_ready_timeout, wapp, wdev);
		scan_ssids=(struct scan_BH_ssids *)msg_buf;
		wdev->scan_cookie =scan_ssids->scan_cookie;
	//	printf("scan_ssids->scan_cookie %lu\n", scan_ssids->scan_cookie);
	//	printf("WAPP rx scan request cnt %d, ssid 0 %s ssid len %d, ssid 1 %s, ssid len %d\n", scan_ssids->profile_cnt , scan_ssids->scan_SSID_val[0].ssid,scan_ssids->scan_SSID_val[0].SsidLen, scan_ssids->scan_SSID_val[1].ssid, scan_ssids->scan_SSID_val[1].SsidLen);
		//Send OID sonal here
		wapp_set_scan_BH_ssids(wapp, wdev, scan_ssids);

		if (wapp->map->off_ch_scan_state.ch_scan_state != CH_SCAN_IDLE)
			off_ch_scan_timeout((void *) wapp, NULL);
		//radio = wdev->radio;
		wapp_issue_scan_request(wapp, wdev);
		//if (IS_MAP_CH_24G(radio->op_ch))
		//	sec = 10;

		wapp->scan_wdev = wdev;
		eloop_register_timeout(sec, 0, map_get_scan_result, wapp, wdev);
	} else
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);

	return MAP_SUCCESS;
}

int map_receive_null_frame_req(struct wifi_app *wapp, unsigned char *bss_addr, unsigned char *sta_addr, char *msg_buf)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_sta *sta = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	UCHAR count = *msg_buf;

	if (count == 0) {
		DBGPRINT(RT_DEBUG_ERROR, "%s count is %d\n", __func__, count);
		count++;
	}

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bss_addr, WAPP_DEV_TYPE_AP);
	if (wdev) {
		sta = wdev_ap_client_list_lookup_for_all_bss(wapp, sta_addr);
		if (sta)
			wapp_send_null_frames(wapp, wdev, sta_addr, count);
		else
			DBGPRINT(RT_DEBUG_ERROR, "%s  peer_mac_addr "MACSTR"\n"
			, __FUNCTION__, MAC2STR(sta_addr));
	} else
		DBGPRINT(RT_DEBUG_ERROR, "%s:wdev not found\n", __func__);

	return MAP_SUCCESS;
}

int map_set_enrollee_bh(struct wifi_app *wapp, unsigned char *bss_addr, unsigned char *sta_addr, char *msg_buf,
	char *evt_buf, int* len_buf)
{
	struct map_info *map = wapp->map;
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list;
	u8 dev_found = 0;
	u8 eth_ifname[IFNAMSIZ] = {0};
	struct enrollee_bh *en_type = (struct enrollee_bh*)msg_buf;


	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);


	printf("\033[1;36m %s \033[0m\n", __FUNCTION__);

	if (en_type->if_type == MAP_BH_ETH) {
		map->bh_type = MAP_BH_ETH;
		map_bh_ready(wapp, MAP_BH_ETH, eth_ifname, en_type->mac_address, en_type->mac_address);
	} else if (en_type->if_type == MAP_BH_WIFI) {
		printf("\033[1;36m %s if mac = "MACSTR"\033[0m\n", __FUNCTION__, MAC2STR(en_type->mac_address));
		dev_list = &wapp->dev_list;
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
			if (os_memcmp(wdev->mac_addr, en_type->mac_address, MAC_ADDR_LEN) == 0) {
					dev_found = 1;
					break;
			}
		}

		map->bh_type = MAP_BH_WIFI;
		if (dev_found == 1) {
			map->bh_wifi_dev = wdev;
			printf("bh_wifi_dev  %s\n", map->bh_wifi_dev->ifname);
		} else {
			printf(RED("%s: dev not found use default %s\n"),
					__FUNCTION__, map->bh_wifi_dev->ifname);

			os_memcpy(evt_buf, "DEV_NOT_FOUND", os_strlen("DEV_NOT_FOUND"));
			*len_buf = os_strlen("DEV_NOT_FOUND");
		}
	} else {
		printf("\033[1;36m %s: unknown bh type %d\033[0m\n", __FUNCTION__, en_type->if_type);
		os_memcpy(evt_buf, "UNKNOWN_TYPE", os_strlen("UNKNOWN_TYPE"));
		*len_buf = os_strlen("UNKNOWN_TYPE");
	}
	os_memcpy(evt_buf, "OK", os_strlen("OK"));
	*len_buf = os_strlen("OK");

	return WAPP_SUCCESS;
}

int map_set_bss_role(struct wifi_app *wapp, unsigned char *bss_addr, unsigned char *sta_addr, char *msg_buf,
	char *evt_buf, int *len_buf)
{
	struct wapp_dev *wdev = NULL;
	struct bss_role *role = (struct bss_role*)msg_buf;


	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, role->bssid, WAPP_DEV_TYPE_AP);
	if (wdev == NULL) {
		printf(RED("%s: dev not found\n"), __FUNCTION__);
		os_memcpy(evt_buf, "DEV_NOT_FOUND", os_strlen("DEV_NOT_FOUND"));
		*len_buf = os_strlen("DEV_NOT_FOUND");
		return WAPP_LOOKUP_ENTRY_NOT_FOUND;
	}

	wdev_set_bss_role(wdev, role->role);

	os_memcpy(evt_buf, "OK", os_strlen("OK"));
	*len_buf = os_strlen("OK");
	return WAPP_SUCCESS;
}

int map_trigger_wps(struct wifi_app *wapp, unsigned char *bss_addr, unsigned char *sta_addr, char *msg_buf,
	char *evt_buf, int *len_buf)
{
	char cmd_bk[1024];
	int wps_mode = 0;
	int ret;
	struct wapp_dev *target_wdev = NULL;
	struct trigger_wps_param *wps = (struct trigger_wps_param*)msg_buf;
	u8 role = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (wps->mode == 2) {
		wps_mode = 2;
	} else {
		DBGPRINT_RAW(RT_DEBUG_ERROR, "%s UNKNOWN WPS MODE\n", __FUNCTION__);
		os_memcpy(evt_buf, "INVALID_ARG", os_strlen("INVALID_ARG"));
		*len_buf = os_strlen("INVALID_ARG");
		return WAPP_INVALID_ARG;
	}

	target_wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, wps->if_mac, WAPP_DEV_TYPE_AP);
	if (target_wdev == NULL) {
		target_wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, wps->if_mac, WAPP_DEV_TYPE_STA);
		if (target_wdev == NULL) {
			printf(RED("%s: dev not found\n"), __FUNCTION__);
			os_memcpy(evt_buf, "DEV_NOT_FOUND", os_strlen("DEV_NOT_FOUND"));
			*len_buf = os_strlen("DEV_NOT_FOUND");
			return WAPP_LOOKUP_ENTRY_NOT_FOUND;
		}
		else
			/*means set as enrollee*/
			role = 1;
	} else
		/*means set as registrar*/
		role = 0;

#if NL80211_SUPPORT
	/*enrollee*/
	if (role == 1) {
		u8 enable = 1;
		/* Add function for OID call for ApCliEnable = 1 */
		wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
				(char *)&enable, 1);
		wapp_set_wsc_conf_mode(wapp, (const char *)target_wdev->ifname,
						(char *)&enable,1);
		wapp_set_wsc_mode(wapp, (const char *)target_wdev->ifname,
						(char *)&wps_mode,
						(size_t)sizeof(wps_mode));
		wapp_set_wsc_get_conf(wapp, (const char *)target_wdev->ifname,
						(char *)&enable,1);
	/*registrar*/
	} else if (role == 0) {
		u8 WscConfMode = 4;
		u8 WscConfStatus = 2;
		u8 WscGetConf = 1;
		wapp_set_wsc_conf_mode(wapp, (const char *)target_wdev->ifname,
						(char *)&WscConfMode,
						(size_t)sizeof(WscConfMode));
		wapp_set_wsc_mode(wapp, (const char *)target_wdev->ifname,
						(char *)&wps_mode,
						(size_t)sizeof(wps_mode));
		wapp_set_wsc_conf_status(wapp, (const char *)target_wdev->ifname,
						(char *)&WscConfStatus,1);
		wapp_set_wsc_get_conf(wapp, (const char *)target_wdev->ifname,
						(char *)&WscGetConf,1);
	}
#else
	char cmd[1024];
	memset(cmd,0,sizeof(cmd));
	/*enrollee*/
	if (role == 1) {
		ret = os_snprintf(cmd, sizeof(cmd),
			"iwpriv %s set ApCliEnable=1;iwpriv %s set WscConfMode=1;iwpriv %s set WscMode=%d;iwpriv %s set WscGetConf=1",
			target_wdev->ifname,target_wdev->ifname,target_wdev->ifname,wps_mode,target_wdev->ifname);
		if (os_snprintf_error(sizeof(cmd), ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	/*registrar*/
	} else if (role == 0) {
		ret = os_snprintf(cmd, sizeof(cmd),
			"iwpriv %s set WscConfMode=4;iwpriv %s set WscMode=%d;iwpriv %s set WscConfStatus=2;iwpriv %s set WscGetConf=1",
			target_wdev->ifname,target_wdev->ifname,wps_mode,target_wdev->ifname,target_wdev->ifname);
		if (os_snprintf_error(sizeof(cmd), ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	}
	DBGPRINT_RAW(RT_DEBUG_OFF, "\033[1;36m cmd [%s] \033[0m\n", cmd);
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif /* NL80211_SUPPORT */

	memset(cmd_bk,0,sizeof(cmd_bk));
	ret = os_snprintf(cmd_bk, sizeof(cmd_bk), "echo \"%s\" > /tmp/wps_dbg", cmd);
	if (os_snprintf_error(sizeof(cmd), ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	if (system(cmd_bk) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);

	os_memcpy(evt_buf, "OK", os_strlen("OK"));
	*len_buf = os_strlen("OK");
	return WAPP_SUCCESS;
}

int map_receive_flush_acl_msg(
	struct wifi_app *wapp, unsigned char *bss_addr)
{
#ifndef ACL_CTRL
	struct wapp_dev *wdev = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bss_addr, WAPP_DEV_TYPE_AP);
	if (wdev)
		map_acl_system_cmd(wapp, wdev, NULL, ACL_FLUSH);
	else
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
#endif /*ACL_CTRL*/
	return MAP_SUCCESS;
}

int map_receive_deauth_sta_msg(
	struct wifi_app *wapp, unsigned char *sta_addr, unsigned char *bssid)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_sta *cli = NULL;
	struct dl_list *dev_list;
	struct ap_dev *ap = NULL;
	u8 ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	if (!wapp || !sta_addr || !bssid)
		return MAP_ERROR;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	dev_list = &wapp->dev_list;
	if (!dl_list_empty(dev_list)){
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
			if (wdev->dev_type == WAPP_DEV_TYPE_AP)
				ap = (struct ap_dev *) wdev->p_dev;
			else
				continue;
			if ((os_memcmp(bssid, ZERO_MAC_ADDR, MAC_ADDR_LEN)) &&
				(os_memcmp(wdev->mac_addr, bssid, MAC_ADDR_LEN)))
				continue;
			cli = wdev_ap_client_list_lookup(wapp, ap, sta_addr);
			if (cli)
				map_trigger_deauth(wapp, wdev->ifname, sta_addr);
		}
	}
	return MAP_SUCCESS;
}

int map_receive_disconnect_apcli_msg(
	struct wifi_app *wapp, struct msg_1905 *map_msg, int len_recv)
{
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list;
	char *ifname = NULL;
	int ret;
#if NL80211_SUPPORT
	u8 Enable = 0;
#else
	char cmd[MAX_CMD_MSG_LEN] = {0};
#endif /* NL80211_SUPPORT */

	if (!wapp )
		return MAP_ERROR;
	DBGPRINT(RT_DEBUG_OFF, "%s recv size %d struct size %zd\n", __func__,len_recv, sizeof(struct msg_1905));
	if(len_recv > sizeof(struct msg_1905)) {
		ifname = map_msg->body;
		DBGPRINT(RT_DEBUG_OFF, "%s Disconnect %s\n", __func__, ifname);
#if NL80211_SUPPORT
		wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
				(char *)&Enable, 1);
#else
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
		ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ApCliEnable=0;", ifname);
		if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		if (system(cmd) == -1)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif
		return MAP_SUCCESS;
	}
	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
		DBGPRINT(RT_DEBUG_OFF, "wdev type %d\n", wdev->dev_type);

		if (wdev->dev_type == WAPP_DEV_TYPE_STA) {

			DBGPRINT(RT_DEBUG_ERROR, "ifname %s\n", wdev->ifname);
#if NL80211_SUPPORT
		wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
				(char *)&Enable, 1);
#else
			os_memset(cmd, 0, MAX_CMD_MSG_LEN);
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ApCliEnable=0;", wdev->ifname);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif
		}
		else
			continue;
	}
	return MAP_SUCCESS;
}
int wapp_set_bh_wsc_profile(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	struct wireless_setting *bh_wsc_profile);

int set_bh_wsc_profile(struct wifi_app *wapp,
	struct wireless_setting *config)
{
	struct wapp_dev *wdev;
	struct dl_list *dev_list;

	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);
	dev_list = &wapp->dev_list;
	if (!dl_list_empty(dev_list)) {
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
			if (wdev->dev_type == WAPP_DEV_TYPE_AP)
				wapp_set_bh_wsc_profile(wapp, wdev, config);
		}
	}
	return 0;
}

int map_config_bh_wireless_setting_msg(
	struct wifi_app *wapp, char *msg_buf)
{
	struct wsc_config *pountconf = NULL;
	struct wireless_setting* psetting = NULL;

	pountconf = (struct wsc_config *)msg_buf;
	psetting = &pountconf->setting[0];
	set_bh_wsc_profile(wapp, psetting);
	return 0;
}

int map_config_bh_priority
	(struct wifi_app *wapp, char *msg_buf, char *evt_buf, int* len_buf)
{
	struct wapp_dev *wdev = NULL;
	struct bh_priority *bh_priority_msg = NULL;

	bh_priority_msg = (struct bh_priority *)msg_buf;

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bh_priority_msg->bh_mac, WAPP_DEV_TYPE_STA);

	if (!wdev)
		return -1;
	wdev->bh_connect_priority = bh_priority_msg->priority;
	return 0;
}

int map_config_wireless_setting_msg(
	struct wifi_app *wapp, char *msg_buf,
	struct map_radio_identifier *ra_identifier, unsigned char Role)
{
	struct wapp_dev *wdev = NULL;
	struct wsc_config *pountconf = NULL;
	struct sec_info sec;
	struct wapp_radio *ra = NULL;
	struct wireless_setting* psetting = NULL;
	wsc_apcli_config_msg *apcli_config_msg = NULL;
	wsc_apcli_config *apcli_config = NULL;
	int i = 0, j = 0;
	struct dl_list *dev_list;
	struct ap_dev * ap = NULL;
	char value[10] = {0};
	int ret;
#ifdef HOSTAPD_MAP_SUPPORT
	struct wireless_setting curr_hapd_wifi_profile = {0};
	char cmd[MAX_CMD_MSG_LEN] = {0};
#endif /* HOSTAPD_MAP_SUPPORT */
	struct wireless_setting bh_config;
#ifdef MAP_R3
	struct dpp_authentication *auth = NULL;
	unsigned short connector_len=0;
	char * cred_ptr = NULL;
#endif /* MAP_R3 */


	int msg_size = sizeof(wsc_apcli_config_msg) +
		sizeof(wsc_apcli_config)*MAX_NUM_OF_RADIO;
	unsigned char need_write_bh_config = 0;
	unsigned int identical_count = 0, old_valid_bh_setting_cnt = 0;
	char ra_match[8] = {0};

	os_alloc_mem(NULL, (unsigned char **)&apcli_config_msg, msg_size);
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	memset(&sec, 0, sizeof(struct sec_info));
	pountconf = (struct wsc_config *)msg_buf;
#ifdef MAP_R3
	cred_ptr = msg_buf + 1;
#endif /* MAP_R3 */
	unsigned int bss_coex_buffer = 0;
	os_memset(apcli_config_msg, 0, msg_size);
	for (i = 0; i < pountconf->num; i++) {
#ifdef MAP_R3
		psetting = (struct wireless_setting *)cred_ptr;
#else
		psetting = &pountconf->setting[i];
#endif /* MAP_R3 */
		DBGPRINT(RT_DEBUG_TRACE, AUTO_CONFIG_PREX"wireless_setting%d intf_mac="MACSTR" ssid=%s, autmode=0x%04x, encryptype=0x%04x, key=%s bh_bss=%s, fh_bss=%s hidden_ssid=%d\n", i,
			MAC2STR(psetting->mac_addr),
			psetting->Ssid, psetting->AuthMode, psetting->EncrypType, psetting->WPAKey,
			psetting->map_vendor_extension & BIT_BH_BSS ? "1" : "0",
			psetting->map_vendor_extension & BIT_FH_BSS ? "1" : "0",
			psetting->hidden_ssid);
		if (psetting->map_vendor_extension & BIT_BH_BSS) {
#ifdef HOSTAPD_MAP_SUPPORT
			/*move reload flag down after bh config is set*/
			/*set_bh_wsc_profile(wapp, psetting);*/
#endif /*HOSTAPD_MAP_SUPPORT*/
			apcli_config = &apcli_config_msg->apcli_config[(apcli_config_msg->profile_count)++];
			apcli_config->AuthType = psetting->AuthMode;
#ifdef MAP_R3
		/* For comparison mapping of wts and driver*/
			if (apcli_config->AuthType & AUTH_DPP_ONLY) {
				apcli_config->AuthType &= ~AUTH_DPP_ONLY;
				apcli_config->AuthType |= WSC_AUTHTYPE_DPP;
			}
#endif
			apcli_config->EncrType = psetting->EncrypType;
			apcli_config->SsidLen = strlen((const char *)psetting->Ssid);
			os_memcpy(apcli_config->Key, psetting->WPAKey, strlen((const char *)psetting->WPAKey));
			os_memcpy(apcli_config->ssid, psetting->Ssid, apcli_config->SsidLen);
			apcli_config->KeyLength = strlen((const char *)psetting->WPAKey);
		}
#ifdef MAP_R3
		connector_len = (unsigned short)sizeof(*psetting) + psetting->cred_len + psetting->ext_cred_len;
		cred_ptr =  cred_ptr + connector_len;
#endif /* MAP_R3 */
	}
#ifdef MAP_R3
	cred_ptr = msg_buf + 1;
#endif /* MAP_R3 */

	/*get the radio identifer of the wireless setting*/
	for (i = 0; i < pountconf->num; i++) {
#ifdef MAP_R3
		psetting = (struct wireless_setting *)cred_ptr;
#else
		psetting = &pountconf->setting[i];
#endif /* MAP_R3 */
		wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, psetting->mac_addr, WAPP_DEV_TYPE_AP);
		if (wdev && wdev->radio) {
			ra = wdev->radio;
			break;
		}
#ifdef MAP_R3
		connector_len = (unsigned short)sizeof(*psetting) + psetting->cred_len + psetting->ext_cred_len;
		cred_ptr =  cred_ptr + connector_len;
#endif /* MAP_R3 */
	}
#ifdef MAP_R3
	cred_ptr = msg_buf + 1;
#endif /* MAP_R3 */

	if (i >= pountconf->num) {
		DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"unkown radio\n");
		os_free(apcli_config_msg);
		return MAP_ERROR;
	}

	wdev = NULL;
	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		/*check wdev with ap cap in the same radio of wireless setting*/
		if (wdev->dev_type == WAPP_DEV_TYPE_AP && wdev->radio == ra) {
			ap = (struct ap_dev *)wdev->p_dev;

#ifdef WIFI_MD_COEX_SUPPORT
			if (!ra->operatable) {
				DBGPRINT(RT_DEBUG_OFF,
					"wdev("MACSTR")cannot be configued for cur radio\n",
					MAC2STR(wdev->mac_addr));
				continue;
			}
#endif
			for (i = 0; i < pountconf->num; i++) {
#ifdef MAP_R3
				psetting = (struct wireless_setting *)cred_ptr;
#else
				psetting = &pountconf->setting[i];
#endif /* MAP_R3 */
				if (os_memcmp(wdev->mac_addr, psetting->mac_addr, MAC_ADDR_LEN) == 0) {
					break;
				}
#ifdef MAP_R3
				connector_len = (unsigned short)sizeof(*psetting) + psetting->cred_len + psetting->ext_cred_len;
				cred_ptr =  cred_ptr + connector_len;
#endif /* MAP_R3 */
			}

			/*this wdev exist in wireless setting*/
			if (i < pountconf->num) {
				memset(&sec, 0, sizeof(struct sec_info));
				if (MAP_SUCCESS != fill_sec_info(&sec, psetting)) {
					DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"fill_sec_info fail for "MACSTR" skip!!!\n", MAC2STR(psetting->mac_addr));
					continue;
				}

				/* Update this info in wapp locale structure, will be sent to to mapd */
				/*if the setting is the same, no need configure it again*/
				if ((!os_memcmp(ap->bss_info.ssid, psetting->Ssid, ap->bss_info.SsidLen) &&
						ap->bss_info.SsidLen == strlen((const char *)psetting->Ssid)) &&
					(ap->bss_info.auth_mode == psetting->AuthMode) &&
					(ap->bss_info.enc_type == psetting->EncrypType) &&
					(!os_memcmp(ap->bss_info.key, psetting->WPAKey, ap->bss_info.key_len) &&
						ap->bss_info.key_len == strlen((const char *)psetting->WPAKey)) &&
					(ap->bss_info.map_role == psetting->map_vendor_extension)) {
					DBGPRINT(RT_DEBUG_TRACE, AUTO_CONFIG_PREX"same config(ssid/auth_mode/enc_type/passphase/role) for %s\n", wdev->ifname);
					if (ap->bss_info.hidden_ssid != psetting->hidden_ssid) {
						wdev_set_hidden_ssid(wdev, psetting->hidden_ssid);
						DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"set hidden ssid(%d) on %s\n", psetting->hidden_ssid, wdev->ifname);
					}
					continue;
				}
				os_memset(ap->bss_info.ssid, 0, MAX_LEN_OF_SSID);
				os_memset(ap->bss_info.key, 0, 64);
				ap->bss_info.auth_mode = psetting->AuthMode;
				ap->bss_info.enc_type = psetting->EncrypType;
				ap->bss_info.key_len = strlen((const char *)psetting->WPAKey);
				os_memcpy(ap->bss_info.key, psetting->WPAKey, strlen((const char *)psetting->WPAKey));
				ap->bss_info.SsidLen = strlen((const char *)psetting->Ssid);
				os_memcpy(ap->bss_info.ssid, psetting->Ssid, ap->bss_info.SsidLen);
#ifdef HOSTAPD_MAP_SUPPORT
				wapp_get_hapd_wifi_profile(wapp, wdev, &curr_hapd_wifi_profile, 0, FALSE);

				if((psetting->AuthMode == curr_hapd_wifi_profile.AuthMode) &&
					(psetting->EncrypType == curr_hapd_wifi_profile.EncrypType) &&
					(psetting->map_vendor_extension ==  curr_hapd_wifi_profile.map_vendor_extension)&&
					!(os_memcmp(psetting->mac_addr, curr_hapd_wifi_profile.mac_addr, ETH_ALEN)) &&
					!(os_memcmp(psetting->Ssid, curr_hapd_wifi_profile.Ssid, os_strlen((char *)psetting->Ssid))) &&
					!(os_memcmp(psetting->WPAKey, curr_hapd_wifi_profile.WPAKey, os_strlen((char *)psetting->WPAKey)))
				)
				{
					DBGPRINT(RT_DEBUG_OFF, "hostapd conf same as new wifi profile\n");
				}
				else
				{
					DBGPRINT(RT_DEBUG_OFF, "write hostapd conf with new wifi profile\n");
					wapp_set_hapd_wifi_profile(wapp, wdev, psetting, 0, FALSE);
					wdev->i_need_hostapd_reload = TRUE;
				}
#endif /*HOSTAPD_MAP_SUPPORT */
				driver_wext_get_bss_coex(wapp->drv_data, wdev->ifname,
					(void *)&bss_coex_buffer);
				if (bss_coex_buffer) {
					wdev_set_bss_coex(wapp, wdev, FALSE);
				}
				wdev_set_bss_role(wdev, psetting->map_vendor_extension);
#ifdef MAP_R3
				if((wapp->map->map_version == DEV_TYPE_R3) && wapp->dpp) {
					if(!wapp->dpp->onboarding_type && (psetting->AuthMode & AUTH_DPP_ONLY)) {
						wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: configurationObject JSON",
								(const u8 *)psetting->Cred, psetting->cred_len);

						if(wdev_set_dpp_cred(wapp, wdev, psetting, 1) != MAP_SUCCESS) {
							DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s error wireless setting while applying dpp conf\n", __func__);
							continue;
						}
						/* If the BSS is supporting both FH and BH go for parsing another cred */
						if(wdev->i_am_fh_bss && wdev->i_am_bh_bss) {
							wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: 2nd configurationObject JSON",
									(const u8 *)(psetting->Cred + psetting->cred_len), psetting->ext_cred_len);

							if(wdev_set_dpp_cred(wapp, wdev, psetting, 0) != MAP_SUCCESS) {
								DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s error wireless setting while applying dpp conf\n", __func__);
								continue;
							}
						}
					} else{
						wapp_set_bss_start(wapp, wdev->ifname);
						wdev_set_sec_and_ssid(wapp, wdev, &sec, (char *)psetting->Ssid);
					}
				}
					else
#endif /* MAP_R3 */
					{
							wapp_set_bss_start(wapp, wdev->ifname);
							wdev_set_sec_and_ssid(wapp, wdev, &sec, (char *)psetting->Ssid);
					}


#ifdef MAP_R3
				wdev->is_configured = TRUE;
				wapp->conf_op_bnd = wapp_op_band_frm_ch(wapp, wdev->radio->op_ch);
#ifdef MAP_R3_RECONFIG
				wapp->conf_op_ch = wdev->radio->op_ch;
#endif
#endif /* MAP_R3 */
				wdev_set_hidden_ssid(wdev, psetting->hidden_ssid);
				if (ap->isActive == WAPP_BSS_STOP) {
					wapp_set_bss_start(wapp, wdev->ifname);
#ifdef HOSTAPD_MAP_SUPPORT
					memset(cmd,0,sizeof(cmd));
					os_snprintf(cmd,sizeof(cmd), "hostapd_cli -i%s enable", wdev->ifname);
				if (system(cmd) == -1)
					DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif
				}
				ap->isActive = WAPP_BSS_START;
				if (bss_coex_buffer) {
					wdev_set_bss_coex(wapp, wdev, bss_coex_buffer);
				}
			} else {
				/*this wdev need do bss stop*/
				if (ap->isActive == WAPP_BSS_START) {
					ap->isActive = WAPP_BSS_STOP;
					if (ap->bss_info.map_role & BIT_BH_BSS) {
						memset(&bh_config, 0, sizeof(bh_config));
						os_memcpy(bh_config.mac_addr, ap->bss_info.if_addr, MAC_ADDR_LEN);
						os_memcpy(bh_config.Ssid, ap->bss_info.ssid, ap->bss_info.SsidLen);
						bh_config.AuthMode = ap->bss_info.auth_mode;
						bh_config.EncrypType = ap->bss_info.enc_type;
						os_memcpy(bh_config.WPAKey, ap->bss_info.key, ap->bss_info.key_len);
						bh_config.map_vendor_extension = BIT_TEAR_DOWN;
						bh_config.hidden_ssid = ap->bss_info.hidden_ssid;
						DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"clear wsc setting for %s\n", wdev->ifname);
						set_bh_wsc_profile(wapp, &bh_config);
					}
					if (os_memcmp(ap->bss_info.ssid, "MAP-UNCONF", os_strlen("MAP-UNCONF")))
						wdev_set_ssid(wapp, wdev, "MAP-UNCONF");
					else
						DBGPRINT(RT_DEBUG_OFF, "No need to update %s\n",wdev->ifname);
					DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"bss stop for %s\n", wdev->ifname);
					wapp_set_bss_stop(wapp, wdev->ifname);
#ifdef HOSTAPD_MAP_SUPPORT
					{
						struct wireless_setting unConf_wifi_profile = {0};

						os_memcpy(unConf_wifi_profile.Ssid, "MAP-UNCONF", os_strlen("MAP-UNCONF"));
						wapp_set_hapd_wifi_profile(wapp, wdev, &unConf_wifi_profile, 0, FALSE);
						/* Bringdown hostapd for this interface*/
						memset(cmd,0,sizeof(cmd));
						os_snprintf(cmd,sizeof(cmd), "hostapd_cli -i%s disable", wdev->ifname);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
					}
#endif
				}
			}
		}
	}
#ifdef MAP_R3
	cred_ptr = msg_buf + 1;
#endif /* MAP_R3 */
	wapp->map->radio_conf_count++;

#ifdef MAP_R3
	if (wapp->map->map_version == DEV_TYPE_R3) {
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"radio configured count:%u\n",wapp->map->radio_conf_count);
		if(wapp->map->radio_conf_count >= wapp->radio_count) {
			wapp->map->radio_conf_count=0;
			if(wapp->dpp) {
				if(wapp->dpp->onboarding_type == 0) {
					DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"done with auth instance deinit now\n");
					if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_CONFIGURATOR)
						auth = wapp_dpp_get_own_auth_cont(wapp);
					else
						auth = wapp_dpp_get_first_auth(wapp);

					if (!auth) {
						DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s auth instance not found\n", __func__);
					} else {
						wapp_dpp_cancel_timeouts(wapp, auth);
						dpp_auth_deinit(auth);
					}
					if(wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE) {
						if(wapp->dpp->map_sec_done) {
							/* Changing role to proxy agent when agent onboarded
							 * on bootup without DPP handshakes */
							DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"Changing the role to proxy agnt\n");
							wapp->dpp->dpp_allowed_roles = DPP_CAPAB_PROXYAGENT;
						}else {
							if(wapp->map->bh_type != MAP_BH_ETH) {
								/* If the R3 wifi device onboarded through R1/R2 and
								 * onbording method is DPP then start chirping */
								wapp->dpp->wsc_onboard_done = 1;
								DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX "R3 device onboarded through R1/R2 so start chirping..\n");
								wapp->dpp->annouce_enrolle.is_enable = 1;
								wapp_dpp_presence_annouce(wapp, NULL);
							}
						}
					}
				}
			}
		}
		else {
			DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX "radio configured count not reached max\n");
		}
	}
#endif /* MAP_R3 */

#ifdef HOSTAPD_MAP_SUPPORT
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		/*check wdev with ap cap in the same radio of wireless setting*/
		if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP &&
			wdev->radio == ra && wdev->i_need_hostapd_reload == TRUE) {
			memset(cmd,0,sizeof(cmd));
			os_snprintf(cmd,sizeof(cmd), "hostapd_cli -i%s config_reload",wdev->ifname);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			wdev->i_need_hostapd_reload = FALSE;
		}
	}
#endif /* HOSTAPD_MAP_SUPPORT*/
	if (IS_CONF_STATE((&ra->conf_state), MAP_CONF_WAIT_RSP) ||
		IS_CONF_STATE((&ra->conf_state), MAP_CONF_UNCONF) ||
		IS_CONF_STATE((&ra->conf_state), MAP_CONF_STOP)) {
		MAP_CONF_STATE_SET((&ra->conf_state), MAP_CONF_CONFED);
		DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"radio id=%04x cardid=%04x set MAP_CONF_CONFED\n", ra->radio_id, ra->card_id);
	}
	/*Set BH profile on driver after clearing it while resetting bss role*/
	for (i = 0; i < pountconf->num; i++) {
#ifdef MAP_R3
		psetting = (struct wireless_setting *)cred_ptr;
#else
		psetting = &pountconf->setting[i];
#endif /* MAP_R3 */
		if (psetting->map_vendor_extension & BIT_BH_BSS) {
			set_bh_wsc_profile(wapp, psetting);
		}
#ifdef MAP_R3
		connector_len = (unsigned short)sizeof(*psetting) + psetting->cred_len + psetting->ext_cred_len;
		cred_ptr =  cred_ptr + connector_len;
#endif /* MAP_R3 */
	}

#ifdef HOSTAPD_MAP_SUPPORT
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
			/*check wdev with ap cap in the same radio of wireless setting*/
			if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP &&
				wdev->radio == ra && wdev->i_need_hostapd_reload == TRUE) {
				DBGPRINT(RT_DEBUG_OFF, "%s config reload %s\n", __func__, wdev->ifname);
				memset(cmd,0,sizeof(cmd));
				os_snprintf(cmd,sizeof(cmd), "hostapd_cli -i%s config_reload",wdev->ifname);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
				wdev->i_need_hostapd_reload = FALSE;
			}
		}
#endif /* HOSTAPD_MAP_SUPPORT*/

	MAP_GET_RADIO_IDNFER(ra, ra_identifier);
	/*compare setting to avoid write mapd_user cfg repeatedly*/
	ret = os_snprintf(ra_match, sizeof(ra_match), "%02x:%02x", ra_identifier->card_id, ra_identifier->ra_id);
	if (os_snprintf_error(sizeof(ra_match), ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		if (wapp->map->apcli_configs[i].config_valid &&
			!os_strcmp((char *)wapp->map->apcli_configs[i].raid, ra_match))
			old_valid_bh_setting_cnt++;
	}
	if (old_valid_bh_setting_cnt != apcli_config_msg->profile_count) {
		need_write_bh_config = 1;
		DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"old_valid_bh_setting_cnt(%d) != new_valid_bh_setting_cnt(%d) need rewrite bh config\n",
			old_valid_bh_setting_cnt, apcli_config_msg->profile_count);
	} else {
		for (i = 0; i < apcli_config_msg->profile_count; i++) {
			for (j = 0; j < MAX_NUM_OF_RADIO; j++) {
				if(os_strcmp((char *)wapp->map->apcli_configs[j].raid, ra_match))
					continue;
				if (wapp->map->apcli_configs[j].config_valid &&
					!os_strcmp((char *)wapp->map->apcli_configs[j].apcli_config.ssid,
						(char *)apcli_config_msg->apcli_config[i].ssid) &&
					!os_strcmp((char *)wapp->map->apcli_configs[j].apcli_config.Key,
						(char *)apcli_config_msg->apcli_config[i].Key) &&
					(wapp->map->apcli_configs[j].apcli_config.AuthType ==
						apcli_config_msg->apcli_config[i].AuthType) &&
					(wapp->map->apcli_configs[j].apcli_config.EncrType ==
						apcli_config_msg->apcli_config[i].EncrType)
				)
					identical_count++;
			}
		}
		if (identical_count != apcli_config_msg->profile_count) {
			need_write_bh_config = 1;
			DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"backhaul setting changed!!! need rewrite bh config\n");
		}
	}

	/*write config logic*/
	get_map_parameters(wapp->map, "role_detection_external", value, NON_DRIVER_PARAM, sizeof(value));
	if (!strcmp(value,"1"))
		write_backhaul_configs_all(wapp, apcli_config_msg, ra_identifier);
	else if(Role!=DEVICE_ROLE_CONTROLLER && need_write_bh_config)
		write_backhaul_configs_all(wapp, apcli_config_msg, ra_identifier);
	os_free(apcli_config_msg);
	return MAP_SUCCESS;
}

int map_config_bssload_thrd_setting_msg(
	struct wifi_app *wapp, const char *iface, char *high_thrd, char *low_thrd)
{
	wapp_set_bssload_thrd(wapp, iface, high_thrd, low_thrd);
	return MAP_SUCCESS;
}

int map_config_local_steer_disallow_sta_msg(
	struct wifi_app *wapp, unsigned char *bss_addr, char *msg_buf)
{
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	struct wapp_sta *sta = NULL;
	unsigned char *sta_addr = NULL;
	struct local_disallow_sta_head *sta_info = NULL;
	int sta_cnt = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bss_addr, WAPP_DEV_TYPE_AP);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return MAP_ERROR;
	}

	ap = (struct ap_dev *)wdev->p_dev;

	sta_info = (struct local_disallow_sta_head*)msg_buf;
	sta_cnt = sta_info->sta_cnt;
	DBGPRINT(RT_DEBUG_OFF, "(%s) Local disallow sta_cnt = %d\n", __func__, sta_cnt);

	sta_addr = sta_info->sta_list;
	while (sta_cnt > 0) {
		sta = wdev_ap_client_list_lookup(wapp, ap, sta_addr);
		DBGPRINT(RT_DEBUG_INFO, "(%s) StaAddr("MACSTR")\n", __func__, MAC2STR(sta_addr));
		if (sta)
			sta->bLocalSteerDisallow = TRUE;
		else
			DBGPRINT(RT_DEBUG_WARN, "%s cant fined local steer disallow sta\n", __func__);

		sta_addr += MAC_ADDR_LEN;
		sta_cnt--;
	}

	return MAP_SUCCESS;
}

int map_config_btm_steer_disallow_sta_msg(
	struct wifi_app *wapp, unsigned char *bss_addr, char *msg_buf)
{
	struct wapp_sta *sta = NULL;
	unsigned char *sta_addr = NULL;
	struct local_disallow_sta_head *sta_info = NULL;
	int sta_cnt = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	sta_info = (struct local_disallow_sta_head*)msg_buf;
	sta_cnt = sta_info->sta_cnt;
	DBGPRINT(RT_DEBUG_OFF, "%s BTM disallow sta_cnt = %d\n", __func__, sta_cnt);

	sta_addr = sta_info->sta_list;
	while (sta_cnt > 0) {
		DBGPRINT(RT_DEBUG_INFO, "(%s) StaAddr("MACSTR")\n", __func__, MAC2STR(sta_addr));
		sta = wdev_ap_client_list_lookup_for_all_bss(wapp, sta_addr);

		if (sta) {
			DBGPRINT(RT_DEBUG_ERROR, GRN("%s found sta\n"), __func__);
			sta->bBTMSteerDisallow = TRUE;
		} else
			DBGPRINT(RT_DEBUG_WARN, RED("%s cant find btm steer disallow sta\n"), __func__);

		sta_addr += MAC_ADDR_LEN;
		sta_cnt--;
	}

	return MAP_SUCCESS;
}

int map_config_radio_control_policy_msg(
	struct wifi_app *wapp, unsigned char *addr, char *msg_buf)
{
	struct wapp_dev *wdev = NULL;
	struct radio_policy *ra_policy = NULL;
	struct radio_policy_head *policy_info = NULL;
	wdev_steer_policy policy;
	int i = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	ra_policy = (struct radio_policy*)msg_buf;

	DBGPRINT(RT_DEBUG_OFF, "%s  ra_policy->radio_cnt = %d\n", __func__, ra_policy->radio_cnt);
	for (i = 0; i < ra_policy->radio_cnt; i++) {
		policy_info = (struct radio_policy_head *) ra_policy->radio;

		wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)policy_info->identifier);
		if (!wdev) {
			DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
			return 0;
		}

		if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
			DBGPRINT(RT_DEBUG_OFF, "steer_policy = %d, cu_thr = %d, rssi_thr = %d\n",
									policy_info->policy, policy_info->ch_ultil_thres, policy_info->rssi_thres);
			os_memset(&policy, 0, sizeof(wdev_steer_policy));
			policy.steer_policy = policy_info->policy;
			policy.cu_thr= policy_info->ch_ultil_thres;
			policy.rcpi_thr= policy_info->rssi_thres;	//dBm
			wapp_set_steering_policy(wapp, wdev, &policy);
		}
	}

	return MAP_SUCCESS;
}

int wapp_send_backhaul_steering_rsp_msg(struct map_info *map, char *buf,
	int* len_buf, struct backhaul_steer_rsp *bh_evt)
{
	struct evt *wapp_event = NULL;
	int send_pkt_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_BACKHAUL_STEER_RSP;
	wapp_event->length = sizeof(struct backhaul_steer_rsp);

	memcpy(wapp_event->buffer, bh_evt, wapp_event->length);
	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}

int map_config_backhaul_steering_msg(
	struct wifi_app *wapp, char *msg_buf, char *evt_buf, int* len_buf)
{
	struct wapp_dev *wdev = NULL,*ap_wdev = NULL;
	struct map_info *map = NULL;
	struct backhaul_steer_request_extended *bh = NULL;
	struct backhaul_steer_rsp bh_rsp;
	unsigned char wdev_identifier[ETH_ALEN];
	unsigned char ap_wdev_identifier[ETH_ALEN];
	int res;
#if NL80211_SUPPORT
	u8 Enable = 0;
#else
	char cmd[MAX_CMD_MSG_LEN] = {0};
#endif /* NL80211_SUPPORT */
        int ret = 0;
	struct ap_dev *ap = NULL;
	unsigned char found = 0, bss_start_needed = 1;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	bh = (struct backhaul_steer_request_extended*)msg_buf;

	if (!bh) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null msg\n", __func__);
		return MAP_ERROR;
	}
	map = wapp->map;

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bh->request.backhaul_mac, WAPP_DEV_TYPE_STA);

	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return MAP_ERROR;
	}

	if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		os_memset(wdev_identifier,0,ETH_ALEN);
		os_memset(ap_wdev_identifier,0,ETH_ALEN);

		MAP_GET_RADIO_IDNFER(wdev->radio, wdev_identifier);
		struct dl_list *dev_list = &wapp->dev_list;
		if (!dl_list_empty(dev_list)) {
			dl_list_for_each(ap_wdev, &wapp->dev_list, struct wapp_dev, list){
				/*check if all BSS are down of this Band*/
				if(ap_wdev->dev_type == WAPP_DEV_TYPE_AP){
					ap = (struct ap_dev *) ap_wdev->p_dev;
					DBGPRINT(RT_DEBUG_ERROR, "1Intf %s Is Active %d \n", ap_wdev->ifname, ap->isActive);
					MAP_GET_RADIO_IDNFER(ap_wdev->radio, ap_wdev_identifier);
				}
				if (ap_wdev->radio && (os_memcmp(wdev_identifier,ap_wdev_identifier,ETH_ALEN) == 0) &&
						(ap_wdev->dev_type == WAPP_DEV_TYPE_AP) && ap->isActive == WAPP_BSS_START) {
					DBGPRINT(RT_DEBUG_ERROR, " 1BSS Start needed Intf %s Is Active %d \n", ap_wdev->ifname, ap->isActive);
					bss_start_needed = 0;
					break;
				}
			}
		}
		os_memset(ap_wdev_identifier,0,ETH_ALEN);
		if(bss_start_needed) {
			if (!dl_list_empty(dev_list)) {
				dl_list_for_each(ap_wdev, &wapp->dev_list, struct wapp_dev, list){
					DBGPRINT(RT_DEBUG_OFF, "wdev %d ap wdev %d \n", wdev->radio->radio_id, ap_wdev->radio->radio_id);
					MAP_GET_RADIO_IDNFER(ap_wdev->radio, ap_wdev_identifier);
					DBGPRINT(RT_DEBUG_OFF, "wdev =   "MACSTR" ap wdev =   "MACSTR" \n",
							MAC2STR(wdev_identifier), MAC2STR(ap_wdev_identifier));
					if (ap_wdev->radio && (ap_wdev->dev_type == WAPP_DEV_TYPE_AP) &&
							(os_memcmp(wdev_identifier,ap_wdev_identifier,ETH_ALEN) == 0)) {
						found = 1;
						break;
					}
				}
			}
		}
		/*work around for MAP Cert TC 4.9.1 Marvel Controller */
		/*Its observed that in this case all BSS are down ,Aplci is not able to connect in thsis case*/
		/*As a workaround we are starting one of the BSS of the Apcli band */
		/*stop the BSS in the same function once connction is completed*/
		if(found){
			DBGPRINT(RT_DEBUG_ERROR, "bh_steer_if: %s\n", ap_wdev->ifname);
			if (map->MapMode != 4)
				wapp_set_bss_start(wapp, ap_wdev->ifname);
		}
		eloop_cancel_timeout(bh_steering_ready_timeout, wapp, wdev);
		DBGPRINT(RT_DEBUG_ERROR, "bh_steer_if: %s\n", wdev->ifname);

#if NL80211_SUPPORT
		wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
				(char *)&Enable, 1);
		if (map->MapMode != 4) {
			if(map->quick_ch_change)
				wapp_set_map_channel(wapp, (const char *)wdev->ifname,
						(char *)&bh->request.channel,
						(size_t)sizeof(bh->request.channel));
			else
				wapp_set_channel(wapp, (const char *)wdev->ifname,
						(char *)&bh->request.channel,
						(size_t)sizeof(bh->request.channel));
		}
#else
		res = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ApCliEnable=0;", wdev->ifname);
		if (os_snprintf_error(MAX_CMD_MSG_LEN, res))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		DBGPRINT(RT_DEBUG_ERROR," Send Command: %s\n",cmd);
		if (system(cmd) == -1)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);

		if (map->MapMode != 4) {
			if (map->quick_ch_change) {
				res = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set MapChannel=%d;", wdev->ifname, bh->request.channel);
				if (os_snprintf_error(MAX_CMD_MSG_LEN, res))
					DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			} else {
				res = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set Channel=%d;", wdev->ifname, bh->request.channel);
				if (os_snprintf_error(MAX_CMD_MSG_LEN, res))
					DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			}
			DBGPRINT(RT_DEBUG_ERROR," Send Command: %s\n",cmd);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);

			os_memset(cmd,0,MAX_CMD_MSG_LEN);
			res = snprintf(cmd, MAX_CMD_MSG_LEN, "wifi_config_save %s Channel %d", wdev->ifname, bh->request.channel);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, res))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			ret = system(cmd);
			if (ret == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			DBGPRINT(RT_DEBUG_OFF," [%s] Send Channel  Command: %s, ret = %d\n", __func__, cmd, ret);
		}
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
#endif /* NL80211_SUPPORT */

		driver_wext_set_ssid(wapp->drv_data, wdev->ifname, (char *)bh->target_ssid);
		wapp->map->bh_link_ready = 0;
#if NL80211_SUPPORT
		wapp_set_bssid(wapp, (const char *)wdev->ifname,
				(char *)bh->request.target_bssid, MAC_ADDR_LEN);
		Enable = 1;
		wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
				(char *)&Enable, 1);
#else
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
		res = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ApCliBssid=%02x:%02x:%02x:%02x:%02x:%02x;", wdev->ifname, PRINT_MAC(bh->request.target_bssid));
		if (os_snprintf_error(MAX_CMD_MSG_LEN, res))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		DBGPRINT(RT_DEBUG_ERROR," Send Command: %s\n",cmd);
		if (system(cmd) == -1)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);

		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
		res = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ApCliEnable=1;", wdev->ifname);
		if (os_snprintf_error(MAX_CMD_MSG_LEN, res))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		DBGPRINT(RT_DEBUG_ERROR," Send Command: %s\n",cmd);
		if (system(cmd) == -1)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif /* NL80211_SUPPORT */
	}

	//fill backhaul steering rsp
	COPY_MAC_ADDR(bh_rsp.backhaul_mac, bh->request.backhaul_mac);
	COPY_MAC_ADDR(bh_rsp.target_bssid, bh->request.target_bssid);

	bh_rsp.status = BH_SUCCESS;
	eloop_register_timeout(1, 0, bh_steering_ready_timeout, wapp, wdev);

	wapp_send_backhaul_steering_rsp_msg(map, evt_buf, len_buf, &bh_rsp);
	DBGPRINT(RT_DEBUG_ERROR, "bh_link_ready: %d\n", wapp->map->bh_link_ready);
	DBGPRINT(RT_DEBUG_ERROR, "[%s]: backhaul_mac("MACSTR")\n", __func__, MAC2STR(bh->request.backhaul_mac));
	DBGPRINT(RT_DEBUG_ERROR, "[%s]: target_bssid("MACSTR")\n", __func__, MAC2STR(bh->request.target_bssid));
	if(found){
		wapp_set_bss_stop(wapp, wdev->ifname);
	}

	return MAP_SUCCESS;
}

#ifdef DFS_CAC_R2
int wapp_send_cac_req(struct wifi_app *wapp,
						 const char *iface,
						 u32 param,
						 size_t msg_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);

	ret = wapp->drv_ops->drv_cac_req(wapp->drv_data, iface, param, msg_len);

	return ret;
}
#endif

int map_config_backhaul_connect_msg(
	struct wifi_app *wapp, char *msg_buf, char *evt_buf, int* len_buf)
{
	struct wapp_dev *wdev = NULL;
#if defined(DFS_CAC_R2)  && defined(MAP_R3)
	struct wapp_dev *ap_wdev = NULL;
	struct wapp_radio *ra = NULL;
#endif
	struct backhaul_connect_request *bh = NULL;
	char cmd[MAX_CMD_MSG_LEN] = {0};
#if defined(DFS_CAC_R2) && defined(MAP_R3)
	unsigned char radio_id[MAC_ADDR_LEN] = {0};
	u8 band = 0;
#endif
	int ret;
	//u8 i = 0;
	u8 bw;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);

	if(!wapp){
		DBGPRINT(RT_DEBUG_ERROR, "%s null wapp\n", __func__);
		return MAP_ERROR;
	}

	bh = (struct backhaul_connect_request *)os_zalloc(sizeof(struct backhaul_connect_request));
	if (!bh) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null msg\n", __func__);
		return MAP_ERROR;
	}
	os_memcpy(bh, msg_buf, sizeof(struct backhaul_connect_request));

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bh->backhaul_mac, WAPP_DEV_TYPE_STA);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		if (bh)
			os_free(bh);
		return MAP_ERROR;
	}

	if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		eloop_cancel_timeout(bh_steering_ready_timeout, wapp, wdev);
		DBGPRINT(RT_DEBUG_OFF, "bh_steer_if: %s\n", wdev->ifname);
		/* bh link is disconnected, wait 15 sec max for scanning 5G band */
#if NL80211_SUPPORT
		wapp_set_authtype(wapp, (const char *)wdev->ifname,
				(char *)WscGetAuthTypeStr(bh->AuthType),
				(size_t)strlen(WscGetAuthTypeStr(bh->AuthType)));
		wapp_set_apcli_EncrypType(wapp, (const char *)wdev->ifname,
				(char*)WscGetEncryTypeStr(bh->EncrType),
				strlen(WscGetEncryTypeStr(bh->EncrType)));
#else
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
		ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ApCliAuthMode=%s;",
			wdev->ifname,
			WscGetAuthTypeStr(bh->AuthType));
		if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

		if (system(cmd) == -1)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
		DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);

		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
		ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ApCliEncrypType=%s;",
			wdev->ifname,
			WscGetEncryTypeStr(bh->EncrType));
		if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		if (system(cmd) == -1)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
		DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
#endif
/*To work WPA3 test with MAP_R1*/
#ifdef MAP_SUPPORT
		if (strcmp(WscGetAuthTypeStr(bh->AuthType), "WPA2PSKWPA3PSK") == 0 ||
				(strcmp(WscGetAuthTypeStr(bh->AuthType), "WPA3PSK") == 0)
#ifdef MAP_R3
				|| (bh->AuthType & WSC_AUTHTYPE_DPP)
#endif
		) {
#if NL80211_SUPPORT
			u8 Enable = 1;
			wapp_set_apcli_PMFMFPC(wapp, (const char *)wdev->ifname,
					(char*)&Enable, 1);
#else
			os_memset(cmd, 0, MAX_CMD_MSG_LEN);
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ApCliPMFMFPC=1;",
					wdev->ifname);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
#endif
		} else {
#if NL80211_SUPPORT
			u8 Enable = 0;

			wapp_set_apcli_PMFMFPC(wapp, (const char *)wdev->ifname,
					(char *)&Enable, 1);
#else
			/*MAPD config renew, we may move back to non PMF setup, like WPA.*/
			os_memset(cmd, 0, MAX_CMD_MSG_LEN);
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ApCliPMFMFPC=0;",
					wdev->ifname);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);
#endif
		}
#endif
#if 0
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
		snprintf(cmd,MAX_CMD_MSG_LEN,"iwpriv %s set ApCliEncrypType=%s;",
			wdev->ifname,
			WscGetEncryTypeStr(bh->EncrType));
		if (system(cmd) == -1)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
		DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
#endif
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
		driver_wext_set_psk(wapp->drv_data, wdev->ifname, (char *)bh->Key);

#if NL80211_SUPPORT
		wapp_set_bssid(wapp, (const char *)wdev->ifname,
				(char *)bh->target_bssid, MAC_ADDR_LEN);
#else
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
		ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ApCliBssid=%02x:%02x:%02x:%02x:%02x:%02x",
			wdev->ifname, PRINT_MAC(bh->target_bssid));
		if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		if (system(cmd) == -1)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
		DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
#endif
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
		driver_wext_set_ssid(wapp->drv_data, wdev->ifname, (char *)bh->target_ssid);

		if (WMODE_CAP_AX(wdev->wireless_mode)) {
			if (bh->bw == BW_160)
				bw = bw_160;
			else if (bh->bw == BW_80)
				bw = bw_80;
			else
				bw = bw_20_40;
#if NL80211_SUPPORT
			wapp_set_vhtbw(wapp, (const char *)wdev->ifname,
				(char *)&bw,
				(size_t)sizeof(bw));
#else
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set VhtBw=%d;", wdev->ifname, bw);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);

#endif /* NL80211_SUPPORT */
			if (bh->bw >= BW_40)
				bw = bw_40;
			else
				bw = bw_20;
#if NL80211_SUPPORT
			wapp_set_htbw(wapp, (const char *)wdev->ifname,
				(char *)&bw,
				(size_t)sizeof(bw));
#else
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set HtBw=%d;", wdev->ifname, bw);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);

#endif /* NL80211_SUPPORT */
		} else if (WMODE_CAP_AC(wdev->wireless_mode)) {
			if (bh->bw == BW_160)
				bw = bw_160;
			else if (bh->bw == BW_80)
				bw = bw_80;
			else
				bw = bw_20_40;
#if NL80211_SUPPORT
			wapp_set_vhtbw(wapp, (const char *)wdev->ifname,
				(char *)&bw,
				(size_t)sizeof(bw));
#else
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set VhtBw=%d;", wdev->ifname, bw);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);
#endif /* NL80211_SUPPORT */
		} else {
			if (bh->bw >= BW_40)
				bw = bw_40;
			else
				bw = bw_20;
#if NL80211_SUPPORT
			wapp_set_htbw(wapp, (const char *)wdev->ifname,
				(char *)&bw,
				(size_t)sizeof(bw));
#else
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set HtBw=%d;", wdev->ifname, bw);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);
#endif /* NL80211_SUPPORT */
		}
#ifdef MAP_R3
		if (wapp && wapp->map && wapp->map->TurnKeyEnable == 1
			&& wapp->map->map_version == DEV_TYPE_R3 && (bh->AuthType & WSC_AUTHTYPE_DPP)) {
			//dpp_read_config_file_for_connection(wapp);
			DBGPRINT(RT_DEBUG_OFF, "bh_link_ready: %d\n", wapp->map->bh_link_ready);
			DBGPRINT(RT_DEBUG_OFF, "[%s]: backhaul_mac("MACSTR")\n", __func__, MAC2STR(bh->backhaul_mac));
			DBGPRINT(RT_DEBUG_OFF, "[%s]: target_bssid("MACSTR")\n", __func__, MAC2STR(bh->target_bssid));
#ifdef DFS_CAC_R2
			os_memset(radio_id, 0, MAC_ADDR_LEN);
			if (wdev->radio) {
				ra = wdev->radio;
				MAP_GET_RADIO_IDNFER(ra, radio_id);
				band = (ra->op_ch > 14) ? BAND_5G : BAND_24G;
				if (band == BAND_5G) {
					ap_wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)radio_id);
					if (ap_wdev) {
						wapp_send_cac_req(wapp, ap_wdev->ifname, WAPP_SET_CAC_STOP, 0);
					}
				}
			}
#endif
			if (wdev->radio && (wdev->radio->op_ch != bh->channel)) {
				wdev_set_ch(wapp, wdev, bh->channel, bh->oper_class);
				eloop_register_timeout(1, 0, wapp_dpp_check_conn_wrapper, wapp, bh);
			}
			else {
				wapp->map->ch_change_done = 1;/*Since no channel change*/
				eloop_register_timeout(0, 0, wapp_dpp_check_conn_wrapper, wapp, bh);
			}
			//if (wapp_dpp_check_connect(wapp, wdev, (char *)bh->target_bssid, bh->channel) <= 0)
				//return MAP_ERROR;
			return MAP_SUCCESS;
		}
#endif
		wdev_set_ch(wapp, wdev, bh->channel, bh->oper_class);
#if NL80211_SUPPORT
		u8 Enable = 1;
		wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
				(char *)&Enable, 1);
#else
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
		ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set ApCliEnable=1",
			wdev->ifname);
		if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		if (system(cmd) == -1)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
		DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
#endif

	}
#if 0
	/* polling max 20 sec to wait for SiteSurvey & bh link ready before sending out bh steering rsp */
	for(i = 0;i < 20; i++) {
		if(wapp->map->bh_link_ready)
			break;
		sleep(1);
		sched_yield(); /* yield for recving driver's bh_link_ready event */
	}
#endif
	DBGPRINT(RT_DEBUG_OFF, "bh_link_ready: %d\n", wapp->map->bh_link_ready);
	DBGPRINT(RT_DEBUG_OFF, "[%s]: backhaul_mac("MACSTR")\n", __func__, MAC2STR(bh->backhaul_mac));
	DBGPRINT(RT_DEBUG_OFF, "[%s]: target_bssid("MACSTR")\n", __func__, MAC2STR(bh->target_bssid));

	if (bh)
		os_free(bh);

	return MAP_SUCCESS;
}

void map_handle_garp_request(struct wifi_app *wapp, struct garp_req_s *garp_req)
{
	int i = 0, j = 0;
	struct wapp_dev *wdev = NULL;
	unsigned char BC_MAC[MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uint32_t src = 0;
    uint32_t dst = inet_addr("10.10.10.254");

	/*in auto dhcp scenario, the agent may not get brigde ip at initial statge
	*and the agent's ip may change when it disconnects and reconnects to controller.
	*so it better get bridge ip when it send arp for bridge.
	*if still cannot get ip here, use the old ip.
	*/
	get_if_ip4(wapp->map->br.arp_sock, (const char *)wapp->map->br_iface,
		(uint32_t *)&(wapp->map->br.ip));

	if ((garp_req->dev_cnt== 1) &&
		!os_memcmp(garp_req->dev_addr_list[0].addr, BC_MAC, MAC_ADDR_LEN)) {
		for (i = 0; i < garp_req->sta_count; i++) {
			test_arping(wapp->map->br.arp_sock, wapp->map->br.ifindex,
				(char *)(garp_req->mac_addr_list[i].addr), src, dst);
			DBGPRINT(RT_DEBUG_TRACE, "ifname(%s)!\n", wapp->map->br_iface);
			DBGPRINT(RT_DEBUG_TRACE, "addr("MACSTR")\n",
				MAC2STR(garp_req->mac_addr_list[i].addr));
		}
	} else {
		for (j = 0; j < garp_req->dev_cnt; j++) {
			dl_list_for_each(wdev, &wapp->dev_list, struct wapp_dev, list){
				if (!os_memcmp(wdev->mac_addr, garp_req->dev_addr_list[j].addr, MAC_ADDR_LEN)) {
					break;
				}
			}
			if (wdev) {
				for (i = 0; i < garp_req->sta_count; i++) {
					test_arping(wdev->arp_sock, wdev->ifindex,
						(char *)(garp_req->mac_addr_list[i].addr), src, dst);
					DBGPRINT(RT_DEBUG_TRACE, "ifname(%s)!\n", wdev->ifname);
					DBGPRINT(RT_DEBUG_TRACE, "addr("MACSTR")\n",
						MAC2STR(garp_req->mac_addr_list[i].addr));
				}
			} else {
				DBGPRINT(RT_DEBUG_ERROR, "invalid mac("MACSTR")\n",
					MAC2STR(garp_req->dev_addr_list[j].addr));
			}
		}
	}
	test_arping(wapp->map->br.arp_sock, wapp->map->br.ifindex,
		wapp->map->br.mac_addr, (uint32_t)wapp->map->br.ip, 0);
}

void map_handle_dhcp_ctl_request(struct wifi_app *wapp, struct dhcp_ctl_req *dhcp_req){
	if (NULL == wapp || NULL == wapp->map || NULL == dhcp_req) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] input parameter is null!\n", __func__);
		return ;
	}

	if (strlen(wapp->map->br_iface) <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s]: bridge name is invalid,br_name is:%s!\n",
		__func__, wapp->map->br_iface);
		return ;
	}

	if (dhcp_req->dhcp_server_enable ==1 && dhcp_req->dhcp_client_enable == 0 ) {
		if (0 != enable_dhcp_server(wapp->map->br_iface)) {
			DBGPRINT(RT_DEBUG_ERROR, "enable_dhcp_server fail!\n");
		}else {
			DBGPRINT(RT_DEBUG_TRACE, "enable_dhcp_server success!\n");
		}
	}
	else if(dhcp_req->dhcp_server_enable ==0 && dhcp_req->dhcp_client_enable == 1) {
		if (0 != enable_dhcp_client(wapp->map->br_iface)) {
			DBGPRINT(RT_DEBUG_ERROR,"enable_dhcp_client fail!\n");
		}else {
			DBGPRINT(RT_DEBUG_TRACE, "enable_dhcp_client success!\n");
		}
	}else {
		DBGPRINT(RT_DEBUG_ERROR,
		"invalid cmd not support,dhcp_server_enable: %d,dhcp_client_enable: %d\n",
		dhcp_req->dhcp_server_enable,dhcp_req->dhcp_client_enable);
	}

	return;
}

void map_handle_get_br_ip_request(struct wifi_app * wapp) {
	char *ipbuf = NULL;
	int msg_size = 128;
	if (NULL == wapp) {
		DBGPRINT(RT_DEBUG_ERROR,"invalid input parameters!");
		return ;
	}
	os_alloc_mem(NULL, (UCHAR **)&ipbuf, msg_size);
	os_memset(ipbuf, 0, msg_size);

	if (get_bridge_ip(wapp->map->br_iface, ipbuf) != 0) {
		DBGPRINT(RT_DEBUG_ERROR," get ip fail!\n");
		os_free(ipbuf);
		return ;
	}
	msg_size = strlen(ipbuf) + 1;
	DBGPRINT(RT_DEBUG_OFF,"get ip success: %s\n", ipbuf);
	wapp_send_1905_msg(wapp, WAPP_BRIDGE_IP, msg_size, ipbuf);
	os_free(ipbuf);
}

void map_handle_set_br_default_ip_request(struct wifi_app * wapp) {
	if (NULL == wapp) {
		DBGPRINT(RT_DEBUG_ERROR,"invalid input parameters!");
		return ;
	}

	if (set_br_default_ip(wapp->map->br_iface) != 0) {
		DBGPRINT(RT_DEBUG_ERROR," Set ip fail!\n");
		return ;
	}
	DBGPRINT(RT_DEBUG_TRACE," Set br default ip success!\n");
	return ;
}

#ifdef ACL_CTRL
int map_config_acl_control_msg(
	struct wifi_app *wapp, unsigned char *bss_addr, char *msg_buf)
{
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	unsigned char *sta_addr = NULL;
	struct acl_ctrl *acl_ctrl_msg = NULL;
	int i = 0;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);

	acl_ctrl_msg = (struct acl_ctrl*)msg_buf;

	sta_addr = (unsigned char *) acl_ctrl_msg->sta_mac;

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, acl_ctrl_msg->bssid, WAPP_DEV_TYPE_AP);

	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return MAP_ERROR;
	}

	ap = (struct ap_dev *)wdev->p_dev;


	if (acl_ctrl_msg->cmd == ACL_ADD) {
		for (i = 0; i < acl_ctrl_msg->sta_list_count; i++) {
			if (ap->num_of_acl_cli++ < ap->max_num_of_block_cli)
				map_acl_system_cmd(wapp, wdev, sta_addr, acl_ctrl_msg->cmd);
			sta_addr += MAC_ADDR_LEN;
		}
	} else if (acl_ctrl_msg->cmd == ACL_DEL) {
		for (i = 0; i < acl_ctrl_msg->sta_list_count; i++) {
			if (ap->num_of_acl_cli > 0)
				ap->num_of_acl_cli--;
			map_acl_system_cmd(wapp, wdev, sta_addr, acl_ctrl_msg->cmd);
			sta_addr += MAC_ADDR_LEN;
		}
	} else {
		if (acl_ctrl_msg->cmd == ACL_FLUSH)
			ap->num_of_acl_cli = 0;
		map_acl_system_cmd(wapp, wdev, sta_addr, acl_ctrl_msg->cmd);
	}

	return MAP_SUCCESS;
}
#endif /*ACL_CTRL*/

int map_config_client_assoc_control_msg(
	struct wifi_app *wapp, unsigned char *bss_addr, char *msg_buf)
{
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	unsigned char *sta_addr = NULL;
	struct cli_assoc_control *assoc_ctrl = NULL;
	struct wapp_block_sta *block_sta = NULL;
	int i = 0;
	int status = -1;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	assoc_ctrl = (struct cli_assoc_control*)msg_buf;
	DBGPRINT(RT_DEBUG_OFF, "[%s]: assoc_ctrl->sta_list_count = %d\n", __func__, assoc_ctrl->sta_list_count);

	sta_addr = (unsigned char *) assoc_ctrl->sta_mac;

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, assoc_ctrl->bssid, WAPP_DEV_TYPE_AP);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return MAP_ERROR;
	}

	ap = (struct ap_dev *)wdev->p_dev;

	if (assoc_ctrl->assoc_control == BLOCK) {
		for (i = 0; i < assoc_ctrl->sta_list_count; i++) {
			DBGPRINT(RT_DEBUG_INFO, "Block sta("MACSTR"), valid_period(%d)\n",
									MAC2STR(sta_addr), assoc_ctrl->valid_period);

			block_sta = wdev_ap_block_list_lookup(wapp, ap, sta_addr);
			if (!block_sta) {
#ifdef ACL_CTRL
				status = wapp_add_block_sta(wapp, ap, sta_addr, assoc_ctrl->valid_period);
#else
				status = wapp_add_block_sta(wapp, ap, sta_addr, assoc_ctrl->valid_period);
#endif
			}
			else
				block_sta->valid_period = assoc_ctrl->valid_period;

			if (status == WAPP_SUCCESS) {
#ifdef ACL_CTRL
				map_blacklist_system_cmd(wdev, sta_addr, BLOCK);
#else
				map_acl_system_cmd(wapp, wdev, sta_addr, ACL_ADD);
#endif /*ACL_CTRL*/
			}

			sta_addr += MAC_ADDR_LEN;
		}
	} else if (assoc_ctrl->assoc_control == UNBLOCK) {
		for (i = 0; i < assoc_ctrl->sta_list_count; i++) {
			DBGPRINT(RT_DEBUG_INFO, "UnBlock sta("MACSTR")\n", MAC2STR(sta_addr));

			block_sta = wdev_ap_block_list_lookup(wapp, ap, sta_addr);
			if (block_sta) {
				wapp_del_block_sta(ap, sta_addr);
#ifdef ACL_CTRL
				map_blacklist_system_cmd(wdev, sta_addr, UNBLOCK);
#else
				map_acl_system_cmd(wapp, wdev, sta_addr, ACL_DEL);
#endif
			}
			sta_addr += MAC_ADDR_LEN;
		}
	}

	return MAP_SUCCESS;
}

int wapp_send_steering_completed_msg(
	struct wifi_app *wapp, char *buf, int* len_buf)
{
	struct evt *wapp_event;
	int send_pkt_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_STEERING_COMPLETED;
	wapp_event->length = 0;

	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}

int wapp_send_cli_steer_btm_report_msg(
	struct wifi_app *wapp, char *buf, int max_len, struct cli_steer_btm_event *btm_evt)
{
	struct evt *wapp_event;
	int send_pkt_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_CLI_STEER_BTM_REPORT;
	wapp_event->length = sizeof(struct cli_steer_btm_event);

	/*it seems almac and type will not used in topology discovery*/
	memcpy(wapp_event->buffer, btm_evt, wapp_event->length);

	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;
	DBGPRINT(RT_DEBUG_TRACE, "%d  send_pkt_len %d\n", __LINE__,send_pkt_len);
	if(0 > map_1905_send(wapp, buf, send_pkt_len)) {
		DBGPRINT(RT_DEBUG_ERROR, "send cli_steer_btm_report msg fail\n");
		return -1;
	}
	memset(buf, 0, send_pkt_len);

	return MAP_SUCCESS;
}

u8 set_btm_report_status(
	struct wifi_app *wapp, struct ap_dev *ap)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	return MAP_SUCCESS;
}

void map_trigger_btm_req(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	struct steer_request *steer_req,
	struct target_bssid_info *info,
	u8 req_mode)
{
	wapp_nr_info nr_entry;
#ifdef MAP_R2
	struct wapp_sta *sta = NULL;
	struct ap_dev *ap = NULL;
#endif
	u8 btm_neighbor_report_header[2] = {0};
	char cand_list[512] = {0};
	char buf[128] = {0};
	u16 append_entry_len = NEIGHBOR_REPORT_IE_SIZE;
	size_t btm_req_len = 0, cand_list_len = 0;
	u8 frame_pos = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	os_memset(&nr_entry,0,sizeof(nr_entry));
	/* element ID: 52, len: 16 */
	btm_neighbor_report_header[0] = IE_RRM_NEIGHBOR_REP;
	btm_neighbor_report_header[1] = append_entry_len;
	// fill neighbor report info
	os_memcpy(nr_entry.Bssid, info->target_bssid, MAC_ADDR_LEN);
	nr_entry.RegulatoryClass = info->op_class;
	nr_entry.ChNum = info->channel;
	nr_entry.CandidatePrefSubID = 0x3;
	nr_entry.CandidatePrefSubLen = 1;
	nr_entry.CandidatePref = 255;

	os_memcpy(&cand_list[frame_pos], &btm_neighbor_report_header, sizeof(btm_neighbor_report_header));
	frame_pos += sizeof(btm_neighbor_report_header);
	cand_list_len += sizeof(btm_neighbor_report_header);

	/* append this nr_entry */
	os_memcpy(&cand_list[frame_pos], &nr_entry, append_entry_len);
	cand_list_len += append_entry_len;
	//hex_dump_dbg("entry", (u8 *)frame_pos, append_entry_len);
	DBGPRINT(RT_DEBUG_OFF, RED("%s: steer_req->btm_disassoc_timer %d\n"), __func__,steer_req->btm_disassoc_timer);

	btm_req_len = wapp_build_btm_req(req_mode, steer_req->btm_disassoc_timer/100, 200,	//Validate interval
									NULL, NULL, 0, cand_list, cand_list_len, buf);

#ifdef MAP_R2
	ap = (struct ap_dev *) wdev->p_dev;
	sta = wdev_ap_client_list_lookup(wapp, ap, info->sta_mac);
	//if(steer_req->steering_type == STEERING_R2)
	if(sta && sta->cli_caps.mbo_capable) {
			/* append MBO IE */
			{
				u16 ie_len = 0;
				char *pos = buf + btm_req_len;
				//struct mbo_cfg *mbo = wapp->mbo;
				mbo_make_mbo_ie_for_btm(
							wapp,
							pos,
							&ie_len,
							FALSE,
							TRUE,
							info->reason,
							FALSE);
				btm_req_len += ie_len;
			}
	}
#endif

#ifndef KV_API_SUPPORT
	wapp_send_btm_req(wapp, wdev->ifname, info->sta_mac, buf, btm_req_len);
#else
	wapp_send_btm_req_11kv_api(wapp, wdev->ifname, info->sta_mac, buf, btm_req_len);
#endif /* KV_API_SUPPORT */
}

void map_trigger_deauth(
	struct wifi_app *wapp,
	char *ifname,
	unsigned char *sta_addr)
{
	int ret;
#if NL80211_SUPPORT
	struct wapp_dev *wdev = NULL;
	wdev = wapp_dev_list_lookup_by_ifname(wapp, ifname);
	if (wdev)
		wapp_set_DisConnectSta(wapp, (const char *)wdev->ifname,
				(char*)sta_addr, MAC_ADDR_LEN);
#else
	char cmd[256] = {0};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	DBGPRINT_RAW(RT_DEBUG_OFF, "disconnect sta:\n"
								"\t sta = "MACSTR"\n", MAC2STR(sta_addr));

	ret = snprintf(cmd, sizeof(cmd), "iwpriv %s set DisConnectSta=%02x:%02x:%02x:%02x:%02x:%02x;", ifname, PRINT_MAC(sta_addr));
	if (os_snprintf_error(sizeof(cmd), ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif
}
struct wapp_dev* wapp_dev_list_lookup_by_sta_mac_and_type(struct wifi_app *wapp, const u8 *mac_addr, const u8 wdev_type)
{
	struct wapp_dev *wdev, *target_wdev = NULL;
	struct dl_list *dev_list;
	struct wapp_sta *sta = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev->dev_type != WAPP_DEV_TYPE_AP) {
			DBGPRINT(RT_DEBUG_TRACE, "%s: wdev = %p wdev->dev_type != WAPP_DEV_TYPE_AP, continue!!!\n", __func__, wdev);
			continue;
		}

		sta = wdev_ap_client_list_lookup(wapp, (struct ap_dev *)wdev->p_dev, mac_addr);
		if (sta) {
			target_wdev = wdev;
			break;
		}
	}

	return target_wdev;

}

int map_config_beacon_metrics_query_msg(
	struct wifi_app *wapp, char *msg_buf,
	u8 *assoc_bssid)
{
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	struct wapp_sta *sta = NULL;
	struct beacon_metrics_query *bcn_req = NULL;
	struct ap_chn_rpt *ch_rpt = NULL;
#if NL80211_SUPPORT
#else
	char cmd[MAX_CMD_MSG_LEN] = {0};
	char ch_list[MAX_CH_NUM] = {0};
	char report_detail[8] = {0};
	char ie_list[MAX_ELEMNT_NUM*2+1] = {0};
	int ret;
#endif
	int i = 0, ch_list_len = 0;
#ifdef KV_API_SUPPORT
	p_bcn_req_info p_bcn_req_to_driver = NULL;
	unsigned int len = 0;
#endif /* KV_API_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

#ifdef KV_API_SUPPORT
	p_bcn_req_to_driver = (p_bcn_req_info)
			os_zalloc(sizeof(struct bcn_req_info_s));
	if (!p_bcn_req_to_driver) {
		DBGPRINT(RT_DEBUG_ERROR, "FAILED OOM");
		return MAP_ERROR;
	}
#endif /* KV_API_SUPPORT */

	bcn_req = (struct beacon_metrics_query *)msg_buf;

	if (assoc_bssid)
		wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, assoc_bssid, WAPP_DEV_TYPE_AP);
	else
		wdev = wapp_dev_list_lookup_by_sta_mac_and_type(wapp, bcn_req->sta_mac, WAPP_DEV_TYPE_AP);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
#ifdef KV_API_SUPPORT
		os_free(p_bcn_req_to_driver);
#endif /* KV_API_SUPPORT */
		return MAP_ERROR;
	}

	ap = (struct ap_dev *)wdev->p_dev;

	DBGPRINT(RT_DEBUG_OFF, "search AP wdev(%d)\n", wdev->ifindex);

	sta = wdev_ap_client_list_lookup(wapp, ap, bcn_req->sta_mac);

	if (!sta) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Sta not found\n", __func__);
#ifdef KV_API_SUPPORT
		os_free(p_bcn_req_to_driver);
#endif /* KV_API_SUPPORT */
		return MAP_ERROR;
	}
	if (sta->sta_status != WAPP_STA_CONNECTED) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Sta not connected\n", __func__);
#ifdef KV_API_SUPPORT
		os_free(p_bcn_req_to_driver);
#endif /* KV_API_SUPPORT */
		return MAP_ERROR;
	}

	DBGPRINT(RT_DEBUG_OFF, "find sta in wdev (%d)\n", wdev->ifindex);

if (wapp->map->MapMode == 4) {
	ch_rpt = bcn_req->rpt;
	ch_list_len = (ch_rpt->ch_rpt_len- 1);

	DBGPRINT(RT_DEBUG_OFF, "%s Send Becon Report Query \n", __func__);
#if NL80211_SUPPORT
	wapp_set_BcnReq(wapp, (const char *)wdev->ifname,
			(char *)bcn_req, sizeof(struct beacon_metrics_query));
#else
	ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set BcnReq=%02x:%02x:%02x:%02x:%02x:%02x!50!%d!%02x:%02x:%02x:%02x:%02x:%02x!%s!%d!1!%d!",
					wdev->ifname,
					bcn_req->sta_mac[0], bcn_req->sta_mac[1], bcn_req->sta_mac[2], bcn_req->sta_mac[3], bcn_req->sta_mac[4], bcn_req->sta_mac[5],
					bcn_req->oper_class,
					bcn_req->bssid[0], bcn_req->bssid[1], bcn_req->bssid[2], bcn_req->bssid[3], bcn_req->bssid[4], bcn_req->bssid[5],
					bcn_req->ssid,
					bcn_req->ch,
					ch_rpt->oper_class);
	if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

	for (i = 0; i < ch_list_len; i++) {
		if (i < (ch_list_len - 1)) {
			ret = snprintf(ch_list, MAX_CH_NUM, "%d#", ch_rpt->ch_list[i]);
			if (os_snprintf_error(MAX_CH_NUM, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		} else {
			ret = snprintf(ch_list, MAX_CH_NUM, "%d", ch_rpt->ch_list[i]);
			if (os_snprintf_error(MAX_CH_NUM, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		}
		ret = snprintf(cmd + strlen(cmd), MAX_CMD_MSG_LEN - strlen(cmd), "%s", ch_list);
		if (os_snprintf_error(MAX_CMD_MSG_LEN - strlen(cmd), ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	}

	ret = snprintf(report_detail, 8, "!%d!", bcn_req->rpt_detail_val);
	if (os_snprintf_error(8, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

	ret = snprintf(cmd + strlen(cmd), MAX_CMD_MSG_LEN - strlen(cmd), "%s", report_detail);
	if (os_snprintf_error(MAX_CMD_MSG_LEN - strlen(cmd), ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

	for (i = 0; i < bcn_req->elemnt_num; i++) {
		if (i < (bcn_req->elemnt_num - 1)) {
			ret = snprintf(ie_list, (MAX_ELEMNT_NUM*2+1), "%d#", bcn_req->elemnt_list[i]);
			if (os_snprintf_error((MAX_ELEMNT_NUM*2+1), ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		} else {
			ret = snprintf(ie_list, (MAX_ELEMNT_NUM*2+1), "%d", bcn_req->elemnt_list[i]);
			if (os_snprintf_error((MAX_ELEMNT_NUM*2+1), ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		}
		ret = snprintf(cmd + strlen(cmd), MAX_CMD_MSG_LEN - strlen(cmd), "%s", ie_list);
		if (os_snprintf_error(MAX_CMD_MSG_LEN - strlen(cmd), ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	}

	printf("cmd: %s\n", cmd);
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif

#ifdef KV_API_SUPPORT
	os_free(p_bcn_req_to_driver);
#endif /* KV_API_SUPPORT */
} else {
#ifdef KV_API_SUPPORT
	os_memcpy(p_bcn_req_to_driver->peer_address, bcn_req->sta_mac, MAC_ADDR_LEN);
	p_bcn_req_to_driver->regclass = bcn_req->oper_class;
	p_bcn_req_to_driver->channum = bcn_req->ch;
	if (wapp->map->MapMode == 1)
		p_bcn_req_to_driver->timeout = 2;
	os_memcpy(p_bcn_req_to_driver->bssid, bcn_req->bssid, MAC_ADDR_LEN);
	if(bcn_req->ssid_len) {
		os_memcpy(p_bcn_req_to_driver->req_ssid, bcn_req->ssid, bcn_req->ssid_len);
		p_bcn_req_to_driver->req_ssid_len = bcn_req->ssid_len;
	}

	ch_rpt = bcn_req->rpt;
	ch_list_len = (ch_rpt->ch_rpt_len- 1);

	if (ch_list_len >= 0)
		p_bcn_req_to_driver->ch_list_len = (ch_list_len < CH_LEN) ? ch_list_len : CH_LEN;
	else
		p_bcn_req_to_driver->ch_list_len = 0;

	for (i = 0; i < p_bcn_req_to_driver->ch_list_len && i < MAX_CH_NUM; i++)
		p_bcn_req_to_driver->ch_list[i] = ch_rpt->ch_list[i];

	p_bcn_req_to_driver->detail = bcn_req->rpt_detail_val;
	p_bcn_req_to_driver->request_len = (bcn_req->elemnt_num < REQ_LEN) ? bcn_req->elemnt_num : REQ_LEN;

	for (i = 0; i < p_bcn_req_to_driver->request_len && i < MAX_ELEMNT_NUM; i++)
		p_bcn_req_to_driver->request[i] = bcn_req->elemnt_list[i];
	p_bcn_req_to_driver->timeout = RRM_REQUEST_TIMEOUT;

	len = sizeof(*p_bcn_req_to_driver);

	/* send beacon request frame */
	driver_rrm_send_bcn_req_param(wapp->drv_data, wdev->ifname, (const char *)p_bcn_req_to_driver, len);

	os_free(p_bcn_req_to_driver);
#else
        ch_rpt = bcn_req->rpt;
        ch_list_len = (ch_rpt->ch_rpt_len- 1);

        DBGPRINT(RT_DEBUG_OFF, "%s Send Becon Report Query \n", __func__);
#if NL80211_SUPPORT
	wapp_set_BcnReq(wapp, (const char *)wdev->ifname,
			(char *)bcn_req, sizeof(struct beacon_metrics_query));
#else
        snprintf(cmd, MAX_CMD_MSG_LEN,"iwpriv %s set BcnReq=%02x:%02x:%02x:%02x:%02x:%02x!50!%d!%02x:%02x:%02x:%02x:%02x:%02x!%s!%d!1!%d!",
                                        wdev->ifname,
                                        bcn_req->sta_mac[0], bcn_req->sta_mac[1], bcn_req->sta_mac[2], bcn_req->sta_mac[3], bcn_req->sta_mac[4], bcn_req->sta_mac[5],
                                        bcn_req->oper_class,
                                        bcn_req->bssid[0], bcn_req->bssid[1], bcn_req->bssid[2], bcn_req->bssid[3], bcn_req->bssid[4], bcn_req->bssid[5],
                                        bcn_req->ssid,
                                        bcn_req->ch,
                                        ch_rpt->oper_class);

        for (i = 0; i < ch_list_len; i++) {
                if (i < (ch_list_len - 1))
                        snprintf(ch_list,MAX_CH_NUM, "%d#", ch_rpt->ch_list[i]);
                else
                        snprintf(ch_list,MAX_CH_NUM, "%d", ch_rpt->ch_list[i]);

                strcat(cmd, ch_list);
        }

        snprintf(report_detail,8, "!%d!", bcn_req->rpt_detail_val);
        strcat(cmd, report_detail);

        for (i = 0; i < bcn_req->elemnt_num; i++) {
                if (i < (bcn_req->elemnt_num - 1))
                        snprintf(ie_list,(MAX_ELEMNT_NUM*2+1), "%d#", bcn_req->elemnt_list[i]);
                else
                        snprintf(ie_list,(MAX_ELEMNT_NUM*2+1), "%d", bcn_req->elemnt_list[i]);
                strcat(cmd, ie_list);
        }

        printf("cmd: %s\n", cmd);
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif /* KV_API_SUPPORT */
#endif
}

	return MAP_SUCCESS;
}

int map_config_steering_setting_msg(
	struct wifi_app *wapp, char *msg_buf, char *evt_buf, int* len_buf)
{
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	struct wapp_sta *sta = NULL;
	struct steer_request *steer_req = NULL;
	struct target_bssid_info *info = NULL;
	//char buf[128] = {0};
	int sta_cnt = 0;
	u8 req_mode = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	steer_req = (struct steer_request *)msg_buf;
	req_mode = (steer_req->btm_abridged << ABIDGED_BIT_MAP) |
				(steer_req->btm_disassoc_immi << DISASSOC_IMNT_BIT_MAP);

	DBGPRINT_RAW(RT_DEBUG_OFF, "req_mode(%u), steer_window(%hu), btm_disassoc_timer(%hu), sta_count(%u)\n",
				steer_req->request_mode, steer_req->steer_window, steer_req->btm_disassoc_timer, steer_req->sta_count);

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, steer_req->assoc_bssid, WAPP_DEV_TYPE_AP);

	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return MAP_ERROR;
	}

	ap = (struct ap_dev *)wdev->p_dev;

	if (steer_req->request_mode == STEERING_MANDATE) {
		DBGPRINT(RT_DEBUG_INFO, "(STEERING_MANDATE)\n");
		sta_cnt = steer_req->sta_count ;
		info = (struct target_bssid_info *) (steer_req->info);

		while (sta_cnt > 0) {
			sta = wdev_ap_client_list_lookup(wapp, ap, info->sta_mac);
			if (!sta) {
				DBGPRINT(RT_DEBUG_ERROR, "%s null sta\n", __func__);
				return MAP_ERROR;
			}

			if (sta->bLocalSteerDisallow == FALSE) {
				if ((sta->bBSSMantSupport == TRUE) && (sta->bBTMSteerDisallow == FALSE)) {
					DBGPRINT(RT_DEBUG_INFO, "bBSSMantSupport Sta\n");
					map_trigger_btm_req(wapp, wdev, steer_req, info, req_mode);
				} else {
					DBGPRINT(RT_DEBUG_INFO, "Legacy Sta\n");
					map_trigger_deauth(wapp, wdev->ifname, info->sta_mac);
				}
			}
			else
				DBGPRINT(RT_DEBUG_WARN, "%s Local steering disallow sta \n", __func__);

			sta_cnt--;
			info++;
		}
	}	else if (steer_req->request_mode == STEERING_OPPORTUNITY) {
		DBGPRINT(RT_DEBUG_INFO, "(STEERING_OPPORTUNITY)\n");
		//To Do

		wapp_send_steering_completed_msg(wapp, evt_buf, len_buf);
	}

	return MAP_SUCCESS;
}

int map_send_chn_pref_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	send_pkt_len = map_build_chn_pref(wapp, addr, evt_buf);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}

int map_send_radio_op_restrict_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	send_pkt_len = map_build_ra_op_restrict(wapp, addr, evt_buf);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}

int map_send_radio_basic_capability_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	send_pkt_len = map_build_ap_ra_basic_cap(wapp, addr, evt_buf);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}

int map_send_ap_capability_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	send_pkt_len = map_build_ap_cap(wapp, addr, evt_buf);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}

int map_send_ap_ht_capability_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	send_pkt_len = map_build_ap_ht_cap(wapp, addr, evt_buf);
	if (send_pkt_len <= 0) {
		printf("[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}

int map_send_ap_vht_capability_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	send_pkt_len = map_build_ap_vht_cap(wapp, addr, evt_buf);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}

#ifdef MAP_R3_WF6
int map_send_ap_wf6_capability_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	DBGPRINT(RT_DEBUG_TRACE, "WF6:WAPPD:%s Sending the AP WF6 capa to MAPD\n", __func__);

	send_pkt_len = map_build_ap_wf6_cap(wapp, addr, evt_buf);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}
#endif /*MAP_R3_WF6*/

#ifdef MAP_R4_SPT
int map_build_spt_reuse_query(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf);


int map_send_ap_spt_reuse_req_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);

	DBGPRINT(RT_DEBUG_TRACE, "SPT Reuse:WAPPD:%s Sending the AP WF6 capa to MAPD\n", __func__);

	send_pkt_len = map_build_spt_reuse_query(wapp, addr, evt_buf);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}
#endif


int map_send_ap_he_capability_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	send_pkt_len = map_build_ap_he_cap(wapp, addr, evt_buf);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}

#ifdef MAP_R3_DE
int map_send_dev_inven_tlv(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	send_pkt_len = map_send_dev_inven(wapp, addr, evt_buf);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}
#endif /*MAP_R3_DE*/

#ifdef MAP_R3
int map_send_dpp_uri_msg(
	struct wifi_app *wapp, char *evt_buf, int* len_buf);

int map_send_dpp_uri_msg(
	struct wifi_app *wapp, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	struct evt *map_event = NULL;
	struct dpp_bootstrap_info *bi = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	DBGPRINT(RT_DEBUG_OFF, "WAPPD:%s Sending the DPP URI to MAPD\n", __func__);


	if(wapp->map && (wapp->map->map_version != DEV_TYPE_R3)) {
		wapp_version_mismatch(wapp);
		return MAP_ERROR;
	}

	if(!wapp->dpp) {
		dpp_auth_fail_wrapper(wapp, "DPP not init yet");
		return MAP_ERROR;
	}

	bi = dpp_get_own_bi(wapp->dpp);
	if(!bi) {
		dpp_auth_fail_wrapper(wapp, "Own URI not found,Generate First");
		return MAP_ERROR;
	}

	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_RX_DPP_URI;
	map_event->length = os_strlen(bi->uri);
	os_memcpy(map_event->buffer, bi->uri, os_strlen(bi->uri));
	map_event->buffer[os_strlen(bi->uri)] = '\0';

	send_pkt_len = sizeof(*map_event) + map_event->length;

	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}
#endif /* MAP_R3 */

#ifdef MAP_R2
int map_build_scan_cap(struct wifi_app *wapp, char *evt_buf);
int map_build_r2_ap_cap(struct wifi_app *wapp, char *evt_buf);


u8 mapd_get_channel_scan_capab_from_driver(struct wifi_app *wapp);


int map_send_channel_scan_capability_msg(
	struct wifi_app *wapp, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(mapd_get_channel_scan_capab_from_driver(wapp) == FALSE)
		return FALSE;

	send_pkt_len = map_build_scan_cap(wapp, evt_buf);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}

int map_send_r2_ap_capability_msg(
	struct wifi_app *wapp, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	send_pkt_len = map_build_r2_ap_cap(wapp, evt_buf);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}


#ifdef DFS_CAC_R2

int map_build_cac_cap(
	struct wifi_app *wapp, char *evt_buf)
{
	struct evt *map_event = NULL;
	int send_pkt_len = 0;
	unsigned char *buff;
	u8 i=0, j=0,m=0, ptr=0, l=0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	DBGPRINT(RT_DEBUG_ERROR,"%s %d\n", __func__, __LINE__);
	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_CAC_CAPAB;
	map_event->length = wapp->map->cac_capab_final_len;
//	os_memcpy(map_event->buffer, wapp->map->cac_capab, wapp->map->cac_capab_final_len);
	buff = map_event->buffer;
	os_memcpy(buff, wapp->map->cac_capab->country_code, 2);
	buff[2]=wapp->map->cac_capab->radio_num;
	ptr += 3;
	for (i=0; i < wapp->map->cac_capab->radio_num; i++) {
		os_memcpy(&buff[ptr], wapp->map->cac_capab->cap[i].identifier, MAC_ADDR_LEN);
		ptr += 6;
		buff[ptr] = wapp->map->cac_capab->cap[i].cac_type_num;
		ptr += 1;

		for(m=0; m < wapp->map->cac_capab->cap[i].cac_type_num; m++) {
			buff[ptr] = wapp->map->cac_capab->cap[i].type[m].cac_mode;
			ptr += 1;
			os_memcpy(&buff[ptr], &wapp->map->cac_capab->cap[i].type[m].cac_interval[0], 3);
			ptr += 3;
			buff[ptr] = wapp->map->cac_capab->cap[i].type[m].op_class_num;
			ptr += 1;
			for(j=0; j < wapp->map->cac_capab->cap[i].type[m].op_class_num; j++)
			{
				buff[ptr] = wapp->map->cac_capab->cap[i].type[m].opcap[j].op_class;
				ptr += 1;
				buff[ptr] = wapp->map->cac_capab->cap[i].type[m].opcap[j].ch_num;
				ptr += 1;
				for(l=0; l < wapp->map->cac_capab->cap[i].type[m].opcap[j].ch_num; l++)
				{
					buff[ptr] = wapp->map->cac_capab->cap[i].type[m].opcap[j].ch_list[l];
					ptr += 1;
				}
			}
		}
	}
	DBGPRINT(RT_DEBUG_ERROR,"\n ptr %d, map_event->length-%d",ptr,map_event->length);
	send_pkt_len = sizeof(*map_event) + map_event->length;
	return send_pkt_len;
}


int map_send_cac_capability_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
//	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(mapd_get_cac_capab_from_driver(wapp, addr) == FALSE)
		return FALSE;
#if 1
	send_pkt_len = map_build_cac_cap(wapp, evt_buf);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
#endif
	return MAP_SUCCESS;
}

int map_send_cac_status_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf);

int map_send_cac_status_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf)
{
	u8 cac_completion = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	mapd_get_cac_status_from_driver(wapp, evt_buf, len_buf, cac_completion);

	if (*len_buf <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}

	return MAP_SUCCESS;
}


#endif
int map_send_metric_reporting_capability_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);

	send_pkt_len = map_build_metric_reporting_info(wapp, addr, evt_buf);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}

#endif
int map_send_empty_msg(
	struct wifi_app *wapp, unsigned short msg_type, char *evt_buf)
{
	struct evt *map_event = NULL;
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	map_event = (struct evt *)evt_buf;
	map_event->type = msg_type;
	map_event->length = 0;
	send_pkt_len = sizeof(struct evt);

	if (0 > map_1905_send(wapp, evt_buf, send_pkt_len)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): send empty msg fail\n", __func__, __LINE__);
		return MAP_ERROR;
	}
	return MAP_SUCCESS;
}

int map_recieve_txrx_link_stats_msg(
	struct wifi_app *wapp, char *msg_buf, unsigned short msg_type, unsigned char msg_role, char *evt_buf, int* len_buf)
{
	struct wapp_dev *wdev = NULL;
	struct link_stat_query *link_qry;
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	link_qry = (struct link_stat_query *)msg_buf;
	if (link_qry->media_type == MAP_BH_ETH) {
		if (msg_type == WAPP_USER_GET_TX_LINK_STATISTICS)
			send_pkt_len = map_build_eth_tx_link_stats(wapp, msg_buf, evt_buf);
		else if (msg_type == WAPP_USER_GET_RX_LINK_STATISTICS)
			send_pkt_len = map_build_eth_rx_link_stats(wapp, msg_buf, evt_buf);
		else
			return MAP_ERROR;

		if (send_pkt_len < 0) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
			return MAP_ERROR;
		}
		*len_buf = send_pkt_len;

		return MAP_SUCCESS;
	}
	else {
		link_qry = (struct link_stat_query *)msg_buf;

		wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, link_qry->local_if, WAPP_DEV_TYPE_STA);
		if (wdev) {
			wapp_query_wdev_by_req_id(wapp, wdev->ifname, WAPP_APCLI_QUERY_REQ);
		} else {
			wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, link_qry->local_if, WAPP_DEV_TYPE_AP);
			if (wdev) {
				wapp_query_cli(wapp, wdev->ifname, link_qry->neighbor_if);
			} else {
				DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
				return MAP_ERROR;
			}
		}
	}
	return MAP_SUCCESS;
}


int map_recieve_ap_metric_msg(struct wifi_app *wapp, u8 *bssid)
{
	struct wapp_dev *wdev = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bssid, WAPP_DEV_TYPE_AP);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return MAP_ERROR;
	}

	wapp_query_wdev_by_req_id(wapp, wdev->ifname, WAPP_AP_METRIC_QUERY_REQ);
	return MAP_SUCCESS;
}

#ifdef MAP_R2
int map_recieve_radio_metric_msg(struct wifi_app *wapp, char *radio_id)
{
	struct wapp_dev *wdev = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	DBGPRINT(RT_DEBUG_TRACE,
	"idfer	=	%02x%02x%02x%02x%02x%02x\n",
	PRINT_RA_IDENTIFIER(radio_id)
	);
	wdev = wapp_dev_list_lookup_by_radio(wapp, radio_id);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return MAP_ERROR;
	}

	DBGPRINT(RT_DEBUG_TRACE,
	"idfer	=	%02x%02x%02x%02x%02x%02x\n",
	PRINT_RA_IDENTIFIER(radio_id)
	);
	//os_strcpy(iface,"ra0");
	//wdev = wapp_dev_list_lookup_by_ifname(wapp,iface);

	wapp_query_wdev_by_req_id(wapp, wdev->ifname, WAPP_RADIO_METRICS_REQ);
	return MAP_SUCCESS;
}
#endif


int map_send_ap_metric_msg(struct wifi_app *wapp, struct ap_dev *ap)
{
	int send_pkt_len = 0;
	char *map_evt_buf;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (prev_1905_msg == WAPP_USER_GET_AP_METRICS_INFO) {
		map_evt_buf = (char*)malloc(MAX_EVT_BUF_LEN * sizeof(char));
		if (!map_evt_buf) {
			DBGPRINT(RT_DEBUG_TRACE, "%s  Alloc memory failed !!!!! \n", __func__);
			return MAP_ERROR;
		}
		memset(map_evt_buf, 0, MAX_EVT_BUF_LEN);
		send_pkt_len = map_build_ap_metric(wapp, ap, map_evt_buf);
	}
	else {
		return MAP_ERROR;
	}


	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		goto err;
	}

	if (0 > map_1905_send(wapp, map_evt_buf, send_pkt_len)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): send ap metric msg fail\n", __func__, __LINE__);
		goto err;
	}

	free(map_evt_buf);
	return MAP_SUCCESS;
err:
	free(map_evt_buf);
	return MAP_ERROR;

}

#ifdef MAP_R2

int map_send_radio_metric_msg(struct wifi_app *wapp, wapp_event_data *event_data, u32 ifindex)
{
	int send_pkt_len = 0;
	char *map_evt_buf;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

//	if (prev_1905_msg == WAPP_USER_GET_RADIO_METRICS_INFO) {
	map_evt_buf = (char*)malloc(MAX_EVT_BUF_LEN * sizeof(char));
	if (!map_evt_buf) {
		DBGPRINT(RT_DEBUG_TRACE, "%s  Alloc memory failed !!!!! \n", __func__);
		return MAP_ERROR;
	}
	memset(map_evt_buf, 0, MAX_EVT_BUF_LEN);
	send_pkt_len = map_build_radio_metric(wapp, event_data, map_evt_buf, ifindex);
	//}
	//else {
		//return MAP_ERROR;
	//}


	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		goto err;
	}

	if (0 > map_1905_send(wapp, map_evt_buf, send_pkt_len)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): send radio metric msg fail\n", __func__, __LINE__);
		goto err;
	}

	free(map_evt_buf);
	return MAP_SUCCESS;
err:
	free(map_evt_buf);
	return MAP_ERROR;

}
#endif

int map_recieve_assoc_sta_msg(struct wifi_app *wapp, char *ra_identifier)
{
	struct wapp_dev *wdev = NULL;
	struct os_time now;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_radio(wapp, ra_identifier);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return MAP_ERROR;
	}

	map_send_assoc_sta_msg(wapp);

	os_get_time(&now);
	if (now.sec >= wdev->cli_list_last_update_time.sec + 1) {
		if (wdev->cli_info_trigger == FALSE) {
			DBGPRINT_RAW(RT_DEBUG_TRACE, "WAPP_CLI_LIST_QUERY_REQ: %d %s\n", wdev->ifindex, wdev->ifname);
			wapp_query_wdev_by_req_id(wapp, wdev->ifname, WAPP_CLI_LIST_QUERY_REQ);
			wdev->cli_info_trigger = TRUE;
		}
	}

	return MAP_SUCCESS;
}

int map_send_assoc_sta_msg(struct wifi_app *wapp)
{
	int send_pkt_len = 0;
	char *map_evt_buf = NULL;

	if (prev_1905_msg == WAPP_USER_GET_ASSOC_STA_TRAFFIC_STATS)
		send_pkt_len = map_get_assoc_sta_traffic_stats_len(wapp, prev_req_radio_id);
	else if (prev_1905_msg == WAPP_USER_GET_ASSOC_STA_LINK_METRICS)
		send_pkt_len = map_get_assoc_sta_link_metric_len(wapp, prev_req_radio_id);
	else if (prev_1905_msg == WAPP_USER_GET_ALL_ASSOC_TP_METRICS)
		send_pkt_len = map_get_assoc_sta_tp_metric_len(wapp, prev_req_radio_id);
#ifdef MAP_R2
	else if (prev_1905_msg == WAPP_USER_GET_ASSOC_STA_EXTENDED_LINK_METRICS)
		send_pkt_len = map_get_assoc_sta_ext_metric_len(wapp, prev_req_radio_id);
#endif
#ifdef MAP_R3_WF6
	else if (prev_1905_msg == WAPP_USER_GET_ASSOC_WIFI6_STA_STATUS)
		send_pkt_len = map_get_assoc_wifi6_sta_status_len(wapp, prev_req_radio_id);
#endif
	else
		return MAP_ERROR;
	DBGPRINT(RT_DEBUG_TRACE, "%s  need Alloc memory len:%d !!!!!\n", __func__, send_pkt_len);

	map_evt_buf = (char *)malloc(send_pkt_len);
	if (!map_evt_buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s  Alloc memory failed !!!!! \n", __func__);
		return MAP_ERROR;
	}
	memset(map_evt_buf, 0, send_pkt_len);

	if (prev_1905_msg == WAPP_USER_GET_ASSOC_STA_TRAFFIC_STATS)
		send_pkt_len = map_build_assoc_sta_traffic_stats(wapp, map_evt_buf, prev_req_radio_id);
	else if (prev_1905_msg == WAPP_USER_GET_ASSOC_STA_LINK_METRICS)
		send_pkt_len = map_build_assoc_sta_link_metric(wapp, map_evt_buf, prev_req_radio_id);
	else if (prev_1905_msg == WAPP_USER_GET_ALL_ASSOC_TP_METRICS)
		send_pkt_len = map_build_assoc_sta_tp_metric(wapp, map_evt_buf, prev_req_radio_id);
#ifdef MAP_R2
	else if (prev_1905_msg == WAPP_USER_GET_ASSOC_STA_EXTENDED_LINK_METRICS) {
		//printf("build map assoc ext metrics\n");
		send_pkt_len = map_build_assoc_sta_ext_metric(wapp, map_evt_buf, prev_req_radio_id);
	}
#endif
#ifdef MAP_R3_WF6
	else if (prev_1905_msg == WAPP_USER_GET_ASSOC_WIFI6_STA_STATUS) {
		DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:: calling WAPP_USER_GET_ASSOC_WIFI6_STA_STATUS\n");
		send_pkt_len = map_build_assoc_wifi6_sta_status(wapp, map_evt_buf, prev_req_radio_id);
	}
#endif
	else
		goto err;


	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		goto err;
	}

	if (0 > map_1905_send(wapp, map_evt_buf, send_pkt_len)) {
		if (prev_1905_msg == WAPP_USER_GET_ASSOC_STA_TRAFFIC_STATS) {
			DBGPRINT(RT_DEBUG_ERROR, "%s send assoc sta traffic stats msg fail\n", __func__);
		} else if (prev_1905_msg == WAPP_USER_GET_ASSOC_STA_LINK_METRICS) {
			DBGPRINT(RT_DEBUG_ERROR, "%s send assoc sta link metric msg fail\n", __func__);
		} else if (prev_1905_msg == WAPP_USER_GET_ALL_ASSOC_TP_METRICS) {
			DBGPRINT(RT_DEBUG_ERROR, "%s send assoc sta tp metric msg fail\n", __func__);
#ifdef MAP_R2
		} else if (prev_1905_msg == WAPP_USER_GET_ASSOC_STA_EXTENDED_LINK_METRICS) {
			DBGPRINT(RT_DEBUG_ERROR, "%s send assoc sta ext metric msg fail\n", __func__);
#endif
#ifdef MAP_R3_WF6
		} else if (prev_1905_msg == WAPP_USER_GET_ASSOC_WIFI6_STA_STATUS) {
			DBGPRINT(RT_DEBUG_ERROR, "%s send assoc wifi6 sta status fail\n", __func__);
#endif
		}
		goto err;
	}

	free(map_evt_buf);
	return MAP_SUCCESS;
err:
	free(map_evt_buf);
	return MAP_ERROR;
}

int map_recieve_one_assoc_sta_msg(struct wifi_app *wapp, u8 *mac_addr)
{
	struct dl_list *dev_list;
	struct wapp_dev *wdev = NULL;
	struct ap_dev	*ap = NULL;
	struct wapp_sta * sta = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
			ap = (struct ap_dev *)wdev->p_dev;
			sta = wdev_ap_client_list_lookup(wapp, ap, mac_addr);
			if (sta) {
				if (sta->sta_status == WAPP_STA_CONNECTED) {
					wapp_query_cli(wapp, wdev->ifname, mac_addr);
					return MAP_SUCCESS;
				}
			}
		}
	}

	DBGPRINT(RT_DEBUG_ERROR, "%s assoc sta not found!\n", __func__);
	return MAP_ERROR;
}


int map_send_one_assoc_sta_msg(struct wifi_app *wapp, struct wapp_sta *sta)
{
	int send_pkt_len = 0;
	char *map_evt_buf;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);


	if (prev_1905_msg == WAPP_USER_GET_ONE_ASSOC_STA_LINK_METRICS ||
		prev_1905_msg == WAPP_USER_GET_TX_LINK_STATISTICS ||
		prev_1905_msg == WAPP_USER_GET_RX_LINK_STATISTICS ||
		prev_1905_msg == WAPP_USER_GET_ONE_ASSOC_STA_TRAFFIC_STATS
#ifdef MAP_R2
		|| prev_1905_msg == WAPP_USER_GET_ONE_ASSOC_STA_EXTENDED_LINK_METRICS
#endif
		) {
		map_evt_buf = (char*)malloc(MAX_EVT_BUF_LEN * sizeof(char));
		if (!map_evt_buf) {
			DBGPRINT(RT_DEBUG_TRACE, "%s  Alloc memory failed !!!!! \n", __func__);
			return MAP_ERROR;
		}
		memset(map_evt_buf, 0, MAX_EVT_BUF_LEN);
		if (prev_1905_msg == WAPP_USER_GET_ONE_ASSOC_STA_LINK_METRICS)
			send_pkt_len = map_build_one_assoc_sta_link_metric(wapp, sta, map_evt_buf);
		else if (prev_1905_msg == WAPP_USER_GET_TX_LINK_STATISTICS)
			send_pkt_len = map_build_wifi_tx_link_stats(wapp, sta, map_evt_buf);
		else if (prev_1905_msg == WAPP_USER_GET_RX_LINK_STATISTICS)
			send_pkt_len = map_build_wifi_rx_link_stats(wapp, sta, map_evt_buf);
		else if (prev_1905_msg == WAPP_USER_GET_ONE_ASSOC_STA_TRAFFIC_STATS)
			send_pkt_len = map_build_one_assoc_sta_traffic_stats(wapp, sta, map_evt_buf);
#ifdef MAP_R2
		else if (prev_1905_msg == WAPP_USER_GET_ONE_ASSOC_STA_EXTENDED_LINK_METRICS) {
			DBGPRINT(RT_DEBUG_TRACE,"sending one assoc link metrics\n");
			send_pkt_len = map_build_one_assoc_sta_ext_link_metric(wapp, sta, map_evt_buf);
		}
#endif
	}
	else {
		return MAP_ERROR;
	}


	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		goto err;
	}

	if(0 > map_1905_send(wapp, map_evt_buf, send_pkt_len)) {
		if (prev_1905_msg == WAPP_USER_GET_ONE_ASSOC_STA_LINK_METRICS) {
			DBGPRINT(RT_DEBUG_TRACE, "%s send one assoc sta msg fail\n", __func__);
		} else if (prev_1905_msg == WAPP_USER_GET_TX_LINK_STATISTICS) {
			DBGPRINT(RT_DEBUG_TRACE, "%s send tx link statistics msg fail\n", __func__);
		} else if (prev_1905_msg == WAPP_USER_GET_RX_LINK_STATISTICS) {
			DBGPRINT(RT_DEBUG_TRACE, "%s send rx link statistics msg fail\n", __func__);
		} else if (prev_1905_msg == WAPP_USER_GET_ONE_ASSOC_STA_TRAFFIC_STATS) {
			DBGPRINT(RT_DEBUG_TRACE, "%s send one assoc stat traffic stats msg fail\n", __func__);
		}
#ifdef MAP_R2
                else if (prev_1905_msg == WAPP_USER_GET_ONE_ASSOC_STA_EXTENDED_LINK_METRICS) {
			DBGPRINT(RT_DEBUG_TRACE, "%s send one assoc ext sta msg fail\n", __func__);
		}
#endif
		goto err;
	}

	free(map_evt_buf);
	return MAP_SUCCESS;
err:
	free(map_evt_buf);
	return MAP_ERROR;

}


int map_send_unassoc_sta_link_metrics_msg(
	struct wifi_app *wapp, char *msg_buf, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	send_pkt_len = map_build_unassoc_sta_link_metrics(wapp, msg_buf, evt_buf);
	if (send_pkt_len < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}


int map_config_metrics_policy_msg(
	struct wifi_app *wapp, char *msg_buf)
{
	struct wapp_radio *ra = NULL;
	struct metric_policy *policy = NULL;
	struct metric_policy_head * policy_head = NULL;
	u8 idfr[MAC_ADDR_LEN];
	int i, j;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	policy = (struct metric_policy *)msg_buf;
	DBGPRINT(RT_DEBUG_INFO, "WAPP got metric policy, policy count: %d\n", policy->policy_cnt);

	for (i = 0; i < policy->policy_cnt; i++) {
		policy_head = &policy->policy[i];
		for (j = 0; j < MAX_NUM_OF_RADIO; j++)
		{
			ra = &wapp->radio[j];
			if (ra->adpt_id) {
				MAP_GET_RADIO_IDNFER(ra, idfr);
				if (!os_memcmp(policy_head->identifier, idfr, ETH_ALEN)) {
					ra->metric_policy.sta_rssi_thres = policy_head->rssi_thres;
					ra->metric_policy.sta_hysteresis_margin = policy_head->hysteresis_margin;
					ra->metric_policy.ch_util_thres = policy_head->ch_util_thres;

					DBGPRINT(RT_DEBUG_INFO, "save policy to radio: %02x%02x%02x%02x%02x%02x\n", PRINT_RA_IDENTIFIER(idfr));
					DBGPRINT(RT_DEBUG_INFO, "sta_rssi_thres: %d\n", ra->metric_policy.sta_rssi_thres);
					DBGPRINT(RT_DEBUG_INFO, "sta_hysteresis_margin: %d\n", ra->metric_policy.sta_hysteresis_margin);
					DBGPRINT(RT_DEBUG_INFO, "ch_util_thres: %d\n", ra->metric_policy.ch_util_thres);

					ra->metric_policy.ch_util_current = 0;
					ra->metric_policy.ch_util_prev = 0;
				}
			}
		}
	}

	return MAP_SUCCESS;
}


int map_send_ap_oper_bss_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	send_pkt_len = map_build_ap_op_bss(wapp, addr, evt_buf);
	if (send_pkt_len < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;

	return MAP_SUCCESS;
}

int map_send_assoc_cli_msg(
	struct wifi_app *wapp, unsigned char *bss_addr, unsigned char *sta_addr, unsigned char stat,
	char *evt_buf, u16 disassoc_reason)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	send_pkt_len = map_build_assoc_cli(wapp, bss_addr, sta_addr, stat, evt_buf, disassoc_reason);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size <= 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}

	if (0 > map_1905_send(wapp, evt_buf, send_pkt_len)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): send assoc client msg fail\n", __func__, __LINE__);
		return MAP_ERROR;
	}

	return MAP_SUCCESS;
}
void  map_send_ch_list_dfs_info(
	struct wifi_app *wapp, unsigned char *addr)
{
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap ;
	struct chnList chn_list[16] = {0};
	u8 i=0;
	DBGPRINT(RT_DEBUG_ERROR, "%s\n", __func__);
	if (!wapp)
		return;
	wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)addr);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return;
	}

	if(wdev->dev_type == WAPP_DEV_TYPE_AP) {
		ap = (struct ap_dev *)wdev->p_dev;
		for(i=0;i<16;i++) {
            chn_list[i].channel = ap->ch_info.ch_list[i].channel;
            chn_list[i].pref = ap->ch_info.ch_list[i].pref;
			chn_list[i].cac_timer = ap->ch_info.ch_list[i].cac_timer;
		}
	}
	wapp_send_1905_msg(wapp,
	WAPP_CH_LIST_DFS_INFO, sizeof(struct chnList)*16,(char *)&chn_list);
}
#ifdef MAP_R2
int map_send_sta_disassoc_stats_msg(
	struct wifi_app *wapp, unsigned char *bss_addr, wapp_client_info *cli_info, char *evt_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);

	send_pkt_len = map_build_disassoc_stats(wapp, bss_addr, cli_info, evt_buf);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size <= 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}

	if (0 > map_1905_send(wapp, evt_buf, send_pkt_len)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): send assoc client msg fail\n", __func__, __LINE__);
		return MAP_ERROR;
	}

	return MAP_SUCCESS;
}
int map_traffic_separarion_setting_msg(
	struct wifi_app *wapp, char *msg_buf);
int map_service_prioritization_rule_msg(
	struct wifi_app *wapp, char *msg_buf, unsigned short msg_len);
int map_service_prioritization_dscp_tbl_msg(
	struct wifi_app *wapp, char *msg_buf, unsigned short msg_len);
#endif
int map_send_wireless_inf_info(
	struct wifi_app *wapp, unsigned char write_to_conf, char send_to_1905)
{
	struct wapp_dev *wdev;
	struct dl_list *dev_list = &wapp->dev_list;
	struct interface_info_list_hdr *inf_list_hdr = NULL;
	struct interface_info *info = NULL;
	char *cmd_buf = NULL, *pos = NULL;
	FILE *file;
	int cmd_buf_len = 0, inf_count = 0;
	char all_wintf_valid = 1;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev->dev_type == WAPP_DEV_TYPE_AP ||
			wdev->dev_type == WAPP_DEV_TYPE_STA ||
			wdev->dev_type == WAPP_DEV_TYPE_APCLI) {
			if (!wdev->valid) {
				all_wintf_valid = 0;
			}
			inf_count++;
		}
	}

	if (send_to_1905) {
		if (!all_wintf_valid) {
			cmd_buf_len = 2;
			cmd_buf = os_zalloc(cmd_buf_len);
			if (!cmd_buf) {
				DBGPRINT(RT_DEBUG_ERROR, "%s alloc cmd_buf fail\n", __func__);
				return WAPP_RESOURCE_ALLOC_FAIL;
			}
			cmd_buf[0] = all_wintf_valid;
			cmd_buf[1] = send_to_1905;
			wapp_send_1905_msg(wapp, WAPP_WIRELESS_INF_INFO, cmd_buf_len, cmd_buf);
			os_free(cmd_buf);
			return MAP_SUCCESS;
		}
	}

	if (inf_count > MAX_SUPPORT_INF_NUM)
		inf_count = MAX_SUPPORT_INF_NUM;
	cmd_buf_len = 2 + sizeof(struct interface_info_list_hdr) +
		inf_count * sizeof(struct interface_info);
	cmd_buf = os_zalloc(cmd_buf_len);
	if (!cmd_buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s alloc cmd_buf fail\n", __func__);
		return WAPP_RESOURCE_ALLOC_FAIL;
	}
	pos = cmd_buf;
	*pos++ = all_wintf_valid;
	*pos++ = send_to_1905;
	*pos++ = inf_count;
	info = (struct interface_info *) pos;
	printf("[WAPP]map_send_wirelss_inf_info\n");
	/* report all active wireless inf (radio->op_ch!=0) */
	inf_count = 0;
	if (!dl_list_empty(dev_list)) {
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
			if (wdev->radio && wdev->radio->op_ch != 0
				&& (wdev->dev_type == WAPP_DEV_TYPE_AP ||
				wdev->dev_type == WAPP_DEV_TYPE_STA ||
				wdev->dev_type == WAPP_DEV_TYPE_APCLI)) {
				info->if_ch = wdev->radio->op_ch;
				os_memcpy(info->if_name, wdev->ifname, IFNAMSIZ);

				if (wdev->dev_type == WAPP_DEV_TYPE_STA)
					os_strncpy((char *)info->if_role,"wista",sizeof(info->if_role));
				else
					os_strncpy((char *)info->if_role,"wiap",sizeof(info->if_role));

				printf("send info %s, role: %s, dev type = %d\n",
					wdev->ifname, info->if_role, wdev->dev_type);
				COPY_MAC_ADDR(info->if_mac_addr,wdev->mac_addr);
				printf("MAC Address = "MACSTR"\n",
					MAC2STR(info->if_mac_addr));
				info->if_ch = wdev->radio->op_ch;
#ifdef MAP_6E_SUPPORT
				/* ToDo: Need to add and parse in 1905 as well, as we send this info to 1905daemon */
				info->if_opclass = wdev->radio->opclass;
#endif
				if (WMODE_CAP_AX(wdev->wireless_mode))
					os_strncpy((char *)info->if_phymode,"AX",sizeof(info->if_phymode));
				else if (WMODE_CAP_AC(wdev->wireless_mode))
					os_strncpy((char *)info->if_phymode,"AC",sizeof(info->if_phymode));
				else if (WMODE_CAP_N(wdev->wireless_mode))
					os_strncpy((char *)info->if_phymode,"N",sizeof(info->if_phymode));
#ifndef MAP_6E_SUPPORT
				else if(info->if_ch > 14)
					os_strncpy((char *)info->if_phymode,"A",sizeof(info->if_phymode));
				else
					os_strncpy((char *)info->if_phymode,"B",sizeof(info->if_phymode));
#else
				else if ((IS_MAP_CH_5G(info->if_ch) || IS_MAP_CH_6G(info->if_ch)) &&
					(IS_OP_CLASS_5G(info->if_opclass) || IS_OP_CLASS_6G(info->if_opclass)))
					os_strncpy((char *)info->if_phymode, "A", sizeof(info->if_phymode));
				else if (IS_MAP_CH_24G(info->if_ch) && IS_OP_CLASS_24G(info->if_opclass))
					os_strncpy((char *)info->if_phymode, "B", sizeof(info->if_phymode));
#endif
				MAP_GET_RADIO_IDNFER(wdev->radio, info->identifier);
				info++;
				inf_count++;
				if (inf_count == MAX_SUPPORT_INF_NUM)
					break;
			}
		}
	}

	inf_list_hdr = (struct interface_info_list_hdr *)(cmd_buf + 2);
	if (write_to_conf) {
		file = fopen(MAP_WIFI_INFO_FILE, "w");
		if (!file) {
			printf("\033[1;31m %s, %u \033[0m\n", __FUNCTION__, __LINE__);
			DBGPRINT(RT_DEBUG_ERROR, "open MAP cfg file (%s) fail\n", MAP_WIFI_INFO_FILE);
			os_free(cmd_buf);
			return WAPP_UNEXP;
		}
		printf("\033[1;32m  inf_count = %d \033[0m\n", inf_list_hdr->interface_count);
		fprintf(file, "inf_count=%d\n",inf_list_hdr->interface_count);
		info = (struct interface_info *)inf_list_hdr->if_info;
		for (inf_count = 0; inf_count < inf_list_hdr->interface_count; inf_count++) {
			printf("\033[1;32m %d name %s ch %d, phymode [%s] role [%s]\033[0m\n",
				(inf_count + 1), info->if_name, info->if_ch, info->if_phymode, info->if_role);  /* Kyle Debug Print (G) */
			fprintf(file, "interface=%s;%s;%s;%s;%02x:%02x:%02x:%02x:%02x:%02x;%02x:%02x:%02x:%02x:%02x:%02x;\n",
				info->if_name, info->if_role, (info->if_ch>14)?"5g":"2.4g",
				info->if_phymode,PRINT_MAC(info->if_mac_addr), PRINT_MAC(info->identifier));
			info++;
		}
		fclose(file);
	} else {
		wapp_send_1905_msg(wapp, WAPP_WIRELESS_INF_INFO, cmd_buf_len, cmd_buf);
	}

	os_free(cmd_buf);

	return MAP_SUCCESS;
}

int map_receive_addtional_bh_assoc_msg(struct wifi_app *wapp, char *msg_buf)
{
	struct bh_assoc_wireless_setting *bh_setting = (struct bh_assoc_wireless_setting *)msg_buf;
	struct wireless_setting setting;
	struct sec_info sec;
	struct wapp_dev *wdev = NULL;
	char cmd[1024] = {0};
	char dbg_log[1024] = {0};
	int res;
#if NL80211_SUPPORT
	u8 Enable = 0;
#endif /* NL80211_SUPPORT */

	if (!wapp || !msg_buf)
		return MAP_ERROR;

	setting.AuthMode = bh_setting->auth_mode;
	setting.EncrypType = bh_setting->encryp_type;
	memcpy(setting.WPAKey, bh_setting->wpa_key,sizeof(setting.WPAKey));
	memcpy(setting.mac_addr, bh_setting->bh_mac_addr, MAC_ADDR_LEN);
	if (MAP_SUCCESS != fill_sec_info(&sec, &setting)) {
		DBGPRINT(RT_DEBUG_ERROR, "%s fill_sec_info error\n", __func__);
		return MAP_ERROR;
	}

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bh_setting->bh_mac_addr, WAPP_DEV_TYPE_STA);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s bh sta "MACSTR" not found\n", __func__, MAC2STR(bh_setting->bh_mac_addr));
		return MAP_ERROR;
	}
/*
	iwpriv apcli0 set ApCliEnable=0
	iwpriv apcli0 set ApCliAuthMode=WPA2PSK
	iwpriv apcli0 set ApCliEncrypType=AES
	iwpriv apcli0 set ApCliWPAPSK=12345678
	iwpriv apcli0 set ApCliBssid=02:00:11:22:33:88
	iwpriv apcli0 set ApCliSsid=12k
	iwpriv apcli0 set ApCliEnable=1
*/
	if (bh_setting->target_channel != 0) {
		int ret = 0;
#if NL80211_SUPPORT
		if(wapp->map->quick_ch_change)
			wapp_set_map_channel(wapp, (const char *)wdev->ifname,
					(char *)&bh_setting->target_channel,
					(size_t)sizeof(bh_setting->target_channel));
		else
			wapp_set_channel(wapp, (const char *)wdev->ifname,
					(char *)&bh_setting->target_channel,
					(size_t)sizeof(bh_setting->target_channel));
#else
		if (wapp->map->quick_ch_change) {
			res = snprintf(cmd, sizeof(cmd), "iwpriv %s set MapChannel=%d;", wdev->ifname, bh_setting->target_channel);
			if (os_snprintf_error(sizeof(cmd), res))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		} else {
			res = snprintf(cmd, sizeof(cmd), "iwpriv %s set Channel=%d;", wdev->ifname, bh_setting->target_channel);
			if (os_snprintf_error(sizeof(cmd), res))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		}
		if (system(cmd) == -1)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif /* NL80211_SUPPORT */

		os_memset(cmd,0,sizeof(cmd));
		res = snprintf(cmd, sizeof(cmd), "wifi_config_save %s Channel %d", wdev->ifname, bh_setting->target_channel);
		if (os_snprintf_error(sizeof(cmd), res))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

		ret = system(cmd);
		if (ret == -1)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
		DBGPRINT(RT_DEBUG_OFF," [%s] Send Channel  Command: %s, ret = %d\n", __func__, cmd, ret);
	}
#if NL80211_SUPPORT
	wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
			(char *)&Enable, 1);
	wapp_set_authtype(wapp, (const char *)wdev->ifname,
			(char *)sec.auth,
			(size_t)strlen(sec.auth));
	wapp_set_apcli_EncrypType(wapp, (const char *)wdev->ifname,
			(char*)sec.encryp,
			strlen(sec.encryp));
#else
	res = snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliEnable=0;iwpriv %s set ApCliAuthMode=%s;iwpriv %s set ApCliEncrypType=%s;",
		wdev->ifname,
		wdev->ifname,
		sec.auth,
		wdev->ifname,
		sec.encryp);
	if (os_snprintf_error(sizeof(cmd), res))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif
	driver_wext_set_psk(wapp->drv_data, wdev->ifname, sec.psphr);
	res = snprintf(dbg_log, sizeof(dbg_log), "echo \"%s\" > /tmp/bh_assoc_cmd_dbg", cmd);
	if (os_snprintf_error(sizeof(dbg_log), res))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

	if (system(dbg_log) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
	os_memset(cmd,0,sizeof(cmd));
	os_memset(dbg_log,0,sizeof(dbg_log));
	driver_wext_set_ssid(wapp->drv_data, wdev->ifname, (char *)bh_setting->target_ssid);
#if NL80211_SUPPORT
	Enable = 1;
	wapp_set_bssid(wapp, (const char *)wdev->ifname,
			(char *)bh_setting->target_bssid, MAC_ADDR_LEN);
	wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
			(char *)&Enable, 1);
#else
	res = snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliBssid=%02x:%02x:%02x:%02x:%02x:%02x;iwpriv %s set ApCliEnable=1;",
		wdev->ifname,
		bh_setting->target_bssid[0],bh_setting->target_bssid[1],bh_setting->target_bssid[2],
		bh_setting->target_bssid[3],bh_setting->target_bssid[4],bh_setting->target_bssid[5],
		wdev->ifname);
	if (os_snprintf_error(sizeof(cmd), res))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif
	res = snprintf(dbg_log, sizeof(dbg_log), "echo \"%s\" >> /tmp/bh_assoc_cmd_dbg", cmd);
	if (os_snprintf_error(sizeof(dbg_log), res))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

	if (system(dbg_log) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
	os_memset(cmd,0,sizeof(cmd));
	os_memset(dbg_log,0,sizeof(dbg_log));
	if (bh_setting->target_channel == 0) {
#if NL80211_SUPPORT
		Enable = 1;
		wapp_set_apcli_AutoConnect(wapp, (const char *)wdev->ifname,
				(char *)&Enable, 1);
#else
		res = snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliAutoConnect=1;", wdev->ifname);
		if (os_snprintf_error(sizeof(cmd), res))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

		if (system(cmd) == -1)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif
	}
	return MAP_SUCCESS;

}
void read_backhaul_configs(struct wifi_app *wapp);
void check_redio_conf_status(struct wifi_app *wapp)
{
	struct map_conf_state *conf_state;
	unsigned int i = 0;
	unsigned char conf_flag = 0;
#ifdef MAP_R3
	struct dpp_authentication *auth = NULL;
#endif /* MAP_R3 */
	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		conf_state = &wapp->radio[i].conf_state;
		if ((wapp->radio[i].adpt_id) && (IS_CONF_STATE(conf_state, MAP_CONF_UNCONF)
			|| IS_CONF_STATE(conf_state, MAP_CONF_WAIT_RSP))) {
			conf_flag = 1;
		}
	}
	if(conf_flag != 1) {
		wapp_device_status *device_status = &wapp->map->device_status;
		eloop_cancel_timeout(map_wps_timeout, wapp, device_status);
		wapp->map->conf = MAP_CONN_STATUS_CONF;
		device_status->status_fhbss = STATUS_FHBSS_CONFIGURED;
		device_status->status_bhsta = STATUS_BHSTA_CONFIGURED;
		wapp_send_1905_msg(
			wapp,
			WAPP_DEVICE_STATUS,
			sizeof(wapp_device_status),
			(char *)device_status);
		DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"auto config done!!!\n");
#ifdef MAP_R3
		if (wapp->map && (wapp->map->map_version == DEV_TYPE_R3)) {
			wapp->map->radio_conf_count=0;
			if(wapp->dpp && (wapp->dpp->onboarding_type == 0)) {
				/* Clearing first self auth after BSS configured */
				if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_CONFIGURATOR)
					auth = wapp_dpp_get_own_auth_cont(wapp);
				else
					auth = wapp_dpp_get_first_auth(wapp);

				if(!auth) {
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s auth instance not found\n", __func__);
				}
				else
					dpp_auth_deinit(auth);

				if(wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE) {
					if(wapp->dpp->map_sec_done) {
						/* Changing role to proxy agent when agent onboarded
						 * on bootup without DPP handshakes */
						DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"Changing the role to proxy agnt\n");
						wapp->dpp->dpp_allowed_roles = DPP_CAPAB_PROXYAGENT;
					}else {
						if(wapp->map->bh_type != MAP_BH_ETH) {
							/* If the R3 wifi device onboarded through R1/R2 and
							 * onbording method is DPP then start chirping */
							wapp->dpp->wsc_onboard_done = 1;
							DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX "R3 device onboarded through R1/R2 so start chirping..\n");
							wapp->dpp->annouce_enrolle.is_enable = 1;
							wapp_dpp_presence_annouce(wapp, NULL);
						}
					}
				}
			}
		}
#endif /* MAP_R3 */
		eloop_cancel_timeout(map_config_state_check,wapp,NULL);
	}

}
int map_set_wapp_avoid_scan_during_CAC(struct wifi_app *wapp, char enable)
{
	struct wapp_dev *wdev = NULL;
	int ret = MAP_SUCCESS;
	unsigned char count = 0;
	char ra_identifier[MAC_ADDR_LEN];
	struct wapp_radio *ra;
	for (count = 0; count < MAX_NUM_OF_RADIO; count++)
	{
		ra = &wapp->radio[count];
		if(ra->adpt_id) {
			MAP_GET_RADIO_IDNFER(ra, ra_identifier);
			wdev = wapp_dev_list_lookup_by_radio(wapp, ra_identifier);
			if (!wdev) {
				DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
				return MAP_ERROR;
			}
			ret = wapp_set_AvoidScanDuringCAC(wapp,wdev, enable);
		}
	}
	return ret;
}
#ifdef MAP_R2
int map_transparent_vlan_setting_msg(
	struct wifi_app *wapp, char *msg_buf);
#endif
int map_handler(struct wifi_app *wapp, char *buffer_recv, int len_recv, char* event_buf, int* len_event)
{
	int ret = MAP_SUCCESS;
	struct msg_1905 *map_msg = (struct msg_1905 *)buffer_recv;
	wapp_device_status *device_status = NULL;
	int i = 0;
	int va_num = 0;
	int buf_size = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s receive %s msg(0x%04x)\n", __func__, MapMsgTypeToString(map_msg->type), map_msg->type);
	switch (map_msg->type) {
		case WAPP_USER_SET_CHANNEL_SETTING:
			ret = map_config_channel_setting_msg(wapp, map_msg->body, event_buf, len_event);
			break;
		case WAPP_USER_SET_WIRELESS_SETTING:
			{
				struct map_radio_identifier ra_identifier;
				os_memset(&ra_identifier, 0, sizeof(ra_identifier));

				DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"received WAPP_USER_SET_WIRELESS_SETTING!!!\n");
				ret = map_config_wireless_setting_msg(wapp, map_msg->body, &ra_identifier, map_msg->role);
				DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"send WAPP_OPERBSS_REPORT to notify mapd get operating channel info;"
					"radio capability info(ht/vht/channel preference/...)\n");
				ret = map_send_ap_oper_bss_msg(wapp,
					(unsigned char *)&ra_identifier,
					event_buf, len_event);
				check_redio_conf_status(wapp);
				break;
			}
		case WAPP_USER_SET_BEACON_METRICS_QRY:
			ret = map_config_beacon_metrics_query_msg(wapp, map_msg->body,  map_msg->bssAddr);
			break;
		case WAPP_USER_SET_STEERING_SETTING:
			ret = map_config_steering_setting_msg(wapp, map_msg->body, event_buf, len_event);
			break;
		case WAPP_USER_SET_LOCAL_STEER_DISALLOW_STA:
			ret = map_config_local_steer_disallow_sta_msg(wapp, map_msg->bssAddr, map_msg->body);
			break;
		case WAPP_USER_SET_BTM_STEER_DISALLOW_STA:
			ret = map_config_btm_steer_disallow_sta_msg(wapp, map_msg->bssAddr, map_msg->body);
			break;
		case WAPP_USER_SET_RADIO_CONTROL_POLICY:
			ret = map_config_radio_control_policy_msg(wapp, map_msg->bssAddr, map_msg->body);
			break;
		case WAPP_USER_SET_ASSOC_CNTRL_SETTING:
		case WAPP_USER_SET_WHPROBE_REQ:
			ret = map_config_client_assoc_control_msg(wapp, map_msg->bssAddr, map_msg->body);
			break;
#ifdef ACL_CTRL
		case WAPP_USER_SET_ACL_CNTRL_SETTING:
			ret = map_config_acl_control_msg(wapp, map_msg->bssAddr, map_msg->body);
			break;
#endif
		case WAPP_USER_SET_BACKHAUL_STEER:
			ret = map_config_backhaul_steering_msg(wapp, map_msg->body, event_buf, len_event);
			break;
		case WAPP_USER_GET_RADIO_BASIC_CAP:
			ret = map_send_radio_basic_capability_msg(wapp, map_msg->bssAddr, event_buf, len_event);
			break;
		case WAPP_USER_GET_AP_CAPABILITY:
			ret = map_send_ap_capability_msg(wapp, map_msg->bssAddr, event_buf, len_event);
			break;
		case WAPP_USER_GET_AP_HT_CAPABILITY:
			ret = map_send_ap_ht_capability_msg(wapp, map_msg->bssAddr, event_buf, len_event);
			break;
		case WAPP_USER_GET_AP_HE_CAPABILITY:
			ret = map_send_ap_he_capability_msg(wapp, map_msg->bssAddr, event_buf, len_event);
			break;
#ifdef MAP_R3_DE
		case WAPP_USER_GET_DEV_INVEN_TLV:
			ret = map_send_dev_inven_tlv(wapp, map_msg->bssAddr, event_buf, len_event);
			break;
#endif //MAP_R3_DE
		case WAPP_USER_GET_AP_VHT_CAPABILITY:
			ret = map_send_ap_vht_capability_msg(wapp, map_msg->bssAddr, event_buf, len_event);
			break;
		case WAPP_USER_GET_RA_OP_RESTRICTION:
			ret = map_send_radio_op_restrict_msg(wapp, map_msg->bssAddr, event_buf, len_event);
            map_send_ch_list_dfs_info(wapp, map_msg->bssAddr);
            break;
		case WAPP_USER_GET_CHANNEL_PREFERENCE:
			ret = map_send_chn_pref_msg(wapp, map_msg->bssAddr, event_buf, len_event);
			break;
		case WAPP_USER_GET_OPERATIONAL_BSS:
			ret = map_send_ap_oper_bss_msg(wapp, map_msg->bssAddr, event_buf, len_event);
			break;
		case WAPP_USER_GET_AP_METRICS_INFO:
			ret = map_recieve_ap_metric_msg(wapp, map_msg->bssAddr);
			break;
#ifdef MAP_R2
		case WAPP_USER_GET_RADIO_METRICS_INFO:
			ret = map_recieve_radio_metric_msg(wapp, map_msg->body);
			break;
#endif
		case WAPP_USER_GET_ASSOC_STA_TRAFFIC_STATS:
		case WAPP_USER_GET_ASSOC_STA_LINK_METRICS:
		case WAPP_USER_GET_ALL_ASSOC_TP_METRICS:
#ifdef MAP_R2
		case WAPP_USER_GET_ASSOC_STA_EXTENDED_LINK_METRICS:
#endif
#ifdef MAP_R3_WF6
		case WAPP_USER_GET_ASSOC_WIFI6_STA_STATUS:
#endif
			prev_1905_msg = map_msg->type;
			os_memcpy(prev_req_radio_id, map_msg->body, ETH_ALEN);
			ret = map_recieve_assoc_sta_msg(wapp, map_msg->body);
			break;
#ifdef MAP_R3_WF6
		case WAPP_USER_GET_AP_WF6_CAPABILITY:
			ret = map_send_ap_wf6_capability_msg(wapp, map_msg->bssAddr, event_buf, len_event);
			break;
#endif /*MAP_R3_WF6*/
#ifdef MAP_R4_SPT
		case WAPP_USER_GET_AP_SPT_REUSE:
			ret = map_send_ap_spt_reuse_req_msg(wapp, map_msg->bssAddr, event_buf, len_event);
			break;
		case WAPP_USER_SET_AP_SPT_REUSE:
			ret = mapd_recieve_spt_reuse_req(wapp, map_msg->body, event_buf, len_event);
			break;
		case WAPP_USER_SET_AP_SR_BM:
			ret = mapd_receive_operating_sr_rpt(wapp, map_msg->body, event_buf, len_event);
			break;
		case WAPP_USER_SET_SR_TOPO_INFO:
			ret = mapd_receive_sr_topo_info(wapp, map_msg->body, event_buf, len_event);
			break;
		case WAPP_USER_SET_UPLINK_TRAFFIC_STATUS:
			ret = mapd_receive_uplink_traffic_status(wapp, map_msg->body, event_buf, len_event);
			break;
#endif
		case WAPP_USER_GET_ONE_ASSOC_STA_TRAFFIC_STATS:
		case WAPP_USER_GET_ONE_ASSOC_STA_LINK_METRICS:
#ifdef MAP_R2
		case WAPP_USER_GET_ONE_ASSOC_STA_EXTENDED_LINK_METRICS:
#endif
			ret = map_recieve_one_assoc_sta_msg(wapp, map_msg->staAddr);
			break;
		case WAPP_USER_GET_TX_LINK_STATISTICS:
		case WAPP_USER_GET_RX_LINK_STATISTICS:
			ret = map_recieve_txrx_link_stats_msg(wapp, map_msg->body, map_msg->type, map_msg->role, event_buf, len_event);
			break;
		case WAPP_USER_GET_UNASSOC_STA_LINK_METRICS:
			ret = map_send_unassoc_sta_link_metrics_msg(wapp, map_msg->body, event_buf, len_event);
			break;
		case WAPP_USER_SET_METIRCS_POLICY:
			ret = map_config_metrics_policy_msg(wapp, map_msg->body);
			break;
#ifdef MAP_R2
		case WAPP_USER_SET_UNSUCCESSFUL_ASSOC_POLICY:
			ret = map_config_unsuccessful_assoc_policy_msg(wapp, map_msg->body);
			break;
#endif
		case WAPP_USER_MAP_CONTROLLER_FOUND:
			device_status = &wapp->map->device_status;
			wapp->map->ctrler_found = 1;
			DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"[WAPP_USER_MAP_CONTROLLER_FOUND]ctrler_found\n");
			if (device_status->status_bhsta != STATUS_BHSTA_CONFIGURED && wapp->map->bh_type == MAP_BH_ETH) {
				device_status->status_bhsta = STATUS_BHSTA_BH_READY;
				device_status->status_fhbss = STATUS_FHBSS_UNCONFIGURED;
				wapp_send_1905_msg(
					wapp,
					WAPP_DEVICE_STATUS,
					sizeof(wapp_device_status),
					(char *)device_status);
			}
			// start timer to check config status
			for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
				if (wapp->radio[i].adpt_id)
					va_num++;
			}
			eloop_cancel_timeout(map_config_state_check,wapp,NULL);
			if(va_num)
				eloop_register_timeout(va_num*MAP_CONF_PER_RADIO_TIMEOUT, 0, map_config_state_check, wapp, NULL);
			break;
		case WAPP_USER_SET_RADIO_TEARED_DOWN:
			{
				char radio_identifier[ETH_ALEN];
				memcpy(radio_identifier, map_msg->body, ETH_ALEN);
				ret = map_radio_tear_down(wapp, radio_identifier);
				check_redio_conf_status(wapp);
			}
			break;
		case WAPP_USER_SET_RADIO_RENEW:
			{
				int i = 0;
				u8  band = 0;
				struct wapp_radio *ra = NULL;

				band = (u8) map_msg->body[0];

				printf(BLUE("renew band = %u\n"), band);
				for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
					struct map_conf_state *conf_stat = NULL;
					ra = &wapp->radio[i];
					conf_stat = &ra->conf_state;
					wapp_reset_backhaul_config(wapp, NULL);
					if ((band == BAND_24G && ra->op_ch <= 14) ||
						(band == BAND_5G && ra->op_ch > 14)
#ifdef MAP_6E_SUPPORT
							|| (band == BAND_6G && IS_MAP_CH_6G(ra->op_ch))
#endif
						) {
						printf(BLUE("renew ra index = %u op_ch = %d\n"),
							ra->index, ra->op_ch);
						MAP_CONF_STATE_SET(conf_stat, MAP_CONF_UNCONF);
					}
				}
			}
			break;
		case WAPP_USER_GET_OPERATING_CHANNEL_INFO:
			map_operating_channel_info(wapp);
			break;
		case WAPP_USER_FLUSH_ACL:
			map_receive_flush_acl_msg(wapp, map_msg->bssAddr);
			break;
		case WAPP_USER_GET_BSSLOAD:
			map_receive_bssload_query_msg(wapp, map_msg->bssAddr, map_msg->body);
			break;
		case WAPP_USER_GET_RSSI_REQ:
			map_receive_rssi_query_msg(wapp, map_msg->bssAddr, map_msg->staAddr);
			break;
		case WAPP_USER_SET_NAC_REQ:
			map_receive_nac_query_msg(wapp, map_msg->bssAddr, map_msg->staAddr, map_msg->body);
			break;
#ifdef MAP_SUPPORT
		case WAPP_USER_SET_AIR_MONITOR_REQUEST:
			wapp->air_mnt_max_pkt = MONITOR_PACKET_COUNT;
			map_receive_air_monitor_request(wapp, map_msg->bssAddr, map_msg->staAddr, map_msg->body);
			break;
#endif
		case WAPP_USER_SET_DEAUTH_STA:
			map_receive_deauth_sta_msg(wapp, map_msg->staAddr, map_msg->bssAddr);
			break;
		case WAPP_USER_ISSUE_APCLI_DISCONNECT:
			//printf(" WAPP cmd received\n");
			map_receive_disconnect_apcli_msg(wapp, map_msg, len_recv);
		    break;
		case WAPP_USER_SET_VENDOR_IE:
			map_receive_set_vendor_ie(wapp, map_msg->bssAddr, map_msg->body);
			break;
		case WAPP_USER_GET_APCLI_RSSI_REQ:
			map_receive_apcli_rssi_query_msg(wapp, map_msg->bssAddr, map_msg->staAddr);
			break;
		case WAPP_USER_GET_WIRELESS_INF_INFO:
			map_send_wireless_inf_info(wapp, FALSE, *map_msg->body);
			break;
		case WAPP_USER_SET_ADDITIONAL_BH_ASSOC:
			map_receive_addtional_bh_assoc_msg(wapp, map_msg->body);
			break;
		case WAPP_USER_ISSUE_SCAN_REQ:
			map_receive_scan_request(wapp, map_msg->bssAddr, map_msg->body);
			break;
		case WAPP_USER_SEND_NULL_FRAMES:
			map_receive_null_frame_req(wapp, map_msg->bssAddr, map_msg->staAddr, map_msg->body);
			break;
		case WAPP_USER_SET_ENROLLEE_BH:
			map_set_enrollee_bh(wapp, map_msg->bssAddr, map_msg->staAddr, map_msg->body, event_buf, len_event);
			break;
		case WAPP_USER_SET_BSS_ROLE:
			map_set_bss_role(wapp, map_msg->bssAddr, map_msg->staAddr, map_msg->body, event_buf, len_event);
			break;
		case WAPP_USER_TRIGGER_WPS:
			map_trigger_wps(wapp, map_msg->bssAddr, map_msg->staAddr, map_msg->body, event_buf, len_event);
			break;
		case WAPP_USER_SET_BH_WIRELESS_SETTING:
			ret = map_config_bh_wireless_setting_msg(wapp, map_msg->body);
			break;
		case WAPP_USER_SET_BH_PRIORITY:
			ret = map_config_bh_priority(wapp, map_msg->body, event_buf, len_event);
			break;
		case WAPP_USER_SET_BH_CONNECT_REQUEST:
			map_reset_conf_sm(wapp->map);
			ret = map_config_backhaul_connect_msg(wapp, map_msg->body, event_buf, len_event);
			break;
		case WAPP_USER_GET_BH_WIRELESS_SETTING:
			read_backhaul_configs(wapp);
#ifdef MAP_R3
			if (wapp->map && (wapp->map->map_version == DEV_TYPE_R3)
				&& (wapp->map->TurnKeyEnable == 1))
				dpp_read_config_file_for_connection(wapp);
#endif
			ret = MAP_SUCCESS;
			break;
		case WAPP_USER_SET_ETH_BH_DOWN:
			map_update_bh_link_info(wapp, MAP_BH_ETH, 0, 0);
			if (!wapp->map->bh_link_num) {
				DBGPRINT(RT_DEBUG_ERROR, "no exist bh; set bh_link_ready&ctrler_found 0\n");
				wapp->map->bh_link_ready = 0;
				wapp->map->ctrler_found = 0;
			}
			break;
		case WAPP_USER_SET_DHCP_CTL_REQUEST:
			map_handle_dhcp_ctl_request(wapp, (struct dhcp_ctl_req *)map_msg->body);
			break;
		case WAPP_USER_GET_BRIDGE_IP_REQUEST:
			map_handle_get_br_ip_request(wapp);
			break;
		case WAPP_USER_SET_BRIDGE_DEFAULT_IP_REQUEST:
			map_handle_set_br_default_ip_request(wapp);
			break;
		case WAPP_USER_SET_TX_POWER_PERCENTAGE:
			map_config_tx_power_percentage_msg(wapp, map_msg->body, event_buf, len_event);
			break;
		case WAPP_USER_SET_OFF_CH_SCAN_REQ:
		case WAPP_USER_SET_NET_OPT_SCAN_REQ:
			map_prepare_off_channel_scan_req(wapp,
				map_msg->body, map_msg->type);
			break;
#ifdef MAP_SUPPORT
		case WAPP_USER_SET_GARP_REQUEST:
			map_handle_garp_request(wapp, (struct garp_req_s *)map_msg->body);
			break;
#ifdef AUTOROLE_NEGO
		case WAPP_NEGOTIATE_ROLE:
			map_prepare_rawpacket(wapp,map_msg->body[0]);
			break;
#endif //AUTOROLE_NEGO
		case WAPP_UPDATE_MAP_DEVICE_ROLE:
			wapp->map->my_map_dev_role=map_msg->body[0];
#ifdef MAP_R3
			char value[10] = {0};
			if (wapp->map->map_version == DEV_TYPE_R3 && wapp->map->TurnKeyEnable) {
				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"wapp: got WAPP_UPDATE_MAP_DEVICE_ROLE\n");
#if 0
				get_dpp_parameters(wapp->map, "allowed_role", value, sizeof(value));
				if ((wapp->map->my_map_dev_role == DEVICE_ROLE_CONTROLLER &&
					atoi(value) != 2) || (wapp->map->my_map_dev_role == DEVICE_ROLE_AGENT &&
					atoi(value) != 1))
					map_reset_default(wapp); // To reset default or to to just reset profile params
#endif
				if (wapp->dpp && (wapp->map->my_map_dev_role == DEVICE_ROLE_CONTROLLER &&
							wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE)) {
					/* For R3 the Enrollee device which is not onboarded yet got an    *
					 * indication to change role to controller so change dpp role here */
					if(wapp->dpp->chirp_ongoing) {
						/* Stop chirping if the chirp is ongoing */
						DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"stop chirping in DPP enrollee mode\n");
						wapp_dpp_presence_announce_stop(wapp);
						wapp->dpp->dpp_allowed_roles = 0;
					}
					if(wapp->dpp->dpp_onboard_ongoing) {
						struct dpp_authentication *auth = NULL;
						/* If dpp onboard ongoing and ETH link connected
						 * clear the existing wifi state and continue with ETH */
						DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"clear auth state in DPP enrollee mode\n");
						wapp->dpp->dpp_onboard_ongoing = 0;
						wapp->dpp->dpp_wifi_onboard_ongoing = FALSE;
						auth = wapp_dpp_get_first_auth(wapp);
						if(!auth) {
							DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s auth instance not found\n", __func__);
						}
						else {
							wapp_dpp_cancel_timeouts(wapp,auth);
							eloop_cancel_timeout(wapp_dpp_conn_result_timeout, wapp,auth);
							dpp_auth_deinit(auth);
						}
						wapp->dpp->dpp_allowed_roles = 0;
					}
				}
				if(wapp->dpp && (wapp->dpp->dpp_allowed_roles == 0)) {
					if (wapp->map->my_map_dev_role == DEVICE_ROLE_CONTROLLER)
						dpp_save_config(wapp, "allowed_role", "2", NULL);
					if (wapp->map->my_map_dev_role == DEVICE_ROLE_AGENT)
						dpp_save_config(wapp, "allowed_role", "1", NULL);
				}
				else {
					if (wapp->dpp) {
						DBGPRINT(RT_DEBUG_OFF,
						DPP_MAP_PREX "wapp: DPP already init with Role:%d\n", wapp->dpp->dpp_allowed_roles);
					} else
						DBGPRINT(RT_DEBUG_OFF,
						DPP_MAP_PREX "wapp: wapp->dpp NULL");
				}

				get_dpp_parameters(wapp->map, "allowed_role", value, sizeof(value));
				if(wapp->dpp && (wapp->dpp->dpp_allowed_roles == 0) && (atoi(value) != 0)) {
					wapp_dpp_config_init(wapp);
					get_map_parameters(wapp->map, "DppOnboardType", value, NON_DRIVER_PARAM, sizeof(value));
					if (!strcmp(value, "1"))
						wapp->dpp->onboarding_type = atoi(value);
					else
						wapp->dpp->onboarding_type = 0;
					wapp_dpp_init_notify_handler(wapp);
					/* Modify the value in dpp_cfg to zero again for bootup */
					dpp_save_config(wapp, "allowed_role", "0", NULL);
				}
			}
#endif
			break;
#endif
		case WAPP_USER_SET_AVOID_SCAN_CAC:
			ret = map_set_wapp_avoid_scan_during_CAC(wapp,map_msg->body[0]);
			break;
#if defined(MAP_SUPPORT) && defined(MAP_R2)
		case WAPP_USER_SET_CHANNEL_SCAN_REQ:
			//ret = map_receive_channel_scan_req(wapp, map_msg->body, map_msg->length);
			if (wapp->map->off_ch_scan_state.ch_scan_state != CH_SCAN_IDLE)//(wapp->map->f_scan_req == 1)
			{
				//printf("SSS scan is already ongoing , need to abort current scan , then later start new scan after report is sent\n");
				wapp->map->msg_info.enqueue_pending_msg = WAPP_USER_SET_CHANNEL_SCAN_REQ;
				wapp->map->msg_info.msg_body_ptr = os_malloc(map_msg->length);
				if (!wapp->map->msg_info.msg_body_ptr) {
					DBGPRINT(RT_DEBUG_ERROR, "%s %d memory allocation failed\n", __func__, __LINE__);
					return MAP_ERROR;
				}
				wapp->map->msg_info.msg_len = map_msg->length;
				os_memcpy(wapp->map->msg_info.msg_body_ptr,map_msg->body,map_msg->length);
				break;
			}
			wapp->map->msg_info.enqueue_pending_msg = 0;
			wapp->map->msg_info.ignore_req_too_soon = 0;
			wapp->map->f_scan_req =1;
			ret = map_receive_off_channel_scan_req(wapp, map_msg->body, map_msg->length);
			if (wapp->map->off_ch_scan_state.ch_scan_state != CH_SCAN_ONGOING)
				wapp->map->f_scan_req =0;
			break;
		case WAPP_USER_GET_SCAN_CAP:
			ret = map_send_channel_scan_capability_msg(wapp, event_buf, len_event);
			break;
		case WAPP_USER_GET_R2_AP_CAP:
			ret = map_send_r2_ap_capability_msg(wapp, event_buf, len_event);
			break;
#ifdef DFS_CAC_R2
		case WAPP_USER_SET_CAC_REQ:
			printf("in WAPP rx CAC REQ\n");
			ret = map_receive_cac_req(wapp, map_msg->body, map_msg->length);
			break;
		case WAPP_USER_SET_CAC_TERMINATE_REQ:
			ret = map_receive_cac_terminate_req(wapp, map_msg->body, map_msg->length);
			break;
		case WAPP_USER_GET_CAC_CAP:
			ret = map_send_cac_capability_msg(wapp, map_msg->bssAddr, event_buf, len_event);
#if 0
                        if (wapp->dpp && wapp->dpp->is_map && (wapp->dpp->dpp_allowed_roles != DPP_CAPAB_CONFIGURATOR)) {
                                struct dpp_bootstrap_info *bi = NULL;
                                bi = dpp_get_own_bi(wapp->dpp);
				if(bi) {
					DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX "Send chirp tlv\n ");
					ChirpTLV_1905_send(wapp, bi);
				}

                        }
#endif
			break;
		case WAPP_USER_GET_CAC_STATUS:
			ret = map_send_cac_status_msg(wapp, map_msg->bssAddr, event_buf, len_event);
			break;

#endif
		case WAPP_USER_GET_METRIC_REP_INTERVAL_CAP:
			DBGPRINT(RT_DEBUG_OFF, "WAPP_USER_GET_METRIC_REP_INTERVAL_CAP\n");
			ret = map_send_metric_reporting_capability_msg(wapp, map_msg->bssAddr, event_buf, len_event);
			break;
#endif
#ifdef MAP_R2
		case WAPP_USER_SET_TRAFFIC_SEPARATION_SETTING:
			ret = map_traffic_separarion_setting_msg(wapp, map_msg->body);
			break;
		case WAPP_USER_SET_TRANSPARENT_VLAN_SETTING:
			ret = map_transparent_vlan_setting_msg(wapp, map_msg->body);
			break;
		case WAPP_USER_SET_SERVICE_PRIORITIZATION_RULE:
			ret = map_service_prioritization_rule_msg(wapp, map_msg->body, map_msg->length);
			break;
		case WAPP_USER_SET_DSCP_MAPPINT_TABLE:
			ret = map_service_prioritization_dscp_tbl_msg(wapp, map_msg->body, map_msg->length);
			break;
#endif
		case WAPP_USER_GET_WTS_CONFIG:
			ret = map_handle_get_wts_config(wapp, event_buf, len_event);
			break;
#ifdef MAP_R3
		case WAPP_USER_SEND_DPP_FRAME:
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"wapp: got dpp frame \n");
			ret = dpp_parse_1905_frame(wapp, map_msg->body, map_msg->length);
			break;
//Prakhar
		case WAPP_USER_SEND_DPP_CCE_INDICATION_FRAME:
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"wapp: got dpp cce indication frame \n");
			ret = dpp_parse_1905_cce_frame(wapp, map_msg->body, map_msg->length);
			break;
		case WAPP_USER_SEND_AUTOCONFIG_TRIGGER:
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"wapp: got autoconfig trigger \n");
			ret = dpp_parse_autoconfig_frame(wapp, map_msg->body, map_msg->length);
			break;
		case WAPP_USER_SEND_DPP_DIRECT_FRAME:
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"wapp: got direct frame in wapp \n");
			ret = dpp_parse_direct_1905_frame(wapp, map_msg->body, map_msg->length);
			break;
		case WAPP_USER_SEND_CHIRP_TLV_FRAME:
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"wapp: got chirp TLV frame in wapp \n");
			ret = dpp_parse_1905_chirp_msg(wapp, map_msg->body, map_msg->length);
			break;
		case WAPP_USER_SEND_DPP_URI_NOTIFY_FRAME:
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"wapp: got DPP URI frame in wapp \n");
			ret = dpp_parse_1905_dpp_uri_msg(wapp, map_msg->body, map_msg->length);
			break;
		case WAPP_USER_SEND_1905_SEC_NOTIFY_FRAME:
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"wapp: got 1905 sec notify frame \n");
			ret = dpp_parse_1905_sec_notify_frame(wapp, map_msg->body, map_msg->length);
			break;
		case WAPP_USER_SEND_DPP_INIT_NOTIFY_FRAME:
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"wapp: got init notify frame in wapp\n");
			/* Now init is done so perform action as per role
			 * and if R3 version set onboarding type to driver */
			if(wapp->map && (wapp->map->map_version == DEV_TYPE_R3) && wapp->dpp) {
				wapp_dpp_init_notify_handler(wapp);
#if 0
				char onb_type = 0;
				wapp_dpp_set_onboarding_type(wapp, wapp->dpp->onboarding_type);
				onb_type = wapp->dpp->onboarding_type;
				wapp_send_1905_msg(
					wapp,
					WAPP_SEND_ONBOARD_TYPE,
					sizeof(char),
					(char *)&onb_type);
				if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE) {
					/* For enrollee mode if URI present and no config
					 * saved then on bootup start chirping */
					struct dpp_bootstrap_info *bi = NULL;
					bi = dpp_get_own_bi(wapp->dpp);
					if(bi) {
						/* Provide URI to driver if onboarding type is DPP*/
						wapp_dpp_agnt_uri(wapp, bi->uri);
						wapp_dpp_parse_ch_list_agnt_uri(wapp,bi);
						/*if(wapp_dpp_get_conf_frm_file(wapp, "valid")
							&& (wapp->dpp->onboarding_type == 0)) {
							DBGPRINT(RT_DEBUG_OFF, "Starting chirping as onboarding type is DPP on bootup.\n");
							wapp_dpp_presence_annouce(wapp, bi);
						}*/
						DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX "Send chirp tlv\n ");
						ChirpTLV_1905_send(wapp, bi);
					}
				}
				else if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_CONFIGURATOR) {
					/*On bootup generate own QR code if not generated already */
					char mode_str[20] = "";
					struct dpp_bootstrap_info *bi = NULL;
					struct dpp_configurator *conf = NULL;
					bi = dpp_get_own_bi(wapp->dpp);
					if(!bi) {
						DBGPRINT(RT_DEBUG_OFF, "Bootstrap not found on bootup generating key\n");
						os_snprintf(mode_str, sizeof(mode_str), "type=qrcode");
						dpp_bootstrap_gen(wapp->dpp,(const char *)mode_str);
					}
					/* On bootup if configurator add is not done then add it */
					conf = dpp_configurator_get_id(wapp->dpp, 1);
					if (!conf) {
						wapp_dpp_configurator_add(wapp->dpp);
					}

					if(!wapp_dpp_get_conf_frm_file(wapp, "1905valid")) {
						/* For Configurator make self sign in not saved already */
						os_memset(mode_str, 0, 20);
						os_snprintf(mode_str, sizeof(mode_str), " mode=map");
						wapp_dpp_configurator_sign(wapp, NULL, (const char *)mode_str);
					}
					else {
						if (wapp_send_1905_msg(wapp, WAPP_SEND_BSS_CONNECTOR,
									0, NULL) < 0)
							DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed\n");
					}

					/* In controller mode, check for saved QR code, save those in
					 * bootstrap info list for future use */
					wapp_dpp_get_uri_frm_file(wapp);

					/* Reading the 1905 cfg file for updating the cont alid */
					if (wapp->map && (map_read_config_file(wapp->map) != WAPP_SUCCESS))
						DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX "Error in the map read\n");

					/* Enable CCE IE in own BSS  and enable 1905 security flag if onboarding is DPP */
					if(wapp->dpp->onboarding_type == 0) {
						wapp_dpp_set_ie(wapp);
						wapp_dpp_set_1905_sec(wapp, 1);
					}
				}
				else {
					if (wapp_send_1905_msg(wapp, WAPP_SEND_BSS_CONNECTOR,
								0, NULL) < 0)
						DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed\n");
				}
#endif
			}
			else {
				if (wapp_send_1905_msg(wapp, WAPP_SEND_BSS_CONNECTOR,
							0, NULL) < 0)
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed\n");
			}
			break;
		case WAPP_USER_SEND_TOPO_NOTIFY_FRAME:
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"wapp: got topo notify frame in wapp\n");
			/* Got Topo notify enable CCE for onboarded agents*/
			if ((wapp->map->map_version == DEV_TYPE_R3) && wapp->dpp) {
				if ((wapp->dpp->onboarding_type == 0) &&
					(wapp->dpp->dpp_allowed_roles == DPP_CAPAB_CONFIGURATOR)) {
					//Enable CCE for agents
					CCEIndication_1905_send(wapp, NULL, 1);
				}
			}
			break;
		case WAPP_USER_SEND_1905_ONBOARD_NOTIFY:
				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"wapp: got 1905 onboard notify frame \n");
				ret = dpp_parse_1905_onboard_notify_msg(wapp, map_msg->body, map_msg->length);
			/* Update AGentList TLV maintained in WAPP */
			break;
#ifdef MAP_R3_RECONFIG
		case WAPP_USER_GET_RECONFIG_TRIGGER:
				printf("\n reconfig status req received");
				ret = dpp_get_reconfig_status(wapp, map_msg->body, map_msg->length, event_buf, len_event);
			break;
		case WAPP_USER_SET_RECONFIG_START:
				printf("\n reconfig start req received");
				ret = dpp_get_reconfig_start(wapp, map_msg->body, map_msg->length);
			break;
#endif
		case WAPP_USER_GET_DPP_URI:
			DBGPRINT(RT_DEBUG_OFF, "wapp: got DPP URI req from mapd\n");
			ret = map_send_dpp_uri_msg(wapp, event_buf, len_event);
			break;
#endif /* MAP_R3 */
		case WAPP_SEND_BUFF_INCR_EVT:
			DBGPRINT(RT_DEBUG_OFF, "wapp: got buffer increase indication frame \n");
			char status = 1;
			os_memcpy(&buf_size, map_msg->body, map_msg->length);
			DBGPRINT(RT_DEBUG_WARN, "Buf size from the mapd :%d\n",buf_size);
			if(wapp->map_wapp_buffer) {
				wapp->map_wapp_buffer = os_realloc(wapp->map_wapp_buffer, buf_size);
				if(!wapp->map_wapp_buffer) {
					DBGPRINT(RT_DEBUG_ERROR, "Failed to allocate memory for map_wapp_buffer");
				}
				else {
					wapp_send_1905_msg(wapp, WAPP_RX_BUFFER_INCR_EVT, sizeof(status), &status);
					wapp->map_wapp_buffer_size = buf_size;
				}
			}
			else {
				DBGPRINT(RT_DEBUG_ERROR, "Failed to find buffer for map_wapp_buffer");
			}
			break;
		default:
			break;
	}

	prev_1905_msg = map_msg->type;

	return ret;
}


int wapp_send_1905_msg(
	struct wifi_app *wapp,
	u16 msg_type,
	u16 data_len,
	char *data)
{
	struct evt *map_event;
	int pkt_len = 0;
	char *buf = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s msg_type = %u\n", __func__, msg_type);

	pkt_len = sizeof(struct evt) + data_len;
	buf = os_zalloc(pkt_len);
	if (buf == NULL) {
		DBGPRINT(RT_DEBUG_TRACE, "%s  Alloc memory failed !!!!! \n", __func__);
		return -1;
	}

	map_event = (struct evt *)buf;
	map_event->type = msg_type;
	map_event->length = data_len;
	os_memcpy(map_event->buffer, data, data_len);

	if (0 > map_1905_send(wapp, buf, pkt_len)) {
		DBGPRINT(RT_DEBUG_TRACE, "%s  send fail \n", __func__);
		os_free(buf);
		return -1;
	}

	os_memset(buf, 0, pkt_len);
	os_free(buf);
	return 0;
}

int wapp_send_operbss_msg(struct wifi_app *wapp, char *buf,
	int max_len, unsigned char *identifier, unsigned char (*bssid)[6],
	unsigned char **ssid, unsigned char num)
{
	struct evt *wapp_event;
	struct oper_bss_cap *opcap = NULL;
	struct op_bss_cap *opbss = NULL;
	int send_pkt_len = 0, i = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_OPERBSS_REPORT;
	wapp_event->length = sizeof(struct oper_bss_cap) + num * sizeof(struct op_bss_cap);
	opcap = (struct oper_bss_cap *)wapp_event->buffer;
	memcpy(opcap->identifier, identifier, 6);
	opcap->oper_bss_num = num;
	opcap->band = 0;

	opbss = opcap->cap;
	for (i = 0; i < num; i++) {
		memcpy(opbss->bssid, bssid[i], 6);
		opbss->ssid_len = strlen((char *)ssid[i]);
		memcpy(opbss->ssid, ssid[i], opbss->ssid_len);
		opbss++;
	}

	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;
	if (0 > map_1905_send(wapp, buf, send_pkt_len)) {
		DBGPRINT(RT_DEBUG_TRACE, "%s  send operbss msg fail\n", __func__);
		return -1;
	}

	memset(buf, 0, send_pkt_len);
	return 0;
}

void map_1905_req(struct wifi_app *wapp, struct wapp_1905_req *req)
{
	wapp_send_1905_msg(
		wapp,
		WAPP_1905_REQ,
		sizeof (struct wapp_1905_req),
		(char *) req);

}

int map_update_neighbor_bss(struct wifi_app *wapp, struct topo_info* top_info)
{
	wapp_nr_info nr;
	int ret;//, i;

	memset(&nr, 0, sizeof(wapp_nr_info));
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	printf(BLUE("%s: bssid_num = %u\n"), __func__, top_info->bssid_num);
	COPY_MAC_ADDR(nr.Bssid, top_info->bssid);
	//for (i = 0; i < top_info->bssid_num; i++) {
		ret = nr_entry_add(wapp, &nr);
	//}

	if (ret != WAPP_SUCCESS)
		printf(RED("%s: failed\n"), __func__);

	return ret;
}
#ifdef MAP_SUPPORT
#ifdef AUTOROLE_NEGO
static void recv_DevRoleQuery_Response(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);
	int receive_len;
	char buf[100];
	struct dev_role_negotiate dev_role_event;
	char rx_pkt_type=0;
	DBGPRINT(RT_DEBUG_ERROR, "recv_DevRoleQuery_Response\n");
	os_memset(buf, 0, 100);

	receive_len = recvfrom(sock, buf, sizeof(buf) - 1, 0,
					(struct sockaddr *)&from, &fromlen);
	if(receive_len < 0){
		DBGPRINT(RT_DEBUG_ERROR, "receive from socket fail\n");
		return;
	}
	DBGPRINT(RT_DEBUG_INFO, "own almac address "MACSTR"\n",
			MAC2STR(wapp->map->agnt_alid));
	if (!memcmp(&buf[0], "5003", TAG_LEN))
		{
			if (!memcmp(&buf[TAG_LEN+PKT_TYPE_LEN], wapp->map->agnt_alid, MAC_ADDR_LEN))
			{
				DBGPRINT(RT_DEBUG_ERROR, "\n This is send by me\n");
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, "\n This is recieved by me from another dev\n");
				rx_pkt_type=buf[TAG_LEN];
				memcpy(dev_role_event.other_dev_almac, &buf[TAG_LEN+PKT_TYPE_LEN],MAC_ADDR_LEN);
				dev_role_event.other_dev_role=buf[TAG_LEN+PKT_TYPE_LEN+MAC_ADDR_LEN];
				DBGPRINT(RT_DEBUG_ERROR, "almac address other dev "MACSTR", dev_role_event->other_dev_role %d\n",
					MAC2STR(dev_role_event.other_dev_almac), dev_role_event.other_dev_role);
				if(wapp->map->my_map_dev_role == DEVICE_ROLE_UNCONFIGURED){
					wapp_send_1905_msg(
						wapp,
						WAPP_MAP_NEGO_ROLE_RESP,
						sizeof(struct dev_role_negotiate),
						(char *)&dev_role_event);
				}
				if(rx_pkt_type == PACKET_TYPE_QUERY) {
					send_DevRoleQuery_Response((unsigned int *)wapp->map->agnt_alid, PACKET_TYPE_RESPONSE, wapp->map->my_map_dev_role, wapp);
				}
			}
		}
	return;
}
void wapp_MapDevRoleNegotiation_init(struct wifi_app *wapp)
{
	struct sockaddr_in myaddr;
	int s;
	int yes = 1, on = 1;
	myaddr.sin_addr.s_addr=htonl(-1);
	myaddr.sin_port = htons(PORT3); /* port number */
	myaddr.sin_family = AF_INET;
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	setsockopt (s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));
	if (bind(s, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "bind addr to socket (%s)\n",strerror(errno));
		close(s);
		return;
	}
	eloop_register_read_sock(s, recv_DevRoleQuery_Response, wapp, NULL);
	wapp->map->nego_role_send_sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	setsockopt(wapp->map->nego_role_send_sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes) );
	setsockopt(wapp->map->nego_role_send_sock, SOL_SOCKET, SO_BINDTODEVICE, wapp->map->br_iface, strlen(wapp->map->br_iface));
	return;
}
void send_DevRoleQuery_Response(unsigned int *almac,unsigned char packet_type, unsigned char my_dev_role, struct wifi_app *wapp)
{
	int sock,sinlen, buf_len;
	struct sockaddr_in sock_in;
	char buf[25];
	DBGPRINT(RT_DEBUG_ERROR,"send_DevRoleQuery_Response\n");
	sinlen = sizeof(struct sockaddr_in);
	memset(&sock_in, 0, sinlen);
	sock = wapp->map->nego_role_send_sock;
	/* -1 = 255.255.255.255 this is a BROADCAST address,
	 a local broadcast address could also be used.
	 you can comput the local broadcat using NIC address and its NETMASK
	*/
	sock_in.sin_addr.s_addr=htonl(-1); /* send message to 255.255.255.255 */
	sock_in.sin_port = htons(PORT3); /* port number */
	sock_in.sin_family = AF_INET;
	memcpy(&buf[0], "5003", TAG_LEN);
	memcpy(&buf[TAG_LEN], &packet_type, PKT_TYPE_LEN);
	memcpy(&buf[TAG_LEN+PKT_TYPE_LEN], almac, MAC_ADDR_LEN);
	memcpy(&buf[TAG_LEN+PKT_TYPE_LEN+MAC_ADDR_LEN], &my_dev_role, ROLE_LEN);
	buf_len = TAG_LEN+PKT_TYPE_LEN+MAC_ADDR_LEN+ROLE_LEN;
	DBGPRINT(RT_DEBUG_ERROR, "pkt type %d and owndevRole %d\n",buf[TAG_LEN], buf[TAG_LEN+PKT_TYPE_LEN+MAC_ADDR_LEN]);
	sendto(sock, buf, buf_len, 0, (struct sockaddr *)&sock_in, sinlen);
}
void map_prepare_rawpacket(struct wifi_app *wapp, int role)
{
	if(role == DEVICE_ROLE_UNCONFIGURED)
		send_DevRoleQuery_Response((unsigned int *)wapp->map->agnt_alid, PACKET_TYPE_QUERY, role, wapp);
}
#endif //AUTOROLE_NEGO
#endif
#ifdef MAP_R2
void write_timestamp(char *timestamp, u8*ts_len);

extern u8 Cfg80211_RadarChan[];
u8 is_chan_op_class_supported(struct wifi_app *wapp, u8 ch, u8 op_class);
u32 wapp_fill_ch_list(struct wifi_app *wapp, OFFCHANNEL_SCAN_PARAM *scan_param,u8 op_class, u8 chan);

u8 map_is_channel_scan_allowed(struct wifi_app *wapp, struct wapp_dev *wdev, u8 ch,u8 op_class,u8 *status)
{
	struct os_time now;

	// operating class not supported
	// channel not supported
	//request too soon
	//radio too busy
	//stored result not availiable
	*status = SCAN_SUCCESS;

	os_get_time(&now);
	if(wdev->radio->last_scan_time.sec + wdev->radio->min_scan_interval > now.sec) {
		*status = REQ_TOO_SOON;
	} else if (is_chan_op_class_supported(wapp,ch, op_class) == FALSE) {
		*status = OP_CLASS_CHAN_NOT_SUPP;
	}

	return (*status == SCAN_SUCCESS)? TRUE: FALSE;
}
void wapp_on_boot_scan(void *eloop_data, void *user_ctx)
{
#if 1
	struct wifi_app	*wapp = (struct wifi_app *)eloop_data;
	struct off_ch_scan_req_s *scan_req = NULL;
	u8 buf[8000] = {0};
	u32 len;
	//u8 radio_id_5GH[6] = {0,0,0,0,1,0};
	//u8 radio_id_5GL[6] = {0,0,0,0,3,0};
	//u8 radio_id_2G[6] = {0,0,0,0,2,0};
	u8 i=0, k = 0;
#endif
#if 1
	scan_req = (struct off_ch_scan_req_s *)buf;
	DBGPRINT(RT_DEBUG_ERROR,"%s %d\n", __func__, __LINE__);
	mapd_get_channel_scan_capab_from_driver(wapp);

	scan_req->fresh_scan= 0x80;
	scan_req->radio_num = wapp->map->off_ch_scan_capab->radio_num;
	scan_req->neighbour_only = 2;
	scan_req->bw = 0;

	for(i=0; i< scan_req->radio_num; i++) {
		u8 j=0;
		os_memcpy(scan_req->body[i].radio_id, wapp->map->off_ch_scan_capab->radio_scan_params[i].radio_id, 6);
		scan_req->body[i].oper_class_num = wapp->map->off_ch_scan_capab->radio_scan_params[i].oper_class_num;
		for(j=0; j < scan_req->body[i].oper_class_num; j++) {
			scan_req->body[i].ch_body[j].oper_class = wapp->map->off_ch_scan_capab->radio_scan_params[i].ch_body[j].oper_class;
			scan_req->body[i].ch_body[j].ch_list_num = wapp->map->off_ch_scan_capab->radio_scan_params[i].ch_body[j].ch_list_num;
			for (k = 0;k < scan_req->body[i].ch_body[j].ch_list_num; k++) {
				scan_req->body[i].ch_body[j].ch_list[k] = wapp->map->off_ch_scan_capab->radio_scan_params[i].ch_body[j].ch_list[k];
			}
		}
	}
	len = sizeof(struct off_ch_scan_req_s) + scan_req->radio_num*sizeof(struct off_ch_scan_body);
	DBGPRINT(RT_DEBUG_ERROR,"%s: scanReqLen = %d\n",__func__,len);
	wapp->map->f_scan_req = 1;
	map_receive_off_channel_scan_req(wapp, (char *)buf,(u16)len);
	wapp->map->off_ch_scan_state.ch_scan_state = CH_ONBOOT_SCAN_ONGOING;
#endif
	// fill send channel scan req
}

int map_build_scan_cap(
	struct wifi_app *wapp, char *evt_buf)
{
	struct evt *map_event = NULL;
	//struct channel_scan_capab *scan_capab = wapp->map->scan_capab;
	int send_pkt_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_CHANNEL_SCAN_CAPAB;
	map_event->length = wapp->map->off_ch_scan_capab_len;
	os_memcpy(map_event->buffer, wapp->map->off_ch_scan_capab, wapp->map->off_ch_scan_capab_len);

	send_pkt_len = sizeof(*map_event) + map_event->length;
	return send_pkt_len;
}

int map_build_r2_ap_cap(
	struct wifi_app *wapp, char *evt_buf)
{
	struct evt *map_event = NULL;
	int send_pkt_len = 0;
	size_t r2_ap_capab_len;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_R2_AP_CAP;
	r2_ap_capab_len = sizeof(struct r2_ap_cap);
	map_event->length = r2_ap_capab_len;
	os_memcpy(map_event->buffer, &(wapp->map->r2_ap_capab), r2_ap_capab_len);

	send_pkt_len = sizeof(*map_event) + map_event->length;
	return send_pkt_len;
}


u8 wapp_get_radio_num(struct wifi_app *wapp)
{
	int i =0, num=0;
	for (i=0; i< MAX_NUM_OF_RADIO; i++) {
		if(wapp->radio[i].adpt_id)
			num++;
	}
	return num;
}

void fill_scan_cap_5g(struct ap_dev *ap, struct radio_scan_capab *scan_radio_capab, int band)
{
	int a = 0, b = 0, op_num = 0;
#ifdef MAP_6E_SUPPORT
	struct _wdev_op_class_info_ext *op_class = NULL;
#else
	wdev_op_class_info *op_class = NULL;
#endif

	op_class = &ap->op_class;

#ifdef MAP_6E_SUPPORT
	for (a = 0; a < op_class->num_of_op_class; a++) {
		if (band == WPS_BAND_5G) {
			if (op_class->opClassInfoExt[a].op_class != 115
				&& op_class->opClassInfoExt[a].op_class != 118
				&& op_class->opClassInfoExt[a].op_class != 121
				&& op_class->opClassInfoExt[a].op_class != 125) {
				continue;
			}
		} else if (band == WPS_BAND_5GL) {
			if (op_class->opClassInfoExt[a].op_class != 115
				&& op_class->opClassInfoExt[a].op_class != 118)
				continue;
		} else if (band == WPS_BAND_5GH) {
			if (op_class->opClassInfoExt[a].op_class != 121
				&& op_class->opClassInfoExt[a].op_class != 125)
				continue;
		}

		op_num = scan_radio_capab->oper_class_num;
		scan_radio_capab->oper_class_num++;
		scan_radio_capab->ch_body[op_num].oper_class = op_class->opClassInfoExt[a].op_class;
		scan_radio_capab->ch_body[op_num].ch_list_num = op_class->opClassInfoExt[a].num_of_ch;

		for (b = 0; b < op_class->opClassInfoExt[a].num_of_ch; b++)
			scan_radio_capab->ch_body[op_num].ch_list[b] = op_class->opClassInfoExt[a].ch_list[b];
	}
#else
	for (a = 0; a < op_class->num_of_op_class; a++) {
		if (band == WPS_BAND_5G) {
			if (op_class->opClassInfo[a].op_class != 115
				&& op_class->opClassInfo[a].op_class != 118
				&& op_class->opClassInfo[a].op_class != 121
				&& op_class->opClassInfo[a].op_class != 125)
				continue;
			} else if (band == WPS_BAND_5GL) {
				if (op_class->opClassInfo[a].op_class != 115
					&& op_class->opClassInfo[a].op_class != 118)
					continue;
			} else if (band == WPS_BAND_5GH) {
				if (op_class->opClassInfo[a].op_class != 121
					&& op_class->opClassInfo[a].op_class != 125)
					continue;
			}
		op_num = scan_radio_capab->oper_class_num;
		scan_radio_capab->oper_class_num++;
		scan_radio_capab->ch_body[op_num].oper_class = op_class->opClassInfo[a].op_class;
		scan_radio_capab->ch_body[op_num].ch_list_num = op_class->opClassInfo[a].num_of_ch;
		for (b = 0; b < op_class->opClassInfo[a].num_of_ch; b++)
			scan_radio_capab->ch_body[op_num].ch_list[b] = op_class->opClassInfo[a].ch_list[b];
	}
#endif
}
#ifdef MAP_6E_SUPPORT
void fill_scan_cap_6g(struct ap_dev *ap, struct radio_scan_capab *scan_radio_capab)
{
	int a = 0, b = 0, op_num = 0;
	struct _wdev_op_class_info_ext *op_class = NULL;

	op_class = &ap->op_class;

	for (a = 0; a < op_class->num_of_op_class; a++) {
		if (op_class->opClassInfoExt[a].op_class != 131)
			continue;

		op_num = scan_radio_capab->oper_class_num;
		scan_radio_capab->oper_class_num++;
		scan_radio_capab->ch_body[op_num].oper_class = op_class->opClassInfoExt[a].op_class;
		scan_radio_capab->ch_body[op_num].ch_list_num = op_class->opClassInfoExt[a].num_of_ch;
		for (b = 0; b < op_class->opClassInfoExt[a].num_of_ch; b++)
			scan_radio_capab->ch_body[op_num].ch_list[b] = op_class->opClassInfoExt[a].ch_list[b];
	}
}
#endif
void fill_scan_cap_24g(struct ap_dev *ap, struct radio_scan_capab *scan_radio_capab)
{
	int a = 0, b = 0, op_num = 0;
#ifdef MAP_6E_SUPPORT
	struct _wdev_op_class_info_ext *op_class = NULL;
#else
	wdev_op_class_info *op_class = NULL;
#endif

	op_class = &ap->op_class;
#ifdef MAP_6E_SUPPORT
	for (a = 0; a < op_class->num_of_op_class; a++) {
		if (op_class->opClassInfoExt[a].op_class != 81
			&& op_class->opClassInfoExt[a].op_class != 82)
			continue;
		op_num = scan_radio_capab->oper_class_num;
		scan_radio_capab->oper_class_num++;
		scan_radio_capab->ch_body[op_num].oper_class = op_class->opClassInfoExt[a].op_class;
		scan_radio_capab->ch_body[op_num].ch_list_num = op_class->opClassInfoExt[a].num_of_ch;
		for (b = 0; b < op_class->opClassInfoExt[a].num_of_ch; b++)
			scan_radio_capab->ch_body[op_num].ch_list[b] = op_class->opClassInfoExt[a].ch_list[b];
	}
#else
	for (a = 0; a < op_class->num_of_op_class; a++) {
		if (op_class->opClassInfo[a].op_class != 81
			&& op_class->opClassInfo[a].op_class != 82)
			continue;
		op_num = scan_radio_capab->oper_class_num;
		scan_radio_capab->oper_class_num++;
		scan_radio_capab->ch_body[op_num].oper_class = op_class->opClassInfo[a].op_class;
		scan_radio_capab->ch_body[op_num].ch_list_num = op_class->opClassInfo[a].num_of_ch;
		for (b = 0; b < op_class->opClassInfo[a].num_of_ch; b++)
			scan_radio_capab->ch_body[op_num].ch_list[b] = op_class->opClassInfo[a].ch_list[b];
	}
#endif
}
u8 mapd_get_channel_scan_capab_from_driver(struct wifi_app *wapp)
{
	//get the channel scan capabilities from driver for each radio.
	// for cert this can be hardcoded.

	//cert. : fill the channel scan capabilities.
	struct off_ch_scan_capab *scan_capab = NULL;//wapp->map->scan_capab;
	size_t scan_capab_len;
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	int scan_radio_num = 0;
	struct radio_scan_capab *scan_radio_capab = NULL;
	u8 radio_num = wapp_get_radio_num(wapp);
	//u8 op_class_num = 6;
	u8 i=0;

	if(!(radio_num == 1 || radio_num ==2 || radio_num ==3)) {
		DBGPRINT(RT_DEBUG_ERROR,"%s: Invalid radio num %d\n", __func__, radio_num);
#ifndef SINGLE_BAND_SUPPORT	/* for single/no wifi radio*/
		return FALSE;
#endif
	}

	scan_capab_len = sizeof(struct off_ch_scan_capab);

	if(wapp->map->off_ch_scan_capab == NULL || (scan_capab_len != wapp->map->off_ch_scan_capab_len)) {

		if(wapp->map->off_ch_scan_capab != NULL) {
			os_free(wapp->map->off_ch_scan_capab);
			wapp->map->off_ch_scan_capab= NULL;
			wapp->map->off_ch_scan_capab_len = 0;
		}
		scan_capab = os_zalloc(scan_capab_len);
		if(scan_capab == NULL)
			return FALSE;
	} else
		scan_capab = wapp->map->off_ch_scan_capab;

	scan_capab->radio_num = radio_num;
	for (i=0; i < radio_num; i++) {
		scan_capab->radio_scan_params[i].boot_scan_only = 0;
		scan_capab->radio_scan_params[i].scan_impact = 3;
		scan_capab->radio_scan_params[i].min_scan_interval = 60;
	}

	for (i=0; i< MAX_NUM_OF_RADIO; i++) {
		DBGPRINT(RT_DEBUG_ERROR,"%s %d %d %d\n", __func__, __LINE__, i, wapp->radio[i].op_ch);
		if(wapp->radio[i].adpt_id == 0 || wapp->radio[i].op_ch == 0) {
			DBGPRINT(RT_DEBUG_TRACE, "%s %d %d\n", __func__, __LINE__, i);
			continue;
		}

		wdev = wapp_dev_list_lookup_by_band_and_type(wapp, *wapp->radio[i].radio_band, WAPP_DEV_TYPE_AP);
		if(wdev == NULL)
			continue;
		ap = (struct ap_dev *)wdev->p_dev;
		scan_radio_capab = &scan_capab->radio_scan_params[scan_radio_num];
		if (IS_MAP_CH_24G(wapp->radio[i].op_ch)
#ifdef MAP_6E_SUPPORT
				&& (*wapp->radio[i].radio_band == WPS_BAND_24G)
#endif
			) {

			DBGPRINT(RT_DEBUG_ERROR,"%s %d: 2G\n", __func__, __LINE__);
			MAP_GET_RADIO_IDNFER((&wapp->radio[i]), scan_capab->radio_scan_params[scan_radio_num].radio_id);
			fill_scan_cap_24g(ap, scan_radio_capab);
			scan_radio_num++;
			wapp->radio[i].min_scan_interval = 2;
		} else if
#ifndef MAP_6E_SUPPORT
			(IS_MAP_CH_5G(wapp->radio[i].op_ch)
#else
			((IS_MAP_CH_5G(wapp->radio[i].op_ch) && ((*wapp->radio[i].radio_band == WPS_BAND_5G) ||
			(*wapp->radio[i].radio_band == WPS_BAND_5GL) || (*wapp->radio[i].radio_band == WPS_BAND_5GH))) ||
			(IS_MAP_CH_6G(wapp->radio[i].op_ch) && (*wapp->radio[i].radio_band == WPS_BAND_6G))
#endif
			) {
			if (radio_num < 3) {
#ifdef MAP_6E_SUPPORT
				if (IS_MAP_CH_5G(wapp->radio[i].op_ch) && (*wapp->radio[i].radio_band == WPS_BAND_5G)) {
#endif
					MAP_GET_RADIO_IDNFER((&wapp->radio[i]), (scan_capab->radio_scan_params[scan_radio_num].radio_id));
					fill_scan_cap_5g(ap, scan_radio_capab, WPS_BAND_5G);
					scan_radio_num++;
					wapp->radio[i].min_scan_interval = 2;
#ifdef MAP_6E_SUPPORT
				} else {
					MAP_GET_RADIO_IDNFER((&wapp->radio[i]), (scan_capab->radio_scan_params[scan_radio_num].radio_id));
					fill_scan_cap_6g(ap, scan_radio_capab);
					scan_radio_num++;
					wapp->radio[i].min_scan_interval = 2;
				}
#endif
			} else if (radio_num == 3) {
				if (IS_MAP_CH_5GL(wapp->radio[i].op_ch)
#ifdef MAP_6E_SUPPORT
					&& (*wapp->radio[i].radio_band == WPS_BAND_5GL)
#endif
				) {
					MAP_GET_RADIO_IDNFER((&wapp->radio[i]), (scan_capab->radio_scan_params[scan_radio_num].radio_id));
					fill_scan_cap_5g(ap, scan_radio_capab, WPS_BAND_5GL);
					scan_radio_num++;
					wapp->radio[i].min_scan_interval = 2;
				} else if (IS_MAP_CH_5GH(wapp->radio[i].op_ch)
#ifdef MAP_6E_SUPPORT
						&& (*wapp->radio[i].radio_band == WPS_BAND_5GH)
#endif
				) {
					MAP_GET_RADIO_IDNFER((&wapp->radio[i]), (scan_capab->radio_scan_params[scan_radio_num].radio_id));
					fill_scan_cap_5g(ap, scan_radio_capab, WPS_BAND_5GH);
					scan_radio_num++;
					wapp->radio[i].min_scan_interval = 2;

				}
#ifdef MAP_6E_SUPPORT
				else if (IS_MAP_CH_6G(wapp->radio[i].op_ch) && (*wapp->radio[i].radio_band == WPS_BAND_5G)) {
					MAP_GET_RADIO_IDNFER((&wapp->radio[i]), (scan_capab->radio_scan_params[scan_radio_num].radio_id));
					fill_scan_cap_5g(ap, scan_radio_capab, WPS_BAND_5G);
					scan_radio_num++;
					wapp->radio[i].min_scan_interval = 2;
				}
				else if (IS_MAP_CH_6G(wapp->radio[i].op_ch) && (*wapp->radio[i].radio_band == WPS_BAND_6G)) {
					MAP_GET_RADIO_IDNFER((&wapp->radio[i]), (scan_capab->radio_scan_params[scan_radio_num].radio_id));
					fill_scan_cap_6g(ap, scan_radio_capab);
					scan_radio_num++;
					wapp->radio[i].min_scan_interval = 2;
				}
#endif
			}

		}
	}

	wapp->map->off_ch_scan_capab = scan_capab;
	wapp->map->off_ch_scan_capab_len = scan_capab_len;
	hex_dump_dbg("ScanCapab",(unsigned char *)wapp->map->off_ch_scan_capab,(u32)wapp->map->off_ch_scan_capab_len);
	return TRUE;
}

#ifdef DFS_CAC_R2
int map_send_cac_status(
	struct wifi_app *wapp, char *evt_buf,
	struct cac_status_report_lib *cac_status, u16 *status_len, u8 cac_completion)
{
	unsigned char *buff;
	struct evt *map_event = NULL;
	u16 i=0, ptr=0;

	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);
	*status_len = sizeof(struct cac_status_report_lib) + (cac_status->ongoing_cac_channel_num * sizeof(struct cac_ongoing_channel));

	if(!cac_completion) {
		map_event = (struct evt *)evt_buf;
		map_event->type = WAPP_CAC_STATUS_REPORT;
		map_event->length = *status_len;
		buff = &map_event->buffer[0];
	} else {
		buff = (unsigned char *)evt_buf;
	}
	buff[ptr] = cac_status->allowed_channel_num;
	ptr += 1;
	buff[ptr] = cac_status->non_allowed_channel_num;
	ptr += 1;
	buff[ptr] = cac_status->ongoing_cac_channel_num;
	ptr += 1;

	for (i=0; i < MAX_CLASS_CHANNEL; i++) {
		buff[ptr] = cac_status->allowed_channel[i].op_class;
		ptr += 1;
		buff[ptr] = cac_status->allowed_channel[i].ch_num;
		ptr += 1;
		os_memcpy(&buff[ptr], &cac_status->allowed_channel[i].cac_interval, 2);
		ptr += 2;
	}


	for (i = 0; i < MAX_CLASS_CHAN_NON_ALLOWED; i++) {
		buff[ptr] = cac_status->non_allowed_channel[i].op_class;
		ptr += 1;
		buff[ptr] = cac_status->non_allowed_channel[i].ch_num;
		ptr += 1;
		os_memcpy(&buff[ptr], &cac_status->non_allowed_channel[i].remain_interval, 2);
		ptr += 2;
	}

	for (i=0; i < cac_status->ongoing_cac_channel_num; i++) {
		buff[ptr] = cac_status->cac_ongoing_channel[i].op_class;
		ptr += 1;
		buff[ptr] = cac_status->cac_ongoing_channel[i].ch_num;
		ptr += 1;
		os_memcpy(&buff[ptr], &cac_status->cac_ongoing_channel[i].remain_interval, 3);
		ptr += 2;
	}
	*status_len = sizeof(struct evt) + (*status_len);
	return MAP_SUCCESS;
}


int mapd_get_cac_status_from_driver(struct wifi_app *wapp,
	char *evt_buf, int* len_buf, u8 cac_completion)
{
	u8 radio_num = wapp_get_radio_num(wapp);
	struct wapp_dev *wdev = NULL;
	struct cac_status_report_lib cac_status = {0};
	struct cac_driver_capab *driver_cap = NULL;
	u8 i=0, k=0, m=0, n=0, o=0, p=0;
	char buf[4096]={0};
	u32 len=4096;
	u16 status_len=0;
	char radio_id[MAC_ADDR_LEN];
	struct ap_dev *ap = NULL;
	struct os_time now, delta;

	if(!(radio_num == 1 || radio_num == 2 || radio_num == 3)) {
		printf("invalid radio num %d\n", radio_num);
#ifndef SINGLE_BAND_SUPPORT	/* for single/no wifi radio*/
		return FALSE;
#endif
	}

	os_memset(&cac_status, 0, sizeof(struct cac_status_report_lib));

	for (i=0; i < radio_num; i++) {
		if(wapp->radio[i].adpt_id == 0) {
			continue;
		}
		if (IS_MAP_CH_24G(wapp->radio[i].op_ch)
#ifdef MAP_6E_SUPPORT
			&& IS_OP_CLASS_24G(wapp->radio[i].opclass)
#endif
		) {
			continue;
		}
#ifdef MAP_6E_SUPPORT
		if (IS_MAP_CH_6G(wapp->radio[i].op_ch) && IS_OP_CLASS_6G(wapp->radio[i].opclass))
			continue;
#endif
		MAP_GET_RADIO_IDNFER((&wapp->radio[i]), radio_id);
		wdev = wapp_dev_list_lookup_by_radio(wapp, radio_id);
		if (!wdev) {
			DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
			return 0;
		}
			ap = (struct ap_dev *)wdev->p_dev;
			driver_wext_get_cac_capability(wapp->drv_data, wdev->ifname, buf, len);
			driver_cap = (struct cac_driver_capab *)buf;
			if (driver_cap->active_cac == TRUE) {
				cac_status.cac_ongoing_channel[cac_status.ongoing_cac_channel_num].ch_num = driver_cap->ch_num;
				cac_status.cac_ongoing_channel[cac_status.ongoing_cac_channel_num].op_class = ap->ch_info.op_class;
				cac_status.cac_ongoing_channel[cac_status.ongoing_cac_channel_num].remain_interval = driver_cap->cac_remain_time;
				cac_status.ongoing_cac_channel_num++;
				status_len += 5;
			}

			for(m=0; m < driver_cap->op_class_num; m++) {
				for(n=0; n < driver_cap->opcap[m].ch_num; n++){
					if (driver_cap->opcap[m].non_occupancy_remain[n] != 0) {
						//not available channel
						if((IS_MAP_CH_5GL(wapp->radio[i].op_ch)) ||
							(IS_MAP_CH_5GH(wapp->radio[i].op_ch))){
							cac_status.non_allowed_channel_num++;
							cac_status.non_allowed_channel[k].op_class = driver_cap->opcap[m].op_class;
							cac_status.non_allowed_channel[k].ch_num = driver_cap->opcap[m].ch_list[n];
							cac_status.non_allowed_channel[k].remain_interval = (u16)driver_cap->opcap[m].non_occupancy_remain[n];
							status_len += 4;
						}

						k++;
					}else { //available channels
						if((IS_MAP_CH_5GL(wapp->radio[i].op_ch)) ||
							(IS_MAP_CH_5GH(wapp->radio[i].op_ch))){
							cac_status.allowed_channel[o].op_class = driver_cap->opcap[m].op_class;
							cac_status.allowed_channel[o].ch_num = driver_cap->opcap[m].ch_list[n];
							cac_status.allowed_channel_num++;
							status_len += 4;

							for(p=0; p<wapp->map->cac_list.ch_num; p++)
							{
								if(cac_status.allowed_channel[o].ch_num == wapp->map->cac_list.ch_list[p]) {
									os_get_time(&now);
									os_time_sub(&now, &wapp->map->cac_list.last_cac_time[p], &delta);
									cac_status.allowed_channel[o].cac_interval =
										(u16)(delta.sec / 60) ? (u16)(delta.sec / 60): (u16)1;
									break;
								}
							}
							o++;
						}

					}
				}
			}
	}

	status_len += 3; // 3 bytes for allowed/non_allowed/ongoing cac channel number
	map_send_cac_status(wapp, evt_buf, &cac_status, &status_len, cac_completion);
	*len_buf = status_len;
	return TRUE;

}

int mapd_get_cac_capab_from_driver(struct wifi_app *wapp, unsigned char *addr)
{
	u8 radio_num = wapp_get_radio_num(wapp);
	struct wapp_dev *wdev = NULL;
	struct cac_capability_lib *cac_capab = NULL;
	struct cac_driver_capab *driver_cap = NULL;
	size_t capab_len;
	u8 i=0, j=0, k=0, m=0, l=0, n=0, o=0;
	char buf[4096]={0};
	unsigned int len=4096;
	char radio_id[MAC_ADDR_LEN];
	BOOLEAN ch_added = FALSE;

	if(!(radio_num == 1 || radio_num ==2 || radio_num ==3)) {
		printf("invalid radio num %d\n", radio_num);
#ifndef SINGLE_BAND_SUPPORT	/* for single/no wifi radio*/
		return FALSE;
#endif
	}

	capab_len = sizeof(struct cac_capability_lib);

	if(wapp->map->cac_capab == NULL || (capab_len != wapp->map->cac_capab_len)) {

		if(wapp->map->cac_capab != NULL) {
			os_free(wapp->map->cac_capab);
			wapp->map->cac_capab= NULL;
			wapp->map->cac_capab_len = 0;
		}
		cac_capab = os_zalloc(capab_len);
		if(cac_capab == NULL)
			return FALSE;
	} else
		cac_capab = wapp->map->cac_capab;

	for (i=0; i < radio_num; i++) {
		if(wapp->radio[i].adpt_id == 0) {
			printf("%s %d %d\n", __func__, __LINE__, i);
			continue;
		}
		if (IS_MAP_CH_24G(wapp->radio[i].op_ch)
#ifdef MAP_6E_SUPPORT
			&& IS_OP_CLASS_6G(wapp->radio[i].opclass)
#endif
		) {
			continue;
		}

#ifdef MAP_6E_SUPPORT
		if (IS_MAP_CH_6G(wapp->radio[i].op_ch) && IS_OP_CLASS_6G(wapp->radio[i].opclass))
			continue;
#endif

		MAP_GET_RADIO_IDNFER((&wapp->radio[i]), radio_id);
		wdev = wapp_dev_list_lookup_by_radio(wapp, radio_id);
		if (!wdev) {
			DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
			os_free(cac_capab);
			return 0;
		}
			driver_wext_get_cac_capability(wapp->drv_data, wdev->ifname,buf,len);
			cac_capab->radio_num++;
			driver_cap = (struct cac_driver_capab *)buf;
			os_memcpy(cac_capab->country_code, driver_cap->country_code, 2);
			if (driver_cap->rdd_region == CE)
				cac_capab->cap[k].cac_type_num = 2;
			else
				cac_capab->cap[k].cac_type_num = 1;
			os_memcpy(&cac_capab->cap[k].identifier[0], radio_id, MAC_ADDR_LEN);
			wapp->map->cac_capab_final_len += 7;
			for(m=0; m<cac_capab->cap[k].cac_type_num && m < 2; m++) {
				cac_capab->cap[k].type[m].cac_mode = driver_cap->cac_mode;
				wdev->cac_method = driver_cap->cac_mode;
				printf("in WAPP driver_cap->cac_mode %d", driver_cap->cac_mode);
				if(m == 0)
					cac_capab->cap[k].type[m].cac_interval[2] = 65;
				else {
					//For setting cac time 650 secs
					cac_capab->cap[k].type[m].cac_interval[2] = 0x8A;
					cac_capab->cap[k].type[m].cac_interval[1] = 0x02;
				}
				cac_capab->cap[k].type[m].op_class_num = driver_cap->op_class_num;
				wapp->map->cac_capab_final_len += 5;
				for(j=0; j<driver_cap->op_class_num; j++)
				{	if (radio_num == 3)
					{
						if(IS_MAP_CH_5GL(wapp->radio[i].op_ch)){
							if ((radio_num < 3) || (driver_cap->opcap[j].op_class < 121 ||
								driver_cap->opcap[j].op_class >= 128))
							{
								if (m == 0)
								{
									cac_capab->cap[k].type[m].opcap[n].ch_num = driver_cap->opcap[j].ch_num;
									for (l=0; l<driver_cap->opcap[j].ch_num; l++)
									{
											if (driver_cap->opcap[j].ch_list[l] < 100
												&& driver_cap->opcap[j].cac_time[l] == 65)
											{
												cac_capab->cap[k].type[m].opcap[n].ch_list[o] = driver_cap->opcap[j].ch_list[l];
												o++;
												wapp->map->cac_capab_final_len += 1;
												ch_added = TRUE;
											}
											else if(cac_capab->cap[k].type[m].opcap[n].ch_num > 0)
												cac_capab->cap[k].type[m].opcap[n].ch_num--;

									}
									if (ch_added == TRUE)
									{
										cac_capab->cap[k].type[m].opcap[n].op_class = driver_cap->opcap[j].op_class;
										wapp->map->cac_capab_final_len += 2;
										ch_added = FALSE;
										n++;
									}
									else
										cac_capab->cap[k].type[m].op_class_num--;

									o=0;
								}
								else if(m == 1)
								{
									cac_capab->cap[k].type[m].opcap[n].ch_num = driver_cap->opcap[j].ch_num;
									for(l=0; l<driver_cap->opcap[j].ch_num; l++)
									{
											if(driver_cap->opcap[j].ch_list[l] < 100
												&& driver_cap->opcap[j].cac_time[l] == 650) {
												cac_capab->cap[k].type[m].opcap[n].ch_list[o] = driver_cap->opcap[j].ch_list[l];
												o++;
												wapp->map->cac_capab_final_len += 1;
												ch_added = TRUE;
											}
											else if(cac_capab->cap[k].type[m].opcap[n].ch_num > 0)
												cac_capab->cap[k].type[m].opcap[n].ch_num--;
									}
									if(ch_added == TRUE)
									{
										cac_capab->cap[k].type[m].opcap[n].op_class = driver_cap->opcap[j].op_class;
										wapp->map->cac_capab_final_len += 2;
										ch_added = FALSE;
										n++;
									}
									else
										cac_capab->cap[k].type[m].op_class_num--;
									o=0;
								}
								else
									cac_capab->cap[k].type[m].op_class_num--;
							}
							else
							{
								cac_capab->cap[k].type[m].op_class_num--;

							}
						}
						else if(IS_MAP_CH_5GH(wapp->radio[i].op_ch)) {
							if((radio_num < 3) || (driver_cap->opcap[j].op_class >= 121)) {
								if(m == 0)
								{
									cac_capab->cap[k].type[m].opcap[n].ch_num = driver_cap->opcap[j].ch_num;
									for(l=0; l<driver_cap->opcap[j].ch_num; l++)
									{
											if(driver_cap->opcap[j].ch_list[l] >= 100 &&
												driver_cap->opcap[j].cac_time[l] == 65) {
												cac_capab->cap[k].type[m].opcap[n].ch_list[o] = driver_cap->opcap[j].ch_list[l];
												o++;
												wapp->map->cac_capab_final_len += 1;
												ch_added = TRUE;
											}
											else if(cac_capab->cap[k].type[m].opcap[n].ch_num > 0)
												cac_capab->cap[k].type[m].opcap[n].ch_num--;


									}
									if(ch_added == TRUE)
									{
										cac_capab->cap[k].type[m].opcap[n].op_class = driver_cap->opcap[j].op_class;
										wapp->map->cac_capab_final_len += 2;
										ch_added = FALSE;
										n++;
									}
									else
										cac_capab->cap[k].type[m].op_class_num--;
									o=0;
								}
								else if(m == 1)
								{
									cac_capab->cap[k].type[m].opcap[n].ch_num = driver_cap->opcap[j].ch_num;
									for(l=0; l<driver_cap->opcap[j].ch_num; l++)
									{
											if(driver_cap->opcap[j].ch_list[l] >= 100
												&& driver_cap->opcap[j].cac_time[l] == 650) {
												cac_capab->cap[k].type[m].opcap[n].ch_list[o] = driver_cap->opcap[j].ch_list[l];
												o++;
												 wapp->map->cac_capab_final_len += 1;
												 ch_added = TRUE;
											}
											else if(cac_capab->cap[k].type[m].opcap[n].ch_num > 0)
												cac_capab->cap[k].type[m].opcap[n].ch_num--;

									}
									if(ch_added == TRUE)
									{
										cac_capab->cap[k].type[m].opcap[n].op_class = driver_cap->opcap[j].op_class;
										wapp->map->cac_capab_final_len += 2;
										ch_added = FALSE;
										n++;
									}
									else
										cac_capab->cap[k].type[m].op_class_num--;
									o=0;
								}
								else
									cac_capab->cap[k].type[m].op_class_num--;
							}
							else
							{
								cac_capab->cap[k].type[m].op_class_num--;

							}

						}
					}
					else {
					if(driver_cap->opcap[j].op_class >= 121) {
						if(m == 0) {
							cac_capab->cap[k].type[m].opcap[n].ch_num = driver_cap->opcap[j].ch_num;
								for(l=0; l<driver_cap->opcap[j].ch_num; l++)
								{
									if(driver_cap->opcap[j].cac_time[l] == 65) {
										cac_capab->cap[k].type[m].opcap[n].ch_list[o] = driver_cap->opcap[j].ch_list[l];
										o++;
										wapp->map->cac_capab_final_len += 1;
										ch_added = TRUE;
									}
									else if(cac_capab->cap[k].type[m].opcap[n].ch_num > 0)
										cac_capab->cap[k].type[m].opcap[n].ch_num--;


								}
								if(ch_added == TRUE)
								{
									cac_capab->cap[k].type[m].opcap[n].op_class = driver_cap->opcap[j].op_class;
									wapp->map->cac_capab_final_len += 2;
									ch_added = FALSE;
									n++;
								}
								else
									cac_capab->cap[k].type[m].op_class_num--;
								o=0;
							}
							else if(m == 1)
							{
								cac_capab->cap[k].type[m].opcap[n].ch_num = driver_cap->opcap[j].ch_num;
								for(l=0; l<driver_cap->opcap[j].ch_num; l++)
								{
									if(driver_cap->opcap[j].cac_time[l] == 650) {
										cac_capab->cap[k].type[m].opcap[n].ch_list[o] = driver_cap->opcap[j].ch_list[l];
										o++;
										 wapp->map->cac_capab_final_len += 1;
										 ch_added = TRUE;
									}
									else if(cac_capab->cap[k].type[m].opcap[n].ch_num > 0)
										cac_capab->cap[k].type[m].opcap[n].ch_num--;

								}
								if(ch_added == TRUE)
								{
									cac_capab->cap[k].type[m].opcap[n].op_class = driver_cap->opcap[j].op_class;
									wapp->map->cac_capab_final_len += 2;
									ch_added = FALSE;
									n++;
								}
								else
									cac_capab->cap[k].type[m].op_class_num--;
								o=0;
							}
							else
								cac_capab->cap[k].type[m].op_class_num--;
						}
						else
						{
							cac_capab->cap[k].type[m].op_class_num--;

						}
					}
				}
			n=0;o=0;
			}
			k++;
	}

	wapp->map->cac_capab = cac_capab;
	wapp->map->cac_capab_len = capab_len;
	wapp->map->cac_capab_final_len += sizeof(struct cac_lib);
	return TRUE;
}



#endif
void map_send_assoc_notification(struct wifi_app *wapp, const char *iface, u8 assoc_disallow_reason)
{
	struct assoc_notification_lib *assoc_notification = NULL;
	u16 assoc_status_len;
	int i=0, j=0;
	u8 *buf = NULL;

	u8 assoc_tlv_num = 1;
	u8 bssid_num = 1;


	struct wapp_dev *wdev = NULL;

	wdev = wapp_dev_list_lookup_by_ifname(wapp, iface);
	if (!wdev)
		return;

	assoc_status_len = sizeof(struct assoc_notification_lib) + (assoc_tlv_num* sizeof(struct assoc_notification_tlv)) + (bssid_num* sizeof(struct assoc_status));


	assoc_notification = os_zalloc(assoc_status_len);
	if(!assoc_notification) {
		DBGPRINT(RT_DEBUG_ERROR,"memory alloc failed %s",__func__);
		return;
	}

	assoc_notification->assoc_notification_tlv_num = assoc_tlv_num;

	buf = (u8 *)assoc_notification->notification_tlv;

	struct assoc_notification_tlv * assoc_status_evt = (struct assoc_notification_tlv *)buf;

	for (i = 0; i < assoc_tlv_num; i++) {
		assoc_status_evt->bssid_num = bssid_num;
		for (j = 0; j < bssid_num; j++) {
			os_memcpy(&assoc_status_evt->status[j].bssid, wdev->mac_addr, MAC_ADDR_LEN);
			assoc_status_evt->status[j].status = !assoc_disallow_reason;/*0 = Disallow, 1 = Allow*/
		}
	}

	map_build_and_send_assoc_status_notification(wapp, assoc_notification, bssid_num);
}

#if 0
void dump_assoc(struct assoc_notification_lib *assoc)
{
	int i,j;
	for(i=0; i< assoc->assoc_notification_tlv_num;i++) {
		struct assoc_notification_tlv *tlv = (struct assoc_notification_tlv *)assoc->notification_tlv;
		printf("RESULT : %d--------------------->START\n", i);
		printf("bssid_num = %d\n",
				tlv->bssid_num);
		for (j=0;j<tlv->bssid_num;j++) {
			struct assoc_status *tlv_status = &tlv->status[j];
			printf("BSSID:"MACSTR"\n", MAC2STR(tlv_status->bssid));
			printf("Status: %d \n", tlv_status->status);

		}
	}

}
#endif


void map_build_and_send_assoc_status_notification(struct wifi_app *wapp, struct assoc_notification_lib *assoc_notify,u8  bssid_num)
{
	struct evt *map_event = NULL;
	struct assoc_notification_lib *assoc_notification = NULL;
	u16 evt_len = 0;
	u16 assoc_status_len = 0;
	int i=0, j=0;
	u8 *buf = NULL;
	struct assoc_notification_tlv * assoc_status_evt = NULL;

	assoc_status_len = sizeof(struct assoc_notification_lib) + (assoc_notify->assoc_notification_tlv_num* sizeof(struct assoc_notification_tlv))
				+ (bssid_num* sizeof(struct assoc_status));

	evt_len = sizeof(struct evt) + assoc_status_len;
	map_event = os_zalloc(evt_len);
	if(map_event == NULL)
		goto Error;

	map_event->type = WAPP_ASSOC_STATUS_NOTIFICATION;
	map_event->length = assoc_status_len;

	assoc_notification = (struct assoc_notification_lib *)map_event->buffer;

	assoc_notification->assoc_notification_tlv_num = assoc_notify->assoc_notification_tlv_num;

	buf = (u8 *)assoc_notification->notification_tlv;

	assoc_status_evt = (struct assoc_notification_tlv *)buf;

	for (i = 0; i < assoc_notify->assoc_notification_tlv_num; i++) {
		assoc_status_evt->bssid_num = assoc_notify->notification_tlv[i].bssid_num;
		for (j = 0; j < assoc_status_evt->bssid_num; j++) {
			os_memcpy(&assoc_status_evt->status[j].bssid, assoc_notify->notification_tlv[i].status[j].bssid, MAC_ADDR_LEN);
			assoc_status_evt->status[j].status = assoc_notify->notification_tlv[i].status[j].status;
		}
	}

	//dump_assoc(assoc_notification);

	if (0 > map_1905_send(wapp, (char *)map_event, evt_len)) {
		DBGPRINT(RT_DEBUG_ERROR, "%s send ch scan msg fail\n", __func__);
	}


	Error:

		if(map_event)
			os_free(map_event);

		os_free(assoc_notify);
	return;
}

#define CATEGORY_PUBLIC		4
#define BSS_TRANSITION_QUERY    6
#define CATEGORY_WNM            10
#define ACTION_GAS_INIT_REQ     10

void map_send_btm_tunneled_message(struct wifi_app *wapp, const unsigned char *peer_addr,const char *btm_query, size_t btm_query_len)
{

	unsigned int send_pkt_len = 0;
	struct tunneled_msg_tlv *tunneled_tlv=NULL;
	u8 num_payload_tlv = 1;
	u8 proto_type = 2;

	send_pkt_len = sizeof(struct tunneled_msg_tlv) + btm_query_len + 2;
	tunneled_tlv = os_zalloc(send_pkt_len);
	if(tunneled_tlv == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,"memory alloc fail %s", __func__);
		return;
	}

	tunneled_tlv->payload_len = btm_query_len + 1;

	tunneled_tlv->payload[0] = CATEGORY_WNM;
        tunneled_tlv->payload[1] = BSS_TRANSITION_QUERY;

	os_memcpy(&tunneled_tlv->payload[2],btm_query,btm_query_len - 1);

	map_build_and_send_tunneled_message(wapp, peer_addr, proto_type, num_payload_tlv, tunneled_tlv);
}

void map_send_anqp_req_tunneled_message(struct wifi_app *wapp,  const unsigned char *peer_mac_addr, const char *anqp_req, size_t anqp_req_len)
{
	unsigned int send_pkt_len = 0;
	struct tunneled_msg_tlv *tunneled_tlv=NULL;
	u8 num_payload_tlv = 1;
	u8 proto_type = 4;



	send_pkt_len = sizeof(struct tunneled_msg_tlv) + anqp_req_len;

	tunneled_tlv = os_zalloc(send_pkt_len);
	if(tunneled_tlv == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,"memory alloc fail %s", __func__);
		return;
	}

	tunneled_tlv->payload_len = anqp_req_len;

#if 0
	tunneled_tlv->payload[0] = CATEGORY_PUBLIC;
	tunneled_tlv->payload[1] = ACTION_GAS_INIT_REQ;
#endif
	os_memcpy(&tunneled_tlv->payload,anqp_req,anqp_req_len);

	map_build_and_send_tunneled_message(wapp, peer_mac_addr, proto_type, num_payload_tlv, tunneled_tlv);

}

void map_send_wnm_tunneled_message(struct wifi_app *wapp,  const unsigned char *peer_mac_addr, const char *wnm_req, size_t wnm_req_len)
{

	unsigned int send_pkt_len = 0;
	struct tunneled_msg_tlv *tunneled_tlv=NULL;
	u8 num_payload_tlv = 1;
	u8 proto_type = 3;

	send_pkt_len = sizeof(struct tunneled_msg_tlv) + wnm_req_len;
	tunneled_tlv = os_zalloc(send_pkt_len);
	if(tunneled_tlv == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,"memory alloc fail %s", __func__);
		return;
	}

	tunneled_tlv->payload_len = wnm_req_len;
	os_memcpy(&tunneled_tlv->payload[0],wnm_req,wnm_req_len);

	map_build_and_send_tunneled_message(wapp, peer_mac_addr, proto_type, num_payload_tlv, tunneled_tlv);


}

void dump_tunneled(struct tunneled_message_lib *tunneled)
{
	int i;
	DBGPRINT(RT_DEBUG_ERROR,"BSSID:"MACSTR"\n", MAC2STR(tunneled->sta_mac));
	DBGPRINT(RT_DEBUG_ERROR,"Prototype: %d",tunneled->proto_type);
	for(i=0; i< tunneled->num_tunneled_tlv;i++) {
		struct tunneled_msg_tlv *tlv = (struct tunneled_msg_tlv *)&tunneled->tunneled_msg_tlv[i];
		DBGPRINT(RT_DEBUG_ERROR,"RESULT : %d--------------------->START\n", i);
		DBGPRINT(RT_DEBUG_ERROR,"payload_len = %d\n",
				tlv->payload_len);

	}

}


void map_build_and_send_tunneled_message(struct wifi_app *wapp, const unsigned char *sta_mac, u8 proto_type, u8 num_payload_tlv, struct tunneled_msg_tlv *tlv)
{
	struct evt *map_event = NULL;
	struct tunneled_message_lib *tunelled_msg=NULL;
	struct tunneled_msg_tlv *tunelled_tlv=NULL;
	u16 evt_len;
	u16 tunelled_msg_len;
	int i=0;
	u8 *buf = NULL;
	u16 offset = 0;

	if (!tlv) {
		DBGPRINT(RT_DEBUG_ERROR, "%s tlv is NULL!!\n", __func__);
		return;
	}

	tunelled_msg_len = sizeof(struct tunneled_message_lib) + (num_payload_tlv* sizeof(struct tunneled_msg_tlv))
					+ (num_payload_tlv*tlv->payload_len);

	evt_len = sizeof(struct evt) + tunelled_msg_len;
	map_event = os_zalloc(evt_len);
	if(map_event == NULL)
		goto Error;

	map_event->type = WAPP_TUNNELED_MESSAGE;
	map_event->length = tunelled_msg_len;

	tunelled_msg = (struct tunneled_message_lib *)map_event->buffer;

	COPY_MAC_ADDR(&tunelled_msg->sta_mac[0], sta_mac);
	tunelled_msg->num_tunneled_tlv = num_payload_tlv;
	tunelled_msg->proto_type = proto_type;

	buf = (u8 *)tunelled_msg->tunneled_msg_tlv;

	tunelled_tlv = (struct tunneled_msg_tlv *)buf;

	for (i = 0; i < num_payload_tlv; i++) {
		tunelled_tlv[i].payload_len = tlv[i].payload_len;
		os_memcpy(&tunelled_tlv[i].payload[i+offset], &tlv[i].payload, tunelled_tlv[i].payload_len);
		offset += tunelled_tlv[i].payload_len;
	}

	dump_tunneled(tunelled_msg);
	//dump_assoc_status_noti(assoc_notification`);
	if (0 > map_1905_send(wapp, (char *)map_event, evt_len)) {
		DBGPRINT(RT_DEBUG_ERROR, "%s send ch scan msg fail\n", __func__);
	}


	Error:

		if(map_event)
			os_free(map_event);
		if (tlv)
			os_free(tlv);
	return;
}

int map_config_unsuccessful_assoc_policy_msg(
	struct wifi_app *wapp, char *msg_buf)
{
	struct unsuccessful_association_policy *policy = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	policy = (struct unsuccessful_association_policy *)msg_buf;
	DBGPRINT(RT_DEBUG_INFO, "WAPP got unsuccessful asooc policy count");
	wapp->map->assoc_failed_policy.report_unsuccessful_association = policy->report_unsuccessful_association;
	wapp->map->assoc_failed_policy.max_supporting_rate = policy->max_supporting_rate;

	DBGPRINT(RT_DEBUG_ERROR,"### %d %s report_unsuccessful_association %d###\n", __LINE__, __func__, policy->report_unsuccessful_association);
	DBGPRINT(RT_DEBUG_ERROR,"### %d %s max_supporting_rate %d###\n", __LINE__, __func__, wapp->map->assoc_failed_policy.max_supporting_rate);
	return MAP_SUCCESS;
}

#ifdef DFS_CAC_R2
void dump_completion(struct cac_completion_report_lib *completion)
{
	int i;
	//printf("BSSID:"MACSTR"\n", MAC2STR(tunneled->sta_mac));
	DBGPRINT(RT_DEBUG_ERROR,"Radio num: %d",completion->radio_num);
	for(i=0; i< completion->radio_num;i++) {
		struct cac_completion_status_lib *tlv = (struct cac_completion_status_lib *)&completion->cac_completion_status[i];
		DBGPRINT(RT_DEBUG_ERROR,"RESULT : %d--------------------->START\n", i);
		DBGPRINT(RT_DEBUG_ERROR,"BSSID:"MACSTR"\n", MAC2STR(tlv->identifier));
		DBGPRINT(RT_DEBUG_ERROR,"op_class, = %d,channel-%d,cac_status-%d,op_class_num-%d\n",
				tlv->op_class, tlv->channel, tlv->cac_status, tlv->op_class_num);

	}

}
#endif
#endif
void wdev_handle_cac_stop(struct wifi_app *wapp, u32 ifindex, u8 *channel, u8 ret, int radar_status)
{
	struct evt *map_event = NULL;
	struct cac_completion_report_lib *completion_report=NULL;
	struct wapp_dev *wdev = NULL;
#ifdef MAP_R2
#ifdef DFS_CAC_R2
	unsigned char identifier[MAC_ADDR_LEN];
	char cmd[MAX_CMD_MSG_LEN] = {0};
	u8 cac_completion = 1, bw=0;
	u8 trigger_cac=FALSE, cac_done=FALSE, matched = FALSE;
	int  len = 0;
	int i=0;
	int* len_buf = NULL;
	len_buf = &len;
	char evt_buf[4096]={0};
#endif
#endif
	struct ap_dev *ap = NULL;
	u8 op_class = 0;
	u16 evt_len;
	u16 report_len;
	u8 radio_num = 1;
	u8 op_class_num =1;

#ifdef MAP_R2
#ifdef DFS_CAC_R2
	if(ret == TRUE) {
		for(i = 0; i < wapp->map->cac_list.ch_num ;i++)
		{
			if(*channel == wapp->map->cac_list.ch_list[i]) {
				matched = TRUE;
				break;
			}
		}

		if (matched == TRUE) {
			os_get_time(&wapp->map->cac_list.last_cac_time[i]);
		}
		else {
			if (*channel >= 36 && *channel <= 48)
				wapp->map->cac_list.ch_list[i] = 50;
			else
				wapp->map->cac_list.ch_list[i] = *channel;
			wapp->map->cac_list.ch_num++;
			os_get_time(&wapp->map->cac_list.last_cac_time[i]);
			i++;
		}
	}

	if (wapp->map->cac_req_ongoing == TRUE) {
		wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
		if (!wdev) {
			printf("goto ERROR 1\n");
			goto Error;
		}

		//fix for concurrent DFS on different radios
		for(i=0; i< MAX_RADIO_NUM; i++){
			MAP_GET_RADIO_IDNFER(wdev->radio,identifier);
			if(!os_memcmp(&wapp->map->cac_state.radio_state[i].radio_id, identifier, MAC_ADDR_LEN))
			{
				wapp->map->cac_state.radio_state[i].state_cac = CAC_DONE;
				break;
			}
		}

		if (radar_status == FALSE)
			op_class_num = 0;

		//add status report
		if(mapd_get_cac_status_from_driver(wapp, evt_buf, len_buf, cac_completion) == FALSE)
			return;

		report_len = sizeof(struct cac_completion_report_lib) + (radio_num* sizeof(struct cac_completion_status_lib))
						+ (op_class_num*sizeof(struct cac_completion_report_opcap));

		evt_len = sizeof(struct evt) + report_len + *len_buf;
		map_event = os_zalloc(evt_len);
		if(map_event == NULL)
			goto Error;

		completion_report = (struct cac_completion_report_lib *)map_event->buffer;

		completion_report->radio_num = radio_num;

		MAP_GET_RADIO_IDNFER(wdev->radio,&completion_report->cac_completion_status[0].identifier);

		completion_report->cac_completion_status[0].op_class = wapp->map->cac_req.body[wapp->map->cac_radio_ongoing].op_class_num;
		completion_report->cac_completion_status[0].channel = *channel;
		wapp->map->cac_list.op_class = completion_report->cac_completion_status[0].op_class;

		if (radar_status == TRUE) {
			if ((wdev->cac_method == REDUCED_MIMO_CAC || wdev->cac_method == DEDICATED_CAC)
				&& wdev->synA == *channel) {
				completion_report->cac_completion_status[0].cac_status = CAC_FAILURE;	// Error
				//cancel CAC on SynB
				wapp_send_cac_req(wapp, wdev->ifname, WAPP_SET_CAC_STOP, 0);
			} else
				completion_report->cac_completion_status[0].cac_status = RADAR_DETECTED; //RADAR Detected
			completion_report->cac_completion_status[0].op_class_num = 1;
			completion_report->cac_completion_status[0].opcap[0].ch_num = completion_report->cac_completion_status[0].channel;
			completion_report->cac_completion_status[0].opcap[0].op_class = completion_report->cac_completion_status[0].op_class;
		}
		else {
			if (ret == TRUE)
				completion_report->cac_completion_status[0].cac_status = CAC_SUCCESSFUL;
			else
				completion_report->cac_completion_status[0].cac_status = CAC_FAILURE;
			completion_report->cac_completion_status[0].op_class_num = 0;
			completion_report->cac_completion_status[0].opcap[0].ch_num = *channel;
			completion_report->cac_completion_status[0].opcap[0].op_class = 0;
		}


		os_memcpy(&map_event->buffer[report_len], evt_buf, *len_buf);
		map_event->type = WAPP_CAC_COMPLETION_REPORT;
		map_event->length = report_len + *len_buf;
		if (0 > map_1905_send(wapp, (char *)map_event, evt_len)) {
			DBGPRINT(RT_DEBUG_ERROR, "%s send cac stop msg fail\n", __func__);
		}

#ifdef MAP_R4
		if (wapp->map->MapMode == 4)
			os_sleep(0, 3000);
#endif /* MAP_R4 */

		DBGPRINT(RT_DEBUG_ERROR, "%s cac_action %d\n", __func__,
			wapp->map->cac_req.body[wapp->map->cac_radio_ongoing].cac_action);

		// Return to previous state i.e BW and Channel
		if(((wapp->map->cac_req.body[wapp->map->cac_radio_ongoing].cac_action == 1 ||
			ret == FALSE) || (radar_status == TRUE))&& (wapp->map->cac_req.body[wapp->map->cac_radio_ongoing].cac_method != DEDICATED_CAC) &&
				(wapp->map->cac_req.body[wapp->map->cac_radio_ongoing].ch_num == *channel))
		{
			// issue command for different channel case
			if (WMODE_CAP_AX(wdev->wireless_mode)) {
				bw = chan_mon_get_vht_bw_from_op_class(wapp->map->cac_state.radio_state[wapp->map->cac_radio_ongoing].op_class);
#if NL80211_SUPPORT
				wapp_set_vhtbw(wapp, (const char *)wdev->ifname,
						(char *)&bw,
						(size_t)sizeof(bw));
#else
				snprintf(cmd,MAX_CMD_MSG_LEN, "iwpriv %s set VhtBw=%d;", wdev->ifname, bw);
				if (system(cmd) == -1)
					DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
				DBGPRINT(RT_DEBUG_INFO,"%s\n", cmd);
#endif /* NL80211_SUPPORT */

				bw = chan_mon_get_ht_bw_from_op_class(wapp->map->cac_state.radio_state[wapp->map->cac_radio_ongoing].op_class);
#if NL80211_SUPPORT
				wapp_set_htbw(wapp, (const char *)wdev->ifname,
						(char *)&bw,
						(size_t)sizeof(bw));
#else
                                snprintf(cmd,MAX_CMD_MSG_LEN, "iwpriv %s set HtBw=%d;", wdev->ifname, bw);
				if (system(cmd) == -1)
					DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
				DBGPRINT(RT_DEBUG_INFO,"%s\n", cmd);
#endif /* NL80211_SUPPORT */
			} else if(WMODE_CAP_AC(wdev->wireless_mode)) {
				bw = chan_mon_get_vht_bw_from_op_class(wapp->map->cac_state.radio_state[wapp->map->cac_radio_ongoing].op_class);
				printf("in WAPP vht bw %d\n", bw );
#if NL80211_SUPPORT
				wapp_set_vhtbw(wapp, (const char *)wdev->ifname,
							(char *)&bw,
							(size_t)sizeof(bw));
#else
				snprintf(cmd,MAX_CMD_MSG_LEN, "iwpriv %s set VhtBw=%d;", wdev->ifname, bw);
				if (system(cmd) == -1)
					DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif /* NL80211_SUPPORT */
			}else {
				bw = chan_mon_get_ht_bw_from_op_class(wapp->map->cac_state.radio_state[wapp->map->cac_radio_ongoing].op_class);
				printf("in WAPP ht bw %d\n", bw );
#if NL80211_SUPPORT
				wapp_set_htbw(wapp, (const char *)wdev->ifname,
							(char *)&bw,
							(size_t)sizeof(bw));
#else
				snprintf(cmd,MAX_CMD_MSG_LEN, "iwpriv %s set HtBw=%d;", wdev->ifname, bw);
				if (system(cmd) == -1)
					DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif /* NL80211_SUPPORT */
			}

			printf("wdev_handle_cac_stop prev channel: %d , cmd %s\n", wapp->map->cac_state.radio_state[wapp->map->cac_radio_ongoing].prev_ch, cmd);
			wdev_set_ch(wapp, wdev, wapp->map->cac_state.radio_state[wapp->map->cac_radio_ongoing].prev_ch,
				wapp->map->cac_state.radio_state[wapp->map->cac_radio_ongoing].op_class);

		}


		Error:

			if(map_event)
				os_free(map_event);


		for(i=0; i<MAX_RADIO_NUM; i++){
			if(wapp->map->cac_state.radio_state[i].state_cac == CAC_IDLE) {
				trigger_cac = TRUE;
				break;
			}
			else if (wapp->map->cac_state.radio_state[i].state_cac == CAC_DONE)
			{
				cac_done = TRUE;
			}
		}

		if(trigger_cac == TRUE)
			map_start_cac_req(wapp);
		else if(cac_done == TRUE ) {
			wapp->map->cac_req_ongoing = FALSE;
			os_memset(&wapp->map->cac_req, 0, sizeof(struct cac_req));
		}

	}else
#endif
#endif
	{
		wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
		if (!wdev)
			return;

		ap = (struct ap_dev *)wdev->p_dev;

		if(ap != NULL)
			op_class = ap->ch_info.op_class;
#ifdef DFS_CAC_R2
		wapp->map->cac_list.op_class = op_class;
#endif
		if (radar_status == FALSE)
			op_class_num = 0;
		report_len = sizeof(struct cac_completion_report_lib) + (radio_num* sizeof(struct cac_completion_status_lib))
						+ (op_class_num*sizeof(struct cac_completion_report_opcap));
		evt_len = sizeof(struct evt) + report_len;
		map_event = os_zalloc(evt_len);
		if(map_event == NULL)
			return;
		completion_report = (struct cac_completion_report_lib *)map_event->buffer;
		completion_report->radio_num = radio_num;
		MAP_GET_RADIO_IDNFER(wdev->radio,&completion_report->cac_completion_status[0].identifier);
		if (radar_status == FALSE)
		{
			if (ret == TRUE)
				completion_report->cac_completion_status[0].cac_status = CAC_SUCCESSFUL;
			else
				completion_report->cac_completion_status[0].cac_status = CAC_FAILURE;
			completion_report->cac_completion_status[0].op_class_num = 0;
			completion_report->cac_completion_status[0].channel = *channel;
			completion_report->cac_completion_status[0].op_class = op_class;
		} else {
			completion_report->cac_completion_status[0].cac_status = RADAR_DETECTED; //RADAR Detected
			completion_report->cac_completion_status[0].op_class_num = 1;
			completion_report->cac_completion_status[0].channel = *channel;
			completion_report->cac_completion_status[0].opcap[0].ch_num = *channel;
			completion_report->cac_completion_status[0].opcap[0].op_class = completion_report->cac_completion_status[0].op_class;
		}
		map_event->type = WAPP_CAC_COMPLETION_REPORT;
		map_event->length = report_len;
		DBGPRINT(RT_DEBUG_ERROR, "%s\n", __func__);
		hex_dump_dbg("completion_report",map_event->buffer,map_event->length);
		if (0 > map_1905_send(wapp, (char *)map_event, evt_len)) {
			DBGPRINT(RT_DEBUG_ERROR, "%s send cac stop msg fail\n", __func__);
		}
	if(map_event)
		os_free(map_event);
	}
	return;
}


#if 0
enum max_bw {
	BW_20,
	BW_40,
	BW_80,
	BW_160,
	BW_10,
	BW_5,
	BW_8080
};
#endif

struct oper_class_map {
	u8 op_class;
	u8 min_chan;
	u8 max_chan;
	u8 inc;
#ifdef MAP_320BW
	enum openum { BW20, BW40PLUS, BW40MINUS, BW80, BW2160, BW160, BW80P80, BW320 } bw;
#else
	enum openum { BW20, BW40PLUS, BW40MINUS, BW80, BW2160, BW160, BW80P80 } bw;
#endif
};

const static struct oper_class_map global_op_class[] = {
	{81, 1, 13, 1, BW20},
	{82, 14, 14, 1, BW20},
	{83, 1, 9, 1, BW40PLUS},
	{84, 5, 13, 1, BW40MINUS},
	{112, 8, 16, 4, BW20},
	{115, 36, 48, 4, BW20},
	{116, 36, 44, 8, BW40PLUS},
	{117, 40, 48, 8, BW40MINUS},
	{118, 52, 64, 4, BW20},
	{119, 52, 60, 8, BW40PLUS},
	{120, 56, 64, 8, BW40MINUS},
	{121, 100, 144, 4, BW20},
	{122, 100, 140, 8, BW40PLUS},
	{123, 104, 144, 8, BW40MINUS},
	{124, 149, 161, 4, BW20},
	{125, 149, 177, 4, BW20},
	{126, 149, 173, 8, BW40PLUS},
	{127, 153, 177, 8, BW40MINUS},
	{128, 36, 177, 4, BW80},/* >64 and <100 not included*/
	{129, 36, 177, 4, BW160},/* (>64 and <100) or (>=132 and <=144) not included */
	{130, 36, 177, 4, BW80P80},
	{180, 1, 4, 1, BW2160},
#ifdef MAP_6E_SUPPORT
	{131, 1, 233, 4, BW20},
	{132, 3, 227, 8, BW40PLUS},
	{133, 7, 215, 16, BW80},
	{134, 15, 207, 32, BW160},
	{135, 7, 215, 16, BW80P80},
#endif
#ifdef MAP_320BW
	{137, 1, 233, 4, BW320},
#endif
	{0, 0, 0, 0, BW20}
};

u8 grp_bw_160[MAX_BW_160_BLOCK][8] = {
	{36, 40, 44, 48, 52, 56, 60, 64},
	{100, 104, 108, 112, 116, 120, 124, 128},
	{149, 153, 157, 161, 165, 169, 173, 177}
};
u8 grp_bw_80[MAX_BW_80_BLOCK][4] = {
	{36, 40, 44, 48},
	{52, 56, 60, 64},
	{100, 104, 108, 112},
	{116, 120, 124, 128},
	{132, 136, 140, 144},
	{149, 153, 157, 161},
	{165, 169, 173, 177}
};
u8 grp_bw_40[MAX_BW_40_BLOCK][2] = {
	{36, 40},
	{44, 48},
	{52, 56},
	{60, 64},
	{100, 104},
	{108, 112},
	{116, 120},
	{124, 128},
	{132, 136},
	{140, 144},
	{149, 153},
	{157, 161},
	{165, 169},
	{173, 177}
};

#ifdef MAP_320BW
u8 grp_bw_320[MAX_BW_320_BLOCK][16] = {
	{1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61},
	{33, 37, 41, 45, 49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93},
	{65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125},
	{97, 101, 105, 109, 113, 117, 121, 125, 129, 133, 137, 141, 145, 149, 153, 157},
	{129, 133, 137, 141, 145, 149, 153, 157, 161, 165, 169, 173, 177, 181, 185, 189},
	{161, 165, 169, 173, 177, 181, 185, 189, 193, 197, 201, 205, 209, 213, 217, 221}
};
#endif

unsigned char is_channel_in_opclass(unsigned char cac_channel, unsigned char maxbw,
	unsigned char op, unsigned char channel)
{
	u8 *grp_list = NULL;
	int i = 0;
	unsigned char channel_count = 0;
	unsigned char channel_block[8] = {0};

	if (maxbw == BW_160 || op == 129) {
		grp_list = &grp_bw_160[0][0];
		if (cac_channel >= 36 && cac_channel <= 64) {
			os_memcpy(&channel_block[0], (grp_list), 8);
			channel_count = 8;
		} else if (cac_channel >= 100 && cac_channel <= 128) {
			os_memcpy(&channel_block[0], (grp_list + 8), 8);
			channel_count = 8;
		}
	} else {
		if (maxbw == BW_80) {
			grp_list = &grp_bw_80[0][0];
			for (i = 0; i < MAX_BW_80_BLOCK; i++) {
				if (cac_channel == grp_bw_80[i][0] || cac_channel == grp_bw_80[i][1]
					|| cac_channel == grp_bw_80[i][2] || cac_channel == grp_bw_80[i][3]) {
					channel_block[0] = grp_bw_80[i][0];
					channel_block[1] = grp_bw_80[i][1];
					channel_block[2] = grp_bw_80[i][2];
					channel_block[3] = grp_bw_80[i][3];
					channel_count = 4;
					break;
				}
			}
		} else if (maxbw == BW_40) {
			for (i = 0; i < MAX_BW_40_BLOCK; i++) {
				if (cac_channel == grp_bw_40[i][0] || cac_channel == grp_bw_40[i][1]) {
					channel_block[0] = grp_bw_40[i][0];
					channel_block[1] = grp_bw_40[i][1];
					channel_count = 2;
					break;
				}
			}
		} else {
			channel_block[0] = cac_channel;
			channel_count = 1;
		}
	}
	for (i = 0; i < channel_count; i++) {
		if (channel == channel_block[i])
			return 1;
	}
	return 0;
}

/**
 * @brief : mapping function from operating class to channel bandwidth using a
 * global op_class table
 *
 * @param op_class: operating class to be mapped to bandwidth
 *
 * @return : return the bandwidth (to be interpreted as enum max_bw)
 */
int chan_mon_get_bw_from_op_class(u8 op_class)
{
	const struct oper_class_map *op = &global_op_class[0];

	DBGPRINT(RT_DEBUG_ERROR, "Op Class=%d\n", op_class);

	op = &global_op_class[0];
	while (op->op_class && op->op_class != op_class)
			op++;

	if (!op->op_class) {
		DBGPRINT(RT_DEBUG_ERROR,"Op Class not found in Global OpClass Table");
		return -1;
	}
	switch(op->bw)
	{
		case BW20:
			return BW_20;
		case BW40PLUS:
		case BW40MINUS:
			return BW_40;
		case BW80:
			return BW_80;
		case BW160:
			return BW_160;
		case BW80P80:
			return BW_80; /*XXX:We don't have the data for 160Mhz*/
#ifdef MAP_320BW
		case BW320:
			return BW_320;
#endif
		case BW2160:
			//mapd_printf(MSG_ERROR, "11ad opclass not supp");
			//mapd_ASSERT(0);
		default:
			DBGPRINT(RT_DEBUG_ERROR,"opclass not supp");
		//	mapd_ASSERT(0);
	}
	return BW_20;
}

int chan_mon_get_ht_bw_from_op_class(u8 op_class)
{
	const struct oper_class_map *op = &global_op_class[0];

	DBGPRINT(RT_DEBUG_ERROR, "Op Class=%d", op_class);

	op = &global_op_class[0];
	while (op->op_class && op->op_class != op_class)
			op++;

	if (!op->op_class) {
		DBGPRINT(RT_DEBUG_ERROR,"Op Class not found in Global OpClass Table");
		return -1;
	}
	switch(op->bw)
	{
		case BW20:
			return BW_20;
		case BW40PLUS:
		case BW40MINUS:
			return BW_40;
		case BW80:
			return BW_40;
		case BW160:
		case BW80P80:
			//return BW_80; //XXX:We don't have the data for 160Mhz
			return BW_40;
		case BW2160:
			//mapd_printf(MSG_ERROR, "11ad opclass not supp");
			//mapd_ASSERT(0);
		default:
			DBGPRINT(RT_DEBUG_ERROR,"opclass not supp");
		//	mapd_ASSERT(0);
	}
	return BW_20;
}
int chan_mon_get_vht_bw_from_op_class(u8 op_class)
{
	const struct oper_class_map *op = &global_op_class[0];

	DBGPRINT(RT_DEBUG_ERROR, "Op Class=%d", op_class);

	op = &global_op_class[0];
	while (op->op_class && op->op_class != op_class)
			op++;

	if (!op->op_class) {
		DBGPRINT(RT_DEBUG_ERROR,"Op Class not found in Global OpClass Table");
		return -1;
	}
	switch(op->bw)
	{
		case BW20:
		case BW40PLUS:
		case BW40MINUS:
			return bw_20_40;
		case BW80:
			return bw_80;
		case BW160:
			return bw_160;
		case BW80P80:
			return bw_80; //XXX:We don't have the data for 160Mhz
		case BW2160:
			//mapd_printf(MSG_ERROR, "11ad opclass not supp");
			//mapd_ASSERT(0);
		default:
			DBGPRINT(RT_DEBUG_ERROR,"opclass not supp");
		//	mapd_ASSERT(0);
	}
	return bw_20_40;
}

#ifdef MAP_R2
#ifdef DFS_CAC_R2
int channel_req_check_in_avail_list(u8 ch_num, u8 bw, union dfs_zero_wait_msg *av_ch_info)
{
	int i;
	if (bw == BW_160) {
		for (i = 0; i < av_ch_info->aval_channel_list_msg.Bw160TotalChNum; i++) {
			if (ch_num == av_ch_info->aval_channel_list_msg.Bw160AvalChList[i].Channel)
				return 1;
		}
	} else if (bw == BW_80) {
		for (i = 0; i<av_ch_info->aval_channel_list_msg.Bw80TotalChNum; i++) {
			if (ch_num == av_ch_info->aval_channel_list_msg.Bw80AvalChList[i].Channel)
				return 1;
		}
	} else if (bw == BW_40) {
		for (i = 0; i<av_ch_info->aval_channel_list_msg.Bw40TotalChNum; i++) {
			if (ch_num == av_ch_info->aval_channel_list_msg.Bw40AvalChList[i].Channel)
				return 1;
		}
	} else if (bw == BW_20){
		for (i = 0; i<av_ch_info->aval_channel_list_msg.Bw20TotalChNum; i++) {
			if (ch_num == av_ch_info->aval_channel_list_msg.Bw20AvalChList[i].Channel)
				return 1;
		}
	}
	return 0;
}

void map_issue_cac_req(struct wifi_app *wapp)
{
	struct cac_req *cac_req = &wapp->map->cac_req;
	struct wapp_dev *wdev = NULL;
	union dfs_zero_wait_msg *avail_ch_info, msg;
	u8 bw=0, i=0, trigger_cac=FALSE;
	char cmd[MAX_CMD_MSG_LEN] = {0};
	char buf[4096]={0};
	unsigned int len=4096;
	size_t msg_len = 0;

	struct ap_dev *ap = NULL;
	int check_channel = 0;
	int ret;

	for(i=0; i<MAX_RADIO_NUM; i++){
		if(wapp->map->cac_state.radio_state[i].state_cac == CAC_IDLE) {
			wapp->map->cac_radio_ongoing = i;
			trigger_cac = TRUE;
			break;
		}
	}

	if (i == MAX_RADIO_NUM) {
		goto Error;
	}

	if(trigger_cac == FALSE) {
		wapp->map->cac_state.radio_state[i].state_cac = CAC_DONE;
		return;
	}
	wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)cac_req->body[wapp->map->cac_radio_ongoing].identifier);

	if(!wdev) {
		wapp->map->cac_state.radio_state[i].state_cac = CAC_DONE;
		goto Error;
	}
	if (wdev->cac_method != cac_req->body[wapp->map->cac_radio_ongoing].cac_method)	{
		//wapp->map->cac_state.radio_state[i].state_cac = CAC_DONE;//sonal test
		DBGPRINT(RT_DEBUG_ERROR,"cac method not matched wdev mode: %d, cac req mode: %d\n", wdev->cac_method,
			cac_req->body[wapp->map->cac_radio_ongoing].cac_method);
		//goto Error;//sonal test want to do dedicated radio , even if driver is not saying it
	}
	ap = (struct ap_dev *)wdev->p_dev;

	wapp->map->cac_state.radio_state[wapp->map->cac_radio_ongoing].id = wapp->map->cac_radio_ongoing;
	os_memcpy(&wapp->map->cac_state.radio_state[wapp->map->cac_radio_ongoing].radio_id, &cac_req->body[wapp->map->cac_radio_ongoing].identifier,6);
	wapp->map->cac_state.radio_state[wapp->map->cac_radio_ongoing].state_cac = CAC_ONGOING;
	// issue command for different channel case

	DBGPRINT(RT_DEBUG_ERROR,"\n wdev->radio->op_ch is %d",wdev->radio->op_ch);
	DBGPRINT(RT_DEBUG_ERROR,"\n num_of_op_class is %d",ap->ch_info.op_class);
	wapp->map->cac_state.radio_state[wapp->map->cac_radio_ongoing].prev_ch = wdev->radio->op_ch;
	wapp->map->cac_state.radio_state[wapp->map->cac_radio_ongoing].op_class = ap->ch_info.op_class; //current opclass is incorrect


	bw = chan_mon_get_bw_from_op_class(cac_req->body[wapp->map->cac_radio_ongoing].op_class_num);
#if 1
	wdev->cac_method = cac_req->body[wapp->map->cac_radio_ongoing].cac_method;//sonal only for test , hardcode cac method
#endif
	DBGPRINT(RT_DEBUG_ERROR,"\n bw %d,wdev->cac_method %d ",bw, wdev->cac_method);
	if (wdev->cac_method == DEDICATED_CAC) {
		DBGPRINT(RT_DEBUG_ERROR,"\n syn A %d ",wdev->radio->op_ch);
		wdev->synA = wdev->radio->op_ch;
		wdev->synB = 0;
		// Query Available channel list
		//OID_DFS_ZERO_WAIT : QUERY_AVAL_CH_LIST
		avail_ch_info = (union dfs_zero_wait_msg *)buf;
		avail_ch_info->aval_channel_list_msg.Action = QUERY_AVAL_CH_LIST;
		msg_len = sizeof(union dfs_zero_wait_msg);
		wapp_get_channel_avail_ch_list(wapp, (const char *)wdev->ifname,
			(char *)buf, msg_len);
		avail_ch_info = (union dfs_zero_wait_msg *)buf;
		check_channel = channel_req_check_in_avail_list(cac_req->body[wapp->map->cac_radio_ongoing].ch_num,
							bw, avail_ch_info);
		DBGPRINT(RT_DEBUG_ERROR,"\n check_channel%d ",check_channel);
		if (check_channel) {
		// Set the Channel
		//OID - OID_DFS_ZERO_WAIT : MONITOR_CH_ASSIGN
			//SynB
			//Bw etc.
			msg.set_monitored_ch_msg.Action = MONITOR_CH_ASSIGN;
			msg.set_monitored_ch_msg.Channel = cac_req->body[wapp->map->cac_radio_ongoing].ch_num;
			msg.set_monitored_ch_msg.Bw = bw;
			msg.set_monitored_ch_msg.doCAC = 1;
			msg.set_monitored_ch_msg.SyncNum = RDD_DEDICATED_RX;
			msg_len = sizeof(union dfs_zero_wait_msg);
			os_memset(buf, 0, len);
			os_memcpy(buf, &msg, msg_len);
			//hex_dump("dedicated msg", (u8*)&msg,msg_len);//sonal test
			DBGPRINT(RT_DEBUG_ERROR,"\n in WAPP command wdev->ifname %s, ifindex %d ",wdev->ifname, wdev->ifindex);
			wdev->synB = cac_req->body[wapp->map->cac_radio_ongoing].ch_num;
			DBGPRINT(RT_DEBUG_ERROR,"\n wdev->synB %d ",wdev->synB);
			wapp_set_channel_monitor_assign(wapp, (const char *) wdev->ifname,
				(char *)buf, msg_len);

		} else {
			goto Error;
		}
	} else if (wdev->cac_method == REDUCED_MIMO_CAC) {
		DBGPRINT(RT_DEBUG_ERROR,"\n reduced MIMO , need to test later %d",wdev->radio->op_ch);
		wdev->synA = wdev->radio->op_ch;
		wdev->synB = 0;
		// Query Available channel list
		//OID_DFS_ZERO_WAIT : QUERY_AVAL_CH_LIST
		avail_ch_info = (union dfs_zero_wait_msg *)buf;
		avail_ch_info->aval_channel_list_msg.Action = QUERY_AVAL_CH_LIST;
		msg_len = sizeof(union dfs_zero_wait_msg);
		wapp_get_channel_avail_ch_list(wapp, (const char *)wdev->ifname,
			(char *)buf, msg_len);
		avail_ch_info = (union dfs_zero_wait_msg *)buf;
		check_channel = channel_req_check_in_avail_list(cac_req->body[wapp->map->cac_radio_ongoing].ch_num,
							bw, avail_ch_info);
		DBGPRINT(RT_DEBUG_ERROR,"\n check_channel%d ",check_channel);
		if (check_channel) {
		// Set the Channel
		//OID - OID_DFS_ZERO_WAIT : MONITOR_CH_ASSIGN
			//SynB
			//Bw etc.
			msg.set_monitored_ch_msg.Action = MONITOR_CH_ASSIGN;
			msg.set_monitored_ch_msg.Channel = cac_req->body[wapp->map->cac_radio_ongoing].ch_num;
			msg.set_monitored_ch_msg.Bw = bw;
			msg.set_monitored_ch_msg.doCAC = 1;
			msg.set_monitored_ch_msg.SyncNum = RDD_BAND1; // What in case of dedicated radio SPS
			msg_len = sizeof(union dfs_zero_wait_msg);
			os_memset(buf, 0, len);
			os_memcpy(buf, &msg, msg_len);
			wapp_set_channel_monitor_assign(wapp, (const char *) wdev->ifname,
				(char *)buf, msg_len);
			wdev->synB = cac_req->body[wapp->map->cac_radio_ongoing].ch_num;
			DBGPRINT(RT_DEBUG_ERROR,"\n wdev->synB %d ",wdev->synB);
		} else {
			goto Error;
		}
	} else {
		if (WMODE_CAP_AX(wdev->wireless_mode)) {
			bw = chan_mon_get_vht_bw_from_op_class(cac_req->body[wapp->map->cac_radio_ongoing].op_class_num);
#if NL80211_SUPPORT
			wapp_set_vhtbw(wapp, (const char *)wdev->ifname,
					(char *)&bw,
					(size_t)sizeof(bw));
#else
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set VhtBw=%d;", wdev->ifname, bw);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			DBGPRINT(RT_DEBUG_INFO,"%s\n", cmd);
#endif /* NL80211_SUPPORT */

			bw = chan_mon_get_ht_bw_from_op_class(cac_req->body[wapp->map->cac_radio_ongoing].op_class_num);
#if NL80211_SUPPORT
			wapp_set_htbw(wapp, (const char *)wdev->ifname,
					(char *)&bw,
					(size_t)sizeof(bw));
#else
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set HtBw=%d;", wdev->ifname, bw);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
			DBGPRINT(RT_DEBUG_INFO,"%s\n", cmd);
#endif /* NL80211_SUPPORT */
		} else if(WMODE_CAP_AC(wdev->wireless_mode)) {
			bw = chan_mon_get_vht_bw_from_op_class(cac_req->body[wapp->map->cac_radio_ongoing].op_class_num);
#if NL80211_SUPPORT
			wapp_set_vhtbw(wapp, (const char *)wdev->ifname,
					(char *)&bw,
					(size_t)sizeof(bw));
#else
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set VhtBw=%d;", wdev->ifname, bw);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif /* NL80211_SUPPORT */
		} else {
			bw = chan_mon_get_ht_bw_from_op_class(cac_req->body[wapp->map->cac_radio_ongoing].op_class_num);
#if NL80211_SUPPORT
			wapp_set_htbw(wapp, (const char *)wdev->ifname,
					(char *)&bw,
					(size_t)sizeof(bw));
#else
			ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set HtBw=%d;", wdev->ifname, bw);
			if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

			if (system(cmd) == -1)
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
#endif /* NL80211_SUPPORT */
		}
		DBGPRINT(RT_DEBUG_ERROR,"cac start ch-%d",cac_req->body[wapp->map->cac_radio_ongoing].ch_num);
		wdev_set_ch(wapp, wdev, cac_req->body[wapp->map->cac_radio_ongoing].ch_num,
			cac_req->body[wapp->map->cac_radio_ongoing].op_class_num);
	}
Error:

	return;
}


int map_start_cac_req(struct wifi_app *wapp)
{
	//start CAC.

	map_issue_cac_req(wapp);
	return MAP_SUCCESS;
}

void dump_cac_req(	struct cac_req *cac_req)
{
	int i;
	DBGPRINT(RT_DEBUG_ERROR,"\n cac req radio is %d", cac_req->num_radio);
	for(i=0; i< cac_req->num_radio;i++) {
		struct cac_tlv *tlv = (struct cac_tlv *)cac_req->body;
		DBGPRINT(RT_DEBUG_ERROR,"RESULT : %d--------------------->START\n", i);
			DBGPRINT(RT_DEBUG_ERROR,"BSSID:"MACSTR"\n", MAC2STR(tlv->identifier));
			DBGPRINT(RT_DEBUG_ERROR,"Status: %d,%d,%d,%d \n", tlv->cac_action, tlv->cac_method, tlv->ch_num, tlv->op_class_num);
	}

}

int map_receive_cac_req(struct wifi_app *wapp, char *msg_buf, unsigned short msg_len)
{
	int i=0, j=0;
	struct cac_req *cac_req = (struct cac_req *)msg_buf;
	struct wapp_dev *wdev = NULL;
	u8 bw = 0;
	u8 matched = FALSE;
	char cmd[MAX_CMD_MSG_LEN] = {0};

	dump_cac_req(cac_req);

	if(wapp->map->cac_req.num_radio) {
		for(i=0;i<cac_req->num_radio;i++) {
			for(j=0; j<wapp->map->cac_req.num_radio; j++) {
				matched = FALSE;
				if(!os_memcmp(&wapp->map->cac_req.body[j].identifier, cac_req->body[i].identifier, MAC_ADDR_LEN)) {
					matched = TRUE;
					wapp->map->cac_state.radio_state[j].id = j;

					wapp->map->cac_req.body[j].op_class_num = cac_req->body[i].op_class_num;
					wapp->map->cac_req.body[j].ch_num = cac_req->body[i].ch_num;
					wapp->map->cac_req.body[j].cac_method = cac_req->body[i].cac_method;
					wapp->map->cac_req.body[j].cac_action = cac_req->body[i].cac_action;

					if(wapp->map->cac_state.radio_state[j].state_cac == CAC_ONGOING
						&& ((wapp->map->cac_req.body[j].ch_num != cac_req->body[i].ch_num)
						|| ((wapp->map->cac_req.body[j].cac_method != cac_req->body[i].cac_method))
						|| ((wapp->map->cac_req.body[j].op_class_num != cac_req->body[i].op_class_num)))) {


						wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)wapp->map->cac_req.body[j].identifier);
						if(!wdev) {
							//hex_dump("RadioID", &wapp->map.cac_req->body[j].identifier,6);
							return FALSE;
						}
						if (wdev->cac_method != cac_req->body[i].cac_method) {
							printf("CAC method clashed!!! wdev mode: %d, cac req mode: %d\n", wdev->cac_method, cac_req->body[i].cac_method);
							return FALSE;
						}

						wapp_send_cac_req(wapp, wdev->ifname, WAPP_SET_CAC_STOP, 0);

						// Return to previous state i.e BW and Channel
						//if(wapp->map->cac_req->body[j].cac_method == 1)
						{
							// issue command for different channel case
							if (WMODE_CAP_AX(wdev->wireless_mode)) {
								bw = chan_mon_get_vht_bw_from_op_class(wapp->map->cac_state.radio_state[j].op_class);
#if NL80211_SUPPORT
								wapp_set_vhtbw(wapp, (const char *)wdev->ifname,
										(char *)&bw,
										(size_t)sizeof(bw));
#else
								snprintf(cmd,MAX_CMD_MSG_LEN, "iwpriv %s set VhtBw=%d;", wdev->ifname, bw);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n",
								 __func__, __LINE__);
								DBGPRINT(RT_DEBUG_INFO,"%s\n", cmd);
#endif /* NL80211_SUPPORT */

								bw = chan_mon_get_ht_bw_from_op_class(wapp->map->cac_state.radio_state[j].op_class);
#if NL80211_SUPPORT
								wapp_set_htbw(wapp, (const char *)wdev->ifname,
										(char *)&bw,
										(size_t)sizeof(bw));
#else
                                                               snprintf(cmd,MAX_CMD_MSG_LEN, "iwpriv %s set HtBw=%d;", wdev->ifname, bw);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n",
								__func__, __LINE__);
								DBGPRINT(RT_DEBUG_INFO,"%s\n", cmd);
#endif /* NL80211_SUPPORT */
							} else if(WMODE_CAP_AC(wdev->wireless_mode)) {
								bw = chan_mon_get_vht_bw_from_op_class(wapp->map->cac_state.radio_state[j].op_class);
#if NL80211_SUPPORT
								wapp_set_vhtbw(wapp, (const char *)wdev->ifname,
											(char *)&bw,
											(size_t)sizeof(bw));
#else
								snprintf(cmd,MAX_CMD_MSG_LEN, "iwpriv %s set VhtBw=%d;", wdev->ifname, bw);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n",
								__func__, __LINE__);
#endif /* NL80211_SUPPORT */
							}else {
								bw = chan_mon_get_ht_bw_from_op_class(wapp->map->cac_state.radio_state[j].op_class);
#if NL80211_SUPPORT
								wapp_set_htbw(wapp, (const char *)wdev->ifname,
											(char *)&bw,
											(size_t)sizeof(bw));
#else
								snprintf(cmd,MAX_CMD_MSG_LEN, "iwpriv %s set HtBw=%d;", wdev->ifname, bw);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n",
								__func__, __LINE__);
#endif /* NL80211_SUPPORT */
							}

							wdev_set_ch(wapp, wdev, wapp->map->cac_state.radio_state[j].prev_ch,
								wapp->map->cac_state.radio_state[j].op_class);

							//snprintf(cmd,MAX_CMD_MSG_LEN, "iwpriv %s set Channel=%d;", wdev->ifname, wapp->map->cac_state.radio_state[j].prev_ch);
							//TODO: //Prakhar add previous channel info
							//Set previous state of CAC to stop
							//wapp->map->cac_req_ongoing[j]=0;
							//system(cmd);
						}

					}
				}
				wapp->map->cac_state.radio_state[j].state_cac = CAC_IDLE;

			}
			if(matched == FALSE) { //cac request first time
					int num_radio = wapp->map->cac_req.num_radio;
					wapp->map->cac_state.radio_state[num_radio].state_cac = CAC_IDLE;
					wapp->map->cac_state.radio_state[num_radio].id = num_radio;

					wapp->map->cac_req.body[num_radio].op_class_num = cac_req->body[i].op_class_num;
					wapp->map->cac_req.body[num_radio].ch_num = cac_req->body[i].ch_num;
					wapp->map->cac_req.body[num_radio].cac_method = cac_req->body[i].cac_method;
					wapp->map->cac_req.body[num_radio].cac_action = cac_req->body[i].cac_action;
					wapp->map->cac_req.num_radio++;
			}
		}

	}
	else {
		DBGPRINT(RT_DEBUG_ERROR,"\n New CAC Request Received");
//		os_memcpy(&wapp->map->cac_req, msg_buf, msg_len);
		wapp->map->cac_req.num_radio = cac_req->num_radio;
		for(i=0;i<cac_req->num_radio;i++) {
			os_memcpy(&wapp->map->cac_req.body[i].identifier[0],&cac_req->body[i].identifier[0],6);
			wapp->map->cac_req.body[i].op_class_num = cac_req->body[i].op_class_num;
			wapp->map->cac_req.body[i].ch_num = cac_req->body[i].ch_num;
			wapp->map->cac_req.body[i].cac_method = cac_req->body[i].cac_method;
			wapp->map->cac_req.body[i].cac_action = cac_req->body[i].cac_action;
		}

		dump_cac_req(&wapp->map->cac_req);


		wapp->map->cac_state.radio_state[0].state_cac = CAC_IDLE;
	//	wapp->map->cac_req_len = msg_len;
	//	wapp->map->cac_req.num_radio++;

		wapp->map->num_cac_req = wapp->map->cac_req.num_radio;
		wapp->map->cac_radio_ongoing = 0;
		//perform fresh CAC

	}
	wapp->map->cac_req_ongoing = TRUE;

	if (map_start_cac_req(wapp) == MAP_ERROR) {
			DBGPRINT(RT_DEBUG_ERROR,"ERROR: %s %d\n", __func__, __LINE__);
	}

#if 0
	struct cac_req *cac_req = (struct cac_req *)msg_buf;
	int i=0,j=0;

	if(wapp->map->cac_req_ongoing) {
		// To Do
		//If Agent is performing a CAC,and receives a CAC request for a different CAC type, bandwidth, or channel, on a given radio unique identifier, it shall terminate any current CAC and begin a new CAC
			for(i=0;i<cac_req->num_radio;i++) {
				for(j=0; j<wapp->map->cac_req->num_radio; j++) {
					if(!os_memcmp(&wapp->map->cac_req->body[j].identifier, cac_req->body[i].identifier, MAC_ADDR_LEN)) {
						if(wapp->map->cac_state.radio_state[j].state_cac) {
							wapp->map->cac_state.radio_state[j].state_cac = CAC_IDLE;
							wapp->map->cac_radio_ongoing++;

							wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)wapp->map->cac_req->body[j].identifier);
							if(!wdev) {
								printf("%s %d\n", __func__, __LINE__);
								hex_dump("RadioID", &cac_term->term_tlv[i].identifier,6);
								goto Error;
							}

							wapp_send_cac_req(wapp, wdev->ifname, WAPP_SET_CAC_STOP, 0);

							// Return to previous state i.e BW and Channel
							//if(wapp->map->cac_req->body[j].cac_method == 1)
							{
								// issue command for different channel case

								bw = chan_mon_get_bw_from_op_class(wapp->map->cac_state.radio_state[j].op_class);

								if(WMODE_CAP_AC(wdev->wireless_mode)) {
										snprintf(cmd,MAX_CMD_MSG_LEN, "iwpriv %s set VhtBw=%d;", wdev->ifname, bw);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n",
								__func__, __LINE__);
								}else {
										snprintf(cmd,MAX_CMD_MSG_LEN, "iwpriv %s set HtBw=%d;", wdev->ifname, bw);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n",
								__func__, __LINE__);

									}
								snprintf(cmd,MAX_CMD_MSG_LEN, "iwpriv %s set Channel=%d;", wdev->ifname, wapp->map->cac_state.radio_state[j].prev_ch);
								//TODO: //Prakhar add previous channel info
								//Set previous state of CAC to stop
								//wapp->map->cac_req_ongoing[j]=0;
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n",
								__func__, __LINE__);

							}

						}
					}

				}

			}


			//if(wapp->map->cac_radio_ongoing == wapp->map->num_cac_req) //all requests handled
			wapp->map->cac_req_ongoing = CAC_IDLE;

	}
	else

	{

		if(wapp->map->cac_req != NULL)
			os_free(wapp->map->cac_req);

		wapp->map->cac_req = os_zalloc(msg_len);

		if(wapp->map->cac_req == NULL)
			return MAP_ERROR;

		os_memcpy(wapp->map->cac_req, msg_buf, msg_len);
		wapp->map->cac_req_len = msg_len;


		wapp->map->num_cac_req = wapp->map->cac_req->num_radio;
		wapp->map->cac_radio_ongoing=0;
		//perform fresh CAC
		wapp->map->cac_req_ongoing = CAC_ONGOING;


		if (map_start_cac_req(wapp) == MAP_ERROR) {
			printf("%s %d\n", __func__, __LINE__);
		}

	}
#endif

	return MAP_SUCCESS;

}

int map_receive_cac_terminate_req(struct wifi_app *wapp, char *msg_buf,unsigned short msg_len)
{
	struct cac_terminate *cac_term = (struct cac_terminate *)msg_buf;
	u8 i=0, j=0, bw=0;
	struct wapp_dev *wdev = NULL;
	char cmd[MAX_CMD_MSG_LEN] = {0};
	u8 trigger_cac=FALSE, cac_done=FALSE;
	int ret;
#if 0
                char bssid1[6]={0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
		wapp->map->cac_state.radio_state[j].state_cac = CAC_DONE;
		//wapp->map->cac_radio_ongoing++;

		wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)bssid1);
		if(!wdev) {
			printf("%s %d\n", __func__, __LINE__);
			//hex_dump("RadioID", &cac_term->term_tlv[i].identifier,6);
		}
		printf("==>wapp send cac stop\n");

		wapp_send_cac_req(wapp, wdev->ifname, WAPP_SET_CAC_STOP, 0);

		return 0;
#endif
	if(wapp->map->cac_req_ongoing == TRUE) {
	//Terminate CAC
		for(i=0;i<cac_term->num_radio;i++) {
			for(j=0; j<wapp->map->cac_req.num_radio; j++) {
				if(!os_memcmp(&wapp->map->cac_req.body[j].identifier, cac_term->term_tlv[i].identifier, MAC_ADDR_LEN)) {
					if(wapp->map->cac_state.radio_state[j].state_cac != CAC_IDLE) {
						wapp->map->cac_state.radio_state[j].state_cac = CAC_DONE;
						//wapp->map->cac_radio_ongoing++;

						wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)wapp->map->cac_req.body[j].identifier);
						if(!wdev) {
							DBGPRINT(RT_DEBUG_ERROR,"ERROR: %s %d\n", __func__, __LINE__);
							//hex_dump("RadioID", &cac_term->term_tlv[i].identifier,6);
							goto Error;
						}

						wapp_send_cac_req(wapp, wdev->ifname, WAPP_SET_CAC_STOP, 0);

						// Return to previous state i.e BW and Channel
						if(wapp->map->cac_req.body[j].cac_action == 1)
						{
							// issue command for different channel case
							if (WMODE_CAP_AX(wdev->wireless_mode)) {
								bw = chan_mon_get_vht_bw_from_op_class(wapp->map->cac_state.radio_state[j].op_class);
#if NL80211_SUPPORT
								wapp_set_vhtbw(wapp, (const char *)wdev->ifname,
										(char *)&bw,
										(size_t)sizeof(bw));
#else
								ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set VhtBw=%d;",
wdev->ifname, bw);
								if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
									DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n",
__func__, __LINE__);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n",
								__func__, __LINE__);
								DBGPRINT(RT_DEBUG_INFO,"%s\n", cmd);
#endif /* NL80211_SUPPORT */

								bw = chan_mon_get_ht_bw_from_op_class(wapp->map->cac_state.radio_state[j].op_class);
#if NL80211_SUPPORT
								wapp_set_htbw(wapp, (const char *)wdev->ifname,
										(char *)&bw,
										(size_t)sizeof(bw));
#else
								ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set HtBw=%d;",
wdev->ifname, bw);
								if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
									DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n",
__func__, __LINE__);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n",
								__func__, __LINE__);
								DBGPRINT(RT_DEBUG_INFO,"%s\n", cmd);
#endif /* NL80211_SUPPORT */
							} else if(WMODE_CAP_AC(wdev->wireless_mode)) {
								bw = chan_mon_get_vht_bw_from_op_class(wapp->map->cac_state.radio_state[j].op_class);
#if NL80211_SUPPORT
								wapp_set_vhtbw(wapp, (const char *)wdev->ifname,
											(char *)&bw,
											(size_t)sizeof(bw));
#else
								ret = snprintf(cmd, MAX_CMD_MSG_LEN, "iwpriv %s set VhtBw=%d;",
wdev->ifname, bw);
								if (os_snprintf_error(MAX_CMD_MSG_LEN, ret))
									DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n",
__func__, __LINE__);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n",
								__func__, __LINE__);
#endif
							}else {
								bw = chan_mon_get_ht_bw_from_op_class(wapp->map->cac_state.radio_state[j].op_class);
#if NL80211_SUPPORT
								wapp_set_htbw(wapp, (const char *)wdev->ifname,
											(char *)&bw,
											(size_t)sizeof(bw));
#else
								snprintf(cmd,MAX_CMD_MSG_LEN, "iwpriv %s set HtBw=%d;", wdev->ifname, bw);
					if (system(cmd) == -1)
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n",
								__func__, __LINE__);
#endif
							}

							wdev_set_ch(wapp, wdev, wapp->map->cac_state.radio_state[j].prev_ch,
								wapp->map->cac_state.radio_state[j].op_class);
							//snprintf(cmd,MAX_CMD_MSG_LEN, "iwpriv %s set Channel=%d;", wdev->ifname, wapp->map->cac_state.radio_state[j].prev_ch);
							//TODO: //Prakhar add previous channel info
							//Set previous state of CAC to stop
							//wapp->map->cac_req_ongoing[j]=0;
							//system(cmd);

						}

					}
				}

			}

		}

Error:
		for(i=0; i<MAX_RADIO_NUM; i++){
			if(wapp->map->cac_state.radio_state[i].state_cac == CAC_IDLE) {
				trigger_cac = TRUE;
				break;
			}
			else if (wapp->map->cac_state.radio_state[i].state_cac == CAC_DONE)
			{
				cac_done = TRUE;
			}
		}

		if(trigger_cac == TRUE)
			map_start_cac_req(wapp);
		else if(cac_done == TRUE ) {
			wapp->map->cac_req_ongoing = FALSE;
			os_memset(&wapp->map->cac_req, 0, sizeof(struct cac_req));
		}

	}
	return 0;

}
#endif

void ts_bh_set_default_8021q(
#if NL80211_SUPPORT
struct wifi_app *wapp, struct wapp_dev *wdev, unsigned short primary_vid,
			unsigned char pcp
#else
struct wapp_dev *wdev, unsigned short primary_vid, unsigned char pcp
#endif /* NL80211_SUPPORT */
)
{
	int ret;
#if NL80211_SUPPORT
	wapp_set_ts_bh_primary_vid(wapp, (const char *)wdev->ifname,
				(char *)&primary_vid,
				(size_t)sizeof(primary_vid));

	wapp_set_ts_bh_primary_pcp(wapp, (const char *)wdev->ifname,
				(char *)&pcp,
				(size_t)sizeof(pcp));
#else
	char cmd[256];

	os_memset(cmd, 0, sizeof(cmd));
	ret = snprintf(cmd, (sizeof(cmd)), "iwpriv %s set ts_bh_primary_vid=%d;", wdev->ifname, primary_vid);
	if (os_snprintf_error(sizeof(cmd), ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
	DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);

	os_memset(cmd, 0, sizeof(cmd));
	ret = snprintf(cmd, (sizeof(cmd)), "iwpriv %s set ts_bh_primary_pcp=%d;", wdev->ifname, pcp);
	if (os_snprintf_error(sizeof(cmd), ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
	DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
#endif /* NL80211_SUPPORT */
}

void ts_bh_set_all_vid(
#if NL80211_SUPPORT
struct wifi_app *wapp, struct wapp_dev *wdev, unsigned char vlan_num, unsigned short vids[]
#else
struct wapp_dev *wdev, unsigned char vlan_num, unsigned short vids[]
#endif /* NL80211_SUPPORT */
)
{
#if NL80211_SUPPORT
	unsigned char i = 0;

	for (i = 0; i < vlan_num; i++) {
		wapp_set_ts_bh_vid(wapp, (const char *)wdev->ifname,
					(char *)&vids[i],
					(size_t)sizeof(vids[i]));
	}
#else
	char cmd[256];
	unsigned char i = 0;
	int len = 0;

	os_memset(cmd, 0, sizeof(cmd));
	len = snprintf(cmd, sizeof(cmd),"iwpriv %s set ts_bh_vid=", wdev->ifname);
	if (os_snprintf_error(sizeof(cmd), len))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

	for (i = 0; i < vlan_num; i++) {
		len += snprintf(cmd + strlen(cmd),(sizeof(cmd) - strlen(cmd)),"%d,", vids[i]);
	}

	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
	DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
#endif /* NL80211_SUPPORT */
}

void ts_fh_set_vid(
#if NL80211_SUPPORT
struct wifi_app *wapp, struct wapp_dev *wdev, unsigned short vid
#else
struct wapp_dev *wdev, unsigned short vid
#endif /* NL80211_SUPPORT */
)
{
#if NL80211_SUPPORT
	wapp_set_ts_fh_vid(wapp, (const char *)wdev->ifname,
				(char *)&vid,
				(size_t)sizeof(vid));
#else
	char cmd[256];
	int ret;
	os_memset(cmd, 0, sizeof(cmd));

	ret = snprintf(cmd, (sizeof(cmd)), "iwpriv %s set ts_fh_vid=%d;", wdev->ifname, vid);
	if (os_snprintf_error((sizeof(cmd)), ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
	DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
#endif /* NL80211_SUPPORT */
}

void reset_traffic_separation_setting(struct wifi_app *wapp, struct wapp_dev *wdev)
{
	/*clear all traffic separation setting*/
#if NL80211_SUPPORT
	ts_bh_set_default_8021q(wapp, wdev, VLAN_N_VID, 0);
	ts_bh_set_all_vid(wapp, wdev, 0, NULL);
	ts_fh_set_vid(wapp, wdev, VLAN_N_VID);
#else
	ts_bh_set_default_8021q(wdev, VLAN_N_VID, 0);
	ts_bh_set_all_vid(wdev, 0, NULL);
	ts_fh_set_vid(wdev, VLAN_N_VID);
#endif
}

void apply_common_traffic_separation_setting(struct wifi_app *wapp,
	struct wapp_dev *wdev,
	struct ts_common_setting *setting)
{
#if NL80211_SUPPORT
	ts_bh_set_default_8021q(wapp, wdev, setting->primary_vid,
						setting->primary_pcp);
	ts_bh_set_all_vid(wapp, wdev, setting->policy_vid_num,
						setting->policy_vids);
#else
	ts_bh_set_default_8021q(wdev, setting->primary_vid, setting->primary_pcp);
	ts_bh_set_all_vid(wdev, setting->policy_vid_num, setting->policy_vids);
#endif
}


int map_traffic_separarion_setting_msg(
	struct wifi_app *wapp, char *msg_buf)
{
	struct wapp_dev *wdev = NULL;
	struct ts_setting *setting = (struct ts_setting*)msg_buf;
	unsigned char i = 0;
	struct dl_list *dev_list;
	struct ts_fh_bss_setting *fh_setting = &setting->fh_bss_setting;

	dev_list = &wapp->dev_list;

	dl_list_for_each (wdev, dev_list, struct wapp_dev, list) {
		/*1. reset default for all interface*/
		reset_traffic_separation_setting(wapp, wdev);

		/*2. apply the common setting for all interface*/
		apply_common_traffic_separation_setting(wapp, wdev, &setting->common_setting);
	}

	/*3. apply fh bss setting for all fh interface*/
	DBGPRINT(RT_DEBUG_ERROR, "fh setting inf_num=%d\n", fh_setting->itf_num);
	for (i = 0; i < fh_setting->itf_num; i++) {
		DBGPRINT(RT_DEBUG_ERROR, "fh setting inf[%d] "MACSTR" vid=%d\n",
			i, MAC2STR(fh_setting->fh_configs[i].itf_mac),
			fh_setting->fh_configs[i].vid);
		wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, fh_setting->fh_configs[i].itf_mac,
			WAPP_DEV_TYPE_AP);

		if (!wdev)
			continue;
#if NL80211_SUPPORT
		ts_fh_set_vid(wapp, wdev, fh_setting->fh_configs[i].vid);
#else
		ts_fh_set_vid(wdev, fh_setting->fh_configs[i].vid);
#endif
	}

	return MAP_SUCCESS;
}

void transparent_set_vid(
#if NL80211_SUPPORT
struct wifi_app *wapp, struct wapp_dev *wdev,
			struct trans_vlan_config *config
#else
struct wapp_dev *wdev, struct trans_vlan_config *config
#endif /*NL80211_SUPPORT*/
)
{
#if NL80211_SUPPORT
	unsigned char i = 0;

	for (i = 0; i < config->trans_vid_num; i++) {
		wapp_set_transparent_vid(wapp, (const char *)wdev->ifname,
					(char *)&config->vids[i],
					(size_t)sizeof(config->vids[i]));
	}
#else
	char cmd[512];
	unsigned char i = 0;
	int len = 0;

	os_memset(cmd, 0, sizeof(cmd));
	len = snprintf(cmd,sizeof(cmd), "iwpriv %s set transparent_vid=", wdev->ifname);
	if (os_snprintf_error(sizeof(cmd), len))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

	for (i = 0; i < config->trans_vid_num; i++) {
		len += snprintf(cmd + strlen(cmd),(sizeof(cmd) - strlen(cmd)),"%d,", config->vids[i]);
	}

	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call fail\n", __func__, __LINE__);
	printf("%s\n", cmd);
#endif /*NL80211_SUPPORT*/
}

int map_transparent_vlan_setting_msg(
	struct wifi_app *wapp, char *msg_buf)
{
	struct wapp_dev *wdev = NULL;
	struct trans_vlan_setting *trans_vlan = (struct trans_vlan_setting*)msg_buf;
	unsigned char i = 0;

	for (i = 0; i < trans_vlan->apply_itf_num; i++) {
		wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, &trans_vlan->apply_itf_mac[i * ETH_ALEN],
			WAPP_DEV_TYPE_AP);
		if (!wdev)
			wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, &trans_vlan->apply_itf_mac[i * ETH_ALEN],
				WAPP_DEV_TYPE_STA);
		if (!wdev)
			continue;

#if NL80211_SUPPORT
		transparent_set_vid(wapp, wdev, &trans_vlan->trans_vlan_configs);
#else
		transparent_set_vid(wdev, &trans_vlan->trans_vlan_configs);
#endif /* NL80211_SUPPORT */
	}

	return MAP_SUCCESS;
}


int map_service_prioritization_rule_msg(
	struct wifi_app *wapp, char *msg_buf, unsigned short msg_len)
{
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list;

	dev_list = &wapp->dev_list;
	dl_list_for_each (wdev, dev_list, struct wapp_dev, list) {
		driver_wext_set_sp_rule(wapp->drv_data, wdev->ifname, msg_buf, msg_len);
	}

	return MAP_SUCCESS;
}


int map_service_prioritization_dscp_tbl_msg(
	struct wifi_app *wapp, char *msg_buf, unsigned short msg_len)
{
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list;

	dev_list = &wapp->dev_list;
	dl_list_for_each (wdev, dev_list, struct wapp_dev, list) {
		driver_wext_set_dscp_tbl(wapp->drv_data, wdev->ifname, msg_buf, msg_len);
	}

	return MAP_SUCCESS;
}
#endif

void map_config_state_check(void *eloop_data, void *user_ctx)
{
	struct wifi_app *wapp = (struct wifi_app*) eloop_data;
	struct wapp_radio *ra = NULL;
	wapp_device_status *device_status = NULL;
	int i = 0;
#ifdef MAP_R3
	struct dpp_authentication *auth = NULL;
#endif /* MAP_R3 */
	if (!wapp) {
		DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"Error! wapp is NULL.\n");
		return;
	}

#ifdef MAP_R3
	if(wapp->map && (wapp->map->map_version == DEV_TYPE_R3)
			&& wapp->dpp && (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE)) {
		/* If autoconfig timeout on enrollee mode
		 * clear the existing state */
		auth = wapp_dpp_get_first_auth(wapp);
		if(!auth) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s auth instance not found\n", __func__);
		}
		else {
			wapp_dpp_cancel_timeouts(wapp, auth);
			dpp_auth_deinit(auth);
		}
	}
#endif /* MAP_R3 */

	if (wapp->map && ((wapp->map->conf == MAP_CONN_STATUS_CONF)
		|| (wapp->map->MapMode == 4))) {
		DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"Error! %s timeout, auto config done \n", __func__);
		return;
	}
	else {
		DBGPRINT(RT_DEBUG_ERROR, AUTO_CONFIG_PREX"timeout!!! auto config fail!!!\n");
		device_status = &wapp->map->device_status;
		for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
			ra = &wapp->radio[i];
			MAP_CONF_STATE_SET((&ra->conf_state), MAP_CONF_UNCONF);
		}
		if(wapp->map->TurnKeyEnable) {
			map_reset_conf_sm(wapp->map);
		}
		wapp->map->bh_link_ready = 0;
		if(wapp->map->is_agnt)
			wapp->map->ctrler_found = 0;
		device_status->status_fhbss = STATUS_FHBSS_UNCONFIGURED;
		device_status->status_bhsta = STATUS_BHSTA_UNCONFIGURED;
		wapp_send_1905_msg(
		wapp,
		WAPP_DEVICE_STATUS,
		sizeof(wapp_device_status),
		(char *)device_status);
		wapp_soft_reset_scan_states(wapp);
	}
}

int map_build_wts_config(
	struct wifi_app *wapp, char *evt_buf, struct set_config_bss_info *bss_config)
{
	struct evt *map_event = NULL;
	int send_pkt_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_WTS_CONFIG;
	map_event->length = sizeof(struct set_config_bss_info) * MAX_SET_BSS_INFO_NUM;
	os_memcpy(map_event->buffer, bss_config, map_event->length);

	send_pkt_len = sizeof(*map_event) + map_event->length;
	return send_pkt_len;
}

int map_handle_get_wts_config(struct wifi_app *wapp, char *evt_buf, int* len_buf)
{
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	u8 bss_config_num = 0;
	struct set_config_bss_info bss_config[MAX_SET_BSS_INFO_NUM];

	wapp_read_wts_map_config(wapp, MAPD_WTS_FILE,
		bss_config, MAX_SET_BSS_INFO_NUM, &bss_config_num);

	send_pkt_len = map_build_wts_config(wapp, evt_buf, bss_config);
	if (send_pkt_len <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ptk size < 0 \n", __func__, __LINE__);
		return MAP_ERROR;
	}
	*len_buf = send_pkt_len;
	return MAP_SUCCESS;
}
