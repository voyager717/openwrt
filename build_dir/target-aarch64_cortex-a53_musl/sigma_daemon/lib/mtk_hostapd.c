#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <linux/socket.h>
#include <linux/wireless.h>
#include <poll.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <time.h>

#include "mtk_hostapd.h"
#include "wfa_main.h"
#include "wfa_types.h"
#include "mtk_ap.h"
#include "mtk_parse.h"
#include "wfa_sock.h"
#include "wfa_debug.h"

#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]

char gCmdStr[WFA_CMD_STR_SZ];

typeDUT_t hostapd_dut_tbl[] = {
    {0, "NO_USED_STRING", NULL, NULL, NULL},
    {WFA_GET_VERSION_TLV, "ca_get_version", parse_ap_ca_version, mtk_ap_ca_version, mtk_ap_ca_version_resp},
    {WFA_DEVICE_GET_INFO_TLV, "device_get_info", parse_device_get_info, mtk_device_get_info, mtk_device_get_info_resp},
    {WFA_AP_CA_VERSION_TLV, "ap_ca_version", parse_ap_ca_version, mtk_ap_ca_version, mtk_ap_ca_version_resp},
    {WFA_AP_CONFIG_COMMIT_TLV, "ap_config_commit", parse_ap_config_commit, hostapd_ap_config_commit,
     mtk_ap_config_commit_resp},
    {WFA_AP_DEAUTH_STA_TLV, "ap_deauth_sta", parse_ap_deauth_sta, hostapd_ap_deauth_sta, mtk_ap_deauth_sta_resp},
    {WFA_AP_GET_MAC_ADDRESS_TLV, "ap_get_mac_address", parse_ap_get_mac_address, mtk_ap_get_mac_address,
     mtk_ap_get_mac_address_resp},
    {WFA_AP_RESET_DEFAULT_TLV, "ap_reset_default", parse_ap_reset_default, hostapd_ap_reset_default,
     mtk_ap_reset_default_resp},
    {WFA_AP_SET_RFEATURE_TLV, "ap_set_rfeature", parse_ap_set_rfeature, hostapd_ap_set_rfeature,
     mtk_ap_set_rfeature_resp},
    {WFA_AP_SET_WIRELESS_TLV, "ap_set_wireless", parse_ap_set_wireless, hostapd_ap_set_wireless,
     mtk_ap_set_wireless_resp},
    {WFA_AP_SET_SECURITY_TLV, "ap_set_security", parse_ap_set_security, hostapd_ap_set_security,
     mtk_ap_set_security_resp},
    {WFA_AP_SET_RADIUS_TLV, "ap_set_radius", parse_ap_set_radius, hostapd_ap_set_radius, mtk_ap_set_radius_resp},
    {WFA_AP_IGNORE_CAPI_TLV, "AccessPoint", parse_ap_reset_default, hostapd_ap_reset_default,
     mtk_ap_reset_default_resp},
    {WFA_AP_SET_11N_WIRELESS_TLV, "ap_set_11n_wireless", parse_ap_set_wireless, hostapd_ap_set_wireless,
     mtk_ap_set_wireless_resp},
    {WFA_AP_SET_PMF_TLV, "ap_set_pmf", parse_ap_set_pmf, hostapd_ap_set_pmf, mtk_ap_set_pmf_resp},
    {WFA_AP_SET_APQOS_TLV, "ap_set_apqos", parse_ap_set_apqos, hostapd_ap_set_apqos, mtk_ap_set_apqos_resp},
    {WFA_AP_SET_STAQOS_TLV, "ap_set_staqos", parse_ap_set_staqos, hostapd_ap_set_staqos, mtk_ap_set_staqos_resp},
    {WFA_DEV_SEND_FRAME_TLV, "dev_send_frame", parse_dev_send_frame, hostapd_dev_send_frame, mtk_dev_send_frame_resp},
    {WFA_TRAFFIC_AGENT_RESET_TLV, "traffic_agent_reset", parse_traffic_agent_reset, mtk_traffic_agent_reset,
     mtk_traffic_agent_reset_resp},
/* TODO: other capi cmd */
#if 0
	{WFA_AP_SEND_ADDBA_REQ_TLV, "ap_send_addba_req", parse_ap_send_addba_req, mtk_ap_send_addba_req,
	 mtk_ap_send_addba_req_resp},
	{WFA_AP_SEND_BCNRPT_REQ_TLV, "ap_send_bcnrpt_req", parse_ap_send_bcnrpt_req, mtk_ap_send_bcnrpt_req,
	 mtk_ap_send_bcnrpt_req_resp},
	{WFA_AP_SEND_BSSTRANS_MGMT_REQ_TLV, "ap_send_bsstrans_mgmt_req", parse_ap_send_bsstrans_mgmt_req,
	 mtk_ap_send_bsstrans_mgmt_req, mtk_ap_send_bsstrans_mgmt_req_resp},
	{WFA_AP_SEND_LINK_MEA_REQ_TLV, "ap_send_link_mea_req", parse_ap_send_link_mea_req, mtk_ap_send_link_mea_req,
	 mtk_ap_send_link_mea_req_resp},
	{WFA_AP_SEND_TSMRPT_REQ_TLV, "ap_send_tsmrpt_req", parse_ap_send_tsmrpt_req, mtk_ap_send_tsmrpt_req,
	 mtk_ap_send_tsmrpt_req_resp},

	{WFA_AP_SET_11D_TLV, "ap_set_11d", parse_ap_set_11d, mtk_ap_set_11d, mtk_ap_set_11d_resp},
	{WFA_AP_SET_11H_TLV, "ap_set_11h", parse_ap_set_11h, mtk_ap_set_11h, mtk_ap_set_11h_resp},
	{WFA_AP_SET_HS2_TLV, "ap_set_hs2", parse_ap_set_hs2, mtk_ap_set_hs2, mtk_ap_set_hs2_resp},

	{WFA_AP_SET_RRM_TLV, "ap_set_rrm", parse_ap_set_rrm, mtk_ap_set_rrm, mtk_ap_set_rrm_resp},

	{WFA_DEV_CONFIGURE_IE_TLV, "dev_configure_ie", parse_dev_configure_ie, mtk_dev_configure_ie,
	 mtk_dev_configure_ie_resp},
	{WFA_DEV_EXEC_ACTION_TLV, "dev_exec_action", parse_dev_exec_action, mtk_dev_exec_action,
	 mtk_dev_exec_action_resp},
	{WFA_AP_GET_PARAMETER_TLV, "ap_get_parameter", parse_ap_get_parameter, mtk_ap_get_parameter,
	 mtk_ap_get_parameter_resp},

	{WFA_TRAFFIC_SEND_PING_TLV, "traffic_send_ping", parse_traffic_send_ping, mtk_traffic_send_ping,
	 mtk_traffic_send_ping_resp},
	{WFA_TRAFFIC_STOP_PING_TLV, "traffic_stop_ping", parse_traffic_stop_ping, mtk_traffic_stop_ping,
	 mtk_traffic_stop_ping_resp},

#endif
    {-1, "", NULL, NULL, NULL},
};

int hostapd_str_to_hex(char *out_buf, int out_size, char *in_str, int in_len)
{
	int index = 0, offset = 0;
	char *fmt = "%02x";

	DPRINT_INFO(WFA_OUT, "input str %s\n", in_str);
	for (index = 0; index < in_len; index++)
		offset += snprintf(&out_buf[index << 1], out_size - offset - 1, fmt, in_str[index]);

	DPRINT_INFO(WFA_OUT, "out hex str: %s\n", out_buf);
	return offset;
}

int hostapd_parameter_to_tlv(char *out_buf, int out_size, char *in_str, int in_len, enum ENUM_TLV_TYPE type,
			     intf_desc_t *def_intf)
{
	int offset = 0;
	char hex_buf[out_size], chan_cnt = 0;
	char *fmt = "%02x";
	int ret_len = 0;

	memset(hex_buf, 0, sizeof(hex_buf));

	switch (type) {
	case TLV_TYPE_SSID:
		ret_len = hostapd_str_to_hex(hex_buf, sizeof(hex_buf), in_str, strlen(in_str));
		if (ret_len >= out_size)
			DPRINT_INFO(WFA_OUT, "SSID too long\n");

		offset = 0;
		/* t */
		offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, TLV_TYPE_SSID);
		/* l */
		offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, in_len);
		/* v */
		offset += snprintf(out_buf + offset, out_size - offset - 1, "%s", hex_buf);
		DPRINT_INFO(WFA_OUT, "SSID tlv: %s\n", out_buf);
		break;

	case TLV_TYPE_BECON_REPT_INF:
		DPRINT_INFO(WFA_OUT, "RptCond : %s\n", in_str);
		/* t */
		offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, TLV_TYPE_BECON_REPT_INF);
		/* l */
		offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, 2);
		/* v */
		if (!strncmp(in_str, "0", min(in_len, 2)))
			offset += snprintf(out_buf + offset, out_size - offset - 1, "%s", "0000");
		DPRINT_INFO(WFA_OUT, "RptCond tlv: %s\n", out_buf);
		break;

	case TLV_TYPE_BECON_REPT_DET:
		DPRINT_INFO(WFA_OUT, "RptDet : %s\n", in_str);
		/* t */
		offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, TLV_TYPE_BECON_REPT_DET);
		/* l */
		offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, 1);
		/* v */
		if (!strncmp(in_str, "1", min(in_len, 2)))
			offset += snprintf(out_buf + offset, out_size - offset - 1, "%s", "01");
		else if (!strncmp(in_str, "0", min(in_len, 2)))
			offset += snprintf(out_buf + offset, out_size - offset - 1, "%s", "00");
		DPRINT_INFO(WFA_OUT, "RptDet tlv: %s\n", out_buf);
		break;

	case TLV_TYPE_REQ_INFO:
		if (!in_len)
			return 0;

		DPRINT_INFO(WFA_OUT, "ReqInfo : %s\n", in_str);
		if (strncmp(in_str, "0_54_221", strlen("0_54_221")) == 0) {
			/* t */
			offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, TLV_TYPE_REQ_INFO);
			/* l */
			offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, 13);
			/* v */
			offset += snprintf(out_buf + offset, out_size - offset - 1, "%s", "0001322d3d7fbfc0c3304636dd");
		}
		DPRINT_INFO(WFA_OUT, "ReqInfo tlv: %s\n", out_buf);
		break;

	case TLV_TYPE_AP_CHAN_RPT:
		if (!in_len)
			return 0;
		DPRINT_INFO(WFA_OUT, "APChanRpt : %s\n", in_str);

		if (strstr(in_str, "1")) {
			offset += snprintf(hex_buf + offset, out_size - offset - 1, fmt, 1);
			chan_cnt++;
		}
		if (strstr(in_str, "6")) {
			chan_cnt++;
			offset += snprintf(hex_buf + offset, out_size - offset - 1, fmt, 6);
		}
		if (strstr(in_str, "36")) {
			chan_cnt++;
			offset += snprintf(hex_buf + offset, out_size - offset - 1, fmt, 36);
		}
		if (strstr(in_str, "48")) {
			chan_cnt++;
			offset += snprintf(hex_buf + offset, out_size - offset - 1, fmt, 48);
		}

		offset = 0;
		/* t */
		offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, TLV_TYPE_AP_CHAN_RPT);
		/* l */
		offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, chan_cnt + 1);
		/* v */
		/* opclass */
		offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, def_intf->frame_opclass);
		/* channel list */
		offset += snprintf(out_buf + offset, out_size - offset - 1, "%s", hex_buf);
		DPRINT_INFO(WFA_OUT, "APChanRpt tlv: %s\n", out_buf);
		break;

	case TLV_TYPE_BECON_REPT_IND_REQ:
		DPRINT_INFO(WFA_OUT, "LastBeaconRptIndication : %s\n", in_str);
		/* t */
		offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, TLV_TYPE_BECON_REPT_IND_REQ);
		/* l */
		offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, 1);
		/* v */
		if (strstr(in_str, "1"))
			offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, 1);
		else if (strstr(in_str, "0"))
			offset += snprintf(out_buf + offset, out_size - offset - 1, fmt, 0);
		DPRINT_INFO(WFA_OUT, "LastBeaconRptIndication tlv: %s\n", out_buf);
		break;

	default:
		break;
	}

	return offset;
}

void hostapd_handle_post_cmd(mtk_ap_buf_t *mtk_ap_buf)
{
	handle_post_cmd(mtk_ap_buf);
	sleep(1);
}

char *hostapd_apmode_2_string(enum ENUM_AP_MODE ap_mode)
{
	switch (ap_mode) {
	case AP_MODE_11b:
		return "AP_MODE_11b";
	case AP_MODE_11bg:
		return "AP_MODE_11bg";
	case AP_MODE_11bgn:
		return "AP_MODE_11bgn";
	case AP_MODE_11a:
		return "AP_MODE_11a";
	case AP_MODE_11g:
		return "AP_MODE_11g";
	case AP_MODE_11na:
		return "AP_MODE_11na";
	case AP_MODE_11ng:
		return "AP_MODE_11ng";
	case AP_MODE_11ac:
		return "AP_MODE_11ac";
	case AP_MODE_11ad:
		return "AP_MODE_11ad";
	case AP_MODE_11ax:
		return "AP_MODE_11ax";
	default:
		return "AP_MODE_NONE";
	}
}

void hostapd_ch_bw_get_freq(char ch, enum ENUM_CHANNEL_WIDTH bw, int *freq, int *cent_freq)
{
	if (!freq || !cent_freq)
		return;

	switch (ch) {
	case 36:
		*freq = 5180;
		DPRINT_INFO(WFA_OUT, "freq %d\n", *freq);
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5190;
			DPRINT_INFO(WFA_OUT, "cent_freq %d\n", *cent_freq);
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5210;
			DPRINT_INFO(WFA_OUT, "cent_freq %d\n", *cent_freq);
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5250;
			DPRINT_INFO(WFA_OUT, "cent_freq %d\n", *cent_freq);
			break;
		default:
			*cent_freq = 5180;
			DPRINT_INFO(WFA_OUT, "cent_freq %d\n", *cent_freq);
			break;
		}
		break;
	case 40:
		*freq = 5200;
		DPRINT_INFO(WFA_OUT, "freq %d\n", *freq);
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5190;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5210;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5250;
			break;
		default:
			*cent_freq = 5200;
			break;
		}
		break;
	case 44:
		*freq = 5220;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5230;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5210;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5250;
			break;
		default:
			*cent_freq = 5220;
			break;
		}
		break;
	case 48:
		*freq = 5240;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5230;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5210;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5250;
			break;
		default:
			*cent_freq = 5240;
			break;
		}
		break;
	case 52:
		*freq = 5260;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5270;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5290;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5250;
			break;
		default:
			*cent_freq = 5260;
			break;
		}
		break;
	case 56:
		*freq = 5280;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5270;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5290;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5250;
			break;
		default:
			*cent_freq = 5260;
			break;
		}
		break;
	case 60:
		*freq = 5300;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5310;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5290;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5250;
			break;
		default:
			*cent_freq = 5300;
			break;
		}
		break;
	case 64:
		*freq = 5320;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5310;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5290;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5250;
			break;
		default:
			*cent_freq = 5320;
			break;
		}
		break;
	case 100:
		*freq = 5500;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5510;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5530;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5570;
			break;
		default:
			*cent_freq = 5500;
			break;
		}
		break;
	case 104:
		*freq = 5520;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5510;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5530;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5570;
			break;
		default:
			*cent_freq = 5520;
			break;
		}
		break;
	case 108:
		*freq = 5540;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5550;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5530;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5570;
			break;
		default:
			*cent_freq = 5540;
			break;
		}
		break;
	case 112:
		*freq = 5560;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5550;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5530;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5570;
			break;
		default:
			*cent_freq = 5560;
			break;
		}
		break;
	case 116:
		*freq = 5580;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5590;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5610;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5570;
			break;
		default:
			*cent_freq = 5580;
			break;
		}
		break;
	case 120:
		*freq = 5600;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5590;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5610;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5570;
			break;
		default:
			*cent_freq = 5600;
			break;
		}
		break;
	case 124:
		*freq = 5620;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5630;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5610;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5570;
			break;
		default:
			*cent_freq = 5620;
			break;
		}
		break;
	case 128:
		*freq = 5640;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5630;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5610;
			break;
		case CHANNEL_WIDTH_160:
			*cent_freq = 5570;
			break;
		default:
			*cent_freq = 5640;
			break;
		}
		break;
	case 132:
		*freq = 5660;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5670;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5690;
			break;
		default:
			*cent_freq = 5660;
			break;
		}
		break;
	case 136:
		*freq = 5680;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5670;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5690;
			break;
		default:
			*cent_freq = 5680;
			break;
		}
		break;
	case 140:
		*freq = 5700;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5710;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5690;
			break;
		default:
			*cent_freq = 5700;
			break;
		}
		break;
	case 144:
		*freq = 5720;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5710;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5690;
			break;
		default:
			*cent_freq = 5720;
			break;
		}
		break;
	case 149:
		*freq = 5745;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5755;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5775;
			break;
		default:
			*cent_freq = 5745;
			break;
		}
		break;
	case 153:
		*freq = 5765;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5755;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5775;
			break;
		default:
			*cent_freq = 5765;
			break;
		}
		break;
	case 157:
		*freq = 5785;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5795;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5775;
			break;
		default:
			*cent_freq = 5785;
			break;
		}
		break;
	case 161:
		*freq = 5805;
		switch (bw) {
		case CHANNEL_WIDTH_40:
			*cent_freq = 5795;
			break;
		case CHANNEL_WIDTH_80:
			*cent_freq = 5775;
			break;
		default:
			*cent_freq = 5805;
			break;
		}
		break;
	default:
		break;
	}
}

char *hostapd_wifi_mode_2_string(wifi_mode mode)
{
	switch (mode) {
	case WIFI_2G:
		return "WIFI_2G";
	case WIFI_5G:
		return "WIFI_5G";
	case WIFI_6G:
		return "WIFI_6G";
	default:
		return "NORMAL";
	}
}

int hostapd_get_cmd_output(char *out_buf, size_t out_len, char *cmd)
{
	FILE *f = NULL;

	DPRINT_INFO(WFA_OUT, "run cmd ==> %s\n", cmd);
	system(cmd);

	f = fopen(HOSTAPD_TEMP_OUTPUT, "r");
	if (f == NULL) {
		DPRINT_ERR(WFA_ERR, "file open fail\n");
		return WFA_ERROR;
	}
	fseek(f, 0, SEEK_END);
	if (!ftell(f))
		return WFA_ERROR;
	fseek(f, 0, SEEK_SET);
	fgets(out_buf, out_len, f);
	fclose(f);
	return WFA_SUCCESS;
}

static void hostapd_cli_cmd(intf_desc_t *def_intf, char *buf)
{
	if (!def_intf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return;
	}
	DPRINT_INFO(WFA_OUT, "%s(), buf: %s\n", __func__, buf);

	memset(gCmdStr, 0, sizeof(gCmdStr));
	snprintf(gCmdStr, sizeof(gCmdStr), "hostapd_cli -i %s -p %s %s", def_intf->name, def_intf->ctrl_inf, buf);
	DPRINT_INFO(WFA_OUT, "run command ==> %s\n", gCmdStr);
	system(gCmdStr);
}

int hostapd_get_cmd_tag(char *capi_name)
{
	int cmd_tag = 0;

	if (!capi_name) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return WFA_ERROR;
	}

	while (hostapd_dut_tbl[cmd_tag].type != -1) {
		if (strcmp(hostapd_dut_tbl[cmd_tag].name, capi_name) == 0)
			return cmd_tag;
		cmd_tag++;
	}
	return 0;
}

void hostapd_init_profile_name(mtk_ap_buf_t *mtk_ap_buf)
{
	char shellResult[WFA_CMD_STR_SZ];
	FILE *file = NULL;
	char line[256] = {0};
	char profile_name[60] = {0};
	char *start = NULL, *end = NULL;
	int i = 0;

	if (!mtk_ap_buf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return;
	}

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);

	if (!l1_valid) {
		file = fopen(PROFILE_INF, "r");
		if (file == NULL) {
			DPRINT_INFO(WFA_OUT, "Can't find profile info file: %s\n", PROFILE_INF);
			return;
		}

		while (fgets(line, sizeof(line), file)) {
			if ((line[0] == '\0') || (line[1] == '\0') || (strlen(line) < 3))
				continue;

			start = strstr(line, "main_ifname=");
			if (start == NULL)
				continue;
			/* Replace the last character from '\n' to '\0' */
			start[strlen(start) - 1] = '\0';
			start = start + 12;
			end = strstr(start, ";");
			if (end == NULL) {
				strcpy(mtk_ap_buf->profile_names[i].name, start);
				DPRINT_INFO(WFA_OUT, "interface %d: %s\n", i, mtk_ap_buf->profile_names[i].name);

				snprintf(profile_name, sizeof(profile_name) - 1, "/var/run/hostapd-phy%d.conf", i);

				strcpy(mtk_ap_buf->profile_names[i].profile, profile_name);
				DPRINT_INFO(WFA_OUT, "Profile %d: name %s\n", i, mtk_ap_buf->profile_names[i].profile);
				strcpy(mtk_ap_buf->profile_names[i].profile_bak, profile_name);
				strcat(mtk_ap_buf->profile_names[i].profile_bak, ".bak");
				DPRINT_INFO(WFA_OUT, "Profile bak %d: name %s\n", i,
					    mtk_ap_buf->profile_names[i].profile_bak);
				strcpy(mtk_ap_buf->profile_names[i].profile_cmt, profile_name);
				strcat(mtk_ap_buf->profile_names[i].profile_cmt, ".cmt");
				DPRINT_INFO(WFA_OUT, "Profile name %d for last committed : %s\n", i,
					    mtk_ap_buf->profile_names[i].profile_cmt);
				strcpy(mtk_ap_buf->profile_names[i].sigma_dut_profile, profile_name);
				strcpy(mtk_ap_buf->profile_names[i].sigma_tb_profile, profile_name);
				DPRINT_INFO(WFA_OUT, "sigma_dut_profile %d: %s\n", i,
					    mtk_ap_buf->profile_names[i].sigma_dut_profile);
				DPRINT_INFO(WFA_OUT, "sigma_tb_profile %d: %s\n", i,
					    mtk_ap_buf->profile_names[i].sigma_tb_profile);

				i++;
			} else {
				end[0] = '\0';
				strcpy(mtk_ap_buf->profile_names[i].name, start);

				snprintf(profile_name, sizeof(profile_name) - 1, "/var/run/hostapd-phy%d.conf", i);

				DPRINT_INFO(WFA_OUT, "interface %d: %s\n", i, mtk_ap_buf->profile_names[i].name);
				strcpy(mtk_ap_buf->profile_names[i].profile, profile_name);
				DPRINT_INFO(WFA_OUT, "Profile %d: name %s\n", i, mtk_ap_buf->profile_names[i].profile);
				strcpy(mtk_ap_buf->profile_names[i].profile_bak, profile_name);
				strcat(mtk_ap_buf->profile_names[i].profile_bak, ".bak");
				DPRINT_INFO(WFA_OUT, "Profile bak %d: name %s\n", i,
					    mtk_ap_buf->profile_names[i].profile_bak);
				strcpy(mtk_ap_buf->profile_names[i].profile_cmt, profile_name);
				strcat(mtk_ap_buf->profile_names[i].profile_cmt, ".cmt");
				DPRINT_INFO(WFA_OUT, "Profile name %d for last committed : %s\n", i,
					    mtk_ap_buf->profile_names[i].profile_cmt);
				strcpy(mtk_ap_buf->profile_names[i].sigma_dut_profile, profile_name);
				strcpy(mtk_ap_buf->profile_names[i].sigma_tb_profile, profile_name);
				DPRINT_INFO(WFA_OUT, "sigma_dut_profile %d: %s\n", i,
					    mtk_ap_buf->profile_names[i].sigma_dut_profile);
				DPRINT_INFO(WFA_OUT, "sigma_tb_profile %d: %s\n", i,
					    mtk_ap_buf->profile_names[i].sigma_tb_profile);
				i++;

				snprintf(profile_name, sizeof(profile_name) - 1, "/var/run/hostapd-phy%d.conf", i);
				strcpy(mtk_ap_buf->profile_names[i].name, end + 1);
				DPRINT_INFO(WFA_OUT, "interface %d: %s\n", i, mtk_ap_buf->profile_names[i].name);
				strcpy(mtk_ap_buf->profile_names[i].profile, profile_name);
				DPRINT_INFO(WFA_OUT, "Profile %d: name %s\n", i, mtk_ap_buf->profile_names[i].profile);
				strcpy(mtk_ap_buf->profile_names[i].profile_bak, profile_name);
				strcat(mtk_ap_buf->profile_names[i].profile_bak, ".bak");
				DPRINT_INFO(WFA_OUT, "Profile bak %d: name %s\n", i,
					    mtk_ap_buf->profile_names[i].profile_bak);
				strcpy(mtk_ap_buf->profile_names[i].profile_cmt, profile_name);
				strcat(mtk_ap_buf->profile_names[i].profile_cmt, ".cmt");
				DPRINT_INFO(WFA_OUT, "Profile name %d for last committed : %s\n", i,
					    mtk_ap_buf->profile_names[i].profile_cmt);
				strcpy(mtk_ap_buf->profile_names[i].sigma_dut_profile, profile_name);
				strcpy(mtk_ap_buf->profile_names[i].sigma_tb_profile, profile_name);
				DPRINT_INFO(WFA_OUT, "sigma_dut_profile %d: %s\n", i,
					    mtk_ap_buf->profile_names[i].sigma_dut_profile);
				DPRINT_INFO(WFA_OUT, "sigma_tb_profile %d: %s\n", i,
					    mtk_ap_buf->profile_names[i].sigma_tb_profile);
				i++;
			}
		}
		fclose(file);
		return;
	}

	memset(shellResult, 0, sizeof(shellResult));
	memset(gCmdStr, 0, sizeof(gCmdStr));
	snprintf(gCmdStr, sizeof(gCmdStr) - 1, "cat %s | grep %s | cut -d: -f 1 > %s", PROC_NET_WIRELESS,
		 HOSTAPD_RADIO1_INF_NAME, HOSTAPD_TEMP_OUTPUT);

	if (hostapd_get_cmd_output(shellResult, sizeof(shellResult), gCmdStr)) {
		DPRINT_INFO(WFA_OUT, "shell output:%s\n", shellResult);
		DPRINT_INFO(WFA_OUT, "%s not found\n", HOSTAPD_RADIO1_INF_NAME);
	} else {
		DPRINT_INFO(WFA_OUT, "shell output:%s\n", shellResult);
		/* /var/run/hostapd-phy0.conf first radio */
		strncpy(mtk_ap_buf->profile_names[0].name, HOSTAPD_RADIO1_INF_NAME,
			sizeof(mtk_ap_buf->profile_names[0].name) - 1);
		DPRINT_INFO(WFA_OUT, "Profile 1: inf_name %s\n", mtk_ap_buf->profile_names[0].name);
		strcpy(mtk_ap_buf->profile_names[0].profile, HOSTAPD_RADIO1_PROFILE);
		DPRINT_INFO(WFA_OUT, "Profile 1: name %s\n", mtk_ap_buf->profile_names[0].profile);
		strcpy(mtk_ap_buf->profile_names[0].profile_bak, HOSTAPD_RADIO1_PROFILE);
		strcat(mtk_ap_buf->profile_names[0].profile_bak, ".bak");
		DPRINT_INFO(WFA_OUT, "Profile bak 1: name %s\n", mtk_ap_buf->profile_names[0].profile_bak);
		strcpy(mtk_ap_buf->profile_names[0].profile_cmt, HOSTAPD_RADIO1_PROFILE);
		strcat(mtk_ap_buf->profile_names[0].profile_cmt, ".cmt");
		DPRINT_INFO(WFA_OUT, "Profile name 1 for last committed : %s\n",
			    mtk_ap_buf->profile_names[0].profile_cmt);

		strcpy(mtk_ap_buf->profile_names[0].sigma_dut_profile, HOSTAPD_RADIO1_PROFILE);
		strcpy(mtk_ap_buf->profile_names[0].sigma_tb_profile, HOSTAPD_RADIO1_PROFILE);
		DPRINT_INFO(WFA_OUT, "sigma_dut_profile 1: %s\n", mtk_ap_buf->profile_names[0].sigma_dut_profile);
		DPRINT_INFO(WFA_OUT, "sigma_tb_profile 1: %s\n", mtk_ap_buf->profile_names[0].sigma_tb_profile);
	}

	memset(shellResult, 0, sizeof(shellResult));
	memset(gCmdStr, 0, sizeof(gCmdStr));
	snprintf(gCmdStr, sizeof(gCmdStr) - 1, "cat %s | grep %s | cut -d: -f 1 > %s", PROC_NET_WIRELESS,
		 HOSTAPD_RADIO2_INF_NAME, HOSTAPD_TEMP_OUTPUT);
	if (hostapd_get_cmd_output(shellResult, sizeof(shellResult), gCmdStr)) {
		DPRINT_INFO(WFA_OUT, "shell output:%s\n", shellResult);
		DPRINT_INFO(WFA_OUT, "%s not found\n", HOSTAPD_RADIO2_INF_NAME);
	} else {
		DPRINT_INFO(WFA_OUT, "shell output:%s\n", shellResult);
		/* /var/run/hostapd-phy0.conf first radio */
		strncpy(mtk_ap_buf->profile_names[1].name, HOSTAPD_RADIO2_INF_NAME,
			sizeof(mtk_ap_buf->profile_names[1].name) - 1);
		DPRINT_INFO(WFA_OUT, "Profile 2: inf_name %s\n", mtk_ap_buf->profile_names[1].name);
		strcpy(mtk_ap_buf->profile_names[1].profile, HOSTAPD_RADIO2_PROFILE);
		DPRINT_INFO(WFA_OUT, "Profile 2: name %s\n", mtk_ap_buf->profile_names[1].profile);
		strcpy(mtk_ap_buf->profile_names[1].profile_bak, HOSTAPD_RADIO2_PROFILE);
		strcat(mtk_ap_buf->profile_names[1].profile_bak, ".bak");
		DPRINT_INFO(WFA_OUT, "Profile bak 2: name %s\n", mtk_ap_buf->profile_names[1].profile_bak);
		strcpy(mtk_ap_buf->profile_names[1].profile_cmt, HOSTAPD_RADIO2_PROFILE);
		strcat(mtk_ap_buf->profile_names[1].profile_cmt, ".cmt");
		DPRINT_INFO(WFA_OUT, "Profile name 2 for last committed : %s\n",
			    mtk_ap_buf->profile_names[1].profile_cmt);

		strcpy(mtk_ap_buf->profile_names[1].sigma_dut_profile, HOSTAPD_RADIO2_PROFILE);
		strcpy(mtk_ap_buf->profile_names[1].sigma_tb_profile, HOSTAPD_RADIO2_PROFILE);
		DPRINT_INFO(WFA_OUT, "sigma_dut_profile 2: %s\n", mtk_ap_buf->profile_names[1].sigma_dut_profile);
		DPRINT_INFO(WFA_OUT, "sigma_tb_profile 2: %s\n", mtk_ap_buf->profile_names[1].sigma_tb_profile);
	}
	DPRINT_INFO(WFA_OUT, "%s()<====exit===\n", __func__);
}

void hostapd_backup_profile(mtk_ap_buf_t *mtk_ap_buf)
{
	FILE *file;

	if (!mtk_ap_buf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return;
	}

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);

	if (mtk_ap_buf->intf_2G.status && !mtk_ap_buf->intf_5G.status) {
		strcpy(mtk_ap_buf->intf_5G.name, mtk_ap_buf->intf_2G.name);
		mtk_ap_buf->intf_5G.profile_names = mtk_ap_buf->intf_2G.profile_names;
	}
	if (!mtk_ap_buf->intf_2G.status && mtk_ap_buf->intf_5G.status) {
		strcpy(mtk_ap_buf->intf_2G.name, mtk_ap_buf->intf_5G.name);
		mtk_ap_buf->intf_2G.profile_names = mtk_ap_buf->intf_5G.profile_names;
	}

	DPRINT_INFO(WFA_OUT, "%s() intf_2G name %s\n", __func__, mtk_ap_buf->intf_2G.name);
	DPRINT_INFO(WFA_OUT, "%s() intf_5G name %s\n", __func__, mtk_ap_buf->intf_5G.name);

	if (mtk_ap_buf->intf_2G.status) {
		file = fopen(mtk_ap_buf->intf_2G.profile_names->profile_bak, "r");
		if (file == NULL) {
			sprintf(gCmdStr, "cp %s %s", mtk_ap_buf->intf_2G.profile_names->profile,
				mtk_ap_buf->intf_2G.profile_names->profile_bak);
			system(gCmdStr);
			system("sync");
			DPRINT_INFO(WFA_OUT, "%s() First time to use daemon, backup 2G profile to %s!\n", __func__,
				    mtk_ap_buf->intf_2G.profile_names->profile_bak);
		} else {
			DPRINT_INFO(WFA_OUT, "%s() 2G profile has been backup(%s)!\n", __func__,
				    mtk_ap_buf->intf_2G.profile_names->profile_bak);
			fclose(file);
		}
	}
	if (mtk_ap_buf->intf_5G.status) {
		file = fopen(mtk_ap_buf->intf_5G.profile_names->profile_bak, "r");
		if (file == NULL) {
			sprintf(gCmdStr, "cp %s %s", mtk_ap_buf->intf_5G.profile_names->profile,
				mtk_ap_buf->intf_5G.profile_names->profile_bak);
			system(gCmdStr);
			system("sync");
			DPRINT_INFO(WFA_OUT, "%s() First time to use daemon, backup 5G profile to %s!\n", __func__,
				    mtk_ap_buf->intf_5G.profile_names->profile_bak);
		} else {
			DPRINT_INFO(WFA_OUT, "%s() 5G profile has been backup(%s)!\n", __func__,
				    mtk_ap_buf->intf_5G.profile_names->profile_bak);
			fclose(file);
		}
	}

	if (mtk_ap_buf->intf_6G.status) {
		file = fopen(mtk_ap_buf->intf_6G.profile_names->profile_bak, "r");
		if (file == NULL) {
			sprintf(gCmdStr, "cp %s %s", mtk_ap_buf->intf_6G.profile_names->profile,
				mtk_ap_buf->intf_6G.profile_names->profile_bak);
			system(gCmdStr);
			system("sync");
			DPRINT_INFO(WFA_OUT, "%s() First time to use daemon, backup 6G profile to %s!\n", __func__,
				    mtk_ap_buf->intf_6G.profile_names->profile_bak);
		} else {
			DPRINT_INFO(WFA_OUT, "%s() 6G profile has been backup(%s)!\n", __func__,
				    mtk_ap_buf->intf_6G.profile_names->profile_bak);
			fclose(file);
		}
	}
	DPRINT_INFO(WFA_OUT, "%s()<====exit===\n", __func__);
}

void hostapd_apply_sigma_profile(mtk_ap_buf_t *mtk_ap_buf, device_type dev_type)
{
	if (!mtk_ap_buf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return;
	}

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);
	sprintf(gCmdStr, "cp %s %s", mtk_ap_buf->intf_2G.profile_names->profile_bak,
		mtk_ap_buf->intf_2G.profile_names->profile);
	system(gCmdStr);
	DPRINT_INFO(WFA_OUT, "%s() %s\n", __func__, gCmdStr);
	system("sync");

	if (strcasecmp(mtk_ap_buf->intf_2G.profile_names->profile, mtk_ap_buf->intf_5G.profile_names->profile) != 0) {
		sprintf(gCmdStr, "cp %s %s", mtk_ap_buf->intf_5G.profile_names->profile_bak,
			mtk_ap_buf->intf_5G.profile_names->profile);
		system(gCmdStr);
		DPRINT_INFO(WFA_OUT, "%s() %s\n", __func__, gCmdStr);
		system("sync");
	}
	if (mtk_ap_buf->intf_6G.status) {
		sprintf(gCmdStr, "cp %s %s", mtk_ap_buf->intf_6G.profile_names->profile_bak,
			mtk_ap_buf->intf_6G.profile_names->profile);
		system(gCmdStr);
		DPRINT_INFO(WFA_OUT, "%s() %s\n", __func__, gCmdStr);
		system("sync");
	}

	DPRINT_INFO(WFA_OUT, "Apply default profile!\n");
	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
}

void hostapd_restore_profile(mtk_ap_buf_t *mtk_ap_buf)
{
	if (!mtk_ap_buf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return;
	}

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);
	sprintf(gCmdStr, "cp %s %s", mtk_ap_buf->intf_2G.profile_names->profile,
		mtk_ap_buf->intf_2G.profile_names->profile_cmt);
	system(gCmdStr);
	DPRINT_INFO(WFA_OUT, "%s() %s!\n", __func__, gCmdStr);
	system("sync");

	sprintf(gCmdStr, "cp %s %s", mtk_ap_buf->intf_2G.profile_names->profile_bak,
		mtk_ap_buf->intf_2G.profile_names->profile);
	system(gCmdStr);
	DPRINT_INFO(WFA_OUT, "%s() %s!\n", __func__, gCmdStr);
	system("sync");

	if (strcasecmp(mtk_ap_buf->intf_2G.profile_names->profile, mtk_ap_buf->intf_5G.profile_names->profile) != 0) {
		sprintf(gCmdStr, "cp %s %s", mtk_ap_buf->intf_5G.profile_names->profile,
			mtk_ap_buf->intf_5G.profile_names->profile_cmt);
		system(gCmdStr);
		DPRINT_INFO(WFA_OUT, "%s() %s!\n", __func__, gCmdStr);
		system("sync");

		sprintf(gCmdStr, "cp %s %s", mtk_ap_buf->intf_5G.profile_names->profile_bak,
			mtk_ap_buf->intf_5G.profile_names->profile);
		system(gCmdStr);
		DPRINT_INFO(WFA_OUT, "%s() %s!\n", __func__, gCmdStr);
		system("sync");
	}

	if (mtk_ap_buf->intf_6G.status) {
		sprintf(gCmdStr, "cp %s %s", mtk_ap_buf->intf_6G.profile_names->profile,
			mtk_ap_buf->intf_6G.profile_names->profile_cmt);
		system(gCmdStr);
		DPRINT_INFO(WFA_OUT, "%s() %s!\n", __func__, gCmdStr);
		system("sync");

		sprintf(gCmdStr, "cp %s %s", mtk_ap_buf->intf_6G.profile_names->profile_bak,
			mtk_ap_buf->intf_6G.profile_names->profile);
		system(gCmdStr);
		DPRINT_INFO(WFA_OUT, "%s() %s!\n", __func__, gCmdStr);
		system("sync");
	}

	DPRINT_INFO(WFA_OUT, "Restore profile!\n");
	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
}

static int hostapd_assign_profile_pointer_to_intf(mtk_ap_buf_t *mtk_ap_buf, char *ifname,
						  intf_profile_t **profile_names)
{
	int i;

	if (!mtk_ap_buf || !ifname || !profile_names) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return WFA_ERROR;
	}

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);
	for (i = 0; i < INTF_NUM; i++) {
		if (strcasecmp(ifname, mtk_ap_buf->profile_names[i].name) == 0) {
			*profile_names = &mtk_ap_buf->profile_names[i];
			DPRINT_INFO(WFA_OUT, "Assign inerface %s with profile name: %s\n", ifname,
				    (*profile_names)->profile);
			DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
			return WFA_SUCCESS;
		}
	}
	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
	return WFA_ERROR;
}

static void hostapd_ap_set_htcap(FILE *f, intf_desc_t *def_intf)
{
	int ht40plus = 0, ht40minus = 0;
	char ht_cap_str[WFA_BUFF_1K] = {0};
	char ht_cap_offset = 0;

	if (!def_intf || !f) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return;
	}

	memset(ht_cap_str, 0, sizeof(ht_cap_str));

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);

	if ((def_intf->ap_mode != AP_MODE_11g) && (def_intf->ap_mode != AP_MODE_11na) &&
	    (def_intf->ap_mode != AP_MODE_11ng) && (def_intf->ap_mode != AP_MODE_11ac) &&
	    (def_intf->ap_mode != AP_MODE_11ax)) {
		DPRINT_INFO(WFA_OUT, "mode %s, don't set ht cap\n", hostapd_apmode_2_string(def_intf->ap_mode));
		return;
	}

	if (def_intf->ch_width > CHANNEL_WIDTH_20) {
		if (def_intf->ch_offset == CHANNEL_OFFSET_BELOW) {
			ht40plus = 0;
			ht40minus = 1;
			DPRINT_INFO(WFA_OUT, "%d, ht40plus %d ht40minus %d\n", __LINE__, ht40plus, ht40minus);
		} else {
			ht40plus = 1;
			ht40minus = 0;
			DPRINT_INFO(WFA_OUT, "%d, ht40plus %d ht40minus %d\n", __LINE__, ht40plus, ht40minus);
		}

		/* TGn: "[HT40+]" or "[HT40-]" should be set if only channel from ucc */
		if (def_intf->ch_width == CHANNEL_WIDTH_40) {
			if ((def_intf->channel == 1) || (def_intf->channel == 2) || (def_intf->channel == 3) ||
			    (def_intf->channel == 4) || (def_intf->channel == 36) ||
			    (def_intf->channel == 44) | (def_intf->channel == 52) || (def_intf->channel == 60) ||
			    (def_intf->channel == 100) || (def_intf->channel == 108) || (def_intf->channel == 116) ||
			    (def_intf->channel == 124) || (def_intf->channel == 132) || (def_intf->channel == 140) ||
			    (def_intf->channel == 149) || (def_intf->channel == 157)) {
				ht40plus = 1;
				ht40minus = 0;
				DPRINT_INFO(WFA_OUT, "%d, ht40plus %d ht40minus %d\n", __LINE__, ht40plus, ht40minus);
			} else {
				ht40plus = 0;
				ht40minus = 1;
				DPRINT_INFO(WFA_OUT, "%d, ht40plus %d ht40minus %d\n", __LINE__, ht40plus, ht40minus);
			}
		}
	} else {
		ht40plus = 0;
		ht40minus = 0;
		DPRINT_INFO(WFA_OUT, "%d, ht40plus %d ht40minus %d\n", __LINE__, ht40plus, ht40minus);
	}

	if (!ht40plus && !ht40minus) {
		if (def_intf->sgi20 != WFA_ENABLED) {
			DPRINT_INFO(WFA_OUT, "OH no, def_intf->sgi20 not enabled, no ht40 plus, no ht40 minus\n");
			goto done;
		}
	}
	DPRINT_INFO(WFA_OUT, "ht40plus %d, ht40minus %d, sgi20 %d\n", ht40plus, ht40minus, def_intf->sgi20);

	if (ht40plus)
		ht_cap_offset += snprintf(ht_cap_str + ht_cap_offset, sizeof(ht_cap_str) - ht_cap_offset, "[HT40+]");
	if (ht40minus)
		ht_cap_offset += snprintf(ht_cap_str + ht_cap_offset, sizeof(ht_cap_str) - ht_cap_offset, "[HT40-]");
	if (!def_intf->bcc_mode)
		ht_cap_offset += snprintf(ht_cap_str + ht_cap_offset, sizeof(ht_cap_str) - ht_cap_offset, "[LDPC]");
	if (def_intf->sgi20)
		ht_cap_offset +=
		    snprintf(ht_cap_str + ht_cap_offset, sizeof(ht_cap_str) - ht_cap_offset, "[SHORT-GI-20]");
	if (def_intf->sgi40 && (def_intf->ch_width >= CHANNEL_WIDTH_40))
		ht_cap_offset +=
		    snprintf(ht_cap_str + ht_cap_offset, sizeof(ht_cap_str) - ht_cap_offset, "[SHORT-GI-40]");
	if (def_intf->stbc)
		ht_cap_offset +=
		    snprintf(ht_cap_str + ht_cap_offset, sizeof(ht_cap_str) - ht_cap_offset, "[TX-STBC][RX-STBC1]");
	if (def_intf->max_amsdu == 7935)
		ht_cap_offset +=
		    snprintf(ht_cap_str + ht_cap_offset, sizeof(ht_cap_str) - ht_cap_offset, "[MAX-AMSDU-7935]");

done:

	fprintf(f, "ht_capab=%s\n", ht_cap_str);
	DPRINT_INFO(WFA_OUT, "ht_capab=%s\n", ht_cap_str);

	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
}

static void hostapd_ap_set_vhtcap(FILE *f, intf_desc_t *def_intf)
{
	char vht_cap_str[WFA_BUFF_1K] = {0};
	char vht_cap_offset = 0;
	int ucVhtChannelWidth = 1;
	int ucVhtChannelFrequencyS1 = 0;

	if (!def_intf || !f) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return;
	}

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);

	memset(vht_cap_str, 0, sizeof(vht_cap_str));

	if ((def_intf->ap_mode != AP_MODE_11g) && (def_intf->ap_mode != AP_MODE_11na) &&
	    (def_intf->ap_mode != AP_MODE_11ac) && (def_intf->ap_mode != AP_MODE_11ax)) {
		DPRINT_INFO(WFA_OUT, "mode %s, don't set vht cap\n", hostapd_apmode_2_string(def_intf->ap_mode));
		return;
	}

	if (def_intf->channel <= 14) {
		DPRINT_INFO(WFA_OUT, "channel %d, don't set vht cap\n", def_intf->channel);
		return;
	}

	if (def_intf->sgi80 && (def_intf->ch_width >= CHANNEL_WIDTH_80))
		vht_cap_offset +=
		    snprintf(vht_cap_str + vht_cap_offset, sizeof(vht_cap_str) - vht_cap_offset, "[SHORT-GI-80]");

	if (def_intf->sgi160 && (def_intf->ch_width >= CHANNEL_WIDTH_160))
		vht_cap_offset +=
		    snprintf(vht_cap_str + vht_cap_offset, sizeof(vht_cap_str) - vht_cap_offset, "[SHORT-GI-160]");

	if (def_intf->stbc)
		vht_cap_offset += snprintf(vht_cap_str + vht_cap_offset, sizeof(vht_cap_str) - vht_cap_offset,
					   "[TX-STBC-2BY1][RX-STBC-1]");

	if (def_intf->txbf)
		vht_cap_offset += snprintf(vht_cap_str + vht_cap_offset, sizeof(vht_cap_str) - vht_cap_offset,
					   "[SU-BEAMFORMER][SU-BEAMFORMEE][MU-BEAMFORMER][MU-BEAMFORMEE]");

	else if (def_intf->mimo) {
		if (def_intf->DL)
			vht_cap_offset += snprintf(vht_cap_str + vht_cap_offset, sizeof(vht_cap_str) - vht_cap_offset,
						   "[SU-BEAMFORMER][MU-BEAMFORMER]");
		else
			DPRINT_INFO(WFA_OUT, "UL MIMO not support yet\n");
	}

	if (def_intf->max_mpdu == 11454)
		vht_cap_offset +=
		    snprintf(vht_cap_str + vht_cap_offset, sizeof(vht_cap_str) - vht_cap_offset, "[MAX-MPDU-11454]");

	else if (def_intf->max_mpdu == 7991)
		vht_cap_offset +=
		    snprintf(vht_cap_str + vht_cap_offset, sizeof(vht_cap_str) - vht_cap_offset, "[MAX-MPDU-7991]");

	if (def_intf->max_ampdu == 7)
		vht_cap_offset += snprintf(vht_cap_str + vht_cap_offset, sizeof(vht_cap_str) - vht_cap_offset,
					   "[MAX-A-MPDU-LEN-EXP7]");

	if (!def_intf->bcc_mode)
		vht_cap_offset +=
		    snprintf(vht_cap_str + vht_cap_offset, sizeof(vht_cap_str) - vht_cap_offset, "[RXLDPC]");

	if (def_intf->ch_width == CHANNEL_WIDTH_80_80)
		vht_cap_offset +=
		    snprintf(vht_cap_str + vht_cap_offset, sizeof(vht_cap_str) - vht_cap_offset, "[VHT160-80PLUS80]");

	else if (def_intf->ch_width == CHANNEL_WIDTH_160)
		vht_cap_offset +=
		    snprintf(vht_cap_str + vht_cap_offset, sizeof(vht_cap_str) - vht_cap_offset, "[VHT160]");

	if (def_intf->mimo) {
		if (def_intf->DL)
			vht_cap_offset += snprintf(vht_cap_str + vht_cap_offset, sizeof(vht_cap_str) - vht_cap_offset,
						   "[SU-BEAMFORMER][MU-BEAMFORMER]");
		else
			DPRINT_INFO(WFA_OUT, "UL MIMO not support yet\n");
	}

	vht_cap_offset += snprintf(vht_cap_str + vht_cap_offset, sizeof(vht_cap_str) - vht_cap_offset,
				   "[RX-ANTENNA-PATTERN][TX-ANTENNA-PATTERN]");
	fprintf(f, "vht_capab=%s\n", vht_cap_str);
	DPRINT_INFO(WFA_OUT, "vht_capab=%s\n", vht_cap_str);

	DPRINT_INFO(WFA_OUT, "ch_width=%d, channel %d\n", def_intf->ch_width, def_intf->channel);
	if (def_intf->ch_width == CHANNEL_WIDTH_20) {
		ucVhtChannelWidth = 0;
		ucVhtChannelFrequencyS1 = 0;
		DPRINT_INFO(WFA_OUT, "ucVhtChannelWidth=%d\n", ucVhtChannelWidth);
		DPRINT_INFO(WFA_OUT, "ucVhtChannelFrequencyS1=%d\n", ucVhtChannelFrequencyS1);
	} else if (def_intf->ch_width == CHANNEL_WIDTH_40) {
		ucVhtChannelWidth = 0;
		ucVhtChannelFrequencyS1 = 0;
		DPRINT_INFO(WFA_OUT, "ucVhtChannelWidth=%d\n", ucVhtChannelWidth);
		DPRINT_INFO(WFA_OUT, "ucVhtChannelFrequencyS1=%d\n", ucVhtChannelFrequencyS1);
	} else if (def_intf->ch_width == CHANNEL_WIDTH_160) {
		ucVhtChannelWidth = 2;
		if (def_intf->channel >= 36 && def_intf->channel <= 64)
			ucVhtChannelFrequencyS1 = 50;
		else if (def_intf->channel >= 100 && def_intf->channel <= 128)
			ucVhtChannelFrequencyS1 = 114;

		DPRINT_INFO(WFA_OUT, "ucVhtChannelWidth=%d\n", ucVhtChannelWidth);
		DPRINT_INFO(WFA_OUT, "ucVhtChannelFrequencyS1=%d\n", ucVhtChannelFrequencyS1);
	} else {
		ucVhtChannelWidth = 1;
		if (def_intf->channel >= 36 && def_intf->channel <= 48)
			ucVhtChannelFrequencyS1 = 42;
		else if (def_intf->channel >= 52 && def_intf->channel <= 64)
			ucVhtChannelFrequencyS1 = 58;
		else if (def_intf->channel >= 100 && def_intf->channel <= 112)
			ucVhtChannelFrequencyS1 = 106;
		else if (def_intf->channel >= 116 && def_intf->channel <= 128)
			ucVhtChannelFrequencyS1 = 122;
		else if (def_intf->channel >= 132 && def_intf->channel <= 144)
			ucVhtChannelFrequencyS1 = 138;
		else if (def_intf->channel >= 149 && def_intf->channel <= 161)
			ucVhtChannelFrequencyS1 = 155;
		DPRINT_INFO(WFA_OUT, "ucVhtChannelWidth=%d\n", ucVhtChannelWidth);
		DPRINT_INFO(WFA_OUT, "ucVhtChannelFrequencyS1=%d\n", ucVhtChannelFrequencyS1);
	}

	fprintf(f, "vht_oper_centr_freq_seg0_idx=%d\n", ucVhtChannelFrequencyS1);
	fprintf(f, "vht_oper_chwidth=%d\n", ucVhtChannelWidth);

	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
}

static void hostapd_ap_set_he(FILE *f, intf_desc_t *def_intf)
{

	if (!f || !def_intf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return;
	}

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);

	if (def_intf->ap_mode != AP_MODE_11ax && def_intf->program != PROGRAME_TYPE_HE) {

		DPRINT_INFO(WFA_OUT, "mode %s, don't need to set he\n", hostapd_apmode_2_string(def_intf->ap_mode));
		return;
	}

	DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

	if (def_intf->encpType == ENCP_TYPE_CCMP || def_intf->encpType == ENCP_TYPE_GCMP_128 ||
	    def_intf->keyMgmtType == KEY_MGNT_TYPE_OPEN) {
		fprintf(f, "ieee80211n=1\n");
		fprintf(f, "ieee80211ax=1\n");
		DPRINT_INFO(WFA_OUT, "ieee80211n=1\n");
		DPRINT_INFO(WFA_OUT, "ieee80211ax=1\n");
	}

	DPRINT_INFO(WFA_OUT, "channel=%d\n", def_intf->channel);

	if (def_intf->channel <= 14) {
		/* 2.4G */
		fprintf(f, "hw_mode=g\n");
		DPRINT_INFO(WFA_OUT, "hw_mode=g\n");
	} else {
		int channel = def_intf->channel;
		int ucHeChannelWidth = 1;
		int ucHeChannelFrequencyS1 = 0;

		/* 5G */
		fprintf(f, "hw_mode=a\n");
		DPRINT_INFO(WFA_OUT, "hw_mode=a\n");
		if (def_intf->encpType == ENCP_TYPE_CCMP || def_intf->encpType == ENCP_TYPE_GCMP_128 ||
		    def_intf->keyMgmtType == KEY_MGNT_TYPE_OPEN) {
			fprintf(f, "ieee80211ac=1\n");
			DPRINT_INFO(WFA_OUT, "ieee80211ac=1\n");
		}

		DPRINT_INFO(WFA_OUT, "ch_width=%d, channel %d\n", def_intf->ch_width, channel);
		if (def_intf->ch_width == CHANNEL_WIDTH_20) {
			ucHeChannelWidth = 0;
			ucHeChannelFrequencyS1 = 0;
			DPRINT_INFO(WFA_OUT, "ucVhtChannelWidth=%d\n", ucHeChannelWidth);
			DPRINT_INFO(WFA_OUT, "ucVhtChannelFrequencyS1=%d\n", ucHeChannelFrequencyS1);
		} else if (def_intf->ch_width == CHANNEL_WIDTH_40) {
			ucHeChannelWidth = 0;
			ucHeChannelFrequencyS1 = 0;
			DPRINT_INFO(WFA_OUT, "ucVhtChannelWidth=%d\n", ucHeChannelWidth);
			DPRINT_INFO(WFA_OUT, "ucVhtChannelFrequencyS1=%d\n", ucHeChannelFrequencyS1);
		} else if (def_intf->ch_width == CHANNEL_WIDTH_160) {
			ucHeChannelWidth = 2;
			if (channel >= 36 && channel <= 64)
				ucHeChannelFrequencyS1 = 50;
			else if (channel >= 100 && channel <= 128)
				ucHeChannelFrequencyS1 = 114;

			DPRINT_INFO(WFA_OUT, "ucVhtChannelWidth=%d\n", ucHeChannelWidth);
			DPRINT_INFO(WFA_OUT, "ucVhtChannelFrequencyS1=%d\n", ucHeChannelFrequencyS1);
		} else {
			ucHeChannelWidth = 1;
			if (channel >= 36 && channel <= 48)
				ucHeChannelFrequencyS1 = 42;
			else if (channel >= 52 && channel <= 64)
				ucHeChannelFrequencyS1 = 58;
			else if (channel >= 100 && channel <= 112)
				ucHeChannelFrequencyS1 = 106;
			else if (channel >= 116 && channel <= 128)
				ucHeChannelFrequencyS1 = 122;
			else if (channel >= 132 && channel <= 144)
				ucHeChannelFrequencyS1 = 138;
			else if (channel >= 149 && channel <= 161)
				ucHeChannelFrequencyS1 = 155;
			DPRINT_INFO(WFA_OUT, "ucVhtChannelWidth=%d\n", ucHeChannelWidth);
			DPRINT_INFO(WFA_OUT, "ucVhtChannelFrequencyS1=%d\n", ucHeChannelFrequencyS1);
		}

		fprintf(f, "he_oper_centr_freq_seg0_idx=%d\n", ucHeChannelFrequencyS1);
		fprintf(f, "he_oper_chwidth=%d\n", ucHeChannelWidth);
	}

	if (!def_intf->txbf) {
		fprintf(f, "he_su_beamformer=0\n");
		fprintf(f, "he_su_beamformee=0\n");
	} else if (def_intf->mimo) {
		if (def_intf->DL) {
			fprintf(f, "he_su_beamformer=1\n");
			fprintf(f, "he_mu_beamformer=1\n");
		}
		/* MIMO UL not support yet */
	} else if (def_intf->txbf) {
		fprintf(f, "he_su_beamformer=1\n");
		fprintf(f, "he_mu_beamformer=0\n");
	} else {
		fprintf(f, "he_su_beamformer=0\n");
		fprintf(f, "he_mu_beamformer=0\n");
	}

	fprintf(f, "he_su_beamformee=0\n");
	fprintf(f, "he_twt_required=0\n");
	fprintf(f, "he_bss_color=%d\n", (int)(def_intf->bss_color));

	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
}

static void hostapd_init_intf_default_param(mtk_ap_buf_t *mtk_ap_buf, wifi_mode mode)
{
	intf_desc_t *intf = NULL;

	if (!mtk_ap_buf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return;
	}

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);
	DPRINT_INFO(WFA_OUT, "mode %s\n", hostapd_wifi_mode_2_string(mode));
	mtk_ap_buf->WappEnable = 0;

	if (mode == WIFI_2G) {
		intf = &mtk_ap_buf->intf_2G;
		intf->ch_width = CHANNEL_WIDTH_40;
		intf->ch_offset = CHANNEL_OFFSET_ABOVE;
	} else if (mode == WIFI_5G) {
		intf = &mtk_ap_buf->intf_5G;
		intf->ch_width = CHANNEL_WIDTH_80;
		intf->sgi80 = WFA_ENABLED;
	} else if (mode == WIFI_6G) {
		intf = &mtk_ap_buf->intf_6G;
		intf->ch_width = CHANNEL_WIDTH_80;
		intf->sgi80 = WFA_ENABLED;
	} else {
		DPRINT_INFO(WFA_OUT, "Error, No default interface is init, check the reason!!!\n");
		return;
	}

	intf->mode = mode;
	intf->sgi20 = WFA_ENABLED;
	intf->sgi40 = WFA_ENABLED;
	intf->mbss_en = 0;
	intf->bss_idx = 0;
	intf->bss_num = 0;
	intf->WLAN_TAG_bss_num = 0;
	memset(intf->passphrase, 0, sizeof(intf->passphrase));
	memset(intf->auth_server_addr, 0, sizeof(intf->auth_server_addr));
	memset(intf->auth_server_port, 0, sizeof(intf->auth_server_port));
	memset(intf->auth_server_shared_secret, 0, sizeof(intf->auth_server_shared_secret));
	intf->bcc_mode = 0;
	intf->stbc = 1;
	intf->txbf = 1;
	intf->max_amsdu = 7935;
	/* vht_max_mpdu_len: [MAX-MPDU-7991] [MAX-MPDU-11454] */
	intf->max_mpdu = 7991;
	/* Maximum A-MPDU Length Exponent:
	 ** [MAX-A-MPDU-LEN-EXP0]..[MAX-A-MPDU-LEN-EXP7]
	 */
	intf->max_ampdu = 7;
	intf->ap_isolate = 1;
	intf->bss_load_update_period = 60;
	intf->chan_util_avg_period = 600;
	intf->disassoc_low_ack = 1;
	intf->skip_inactivity_poll = 0;
	intf->preamble = 1;
	intf->wme = 1;
	intf->ignore_broadcast_ssid = 0;
	intf->uapsd_advertisement_enabled = 1;
	intf->utf8_ssid = 1;
	intf->multi_ap = 0;
	intf->auth_algs = 1;
	intf->mimo = 0;
	intf->UL_MUMIMO = 0;
	intf->DL = 0;
	intf->bss_color = 1;
	intf->sha256ad = 0;
	intf->BTMReq_Term_Bit = 0;
	intf->BSS_Term_Duration = 0;
	intf->BTMReq_DisAssoc_Imnt = 0;

	intf->keyMgmtType = KEY_MGNT_TYPE_OPEN;
	/* default intf is phy0 */

	snprintf(intf->ctrl_inf, sizeof(intf->ctrl_inf) - 1, HOSTAPD_BIN);
	DPRINT_INFO(WFA_OUT, "intf->name:%s, intf->ctrl_inf:%s!\n", intf->name, intf->ctrl_inf);

	memset(intf->ssid[0], 0, sizeof(intf->ssid[0]));
	memset(intf->ssid[1], 0, sizeof(intf->ssid[1]));
	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
}

static void hostapd_set_default_intf(mtk_ap_buf_t *mtk_ap_buf, wifi_mode mode)
{
	if (!mtk_ap_buf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return;
	}

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);

	if (mode == WIFI_2G) {
		mtk_ap_buf->def_mode = WIFI_2G;
		mtk_ap_buf->def_intf = &mtk_ap_buf->intf_2G;
		mtk_ap_buf->def_intf->mode = WIFI_2G;
		mtk_ap_buf->def_intf->status = 1;
		DPRINT_INFO(WFA_OUT, "Set default interface to 2G!\n");
	} else if (mode == WIFI_5G) {
		mtk_ap_buf->def_mode = WIFI_5G;
		mtk_ap_buf->def_intf = &mtk_ap_buf->intf_5G;
		mtk_ap_buf->def_intf->mode = WIFI_5G;
		mtk_ap_buf->def_intf->status = 1;
		DPRINT_INFO(WFA_OUT, "Set default interface to 5G!\n");
	} else if (mode == WIFI_6G) {
		mtk_ap_buf->def_mode = WIFI_6G;
		mtk_ap_buf->def_intf = &mtk_ap_buf->intf_6G;
		mtk_ap_buf->def_intf->mode = WIFI_6G;
		mtk_ap_buf->def_intf->status = 1;
		DPRINT_INFO(WFA_OUT, "Set default interface to 6G!\n");
	}

	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
}

static int hostapd_ap_init(mtk_ap_buf_t *mtk_ap_buf)
{
	if (!mtk_ap_buf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return WFA_FAILURE;
	}

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);

	/* parameters init for both 24G and 5G */
	hostapd_init_intf_default_param(mtk_ap_buf, WIFI_2G);
	hostapd_init_intf_default_param(mtk_ap_buf, WIFI_5G);

	/* but only one def_intf was set while init stage */
	if (mtk_ap_buf->intf_2G.status) {
		hostapd_set_default_intf(mtk_ap_buf, WIFI_2G);
		mtk_ap_buf->intf_2G.bss_num = 1;
		if (mtk_ap_buf->intf_2G.bss_num > 1)
			mtk_ap_buf->intf_2G.mbss_en = 1;
	}

	if (mtk_ap_buf->intf_5G.status) {
		hostapd_set_default_intf(mtk_ap_buf, WIFI_5G);
		mtk_ap_buf->intf_5G.bss_num = 1;
		if (mtk_ap_buf->intf_5G.bss_num > 1)
			mtk_ap_buf->intf_5G.mbss_en = 1;
	}

	if (mtk_ap_buf->intf_6G.status) {
		mtk_ap_buf->intf_6G.bss_num = 1;
		if (mtk_ap_buf->intf_6G.bss_num > 1)
			mtk_ap_buf->intf_6G.mbss_en = 1;
	}

	if (!(mtk_ap_buf->intf_2G.status || mtk_ap_buf->intf_5G.status)) {
		DPRINT_INFO(WFA_OUT, "%s No valid interface!!!\n", __func__);
		DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
		return WFA_FAILURE;
	}

	/* check if need set OWN_IP_ADDR and BssidNum */

	mtk_ap_buf->WLAN_TAG = 0;
	mtk_ap_buf->post_cmd_idx = 0;
	strncpy(mtk_ap_buf->Reg_Domain, "", sizeof(mtk_ap_buf->Reg_Domain));
	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
	return WFA_SUCCESS;
}

static void hostapd_write_conf(intf_desc_t *def_intf)
{
	FILE *f = NULL;
	int bss_idx = 0, bss_end_idx = 0;
	int ret_len = 0;
	char shellResult[WFA_BUFF_64] = {0};
	char shellResult_interface[WFA_BUFF_64] = {0};
	char shellResult_bss[WFA_BUFF_64] = {0};
	/* hostapd_qos_t *ap_qos = &(def_intf->ap_qos); */
	hostapd_qos_t *sta_qos = &(def_intf->sta_qos);
	char *keymgnt = NULL;

	DPRINT_INFO(WFA_OUT, "*********************************\n");
	DPRINT_INFO(WFA_OUT, "%s() enter\n", __func__);

	f = fopen(def_intf->profile_names->profile, "w");
	if (f == NULL) {
		DPRINT_ERR(WFA_ERR, "file open fail\n");
		return;
	}
	DPRINT_INFO(WFA_OUT, "open %s()\n", def_intf->profile_names->profile);

	fprintf(f, "driver=nl80211\n");
	fprintf(f, "logger_syslog=127\n");
	fprintf(f, "logger_syslog_level=2\n");
	fprintf(f, "logger_stdout=127\n");
	fprintf(f, "logger_stdout_level=2\n");
	fprintf(f, "noscan=1\n");
	switch (def_intf->ap_mode) {
	case AP_MODE_11b:
		fprintf(f, "hw_mode=b\n");
		DPRINT_INFO(WFA_OUT, "hw_mode=b\n");
		break;
	case AP_MODE_11g:
	case AP_MODE_11ng:
		fprintf(f, "hw_mode=g\n");
		DPRINT_INFO(WFA_OUT, "hw_mode=g\n");
		if (def_intf->ap_mode == AP_MODE_11ng) {
			if (def_intf->encpType == ENCP_TYPE_CCMP || def_intf->encpType == ENCP_TYPE_GCMP_128 ||
			    def_intf->keyMgmtType == KEY_MGNT_TYPE_OPEN) {
				fprintf(f, "ieee80211n=1\n");
				DPRINT_INFO(WFA_OUT, "ieee80211n=1\n");
			}
		}
		break;
	case AP_MODE_11a:
	case AP_MODE_11na:
	case AP_MODE_11ac:
		fprintf(f, "hw_mode=a\n");
		DPRINT_INFO(WFA_OUT, "hw_mode=a\n");
		if (def_intf->ap_mode == AP_MODE_11na || def_intf->ap_mode == AP_MODE_11ac) {
			if (def_intf->encpType == ENCP_TYPE_CCMP || def_intf->encpType == ENCP_TYPE_GCMP_128 ||
			    def_intf->encpType == ENCP_TYPE_CCMP_TKIP || def_intf->keyMgmtType == KEY_MGNT_TYPE_OPEN ||
			    def_intf->keyMgmtType == KEY_MGNT_TYPE_WPA2_PSK_MIXED) {
				fprintf(f, "ieee80211n=1\n");
				DPRINT_INFO(WFA_OUT, "ieee80211n=1\n");
			}
		}
		if (def_intf->ap_mode == AP_MODE_11ac) {
			if (def_intf->encpType == ENCP_TYPE_CCMP || def_intf->encpType == ENCP_TYPE_GCMP_128 ||
			    def_intf->encpType == ENCP_TYPE_CCMP_TKIP || def_intf->keyMgmtType == KEY_MGNT_TYPE_OPEN ||
			    def_intf->keyMgmtType == KEY_MGNT_TYPE_WPA2_PSK_MIXED) {
				fprintf(f, "ieee80211ac=1\n");
				DPRINT_INFO(WFA_OUT, "ieee80211ac=1\n");
			}
			if (def_intf->country_code[0])
				fprintf(f, "country_code=%s\n", def_intf->country_code);
			DPRINT_INFO(WFA_OUT, "country_code=%s\n", def_intf->country_code);
		}
		break;
	case AP_MODE_11ax:
		hostapd_ap_set_he(f, def_intf);
		break;
	default:
		DPRINT_WARNING(WFA_WNG, "unsupport mode: %d\n", def_intf->ap_mode);
		break;
	}

	if (def_intf->channel) {
		fprintf(f, "channel=%d\n", def_intf->channel);
		DPRINT_INFO(WFA_OUT, "channel=%d\n", def_intf->channel);

		if ((def_intf->channel <= 14) && (def_intf->ch_width == CHANNEL_WIDTH_40)) {
			fprintf(f, "ht_coex=1\n");
			fprintf(f, "obss_interval=300\n");
			DPRINT_INFO(WFA_OUT, "ht_coex=1\n");
			DPRINT_INFO(WFA_OUT, "obss_interval=300\n");
		}
	}
	if (def_intf->bcnint)
		fprintf(f, "beacon_int=%d\n", def_intf->bcnint);
	DPRINT_INFO(WFA_OUT, "beacon_int=%d\n", def_intf->bcnint);

	if (def_intf->dtim_period)
		fprintf(f, "dtim_period=%d\n", def_intf->dtim_period);
	DPRINT_INFO(WFA_OUT, "dtim_period=%d\n", def_intf->dtim_period);

	memset(gCmdStr, 0, sizeof(gCmdStr));
	if (def_intf->mode == WIFI_2G) {
		snprintf(gCmdStr, sizeof(gCmdStr), "md5sum %s | cut -d\" \" -f1 > %s", def_intf->profile_names->profile,
			 HOSTAPD_TEMP_OUTPUT);
	} else if (def_intf->mode == WIFI_5G) {
		snprintf(gCmdStr, sizeof(gCmdStr), "md5sum %s | cut -d\" \" -f1 > %s", def_intf->profile_names->profile,
			 HOSTAPD_TEMP_OUTPUT);
	}
	if (hostapd_get_cmd_output(shellResult, sizeof(shellResult), gCmdStr))
		DPRINT_INFO(WFA_OUT, "no md5 found\n");

	fprintf(f, "radio_config_id=%s\n", shellResult);
	DPRINT_INFO(WFA_OUT, "radio_config_id %s\n", shellResult);

	hostapd_ap_set_htcap(f, def_intf);

	hostapd_ap_set_vhtcap(f, def_intf);

	if (def_intf->bss_idx)
		bss_end_idx = 2;
	else
		bss_end_idx = 1;

	if (strcasecmp(def_intf->keymgnt, "OWE") == 0) {
		for (bss_idx = 0; bss_idx < bss_end_idx; bss_idx++) {
			if (l1_valid) {
				memset(gCmdStr, 0, sizeof(gCmdStr));
				snprintf(gCmdStr, sizeof(gCmdStr), "ifconfig %s | grep %s | awk '{print $5}' > %s",
					 def_intf->name, def_intf->name, HOSTAPD_TEMP_OUTPUT);

				if (hostapd_get_cmd_output(shellResult, sizeof(shellResult), gCmdStr))
					DPRINT_INFO(WFA_OUT, "no bssid found\n");
				if (bss_idx) {
					unsigned int mac_hex[6] = {0};

					/* last bytes plus one for multi bss */
					ret_len =
					    sscanf(shellResult, "%02x:%02x:%02x:%02x:%02x:%02x", &mac_hex[0],
						   &mac_hex[1], &mac_hex[2], &mac_hex[3], &mac_hex[4], &mac_hex[5]);
					if (ret_len != 6)
						DPRINT_INFO(WFA_OUT, "sscanf of BSSID not match\n");
					mac_hex[5] += 1;
					snprintf(shellResult, sizeof(shellResult), "%02x:%02x:%02x:%02x:%02x:%02x",
						 mac_hex[0], mac_hex[1], mac_hex[2], mac_hex[3], mac_hex[4],
						 mac_hex[5]);

					strncpy(shellResult_bss, shellResult, sizeof(shellResult));
					DPRINT_INFO(WFA_OUT, "bss_bssid=%s\n", shellResult);
				} else {
					strncpy(shellResult_interface, shellResult, sizeof(shellResult));
					DPRINT_INFO(WFA_OUT, "interface_bssid=%s\n", shellResult);
				}
			}
		}
	}

	for (bss_idx = 0; bss_idx < bss_end_idx; bss_idx++) {
		if (!l1_valid) {
			char mbss_intf_name[WFA_NAME_LEN];
			char bss_idx_str[2];
			int offset = 0;

			if (bss_idx) {
				memset(mbss_intf_name, 0, sizeof(mbss_intf_name));
				snprintf(bss_idx_str, 2, "%d", def_intf->bss_idx);
				offset += snprintf(mbss_intf_name + offset, sizeof(mbss_intf_name) - offset - 1,
						   def_intf->name);
				offset += snprintf(mbss_intf_name + offset - 1, sizeof(mbss_intf_name) - offset - 1,
						   bss_idx_str);
				DPRINT_INFO(WFA_OUT, "mbss_intf_name=%s\n", mbss_intf_name);
				/* interface name for mbss: ra1/rai1??? */
				fprintf(f, "bss=%s\n", mbss_intf_name);
				DPRINT_INFO(WFA_OUT, "bss=%s\n", mbss_intf_name);
			} else {
				fprintf(f, "interface=%s\n", def_intf->name);
				DPRINT_INFO(WFA_OUT, "interface=%s\n", def_intf->name);
			}
		} else {
			if (bss_idx) {
				/* interface name for mbss: bss=wlan<ra_idx>-<bss_idx> */
				fprintf(f, "bss=wlan%d-%d\n", def_intf->mode, def_intf->bss_idx);
				DPRINT_INFO(WFA_OUT, "bss=wlan%d-%d\n", def_intf->mode, def_intf->bss_idx);
			} else {
				/* first interface name: interface=wlan<ra_idx> */
				fprintf(f, "interface=wlan%d\n", def_intf->mode);
				DPRINT_INFO(WFA_OUT, "interface=wlan%d\n", def_intf->mode);
			}
		}

		fprintf(f, "ctrl_interface=%s\n", def_intf->ctrl_inf);
		DPRINT_INFO(WFA_OUT, "ctrl_interface=%s\n", def_intf->ctrl_inf);

		if (bss_end_idx == 1) {
			fprintf(f, "ssid=%s\n", def_intf->ssid[0]);
			DPRINT_INFO(WFA_OUT, "ssid=%s\n", def_intf->ssid[0]);
		} else {
			if (bss_idx) {
				if (strlen(def_intf->ssid[1])) {
					fprintf(f, "ssid=%s\n", def_intf->ssid[1]);
					DPRINT_INFO(WFA_OUT, "multi ssid=%s\n", def_intf->ssid[1]);
				} else {
					fprintf(f, "ssid=sigma-unconfig\n");
					DPRINT_INFO(WFA_OUT, "multi ssid=sigma-unconfig\n");
				}
			} else {
				if (strlen(def_intf->ssid[0])) {
					fprintf(f, "ssid=%s\n", def_intf->ssid[0]);
					DPRINT_INFO(WFA_OUT, "main ssid=%s\n", def_intf->ssid[0]);
				} else {
					fprintf(f, "ssid=sigma-unconfig\n");
					DPRINT_INFO(WFA_OUT, "main ssid=sigma-unconfig\n");
				}
			}
		}

		/* set the same channel for main bss and bss[bss_idx] */

		if (def_intf->rts)
			fprintf(f, "rts_threshold=%d\n", def_intf->rts);

		if (def_intf->frgmnt)
			fprintf(f, "fragm_threshold=%d\n", def_intf->frgmnt);

		if (def_intf->wme)
			fprintf(f, "wmm_enabled=1\n");

		if (def_intf->wmmps)
			fprintf(f, "uapsd_advertisement_enabled=1\n");

		if (def_intf->p2p_mgmt)
			fprintf(f, "manage_p2p=1\n");

		switch (def_intf->keyMgmtType) {
		case KEY_MGNT_TYPE_OPEN:
			fprintf(f, "wpa=0\n");
			if (def_intf->wepkey[0]) {
				fprintf(f, "wep_key0=%s\n", def_intf->wepkey);
				fprintf(f, "wep_default_key=0\n");
			}
			break;
		case KEY_MGNT_TYPE_WPA_PSK:
		case KEY_MGNT_TYPE_WPA2_PSK:
		case KEY_MGNT_TYPE_WPA2_PSK_MIXED:
			/* config wpa */
			if (def_intf->keyMgmtType == KEY_MGNT_TYPE_WPA2_PSK) {
				fprintf(f, "rsn_pairwise=CCMP\n");
				fprintf(f, "wpa=2\n");
				DPRINT_INFO(WFA_OUT, "rsn_pairwise=CCMP\n");
				DPRINT_INFO(WFA_OUT, "wpa=2\n");
			} else if (def_intf->keyMgmtType == KEY_MGNT_TYPE_WPA2_PSK_MIXED) {
				fprintf(f, "wpa=3\n");
				DPRINT_INFO(WFA_OUT, "wpa=3\n");
			} else {
				fprintf(f, "wpa=1\n");
				DPRINT_INFO(WFA_OUT, "wpa=1\n");
			}

			switch (def_intf->encpType) {
			case ENCP_TYPE_NONE:
				/* do nothing */
				DPRINT_INFO(WFA_OUT, "do nothing, ENCP_TYPE_NONE\n");
				break;
			case ENCP_TYPE_TKIP:
				fprintf(f, "wpa_pairwise=TKIP\n");
				DPRINT_INFO(WFA_OUT, "wpa_pairwise=TKIP\n");
				break;
			case ENCP_TYPE_CCMP:
				fprintf(f, "wpa_pairwise=CCMP\n");
				DPRINT_INFO(WFA_OUT, "wpa_pairwise=CCMP\n");
				break;
			case ENCP_TYPE_GCMP_128:
				fprintf(f, "wpa_pairwise=GCMP\n");
				DPRINT_INFO(WFA_OUT, "wpa_pairwise=GCMP\n");
				break;
			case ENCP_TYPE_CCMP_TKIP:
				fprintf(f, "wpa_pairwise=TKIP CCMP\n");
				DPRINT_INFO(WFA_OUT, "wpa_pairwise=TKIP CCMP\n");
				break;
			default:
				DPRINT_INFO(WFA_OUT, "unknown encpType: %d", def_intf->encpType);
				break;
			}

			if (def_intf->passphrase[0]) {
				fprintf(f, "wpa_passphrase=%s\n", def_intf->passphrase);
				DPRINT_INFO(WFA_OUT, "wpa_passphrase=%s\n", def_intf->passphrase);
			}

			switch (def_intf->pmf) {
			case WFA_DISABLED:
			case WFA_OPTIONAL:
				if (def_intf->sha256ad) {
					fprintf(f, "wpa_key_mgmt=WPA-PSK WPA-PSK-SHA256\n");
					DPRINT_INFO(WFA_OUT, "wpa_key_mgmt=WPA-PSK WPA-PSK-SHA256\n");
				} else {
					fprintf(f, "wpa_key_mgmt=WPA-PSK\n");
					DPRINT_INFO(WFA_OUT, "wpa_key_mgmt=WPA-PSK\n");
				}
				break;
			case WFA_REQUIRED:
				fprintf(f, "wpa_key_mgmt=WPA-PSK-SHA256\n");
				DPRINT_INFO(WFA_OUT, "wpa_key_mgmt=WPA-PSK-SHA256\n");
				break;
			default:
				break;
			}
			break;

		case KEY_MGNT_TYPE_WPA_EAP:
		case KEY_MGNT_TYPE_WPA2_EAP:
		case KEY_MGNT_TYPE_WPA2_EAP_MIXED:
		case KEY_MGNT_TYPE_SUITEB:
			/* config wpa */
			if (def_intf->keyMgmtType == KEY_MGNT_TYPE_WPA2_EAP) {
				fprintf(f, "rsn_pairwise=CCMP\n");
				fprintf(f, "wpa=2\n");
				DPRINT_INFO(WFA_OUT, "rsn_pairwise=CCMP\n");
				DPRINT_INFO(WFA_OUT, "wpa=2\n");
			} else if (def_intf->keyMgmtType == KEY_MGNT_TYPE_SUITEB) {
				fprintf(f, "wpa=2\n");
				DPRINT_INFO(WFA_OUT, "wpa=2\n");
			} else if (def_intf->keyMgmtType == KEY_MGNT_TYPE_WPA2_EAP_MIXED) {
				fprintf(f, "wpa=3\n");
				DPRINT_INFO(WFA_OUT, "wpa=3\n");
			} else {
				fprintf(f, "wpa=1\n");
				DPRINT_INFO(WFA_OUT, "wpa=1\n");
			}

			fprintf(f, "ieee8021x=1\n");
			if (def_intf->keyMgmtType == KEY_MGNT_TYPE_SUITEB) {
				fprintf(f, "wpa_key_mgmt=WPA-EAP-SUITE-B-192\n");
				fprintf(f, "group_mgmt_cipher=BIP-GMAC-256\n");
			} else {
				fprintf(f, "wpa_key_mgmt=WPA-EAP\n");
			}

			switch (def_intf->encpType) {
			case ENCP_TYPE_NONE:
				/* do nothing */
				DPRINT_INFO(WFA_OUT, "do nothing, ENCP_TYPE_NONE\n");
				break;
			case ENCP_TYPE_TKIP:
				fprintf(f, "wpa_pairwise=TKIP\n");
				DPRINT_INFO(WFA_OUT, "wpa_pairwise=TKIP\n");
				break;
			case ENCP_TYPE_CCMP:
				fprintf(f, "wpa_pairwise=CCMP\n");
				DPRINT_INFO(WFA_OUT, "wpa_pairwise=CCMP\n");
				break;
			case ENCP_TYPE_CCMP_256:
				fprintf(f, "wpa_pairwise=CCMP-256\n");
				DPRINT_INFO(WFA_OUT, "wpa_pairwise=CCMP-256\n");
				break;
			case ENCP_TYPE_GCMP_128:
				fprintf(f, "wpa_pairwise=GCMP\n");
				DPRINT_INFO(WFA_OUT, "wpa_pairwise=GCMP\n");
				break;
			case ENCP_TYPE_GCMP_256:
				fprintf(f, "wpa_pairwise=GCMP-256\n");
				DPRINT_INFO(WFA_OUT, "wpa_pairwise=GCMP-256\n");
				break;
			case ENCP_TYPE_CCMP_TKIP:
				fprintf(f, "wpa_pairwise=TKIP CCMP\n");
				DPRINT_INFO(WFA_OUT, "wpa_pairwise=TKIP CCMP\n");
				break;
			default:
				DPRINT_INFO(WFA_OUT, "unknown encpType: %d", def_intf->encpType);
				break;
			}
			break;

		case KEY_MGNT_TYPE_WPA2_PSK_SAE:
		case KEY_MGNT_TYPE_WPA2_SAE: {
			const char *key_mgmt;

			fprintf(f, "wpa=2\n");

			switch (def_intf->pmf) {
			case WFA_DISABLED:
			case WFA_OPTIONAL:
				if (def_intf->keyMgmtType == KEY_MGNT_TYPE_WPA2_SAE)
					key_mgmt = "SAE";
				else
					key_mgmt = "WPA-PSK SAE";

				fprintf(f, "wpa_key_mgmt=%s%s\n", key_mgmt,
					def_intf->sha256ad ? " WPA-PSK-SHA256" : "");
				break;
			case WFA_REQUIRED:
				if (def_intf->keyMgmtType == KEY_MGNT_TYPE_WPA2_SAE)
					key_mgmt = "SAE";
				else
					key_mgmt = "WPA-PSK-SHA256 SAE";

				fprintf(f, "wpa_key_mgmt=%s\n", key_mgmt);
				break;
			default:
				break;
			}

			switch (def_intf->encpType) {
			case ENCP_TYPE_NONE:
				/* do nothing */
				break;
			case ENCP_TYPE_TKIP:
				fprintf(f, "wpa_pairwise=TKIP\n");
				break;
			case ENCP_TYPE_CCMP:
				fprintf(f, "wpa_pairwise=CCMP\n");
				break;
			case ENCP_TYPE_GCMP_128:
				fprintf(f, "wpa_pairwise=GCMP\n");
				break;
			case ENCP_TYPE_CCMP_TKIP:
				fprintf(f, "wpa_pairwise=CCMP TKIP\n");
				break;
			default:
				DPRINT_WARNING(WFA_WNG, "unknown encpType: %d", def_intf->encpType);
				break;
			}

			if (def_intf->keyMgmtType == KEY_MGNT_TYPE_WPA2_SAE) {
				if (def_intf->passphrase[0])
					fprintf(f, "sae_password=%s\n", def_intf->passphrase);
			} else {
				if (def_intf->passphrase[0]) {
					fprintf(f, "wpa_passphrase=%s\n", def_intf->passphrase);
					fprintf(f, "sae_password=%s\n", def_intf->passphrase);
				}
			}
			fprintf(f, "sae_pwe=%d\n", def_intf->sae_pwe);
			fprintf(f, "group_mgmt_cipher=AES-128-CMAC\n");
			break;
		}

		case KEY_MGNT_TYPE_WPA2_OWE:
			keymgnt = def_intf->keymgnt;
			if (strcasecmp(keymgnt, "OWE") == 0) {
				DPRINT_INFO(WFA_OUT, "OWE idx/bss_end_idx=%d/%d\n", bss_idx, bss_end_idx);
				def_intf->auth_algs = 1;
				if (bss_end_idx > 1) {
					if (bss_idx) {
						fprintf(f, "bssid=%s\n", shellResult_bss);
						fprintf(f, "owe_transition_bssid=%s\n", shellResult_interface);
						fprintf(f, "owe_transition_ssid=%s\n", "\"Open\"");
						fprintf(f, "ignore_broadcast_ssid=1\n");
						fprintf(f, "wpa=2\n");
						fprintf(f, "wpa_key_mgmt=OWE\n");
						fprintf(f, "ieee8021x=0\n");
						fprintf(f, "ieee80211w=2\n");
						fprintf(f, "rsn_pairwise=CCMP\n");
						if (def_intf->ap_ecGroupID[0]) {
							fprintf(f, "owe_groups=%s\n", def_intf->ap_ecGroupID);
							fprintf(f, "owe_ptk_workaround=1\n");
							DPRINT_INFO(WFA_OUT, "OWE groups=%s\n", def_intf->ap_ecGroupID);
						}
					} else {
						fprintf(f, "bssid=%s\n", shellResult_interface);
						fprintf(f, "owe_transition_bssid=%s\n", shellResult_bss);
						fprintf(f, "owe_transition_ssid=%s\n", "\"sigma-unconfig\"");
						fprintf(f, "ignore_broadcast_ssid=0\n");
						fprintf(f, "wpa=0\n");
						fprintf(f, "ieee8021x=0\n");
						fprintf(f, "ieee80211w=0\n");
					}
				} else if (bss_end_idx == 1) {
					fprintf(f, "bssid=%s\n", shellResult_interface);
					fprintf(f, "ignore_broadcast_ssid=0\n");
					fprintf(f, "wpa=2\n");
					fprintf(f, "wpa_key_mgmt=OWE\n");
					fprintf(f, "ieee8021x=0\n");
					fprintf(f, "ieee80211w=2\n");
					fprintf(f, "rsn_pairwise=CCMP\n");
					if (def_intf->ap_ecGroupID[0]) {
						fprintf(f, "owe_groups=%s\n", def_intf->ap_ecGroupID);
						fprintf(f, "owe_ptk_workaround=1\n");
						DPRINT_INFO(WFA_OUT, "OWE groups=%s\n", def_intf->ap_ecGroupID);
					}
				}
			}
			break;

		case KEY_MGNT_TYPE_UNKNOWN:
			keymgnt = def_intf->keymgnt;
			if (strcasecmp(keymgnt, "SAE FT-SAE") == 0) {
				def_intf->auth_algs = 1;
				fprintf(f, "wpa=2\n");
				fprintf(f, "wpa_key_mgmt=%s\n", keymgnt);
				fprintf(f, "ieee8021x=0\n");
				fprintf(f, "ieee80211w=2\n");
				fprintf(f, "rsn_pairwise=CCMP\n");
				fprintf(f, "disable_pmksa_caching=0\n");
				fprintf(f, "sae_pwe=%d\n", def_intf->sae_pwe);
				fprintf(f, "mobility_domain=0101\n");
				fprintf(f, "ft_r0_key_lifetime=10000\n");
				fprintf(f, "bss_transition=1\n");
				fprintf(f, "sae_password=%s\n", def_intf->passphrase);
				fprintf(f, "r1_key_holder=%s\n", def_intf->mac_addr);
				fprintf(f, "pmk_r1_push=1\n");
				fprintf(f, "nas_identifier=ap%c.mtk.com\n", def_intf->cmd_name[2]);
				fprintf(f, "ft_over_ds=%d\n", def_intf->ft_oa ? 0 : 1);
				def_intf->ft_oa = 0;
			} else if (strcasecmp(keymgnt, "WPA-EAP-SHA256 FT-EAP") == 0) {
				def_intf->auth_algs = 3;
				fprintf(f, "wpa=2\n");
				fprintf(f, "wpa_key_mgmt=%s\n", keymgnt);
				fprintf(f, "ieee8021x=1\n");
				fprintf(f, "ieee80211w=2\n");
				fprintf(f, "rsn_pairwise=CCMP\n");
				fprintf(f, "disable_pmksa_caching=0\n");
				fprintf(f, "mobility_domain=0101\n");
				fprintf(f, "ft_r0_key_lifetime=10000\n");
				fprintf(f, "own_ip_addr=%s\n", gIPaddr);
				fprintf(f, "bss_transition=1\n");
				fprintf(f, "r1_key_holder=%s\n", def_intf->mac_addr);
				fprintf(f, "pmk_r1_push=1\n");
				fprintf(f, "nas_identifier=ap%c.mtk.com\n", def_intf->cmd_name[2]);
				fprintf(f, "ft_over_ds=%d\n", def_intf->ft_oa ? 0 : 1);
				def_intf->ft_oa = 0;
			} else if (strcasecmp(keymgnt, "WPA-EAP FT-EAP WPA-EAP-SHA256") == 0) {
				def_intf->auth_algs = 3;
				fprintf(f, "wpa=2\n");
				fprintf(f, "wpa_key_mgmt=%s\n", keymgnt);
				fprintf(f, "ieee8021x=1\n");
				fprintf(f, "ieee80211w=1\n");
				fprintf(f, "rsn_pairwise=CCMP\n");
				fprintf(f, "disable_pmksa_caching=0\n");
				fprintf(f, "mobility_domain=0101\n");
				fprintf(f, "ft_r0_key_lifetime=10000\n");
				fprintf(f, "own_ip_addr=%s\n", gIPaddr);
				fprintf(f, "bss_transition=1\n");
				fprintf(f, "r1_key_holder=%s\n", def_intf->mac_addr);
				fprintf(f, "pmk_r1_push=1\n");
				fprintf(f, "nas_identifier=ap%c.mtk.com\n", def_intf->cmd_name[2]);
				fprintf(f, "ft_over_ds=%d\n", def_intf->ft_oa ? 0 : 1);
				def_intf->ft_oa = 0;
			} else if (strcasecmp(keymgnt, "WPA-PSK-SHA256 FT-PSK WPA-PSK SAE FT-SAE") == 0) {
				def_intf->auth_algs = 1;
				fprintf(f, "wpa=2\n");
				fprintf(f, "wpa_key_mgmt=%s\n", keymgnt);
				fprintf(f, "ieee8021x=0\n");
				fprintf(f, "ieee80211w=1\n");
				fprintf(f, "rsn_pairwise=CCMP\n");
				fprintf(f, "disable_pmksa_caching=0\n");
				fprintf(f, "sae_pwe=%d\n", def_intf->sae_pwe);
				fprintf(f, "mobility_domain=0101\n");
				fprintf(f, "ft_r0_key_lifetime=10000\n");
				fprintf(f, "wpa_passphrase=%s\n", def_intf->passphrase);
				fprintf(f, "bss_transition=1\n");
				fprintf(f, "r1_key_holder=%s\n", def_intf->mac_addr);
				fprintf(f, "pmk_r1_push=1\n");
				fprintf(f, "nas_identifier=ap%c.mtk.com\n", def_intf->cmd_name[2]);
				fprintf(f, "ft_over_ds=%d\n", def_intf->ft_oa ? 0 : 1);
				def_intf->ft_oa = 0;
			} else {
				DPRINT_WARNING(WFA_WNG, "unknown keymgnt: %s\n", def_intf->keymgnt);
			}
			break;
		default:
			DPRINT_WARNING(WFA_WNG, "unknown keyMgmtType: %d\n", def_intf->keyMgmtType);
			break;
		}

		if (def_intf->ft_bss_list[0]) {
			char *list = def_intf->ft_bss_list;
			char kh[150];

			strcpy(kh, list);
			if (strcasecmp(def_intf->cmd_name, "ap1mbo") == 0) {
				strcat(kh, " ");
				strcat(kh, list);
				strcat(kh, " 000102030405060708090a0b0c0d0e0f");
				fprintf(f, "r1kh=%s\n", kh);
				strcpy(kh, list);
				strcat(kh, " ap2.mtk.com 000102030405060708090a0b0c0d0e0f");
				fprintf(f, "r0kh=%s\n", kh);
			} else if (strcasecmp(def_intf->cmd_name, "ap2mbo") == 0) {
				strcat(kh, " ");
				strcat(kh, list);
				strcat(kh, " 000102030405060708090a0b0c0d0e0f");
				fprintf(f, "r1kh=%s\n", kh);
				strcpy(kh, list);
				strcat(kh, " ap1.mtk.com 000102030405060708090a0b0c0d0e0f");
				fprintf(f, "r0kh=%s\n", kh);
			}
			def_intf->ft_bss_list[0] = 0;
		}

		switch (def_intf->pmf) {
		case WFA_DISABLED:
			/* do nothing */
			break;
		case WFA_OPTIONAL:
			fprintf(f, "ieee80211w=1\n");
			if (def_intf->keyMgmtType == KEY_MGNT_TYPE_WPA2_PSK_SAE)
				fprintf(f, "sae_require_mfp=1\n");
			break;
		case WFA_REQUIRED:
			fprintf(f, "ieee80211w=2\n");
			break;
		default:
			break;
		}

		if (def_intf->trans_disable)
			fprintf(f, "transition_disable=0x%x\n", def_intf->trans_disable);

		if (def_intf->preauthentication)
			fprintf(f, "rsn_preauth=1\n");

		if (def_intf->ap_ecGroupID[0]) {
			fprintf(f, "sae_groups=%s\n", def_intf->ap_ecGroupID);
			DPRINT_INFO(WFA_OUT, "SAE groups=%s\n", def_intf->ap_ecGroupID);
		}

		if (def_intf->ap_sae_commit_override[0])
			fprintf(f, "sae_commit_override=%s\n", def_intf->ap_sae_commit_override);

		if (def_intf->antiCloggingThreshold >= 0)
			fprintf(f, "anti_clogging_threshold=%d\n", def_intf->antiCloggingThreshold);

		if (def_intf->ap_reflection)
			fprintf(f, "sae_reflection_attack=1\n");

		if (strlen(def_intf->auth_server_addr)) {
			fprintf(f, "auth_server_addr=%s\n", def_intf->auth_server_addr);
			DPRINT_INFO(WFA_OUT, "auth_server_addr=%s\n", def_intf->auth_server_addr);
		}

		if (strlen(def_intf->auth_server_port)) {
			fprintf(f, "auth_server_port=%s\n", def_intf->auth_server_port);
			DPRINT_INFO(WFA_OUT, "auth_server_port=%s\n", def_intf->auth_server_port);
		}

		if (strlen(def_intf->auth_server_shared_secret)) {
			fprintf(f, "auth_server_shared_secret=%s\n", def_intf->auth_server_shared_secret);
			DPRINT_INFO(WFA_OUT, "auth_server_shared_secret=%s\n", def_intf->auth_server_shared_secret);
		}

		fprintf(f, "ap_isolate=%d\n", (int)(def_intf->ap_isolate));
		fprintf(f, "bss_load_update_period=%d\n", (int)(def_intf->bss_load_update_period));
		fprintf(f, "chan_util_avg_period=%d\n", (int)(def_intf->chan_util_avg_period));
		fprintf(f, "disassoc_low_ack=%d\n", (int)(def_intf->disassoc_low_ack));
		fprintf(f, "skip_inactivity_poll=%d\n", (int)(def_intf->skip_inactivity_poll));
		fprintf(f, "preamble=%d\n", (int)(def_intf->preamble));
		if (strcasecmp(def_intf->keymgnt, "OWE") != 0)
			fprintf(f, "ignore_broadcast_ssid=%d\n", (int)(def_intf->ignore_broadcast_ssid));
		fprintf(f, "uapsd_advertisement_enabled=%d\n", (int)(def_intf->uapsd_advertisement_enabled));
		fprintf(f, "utf8_ssid=%d\n", (int)(def_intf->utf8_ssid));
		fprintf(f, "multi_ap=%d\n", (int)(def_intf->multi_ap));
		fprintf(f, "auth_algs=%d\n", (int)(def_intf->auth_algs));

		fprintf(f, "bridge=br-lan\n");

		if (sta_qos->ACM_BE || sta_qos->ACM_BK || sta_qos->ACM_VI || sta_qos->ACM_VO || sta_qos->AIFS_BE ||
		    sta_qos->AIFS_BK || sta_qos->AIFS_VI || sta_qos->AIFS_VO || sta_qos->cwmax_BE ||
		    sta_qos->cwmax_BK || sta_qos->cwmax_VI || sta_qos->cwmax_VO || sta_qos->cwmin_BE ||
		    sta_qos->cwmin_BK || sta_qos->cwmin_VI || sta_qos->cwmin_VO) {
			fprintf(f, "wmm_ac_bk_aifs=%d\n", sta_qos->AIFS_BK);
			fprintf(f, "wmm_ac_bk_cwmin=%d\n", sta_qos->cwmin_BK);
			fprintf(f, "wmm_ac_bk_cwmax=%d\n", sta_qos->cwmax_BK);
			fprintf(f, "wmm_ac_bk_txop_limit=%d\n", 0);
			fprintf(f, "wmm_ac_bk_acm=%d\n", sta_qos->ACM_BK);

			fprintf(f, "wmm_ac_be_aifs=%d\n", sta_qos->AIFS_BE);
			fprintf(f, "wmm_ac_be_cwmin=%d\n", sta_qos->cwmin_BE);
			fprintf(f, "wmm_ac_be_cwmax=%d\n", sta_qos->cwmax_BE);
			fprintf(f, "wmm_ac_be_txop_limit=%d\n", 0);
			fprintf(f, "wmm_ac_be_acm=%d\n", sta_qos->ACM_BE);

			fprintf(f, "wmm_ac_vi_aifs=%d\n", sta_qos->AIFS_VI);
			fprintf(f, "wmm_ac_vi_cwmin=%d\n", sta_qos->cwmin_VI);
			fprintf(f, "wmm_ac_vi_cwmax=%d\n", sta_qos->cwmax_VI);
			fprintf(f, "wmm_ac_vi_txop_limit=%d\n", 94);
			fprintf(f, "wmm_ac_vi_acm=%d\n", sta_qos->ACM_VI);

			fprintf(f, "wmm_ac_vo_aifs=%d\n", sta_qos->AIFS_VO);
			fprintf(f, "wmm_ac_vo_cwmin=%d\n", sta_qos->cwmin_VO);
			fprintf(f, "wmm_ac_vo_cwmax=%d\n", sta_qos->cwmax_VO);
			fprintf(f, "wmm_ac_vo_txop_limit=%d\n", 47);
			fprintf(f, "wmm_ac_vo_acm=%d\n", sta_qos->ACM_VO);
		}

		if (def_intf->program == PROGRAME_TYPE_MBO) {
			/* 11k */
			fprintf(f, "rrm_neighbor_report=%d\n", 1);
			fprintf(f, "rrm_beacon_report=%d\n", 1);
			/* 11v N/A */
			/* 11r */
			fprintf(f, "mobility_domain=ea91\n");
			fprintf(f, "ft_psk_generate_local=%d", 1);
			fprintf(f, "ft_over_ds=%d\n", 1);
			fprintf(f, "reassociation_deadline=%d\n", 1000);
			fprintf(f, "nas_identifier=000c432a57ab\n");
			/* Interworking IE & Extented Capabilities bit31 */
			fprintf(f, "interworking=%d\n", 1);
			/* BSS Transition Management IE & Extended Capabilities bit19 */
			fprintf(f, "bss_transition=%d\n", 1);
			/* MBO-OCE IE */
			fprintf(f, "oce=%d\n", 4);
			fprintf(f, "mbo=%d\n", 1);
			/* Country Information IE */
			fprintf(f, "ieee80211d=%d\n", 1);
			fprintf(f, "ieee80211h=%d\n", 1);
			fprintf(f, "country_code=US\n");
			fprintf(f, "country3=0x04\n");
		}

		if (def_intf->program == PROGRAME_TYPE_HE) {

			/* HE-4.60.1, ch100 */
			fprintf(f, "country_code=ZA\n");

			/* For OFDMA, pe duration: 16us */
			fprintf(f, "he_default_pe_duration=4\n");

			/* For UL OFDMA, MU EDCA */
			fprintf(f, "he_mu_edca_ac_be_aifsn=0\n");
			fprintf(f, "he_mu_edca_ac_be_ecwmin=15\n");
			fprintf(f, "he_mu_edca_ac_be_ecwmax=15\n");
			fprintf(f, "he_mu_edca_ac_be_timer=255\n");
			fprintf(f, "he_mu_edca_ac_bk_aifsn=0\n");
			fprintf(f, "he_mu_edca_ac_bk_aci=1\n");
			fprintf(f, "he_mu_edca_ac_bk_ecwmin=15\n");
			fprintf(f, "he_mu_edca_ac_bk_ecwmax=15\n");
			fprintf(f, "he_mu_edca_ac_bk_timer=255\n");
			fprintf(f, "he_mu_edca_ac_vi_aifsn=0\n");
			fprintf(f, "he_mu_edca_ac_vi_aci=2\n");
			fprintf(f, "he_mu_edca_ac_vi_ecwmin=15\n");
			fprintf(f, "he_mu_edca_ac_vi_ecwmax=15\n");
			fprintf(f, "he_mu_edca_ac_vi_timer=255\n");
			fprintf(f, "he_mu_edca_ac_vo_aifsn=0\n");
			fprintf(f, "he_mu_edca_ac_vo_aci=3\n");
			fprintf(f, "he_mu_edca_ac_vo_ecwmin=15\n");
			fprintf(f, "he_mu_edca_ac_vo_ecwmax=15\n");
			fprintf(f, "he_mu_edca_ac_vo_timer=255\n");
		}

		if (l1_valid) {
			memset(gCmdStr, 0, sizeof(gCmdStr));
			snprintf(gCmdStr, sizeof(gCmdStr), "ifconfig %s | grep %s | awk '{print $5}' > %s",
				 def_intf->name, def_intf->name, HOSTAPD_TEMP_OUTPUT);

			if (hostapd_get_cmd_output(shellResult, sizeof(shellResult), gCmdStr))
				DPRINT_INFO(WFA_OUT, "no bssid found\n");
			if (bss_idx) {
				unsigned int mac_hex[6] = {0};

				/* last bytes plus one for multi bss */
				ret_len = sscanf(shellResult, "%02x:%02x:%02x:%02x:%02x:%02x", &mac_hex[0], &mac_hex[1],
						 &mac_hex[2], &mac_hex[3], &mac_hex[4], &mac_hex[5]);
				if (ret_len != 6)
					DPRINT_INFO(WFA_OUT, "sscanf of BSSID not match\n");
				mac_hex[5] += 1;
				snprintf(shellResult, sizeof(shellResult), "%02x:%02x:%02x:%02x:%02x:%02x", mac_hex[0],
					 mac_hex[1], mac_hex[2], mac_hex[3], mac_hex[4], mac_hex[5]);
			}

			if (strcasecmp(def_intf->keymgnt, "OWE") != 0) {
				fprintf(f, "bssid=%s\n", shellResult);
				DPRINT_INFO(WFA_OUT, "bssid=%s\n", shellResult);
			}
		}
	}

	if (f != NULL)
		fclose(f);

	snprintf(gCmdStr, sizeof(gCmdStr), "chmod 0777 %s", def_intf->profile_names->profile);
	if (system(gCmdStr) != 0)
		DPRINT_INFO(WFA_OUT, "chmod 0777 %s fail\n", def_intf->profile_names->profile);

	DPRINT_INFO(WFA_OUT, "chmod 0777 %s success\n", def_intf->profile_names->profile);
	DPRINT_INFO(WFA_OUT, "*********************************\n");

	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
}

void hostapd_handle_special_cmd(mtk_ap_buf_t *mtk_ap_buf)
{
	intf_desc_t *def_intf = mtk_ap_buf->def_intf;
	int offset = 0;

	if ((def_intf->spatialRxStream < 1) || (def_intf->spatialRxStream > 4)) {
		DPRINT_INFO(WFA_OUT, "%s(), skip to set tx rx stream\n", __func__);
		return;
	}

	memset(gCmdStr, 0, sizeof(gCmdStr));

	if (mtk_ap_buf->intern_flag.capi_dual_pf) {
		if (def_intf->spatialRxStream == 1) {
			DPRINT_INFO(WFA_OUT, "%s(), spacial tx rx stream 1\n", __func__);
			offset += snprintf(gCmdStr + offset, sizeof(gCmdStr) - offset, "iw phy phy0 set antenna 1 1;");

			offset += snprintf(gCmdStr + offset, sizeof(gCmdStr) - offset, "iw phy phy0 set antenna 2 2;");

			offset += snprintf(gCmdStr + offset, sizeof(gCmdStr) - offset, "iw phy phy1 set antenna 1 1;");

			offset += snprintf(gCmdStr + offset, sizeof(gCmdStr) - offset, "iw phy phy1 set antenna 2 2");
		} else if (def_intf->spatialRxStream == 2) {
			DPRINT_INFO(WFA_OUT, "%s(), spacial tx rx stream 2\n", __func__);

			offset += snprintf(gCmdStr + offset, sizeof(gCmdStr) - offset, "iw phy phy0 set antenna 3 3;");

			offset += snprintf(gCmdStr + offset, sizeof(gCmdStr) - offset, "iw phy phy1 set antenna 3 3");
		}
	} else {
		DPRINT_INFO(WFA_OUT, "%s(), spacial rx stream %d tx stream %d\n", __func__, def_intf->spatialRxStream,
			    def_intf->spatialTxStream);
		if (def_intf->mode == WIFI_2G) {
			if (def_intf->spatialRxStream == 1) {
				DPRINT_INFO(WFA_OUT, "%s(), spacial tx rx stream 1\n", __func__);
				offset += snprintf(gCmdStr + offset, sizeof(gCmdStr) - offset,
						   "iw phy phy0 set antenna 1 1;");

				offset +=
				    snprintf(gCmdStr + offset, sizeof(gCmdStr) - offset, "iw phy phy0 set antenna 2 2");
			} else if (def_intf->spatialRxStream == 2) {
				DPRINT_INFO(WFA_OUT, "%s(), spacial tx rx stream 2\n", __func__);

				offset +=
				    snprintf(gCmdStr + offset, sizeof(gCmdStr) - offset, "iw phy phy0 set antenna 3 3");
			}
		} else {
			if (def_intf->spatialRxStream == 1) {
				DPRINT_INFO(WFA_OUT, "%s(), spacial tx rx stream 1\n", __func__);

				offset += snprintf(gCmdStr + offset, sizeof(gCmdStr) - offset,
						   "iw phy phy1 set antenna 1 1;");

				offset +=
				    snprintf(gCmdStr + offset, sizeof(gCmdStr) - offset, "iw phy phy1 set antenna 2 2");
			} else if (def_intf->spatialRxStream == 2) {
				DPRINT_INFO(WFA_OUT, "%s(), spacial tx rx stream 2\n", __func__);
				snprintf(gCmdStr, sizeof(gCmdStr), "iw phy phy1 set antenna 3 3");
				offset +=
				    snprintf(gCmdStr + offset, sizeof(gCmdStr) - offset, "iw phy phy1 set antenna 3 3");
			}
		}
	}

	system(gCmdStr);
	DPRINT_INFO(WFA_OUT, "run cmd===>%s\n", gCmdStr);
	sleep(2);
}

int hostapd_ap_config_commit(int len, uint8_t *ap_buf, int *resp_len_ptr, uint8_t *resp_buf)
{
	char conf_str[WFA_NAME_LEN];
	mtk_ap_buf_t *mtk_ap_buf = (mtk_ap_buf_t *)ap_buf;
	intf_desc_t *def_intf = mtk_ap_buf->def_intf;

	int radio_index = 0, wlan_tag_idx = 0;
	int radio_end_idx = 0;

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);

	if (mtk_ap_buf->intern_flag.capi_dual_pf)
		radio_end_idx = 2;
	else if (mtk_ap_buf->WLAN_TAG > 0) {
		for (wlan_tag_idx = 0; wlan_tag_idx < 3; wlan_tag_idx++) {
			if (mtk_ap_buf->WLAN_TAG_inf[wlan_tag_idx])
				radio_end_idx++;
		}
	} else
		radio_end_idx = 1;
	DPRINT_INFO(WFA_OUT, "radio_end_idx [%d]\n", radio_end_idx);

	for (radio_index = 0; radio_index < radio_end_idx; radio_index++) {
		DPRINT_INFO(WFA_OUT, "mtk_ap_buf->WLAN_TAG_inf[%d] %p\n", radio_index,
			    mtk_ap_buf->WLAN_TAG_inf[radio_index]);
		if (mtk_ap_buf->WLAN_TAG_inf[radio_index]) {
			def_intf = mtk_ap_buf->WLAN_TAG_inf[radio_index];
			DPRINT_INFO(WFA_OUT, "def_intf channel %d\n", def_intf->channel);
			if (def_intf->channel > 14) {
				DPRINT_INFO(WFA_OUT, "change to 5G band\n");
			} else {
				DPRINT_INFO(WFA_OUT, "change to 2G band\n");
			}
		} else {
			if (radio_index == 1) {
				if (mtk_ap_buf->def_mode == WIFI_2G) {
					def_intf = &mtk_ap_buf->intf_5G;
					DPRINT_INFO(WFA_OUT, "change to 5G band\n");
				} else if (mtk_ap_buf->def_mode == WIFI_5G) {
					def_intf = &mtk_ap_buf->intf_2G;
					DPRINT_INFO(WFA_OUT, "change to 2G band\n");
				}
			}
		}

		/* ubus call hostapd config_remove '{"iface":"wlan0"}' */
		memset(gCmdStr, 0, sizeof(gCmdStr));
		snprintf(gCmdStr, sizeof(gCmdStr), "ubus call hostapd config_remove '{\"iface\":\"%s\"}'",
			 def_intf->name);
		DPRINT_INFO(WFA_OUT, "%s\n", gCmdStr);
		if (system(gCmdStr) != 0) {
			DPRINT_INFO(WFA_OUT, "%s fail\n", gCmdStr);
		}

		sleep(2);

		hostapd_handle_special_cmd(mtk_ap_buf);

		sleep(2);
		if (mtk_ap_buf->program)
			def_intf->program = mtk_ap_buf->program;
		hostapd_write_conf(def_intf);
		sleep(2);

		DPRINT_INFO(WFA_OUT, "Delay %d seconds before turn on interface!\n",
			    mtk_ap_buf->cmd_cfg.intf_rst_delay);
		sleep(mtk_ap_buf->cmd_cfg.intf_rst_delay);

		/* ubus call hostapd config_add '{"iface":"wlan0","config":"/var/run/hostapd-phy0.conf"}' */
		snprintf(conf_str, sizeof(conf_str), " %s", def_intf->profile_names->profile);
		DPRINT_INFO(WFA_OUT, "one config %s()\n", conf_str);

		memset(gCmdStr, 0, sizeof(gCmdStr));
		snprintf(gCmdStr, sizeof(gCmdStr),
			 "ubus call hostapd config_add '{\"iface\":\"%s\",\"config\":\"%s\"}'", def_intf->name,
			 def_intf->profile_names->profile);
		DPRINT_INFO(WFA_OUT, "%s\n", gCmdStr);
		if (system(gCmdStr) != 0) {
			DPRINT_INFO(WFA_OUT, "%s fail\n", gCmdStr);
		}
		sleep(2);
	}

	if (mtk_ap_buf->cmd_cfg.post_intf_rst_delay) {
		DPRINT_INFO(WFA_OUT, "Delay %d seconds after interfaces turn on!\n",
			    mtk_ap_buf->cmd_cfg.post_intf_rst_delay);
		sleep(mtk_ap_buf->cmd_cfg.post_intf_rst_delay);
	}

	hostapd_handle_post_cmd(mtk_ap_buf);

	/* hairpin_mode=1 make sure success to ping
	 ** between two stas in the same bss
	 */
	memset(gCmdStr, 0, sizeof(gCmdStr));
	snprintf(gCmdStr, sizeof(gCmdStr), "echo 1 > /sys/devices/virtual/net/br-lan/lower_%s/brport/hairpin_mode",
		 def_intf->name);
	DPRINT_INFO(WFA_OUT, "%s\n", gCmdStr);
	if (system(gCmdStr) != 0) {
		DPRINT_INFO(WFA_OUT, "%s fail\n", gCmdStr);
	}

	hostapd_restore_profile(mtk_ap_buf);

	mtk_ap_buf->intern_flag.capi_dual_pf = 0;

	def_intf->sha256ad = 0;
	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
	return WFA_SUCCESS;
}

static int hostapd_capi_set_intf(mtk_ap_buf_t *mtk_ap_buf)
{
	capi_data_t *data = mtk_ap_buf->capi_data;

	data = mtk_ap_buf->capi_data;

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);

	DPRINT_INFO(WFA_OUT, "%s() interface %s\n", __func__, data->interface);

	DPRINT_INFO(WFA_OUT, "mtk_ap_buf->intf_2G.status=%d\n", mtk_ap_buf->intf_2G.status);
	DPRINT_INFO(WFA_OUT, "mtk_ap_buf->intf_5G.status=%d\n", mtk_ap_buf->intf_5G.status);
	DPRINT_INFO(WFA_OUT, "mtk_ap_buf->intf_6G.status=%d\n", mtk_ap_buf->intf_6G.status);
	/* set 5G default interface */
	if ((strcasecmp(data->interface, "5G") == 0) || (strcasecmp(data->interface, "5.0") == 0)) {
		if (mtk_ap_buf->intf_5G.status)
			hostapd_set_default_intf(mtk_ap_buf, WIFI_5G);
		else if (mtk_ap_buf->intf_2G.status) {
			/* Single interface, swtich to another band */
			hostapd_set_default_intf(mtk_ap_buf, WIFI_5G);
			strcpy(mtk_ap_buf->intf_5G.name, mtk_ap_buf->intf_2G.name);
			mtk_ap_buf->intf_2G.status = 0;
			DPRINT_INFO(WFA_OUT, "Single interface, Switch 2G to 5G, interface=%s\n",
				    mtk_ap_buf->intf_5G.name);
		} else {
			DPRINT_INFO(WFA_OUT, "No valid interface exist, skip!\n");
		}
	} else if ((strcasecmp(data->interface, "2G") == 0) || (strcasecmp(data->interface, "24G") == 0) ||
		   (strcasecmp(data->interface, "2.4") == 0)) {
		/* set 2G default interface */
		if (mtk_ap_buf->intf_2G.status)
			hostapd_set_default_intf(mtk_ap_buf, WIFI_2G);
		else if (mtk_ap_buf->intf_5G.status) {
			/* Single interface, swtich to another band */
			hostapd_set_default_intf(mtk_ap_buf, WIFI_2G);
			strcpy(mtk_ap_buf->intf_2G.name, mtk_ap_buf->intf_5G.name);
			mtk_ap_buf->intf_5G.status = 0;
			DPRINT_INFO(WFA_OUT, "Single interface, Switch 5G to 2G, interface=%s\n",
				    mtk_ap_buf->intf_2G.name);
		} else
			DPRINT_INFO(WFA_OUT, "No valid interface exist, skip!\n");
	} else if (strcasecmp(data->interface, "6G") == 0) {
		/* set 6G default interface */
		DPRINT_INFO(WFA_OUT, "mtk_ap_buf->intf_6G.name=%s\n", mtk_ap_buf->intf_6G.name);
		if (strcasecmp(mtk_ap_buf->intf_6G.name, "rai0") == 0) {
			if (!mtk_ap_buf->intf_6G.status) {
				if (mtk_ap_buf->intf_5G.status) {
					strcpy(mtk_ap_buf->intf_5G.name, mtk_ap_buf->intf_2G.name);
					mtk_ap_buf->intf_5G.status = 1;
					mtk_ap_buf->intf_2G.status = 0;
					if (hostapd_assign_profile_pointer_to_intf(
						mtk_ap_buf, mtk_ap_buf->intf_5G.name,
						&mtk_ap_buf->intf_5G.profile_names) != WFA_SUCCESS) {
						DPRINT_INFO(WFA_OUT, "Profile name can't be found, use 5G default!\n");
						return WFA_ERROR;
					}
				} else if (mtk_ap_buf->intf_2G.status) {
					strcpy(mtk_ap_buf->intf_2G.name, mtk_ap_buf->intf_5G.name);
					mtk_ap_buf->intf_5G.status = 0;
					mtk_ap_buf->intf_2G.status = 1;
					if (hostapd_assign_profile_pointer_to_intf(
						mtk_ap_buf, mtk_ap_buf->intf_2G.name,
						&mtk_ap_buf->intf_2G.profile_names) != WFA_SUCCESS) {
						DPRINT_INFO(WFA_OUT, "Profile name can't be found, use 2G default!\n");
						return WFA_ERROR;
					}
				}

				hostapd_init_intf_default_param(mtk_ap_buf, WIFI_6G);
			}
			hostapd_set_default_intf(mtk_ap_buf, WIFI_6G);
		} else
			DPRINT_INFO(WFA_OUT, "No valid interface exist, skip!\n");
	} else
		return WFA_ERROR;

	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
	return WFA_SUCCESS;
}

int hostapd_ap_exec(uint8_t *ap_buf, uint8_t *capi_data, uint8_t *resp_buf, int cmd_len, int *resp_len_ptr, int cmd_tag)
{
	mtk_ap_buf_t *mtk_ap_buf = NULL;
	retType_t status;

	mtk_ap_buf = (mtk_ap_buf_t *)ap_buf;
	mtk_ap_buf->capi_data = (capi_data_t *)capi_data;

	if (!ap_buf || !capi_data || !resp_buf || !resp_len_ptr) {
		DPRINT_INFO(WFA_OUT, "%s() NULL pointer\n", __func__);
		return WFA_ERROR;
	}

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);
	status = hostapd_dut_tbl[cmd_tag].cmd_mtk(cmd_len, (uint8_t *)mtk_ap_buf, resp_len_ptr, (uint8_t *)resp_buf);
	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
	return status;
}

int hostapd_ap_deauth_sta(int len, uint8_t *ap_buf, int *resp_len_ptr, uint8_t *resp_buf)
{
	mtk_ap_buf_t *mtk_ap_buf = (mtk_ap_buf_t *)ap_buf;
	intf_desc_t *def_intf = mtk_ap_buf->def_intf;
	capi_data_t *data = mtk_ap_buf->capi_data;
	char **value_ptr = data->values;
	int i = 0;
	char deauth_str[WFA_BUFF_128] = {0};

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);

	for (i = 0; i < data->count; i++) {
		if (strcasecmp((data->params)[i], "STA_MAC_Address") == 0) {
			memset(deauth_str, 0, sizeof(deauth_str));
			snprintf(deauth_str, sizeof(deauth_str), "deauthenticate %s", (value_ptr)[i]);
			hostapd_cli_cmd(def_intf, deauth_str);
		}
	}

	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
	return WFA_SUCCESS;
}

int hostapd_init(mtk_ap_buf_t *mtk_ap_buf)
{
	int wifi_intf_fd = 0;
	int ret = WFA_SUCCESS;

	if (!mtk_ap_buf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		ret = WFA_ERROR;
		goto fail;
	}

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);
	/* init profile name */
	hostapd_init_profile_name(mtk_ap_buf);

	wifi_intf_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (wifi_intf_fd < 0) {
		DPRINT_ERR(WFA_ERR, "create WIFI Interface socket() failed");
		ret = WFA_ERROR;
		goto fail;
	}
	wifi_enum_devices(wifi_intf_fd, &fillup_intf, mtk_ap_buf, 0);

	/* backup profile */
	hostapd_backup_profile(mtk_ap_buf);

	/* ap init interface and parameters */
	if (hostapd_ap_init(mtk_ap_buf) != WFA_SUCCESS) {
		DPRINT_INFO(WFA_OUT, "fail to hostapd_ap_init.\n");
		ret = WFA_ERROR;
		goto fail;
	}

fail:
	if (wifi_intf_fd > 0)
		close(wifi_intf_fd);

	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
	return ret;
}

int hostapd_ap_reset_default(int len, uint8_t *ap_buf, int *resp_len_ptr, uint8_t *resp_buf)
{
	mtk_ap_buf_t *mtk_ap_buf = (mtk_ap_buf_t *)ap_buf;
	capi_data_t *data = mtk_ap_buf->capi_data;
	retType_t status;
	char **value_ptr;
	int i = 0;

	if (!mtk_ap_buf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return WFA_ERROR;
	}

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);

	value_ptr = data->values;
	mtk_ap_buf->dev_type = UNKNOWN;
	for (i = 0; i < data->count; i++) {
		if (strcasecmp((data->params)[i], "type") == 0) {
			if (strcasecmp((value_ptr)[i], "DUT") == 0)
				mtk_ap_buf->dev_type = DUT;
			else if (strcasecmp((value_ptr)[i], "TestBed") == 0)
				mtk_ap_buf->dev_type = TESTBED;
		}
	}

	if (strcasecmp(data->program, "wpa2") == 0)
		mtk_ap_buf->program = PROGRAME_TYPE_WPA2;
	else if (strcasecmp(data->program, "wmm") == 0)
		mtk_ap_buf->program = PROGRAME_TYPE_WMM;
	else if (strcasecmp(data->program, "hs2") == 0)
		mtk_ap_buf->program = PROGRAME_TYPE_HS2;
	else if (strcasecmp(data->program, "hs2-r2") == 0)
		mtk_ap_buf->program = PROGRAME_TYPE_HS2_R2;
	else if (strcasecmp(data->program, "vht") == 0)
		mtk_ap_buf->program = PROGRAME_TYPE_VHT;
	else if (strcasecmp(data->program, "11n") == 0)
		mtk_ap_buf->program = PROGRAME_TYPE_11N;
	else if (strcasecmp(data->program, "60ghz") == 0)
		mtk_ap_buf->program = PROGRAME_TYPE_60GHZ;
	else if (strcasecmp(data->program, "loc") == 0)
		mtk_ap_buf->program = PROGRAME_TYPE_LOC;
	else if (strcasecmp(data->program, "wpa3") == 0)
		mtk_ap_buf->program = PROGRAME_TYPE_WPA3;
	else if (strcasecmp(data->program, "he") == 0)
		mtk_ap_buf->program = PROGRAME_TYPE_HE;
	else if (strcasecmp(data->program, "MBO") == 0)
		mtk_ap_buf->program = PROGRAME_TYPE_MBO;
	else
		mtk_ap_buf->program = PROGRAME_TYPE_NONE;
	DPRINT_INFO(WFA_OUT, "program: %d\n", mtk_ap_buf->program);

	if (mtk_ap_buf->Band6Gonly.intf_6G_only) {
		mtk_ap_buf->Band6Gonly.intf_6G_only = 0;
		if (mtk_ap_buf->Band6Gonly.intf_2G_orig_stat) {
			snprintf(gCmdStr, sizeof(gCmdStr), "ifconfig %s up", mtk_ap_buf->intf_2G.name);
			DPRINT_INFO(WFA_OUT, "%s() %s!\n", __func__, gCmdStr);
			system(gCmdStr);

			sleep(3);
			mtk_ap_buf->Band6Gonly.intf_2G_orig_stat = 0;
			mtk_ap_buf->intf_2G.status = 1;
		}
		if (mtk_ap_buf->Band6Gonly.intf_5G_orig_stat) {
			if (strcasecmp(mtk_ap_buf->intf_2G.name, mtk_ap_buf->intf_5G.name) != 0) {
				snprintf(gCmdStr, sizeof(gCmdStr), "ifconfig %s up", mtk_ap_buf->intf_5G.name);
				DPRINT_INFO(WFA_OUT, "%s() %s!\n", __func__, gCmdStr);
				system(gCmdStr);

				sleep(3);
			}
			mtk_ap_buf->Band6Gonly.intf_5G_orig_stat = 0;
			mtk_ap_buf->intf_5G.status = 1;
		}
	}

	hostapd_apply_sigma_profile(mtk_ap_buf, mtk_ap_buf->dev_type);

	if (mtk_ap_buf->intf_6G.status) {
		strncpy(mtk_ap_buf->intf_5G.name, mtk_ap_buf->intf_6G.name, sizeof(mtk_ap_buf->intf_5G.name));
		mtk_ap_buf->intf_5G.status = 1;
		mtk_ap_buf->intf_2G.status = 1;
		if (hostapd_assign_profile_pointer_to_intf(mtk_ap_buf, mtk_ap_buf->intf_5G.name,
							   &mtk_ap_buf->intf_5G.profile_names) != WFA_SUCCESS) {
			DPRINT_INFO(WFA_OUT, "Profile name can't be found, use default!\n");
			status = WFA_SUCCESS;
			goto ret;
		}

		mtk_ap_buf->intf_6G.status = 0;
	}

	status = hostapd_ap_init(mtk_ap_buf);

	mtk_ap_buf->intf_2G.sae_pwe = 2;
	mtk_ap_buf->intf_5G.sae_pwe = 2;
	mtk_ap_buf->intf_2G.antiCloggingThreshold = -1;
	mtk_ap_buf->intf_5G.antiCloggingThreshold = -1;
	mtk_ap_buf->intf_2G.ap_ecGroupID[0] = 0;
	mtk_ap_buf->intf_5G.ap_ecGroupID[0] = 0;
	mtk_ap_buf->intf_2G.trans_disable = 0;
	mtk_ap_buf->intf_5G.trans_disable = 0;

	mtk_ap_buf->intern_flag.committed = 0;
	mtk_ap_buf->intern_flag.BW_5G_set = 0;
	mtk_ap_buf->WLAN_TAG = -1;
	for (i = 0; i < 3; i++)
		mtk_ap_buf->WLAN_TAG_inf[i] = NULL;

	if (mtk_ap_buf->program == PROGRAME_TYPE_HE) {
		/* temp, Add for fw reload */
		DPRINT_INFO(WFA_OUT, "system: rmmod mt7615e\n");
		system("rmmod mt7615e");
		DPRINT_INFO(WFA_OUT, "system: rmmod mt7615_common\n");
		system("rmmod mt7615_common");
		DPRINT_INFO(WFA_OUT, "system: rmmod mt7915e\n");
		system("rmmod mt7915e");
		DPRINT_INFO(WFA_OUT, "system: rmmod mt76-connac-lib\n");
		system("rmmod mt76-connac-lib");
		DPRINT_INFO(WFA_OUT, "system: rmmod mt76\n");
		system("rmmod mt76");
		DPRINT_INFO(WFA_OUT, "system: rmmod mac80211\n");
		system("rmmod mac80211");
		DPRINT_INFO(WFA_OUT, "system: rmmod cfg80211\n");
		system("rmmod cfg80211");
		DPRINT_INFO(WFA_OUT, "system: insmod cfg80211\n");
		system("insmod cfg80211");
		DPRINT_INFO(WFA_OUT, "system: insmod mac80211\n");
		system("insmod mac80211");
		DPRINT_INFO(WFA_OUT, "system: insmod mt76\n");
		system("insmod mt76");
		DPRINT_INFO(WFA_OUT, "system: insmod mt76-connac-lib\n");
		system("insmod mt76-connac-lib");
		DPRINT_INFO(WFA_OUT, "system: insmod mt7915e\n");
		system("insmod mt7915e");

		DPRINT_INFO(WFA_OUT, "system: Sleep 2...\n");
		sleep(2);
		DPRINT_INFO(WFA_OUT, "system: killall hostapd\n");
		system("killall hostapd");
		DPRINT_INFO(WFA_OUT, "system: Sleep 1...\n");
		sleep(1);
	}

	/* Flush arp */
	system("ip -s -s neigh flush all");
	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);

ret:
	return status;
}

void hostapd_update_capi_dual_table(mtk_ap_buf_t *mtk_ap_buf, char *key_ptr, char *value_ptr)
{
	intf_desc_t *def_intf = NULL;
	intf_desc_t *def_intf2 = NULL;
	char value[WFA_SSID_NAME_LEN];
	char param1[WFA_SSID_NAME_LEN];
	char param2[WFA_SSID_NAME_LEN];
	char *token = NULL;

	if (!mtk_ap_buf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return;
	}
	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);

	strncpy(value, value_ptr, sizeof(value) - 1);
	if (NULL != (strstr(value, ";"))) {
		token = strtok(value, ";");
		strncpy(param1, token, sizeof(param1) - 1);
		param1[sizeof(param1) - 1] = '\0';
		DPRINT_INFO(WFA_OUT, "param1: %s\n", param1);
		token = strtok(NULL, ";");
		strncpy(param2, token, sizeof(param2) - 1);
		param2[sizeof(param2) - 1] = '\0';
		DPRINT_INFO(WFA_OUT, "param2: %s\n", param2);
	} else {
		strncpy(param1, value, sizeof(param1) - 1);
		param1[sizeof(param1) - 1] = '\0';
		strncpy(param2, value, sizeof(param2) - 1);
		param2[sizeof(param2) - 1] = '\0';
	}

	def_intf = mtk_ap_buf->def_intf;
	if (mtk_ap_buf->def_mode == WIFI_5G)
		def_intf2 = &mtk_ap_buf->intf_2G;
	else if (mtk_ap_buf->def_mode == WIFI_2G)
		def_intf2 = &mtk_ap_buf->intf_5G;

	if (strcasecmp(key_ptr, "SSID") == 0) {
		strncpy(def_intf->ssid[0], param1, sizeof(def_intf->ssid[0]) - 1);
		def_intf->ssid[0][sizeof(def_intf->ssid[0]) - 1] = '\0';
		strncpy(def_intf2->ssid[0], param2, sizeof(def_intf2->ssid[0]) - 1);
		def_intf2->ssid[0][sizeof(def_intf2->ssid[0]) - 1] = '\0';
		DPRINT_INFO(WFA_OUT, "ssid1: %s\n", def_intf->ssid[0]);
		DPRINT_INFO(WFA_OUT, "ssid2: %s\n", def_intf2->ssid[0]);
	} else if (strcasecmp(key_ptr, "CHANNEL") == 0) {
		def_intf->channel = (char)atoi(param1);
		def_intf2->channel = (char)atoi(param2);
		DPRINT_INFO(WFA_OUT, "channel 1: %d\n", def_intf->channel);
		DPRINT_INFO(WFA_OUT, "channel 2: %d\n", def_intf2->channel);
	} else if (strcasecmp(key_ptr, "Mode") == 0) {
		if (strcasecmp(param1, "11b") == 0)
			def_intf->ap_mode = AP_MODE_11b;
		else if (strcasecmp(param1, "11bg") == 0)
			def_intf->ap_mode = AP_MODE_11bg;
		else if (strcasecmp(param1, "11bgn") == 0)
			def_intf->ap_mode = AP_MODE_11bgn;
		else if (strcasecmp(param1, "11a") == 0)
			def_intf->ap_mode = AP_MODE_11a;
		else if (strcasecmp(param1, "11g") == 0)
			def_intf->ap_mode = AP_MODE_11g;
		else if (strcasecmp(param1, "11na") == 0)
			def_intf->ap_mode = AP_MODE_11na;
		else if (strcasecmp(param1, "11ng") == 0)
			def_intf->ap_mode = AP_MODE_11ng;
		else if (strcasecmp(param1, "11ac") == 0)
			def_intf->ap_mode = AP_MODE_11ac;
		else if (strcasecmp(param1, "11ad") == 0)
			def_intf->ap_mode = AP_MODE_11ad;
		else if (strcasecmp(param1, "11ax") == 0)
			def_intf->ap_mode = AP_MODE_11ax;
		else
			def_intf->ap_mode = AP_MODE_NONE;
		DPRINT_INFO(WFA_OUT, "mode1 %s\n", hostapd_apmode_2_string(def_intf->ap_mode));

		if (strcasecmp(param2, "11b") == 0)
			def_intf2->ap_mode = AP_MODE_11b;
		else if (strcasecmp(param2, "11bg") == 0)
			def_intf2->ap_mode = AP_MODE_11bg;
		else if (strcasecmp(param2, "11bgn") == 0)
			def_intf2->ap_mode = AP_MODE_11bgn;
		else if (strcasecmp(param2, "11a") == 0)
			def_intf2->ap_mode = AP_MODE_11a;
		else if (strcasecmp(param2, "11g") == 0)
			def_intf2->ap_mode = AP_MODE_11g;
		else if (strcasecmp(param2, "11na") == 0)
			def_intf2->ap_mode = AP_MODE_11na;
		else if (strcasecmp(param2, "11ng") == 0)
			def_intf2->ap_mode = AP_MODE_11ng;
		else if (strcasecmp(param2, "11ac") == 0)
			def_intf2->ap_mode = AP_MODE_11ac;
		else if (strcasecmp(param2, "11ad") == 0)
			def_intf2->ap_mode = AP_MODE_11ad;
		else if (strcasecmp(param2, "11ax") == 0)
			def_intf2->ap_mode = AP_MODE_11ax;
		else
			def_intf2->ap_mode = AP_MODE_NONE;
		DPRINT_INFO(WFA_OUT, "mode2 %s\n", hostapd_apmode_2_string(def_intf2->ap_mode));
	} else if (strcasecmp(key_ptr, "BCNINT") == 0) {
		def_intf->bcnint = atoi(param1);
		def_intf2->bcnint = atoi(param2);
		DPRINT_INFO(WFA_OUT, "bcnint %d\n", def_intf->bcnint);
		DPRINT_INFO(WFA_OUT, "bcnint2 %d\n", def_intf2->bcnint);
	} else if (strcasecmp(key_ptr, "PSK") == 0) {
		strncpy(def_intf->passphrase, param1, sizeof(def_intf->passphrase) - 1);
		strncpy(def_intf2->passphrase, param2, sizeof(def_intf2->passphrase) - 1);
		def_intf->passphrase[sizeof(def_intf->passphrase) - 1] = '\0';
		def_intf2->passphrase[sizeof(def_intf2->passphrase) - 1] = '\0';
		DPRINT_INFO(WFA_OUT, "PSK %s\n", def_intf->passphrase);
		DPRINT_INFO(WFA_OUT, "PSK2 %s\n", def_intf2->passphrase);
	} else if (strcasecmp(key_ptr, "None") == 0) {
		def_intf->keyMgmtType = KEY_MGNT_TYPE_OPEN;
		def_intf->encpType = ENCP_TYPE_NONE;
		def_intf2->keyMgmtType = KEY_MGNT_TYPE_OPEN;
		def_intf2->encpType = ENCP_TYPE_NONE;
		DPRINT_INFO(WFA_OUT, "keyMgmtType KEY_MGNT_TYPE_OPEN, encpType ENCP_TYPE_NONE\n");
		DPRINT_INFO(WFA_OUT, "keyMgmtType2 KEY_MGNT_TYPE_OPEN, encpType2 ENCP_TYPE_NONE\n");
	} else if (strcasecmp(key_ptr, "WPA-PSK") == 0) {
		def_intf->keyMgmtType = KEY_MGNT_TYPE_WPA_PSK;
		def_intf->encpType = ENCP_TYPE_TKIP;
		def_intf2->keyMgmtType = KEY_MGNT_TYPE_WPA_PSK;
		def_intf2->encpType = ENCP_TYPE_TKIP;
		DPRINT_INFO(WFA_OUT, "keyMgmtType KEY_MGNT_TYPE_WPA_PSK, encpType ENCP_TYPE_TKIP\n");
		DPRINT_INFO(WFA_OUT, "keyMgmtType2 KEY_MGNT_TYPE_WPA_PSK, encpType2 ENCP_TYPE_TKIP\n");
	} else if (strcasecmp(key_ptr, "WPA2-PSK") == 0) {
		def_intf->keyMgmtType = KEY_MGNT_TYPE_WPA2_PSK;
		def_intf->encpType = ENCP_TYPE_CCMP;
		def_intf2->keyMgmtType = KEY_MGNT_TYPE_WPA2_PSK;
		def_intf2->encpType = ENCP_TYPE_CCMP;
		DPRINT_INFO(WFA_OUT, "keyMgmtType KEY_MGNT_TYPE_WPA2_PSK, encpType ENCP_TYPE_CCMP\n");
		DPRINT_INFO(WFA_OUT, "keyMgmtType2 KEY_MGNT_TYPE_WPA2_PSK, encpType2 ENCP_TYPE_CCMP\n");
	} else if (strcasecmp(key_ptr, "WPA2-PSK-Mixed") == 0) {
		def_intf->keyMgmtType = KEY_MGNT_TYPE_WPA2_PSK_MIXED;
		def_intf->encpType = ENCP_TYPE_CCMP_TKIP;
		def_intf2->keyMgmtType = KEY_MGNT_TYPE_WPA2_PSK_MIXED;
		def_intf2->encpType = ENCP_TYPE_CCMP_TKIP;
		DPRINT_INFO(WFA_OUT, "keyMgmtType KEY_MGNT_TYPE_WPA2_PSK_MIXED, encpType ENCP_TYPE_CCMP_TKIP\n");
		DPRINT_INFO(WFA_OUT, "keyMgmtType2 KEY_MGNT_TYPE_WPA2_PSK_MIXED, encpType2 ENCP_TYPE_CCMP_TKIP\n");
	} else if (strcasecmp(key_ptr, "WPA2-ENT") == 0) {
		def_intf->keyMgmtType = KEY_MGNT_TYPE_WPA2_EAP;
		def_intf->encpType = ENCP_TYPE_CCMP;
		def_intf2->keyMgmtType = KEY_MGNT_TYPE_WPA2_EAP;
		def_intf2->encpType = ENCP_TYPE_CCMP;
		DPRINT_INFO(WFA_OUT, "keyMgmtType KEY_MGNT_TYPE_WPA2_EAP, encpType ENCP_TYPE_CCMP\n");
		DPRINT_INFO(WFA_OUT, "keyMgmtType2 KEY_MGNT_TYPE_WPA2_EAP, encpType2 ENCP_TYPE_CCMP\n");
	} else if (strcasecmp(key_ptr, "wpa2-psk-sae") == 0) {
		def_intf->keyMgmtType = KEY_MGNT_TYPE_WPA2_PSK_SAE;
		def_intf->encpType = ENCP_TYPE_CCMP;
		def_intf->pmf = WFA_OPTIONAL;
		def_intf2->keyMgmtType = KEY_MGNT_TYPE_WPA2_PSK_SAE;
		def_intf2->encpType = ENCP_TYPE_CCMP;
		def_intf2->pmf = WFA_OPTIONAL;
		DPRINT_INFO(WFA_OUT,
			    "keyMgmtType KEY_MGNT_TYPE_WPA2_PSK_SAE, encpType ENCP_TYPE_CCMP, pmf WFA_OPTIONAL\n");
		DPRINT_INFO(WFA_OUT,
			    "keyMgmtType2 KEY_MGNT_TYPE_WPA2_PSK_SAE, encpType2 ENCP_TYPE_CCMP pmf WFA_OPTIONAL\n");
	} else if (strcasecmp(key_ptr, "SAE") == 0) {
		def_intf->keyMgmtType = KEY_MGNT_TYPE_WPA2_SAE;
		def_intf->encpType = ENCP_TYPE_CCMP;
		def_intf->pmf = WFA_OPTIONAL;
		def_intf2->keyMgmtType = KEY_MGNT_TYPE_WPA2_SAE;
		def_intf2->encpType = ENCP_TYPE_CCMP;
		def_intf2->pmf = WFA_OPTIONAL;
		DPRINT_INFO(WFA_OUT, "keyMgmtType KEY_MGNT_TYPE_WPA2_SAE, encpType ENCP_TYPE_CCMP, pmf WFA_OPTIONAL\n");
		DPRINT_INFO(WFA_OUT,
			    "keyMgmtType2 KEY_MGNT_TYPE_WPA2_SAE, encpType2 ENCP_TYPE_CCMP pmf WFA_OPTIONAL\n");
	}

	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
}

int hostapd_ap_set_security(int len, uint8_t *ap_buf, int *resp_len_ptr, uint8_t *resp_buf)
{
	mtk_ap_buf_t *mtk_ap_buf = (mtk_ap_buf_t *)ap_buf;
	capi_data_t *data = mtk_ap_buf->capi_data;
	intf_desc_t *def_intf = NULL;
	char **value_ptr;
	char pairwise_cipher[42], group_cipher[42];
	int i = 0, j = 0;
	int intf_set = 1;

	DPRINT_INFO(WFA_OUT, "===== running %s function =====\n", __func__);
	if (!mtk_ap_buf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return WFA_ERROR;
	}

	strcpy(pairwise_cipher, "");
	strcpy(group_cipher, "");
	value_ptr = data->values;

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);

	intf_set = hostapd_capi_set_intf(mtk_ap_buf);
	def_intf = mtk_ap_buf->def_intf;

	for (i = 0; i < data->count; i++) {
		if (strcasecmp((data->params)[i], "WLAN_TAG") == 0) {
			if (intf_set == WFA_ERROR) {
				mtk_ap_buf->WLAN_TAG = atoi((value_ptr)[i]) - 1;
				if (mtk_ap_buf->WLAN_TAG < 0)
					mtk_ap_buf->WLAN_TAG = 0;
				if (mtk_ap_buf->WLAN_TAG_inf[mtk_ap_buf->WLAN_TAG]) {
					hostapd_set_default_intf(mtk_ap_buf,
								 mtk_ap_buf->WLAN_TAG_inf[mtk_ap_buf->WLAN_TAG]->mode);
					def_intf = mtk_ap_buf->def_intf;
				}
			}
			DPRINT_INFO(WFA_OUT, "Default interface=%s\n", mtk_ap_buf->def_intf->name);

			if (def_intf->mbss_en) {
				for (j = 0; j < def_intf->WLAN_TAG_bss_num; j++) {
					if (def_intf->WLAN_TAG[j] == atoi((value_ptr)[i])) {
						DPRINT_INFO(WFA_OUT, "found WLAN_TAG [%d]\n", def_intf->WLAN_TAG[j]);
						break;
					}
				}
			}
			DPRINT_INFO(WFA_OUT, "bss_idx %d, WLAN_TAG_bss_num %d\n", def_intf->bss_idx,
				    def_intf->WLAN_TAG_bss_num);
			continue;
		} else if ((strcasecmp((data->params)[i], "KEYMGNT") == 0) ||
			   (strcasecmp((data->params)[i], "AKMSuiteType") == 0)) {
			char *keymgnt = def_intf->keymgnt;
			if (strcasecmp((data->params)[i], "AKMSuiteType") == 0) {
				DPRINT_INFO(WFA_OUT, "AKMSuiteType: %s\n", (value_ptr)[i]);
				strcpy(keymgnt, table_search_lower(hostapd_AKM_keymgnt_tbl, (value_ptr)[i]) ?: "");
			} else {
				strcpy(keymgnt, (value_ptr)[i]);
			}
			DPRINT_INFO(WFA_OUT, "KEYMGNT: %s\n", keymgnt);

			if (strcasecmp(keymgnt, "None") == 0) {
				DPRINT_INFO(WFA_OUT, "set none attr!!\n");
				if (mtk_ap_buf->intern_flag.capi_dual_pf) {
					hostapd_update_capi_dual_table(mtk_ap_buf, "None", "0");
				} else {
					def_intf->keyMgmtType = KEY_MGNT_TYPE_OPEN;
					def_intf->encpType = ENCP_TYPE_NONE;
					DPRINT_INFO(WFA_OUT,
						    "keyMgmtType: KEY_MGNT_TYPE_OPEN, encpType: ENCP_TYPE_NONE\n");
				}
			} else if (strcasecmp(keymgnt, "WPA-PSK") == 0) {
				def_intf->keyMgmtType = KEY_MGNT_TYPE_WPA_PSK;
				def_intf->encpType = ENCP_TYPE_TKIP;
				DPRINT_INFO(WFA_OUT, "keyMgmtType: KEY_MGNT_TYPE_WPA_PSK, encpType: ENCP_TYPE_TKIP\n");
			} else if (strcasecmp(keymgnt, "WPA2-PSK") == 0) {
				if (mtk_ap_buf->intern_flag.capi_dual_pf)
					hostapd_update_capi_dual_table(mtk_ap_buf, "WPA2-PSK", "0");
				else {
					def_intf->keyMgmtType = KEY_MGNT_TYPE_WPA2_PSK;
					def_intf->encpType = ENCP_TYPE_CCMP;
					def_intf->pmf = WFA_OPTIONAL;
					DPRINT_INFO(WFA_OUT, "keyMgmtType: KEY_MGNT_TYPE_WPA2_PSK, encpType: "
							     "ENCP_TYPE_CCMP, pmf: WFA_OPTIONAL\n");
				}
			} else if (strcasecmp(keymgnt, "WPA2-PSK-Mixed") == 0) {
				if (mtk_ap_buf->intern_flag.capi_dual_pf)
					hostapd_update_capi_dual_table(mtk_ap_buf, "WPA2-PSK-Mixed", "0");
				else {
					def_intf->keyMgmtType = KEY_MGNT_TYPE_WPA2_PSK_MIXED;
					def_intf->encpType = ENCP_TYPE_CCMP_TKIP;
					DPRINT_INFO(WFA_OUT, "keyMgmtType: KEY_MGNT_TYPE_WPA2_PSK_MIXED, encpType: "
							     "ENCP_TYPE_CCMP_TKIP\n");
				}
			} else if (strcasecmp(keymgnt, "SAE") == 0) {
				if (mtk_ap_buf->intern_flag.capi_dual_pf)
					hostapd_update_capi_dual_table(mtk_ap_buf, "SAE", "0");
				else {
					def_intf->keyMgmtType = KEY_MGNT_TYPE_WPA2_SAE;
					def_intf->encpType = ENCP_TYPE_CCMP;
					def_intf->pmf = WFA_OPTIONAL;
					DPRINT_INFO(WFA_OUT, "keyMgmtType: KEY_MGNT_TYPE_WPA2_SAE, encpType: "
							     "ENCP_TYPE_CCMP, pmf: WFA_REQUIRED\n");
				}
			} else if (strcasecmp(keymgnt, "wpa2-psk-sae") == 0) {
				if (mtk_ap_buf->intern_flag.capi_dual_pf)
					hostapd_update_capi_dual_table(mtk_ap_buf, "wpa2-psk-sae", "0");
				else {
					def_intf->keyMgmtType = KEY_MGNT_TYPE_WPA2_PSK_SAE;
					def_intf->encpType = ENCP_TYPE_CCMP;
					def_intf->pmf = WFA_OPTIONAL;
					DPRINT_INFO(WFA_OUT, "keyMgmtType: KEY_MGNT_TYPE_WPA2_SAE, encpType: "
							     "ENCP_TYPE_CCMP, pmf: WFA_OPTIONAL\n");
				}
			} else if (strcasecmp(keymgnt, "WPA2-Ent") == 0) {
				if (mtk_ap_buf->intern_flag.capi_dual_pf)
					hostapd_update_capi_dual_table(mtk_ap_buf, "WPA2-Ent", "0");
				else {
					def_intf->keyMgmtType = KEY_MGNT_TYPE_WPA2_EAP;
					def_intf->encpType = ENCP_TYPE_CCMP;
					def_intf->pmf = WFA_OPTIONAL;
					DPRINT_INFO(WFA_OUT, "keyMgmtType: KEY_MGNT_TYPE_WPA2_EAP, encpType: "
							     "ENCP_TYPE_CCMP, pmf: WFA_OPTIONAL\n");
				}
			} else if (strcasecmp(keymgnt, "WPA-Ent") == 0) {
				if (mtk_ap_buf->intern_flag.capi_dual_pf)
					hostapd_update_capi_dual_table(mtk_ap_buf, "WPA-Ent", "0");
				else {
					def_intf->keyMgmtType = KEY_MGNT_TYPE_WPA_EAP;
					def_intf->encpType = ENCP_TYPE_TKIP;
					DPRINT_INFO(WFA_OUT,
						    "keyMgmtType: KEY_MGNT_TYPE_WPA_EAP, encpType: ENCP_TYPE_TKIP\n");
				}
			} else if (strcasecmp(keymgnt, "SuiteB") == 0) {
				if (mtk_ap_buf->intern_flag.capi_dual_pf) {
					hostapd_update_capi_dual_table(mtk_ap_buf, "SuiteB", "0");
				} else {
					def_intf->keyMgmtType = KEY_MGNT_TYPE_SUITEB;
					def_intf->encpType = ENCP_TYPE_GCMP_256;
					def_intf->pmf = WFA_REQUIRED;
					DPRINT_INFO(WFA_OUT, "keyMgmtType: KEY_MGNT_TYPE_SUITEB, encpType: "
							     "ENCP_TYPE_GCMP_256, pmf: WFA_REQUIRED\n");
				}
			} else if (strcasecmp(keymgnt, "OWE") == 0) {
				def_intf->keyMgmtType = KEY_MGNT_TYPE_WPA2_OWE;
				def_intf->encpType = ENCP_TYPE_CCMP;
				DPRINT_INFO(WFA_OUT, "keyMgmtType: KEY_MGNT_TYPE_WPA2_OWE, encpType: ENCP_TYPE_CCMP, "
						     "pmf: DISABLE or OPTIONAL or REQUIRED\n");
			} else {
				def_intf->keyMgmtType = KEY_MGNT_TYPE_UNKNOWN;
				DPRINT_INFO(WFA_OUT, "keyMgmtType: KEY_MGNT_TYPE_UNKNOWN\n");
			}
		} else if ((strcasecmp((data->params)[i], "PairwiseCipher") == 0)) {
			if (strcasecmp((value_ptr)[i], "AES-CCMP-128") == 0) {
				def_intf->encpType = ENCP_TYPE_CCMP;
				DPRINT_INFO(WFA_OUT, "encpType: ENCP_TYPE_CCMP\n");
			} else if (strcasecmp((value_ptr)[i], "AES-CCMP-256") == 0) {
				def_intf->encpType = ENCP_TYPE_CCMP_256;
				DPRINT_INFO(WFA_OUT, "encpType: ENCP_TYPE_CCMP_256\n");
			} else if (strcasecmp((value_ptr)[i], "AES-GCMP-128") == 0) {
				def_intf->encpType = ENCP_TYPE_GCMP_128;
				DPRINT_INFO(WFA_OUT, "encpType: ENCP_TYPE_GCMP_128\n");
			} else if (strcasecmp((value_ptr)[i], "AES-GCMP-256") == 0) {
				def_intf->encpType = ENCP_TYPE_GCMP_256;
				DPRINT_INFO(WFA_OUT, "encpType: ENCP_TYPE_GCMP_256\n");
			} else {
				DPRINT_INFO(WFA_OUT, "invalid PairwCipher: %s, use defalut\n", (value_ptr)[i]);
			}
		} else if (strcasecmp((data->params)[i], "Transition_Disable") == 0) {
			if ((strcasecmp((value_ptr)[i], "1") == 0) && (i + 1 < data->count) &&
			    (strcasecmp((data->params)[i + 1], "Transition_Disable_Index") == 0)) {
				char *index = (value_ptr)[i + 1];
				char idx_num[2];
				int j, bitmask = 0;

				memset(idx_num, 0, sizeof(idx_num));
				for (j = 0; j < strlen(index); j++) {
					idx_num[0] = index[j];
					bitmask |= 1 << atoi(idx_num);
				}
				def_intf->trans_disable = bitmask;
				i++;
			}
			DPRINT_INFO(WFA_OUT, "trans_disable: 0x%x\n", def_intf->trans_disable);
		} else if ((strcasecmp((data->params)[i], "PSK") == 0) ||
			   (strcasecmp((data->params)[i], "PSKPassPhrase") == 0) ||
			   (strcasecmp((data->params)[i], "SAEPasswords") == 0)) {
			if ((def_intf->bss_idx > 1) && (def_intf->bss_idx < 6))
				sprintf(gCmdStr, "PSK%d", def_intf->bss_idx);
			else if (def_intf->bss_idx <= 1) {
				if (mtk_ap_buf->intern_flag.capi_dual_pf)
					hostapd_update_capi_dual_table(mtk_ap_buf, "PSK", (value_ptr)[i]);
				else {
					strncpy(def_intf->passphrase, (value_ptr)[i], sizeof(def_intf->passphrase) - 1);
					DPRINT_INFO(WFA_OUT, "wpa_passphrase: %s\n", (value_ptr)[i]);
				}
				strcpy(def_intf->PWDIDR, "0");
			}
		} else if (strcasecmp((data->params)[i], "WEPKey") == 0) {
			strncpy(def_intf->wepkey, (value_ptr)[i], sizeof(def_intf->wepkey) - 1);
			DPRINT_INFO(WFA_OUT, "wep_key0:%s\n", def_intf->wepkey);
		} else if (strcasecmp((data->params)[i], "ENCRYPT") == 0) {
			if (strcasecmp((value_ptr)[i], "wep") == 0) {
				def_intf->encpType = ENCP_TYPE_WEP;
				/*
				** auth_algs=1: open,wep open
				** auth_algs=2: wep share key(ucc not test it)
				*/
				def_intf->auth_algs = 1;
			} else if (strcasecmp((value_ptr)[i], "tkip") == 0)
				def_intf->encpType = ENCP_TYPE_TKIP;
			if (strcasecmp((value_ptr)[i], "aes") == 0)
				def_intf->encpType = ENCP_TYPE_CCMP;
			else if (strcasecmp((value_ptr)[i], "aes-gcmp") == 0)
				def_intf->encpType = ENCP_TYPE_GCMP_128;
			DPRINT_INFO(WFA_OUT, "ENCRYPT:%s\n", (value_ptr)[i]);
		} else if (strcasecmp((data->params)[i], "NonTxBSSIndex") == 0) {
			def_intf->bss_idx = atoi((value_ptr)[i]) + 1;
			DPRINT_INFO(WFA_OUT, "NonTxBSSIndex:%d\n", def_intf->bss_idx);
		} else if (strcasecmp((data->params)[i], "PMF") == 0) {
			if (strcasecmp((value_ptr)[i], "Required") == 0)
				def_intf->pmf = WFA_REQUIRED;
			else if (strcasecmp((value_ptr)[i], "Optional") == 0)
				def_intf->pmf = WFA_OPTIONAL;
			else if (strcasecmp((value_ptr)[i], "Disabled") == 0)
				def_intf->pmf = WFA_DISABLED;
			DPRINT_INFO(WFA_OUT, "pmf:%s\n", (value_ptr)[i]);
		} else if (strcasecmp((data->params)[i], "SHA256AD") == 0) {
			if (strcasecmp((value_ptr)[i], "Enabled") == 0)
				def_intf->sha256ad = WFA_ENABLED;
			DPRINT_INFO(WFA_OUT, "SHA256AD:%s\n", (value_ptr)[i]);
		} else if (strcasecmp((data->params)[i], "ECGroupID") == 0) {
			snprintf(def_intf->ap_ecGroupID, sizeof(def_intf->ap_ecGroupID) - 1, (value_ptr)[i]);
			DPRINT_INFO(WFA_OUT, "ECGroupID:%s\n", def_intf->ap_ecGroupID);
		} else if (strcasecmp((data->params)[i], "sae_pwe") == 0) {
			const char *str = table_search_lower(PweMethod_tbl, (value_ptr)[i]);

			def_intf->sae_pwe = str ? atoi(str) - 1 : 2;
			DPRINT_INFO(WFA_OUT, "sae_pwe: %d\n", def_intf->sae_pwe);
		}
	}

	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
	return WFA_SUCCESS;
}

int hostapd_ap_set_rfeature(int len, uint8_t *ap_buf, int *resp_len_ptr, uint8_t *resp_buf)
{
	mtk_ap_buf_t *mtk_ap_buf = (mtk_ap_buf_t *)ap_buf;
	intf_desc_t *def_intf = NULL;
	capi_data_t *data = mtk_ap_buf->capi_data;
	char **value_ptr;
	int i = 0, j = 0;
	char ltf = 0, ltf_set = 0;
	char gi[4] = {0}, gi_set = 0;
	int Nebor_Pref = 0, Nebor_Op_Class = 0, Nebor_Op_Ch = 0;
	unsigned char BSSID[6];
	unsigned int mac_int[6] = {0};
	char cmd_content[WFA_CMD_STR_SZ];
	char Nebor_BSSID[WFA_BUFF_64];
	char SSID[WFA_BUFF_64], hex_ssid[WFA_BUFF_64];
	int offset = 0;
	int ret_len = 0;
	unsigned char Own_BSSID[6];
	unsigned int mac_hex[6] = {0};

	memset(BSSID, 0, sizeof(BSSID));
	memset(mac_int, 0, sizeof(mac_int));
	memset(cmd_content, 0, sizeof(cmd_content));
	memset(Nebor_BSSID, 0, sizeof(Nebor_BSSID));
	memset(SSID, 0, sizeof(SSID));
	memset(hex_ssid, 0, sizeof(hex_ssid));

	if (!ap_buf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return WFA_ERROR;
	}

	def_intf = mtk_ap_buf->def_intf;
	value_ptr = data->values;

	for (i = 0; i < data->count; i++) {
		if (strcasecmp((data->params)[i], "LTF") == 0) {
			if (strncmp((value_ptr)[i], "3.2", 4) == 0)
				ltf = 1;
			else if (strncmp((value_ptr)[i], "6.4", 4) == 0)
				ltf = 2;
			else if (strncmp((value_ptr)[i], "12.8", 4) == 0)
				ltf = 4;
			ltf_set = 1;
			DPRINT_INFO(WFA_OUT, "ltf %d\n", ltf);
		} else if (strcasecmp((data->params)[i], "GI") == 0) {
			snprintf(gi, sizeof(gi), (value_ptr)[i]);
			gi[3] = '\0';
			gi_set = 1;
			DPRINT_INFO(WFA_OUT, "gi %s\n", gi);
		} else if (strcasecmp((data->params)[i], "txBandwidth") == 0) {
			if (strcasecmp((value_ptr)[i], "20") == 0)
				def_intf->ch_width = CHANNEL_WIDTH_20;
			else if (strcasecmp((value_ptr)[i], "40") == 0)
				def_intf->ch_width = CHANNEL_WIDTH_40;
			else if (strcasecmp((value_ptr)[i], "80") == 0)
				def_intf->ch_width = CHANNEL_WIDTH_80;
			else if (strcasecmp((value_ptr)[i], "160") == 0)
				def_intf->ch_width = CHANNEL_WIDTH_160;
			else
				def_intf->ch_width = CHANNEL_WIDTH_AUTO;
			DPRINT_INFO(WFA_OUT, "txBandwidth %s, channel %d\n", (value_ptr)[i], def_intf->channel);

			/* Re-run hostapd */
			hostapd_ap_config_commit(len, ap_buf, resp_len_ptr, resp_buf);
		} else if (strcasecmp((data->params)[i], "PPDUTxType") == 0) {
			char ppdu_type = 0;

			if (strcasecmp((value_ptr)[i], "MU") == 0)
				ppdu_type = 1;
			else if (strcasecmp((value_ptr)[i], "SU") == 0)
				ppdu_type = 0;
			else
				ppdu_type = 4;

			memset(gCmdStr, 0, sizeof(gCmdStr));
			snprintf(gCmdStr, sizeof(gCmdStr), "mt76-vendor %s set ap_wireless ppdu_type=%d\n",
				 def_intf->name, (int)ppdu_type);
			DPRINT_INFO(WFA_OUT, "run command ==> %s", gCmdStr);
			system(gCmdStr);
		} else if (strcasecmp((data->params)[i], "TriggerType") == 0) {
			memset(gCmdStr, 0, sizeof(gCmdStr));
			snprintf(gCmdStr, sizeof(gCmdStr), "mt76-vendor %s set ap_rfeatures trig_type=1,%s\n",
				 def_intf->name, (value_ptr)[i]);
			DPRINT_INFO(WFA_OUT, "run command ==> %s", gCmdStr);
			system(gCmdStr);
		} else if (strcasecmp((data->params)[i], "AckPolicy") == 0) {
			memset(gCmdStr, 0, sizeof(gCmdStr));
			snprintf(gCmdStr, sizeof(gCmdStr), "mt76-vendor %s set ap_rfeatures ack_policy=%s\n",
				 def_intf->name, (value_ptr)[i]);
			DPRINT_INFO(WFA_OUT, "run command ==> %s", gCmdStr);
			system(gCmdStr);
		} else if (strcasecmp((data->params)[i], "DisableTriggerType") == 0) {
			memset(gCmdStr, 0, sizeof(gCmdStr));
			snprintf(gCmdStr, sizeof(gCmdStr), "mt76-vendor %s set ap_rfeatures trig_type=0,%s\n",
				 def_intf->name, (value_ptr)[i]);
			DPRINT_INFO(WFA_OUT, "run command ==> %s", gCmdStr);
			system(gCmdStr);
		} else if (strcasecmp((data->params)[i], "BTMReq_DisAssoc_Imnt") == 0)
			def_intf->BTMReq_DisAssoc_Imnt = 1;
		else if (strcasecmp((data->params)[i], "BTMReq_Term_Bit") == 0)
			def_intf->BTMReq_Term_Bit = 1;
		else if (strcasecmp((data->params)[i], "BSS_Term_Duration") == 0)
			def_intf->BSS_Term_Duration = atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "BSS_Term_TSF") == 0) {
			/* This is special for MBO TC 4.2.5.4 */
			char shellResult[WFA_CMD_STR_SZ];

			memset(gCmdStr, 0, sizeof(gCmdStr));
			snprintf(gCmdStr, sizeof(gCmdStr), "hostapd_cli set bss_termination_tsf %s\n", (value_ptr)[i]);
			DPRINT_INFO(WFA_OUT, "run command ==> %s", gCmdStr);
			system(gCmdStr);

			memset(gCmdStr, 0, sizeof(gCmdStr));
			snprintf(gCmdStr, sizeof(gCmdStr), "ifconfig %s | grep %s | awk '{print $5}' > %s",
				 def_intf->name, def_intf->name, HOSTAPD_TEMP_OUTPUT);

			if (hostapd_get_cmd_output(shellResult, sizeof(shellResult), gCmdStr)) {
				DPRINT_INFO(WFA_OUT, "no bssid found\n");
			}

			ret_len = sscanf(shellResult, "%02x:%02x:%02x:%02x:%02x:%02x", &mac_hex[0], &mac_hex[1],
					 &mac_hex[2], &mac_hex[3], &mac_hex[4], &mac_hex[5]);
			if (ret_len != 6) {
				DPRINT_INFO(WFA_OUT, "sscanf of BSSID of BSS_Term_TSF not match\n");
			}

			for (j = 0; j < 6; j++)
				Own_BSSID[j] = (unsigned char)mac_hex[j];

			DPRINT_INFO(WFA_OUT, "Delete Neighbor Report for own BSS %s", shellResult);
			offset = 0;
			memset(cmd_content, 0, sizeof(cmd_content));
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "remove_neighbor ");

			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1,
					   " %02x:%02x:%02x:%02x:%02x:%02x ", MAC2STR(Own_BSSID));
			hostapd_cli_cmd(def_intf, cmd_content);

			DPRINT_INFO(WFA_OUT, "Add Neighbor Report for own BSS %s", shellResult);
			offset = 0;
			memset(cmd_content, 0, sizeof(cmd_content));
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "set_neighbor ");

			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1,
					   " %02x:%02x:%02x:%02x:%02x:%02x ", MAC2STR(Own_BSSID));

			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1,
					   " ssid=\"6f6f6f6f6f\" nr=");
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1,
					   "%02x%02x%02x%02x%02x%02x", MAC2STR(Own_BSSID));
			offset +=
			    snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "931c00007324090301ff");
			hostapd_cli_cmd(def_intf, cmd_content);
		} else if (strcasecmp((data->params)[i], "Assoc_Disallow") == 0) {
			memset(gCmdStr, 0, sizeof(gCmdStr));
			snprintf(gCmdStr, sizeof(gCmdStr), "hostapd_cli set mbo_assoc_disallow 1\n");
			DPRINT_INFO(WFA_OUT, "run command ==> %s", gCmdStr);
			system(gCmdStr);
		} else if (strcasecmp((data->params)[i], "Nebor_BSSID") == 0) {
			snprintf(Nebor_BSSID, sizeof(Nebor_BSSID) - 1, (value_ptr)[i]);
			ret_len = sscanf((value_ptr)[i], "%02x:%02x:%02x:%02x:%02x:%02x", mac_int, (mac_int + 1),
					 (mac_int + 2), (mac_int + 3), (mac_int + 4), (mac_int + 5));
			if (ret_len != 6) {
				DPRINT_INFO(WFA_OUT, "sscanf of Nebor_BSSID not match\n");
			}
			for (j = 0; j < 6; j++)
				BSSID[j] = (unsigned char)mac_int[j];
			DPRINT_INFO(WFA_OUT, "Nebor_BSSID " MACSTR "\n", MAC2STR(BSSID));

			memset(cmd_content, 0, sizeof(cmd_content));
			snprintf(cmd_content, sizeof(cmd_content), "remove_neighbor %s\n", (value_ptr)[i]);
			hostapd_cli_cmd(def_intf, cmd_content);
		} else if (strcasecmp((data->params)[i], "Nebor_Pref") == 0) {
			ret_len = sscanf((value_ptr)[i], "%d", &Nebor_Pref);
			if (ret_len != 1) {
				DPRINT_INFO(WFA_OUT, "sscanf of Nebor_Pref not match\n");
			}
		} else if (strcasecmp((data->params)[i], "Nebor_Op_Class") == 0) {
			ret_len = sscanf((value_ptr)[i], "%d", &Nebor_Op_Class);
			if (ret_len != 1) {
				DPRINT_INFO(WFA_OUT, "sscanf of Nebor_Op_Class not match\n");
			}
		} else if (strcasecmp((data->params)[i], "Nebor_Op_Ch") == 0) {
			ret_len = sscanf((value_ptr)[i], "%d", &Nebor_Op_Ch);
			if (ret_len != 1) {
				DPRINT_INFO(WFA_OUT, "sscanf of Nebor_Op_Ch not match\n");
			}
		} else {
			DPRINT_INFO(WFA_OUT, "[%s] [%s]\n", data->params[i], (value_ptr)[i]);
		}
	}

	if (ltf_set || gi_set) {
		char band[4] = {0};

		if (def_intf->channel > 14)
			snprintf(band, sizeof(band), "5");
		else
			snprintf(band, sizeof(band), "2.4");
		band[3] = '\0';

		memset(gCmdStr, 0, sizeof(gCmdStr));
		snprintf(gCmdStr, sizeof(gCmdStr), "iw dev %s set bitrates he-gi-%s %s he-ltf-%s %d\n", def_intf->name,
			 band, gi, band, ltf);
		DPRINT_INFO(WFA_OUT, "run command ==> %s", gCmdStr);
		system(gCmdStr);
	}

	if (strlen(Nebor_BSSID) || Nebor_Pref) {
		memset(cmd_content, 0, sizeof(cmd_content));
		/* set_neighbor BSSID */
		offset +=
		    snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "set_neighbor %s", Nebor_BSSID);
		/* ssid="MBOxx" with random num */
		srand((unsigned int)time(NULL));
		snprintf(SSID, sizeof(SSID) - 1, "MBO%d", rand());
		ret_len = hostapd_str_to_hex(hex_ssid, sizeof(hex_ssid), SSID, strlen(SSID));
		if (ret_len >= sizeof(hex_ssid))
			DPRINT_INFO(WFA_OUT, "SSID too long\n");
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, " ssid=\"%s\"", hex_ssid);
		/* BSSID */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1,
				   " nr=%02x%02x%02x%02x%02x%02x", MAC2STR(BSSID));

		if (Nebor_Op_Class && Nebor_Op_Ch) {
			/* BSSID info: 00000000 */
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "00000000");
			/* Nebor_Op_Class */
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%02x",
					   (unsigned char)Nebor_Op_Class);
			/* Nebor_Op_Ch */
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%02x",
					   (unsigned char)Nebor_Op_Ch);
			/* Phy type: 0 */
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%02x", 0);
		} else {
			/* BSSID info: 931c0000 */
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "931c0000");
			/* Nebor_Op_Class: 115 */
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%02x", 115);
			/* Nebor_Op_Ch: 36 */
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%02x", 36);
			/* Phy type: 09 */
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%02x", 9);
		}
		/* BSS Transition Preference tlv */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "0301%02x\n",
				   (unsigned char)Nebor_Pref);
		hostapd_cli_cmd(def_intf, cmd_content);
	}

	return 0;
}

int hostapd_ap_set_wireless(int len, uint8_t *ap_buf, int *resp_len_ptr, uint8_t *resp_buf)
{
	mtk_ap_buf_t *mtk_ap_buf = (mtk_ap_buf_t *)ap_buf;
	intf_desc_t *def_intf = NULL;
	capi_data_t *data = mtk_ap_buf->capi_data;
	char **value_ptr = NULL;
	int i = 0, j = 0;
	int intf_set = 1;

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);
	intf_set = hostapd_capi_set_intf(mtk_ap_buf);
	mtk_ap_buf->WLAN_TAG = -1;
	def_intf = mtk_ap_buf->def_intf;

	if (((mtk_ap_buf->def_mode == WIFI_5G) || (mtk_ap_buf->def_mode == WIFI_6G)) &&
	    (!mtk_ap_buf->intern_flag.BW_5G_set)) {
		DPRINT_INFO(WFA_OUT, "Set 5G/6G interface default VTH BW to 80MHz!\n");
		def_intf->ch_width = CHANNEL_WIDTH_80;
	}

	value_ptr = data->values;
	DPRINT_INFO(WFA_OUT, "-----start looping: %d\n", data->count);

	for (i = 0; i < data->count; i++) {
		if (strcasecmp((data->params)[i], "WLAN_TAG") == 0) {
			mtk_ap_buf->WLAN_TAG = atoi((value_ptr)[i]) - 1;
			DPRINT_INFO(WFA_OUT, "WLAN_TAG: %d\n", mtk_ap_buf->WLAN_TAG);
			if (mtk_ap_buf->WLAN_TAG < 0)
				mtk_ap_buf->WLAN_TAG = 0;
			if (mtk_ap_buf->WLAN_TAG > 2)
				mtk_ap_buf->WLAN_TAG = 2;

			DPRINT_INFO(WFA_OUT, "intf_set: %d\n", intf_set);

			if (intf_set == WFA_SUCCESS) {
				mtk_ap_buf->WLAN_TAG_inf[mtk_ap_buf->WLAN_TAG] = def_intf;

				for (j = 0; j < def_intf->WLAN_TAG_bss_num; j++) {
					if (def_intf->WLAN_TAG[j] == atoi((value_ptr)[i]))
						break;
				}

				if (j == def_intf->WLAN_TAG_bss_num) {
					def_intf->WLAN_TAG[def_intf->WLAN_TAG_bss_num++] = atoi((value_ptr)[i]);
					if (def_intf->WLAN_TAG_bss_num > 1) {
						def_intf->bss_idx = def_intf->WLAN_TAG_bss_num - 1;
						if (def_intf->mbss_en == 0)
							def_intf->mbss_en = 1;
					}
				}
				DPRINT_INFO(WFA_OUT, "mbss_en: %d\n", def_intf->mbss_en);
			} else {
				if (mtk_ap_buf->WLAN_TAG_inf[mtk_ap_buf->WLAN_TAG]) {
					hostapd_set_default_intf(mtk_ap_buf,
								 mtk_ap_buf->WLAN_TAG_inf[mtk_ap_buf->WLAN_TAG]->mode);
					def_intf = mtk_ap_buf->def_intf;
				}
			}
			DPRINT_INFO(WFA_OUT, "Default interface=%s\n", mtk_ap_buf->def_intf->name);
			continue;
		} else if (strcasecmp((data->params)[i], "MBSSID") == 0) {
			if (strcasecmp((value_ptr)[i], "enable") == 0)
				def_intf->mbss_en = 1;
			else if (strcasecmp((value_ptr)[i], "disable") == 0)
				def_intf->mbss_en = 0;
			continue;
		} else if (strcasecmp((data->params)[i], "ssid") == 0) {
			if (def_intf->bss_idx > 1) {
				strncpy(def_intf->ssid[1], (value_ptr)[i], sizeof(def_intf->ssid[1]) - 1);
				def_intf->ssid[1][WFA_IF_NAME_LEN - 1] = '\0';
				DPRINT_INFO(WFA_OUT, "multi ssid: %s\n", def_intf->ssid[1]);
				continue;
			}

			if (mtk_ap_buf->intern_flag.capi_dual_pf)
				hostapd_update_capi_dual_table(mtk_ap_buf, "SSID", (value_ptr)[i]);
			else {
				strncpy(def_intf->ssid[0], (value_ptr)[i], sizeof(def_intf->ssid[0]) - 1);
				def_intf->ssid[0][WFA_IF_NAME_LEN - 1] = '\0';
				DPRINT_INFO(WFA_OUT, "main ssid: %s\n", def_intf->ssid[0]);
			}
			continue;
		} else if (strcasecmp((data->params)[i], "channel") == 0) {
			if (NULL != (strstr((value_ptr)[i], ";"))) {
				mtk_ap_buf->intern_flag.capi_dual_pf = 1;
				DPRINT_INFO(WFA_OUT, "set capi_dual_pf %d\n", mtk_ap_buf->intern_flag.capi_dual_pf);
			}

			if (mtk_ap_buf->intern_flag.capi_dual_pf)
				hostapd_update_capi_dual_table(mtk_ap_buf, "CHANNEL", (value_ptr)[i]);
			else {
				def_intf->channel = atoi((value_ptr)[i]);
				DPRINT_INFO(WFA_OUT, "channel %d\n", def_intf->channel);
			}
			continue;
		} else if (strcasecmp((data->params)[i], "width") == 0) {
			if (strcasecmp((value_ptr)[i], "20") == 0)
				def_intf->ch_width = CHANNEL_WIDTH_20;
			else if (strcasecmp((value_ptr)[i], "40") == 0)
				def_intf->ch_width = CHANNEL_WIDTH_40;
			else if (strcasecmp((value_ptr)[i], "80") == 0)
				def_intf->ch_width = CHANNEL_WIDTH_80;
			else if (strcasecmp((value_ptr)[i], "160") == 0)
				def_intf->ch_width = CHANNEL_WIDTH_160;
			else
				def_intf->ch_width = CHANNEL_WIDTH_AUTO;
			DPRINT_INFO(WFA_OUT, "width %s\n", (value_ptr)[i]);
			mtk_ap_buf->intern_flag.BW_5G_set = 1;
		} else if (strcasecmp((data->params)[i], "mode") == 0) {
			if (mtk_ap_buf->intern_flag.capi_dual_pf)
				hostapd_update_capi_dual_table(mtk_ap_buf, "MODE", (value_ptr)[i]);
			else {
				if (strcasecmp((value_ptr)[i], "11b") == 0)
					def_intf->ap_mode = AP_MODE_11b;
				else if (strcasecmp((value_ptr)[i], "11bg") == 0)
					def_intf->ap_mode = AP_MODE_11bg;
				else if (strcasecmp((value_ptr)[i], "11bgn") == 0)
					def_intf->ap_mode = AP_MODE_11bgn;
				else if (strcasecmp((value_ptr)[i], "11a") == 0)
					def_intf->ap_mode = AP_MODE_11a;
				else if (strcasecmp((value_ptr)[i], "11g") == 0)
					def_intf->ap_mode = AP_MODE_11g;
				else if (strcasecmp((value_ptr)[i], "11na") == 0)
					def_intf->ap_mode = AP_MODE_11na;
				else if (strcasecmp((value_ptr)[i], "11ng") == 0)
					def_intf->ap_mode = AP_MODE_11ng;
				else if (strcasecmp((value_ptr)[i], "11ac") == 0)
					def_intf->ap_mode = AP_MODE_11ac;
				else if (strcasecmp((value_ptr)[i], "11ad") == 0)
					def_intf->ap_mode = AP_MODE_11ad;
				else if (strcasecmp((value_ptr)[i], "11ax") == 0) {
					char phy_name[5];

					if (strncasecmp(def_intf->name, "wlan0", 5) == 0)
						strcpy(phy_name, "phy0");
					else if (strncasecmp(def_intf->name, "wlan1", 5) == 0)
						strcpy(phy_name, "phy1");

					def_intf->ap_mode = AP_MODE_11ax;
					/* Only support BW20 in 2.4 GHz*/
					if (def_intf->channel <= 14)
						def_intf->ch_width = CHANNEL_WIDTH_20;
					if (strncasecmp(def_intf->ssid[0], "HE-4.53.1", 9) == 0) {
						/* Add for DL Mimo, HE-4.53.1 */
						def_intf->txbf = WFA_ENABLED;
						def_intf->DL = 1;
						def_intf->mimo = 1;
						memset(gCmdStr, 0, sizeof(gCmdStr));
						snprintf(gCmdStr, sizeof(gCmdStr),
							 "mt76-vendor %s set ap_wireless ppdu_type=1\n",
							 def_intf->name);
						add_post_cmd(mtk_ap_buf);
						DPRINT_INFO(WFA_OUT, "Mode 11ax, SSID:%s, txbf:%d, DL:%d, mimo:%d\n",
							    def_intf->ssid[0], def_intf->txbf, def_intf->DL,
							    def_intf->mimo);
					}
					/* To enable OMI and bypass bf smooth*/
					snprintf(gCmdStr, sizeof(gCmdStr), "mt76-vendor %s set ap_wireless cert=1\n",
						 def_intf->name);
					add_post_cmd(mtk_ap_buf);
					/* Disable Mac80211 atf */
					if (strncasecmp(def_intf->ssid[0], "HE-4.54.1", 9) == 0) {
						snprintf(gCmdStr, sizeof(gCmdStr),
							 "echo 0 > /sys/kernel/debug/ieee80211/%s/airtime_flags\n",
							 phy_name);
						add_post_cmd(mtk_ap_buf);
					}
				} else
					def_intf->ap_mode = AP_MODE_NONE;
				DPRINT_INFO(WFA_OUT, "mode %s\n", hostapd_apmode_2_string(def_intf->ap_mode));
			}
		} else if (strcasecmp((data->params)[i], "rts") == 0) {
			def_intf->rts = atoi((value_ptr)[i]);
			DPRINT_INFO(WFA_OUT, "rts %d\n", def_intf->rts);
		} else if (strcasecmp((data->params)[i], "frgmnt") == 0) {
			def_intf->frgmnt = atoi((value_ptr)[i]);
			DPRINT_INFO(WFA_OUT, "fragment: %d\n", def_intf->frgmnt);
		} else if (strcasecmp((data->params)[i], "bcnint") == 0) {
			if (mtk_ap_buf->intern_flag.capi_dual_pf)
				hostapd_update_capi_dual_table(mtk_ap_buf, "BCNINT", (value_ptr)[i]);
			else {
				def_intf->bcnint = atoi((value_ptr)[i]);
				DPRINT_INFO(WFA_OUT, "bcnint %d\n", def_intf->bcnint);
			}
		} else if (strcasecmp((data->params)[i], "offset") == 0) {
			if (strcasecmp((value_ptr)[i], "above") == 0)
				def_intf->ch_offset = CHANNEL_OFFSET_ABOVE;
			else if (strcasecmp((value_ptr)[i], "below") == 0)
				def_intf->ch_offset = CHANNEL_OFFSET_BELOW;
			else {
				DPRINT_INFO(WFA_OUT, "invalid offset value: %s\n", (value_ptr)[i]);
				def_intf->ch_offset = CHANNEL_OFFSET_NONE;
			}
			DPRINT_INFO(WFA_OUT, "offset %d\n", def_intf->ch_offset);
		} else if (strcasecmp((data->params)[i], "spatial_tx_stream") == 0) {
			def_intf->spatialTxStream = (char)atoi((value_ptr)[i]);
			DPRINT_INFO(WFA_OUT, "spatial_tx_stream %d\n", def_intf->spatialTxStream);
		} else if (strcasecmp((data->params)[i], "spatial_rx_stream") == 0) {
			def_intf->spatialRxStream = (char)atoi((value_ptr)[i]);
			DPRINT_INFO(WFA_OUT, "spatial_rx_stream %d\n", def_intf->spatialRxStream);
		} else if (strcasecmp((data->params)[i], "dtim") == 0) {
			def_intf->dtim_period = atoi((value_ptr)[i]);
			DPRINT_INFO(WFA_OUT, "dtim: %d\n", def_intf->dtim_period);
		} else if (strcasecmp((data->params)[i], "sgi20") == 0) {
			if (strcasecmp((value_ptr)[i], "enable") == 0)
				def_intf->sgi20 = WFA_ENABLED;
			else
				def_intf->sgi20 = WFA_DISABLED;
			DPRINT_INFO(WFA_OUT, "sgi20: %d\n", def_intf->sgi20);
		} else if (strcasecmp((data->params)[i], "prog") == 0 ||
			   strcasecmp((data->params)[i], "program") == 0) {
			if (strcasecmp((value_ptr)[i], "wpa2") == 0)
				def_intf->program = PROGRAME_TYPE_WPA2;
			else if (strcasecmp((value_ptr)[i], "wmm") == 0)
				def_intf->program = PROGRAME_TYPE_WMM;
			else if (strcasecmp((value_ptr)[i], "hs2") == 0)
				def_intf->program = PROGRAME_TYPE_HS2;
			else if (strcasecmp((value_ptr)[i], "hs2-r2") == 0)
				def_intf->program = PROGRAME_TYPE_HS2_R2;
			else if (strcasecmp((value_ptr)[i], "vht") == 0)
				def_intf->program = PROGRAME_TYPE_VHT;
			else if (strcasecmp((value_ptr)[i], "11n") == 0)
				def_intf->program = PROGRAME_TYPE_11N;
			else if (strcasecmp((value_ptr)[i], "60ghz") == 0)
				def_intf->program = PROGRAME_TYPE_60GHZ;
			else if (strcasecmp((value_ptr)[i], "loc") == 0)
				def_intf->program = PROGRAME_TYPE_LOC;
			else if (strcasecmp((value_ptr)[i], "wpa3") == 0)
				def_intf->program = PROGRAME_TYPE_WPA3;
			else if (strcasecmp((value_ptr)[i], "he") == 0)
				def_intf->program = PROGRAME_TYPE_HE;
			else if (strcasecmp((value_ptr)[i], "MBO") == 0)
				def_intf->program = PROGRAME_TYPE_MBO;
			else
				def_intf->program = PROGRAME_TYPE_NONE;
			DPRINT_INFO(WFA_OUT, "program: %d\n", def_intf->program);
		} else if (strcasecmp((data->params)[i], "dyn_bw_sgnl") == 0) {
			if (strcasecmp((value_ptr)[i], "enable") == 0)
				def_intf->dynamic_bw_signaling = WFA_ENABLED;
			else
				def_intf->dynamic_bw_signaling = WFA_DISABLED;
			DPRINT_INFO(WFA_OUT, "dynamic_bw_signaling: %d\n", def_intf->dynamic_bw_signaling);
		} else if (strcasecmp((data->params)[i], "TxBF") == 0) {
			if (strcasecmp((value_ptr)[i], "enable") == 0)
				def_intf->txbf = WFA_ENABLED;
			else
				def_intf->txbf = WFA_DISABLED;
			DPRINT_INFO(WFA_OUT, "txbf: %d\n", def_intf->txbf);
		} else if (strcasecmp((data->params)[i], "mcs_fixedrate") == 0) {
			DPRINT_INFO(WFA_OUT, "mcs_fixed_rate: %s\n", (value_ptr)[i]);
			memset(gCmdStr, 0, sizeof(gCmdStr));
			snprintf(gCmdStr, sizeof(gCmdStr), "iw dev %s set bitrates he-mcs-%s %d:%s", def_intf->name,
				 (def_intf->channel > 14) ? "5" : "2.4", 1, (value_ptr)[i]);

			add_post_cmd(mtk_ap_buf);
		} else if (strcasecmp((data->params)[i], "NonTxBSSIndex") == 0) {
			def_intf->bss_idx = atoi((value_ptr)[i]) + 1;
			DPRINT_INFO(WFA_OUT, "NonTxBSSIndex:%d\n", def_intf->bss_idx);
			continue;
		} else if (strcasecmp((data->params)[i], "NumNonTxBSS") == 0) {
			def_intf->bss_num = atoi((value_ptr)[i]) + 1;
			DPRINT_INFO(WFA_OUT, "NumNonTxBSS: in str %s, in integer %d\n", (value_ptr)[i],
				    def_intf->bss_num - 1);
		} else if (strcasecmp((data->params)[i], "LDPC") == 0) {
			if (strcasecmp((value_ptr)[i], "enable") == 0)
				def_intf->bcc_mode = 0;
			DPRINT_INFO(WFA_OUT, "LDPC: %d\n", !def_intf->bcc_mode);
		} else if (strcasecmp((data->params)[i], "BCC") == 0) {
			if (strcasecmp((value_ptr)[i], "enable") == 0)
				def_intf->bcc_mode = 1;
			else if (strcasecmp((value_ptr)[i], "disable") == 0)
				def_intf->bcc_mode = 0;
			DPRINT_INFO(WFA_OUT, "bcc_mode: %d\n", def_intf->bcc_mode);
		} else if (strcasecmp((data->params)[i], "OFDMA") == 0) {
			int ofdma = 0;

			if (strcasecmp((value_ptr)[i], "DL") == 0) {
				def_intf->DL = 1;
				ofdma = 1;
			} else if (strcasecmp((value_ptr)[i], "UL") == 0)
				ofdma = 2;
			else if (strcasecmp((value_ptr)[i], "DL-20and80") == 0) {
				def_intf->DL = 1;
				ofdma = 3;
			} else
				ofdma = 0;

			memset(gCmdStr, 0, sizeof(gCmdStr));
			sprintf(gCmdStr, "mt76-vendor %s set ap_wireless ofdma=%d\n", def_intf->name, ofdma);
			add_post_cmd(mtk_ap_buf);

			memset(gCmdStr, 0, sizeof(gCmdStr));
			sprintf(gCmdStr, "mt76-vendor %s set ap_wireless add_ba_req_bufsize=64\n", def_intf->name);
			add_post_cmd(mtk_ap_buf);
		} else if (strcasecmp((data->params)[i], "PPDUTxType") == 0) {
			char ppdu_tx_type = 0;

			if (strcasecmp((value_ptr)[i], "MU") == 0)
				ppdu_tx_type = 1;
			else if (strcasecmp((value_ptr)[i], "SU") == 0)
				ppdu_tx_type = 0;
			else
				ppdu_tx_type = 4;

			memset(gCmdStr, 0, sizeof(gCmdStr));
			snprintf(gCmdStr, sizeof(gCmdStr), "mt76-vendor %s set ap_wireless ppdu_type=%d\n",
				 def_intf->name, (int)ppdu_tx_type);
			add_post_cmd(mtk_ap_buf);
		} else if (strcasecmp((data->params)[i], "MIMO") == 0) {
			if (strcasecmp((value_ptr)[i], "DL") == 0) {
				def_intf->DL = 1;
				def_intf->mimo = 1;
				memset(gCmdStr, 0, sizeof(gCmdStr));
				sprintf(gCmdStr, "mt76-vendor %s set ap_wireless mimo=0\n", def_intf->name);
				add_post_cmd(mtk_ap_buf);
			} else if (strcasecmp((value_ptr)[i], "UL") == 0)
				def_intf->UL_MUMIMO = 1;
			DPRINT_INFO(WFA_OUT, "MIMO UL %d, MIMO DL %d\n", def_intf->UL_MUMIMO, def_intf->DL);
		} else if (strcasecmp((data->params)[i], "NumUsersOFDMA") == 0) {
			memset(gCmdStr, 0, sizeof(gCmdStr));
			sprintf(gCmdStr, "mt76-vendor %s set ap_wireless nusers_ofdma=%s\n", def_intf->name,
				(value_ptr)[i]);
			add_post_cmd(mtk_ap_buf);
		} else if (strcasecmp((data->params)[i], "radio") == 0) {
			if (strcasecmp((value_ptr)[i], "off") == 0) {
				if (!def_intf) {
					/* no interface ready, off all the radios */
					memset(gCmdStr, 0, sizeof(gCmdStr));
					snprintf(gCmdStr, sizeof(gCmdStr),
						 "ubus call hostapd config_remove '{\"iface\":\"wlan0\"}'");
					DPRINT_INFO(WFA_OUT, "run command ==> %s\n", gCmdStr);
					system(gCmdStr);

					memset(gCmdStr, 0, sizeof(gCmdStr));
					snprintf(gCmdStr, sizeof(gCmdStr),
						 "ubus call hostapd config_remove '{\"iface\":\"wlan1\"}'");
					DPRINT_INFO(WFA_OUT, "run command ==> %s\n", gCmdStr);
					system(gCmdStr);
				} else {
					/* radio off the specified radio */
					memset(gCmdStr, 0, sizeof(gCmdStr));
					snprintf(gCmdStr, sizeof(gCmdStr),
						 "ubus call hostapd config_remove '{\"iface\":\"%s\"}'",
						 def_intf->name);
					DPRINT_INFO(WFA_OUT, "run command ==> %s\n", gCmdStr);
					system(gCmdStr);
				}
			} else if (strcasecmp((value_ptr)[i], "on") == 0) {
				memset(gCmdStr, 0, sizeof(gCmdStr));
				snprintf(gCmdStr, sizeof(gCmdStr),
					 "ubus call hostapd config_add '{\"iface\":\"%s\",\"config\":\"%s\"}'",
					 def_intf->name, def_intf->profile_names->profile_cmt);
				DPRINT_INFO(WFA_OUT, "run command ==> %s\n", gCmdStr);
				system(gCmdStr);
			}
		} else if (strcasecmp((data->params)[i], "Reg_Domain") == 0) {
			char Reg_Domain[WFA_NAME_LEN] = "00";

			DPRINT_INFO(WFA_OUT, "param[Reg_Domain] %s\n", (value_ptr)[i]);
			if (strcasecmp((value_ptr)[i], "Global") == 0)
				strncpy(Reg_Domain, "00", sizeof(Reg_Domain) - 1);
			Reg_Domain[sizeof(Reg_Domain) - 1] = '\0';
			memset(gCmdStr, 0, sizeof(gCmdStr));
			snprintf(gCmdStr, sizeof(gCmdStr), "iw reg set %s", Reg_Domain);
			add_post_cmd(mtk_ap_buf);
		} else if (strcasecmp((data->params)[i], "ADDBAReq_BufSize") == 0) {
			DPRINT_INFO(WFA_OUT, "ADDBAReq_BufSize %s\n", (value_ptr)[i]);
			memset(gCmdStr, 0, sizeof(gCmdStr));
			snprintf(gCmdStr, sizeof(gCmdStr), "mt76-vendor wlan0 set ap_wireless add_ba_req_bufsize=%s",
				 (value_ptr)[i]);
			add_post_cmd(mtk_ap_buf);
		} else if (strcasecmp((data->params)[i], "GAS_CB_Delay") == 0) {
			DPRINT_INFO(WFA_OUT, "Noting for GAS_CB_Delay\n");
		} else if (strcasecmp((data->params)[i], "CountryCode") == 0) {
			strncpy(def_intf->country_code, (value_ptr)[i], 2);
			DPRINT_INFO(WFA_OUT, "CountryCode %s\n", def_intf->country_code);
		} else if (strcasecmp((data->params)[i], "FT_OA") == 0 || strcasecmp((data->params)[i], "FT_DS") == 0) {
			strcpy(def_intf->cmd_name, data->name);
			read_mac_address_file(def_intf->mac_addr, mtk_ap_buf->def_intf->name, mtk_ap_buf);
			strip_char(def_intf->mac_addr, ':');
			if (strcasecmp((data->params)[i], "FT_OA") == 0)
				def_intf->ft_oa = 1;
			DPRINT_INFO(WFA_OUT, "NAME: %s, ft_oa: %s, mac_addr: %s\n", data->name, (value_ptr)[i],
				    def_intf->mac_addr);
		} else if (strcasecmp((data->params)[i], "FT_BSS_LIST") == 0) {
			strcpy(def_intf->cmd_name, data->name);
			strcpy(def_intf->ft_bss_list, (value_ptr)[i]);
			DPRINT_INFO(WFA_OUT, "NAME: %s, FT_BSS_LIST: %s\n", data->name, (value_ptr)[i]);
		} else {
			DPRINT_INFO(WFA_OUT, "param[%s] to be done\n", data->params[i]);
		}
	}

	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
	return WFA_SUCCESS;
}

int hostapd_ap_set_radius(int len, uint8_t *ap_buf, int *resp_len_ptr, uint8_t *resp_buf)
{
	mtk_ap_buf_t *mtk_ap_buf = (mtk_ap_buf_t *)ap_buf;
	intf_desc_t *def_intf = NULL;
	intf_desc_t *def_intf2 = NULL;
	capi_data_t *data = mtk_ap_buf->capi_data;
	char **value_ptr;
	int i;
	int wlan_tag_idx = 0;

	DPRINT_INFO(WFA_OUT, "===== running %s function =====\n", __func__);
	if (!ap_buf) {
		DPRINT_INFO(WFA_OUT, "%s(), NULL pointer\n", __func__);
		return WFA_ERROR;
	}

	value_ptr = data->values;
	def_intf = mtk_ap_buf->def_intf;
	if (mtk_ap_buf->intern_flag.capi_dual_pf) {
		if (def_intf->mode == WIFI_2G)
			def_intf2 = &mtk_ap_buf->intf_5G;

		if (def_intf->mode == WIFI_5G)
			def_intf2 = &mtk_ap_buf->intf_2G;

	} else if (mtk_ap_buf->WLAN_TAG > 0) {
		for (wlan_tag_idx = 0; wlan_tag_idx < 3; wlan_tag_idx++) {
			if (wlan_tag_idx == 0)
				def_intf = mtk_ap_buf->WLAN_TAG_inf[wlan_tag_idx];
			else if (wlan_tag_idx == 1)
				def_intf2 = mtk_ap_buf->WLAN_TAG_inf[wlan_tag_idx];
		}
	}

	DPRINT_INFO(WFA_OUT, "-----start looping: %d\n", data->count);
	for (i = 0; i < data->count; i++) {
		if (strcasecmp((data->params)[i], "IPADDR") == 0) {
			DPRINT_INFO(WFA_OUT, "found! radius server IPADDR %s\n", value_ptr[i]);
			strncpy(def_intf->auth_server_addr, value_ptr[i], sizeof(def_intf->auth_server_addr) - 1);
			if (def_intf2)
				strncpy(def_intf2->auth_server_addr, value_ptr[i],
					sizeof(def_intf2->auth_server_addr) - 1);
			if (mtk_ap_buf->intf_6G.status)
				strncpy(mtk_ap_buf->intf_6G.auth_server_addr, value_ptr[i],
					sizeof(mtk_ap_buf->intf_6G.auth_server_addr) - 1);
		} else if (strcasecmp((data->params)[i], "PASSWORD") == 0) {
			DPRINT_INFO(WFA_OUT, "found! radius server PASSWORD %s\n", value_ptr[i]);
			strncpy(def_intf->auth_server_shared_secret, value_ptr[i],
				sizeof(def_intf->auth_server_shared_secret) - 1);
			if (def_intf2)
				strncpy(def_intf2->auth_server_shared_secret, value_ptr[i],
					sizeof(def_intf2->auth_server_shared_secret) - 1);
			if (mtk_ap_buf->intf_6G.status) {
				strncpy(mtk_ap_buf->intf_6G.auth_server_shared_secret, value_ptr[i],
					sizeof(mtk_ap_buf->intf_6G.auth_server_shared_secret) - 1);
			}
		} else if (strcasecmp((data->params)[i], "PORT") == 0) {
			DPRINT_INFO(WFA_OUT, "found! radius server PORT %s\n", value_ptr[i]);
			strncpy(def_intf->auth_server_port, value_ptr[i], sizeof(def_intf->auth_server_port) - 1);
			if (def_intf2)
				strncpy(def_intf2->auth_server_port, value_ptr[i],
					sizeof(def_intf2->auth_server_port) - 1);
			if (mtk_ap_buf->intf_6G.status) {
				strncpy(mtk_ap_buf->intf_6G.auth_server_port, value_ptr[i],
					sizeof(mtk_ap_buf->intf_6G.auth_server_port) - 1);
			}
		}
	}

	return WFA_SUCCESS;
}

int hostapd_ap_set_pmf(int len, uint8_t *ap_buf, int *resp_len_ptr, uint8_t *resp_buf)
{
	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);

	mtk_ap_buf_t *mtk_ap_buf = (mtk_ap_buf_t *)ap_buf;
	capi_data_t *data = mtk_ap_buf->capi_data;
	intf_desc_t *def_intf = mtk_ap_buf->def_intf;
	char **value_ptr = NULL;
	int i = 0;

	value_ptr = data->values;

	for (i = 0; i < data->count; i++) {
		if (strcasecmp((data->params)[i], "PMF") == 0) {
			if (strcasecmp((value_ptr)[i], "Required") == 0)
				def_intf->pmf = WFA_REQUIRED;
			else if (strcasecmp((value_ptr)[i], "Optional") == 0)
				def_intf->pmf = WFA_OPTIONAL;
			else if (strcasecmp((value_ptr)[i], "Disabled") == 0)
				def_intf->pmf = WFA_DISABLED;

			DPRINT_INFO(WFA_OUT, "pmf:%s\n", (value_ptr)[i]);
		}
	}

	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
	return WFA_SUCCESS;
}

int hostapd_ap_set_apqos(int len, uint8_t *ap_buf, int *resp_len_ptr, uint8_t *resp_buf)
{
	mtk_ap_buf_t *mtk_ap_buf = (mtk_ap_buf_t *)ap_buf;
	capi_data_t *data = mtk_ap_buf->capi_data;
	intf_desc_t *def_intf = mtk_ap_buf->def_intf;
	char **value_ptr = NULL;
	hostapd_qos_t *ap_qos = &(def_intf->ap_qos);
	int i = 0;

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);
	value_ptr = data->values;

	for (i = 0; i < data->count; i++) {
		if (strcasecmp((data->params)[i], "cwmin_VO") == 0)
			ap_qos->cwmin_VO = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "cwmin_VI") == 0)
			ap_qos->cwmin_VI = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "cwmin_BE") == 0)
			ap_qos->cwmin_BE = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "cwmin_BK") == 0)
			ap_qos->cwmin_BK = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "cwmax_VO") == 0)
			ap_qos->cwmax_VO = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "cwmax_VI") == 0)
			ap_qos->cwmax_VI = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "cwmax_BE") == 0)
			ap_qos->cwmax_BE = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "cwmax_BK") == 0)
			ap_qos->cwmax_BK = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "AIFS_VO") == 0)
			ap_qos->AIFS_VO = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "AIFS_VI") == 0)
			ap_qos->AIFS_VI = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "AIFS_BE") == 0)
			ap_qos->AIFS_BE = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "AIFS_BK") == 0)
			ap_qos->AIFS_BK = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "TXOP_VO") == 0)
			ap_qos->TXOP_VO = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "TXOP_VI") == 0)
			ap_qos->TXOP_VI = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "TXOP_BE") == 0)
			ap_qos->TXOP_BE = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "TXOP_BK") == 0)
			ap_qos->TXOP_BK = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "ACM_VO") == 0) {
			if (strcasecmp((value_ptr)[i], "off") == 0)
				ap_qos->ACM_VO = 0;
			else
				ap_qos->ACM_VO = (char)atoi((value_ptr)[i]);
		} else if (strcasecmp((data->params)[i], "ACM_VI") == 0) {
			if (strcasecmp((value_ptr)[i], "off") == 0)
				ap_qos->ACM_VI = 0;
			else
				ap_qos->ACM_VI = (char)atoi((value_ptr)[i]);
		} else if (strcasecmp((data->params)[i], "ACM_BE") == 0) {
			if (strcasecmp((value_ptr)[i], "off") == 0)
				ap_qos->ACM_BE = 0;
			else
				ap_qos->ACM_BE = (char)atoi((value_ptr)[i]);
		} else if (strcasecmp((data->params)[i], "ACM_BK") == 0) {
			if (strcasecmp((value_ptr)[i], "off") == 0)
				ap_qos->ACM_BK = 0;
			else
				ap_qos->ACM_BK = (char)atoi((value_ptr)[i]);
		}
	}
	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
	return WFA_SUCCESS;
}

int hostapd_ap_set_staqos(int len, uint8_t *ap_buf, int *resp_len_ptr, uint8_t *resp_buf)
{
	mtk_ap_buf_t *mtk_ap_buf = (mtk_ap_buf_t *)ap_buf;
	capi_data_t *data = mtk_ap_buf->capi_data;
	intf_desc_t *def_intf = mtk_ap_buf->def_intf;
	char **value_ptr = NULL;
	hostapd_qos_t *sta_qos = &(def_intf->sta_qos);
	int i = 0;

	DPRINT_INFO(WFA_OUT, "%s()===enter====>\n", __func__);
	value_ptr = data->values;

	for (i = 0; i < data->count; i++) {
		if (strcasecmp((data->params)[i], "cwmin_VO") == 0)
			sta_qos->cwmin_VO = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "cwmin_VI") == 0)
			sta_qos->cwmin_VI = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "cwmin_BE") == 0)
			sta_qos->cwmin_BE = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "cwmin_BK") == 0)
			sta_qos->cwmin_BK = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "cwmax_VO") == 0)
			sta_qos->cwmax_VO = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "cwmax_VI") == 0)
			sta_qos->cwmax_VI = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "cwmax_BE") == 0)
			sta_qos->cwmax_BE = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "cwmax_BK") == 0)
			sta_qos->cwmax_BK = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "AIFS_VO") == 0)
			sta_qos->AIFS_VO = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "AIFS_VI") == 0)
			sta_qos->AIFS_VI = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "AIFS_BE") == 0)
			sta_qos->AIFS_BE = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "AIFS_BK") == 0)
			sta_qos->AIFS_BK = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "TXOP_VO") == 0)
			sta_qos->TXOP_VO = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "TXOP_VI") == 0)
			sta_qos->TXOP_VI = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "TXOP_BE") == 0)
			sta_qos->TXOP_BE = (char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "TXOP_BK") == 0)
			sta_qos->TXOP_BK = (char)atoi((value_ptr)[i]);
	}
	DPRINT_INFO(WFA_OUT, "%s()<===exit====\n", __func__);
	return WFA_SUCCESS;
}

int hostapd_dev_send_frame(int len, uint8_t *ap_buf, int *resp_len_ptr, uint8_t *resp_buf)
{
	mtk_ap_buf_t *mtk_ap_buf = (mtk_ap_buf_t *)ap_buf;
	intf_desc_t *def_intf = mtk_ap_buf->def_intf;
	capi_data_t *data = mtk_ap_buf->capi_data;
	char **value_ptr;
	char intf[10], FrameName[20], Dest_MAC[20];
	unsigned char RegClass = 0, Channel = 0, BSSID[6];
	unsigned short RandInt = 0, MeaDur = 0;
	char MeaMode = 0, ssid_hex_str[WFA_BUFF_64];
	char RptCond_hex_str[WFA_BUFF_64], RptDet_hex_str[WFA_BUFF_64];
	char APChanRpt_hex_str[WFA_BUFF_64], ReqInfo_hdex_str[WFA_BUFF_64];
	char LastBeaconRptInd_hex_str[WFA_BUFF_64];
	int CandList = 0;
	char cmd_content[WFA_CMD_STR_SZ];
	unsigned int mac_int[6] = {0};
	int i = 0, j = 0;
	int offset = 0, ret_len = 0;

	DPRINT_INFO(WFA_OUT, "===== running %s function =====\n", __func__);

	memset(RptCond_hex_str, 0, sizeof(RptCond_hex_str));
	memset(RptDet_hex_str, 0, sizeof(RptDet_hex_str));
	memset(APChanRpt_hex_str, 0, sizeof(APChanRpt_hex_str));
	memset(ReqInfo_hdex_str, 0, sizeof(ReqInfo_hdex_str));
	memset(LastBeaconRptInd_hex_str, 0, sizeof(LastBeaconRptInd_hex_str));
	memset(ssid_hex_str, 0, sizeof(ssid_hex_str));
	memset(cmd_content, 0, sizeof(cmd_content));
	memset(BSSID, 0, sizeof(BSSID));
	memset(mac_int, 0, sizeof(mac_int));
	snprintf(intf, sizeof(intf) - 1, mtk_ap_buf->def_intf->name);
	value_ptr = data->values;
	if ((strcasecmp(data->interface, "5G") == 0) || (strcasecmp(data->interface, "5.0") == 0)) {
		if (mtk_ap_buf->intf_5G.status)
			snprintf(intf, sizeof(intf) - 1, mtk_ap_buf->intf_5G.name);
		else
			DPRINT_INFO(WFA_OUT, "5G interface is not supported, skip!\n");
	} else if ((strcasecmp(data->interface, "2G") == 0) || (strcasecmp(data->interface, "24G") == 0) ||
		   (strcasecmp(data->interface, "2.4") == 0)) {
		if (mtk_ap_buf->intf_2G.status)
			snprintf(intf, sizeof(intf) - 1, mtk_ap_buf->intf_2G.name);
		else
			DPRINT_INFO(WFA_OUT, "2G interface is not supported, skip!\n");
	} else if (strcasecmp(data->interface, "6G") == 0) {
		if (mtk_ap_buf->intf_6G.status)
			snprintf(intf, sizeof(intf) - 1, mtk_ap_buf->intf_6G.name);
		else
			DPRINT_INFO(WFA_OUT, "6G interface is not supported, skip!\n");
	}

	for (i = 0; i < data->count; i++) {
		if (strcasecmp((data->params)[i], "FrameName") == 0)
			snprintf(FrameName, sizeof(FrameName) - 1, (value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "Dest_MAC") == 0)
			snprintf(Dest_MAC, sizeof(Dest_MAC) - 1, (value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "DestMAC") == 0)
			snprintf(Dest_MAC, sizeof(Dest_MAC) - 1, (value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "Cand_List") == 0)
			CandList = 1;
		else if (strcasecmp((data->params)[i], "RegClass") == 0) {
			RegClass = (unsigned char)atoi((value_ptr)[i]);
			def_intf->frame_opclass = RegClass;
		} else if (strcasecmp((data->params)[i], "Channel") == 0)
			Channel = (unsigned char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "OCIChannel") == 0)
			Channel = (unsigned char)atoi((value_ptr)[i]);
		else if (strcasecmp((data->params)[i], "RandInt") == 0) {
			RandInt = (unsigned short)atoi((value_ptr)[i]);
			RandInt = ((RandInt & 0xff) << 8) | (RandInt >> 8);
		} else if (strcasecmp((data->params)[i], "MeaDur") == 0) {
			MeaDur = (unsigned short)atoi((value_ptr)[i]);
			MeaDur = ((MeaDur & 0xff) << 8) | (MeaDur >> 8);
		} else if (strcasecmp((data->params)[i], "MeaMode") == 0) {
			if (strcasecmp((value_ptr)[i], "PASSIVE") == 0)
				MeaMode = 0;
			else if (strcasecmp((value_ptr)[i], "ACTIVE") == 0)
				MeaMode = 1;
			else if (strcasecmp((value_ptr)[i], "TABLE") == 0)
				MeaMode = 2;
		} else if (strcasecmp((data->params)[i], "BSSID") == 0) {
			ret_len = sscanf((value_ptr)[i], "%02x:%02x:%02x:%02x:%02x:%02x", mac_int, (mac_int + 1),
					 (mac_int + 2), (mac_int + 3), (mac_int + 4), (mac_int + 5));
			if (ret_len != 6)
				DPRINT_INFO(WFA_OUT, "sscanf of BSSID not match\n");

			for (j = 0; j < 6; j++)
				BSSID[j] = (unsigned char)mac_int[j];
			DPRINT_INFO(WFA_OUT, "BSSID " MACSTR "\n", MAC2STR(BSSID));
		} else if (strcasecmp((data->params)[i], "SSID") == 0) {
			ret_len = hostapd_parameter_to_tlv(ssid_hex_str, sizeof(ssid_hex_str), (value_ptr)[i],
							   strlen((value_ptr)[i]), TLV_TYPE_SSID, def_intf);
			if (ret_len >= sizeof(ssid_hex_str)) {
				DPRINT_INFO(WFA_OUT, "SSID is too long!\n");
			}
		} else if (strcasecmp((data->params)[i], "RptCond") == 0) {
			ret_len = hostapd_parameter_to_tlv(RptCond_hex_str, sizeof(RptCond_hex_str), (value_ptr)[i],
							   strlen((value_ptr)[i]), TLV_TYPE_BECON_REPT_INF, def_intf);
			if (ret_len >= sizeof(RptCond_hex_str)) {
				DPRINT_INFO(WFA_OUT, "RptCond is too long!\n");
			}
		} else if (strcasecmp((data->params)[i], "RptDet") == 0) {
			ret_len = hostapd_parameter_to_tlv(RptDet_hex_str, sizeof(RptDet_hex_str), (value_ptr)[i],
							   strlen((value_ptr)[i]), TLV_TYPE_BECON_REPT_DET, def_intf);
			if (ret_len >= sizeof(RptDet_hex_str)) {
				DPRINT_INFO(WFA_OUT, "RptDet is too long!\n");
			}
		} else if (strcasecmp((data->params)[i], "MeaDurMand") == 0) {
			DPRINT_INFO(WFA_OUT, "MeaDurMand %s\n", (value_ptr)[i]);
		} else if (strcasecmp((data->params)[i], "APChanRpt") == 0) {
			ret_len = hostapd_parameter_to_tlv(APChanRpt_hex_str, sizeof(APChanRpt_hex_str), (value_ptr)[i],
							   strlen((value_ptr)[i]), TLV_TYPE_AP_CHAN_RPT, def_intf);
			if (ret_len >= sizeof(APChanRpt_hex_str))
				DPRINT_INFO(WFA_OUT, "APChanRpt is too long!\n");
		} else if (strcasecmp((data->params)[i], "LastBeaconRptIndication") == 0) {
			ret_len = hostapd_parameter_to_tlv(LastBeaconRptInd_hex_str, sizeof(LastBeaconRptInd_hex_str),
							   (value_ptr)[i], strlen((value_ptr)[i]),
							   TLV_TYPE_BECON_REPT_IND_REQ, def_intf);
			if (ret_len >= sizeof(LastBeaconRptInd_hex_str)) {
				DPRINT_INFO(WFA_OUT, "LastBeaconRptIndication is too long!\n");
			}
		} else if (strcasecmp((data->params)[i], "ReqInfo") == 0) {
			ret_len = hostapd_parameter_to_tlv(ReqInfo_hdex_str, sizeof(ReqInfo_hdex_str), (value_ptr)[i],
							   strlen((value_ptr)[i]), TLV_TYPE_REQ_INFO, def_intf);
			if (ret_len >= sizeof(ReqInfo_hdex_str)) {
				DPRINT_INFO(WFA_OUT, "ReqInfo is too long!\n");
			}
		} else if (strcasecmp((data->params)[i], "type") == 0)
			continue;
		else {
			DPRINT_INFO(WFA_OUT, "dev_send_frame %s  Command is ignored or invalid!\n", data->params[i]);
		}
	}

	if (strcasecmp(FrameName, "BTMReq") == 0) {
		offset = 0;
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "bss_tm_req");
		/* dst mac */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, " %s ", Dest_MAC);
		/* disassoc_timer */
		if (def_intf->BTMReq_Term_Bit)
			offset +=
			    snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "disassoc_timer=50 ");
		else if (def_intf->BTMReq_DisAssoc_Imnt)
			offset +=
			    snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "disassoc_timer=10 ");
		else
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "disassoc_timer=0 ");

		/* valid_int */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "valid_int=200 ");
		/* bss_term */
		if (def_intf->BTMReq_Term_Bit)
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "bss_term=1, %x ",
					   def_intf->BSS_Term_Duration);

		/* pref */
		if (CandList)
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "pref=1 ");

		/* disassoc_imminent */
		if (def_intf->BTMReq_Term_Bit || def_intf->BTMReq_DisAssoc_Imnt)
			offset +=
			    snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "disassoc_imminent=1 ");

		/* mbo */
		if (def_intf->BTMReq_DisAssoc_Imnt)
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "mbo=0:10:1 ");
		else
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "mbo=0:0:1 ");

		hostapd_cli_cmd(def_intf, cmd_content);

		def_intf->BTMReq_Term_Bit = 0;
		def_intf->BSS_Term_Duration = 0;
		def_intf->BTMReq_DisAssoc_Imnt = 0;
	} else if (strcasecmp(FrameName, "BcnRptReq") == 0) {
		offset = 0;
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "req_beacon");
		/* dst mac */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, " %s ", Dest_MAC);
		/* req mode */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "req_mode=00 ");
		/* RegClass */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%02x", RegClass);
		/* Channel */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%02x", Channel);
		/* RandInt */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%04x", RandInt);
		/* MeaDur */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%04x", MeaDur);
		/* MeaMode */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%02x", MeaMode);
		/* BSSID */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%02x%02x%02x%02x%02x%02x",
				   BSSID[0], BSSID[1], BSSID[2], BSSID[3], BSSID[4], BSSID[5]);
		/* hex ssid */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%s", ssid_hex_str);
		/* RptCond */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%s", RptCond_hex_str);
		/* RptDet */
		offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%s", RptDet_hex_str);
		/* APChanRpt */
		if (strlen(APChanRpt_hex_str)) {
			offset +=
			    snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%s", APChanRpt_hex_str);
		}
		/* ReqInfo */
		if (strlen(ReqInfo_hdex_str)) {
			offset +=
			    snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%s", ReqInfo_hdex_str);
		}
		/* LastBeaconRptIndication */
		if (strlen(LastBeaconRptInd_hex_str))
			offset += snprintf(cmd_content + offset, sizeof(cmd_content) - offset - 1, "%s",
					   LastBeaconRptInd_hex_str);
		hostapd_cli_cmd(def_intf, cmd_content);
	}
	return WFA_SUCCESS;
}
