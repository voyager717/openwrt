/*
 * DPP functionality shared between hostapd and wpa_supplicant
 * Copyright (c) 2017, Qualcomm Atheros, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
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



#ifdef CONFIG_DPP2
#ifdef MAP_R3
#if 0
static void wapp_dpp_relay_tx(void *ctx, struct wapp_dev *wdev,
				const u8 *addr, unsigned int chan,
				 const u8 *msg, size_t len)
{
	struct wifi_app *wapp = ctx;
	u8 *buf;

	if (!wdev)
		wdev = wapp_get_dpp_default_wdev(wapp, chan);

	if (!wdev) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to get default wdev for chan=%u",
			   chan);
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Send action frame dst=" MACSTR " chan=%u",
		   MAC2STR(addr), chan);
	buf = os_malloc(1 + len);
	if (!buf)
		return;
	buf[0] = WLAN_ACTION_PUBLIC;
	os_memcpy(buf + 1, msg, len);
	wapp_drv_send_action(wapp, wdev, chan, 0, addr, buf, 2 + len);
	os_free(buf);
}
#endif
void wapp_dpp_relay_gas_resp_tx(void *ctx, const u8 *addr,
					  u8 dialog_token, int prot,
					  struct wpabuf *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)ctx;
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)addr);
	if(!auth){
		DBGPRINT(RT_DEBUG_ERROR,DPP_MAP_PREX "auth_NULL_return");
		return;
	}

	//TODO Change later when wlan ack part fixed
	auth->current_state = DPP_STATE_CONFIG_RESULT_WAITING;

	gas_serv_req_dpp_processing(wapp->dpp->gas_server, auth->wdev, addr, dialog_token, prot, buf);
}
#endif /* MAP_R3 */


static void dpp_controller_rx(int sd, void *eloop_ctx, void *sock_ctx);
void dpp_conn_tx_ready(int sock, void *eloop_ctx, void *sock_ctx);

int dpp_relay_add_controller(struct dpp_global *dpp,
			     struct dpp_relay_config *config)
{
	struct dpp_relay_controller *ctrl;

	if (!dpp)
		return -1;

	ctrl = os_zalloc(sizeof(*ctrl));
	if (!ctrl)
		return -1;
	dl_list_init(&ctrl->auth);
	ctrl->global = dpp;
	os_memcpy(&ctrl->ipaddr, &config->ipaddr, sizeof(config->ipaddr));
	os_memcpy(ctrl->pkhash, config->pkhash, SHA256_MAC_LEN);
	ctrl->cb_ctx = config->cb_ctx;
	ctrl->tx = config->tx;
	ctrl->gas_resp_tx = config->gas_resp_tx;
	dl_list_add(&dpp->controllers, &ctrl->list);
	return 0;
}

static struct wpabuf * dpp_tcp_encaps(const u8 *hdr, const u8 *buf, size_t len)
{
	struct wpabuf *msg;

	msg = wpabuf_alloc(4 + 1 + DPP_HDR_LEN + len);
	if (!msg)
		return NULL;
	wpabuf_put_be32(msg, 1 + DPP_HDR_LEN + len);
	wpabuf_put_u8(msg, WLAN_PA_VENDOR_SPECIFIC);
	wpabuf_put_data(msg, hdr, DPP_HDR_LEN);
	wpabuf_put_data(msg, buf, len);
	wpa_hexdump_buf(MSG_MSGDUMP, "DPP: Outgoing TCP message", msg);
	return msg;
}

static struct dpp_relay_controller *
dpp_relay_controller_get(struct dpp_global *dpp, const u8 *pkhash)
{
	struct dpp_relay_controller *ctrl;

	if (!dpp)
		return NULL;

	dl_list_for_each(ctrl, &dpp->controllers, struct dpp_relay_controller,
			 list) {
		if (os_memcmp(pkhash, ctrl->pkhash, SHA256_MAC_LEN) == 0)
			return ctrl;
	}

	return NULL;
}

static void dpp_controller_gas_done(struct dpp_authentication *auth)
{
	if (auth->peer_version >= 2 &&
	    auth->conf_resp_status == DPP_STATUS_OK) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Wait for Configuration Result");
		auth->current_state = DPP_STATE_CONFIG_RESULT_WAITING;
		return;
	}

	dpp_auth_deinit(auth);
}

static int dpp_tcp_send(struct dpp_authentication *auth)
{
	int res;

	if (!auth->msg_out) {
		eloop_unregister_sock(auth->sock, EVENT_TYPE_WRITE);
		auth->write_eloop = 0;
		return -1;
	}
	res = send(auth->sock,
		   wpabuf_head_u8(auth->msg_out) + auth->msg_out_pos,
		   wpabuf_len(auth->msg_out) - auth->msg_out_pos, 0);
	if (res < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to send buffer: %s",
			   strerror(errno));
		dpp_auth_deinit(auth);
		return -1;
	}

	auth->msg_out_pos += res;
	if (wpabuf_len(auth->msg_out) > auth->msg_out_pos) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: %u/%u bytes of message sent to Controller",
			   (unsigned int) auth->msg_out_pos,
			   (unsigned int) wpabuf_len(auth->msg_out));
		if (!auth->write_eloop &&
		    eloop_register_sock(auth->sock, EVENT_TYPE_WRITE,
					dpp_conn_tx_ready, auth, NULL) == 0)
			auth->write_eloop = 1;
		return 1;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Full message sent over TCP");
	wpabuf_free(auth->msg_out);
	auth->msg_out = NULL;
	auth->msg_out_pos = 0;
	eloop_unregister_sock(auth->sock, EVENT_TYPE_WRITE);
	auth->write_eloop = 0;
	if (!auth->read_eloop &&
	    eloop_register_sock(auth->sock, EVENT_TYPE_READ,
				dpp_controller_rx, auth, NULL) == 0)
		auth->read_eloop = 1;
	if (auth->on_tcp_tx_complete_remove) {
		dpp_auth_deinit(auth);
		return -1;
	} else if (auth->on_tcp_tx_complete_gas_done) {
		dpp_controller_gas_done(auth);
	} else if (auth->on_tcp_tx_complete_auth_ok) {
		auth->on_tcp_tx_complete_auth_ok = 0;
		wapp_dpp_auth_success(wapp, 1, NULL);
	}

	return 0;
}
enum dpp_public_action_frame_type wapp_get_frame_type(struct dpp_authentication *auth)
{
	switch (auth->current_state) {
	case DPP_STATE_AUTH_RESP_WAITING:
		return DPP_PA_AUTHENTICATION_REQ;
	case DPP_STATE_AUTH_CONF_WAITING:
		return DPP_PA_AUTHENTICATION_RESP;
	case DPP_STATE_CONFIG_REQ_WAITING:
		return DPP_PA_AUTHENTICATION_CONF;
	case DPP_STATE_CONFIG_RESULT_WAITING:
		return DPP_PA_CONFIGURATION_RESULT;
#ifdef MAP_R3_RECONFIG
	case DPP_STATE_RECONFIG_AUTH_RESP_WAITING:
		return DPP_PA_RECONFIG_AUTH_REQ;
	case DPP_STATE_RECONFIG_AUTH_CONF_WAITING:
		return DPP_PA_RECONFIG_AUTH_RESP;
	case DPP_STATE_RECONFIG_CONFIG_REQ_WAITING:
		return DPP_PA_RECONFIG_AUTH_CONF;
#endif
	case DPP_STATE_DEINIT:
	case DPP_STATE_CONFIG_RSP_WAITING:
	case DPP_STATE_CONFIG_DONE:
	default:
		return DPP_PA_UNDEFINED_FRAME;
	}
}

//Prakhar
#ifdef MAP_R3
unsigned char almac_broad[6]={ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
void ChirpMsg_1905_send(struct wifi_app *wapp, u8 *chirp_hash, u8 *peer_mac
#ifdef MAP_R3_RECONFIG
	,unsigned char reconfigure
#endif
	, unsigned char hash_validity, u8 * al_mac)
{
	struct chirp_info *chirp_tlv;
	int chirp_cnt = 1;
	int len = 0;
	unsigned char almac[ETH_ALEN] = {0};

	len = sizeof(struct chirp_info) + (chirp_cnt * sizeof (struct chirp_tlv_info));

	chirp_tlv = os_zalloc(len);

	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"\n In chirpmsg_1905 send func");

	os_memcpy(chirp_tlv->almac, almac, MAC_ADDR_LEN);
	chirp_tlv->chirp_cnt = chirp_cnt;
#ifdef MAP_R3_RECONFIG
	if (reconfigure == 1) {
		chirp_tlv->item->hash_len = 0; 
		chirp_tlv->item->hash_validity = 0; //value should be 1
		chirp_tlv->item->enrollee_mac_address_present = 1;
		if (peer_mac)
			os_memcpy(chirp_tlv->item->enrollee_mac, peer_mac, MAC_ADDR_LEN);
		if (al_mac != NULL)
			os_memcpy(chirp_tlv->almac, al_mac, MAC_ADDR_LEN);
		else
			os_memcpy(chirp_tlv->almac, almac_broad, MAC_ADDR_LEN);
	} else
#endif 
	{
		if(os_memcmp(wapp->dpp->almac_cont, almac, MAC_ADDR_LEN) == 0)
			os_memcpy(chirp_tlv->almac, almac_broad, MAC_ADDR_LEN);
		else
			os_memcpy(chirp_tlv->almac, wapp->dpp->almac_cont, MAC_ADDR_LEN);
		if (al_mac != NULL)
			os_memcpy(chirp_tlv->almac, al_mac, MAC_ADDR_LEN);
		os_memcpy(chirp_tlv->item->hash_payload, chirp_hash, SHA256_MAC_LEN);
		hex_dump_dbg(DPP_MAP_PREX"Chirp TLV dump= ",(UCHAR *)chirp_tlv->item->hash_payload, SHA256_MAC_LEN);
		chirp_tlv->item->hash_len = SHA256_MAC_LEN; 
		chirp_tlv->item->hash_validity = hash_validity;
		chirp_tlv->item->enrollee_mac_address_present = 0;
		if (peer_mac) {
			chirp_tlv->item->enrollee_mac_address_present = 1;
			os_memcpy(chirp_tlv->item->enrollee_mac, peer_mac, MAC_ADDR_LEN);
		}
		//chirp_tlv.hash_type = 0;
	}

	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"\n hash len - %d, hash_validity - %d",chirp_tlv->item->hash_len,
			chirp_tlv->item->hash_validity);

	hex_dump_dbg(DPP_MAP_PREX"total dump= ",(UCHAR *)chirp_tlv, len);
	if (wapp_send_1905_msg(wapp, WAPP_SEND_CHIRP_NEW_MSG,
				len, (char *)chirp_tlv) < 0)
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed\n");

	os_free(chirp_tlv);

	return;
}

void ChirpTLV_1905_send(struct wifi_app *wapp, struct dpp_bootstrap_info *bi)
{
	struct chirp_info *chirp_tlv;
	int chirp_cnt = 1;
	int len = 0, i =0;
  
	len = sizeof(struct chirp_info) + (chirp_cnt * sizeof (struct chirp_tlv_info));
  
	chirp_tlv = os_zalloc(len);
	if(!chirp_tlv) {
                DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"malloc failed\n");
				return;
        }

	        DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"\n In chiprtlv_1905 send func len - %d", len);


        chirp_tlv->chirp_cnt = 1;

        for(i=0; i<chirp_tlv->chirp_cnt; i++) {
                os_memcpy(&chirp_tlv->item[i].hash_payload[0], bi->chirp_hash, SHA256_MAC_LEN);
                hex_dump_dbg(DPP_MAP_PREX"Chirp Hash dump= ",(UCHAR *)chirp_tlv->item[i].hash_payload, SHA256_MAC_LEN);

                chirp_tlv->item[i].hash_len = SHA256_MAC_LEN;
                chirp_tlv->item[i].hash_validity = 1;
		if(wapp->dpp->dpp_configurator_supported && !is_zero_ether_addr(bi->mac_addr)) {
			chirp_tlv->item[i].enrollee_mac_address_present = 1;
			os_memcpy(chirp_tlv->item[i].enrollee_mac, bi->mac_addr, MAC_ADDR_LEN);
		}
		else
			chirp_tlv->item[i].enrollee_mac_address_present = 0;

                DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"\n hash len - %d, hash_validity - %d",chirp_tlv->item[i].hash_len,
                                chirp_tlv->item[i].hash_validity);

#if 0
                chirp_tlv->item->hash_len = SHA256_MAC_LEN;
                chirp_tlv->item->hash_validity = 1;
                chirp_tlv->item->enrollee_mac_address_present = 0;

                os_memcpy(chirp_tlv->item->hash_payload, bi->chirp_hash, SHA256_MAC_LEN);
                hex_dump_dbg(DPP_MAP_PREX"Chirp TLV dump= ",(UCHAR *)chirp_tlv->item->hash_payload, SHA256_MAC_LEN);
                DBGPRINT(RT_DEBUG_ERROR,"\n hash len - %d, hash_validity - %d",chirp_tlv->item->hash_len,
                         chirp_tlv->item->hash_validity);
#endif
        }

        hex_dump_dbg(DPP_MAP_PREX"Chirp TLV dump= ",(UCHAR *)chirp_tlv, len);

        if (wapp_send_1905_msg(wapp, WAPP_SEND_CHIRP_MSG,
                        len, (char *)chirp_tlv) < 0) {
                DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed\n");
        }
	
	os_free(chirp_tlv);
        return;
}

//unsigned char almac_broad[6]={ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
int CCEIndication_1905_send(struct wifi_app *wapp, unsigned char *almac, unsigned char flag)
{
		//unsigned char almac_zero[6]={0};
		int len = sizeof (struct cce_msg);
	
		struct cce_msg *cce_pkt = os_zalloc(len);

		cce_pkt->cce_flag = flag;

		if (!almac)
			os_memcpy(cce_pkt->almac, almac_broad, ETH_ALEN);
		else
			os_memcpy(cce_pkt->almac, almac, ETH_ALEN);
		
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"\n Send CCE Indication cce_pkt->cce_flag-%d\n",cce_pkt->cce_flag);
		
		if (wapp_send_1905_msg(wapp, WAPP_SEND_CCE_MSG,
			sizeof(*cce_pkt), (char *)cce_pkt) < 0) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed\n");
			os_free(cce_pkt);
			return -1;
		}


		os_free(cce_pkt);
		
		return 0;
}


void dpp_URI_1905_send(struct wifi_app *wapp, unsigned char *almac, struct wapp_dev *wdev,
		unsigned char *src_mac, unsigned short uri_len, unsigned char *rcvd_uri)
{
	struct dpp_uri_msg *dpp_uri_pkt = NULL;
	int len = 0;
	unsigned char zero_almac[ETH_ALEN] = {0};

	len = sizeof(struct dpp_uri_msg) + (int)uri_len;

	dpp_uri_pkt = os_zalloc(len);

	if (!almac)
		os_memcpy(dpp_uri_pkt->almac, almac_broad, ETH_ALEN);

	if(os_memcmp(wapp->dpp->almac_cont, zero_almac, MAC_ADDR_LEN) == 0)
		os_memcpy(dpp_uri_pkt->almac, almac_broad, MAC_ADDR_LEN);
	else
		os_memcpy(dpp_uri_pkt->almac, wapp->dpp->almac_cont, MAC_ADDR_LEN);

	MAP_GET_RADIO_IDNFER(wdev->radio, dpp_uri_pkt->uri_info.identifier);
	os_memcpy(dpp_uri_pkt->uri_info.local_intf_mac, wdev->mac_addr, ETH_ALEN);
	os_memcpy(dpp_uri_pkt->uri_info.sta_mac, src_mac, ETH_ALEN);

	dpp_uri_pkt->len = uri_len;
	os_memcpy(dpp_uri_pkt->uri, rcvd_uri, uri_len);

	hex_dump_dbg(DPP_MAP_PREX"total URI dump= ",(UCHAR *)dpp_uri_pkt, len);

	if (wapp_send_1905_msg(wapp, WAPP_SEND_URI_MSG,
			len, (char *)dpp_uri_pkt) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed\n");
	}

	os_free(dpp_uri_pkt);

	return;
}

static int dpp_direct_1905msg_send(struct dpp_authentication *auth)
{
	struct dpp_direct_msg *dpp_pkt = NULL;
	int len = 0;
	struct wifi_app *wapp = (struct wifi_app *)auth->msg_ctx;

	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"\n Dpp direct msg send");
	
	len = sizeof(struct dpp_direct_msg) + wpabuf_len(auth->msg_out) - auth->msg_out_pos;
		
	dpp_pkt = os_zalloc(len);
			
	os_memcpy(dpp_pkt->almac, auth->peer_mac_addr, ETH_ALEN);				

	dpp_pkt->payload_len = wpabuf_len(auth->msg_out) - auth->msg_out_pos;
	os_memcpy(dpp_pkt->payload, wpabuf_head_u8(auth->msg_out) + auth->msg_out_pos, dpp_pkt->payload_len);
	
	hex_dump_dbg(DPP_MAP_PREX"pkt dump= ",(UCHAR *)dpp_pkt->payload, dpp_pkt->payload_len);
	if (wapp_send_1905_msg(wapp, WAPP_SEND_DPP_DIRECT_MSG,
		sizeof(*dpp_pkt) + dpp_pkt->payload_len, (char *)dpp_pkt) < 0)
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed\n");
	
	os_free(dpp_pkt);
	
	return 0;
}
static int dpp_1905msg_send(struct dpp_authentication *auth)
{
	struct dpp_msg *dpp_pkt = NULL;
	int len = 0;
	struct wifi_app *wapp = (struct wifi_app *)auth->msg_ctx;
	//unsigned char almac_zero[6]={0};	

	//Req 1 - 
	//After receipt of one or more DPP URIs and generating the DPP CCE Indication message, the Multi-AP Controller shall generate a 1905 Encap DPP message containing both aone 1905 Encap DPP TLV and one a DPP Chirp Value TLV pertaining to each received DPP URI. 
	//a.	In the 1905 Encap DPP TLV, the Multi-AP Controller shall set the DPP Frame Indicator to 0, the Frame Type to 0, and the Channel List Present bit to 0. If the Multi-AP Controller knows the MAC address of the Enrollee from the DPP URI, then it shall set the Enrollee MAC Address Present bit field to one and shall set the Destination STA MAC Address field to the MAC address of the Enrollee. If the Multi-AP Controller does not know the MAC address of the Enrollee, the Multi-AP Controller shall set the Enrollee MAC Address Present bit field to zero.
	//b.	In the DPP Chirp Value TLV, the Multi-AP Controller shall set the Hash Type value to 0, the Hash Validity to 1 and the Hash Value to the value computed from the DPP URI as per Section XX of [15]


	// Req - If a Multi-AP Controller receives a 1905 Encap DPP message containing an encapsulated 
	// DPP Authentication Response frame, it shall generate a DPP Authentication Confirm frame as per [15] 
	// and encapsulate it into a 1905 Encap DPP TLV, set the Channel List Present bit to 0, the DPP Frame
	// Indicator bit to 0, Enrollee MAC Address Present bit to 1, the Frame Type field to 2 and include the 
	// Enrollee MAC Address into the Destination STA MAC Address field. The Multi-AP Controller TLV 
	// shall be included the TLV into a 1905 Encap DPP message and sent send it to the Multi-AP Agent 
	// from which the previous 1905 Encap DPP message carrying the DPP Authentication Response frame 
	// was received.
	
	if (auth->allowed_roles == DPP_CAPAB_CONFIGURATOR && 
		((auth->current_state == DPP_STATE_AUTH_RESP_WAITING) ||
		(auth->current_state == DPP_STATE_CONFIG_REQ_WAITING) ||
		(auth->current_state == DPP_STATE_CONFIG_RESULT_WAITING)
#ifdef MAP_R3_RECONFIG
		|| (auth->current_state == DPP_STATE_RECONFIG_AUTH_RESP_WAITING)
		|| (auth->current_state == DPP_STATE_RECONFIG_CONFIG_REQ_WAITING)
#endif
		)) {	
		
		len = sizeof (struct dpp_msg) + wpabuf_len(auth->msg_out) - auth->msg_out_pos;
		
		dpp_pkt = os_zalloc(len);
		if (dpp_pkt == NULL) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"dpp_pkt allocation fail\n");
			return 0;
		}
		
		if(auth->current_state == DPP_STATE_CONFIG_RESULT_WAITING) //TODO change later
			dpp_pkt->dpp_frame_indicator = 1;
		else
			dpp_pkt->dpp_frame_indicator = 0;
		
		dpp_pkt->frame_type = wapp_get_frame_type(auth);		
		dpp_pkt->dpp_info.chn_list_flag = 0;
		if (!is_zero_ether_addr(auth->peer_mac_addr)) {
			dpp_pkt->dpp_info.chn_list_flag = 0;
			dpp_pkt->dpp_info.enrollee_mac_flag = 1;
			os_memcpy(&dpp_pkt->dpp_info.enrollee_mac[0], auth->peer_mac_addr, ETH_ALEN);
		}

		if (auth->current_state == DPP_STATE_AUTH_RESP_WAITING) {
			os_memcpy(dpp_pkt->almac, almac_broad, ETH_ALEN);			
			dpp_pkt->chirp_tlv_present = 1;
			if (auth->chirp_tlv.hash_len)
				os_memcpy(&dpp_pkt->chirp_info.hash_payload[0], &auth->chirp_tlv.hash_payload[0], auth->chirp_tlv.hash_len);

			dpp_pkt->chirp_info.hash_len = auth->chirp_tlv.hash_len; 
		//	dpp_pkt->chirp_info.hash_type = auth->chirp_tlv.hash_type; //value should be 0
			dpp_pkt->chirp_info.hash_validity = auth->chirp_tlv.hash_validity; //value should be 1
			dpp_pkt->chirp_info.enrollee_mac_address_present = 1;
			os_memcpy(&dpp_pkt->chirp_info.enrollee_mac[0], auth->peer_mac_addr, ETH_ALEN);

			DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"\n hash len - %d, hash_validity - %d",dpp_pkt->chirp_info.hash_len,
			dpp_pkt->chirp_info.hash_validity);
			
		}
		else {
			os_memcpy(dpp_pkt->almac, auth->relay_mac_addr, ETH_ALEN);
			dpp_pkt->chirp_tlv_present = 0;
		}
	} else 
		if (auth->allowed_roles == DPP_CAPAB_PROXYAGENT && 
				((auth->current_state == DPP_STATE_AUTH_RESP_WAITING)
				|| (auth->current_state == DPP_STATE_CONFIG_REQ_WAITING)
				|| (auth->current_state == DPP_STATE_CONFIG_RSP_WAITING)
				|| (auth->current_state == DPP_STATE_CONFIG_RESULT_WAITING)
				|| (auth->current_state == DPP_STATE_CONFIG_DONE)
#ifdef MAP_R3_RECONFIG
				|| (auth->current_state == DPP_STATE_RECONFIG_AUTH_RESP_WAITING)
				|| (auth->current_state == DPP_STATE_RECONFIG_AUTH_CONF_WAITING)
#endif
		)){	
		//Req - If a  Multi-APProxy Agent  receives a DPP Public Action frame with frame type set to 1 
		//(DPP Authentication Repsonse)  from an Enrollee Multi-AP Agent, it shall generate a 
		//1905 Encap DPP message containing an Encap DPP TLV with the Encapsulated frame field set to, 
		//encapsulate the received DPP Public Action frame body in it, set the Enrollee MAC Address 
		//present bit set to 1, include the Enrollee's MAC address inX the Destination STA MAC Address 
		//field set to the Enrolee's MAC address, set the Channel List Present bit set to 0, 
		//set the DPP Frame Indicator bit set to 0 and the Frame Type field set to 1, and shall send 
		//it the message to the Multi-AP Controller.   

				if (auth->current_state == DPP_STATE_AUTH_RESP_WAITING)
					auth->current_state = DPP_STATE_AUTH_CONF_WAITING;
				else if (auth->current_state == DPP_STATE_CONFIG_REQ_WAITING)
					auth->current_state = DPP_STATE_CONFIG_RSP_WAITING;
#ifdef MAP_R3_RECONFIG
				if (auth->current_state == DPP_STATE_RECONFIG_AUTH_RESP_WAITING)
					auth->current_state = DPP_STATE_RECONFIG_AUTH_CONF_WAITING;
#endif
				len = sizeof (struct dpp_msg) +	wpabuf_len(auth->msg_out) - auth->msg_out_pos;
	
				dpp_pkt = os_zalloc(len);
				
				os_memcpy(dpp_pkt->almac, auth->relay_mac_addr, ETH_ALEN);			
				dpp_pkt->dpp_frame_indicator = 0;
				dpp_pkt->chirp_tlv_present = 0;
				
				if (auth->current_state == DPP_STATE_CONFIG_RSP_WAITING)
					dpp_pkt->dpp_frame_indicator = 1;

				dpp_pkt->frame_type = wapp_get_frame_type(auth);

				if(auth->waiting_conn_status_result == 1)
					dpp_pkt->frame_type = DPP_PA_CONNECTION_STATUS_RESULT;
	
				dpp_pkt->dpp_info.chn_list_flag = 0;
				if (!is_zero_ether_addr(auth->peer_mac_addr)) {
					dpp_pkt->dpp_info.chn_list_flag = 0;
					dpp_pkt->dpp_info.enrollee_mac_flag = 1;
					os_memcpy(&dpp_pkt->dpp_info.enrollee_mac[0], auth->peer_mac_addr, ETH_ALEN);
				}	
			} else {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed, incorrect state: %d\n", auth->current_state);
				return -1;
			}

	
	dpp_pkt->payload_len = wpabuf_len(auth->msg_out) - auth->msg_out_pos;
	os_memcpy(dpp_pkt->payload, wpabuf_head_u8(auth->msg_out) + auth->msg_out_pos, dpp_pkt->payload_len);

	hex_dump_dbg(DPP_MAP_PREX"dpp dump= ",(UCHAR *)dpp_pkt->payload, dpp_pkt->payload_len);
	hex_dump_dbg(DPP_MAP_PREX"total dump= ",(UCHAR *)dpp_pkt, len);
	if (wapp_send_1905_msg(wapp, WAPP_SEND_DPP_MSG,
		len , (char *)dpp_pkt) < 0)
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed\n");

	os_free(dpp_pkt);

	return 0;
}
int dpp_direct_1905msg_auth_next(struct wifi_app *wapp, struct dpp_authentication *auth)
{
	unsigned int wait_time = 0, max_tries = 0;
	if (wapp->dpp->dpp_init_max_tries)
		max_tries = wapp->dpp->dpp_init_max_tries;
	else
		max_tries = 5;
	if (auth->auth_req_retry >= max_tries) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX
				   "DPP: No response received from responder - stopping initiation attempt");
		eloop_cancel_timeout(wapp_dpp_auth_timeout, wapp, auth);
		auth->auth_req_retry = 0;
		dpp_auth_deinit(auth);
		return -1;
	}

	/* Based on the Auth type send auth req */
	if(auth->ethernetTrigger == TRUE)
		dpp_direct_1905msg_send(auth);
	else
		dpp_1905msg_send(auth);

	auth->auth_req_retry++;
	eloop_cancel_timeout(wapp_dpp_auth_timeout, wapp, auth);
	if (wapp->dpp->dpp_init_retry_time)
		wait_time = wapp->dpp->dpp_init_retry_time;
	else
		wait_time = 10000;
	eloop_register_timeout(wait_time/1000,
		(wait_time % 1000) * 1000, wapp_dpp_auth_timeout, wapp, auth);
	return 0;
}

#endif /* MAP_R3 */

int dpp_wired_send(struct dpp_authentication *auth)
{
#ifdef MAP_R3
	if (auth->is_map_connection && (auth->allowed_roles != DPP_CAPAB_ENROLLEE)
		&& (auth->ethernetTrigger != TRUE)) {
		return dpp_1905msg_send(auth);
	}
	else if (auth->is_map_connection) { // Enrollee case 
		return dpp_direct_1905msg_send(auth);
	}
	else
#endif /* MAP_R3 */
		return dpp_tcp_send(auth);
}

void dpp_conn_tx_ready(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct dpp_authentication *auth = eloop_ctx;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: TCP socket %d ready for TX", sock);
	dpp_wired_send(auth);
}

static int dpp_ipaddr_to_sockaddr(struct sockaddr *addr, socklen_t *addrlen,
				  const struct wapp_ip_addr *ipaddr,
				  int port)
{
	struct sockaddr_in *dst;
#ifdef CONFIG_IPV6
	struct sockaddr_in6 *dst6;
#endif /* CONFIG_IPV6 */

	switch (ipaddr->af) {
	case AF_INET:
		dst = (struct sockaddr_in *) addr;
		os_memset(dst, 0, sizeof(*dst));
		dst->sin_family = AF_INET;
		dst->sin_addr.s_addr = ipaddr->u.v4.s_addr;
		dst->sin_port = htons(port);
		*addrlen = sizeof(*dst);
		break;
#ifdef CONFIG_IPV6
	case AF_INET6:
		dst6 = (struct sockaddr_in6 *) addr;
		os_memset(dst6, 0, sizeof(*dst6));
		dst6->sin6_family = AF_INET6;
		os_memcpy(&dst6->sin6_addr, &ipaddr->u.v6,
			  sizeof(struct in6_addr));
		dst6->sin6_port = htons(port);
		*addrlen = sizeof(*dst6);
		break;
#endif /* CONFIG_IPV6 */
	default:
		return -1;
	}

	return 0;
}

static struct dpp_authentication *
dpp_relay_new_conn(struct dpp_relay_controller *ctrl, const u8 *src,
		   unsigned int chan)
{
	struct dpp_authentication *auth;
	struct sockaddr_storage addr;
	socklen_t addrlen;
	char txt[100];

	if (dl_list_len(&ctrl->auth) >= 15) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Too many ongoing Relay connections to the Controller - cannot start a new one");
		return NULL;
	}

	if (dpp_ipaddr_to_sockaddr((struct sockaddr *) &addr, &addrlen,
				   &ctrl->ipaddr, DPP_TCP_PORT) < 0)
		return NULL;

	auth = os_zalloc(sizeof(*auth));
	if (!auth)
		return NULL;

	//auth->global = ctrl->global;
	auth->relay = ctrl;
	os_memcpy(auth->mac_addr, src, ETH_ALEN);
	auth->curr_chan = chan;
	auth->neg_chan = chan;

	auth->sock = socket(AF_INET, SOCK_STREAM, 0);
	if (auth->sock < 0)
		goto fail;
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: TCP relay socket %d connection to %s",
		   auth->sock, wapp_ip_txt(&ctrl->ipaddr, txt, sizeof(txt)));

	if (fcntl(auth->sock, F_SETFL, O_NONBLOCK) != 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: fnctl(O_NONBLOCK) failed: %s",
			   strerror(errno));
		goto fail;
	}

	if (connect(auth->sock, (struct sockaddr *) &addr, addrlen) < 0) {
		if (errno != EINPROGRESS) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to connect: %s",
				   strerror(errno));
			goto fail;
		}

		/*
		 * Continue connecting in the background; eloop will call us
		 * once the connection is ready (or failed).
		 */
	}

	if (eloop_register_sock(auth->sock, EVENT_TYPE_WRITE,
				dpp_conn_tx_ready, auth, NULL) < 0)
		goto fail;
	auth->write_eloop = 1;

	/* TODO: eloop timeout to clear a connection if it does not complete
	 * properly */

	dl_list_add(&ctrl->auth, &auth->list);
	return auth;
fail:
	dpp_auth_deinit(auth);
	return NULL;
}

struct wpabuf * dpp_map_encaps(const u8 *hdr, const u8 *buf, size_t len)
{
	struct wpabuf *msg;

	msg = wpabuf_alloc(1 + DPP_HDR_LEN + len);
	if (!msg)
		return NULL;

	wpabuf_put_u8(msg, WLAN_PA_VENDOR_SPECIFIC);
	wpabuf_put_data(msg, hdr, DPP_HDR_LEN);
	wpabuf_put_data(msg, buf, len);

	return msg;
}

#ifdef MAP_R3
static int dpp_relay_tx_map(struct dpp_authentication *auth, const u8 *hdr,
			const u8 *buf, size_t len)
{
	//const u8  *pos, *end;

	//end = hdr + len;
	//hdr += 2; /* skip Category and Actiom */
	//pos = hdr + DPP_HDR_LEN;
	wpabuf_free(auth->msg_out);
	auth->msg_out_pos = 0;
	auth->msg_out = dpp_map_encaps(hdr, buf, len);
	if (!auth->msg_out)
		goto fail;
					
	auth->is_wired = TRUE;
	return dpp_wired_send(auth);
	
fail:
	dpp_auth_deinit(auth);
	return -1;

}

#endif


static int dpp_relay_tx(struct dpp_authentication *auth, const u8 *hdr,
			const u8 *buf, size_t len)
{
	u8 type = hdr[DPP_HDR_LEN - 1];

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		   "DPP: Continue already established Relay/Controller connection for this session");
	wpabuf_free(auth->msg_out);
	auth->msg_out_pos = 0;
	auth->msg_out = dpp_tcp_encaps(hdr, buf, len);
	if (!auth->msg_out) {
		dpp_auth_deinit(auth);
		return -1;
	}

	/* TODO: for proto ver 1, need to do remove connection based on GAS Resp
	 * TX status */
	if (type == DPP_PA_CONFIGURATION_RESULT)
		auth->on_tcp_tx_complete_remove = 1;
	dpp_wired_send(auth);
	return 0;
}


#ifdef MAP_R3
#ifdef MAP_R3_RECONFIG
int dpp_get_reconfig_start(
	struct wifi_app *wapp, char *body, int body_len)
{
	struct wapp_dev *wdev = NULL, *wdevlist=NULL;
	struct dl_list *dev_list;
	int ret;
		
	char cmd[100];
	
	if ((wapp->dpp->onboarding_type == DPP_ONBOARDING_TYPE) && (wapp->map->map_version > 2)) {
	
		if (!wapp->drv_ops || !wapp->drv_ops->drv_set_1905_sec)
				return -1;
	
		dev_list = &wapp->dev_list;
		dl_list_for_each(wdevlist, dev_list, struct wapp_dev, list) {
			if (!wdevlist)
				break;
			if (wdevlist->radio
					&& wdevlist->dev_type == WAPP_DEV_TYPE_STA) {
				os_memset(cmd, 0, 100);
				ret = snprintf(cmd, 100, "iwpriv %s set ApCliEnable=0;", wdevlist->ifname);
				if (os_snprintf_error(100, ret))
					DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
				ret = system(cmd);
				if (ret != 0)
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s, error status_code:%d\n", __func__, ret);

				sleep(5);
				if (wdevlist->radio->op_ch == body[0])
					wdev = wdevlist;
			}
		}
		wapp->dpp->reconfig_annouce.is_enable = 1;
		wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT;
	
		wapp_dpp_send_reconfig_annouce(wapp, wdev);
	}
	return MAP_SUCCESS;
}




int dpp_get_reconfig_status(
	struct wifi_app *wapp, char *body, int body_len, char *evt_buf, int* len_buf)
{
	u8 trigger_reconfig = FALSE;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);

	struct wapp_dev *wdevlist=NULL;
	struct dl_list *dev_list;
	struct evt *map_event = NULL;
	unsigned char *buff = NULL;
//	char cmd[100];
	
	wapp->dpp->reconfig_channel = 0;
	if ((wapp->dpp->onboarding_type == DPP_ONBOARDING_TYPE) && (wapp->map->map_version > 2)) {
		dev_list = &wapp->dev_list;
		dl_list_for_each(wdevlist, dev_list, struct wapp_dev, list) {
			if (!wdevlist)
				break;
			if (wdevlist->radio
					&& wdevlist->dev_type == WAPP_DEV_TYPE_STA && wdevlist->config->dpp_ppkey) {
					trigger_reconfig = TRUE;
					wapp->dpp->reconfig_channel = body[0];
			}
		}
	}
	
	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_RECONFIG_STATUS;
	map_event->length = 1;
	buff = map_event->buffer;
	buff[0] = trigger_reconfig;
	*len_buf = sizeof(*map_event) + map_event->length;

	return MAP_SUCCESS;
}

#endif

int dpp_parse_autoconfig_frame(struct wifi_app *wapp, char *buf, int buf_len)
{
	unsigned char *temp_buf;
	struct dpp_authentication *auth = NULL;
	unsigned char almac[ETH_ALEN] = {0};
	struct dpp_bootstrap_info *peer_bi = NULL;
	int test_buf_len;
	struct dpp_agent_info *agnt_info = NULL;
	unsigned char hash_buf[SHA256_MAC_LEN];
	unsigned char hash_len = 0;
	unsigned char chirp_present = 0;
	//unsigned int wait_time, max_wait_time;

#if 0
	//TODO workaround for autoconf search here need handling as per Gabor
	if(wapp->dpp->dpp_wifi_onboard_ongoing == TRUE)
	{
		DBGPRINT(RT_DEBUG_ERROR,"bh type is wifi so ignring autoconfig in wapp.\n");
		return 0;
	}
#endif

	temp_buf = (unsigned char *)buf;
	os_memcpy(almac, temp_buf, ETH_ALEN); 
	temp_buf += ETH_ALEN;
	test_buf_len = buf_len - 6;

	while(test_buf_len) {
		if ((*temp_buf) == DPP_CHIRP_TLV) {
			chirp_present = 1;
			break;
		}
		temp_buf++;
		test_buf_len--;
	}
	
	hex_dump_dbg(DPP_MAP_PREX"autoconfig search frame ",(u8 *)buf , buf_len);

#if 1 //Milan added code use after code from Jie
	if(chirp_present) {
		temp_buf = temp_buf + 4;
		hash_len = *temp_buf;
		if (buf_len < hash_len) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s:Buffer length %d less than hash length %d\n"
				, __func__, buf_len, hash_len);
				return -1;
		}

		temp_buf++;

		os_memcpy(hash_buf, temp_buf, hash_len);
		hex_dump_dbg(DPP_MAP_PREX"hash buffer frm autoconfig",(u8 *)temp_buf , hash_len);

		agnt_info = wapp_dpp_get_agent_list_from_chirp_hash_val(wapp, (u8 *)hash_buf);
		if (!agnt_info) {
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: Agent not found with peer chirp hash buf\n");
			return 0;
		}

		if(agnt_info->agent_state == DPP_AGT_STATE_DPP_DONE) {
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"agnt state is %u so bh type is wifi, ignore autoconfig in wapp.\n",
				agnt_info->agent_state);
			return 0;
		}
		agnt_info->bh_type = MAP_BH_ETH;
	}
	else {
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"chirp TLV not present so bh type is wifi, ignore autoconfig in wapp.\n");
		return 0;
	}

	auth = wapp_dpp_get_auth_from_hash_val(wapp, hash_buf);
	/* Handle case if ethernet onboarding is received and WiFi onboarding is already initiated for this agent */
	/* Drop WiFi Onboarding */
	if (auth && auth->ethernetTrigger == FALSE) {
		dpp_auth_fail_wrapper(wapp, "Ethernet Onboarding Triggered, in between WiFi Onboarding");
		dpp_auth_deinit(auth);
		auth = NULL;
	}
	
	if (!auth) {
		dpp_auth_fail_wrapper(wapp, "auth insance from hash not found");
		
		peer_bi = dpp_chirp_find_pair(wapp->dpp, (const u8 *)hash_buf);
		if (!peer_bi) {
			dpp_auth_fail_wrapper(wapp, "Hash value did not match");
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Hash value did not match..\n");
			return 0;
		}

		auth = wapp_map_dpp_auth_init(wapp, NULL, peer_bi);
		if (auth == NULL) {
			dpp_auth_fail_wrapper(wapp, "Auth Request not send as URI didn't matched");
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"Auth Request not send as URI didn't matched\n");
			return 0;
		}
	} else { /* drop new auth request */				
				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: Auth already in process");
				dpp_auth_fail_wrapper(wapp, "Drop in between Auth Request");
				return 0;
	}
#else
	auth = wapp_dpp_get_auth_from_peer_mac(wapp, almac);
#endif

	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"\n almac is" MACSTR "\n",
			almac[0],almac[1],almac[2],almac[3],almac[4],almac[5]);
#if 0
	if (!auth) {
		DBGPRINT(RT_DEBUG_ERROR,"\n no auth init");
		auth = wapp_dpp_get_first_auth(wapp);
		if (!auth) {
			DBGPRINT(RT_DEBUG_ERROR,"\n auth not found return");
			return 0;
		}
		os_memcpy(auth->peer_mac_addr, almac,
			  MAC_ADDR_LEN);
	}
#endif	
	wapp->dpp->dpp_eth_onboard_ongoing = TRUE;
	os_memcpy(auth->peer_mac_addr, almac, MAC_ADDR_LEN);
	auth->ethernetTrigger = TRUE;

#if 0
	dpp_direct_1905msg_send(auth); //Send Auth request.

	eloop_cancel_timeout(wapp_dpp_eth_resp_wait_timeout, wapp, auth);
	wait_time = wapp->dpp->max_remain_on_chan;
	max_wait_time = wapp->dpp->dpp_resp_wait_time ?
		wapp->dpp->dpp_resp_wait_time : 10000;
	if (wait_time > max_wait_time)
		wait_time = max_wait_time;
	eloop_register_timeout(wait_time / 1000, (wait_time % 1000) * 1000,
			       wapp_dpp_eth_resp_wait_timeout, wapp, auth);
#endif
	dpp_direct_1905msg_auth_next(wapp, auth); //Send Auth request.
	return 0;
}

int dpp_parse_1905_onboard_notify_msg (struct wifi_app *wapp, char *buf, int buf_len)
{

	struct onboard_notify *notify = (struct onboard_notify *)buf;
	struct dpp_agent_info *agnt_info = NULL;
	
	if (notify->hash_len) {
		agnt_info = wapp_dpp_get_agent_list_from_chirp_hash_val(wapp, (u8 *)notify->hash);
		if (!agnt_info) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: Agent not found for which onboard notify\n");
			return 0;
		}
		eloop_cancel_timeout(
			wapp_dpp_agent_list_timeout, wapp, agnt_info);
#ifdef MAP_R4
		if (wapp->map->MapMode != 4)
#endif /* MAP_R4 */
			agnt_info->agent_state = DPP_AGT_STATE_INIT;
	}
	return 0;	
}


int dpp_parse_1905_cce_frame (struct wifi_app *wapp, char *buf, int buf_len)
{
	unsigned char *temp_buf;
	unsigned char almac[ETH_ALEN] = {0};
	unsigned short length = 0;
	unsigned char en_flag = 0;

	temp_buf = (unsigned char *)buf;
	os_memcpy(almac, temp_buf, ETH_ALEN); 
	temp_buf += ETH_ALEN;

	if((*temp_buf) == DPP_CCE_INDIACTION_NOTIFY_TYPE) {
		temp_buf++;
		buf_len--;
	}
	else {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed to parse\n");
		return -1;
	}

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be2cpu16(length);
	buf_len = length;
	if (buf_len != DPP_CCE_FRAME_LEN) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Invalid CCE Frame length %d\n", buf_len);
		return -1;
	}

	temp_buf += 2;
	
	hex_dump_dbg(DPP_MAP_PREX"cce frame ",(u8 *)temp_buf , buf_len);
	en_flag = *temp_buf;
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"enable_flag_val: %u & len:%u\n", en_flag, buf_len);
	
	if(en_flag == 1){
		wapp_dpp_set_ie(wapp);
	}
	else{
		wapp_dpp_reset_cce_ie(wapp);
	}

	return 0;
}

int dpp_parse_1905_dpp_uri_msg(struct wifi_app *wapp, char *buf, int buf_len)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
	unsigned char almac[ETH_ALEN] = {0};
	unsigned short uri_length = 0;
	unsigned char rcvd_dpp_uri[120] = {0};
	unsigned char sta_addr[ETH_ALEN] = {0};
	int peer_bi_id;
	int tlv_len = 0;

	temp_buf = (unsigned char *)buf;

	hex_dump_dbg(DPP_MAP_PREX"1905 URI notification frame ",(unsigned char *)buf , (unsigned int )buf_len);

	os_memcpy(almac, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;
	if((*temp_buf) == DPP_BOOTSTRAP_URI_NOTIFY_TYPE) {
		temp_buf++;
		buf_len--;
	}
	else {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed to parse\n");
		return -1;
	}

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be2cpu16(length);
	tlv_len = length + 3;
	if (buf_len < tlv_len) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s:Buffer length %d less than TLV length %d\n", __func__, buf_len, tlv_len);
		return -1;
	}


	temp_buf += 2;

	//TODO check wether the RUID and mac address need to be used
	temp_buf += 12;

	os_memcpy(sta_addr, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"sta_mac.."MACSTR"\n",MAC2STR(sta_addr));

	uri_length = length - 18;

	os_memcpy(rcvd_dpp_uri, temp_buf, uri_length);
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"Received URI from the 1905 on wapp:%s\n",rcvd_dpp_uri);
	peer_bi_id = wapp_dpp_qr_code(wapp, (const char *)rcvd_dpp_uri);

	if(peer_bi_id == 0)
		dpp_auth_fail_wrapper(wapp, "DPP: invalid URI added\n");

	return 0;
}

int dpp_parse_1905_sec_notify_frame(struct wifi_app *wapp, char *buf, int buf_len)
{
	unsigned char *temp_buf;
	unsigned char almac[ETH_ALEN] = {0};

	temp_buf = (unsigned char *)buf;
	os_memcpy(almac, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	hex_dump_dbg(DPP_MAP_PREX"1905 sec notify frame ",(unsigned char *)buf , (unsigned int )buf_len);

	wapp->dpp->map_sec_done = *temp_buf;
	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"value for the 1905 security flag is: %u\n", wapp->dpp->map_sec_done);
	wapp_dpp_set_1905_sec(wapp, wapp->dpp->map_sec_done);

	return 0;
}

int dpp_parse_direct_1905_frame(struct wifi_app *wapp, char *buf, int buf_len)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
	//unsigned char sta_addr[ETH_ALEN] = {0};
	unsigned char almac[ETH_ALEN] = {0};
	int tlv_len = 0;

	temp_buf = (unsigned char *)buf;

	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"\n direct frame received buf_len is %d",buf_len);

	hex_dump_dbg(DPP_MAP_PREX"dpp direct frame ",(u8 *)buf , buf_len);
	
	os_memcpy(almac, temp_buf, ETH_ALEN); 
	if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_CONFIGURATOR)
		os_memcpy(wapp->dpp->al_mac, almac, ETH_ALEN);

	temp_buf += ETH_ALEN;
	if((*temp_buf) == _1905_ENCAP_DPP_DIRECT_MESSAGE_TYPE) {
		temp_buf++;
		buf_len--;
	}
	else {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed to parse\n");
		return -1;
	}

	//copying the controller alid in wapp for using in chirp message
	if(wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE)
		os_memcpy(wapp->dpp->almac_cont, almac, MAC_ADDR_LEN);

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be2cpu16(length);
	tlv_len = length + 3;
	if (buf_len < tlv_len) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s:Buffer length %d less than TLV length %d\n", __func__, buf_len, tlv_len);
		return -1;
	}

	buf_len = length;
	if (buf_len == 0 || buf_len > DPP_SHORT_MAX_LEN) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s:Buffer length %d greater then range\n",
			__func__, buf_len);
		return -1;
	}

	hex_dump_dbg(DPP_MAP_PREX"dpp frame ",(u8 *)buf , buf_len);

	temp_buf += 2;

	wapp->is_eth_onboard = TRUE;
	/* not a relay packet it is for us, enqueue in current state machine for process */
	map_process_dpp_packet(wapp, temp_buf, buf_len, almac, almac);
	return 0;
}

int dpp_parse_1905_chirp_msg(struct wifi_app * wapp, char *buf, int buf_len)
{
	unsigned char *temp_buf;
	unsigned char hash_buf[SHA256_MAC_LEN] = {0};
	unsigned char hash_len;
	unsigned char hash_validity = 0;
	unsigned short length = 0;
	unsigned char *p_bitmap = NULL;
	unsigned char sta_addr[ETH_ALEN] = {0};
	unsigned char almac[ETH_ALEN] = {0};
	struct dpp_authentication *auth = NULL;
	struct dpp_bootstrap_info *peer_bi = NULL;
	struct dpp_agent_info *agnt_info = NULL;
	int tlv_len = 0;
	temp_buf = (unsigned char *)buf;

	os_memcpy(almac, temp_buf, ETH_ALEN); 
	temp_buf += ETH_ALEN;
	if((*temp_buf) == DPP_CHIRP_TLV) {
		temp_buf++;
		buf_len--;
	}
	else {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed to parse\n");
		return -1;
	}
	
	hex_dump_dbg(DPP_MAP_PREX"chirp frame ",(u8 *)buf , buf_len+1);

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be2cpu16(length);
	tlv_len = length + 3;
	if (buf_len < tlv_len) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s:Buffer length %d less than TLV length %d\n", __func__, buf_len, tlv_len);
		return -1;
	}

	buf_len = length;
	temp_buf += 2;

	//get bitmap from buf
	p_bitmap = temp_buf++;
	buf_len--;

	if ((*p_bitmap & 0x80)) {
		os_memcpy(sta_addr, temp_buf, ETH_ALEN); 
		temp_buf += ETH_ALEN;
		//is_relay = TRUE;
		buf_len -= ETH_ALEN;
	}
	
	if (*p_bitmap & 0x40)
		hash_validity = 1;

	hash_len = *temp_buf;

#ifdef MAP_R3_RECONFIG
	if (hash_len != 0) {
#endif /* MAP_R3_RECONFIG */
		temp_buf++;
		buf_len--;
		if (buf_len < hash_len) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s:Buffer length %d less than Hash length %d\n", __func__, buf_len, hash_len);
			return -1;
		}

		os_memcpy(hash_buf, temp_buf, hash_len);
		hex_dump_dbg(DPP_MAP_PREX"hash buffer ", (u8 *)temp_buf, hash_len);
#ifdef MAP_R3_RECONFIG
	}
#endif /* MAP_R3_RECONFIG */

#ifdef MAP_R3_RECONFIG
	if ((hash_len == 0) && is_zero_ether_addr(sta_addr)) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" Invalid enrollee mac addr\n");
		return -1;
	}
#endif /* MAP_R3_RECONFIG */
	
	if(!hash_validity) {
#ifdef MAP_R3_RECONFIG
		if (hash_len == 0)
			auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)sta_addr);
		else
#endif /* MAP_R3_RECONFIG */
		auth = wapp_dpp_get_auth_from_hash_val(wapp, hash_buf);
		if(!auth) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"auth insance from hash not found\n");
		}
		else {
#ifdef MAP_R3_RECONFIG
			if (auth->current_state != DPP_STATE_CONFIG_DONE) {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Auth is not in correct state:%d\n", auth->current_state);
				return 0;
			}
#endif /* MAP_R3_RECONFIG */
			//clearing auth from memory
			dpp_auth_deinit(auth);
		}
	}
	else {
		if(wapp->dpp->dpp_allowed_roles == DPP_CAPAB_CONFIGURATOR) {
			/* Try to find peer chirp key matches based on the
			 * received hash values */
			peer_bi= dpp_chirp_find_pair(wapp->dpp, (const u8 *)hash_buf);

			if (!peer_bi) {
				dpp_auth_fail_wrapper(wapp, "Hash value did not match.");
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Hash value did not match..\n");
				return 0;
			}

			agnt_info = wapp_dpp_get_agent_list_from_chirp_hash_val(wapp, (u8 *)hash_buf);
			if (!agnt_info) {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: Agent not found with peer chirp hash buf\n");
				return 0;
			}

			auth = wapp_dpp_get_auth_from_hash_val(wapp, hash_buf);
			if(!auth) {
				dpp_auth_fail_wrapper(wapp, "auth insance from hash not found");
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"auth insance from hash not found\n");
				
				auth = wapp_map_dpp_auth_init(wapp, NULL, peer_bi);
				if (auth == NULL) {
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Auth Request not send as URI didn't matched\n");
					dpp_auth_fail_wrapper(wapp, "Auth Request not send as URI didn't matched");
					return 0;
				}
			} else { /* drop new auth request */
				dpp_auth_fail_wrapper(wapp, "DPP: Auth already in process");
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP: Auth already in process");
				// Send event to User. - ToDo
				return 0;
			}
			if(auth->current_state == DPP_STATE_AUTH_RESP_WAITING) {
				agnt_info->bh_type = MAP_BH_WIFI;
				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP: Setting the agent info with mac to bh %u\n",
						agnt_info->bh_type);
				DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"sending auth request to agent\n");
				os_memcpy(auth->peer_mac_addr, sta_addr, MAC_ADDR_LEN);
				dpp_direct_1905msg_auth_next(wapp, auth); //Send Auth request.
			}
			else {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Auth is not in correct state:%d so returning\n", auth->current_state);
				return 0;
			}
		}
	}
	return 0;
}

int dpp_parse_1905_frame(struct wifi_app *wapp, char *buf, int buf_len)
{
	unsigned char *temp_buf;
	unsigned char *chirp_buf=NULL;
	unsigned short length = 0;
	unsigned char *p_bitmap = NULL;
	unsigned char sta_addr[ETH_ALEN] = {0};
	enum dpp_public_action_frame_type dpp_action_type = DPP_PA_UNDEFINED_FRAME;
	//unsigned char is_relay = 0;
	unsigned char is_gas = 0; //, chan = 0;
	unsigned char almac[ETH_ALEN] = {0};
	struct dpp_authentication *auth = NULL;
	int tlv_len = 0, rem_buf_len = 0, buf_len_recv;

	int i = 0;

	temp_buf = (unsigned char *)buf;

	os_memcpy(almac, temp_buf, ETH_ALEN);
	os_memcpy(wapp->dpp->al_mac, almac, ETH_ALEN);
	wapp->dpp->dpp_wifi_onboard_ongoing = TRUE;
	temp_buf += ETH_ALEN;
	if((*temp_buf) == _1905_ENCAP_DPP_MESSAGE_TYPE) {
		temp_buf++;
		buf_len--;
	}
	else {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed to parse\n");
		return -1;
	}

	hex_dump_dbg(DPP_MAP_PREX"dpp frame ",(u8 *)buf , buf_len+1);
	buf_len_recv = buf_len;
	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be2cpu16(length);
	tlv_len = length + 3;
	if (buf_len < tlv_len) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s:Buffer length %d less than TLV length %d\n", __func__, buf_len, tlv_len);
		return -1;
	}

	buf_len = length;

	temp_buf += 2;
	p_bitmap = temp_buf++;
	buf_len--;

	/* STA MAC address.
	 ** (This field is present if the Final destination field is set to 0)
	 */
 
	if ((*p_bitmap & 0x80)) {
		os_memcpy(sta_addr, temp_buf, ETH_ALEN); 
		temp_buf += ETH_ALEN;
		//is_relay = TRUE;
		buf_len -= ETH_ALEN;
	}


	/* Number of Operating Classes
	 ** (This field is present if the Channel list field is set to 1)
	 */
	if (*p_bitmap & 0x40) {
		unsigned char opclass_num = 0;
		unsigned char ch_num = 0;

		opclass_num = *(temp_buf++);
		buf_len--;

		for (i = 0; i < opclass_num; i ++) {
			ch_num = *(temp_buf++);
			buf_len--;
			//chan = *temp_buf;
			temp_buf += ch_num;
			buf_len -= ch_num;
		}
	}
	if (*p_bitmap & 0x20)
		is_gas = TRUE;
	else
		dpp_action_type = (*temp_buf);

	temp_buf++;
	buf_len--;
	
	//buf_len = *temp_buf;
	//temp_buf++;
	//calculate encap frame length
	length = *(unsigned short *)temp_buf;
	length = be2cpu16(length);
	if (buf_len < length) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s:Buffer length %d less than Encap TLV length %d\n", __func__, buf_len, length);
		return -1;
	}

	buf_len = length;
	temp_buf += 2;

	//Add MAP Check and Agent Check
	if(!is_gas){
		if (dpp_action_type == DPP_PA_AUTHENTICATION_REQ 
#ifdef MAP_R3_RECONFIG
		|| dpp_action_type == DPP_PA_RECONFIG_AUTH_REQ
#endif
		) {
			// Req - If a Proxy Agent receives a  1905 Encap DPP messages that includes an encapsulated DPP Authentication Request
			//frame and a DPP Chirp Value TLV, the Multi-AP Agent shall listen for Presence Announcement messages on its operating
			//primary channel(s) and shall compare the hash value in any received Presence Announcement with the value(s) received in the Hash Value of the DPP Chirp Value TLV. A The Multi-AP Agent shall decapsulate and store the DPP Authentication Request frame until it receives a Presence Announcement message (chirp) with a matching hash value from the Enrollee Multi-AP Agent. If a match is received, the Multi-AP Agent shall send the DPP Authentication Request frame to the Enrollee 
			//within 1 second of receiving the Presence Announcement message from that Enrollee, using a DPP Public Action frame. 
			wapp->map->bh_type = MAP_BH_WIFI;  //Setting BH type to wifi for proxy agent here
#ifdef MAP_R3_RECONFIG
			if(dpp_action_type != DPP_PA_RECONFIG_AUTH_REQ)
				chirp_buf = temp_buf + buf_len;
#else
				chirp_buf = temp_buf + buf_len;
#endif
			rem_buf_len = buf_len_recv - tlv_len;
			auth = dpp_auth_init_relay(wapp, almac, sta_addr, chirp_buf, temp_buf, buf_len, rem_buf_len);
			if(auth == NULL)
			{
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed to get auth instance..\n");
				return -1;
			}
			return 0;
			//Add code to listen for Presence announce frame and 
			// Create a auth state machine using STA MAC Address

		}
#ifdef MAP_R3_RECONFIG
		else if (dpp_action_type == DPP_PA_RECONFIG_ANNOUNCEMENT)
			wapp->dpp->dpp_reconfig_ongoing = TRUE;		
#endif
#if 0
		else{
			auth = wapp_dpp_get_auth_from_peer_mac(wapp, sta_addr);
			if(auth == NULL)
			{
				DBGPRINT(RT_DEBUG_ERROR,"failed to get auth instance..\n");
				return -1;
			}

			if (is_relay && (auth->allowed_roles == DPP_CAPAB_PROXYAGENT)) {
				/* Send it to air for STA */ //TODO handle multiple channels
				if(chan)
					wapp_dpp_relay_tx(wapp, NULL, sta_addr, chan, temp_buf, buf_len);
				else
					wapp_dpp_relay_tx(wapp, NULL, sta_addr, auth->curr_chan, temp_buf, buf_len);
				return 0;
			}
		}
#endif
	}
#if 0
	if (is_relay && !is_gas) {
		/* Send it to air for STA */ //TODO handle multiple channels
		wapp_dpp_relay_tx(wapp, NULL, sta_addr, chan, temp_buf, buf_len);
	} else 
	if (is_relay && is_gas) { 
		//Shouldn't be called if packet received from proxy agent - Need to handle
		
		struct wpabuf *opbuf = wpabuf_alloc_copy(temp_buf, buf_len);
		if (opbuf == NULL)
			return -1;

		wapp_dpp_relay_gas_resp_tx(wapp, sta_addr, 1, 0, opbuf);
	}
#endif

	/* not a relay packet it is for us, enqueue in current state machine for process */
	map_process_dpp_packet(wapp, temp_buf, buf_len, sta_addr, almac);
	return 0;
}


int dpp_relay_rx_action_map(struct wifi_app *wapp, const u8 *src, const u8 *hdr,
			const u8 *buf, size_t len, unsigned int chan,
			const u8 *i_bootstrap, const u8 *r_bootstrap,
			enum dpp_public_action_frame_type type)
{
#ifdef MAP_R3
	int ret;
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);
	if(auth == NULL){
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Auth instance NULL\n");
		return -1;
	}

	//For Proxy agent adding check for conn status result	
	if (type == DPP_PA_CONNECTION_STATUS_RESULT) {
		auth->waiting_conn_status_result = 1;
		eloop_cancel_timeout(
			wapp_dpp_conn_status_result_wait_timeout, wapp, auth);
	}

	/* Clearing the GAS server state machine for MAP here to avoid timeout */
	if (wapp->dpp && (type == DPP_PA_CONFIGURATION_RESULT)) {
		wapp->dpp->conf_res_received = 1;
		gas_server_tx_status(wapp->dpp->gas_server, src,
				NULL, 0, 1);
		wapp->dpp->conf_res_received = 0;
		/* Registering timer to clear auth state of enrollee on proxy agent */
		eloop_cancel_timeout(
			wapp_dpp_conn_status_result_wait_timeout, wapp, auth);
		eloop_register_timeout(
			DPP_CONNECT_STATUS_TIMEOUT, 0, wapp_dpp_conn_status_result_wait_timeout, wapp, auth);
	}

	ret = dpp_relay_tx_map(auth, hdr, buf, len);
	if (ret < 0) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Relay sending failure.\n");
		return -1;
	}

	if (type == DPP_PA_CONFIGURATION_RESULT)
		auth->current_state = DPP_STATE_CONFIG_DONE;

	//For proxy agent deleting the auth isnstance here after sending conn status result
	if (type == DPP_PA_CONNECTION_STATUS_RESULT && ret != -1)
		dpp_auth_deinit(auth);

	return ret;
#else
	struct dpp_relay_controller *ctrl = NULL;
	struct dpp_authentication *auth;
	u8 type = hdr[DPP_HDR_LEN - 1];

	/* Check if there is an already started session for this peer and if so,
	 * continue that session (send this over TCP) and return 0.
	 */
	if (type != DPP_PA_PEER_DISCOVERY_REQ &&
	    type != DPP_PA_PEER_DISCOVERY_RESP) {
		dl_list_for_each(auth, &ctrl->auth,
				 struct dpp_authentication, list) {
			if (os_memcmp(src, auth->mac_addr,
				      ETH_ALEN) == 0) {
				 //Make sure to fill auth response in auth
				return dpp_relay_tx_map(auth, hdr, buf, len);
			}
		}

	}

	//To Do - If no auth found create auth init if has matches
#endif /* MAP_R3 */
	return 0;
}


int dpp_relay_rx_gas_req_map(struct wifi_app *wapp, const u8 *src, const u8 *data,
			 size_t data_len)
{
#ifdef MAP_R3
	struct wpabuf *msg;

	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);

	if(!auth)
		return -1;
#else
	struct dpp_relay_controller *ctrl = NULL;
	struct dpp_authentication *auth, *found = NULL;
	struct wpabuf *msg;

	/* Check if there is a successfully completed authentication for this
	 * and if so, continue that session (send this over MAP Link) and return 0.
	 */
	dl_list_for_each(auth, &ctrl->auth,
			 struct dpp_authentication, list) {
		if (os_memcmp(src, auth->mac_addr,
			      ETH_ALEN) == 0) {
			found = auth;
			break;
		}
	}

	if (!found)
		return -1;
#endif /* MAP_R3 */

	msg = wpabuf_alloc(4 + 2 + data_len);
	if (!msg)
		return -1;
	//wpabuf_put_be32(msg, 1 + data_len);
	wpabuf_put_u8(msg, WLAN_PA_GAS_INITIAL_REQ);
	wpabuf_put_data(msg, data, data_len);
	wpa_hexdump_buf(MSG_MSGDUMP, "DPP: Outgoing MAP Agent message to controller", msg);

	wpabuf_free(auth->msg_out);
	auth->msg_out_pos = 0;
	auth->msg_out = msg;
	dpp_wired_send(auth);
	return 0;
}
#endif

int dpp_relay_rx_action(struct dpp_global *dpp, const u8 *src, const u8 *hdr,
			const u8 *buf, size_t len, unsigned int chan,
			const u8 *i_bootstrap, const u8 *r_bootstrap)
{
	struct dpp_relay_controller *ctrl;
	struct dpp_authentication *auth;
	u8 type = hdr[DPP_HDR_LEN - 1];

	/* Check if there is an already started session for this peer and if so,
	 * continue that session (send this over TCP) and return 0.
	 */
	if (type != DPP_PA_PEER_DISCOVERY_REQ &&
	    type != DPP_PA_PEER_DISCOVERY_RESP) {
		dl_list_for_each(ctrl, &dpp->controllers,
				 struct dpp_relay_controller, list) {
			dl_list_for_each(auth, &ctrl->auth,
					 struct dpp_authentication, list) {
				if (os_memcmp(src, auth->mac_addr,
					      ETH_ALEN) == 0)
					return dpp_relay_tx(auth, hdr, buf, len);
			}
		}
	}

	if (!r_bootstrap)
		return -1;

	ctrl = dpp_relay_controller_get(dpp, r_bootstrap);
	if (!ctrl)
		return -1;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		   "DPP: Authentication Request for a configured Controller");
	auth = dpp_relay_new_conn(ctrl, src, chan);
	if (!auth)
		return -1;

	auth->msg_out = dpp_tcp_encaps(hdr, buf, len);
	if (!auth->msg_out) {
		dpp_auth_deinit(auth);
		return -1;
	}
	/* Message will be sent in dpp_conn_tx_ready() */

	return 0;
}

int dpp_relay_rx_gas_req(struct dpp_global *dpp, const u8 *src, const u8 *data,
			 size_t data_len)
{
	struct dpp_relay_controller *ctrl;
	struct dpp_authentication *auth = NULL, *found = NULL;
	struct wpabuf *msg = NULL;

	/* Check if there is a successfully completed authentication for this
	 * and if so, continue that session (send this over TCP) and return 0.
	 */
	dl_list_for_each(ctrl, &dpp->controllers,
			 struct dpp_relay_controller, list) {
		if (found)
			break;
		dl_list_for_each(auth, &ctrl->auth,
				 struct dpp_authentication, list) {
			if (os_memcmp(src, auth->mac_addr,
				      ETH_ALEN) == 0) {
				found = auth;
				break;
			}
		}
	}

	if (!found)
		return -1;

	msg = wpabuf_alloc(4 + 1 + data_len);
	if (!msg)
		return -1;
	wpabuf_put_be32(msg, 1 + data_len);
	wpabuf_put_u8(msg, WLAN_PA_GAS_INITIAL_REQ);
	wpabuf_put_data(msg, data, data_len);
	wpa_hexdump_buf(MSG_MSGDUMP, "DPP: Outgoing TCP message", msg);

	wpabuf_free(auth->msg_out);
	auth->msg_out_pos = 0;
	auth->msg_out = msg;
	dpp_wired_send(auth);
	return 0;
}

static int dpp_controller_rx_action(struct wifi_app *wapp, struct dpp_authentication *auth, const u8 *msg, u8 *src,
				    size_t len)
{
	const u8 *pos, *end;
	u8 type;
	struct wapp_dev *wdev = NULL;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Received DPP Action frame over wire");
	pos = msg;
	end = msg + len;
	hex_dump_dbg(DPP_MAP_PREX"pkt dump= ",(UCHAR *)msg, len);

	if (end - pos < DPP_HDR_LEN ||
	    WPA_GET_BE24(pos) != OUI_WFA ||
	    pos[3] != DPP_OUI_TYPE) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unrecognized header");
		return -1;
	}

	if (pos[4] != 1) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unsupported Crypto Suite %u",
			   pos[4]);
		return -1;
	}
	type = pos[5];
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Received message type %u", type);
	pos += DPP_HDR_LEN;

	wpa_hexdump(MSG_MSGDUMP, DPP_MAP_PREX"DPP: Received message attributes",
		    pos, end - pos);
	if (dpp_check_attrs(pos, end - pos) < 0)
		return -1;

	// MAP case, should not go into this
	if (auth && auth->relay) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Relay - send over WLAN");
		auth->relay->tx(auth->relay->cb_ctx, auth->wdev, auth->mac_addr,
				auth->neg_chan, msg, len);
		return 0;
	}

	return wapp_dpp_process_frames(wapp, wdev, src, msg, pos, end - pos, 0, type);
}

static int dpp_controller_rx_gas_req(struct dpp_authentication *auth, const u8 *msg,
				     size_t len)
{
	const u8 *pos, *end, *next;
	u8 dialog_token;
	const u8 *adv_proto;
	u16 slen;
	struct wpabuf *resp, *buf;

	if (len < 1 + 2)
		return -1;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		   "DPP: Received DPP Configuration Request over TCP");

	if (!auth || !auth->auth_success) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No matching exchange in progress");
		return -1;
	}

	struct wifi_app *wapp = (struct wifi_app *)auth->msg_ctx;
	if(!wapp)
	{
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"invalid auth or wapp insance.\n");
		return -1;
	}

	pos = msg;
	end = msg + len;

	dialog_token = *pos++;
	adv_proto = pos++;
	slen = *pos++;
	if (*adv_proto != WLAN_EID_ADV_PROTO ||
	    slen > end - pos || slen < 2)
		return -1;

	next = pos + slen;
	pos++; /* skip QueryRespLenLimit and PAME-BI */

	if (slen != 8 || *pos != WLAN_EID_VENDOR_SPECIFIC ||
	    pos[1] != 5 || WPA_GET_BE24(&pos[2]) != OUI_WFA ||
	    pos[5] != DPP_OUI_TYPE || pos[6] != 0x01)
		return -1;

	pos = next;
	/* Query Request */
	if (end - pos < 2)
		return -1;
	slen = WPA_GET_LE16(pos);
	pos += 2;
	if (slen > end - pos)
		return -1;

	eloop_cancel_timeout(wapp_dpp_config_req_wait_timeout, wapp, auth);
	resp = dpp_conf_req_rx(auth, pos, slen);
	if (!resp)
		return -1;

	buf = wpabuf_alloc(4 + 18 + wpabuf_len(resp));
	if (!buf) {
		wpabuf_free(resp);
		return -1;
	}

	if (!auth->is_map_connection)
		wpabuf_put_be32(buf, 18 + wpabuf_len(resp));

	wpabuf_put_u8(buf, WLAN_PA_GAS_INITIAL_RESP);
	wpabuf_put_u8(buf, dialog_token);
	wpabuf_put_le16(buf, WLAN_STATUS_SUCCESS);
	wpabuf_put_le16(buf, 0); /* GAS Comeback Delay */

	dpp_write_adv_proto(buf, 0);
	dpp_write_gas_query(buf, resp);
	wpabuf_free(resp);

	/* Send Config Response over TCP; GAS fragmentation is taken care of by
	 * the Relay */
	wpa_hexdump_buf(MSG_MSGDUMP, "DPP: Outgoing TCP message", buf);
	wpabuf_free(auth->msg_out);
	auth->msg_out_pos = 0;
	auth->msg_out = buf;
	auth->on_tcp_tx_complete_gas_done = 1;
	auth->current_state = DPP_STATE_CONFIG_RESULT_WAITING;
	dpp_wired_send(auth);
	eloop_cancel_timeout(wapp_dpp_config_result_wait_timeout,
			wapp, auth);
	eloop_register_timeout(10, 0, //Milan to do check exact timeout period
			wapp_dpp_config_result_wait_timeout,
			wapp, auth);
	return 0;
}

#ifdef MAP_R3   //TODO check later
static int dpp_tcp_rx_gas_resp(struct dpp_authentication *auth, struct wpabuf *resp)
{
	int res;
	struct wpabuf *msg, *encaps;
	enum dpp_status_error status;

	struct wifi_app *wapp = (struct wifi_app *)auth->msg_ctx;
	if(!wapp)
	{
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"invalid auth or wapp insance.\n");
		return -1;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		   "DPP: Configuration Response for local stack from wire");

	res = dpp_conf_resp_rx(auth, resp);
	wpabuf_free(resp);
	if (res < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Configuration attempt failed");
	}

	eloop_cancel_timeout(wapp_dpp_eth_resp_wait_timeout, wapp, auth);
#if 0
	res = wapp_dpp_process_conf_obj(wapp, auth);
#endif
	if (auth->peer_version < 2 || auth->conf_resp_status != DPP_STATUS_OK)
		return -1;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Send DPP Configuration Result");
	status = res < 0 ? DPP_STATUS_CONFIG_REJECTED : DPP_STATUS_OK;
	msg = dpp_build_conf_result(auth, status);
	if (!msg)
		return -1;

	encaps = wpabuf_alloc(4 + wpabuf_len(msg) - 1);
	if (!encaps) {
		wpabuf_free(msg);
		return -1;
	}
	if (!auth->is_map_connection)
		wpabuf_put_be32(encaps, wpabuf_len(msg) - 1);
	wpabuf_put_data(encaps, wpabuf_head(msg) + 1, wpabuf_len(msg) - 1);
	wpabuf_free(msg);
	wpa_hexdump_buf(MSG_MSGDUMP, "DPP: Outgoing TCP message", encaps);

	wpabuf_free(auth->msg_out);
	auth->msg_out_pos = 0;
	auth->msg_out = encaps;
	auth->on_tcp_tx_complete_remove = 1;
	dpp_wired_send(auth);
	if(status == DPP_STATUS_OK) {
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"setting config done here..\n");
		wapp->dpp->config_done = 1;
	}

	/* This exchange will be terminated in the TX status handler */
	 //if(!auth->conn_status_requested) //Milan skip this check
	//Deinit auth instance for eth onboarding here
	if(status != DPP_STATUS_OK)
		dpp_auth_deinit(auth); 

	return 0;
}
#endif /* MAP_R3 */
static int dpp_rx_gas_resp(struct dpp_authentication *auth, const u8 *msg,
			   size_t len)
{
	struct wpabuf *buf;
	u8 dialog_token;
	const u8 *pos, *end, *next, *adv_proto;
	u16 status, slen;
#ifdef MAP_R3
	struct wifi_app *wapp = NULL;
#endif /* MAP_R3 */
	if (!auth) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: No matching exchange in progress");
		return -1;
	}
	wapp = (struct wifi_app *) auth->msg_ctx;

	if (len < 5 + 2)
		return -1;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		   "DPP: Received DPP Configuration Response over wire");
	pos = msg;
	end = msg + len;

	dialog_token = *pos++;
	status = WPA_GET_LE16(pos);
	if (status != WLAN_STATUS_SUCCESS) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unexpected Status Code %u", status);
		return -1;
	}
	pos += 2;
	pos += 2; /* ignore GAS Comeback Delay */

	adv_proto = pos++;
	slen = *pos++;
	if (*adv_proto != WLAN_EID_ADV_PROTO ||
	    slen > end - pos || slen < 2)
		return -1;

	next = pos + slen;
	pos++; /* skip QueryRespLenLimit and PAME-BI */

	if (slen != 8 || *pos != WLAN_EID_VENDOR_SPECIFIC ||
	    pos[1] != 5 || WPA_GET_BE24(&pos[2]) != OUI_WFA ||
	    pos[5] != DPP_OUI_TYPE || pos[6] != 0x01)
		return -1;
	
	pos = next;
	/* Query Response */
	if (end - pos < 2)
		return -1;

	slen = WPA_GET_LE16(pos);
	pos += 2;
	if (slen > (end - pos))
		return -1;
	
	buf = wpabuf_alloc(slen);
	if (!buf)
		return -1;

	wpabuf_put_data(buf, pos, slen);
	
	if (!auth->relay
#ifdef MAP_R3
		&& (!wapp->dpp->dpp_wifi_onboard_ongoing) && (wapp->dpp->dpp_allowed_roles != DPP_CAPAB_PROXYAGENT)
#endif
		)
#ifdef MAP_R3
		return dpp_tcp_rx_gas_resp(auth, buf);
#endif /* MAP_R3 */

	if (!auth->relay
#ifdef MAP_R3
		&& (!wapp->dpp->dpp_wifi_onboard_ongoing) && (wapp->dpp->dpp_allowed_roles != DPP_CAPAB_PROXYAGENT)
#endif
		) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No matching exchange in progress");
		wpabuf_free(buf);
		return -1;
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Relay - send over WLAN");
#ifdef MAP_R3
	wapp_dpp_relay_gas_resp_tx(auth->msg_ctx, auth->peer_mac_addr,
		 dialog_token, 0, buf);  //TODO modify when required
#else
	auth->relay->gas_resp_tx(auth->relay->cb_ctx, auth->mac_addr,
				 dialog_token, 0, buf);
#endif

	return 0;
}

void map_process_dpp_packet(struct wifi_app *wapp, u8 *msg, int len, u8 *src, u8 *almac)
{
	/* find auth for this */
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);
#ifdef MAP_R3
	//For case of MAC not present in URI added this condition
	//commented as no need now.
#if 0
	if(!auth 
#ifdef MAP_R3_RECONFIG
		&& (wapp->dpp->dpp_reconfig_ongoing == FALSE)
#endif	
	) { 
		auth = wapp_dpp_get_last_auth(wapp);
	}
#endif
	/* Drop WiFi Connection if ether onboarding ongoing */
	//if (auth && auth->ethernetTrigger == TRUE)
		//return;
		
	if (auth && auth->is_map_connection) {
		os_memcpy(auth->relay_mac_addr, almac, MAC_ADDR_LEN);
	}
	/* Storing ALMAC of the agent to be used while updating auth resp */
	os_memcpy(wapp->dpp->relay_almac_addr, almac, MAC_ADDR_LEN);
#endif
	dpp_controller_process_rx(wapp, (void *) auth, msg, src, len);
} 

void dpp_controller_process_rx(struct wifi_app *wapp, void *ctx, const u8 *pos, u8 *src, size_t len)
{
	struct dpp_authentication *auth = (struct dpp_authentication *)ctx;
	switch (*pos) {
	case WLAN_PA_VENDOR_SPECIFIC:
		if (dpp_controller_rx_action(wapp, auth, pos + 1, src,
					     len - 1) < 0)
			dpp_auth_deinit(auth);
		break;
	case WLAN_PA_GAS_INITIAL_REQ:
		if (dpp_controller_rx_gas_req(auth, pos + 1,
					      len - 1) < 0)
			dpp_auth_deinit(auth);
		break;
	case WLAN_PA_GAS_INITIAL_RESP:
		if (dpp_rx_gas_resp(auth, pos + 1,
				    len - 1) < 0)
			dpp_auth_deinit(auth);
		break;
	default:
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Ignore unsupported message type %u",
			   *pos);
		break;
	}
}

static void dpp_controller_rx(int sd, void *eloop_ctx, void *sock_ctx)
{
	struct dpp_authentication *auth = eloop_ctx;
	int res;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: TCP data available for reading (sock %d)",
		   sd);

	if (auth->msg_len_octets < 4) {
		u32 msglen;

		res = recv(sd, &auth->msg_len[auth->msg_len_octets],
			   4 - auth->msg_len_octets, 0);
		if (res < 0) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: recv failed: %s",
				   strerror(errno));
			dpp_auth_deinit(auth);
			return;
		}
		if (res == 0) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: No more data available over TCP");
			dpp_auth_deinit(auth);
			return;
		}
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Received %d/%d octet(s) of message length field",
			   res, (int) (4 - auth->msg_len_octets));
		auth->msg_len_octets += res;

		if (auth->msg_len_octets < 4) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Need %d more octets of message length field",
				   (int) (4 - auth->msg_len_octets));
			return;
		}

		msglen = WPA_GET_BE32(auth->msg_len);
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Message length: %u", msglen);
		if (msglen > 65535) {
			wpa_printf(MSG_INFO, "DPP: Unexpectedly long message");
			dpp_auth_deinit(auth);
			return;
		}

		wpabuf_free(auth->msg);
		auth->msg = wpabuf_alloc(msglen);
	}

	if (!auth->msg) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: No buffer available for receiving the message");
		dpp_auth_deinit(auth);
		return;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Need %u more octets of message payload",
		   (unsigned int) wpabuf_tailroom(auth->msg));

	res = recv(sd, wpabuf_put(auth->msg, 0), wpabuf_tailroom(auth->msg), 0);
	if (res < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: recv failed: %s", strerror(errno));
		dpp_auth_deinit(auth);
		return;
	}
	if (res == 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No more data available over TCP");
		dpp_auth_deinit(auth);
		return;
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Received %d octets", res);
	wpabuf_put(auth->msg, res);

	if (wpabuf_tailroom(auth->msg) > 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Need %u more octets of message payload",
			   (unsigned int) wpabuf_tailroom(auth->msg));
		return;
	}

	auth->msg_len_octets = 0;
	wpa_hexdump_buf(MSG_DEBUG, "DPP: Received TCP message", auth->msg);
	if (wpabuf_len(auth->msg) < 1) {
		dpp_auth_deinit(auth);
		return;
	}

	dpp_controller_process_rx((struct wifi_app *)auth->msg_ctx, (void *)auth, wpabuf_head(auth->msg), auth->peer_mac_addr, wpabuf_len(auth->msg));
}


static void dpp_controller_tcp_cb(int sd, void *eloop_ctx, void *sock_ctx)
{
	struct dpp_global *dpp = eloop_ctx;
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	int fd;
	struct dpp_authentication *auth = os_zalloc(sizeof(*auth));

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: New TCP connection");
	if (!auth)
		return;

	fd = accept(auth->sock, (struct sockaddr *) &addr, &addr_len);
	if (fd < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Failed to accept new connection: %s",
			   strerror(errno));
		os_free(auth);
		return;
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Connection from %s:%d",
		   inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));


	auth->sock = fd;

	if (fcntl(auth->sock, F_SETFL, O_NONBLOCK) != 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: fnctl(O_NONBLOCK) failed: %s",
			   strerror(errno));
		goto fail;
	}

	if (eloop_register_sock(auth->sock, EVENT_TYPE_READ,
				dpp_controller_rx, auth, NULL) < 0)
		goto fail;
	auth->read_eloop = 1;

	/* TODO: eloop timeout to expire connections that do not complete in
	 * reasonable time */
	if(wapp_dpp_auth_list_insert(dpp->msg_ctx, auth) < 0) {
		dpp_auth_deinit(auth);
			DBGPRINT(RT_DEBUG_OFF, "dpp auth list insert fail \n");
		auth = NULL;
		goto fail;
	}
	//dl_list_add(&ctrl->auth, &auth->list); kapil TODO
	return;

fail:
	close(fd);
	if (auth) {
		os_free(auth);
		auth = NULL;
	}
}

int dpp_map_init(struct dpp_global *dpp, struct dpp_authentication *auth, u8 *alid)
{
	const u8 *hdr, *pos, *end;
	//struct wifi_app *wapp = (struct wifi_app *)dpp->msg_ctx;

	auth->is_map_connection = 1;
	os_memcpy(auth->relay_mac_addr, alid, 6);

	hdr = wpabuf_head(auth->req_msg);
	end = hdr + wpabuf_len(auth->req_msg);
	hdr += 2; /* skip Category and Actiom */
	pos = hdr + DPP_HDR_LEN;
	auth->msg_out = dpp_map_encaps(hdr, pos, end - pos);
	if (!auth->msg_out)
		goto fail;

	auth->is_wired = TRUE;
#if 0
	if(!wapp->dpp->dpp_eth_onboard_ongoing)
	if (wapp->map->bh_type != MAP_BH_ETH)
#endif
#ifdef MAP_R3_RECONFIG
	if (auth->reconfigTrigger == TRUE) {
		auth->msg_out_pos = 0;
		dpp_wired_send(auth);
	}
#endif
	return 0;
fail:
	dpp_auth_deinit(auth);
	return -1;
}


int dpp_tcp_init(struct dpp_global *dpp, struct dpp_authentication *auth,
		 const struct wapp_ip_addr *addr, int port)
{
	struct sockaddr_storage saddr;
	socklen_t addrlen;
	const u8 *hdr, *pos, *end;
	char txt[100];

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Initialize TCP connection to %s port %d",
		   wapp_ip_txt(addr, txt, sizeof(txt)), port);
	if (dpp_ipaddr_to_sockaddr((struct sockaddr *) &saddr, &addrlen,
				   addr, port) < 0) {
		dpp_auth_deinit(auth);
		return -1;
	}

	//auth->global = dpp; kapil TODO
	auth->sock = socket(AF_INET, SOCK_STREAM, 0);
	if (auth->sock < 0)
		goto fail;

	if (fcntl(auth->sock, F_SETFL, O_NONBLOCK) != 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: fnctl(O_NONBLOCK) failed: %s",
			   strerror(errno));
		goto fail;
	}

	if (connect(auth->sock, (struct sockaddr *) &saddr, addrlen) < 0) {
		if (errno != EINPROGRESS) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to connect: %s",
				   strerror(errno));
			goto fail;
		}

		/*
		 * Continue connecting in the background; eloop will call us
		 * once the connection is ready (or failed).
		 */
	}

	if (eloop_register_sock(auth->sock, EVENT_TYPE_WRITE,
				dpp_conn_tx_ready, auth, NULL) < 0)
		goto fail;
	auth->write_eloop = 1;

	hdr = wpabuf_head(auth->req_msg);
	end = hdr + wpabuf_len(auth->req_msg);
	hdr += 2; /* skip Category and Actiom */
	pos = hdr + DPP_HDR_LEN;
	auth->msg_out = dpp_tcp_encaps(hdr, pos, end - pos);
	if (!auth->msg_out)
		goto fail;
	/* Message will be sent in dpp_conn_tx_ready() */

	/* TODO: eloop timeout to clear a connection if it does not complete
	 * properly */
	dl_list_add(&dpp->tcp_init, &auth->list);
	return 0;
fail:
	dpp_auth_deinit(auth);
	return -1;
}


int dpp_controller_start(struct dpp_global *dpp)
{
	int on = 1;
	struct sockaddr_in sin;
	int port;
	struct wifi_app *wapp = NULL;

	if (!dpp)
		return -1;
	wapp = (struct wifi_app *)dpp->msg_ctx;
	dpp->qr_mutual = 0;
	/* wired based controller only role */
	// TODO add both if needed later, kapil
	dpp->dpp_allowed_roles = DPP_CAPAB_CONFIGURATOR;

	if (wapp->map) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"map based controller, no need for IP connections\n");
		return 0;
	}


	dpp->sock = socket(AF_INET, SOCK_STREAM, 0);
	if (dpp->sock < 0)
		goto fail;

	if (setsockopt(dpp->sock, SOL_SOCKET, SO_REUSEADDR,
		       &on, sizeof(on)) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: setsockopt(SO_REUSEADDR) failed: %s",
			   strerror(errno));
		/* try to continue anyway */
	}

	if (fcntl(dpp->sock, F_SETFL, O_NONBLOCK) < 0) {
		wpa_printf(MSG_INFO, "DPP: fnctl(O_NONBLOCK) failed: %s",
			   strerror(errno));
		goto fail;
	}

	/* TODO: IPv6 */
	os_memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	port = dpp->tcp_port ? dpp->tcp_port : DPP_TCP_PORT;
	sin.sin_port = htons(port);
	if (bind(dpp->sock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
		wpa_printf(MSG_INFO,
			   "DPP: Failed to bind Controller TCP port: %s",
			   strerror(errno));
		goto fail;
	}
	if (listen(dpp->sock, 10 /* max backlog */) < 0 ||
	    fcntl(dpp->sock, F_SETFL, O_NONBLOCK) < 0 ||
	    eloop_register_sock(dpp->sock, EVENT_TYPE_READ,
				dpp_controller_tcp_cb, dpp, NULL))
		goto fail;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Controller started on TCP port %d", port);
	return 0;
fail:
	return -1;
}

void dpp_controller_stop(struct dpp_global *dpp)
{
	if (dpp) {
		//kapil TODO
	}
}

#endif /* CONFIG_DPP2 */

#if 0
int wapp_dpp_add_controllers(struct dpp_controller_conf *ctrl)
{
#ifdef CONFIG_DPP2
	struct dpp_relay_config config;

	os_memset(&config, 0, sizeof(config));
	config.cb_ctx = wapp;
	config.tx = wapp_dpp_relay_tx;
	config.gas_resp_tx = wapp_dpp_relay_gas_resp_tx;

	config.ipaddr = ctrl->ipaddr;
	config.pkhash = ctrl->pkhash;
	if (dpp_relay_add_controller(wapp->dpp,
				     &config) < 0)
		return -1;
#endif /* CONFIG_DPP2 */

	return 0;
}
#endif

