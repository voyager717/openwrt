/****************************************************************************
 *
 * Copyright (c) 2016 Wi-Fi Alliance
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *****************************************************************************/

/*
 * mtk_hostapd.h:
 *   Defines general types and enum
 */

#ifndef MTK_HOSTAPD_H
#define MTK_HOSTAPD_H

#define ETC_CONFIG_WIRELESS "/etc/config/wireless"
#define HOSTAPD_BIN "/var/run/hostapd"
#define HOSTAPD_RADIO1_INF_NAME "wlan0"
#define HOSTAPD_RADIO2_INF_NAME "wlan1"
#define HOSTAPD_RADIO1_PROFILE "/var/run/hostapd-phy0.conf"
#define HOSTAPD_RADIO2_PROFILE "/var/run/hostapd-phy1.conf"
#define HOSTAPD_TEMP_OUTPUT "/tmp/hostapd_output"

enum ENUM_CHANNEL_OFFSET {
	CHANNEL_OFFSET_NONE,
	CHANNEL_OFFSET_BELOW,
	CHANNEL_OFFSET_ABOVE,
};

enum ENUM_CHANNEL_WIDTH {
	CHANNEL_WIDTH_AUTO,
	CHANNEL_WIDTH_20,
	CHANNEL_WIDTH_40,
	CHANNEL_WIDTH_80,
	CHANNEL_WIDTH_160,
	CHANNEL_WIDTH_80_80,
};

enum ENUM_AP_MODE {
	AP_MODE_NONE,
	AP_MODE_11b,
	AP_MODE_11bg,
	AP_MODE_11bgn,
	AP_MODE_11a,
	AP_MODE_11g,
	AP_MODE_11na,
	AP_MODE_11ng,
	AP_MODE_11ac,
	AP_MODE_11ad,
	AP_MODE_11ax,
};

enum ENUM_KEY_MGNT_TYPE {
	KEY_MGNT_TYPE_UNKNOWN,
	KEY_MGNT_TYPE_OPEN,
	KEY_MGNT_TYPE_WPA_PSK,
	KEY_MGNT_TYPE_WPA2_PSK,
	KEY_MGNT_TYPE_WPA_EAP,
	KEY_MGNT_TYPE_WPA2_EAP,
	KEY_MGNT_TYPE_WPA2_EAP_MIXED,
	KEY_MGNT_TYPE_WPA2_PSK_MIXED,
	KEY_MGNT_TYPE_SUITEB,
	KEY_MGNT_TYPE_WPA2_SAE,
	KEY_MGNT_TYPE_WPA2_PSK_SAE,
	KEY_MGNT_TYPE_WPA2_OWE,
};

enum ENUM_ENCP_TYPE {
	ENCP_TYPE_UNKNOWN,
	ENCP_TYPE_NONE,
	ENCP_TYPE_WEP,
	ENCP_TYPE_TKIP,
	ENCP_TYPE_CCMP,
	ENCP_TYPE_CCMP_256,
	ENCP_TYPE_GCMP_128,
	ENCP_TYPE_GCMP_256,
	ENCP_TYPE_CCMP_TKIP,
};

enum ENUM_PROGRAME_TYPE {
	PROGRAME_TYPE_NONE,
	PROGRAME_TYPE_WPA2,
	PROGRAME_TYPE_WMM,
	PROGRAME_TYPE_HS2,
	PROGRAME_TYPE_HS2_R2,
	PROGRAME_TYPE_VHT,
	PROGRAME_TYPE_11N,
	PROGRAME_TYPE_60GHZ,
	PROGRAME_TYPE_LOC,
	PROGRAME_TYPE_WPA3,
	PROGRAME_TYPE_HE,
	PROGRAME_TYPE_MBO,
};

enum ENUM_TLV_TYPE {
	/* SSID, 0x00 */
	TLV_TYPE_SSID = 0,
	/* RptCond, 0x01 */
	TLV_TYPE_BECON_REPT_INF = 1,
	/* RptDet, 0x02 */
	TLV_TYPE_BECON_REPT_DET = 2,
	/* ReqInfo,0x0a */
	TLV_TYPE_REQ_INFO = 10,
	/* APChanRpt, 0x33 */
	TLV_TYPE_AP_CHAN_RPT = 51,
	/* LastBeaconRptIndication, 0xa4 */
	TLV_TYPE_BECON_REPT_IND_REQ = 164,
};

typedef int (*get_cmd_tag_ptr)(char *);

typedef int (*mtk_ap_exec_ptr)(uint8_t *, uint8_t *, uint8_t *, int, int *, int);

int hostapd_ap_config_commit(int, uint8_t *, int *, uint8_t *);
int hostapd_ap_deauth_sta(int, uint8_t *, int *, uint8_t *);
int hostapd_ap_reset_default(int, uint8_t *, int *, uint8_t *);
int hostapd_ap_set_rfeature(int, uint8_t *, int *, uint8_t *);
int hostapd_ap_set_wireless(int, uint8_t *, int *, uint8_t *);
int hostapd_ap_set_security(int, uint8_t *, int *, uint8_t *);
int hostapd_ap_set_radius(int, uint8_t *, int *, uint8_t *);
int hostapd_ap_exec(uint8_t *, uint8_t *, uint8_t *, int, int *, int);
int hostapd_get_cmd_tag(char *);
int hostapd_ap_set_pmf(int, uint8_t *, int *, uint8_t *);
int hostapd_ap_set_apqos(int, uint8_t *, int *, uint8_t *);
int hostapd_ap_set_staqos(int, uint8_t *, int *, uint8_t *);
int hostapd_dev_send_frame(int, uint8_t *, int *, uint8_t *);

#endif
