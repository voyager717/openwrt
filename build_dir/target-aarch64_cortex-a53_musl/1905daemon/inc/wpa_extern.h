#ifndef WPA_EXTERN_H
#define WPA_EXTERN_H

#include "list.h"
#include "defs.h"
#include "wpa_common.h"
#define ROLE_4_WAY_AUTHENTICATOR 1
#define ROLE_4_WAY_SUPPLICANT 2

#define EAPOL_MSG_INVALID      0
#define EAPOL_PAIR_MSG_1        1
#define EAPOL_PAIR_MSG_2        2
#define EAPOL_PAIR_MSG_3        3
#define EAPOL_PAIR_MSG_4        4
#define EAPOL_GROUP_MSG_1    5
#define EAPOL_GROUP_MSG_2    6


typedef enum {
	wpa_idle = 0,
	wpa_recv_pair_msg1,
	wpa_send_pair_msg1,
	wpa_send_pair_msg2,
	wpa_wait_pair_msg2,
	wpa_recv_pair_msg2,
	wpa_send_pair_msg3,
	wpa_wait_pair_msg3,
	wpa_recv_pair_msg3,
	wpa_send_pair_msg4,
	wpa_wait_pair_msg4,
	wpa_recv_pair_msg4,
	wpa_pair_suc,
	wpa_pair_fail,
	wpa_send_group_msg1,
	wpa_recv_group_msg1,
	wpa_send_group_msg2,
	wpa_wait_group_msg2,
	wpa_recv_group_msg2,
	wpa_group_suc,
	wpa_group_fail,
} hs_state;


/* n hours, now n = 1 */
#define PTK_REKEY_TIMER (1 * 60 * 60)
#define GTK_REKEY_TIMER (1 * 60 * 60)

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

struct GNU_PACKED wpa_auth_config {
	/* wpa = WPA_PROTO_WPA or WPA_PROTO_RSN */
	int wpa;
	/* key mgmt = WPA_KEY_MGMT_PSK */
	int wpa_key_mgmt;
	/* wpa_pairwise = WPA_CIPHER_CCMP; */
	int wpa_pairwise;
	/* rsn_pairwise = WPA_CIPHER_CCMP; */
	int rsn_pairwise;
	/* wpa_group = WPA_CIPHER_CCMP */
	int wpa_group;
	/* group rekey timer secs */
	int wpa_group_rekey;
	/* gmk rekey timer secs */
	int gmk_rekey_timer;
	/* gtk rekey timer secs */
	int gtk_rekey_timer;
	/* ptk rekey timer secs */
	int ptk_rekey_timer;
	/* max num for retry the gtk  */
	u32 max_gtk_rekey_count;
	/* max num for retry the ptk */
	u32 max_ptk_rekey_count;
	/* eapol frame type version */
	int eapol_version;
};

/* per group key state machine data */
struct GNU_PACKED wpa_group {
	u8 gtk_sm;
    u8 valid;
	Boolean GTKReKey;
	int GTK_len;
	int GN, GM;
	Boolean GTKAuthenticator;
	/* inc by 1 after send pkt */
	u8 Counter[WPA_NONCE_LEN];

	u8 GMK[WPA_GMK_LEN];
	u8 GTK[4][WPA_GTK_MAX_LEN];
	u8 GNonce[WPA_NONCE_LEN];
};

struct GNU_PACKED wpa_state_machine {
	struct wpa_auth_config wpa_conf;
	struct wpa_parameter *wpa_param;
	struct wpa_group *group;

	/* Controller or Agent */
	u8 role;

	/* sm state of PTK */
	u8 ptk_sm;
	/* sm state of GTK */
	u8 gtk_sm;

	/* WPA_PROTO_RSN or WPA_PROTO_WPA */
	u8 proto;

	/* own al mac */
	u8 own_addr[ETH_ALEN];

	/* peer al mac */
	u8 peer_addr[ETH_ALEN];

	/* time out cnt of ptk  */
	u32 ptk_retry_cnt;
	/* timeout cnt of GTK */
	u32 gtk_retry_cnt;
	/* MIC verivied */
	Boolean MICVerified;

	u8 ANonce[WPA_NONCE_LEN];
	u8 SNonce[WPA_NONCE_LEN];

	u8 PMK[PMK_LEN_MAX];
	unsigned int pmk_len;
	u8 pmkid[PMKID_LEN]; /* valid if pmkid_set == 1 */
	struct wpa_ptk PTK;
	/* dose ptk derived from pmk? */
	Boolean PTK_valid;
	/* pairwise key? */
	Boolean Pair;
	u8 ReplayCounter[WPA_REPLAY_COUNTER_LEN];

	u8 *last_rx_eapol_key; /* starting from IEEE 802.1X header */
	size_t last_rx_eapol_key_len;

	u8 RSC[WPA_KEY_RSC_LEN];

	/* wpa version */
	wpa_version wpa;
	/* the selected WPA_KEY_MGMT_* */
	int key_mgmt;
	/*WPA_CIPHER_TKIP or WPA_CIPHER_CCMP*/
	int pairwise_cipher;

	u32 dot11RSNAStatsTKIPLocalMICFailures;
	u32 dot11RSNAStatsTKIPRemoteMICFailures;
};


/* entry info to be stored in entry_list of authenticator or supplicants */
struct GNU_PACKED wpa_entry_info{
    struct dl_list list;

	struct wpa_auth_config *wpa_conf;

	/* own al mac */
	u8 own_addr[ETH_ALEN];

	/* peer almac address */
	u8 peer_addr[ETH_ALEN];

	u16 capability;
	u16 auth_alg;

	/* state machine per peer device */
	struct wpa_state_machine *wpa_sm;
};


/* per authenticator data */
struct GNU_PACKED wpa_authenticator {
	/* configuration of Authenticator*/
	struct wpa_auth_config conf;
	/* wpa group  */
	struct wpa_group *group;
	/* ALMAC of authenticator */
	unsigned char own_addr[ETH_ALEN];
	/* record the failures of 4wayhandshake */
	unsigned int dot11RSNA4WayHandshakeFailures;
	/* number of supplicants in sta_list */
	int num_entry;
	/* list of supplicants */
	struct wpa_entry_info *entry_list;
};


struct GNU_PACKED wpa_supplicant {
	struct wpa_auth_config conf;
	void *p1905_ctx;
	unsigned char own_addr[ETH_ALEN];

	/* wpa group  */
	struct wpa_group *group;

	/* number of entries in authenticator list */
	int num_entry;

	/* list of authenticators */
	struct wpa_entry_info *entry_list;
};

struct GNU_PACKED wpa_parameter {
	/* store confi for wpa, this config will apply to sm->wpa_conf */
	struct wpa_auth_config wpa_conf;

	/* number of entries in authenticator list */
	int num_entry;

	/* list of entry */
	struct dl_list entry_list;


	/* wpa_ie info of authenticator */
	u8 *wpa_ie;
	/* wpa_ie length of authenticator */
	size_t wpa_ie_len;

	/* point to struct p1905_managerd_ctx ctx */
	void *p1905_ctx;

	/* role: 0 - controller, 1 - agent */
	unsigned char role;

    struct wpa_group group;
};

void *wpa_init(void *ctx, unsigned char role);
int wpa_set_pmk_by_1905(u8 *own_addr, u8 *peer_addr, u8 *pmk, u16 pmk_len);
int wpa_receive_from_1905(u8 *peer_addr, u8 *data, size_t data_len);
void wpa_start_4way_handshake(void *eloop_ctx, void *timeout_ctx);
void wpa_deinit();
struct wpa_entry_info * wpa_get_entry(u8 *mac);
void resend_ptk_rekey_request_timer(void *eloop_ctx, void *timeout_ctx);
void resend_gtk_timer(void *eloop_ctx, void *timeout_ctx);
void wpa_gtk_update(struct wpa_group *group);
void wpa_set_gtk_rekey_interval(unsigned int interval);
void wpa_clear_entry_by_mac(u8 *mac);
void check_trigger_gtk_rekey();
void wpa_eloop_cancel_entry_timeout(u8 *mac);
void wpa_send_group_msg_1(struct wpa_state_machine *sm);

#endif
