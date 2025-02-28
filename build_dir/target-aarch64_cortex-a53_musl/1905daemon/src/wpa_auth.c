
#include "includes.h"
#include "os.h"
#include "eloop.h"
#include "p1905_managerd.h"
#include "common.h"
#include "wpa_extern.h"
#include "defs.h"
#include "wpa_common.h"
#include "byteorder.h"

#include "aes.h"
#include "aes_wrap.h"
#include "aes_siv.h"
#include "crypto.h"
#include "defs.h"
#include "sha1.h"
#include "sha256.h"
#include "sha384.h"
#include "sha512.h"
#include "random.h"

#include "wpa_auth.h"
#include "eapol_common.h"
#include "debug.h"
#include "p1905_ap_autoconfig.h"

u8 msg_buf[TX_EAPOL_BUFFER] = {0};
struct wpa_parameter wpa_global = {0};
#define HAND_SHAKE_PREFIX	"[4-way HS]"

#if 0
static inline int wpa_key_mgmt_suite_b(int akm)
{
	return !!(akm & (WPA_KEY_MGMT_IEEE8021X_SUITE_B |
			 WPA_KEY_MGMT_IEEE8021X_SUITE_B_192));
}


static inline int wpa_key_mgmt_sae(int akm)
{
	return !!(akm & (WPA_KEY_MGMT_SAE |
			 WPA_KEY_MGMT_FT_SAE));
}


static inline int wpa_key_mgmt_sha256(int akm)
{
	return !!(akm & (WPA_KEY_MGMT_PSK_SHA256 |
			 WPA_KEY_MGMT_IEEE8021X_SHA256 |
			 WPA_KEY_MGMT_SAE |
			 WPA_KEY_MGMT_FT_SAE |
			 WPA_KEY_MGMT_OSEN |
			 WPA_KEY_MGMT_IEEE8021X_SUITE_B |
			 WPA_KEY_MGMT_FILS_SHA256 |
			 WPA_KEY_MGMT_FT_FILS_SHA256));
}


static inline int wpa_key_mgmt_fils(int akm)
{
	return !!(akm & (WPA_KEY_MGMT_FILS_SHA256 |
			 WPA_KEY_MGMT_FILS_SHA384 |
			 WPA_KEY_MGMT_FT_FILS_SHA256 |
			 WPA_KEY_MGMT_FT_FILS_SHA384));
}
#endif


void wpa_sm(struct wpa_state_machine *sm);
void wpa_sm_step(void *eloop_ctx, void *timeout_ctx);
void wpa_send_group_msg_1(struct wpa_state_machine *sm);
extern void resend_ptk_rekey_request_timer(void *eloop_ctx, void *timeout_ctx);


void check_trigger_gtk_rekey()
{
	struct wpa_parameter *wpa_param = &wpa_global;
	struct wpa_entry_info *entry = NULL, *entry_next = NULL;
	struct p1905_managerd_ctx *ctx = NULL;

	if (!wpa_param->p1905_ctx) {
		debug(DEBUG_ERROR, "p1905_ctx null pointer\n");
		return;
	}
	ctx = (struct p1905_managerd_ctx *)wpa_param->p1905_ctx;

	/* GTK only can be update @ init or rekey stage */
	if (wpa_param->role == CONTROLLER && (!ctx->MAP_Cer)) {
		wpa_gtk_update(&wpa_param->group);

		dl_list_for_each_safe(entry, entry_next, &wpa_param->entry_list, struct wpa_entry_info, list) {
			if (entry->wpa_sm) {
				debug(DEBUG_ERROR, "resend gtk rekey request to "MACSTR"\n", MAC2STR(entry->peer_addr));
				wpa_send_group_msg_1(entry->wpa_sm);
			}
		}
	}

	return;
}


void wpa_set_gtk_rekey_interval(unsigned int interval)
{
	wpa_global.wpa_conf.gtk_rekey_timer = interval;
	debug(DEBUG_ERROR, "set gtk rekey interval %d(s)\n",interval);
}

char *wpa_get_eapol_msg_type(u8 msg)
{
	if (msg == EAPOL_PAIR_MSG_1)
		return "Pairwise Message 1";
	else if (msg == EAPOL_PAIR_MSG_2)
		return "Pairwise Message 2";
	else if (msg == EAPOL_PAIR_MSG_3)
		return "Pairwise Message 3";
	else if (msg == EAPOL_PAIR_MSG_4)
		return "Pairwise Message 4";
	else if (msg == EAPOL_GROUP_MSG_1)
		return "Group Message 1";
	else if (msg == EAPOL_GROUP_MSG_2)
		return "Group Message 2";
	else
		return "Invalid Message";
}


int wpa_use_akm_defined(int akmp)
{
	return akmp == WPA_KEY_MGMT_OSEN ||
		akmp == WPA_KEY_MGMT_OWE ||
		akmp == WPA_KEY_MGMT_DPP ||
		akmp == WPA_KEY_MGMT_FT_IEEE8021X_SHA384 ||
		wpa_key_mgmt_sae(akmp) ||
		wpa_key_mgmt_suite_b(akmp) ||
		wpa_key_mgmt_fils(akmp);
}

int wpa_use_aes_key_wrap(int akmp)
{
	return akmp == WPA_KEY_MGMT_OSEN ||
		akmp == WPA_KEY_MGMT_OWE ||
		akmp == WPA_KEY_MGMT_DPP ||
		wpa_key_mgmt_sha256(akmp) ||
		wpa_key_mgmt_sae(akmp) ||
		wpa_key_mgmt_suite_b(akmp);
}




int wpa_cipher_key_len(int cipher)
{
	switch (cipher) {
	case WPA_CIPHER_CCMP_256:
	case WPA_CIPHER_GCMP_256:
	case WPA_CIPHER_BIP_GMAC_256:
	case WPA_CIPHER_BIP_CMAC_256:
		return 32;
	case WPA_CIPHER_CCMP:
	case WPA_CIPHER_GCMP:
	case WPA_CIPHER_AES_128_CMAC:
	case WPA_CIPHER_BIP_GMAC_128:
		return 16;
	case WPA_CIPHER_TKIP:
		return 32;
	}

	return 0;
}

unsigned int wpa_mic_len(int akmp, size_t pmk_len)
{
	switch (akmp) {
	case WPA_KEY_MGMT_IEEE8021X_SUITE_B_192:
	case WPA_KEY_MGMT_FT_IEEE8021X_SHA384:
		return 24;
	case WPA_KEY_MGMT_FILS_SHA256:
	case WPA_KEY_MGMT_FILS_SHA384:
	case WPA_KEY_MGMT_FT_FILS_SHA256:
	case WPA_KEY_MGMT_FT_FILS_SHA384:
		return 0;
	case WPA_KEY_MGMT_DPP:
		return pmk_len / 2;
	case WPA_KEY_MGMT_OWE:
		return pmk_len / 2;
	default:
		return 16;
	}
}


struct wpa_entry_info * wpa_get_entry(u8 *mac)
{
	struct wpa_entry_info *entry = NULL, *entry_next = NULL;
	struct wpa_parameter *wpa_ctx = &wpa_global;

	debug(DEBUG_TRACE,"look for entry"MACSTR"\n", MAC2STR(mac));
	dl_list_for_each_safe(entry, entry_next, &wpa_ctx->entry_list, struct wpa_entry_info, list) {
		debug(DEBUG_TRACE,"entry "MACSTR" in list\n", MAC2STR(entry->peer_addr));
		if (os_memcmp(entry->peer_addr, mac, ETH_ALEN) == 0)
		return entry;
	}

	return NULL;
}

void wpa_clear_entry_by_mac(u8 *mac)
{
	struct wpa_parameter *wpa_param = &wpa_global;
	struct wpa_entry_info *entry = NULL, *entry_next = NULL;

	eloop_cancel_timeout(check_trigger_gtk_rekey, NULL, NULL);
	eloop_register_timeout(5, 0, check_trigger_gtk_rekey, NULL, NULL);

	dl_list_for_each_safe(entry, entry_next, &wpa_param->entry_list, struct wpa_entry_info, list) {
		if (!os_memcmp(entry->peer_addr, mac, ETH_ALEN)) {
			wpa_eloop_cancel_entry_timeout(mac);
			if (entry->wpa_sm) {
				/* free group buf of sm */
				if (entry->wpa_sm->group) {
					os_free(entry->wpa_sm->group);
				}
				/* free sm for each entry */
				os_free(entry->wpa_sm);
			}
			debug(DEBUG_ERROR,"clear wpa entry "MACSTR"\n", MAC2STR(entry->peer_addr));
			dl_list_del(&entry->list);
			os_free(entry);
			break;
		}
	}

	return;
}


/* controller will update GTK in init/rekey stage*/
void wpa_gtk_update(struct wpa_group *group)
{
	random_get_bytes(group->GTK[group->GN - 1], WPA_GTK_MAX_LEN);
	group->valid = 1;
	hex_dump_all("Controller update GTK",group->GTK[group->GN - 1], group->GTK_len);

	return;
}

static struct wpa_group * wpa_group_init_global(struct wpa_group *group, int group_cipher)
{
	group->GTKAuthenticator = TRUE;
	group->GTK_len = wpa_cipher_key_len(group_cipher);

	if (random_pool_ready() != 1) {
		debug(DEBUG_ERROR, "WPA: Not enough entropy in random pool "
			   "for secure operations - update keys later when "
			   "the first station connects\n");
	}

	group->gtk_sm = wpa_idle;

	/* default gtk index */
	group->GN = 1;

	wpa_gtk_update(group);

	return group;
}

static struct wpa_group * wpa_group_init_peer(struct wpa_state_machine *sm)
{
	struct wpa_group *group = NULL;

	group = os_zalloc(sizeof(struct wpa_group));
	if (group == NULL)
		return NULL;

	sm->group = group;

	if (random_pool_ready() != 1) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"WPA: Not enough entropy in random pool "
			   "for secure operations - update keys later when "
			   "the first station connects\n");
	}


	return group;
}



struct wpa_state_machine *wpa_sm_init(struct wpa_entry_info *entry, u8 *own_addr, u8 *peer_addr)
{
	struct wpa_state_machine *sm = NULL;

	struct wpa_parameter *wpa_ctx = &wpa_global;

	sm = os_zalloc(sizeof(struct wpa_state_machine));
	if (sm == NULL) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"Error os_zalloc wpa_state_machine.\n");
		return NULL;
	}
	os_memcpy(sm->own_addr, own_addr, ETH_ALEN);
	os_memcpy(sm->peer_addr, peer_addr, ETH_ALEN);
	os_memcpy(&sm->wpa_conf, &wpa_ctx->wpa_conf, sizeof(struct wpa_auth_config));

	if (NULL == wpa_group_init_peer(sm)) {
		debug(DEBUG_ERROR, "sm group init failed\n");
		os_free(sm);
		return NULL;
	}

	/* wpa_key_mgmt WPA_KEY_MGMT_PSK */
	sm->key_mgmt = wpa_ctx->wpa_conf.wpa_key_mgmt;

	/* wpa WPA_VERSION_WPA2 */
	sm->wpa = wpa_ctx->wpa_conf.wpa;

	/* pairwise_cipher WPA_CIPHER_CCMP */
	sm->pairwise_cipher = wpa_ctx->wpa_conf.rsn_pairwise;

	/* point back to the wpa_global */
	sm->wpa_param = wpa_ctx;

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"init sm for "MACSTR" success.\n", MAC2STR(peer_addr));

	return sm;
}



struct wpa_entry_info * wpa_entry_add(u8 *own_addr, u8 *peer_addr)
{
	struct wpa_entry_info *entry = NULL;
	struct wpa_state_machine *sm = NULL;

	struct wpa_parameter *wpa_ctx = &wpa_global;

	entry = wpa_get_entry(peer_addr);

	if (entry) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX" found a existed entry\n");
		return entry;
	}

	if (wpa_ctx->num_entry >= WPA_MAX_ENTRY_NUM) {
		/* FIX: might try to remove some old STAs first? */
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"no more room for new entry (%d/%d)\n",
			   wpa_ctx->num_entry, WPA_MAX_ENTRY_NUM);
		return NULL;
	}


	entry = os_zalloc(sizeof(struct wpa_entry_info));
	if (entry == NULL) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"malloc entry failed\n");
		return NULL;
	}

	os_memcpy(entry->own_addr, own_addr, ETH_ALEN);
	os_memcpy(entry->peer_addr, peer_addr, ETH_ALEN);

	if (NULL == (sm = wpa_sm_init(entry, own_addr, peer_addr))) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"wpa sm init failed\n");
		os_free(entry);
		return NULL;
	}

	entry->wpa_sm = sm;
	entry->wpa_sm->wpa_param = wpa_ctx;
	entry->wpa_sm->role = wpa_ctx->role;

	dl_list_add(&wpa_ctx->entry_list, &entry->list);
	wpa_ctx->num_entry++;

	return entry;
}


/*
** role_4way	-	role of peer_addr
** own_addr	-	own al mac
** peer_addr	-	peer al mac
** pmk
** pmk_len
*/
int wpa_set_pmk_by_1905(u8 *own_addr, u8 *peer_addr, u8 *pmk, u16 pmk_len)
{
	struct wpa_entry_info * entry = NULL;
	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"set PMK for "MACSTR"\n", MAC2STR(peer_addr));

	if (NULL == (entry = wpa_entry_add(own_addr, peer_addr))) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"wpa entry add failed.\n");
		return -1;
	}

	os_memcpy(entry->wpa_sm->PMK, pmk, pmk_len);
	entry->wpa_sm->pmk_len = pmk_len;

	/* PMK update, repley counter should be reset to 0 */
	os_memset(entry->wpa_sm->ReplayCounter, 0, WPA_REPLAY_COUNTER_LEN);

	return 0;
}



int wpa_eapol_key_mic(const u8 *key, size_t key_len,
		const u8 *buf, size_t len, u8 *mic)
{
	u8 hash[SHA512_MAC_LEN];

	if (key_len == 0) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"WPA: KCK not set - cannot calculate MIC\n");
		return -1;
	}

	/* 256-bit MIC using the HMAC-SHA256 */
	if (hmac_sha256(key, key_len, buf, len, hash)) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"WPA: Khmac_sha256 failed\n");
		return -1;
	}

	os_memcpy(mic, hash, key_len);

	return 0;
}


static unsigned int wpa_kck_len(int akmp, size_t pmk_len)
{
	switch (akmp) {
	case WPA_KEY_MGMT_IEEE8021X_SUITE_B_192:
	case WPA_KEY_MGMT_FT_IEEE8021X_SHA384:
		return 24;
	case WPA_KEY_MGMT_FILS_SHA256:
	case WPA_KEY_MGMT_FT_FILS_SHA256:
	case WPA_KEY_MGMT_FILS_SHA384:
	case WPA_KEY_MGMT_FT_FILS_SHA384:
		return 0;
	case WPA_KEY_MGMT_DPP:
		return pmk_len / 2;
	case WPA_KEY_MGMT_OWE:
		return pmk_len / 2;
	default:
		return 16;
	}
}

static unsigned int wpa_kek_len(int akmp, size_t pmk_len)
{
	switch (akmp) {
	case WPA_KEY_MGMT_FILS_SHA384:
	case WPA_KEY_MGMT_FT_FILS_SHA384:
		return 64;
	case WPA_KEY_MGMT_IEEE8021X_SUITE_B_192:
	case WPA_KEY_MGMT_FILS_SHA256:
	case WPA_KEY_MGMT_FT_FILS_SHA256:
	case WPA_KEY_MGMT_FT_IEEE8021X_SHA384:
		return 32;
	case WPA_KEY_MGMT_DPP:
		return pmk_len <= 32 ? 16 : 32;
	case WPA_KEY_MGMT_OWE:
		return pmk_len <= 32 ? 16 : 32;
	default:
		return 16;
	}
}



/**
 * wpa_pmk_to_ptk - Calculate PTK from PMK, addresses, and nonces
 * @pmk: Pairwise master key
 * @pmk_len: Length of PMK
 * @label: Label to use in derivation
 * @addr1: AA or SA
 * @addr2: SA or AA
 * @nonce1: ANonce or SNonce
 * @nonce2: SNonce or ANonce
 * @ptk: Buffer for pairwise transient key
 * @akmp: Negotiated AKM
 * @cipher: Negotiated pairwise cipher
 * Returns: 0 on success, -1 on failure
 *
 * IEEE Std 802.11i-2004 - 8.5.1.2 Pairwise key hierarchy
 * PTK = PRF-X(PMK, "Pairwise key expansion",
 *             Min(AA, SA) || Max(AA, SA) ||
 *             Min(ANonce, SNonce) || Max(ANonce, SNonce))
 */
int wpa_pmk_to_ptk(const u8 *pmk, size_t pmk_len, const char *label,
		   const u8 *addr1, const u8 *addr2,
		   const u8 *nonce1, const u8 *nonce2,
		   struct wpa_ptk *ptk, int akmp, int cipher)
{
	u8 data[2 * ETH_ALEN + 2 * WPA_NONCE_LEN];
	u8 tmp[WPA_KCK_MAX_LEN + WPA_KEK_MAX_LEN + WPA_TK_MAX_LEN];
	size_t ptk_len;

	if (pmk_len == 0) {
		debug(DEBUG_ERROR, "WPA: No PMK set for PTK derivation\n");
		return -1;
	}

	debug(DEBUG_ERROR, "%s \n", __func__);

	if (os_memcmp(addr1, addr2, ETH_ALEN) < 0) {
		os_memcpy(data, addr1, ETH_ALEN);
		os_memcpy(data + ETH_ALEN, addr2, ETH_ALEN);
	} else {
		os_memcpy(data, addr2, ETH_ALEN);
		os_memcpy(data + ETH_ALEN, addr1, ETH_ALEN);
	}

	if (os_memcmp(nonce1, nonce2, WPA_NONCE_LEN) < 0) {
		os_memcpy(data + 2 * ETH_ALEN, nonce1, WPA_NONCE_LEN);
		os_memcpy(data + 2 * ETH_ALEN + WPA_NONCE_LEN, nonce2,
			  WPA_NONCE_LEN);
	} else {
		os_memcpy(data + 2 * ETH_ALEN, nonce2, WPA_NONCE_LEN);
		os_memcpy(data + 2 * ETH_ALEN + WPA_NONCE_LEN, nonce1,
			  WPA_NONCE_LEN);
	}

	ptk->kck_len = wpa_kck_len(akmp, pmk_len);
	ptk->kek_len = wpa_kek_len(akmp, pmk_len);
	ptk->tk_len = wpa_cipher_key_len(cipher);
	if (ptk->tk_len == 0) {
		debug(DEBUG_ERROR,
			   "WPA: Unsupported cipher (0x%x) used in PTK derivation\n",
			   cipher);
		return -1;
	}
	ptk_len = ptk->kck_len + ptk->kek_len + ptk->tk_len;

	debug(DEBUG_ERROR, "WPA: PTK derivation using KDF-SHA-%d-%d\n",
		(int)(pmk_len*8), (int)(ptk_len*8));
	/* generate a 512 bit 1905 PTK using KDF-SHA-256-512(K, A, B)
		and a resulting 256-bit 1905 TK  */
	if (sha256_prf(pmk, pmk_len, label, data, sizeof(data), tmp, ptk_len) < 0)
		return -1;

	debug(DEBUG_ERROR, "WPA: PTK derivation - A1=" MACSTR " A2=" MACSTR"\n",
		   MAC2STR(addr1), MAC2STR(addr2));
	debug(DEBUG_ERROR, "WPA: ADDR1 "MACSTR"\n", MAC2STR(addr1));
	debug(DEBUG_ERROR, "WPA: ADDR2 "MACSTR"\n", MAC2STR(addr2));
	debug(DEBUG_ERROR, "WPA: PTK derivation using PRF(SHA1)\n");
	hex_dump_info("WPA: Nonce1", (u8 *)nonce1, WPA_NONCE_LEN);
	hex_dump_info("WPA: Nonce2", (u8 *)nonce2, WPA_NONCE_LEN);
	hex_dump_info("WPA: PMK", (u8 *)pmk, pmk_len);
	hex_dump_info("WPA: PTK", tmp, ptk_len);

	os_memcpy(ptk->kck, tmp, ptk->kck_len);
	hex_dump_info("WPA: KCK", ptk->kck, ptk->kck_len);

	os_memcpy(ptk->kek, tmp + ptk->kck_len, ptk->kek_len);
	hex_dump_info("WPA: KEK", ptk->kek, ptk->kek_len);

	os_memcpy(ptk->tk, tmp + ptk->kck_len + ptk->kek_len, ptk->tk_len);
	hex_dump_info("WPA: TK", ptk->tk, ptk->tk_len);

	ptk->kek2_len = 0;
	ptk->kck2_len = 0;

	os_memset(tmp, 0, sizeof(tmp));
	return 0;
}






/* kde maybe include RSNE, GTK, etc. */
static u8 *wpa_add_kde(u8 *pos, u32 kde, u8 *data, size_t data_len,
				u8 *data2, size_t data2_len)
{
	u8 * tmp = pos;
	/* Type (0xdd) */
	*pos++ = WLAN_EID_VENDOR_SPECIFIC;
	/* Length */
	*pos++ = RSN_SELECTOR_LEN + data_len + data2_len;
	/* OUI and DataType */
	RSN_SELECTOR_PUT(pos, kde);
	pos += RSN_SELECTOR_LEN;
	/* Key ID */
	os_memcpy(pos, data, data_len);
	pos += data_len;
	/* Multi-AP GTK */
	if (data2) {
		os_memcpy(pos, data2, data2_len);
		pos += data2_len;
	}

	hex_dump_info("added kde", tmp, pos - tmp);
	return pos;
}

/* frame: point to the eapol key
** dataLen: key data len
*/
void wpa_construct_eapol_key_data(
	struct wpa_state_machine *sm,
	u8 MsgType,
	int key_info,
	u8 *frame,
	u16 *dataLen)
{
	struct ieee802_1x_hdr *_1x_hdr = NULL;
	struct wpa_eapol_key *key = NULL;
	Boolean bWPA2 = TRUE;
	size_t len = 0, mic_len = 0, keyhdrlen = 0;
	u8 *wpa_ie = NULL, *gtk = NULL, *pos = NULL;
	u8 kde[MAX_LEN_OF_RSNIE], buf[MAX_LEN_OF_RSNIE];
	int key_data_len = 0, pad_len = 0;
	int wpa_ie_len = 0, gtk_kde_len = 0, gtk_len = 0;
	u8 keyidx = 0;
	u8 *key_mic = NULL, *key_data = NULL;
	u8 *key_data_len_ptr = NULL;

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX" %s\n", wpa_get_eapol_msg_type(MsgType));
	os_memset(kde, 0, sizeof(kde));
	os_memset(buf, 0, sizeof(buf));
	mic_len = wpa_mic_len(sm->key_mgmt, sm->pmk_len);

	_1x_hdr = (struct ieee802_1x_hdr *)frame;
	key = (struct wpa_eapol_key *)(_1x_hdr + 1);
	keyhdrlen = sizeof(struct wpa_eapol_key) + mic_len + 2;
	len = sizeof(struct ieee802_1x_hdr) + keyhdrlen;

	key_mic = (u8 *)(key + 1);
	key_data = ((u8 *)(_1x_hdr + 1)) + keyhdrlen;
	key_data_len_ptr = key_mic + mic_len;

	if (MsgType == EAPOL_PAIR_MSG_1
		|| MsgType == EAPOL_PAIR_MSG_4
		|| MsgType == EAPOL_GROUP_MSG_2)
		return;

	/* Choose WPA2 or not*/
	if (sm->wpa == WPA_VERSION_WPA)
		bWPA2 = FALSE;

	/* Encapsulate WPAIE/RSNIE in pairwise_msg2 & pairwise_msg3 */
	/* but,should not include RSN/WPA IE in 1905 4way handshake */
	if ((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3)) {
		wpa_ie = sm->wpa_param->wpa_ie;
		wpa_ie_len = sm->wpa_param->wpa_ie_len;
		pos = key_data;
		//hex_dump_all("wpa ie", wpa_ie, wpa_ie_len);

		key_data_len += wpa_ie_len;

		os_memcpy(kde, wpa_ie, wpa_ie_len);
	}

	/* Encapsulate GTK */
	/* Only for pairwise_msg3_WPA2 and group_msg1*/
	if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1)) {
		/* use global GTK,and share with other Agents */
		keyidx = sm->wpa_param->group.GN;
		gtk = sm->wpa_param->group.GTK[keyidx - 1];

		/* spec says gtk len of multi-ap is 32 bytes */
		gtk_len = WPA_GTK_MAX_LEN;


		/*	KDE format
			Type (0xdd) Length OUI Data-Type Data
		Octets:   1       1     3     1      (Length ¨C 4)


		*/
		/*	Multi-AP GTK KDE Data
			OUI : 0x50-6F-9A
			Data-Type: 0


			Data

			Key ID		Reserved	Multi-AP GTK
			Bits 0-1	Bits 2 - 7	32 octets

		*/
		if (sm->role == 0) {
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"Controller append GTK for "MACSTR"\n", MAC2STR(sm->peer_addr));

			//hex_dump_all("GTK", gtk, gtk_len);
			keyidx = (keyidx & 0x03);

			gtk_kde_len = 2 + RSN_SELECTOR_LEN + 1 + gtk_len;
			key_data_len += gtk_kde_len;

			if (key_data_len > sizeof(kde)) {
				debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"key_data_len %d over flow\n", key_data_len);
				return;
			}

			pos = kde + wpa_ie_len;

			/* add group key in pairwise msg3/4 and group msg1/2 */
			pos = wpa_add_kde(pos, RSN_MULTI_AP_GTK_KDE, &keyidx, 1, gtk, gtk_len);
		}

		/* AKM DPP */
		if (1/*key_info & WPA_KEY_INFO_TYPE_HMAC_SHA1_AES*/) {
			/* cac pad len */
			pad_len = key_data_len % 8;

			if (pad_len) {
				pad_len = 8 - pad_len;
			}
			key_data_len += pad_len + 8;
		}
	}

	if (key_data_len && (key_info & WPA_KEY_INFO_ENCR_KEY_DATA)) {
		/* The padding consists of appending */
		/* a single octet 0xdd followed by zero or more 0x00 octets */
		pos = buf;

		os_memcpy(pos, kde, wpa_ie_len + gtk_kde_len);
		pos += (wpa_ie_len + gtk_kde_len);

		if (pad_len)
			*pos++ = 0xdd;

		hex_dump_info("Plaintext EAPOL-Key Key Data", buf, key_data_len);
		if (key_info & WPA_KEY_INFO_ENCR_KEY_DATA) {
			if (aes_wrap(sm->PTK.kek, sm->PTK.kek_len,
					(key_data_len - 8) / 8, buf, key_data)) {
				debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"RSNE: aes_wrap failed\n");
				return;
			}

			//hex_dump_all("encypted EAPOL-Key Key Data", key_data, key_data_len);

			/* Update key data length field and total body length*/
			WPA_PUT_BE16(key_data_len_ptr, key_data_len);
		}
		else if (sm->PTK.kek_len == 16) {
			u8 ek[32];

			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"WPA: Encrypt Key Data using RC4\n");
			os_memcpy(key->key_iv,
				sm->group->Counter + WPA_NONCE_LEN - 16, 16);
			inc_byte_array(sm->group->Counter, WPA_NONCE_LEN);
			os_memcpy(ek, key->key_iv, 16);
			os_memcpy(ek + 16, sm->PTK.kek, sm->PTK.kek_len);
			os_memcpy(key_data, buf, key_data_len);
			rc4_skip(ek, 32, 256, key_data, key_data_len);

			/* Update key data length field and total body length*/
			WPA_PUT_BE16(key_data_len_ptr, key_data_len);
		}
		else {
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"don't know how to encrypt key data\n");
			return;
		}
		WPA_PUT_BE16(key_data_len_ptr, key_data_len);
	}
	else if (wpa_ie) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"only copy wpa ie to key data for %s\n",
			wpa_get_eapol_msg_type(MsgType));
		os_memcpy(key_data, wpa_ie, wpa_ie_len);
		WPA_PUT_BE16(key_data_len_ptr, wpa_ie_len);
	}
	else {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"no key data for msg %s\n",
			wpa_get_eapol_msg_type(MsgType));
	}

	if (!mic_len && gtk) {
		len += AES_BLOCK_SIZE;
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"len(%d) plus AES_BLOCK_SIZE(16)\n", (int)len);
	}

	/* length of key data */
	*dataLen = key_data_len;
}



/*
** buf: point to ieee802_1x_hdr
** len: (out)send buf total len
*/
void wpa_construct_eapol_msg(
	struct wpa_state_machine *sm,
	u8 MsgType,
	u8 *buf,
	u16 *len)
{
	Boolean bWPA2 = TRUE;
	int key_info = 0, pairwise, alg;
	/* data len of wpa_eapol_key */
	u16 dataLen = 0;
	/* key_length of wpa_eapol_key */
	u32 key_len = MIN_LEN_OF_EAPOL_KEY_MSG;
	struct ieee802_1x_hdr *hdr;
	struct wpa_eapol_key *key;
	u8 *key_mic = NULL;

	hdr = (struct ieee802_1x_hdr *)buf;
	key = (struct wpa_eapol_key *)(hdr + 1);
	key_mic = (u8 *)(key + 1);

	if (sm->wpa == WPA_VERSION_WPA)
		bWPA2 = FALSE;

	hdr->version = EAPOL_VERSION;
	hdr->type = IEEE802_1X_TYPE_EAPOL_KEY;

	WPA_PUT_BE16((u8 *)&hdr->length, key_len);

	/* key type: RSN or WPA */
	if (bWPA2) {
		key->type = EAPOL_KEY_TYPE_RSN;

		/* Key Descriptor Version should be set to 00 */
		//key_info |= WPA_KEY_INFO_TYPE_HMAC_SHA1_AES;
	}
	else {
		key->type = EAPOL_KEY_TYPE_WPA;

		key_info |= WPA_KEY_INFO_TYPE_HMAC_MD5_RC4;
	}

	/**/

	/* cipher: TKIP or CCMP
	if (sm->pairwise_cipher == WPA_CIPHER_TKIP)
		key_info |= WPA_CIPHER_TKIP;
	else
		key_info |= WPA_CIPHER_CCMP;*/

	/* Specify Key Type as Group(0) or Pairwise(1)*/
	if (MsgType <= EAPOL_PAIR_MSG_4)
		key_info |= WPA_KEY_INFO_KEY_TYPE;

	/* Specify Key Index, only group_msg1_WPA1*/
	//TODO
	if (!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1))
		;//key_info |= WPA_KEY_INFO_KEY_TYPE;

	/* INSTALL bit */
	if (MsgType == EAPOL_PAIR_MSG_3)
		key_info |= WPA_KEY_INFO_INSTALL;

	/* ACK bit */
	if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3) || (MsgType == EAPOL_GROUP_MSG_1))
		key_info |= WPA_KEY_INFO_ACK;


	if (MsgType != EAPOL_PAIR_MSG_1)
		key_info |= WPA_KEY_INFO_MIC;

	if ((bWPA2 && (MsgType >= EAPOL_PAIR_MSG_3)) ||
		(!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1)))
		key_info |= WPA_KEY_INFO_SECURE;


	/* This subfield shall be set, and the Key Data field shall be encrypted, if
	   any key material (e.g., GTK or SMK) is included in the frame. */
	if (bWPA2 && ((MsgType == EAPOL_PAIR_MSG_3)
				  || (MsgType == EAPOL_GROUP_MSG_1)))
		key_info |= WPA_KEY_INFO_ENCR_KEY_DATA;

	WPA_PUT_BE16(key->key_info, key_info);

	pairwise = !!(key_info & WPA_KEY_INFO_KEY_TYPE);
	alg = pairwise ? sm->pairwise_cipher : WPA_CIPHER_CCMP;

	/* Fill in Key Length*/
	if (sm->wpa == WPA_VERSION_WPA2 && !pairwise)
		WPA_PUT_BE16(key->key_length, 0);
	else
		WPA_PUT_BE16(key->key_length, wpa_cipher_key_len(alg));

	/* Fill in replay counter */
	/* replay counter add by 1 in msg1/4 msg3/4 msg2/2 */
	os_memcpy(key->replay_counter, sm->ReplayCounter, WPA_REPLAY_COUNTER_LEN);

	/* Fill Key Nonce field   */
	/* ANonce : pairwise_msg1 & pairwise_msg3*/
	/* SNonce : pairwise_msg2*/
	/* GNonce : group_msg1_wpa1 */
	if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3))
		os_memcpy(key->key_nonce, sm->ANonce, WPA_NONCE_LEN);
	else if (MsgType == EAPOL_PAIR_MSG_2)
		os_memcpy(key->key_nonce, sm->SNonce, WPA_NONCE_LEN);
	else if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1)) {
		os_memcpy(key->key_nonce, sm->group->GNonce, WPA_NONCE_LEN);
		/* Fill key IV - WPA2 as 0, WPA1 as random*/
		/* Suggest IV be random number plus some number,*/
		os_memcpy(key->key_iv, &sm->group->GNonce[16], WPA_KEY_IV_LEN);
		key->key_iv[15] += 2;
	}

	/* Fill Key RSC field		 */
	/* where is the rsc from???? look into the implementation in driver.*/
	if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1))
		os_memcpy(key->key_rsc, sm->RSC, WPA_KEY_RSC_LEN);

	wpa_construct_eapol_key_data(sm, MsgType, key_info, buf, &dataLen);

	/* eapol pkt total len */
	*len = key_len + sizeof(struct ieee802_1x_hdr) + dataLen;

	/* update 1x hdr len */
	WPA_PUT_BE16((u8 *)&hdr->length, key_len + dataLen);

	/* cac mic: compute a 256-bit MIC using the HMAC-SHA256 */
	if (key_info & WPA_KEY_INFO_MIC) {
		/* MIC include 1xhdr, eapol hdr, key data */
		if (wpa_eapol_key_mic(sm->PTK.kck, sm->PTK.kck_len,
					buf, *len, key_mic) < 0) {
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"calc MIC failed\n");
			return;
		}
	}

	//hex_dump_all("8021x hdr", (u8 *)buf, *len);

}

/**
 * wpa_parse_generic - Parse EAPOL-Key Key Data Generic IEs
 * @pos: Pointer to the IE header
 * @end: Pointer to the end of the Key Data buffer
 * @ie: Pointer to parsed IE data
 * Returns: 0 on success, 1 if end mark is found, -1 on failure
 */
static int wpa_parse_generic(const u8 *pos, const u8 *end,
				struct wpa_eapol_ie_parse *ie)
{
	if (pos[1] == 0)
		return 1;

	/* KDE IE format
	** Type (0xdd) Length OUI DataType Data
	**  1            1     3    1      len-4
	*/

	/* WPA IE */
	if (pos[1] >= 6 &&
		RSN_SELECTOR_GET(pos + 2) == WPA_OUI_TYPE &&
		pos[2 + WPA_SELECTOR_LEN] == 1 &&
		pos[2 + WPA_SELECTOR_LEN + 1] == 0) {
		ie->wpa_ie = pos;
		ie->wpa_ie_len = pos[1] + 2;
		return 0;
	}


	/* PMKID IE */
	if (1 + RSN_SELECTOR_LEN < end - pos &&
		pos[1] >= RSN_SELECTOR_LEN + PMKID_LEN &&
		RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_PMKID) {
		ie->pmkid = pos + 2 + RSN_SELECTOR_LEN;
		return 0;
	}

	/* GTK IE format is the data of KDE ie*/
	/* KeyID      Tx     Reserved  Reserved GTK
	**   bit0-1  bit2    bit 3-7   1        len-6
	*/
	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
		RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_GROUPKEY) {
		ie->gtk = pos + 2 + RSN_SELECTOR_LEN;
		ie->gtk_len = pos[1] - RSN_SELECTOR_LEN;
		return 0;
	}

	/* KDE for Multi-AP */
	/* 	OUI		Type	Meaning
		0x50-6F-9A	0	Multi-AP GTK KDE
	*/
	/*	data
		Key ID		Reserved	Multi-AP GTK
		Bits 0-1	Bits 2 - 7	32 octets
	*/
	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
		RSN_SELECTOR_GET(pos + 2) == RSN_MULTI_AP_GTK_KDE) {
		ie->GN = pos + 2 + RSN_SELECTOR_LEN;
		ie->gtk = pos + 2 + RSN_SELECTOR_LEN + 1;
		ie->gtk_len = pos[1] - RSN_SELECTOR_LEN - 1;

		//debug(DEBUG_ERROR, " MultiAP GTK KDE keyindex %x\n", ((*(ie->GN)  & 0xC0) >> 6));
		//hex_dump_all("WPA: GTK in EAPOL-Key", (u8 *)pos, pos[1] + 2);
		return 0;
	}


	/* MAC Address IE */
	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
		RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_MAC_ADDR) {
		ie->mac_addr = pos + 2 + RSN_SELECTOR_LEN;
		ie->mac_addr_len = pos[1] - RSN_SELECTOR_LEN;
		return 0;
	}
	return 0;
}



/**
 * wpa_parse_kde_ies - Parse EAPOL-Key Key Data IEs
 * @buf: Pointer to the Key Data buffer
 * @len: Key Data Length
 * @ie: Pointer to parsed IE data
 * Returns: 0 on success, -1 on failure
 */
int wpa_parse_kde_ies(const u8 *buf, size_t len, struct wpa_eapol_ie_parse *ie)
{
	const u8 *pos, *end;
	int ret = 0;

	//hex_dump_all("parse received kde", (u8 *)buf, len);


	os_memset(ie, 0, sizeof(*ie));
	for (pos = buf, end = pos + len; end - pos > 1; pos += 2 + pos[1]) {
		if (pos[0] == 0xdd &&
			((pos == buf + len - 1) || pos[1] == 0)) {
			/* Ignore padding */
			break;
		}
		if (2 + pos[1] > end - pos) {
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"WPA: EAPOL-Key Key Data "
				   "overflow (ie=%d len=%d pos=%d)\n",
				   pos[0], pos[1], (int) (pos - buf));
			hex_dump_info("WPA: Key Data",
					(u8 *)buf, len);
			ret = -1;
			break;
		}

		debug(DEBUG_TRACE, HAND_SHAKE_PREFIX"WPA: EAPOL-Key Key Data "
			   "(ie=%d len=%d pos=%d)\n",
			   pos[0], pos[1], (int) (pos - buf));


		if (*pos == WLAN_EID_RSN) {
			ie->rsn_ie = pos;
			ie->rsn_ie_len = pos[1] + 2;

			debug(DEBUG_TRACE, HAND_SHAKE_PREFIX"WPA: WLAN_EID_RSN (ie=%d len=%d)\n",
				pos[0], (u32)ie->rsn_ie_len);
		}
		else if (*pos == WLAN_EID_VENDOR_SPECIFIC) {

			/* Parse KDE format in pairwise_msg_3_WPA2 && group_msg_1_WPA2*/

			ret = wpa_parse_generic(pos, end, ie);
			if (ret < 0)
				break;
			if (ret > 0) {
				ret = 0;
				break;
			}
		}
		else {
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"WPA: WPA: Unrecognized EAPOL-Key Key Data IE\n");
		}
	}

	return ret;
}

Boolean wpa_parse_eapol_key_data(struct wpa_state_machine *sm, u8 GroupKeyIndex, u8 *pKeyData, u8 KeyDataLen, u8 MsgType, Boolean bWPA2)
{
	u8 *pMyKeyData = pKeyData;
	u8 KeyDataLength = KeyDataLen;
	u8 GTK[WPA_GTK_MAX_LEN], gtk_idx = 0, *pgtk = NULL;
	u8 GTKLEN = 0;
	u8 DefaultIdx = 0;
	struct wpa_eapol_ie_parse kde, *pkde = &kde;
	u8 _1905_mcast_addr[ETH_ALEN] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x13 };

	os_memset(GTK, 0, WPA_GTK_MAX_LEN);

	wpa_parse_kde_ies(pKeyData, KeyDataLen, pkde);
	if (pkde->gtk_len > WPA_GTK_MAX_LEN) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"gtk_len bigger than WPA_GTK_MAX_LEN!\n");
		return FALSE;
	}

	/* Verify The RSN IE contained in pairewise_msg_2 && pairewise_msg_3 and skip it
	if (MsgType == EAPOL_PAIR_MSG_2 || MsgType == EAPOL_PAIR_MSG_3) {

		 Check RSN IE whether it is WPA2/WPA2PSK
		if (bWPA2) {
			hex_dump_all("Receive RSN_IE ", (u8 *)pkde->rsn_ie, pkde->rsn_ie_len);
			hex_dump_all("Desired RSN_IE ", sm->wpa_param->wpa_ie, sm->wpa_param->wpa_ie_len);
		}
		else {
			hex_dump_all("Receive WPA_IE ", (u8 *)pkde->wpa_ie, pkde->wpa_ie_len);
			hex_dump_all("Desired WPA_IE ", sm->wpa_param->wpa_ie, sm->wpa_param->wpa_ie_len);
		}
	}*/

	/* ckech KDE format in pairwise_msg_3_WPA2 && group_msg_1_WPA2*/
	/* set gtk to security engine @Pair MSG3 or Group Msg1 */
	if (bWPA2 && (MsgType == EAPOL_PAIR_MSG_3 || MsgType == EAPOL_GROUP_MSG_1)) {
		//hex_dump_all("Receive KDE_IE ", (u8 *)pkde->gtk, (u32)pkde->gtk_len);
		if (!pkde->gtk_len || (!pkde->GN) || (!pkde->gtk)) {
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"no gtk in pairwise msg3 or group msg1\n");
			return TRUE;
		}

		sm->group->GN = gtk_idx = (*(pkde->GN)  & 0x03);
		if (gtk_idx < 1 || gtk_idx > 3) {
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"key index %d in pkt, reset to default 1\n", gtk_idx);
			gtk_idx = 1;
			//return FALSE;
		}

		pgtk = sm->group->GTK[gtk_idx - 1];
		sm->group->GTK_len = pkde->gtk_len;

		/* group valid if msg 3/4 and 1/2 included GTK on Agent */
		sm->group->valid = 1;

		os_memcpy(pgtk, pkde->gtk, pkde->gtk_len);
		if (MsgType == EAPOL_PAIR_MSG_3)
			hex_dump_info("got gtk in pair msg3", pgtk, pkde->gtk_len);
		else
			hex_dump_info("got gtk in grp msg 1", pgtk, pkde->gtk_len);
	}
	/*should not come here*/
	else if (!bWPA2 && MsgType == EAPOL_GROUP_MSG_1) {
		DefaultIdx = GroupKeyIndex;
		GTKLEN = KeyDataLength;
		if (GTKLEN > WPA_GTK_MAX_LEN) {
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"ERROR: GTK Key length is invalid (%d)\n", GTKLEN);
			return FALSE;
		}
		os_memcpy(GTK, pMyKeyData, KeyDataLength);
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"GTK without KDE, DefaultKeyID=%d, KeyLen=%d\n", DefaultIdx, GTKLEN);

		hex_dump_info("got gtk in group msg1", GTK, GTKLEN);

		sec_set_key(_1905_mcast_addr, GTK, GTKLEN, DefaultIdx);
	}

	/* Sanity check - shared key index must be 0 ~ 3*/
	if (DefaultIdx > 3) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"ERROR: GTK Key index(%d) is invalid in %s %s\n",
			DefaultIdx, ((bWPA2) ? "WPA2" : "WPA"), wpa_get_eapol_msg_type(MsgType));
		return FALSE;
	}
	return TRUE;
}


static int wpa_verify_key_mic(int akmp, size_t pmk_len, struct wpa_ptk *PTK,
				u8 *data, size_t data_len)
{
	struct ieee802_1x_hdr *hdr;
	struct wpa_eapol_key *key;
	int ret = 0;
	u8 mic[WPA_EAPOL_KEY_MIC_MAX_LEN], *mic_pos;
	size_t mic_len = wpa_mic_len(akmp, pmk_len);

	if (data_len < sizeof(*hdr) + sizeof(*key)) {
		return -1;
	}

	hdr = (struct ieee802_1x_hdr *) data;
	key = (struct wpa_eapol_key *) (hdr + 1);
	mic_pos = (u8 *) (key + 1);
	os_memcpy(mic, mic_pos, mic_len);
	os_memset(mic_pos, 0, mic_len);

	/* spec say: 256-bit MIC using the HMAC-SHA256 */
	if (wpa_eapol_key_mic(PTK->kck, PTK->kck_len,
				data, data_len, mic_pos) ||
		os_memcmp_const(mic, mic_pos, mic_len) != 0) {

		hex_dump_info("Received MIC", mic_pos, mic_len);
		hex_dump_info("Desired  MIC", mic, mic_len);
		ret = -1;
	}
	os_memcpy(mic_pos, mic, mic_len);
	return ret;
}




Boolean wpa_msg_sanity(struct wpa_state_machine *sm, u8 *buf, u16 msgLen, u8 MsgType)
{
	Boolean bWPA2 = TRUE;
	Boolean bReplayDiff = FALSE;
	u16 key_info = 0, keylen = 0, key_data_len = 0;
	u8 mic_len = wpa_mic_len(sm->key_mgmt, sm->pmk_len);
	struct ieee802_1x_hdr *hdr = NULL;
	struct wpa_eapol_key *key = NULL;
	u8 *key_data_len_ptr = NULL;
	u8 *key_data_ptr = NULL;
	u8 plaintext[MAX_LEN_OF_RSNIE]= {0};

	/* 0. Check MsgType*/
	if ((MsgType > EAPOL_GROUP_MSG_2) || (MsgType < EAPOL_PAIR_MSG_1)) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"The message type is invalid(%d)!\n", MsgType);
		return FALSE;
	}

	hdr = (struct ieee802_1x_hdr *) buf;
	key = (struct wpa_eapol_key *)(hdr + 1);
	keylen = be_to_host16(hdr->length) + sizeof(struct ieee802_1x_hdr);
	if (msgLen < keylen) {
		debug(DEBUG_ERROR,HAND_SHAKE_PREFIX"recv pkt len %d error less than %d!\n", msgLen,keylen);
		return FALSE;
	}

	os_memcpy(&key_info, key->key_info, sizeof(key->key_info));
	key_info = be_to_host16(key_info);

	/* Choose WPA2 or not*/
	if (sm->wpa == WPA_VERSION_WPA) {
		bWPA2 = FALSE;
	}

	/* 1. Replay counter check */
	/* For supplicant*/
	if (MsgType == EAPOL_PAIR_MSG_1 || MsgType == EAPOL_PAIR_MSG_3 || MsgType == EAPOL_GROUP_MSG_1) {
		/* First validate replay counter, only accept message with larger replay counter.*/
		/* Let equal pass, some AP start with all zero replay counter*/
		u8 ZeroReplay[WPA_REPLAY_COUNTER_LEN];

		os_memset(ZeroReplay, 0, WPA_REPLAY_COUNTER_LEN);

		if ((os_memcmp(key->replay_counter, sm->ReplayCounter, WPA_REPLAY_COUNTER_LEN) <= 0) &&
			(os_memcmp(key->replay_counter, ZeroReplay, WPA_REPLAY_COUNTER_LEN) != 0)) {
			bReplayDiff = TRUE;
		}
	}
	/* For authenticator*/
	else if (MsgType == EAPOL_PAIR_MSG_2 || MsgType == EAPOL_PAIR_MSG_4 || MsgType == EAPOL_GROUP_MSG_2) {
		/* check Replay Counter coresponds to MSG from authenticator, otherwise discard*/
		if (os_memcmp(key->replay_counter, sm->ReplayCounter, WPA_REPLAY_COUNTER_LEN)) {
			bReplayDiff = TRUE;
		}
	}

	/* Replay Counter different condition*/
	if (bReplayDiff) {

		if (MsgType < EAPOL_GROUP_MSG_1) {
			debug(DEBUG_ERROR,HAND_SHAKE_PREFIX"Replay Counter Different in pairwise msg %d of 4-way handshake!\n", MsgType);
		}
		else {
			debug(DEBUG_ERROR,HAND_SHAKE_PREFIX"Replay Counter Different in group msg %d of 2-way handshake!\n", (MsgType - EAPOL_PAIR_MSG_4));
		}

		hex_dump_info("Receive replay counter ", key->replay_counter, WPA_REPLAY_COUNTER_LEN);
		hex_dump_info("Current replay counter ", sm->ReplayCounter, WPA_REPLAY_COUNTER_LEN);
		debug(DEBUG_ERROR,"ignor replay counter check, just let it go!\n");
		//return FALSE;
	}

	key_data_len_ptr = (u8 *)((u8 *)(key + 1) + mic_len);
	key_data_ptr = (u8 *)(key_data_len_ptr + 2);

	/* 2. Verify MIC except Pairwise Msg1*/
	if (MsgType != EAPOL_PAIR_MSG_1) {
		if (-1 == wpa_verify_key_mic(sm->key_mgmt, sm->pmk_len, &sm->PTK, buf, msgLen)) {
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"mic check failed\n");
		}
		else {
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"mic check ok\n");
			sm->MICVerified = TRUE;
		}
	}

	/* 1. Decrypt the Key Data field if GTK is included.*/
	/* 2. Extract the context of the Key Data field if it exist.	 */
	/* The field in pairwise_msg_2_WPA1(WPA2) & pairwise_msg_3_WPA1 is clear.*/
	/* The field in group_msg_1_WPA1(WPA2) & pairwise_msg_3_WPA2 is encrypted.*/

	key_data_len = WPA_GET_BE16(key_data_len_ptr);
	if (key_data_len > MAX_LEN_OF_RSNIE) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX
			"WPA: RC4 key data too long (%lu)\n",
			(unsigned long) key_data_len);
		return FALSE;
	}

	if (key_data_len > 0) {
		u8 GroupKeyIndex = 0;

		/* Decrypt this field */
		if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1)) {

			/*AES unwrap*/
			if (key_info  & WPA_KEY_INFO_ENCR_KEY_DATA) {

				debug(DEBUG_ERROR,HAND_SHAKE_PREFIX
					   "WPA: Decrypt Key Data using AES-UNWRAP (KEK length %u)\n",
					   (u32) sm->PTK.kek_len);
				if (key_data_len < 8 || key_data_len % 8) {
					debug(DEBUG_ERROR,HAND_SHAKE_PREFIX
						"WPA: Unsupported AES-WRAP len %u\n",
						(u32) key_data_len);
					return FALSE;
				}
				key_data_len -= 8; /* AES-WRAP adds 8 bytes */

				/* AES unwrap */
				if (aes_unwrap(sm->PTK.kek, sm->PTK.kek_len, key_data_len/ 8,
								key_data_ptr, plaintext)) {
					debug(DEBUG_ERROR,HAND_SHAKE_PREFIX"WPA: AES unwrap failed - could not decrypt GTK\n");
					return FALSE;
				}
				//debug(DEBUG_ERROR, "GTK IE in msg %s\n", wpa_get_eapol_msg_type(MsgType));
				//hex_dump_all("KDE IE", plaintext, key_data_len);
			}
			/* TKIP unwrap */
			else {
				u8 ek[32];
				os_memcpy(ek, key->key_iv, 16);
				os_memcpy(ek + 16, sm->PTK.kek, sm->PTK.kek_len);
				os_memcpy(plaintext, key_data_ptr, key_data_len);
				if (rc4_skip(ek, 32, 256, plaintext, key_data_len)) {
					os_memset(ek, 0, sizeof(ek));
					debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"WPA: RC4 failed\n");
					return FALSE;
				}
				os_memset(ek, 0, sizeof(ek));
			}

			if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1)) {
				GroupKeyIndex = ((key_info & WPA_KEY_INFO_KEY_INDEX_MASK) >>
							WPA_KEY_INFO_KEY_INDEX_SHIFT);

				debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"WPA: GroupKeyIndex %d\n", GroupKeyIndex);
			}

		}
		/* plaintext of WPA IE in msg2 and ( msg3 && !wpa2) */
		else if ((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3 && !bWPA2)) {
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"WPA IE in msg %s\n", wpa_get_eapol_msg_type(MsgType));
			os_memcpy(plaintext, key_data_ptr, key_data_len);
		}
		/* no key data, return true */
		else {
			return TRUE;
		}

		/* Parse Key Data field to */
		/* 1. verify RSN IE for pairwise_msg_2_WPA1(WPA2) ,pairwise_msg_3_WPA1(WPA2)*/
		/* 2. verify KDE format for pairwise_msg_3_WPA2, group_msg_1_WPA2*/
		/* 3. update shared key for pairwise_msg_3_WPA2, group_msg_1_WPA1(WPA2)*/
		wpa_parse_eapol_key_data(sm, GroupKeyIndex, plaintext, key_data_len, MsgType, bWPA2);
	}
	return TRUE;
}


void wpa_set_key_to_security(struct wpa_state_machine *sm)
{
	u8 _1905_mcast_addr[ETH_ALEN] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x13 };
	u8 key_idx = 0, *pgtk = NULL;
	int GTK_len = 0;
	struct wpa_group *group = NULL;
	if (sm->PTK_valid) {
		//hex_dump_all("tk", sm->PTK.tk, sm->PTK.tk_len);

		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"set PTK for "MACSTR"\n", MAC2STR(sm->peer_addr));
		sec_set_key(sm->peer_addr, sm->PTK.tk, sm->PTK.tk_len, 0);
		sm->PTK_valid = 0;
	}

	/*
	** controller set key using global group parameter
	** Controller set GTK while exchanging 4-way message with first Agent after generating GTK
	*/
	if (sm->role == 0) {
		group = &sm->wpa_param->group;
		key_idx = group->GN;
		pgtk = group->GTK[key_idx - 1];
		GTK_len = group->GTK_len;
	}
	/*
	** agent set key using peer group parameter
	** Agent set GTK while GTK is included in msg3 from Controller to Agent
	*/
	else {
		group = sm->group;
		key_idx = group->GN;
		pgtk = group->GTK[key_idx - 1];
		GTK_len = group->GTK_len;
	}

	if (GTK_len)
		group->valid = 1;

	if (group->valid) {
		//hex_dump_all("GTK", pgtk, GTK_len);
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"set GTK to engine, index %d \n", key_idx);
		sec_set_key(_1905_mcast_addr, pgtk, GTK_len, key_idx);

		/*
		** reset group valid to 0
		*/
		group->valid = 0;
	}
	return;
}


void wpa_send_pair_msg_1(struct wpa_state_machine *sm)
{
	u16 msg_len = 0;

	os_memset(msg_buf, 0, TX_EAPOL_BUFFER);
	/* Increment replay counter by 1*/
	inc_byte_array(sm->ReplayCounter, WPA_REPLAY_COUNTER_LEN);

	/* Randomly generate ANonce */
	random_get_bytes(sm->ANonce, WPA_NONCE_LEN);

	hex_dump_info("ANONCE", (u8 *)sm->ANonce, WPA_NONCE_LEN);

	wpa_construct_eapol_msg(sm, EAPOL_PAIR_MSG_1, msg_buf, &msg_len);

	dev_send_eapol(sm->wpa_param->p1905_ctx, sm->peer_addr, msg_buf, msg_len);

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"\n");
}


void wpa_send_pair_msg_2(struct wpa_state_machine *sm)
{
	u16 msg_len = 0;

	os_memset(msg_buf, 0, TX_EAPOL_BUFFER);

	wpa_construct_eapol_msg(sm, EAPOL_PAIR_MSG_2,  msg_buf, &msg_len);

	dev_send_eapol(sm->wpa_param->p1905_ctx, sm->peer_addr, msg_buf, msg_len);

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"\n");
}

void wpa_send_pair_msg_3(struct wpa_state_machine *sm)
{
	u16 msg_len = 0;
	u8 key_index = 0;

	os_memset(msg_buf, 0, sizeof(msg_buf));

	/* Increment replay counter by 1*/
	inc_byte_array(sm->ReplayCounter, WPA_REPLAY_COUNTER_LEN);

	/* Randomly generate ANonce */
	//random_get_bytes(sm->ANonce, WPA_NONCE_LEN);

	/* Get RSC */
	os_memset(sm->RSC, 0, WPA_KEY_RSC_LEN);

	if (sm->role == 0 && !sm->wpa_param->group.GTK_len) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX" INIT GTK here\n");

		/* default keyidex = 1 */
		key_index = sm->wpa_param->group.GN;
		if (key_index<1 || key_index > 3) {
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX" GTK index invalid %d\n", key_index);
			return;
		}
		if (random_get_bytes(&sm->wpa_param->group.GTK[key_index-1], WPA_GMK_LEN) < 0)
			return;
		sm->wpa_param->group.GTK_len = WPA_GMK_LEN;
		hex_dump_info("GTK", sm->wpa_param->group.GTK[key_index-1], WPA_GMK_LEN);
	}

	wpa_construct_eapol_msg(sm, EAPOL_PAIR_MSG_3, msg_buf, &msg_len);

	dev_send_eapol(sm->wpa_param->p1905_ctx, sm->peer_addr, msg_buf, msg_len);

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"\n");
}


void wpa_send_pair_msg_4(struct wpa_state_machine *sm)
{
	u16 msg_len = 0;

	os_memset(msg_buf, 0, sizeof(msg_buf));
	/* install key */
	//WPAInstallKey(pAd, &Info, TRUE, TRUE);

	os_memset(msg_buf, 0, TX_EAPOL_BUFFER);

	wpa_construct_eapol_msg(sm, EAPOL_PAIR_MSG_4, msg_buf, &msg_len);

	dev_send_eapol(sm->wpa_param->p1905_ctx, sm->peer_addr, msg_buf, msg_len);

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"\n");
}


void wpa_send_group_msg_1(struct wpa_state_machine *sm)
{
	u16 msg_len = 0;

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"group key handshake start\n");

	/* Increment replay counter by 1*/
	inc_byte_array(sm->ReplayCounter, WPA_REPLAY_COUNTER_LEN);

	/* Get RSC */

	/* install key */
	//WPAInstallKey(pAd, &Info, TRUE, TRUE);

	os_memset(msg_buf, 0, TX_EAPOL_BUFFER);

	sm->gtk_sm = EAPOL_GROUP_MSG_1;

	wpa_construct_eapol_msg(sm, EAPOL_GROUP_MSG_1, msg_buf, &msg_len);

	/* create timer to rekey GTK??? */

	dev_send_eapol(sm->wpa_param->p1905_ctx, sm->peer_addr, msg_buf, msg_len);

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"\n");
}


void wpa_send_group_msg_2(struct wpa_state_machine *sm)
{
	u16 msg_len = 0;

	os_memset(msg_buf, 0, TX_EAPOL_BUFFER);

	sm->gtk_sm = EAPOL_GROUP_MSG_2;

	wpa_construct_eapol_msg(sm, EAPOL_GROUP_MSG_2, msg_buf, &msg_len);

	/* create timer to rekey GTK??? */

	dev_send_eapol(sm->wpa_param->p1905_ctx, sm->peer_addr, msg_buf, msg_len);

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"\n");
}


void peer_pair_msg_1_action(struct wpa_state_machine *sm, u8 *data, u16 data_len)
{
	struct ieee802_1x_hdr *hdr = NULL;
	struct wpa_eapol_key *key = NULL;
	struct wpa_ptk *ptk = NULL;

	hdr = (struct ieee802_1x_hdr *)data;
	key = (struct wpa_eapol_key *)(hdr + 1);

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"\n");

	if (FALSE == wpa_msg_sanity(sm, data, data_len, EAPOL_PAIR_MSG_1)) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"*****sanity check fail*****,skip\n");
		return;
	}

	/* Store Replay counter, it will use to verify message 3 and construct message 2*/
	os_memcpy(sm->ReplayCounter, key->replay_counter, WPA_REPLAY_COUNTER_LEN);
	/* Store ANonce*/
	os_memcpy(sm->ANonce, key->key_nonce, WPA_NONCE_LEN);
	/* Generate random SNonce*/
	random_get_bytes(sm->SNonce, WPA_NONCE_LEN);

	/* Calculate PTK which will be stored as a temporary PTK until it has
	 * been verified when processing message 3/4. */
	ptk = &sm->PTK;
	if (wpa_pmk_to_ptk(sm->PMK, sm->pmk_len, "Pairwise key expansion",
		sm->own_addr, sm->peer_addr, sm->SNonce,
		sm->ANonce, ptk, sm->key_mgmt,
		sm->pairwise_cipher) < 0) {
		sm->PTK_valid = 0;
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"derive ptk failed\n");
		return;
	}

	sm->PTK_valid = 1;
	if (sm->pairwise_cipher == WPA_CIPHER_TKIP) {
		u8 buf[8];
		/* Supplicant: swap tx/rx Mic keys */
		os_memcpy(buf, &ptk->tk[16], 8);
		os_memcpy(&ptk->tk[16], &ptk->tk[24], 8);
		os_memcpy(&ptk->tk[24], buf, 8);
		os_memset(buf, 0, sizeof(buf));
	}

	sm->ptk_sm = wpa_send_pair_msg2;
	eloop_cancel_timeout(wpa_sm_step, (void *)sm, NULL);
	eloop_register_timeout(0, 0, wpa_sm_step, (void *)sm, NULL);
}


void peer_pair_msg_2_action(struct wpa_state_machine *sm, u8 *data, u16 data_len)
{
	struct ieee802_1x_hdr *hdr;
	struct wpa_eapol_key *key;

	hdr = (struct ieee802_1x_hdr *)data;
	key = (struct wpa_eapol_key *)(hdr + 1);

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"\n");

	/* cancel timer for seding Pair Msg 1 after receving Pair Msg 2  */
	eloop_cancel_timeout(wpa_start_4way_handshake, (void *)sm->peer_addr, NULL);
	sm->ptk_retry_cnt = 0;
	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"cancel timeout 4-way handshake for "MACSTR"\n", MAC2STR(sm->peer_addr));

	/* Store SNonce*/
	os_memcpy(sm->SNonce, key->key_nonce, WPA_NONCE_LEN);

	/* install key */
	if (wpa_pmk_to_ptk(sm->PMK, sm->pmk_len, "Pairwise key expansion",
		sm->own_addr, sm->peer_addr, sm->ANonce,
		sm->SNonce, &sm->PTK, sm->key_mgmt,
		sm->pairwise_cipher) < 0) {
		sm->PTK_valid = 0;
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"derive ptk failed\n");
		return;
	}

	if (FALSE == wpa_msg_sanity(sm, data, data_len, EAPOL_PAIR_MSG_2)) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"*****sanity check fail*****,skip\n");
		return;
	}

	sm->PTK_valid = 1;

	sm->ptk_sm = wpa_send_pair_msg3;
	eloop_cancel_timeout(wpa_sm_step, (void *)sm, NULL);
	eloop_register_timeout(0, 0, wpa_sm_step, (void *)sm, NULL);
	return;
}

void peer_pair_msg_3_action(struct wpa_state_machine *sm, u8 *data, u16 data_len)
{
	struct ieee802_1x_hdr *hdr;
	struct wpa_eapol_key *key;

	hdr = (struct ieee802_1x_hdr *)data;
	key = (struct wpa_eapol_key *)(hdr + 1);

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"\n");

	if (FALSE == wpa_msg_sanity(sm, data, data_len, EAPOL_PAIR_MSG_3)) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"*****sanity check fail*****,skip\n");
		return;
	}

	/* rsn handle */

	/* RSC update */
	os_memcpy(sm->RSC, key->key_rsc, WPA_KEY_RSC_LEN);

	/* Save Replay counter, it will use construct message 4*/
	os_memcpy(sm->ReplayCounter, key->replay_counter, WPA_REPLAY_COUNTER_LEN);

	/* Double check ANonce*/
	if (os_memcmp(sm->ANonce, key->key_nonce, WPA_NONCE_LEN) != 0) {
		debug(DEBUG_ERROR,HAND_SHAKE_PREFIX
			"WPA: ANonce from message 1 of 4-Way Handshake "
			"differs from 3 of 4-Way Handshake - drop packet (src="
			MACSTR ")\n", MAC2STR(sm->peer_addr));
		return;
	}

	sm->ptk_sm = wpa_send_pair_msg4;

	wpa_send_pair_msg_4(sm);
	sm->ptk_sm = wpa_pair_suc;
	/* supplicant set ptk to security engine after receiving Pair Msg3 */
	wpa_set_key_to_security(sm);

}



void peer_pair_msg_4_action(struct wpa_state_machine *sm, u8 *data, u16 data_len)
{
	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"\n");

	if (FALSE == wpa_msg_sanity(sm, data, data_len, EAPOL_PAIR_MSG_4)) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"*****sanity check fail*****,skip\n");
		return;
	}

	/* cancel wpa_send_pair_msg1 after receiving m4 */
	eloop_cancel_timeout(wpa_sm_step, (void *)sm, NULL);
	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"cancel retry 1/4 for "MACSTR"\n", MAC2STR(sm->peer_addr));

	/* authenticator set ptk to security engine after receiving Pair Msg2 */
	//hex_dump_all("tk", sm->PTK.tk, sm->PTK.tk_len);
	/* supplicant set ptk to security engine after receiving Pair Msg3 */
	wpa_set_key_to_security(sm);

	/* WPA: start group key */
	if (sm->wpa == WPA_VERSION_WPA) {
		sm->gtk_sm = wpa_send_group_msg1;
		eloop_cancel_timeout(wpa_sm_step, (void *)sm, NULL);
		eloop_register_timeout(0, 0, wpa_sm_step, (void *)sm, NULL);
	}

	sm->ptk_sm = wpa_pair_suc;

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"with "MACSTR" finished\n", MAC2STR(sm->peer_addr));
}


void peer_group_msg_1_action(struct wpa_state_machine *sm, u8 *data, u16 data_len)
{
	struct ieee802_1x_hdr *hdr;
	struct wpa_eapol_key *key;

	hdr = (struct ieee802_1x_hdr *)data;
	key = (struct wpa_eapol_key *)(hdr + 1);

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"\n");

	if (FALSE == wpa_msg_sanity(sm, data, data_len, EAPOL_GROUP_MSG_1)) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"*****sanity check fail*****,skip\n");
		return;
	}

	/* RSC handle */

	/* Save Replay counter, it will use to construct message 2*/
	os_memcpy(sm->ReplayCounter, key->replay_counter, WPA_REPLAY_COUNTER_LEN);

	sm->gtk_sm = wpa_send_group_msg2;
	eloop_cancel_timeout(wpa_sm_step, (void *)sm, NULL);
	eloop_register_timeout(0, 0, wpa_sm_step, (void *)sm, NULL);

	wpa_set_key_to_security(sm);

}

void peer_group_msg_2_action(struct wpa_state_machine *sm, u8 *data, u16 data_len)
{
	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"\n");

	if (FALSE == wpa_msg_sanity(sm, data, data_len, EAPOL_GROUP_MSG_2)) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"*****sanity check fail*****,skip\n");
		return;
	}

	wpa_set_key_to_security(sm);

	sm->gtk_sm = wpa_group_suc;

	debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"group key hankshake finished\n");
}



void wpa_start_4way_handshake(void *eloop_ctx, void *timeout_ctx)
{
	struct wpa_entry_info *entry = NULL;
	u8 *peer_addr = eloop_ctx;

	if (NULL == (entry = wpa_get_entry(peer_addr))) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"failed,can't find entry\n");
		return;
	}

	if (entry->wpa_sm &&
		(!(entry->wpa_sm->ptk_sm > wpa_idle && entry->wpa_sm->ptk_sm < wpa_pair_suc))) {
		entry->wpa_sm->ptk_sm = wpa_send_pair_msg1;
		eloop_cancel_timeout(wpa_sm_step, (void *)entry->wpa_sm, NULL);
		eloop_register_timeout(0, 0, wpa_sm_step, (void *)entry->wpa_sm, NULL);
	}

	return;
}

void wpa_sm_step(void *eloop_ctx, void *timeout_ctx)
{
	struct wpa_state_machine *sm = (struct wpa_state_machine *)eloop_ctx;

	wpa_sm(sm);
}


void wpa_sm(struct wpa_state_machine *sm)
{
	switch(sm->ptk_sm) {
		case wpa_send_pair_msg1:
		{
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"send pair msg 1\n");
			wpa_send_pair_msg_1(sm);
			sm->ptk_sm = wpa_wait_pair_msg2;
			eloop_cancel_timeout(wpa_sm_step, (void *)sm, NULL);
			eloop_register_timeout(5, 0, wpa_sm_step, (void *)sm, NULL);
		}
		break;

		case wpa_wait_pair_msg2:
		{
			if (sm->ptk_retry_cnt < 3) {
				sm->ptk_retry_cnt ++;
				debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"retry %d pair msg1\n", sm->ptk_retry_cnt);
				sm->ptk_sm = wpa_send_pair_msg1;
				eloop_cancel_timeout(wpa_sm_step, (void *)sm, NULL);
				eloop_register_timeout(0, 0, wpa_sm_step, (void *)sm, NULL);
			}
			else {
				struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)sm->wpa_param->p1905_ctx;
				debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"retry pair msg1 timeout,start autoconfig search procedure\n");
				sm->ptk_sm = wpa_pair_fail;
				sm->ptk_retry_cnt = 0;

				/* timeout, trigger autoconfig search procedure */
				if (ctx) {
					ctx->r3_oboard_ctx.bss_renew = 0;
					ctx->r3_oboard_ctx.onboarding_stage = R3_ONBOARDING_STAGE_INVALID;
					ctx->enrolle_state = wait_4_send_controller_search;
					eloop_cancel_timeout(ap_controller_search_step, (void *)ctx, NULL);
					eloop_register_timeout(3, 0, ap_controller_search_step, (void *)ctx, NULL);
				}
			}
		}
		break;

		case wpa_send_pair_msg2:
		{
			wpa_send_pair_msg_2(sm);
			sm->ptk_sm = wpa_wait_pair_msg3;
		}
		break;

		case wpa_send_pair_msg3:
		{
			wpa_send_pair_msg_3(sm);
			sm->ptk_sm = wpa_wait_pair_msg4;
			eloop_cancel_timeout(wpa_sm_step, (void *)sm, NULL);
			eloop_register_timeout(3, 0, wpa_sm_step, (void *)sm, NULL);
		}
		break;

		case wpa_wait_pair_msg4:
		{
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"fail to receive msg4, retry msg1\n");
			sm->ptk_sm = wpa_send_pair_msg1;
			eloop_cancel_timeout(wpa_sm_step, (void *)sm, NULL);
			eloop_register_timeout(0, 0, wpa_sm_step, (void *)sm, NULL);
		}
		break;

		default:
			break;
	}

	switch(sm->gtk_sm) {
		case wpa_send_group_msg1:
		{
			wpa_send_group_msg_1(sm);
			sm->ptk_sm = wpa_wait_group_msg2;
		}
		break;

		case wpa_send_group_msg2:
		{
			wpa_send_group_msg_2(sm);
			sm->gtk_sm = wpa_group_suc;
		}
		break;

		default:
			break;
	}

}



int wpa_receive(struct wpa_state_machine *sm,
		 u8 *data, size_t data_len)
{
	struct ieee802_1x_hdr *hdr;
	struct wpa_eapol_key *key;
	u16 key_info, key_data_length, ver;
	size_t mic_len;
	u8 *mic;
	int msgType = EAPOL_MSG_INVALID;

	if (!sm) {
		debug(DEBUG_ERROR,HAND_SHAKE_PREFIX"%s sm is null\n", __func__);
		goto invalid;
	}

	mic_len = wpa_mic_len(sm->key_mgmt, sm->pmk_len);

	if (data_len < sizeof(struct ieee802_1x_hdr) + MIN_LEN_OF_EAPOL_KEY_MSG) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"WPA: Ignore too short EAPOL-Key frame %d\n", (int)data_len);
		goto invalid;
	}

	hdr = (struct ieee802_1x_hdr *) data;
	if (hdr->type != IEEE802_1X_TYPE_EAPOL_KEY) {
		debug(DEBUG_ERROR,HAND_SHAKE_PREFIX
			"WPA: EAPOL frame (type %u) discarded, "
			"not a Key frame\n", hdr->type);
		goto invalid;
	}

	key = (struct wpa_eapol_key *) (hdr + 1);
	if (key->type != EAPOL_KEY_TYPE_WPA && key->type != EAPOL_KEY_TYPE_RSN)
	{
		debug(DEBUG_ERROR,HAND_SHAKE_PREFIX
			"WPA: EAPOL-Key type (%d) unknown, discarded\n",
			key->type);
		goto invalid;
	}

	/* ker version check */
	key_info = WPA_GET_BE16(key->key_info);
	ver = key_info & WPA_KEY_INFO_TYPE_MASK;

	if (ver != WPA_KEY_INFO_TYPE_HMAC_MD5_RC4 &&
		ver != WPA_KEY_INFO_TYPE_HMAC_SHA1_AES &&
		ver != 0) {
		/* DPP ver is 0 */
		debug(DEBUG_ERROR,HAND_SHAKE_PREFIX
			"WPA: Unsupported EAPOL-Key descriptor version %d\n", ver);
		goto invalid;
	}

	/* check if wpa_version match key type */
	if (sm->wpa == WPA_VERSION_WPA2) {
		if (key->type == EAPOL_KEY_TYPE_WPA) {
			/*
			 * Some deployed station implementations seem to send
			 * msg 4/4 with incorrect type value in WPA2 mode.
			 */
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"Workaround: Allow EAPOL-Key "
				   "with unexpected WPA type in RSN mode\n");
		} else if (key->type != EAPOL_KEY_TYPE_RSN) {
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"Ignore EAPOL-Key with "
				   "unexpected type %d in RSN mode\n",
				   key->type);
			goto invalid;
		}
	} else {
		if (key->type != EAPOL_KEY_TYPE_WPA) {
			debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"Ignore EAPOL-Key with "
				   "unexpected type %d in WPA mode\n",
				   key->type);
			goto invalid;
		}
	}

	/* Check the Key Ack (bit 7) of the Key Information to determine the Authenticator or not.*/
	/* An EAPOL-Key frame that is sent by the Supplicant in response to an EAPOL-*/
	/* Key frame from the Authenticator must not have the Ack bit set.*/

	/* ACK */
	if (key_info & WPA_KEY_INFO_ACK) {
		/* The frame is sent by Authenticator. So the Supplicant side shall handle this.*/

		if ((!(key_info & WPA_KEY_INFO_REQUEST))
			&& (!(key_info & WPA_KEY_INFO_ERROR))
			&& (key_info & WPA_KEY_INFO_KEY_TYPE)) {

			/* Process
			    1. the message 1 of 4-way HS in WPA or WPA2
				EAPOL-Key(0,0,1,0,P,0,0,ANonce,0,DataKD_M1)
			    2. the message 3 of 4-way HS in WPA
				EAPOL-Key(0,1,1,1,P,0,KeyRSC,ANonce,MIC,DataKD_M3)
			 */
			if (!(key_info & WPA_KEY_INFO_MIC)) {
				debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"handle peer pair msg 1\n");
				peer_pair_msg_1_action(sm, data, data_len);
				msgType = EAPOL_PAIR_MSG_1;
			}
			else {
				debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"handle peer pair msg 3\n");
				peer_pair_msg_3_action(sm, data, data_len);
				msgType = EAPOL_PAIR_MSG_3;
			}
		}
		else if ((key_info & WPA_KEY_INFO_SECURE) &&
			(key_info & WPA_KEY_INFO_MIC) &&
			(!(key_info & WPA_KEY_INFO_REQUEST)) &&
			(!(key_info & WPA_KEY_INFO_ERROR))) {
			/* Process
			    1. the message 3 of 4-way HS in WPA2
				EAPOL-Key(1,1,1,1,P,0,KeyRSC,ANonce,MIC,DataKD_M3)
			    2. the message 1 of group KS in WPA or WPA2
				EAPOL-Key(1,1,1,0,G,0,Key RSC,0, MIC,GTK[N])
			*/
			if (key_info & WPA_KEY_INFO_KEY_TYPE) {
				debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"handle peer pair msg 3\n");
				peer_pair_msg_3_action(sm, data, data_len);
				msgType = EAPOL_PAIR_MSG_3;
			}
			else {
				debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"handle peer group msg 1\n");
				peer_group_msg_1_action(sm, data, data_len);
				msgType = EAPOL_GROUP_MSG_1;
			}
		}
	}
	else {
		/* The frame is snet by Supplicant.So the Authenticator side shall handle this. */
		if ((key_info & WPA_KEY_INFO_MIC) && (key_info & WPA_KEY_INFO_REQUEST)
			&& (key_info & WPA_KEY_INFO_ERROR)) {
			/* The Supplicant uses a single Michael MIC Failure Report frame */
			/* to report a MIC failure event to the Authenticator. */
			/* A Michael MIC Failure Report is an EAPOL-Key frame with */
			/* the following Key Information field bits set to 1: */
			/* MIC bit, Error bit, Request bit, Secure bit.*/
			debug(DEBUG_ERROR, (HAND_SHAKE_PREFIX"Received an Michael MIC Failure Report, active countermeasure\n"));
			sm->dot11RSNAStatsTKIPRemoteMICFailures ++;
		} else {
			if ((!(key_info & WPA_KEY_INFO_REQUEST)) && (!(key_info & WPA_KEY_INFO_ERROR))
				&& (key_info & WPA_KEY_INFO_MIC)) {
				if ((!(key_info & WPA_KEY_INFO_SECURE)) && (key_info & WPA_KEY_INFO_KEY_TYPE)) {
					/*
					EAPOL-Key(0,1,0,0,P,0,0,SNonce,MIC,Data) Process:
					1. message 2 of 4-way HS in WPA or WPA2
					2. message 4 of 4-way HS in WPA
					*/

					mic = (u8 *) (key + 1);

					key_data_length = WPA_GET_BE16(mic + mic_len);

					if (key_data_length == 0) {
						debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"handle peer pair msg 4\n");
						peer_pair_msg_4_action(sm, data, data_len);
						msgType = EAPOL_PAIR_MSG_4;
					}
					else {
						debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"handle peer pair msg 2\n");
						peer_pair_msg_2_action(sm, data, data_len);
						msgType = EAPOL_PAIR_MSG_2;
					}
				} else if ((key_info & WPA_KEY_INFO_SECURE) && (key_info & WPA_KEY_INFO_KEY_TYPE)) {
					/* EAPOL-Key(1,1,0,0,P,0,0,0,MIC,0) */
					/* Process message 4 of 4-way HS in WPA2*/
					debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"handle peer pair msg 4\n");
					peer_pair_msg_4_action(sm, data, data_len);
					msgType = EAPOL_PAIR_MSG_4;
				} else if ((key_info & WPA_KEY_INFO_SECURE) && (!(key_info & WPA_KEY_INFO_KEY_TYPE))) {
					/* EAPOL-Key(1,1,0,0,G,0,0,0,MIC,0)*/
					/* Process message 2 of Group key HS in WPA or WPA2 */
					debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"handle peer group msg 2\n");
					peer_group_msg_2_action(sm, data, data_len);
					msgType = EAPOL_GROUP_MSG_2;
				}
			}
		}
	}

invalid:
	return msgType;
}


int wpa_receive_from_1905(u8 *peer_addr, u8 *data, size_t data_len)
{
	struct wpa_entry_info *entry = NULL;

	entry = wpa_get_entry(peer_addr);

	if (entry) {
		return wpa_receive(entry->wpa_sm, data, data_len);
	}

	return EAPOL_MSG_INVALID;
}


u32 wpa_cipher_to_suite(int proto, int cipher)
{
	if (cipher & WPA_CIPHER_CCMP_256)
		return RSN_CIPHER_SUITE_CCMP_256;
	if (cipher & WPA_CIPHER_GCMP_256)
		return RSN_CIPHER_SUITE_GCMP_256;
	if (cipher & WPA_CIPHER_CCMP)
		return (proto == WPA_PROTO_RSN ?
			RSN_CIPHER_SUITE_CCMP : WPA_CIPHER_SUITE_CCMP);
	if (cipher & WPA_CIPHER_GCMP)
		return RSN_CIPHER_SUITE_GCMP;
	if (cipher & WPA_CIPHER_TKIP)
		return (proto == WPA_PROTO_RSN ?
			RSN_CIPHER_SUITE_TKIP : WPA_CIPHER_SUITE_TKIP);
	if (cipher & WPA_CIPHER_NONE)
		return (proto == WPA_PROTO_RSN ?
			RSN_CIPHER_SUITE_NONE : WPA_CIPHER_SUITE_NONE);
	if (cipher & WPA_CIPHER_GTK_NOT_USED)
		return RSN_CIPHER_SUITE_NO_GROUP_ADDRESSED;
	if (cipher & WPA_CIPHER_AES_128_CMAC)
		return RSN_CIPHER_SUITE_AES_128_CMAC;
	if (cipher & WPA_CIPHER_BIP_GMAC_128)
		return RSN_CIPHER_SUITE_BIP_GMAC_128;
	if (cipher & WPA_CIPHER_BIP_GMAC_256)
		return RSN_CIPHER_SUITE_BIP_GMAC_256;
	if (cipher & WPA_CIPHER_BIP_CMAC_256)
		return RSN_CIPHER_SUITE_BIP_CMAC_256;
	return 0;
}


int rsn_cipher_put_suites(u8 *start, int ciphers)
{
	u8 *pos = start;

	if (ciphers & WPA_CIPHER_CCMP_256) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_CCMP_256);
		pos += RSN_SELECTOR_LEN;
	}
	if (ciphers & WPA_CIPHER_GCMP_256) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_GCMP_256);
		pos += RSN_SELECTOR_LEN;
	}
	if (ciphers & WPA_CIPHER_CCMP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_CCMP);
		pos += RSN_SELECTOR_LEN;
	}
	if (ciphers & WPA_CIPHER_GCMP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_GCMP);
		pos += RSN_SELECTOR_LEN;
	}
	if (ciphers & WPA_CIPHER_TKIP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_TKIP);
		pos += RSN_SELECTOR_LEN;
	}
	if (ciphers & WPA_CIPHER_NONE) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_NONE);
		pos += RSN_SELECTOR_LEN;
	}

	return (pos - start) / RSN_SELECTOR_LEN;
}



int wpa_write_rsn_ie(struct wpa_auth_config *conf, u8 *buf, size_t len,
			const u8 *pmkid)
{
	struct rsn_ie_hdr *hdr;
	int num_suites, res;
	u8 *pos, *count;
	u16 capab;
	u32 suite;

	hdr = (struct rsn_ie_hdr *) buf;
	hdr->elem_id = WLAN_EID_RSN;
	WPA_PUT_LE16(hdr->version, RSN_VERSION);
	pos = (u8 *) (hdr + 1);

	suite = wpa_cipher_to_suite(WPA_PROTO_RSN, conf->wpa_group);
	if (suite == 0) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"Invalid group cipher (%d).\n",
			   conf->wpa_group);
		return -1;
	}
	RSN_SELECTOR_PUT(pos, suite);
	pos += RSN_SELECTOR_LEN;

	num_suites = 0;
	count = pos;
	pos += 2;


	res = rsn_cipher_put_suites(pos, conf->rsn_pairwise);
	num_suites += res;
	pos += res * RSN_SELECTOR_LEN;


	if (num_suites == 0) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"Invalid pairwise cipher (%d).\n",
			   conf->rsn_pairwise);
		return -1;
	}
	WPA_PUT_LE16(count, num_suites);

	num_suites = 0;
	count = pos;
	pos += 2;


	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_IEEE8021X) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_UNSPEC_802_1X);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_PSK) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_IEEE8021X_SUITE_B) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_802_1X_SUITE_B);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_IEEE8021X_SUITE_B_192) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_802_1X_SUITE_B_192);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}


	if (num_suites == 0) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"Invalid key management type (%d).\n",
			   conf->wpa_key_mgmt);
		return -1;
	}
	WPA_PUT_LE16(count, num_suites);

	/* RSN Capabilities */
	capab = 0;
	//if (conf->rsn_preauth)
	//	capab |= WPA_CAPABILITY_PREAUTH;
	WPA_PUT_LE16(pos, capab);
	pos += 2;

	if (pmkid) {
		if (2 + PMKID_LEN > buf + len - pos)
			return -1;
		/* PMKID Count */
		WPA_PUT_LE16(pos, 1);
		pos += 2;
		os_memcpy(pos, pmkid, PMKID_LEN);
		pos += PMKID_LEN;
	}



	hdr->len = (pos - buf) - 2;

	return pos - buf;
}

int wpa_cipher_put_suites(u8 *start, int ciphers)
{
	u8 *pos = start;

	if (ciphers & WPA_CIPHER_CCMP) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_CCMP);
		pos += WPA_SELECTOR_LEN;
	}
	if (ciphers & WPA_CIPHER_TKIP) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_TKIP);
		pos += WPA_SELECTOR_LEN;
	}
	if (ciphers & WPA_CIPHER_NONE) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_NONE);
		pos += WPA_SELECTOR_LEN;
	}

	return (pos - start) / RSN_SELECTOR_LEN;
}


static int wpa_write_wpa_ie(struct wpa_auth_config *conf, u8 *buf, size_t len)
{
	struct wpa_ie_hdr *hdr;
	int num_suites;
	u8 *pos, *count;
	u32 suite;

	hdr = (struct wpa_ie_hdr *) buf;
	hdr->elem_id = WLAN_EID_VENDOR_SPECIFIC;
	RSN_SELECTOR_PUT(hdr->oui, WPA_OUI_TYPE);
	WPA_PUT_LE16(hdr->version, WPA_VERSION);
	pos = (u8 *) (hdr + 1);

	suite = wpa_cipher_to_suite(WPA_PROTO_WPA, conf->wpa_group);
	if (suite == 0) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"Invalid group cipher (%d).\n",
			   conf->wpa_group);
		return -1;
	}
	RSN_SELECTOR_PUT(pos, suite);
	pos += WPA_SELECTOR_LEN;

	count = pos;
	pos += 2;

	num_suites = wpa_cipher_put_suites(pos, conf->wpa_pairwise);
	if (num_suites == 0) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"Invalid pairwise cipher (%d).\n",
			   conf->wpa_pairwise);
		return -1;
	}
	pos += num_suites * WPA_SELECTOR_LEN;
	WPA_PUT_LE16(count, num_suites);

	num_suites = 0;
	count = pos;
	pos += 2;

	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_IEEE8021X) {
		RSN_SELECTOR_PUT(pos, WPA_AUTH_KEY_MGMT_UNSPEC_802_1X);
		pos += WPA_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_PSK) {
		RSN_SELECTOR_PUT(pos, WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X);
		pos += WPA_SELECTOR_LEN;
		num_suites++;
	}

	if (num_suites == 0) {
		debug(DEBUG_ERROR, HAND_SHAKE_PREFIX"Invalid key management type (%d).\n",
			   conf->wpa_key_mgmt);
		return -1;
	}
	WPA_PUT_LE16(count, num_suites);

	/* WPA Capabilities; use defaults, so no need to include it */

	hdr->len = (pos - buf) - 2;

	return pos - buf;
}



int wpa_gen_wpa_ie(struct wpa_parameter *param)
{
	u8 *pos, buf[128];
	int res;
	pos = buf;

	if (param->wpa_conf.wpa & WPA_PROTO_RSN) {
		res = wpa_write_rsn_ie(&param->wpa_conf,
					pos, buf + sizeof(buf) - pos, NULL);
		if (res < 0)
			return res;
		pos += res;
	}
	if (param->wpa_conf.wpa & WPA_PROTO_WPA) {
		res = wpa_write_wpa_ie(&param->wpa_conf,
					pos, buf + sizeof(buf) - pos);
		if (res < 0)
			return res;
		pos += res;
	}

	param->wpa_ie = os_malloc(pos - buf);
	if (param->wpa_ie == NULL)
		return -1;
	os_memcpy(param->wpa_ie, buf, pos - buf);
	param->wpa_ie_len = pos - buf;

	return 0;
}


/* init config and generate wpa/rsn ie */
void * wpa_init(void *ctx, unsigned char role)
{
	struct wpa_parameter *wpa_param = &wpa_global;
	struct wpa_auth_config *conf = &wpa_param->wpa_conf;
	struct p1905_managerd_ctx *p1905_ctx = NULL;

	os_memset(wpa_param, 0, sizeof(struct wpa_parameter));

	wpa_param->p1905_ctx = ctx;
	wpa_param->role = role;

	conf->wpa_key_mgmt = WPA_KEY_MGMT_PSK;
	conf->wpa = WPA_VERSION_WPA2;

	conf->wpa_pairwise = WPA_CIPHER_CCMP_256;
	conf->rsn_pairwise = WPA_CIPHER_CCMP_256;

	conf->wpa_group = WPA_CIPHER_CCMP_256;

	/* PTK rekey every 60 min */
	conf->ptk_rekey_timer = PTK_REKEY_TIMER;
	p1905_ctx = (struct p1905_managerd_ctx *)wpa_param->p1905_ctx;
	conf->gtk_rekey_timer = p1905_ctx->gtk_rekey_interval;

	/* controller timer to send 1905 Rekey Request Message */
	if (role == 0) {
#ifdef PTK_REKEY
		debug(DEBUG_ERROR, "PTK rekey timer %d\n",conf->ptk_rekey_timer);
		eloop_register_timeout(conf->ptk_rekey_timer, 0,
			resend_ptk_rekey_request_timer, (void *)wpa_param, NULL);
#endif//#ifdef PTK_REKEY
		debug(DEBUG_ERROR, "GTK rekey timer %d\n",conf->gtk_rekey_timer);
		if (!p1905_ctx->MAP_Cer) {
			eloop_register_timeout(conf->gtk_rekey_timer, 0,
				resend_gtk_timer, (void *)wpa_param, NULL);
		}
		if (NULL == wpa_group_init_global(&wpa_param->group, conf->wpa_group)) {
			debug(DEBUG_ERROR, "sm group init failed\n");
			return NULL;
		}
	}
	debug(DEBUG_ERROR, "init wpa parameter as %s.\n", (role == 0)?"Controller":"Agent");

	dl_list_init(&wpa_param->entry_list);

	if (wpa_gen_wpa_ie(wpa_param)) {
		debug(DEBUG_ERROR, "Could not generate WPA IE.\n");
		return NULL;
	}

	debug(DEBUG_ERROR, "success\n");
	return wpa_param;
}


void wpa_eloop_cancel_entry_timeout(u8 *mac)
{
	struct wpa_entry_info *entry = NULL;
	entry = wpa_get_entry(mac);
	if (entry) {
		eloop_cancel_timeout(wpa_start_4way_handshake, (void *)entry->peer_addr, NULL);
		if (entry->wpa_sm) {
			eloop_cancel_timeout(wpa_sm_step, (void *)entry->wpa_sm, NULL);
		}
	}
}


void wpa_deinit()
{
	struct wpa_parameter *wpa_param = &wpa_global;
	struct wpa_entry_info *entry, *entry_next;

	eloop_cancel_timeout(resend_gtk_timer, (void *)wpa_param, NULL);
	dl_list_for_each_safe(entry, entry_next, &wpa_param->entry_list, struct wpa_entry_info, list) {
		eloop_cancel_timeout(wpa_start_4way_handshake, (void *)entry->peer_addr, NULL);
		if (entry->wpa_sm) {
			eloop_cancel_timeout(wpa_sm_step, (void *)entry->wpa_sm, NULL);
			/* free group buf of sm */
			if (entry->wpa_sm->group) {
				os_free(entry->wpa_sm->group);
			}
			/* free sm for each entry */
			os_free(entry->wpa_sm);
		}

		dl_list_del(&entry->list);
		os_free(entry);
	}

	/* free wpa ie */
	if (wpa_param->wpa_ie) {
		os_free(wpa_param->wpa_ie);
	}
	debug(DEBUG_ERROR, "success\n");
}

