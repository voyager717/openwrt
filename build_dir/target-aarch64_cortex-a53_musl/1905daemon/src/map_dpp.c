#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>
#include <linux/types.h>
#include "common.h"
#include "p1905_managerd.h"
#include "p1905_ap_autoconfig.h"
#include "cmdu_message.h"
#include "cmdu.h"
#include "multi_ap.h"
#include "_1905_lib_io.h"
#include "debug.h"
#include "eloop.h"
#include "topology.h"
#include "map_dpp.h"
#include "ieee802_11_defs.h"
#include "dpp.h"
#include "wpabuf.h"
#include "multi_ap.h"
#include "wpa_extern.h"
#include "security_engine.h"

#define DPP_KEYS_FILE "/etc/map/1905_dpp_keys.txt"
extern char * map_config_get_line(char *s,
	int size, FILE *stream, int *line, char **_pos);

struct dl_list r3_member_head;
/* Use a hardcoded Transaction ID 1 in Peer Discovery frames since there is only
 * a single transaction in progress at any point in time. */
static const u8 TRANSACTION_ID = 1;

void map_set_peer_dpp_sm_state(struct r3_information *r3_info, dpp_state cur_dpp_state);
void map_dpp_intro_sm_step(void *eloop_ctx, void *timeout_ctx);


/* replace if existed, append if not existed */
int map_dpp_save_config(char *param, char *value)
{
	char cmd[2048] = {0};
	int ret = 0;

	os_memset(cmd, 0, sizeof(cmd));
	ret = os_snprintf(cmd, sizeof(cmd),
		"cat %s | grep %s && sed -i /^%s=/c\\%s=\'%s\' %s || echo %s=\'%s\' >> %s"
		, DPP_KEYS_FILE, param, param, param, value, DPP_KEYS_FILE, param, value, DPP_KEYS_FILE);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		return -1;
	}

	system(cmd);
	debug(DEBUG_ERROR, "----------------------DPP SAVE CMD start---------------------------\n");
	debug(DEBUG_ERROR, "%s\n", cmd);
	debug(DEBUG_ERROR, "----------------------DPP SAVE CMD end---------------------------\n");
	return 0;
}


void map_dpp_save_keys(struct r3_dpp_information *dpp)
{
	char *strbuf = NULL;
	unsigned short strbuf_len = 512;

	strbuf = os_zalloc(strbuf_len);
	if (!strbuf) {
		debug(DEBUG_ERROR, "malloc buf error\n");
		return;
	}

	if (dpp) {
		if (dpp->connector)
			map_dpp_save_config("connector", dpp->connector);

		os_memset(strbuf, 0, strbuf_len);
		if (dpp->netAccessKey) {
			wpa_snprintf_hex(strbuf, strbuf_len, dpp->netAccessKey, dpp->netAccessKey_len);
			map_dpp_save_config("netAccessKey", strbuf);
		}

		os_memset(strbuf, 0, strbuf_len);
		if (dpp->CsignKey) {
			wpa_snprintf_hex(strbuf, strbuf_len, dpp->CsignKey, dpp->CsignKey_len);
			map_dpp_save_config("CsignKey", strbuf);
		}
	}

	if (strbuf)
		os_free(strbuf);

	return;
}


void map_dpp_save_bss_connector(struct bss_connector_item *bc)
{
	char almac_str[32] = {0};
	char fh_key[64] = {0};
	char bh_key[64] = {0};
	int ret = 0;

	if (!bc)
		return;

	ret = os_snprintf(almac_str, sizeof(almac_str), "%02x%02x%02x%02x%02x%02x",
		bc->almac[0], bc->almac[1], bc->almac[2], bc->almac[3],
		bc->almac[4],bc->almac[5]);
	if (os_snprintf_error(sizeof(almac_str), ret)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		return;
	}
	ret = os_snprintf(fh_key, sizeof(fh_key), "fh_connector-%s", almac_str);
	if (os_snprintf_error(sizeof(fh_key), ret)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		return;
	}
	ret = os_snprintf(bh_key, sizeof(bh_key), "bh_connector-%s", almac_str);
	if (os_snprintf_error(sizeof(bh_key), ret)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		return;
	}

	if (bc->fh_connector)
		map_dpp_save_config(fh_key, bc->fh_connector);

	if (bc->bh_connector)
		map_dpp_save_config(bh_key, bc->bh_connector);

	return;
}


void map_dpp_read_bss_connector(struct p1905_managerd_ctx *ctx, char *pos, unsigned char is_fh)
{
	struct bss_connector_item *bc_item = NULL, *bc_item_nxt = NULL;
	struct bss_connector_item bc = {0};
	int found = 0;
	unsigned char all_zero_mac[ETH_ALEN] = {0};
	char *ori_token = NULL, *token = NULL, *mac_token = NULL, *bss_connetor_token = NULL;

	os_memset(&bc, 0, sizeof(bc));
	os_memset(&all_zero_mac, 0, sizeof(all_zero_mac));
	found = 0;

	/* fh_connector-almac || bh_connector-almac */
	ori_token = token = strtok(pos, "=");
	if (!token) {
		return;
	}

	/* fh_connector|bh_connector value */
	bss_connetor_token = strtok(NULL, "=");
	if (!bss_connetor_token) {
		debug(DEBUG_ERROR, "no connector specified\n");
		return;
	}

	if (is_fh) {
		bc.fh_connector_len = os_strlen(bss_connetor_token);
		bc.fh_connector = bss_connetor_token;
	}
	else {
		bc.bh_connector_len = os_strlen(bss_connetor_token);
		bc.bh_connector = bss_connetor_token;
	}

	/* string: fh_connector */
	token = strtok(ori_token, "-");
	if (!token) {
		debug(DEBUG_ERROR, "no any mac specified\n");
		return;
	}

	/* almac in key string */
	mac_token = strtok(NULL, "-");
	if (!mac_token) {
		debug(DEBUG_ERROR, "no almac specified\n");
		return;
	}
	hexstr2bin(mac_token, bc.almac, ETH_ALEN);

	dl_list_for_each_safe(bc_item, bc_item_nxt,
		&ctx->r3_dpp.bss_connector_list, struct bss_connector_item, member) {
		debug(DEBUG_ERROR, "bc_item almac "MACSTR"\n", MAC2STR(bc_item->almac));
		if (os_memcmp(bc_item->almac, bc.almac, ETH_ALEN) == 0) {
			if (is_fh) {
				bc_item->fh_connector_len = bc.fh_connector_len;
				if (bc_item->fh_connector) {
					os_free(bc_item->fh_connector);
				}

				bc_item->fh_connector = os_zalloc(bc.fh_connector_len + 1);
				if (!bc_item->fh_connector) {
					debug(DEBUG_ERROR, "malloc fh_connector error\n");
					return;
				}
				debug(DEBUG_ERROR, "load fh connector for almac "MACSTR"\n", MAC2STR(bc_item->almac));
				os_memcpy(bc_item->fh_connector, bc.fh_connector, bc.fh_connector_len);
				debug(DEBUG_ERROR, "fh_connector:\n%s\n", bc_item->fh_connector);
			}
			else {
				bc_item->bh_connector_len = bc.bh_connector_len;
				if (bc_item->bh_connector) {
					os_free(bc_item->bh_connector);
				}

				bc_item->bh_connector = os_zalloc(bc.bh_connector_len + 1);
				if (!bc_item->bh_connector) {
					debug(DEBUG_ERROR, "malloc bh_connector error\n");
					return;
				}
				debug(DEBUG_ERROR, "load bh connector for almac "MACSTR"\n", MAC2STR(bc_item->almac));
				os_memcpy(bc_item->bh_connector, bc.bh_connector, bc.bh_connector_len);
				debug(DEBUG_ERROR, "bh_connector:\n%s\n", bc_item->bh_connector);
			}

			found = 1;
		}
	}

	/* add new one */
	if (!found) {
		bc_item = os_zalloc(sizeof(struct bss_connector_item));
		if (!bc_item) {
			debug(DEBUG_ERROR, "malloc bss_connector_item error\n");
			return;
		}

		if (is_fh) {
			bc_item->fh_connector = os_zalloc(bc.fh_connector_len + 1);
			if (!bc_item->fh_connector) {
				os_free(bc_item);
				debug(DEBUG_ERROR, "malloc fh_connector error\n");
				return;
			}
		}
		else {
			bc_item->bh_connector = os_zalloc(bc.bh_connector_len + 1);
			if (!bc_item->bh_connector) {
				os_free(bc_item);
				debug(DEBUG_ERROR, "malloc bh_connector error\n");
				return;
			}
		}

		os_memcpy(bc_item->mac_apcli, (unsigned char *)bc.mac_apcli, ETH_ALEN);
		os_memcpy(bc_item->almac, (unsigned char *)bc.almac, ETH_ALEN);

		if (is_fh) {
			debug(DEBUG_ERROR, "load fh connector for almac "MACSTR"\n", MAC2STR(bc_item->almac));
			bc_item->fh_connector_len = bc.fh_connector_len;
			os_memcpy(bc_item->fh_connector, bc.fh_connector, bc.fh_connector_len);
			debug(DEBUG_ERROR, "fh_connector:\n%s\n", bc_item->fh_connector);
		}
		else {
			debug(DEBUG_ERROR, "load bh connector for almac "MACSTR"\n", MAC2STR(bc_item->almac));
			bc_item->bh_connector_len = bc.bh_connector_len;
			os_memcpy(bc_item->bh_connector, bc.bh_connector, bc.bh_connector_len);
			debug(DEBUG_ERROR, "bh_connector:\n%s\n", bc_item->bh_connector);
		}

		dl_list_add(&ctx->r3_dpp.bss_connector_list, &bc_item->member);
	}

	return;
}


void map_dpp_read_keys(struct p1905_managerd_ctx *ctx)
{
	FILE *file;
	char *buf = NULL, *pos, *token;
	char *tmpbuf = NULL;
	int line = 0;
	int buf_len = 2048;
	int ret = 0;

	if (os_strlen(ctx->dpp_keys_file) == 0) {
		ret = os_snprintf(ctx->dpp_keys_file, sizeof(ctx->dpp_keys_file), "%s", DPP_KEYS_FILE);
		if (os_snprintf_error(sizeof(ctx->dpp_keys_file), ret)) {
			debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
			return;
		}
	}

	debug(DEBUG_ERROR, "open MAP keys file (%s)\n", ctx->dpp_keys_file);

	file = fopen(ctx->dpp_keys_file, "r");
	if (!file) {
		debug(DEBUG_ERROR, "open MAP keys file (%s) fail\n", ctx->dpp_keys_file);
		return;
	}

	buf = os_zalloc(buf_len);
	tmpbuf = os_zalloc(buf_len);
	if (!buf || !tmpbuf)
		goto error;

	while (map_config_get_line(buf, buf_len, file, &line, &pos)) {
		if (os_strlen(pos) < buf_len) {
			ret = os_snprintf(tmpbuf, buf_len, "%s", pos);
			if (os_snprintf_error(buf_len, ret)) {
				debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
				goto error;
			}
		}

		token = strtok(pos, "=");
		if (token != NULL) {
			if (os_strcmp(token, "connector") == 0) {
				token = strtok(NULL, "");
				if (!token) {
					debug(DEBUG_ERROR, "no connector specified\n");
					continue;
				}
				ctx->r3_dpp.connector_len = os_strlen(token);
				if (ctx->r3_dpp.connector)
					os_free(ctx->r3_dpp.connector);
				ctx->r3_dpp.connector = os_zalloc(ctx->r3_dpp.connector_len + 1);
				if (!ctx->r3_dpp.connector) {
					debug(DEBUG_ERROR, "malloc connector space fail\n");
					goto error;
				}
				os_memcpy(ctx->r3_dpp.connector, token, ctx->r3_dpp.connector_len);
				hex_dump("connector from file", (unsigned char *)ctx->r3_dpp.connector, ctx->r3_dpp.connector_len);
			}
			else if (os_strcmp(token, "CsignKey") == 0) {
				token = strtok(NULL, "");
				if (!token) {
					debug(DEBUG_ERROR, "no CsignKey specified\n");
					continue;
				}
				ctx->r3_dpp.CsignKey_len = os_strlen(token)/2;
				if (ctx->r3_dpp.CsignKey)
					os_free(ctx->r3_dpp.CsignKey);
				ctx->r3_dpp.CsignKey = os_zalloc(ctx->r3_dpp.CsignKey_len+ 1);
				if (!ctx->r3_dpp.CsignKey) {
					debug(DEBUG_ERROR, "malloc CsignKey space fail\n");
					goto error;
				}
				hexstr2bin(token, ctx->r3_dpp.CsignKey, ctx->r3_dpp.CsignKey_len);
				hex_dump("CsignKey from file", ctx->r3_dpp.CsignKey, ctx->r3_dpp.CsignKey_len);
			}
			else if (os_strcmp(token, "netAccessKey") == 0) {
				token = strtok(NULL, "");
				if (!token) {
					debug(DEBUG_ERROR, "no netAccessKey specified\n");
					continue;
				}
				ctx->r3_dpp.netAccessKey_len = os_strlen(token)/2;
				if (ctx->r3_dpp.netAccessKey)
					os_free(ctx->r3_dpp.netAccessKey);
				ctx->r3_dpp.netAccessKey = os_zalloc(ctx->r3_dpp.netAccessKey_len + 1);
				if (!ctx->r3_dpp.netAccessKey) {
					debug(DEBUG_ERROR, "malloc netAccessKey space fail\n");
					goto error;
				}
				hexstr2bin(token, ctx->r3_dpp.netAccessKey, ctx->r3_dpp.netAccessKey_len);
				hex_dump("netAccessKey from file", ctx->r3_dpp.netAccessKey, ctx->r3_dpp.netAccessKey_len);

			}
			else if (os_strncmp(token, "fh_connector", os_strlen("fh_connector")) == 0) {
				map_dpp_read_bss_connector(ctx, tmpbuf, 1);
			}
			else if (os_strncmp(token, "bh_connector", os_strlen("bh_connector")) == 0) {
				map_dpp_read_bss_connector(ctx, tmpbuf, 0);
			}
		}
		os_memset(buf, 0, buf_len);
	}

error:
	if (buf)
		os_free(buf);
	if (tmpbuf)
		os_free(tmpbuf);
	(void)fclose(file);

	return;
}





struct r3_member *get_r3_member(unsigned char *al_mac)
{
	struct r3_member *peer = NULL;

	dl_list_for_each(peer, &r3_member_head, struct r3_member, entry) {
		if (!os_memcmp(peer->al_mac, al_mac, ETH_ALEN))
			return peer;
	}

	return NULL;
}

struct r3_member *create_r3_member(unsigned char *al_mac)
{
	struct r3_member *peer = NULL;

	peer = get_r3_member(al_mac);
	if (!peer) {
		peer = (struct r3_member *)os_zalloc(sizeof(struct r3_member));

		if (peer) {
			os_memcpy(peer->al_mac, al_mac, ETH_ALEN);
			peer->active = 1;
			/* default R3 Agent */
			peer->role = AGENT;
			dl_list_add(&r3_member_head, &peer->entry);
		}
	}

	return peer;
}

void delete_r3_member(struct p1905_managerd_ctx *ctx, unsigned char *al_mac)
{
	struct r3_member *peer, *tmp;

	dl_list_for_each_safe(peer, tmp, &r3_member_head, struct r3_member, entry) {
		if (!os_memcmp(peer->al_mac, al_mac, ETH_ALEN)) {
			eloop_cancel_timeout(map_dpp_intro_sm_step, (void *)ctx, (void *)peer);
			eloop_cancel_timeout(wait_send_bss_configuration_result, (void *)ctx, (void *)peer);
			debug(DEBUG_ERROR, "delete entry of "MACSTR"\n", MAC2STR(al_mac));
			dl_list_del(&peer->entry);
			os_free(peer);
			break;
		}
	}
}

extern void reset_radio_config_state(struct p1905_managerd_ctx *ctx);
extern void trigger_auto_config_flow(struct p1905_managerd_ctx *ctx);
/*timeout waiting for dpp auth request*/
void r3_wait_for_dpp_auth_req(void *eloop_ctx, void *timeout_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)eloop_ctx;

	ctx->r3_oboard_ctx.active = 0;

	debug(DEBUG_ERROR, AUTO_CONFIG_PREX"ETH BH, wait dpp auth req timeout, start WSC onboarding\n");
	/*start normal autoconfiguration procedure*/
	reset_radio_config_state(ctx);
	trigger_auto_config_flow(ctx);
}

struct wpabuf * map_dpp_alloc_dpp_msg_tlv(enum dpp_public_action_frame_type type, size_t len)
{
	struct wpabuf *msg;

	msg = wpabuf_alloc(11 + len);
	if (!msg)
		return NULL;
	/* DPP message TLV */
	wpabuf_put_u8(msg, DIRECT_ENCAP_DPP_MESSAGE_TYPE);
	/* tlv len filed */
	wpabuf_put_le16(msg, 0);

	/* skip category, start from action field not category of action frame */
	/* category */
	//wpabuf_put_u8(msg, WLAN_ACTION_PUBLIC);

	/* Action field */
	wpabuf_put_u8(msg, WLAN_PA_VENDOR_SPECIFIC);
	/* OUI */
	wpabuf_put_be24(msg, OUI_WFA);
	/* OUI type */
	wpabuf_put_u8(msg, DPP_OUI_TYPE);
	/* Crypto Suite */
	wpabuf_put_u8(msg, 1);
	/* DPP frame type */
	wpabuf_put_u8(msg, type);
	return msg;
}

int map_dpp_check_connect(struct p1905_managerd_ctx *ctx, unsigned char *al_mac)
{
	unsigned short tlv_len = 0;
	struct os_time now;
	struct wpabuf *msg;
	struct r3_dpp_information *r3_info = &ctx->r3_dpp;
	unsigned char *p = NULL;

	if (!r3_info->connector || !r3_info->netAccessKey ||
		!r3_info->CsignKey) {
		debug(DEBUG_ERROR, "[dpp deer discovery]missing %s\n",
			!r3_info->connector ? "Connector" :
			(!r3_info->netAccessKey ? "netAccessKey" :
			 "C-sign-key"));
		return -1;
	}

	os_get_time(&now);

	/*shall we check if the dpp netaccesskey expired or not*/
#if 0
	if (ssid->dpp_netaccesskey_expiry &&
		(os_time_t) ssid->dpp_netaccesskey_expiry < now.sec) {
		wpa_msg(wpa_s, MSG_INFO, DPP_EVENT_MISSING_CONNECTOR
			"netAccessKey expired");
		return -1;
	}
#endif

	debug(DEBUG_ERROR,
			"[dpp deer discovery]: Starting deriving PMKSA for "
			MACSTR"\n", MAC2STR(al_mac));

	/* GAS hdr: 8
	** Transaction ID: 5
	** connector: 4 + connector_len
	*/
	tlv_len = 8 + 5 + 4 + r3_info->connector_len;
	msg = map_dpp_alloc_dpp_msg_tlv(DPP_PA_PEER_DISCOVERY_REQ, tlv_len);
	if (!msg)
		return -1;

	/* Transaction ID */
	wpabuf_put_le16(msg, DPP_ATTR_TRANSACTION_ID);
	wpabuf_put_le16(msg, 1);
	wpabuf_put_u8(msg, TRANSACTION_ID);

	/* DPP Connector */
	wpabuf_put_le16(msg, DPP_ATTR_CONNECTOR);
	wpabuf_put_le16(msg, r3_info->connector_len);

	wpabuf_put_data(msg, r3_info->connector, r3_info->connector_len);

	p = wpabuf_mhead_u8(msg);
	p++;

	/*fill tlv length*/
	*((unsigned short *)p) = host_to_be16(wpabuf_len(msg) - 3);

	/*send to peer device*/
	reset_send_tlv(ctx);
	if (fill_send_tlv(ctx, wpabuf_mhead(msg), wpabuf_len(msg)) == -1) {
		debug(DEBUG_ERROR, "[%d]fill_send_tlv() fail!\n", __LINE__);
		wpabuf_free(msg);
		return -1;
	}
	insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_direct_encap_dpp,
		++ctx->mid, ctx->br_name, 0);

	process_cmdu_txq(ctx, ctx->trx_buf.buf);
	wpabuf_free(msg);

	return 1;
}

static int map_dpp_rx_peer_disc_resp(struct p1905_managerd_ctx *ctx,
				u8 *src,
				u8 *buf, size_t len, struct r3_member *peer)
{
	const u8 *connector, *trans_id, *status;
	u16 connector_len, trans_id_len, status_len;
	struct dpp_introduction intro;
	os_time_t expiry;
	enum dpp_status_error res;
	struct r3_dpp_information *r3_info = &ctx->r3_dpp;

	debug(DEBUG_ERROR, "[dpp deer discovery]: Response from " MACSTR"\n",
		MAC2STR(src));

	if (!r3_info->connector || !r3_info->netAccessKey||
		!r3_info->CsignKey) {
		debug(DEBUG_ERROR,
			"[dpp deer discovery]: Profile not found for network introduction\n");
		return -1;
	}

	trans_id = dpp_get_attr(buf, len, DPP_ATTR_TRANSACTION_ID,
		&trans_id_len);
	if (!trans_id || trans_id_len != 1) {
		debug(DEBUG_ERROR,
			"[dpp deer discovery]: Peer did not include Transaction ID\n");
		goto fail;
	}
	if (trans_id[0] != TRANSACTION_ID) {
		debug(DEBUG_ERROR,
			"[dpp deer discovery]: Ignore frame with unexpected Transaction ID %u\n", trans_id[0]);
		goto fail;
	}

	status = dpp_get_attr(buf, len, DPP_ATTR_STATUS, &status_len);
	if (!status || status_len != 1) {
		debug(DEBUG_ERROR, "[dpp deer discovery]: Peer did not include Status\n");
		goto fail;
	}
	if (status[0] != DPP_STATUS_OK) {
		debug(DEBUG_ERROR,
			"[dpp deer discovery]: Peer rejected network introduction: Status %u\n", status[0]);
		goto fail;
	}

	connector = dpp_get_attr(buf, len, DPP_ATTR_CONNECTOR, &connector_len);
	if (!connector) {
		debug(DEBUG_ERROR,
			"[dpp deer discovery]: Peer did not include its Connector\n");
		goto fail;
	}

	res = dpp_peer_intro(&intro, r3_info->connector,
			 r3_info->netAccessKey,
			r3_info->netAccessKey_len,
			r3_info->CsignKey,
			r3_info->CsignKey_len,
			connector, connector_len, &expiry);
	if (res != DPP_STATUS_OK) {
		goto fail;
	}

	os_memcpy(peer->r3_info.pmk, intro.pmk, intro.pmk_len);
	os_memcpy(peer->r3_info.pmkid, intro.pmkid, PMKID_LEN);
	peer->r3_info.pmk_len = intro.pmk_len;

	map_set_peer_dpp_sm_state(&peer->r3_info, dpp_disc_suc);

	debug(DEBUG_ERROR, "\033[1;31m[dpp deer discovery]\033[0\n"
	"***********************************************\n"
	"**********DPP PEER Discovery Procedure*********\n"
	"*************<----success with "MACSTR"---->***********\n", MAC2STR(src));

	return 0;
fail:
	map_set_peer_dpp_sm_state(&peer->r3_info, dpp_disc_fail);
	debug(DEBUG_ERROR,
			"[dpp deer discovery]:fail(peer "MACSTR")\n", MAC2STR(src));
	return -1;
}

static void map_dpp_send_peer_disc_resp(struct p1905_managerd_ctx *ctx,
					u8 *src, u8 trans_id,
					enum dpp_status_error status,
					struct r3_member *peer)
{
	struct wpabuf *msg;
	struct r3_dpp_information *r3_info = &ctx->r3_dpp;
	unsigned short tlv_len = 0;
	unsigned char *p = NULL;

	/* GAS hdr: 8
	** Transaction ID: 5
	** DPP status: 5
	** connector: 4 + connector_len
	*/

	tlv_len = 8 + 5 + 5 + 4 + r3_info->connector_len;
	msg = map_dpp_alloc_dpp_msg_tlv(DPP_PA_PEER_DISCOVERY_RESP, tlv_len);
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
		wpabuf_put_le16(msg, os_strlen(r3_info->connector));
		wpabuf_put_str(msg, r3_info->connector);
	}

	debug(DEBUG_ERROR, "[dpp deer discovery]: Send Response to " MACSTR
		" status=%d\n", MAC2STR(src), status);

	p = wpabuf_mhead_u8(msg);
	p++;

	/*fill tlv length*/
	*((unsigned short *)p) = host_to_be16(wpabuf_len(msg) - 3);

	/*send to peer device*/
	reset_send_tlv(ctx);
	if (fill_send_tlv(ctx, wpabuf_mhead(msg), wpabuf_len(msg)) == -1)
		debug(DEBUG_ERROR, "[%d]fill_send_tlv fail!\n", __LINE__);
	insert_cmdu_txq(src, ctx->p1905_al_mac_addr, e_direct_encap_dpp,
		++ctx->mid, ctx->br_name, 0);
	process_cmdu_txq(ctx, ctx->trx_buf.buf);

	wpabuf_free(msg);
}

static void map_dpp_rx_peer_disc_req(struct p1905_managerd_ctx *ctx,
					 u8 *src,
					 u8 *buf, size_t len,
					 struct r3_member *peer)
{
	const u8 *connector, *trans_id;
	u16 connector_len, trans_id_len;
	struct os_time now;
	struct dpp_introduction intro;
	os_time_t expire;
	enum dpp_status_error res;
	struct r3_dpp_information *r3_info = &ctx->r3_dpp;

	debug(DEBUG_ERROR, "[dpp deer discovery] request from " MACSTR"\n",MAC2STR(src));

	if (!r3_info->connector || !r3_info->netAccessKey||
		!r3_info->CsignKey) {
		debug(DEBUG_ERROR, "[dpp deer discovery]: No own Connector/keys set\n");
		goto fail;
	}

	os_get_time(&now);

	/*shall we check if the dpp netaccesskey expired or not*/
#if 0
	if (hapd->conf->dpp_netaccesskey_expiry &&
		(os_time_t) hapd->conf->dpp_netaccesskey_expiry < now.sec) {
		wpa_printf(MSG_INFO, "DPP: Own netAccessKey expired");
		return;
	}
#endif

	trans_id = dpp_get_attr(buf, len, DPP_ATTR_TRANSACTION_ID,
				&trans_id_len);
	if (!trans_id || trans_id_len != 1) {
		debug(DEBUG_ERROR,
			"[dpp deer discovery]: Peer did not include Transaction ID\n");
		goto fail;
	}

	connector = dpp_get_attr(buf, len, DPP_ATTR_CONNECTOR, &connector_len);
	if (!connector) {
		debug(DEBUG_ERROR,
			"[dpp deer discovery]: Peer did not include its Connector\n");
		goto fail;
	}

	res = dpp_peer_intro(&intro, r3_info->connector,
			r3_info->netAccessKey,
			r3_info->netAccessKey_len,
			r3_info->CsignKey,
			r3_info->CsignKey_len,
			connector, connector_len, &expire);
	if (res == 255) {
		debug(DEBUG_ERROR,
			"[dpp deer discovery]: internal failure (peer "
			MACSTR ")\n", MAC2STR(src));
		goto fail;
	}
	if (res != DPP_STATUS_OK) {
		debug(DEBUG_ERROR,
			"[dpp deer discovery]: failure (peer "
			MACSTR " status %d)\n", MAC2STR(src), res);
		map_dpp_send_peer_disc_resp(ctx, src, trans_id[0],
						res, peer);
		goto fail;
	}

	/*store pmk and pmk_len for a specific peer device*/
	if (intro.pmk_len > sizeof(peer->r3_info.pmk)) {
		debug(DEBUG_ERROR, "pmk len too long, reset to %d\n", PMK_LEN_MAX);
		intro.pmk_len = PMK_LEN_MAX;
	}
	os_memcpy(peer->r3_info.pmk, intro.pmk, intro.pmk_len);
	os_memcpy(peer->r3_info.pmkid, intro.pmkid, PMKID_LEN);
	peer->r3_info.pmk_len = intro.pmk_len;

	map_dpp_send_peer_disc_resp(ctx, src, trans_id[0],
					DPP_STATUS_OK, peer);

	map_set_peer_dpp_sm_state(&peer->r3_info, dpp_disc_suc);
	debug(DEBUG_ERROR,
			"[dpp deer discovery]: success(peer "
			MACSTR ")\n", MAC2STR(src));
	return;
fail:
	map_set_peer_dpp_sm_state(&peer->r3_info, dpp_disc_fail);
	debug(DEBUG_ERROR,
			"[dpp deer discovery]: resulted in fail\n");
	return;
}

void map_set_peer_dpp_sm_state(struct r3_information *r3_info, dpp_state cur_dpp_state)
{
	r3_info->cur_dpp_state = cur_dpp_state;
	debug(DEBUG_ERROR,
			"[dpp deer discovery]: dpp state change to %d\n", (int)cur_dpp_state);
}


void map_dpp_intro_sm(struct p1905_managerd_ctx *ctx, struct r3_member *peer)
{
	switch(peer->r3_info.cur_dpp_state) {
		case dpp_send_dpp_disc_req:
		{
			debug(DEBUG_ERROR, "[dpp deer discovery]send dpp_send_dpp_disc_req\n");
			map_dpp_check_connect(ctx, peer->al_mac);
			map_set_peer_dpp_sm_state(&peer->r3_info, dpp_wait_dpp_disc_resp);
			eloop_cancel_timeout(map_dpp_intro_sm_step, (void *)ctx, (void *)peer);
			eloop_register_timeout(5, 0, map_dpp_intro_sm_step, (void *)ctx, (void *)peer);
		}
			break;
		case dpp_wait_dpp_disc_resp:
		{
			if (peer->r3_info.retry_cnt < 3) {
				debug(DEBUG_ERROR, "[dpp deer discovery]retry %d dpp discover req\n",
					peer->r3_info.retry_cnt+1);
				map_dpp_check_connect(ctx, peer->al_mac);
				map_set_peer_dpp_sm_state(&peer->r3_info, dpp_wait_dpp_disc_resp);
				eloop_cancel_timeout(map_dpp_intro_sm_step, (void *)ctx, (void *)peer);
				eloop_register_timeout(5, 0, map_dpp_intro_sm_step, (void *)ctx, (void *)peer);
				peer->r3_info.retry_cnt ++;
			}
			else {
				debug(DEBUG_ERROR, "[dpp deer discovery]retry timeout with "MACSTR"\n", MAC2STR(peer->al_mac));
				map_set_peer_dpp_sm_state(&peer->r3_info, dpp_disc_fail);
				peer->r3_info.retry_cnt = 0;
				ctx->r3_oboard_ctx.bss_renew = 0;

				if (peer->role == CONTROLLER) {
					debug(DEBUG_ERROR, "[dpp deer discovery]start autoconfig search procedure\n");
					ctx->r3_oboard_ctx.onboarding_stage = R3_ONBOARDING_STAGE_INVALID;
					eloop_cancel_timeout(ap_controller_search_step, (void *)ctx, NULL);

					/* timeout, trigger autoconfig search procedure */
					ctx->enrolle_state = wait_4_send_controller_search;
					eloop_register_timeout(RECV_CONFIG_RSP_TIME, 0, ap_controller_search_step, (void *)ctx, NULL);
				}
			}
		}
			break;
		default:
			break;
	}
}

void map_dpp_intro_sm_step(void *eloop_ctx, void *timeout_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;
	struct r3_member *peer = (struct r3_member *)timeout_ctx;

	map_dpp_intro_sm(ctx, peer);

	process_cmdu_txq(ctx, ctx->trx_buf.buf);
}

void map_dpp_trigger_all_member_dpp_intro(struct p1905_managerd_ctx *ctx)
{
	struct r3_member *peer = NULL;

	/* DPP Discovery with other agents */
	dl_list_for_each(peer, &r3_member_head, struct r3_member, entry) {
		debug(DEBUG_ERROR, "[dpp deer discovery] peer "MACSTR" role %d\n", MAC2STR(peer->al_mac), peer->role);
		if ((0 == os_memcmp(peer->al_mac, ctx->p1905_al_mac_addr, ETH_ALEN)) ||
			(0 == os_memcmp(peer->al_mac, ctx->cinfo.almac, ETH_ALEN)))
			continue;
		else {
			if (peer->profile < MAP_PROFILE_R3) {
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX"peer("MACSTR") not R3 dev\n",
						MAC2STR(peer->al_mac));
				continue;
			}
			map_set_peer_dpp_sm_state(&peer->r3_info, dpp_idle);
			debug(DEBUG_ERROR, "[dpp deer discovery] with agent "MACSTR"\n", MAC2STR(peer->al_mac));
			map_dpp_trigger_member_dpp_intro(ctx, peer);
		}
	}
	ctx->r3_oboard_ctx.active = 0;
}


void map_cancel_onboarding_timer(struct p1905_managerd_ctx *ctx, struct r3_member *peer)
{
	eloop_cancel_timeout(map_dpp_intro_sm_step, (void *)ctx, (void *)peer);
	wpa_eloop_cancel_entry_timeout(peer->al_mac);
	eloop_cancel_timeout(r3_wait_for_dpp_auth_req, (void *)ctx, NULL);
	eloop_cancel_timeout(r3_config_sm_step, (void *)ctx, NULL);
	eloop_cancel_timeout(r3_set_config, (void *)ctx, NULL);
}


void map_dpp_trigger_member_dpp_intro(struct p1905_managerd_ctx *ctx, struct r3_member *peer)
{
	/*
	** do not run DPP Peer Discovery if a 1905 PTK is available,
	** if a 1905 PTK is available, and the bSTA associates/disassociates/reassociates,
	** that should not impact the 1905 layer
	*/
	dpp_state cur_dpp_state = peer->r3_info.cur_dpp_state;

	if (peer->active && (!(cur_dpp_state > dpp_idle && cur_dpp_state < dpp_disc_suc))) {
		map_set_peer_dpp_sm_state(&peer->r3_info, dpp_send_dpp_disc_req);
		eloop_register_timeout(0, 0, map_dpp_intro_sm_step, (void *)ctx, (void *)peer);
	}
}

void map_dpp_handle_public_action_frame(struct p1905_managerd_ctx *ctx,
	unsigned char public_action_type, unsigned char *buf, size_t len,
	unsigned char *al_mac)
{
	struct r3_member *peer = NULL;

	peer = get_r3_member(al_mac);

	if (!peer) {
		peer = create_r3_member(al_mac);
	}

	if (!peer)
		goto error;

	if (!peer->active)
		peer->active = 1;

	switch (public_action_type) {
		case DPP_PA_PEER_DISCOVERY_REQ:
		{
			if (!os_memcmp(al_mac, ctx->cinfo.almac, ETH_ALEN)) {
				ctx->r3_oboard_ctx.onboarding_stage = R3_ONBOARDING_STAGE_PEER_DISCOVERY;
				debug(DEBUG_ERROR, "\033[1;31m[DPP PEER Discovery]\033[0\n"
				"***********************************************\n"
				"**********DPP PEER Discovery start with "MACSTR"*********\n", MAC2STR(al_mac));
			}
			map_dpp_rx_peer_disc_req(ctx, al_mac, buf, len, peer);

			wpa_set_pmk_by_1905(ctx->p1905_al_mac_addr, al_mac, peer->r3_info.pmk, peer->r3_info.pmk_len);

			/*
			** PMK establishment between the Multi-AP Controller and the enrollee
			** Multi-AP Controller shall initiate and perform the 4-way handshake procedures
			*/
			if (ctx->role == CONTROLLER) {
				debug(DEBUG_OFF, "[dpp deer discovery]Controller initial 4-way handshake with Enrolle "MACSTR"\n",
					MAC2STR(al_mac));
				eloop_cancel_timeout(wpa_start_4way_handshake, (void *)peer->al_mac, NULL);
				eloop_register_timeout(1, 0, wpa_start_4way_handshake, (void *)peer->al_mac, NULL);
			}
		}
			break;
		case DPP_PA_PEER_DISCOVERY_RESP:
		{
			if (map_dpp_rx_peer_disc_resp(ctx, al_mac, buf, len, peer)) {
				debug(DEBUG_ERROR, "skip peer discovery procedure\n");
				ctx->r3_oboard_ctx.onboarding_stage = R3_ONBOARDING_STAGE_INVALID;
				break;
			}

			wpa_set_pmk_by_1905(ctx->p1905_al_mac_addr, al_mac, peer->r3_info.pmk, peer->r3_info.pmk_len);

			/*
			** PMK establishment between a Multi-AP Agent and the enrollee
			** The enrollee shall initiate and perform the 4-way handshake procedures
			*/
			dl_list_for_each(peer, &r3_member_head, struct r3_member, entry) {
				if ((!os_memcmp(peer->al_mac, al_mac, ETH_ALEN)) && peer->role == AGENT) {
					debug(DEBUG_OFF, "[dpp deer discovery]Enrollee initial 4-way handshake with Agent "MACSTR"\n",
						MAC2STR(al_mac));
					eloop_cancel_timeout(wpa_start_4way_handshake, (void *)peer->al_mac, NULL);
					eloop_register_timeout(1, 0, wpa_start_4way_handshake, (void *)peer->al_mac, NULL);
					break;
				}
			}
		}
			break;
		default:
			break;
	}

error:
	return;
}

void map_dpp_init(struct p1905_managerd_ctx *ctx)
{
	dl_list_init(&r3_member_head);
	dl_list_init(&ctx->r3_dpp.hash_value_list);
	dl_list_init(&ctx->r3_dpp.bss_connector_list);
	ctx->r3_oboard_ctx.bh_type = MAP_BH_UNKNOWN;
	map_dpp_read_keys(ctx);
}


void timeout_delete_r3_member(void *eloop_data, void *user_ctx)
{
	struct p1905_managerd_ctx *ctx = NULL;
	struct r3_member *peer = (struct r3_member *)user_ctx;
	unsigned char al_mac[ETH_ALEN] = {0};

	ctx = (struct p1905_managerd_ctx *)eloop_data;
	os_memcpy(al_mac, peer->al_mac, ETH_ALEN);

	if (!lookup_tprdb_by_almac(ctx, al_mac)) {
		debug(DEBUG_ERROR, "clear sec info for ("MACSTR")\n", PRINT_MAC(al_mac));
		/* reset my onboarding stage after disconnected from Controller */
		if (peer->role == CONTROLLER)
			ctx->r3_oboard_ctx.onboarding_stage = R3_ONBOARDING_STAGE_INVALID;
		delete_r3_member(ctx, al_mac);
		wpa_clear_entry_by_mac(al_mac);
		sec_entry_clear_by_mac(al_mac);
	}
}

void map_dpp_deinit(struct p1905_managerd_ctx *ctx)
{
	struct r3_member *peer, *tmp;
	struct hash_value_item *hv_item = NULL, *hv_item_nxt = NULL;
	struct bss_connector_item *bc_item = NULL, *bc_item_nxt = NULL;

	dl_list_for_each_safe(peer, tmp, &r3_member_head, struct r3_member, entry) {
		eloop_cancel_timeout(wait_send_bss_configuration_result, (void *)ctx, (void *)peer);
		eloop_cancel_timeout(map_dpp_intro_sm_step, (void *)ctx, (void *)peer);
		eloop_cancel_timeout(timeout_delete_r3_member,
			(void *)ctx, (void *)peer);
		dl_list_del(&peer->entry);
		os_free(peer);
	}

	dl_list_for_each_safe(hv_item, hv_item_nxt,
		&ctx->r3_dpp.hash_value_list, struct hash_value_item, member) {
		dl_list_del(&hv_item->member);
		os_free(hv_item);
	}

	dl_list_for_each_safe(bc_item, bc_item_nxt,
		&ctx->r3_dpp.bss_connector_list, struct bss_connector_item, member) {
		if (bc_item->bh_connector)
			os_free(bc_item->bh_connector);
		if (bc_item->fh_connector)
			os_free(bc_item->fh_connector);
		dl_list_del(&bc_item->member);
		os_free(bc_item);
	}

	if (ctx->r3_dpp.connector) {
		os_free(ctx->r3_dpp.connector);
		ctx->r3_dpp.connector = NULL;
	}

	if (ctx->r3_dpp.CsignKey) {
		os_free(ctx->r3_dpp.CsignKey);
		ctx->r3_dpp.CsignKey = NULL;
	}

	if (ctx->r3_dpp.netAccessKey) {
		os_free(ctx->r3_dpp.netAccessKey);
		ctx->r3_dpp.netAccessKey = NULL;
	}
}


