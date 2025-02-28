#include <sys/un.h>
#include <sys/time.h>
#include "wapp_usr_intf_ctrl.h"
#include "includes.h"
#include "interface.h"
#include "common.h"
#include "client_db.h"
#include "mapd_i.h"
#include "wapp_if.h"
#ifdef SUPPORT_MULTI_AP
#include "data_def.h"
#include "topologySrv.h"
#else
#include "ieee80211.h"
#endif
#include "client_mon.h"
#include "ap_est.h"
#include "eloop.h"
#ifdef SUPPORT_MULTI_AP
#include "1905_if.h"
#include "apSelection.h"
#include "1905_map_interface.h"
#include "mapfilter_if.h"
#include "mapd_user_iface.h"
#endif
#include "steer_action.h"
#ifdef VENDOR1_FEATURE_EXTEND
#include "mapd_user_iface.h"
#include "ctrl_iface.h"
#endif /*VENDOR1_FEATURE_EXTEND*/
#include "ap_roam_algo.h"
#include "chan_mon.h"
#ifdef CENT_STR
#include "ap_cent_str.h"
#endif

#ifdef MAP_R2
#define MAX_PKT_LEN 8192
#else
#define MAX_PKT_LEN 3072
#endif

#ifdef DATA_ELEMENT_SUPPORT
#include "de2json.h"
#endif

extern u8 ZERO_MAC_ADDR[ETH_ALEN];

struct wapp_usr_intf_ctrl *ctrl_conn;

static int wapp_send_incr_conf(struct wapp_usr_intf_ctrl *ctrl, char incr_decr);
static int wapp_send_buf_incr_evt(struct mapd_global *global, int buf_len);


/* Process events from WAPP, which could be :-
 * 1)Solicited WAPP Responses to commands from 1905D, proxied by MAPD
 *    --wapp_event->type == type == non_zero; from == 1
 *    --Pass on to 1905D, and sniff specific responses
 * 2)Solicited WAPP Responses to commands from MAPD //Don't pass on to 1905D.
 *    --wapp_event->type == type = non_zero; from == 0
 *    --Consume
 * 3)Commands to 1905D from WAPP
 *    --type == non_zero; type != wapp_event->type;  wapp_event->type == {COMMANDS}; from = -1
 *    --type = 0; wapp_event->type == {COMMANDS}; from = -1
 *    --Send to 1905D; Wait for Response; Relay it to WAPP
 * 4)Unsolicited messages from WAPP // Pass on to 1905D, sniff specific messages.
 *    --type == non_zero; type != wapp_event->type;  wapp_event->type == {RESPONSES}; from = -1
 *    --type = 0; wapp_event->type == {Responses}; from = -1
 *    --Pass on to 1905D, and sniff specific responses
 * */

void wlanif_handle_sta_activity_status(void *eloop_ctx, void *timeout_ctx)
{

	struct msg *tlv_msg = (struct msg *) timeout_ctx;
	mapd_hexdump(MSG_DEBUG, "receive WAPP_STA_STAT event", (char *)tlv_msg,
			tlv_msg->length + TLV_BUFFER_OFFSET);

	mapd_printf(MSG_DEBUG, "Ignoring Activity change from WAPP");
	os_free(timeout_ctx);
	return;
}

#ifdef SUPPORT_MULTI_AP
void wlanif_handle_bh_config_event(void *eloop_ctx, void *timeout_ctx)
{

	unsigned int profile_idx = 0;
	struct mapd_global *global = (struct mapd_global *) eloop_ctx;
	struct msg *tlv_msg = (struct msg *) timeout_ctx;
	struct own_1905_device *ctx = &global->dev;
	struct wsc_apcli_config_msg *bh_configs_msg = (struct wsc_apcli_config_msg *)tlv_msg->buffer;

	if(bh_configs_msg->profile_count == 0) {
		eloop_cancel_timeout(issue_connect_timeout_handle, &global->dev, NULL);
		os_memcpy(ctx->conn_attempted_mac, ZERO_MAC_ADDR, ETH_ALEN);
	} else if (bh_configs_msg->profile_count == WSC_APCLI_CONFIG_MSG_SAVE_PROFILE) {
		eloop_cancel_timeout(issue_connect_timeout_handle, &global->dev, NULL);
		ctx->wsc_save_bh_profile = TRUE;
		/** set timeout to 160 as WSC_TIMEOUT defined in wappd is 150 , give a time gap of 10 seconds */
		eloop_register_timeout(160, 0, ap_selection_retrigger_scan, ctx, NULL);
		if (timeout_ctx) {
			os_free(timeout_ctx);
		}
		return;
	} else if (bh_configs_msg->profile_count == WSC_APCLI_CONFIG_MSG_USE_SAVED_PROFILE) {
		eloop_cancel_timeout(issue_connect_timeout_handle, &global->dev, NULL);
		eloop_register_timeout(5, 0, ap_selection_retrigger_scan, ctx, NULL);
		if (timeout_ctx) {
			os_free(timeout_ctx);
		}
		return;
	}

	while (profile_idx < bh_configs_msg->profile_count)
	{
		os_memset(&global->dev.bh_configs[profile_idx],
			0, sizeof(struct wsc_apcli_config));
		os_memcpy(&global->dev.bh_configs[profile_idx],
			&bh_configs_msg->apcli_config[profile_idx],sizeof(struct wsc_apcli_config));
		err("BH SSID = %s", bh_configs_msg->apcli_config[profile_idx].ssid);
		profile_idx++;
	}
	global->dev.bh_config_count = bh_configs_msg->profile_count;
	os_free(timeout_ctx);
	if(global->dev.bh_config_count
		&& global->dev.current_bh_state != BH_STATE_WIFI_LINKUP
		&& global->dev.current_bh_state !=BH_STATE_ETHERNET_PLUGGED) {
			ap_selection_reset_scan(&global->dev);
			if (global->dev.current_bh_state != BH_STATE_ETHERNET_UPLUGGED) {
				global->dev.current_bh_state = BH_STATE_WIFI_BOOTUP;
				mapd_printf(MSG_OFF, "****bh_state = BH_STATE_WIFI_BOOTUP****** BH Profile avail(wait for 2 seconds for ETH)");
				eloop_register_timeout(2,0,
								ap_selection_retrigger_scan,
								&global->dev,NULL);
			} else {
				eloop_register_timeout(0,0,
								ap_selection_retrigger_scan,
								&global->dev,NULL);
			}
	}
	return;
}

void wlanif_handle_br_ip_event(void *eloop_ctx, void *timeout_ctx)
{

	struct mapd_global *global = (struct mapd_global *) eloop_ctx;
	struct msg *tlv_msg = (struct msg *) timeout_ctx;
	char *ip_buf = (char *)tlv_msg->buffer;
	if (strlen(ip_buf) <= 0 ) {
		err("get ip length is zero!\n");
		os_free(timeout_ctx);
		return;
	}
	memset(global->dev.ipbuf, 0, sizeof(global->dev.ipbuf));
	memcpy(global->dev.ipbuf, ip_buf, strlen(ip_buf) + 1);
	mapd_printf(MSG_OFF,"get ip is: %s!\n", global->dev.ipbuf);
	os_free(timeout_ctx);

	return;
}

void wlanif_handle_set_bh_type(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *) eloop_ctx;
	struct own_1905_device *ctx = &global->dev;
	struct msg *tlv_msg = (struct msg *) timeout_ctx;
	struct bh_type_info *bh_type = (struct bh_type_info *) tlv_msg->buffer;

	if (!ctx->auto_bh_switch)
	{
		if (bh_type->type == MAP_BH_ETH)
		{
			ctx->current_bh_state = BH_STATE_ETHERNET_PLUGGED;
		} else {
			if (ctx->current_bh_state == BH_STATE_ETHERNET_PLUGGED) {
				ctx->current_bh_state = BH_STATE_ETHERNET_UPLUGGED;
				mapd_printf(MSG_OFF, "\nLocal Eth disconnected!\n");
				err("Start WiFi based onboarding");
				ctx->link_fail_single_channel_scan_count = 3;
				wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr,
								WAPP_USER_SET_ETH_BH_DOWN,
								0, NULL, NULL, NULL, 0, 0, 0, 0);
				wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_BH_WIRELESS_SETTING,
						WAPP_MAP_BH_CONFIG, NULL, NULL, NULL, 0, 0, 1, 0);
			} else {
				err("WiFi onboarding triggered when ethernet is not previously plugged");
			}
		}
	}
	os_free(timeout_ctx);
	return;
}
#endif
/* OK */
void wlanif_handle_sta_cnnct_rej_info(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *) eloop_ctx;
	struct msg *tlv_msg = (struct msg *) timeout_ctx;
	wapp_sta_cnnct_rej_info *rej_cnnct_info =
			(wapp_sta_cnnct_rej_info *)tlv_msg->buffer;

	mapd_hexdump(MSG_MSGDUMP, "WAPP_STA_CNNCT_REJ_INFO event", (char *)tlv_msg,
					tlv_msg->length + TLV_BUFFER_OFFSET);

#ifdef MAP_R2
	client_mon_handle_auth_rej(global, rej_cnnct_info);
#else
	client_mon_handle_auth_rej(global, rej_cnnct_info->sta_mac);
#endif
	os_free(timeout_ctx);
}

/* OK */
void wlanif_handle_update_probe_info(void *eloop_ctx, void *timeout_ctx)
{
	struct os_reltime now = {0};
	struct mapd_global *global = (struct mapd_global *) eloop_ctx;
	struct msg *tlv_msg = (struct msg *) timeout_ctx;
	if (timeout_ctx == NULL) {
		err("Buffer NULL");
		return;
	}
	wapp_probe_info *probe_evt = (wapp_probe_info *) tlv_msg->buffer;
	os_get_reltime(&now);
	mapd_hexdump(MSG_MSGDUMP, "receive WAPP_UPDATE_PROBE_INFO event",
					(char *)tlv_msg, tlv_msg->length + TLV_BUFFER_OFFSET);

	client_mon_handle_client_preq(global, probe_evt->mac_addr, probe_evt->channel,
			probe_evt->rssi, probe_evt->preq_len, probe_evt->preq, now);

	if (timeout_ctx)
		os_free(timeout_ctx);
}

/* OK */
void wlanif_handle_traffic_stats(void *eloop_ctx, void *timeout_ctx)
{

    struct mapd_global *global = (struct mapd_global *) eloop_ctx;
	struct event_wrapper_s *event_wrapper = timeout_ctx;
	struct msg *tlv_msg = (struct msg *) event_wrapper->event;
#ifdef SUPPORT_MULTI_AP
	int from = event_wrapper->from;
#endif
    struct sta_traffic_stats *stats = (struct sta_traffic_stats *) tlv_msg->buffer;

    mapd_hexdump(MSG_MSGDUMP, "WAPP_ALL_ASSOC_STA_TRAFFIC_STATS event",
					(char *)tlv_msg, tlv_msg->length + TLV_BUFFER_OFFSET);
#ifdef SUPPORT_MULTI_AP
	topo_srv_parse_wapp_all_sta_traffic_stats(global,
		stats, from);
#endif
	mapd_handle_traffic_stats(global, stats);
	os_free(tlv_msg);
	os_free(timeout_ctx);
}

#ifdef MAP_R3_WF6
void wlanif_handle_assoc_wifi6_sta_status(void *eloop_ctx, void *timeout_ctx)
{
	/* TODO: Implementation pending. */
	struct mapd_global *global = (struct mapd_global *) eloop_ctx;
	struct event_wrapper_s *event_wrapper = timeout_ctx;
	struct msg *tlv_msg = (struct msg *) event_wrapper->event;
#ifdef SUPPORT_MULTI_AP
	int from = event_wrapper->from;
#endif
	struct assoc_wifi6_sta_status *status = (struct assoc_wifi6_sta_status *) tlv_msg->buffer;

	mapd_hexdump(MSG_MSGDUMP, "WAPP_ALL_ASSOC_STA_TRAFFIC_STATS event",
			(char *)tlv_msg, tlv_msg->length + TLV_BUFFER_OFFSET);
#ifdef SUPPORT_MULTI_AP
	topo_srv_parse_wapp_assoc_wifi6_sta_status(global,
			status, from);
#endif
	//mapd_handle_assoc_wifi6_sta_status(global, status); I guess not needed for API.
	os_free(tlv_msg);
	os_free(timeout_ctx);
}
#endif

/* OK */
static void wlanif_handle_sta_link_metrics(struct mapd_global *global,
		struct link_metrics *l_metrics)
{
	mapd_printf(MSG_DEBUG, MACSTR " DL Data rate=%d UL Data rate=%d"
					"RSSI=%lddBm", MAC2STR(l_metrics->mac),
					l_metrics->erate_downlink, l_metrics->erate_uplink,
					(signed long)l_metrics->rssi_uplink);

	/* Ignore BACKHAUL STAs */
#if 0
	if (is_local_adm_ether_addr(l_metrics->mac))
		return ;
#endif

	if (l_metrics->is_APCLI)
		return;

	client_mon_handle_link_metrics(global, l_metrics->mac,
					l_metrics->erate_downlink,
					(int8_t)l_metrics->rssi_uplink, l_metrics->bssid);
}

/* OK */
static void wlanif_handle_sta_tp_metrics(struct mapd_global *global,
		struct tp_metrics *tp_metrics)
{
	mapd_printf(MSG_DEBUG, MACSTR " Tx_TP=%d, Rx_TP=%d",
					MAC2STR(tp_metrics->mac),
					tp_metrics->tx_tp, tp_metrics->rx_tp);

	/* Ignore BACKHAUL STAs */
	if (tp_metrics->is_APCLI)
			return ;

	client_mon_handle_tp_metrics(global, tp_metrics->mac,
					tp_metrics->tx_tp, tp_metrics->rx_tp,
					tp_metrics->bssid);
}

void wlanif_beacon_metrics_query(struct mapd_global *global, u8 *sta_mac,
				u8 *assoc_bssid, u8 ssid_len, u8 *ssid,
				u8 channel, u8 op_class, u8 *bssid,
				u8 rpt_detail, u8 num_elem, char *elem_list,
				u8 num_chrep, struct ap_chn_rpt *chan_rpt)
{
	struct beacon_metrics_query *bcn_query = NULL;
	struct ap_chn_rpt *chn_rpt = NULL;
	uint8_t i = 0;

	mapd_printf(MSG_ERROR, "Preparing to send BeaconReq");

	bcn_query = (struct beacon_metrics_query *)
			os_zalloc(sizeof(struct beacon_metrics_query) + num_chrep * sizeof(struct ap_chn_rpt));
	if (!bcn_query) {
		mapd_printf(MSG_ERROR, "FAILED OOM");
		return;
	}

	os_memcpy(bcn_query->sta_mac, sta_mac, ETH_ALEN);
	os_memcpy(bcn_query->bssid, bssid, ETH_ALEN);
	bcn_query->ssid_len = ssid_len;
	bcn_query->ch = channel;
	bcn_query->oper_class = op_class;
	os_memcpy(bcn_query->ssid, ssid, ssid_len);

	bcn_query->rpt_detail_val = rpt_detail;

	/*ap channel report info*/
	bcn_query->ap_ch_rpt_num = num_chrep;
	chn_rpt = bcn_query->rpt;
	for (i = 0; i < num_chrep; i++) {
			chn_rpt->ch_rpt_len = chan_rpt->ch_rpt_len;
			chn_rpt->oper_class = chan_rpt->oper_class;
			memcpy(chn_rpt->ch_list, chan_rpt->ch_list, chan_rpt->ch_rpt_len - 1);
			chn_rpt++;
	}

	bcn_query->elemnt_num = num_elem;
	if (num_elem > MAX_ELEMNT_NUM)
			bcn_query->elemnt_num = MAX_ELEMNT_NUM;
	os_memcpy(bcn_query->elemnt_list, elem_list, bcn_query->elemnt_num);

	wlanif_issue_wapp_command(global, WAPP_USER_SET_BEACON_METRICS_QRY,
					WAPP_BEACON_METRICS_REPORT, assoc_bssid, NULL, (void *)bcn_query,
					sizeof(struct beacon_metrics_query), 0, 0, 0);
	os_free(bcn_query);
}



/* OK */
void wlanif_handle_all_assoc_sta_link_metrics(void *eloop_ctx, void *timeout_ctx)
{
    uint32_t i = 0;
    struct mapd_global *global = (struct mapd_global *) eloop_ctx;
	struct event_wrapper_s *event_wrapper = timeout_ctx;
    struct msg *tlv_msg = (struct msg *) event_wrapper->event;
#ifdef SUPPORT_MULTI_AP
	int from = event_wrapper->from;
#endif
    struct sta_link_metrics *metrics = (struct sta_link_metrics *) tlv_msg->buffer;

    mapd_hexdump(MSG_MSGDUMP, "receive WAPP_ALL_ASSOC_STA_LINK_METRICS event",
					(char *)tlv_msg, tlv_msg->length + TLV_BUFFER_OFFSET);
    mapd_printf(MSG_DEBUG, "%s: Radio id:" MACSTR "sta_cnt=%d", __func__,
					MAC2STR(metrics->identifier), metrics->sta_cnt);
    for (i = 0; i < metrics->sta_cnt; i++) {
        struct link_metrics *l_metrics = &metrics->info[i];
		wlanif_handle_sta_link_metrics(global, l_metrics);
    }
#ifdef SUPPORT_MULTI_AP
	topo_srv_parse_wapp_all_assoc_link_metric(global, metrics,
		from);
#endif
	os_free(tlv_msg);
	os_free(timeout_ctx);
}

/* OK */
void wlanif_handle_all_assoc_sta_tp_metrics(void *eloop_ctx, void *timeout_ctx)
{
    uint32_t i = 0;
    struct mapd_global *global = (struct mapd_global *) eloop_ctx;
    struct msg *tlv_msg = (struct msg *) timeout_ctx;
    struct sta_tp_metrics *metrics = (struct sta_tp_metrics *) tlv_msg->buffer;

    mapd_hexdump(MSG_MSGDUMP, "receive WAPP_ALL_ASSOC_TP_METRICS",
		    (char *)tlv_msg, tlv_msg->length + TLV_BUFFER_OFFSET);
    mapd_printf(MSG_DEBUG, "%s: Radio id:" MACSTR "sta_cnt=%d", __func__,
		    MAC2STR(metrics->identifier), metrics->sta_cnt);

    for (i = 0; i < metrics->sta_cnt; i++) {
        struct tp_metrics *tp_metric_info = &metrics->info[i];
		wlanif_handle_sta_tp_metrics(global, tp_metric_info);
    }
	os_free(timeout_ctx);
}

/* OK */
void wlanif_handle_one_assoc_sta_link_metrics(void *eloop_ctx, void *timeout_ctx)
{
	struct event_wrapper_s *event_wrapper = timeout_ctx;
    struct mapd_global *global = (struct mapd_global *) eloop_ctx;
    struct msg *tlv_msg = (struct msg *) event_wrapper->event;
#ifdef SUPPORT_MULTI_AP
	int from = event_wrapper->from;
#endif
    struct link_metrics *l_metrics = (struct link_metrics *) tlv_msg->buffer;

	mapd_hexdump(MSG_MSGDUMP, "receive WAPP_ONE_ASSOC_STA_LINK_METRICS event",
					(char *)tlv_msg, tlv_msg->length + TLV_BUFFER_OFFSET);
#ifdef SUPPORT_MULTI_AP
	topo_srv_parse_wapp_one_assoc_link_metric(global, l_metrics,
		from);
#endif
	wlanif_handle_sta_link_metrics(global, l_metrics);
	os_free(tlv_msg);
	os_free(timeout_ctx);
}
#ifdef SUPPORT_MULTI_AP
void wlanif_handle_rx_link_stats(void *eloop_ctx, void *timeout_ctx)
{
	mapd_printf(MSG_INFO, "%s: Handled", __func__);
	struct rx_link_stat_rsp *rx_link;
	struct mapd_global *global = (struct mapd_global *) eloop_ctx;
	struct backhaul_link_info *bh = global->dev.metric_entry.bh;
	struct msg *tlv_msg = (struct msg *) timeout_ctx;
	rx_link = (struct rx_link_stat_rsp *) tlv_msg->buffer;
	mapd_hexdump(MSG_MSGDUMP, "receive WAPP_RX_LINK_STATISTICS event",
		(char *)tlv_msg, tlv_msg->length + TLV_BUFFER_OFFSET);
	/* Rx link */
	if(bh) {
		debug("rx_link->packetErrors: %d", rx_link->pkt_errs);
		debug("rx_link->packetsReceived: %d", rx_link->rx_pkts);
		debug("rx_link->rssi: %d", rx_link->rssi);
		debug("rx_link->rx_tp: %d", rx_link->rx_tp);
		bh->rx.pkt_err = rx_link->pkt_errs;
		bh->rx.pkt_received = rx_link->rx_pkts;
		bh->rx.rssi = rx_link->rssi;
		bh->rx.rx_tp = rx_link->rx_tp;
	}
	os_free(timeout_ctx);
}

void wlanif_handle_tx_link_stats(void *eloop_ctx, void *timeout_ctx)
{
	mapd_printf(MSG_INFO, "Handled");
	struct tx_link_stat_rsp *tx_link;
	struct mapd_global *global = (struct mapd_global *) eloop_ctx;
	struct backhaul_link_info *bh = global->dev.metric_entry.bh;
	struct msg *tlv_msg = (struct msg *) timeout_ctx;
	tx_link = (struct tx_link_stat_rsp *) tlv_msg->buffer;
	mapd_hexdump(MSG_MSGDUMP, "receive WAPP_TX_LINK_STATISTICS event",
		(char *)tlv_msg, tlv_msg->length + TLV_BUFFER_OFFSET);
	/* Tx link */
	if(bh) {
		debug("tx_link->packetErrors: %d", tx_link->pkt_errs);
		debug("tx_link->transmittedPackets: %d", tx_link->tx_pkts);
		debug("tx_link->macThroughputCapacity: %d", tx_link->mac_tp_cap);
		debug("tx_link->linkAvailability: %d", tx_link->link_avail);
		debug("tx_link->phyRate: %d", tx_link->phyrate);
		debug("tx_link->tx_tp: %d", tx_link->tx_tp);
		bh->tx.pkt_err = tx_link->pkt_errs;
		bh->tx.tx_packet = tx_link->tx_pkts;
		bh->tx.mac_throughput = tx_link->mac_tp_cap;
		bh->tx.link_availability = tx_link->link_avail;
		bh->tx.phy_rate = tx_link->phyrate;
		bh->tx.tx_tp = tx_link->tx_tp;
	}
	os_free(timeout_ctx);
}

void wlanif_handle_unassoc_sta_link_metrics(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_unassoc_link_metric(global, (struct unlink_metrics_rsp *)tlv_msg->buffer);
	os_free(timeout_ctx);
}
#endif
void wlanif_handle_radio_basic_cap(void *eloop_ctx, void *timeout_ctx)
{

	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	struct ap_radio_basic_cap *ap_radio_basic_cap_rep = (struct ap_radio_basic_cap *)(tlv_msg->buffer);
	struct mapd_radio_info *radio_info = NULL;

	mapd_hexdump(MSG_DEBUG, "Receive Radio basic capability info", (char *)(tlv_msg), tlv_msg->length + 4);
#ifdef SUPPORT_MULTI_AP
	topo_srv_parse_wapp_radio_basic_cap(global,
		ap_radio_basic_cap_rep);
#endif
	//get radio info from identifier
	radio_info = get_radio_info_by_radio_id(global, ap_radio_basic_cap_rep->identifier);

	if(radio_info !=NULL)
		radio_info->wireless_mode = ap_radio_basic_cap_rep->wireless_mode;
	else
		mapd_printf(MSG_ERROR, "Identifier not found for which the report was received");

	os_free(timeout_ctx);
}
#ifdef SUPPORT_MULTI_AP
void update_apcli_info_in1905(struct mapd_global *global)
{
	struct bh_link_entry *bh_entry, *tbh_entry = NULL;
	struct channel_bw_info ch_bw_info_temp;
	struct _1905_map_device *own_1905_map_device = NULL;
	struct radio_info_db *radio = NULL;
	int status = 0;
	own_1905_map_device = topo_srv_get_1905_device(&global->dev,NULL);
	SLIST_FOREACH_SAFE(bh_entry, &global->dev.bh_link_head, next_bh_link, tbh_entry) {

		radio = topo_srv_get_radio(own_1905_map_device, bh_entry->radio_identifier);
		if (!radio) {
			err("peer_radio not found corresponding to identifier");
			return;
		}
		status = find_bhlink_bw_ch(&global->dev,radio,bh_entry,&ch_bw_info_temp);
		if(status < 0)
			continue;
		info("ch_bw_info mac address("MACSTR")", MAC2STR(ch_bw_info_temp.iface_addr));
		info("ch_bw_info.channel_num %d",ch_bw_info_temp.channel_num);
		info("ch_bw_info.channel_bw %d",ch_bw_info_temp.channel_bw);
		map_1905_Set_Channel_BandWidth(global->_1905_ctrl,
				&ch_bw_info_temp);
	}
}

void wlanif_handle_wireless_inf_info(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	char *recv_cmd_buf = (char *)tlv_msg->buffer;
	struct interface_info_list_hdr *interface_info_msg = NULL;
	struct interface_info *info = NULL;
	struct local_interface **itfs = NULL;
	int i = 0, ret = 0;
	char all_wintf_valid = 0, send_to_1905 = 0;

	mapd_hexdump(MSG_DEBUG, "Receive wireless_inf_info", (char *)(tlv_msg), tlv_msg->length + 4);

	all_wintf_valid = recv_cmd_buf[0];
	send_to_1905 = recv_cmd_buf[1];

	if (!all_wintf_valid) {
		global->wapp_wintf_status = 0;
		os_free(timeout_ctx);
		return;
	}
	global->wapp_wintf_status = 1;

	interface_info_msg = (struct interface_info_list_hdr *)(recv_cmd_buf + 2);
	info = interface_info_msg->if_info;
	if (send_to_1905) {
		ret = map_1905_Set_Wireless_Interface_Info(global->_1905_ctrl, interface_info_msg);
		while (ret < 0) {
			mapd_printf(MSG_ERROR, "send wireless interface to 1905 fail, need retry");
			os_sleep(1, 0);
			ret = map_1905_Set_Wireless_Interface_Info(global->_1905_ctrl, interface_info_msg);
		}
		os_free(timeout_ctx);
		return;
	}

	while (i < interface_info_msg->interface_count) {
		if (!os_strncmp((char *)info->if_role,"wista",os_strlen("wista"))) {
			struct bh_link_entry *bh = os_zalloc(sizeof(struct bh_link_entry));
			if(!bh) {
				err("mem alloc fail, return");
				os_free(timeout_ctx);
				return;
			}
			/*
			 * Check the configuration which will monitor
			 * the link based on rss and disconnection
			 */
			if (SLIST_EMPTY(&global->dev.bh_link_head)) {
				SLIST_INIT(&global->dev.bh_link_head);
			}

			bh->type = 1;
			bh->bh_channel = info->if_ch;
#ifdef MAP_6E_SUPPORT
			bh->bh_band = get_band_from_chan_op(info->if_ch, info->if_opclass);
#endif
			SLIST_INIT(&bh->scan_bss_list_head);

			memcpy(bh->ifname, info->if_name, IFNAMSIZ);
			memcpy(bh->mac_addr, info->if_mac_addr, ETH_ALEN);
			memcpy(bh->radio_identifier, info->identifier, ETH_ALEN);
			/* add this entry in list */
			SLIST_INSERT_HEAD(&global->dev.bh_link_head, bh, next_bh_link);
		}
		i++;
		info++;
	}
	info = interface_info_msg->if_info;
	if (interface_info_msg->interface_count > 0) {
		global->dev.wifi_itfs = (struct local_interface **)os_malloc(interface_info_msg->interface_count * sizeof(struct local_interface *));
		if (global->dev.wifi_itfs) {
			itfs = global->dev.wifi_itfs;

			for (i = 0; i < interface_info_msg->interface_count; i++) {
				itfs[i] = (struct local_interface *)os_malloc(sizeof(struct local_interface));
				os_memcpy(itfs[i]->name, info->if_name, IFNAMSIZ);
				os_memcpy(itfs[i]->mac, info->if_mac_addr, ETH_ALEN);
				if (!os_strncmp((char *)info->if_role,"wista",os_strlen("wista")))
					itfs[i]->dev_type = APCLI;
				else if (!os_strncmp((char *)info->if_role,"wiap",os_strlen("wiap")))
					itfs[i]->dev_type = AP;
				else
					itfs[i]->dev_type = ETH;
#ifdef MAP_6E_SUPPORT
				if (IS_MAP_CH_24G(info->if_ch) && IS_OP_CLASS_24G(info->if_opclass))
					itfs[i]->band = _24G;
				else if (IS_MAP_CH_5G(info->if_ch) && IS_OP_CLASS_5G(info->if_opclass))
					itfs[i]->band = _5G;
				else if (IS_MAP_CH_6G(info->if_ch) && IS_OP_CLASS_6G(info->if_opclass))
					itfs[i]->band = _6G;
#else
				if (info->if_ch <= 14)
					itfs[i]->band = _24G;
				else if (info->if_ch >= 32 && info->if_ch <= 64)
					itfs[i]->band = _5GL;
				else if (info->if_ch >= 100 && info->if_ch <= 161)
					itfs[i]->band = _5GH;
#endif
				printf("[%d] ifname:%s mac " MACSTR "type=%02x, band=%d\n", i,
					info->if_name, MAC2STR(info->if_mac_addr), itfs[i]->dev_type, itfs[i]->band);
				info++;
				global->dev.num_wifi_itfs++;
			}
		}
	}
	update_apcli_info_in1905(global);
	os_free(timeout_ctx);
}
#endif
void wlanif_handle_ap_ht_cap(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	uint8_t j = 0;
	struct ap_ht_capability *ap_ht_cap_rep =
			(struct ap_ht_capability *)(tlv_msg->buffer);

	mapd_hexdump(MSG_MSGDUMP, "WAPP_AP_HT_CAPABILITY",
					(char *)(tlv_msg), tlv_msg->length + TLV_BUFFER_OFFSET);
	//mapd_handle_ap_radio_cap();
#ifdef SUPPORT_MULTI_AP
	/* Update Status of Existing Radios */
	topo_srv_parse_wapp_ht_capability(global,
		(struct ap_ht_capability *)ap_ht_cap_rep);
#endif
	for (j = 0; j < MAX_NUM_OF_RADIO; j++) {
		struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[j];
		if (radio_info->radio_idx == (uint8_t)-1) {
			continue;
		}
		if (!os_memcmp(radio_info->identifier, ap_ht_cap_rep->identifier,
								ETH_ALEN)) {
			mapd_printf(MSG_INFO, "Found the reported radio " MACSTR,
								MAC2STR(ap_ht_cap_rep->identifier));
			os_memcpy(&radio_info->ht_capab, (char *)&ap_ht_cap_rep->tx_stream,
							sizeof(struct ht_cap));
			radio_info->ht_capab.tx_stream++;
			radio_info->ht_capab.rx_stream++;
			os_free(timeout_ctx);
			return;
		}
	}
	mapd_printf(MSG_ERROR, "Could not find the reported radio " MACSTR,
					MAC2STR(ap_ht_cap_rep->identifier));
	os_free(timeout_ctx);
	return;
}

void wlanif_handle_ap_vht_cap(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	uint8_t j = 0;
	struct ap_vht_capability *ap_vht_cap_rep = (struct ap_vht_capability *)(tlv_msg->buffer);

	mapd_hexdump(MSG_DEBUG, "Receive AP VHT Cap info", (char *)(tlv_msg), tlv_msg->length + 4);
#ifdef SUPPORT_MULTI_AP
	topo_srv_parse_wapp_vht_capability(global, ap_vht_cap_rep);
#endif
	/* Update Status of Existing Radios */
	for (j = 0; j < MAX_NUM_OF_RADIO; j++) {
		struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[j];
		if (radio_info->radio_idx == (uint8_t)-1) {
			continue;
		}
		if (!os_memcmp(radio_info->identifier, ap_vht_cap_rep->identifier,
								ETH_ALEN)) {
			mapd_printf(MSG_INFO, "Found the reported radio " MACSTR,
								MAC2STR(ap_vht_cap_rep->identifier));
			os_memcpy(&radio_info->vht_capab, (char *)&ap_vht_cap_rep->vht_tx_mcs,
							sizeof(struct vht_cap));
			radio_info->vht_capab.tx_stream++;
			radio_info->vht_capab.rx_stream++;
			os_free(timeout_ctx);
			return;
		}
	}
	mapd_printf(MSG_ERROR, "Could not find the reported radio " MACSTR,
					MAC2STR(ap_vht_cap_rep->identifier));
	os_free(timeout_ctx);
	return;
}

#ifdef MAP_R3_DE
void wlanif_handle_dev_inven_tlv(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	struct dev_inven *de = (struct dev_inven *)(tlv_msg->buffer);
#ifdef SUPPORT_MULTI_AP
	debug(TOPO_PREX"Mapd will be handling the event from WAPPD DE _TLV\n");
	topo_srv_parse_wapp_dev_inven_tlv(global, de);
#endif
	os_free(timeout_ctx);
	return;
}
#endif /*MAP_R3_DE*/

void wlanif_handle_ap_he_cap(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	uint8_t j = 0;
	struct ap_he_capability *ap_he_cap_rep = (struct ap_he_capability *)(tlv_msg->buffer);
#ifdef SUPPORT_MULTI_AP
	topo_srv_parse_wapp_he_capability(global, ap_he_cap_rep);
#endif
	for (j = 0; j < MAX_NUM_OF_RADIO; j++) {
		struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[j];
		if (radio_info->radio_idx == (uint8_t)-1) {
			continue;
		}
		if (!os_memcmp(radio_info->identifier, ap_he_cap_rep->identifier,
								ETH_ALEN)) {
			mapd_printf(MSG_INFO, "Found the reported radio " MACSTR,
								MAC2STR(ap_he_cap_rep->identifier));
			os_memcpy(&radio_info->he_capab, (char *)&ap_he_cap_rep->he_mcs_len,
							sizeof(struct he_cap));
			radio_info->he_capab.tx_stream++;
			radio_info->he_capab.rx_stream++;
			os_free(timeout_ctx);
			return;
		}
	}
	mapd_printf(MSG_ERROR, "Could not find the reported radio " MACSTR,
					MAC2STR(ap_he_cap_rep->identifier));
	os_free(timeout_ctx);
	return;
}

#ifdef MAP_R3_WF6
void wlanif_handle_ap_wf6_cap(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	struct ap_wf6_cap_roles *ap_wf6_cap = (struct ap_wf6_cap_roles *)(tlv_msg->buffer);
#ifdef SUPPORT_MULTI_AP
	debug("WF6:MAPD:%s mapd will be handling the event from WAPPD for Ap capa\n", __func__);
	topo_srv_parse_wapp_wf6_capability(global, ap_wf6_cap);
#endif
	os_free(timeout_ctx);
	return;
}
#endif /*MAP_R3_WF6*/


#ifdef MAP_R4_SPT
void wlanif_handle_ap_spt_reuse_req(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	struct ap_spt_reuse_req *ap_spt_reuse = (struct ap_spt_reuse_req *)(tlv_msg->buffer);

#ifdef SUPPORT_MULTI_AP
		debug("DM:MAPD:%s mapd will be handling the event from WAPPD for Ap capa\n", __func__);
		topo_srv_parse_spt_reuse_req(global, ap_spt_reuse);
#endif
	os_free(timeout_ctx);
	return;
}

void wlanif_handle_ch_select_info(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	struct ap_spt_reuse_req *ap_spt_reuse = (struct ap_spt_reuse_req *)(tlv_msg->buffer);

	debug("DM:MAPD:%s mapd will be handling the event from WAPPD for sr select\n", __func__);
	topo_srv_parse_ch_selection_info(global, ap_spt_reuse);

	os_free(timeout_ctx);
	return;
}

void wlanif_handle_uplink_status_event(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	struct uplink_traffic_status *up_traff_status = (struct uplink_traffic_status *)(tlv_msg->buffer);

	topo_srv_parse_uplink_traffic_info(global, up_traff_status);
	os_free(timeout_ctx);
	return;
}
#endif /* MAP_R4_SPT*/

void wlanif_handle_nac_info(void *eloop_ctx, void *timeout_ctx)
{
	//TBD by SS5

	os_free(timeout_ctx);
}

extern void steer_action_handle_steer_timeout(void *eloop_ctx, void *timeout_ctx);

void wlanif_handle_steer_btm_report(void *eloop_ctx, void *timeout_ctx)
{
	struct msg *tlv_msg = (struct msg *) timeout_ctx;
	mapd_hexdump(MSG_ERROR, "[Steer] receive btm report", (char *)tlv_msg,
					tlv_msg->length + TLV_BUFFER_OFFSET);
	struct cli_steer_btm_event *btm_event = (struct cli_steer_btm_event *) tlv_msg->buffer;
	struct bss_info_db *bss;
	uint8_t target_op_chan = 0;
	uint8_t target_op_class = 0;
	struct mapd_global *global = (struct mapd_global *) eloop_ctx;
	struct client *cli = NULL;
	struct mapd_bss *connected_bss = NULL;

#ifdef SUPPORT_MULTI_AP
	topo_srv_parse_wapp_cli_steer_btm_report(eloop_ctx, btm_event);
#endif
	{
		mapd_printf(MSG_ERROR, "[Steer] ------------------------ BTM RESP----------------------------");
		mapd_printf(MSG_ERROR, "[Steer] STA = " MACSTR "Source BSSID = " MACSTR,
						MAC2STR(btm_event->sta_mac), MAC2STR(btm_event->bssid));
		mapd_printf(MSG_ERROR, "[Steer] Status=%d tbssid= " MACSTR,
						btm_event->status, MAC2STR(btm_event->tbssid));
		mapd_printf(MSG_ERROR, "[Steer] -----------------------------------------------------------------");

		mapd_printf(MSG_ERROR, "[Steer] Recv BTM Response STA:" MACSTR " Target:" MACSTR " status(%d)" ,
			MAC2STR(btm_event->sta_mac), MAC2STR(btm_event->tbssid), btm_event->status);

		cli = client_db_get_client_from_sta_mac(global, btm_event->sta_mac);
		if(!cli) {
			mapd_printf(MSG_ERROR, "cli is not in DB:" MACSTR, MAC2STR(btm_event->sta_mac));
			os_free(timeout_ctx);
			return;
		}

		connected_bss = mapd_get_bss_from_bssid(global, cli->bssid);
		if(!connected_bss) {
			mapd_printf(MSG_ERROR, "cli has already left current dev:" MACSTR, MAC2STR(btm_event->sta_mac));
			cli->btm_req_retry_count = global->dev.Btm_Retry_Time;
			os_free(timeout_ctx);
			return;
		}

#ifdef SUPPORT_MULTI_AP
		bss = topo_srv_get_bss_by_bssid(&global->dev,NULL, cli->exec_mon_data.target_bssid);
#else
		bss = mapd_get_bss_from_mac(global,cli->exec_mon_data.target_bssid );
#endif
		if(bss == NULL) {
			mapd_printf(MSG_ERROR, "target_bssid is invalid:" MACSTR, MAC2STR(cli->exec_mon_data.target_bssid));
			cli->btm_req_retry_count = global->dev.Btm_Retry_Time;
			os_free(timeout_ctx);
			return;
		}
	
		if(!os_memcmp(cli->bssid, cli->exec_mon_data.target_bssid, ETH_ALEN)) {
			mapd_printf(MSG_ERROR, "cli has already connect to target bssid, bs case" MACSTR, MAC2STR(cli->bssid));
			cli->btm_req_retry_count = global->dev.Btm_Retry_Time;
			os_free(timeout_ctx);
			return;
		}
		
		if (btm_event->status && cli->btm_req_retry_count < global->dev.Btm_Retry_Time) {
			mapd_printf(MSG_ERROR, "Retry BTM Request to" MACSTR " retry count = %d",
				MAC2STR(btm_event->sta_mac), cli->btm_req_retry_count);

		
			if (eloop_is_timeout_registered(steer_action_handle_steer_timeout,
							(void *)global, cli)) {
				mapd_printf(MSG_OFF, "Cancel steer_action_handle_steer");
				eloop_cancel_timeout(steer_action_handle_steer_timeout, global, cli);
			}

			if(bss == NULL){
				} else {
#ifdef SUPPORT_MULTI_AP
					if (cli->meas_data.cli_measurement_state ==
						MEAS_STATE_AIR_MON_TRIGGERED) {
						target_op_chan = cli->current_chan;
						err("Air monitor was triggered, channel = %d", target_op_chan);
					} else if
#else
					if
#endif
						(cli->meas_data.cli_measurement_state ==
						MEAS_STATE_11K_TRIGGERED) {
						target_op_chan = ap_roam_algo_get_bss_channel(bss,
							cli);
						err("11K was triggered, channel = %d", target_op_chan);
					} else {
						/* idle BTM steering case when no 11k or air monitor measurement is required */
						target_op_chan = (uint8_t) bss->radio->channel[0];
					}

					if( target_op_chan != cli->current_chan ){
						mapd_printf(MSG_ERROR, "Channel Invalid. change to target_op_chan:%d => current_chan:%d ", target_op_chan, cli->current_chan);
						target_op_chan = cli->current_chan;
					}
					target_op_class = chan_mon_get_op_class_frm_channel(target_op_chan, BW_20);
				}

				cli->btm_req_retry_count++;

				wlanif_trigger_btm_req(global, cli->mac_addr, cli->bssid,
						cli->exec_mon_data.target_bssid, target_op_chan,
						target_op_class, 0, 1, 0);

				eloop_register_timeout(global->dev.cli_steer_params.BTMStrTimeout, 0,
						steer_action_handle_steer_timeout, global, cli);
		}
	}
#ifdef VENDOR1_FEATURE_EXTEND
	{
		char *ssid = NULL;
		struct mapd_user_iface_get_btm_response_event *client_notif = NULL;
		struct mapd_user_event *user_event = NULL;
		struct ctrl_iface_global_priv *priv = global->ctrl_iface;
		struct mapd_bss *bss = mapd_get_bss_from_mac(global,btm_event->bssid);

		user_event = (struct mapd_user_event *)os_zalloc(sizeof(struct mapd_user_event) +
									sizeof(struct mapd_user_iface_get_btm_response_event));
		if (!user_event) {
			err("[Steer] mem alloc failed");
			if (timeout_ctx)
				os_free(timeout_ctx);
			return;
		}
		os_memset(user_event, 0, sizeof(struct mapd_user_event) +
			sizeof(struct mapd_user_iface_get_btm_response_event));

		user_event->event_id = GET_BTM_RESPONSE_NOTIF;
		client_notif = (struct mapd_user_iface_get_btm_response_event *)user_event->event_body;

		client_notif->band = (bss->channel < 14)? 2 : 5;

		os_memcpy(client_notif->sta_mac, btm_event->sta_mac, ETH_ALEN);
		//os_memcpy(client_notif->bssid, btm_event->bssid, ETH_ALEN);

		ssid = (char *) mapd_get_ssid_from_bssid(global, btm_event->bssid);
		os_memcpy(client_notif->ssid, ssid, strlen(ssid));

		client_notif->status_code = btm_event->status;

		if (!dl_list_empty(&priv->ctrl_dst)) {
			mapd_ctrl_iface_send(global,
						priv->sock,
						&priv->ctrl_dst,
						(const char *)user_event, sizeof(struct mapd_user_event) +
							sizeof(struct mapd_user_iface_get_btm_response_event),
						priv);
		}
		os_free(user_event);
	}
#endif /*VENDOR1_FEATURE_EXTEND*/
	/* Populate internal structures */
	os_free(timeout_ctx);
}

void wlanif_handle_cli_cap_report(void *eloop_ctx, void *timeout_ctx)
{
	//TBD by SS5
	os_free(timeout_ctx);
}

void wlanif_handle_bcn_metrics_report(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *) eloop_ctx;
	struct msg *tlv_msg = (struct msg *) timeout_ctx;
	uint8_t isSuccess = 1, islastreport= 0;
	PEID_STRUCT eid_ptr = NULL;
	struct beacon_metrics_rsp *bcn_rpt = (struct beacon_metrics_rsp *) tlv_msg->buffer;
	uint8_t	bcnrpt_cnt = 0;
	prrm_beacon_rep_info pBcnRep = NULL;
	struct client *cli = NULL;
	struct _1905_map_device *dev = NULL;

#ifdef VENDOR1_FEATURE_EXTEND
	prrm_beacon_rep_info pOwnerBcnRep = NULL;
	u8 bFindOwner = 0;
#endif //VENDOR1_FEATURE_EXTEND
	unsigned short parse_len = 0;

    mapd_hexdump(MSG_MSGDUMP, "WAPP_BEACON_METRICS_REPORT", (char *)tlv_msg,
		    tlv_msg->length + TLV_BUFFER_OFFSET);
	cli = client_db_get_client_from_sta_mac(global, bcn_rpt->sta_mac);
	if(cli) {
		if (cli->beacon_report_ie) {
			os_free(cli->beacon_report_ie);
			cli->beacon_report_ie = NULL;
			cli->beacon_report_ie_len = 0;
			cli->num_beacon_report = 0;
		}
		cli->beacon_report_ie = os_zalloc(bcn_rpt->rpt_len);
		if (cli->beacon_report_ie) {
			os_memcpy(cli->beacon_report_ie, bcn_rpt->rpt, bcn_rpt->rpt_len);
			cli->beacon_report_ie_len = bcn_rpt->rpt_len;
 			cli->num_beacon_report = bcn_rpt->bcn_rpt_num;
		} else {
			err("alloc beacon_report_ie for cli("MACSTR") fail. ie_len(%d)",
				MAC2STR(cli->mac_addr), bcn_rpt->rpt_len);
		}

	}
	if(cli)
		dev = topo_srv_get_1905_by_bssid(&global->dev,cli->bssid);

#ifdef VENDOR1_FEATURE_EXTEND
	/* Only Beacon Reports containing 2 or more RCPIs are allowed  */
	if ((bcn_rpt->rpt_len == 0) || (bcn_rpt->bcn_rpt_num <=1)){
		isSuccess = 0;

		mapd_printf(MSG_ERROR, "BSSID " MACSTR " Invalid Beacon Report num:%d len:%d",
			MAC2STR(bcn_rpt->sta_mac), bcn_rpt->bcn_rpt_num, bcn_rpt->rpt_len );

		ap_est_handle_11k_report(global, bcn_rpt->sta_mac, isSuccess,
					NULL, 0, 0, 0);
		os_free(timeout_ctx);
		return;
	} else
#endif //VENDOR1_FEATURE_EXTEND
	{
		mapd_printf(MSG_INFO, "From " MACSTR " Num=%d len=%d",
					MAC2STR(bcn_rpt->sta_mac), bcn_rpt->bcn_rpt_num, bcn_rpt->rpt_len);
	}
#ifdef SUPPORT_MULTI_AP
	if((!global->dev.cent_str_en) || (global->dev.cent_str_en && global->dev.device_role == DEVICE_ROLE_AGENT))
		topo_srv_parse_wapp_beacon_metrics_report(global, bcn_rpt);
#endif
	if ((!global->dev.cent_str_en) || (global->dev.cent_str_en && dev && (dev->device_role == DEVICE_ROLE_CONTROLLER
	|| dev->device_role == DEVICE_ROLE_CONTRAGENT))) {
		if(bcn_rpt->rpt_len > 0){
			eid_ptr = (PEID_STRUCT)bcn_rpt->rpt;
			parse_len = sizeof(struct beacon_metrics_rsp);
			while ( (parse_len + eid_ptr->Len + 2) <= tlv_msg->length ) {
				switch (eid_ptr->Eid) {
					case IE_MEASUREMENT_REPORT:
						/*Skip measurement report token and type*/
						if(eid_ptr->Len - 3 > 0){
							pBcnRep = (prrm_beacon_rep_info)((u8*)eid_ptr->Octet + 3);
#ifdef VENDOR1_FEATURE_EXTEND
							/*
							* Because of the ap_roam_algo_find_best_bss_map implementation method,
							* the onwer (currently connected AP) must be processed first.
							* (The candidate_target_rssi must be calculated first with the owner information)
							* Due to the list processing method,
							*  you must add to the list of ap_est_handle_11k_report last to process it first.
							* so skip owner bcn report here, and handle in last
							*/
							if( memcmp( cli->bssid, pBcnRep->bssid, ETH_ALEN) == 0 ){
								mapd_printf(MSG_ERROR, "BSSID " MACSTR " Owner Beacon Report found, handle in last",
												MAC2STR(pBcnRep->bssid) );
								pOwnerBcnRep = pBcnRep;
								bFindOwner = 1;
							} else
#endif //VENDOR1_FEATURE_EXTEND
							bcnrpt_cnt++;
							err("bcnrptcnt:%d,Chan No:%d,rcpi:%d,bssid:"MACSTR,bcnrpt_cnt,pBcnRep->ch_number,pBcnRep->rcpi,MAC2STR(pBcnRep->bssid));

							if(bcnrpt_cnt == bcn_rpt->bcn_rpt_num)
								islastreport = 1;

							ap_est_handle_11k_report(global, bcn_rpt->sta_mac, isSuccess,
								pBcnRep->bssid, pBcnRep->ch_number,
								pBcnRep->rcpi, islastreport);
						} else {
							isSuccess = 0;
							ap_est_handle_11k_report(global, bcn_rpt->sta_mac, isSuccess,
										NULL, 0, 0, 0);
						}
						break;
					default:
						break;
				}
				mapd_printf(MSG_ERROR, "parse_len:%u, eid_ptr->Len:%d,  tlv_msg->length:%u, bcn_rpt->rpt_len:%u", parse_len, eid_ptr->Len, tlv_msg->length, bcn_rpt->rpt_len);
				parse_len += 2 + eid_ptr->Len;
				if(parse_len >= tlv_msg->length) {
					mapd_printf(MSG_ERROR, " parse beacon resp done");
					break;
				}
				eid_ptr = (PEID_STRUCT)((u8 *)eid_ptr + 2 + eid_ptr->Len);
			}
#ifdef VENDOR1_FEATURE_EXTEND
			/*Handle OwnerBcnReport here in the last*/
			if(bFindOwner) {
				mapd_printf(MSG_ERROR, "Owner Beacon Report handle");
				islastreport = 1;
				ap_est_handle_11k_report(global, bcn_rpt->sta_mac, isSuccess,
								pOwnerBcnRep->bssid, pOwnerBcnRep->ch_number,
								pOwnerBcnRep->rcpi, islastreport);
			}
#endif //VENDOR1_FEATURE_EXTEND
		}
	}
	os_free(timeout_ctx);
}

int fill_assoc_client_caps(struct mapd_global *global, struct map_client_association_event_local *pevt)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pevt->assoc_req;
	PEID_STRUCT eid_ptr = NULL;
	struct client *cli = NULL;
	unsigned short minus_len = 0;
	char is_found = 0;
	char is_80211 = 0, is_reassoc = 0;
	uint8_t band_idx = 0;

	if (!global || !pevt) {
		err("invalid params!! global(%p) or pevt(%p) is null", global, pevt);
		return -1;
	}

	if (!pevt->assoc_evt) {
		err("invalid params!! assoc_evt(0) means it'a an disconnect event");
		return -1;
	}

	dl_list_for_each(cli, &global->dev.sta_seen_list, struct client, sta_seen_entry)
	{
		if (!os_memcmp(&cli->mac_addr[0], pevt->sta_mac, ETH_ALEN)) {
			is_found = 1;
			break;
		}
	}
	if (!is_found) {
		err("invalid params!! can't find client by mac("MACSTR")", MAC2STR(pevt->sta_mac));
		return -1;
	}

	if(os_memcmp(Fr->Hdr.Addr2, cli->mac_addr, ETH_ALEN) == 0){
		is_80211 = 1;
	}
	if(is_80211){
		if (Fr->Hdr.FC.SubType != SUBTYPE_ASSOC_REQ)
			is_reassoc = 1;

		if (is_reassoc) {
			eid_ptr = (PEID_STRUCT) &Fr->Octet[10];
			minus_len = sizeof(HEADER_802_11) + 10;
		} else {
			eid_ptr = (PEID_STRUCT) &Fr->Octet[4];
			minus_len = sizeof(HEADER_802_11) + 4;
		}
	} else {
		err("assoc req is invalid");
		return -1;
	}
	os_get_reltime(&cli->assoc_time);
	if (cli->current_chan > 14) {
		band_idx = BAND_5G_IDX;
	} else {
		band_idx = BAND_2G_IDX;
	}
#ifdef MAP_R2
	client_db_update_cli_ht_vht_cap(global, cli->client_id,
		(unsigned char *)eid_ptr, pevt->assoc_req_len - minus_len, band_idx);
	if (cli->assoc_req_ie) {
		os_free(cli->assoc_req_ie);
		cli->assoc_req_ie = NULL;
		cli->assoc_req_ie_len = 0;
	}
	cli->assoc_req_ie_len = pevt->assoc_req_len - sizeof(HEADER_802_11);
	cli->assoc_req_ie = os_zalloc(cli->assoc_req_ie_len);
	if (!cli->assoc_req_ie) {
		cli->assoc_req_ie_len = 0;
	} else {
		os_memcpy(cli->assoc_req_ie, (char *)(pevt->assoc_req + sizeof(HEADER_802_11)), cli->assoc_req_ie_len);
	}
#endif

	return 0;
}

#ifdef MAP_R2
void wlanif_sta_dissassoc_stats_update(struct mapd_global *global,
	u8 *mac_addr, u16 reason)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	struct traffic_stats_db *traffic_stats = NULL, *ttraffic_stats = NULL;
	struct stats_db *stats = NULL, *tstats = NULL;
	struct stat_info *stat_buf = NULL;
	struct _1905_map_device *dev = topo_srv_get_controller_device(ctx);
	struct client *cli = NULL;

	if (!dev) {
		mapd_printf(MSG_ERROR, "can't found controller.\n");
		return;
	}
	dl_list_for_each(cli, &ctx->sta_seen_list, struct client, sta_seen_entry)
	{
		if (!os_memcmp(&cli->mac_addr[0], mac_addr, ETH_ALEN)) {
			cli->disassoc_reason = reason;
			break;
		}
	}
	SLIST_FOREACH_SAFE(traffic_stats, &ctx->metric_entry.traffic_stats_head, traffic_stats_entry, ttraffic_stats) {
		SLIST_FOREACH_SAFE(stats, &traffic_stats->stats_head, stats_entry, tstats) {
			if (os_memcmp(stats->mac, mac_addr, ETH_ALEN) == 0) {
				stat_buf = (struct stat_info *)os_zalloc(sizeof(struct stat_info));
				if (!stat_buf) {
					err("Memory Allocation Failed!!\n");
					return;
				}
				os_memcpy(stat_buf, stats, sizeof(struct stat_info));
				map_1905_Send_disassoc_sta_stats_message(_1905_ctrl, dev->_1905_info.al_mac_addr, reason, stat_buf);
				os_free(stat_buf);
				break;
			}
		}
	}
}
#endif
void wlanif_handle_client_notification(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *) eloop_ctx;
	struct msg *tlv_msg = (struct msg *) timeout_ctx;
	struct map_priv_cli_cap cli_caps;
	struct client_association_event_local *evt =
			(struct client_association_event_local *) tlv_msg->buffer;
	struct map_client_association_event_local *pevt =
			(struct map_client_association_event_local *)&evt->map_assoc_evt;
	struct bss_info_db *bss_info = NULL;

	mapd_hexdump(MSG_MSGDUMP, "WAPP_CLIENT_NOTIFICATION", (char *)tlv_msg,
		    tlv_msg->length + TLV_BUFFER_OFFSET);
	bss_info = topo_srv_get_bss_by_bssid(&global->dev, NULL, pevt->bssid);
	if (!bss_info)
	{
		mapd_printf(MSG_INFO, MACSTR " assoc_evt = %d error BSSID: " MACSTR,
					MAC2STR(pevt->sta_mac),
					pevt->assoc_evt, MAC2STR(pevt->bssid));
		os_free(timeout_ctx);
		return;
	}
#ifdef MAP_R2
	if (!pevt->assoc_evt)
		wlanif_sta_dissassoc_stats_update(global, pevt->sta_mac, pevt->reason);
#endif
#ifdef SUPPORT_MULTI_AP
	topo_srv_parse_wapp_client_notification(global, evt);
#endif
	cli_caps = evt->cli_caps;
	mapd_printf(MSG_INFO, MACSTR " assoc_evt = %d BSSID : " MACSTR ,
					MAC2STR(pevt->sta_mac),
					pevt->assoc_evt, MAC2STR(pevt->bssid));

#ifdef SUPPORT_MULTI_AP
	if (bss_info) {
		if (pevt->assoc_evt) {
			if (pevt->is_APCLI)
				always("inform local wifi APCLI join\n");
			else
				info("inform local wifi STA join\n");
			mapd_user_wireless_client_join(
				global,
				pevt->sta_mac, pevt->bssid,
				(char *)bss_info->ssid);
		} else {
			if (pevt->is_APCLI)
				always("inform local wifi APCLI left\n");
			else
				info("inform local wifi STA left\n");
			mapd_user_wireless_client_leave(
				global,
				pevt->sta_mac, pevt->bssid,
				(char *)bss_info->ssid);
		}
	}
#endif
	/* Ignore BACKHAUL STAs */
	if (pevt->is_APCLI) {
		os_free(timeout_ctx);
		return ;
	}

	if (pevt->assoc_evt) {
		client_mon_handle_local_join(global, pevt->sta_mac, pevt->bssid,
						1, cli_caps.btm_capable, cli_caps.rrm_capable, cli_caps.mbo_capable,
						cli_caps.nss, cli_caps.phy_mode, cli_caps.bw, &cli_caps.nss_he);
		fill_assoc_client_caps(global, pevt);
#ifdef DATA_ELEMENT_SUPPORT
		if (global->dev.device_role == DEVICE_ROLE_CONTROLLER) {
			create_sta_association_json_file(&global->dev, pevt->sta_mac);
		}
#endif
	} else {
		client_mon_handle_local_leave(global, pevt->sta_mac, pevt->bssid);
	}
	os_free(timeout_ctx);
}


void wlanif_handle_oper_bss_report(void *eloop_ctx, void *timeout_ctx)
{
	uint8_t idx = 0;
	uint8_t bss_idx = 0;
	uint8_t found_flag = 0;
	struct mapd_bss *bss;
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	struct mapd_radio_info *radio_info;
#ifdef SUPPORT_MULTI_AP
	struct radio_info_db *radio = NULL;
#endif
	struct mapd_bss *temp;

	mapd_hexdump(MSG_DEBUG, "receive operational bss report event",
					(char *)tlv_msg, tlv_msg->length + TLV_BUFFER_OFFSET);
	struct oper_bss_cap *bss_cap = (struct oper_bss_cap *) tlv_msg->buffer;
	struct op_bss_cap *bss_report = &bss_cap->cap[0];
#ifdef SUPPORT_MULTI_AP
	topo_srv_parse_wapp_oper_bss_report(global, bss_cap);
#endif
	radio_info = get_radio_info_by_radio_id(global, bss_cap->identifier);
	if (!radio_info) {
		err("-----------Radio Info NULL------???");
		os_free(timeout_ctx);
		return;
	}
	if (bss_cap->oper_bss_num == 0) {
		err("BSS num is 0 from WAPP");
	}
#ifdef SUPPORT_MULTI_AP
	radio = topo_srv_get_radio(
		topo_srv_get_1905_device(&global->dev, NULL),
		radio_info->identifier);
	if (!radio) {
		err("invalid oper bss report");
		os_free(timeout_ctx);
		return;
	}
	if (global->dev.device_role == DEVICE_ROLE_CONTROLLER
		&& global->dev.need_to_update_wts)
		topo_srv_update_wts_config(&global->dev);

	topo_srv_update_1905_radio_capinfo(&global->dev, radio);
#endif
	for(idx = 0; idx < bss_cap->oper_bss_num; ++idx, bss_report++) {
		found_flag = 0;
		if (dl_list_empty(&radio_info->bss_list)) {
			bss_idx  = get_free_bss_idx_in_bitmap(radio_info);
			bss_init(radio_info, bss_report->bssid, bss_report->ssid,bss_report->ssid_len, bss_idx);
			continue;
		}
		dl_list_for_each(bss, &radio_info->bss_list, struct mapd_bss, bss_entry)
		{
			if(!os_memcmp(bss->bssid, bss_report->bssid, ETH_ALEN)) {
				os_memcpy(bss->ssid, bss_report->ssid, 33);
				bss->ssid[bss_report->ssid_len] = '\0';
				found_flag = 1; //XXX: Added by NM, Sync with Anirudh
			}
		}
		if(!found_flag) {
			bss_idx  = get_free_bss_idx_in_bitmap(radio_info);
			bss_init(radio_info, bss_report->bssid, bss_report->ssid,bss_report->ssid_len, bss_idx);
		}
	}

	dl_list_for_each_safe(bss, temp, &radio_info->bss_list, struct mapd_bss, bss_entry) {
		bss_report = &bss_cap->cap[0];
		if (!bss)
			err("BSS is NULL");
		for(idx = 0; idx < bss_cap->oper_bss_num; ++idx, bss_report++) {
			if(!os_memcmp(bss->bssid, bss_report->bssid, ETH_ALEN)) {
				break;
			}
		}
		if(idx == bss_cap->oper_bss_num) {
			reset_bss_idx_in_bitmap(radio_info, bss->bss_idx);
			bss_deinit(radio_info, bss);
		}

	}
#ifdef SUPPORT_MULTI_AP
	if (global->dev.config_status == DEVICE_CONFIG_ONGOING) {
		if (radio->is_configured == FALSE)
			radio->is_configured = TRUE;
		topo_srv_update_own_device_config_status(&global->dev);
	}
	if (global->dev.config_status != DEVICE_UNCONFIGURED)
		topo_srv_update_radio_config_status(&global->dev, radio->identifier, TRUE);

#endif
	os_free(timeout_ctx);
}

/* Move parsing to chanMon/mapd */
void wlanif_handle_op_chan_info(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	unsigned char i = 0, actual_opclass = 0;
	uint8_t j = 0;
	int first_free_radio_idx = -1, actual_idx = 0;
	struct channel_report_wapp *chan_rpt_wapp = (struct channel_report_wapp *)(tlv_msg->buffer);

	struct channel_report *chan_rpt = (struct channel_report *) &chan_rpt_wapp->ch_rep_num;


#ifdef MAP_R4_SPT
	struct spt_reuse_report *spt_report = (struct spt_reuse_report *) &chan_rpt_wapp->spt_rep_num;


	//mapd_hexdump(MSG_OFF, "Receive OP_CHAN_INFO", (char *)(tlv_msg),
		//			tlv_msg->length + TLV_BUFFER_OFFSET);

	//mapd_printf(MSG_INFO, "%s: ENTERED Num of ch in rep=%d No of spt reuse in resp %d\n", __func__,
		//			chan_rpt->ch_rep_num, chan_rpt->spt_rep_num);
#endif
	topo_srv_parse_wapp_skip_list_channel(global, chan_rpt_wapp);
#ifdef SUPPORT_MULTI_AP
	topo_srv_parse_wapp_operating_channel_info(global, chan_rpt
#ifdef MAP_R4_SPT
	, spt_report
#endif
	);
#endif
	/* Update Status of Existing Radios */
	for (j = 0; j < MAX_NUM_OF_RADIO; j++) {
		struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[j];
		if (radio_info->radio_idx == (uint8_t)-1) {
			continue;
		}

		for (i = 0; i < chan_rpt->ch_rep_num; i++) {
			struct ch_rep_info *rep_info = &chan_rpt->info[i];
			if (!os_memcmp(rep_info->identifier, radio_info->identifier, ETH_ALEN)) {
				mapd_printf(MSG_ERROR, "%s: Found already listed Radio with RUID="
								MACSTR, __func__,
								MAC2STR(radio_info->identifier));
					if (rep_info->op_class == 128 || rep_info->op_class == 129) {
						continue;
					}
#ifdef MAP_6E_SUPPORT
					if ((rep_info->op_class == 133) || (rep_info->op_class == 134))
						continue;
#endif
#ifdef MAP_320BW
					if (rep_info->op_class == 137)
						continue;
#endif
					if (rep_info->channel != radio_info->channel) {
						info("-----> Channel changed on Radio with RUID="
									MACSTR,
									MAC2STR(radio_info->identifier));
						mapd_handle_radio_channel_change(radio_info, rep_info->channel);
					}
				break;
			}
		}
		if (i == chan_rpt->ch_rep_num) {
			mapd_printf(MSG_INFO, "Radio with RUID=" MACSTR " is down",
							MAC2STR(radio_info->identifier));
			mapd_radio_deinit(global, radio_info);
		}
	}
	/* Add new Radios */
	for (i = 0; i < chan_rpt->ch_rep_num; i++) {
		struct ch_rep_info *rep_info = &chan_rpt->info[i];
		if (rep_info->op_class == 128 || rep_info->op_class == 129
#ifdef MAP_6E_SUPPORT
			|| (rep_info->op_class == 133) || (rep_info->op_class == 134)
#endif
#ifdef MAP_320BW
			|| (rep_info->op_class == 137)
#endif
			) {
			actual_opclass = rep_info->op_class;
			continue;
		}
		for (j = 0; j < MAX_NUM_OF_RADIO; j++) {
			struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[j];
			if (radio_info->radio_idx == (uint8_t)-1) {
				if (first_free_radio_idx == -1)
					first_free_radio_idx = i;
				continue;
			}
			if (!os_memcmp(rep_info->identifier, radio_info->identifier,
							ETH_ALEN)) {
				break;
			}
		}
		if (j == MAX_NUM_OF_RADIO) {
			struct mapd_radio_info *radio_info = NULL;
			mapd_printf(MSG_INFO, "Found New radio in chan report-->");
			if (first_free_radio_idx != -1) {
				mapd_printf(MSG_INFO, "Insert at %d", actual_idx);
				radio_info = &global->dev.dev_radio_info[actual_idx];
				if (actual_opclass)
					rep_info->op_class = actual_opclass;
				mapd_radio_init((uint8_t)actual_idx, radio_info,
								rep_info->channel,
								rep_info->op_class, rep_info->tx_power,
								&rep_info->identifier[0]);
				first_free_radio_idx = -1;
				actual_idx++;
				actual_opclass = 0;
			} else {
				mapd_printf(MSG_ERROR, "No more space-- Cant insert this radio");
			}
		}
	}
	os_free(timeout_ctx);
}

void wlanif_handle_op_bss_load(void *eloop_ctx, void *timeout_ctx)
{
	mapd_printf(MSG_ERROR, "To be handled");
	os_free(timeout_ctx);
}

/* OK */
void wlanif_handle_ap_metrics_info(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct event_wrapper_s *event_wrapper = timeout_ctx;
	struct msg *tlv_msg = (struct msg *) event_wrapper->event;
#ifdef SUPPORT_MULTI_AP
	int from = event_wrapper->from;
#endif
	struct ap_metrics_info *ap_info = (struct ap_metrics_info *)(tlv_msg->buffer);

	mapd_hexdump(MSG_MSGDUMP, "AP_METRICS_INFO", (char *)(tlv_msg),
					tlv_msg->length + TLV_BUFFER_OFFSET);
#ifdef SUPPORT_MULTI_AP
	topo_srv_parse_wapp_ap_metric_event(global, ap_info,
		from);
#endif
	mapd_handle_ap_metrics_info(global, ap_info->bssid, ap_info->ch_util,
					ap_info->assoc_sta_cnt);
	os_free(tlv_msg);
	os_free(timeout_ctx);
}
#ifdef SUPPORT_MULTI_AP
void wlanif_handle_air_monitor_report(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

#ifdef CENT_STR
	if(global->dev.cent_str_en && global->dev.device_role == DEVICE_ROLE_CONTROLLER)
		ap_est_handle_own_unassoc_sta_link_metric_rsp(global,(struct unassoc_link_metric_rsp *)tlv_msg->buffer,tlv_msg->length);
	else
#endif
	topo_srv_parse_wapp_air_monitor_report(global,
		(struct unlink_metrics_rsp *)tlv_msg->buffer);

	os_free(timeout_ctx);
}

void wlanif_handle_1905_read_bss_conf_req(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_read_bss_conf_request(global,
		(char *)tlv_msg->buffer, tlv_msg->length);
	os_free(timeout_ctx);
}

void wlanif_handle_cli_steering_complete(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_cli_steering_completed(global,
		(struct cli_steer_btm_event *)tlv_msg->buffer);
	os_free(timeout_ctx);
}

void wlanif_handle_operating_channel_report(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_operating_channel_report(global,
		tlv_msg->buffer);
	os_free(timeout_ctx);
}

void wlanif_handle_bh_ready(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_bh_ready(global,
		(struct bh_link_info *)tlv_msg->buffer);
	os_free(timeout_ctx);
}

void wlanif_handle_1905_cmdu_req(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_1905_cmdu_request(global,
		(struct _1905_cmdu_request *)tlv_msg->buffer);
	os_free(timeout_ctx);
}

void wlanif_handle_radio_op_restrict(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_radio_operation_restriction(global,
		(struct restriction *)tlv_msg->buffer);
	os_free(timeout_ctx);
}

void wlanif_handle_channel_pref(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_channel_preferrence(global,
		(struct ch_prefer *)tlv_msg->buffer);
	os_free(timeout_ctx);
}

void wlanif_handle_bh_steer_resp(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_bh_steer_resp(global,
		(struct backhaul_steer_rsp *)tlv_msg->buffer);
	os_free(timeout_ctx);
}

void wlanif_handle_ap_capability(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_ap_capability(global,
		(struct ap_capability *)tlv_msg->buffer);
	os_free(timeout_ctx);
}

void wlanif_handle_map_scan_result(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_scan_result(global,
		(struct wapp_scan_info *)tlv_msg->buffer);
	os_free(timeout_ctx);
}

void wlanif_handle_map_scan_done(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;

	topo_srv_parse_wapp_scan_done(global);
	os_free(timeout_ctx);
}

void wlanif_handle_map_ie_changed(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_vend_ie_changed(global,
		(struct map_vendor_ie *)tlv_msg->buffer);
	os_free(timeout_ctx);
}

void wlanif_handle_get_wsc_config(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_get_wsc_config(global,
		(struct wps_get_config *)tlv_msg->buffer);
	os_free(timeout_ctx);
}

void wlanif_handle_ap_link_metric_info(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_ap_link_metirc_request(global,
		tlv_msg->buffer);
	os_free(timeout_ctx);
}

void wlanif_handle_assoc_state_change(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	struct wapp_apcli_association_info *cli_assoc_info=
		(struct wapp_apcli_association_info *)(tlv_msg->buffer);

	topo_srv_parse_wapp_assoc_state_changed(global, cli_assoc_info);
	os_free(timeout_ctx);
}

void wlanif_handle_1905_tlv_read(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_1905_read_tlv_req(global, (char *)tlv_msg->buffer,
		tlv_msg->length);
	os_free(timeout_ctx);
}
void wlanif_handle_mapd_renew(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	unsigned short event =  tlv_msg->type;

	err("\nWPS configuration done  --> issue mapd renew = %d\n", event);
	mapd_renew(global);

	os_free(timeout_ctx);
}

void wlanif_handle_net_opt_scan_result(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	struct net_opt_scan_report_event *scan_rep_evt =
		(struct net_opt_scan_report_event *)(tlv_msg->buffer);
	info("in map wlanif_handle_off_channel_scan_result, just received from wapp\n ");
	//dump_net_opt_off_ch_scan_rep(scan_rep_evt);
	topo_srv_parse_wapp_net_opt_scan_report(global, scan_rep_evt);
	os_free(timeout_ctx);
}
void wlanif_handle_off_channel_scan_result(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	struct off_ch_scan_report_event *scan_rep_evt =
		(struct off_ch_scan_report_event *)(tlv_msg->buffer);

	topo_srv_parse_wapp_off_channel_scan_report(global, scan_rep_evt);
	os_free(timeout_ctx);
}

void wlanif_handle_device_status(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;
	struct _wapp_device_status *device_status =
		(struct _wapp_device_status *)(tlv_msg->buffer);

	topo_srv_parse_wapp_device_status(global,
		(wapp_device_status *)device_status);
	os_free(timeout_ctx);
}


void wlanif_handle_map_reset(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;

	/*Disconnect clients on reset*/
	client_disconn_trigger(global);
#ifdef CENT_STR
	if(global->dev.cent_str_en && global->dev.device_role == DEVICE_ROLE_CONTROLLER)
		cent_str_cu_mon_remove_chan_list(&global->dev);
#endif

	if(timeout_ctx)
		os_free(timeout_ctx);
}

#ifdef MAP_R2
#ifdef MAP_R4
static int count;
#endif /* MAP_R4 */
void wlanif_handle_tunneled_frame(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

	topo_srv_parse_wapp_tunneled_msg(global,(struct tunneled_msg *)tlv_msg->buffer);

#ifdef MAP_R4
	if (global->params.Certification) {
		count++;

		if (count != 1) {
			count = 0;
			if (tlv_msg != NULL)
				os_free(tlv_msg);
		} else
			eloop_register_timeout(1, 0, wlanif_handle_tunneled_frame, global, tlv_msg);
	} else {
#endif /* MAP_R4 */
		if (tlv_msg != NULL)
			os_free(tlv_msg);
#ifdef MAP_R4
	}
#endif /* MAP_R4 */
}
#endif
void wlanif_handle_agent_wps_success(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *tlv_msg = (struct msg *)timeout_ctx;

#ifdef ACL_CTRL
	mapd_acl_sync_new_agent_info(global, (struct wapp_bhsta_info *)tlv_msg->buffer);
#endif
	if (tlv_msg != NULL)
		os_free(tlv_msg);
}

#endif
static int wapp_usr_intf_parse_event(struct mapd_global *global, char *s_buf, size_t len, unsigned char type, int from)
{
	struct msg *wapp_event = NULL;
	struct own_1905_device *ctx = &global->dev;
#ifdef SUPPORT_MULTI_AP
#ifdef AUTOROLE_NEGO
	struct dev_role_negotiate *dev_role_event = NULL;
	struct _1905_map_device  *own_dev;
	int j = 0;
#endif //AUTOROLE_NEGO
#endif
	struct event_wrapper_s *event_wrapper = NULL;

#ifdef SUPPORT_AP_ENROLLE
	int i = 0;
#endif
	u16 buffer_size = 0;
	unsigned char wapp_type = 0;

	if(len < sizeof(struct msg)) {
		mapd_printf(MSG_ERROR, "input len is less");
		return -1;
	}

	char *buf = (char *)os_zalloc(len);
	if(!buf) {
		err("all buf fail");
		return -1;
	}
	os_memcpy(buf, &s_buf[0], len);
	wapp_event = (struct msg *)buf;
	/* hex_dump("wapp msg",buf,len); */
	if (wapp_event && wapp_event->length == 0) {
#ifdef SUPPORT_MULTI_AP
		if (wapp_event->type != WAPP_1905_CMDU_REQUEST &&
			wapp_event->type != WAPP_STEERING_COMPLETED &&
			wapp_event->type != WAPP_MAP_BH_READY &&
			wapp_event->type != WAPP_MAP_RESET &&
			wapp_event->type != WAPP_MAP_RENEW
#ifdef MAP_R3
			&& wapp_event->type != WAPP_SEND_BSS_CONNECTOR
#endif /* MAP_R3 */
		) {
#endif
			err("invalid data from wapp! type=%02x", wapp_event->type);
			os_free(buf);
			return -1;
#ifdef SUPPORT_MULTI_AP
		}
#endif
	}

	if (wapp_event && ((wapp_event->length + sizeof(struct msg)) > len )) {

		err("invalid data len from wapp! wapp_event->length:%d bigger than input len", wapp_event->length);
		os_free(buf);
		return -1;
	}

	wapp_type = wapp_event->type;
	/* topo_srv_parse_wapp_event(global, buf, len, from); */
	if ((type != 0) && (type == wapp_event->type)) {
		mapd_printf(MSG_DEBUG, "Solicited WAPP Response to command from MAPD");
			switch (wapp_event->type) {
			case WAPP_ALL_ASSOC_STA_LINK_METRICS:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_ALL_ASSOC_STA_LINK_METRICS"
					"from WAPP: CONSUME");
				event_wrapper = os_zalloc(
					sizeof(struct event_wrapper_s));
				event_wrapper->from = from;
				event_wrapper->event = buf;
				wlanif_handle_all_assoc_sta_link_metrics((void *)global, (void *)event_wrapper);
				break;
			case WAPP_ONE_ASSOC_STA_LINK_METRICS:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_ONE_ASSOC_STA_LINK_METRICS"
								"from WAPP: CONSUME");
				event_wrapper = os_zalloc(
					sizeof(struct event_wrapper_s));
				event_wrapper->from = from;
				event_wrapper->event = buf;
				wlanif_handle_one_assoc_sta_link_metrics((void *)global, (void *)event_wrapper);
				break;
#ifdef SUPPORT_MULTI_AP
			case WAPP_RX_LINK_STATISTICS:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_RX_LINK_STATISTICS"
								"from WAPP: CONSUME");
				wlanif_handle_rx_link_stats((void *)global, (void *)buf);
				break;
			case WAPP_TX_LINK_STATISTICS:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_RX_LINK_STATISTICS"
								"from WAPP: CONSUME");
				wlanif_handle_tx_link_stats((void *)global, (void *)buf);
				break;
			case WAPP_UNASSOC_STA_LINK_METRICS:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_UNASSOC_STA_LINK_METRICS"
								"from WAPP: CONSUME");
				wlanif_handle_unassoc_sta_link_metrics((void *)global, (void *)buf);
				break;
#endif
			case WAPP_OPERATING_CHANNEL_INFO:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_OPERATING_CHANNEL_INFO"
								"from WAPP: CONSUME");
				wlanif_handle_op_chan_info((void *)global, (void *)buf);
				break;
			case WAPP_STA_BSSLOAD:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_STA_BSSLOAD"
								"from WAPP: CONSUME");
				wlanif_handle_op_bss_load((void *)global, (void *)buf);
				break;
			case WAPP_AP_HT_CAPABILITY:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_AP_HT_CAPABILITY"
								"from WAPP: CONSUME");
				wlanif_handle_ap_ht_cap((void *)global, (void *)buf);
				break;
			case WAPP_AP_VHT_CAPABILITY:
				debug("Solicited WAPP_AP_VHT_CAPABILITY from WAPP: CONSUME");
				wlanif_handle_ap_vht_cap((void *)global, (void *)buf);
				break;
			case WAPP_OPERBSS_REPORT:
				debug("Solicited Operational bss report from WAPP: CONSUME");
				wlanif_handle_oper_bss_report((void *)global, (void *)buf);
				break;
#ifdef SUPPORT_MULTI_AP
			case WAPP_CLI_CAPABILITY_REPORT:
				debug("Solicited WAPP_CLI_CAPABILITY_REPORT from WAPP CONSUME");
				wlanif_handle_cli_cap_report((void *)global, (void *)buf);
				break;
#endif
			case WAPP_CLI_STEER_BTM_REPORT:
				//Check if we we will wait for BTM report
				debug("Solicited WAPP_CLI_STEER_BTM_REPORT from WAPP CONSUME");
				wlanif_handle_steer_btm_report((void *)global, (void *)buf);
				break;
			case WAPP_AP_METRICS_INFO:
				debug("Solicited WAPP_AP_METRICS_INFO from WAPP:"
								"CONSUME\n");
				event_wrapper = os_zalloc(
					sizeof(struct event_wrapper_s));
				event_wrapper->from = from;
				event_wrapper->event = buf;
				wlanif_handle_ap_metrics_info((void *)global, (void *)event_wrapper);
				break;
#ifdef SUPPORT_MULTI_AP
			case WAPP_NAC_INFO: /* not in API doc??? */
				debug("Solicited WAPP_NAC_INFO from WAPP: CONSUME");
				wlanif_handle_nac_info((void *)global, (void *)buf);
				break;
#endif
			case WAPP_ALL_ASSOC_STA_TRAFFIC_STATS:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_ALL_ASSOC_STA_TRAFFIC_STATS CONSUME");
				event_wrapper = os_zalloc(
					sizeof(struct event_wrapper_s));
				event_wrapper->from = from;
				event_wrapper->event = buf;
				wlanif_handle_traffic_stats((void *)global, (void *)event_wrapper);
				break;
			case WAPP_RADIO_BASIC_CAP:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_RADIO_BASIC_CAP report");
				wlanif_handle_radio_basic_cap((void *)global, (void *)buf);
				break;
#ifdef SUPPORT_MULTI_AP
			case WAPP_WIRELESS_INF_INFO:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_WIRELESS_INF_INFO report");
				wlanif_handle_wireless_inf_info((void *)global, (void *)buf);
				break;
#endif
			case WAPP_ALL_ASSOC_TP_METRICS:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_ALL_ASSOC_STA_TP_METRICS"
						"from WAPP: CONSUME");
				wlanif_handle_all_assoc_sta_tp_metrics((void *)global, (void *)buf);
				break;
#ifdef SUPPORT_MULTI_AP
			case WAPP_MAP_BH_CONFIG:
				debug("Solicited WAPP_MAP_BH_CONFIG");
				wlanif_handle_bh_config_event((void *)global, (void *)buf);
				break;
			case WAPP_BRIDGE_IP:
				debug("Solicited WAPP_BRIDGE_IP");
				wlanif_handle_br_ip_event((void *)global, (void *)buf);
				break;
			case WAPP_SET_BH_TYPE:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_SET_BH_TYPE");
				wlanif_handle_set_bh_type((void *)global, (void *)buf);
				break;
			case WAPP_AIR_MONITOR_REPORT:
				wlanif_handle_air_monitor_report(global, (void *)buf);
				break;
			case WAPP_STEERING_COMPLETED:/* XXX: Usage? */
				wlanif_handle_cli_steering_complete(global, (void *)buf);
				break;
			case WAPP_1905_READ_BSS_CONF_REQUEST:
				wlanif_handle_1905_read_bss_conf_req(global, (void *)buf);
				break;
			case WAPP_OPERATING_CHANNEL_REPORT:
				wlanif_handle_operating_channel_report(global, (void *)buf);
				break;
			case WAPP_BEACON_METRICS_REPORT:
				wlanif_handle_bcn_metrics_report(global, (void *)buf);
				break;
			case WAPP_MAP_BH_READY:
				wlanif_handle_bh_ready(global, (void *)buf);
				break;
			case WAPP_1905_CMDU_REQUEST:
				wlanif_handle_1905_cmdu_req(global, (void *)buf);
				break;
			case WAPP_RADIO_OPERATION_RESTRICTION:
				wlanif_handle_radio_op_restrict(global, (void *)buf);
				break;
			case WAPP_CHANNLE_PREFERENCE:
				wlanif_handle_channel_pref(global, (void *)buf);
				break;
			case WAPP_BACKHAUL_STEER_RSP:
				wlanif_handle_bh_steer_resp(global, (void *)buf);
				break;
			case WAPP_AP_CAPABILITY:
				wlanif_handle_ap_capability(global, (void *)buf);
				break;
			case WAPP_SCAN_RESULT:
				wlanif_handle_map_scan_result(global, (void *)buf);
				break;
			case WAPP_SCAN_DONE:
				wlanif_handle_map_scan_done(global, (void *)buf);
				break;
			case WAPP_MAP_VEND_IE_CHANGED:
				wlanif_handle_map_ie_changed(global, (void *)buf);
				break;
			case WAPP_GET_WSC_CONF:
				wlanif_handle_get_wsc_config(global, (void *)buf);
				break;
			case WAPP_AP_LINK_METRIC_REQ:
				wlanif_handle_ap_link_metric_info(global, (void *)buf);
				break;
			case WAPP_APCLI_ASSOC_STAT_CHANGE:
			/* break not needed */
			case WAPP_APCLI_UPLINK_RSSI:
				wlanif_handle_assoc_state_change(global, (void *)buf);
				break;
			case WAPP_1905_READ_1905_TLV_REQUEST:
				wlanif_handle_1905_tlv_read(global, (void *)buf);
				break;
			case WAPP_DEVICE_STATUS:
				wlanif_handle_device_status(global, (void *)buf);
				break;
			case WAPP_OFF_CH_SCAN_REPORT:
				err("in map receive from wapp : WAPP_OFF_CH_SCAN_REPORT\n");
				wlanif_handle_off_channel_scan_result(global, (void *)buf);
				break;
			case WAPP_NET_OPT_SCAN_REPORT:
				err("in map receive from wapp : WAPP_OFF_CH_SCAN_REPORT\n");
				wlanif_handle_net_opt_scan_result(global, (void *)buf);
				break;
			case WAPP_MAP_RENEW:
				err("in map receive from wapp : WAPP_MAP_RENEW\n");
				wlanif_handle_mapd_renew(global, (void *)buf);
				break;
#endif
			case WAPP_AP_HE_CAPABILITY:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_AP_HE_CAPABILITY"
								"from WAPP: CONSUME");
				wlanif_handle_ap_he_cap((void *)global, (void *)buf);
				break;
#ifdef MAP_R3_DE
			case WAPP_DEV_INVEN_TLV:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_DEV_INVEN_TLV"
								"from WAPP: CONSUME");
				wlanif_handle_dev_inven_tlv((void *)global, (void *)buf);
				break;

#endif /*MAP_R3_DE*/
#ifdef MAP_R2
			case WAPP_SCAN_CAPAB:
				topo_srv_parse_wapp_scan_capab(global, (struct channel_scan_capab *)wapp_event->buffer, wapp_event->length);
				if (buf != NULL) {
					os_free(buf);
				}

				break;
#ifdef DFS_CAC_R2
			case WAPP_CAC_CAPAB:
				topo_srv_parse_wapp_cac_capab(global, (struct cac_capability *)wapp_event->buffer, wapp_event->length);
				if (buf != NULL) {
					os_free(buf);
				}
				break;
#endif
#endif
			case WAPP_CAC_COMPLETION_REPORT:
				eloop_register_timeout(0, 0, topo_srv_parse_wapp_cac_completion_report_wrapper, global, wapp_event);
				/* topo_srv_parse_wapp_cac_completion_report(global, wapp_event, wapp_event->length); */
				break;
#ifdef MAP_R2
#ifdef DFS_CAC_R2
			case WAPP_CAC_STATUS_REPORT:
				topo_srv_parse_wapp_cac_status_report(global, wapp_event, wapp_event->length);
				break;
#endif
			case WAPP_METRIC_REP_INTERVAL_CAP:
				topo_srv_parse_wapp_metric_rep_interval(global, (u32 *)wapp_event->buffer);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
				break;
			case WAPP_DISASSOC_STATS_EVT:
				topo_srv_parse_wapp_dissassoc_stats(global, (struct client_disassociation_stats_event *)wapp_event->buffer, wapp_event->length, from);
				if (buf != NULL) {
					os_free(buf);
				}
				break;
			case WAPP_ALL_ASSOC_STA_EXTENDED_LINK_METRICS:
				topo_srv_parse_wapp_all_sta_extended_link_metrics(global, (struct ext_sta_link_metrics *)wapp_event->buffer);
				if (buf != NULL) {
					os_free(buf);
				}
				break;
			case WAPP_ONE_ASSOC_STA_EXTENDED_LINK_METRICS:
				topo_srv_parse_wapp_one_sta_extended_link_metrics(global, (struct ext_link_metrics *)wapp_event->buffer);
				if (buf != NULL) {
					os_free(buf);
				}
				break;
			case WAPP_RADIO_METRICS_INFO:
				topo_srv_parse_wapp_radio_metric_event(global, (struct radio_metrics_info *)wapp_event->buffer, from);
				if (buf != NULL) {
					os_free(buf);
				}
				break;
			case WAPP_R2_AP_CAP:
				topo_srv_parse_wapp_r2_ap_cap(global, (struct ap_r2_capability *)wapp_event->buffer, wapp_event->length);
				if (buf != NULL) {
					os_free(buf);
				}
				break;
#endif
			case WAPP_CH_LIST_DFS_INFO:
				topo_srv_parse_ch_list_dfs_info(global, (u8 *)wapp_event->buffer, wapp_event->length);
				if (buf != NULL) {
					os_free(buf);
				}
				break;
#ifdef MAP_R2
			case WAPP_MBO_STA_PREF_CH_LIST:
				topo_srv_parse_r2_mbo_sta_non_pref_list(global, wapp_event->buffer, wapp_event->length);
				if (buf != NULL) {
					os_free(buf);
				}
				break;
#endif
			case WAPP_CAC_PERIOD_ENABLE:
				if (wapp_event) {
					topo_srv_parse_wapp_cac_periodic_enable(global, wapp_event, ctx);
				}
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
				break;
			case WAPP_MAP_RESET:
				eloop_register_timeout(0, 0,
					wlanif_handle_map_reset,
						global, (void *)buf);
				break;
			case WAPP_WTS_CONFIG:
				topo_srv_parse_wts_config(global, (struct set_config_bss_info  *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
				break;
#ifdef MAP_R3
			case WAPP_SEND_DPP_MSG:
				topo_srv_send_dpp_frame(global, (struct dpp_msg *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
				break;
			case WAPP_SEND_URI_MSG:
				topo_srv_send_uri_msg(global, (struct dpp_uri_msg *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
				break;

			case WAPP_SEND_CCE_MSG:
				topo_srv_send_cce_frame(global, (struct cce_msg *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
				break;

			case WAPP_SEND_1905_CONNECTOR:
				topo_srv_send_1905_connector(global, (struct dpp_sec_cred *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
				break;

			case WAPP_SEND_BSS_CONNECTOR:
				topo_srv_send_bss_connector(global, (struct dpp_bss_cred *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
				break;
#endif /* MAP_R3 */
#ifdef MAP_R3_WF6
		       case WAPP_ASSOC_WIFI6_STA_STATUS:
			       mapd_printf(MSG_DEBUG, "Solicited WAPP_ASSOC_WIFI6_STA_STATUS CONSUME");
			       event_wrapper = os_zalloc(
					       sizeof(struct event_wrapper_s));
			       event_wrapper->from = from;
			       event_wrapper->event = buf;
			       wlanif_handle_assoc_wifi6_sta_status((void *)global, (void *)event_wrapper);
			       break;
			case WAPP_AP_WF6_CAPABILITY:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_AP_WF6_CAPABILITY"
								"from WAPP: CONSUME");
				wlanif_handle_ap_wf6_cap((void *)global, (void *)buf);
				break;

#endif
#ifdef MAP_R4_SPT
			case WAPP_AP_SPT_REUSE_REQ:
				mapd_printf(MSG_OFF, "DM:Solicited WAPP_AP_SPT_REUSE_REQ "
								"from WAPP: CONSUME\n");
				wlanif_handle_ap_spt_reuse_req((void *)global, (void *)buf);
				break;
#endif
#ifdef MAP_R3
			case WAPP_SEND_CHIRP_MSG:
				topo_srv_send_chirp_frame(global, (struct chirp_info *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
				break;
			case WAPP_SEND_DPP_DIRECT_MSG:
				topo_srv_send_direct_frame(global, (struct dpp_direct_msg *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
				break;
			case WAPP_SEND_CHIRP_NEW_MSG:
				mapd_printf(MSG_OFF, "\n WAPP_SEND_CHIRP_NEW_MSG recv");
				topo_srv_send_chirp_msg(global, (struct chirp_info *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
				break;
			case WAPP_SEND_CONN_FAIL_NOTIF:
				mapd_printf(MSG_OFF, "WAPP_SEND_CONN_FAIL_NOTIF\n");
				topo_srv_conn_failure_msg(global, (struct conn_fail_notif *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
				break;
			case WAPP_MAP_DPP_SAVED_CONFIG:
				/* To Do: Handle this event*/
				os_free(buf);
				printf("To Do handle WAPP_MAP_DPP_SAVED_CONFIG\n");
				break;
			case WAPP_SEND_USER_FAIL_NOTIF:
				mapd_printf(MSG_OFF, "WAPP_SEND_USER_FAIL_NOTIF\n");
				topo_srv_user_failure_msg(global, (struct user_fail_notif *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
				break;
			case WAPP_SEND_ONBOARD_TYPE:
				mapd_printf(MSG_OFF, "received WAPP_SEND_ONBOARD_TYPE\n");
				topo_srv_dpp_onboard_type(global, (u8 *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
				break;
			case WAPP_RX_DPP_URI:
				mapd_printf(MSG_OFF, "received WAPP_RX_DPP_URI\n");
				mapd_printf(MSG_DEBUG, "URI value from wapp:%s with len:%d\n", wapp_event->buffer, wapp_event->length);
				if (buf) {
					os_free(buf);
				}
				break;
#endif /* MAP_R3 */
			case WAPP_RX_BUFFER_INCR_EVT:
				mapd_printf(MSG_OFF, "received WAPP_RX_BUFFER_INCR_EVT\n");
				if (buf) {
					os_free(buf);
				}
				break;
#ifdef MAP_R4_SPT
			case WAPP_CH_SELECTION_INFO:
				debug("BSS color collision or srg event come at wapp");
				wlanif_handle_ch_select_info((void *)global, (void *)buf);
				break;
#endif
			case WAPP_MAP_AGENT_WPS_SUCCESS:
				mapd_printf(MSG_ERROR, "Solicited WAPP_MAP_AGENT_WPS_SUCCESS event at map\n");
				wlanif_handle_agent_wps_success((void *)global, (void *)buf);
				break;
			default:
				mapd_printf(MSG_ERROR, "Unhandled Solicited response to command from MAPD");
				if (buf) {
					os_free(buf);
				}
				break;
		}
	} else {
			switch (wapp_event->type) {
			case WAPP_POLICY_CONFIG_REQUEST:
			case WAPP_CLI_ASSOC_CNTRL_REQUEST:
			case WAPP_CH_SELECTION_REQUEST:
			case WAPP_CLI_STEER_REQUEST:
			case WAPP_1905_REQ:
				debug("Command to 1905 from WAPP, proxied by MAPD");
				/* 1905_command(buf, len); */ /* Send command to 1905 and synchronusly relay response to WAPP */
				if (buf) {
					os_free(buf);
				}
				break;
#ifdef SUPPORT_MULTI_AP
			case WAPP_CLI_CAPABILITY_REPORT:
				if (buf) {
					os_free(buf);
				}
				break;
#endif
			case WAPP_ALL_ASSOC_STA_TRAFFIC_STATS:
				if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
					debug("Solicited message WAPP_ALL_ASSOC_STA_TRAFFIC_STATS from WAPP to 1905D: sniff and forward it to 1905D");
				} else {
					debug("Un-solicited WAPP_ALL_ASSOC_STA_TRAFFIC_STATS` from WAPP: Sniff and forward it to 1905D");
				}
				event_wrapper = os_zalloc(
					sizeof(struct event_wrapper_s));
				event_wrapper->from = from;
				event_wrapper->event = buf;
				eloop_register_timeout(0,0, wlanif_handle_traffic_stats, global, event_wrapper);
				break;
			case WAPP_WIRELESS_INF_INFO:
				mapd_printf(MSG_OFF, "un-Solicited WAPP_WIRELESS_INF_INFO report");
				wlanif_handle_wireless_inf_info((void *)global, (void *)buf);
				break;
			case WAPP_ALL_ASSOC_TP_METRICS:
				if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
						mapd_printf(MSG_DEBUG, "Solicited message"
										"WAPP_ALL_ASSOC_TP_METRICS from WAPP to 1905D:"
										"sniff and forward it to 1905D");
				} else {
						mapd_printf(MSG_DEBUG, "Un-solicited WAPP_ALL_ASSOC_TP_METRICS"
										"from WAPP: Sniff and forward it to 1905D");
				}
				eloop_register_timeout(0,0, wlanif_handle_all_assoc_sta_tp_metrics, global, buf);
				break;
            case WAPP_TX_LINK_STATISTICS:
            case WAPP_CH_PREFER_QUERY:
                if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
                    debug("Solicited message from WAPP to 1905D: Does not require sniff..forward it to 1905D");
                } else {
			//command response received after timeout, hence do not handle : IGNORE
                    debug("Un-solicited message from WAPP: Does not require sniff..forward it to 1905D");
                }
                //1905_forward(buf, len);
				if (buf) {
					os_free(buf);
				}
                break;
            case WAPP_ALL_ASSOC_STA_LINK_METRICS:
                if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
                    debug("Solicited message WAPP_ALL_ASSOC_STA_LINK_METRICS from WAPP to 1905D: sniff and forward it to 1905D");
                } else {
                    debug("Un-solicited WAPP_ALL_ASSOC_STA_LINK_METRICS from WAPP: Sniff and forward it to 1905D");
                }
				event_wrapper = os_zalloc(
					sizeof(struct event_wrapper_s));
				event_wrapper->from = from;
				event_wrapper->event = buf;
				eloop_register_timeout(0,0, wlanif_handle_all_assoc_sta_link_metrics, global, event_wrapper);
                //1905_forward(buf, len);
                break;
            case WAPP_ONE_ASSOC_STA_LINK_METRICS:
                if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
                    debug("Solicited message WAPP_ONE_ASSOC_STA_LINK_METRICS from WAPP to 1905D: sniff and forward it to 1905D");
                } else {
                    debug("Un-solicited WAPP_ONE_ASSOC_STA_LINK_METRICS from WAPP: Sniff and forward it to 1905D");
                }
				event_wrapper = os_zalloc(
					sizeof(struct event_wrapper_s));
				event_wrapper->from = from;
				event_wrapper->event = buf;
				eloop_register_timeout(0,0, wlanif_handle_one_assoc_sta_link_metrics, global,
					event_wrapper);
                break;
#ifdef SUPPORT_MULTI_AP
            case WAPP_RX_LINK_STATISTICS:
                if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
                    debug("Solicited message WAPP_RX_LINK_STATISTICS from WAPP to 1905D: sniff and forward it to 1905D");
                } else {
                    debug("Un-solicited WAPP_RX_LINK_STATISTICS from WAPP: Sniff and forward it to 1905D");
                }
				eloop_register_timeout(0,0, wlanif_handle_rx_link_stats, global, buf);
                break;
            case WAPP_UNASSOC_STA_LINK_METRICS:
                if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
                    debug("Solicited message WAPP_UNASSOC_STA_LINK_METRICS from WAPP to 1905D: sniff and forward it to 1905D");
                } else {
                    debug("Un-solicited WAPP_UNASSOC_STA_LINK_METRICS from WAPP: Sniff and forward it to 1905D");
                }
				eloop_register_timeout(0, 0, wlanif_handle_unassoc_sta_link_metrics, global, buf);
                break;
#endif
            case WAPP_BEACON_METRICS_REPORT:
                if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
                    debug("Solicited message WAPP_BEACON_METRICS_REPORT from WAPP to 1905D: sniff and forward it to 1905D");
                } else {
                    debug("Un-solicited WAPP_BEACON_METRICS_REPORT from WAPP: Sniff and forward it to 1905D");
                }
				eloop_register_timeout(0, 0, wlanif_handle_bcn_metrics_report, global, buf);
                break;
            case WAPP_OPERATING_CHANNEL_INFO: //XXX: What is the differnce b/w this and WAPP_OPERATING_CHANNEL_REPORT:
				if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
						debug("Solicited message WAPP_OPERATING_CHANNEL_INFO from WAPP to 1905D: sniff and forward it to 1905D");
				} else {
						debug("Un-solicited WAPP_OPERATING_CHANNEL_INFO from WAPP: Sniff and forward it to 1905D");
				}
				if (global->wapp_get_radio_status == FALSE)
					wlanif_handle_op_chan_info((void *)global, (void *)buf);
				else
					eloop_register_timeout(0, 0, wlanif_handle_op_chan_info, global, buf);
				break;
			case WAPP_STA_BSSLOAD:
				if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
						debug("Solicited message WAPP_STA_BSSLOAD from WAPP to 1905D: sniff and forward it to 1905D");
				} else {
						debug("Un-solicited WAPP_STA_BSSLOAD Sniff and forward it to 1905D");
				}
				eloop_register_timeout(0, 0, wlanif_handle_op_bss_load, global, buf);
				break;
            case WAPP_CLIENT_NOTIFICATION:
                debug("Un-solicited WAPP_CLIENT_NOTIFICATION from WAPP: Sniff and forward it to 1905D");
                eloop_register_timeout(0,0, wlanif_handle_client_notification, global, buf);
                break;
            case WAPP_AP_HT_CAPABILITY:
				if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
						debug("Solicited message WAPP_AP_HT_CAPABILITY from WAPP to 1905D: sniff and forward it to 1905D");
				} else {
						debug("Un-solicited WAPP_AP_HT_CAPABILITY Sniff and forward it to 1905D");
				}
				eloop_register_timeout(0,0, wlanif_handle_ap_ht_cap, global, buf);
				break;
            case WAPP_AP_VHT_CAPABILITY:
				if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
						debug("Solicited message WAPP_AP_VHT_CAPABILITY from WAPP to 1905D: sniff and forward it to 1905D");
				} else {
						debug("Un-solicited WAPP_AP_VHT_CAPABILITY Sniff and forward it to 1905D");
				}
				eloop_register_timeout(0,0, wlanif_handle_ap_vht_cap, global, buf);
				break;
            case WAPP_OPERBSS_REPORT:
				if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
						debug("Solicited message WAPP_OPERBSS_REPORT from WAPP to 1905D: sniff and forward it to 1905D");
				} else {
						debug("Un-solicited WAPP_OPERBSS_REPORT Sniff and forward it to 1905D");
				}
				eloop_register_timeout(0,0, wlanif_handle_oper_bss_report, global, buf);
                break;
            case WAPP_CLI_STEER_BTM_REPORT:
				if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
						debug("Solicited message WAPP_CLI_STEER_BTM_REPORT from WAPP to 1905D: sniff and forward it to 1905D");
				} else {
						debug("Un-solicited WAPP_CLI_STEER_BTM_REPORT Sniff and forward it to 1905D");
				}
				eloop_register_timeout(0, 0, wlanif_handle_steer_btm_report, global, buf);
                break;
            case WAPP_AP_METRICS_INFO:
				if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
						debug("Solicited message WAPP_AP_METRICS_INFO from WAPP to 1905D: sniff and forward it to 1905D");
				} else {
						debug("Un-solicited WAPP_AP_METRICS_INFO Sniff and forward it to 1905D");
				}
				event_wrapper = os_zalloc(
					sizeof(struct event_wrapper_s));
				event_wrapper->from = from;
				event_wrapper->event = buf;
				eloop_register_timeout(0, 0, wlanif_handle_ap_metrics_info, global,
				event_wrapper);
				break;
#ifdef SUPPORT_MULTI_AP
            case WAPP_NAC_INFO: //not in API doc???
				if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
						debug("Solicited message WAPP_NAC_INFO from WAPP to 1905D: sniff and forward it to 1905D");
				} else {
						debug("Un-solicited WAPP_NAC_INFO Sniff and forward it to 1905D");
				}
				eloop_register_timeout(0, 0, wlanif_handle_nac_info, global, buf);
				break;
#endif
			case WAPP_RADIO_BASIC_CAP:
				mapd_printf(MSG_DEBUG, "Solicited WAPP_RADIO_BASIC_CAP report");
				eloop_register_timeout(0, 0, wlanif_handle_radio_basic_cap, global, buf);
				break;

			case WAPP_UPDATE_PROBE_INFO:
				mapd_printf(MSG_DEBUG, "Unsolicited WAPP_UPDATE_PROBE_INFO event");
				eloop_register_timeout(0, 0, wlanif_handle_update_probe_info, global, buf);
				break;

			case WAPP_STA_CNNCT_REJ_INFO:
				mapd_printf(MSG_DEBUG, "Unsolicited WAPP_STA_CNNCT_REJ_INFO");
				eloop_register_timeout(0, 0, wlanif_handle_sta_cnnct_rej_info, global, buf);
				break;

			case WAPP_STA_STAT:
				mapd_printf(MSG_DEBUG, "Unsolicited WAPP_STA_STAT");
				eloop_register_timeout(0, 0, wlanif_handle_sta_activity_status,
						(void *)global, (void *)buf);
				break;
#ifdef SUPPORT_MULTI_AP
			case WAPP_MAP_BH_CONFIG:
			{
				mapd_printf(MSG_DEBUG, "Unsolicited WAPP_MAP_BH_CONFIG");
				wlanif_handle_bh_config_event((void *)global, (void *)buf);
				break;
			}
			case WAPP_BRIDGE_IP:
				mapd_printf(MSG_DEBUG, "Unsolicited WAPP_BRIDGE_IP");
				wlanif_handle_br_ip_event((void *)global, (void *)buf);
				break;
			case WAPP_SET_BH_TYPE:
				mapd_printf(MSG_DEBUG, "Unsolicited WAPP_SET_BH_TYPE");
				wlanif_handle_set_bh_type((void *)global, (void *)buf);
				break;
#ifdef AUTOROLE_NEGO
			case WAPP_MAP_NEGO_ROLE_RESP:
				own_dev = topo_srv_get_1905_device(&global->dev, NULL);
				if (!own_dev) {
					err("own 1905 dev is missing\n");
					break;
				}
				if(global->dev.own_new_DevRole == DEVICE_ROLE_AGENT) {
					err("you need to be agent for sure\n");
					break;
				}
				dev_role_event = (struct dev_role_negotiate *)wapp_event->buffer;
				err("In map got WAPP event WAPP_MAP_NEGO_ROLE_RESP other dev role %d , your new role %d\n",dev_role_event->other_dev_role, global->dev.own_new_DevRole);
				if((ctx->device_role == DEVICE_ROLE_UNCONFIGURED) && (global->dev.ThirdPartyConnection)) {
					if(dev_role_event->other_dev_role == DEVICE_ROLE_CONTROLLER) {
						err("other dev is controller , so you need to become agent\n");
						global->dev.own_new_DevRole = DEVICE_ROLE_AGENT;
					} else if(dev_role_event->other_dev_role == DEVICE_ROLE_UNCONFIGURED){
						debug("Other dev ALMAC "MACSTR", own dev ALMAC "MACSTR"\n", MAC2STR(dev_role_event->other_dev_almac), MAC2STR(own_dev->_1905_info.al_mac_addr));
						while(j < ETH_ALEN) {
							if (own_dev->_1905_info.al_mac_addr[j] < dev_role_event->other_dev_almac[j]) {
								err("FORCE agent: own %d, other %d\n", own_dev->_1905_info.al_mac_addr[j],dev_role_event->other_dev_almac[j] );
								global->dev.own_new_DevRole = DEVICE_ROLE_AGENT;
							break;
							} else if (own_dev->_1905_info.al_mac_addr[j] > dev_role_event->other_dev_almac[j]) {
								err("keep previous role but stop checking further\n");
							break;
							} else {
								debug("J++\n");
								j=j+1;
							}
						}
					}
				}
				err("NEW Role is %d\n",global->dev.own_new_DevRole);
				if(global->dev.own_new_DevRole != DEVICE_ROLE_UNCONFIGURED) {
					err("need to cancel timer and configure role\n");
					eloop_cancel_timeout(map_1905_poll_timeout, global, ctx);
					eloop_register_timeout(0,0, map_1905_poll_timeout, global, ctx);
				}
				if (buf) {
					os_free(buf);
					buf = NULL;
				}
				break;
#endif // AUTOROLE_NEGO
			case WAPP_AIR_MONITOR_REPORT:
				eloop_register_timeout(0,0,
					wlanif_handle_air_monitor_report,
					global, (void *)buf);
				break;
			case WAPP_STEERING_COMPLETED://XXX: Usage?
				eloop_register_timeout(0,0,
					wlanif_handle_cli_steering_complete,
					global, (void *)buf);
				break;
			case WAPP_1905_READ_BSS_CONF_REQUEST:
				eloop_register_timeout(0,0,
					wlanif_handle_1905_read_bss_conf_req,
					global, (void *)buf);
				break;
			case WAPP_OPERATING_CHANNEL_REPORT:
				eloop_register_timeout(0,0,
					wlanif_handle_operating_channel_report,
					global, (void *)buf);
				break;
			case WAPP_MAP_BH_READY:
				eloop_register_timeout(0,0,
					wlanif_handle_bh_ready,
					global, (void *)buf);
				break;
			case WAPP_1905_CMDU_REQUEST:
				eloop_register_timeout(0,0,
					wlanif_handle_1905_cmdu_req,
					global, (void *)buf);
				break;
			case WAPP_RADIO_OPERATION_RESTRICTION:
				eloop_register_timeout(0,0,
					wlanif_handle_radio_op_restrict,
					global, (void *)buf);
				break;
			case WAPP_CHANNLE_PREFERENCE:
				eloop_register_timeout(0,0,
					wlanif_handle_channel_pref,
					global, (void *)buf);
				break;
			case WAPP_BACKHAUL_STEER_RSP:
				eloop_register_timeout(0,0,
					wlanif_handle_bh_steer_resp,
					global, (void *)buf);
				break;
			case WAPP_AP_CAPABILITY:
					eloop_register_timeout(0,0,
						wlanif_handle_ap_capability,
						global, (void *)buf);
				break;
			case WAPP_SCAN_RESULT:
				eloop_register_timeout(0,0,
					wlanif_handle_map_scan_result,
					global, (void *)buf);
			break;
			case WAPP_SCAN_DONE:
				eloop_register_timeout(0,0,
					wlanif_handle_map_scan_done,
					global, (void *)buf);
			break;
			case WAPP_MAP_VEND_IE_CHANGED:
				eloop_register_timeout(0,0,
					wlanif_handle_map_ie_changed,
					global, (void *)buf);
			break;
			case WAPP_GET_WSC_CONF:
				eloop_register_timeout(0,0,
					wlanif_handle_get_wsc_config,
					global, (void *)buf);
			break;
			case WAPP_AP_LINK_METRIC_REQ:
				eloop_register_timeout(0,0,
					wlanif_handle_ap_link_metric_info,
					global, (void *)buf);
			break;
			case WAPP_APCLI_ASSOC_STAT_CHANGE:
			/* break not needed */
			case WAPP_APCLI_UPLINK_RSSI:
				eloop_register_timeout(0,0,
					wlanif_handle_assoc_state_change,
					global, (void *)buf);
				break;
			break;
			case WAPP_1905_READ_1905_TLV_REQUEST:
				eloop_register_timeout(0,0,
					wlanif_handle_1905_tlv_read,
					global, (void *)buf);
				break;
			case WAPP_DEVICE_STATUS:
				eloop_register_timeout(0,0,
					wlanif_handle_device_status,
					global, (void *)buf);
				break;
			case WAPP_OFF_CH_SCAN_REPORT:
				eloop_register_timeout(0,0,
					wlanif_handle_off_channel_scan_result,
					global, (void *)buf);
				break;
			case WAPP_NET_OPT_SCAN_REPORT:
				err("in MAP WAPP_NET_OPT_SCAN_REPORT ");
				eloop_register_timeout(0,0,
					wlanif_handle_net_opt_scan_result,
					global, (void *)buf);
				break;
			case WAPP_MAP_RENEW:
				err("in map receive from wapp : WAPP_MAP_RENEW\n");
				eloop_register_timeout(0, 0,
					wlanif_handle_mapd_renew,
					global, (void *)buf);
			break;
#endif /* #ifdef SUPPORT_MULTI_AP */
			case WAPP_AP_HE_CAPABILITY:
				if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
						debug("Solicited message WAPP_AP_HE_CAPABILITY from WAPP to 1905D: sniff and forward it to 1905D");
				} else {
						debug("Un-solicited WAPP_AP_HE_CAPABILITY Sniff and forward it to 1905D");
				}
				eloop_register_timeout(0,0, wlanif_handle_ap_he_cap, global, buf);
				break;
#ifdef MAP_R3_DE
			case WAPP_DEV_INVEN_TLV:
				if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
						debug("Solicited message WAPP_DEV_INVEN_TLV from WAPP to 1905D: sniff and forward it to 1905D");
				} else {
						debug("Un-solicited WAPP_DEV_INVEN_TLV Sniff and forward it to 1905D");
				}
				eloop_register_timeout(0,0, wlanif_handle_dev_inven_tlv, global, buf);
				break;

#endif /*MAP_R3_DE*/
#ifdef MAP_R2
		case WAPP_ASSOC_STATUS_NOTIFICATION:
			topo_srv_parse_wapp_assoc_status_notif(global,(struct assoc_notification *)wapp_event->buffer);
			if (wapp_event != NULL) {
				os_free(wapp_event);
			}
			break;
		case WAPP_TUNNELED_MESSAGE:
			if(global->params.Certification)
#ifdef MAP_R4
				eloop_register_timeout(0, 0, wlanif_handle_tunneled_frame, global, buf);
#else
				eloop_register_timeout(1, 0, wlanif_handle_tunneled_frame, global, buf);
#endif /* MAP_R4 */
			else
				eloop_register_timeout(0, 0, wlanif_handle_tunneled_frame, global, buf);
			break;
		case WAPP_CHANNEL_SCAN_REPORT:
			topo_srv_parse_wapp_ch_scan_report(global, (struct net_opt_scan_report_event *)wapp_event->buffer, wapp_event->length);
			if (buf != NULL) {
				os_free(buf);
			}
			break;
#endif
		case WAPP_CAC_COMPLETION_REPORT:
			topo_srv_parse_wapp_cac_completion_report(global, wapp_event, wapp_event->length);
			break;
#ifdef MAP_R2
#ifdef DFS_CAC_R2
		case WAPP_CAC_STATUS_REPORT:
			topo_srv_parse_wapp_cac_status_report(global, wapp_event, wapp_event->length);
			break;
#endif
		case WAPP_DISASSOC_STATS_EVT:
			topo_srv_parse_wapp_dissassoc_stats(global,(struct client_disassociation_stats_event *)wapp_event->buffer, wapp_event->length, from);
			if (buf != NULL){
				os_free(buf);
			}
			break;
#endif
		case WAPP_CH_LIST_DFS_INFO:
			topo_srv_parse_ch_list_dfs_info(global, (u8 *)wapp_event->buffer, wapp_event->length);
			if (wapp_event != NULL) {
				os_free(wapp_event);
			}
			break;
#ifdef MAP_R2
		case WAPP_RADIO_METRICS_INFO:
			topo_srv_parse_wapp_radio_metric_event(global,(struct radio_metrics_info *)wapp_event->buffer, from);
			if (wapp_event != NULL) {
				os_free(wapp_event);
			};
			break;
		case WAPP_MBO_STA_PREF_CH_LIST:
			topo_srv_parse_r2_mbo_sta_non_pref_list(global, wapp_event->buffer, wapp_event->length);
			if (wapp_event != NULL) {
				os_free(wapp_event);
			}
			break;
#endif
		case WAPP_CAC_PERIOD_ENABLE:
			if(wapp_event){
				topo_srv_parse_wapp_cac_periodic_enable(global, wapp_event, ctx);
			}
			if (wapp_event != NULL) {
				os_free(wapp_event);
			}
			break;
		case WAPP_MAP_RESET:
			eloop_register_timeout(0,0,
				wlanif_handle_map_reset,
					global, (void *)buf);
			break;
		case WAPP_WTS_CONFIG:
			topo_srv_parse_wts_config(global,(struct set_config_bss_info  *)wapp_event->buffer, wapp_event->length);
			if (wapp_event != NULL) {
				os_free(wapp_event);
			}
			break;
#ifdef MAP_R3
		case WAPP_SEND_DPP_MSG:
			topo_srv_send_dpp_frame(global, (struct dpp_msg *)wapp_event->buffer, wapp_event->length);
			if (wapp_event != NULL) {
				os_free(wapp_event);
			}
			break;
		case WAPP_SEND_URI_MSG:
			topo_srv_send_uri_msg(global, (struct dpp_uri_msg *)wapp_event->buffer, wapp_event->length);
			if (wapp_event != NULL) {
				os_free(wapp_event);
			}
			break;
		case WAPP_SEND_CCE_MSG:
			topo_srv_send_cce_frame(global, (struct cce_msg *)wapp_event->buffer, wapp_event->length);
			if (wapp_event != NULL) {
				os_free(wapp_event);
			}
			break;

		case WAPP_SEND_1905_CONNECTOR:
			topo_srv_send_1905_connector(global, (struct dpp_sec_cred *)wapp_event->buffer, wapp_event->length);
			if (wapp_event != NULL) {
				os_free(wapp_event);
			}
			break;

		case WAPP_SEND_BSS_CONNECTOR:
			topo_srv_send_bss_connector(global, (struct dpp_bss_cred *)wapp_event->buffer, wapp_event->length);
			if (wapp_event != NULL) {
				os_free(wapp_event);
			}
			break;
#endif /* MAP_R3 */
#ifdef MAP_R3_WF6
            case WAPP_ASSOC_WIFI6_STA_STATUS:
				if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
                    debug("Solicited message WAPP_ASSOC_WIFI6_STA_STATUS from WAPP to 1905D: sniff and forward it to 1905D");
                } else {
                    debug("Un-solicited WAPP_ASSOC_WIFI6_STA_STATUS from WAPP: Sniff and forward it to 1905D");
                }
                event_wrapper = os_zalloc(
                sizeof(struct event_wrapper_s));
                event_wrapper->from = from;
                event_wrapper->event = buf;
                eloop_register_timeout(0,0, wlanif_handle_assoc_wifi6_sta_status, global, event_wrapper);
                break;
	       case WAPP_AP_WF6_CAPABILITY:
		       if ((type !=0) && (type == wapp_event->type) && (from == 1)) {
			       debug("Solicited message WAPP_AP_WF6_CAPABILITY from WAPP to 1905D: sniff and forward it to 1905D");
		       } else {
			       debug("Un-solicited WAPP_AP_WF6_CAPABILITY Sniff and forward it to 1905D");
		       }
		       eloop_register_timeout(0,0, wlanif_handle_ap_wf6_cap, global, buf);
		       break;

#endif
#ifdef MAP_R3
		//Prakhar
		case WAPP_SEND_CHIRP_MSG:
				topo_srv_send_chirp_frame(global, (struct chirp_info *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
		break;
		case WAPP_SEND_DPP_DIRECT_MSG:
				topo_srv_send_direct_frame(global, (struct dpp_direct_msg *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
		break;
		case WAPP_SEND_CHIRP_NEW_MSG:
				topo_srv_send_chirp_msg(global, (struct chirp_info *)wapp_event->buffer, wapp_event->length);
				if (wapp_event != NULL) {
					os_free(wapp_event);
				}
		break;
		case WAPP_SEND_CONN_FAIL_NOTIF:
			printf("WAPP_SEND_CONN_FAIL_NOTIF\n");
			topo_srv_conn_failure_msg(global, (struct conn_fail_notif *)wapp_event->buffer, wapp_event->length);
			if (wapp_event != NULL) {
				os_free(wapp_event);
			}
			break;
		case WAPP_MAP_DPP_SAVED_CONFIG:
			/* To Do: Handle this event*/
			os_free(buf);
			printf("To Do handle WAPP_MAP_DPP_SAVED_CONFIG\n");
			break;
		case WAPP_SEND_USER_FAIL_NOTIF:
			printf("WAPP_SEND_USER_FAIL_NOTIF\n");
			topo_srv_user_failure_msg(global, (struct user_fail_notif *)wapp_event->buffer, wapp_event->length);
			if (wapp_event != NULL) {
				os_free(wapp_event);
			}
			break;
		case WAPP_SEND_ONBOARD_TYPE:
			printf("WAPP_SEND_ONBOARD_TYPE\n");
			topo_srv_dpp_onboard_type(global, (u8 *)wapp_event->buffer, wapp_event->length);
			if (wapp_event != NULL) {
				os_free(wapp_event);
			}
			break;
#endif /* MAP_R3 */
		case WAPP_SEND_BUFFER_INCR:

			os_memcpy((char*)&buffer_size,&wapp_event->buffer[0],sizeof(buffer_size));
			debug("WAPP_SEND_BUFFER_INCR len:%d\n",buffer_size);


			if (global->wapp_map_buffer) {

				global->wapp_map_buffer = os_realloc(global->wapp_map_buffer, buffer_size);
					if(!global->wapp_map_buffer) {
					err("wapp_map_buffer allocation failed for %d bytes!!!!!\n",buffer_size);
						assert(0);
				} else {
					/*Send buffer increase success intimation to wapp*/
					wapp_send_incr_conf(global->wapp_ctrl,1);
					global->wapp_map_buffer_size = buffer_size;
				}
			} else {
				err("wapp_map_buffer is NULL !!!!!\n");
				assert(0);
			}
			if (wapp_event != NULL) {
				os_free(wapp_event);
			}
			break;
#ifdef MAP_R3_RECONFIG
		case WAPP_RECONFIG_STATUS:
        	printf("WAPP_USER_GET_RECONFIG_TRIGGER %d\n",*(u8 *)wapp_event->buffer);
			struct own_1905_device *ctx = &global->dev;
			ctx->ReconfigTrigger = *(u8 *)wapp_event->buffer;
			if (buf != NULL) {
				os_free (buf);
			}
            break;
#endif
#ifdef MAP_R4_SPT
		case WAPP_CH_SELECTION_INFO:
			debug("BSS color collision or srg event come at wapp");
			wlanif_handle_ch_select_info((void *)global, (void *)buf);
			break;
		case WAPP_UPLINK_TRAFFIC_STATUS:
			debug("WAPP_UPLINK_TRAFFIC_STATUS at mapp");
			eloop_register_timeout(0, 0, wlanif_handle_uplink_status_event, global, buf);
			break;
#endif
		case WAPP_MAP_AGENT_WPS_SUCCESS:
			mapd_printf(MSG_ERROR, "Unsolicited WAPP_MAP_AGENT_WPS_SUCCESS event at map\n");
			eloop_register_timeout(0, 0, wlanif_handle_agent_wps_success, global, buf);
			break;
		default:
			if (wapp_event != NULL)
				debug("Unhandled response / command %x to 1905D ", wapp_event->type);
			if (buf) {
				os_free(buf);
			}
			return -1;
			/*break;*/ /*This comment just for readability*/
        }
		if (type != 0 && wapp_type != type)
			return -1;
    }
    return 0;
}

/* Asyncronous events from WAPP daemon */
void wlanif_process_wapp_events(struct mapd_global *global, char *buf, size_t length)
{
    mapd_printf(MSG_MSGDUMP, "MSG FROM WAPP");
    wapp_usr_intf_parse_event(global, (char *)buf, length, 0, -1);
}

static void mapd_recv_pending_from_wapp(struct wapp_usr_intf_ctrl *ctrl,
        struct mapd_global *global)
{
    /* No need to wait..I already know there is something to be received */
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	if (ctrl_conn == NULL) {
		err("WHAT??");
		return;
	}

    while (wapp_usr_intf_ctrl_pending(ctrl, &tv) > 0) {

        size_t len = global->wapp_map_buffer_size - 1;
		if(!global->wapp_map_buffer){
            err("wapp_map_buffer is NULL");
			break;
		}
		os_memset(global->wapp_map_buffer, 0, global->wapp_map_buffer_size);
        if (wapp_usr_intf_ctrl_recv(ctrl, (char*)global->wapp_map_buffer, &len) == 0) {
            global->wapp_map_buffer[len] = '\0';
            wlanif_process_wapp_events(global,(char*) global->wapp_map_buffer, len);
		} else {
			err("Could not read pending message.");
			break;
		}
	}

	if (wapp_usr_intf_ctrl_pending(ctrl, &tv) < 0) {
		err("**********Connection to WAPP lost*********");
	}
}

//Used by both set and get
static int wapp_usr_intf_send_command(struct mapd_global *global,
        char *buffer_send, int length)
{
	if(global->wapp_ctrl == NULL) {
		mapd_printf(MSG_ERROR, "wapp_ctrl is NULL, check conenction with WAPP");
		return -1;
	}
	return wapp_usr_intf_ctrl_request(global->wapp_ctrl, buffer_send, length);
}


int wapp_wait_recv_parse_wapp_resp(struct mapd_global *global, char* buf,
        size_t len, unsigned char event_type, long sec, long usec, int from)
{

    struct timeval tv;
    int ret = 0;
    size_t orig_len = len;
	struct os_reltime now;
	struct os_reltime started;

	os_memset(&now, 0, sizeof(struct os_reltime));
	os_memset(&started, 0, sizeof(struct os_reltime));
	os_get_reltime(&started);

    tv.tv_sec = 2;
    tv.tv_usec = 0;
    while(1)
    {
        len = orig_len;
        ret = wapp_usr_intf_ctrl_pending(global->wapp_ctrl, &tv);
        if(ret == 1)
        {
		if (wapp_usr_intf_ctrl_recv(global->wapp_ctrl, buf, &len) < 0) {
			err("recv wapp fail");
			return -1;
		} else if (len <= orig_len) {
			if (wapp_usr_intf_parse_event(global, buf, len, event_type, from) != -1) {
				return len;
			}
		}
	}
        else
        {
            err("[%s]wait for event timeout", __func__);
            return 0;
        }

		os_get_reltime(&now);
		if (os_reltime_expired(&now, &started, sec)) {
			err("[%s]wait for event type:%d timeout", __func__, event_type);
			return 0;
		}
    }
}


int wlanif_issue_wapp_command(struct mapd_global *global, int msgtype,
        int waitmsgtype, unsigned char *bssid, unsigned char *stamac,
        void *data, int datalen, int from, int resp_expected, int cmd_role)
{
    struct cmd_to_wapp *cmd = NULL;
    unsigned char *buf = NULL;
    unsigned char *recv_buf = NULL;
    int send_pkt_len = 0, recv_pkt_len = 0;
	int ret;

    send_pkt_len = sizeof(struct cmd_to_wapp) + datalen;

	/* If send buffer is increased more than limit, send event and wait for resp */
	if(send_pkt_len > global->map_snd_buffer_size) {
		ret = wapp_send_buf_incr_evt(global, send_pkt_len);
		if (ret != 0) {
			err("buffer increase handshake failed, can't send message");
			return -1;
		}
	}

	buf = os_zalloc(send_pkt_len+1);
	if (buf == NULL) {
                mapd_printf(MSG_ERROR, "%s  Alloc memory failed !!!!! \n", __func__);
                return -1;
        }

    cmd = (struct cmd_to_wapp *)buf;
    cmd->type = msgtype;
    cmd->role = cmd_role;

    if (bssid)
        memcpy(cmd->bssAddr, bssid, ETH_ALEN);
    if (stamac)
        memcpy(cmd->staAddr, stamac, ETH_ALEN);
    if (data)
        memcpy(cmd->body, data, datalen);

    if (msgtype == WAPP_USER_SET_VENDOR_IE
#ifdef MAP_R2
      || msgtype == WAPP_USER_SET_CHANNEL_SCAN_REQ
#endif
#ifdef MAP_R3
	|| msgtype == WAPP_USER_SET_SERVICE_PRIORITIZATION_RULE
	|| msgtype == WAPP_USER_SET_DSCP_MAPPINT_TABLE
	|| msgtype == WAPP_USER_SEND_DPP_FRAME
	|| msgtype == WAPP_USER_SEND_AUTOCONFIG_TRIGGER
	|| msgtype == WAPP_USER_SEND_DPP_DIRECT_FRAME
	|| msgtype == WAPP_USER_SEND_CHIRP_TLV_FRAME

#endif /* MAP_R3 */
    )
        cmd->length = datalen;
    send_pkt_len = sizeof(struct cmd_to_wapp) + datalen;
    mapd_printf(MSG_DEBUG, "%s (0x%04x) To WAPP from=%d rsp_expected=%d",
					__func__, msgtype, from, resp_expected);
	ret = wapp_usr_intf_send_command(global, (char *)buf, send_pkt_len);
	if (0 > ret) {
		err("send msgtype(0x%04x) fail", msgtype);
		if (ret == -1) {
			err("not EAGAIN/EBUSY/EWOULDBLOCK cause send fail, need try reconnect");
			wapp_open_reconnection(global);
		}
		os_free(buf);
		return -1;
	}
    if(resp_expected) {
		if(!global->wapp_map_buffer) {
			os_free(buf);
			err("wapp_map_buffer NULL");
			return -1;
		}
		os_memset(global->wapp_map_buffer, 0, global->wapp_map_buffer_size);
        recv_buf = global->wapp_map_buffer;
        recv_pkt_len = wapp_wait_recv_parse_wapp_resp(global, (char *)recv_buf,
                global->wapp_map_buffer_size, waitmsgtype, 2, 0, from);
        if (0 >= recv_pkt_len) {
            mapd_printf(MSG_ERROR, "receive waitmsgtype(0x%04x) fail", waitmsgtype);
	    os_free(buf);
            return -1;
        }
    }
	os_free(buf);
    return 0;
}

/* BTM Request */
void wlanif_trigger_btm_req(struct mapd_global *global, u8 *mac_addr,
        u8 *curr_bssid, u8 *target_bssid, u8 target_op_chan, u8 target_op_class,
        u8 disassoc_imm, u8 btm_abridged, u16 btm_disassoc_timer)
{
    struct steer_request *req = NULL;
    struct target_bssid_info *b_info = NULL;
#ifdef MAP_R2
    struct mapd_bss *own_bss = NULL;
#endif
    mapd_printf(MSG_ERROR, "[Steer] STA " MACSTR " Current bssid " MACSTR " Target bssid " MACSTR,
					 MAC2STR(mac_addr), MAC2STR(curr_bssid), MAC2STR(target_bssid));

    req = os_zalloc(sizeof(struct steer_request) +
            sizeof(struct target_bssid_info));
    os_memcpy(req->assoc_bssid, curr_bssid, ETH_ALEN);
    req->request_mode = 1;
    req->btm_disassoc_immi = disassoc_imm;
    req->btm_abridged = btm_abridged;
    req->steer_window = 0;
    req->btm_disassoc_timer = btm_disassoc_timer;
    req->sta_count = 1;
    req->target_bssid_count = 1;
#ifdef MAP_R2
    own_bss = mapd_get_bss_from_mac(global, curr_bssid);
	if (own_bss && own_bss->_1905_steer_req_msg){
		req->steering_type = own_bss->_1905_steer_req_msg->steering_type;
		req->btm_disassoc_immi = own_bss->_1905_steer_req_msg->btm_disassoc_immi;
		req->btm_abridged = own_bss->_1905_steer_req_msg->btm_abridged;
		req->btm_disassoc_timer = own_bss->_1905_steer_req_msg->btm_disassoc_timer;
	}
#endif
    b_info = (struct target_bssid_info *)req->info;
    os_memcpy(&b_info->target_bssid[0], target_bssid, ETH_ALEN);
    os_memcpy(&b_info->sta_mac[0], mac_addr, ETH_ALEN);
    b_info->op_class = target_op_class;
    b_info->channel = target_op_chan;
    wlanif_issue_wapp_command(global, WAPP_USER_SET_STEERING_SETTING,
            WAPP_CLI_STEER_BTM_REPORT, NULL, NULL, (void *)req,
			sizeof(struct steer_request) + sizeof(struct target_bssid_info), 0, 0, 0);
	os_free(req);
}

void wlanif_get_all_assoc_sta_link_metrics(struct mapd_global *global, u8 *radio_id)
{
    wlanif_issue_wapp_command(global, WAPP_USER_GET_ASSOC_STA_LINK_METRICS,
            WAPP_ALL_ASSOC_STA_LINK_METRICS, NULL, NULL, (void *)radio_id, ETH_ALEN, 0, 1, 0);
}
#ifdef MAP_R2
void wlanif_get_all_assoc_sta_ext_link_metrics(struct mapd_global *global, u8 *radio_id)
{
	debug("WAPP_USER_GET_ASSOC_STA_EXTENDED_LINK_METRICS issuing");
    wlanif_issue_wapp_command(global, WAPP_USER_GET_ASSOC_STA_EXTENDED_LINK_METRICS,
            WAPP_ALL_ASSOC_STA_EXTENDED_LINK_METRICS, NULL, NULL, (void *)radio_id, ETH_ALEN, 0, 1, 0);
}
#endif

void wlanif_get_all_assoc_sta_tp_metrics(struct mapd_global *global, u8 *radio_id)
{
    wlanif_issue_wapp_command(global, WAPP_USER_GET_ALL_ASSOC_TP_METRICS,
            WAPP_ALL_ASSOC_TP_METRICS, NULL, NULL, (void *)radio_id, ETH_ALEN, 0, 1, 0);
}

void wlanif_get_assoc_sta_traffic_stats(struct mapd_global *global, unsigned char *radio_id)
{
    wlanif_issue_wapp_command(global, WAPP_USER_GET_ASSOC_STA_TRAFFIC_STATS,
            WAPP_ALL_ASSOC_STA_TRAFFIC_STATS, NULL, NULL, (void *)radio_id, ETH_ALEN, 0, 1, 0);
}

void wlanif_get_op_chan_info(struct mapd_global *global)
{
		wlanif_issue_wapp_command(global, WAPP_USER_GET_OPERATING_CHANNEL_INFO,
						WAPP_OPERATING_CHANNEL_INFO, NULL, NULL, NULL, 0, 0, 1, 0);
}

void wlanif_get_ap_cap(struct mapd_global *global, unsigned char *identifier)
{
		/* HT CAP */
		wlanif_issue_wapp_command(global, WAPP_USER_GET_AP_HT_CAPABILITY,
						WAPP_AP_HT_CAPABILITY, identifier, NULL, NULL, 0, 0, 1, 0);
		/* VHT CAP */
		wlanif_issue_wapp_command(global, WAPP_USER_GET_AP_VHT_CAPABILITY,
						WAPP_AP_VHT_CAPABILITY, identifier, NULL, NULL, 0, 0, 1, 0);
}

/* OK */
void wlanif_get_op_bss_info(struct mapd_global *global, unsigned char *identifier)
{
	mapd_printf(MSG_INFO, "%s: RUID=" MACSTR, __func__, MAC2STR(identifier));
	wlanif_issue_wapp_command(global, WAPP_USER_GET_OPERATIONAL_BSS,
					WAPP_OPERBSS_REPORT, identifier, NULL, NULL, 0, 0, 1, 0);
}

/* OK */
int wlanif_get_ap_metrics_info(struct mapd_global *global, u8 *bssid)
{
	mapd_printf(MSG_DEBUG, "%s: bssid=" MACSTR, __func__, MAC2STR(bssid));
	return wlanif_issue_wapp_command(global, WAPP_USER_GET_AP_METRICS_INFO,
					WAPP_AP_METRICS_INFO, bssid, NULL, NULL, 0, 0, 1, 0);
}

int wlanif_deauth_sta(struct mapd_global *global, u8 *mac_addr, u8 *bssid)
{
	mapd_printf(MSG_ERROR, "mac_addr=" MACSTR, MAC2STR(mac_addr));
	return wlanif_issue_wapp_command(global, WAPP_USER_SET_DEAUTH_STA,
					0, bssid, mac_addr, NULL, 0, 0, 0, 0);
}

int wlanif_disconnect_apcli(struct mapd_global *global, unsigned char *intfname)
{

	if(intfname) {
		err("%s ",intfname);
		return wlanif_issue_wapp_command(global, WAPP_USER_ISSUE_APCLI_DISCONNECT,
					0, NULL, NULL, intfname, os_strlen((const char *)intfname), 0, 0, 0);
	} else {
		err("*");
		return wlanif_issue_wapp_command(global, WAPP_USER_ISSUE_APCLI_DISCONNECT,
					0, NULL, NULL, NULL, 0, 0, 0, 0);
	}
}

int wlanif_flush_bl_for_bss(struct mapd_global *global, u8 *bssid)
{
		return wlanif_issue_wapp_command(global, WAPP_USER_FLUSH_ACL,
					0, bssid, NULL, NULL,
					0, 0, 0, 0);
}

/* OK */
int wlanif_bl_sta_for_bss(struct mapd_global *global, u8 *mac_addr,
		u8 *bssid, Boolean blacklist)
{
	struct cli_assoc_control *assoc_ctrl = NULL;
	int ret = 0;

	mapd_printf(MSG_DEBUG, "%s: %sBLACKLIST STA=(" MACSTR ") BSSID=(" MACSTR ")",
					__func__, (blacklist == TRUE ? "" : "UN-"), MAC2STR(mac_addr),
					MAC2STR(bssid));

	assoc_ctrl = (struct cli_assoc_control *)
			os_zalloc(sizeof(struct cli_assoc_control) + ETH_ALEN);
	os_memcpy(assoc_ctrl->bssid, bssid, ETH_ALEN);
	assoc_ctrl->assoc_control = !blacklist; // 0: BL 1: UN-BL
	assoc_ctrl->valid_period = 0;
	assoc_ctrl->sta_list_count = 1;
	os_memcpy(&assoc_ctrl->sta_mac[0], mac_addr, ETH_ALEN);
	ret =  wlanif_issue_wapp_command(global, WAPP_USER_SET_ASSOC_CNTRL_SETTING,
			0, bssid, NULL, (void *)assoc_ctrl,
			sizeof(struct cli_assoc_control) + ETH_ALEN, 0, 0, 0);
	os_free(assoc_ctrl);
	return ret;
}

#ifdef ACL_CTRL
int wlanif_acl_ctrl_for_bss(struct mapd_global *global, u8 *mac_addr,
		u8 *bssid, u8 cmd)
{

	struct acl_ctrl *acl_ctrl = NULL;
	int ret = 0;

	mapd_printf(MSG_DEBUG, "%s cmd:%d STA=(" MACSTR ") BSSID=(" MACSTR ")", __func__,
				 cmd, MAC2STR(mac_addr), MAC2STR(bssid));

	acl_ctrl = (struct acl_ctrl *)
			os_zalloc(sizeof(struct acl_ctrl) + ETH_ALEN);
	acl_ctrl->cmd = cmd;
	os_memcpy(acl_ctrl->bssid, bssid, ETH_ALEN);
	acl_ctrl->sta_list_count = 1;
	os_memcpy(&acl_ctrl->sta_mac[0], mac_addr, ETH_ALEN);
	ret =  wlanif_issue_wapp_command(global, WAPP_USER_SET_ACL_CNTRL_SETTING,
			0, bssid, NULL, (void *)acl_ctrl,
			sizeof(struct acl_ctrl) + ETH_ALEN, 0, 0, 0);
	os_free(acl_ctrl);
	return ret;

}
#endif /*ACL_CTRL*/

static void mapd_receive_from_wapp(int sock, void *eloop_ctx, void *sock_ctx)
{
    struct wapp_usr_intf_ctrl *ctrl = sock_ctx;
    struct mapd_global *global = eloop_ctx;
    mapd_recv_pending_from_wapp(ctrl, global);
}

void wapp_close_connection(void)
{
    if (ctrl_conn == NULL) {
        return;
    }
    eloop_unregister_read_sock(ctrl_conn->s);
    wapp_usr_intf_ctrl_close(ctrl_conn); //Should do detach
    ctrl_conn = NULL;
}

int wapp_open_connection(const char *ctrl_path, struct mapd_global *global)
{
#ifdef SUPPORT_MULTI_AP
    /* For Syncronous commapd-response */
    if (is_1905_present())
        ctrl_conn = wapp_usr_intf_ctrl_open("mapd", ctrl_path);//Should do attach
    else
#endif
        ctrl_conn = wapp_usr_intf_ctrl_open("bs20", ctrl_path);//Should do attach

    if(ctrl_conn == NULL)
        return -1;

    global->wapp_ctrl = ctrl_conn;
    eloop_register_read_sock(ctrl_conn->s, mapd_receive_from_wapp, global, ctrl_conn);
    return 0;
}
int wapp_open_reconnection(struct mapd_global *global)
{
	int ret;
	eloop_unregister_read_sock(ctrl_conn->s);
	if (ctrl_conn->s >= 0)
		close(ctrl_conn->s);
	os_free(ctrl_conn);
	ctrl_conn = NULL;
	ret = wapp_open_connection("/tmp/wapp_ctrl", global);
	while (ret != 0) {
		mapd_printf(MSG_ERROR, "Failed to connect to WAPP");
		/* Sleep for 1 sec */
		os_sleep(1, 0);
		/* Try again */
		ret = wapp_open_connection("/tmp/wapp_ctrl", global);
	}
	wlanif_register_wapp_events(global);
	return ret;
}

static unsigned short event_arr[] = {
#ifdef SUPPORT_MULTI_AP
    WAPP_MAP_BH_READY,
    WAPP_WPS_CONFIG_STATUS,
    WAPP_DISCOVERY,
    WAPP_CLIENT_NOTIFICATION,
    WAPP_AP_OP_BSS,
    WAPP_ASSOC_CLI,
    WAPP_CHN_SEL_RSP,
    WAPP_TOPOQUERY,
    WAPP_CLI_CAPABILITY_REPORT,
    WAPP_1905_CMDU_REQUEST,
    WAPP_1905_READ_BSS_CONF_REQUEST,  /* for wts controller bss_info ready case */
    WAPP_GET_WSC_CONF,
    WAPP_1905_READ_1905_TLV_REQUEST, /* for wts send 1905 data ready case */
    WAPP_OPERATING_CHANNEL_REPORT,
    WAPP_1905_REQ,
    WAPP_AP_LINK_METRIC_REQ,
#endif
    WAPP_OPERATING_CHANNEL_INFO,
#ifdef SUPPORT_MULTI_AP
    WAPP_AP_CAPABLILTY_QUERY,
    WAPP_CLI_CAPABLILTY_QUERY,
    WAPP_CH_SELECTION_REQUEST,
    WAPP_CLI_STEER_REQUEST,
    WAPP_CH_PREFER_QUERY,
    WAPP_POLICY_CONFIG_REQUEST,
#endif
    WAPP_CLI_ASSOC_CNTRL_REQUEST,
#ifdef SUPPORT_MULTI_AP
    WAPP_OP_CHN_RPT,
    WAPP_STA_RSSI,
#endif
    WAPP_STA_STAT,
#ifdef SUPPORT_MULTI_AP
    WAPP_NAC_INFO,
#endif
    WAPP_STA_BSSLOAD,
#ifdef SUPPORT_MULTI_AP
    WAPP_TXPWR_CHANGE,
    WAPP_CSA_INFO,
    WAPP_BSS_STAT_CHANGE,
    WAPP_BSS_LOAD_CROSSING,
    WAPP_APCLI_ASSOC_STAT_CHANGE,
#endif
    WAPP_STA_CNNCT_REJ_INFO,
    WAPP_UPDATE_PROBE_INFO,
#ifdef SUPPORT_MULTI_AP
    WAPP_SCAN_RESULT,
    WAPP_SCAN_DONE,
    WAPP_MAP_VEND_IE_CHANGED,
    WAPP_MAP_BH_CONFIG,
    WAPP_WIRELESS_INF_INFO,
    WAPP_DEVICE_STATUS,
    WAPP_BRIDGE_IP,
    WAPP_SET_BH_TYPE,
    WAPP_CHANNLE_PREFERENCE,
    WAPP_OFF_CH_SCAN_REPORT,
    WAPP_NET_OPT_SCAN_REPORT,
#ifdef AUTOROLE_NEGO
    WAPP_MAP_NEGO_ROLE_RESP,
#endif //AUTOROLE_NEGO
#ifdef MAP_R2
	WAPP_SCAN_CAPAB,
	WAPP_CHANNEL_SCAN_REPORT,
    WAPP_ASSOC_STATUS_NOTIFICATION,
    WAPP_TUNNELED_MESSAGE,
#ifdef DFS_CAC_R2
	WAPP_CAC_CAPAB,
#endif
#endif
	WAPP_CAC_COMPLETION_REPORT,
#ifdef MAP_R2
	WAPP_METRIC_REP_INTERVAL_CAP,
	WAPP_DISASSOC_STATS_EVT,
	WAPP_ALL_ASSOC_STA_EXTENDED_LINK_METRICS,
	WAPP_ONE_ASSOC_STA_EXTENDED_LINK_METRICS,
	WAPP_RADIO_METRICS_INFO,
	WAPP_USER_GET_R2_AP_CAP,
	WAPP_CH_LIST_DFS_INFO,
	WAPP_MBO_STA_PREF_CH_LIST,
#endif
	WAPP_CLI_STEER_BTM_REPORT,
#endif
	WAPP_CAC_PERIOD_ENABLE,
	WAPP_MAP_RESET,
	WAPP_WTS_CONFIG
#ifdef WIFI_MD_COEX_SUPPORT
	,
	WAPP_OPERBSS_REPORT
#endif
#ifdef MAP_R3
	,
	WAPP_SEND_DPP_MSG,
	WAPP_SEND_URI_MSG,
	WAPP_SEND_CCE_MSG,
	WAPP_SEND_1905_CONNECTOR,
	WAPP_SEND_BSS_CONNECTOR,
	WAPP_SEND_CHIRP_MSG,
	WAPP_SEND_DPP_DIRECT_MSG,
	WAPP_SEND_CHIRP_NEW_MSG,
	WAPP_SEND_CONN_FAIL_NOTIF,
	WAPP_MAP_DPP_SAVED_CONFIG,
	WAPP_SEND_USER_FAIL_NOTIF,
	WAPP_SEND_ONBOARD_TYPE
#endif /* MAP_R3 */
	,
	WAPP_SEND_BUFFER_INCR,
	WAPP_RECONFIG_STATUS,
	WAPP_RX_BUFFER_INCR_EVT,
#ifdef MAP_R4_SPT
	WAPP_CH_SELECTION_INFO,
	WAPP_UPLINK_TRAFFIC_STATUS,
#endif
	WAPP_RX_DPP_URI,
	WAPP_MAP_RENEW,
	WAPP_MAP_AGENT_WPS_SUCCESS,
};

static int wapp_send_incr_conf(struct wapp_usr_intf_ctrl *ctrl, char incr_decr)
{
	int ret;

	struct cmd_to_wapp *cmd = NULL;
	char request_buf[50] = {0};


	os_memset(request_buf, 0, sizeof(request_buf));
	cmd = (struct cmd_to_wapp *)request_buf;
	cmd->type = WAPP_SET_SEND_BUFFER_INCR;
	cmd->length = sizeof(incr_decr);
	os_memcpy(cmd->body, &incr_decr, sizeof(incr_decr));

	ret = wapp_usr_intf_ctrl_request(ctrl, request_buf,
					sizeof(struct cmd_to_wapp) + cmd->length);
	if(ret < 0)
		return ret;

	return 0; ;
}

static int wapp_send_buf_incr_evt(struct mapd_global *global, int buf_len)
{
	int ret = -1;
	int len = 0;
	struct cmd_to_wapp *cmd = NULL;
	char request_buf[MAX_BUFF_DYNAMIC_INC_CMD] = {0};
	struct msg *wapp_event = NULL;
	int recv_pkt_len = 0;
	char *status = NULL;

	if(global->wapp_ctrl == NULL) {
                mapd_printf(MSG_ERROR, "wapp_ctrl is NULL, check conenction with WAPP\n");
                return ret;
        }

	len = buf_len;
	os_memset(request_buf, 0, sizeof(request_buf));
	cmd = (struct cmd_to_wapp *)request_buf;
	cmd->type = WAPP_SEND_BUFF_INCR_EVT;
	cmd->length = sizeof(len);
	os_memcpy(cmd->body, &len, sizeof(len));

	ret = wapp_usr_intf_ctrl_request(global->wapp_ctrl, request_buf,
					sizeof(struct cmd_to_wapp) + cmd->length);
	if(ret < 0) {
                mapd_printf(MSG_ERROR, "Sending to wapp failed\n");
		return ret;
	}

	os_memset(request_buf, 0, sizeof(request_buf));
	recv_pkt_len = wapp_wait_recv_parse_wapp_resp(global, request_buf,
                sizeof(request_buf), WAPP_RX_BUFFER_INCR_EVT, 2, 0, 0);
        if (0 >= recv_pkt_len) {
            mapd_printf(MSG_ERROR, "receive waitmsgtype(0x%04x) fail\n", WAPP_RX_BUFFER_INCR_EVT);
            return -1;
        }
	wapp_event = (struct msg *)&request_buf[0];
	status = (char *)wapp_event->buffer;
	if(*status != 1) {
		mapd_printf(MSG_ERROR, "received incorrect status\n");
		return -1;
	}
	global->map_snd_buffer_size = len;

	return 0;
}

#ifdef MAP_R3
int wapp_send_uri_get_evt(struct mapd_global *global, char *reply_buf)
{
	int ret = -1;
	struct cmd_to_wapp *cmd = NULL;
	char request_buf[MAX_BUFF_DYNAMIC_INC_CMD] = {0};
	unsigned char *recv_buf = NULL;
	struct msg *wapp_event = NULL;
	int recv_pkt_len = 0, reply_len = 0;

	if(global->wapp_ctrl == NULL) {
                mapd_printf(MSG_ERROR, "wapp_ctrl is NULL, check conenction with WAPP\n");
                return ret;
        }

	os_memset(request_buf, 0, sizeof(request_buf));
	cmd = (struct cmd_to_wapp *)request_buf;
	cmd->type = WAPP_USER_GET_DPP_URI;

	ret = wapp_usr_intf_ctrl_request(global->wapp_ctrl, request_buf,
					sizeof(struct cmd_to_wapp) + cmd->length);
	if(ret < 0) {
                mapd_printf(MSG_ERROR, "Sending to wapp failed\n");
		return ret;
	}

	if(!global->wapp_map_buffer) {
		err("wapp_map_buffer NULL");
		return -1;
	}
	os_memset(global->wapp_map_buffer, 0, global->wapp_map_buffer_size);
        recv_buf = global->wapp_map_buffer;

	recv_pkt_len = wapp_wait_recv_parse_wapp_resp(global, (char *)recv_buf,
                 global->wapp_map_buffer_size, WAPP_RX_DPP_URI, 2, 0, 0);
	if (recv_pkt_len <= 0) {
		mapd_printf(MSG_ERROR, "receive waitmsgtype(0x%04x)fail\n", WAPP_RX_DPP_URI);
		return -1;
	}
	wapp_event = (struct msg *)&recv_buf[0];
	if (wapp_event->length <= global->wapp_map_buffer_size - 1) {
		os_memcpy(reply_buf, wapp_event->buffer, wapp_event->length);
		reply_len = wapp_event->length;
		reply_buf[reply_len] = '\0';
	}

	mapd_printf(MSG_WARNING, "recevied buf value:%s\n",reply_buf);
	return reply_len;
}
#endif /* MAP_R3 */

static int wapp_register_event(struct wapp_usr_intf_ctrl *ctrl, int reg,
				unsigned short event_id)
{
	int ret;
	size_t len = 10;
	struct timeval tv;
	struct cmd_to_wapp *cmd = NULL;
	char request_buf[3072] = {0};

	if(reg == 0) {
		mapd_printf(MSG_ERROR, "WAPP doesn;t allow to de-register events");
		return -1;
	}

	os_memset(request_buf, 0, sizeof(request_buf));
	cmd = (struct cmd_to_wapp *)request_buf;
	cmd->type = SET_REGITER_EVENT;
	cmd->length = sizeof(event_id);
	os_memcpy(cmd->body, &event_id, sizeof(unsigned short));

	tv.tv_sec = 3;
	tv.tv_usec = 0;
	ret = wapp_usr_intf_ctrl_request(ctrl, request_buf,
					sizeof(struct cmd_to_wapp) + cmd->length);
	if (ret < 0)
		return ret;
	if(wapp_usr_intf_ctrl_pending(ctrl, &tv))
	{
		if(wapp_usr_intf_ctrl_recv(ctrl, request_buf, &len) < 0)
		{
			return -1;
		}
	}
	if (len == 3 && os_memcmp(request_buf, "OK\n", 3) == 0)
		return 0;

	mapd_printf(MSG_ERROR, "Registeration failed, buf : %s", request_buf);
	return -1;
}

void wlanif_register_wapp_events(struct mapd_global *global)
{
    int i = 0;
    for (i = 0; i < sizeof(event_arr)/sizeof(unsigned short); i++)
            wapp_register_event(global->wapp_ctrl, 1, event_arr[i]);
}

void wlanif_trigger_null_frames(struct mapd_global *global, u8 *mac_addr,
				u8 *bssid, u8 count)
{
	mapd_printf(MSG_DEBUG, "STA=" MACSTR " BSSID=" MACSTR " count=%d",
				MAC2STR(mac_addr), MAC2STR(bssid), count);

	if ((is_zero_ether_addr(bssid)) || (is_zero_ether_addr(mac_addr)))
		return;

	wlanif_issue_wapp_command(global, WAPP_USER_SEND_NULL_FRAMES, 0,
					bssid, mac_addr, (void *)&count,
					sizeof(u8), 0, 0, 0);
}

int wapp_get_all_wifi_interface_status(struct mapd_global *global)
{
	char send_to_1905 = 1;

	wlanif_issue_wapp_command(global, WAPP_USER_GET_WIRELESS_INF_INFO,
		WAPP_WIRELESS_INF_INFO, NULL, 0, (void *)&send_to_1905,
		sizeof(send_to_1905), 0, 1, 0);

	return global->wapp_wintf_status;

}

