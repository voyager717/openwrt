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
 * Copyright  (C) 2020-2021  MediaTek Inc. All rights reserved.
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
#include <linux/netlink.h>

#include "wapp_cmm.h"
#include "qos.h"
#include "driver_wext.h"

#ifdef QOS_R1

#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN	6
#endif

#define QOS_MAP_RESET 0xFF

#define CMD_BUF_LEN 1024
#define SEND_BUF_LEN 1024

#define MIN(a, b) ((a < b) ? (a) : (b))

#define MAX_PAYLOAD		1562

static int netlink_sock = -1;
static int netlink_pid = 0;
struct nlmsghdr *g_nlmsghdr = NULL;

unsigned char cmd_buf[CMD_BUF_LEN] = {0};
unsigned char sendbuf[SEND_BUF_LEN] = {0};

struct qos_wifi_config qos_config;

unsigned char mscs_active_sta_cnt = 0;
struct dl_list mscs_cfg_list;

u8 ra_onoff = RADIO_ON;

#ifdef QOS_R2
u8 WFA_OUI[] = {0x50, 0x6F, 0x9A};

#define str_equal(s1, s2) ((strlen(s1) == strlen(s2)) && (strncmp(s1, s2, strlen(s2)) == 0))

unsigned char scs_active_sta_cnt = 0;
struct dl_list scs_cfg_list;

unsigned char dscp_policy_cnt = 0;
struct dl_list dscp_policy_list;

#define IPV4STR "%d.%d.%d.%d"
#define IPV6STR "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"

#define NIP4(addr) \
    ((unsigned char *)&addr)[0], \
    ((unsigned char *)&addr)[1], \
    ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[3]

#define NIP6(addr) \
    ntohs(addr[0]), \
    ntohs(addr[1]), \
    ntohs(addr[2]), \
    ntohs(addr[3]), \
    ntohs(addr[4]), \
    ntohs(addr[5]), \
    ntohs(addr[6]), \
    ntohs(addr[7])
#endif

struct wapp_ctrl_cmd qos_cmd[] = {
	{"reset_default",		qos_cmd_reset_default,	"reset default (unconf)"},
	{"qos_ap_set_config",		qos_cmd_set_config,	"set ap configuration"},
	{"qos_ap_commit_config",	qos_cmd_commit_config,	"commit ap configuration"},
	{"show_qos_config",		qos_cmd_show_config,	"show qos configuration"},
	{"help",			qos_cmd_show_help,	"show this help"},
	{NULL,				qos_cmd_show_help,	NULL}
};

struct qos_map qosmap[] = {
	/*num,		dscp exception,						dscp range					*/
	{2,	{[0]={53, 2}, [1]={22, 6}},	{{8, 15}, {0, 7}, {255, 255}, {16, 31}, {32, 39}, {255, 255}, {40, 47}, {255, 255}}},
	{0,	{{0}},				{{8, 15}, {0, 7}, {255, 255}, {16, 31}, {32, 39}, {255, 255}, {40, 47}, { 48,  63}}}
};

static int wapp_drv_send_netlink_msg(const unsigned char *message, int len, unsigned int dst_group);
int wapp_drv_send_event(unsigned char event, unsigned char *buf, int len);
int wapp_drv_send_up_tuple_expired_notify(
		struct wifi_app *wapp, const char *ifname, char *buf, u32 buflen);
static int set_qosmap_ie(struct wifi_app *wapp, const char *ifname, u8 setid);
void qos_teardown_client(struct wifi_app *wapp, const char *ifname, u8 *climac);
void clean_mscs_configuration();
#ifdef QOS_R2
void clean_scs_configuration();
void clean_dscp_policy();
void parse_dscp_policy(char *str);
void parse_dscp_policy_req(struct wifi_app * wapp, char *str);
void parse_scs_response(struct wifi_app * wapp, const char *ifname, char *str);
int get_pmk_by_peer_mac(struct wifi_app *wapp, const char *ifname, u8 *peermac);
#endif

static inline u8 get_band_by_channel(u8 Channel)
{
	if (Channel <= 14)
		return BD_24G;
	else if (Channel < 100)
		return BD_5GL;
	else
		return BD_5GH;
}

void get_config_ifname(struct wifi_app *wapp, char *ifname, int len)
{
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list = NULL;

	if (!wapp || !ifname)
		return;

	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
		if (wdev->radio && (wdev->dev_type == WAPP_DEV_TYPE_AP)) {
			if (get_band_by_channel(wdev->radio->op_ch) !=
				get_band_by_channel(qos_config.channel))
				continue;

			memset(ifname, 0, len);
			memcpy(ifname, wdev->ifname, MIN(strlen(wdev->ifname), len));
		}
	}
}

int qos_cmd_reset_default(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	char ifname[16] = {0};

	DBGPRINT(RT_DEBUG_ERROR, "%s\n", __func__);
	if (!argv[1]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}
	get_config_ifname(wapp, ifname, sizeof(ifname));
	set_qosmap_ie(wapp, ifname, 0xFF);
	wapp_drv_send_event(NL_SET_QOS_MAP, NULL, 0);
	wapp_drv_send_event(NL_RESET_QOS_CONFIG, NULL, 0);
	memset(&qos_config, 0, sizeof(qos_config));

	clean_mscs_configuration();
#ifdef QOS_R2
	clean_scs_configuration();
	clean_dscp_policy();
#endif

	return 0;
}

u8 get_wireless_mode(char *strMode)
{
	if (os_strncmp(strMode, "11ng", 4) == 0) {
		return PHY_11BGN_MIXED;
	} else if (os_strncmp(strMode, "11bng", 5) == 0) {
		return PHY_11BGN_MIXED;
	} else if (os_strncmp(strMode, "11na", 4) == 0) {
		return PHY_11AN_MIXED;
	}

	return PHY_MODE_MAX;
}

void parse_macstr(char *str, u8 *macaddr)
{
	u8 mac[MAC_ADDR_LEN];
	char *token;
	int i;

	if (!str || !macaddr) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: Invalid argument\n", __func__);
		return ;
	}

	i = 0;
	token = strtok(str, ":");

	while (token != NULL) {
		AtoH(token, (char *) &mac[i], 1);
		i++;
		if (i >= MAC_ADDR_LEN)
			break;
		token = strtok(NULL, ":");
	}

	memcpy(macaddr, mac, sizeof(mac));
	DBGPRINT(RT_DEBUG_TRACE, "%s: "MACSTR"\n", __func__, MAC2STR(macaddr));
}

static int set_qosmap_ie(struct wifi_app *wapp, const char *ifname, u8 setid)
{
	struct qosmap_element *ie = NULL;
	char *buf, *pos;
	int ie_len = 0, varlen = 0, i = 0;
	int ret = 0, exc_cnt = 0;
	struct dscp_exception *pdscp_ex = NULL;
	struct dscp_range *pdscp_rg = NULL;

	if (setid < 2) {
		exc_cnt = qosmap[setid].num;
		pdscp_ex = qosmap[setid].dscp_ex;
		pdscp_rg = qosmap[setid].dscp_rg;

		DBGPRINT(RT_DEBUG_ERROR, "setid:%d, exc_cnt:%d\n", setid, exc_cnt);

		varlen = exc_cnt * 2;
		buf = os_zalloc(sizeof(*ie) + varlen);
		if (!buf) {
			DBGPRINT(RT_DEBUG_ERROR, ("memory is not availale"));
			return -1;
		}

		ie = (struct qosmap_element *)buf;

		ie->eid = IE_QOS_R1_MAP_SET;
		ie_len += 1;

		pos = (char *)ie->dscp_range;

		/* dscp exception */
		for (i = 0; i < exc_cnt && i < MAX_EXCEPT_CNT; i++) {
			*pos = pdscp_ex[i].dscp;
			*(pos+1) = pdscp_ex[i].up;;
			pos += 2;
			ie_len += 2;
		}

		/* dscp range */
		for (i = 0; i < UP_MAX; i++) {
			*pos = pdscp_rg[i].low;
			*(pos+1) = pdscp_rg[i].high;
			pos += 2;
			ie_len += 2;
		}

		ie->length = ie_len - 1;
		ie_len += 1;
	} else {
		/*reset QoS Map setting.*/
		buf = os_zalloc(sizeof(*ie));
		if (!buf) {
			DBGPRINT(RT_DEBUG_ERROR, ("memory is not availale"));
			return -1;
		}
		ie = (struct qosmap_element *)buf;

		ie->eid = IE_QOS_R1_MAP_SET;
		ie->length = 0;
		ie_len = 2;
	}

	ret = wapp_set_ie(wapp, ifname, buf, ie_len);
	os_free(buf);

	return ret;
}
#ifdef QOS_R2

/*
* set radio on/off.
* Channel: set the specified radio on/off by channel,
* if channel=0, set all radio on/off.
*/
static inline void radio_on_off(struct wifi_app *wapp, u8 Channel, u8 onoff)
{
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list = NULL;

	if (!wapp)
		return;

	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
		if (wdev->radio) {
			if (Channel &&
				(get_band_by_channel(wdev->radio->op_ch) !=
				get_band_by_channel(Channel)))
				continue;

			wdev_set_radio_onoff(wapp, wdev, onoff);
		}
	}
}
#endif
int qos_cmd_set_config(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
/*	struct evt *wapp_event;*/
	char *keyword, *value, wirelessmode;
	int exc_cnt = 0;
	struct dscp_exception *pdscp_ex = NULL;
	struct dscp_range *pdscp_rg = NULL;
	u8 mac[MAC_ADDR_LEN] = {0};
	char ifname[16] = {0};
	u8 dft_qos_map_idx = 0;
#ifdef QOS_R2
	int ret = 0;
	char cmd[256] = {0};
#endif

	if (!argv[1]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	DBGPRINT(RT_DEBUG_TRACE, "%s, %s\n", __func__, argv[1]);

	get_config_ifname(wapp, ifname, sizeof(ifname));

	keyword = strtok(argv[1], "=");
	if (!keyword) {
		DBGPRINT(RT_DEBUG_ERROR, "%s(%d), strtok got NULL.\n", __func__, __LINE__);
		return WAPP_UNEXP;
	}
	value = strtok(NULL, "=");
	if (!value) {
		DBGPRINT(RT_DEBUG_ERROR, "%s(%d), strtok got NULL.\n", __func__, __LINE__);
		return WAPP_UNEXP;
	}
	if (os_strncmp(keyword, "radio", 5) == 0) {
		DBGPRINT(RT_DEBUG_ERROR, "set radio: %s\n", value);
#ifdef QOS_R2
		if (os_strncmp(value, "off", 3) == 0) {
			ra_onoff = RADIO_OFF;
			radio_on_off(wapp, 0, RADIO_OFF);
		} else {
			qos_config.update = 1;
			qos_cmd_commit_config(wapp, iface, 0, 0);
		}
#endif
	} else if (os_strncmp(keyword, "channel", 7) == 0) {
		qos_config.channel = strtol(value, 0, 10);
		if (errno != 0)
			DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
		qos_config.update = 1;
	} else if (os_strncmp(keyword, "mode", 4) == 0) {
		wirelessmode = get_wireless_mode(value);
		if (wirelessmode != PHY_MODE_MAX) {
			qos_config.mode = wirelessmode;
			qos_config.update = 1;
		}
	} else if (os_strncmp(keyword, "ssid", 4) == 0) {
		memset(qos_config.ssid, 0, sizeof(qos_config.ssid));
		memcpy(qos_config.ssid, value, MIN(strlen(value), 32));
		qos_config.update = 1;
	} else if (os_strncmp(keyword, "keymgnt", 7) == 0) {
		memset(qos_config.keymgnt, 0, sizeof(qos_config.keymgnt));
		memcpy(qos_config.keymgnt, value, MIN(strlen(value), 16));
		qos_config.update = 1;
	} else if (os_strncmp(keyword, "psk", 3) == 0) {
		memset(qos_config.psk, 0, sizeof(qos_config.psk));
		memcpy(qos_config.psk, value, MIN(strlen(value), 16));
		qos_config.update = 1;
	} else if (os_strncmp(keyword, "sta_mac", 7) == 0) {
		parse_macstr(value, qos_config.stamac);
		if (qos_config.qosmap_set == 1 || qos_config.qosmap_set == 2)
			dft_qos_map_idx = qos_config.qosmap_set - 1;
		else
			dft_qos_map_idx = 0;
		exc_cnt = qosmap[dft_qos_map_idx].num;
		pdscp_ex = qosmap[dft_qos_map_idx].dscp_ex;
		pdscp_rg = qosmap[dft_qos_map_idx].dscp_rg;
		wapp_send_qos_map_configure_frame(wapp, ifname,
			qos_config.channel, qos_config.stamac, pdscp_ex, exc_cnt, pdscp_rg);
	} else if (os_strncmp(keyword, "qos_map_set", 11) == 0) {
		qos_config.qosmap_set = strtol(value, 0, 10);
		if (errno != 0)
			DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
		if (ra_onoff == RADIO_OFF) {
			qos_config.update = 1;
		} else if (qos_config.qosmap_set == 1 || qos_config.qosmap_set == 2) {
			dft_qos_map_idx = qos_config.qosmap_set - 1;
			set_qosmap_ie(wapp, ifname, dft_qos_map_idx);
			wapp_drv_send_event(NL_SET_QOS_MAP,
				(unsigned char *)&qosmap[dft_qos_map_idx], sizeof(struct qos_map));
		}
	} else if (os_strncmp(keyword, "teardownclient", 14) == 0) {
		DBGPRINT(RT_DEBUG_ERROR, "%s(), %s:%s\n", __func__, keyword, value);
		parse_macstr(value, mac);
		qos_teardown_client(wapp, ifname, mac);
#ifdef QOS_R2
	} else if (os_strncmp(keyword, "dscppolicycapa", 14) == 0) {
		qos_config.dscppolicycapa = strtol(value, 0, 10);
		if (errno != 0)
			DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
		if (ra_onoff == RADIO_OFF) {
			qos_config.update = 1;
		} else {
			os_memset(cmd, 0, sizeof(cmd));
			ret = snprintf(cmd, sizeof(cmd), "iwpriv %s set DSCPPolicyEnable=%d;",
				ifname, qos_config.dscppolicycapa);
			if (os_snprintf_error(sizeof(cmd), ret))
				goto error;
			ret = system(cmd);
			if (ret < 0)
				DBGPRINT(RT_DEBUG_ERROR,"%s, command failed.\n", cmd);
			DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
		}
	} else if (os_strncmp(keyword, "qosmapcapa", 10) == 0) {
		qos_config.qosmapcapa = strtol(value, 0, 10);
		if (errno != 0)
			DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
		if (ra_onoff == RADIO_OFF) {
			qos_config.update = 1;
		} else {
			os_memset(cmd, 0, sizeof(cmd));
			ret = snprintf(cmd, sizeof(cmd), "iwpriv %s set QosMapCapa=%d;", ifname, qos_config.qosmapcapa);
			if (os_snprintf_error(sizeof(cmd), ret))
				goto error;
			ret = system(cmd);
			if (ret < 0)
				DBGPRINT(RT_DEBUG_ERROR,"%s, command failed.\n", cmd);
			DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
		}
	} else if (os_strncmp(keyword, "DSCPPolicies", 12) == 0) {
		parse_dscp_policy(value);
	} else if (os_strcasecmp(keyword, "DSCPPolicyReq") == 0) {
		parse_dscp_policy_req(wapp, value);
	} else if (os_strcasecmp(keyword, "SCSResp") == 0) {
		parse_scs_response(wapp, ifname, value);
	} else if (os_strncmp(keyword, "PMK", 3) == 0) {
		parse_macstr(value, mac);
		get_pmk_by_peer_mac(wapp, ifname, mac);
	} else if (os_strncmp(keyword, "ReqCtrlReset", 12) == 0) {
		qos_config.RequestCtrlReset = strtol(value, 0, 10);
		if (errno != 0)
			DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
#endif
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "unknow param:%s\n", value);
	}

	return WAPP_SUCCESS;
error:
	DBGPRINT(RT_DEBUG_ERROR, "%s(), error happen, ret=%d\n", __func__, ret);
	return WAPP_UNEXP;
}

int qos_cmd_commit_config(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	int ret = 0;
	char cmd[256] = {0}, ifname[16] = {0};

	if (!qos_config.update)
		return WAPP_SUCCESS;

#ifdef QOS_R2
	if (ra_onoff == RADIO_OFF) {
		radio_on_off(wapp, qos_config.channel, RADIO_ON);
		ra_onoff = RADIO_ON;
		sleep(5);
	}
#endif

	get_config_ifname(wapp, ifname, sizeof(ifname));

	os_memset(cmd, 0, sizeof(cmd));
	ret = snprintf(cmd, sizeof(cmd), "iwpriv %s set Channel=%d;", ifname, qos_config.channel);
	if (os_snprintf_error(sizeof(cmd), ret))
		goto error;
	ret = system(cmd);
	if (ret < 0)
		DBGPRINT(RT_DEBUG_ERROR,"%s, command failed.\n", cmd);

	DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);

	if (qos_config.dscppolicycapa) {
		os_memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd),
			"iwpriv %s set DSCPPolicyEnable=%d;",
			ifname, qos_config.dscppolicycapa);
		if (os_snprintf_error(sizeof(cmd), ret))
			goto error;
		ret = system(cmd);
		if (ret < 0)
			DBGPRINT(RT_DEBUG_ERROR, "%s, command failed.\n", cmd);
		DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);
	}

	if (qos_config.qosmapcapa) {
		os_memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd),
			"iwpriv %s set QosMapCapa=%d;",
			ifname, qos_config.qosmapcapa);
		if (os_snprintf_error(sizeof(cmd), ret))
			goto error;
		ret = system(cmd);
		if (ret < 0)
			DBGPRINT(RT_DEBUG_ERROR, "%s, command failed.\n", cmd);
		DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);
	}

	if (qos_config.mode != PHY_MODE_MAX) {
		os_memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd), "iwpriv %s set WirelessMode=%d;", ifname, qos_config.mode);
		if (os_snprintf_error(sizeof(cmd), ret))
			goto error;
		ret = system(cmd);
		if (ret < 0)
			DBGPRINT(RT_DEBUG_ERROR,"%s, command failed.\n", cmd);
		DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);
	}

	if (os_strcasecmp(qos_config.keymgnt, "SAE") == 0) {
		os_memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd), "iwpriv %s set AuthMode=WPA3PSK;", ifname);
		if (os_snprintf_error(sizeof(cmd), ret))
			goto error;
		ret = system(cmd);
		if (ret < 0)
			DBGPRINT(RT_DEBUG_ERROR,"%s, command failed.\n", cmd);
		DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);

		os_memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd), "iwpriv %s set EncrypType=AES;", ifname);
		if (os_snprintf_error(sizeof(cmd), ret))
			goto error;
		ret = system(cmd);
		if (ret < 0)
			DBGPRINT(RT_DEBUG_ERROR,"%s, command failed.\n", cmd);
	}
	if (strlen(qos_config.psk)) {
		os_memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd), "iwpriv %s set WPAPSK=%s;", ifname, qos_config.psk);
		if (os_snprintf_error(sizeof(cmd), ret))
			goto error;
		ret = system(cmd);
		if (ret < 0)
			DBGPRINT(RT_DEBUG_ERROR,"%s, command failed.\n", cmd);
		DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);
	}

	if (strlen(qos_config.ssid)) {
		os_memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd), "iwpriv %s set SSID=%s;", ifname, qos_config.ssid);
		if (os_snprintf_error(sizeof(cmd), ret))
			goto error;
		ret = system(cmd);
		if (ret < 0)
			DBGPRINT(RT_DEBUG_ERROR,"%s, command failed.\n", cmd);
		DBGPRINT(RT_DEBUG_ERROR, "%s\n", cmd);
	}

	if (qos_config.qosmap_set == 1 || qos_config.qosmap_set == 2) {
		set_qosmap_ie(wapp, ifname, qos_config.qosmap_set - 1);
		wapp_drv_send_event(NL_SET_QOS_MAP,
			(unsigned char *)&qosmap[qos_config.qosmap_set - 1], sizeof(struct qos_map));
	}

	qos_config.update = 0;

	return WAPP_SUCCESS;
error:
	DBGPRINT(RT_DEBUG_ERROR, "%s(), error happen, ret=%d\n", __func__, ret);
	return WAPP_UNEXP;
}

int qos_cmd_show_config(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	char buf[1024] = {0};
	u8 i = 0;
	struct mscs_configuration *mscscfg = NULL, *mscscfg_tmp = NULL;
	struct classifier_parameter *pConf = NULL;
#ifdef QOS_R2
	u8 j = 0, k = 0;
	struct scs_configuration *scscfg = NULL, *scscfg_tmp = NULL;
	struct scs_classifier_parameter *pScsTuple = NULL;
	ptclas_element ptclas = NULL;
	struct dscp_policy_db *dpolicy = NULL, *dpolicy_tmp = NULL;
#endif
	int buflen, ret = 1, offset = 0;

	buflen = sizeof(buf);
	memset(buf, 0, buflen);
	if (mscs_active_sta_cnt) {
		ret = snprintf(buf+offset, buflen-offset, "[wappd]---------MSCS Configuration---------\n");
		if (os_snprintf_error(buflen-offset, ret))
			goto exit;
		offset += ret;
		ret = snprintf(buf+offset, buflen-offset, "mscs active sta cnt: %d\n", mscs_active_sta_cnt);
		if (os_snprintf_error(buflen-offset, ret))
			goto exit;
		offset += ret;
		ret = snprintf(buf+offset, buflen-offset,
			" NO.        sta_mac	 request_type up_bitmap up_limit");
		if (os_snprintf_error(buflen-offset, ret))
			goto exit;
		offset += ret;
		ret = snprintf(buf+offset, buflen-offset, " timeout cs_type cs_mask\n");
		if (os_snprintf_error(buflen-offset, ret))
			goto exit;
		offset += ret;
		dl_list_for_each_safe(mscscfg, mscscfg_tmp, &mscs_cfg_list, struct mscs_configuration, list) {
			pConf = &mscscfg->csparam;
			ret = snprintf(buf+offset, buflen-offset,
				" %2d   "MACSTR" %7d	      0x%x %8d %8d  %5d      0x%x\n",
				i++, MAC2STR(pConf->sta_mac), pConf->request_type, pConf->up_bitmap, pConf->up_limit,
				pConf->timeout, pConf->cs.header.cs_type, pConf->cs.header.cs_mask);
			if (os_snprintf_error(buflen-offset, ret))
				goto exit;
			offset += ret;
		}
	}
#ifdef QOS_R2
	if (scs_active_sta_cnt) {
		ret = snprintf(buf+offset, buflen-offset, "[wappd]---------SCS Configuration---------\n");
		if (os_snprintf_error(buflen-offset, ret))
			goto exit;
		offset += ret;
		ret = snprintf(buf+offset, buflen-offset, "scs active sta cnt: %d\n", scs_active_sta_cnt);
		if (os_snprintf_error(buflen-offset, ret))
			goto exit;
		offset += ret;
		ret = snprintf(buf+offset, buflen-offset,
			" scsid	    sta_mac	   up alt_queue drop_elig processing\n");
		if (os_snprintf_error(buflen-offset, ret))
			goto exit;
		offset += ret;
		dl_list_for_each_safe(scscfg, scscfg_tmp, &scs_cfg_list, struct scs_configuration, list) {
			pScsTuple = &scscfg->scsparam;
			ret = snprintf(buf+offset, buflen-offset, " %3d   "MACSTR" %3d %5d %8d %10d",
				pScsTuple->scsid, MAC2STR(pScsTuple->sta_mac), pScsTuple->up,
				pScsTuple->alt_queue, pScsTuple->drop_elig, pScsTuple->processing);
			if (os_snprintf_error(buflen-offset, ret))
				goto exit;
			offset += ret;

			for (j = 0; (j < pScsTuple->tclas_num) && (j < MAX_TCLAS_NUM); j++) {
				ptclas = &pScsTuple->tclas_elem[j];
				if (ptclas->header.cs_type == CLASSIFIER_TYPE4) {
					ret = snprintf(buf+offset, buflen-offset,
						" {type4, mask:0x%x, version:%d, srcP:%d, dstP:%d, dscp:%d,",
						ptclas->type4.cs_mask, ptclas->type4.version,
						ntohs(ptclas->type4.srcPort), ntohs(ptclas->type4.destPort),
						ptclas->type4.DSCP);
					if (os_snprintf_error(buflen-offset, ret))
						goto exit;
					offset += ret;

					if (ptclas->type4.version == VER_IPV4) {
						ret = snprintf(buf+offset, buflen-offset,
							"srcIP:"IPV4STR", dstIP:"IPV4STR" protocol:%d},",
							NIP4(ptclas->type4.srcIp.ipv4), NIP4(ptclas->type4.destIp.ipv4),
							ptclas->type4.u.protocol);
						if (os_snprintf_error(buflen-offset, ret))
							goto exit;
						offset += ret;
					} else {
						ret = snprintf(buf+offset, buflen-offset,
							"srcIP:"IPV6STR", dstIP:"IPV6STR", proto/nxthd:%d,",
							NIP6(ptclas->type4.srcIp.ipv6),	NIP6(ptclas->type4.destIp.ipv6),
							ptclas->type4.u.nextheader);
						if (os_snprintf_error(buflen-offset, ret))
							goto exit;
						offset += ret;
						ret = snprintf(buf+offset, buflen-offset,
							" flabel:0x%02x%02x%02x},",
							ptclas->type4.flowLabel[0], ptclas->type4.flowLabel[1],
							ptclas->type4.flowLabel[2]);
						if (os_snprintf_error(buflen-offset, ret))
							goto exit;
						offset += ret;
					}
				} else if (ptclas->header.cs_type == CLASSIFIER_TYPE10) {
					ret = snprintf(buf+offset, buflen-offset,
						" {type10, protoInst:%d, proto/nxthd:%d,",
						ptclas->type10.protocolInstance, ptclas->type10.u.protocol);
					if (os_snprintf_error(buflen-offset, ret))
						goto exit;
					offset += ret;
					ret = snprintf(buf+offset, buflen-offset, " filterlen:%d, filterValue:0x",
						ptclas->type10.filterlen);
					if (os_snprintf_error(buflen-offset, ret))
						goto exit;
					offset += ret;
					for (k = 0; (k < ptclas->type10.filterlen) && (k < MAX_FILTER_LEN); k++) {
						ret = snprintf(buf+offset, buflen-offset,
							"%02x", ptclas->type10.filterValue[k]);
						if (os_snprintf_error(buflen-offset, ret))
							goto exit;
						offset += ret;
					}
					ret = snprintf(buf+offset, buflen-offset, ", filterMask:0x");
					if (os_snprintf_error(buflen-offset, ret))
						goto exit;
					offset += ret;
					for (k = 0; (k < ptclas->type10.filterlen) && (k < MAX_FILTER_LEN); k++) {
						ret = snprintf(buf+offset, buflen-offset, "%02x",
							ptclas->type10.filterMask[k]);
						if (os_snprintf_error(buflen-offset, ret))
							goto exit;
						offset += ret;
					}
					ret = snprintf(buf+offset, buflen-offset, "},");
					if (os_snprintf_error(buflen-offset, ret))
						goto exit;
					offset += ret;
				}
			}
			ret = snprintf(buf+offset, buflen-offset, "\n");
			if (os_snprintf_error(buflen-offset, ret))
				goto exit;
			offset += ret;
		}
	}

	if (dscp_policy_cnt) {
		ret = snprintf(buf+offset, buflen-offset, "[wappd]---------dscp policy---------\n");
		if (os_snprintf_error(buflen-offset, ret))
			goto exit;
		offset += ret;
		ret = snprintf(buf+offset, buflen-offset, " DSCP Policy cnt: %d\n", dscp_policy_cnt);
		if (os_snprintf_error(buflen-offset, ret))
			goto exit;
		offset += ret;
		ret = snprintf(buf+offset, buflen-offset,
			" PolicyID RequestType DSCP AttrID cs_type cs_mask version srcPort destPort");
		if (os_snprintf_error(buflen-offset, ret))
			goto exit;
		offset += ret;
		ret = snprintf(buf+offset, buflen-offset, " protocol     srcIp           destIp\n");
		if (os_snprintf_error(buflen-offset, ret))
			goto exit;
		offset += ret;
		dl_list_for_each_safe(dpolicy, dpolicy_tmp, &dscp_policy_list, struct dscp_policy_db, list) {
			ret = snprintf(buf+offset, buflen-offset, " %4d %8d %10d ", dpolicy->policyattri.policyid,
				dpolicy->policyattri.request_type, dpolicy->policyattri.dscp);
			if (os_snprintf_error(buflen-offset, ret))
				goto exit;
			offset += ret;
			if (dpolicy->tclasattri.header.attrid == ATTR_ID_TCLAS) {
				ret = snprintf(buf+offset, buflen-offset, " %5d %6d     0x%x  %5d",
					dpolicy->tclasattri.header.attrid,
					dpolicy->tclasattri.ipv4.cs_type, dpolicy->tclasattri.ipv4.cs_mask,
					dpolicy->tclasattri.ipv4.version);
				if (os_snprintf_error(buflen-offset, ret))
					goto exit;
				offset += ret;
				if (dpolicy->tclasattri.ipv4.version == VER_IPV4) {
					ret = snprintf(buf+offset, buflen-offset,
						"  %6d %8d %7d       "IPV4STR"       "IPV4STR"",
						ntohs(dpolicy->tclasattri.ipv4.srcPort),
						ntohs(dpolicy->tclasattri.ipv4.dstPort),
						dpolicy->tclasattri.ipv4.protocol,
						NIP4(dpolicy->tclasattri.ipv4.srcIp),
						NIP4(dpolicy->tclasattri.ipv4.dstIp));
					if (os_snprintf_error(buflen-offset, ret))
						goto exit;
					offset += ret;
				} else if (dpolicy->tclasattri.ipv4.version == VER_IPV6){
					ret = snprintf(buf+offset, buflen-offset,
						"  %6d %8d %7d   "IPV6STR"    "IPV6STR" ",
						ntohs(dpolicy->tclasattri.ipv6.srcPort),
						ntohs(dpolicy->tclasattri.ipv6.dstPort),
						dpolicy->tclasattri.ipv6.nextheader,
						NIP6(dpolicy->tclasattri.ipv6.srcIp),
						NIP6(dpolicy->tclasattri.ipv6.dstIp));
					if (os_snprintf_error(buflen-offset, ret))
						goto exit;
					offset += ret;
				}
			} else if (dpolicy->domainattri.attrid == ATTR_ID_DOMAIN_NAME) {
				ret = snprintf(buf+offset, buflen-offset, " %5d    %s ",
					dpolicy->domainattri.attrid, dpolicy->domainattri.domainname);
				if (os_snprintf_error(buflen-offset, ret))
					goto exit;
				offset += ret;
			} else if (dpolicy->portrange.attrid == ATTR_ID_PORT_RANGE && dpolicy->portrange.length == 4) {
				ret = snprintf(buf+offset, buflen-offset, " %5d    port_rg:[%d, %d] ", dpolicy->portrange.attrid,
					ntohs(dpolicy->portrange.startp), ntohs(dpolicy->portrange.endp));
				if (os_snprintf_error(buflen-offset, ret))
					goto exit;
				offset += ret;
			}
			ret = snprintf(buf+offset, buflen-offset, "\n");
			if (os_snprintf_error(buflen-offset, ret))
				goto exit;
			offset += ret;
		}
	}
#endif

exit:
	if (os_snprintf_error(buflen-offset, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s(), error happen, ret = %d\n", __func__, ret);

	if (strlen(buf)) {
		DBGPRINT(RT_DEBUG_ERROR, "%s\n", buf);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "[wappd] no qos configuration.\n");
	}

	return 0;
}

int qos_ctrl_interface_cmd_handle(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	int i, ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!argv[0]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	for (i = 0; qos_cmd[i].cmd != NULL; i++)
	{
		if (os_strncmp(qos_cmd[i].cmd, argv[0], os_strlen(argv[0])) == 0) {
			ret = qos_cmd[i].cmd_proc(wapp, iface, argc, argv);
			if (ret != WAPP_SUCCESS)
				DBGPRINT(RT_DEBUG_ERROR, "cmd [%s] failed. ret = %d\n",	qos_cmd[i].cmd, ret);
			break;
		}
	}

	return WAPP_SUCCESS;
}

int qos_cmd_show_help(struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 i = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	printf("\033[1;36m available cmds: \033[0m\n");
	for(i=0;(qos_cmd[i].cmd != NULL);i++){
		printf("\033[1;36m %20s  \t -  %s\033[0m\n",qos_cmd[i].cmd,qos_cmd[i].help);
	}

	return WAPP_SUCCESS;
}

int wapp_ctrl_iface_cmd_qos(struct wifi_app *wapp, const char *iface,
				 char *param_value_pair, char *reply, size_t *reply_len)
{
	int ret = 0; //, i = 0;
	struct wapp_conf *conf;
	int is_found = 0;
	char *token;
	u8 argc = 0;
	char *argv[9];

	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found) {
		if (wapp_dev_list_lookup_by_ifname(wapp, iface) == NULL) {
			DBGPRINT(RT_DEBUG_ERROR, "can not find configuration for interface(%s)\n", iface);
			return -1;
		}
	}

	/* for now, it always has 4 argc, due to the format of the source msg */
	token = strtok(param_value_pair, " ");
	while (token != NULL) {
		argv[argc] = token;
		argc++;
		token = strtok(NULL, " ");
	}

	qos_ctrl_interface_cmd_handle(wapp, iface, argc, argv);

	return ret;
}

static void mtqos_event_handler(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct iovec iov;
	struct msghdr msghdr;
	struct sockaddr_nl dest_addr;
	struct qos_netlink_message *msg = NULL;
	struct wifi_app *wapp = (struct wifi_app *)eloop_ctx;
	struct wapp_vend_spec_classifier_para_report *pcs = NULL;

	/*init dest address for receive msg from netlink_sock*/
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; /* For Linux Kernel */
	dest_addr.nl_groups = 0; /* unicast */
	iov.iov_base = (void *)g_nlmsghdr;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);

	memset(&msghdr, 0, sizeof(msghdr));
	msghdr.msg_name = (void *)&dest_addr;
	msghdr.msg_namelen = sizeof(dest_addr);
	msghdr.msg_iov = &iov;
	msghdr.msg_iovlen = 1;

	if (recvmsg(sock, &msghdr, 0) < 0)
		return;

	msg = (struct qos_netlink_message *)NLMSG_DATA(g_nlmsghdr);

	DBGPRINT(RT_DEBUG_TRACE, "event handler, msg->type:%d\n", msg->type);

	switch (msg->type) {
	case NL_VEND_SPECIFIC_CONFIG_EXPIRED:
		pcs = (struct wapp_vend_spec_classifier_para_report *)&msg->data[0];

		DBGPRINT(RT_DEBUG_ERROR,
			"[wapp] NL_VEND_SPECIFIC_CONFIG_EXPIRED, ifname:%s\n", pcs->ifname);

		wapp_drv_send_up_tuple_expired_notify(wapp, pcs->ifname,
			(char *)&msg->data[0], msg->len);
		break;
	default:
		DBGPRINT(RT_DEBUG_ERROR, "[wapp] unknown msg type(%02x)\n", msg->type);
		break;
	}
}

int wapp_drv_qos_init(struct wifi_app *wapp)
{
	struct sockaddr_nl addr;

	mscs_active_sta_cnt = 0;
	dl_list_init(&mscs_cfg_list);
#ifdef QOS_R2
	scs_active_sta_cnt = 0;
	dl_list_init(&scs_cfg_list);

	dscp_policy_cnt = 0;
	dl_list_init(&dscp_policy_list);
#endif

	netlink_sock = socket(AF_NETLINK, SOCK_RAW, QOS_NETLINK_EXT);
	if (netlink_sock < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "sock < 0.\n");
		return -1;
	}
	netlink_pid = getpid();

	/*1. initial nlmsg structure for sending*/
	memset((void *) &addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = netlink_pid;
	addr.nl_groups = 0;

	if (bind(netlink_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "bind < 0.\n");
		close(netlink_sock);
		return -1;
	}

	/*2. initial nlmsg structure for recieving*/
	g_nlmsghdr = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	if(!g_nlmsghdr){
		DBGPRINT(RT_DEBUG_ERROR, "%s(), malloc nlmsghdr error!\n", __func__);
		close(netlink_sock);
		return -1;
	}

	memset(g_nlmsghdr, 0, NLMSG_SPACE(MAX_PAYLOAD));
        g_nlmsghdr->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
        g_nlmsghdr->nlmsg_pid = netlink_pid;
        g_nlmsghdr->nlmsg_flags = 0;

	eloop_register_read_sock(netlink_sock, mtqos_event_handler, wapp, NULL);
	return 0;
}

int wapp_drv_qos_deinit(struct wifi_app *wapp)
{
	memset(&qos_config, 0, sizeof(qos_config));
	wapp_drv_send_event(NL_SET_QOS_MAP, NULL, 0);
	wapp_drv_send_event(NL_RESET_QOS_CONFIG, NULL, 0);
	clean_mscs_configuration();
#ifdef QOS_R2
	clean_scs_configuration();
	clean_dscp_policy();
#endif

	eloop_unregister_read_sock(netlink_sock);
	close(netlink_sock);
	if (g_nlmsghdr)
		free(g_nlmsghdr);

	return 0;
}

/*
* send netlink msg to mtqos module
*/
static int wapp_drv_send_netlink_msg(const unsigned char *message, int len, unsigned int dst_group)
{
	struct nlmsghdr *nlh = NULL;
	struct sockaddr_nl dest_addr;
	struct iovec iov;
	struct msghdr msg;

	if(!message ) {
		printf("send nl msg, message is null.\n");
		return -1;
	}

	//create message
	nlh = (struct nlmsghdr *)os_malloc(NLMSG_SPACE(len));
	if (!nlh) {
		printf("allocate nlmsghdr fail\n");
		return -1;
	}
	nlh->nlmsg_len = NLMSG_SPACE(len);
	nlh->nlmsg_pid = netlink_pid;
	nlh->nlmsg_flags = 0;
	os_memcpy(NLMSG_DATA(nlh), message, len);

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	os_memset(&dest_addr, 0, sizeof(struct sockaddr_nl));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;
	dest_addr.nl_groups = dst_group;

	os_memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(struct sockaddr_nl);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	//send message
	if (sendmsg(netlink_sock, &msg, 0) < 0) {
		printf("netlink msg send fail\n");
		os_free(nlh);
		return -3;
	}

	os_free(nlh);
	return 0;
}

int wapp_drv_send_event(unsigned char event, unsigned char *buf, int len)
{
	int ret = 0;
	struct qos_netlink_message *msg = (struct qos_netlink_message*)cmd_buf;

	if ((sizeof(struct qos_netlink_message) + len) > CMD_BUF_LEN) {
		DBGPRINT(RT_DEBUG_ERROR, "%s, fail to send event, len=%d\n", __func__, len);
		return -1;
	}

	msg->type = event;
	msg->len = len;
	if (buf)
		os_memcpy(&msg->data[0], buf, len);

	ret = wapp_drv_send_netlink_msg(cmd_buf, sizeof(struct qos_netlink_message) + len, 0);
	return ret;
}

int wapp_drv_send_action_frame(unsigned short oid, struct wifi_app *wapp, struct wapp_dev *wdev,
		unsigned int chan, unsigned int wait, const u8 *dst, const u8 *data, size_t len)
{
	static u16 seq_no = 0;

	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "wdev is null\n");
		return -1;
	}

	if (chan == 0) {
		chan = wdev->radio->op_ch;
	}
	if (!wapp->drv_ops || !wapp->drv_ops->send_action)
		return 0;

	DBGPRINT(RT_DEBUG_WARN, "send action frame to "MACSTR " seq_no =%u, chan=%u\n",
		MAC2STR(dst), seq_no++, chan);

	return wapp->drv_ops->send_action(oid, wapp->drv_data, wdev->ifname, chan, wait, dst,
			wdev->mac_addr, wdev->mac_addr, data, len, seq_no, 0);
}

u8 update_mscs_configuration(struct wifi_app *wapp, struct classifier_parameter *cs_param)
{
	u8 sta_code = STA_SUCCESS, existed = 0;
	struct mscs_configuration *mscscfg = NULL, *mscscfg_tmp = NULL;

	dl_list_for_each_safe(mscscfg, mscscfg_tmp, &mscs_cfg_list, struct mscs_configuration, list) {
		if (MAC_ADDR_EQUAL(mscscfg->csparam.sta_mac, cs_param->sta_mac)) {
			existed = 1;
			if (cs_param->request_type == REQUEST_TYPE_ADD) {
				sta_code = STA_REQUEST_DECLINED;
			} else if (cs_param->request_type == REQUEST_TYPE_CHANGE) {
				/*overwrite setting*/
				memcpy(&mscscfg->csparam, cs_param, sizeof(*cs_param));
				sta_code = STA_SUCCESS;
			} else if (cs_param->request_type == REQUEST_TYPE_REMOVE) {
				/*remove setting*/
				dl_list_del(&mscscfg->list);
				os_free(mscscfg);
				mscs_active_sta_cnt--;
				sta_code = STA_TERMINATED;
			} else
				sta_code = STA_REQUEST_DECLINED;
			break;
		}
	}

	if (!existed) {
		if (cs_param->request_type == REQUEST_TYPE_ADD) {
			if (mscs_active_sta_cnt < MAX_MSCS_STA_CNT) {
				mscscfg = (struct mscs_configuration *)os_zalloc(sizeof(struct mscs_configuration));
				if(mscscfg) {
					memcpy(&mscscfg->csparam, cs_param, sizeof(*cs_param));
					/*add setting*/
					dl_list_add_tail(&mscs_cfg_list, &mscscfg->list);
					mscs_active_sta_cnt++;
					sta_code = STA_SUCCESS;
				} else {
					DBGPRINT(RT_DEBUG_OFF, "%s Memory Alloc Fail\n", __func__);
					sta_code = STA_INSUFFICIENT_RESOURCES;
				}
			} else
				sta_code = STA_INSUFFICIENT_RESOURCES;
		} else
			sta_code = STA_REQUEST_DECLINED;
	}

	return sta_code;
}

void clean_mscs_configuration()
{
	struct mscs_configuration *mscscfg, *mscscfg_tmp;

	if (mscs_active_sta_cnt > 0) {
		dl_list_for_each_safe(mscscfg, mscscfg_tmp, &mscs_cfg_list, struct mscs_configuration, list) {
			dl_list_del(&mscscfg->list);
			os_free(mscscfg);
		}
		mscs_active_sta_cnt = 0;
	}
}

#ifdef QOS_R2
u8 update_scs_configuration(struct wifi_app *wapp, struct scs_classifier_parameter *scs_param)
{
	u8 sta_code = STA_SUCCESS, existed = 0;
	struct scs_configuration *scscfg = NULL, *scscfg_tmp = NULL;

	dl_list_for_each_safe(scscfg, scscfg_tmp, &scs_cfg_list, struct scs_configuration, list) {
		if (MAC_ADDR_EQUAL(scscfg->scsparam.sta_mac, scs_param->sta_mac)
			&& (scscfg->scsparam.scsid == scs_param->scsid)) {
			existed = 1;
			if (scs_param->request_type == REQUEST_TYPE_ADD) {
				sta_code = STA_REQUEST_DECLINED;
			} else if (scs_param->request_type == REQUEST_TYPE_CHANGE) {
				/*overwrite setting*/
				memcpy(&scscfg->scsparam, scs_param, sizeof(*scs_param));
				sta_code = STA_SUCCESS;
			} else if (scs_param->request_type == REQUEST_TYPE_REMOVE) {
				/*remove setting*/
				dl_list_del(&scscfg->list);
				os_free(scscfg);
				scs_active_sta_cnt--;
				sta_code = STA_TERMINATED;
			} else
				sta_code = STA_REQUEST_DECLINED;
			break;
		}
	}

	if (!existed) {
		if (scs_param->request_type == REQUEST_TYPE_ADD) {
			if (scs_active_sta_cnt < MAX_SCS_STA_CNT) {
				scscfg = (struct scs_configuration *)os_zalloc(sizeof(struct scs_configuration));
				if(scscfg) {
					memcpy(&scscfg->scsparam, scs_param, sizeof(*scs_param));
					/*add setting*/
					dl_list_add_tail(&scs_cfg_list, &scscfg->list);
					scs_active_sta_cnt++;
					sta_code = STA_SUCCESS;
				} else {
					DBGPRINT(RT_DEBUG_OFF, "%s Memory Alloc Fail\n", __func__);
					sta_code = STA_INSUFFICIENT_RESOURCES;
				}
			} else
				sta_code = STA_INSUFFICIENT_RESOURCES;
		} else
			sta_code = STA_REQUEST_DECLINED;
	}

	return sta_code;
}

void clean_scs_configuration()
{
	struct scs_configuration *scscfg, *scscfg_tmp;

	dl_list_for_each_safe(scscfg, scscfg_tmp, &scs_cfg_list, struct scs_configuration, list) {
		dl_list_del(&scscfg->list);
		os_free(scscfg);
	}
	scs_active_sta_cnt = 0;
}

u8 update_dscp_policy(struct dscp_policy_db *newpolicy)
{
	u8 sta_code = STA_SUCCESS, existed = 0;
	struct dscp_policy_db *pos = NULL, *pos_tmp = NULL;

	dl_list_for_each_safe(pos, pos_tmp, &dscp_policy_list, struct dscp_policy_db, list) {
		if (pos->policyattri.policyid == newpolicy->policyattri.policyid) {
			existed = 1;
			if (newpolicy->policyattri.request_type == REQUEST_TYPE_ADD) {
				sta_code = STA_SUCCESS;
				memcpy(&pos->policyattri, &newpolicy->policyattri, sizeof(newpolicy->policyattri));
				memcpy(&pos->tclasattri, &newpolicy->tclasattri, sizeof(newpolicy->tclasattri));
				memcpy(&pos->domainattri, &newpolicy->domainattri, sizeof(newpolicy->domainattri));
				memcpy(&pos->portrange, &newpolicy->portrange, sizeof(newpolicy->portrange));
			} else if (newpolicy->policyattri.request_type ==REQUEST_TYPE_REMOVE
				&& pos->policyattri.request_type == REQUEST_TYPE_ADD) {
				pos->policyattri.request_type = REQUEST_TYPE_REMOVE;
				pos->policyattri.dscp = 0xFF;
				sta_code = STA_SUCCESS;
				/*not remove setting from list, just change request type*/
				/*dl_list_del(&pos->list);
				os_free(pos);
				dscp_policy_cnt--;
				sta_code = STA_SUCCESS;*/
			}
			break;
		}
	}

	if (!existed) {
		if (newpolicy->policyattri.request_type == REQUEST_TYPE_ADD ||
			newpolicy->policyattri.request_type == REQUEST_TYPE_REMOVE) {
			pos = (struct dscp_policy_db *)os_zalloc(sizeof(struct dscp_policy_db));
			if(pos) {
				memcpy(pos, newpolicy, sizeof(*newpolicy));
				/*add setting*/
				dl_list_add_tail(&dscp_policy_list, &pos->list);
				dscp_policy_cnt++;
				sta_code = STA_SUCCESS;
			} else {
				DBGPRINT(RT_DEBUG_OFF, "%s Memory Alloc Fail\n", __func__);
				sta_code = STA_INSUFFICIENT_RESOURCES;
			}
		} else
			sta_code = STA_REQUEST_DECLINED;
	}

	return sta_code;
}

struct dscp_policy_db *get_dscp_policy_byid(u8 id)
{
	struct dscp_policy_db *pos = NULL, *pos_tmp = NULL;

	dl_list_for_each_safe(pos, pos_tmp, &dscp_policy_list, struct dscp_policy_db, list) {
		if (pos->policyattri.policyid == id)
			return pos;
	}

	return NULL;
}

void clean_dscp_policy()
{
	struct dscp_policy_db *dscppolicy, *dscppolicy_tmp;

	dl_list_for_each_safe(dscppolicy, dscppolicy_tmp, &dscp_policy_list, struct dscp_policy_db, list) {
		dl_list_del(&dscppolicy->list);
		os_free(dscppolicy);
	}
	dscp_policy_cnt = 0;
}

void parse_ip_address(u8 type, char *strip, char *buf, int buflen)
{
	u8 i = 0;
	char *value = NULL;

	if (!strip || !buf || !buflen)
		return;

	if (type == VER_IPV4) {
		if (buflen < 4)
			return;

		value = strtok(strip, ".");
		for ( i = 0; (i < 4) && value; i++) {
			buf[i] = strtol(value, 0, 10);
			if (errno != 0)
				DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
			value = strtok(NULL, ".");

			if (value == NULL)
				break;
		}
	} else if (type == VER_IPV6) {
		if (buflen < 16)
			return;
		inet_pton(AF_INET6, strip, (void*)buf);
	}
}

/*
* will parse dscp policy from string.
* example
* 1. DSCPPolicy_PolicyID,1,DSCPPolicy_RequestType,Add,DSCPPolicy_DSCP,0x2E,TCLAS_ClassifierType,4,
     TCLAS_ClassifierMask,0x55,TCLAS_ClassifierParams_Version,4,TCLAS_ClassifierParams_DestIPAddr,192.165.100.36,
     TCLAS_ClassifierParams_DestPort,5201,TCLAS_ClassifierParams_Protocol,17
* 2. DSCPPolicy_PolicyID,4,DSCPPolicy_RequestType,Add,DSCPPolicy_DSCP,0x18, DSCPPolicy_DomainName,video.wifitest.org
*/
void parse_dscp_policy(char *str)
{
	char *type = NULL, *value = NULL;
	char srcIP[64] = {0}, dstIP[64] = {0};
	struct dscp_policy_db policy;
	u8 ipver = 0;

	if (!str) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: Invalid argument\n", __func__);
		return ;
	}

	memset(srcIP, 0, sizeof(srcIP));
	memset(dstIP, 0, sizeof(dstIP));
	memset(&policy, 0, sizeof(policy));

	type = strtok(str, ",");
	if (!type) {
		DBGPRINT(RT_DEBUG_ERROR, "%s(%d), strtok got NULL.\n", __func__, __LINE__);
		return;
	}
	value = strtok(NULL, ",");
	if (!value) {
		DBGPRINT(RT_DEBUG_ERROR, "%s(%d), strtok got NULL.\n", __func__, __LINE__);
		return;
	}
	do {
		if (str_equal(type, "DSCPPolicy_PolicyID")) {
			policy.policyattri.policyid = strtol(value, 0, 10);
			if (errno != 0)
				DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
		} else if (str_equal(type, "DSCPPolicy_RequestType")) {
			if(str_equal(value, "Add"))
				policy.policyattri.request_type = REQUEST_TYPE_ADD;
			else {
				policy.policyattri.request_type = REQUEST_TYPE_REMOVE;
				policy.policyattri.dscp = 0xFF;
			}
		} else if (str_equal(type, "DSCPPolicy_DSCP")) {
			policy.policyattri.dscp = strtol(value, 0, 10);
			if (errno != 0)
				DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
		} else if (str_equal(type, "DomainName_Domain")) {
			policy.domainattri.attrid = ATTR_ID_DOMAIN_NAME;
			memcpy(policy.domainattri.domainname, value, MIN(strlen(value), MAX_DOMAIN_NAME_LEN));
		} else if (str_equal(type, "TCLAS_ClassifierType")) {
			policy.tclasattri.header.attrid = ATTR_ID_TCLAS;
			policy.tclasattri.ipv4.cs_type = strtol(value, 0, 10);
			if (errno != 0)
				DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
		} else if (str_equal(type, "TCLAS_ClassifierMask")) {
			policy.tclasattri.ipv4.cs_mask = strtol(value, 0, 16);
			if (errno != 0)
				DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
		} else if (str_equal(type, "TCLAS_ClassifierParams_Version")) {
			ipver = strtol(value, 0, 10);
			if (errno != 0)
				DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
			policy.tclasattri.ipv4.version = ipver;
		} else if (str_equal(type, "TCLAS_ClassifierParams_SrcIPAddr")) {
			memcpy(srcIP, value, MIN(strlen(value), sizeof(srcIP)));
		} else if (str_equal(type, "TCLAS_ClassifierParams_DestIPAddr")) {
			memcpy(dstIP, value, MIN(strlen(value), sizeof(dstIP)));
		} else if (str_equal(type, "TCLAS_ClassifierParams_SrcPort")) {
			if (ipver == VER_IPV4) {
				policy.tclasattri.ipv4.srcPort = strtol(value, 0, 10);
				if (errno != 0)
					DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
				policy.tclasattri.ipv4.srcPort = htons(policy.tclasattri.ipv4.srcPort);
			} else if (ipver == VER_IPV6) {
				policy.tclasattri.ipv6.srcPort = strtol(value, 0, 10);
				if (errno != 0)
					DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
				policy.tclasattri.ipv6.srcPort = htons(policy.tclasattri.ipv6.srcPort);
			}
		} else if (str_equal(type, "TCLAS_ClassifierParams_DestPort")) {
			if (ipver == VER_IPV4) {
				policy.tclasattri.ipv4.dstPort = strtol(value, 0, 10);
				if (errno != 0)
					DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
				policy.tclasattri.ipv4.dstPort = htons(policy.tclasattri.ipv4.dstPort);
			} else if (ipver == VER_IPV6) {
				policy.tclasattri.ipv6.dstPort = strtol(value, 0, 10);
				if (errno != 0)
					DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
				policy.tclasattri.ipv6.dstPort = htons(policy.tclasattri.ipv6.dstPort);
			}
		} else if (str_equal(type, "TCLAS_ClassifierParams_Protocol")) {
			if (ipver == VER_IPV4) {
				policy.tclasattri.ipv4.protocol = strtol(value, 0, 10);
				if (errno != 0)
					DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
			} else if (ipver == VER_IPV6) {
				policy.tclasattri.ipv6.nextheader = strtol(value, 0, 10);
				if (errno != 0)
					DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
			}
		} else if (str_equal(type, "PortRange_StartPort")) {
			policy.portrange.attrid = ATTR_ID_PORT_RANGE;
			policy.portrange.length = 4;
			policy.portrange.startp = strtol(value, 0, 10);
			if (errno != 0)
				DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
			policy.portrange.startp = htons(policy.portrange.startp);
		} else if (str_equal(type, "PortRange_EndPort")) {
			policy.portrange.endp = strtol(value, 0, 10);
			if (errno != 0)
				DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
			policy.portrange.endp = htons(policy.portrange.endp);
		} else {
			DBGPRINT(RT_DEBUG_ERROR, "%s: unknow parameters %s.\n", __func__, type);
		}
		type = strtok(NULL, ",");
		if (type == NULL)
			break;
		value = strtok(NULL, ",");
		if (value == NULL)
			break;
	} while (type != NULL && value != NULL);

	if (ipver == VER_IPV4) {
		if (strlen(srcIP))
			parse_ip_address(ipver, srcIP, (char *)&policy.tclasattri.ipv4.srcIp,
					sizeof(policy.tclasattri.ipv4.srcIp));
		if (strlen(dstIP))
			parse_ip_address(ipver, dstIP, (char *)&policy.tclasattri.ipv4.dstIp,
					sizeof(policy.tclasattri.ipv4.dstIp));
	}
	if (ipver == VER_IPV6) {
		if (strlen(srcIP))
			parse_ip_address(ipver, srcIP, (char *)&policy.tclasattri.ipv6.srcIp,
					sizeof(policy.tclasattri.ipv6.srcIp));
		if (strlen(dstIP))
			parse_ip_address(ipver, dstIP, (char *)&policy.tclasattri.ipv6.dstIp,
					sizeof(policy.tclasattri.ipv6.dstIp));
	}

	update_dscp_policy(&policy);
}

u8 parse_policyid_list(char *str, u8 *buf, u8 buflen)
{
	char *value = NULL;
	int num = 0;

	if (!str || !buf || !buflen) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: Invalid argument\n", __func__);
		return 0;
	}

	value = strtok(str, "_");
	while (value != NULL) {
		buf[num++] = strtol(value, 0, 10);
		if (errno != 0)
			DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
		if (num >= buflen)
			break;

		value = strtok(NULL, "_");
	}
	return num;
}

static inline u8 build_dscp_policy_req_frame(struct dscp_policy_req *policyreq, u8 *frmbuf, u16 buflen)
{
	static u8 token = 1;
	u8 i = 0, frmlen = 0;
	struct qos_management_element *qos_mgmt_elem = NULL;
	struct dscp_policy_attribute *policyattr = NULL;
	struct tclas_attribute_ipv4 *tclasattr_ipv4 = NULL;
	struct tclas_attribute_ipv6 *tclasattr_ipv6 = NULL;
	struct domain_name_attribute *domianattr = NULL;
	struct port_range_attribute *portattr = NULL;
	struct dscp_policy_db *policy = NULL;

	if (!frmbuf || !buflen || !policyreq)
		return 0;

	memset(frmbuf, 0, buflen);
	frmlen = 0;

	/*build QOS Policy Request Frame Header*/
	frmbuf[frmlen] = CATEGORY_VEND_SPECIFIC_PROTECTED;
	frmlen++;
	memcpy(&frmbuf[frmlen], WFA_OUI, sizeof(WFA_OUI));
	frmlen += sizeof(WFA_OUI);
	frmbuf[frmlen] = QOS_ACT_FRM_OUI_TYPE;
	frmlen++;
	frmbuf[frmlen] = DSCP_POLICY_REQ;
	frmlen++;
	frmbuf[frmlen] = (policyreq->token) ? (policyreq->token) : (token++);
	frmlen++;
	frmbuf[frmlen] = 0; /*Request Control field*/
	if (policyreq->moreflag == 1)
		frmbuf[frmlen] |= 1;
	if (policyreq->resetflag == 1)
		frmbuf[frmlen] |= 2;
	frmlen++;

	/*build QoS Management elements*/
	for (i = 0; i < policyreq->id_num; i++) {
		policy = get_dscp_policy_byid(policyreq->policyids[i]);
		if (!policy)
			continue;

		/*QoS Management element*/
		qos_mgmt_elem = (struct qos_management_element *)&frmbuf[frmlen];
		qos_mgmt_elem->elementid = QOS_MGMT_ELEMENT_ID;
		qos_mgmt_elem->length = 4;
		memcpy(qos_mgmt_elem->oui, WFA_OUI, sizeof(WFA_OUI));
		qos_mgmt_elem->ouitype = QOS_MGMT_IE_OUI_TYPE;
		frmlen += sizeof(*qos_mgmt_elem);

		/*dscp policy attribute*/
		policyattr = (struct dscp_policy_attribute *)&frmbuf[frmlen];
		memcpy(policyattr, &policy->policyattri, sizeof(policy->policyattri));
		policyattr->attrid = ATTR_ID_DSCP_POLICY;
		policyattr->length = sizeof(*policyattr) -2;
		frmlen += sizeof(*policyattr);
		qos_mgmt_elem->length += sizeof(*policyattr);

		if (policyattr->request_type == REQUEST_TYPE_REMOVE)
			continue;

		/*tclas attribute*/
		if (policy->tclasattri.header.attrid == ATTR_ID_TCLAS) {
			if (policy->tclasattri.ipv4.version == VER_IPV4) {
				tclasattr_ipv4 = (struct tclas_attribute_ipv4 *)&frmbuf[frmlen];
				memcpy(tclasattr_ipv4, &policy->tclasattri.ipv4, sizeof(policy->tclasattri.ipv4));
				tclasattr_ipv4->length = sizeof(*tclasattr_ipv4) - 2;
				frmlen += sizeof(*tclasattr_ipv4);
				qos_mgmt_elem->length += sizeof(*tclasattr_ipv4);
			} else if (policy->tclasattri.ipv4.version == VER_IPV6) {
				tclasattr_ipv6 = (struct tclas_attribute_ipv6 *)&frmbuf[frmlen];
				memcpy(tclasattr_ipv6, &policy->tclasattri.ipv6, sizeof(policy->tclasattri.ipv6));
				tclasattr_ipv6->length = sizeof(*tclasattr_ipv6) - 2;
				frmlen += sizeof(*tclasattr_ipv6);
				qos_mgmt_elem->length += sizeof(*tclasattr_ipv6);
			} else {
				DBGPRINT(RT_DEBUG_ERROR, "%s: unknow type4.version: %d.\n",
					__func__, policy->tclasattri.ipv4.version);
			}
		}

		/*domain name attribute*/
		if (policy->domainattri.attrid == ATTR_ID_DOMAIN_NAME) {
			domianattr = (struct domain_name_attribute *)&frmbuf[frmlen];
			domianattr->attrid = ATTR_ID_DOMAIN_NAME;
			domianattr->length = MIN(strlen(policy->domainattri.domainname), MAX_DOMAIN_NAME_LEN);
			memcpy(&domianattr->domainname, policy->domainattri.domainname, domianattr->length);
			frmlen += domianattr->length + 2;
			qos_mgmt_elem->length += domianattr->length + 2;
		}

		/*port range attribute*/
		if (policy->portrange.attrid == ATTR_ID_PORT_RANGE) {
			portattr = (struct port_range_attribute *)&frmbuf[frmlen];
			portattr->attrid = ATTR_ID_PORT_RANGE;
			portattr->length = 4;
			portattr->startp = policy->portrange.startp;
			portattr->endp = policy->portrange.endp;
			frmlen += portattr->length + 2;
			qos_mgmt_elem->length += portattr->length + 2;
		}
	}

	return frmlen;
}

void wapp_send_dscp_policy_req(struct wifi_app *wapp, u8 *dstmac, struct dscp_policy_req *policyreq)
{
	u32 len = 0;
	struct wapp_dev *wdev = NULL;
	char ifname[16] = {0};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	get_config_ifname(wapp, ifname, sizeof(ifname));
	wdev = wapp_dev_list_lookup_by_ifname(wapp, ifname);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s invalid wdev:%p\n", __func__, wdev);
		return;
	}

	len = build_dscp_policy_req_frame(policyreq, sendbuf, sizeof(sendbuf));
	wapp_drv_send_action_frame(OID_SEND_QOS_ACTION_FRAME, wapp, wdev, qos_config.channel, 0, dstmac, sendbuf, len);
}

void wapp_dscp_policy_action_frame_process(struct wifi_app *wapp,
	struct wapp_dev *wdev, u8 channel, u8 *peermac, u8 *frmbuf, u32 frmlen)
{
	u32 len = 0;
	u8 *buf = NULL;
	u8 token = 0, type = 0;
	struct dscp_policy_req policyreq;
	struct dscp_policy_db *pos = NULL, *pos_tmp = NULL;


	if (!wapp || !wdev || !peermac || !frmbuf)
		return;

	buf = frmbuf;
	len = frmlen;
	if ((buf[0] != CATEGORY_VEND_SPECIFIC_PROTECTED && buf[0] != CATEGORY_VEND_SPECIFIC)
		|| ((buf[5] != DSCP_POLICY_QRY) && (buf[5] != DSCP_POLICY_RSP))) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: unknown action type, cat:%d, action:%d \n", __func__, buf[0], buf[5]);
		return;
	}
	type = buf[5];
	token = buf[6];

	/*buf += 7;*/
	/*len -= 7;*/

	DBGPRINT(RT_DEBUG_TRACE, "Received DSCP Policy Action frame from "MACSTR" type=%u, token=%u, chan=%u, len=%d\n",
		MAC2STR(peermac), type, token, channel, len);

	if (type == DSCP_POLICY_QRY) {
		memset(&policyreq, 0, sizeof(policyreq));
		policyreq.resetflag = qos_config.RequestCtrlReset;
		policyreq.token = token;

		if(frmlen < 7) {
			DBGPRINT(RT_DEBUG_ERROR, "%s, invalid DSCP_POLICY_QRY frame.\n", __func__);
		} else if (frmlen == 7) { /* wildcard DSCP Policy Query*/
			dl_list_for_each_safe(pos, pos_tmp, &dscp_policy_list, struct dscp_policy_db, list) {
				if (policyreq.id_num < MAX_POLICY_NUM)
					policyreq.policyids[policyreq.id_num++] = pos->policyattri.policyid;
				else
					DBGPRINT(RT_DEBUG_ERROR, "%s, error, reach max policy mumber.\n", __func__);
			}
		} else {
			dl_list_for_each_safe(pos, pos_tmp, &dscp_policy_list, struct dscp_policy_db, list) {
				policyreq.policyids[policyreq.id_num++] = pos->policyattri.policyid;
			}
			DBGPRINT(RT_DEBUG_ERROR, "%s, dscp policy query with QoS Managment Element.\n", __func__);
		}
		wapp_send_dscp_policy_req(wapp, peermac, &policyreq);
	} else if (type == DSCP_POLICY_RSP) {
		DBGPRINT(RT_DEBUG_ERROR, "%s, DSCP_POLICY_RSP was received, do nothing.\n", __func__);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "%s, unexpected DSCP Policy Action Frame, type:%d.\n", __func__, type);
	}

	return;
}

/*
* will parse dscp policy req cmd string.
* example
* Dest_MAC,11:22:33:44:55:66,PolicyID_List,1_2_3
*/
void parse_dscp_policy_req(struct wifi_app * wapp, char *str)
{
	char *type = NULL, *value = NULL;
	u8 dstmac[MAC_ADDR_LEN] = {0};
	char str_mac[64] = {0}, str_id_list[32] = {0};
	struct dscp_policy_req policyrep;

	if (!str) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: Invalid argument\n", __func__);
		return ;
	}

	DBGPRINT(RT_DEBUG_TRACE, "%s: %s\n", __func__, str);

	memset(dstmac, 0, sizeof(dstmac));
	memset(str_mac, 0, sizeof(str_mac));
	memset(str_id_list, 0, sizeof(str_id_list));
	memset(&policyrep, 0, sizeof(policyrep));

	type = strtok(str, ",");
	if (!type) {
		DBGPRINT(RT_DEBUG_ERROR, "%s(%d), strtok got NULL.\n", __func__, __LINE__);
		return;
	}
	value = strtok(NULL, ",");
	if (!value) {
		DBGPRINT(RT_DEBUG_ERROR, "%s(%d), strtok got NULL.\n", __func__, __LINE__);
		return;
	}
	do {
		if (!os_strcasecmp(type, "Dest_MAC")) {
			memcpy(str_mac, value, MIN(strlen(value), sizeof(str_mac)));
		} else if (!os_strcasecmp(type, "PolicyID_List")) {
			memcpy(str_id_list, value, MIN(strlen(value), sizeof(str_id_list)));
		} else if (!os_strcasecmp(type, "RequestControl_Reset")) {
			policyrep.resetflag = strtol(value, 0, 10);
			if (errno != 0)
				DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
		} else {
			DBGPRINT(RT_DEBUG_ERROR, "%s: unknow parameters %s.\n", __func__, type);
		}
		type = strtok(NULL, ",");
		if (type == NULL)
			break;
		value = strtok(NULL, ",");
		if (value == NULL)
			break;
	} while (type != NULL && value != NULL);
	if (strlen(str_mac))
		parse_macstr(str_mac, dstmac);
	if (strlen(str_id_list))
		policyrep.id_num = parse_policyid_list(str_id_list, policyrep.policyids, sizeof(policyrep.policyids));

	DBGPRINT(RT_DEBUG_ERROR, "%s: "MACSTR"\n", __func__, MAC2STR(dstmac));

	if (policyrep.id_num)
		wapp_send_dscp_policy_req(wapp, dstmac, &policyrep);
}

static inline u8 build_scs_resp_frame(u8 token, struct scs_status *scssta, u8 scsnum, u8 *frmbuf, u16 buflen)
{
	u8 i = 0, frmlen = 0;

	if (!frmbuf || !buflen)
		return 0;

	memset(frmbuf, 0, buflen);
	frmlen = 0;
	frmbuf[frmlen] = CATEGORY_RAVS;
	frmlen++;
	frmbuf[frmlen] = ACT_SCS_RSP;
	frmlen++;
	frmbuf[frmlen] = token;
	frmlen++;

	/*count field*/
	frmbuf[frmlen] = scsnum;
	frmlen++;

	for (i = 0; i < scsnum && scssta; i++) {
		memcpy(&frmbuf[frmlen], &scssta[i].scsid, 1);
		frmlen++;
		memcpy(&frmbuf[frmlen], &scssta[i].stacode, 2);
		frmlen += 2;
	}

	return frmlen;
}

/*
* will parse unsolicted SCS Response cmd string from UCC.
* example
* SCSRsp,Dest_MAC,11:22:33:44:55:66,SCSID,1,Status,Remove
*/
void parse_scs_response(struct wifi_app * wapp, const char *ifname, char *str)
{
	u32 len = 0;
	char *type = NULL, *value = NULL;
	u8 dstmac[MAC_ADDR_LEN] = {0};
	char str_mac[64] = {0};
	struct scs_status sta_code;
	struct wapp_dev *wdev = NULL;
	struct scs_classifier_parameter scs_param;

	wdev = wapp_dev_list_lookup_by_ifname(wapp, ifname);
	if (!wdev)
		return;

	if (!str) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: Invalid argument\n", __func__);
		return;
	}

	DBGPRINT(RT_DEBUG_ERROR, "%s: %s\n", __func__, str);

	memset(dstmac, 0, sizeof(dstmac));
	memset(str_mac, 0, sizeof(str_mac));
	memset(&sta_code, 0, sizeof(sta_code));
	memset(&scs_param, 0, sizeof(scs_param));

	type = strtok(str, ",");
	if (!type) {
		DBGPRINT(RT_DEBUG_ERROR, "%s(%d), strtok got NULL.\n", __func__, __LINE__);
		return;
	}
	value = strtok(NULL, ",");
	if (!value) {
		DBGPRINT(RT_DEBUG_ERROR, "%s(%d), strtok got NULL.\n", __func__, __LINE__);
		return;
	}
	do {
		if (!os_strcasecmp(type, "Dest_MAC")) {
			memcpy(str_mac, value, MIN(strlen(value), sizeof(str_mac)));
		} else if (!os_strcasecmp(type, "SCSDescrElem_SCSID_1")) {
			sta_code.scsid = strtol(value, 0, 10);
			if (errno != 0)
				DBGPRINT(RT_DEBUG_ERROR, "%s(), strtol got error:%s\n", __func__, strerror(errno));
		} else if (!os_strcasecmp(type, "SCSDescrElem_RequestType_1")) {
			if (!os_strcasecmp(value, "Remove")) {
				sta_code.stacode = STA_TERMINATED;
				DBGPRINT(RT_DEBUG_ERROR, "%s\n", value);
			} else
				sta_code.stacode = STA_SUCCESS;
		} else {
			DBGPRINT(RT_DEBUG_ERROR, "%s: unknow parameters %s.\n", __func__, type);
		}
		type = strtok(NULL, ",");
		if (type == NULL)
			break;
		value = strtok(NULL, ",");
		if (value == NULL)
			break;
	} while (type != NULL && value != NULL);

	if (strlen(str_mac))
		parse_macstr(str_mac, dstmac);

	/*Update SCS Configuration*/
	if (sta_code.stacode != STA_SUCCESS) {
		memcpy(scs_param.sta_mac, dstmac, MAC_ADDR_LEN);
		scs_param.scsid = sta_code.scsid;
		scs_param.request_type = REQUEST_TYPE_REMOVE;

		sendbuf[0] = 1;
		memcpy(&sendbuf[1], &scs_param, sizeof(scs_param));
		wapp_drv_send_event(NL_SET_SCS_CONFIG, sendbuf, 1 + sizeof(scs_param));
		update_scs_configuration(wapp, &scs_param);
	}

	/*build scs response action frame and send to peer.*/
	len = build_scs_resp_frame(0, &sta_code, 1, sendbuf, sizeof(sendbuf));
	wapp_drv_send_action_frame(OID_SEND_QOS_ACTION_FRAME, wapp, wdev, qos_config.channel, 0, dstmac, sendbuf, len);
}

int wapp_write_pmk(char* filename, char *buf, u8 len)
{
	FILE *f = NULL;
	int fd = 0, i = 0;

	if (!buf || !len)
		return -1;

	f = fopen(filename, "w");
	if (f == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, "Failed to open '%s' for writing\n", filename);
		return -1;
	}

	for (i = 0; i < len; i++) {
		if (fprintf(f, "%02x", buf[i]) < 0) {
			DBGPRINT(RT_DEBUG_ERROR, "%s(), fprintf got error.\n", __func__);
			break;
		}
	}
	fd = fileno(f);
	fsync(fd);
	if (!fclose(f))
		DBGPRINT(RT_DEBUG_ERROR, "%s(), fclose get error %s\n", __func__, strerror(errno));

	return 0;
}

int get_pmk_by_peer_mac(struct wifi_app *wapp, const char *ifname, u8 *peermac)
{
	int buflen = 64;
	char buf[64] = {0};
	struct wapp_dev *wdev = NULL;

	if (!wapp || !peermac) {
		DBGPRINT(RT_DEBUG_ERROR, "invalid is parameter\n");
		return -1;
	}

	wdev = wapp_dev_list_lookup_by_ifname(wapp, ifname);
	if (!wdev)
		return -1;

	if (!wapp->drv_ops || !wapp->drv_ops->send_action)
		return 0;

	memcpy(buf, peermac, MAC_ADDR_LEN);
	wapp->drv_ops->drv_get_pmk_by_peermac(wapp->drv_data, wdev->ifname, buf, &buflen);
	wapp_write_pmk("/tmp/pmk.txt", buf, buflen);

	return 0;
}

#endif

static inline u8 build_mscs_resp_frame(u8 token, u16 statuscode, u8 *frmbuf, u16 buflen)
{
	u8 frmlen = 0;

	if (!frmbuf || !buflen)
		return 0;

	memset(frmbuf, 0, buflen);
	frmlen = 0;
	frmbuf[frmlen] = CATEGORY_RAVS;
	frmlen++;
	frmbuf[frmlen] = ACT_MSCS_RSP;
	frmlen++;
	frmbuf[frmlen] = token;
	frmlen++;

	memcpy(&frmbuf[frmlen], &statuscode, 2);
	frmlen += 2;

	return frmlen;
}

void wapp_mscs_action_frame_process(struct wifi_app *wapp,
	struct wapp_dev *wdev, u8 channel, u8 *peermac, u8 *frmbuf, u32 frmlen)
{
	u32 len = 0;
	u8 *buf = NULL;
	u8 token = 0, type = 0, subielen = 0;
	struct classifier_parameter cs_param;
	u16 sta_code = STA_SUCCESS;

	if (!wapp || !wdev || !peermac || !frmbuf)
		return;

	buf = frmbuf;
	len = frmlen;
	if (buf[0] != CATEGORY_RAVS ||
		((buf[1] != ACT_MSCS_REQ) &&
		(buf[1] != ACT_MSCS_RSP))) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: unknown action type, cat:%d, action:%d \n",
			__func__, buf[0], buf[1]);
		sta_code = STA_REQUEST_DECLINED;
		goto exit1;
	}
	type = buf[1];
	token = buf[2];
	buf += 3;
	len -= 3;

	DBGPRINT(RT_DEBUG_TRACE, "MSCS: Received MSCS Action frame from "MACSTR" type=%u, token=%u, chan=%u, len=%d\n",
		MAC2STR(peermac), type, token, channel, len);

	if (type == ACT_MSCS_RSP)
		goto exit2;

	/*MSCS Descriptor TLV Header Check*/
	if ((*buf) == IE_WLAN_EXTENSION && (*(buf+2)) == IE_EXTENSION_ID_MSCS_DESC) {
		buf += 3;
		len -= 3;
	} else {
		sta_code = STA_REQUESTED_NOT_SUPPORTED;
		DBGPRINT(RT_DEBUG_ERROR, "%s, invalid mscs descriptor header, ID:%u, len:%u, extID:%u.\n",
			__func__, *buf, *(buf+1), *(buf+2));
		goto exit1;
	}

	memset(&cs_param, 0, sizeof(cs_param));
	COPY_MAC_ADDR(cs_param.sta_mac, peermac);

	if ((*buf) >= REQUEST_TYPE_UNKNOWN){
		sta_code = STA_REQUEST_DECLINED;
		DBGPRINT(RT_DEBUG_ERROR, "%s, unknown mscs request type:%u\n", __func__, *buf);
		goto exit1;
	}

	if ((((*buf) == REQUEST_TYPE_ADD || (*buf) == REQUEST_TYPE_CHANGE) && len < 13) ||
		(((*buf) == REQUEST_TYPE_REMOVE) && len < 7)) {
		DBGPRINT(RT_DEBUG_ERROR, "%s, invalid mscs descriptor element.\n", __func__);
		sta_code = STA_REQUEST_DECLINED;
		goto exit1;
	}

	cs_param.request_type = *buf;
	cs_param.up_bitmap = *(buf+1);
	cs_param.up_limit = (*(buf+2)) & 0x07;
	cs_param.timeout = ((*((UINT32 *)(buf+3)))*1024)/1000000;
	buf += 7;
	len -= 7;

	if (cs_param.request_type == REQUEST_TYPE_REMOVE) {
		sta_code = update_mscs_configuration(wapp, &cs_param);
		sendbuf[0] = 1;
		memcpy(&sendbuf[1], &cs_param, sizeof(cs_param));
		wapp_drv_send_event(NL_SET_MSCS_CONFIG, sendbuf, 1 + sizeof(cs_param));
		goto exit1;
	}

	while ((*buf) == IE_WLAN_EXTENSION && (*(buf+2)) == IE_EXTENSION_ID_TCLAS_MASK) {
		subielen = *(buf+1);
		buf += 2;
		len -= 2;
		if (subielen > len || sta_code != STA_SUCCESS) {
			DBGPRINT(RT_DEBUG_TRACE, "%s, len:%u, leftlen:%d, sta_code:%d.\n",
				__func__, subielen, len, sta_code);
			break;
		}

		if((*(buf+1)) == CLASSIFIER_TYPE4) {
			cs_param.cs.header.cs_type = *(buf+1);
			cs_param.cs.header.cs_mask = *(buf+2);
			sta_code = update_mscs_configuration(wapp, &cs_param);
			sendbuf[0] = 1;
			memcpy(&sendbuf[1], &cs_param, sizeof(cs_param));
			wapp_drv_send_event(NL_SET_MSCS_CONFIG, sendbuf, 1 + sizeof(cs_param));
		} else
			DBGPRINT(RT_DEBUG_ERROR, "%s, not support mscs classifier type= %d.\n", __func__, (*buf));

		buf += subielen;
		len -= subielen;
	}

exit1:
	/*build mscs response action frame and send to peer.*/
	len = build_mscs_resp_frame(token, sta_code, sendbuf, sizeof(sendbuf));
	wapp_drv_send_action_frame(OID_SEND_QOS_ACTION_FRAME, wapp, wdev, channel, 0, peermac, sendbuf, len);

	return;
exit2:
	DBGPRINT(RT_DEBUG_ERROR, "%s, ACT_MSCS_RSP was received, do nothing.\n", __func__);
	return;
}

#ifdef QOS_R2
void wapp_scs_action_frame_process(struct wifi_app *wapp,
	struct wapp_dev *wdev, u8 channel, u8 *peermac, u8 *frmbuf, u32 frmlen)
{
	u32 len = 0;
	int leftlen = 0;
	u8 filterlen = 0, i = 0, valid_len = 0;
	u8 *filtervalue = NULL, *filtermask = NULL;
	u8 *buf = NULL, scs_num = 0;
	u8 token = 0, type = 0, subielen = 0;
	struct scs_classifier_parameter scs_param;
	struct scs_status sta_code[10];
	ptclas_element ptclas = NULL;

	if (!wapp || !wdev || !peermac || !frmbuf)
		return;

	buf = frmbuf;
	len = frmlen;
	if (buf[0] != CATEGORY_RAVS || ((buf[1] != ACT_SCS_REQ) && (buf[1] != ACT_SCS_RSP))) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: unknown action type, cat:%d, action:%d \n", __func__, buf[0], buf[1]);
		return;
	}

	type = buf[1];
	token = buf[2];
	buf += 3;
	len -= 3;

	DBGPRINT(RT_DEBUG_TRACE, "SCS: Received SCS Action frame from "MACSTR" type=%u, token=%u, chan=%u, len=%d\n",
		MAC2STR(peermac), type, token, channel, len);

	if (type == ACT_MSCS_RSP) {
		DBGPRINT(RT_DEBUG_ERROR, "%s, ACT_SCS_RSP was received, do nothing.\n", __func__);
		return;
	}

	/*Parsing SCS Descriptor list*/
	while ((*buf) == IE_SCS_DESCRIPTOR_ELEMENT_ID) {
		leftlen = *(buf+1);
		buf += 2;
		len -= 2;
		if (leftlen > len) {
			DBGPRINT(RT_DEBUG_ERROR, "%s, len:%u, leftlen:%d.\n", __func__, len, leftlen);
			break;
		}
		if (scs_num >= 10) {
			DBGPRINT(RT_DEBUG_ERROR, "%s, SCS Descriptor number exceed 10 in SCS Request.\n", __func__);
			break;
		}

		memset(&scs_param, 0, sizeof(scs_param));
		COPY_MAC_ADDR(scs_param.sta_mac, peermac);

		scs_param.scsid = *buf;
		scs_param.request_type = *(buf+1);
		buf += 2;
		len -= 2;
		leftlen -= 2;

		sta_code[scs_num].scsid = scs_param.scsid;

		while (leftlen > 0) {
			if ((*buf) == IE_INTRA_ACCESS_CATE_PRI_ID) {
				subielen = *(buf+1);
				buf += 2;
				len -= 2;
				scs_param.up = (*buf) & 0x07;
				scs_param.alt_queue = (*buf) & 0x08;
				scs_param.drop_elig = (*buf) & 0x10;
			} else if ((*buf) == IE_TCLAS_PROCESSING_ID) {
				subielen = *(buf+1);
				buf += 2;
				len -= 2;
				scs_param.processing = *buf;
			} else if ((*buf) == IE_TCLAS_ELEMENT_ID) {
				subielen = *(buf+1);
				buf += 2;
				len -= 2;
				if (scs_param.tclas_num >= MAX_TCLAS_NUM) {
					DBGPRINT(RT_DEBUG_ERROR,
						"warning, tclas element num(%d) exceed %d.\n",
						scs_param.tclas_num,
						MAX_TCLAS_NUM);
					buf += subielen;
					len -= subielen;
					leftlen -= (subielen + 2);
					break;
				}

				ptclas = &scs_param.tclas_elem[scs_param.tclas_num];

				if (*buf != 0xFF)
					DBGPRINT(RT_DEBUG_ERROR, "incorrect tclas element with up:%d .\n", *buf);

				if ((*(buf+1)) == CLASSIFIER_TYPE4) {
					ptclas->type4.cs_type = *(buf+1);
					ptclas->type4.cs_mask = *(buf+2);
					ptclas->type4.version = *(buf+3);
					if (ptclas->type4.version == VER_IPV4) {
						ptclas->type4.srcIp.ipv4 = *((u32*)(buf+4));
						ptclas->type4.destIp.ipv4 = *((u32*)(buf+8));
						ptclas->type4.srcPort = *((u16*)(buf+12));
						ptclas->type4.destPort = *((u16*)(buf+14));
						ptclas->type4.DSCP = *(buf+16);
						ptclas->type4.u.protocol = *(buf+17);
					} else if (ptclas->type4.version == VER_IPV6) {
						memcpy(ptclas->type4.srcIp.ipv6, (buf+4), 16);
						memcpy(ptclas->type4.destIp.ipv6, (buf+20), 16);
						ptclas->type4.srcPort = *((u16*)(buf+36));
						ptclas->type4.destPort = *((u16*)(buf+38));
						ptclas->type4.DSCP = *(buf+40);
						ptclas->type4.u.nextheader = *(buf+41);
						memcpy(ptclas->type4.flowLabel, (buf+42), 3);
					}
					scs_param.tclas_num++;
				} else if ((*(buf+1)) == CLASSIFIER_TYPE10) {
					ptclas->type10.cs_type = *(buf+1);
					ptclas->type10.protocolInstance = *(buf+2);
					ptclas->type10.u.protocol = *(buf+3);
					filterlen = (subielen - 4) / 2;
					if (filterlen <= MAX_FILTER_LEN) {
						filtervalue = (buf + 4);
						filtermask = (filtervalue + filterlen);

						valid_len = 0;
						for (i = 0; i < filterlen; i++) {
							if(filtermask[i]) {
								ptclas->type10.filterValue[valid_len] = filtervalue[i];
								ptclas->type10.filterMask[valid_len] = filtermask[i];
								valid_len++;
							}
						}
						ptclas->type10.filterlen= valid_len;
					} else {
						DBGPRINT(RT_DEBUG_ERROR,
							"%s, filter value is too large, filterlen = %d.\n",
							__func__, filterlen);
						ptclas->type10.filterlen = 0;
					}
					scs_param.tclas_num++;
				} else {
					sta_code[scs_num].stacode = STA_REQUESTED_NOT_SUPPORTED;
					DBGPRINT(RT_DEBUG_ERROR, "%s, not support scs classifier type = %d.\n",
						__func__, (*(buf+1)));
				}
			} else {
				DBGPRINT(RT_DEBUG_ERROR, "%s, unkown subelement, ID:%d, len:%d.\n",
						__func__, *buf, (*(buf+1)));
				subielen = *(buf+1);
				buf += 2;
				len -= 2;
			}
			buf += subielen;
			len -= subielen;
			leftlen -= (subielen + 2);
		}
		/*Update SCS Configuration*/
		sendbuf[0] = 1;
		memcpy(&sendbuf[1], &scs_param, sizeof(scs_param));
		wapp_drv_send_event(NL_SET_SCS_CONFIG, sendbuf, 1 + sizeof(scs_param));

		sta_code[scs_num].stacode = update_scs_configuration(wapp, &scs_param);
		scs_num++;
	}

	/*build scs response action frame and send to peer.*/
	len = build_scs_resp_frame(token, sta_code, scs_num, sendbuf, sizeof(sendbuf));
	wapp_drv_send_action_frame(OID_SEND_QOS_ACTION_FRAME, wapp, wdev, channel, 0, peermac, sendbuf, len);

	return;
}
#endif

void wapp_send_qos_map_configure_frame(struct wifi_app *wapp, const char *ifname,
	unsigned int channel, const u8 *dst, struct dscp_exception *dscp_ex, u8 ex_cnt, struct dscp_range *dscp_rg)
{
	u32 len = 0;
	u8 buf[64] = {0}, *pos, i = 0;
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!ifname) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return;
	}
	wdev = wapp_dev_list_lookup_by_ifname(wapp, ifname);

	if (!dscp_ex || !dscp_rg || !wdev)
		return;

	memset(buf, 0, sizeof(buf));
	len = 0;
	buf[0] = CATEGORY_QOS;
	buf[1] = ACT_QOSMAP_CONFIG;
	len += 2;

	buf[2] = IE_QOS_MAP_SET;
	buf[3] = ex_cnt * 2 + 16;
	len += 2;

	pos = &buf[4];
	/* dscp exception */
	for (i = 0; i < ex_cnt && i < MAX_EXCEPT_CNT; i++) {
		*pos = dscp_ex[i].dscp;
		*(pos+1) = dscp_ex[i].up;;
		pos += 2;
		len += 2;
	}

	/* dscp range */
	for (i = 0; i < UP_MAX; i++) {
		*pos = dscp_rg[i].low;
		*(pos+1) = dscp_rg[i].high;
		pos += 2;
		len += 2;
	}

	wapp_drv_send_action_frame(OID_SEND_QOS_ACTION_FRAME, wapp, wdev, channel, 0, dst, buf, len);
}

int wapp_drv_send_up_tuple_expired_notify(
		struct wifi_app *wapp, const char *ifname, char *buf, u32 buflen)
{
	if (!wapp->drv_ops || !wapp->drv_ops->drv_send_up_tuple_expired)
		return 0;

	return wapp->drv_ops->drv_send_up_tuple_expired(wapp->drv_data, ifname, buf, buflen);
}

void wapp_handle_qos_action_frame(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	u32 len = 0;
	u8 *buf = NULL, *frmbuf = NULL;
	u8 channel = 0;
	u8 peermac[MAC_ADDR_LEN] = {0};
	struct wapp_dev *wdev = NULL;
	struct wapp_qos_action_frame *act_frm = NULL;

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev || !event_data)
		return;

	act_frm = (struct wapp_qos_action_frame *)&event_data->qos_frm;
	if (!act_frm) {
		DBGPRINT(RT_DEBUG_ERROR, "%s, qos action frame pointer is NULL.\n", __func__);
		return;
	}

	if (!act_frm->frm_len ||
		act_frm->frm_len > (sizeof(*event_data) - offsetof(struct wapp_qos_action_frame, frm))) {
		DBGPRINT(RT_DEBUG_ERROR, "%s, qos action frame frm_len:%d is invalid.\n",
			__func__, act_frm->frm_len);
		return;
	}

	len = act_frm->frm_len;
	channel = act_frm->chan;
	frmbuf = (u8 *)os_zalloc(len);
	if (!frmbuf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s, alloc memory fail.\n", __func__);
		return;
	}
	memcpy(frmbuf, act_frm->frm, len);
	memcpy(peermac, act_frm->src, MAC_ADDR_LEN);
	buf = frmbuf;

	/* skipping the wlan header part */
	buf += _802_11_HEADER;
	len -= _802_11_HEADER;
	if (buf[0] == CATEGORY_RAVS) {
		if ((buf[1] == ACT_MSCS_REQ) || (buf[1] == ACT_MSCS_RSP))
			wapp_mscs_action_frame_process(wapp, wdev, channel, peermac, buf, len);
#ifdef QOS_R2
		else if ((buf[1] == ACT_SCS_REQ) || (buf[1] == ACT_SCS_RSP))
			wapp_scs_action_frame_process(wapp, wdev, channel, peermac, buf, len);
#endif
		else
			DBGPRINT(RT_DEBUG_ERROR, "%s: unknown action frame, action:%d\n", __func__, buf[1]);
#ifdef QOS_R2
	} else if((buf[0] == CATEGORY_VEND_SPECIFIC_PROTECTED || buf[0] == CATEGORY_VEND_SPECIFIC)) {
		if ((memcmp(&buf[1], WFA_OUI, 3) == 0) && (buf[4] == QOS_ACT_FRM_OUI_TYPE)) {
			if (buf[5] == DSCP_POLICY_QRY || buf[5] == DSCP_POLICY_RSP) {
				wapp_dscp_policy_action_frame_process(wapp, wdev, channel, peermac, buf, len);
			} else if (buf[5] == DSCP_POLICY_REQ) {
				DBGPRINT(RT_DEBUG_ERROR, "%s: Received DSCP Policy Request Frame\n", __func__);
			} else {
				DBGPRINT(RT_DEBUG_ERROR, "%s: unknow DSCP Policy Frame, Type:%d\n", __func__, buf[1]);
			}
		} else {
			DBGPRINT(RT_DEBUG_ERROR, "%s: unknow WFA_OUI:0x%02x%02x%02x, OUIType:0x%x\n",
				__func__, buf[1], buf[2], buf[3], buf[4]);
		}
#endif
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "%s: unknown action frame, category:%d\n", __func__, buf[0]);
	}

	os_free(frmbuf);
}

void wapp_handle_mscs_descriptor_element(struct wifi_app *wapp,
		u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct classifier_parameter *cs_elem = NULL;
	u16 sta_code = STA_SUCCESS;
	u32 len = 0;

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev || !event_data)
		return;

	cs_elem = (struct classifier_parameter *)&event_data->qos_frm;

	if (cs_elem->request_type == REQUEST_TYPE_REMOVE) {
		sta_code = update_mscs_configuration(wapp, cs_elem);
		sendbuf[0] = 1;
		memcpy(&sendbuf[1], cs_elem, sizeof(*cs_elem));
		wapp_drv_send_event(NL_SET_MSCS_CONFIG, sendbuf, 1 + sizeof(*cs_elem));
	} else if ((cs_elem->request_type == REQUEST_TYPE_ADD ||
			cs_elem->request_type == REQUEST_TYPE_CHANGE)) {
		sta_code = update_mscs_configuration(wapp, cs_elem);
		sendbuf[0] = 1;
		memcpy(&sendbuf[1], cs_elem, sizeof(*cs_elem));
		wapp_drv_send_event(NL_SET_MSCS_CONFIG, sendbuf, 1 + sizeof(*cs_elem));
	} else
		sta_code = STA_TERMINATED;

	if (sta_code != STA_SUCCESS) {
		/*build mscs response action frame and send to peer.*/
		len = build_mscs_resp_frame(0, sta_code, sendbuf, sizeof(sendbuf));
		wapp_drv_send_action_frame(OID_SEND_QOS_ACTION_FRAME, wapp, wdev,
			0, 0, cs_elem->sta_mac, sendbuf, len);
	}
	return;
}

void wdev_handle_vend_spec_up_tuple_event(struct wifi_app *wapp,
		u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_vend_spec_classifier_para_report *cs_elem = NULL;
	u32 offset = 0;

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev || !event_data)
		return;

	offset = offsetof(struct wapp_vend_spec_classifier_para_report, ifname);
	cs_elem = (struct wapp_vend_spec_classifier_para_report *)&event_data->qos_frm;

	sendbuf[0] = 1;
	memcpy(&sendbuf[1], cs_elem, sizeof(*cs_elem) - IFNAMSIZ);
	memcpy(&sendbuf[1 + offset], wdev->ifname, IFNAMSIZ);
	wapp_drv_send_event(NL_SET_VEND_SPECIFIC_CONFIG, sendbuf, 1 + sizeof(*cs_elem));

	return;
}

void qos_teardown_client(struct wifi_app *wapp, const char *ifname, u8 *climac)
{
	u32 len = 0;
	struct wapp_dev *wdev = NULL;
	struct classifier_parameter cs_elem;

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifname(wapp, ifname);
	if (!wdev || !climac)
		return;

	len = build_mscs_resp_frame(0, STA_TERMINATED, sendbuf, sizeof(sendbuf));
	wapp_drv_send_action_frame(OID_SEND_QOS_ACTION_FRAME, wapp, wdev,
		0, 0, climac, sendbuf, len);

	memset(sendbuf, 0, sizeof(sendbuf));
	memset(&cs_elem, 0, sizeof(cs_elem));
	memcpy(cs_elem.sta_mac, climac, MAC_ADDR_LEN);
	cs_elem.request_type = REQUEST_TYPE_REMOVE;
	update_mscs_configuration(wapp, &cs_elem);
	sendbuf[0] = 1;
	memcpy(&sendbuf[1], &cs_elem, sizeof(cs_elem));
	wapp_drv_send_event(NL_SET_MSCS_CONFIG, sendbuf, 1 + sizeof(cs_elem));
}

#endif /* QOS_R1 */
