/*
 * hostapd / DPP integration
 * Copyright (c) 2017, Qualcomm Atheros, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See BSD_LICENSE for more details.
 */

#ifndef DPP_WDEV_H
#define DPP_WDEV_H

#ifdef DPP_R2_SUPPORT
#define DPP_AUTH_WAIT_TIMEOUT 10
#else
#define DPP_AUTH_WAIT_TIMEOUT 4
#endif

#define GAS_QUERY_TIMEOUT_PERIOD 10

#define _1905_ENCAP_DPP_MESSAGE_TYPE 0xCD
#define _1905_ENCAP_EAPOL_MESSAGE_TYPE 0xCE
#define DPP_BOOTSTRAP_URI_NOTIFY_TYPE 0xCF
#define DPP_CCE_INDIACTION_NOTIFY_TYPE 0xD2
#define DPP_CCE_FRAME_LEN 0x1
#define DPP_CHIRP_TLV 0xD3
#define _1905_ENCAP_DPP_DIRECT_MESSAGE_TYPE 0xD1

#define DPP_PRESENCE_MIN_TIMEOUT 2
#define DPP_PRESENCE_MAX_TIMEOUT 30
#define DPP_PRESENCE_MAX_RETRY 4
#define DPP_RECONF_MAX_RETRY 2
#define DPP_PRESENCE_LISTEN_TIMEOUT 60
#define DPP_CONNECT_STATUS_TIMEOUT 15
#define DPP_CONT_AGNT_LIST_TIMEOUT 15

void wapp_dpp_rx_action(struct wifi_app *wapp, struct wapp_dev *wdev, const u8 *src,
			const u8 *buf, size_t len, unsigned int chan);
void wapp_dpp_tx_status(struct wifi_app *wapp, const u8 *dst,
			   const u8 *data, size_t data_len, int ok);
struct wpabuf *
wapp_dpp_gas_req_handler(void *ctx, const u8 *sa,
			    const u8 *query, size_t query_len,
			    const u8 *data, size_t data_len);
int wapp_dpp_gas_req_relay_handler(struct wifi_app *wapp, const u8 *sa,
			    const u8 *query, size_t query_len, const u8 *data, size_t data_len);
void wapp_dpp_gas_status_handler(void *ctx, u8 *dst, int ok);
int wapp_dpp_configurator_remove(struct wifi_app *wapp, const char *id);
int wapp_dpp_configurator_get_key(struct wifi_app *wapp, unsigned int id,
				     char *buf, size_t buflen);
int wapp_dpp_pkex_add(struct wifi_app *wapp, struct wapp_dev *wdev, const char *cmd);
int wapp_dpp_pkex_remove(struct wifi_app *wapp, const char *id);
void wapp_dpp_stop(struct wifi_app *wapp);
int wapp_dpp_gas_server_init(struct wifi_app *wapp);
void wapp_ap_dpp_deinit(struct wifi_app *wapp);
int wapp_dpp_controller_start(struct wifi_app *wapp, const char *cmd);

int wapp_dpp_qr_code(struct wifi_app *wapp, const char *cmd);
int wapp_dpp_auth_init(struct wifi_app *wapp, struct wapp_dev *wdev, const char *cmd);
int wapp_dpp_listen(struct wifi_app *wapp, struct wapp_dev *wdev, const char *cmd);
void wapp_dpp_listen_stop(struct wifi_app *wapp, struct wapp_dev *wdev);
int wapp_dpp_configurator_sign(struct wifi_app *wapp, struct wapp_dev *wdev, const char *cmd);
int wapp_dpp_init(struct wifi_app *wapp);
void wapp_dpp_deinit(struct wifi_app *wapp);
int dpp_read_config_file(struct dpp_global *dpp);
int wapp_handle_dpp_scan(struct wifi_app *wapp, struct wapp_dev *wdev);
void wapp_dpp_config_req_wait_timeout(void *eloop_ctx, void *timeout_ctx);
struct dpp_authentication *wapp_dpp_get_last_auth(struct wifi_app *wapp);
#ifdef DPP_R2_MUOBJ
void dpp_save_config_to_file(struct wifi_app *wapp, struct dpp_authentication *auth, struct dpp_config_obj *conf);
#else
void dpp_save_config_to_file(struct wifi_app *wapp, struct dpp_authentication *auth);
#endif /* DPP_R2_MUOBJ */
void dpp_conf_init(struct wifi_app *wapp, wapp_dev_info *dev_info);

void wapp_dpp_scan_channel(struct wifi_app *wapp, struct wapp_dev *wdev);
unsigned char wapp_dpp_get_opclass_from_channel(unsigned char channel);
#ifdef DPP_R2_SUPPORT
int wapp_dpp_set_ie(struct wifi_app *wapp);
void wapp_dpp_presence_annouce(struct wifi_app *wapp, struct dpp_bootstrap_info *dpp_bi);
void wapp_dpp_trigger_auth_req(struct wifi_app *wapp, struct wapp_dev * wdev, struct dpp_bootstrap_info *own_bi,  
	struct dpp_bootstrap_info *peer_bi, struct dpp_authentication *auth);
void wapp_dpp_rx_presence_annouce(struct wifi_app *wapp, struct wapp_dev *wdev,
				const u8 *src, const u8 *buf, size_t len, unsigned int chan);
void wapp_dpp_check_presence_auth_req(struct wifi_app* wapp, const u8 *r_bootstrap,
		struct dpp_bootstrap_info *own_bi, struct dpp_authentication *auth);
struct wapp_dev* wapp_dpp_get_valid_wdev(struct dpp_global *dpp);
void wapp_dpp_presence_auth_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx);
void wapp_dpp_query_cce_channel(struct wifi_app* wapp);
void wapp_dpp_handle_cce_channel(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data);
void wapp_dpp_conn_status_result_wait_timeout(void *eloop_ctx, void *timeout_ctx);
void wapp_dpp_conn_result_timeout(void *eloop_ctx, void *timeout_ctx);
void wapp_dpp_rx_conn_status_result(struct wifi_app *wapp, const u8 *src, const u8 *hdr,
					   const u8 *buf, size_t len);
int wapp_dpp_send_conn_status_result(struct wifi_app *wapp, struct dpp_authentication *auth,
				      enum dpp_status_error result);
void wapp_dpp_connected(struct wifi_app *wapp, u32 ifindex);
void wapp_dpp_config_result_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx);
#endif /* DPP_R2_SUPPORT */
#ifdef MAP_R3
int wapp_dpp_add_chirp_tlv(struct dpp_authentication *auth);
void ChirpMsg_1905_send(struct wifi_app *wapp, u8 *chirp_hash, u8 *peer_mac
#ifdef MAP_R3_RECONFIG
	,unsigned char reconfigure
#endif
	, unsigned char hash_validity, u8 * al_mac);
int CCEIndication_1905_send(struct wifi_app *wapp, unsigned char *almac, unsigned char flag);
void ChirpTLV_1905_send(struct wifi_app *wapp, struct dpp_bootstrap_info *bi);

int dpp_relay_rx_action_map(struct wifi_app *wapp, const u8 *src, const u8 *hdr,
                       const u8 *buf, size_t len, unsigned int chan,
                       const u8 *i_bootstrap, const u8 *r_bootstrap, 
			enum dpp_public_action_frame_type type);
int dpp_relay_rx_gas_req_map(struct wifi_app *wapp, const u8 *src, const u8 *data,
                        size_t data_len);
int wapp_dpp_cce_indication(struct wifi_app *wapp, const char *cmd);
int wapp_dpp_reset_cce_ie(struct wifi_app *wapp);
struct wpabuf * dpp_map_encaps(const u8 *hdr, const u8 *buf, size_t len);
void wapp_snd_bss_conn_1905(struct wifi_app *wapp, struct dpp_authentication *auth);
int dpp_send_chirp_notif_wrapper(struct wifi_app *wapp, const char *cmd);
int dpp_send_chirp_en_disable_wrapper(struct wifi_app *wapp, const char *cmd);
int dpp_send_onboarding_type(struct wifi_app *wapp, const char *cmd);
void wapp_dpp_eth_resp_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx);
void wapp_dpp_agent_list_timeout(void *eloop_ctx,
						void *timeout_ctx);
int wapp_dpp_check_connect(struct wifi_app *wapp, struct wapp_dev *wdev, char *bssid, unsigned char chan);
int get_dpp_parameters(struct map_info *map, char* param, char* value, size_t val_len);
int wapp_dpp_presence_announce_stop(struct wifi_app *wapp);
struct dpp_authentication *wapp_map_dpp_auth_init(struct wifi_app *wapp, struct wapp_dev *wdev,
		struct dpp_bootstrap_info *peer_bi_recv);
void dpp_save_bss_to_file(struct wifi_app *wapp, struct dpp_authentication *auth, struct dpp_bss_cred *bss_cred, u16 credlen);
struct dpp_configurator * dpp_configurator_get_id(struct dpp_global *dpp, unsigned int id);
void dpp_save_dpp_uri_to_file(struct wifi_app *wapp, char *uri_str);
void wapp_dpp_presence_ch_scan(struct wifi_app *wapp);
void wapp_dpp_rx_presence_announce(struct wifi_app *wapp, struct wapp_dev *wdev, 
		const u8 *src, const u8 *hdr, const u8 *buf, size_t len, unsigned int chan);
void wapp_dpp_check_conn_wrapper(void *eloop_ctx,
						   void *timeout_ctx);
void wapp_dpp_auth_rx_wrapper(void *eloop_ctx,
						   void *timeout_ctx);
#endif /* MAP_R3 */
#ifdef DPP_R2_RECONFIG
void wapp_dpp_send_reconfig_annouce(struct wifi_app *wapp, struct wapp_dev * wdev_info);
void wapp_dpp_rx_reconfig_annouce(struct wifi_app *wapp, 
					   struct wapp_dev *wdev, const u8 *src, const u8 *buf, size_t len
						, const u8 *hdr , unsigned int chan
);
void wapp_dpp_rx_reconfig_auth_req(struct wifi_app *wapp, struct wapp_dev *wdev, 
					const u8 *src, const u8 *hdr,const u8 *buf, size_t len, unsigned int chan);
#ifdef RECONFIG_OLD
void wapp_dpp_rx_reconfig_auth_resp(struct wifi_app *wapp, struct wapp_dev *wdev, 
					const u8 *src, const u8 *buf, size_t len, unsigned int chan);
#else
void wapp_dpp_rx_reconfig_auth_resp(struct wifi_app *wapp, struct wapp_dev *wdev, 
					const u8 *src, const u8 *hdr, const u8 *buf, size_t len, unsigned int chan);
#endif
void wapp_dpp_rx_reconfig_auth_confirm(struct wifi_app *wapp, struct wapp_dev *wdev, 
					const u8 *src, const u8 *hdr, const u8 *buf, size_t len, unsigned int chan);
void wapp_dpp_cce_res_wait_timeout_recon(void *eloop_ctx,
                                                   void *timeout_ctx);
void wapp_dpp_reconfig_auth_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx);
#endif /* DPP_R2_RECONFIG */
int dpp_read_config_file_for_connection(struct wifi_app *wapp);
struct dpp_sec_cred * wapp_dpp_save_dpp_cred(struct dpp_authentication *auth);
int dpp_save_config(struct wifi_app *wapp, const char *param, const char *value, char *ifname);
int dpp_delete_parameter_from_file(struct wifi_app *wapp, const char *param);
void wapp_dpp_cancel_timeouts(struct wifi_app *wapp, struct dpp_authentication *auth);
void wapp_dpp_auth_conf_wait_timeout(void *eloop_ctx, void *timeout_ctx);
#endif /* DPP_WDEV_H */
