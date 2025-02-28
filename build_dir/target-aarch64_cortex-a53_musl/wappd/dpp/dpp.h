/*
 * DPP functionality shared between hostapd and wpa_supplicant
 * Copyright (c) 2017, Qualcomm Atheros, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See BSD_LICENSE for more details.
 */

#ifndef DPP_H
#define DPP_H

#include "util.h"
#include <openssl/x509.h>

#include "list.h"
#include "crypto/sha256.h"
#include "wapp_cmm_type.h"
#ifdef DPP_R2_RECONFIG
#include "utils/json.h"
#endif

struct crypto_ecdh;
struct wapp_ip_addr;
struct dpp_global;

#if defined(CONFIG_DPP2) || defined(MAP_R3)
#define DPP_VERSION 2
#else
#define DPP_VERSION 1
#endif

#define DPP_HDR_LEN (4 + 2) /* OUI, OUI Type, Crypto Suite, DPP frame type */
#define DPP_TCP_PORT 7871

#define PMKID_LEN 16
#define PMK_LEN 32
#define PMK_LEN_MAX 64
#define PTK_LEN 64
#define DPP_SHORT_MAX_LEN 65535

#ifndef BIT
#define BIT(n)                          ((UINT32) 1 << (n))
#endif

#define WPA_KEY_MGMT_PSK BIT(1)
#define WPA_KEY_MGMT_FT_PSK BIT(6)
#define WPA_KEY_MGMT_PSK_SHA256 BIT(8)
#define WPA_KEY_MGMT_SAE BIT(10)
#define WPA_KEY_MGMT_FT_SAE BIT(11)
#define WPA_KEY_MGMT_DPP BIT(24)

#define DEFAULT_5GH_IFACE      "wlan0"
#define DEFAULT_5GL_IFACE      "rai0"
#define DEFAULT_2G_IFACE      "ra0"
#define MAX_CONN_TRIES           3

struct wapp_radio;

enum dpp_public_action_frame_type {
	DPP_PA_AUTHENTICATION_REQ = 0,
	DPP_PA_AUTHENTICATION_RESP = 1,
	DPP_PA_AUTHENTICATION_CONF = 2,
	DPP_PA_PEER_DISCOVERY_REQ = 5,
	DPP_PA_PEER_DISCOVERY_RESP = 6,
	DPP_PA_PKEX_EXCHANGE_REQ = 7,
	DPP_PA_PKEX_EXCHANGE_RESP = 8,
	DPP_PA_PKEX_COMMIT_REVEAL_REQ = 9,
	DPP_PA_PKEX_COMMIT_REVEAL_RESP = 10,
	DPP_PA_CONFIGURATION_RESULT = 11,
#ifdef CONFIG_DPP2
	DPP_PA_CONNECTION_STATUS_RESULT = 12,
#endif /* CONFIG_DPP2 */
	DPP_PA_PRESENCE_ANNOUNCEMENT = 13,
#ifdef DPP_R2_RECONFIG
	DPP_PA_RECONFIG_ANNOUNCEMENT = 14,
	DPP_PA_RECONFIG_AUTH_REQ = 15,
	DPP_PA_RECONFIG_AUTH_RESP = 16,
	DPP_PA_RECONFIG_AUTH_CONF = 17,
#endif /* DPP_R2_RECONFIG */
	DPP_PA_UNDEFINED_FRAME = 255,
};

enum dpp_attribute_id {
	DPP_ATTR_STATUS = 0x1000,
	DPP_ATTR_I_BOOTSTRAP_KEY_HASH = 0x1001,
	DPP_ATTR_R_BOOTSTRAP_KEY_HASH = 0x1002,
	DPP_ATTR_I_PROTOCOL_KEY = 0x1003,
	DPP_ATTR_WRAPPED_DATA = 0x1004,
	DPP_ATTR_I_NONCE = 0x1005,
	DPP_ATTR_I_CAPABILITIES = 0x1006,
	DPP_ATTR_R_NONCE = 0x1007,
	DPP_ATTR_R_CAPABILITIES = 0x1008,
	DPP_ATTR_R_PROTOCOL_KEY = 0x1009,
	DPP_ATTR_I_AUTH_TAG = 0x100A,
	DPP_ATTR_R_AUTH_TAG = 0x100B,
	DPP_ATTR_CONFIG_OBJ = 0x100C,
	DPP_ATTR_CONNECTOR = 0x100D,
	DPP_ATTR_CONFIG_ATTR_OBJ = 0x100E,
	DPP_ATTR_BOOTSTRAP_KEY = 0x100F,
	DPP_ATTR_OWN_NET_NK_HASH = 0x1011,
	DPP_ATTR_FINITE_CYCLIC_GROUP = 0x1012,
	DPP_ATTR_ENCRYPTED_KEY = 0x1013,
	DPP_ATTR_ENROLLEE_NONCE = 0x1014,
	DPP_ATTR_CODE_IDENTIFIER = 0x1015,
	DPP_ATTR_TRANSACTION_ID = 0x1016,
	DPP_ATTR_BOOTSTRAP_INFO = 0x1017,
	DPP_ATTR_CHANNEL = 0x1018,
	DPP_ATTR_PROTOCOL_VERSION = 0x1019,
	DPP_ATTR_ENVELOPED_DATA = 0x101A,
#ifdef CONFIG_DPP2
	DPP_ATTR_SEND_CONN_STATUS = 0x101B,
#endif /* CONFIG_DPP2 */
	DPP_ATTR_CONN_STATUS = 0x101C,
#ifdef DPP_R2_RECONFIG
	DPP_ATTR_RECONFIG_FLAGS = 0x101D,
	DPP_ATTR_C_SIGN_KEY_HASH = 0x101E,
	DPP_ATTR_CSR_ATTR_REQ = 0x101F,
	DPP_ATTR_A_NONCE = 0x1020,
	DPP_ATTR_E_PRIME_ID = 0x1021,
	DPP_ATTR_CONFIGURATOR_NONCE = 0x1022,
#endif /* DPP_R2_RECONFIG */
};

enum dpp_status_error {
	DPP_STATUS_OK = 0,
	DPP_STATUS_NOT_COMPATIBLE = 1,
	DPP_STATUS_AUTH_FAILURE = 2,
	DPP_STATUS_UNWRAP_FAILURE = 3,
	DPP_STATUS_BAD_GROUP = 4,
	DPP_STATUS_CONFIGURE_FAILURE = 5,
	DPP_STATUS_RESPONSE_PENDING = 6,
	DPP_STATUS_INVALID_CONNECTOR = 7,
	DPP_STATUS_NO_MATCH = 8,
	DPP_STATUS_CONFIG_REJECTED = 9,
#ifdef DPP_R2_SUPPORT
	DPP_STATUS_NO_AP = 10,
#endif /* DPP_R2_SUPPORT */
};

enum dpp_config_type {
	INFRA_STA,
	INFRA_AP,
	MAP_1905,
	MAP_FRONTHAUL_AP,
	MAP_BACKHAUL_AP,
	MAP_BACKHAUL_STA,
	MAP_1905_CONT,
	INFRA_CONFIGURATOR,
};

#ifdef DPP_R2_SUPPORT
enum dpp_presence_status {
	DPP_PRE_STATUS_INIT = 0,
	DPP_PRE_STATUS_WAIT_AUTHREQ= 1,
	DPP_PRE_STATUS_SUCCESS = 2,
};
#endif /* DPP_R2_SUPPORT */


enum dpp_annouce_status {
	DPP_AN_STATUS_INIT = 0,
	DPP_AN_STATUS_WAIT_AUTHREQ= 1,
	DPP_AN_STATUS_SUCCESS = 2,
};
#ifdef DPP_R2_RECONFIG
/* DPP Reconfig Flags object - connectorKey values */
enum dpp_connector_key {
	DPP_CONFIG_REUSEKEY = 0,
	DPP_CONFIG_REPLACEKEY = 1,
	DPP_CONFIG_UNDEFINED= 255,
};
#endif /* DPP_R2_RECONFIG */

#define DPP_ONBOARDING_TYPE 0
#define DPP_CAPAB_ENROLLEE BIT(0)
#define DPP_CAPAB_CONFIGURATOR BIT(1)
#ifdef MAP_R3
#define DPP_CAPAB_PROXYAGENT BIT(2)
#endif

#define DPP_CAPAB_ROLE_MASK (BIT(0) | BIT(1) | BIT(2))

#define DPP_BOOTSTRAP_MAX_FREQ 30
#define DPP_MAX_NONCE_LEN 32
#define DPP_MAX_HASH_LEN 64
#define DPP_MAX_SHARED_SECRET_LEN 66
#define DPP_MAX_URI_LEN 120

#define DPP_PRESENCE_CH_MAX 90
#define DPP_CCE_CH_MAX 30
#define DPP_RECONF_CH_TRY_MIN 2
#define DPP_CONF_OBJ_MAX 4
#ifdef MAP_R3
#define MAP_MAX_DPP_URI_COUNT 100
#endif /* MAP_R3 */

struct wapp_bss_config {
        char *dpp_connector;
        struct wpabuf *dpp_netaccesskey;
        unsigned int dpp_netaccesskey_expiry;
        struct wpabuf *dpp_csign;
};

/* Note: dont add any variable length variable here.
 * Adding variable length is also not need by looking
 * functionality of this structure.
 * This structure us used to read a predecided format
 * from wts file.
 * after adding any variable here. please compute,
 * MACRO :"WAPP_MAP_REPLY_SIZE_BSS_INFO"
 */
struct GNU_PACKED set_config_bss_info{
	unsigned char mac[ETH_ALEN];
	char oper_class[4];
	char ssid[33];
	unsigned short authmode;
	unsigned short encryptype;
	char key[65];
	unsigned char wfa_vendor_extension;
	unsigned char hidden_ssid;
	/* local */
	unsigned char operating_chan;
	unsigned char is_used;
};

struct wapp_ip_addr {
	int af; /* AF_INET / AF_INET6 */
	union {
		struct in_addr v4;
		u8 max_len[16];
	} u;
};

/* Remote Controller */
struct dpp_relay_controller {
	struct dl_list list;
	struct dpp_global *global;
	u8 pkhash[SHA256_MAC_LEN];
	struct wapp_ip_addr ipaddr;
	void *cb_ctx;
	void (*tx)(void *ctx, struct wapp_dev *wdev, const u8 *addr, unsigned int chan, const u8 *msg,
		   size_t len);
	void (*gas_resp_tx)(void *ctx, const u8 *addr, u8 dialog_token,
			    int prot, struct wpabuf *buf);
	struct dl_list auth; /* struct dpp_connection */
};

struct dpp_annouce_info {
	u8 is_enable;
	unsigned int cur_an_chan_id;
#ifdef MAP_R3
	unsigned int curr_presence_chan;
#endif /* MAP_R3 */
	unsigned int an_retry;
	enum dpp_annouce_status  an_status;
	unsigned int wait_annouce;
	unsigned int ch_num;
#ifdef MAP_R3_RECONFIG
	unsigned int ch_retry;
	unsigned int reconf_ch;
#endif /* MAP_R3_RECONFIG */
	unsigned int chan[DPP_CCE_CH_MAX];
};

#ifdef DPP_R2_SUPPORT
struct dpp_pre_annouce_info {
	u8 is_enable;
	unsigned int cur_presence_chan_id;
#ifdef MAP_R3
	unsigned int curr_presence_chan;
#endif /* MAP_R3 */
	unsigned int presence_retry;
	enum dpp_presence_status  pre_status;
	unsigned int wait_pre_annouce;
	unsigned int ch_num;
#ifdef MAP_R3_RECONFIG
	unsigned int ch_retry;
	unsigned int reconf_ch;
#endif /* MAP_R3_RECONFIG */
	unsigned int chan[DPP_CCE_CH_MAX];
#ifdef MAP_6E_SUPPORT
	unsigned int ch_num_6g;
	unsigned int chan_6g[DPP_CCE_CH_MAX];
	unsigned int cur_presence_chan_id_6g;
#endif
};
#endif /* DPP_R2_SUPPORT */

struct dpp_cce_channel {
	unsigned int ch_num;
	unsigned int chan[DPP_CCE_CH_MAX];
};

struct dpp_cce_channel_info {
	struct dpp_cce_channel cce_2g;
	struct dpp_cce_channel cce_5gl;
	struct dpp_cce_channel cce_5gh;
#ifdef MAP_6E_SUPPORT
	struct dpp_cce_channel cce_6g;
#endif
};

struct dpp_scan_channel {
	unsigned int ch_num;
	unsigned int chan[DPP_BOOTSTRAP_MAX_FREQ];
};

enum dpp_akm {
	DPP_AKM_UNKNOWN,
	DPP_AKM_DPP,
	DPP_AKM_PSK,
	DPP_AKM_SAE,
	DPP_AKM_PSK_SAE,
	DPP_AKM_PSK_DPP,
	DPP_AKM_SAE_DPP,
	DPP_AKM_PSK_SAE_DPP,
};

struct dpp_configuration {
	u8 ssid[32];
	size_t ssid_len;
	enum dpp_akm akm;

	/* For DPP configuration (connector) */
	os_time_t netaccesskey_expiry;

	/* TODO: groups */
	char *group_id;

	/* For legacy configuration */
	char *passphrase;
	u8 psk[32];
	int psk_set;
};

struct dpp_global {
	void *msg_ctx;
	struct dl_list bootstrap; /* struct dpp_bootstrap_info */
	struct dl_list configurator; /* struct dpp_configurator */
	u16 dpp_frame_seq_no;

	/* GAS */
	struct gas_query *gas_query_ctx;
	struct gas_server *gas_server;
	int dpp_gas_dialog_token;

	unsigned int is_map;
	struct wapp_bss_config *conf;
	struct dl_list dpp_auth_list; /* struct dpp_authentication */
        unsigned int dpp_listen_chan;
        u8 dpp_allowed_roles;
        int dpp_qr_mutual;

	/* PKEX */
        struct dpp_pkex *dpp_pkex;
        struct dpp_bootstrap_info *dpp_pkex_bi;
        char *dpp_pkex_code;
        char *dpp_pkex_identifier;
        char *dpp_pkex_auth_cmd;

	/* Variables may need to modify in IOT testing */
        unsigned int dpp_init_max_tries;
        unsigned int dpp_init_retry_time;
        unsigned int dpp_resp_wait_time;
        unsigned int dpp_resp_max_tries;
        unsigned int dpp_resp_retry_time;
	unsigned long max_remain_on_chan;
	
	char dpp_macaddr_key[20];
	char dpp_private_key[512];
	UCHAR dpp_configurator_supported;
	char curve_name[32];

#ifdef DPP_R2_MUOBJ
	unsigned int dpp_conf_ap_num;
	struct dpp_configuration config_ap[DPP_CONF_OBJ_MAX];
	unsigned int dpp_conf_sta_num;
	struct dpp_configuration config_sta[DPP_CONF_OBJ_MAX];
#else
	struct dpp_configuration *conf_ap;
	struct dpp_configuration *conf_sta;
#endif /* DPP_R2_MUOBJ */
	struct dl_list dpp_txstatus_pending_list; /* struct dpp_tx_status */
	struct wapp_dev *default_5gh_iface;
	struct wapp_dev *default_5gl_iface;
	struct wapp_dev *default_2g_iface;
	u8 dpp_max_connection_tries;

#ifdef CONFIG_DPP2
	struct dl_list controllers; /* struct dpp_relay_controller */
	struct dl_list tcp_init; /* struct dpp_connection */
	void *cb_ctx;
	/* MAP config INFO */
	struct set_config_bss_info bss_config[MAX_SET_BSS_INFO_NUM];
	UCHAR bss_config_num;

	// kapil dpp map r3
	int tcp_port;
	u8 is_map_controller;
	u8 al_mac[ETH_ALEN];
	int sock;
	int qr_mutual;
#endif /* CONFIG_DPP2 */
#ifdef DPP_R2_SUPPORT
	struct dpp_pre_annouce_info annouce_enrolle;
	char dpp_mud_url[100];
#endif /* DPP_R2_SUPPORT */
	struct dpp_cce_channel_info cce_ch;
	struct dpp_scan_channel scan_ch;
	unsigned int version_ctrl;
#ifdef MAP_R3
	u8 dpp_eth_onboard_ongoing;
	u8 dpp_wifi_onboard_ongoing; 
	u8 almac_cont[ETH_ALEN];
	u8 dpp_chirp_handling;
	u8 config_done;
	u8 map_sec_done;
#ifdef MAP_R3_RECONFIG
	u8 dpp_reconfig_ongoing;
	u8 radar_detect_ind;
	u8 dpp_reconf_announce_try;
#endif
	struct dl_list dpp_agent_list; /* struct dpp_agent_info */
	u8 cce_scan_ongoing;
	u8 cce_driver_scan_ongoing;
	u8 cce_driver_scan_done;
	u8 cce_rsp_rcvd_cnt;
	u8 chirp_ongoing;
	u8 dpp_map_cont_self;
	u8 dpp_onboard_ongoing;
	u8 dpp_eth_conn_ind;
	u8 wsc_profile_cnt;
	u8 wsc_onboard_done;
	char dpp_chan_list[100];
	u8 qr_code_num;
	u8 relay_almac_addr[ETH_ALEN];
	u8 qr_cmd;
#endif /* MAP_R3 */
#ifdef DPP_R2_RECONFIG
	struct dpp_annouce_info reconfig_annouce;
	u8 reconfig_channel;
#endif /* DPP_R2_RECONFIG */
#ifdef MAP_R3
	u8 chirp_add_ch_num;
	u8 *chirp_list_add;
	u8 chirp_del_ch_num;
	u8 *chirp_list_del;
	u8 onboarding_type;
	u8 conf_res_received;
	char band_priority[5];
	unsigned int prev_chan[15];
	u8 prev_chan_detected;
	u8 chirp_stop_done;
#endif
};

struct bss_info_scan_result {
	struct dl_list list;
	struct scan_bss_info bss;
};

struct map_dev_list {
	struct dl_list list;
	u8 al_mac[ETH_ALEN];
};

struct dpp_curve_params {
	const char *name;
	size_t hash_len;
	size_t aes_siv_key_len;
	size_t nonce_len;
	size_t prime_len;
	const char *jwk_crv;
	u16 ike_group;
	const char *jws_alg;
};

enum dpp_bootstrap_type {
	DPP_BOOTSTRAP_QR_CODE,
	DPP_BOOTSTRAP_PKEX,
};

struct dpp_bootstrap_info {
	struct dl_list list;
	unsigned int id;
	enum dpp_bootstrap_type type;
	char *uri;
	u8 mac_addr[ETH_ALEN];
	char *info;
	unsigned int chan[DPP_BOOTSTRAP_MAX_FREQ];
	unsigned int num_chan;
	u8 version;
	int own;
	EVP_PKEY *pubkey;
	u8 pubkey_hash[SHA256_MAC_LEN];
	const struct dpp_curve_params *curve;
	unsigned int pkex_t; /* number of failures before dpp_pkex
			      * instantiation */
#ifdef DPP_R2_SUPPORT
	u8 chirp_hash[SHA256_MAC_LEN];
#endif /* DPP_R2_SUPPORT */
};

#define PKEX_COUNTER_T_LIMIT 5

struct dpp_pkex {
	void *msg_ctx;
	unsigned int initiator:1;
	unsigned int exchange_done:1;
	unsigned int failed:1;
	struct dpp_bootstrap_info *own_bi;
	u8 own_mac[ETH_ALEN];
	u8 peer_mac[ETH_ALEN];
	char *identifier;
	char *code;
	EVP_PKEY *x;
	EVP_PKEY *y;
	u8 Mx[DPP_MAX_SHARED_SECRET_LEN];
	u8 Nx[DPP_MAX_SHARED_SECRET_LEN];
	u8 z[DPP_MAX_HASH_LEN];
	EVP_PKEY *peer_bootstrap_key;
	struct wpabuf *exchange_req;
	struct wpabuf *exchange_resp;
	unsigned int t; /* number of failures on code use */
	unsigned int exch_req_wait_time;
	unsigned int exch_req_tries;
	unsigned int chan;
	struct wapp_dev *wdev;
};

struct peer_radio_info {
	u8 identifier[6];
	u8 is_bh_sta_supported;
	u8 max_bss;
	u8 operating_chan;
};

enum dpp_auth_state {
	DPP_STATE_DEINIT,
	DPP_STATE_AUTH_RESP_WAITING,
	DPP_STATE_AUTH_CONF_WAITING,
	DPP_STATE_CONFIG_REQ_WAITING,
	DPP_STATE_CONFIG_RSP_WAITING,
	DPP_STATE_CONFIG_RESULT_WAITING,
	DPP_STATE_CONFIG_DONE,
#ifdef DPP_R2_RECONFIG
	DPP_STATE_RECONFIG_AUTH_RESP_WAITING,
	DPP_STATE_RECONFIG_AUTH_CONF_WAITING,
#endif /* DPP_R2_RECONFIG */
#ifdef DPP_R2_RECONFIG
	DPP_STATE_RECONFIG_CONFIG_REQ_WAITING,
#endif
};

struct dpp_tx_status {
	struct dl_list list;
	u8 dst[MAC_ADDR_LEN];
	struct wapp_dev *wdev;
	u16 seq_no;
	struct os_time sent_time;
	u8 is_gas_frame;
};

#ifdef DPP_R2_MUOBJ
struct dpp_config_obj {
	char *connector; /* received signedConnector */
	u8 ssid[SSID_MAX_LEN];
	u8 ssid_len;
	int ssid_charset;
	char passphrase[64];
	u8 psk[PMK_LEN];
	int psk_set;
	enum dpp_akm akm;
//	struct wpabuf *c_sign_key;
};
#endif /* DPP_R2_MUOBJ */

struct dpp_authentication {
	struct dpp_global *global;
	/* Auth list */
	struct dl_list list;
	/* Back pointer to wapp */
	void *msg_ctx;
	/* DPP R1/R2 */
	u8 peer_version;
	u8 own_version;
	const struct dpp_curve_params *curve;
	struct dpp_bootstrap_info *peer_bi;
	struct dpp_bootstrap_info *own_bi;
	struct dpp_bootstrap_info *tmp_own_bi;
	u8 waiting_pubkey_hash[SHA256_MAC_LEN];
	int response_pending;
	enum dpp_status_error auth_resp_status;
	enum dpp_status_error conf_resp_status;

	/* Run time info while going through DPP */
	u8 peer_mac_addr[ETH_ALEN];
	u8 i_nonce[DPP_MAX_NONCE_LEN];
	u8 r_nonce[DPP_MAX_NONCE_LEN];
	u8 e_nonce[DPP_MAX_NONCE_LEN];
	u8 c_nonce[DPP_MAX_NONCE_LEN];
	u8 i_capab;
	u8 r_capab;
	EVP_PKEY *own_protocol_key;
	EVP_PKEY *peer_protocol_key;
	struct wpabuf *req_msg;
	struct wpabuf *resp_msg;

	/* Curve info */
	size_t secret_len;
	u8 Mx[DPP_MAX_SHARED_SECRET_LEN];
	size_t Mx_len;
	u8 Nx[DPP_MAX_SHARED_SECRET_LEN];
	size_t Nx_len;
	u8 Lx[DPP_MAX_SHARED_SECRET_LEN];
	size_t Lx_len;
	u8 k1[DPP_MAX_HASH_LEN];
	u8 k2[DPP_MAX_HASH_LEN];
	u8 ke[DPP_MAX_HASH_LEN];

	/* Intersection of possible frequencies for initiating DPP
	 * Authentication exchange */
	unsigned int chan[DPP_BOOTSTRAP_MAX_FREQ];
	unsigned int num_chan, chan_idx;
	unsigned int curr_chan;
	unsigned int neg_chan;
	unsigned int pre_home_chan;
	unsigned int num_chan_iters;

	int initiator;

	int auth_req_ack;
	unsigned int auth_resp_tries;
	u8 allowed_roles;
	int configurator;
	int remove_on_tx_status;
        int dpp_auth_success_on_ack_rx;

	int auth_success;
#ifdef DPP_R2_RECONFIG
	BOOLEAN reconfig_success;
#endif
	struct wpabuf *conf_req;
	const struct wpabuf *conf_resp; /* owned by GAS server */
	struct dpp_configurator *conf;
#ifdef DPP_R2_MUOBJ
	struct dpp_configuration *config_ap[DPP_CONF_OBJ_MAX];
	struct dpp_configuration *config_sta[DPP_CONF_OBJ_MAX];
	//struct dpp_configurator *conf1[DPP_CONF_OBJ_MAX];
	struct dpp_config_obj     conf_obj[DPP_CONF_OBJ_MAX];
	unsigned int conf_obj_num;
#else
	struct dpp_configuration *conf_ap;
	struct dpp_configuration *conf_sta;

	char *connector; /* received signedConnector */
	u8 ssid[SSID_MAX_LEN];
	u8 ssid_len;
	char passphrase[64];
	u8 psk[PMK_LEN];
	int psk_set;
	enum dpp_akm akm;
#endif /* DPP_R2_MUOBJ */
	struct wpabuf *c_sign_key;
	struct wpabuf *net_access_key;
	os_time_t net_access_key_expiry;
	enum dpp_auth_state current_state;

	struct os_time dpp_last_init;
	struct os_time dpp_init_iter_start;

	/* wdev on which this DPP session is ongoing */
	struct wapp_dev *wdev;
	struct wapp_dev *config_wdev;

#ifdef CONFIG_DPP2
	/* TCP/MAP */
	int sock;
	u8 mac_addr[ETH_ALEN];
	u8 relay_mac_addr[ETH_ALEN];
	u8 msg_len[4];
	size_t msg_len_octets;
	struct wpabuf *msg;
	struct wpabuf *msg_out;
	size_t msg_out_pos;
	u8 is_map_connection;
	u8 is_wired;
	unsigned int read_eloop:1;
	unsigned int write_eloop:1;
	unsigned int on_tcp_tx_complete_gas_done:1;
	unsigned int on_tcp_tx_complete_remove:1;
	unsigned int on_tcp_tx_complete_auth_ok:1;
	struct dpp_relay_controller *relay;
	struct peer_radio_info radio[3];
	u8 bss_index;
	struct chirp_tlv_info chirp_tlv;
#endif

#ifdef CONFIG_DPP2
	int send_conn_status; /* configurator send request */
	int waiting_conn_status_result; /* for configurator wait result */
	int conn_status_requested; /* for sta send result */
#endif /* CONFIG_DPP2 */

#ifdef DPP_R2_RECONFIG
	enum dpp_connector_key reconfig_connector_key;
	EVP_PKEY *own_connector_key;
	EVP_PKEY *peer_connector_key;
	EVP_PKEY *reconfig_old_protocol_key;
	u8 transaction_id;
	int reconfig;
	int waiting_auth_resp;
#endif /* DPP_R2_RECONFIG */
#ifdef MAP_R3
	BOOLEAN ethernetTrigger;
	u8 is_1905_connector;
	u8 self_sign_auth;
	unsigned short decrypt_thresold;
#ifdef MAP_R3_RECONFIG
	u8 reconfigTrigger;
	struct wpabuf *reconfig_resp_msg;
	u8 timeout_recon;
	u8 reconfig_bhconfig_done;
#endif
	u8 bss_conf;
	struct wpabuf *pp_key;
#endif /* MAP_R3 */
	unsigned int auth_req_retry;
};

struct dpp_configurator {
	struct dl_list list;
	unsigned int id;
	int own;
	EVP_PKEY *csign;
	char *kid;
	const struct dpp_curve_params *curve;
	char *connector; /* own Connector for reconfiguration */
	EVP_PKEY *connector_key;
	EVP_PKEY *pp_key;
};

struct dpp_introduction {
	u8 pmkid[PMKID_LEN];
	u8 pmk[PMK_LEN_MAX];
	size_t pmk_len;
};

struct dpp_controller_conf {
        struct dpp_controller_conf *next;
        u8 pkhash[SHA256_MAC_LEN];
        struct wapp_ip_addr ipaddr;
};

struct dpp_relay_config {
	struct wapp_ip_addr ipaddr;
	const u8 *pkhash;

	void *cb_ctx;
	void (*tx)(void *ctx, struct wapp_dev *wdev, const u8 *addr, unsigned int chan, const u8 *msg,
		   size_t len);
	void (*gas_resp_tx)(void *ctx, const u8 *addr, u8 dialog_token, int prot,
			    struct wpabuf *buf);
};

struct pmk_cache {
	u8 pmk;
	u8 pmkid;
	u8 bssid;
        int key_mgmt;
};

#ifdef DPP_R2_RECONFIG
struct dpp_reconfig_id {
	const EC_GROUP *group;
	EC_POINT *e_id; /* E-id */
	EVP_PKEY *csign;
	EVP_PKEY *a_nonce; /* A-NONCE */
	EVP_PKEY *e_prime_id; /* E'-id */
	EVP_PKEY *pp_key;
};

#if 0
const struct dpp_curve_params dpp_curves[] = {
	/* The mandatory to support and the default NIST P-256 curve needs to
	 * be the first entry on this list. */
	{ "prime256v1", 32, 32, 16, 32, "P-256", 19, "ES256" },
	{ "secp384r1", 48, 48, 24, 48, "P-384", 20, "ES384" },
	{ "secp521r1", 64, 64, 32, 66, "P-521", 21, "ES512" },
	{ "brainpoolP256r1", 32, 32, 16, 32, "BP-256", 28, "BS256" },
	{ "brainpoolP384r1", 48, 48, 24, 48, "BP-384", 29, "BS384" },
	{ "brainpoolP512r1", 64, 64, 32, 64, "BP-512", 30, "BS512" },
	{ NULL, 0, 0, 0, 0, NULL, 0, NULL }
};
#endif
#endif

struct dpp_signed_connector_info {
	unsigned char *payload;
	size_t payload_len;
};

struct dpp_config {
        u8 *ssid;
        size_t ssid_len;

        int psk_set;
	u8 psk[32];
        int key_mgmt;

        char *dpp_connector;

        u8 *dpp_netaccesskey;
        size_t dpp_netaccesskey_len;
        unsigned int dpp_netaccesskey_expiry;

        u8 *dpp_csign;
        size_t dpp_csign_len;
	struct pmk_cache pmk_list[16];
#ifdef DPP_R2_RECONFIG
	int conn_result;
#endif /* DPP_R2_RECONFIG */
        u8 *dpp_ppkey;
        size_t dpp_ppkey_len;
	u8 akm;
	enum dpp_akm map_bss_akm;
#ifdef DPP_R2_RECONFIG
	struct dpp_reconfig_id *dpp_reconfig_id;
#endif

};
#if MAP_R3
enum dpp_agent_state {
	DPP_AGT_STATE_INIT,
	DPP_AGT_STATE_DPP_DONE,
};

struct dpp_agent_info {
	struct dl_list list;
	u8 agnt_mac_addr[ETH_ALEN];
	u8 resp_pubkey_hash[SHA256_MAC_LEN];
	u8 chirp_hash[SHA256_MAC_LEN];
	enum dpp_agent_state agent_state;
	u8 bh_type;
};
struct GNU_PACKED onboard_notify {
	u8 almac[ETH_ALEN];
	u8 onboard_status;
	u8 hash_len;
	u8 hash[0];		
};
#endif /* MAP_R3 */

#if 0
struct GNU_PACKED dpp_sec_cred {

	char *connector;
	size_t connector_len;

        u8 *netaccesskey;
        size_t netaccesskey_len;
        unsigned int netaccesskey_expiry;

        u8 *csign;
        size_t csign_len;
};
#endif

#ifdef MAP_R3
int  dpp_reset_dpp_config_file(struct wifi_app *wapp);
int  dpp_reset_mapd_user_config_file(struct wifi_app *wapp);
#endif 

void dpp_bootstrap_info_free(struct dpp_bootstrap_info *info);
const char * dpp_bootstrap_type_txt(enum dpp_bootstrap_type type);
int dpp_bootstrap_key_hash(struct dpp_bootstrap_info *bi);
int dpp_parse_uri_chan_list(struct dpp_bootstrap_info *bi,
			    const char *chan_list
#ifdef MAP_R3
				, struct dpp_global *dpp
#endif
		);
int dpp_parse_uri_mac(struct dpp_bootstrap_info *bi, const char *mac);
int dpp_parse_uri_info(struct dpp_bootstrap_info *bi, const char *info);
struct dpp_bootstrap_info * dpp_parse_qr_code(const char *uri
#ifdef MAP_R3
		, struct dpp_global *dpp
#endif
		);
char * dpp_keygen(struct dpp_bootstrap_info *bi, const char *curve,
		  const u8 *privkey, size_t privkey_len);
struct dpp_authentication * dpp_auth_init(void *msg_ctx,
					  struct wapp_dev *wdev,
					  struct dpp_bootstrap_info *peer_bi,
					  struct dpp_bootstrap_info *own_bi,
					  u8 dpp_allowed_roles,
					  unsigned int neg_chan);

struct dpp_authentication *
dpp_auth_req_rx(void *msg_ctx, u8 dpp_allowed_roles, int qr_mutual,
		struct dpp_bootstrap_info *peer_bi,
		struct dpp_bootstrap_info *own_bi,
		unsigned int chan, const u8 *hdr, const u8 *attr_start,
		size_t attr_len);
struct wpabuf *
dpp_auth_resp_rx(struct dpp_authentication *auth, const u8 *hdr,
		 const u8 *attr_start, size_t attr_len);
struct wpabuf * dpp_build_conf_req(struct dpp_authentication *auth,
				   const char *json);
int dpp_auth_conf_rx(struct dpp_authentication *auth, const u8 *hdr,
		     const u8 *attr_start, size_t attr_len);
int dpp_notify_new_qr_code(struct dpp_authentication *auth,
			   struct dpp_bootstrap_info *peer_bi);
struct dpp_configuration * dpp_configuration_alloc(const char *type);
int dpp_akm_psk(enum dpp_akm akm);
int dpp_akm_sae(enum dpp_akm akm);
int dpp_akm_legacy(enum dpp_akm akm);
int dpp_akm_dpp(enum dpp_akm akm);
int dpp_akm_ver2(enum dpp_akm akm);
int dpp_configuration_valid(const struct dpp_configuration *conf);
void dpp_configuration_free(struct dpp_configuration *conf);
int dpp_set_configurator(struct dpp_global *dpp, void *msg_ctx,
			 struct dpp_authentication *auth,
			 const char *cmd);
void dpp_auth_deinit(struct dpp_authentication *auth);
struct wpabuf *
dpp_conf_req_rx(struct dpp_authentication *auth, const u8 *attr_start,
		size_t attr_len);
int dpp_conf_resp_rx(struct dpp_authentication *auth,
		     const struct wpabuf *resp);
enum dpp_status_error dpp_conf_result_rx(struct dpp_authentication *auth,
					 const u8 *hdr,
					 const u8 *attr_start, size_t attr_len);
struct wpabuf * dpp_build_conf_result(struct dpp_authentication *auth,
				      enum dpp_status_error status);
struct wpabuf * dpp_alloc_msg(enum dpp_public_action_frame_type type,
			      size_t len);
const u8 * dpp_get_attr(const u8 *buf, size_t len, u16 req_id, u16 *ret_len);
int dpp_check_attrs(const u8 *buf, size_t len);
int dpp_key_expired(const char *timestamp, os_time_t *expiry);
const char * dpp_akm_str(enum dpp_akm akm);
int dpp_configurator_get_key(const struct dpp_configurator *conf, char *buf,
			     size_t buflen);
void dpp_configurator_free(struct dpp_configurator *conf);
struct dpp_configurator *
dpp_keygen_configurator(const char *curve, const u8 *privkey,
			size_t privkey_len
#ifdef MAP_R3
			, const u8 *pp_key, size_t pp_key_len
#endif /* MAP_R3 */
);
int dpp_configurator_own_config(struct dpp_authentication *auth,
				const char *curve, int ap
#ifdef MAP_R3
	, int is_map
#endif /* MAP_R3 */	
);
enum dpp_status_error
dpp_peer_intro(struct dpp_introduction *intro, const char *own_connector,
	       const u8 *net_access_key, size_t net_access_key_len,
	       const u8 *csign_key, size_t csign_key_len,
	       const u8 *peer_connector, size_t peer_connector_len,
	       os_time_t *expiry);
struct dpp_pkex * dpp_pkex_init(void *msg_ctx, struct dpp_bootstrap_info *bi,
				const u8 *own_mac,
				const char *identifier,
				const char *code);
struct dpp_pkex * dpp_pkex_rx_exchange_req(void *msg_ctx,
					   struct dpp_bootstrap_info *bi,
					   const u8 *own_mac,
					   const u8 *peer_mac,
					   const char *identifier,
					   const char *code,
					   const u8 *buf, size_t len);
struct wpabuf * dpp_pkex_rx_exchange_resp(struct dpp_pkex *pkex,
					  const u8 *peer_mac,
					  const u8 *buf, size_t len);
struct wpabuf * dpp_pkex_rx_commit_reveal_req(struct dpp_pkex *pkex,
					      const u8 *hdr,
					      const u8 *buf, size_t len);
int dpp_pkex_rx_commit_reveal_resp(struct dpp_pkex *pkex, const u8 *hdr,
				   const u8 *buf, size_t len);
void dpp_pkex_free(struct dpp_pkex *pkex);

char * dpp_corrupt_connector_signature(const char *connector);


struct dpp_pfs {
	struct crypto_ecdh *ecdh;
	const struct dpp_curve_params *curve;
	struct wpabuf *ie;
	struct wpabuf *secret;
};

struct dpp_pfs * dpp_pfs_init(const u8 *net_access_key,
			      size_t net_access_key_len);
int dpp_pfs_process(struct dpp_pfs *pfs, const u8 *peer_ie, size_t peer_ie_len);
void dpp_pfs_free(struct dpp_pfs *pfs);

struct dpp_bootstrap_info * dpp_add_qr_code(struct dpp_global *dpp,
					    const char *uri);
int dpp_bootstrap_gen_at_bootup(struct dpp_global *dpp, char *key, char *mac, char *chan);
int dpp_bootstrap_gen(struct dpp_global *dpp, const char *cmd);
struct dpp_bootstrap_info *
dpp_bootstrap_get_id(struct dpp_global *dpp, unsigned int id);
int dpp_bootstrap_remove(struct wifi_app *wapp,struct dpp_global *dpp, const char *id);
struct dpp_bootstrap_info *
dpp_pkex_finish(struct dpp_global *dpp, struct dpp_pkex *pkex, const u8 *peer,
		unsigned int chan);
const char * dpp_bootstrap_get_uri(struct dpp_global *dpp, unsigned int id);
int dpp_bootstrap_info(struct dpp_global *dpp, int id,
		       char *reply, int reply_size);
void dpp_bootstrap_find_pair(struct dpp_global *dpp, const u8 *i_bootstrap,
			     const u8 *r_bootstrap,
			     struct dpp_bootstrap_info **own_bi,
			     struct dpp_bootstrap_info **peer_bi);
int dpp_configurator_add(struct dpp_global *dpp, const char *cmd);
int dpp_configurator_remove(struct dpp_global *dpp, const char *id);
int dpp_configurator_get_key_id(struct dpp_global *dpp, unsigned int id,
				char *buf, size_t buflen);
int dpp_relay_add_controller(struct dpp_global *dpp,
			     struct dpp_relay_config *config);
int dpp_relay_rx_action(struct dpp_global *dpp, const u8 *src, const u8 *hdr,
			const u8 *buf, size_t len, unsigned int chan,
			const u8 *i_bootstrap, const u8 *r_bootstrap);
int dpp_relay_rx_gas_req(struct dpp_global *dpp, const u8 *src, const u8 *data,
			 size_t data_len);
int dpp_controller_start(struct dpp_global *dpp);
void dpp_controller_stop(struct dpp_global *dpp);
int dpp_tcp_init(struct dpp_global *dpp, struct dpp_authentication *auth,
		 const struct wapp_ip_addr *addr, int port);
int dpp_map_init(struct dpp_global *dpp, struct dpp_authentication *auth, u8 *alid);

void dpp_global_clear(struct dpp_global *dpp);
void dpp_global_deinit(struct dpp_global *dpp);
void dpp_controller_process_rx(struct wifi_app *wapp, void *ctx, const u8 *pos, u8 *src, size_t len);
void map_process_dpp_packet(struct wifi_app *wapp, u8 *msg, int len, u8 *src, u8 *almac);
#ifdef DPP_R2_MUOBJ
void dpp_free_config_info(struct dpp_configuration *conf, unsigned int start, unsigned int end);
int wapp_dpp_handle_config_obj(struct wifi_app *wapp,
					  struct dpp_authentication *auth, struct dpp_config_obj *conf);
#else
int wapp_dpp_handle_config_obj(struct wifi_app *wapp,
					  struct dpp_authentication *auth);
#endif  /* DPP_R2_MUOBJ */
int wapp_dpp_configurator_add(struct dpp_global *dpp);
void wdev_handle_dpp_action_frame(struct wifi_app *wapp,
	struct wapp_dev *wdev, struct wapp_dpp_action_frame *frame);
void wdev_handle_dpp_frm_tx_status(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);
void wdev_get_dpp_action_frame(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data);
void wapp_dpp_auth_list_remove(struct dpp_authentication *auth);
int wapp_drv_send_action(struct wifi_app *wapp, struct wapp_dev *wdev, unsigned int chan,
		unsigned int wait, const u8 *dst, const u8 *data,
		size_t len);
struct dpp_tx_status *wapp_dpp_get_status_info_from_sq(struct wifi_app *wapp, u16 seq_no);
int wapp_dpp_process_conf_obj(void *ctx,
				struct dpp_authentication *auth);
int dpp_wired_send(struct dpp_authentication *auth);
void dpp_conn_tx_ready(int sock, void *eloop_ctx, void *sock_ctx);
void wapp_dpp_auth_success(struct wifi_app *wapp, int initiator, struct wapp_dev *wdev);
int wapp_dpp_process_frames(struct wifi_app *wapp, struct wapp_dev *wdev, const u8 *src,
				const u8 *hdr, const u8 *buf, size_t len,  unsigned int chan,
				enum dpp_public_action_frame_type type);
void dpp_write_adv_proto(struct wpabuf *buf, u8 build_req);
void dpp_write_gas_query(struct wpabuf *buf, struct wpabuf *query);
struct dpp_authentication *wapp_dpp_get_auth_from_peer_mac(struct wifi_app *wapp, u8 *addr);
int wapp_dpp_auth_list_insert(struct wifi_app *wapp, struct dpp_authentication *auth);
void dpp_map_controller_start(struct wifi_app *wapp);
struct wapp_dev *wapp_get_dpp_default_wdev(struct wifi_app *wapp, unsigned int chan);
u8 wapp_op_band_frm_ch(struct wifi_app *wapp, unsigned int chan);
unsigned int get_chan_from_ch_list_str(char * chan_list_str, unsigned int chan[]);
struct wpabuf * dpp_bootstrap_key_der(EVP_PKEY *key);
enum dpp_akm dpp_akm_from_str(const char *akm);
#ifdef MAP_R3
struct dpp_authentication * dpp_auth_init_relay(void *msg_ctx,
					  unsigned char *almac,
					  unsigned char *sta_addr,
					  unsigned char *chirp_buf,
					  unsigned char *auth_buf,
					  int len, int rem_buf_len);

void wapp_dpp_relay_gas_resp_tx(void *ctx, const u8 *addr,
					  u8 dialog_token, int prot,
					  struct wpabuf *buf);
struct dpp_authentication *wapp_dpp_get_auth_from_hash_val(struct wifi_app *wapp, u8 *hash_val);
struct dpp_authentication *wapp_dpp_get_auth_from_resp_hash_val(struct wifi_app *wapp, u8 *hash_val);
struct dpp_authentication *wapp_dpp_get_own_auth_cont(struct wifi_app *wapp);
int wapp_dpp_agnt_uri(struct wifi_app *wapp, char *uri);
int wapp_dpp_set_1905_sec(struct wifi_app *wapp, u8 flag);
struct dpp_agent_info *wapp_dpp_get_agent_list_from_resp_hash_val(struct wifi_app *wapp, u8 *hash_val);
struct dpp_agent_info *wapp_dpp_get_agent_list_from_chirp_hash_val(struct wifi_app *wapp, u8 *hash_val);
struct dpp_agent_info *wapp_dpp_get_agent_list_from_agnt_mac_addr(struct wifi_app *wapp, u8 *addr);
int wapp_dpp_ch_validation(struct wifi_app *wapp, u8 ch);
void wapp_dpp_default_iface(struct wifi_app *wapp);
int wapp_dpp_ch_list_chirp_del(int *ch_list, unsigned int ch,
					unsigned int tot_num);
int wapp_dpp_get_conf_frm_file(struct wifi_app *wapp, const char *str);
int wapp_dpp_set_onboarding_type(struct wifi_app *wapp, u8 flag);
int dpp_map_wsc_done_chirp_channel_list(struct wifi_app *wapp, struct dpp_pre_annouce_info *annouce_info);
void wapp_dpp_get_uri_frm_file(struct wifi_app *wapp);
void wapp_dpp_parse_ch_list_agnt_uri(struct wifi_app *wapp, struct dpp_bootstrap_info *bi);
void wapp_dpp_init_notify_handler(struct wifi_app *wapp);
#endif /* MAP_R3 */
struct dpp_authentication *wapp_dpp_get_first_auth(struct wifi_app *wapp);

struct dpp_bootstrap_info *dpp_get_own_bi(struct dpp_global *dpp);
int dpp_handle_presence_channel_dup(int *chan_list, unsigned int len);
#ifdef DPP_R2_SUPPORT
struct wpabuf * dpp_build_annouce_frame(struct dpp_global *dpp, struct dpp_bootstrap_info *bootinfo);
int dpp_gen_presence_annouce_channel_list(struct dpp_global *dpp, struct dpp_pre_annouce_info *annouce_info);
int dpp_chirp_key_hash(struct dpp_bootstrap_info *bi);
struct dpp_bootstrap_info *dpp_chirp_find_pair(struct dpp_global *dpp, const u8 *r_bootstrap);
enum dpp_status_error dpp_conn_status_result_rx(struct dpp_authentication *auth,
						const u8 *hdr, const u8 *attr_start, size_t attr_len,
						u8 *ssid, size_t *ssid_len, char **channel_list);
struct wpabuf * dpp_build_conn_status_result(struct dpp_authentication *auth,
					     enum dpp_status_error result, const u8 *ssid, size_t ssid_len,
					     const char *channel_list);
#endif  /* DPP_R2_SUPPORT */

#ifdef DPP_R2_RECONFIG
void wapp_dpp_reconfig_auth_resp_wait_timeout(void *eloop_ctx, void *timeout_ctx);
struct wpabuf * dpp_build_conn_status(enum dpp_status_error result,
					     const u8 *ssid, size_t ssid_len,
					     const char *channel_list);
struct wpabuf * dpp_build_reconfig_annouce_frame(struct dpp_global *dpp, const u8 *csign_key,
                      size_t csign_key_len);
//#ifdef MAP_R3_RECONFIG

struct wpabuf * dpp_build_reconfig_announcement(const u8 *csign_key,
						size_t csign_key_len,
						const u8 *net_access_key,
						size_t net_access_key_len,
						struct dpp_reconfig_id *id);
struct dpp_reconfig_id * dpp_gen_reconfig_id(const u8 *csign_key,
					     size_t csign_key_len,
					     const u8 *pp_key,
					     size_t pp_key_len);
int dpp_update_reconfig_id(struct dpp_reconfig_id *id);
void dpp_free_reconfig_id(struct dpp_reconfig_id *id);
int dpp_connector_match_groups(struct json_token *own_root,
				      struct json_token *peer_root, BOOLEAN reconfig);


EVP_PKEY * dpp_build_csign_hash(const u8 *csign_key, size_t csign_key_len);
//#endif
int dpp_gen_reconfig_annouce_channel_list(struct dpp_global *dpp, struct dpp_annouce_info *annouce_info
			, u8 wdev_ch);
struct dpp_authentication * dpp_reconfig_auth_init(void *msg_ctx,
					  struct wapp_dev *wdev,
					  struct dpp_bootstrap_info *peer_bi,
					  struct dpp_bootstrap_info *own_bi,
					  u8 dpp_allowed_roles,
					  unsigned int neg_chan);
struct wpabuf * dpp_build_reconf_connetor(struct dpp_authentication *auth);
struct wpabuf * dpp_build_reconfig_auth_req(
#ifdef MAP_R3_RECONFIG
	struct wifi_app *wapp,
#endif
	struct dpp_authentication *auth, u8 trans_id);
#if 0
EVP_PKEY *
dpp_check_valid_connector(const u8 *csign_key, size_t csign_key_len,
	       const u8 *peer_connector, size_t peer_connector_len,
	       int is_enrolle, int * result);
#endif
struct dpp_authentication * dpp_check_valid_connector(struct wifi_app *wapp, struct wapp_dev *wdev,
	const u8 *peer_connector, size_t peer_connector_len, const u8 *src, unsigned int chan, struct dpp_bootstrap_info *own_bi,
	const u8 *c_nonce, u16 c_nonce_len, const u8 *trans_id, const u8 *version);

EVP_PKEY * dpp_parse_jwk(struct json_token *jwk, const struct dpp_curve_params **key_curve);

int dpp_reconfig_derive_ke_responder(struct dpp_authentication *auth,
				     const u8 *net_access_key,
				     size_t net_access_key_len,
				     struct json_token *peer_net_access_key);

const struct dpp_curve_params * dpp_get_curve_ike_group(u16 group);

//#endif

struct wpabuf * dpp_build_reconfig_auth_resp(struct wifi_app *wapp, struct dpp_authentication *auth,
				struct dpp_config *config, u8 trans_id, struct wpabuf *conn_result);
int dpp_parse_reconfig_auth_resp(struct wifi_app *wapp, struct dpp_authentication *auth,
				const u8 *buf, size_t buf_len, int *conn_value);
struct wpabuf * dpp_build_reconfig_auth_confirm(struct wifi_app *wapp, 
				struct dpp_authentication *auth, int conn_value,  u8 trans_id);
int dpp_parse_reconfig_auth_confirm(struct wifi_app *wapp, struct dpp_authentication *auth,
				const u8 *buf, size_t buf_len, int *reconfig_value);
struct wpabuf * dpp_reconfig_auth_resp_rx(struct dpp_authentication *auth, struct wifi_app *wapp, const u8 *hdr,
			  const u8 *attr_start, size_t attr_len);
int dpp_reconfig_derive_ke_initiator(struct dpp_authentication *auth,
				     const u8 *r_proto, u16 r_proto_len,
				     struct json_token *net_access_key);
int dpp_reconfig_auth_conf_rx(struct wifi_app *wapp, struct dpp_authentication *auth, const u8 *hdr,
			      const u8 *attr_start, size_t attr_len);
EC_POINT * dpp_decrypt_e_id(EVP_PKEY *ppkey, EVP_PKEY *a_nonce,
			    EVP_PKEY *e_prime_id);

struct dpp_authentication *
dpp_reconfig_init(void *msg_ctx, struct dpp_global *dpp, 
		  struct dpp_configurator *conf, struct dpp_bootstrap_info *own_bi,
		  unsigned int freq, u16 group,
		  const u8 *a_nonce_attr, size_t a_nonce_len,
		  const u8 *e_id_attr, size_t e_id_len);

struct wpabuf * dpp_reconfig_build_req(struct dpp_authentication *auth);
int dpp_parse_reconfig_annouce(struct wifi_app *wapp, const u8 *buf, size_t buf_len
#ifdef MAP_R3_RECONFIG
        , struct wapp_dev *wdev
#endif
);

EVP_PKEY * dpp_set_pubkey_point(EVP_PKEY *group_key,
				       const u8 *buf, size_t len);
void dpp_auth_fail(struct dpp_authentication *auth, const char *txt);
void dpp_debug_print_key(const char *title, EVP_PKEY *key);
EC_KEY * EVP_PKEY_get0_EC_KEY(EVP_PKEY *pkey);

char * wapp_dpp_get_scan_channel_list(struct wifi_app *wapp);

struct wpabuf * dpp_reconfig_build_resp(struct dpp_authentication *auth,
                                   const char *own_connector,
                                   struct wpabuf *conn_status);

#endif /* DPP_R2_RECONFIG */

#ifdef MAP_R3
void wdev_handle_dpp_sta_info(struct wifi_app *wapp,
       u32 ifindex,
       wapp_event_data *event_data);

void wdev_handle_dpp_uri_info(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void dpp_URI_1905_send(struct wifi_app *wapp, unsigned char *almac, struct wapp_dev *wdev,
		unsigned char *src_mac, unsigned short uri_len, unsigned char *rcvd_uri);

struct wpabuf *
dpp_build_conf_obj_bss(struct dpp_authentication *auth, enum dpp_config_type type,
		       struct dpp_configuration *conf);

int wapp_read_wts_map_config(struct wifi_app *wapp, char *name,
		struct set_config_bss_info bss_info[], unsigned char max_bss_num,
		unsigned char *config_bss_num);
void dpp_auth_fail_wrapper(struct wifi_app *wapp,const char * txt);
int wapp_dpp_ch_chirp_valid(struct wifi_app *wapp, u8 ch);
int dpp_parse_map_conf_obj(struct dpp_config *wdev_config,
			      struct wifi_app *wapp, const u8 *conf_obj, u16 conf_obj_len);
enum dpp_akm dpp_map_r3_akm_from_inter_val(unsigned short akm_val);
void wapp_version_mismatch(struct wifi_app *wapp);
#endif /* MAP_R3 */
void wapp_dpp_auth_timeout(void *eloop_ctx, void *timeout_ctx);
int dpp_direct_1905msg_auth_next(struct wifi_app *wapp, struct dpp_authentication *auth);
int dpp_bn2bin_pad(const BIGNUM *bn, u8 *pos, size_t len);
int dpp_hmac(size_t hash_len, const u8 *key, size_t key_len,
                    const u8 *data, size_t data_len, u8 *mac);
int dpp_hkdf_expand(size_t hash_len, const u8 *secret, size_t secret_len,
                           const char *label, u8 *out, size_t outlen);
EVP_PKEY * dpp_set_keypair(const struct dpp_curve_params **curve,
                                  const u8 *privkey, size_t privkey_len);
EVP_PKEY * dpp_gen_keypair(const struct dpp_curve_params *curve);
struct wpabuf * dpp_get_pubkey_point(EVP_PKEY *pkey, int prefix);
int dpp_build_jwk(struct wpabuf *buf, const char *name, EVP_PKEY *key,
                         const char *kid, const struct dpp_curve_params *curve);
char * dpp_sign_connector(struct dpp_configurator *conf,
                          const struct wpabuf *dppcon);
int dpp_prepare_channel_list(struct dpp_authentication *auth);
struct dpp_authentication *
dpp_alloc_auth(struct dpp_global *dpp, void *msg_ctx);
void dpp_build_attr_status(struct wpabuf *msg,
				  enum dpp_status_error status);
enum dpp_status_error
dpp_process_signed_connector(struct dpp_signed_connector_info *info, EVP_PKEY *csign_pub, const char *connector);

enum dpp_status_error
dpp_check_signed_connector(struct dpp_signed_connector_info *info,
                           const u8 *csign_key, size_t csign_key_len,
                           const u8 *peer_connector, size_t peer_connector_len);
int wapp_cancel_remain_on_channel(
        struct wifi_app *wapp, struct wapp_dev *wdev);
void wapp_dpp_reconfig_auth_confirm_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx);
struct json_token * dpp_parse_own_connector(const char *own_connector);
int dpp_get_reconfig_status(struct wifi_app *wapp, char *body, int body_len, char *evt_buf, int* len_buf);
#ifdef MAP_R3_RECONFIG
void dpp_auth_success(struct dpp_authentication *auth);
int dpp_get_reconfig_start(struct wifi_app *wapp, char *body, int body_len);
int dpp_get_reconfig_status(struct wifi_app *wapp, char *body, int body_len, char *evt_buf, int* len_buf);
#endif
#endif /* DPP_H */
