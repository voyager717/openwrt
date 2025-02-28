/*
 * hostapd / DPP integration
 * Copyright (c) 2017, Qualcomm Atheros, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See BSD_LICENSE for more details.
 */

/* This is a merged file for hostapd and wpa_supplicant dpp state mahine code
 * wapp and wdev are introduced and state machines are changed for handling
 * for AP, STA, enrollee, configurator role.
 */
#include "os.h"
#include "util.h"
#include "eloop.h"
#include "wapp_cmm.h"
#include "dpp.h"
#include "gas.h"
#include "dpp_wdev.h"
#include "gas_server.h"
#include "utils/ip_addr.h"
#include "gas_query.h"
#include "utils/wpabuf.h"
#include "utils/base64.h"
#include "utils/wpa_debug.h"
#include "map.h"
#ifdef MAP_R3
#include "crypto/crypto.h"
#include "wps.h"
#endif
#ifdef OPENWRT_SUPPORT
#include <libdatconf.h>
#endif

#ifdef DPP_R2_RECONFIG
#include "random.h"
#endif /* DPP_R2_RECONFIG */
#define max_buf_len 2048
#ifdef OPENWRT_SUPPORT
#ifdef DPP_R2_MUOBJ
static void dpp_fetch_dpp_akm_param(struct wapp_dev *wdev, struct dpp_authentication *auth, 
	struct dpp_config_obj *conf, struct kvc_context *dat_ctx);
static void dpp_fetch_psk_akm_param(struct wapp_dev *wdev, struct dpp_config_obj *conf, struct kvc_context *dat_ctx);

#else
static void dpp_fetch_dpp_akm_param(struct wapp_dev *wdev, struct dpp_authentication *auth,
				    struct kvc_context *dat_ctx);
static void dpp_fetch_psk_akm_param(struct wapp_dev *wdev, struct dpp_authentication *auth,
				    struct kvc_context *dat_ctx);
#endif /*DPP_R2_MUOBJ*/
#endif
static void wapp_dpp_reply_wait_timeout(void *eloop_ctx, void *timeout_ctx);
static void wapp_dpp_init_timeout(void *eloop_ctx, void *timeout_ctx);
static int wapp_dpp_auth_init_next(struct wifi_app *wapp, struct dpp_authentication *auth);

static const u8 broadcast[MAC_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
#ifdef DPP_R2_MUOBJ
static int wapp_dpp_process_config(struct wifi_app *wapp,
				    struct dpp_authentication *auth, struct dpp_config_obj *conf);
#else
static int wapp_dpp_process_config(struct wifi_app *wapp,
				    struct dpp_authentication *auth);
#endif /*DPP_R2_MUOBJ*/

void
wapp_dpp_tx_pkex_status(struct wifi_app *wapp,
			unsigned int chan, const u8 *dst,
			const u8 *src, const u8 *bssid,
			const u8 *data, size_t data_len,
			int ok);
static void wapp_dpp_auth_resp_retry_timeout(void *eloop_ctx, void *timeout_ctx);
/*void wapp_dpp_auth_conf_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx);*/
static void wapp_dpp_auth_resp_retry(struct wifi_app *wapp, struct dpp_authentication *auth);

/* Use a hardcoded Transaction ID 1 in Peer Discovery frames since there is only
 * a single transaction in progress at any point in time. */
static const u8 TRANSACTION_ID = 1;
#ifdef MAP_R3
u8 obj_count = 0;
#endif /* MAP_R3 */

void dpp_auth_list_deinit(struct wifi_app *wapp);

struct wapp_dev *wapp_get_dpp_default_wdev(struct wifi_app *wapp, unsigned int chan)
{
	struct wapp_dev *wdev = NULL;

	if (IS_MAP_CH_5GH(chan))
		wdev = wapp->dpp->default_5gh_iface;
	else if (IS_MAP_CH_5GL(chan))
		wdev = wapp->dpp->default_5gl_iface;	
	else if (IS_MAP_CH_24G(chan))
		wdev = wapp->dpp->default_2g_iface;				

	return wdev;
}

u8 wapp_op_band_frm_ch(struct wifi_app *wapp, unsigned int chan)
{
	u8 res=0;

	if (IS_MAP_CH_5GH(chan))
		res = RADIO_5GH;
	else if (IS_MAP_CH_5GL(chan))
		res = RADIO_5GL;	
	else if (IS_MAP_CH_24G(chan))
		res = RADIO_24G;				
	if (wapp->radio_count == 2 && IS_MAP_CH_5G(chan))
		res = RADIO_5G;
	return res;
}

unsigned int get_chan_from_ch_list_str(char * chan_list_str, unsigned int chan[])
{
	const char *pos = chan_list_str;
	unsigned int opclass, channel;
	unsigned int count = 0;

	while (chan_list_str && *chan_list_str)
	{
		opclass = atoi(pos);
		if (opclass <= 0) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed_reading_opclass\n");
			continue;
		}

		pos = os_strchr(pos, '/');
		if (!pos) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed_reading_identifier\n");
			continue;
		}

		pos++;
		channel = atoi(pos);
		if (channel <= 0) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed_reading_channel\n");
			continue;
		}

		chan[count] = channel;
		count++;

		while (*pos >= '0' && *pos <= '9')
			pos++;

		if(*pos == '\0')
			break;

		if(*pos!=',')
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"error in str");
		pos++;
	}

	return count;
}

/* dpp auth helper functions */
struct dpp_authentication *wapp_dpp_get_first_auth(struct wifi_app *wapp)
{
	struct dpp_authentication *auth = dl_list_first(&wapp->dpp->dpp_auth_list, struct dpp_authentication,
							list);

	return auth;
}

struct dpp_authentication *wapp_dpp_get_last_auth(struct wifi_app *wapp)
{
	struct dpp_authentication *auth, *auth_tmp, *auth_tmp2 = NULL;

	dl_list_for_each_safe(auth, auth_tmp, &(wapp->dpp->dpp_auth_list),
			struct dpp_authentication, list) {
		auth_tmp2 = auth;
	}

	if (!auth_tmp2) {
		return NULL;
	}
	auth = auth_tmp2;
	return auth;
}

struct dpp_authentication *wapp_dpp_get_auth_from_peer_mac(struct wifi_app *wapp, u8 *addr)
{
	struct dpp_authentication *auth;

	dl_list_for_each(auth, &wapp->dpp->dpp_auth_list, struct dpp_authentication, list) {
		if (os_memcmp(auth->peer_mac_addr, addr, ETH_ALEN) == 0)
			return auth;
	}

	return NULL;
}

#ifdef MAP_R3
struct dpp_authentication *wapp_dpp_get_auth_from_hash_val(struct wifi_app *wapp, u8 *hash_val)
{
	struct dpp_authentication *auth;

	dl_list_for_each(auth, &wapp->dpp->dpp_auth_list, struct dpp_authentication, list) {
		if (os_memcmp(auth->chirp_tlv.hash_payload, hash_val, SHA256_MAC_LEN) == 0)
			return auth;
	}

	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX
		"DPP: Auth not found Expected Chirp Bootstrapping Key Hash",
		hash_val, SHA256_MAC_LEN);
	return NULL;
}

struct dpp_authentication *wapp_dpp_get_auth_from_resp_hash_val(struct wifi_app *wapp, u8 *hash_val)
{
	struct dpp_authentication *auth;

	dl_list_for_each(auth, &wapp->dpp->dpp_auth_list, struct dpp_authentication, list) {
		if (auth->peer_bi) {
			if (os_memcmp(auth->peer_bi->pubkey_hash, hash_val, SHA256_MAC_LEN) == 0)
				return auth;
		} else {
			 DBGPRINT(RT_DEBUG_WARN, "Auth does not have peer bi value\n");
		}
	}

	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX
		"DPP: Auth not found Expected Responder Bootstrapping Key Hash",
		hash_val, SHA256_MAC_LEN);
	return NULL;
}

struct dpp_authentication *wapp_dpp_get_own_auth_cont(struct wifi_app *wapp)
{
	struct dpp_authentication *auth;

	dl_list_for_each(auth, &wapp->dpp->dpp_auth_list, struct dpp_authentication, list) {
		if (auth->self_sign_auth)
			return auth;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		"DPP: Auth not found for self sign");
	return NULL;
}

struct dpp_agent_info *wapp_dpp_get_agent_list_from_chirp_hash_val(struct wifi_app *wapp, u8 *hash_val)
{
	struct dpp_agent_info *agnt_info;

	dl_list_for_each(agnt_info, &wapp->dpp->dpp_agent_list, struct dpp_agent_info, list) {
		if (os_memcmp(agnt_info->chirp_hash, hash_val, SHA256_MAC_LEN) == 0)
			return agnt_info;
	}

	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX
		"DPP: Expected Peer Chirp Bootstrapping Key Hash",
		hash_val, SHA256_MAC_LEN);
	return NULL;
}

struct dpp_agent_info *wapp_dpp_get_agent_list_from_agnt_mac_addr(struct wifi_app *wapp, u8 *addr)
{
	struct dpp_agent_info *agnt_info;

	dl_list_for_each(agnt_info, &wapp->dpp->dpp_agent_list, struct dpp_agent_info, list) {
		if (os_memcmp(agnt_info->agnt_mac_addr, addr, ETH_ALEN) == 0)
			return agnt_info;
	}

	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX
		"DPP: Expected Peer MAC Address",
		addr, ETH_ALEN);
	return NULL;
}


struct dpp_agent_info *wapp_dpp_get_agent_list_from_resp_hash_val(struct wifi_app *wapp, u8 *hash_val)
{
	struct dpp_agent_info *agnt_info;

	dl_list_for_each(agnt_info, &wapp->dpp->dpp_agent_list, struct dpp_agent_info, list) {
		if (os_memcmp(agnt_info->resp_pubkey_hash, hash_val, SHA256_MAC_LEN) == 0)
			return agnt_info;
	}

	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX
		"DPP: Expected Responder Bootstrapping Key Hash",
		hash_val, SHA256_MAC_LEN);
	return NULL;
}


int wapp_dpp_reset_cce_ie(struct wifi_app *wapp)
{
	int ret = 0;

	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list;
	struct ap_dev *ap = NULL;

    if (!wapp->drv_ops || !wapp->drv_ops->drv_reset_cce_ie)
            return -1;

	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
			ap = (struct ap_dev *)wdev->p_dev;
			if (ap->bss_info.map_role & BIT_FH_BSS)
				ret = wapp->drv_ops->drv_reset_cce_ie(wapp->drv_data, wdev->ifname);
			if(ret == -1)
				return ret;
		}
	}
	return ret;
}

void wapp_dpp_eth_resp_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx)
{
	//struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = timeout_ctx;

	if(auth->current_state == DPP_STATE_AUTH_RESP_WAITING)
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			"DPP: Timeout while waiting for Direct auth response");
		else
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			"DPP: Timeout while waiting for Direct config response");

	dpp_auth_deinit(auth);
	return;
}

void wapp_dpp_agent_list_timeout(void *eloop_ctx,
						void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_agent_info *agnt_info = timeout_ctx;

	if(!wapp || !agnt_info) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"invalid wapp or agent list instance, returning");
		return;
	}

	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"agent list timeout for list for"MACSTR "\n",
							MAC2STR(agnt_info->agnt_mac_addr));
	/* reset the agent list state in case of timeout */
	agnt_info->agent_state = DPP_AGT_STATE_INIT;
	return;
}

void wapp_dpp_default_iface(struct wifi_app *wapp)
{
	struct wapp_dev * wdev=  NULL;
	struct dl_list *dev_list;

	if(!wapp) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"invalid wapp instance, returning");
		return;
	}
	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
		if (wdev->radio
				&& wdev->dev_type == WAPP_DEV_TYPE_STA) {

			if(IS_MAP_CH_24G(wdev->radio->op_ch)) {
				wapp->dpp->default_2g_iface = wdev;
				DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"default 2G iface defined:%s\n",wdev->ifname);
			}

			if (wapp->radio_count == MAX_RADIO_DBDC
#ifdef MAP_R3_RECONFIG
				&& !wapp->dpp->radar_detect_ind
#endif /* MAP_R3_RECONFIG */
			) {
				if(IS_MAP_CH_5G(wdev->radio->op_ch)) {
					wapp->dpp->default_5gl_iface = wdev;
					DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"default 5G iface defined:%s\n",wdev->ifname);
				}
			} else if (wapp->radio_count == MAP_MAX_RADIO
#ifdef MAP_R3_RECONFIG
				&& !wapp->dpp->radar_detect_ind
#endif /* MAP_R3_RECONFIG */
			) {
				if(IS_MAP_CH_5GL(wdev->radio->op_ch)) {
					wapp->dpp->default_5gl_iface = wdev;
					DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"default 5GL iface defined:%s\n",wdev->ifname);
				}

				if(IS_MAP_CH_5GH(wdev->radio->op_ch)) {
					wapp->dpp->default_5gh_iface = wdev;
					DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"default 5GH iface defined:%s\n",wdev->ifname);
				}
			}
		}
	}
}

int wapp_dpp_agnt_uri(struct wifi_app *wapp, char *uri)
{
	int ret = 0;
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list;

	if (!wapp->drv_ops || !wapp->drv_ops->drv_agnt_dpp_uri)
		return -1;

	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
		if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
			ret = wapp->drv_ops->drv_agnt_dpp_uri(wapp->drv_data, wdev->ifname, (char *)uri);
				if (ret == -1) {
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed for wdev:%s continue loop\n",wdev->ifname);
					continue;
				}
			}
		}

	return ret;
}

int wapp_dpp_set_1905_sec(struct wifi_app *wapp, u8 flag)
{
	int ret = 0;
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list;

	if (!wapp->drv_ops || !wapp->drv_ops->drv_set_1905_sec)
		return -1;

	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
		if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
			ret = wapp->drv_ops->drv_set_1905_sec(wapp->drv_data, wdev->ifname, (const u8)flag);
			if (ret == -1) {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed for wdev:%s continue loop\n", wdev->ifname);
				continue;
			}
		}
	}
	return ret;
}

int wapp_dpp_get_conf_frm_file(struct wifi_app *wapp, const char *str)
{
	int ret = 0;
	char param[64], value[64];
	ret = os_snprintf(param, sizeof(param), "_%s", str);
	if (os_snprintf_error(sizeof(param), ret))
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s %d os_snprintf error\n", __func__, __LINE__);

	ret = 0;
	get_dpp_parameters(wapp->map, param, value, sizeof(value));
	if(os_strcmp(value,"1") == 0) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"found config %s\n", str);
		return 1;
	}

	return ret;
}

void wapp_dpp_get_uri_frm_file(struct wifi_app *wapp)
{
	char qr_param[20] = "0";
	char value[150] = "0";
	char param_str[15] = "agt_qr_code";
	char uri_num = 1;
	int uri_cnt = 0;
	int ret;

	/* Get QR code save conunt for */
	get_dpp_parameters(wapp->map, "qr_count", value, sizeof(value));
	uri_cnt = atoi(value);
	if(uri_cnt != 0) {
		/* Resetting the flag so the same QR code is not saved */
		wapp->dpp->qr_cmd = 0;
		for(uri_num = 1; uri_num <= uri_cnt; uri_num++) {
			os_memset(qr_param, 0, 20);
			os_memset(value, 0, 150);
			ret = os_snprintf(qr_param, sizeof(qr_param), "%s_%u", param_str, uri_num);
			if (os_snprintf_error(sizeof(qr_param), ret)) {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s %d os_snprintf error\n", __func__, __LINE__);
				continue;
			}

			get_dpp_parameters(wapp->map, qr_param, value, sizeof(value));
			if(!is_str_null(value))
				wapp_dpp_qr_code(wapp, (const char *)value);
		}
		wapp->dpp->qr_cmd = 1;
	}
}

int wapp_dpp_set_onboarding_type(struct wifi_app *wapp, u8 flag)
{
	int ret = 0;
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list;

	if (!wapp->drv_ops || !wapp->drv_ops->drv_set_onboarding_type)
		return -1;

	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
			ret = wapp->drv_ops->drv_set_onboarding_type(wapp->drv_data, wdev->ifname, (const u8)flag);
			if (ret == -1) {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed for wdev:%s continue loop\n", wdev->ifname);
				continue;
			}
		}
	}

	return ret;
}

void wapp_dpp_init_notify_handler(struct wifi_app *wapp)
{
	char onb_type = 0;

	if(!wapp->dpp) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX "DPP not init yet\n");
		return;
	}

	if(!wapp->map) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX "map not init yet\n");
		return;
	}

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
		int ret;
		bi = dpp_get_own_bi(wapp->dpp);
		if(!bi) {
			DBGPRINT(RT_DEBUG_OFF, "Bootstrap not found on bootup generating key\n");
			ret = os_snprintf(mode_str, sizeof(mode_str), "type=qrcode");

			if (os_snprintf_error(sizeof(mode_str), ret))
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s %d os_snprintf error\n", __func__, __LINE__);

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
			ret = os_snprintf(mode_str, sizeof(mode_str), " mode=map");

			if (os_snprintf_error(sizeof(mode_str), ret))
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s %d os_snprintf error\n", __func__, __LINE__);

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
	return;
}
#endif /* MAP_R3 */

struct dpp_authentication *wapp_get_next_auth(struct wifi_app *wapp, struct dpp_authentication *auth)
{
	struct dpp_authentication *auth1 = (struct dpp_authentication *)((&(auth->list))->next);
	return auth1;
	//return dl_list_entry(&(auth->list)->next, struct dpp_authentication, list);
}

int wapp_dpp_auth_list_insert(struct wifi_app *wapp, struct dpp_authentication *auth)
{
	struct dpp_authentication *auth_iter;
	dl_list_for_each(auth_iter, &wapp->dpp->dpp_auth_list, struct dpp_authentication, list) {
		if (os_memcmp(auth->peer_mac_addr, auth_iter->peer_mac_addr, ETH_ALEN) == 0) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"another auth exchange from same mac address\n");
			dl_list_add_tail(&wapp->dpp->dpp_auth_list, &auth->list);
			return -1;
		}
	}
	dl_list_add_tail(&wapp->dpp->dpp_auth_list, &auth->list);

	if (auth->wdev)
		auth->wdev->radio->ongoing_dpp_cnt++;
	return 0;
}

void wapp_dpp_auth_list_remove(struct dpp_authentication *auth)
{
	if (!auth)
		return;
	if ((auth->list.next == NULL && auth->list.prev == NULL))
		return;
	if (!(auth->list.next == NULL && auth->list.prev == NULL))
		dl_list_del(&auth->list);
	if (auth->wdev)
		auth->wdev->radio->ongoing_dpp_cnt--;
}

void wapp_dpp_auth_list_remove_by_mac(struct wifi_app *wapp, char *addr)
{
	struct dpp_authentication *auth;

	dl_list_for_each(auth, &wapp->dpp->dpp_auth_list, struct dpp_authentication, list) {
		if (os_memcmp(auth->peer_mac_addr, addr, ETH_ALEN) == 0)
			return dl_list_del(&auth->list);
	}
}

struct dpp_tx_status *wapp_dpp_get_status_info_from_sq(struct wifi_app *wapp, u16 seq_no)
{
	struct dpp_tx_status *tx_status;

	dl_list_for_each(tx_status, &wapp->dpp->dpp_txstatus_pending_list, struct dpp_tx_status, list) {
		if (seq_no == tx_status->seq_no)
			return tx_status;
	}

	return NULL;
}

void wapp_dpp_txstatus_list_insert(struct wifi_app *wapp, struct dpp_tx_status *tx_status)
{
	dl_list_add_tail(&wapp->dpp->dpp_txstatus_pending_list, &tx_status->list);
}

void wapp_dpp_tx_status_list_remove(struct dpp_tx_status *tx_status)
{
	dl_list_del(&tx_status->list);
}

int wapp_get_band_from_chan(unsigned int chan)
{
	// TODO kapil check for triband
	if (chan <= 14)
		return BAND_24G;
	else
		return BAND_5G;
}

int wapp_set_pmk(struct wifi_app *wapp, const u8 *addr,
                        const u8 *pmk, size_t pmk_len, const u8 *pmkid,
                        int session_timeout, int akmp, struct wapp_dev *wdev)
{
	struct dpp_config *wdev_config = NULL;

	if (!wdev) {
		return -1;
	}
        if (!wapp->drv_ops || !wapp->drv_ops->drv_set_pmk)
                return 0;

	wdev_config = wdev->config;
	if (!wdev_config) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"\n wdev_config not set");
		return 0;	
	}

        return wapp->drv_ops->drv_set_pmk(wapp->drv_data, wdev->ifname, pmk, pmk_len, pmkid,
                                 wdev->mac_addr, addr, session_timeout, akmp, wdev_config->ssid, wdev_config->ssid_len);
}

int wapp_insert_dpp_tx_status_list(struct wifi_app *wapp, struct wapp_dev *wdev,
					const u8 *dst, const u8 *data)
{
	struct dpp_tx_status *status;
	/* go to type */
	data++;

	/* Insert into auth status list if da is not broadcast */
	if (os_memcmp(dst, broadcast, MAC_ADDR_LEN) == 0) {
		return 0;
	}
	wapp->dpp->dpp_frame_seq_no++;
	if(wapp->dpp->dpp_frame_seq_no > 0xfff)
		wapp->dpp->dpp_frame_seq_no = 1011; //Reset Sequence Number

	status = os_zalloc(sizeof(*status));

	os_memcpy(status->dst, dst, MAC_ADDR_LEN);
	status->seq_no = wapp->dpp->dpp_frame_seq_no;
	status->wdev = wdev;
	os_get_time(&status->sent_time);

	if (*data == 0xa || *data == 0xb)
		status->is_gas_frame = 1;

	/* TODO add a timer to delete it if we don't get the result, or use periodic */
	wapp_dpp_txstatus_list_insert(wapp, status);

	return 0;
}

int wapp_drv_send_action(struct wifi_app *wapp, struct wapp_dev *wdev, unsigned int chan,
		unsigned int wait, const u8 *dst, const u8 *data,
		size_t len)
{
	const u8 *bssid;

	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"wdev is null\n");
		return -1;
	}

	if (chan == 0) {
		chan = wdev->radio->op_ch;
	}
	if (!wapp->drv_ops || !wapp->drv_ops->send_action)
		return 0;

	/* All the DPP frames are sent in non connected state
	 * Set BSSID as always wildcard */
	bssid = broadcast;

	/* Add this for tx status list */
	wapp_insert_dpp_tx_status_list(wapp, wdev, dst, data);

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			"DPP: Sending out dpp frame to "MACSTR " seq_no =%u, chan=%u\n",
			MAC2STR(dst), wapp->dpp->dpp_frame_seq_no, chan);
	return wapp->drv_ops->send_action(OID_SEND_OFFCHAN_ACTION_FRAME, wapp->drv_data, wdev->ifname, chan, wait, dst,
			wdev->mac_addr, bssid, data, len, wapp->dpp->dpp_frame_seq_no, 0);
}

int wapp_cancel_remain_on_channel(
        struct wifi_app *wapp, struct wapp_dev *wdev)
{

	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"no send action waiting\n");
		return -1;
	}

	if (wapp->drv_ops->drv_cancel_roc)
	        return wapp->drv_ops->drv_cancel_roc(wapp->drv_data, wdev->ifname);

	return -1;
}
#ifdef MAP_R3
void wapp_version_mismatch(struct wifi_app *wapp) {
	user_fail_reason *info_to_mapd = os_zalloc(sizeof(user_fail_reason));
	if (info_to_mapd == NULL)
		return;
	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX" [%s] \n", __func__);
	info_to_mapd->reason_id = VERSION_MISMATCH;
	os_memcpy(info_to_mapd->reason, "Version Different", os_strlen("Version Different"));
	wapp_send_1905_msg(
			wapp,
			WAPP_SEND_USER_FAIL_NOTIF,
			sizeof(user_fail_reason),
			(char *)info_to_mapd);
	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" %s [%s] \n", info_to_mapd->reason, __func__);
	os_free(info_to_mapd);
	return;
}
#endif
/**
 * wapp_dpp_qr_code - Parse and add DPP bootstrapping info from a QR Code
 * @wapp: Pointer to wifi_app
 * @cmd: DPP URI read from a QR Code
 * Returns: Identifier of the stored info or -1 on failure
 */
int wapp_dpp_qr_code(struct wifi_app *wapp, const char *cmd)
{
	struct dpp_bootstrap_info *bi;
	struct dpp_authentication *auth;
#ifdef MAP_R3
	struct map_info *map = NULL;
/* When version check will be present use this*/
	if (wapp->map && wapp->map->map_version != DEV_TYPE_R3) {
		wapp_version_mismatch(wapp);
		return -1;
	}

	if(!wapp->dpp) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"[%s] DPP is not initalized\n", __func__);
		return -1;
	}

	if (wapp->dpp->dpp_allowed_roles != DPP_CAPAB_CONFIGURATOR) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"[%s] Device is not controller\n", __func__);
		return -1;
	}
#endif
	bi = dpp_add_qr_code(wapp->dpp, cmd);
	if (!bi) {
#ifdef MAP_R3
		user_fail_reason *info_to_mapd = os_zalloc(sizeof(user_fail_reason));
		if (info_to_mapd == NULL)
			return -1;
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX" [%s] \n", __func__);
		info_to_mapd->reason_id = DPP_QR_PARSING_FAIL;
		os_memcpy(info_to_mapd->reason, "DPP URI Parsing Fail", os_strlen("DPP URI Parsing Fail"));
		wapp_send_1905_msg(
				wapp,
				WAPP_SEND_USER_FAIL_NOTIF,
				sizeof(user_fail_reason),
				(char *)info_to_mapd);
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" %s [%s] \n", info_to_mapd->reason, __func__);
		os_free(info_to_mapd);
#endif
		return -1;
	}

#ifdef MAP_R3
	if (wapp->dpp->is_map && (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_CONFIGURATOR)
		&& !wapp->dpp->onboarding_type) {
		ChirpTLV_1905_send(wapp, bi);
		/* Send CCE enable indication message to R3 onboarded agents on reception of URI */
		CCEIndication_1905_send(wapp, NULL, 1);
		if((wapp->map->TurnKeyEnable == 1) && wapp->dpp->qr_cmd) {
			/* Save DPP URI in CFG file for reboot purpose */
			dpp_save_dpp_uri_to_file(wapp, bi->uri);
		}
	}

	map = wapp->map;
	if (!map) {
#endif
		auth = wapp_dpp_get_first_auth(wapp);

	while (auth) {
		if (auth && auth->response_pending &&
				dpp_notify_new_qr_code(auth, bi) == 1) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
					"DPP: Sending out pending authentication response");
			wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR
				" chan=%u type=%d", MAC2STR(auth->peer_mac_addr), auth->curr_chan,
				DPP_PA_AUTHENTICATION_RESP);
			wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0,
 					auth->peer_mac_addr,
					wpabuf_head(auth->resp_msg),
					wpabuf_len(auth->resp_msg));
			auth->current_state = DPP_STATE_AUTH_CONF_WAITING;
			auth->response_pending = 0;
		}
		auth = wapp_get_next_auth(wapp, auth); 

		eloop_cancel_timeout(wapp_dpp_auth_conf_wait_timeout, wapp, auth);
		eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_auth_conf_wait_timeout, wapp, auth); 
		}
#ifdef MAP_R3
	}
#endif	

	return bi->id;
}

void wapp_dpp_tx_status(struct wifi_app *wapp, const u8 *dst,
			   const u8 *data, size_t data_len, int ok)
{
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)dst);
	struct wapp_dev *wdev = NULL;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: TX status: dst=" MACSTR " ok=%d",
		   MAC2STR(dst), ok);

	if (!auth) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Ignore TX status since there is no ongoing authentication exchange");
		return;
	}

	wdev = auth->wdev;

	if (!ok)
	{
		eloop_cancel_timeout(wapp_dpp_reply_wait_timeout,wapp, auth);
		eloop_cancel_timeout(wapp_dpp_auth_conf_wait_timeout, wapp, auth);
	}

	if (auth->remove_on_tx_status) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Terminate authentication exchange due to an earlier error");
		eloop_cancel_timeout(wapp_dpp_init_timeout, wapp, auth);
 		eloop_cancel_timeout(wapp_dpp_reply_wait_timeout,
				     wapp, auth);
		eloop_cancel_timeout(wapp_dpp_auth_conf_wait_timeout, wapp, auth);
		wapp_cancel_remain_on_channel(wapp, auth->wdev);
		dpp_auth_deinit(auth);
		return;
	}

	if (auth->dpp_auth_success_on_ack_rx && ok)
		wapp_dpp_auth_success(wapp, 1, wdev);

	if (!is_broadcast_ether_addr(dst) && !ok) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Unicast DPP Action frame was not ACKed");
		if (auth->current_state == DPP_STATE_AUTH_RESP_WAITING) {
			/* In case of DPP Authentication Request frame, move to
			 * the next channel immediately. */
			wapp_cancel_remain_on_channel(wapp, auth->wdev);
			wapp_dpp_auth_init_next(wapp, auth);
			return;
		}
		if (auth->current_state == DPP_STATE_AUTH_CONF_WAITING) {
			wapp_dpp_auth_resp_retry(wapp, auth);
			return;
		}
	}

	if (!is_broadcast_ether_addr(dst) &&
		(auth->current_state == DPP_STATE_AUTH_RESP_WAITING) && ok) {
		/* Allow timeout handling to stop iteration if no response is
		 * received from a peer that has ACKed a request. */
		auth->auth_req_ack = 1;
	}

	if (auth->dpp_auth_success_on_ack_rx)
		auth->dpp_auth_success_on_ack_rx = 0;
}

void wapp_dpp_cancel_timeouts(struct wifi_app *wapp, struct dpp_authentication *auth)
{
	eloop_cancel_timeout(wapp_dpp_init_timeout, wapp, auth);
	eloop_cancel_timeout(wapp_dpp_reply_wait_timeout,wapp, auth);
	eloop_cancel_timeout(wapp_dpp_auth_conf_wait_timeout, wapp, auth);
	eloop_cancel_timeout(wapp_dpp_config_req_wait_timeout, wapp, auth);
	eloop_cancel_timeout(wapp_dpp_auth_resp_retry_timeout, wapp, auth);
#ifdef DPP_R2_SUPPORT
	eloop_cancel_timeout(wapp_dpp_config_result_wait_timeout, wapp, auth);
#endif /* DPP_R2_SUPPORT */
#ifdef MAP_R3
	eloop_cancel_timeout(wapp_dpp_eth_resp_wait_timeout, wapp, auth);
	eloop_cancel_timeout(wapp_dpp_auth_timeout, wapp, auth);
	eloop_cancel_timeout(wapp_dpp_conn_result_timeout, wapp, auth);
#endif /* MAP_R3 */
#ifdef MAP_R3_RECONFIG
	if (wapp->wdev_backup) {
		eloop_cancel_timeout(wapp_dpp_cce_res_wait_timeout_recon, wapp, wapp->wdev_backup);
		eloop_cancel_timeout(wapp_dpp_reconfig_auth_wait_timeout, wapp, wapp->wdev_backup);
	}
	if (auth) {
		eloop_cancel_timeout(wapp_dpp_reconfig_auth_resp_wait_timeout, wapp, auth->peer_mac_addr);
		eloop_cancel_timeout(wapp_dpp_reconfig_auth_confirm_wait_timeout, wapp, auth->peer_mac_addr);
	}
#endif
}

static void wapp_dpp_reply_wait_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = timeout_ctx;
	unsigned int chan;
	struct os_time now, diff;  
	unsigned int wait_time, diff_ms;

	if (!auth || (auth->current_state != DPP_STATE_AUTH_RESP_WAITING))
		return;
#if 0
	wait_time = wapp->dpp->dpp_resp_wait_time ?
		wapp->dpp->dpp_resp_wait_time : 2000;
#endif	
	if (wapp->dpp->dpp_resp_wait_time)
			wait_time = wapp->dpp->dpp_init_retry_time;
		else
			wait_time = 10000;
	
	os_get_time(&now);
	os_time_sub(&now, &auth->dpp_last_init, &diff);
	diff_ms = diff.sec * 1000 + diff.usec / 1000;
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		   "DPP: Reply wait timeout - wait_time=%u diff_ms=%u",
		   wait_time, diff_ms);

	if (auth->auth_req_ack && diff_ms >= wait_time) {
		/* Peer ACK'ed Authentication Request frame, but did not reply
		 * with Authentication Response frame within two seconds. */
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: No response received from responder - stopping initiation attempt");
		wapp_cancel_remain_on_channel(wapp, auth->wdev);
		wapp_dpp_listen_stop(wapp, auth->wdev);
		/* Cacelling all timouts if ACK for unicast auth request received */
                wapp_dpp_cancel_timeouts(wapp,auth);
		dpp_auth_deinit(auth);
		return;
	}

	if (diff_ms >= wait_time) {
		/* Authentication Request frame was not ACK'ed and no reply
		 * was receiving within two seconds. */
		wpa_printf(MSG_INFO1, DPP_MAP_PREX
			   "DPP: Continue Initiator channel iteration");
		wapp_cancel_remain_on_channel(wapp, auth->wdev);
		wapp_dpp_listen_stop(wapp, auth->wdev);
		wapp_dpp_auth_init_next(wapp, auth);
		return;
	}

	/* wait rest of the time on home channel
	 * and reuse this API to send on next channel */
	wait_time -= diff_ms;

	if (auth->neg_chan > 0) {
		chan = auth->neg_chan;
		wapp_cancel_remain_on_channel(wapp, auth->wdev);

		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				"DPP: Continue reply wait on channel %u for %u ms",
				chan, wait_time);

		eloop_register_timeout(wait_time / 1000, (wait_time % 1000) * 1000,
				wapp_dpp_reply_wait_timeout, wapp, auth);
	}
}


static void wapp_dpp_init_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = timeout_ctx;

	if (!auth)
		return;
	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Retry initiation after timeout: pre ch:%d, neg ch:%d", auth->pre_home_chan, auth->neg_chan);
	if(auth->neg_chan > 0 && auth->wdev->radio->op_ch == auth->neg_chan 
		&& auth->neg_chan != auth->pre_home_chan && !auth->pre_home_chan) {
		wdev_set_quick_ch(wapp, auth->wdev, auth->pre_home_chan);
                auth->pre_home_chan = 0;
	}
	wapp_dpp_auth_init_next(wapp, auth);
}


static int wapp_dpp_auth_init_next(struct wifi_app *wapp, struct dpp_authentication *auth)
{
	const u8 *dst;
	unsigned int wait_time, max_wait_time = 10000, chan, max_tries, used;
	struct os_time now, diff; 

	if (!auth)
		return -1;

	if (auth->chan_idx == 0)
		os_get_time(&auth->dpp_init_iter_start);

	if (auth->chan_idx >= auth->num_chan) {
		auth->num_chan_iters++;
		if (wapp->dpp->dpp_init_max_tries)
			max_tries = wapp->dpp->dpp_init_max_tries;
		else
			max_tries = 5; 
		if (auth->num_chan_iters >= max_tries || auth->auth_req_ack) {
			wpa_printf(MSG_INFO1, DPP_MAP_PREX
				   "DPP: No response received from responder - stopping initiation attempt");
			eloop_cancel_timeout(wapp_dpp_reply_wait_timeout,
					     wapp, auth);
			wapp_cancel_remain_on_channel(wapp, auth->wdev);
			dpp_auth_deinit(auth);
			return -1;
		}
		auth->chan_idx = 0;
		eloop_cancel_timeout(wapp_dpp_init_timeout, wapp, auth);
		if (wapp->dpp->dpp_init_retry_time)
			wait_time = wapp->dpp->dpp_init_retry_time;
		else
			wait_time = 10000;
		os_get_time(&now);
		os_time_sub(&now, &auth->dpp_init_iter_start, &diff);
		used = diff.sec * 1000 + diff.usec / 1000;
		if (used > wait_time)
			wait_time = 0;
		else
			wait_time -= used;
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Next init attempt in %u ms",
			   wait_time);
		eloop_register_timeout(wait_time / 1000,
				       (wait_time % 1000) * 1000,
				       wapp_dpp_init_timeout, wapp,
				       auth);
		return 0;
	}
	chan = auth->chan[auth->chan_idx++];
	auth->curr_chan = chan;

	if (is_zero_ether_addr(auth->peer_bi->mac_addr))
		dst = broadcast;
	else
		dst = auth->peer_bi->mac_addr;
	auth->dpp_auth_success_on_ack_rx = 0; 
	eloop_cancel_timeout(wapp_dpp_reply_wait_timeout, wapp, auth);
	//max_wait_time = wapp->dpp->dpp_resp_wait_time ?
		//wapp->dpp->dpp_resp_wait_time : 2000;

	if (wapp->dpp->dpp_resp_wait_time)
			wait_time = wapp->dpp->dpp_init_retry_time;
		else
			wait_time = 10000;
	

	if (wait_time > max_wait_time)
		wait_time = max_wait_time;
	wait_time += 10; /* give the driver some extra time to complete */
	if (auth->neg_chan > 0 && chan != auth->neg_chan) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Initiate on %u and move to neg_chan %u for response",
			   chan, auth->neg_chan);
		wait_time = 100;
		if(auth->wdev->radio->op_ch != auth->neg_chan) {
			wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: pre ch:%d, neg ch:%d", auth->pre_home_chan, auth->neg_chan);
			auth->pre_home_chan = auth->wdev->radio->op_ch;
			wdev_set_quick_ch(wapp, auth->wdev, auth->neg_chan);
		}

	}
	eloop_register_timeout(wait_time / 1000, (wait_time % 1000) * 1000,
			       wapp_dpp_reply_wait_timeout, wapp, auth);
	wait_time -= 10;
	auth->auth_req_ack = 0;
	os_get_time(&auth->dpp_last_init);
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR
		" chan=%u type=%d", MAC2STR(dst), auth->curr_chan,
		DPP_PA_AUTHENTICATION_REQ);
	auth->current_state = DPP_STATE_AUTH_RESP_WAITING;
	return wapp_drv_send_action(wapp, auth->wdev, chan, wait_time,
 				       dst,
				       wpabuf_head(auth->req_msg),
				       wpabuf_len(auth->req_msg));
}
void wapp_dpp_auth_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = timeout_ctx;

	if (!auth)
		return;
	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Retry initiation after timeout");
#ifdef MAP_R3
	dpp_auth_fail_wrapper(wapp, "DPP: Retry initiation after timeout");
#endif
	dpp_direct_1905msg_auth_next(wapp, auth);
}

#ifdef MAP_R3
int wapp_dpp_add_chirp_tlv(struct dpp_authentication *auth)
{
#if 0
	struct wpabuf *der = NULL, *prefix = NULL;
	const u8 * addr[2];
	size_t len[2];
	int res;
	const char chirp_str[10] = "chirp";

	der = dpp_bootstrap_key_der(auth->peer_bi->pubkey);
        if (!der)
                return -1;
        wpa_hexdump_buf(MSG_DEBUG, "DPP: Compressed public key (DER)",
                        der);


	prefix = wpabuf_alloc_copy(chirp_str, os_strlen(chirp_str));
        if (!prefix){
                wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Prefix alloc failed");
                return -1;
	}

        addr[0] = wpabuf_head(prefix);
        len[0] = wpabuf_len(prefix);

	addr[1] = wpabuf_head(der);
        len[1] = wpabuf_len(der);

	res = sha256_vector(2, addr, len, auth->chirp_tlv.hash_payload);
        if (res < 0){
                wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to hash public key");
        	wpabuf_free(der);
		return -1;
	}
        else
                wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Public key hash", auth->chirp_tlv.hash_payload,
                            SHA256_MAC_LEN);
#endif
	if(auth == NULL)
		return -1;
	/* 
	In the DPP Chirp Value TLV, the Multi-AP Controller shall set the Hash Type value to 0, 
	the Hash Validity to 1 and the Hash Value to the value computed from the DPP URI 
	*/
	os_memcpy(auth->chirp_tlv.hash_payload, auth->peer_bi->chirp_hash, SHA256_MAC_LEN);

	auth->chirp_tlv.hash_len = SHA256_MAC_LEN;
	//auth->chirp_tlv.hash_type = 0;
	auth->chirp_tlv.hash_validity = 1;
	
        return 0;
}

struct dpp_authentication *wapp_map_dpp_auth_init(struct wifi_app *wapp, struct wapp_dev *wdev,
		struct dpp_bootstrap_info *peer_bi_recv)
{
	unsigned int pos = 1;
	struct dpp_bootstrap_info *peer_bi, *own_bi = NULL;
	u8 allowed_roles = wapp->dpp->dpp_allowed_roles;
	unsigned int neg_chan = 0;
#ifdef CONFIG_DPP2
    int map = 0;
	u8 agnt_alid[MAC_ADDR_LEN]= {0};
#ifdef MAP_R3
	int res = 0;
#endif /* MAP_R3 */
#endif /* CONFIG_DPP2 */
	struct dpp_authentication *auth = NULL;

	peer_bi = peer_bi_recv;
	
	own_bi = dpp_bootstrap_get_id(wapp->dpp,
				      pos);
	if (!own_bi) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX
			   "DPP: Could not find bootstrapping info for the identified local entry");
		return NULL;
	}

	if (peer_bi->curve != own_bi->curve) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Mismatching curves in bootstrapping info (peer=%s own=%s)",
			   peer_bi->curve->name, own_bi->curve->name);
		return NULL;
	}

#ifdef MAP_R3
	if (wapp->dpp->is_map)
		map = 1;
#endif /* MAP_R3 */

	if (!auth)
		auth = dpp_auth_init(wapp, wdev, peer_bi, own_bi,
				       allowed_roles, neg_chan);
	if (!auth)
		goto fail;

#ifdef MAP_R3
#ifdef CONFIG_DPP2
	if (map) {
		os_memcpy(auth->peer_mac_addr, agnt_alid,
			  MAC_ADDR_LEN);
		res = wapp_dpp_add_chirp_tlv(auth);
		if(res < 0)
			goto fail;
	}
#endif /* CONFIG_DPP2 */
#endif /* MAP_R3 */

	if (!is_zero_ether_addr(peer_bi->mac_addr))
		os_memcpy(auth->peer_mac_addr, peer_bi->mac_addr,
			  MAC_ADDR_LEN);

	if(wapp_dpp_auth_list_insert(wapp, auth) < 0) {
		dpp_auth_deinit(auth);
		goto fail;
	}
	if (dpp_set_configurator(wapp->dpp, wapp,
				 auth, NULL) < 0) {
		dpp_auth_deinit(auth);
		goto fail;
	}

	auth->neg_chan = neg_chan;

    if (dpp_map_init(wapp->dpp, auth, peer_bi->mac_addr) < 0) {
		goto fail;
    }

	return auth;
fail:
	return NULL;
}

#endif /* MAP_R3 */

int wapp_dpp_auth_init(struct wifi_app *wapp, struct wapp_dev *wdev, const char *cmd)
{
	const char *pos;
	struct dpp_bootstrap_info *peer_bi, *own_bi = NULL;
	u8 allowed_roles = wapp->dpp->dpp_allowed_roles;
	unsigned int neg_chan = 0;
#ifdef CONFIG_DPP2
        int tcp = 0, map = 0;
	int tcp_port = DPP_TCP_PORT;
	struct wapp_ip_addr ipaddr;
	char *addr, *token1;
	u8 agnt_alid[MAC_ADDR_LEN]= {0}, i = 0;
#ifdef MAP_R3
	int res = 0;
#endif /* MAP_R3 */
#endif /* CONFIG_DPP2 */
	struct dpp_authentication *auth = NULL;
	char old_auth = 0;

	pos = os_strstr(cmd, " peer=");
	if (!pos)
		return -1;
	pos += 6;
	peer_bi = dpp_bootstrap_get_id(wapp->dpp, atoi(pos));
	if (!peer_bi) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX
			   "DPP: Could not find bootstrapping info for the identified peer");
		return -1;
	}

	pos = os_strstr(cmd, " own=");
	if (pos) {
		pos += 5;
		own_bi = dpp_bootstrap_get_id(wapp->dpp,
					      atoi(pos));
		if (!own_bi) {
			wpa_printf(MSG_INFO1, DPP_MAP_PREX
				   "DPP: Could not find bootstrapping info for the identified local entry");
			return -1;
		}

		if (peer_bi->curve != own_bi->curve) {
			wpa_printf(MSG_ERROR, DPP_MAP_PREX
				   "DPP: Mismatching curves in bootstrapping info (peer=%s own=%s)",
				   peer_bi->curve->name, own_bi->curve->name);
			return -1;
		}
	}

#ifdef CONFIG_DPP2
	pos = os_strstr(cmd, " tcp_port=");
	if (pos) {
		pos += 10;
		tcp_port = atoi(pos);
	}

	addr = get_param(cmd, " tcp_addr=");
	if (addr) {
		int res;

		res = wapp_parse_ip_addr(addr, &ipaddr);
		os_free(addr);
		if (res)
			return -1;
		tcp = 1;
	}
	addr = get_param(cmd, " almac=");
	if (addr) {
		token1 = strtok(addr, ":");

		while (token1 != NULL) {
			AtoH(token1, (char *) &agnt_alid[i], 1);
			i++;
			if (i >= MAC_ADDR_LEN)
				break;
			token1 = strtok(NULL, ":");
		}
		os_free(addr);
		map = 1;
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"almac is %02x:%02x:%02x:%02x:%02x:%02x;\n", PRINT_MAC(agnt_alid));
	}
#endif /* CONFIG_DPP2 */

	pos = os_strstr(cmd, " role=");
	if (pos) {
		pos += 6;
		if (os_strncmp(pos, "configurator", 12) == 0)
			allowed_roles = DPP_CAPAB_CONFIGURATOR;
		else if (os_strncmp(pos, "enrollee", 8) == 0)
			allowed_roles = DPP_CAPAB_ENROLLEE;
		else if (os_strncmp(pos, "either", 6) == 0)
			allowed_roles = DPP_CAPAB_CONFIGURATOR |
				DPP_CAPAB_ENROLLEE;
		else
			goto fail;
	}

	pos = os_strstr(cmd, " neg_chan=");
	if (pos)
		neg_chan = atoi(pos + 10);

#ifdef MAP_R3
	if(wapp->dpp->is_map)
		map=1;
#endif /* MAP_R3 */

	/* Multiple auth are allowed in case of configurator but in same channel
	 */
	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"allowed role = %d, %d\n", allowed_roles, DPP_CAPAB_CONFIGURATOR);
	if (allowed_roles != DPP_CAPAB_CONFIGURATOR) {
		auth = wapp_dpp_get_first_auth(wapp);
		if (auth) {
			eloop_cancel_timeout(wapp_dpp_init_timeout, wapp, auth);
			eloop_cancel_timeout(wapp_dpp_reply_wait_timeout,
				     wapp, auth);
			wapp_cancel_remain_on_channel(wapp, auth->wdev);
			dpp_auth_deinit(auth);
		}
		auth = NULL;
	} else {
		if (wdev->radio->ongoing_dpp_cnt
#ifdef CONFIG_DPP2
			&& !map
#endif /* CONFIG_DPP2 */
	) { //TODO correct kapil
			wpa_printf(MSG_ERROR, DPP_MAP_PREX
				   "already %d dpp session ongoing on radio, return", wdev->radio->ongoing_dpp_cnt);
			goto fail;
		}
	} 

#ifdef CONFIG_DPP2
	if (map) {
		auth = wapp_dpp_get_auth_from_peer_mac(wapp, peer_bi->mac_addr);
		if (auth) {
			old_auth = 1;
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"got the auth instrance \n");
		}
	}
#endif /* CONFIG_DPP2 */

#ifdef MAP_R3
// Prakhar
	//Req-After receipt of a DPP URI, the Multi-AP Controller shall generate a DPP CCE Indication message containing one DPP CCE Indication TLV with tlvvalue set to 1 and send it to one or more Multi-AP Agents, which will act as Proxy Agents during the onboarding process of an Enrollee Multi-AP Agent.
	if (map) {
#if 0 //TODO after Jie fix
		if(wapp->dpp->dpp_eth_onboard_ongoing == TRUE)
		if (wapp->map->bh_type != MAP_BH_ETH)
#endif
			CCEIndication_1905_send(wapp, NULL, 1); //Enable CCE Indication in Agent
	}
#endif /* MAP_R3 */

	if (!auth)
		auth = dpp_auth_init(wapp, wdev, peer_bi, own_bi,
				       allowed_roles, neg_chan);
	if (!auth)
		goto fail;

#ifdef CONFIG_DPP2
	/* No interface needed here */
	if (!map || !tcp)
#endif /* CONFIG_DPP2 */
		auth->wdev = wdev;

#ifdef CONFIG_DPP2
	if (map){
		os_memcpy(auth->peer_mac_addr, agnt_alid,
			  MAC_ADDR_LEN);
#ifdef MAP_R3
		res = wapp_dpp_add_chirp_tlv(auth);
		if(res < 0)
			goto fail;
#endif /* MAP_R3 */
	}
#endif /* CONFIG_DPP2 */

	if (!is_zero_ether_addr(peer_bi->mac_addr))
		os_memcpy(auth->peer_mac_addr, peer_bi->mac_addr,
			  MAC_ADDR_LEN);

	if(!old_auth && wapp_dpp_auth_list_insert(wapp, auth) < 0) {
		dpp_auth_deinit(auth);
		goto fail;
	}
	if (dpp_set_configurator(wapp->dpp, wapp,
				 auth, cmd) < 0) {
		dpp_auth_deinit(auth);
		goto fail;
	}

	auth->neg_chan = neg_chan;

#ifdef CONFIG_DPP2
        if (tcp)
                return dpp_tcp_init(wapp->dpp, auth, &ipaddr, tcp_port);
	else if (map)
                return dpp_map_init(wapp->dpp, auth, peer_bi->mac_addr);
#endif /* CONFIG_DPP2 */
	return wapp_dpp_auth_init_next(wapp, auth);
fail:
	return -1;
}

void wapp_dpp_auth_conf_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = timeout_ctx;

	wpa_printf(MSG_INFO1, DPP_MAP_PREX
		   "DPP: Timeout while waiting for Authentication Confirm");
#ifdef MAP_R3
	dpp_auth_fail_wrapper(wapp, "DPP: Timeout while waiting for Authentication Confirm");
#endif
#ifdef DPP_R2_SUPPORT
	if((!auth->configurator && (auth->current_state == DPP_STATE_AUTH_CONF_WAITING))
#ifdef MAP_R3
        && (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE)
#endif /* MAP_R3 */
	) {
		wapp_cancel_remain_on_channel(wapp, auth->wdev);
		wapp_dpp_listen_stop(wapp, auth->wdev);
		dpp_auth_deinit(auth);
		if(wapp->dpp->dpp_eth_conn_ind == FALSE) {
			wapp->dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_INIT;
			wapp_dpp_presence_annouce(wapp, NULL);
		}
	}
#endif /* DPP_R2_SUPPORT */

	return;
}

static void wapp_dpp_rx_auth_req(struct wifi_app *wapp,  struct wapp_dev *wdev,
				 const u8 *src, const u8 *hdr, const u8 *buf,
				 size_t len, unsigned int chan)
{
	const u8 *r_bootstrap, *i_bootstrap;
	u16 r_bootstrap_len, i_bootstrap_len;
	struct dpp_bootstrap_info *own_bi = NULL, *peer_bi = NULL;
	struct dpp_authentication *auth = NULL;
#ifdef MAP_R3
	struct wapp_dev * agnt_wdev=  NULL;
	struct dl_list *dev_list;
	char cmd[100];
	unsigned char change_ch = 0;
#endif /* MAP_R3 */

	if (!wapp->dpp){
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			"Invalid DPP instance");
		return;
	}

#ifdef MAP_R3
	/* For wifi onboarding, check for auth request wait check */
	if(wapp->dpp->dpp_wifi_onboard_ongoing == TRUE) {
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX" Pre status enrollee:%d and Current presence channel:%d\n",
				wapp->dpp->annouce_enrolle.pre_status,wapp->dpp->annouce_enrolle.curr_presence_chan);
		if (((wapp->dpp->annouce_enrolle.curr_presence_chan == chan) &&
					(wapp->dpp->annouce_enrolle.pre_status != DPP_PRE_STATUS_WAIT_AUTHREQ)) ||
				(wapp->dpp->annouce_enrolle.curr_presence_chan != chan)) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Enrollee not waiting for auth req on this channel, ignore auth req.\n");
			return;
		}
	}
#endif /* MAP_R3 */
	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Authentication Request from " MACSTR,
		   MAC2STR(src));

	r_bootstrap = dpp_get_attr(buf, len, DPP_ATTR_R_BOOTSTRAP_KEY_HASH,
				   &r_bootstrap_len);
	if (!r_bootstrap || r_bootstrap_len != SHA256_MAC_LEN) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			"Missing or invalid required Responder Bootstrapping Key Hash attribute");
		return;
	}
	wpa_hexdump(MSG_MSGDUMP, DPP_MAP_PREX "DPP: Responder Bootstrapping Key Hash",
		    r_bootstrap, r_bootstrap_len);

	i_bootstrap = dpp_get_attr(buf, len, DPP_ATTR_I_BOOTSTRAP_KEY_HASH,
				   &i_bootstrap_len);
	if (!i_bootstrap || i_bootstrap_len != SHA256_MAC_LEN) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			"Missing or invalid required Initiator Bootstrapping Key Hash attribute");
		return;
	}
	wpa_hexdump(MSG_MSGDUMP, DPP_MAP_PREX "DPP: Initiator Bootstrapping Key Hash",
		    i_bootstrap, i_bootstrap_len);

	/* Try to find own and peer bootstrapping key matches based on the
	 * received hash values */
	dpp_bootstrap_find_pair(wapp->dpp, i_bootstrap,
				r_bootstrap, &own_bi, &peer_bi);
#ifdef CONFIG_DPP2
	if (!own_bi) {
		if (dpp_relay_rx_action(wapp->dpp,
					src, hdr, buf, len, chan, i_bootstrap,
					r_bootstrap) == 0)
			return;
	}
#endif /* CONFIG_DPP2 */
	if (!own_bi) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX
			"No matching own bootstrapping key found - ignore message");
		return;
	}

	/* Get first instance, if not NULL and enrollee mode, return */
	if (wapp_dpp_get_first_auth(wapp) && (wapp->dpp->dpp_allowed_roles != DPP_CAPAB_CONFIGURATOR)) {
#ifdef MAP_R3
		if (wapp->is_eth_onboard == TRUE)
			dpp_auth_deinit(auth);
		else 
#endif
		{
			wpa_printf(MSG_ERROR, DPP_MAP_PREX
				"Already in DPP authentication exchange - ignore new one");
			return;
		}
	}
	
	auth = dpp_auth_req_rx(wapp, wapp->dpp->dpp_allowed_roles,
					 wapp->dpp->dpp_qr_mutual,
					 peer_bi, own_bi, chan, hdr, buf, len);
	/* add in list */
	if (!auth) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No response generated");
		return;
	}
#ifdef DPP_R2_SUPPORT
	if(auth && !auth->configurator)
		wapp_dpp_check_presence_auth_req(wapp, r_bootstrap, own_bi, auth);
#endif /* DPP_R2_SUPPORT */

#ifdef MAP_R3
	if(wapp->map && (wapp->map->map_version == DEV_TYPE_R3) && !wapp->dpp->onboarding_type
		&& wapp->dpp->wsc_onboard_done) {
		/* If device is onboarded previosly using R1/R2 and
		 * needs to be onboarded via DPP then disconnect any existing connection*/
		dev_list = &wapp->dev_list;
		dl_list_for_each(agnt_wdev, dev_list, struct wapp_dev, list) {
			if ((agnt_wdev) && (agnt_wdev->dev_type == WAPP_DEV_TYPE_STA)) {

#if NL80211_SUPPORT
				u8 Enable = 0;
				wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
						(char *)&Enable, 1);
#else
				DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"ifname %s\n", agnt_wdev->ifname);
				os_memset(cmd, 0, 100);
				snprintf(cmd,100, "iwpriv %s set ApCliEnable=0;", agnt_wdev->ifname);
				system(cmd);
#endif
				wapp->dpp_caused_disconnect = 1;
			}
		}
		/* Resetting WSC done flag here to support R3 onboarding after this */
		wapp->dpp->wsc_onboard_done = 0;
	}
#endif /* MAP_R3 */

	auth->dpp_auth_success_on_ack_rx = 0;

	if (dpp_set_configurator(wapp->dpp, wapp,
				 auth,
				 " ") < 0) {
#ifdef DPP_R2_SUPPORT
		if(!auth->configurator
#ifdef MAP_R3
		&& (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE)
#endif /* MAP_R3 */
		) {
			dpp_auth_deinit(auth);
			if(wapp->dpp->dpp_eth_conn_ind == FALSE) {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"rx auth req set configurator fail, restart presence frame \n");
				wapp->dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_INIT;
				wapp_dpp_presence_annouce(wapp, NULL);
			}
		}
#endif /* DPP_R2_SUPPORT */
		return;
	}

#ifdef CONFIG_DPP2
	if (!chan) {
		auth->is_wired = 1;
		auth->is_map_connection = 1; //kapil TODO correct, tcp/map how to
	}
	if (auth->is_wired) {
		auth->wdev = wdev; //Prakhar
		wpabuf_free(auth->msg_out);
		auth->msg_out_pos = 0;
		auth->msg_out = wpabuf_alloc(4 + wpabuf_len(auth->resp_msg) - 1);
		if (!auth->msg_out)
			return;
		if (!auth->is_map_connection)
			wpabuf_put_be32(auth->msg_out, wpabuf_len(auth->resp_msg) - 1);
		wpabuf_put_data(auth->msg_out, wpabuf_head(auth->resp_msg) + 1,
				wpabuf_len(auth->resp_msg) - 1);

		if (auth->is_map_connection) {
			auth->allowed_roles = wapp->dpp->dpp_allowed_roles;
			os_memcpy(auth->peer_mac_addr, src, ETH_ALEN);
			os_memcpy(auth->relay_mac_addr, src, ETH_ALEN);
		}
		if(wapp_dpp_auth_list_insert(wapp, auth) < 0) {
			dpp_auth_deinit(auth);
			return;
		}

		if (dpp_wired_send(auth) == 1) {
			if (!auth->write_eloop) {
				if (eloop_register_sock(auth->sock, EVENT_TYPE_WRITE,
							dpp_conn_tx_ready,
							auth, NULL) < 0)
					return;
				auth->write_eloop = 1;
			}
		}

		eloop_cancel_timeout(wapp_dpp_auth_conf_wait_timeout, wapp, auth);
		eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_auth_conf_wait_timeout, wapp, auth);
	} else 
#endif /* CONFIG_DPP2 */
	{
		if ((auth->curr_chan != chan) && wdev->radio->ongoing_dpp_cnt) {
			/* dpp ongoing on one channel, can't go on another channel */
			wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: dpp ongoing on one channel, reject");
			auth->wdev = wdev;
#ifdef DPP_R2_SUPPORT
			if(!auth->configurator
#ifdef MAP_R3
			&& (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE)
#endif /* MAP_R3 */
			) {
				dpp_auth_deinit(auth);
				if(wapp->dpp->dpp_eth_conn_ind == FALSE) {
					DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"rx auth req chan fail, restart presence frame \n");
					wapp->dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_INIT;
					wapp_dpp_presence_annouce(wapp, NULL);
				}
			}
#endif /* DPP_R2_SUPPORT */
			return;
		} else {
			if (auth->curr_chan != chan) {
				auth->wdev = wdev;
				/* Since we are responder , move to initiator's channel */
				wapp_cancel_remain_on_channel(wapp, auth->wdev);  
				wdev_set_quick_ch(wapp, auth->wdev, auth->curr_chan);
#ifdef MAP_R3 //TODO optimize this
				sleep(2);
#endif /* MAP_R3 */
			}
		}

#ifdef DPP_R2_SUPPORT
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"wdev cur ch:%d, %d, wdev op_ch:%d \n",
							auth->curr_chan, chan, wdev->radio->op_ch);

		if (auth->curr_chan != wdev->radio->op_ch) {
			auth->wdev = wdev;
			wapp_cancel_remain_on_channel(wapp, wdev);
			wdev_set_quick_ch(wapp, auth->wdev, auth->curr_chan);
#ifdef MAP_R3 //TODO optimize this
			change_ch = 1;
			//sleep(2);
#endif /* MAP_R3 */
		}
#endif /* DPP_R2_SUPPORT */
		os_memcpy(auth->peer_mac_addr, src, ETH_ALEN);
		auth->wdev = wdev;
		if(wapp_dpp_auth_list_insert(wapp, auth) < 0) {
#ifdef DPP_R2_SUPPORT
			if(!auth->configurator
#ifdef MAP_R3
			&& (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE)
#endif /* MAP_R3 */
			) {
				dpp_auth_deinit(auth);
				if(wapp->dpp->dpp_eth_conn_ind == FALSE) {
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"rx auth req insert fail, restart presence frame \n");
					wapp->dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_INIT;
					wapp_dpp_presence_annouce(wapp, NULL);
				}
			}
#endif /* DPP_R2_SUPPORT */
			return;
		}

#ifdef MAP_R3
		if (change_ch)
			eloop_register_timeout(1, 0, wapp_dpp_auth_rx_wrapper, wapp, auth);
		else {
			wapp->map->ch_change_done = 1;/*Since no channel change*/
			eloop_register_timeout(0, 0, wapp_dpp_auth_rx_wrapper, wapp, auth);
		}
#else
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR
				" chan=%u type=%d", MAC2STR(src), auth->curr_chan,
				DPP_PA_AUTHENTICATION_RESP);
		dpp_auth_fail_wrapper(wapp,"DPP: Authentication Request received");
		auth->current_state = DPP_STATE_AUTH_CONF_WAITING;
		wapp_drv_send_action(wapp, wdev, auth->curr_chan, 0,
				src, wpabuf_head(auth->resp_msg),
				wpabuf_len(auth->resp_msg));
		if(!wapp->dpp->dpp_qr_mutual)
		{
			eloop_cancel_timeout(wapp_dpp_auth_conf_wait_timeout, wapp, auth);
			eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_auth_conf_wait_timeout, wapp, auth);
		}
#endif
	}
}

#ifdef DPP_R2_MUOBJ
int wapp_dpp_handle_config_obj(struct wifi_app *wapp,
					  struct dpp_authentication *auth, struct dpp_config_obj *conf)
{
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONF_RECEIVED);
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONFOBJ_AKM "%s",
		dpp_akm_str(conf->akm));
	if (conf->ssid_len)
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONFOBJ_SSID "%s",
			os_ssid_txt(conf->ssid, conf->ssid_len));
	if (conf->connector) {
		/* TODO: Save the Connector and consider using a command
		 * to fetch the value instead of sending an event with
		 * it. The Connector could end up being larger than what
		 * most clients are ready to receive as an event
		 * message. */
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX DPP_EVENT_CONNECTOR "%s",
			conf->connector);
	} else if (conf->passphrase[0]) {
		char hex[64 * 2 + 1];
		os_snprintf_hex(hex, sizeof(hex),
				 (const u8 *) conf->passphrase,
				 os_strlen(conf->passphrase));
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONFOBJ_PASS "%s",
			hex);
	} else if (conf->psk_set) {
		char hex[PMK_LEN * 2 + 1];
		os_snprintf_hex(hex, sizeof(hex), conf->psk, PMK_LEN);
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONFOBJ_PSK "%s",
			hex);
	}
	if (auth->c_sign_key) {
		char *hex;
		size_t hexlen;

		hexlen = 2 * wpabuf_len(auth->c_sign_key) + 1;
		hex = os_malloc(hexlen);
		if (hex) {
			os_snprintf_hex(hex, hexlen,
					 wpabuf_head(auth->c_sign_key),
					 wpabuf_len(auth->c_sign_key));
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				DPP_EVENT_C_SIGN_KEY "%s", hex);
			os_free(hex);
		}
	}
	if (auth->net_access_key) {
		char *hex;
		size_t hexlen;

		hexlen = 2 * wpabuf_len(auth->net_access_key) + 1;
		hex = os_malloc(hexlen);
		if (hex) {
			os_snprintf_hex(hex, hexlen,
					 wpabuf_head(auth->net_access_key),
					 wpabuf_len(auth->net_access_key));
			if (auth->net_access_key_expiry)
				wpa_printf(MSG_DEBUG, DPP_MAP_PREX
					DPP_EVENT_NET_ACCESS_KEY "%s %lu", hex,
					(unsigned long)
					auth->net_access_key_expiry);
			else
				wpa_printf(MSG_DEBUG, DPP_MAP_PREX
					DPP_EVENT_NET_ACCESS_KEY "%s", hex);
			os_free(hex);
		}
	}
#ifdef MAP_R3 //Not saving for MAP_R3
	/* Saving the configuration here */
	if (wapp->map->TurnKeyEnable == 1)
		dpp_save_config_to_file(wapp, auth, conf);
#endif /* MAP_R3 */

	return wapp_dpp_process_config(wapp, auth, conf);
}
#else

int wapp_dpp_handle_config_obj(struct wifi_app *wapp,
					  struct dpp_authentication *auth)
{
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONF_RECEIVED);
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONFOBJ_AKM "%s",
		dpp_akm_str(auth->akm));
	if (auth->ssid_len)
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONFOBJ_SSID "%s",
			os_ssid_txt(auth->ssid, auth->ssid_len));
	if (auth->connector) {
		/* TODO: Save the Connector and consider using a command
		 * to fetch the value instead of sending an event with
		 * it. The Connector could end up being larger than what
		 * most clients are ready to receive as an event
		 * message. */
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONNECTOR "%s",
			auth->connector);
	} else if (auth->passphrase[0]) {
		char hex[64 * 2 + 1];
		os_snprintf_hex(hex, sizeof(hex),
				 (const u8 *) auth->passphrase,
				 os_strlen(auth->passphrase));
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONFOBJ_PASS "%s",
			hex);
	} else if (auth->psk_set) {
		char hex[PMK_LEN * 2 + 1];
		os_snprintf_hex(hex, sizeof(hex), auth->psk, PMK_LEN);
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONFOBJ_PSK "%s",
			hex);
	}
	if (auth->c_sign_key) {
		char *hex;
		size_t hexlen;

		hexlen = 2 * wpabuf_len(auth->c_sign_key) + 1;
		hex = os_malloc(hexlen);
		if (hex) {
			os_snprintf_hex(hex, hexlen,
					 wpabuf_head(auth->c_sign_key),
					 wpabuf_len(auth->c_sign_key));
			wpa_printf(MSG_INFO1, DPP_MAP_PREX
				DPP_EVENT_C_SIGN_KEY "%s", hex);
			os_free(hex);
		}
	}

#ifdef MAP_R3_RECONFIG
	if (auth->pp_key) {
		char *hex;
		size_t hexlen;

		hexlen = 2 * wpabuf_len(auth->pp_key) + 1;
		hex = os_malloc(hexlen);
		if (hex) {
			os_snprintf_hex(hex, hexlen,
					 wpabuf_head(auth->pp_key),
					 wpabuf_len(auth->pp_key));
			wpa_printf(MSG_ERROR, DPP_MAP_PREX
				DPP_EVENT_PP_KEY "%s", hex);
			os_free(hex);
		}
	}
#endif /* MAP_R3 */

	if (auth->net_access_key) {
		char *hex;
		size_t hexlen;

		hexlen = 2 * wpabuf_len(auth->net_access_key) + 1;
		hex = os_malloc(hexlen);
		if (hex) {
			os_snprintf_hex(hex, hexlen,
					 wpabuf_head(auth->net_access_key),
					 wpabuf_len(auth->net_access_key));
			if (auth->net_access_key_expiry)
				wpa_printf(MSG_INFO1, DPP_MAP_PREX
					DPP_EVENT_NET_ACCESS_KEY "%s %lu", hex,
					(unsigned long)
					auth->net_access_key_expiry);
			else
				wpa_printf(MSG_INFO1, DPP_MAP_PREX
					DPP_EVENT_NET_ACCESS_KEY "%s", hex);
			os_free(hex);
		}
	}
#ifdef MAP_R3 //Not saving for MAP_R3
	/* Saving the configuration here */
	if (wapp->map && wapp->map->TurnKeyEnable == 1)
		dpp_save_config_to_file(wapp, auth);
#endif /* MAP_R3 */
	return wapp_dpp_process_config(wapp, auth);
}
#endif /* DPP_R2_MUOBJ */

void wapp_dpp_config_req_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = timeout_ctx;

	wpa_printf(MSG_ERROR, DPP_MAP_PREX
		   "DPP: Timeout while waiting for Configuration Request");
#ifdef MAP_R3
	dpp_auth_fail_wrapper(wapp, "Timeout while waiting for Configuration Request\n");
#endif /* MAP_R3 */
	wapp_cancel_remain_on_channel(wapp, auth->wdev);
	wapp_dpp_listen_stop(wapp, auth->wdev);
	dpp_auth_deinit(auth);
	return;
}

static void wapp_dpp_rx_auth_resp(struct wifi_app *wapp, const u8 *src,
				     const u8 *hdr, const u8 *buf, size_t len,
				     unsigned int chan)
{
	/* Since we can't be initiator in more than one case
	 * We should be able to handle multiple enrollee in responder mode
	 * only. In initiator mode, its not possible to loop thorugh the channels.
	 */
	/* Food for thought: Can we become initiator in #radio times at the same time
	 * but do we know the channels of each enrollee in that case?
	 */
#ifdef MAP_R3
	const u8 *r_bootstrap;
	u16 r_bootstrap_len;
	struct dpp_agent_info *agnt_info = NULL;

	struct dpp_authentication *auth = NULL;
	r_bootstrap = dpp_get_attr(buf, len, DPP_ATTR_R_BOOTSTRAP_KEY_HASH,
				   &r_bootstrap_len);
	if (!r_bootstrap || r_bootstrap_len != SHA256_MAC_LEN) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			"Missing or invalid required Responder Bootstrapping Key Hash attribute");
		return;
	}
	wpa_hexdump(MSG_INFO1, DPP_MAP_PREX "DPP: Responder Bootstrapping Key Hash",
		    r_bootstrap, r_bootstrap_len);
	auth = wapp_dpp_get_auth_from_resp_hash_val(wapp, (u8 *) r_bootstrap);
	agnt_info = wapp_dpp_get_agent_list_from_resp_hash_val(wapp, (u8 *) r_bootstrap);
	if (!agnt_info) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: No Agent with responder hash found\n");
	}
#else
	struct dpp_authentication *auth = wapp_dpp_get_last_auth(wapp);
#endif /* MAP_R3 */

	struct wpabuf *msg;

	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Authentication Response from " MACSTR,
		   MAC2STR(src));

	if (!auth) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: No DPP Authentication in progress - drop");
#ifdef MAP_R3
		dpp_auth_fail_wrapper(wapp, "DPP: No DPP Authentication in progress - drop");
#endif
		return;
	}

	if (auth->current_state != DPP_STATE_AUTH_RESP_WAITING) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Incorrect auth state=%d", auth->current_state);
#ifdef MAP_R3
		dpp_auth_fail_wrapper(wapp, "DPP: Incorrect auth state");
#endif
		//dpp_auth_deinit(auth);
		return;
	}

#ifndef MAP_R3
	if (!is_zero_ether_addr(auth->peer_mac_addr) &&
	    os_memcmp(src, auth->peer_mac_addr, MAC_ADDR_LEN) != 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: MAC address mismatch (expected "
			   MACSTR ") - drop", MAC2STR(auth->peer_mac_addr));
#ifdef MAP_R3
		dpp_auth_fail_wrapper(wapp, "DPP: MAC address mismatch");
#endif

		dpp_auth_deinit(auth);
		return;
	}
#endif /* MAP_R3 */
	wapp_dpp_cancel_timeouts(wapp,auth); /* Timeout shoud be cancelled in all cases */
#ifdef CONFIG_DPP2
        if (!auth->is_wired) {
		//wapp_dpp_cancel_timeouts(wapp,auth);
	/* It may be possible that we got the response in different channel
	 * Ideally in that case, timer in the driver may still be running
	 * cancel that timer, new channel info will be stored in auth packet
	 * and channel switch should be taken care by that.
	 */
		wapp_cancel_remain_on_channel(wapp, auth->wdev);
		if (auth->curr_chan != chan && auth->neg_chan == chan) {
			wpa_printf(MSG_INFO1, DPP_MAP_PREX
				   "DPP: Responder accepted request for different negotiation channel");
			auth->curr_chan = chan;
		}
	
		if (auth->wdev && auth->wdev->radio->op_ch != chan) {
			/* Since we are initiator, move to responder's channel */
			wdev_set_quick_ch(wapp, auth->wdev, chan);
		}
	}
#endif
	msg = dpp_auth_resp_rx(auth, hdr, buf, len);
	if (!msg) {
		if (auth->auth_resp_status == DPP_STATUS_RESPONSE_PENDING) {
			wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Wait for full response");
			return;
		}
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: No confirm generated");
		dpp_auth_deinit(auth);
		return;
	}

#ifdef MAP_R3
	os_memcpy(auth->peer_mac_addr, src, MAC_ADDR_LEN);
	os_memcpy(auth->relay_mac_addr, wapp->dpp->relay_almac_addr, MAC_ADDR_LEN); /* relay mac is almac used just in auth response*/
	os_memset(wapp->dpp->relay_almac_addr, 0, MAC_ADDR_LEN);

	if(agnt_info) {
		os_memcpy(agnt_info->agnt_mac_addr, src, MAC_ADDR_LEN);
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: peer mac addr" MACSTR "copied in agnt list with bh type %u\n",
			MAC2STR(agnt_info->agnt_mac_addr),agnt_info->bh_type);
	}
#endif /* MAP_R3 */

#ifdef CONFIG_DPP2
	if (auth->is_wired) {
		wpabuf_free(auth->msg_out);
		auth->msg_out_pos = 0;
		auth->msg_out = wpabuf_alloc(4 + wpabuf_len(msg) - 1);
		if (!auth->msg_out) {
			wpabuf_free(msg);
			return;
		}
		if (!auth->is_map_connection)
			wpabuf_put_be32(auth->msg_out, wpabuf_len(msg) - 1);
		wpabuf_put_data(auth->msg_out, wpabuf_head(msg) + 1,
				wpabuf_len(msg) - 1);
		wpabuf_free(msg);

		auth->on_tcp_tx_complete_auth_ok = 1;
		if (auth->is_map_connection) {
			if (wapp->dpp->dpp_configurator_supported)
				auth->current_state = DPP_STATE_CONFIG_REQ_WAITING;
		}
		if (dpp_wired_send(auth) == 1) {
			if (!auth->write_eloop) {
				if (eloop_register_sock(auth->sock, EVENT_TYPE_WRITE,
							dpp_conn_tx_ready,
							auth, NULL) < 0)
					return;
				auth->write_eloop = 1;
			}
		}

		eloop_cancel_timeout(wapp_dpp_config_req_wait_timeout, wapp, auth);
		eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_config_req_wait_timeout, wapp, auth);

	} else 
#endif /* CONFIG_DPP2 */
	{
		wpa_printf(MSG_INFO, DPP_EVENT_TX "dst=" MACSTR
				" chan=%u type=%d", MAC2STR(src), auth->curr_chan,
				DPP_PA_AUTHENTICATION_CONF);

		if (wapp->dpp->dpp_configurator_supported)
			auth->current_state = DPP_STATE_CONFIG_REQ_WAITING;
		else
			auth->current_state = DPP_STATE_CONFIG_RSP_WAITING;

		wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0, src,
				wpabuf_head(msg), wpabuf_len(msg));

		if (wapp->dpp->dpp_configurator_supported)
		{
			eloop_cancel_timeout(wapp_dpp_config_req_wait_timeout, wapp, auth);
			eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_config_req_wait_timeout, wapp, auth);
		}
		wpabuf_free(msg);
		auth->dpp_auth_success_on_ack_rx = 1;
		if(auth->wdev)
			wapp_dpp_auth_success(wapp, 0, auth->wdev);;
	}
}

static void wapp_dpp_rx_auth_conf(struct wifi_app *wapp, const u8 *src,
				     const u8 *hdr, const u8 *buf, size_t len)
{
#ifdef MAP_R3
	struct wpabuf *msg;
#endif /* MAP_R3 */
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);

	if (!auth) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to get auth instance\n");
#ifdef MAP_R3
		dpp_auth_fail_wrapper(wapp, "DPP: Failed to get auth");
#endif
		return;
	}
	struct wapp_dev *wdev = auth->wdev;

	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Authentication Confirmation from " MACSTR,
		   MAC2STR(src));

	if (!auth || (auth->auth_success)) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: No DPP Authentication in progress - drop");
#ifdef MAP_R3
		dpp_auth_fail_wrapper(wapp, "DPP: No DPP Authentication in progress - drop");
#endif
		return;
	}

	if (auth->current_state != DPP_STATE_AUTH_CONF_WAITING) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Incorrect auth state=%d", auth->current_state);
#ifdef MAP_R3
		dpp_auth_fail_wrapper(wapp, "DPP: Incorrect auth state");
#endif
		return;
	}

	wapp_dpp_cancel_timeouts(wapp,auth);

#ifdef MAP_R3

	//Send auth confirmation received from Controller to Enrollee
	if(auth->is_map_connection && 
		(auth->allowed_roles == DPP_CAPAB_PROXYAGENT)) {
	
	        msg = wpabuf_alloc(2 + DPP_HDR_LEN + len);
        	if (!msg)
                	return;

        	wpabuf_put_u8(msg, WLAN_ACTION_PUBLIC);
        	wpabuf_put_u8(msg, WLAN_PA_VENDOR_SPECIFIC);
        	wpabuf_put_data(msg, hdr, DPP_HDR_LEN);
        	wpabuf_put_data(msg, buf, len);
	
		
		wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0, src,
				wpabuf_head(msg), wpabuf_len(msg)); //TODO check this

		wpabuf_free(msg);

		auth->current_state = DPP_STATE_CONFIG_REQ_WAITING;
		auth->auth_success = 1;
		
		return;
	}
#endif /* MAP_R3 */

	if (os_memcmp(src, auth->peer_mac_addr, MAC_ADDR_LEN) != 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: MAC address mismatch (expected "
			   MACSTR ") - drop", MAC2STR(auth->peer_mac_addr));
		return;
	}

	if (dpp_auth_conf_rx(auth, hdr, buf, len) < 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Authentication Confirm failed");
#ifdef DPP_R2_SUPPORT
		if(!auth->configurator
#ifdef MAP_R3
		&& (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE)
#endif /* MAP_R3 */
		) {
			dpp_auth_deinit(auth);
			if(wapp->dpp->dpp_eth_conn_ind == FALSE) {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"rx auth confirm fail, restart presence frame \n");
				wapp->dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_INIT;
				wapp_dpp_presence_annouce(wapp, NULL);
			}
		}
#endif /* DPP_R2_SUPPORT */
		return;
	}

	if (wapp->dpp->dpp_configurator_supported)
		auth->current_state = DPP_STATE_CONFIG_REQ_WAITING;
	else
		auth->current_state = DPP_STATE_CONFIG_RSP_WAITING;

	if (wapp->dpp->dpp_configurator_supported)
	{
		eloop_cancel_timeout(wapp_dpp_config_req_wait_timeout, wapp, auth);
		eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_config_req_wait_timeout, wapp, auth);
	}

	wapp_dpp_auth_success(wapp, 0, wdev);
}

#ifdef MAP_R3
void wapp_snd_bss_conn_1905(struct wifi_app *wapp, struct dpp_authentication *auth)
{
	struct dpp_configuration *conf = NULL;
	struct wpabuf *msg1 = NULL;
	struct wpabuf *msg2 = NULL;
	char conf_grpid_str[20] = "MAP_GRP_ID";	
	size_t group_id_len;
	struct dpp_bss_cred *bss_cred = NULL;
	u16 cred_len;
	unsigned char * cred_buffer = NULL;
	unsigned char * pos = NULL;
	char all_zero_mac[MAC_ADDR_LEN]={0};

	conf = os_zalloc(sizeof(*conf));
	if(conf == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"conf zalloc failed\n");
		return;
	}

	group_id_len = os_strlen(conf_grpid_str);
	conf->group_id = os_zalloc(group_id_len + 1);
	if(!conf->group_id) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"conf group ID zalloc failed\n");
		os_free(conf);
		return;
	}
	os_memcpy(conf->group_id, conf_grpid_str, group_id_len);

	msg1 = dpp_build_conf_obj_bss(auth, MAP_BACKHAUL_AP, conf);
	msg2 = dpp_build_conf_obj_bss(auth, MAP_FRONTHAUL_AP, conf);
	
	if (msg1) {
		wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: first connector",
			wpabuf_head(msg1), wpabuf_len(msg1));
	} else {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: first connector msg1:NULL\n");
		goto fail;
	}

	if (msg2) {
		wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: second connector",
			  wpabuf_head(msg2), wpabuf_len(msg2));
	} else {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: second connector msg2:NULL\n");
		goto fail;
	}

	cred_len = sizeof(*bss_cred) + wpabuf_len(msg1) + wpabuf_len(msg2);

	bss_cred = os_zalloc(cred_len);
	if(!bss_cred)
		goto fail;

	if(auth->self_sign_auth) {
		os_memcpy(bss_cred->enrollee_mac, all_zero_mac, MAC_ADDR_LEN);
		os_memcpy(bss_cred->almac, all_zero_mac, MAC_ADDR_LEN);
	}
	else {
		if(auth->ethernetTrigger == TRUE) {
			os_memcpy(bss_cred->enrollee_mac, all_zero_mac, MAC_ADDR_LEN);
			os_memcpy(bss_cred->almac, auth->peer_mac_addr, MAC_ADDR_LEN);
		}
		else {
			os_memcpy(bss_cred->enrollee_mac, auth->peer_mac_addr, MAC_ADDR_LEN);
			os_memcpy(bss_cred->almac, all_zero_mac, MAC_ADDR_LEN);
		}
	}

	bss_cred->bh_connect_len = (unsigned short)wpabuf_len(msg1);
	bss_cred->fh_connect_len = (unsigned short)wpabuf_len(msg2); 
	bss_cred->payload_len = bss_cred->bh_connect_len + bss_cred->fh_connect_len;

	cred_buffer = os_zalloc(bss_cred->payload_len);
	if(!cred_buffer)
		goto fail;
	
	pos = cred_buffer;	
	os_memcpy(cred_buffer, wpabuf_head(msg1), bss_cred->bh_connect_len);
	cred_buffer += bss_cred->bh_connect_len;
	os_memcpy(cred_buffer, wpabuf_head(msg2), bss_cred->fh_connect_len);

	os_memcpy(bss_cred->payload, pos, bss_cred->payload_len);

	hex_dump_dbg(DPP_MAP_PREX"BSS_connectors= ",(UCHAR *)bss_cred, cred_len);
#if 0
	/* Save BSS information to a file */
	if ((wapp->map->TurnKeyEnable == 1) && auth->self_sign_auth)
		dpp_save_bss_to_file(wapp, auth, bss_cred, cred_len);
#endif
	
	if (wapp_send_1905_msg(wapp, WAPP_SEND_BSS_CONNECTOR,
		cred_len, (char *)bss_cred) < 0)
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed\n");
	wpabuf_free(msg1);
	wpabuf_free(msg2);

fail:
	if(bss_cred) {
		os_free(bss_cred);
		bss_cred = NULL;
	}
	if(pos) {
		os_free(pos);
		pos = NULL;
		cred_buffer = NULL;
	}
	if(conf->group_id) {
		os_free(conf->group_id);
	}
	if(conf) {
		os_free(conf);
	}
	return;
}
#endif /* MAP_R3 */

#ifdef DPP_R2_SUPPORT
void wapp_dpp_config_result_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx)
{
	struct dpp_authentication *auth = timeout_ctx;

	if (!auth || (auth->current_state != DPP_STATE_CONFIG_RESULT_WAITING))
		return;

	wpa_printf(MSG_ERROR, DPP_MAP_PREX
		   "DPP: Timeout while waiting for Configuration Result");
	dpp_auth_deinit(auth);
}


static void wapp_dpp_rx_conf_result(struct wifi_app *wapp, const u8 *src,
				       const u8 *hdr, const u8 *buf, size_t len)
{
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);
	enum dpp_status_error status;
#ifdef MAP_R3
	struct dpp_agent_info *agnt_info = NULL;
	unsigned char reconf_flag = 0;
#endif /* MAP_R3 */

	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Configuration Result from " MACSTR,
		   MAC2STR(src));

	if ((!auth) || (auth && auth->current_state == DPP_STATE_CONFIG_DONE)) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: No DPP Configuration waiting for result - drop");
		return;
	}

	if (os_memcmp(src, auth->peer_mac_addr, MAC_ADDR_LEN) != 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: MAC address mismatch (expected "
			   MACSTR ") - drop", MAC2STR(auth->peer_mac_addr));
		return;
	}
	eloop_cancel_timeout(wapp_dpp_config_result_wait_timeout, wapp,
			     auth);
	auth->current_state = DPP_STATE_CONFIG_DONE;
	status = dpp_conf_result_rx(auth, hdr, buf, len);
#ifdef CONFIG_DPP2
	user_fail_reason *info_to_mapd = os_zalloc(sizeof(user_fail_reason));
	if (info_to_mapd == NULL)
		return;
	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX" [%s] \n", __func__);
	info_to_mapd->reason_id = status;
	os_memcpy(info_to_mapd->reason, "DPP Configuration Result", os_strlen("DPP Configuration Result"));
	wapp_send_1905_msg(
			auth->msg_ctx,
			WAPP_SEND_USER_FAIL_NOTIF,
			sizeof(user_fail_reason),
			(char *)info_to_mapd);
	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" %s [%s] \n", info_to_mapd->reason, __func__);
	os_free(info_to_mapd);

#ifdef MAP_R3
	agnt_info = wapp_dpp_get_agent_list_from_agnt_mac_addr(wapp, (u8 *)src);
	if (!agnt_info) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: agnt not found with peer mac addr\n");
	}

	if (status == DPP_STATUS_OK) {
		if(agnt_info) {
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: setting the agent with mac " MACSTR " flag to dpp done state\n",
					MAC2STR(agnt_info->agnt_mac_addr));
			agnt_info->agent_state = DPP_AGT_STATE_DPP_DONE;
			/* Run timer for the agent list update */
			eloop_cancel_timeout(
				wapp_dpp_agent_list_timeout, wapp, agnt_info);
			eloop_register_timeout(
				DPP_CONT_AGNT_LIST_TIMEOUT, 0, wapp_dpp_agent_list_timeout, wapp, agnt_info);
		}
		wapp_snd_bss_conn_1905(wapp, auth);
	}

	/* Clearing the GAS server state machine for MAP here to avoid timeout */
	if (wapp->dpp && wapp->dpp->dpp_configurator_supported) {
		wapp->dpp->conf_res_received = 1;
		gas_server_tx_status(wapp->dpp->gas_server, src,
				NULL, 0, (status != DPP_STATUS_OK) ? 0 : 1);
		wapp->dpp->conf_res_received = 0;
	}
#endif /* MAP_R3 */

	if (status == DPP_STATUS_OK && auth->send_conn_status) {

		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Wait for Connection Status Result \n");
		eloop_cancel_timeout(wapp_dpp_config_result_wait_timeout, wapp,
			     auth);
		auth->waiting_conn_status_result = 1;
		eloop_cancel_timeout(
			wapp_dpp_conn_status_result_wait_timeout, wapp, auth);
		eloop_register_timeout(
			DPP_CONNECT_STATUS_TIMEOUT, 0, wapp_dpp_conn_status_result_wait_timeout, wapp, auth);
		return;
	}
#endif /* CONFIG_DPP2 */

#ifdef CONFIG_DPP2
	if (!auth->is_wired) {
		wapp_cancel_remain_on_channel(wapp, auth->wdev);
		wapp_dpp_listen_stop(wapp, auth->wdev);
	}
#endif

	if (status == DPP_STATUS_OK)
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONF_SENT);
	else
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONF_FAILED);

#ifdef MAP_R3_RECONFIG
#if 0
	if (wapp->dpp->is_map && wapp->dpp->dpp_reconfig_ongoing == TRUE) {
		ChirpMsg_1905_send(wapp, auth->own_bi, 1, auth->peer_mac_addr);
	}
#endif
#endif
#ifdef MAP_R3_RECONFIG
	//if (auth && auth->reconfigTrigger != 1)
#endif
#ifdef MAP_R3
	/* Sending chirp notification to clear enrollee state on proxy agent */
	if (auth->is_wired) {
		if (auth->reconfigTrigger)
			reconf_flag = 1;
		ChirpMsg_1905_send(wapp, auth->chirp_tlv.hash_payload, (u8 *)src
#ifdef MAP_R3_RECONFIG
					, reconf_flag
#endif
			, 0, auth->relay_mac_addr);
	}
#endif /* MAP_R3 */
	dpp_auth_deinit(auth);
}

#ifdef MAP_R3
void wapp_dpp_rx_presence_announce(struct wifi_app *wapp, struct wapp_dev *wdev, 
		const u8 *src, const u8 *hdr, const u8 *buf, size_t len, unsigned int chan)
{
 	const u8 *r_bootstrap;
	u16 r_bootstrap_len;
	unsigned int wait_time = 0;
	u8 op_bnd = 0;
	
	//struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);
	struct dpp_authentication *auth = NULL;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Presence Announcement from " MACSTR,
		   MAC2STR(src));
	if (!wapp->dpp){
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Invalid instance, return \n");
		return;
	}

	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"wdev is null, return \n");
		return;
	}
	if(wapp->dpp->dpp_allowed_roles != DPP_CAPAB_PROXYAGENT)
	{
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Device is not proxy agent, ignore this\n");
		return;
	}

	if(!wapp->dpp->dpp_chirp_handling)
	{
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"chirp handling is not set return \n");
		return;
	}
	else {
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"chirp handling is enabled\n");
	}

	if (wapp->map->MapMode == 4) {
		op_bnd = wapp_op_band_frm_ch(wapp, chan);
		if(op_bnd != wapp->conf_op_bnd) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"band %u not configured so ignoring chirp message\n",op_bnd);
			return;
		}
		else {
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"band %u configured so process the frame\n",op_bnd);
		}
	}

	r_bootstrap = dpp_get_attr(buf, len, DPP_ATTR_R_BOOTSTRAP_KEY_HASH,
				   &r_bootstrap_len);
	if (!r_bootstrap || r_bootstrap_len != SHA256_MAC_LEN) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			"Missing or invalid required Responder Bootstrapping Key Hash attribute");
		return;
	}
	wpa_hexdump(MSG_MSGDUMP, DPP_MAP_PREX "DPP: Responder Bootstrapping Key Hash",
		    r_bootstrap, r_bootstrap_len);
#if 0
	if (os_memcmp(r_bootstrap, auth->chirp_tlv.hash_payload,
		      SHA256_MAC_LEN) != 0) {
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX
			    "DPP: Expected Responder Bootstrapping Key Hash",
			    auth->chirp_tlv.hash_payload, SHA256_MAC_LEN);
		return;
	}
#endif
	auth = wapp_dpp_get_auth_from_hash_val(wapp, (u8 *) r_bootstrap);
	if(!auth) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Auth instance not found providing to controller\n");
		ChirpMsg_1905_send(wapp, (u8 *)r_bootstrap, (u8 *)src
#ifdef MAP_R3_RECONFIG
		, 0
#endif /* MAP_R3_RECONFIG */
		, 1, NULL);
		return;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Matching Key hash value found");
	
	//updating the current channel,wdev and peer mac as per presence announcement frame
	auth->wdev = wdev;
	auth->curr_chan = chan;
	os_memcpy(auth->peer_mac_addr, src, MAC_ADDR_LEN);

	wait_time=wapp->dpp->max_remain_on_chan;
	eloop_register_timeout(wait_time / 1000, (wait_time % 1000) * 1000,
			       wapp_dpp_reply_wait_timeout, wapp, auth);
	auth->current_state = DPP_STATE_AUTH_RESP_WAITING;
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR
		" chan=%u type=%d", MAC2STR(src), auth->curr_chan,
		DPP_PA_AUTHENTICATION_REQ);
	wapp_drv_send_action(wapp, auth->wdev, chan, wait_time,
 				       src,
				       wpabuf_head(auth->msg_out),
				       wpabuf_len(auth->msg_out));
}
#endif /* MAP_R3 */
#endif /* DPP_R2_SUPPORT  */

static void wapp_dpp_send_peer_disc_resp(struct wifi_app *wapp,
					 struct wapp_dev *wdev,
					    const u8 *src, unsigned int chan,
					    u8 trans_id,
					    enum dpp_status_error status)
{
	struct wpabuf *msg;
	size_t len;
#ifdef CONFIG_DPP2
	u8 version;
#endif /* CONFIG_DPP2 */

	len = 5 + 5 + 4 + os_strlen(wdev->config->dpp_connector);
#ifdef CONFIG_DPP2
	len += 5;
#endif /* CONFIG_DPP2 */
	msg = dpp_alloc_msg(DPP_PA_PEER_DISCOVERY_RESP, len);
	if (!msg)
		return;

	/* Transaction ID */
	wpabuf_put_le16(msg, DPP_ATTR_TRANSACTION_ID);
	wpabuf_put_le16(msg, 1);
	wpabuf_put_u8(msg, trans_id);

	/* DPP Status */
	wpabuf_put_le16(msg, DPP_ATTR_STATUS);
	wpabuf_put_le16(msg, 1);
	wpabuf_put_u8(msg, status);

	/* DPP Connector */
	if (status == DPP_STATUS_OK) {
		wpabuf_put_le16(msg, DPP_ATTR_CONNECTOR);
		wpabuf_put_le16(msg, os_strlen(wdev->config->dpp_connector));
		wpabuf_put_str(msg, wdev->config->dpp_connector);
	}

#ifdef CONFIG_DPP2
		if (wapp->dpp->version_ctrl> 1) {
			/* Protocol Version */
			version = wapp->dpp->version_ctrl;
			wpabuf_put_le16(msg, DPP_ATTR_PROTOCOL_VERSION);
			wpabuf_put_le16(msg, 1);
			wpabuf_put_u8(msg, version);
		}
#endif /* CONFIG_DPP2 */

	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Send Peer Discovery Response to " MACSTR
		   " status=%d", MAC2STR(src), status);

	wapp_drv_send_action(wapp, wdev, chan, 0, src,
				wpabuf_head(msg), wpabuf_len(msg));
	wpabuf_free(msg);
}

static void wapp_dpp_rx_peer_disc_req(struct wifi_app *wapp,
					struct wapp_dev *wdev,
					 const u8 *src,
					 const u8 *buf, size_t len,
					 unsigned int chan)
{
	const u8 *connector, *trans_id;
	u16 connector_len, trans_id_len;
	struct os_time now;
	struct dpp_introduction intro;
	os_time_t expire;
	int expiration;
	enum dpp_status_error res;

	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Peer Discovery Request from " MACSTR,
		   MAC2STR(src));

	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"wdev is null, return \n");
		return;
	}

	if (!wdev->config || !wdev->config->dpp_connector ||
		!wdev->config->dpp_netaccesskey || !wdev->config->dpp_csign) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: No own Connector/keys set");
		return;
	}

#ifdef MAP_R3
	/* If the wdev supports fh and bh both check for another config also */
	if (wdev->i_am_fh_bss && wdev->i_am_bh_bss && (!wdev->config2 || !wdev->config2->dpp_connector ||
		!wdev->config2->dpp_netaccesskey || !wdev->config2->dpp_csign)) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: No own second Connector/keys set");
		return;
	}
#endif /* MAP_R3 */
	os_get_time(&now);

	if (wdev->config->dpp_netaccesskey_expiry &&
	    (os_time_t) wdev->config->dpp_netaccesskey_expiry < now.sec) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Own netAccessKey expired");
		return;
	}

#ifdef MAP_R3
	/* If the wdev supports fh and bh both check for another config also */
	if (wdev->i_am_fh_bss && wdev->i_am_bh_bss && wdev->config2->dpp_netaccesskey_expiry &&
	    (os_time_t) wdev->config2->dpp_netaccesskey_expiry < now.sec) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Own netAccessKey2 expired");
		return;
	}
#endif /* MAP_R3 */
	trans_id = dpp_get_attr(buf, len, DPP_ATTR_TRANSACTION_ID,
			       &trans_id_len);
	if (!trans_id || trans_id_len != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Peer did not include Transaction ID");
		return;
	}

	connector = dpp_get_attr(buf, len, DPP_ATTR_CONNECTOR, &connector_len);
	if (!connector) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Peer did not include its Connector");
		return;
	}

	res = dpp_peer_intro(&intro, wdev->config->dpp_connector,
			     wdev->config->dpp_netaccesskey,
			     wdev->config->dpp_netaccesskey_len,
			     wdev->config->dpp_csign,
			     wdev->config->dpp_csign_len,
			     connector, connector_len, &expire);
	if (res == 255) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Network Introduction protocol resulted in internal failure (peer "
			   MACSTR ")", MAC2STR(src));
#ifdef MAP_R3
		if (wdev->i_am_fh_bss && wdev->i_am_bh_bss) {
			goto int_peer_fail;
		}
#endif /* MAP_R3 */
		return;
	}

	if (res != DPP_STATUS_OK) {
#ifdef MAP_R3
		if ((res == DPP_STATUS_NO_MATCH) && wdev->i_am_fh_bss && wdev->i_am_bh_bss) {
			/* If peer disc failure with first config the try with second if both 
			 * bh and fh are supported in the same BSS */
int_peer_fail:
			wpa_printf(MSG_ERROR, DPP_MAP_PREX
				"DPP: First Network Introduction protocol resulted in failure (peer "
				MACSTR " status %d)", MAC2STR(src), res);

			res = dpp_peer_intro(&intro, wdev->config2->dpp_connector,
					wdev->config2->dpp_netaccesskey,
					wdev->config2->dpp_netaccesskey_len,
					wdev->config2->dpp_csign,
					wdev->config2->dpp_csign_len,
					connector, connector_len, &expire);
			if (res == 255) {
				wpa_printf(MSG_ERROR, DPP_MAP_PREX
						"DPP: Network Introduction protocol resulted in internal failure (peer "
						MACSTR ")", MAC2STR(src));
				return;
			}
			if(res == DPP_STATUS_OK)
				goto peer_success;
		}
#endif /* MAP_R3 */

		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Network Introduction protocol resulted in failure (peer "
			   MACSTR " status %d)", MAC2STR(src), res);
		wapp_dpp_send_peer_disc_resp(wapp, wdev, src, chan, trans_id[0],
						res);
#if 0 /* For Controller no need to start reconfig */
		if(res == DPP_STATUS_INVALID_CONNECTOR || res == DPP_STATUS_NO_MATCH){
			wdev->config->conn_result = res;
			wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT;
			wapp_dpp_send_reconfig_annouce(wapp, wdev);
		}
#endif /* DPP_R2_RECONFIG */			
		return;
	}

#ifdef MAP_R3
peer_success:
#endif /* MAP_R3 */
	if (!expire || (os_time_t) wdev->config->dpp_netaccesskey_expiry < expire)
		expire = wdev->config->dpp_netaccesskey_expiry;
	if (expire)
		expiration = expire - now.sec;
	else
		expiration = 0;

	if (wapp_set_pmk(wapp, src, intro.pmk, intro.pmk_len,
				intro.pmkid, expiration,
				WPA_KEY_MGMT_DPP, wdev) < 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to add PMKSA cache entry");
		return;
	}

	wapp_dpp_send_peer_disc_resp(wapp, wdev, src, chan, trans_id[0],
					DPP_STATUS_OK);
}

static void wapp_dpp_rx_peer_disc_resp(struct wifi_app *wapp,
					struct wapp_dev *wdev,
				       const u8 *src,
				       const u8 *buf, size_t len)
{
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"wdev is null, return \n");
		return;
	}
	struct dpp_config *config = wdev->config;
	const u8 *connector, *trans_id, *status;
	u16 connector_len, trans_id_len, status_len;
	struct dpp_introduction intro;
	//struct rsn_pmksa_cache_entry *entry;
	struct os_time now;
	struct os_time rnow;
	os_time_t expiry;
	unsigned int seconds;
	enum dpp_status_error res;
	int expiration;
#ifdef DPP_R2_SUPPORT
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);
#endif /* DPP_R2_SUPPORT */
#ifdef MAP_R3
	struct apcli_association_info cli_assoc_info;
	os_memset(&cli_assoc_info, 0, sizeof(cli_assoc_info));
	user_fail_reason *info_to_mapd = os_zalloc(sizeof(user_fail_reason));
	if (info_to_mapd == NULL)
	{
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: mem allocation to info_to_mapd failed ");
		return;
	}
#endif
	int ret = 0;

	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Peer Discovery Response from " MACSTR,
		   MAC2STR(src));

	if (!config || !config->dpp_connector || !config->dpp_netaccesskey ||
	    !config->dpp_csign) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Profile not found for network introduction");
#ifdef MAP_R3
	if(info_to_mapd)
		os_free(info_to_mapd);
#endif
		
		return;
	}
	trans_id = dpp_get_attr(buf, len, DPP_ATTR_TRANSACTION_ID,
			       &trans_id_len);
	if (!trans_id || trans_id_len != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Peer did not include Transaction ID");
		wpa_printf(MSG_ERROR, DPP_MAP_PREX DPP_EVENT_INTRO "peer=" MACSTR
			" fail=missing_transaction_id", MAC2STR(src));
		goto fail;
	}
	if (trans_id[0] != TRANSACTION_ID) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Ignore frame with unexpected Transaction ID %u",
			   trans_id[0]);
		wpa_printf(MSG_ERROR, DPP_MAP_PREX DPP_EVENT_INTRO "peer=" MACSTR
			" fail=transaction_id_mismatch", MAC2STR(src));
		goto fail;
	}

	status = dpp_get_attr(buf, len, DPP_ATTR_STATUS, &status_len);
	if (!status || status_len != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Peer did not include Status");
		wpa_printf(MSG_ERROR, DPP_MAP_PREX DPP_EVENT_INTRO "peer=" MACSTR
			" fail=missing_status", MAC2STR(src));
		goto fail;
	}
	if (status[0] != DPP_STATUS_OK) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Peer rejected network introduction: Status %u",
			   status[0]);
		wpa_printf(MSG_ERROR, DPP_MAP_PREX DPP_EVENT_INTRO "peer=" MACSTR
			" status=%u", MAC2STR(src), status[0]);
#ifdef CONFIG_DPP2
		if (auth)
			ret = wapp_dpp_send_conn_status_result(wapp, auth, status[0]);
		if (ret < 0)
			auth = NULL;
#endif /* CONFIG_DPP2 */
		goto fail;
	}

	connector = dpp_get_attr(buf, len, DPP_ATTR_CONNECTOR, &connector_len);
	if (!connector) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Peer did not include its Connector");
		wpa_printf(MSG_ERROR, DPP_MAP_PREX DPP_EVENT_INTRO "peer=" MACSTR
			" fail=missing_connector", MAC2STR(src));
		goto fail;
	}

	res = dpp_peer_intro(&intro, config->dpp_connector,
			     config->dpp_netaccesskey,
			     config->dpp_netaccesskey_len,
			     config->dpp_csign,
			     config->dpp_csign_len,
			     connector, connector_len, &expiry);
	if (res != DPP_STATUS_OK) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX
			   "DPP: Network Introduction protocol resulted in failure");
		wpa_printf(MSG_ERROR, DPP_MAP_PREX DPP_EVENT_INTRO "peer=" MACSTR
			" fail=peer_connector_validation_failed", MAC2STR(src));
#ifdef CONFIG_DPP2
		if (auth)
			ret = wapp_dpp_send_conn_status_result(wapp, auth, res);
		if (ret < 0)
			auth = NULL;
#endif /* CONFIG_DPP2 */
#ifdef DPP_R2_RECONFIG
		if(res == DPP_STATUS_INVALID_CONNECTOR || res == DPP_STATUS_NO_MATCH) {
			config->conn_result = res;
#ifdef MAP_R4
			/* wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT; */
			wdev_handle_reconfig_trigger(wapp, wdev->ifindex, NULL); /* To be tested for reconfig */
#endif
		}
#endif /* DPP_R2_RECONFIG */
		goto fail;
	}
	if (expiry) {
		os_get_time(&now);
		seconds = expiry - now.sec;
	} else {
		seconds = 86400 * 7;
	}
	os_get_time(&rnow);
	expiration = rnow.sec + seconds;
	
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX DPP_EVENT_INTRO "peer=" MACSTR
		" status=%u", MAC2STR(src), status[0]);


	wpa_printf(MSG_INFO1, DPP_MAP_PREX
		   "DPP: Try connection again after successful network introduction");
	if (wapp_set_pmk(wapp, src, intro.pmk, intro.pmk_len,
				intro.pmkid, expiration,
				WPA_KEY_MGMT_DPP, wdev) < 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to add PMKSA cache entry");
		return;
	}
	wdev_enable_apcli_iface(wapp, wdev, 1);

	return;
fail:
#ifdef MAP_R3
	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" [%s] \n", __func__);
	info_to_mapd->reason_id = PEER_DISC_FAIL;
	os_memcpy(info_to_mapd->reason, "Peer Discovery rsp fail", os_strlen("Peer Discovery rsp fail"));
	wapp_send_1905_msg(
			wapp,
			WAPP_SEND_USER_FAIL_NOTIF,
			sizeof(user_fail_reason),
			(char *)info_to_mapd);
	if (wapp->map->TurnKeyEnable) {
		cli_assoc_info.apcli_assoc_state = WAPP_APCLI_DISASSOCIATED;
		cli_assoc_info.interface_index = wdev->ifindex;
		wapp_send_1905_msg(wapp, WAPP_APCLI_ASSOC_STAT_CHANGE, sizeof(struct apcli_association_info),
						(char *)&cli_assoc_info);
	}
	if (auth)
		dpp_auth_deinit(auth);
	if(info_to_mapd)
		os_free(info_to_mapd);
#endif
	os_memset(&intro, 0, sizeof(intro));
}

static void
wapp_dpp_rx_pkex_exchange_req(struct wifi_app *wapp, struct wapp_dev *wdev,
				const u8 *src,
				 const u8 *buf, size_t len,
				 unsigned int chan)
{
	struct wpabuf *msg;

	if (!wdev) {
		return;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: PKEX Exchange Request from " MACSTR,
		   MAC2STR(src));

	/* TODO: Support multiple PKEX codes by iterating over all the enabled
	 * values here */

	if (!wapp->dpp->dpp_pkex_code || !wapp->dpp->dpp_pkex_bi) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: No PKEX code configured - ignore request");
		return;
	}

	if (wapp->dpp->dpp_pkex) {
		/* TODO: Support parallel operations */
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Already in PKEX session - ignore new request");
		return;
	}

	wapp->dpp->dpp_pkex = dpp_pkex_rx_exchange_req(wapp,
						  wapp->dpp->dpp_pkex_bi,
						  wdev->mac_addr, src,
						  wapp->dpp->dpp_pkex_identifier,
						  wapp->dpp->dpp_pkex_code,
						  buf, len);
	if (!wapp->dpp->dpp_pkex) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Failed to process the request - ignore it");
		return;
	}

	msg = wapp->dpp->dpp_pkex->exchange_resp;
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR
		" chan=%u type=%d", MAC2STR(src), chan,
		DPP_PA_PKEX_EXCHANGE_RESP);
	wapp_drv_send_action(wapp, wdev, chan, 0, src,
				wpabuf_head(msg), wpabuf_len(msg));
	if (wapp->dpp->dpp_pkex->failed) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Terminate PKEX exchange due to an earlier error");
		if (wapp->dpp->dpp_pkex->t > wapp->dpp->dpp_pkex->own_bi->pkex_t)
			wapp->dpp->dpp_pkex->own_bi->pkex_t = wapp->dpp->dpp_pkex->t;
		dpp_pkex_free(wapp->dpp->dpp_pkex);
		wapp->dpp->dpp_pkex = NULL;
	}
}


static void
wapp_dpp_rx_pkex_exchange_resp(struct wifi_app *wapp, struct wapp_dev *wdev, const u8 *src,
				  const u8 *buf, size_t len, unsigned int chan)
{
	struct wpabuf *msg;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: PKEX Exchange Response from " MACSTR,
		   MAC2STR(src));

	/* TODO: Support multiple PKEX codes by iterating over all the enabled
	 * values here */

	if (!wapp->dpp->dpp_pkex || !wapp->dpp->dpp_pkex->initiator ||
	    wapp->dpp->dpp_pkex->exchange_done) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No matching PKEX session");
		return;
	}

	msg = dpp_pkex_rx_exchange_resp(wapp->dpp->dpp_pkex, src, buf, len);
	if (!msg) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to process the response");
		return;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Send PKEX Commit-Reveal Request to " MACSTR,
		   MAC2STR(src));

	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR
		" chan=%u type=%d", MAC2STR(src), chan,
		DPP_PA_PKEX_COMMIT_REVEAL_REQ);
	wapp_drv_send_action(wapp, wdev, chan, 0, src,
				wpabuf_head(msg), wpabuf_len(msg));
	wpabuf_free(msg);
}


static void
wapp_dpp_rx_pkex_commit_reveal_req(struct wifi_app *wapp, const u8 *src,
				      const u8 *hdr, const u8 *buf, size_t len,
				      unsigned int chan)
{
	struct wpabuf *msg;
	struct dpp_pkex *pkex = wapp->dpp->dpp_pkex;
	struct dpp_bootstrap_info *bi;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: PKEX Commit-Reveal Request from " MACSTR,
		   MAC2STR(src));

	if (!pkex || pkex->initiator || !pkex->exchange_done) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No matching PKEX session");
		return;
	}

	msg = dpp_pkex_rx_commit_reveal_req(pkex, hdr, buf, len);
	if (!msg) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to process the request");
		if (wapp->dpp->dpp_pkex->failed) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Terminate PKEX exchange");
			if (wapp->dpp->dpp_pkex->t > wapp->dpp->dpp_pkex->own_bi->pkex_t)
				wapp->dpp->dpp_pkex->own_bi->pkex_t =
					wapp->dpp->dpp_pkex->t;
			dpp_pkex_free(wapp->dpp->dpp_pkex);
			wapp->dpp->dpp_pkex = NULL;
		}
		return;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Send PKEX Commit-Reveal Response to "
		   MACSTR, MAC2STR(src));

	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR
		" chan=%u type=%d", MAC2STR(src), chan,
		DPP_PA_PKEX_COMMIT_REVEAL_RESP);
	wapp_drv_send_action(wapp, wapp->dpp->dpp_pkex->wdev, chan, 0, src,
				wpabuf_head(msg), wpabuf_len(msg));
	wpabuf_free(msg);

	bi = dpp_pkex_finish(wapp->dpp, pkex, src, chan);
	if (!bi)
		return;
	wapp->dpp->dpp_pkex = NULL;
}


static void
wapp_dpp_rx_pkex_commit_reveal_resp(struct wifi_app *wapp, const u8 *src,
				       const u8 *hdr, const u8 *buf, size_t len,
				       unsigned int chan)
{
	int res;
	struct dpp_bootstrap_info *bi;
	struct dpp_pkex *pkex = wapp->dpp->dpp_pkex;
	char cmd[500];

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: PKEX Commit-Reveal Response from " MACSTR,
		   MAC2STR(src));

	if (!pkex || !pkex->initiator || !pkex->exchange_done) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No matching PKEX session");
		return;
	}

	res = dpp_pkex_rx_commit_reveal_resp(pkex, hdr, buf, len);
	if (res < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to process the response");
		return;
	}

	bi = dpp_pkex_finish(wapp->dpp, pkex, src, chan);
	if (!bi)
		return;
	wapp->dpp->dpp_pkex = NULL;

	res = os_snprintf(cmd, sizeof(cmd), " peer=%u %s",
		    bi->id,
		    wapp->dpp->dpp_pkex_auth_cmd ? wapp->dpp->dpp_pkex_auth_cmd : "");
	if (os_snprintf_error(sizeof(cmd), res)) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX"%s %d os_snprintf error\n", __func__, __LINE__);
		return;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		   "DPP: Start authentication after PKEX with parameters: %s",
		   cmd);
	if (pkex && wapp_dpp_auth_init(wapp, pkex->wdev, cmd) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Authentication initialization failed");
		return;
	}
}


int wapp_dpp_process_frames(struct wifi_app *wapp, struct wapp_dev *wdev, const u8 *src,
				const u8 *hdr, const u8 *buf, size_t len,  unsigned int chan,
				enum dpp_public_action_frame_type type)
{
	switch (type) {
	case DPP_PA_AUTHENTICATION_REQ:
		wapp_dpp_rx_auth_req(wapp, wdev, src, hdr, buf, len, chan);
		break;
	case DPP_PA_AUTHENTICATION_RESP:
		wapp_dpp_rx_auth_resp(wapp, src, hdr, buf, len, chan);
		break;
	case DPP_PA_AUTHENTICATION_CONF:
		wapp_dpp_rx_auth_conf(wapp, src, hdr, buf, len);
		break;
	case DPP_PA_PEER_DISCOVERY_REQ:
		wapp_dpp_rx_peer_disc_req(wapp, wdev, src, buf, len, chan);
		break;
	case DPP_PA_PEER_DISCOVERY_RESP:
		wapp_dpp_rx_peer_disc_resp(wapp, wdev, src, buf, len);
		break;
	case DPP_PA_PKEX_EXCHANGE_REQ:
		wapp_dpp_rx_pkex_exchange_req(wapp, wdev, src, buf, len, chan);
		break;
	case DPP_PA_PKEX_EXCHANGE_RESP:
		wapp_dpp_rx_pkex_exchange_resp(wapp, wdev, src, buf, len, chan);
		break;
	case DPP_PA_PKEX_COMMIT_REVEAL_REQ:
		wapp_dpp_rx_pkex_commit_reveal_req(wapp, src, hdr, buf, len,
						      chan);
		break;
	case DPP_PA_PKEX_COMMIT_REVEAL_RESP:
		wapp_dpp_rx_pkex_commit_reveal_resp(wapp, src, hdr, buf, len,
						       chan);
		break;
#ifdef DPP_R2_SUPPORT
	case DPP_PA_CONFIGURATION_RESULT:
		wapp_dpp_rx_conf_result(wapp, src, hdr, buf, len);
		break;
#endif /* DPP_R2_SUPPORT */

	case DPP_PA_PRESENCE_ANNOUNCEMENT:
#ifdef MAP_R3
		wapp_dpp_rx_presence_annouce(wapp, wdev, src, buf, len, chan);
#else
               wapp_dpp_rx_presence_annouce(wapp, wdev, src, buf, len, 0);
#endif /* MAP_R3 */
		break;
#ifdef DPP_R2_SUPPORT
case DPP_PA_CONNECTION_STATUS_RESULT:
		wapp_dpp_rx_conn_status_result(wapp, src, hdr, buf, len);
		break;
#endif /* DPP_R2_SUPPORT */

#ifdef DPP_R2_RECONFIG
	case DPP_PA_RECONFIG_ANNOUNCEMENT:
		wapp_dpp_rx_reconfig_annouce(wapp, wdev, src, buf, len
			, hdr , chan);
		break;
	case DPP_PA_RECONFIG_AUTH_REQ:
		wapp_dpp_rx_reconfig_auth_req(wapp, wdev, src, hdr, buf, len, chan);
		break;
	case DPP_PA_RECONFIG_AUTH_RESP:
#ifdef RECONFIG_OLD
		wapp_dpp_rx_reconfig_auth_resp(wapp, wdev, src, buf, len, chan);
#else
		wapp_dpp_rx_reconfig_auth_resp(wapp, wdev, src, hdr, buf, len, chan);
#endif
		break;
	case DPP_PA_RECONFIG_AUTH_CONF:
		wapp_dpp_rx_reconfig_auth_confirm(wapp, wdev, src, hdr, buf, len, chan);
		break;
#endif /* DPP_R2_RECONFIG */
	default:
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Ignored unsupported frame subtype %d", type);
		break;
	}

	return 0;
}

void wapp_dpp_rx_action(struct wifi_app *wapp, struct wapp_dev *wdev, const u8 *src,
			   const u8 *buf, size_t len, unsigned int chan)
{
	u8 crypto_suite;
	enum dpp_public_action_frame_type type;
	const u8 *hdr,*wlan_hdr;
	u8 da[MAC_ADDR_LEN];
	unsigned int pkex_t;
#ifdef MAP_R3
	struct dl_list *dev_list;
	struct wapp_dev *conf_wdev = NULL;
	BOOLEAN found = FALSE;
	struct ap_dev * ap = NULL;
#endif /* MAP_R3 */

	if (len < DPP_HDR_LEN)
		return;

	wlan_hdr = buf;
	os_memcpy(da, &wlan_hdr[4], MAC_ADDR_LEN);
	/* skipping the wlan header part, category and pub action type*/
	buf += 24 + 2;
	len -= (24 - 2);
		
	if (WPA_GET_BE24(buf) != OUI_WFA || buf[3] != DPP_OUI_TYPE)
		return;

	hdr = buf;
	buf += 4;
	len -= 4;
	crypto_suite = *buf++;
	type = *buf++;
	len -= 2;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		   "DPP: Received DPP Public Action frame crypto suite %u type %d from "
		   MACSTR " chan=%u\n",
		   crypto_suite, type, MAC2STR(src), chan);
	if (crypto_suite != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Unsupported crypto suite %u",
			   crypto_suite);
		wpa_printf(MSG_ERROR, DPP_MAP_PREX DPP_EVENT_RX "src=" MACSTR
			" chan=%u type=%d ignore=unsupported-crypto-suite",
			MAC2STR(src), chan, type);
		return;
	}

	if (is_broadcast_ether_addr(da) && ((type == DPP_PA_AUTHENTICATION_REQ)
		|| (type == DPP_PA_PRESENCE_ANNOUNCEMENT))){
#ifdef MAP_R3
		dev_list = &wapp->dev_list;
		dl_list_for_each(conf_wdev, dev_list, struct wapp_dev, list) {
			if (conf_wdev->radio && (conf_wdev->dev_type == WAPP_DEV_TYPE_AP)) {
				if ((conf_wdev->radio->op_ch == (u8)chan) && conf_wdev->i_am_fh_bss) {
					ap = (struct ap_dev *)conf_wdev->p_dev;
					if (ap && (ap->isActive == WAPP_BSS_STOP))
						continue;
					DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"[%s] HWAddr "MACSTR" \n",
					conf_wdev->ifname, PRINT_MAC(conf_wdev->mac_addr));
					wdev = conf_wdev;
					found = TRUE;
					break;
				}
			}
		}
		if (found == FALSE) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"wdev not found so retuning\n");
			return;
		}
#else
		wdev = wapp_get_dpp_default_wdev(wapp, chan);
#endif /* MAP_R3 */
		if (!wdev)
			return;
	}
	wpa_hexdump(MSG_MSGDUMP, DPP_MAP_PREX "DPP: Received message attributes", buf, len);
	if (dpp_check_attrs(buf, len) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX DPP_EVENT_RX "src=" MACSTR
			" chan=%u type=%d ignore=invalid-attributes",
			MAC2STR(src), chan, type);
		return;
	}
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_RX "src=" MACSTR
		" chan=%u type=%d", MAC2STR(src), chan, type);

#ifdef CONFIG_DPP2
#ifdef MAP_R3
	if (wapp->dpp->is_map && ((type != DPP_PA_PRESENCE_ANNOUNCEMENT)
		&& (type != DPP_PA_PEER_DISCOVERY_REQ) && (type != DPP_PA_PEER_DISCOVERY_RESP)
#ifdef MAP_R3_RECONFIG
		&& (type != DPP_PA_RECONFIG_ANNOUNCEMENT)
#endif
		)
		&& (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_PROXYAGENT)){
		
		dpp_relay_rx_action_map(wapp, src, hdr, 
			buf, len, chan, NULL, NULL, type);
			return;
	}
#else
	if (dpp_relay_rx_action(wapp->dpp,
				src, hdr, buf, len, chan, NULL, NULL) == 0)
		return;
#endif
#endif /* CONFIG_DPP2 */

	wapp_dpp_process_frames(wapp, wdev, src, hdr, buf, len, chan, type);

	if (wapp->dpp->dpp_pkex)
		pkex_t = wapp->dpp->dpp_pkex->t;
	else if (wapp->dpp->dpp_pkex_bi)
		pkex_t = wapp->dpp->dpp_pkex_bi->pkex_t;
	else
		pkex_t = 0;
	if (pkex_t >= PKEX_COUNTER_T_LIMIT) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_PKEX_T_LIMIT "id=0");
		wapp_dpp_pkex_remove(wapp, "*");
	}
}

struct wpabuf *
wapp_dpp_gas_req_handler(void *ctx, const u8 *sa,
			    const u8 *query, size_t query_len,
			    const u8 *data, size_t data_len)
{
	struct wifi_app *wapp = (struct wifi_app *)ctx;
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)sa);
	struct wpabuf *resp;

	wapp_dpp_cancel_timeouts(wapp,auth);
	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: GAS request from " MACSTR, MAC2STR(sa));
	if (!auth || (!auth->auth_success 
#ifdef MAP_R3_RECONFIG	
	&& !auth->reconfig_success
#endif	
		) || os_memcmp(sa, auth->peer_mac_addr, MAC_ADDR_LEN) != 0) {
#ifdef CONFIG_DPP2
		if (dpp_relay_rx_gas_req(wapp->dpp, sa, data,
				     data_len) == 0) {
			/* Response will be forwarded once received over TCP */
			return 0;
		}
#endif /* CONFIG_DPP2 */
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: No matching exchange in progress");
		return NULL;
	} 
#ifdef MAP_R3
	else if (auth->is_map_connection && (auth->allowed_roles == DPP_CAPAB_PROXYAGENT)
		&& (os_memcmp(sa, auth->peer_mac_addr, MAC_ADDR_LEN) == 0)) {
			if (dpp_relay_rx_gas_req_map(wapp, sa, data,
					     data_len) == 0) {
				/* Response will be forwarded once received over 1905*/
				return 0;
			}
		return 0;

	}
#endif

	if (auth->current_state != DPP_STATE_CONFIG_REQ_WAITING
#ifdef MAP_R3_RECONFIG
		&& (auth->current_state != DPP_STATE_RECONFIG_CONFIG_REQ_WAITING)
#endif
	) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Incorrect auth state=%d", auth->current_state);
		dpp_auth_deinit(auth);
		return NULL;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX
		    "DPP: Received Configuration Request (GAS Query Request)",
		    query, query_len);
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX DPP_EVENT_CONF_REQ_RX "src=" MACSTR,
		MAC2STR(sa));
	resp = dpp_conf_req_rx(auth, query, query_len);
	if (!resp) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONF_FAILED);
		dpp_auth_deinit(auth);
	}


	return resp;
}


void wapp_dpp_gas_status_handler(void *ctx, u8 *dst, int ok)
{
	struct wifi_app *wapp = (struct wifi_app *)ctx;
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, dst);

	if (!auth)
		return;

	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Configuration exchange completed (ok=%d)",
		   ok);
	wapp_dpp_cancel_timeouts(wapp,auth);
#if defined(DPP_R2_SUPPORT) && !defined(MAP_R3)
	if (ok && auth->peer_version >= 2 &&
	    auth->conf_resp_status == DPP_STATUS_OK && auth->current_state != DPP_STATE_CONFIG_DONE) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Wait for Configuration Result");
		auth->current_state = DPP_STATE_CONFIG_RESULT_WAITING;
		eloop_cancel_timeout(wapp_dpp_config_result_wait_timeout,
				     wapp, auth);
		eloop_register_timeout(2, 0, //kapil to do
				       wapp_dpp_config_result_wait_timeout,
				       wapp, auth);
		return;
	}
#endif /* DPP_R2_SUPPORT && !MAP_R3 */
	wapp_cancel_remain_on_channel(wapp, auth->wdev);

	if (ok)
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONF_SENT);
	else
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONF_FAILED);

#ifdef MAP_R3
	if(wapp->dpp->conf_res_received) {
		return;
	}
#endif /* MAP_R3 */

#ifdef CONFIG_DPP2
	if(auth && !auth->waiting_conn_status_result)
#endif /* CONFIG_DPP2 */
		dpp_auth_deinit(auth);
}

int wapp_dpp_configurator_sign(struct wifi_app *wapp, struct wapp_dev *wdev, const char *cmd)
{
	struct dpp_authentication *auth;
	int ret = -1;
	char *curve = NULL;
	int ap = 1;
#ifdef MAP_R3
	char *mode = NULL;
	int map = 0;
#endif /* MAP_R3 */

	auth = os_zalloc(sizeof(*auth));
	if (!auth)
		return -1;

#ifdef MAP_R3
	mode = get_param(cmd, "mode=");
	if(mode) {
		auth->msg_ctx = (void *) wapp;
		if (os_strncmp(mode, "map", 3) == 0){
			map = 1;
		}
	}
	else{
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"mode did not match\n");
		dpp_auth_deinit(auth);
		return -1;
	}
	auth->self_sign_auth = 1;
	if(wapp->dpp->dpp_map_cont_self) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"already self signed, no need again.\n");
		dpp_auth_deinit(auth);
		if(mode)
			  os_free(mode);

		return 0;
	}
#endif /* MAP_R3 */

	curve = get_param(cmd, " curve=");

#ifndef MAP_R3
	if (wdev->dev_type == WAPP_DEV_TYPE_STA)
		ap = 0;
#endif /* MAP_R3 */

	if (dpp_set_configurator(wapp->dpp, wapp,
				 auth, cmd) == 0 &&
	    dpp_configurator_own_config(auth, curve, ap
#ifdef MAP_R3
		, map
#endif /* MAP_R3 */	
	) == 0) {
		auth->wdev = wdev;
		auth->config_wdev = wdev;
#ifdef DPP_R2_MUOBJ
		ret = wapp_dpp_handle_config_obj(wapp, auth, &auth->conf_obj[0]);
#else
		ret = wapp_dpp_handle_config_obj(wapp, auth);
#endif  /* DPP_R2_MUOBJ */
		if(ret == 0)
			wapp->dpp->dpp_map_cont_self = TRUE;
	}

#ifdef MAP_R3
	wapp_snd_bss_conn_1905(wapp ,auth);
	auth->peer_protocol_key = NULL;
	if(wapp_dpp_auth_list_insert(wapp, auth) < 0) {
		dpp_auth_deinit(auth);
		os_free(curve);
		os_free(mode);
		DBGPRINT(RT_DEBUG_OFF, "dpp auth list insert fail \n");
		return -1;
	}
#endif /* MAP_R3 */

#ifdef DPP_R2_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"%s()  config result:%d, pre_status :%d\n", __func__,
		ret, wapp->dpp->annouce_enrolle.pre_status);
	if(!ret && wapp->dpp->annouce_enrolle.pre_status == DPP_PRE_STATUS_WAIT_AUTHREQ) {
		struct dpp_bootstrap_info *bi = NULL;

		bi = dpp_get_own_bi(wapp->dpp);
		if (bi)
			eloop_cancel_timeout(wapp_dpp_presence_auth_wait_timeout, wapp, bi);

		wapp->dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_SUCCESS;
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"%s() cancel the presence frame \n", __func__);
	}
#endif /* DPP_R2_SUPPORT */

#ifndef MAP_R3
	/* For MAP-R3 controller deinit auth after the BSS configurations are done */
	dpp_auth_deinit(auth);
#endif /* MAP_R3 */
	os_free(curve);
#ifdef MAP_R3
	os_free(mode);
#endif /* MAP_R3 */
	
	return ret;
}

static void wapp_gas_server_tx(void *ctx, struct wapp_dev *wdev,
				int chan, const u8 *da,
                               struct wpabuf *buf, unsigned int wait_time)
{
        struct wifi_app *wapp = ctx;

	wapp_drv_send_action(wapp, wdev, chan, wait_time,
			       da,
			       wpabuf_head(buf),
			       wpabuf_len(buf));
}

int wapp_dpp_gas_server_init(struct wifi_app *wapp)
{
	if (!wapp->dpp)
		return -1;

	wapp->dpp->gas_server = gas_server_init(wapp, wapp_gas_server_tx);
	u8 adv_proto_id[7];

	adv_proto_id[0] = WLAN_EID_VENDOR_SPECIFIC;
	adv_proto_id[1] = 5;
	WPA_PUT_BE24(&adv_proto_id[2], OUI_WFA);
	adv_proto_id[5] = DPP_OUI_TYPE;
	adv_proto_id[6] = 0x01;
	if (gas_server_register(wapp->dpp->gas_server, adv_proto_id,
				sizeof(adv_proto_id), wapp_dpp_gas_req_handler,
				wapp_dpp_gas_status_handler, wapp) < 0)
		return -1;
	return 0;
}

void wapp_ap_dpp_deinit(struct wifi_app *wapp)
{
	dpp_auth_list_deinit(wapp);
	wapp_dpp_pkex_remove(wapp, "*");
	wapp->dpp->dpp_pkex = NULL;
}

#ifdef CONFIG_DPP2
int wapp_dpp_controller_start(struct wifi_app *wapp, const char *cmd)
{
        const char *pos;

        if (cmd) {
                pos = os_strstr(cmd, " tcp_port=");
                if (pos) {
                        pos += 10;
			wapp->dpp->tcp_port = atoi(pos);
                }
        }
        return dpp_controller_start(wapp->dpp);
}
#endif

int wapp_dpp_pkex_add(struct wifi_app *wapp,  struct wapp_dev *wdev, const char *cmd)
{
	struct dpp_bootstrap_info *own_bi;
	const char *pos, *end;
	unsigned int wait_time;

	pos = os_strstr(cmd, " own=");
	if (!pos)
		return -1;
	pos += 5;
	own_bi = dpp_bootstrap_get_id(wapp->dpp, atoi(pos));
	if (!own_bi) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Identified bootstrap info not found");
		return -1;
	}
	if (own_bi->type != DPP_BOOTSTRAP_PKEX) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Identified bootstrap info not for PKEX");
		return -1;
	}
	wapp->dpp->dpp_pkex_bi = own_bi;
	own_bi->pkex_t = 0; /* clear pending errors on new code */

	os_free(wapp->dpp->dpp_pkex_identifier);
	wapp->dpp->dpp_pkex_identifier = NULL;

	pos = os_strstr(cmd, " identifier=");
	if (pos) {
		pos += 12;
		end = os_strchr(pos, ' ');
		if (!end)
			return -1;
		wapp->dpp->dpp_pkex_identifier = os_malloc(end - pos + 1);
		if (!wapp->dpp->dpp_pkex_identifier)
			return -1;
		os_memcpy(wapp->dpp->dpp_pkex_identifier, pos, end - pos);
		wapp->dpp->dpp_pkex_identifier[end - pos] = '\0';
	}

	pos = os_strstr(cmd, " code=");
	if (!pos)
		return -1;
	os_free(wapp->dpp->dpp_pkex_code);
	wapp->dpp->dpp_pkex_code = os_strdup(pos + 6);
	if (!wapp->dpp->dpp_pkex_code)
		return -1;

	if (os_strstr(cmd, " init=1")) {
		struct dpp_pkex *pkex;
		struct wpabuf *msg;

		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Initiating PKEX");
		dpp_pkex_free(wapp->dpp->dpp_pkex);
		wapp->dpp->dpp_pkex = dpp_pkex_init(wapp, own_bi, wdev->mac_addr,
						wapp->dpp->dpp_pkex_identifier,
						wapp->dpp->dpp_pkex_code);
		pkex = wapp->dpp->dpp_pkex;
		if (!pkex)
			return -1;

		msg = pkex->exchange_req;
		wait_time = wapp->dpp->max_remain_on_chan;
		if (wait_time > 2000)
			wait_time = 2000;
		pkex->chan = 2437;
		pkex->wdev = wdev;
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR
			" chan=%u type=%d",
			MAC2STR(broadcast), pkex->chan,
			DPP_PA_PKEX_EXCHANGE_REQ);
		wapp_drv_send_action(wapp, wdev, pkex->chan, wait_time,
 					broadcast,
					wpabuf_head(msg),
					wpabuf_len(msg));

		if (wait_time == 0)
			wait_time = 2000;
		pkex->exch_req_wait_time = wait_time;
		pkex->exch_req_tries = 1;
	}

	/* TODO: Support multiple PKEX info entries */

	os_free(wapp->dpp->dpp_pkex_auth_cmd);
	wapp->dpp->dpp_pkex_auth_cmd = os_strdup(cmd);

	return 1;
}


int wapp_dpp_pkex_remove(struct wifi_app *wapp, const char *id)
{
	unsigned int id_val;

	if (os_strcmp(id, "*") == 0) {
		id_val = 0;
	} else {
		id_val = atoi(id);
		if (id_val == 0)
			return -1;
	}

	if ((id_val != 0 && id_val != 1) || !wapp->dpp->dpp_pkex_code)
		return -1;

	/* TODO: Support multiple PKEX entries */
	os_free(wapp->dpp->dpp_pkex_code);
	wapp->dpp->dpp_pkex_code = NULL;
	os_free(wapp->dpp->dpp_pkex_identifier);
	wapp->dpp->dpp_pkex_identifier = NULL;
	os_free(wapp->dpp->dpp_pkex_auth_cmd);
	wapp->dpp->dpp_pkex_auth_cmd = NULL;
	wapp->dpp->dpp_pkex_bi = NULL;
	/* TODO: Remove dpp_pkex only if it is for the identified PKEX code */
	dpp_pkex_free(wapp->dpp->dpp_pkex);
	wapp->dpp->dpp_pkex = NULL;
	return 0;
}


void wapp_dpp_stop(struct wifi_app *wapp)
{
	dpp_auth_list_deinit(wapp);
	dpp_pkex_free(wapp->dpp->dpp_pkex);
	wapp->dpp->dpp_pkex = NULL;
	if (wapp->dpp->dpp_gas_dialog_token >= 0)
		gas_query_stop(wapp->dpp->gas_query_ctx, wapp->dpp->dpp_gas_dialog_token);
}

static void wapp_dpp_auth_resp_retry_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = timeout_ctx;

	if (!auth || !auth->resp_msg)
		return;

	wpa_printf(MSG_INFO1, DPP_MAP_PREX
		   "DPP: Retry Authentication Response after timeout");
#ifdef MAP_R3
	dpp_auth_fail_wrapper(wapp, "DPP: Retry Authentication Response after timeout");
#endif
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR
		" chan=%u type=%d",
		MAC2STR(auth->peer_mac_addr), auth->curr_chan,
		DPP_PA_AUTHENTICATION_RESP);
	auth->current_state = DPP_STATE_AUTH_CONF_WAITING;
	wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 500, auth->peer_mac_addr,
				wpabuf_head(auth->resp_msg), wpabuf_len(auth->resp_msg));

	if(!wapp->dpp->dpp_qr_mutual)
	{
		eloop_cancel_timeout(wapp_dpp_auth_conf_wait_timeout, wapp, auth);
		eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_auth_conf_wait_timeout, wapp, auth);
	}
}

static void wapp_dpp_auth_resp_retry(struct wifi_app *wapp, struct dpp_authentication *auth)
{
	unsigned int wait_time, max_tries;

	if (!auth || !auth->resp_msg)
		return;

	if (wapp->dpp->dpp_resp_max_tries)
		max_tries = wapp->dpp->dpp_resp_max_tries;
	else
		max_tries = 5;
	auth->auth_resp_tries++;
	if (auth->auth_resp_tries >= max_tries) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: No confirm received from initiator - stopping exchange");
		dpp_auth_deinit(auth);
		return;
	}

	if (wapp->dpp->dpp_resp_retry_time)
		wait_time = wapp->dpp->dpp_resp_retry_time;
	else
		wait_time = 10000;
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		   "DPP: Schedule retransmission of Authentication Response frame in %u ms",
		wait_time);
	eloop_cancel_timeout(wapp_dpp_auth_resp_retry_timeout, wapp, auth);
	eloop_register_timeout(wait_time / 1000,
			       (wait_time % 1000) * 1000,
			       wapp_dpp_auth_resp_retry_timeout, wapp, auth);
}

int wpa_start_roc(struct wifi_app *wapp, unsigned int chan, struct wapp_radio *radio, unsigned int wait_time)
{
	struct wapp_dev *target_wdev = NULL, *wdev = NULL;
	struct dl_list *dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev->radio == radio && wdev->dev_type == WAPP_DEV_TYPE_STA) {
			target_wdev = wdev;
			break;
		}
	}

	if (target_wdev)
        	wapp->drv_ops->drv_start_roc(wapp->drv_data, wdev->ifname, chan, wait_time);
	return 0;
}

static int wapp_dpp_listen_start(struct wifi_app *wapp, struct wapp_dev *wdev,
		unsigned int channel)
{
	if (!((IS_MAP_CH_24G(channel) && IS_MAP_CH_24G(wdev->radio->op_ch)) ||
	      (IS_MAP_CH_5GL(channel) && IS_MAP_CH_5GL(wdev->radio->op_ch)) ||
	      (IS_MAP_CH_5GH(channel) && IS_MAP_CH_5GH(wdev->radio->op_ch)))) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "Invalid channel %d interface combo",
			   channel);
		return -1;
	}
	wdev_set_quick_ch(wapp, wdev, channel);

	/* TODO should these also be band specific?? */
	wapp->dpp->dpp_listen_chan = channel;

	return 0;
}

int wapp_dpp_listen(struct wifi_app *wapp, struct wapp_dev *wdev, const char *cmd)
{
	int chan;

	chan = atoi(cmd);
	if (chan <= 0)
		return -1;

	wapp->dpp->dpp_qr_mutual = os_strstr(cmd, " qr=mutual") != NULL;

	wdev = wapp_get_dpp_default_wdev(wapp, chan);
	if (!wdev)
		return -1;

	if (wapp->dpp->dpp_listen_chan == (unsigned int) chan) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Already listening on %u MHz",
			   chan);
		return 0;
	}

	return wapp_dpp_listen_start(wapp, wdev, chan);
}

void wapp_dpp_listen_stop(struct wifi_app *wapp, struct wapp_dev *wdev)
{
	if (!wapp->dpp || !wapp->dpp->dpp_listen_chan)
		return;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Stop listen on %u MHz",
		   wapp->dpp->dpp_listen_chan);
	wapp_cancel_remain_on_channel(wapp, wdev);
	wapp->dpp->dpp_listen_chan = 0;
}

#ifdef MAP_R3
int wapp_dpp_ch_validation(struct wifi_app *wapp, u8 ch)
{
	struct dl_list *dev_list;
	struct wapp_dev *wdev = NULL;
	int i=0, j=0;

	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
		if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
			struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
#ifndef MAP_6E_SUPPORT
			wdev_op_class_info *op_class = &ap->op_class;

			for(i = 0; i < op_class->num_of_op_class; i++) {
				if(op_class->opClassInfo[i].op_class != 81
						&& op_class->opClassInfo[i].op_class != 115
						&& op_class->opClassInfo[i].op_class != 118
						&& op_class->opClassInfo[i].op_class != 121
						&& op_class->opClassInfo[i].op_class != 125)
					continue;

				for(j = 0; j < op_class->opClassInfo[i].num_of_ch; j++) {
					if(ch == op_class->opClassInfo[i].ch_list[j]) {
						return 1;
					}
				}
			}
#else
			struct _wdev_op_class_info_ext *op_class = &ap->op_class;

			for (i = 0; i < op_class->num_of_op_class; i++) {
				if (op_class->opClassInfoExt[i].op_class != 81
						&& op_class->opClassInfoExt[i].op_class != 115
						&& op_class->opClassInfoExt[i].op_class != 118
						&& op_class->opClassInfoExt[i].op_class != 121
						&& op_class->opClassInfoExt[i].op_class != 125)
					continue;

				for (j = 0; j < op_class->opClassInfoExt[i].num_of_ch; j++) {
					if (ch == op_class->opClassInfoExt[i].ch_list[j])
						return 1;
				}
			}
#endif
		}
	}
	return 0;
}

int wapp_dpp_ch_chirp_valid(struct wifi_app *wapp, u8 ch)
{
	int i;
	if (!wapp || !wapp->dpp)
		return 0;
	for (i = 0; i < wapp->dpp->chirp_add_ch_num; i++) {
		if (ch == wapp->dpp->chirp_list_add[i])
			return 1;
	}
	for (i = 0; i < wapp->dpp->chirp_del_ch_num; i++) {
		if (ch == wapp->dpp->chirp_list_del[i])
			return -1;
	}
	return 0;
}
int wapp_dpp_ch_chirp_valid_del(u8 *ch_list, u8 ch, u8 tot_num)
{
	int i, k = 0;
	u8 total_channel[64] = {0};
	if (!ch_list)
		return 0;
	os_memcpy(total_channel, ch_list, tot_num);
	/*os_free(ch_list);
	ch_list = os_zalloc(tot_num - 1);
	if (!ch_list)
		return 0;*/
	os_memset(ch_list, 0, tot_num);
	for (i = 0; i < tot_num; i++) {
		if (ch != total_channel[i]) {
			ch_list[k++] = total_channel[i];
		}
	}
	return k;
}

int wapp_dpp_presence_announce_stop(struct wifi_app *wapp)
{
	struct dpp_global *dpp = NULL;
        struct dpp_bootstrap_info *own_bi =NULL;

        if(! wapp ||!wapp->dpp)
                return -1;

        dpp = wapp->dpp;

#ifdef MAP_R3
	/* When version check will be present use this*/
	if (wapp->map && wapp->map->map_version != DEV_TYPE_R3) {
		wapp_version_mismatch(wapp);
		return -1;
	}
#endif
	if(!dpp->chirp_ongoing) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"chirp is not ongoing, No need of cmd\n");
		return -1;
	}

        own_bi = dpp_get_own_bi(dpp);
        if (!own_bi) {
                DBGPRINT(RT_DEBUG_OFF,
                           DPP_MAP_PREX"DPP: Could not find local bootstrapping info ");
                return -1;
        }
        eloop_cancel_timeout(wapp_dpp_presence_auth_wait_timeout, wapp, own_bi);
        dpp->annouce_enrolle.presence_retry = 0;
        dpp->annouce_enrolle.cur_presence_chan_id = 0;
        dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_INIT;
	dpp->chirp_ongoing = 0;
	dpp->chirp_stop_done = 1;
	wapp->dpp->cce_driver_scan_done = 0;
        DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: presence annoucement stopped\n");
#ifdef MAP_6E_SUPPORT
	dpp->annouce_enrolle.cur_presence_chan_id_6g = 0;
#endif

	return 0;
}

int wapp_dpp_cce_indication(struct wifi_app *wapp, const char *cmd)
{
	unsigned int flag = 0;
	const char *pos = NULL;
	u8 almac[MAC_ADDR_LEN];
#ifdef MAP_R3
/* When version check will be present use this*/
	if (wapp->map && wapp->map->map_version != DEV_TYPE_R3) {
		wapp_version_mismatch(wapp);
		return -1;
	}
#endif
	pos = os_strstr(cmd, "flag=");
	if (!pos) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"flag arg not found in cmd\n");
		return -1;
	}
	pos += 5;
	
	flag = atoi(pos);
	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"val of flag:%d",flag);

	pos = os_strstr(cmd, "almac=");
	if (!pos) {
		DBGPRINT(RT_DEBUG_ERROR,DPP_MAP_PREX"MAC null, broadcast it!!\n");
		CCEIndication_1905_send(wapp, NULL, (unsigned char)flag);
		if (flag == 1)
			wapp_dpp_set_ie(wapp);
		else
			wapp_dpp_reset_cce_ie(wapp);
	} else {
		pos += 6;
		int ret = hwaddr_aton((const char *)pos, almac);

		if (ret)
			return -1;

		if (wapp->map &&
		os_strcmp((char *)almac, (char *)wapp->map->agnt_alid) == 0) {
			if (flag == 1)
				wapp_dpp_set_ie(wapp);
			else
				wapp_dpp_reset_cce_ie(wapp);
		} else
			CCEIndication_1905_send(wapp, almac, (unsigned char)flag);
	}
	return 0;
}

int dpp_send_chirp_notif_wrapper(struct wifi_app *wapp, const char *cmd)
{
	unsigned char peer_mac[ETH_ALEN] = {0};
	unsigned char al_mac[ETH_ALEN] = {0};
	struct dpp_authentication *auth = NULL;
	const char * buf = cmd;
	unsigned int flag = 0;
	unsigned char hash_validity = atoi(buf);

	buf += 2;
	flag = atoi(buf);
	buf += 2;
	if (hwaddr_aton((const char *)buf, peer_mac) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX "hwaddr_aton error %d", __LINE__);
		return -1;
	}
	if (flag == 1) {
		buf += 3*ETH_ALEN;
		if (hwaddr_aton((const char *)buf, al_mac) < 0) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX "hwaddr_aton error %d", __LINE__);
			return -1;
		}
	} else {
		os_memcpy(al_mac, broadcast, ETH_ALEN);
	}
	wpa_hexdump(MSG_INFO1, DPP_MAP_PREX "chirp peer addr", peer_mac, ETH_ALEN);
	wpa_hexdump(MSG_INFO1, DPP_MAP_PREX "almac peer addr", al_mac, ETH_ALEN);
	auth = wapp_dpp_get_auth_from_peer_mac(wapp, peer_mac);
	if (auth) {
		wpa_hexdump(MSG_INFO1, DPP_MAP_PREX
			    "DPP: Expected Responder Bootstrapping Key Hash",
			    auth->peer_mac_addr, ETH_ALEN);
		if (os_memcmp(auth->peer_mac_addr, peer_mac, ETH_ALEN) == 0) {
			ChirpMsg_1905_send(wapp, auth->chirp_tlv.hash_payload, peer_mac
#ifdef MAP_R3_RECONFIG
					, 0
#endif
			, hash_validity, al_mac);
		}
	}
	return 0;
}

int dpp_send_chirp_en_disable_wrapper(struct wifi_app *wapp, const char *cmd)
{
	
	unsigned int flag = 0, i = 0, k;
	const char *pos = NULL;
	u8 channel[MAX_NUM_OF_CHANNELS];
	char *token;
#ifdef MAP_R3
/* When version check will be present use this*/
	if (wapp->map && wapp->map->map_version != DEV_TYPE_R3) {
		wapp_version_mismatch(wapp);
		return -1;
	}
#endif
	pos = os_strstr(cmd, "en=");
	if (!pos) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"flag arg not found in cmd\n");
		return -1;
	}
	pos += 3;
	
	flag = atoi(pos);
	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"val of flag:%d\n",flag);
	pos = os_strstr(cmd, "chlist=");
	if (!pos) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Ch list null, broadcast it!!\n");
		// Send on all channels
	} else {
		pos += 7;
		token = strtok((char *)pos, ":");	
		while (token != NULL) {
			if (wapp_dpp_ch_validation(wapp, atoi(token)))
				channel[i++] = atoi(token);
			token = strtok(NULL, ":");
		}
		// Send on channel list
		if (i > 0) {
			if (flag == 1) {
				if (wapp->dpp->chirp_list_add)
					os_free(wapp->dpp->chirp_list_add);
				wapp->dpp->chirp_list_add = os_zalloc(i); // Number of channels
				if(!wapp->dpp->chirp_list_add)
					return -1;
				for(k = 0;k < i;k++) {
					if (wapp_dpp_ch_chirp_valid(wapp, channel[k]) == -1)
						wapp->dpp->chirp_del_ch_num = wapp_dpp_ch_chirp_valid_del(wapp->dpp->chirp_list_del, channel[k], wapp->dpp->chirp_del_ch_num);
				}
				os_memcpy(wapp->dpp->chirp_list_add, channel, i);
				wapp->dpp->chirp_add_ch_num = i;
			} else if (flag == 0) {
				if (wapp->dpp->chirp_list_del)
					os_free(wapp->dpp->chirp_list_del);
				wapp->dpp->chirp_list_del = os_zalloc(i); // Number of channels
				if(!wapp->dpp->chirp_list_del)
					return -1;
				for(k = 0;k < i;k++) {
					if (wapp_dpp_ch_chirp_valid(wapp, channel[k]) == 1)
						wapp->dpp->chirp_add_ch_num = wapp_dpp_ch_chirp_valid_del(wapp->dpp->chirp_list_add, channel[k], wapp->dpp->chirp_add_ch_num);
				}
				os_memcpy(wapp->dpp->chirp_list_del, channel, i);
				wapp->dpp->chirp_del_ch_num = i;
			}
		}
	}
	return 0;
}

int dpp_send_onboarding_type(struct wifi_app *wapp, const char *cmd)
{
	char type = 0;
	const char *pos = NULL;
#ifdef MAP_R3
/* When version check will be present use this*/
	if (wapp->map && wapp->map->map_version != DEV_TYPE_R3) {
		wapp_version_mismatch(wapp);
		return -1;
	}
#endif
	pos = os_strstr(cmd, "type=");
	if (!pos) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"flag arg not found in cmd\n");
		return -1;
	}
	pos += 5;
	
	type = atoi(pos);
	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"val of flag:%d\n",type);
	wapp->dpp->onboarding_type = type;
	/* Update onboarding type from cmd to driver and 1905 */
	wapp_dpp_set_onboarding_type(wapp, wapp->dpp->onboarding_type);
	wapp_send_1905_msg(
			wapp,
			WAPP_SEND_ONBOARD_TYPE,
			sizeof(char),
			(char *)&type);
	if (type != 0)
		CCEIndication_1905_send(wapp, NULL, 0);
	return 0;
}



#ifdef MAP_R3_RECONFIG
int map_dpp_send_reconfig_announce(struct wifi_app *wapp, const u8 *msg, const u8 *src, size_t msglen
					, const u8 *hdr)
{
	struct dpp_msg *dpp_pkt = NULL;
	int len = 0;
	struct wpabuf *msg_out = NULL;
	size_t msg_out_pos = 0;

	msg_out = dpp_map_encaps(hdr, msg, msglen);	
	if (!msg_out) {
		return 0;
	}
	len = sizeof (struct dpp_msg) + wpabuf_len(msg_out) - msg_out_pos;
		
	dpp_pkt = os_zalloc(len);

	if (os_memcmp(dpp_pkt->almac, wapp->dpp->almac_cont, MAC_ADDR_LEN) == 0) {
		unsigned char almac_broad_cont[6] = {0x00, 0x0c, 0x43, 0x49, 0xa1, 0x5b};

		/* Hardcoded fix for Wifi Proxy Topology*/
		os_memcpy(dpp_pkt->almac, almac_broad_cont, MAC_ADDR_LEN);
	} else
		os_memcpy(dpp_pkt->almac, wapp->dpp->almac_cont, MAC_ADDR_LEN);

	dpp_pkt->dpp_frame_indicator = 0;
	dpp_pkt->chirp_tlv_present = 0;
						
	dpp_pkt->frame_type = DPP_PA_RECONFIG_ANNOUNCEMENT;
	
	dpp_pkt->dpp_info.chn_list_flag = 0;
	dpp_pkt->dpp_info.chn_list_flag = 0;
	dpp_pkt->dpp_info.enrollee_mac_flag = 1;
	os_memcpy(dpp_pkt->dpp_info.enrollee_mac, src, MAC_ADDR_LEN);
		
	
	dpp_pkt->payload_len = wpabuf_len(msg_out) - msg_out_pos;
	os_memcpy(dpp_pkt->payload, wpabuf_head_u8(msg_out) + msg_out_pos, dpp_pkt->payload_len);
	
	hex_dump_dbg(DPP_MAP_PREX"dpp dump= ",(UCHAR *)dpp_pkt->payload, dpp_pkt->payload_len);
	hex_dump_dbg(DPP_MAP_PREX"total dump= ",(UCHAR *)dpp_pkt, len);
	if (wapp_send_1905_msg(wapp, WAPP_SEND_DPP_MSG,
		len , (char *)dpp_pkt) < 0)
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed\n");
	
	os_free(dpp_pkt);
	if(msg_out)
		os_free(msg_out);
	
	return 0;
}


#endif



struct dpp_sec_cred * wapp_dpp_save_dpp_cred(struct dpp_authentication *auth)
{
	struct dpp_sec_cred * sec_cred = NULL;
	unsigned char * cred_buffer = NULL;
	unsigned char * pos = NULL;
	unsigned short cred_len=0;
                        
#if 0
	if (auth->connector) {
		sec_cred->connector = os_strdup(auth->connector);
		if (!sec_cred->connector)
			goto fail;
		sec_cred->connector_len = os_strlen(sec_cred->connector);
	}

	if (auth->c_sign_key) {
		sec_cred->csign = os_malloc(wpabuf_len(auth->c_sign_key));
		if (!sec_cred->csign)
			goto fail;
		os_memcpy(sec_cred->csign, wpabuf_head(auth->c_sign_key),
			  wpabuf_len(auth->c_sign_key));
		sec_cred->csign_len = wpabuf_len(auth->c_sign_key);
	}

	if (auth->net_access_key) {
		sec_cred->netaccesskey =
			os_malloc(wpabuf_len(auth->net_access_key));
		if (!sec_cred->netaccesskey)
			goto fail;
		os_memcpy(sec_cred->netaccesskey,
			  wpabuf_head(auth->net_access_key),
			  wpabuf_len(auth->net_access_key));
		sec_cred->netaccesskey_len = wpabuf_len(auth->net_access_key);
		sec_cred->netaccesskey_expiry = auth->net_access_key_expiry;
	}
#endif
	if (!auth->c_sign_key)
		goto fail;
	if (!auth->connector)
		goto fail;
	cred_len = os_strlen(auth->connector) + wpabuf_len(auth->c_sign_key) + wpabuf_len(auth->net_access_key);

	sec_cred = os_zalloc(sizeof(*sec_cred) + cred_len);
	if(!sec_cred)
		goto fail;

	sec_cred->payload_len = cred_len;
	sec_cred->connector_len = (unsigned short)os_strlen(auth->connector);
	sec_cred->csign_len = (unsigned short)wpabuf_len(auth->c_sign_key);
	sec_cred->netaccesskey_len = (unsigned short)wpabuf_len(auth->net_access_key);
	sec_cred->netaccesskey_expiry = auth->net_access_key_expiry;
	sec_cred->decrypt_thresold = auth->decrypt_thresold;
	
	cred_buffer = os_zalloc(sec_cred->payload_len);
	if(!cred_buffer)
		goto fail;
	
	pos = cred_buffer;	
	os_memcpy(cred_buffer, auth->connector, sec_cred->connector_len);
	cred_buffer += sec_cred->connector_len;
	os_memcpy(cred_buffer, wpabuf_head(auth->c_sign_key), sec_cred->csign_len);
	cred_buffer += sec_cred->csign_len;
	os_memcpy(cred_buffer, wpabuf_head(auth->net_access_key), sec_cred->netaccesskey_len);

	os_memcpy(sec_cred->payload, pos, sec_cred->payload_len);
        if(pos)
	{
		os_free(pos);
		pos=NULL;
		cred_buffer=NULL;
	}
	return sec_cred;
fail:
	if(sec_cred)
		os_free(sec_cred);
	return NULL;
}

static void wapp_dpp_clear_cred(struct dpp_authentication *auth)
{
	//clearing existing configurations from auth
	os_memset(auth->ssid, 0, SSID_MAX_LEN);
	os_memset(auth->passphrase, 0, 64);
	auth->ssid_len = 0;
}

#endif /* MAP_R3 */

#ifdef DPP_R2_MUOBJ
static struct dpp_config * wapp_dpp_add_network(struct wifi_app *wapp,
						struct wapp_dev *wdev,
					      struct dpp_authentication *auth, struct dpp_config_obj *conf)
{
	struct dpp_config *wdev_config = os_zalloc(sizeof(*wdev_config));

	wdev_config->ssid = os_malloc(conf->ssid_len + 1);
	if (!wdev_config->ssid)
		goto fail;
	os_memcpy(wdev_config->ssid, conf->ssid, conf->ssid_len);
	wdev_config->ssid[conf->ssid_len] = '\0';
	wdev_config->ssid_len = conf->ssid_len;

	if (conf->connector) {
		wdev_config->key_mgmt = WPA_KEY_MGMT_DPP;
		wdev_config->dpp_connector = os_strdup(conf->connector);
		if (!wdev_config->dpp_connector)
			goto fail;
	}

	if (auth->c_sign_key) {
		wdev_config->dpp_csign = os_malloc(wpabuf_len(auth->c_sign_key));
		if (!wdev_config->dpp_csign)
			goto fail;
		os_memcpy(wdev_config->dpp_csign, wpabuf_head(auth->c_sign_key),
			  wpabuf_len(auth->c_sign_key));
		wdev_config->dpp_csign_len = wpabuf_len(auth->c_sign_key);
	}

	if (auth->net_access_key) {
		wdev_config->dpp_netaccesskey =
			os_malloc(wpabuf_len(auth->net_access_key));
		if (!wdev_config->dpp_netaccesskey)
			goto fail;
		os_memcpy(wdev_config->dpp_netaccesskey,
			  wpabuf_head(auth->net_access_key),
			  wpabuf_len(auth->net_access_key));
		wdev_config->dpp_netaccesskey_len = wpabuf_len(auth->net_access_key);
		wdev_config->dpp_netaccesskey_expiry = auth->net_access_key_expiry;
	}

	if (!conf->connector || dpp_akm_psk(conf->akm) ||
	    dpp_akm_sae(conf->akm)) {
		if (!conf->connector)
			wdev_config->key_mgmt = 0;
		if (dpp_akm_psk(conf->akm))
			wdev_config->key_mgmt |= WPA_KEY_MGMT_PSK |
				WPA_KEY_MGMT_PSK_SHA256 | WPA_KEY_MGMT_FT_PSK;
		if (dpp_akm_sae(conf->akm))
			wdev_config->key_mgmt |= WPA_KEY_MGMT_SAE |
				WPA_KEY_MGMT_FT_SAE;
#if 0 // Kapil not needed atm
		if (auth->passphrase[0]) {
			if (wpa_config_set_quoted(ssid, "psk",
						  auth->passphrase) < 0)
				goto fail;
			wpa_config_update_psk(ssid);
			ssid->export_keys = 1;
		} else
#endif
		{
			wdev_config->psk_set = conf->psk_set;
			os_memcpy(wdev_config->psk, conf->psk, PMK_LEN);
		}
	}

	if (wdev && wdev->config) {
		os_free(wdev->config->dpp_connector);	
		os_free(wdev->config->dpp_netaccesskey);	
		os_free(wdev->config->dpp_csign);	
		os_free(wdev->config->ssid);	
		os_free(wdev->config);
		wdev->config = NULL;
	}

	if(wdev)
	/* Set this config in ongoing wdev */
	wdev->config = wdev_config;

	return wdev_config;
fail:
	return NULL;
}

#else

static struct dpp_config * wapp_dpp_add_network(struct wifi_app *wapp,
						struct wapp_dev *wdev,
					      struct dpp_authentication *auth)
{
	struct dpp_config *wdev_config = os_zalloc(sizeof(*wdev_config));

	if (!wdev_config)
		goto fail;

	wdev_config->ssid = os_malloc(auth->ssid_len + 1);
	if (!wdev_config->ssid)
		goto fail;
	os_memcpy(wdev_config->ssid, auth->ssid, auth->ssid_len);
	wdev_config->ssid[auth->ssid_len] = '\0';
	wdev_config->ssid_len = auth->ssid_len;

	if (auth->connector) {
		wdev_config->key_mgmt = WPA_KEY_MGMT_DPP;
		wdev_config->dpp_connector = os_strdup(auth->connector);
		if (!wdev_config->dpp_connector)
			goto fail;
	}

	if (auth->c_sign_key) {
		wdev_config->dpp_csign = os_malloc(wpabuf_len(auth->c_sign_key));
		if (!wdev_config->dpp_csign)
			goto fail;
		os_memcpy(wdev_config->dpp_csign, wpabuf_head(auth->c_sign_key),
			  wpabuf_len(auth->c_sign_key));
		wdev_config->dpp_csign_len = wpabuf_len(auth->c_sign_key);
	}

#ifdef MAP_R3
	if (auth->pp_key) {
		wdev_config->dpp_ppkey = os_malloc(wpabuf_len(auth->pp_key));
		if (!wdev_config->dpp_ppkey)
			goto fail;
		os_memcpy(wdev_config->dpp_ppkey, wpabuf_head(auth->pp_key),
			  wpabuf_len(auth->pp_key));
		wdev_config->dpp_ppkey_len = wpabuf_len(auth->pp_key);
	}
#endif /* MAP_R3 */

	if (auth->net_access_key) {
		wdev_config->dpp_netaccesskey =
			os_malloc(wpabuf_len(auth->net_access_key));
		if (!wdev_config->dpp_netaccesskey)
			goto fail;
		os_memcpy(wdev_config->dpp_netaccesskey,
			  wpabuf_head(auth->net_access_key),
			  wpabuf_len(auth->net_access_key));
		wdev_config->dpp_netaccesskey_len = wpabuf_len(auth->net_access_key);
		wdev_config->dpp_netaccesskey_expiry = auth->net_access_key_expiry;
	}

	if (!auth->connector || dpp_akm_psk(auth->akm) ||
	    dpp_akm_sae(auth->akm)) {
		if (!auth->connector)
			wdev_config->key_mgmt = 0;
		if (dpp_akm_psk(auth->akm))
			wdev_config->key_mgmt |= WPA_KEY_MGMT_PSK |
				WPA_KEY_MGMT_PSK_SHA256 | WPA_KEY_MGMT_FT_PSK;
		if (dpp_akm_sae(auth->akm))
			wdev_config->key_mgmt |= WPA_KEY_MGMT_SAE |
				WPA_KEY_MGMT_FT_SAE;
#if 0 // Kapil not needed atm
		if (auth->passphrase[0]) {
			if (wpa_config_set_quoted(ssid, "psk",
						  auth->passphrase) < 0)
				goto fail;
			wpa_config_update_psk(ssid);
			ssid->export_keys = 1;
		} else
#endif
		{
			wdev_config->psk_set = auth->psk_set;
			os_memcpy(wdev_config->psk, auth->psk, PMK_LEN);
		}
	}

	if (wdev && wdev->config) {
		if (wdev->config->dpp_connector) {
			os_free(wdev->config->dpp_connector);
			wdev->config->dpp_connector = NULL;
		}
		if (wdev->config->ssid) {
			os_free(wdev->config->ssid);
			wdev->config->ssid = NULL;
		}
		if (wdev->config->dpp_csign) {
			os_free(wdev->config->dpp_csign);
			wdev->config->dpp_csign = NULL;
		}
		if (wdev->config->dpp_netaccesskey) {
			os_free(wdev->config->dpp_netaccesskey);
			wdev->config->dpp_netaccesskey = NULL;
		}	
		os_free(wdev->config);
		wdev->config = NULL;
	}
	if(wdev)
	/* Set this config in ongoing wdev */
		wdev->config = wdev_config;
	return wdev_config;
fail:
	if (wdev_config && wdev_config->dpp_ppkey)
		os_free(wdev_config->dpp_ppkey);
	if (wdev_config && wdev_config->dpp_csign)
		os_free(wdev_config->dpp_csign);
	if (wdev_config && wdev_config->dpp_connector)
		os_free(wdev_config->dpp_connector);
	if (wdev_config && wdev_config->ssid)
		os_free(wdev_config->ssid);
	if(wdev_config)
		os_free(wdev_config);
	return NULL;
}

#endif /* DPP_R2_MUOBJ */

static int wapp_wdev_dpp_scan(struct wifi_app *wapp, struct wapp_dev *wdev)
{
	wapp_issue_scan_request(wapp, wdev);
	time_t sec = time(0);

	if (sec != (time_t)(-1))
		srandom(sec); /* Use current time as seed for random generator used with random */
	else
		srandom(5381);
	wdev->scan_cookie = random();
	wdev->connection_tries++;
	//eloop_register_timeout(10, 0, map_get_scan_result, wapp, wdev);  //TODO check the timeout clearing
	return 0;
}

#ifdef DPP_R2_MUOBJ
static int wapp_dpp_process_config(struct wifi_app *wapp,
				    struct dpp_authentication *auth, struct dpp_config_obj *conf)
{
	struct dpp_config *config =  NULL;
#ifdef MAP_R3
	struct dpp_sec_cred *cred;
#endif

	// TODO kapil, this will be modified for MAP-R2, DPP-R2
	struct wapp_dev *wdev =  auth->wdev;

#ifdef MAP_R3
	if (wapp->dpp->is_map && auth->is_1905_connector) {
		cred = wapp_dpp_save_dpp_cred(auth);
		if (cred == NULL) {
			wpa_printf(MSG_ERROR, DPP_MAP_PREX
				"cred = NULL found %s %d", __func__, __LINE__);
				return 0;
		}

		cred_len = sizeof(*cred) + cred->payload_len; 
		hex_dump_dbg(DPP_MAP_PREX"1905_connector= ",(UCHAR *)cred, cred_len);
		if (wapp_send_1905_msg(wapp, WAPP_SEND_1905_CONNECTOR,
			cred_len, (char *)cred) < 0)
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed\n");
		os_free(cred);	
		return 0;
	}
#endif
#ifdef MAP_R3
	if (wapp->map->bh_type != MAP_BH_ETH) {  
#endif
	config = wapp_dpp_add_network(wapp, wdev, auth, conf);
	if (config && config->psk_set) {
		/* MAP turnkey feature, send the event to mapd */
	}
	/* Need to see how this can be broken if possible for best AP selection */
	/* food for Thought: current design of best AP selection itself is wrong, scan should be triggered 
	   from wapp and wapp should ask for best AP out of matched bss */

	/* set config parameters as per interface */
	wdev_set_dpp_akm(wapp, wdev, conf->akm);
	if(!dpp_akm_legacy(conf->akm))
		wdev_enable_pmf(wapp,wdev);

	if (conf->passphrase[0]) {
		wdev_set_psk(wapp, wdev, conf->passphrase);
	}
	wdev_set_ssid(wapp, wdev, (char *)conf->ssid);
	if(auth->wdev)	
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX" %s, wdev_type:%d \n", wdev->ifname, wdev->dev_type);


#ifdef DPP_R2_SUPPORT
	/* to handle the restart wapp for scan and reconnect case */
	eloop_cancel_timeout(wapp_dpp_presence_auth_wait_timeout, wapp, dpp_get_own_bi(wapp->dpp));
#endif /* DPP_R2_SUPPORT */
		if (wdev->dev_type == WAPP_DEV_TYPE_STA
#ifdef MAP_R3
			&& (wapp->map->bh_type != MAP_BH_ETH)
#endif
			) {
		// TODO add more dpp mixed modes
		if(dpp_akm_dpp(conf->akm))
			wapp_wdev_dpp_scan(wapp, wdev);
		else
			wdev_enable_apcli_iface(wapp, wdev, 1);

#ifdef DPP_R2_SUPPORT
		if (auth->conn_status_requested == 1) {
			eloop_cancel_timeout( wapp_dpp_conn_result_timeout, wapp,auth);
			eloop_register_timeout(
				DPP_CONNECT_STATUS_TIMEOUT, 0, wapp_dpp_conn_result_timeout, wapp,auth);
		}
#endif /* DPP_R2_SUPPORT */
		
		}
#ifdef MAP_R3
        }
#endif

	return 0;
}
#else

static int wapp_dpp_process_config(struct wifi_app *wapp,
				    struct dpp_authentication *auth)
{
	struct dpp_config *config =  NULL;
#ifdef MAP_R3
	struct dpp_sec_cred *cred;
	u16 cred_len;
	struct wapp_dev *rem_wdev = NULL;
	struct dl_list *dev_list;
#endif

	// TODO kapil, this will be modified for MAP-R2, DPP-R2
	struct wapp_dev *wdev =  auth->wdev;

#ifdef MAP_R3
#ifdef MAP_R3_RECONFIG
	//if(auth->allowed_roles != DPP_CAPAB_ENROLLEE )
	{
		if (auth->c_sign_key) {
			if (wapp->dpp_csign) {
				os_free(wapp->dpp_csign);
			}
			wapp->dpp_csign = NULL;
			wapp->dpp_csign = os_malloc(wpabuf_len(auth->c_sign_key));
			if (!wapp->dpp_csign) {
				wpa_printf(MSG_ERROR, DPP_MAP_PREX
				   "wapp->dpp_csign allocation failed");
				return 0;
			}
			os_memcpy(wapp->dpp_csign, wpabuf_head(auth->c_sign_key),
				  wpabuf_len(auth->c_sign_key));
			wapp->dpp_csign_len = wpabuf_len(auth->c_sign_key);
		
			wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "in process config cign key",
			    wapp->dpp_csign, wapp->dpp_csign_len);	
		}

		if (auth->connector) {
			if (wapp->dpp_ctrlconnector) {
				wpabuf_free(wapp->dpp_ctrlconnector);
			}
			wapp->dpp_ctrlconnector = NULL;
			wapp->dpp_ctrlconnector = wpabuf_alloc(os_strlen(auth->connector));
			if(wapp->dpp_ctrlconnector) {
				wpabuf_put_str(wapp->dpp_ctrlconnector, auth->connector);
				wpa_printf(MSG_DEBUG, DPP_MAP_PREX  "DPP: Save DPP connector.with len:%d auth connector:%d strlen:%d\n",
					(int)wpabuf_len(wapp->dpp_ctrlconnector),(int)os_strlen(auth->connector)
					,(int)os_strlen(wpabuf_head(wapp->dpp_ctrlconnector)));
			wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "in process config connector",
			    wapp->dpp_ctrlconnector, (int)wpabuf_len(wapp->dpp_ctrlconnector));	
			}
		}
	}
#endif
	if (wapp->dpp->is_map && auth->is_1905_connector) {
		cred = wapp_dpp_save_dpp_cred(auth);
		if (!cred) {
			DBGPRINT(RT_DEBUG_ERROR, "%s %d cred is NULL\n", __func__, __LINE__);
			return 0;
		}
		cred_len = sizeof(*cred) + cred->payload_len; 
		hex_dump_dbg(DPP_MAP_PREX"1905_connector= ",(UCHAR *)cred, cred_len);
		if (wapp_send_1905_msg(wapp, WAPP_SEND_1905_CONNECTOR,
			cred_len, (char *)cred) < 0)
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed\n");
		auth->is_1905_connector = 0;
		os_free(cred);	
		return 0;
	}
#endif
#ifdef MAP_R3
	if (((wapp->map->TurnKeyEnable == 0) && (wapp->map->bh_type != MAP_BH_ETH) && !obj_count)
			|| ((wapp->map->TurnKeyEnable == 1) && !obj_count)) {  
#endif
	if (!wapp->map->TurnKeyEnable) {
		config = wapp_dpp_add_network(wapp, wdev, auth);
		if (config && config->psk_set) {
			/* MAP turnkey feature, send the event to mapd */
		}
		/* Need to see how this can be broken if possible for best AP selection */
		/* food for Thought: current design of best AP selection itself is wrong, scan should be triggered 
		   from wapp and wapp should ask for best AP out of matched bss */

		/* set config parameters as per interface */
		wdev_set_dpp_akm(wapp, wdev, auth->akm);
		if(!dpp_akm_legacy(auth->akm))
			wdev_enable_pmf(wapp,wdev);

		if (auth->passphrase[0]) {
			wdev_set_psk(wapp, wdev, auth->passphrase);
		}
		wdev_set_ssid(wapp, wdev, (char *)auth->ssid);
	}
#ifdef MAP_R3
	obj_count++;	
	if(wapp->map->TurnKeyEnable == 1) {
		/* For Turnkey mode update other STA wdev with same information 
		 * as it is possible that connection can occur on any band */
		dev_list = &wapp->dev_list;
		dl_list_for_each(rem_wdev, dev_list, struct wapp_dev, list){
			if (rem_wdev->dev_type == WAPP_DEV_TYPE_STA) {

				wapp_dpp_add_network(wapp, rem_wdev, auth);
				wdev_set_dpp_akm(wapp, rem_wdev, auth->akm);
				if(!dpp_akm_legacy(auth->akm))
					wdev_enable_pmf(wapp,rem_wdev);

				if (auth->passphrase[0]) {
					wdev_set_psk(wapp, rem_wdev, auth->passphrase);
				}
				wdev_set_ssid(wapp, rem_wdev, (char *)auth->ssid);
			}
		}
	}
	else
		wapp_dpp_clear_cred(auth);
#endif /* MAP_R3 */

#ifdef DPP_R2_SUPPORT
	/* to handle the restart wapp for scan and reconnect case */
	eloop_cancel_timeout(wapp_dpp_presence_auth_wait_timeout, wapp, dpp_get_own_bi(wapp->dpp));
#endif /* DPP_R2_SUPPORT */
		if ((wdev && wdev->dev_type == WAPP_DEV_TYPE_STA
#ifdef MAP_R3
			&& ((wapp->map->TurnKeyEnable == 0) && (wapp->map->bh_type != MAP_BH_ETH))) || (wapp->map->TurnKeyEnable == 1)
#endif
			) {

			if(wapp->map->TurnKeyEnable == 1) {
				/* For Turnkey mode use best AP seletion for BSTA connection */
				wsc_apcli_config_msg *apcli_config_msg = NULL;
				wsc_apcli_config *apcli_config = NULL;
				int msg_size = sizeof(wsc_apcli_config_msg) + sizeof(wsc_apcli_config);
				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"value of msg size:%d\n",msg_size);
				
				os_alloc_mem(NULL, (unsigned char **)&apcli_config_msg, msg_size);

				if(!apcli_config_msg) {
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"alloc failed\n");
					
					return -1;
				}
				os_memset(apcli_config_msg, 0, msg_size);

				apcli_config = &apcli_config_msg->apcli_config[0];

				apcli_config->SsidLen = auth->ssid_len;
				os_memcpy(apcli_config->ssid, auth->ssid, auth->ssid_len);
				apcli_config->AuthType = wdev_get_dpp_driver_auth_type_wsc(auth->akm);
				apcli_config->EncrType = WSC_ENCRTYPE_AES;
				apcli_config->KeyLength = os_strlen(auth->passphrase);
				os_memcpy(apcli_config->Key, auth->passphrase, os_strlen(auth->passphrase));
				apcli_config_msg->profile_count = 1;
				wapp_send_1905_msg(wapp, WAPP_MAP_BH_CONFIG, msg_size, (void *)apcli_config_msg);
				os_free(apcli_config_msg);
				auth->reconfig_bhconfig_done = 1;
				//read_backhaul_configs(wapp);
			}
			else {
#ifdef MAP_R3
				wapp_wdev_dpp_scan(wapp, wdev);
#else
				// TODO add more dpp mixed modes
				if(dpp_akm_dpp(auth->akm))
					wapp_wdev_dpp_scan(wapp, wdev);
				else
					wdev_enable_apcli_iface(wapp, wdev, 1);
#endif /* MAP_R3 */
			}

#ifdef DPP_R2_SUPPORT
		if (auth->conn_status_requested == 1) {
			eloop_cancel_timeout( wapp_dpp_conn_result_timeout, wapp,auth);
			eloop_register_timeout(DPP_CONNECT_STATUS_TIMEOUT,
				0, wapp_dpp_conn_result_timeout, wapp, auth);
		}
#endif /* DPP_R2_SUPPORT */
		
		}
#ifdef MAP_R3
        }
#endif
	return 0;
}

#endif /* DPP_R2_MUOBJ */


static void wapp_dpp_gas_resp_cb(void *ctx, const u8 *addr, u8 dialog_token,
				 enum gas_query_result result,
				 const struct wpabuf *adv_proto,
				 const struct wpabuf *resp, u16 status_code)
{
	struct wifi_app *wapp = ctx;
	const u8 *pos;
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)addr);
	enum dpp_status_error status = DPP_STATUS_CONFIG_REJECTED;

	wapp->dpp->dpp_gas_dialog_token = -1;

	if (!auth || !auth->auth_success) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: No matching exchange in progress");
		return;
	}

	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: GAS response from " MACSTR, MAC2STR(addr));
	if (auth->current_state != DPP_STATE_CONFIG_RSP_WAITING) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Incorrect auth state=%d", auth->current_state);
		goto fail;
	}
	if (result != GAS_QUERY_SUCCESS ||
	    !resp || status_code != WLAN_STATUS_SUCCESS) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: GAS query did not succeed");
		goto fail;
	}

	wpa_hexdump_buf(MSG_DEBUG, "DPP: Configuration Response adv_proto",
			adv_proto);
	wpa_hexdump_buf(MSG_DEBUG, "DPP: Configuration Response (GAS response)",
			resp);

	if (wpabuf_len(adv_proto) != 10 ||
	    !(pos = wpabuf_head(adv_proto)) ||
	    pos[0] != WLAN_EID_ADV_PROTO ||
	    pos[1] != 8 ||
	    pos[3] != WLAN_EID_VENDOR_SPECIFIC ||
	    pos[4] != 5 ||
	    WPA_GET_BE24(&pos[5]) != OUI_WFA ||
	    pos[8] != 0x1a ||
	    pos[9] != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Not a DPP Advertisement Protocol ID");
		goto fail;
	}

	if (dpp_conf_resp_rx(auth, resp) < 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Configuration attempt failed");
		goto fail;
	}
	status = DPP_STATUS_OK;
fail:

	if (status != DPP_STATUS_OK)
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONF_FAILED);
#ifdef DPP_R2_SUPPORT
	if (status != DPP_STATUS_OK) {
		// send the presence annouce again 
		if(!auth->configurator
#ifdef MAP_R3
		&& (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE)
#endif /* MAP_R3 */
		) {
			if (auth->reconfigTrigger != TRUE) {
				dpp_auth_deinit(auth);
				if(wapp->dpp->dpp_eth_conn_ind == FALSE) {
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"config resp fail, restart presence frame \n");
					wapp->dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_INIT;
					wapp_dpp_presence_annouce(wapp, NULL);
				}
			}
#ifdef MAP_R3_RECONFIG
			else {
				auth->timeout_recon = 1;
				dpp_auth_deinit(auth);
				if(wapp->dpp->dpp_eth_conn_ind == FALSE) {
					wapp->dpp->reconfig_annouce.is_enable = 1;
					wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT;
					wapp->dpp->dpp_allowed_roles = DPP_CAPAB_ENROLLEE;
					wapp->dpp->cce_driver_scan_done = 0;

					wapp_dpp_send_reconfig_annouce(wapp, wapp->wdev_backup);
				}
			} 
#endif
		}
		return;
	}
#endif /* DPP_R2_SUPPORT */
#ifdef DPP_R2_SUPPORT
	if (auth->peer_version >= 2 &&
	    auth->conf_resp_status == DPP_STATUS_OK && !auth->configurator) {
		struct wpabuf *msg;

		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Send DPP Configuration Result");
		msg = dpp_build_conf_result(auth, status);
		if (!msg) {
			status = DPP_STATUS_CONFIG_REJECTED;
			goto fail2;
		}

                wpa_printf(MSG_INFO1, DPP_MAP_PREX
                        DPP_EVENT_TX "dst=" MACSTR " chan=%u type=%d",
                        MAC2STR(addr), auth->curr_chan,
                        DPP_PA_CONFIGURATION_RESULT);
#ifdef MAP_R3
		if(status == DPP_STATUS_OK) {
			DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"setting config done here..\n");
			wapp->dpp->config_done = 1;
		}
#endif /* MAP_R3 */
		auth->current_state = DPP_STATE_CONFIG_RESULT_WAITING;
		wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 500,
					addr, wpabuf_head(msg), wpabuf_len(msg));
                wpabuf_free(msg);

		/* This exchange will be terminated in the TX status handler */
	}
fail2:

	if(auth && !auth->conn_status_requested
#ifdef MAP_R3
	&& (status != DPP_STATUS_OK)
#endif /* MAP_R3 */
	)
#endif /* DPP_R2_SUPPORT */		
		dpp_auth_deinit(auth);
}

int append_map_version_tlv(unsigned char *pkt, unsigned char version)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;

	temp_buf = pkt;
	total_length = 4;
	(*temp_buf) = MULTI_AP_VERSION_TYPE;
	*(unsigned short *)(temp_buf+1) = cpu2be16(total_length-3);
	*(temp_buf+3) = version;

	return total_length;
}

unsigned short append_supported_service_tlv(
		unsigned char *pkt, unsigned char service)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;

	temp_buf = pkt;

	*temp_buf = SUPPORTED_SERVICE_TLV_TYPE;
	temp_buf +=1;
	/**
	 * need communicate with WAPP to get supported service
	 * or just write hard code to Multi-AP Agent
	 */
	if (service == 0) {
		*(unsigned short *)(temp_buf) = cpu2be16(SUPPORTED_SERVICE_LENGTH);
		temp_buf += 2;
		*temp_buf++ = 1;
		*temp_buf = SERVICE_CONTROLLER;
		total_length = SUPPORTED_SERVICE_LENGTH + 3;
	} else if (service == 1) {
		*(unsigned short *)(temp_buf) = cpu2be16(SUPPORTED_SERVICE_LENGTH);
		temp_buf += 2;
		*temp_buf++ = 1;
		*temp_buf = SERVICE_AGENT;
		total_length = SUPPORTED_SERVICE_LENGTH + 3;
	} else if (service == 2) {
		*(unsigned short *)(temp_buf) = cpu2be16(SUPPORTED_SERVICE2_LENGTH);
		temp_buf += 2;
		*temp_buf++ = 2;
		*temp_buf++ = SERVICE_CONTROLLER;
		*temp_buf = SERVICE_AGENT;
		total_length = SUPPORTED_SERVICE2_LENGTH + 3;
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "unvalid service\n");
	}

	return total_length;
}

unsigned short append_ap_radio_basic_capability_tlv(unsigned char *pkt,
		struct ap_radio_basic_cap *bcap)
{
	unsigned short total_length = 0, tmp_len = 0, i;
	unsigned char *temp_buf;

	temp_buf = pkt;
	*temp_buf = AP_RADIO_BASIC_CAPABILITY_TYPE;
	temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, bcap->identifier, MAC_ADDR_LEN);
	temp_buf += MAC_ADDR_LEN;
	total_length += MAC_ADDR_LEN;

	*temp_buf = bcap->max_bss_num;
	temp_buf += 1;
	total_length += 1;

	temp_buf[0] = bcap->op_class_num;
	temp_buf += 1;
	total_length += 1;

	for (i = 0; i < bcap->op_class_num; i++)
	{
		tmp_len = 3 + bcap->opcap[i].non_operch_num;
		memcpy(temp_buf, &bcap->opcap[i], tmp_len);
		temp_buf += tmp_len;
		total_length += tmp_len;
	}

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = cpu2be16(total_length - 3);

	return total_length;
}

unsigned short append_backhaul_sta_radio_caps_tlv(unsigned char *pkt,
		struct radio_bhsta_caps *cap)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;

	temp_buf = pkt;
	*temp_buf = BACKHAUL_STATION_RADIO_CAP_TYPE;
	temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, cap->identifier, MAC_ADDR_LEN);
	temp_buf += MAC_ADDR_LEN;
	total_length += MAC_ADDR_LEN;

	if (!cap->is_sta_mac) {
		*temp_buf = 0x1;
		total_length += 1;
	} else {
		memcpy(temp_buf, cap->sta_mac, MAC_ADDR_LEN);
		temp_buf += MAC_ADDR_LEN;
		total_length += MAC_ADDR_LEN;
	}

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = cpu2be16(total_length - 3);

	return total_length;
}

unsigned short append_ap_radio_advanced_capability_tlv(unsigned char *pkt,
		struct radio_adv_caps *cap)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;

	temp_buf = pkt;
	*temp_buf = AP_RADIO_ADVANCE_CAP_TLV;
	temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, cap->identifier, MAC_ADDR_LEN);
	temp_buf += MAC_ADDR_LEN;
	total_length += MAC_ADDR_LEN;

	*temp_buf = cap->ts_rules_support;
	temp_buf += 1;
	total_length += 1;

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = cpu2be16(total_length - 3);

	return total_length;
}


unsigned short append_r2_ap_caps(unsigned char *pkt, struct r2_ap_caps *caps)
{
	unsigned short total_length = 0, i;
	unsigned char *temp_buf;

	temp_buf = pkt;
	*temp_buf = R2_CAP_TLV_TYPE;
	temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	*(unsigned short *)(temp_buf) = cpu2be16(caps->max_sp_rule_cnt);
	temp_buf += 2;
	total_length += 2;

	*(unsigned short *)(temp_buf) = cpu2be16(caps->max_adv_sp_rule_cnt);
	temp_buf += 2;
	total_length += 2;

	*(unsigned short *)(temp_buf) = cpu2be16(caps->max_destination_addr);
	temp_buf += 2;
	total_length += 2;

	*temp_buf = caps->byte_count_unit;
	temp_buf += 1;
	total_length += 1;

	*temp_buf = caps->max_vid;
	temp_buf += 1;
	total_length += 1;

	*temp_buf = caps->eth_edge_iface_cnt;
	temp_buf += 1;
	total_length += 1;

	for (i = 0; i < caps->eth_edge_iface_cnt; i++)
	{
		memcpy(temp_buf, caps->addr[i], MAC_ADDR_LEN);
		temp_buf += MAC_ADDR_LEN;
		total_length += MAC_ADDR_LEN;
	}

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = cpu2be16(total_length - 3);

	return total_length;
}

unsigned short append_akm_suite_capability(unsigned char *pkt,
		struct akm_suite_caps *caps)
{
	unsigned short total_length = 0, i;
	unsigned char *temp_buf;

	temp_buf = pkt;
	*temp_buf = AKM_SUITE_TLV_TYPE;
	temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	*temp_buf = caps->bhsta_akm_cnt;
	temp_buf += 1;
	total_length += 1;

	for (i = 0; i < caps->bhsta_akm_cnt; i++)
	{
		memcpy(temp_buf, caps->bhsta_akm[i].oui, 3);
		temp_buf += 3;
		total_length += 3;
		*temp_buf = caps->bhsta_akm[i].oui_type;
		temp_buf += 1;
		total_length += 1;
	}

	*temp_buf = caps->bhap_akm_cnt;
	temp_buf += 1;
	total_length += 1;

	for (i = 0; i < caps->bhap_akm_cnt; i++)
	{
		memcpy(temp_buf, caps->bhap_akm[i].oui, 3);
		temp_buf += 3;
		total_length += 3;
		*temp_buf = caps->bhap_akm[i].oui_type;
		temp_buf += 1;
		total_length += 1;
	}

	*temp_buf = caps->fhap_akm_cnt;
	temp_buf += 1;
	total_length += 1;

	for (i = 0; i < caps->fhap_akm_cnt; i++)
	{
		memcpy(temp_buf, caps->fhap_akm[i].oui, 3);
		temp_buf += 3;
		total_length += 3;
		*temp_buf = caps->fhap_akm[i].oui_type;
		temp_buf += 1;
		total_length += 1;
	}

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = cpu2be16(total_length - 3);

	return total_length;
}

int dpp_get_map_tlv_blob(struct wifi_app *wapp, unsigned char *map_tvl_blob)
{
	int total_len = 0, i;
	int num_radio = 3; //TODO initialize it

	/* version */
	total_len += append_map_version_tlv(map_tvl_blob + total_len, 3); //TODO check in spec

	/* supported service */
	total_len += append_supported_service_tlv(map_tvl_blob + total_len, 1);
	/* akm suite */
	total_len += append_akm_suite_capability(map_tvl_blob + total_len, &wapp->akm_caps);
	/* radio basic caps multiple */
	for (i = 0; i < num_radio; i++) {
		/* basic radio caps tlv */
		os_memcpy(map_tvl_blob + total_len, wapp->radio[i].bcap_tlv, wapp->radio[i].bcap_len);
		total_len += wapp->radio[i].bcap_len;
		//total_len += append_ap_radio_basic_capability_tlv(map_tvl_blob + total_len, &wapp->wapp->radio[i].bcap);
	}
	/* backhaul sta radio */
	for (i = 0; i < num_radio; i++) {
		total_len += append_backhaul_sta_radio_caps_tlv(map_tvl_blob + total_len, &wapp->radio[i].bhsta_cap);
	}
	/* R2 ap caps */
	total_len += append_r2_ap_caps(map_tvl_blob + total_len, &wapp->r2_ap_cap);
	/* ap radio advaced caps multiple */
	for (i = 0; i < num_radio; i++) {
		total_len += append_ap_radio_advanced_capability_tlv(map_tvl_blob + total_len, &wapp->radio[i].adv_cap);
	}

	wpa_hexdump(0, "tlv blob", map_tvl_blob, total_len);
	return total_len;
}

#ifdef MAP_R3
static void wapp_dpp_bsta_obj(struct wifi_app *wapp, struct dpp_authentication *auth, char *bsta_str,
				int len)
{
	char chan_str[100] = {0};
	char *pos = NULL;
	int radio_count = 0 , i = 0;
	int chan_str_len = 0, j = 0;
	int ret;

#if 0
	buf = os_malloc(len);
	if(buf == NULL){
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"NULL Buffer\n");
	}

	buf = bsta_str;
#endif

	radio_count = wapp_get_valid_radio_count(wapp);
	if(radio_count == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"error in radio count\n");
		return;
	}
	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"radio count:%d\n",radio_count);

	pos = chan_str;
	for(i = radio_count; i > 0 ; i--) {
		ret = os_snprintf(pos, sizeof(chan_str), "%d/%d,",
				wapp_dpp_get_opclass_from_channel(wapp->radio[i-1].op_ch), wapp->radio[i-1].op_ch);
		if (os_snprintf_error(sizeof(chan_str), ret))
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		j = strlen(pos);
		pos = pos + j;
	}
	chan_str_len = strlen(chan_str);
	chan_str[chan_str_len-1] = '\0';  //Removing last comma from channel string

	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"channel list str:%s\n",chan_str);
	ret = os_snprintf(bsta_str, len,
			"{\"netRole\":\"mapbackhaulSta\","
			"\"akm\":\"dpp+sae+psk\","
			"\"channelList\":\"%s\"}",
			chan_str);
	if (os_snprintf_error(len, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	return;
}
#endif /* MAP_R3 */

static void wapp_dpp_start_gas_client(struct wifi_app *wapp)
{
	/* enrollee mode, get the first auth from auth list */
	struct dpp_authentication *auth = wapp_dpp_get_first_auth(wapp);
	struct wpabuf *buf;
#ifdef DPP_R2_SUPPORT
	char json[600];
#else
	char json[300];
#endif /* DPP_R2_SUPPORT */
	//unsigned char map_tvl_blob[1000];
	int res; // len;
	//size_t outlen;
	char is_sta = 0;
#ifdef MAP_R3
	char bsta_str[200] = {0};
#endif /* MAP_R3 */

	if (!auth)
		return;

	wapp_dpp_listen_stop(wapp, auth->wdev);

	if (!wapp->dpp->is_map) {
		if (auth->wdev->dev_type == WAPP_DEV_TYPE_STA)
			is_sta = 1;
#ifdef DPP_R2_SUPPORT
		os_snprintf(json, sizeof(json),
				"{\"name\":\"Test\","
				"\"wi-fi_tech\":\"infra\","
				"\"netRole\":\"%s\","
				"\"mudurl\":\"%s\"}",
				is_sta ? "sta" : "ap",
				wapp->dpp->dpp_mud_url);
#else
		os_snprintf(json, sizeof(json),
				"{\"name\":\"Test\","
				"\"wi-fi_tech\":\"infra\","
				"\"netRole\":\"%s\"}",
				is_sta ? "sta" : "ap");
#endif /* DPP_R2_SUPPORT */
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: GAS Config Attributes: %s", json);
	}
#ifdef MAP_R3
	else {
		wapp_dpp_bsta_obj(wapp, auth, bsta_str, sizeof(bsta_str));
#if 0	
		if (wapp->map->bh_type != MAP_BH_ETH)
			os_snprintf(chan_str, sizeof(chan_str), "%d",auth->wdev->radio->op_ch);	

		os_snprintf(json, sizeof(json),
				"{\"name\":\"MTK\","
				"\"wi-fi_tech\":\"map\","
				"\"bSTAList\":[{"
				"\"netRole\":\"mapAgent\","
				"\"akm\":\"psk\","
				"\"channelList\":\"%s\"}]}",
				chan_str);
#endif
		if(wapp->map->MapMode == 4) {
			if (wapp->map->bh_type == MAP_BH_ETH) {
				os_snprintf(json, sizeof(json),
						"{\"name\":\"MTK\","
						"\"wi-fi_tech\":\"map\","
                                                "\"netRole\":\"mapAgent\"}");
				wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: GAS Config Attributes: %s", json);
			}
			else {
#if 0
#ifdef MAP_R3_RECONFIG
				if (auth->reconfigTrigger != TRUE)
#endif
#endif
				 {
					os_snprintf(json, sizeof(json),
						"{\"name\":\"MTK\","
						"\"wi-fi_tech\":\"map\","
						"\"netRole\":\"mapAgent\","
						"\"bSTAList\":[%s]}",
						bsta_str);
				}
#if 0
#ifdef MAP_R3_RECONFIG
                		else {
		                   	os_snprintf(json, sizeof(json),
                			        "{\"name\":\"MTK\","
			                        "\"wi-fi_tech\":\"map\","
                        			"\"bSTAList\":[%s]}",
		                 	        bsta_str);
                		}
#endif
#endif

				wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: GAS Config Attributes: %s", json);
			}
		}
		else {
			os_snprintf(json, sizeof(json),
					"{\"name\":\"MTK\","
					"\"wi-fi_tech\":\"map\","
					"\"netRole\":\"mapAgent\","
					"\"bSTAList\":[%s]}",
					bsta_str);

			wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: GAS Config Attributes: %s", json);
		}
	}
#endif /* MAP_R3 */

	buf = dpp_build_conf_req(auth, json);
	if (!buf) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: No configuration request data available");
#ifdef DPP_R2_SUPPORT
		if(!auth->configurator
#ifdef MAP_R3
		&& (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE)
#endif /* MAP_R3 */
		) {
			DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"build config request fail, restart presence frame \n");
			if (auth->reconfigTrigger != TRUE) {
				dpp_auth_deinit(auth);
				if(wapp->dpp->dpp_eth_conn_ind == FALSE) {
					wapp->dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_INIT;
					wapp_dpp_presence_annouce(wapp, NULL);
				}
			}
#ifdef MAP_R3_RECONFIG
			else {
				auth->timeout_recon = 1;
				dpp_auth_deinit(auth);
				if(wapp->dpp->dpp_eth_conn_ind == FALSE) {
					wapp->dpp->reconfig_annouce.is_enable = 1;
					wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT;
					wapp->dpp->dpp_allowed_roles = DPP_CAPAB_ENROLLEE;
					wapp->dpp->cce_driver_scan_done = 0;

					wapp_dpp_send_reconfig_annouce(wapp, wapp->wdev_backup);
				}
			}
#endif
		}
#endif /* DPP_R2_SUPPORT */
		return;
	}

#ifdef CONFIG_DPP2
	if (auth->is_wired) {
		wpabuf_free(auth->msg_out);
		auth->msg_out_pos = 0;
		auth->msg_out = wpabuf_alloc(4 + wpabuf_len(buf) - 1);
		if (!auth->msg_out) {
			wpabuf_free(buf);
			return;
		}
		if (!auth->is_map_connection)
			wpabuf_put_be32(auth->msg_out, wpabuf_len(buf) - 1);
		wpabuf_put_data(auth->msg_out, wpabuf_head(buf) + 1,
				wpabuf_len(buf) - 1);
		wpabuf_free(buf);

		if (dpp_wired_send(auth) == 1) {
			if (!auth->write_eloop) {
				if (eloop_register_sock(auth->sock, EVENT_TYPE_WRITE,
							dpp_conn_tx_ready,
							auth, NULL) < 0)
					return;
				auth->write_eloop = 1;
			}
		}
		eloop_cancel_timeout(wapp_dpp_eth_resp_wait_timeout, wapp, auth);
		eloop_register_timeout(GAS_QUERY_TIMEOUT_PERIOD, 0,
					wapp_dpp_eth_resp_wait_timeout, wapp, auth);

	} else 
#endif /* CONFIG_DPP2 */
	{
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: GAS request to " MACSTR " (chan %u MHz)",
				MAC2STR(auth->peer_mac_addr), auth->curr_chan);

		res = gas_query_req(wapp->dpp->gas_query_ctx, auth->wdev, auth->peer_mac_addr, auth->curr_chan,
				1, buf, wapp_dpp_gas_resp_cb, wapp);
		if (res < 0) {
			wpa_printf(MSG_ERROR, DPP_MAP_PREX "GAS: Failed to send Query Request");
			wpabuf_free(buf);
		} else {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
					"DPP: GAS query started with dialog token %u", res);
			wapp->dpp->dpp_gas_dialog_token = res;
		}
	}
}

void wapp_dpp_auth_success(struct wifi_app *wapp, int initiator, struct wapp_dev *wdev)
{
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Authentication succeeded init=%d", initiator);
	if (!wapp->dpp->dpp_configurator_supported)
		wapp_dpp_start_gas_client(wapp);
}

#ifdef CONFIG_DPP2

int wapp_dpp_process_conf_obj(void *ctx,
				struct dpp_authentication *auth)
{
	struct wifi_app *wapp = ctx;

#ifdef DPP_R2_MUOBJ
	return wapp_dpp_handle_config_obj(wapp, auth, &auth->conf_obj[0]);
#else
	return wapp_dpp_handle_config_obj(wapp, auth);
#endif  /* DPP_R2_MUOBJ */
}

#endif /* CONFIG_DPP2 */

void dpp_auth_list_deinit(struct wifi_app *wapp)
{
	struct dpp_authentication *auth = wapp_dpp_get_first_auth(wapp);

	if (auth) {
		eloop_cancel_timeout(wapp_dpp_reply_wait_timeout, wapp, auth);
		eloop_cancel_timeout(wapp_dpp_init_timeout, wapp, auth);
#ifdef DPP_R2_SUPPORT
		eloop_cancel_timeout(wapp_dpp_config_result_wait_timeout, wapp,
			     auth);
#endif /* DPP_R2_SUPPORT */
		/* auth = wapp_dpp_get_first_auth(wapp); */
		eloop_cancel_timeout(wapp_dpp_auth_resp_retry_timeout, wapp, auth);
		wapp_dpp_listen_stop(wapp, auth->wdev);
		dpp_auth_deinit(auth);
	}
}

static int wapp_dpp_pkex_next_channel(struct wifi_app *wapp,
				      struct dpp_pkex *pkex)
{
	if (pkex->chan == 6)
		pkex->chan = 149;
	else if (pkex->chan == 149)
		pkex->chan = 44;
	else
		return -1; /* no more channels to try */

	/* Could not use this channel - try the next one */
	return wapp_dpp_pkex_next_channel(wapp, pkex);
}


static void wapp_dpp_pkex_retry_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_pkex *pkex = wapp->dpp->dpp_pkex;

	if (!pkex || !pkex->exchange_req)
		return;
	if (pkex->exch_req_tries >= 5) {
		if (wapp_dpp_pkex_next_channel(wapp, pkex) < 0) {
			wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_FAIL
				"No response from PKEX peer");
			dpp_pkex_free(pkex);
			wapp->dpp->dpp_pkex = NULL;
			return;
		}
		pkex->exch_req_tries = 0;
	}

	pkex->exch_req_tries++;
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Retransmit PKEX Exchange Request (try %u)",
		   pkex->exch_req_tries);
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR " chan=%u type=%d",
		MAC2STR(broadcast), pkex->chan, DPP_PA_PKEX_EXCHANGE_REQ);
	wapp_drv_send_action(wapp, pkex->wdev, pkex->chan, 0, broadcast,
				wpabuf_head(pkex->exchange_req),
				wpabuf_len(pkex->exchange_req));
}

#ifdef MAP_R3
void dpp_map_controller_start(struct wifi_app *wapp)
{
	struct map_dev_list *dev, *tmp_dev;
	char param_prefix[100] = {0};
	int ret;
	dl_list_for_each_safe(dev, tmp_dev, &(wapp->map_devices),
					struct map_dev_list, list) {
		ret = os_snprintf(param_prefix, sizeof(param_prefix), "almac=%0x:%0x:%0x:%0x:%0x:%0x", PRINT_MAC(dev->al_mac));
		if (os_snprintf_error(sizeof(param_prefix), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			break;
		}

		wapp_dpp_auth_init(wapp, NULL, param_prefix);
		//kapil TODO one by one, create state machine
		break;
	}
}
#endif /* MAP_R3 */

void
wapp_dpp_tx_pkex_status(struct wifi_app *wapp,
			unsigned int chan, const u8 *dst,
			const u8 *src, const u8 *bssid,
			const u8 *data, size_t data_len,
			int ok)
{
	struct dpp_pkex *pkex = wapp->dpp->dpp_pkex;

	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX_STATUS "dst=" MACSTR
		" chan=%u result=%d", MAC2STR(dst), chan, ok);

	if (!pkex) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Ignore TX status since there is no ongoing PKEX exchange");
		return;
	}

	if (pkex->failed) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Terminate PKEX exchange due to an earlier error");
		if (pkex->t > pkex->own_bi->pkex_t)
			pkex->own_bi->pkex_t = pkex->t;
		dpp_pkex_free(pkex);
		wapp->dpp->dpp_pkex = NULL;
		return;
	}

	if (pkex->exch_req_wait_time && pkex->exchange_req) {
		/* Wait for PKEX Exchange Response frame and retry request if
		 * no response is seen. */
		eloop_cancel_timeout(wapp_dpp_pkex_retry_timeout, wapp, NULL);
		eloop_register_timeout(pkex->exch_req_wait_time / 1000,
				       (pkex->exch_req_wait_time % 1000) * 1000,
				       wapp_dpp_pkex_retry_timeout, wapp,
				       NULL);
	}
}

int wapp_dpp_check_connect(struct wifi_app *wapp,
			   struct wapp_dev *wdev,
			   char *bssid, unsigned char chan)
{
	struct dpp_config *config = wdev->config;
	struct os_time now;
	struct wpabuf *msg;
#if 0
	unsigned int wait_time;
#endif
	size_t len;
#ifdef CONFIG_DPP2
	u8 version;
#endif /* CONFIG_DPP2 */

	if(config == NULL) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "No configuration found in wdev:%s\n",wdev->ifname);
		return -1;
	}

	if (!wapp->dpp) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "dpp is null :%d\n", __LINE__);
		return -1;
	}

	if (!(config->key_mgmt & WPA_KEY_MGMT_DPP) || !bssid)
		return 0; /* Not using DPP AKM - continue */

	/* Kapil: Do we need to maintain a cache here as well? */
#if 0
	if (wpa_sm_pmksa_exists(wapp->dpp->wpa, bssid, config))
		return 0; /* PMKSA exists for DPP AKM - continue */
#endif

	if (!config->dpp_connector || !config->dpp_netaccesskey ||
	    !config->dpp_csign) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX DPP_EVENT_MISSING_CONNECTOR
			"missing %s",
			!config->dpp_connector ? "Connector" :
			(!config->dpp_netaccesskey ? "netAccessKey" :
			 "C-sign-key"));
		return -1;
	}

	os_get_time(&now);

	if (config->dpp_netaccesskey_expiry &&
	    (os_time_t) config->dpp_netaccesskey_expiry < now.sec) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX DPP_EVENT_MISSING_CONNECTOR
			"netAccessKey expired");
		return -1;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		   "DPP: Starting network introduction protocol to derive PMKSA for "
		   MACSTR, MAC2STR(bssid));

	len = 5 + 4 + os_strlen(config->dpp_connector);
#ifdef CONFIG_DPP2
	len += 5;
#endif /* CONFIG_DPP2 */
	msg = dpp_alloc_msg(DPP_PA_PEER_DISCOVERY_REQ, len);

	if (!msg)
		return -1;

	/* Transaction ID */
	wpabuf_put_le16(msg, DPP_ATTR_TRANSACTION_ID);
	wpabuf_put_le16(msg, 1);
	wpabuf_put_u8(msg, TRANSACTION_ID);

	/* DPP Connector */
	wpabuf_put_le16(msg, DPP_ATTR_CONNECTOR);
	wpabuf_put_le16(msg, os_strlen(config->dpp_connector));
	wpabuf_put_str(msg, config->dpp_connector);

#ifdef CONFIG_DPP2
	if (wapp->dpp->version_ctrl > 1) {
		/* Protocol Version */
		version = wapp->dpp->version_ctrl;
		wpabuf_put_le16(msg, DPP_ATTR_PROTOCOL_VERSION);
		wpabuf_put_le16(msg, 1);
		wpabuf_put_u8(msg, version);
	}
#endif /* CONFIG_DPP2 */

	if(chan != wdev->radio->op_ch) {
		/*changing the channel if not same*/
		wpa_printf(MSG_INFO1, DPP_MAP_PREX" wdev %s is not on channel:%d so move first\n",
			wdev->ifname, (int)chan);
		wapp_cancel_remain_on_channel(wapp, wdev);
		wdev_set_quick_ch(wapp, wdev, (int)chan);
	}
	
	/* TODO: Timeout on AP response */
#if 0
/* wait_time variable is not using below this */
	wait_time = wapp->dpp->max_remain_on_chan;
	if (wait_time > 2000)
		wait_time = 2000;
#endif
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR " chan=%u type=%d",
		MAC2STR(bssid), chan, DPP_PA_PEER_DISCOVERY_REQ);
	wapp_drv_send_action(wapp, wdev, (unsigned int)chan, 0,
			     (u8 *)bssid, wpabuf_head(msg), wpabuf_len(msg));
	wpabuf_free(msg);

	return 1;
}

int wapp_handle_dpp_scan(struct wifi_app *wapp, struct wapp_dev *wdev)
{
	struct bss_info_scan_result *scan_result, *scan_result_tmp;
	struct dpp_config *wdev_config = wdev->config;
	int ret;
#ifdef MAP_R3
	char cmd[100] = {0};
	struct backhaul_connect_request *bh = NULL;
#endif /* MAP_R3 */

	if (!wdev->connection_tries) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"dpp connection is not ongoing\n");
		return -1;
	}

	if (!wdev_config) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"wdev config not found\n");
		return -1;
	}
#if ((defined DPP_R2_SUPPORT) || (defined DPP_R2_RECONFIG))
	// get scan channel
	wapp_dpp_scan_channel(wapp, wdev);
#endif
	/* parse scan results */
	dl_list_for_each_safe(scan_result, scan_result_tmp, &(wapp->scan_results_list),
								struct bss_info_scan_result, list) {

		if (os_memcmp(scan_result->bss.Ssid, wdev_config->ssid, scan_result->bss.SsidLen) == 0) {
			/* Found our match */
			/* TODO match rsn */
			/* TODO Return best out of all bss found */
			/*do not connect if wildcard ssid comes in results */
			if(scan_result->bss.SsidLen == 0)
				continue;
#ifdef MAP_R3
			/* Checking for the auth type and operate accordingly */
			if(scan_result->bss.AuthMode & WSC_AUTHTYPE_DPP) {
#ifdef NL80211_SUPPORT
				wapp_set_bssid(wapp, (const char *)wdev->ifname,
						(char *)scan_result->bss.Bssid, MAC_ADDR_LEN);
#else
				os_memset(cmd, 0, 100);
				ret = snprintf(cmd, 100, "iwpriv %s set ApCliBssid=%02x:%02x:%02x:%02x:%02x:%02x",
						wdev->ifname, PRINT_MAC(scan_result->bss.Bssid));
				if (os_snprintf_error(100, ret))
					DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
				ret = system(cmd);
				if (ret != 0)
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s, error status_code:%d\n", __func__, ret);

				DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
#endif /* NL80211_SUPPORT */
				bh = os_zalloc(sizeof(*bh));
				if(bh == NULL) {
					DBGPRINT(RT_DEBUG_ERROR,"%s: BH Malloc failed\n", __func__);
					return -1;
				}
				os_memcpy(bh->backhaul_mac, wdev->mac_addr, ETH_ALEN);
				os_memcpy(bh->target_bssid, scan_result->bss.Bssid, ETH_ALEN);
				os_memcpy(bh->target_ssid, scan_result->bss.Ssid, scan_result->bss.SsidLen);
				bh->target_ssid[scan_result->bss.SsidLen] = '\0';
				bh->channel = scan_result->bss.Channel;
				if (scan_result->bss.Channel != wdev->radio->op_ch)
					eloop_register_timeout(1, 0, wapp_dpp_check_conn_wrapper, wapp, bh);
				else {
					wapp->map->ch_change_done = 1;/*Since no channel change*/
					eloop_register_timeout(0, 0, wapp_dpp_check_conn_wrapper, wapp, bh);
				}
				//wapp_dpp_check_connect(wapp, wdev, (char *)scan_result->bss.Bssid, (unsigned char)scan_result->bss.Channel);
			}
			else {
				wdev_enable_apcli_iface(wapp, wdev, 1);
			}
#else
			wapp_dpp_check_connect(wapp, wdev, (char *)scan_result->bss.Bssid, (unsigned char)scan_result->bss.Channel);
#endif /* MAP_R3 */
			/* Free the scan results before returning */
			dl_list_for_each_safe(scan_result, scan_result_tmp, &(wapp->scan_results_list),
					struct bss_info_scan_result, list) {
				dl_list_del(&scan_result->list);
				os_free(scan_result);
			}
			return 0;
		}
	}
	/* if AP not found try again */
	if (wdev->connection_tries < wapp->dpp->dpp_max_connection_tries)
		return wapp_wdev_dpp_scan(wapp,wdev);
	else {
#ifdef DPP_R2_SUPPORT
		struct dpp_authentication *auth = NULL;

		dl_list_for_each(auth, &wapp->dpp->dpp_auth_list, struct dpp_authentication, list) {
			if (os_strcmp((const char *)(auth->wdev->config->ssid), (const char *)(wdev->config->ssid)) == 0)
				break;
		}
		if (auth && (auth->conn_status_requested == 1)) {
			wapp_dpp_send_conn_status_result(wapp, auth, DPP_STATUS_NO_AP);
		}
#endif /* DPP_R2_SUPPORT */
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"AP not found\n");
	}

	dl_list_for_each_safe(scan_result, scan_result_tmp, &(wapp->scan_results_list),
								struct bss_info_scan_result, list) {
		dl_list_del(&scan_result->list);
		os_free(scan_result);
	}

	return -1;
}

int dpp_set_conf_akm(struct dpp_configuration *conf, char *token)
{
	if (os_strcmp(token, "psk") == 0)
		conf->akm = DPP_AKM_PSK;
	else if (os_strcmp(token, "sae") == 0)
		conf->akm = DPP_AKM_SAE;
	else if ((os_strcmp(token, "psk-sae") == 0)||
		 (os_strcmp(token, "psk+sae") == 0))
		conf->akm = DPP_AKM_PSK_SAE;
	else if ((os_strcmp(token, "sae-dpp") == 0) ||
		 (os_strcmp(token, "dpp+sae") == 0))
		conf->akm = DPP_AKM_SAE_DPP;
	else if ((os_strcmp(token, "psk-sae-dpp") == 0) ||
		 (os_strcmp(token, "dpp+psk+sae") == 0))
		conf->akm = DPP_AKM_PSK_SAE_DPP;
	else if (os_strcmp(token, "dpp") == 0)
		conf->akm = DPP_AKM_DPP;
	else {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed to find correct akm\n");
		return -1;
	}

	return 0;
}

int dpp_read_config_file(struct dpp_global *dpp)
{
	FILE *file;
	char buf[512], *pos, *token;
	char tmpbuf[512];
	int line = 0;

	if (!dpp) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"\033[1;31m %s, %u \033[0m\n", __FUNCTION__, __LINE__);
		return WAPP_INVALID_ARG;
	}

	struct wifi_app *wapp = (struct wifi_app *)dpp->msg_ctx;
#ifndef MAP_R3
	int radio_count = wapp_get_valid_radio_count(wapp);
#endif /* MAP_R3 */
	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"%s\n", __func__);

	if (!wapp) {
                DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"\033[1;31m %s, %u \033[0m\n", __FUNCTION__, __LINE__);
                return WAPP_INVALID_ARG;
        }

	file = fopen(DPP_CFG_FILE, "r");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"open DPP cfg file (%s) fail\n", DPP_CFG_FILE);
		return WAPP_NOT_INITIALIZED;
	}

	os_memset(buf, 0, 256);
	os_memset(tmpbuf, 0, 256);
#ifdef DPP_R2_MUOBJ
	dpp->dpp_conf_ap_num = DPP_CONF_OBJ_MAX;
	dpp->dpp_conf_sta_num = DPP_CONF_OBJ_MAX;
	os_memset(dpp->config_ap, 0, sizeof(struct dpp_configuration) * DPP_CONF_OBJ_MAX);
	os_memset(dpp->config_sta, 0, sizeof(struct dpp_configuration) * DPP_CONF_OBJ_MAX);
#endif /* DPP_R2_MUOBJ */

	while (wapp_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
		os_strlcpy(tmpbuf, pos, sizeof(tmpbuf));
		token = strtok(pos, "=");

		/* TODO initialize basic parameters */
		os_strlcpy(dpp->curve_name, "prime256v1", sizeof(dpp->curve_name));
		if (token == NULL)
		{
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"token not found in (%s)\n", DPP_CFG_FILE);
			if (file != NULL && fclose(file) != 0)
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"fclose or file not found (%s)\n", DPP_CFG_FILE);

			return -1;
		}
			if (os_strcmp(token, "dpp_private_key") == 0) {
				token = strtok(NULL, "");
				if (token)
					os_strlcpy(dpp->dpp_private_key, token, sizeof(dpp->dpp_private_key));
#ifdef MAP_R3
			}  else if (os_strcmp(token, "dpp_macaddr_key") == 0) {
				token = strtok(NULL, "");
				if (token)
					os_strlcpy(dpp->dpp_macaddr_key, token, sizeof(dpp->dpp_macaddr_key));

			}  else if (os_strcmp(token, "dpp_chan_list") == 0) {
				token = strtok(NULL, "");
				if (token)
					os_strlcpy(dpp->dpp_chan_list, token, sizeof(dpp->dpp_chan_list));

			}  else if (os_strcmp(token, "presence_band_priority") == 0) {
				token = strtok(NULL, "");
				if (token)
					os_strlcpy(dpp->band_priority, token, sizeof(dpp->band_priority));
#endif /* MAP_R3 */
			}  else if (os_strcmp(token, "configurator_support") == 0) {
				token = strtok(NULL, "");
				if (token)
					dpp->dpp_configurator_supported = atoi(token);
			/* 1 enrollee, 2 configurator, 3 both(1 and 2) , 4 proxy agent*/
			}  else if (os_strcmp(token, "allowed_role") == 0) {
				token = strtok(NULL, "");
				if (token)
					dpp->dpp_allowed_roles = atoi(token);

				if(dpp->dpp_allowed_roles == DPP_CAPAB_CONFIGURATOR)
					dpp->dpp_configurator_supported = 1;
#ifndef MAP_R3
			}  else if (os_strcmp(token, "dpp_interface_2g") == 0) {
				token = strtok(NULL, "");
				if (token)
					dpp->default_2g_iface = wapp_dev_list_lookup_by_ifname(wapp, token);

				if(dpp->default_2g_iface == NULL)
                                        continue;
				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"2g interface is: %s\n", dpp->default_2g_iface->ifname);
			}  else if (os_strcmp(token, "dpp_interface_5gl") == 0) {
				token = strtok(NULL, "");
				if (token)
					dpp->default_5gl_iface = wapp_dev_list_lookup_by_ifname(wapp, token);
				/* condition for DBDC */
				if(radio_count == MAX_RADIO_DBDC){
					if (token)
						dpp->default_5gh_iface = wapp_dev_list_lookup_by_ifname(wapp, token);

					if(dpp->default_5gh_iface == NULL)
						continue;
				}
				if(dpp->default_5gl_iface == NULL)
                                        continue;
				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"5gl interface is: %s\n", dpp->default_5gl_iface->ifname);
#ifdef MAP_R3  //TODO Currently handled for triband needs to be done for dual band
			}  else if (os_strcmp(token, "dpp_interface_5gh") == 0) {
#else
			}  else if ((os_strcmp(token, "dpp_interface_5gh") == 0) && (radio_count == MAP_MAX_RADIO)) {
#endif /* MAP_R3 */
				token = strtok(NULL, "");
				if (token)
					dpp->default_5gh_iface = wapp_dev_list_lookup_by_ifname(wapp, token);

				if(dpp->default_5gh_iface == NULL)
                                        continue;
				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"5gh interface is: %s\n", dpp->default_5gh_iface->ifname);
#endif /* MAP_R3 */
			}  else if (os_strcmp(token, "curve_name") == 0) {
				token = strtok(NULL, "");
				if (token)
					os_strlcpy(dpp->curve_name, token, sizeof(dpp->curve_name));
#ifdef MAP_R3_RECONFIG
			}  else if (os_strcmp(token, "reconf_announce_try") == 0) {
				token = strtok(NULL, "");
				if (token)
					dpp->dpp_reconf_announce_try = atoi(token);

				if (dpp->dpp_reconf_announce_try < DPP_RECONF_CH_TRY_MIN) {
					dpp_auth_fail_wrapper(wapp, "Given value less than allowed, keeping retry value as 2\n");
					dpp->dpp_reconf_announce_try = DPP_RECONF_CH_TRY_MIN;
				}
#endif /* MAP_R3_RECONFIG */
			}  else if (os_strcmp(token, "map_support") == 0) {
				token = strtok(NULL, "");
				if (token)
					dpp->is_map = atoi(token);
			}  else if (os_strcmp(token, "max_conn_retries") == 0) {
				token = strtok(NULL, "");
				if (token)
					dpp->dpp_max_connection_tries = atoi(token);
			}
#ifdef DPP_R2_MUOBJ
			else if (os_strcmp(token, "ap_config") == 0) {
				unsigned int num = 0;
				token = strtok(NULL, "");
				if (token)
					num = atoi(token);

				if(num > DPP_CONF_OBJ_MAX)
					num = DPP_CONF_OBJ_MAX;

				if(num < dpp->dpp_conf_ap_num) {
					dpp_free_config_info(&(dpp->config_ap[0]), num, dpp->dpp_conf_ap_num);
					dpp->dpp_conf_ap_num = num;
				}
				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"config ap num(%d) \n", dpp->dpp_conf_ap_num);
			// TODO move inside of this if block
			}  else if (os_strcmp(token, "ap_ssid") == 0) {
				unsigned int i = 0;
				token = strtok(NULL, "");
				token = strtok(token, ";");

				while (token) {
					os_strlcpy((char *)(dpp->config_ap[i].ssid), token,
						sizeof(dpp->config_ap[i].ssid));
					dpp->config_ap[i].ssid_len = os_strlen(token);
					DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"config ap ssid(%d, len:%ld): %s\n", i, dpp->config_ap[i].ssid_len, 
						dpp->config_ap[i].ssid);
					i++;
					if(i >= dpp->dpp_conf_ap_num)
						break;
					token = strtok(NULL, ";");
				}

				if(i < dpp->dpp_conf_ap_num) {
					dpp_free_config_info(&(dpp->config_ap[0]), i, dpp->dpp_conf_ap_num);
					dpp->dpp_conf_ap_num = i;
				}
			}  else if (os_strcmp(token, "ap_pass") == 0) {
				size_t pass_len;
				unsigned int i = 0;
				token = strtok(NULL, "");
				token = strtok(token, ";");

				while (token) {
					pass_len = os_strlen(token);
					dpp->config_ap[i].passphrase = os_zalloc(pass_len + 1);
					if(!dpp->config_ap[i].passphrase)
						goto  fail;
					os_strlcpy((char *)(dpp->config_ap[i].passphrase), token,
						sizeof((dpp->config_ap[i].passphrase));
					DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"config ap passphrase(%d, len:%ld): %s\n", i, pass_len, 
						dpp->config_ap[i].passphrase);
					i++;
					if(i >= dpp->dpp_conf_ap_num)
						break;
					token = strtok(NULL, ";");
				}

				if(i < dpp->dpp_conf_ap_num) {
					dpp_free_config_info(&(dpp->config_ap[0]), i, dpp->dpp_conf_ap_num);
					dpp->dpp_conf_ap_num = i;
				}
			}  else if (os_strcmp(token, "ap_group_id") == 0) {
				size_t group_id_len;
				unsigned int i = 0;
				token = strtok(NULL, "");
				token = strtok(token, ";");

				while (token) {
					group_id_len = os_strlen(token);
					dpp->config_ap[i].group_id = os_zalloc(group_id_len + 1);
					if(!dpp->config_ap[i].group_id)
						goto  fail;
					os_memcpy(dpp->config_ap[i].group_id, token, group_id_len);
					DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"config ap group_id(%d, len:%ld): %s\n", i, group_id_len, 
						dpp->config_ap[i].group_id);
					i++;
					if(i >= dpp->dpp_conf_ap_num)
						break;
					token = strtok(NULL, ";");
				}

				if(i < dpp->dpp_conf_ap_num) {
					dpp_free_config_info(&(dpp->config_ap[0]), i, dpp->dpp_conf_ap_num);
					dpp->dpp_conf_ap_num = i;
				}
			}  else if (os_strcmp(token, "ap_expiry") == 0) {
				unsigned int i = 0;
				token = strtok(NULL, "");
				token = strtok(token, ";");

				while (token) {
					dpp->config_ap[i].netaccesskey_expiry =  atoi(token);
					DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"config ap expiry(%d): %ld\n", i, dpp->config_ap[i].netaccesskey_expiry);
					i++;
					if(i >= dpp->dpp_conf_ap_num)
						break;
					token = strtok(NULL, ";");
				}

				if(i < dpp->dpp_conf_ap_num) {
					dpp_free_config_info(&(dpp->config_ap[0]), i, dpp->dpp_conf_ap_num);
					dpp->dpp_conf_ap_num = i;
				}
			}  else if (os_strcmp(token, "ap_akm") == 0) {
				unsigned int i = 0;
				token = strtok(NULL, "");
				token = strtok(token, ";");

				while (token) {
					dpp_set_conf_akm(&(dpp->config_ap[i]), token);
					DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"config ap akm(%d): %d\n", i, dpp->config_ap[i].akm);
					i++;
					if(i >= dpp->dpp_conf_ap_num)
						break;
					token = strtok(NULL, ";");
				}

				if(i < dpp->dpp_conf_ap_num) {
					dpp_free_config_info(&(dpp->config_ap[0]), i, dpp->dpp_conf_ap_num);
					dpp->dpp_conf_ap_num = i;
				}
			}  else if (os_strcmp(token, "sta_config") == 0) {
				unsigned int num = 0;
				token = strtok(NULL, "");
				num = atoi(token);
				if(num > DPP_CONF_OBJ_MAX)
					num = DPP_CONF_OBJ_MAX;

				if(num < dpp->dpp_conf_sta_num) {
					dpp_free_config_info(&(dpp->config_sta[0]), num, dpp->dpp_conf_sta_num);
					dpp->dpp_conf_sta_num = num;
				}
			}  else if (os_strcmp(token, "sta_ssid") == 0) {
				unsigned int i = 0;
				token = strtok(NULL, "");
				token = strtok(token, ";");

				while (token) {
					os_strlcpy((char *)(dpp->config_sta[i].ssid), token, sizeof(dpp->config_sta[i].ssid));
					dpp->config_sta[i].ssid_len = os_strlen(token);
					DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"config sta ssid(%d, len:%ld): %s\n", i, dpp->config_sta[i].ssid_len, 
						dpp->config_sta[i].ssid);
					i++;
					if(i >= dpp->dpp_conf_sta_num)
						break;
					token = strtok(NULL, ";");
				}

				if(i < dpp->dpp_conf_sta_num) {
					dpp_free_config_info(&(dpp->config_sta[0]), i, dpp->dpp_conf_sta_num);
					dpp->dpp_conf_sta_num = i;
				}
			}  else if (os_strcmp(token, "sta_pass") == 0) {
				size_t pass_len;
				unsigned int i = 0;
				token = strtok(NULL, "");
				token = strtok(token, ";");

				while (token) {
					pass_len = os_strlen(token);
					dpp->config_sta[i].passphrase = os_zalloc(pass_len + 1);
					if(!dpp->config_sta[i].passphrase)
						goto  fail;
					os_strlcpy((char *)(dpp->config_sta[i].passphrase), token, sizeof(dpp->config_sta[i].passphrase));
					DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"config sta passphrase(%d, len:%ld): %s\n", i, pass_len, 
						dpp->config_sta[i].passphrase);
					i++;
					if(i >= dpp->dpp_conf_sta_num)
						break;
					token = strtok(NULL, ";");
				}
				if(i < dpp->dpp_conf_sta_num) {
					dpp_free_config_info(&(dpp->config_sta[0]), i, dpp->dpp_conf_sta_num);
					dpp->dpp_conf_sta_num = i;
				}
			}  else if (os_strcmp(token, "sta_group_id") == 0) {
				size_t group_id_len;
				unsigned int i = 0;
				token = strtok(NULL, "");
				token = strtok(token, ";");

				while (token) {
					group_id_len = os_strlen(token);
					dpp->config_sta[i].group_id = os_zalloc(group_id_len + 1);
					if(!dpp->config_sta[i].group_id)
						goto  fail;
					os_memcpy(dpp->config_sta[i].group_id, token, group_id_len);
					DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"config sta group_id(%d, len:%ld): %s\n", i, group_id_len, 
						dpp->config_sta[i].group_id);
					i++;
					if(i >= dpp->dpp_conf_sta_num)
						break;
					token = strtok(NULL, ";");
				}
				if(i < dpp->dpp_conf_sta_num) {
					dpp_free_config_info(&(dpp->config_sta[0]), i, dpp->dpp_conf_sta_num);
					dpp->dpp_conf_sta_num = i;
				}

			}  else if (os_strcmp(token, "sta_expiry") == 0) {
				unsigned int i = 0;
				token = strtok(NULL, "");
				token = strtok(token, ";");

				while (token) {
					dpp->config_sta[i].netaccesskey_expiry =  atoi(token);
					DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"config sta expiry(%d): %ld\n", i, dpp->config_sta[i].netaccesskey_expiry);
					i++;
					if(i >= dpp->dpp_conf_sta_num)
						break;
					token = strtok(NULL, ";");
				}

				if(i < dpp->dpp_conf_sta_num) {
					dpp_free_config_info(&(dpp->config_sta[0]), i, dpp->dpp_conf_sta_num);
					dpp->dpp_conf_sta_num = i;
				}
			}  else if (os_strcmp(token, "sta_akm") == 0) {
				unsigned int i = 0;
				token = strtok(NULL, "");
				token = strtok(token, ";");

				while (token) {
					dpp_set_conf_akm(&(dpp->config_sta[i]), token);
					DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"config sta akm(%d): %d\n", i, dpp->config_sta[i].akm);
					i++;
					if(i >= dpp->dpp_conf_sta_num)
						break;
					token = strtok(NULL, ";");
				}

				if(i < dpp->dpp_conf_sta_num) {
					dpp_free_config_info(&(dpp->config_sta[0]), i, dpp->dpp_conf_sta_num);
					dpp->dpp_conf_sta_num = i;
				}
			}
#else
			else if (os_strcmp(token, "ap_config") == 0) {
				struct dpp_configuration *conf;
				conf = os_zalloc(sizeof(*conf));
				if (!conf) {
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"AP Conf read failure\n");
					if (file != NULL && fclose(file) != 0)
						DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"fclose error %s\n", __func__);

					return -1;
				}
				dpp->conf_ap = conf;
			// TODO move inside of this if block
			}  else if (os_strcmp(token, "ap_ssid") == 0) {
				struct dpp_configuration *conf;
				conf = dpp->conf_ap;
				token = strtok(NULL, "");
				if (token == NULL)
					continue;

				os_strlcpy((char *)conf->ssid, token, sizeof(conf->ssid));
				conf->ssid_len = os_strlen(token);
			}  else if (os_strcmp(token, "ap_pass") == 0) {
				struct dpp_configuration *conf;
				size_t pass_len;
				conf = dpp->conf_ap;
				token = strtok(NULL, "");
				if (token == NULL)
					continue;

				pass_len = os_strlen(token);
				conf->passphrase = os_zalloc(pass_len + 1);
				os_strlcpy((char *)conf->passphrase, token, pass_len + 1);
			}  else if (os_strcmp(token, "ap_group_id") == 0) {
				struct dpp_configuration *conf;
				size_t group_id_len;
				token = strtok(NULL, "");
				if (token == NULL)
					continue;

				conf = dpp->conf_ap;
				group_id_len = os_strlen(token);
				conf->group_id = os_zalloc(group_id_len + 1);
				os_memcpy(conf->group_id, token, group_id_len);
			}  else if (os_strcmp(token, "ap_expiry") == 0) {
				struct dpp_configuration *conf;
				conf = dpp->conf_ap;
				token = strtok(NULL, "");
				if (token)
					conf->netaccesskey_expiry = atoi(token);

			}  else if (os_strcmp(token, "ap_akm") == 0) {
				struct dpp_configuration *conf;
				conf = dpp->conf_ap;
				token = strtok(NULL, "");
				if (token)
					dpp_set_conf_akm(conf, token);

			}  else if (os_strcmp(token, "sta_config") == 0) {
				struct dpp_configuration *conf;
				conf = os_zalloc(sizeof(*conf));
				if (!conf) {
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Sta Conf read failure\n");
					if (file != NULL && fclose(file) != 0)
						DBGPRINT(RT_DEBUG_ERROR, "%s %d fclose or invalid fp error\n", __func__, __LINE__);

					return -1;
				}
				dpp->conf_sta = conf;
			}  else if (os_strcmp(token, "sta_ssid") == 0) {
				struct dpp_configuration *conf;
				conf = dpp->conf_sta;
				token = strtok(NULL, "");
				if (token == NULL)
					continue;

				os_strlcpy((char *)conf->ssid, token, sizeof(conf->ssid));
				conf->ssid_len = os_strlen(token);
			}  else if (os_strcmp(token, "sta_pass") == 0) {
				struct dpp_configuration *conf;
				size_t pass_len;
				conf = dpp->conf_sta;
				token = strtok(NULL, "");
				if (token == NULL)
					continue;

				pass_len = os_strlen(token);
				conf->passphrase = os_zalloc(pass_len + 1);
				os_strlcpy((char *)conf->passphrase, token, pass_len + 1);
			}  else if (os_strcmp(token, "sta_group_id") == 0) {
				struct dpp_configuration *conf;
				size_t group_id_len;
				token = strtok(NULL, "");
				if (token == NULL)
					continue;

				conf = dpp->conf_sta;
				group_id_len = os_strlen(token);
				conf->group_id = os_zalloc(group_id_len + 1);
				os_memcpy(conf->group_id, token, group_id_len);
			}  else if (os_strcmp(token, "sta_expiry") == 0) {
				struct dpp_configuration *conf;
				conf = dpp->conf_sta;
				token = strtok(NULL, "");
				if (token)
					conf->netaccesskey_expiry = atoi(token);

			}  else if (os_strcmp(token, "sta_akm") == 0) {
				struct dpp_configuration *conf;
				conf = dpp->conf_sta;
				token = strtok(NULL, "");
				if (token)
					dpp_set_conf_akm(conf, token);
			}
#endif /*DPP_R2_MUOBJ */
#ifdef DPP_R2_SUPPORT
			else if (os_strcmp(token, "mud_url") == 0) {
				token = strtok(NULL, "");
				if (token)
					os_strlcpy(dpp->dpp_mud_url, token, sizeof(dpp->dpp_mud_url));
			}
#endif /* DPP_R2_SUPPORT */
		}

	if (fclose(file) != 0)
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX" fclose1 error, %s\n", __func__);
#ifdef DPP_R2_MUOBJ
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"config ap/sta num (%d,%d)\n", dpp->dpp_conf_ap_num, dpp->dpp_conf_sta_num);
#endif /*DPP_R2_MUOBJ */
	return WAPP_SUCCESS;

#ifdef DPP_R2_MUOBJ
fail:
	if (fclose(file) != 0)
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX" fclose2 error, %s\n", __func__);
	//free buf
	dpp_free_config_info(&(dpp->config_ap[0]), 0, dpp->dpp_conf_ap_num);
	dpp_free_config_info(&(dpp->config_sta[0]), 0, dpp->dpp_conf_sta_num);
	return WAPP_RESOURCE_ALLOC_FAIL;
#endif /*DPP_R2_MUOBJ */
}

void wapp_dpp_deinit(struct wifi_app *wapp)
{
	if (!wapp->dpp)
		return;

	dpp_global_clear(wapp->dpp);
	eloop_cancel_timeout(wapp_dpp_pkex_retry_timeout, wapp, NULL);
#ifdef CONFIG_DPP2
	//dpp_pfs_free(wapp->dpp->dpp_pfs);
	//wapp->dpp->dpp_pfs = NULL;
#endif /* CONFIG_DPP2 */
	wapp_dpp_stop(wapp);
	wapp_dpp_pkex_remove(wapp, "*");
}

int dpp_save_config(struct wifi_app *wapp, const char *param, const char *value, char *ifname)
{
#ifdef OPENWRT_SUPPORT
	struct kvc_context *dat_ctx = NULL;
	char *ifparam;
	int ret = 0;

	os_alloc_mem(NULL, (UCHAR**)&ifparam, IFNAMSIZ + os_strlen(param));
	if(ifparam == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s, mem alloc fail \n", __func__);
		goto out;
	}
	NdisZeroMemory(ifparam, IFNAMSIZ + os_strlen(param));
	if(ifname != NULL)
		strncat(ifparam, ifname, os_strlen(ifname));
	strncat(ifparam, param, os_strlen(param));
	dat_ctx = dat_load(DPP_CFG_FILE);
	if (!dat_ctx) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"load file(%s) fail\n", DPP_CFG_FILE);
		ret = -1;
		goto out;
	}
	ret = kvc_set(dat_ctx, (const char *)ifparam, (const char *)value);
	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"set param(%s) fail\n", param);
		goto out;
	}
	ret = kvc_commit(dat_ctx);
	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"write param(%s) fail\n", param);
		goto out;
	}
	os_free_mem(NULL, ifparam);
	ifparam = NULL;
out:
	if (dat_ctx)
		kvc_unload(dat_ctx);
	if(ifparam) {
		os_free_mem(NULL, ifparam);
	}
	if (ret)
		return -1;
	else
#endif /* OPENWRT_SUPPORT */
		return 0;
}

#ifdef DPP_R2_MUOBJ
void dpp_save_config_to_file(struct wifi_app *wapp, struct dpp_authentication *auth, struct dpp_config_obj *conf)
{
	char *strbuf;
	struct wsc_apcli_config apcli_config;
	os_alloc_mem(NULL, (UCHAR**)&strbuf, 512);
	NdisZeroMemory(strbuf, 512);
	dpp_save_config(wapp, "_ssid", os_ssid_txt(conf->ssid, conf->ssid_len), auth->wdev->ifname);
	os_memcpy(apcli_config.ssid, conf->ssid, conf->ssid_len);
	apcli_config.SsidLen = conf->ssid_len;
	snprintf(strbuf,512, "%i", conf->akm);
	dpp_save_config(wapp, "_akm", strbuf, auth->wdev->ifname);
	NdisZeroMemory(strbuf, 512);

	switch (conf->akm) {
	case DPP_AKM_DPP:
	case DPP_AKM_SAE_DPP:
		dpp_save_config(wapp, "_connector", conf->connector, auth->wdev->ifname);
		if (auth->net_access_key) {
			os_snprintf_hex(strbuf, 512, auth->net_access_key->buf, auth->net_access_key->used);
			dpp_save_config(wapp, "_netAccessKey", strbuf, auth->wdev->ifname);
			NdisZeroMemory(strbuf, 512);
		}

		os_snprintf_hex(strbuf, 512, auth->c_sign_key->buf, auth->c_sign_key->used);
		dpp_save_config(wapp, "_cSignKey", strbuf, auth->wdev->ifname);
		NdisZeroMemory(strbuf, 512);
		if (conf->akm == DPP_AKM_DPP)
			apcli_config.AuthType = WPS_AUTH_DPP;
		if (conf->akm == DPP_AKM_SAE_DPP)
			apcli_config.AuthType = WPS_AUTH_DPP | WSC_AUTHTYPE_SAE;
		break;
	case DPP_AKM_PSK_SAE_DPP:
		dpp_save_config(wapp, "_connector", conf->connector, auth->wdev->ifname);
		if (auth->net_access_key) {
			os_snprintf_hex(strbuf, 512, auth->net_access_key->buf, auth->net_access_key->used);
			dpp_save_config(wapp, "_netAccessKey", strbuf, auth->wdev->ifname);
			NdisZeroMemory(strbuf, 512);
		}

		os_snprintf_hex(strbuf, 512, auth->c_sign_key->buf, auth->c_sign_key->used);
		dpp_save_config(wapp, "_cSignKey", strbuf, auth->wdev->ifname);
		NdisZeroMemory(strbuf, 512);
	case DPP_AKM_PSK:
	case DPP_AKM_PSK_SAE:
		dpp_save_config(wapp, "_passPhrase", conf->passphrase, auth->wdev->ifname);
		os_memcpy(apcli_config.Key, conf->passphrase, os_strlen(conf->passphrase));
		apcli_config.KeyLength = os_strlen(conf->passphrase);
		if (conf->akm == DPP_AKM_PSK_SAE_DPP)
			apcli_config.AuthType = WPS_AUTH_DPP | WSC_AUTHTYPE_SAE | WSC_AUTHTYPE_WPA2PSK;
		if (conf->akm == DPP_AKM_PSK_SAE)
			apcli_config.AuthType = WSC_AUTHTYPE_SAE | WSC_AUTHTYPE_WPA2PSK;
		if (conf->akm == DPP_AKM_PSK)
			apcli_config.AuthType = WSC_AUTHTYPE_WPA2PSK;
		break;
	case DPP_AKM_UNKNOWN:
	case DPP_AKM_SAE:
		if (conf->akm == DPP_AKM_SAE)
			apcli_config.AuthType = WSC_AUTHTYPE_SAE;
		break;
	}
	NdisZeroMemory(strbuf, 512);
	dpp_save_config(wapp, "_valid", 1, auth->wdev->ifname);
	os_free_mem(NULL, strbuf);
	apcli_config.EncrType = WSC_ENCRTYPE_AES;
	write_configs(wapp, &apcli_config, 0, NULL);
	return;
}
#else

void dpp_save_config_to_file(struct wifi_app *wapp, struct dpp_authentication *auth)
{
	char *strbuf = NULL;
	wsc_apcli_config apcli_config;
	char ra_match[8] = {0};
	int i = 0;
	int ret;
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list;
	struct map_radio_identifier *ra_identifier = os_zalloc(sizeof(struct map_radio_identifier));
	os_alloc_mem(NULL, (UCHAR**)&strbuf, max_buf_len);
	if (strbuf == NULL) {
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"Error: malloc fail\n");
		if(ra_identifier)
			os_free(ra_identifier);
		
		return;
	}
	os_memset(&apcli_config, '\0', sizeof(wsc_apcli_config));
	NdisZeroMemory(strbuf, max_buf_len);
	if (wapp->dpp->is_map && auth->is_1905_connector == 0) {
		dpp_save_config(wapp, "_ssid", os_ssid_txt(auth->ssid, auth->ssid_len), NULL);
		os_memcpy(apcli_config.ssid, auth->ssid, auth->ssid_len);
		apcli_config.SsidLen = auth->ssid_len;
	}
	ret = snprintf(strbuf, max_buf_len, "%i", auth->akm);
	if (os_snprintf_error(max_buf_len, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	dpp_save_config(wapp, "_akm", strbuf, NULL);
	NdisZeroMemory(strbuf, max_buf_len);
	switch (auth->akm) {
	case DPP_AKM_DPP:
	case DPP_AKM_SAE_DPP:
	case DPP_AKM_PSK_DPP:
	case DPP_AKM_PSK_SAE_DPP:
		if(dpp_akm_ver2(auth->akm)) {
			if (auth->is_1905_connector == 0) {
				dpp_save_config(wapp, "_passPhrase", auth->passphrase, NULL);
				os_memcpy(apcli_config.Key, auth->passphrase, os_strlen(auth->passphrase));
				apcli_config.KeyLength = os_strlen(auth->passphrase);
			}
		}

		if (wapp->dpp->is_map && auth->is_1905_connector) {
			dpp_save_config(wapp, "_1905valid", "1", NULL);
			NdisZeroMemory(strbuf, max_buf_len);
			if (auth->connector) {
				dpp_save_config(wapp, "_1905connector", auth->connector, NULL);
				hex_dump_dbg("save 1905 connector", \
					(unsigned char *)auth->connector, \
					os_strlen(auth->connector));
			}
			NdisZeroMemory(strbuf, max_buf_len);
			if (auth->net_access_key) {
				os_snprintf_hex(strbuf, max_buf_len, auth->net_access_key->buf, auth->net_access_key->used);
				dpp_save_config(wapp, "_1905netAccessKey", strbuf, NULL);
			}
			NdisZeroMemory(strbuf, max_buf_len);
			if (auth->c_sign_key) {
				os_snprintf_hex(strbuf, max_buf_len,
					 auth->c_sign_key->buf,
					 auth->c_sign_key->used);
				dpp_save_config(wapp, "_1905cSignKey", strbuf, NULL);
			}
			NdisZeroMemory(strbuf, max_buf_len);
			ret = snprintf(strbuf, max_buf_len, "%ld", auth->net_access_key_expiry);
			if (os_snprintf_error(max_buf_len, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			dpp_save_config(wapp, "_netKeyExpiry", strbuf, NULL);
			NdisZeroMemory(strbuf, max_buf_len);
			ret = snprintf(strbuf, max_buf_len, "%i", auth->decrypt_thresold);
			if (os_snprintf_error(max_buf_len, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
			dpp_save_config(wapp, "_decryptThreshold", strbuf, NULL);
		} else {
			dpp_save_config(wapp, "_connector", auth->connector, NULL);
			if (auth->net_access_key) {
				os_snprintf_hex(strbuf, max_buf_len, auth->net_access_key->buf, auth->net_access_key->used);
				dpp_save_config(wapp, "_netAccessKey", strbuf, NULL);
				NdisZeroMemory(strbuf, max_buf_len);
			}

			if (auth->c_sign_key) {
				os_snprintf_hex(strbuf, max_buf_len, \
					auth->c_sign_key->buf, \
					auth->c_sign_key->used);
				dpp_save_config(wapp, "_cSignKey", strbuf, NULL);
			}
			NdisZeroMemory(strbuf, max_buf_len);
		}
		if (auth->akm == DPP_AKM_DPP)
			apcli_config.AuthType = WSC_AUTHTYPE_DPP;
		else if (auth->akm == DPP_AKM_SAE_DPP)
			apcli_config.AuthType = WSC_AUTHTYPE_DPP | WSC_AUTHTYPE_SAE;
		else if (auth->akm == DPP_AKM_PSK_DPP)
			apcli_config.AuthType = WSC_AUTHTYPE_DPP | WSC_AUTHTYPE_WPA2PSK;
		else if (auth->akm == DPP_AKM_PSK_SAE_DPP)
			apcli_config.AuthType = WSC_AUTHTYPE_DPP | WSC_AUTHTYPE_SAE | WSC_AUTHTYPE_WPA2PSK;
		break;
	case DPP_AKM_PSK:
	case DPP_AKM_SAE:
	case DPP_AKM_PSK_SAE:
		dpp_save_config(wapp, "_connector", auth->connector, NULL);
		if (auth->net_access_key) {
			os_snprintf_hex(strbuf, max_buf_len, auth->net_access_key->buf, auth->net_access_key->used);
			dpp_save_config(wapp, "_netAccessKey", strbuf, NULL);
		}
		NdisZeroMemory(strbuf, max_buf_len);
		if (auth->c_sign_key) {
			os_snprintf_hex(strbuf, max_buf_len, auth->c_sign_key->buf, auth->c_sign_key->used);
			dpp_save_config(wapp, "_cSignKey", strbuf, NULL);
		}
		NdisZeroMemory(strbuf, max_buf_len);
		dpp_save_config(wapp, "_passPhrase", auth->passphrase, NULL);
		os_memcpy(apcli_config.Key, auth->passphrase, os_strlen(auth->passphrase));
		apcli_config.KeyLength = os_strlen(auth->passphrase);
		if (auth->akm == DPP_AKM_PSK_SAE)
			apcli_config.AuthType = WSC_AUTHTYPE_SAE | WSC_AUTHTYPE_WPA2PSK;
		else if (auth->akm == DPP_AKM_PSK)
			apcli_config.AuthType = WSC_AUTHTYPE_WPA2PSK;
		else if (auth->akm == DPP_AKM_SAE)
			apcli_config.AuthType = WSC_AUTHTYPE_SAE;
		break;
	case DPP_AKM_UNKNOWN:
		break;
	}
#ifdef MAP_R3_RECONFIG
	NdisZeroMemory(strbuf, max_buf_len);
	if (auth->pp_key && auth->pp_key->buf) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX
				DPP_EVENT_PP_KEY "%s", auth->pp_key->buf);
	        os_snprintf_hex(strbuf, max_buf_len, auth->pp_key->buf, auth->pp_key->used);
        	dpp_save_config(wapp, "_ppkey", strbuf, NULL);
	}
#endif
	NdisZeroMemory(strbuf, max_buf_len);
	if (wapp->dpp->is_map && auth->is_1905_connector == 0) {
		dpp_save_config(wapp, "_valid", "1", NULL);
		apcli_config.EncrType = WSC_ENCRTYPE_AES;
		dev_list = &wapp->dev_list;
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
			if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
				MAP_GET_RADIO_IDNFER(wdev->radio, ra_identifier);
				ret = os_snprintf(ra_match, sizeof(ra_match), "%02x:%02x", ra_identifier->card_id, ra_identifier->ra_id);
				if (os_snprintf_error(sizeof(ra_match), ret))
					DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
				write_configs(wapp, &apcli_config, i, ra_match);
				update_cli_config(&(wapp->map->apcli_configs[i]), &apcli_config, ra_match);
				i++;
			}
		}
	}
	if(strbuf) {
		os_free_mem(NULL, strbuf);
		strbuf = NULL;
	}
	if(ra_identifier)
		os_free(ra_identifier);

	return;
}
//#ifdef OPENWRT_SUPPORT
#endif /* DPP_R2_MUOBJ */
#ifdef MAP_R3
void dpp_save_bss_to_file(struct wifi_app *wapp, struct dpp_authentication *auth,
	struct dpp_bss_cred *bss_cred, u16 credlen)
{

	char *strbuf = NULL;
	int ret;

	os_alloc_mem(NULL, (UCHAR**)&strbuf, 10);
	if (strbuf == NULL) {
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"Error: BSS connector not save to file\n");
		return;
	}
	NdisZeroMemory(strbuf, 10);

	dpp_save_config(wapp, "_bsscred", (char *)bss_cred->payload, NULL);
	ret = snprintf(strbuf, 10, "%i", credlen);
	if (os_snprintf_error(10, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	dpp_save_config(wapp, "_bsscredlen", strbuf, NULL);
	NdisZeroMemory(strbuf, 10);
	ret = snprintf(strbuf, 10, "%i", bss_cred->bh_connect_len);
	if (os_snprintf_error(10, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	dpp_save_config(wapp, "_bhcredlen", strbuf, NULL);
	NdisZeroMemory(strbuf, 10);
	ret = snprintf(strbuf, 10, "%i", bss_cred->fh_connect_len);
	if (os_snprintf_error(10, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	dpp_save_config(wapp, "_fhcredlen", strbuf, NULL);
	NdisZeroMemory(strbuf, 10);

	if (strbuf)
		os_free_mem(NULL, strbuf);

	return;
}

void dpp_save_dpp_uri_to_file(struct wifi_app *wapp, char *uri_str)
{
	char param_str[15] = "agt_qr_code";
	char qr_param[20] = "";
	char *strbuf = NULL;
	int ret;

	os_alloc_mem(NULL, (UCHAR**)&strbuf, 10);
	if (strbuf == NULL) {
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"Error: QR code could not save to file\n");
		return;
	}
	NdisZeroMemory(strbuf, 10);

	NdisZeroMemory(qr_param, 20);

	if(wapp->dpp->qr_code_num >= MAP_MAX_DPP_URI_COUNT){
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"QR code count reached max in file,reset count\n");
		wapp->dpp->qr_code_num = 0;
	}
	wapp->dpp->qr_code_num++;
	ret = os_snprintf(qr_param, sizeof(qr_param), "%s_%u", param_str, wapp->dpp->qr_code_num);
	if (os_snprintf_error(sizeof(qr_param), ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	dpp_save_config(wapp, (const char *)qr_param, (const char *)uri_str, NULL);

	ret = snprintf(strbuf, 10, "%u", wapp->dpp->qr_code_num);
	if (os_snprintf_error(10, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	dpp_save_config(wapp, "qr_count", strbuf, NULL);
	NdisZeroMemory(strbuf, 10);

	if (strbuf)
		os_free_mem(NULL, strbuf);

	return;
}
#endif
#ifdef OPENWRT_SUPPORT
#ifdef DPP_R2_MUOBJ
static void dpp_fetch_dpp_akm_param(struct wapp_dev *wdev, struct dpp_authentication *auth,
				    struct dpp_config_obj *conf, struct kvc_context *dat_ctx)
{
	char ifparam[50];
	const char *buf;
	u8 buftemp[512];
#if NL80211_SUPPORT
	u8 Enable = 1;
#else
	char cmd[100];
#endif

	if (!wdev || !auth || ! conf || !dat_ctx)
		return;

	conf->akm = DPP_AKM_DPP;
	NdisZeroMemory(ifparam, 50);
	strcat(ifparam, wdev->ifname);
	strcat(ifparam, "_connector");
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	if (buf)
		conf->connector = os_strdup(buf);
	else
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" buffer NULL , conf connector not evaluated, %s %d\n", __func__, __LINE__);

	NdisZeroMemory(ifparam, 50);
	strcat(ifparam, wdev->ifname);
	strcat(ifparam, "_netAccessKey");
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	NdisZeroMemory(buftemp, 512);
	if (buf) {
		hexstr2bin(buf, buftemp, os_strlen(buf));
		auth->net_access_key = wpabuf_alloc(os_strlen(buf));
		NdisCopyMemory(auth->net_access_key->buf, buftemp, os_strlen(buf));
		auth->net_access_key->used = os_strlen(buf);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" buffer NULL , auth->net_access_key not evaluated, %s %d\n", __func__, __LINE__);
	}

	NdisZeroMemory(ifparam, 50);
	strcat(ifparam, wdev->ifname);
	strcat(ifparam, "_cSignKey");
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	if (buf) {
		NdisZeroMemory(buftemp, 512);
		hexstr2bin(buf, buftemp, os_strlen(buf));
		auth->c_sign_key = wpabuf_alloc(os_strlen(buf));
		NdisCopyMemory(auth->c_sign_key->buf, buftemp, os_strlen(buf));
		auth->c_sign_key->used = os_strlen(buf);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" buffer NULL , auth->c_sign_key not evaluated, %s %d\n", __func__, __LINE__);
	}

#if NL80211_SUPPORT
	wapp_set_DppEnable(wapp, (const char *)wdev->ifname,
			(char *)&Enable, 1);
#else
	os_memset(cmd, 0, 100);
	snprintf(cmd, 100,"iwpriv %s set DppEnable=1;",
			wdev->ifname);
	system(cmd);
	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s\n", cmd);
#endif

}

static void dpp_fetch_psk_akm_param(struct wapp_dev *wdev, struct dpp_config_obj *conf,
				     struct kvc_context *dat_ctx)
{
	char ifparam[50];
	const char *buf;

	conf->akm = DPP_AKM_PSK;
	NdisZeroMemory(ifparam, 50);
	strcat(ifparam, wdev->ifname);
	strcat(ifparam, "_passPhrase");
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	os_strlcpy(conf->passphrase, buf, sizeof(conf->passphrase));
}
#else
static void dpp_fetch_dpp_akm_param(struct wapp_dev *wdev, struct dpp_authentication *auth,
				    struct kvc_context *dat_ctx)
{
	char ifparam[50];
	const char *buf;
	u8 buftemp[512];
	int ret;
#if NL80211_SUPPORT
	u8 Enable = 1;
#else
	char cmd[100];
#endif

	auth->akm = DPP_AKM_DPP;
	NdisZeroMemory(ifparam, 50);
	strncat(ifparam, wdev->ifname, strlen(wdev->ifname));
	strncat(ifparam, "_connector", strlen("_connector"));
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s %d Buf is null from kvc_get\n", __func__, __LINE__);
		return;
	}
	auth->connector = os_strdup(buf);

	NdisZeroMemory(ifparam, 50);
	strncat(ifparam, wdev->ifname, strlen(wdev->ifname));
	strncat(ifparam, "_netAccessKey", strlen("_netAccessKey"));
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s %d Buf is null from kvc_get\n", __func__, __LINE__);
		return;
	}
	NdisZeroMemory(buftemp, 512);
	hexstr2bin(buf, buftemp, os_strlen(buf));
	auth->net_access_key = wpabuf_alloc(os_strlen(buf));
	if (auth->net_access_key) {
		NdisCopyMemory(auth->net_access_key->buf, buftemp, os_strlen(buf));
		auth->net_access_key->used = os_strlen(buf);
	}

	NdisZeroMemory(ifparam, 50);
	strncat(ifparam, wdev->ifname, strlen(wdev->ifname));
	strncat(ifparam, "_cSignKey", strlen("_cSignKey"));
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s %d Buf is null from kvc_get\n", __func__, __LINE__);
		return;
	}
	NdisZeroMemory(buftemp, 512);
	hexstr2bin(buf, buftemp, os_strlen(buf));
	auth->c_sign_key = wpabuf_alloc(os_strlen(buf));
	if (auth->c_sign_key) {
		NdisCopyMemory(auth->c_sign_key->buf, buftemp, os_strlen(buf));
		auth->c_sign_key->used = os_strlen(buf);
	}

#if NL80211_SUPPORT
	wapp_set_DppEnable(wapp, (const char *)wdev->ifname,
			(char *)&Enable, 1);
#else
	os_memset(cmd, 0, 100);
	ret = snprintf(cmd, 100, "iwpriv %s set DppEnable=1;",
			wdev->ifname);
	if (os_snprintf_error(100, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
	ret = system(cmd);
	if (ret != 0)
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s, error status_code:%d\n", __func__, ret);

	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s\n", cmd);
#endif

}

static void dpp_fetch_psk_akm_param(struct wapp_dev *wdev, struct dpp_authentication *auth,
				    struct kvc_context *dat_ctx)
{
	char ifparam[50];
	const char *buf;

	auth->akm = DPP_AKM_PSK;
	NdisZeroMemory(ifparam, 50);
	strncat(ifparam, wdev->ifname, strlen(wdev->ifname));
	strncat(ifparam, "_passPhrase", strlen("_passPhrase"));
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s %d Buf is null from kvc_get\n", __func__, __LINE__);
		return;
	}
	os_strlcpy(auth->passphrase, buf, sizeof(auth->passphrase));
}

#endif /* DPP_R2_MUOBJ */
#endif

#ifdef DPP_R2_MUOBJ
void dpp_conf_init(struct wifi_app *wapp, wapp_dev_info *dev_info)
{
#ifdef OPENWRT_SUPPORT
	struct wapp_dev *wdev = NULL;
	struct kvc_context *dat_ctx = NULL;
	struct dpp_authentication *auth = NULL;
	char ifparam[50];
	const char *buf;
	int saved_akm;

	os_alloc_mem(NULL, (UCHAR**)&auth, sizeof(*auth));
	if (!auth)
		goto out;
	NdisZeroMemory(auth, sizeof(*auth));

	dat_ctx = dat_load(DPP_CFG_FILE);
	if (!dat_ctx) {
		DBGPRINT(RT_DEBUG_ERROR, "load file(%s) fail\n", DPP_CFG_FILE);
		goto out;
	}

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, dev_info->ifindex);
	if (!wdev)
		goto out;

	NdisZeroMemory(ifparam, 50);
	strcat(ifparam, wdev->ifname);
	strcat(ifparam, "_ssid");
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	/* if ssid for that wdev NULL assume not configured */
	if (buf == NULL) {
		DBGPRINT(RT_DEBUG_OFF, "saved config not found for %s\n", wdev->ifname);
		goto out;
	}
	DBGPRINT(RT_DEBUG_OFF, "saved config found for %s\n", wdev->ifname);
	os_strlcpy((char*)auth->conf_obj[0].ssid, buf, os_strlen(buf) + 1);
	auth->conf_obj[0].ssid_len = os_strlen(buf) + 1;
	NdisZeroMemory(ifparam, 50);
	strcat(ifparam, wdev->ifname);
	strcat(ifparam, "_akm");
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	saved_akm = buf[0] - '0';

	switch(saved_akm) {
		case DPP_AKM_DPP:
		case DPP_AKM_SAE_DPP:
			dpp_fetch_dpp_akm_param(wdev, auth, &(auth->conf_obj[0]), dat_ctx);
			break;
		case DPP_AKM_PSK:
		case DPP_AKM_PSK_SAE:
		case DPP_AKM_PSK_SAE_DPP:
			dpp_fetch_psk_akm_param(wdev, &(auth->conf_obj[0]), dat_ctx);
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, "%s, DPP AKM specified in saved config not supported %s dpp init fail\n",
				__func__, wdev->ifname);
			goto out;
	}
	auth->wdev = wdev;

#ifdef DPP_R2_STATUS
	auth->send_conn_status = 0;
	auth->waiting_conn_status_result = 0;
	auth->conn_status_requested = 0;
#endif /* DPP_R2_STATUS */
	wapp_dpp_process_config(wapp,auth, &(auth->conf_obj[0]));
out:
	if (auth)
		os_free_mem(NULL, auth);
	if (dat_ctx)
		kvc_unload(dat_ctx);
#endif /* OPENWRT_SUPPORT */
		return;
}
#else

void dpp_conf_init(struct wifi_app *wapp, wapp_dev_info *dev_info)
{
#ifdef OPENWRT_SUPPORT
	struct wapp_dev *wdev = NULL;
	struct kvc_context *dat_ctx = NULL;
	struct dpp_authentication *auth = NULL;
	char ifparam[50];
	const char *buf;
	int saved_akm;

	os_alloc_mem(NULL, (UCHAR**)&auth, sizeof(*auth));
	if (!auth)
		goto out;
	NdisZeroMemory(auth, sizeof(*auth));

	dat_ctx = dat_load(DPP_CFG_FILE);
	if (!dat_ctx) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"load file(%s) fail\n", DPP_CFG_FILE);
		goto out;
	}

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, dev_info->ifindex);
	if (!wdev)
		goto out;
	NdisZeroMemory(ifparam, 50);
	strncat(ifparam, wdev->ifname, strlen(wdev->ifname));
	strncat(ifparam, "_ssid", strlen("_ssid"));
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	/* if ssid for that wdev NULL assume not configured */
	if (buf == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"saved config not found for %s\n", wdev->ifname);
		goto out;
	}
	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"saved config found for %s\n", wdev->ifname);
	os_strlcpy((char*)auth->ssid, buf, os_strlen(buf) + 1);
	auth->ssid_len = os_strlen(buf) + 1;
	NdisZeroMemory(ifparam, 50);
	strncat(ifparam, wdev->ifname, strlen(wdev->ifname));
	strncat(ifparam, "_akm", strlen("_akm"));
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s %d Buf is null from kvc_get\n", __func__, __LINE__);
		goto out;
	}
	saved_akm = buf[0] - '0';

	switch(saved_akm) {
		case DPP_AKM_DPP:
		case DPP_AKM_SAE_DPP:
		case DPP_AKM_PSK_DPP:
			dpp_fetch_dpp_akm_param(wdev, auth, dat_ctx);
			break;
		case DPP_AKM_PSK:
		case DPP_AKM_PSK_SAE:
		case DPP_AKM_PSK_SAE_DPP:
			dpp_fetch_psk_akm_param(wdev, auth, dat_ctx);
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, "%s, DPP AKM specified in saved config not supported %s dpp init fail\n",
				__func__, wdev->ifname);
			goto out;
	}
	auth->wdev = wdev;

#ifdef CONFIG_DPP2
	auth->send_conn_status = 0;
	auth->waiting_conn_status_result = 0;
	auth->conn_status_requested = 0;
#endif /* CONFIG_DPP2 */
	wapp_dpp_process_config(wapp,auth);
out:
	if (auth)
		os_free_mem(NULL, auth);
	if (dat_ctx)
		kvc_unload(dat_ctx);
#endif /* OPENWRT_SUPPORT */
		return;
}

#endif /* DPP_R2_MUOBJ */

#ifdef DPP_R2_SUPPORT
int wapp_dpp_set_ie(struct wifi_app *wapp)
{
	int ret = 0;
	struct dpp_cce_info_element *ie;
	char *buf;
	int ie_len = 0;
#ifdef MAP_R3
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list;

#else
	struct wapp_conf *conf;
#endif /* MAP_R3 */
	
	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"%s\n", __FUNCTION__);

	ie_len = sizeof(struct dpp_cce_info_element);
	buf = os_zalloc(ie_len);

	if(!buf) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX "memory is not avaiable\n");
		return -1;
	}

	os_memset(buf, 0, ie_len);
	ie = (struct dpp_cce_info_element *)buf;

	ie->eid = VENDOR_SPECIFIC;
	ie->length = 4;

	ie->oui[0] = 0x50;
	ie->oui[1] = 0x6F;
	ie->oui[2] = 0x9A;

	//ie->oui[0] = 0x00;
	//ie->oui[1] = 0x0C;
	//ie->oui[2] = 0xE7;

	ie->value = 0x1E;

#ifdef MAP_R3
	/* For MAP R3 need to enable CCE IE on Fronthaul BSS as per spec */
	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if ((wdev->dev_type == WAPP_DEV_TYPE_AP) && wdev->i_am_fh_bss) {
			ret = wapp_set_ie(wapp, wdev->ifname, buf, ie_len);
			if (ret == -1) {
				os_free(buf);
				return ret;
			}
		}
	}
#else
	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		ret = wapp_set_ie(wapp, conf->iface, buf, ie_len);
		if (ret == -1) {
			os_free(buf);
			return ret;
		}
	}
#endif /* MAP_R3 */
        if(ie)
         os_free(ie);
       	 
	return ret;
}
#endif
#if ((defined DPP_R2_SUPPORT) || (defined DPP_R2_RECONFIG))
struct wapp_dev* wapp_dpp_get_wdev_by_channel(struct dpp_global *dpp, const u8 chan)
{
	struct wapp_dev *target_wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"%s, input channel :%d\n", __func__, chan);
	if (!dpp) {
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"failed to find dpp, not initialized\n");
		return NULL;
	}

	if (IS_MAP_CH_5GH(chan)) {
		target_wdev = dpp->default_5gh_iface;
	} else if (IS_MAP_CH_5GL(chan)) {
		target_wdev = dpp->default_5gl_iface;
	} else if (IS_MAP_CH_24G(chan)) {
		target_wdev = dpp->default_2g_iface;
	}

	return target_wdev;
}

unsigned char wapp_dpp_get_opclass_from_channel(unsigned char channel)
{
	if (channel <= 14)
	{
		return 81;
	} else if (channel <= 48) {
		return 115;
	} else if (channel <= 64) {
		return 118;
	} else if (channel <= 144) {
		return 121;
	} else if (channel <= 177) {
		return 125;
	}
	return 0;
}

char * wapp_dpp_get_scan_channel_list(struct wifi_app *wapp)
{
	char *str = NULL, *end = NULL, *pos = NULL;
	size_t len;
	unsigned int i = 0;
	u8 last_op_class = 0;
	int res = 0;
	struct dpp_global *dpp = NULL;
	

	if (!wapp || !wapp->dpp)
		return NULL;

	dpp = wapp->dpp;

	len = dpp->scan_ch.ch_num * 8;
	str = os_zalloc(len);
	if (!str)
		return NULL;
	end = str + len;
	pos = str;

	for (i = 0; i < dpp->scan_ch.ch_num; i++) {
		u8 op_class = 0, channel = 0;

		channel = dpp->scan_ch.chan[i];
		op_class = wapp_dpp_get_opclass_from_channel(channel);

		if (op_class == last_op_class)
			res = os_snprintf(pos, end - pos, ",%d", channel);
		else
			res = os_snprintf(pos, end - pos, "%s%d/%d",
					  pos == str ? "" : ",",
					  op_class, channel);
		if (os_snprintf_error(end - pos, res)) {
			*pos = '\0';
			break;
		}
		pos += res;
		last_op_class = op_class;
	}

	if (pos == str) {
		os_free(str);
		str = NULL;
	}
	return str;
}

void wapp_dpp_scan_channel(struct wifi_app *wapp, struct wapp_dev *wdev)
{
	struct bss_info_scan_result *scan_result = NULL, *scan_result_tmp = NULL;
	struct dpp_config *wdev_config = NULL;
	struct dpp_global *dpp = NULL;

	if(!wapp || !wdev)
		return;

	wdev_config = wdev->config;
	dpp = wapp->dpp;

	if(!wdev_config || !dpp)
		return;

	os_memset(&(dpp->scan_ch), 0, sizeof(struct dpp_scan_channel));
	dl_list_for_each_safe(scan_result, scan_result_tmp, &(wapp->scan_results_list),
								struct bss_info_scan_result, list) {
		if (os_memcmp(scan_result->bss.Ssid, wdev_config->ssid, scan_result->bss.SsidLen) == 0) {
			
			if(scan_result->bss.SsidLen == 0)
				continue;
			if(dpp->scan_ch.ch_num > DPP_BOOTSTRAP_MAX_FREQ)
				break;
			dpp->scan_ch.chan[dpp->scan_ch.ch_num] = scan_result->bss.Channel;
			dpp->scan_ch.ch_num ++;
		}
	}
	if(dpp->scan_ch.ch_num)
		dpp->scan_ch.ch_num = dpp_handle_presence_channel_dup((int* )dpp->scan_ch.chan,dpp->scan_ch.ch_num);
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: scan ch num:%d \n", dpp->scan_ch.ch_num);
}
#endif /* (defined DPP_R2_SUPPORT) || (defined DPP_R2_RECONFIG) */
#ifdef MAP_6E_SUPPORT
u8 wapp_6e_ch_lookup(u8 ch, u8 ch_arr[], u8 ch_num)
{
	u8 i = 0;

	for (i = 0; i < ch_num; i++) {
		if (ch == ch_arr[i])
			return 1;
	}
	return 0;
}
int wapp_dpp_6e_prep_ch_list(struct wifi_app *wapp)
{
	struct bss_info_scan_result *scan_result = NULL, *scan_result_tmp = NULL;
	u8 ch_list_6g[MAX_CH_NUM] = {0}, i = 0;
	u8 check_ch = 0;

	dl_list_for_each_safe(scan_result, scan_result_tmp, &(wapp->scan_results_list),
								struct bss_info_scan_result, list) {
		if (scan_result->bss.rnr_6e.channel && IS_OP_CLASS_6G(scan_result->bss.rnr_6e.op)
			&& scan_result->bss.rnr_6e.cce_ind) { /*it will add all 6g channels, need to check cce*/
			check_ch = wapp_6e_ch_lookup(scan_result->bss.rnr_6e.channel, ch_list_6g, i);
			if (check_ch == 0)
				ch_list_6g[i++] = scan_result->bss.rnr_6e.channel;
		}
	}
	if (i == 0)
		return -1;
	wapp_dpp_query_cce_channel(wapp);
	wapp->dpp->cce_ch.cce_6g.ch_num = i;
	for (i = 0; i < wapp->dpp->cce_ch.cce_6g.ch_num && i < DPP_CCE_CH_MAX; i++)
		wapp->dpp->cce_ch.cce_6g.chan[i] = ch_list_6g[i];

	return 0;
}
#endif
#ifdef MAP_R3
void wapp_dpp_presence_ch_scan(struct wifi_app *wapp)
{
	struct dl_list *dev_list;
	struct wapp_dev *chirp_wdev = NULL;
#ifdef MAP_R3_RECONFIG
	struct scan_BH_ssids *scan_ssids = NULL;
#endif
#ifdef MAP_6E_SUPPORT
	static u8 short_6g_scan = 1;
	int ret = 0;
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list2 = NULL;
#endif
	if(!wapp || !wapp->dpp)
		return;


	if(!wapp->dpp->cce_driver_scan_ongoing) {
		DBGPRINT(RT_DEBUG_ERROR,"CCE scan not ongoing, return from here.\n");
		return;
	}

	dev_list = &wapp->dev_list;
	dl_list_for_each(chirp_wdev, dev_list, struct wapp_dev, list) {
		if (chirp_wdev->dev_type == WAPP_DEV_TYPE_STA) {
			if(chirp_wdev->scan_done_ind == TRUE)
				continue;
#ifdef MAP_6E_SUPPORT
			struct ap_dev *ap = (struct ap_dev *)chirp_wdev->p_dev;

			if (short_6g_scan && IS_OP_CLASS_6G(ap->ch_info.op_class)) {
				dev_list2 = &wapp->dev_list;
				dl_list_for_each(wdev, dev_list2, struct wapp_dev, list) {
					/*checking other wdev*/
					if (chirp_wdev != wdev && wdev->scan_done_ind == FALSE)
						ret = -1;
				}
				if (ret == -1)
					continue;
				ret = wapp_dpp_6e_prep_ch_list(wapp);
				if (ret == 0) {
					short_6g_scan = 0;
					return;
				}
			}
			short_6g_scan = 1;
#endif
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: Issue scan request for %s\n", chirp_wdev->ifname);
#ifdef MAP_R3_RECONFIG
			scan_ssids = os_zalloc(sizeof(struct scan_BH_ssids));//max Profile count 4 possible
			if (!scan_ssids) {
				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"%s alloc scan_BH_ssids fail\n", __func__);
				return;
			}
			wapp_set_scan_BH_ssids(wapp, chirp_wdev, scan_ssids);
#endif
			wapp_issue_scan_request(wapp, chirp_wdev);
			time_t sec = time(0);

			if (sec != (time_t)(-1))
				srandom(sec);
			else
				srandom(5381);
			chirp_wdev->scan_cookie = random();
			chirp_wdev->waiting_cce_scan_res = TRUE;
			wapp->scan_wdev = chirp_wdev;
			eloop_cancel_timeout(map_get_scan_result, wapp, chirp_wdev);
			eloop_register_timeout(10, 0, map_get_scan_result, wapp, chirp_wdev);
			break;
		}
	}

	if(wapp->cce_scan_count == wapp->radio_count) {
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: Max radio count reached reset wdev states\n");
		wapp->cce_scan_count = 0;
		dev_list = &wapp->dev_list;
		dl_list_for_each(chirp_wdev, dev_list, struct wapp_dev, list) {
			if (chirp_wdev->dev_type == WAPP_DEV_TYPE_STA) {
				chirp_wdev->scan_done_ind = FALSE;
				chirp_wdev->waiting_cce_scan_res = FALSE;
			}
		}
		wapp->dpp->cce_driver_scan_ongoing = 0;
		wapp->dpp->cce_driver_scan_done = 1;
		/* Query for CCE ch to driver and start chirping */
		wapp_dpp_query_cce_channel(wapp);
#ifdef MAP_R3_RECONFIG
		if (!wapp->wdev_backup)
#endif
			wapp_dpp_presence_annouce(wapp, NULL);
#ifdef MAP_R3_RECONFIG
		else
			wapp_dpp_send_reconfig_annouce (wapp, wapp->wdev_backup);
#endif
	}
#ifdef MAP_R3_RECONFIG
	free(scan_ssids);
#endif
	return;
}

void wapp_dpp_cce_res_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_bootstrap_info *bi = timeout_ctx;

	if(wapp && bi) {
		if(wapp->dpp->cce_scan_ongoing) {
			DBGPRINT(RT_DEBUG_OFF,
					DPP_MAP_PREX"DPP: Timeout while waiting for CCE scan result\n");
		}
		wapp_dpp_presence_annouce(wapp, bi);
	}
}

#ifdef MAP_R3_RECONFIG
void wapp_dpp_cce_res_wait_timeout_recon(void *eloop_ctx,
						   void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct wapp_dev *wdev = timeout_ctx;

	if (wapp) {
		if(wapp->dpp->cce_scan_ongoing) {
			DBGPRINT(RT_DEBUG_OFF,
					DPP_MAP_PREX"DPP: Reconfig Timeout while waiting for CCE scan result\n");
		}
		wapp_dpp_send_reconfig_annouce(wapp, wdev);
	}
}
#endif
#endif /* MAP_R3 */

#ifdef DPP_R2_SUPPORT
void wapp_dpp_presence_auth_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_bootstrap_info *bi = timeout_ctx;

	if(wapp && bi) {
		DBGPRINT(RT_DEBUG_OFF, 
			   DPP_MAP_PREX "DPP: Timeout while presence wait for Auth req (pre_status:%d)",
				wapp->dpp->annouce_enrolle.pre_status);
		if(wapp->dpp->annouce_enrolle.pre_status !=  DPP_PRE_STATUS_SUCCESS 
			&& wapp->dpp->annouce_enrolle.is_enable == 1) {
			wapp->dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_INIT;
			wapp_dpp_presence_annouce(wapp, bi);
		}
	}
}

void wapp_dpp_presence_annouce(struct wifi_app *wapp, struct dpp_bootstrap_info *dpp_bi)
{
	struct wpabuf *msg =  NULL;
	struct wapp_dev * wdev=  NULL;
	struct dpp_bootstrap_info *bi =NULL;
	int wait_time = DPP_PRESENCE_MIN_TIMEOUT;
	struct dpp_global *dpp = NULL;
	unsigned int chan = 0;
	unsigned int action_wait_time = 0;
	int ch_num = 0;
#ifdef MAP_R3
	int target_band;
#if 0
	struct wapp_dev * agnt_wdev=  NULL;
	struct dl_list *dev_list;
	char cmd[100];
#endif
#endif /* MAP_R3 */

	if (!wapp || !wapp->dpp) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed to find wapp, auth not initialized\n");
		return;
	}
#ifdef MAP_R3
	/* When version check will be present use this*/
	if (wapp->map && (wapp->map->map_version != DEV_TYPE_R3 || wapp->dpp->onboarding_type != 0)) {
		wapp_version_mismatch(wapp);
		return;
	}
#endif /* MAP_R3 */

	dpp = wapp->dpp;
	if (!dpp->annouce_enrolle.is_enable) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"presnece feature is disabled \n");
		return;
	}

#ifdef MAP_R3
	if(dpp->dpp_eth_conn_ind == TRUE) {
		dpp_auth_fail_wrapper(wapp, "ETH link is connected, ignore wifi onboarding CMD");
		return;
	}
	if(dpp->chirp_stop_done == TRUE) {
		dpp_auth_fail_wrapper(wapp, "chirp stop received, Trigger using CMD");
		return;
	}
#endif /* MAP_R3 */

	if (dpp->annouce_enrolle.pre_status != DPP_PRE_STATUS_INIT
#ifdef MAP_R3
		&& (dpp->dpp_onboard_ongoing == 1)
#endif /* MAP_R3 */
	) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" presence annouce frame already start (%d)\n",
				dpp->annouce_enrolle.pre_status);
		dpp_auth_fail_wrapper(wapp, "DPP onboarding is already ongoing\n");
		return;
	}
	
	if(!dpp_bi) {
		bi = dpp_get_own_bi(dpp);
	} else
		bi = dpp_bi;

	if (!bi) {
		DBGPRINT(RT_DEBUG_OFF, 
			   DPP_MAP_PREX"DPP: Could not find local bootstrapping info ");
		return;
	}

#ifdef MAP_R3
	dpp->chirp_ongoing = 1;
	/* Tigger scan to driver for updating CCE channel list */
	if(((dpp->annouce_enrolle.presence_retry == 0 && dpp->annouce_enrolle.cur_presence_chan_id == 0)
			|| (dpp->annouce_enrolle.presence_retry == DPP_PRESENCE_MAX_RETRY)) && !wapp->dpp->cce_driver_scan_done
			&& (wapp->map &&
				wapp->map->off_ch_scan_state.ch_scan_state
				== CH_SCAN_IDLE)
			&& !wapp->dpp->wsc_onboard_done) {
		wapp->dpp->cce_driver_scan_ongoing = 1;
		wapp_dpp_presence_ch_scan(wapp);
		return;
	}

	if(wapp->dpp->cce_driver_scan_ongoing) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" CCE driver scan ongoing..\n");
		return;
	}

	/* Select default interface and query for cce scan result for first time here*/
	if((wapp->radio_count > 0) && 
		(!wapp->dpp->default_2g_iface && !wapp->dpp->default_5gl_iface
		&& !wapp->dpp->default_5gh_iface)) {
		wapp_dpp_default_iface(wapp);
		wapp_dpp_query_cce_channel(wapp);
	}

	if(!dpp->cce_scan_ongoing) {
		eloop_cancel_timeout(wapp_dpp_cce_res_wait_timeout, wapp, bi);
	}
#endif /* MAP_R3 */

	msg = dpp_build_annouce_frame(dpp, bi);
	if(!msg) {
		DBGPRINT(RT_DEBUG_OFF,
			   DPP_MAP_PREX"DPP: build annouce frame fail ");
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"presence_retry:%d,  cur_presence_chan_id:%d\n",
				dpp->annouce_enrolle.presence_retry, dpp->annouce_enrolle.cur_presence_chan_id);
	if((dpp->annouce_enrolle.presence_retry == 0 && dpp->annouce_enrolle.cur_presence_chan_id == 0)
						|| dpp->annouce_enrolle.presence_retry == DPP_PRESENCE_MAX_RETRY) {
		DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"gen annouce channel list \n");
#ifdef MAP_R3
		if(dpp->annouce_enrolle.presence_retry == DPP_PRESENCE_MAX_RETRY) {
			/* If max count reached for one cycle, give indication to user*/
			dpp_auth_fail_wrapper(wapp,"Chirp max count reached for one cycle, re-create ch list\n");
		}

		/* Setting the chirp and dpp onboarding ongoing flag */
		dpp->dpp_onboard_ongoing = 1;
		if(dpp->cce_scan_ongoing) {
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"wait for CCE query result\n");
			/* set register to wait the CCE result frm driver, if timeout, resend again */
			eloop_cancel_timeout(wapp_dpp_cce_res_wait_timeout, wapp, bi);
			eloop_register_timeout(5, 0, wapp_dpp_cce_res_wait_timeout, wapp, bi);
			os_free(msg);
			return;
		}

		/* If R1/R2 onboarding is done and R3 onboarding to trigger,
		 * need to genarate list only for working channels */
		if(dpp->wsc_onboard_done)
			ch_num = dpp_map_wsc_done_chirp_channel_list(wapp, &(dpp->annouce_enrolle));
		else
			ch_num = dpp_gen_presence_annouce_channel_list(dpp, &(dpp->annouce_enrolle));
#else
		ch_num = dpp_gen_presence_annouce_channel_list(dpp, &(dpp->annouce_enrolle));
#endif /* MAP_R3 */
		if(ch_num == 0){
#ifdef MAP_R3
			if((wapp->map->TurnKeyEnable == 1) && !dpp->wsc_onboard_done) {
				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"No channel found in list, scan again\n");
				wapp->dpp->cce_driver_scan_ongoing = 1;
				wapp_dpp_presence_ch_scan(wapp);
			}
			else {
				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"gen annouce channel list fail\n");
			}
#else
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"gen annouce channel list fail\n");
#endif /* MAP_R3 */
			if(msg)
				os_free(msg);
			
			return;
		}

		dpp->annouce_enrolle.presence_retry = 0;
	}
	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"ch_num:%d,  cur_presence_chan_id:%d\n",
				dpp->annouce_enrolle.ch_num, dpp->annouce_enrolle.cur_presence_chan_id);
#ifdef MAP_6E_SUPPORT
	if ((os_strcasecmp(wapp->dpp->band_priority, "6g") == 0)) {
		if (dpp->annouce_enrolle.cur_presence_chan_id_6g == dpp->annouce_enrolle.ch_num_6g) {
			if (dpp->annouce_enrolle.cur_presence_chan_id == dpp->annouce_enrolle.ch_num) {
				dpp->annouce_enrolle.cur_presence_chan_id++;
				wait_time = DPP_PRESENCE_MAX_TIMEOUT;
				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"pause %d s before repeating the presence annouce\n", wait_time);
				/* set register to wait the auth req, if timeout, resend again */
				eloop_cancel_timeout(wapp_dpp_presence_auth_wait_timeout, wapp, bi);
				eloop_register_timeout(wait_time, 0, wapp_dpp_presence_auth_wait_timeout, wapp, bi);
				os_free(msg);
				return;
			} else if (dpp->annouce_enrolle.cur_presence_chan_id == dpp->annouce_enrolle.ch_num+1) {
				dpp->annouce_enrolle.presence_retry++;
				dpp->annouce_enrolle.cur_presence_chan_id = 0;
				dpp->annouce_enrolle.cur_presence_chan_id_6g = 0;
			}
		}
	} else
#endif
	if(dpp->annouce_enrolle.cur_presence_chan_id == dpp->annouce_enrolle.ch_num) {
#ifdef MAP_6E_SUPPORT
		if (dpp->annouce_enrolle.cur_presence_chan_id_6g != dpp->annouce_enrolle.ch_num_6g) {
#endif
		dpp->annouce_enrolle.cur_presence_chan_id++;
		wait_time = DPP_PRESENCE_MAX_TIMEOUT;
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"pause %d s before repeating the presence annouce \n", wait_time);
		/* set register to wait the auth req, if timeout, resend again */
		eloop_cancel_timeout(wapp_dpp_presence_auth_wait_timeout, wapp, bi);
		eloop_register_timeout(wait_time, 0, wapp_dpp_presence_auth_wait_timeout, wapp, bi);
		os_free(msg);
		return;
#ifdef MAP_6E_SUPPORT
		}
#endif
	} else if(dpp->annouce_enrolle.cur_presence_chan_id == dpp->annouce_enrolle.ch_num+1) {
		dpp->annouce_enrolle.presence_retry++;
		dpp->annouce_enrolle.cur_presence_chan_id = 0;
	}
#ifdef MAP_6E_SUPPORT
	int chk_6g = 0;

	if ((os_strcasecmp(wapp->dpp->band_priority, "6g") == 0) &&
		dpp->annouce_enrolle.cur_presence_chan_id_6g != dpp->annouce_enrolle.ch_num_6g) {
		chan = dpp->annouce_enrolle.chan_6g[dpp->annouce_enrolle.cur_presence_chan_id_6g];
		chk_6g = 1;
	} else if (dpp->annouce_enrolle.cur_presence_chan_id == dpp->annouce_enrolle.ch_num) {
		chan = dpp->annouce_enrolle.chan_6g[dpp->annouce_enrolle.cur_presence_chan_id_6g];
		chk_6g = 1;
	} else
#endif
	chan = dpp->annouce_enrolle.chan[dpp->annouce_enrolle.cur_presence_chan_id];
	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"chan:%d, cur chan_id:%d", chan, dpp->annouce_enrolle.cur_presence_chan_id);
#ifdef MAP_R3
#ifdef MAP_6E_SUPPORT
	if (chk_6g) {
		target_band = RADIO_6G;
		wdev = wapp_dev_list_lookup_by_band_and_type(wapp, target_band, WAPP_DEV_TYPE_STA);
	} else {
#endif
	target_band = (int)wapp_op_band_frm_ch(wapp, chan);
        wdev = wapp_dev_list_lookup_by_band_and_type(wapp, target_band, WAPP_DEV_TYPE_STA);
#ifdef MAP_6E_SUPPORT
	}
#endif
#else
	wdev = wapp_dpp_get_wdev_by_channel(dpp, chan);
#endif /* MAP_R3 */

#if 0 /* Commented Proxt Agent handling */
	if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_PROXYAGENT
			&& !wapp->dpp->wsc_onboard_done){
		/* If device is proxy agent and DPP onboarding is triggered
		 * by user change role back to enrollee and disconnect any existing connection*/
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"Device is proxy agnt, convert to enrollee\n");

		dev_list = &wapp->dev_list;
		dl_list_for_each(agnt_wdev, dev_list, struct wapp_dev, list) {
			if ((agnt_wdev) && (agnt_wdev->dev_type == WAPP_DEV_TYPE_STA)) {

#if NL80211_SUPPORT
				u8 Enable = 0;
				wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
						(char *)&Enable, 1);
#else
				DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"ifname %s\n", agnt_wdev->ifname);
				os_memset(cmd, 0, 100);
				snprintf(cmd,100, "iwpriv %s set ApCliEnable=0;", agnt_wdev->ifname);
				system(cmd);
#endif
			}
		}
		wapp->dpp->dpp_allowed_roles = DPP_CAPAB_ENROLLEE;
	}
#endif
#ifdef MAP_6E_SUPPORT
	if (chk_6g)
		dpp->annouce_enrolle.cur_presence_chan_id_6g++;
	else
#endif
	dpp->annouce_enrolle.cur_presence_chan_id++;
#ifdef MAP_R3
	wapp->dpp->dpp_wifi_onboard_ongoing = TRUE;
#endif
	if(!wdev) {
		os_free(msg);
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"send presence frame fail, invalid wdev \n");
		return;
	}

	if(wdev->radio) {
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"send presence frame success, channel:%d \n", chan);
		/* send frame */
#ifdef MAP_R3
		//Increasing ROC time for MAP R3 becuase remote peer may take time in changing channel
		action_wait_time = 1500;
		dpp->annouce_enrolle.curr_presence_chan = chan;
#else
		action_wait_time = 0;
#endif
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX" chan=%u type=%d",
				chan, DPP_PA_PRESENCE_ANNOUNCEMENT);
		wapp_drv_send_action(wapp, wdev, chan, action_wait_time,
					broadcast, wpabuf_head(msg),
					wpabuf_len(msg));
		dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_WAIT_AUTHREQ;
	}else {
		wait_time = 2;
		dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_INIT;
	}

#ifdef MAP_R3
	if(dpp->annouce_enrolle.presence_retry == DPP_PRESENCE_MAX_RETRY) {
		DBGPRINT(RT_DEBUG_ERROR, "One cycle for ch list eneded reset the flag here\n");
		wapp->dpp->cce_driver_scan_done = 0;
	}
#endif /* MAP_R3 */
	/* set register to wait the auth req, if timeout, resend again */
	eloop_cancel_timeout(wapp_dpp_presence_auth_wait_timeout, wapp, bi);
	eloop_register_timeout(wait_time, 0, wapp_dpp_presence_auth_wait_timeout, wapp, bi);
	os_free(msg);
}

void wapp_dpp_trigger_auth_req(struct wifi_app *wapp, struct wapp_dev * wdev, struct dpp_bootstrap_info *own_bi, 
	struct dpp_bootstrap_info *peer_bi, struct dpp_authentication *auth_info)
{
	u8 allowed_roles = 0;
	unsigned int neg_chan = 0;
	struct dpp_authentication *auth = NULL;
#ifdef MAP_R3
	int res = 0;
#endif /* MAP_R3 */

	if (!wapp ||!wdev ||!peer_bi || !own_bi) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: input parameter null \n");
		return;
	}

	allowed_roles = wapp->dpp->dpp_allowed_roles;
	if(!auth_info)
	{	auth = dpp_auth_init(wapp, wdev, peer_bi, own_bi,
				       allowed_roles, neg_chan);}
		if (!auth)
			return;
		auth->wdev = wdev;

		if(wapp_dpp_auth_list_insert(wapp, auth) < 0) {
			dpp_auth_deinit(auth);
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: auth insert fail deinit \n");
			return;
		}

		if (dpp_set_configurator(wapp->dpp, wapp,
				 auth, NULL) < 0) {
			dpp_auth_deinit(auth);
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: auth set configuratior info fail \n");
			return;
		}

		auth->neg_chan = neg_chan;
		auth->chan[0] = wdev->radio->op_ch;
		auth->num_chan = 1;
#ifdef MAP_R3
		/* Resetting wired flag for sending all DPP frames on air*/
		auth->is_wired = 0;

		/* Adding chirp value in created auth for multiple connection use */
		res = wapp_dpp_add_chirp_tlv(auth);
		if(res < 0) {
			dpp_auth_deinit(auth);
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: Peer chirp value not added in Auth\n");
			return;
		}
#endif /* MAP_R3 */
		if (!is_zero_ether_addr(peer_bi->mac_addr))
			os_memcpy(auth->peer_mac_addr, peer_bi->mac_addr,
				  MAC_ADDR_LEN);
	else {
		auth = auth_info;
	}

	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"DPP: enter wapp_dpp_auth_init_next \n");
	wapp_dpp_auth_init_next(wapp, auth);
}

void wapp_dpp_check_presence_auth_req(struct wifi_app* wapp, const u8 *r_bootstrap,
		struct dpp_bootstrap_info *own_bi, struct dpp_authentication *auth)
{
	if(! wapp ||!auth || !own_bi || !r_bootstrap) 
		return; 
	DBGPRINT(RT_DEBUG_TRACE,  DPP_MAP_PREX"DPP: pre_status :%d \n", wapp->dpp->annouce_enrolle.pre_status);
	if (wapp->dpp->annouce_enrolle.pre_status == DPP_PRE_STATUS_WAIT_AUTHREQ) {
		if (os_memcmp(r_bootstrap, own_bi->pubkey_hash, SHA256_MAC_LEN) == 0) {
			eloop_cancel_timeout(wapp_dpp_presence_auth_wait_timeout, wapp, own_bi);
			wapp->dpp->annouce_enrolle.presence_retry = 0;
			wapp->dpp->annouce_enrolle.cur_presence_chan_id = 0;
			wapp->dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_SUCCESS;
#ifdef MAP_6E_SUPPORT
			wapp->dpp->annouce_enrolle.cur_presence_chan_id_6g = 0;
#endif
#ifdef MAP_R3
			/* Resetting the chirp ongoing and cce scan done flag */
			wapp->dpp->chirp_ongoing = 0;
			wapp->dpp->cce_driver_scan_done = 0;
			wapp->dpp->annouce_enrolle.curr_presence_chan = 0;
#endif /* MAP_R3 */
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: presence annouce frame receive the auth req \n");
		}
	}
}
void wapp_dpp_rx_presence_annouce(struct wifi_app *wapp, struct wapp_dev *wdev,
				const u8 *src, const u8 *buf, size_t len, unsigned int chan)
{
	const u8 *r_bootstrap  = NULL;
	u16 r_bootstrap_len = 0;
	struct dpp_bootstrap_info *peer_bi = NULL, *own_bi = NULL;
	struct dpp_authentication *auth = NULL;

	if (!wapp ||!wapp->dpp ||!wdev || !src || !buf )
		return;

	DBGPRINT(RT_DEBUG_OFF,  DPP_MAP_PREX"DPP: presence annouce frame from " MACSTR " config:%d\n",
		   MAC2STR(src), wapp->dpp->dpp_configurator_supported);
#ifdef MAP_R3
	struct dpp_agent_info *agnt_info = NULL;
	unsigned int wait_time = 0;
	if(wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: Device is still enrollee no need to handle chirp\n");
		return;
	}
#else
	if ( wdev  && !wapp->dpp->dpp_configurator_supported)
		return;
#endif /* MAP_R3*/

#ifndef MAP_R3
	/* get auth wapp_dpp_get_auth_from_peer_mac() auth->current_state */
	auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);
	if(auth&& auth->current_state != DPP_STATE_DEINIT) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: already auth exchange \n");
		return;
	}
#endif /* MAP_R3*/

	/* get bootstrap key hash */ 
	r_bootstrap = dpp_get_attr(buf, len, DPP_ATTR_R_BOOTSTRAP_KEY_HASH,
				   &r_bootstrap_len);
	if (!r_bootstrap || r_bootstrap_len != SHA256_MAC_LEN) {
		DBGPRINT(RT_DEBUG_ERROR, 
			DPP_MAP_PREX"Missing or invalid required Responder Bootstrapping Key Hash attribute");
		return;
	}

#ifdef MAP_R3
	wpa_hexdump(MSG_MSGDUMP, DPP_MAP_PREX "DPP: Responder Bootstrapping Key Hash",
		    r_bootstrap, r_bootstrap_len);

	/* For MAP R3 find the auth instance from chirp
	 * hash value instead of MAC address */
	auth = wapp_dpp_get_auth_from_hash_val(wapp, (u8 *) r_bootstrap);
	if(!auth && (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_PROXYAGENT)) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Auth instance not found providing to controller\n");
		ChirpMsg_1905_send(wapp, (u8 *)r_bootstrap, (u8 *)src
#ifdef MAP_R3_RECONFIG
		, 0
#endif /* MAP_R3_RECONFIG */
		, 1, NULL);
		return;
	}

	if(auth && auth->current_state != DPP_STATE_DEINIT
		&& wapp->dpp->dpp_configurator_supported) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: already auth exchange \n");
		return;
	}
#endif /* MAP_R3 */

#ifdef MAP_R3
	if(wapp->dpp->dpp_configurator_supported) {
#endif /* MAP_R3 */
		/* Try to find peer chirp key matches based on the
		 * received hash values */
		peer_bi= dpp_chirp_find_pair(wapp->dpp, r_bootstrap);

		if (!peer_bi) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: can not get peer bi \n");
			return;
		}
		if (is_zero_ether_addr(peer_bi->mac_addr))
			os_memcpy(peer_bi->mac_addr, src,  MAC_ADDR_LEN);

#ifdef MAP_R3
		os_memcpy(peer_bi->mac_addr, src,  MAC_ADDR_LEN);
		ChirpTLV_1905_send(wapp, peer_bi);
#endif /* MAP_R3 */
		own_bi = dpp_get_own_bi(wapp->dpp);
		if(!own_bi)
		{
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: can not get own bi \n");
			return;
		}

#ifdef MAP_R3
		agnt_info = wapp_dpp_get_agent_list_from_chirp_hash_val(wapp, (u8 *)r_bootstrap);
		if (agnt_info) {
			agnt_info->bh_type = MAP_BH_WIFI;
			DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"DPP: Setting the agent info  to bh %u\n", agnt_info->bh_type);
		}
#endif /* MAP_R3 */

		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: find the presence hash, then trigger to send auth req\n");
		/* trigger the sending auth request flow */
		wapp_dpp_trigger_auth_req(wapp, wdev, own_bi, peer_bi, auth);
#ifdef MAP_R3
	} else if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_PROXYAGENT) {

		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Matching Key hash value found");

		//updating the current channel,wdev and peer mac as per presence announcement frame
		auth->wdev = wdev;
		auth->curr_chan = chan;
		os_memcpy(auth->peer_mac_addr, src, MAC_ADDR_LEN);

		if (wapp->dpp->dpp_init_retry_time)
			wait_time = wapp->dpp->dpp_init_retry_time;
		else
			wait_time = 10000;
		eloop_register_timeout(wait_time / 1000, (wait_time % 1000) * 1000,
				wapp_dpp_reply_wait_timeout, wapp, auth);
		auth->current_state = DPP_STATE_AUTH_RESP_WAITING;
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR
				" chan=%u type=%d", MAC2STR(src), auth->curr_chan,
				DPP_PA_AUTHENTICATION_REQ);
		wapp_drv_send_action(wapp, auth->wdev, chan, wait_time,
				src,
				wpabuf_head(auth->msg_out),
				wpabuf_len(auth->msg_out));

	}
#endif /* MAP_R3 */
}

void wapp_dpp_query_cce_channel(struct wifi_app* wapp)
{
	if (!wapp || !wapp->dpp 
#ifndef MAP_R3_RECONFIG
		|| wapp->dpp->annouce_enrolle.pre_status == DPP_PRE_STATUS_SUCCESS
#endif
	)
		return;

	if (wapp->dpp->default_2g_iface)
		wapp_get_cce_result(wapp, wapp->dpp->default_2g_iface);

	if (wapp->dpp->default_5gl_iface)
		wapp_get_cce_result(wapp, wapp->dpp->default_5gl_iface);

	if (wapp->dpp->default_5gh_iface
#ifdef MAP_R3
		&& (wapp->radio_count == MAP_MAX_RADIO)
#endif /* MAP_R3 */
	)
		wapp_get_cce_result(wapp, wapp->dpp->default_5gh_iface);

	wapp->dpp->cce_scan_ongoing = 1;
}

void wapp_dpp_handle_cce_channel(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	unsigned int cce_ch_num = 0, i = 0;

	if (!wapp || !event_data)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev  || !wdev->radio)
		return;
	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s cce channel num:%d (cur ch :%d) \n",
				__func__, event_data->cce_ie_result.num, wdev->radio->op_ch);

#ifdef MAP_R3
	wapp->dpp->cce_rsp_rcvd_cnt++;
	if ((wapp->dpp->cce_rsp_rcvd_cnt == wapp->radio_count)
#ifdef MAP_R3_RECONFIG
		|| (wapp->dpp->radar_detect_ind && (wapp->dpp->cce_rsp_rcvd_cnt == (wapp->radio_count - 1)))
#endif /* MAP_R3_RECONFIG */
	) {
		/* resetting the count for cce scan done purpose */
		wapp->dpp->cce_scan_ongoing = 0;
		wapp->dpp->cce_rsp_rcvd_cnt = 0;
	}
#endif /* MAP_R3 */
	if(!event_data->cce_ie_result.num)
		return;

	if(event_data->cce_ie_result.num > DPP_CCE_CH_MAX)
		cce_ch_num = DPP_CCE_CH_MAX;
	else
		cce_ch_num = event_data->cce_ie_result.num;
	
	if ( IS_MAP_CH_24G(wdev->radio->op_ch) && wapp->dpp->default_2g_iface) {
		for(i = 0; i < cce_ch_num; i++) {
			wapp->dpp->cce_ch.cce_2g.chan[i] = event_data->cce_ie_result.cce_ch[i];
			DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"%s CCE AP channel list num:%d\n",
					__func__, (int)wapp->dpp->cce_ch.cce_2g.chan[i]);
		}
		wapp->dpp->cce_ch.cce_2g.ch_num = cce_ch_num;
	}

#ifdef MAP_R3
	if(wapp->radio_count == MAX_RADIO_DBDC) {
		if (IS_MAP_CH_5G(wdev->radio->op_ch) && wapp->dpp->default_5gl_iface) {
			for(i = 0; i < cce_ch_num; i++) {
				wapp->dpp->cce_ch.cce_5gl.chan[i] = event_data->cce_ie_result.cce_ch[i];
				DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"%s CCE AP channel list num:%d\n",
						__func__, (int)wapp->dpp->cce_ch.cce_5gl.chan[i]);
			}
			wapp->dpp->cce_ch.cce_5gl.ch_num = cce_ch_num;
		}
	}
	else if(wapp->radio_count == MAP_MAX_RADIO) {
		if (IS_MAP_CH_5GL(wdev->radio->op_ch) && wapp->dpp->default_5gl_iface) {
			for(i = 0; i < cce_ch_num; i++) {
				wapp->dpp->cce_ch.cce_5gl.chan[i] = event_data->cce_ie_result.cce_ch[i];
				DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"%s CCE AP channel list num:%d\n",
						__func__, (int)wapp->dpp->cce_ch.cce_5gl.chan[i]);
			}
			wapp->dpp->cce_ch.cce_5gl.ch_num = cce_ch_num;
		}

		if (IS_MAP_CH_5GH(wdev->radio->op_ch) && wapp->dpp->default_5gh_iface) {
			for(i = 0; i < cce_ch_num; i++) {
				wapp->dpp->cce_ch.cce_5gh.chan[i] = event_data->cce_ie_result.cce_ch[i];
				DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"%s CCE AP channel list num:%d\n",
						__func__, (int)wapp->dpp->cce_ch.cce_5gh.chan[i]);
			}
			wapp->dpp->cce_ch.cce_5gh.ch_num = cce_ch_num;
		}
#ifdef MAP_6E_SUPPORT
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;

		if (ap && IS_OP_CLASS_6G(ap->ch_info.op_class)) {
			for (i = 0; i < cce_ch_num; i++) {
				wapp->dpp->cce_ch.cce_6g.chan[i] = event_data->cce_ie_result.cce_ch[i];
				DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"%s CCE AP channel list num:%d\n",
						__func__, (int)wapp->dpp->cce_ch.cce_6g.chan[i]);
			}
			wapp->dpp->cce_ch.cce_6g.ch_num = cce_ch_num;
		}
#endif
	}
#else
	if (IS_MAP_CH_5GL(wdev->radio->op_ch) && wapp->dpp->default_5gl_iface) {
		for(i = 0; i < cce_ch_num; i++)
			wapp->dpp->cce_ch.cce_5gl.chan[i] = event_data->cce_ie_result.cce_ch[i];
		wapp->dpp->cce_ch.cce_5gl.ch_num = cce_ch_num;
	}

	if (IS_MAP_CH_5GH(wdev->radio->op_ch) && wapp->dpp->default_5gh_iface) {
		for(i = 0; i < cce_ch_num; i++)
			wapp->dpp->cce_ch.cce_5gh.chan[i] = event_data->cce_ie_result.cce_ch[i];
		wapp->dpp->cce_ch.cce_5gh.ch_num = cce_ch_num;
	}

#endif /* MAP_R3 */

}

void wapp_dpp_conn_status_result_wait_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct dpp_authentication *auth = timeout_ctx;
	struct wifi_app *wapp = eloop_ctx;

	if (!wapp || !auth || !wapp->dpp)
		return;

	if (!auth->waiting_conn_status_result
#ifdef MAP_R3
		&& (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_CONFIGURATOR)
#endif /* MAP_R3 */
	) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: Auth not waiting for connection status result\n");
		return;
	}

	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: configurator timeout for Connection Status Result \n");
	auth->waiting_conn_status_result = 0;
	dpp_auth_deinit(auth);
}

void wapp_dpp_conn_result_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = timeout_ctx;

	if (!wapp || !auth || !auth->conn_status_requested)
		return;

	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: enrolle sta connection timeout \n");
	wapp_dpp_send_conn_status_result(wapp, auth, 255);
}

void wapp_dpp_rx_conn_status_result(struct wifi_app *wapp, const u8 *src, const u8 *hdr,
					   const u8 *buf, size_t len)
{
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);
	enum dpp_status_error status;
	u8 ssid[SSID_MAX_LEN + 1] = {0};
	size_t ssid_len = 0;
	char *channel_list = NULL;
#ifdef MAP_R3
	unsigned char reconf_flag = 0;
#endif /* MAP_R3 */

	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: Connection Status Result from" MACSTR " \n",MAC2STR(src));
	if (!auth) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: No DPP Config wait for connection status result - drop \n");
		return;
	}

	if (!auth->waiting_conn_status_result) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: No DPP auth wait conn status reslut \n");
		return;
	}
	auth->waiting_conn_status_result = 0;
	status = dpp_conn_status_result_rx(auth, hdr, buf, len,
					   ssid, &ssid_len, &channel_list);
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONN_STATUS_RESULT
		"result=%d ssid=%s channel_list=%s",
		status, ssid, channel_list ? channel_list : "N/A");
#ifdef MAP_R3
	user_fail_reason *info_to_mapd = os_zalloc(sizeof(user_fail_reason));
	if (info_to_mapd == NULL) {
		if (channel_list)
			os_free(channel_list);
		return;
	}
	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" [%s] \n", __func__);
	info_to_mapd->reason_id = status;
	os_memcpy(info_to_mapd->reason, "DPP Connection Status", os_strlen("DPP Connection Status"));
	wapp_send_1905_msg(
			auth->msg_ctx,
			WAPP_SEND_USER_FAIL_NOTIF,
			sizeof(user_fail_reason),
			(char *)info_to_mapd);
	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" %s [%s] \n", info_to_mapd->reason, __func__);
	os_free(info_to_mapd);

#endif /* MAP_R3 */
	os_free(channel_list);
	wapp_dpp_listen_stop(wapp, auth->wdev);
	eloop_cancel_timeout(wapp_dpp_conn_status_result_wait_timeout,
			     wapp, auth);
#ifdef MAP_R3
	/* Sending chirp notification to clear enrollee state on proxy agent */
	if (auth->is_wired) {
		if (auth->reconfigTrigger)
			reconf_flag = 1;
		ChirpMsg_1905_send(wapp, auth->chirp_tlv.hash_payload, (u8 *)src
#ifdef MAP_R3_RECONFIG
					, reconf_flag
#endif
			, 0, auth->relay_mac_addr);
	}
#endif /* MAP_R3 */

#ifdef MAP_R3_RECONFIG
	//if (auth->reconfigTrigger != 1)
#endif
	dpp_auth_deinit(auth);
}

int wapp_dpp_send_conn_status_result(struct wifi_app *wapp, struct dpp_authentication *auth,
				      enum dpp_status_error result)
{
	struct wpabuf *msg;
	const char *channel_list = NULL;
	char *channel_list_buf = NULL;

	DBGPRINT(RT_DEBUG_TRACE,  DPP_MAP_PREX"DPP: connection status result :%d \n", result);

	if (!auth) {

		DBGPRINT(RT_DEBUG_ERROR,  DPP_MAP_PREX"DPP:  auth is null, return \n");
		return -1;
	}
#ifdef MAP_R3
	if (result == 255) {
		dpp_auth_fail_wrapper(wapp, "DPP: connection status result: 255");
	}
#endif
	if (auth->conn_status_requested != 1) {

		DBGPRINT(RT_DEBUG_ERROR,  DPP_MAP_PREX"DPP:  conn_status_requested is 0, return \n");
		return 0;
	}

	auth->conn_status_requested = 0;
	eloop_cancel_timeout( wapp_dpp_conn_result_timeout, wapp,auth);

	if (wapp->dpp->scan_ch.ch_num) {
		channel_list_buf = wapp_dpp_get_scan_channel_list(wapp);
		channel_list = channel_list_buf;
	}

	msg = dpp_build_conn_status_result(auth, result,
					   auth->wdev->config->ssid, auth->wdev->config->ssid_len, channel_list);
	os_free(channel_list_buf);
	if (!msg) {
		dpp_auth_deinit(auth);
		DBGPRINT(RT_DEBUG_ERROR,  DPP_MAP_PREX"DPP:  msg is null, return \n");
		return -1;
	}

	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR "type=%d",
		MAC2STR(auth->peer_mac_addr), DPP_PA_CONNECTION_STATUS_RESULT);
	wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0, auth->peer_mac_addr,
				wpabuf_head(msg), wpabuf_len(msg));
	wpabuf_free(msg);
#ifndef MAP_R3
	if (result != 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "Connection result failed by reason:%d, deinit the auth\n", result);
		dpp_auth_deinit(auth);
		return -1;
	}
#endif /* MAP_R3 */
	return 0;
}

void wapp_dpp_connected(struct wifi_app *wapp, u32 ifindex)
{
	struct wapp_dev *wdev = NULL;
	struct dpp_authentication *auth = NULL;
	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"dpp connection event \n");

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

#ifdef MAP_R3
	auth = wapp_dpp_get_first_auth(wapp);
#else
	dl_list_for_each(auth, &wapp->dpp->dpp_auth_list, struct dpp_authentication, list) {
		if(auth && auth->wdev) {
			if (os_strcmp((const char *)(auth->wdev->config->ssid), (const char *)(wdev->config->ssid)) == 0)
				break;
		}
	}
#endif /* MAP_R3 */

	if (!auth) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Enrollee first auth not found\n");
		return;
	}

	if (auth && (auth->conn_status_requested == 1)) {
		eloop_cancel_timeout( wapp_dpp_conn_result_timeout, wapp,auth);

		wapp_dpp_send_conn_status_result(wapp, auth, DPP_STATUS_OK);
	}
}

#endif /* DPP_R2_SUPPORT */

#ifdef DPP_R2_RECONFIG
void wapp_dpp_reconfig_auth_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct wapp_dev *wdev = timeout_ctx;
	if(wapp) {
#ifdef MAP_R3
		dpp_auth_fail_wrapper(wapp, "DPP: Timeout while waiting for Reconfig auth req");
#endif
		DBGPRINT(RT_DEBUG_ERROR, 
			   DPP_MAP_PREX"DPP: Timeout while reconfig annouce wait for reauth req (an_status:%d)",
			   wapp->dpp->reconfig_annouce.an_status);
		if(wapp->dpp->reconfig_annouce.an_status !=  DPP_AN_STATUS_SUCCESS
			&& wapp->dpp->reconfig_annouce.is_enable ==1) {
			wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT;
			wapp_dpp_send_reconfig_annouce(wapp, wdev);
		}
	}
}
#ifdef MAP_R3_RECONFIG
void wapp_dpp_reconfig_auth_resp_wait_timeout(void *eloop_ctx,
                                                   void *timeout_ctx)
{
        struct wifi_app *wapp = eloop_ctx;
        struct dpp_authentication *auth = NULL;
        u8 * src_mac= timeout_ctx;

        DBGPRINT(RT_DEBUG_ERROR,
                   DPP_MAP_PREX"DPP: Timeout while waiting for reconfig Authentication response\n");
        if(wapp && src_mac) {
                auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src_mac);
                if(auth)
                        dpp_auth_deinit(auth);
        }
}
#endif
void wapp_dpp_send_reconfig_auth_req(struct dpp_authentication *auth, struct wifi_app *wapp, struct wapp_dev * wdev, const u8 *src)
{
	if (!wapp ) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: input parameter null \n");
		return;
	}

	if (wdev) {
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"\n wdev exist");
		auth->wdev = wdev;
	}
#ifdef MAP_R3_RECONFIG
	auth->reconfigTrigger = TRUE;
	wapp->reconfigTrigger=TRUE;
	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"\n chan is %u", wapp->conf_op_ch);
	auth->curr_chan = wapp->conf_op_ch; //New
    	//auth->wdev = wapp_get_dpp_default_wdev(wapp, wapp->conf_op_ch);
#endif
	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"\n auth1 -"MACSTR "\n" ,PRINT_MAC(src));

	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR "type=%d",
		MAC2STR(auth->peer_mac_addr), DPP_PA_RECONFIG_AUTH_REQ);

	hex_dump_dbg(DPP_MAP_PREX"dpp frame ",(u8 *)wpabuf_head(auth->msg_out), wpabuf_len(auth->msg_out)+1);

	wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0, src,
			wpabuf_head(auth->msg_out), wpabuf_len(auth->msg_out));

	auth->current_state = DPP_STATE_RECONFIG_AUTH_CONF_WAITING;

	/* set register to wait the reauth reesp */
	eloop_cancel_timeout(wapp_dpp_reconfig_auth_resp_wait_timeout, wapp, auth->peer_mac_addr);
	eloop_register_timeout(5, 0, wapp_dpp_reconfig_auth_resp_wait_timeout, wapp, auth->peer_mac_addr);
}
void wapp_dpp_send_reconfig_annouce(struct wifi_app *wapp, struct wapp_dev * wdev_info)
{
	struct wpabuf *msg =  NULL;
	struct wapp_dev * wdev=  NULL;
	int wait_time = DPP_PRESENCE_MIN_TIMEOUT;
	struct dpp_global *dpp = NULL;
	unsigned int chan = 0;
	unsigned int action_wait_time = 0;
#ifdef MAP_R3_RECONFIG
	int target_band = 0;
#endif

	if (!wapp || !wapp->dpp) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed to find wapp, auth not initialized\n");
		dpp_auth_fail_wrapper(wapp,"wapp not initialized reconfig trigger failed\n");		
		return;
	}

	dpp = wapp->dpp;
	if (!dpp->reconfig_annouce.is_enable || dpp->reconfig_annouce.an_status != DPP_AN_STATUS_INIT) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" reconfig annouce frame already start (%d), is_enable (%d)\n",
			dpp->reconfig_annouce.an_status, dpp->reconfig_annouce.is_enable);
		wapp->wdev_backup = NULL;
		return;
	}

	if (!wdev_info && (wapp->wdev_backup != NULL))
		wdev_info = wapp->wdev_backup;
	else if (!wdev_info) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed to find wdev, auth not initialized\n");
		return;
	} else
		wapp->wdev_backup = wdev_info;
		
#ifdef MAP_R3
	wapp->reconfigTrigger = TRUE;

	/* Tigger scan to driver for updating CCE channel list */
	DBGPRINT(RT_DEBUG_TRACE, "\ncce_driver_scan_done - %d,off ch_scan_state-%d",
			wapp->dpp->cce_driver_scan_done, wapp->map->off_ch_scan_state.ch_scan_state);
	if ((((dpp->reconfig_annouce.an_retry == 0 && dpp->reconfig_annouce.cur_an_chan_id == 0)
					|| dpp->reconfig_annouce.an_retry == DPP_RECONF_MAX_RETRY)) && !wapp->dpp->cce_driver_scan_done &&
			(wapp->map->off_ch_scan_state.ch_scan_state == CH_SCAN_IDLE)) {
		wapp->dpp->cce_driver_scan_ongoing = 1;
		wapp_dpp_presence_ch_scan(wapp);
		return;
	}

	if (wapp->dpp->cce_driver_scan_ongoing) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" CCE driver scan ongoing..\n");
		return;
	}

	/* Select default interface and query for cce scan result for first time here*/
	if ((wapp->radio_count > 0) &&
			(!wapp->dpp->default_2g_iface && !wapp->dpp->default_5gl_iface
			 && !wapp->dpp->default_5gh_iface)) {
		wapp_dpp_default_iface(wapp);
		wapp_dpp_query_cce_channel(wapp);
	}

	if (!dpp->cce_scan_ongoing)
		eloop_cancel_timeout(wapp_dpp_cce_res_wait_timeout_recon, wapp, wdev_info);

#endif /* MAP_R3 */


#ifdef RECONFIG_OLD
	msg = dpp_build_reconfig_annouce_frame(dpp, wdev_info->config->dpp_csign, wdev_info->config->dpp_csign_len);
#else
#if 0
	if (wdev_info->config->dpp_reconfig_id)
		dpp_free_reconfig_id(wdev_info->config->dpp_reconfig_id);

	if (!wdev_info->config->dpp_ppkey) {
		DBGPRINT(RT_DEBUG_OFF,
			   "DPP: No PP-Key, no Reconfiguration");
		return;
	}
		
	wdev_info->config->dpp_reconfig_id = dpp_gen_reconfig_id(wdev_info->config->dpp_csign,
						     wdev_info->config->dpp_csign_len,
						     wdev_info->config->dpp_ppkey,
						     wdev_info->config->dpp_ppkey_len);
	if (!wdev_info->config->dpp_reconfig_id) {
		DBGPRINT(RT_DEBUG_OFF,
			   "DPP: Failed to generate E-id for reconfiguration");
		return;
	}
#endif
	msg = dpp_build_reconfig_announcement(
				wdev_info->config->dpp_csign,
				wdev_info->config->dpp_csign_len,
				wdev_info->config->dpp_netaccesskey,
				wdev_info->config->dpp_netaccesskey_len,
				wdev_info->config->dpp_reconfig_id);
#endif
	if(!msg)
		return;

	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"an_retry:%d,  cur_reconfig_chan_id:%d\n",
			dpp->reconfig_annouce.an_retry, dpp->reconfig_annouce.cur_an_chan_id);
	if ((dpp->reconfig_annouce.an_retry == 0 && dpp->reconfig_annouce.cur_an_chan_id == 0
#ifdef MAP_R3_RECONFIG
		&& (dpp->reconfig_annouce.ch_retry == 0)
#endif /* MAP_R3_RECONFIG */
	) || dpp->reconfig_annouce.an_retry == DPP_RECONF_MAX_RETRY) {

		if (dpp->reconfig_annouce.an_retry == DPP_RECONF_MAX_RETRY) {
			/* If max count reached for one cycle, give indication to user*/
			dpp_auth_fail_wrapper(wapp,"Chirp max count reached for one cycle, re-create ch list\n");
		}

		/* Setting the chirp and dpp onboarding ongoing flag */
		dpp->chirp_ongoing = 1;
		dpp->dpp_onboard_ongoing = 1;
		if (dpp->cce_scan_ongoing) {
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"wait for CCE query result\n");
			/* set register to wait the CCE result frm driver, if timeout, resend again */
			eloop_cancel_timeout(wapp_dpp_cce_res_wait_timeout_recon, wapp, wdev_info);
			eloop_register_timeout(5, 0, wapp_dpp_cce_res_wait_timeout_recon, wapp, wdev_info);
			os_free(msg);
			return;
		}

		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"gen annouce channel list\n");
		if (!dpp_gen_reconfig_annouce_channel_list(dpp, &(dpp->reconfig_annouce), wdev_info->radio->op_ch)) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"gen annouce channel list fail\n");
			dpp_auth_fail_wrapper(wapp, "Reconfig gen announce channel list fail\n");
			return;
		}

		if (dpp->cce_scan_ongoing) {
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"wait for CCE query result\n");
			/* set register to wait the CCE result frm driver, if timeout, resend again */
			eloop_cancel_timeout(wapp_dpp_cce_res_wait_timeout_recon, wapp, wdev_info);
			eloop_register_timeout(5, 0, wapp_dpp_cce_res_wait_timeout_recon, wapp, wdev_info);
			os_free(msg);
			return;
		}
		dpp->reconfig_annouce.an_retry = 0;
	}

	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"ch_num:%d,  cur_reconfig_chan_id:%d\n",
			dpp->reconfig_annouce.ch_num, dpp->reconfig_annouce.cur_an_chan_id);

	if(dpp->reconfig_annouce.cur_an_chan_id == dpp->reconfig_annouce.ch_num) {
		/* dpp->reconfig_annouce.cur_an_chan_id++; */
		dpp->reconfig_annouce.an_retry++;
		dpp->reconfig_annouce.cur_an_chan_id = 0;
#ifdef MAP_R3_RECONFIG
		if (dpp->reconfig_annouce.an_retry == DPP_RECONF_MAX_RETRY) {
			DBGPRINT(RT_DEBUG_ERROR, "One cycle for ch list eneded reset the flag here\n");
			wapp->dpp->cce_driver_scan_done = 0;
		}
#endif /* MAP_R3_RECOFIG */
		wait_time = DPP_PRESENCE_MAX_TIMEOUT;
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"pause %d s before repeating the reconfig annouce\n", wait_time);
		eloop_cancel_timeout(wapp_dpp_reconfig_auth_wait_timeout, wapp, wdev_info);
		eloop_register_timeout(wait_time, 0, wapp_dpp_reconfig_auth_wait_timeout, wapp, wdev_info);
		os_free(msg);
		return;
	}
#if 0
	else if (dpp->reconfig_annouce.cur_an_chan_id == dpp->reconfig_annouce.ch_num+1) {
		dpp->reconfig_annouce.an_retry++;
		dpp->reconfig_annouce.cur_an_chan_id = 0;
	}
#endif

	chan = dpp->reconfig_annouce.chan[dpp->reconfig_annouce.cur_an_chan_id];
#ifdef MAP_R3_RECONFIG
	dpp->reconfig_annouce.ch_retry += 1;
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"chan retry time:%d\n", dpp->reconfig_annouce.ch_retry);
#endif /* MAP_R3_RECONFIG */

	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"chan:%d, cur chan_id:%d", chan, dpp->reconfig_annouce.cur_an_chan_id);
#ifdef MAP_R3_RECONFIG
        target_band = (int)wapp_op_band_frm_ch(wapp, chan);
        wdev = wapp_dev_list_lookup_by_band_and_type(wapp, target_band, WAPP_DEV_TYPE_STA);
#else
	wdev = wapp_dpp_get_wdev_by_channel(dpp, chan);
#endif

#ifdef MAP_R3_RECONFIG
	/* Try 3 times on each channel for TK mode*/
	if (dpp->reconfig_annouce.ch_retry == dpp->dpp_reconf_announce_try) {
#endif /* MAP_R3_RECONFIG */
		dpp->reconfig_annouce.cur_an_chan_id++;
#ifdef MAP_R3_RECONFIG
		dpp->reconfig_annouce.ch_retry = 0;
	}
#endif /* MAP_R3_RECONFIG */

	if(!wdev) {
		os_free(msg);
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"send reconfig annouce frame fail, invalid wdev \n");
		return;
	}

	if(wdev->radio) {
		/* move to  channel */
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"send reconfig annouce frame success, channel:%d \n", chan);
		wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR " type=%d",
		MAC2STR(broadcast), DPP_PA_RECONFIG_ANNOUNCEMENT);
		/* send frame */

#ifdef MAP_R3_RECONFIG
		//Increasing ROC time for MAP R3 becuase remote peer may take time in changing channel
		action_wait_time = 1500;
#else
		action_wait_time = 0;
#endif
		wapp_drv_send_action(wapp, wdev, chan, action_wait_time,
					broadcast, wpabuf_head(msg),
					wpabuf_len(msg));
		dpp->reconfig_annouce.an_status = DPP_AN_STATUS_WAIT_AUTHREQ;
	}else {
		wait_time = DPP_PRESENCE_MIN_TIMEOUT;
		dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT;
	}

	dpp->dpp_onboard_ongoing = 1;
	/* set register to wait the reauth req, if timeout, resend again */
	eloop_cancel_timeout(wapp_dpp_reconfig_auth_wait_timeout, wapp, wdev_info);
	eloop_register_timeout(wait_time, 0, wapp_dpp_reconfig_auth_wait_timeout, wapp, wdev_info);
	os_free(msg);
}
#ifndef MAP_R3_RECONFIG
void wapp_dpp_reconfig_auth_resp_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = NULL;
	u8 * src_mac= timeout_ctx;

	DBGPRINT(RT_DEBUG_ERROR,
		   DPP_MAP_PREX"DPP: Timeout while waiting for reconfig Authentication response\n");
	if(wapp && src_mac) {
		auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src_mac);
		if(auth)
			dpp_auth_deinit(auth);
	}
}
#endif
#ifdef RECONFIG_OLD
void wapp_dpp_trigger_reconfig_auth_req(struct wifi_app *wapp, struct wapp_dev * wdev, const u8 *src)
{
	u8 allowed_roles = wapp->dpp->dpp_allowed_roles;
	unsigned int neg_chan = 0;
	struct dpp_authentication *auth = NULL;
	struct wpabuf *msg = NULL;
	struct dpp_bootstrap_info *own_bi = NULL;
	if (!wapp ) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: input parameter null \n");
		return;
	}

	own_bi = dpp_get_own_bi(wapp->dpp);
	if(!own_bi)
		return;

	auth = dpp_reconfig_auth_init(wapp, wdev, NULL, own_bi, allowed_roles, 0);
	if(!auth)
		return;
	auth->wdev = wdev;
#ifdef MAP_R3_RECONFIG
	auth->reconfigTrigger = TRUE;
#endif
	if(wapp)
		auth->own_version = wapp->dpp->version_ctrl;
	if(wapp_dpp_auth_list_insert(wapp, auth) < 0) {
		dpp_auth_deinit(auth);
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: auth insert fail deinit \n");
		return;
	}

	if (dpp_set_configurator(wapp->dpp, wapp,
			 auth, NULL) < 0) {
		dpp_auth_deinit(auth);
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: auth set configuratior info fail \n");
		return;
	}

	auth->neg_chan = neg_chan;
#ifdef MAP_R3_RECONFIG
	auth->chan[0] = 36;  //Fixed channel here for MAP R3 due to crash
#else
	auth->chan[0] = wdev->radio->op_ch;
#endif /* MAP_R3_RECONFIG */
	auth->num_chan = 1;
	if (!is_zero_ether_addr(src))
		os_memcpy(auth->peer_mac_addr, src,
			  MAC_ADDR_LEN);

	msg = dpp_build_reconfig_auth_req(
#ifdef MAP_R3_RECONFIG
	wapp,
#endif
	auth, TRANSACTION_ID);

	if (!msg) {
		dpp_auth_deinit(auth);
		DBGPRINT(RT_DEBUG_ERROR,  DPP_MAP_PREX"DPP:  msg is null, return \n");
		return;
	}
#ifdef MAP_R3_RECONFIG
	if(wapp->dpp->is_map)
		auth->req_msg = msg;
#endif
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR "type=%d",
		MAC2STR(auth->peer_mac_addr), DPP_PA_RECONFIG_AUTH_REQ);
#ifdef MAP_R3_RECONFIG
	if(wapp->dpp->is_map && 
		!auth->is_wired)
#endif
		wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0, auth->peer_mac_addr,
				wpabuf_head(msg), wpabuf_len(msg));
	auth->current_state = DPP_STATE_RECONFIG_AUTH_RESP_WAITING;
	auth->peer_protocol_key = NULL;

#ifdef MAP_R3_RECONFIG
	if(wapp->dpp->is_map){
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Sending frame to map agnt" MACSTR "\n",
					MAC2STR(wapp->dpp->al_mac));	
		dpp_map_init(wapp->dpp, auth, wapp->dpp->al_mac);
	}
#endif	
	/* set register to wait the reauth reesp */
	eloop_cancel_timeout(wapp_dpp_reconfig_auth_resp_wait_timeout, wapp, auth->peer_mac_addr);
#ifndef MAP_R3_RECONFIG	
	eloop_register_timeout(5, 0, wapp_dpp_reconfig_auth_resp_wait_timeout, wapp, auth->peer_mac_addr);
#endif
	wpabuf_free(msg);
}
#else
static void
wapp_dpp_trigger_reconfig_auth_req(struct wifi_app *wapp, struct wapp_dev * wdev, const u8 *src,
				     const u8 *hdr, const u8 *buf, size_t len,
				     unsigned int freq)
{
	const u8 *csign_hash, *fcgroup, *a_nonce, *e_id;
	u16 csign_hash_len, fcgroup_len, a_nonce_len, e_id_len;
	struct dpp_configurator *conf;
//	unsigned int wait_time, max_wait_time;
	u16 group;

	//u8 allowed_roles = wapp->dpp->dpp_allowed_roles;
	unsigned int neg_chan = 0;
	struct dpp_authentication *auth = NULL;
	struct wpabuf *msg = NULL;
	struct dpp_bootstrap_info *own_bi = NULL;
	if (!wapp ) {
		DBGPRINT(RT_DEBUG_OFF, "DPP: input parameter null \n");
		return;
	}
	
	own_bi = dpp_get_own_bi(wapp->dpp);
		if(!own_bi)
			return;

	wpa_printf(MSG_DEBUG, "DPP: Reconfig Announcement from " MACSTR,
		   MAC2STR(src));

	csign_hash = dpp_get_attr(buf, len, DPP_ATTR_C_SIGN_KEY_HASH,
				  &csign_hash_len);
	if (!csign_hash || csign_hash_len != SHA256_MAC_LEN) {
		wpa_printf(MSG_ERROR, "Missing or invalid required Configurator C-sign key Hash attribute");
		return;
	}
	wpa_hexdump(MSG_MSGDUMP, "DPP: Configurator C-sign key Hash (kid)",
		    csign_hash, csign_hash_len);
	conf = dpp_configurator_get_id(wapp->dpp, 1);
	if (!conf) {
		if (dpp_relay_rx_action(wapp->dpp,
					src, hdr, buf, len, freq, NULL,
					NULL) == 0)
			return;
		wpa_printf(MSG_ERROR,
			   "DPP: No matching Configurator information found");
		return;
	}

	fcgroup = dpp_get_attr(buf, len, DPP_ATTR_FINITE_CYCLIC_GROUP,
			       &fcgroup_len);
	if (!fcgroup || fcgroup_len != 2) {
		wpa_printf(MSG_ERROR, "Missing or invalid required Finite Cyclic Group attribute");
		return;
	}
	group = WPA_GET_LE16(fcgroup);
	wpa_printf(MSG_DEBUG, "DPP: Enrollee finite cyclic group: %u", group);

	a_nonce = dpp_get_attr(buf, len, DPP_ATTR_A_NONCE, &a_nonce_len);
	if (!a_nonce)
		a_nonce_len = 0;
	e_id = dpp_get_attr(buf, len, DPP_ATTR_E_PRIME_ID, &e_id_len);
	if (!e_id)
		e_id_len = 0;
	auth = dpp_reconfig_init(wapp, wapp->dpp, conf, own_bi, freq, group, a_nonce, a_nonce_len,
				 e_id, e_id_len);
	if (!auth)
		return;
	
	//msg = dpp_reconfig_build_req(auth);
	//if (!msg)
	//	return;
	
#ifdef MAP_R3_RECONFIG
	auth->reconfigTrigger = TRUE;
#endif
	if(wapp)
		auth->own_version = wapp->dpp->version_ctrl;

	if (!is_zero_ether_addr(src))
		os_memcpy(auth->peer_mac_addr, src,
			  MAC_ADDR_LEN);

	if(wapp_dpp_auth_list_insert(wapp, auth) < 0) {
		dpp_auth_deinit(auth);
		DBGPRINT(RT_DEBUG_OFF, "DPP: auth insert fail deinit \n");
		return;
	}
	
	if (dpp_set_configurator(wapp->dpp, wapp,
			 auth, NULL) < 0) {
		dpp_auth_deinit(auth);
		DBGPRINT(RT_DEBUG_OFF, "DPP: auth set configuratior info fail \n");
		return;
	}
	
		auth->neg_chan = neg_chan;
//#ifdef MAP_R3_RECONFIG
	//	auth->chan[0] = 36;  //Fixed channel here for MAP R3 due to crash
//#else
		if (wdev) {
			auth->chan[0] = wdev->radio->op_ch;
			auth->wdev = wdev;
		}
//#endif /* MAP_R3_RECONFIG */
		auth->num_chan = 1;
#if 1	
		msg = dpp_reconfig_build_req(auth);	
		if (!msg) {
			dpp_auth_deinit(auth);
			DBGPRINT(RT_DEBUG_OFF,	"DPP:  msg is null, return \n");
			return;
		}
#endif
#ifdef MAP_R3_RECONFIG
		if (wapp->dpp->is_map) {
			auth->req_msg = msg;
		wpa_hexdump_buf(MSG_DEBUG,
			"DPP: Again Reconfig Authentication Request frame attributes",
			auth->req_msg);
		}
#endif
		wpa_printf(MSG_INFO1, DPP_EVENT_TX "dst=" MACSTR "type=%d",
			MAC2STR(auth->peer_mac_addr), DPP_PA_RECONFIG_AUTH_REQ);
#ifdef MAP_R3_RECONFIG
		if(wapp->dpp->is_map && 
			(!auth->is_wired && (wdev != NULL))) {
#endif
			wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0, auth->peer_mac_addr,
					wpabuf_head(msg), wpabuf_len(msg));
#ifdef MAP_R3_RECONFIG
		}
#endif

		auth->current_state = DPP_STATE_RECONFIG_AUTH_RESP_WAITING;
		auth->peer_protocol_key = NULL;
	
#ifdef MAP_R3_RECONFIG
		if (wapp->dpp->is_map &&
			!wdev){
			wpa_printf(MSG_DEBUG, "Sending frame to map agnt" MACSTR "\n", MAC2STR(wapp->dpp->al_mac));
			dpp_map_init(wapp->dpp, auth, wapp->dpp->al_mac);
		}
#endif
//Prakhar - Enable Timer	
		/* set register to wait the reauth reesp */
		eloop_cancel_timeout(wapp_dpp_reconfig_auth_resp_wait_timeout, wapp, auth->peer_mac_addr);
//#ifndef MAP_R3_RECONFIG	
		eloop_register_timeout(5, 0, wapp_dpp_reconfig_auth_resp_wait_timeout, wapp, auth->peer_mac_addr);
//#endif
		wpabuf_free(msg);
		msg = NULL;
		auth->req_msg = NULL;
}
#endif

void wapp_dpp_rx_reconfig_annouce(struct wifi_app *wapp, 
					   struct wapp_dev *wdev, const u8 *src, const u8 *buf, size_t len
						, const u8 *hdr, unsigned int chan )
{
	struct dpp_authentication *auth = NULL;
	DBGPRINT(RT_DEBUG_OFF,  DPP_MAP_PREX"DPP: presence annouce frame from " MACSTR " config:%d\n",
		   MAC2STR(src), wapp->dpp->dpp_configurator_supported);

	if (!wapp ||!wapp->dpp || !src || !buf || !len 
#ifndef MAP_R3_RECONFIG
		||!wdev || !wapp->dpp->dpp_configurator_supported
#endif
	) {
#ifdef MAP_R3
                if (wapp)
                        dpp_auth_fail_wrapper(wapp, "DPP: Reconfig announce rx fail, parameters received wrong");
#endif  
		DBGPRINT(RT_DEBUG_ERROR,  DPP_MAP_PREX"DPP: Reconfig announce rx fail, parameters received wrong");
		return;
	}

#ifndef MAP_R3_RECONFIG
	if(!wapp->dpp->reconfig_annouce.is_enable) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"reconfig annouce disabled \n");
		return;
	}
#endif

	auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);
	if(
#ifdef MAP_R3_RECONFIG	
		(
#endif
		auth
#ifdef MAP_R3_RECONFIG
		&& (wapp->dpp->dpp_allowed_roles != DPP_CAPAB_PROXYAGENT))
		|| (auth && (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_PROXYAGENT) &&
		(auth->reconfigTrigger))
#endif
	)
	{
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"previous flow is not ending,reject the reconfig annouce \n");
#ifdef MAP_R3
                dpp_auth_fail_wrapper(wapp, "DPP: Reconfig announce rx fail, already reconfig ongoing, reject new request");
#endif
		return;
	}

#ifdef MAP_R3_RECONFIG
	if ((!wapp->map->TurnKeyEnable) && (chan != wapp->conf_op_ch) && (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_PROXYAGENT)) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" Not forward reconfig announcement\n");
		return;
	}
#endif
	if(dpp_parse_reconfig_annouce(wapp, buf, len
#ifdef MAP_R3_RECONFIG
		,wdev
#endif
	)) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"parse c-sign key martch fail \n");
		return;
	}
	
#ifdef MAP_R3_RECONFIG
	if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_PROXYAGENT &&
		wapp->dpp->is_map && !auth) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"\n send reconfig announcement to Controller");	
		//Send reconfig announcement to controller if proxy agent
			map_dpp_send_reconfig_announce(wapp, buf, src, len, hdr);
	}else if(wapp->dpp->dpp_allowed_roles == DPP_CAPAB_PROXYAGENT &&
		auth) //&& auth->reconfigTrigger)
		wapp_dpp_send_reconfig_auth_req(auth, wapp, wdev, src);	
	else
#endif
	{
#ifdef RECONFIG_OLD
		wapp_dpp_trigger_reconfig_auth_req(wapp, wdev, src);
#else
		
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Controller creating reconfig auth req\n");	
		wapp_dpp_trigger_reconfig_auth_req(wapp, wdev, src, hdr, buf, len, chan);
#endif
	}
#ifdef RECONFIG_OLD
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: trigger to send reconfig auth req\n");
	/* trigger the sending auth request flow */
	wapp_dpp_trigger_reconfig_auth_req(wapp, wdev, src);
#endif
}

void wapp_dpp_reconfig_auth_confirm_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = NULL;
	u8 * src_mac= timeout_ctx;

	DBGPRINT(RT_DEBUG_ERROR,
		   DPP_MAP_PREX"DPP: Timeout while waiting for reconfig Authentication confirm \n");
	if(wapp && src_mac) {
		if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE) {
	                wapp->dpp->reconfig_annouce.is_enable = 1;
        	        wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT;
                	wapp->dpp->dpp_allowed_roles = DPP_CAPAB_ENROLLEE;
	                wapp->dpp->cce_driver_scan_done = 0;

        	        wapp_dpp_send_reconfig_annouce(wapp, wapp->wdev_backup);
		}

		auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src_mac);
		if(auth)
			dpp_auth_deinit(auth);
	}
}

void  dpp_send_reconfig_auth_resp(struct wifi_app *wapp, struct wapp_dev *wdev, const u8 *src, unsigned int chan,
			struct dpp_bootstrap_info *peer_bi, struct dpp_bootstrap_info *own_bi, const u8 * i_nonce,
			u16 i_nonce_len, u8 trans_id, EVP_PKEY *ckey)
{
	struct wpabuf *msg = NULL, *conn_result = NULL;
	struct dpp_authentication *auth  = NULL;
	const char *channel_list = NULL;
	char *channel_list_buf = NULL;

	if (!wapp  || !wdev || !own_bi || !ckey) {
		if(ckey)
			EVP_PKEY_free(ckey);
		return;
	}

	// create auth and init
	auth = os_zalloc(sizeof(*auth));
	if (!auth) {
		EVP_PKEY_free(ckey);
		goto fail;
	}

	os_memset(auth, 0, sizeof(struct dpp_authentication));
	auth->msg_ctx = (void *)wapp;
	auth->peer_bi = peer_bi;
	auth->own_bi = own_bi;
	auth->curve = own_bi->curve;
	auth->curr_chan = chan;
	auth->own_version = 1;
#ifdef MAP_R3_RECONFIG
	auth->reconfigTrigger = TRUE;
#endif
	if(wapp)
		auth->own_version = wapp->dpp->version_ctrl;
	auth->peer_version = 1; /* default to the first version */
       auth->wdev = wdev;
	if(ckey) {
		if(auth->peer_connector_key)
			EVP_PKEY_free(auth->peer_connector_key);
		auth->peer_connector_key = ckey;
	}

	os_memcpy(auth->i_nonce, i_nonce, i_nonce_len);
	if (!is_zero_ether_addr(src))
		os_memcpy(auth->peer_mac_addr, src, MAC_ADDR_LEN);

	/* build connect status  */
	if (wapp->dpp->scan_ch.ch_num) {
		channel_list_buf = wapp_dpp_get_scan_channel_list(wapp);
		channel_list = channel_list_buf;
	}

#if 0
	conn_result = dpp_build_conn_status(auth, auth->wdev->config->conn_result,
					   auth->wdev->config->ssid, auth->wdev->config->ssid_len, channel_list);
#endif
	conn_result = dpp_build_conn_status(DPP_STATUS_NO_AP,
					   auth->wdev->config->ssid, auth->wdev->config->ssid_len, channel_list);
	if (!conn_result)
		goto fail;

	// build the reconfig auth resp
	msg = dpp_build_reconfig_auth_resp(wapp, auth, wdev->config, trans_id, conn_result);
	if (!msg) {
		dpp_auth_deinit(auth);
		DBGPRINT(RT_DEBUG_ERROR,  DPP_MAP_PREX"DPP: the build reconfig auth resp msg is null \n");
		goto fail;
	}

	if (dpp_set_configurator(wapp->dpp, wapp, auth, " ") < 0) {
		dpp_auth_deinit(auth);
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"dpp_set_configurator \n");
		goto fail;
	}

	if(wapp_dpp_auth_list_insert(wapp, auth) < 0) {
		dpp_auth_deinit(auth);
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"dpp auth list insert fail \n");
		goto fail;
	}
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"cur ch:%d, op_ch:%d \n", auth->curr_chan, wdev->radio->op_ch);
	if (auth->curr_chan != wdev->radio->op_ch) {
		auth->wdev = wdev;
		/* Since we are initiator, move to responder's channel */
		wapp_cancel_remain_on_channel(wapp, auth->wdev);  
		wdev_set_quick_ch(wapp, auth->wdev, auth->curr_chan);
	}

	//send the reconfig auth resp
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_TX "dst=" MACSTR ", type=%d",
		MAC2STR(auth->peer_mac_addr), DPP_PA_RECONFIG_AUTH_RESP);
	wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0, auth->peer_mac_addr,
				wpabuf_head(msg), wpabuf_len(msg));
	auth->current_state = DPP_STATE_RECONFIG_AUTH_CONF_WAITING;
	/* set register to wait the auth firm */
	eloop_cancel_timeout(wapp_dpp_reconfig_auth_confirm_wait_timeout, wapp, auth->peer_mac_addr);
	eloop_register_timeout(5, 0, wapp_dpp_reconfig_auth_confirm_wait_timeout, wapp, auth->peer_mac_addr);
	wpabuf_free(conn_result);
	wpabuf_free(msg);

	if (channel_list_buf != NULL) {
		os_free (channel_list_buf);
		channel_list_buf = NULL;
		channel_list = NULL;
	}

	return;

fail:
	if (channel_list_buf != NULL) {
		os_free (channel_list_buf);
		channel_list_buf = NULL;
		channel_list = NULL;
	}

	if (conn_result)
		wpabuf_free(conn_result);
	if (msg)
		wpabuf_free(msg);
	
	if (auth) {
		auth->timeout_recon = 1;
		dpp_auth_deinit(auth);
	}
	
	if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE) {
		wapp->dpp->reconfig_annouce.is_enable = 1;
        	wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT;
	        wapp->dpp->dpp_allowed_roles = DPP_CAPAB_ENROLLEE;
        	wapp->dpp->cce_driver_scan_done = 0;
	

		wapp_dpp_send_reconfig_annouce(wapp, wapp->wdev_backup);
	}
	
}


void wapp_dpp_rx_reconfig_auth_req(struct wifi_app *wapp, struct wapp_dev *wdev, 
					const u8 *src, const u8 *hdr, const u8 *buf, size_t len, unsigned int chan)
{
	const u8 *connector = NULL, *trans_id = NULL,  *version = NULL, *c_nonce = NULL;
	u16 connector_len = 0, trans_id_len = 0, version_len = 0, c_nonce_len = 0;
#ifdef RECONFIG_OLD
	EVP_PKEY *ckey = NULL;
#endif
	//int res = 100;
	struct dpp_bootstrap_info *own_bi = NULL;
	struct dpp_authentication *auth  = NULL;
	struct dpp_authentication *auth_iter = NULL;
#ifdef MAP_R3_RECONFIG
	struct wpabuf *msg;

#endif /* MAP_R3 */
	if (!wapp ||!wapp->dpp 
#ifndef MAP_R3_RECONFIG
		||!wdev
#endif
		 || !src || !buf ) {
#ifdef MAP_R3
		if (wapp)
			dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth rx fail, need to retry reconfig announcement");

		DBGPRINT(RT_DEBUG_ERROR,  DPP_MAP_PREX"DPP: Reconfig auth rx fail, need to retry reconfig announcement " MACSTR " \n",
		   MAC2STR(src));
		
#endif
		goto fail;
	}

	DBGPRINT(RT_DEBUG_OFF,  DPP_MAP_PREX"DPP: reconfig auth req frame from " MACSTR " \n",
		   MAC2STR(src));
		
#ifdef MAP_R3_RECONFIG
	if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_PROXYAGENT) {
		auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);

		if (!auth) {
			dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth rx fail, no auth found in Proxy");
			wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: auth is NULL\n");
		goto fail;
		}
	}
#endif
#ifndef MAP_R3_RECONFIG
	if(!wapp->dpp->reconfig_annouce.is_enable) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"reconfig annouce disabled \n");
		goto fail;

	}
#endif
#ifdef MAP_R3_RECONFIG
	if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_PROXYAGENT) {
	
	   msg = wpabuf_alloc(2 + DPP_HDR_LEN + len);
       if (!msg)
           	return;

       wpabuf_put_u8(msg, WLAN_ACTION_PUBLIC);
       wpabuf_put_u8(msg, WLAN_PA_VENDOR_SPECIFIC);
       wpabuf_put_data(msg, hdr, DPP_HDR_LEN);
       wpabuf_put_data(msg, buf, len);

		auth->curr_chan = wapp->conf_op_ch; //New
		auth->wdev = wapp_get_dpp_default_wdev(wapp, wapp->conf_op_ch);

		wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0, src,
				wpabuf_head(msg), wpabuf_len(msg)); //TODO check this

		wpabuf_free(msg);

		auth->current_state = DPP_STATE_RECONFIG_AUTH_CONF_WAITING;
			
		return;
	}
#endif /* MAP_R3 */
#if 0
	auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);
	if(auth) {
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"previous flow is not ending (status:%d) \n",
				wapp->dpp->reconfig_annouce.an_status);
		if(wapp->dpp->reconfig_annouce.an_status == DPP_AN_STATUS_WAIT_AUTHREQ ) {
			dpp_auth_deinit(auth);
		} else {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"previous flow is not ending,reject the reconfig auth req \n");
			return;
		}
	}
#endif
	dl_list_for_each(auth_iter, &wapp->dpp->dpp_auth_list, struct dpp_authentication, list) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"auth peer mac is "MACSTR"\n", MAC2STR(auth_iter->peer_mac_addr));
		if (wapp->dpp->dpp_allowed_roles != DPP_CAPAB_CONFIGURATOR) {
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"previous flow is not ending (status:%d)\n",
			wapp->dpp->reconfig_annouce.an_status);
			dpp_auth_fail_wrapper(wapp, "DPP: already reconfig ongoing as auth found in auth list");

			if (wapp->dpp->reconfig_annouce.an_status == DPP_AN_STATUS_WAIT_AUTHREQ) {
				wapp->dpp->reconfig_annouce.is_enable = 1;
				wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT;
				wapp->dpp->dpp_allowed_roles = DPP_CAPAB_ENROLLEE;
				wapp->dpp->cce_driver_scan_done = 0;

				wapp_dpp_send_reconfig_annouce(wapp, wapp->wdev_backup);
				dpp_auth_deinit(auth);
			}
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"previous flow is not ending,reject the reconfig auth req\n");
			return;
		}
	}

#if 0
	/* Get first instance, if not NULL and enrollee mode, return */
	if ((wapp_dpp_get_first_auth(wapp) || wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src)) 
		&& (wapp->dpp->dpp_allowed_roles != DPP_CAPAB_CONFIGURATOR)) {
#ifdef MAP_R3
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"previous flow is not ending (status:%d) \n",
                                wapp->dpp->reconfig_annouce.an_status);
                if(wapp->dpp->reconfig_annouce.an_status == DPP_AN_STATUS_WAIT_AUTHREQ ) {
                        dpp_auth_deinit(auth);
                } else {
                        DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"previous flow is not ending,reject the reconfig auth req \n");
                        return;
                }
#endif
	}
#endif
	own_bi = dpp_get_own_bi(wapp->dpp);
	if(!own_bi)
		goto fail;

	/* get trans_id */ 
	trans_id = dpp_get_attr(buf, len, DPP_ATTR_TRANSACTION_ID,
			       &trans_id_len);
	if (!trans_id || trans_id_len != 1) {
		DBGPRINT(RT_DEBUG_ERROR,
			   DPP_MAP_PREX"DPP: Peer did not include Transaction ID");
		dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth rx fail, Peer did not include Transaction ID");
		goto fail;
	}

	/* get version */
	version = dpp_get_attr(buf, len, DPP_ATTR_PROTOCOL_VERSION,
			       &version_len);
	if (version) {
		if (version_len < 1 || version[0] == 0) {
			DBGPRINT(RT_DEBUG_ERROR,
				      DPP_MAP_PREX"Invalid Protocol Version attribute");
			dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth rx fail, Invalid Protocol Version attribute");
			goto fail;
		}

		if(version[0] < 2) {
			DBGPRINT(RT_DEBUG_ERROR,
				DPP_MAP_PREX"peer Protocol Version (%d) is lower, should =>2", version[0]);
			dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth rx fail, peer Protocol Version is lower");
			goto fail;
		}
	} else {
		/* NULL case handling */
		goto fail;
	}

	/* get connector */
	connector = dpp_get_attr(buf, len, DPP_ATTR_CONNECTOR, &connector_len);
	if (!connector) {
		DBGPRINT(RT_DEBUG_ERROR,
			   DPP_MAP_PREX"DPP: Peer did not include its Connector");
		dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth rx fail, peer didn't include its connector");
		goto fail;
	}
	c_nonce = dpp_get_attr(buf, len,
			       DPP_ATTR_CONFIGURATOR_NONCE, &c_nonce_len);
	if (!c_nonce || c_nonce_len > DPP_MAX_NONCE_LEN) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Missing or invalid C-nonce attribute");
		dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth rx fail, invalid cnonce");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, "DPP: C-nonce", c_nonce, c_nonce_len);
	if(wdev && wdev->config && !wdev->config->dpp_csign) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"\n dpp_csign is null returni len -%zu",wdev->config->dpp_csign_len);
		dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth rx fail, dpp csign is NULL");
		goto fail;
	}

#ifdef RECONFIG_OLD
	ckey = dpp_check_valid_connector (wdev->config->dpp_csign,
			     wdev->config->dpp_csign_len, connector, connector_len, wdev->config->dpp_connector,
			     1, &res, c_nonce, c_nonce_len);

	if (res != DPP_STATUS_OK) {
		DBGPRINT(RT_DEBUG_ERROR,
			   DPP_MAP_PREX"DPP: wapp_dpp_rx_reconfig_auth_req check connector fail (peer "
			   MACSTR " status %d)", MAC2STR(src), res);
		goto fail;
	}

	if(!ckey)
	{
		DBGPRINT(RT_DEBUG_ERROR,  DPP_MAP_PREX"DPP: get ckey fail (peer "
			   MACSTR " status %d)", MAC2STR(src), res);
		goto fail;
	}
#else
	eloop_cancel_timeout(wapp_dpp_reconfig_auth_wait_timeout, wapp, wdev);
	dpp_check_valid_connector(wapp, wdev, connector, connector_len,
			src, chan, own_bi, c_nonce, c_nonce_len, &trans_id[0], version);
#endif
	wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_SUCCESS;
	wapp->dpp->reconfig_annouce.cur_an_chan_id = 0;
	wapp->dpp->cce_driver_scan_ongoing = 0;
	/* Resetting the chirp ongoing flag */
	wapp->dpp->chirp_ongoing = 0;
#ifdef RECONFIG_OLD
#if 0
	i_nonce = dpp_get_attr(buf, len, DPP_ATTR_I_NONCE,
			       &i_nonce_len);
	if (!i_nonce || i_nonce_len != own_bi->curve->nonce_len) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Missing or invalid I-nonce");
		EVP_PKEY_free(ckey);
		return;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX DPP_MAP_PREX"DPP: I-nonce", i_nonce, i_nonce_len);
#endif
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: trigger to send reconfig auth req\n");
	/* trigger the sending reconfig auth resp flow */
#ifdef RECONFIG_OLD
	dpp_send_reconfig_auth_resp (wapp, wdev, src, chan, NULL, own_bi, i_nonce,
		i_nonce_len, trans_id[0], ckey);
#endif
	auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);
	if(auth) {
		auth->peer_version = version[0];
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: Peer protocol version %u",
			   auth->peer_version);
	}
	os_memcpy(auth->c_nonce, c_nonce, c_nonce_len);
#endif
	return;
fail:
	if (wapp && wapp->dpp &&
		wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE) {
		wapp->dpp->reconfig_annouce.is_enable = 1;
		wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT;
		wapp->dpp->dpp_allowed_roles = DPP_CAPAB_ENROLLEE;
		wapp->dpp->cce_driver_scan_done = 0;
		wapp_dpp_send_reconfig_annouce(wapp, wapp->wdev_backup);
	}

}

void  dpp_send_reconfig_auth_confirm(struct wifi_app *wapp, struct wapp_dev *wdev, const u8 *src, 
			struct dpp_authentication *auth, int conn_value,  u8 trans_id)
{
	struct wpabuf *msg = NULL;

	// build the reconfig auth resp
	msg = dpp_build_reconfig_auth_confirm(wapp, auth, conn_value, trans_id);
	if (!msg) {
		dpp_auth_deinit(auth);
		DBGPRINT(RT_DEBUG_ERROR,  DPP_MAP_PREX"DPP:  msg is null, return \n");
		return;
	}

#ifdef MAP_R3_RECONFIG
	if(wapp->dpp->is_map)
		auth->req_msg = msg;
#endif

	//send the reconfig auth confirm
#ifdef MAP_R3_RECONFIG
	if(wapp->dpp->is_map && 
		!auth->is_wired)
#endif
		wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0, auth->peer_mac_addr,
				wpabuf_head(msg), wpabuf_len(msg));

	if (wapp->dpp->dpp_configurator_supported)
	{
		auth->current_state = DPP_STATE_CONFIG_REQ_WAITING;
		eloop_cancel_timeout(wapp_dpp_config_req_wait_timeout, wapp, auth);
		eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_config_req_wait_timeout, wapp, auth); //Prakhar
	}
#ifdef MAP_R3_RECONFIG
	if(wapp->dpp->is_map || auth->is_wired) {
		wpabuf_free(auth->msg_out);
		auth->msg_out_pos = 0;
		auth->msg_out = wpabuf_alloc(4 + wpabuf_len(msg) - 1);
		if (!auth->msg_out) {
			wpabuf_free(msg);
			return;
		}
		wpabuf_put_data(auth->msg_out, wpabuf_head(msg) + 1,
				wpabuf_len(msg) - 1);
		auth->current_state = DPP_STATE_RECONFIG_CONFIG_REQ_WAITING;
		dpp_wired_send(auth);
	}
#endif
	wpabuf_free(msg);

}

#ifdef RECONFIG_OLD
void wapp_dpp_rx_reconfig_auth_resp(struct wifi_app *wapp, struct wapp_dev *wdev, 
					const u8 *src, const u8 *buf, size_t len, unsigned int chan)
#else
void wapp_dpp_rx_reconfig_auth_resp(struct wifi_app *wapp, struct wapp_dev *wdev, 
					const u8 *src, const u8 *hdr,const u8 *buf, size_t len, unsigned int chan)
#endif
{
	struct dpp_authentication *auth  = NULL;
	//const u8 *connector = NULL, *trans_id = NULL, *version = NULL;
	//u16 connector_len = 0, trans_id_len = 0, version_len = 0;
	//EVP_PKEY *ckey = NULL;
	//int res = 100;
	//int  conn_value = 0;
	struct wpabuf *msg = NULL;
#ifdef MAP_R3
	u8 ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	struct dpp_agent_info *agnt_info = NULL;
#endif
	if (!wapp ||!wapp->dpp 
#ifndef MAP_R3_RECONFIG
	||!wdev 
#endif
	|| !src || !buf ) {
#ifdef MAP_R3
		if (wapp)
	                dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth response rx fail, parameters value is wrong");
#endif
		DBGPRINT(RT_DEBUG_ERROR,"DPP: reconfig auth resp rx fail, parameters value is wrong \n");
		
		return;
	}

	DBGPRINT(RT_DEBUG_OFF,  "DPP: reconfig auth resp frame from " MACSTR " \n",
		   MAC2STR(src));

#ifndef MAP_R3_RECONFIG
	if(!wapp->dpp->reconfig_annouce.is_enable) {
		DBGPRINT(RT_DEBUG_OFF, "reconfig annouce disabled \n");
		return;
	}
#endif

	
	auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);
	if (!auth) {
		DBGPRINT(RT_DEBUG_OFF, "can not find auth \n");
#ifdef MAP_R3
		dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth response rx fail, auth not exist");
#endif
		return;
	}

	if(auth && auth->current_state != DPP_STATE_RECONFIG_AUTH_RESP_WAITING) {
		DBGPRINT(RT_DEBUG_OFF, "DPP: does not in reconfig auth exchange \n");
#ifdef MAP_R3
		dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth response rx fail, state not correct");
#endif
		return;
	}

	if (os_memcmp(src, auth->peer_mac_addr, ETH_ALEN) != 0) {
		wpa_printf(MSG_DEBUG, "DPP: MAC address mismatch (expected "
			   MACSTR ") - drop", MAC2STR(auth->peer_mac_addr));
#ifdef MAP_R3
		dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth response rx fail, mac address mismatch");
#endif
		return;
	}

	
	wapp_dpp_cancel_timeouts(wapp,auth); /* Timeout shoud be cancelled in all cases */
#ifdef CONFIG_DPP2
        if (!auth->is_wired) {
                //wapp_dpp_cancel_timeouts(wapp,auth);
        /* It may be possible that we got the response in different channel
         * Ideally in that case, timer in the driver may still be running
         * cancel that timer, new channel info will be stored in auth packet
         * and channel switch should be taken care by that.
         */
                wapp_cancel_remain_on_channel(wapp, auth->wdev);
                if (auth->curr_chan != chan && auth->neg_chan == chan) {
                        wpa_printf(MSG_INFO1, DPP_MAP_PREX
                                   "DPP: Responder accepted request for different negotiation channel");
                        auth->curr_chan = chan;
                }

                if (auth->wdev && auth->wdev->radio->op_ch != chan) {
                        /* Since we are initiator, move to responder's channel */
                        wdev_set_quick_ch(wapp, auth->wdev, chan);
                }
        }
#endif

	msg = dpp_reconfig_auth_resp_rx(auth, wapp, hdr, buf, len);
	if (!msg) {
#ifdef MAP_R3
		dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth response rx fail, auth resp rx parsing failed");
#endif
		return;
	}
	dpp_auth_success(auth);
	eloop_cancel_timeout(wapp_dpp_reconfig_auth_resp_wait_timeout, wapp, auth->peer_mac_addr);

	DBGPRINT(RT_DEBUG_OFF, "DPP: trigger to send reconfig auth confirm 9\n");

#ifdef MAP_R3
        os_memcpy(auth->peer_mac_addr, src, MAC_ADDR_LEN);
        if (os_memcmp(auth->relay_mac_addr, ZERO_MAC_ADDR, MAC_ADDR_LEN) == 0) {
                os_memcpy(auth->relay_mac_addr, wapp->dpp->relay_almac_addr, MAC_ADDR_LEN);
                os_memset(wapp->dpp->relay_almac_addr, 0, MAC_ADDR_LEN);
        }
	
	agnt_info = wapp_dpp_get_agent_list_from_agnt_mac_addr(wapp, (u8 *)src);
	if (!agnt_info) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: agnt not found with peer mac addr\n");
	}

        if (agnt_info) {
                os_memcpy(agnt_info->agnt_mac_addr, src, MAC_ADDR_LEN);
                wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: peer mac addr" MACSTR "copied in agnt list with bh type %u\n",
                        MAC2STR(agnt_info->agnt_mac_addr),agnt_info->bh_type);
        }
#endif /* MAP_R3 */

#ifdef MAP_R3_RECONFIG
	if (wapp->dpp->is_map)
		auth->req_msg = msg;
#endif

	//send the reconfig auth confirm
	wpa_printf(MSG_INFO1, DPP_EVENT_TX "dst=" MACSTR "type=%d",
		MAC2STR(auth->peer_mac_addr), DPP_PA_RECONFIG_AUTH_CONF);

#ifdef MAP_R3_RECONFIG
	if (wapp->dpp->is_map && 
		!auth->is_wired) {
#endif
		wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0, auth->peer_mac_addr,
				wpabuf_head(msg), wpabuf_len(msg));
#ifdef MAP_R3_RECONFIG
		auth->dpp_auth_success_on_ack_rx = 1;
	}
#endif
#ifdef MAP_R3_RECONFIG
	if (wapp->dpp->is_map && auth->is_wired) {
		wpabuf_free(auth->msg_out);
		auth->msg_out_pos = 0;
		auth->msg_out = wpabuf_alloc(4 + wpabuf_len(msg) - 1);
		if (!auth->msg_out) {
			wpabuf_free(msg);
			msg = NULL;
			auth->req_msg = NULL;
			return;
		}
		wpabuf_put_data(auth->msg_out, wpabuf_head(msg) + 1,
				wpabuf_len(msg) - 1);
		auth->current_state = DPP_STATE_RECONFIG_CONFIG_REQ_WAITING;
		dpp_wired_send(auth);

		/* Sending chirp notification as per section 5.3.10.2 *
		 * from the spec Wi-Fi_EasyMesh_Specification_v4.pdf  *
		*/
		ChirpMsg_1905_send(wapp, auth->chirp_tlv.hash_payload, (u8 *)src
#ifdef MAP_R3_RECONFIG
					, 1
#endif
			, 0, auth->relay_mac_addr);
	}
#endif
	if (wapp->dpp->dpp_configurator_supported)
	{
		auth->current_state = DPP_STATE_RECONFIG_CONFIG_REQ_WAITING;
		eloop_cancel_timeout(wapp_dpp_config_req_wait_timeout, wapp, auth);
		eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_config_req_wait_timeout, wapp, auth); //Prakhar
	}
	wpabuf_free(msg);
	msg = NULL;
	auth->req_msg = NULL;
	
#ifdef RECONFIG_OLD
	/* get transaction id */ 
	trans_id = dpp_get_attr(buf, len, DPP_ATTR_TRANSACTION_ID,
			       &trans_id_len);
	if (!trans_id || trans_id_len != 1) {
		DBGPRINT(RT_DEBUG_ERROR,
			   DPP_MAP_PREX"DPP: Peer did not include Transaction ID");
		return;
	}

	if (trans_id[0] != TRANSACTION_ID) {
		DBGPRINT(RT_DEBUG_ERROR,
			   DPP_MAP_PREX "DPP: Ignore frame with unexpected Transaction ID %u",
			   trans_id[0]);
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX DPP_EVENT_RECONFIG "peer=" MACSTR
			" fail=transaction_id_mismatch", MAC2STR(src));
		return;
	}
	
	/* get version */
	version = dpp_get_attr(buf, len, DPP_ATTR_PROTOCOL_VERSION,
			       &version_len);
	if (version) {
		if (version_len < 1 || version[0] == 0) {
 			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Invalid Protocol Version attribute");
			return;
		}
		auth->peer_version = version[0];
		if(auth->peer_version < 2) {
			DBGPRINT(RT_DEBUG_ERROR,
				DPP_MAP_PREX"peer Protocol Version (%d) is lower, should =>2", auth->peer_version);
			return;
		}
	}
	/* get connector  */
	connector = dpp_get_attr(buf, len, DPP_ATTR_CONNECTOR, &connector_len);
	if (!connector) {
		DBGPRINT(RT_DEBUG_ERROR,
			   DPP_MAP_PREX"DPP: Peer did not include its Connector");
		return;
	}

#ifdef DPP_R2_RECONFIG
//	ckey = dpp_check_valid_connector(wapp->dpp_csign,
//			     wapp->dpp_csign_len, connector, connector_len, 0, &res);
#else
#if 0
	ckey = dpp_check_valid_connector(wdev->config->dpp_csign,
			     wdev->config->dpp_csign_len, connector, connector_len, 0, &res);
#endif
//	ckey = dpp_check_valid_connector(wapp, wdev, connector, connector_len, src, chan, own_bi, c_nonce, c_nonce_len, &trans_id[0], version)
#endif
	if (res != DPP_STATUS_OK) {
		DBGPRINT(RT_DEBUG_ERROR,
			   DPP_MAP_PREX"DPP: reconfig auth resp connector verify in failure (peer "
			   MACSTR " status %d)", MAC2STR(src), res);
		return;
	}

	if(!ckey)
	{
		DBGPRINT(RT_DEBUG_ERROR,  DPP_MAP_PREX"DPP: get ckey fail (peer "
			   MACSTR " status %d)", MAC2STR(src), res);
		return;
	}

	if(auth->peer_connector_key) {
		EVP_PKEY_free(auth->peer_connector_key);
	}
	auth->peer_connector_key = ckey;

	if(dpp_parse_reconfig_auth_resp(wapp, auth, buf, len, &conn_value) < 0)  {
		DBGPRINT(RT_DEBUG_ERROR,
			   DPP_MAP_PREX"DPP:reconfig auth resp parse in failure (peer "
			   MACSTR " status %d)", MAC2STR(src), conn_value);
		return;
	}

	eloop_cancel_timeout(wapp_dpp_reconfig_auth_resp_wait_timeout, wapp, auth->peer_mac_addr);

	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"DPP: trigger to send reconfig auth confirm 9\n");

	/* trigger the sending reconfig auth resp flow */
	dpp_send_reconfig_auth_confirm(wapp, wdev, src, auth, conn_value, trans_id[0]);
#endif
}

void  dpp_trigger_config_exchange(struct wifi_app *wapp, struct wapp_dev *wdev,
			struct dpp_authentication *auth)
{
	auth->current_state = DPP_STATE_CONFIG_RSP_WAITING;
	wapp_dpp_auth_success(wapp, 0, wdev);
}

void wapp_dpp_rx_reconfig_auth_confirm(struct wifi_app *wapp, struct wapp_dev *wdev, 
					const u8 *src, const u8 *hdr,const u8 *buf, size_t len, unsigned int chan)
{
#ifdef MAP_R3_RECONFIG
	struct wpabuf *msg;
#endif /* MAP_R3 */

	struct dpp_authentication *auth  = NULL;  
#ifdef RECONFIG_OLD
 	int reconfig_value = 0;
#endif
	if (!wapp ||!wapp->dpp 
#ifndef MAP_R3_RECONFIG
                ||!wdev
#endif
	|| !src || !buf ) {
		if (wapp)
			dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth confirm fail, wapp or src or buf  not exist");
		
		DBGPRINT(RT_DEBUG_OFF,  DPP_MAP_PREX"DPP: reconfig auth confirm fail \n");
		goto fail;
	}
	DBGPRINT(RT_DEBUG_OFF,  DPP_MAP_PREX"DPP: reconfig auth confirm frame from " MACSTR " \n",
		   MAC2STR(src));

#ifndef MAP_R3_RECONFIG
	if(!wapp->dpp->reconfig_annouce.is_enable) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"reconfig annouce disabled \n");
		return;
	}
#endif
	auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);
	if(!auth) {
		dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth confirm fail, no auth found");
		DBGPRINT(RT_DEBUG_ERROR,DPP_MAP_PREX "can not find auth \n");
		goto fail;
	}
	if(auth && auth->current_state != DPP_STATE_RECONFIG_AUTH_CONF_WAITING) {
		dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth confirm fail, state not correct");
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: does not in reconfig auth exchange \n");
		goto fail;
	}
	auth->curr_chan = chan;

#ifdef MAP_R3_RECONFIG
	
	//Send auth confirmation received from Controller to Enrollee
	if(auth->is_map_connection && 
		(auth->allowed_roles == DPP_CAPAB_PROXYAGENT)) {
	
		msg = wpabuf_alloc(2 + DPP_HDR_LEN + len);
		if (!msg)
			return;

		wpabuf_put_u8(msg, WLAN_ACTION_PUBLIC);
		wpabuf_put_u8(msg, WLAN_PA_VENDOR_SPECIFIC);
		wpabuf_put_data(msg, hdr, DPP_HDR_LEN);
		wpabuf_put_data(msg, buf, len);

        	auth->curr_chan = wapp->conf_op_ch;
        	//auth->wdev = wapp_get_dpp_default_wdev(wapp, wapp->conf_op_ch);
	
		if (!auth->wdev) {
			wpabuf_free(msg);
			return;
		}
	
		wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0, src,
				wpabuf_head(msg), wpabuf_len(msg)); //TODO check this
	
		wpabuf_free(msg);
	
		auth->current_state = DPP_STATE_CONFIG_REQ_WAITING;
		auth->auth_success = 1;

		eloop_cancel_timeout(wapp_dpp_reconfig_auth_resp_wait_timeout, wapp, auth->peer_mac_addr);
		
		return;
	}
#endif /* MAP_R3 */

	if (os_memcmp(src, auth->peer_mac_addr, ETH_ALEN) != 0) {
		 dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth confirm fail, mac address mismatch");
		wpa_printf(MSG_DEBUG, "DPP: MAC address mismatch (expected "
			   MACSTR ") - drop", MAC2STR(auth->peer_mac_addr));
		goto fail;
	}
	
#ifdef RECONFIG_OLD
	if(dpp_parse_reconfig_auth_confirm(wapp, auth, buf, len, &reconfig_value) < 0)  {
		DBGPRINT(RT_DEBUG_ERROR,
			   DPP_MAP_PREX"DPP:reconfig auth resp parse in failure (peer "
			   MACSTR " status %d)", MAC2STR(src), reconfig_value);
		return;
	}
#else
	if (dpp_reconfig_auth_conf_rx(wapp, auth, hdr, buf, len) < 0) {
		dpp_auth_fail_wrapper(wapp, "DPP: Reconfig auth confirm fail, parsing fail");
		goto fail;
	}
#endif

	eloop_cancel_timeout(wapp_dpp_reconfig_auth_confirm_wait_timeout, wapp, auth->peer_mac_addr);

	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"DPP: trigger to send reconfig auth confirm\n");
	/* trigger the sending configuration flow */
	dpp_trigger_config_exchange(wapp, wdev, auth);
	return;
fail:
	if (wapp && wapp->dpp &&
			wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE) {
		wapp->dpp->reconfig_annouce.is_enable = 1;
        	wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT;
	        wapp->dpp->dpp_allowed_roles = DPP_CAPAB_ENROLLEE;
        	wapp->dpp->cce_driver_scan_done = 0;

	        wapp_dpp_send_reconfig_annouce(wapp, wapp->wdev_backup);
		dpp_auth_deinit(auth);
	}
}

#endif /* DPP_R2_RECONFIG */

