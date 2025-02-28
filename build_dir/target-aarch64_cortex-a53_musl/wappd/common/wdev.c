/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdlib.h>
#include <stdio.h>
#include "wdev.h"
#include "driver_wext.h"
#include "interface.h"
#include "wps.h"
#include "debug.h"
#ifdef DPP_SUPPORT
#include "dpp/dpp_wdev.h"
#include "gas_query.h"
#include "gas_server.h"
#endif /*DPP_SUPPORT*/

extern unsigned short prev_1905_msg;
struct wapp_dev* wapp_dev_list_lookup_by_radio(struct wifi_app *wapp, char* ra_identifier)
{
	struct wapp_dev *wdev, *target_wdev = NULL;
	unsigned char wdev_identifier[ETH_ALEN];
	struct dl_list *dev_list;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev->radio) {
			MAP_GET_RADIO_IDNFER(wdev->radio, wdev_identifier);
			if(!os_memcmp(wdev_identifier, ra_identifier, ETH_ALEN) &&
				(wdev->dev_type == WAPP_DEV_TYPE_AP)) {
					target_wdev = wdev;
					break;
			}
		}
	}

	return target_wdev;
}

/*
should lookup wdev by addr and wdev type,
due to WDS has the same addr with AP main inf
*/
struct wapp_dev* wapp_dev_list_lookup_by_mac_and_type(struct wifi_app *wapp, const u8 *mac_addr, const u8 wdev_type)
{
	struct wapp_dev *wdev, *target_wdev = NULL;
	struct dl_list *dev_list;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if ((os_memcmp(wdev->mac_addr, mac_addr, MAC_ADDR_LEN) == 0) && wdev->dev_type == wdev_type) {
			target_wdev = wdev;
			break;
		}
	}

	return target_wdev;
}

struct wapp_dev* wapp_dev_list_lookup_by_ifindex(struct wifi_app *wapp, const u32 ifindex)
{
	struct wapp_dev *wdev, *target_wdev = NULL;
	struct dl_list *dev_list;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev->ifindex == ifindex) {
			target_wdev = wdev;
			break;
		}
	}

	return target_wdev;
}

struct wapp_dev* wapp_dev_list_lookup_by_ifname(struct wifi_app *wapp, const char *ifname)
{
	struct wapp_dev *wdev, *target_wdev = NULL;
	struct dl_list *dev_list;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (strcmp(ifname, (char *) wdev->ifname) == 0) {
			target_wdev = wdev;
			break;
		}
	}

	return target_wdev;
}

int wapp_dev_create(
	struct wifi_app *wapp,
	char 			*iface,
	u32				if_idx,
	u8				*mac_addr)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !iface || !mac_addr) {
		//DBGPRINT(RT_DEBUG_OFF, "warning: entry is not NULL!\n");
		return WAPP_INVALID_ARG;
	}
	wdev = os_zalloc(sizeof(struct wapp_dev));

	if (!wdev) {
		//DBGPRINT(RT_DEBUG_OFF, "Creating entry failed!\n");
		return WAPP_RESOURCE_ALLOC_FAIL;
	}

	os_memset(wdev, 0, sizeof(struct wapp_dev));

	dl_list_init(&wdev->list);
	os_memcpy(wdev->mac_addr, mac_addr, MAC_ADDR_LEN);
	os_memcpy(wdev->ifname, iface, IFNAMSIZ);
	wdev->ifindex = if_idx;
#ifdef MAP_R3
	wdev->is_configured = FALSE;
	wdev->scan_done_ind = FALSE;
	wdev->waiting_cce_scan_res = FALSE;
	os_get_time(&wdev->last_sr_time);
#endif /* MAP_R3 */
	dl_list_add_tail(&wapp->dev_list, &wdev->list);

	DBGPRINT(RT_DEBUG_TRACE,
		"new wdev: %s if_idx = %u, mac_addr = "MACSTR"\n",
		wdev->ifname, wdev->ifindex, MAC2STR(wdev->mac_addr));

	wapp_drv_support_version_check(wapp, wdev->ifname);

	wapp_query_wdev(wapp, iface);

	return WAPP_SUCCESS;
}

int wapp_dev_del(
struct wifi_app	*wapp,
struct wapp_dev	*wdev,
const u8		*mac_addr,
const u32		wdev_type)
{
	struct wapp_dev *del_wdev = NULL;
	//struct wdev_ops *ops = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp) {
		return WAPP_INVALID_ARG;
	}

	if (wdev) {
		del_wdev = wdev;
		goto entry_del;
	}

	if (mac_addr) {
		del_wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, mac_addr, wdev_type);
		if (del_wdev)
			goto entry_del;
		else
			return WAPP_LOOKUP_ENTRY_NOT_FOUND;
	}

	/* at least one of sta or mac_addr should not be NULL */
	return WAPP_UNEXP;

entry_del:

	if (del_wdev->p_dev != NULL) {
		if (wdev && wdev->ops->wdev_del)
			wdev->ops->wdev_del(wapp, wdev);
		else
			DBGPRINT_RAW(RT_DEBUG_ERROR, "Error! No wdev_del ops.\n");
		return WAPP_NOT_INITIALIZED;
	}

	dl_list_del(&del_wdev->list);
	os_free(del_wdev);

	return WAPP_SUCCESS;
}

int wapp_clear_dev_list(
struct wifi_app	*wapp)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp) {
		return WAPP_INVALID_ARG;
	}

	if (!dl_list_empty(&wapp->dev_list)) {
		struct wapp_dev *wdev, *wdev_tmp;

		dl_list_for_each_safe(wdev, wdev_tmp,
						&wapp->dev_list, struct wapp_dev, list) {
			wapp_dev_del(wapp, wdev, wdev->mac_addr, wdev->dev_type);
		}
	}

	return WAPP_SUCCESS;
}

int wapp_show_devinfo(struct wapp_dev *wdev)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wdev) {
		return WAPP_INVALID_ARG;
	}

	DBGPRINT_RAW(RT_DEBUG_OFF, "dev_type: %u\n", wdev->dev_type);
	DBGPRINT_RAW(RT_DEBUG_OFF, "ifindex: %u\n", wdev->ifindex);
	DBGPRINT_RAW(RT_DEBUG_OFF, "ifname: %s\n", wdev->ifname);
	if (wdev->radio)
	DBGPRINT_RAW(RT_DEBUG_OFF, "radio index: %u\n", wdev->radio->index);
	DBGPRINT_RAW(RT_DEBUG_OFF, "mac_addr: "MACSTR"\n", MAC2STR(wdev->mac_addr));

#ifdef MAP_SUPPORT
	DBGPRINT_RAW(RT_DEBUG_OFF, "i_am_fh_bss: %d\n", wdev->i_am_fh_bss);
	DBGPRINT_RAW(RT_DEBUG_OFF, "i_am_bh_bss: %d\n", wdev->i_am_bh_bss);
#endif /* MAP_SUPPORT */

#if 0
	DBGPRINT_RAW(RT_DEBUG_OFF, "bssid: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(sta->bssid));
	DBGPRINT_RAW(RT_DEBUG_OFF, "cell_data_cap: %u\n", sta->cell_data_cap);
	DBGPRINT_RAW(RT_DEBUG_OFF, "no_none_pref_ch: %u\n", sta->no_none_pref_ch);
	DBGPRINT_RAW(RT_DEBUG_OFF, "trans_reason: %u\n", sta->trans_reason);
	DBGPRINT_RAW(RT_DEBUG_OFF, "disassoc_imnt: %u\n", sta->disassoc_imnt);
	DBGPRINT_RAW(RT_DEBUG_OFF, "akm: 0x%x\n", sta->akm);
	DBGPRINT_RAW(RT_DEBUG_OFF, "cipher: 0x%x\n", sta->cipher);
	DBGPRINT_RAW(RT_DEBUG_OFF, "non_pref_ch_list: (ch/pref/reason)\n");
	if (!dl_list_empty(&sta->non_pref_ch_list))
		dl_list_for_each(npc_entry, &sta->non_pref_ch_list, struct non_pref_ch_entry, list) {
		DBGPRINT_RAW(RT_DEBUG_OFF, "%u/%u/%u ", npc_entry->npc.ch, npc_entry->npc.pref, npc_entry->npc.reason_code);
	} else
		DBGPRINT_RAW(RT_DEBUG_OFF, "ch list is empty: %u", sta->cell_data_cap);
	DBGPRINT_RAW(RT_DEBUG_OFF, "\n");
#endif
	DBGPRINT_RAW(RT_DEBUG_OFF, "---\n");
	return WAPP_SUCCESS;
}

void wapp_show_dev_list(struct wifi_app *wapp)
{
	struct wapp_dev *wdev_tmp;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	DBGPRINT_RAW(RT_DEBUG_OFF, "wdev_list:\n");
	DBGPRINT_RAW(RT_DEBUG_OFF, "==============================\n");

	if (!dl_list_empty(&wapp->dev_list)){
		dl_list_for_each(wdev_tmp, &wapp->dev_list, struct wapp_dev, list)
			wapp_show_devinfo(wdev_tmp);
	}

	return;
}

void wdev_query_rsp_handle(struct wifi_app *wapp, wapp_event_data *event_data)
{
	wapp_dev_info *dev_info;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dev_info = &event_data->dev_info;

	DBGPRINT_RAW(RT_DEBUG_OFF,
				"wdev_query_rsp_handle: dev_type = %u, mac = "MACSTR", ifname = %s\n",
				dev_info->dev_type,
				MAC2STR(dev_info->mac_addr),
				dev_info->ifname);

	switch (dev_info->dev_type)
	{
		case WAPP_DEV_TYPE_AP:
			wdev_ap_create(wapp, dev_info);
		break;

		case WAPP_DEV_TYPE_STA:
		case WAPP_DEV_TYPE_APCLI:
			wdev_sta_create(wapp, dev_info);
		break;

		default:
		break;
	}
#if defined(DPP_SUPPORT) && !defined(MAP_R3)
	dpp_conf_init(wapp, dev_info);
#endif /* DPP_SUPPORT && !MAP_R3*/
}

void wdev_ht_cap_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_ht_cap *ht_cap_rcvd)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_ht_cap *ht_cap;
		ht_cap = &ap->ht_cap;
		os_memcpy(ht_cap, ht_cap_rcvd, sizeof(wdev_ht_cap));
		DBGPRINT_RAW(RT_DEBUG_OFF,
				"ht_cap (%u):\n"
				"\t tx_stream = %u, rx_stream = %u, sgi_20 = %u, sgi_40 = %u, ht_40 = %u\n",
				ifindex,
				ht_cap->tx_stream,
				ht_cap->rx_stream,
				ht_cap->sgi_20,
				ht_cap->sgi_40,
				ht_cap->ht_40);
	}
}


void wdev_vht_cap_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_vht_cap *vht_cap_rcvd)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_vht_cap *vht_cap;
		vht_cap = &ap->vht_cap;
		os_memcpy(vht_cap, vht_cap_rcvd, sizeof(wdev_vht_cap));
		DBGPRINT_RAW(RT_DEBUG_OFF,
				"vht_cap (%u):\n"
				"\t sup_tx_mcs[]= %02x %02x sup_rx_mcs[] = %u %u\n"
				"\t tx_stream = %u, rx_stream = %u\n"
				"\t sgi_80 = %u, sgi_160 = %u\n"
				"\t vht160 = %u, vht8080 = %u\n"
				"\t su_bf = %u, mu_bf = %u\n",
				ifindex,
				vht_cap->sup_tx_mcs[0], vht_cap->sup_tx_mcs[1],
				vht_cap->sup_rx_mcs[0], vht_cap->sup_rx_mcs[1],
				vht_cap->tx_stream,
				vht_cap->rx_stream,
				vht_cap->sgi_80,
				vht_cap->sgi_160,
				vht_cap->vht_160,
				vht_cap->vht_8080,
				vht_cap->su_bf,
				vht_cap->mu_bf);
	}
}

void wdev_misc_cap_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_misc_cap *misc_cap)
{
	struct wapp_dev *wdev = NULL;
	//wdev_misc_cap *misc_cap = &event_data->misc_cap;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	/* TODO: use wdev hooking funcion? */
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;

		ap->max_num_of_cli = misc_cap->max_num_of_cli;
		ap->max_num_of_bss = misc_cap->max_num_of_bss;
		ap->num_of_bss = misc_cap->num_of_bss;
		ap->max_num_of_block_cli = \
			(misc_cap->max_num_of_block_cli < BLOCK_LIST_NUM) ? misc_cap->max_num_of_block_cli : BLOCK_LIST_NUM;

		if (ap->client_table == NULL) {
			wdev_ap_client_table_create(wapp, ap);
		}

		wdev_ap_block_list_init(wapp, ap);

		DBGPRINT_RAW(RT_DEBUG_OFF,
				"misc_cap (%u):\n"
				"\t max_num_of_cli = %u\n"
				"\t max_num_of_bss = %u\n"
				"\t num_of_bss     = %u\n"
				"\t max_num_of_block_cli = %u\n",
				ifindex,
				ap->max_num_of_cli,
				ap->max_num_of_bss,
				ap->num_of_bss,
				ap->max_num_of_block_cli);
	}
}

void wdev_cli_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_sta *sta = NULL;
	struct ap_dev *ap = NULL;
#ifdef MAP_SUPPORT
	struct wapp_sta temp_sta;
#endif
#ifdef MAP_R3_WF6
	int i = 0;
#endif

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	/* TODO: use wdev hooking funcion? */
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		wapp_client_info *cli = &event_data->cli_info;
		ap = (struct ap_dev *)wdev->p_dev;
		sta = wdev_ap_client_list_lookup(wapp, ap, cli->mac_addr);
		if (!sta && cli->sta_status== WAPP_STA_CONNECTED)
			wapp_client_create(wapp, ap, &sta);

		if (sta) {
#if 1 //MBO
			dl_list_init(&sta->non_pref_ch_list);
#endif
			if (cli->sta_status == WAPP_STA_CONNECTED) {
				if (sta->sta_status != WAPP_STA_CONNECTED) {
					ap->num_of_assoc_cli++;
#ifdef MAP_R2
					wapp_set_mbo_allow_disallow(wapp);
#endif
				}
				wapp_fill_client_info(wapp, cli, sta);
#ifdef MAP_R2
	//			printf("MAP R2 sta ext params\n");
				sta->ext_sta_metrics.sta_info.last_data_dl_rate = cli->ext_metric_info.sta_info.last_data_dl_rate;
				sta->ext_sta_metrics.sta_info.last_data_ul_rate = cli->ext_metric_info.sta_info.last_data_ul_rate;
				sta->ext_sta_metrics.sta_info.utilization_rx = cli->ext_metric_info.sta_info.utilization_rx;
				sta->ext_sta_metrics.sta_info.utilization_tx = cli->ext_metric_info.sta_info.utilization_tx;
				//printf("dl rate: %d, ul rate: %d\n", sta->ext_sta_metrics.sta_info.last_data_dl_rate, sta->ext_sta_metrics.sta_info.last_data_ul_rate);
				//printf("rx rate: %d, tx rate: %d\n", sta->ext_sta_metrics.sta_info.utilization_rx, sta->ext_sta_metrics.sta_info.utilization_tx);
				//printf("dl rate: %d, ul rate: %d\n", cli->ext_metric_info.sta_info.last_data_dl_rate, cli->ext_metric_info.sta_info.last_data_ul_rate);
				//printf("rx rate: %d, tx rate: %d\n", cli->ext_metric_info.sta_info.utilization_rx, cli->ext_metric_info.sta_info.utilization_tx);
#endif
#ifdef MAP_R3_WF6
			sta->tid_cnt = cli->tid_cnt;
			for(i = 0; i < MAX_TID; i++) {
				sta->status_tlv[i].tid = cli->status_tlv[i].tid;
				sta->status_tlv[i].tid_q_size = cli->status_tlv[i].tid_q_size;
				//printf("WF6:WAPP: tid_cnt %x, tid %x, tid_q_size %x\n", cli->tid_cnt, cli->status_tlv[i].tid, cli->status_tlv[i].tid_q_size);
				//printf("WF6:WAPP: cli_info->tx_packets_errors %x\n", cli->tx_packets_errors);
			}
#endif
			}
			else if (cli->sta_status == WAPP_STA_DISCONNECTED) {
				if (sta->sta_status == WAPP_STA_CONNECTED) {
					ap->num_of_assoc_cli--;
#ifdef MAP_R2
					wapp_set_mbo_allow_disallow(wapp);
#endif
				}
				sta->sta_status = WAPP_STA_DISCONNECTED;
				if (sta->beacon_report) {
					free(sta->beacon_report);
					sta->beacon_report = NULL;
				}
			}
#if 0
			DBGPRINT_RAW(RT_DEBUG_OFF, "cli_info: (%u)\n", ifindex);
			wdev_show_wapp_sta_info(sta);
#endif
		}


#ifdef MAP_SUPPORT
		if (!sta) {
			sta = &temp_sta;
			sta->sta_status = WAPP_STA_DISCONNECTED;
		}
		map_send_one_assoc_sta_msg(wapp, sta);
#endif
	}
}


void wdev_cli_list_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	if (!wapp)
		return;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	wdev->cli_info_trigger = FALSE;
	os_get_time(&wdev->cli_list_last_update_time);
#ifdef MAP_SUPPORT
	DBGPRINT_RAW(RT_DEBUG_TRACE, "WAPP_CLI_LIST_QUERY_REQ rsp: %d %s\n", wdev->ifindex, wdev->ifname);
	map_send_assoc_sta_msg(wapp);
#endif
}


void wdev_apcli_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_sta *sta = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		wapp_client_info *cli = &event_data->cli_info;
		sta = (struct wapp_sta *)wdev->p_dev;
		if (sta) {
			wapp_fill_client_info(wapp, cli, sta);
			DBGPRINT_RAW(RT_DEBUG_OFF, "apcli_info: (%u)\n", ifindex);
			wdev_show_wapp_sta_info(sta);
#ifdef MAP_SUPPORT
			map_send_one_assoc_sta_msg(wapp, sta);
#endif
		}
	}
}


void wdev_cli_join_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	unsigned short old_msg;
	struct wapp_sta *sta = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	old_msg = prev_1905_msg;
	/* reset previous 1905 msg type */
	prev_1905_msg = 0;
	wdev_cli_query_rsp_handle(wapp, ifindex, event_data);
#ifdef MAP_SUPPORT
	char buf[MAX_EVT_BUF_LEN];
	wapp_client_info *cli = &event_data->cli_info;
	char assoc_buffer[512] = {0};

	if(!cli)
		return;
	if(!cli->assoc_req_len) {
		DBGPRINT(RT_DEBUG_TRACE, "%s assoc_req_len is 0 \n", __func__);
		return;
	}

	os_memcpy(&assoc_buffer[0], cli->mac_addr, MAC_ADDR_LEN);

	driver_wext_get_assoc_req_frame(wapp->drv_data, wdev->ifname,assoc_buffer,cli->assoc_req_len);

	sta = wdev_ap_client_list_lookup(wapp, (struct ap_dev *)wdev->p_dev, cli->mac_addr);

	if(sta){
		sta->assoc_req_len = cli->assoc_req_len;
		os_memcpy(sta->assoc_req,&assoc_buffer[0],sta->assoc_req_len);
		DBGPRINT(RT_DEBUG_TRACE, "%s:Got Assoc request\n",__func__);
		/*hex_dump_dbg("Assoc Req", (UCHAR *)sta->assoc_req,sta->assoc_req_len);*/
	}


	map_send_assoc_cli_msg(wapp, cli->bssid, cli->mac_addr, STA_JOIN, buf, 0);
#endif
	/* restore previous 1905 msg type */
	prev_1905_msg = old_msg;
#ifdef MAP_R2
	wdev_send_tunnel_assoc_req(wapp, ifindex, cli->assoc_req_len, cli->mac_addr, cli->IsReassoc, (u8 *)assoc_buffer);
#endif
}
void wdev_cli_leave_handle2(struct wifi_app *wapp, u32 ifindex, struct _wapp_event2_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_sta *sta = NULL;
	struct ap_dev *ap = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		wapp_client_info *cli = &event_data->cli_info;
#ifdef MAP_SUPPORT
		char buf[MAX_EVT_BUF_LEN] = {0};

		map_send_assoc_cli_msg(wapp, wdev->mac_addr, cli->mac_addr, STA_LEAVE, buf, cli->disassoc_reason);
#endif
		ap = (struct ap_dev *)wdev->p_dev;
		if (!ap)
			return;
		sta = wdev_ap_client_list_lookup(wapp, (struct ap_dev *) wdev->p_dev, cli->mac_addr);
		if (sta) {
			DBGPRINT(RT_DEBUG_ERROR, "CLI LEAVE MAC "MACSTR"\n", MAC2STR(cli->mac_addr));
			if (sta->sta_status == WAPP_STA_CONNECTED) {
				ap->num_of_assoc_cli--;
#ifdef MAP_R2
				wapp_set_mbo_allow_disallow(wapp);
#endif
				sta->sta_status = WAPP_STA_DISCONNECTED;
				if (sta->beacon_report) {
					free(sta->beacon_report);
					sta->beacon_report = NULL;
				}
			}
		}
	}
}

void wdev_cli_leave_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_sta *sta = NULL;
	struct ap_dev *ap = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	/* TODO: use wdev hooking funcion? */
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		wapp_client_info *cli = &event_data->cli_info;
#ifdef MAP_SUPPORT
		char buf[MAX_EVT_BUF_LEN];
		map_send_assoc_cli_msg(wapp, wdev->mac_addr, cli->mac_addr, STA_LEAVE, buf, cli->disassoc_reason);
#endif
		ap = (struct ap_dev *)wdev->p_dev;
		sta = wdev_ap_client_list_lookup(wapp, (struct ap_dev *) wdev->p_dev, cli->mac_addr);
		if (sta) {
			DBGPRINT(RT_DEBUG_ERROR, " CLI LEAVE MAC "MACSTR"\n", MAC2STR(cli->mac_addr));
			if (sta->sta_status == WAPP_STA_CONNECTED) {
				ap->num_of_assoc_cli--;
#ifdef MAP_R2
				wapp_set_mbo_allow_disallow(wapp);
#endif
				sta->sta_status = WAPP_STA_DISCONNECTED;
				if (sta->beacon_report) {
					free(sta->beacon_report);
					sta->beacon_report = NULL;
				}
			}
		}
	}
}
#ifdef MAP_R2
int mbo_set_assoc_disallow( struct wifi_app *wapp, char *iface, int value)
{
	struct mbo_cfg *mbo = wapp->mbo;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	mbo->assoc_disallow_reason = value;

	/* mbo assoc disallow */
	mbo_param_setting(wapp, (const char *)iface, PARAM_MBO_AP_ASSOC_DISALLOW, wapp->mbo->assoc_disallow_reason);

#ifdef MAP_R2
	/* Send assoc status notification to controller/agents */
	map_send_assoc_notification(wapp, (const char *)iface, wapp->mbo->assoc_disallow_reason);
#endif

	return MBO_SUCCESS;
}

void wdev_sta_disassoc_stats_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	/* TODO: use wdev hooking funcion? */
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		wapp_client_info *cli = &event_data->cli_info;
		printf("event cli info: %d\n", event_data->cli_info.disassoc_reason);
		char buf[MAX_EVT_BUF_LEN];
		map_send_sta_disassoc_stats_msg(wapp, wdev->mac_addr, cli, buf);
	}
}
void wapp_set_mbo_allow_disallow(struct wifi_app *wapp)
{
	int tot_cnt = 0;
	struct ap_dev * ap = NULL;
	struct dl_list *dev_list = NULL;
	struct wapp_dev *wdev = NULL;

	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
			ap = (struct ap_dev *)wdev->p_dev;
			tot_cnt += ap->num_of_assoc_cli;
		}
	}
	if (tot_cnt >= wapp->map->max_client_cnt && wapp->mbo->assoc_disallow_reason == 0) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: MAX no of client cnt reached disallowing\n", __func__);
		wdev = NULL;
		dev_list = &wapp->dev_list;
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
			if (wdev->dev_type == WAPP_DEV_TYPE_AP)
				mbo_set_assoc_disallow(wapp, wdev->ifname, 2);
		}
	} else if (tot_cnt <= wapp->map->max_client_cnt && wapp->mbo->assoc_disallow_reason == 2) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: Reduced no of client, allowing\n", __func__);
		wdev = NULL;
		dev_list = &wapp->dev_list;
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
			if (wdev->dev_type == WAPP_DEV_TYPE_AP)
				mbo_set_assoc_disallow(wapp, wdev->ifname, 0);
		}
	}
}
#endif
void wdev_cli_probe_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct probe_info *info;
	wapp_probe_info *probe = &event_data->probe_info;
	int send_pkt_len = 0;
	char* buf = NULL;

	/* TODO varify conf status for controller */
	if ((wapp->is_bs20_attached == FALSE) &&
	    (wapp->map->conf != MAP_CONN_STATUS_CONF)) {
		DBGPRINT(RT_DEBUG_TRACE, "%s: device is unconfigued, ignore probes\n", __func__);
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	info = wapp_probe_lookup(wapp, probe->mac_addr);
	if (info == NULL) {
		info = wapp_probe_create(wapp, probe->mac_addr);
	}

	//Update probe info in wapp
	info->rssi = probe->rssi;
	info->channel = probe->channel;
	os_memcpy(info->preq, probe->preq, probe->preq_len);
	os_get_time(&info->last_update_time);

	//Send to other daemon
	send_pkt_len = sizeof(struct probe);
	buf = os_zalloc(send_pkt_len);
	if (buf) {
		DBGPRINT(RT_DEBUG_TRACE, "Sending %s\n", __func__);
		os_memcpy(buf, probe, send_pkt_len);
		wapp_send_1905_msg(wapp, WAPP_UPDATE_PROBE_INFO, send_pkt_len, buf);
		os_free(buf);
	}
}

void wdev_chn_list_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_chn_info *chn_list_rcvd)
{
	struct wapp_dev *wdev = NULL;
	int i = 0;
	int j = 0;
	int k = 0;
	int skip_add = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_chn_info *chn_list;
		chn_list = &ap->ch_info;
		os_memcpy(chn_list, chn_list_rcvd, sizeof(wdev_chn_info));
#ifdef MAP_6E_SUPPORT
		wdev->radio->opclass = chn_list->op_class;
#endif
		if (ap->isActive)
			wapp_radio_update_ch(wapp, wdev->radio, chn_list->op_ch);
		for(j = wapp->AutoChannelSkipListNum, i = 0;
				i < chn_list->AutoChannelSkipListNum;
				i++) {

			/*Checking duplicate in skiplist.*/
			for(k = 0; k < j; k++) {
				if(chn_list->AutoChannelSkipList[i] == wapp->AutoChannelSkipList[k]) {
					skip_add = 1;
					break;
				}
			}
			/* channel needs to be added in the list. */
			if(!skip_add) {
				wapp->AutoChannelSkipList[j++] = chn_list->AutoChannelSkipList[i];

				DBGPRINT_RAW(RT_DEBUG_OFF,
						"WAPP: at boot time we have rcv'd skipList[%d]=%d\n",
						j-1, chn_list->AutoChannelSkipList[i]);
			}
			else /* Skip this channel as it is duplicate. */
				skip_add = 0;

		}
		wapp->AutoChannelSkipListNum = j;

		DBGPRINT_RAW(RT_DEBUG_OFF,
				"chn_list: (%u)\n"
				"\t op_class = %u\n"
				"\t op_ch = %u\n"
				"\t band = %u \n"
				"\t ch_list_num = %u\n"
				"\t non_op_chn_num = %d\n"
				"\t dl_mcs = %u\n",
				ifindex,
				chn_list->op_class,
				chn_list->op_ch,
				chn_list->band,
				chn_list->ch_list_num,
				chn_list->non_op_chn_num,
				chn_list->dl_mcs);
	}
}

void wdev_op_class_query_rsp_handle(struct wifi_app *wapp, u32 ifindex,
#ifndef MAP_6E_SUPPORT
	wdev_op_class_info * op_class_rcvd
#else
	struct _wdev_op_class_info_ext *op_class_rcvd
#endif
)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
#ifdef MAP_6E_SUPPORT
		struct _wdev_op_class_info_ext *op_class = &ap->op_class;

		os_memcpy(op_class, op_class_rcvd, sizeof(struct _wdev_op_class_info_ext));
#else
		wdev_op_class_info *op_class = &ap->op_class;
		os_memcpy(op_class, op_class_rcvd, sizeof(wdev_op_class_info));
#endif


		DBGPRINT_RAW(RT_DEBUG_OFF,
				"op_class: (%u)\n"
				"\t num_of_op_class = %u \n",
				ifindex,
				op_class->num_of_op_class);

		int i = 0;
		for(i = 0; i < op_class->num_of_op_class; i++) {
#ifdef MAP_6E_SUPPORT
			printf("\t opClass = %u, chn_num = %u\n", op_class->opClassInfoExt[i].op_class,
				op_class->opClassInfoExt[i].num_of_ch);
#else
			printf("\t opClass = %u, chn_num = %u\n", op_class->opClassInfo[i].op_class,
				op_class->opClassInfo[i].num_of_ch);
#endif
		}
	}
}

void wapp_on_boot_scan(void *eloop_data, void *user_ctx);

void wdev_bss_info_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_bss_info *bss_info_rcvd)
{
	struct wapp_dev *wdev = NULL;
	u8 idx = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_bss_info *bss_info;
		bss_info = os_zalloc(sizeof(wdev_bss_info));

		if (!bss_info) {
			return;
		}
		os_memcpy(bss_info, bss_info_rcvd, sizeof(wdev_bss_info));

		DBGPRINT_RAW(RT_DEBUG_OFF,
				"bss_info: (%u)\n"
				"\t table idx = %u\n"
				"\t ssid = %s\n"
				"\t SsidLen = %u\n"
				"\t map role = %u\n"
				"\t Bssid = "MACSTR"\n"
				"\t Identifier = "MACSTR"\n",
				ifindex,
				wapp->map->bss_tbl_idx,
				bss_info->ssid,
				bss_info->SsidLen,
				bss_info->map_role,
				MAC2STR(bss_info->bssid),
				MAC2STR(bss_info->if_addr));

		os_memcpy(&ap->bss_info, bss_info, sizeof(wdev_bss_info));
		idx = wapp->map->bss_tbl_idx;
		wapp->map->op_bss_table[idx] = *bss_info;
		wapp->map->bss_tbl_idx++;

		os_free(bss_info);
#ifdef MAP_R2
		if(wapp->map->MapMode == 4){// perform onboot scan only for certification
			if(eloop_is_timeout_registered(wapp_on_boot_scan, wapp,NULL))
				eloop_cancel_timeout(wapp_on_boot_scan, wapp,NULL);
			eloop_register_timeout(15,0,wapp_on_boot_scan, wapp, NULL);
		}
#endif
	}
}


void wdev_ap_metric_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_ap_metric *ap_metrics_rcvd)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_ap_metric *ap_metrics;
		ap_metrics = &ap->ap_metrics;
		os_memcpy(ap_metrics, ap_metrics_rcvd, sizeof(wdev_ap_metric));
		DBGPRINT_RAW(RT_DEBUG_TRACE, "Ap_metric: (%u)\n", ifindex);
		wdev_ap_show_ap_metric(wapp, ap);
#ifdef MAP_SUPPORT
		map_send_ap_metric_msg(wapp, ap);
#endif
	}
}
#ifdef MAP_R2
void wdev_radio_metric_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
		map_send_radio_metric_msg(wapp, event_data, ifindex);
}
#endif

void wdev_ch_util_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_radio *ra = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		DBGPRINT(RT_DEBUG_INFO, "WAPP got WAPP_CH_UTIL_QUERY_RSP, ch_util: %d\n", event_data->ch_util);
		ra = wdev->radio;
		ra->metric_policy.ch_util_prev = ra->metric_policy.ch_util_current;
		ra->metric_policy.ch_util_current = event_data->ch_util;
	}
}

void wdev_ap_config_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		wapp->map->rssi_steer = event_data->ap_conf.rssi_steer;
		wapp->map->sta_report_not_cop = event_data->ap_conf.sta_report_not_cop;
		wapp->map->sta_report_on_cop = event_data->ap_conf.sta_report_on_cop;

		DBGPRINT_RAW(RT_DEBUG_OFF,
				"Ap_conf: (%u)\n"
				"\t rssi_steer = %u\n"
				"\t sta_report_not_cop = %u\n"
				"\t sta_report_on_cop = %u\n",
				ifindex,
				wapp->map->rssi_steer,
				wapp->map->sta_report_not_cop,
				wapp->map->sta_report_on_cop);
	}
}

void wdev_bcn_report_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct beacon_metrics_rsp *bcn_rpt = NULL;
	struct wapp_sta *sta = NULL;
	struct ap_dev *ap = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {

		ap = (struct ap_dev *)wdev->p_dev;
		sta = wdev_ap_client_list_lookup(wapp, ap, event_data->bcn_rpt_info.sta_addr);
		if (!sta || sta->sta_status != WAPP_STA_CONNECTED)
			return;
		if (sta->beacon_report == NULL) {
			sta->beacon_report = (struct beacon_metrics_rsp *)malloc(sizeof(struct beacon_metrics_rsp) + MAX_BEACON_REPORT_LEN);
			os_memset(sta->beacon_report, 0, sizeof(*sta->beacon_report));
			os_memcpy(sta->beacon_report->sta_mac, event_data->bcn_rpt_info.sta_addr, MAC_ADDR_LEN);
		}
		bcn_rpt = sta->beacon_report;
		if ((bcn_rpt->rpt_len + event_data->bcn_rpt_info.bcn_rpt_len) > MAX_BEACON_REPORT_LEN)
			return;
		if(event_data->bcn_rpt_info.last_fragment)
			bcn_rpt->bcn_rpt_num++;
		os_memcpy(&bcn_rpt->rpt[bcn_rpt->rpt_len],
			event_data->bcn_rpt_info.bcn_rpt, event_data->bcn_rpt_info.bcn_rpt_len);
		bcn_rpt->rpt_len += event_data->bcn_rpt_info.bcn_rpt_len;

		DBGPRINT_RAW(RT_DEBUG_OFF,
			" get bcn rpt from "MACSTR
			" len: %d"
			" accumulate bcn rpt num: %d\n"
			" accumulate bcn rpt len: %d\n",
			MAC2STR(event_data->bcn_rpt_info.sta_addr),
			event_data->bcn_rpt_info.bcn_rpt_len,
			bcn_rpt->bcn_rpt_num,
			bcn_rpt->rpt_len);
	}
}


void wdev_bcn_report_complete_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
#if defined(MAP_SUPPORT)
	char buf[MAX_EVT_BUF_LEN];
	int send_pkt_len = 0;
#endif

	struct wapp_dev *wdev = NULL;
	struct wapp_sta *sta = NULL;
	struct ap_dev *ap = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		ap = (struct ap_dev *)wdev->p_dev;
		sta = wdev_ap_client_list_lookup(wapp, ap, event_data->bcn_rpt_info.sta_addr);
		if (!sta || sta->sta_status != WAPP_STA_CONNECTED)
			return;
		if (sta->beacon_report) {
			DBGPRINT_RAW(RT_DEBUG_OFF,
				" get bcn rpt complete from "MACSTR"\n",
				MAC2STR(event_data->bcn_rpt_info.sta_addr));
#if defined(MAP_SUPPORT)
			send_pkt_len = sizeof(struct beacon_metrics_rsp) + sizeof(unsigned char) * sta->beacon_report->rpt_len;
			os_memcpy(buf, sta->beacon_report, send_pkt_len);
			wapp_send_1905_msg(wapp, WAPP_BEACON_METRICS_REPORT, send_pkt_len, buf);
#endif
			free(sta->beacon_report);
			sta->beacon_report = NULL;
		}
	}
}

void wdev_bssload_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct bssload_info bssload_info;
	int send_pkt_len = 0;
	char* buf = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		os_memcpy(&ap->bssload, &event_data->bssload_info, sizeof(wapp_bssload_info));

		DBGPRINT_RAW(RT_DEBUG_OFF,
			" sta_cnt: %d"
			" ch_util: %d\n"
			" AvalAdmCap: %x\n",
			event_data->bssload_info.sta_cnt,
			event_data->bssload_info.ch_util,
			event_data->bssload_info.AvalAdmCap);

		os_memcpy(&bssload_info, &ap->bssload, sizeof(struct bssload_info));
		send_pkt_len = sizeof(struct bssload_info);
		buf = os_zalloc(send_pkt_len);
		if (buf) {
			os_memcpy(buf, &bssload_info, send_pkt_len);
			wapp_send_1905_msg(wapp, WAPP_STA_BSSLOAD, send_pkt_len, buf);
			os_free(buf);
		}
	}
}

void wdev_mnt_info_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char* buf = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp)
		return;

#ifdef MAP_SUPPORT
	if (!dl_list_empty(&wapp->sta_mntr_list)){
		air_monitor_packet_handle(wapp, ifindex, event_data);
		return;
	}
#endif

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wapp_mnt_info *mnt_info;

		mnt_info = &ap->mnt;
		os_memcpy(mnt_info, &event_data->mnt_info, sizeof(wapp_mnt_info));

		send_pkt_len = sizeof(wapp_mnt_info);
		buf = os_zalloc(send_pkt_len);
		if (buf) {
			os_memcpy(buf, mnt_info, send_pkt_len);
			wapp_send_1905_msg(wapp, WAPP_NAC_INFO, send_pkt_len, buf);
			os_free(buf);
		}
	}
}

void wdev_sta_rssi_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	wapp_client_info *cli = NULL;
	struct wapp_sta* sta = NULL;
	int send_pkt_len = 0;
	char* buf = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		cli = &event_data->cli_info;

		sta = wdev_ap_client_list_lookup_for_all_bss(wapp, cli->mac_addr);
		if (sta) {
			sta->uplink_rssi = cli->uplink_rssi;
			DBGPRINT_RAW(RT_DEBUG_OFF,
						"sta->uplink_rssi: %d", sta->uplink_rssi);

			send_pkt_len = MAC_ADDR_LEN + sizeof(u8);
			buf = os_zalloc(send_pkt_len);
			if (buf) {
				os_memcpy(buf, sta->mac_addr, MAC_ADDR_LEN);
				os_memcpy(buf + MAC_ADDR_LEN, &sta->uplink_rssi, sizeof(u8));
				wapp_send_1905_msg(wapp, WAPP_STA_RSSI, send_pkt_len, buf);
				os_free(buf);
			}
		}
	}
}

void wdev_cli_active_change_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	wapp_client_info *cli = NULL;
	struct wapp_sta* sta = NULL;
	struct ap_dev *ap = NULL;
	int send_pkt_len = 0;
	wapp_sta_status_info *sta_status = NULL;
	DBGPRINT(RT_DEBUG_TRACE, RED("%s \n"), __func__);

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		cli = &event_data->cli_info;
		ap = (struct ap_dev *)wdev->p_dev;
		sta = wdev_ap_client_list_lookup(wapp, ap, cli->mac_addr);

		if (sta) {
			sta->stat = cli->status;
			DBGPRINT_RAW(RT_DEBUG_OFF, RED("sta->stat: %d"), sta->stat);

			send_pkt_len = sizeof(wapp_sta_status_info);
			sta_status = (wapp_sta_status_info *)os_zalloc(send_pkt_len);
			if (sta_status) {
				os_memcpy(sta_status->sta_mac, sta->mac_addr, MAC_ADDR_LEN);
				sta_status->status = sta->stat;
				wapp_send_1905_msg(wapp, WAPP_STA_STAT, send_pkt_len, (char *)sta_status);
				os_free(sta_status);
			}
		} else
			DBGPRINT(RT_DEBUG_WARN, RED("Null sta\n"));
	}
}

void wdev_chn_change_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_dev *wdev_temp = NULL;
	struct dl_list *dev_list = NULL;
	u8 new_ch = 0;
	u8 op_class = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	dev_list = &wapp->dev_list;
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_chn_info *chn_list;

		chn_list = &ap->ch_info;
		chn_list->op_ch = event_data->ch_change_info.new_ch;
		chn_list->op_class = event_data->ch_change_info.op_class;
		printf("wdev_chn_change_rsp_handle: ch: %u op_class: %u\n", ap->ch_info.op_ch, ap->ch_info.op_class);
		if (wdev->radio) {
			wdev->radio->op_ch = chn_list->op_ch;
#ifdef MAP_6E_SUPPORT
			wdev->radio->opclass = chn_list->op_class;
#endif

		}

		dl_list_for_each(wdev_temp, dev_list, struct wapp_dev, list) {
			if (wdev->radio == wdev_temp->radio && wdev_temp->dev_type == WAPP_DEV_TYPE_AP) {
				struct ap_dev *ap = (struct ap_dev *)wdev_temp->p_dev;

				if (ap) {
					ap->ch_info.op_ch = event_data->ch_change_info.new_ch;
					ap->ch_info.op_class = event_data->ch_change_info.op_class;
				}
			}
		}
	} else if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		new_ch = event_data->ch_change_info.new_ch;
		op_class = event_data->ch_change_info.op_class;
#ifdef MAP_6E_SUPPORT
		wdev->radio->opclass = op_class;
#endif
		wapp_radio_update_ch(wapp, wdev->radio, new_ch);
		dl_list_for_each(wdev_temp, dev_list, struct wapp_dev, list) {
			if (wdev->radio == wdev_temp->radio) {
				if (wdev_temp->dev_type == WAPP_DEV_TYPE_AP) {
					struct ap_dev * ap = (struct ap_dev *)wdev_temp->p_dev;
					ap->ch_info.op_ch = new_ch;
					 ap->ch_info.op_class = op_class;
				}
			}
		}
	}
}

void wdev_csa_event_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct dl_list *dev_list;
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char* buf = NULL;
	u8 ruid[MAC_ADDR_LEN];
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		MAP_GET_RADIO_IDNFER(wdev->radio, ruid);

		if (wdev->radio->op_ch != event_data->csa_info.new_channel) {
			wdev_chn_info *chn_list;
			struct csa_info_rsp csa_info;
#ifdef MAP_6E_SUPPORT
			/* ToDo: Send opclass from driver */
			/* wdev->radio->opclass = event_data->csa_info.new_opclass; */
#endif
			wapp_radio_update_ch(wapp, wdev->radio, event_data->csa_info.new_channel);

			//update op_ch of ap wdev under this ruid
			dev_list = &wapp->dev_list;
			dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
				if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
					u8 wdev_identifier[MAC_ADDR_LEN];
					MAP_GET_RADIO_IDNFER(wdev->radio, wdev_identifier);
					if (!os_memcmp(wdev_identifier, ruid, MAC_ADDR_LEN)) {
						struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;

						ap->ch_info.op_ch = event_data->csa_info.new_channel;
						chn_list = &ap->ch_info;
						chn_list->op_ch = event_data->csa_info.new_channel;
					}
				}
			}

			os_memcpy(csa_info.ruid, ruid, MAC_ADDR_LEN);
			csa_info.new_ch = event_data->csa_info.new_channel;
			send_pkt_len = sizeof(struct csa_info_rsp);
			buf = os_zalloc(send_pkt_len);
			if (buf) {
				os_memcpy(buf, &csa_info, send_pkt_len);
				wapp_send_1905_msg(wapp, WAPP_CSA_INFO, send_pkt_len, buf);
				os_free(buf);
			}
		}
	}
	wapp->csa_new_channel = event_data->csa_info.new_channel;
	if(wapp->map->quick_ch_change == TRUE)
		wapp->cli_assoc_info.current_channel = event_data->csa_info.new_channel;
	map_operating_channel_info(wapp);
}

void wdev_apcli_rssi_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char* buf = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		wapp_apcli_association_info *apcli_info = &event_data->apcli_association_info;

		send_pkt_len = sizeof(char);
		buf = os_zalloc(send_pkt_len);
		if (buf) {
			os_memcpy(buf, &apcli_info->rssi, send_pkt_len);
			wapp_send_1905_msg(wapp, WAPP_APCLI_UPLINK_RSSI, send_pkt_len, buf);
			os_free(buf);
		}
	}
}

#ifdef WPS_UNCONFIG_FEATURE_SUPPORT
int map_wps_conf_done_send_map_renew(struct wifi_app *wapp)
{
	struct evt *map_event = NULL;
	char buf[50] = {0};
	u16 evt_len = 0;

	map_event = (struct evt *)buf;
	map_event->type   = WAPP_MAP_RENEW;
	map_event->length = 0;
	evt_len = sizeof(struct evt);

	DBGPRINT(RT_DEBUG_ERROR, "\nwapp :: WPS Configuration done, sending mapd renew to sync all\n");
	if (map_1905_send(wapp, buf, evt_len) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Error send map renew event\n");
		return -1;
	}
	memset(buf, 0, evt_len);
	return 0;
}
#define MAX_WTS_AP_CONF_LENGTH 200
void wdev_wps_configuration_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	char *buf = NULL, *file = NULL, *index_ptr = NULL, *next_to_index_ptr = NULL;
	int ch = 0, index_2G = 0, index_5GL = 0, index_5GH = 0;
	char index = 0, temp[10], str[200];
	FILE *f;
	int i = 0, size = 0, res = 0, ret;
	struct set_config_bss_info bss_config = {0};
	u8 radio_num = 0;

	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		if (wapp->radio[i].adpt_id)
			radio_num++;
	}

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	i = 0;
	buf = os_zalloc(sizeof(struct set_config_bss_info) * MAX_SET_BSS_INFO_NUM);
	if (buf == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, "\nwdev_wps_configuration_handle, buf memory allocation fail");
		return;
	}

	file = os_zalloc(sizeof(struct set_config_bss_info) * MAX_SET_BSS_INFO_NUM);
	if (file == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, "\nwdev_wps_configuration_handle, file memory allocation fail");
		os_free(buf);
		return;
	}

	DBGPRINT(RT_DEBUG_ERROR, "\nwdev_wps_configuration_handle");
	DBGPRINT(RT_DEBUG_ERROR, "\nSSID = %s, key = %s, len = %d", event_data->wps_conf_info.SSID,
					event_data->wps_conf_info.Key, event_data->wps_conf_info.KeyLength);
	DBGPRINT(RT_DEBUG_ERROR, "\nchannel = %d,", event_data->wps_conf_info.channel);
	DBGPRINT(RT_DEBUG_ERROR, "\nindex = %d,", event_data->wps_conf_info.index);
	DBGPRINT(RT_DEBUG_ERROR, "\nAuth = %d, EncyType = %d, Bss_role = %d", event_data->wps_conf_info.AuthType,
				event_data->wps_conf_info.EncrType, event_data->wps_conf_info.bss_role);
	DBGPRINT(RT_DEBUG_ERROR, "\nMAC :: 02x%x:02x%x:02x%x:02x%x:02x%x:02x%x\n", event_data->wps_conf_info.MacAddr[0],
				event_data->wps_conf_info.MacAddr[1], event_data->wps_conf_info.MacAddr[2],
				event_data->wps_conf_info.MacAddr[3], event_data->wps_conf_info.MacAddr[4],
				event_data->wps_conf_info.MacAddr[5]);

	NdisCopyMemory(bss_config.ssid, event_data->wps_conf_info.SSID, sizeof(event_data->wps_conf_info.SSID));
	NdisCopyMemory(bss_config.key, event_data->wps_conf_info.Key, event_data->wps_conf_info.KeyLength);
	bss_config.authmode   = event_data->wps_conf_info.AuthType;
	bss_config.encryptype = event_data->wps_conf_info.EncrType;
	index = event_data->wps_conf_info.index;

	/* Read wts_bss_info_config file*/
	f = fopen("/etc/map/wts_bss_info_config", "r");
	if (f == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, "\nfile opening error\n");
		goto error;
	}

	while (1) {
		ch = fgetc(f);
		if (ferror(f) != 0) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] fgetc error\n", __func__);
			clearerr(f);
			goto error;
		}
		if (ch == EOF)
			break;
		file[i++] = ch;
	}
	ret = fseek(f, 0, SEEK_SET);
	if (ret != 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] fseek fail\n", __func__);
		goto error;
	}

	while (!feof(f)) {
		if (fgets(str, 200, f) == NULL)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): fgets() return value NULL\n", __func__, __LINE__);
		if (strstr(str, "8x") && !index_2G) {
			memmove(temp, str, 2);
			index_2G = atoi(temp);
		} else if (strstr(str, "11x") && !index_5GL) {
			memmove(temp, str, 2);
			index_5GL = atoi(temp);
		} else if (strstr(str, "12x") && !index_5GH) {
			memmove(temp, str, 2);
			index_5GH = atoi(temp);
		} else
			continue;
	}
	ret = fclose(f);
	if (ret != 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] fclose fail\n", __func__);
		os_free(file);
		os_free(buf);
		return;
	}
	f = NULL;

	/* update the index with new configuration and copy to buf for file write*/
	if (IS_MAP_CH_24G(event_data->wps_conf_info.channel)) {
		index += index_2G;
		if (index == index_2G) {
		res = os_snprintf(buf, MAX_WTS_AP_CONF_LENGTH, "%d,ff:ff:ff:ff:ff:ff 8x %s 0x00%x 0x000%x %s 1 0 hidden-N 4095 N/A N/A\n",
					index, bss_config.ssid, bss_config.authmode, bss_config.encryptype, event_data->wps_conf_info.Key);
		} else {
		res = os_snprintf(buf, MAX_WTS_AP_CONF_LENGTH, "%d,ff:ff:ff:ff:ff:ff 8x %s 0x00%x 0x000%x %s 0 1 hidden-N 4095 N/A N/A\n",
				index, bss_config.ssid, bss_config.authmode, bss_config.encryptype, event_data->wps_conf_info.Key);
		}

		if (os_snprintf_error(MAX_WTS_AP_CONF_LENGTH, res)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%d] Unexpected snprintf fail\n", __LINE__);
			goto error;
		}
	} else if ((radio_num >= 3) && (IS_MAP_CH_5GH(event_data->wps_conf_info.channel))) {
		index += index_5GH;
		if (index == index_5GH) {
		res = os_snprintf(buf, MAX_WTS_AP_CONF_LENGTH, "%d,ff:ff:ff:ff:ff:ff 12x %s 0x00%x 0x000%x %s 1 0 hidden-N 4095 N/A N/A\n",
			index, bss_config.ssid, bss_config.authmode, bss_config.encryptype, event_data->wps_conf_info.Key);
		} else {
		res = os_snprintf(buf, MAX_WTS_AP_CONF_LENGTH, "%d,ff:ff:ff:ff:ff:ff 12x %s 0x00%x 0x000%x %s 0 1 hidden-N 4095 N/A N/A\n",
				index, bss_config.ssid, bss_config.authmode, bss_config.encryptype, event_data->wps_conf_info.Key);
		}

		if (os_snprintf_error(MAX_WTS_AP_CONF_LENGTH, res)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%d] Unexpected snprintf fail\n", __LINE__);
				goto error;
		}
	} else {
		index += index_5GL;
		if (index == index_5GL) {
		res = os_snprintf(buf, MAX_WTS_AP_CONF_LENGTH, "%d,ff:ff:ff:ff:ff:ff 11x %s 0x00%x 0x000%x %s 1 0 hidden-N 4095 pvid 5\n",
				index, bss_config.ssid, bss_config.authmode, bss_config.encryptype, event_data->wps_conf_info.Key);
		} else {
		res = os_snprintf(buf, MAX_WTS_AP_CONF_LENGTH, "%d,ff:ff:ff:ff:ff:ff 11x %s 0x00%x 0x000%x %s 0 1 hidden-N 4095 N/A N/A\n",
				index, bss_config.ssid, bss_config.authmode, bss_config.encryptype, event_data->wps_conf_info.Key);
		}

		if (os_snprintf_error(MAX_WTS_AP_CONF_LENGTH, res)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%d] Unexpected snprintf fail\n", __LINE__);
			goto error;
		}
	}

	DBGPRINT(RT_DEBUG_ERROR, "\n%s", buf);

	memset(temp, 0, sizeof(temp));
	sprintf(temp, "%d,", index);
	index_ptr = strstr(file, temp);
	if (index_ptr == NULL)
		goto error;
	sprintf(temp, "%d,", index+1);
	next_to_index_ptr = strstr(file, temp);

	size = strlen(buf);
	/*  contents after the index, copy to buf*/
	if (next_to_index_ptr == NULL)
		goto error;

	memmove(buf+size, next_to_index_ptr, strlen(next_to_index_ptr));
	/* reset wts file contents from the index*/
	memset(index_ptr, 0, strlen(index_ptr));
	/* copy the updated buf with the new configuration to wts file.*/
	memmove(index_ptr, buf, strlen(buf));
	DBGPRINT(RT_DEBUG_ERROR, "\n%s", file);

	/* open wts file in write mode to write with updated configuration*/
	f = fopen("/etc/map/wts_bss_info_config", "w+");
	if (f == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, "\nfile opening error\n");
		goto error;
	}
	size = 0;
	size = fwrite(file, 1, strlen(file), f);
	if (strlen(file) != size) {
		DBGPRINT(RT_DEBUG_ERROR, "write error");
		goto error;
	}

	/* wts file updated, issue the mapd renew command to mapd*/
	map_wps_conf_done_send_map_renew(wapp);
error:
	os_free(file);
	os_free(buf);
	if (f != NULL) {
		ret = fclose(f);
		if (ret != 0)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] [%d]-->fclose fail\n", __func__, __LINE__);
	}
	return;

}
#endif
void wdev_bss_stat_change_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char *buf = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		ap->isActive = event_data->bss_state_info.bss_state;

		//Send to other daemon
		struct bss_state_info bss_stat_info;
		bss_stat_info.bss_state = event_data->bss_state_info.bss_state;
		bss_stat_info.interface_index = event_data->bss_state_info.interface_index;
		send_pkt_len = sizeof(struct bss_state_info);
		os_alloc_mem(NULL, (UCHAR **)&buf, send_pkt_len);
		if (buf) {
			os_memcpy(buf, &bss_stat_info, send_pkt_len);
			wapp_send_1905_msg(wapp, WAPP_BSS_STAT_CHANGE, send_pkt_len, buf);
			os_free(buf);
		}
	}
}

void wdev_bssload_crossing_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char* buf = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct bssload_crossing_info bssload_crossing_info;

		bssload_crossing_info.bssload = event_data->bssload_crossing_info.bssload;
		bssload_crossing_info.bssload_high_thrd = event_data->bssload_crossing_info.bssload_high_thrd;
		bssload_crossing_info.bssload_low_thrd = event_data->bssload_crossing_info.bssload_low_thrd;
		bssload_crossing_info.interface_index = event_data->bssload_crossing_info.interface_index;

		send_pkt_len = sizeof(struct bssload_crossing_info);
		os_alloc_mem(NULL, (UCHAR **)&buf, send_pkt_len);
		if (buf) {
			os_memcpy(buf, &bssload_crossing_info, send_pkt_len);
			wapp_send_1905_msg(wapp, WAPP_BSS_LOAD_CROSSING, send_pkt_len, buf);
			os_free(buf);
		}
	}
}


void wdev_apcli_assoc_stat_change_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL, *ap_wdev;
	int i;
#if NL80211_SUPPORT
	u8 Enable = 0;
#else
	int ret = 0;
#endif /* NL80211_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		// Send to other daemon
		struct apcli_association_info *apcli_stat_info = &wapp->cli_assoc_info;
		apcli_stat_info->interface_index = event_data->apcli_association_info.interface_index;
		apcli_stat_info->apcli_assoc_state = event_data->apcli_association_info.apcli_assoc_state;
		apcli_stat_info->peer_map_enable = event_data->apcli_association_info.PeerMAPEnable;
		apcli_stat_info->current_channel = wdev->radio->op_ch;

#ifdef MAP_R3
		/* If dpp onboard is ongoing then don't handle BEST AP for disconnection */
		if ((wapp->map && wapp->map->map_version == DEV_TYPE_R3) &&
			(wapp->dpp && ((wapp->dpp_caused_disconnect == 1)
			|| (wapp->reconfigTrigger == TRUE)))
			&& (apcli_stat_info->apcli_assoc_state == WAPP_APCLI_DISASSOCIATED)) {
			apcli_stat_info->dpp_onb_ongoing = 1;
			wapp->dpp_caused_disconnect = 0;
		} else {
			if (wapp->dpp && wapp->dpp->dpp_onboard_ongoing) {
				wapp->dpp->dpp_onboard_ongoing = 0;
				wapp_dpp_presence_announce_stop(wapp);
			}
			apcli_stat_info->dpp_onb_ongoing = 0;
		}

		if(wapp->dpp)
			DBGPRINT(RT_DEBUG_WARN, "value of apcli flag onboard:%u and dpp onboard flag:%u \n",
					apcli_stat_info->dpp_onb_ongoing, wapp->dpp->dpp_onboard_ongoing);
#endif /* MAP_R3 */

		if (!wapp->wsc_configs_pending) {
			if (wapp->map && (wapp->map->quick_ch_change == TRUE || wapp->csa_notif_received != TRUE)) {
				wapp_send_1905_msg(wapp, WAPP_APCLI_ASSOC_STAT_CHANGE, sizeof(struct apcli_association_info),
				(char *)&wapp->cli_assoc_info);
				map_update_bh_link_info(wapp, MAP_BH_WIFI, wapp->cli_assoc_info.apcli_assoc_state, ifindex);
				if (!wapp->map->bh_link_num) {
					wapp->map->bh_link_ready = 0;
					if (wapp->map->my_map_dev_role != DEVICE_ROLE_CONTROLLER)
						wapp->map->ctrler_found = 0;
				}
			} else {
				wapp->link_change_notif_pending = TRUE;
			}

			if (wapp->map && (wapp->map->TurnKeyEnable &&
				apcli_stat_info->apcli_assoc_state
				== WAPP_APCLI_DISASSOCIATED)) {
				eloop_cancel_timeout(map_config_state_check,wapp,NULL);
				if (wdev && wdev->dev_type == WAPP_DEV_TYPE_STA) {
#if NL80211_SUPPORT
					wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
							(char *)&Enable, 1);
#else
					char local_command[128];
					os_memset(local_command, 0, sizeof(local_command));
					ret = os_snprintf(local_command, sizeof(local_command), "iwpriv %s set ApCliEnable=0",
						wdev->ifname);
					if (os_snprintf_error(sizeof(local_command), ret)) {
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
						return;
					}
	if (system(local_command) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
#endif
				}
			} else {
				for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
					/* This command will go twice in case of single chip DBDC */
					if (wapp->radio[i].adpt_id) {
						char idfr[MAC_ADDR_LEN];
						struct wapp_radio *ra = &wapp->radio[i];
						MAP_GET_RADIO_IDNFER(ra, idfr);
						ap_wdev = wapp_dev_list_lookup_by_radio(wapp, idfr);
						if (!ap_wdev) {
							DBGPRINT(RT_DEBUG_ERROR, "null wdev\n");
							continue;
						}
#if NL80211_SUPPORT
						Enable = 1;
						wapp_set_APproxy_refresh(wapp, (const char *)wdev->ifname,
								(char *)&Enable, 1);
#else
						char cmd[256];
						os_memset(cmd, 0, 256);
						ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set APProxyRefresh=1;",
							ap_wdev->ifname);
						if (os_snprintf_error(sizeof(cmd), ret)) {
							DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
							continue;
						}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
#endif
					}
				}
			}
		} else {
			if (apcli_stat_info->apcli_assoc_state == WAPP_APCLI_DISASSOCIATED) {
				stop_con_cli_wps(wapp, NULL);
			}
		}

	}
}
void wdev_apcli_assoc_stat_change_handle_vendor10(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL, *ap_wdev;
	int i;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;
	if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		// Send to other daemon
		struct apcli_association_info *apcli_stat_info = &wapp->cli_assoc_info;
		apcli_stat_info->interface_index = event_data->apcli_association_info.interface_index;
		apcli_stat_info->apcli_assoc_state = event_data->apcli_association_info.apcli_assoc_state;
		apcli_stat_info->current_channel = wdev->radio->op_ch;
		for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		/* This command will go twice in case of single chip DBDC */
			if (wapp->radio[i].adpt_id) {
					char idfr[MAC_ADDR_LEN];
					struct wapp_radio *ra = &wapp->radio[i];
					MAP_GET_RADIO_IDNFER(ra, idfr);
					ap_wdev = wapp_dev_list_lookup_by_radio(wapp, idfr);
					if (!ap_wdev) {
						DBGPRINT(RT_DEBUG_ERROR, "null wdev\n");
						continue;
					}
					if (wdev->radio->op_ch != ap_wdev->radio->op_ch)
						continue;
#if NL80211_SUPPORT
					u8 disable = 0;
					u8 enable = 1;

					if(event_data->apcli_association_info.apcli_assoc_state == WAPP_APCLI_DISASSOCIATED)
						wapp_set_v10converter(wapp, (const char *)ap_wdev->ifname,
										(char *)&disable,1);
					else
						wapp_set_v10converter(wapp, (const char *)ap_wdev->ifname,
										(char *)&enable,1);

#else
					char cmd[256];
					os_memset(cmd, 0, 256);
					int ret = 0;

					if (event_data->apcli_association_info.apcli_assoc_state == WAPP_APCLI_DISASSOCIATED) {
						ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set V10Converter=0;", ap_wdev->ifname);
						if (os_snprintf_error(sizeof(cmd), ret)) {
							DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
							continue;
						}
					} else {
						ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set V10Converter=1;", ap_wdev->ifname);
						if (os_snprintf_error(sizeof(cmd), ret)) {
							DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
							continue;
						}
					}
#endif /* NL80211_SUPPORT */

	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
				}
			}
		}
}

#ifdef MAP_R2
void assoc_fail_count_timer(void *eloop_ctx, void *timeout_ctx)
{
	struct wifi_app *wapp = (struct wifi_app *)eloop_ctx;
	wapp->map->assoc_fail_rep_count =0;
}
#endif

void wdev_sta_cnnct_rej_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char *buf = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct sta_cnnct_rej_info sta_cnnct_rej_info;

		sta_cnnct_rej_info.interface_index = event_data->sta_cnnct_rej_info.interface_index;
		os_memcpy(sta_cnnct_rej_info.sta_mac, event_data->sta_cnnct_rej_info.sta_mac, MAC_ADDR_LEN);
		os_memcpy(sta_cnnct_rej_info.bssid, event_data->sta_cnnct_rej_info.bssid, MAC_ADDR_LEN);
		sta_cnnct_rej_info.cnnct_fail.connect_stage = event_data->sta_cnnct_rej_info.cnnct_fail.connect_stage;
		sta_cnnct_rej_info.cnnct_fail.reason = event_data->sta_cnnct_rej_info.cnnct_fail.reason;
#ifdef MAP_R2
		sta_cnnct_rej_info.assoc_status_code = event_data->sta_cnnct_rej_info.assoc_status_code;
		sta_cnnct_rej_info.assoc_reason_code = event_data->sta_cnnct_rej_info.assoc_reason_code;
		//code to check
		printf("### %d %s assoc_status_code = %d: %d \n", __LINE__, __func__, sta_cnnct_rej_info.assoc_status_code,
																		sta_cnnct_rej_info.assoc_reason_code);
		printf("### %d %s report_unsuccessful_association = %d \n", __LINE__, __func__, wapp->map->assoc_failed_policy.report_unsuccessful_association);
		printf("### %d %s max_supporting_rate = %d \n", __LINE__, __func__, wapp->map->assoc_failed_policy.max_supporting_rate);

		wapp->map->assoc_fail_rep_count++;
		if (wapp->map->assoc_failed_policy.report_unsuccessful_association
			&& (wapp->map->assoc_failed_policy.max_supporting_rate > wapp->map->assoc_fail_rep_count)
			)
			sta_cnnct_rej_info.send_failed_assoc_frame = 1;
		else
			sta_cnnct_rej_info.send_failed_assoc_frame = 0;
		printf("### %d %s send_failed_assoc_frame = %d \n", __LINE__, __func__, sta_cnnct_rej_info.send_failed_assoc_frame);

		if(!eloop_is_timeout_registered(assoc_fail_count_timer,wapp,NULL)) {
			eloop_register_timeout(60,0,assoc_fail_count_timer,wapp,NULL );
		}
#endif
		send_pkt_len = sizeof(struct sta_cnnct_rej_info);
		os_alloc_mem(NULL, (UCHAR **)&buf, send_pkt_len);
		if (buf) {
			os_memcpy(buf, &sta_cnnct_rej_info, send_pkt_len);
			wapp_send_1905_msg(wapp, WAPP_STA_CNNCT_REJ_INFO, send_pkt_len, buf);
			os_free(buf);
		}
	}
}

#define L1_PROFILE_PATH "/etc/wireless/l1profile.dat"
int wapp_read_l1_profile_file(char *ifname, int *radio_index, int *band_index)
{
	FILE *file;
	char buf[256], *pos, *token;
	char tmpbuf[256];
	char test_buf[256];
	int line = 0, i = 0;
	char ra_band_idx = 0;
	int ret = 0;


	file = fopen(L1_PROFILE_PATH, "r");
	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR,TOPO_PREX"open l1 profile file (%s) fail\n", L1_PROFILE_PATH);
		return -1;
	}

	for (i = 0; i < 3; i++) {
		ret = os_snprintf(test_buf, sizeof(test_buf), "INDEX%d_apcli_ifname", i);
		if (os_snprintf_error(sizeof(test_buf), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			continue;
		}
		line = 0;
		os_memset(buf, 0, 256);
		os_memset(tmpbuf, 0, 256);
		ra_band_idx = 0;
		while (wapp_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
			ret = os_snprintf(tmpbuf, sizeof(tmpbuf), "%s", pos);
			if (os_snprintf_error(sizeof(tmpbuf), ret)) {
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
				continue;
			}
			token = strtok(pos, "=");
			if (token != NULL) {
				if (os_strcmp(token, test_buf) == 0) {
					token = strtok(NULL, ";");
					while (token != NULL && ra_band_idx < 2) {
						if (os_strcmp(token, ifname) == 0) {
							*radio_index = i;
							*band_index = ra_band_idx;
							ret = fclose(file);
							if (ret != 0) {
								DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n",
									__func__, __LINE__);
								return -1;
							}
							return 0;
						}
						ra_band_idx++;
						token = strtok(NULL, ";");
					}
				}
			}
		}
	}

	ret = fclose(file);
	if (ret != 0)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] fclose fail\n", __func__);
	return -1;
}

#ifdef DPP_SUPPORT
struct wapp_dev* wapp_dev_list_lookup_by_radio_and_type(struct wifi_app *wapp, char* ra_identifier, const u8 wdev_type)
{
        struct wapp_dev *wdev, *target_wdev = NULL;
        unsigned char wdev_identifier[ETH_ALEN];
        struct dl_list *dev_list;

        DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
        dev_list = &wapp->dev_list;

        dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev->radio) {
                        MAP_GET_RADIO_IDNFER(wdev->radio, wdev_identifier);
                        if(!os_memcmp(wdev_identifier, ra_identifier, ETH_ALEN) &&
                                (wdev->dev_type == wdev_type)) {
                                        target_wdev = wdev;
                                        break;
                        }
                }
        }

        return target_wdev;
}

void wdev_get_dpp_action_frame(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	char dpp_frame_event[2048] = {0};
	int frame_len = 2048;
	u32 frm_id;
	if (!wapp)
		return;
	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;
	frm_id = event_data->wapp_dpp_frame_id_no;
	os_memcpy(dpp_frame_event, (char*)&(frm_id), sizeof(u32));
	DBGPRINT(RT_DEBUG_TRACE,"[%s] frame id number = %d\n", __func__, frm_id);
	driver_wext_get_dpp_frame(wapp->drv_data, wdev->ifname, dpp_frame_event ,frame_len);
	wdev_handle_dpp_action_frame(wapp, wdev, (struct wapp_dpp_action_frame *)dpp_frame_event);
}

void wdev_handle_dpp_action_frame(struct wifi_app *wapp,
	struct wapp_dev *wdev, struct wapp_dpp_action_frame *frame)
{
	if (!wapp->dpp) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s: DPP is NULL\n", __func__);
		return;
	}
#ifdef MAP_R3
	if(wapp->map && (wapp->map->map_version == 3)
		&& (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE) && wapp->dpp->dpp_eth_conn_ind) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s: Ignore wifi action frames as ETH is connected\n", __func__);
		return;
	}
#endif /* MAP_R3 */
	if (frame->is_gas) {
		if (wapp->dpp->dpp_configurator_supported
#ifdef MAP_R3
		|| (wapp->dpp->is_map && (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_PROXYAGENT))
#endif
		)
			gas_server_rx(wapp->dpp->gas_server, wdev, wdev->mac_addr,
					(u8 *)frame->src, wdev->mac_addr,
					10, frame->frm, frame->frm_len, frame->chan);
		else
			gas_query_rx(wapp->dpp->gas_query_ctx, wdev->mac_addr, (u8 *)frame->src, (u8 *)frame->src,
					10, frame->frm, frame->frm_len, frame->chan);

	} else
		wapp_dpp_rx_action(wapp, wdev, (u8 *)frame->src,
				frame->frm, frame->frm_len - 4, frame->chan);
}

void wdev_handle_dpp_frm_tx_status(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	struct dpp_tx_status *tx_status = wapp_dpp_get_status_info_from_sq(wapp, event_data->tx_status.seq_no);

	if (!tx_status) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: failed to get status for sq=%u \n", __func__, event_data->tx_status.seq_no);
		return;
	}
	if (tx_status->is_gas_frame) {
		if (wapp->dpp->dpp_configurator_supported)
			gas_server_tx_status(wapp->dpp->gas_server, tx_status->dst,
				NULL, 0, event_data->tx_status.tx_success ? 0 : 1);
		else
			gas_query_tx_status(wapp, 0, tx_status->dst,
				NULL, NULL, NULL, 0, event_data->tx_status.tx_success ? 0 : 1);
	} else
		wapp_dpp_tx_status(wapp, tx_status->dst,
				NULL, 0, event_data->tx_status.tx_success ? 0 : 1);
}
#endif /*DPP_SUPPORT*/

#ifdef MAP_R3
void wdev_handle_dpp_sta_info(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	char buf[2048] = {0};
	char hex_pmk[event_data->sta_info.pmk_len * 2 + 1];
	char hex_ptk[event_data->sta_info.ptk_len * 2 + 1];
	int ret = 0;

#ifdef MAP_R3_RECONFIG
	char cmd[100];

	if (wapp->reconfigTrigger == TRUE) {
		ret = os_snprintf(cmd, sizeof(cmd), "rm /etc/dpp_cfg_test.txt");
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return;
		}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
		wapp->reconfigTrigger = FALSE;
	}
#endif
	ret = os_snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x_SSID %s\n",
		PRINT_MAC(event_data->sta_info.src), event_data->sta_info.ssid);
	if (os_snprintf_error(sizeof(buf), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	wpa_write_config_file(DPP_CFG_FILE_TEST, buf, strlen(buf));
	os_memset(buf, 0, 2048);
	ret = os_snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x_passphrase %s\n",
		PRINT_MAC(event_data->sta_info.src), event_data->sta_info.passphrase);
	if (os_snprintf_error(sizeof(buf), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	wpa_write_config_file(DPP_CFG_FILE_TEST, buf, strlen(buf));
	os_memset(buf, 0, 2048);
	os_snprintf_hex(hex_pmk, sizeof(hex_pmk), event_data->sta_info.pmk,
		event_data->sta_info.pmk_len);
	ret = os_snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x_PMK %s\n",
		PRINT_MAC(event_data->sta_info.src), hex_pmk);
	if (os_snprintf_error(sizeof(buf), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	wpa_write_config_file(DPP_CFG_FILE_TEST, buf, strlen(buf));
	os_memset(buf, 0, 2048);
	ret = os_snprintf_hex(hex_ptk, sizeof(hex_ptk), event_data->sta_info.ptk,
		event_data->sta_info.ptk_len);
	if (os_snprintf_error(sizeof(hex_ptk), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	ret = os_snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x_PTK %s\n",
		PRINT_MAC(event_data->sta_info.src), hex_ptk);
	if (os_snprintf_error(sizeof(buf), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	wpa_write_config_file(DPP_CFG_FILE_TEST, buf, strlen(buf));
}

void wdev_handle_dpp_uri_info(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int peer_bi_id = 0;
	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev || !wapp || !wapp->dpp) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"valid wdev not found..\n");
		return;
	}

	if((wapp->map->map_version == DEV_TYPE_R3) && (wapp->dpp->onboarding_type != 0)) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"invalid map version or onboarding type..\n");
		return;
	}

	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"Sta_MAC:" MACSTR "for wdev:%s uri:%s len:%u\n", PRINT_MAC(event_data->uri_info.src_mac),
			wdev->ifname, event_data->uri_info.rcvd_uri, event_data->uri_info.uri_len);

	if(wapp->dpp->dpp_allowed_roles == DPP_CAPAB_CONFIGURATOR) {
		peer_bi_id = wapp_dpp_qr_code(wapp, (const char *)event_data->uri_info.rcvd_uri);
		if(peer_bi_id == 0) {
			DBGPRINT(RT_DEBUG_ERROR,"invalid bi id..\n");
			return;
		}
	}

	if((wapp->dpp->dpp_allowed_roles == DPP_CAPAB_PROXYAGENT) && (wapp->dpp->map_sec_done == TRUE)) {
		dpp_URI_1905_send(wapp, NULL, wdev, (unsigned char *)event_data->uri_info.src_mac,
					(unsigned short)event_data->uri_info.uri_len, (unsigned char *)event_data->uri_info.rcvd_uri);
	}
	else {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"1905 security is not enabled.\n");
	}
}
#endif /* MAP_R3 */

void wdev_handle_wsc_eapol_notif(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	stop_con_ap_wps(wapp, wdev);
}

void wdev_handle_wsc_eapol_end_notif(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char *buf = NULL;
	struct wapp_bhsta_info *bhsta_info;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	wapp_device_status *device_status = &wapp->map->device_status;
	device_status->status_fhbss = STATUS_FHBSS_WPS_SUCCESSFULL;
	eloop_cancel_timeout(map_wps_timeout, wapp, device_status);

	bhsta_info = (struct wapp_bhsta_info *)event_data;
	send_pkt_len = sizeof(struct wapp_bhsta_info);

	DBGPRINT(RT_DEBUG_ERROR, "%s connected_bssid: "MACSTR"\n", __func__, MAC2STR(bhsta_info->connected_bssid));
	DBGPRINT(RT_DEBUG_ERROR, "%s mac_addr: "MACSTR"\n", __func__, MAC2STR(bhsta_info->mac_addr));
	DBGPRINT(RT_DEBUG_ERROR, "%s peer_map_enable: %d, send_pkt_len %d\n", __func__, bhsta_info->peer_map_enable, send_pkt_len);

	os_alloc_mem(NULL, (UCHAR **)&buf, send_pkt_len);

	if (buf) {
		os_memcpy(buf, bhsta_info, send_pkt_len);
		if (wapp_send_1905_msg(wapp, WAPP_MAP_AGENT_WPS_SUCCESS, send_pkt_len, buf) < 0)
			DBGPRINT(RT_DEBUG_ERROR, "%s wapp_send_1905_msg failed\n", __func__);
		os_free(buf);
	} else
		DBGPRINT(RT_DEBUG_ERROR, "%s buf alloc failed\n", __func__);

	wapp_send_1905_msg(wapp, WAPP_DEVICE_STATUS, sizeof(wapp_device_status), (char *)device_status);
}

void wdev_handle_scan_complete_notif(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	/*Ignore! if scan is not triggered by wapp*/
	if(!wdev->wapp_triggered_scan) {
		return;
	}
	wdev->wapp_triggered_scan = FALSE;
	map_get_scan_result(wapp, wdev);
}

void wdev_enable_pmf(struct wifi_app *wapp, struct wapp_dev *wdev)
{
	if (!wapp || !wdev) {
		return;
	}
#if NL80211_SUPPORT
	u8 enable = 1;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP)
		wapp_set_pmfmfpc(wapp, (const char *)wdev->ifname,
					(char *)&enable,1);
	else
		wapp_set_apcli_PMFMFPC(wapp, (const char *)wdev->ifname,
				(char *)&Enable, 1);
#else
	char cmd[100];
	int ret = 0;

	os_memset(cmd, 0, 100);
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set PMFMFPC=1", wdev->ifname);
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return;
		}
	} else {
		ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliPMFMFPC=1", wdev->ifname);
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return;
		}
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
#endif /* NL80211_SUPPORT */
}

void wdev_bh_sta_reset_default(struct wifi_app *wapp, struct wapp_dev *wdev)
{
	if (!wapp)
		return;

#if NL80211_SUPPORT
	u8 Enable = 0;
	wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
			(char *)&Enable, 1);
	wapp_set_apcli_ssid(wapp, (const char *)wdev->ifname,
			(char *)"MAP_APCLI_UNCONF", 16);
	wapp_set_apcli_wpapsk(wapp, (const char *)wdev->ifname,
			(char *)"12345678", 8);
	wapp_set_authtype(wapp, (const char *)wdev->ifname,
			"OPEN", 4);
	wapp_set_apcli_PMFMFPC(wapp, (const char *)wdev->ifname,
			(char*)&Enable, 1);
	wapp_set_apcli_EncrypType(wapp, (const char *)wdev->ifname,
			(char*)"NONE", 4);
#else
	char cmd[100];
	int ret = 0;

	memset(cmd, 0, 100);
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliEnable=0;", wdev->ifname);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);

	memset(cmd, 0, 100);
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliSsid=MAP_APCLI_UNCONF", wdev->ifname);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	DBGPRINT(RT_DEBUG_ERROR, "<CMD:%s> \n",cmd);
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);

	memset(cmd, 0, 100);
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliWPAPSK=12345678", wdev->ifname);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	memset(cmd, 0, 100);
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliAuthMode=OPEN", wdev->ifname);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	DBGPRINT(RT_DEBUG_ERROR, "<CMD:%s> \n",cmd);
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	os_memset(cmd, 0, 100);
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliPMFMFPC=0;", wdev->ifname);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	DBGPRINT(RT_DEBUG_ERROR, "<CMD:%s> \n",cmd);

	memset(cmd, 0, 100);
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliEncrypType=NONE", wdev->ifname);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	DBGPRINT(RT_DEBUG_ERROR, "<CMD:%s> \n",cmd);
#endif
	return;
}

#ifdef MAP_R2
void wdev_bh_sta_connect_wsc_profile(struct wifi_app *wapp, struct wapp_dev *wdev, wsc_apcli_config *cli_conf)
{
	char *auth_str[] = {
		"OPEN",
		"WPAPSK",
		"SHARED",
		"WPA",
		"WPA2",
		"WPA2PSK",
		"WPA2PSKWPA3PSK"
	};
	char *encryp_str[] = {
		"NONE",
		"WEP",
		"TKIP",
		"AES",
	};
	int i = 0, j = 0;
	unsigned short authmode = cli_conf->AuthType;
	unsigned short encryptype = cli_conf->EncrType;


#if NL80211_SUPPORT
	u8 Enable = 0;
#else
	char cmd[200];
	int ret = 0;
#endif

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	if (cli_conf->SsidLen <= MAX_LEN_OF_SSID)
		cli_conf->ssid[cli_conf->SsidLen] = '\0';
	else
		cli_conf->ssid[MAX_LEN_OF_SSID] = '\0';

	if (cli_conf->KeyLength) {
		if (cli_conf->KeyLength < 64)
			cli_conf->Key[cli_conf->KeyLength] = '\0';
		else
			cli_conf->Key[63] = '\0';
	}

#if NL80211_SUPPORT
	wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
			(char *)&Enable, 1);
	wapp_set_apcli_ssid(wapp, (const char *)wdev->ifname,
			(char *)cli_conf->ssid, cli_conf->SsidLen);
	wapp_set_apcli_wpapsk(wapp, (const char *)wdev->ifname,
			(char *)cli_conf->Key, cli_conf->KeyLength);
#else
	memset(cmd, 0, 200);
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliEnable=0;", wdev->ifname);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	DBGPRINT(RT_DEBUG_ERROR, "<CMD:%s> \n",cmd);
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	memset(cmd, 0, 200);
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliSsid=%s", wdev->ifname, cli_conf->ssid);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	DBGPRINT(RT_DEBUG_ERROR, "<%s> \n",cmd);

	/* this validation is invalid in case of WPA2+WPA */
	if ((authmode < WSC_AUTHTYPE_OPEN || authmode > (WSC_AUTHTYPE_WPA2PSK | WSC_AUTHTYPE_SAE)) ||
		(encryptype < WSC_ENCRTYPE_NONE || encryptype > WSC_ENCRTYPE_AES)) {
		DBGPRINT(RT_DEBUG_ERROR, "%s, invalid sec_info auth=%d enc=%d\n",
				__func__, authmode, encryptype);
		return;
	}

	memset(cmd, 0, 200);
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliWPAPSK=%s", wdev->ifname, cli_conf->Key);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
#endif
#ifdef MAP_R2
	/* Make it mixed mode */
	if (wapp->map->map_version >= DEV_TYPE_R2 && authmode == WSC_AUTHTYPE_WPA2PSK)
		authmode = authmode << 1;
#endif
	DBGPRINT(RT_DEBUG_ERROR,"setting wpa3\n");
	while (authmode) {
		authmode = authmode >> 1;
		i++;
	}
	i--;
#if NL80211_SUPPORT
	wapp_set_authtype(wapp, (const char *)wdev->ifname,
			(char *)auth_str[i], strlen(auth_str[i]));
	if (strcmp(auth_str[i], "WPA2PSKWPA3PSK") == 0) {
		Enable = 1;
		wapp_set_apcli_PMFMFPC(wapp, (const char *)wdev->ifname,
				(char*)&Enable, 1);
	}
#else
	memset(cmd, 0, 200);
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliAuthMode=%s", wdev->ifname, auth_str[i]);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	DBGPRINT(RT_DEBUG_ERROR, "<CMD:%s> \n",cmd);
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	if (strcmp(auth_str[i], "WPA2PSKWPA3PSK") == 0) {
		os_memset(cmd, 0, 200);
		ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliPMFMFPC=1;", wdev->ifname);
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return;
		}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
		DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
	}
#endif
	while (encryptype) {
		encryptype = encryptype >> 1;
		j++;
	}
	j--;

#if NL80211_SUPPORT
	wapp_set_apcli_EncrypType(wapp, (const char *)wdev->ifname,
			(char*)encryp_str[j], strlen(encryp_str[j]));
	Enable = 1;
	wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
			(char *)&Enable, 1);
#else
	memset(cmd, 0, 200);
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliEncrypType=%s", wdev->ifname, encryp_str[j]);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}

	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	memset(cmd, 0, 200);
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliEnable=1;", wdev->ifname);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
#endif
	return;

}
#endif

void wdev_handle_wsc_config_write(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	wsc_apcli_config_msg *apcli_config_msg;
	char wsc_profile_buffer[512] = {0};
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;
//	send oid here to fill up the wsc profile credentials from driver
	driver_wext_get_wsc_profiles(wapp->drv_data, wdev->ifname,wsc_profile_buffer,&send_pkt_len);
	apcli_config_msg = (wsc_apcli_config_msg *)wsc_profile_buffer;
	DBGPRINT(RT_DEBUG_ERROR, "In WAPP , profile count is %d ,send_pkt_len %d \n",apcli_config_msg->profile_count,send_pkt_len );
#ifdef MAP_R3
	/* Storing the WSC profile count for decision regarding
	 * WSC or DPP onboarding while configuring BSS's */
	if(wapp->dpp) {
		wapp->dpp->wsc_profile_cnt = apcli_config_msg->profile_count;
		if(wapp->map && (wapp->map->map_version == DEV_TYPE_R3) &&
				(wapp->dpp->onboarding_type == 0) && (apcli_config_msg->profile_count == 0)) {
			/* In MAP-R3 DPP case if no credentials found
			 * in M8 then trigger Chirping */
			wapp->wsc_configs_pending = FALSE;
			wapp->dpp->annouce_enrolle.is_enable = 1;
			wapp_dpp_presence_annouce(wapp, NULL);
			return;
		}
	}
#endif /* MAP_R3 */
	if(wapp->wsc_save_bh_profile == TRUE) {
		write_configs(wapp, apcli_config_msg->apcli_config, 0, NULL);
		wapp->wsc_save_bh_profile = FALSE;
	}
	if(wapp->wps_on_controller_cli==1) {
		write_configs(wapp,apcli_config_msg->apcli_config, 0, NULL);
		wapp->wps_on_controller_cli=0;
	}
	wapp->wsc_configs_pending = FALSE;
	if (wapp->map && wapp->map->TurnKeyEnable)
		wapp_send_1905_msg(wapp, WAPP_MAP_BH_CONFIG, send_pkt_len,(char *) apcli_config_msg);
#ifdef MAP_R2
        else {
		/*4.14.2_BH5GL_FH24G: we have received WPS profiles from driver. Need to trigger connection on the bh_sta configured.*/
		if (!wapp->map) {
			DBGPRINT(RT_DEBUG_ERROR, "wap->map found NULL %s -%d\n", __func__, __LINE__);
			return;
		}

		struct wapp_dev *bsta_wdev =  wapp->map->bh_wifi_dev;
		DBGPRINT(RT_DEBUG_ERROR, "<ProfileSSID: %s> \n",apcli_config_msg->apcli_config[0].ssid);
		if(bsta_wdev == NULL ||apcli_config_msg->profile_count == 0)
			return;
		// Stop WPS on APCLI interface

		DBGPRINT(RT_DEBUG_ERROR, "Calling WSC stop \n");
#if NL80211_SUPPORT
		u8 enable = 1;

		wapp_set_wsc_stop(wapp, (const char *)wdev->ifname,
					(char *)&enable,1);
#else
		char cmd[100];
		int ret = 0;

		os_memset(cmd, 0, sizeof(cmd));
		ret = os_snprintf(cmd, sizeof(cmd),
			"iwpriv %s set WscStop=1;",
			wdev->ifname);
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return;
		}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
#endif
		// TODO: what if channel of the BSS is different.
		/*Only trigger connection on the bh_wdev configured through the command.*/
		wdev_bh_sta_connect_wsc_profile(wapp, bsta_wdev, &apcli_config_msg->apcli_config[0]);
		/*Disable APCLI on other interfaces*/
		{
			struct wapp_dev *tmp_wdev;
			struct dl_list *dev_list;

			DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
			dev_list = &wapp->dev_list;

			dl_list_for_each(tmp_wdev, dev_list, struct wapp_dev, list){
				if (tmp_wdev->dev_type == WAPP_DEV_TYPE_STA && tmp_wdev != bsta_wdev) {
#if NL80211_SUPPORT
					u8 Enable = 0;
					wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
							(char *)&Enable, 1);
#else
					memset(cmd, 0, 100);
					ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliEnable=0;", tmp_wdev->ifname);
					if (os_snprintf_error(sizeof(cmd), ret)) {
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
						continue;
					}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
#endif
				}
	}
}

	}
#endif
}

void wdev_handle_map_vend_ie_evt(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char *buf = NULL;
	struct map_vendor_ie *map_ie;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	map_ie = (struct map_vendor_ie *)event_data;
	send_pkt_len = sizeof(struct map_vendor_ie);
	os_alloc_mem(NULL, (UCHAR **)&buf, send_pkt_len);
	os_memcpy(buf, map_ie, send_pkt_len);
	wapp_send_1905_msg(wapp, WAPP_MAP_VEND_IE_CHANGED, send_pkt_len, buf);
	os_free(buf);
}

void wdev_handle_a4_entry_missing_notif(struct wifi_app *wapp, u32 ifindex,
	wapp_event_data *event_data)
{
	uint32_t dstip = (uint32_t)event_data->a4_missing_entry_ip;

	if (dstip == 0 || dstip == 0xffffffff) {
        DBGPRINT(RT_DEBUG_ERROR, "Invalid source IP\n");
        return;
    }

	test_arping(wapp->map->br.arp_sock, wapp->map->br.ifindex,
		wapp->map->br.mac_addr, (uint32_t)wapp->map->br.ip,
		dstip);
}

#ifdef WIFI_MD_COEX_SUPPORT
extern int map_send_ap_oper_bss_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf);
/*below channel list is defined by modem*/
unsigned char channel_24G[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
unsigned char channel_5G0[] = {36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80,
							   84, 88, 92, 96, 100, 104, 108, 112, 116, 120,
							   124, 128, 132, 136, 140, 144};
unsigned char channel_5G1[] = {149, 153, 157, 161, 165, 169, 173, 177, 181};

void wdev_handle_unsafe_ch_event(struct wifi_app *wapp,
	wapp_event_data *event_data)
{
	struct unsafe_channel_notif_s *unsafe_ch = &event_data->unsafe_ch_notif;
	unsigned char radio_id[MAC_ADDR_LEN];
	unsigned int event_size = 0, i = 0;
	char *channel_prefer_info = NULL;
	struct evt * evt_tlv = (struct evt *)channel_prefer_info;

	channel_prefer_info = os_zalloc(1024);
	if (channel_prefer_info == NULL) {
		return;
	}

	wapp->map->off_ch_scan_state.ch_scan_state = CH_SCAN_IDLE;
	evt_tlv = (struct evt *)channel_prefer_info;

	DBGPRINT(RT_DEBUG_ERROR, "bitmap([0~3]=%08x,%08x,%08x,%08x)\n",
		unsafe_ch->ch_bitmap[0], unsafe_ch->ch_bitmap[1], unsafe_ch->ch_bitmap[2],
		unsafe_ch->ch_bitmap[3]);
	/*align with modem, bit 0 is not used for 2.4G band*/
	unsafe_ch->ch_bitmap[0] = unsafe_ch->ch_bitmap[0] >> 1;
	/*update channel_operable_status*/
	for (i = 0; i< (sizeof(channel_5G0) / sizeof(unsigned char)); i++) {
		if(i < (sizeof(channel_24G) / sizeof(unsigned char)))
			update_primary_ch_status(channel_24G[i], ((1 << i) & unsafe_ch->ch_bitmap[0]) ? 1 : 0);
		if(i < (sizeof(channel_5G0) / sizeof(unsigned char)))
			update_primary_ch_status(channel_5G0[i], ((1 << i) & unsafe_ch->ch_bitmap[1]) ? 1 : 0);
		if(i < (sizeof(channel_5G1) / sizeof(unsigned char)))
			update_primary_ch_status(channel_5G1[i], ((1 << i) & unsafe_ch->ch_bitmap[2]) ? 1 : 0);
	}

	dump_operable_channel();

	/*build channel preference for each radio*/
	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		if (wapp->radio[i].adpt_id != 0) {
			MAP_GET_RADIO_IDNFER(((struct wapp_radio *)&wapp->radio[i]), radio_id);
			event_size = map_build_chn_pref(
				wapp, radio_id, channel_prefer_info);
			if (event_size) {
				wapp_send_1905_msg(wapp, WAPP_CHANNLE_PREFERENCE, event_size,
					(char *)evt_tlv->buffer);
			}
			os_sleep(0, 10);
		}
	}
	os_free(channel_prefer_info);
}

void wdev_handle_band_status_change(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	struct band_status_change *band_status_change = &event_data->band_status;
	unsigned char radio_id[MAC_ADDR_LEN];
	struct wapp_dev *wdev = NULL;
	int event_size = 0;
	char *event_buf = NULL;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	event_buf = os_malloc(3072);

	if (!event_buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s fail to alloc memory\n", __func__);
		return;
	}

	wdev->radio->operatable = band_status_change->status;

	MAP_GET_RADIO_IDNFER(wdev->radio, radio_id);
	DBGPRINT(RT_DEBUG_ERROR, "radio("MACSTR")- %s operatable by modem\n",
		MAC2STR(radio_id), !wdev->radio->operatable ? "not" : "");

	/*update operational bss information when current radio's status changing*/
	map_send_ap_oper_bss_msg(wapp,
		(unsigned char *)&radio_id,
		event_buf, &event_size);

	if (0 > map_1905_send(wapp, event_buf, event_size)) {
		DBGPRINT(RT_DEBUG_TRACE, "%s  send fail \n", __func__);
		os_free(event_buf);
		return;
	}

	DBGPRINT(RT_DEBUG_ERROR, "radio("MACSTR")-update oper BSS\n",
		MAC2STR(radio_id));

	os_free(event_buf);
}
#endif

void wdev_handle_radar(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
#ifdef MAP_R3
	struct dpp_authentication *auth = NULL, *auth_tmp = NULL;
	char value[200] = {0};
	char param[65] = {0}, profile_found = 0;
	int i = 0;
#endif
#ifdef MAP_R3_RECONFIG
	u8 target_band = 0;
	u8 reconfig_bh_flag = 0;
	struct wapp_dev *target_wdev = NULL;
	struct wapp_dev *radar_wdev = NULL, *tmp_wdev = NULL;
	struct dl_list *dev_list = NULL;
#endif /* MAP_R3_RECONFIG */
	struct nop_channel_list_s nop_channels;
	struct radar_notif_s *radar_notif = &event_data->radar_notif;
	unsigned char radio_id[MAC_ADDR_LEN] = {0};
	struct wapp_dev *wdev = NULL;
	unsigned int event_size = 0;
	char *channel_prefer_info = NULL;
	struct evt * evt_tlv = (struct evt *)channel_prefer_info;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	channel_prefer_info = os_zalloc(1024);
	if (channel_prefer_info == NULL) {
		return;
	}

	os_memset(&nop_channels, 0, sizeof(struct nop_channel_list_s));
	wapp->map->off_ch_scan_state.ch_scan_state = CH_SCAN_IDLE;
	evt_tlv = (struct evt *)channel_prefer_info;
	if (wdev->radio)
		MAP_GET_RADIO_IDNFER(wdev->radio, radio_id);

	if (radar_notif->status) {
		DBGPRINT(RT_DEBUG_ERROR, "RADAR Absent, channel = %d, bw = %u!!!\n", radar_notif->channel, radar_notif->bw);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "RADAR Present, channel = %d, bw = %u!!!\n", radar_notif->channel, radar_notif->bw);
	}

	wapp_get_nop_channels(wapp, (char *)wdev->ifname, (char *)&nop_channels,
			sizeof(nop_channels));
	wdev_handle_nop_channels(&nop_channels);
	update_primary_ch_status(radar_notif->channel, radar_notif->status);

#ifdef MAP_R3
	if ((wapp->dpp->onboarding_type == DPP_ONBOARDING_TYPE) && (wapp->map->map_version == DEV_TYPE_R3)) {
		if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_CONFIGURATOR ||
			wapp->dpp->dpp_allowed_roles == DPP_CAPAB_PROXYAGENT) {
			dl_list_for_each_safe(auth, auth_tmp, &wapp->dpp->dpp_auth_list, struct dpp_authentication, list) {
				if (auth->curr_chan == radar_notif->channel &&
					auth->ethernetTrigger == FALSE) {
					wapp_dpp_cancel_timeouts(wapp, auth);
					eloop_cancel_timeout(wapp_dpp_conn_status_result_wait_timeout, wapp, auth);
					dpp_auth_deinit(auth);
					dpp_auth_fail_wrapper(wapp, "DPP Onboarding Fail - Radar Hit");
				}
			}
		} else if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE) {
			dl_list_for_each_safe(auth, auth_tmp, &wapp->dpp->dpp_auth_list, struct dpp_authentication, list) {
				if (auth && (wapp->is_eth_onboard != TRUE) &&  (wapp->dpp->dpp_onboard_ongoing == 1)
					&& (auth->curr_chan == radar_notif->channel)) {

					DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX "radar hit enrollee \n");
#ifdef MAP_R3_RECONFIG
					if (auth->reconfigTrigger == TRUE) {
				                wapp->dpp->reconfig_annouce.is_enable = 1;
					        wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT;
				                wapp->dpp->dpp_allowed_roles = DPP_CAPAB_ENROLLEE;
				                wapp->dpp->cce_driver_scan_done = 0;
//New Addition
					if (wdev->radio) {
						target_band = (int)wapp_op_band_frm_ch(wapp, wdev->radio->op_ch);
						radar_wdev = wapp_dev_list_lookup_by_band_and_type(wapp,
									target_band, WAPP_DEV_TYPE_STA);

					if (!radar_wdev) {
						DBGPRINT(RT_DEBUG_OFF,
						"DPP: Radar ApCli wdev not found..\n");
						goto out;
					}

					dev_list = &wapp->dev_list;
					/* Skipping the radar detected wdev for reconfig announce */
					dl_list_for_each(tmp_wdev, dev_list, struct wapp_dev, list) {
					if (((tmp_wdev->dev_type == WAPP_DEV_TYPE_STA)
						&& (tmp_wdev->ifindex == radar_wdev->ifindex))
						|| (tmp_wdev->dev_type == WAPP_DEV_TYPE_AP))
						continue;
					target_wdev = tmp_wdev;
					}
					if (!target_wdev) {
						DBGPRINT(RT_DEBUG_OFF,
						"DPP: ApCli wdev not found..\n");
						goto out;
					}
				} else {
					DBGPRINT(RT_DEBUG_OFF,
						"DPP: wdev invalid:%s\n", target_wdev->ifname);
						goto out;
				}

				if (!target_wdev->config) {
					DBGPRINT(RT_DEBUG_ERROR, "target wdev config is NULL\n");
					goto out;
				}
				if (!target_wdev->config->dpp_ppkey) {
					DBGPRINT(RT_DEBUG_ERROR,
					"DPP: No PP-Key, no Reconfiguration");
					goto out;
				}

				if (!target_wdev->config->dpp_reconfig_id) {
					target_wdev->config->dpp_reconfig_id =
							dpp_gen_reconfig_id(target_wdev->config->dpp_csign,
							target_wdev->config->dpp_csign_len,
							target_wdev->config->dpp_ppkey,
							target_wdev->config->dpp_ppkey_len);

					if (!target_wdev->config->dpp_reconfig_id) {
						DBGPRINT(RT_DEBUG_ERROR,
								"DPP: Failed to generate E-id for reconfiguration");
						goto out;
					}
				}

				wapp_dpp_cancel_timeouts(wapp, auth);
				eloop_cancel_timeout(wapp_dpp_conn_result_timeout, wapp, auth);
				reconfig_bh_flag = auth->reconfig_bhconfig_done;
				dpp_auth_deinit(auth);
				if (reconfig_bh_flag) {
					DBGPRINT(RT_DEBUG_ERROR,
					"DPP: Skipping triggerring reconfiguration if BH Profile is present");
					continue;
				}

				wapp->dpp->radar_detect_ind = TRUE;
				radar_wdev->scan_done_ind = TRUE;
				wapp->cce_scan_count++;
				wapp_dpp_send_reconfig_annouce(wapp, target_wdev);
				} else
#endif
#ifdef DPP_R2_SUPPORT
				{
						int ret = 0;

						wapp_dpp_cancel_timeouts(wapp, auth);
						eloop_cancel_timeout(wapp_dpp_conn_result_timeout, wapp,auth);
						dpp_auth_deinit(auth);
						// Add check to prevent presence announcement if profile already exist
						for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
							ret = os_snprintf(param, sizeof(param), "BhProfile%dValid", i);
							if (os_snprintf_error(sizeof(param), ret)) {
								DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n",
									__func__, __LINE__);
								continue;
							}
//							get_map_parameters(wapp->map, param, value, NON_DRIVER_PARAM, sizeof(value));
							get_parameters((char *)wapp->map->map_user_cfg, param, value, NON_DRIVER_PARAM, sizeof(value));
							if ((strcmp(value,"1")) == 0) {
								 profile_found = 1;
								 break;
							}
						}

						if (profile_found != 1) {
							wapp->dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_INIT;
							wapp_dpp_presence_annouce(wapp, NULL);
						}
					}
#endif /* DPP_R2_SUPPORT */
					dpp_auth_fail_wrapper(wapp, "DPP Onboarding Fail - Radar Hit");
				}
			}
		}
	}
#endif
	event_size = map_build_chn_pref(
	wapp, radio_id, channel_prefer_info);
	if (event_size) {
		wapp_send_1905_msg(wapp, WAPP_CHANNLE_PREFERENCE, event_size,
			(char *)evt_tlv->buffer);
	}

#ifdef MAP_R3
#ifdef MAP_R3_RECONFIG
out:
#endif
#endif
	if (channel_prefer_info != NULL) {
		os_free(channel_prefer_info);
		channel_prefer_info = NULL;
	}
}
#ifdef MAP_R3_RECONFIG

int reconfigset = 0;
void wdev_handle_reconfig_trigger(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL, *wdevlist=NULL;
	struct dl_list *dev_list;

	char cmd[100];
	char reconfigTrigger = TRUE;

	DBGPRINT(RT_DEBUG_OFF,"In wdev_handle_reconfig_trigger");

	if ((wapp->dpp->onboarding_type == DPP_ONBOARDING_TYPE) && (wapp->map->map_version > 2)
		&& (wapp->dpp->dpp_allowed_roles != DPP_CAPAB_CONFIGURATOR)) {

		/* Send event to map for reconfig trigger */
		wapp_send_1905_msg(wapp, WAPP_RECONFIG_STATUS, 1, (char *)&reconfigTrigger);


		wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);


		if (!wdev) {
			DBGPRINT(RT_DEBUG_ERROR,"return wdev error");
			reconfigTrigger = FALSE;
			wapp_send_1905_msg(wapp, WAPP_RECONFIG_STATUS, 1, (char *)&reconfigTrigger);
			return;
		}

		DBGPRINT(RT_DEBUG_OFF,"wdev type %s, %d", __func__, wdev->dev_type);
		if (!wapp->drv_ops || !wapp->drv_ops->drv_set_1905_sec) {
			reconfigTrigger = FALSE;
			wapp_send_1905_msg(wapp, WAPP_RECONFIG_STATUS, 1, (char *)&reconfigTrigger);
			return;
		}

		dev_list = &wapp->dev_list;
		printf("\n Prak wdev ifindex -%d, ifname -%s",wdev->ifindex, wdev->ifname);
#if 0
//	if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		if (reconfigset == 0) {
			reconfigset++;
			wapp->dpp->reconfig_annouce.is_enable = 1;
			wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT;
#if NL80211_SUPPORT
			u8 Enable = 0;
			wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
					(char *)&Enable, 1);
#else
			os_memset(cmd, 0, 100);
			sprintf(cmd, "iwpriv apclix0 set ApCliEnable=0;");
			system(cmd);
#endif
			sleep(20);
			wapp_dpp_send_reconfig_annouce(wapp, wdev);

		}
//	}
#endif
		if (wapp->reconfigTrigger != TRUE) {
			dl_list_for_each(wdevlist, dev_list, struct wapp_dev, list) {
				if (wdevlist->radio
					&& wdevlist->dev_type == WAPP_DEV_TYPE_STA) {
#if NL80211_SUPPORT
					u8 Enable = 0;

					wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
						(char *)&Enable, 1);
#else
					int ret = 0;

					os_memset(cmd, 0, 100);
					printf("\n Prak apcli set disable");

					ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliEnable=0;", wdevlist->ifname);
					if (os_snprintf_error(sizeof(cmd), ret)) {
						DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
						continue;
					}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
					//sleep(1);
#endif
				}
			}
		//if (wapp->reconfigTrigger != TRUE) {
			printf("\n Prak Reconfig trigger");
			wapp->dpp->reconfig_annouce.is_enable = 1;
			wapp->dpp->reconfig_annouce.an_status = DPP_AN_STATUS_INIT;
			wapp->dpp->dpp_allowed_roles = DPP_CAPAB_ENROLLEE;
			wapp->dpp->cce_driver_scan_done = 0;
//New Addition
			printf("\n ppkey len is %zu", wdev->config->dpp_ppkey_len);
			if (!wdev->config->dpp_ppkey) {
				DBGPRINT(RT_DEBUG_OFF,
				   "DPP: No PP-Key, no Reconfiguration");
				reconfigTrigger = FALSE;
				wapp_send_1905_msg(wapp, WAPP_RECONFIG_STATUS, 1, (char *)&reconfigTrigger);
				return;
			}

			wdev->config->dpp_reconfig_id = dpp_gen_reconfig_id(wdev->config->dpp_csign,
						     wdev->config->dpp_csign_len,
						     wdev->config->dpp_ppkey,
						     wdev->config->dpp_ppkey_len);
			if (!wdev->config->dpp_reconfig_id) {
				DBGPRINT(RT_DEBUG_OFF,
				   "DPP: Failed to generate E-id for reconfiguration");
				reconfigTrigger = FALSE;
				wapp_send_1905_msg(wapp, WAPP_RECONFIG_STATUS, 1, (char *)&reconfigTrigger);
				return;
			}

			wapp_dpp_send_reconfig_annouce(wapp, wdev);
		}
	}
}
#endif
void wdev_process_wsc_scan_comp(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "wdev==NULL!!!\n");
		return;
	}

	wdev->wsc_scan_info.bss_count = event_data->wsc_scan_info.bss_count;
	os_memcpy(wdev->wsc_scan_info.Uuid,
		event_data->wsc_scan_info.Uuid,
		sizeof(event_data->wsc_scan_info.Uuid));
	/*
		results for WPS scan on wapp->wsc_trigger_wdev are available now,
		call wps_ctrl_run_cli_wps to check if WPS needs to be executed on next BH CLI
		If status for back haul or fronthaul bss is wps_triggered then only
		check for next bss. otherwise just process the last scan results.
	*/
	if((wapp->map->device_status.status_bhsta == STATUS_BHSTA_WPS_TRIGGERED)
		|| (wapp->map->device_status.status_fhbss == STATUS_FHBSS_WPS_TRIGGERED)){
		wapp->wsc_trigger_wdev = wps_ctrl_run_cli_wps(wapp, wapp->wsc_trigger_wdev);
	}
	else{
		wps_ctrl_process_scan_results(wapp);
		return;
	}
	/*
		if wps_ctrl_run_cli_wps returns a NULL WDEV, we have executed scan on all
		available BH CLI. Now we process scan results of all CLIs together.
	*/
	if (wapp->wsc_trigger_wdev == NULL) {
		printf("will process scan results now\n");
		wps_ctrl_process_scan_results(wapp);
	}
}

#ifdef MAP_R2
void map_fill_last_scan_time(struct wifi_app *wapp, u8 radio_idx)
{
	//u8 *radio_id = wapp->map->ch_scan_req->body[radio_idx].radio_id;
	// TODO: Raghav
}
// handle channel scan compelete event from driver.
void wapp_fill_ch_bw_str(struct neighbor_info *dst,
	wdev_ht_cap ht_cap,
	wdev_vht_cap vht_cap);
u8 rssi_to_rcpi(signed char rssi);

void wdev_send_tunnel_assoc_req(struct wifi_app *wapp, u32 ifindex, u16 assoc_len, u8 *mac_addr, u8 isReassoc, u8 *assoc_buffer)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	struct tunneled_msg_tlv *tunelled_tlv=NULL;
	u8 num_payload_tlv = 1;
	u8 proto_type = 0;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);
	if (!wapp || !assoc_len || (assoc_len <= LENGTH_802_11))
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;
//	send oid here to fill up the wsc profile credentials from driver


	if(isReassoc)
		proto_type = 1;

	send_pkt_len = sizeof(struct tunneled_msg_tlv) + assoc_len - LENGTH_802_11;
	tunelled_tlv = os_zalloc(send_pkt_len);
	if(tunelled_tlv == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,"memory alloc fail %s", __func__);
		return;
	}

	tunelled_tlv->payload_len = assoc_len - LENGTH_802_11;
	os_memcpy(&tunelled_tlv->payload[0],assoc_buffer + LENGTH_802_11, assoc_len - LENGTH_802_11);

	map_build_and_send_tunneled_message(wapp, mac_addr, proto_type, num_payload_tlv, tunelled_tlv);


}
#endif // MAP_R2

void wdev_handle_scan_results(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char *buf = NULL;
	struct wapp_scan_info *scan_info = NULL;
	int scan_done = 1;
#ifdef DPP_SUPPORT
	int i;
#endif /* DPP_SUPPORT */

	if (!wapp)
		goto End;
	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		goto End;

	if (!wdev->scan_cookie)
		goto End;

	DBGPRINT(RT_DEBUG_TRACE, "%s for %s\n", __func__, wdev->ifname);

	scan_info = &event_data->scan_info;
	scan_info->interface_index = ifindex;
	if (scan_info->more_bss) {
		scan_done = 0;
		wapp_query_scan_result(wapp, wdev, 1);
	}

#ifdef MAP_R3
	if(scan_done && (wdev->waiting_cce_scan_res == TRUE))
	{
		DBGPRINT(RT_DEBUG_OFF, "%s CCE scan done for %s\n", __func__, wdev->ifname);
		wdev->scan_done_ind = TRUE;
		wapp->cce_scan_count++;
		wapp_dpp_presence_ch_scan(wapp);
		return;
	}
#endif /* MAP_R3 */
	send_pkt_len = sizeof(struct scan_bss_info)* scan_info->bss_count + sizeof(u8) + sizeof(u8) +
		sizeof(unsigned int);
	os_alloc_mem(NULL, (UCHAR **)&buf, send_pkt_len);
	os_memcpy(buf, scan_info, send_pkt_len);
#ifdef DPP_SUPPORT
#ifdef MAP_R3
	if (wapp->map->map_version == DEV_TYPE_R3 && !wapp->map->TurnKeyEnable) {
#endif /* MAP_R3 */
		for (i = 0; i < scan_info->bss_count; i++) {
			struct bss_info_scan_result *scan_result = os_malloc(sizeof(*scan_result));
			if(!scan_result) {
				DBGPRINT(RT_DEBUG_ERROR, "%s Scan_recult malloc failed\n", __func__);
				goto End;
			}
			scan_result->bss = scan_info->bss[i];
			dl_list_add_tail(&(wapp->scan_results_list), &scan_result->list);
		}
#ifdef MAP_R3
	}
#endif /* MAP_R3 */
#endif /* DPP_SUPPORT */
	wapp_send_1905_msg(wapp, WAPP_SCAN_RESULT, send_pkt_len, buf);
	if (scan_done) {
#ifdef DPP_SUPPORT
#ifdef MAP_R3
		if (wapp->map->map_version == DEV_TYPE_R3 && !wapp->map->TurnKeyEnable)
			wapp_handle_dpp_scan(wapp, wdev);
#endif
		DBGPRINT(RT_DEBUG_OFF, "scan_done: %s\n", __func__);
#endif /* DPP_SUPPORT */
		wapp_send_1905_msg(wapp, WAPP_SCAN_DONE, sizeof(int), (char *)&(wdev->scan_cookie));
	}

End:
	if(buf)
		os_free(buf);
}

int wapp_issue_scan_request(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev)
{
	char cmd[256];
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ClearSiteSurvey=1;", wdev->ifname);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	/*Flag to check if scan is triggered from wapp*/
	wdev->wapp_triggered_scan = TRUE;
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set SiteSurvey=;", wdev->ifname);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}
	DBGPRINT(RT_DEBUG_ERROR,"1-%s\n", cmd);
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);

	return ret;
}

#ifdef MAP_R3
int wdev_set_dpp_cred(struct wifi_app *wapp, struct wapp_dev *wdev,
			struct wireless_setting* psetting, int primary_cred)
{
	struct dpp_config *wdev_config = NULL;

	wdev_config = os_zalloc(sizeof(*wdev_config));
	if (!wdev_config)
		goto fail;

	wdev_config->ssid_len = os_strlen((const char *)psetting->Ssid);
	wdev_config->ssid = os_malloc(wdev_config->ssid_len + 1);
	if (!wdev_config->ssid)
		goto fail;

	os_memcpy(wdev_config->ssid, psetting->Ssid, wdev_config->ssid_len);
	wdev_config->ssid[wdev_config->ssid_len] = '\0';

	if(primary_cred) {
		if(dpp_parse_map_conf_obj(wdev_config, wapp, (u8 *)psetting->Cred,
					(u16)psetting->cred_len) != MAP_SUCCESS) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s error wireless setting while parsing dpp obj\n", __func__);
			goto fail;
		}
	}
	else {
		if(dpp_parse_map_conf_obj(wdev_config, wapp, (u8 *)(psetting->Cred + psetting->cred_len),
					(u16)psetting->ext_cred_len) != MAP_SUCCESS) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s error wireless setting while parsing second dpp obj\n", __func__);
			goto fail;
		}
	}
#if 0
	if (auth->connector) {
		wdev_config->key_mgmt = WPA_KEY_MGMT_DPP;
		wdev_config->dpp_connector = os_strdup(auth->connector);
		if (!wdev_config->dpp_connector)
			goto fail;
		DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX "dpp connector copied\n");
	}

	if (auth->c_sign_key) {
		wdev_config->dpp_csign = os_malloc(wpabuf_len(auth->c_sign_key));
		if (!wdev_config->dpp_csign)
			goto fail;
		os_memcpy(wdev_config->dpp_csign, wpabuf_head(auth->c_sign_key),
			  wpabuf_len(auth->c_sign_key));
		wdev_config->dpp_csign_len = wpabuf_len(auth->c_sign_key);
		DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX "dpp c-sign key copied: %d\n",(int)wdev_config->dpp_csign_len);

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
		DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX "dpp new accesss key copied: %d\n",(int)wdev_config->dpp_csign_len);
	}
#endif

	if (dpp_akm_psk(wdev_config->map_bss_akm) || dpp_akm_sae(wdev_config->map_bss_akm)) {
		if (dpp_akm_psk(wdev_config->map_bss_akm))
			wdev_config->key_mgmt |= WPA_KEY_MGMT_PSK |
				WPA_KEY_MGMT_PSK_SHA256 | WPA_KEY_MGMT_FT_PSK;
		if (dpp_akm_sae(wdev_config->map_bss_akm))
			wdev_config->key_mgmt |= WPA_KEY_MGMT_SAE |
				WPA_KEY_MGMT_FT_SAE;
#if 0 //comment for now to be taken care in future
		{
			wdev_config->psk_set = conf->psk_set;
			os_memcpy(wdev_config->psk, conf->psk, PMK_LEN);
		}
#endif
	}

	if (wdev && wdev->config && primary_cred) {
		os_free(wdev->config->dpp_connector);
		os_free(wdev->config->dpp_netaccesskey);
		os_free(wdev->config->dpp_csign);
		os_free(wdev->config->ssid);
		os_free(wdev->config);
		wdev->config = NULL;
	}
	else if(wdev && wdev->config2) {
		os_free(wdev->config2->dpp_connector);
		os_free(wdev->config2->dpp_netaccesskey);
		os_free(wdev->config2->dpp_csign);
		os_free(wdev->config2->ssid);
		os_free(wdev->config2);
		wdev->config2 = NULL;
	}

	if(wdev && primary_cred) {
		/* Set this config in ongoing wdev */
		wdev->config = wdev_config;
	} else if (wdev)
		wdev->config2 = wdev_config;

	if(primary_cred) {
		/* setting creds to driver only once */
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"came here to set akm is :%s\n",
				wdev_get_dpp_driver_auth_type(wdev_config->map_bss_akm));
		wdev_set_dpp_akm(wapp, wdev, wdev_config->map_bss_akm);

		if(!dpp_akm_legacy(wdev_config->map_bss_akm))
			wdev_enable_pmf(wapp,wdev);

		wdev_set_psk(wapp, wdev, (char *)psetting->WPAKey);
		wdev_set_ssid(wapp, wdev, (char *)psetting->Ssid);
	}
	return WAPP_SUCCESS;

fail:
	if (wdev_config && wdev_config->ssid)
		os_free(wdev_config->ssid);
	if (wdev_config && wdev_config->dpp_connector)
		os_free(wdev_config->dpp_connector);
	if (wdev_config && wdev_config->dpp_csign)
		os_free(wdev_config->dpp_csign);
	if (wdev_config && wdev_config->dpp_netaccesskey)
		os_free(wdev_config->dpp_netaccesskey);
	if (wdev_config)
		os_free(wdev_config);

	return WAPP_INVALID_ARG;
}
#endif /* MAP_R3 */

/* need to set SSID in the end to apply sec settings */
int wdev_set_sec_and_ssid(

	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	struct sec_info *sec,
	char *ssid)
{
	char cmd[MAX_CMD_MSG_LEN];
	int ret = 0;
	struct ap_dev * ap = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev || !sec) {
		return WAPP_INVALID_ARG;
	}

	DBGPRINT(RT_DEBUG_ERROR,
		   "set sec_info:\n"
		   "\t ifname = %s\n"
		   "\t ssid = %s\n"
		   "\t auth = %s\n"
		   "\t encryp = %s\n"
		   "\t passphrases = %s\n",
		   wdev->ifname,
		   ssid,
		   sec->auth,
		   sec->encryp,
		   sec->psphr);


	ap = (struct ap_dev *)wdev->p_dev;

#if NL80211_SUPPORT
	wapp_set_auth_mode(wapp, (const char *)wdev->ifname,
			(char *)sec->auth, sizeof(sec->auth));
#else
	os_memset(cmd, 0, MAX_CMD_MSG_LEN);
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set AuthMode=%s;", wdev->ifname, sec->auth);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
#endif
	os_memset(cmd, 0, MAX_CMD_MSG_LEN);
	ret = os_snprintf(cmd, sizeof(cmd), "wifi_config_save %s AuthMode %s", wdev->ifname, sec->auth);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);

#if NL80211_SUPPORT
    wapp_set_EncrypType(wapp, (const char *)wdev->ifname,
		    (char*)sec->encryp,
		    strlen(sec->encryp));
#else
	os_memset(cmd, 0, MAX_CMD_MSG_LEN);
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set EncrypType=%s;", wdev->ifname, sec->encryp);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
#endif
	os_memset(cmd, 0, MAX_CMD_MSG_LEN);
	ret = os_snprintf(cmd, sizeof(cmd), "wifi_config_save %s EncrypType %s", wdev->ifname, sec->encryp);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);

	if (os_strcmp(sec->encryp, "WEP") == 0) {
			os_memset(cmd, 0, MAX_CMD_MSG_LEN);
			ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set DefaultKeyID=1;", wdev->ifname);
			if (os_snprintf_error(sizeof(cmd), ret)) {
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
				return WAPP_INVALID_ARG;
			}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
			os_memset(cmd,0,MAX_CMD_MSG_LEN);
			ret = os_snprintf(cmd, sizeof(cmd), "wifi_config_save %s DefaultKeyID 1", wdev->ifname);
			if (os_snprintf_error(sizeof(cmd), ret)) {
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
				return WAPP_INVALID_ARG;
			}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);

			if((os_strlen(sec->psphr)!=13)&&(os_strlen(sec->psphr)!=5)&&(os_strlen(sec->psphr)!=10)&&(os_strlen(sec->psphr)!=26)) {
				DBGPRINT(RT_DEBUG_ERROR, "%s in WEP key len should be 13 or 5 ascii OR 10 or 26 in valid hex\n", __func__);
				return WAPP_INVALID_ARG;
			}
#if NL80211_SUPPORT
			wapp_set_key1(wapp, (const char *)wdev->ifname,
						(char *)sec->psphr,
						(size_t)sizeof(sec->psphr));
#else
			os_memset(cmd, 0, MAX_CMD_MSG_LEN);
			ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set Key1=%s;", wdev->ifname, sec->psphr);
			if (os_snprintf_error(sizeof(cmd), ret)) {
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
				return WAPP_INVALID_ARG;
			}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
#endif /* NL80211_SUPPORT */

			os_memset(cmd,0,MAX_CMD_MSG_LEN);
			ret = os_snprintf(cmd, sizeof(cmd), "wifi_config_save %s Key1 %s", wdev->ifname, sec->psphr);
			if (os_snprintf_error(sizeof(cmd), ret)) {
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
				return WAPP_INVALID_ARG;
			}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	} else if (os_strcmp(sec->encryp, "WPAPSKWPA2PSK") == 0) {
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
		ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set DefaultKeyID=2;", wdev->ifname);
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return WAPP_INVALID_ARG;
		}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
		os_memset(cmd,0,MAX_CMD_MSG_LEN);
		ret = os_snprintf(cmd, sizeof(cmd), "wifi_config_save %s DefaultKeyID 2", wdev->ifname);
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return WAPP_INVALID_ARG;
		}
		if (system(cmd) == -1)
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
		wdev_set_psk(wapp, wdev, (char *)sec->psphr);
	} else {
		wdev_set_psk(wapp, wdev, (char *)sec->psphr);
	}

	/* Need hook funcion */
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		driver_wext_set_ssid(wapp->drv_data, wdev->ifname, ssid ? ssid : ap->bss_info.ssid);
	}

	if (ret != -1 && ssid) {

		os_strncpy(ap->bss_info.ssid, ssid, MAX_LEN_OF_SSID);
		os_memcpy(&wdev->sec, sec, sizeof(struct sec_info));
	}

	return WAPP_SUCCESS;
}

int wdev_set_quick_ch(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	int ch)
{
	struct dl_list *dev_list;
	struct wapp_dev *wdev_temp;
	dev_list = &wapp->dev_list;
	int ret = 0;
	unsigned int bss_coex_buffer = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s invalid_argument\n", __func__);
		return WAPP_INVALID_ARG;
	}

	if (ch == 0) {
		DBGPRINT_RAW(RT_DEBUG_OFF,
		"channel is 0, return\n");
		return WAPP_INVALID_ARG;
	}

	if (ch == wdev->radio->op_ch) {
		return WAPP_SUCCESS;
	}
	DBGPRINT_RAW(RT_DEBUG_OFF,
		"set ch:\n"
		"\t ifname = %s\n"
		"\t ch = %d\n",
		wdev->ifname, ch);

	dl_list_for_each(wdev_temp, dev_list, struct wapp_dev, list) {
		if (wdev->radio == wdev_temp->radio)
			if (wdev_temp->dev_type == WAPP_DEV_TYPE_AP) {
				struct ap_dev * ap = (struct ap_dev *)wdev_temp->p_dev;
				ap->ch_info.op_ch = ch;
			}
	}

	/* Disable coex scan for this channel, since this is a configuration/auth phase */
	driver_wext_get_bss_coex(wapp->drv_data, wdev->ifname,
			(void *)&bss_coex_buffer);
	if (bss_coex_buffer)
		wdev_set_bss_coex(wapp, wdev, FALSE);

	// TODO move this code outside of MAP in driver
#if NL80211_SUPPORT
	wapp_set_map_channel(wapp, (const char *)wdev->ifname,
				(char *)&ch,
				(size_t)sizeof(ch));
#else
	char cmd[256] = {0};
	os_memset(cmd,0,sizeof(cmd));
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set MapChannel=%d;", wdev->ifname, ch);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
#endif /* NL80211_SUPPORT */

        DBGPRINT(RT_DEBUG_OFF," [%s] Send Channel Command: %s, ret = %d\n", __func__, cmd, ret);
#ifdef MAP_6E_SUPPORT
	/* ToDo: Pass opclass from the caller */
	/* wdev->radio->opclass = opclass; */
#endif
	wapp_radio_update_ch(wapp, wdev->radio, ch);

	/* Enable coex scan */
	if (bss_coex_buffer)
		wdev_set_bss_coex(wapp, wdev, TRUE);

	return WAPP_SUCCESS;
}

int wdev_set_ch(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	int ch,
	unsigned char op_class)
{
	char cmd[256];
	struct dl_list *dev_list;
	struct wapp_dev *wdev_temp;
	dev_list = &wapp->dev_list;
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}

	if (ch == 0) {
		DBGPRINT_RAW(RT_DEBUG_OFF,
		"channel is 0, return\n");
		return WAPP_INVALID_ARG;
	}

	if (ch == wdev->radio->op_ch) {
		return WAPP_SUCCESS;
	}

	DBGPRINT_RAW(RT_DEBUG_OFF,
		"set ch:\n"
		"\t ifname = %s\n"
		"\t ch = %d\n",
		wdev->ifname, ch);

#ifdef MAP_SUPPORT
#if NL80211_SUPPORT
	if(wapp->map->quick_ch_change)
		wapp_set_map_channel(wapp, (const char *)wdev->ifname,
				(char *)&ch,
				(size_t)sizeof(ch));
	else
		wapp_set_channel(wapp, (const char *)wdev->ifname,
				(char *)&ch,
				(size_t)sizeof(ch));
#else
	if(wapp->map->quick_ch_change) {
#ifndef MAP_R2
		ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set MapChannel=%d;", wdev->ifname, ch);
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return WAPP_INVALID_ARG;
		}
#else
		printf("dev role in wapp is %d", wapp->map->my_map_dev_role);
		ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set MapChannel=%d:%d:%d",
			wdev->ifname, ch, !wdev->cac_not_required, wapp->map->my_map_dev_role);
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return WAPP_INVALID_ARG;
		}
		wdev->cac_not_required = 0;
#endif /* MAP_R2 */
	} else
#endif /* NL80211_SUPPORT */
		{
			ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set Channel=%d;", wdev->ifname, ch);
			if (os_snprintf_error(sizeof(cmd), ret)) {
				DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
				return WAPP_INVALID_ARG;
				}
		}

	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	DBGPRINT_RAW(RT_DEBUG_OFF, "%s\n", cmd);
#endif /* MAP_SUPPORT */

	os_memset(cmd, 0, sizeof(cmd));
	ret = os_snprintf(cmd, sizeof(cmd), "wifi_config_save %s Channel %d", wdev->ifname, ch);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}
	ret = system(cmd);
	if (ret == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
        DBGPRINT(RT_DEBUG_OFF," [%s] Send Channel Command: %s, ret = %d\n", __func__, cmd, ret);
#ifdef MAP_6E_SUPPORT
	if (op_class)
		wdev->radio->opclass = op_class;
#endif
	wapp_radio_update_ch(wapp, wdev->radio, ch);
	dl_list_for_each(wdev_temp, dev_list, struct wapp_dev, list) {
		if (wdev->radio == wdev_temp->radio)
			if (wdev_temp->dev_type == WAPP_DEV_TYPE_AP) {
				struct ap_dev * ap = (struct ap_dev *)wdev_temp->p_dev;
				ap->ch_info.op_ch = ch;
				 if (op_class != 0)
					ap->ch_info.op_class = op_class;
			}
	}
	map_operating_channel_info(wapp);

	return WAPP_SUCCESS;
}


int wdev_set_bss_coex(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	char enable)
{
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}
	DBGPRINT(RT_DEBUG_ERROR,
	   "set bss_coex:\n"
	   "\t ifname = %s\n"
	   "\t enable = %d\n",
	   wdev->ifname,
	   enable);

#if NL80211_SUPPORT
	wapp_set_HtBssCoex(wapp, (const char *)wdev->ifname,
			(char *)&enable, 1);
#else
	char cmd[256];
	int ret = 0;

	os_memset(cmd, 0, sizeof(cmd));
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set HtBssCoex=%d;", wdev->ifname, enable);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
#endif
	return WAPP_SUCCESS;
}

#ifdef MAP_R3
u16 wdev_get_dpp_driver_auth_type_wsc(enum dpp_akm akm)
{
	switch (akm) {
	case DPP_AKM_DPP:
		return WPS_AUTH_DPP;
	case DPP_AKM_PSK:
		return WSC_AUTHTYPE_WPA2PSK;
	case DPP_AKM_SAE:
		return WSC_AUTHTYPE_SAE;
	case DPP_AKM_PSK_SAE:
		return WSC_AUTHTYPE_SAE | WSC_AUTHTYPE_WPA2PSK;
	case DPP_AKM_PSK_DPP:
		return WPS_AUTH_DPP | WSC_AUTHTYPE_WPA2PSK;
	case DPP_AKM_SAE_DPP:
		return WPS_AUTH_DPP | WSC_AUTHTYPE_SAE;
	case DPP_AKM_PSK_SAE_DPP:
		return WPS_AUTH_DPP | WSC_AUTHTYPE_SAE | WSC_AUTHTYPE_WPA2PSK;
	default:
		return WSC_AUTHTYPE_WPA2PSK;
	}
}
#endif /* MAP_R3 */

#ifdef DPP_SUPPORT
char * wdev_get_dpp_driver_auth_type(enum dpp_akm akm)
{
	switch (akm) {
	case DPP_AKM_DPP:
		return "DPP";
	case DPP_AKM_PSK:
		return "WPA2PSK";
	case DPP_AKM_SAE:
		return "WPA3PSK";
	case DPP_AKM_PSK_SAE:
		return "WPA2PSKWPA3PSK";
	case DPP_AKM_PSK_DPP:
		return "DPPWPA2PSK";
	case DPP_AKM_SAE_DPP:
		return "DPPWPA3PSK";
	case DPP_AKM_PSK_SAE_DPP:
		return "DPPWPA3PSKWPA2PSK";
	default:
		return "WPA2PSK";
	}
}

int wdev_enable_apcli_iface(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	int enable)
{
	char cmd[100] = {0};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}


	if (wdev->dev_type != WAPP_DEV_TYPE_STA) {
		DBGPRINT(RT_DEBUG_TRACE, "%s incorrect iface type =%d\n", __func__, wdev->dev_type);
		return -1;
	}

#if NL80211_SUPPORT
	wapp_set_apcli_mode(wapp, (const char *)wdev->ifname,
			(char *)&enable, 1);
#else
	int ret = 0;

	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliEnable=%d", wdev->ifname, enable);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
#endif

	return WAPP_SUCCESS;
}

int wdev_set_dpp_akm(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	enum dpp_akm akm)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}

#if NL80211_SUPPORT
	if (wdev->dev_type == WAPP_DEV_TYPE_AP)
		wapp_set_auth_mode(wapp, (const char *)wdev->ifname,
				(char *)wdev_get_dpp_driver_auth_type(akm),
				strlen(sec->auth));
	else
		wapp_set_authtype(wapp, (const char *)wdev->ifname,
				(char *)wdev_get_dpp_driver_auth_type(akm),
				strlen(sec->auth));

	if (wdev->dev_type == WAPP_DEV_TYPE_AP)
		wapp_set_EncrypType(wapp, (const char *)wdev->ifname,
				(char*)"AES", 3);
	else
		wapp_set_apcli_EncrypType(wapp, (const char *)wdev->ifname,
				(char*)"AES", 3);
#else
	char cmd[100];
	int ret = 0;

	memset(cmd, 0, 100);
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set AuthMode=%s", wdev->ifname,
			wdev_get_dpp_driver_auth_type(akm));
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return WAPP_INVALID_ARG;
		}
	} else {
		ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliAuthMode=%s", wdev->ifname,
			wdev_get_dpp_driver_auth_type(akm));
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return WAPP_INVALID_ARG;
		}
	}


	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	memset(cmd, 0, 100);
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		ret = os_snprintf(cmd, sizeof(cmd), "wifi_config_save %s AuthMode %s", wdev->ifname, wdev_get_dpp_driver_auth_type(akm));
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return WAPP_INVALID_ARG;
		}
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	memset(cmd, 0, 100);
	if (wdev->dev_type == WAPP_DEV_TYPE_AP)
		snprintf(cmd, sizeof(cmd), "iwpriv %s set EncrypType=AES", wdev->ifname);
	else
		snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliEncrypType=AES", wdev->ifname);

	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);

	memset(cmd, 0, 100);
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		ret = os_snprintf(cmd, sizeof(cmd), "wifi_config_save %s EncrypType %s", wdev->ifname, "AES");
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return WAPP_INVALID_ARG;
		}
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
#endif

	return WAPP_SUCCESS;
}
#endif /*DPP_SUPPORT*/

int wdev_set_ssid(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	char *ssid)
{

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev || !ssid) {
		return WAPP_INVALID_ARG;
	}

	DBGPRINT_RAW(RT_DEBUG_OFF,
		"set ssid:\n"
		"\t ifname = %s\n"
		"\t ssid = %s\n",
		wdev->ifname, ssid);
	wapp->drv_ops->drv_set_ssid(wapp->drv_data, wdev->ifname, ssid);

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev * ap = (struct ap_dev *)wdev->p_dev;
		os_strncpy(ap->bss_info.ssid, ssid, MAX_LEN_OF_SSID);
	}

	return WAPP_SUCCESS;
}
int wdev_set_psk(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	char *psk)
{

	if (!wapp || !wdev || !psk) {
		return WAPP_INVALID_ARG;
	}

	DBGPRINT_RAW(RT_DEBUG_OFF,
		"set psk:\n"
		"\t ifname = %s\n"
		"\t psk = %s\n",
		wdev->ifname, psk);
	wapp->drv_ops->drv_set_psk(wapp->drv_data, wdev->ifname, psk);

	return WAPP_SUCCESS;
}

int wdev_set_radio_onoff(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	int onoff)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev)
		return WAPP_INVALID_ARG;

	if (!wdev->radio)
		return WAPP_NOT_INITIALIZED;

	if (wdev->radio->onoff == onoff)
		return WAPP_SUCCESS;

#if 1
	if (onoff == RADIO_ON) {
		DBGPRINT_RAW(RT_DEBUG_OFF,
			GRN(
			"set radio:\n"
			"\t ifname = %s\n"
			"\t onoff = %d\n"),
			wdev->ifname, onoff);

	} else {
		DBGPRINT_RAW(RT_DEBUG_OFF,
			RED(
			"set radio:\n"
			"\t ifname = %s\n"
			"\t onoff = %d\n"),
			wdev->ifname, onoff);
	}
#endif
#if NL80211_SUPPORT
	wapp_set_radio_on(wapp, (const char *)wdev->ifname,
				(char *)&onoff,
				(size_t)sizeof(onoff));
#else
	char cmd[256];
	int ret = 0;

	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set RadioOn=%d;", wdev->ifname, onoff);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	wdev->radio->onoff = onoff;
#endif /* NL80211_SUPPORT */

	return WAPP_SUCCESS;
}

int wdev_set_bss_role(struct wapp_dev *wdev, unsigned char map_vendor_extension)
{
	struct ap_dev *ap = NULL;
	BOOLEAN need_disconnect = FALSE;
	u8 new_i_am_fh_bss = 0;
	u8 new_i_am_bh_bss = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wdev) {
		return WAPP_INVALID_ARG;
	}

	ap = (struct ap_dev *)wdev->p_dev;

	new_i_am_fh_bss = (map_vendor_extension & (1<<MAP_ROLE_FRONTHAUL_BSS))?1:0;
	new_i_am_bh_bss = (map_vendor_extension & (1<<MAP_ROLE_BACKHAUL_BSS))?1:0;

	if ((wdev->i_am_fh_bss != new_i_am_fh_bss) || (wdev->i_am_bh_bss != new_i_am_bh_bss))
	{
		need_disconnect = TRUE;
	}

	wdev->i_am_fh_bss = new_i_am_fh_bss;
	wdev->i_am_bh_bss = new_i_am_bh_bss;

	/* This info will be sent back to mapd */
	ap->bss_info.map_role = map_vendor_extension;

	DBGPRINT(RT_DEBUG_ERROR,"[%s] i_am_fh_bss %d , i_am_bh_bss %d, need_disconnect %d\n",
		wdev->ifname, wdev->i_am_fh_bss, wdev->i_am_bh_bss, need_disconnect);

	/* Reset previous values for bh and fh bss */
#if NL80211_SUPPORT
	if (!wdev->i_am_fh_bss)
		wapp_set_fhbss(wapp, (const char *)wdev->ifname,
				(char *)&wdev->i_am_fh_bss, 1);
	if(!wdev->i_am_bh_bss)
		wapp_set_bhbss(wapp, (const char *)wdev->ifname,
				(char *)&wdev->i_am_bh_bss, 1);
	if (wdev->i_am_fh_bss)
		wapp_set_fhbss(wapp, (const char *)wdev->ifname,
				(char *)&wdev->i_am_fh_bss, 1);
	if(wdev->i_am_bh_bss)
		wapp_set_bhbss(wapp, (const char *)wdev->ifname,
				(char *)&wdev->i_am_bh_bss, 1);
	if (need_disconnect)
		wapp_set_DisConnectAllSta(wapp, (const char *)wdev->ifname,
				(char *)&need_disconnect, 1);
#else
	char cmd[256] = {0};
	int ret = 0;

	if (!wdev->i_am_fh_bss) {
		ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set fhbss=0;", wdev->ifname);
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return WAPP_INVALID_ARG;
		}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
		os_memset(cmd, 0, 256);
	}

	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set bhbss=0;", wdev->ifname);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	os_memset(cmd, 0, 256);

	if(wdev->i_am_fh_bss) {
		ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set fhbss=1;", wdev->ifname);
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return WAPP_INVALID_ARG;
		}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	}
	if(wdev->i_am_bh_bss) {
		ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set bhbss=1;", wdev->ifname);
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return WAPP_INVALID_ARG;
		}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	}

	os_memset(cmd, 0, 256);
	if (need_disconnect) {
		ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set DisConnectAllSta=;", wdev->ifname);
		if (os_snprintf_error(sizeof(cmd), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
			return WAPP_INVALID_ARG;
		}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
	}
#endif
	return WAPP_SUCCESS;
}
int wdev_set_hidden_ssid(
	struct wapp_dev *wdev,
	unsigned char hidden_ssid)
{
	char cmd[256] = {0};
	struct ap_dev *ap = NULL;
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wdev) {
		return WAPP_INVALID_ARG;
	}

	ap = (struct ap_dev *)wdev->p_dev;
	ap->bss_info.hidden_ssid = hidden_ssid;

	DBGPRINT(RT_DEBUG_ERROR,
		"set ifname = %s\n"
		"\t hidessid = %d\n",
		wdev->ifname, hidden_ssid);

#if NL80211_SUPPORT
	wapp_set_HideSSID(wapp, (const char *)wdev->ifname,
			(char *)&hidden_ssid, sizeof(hidden_ssid));
#else
	ret = os_snprintf(cmd, sizeof(cmd), "iwpriv %s set HideSSID=%d;", wdev->ifname, hidden_ssid);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);
#endif
	os_memset(cmd, 0, sizeof(cmd));
	ret = os_snprintf(cmd, sizeof(cmd), "wifi_config_save %s HideSSID %d", wdev->ifname, hidden_ssid);
	if (os_snprintf_error(sizeof(cmd), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): os_snprintf fail\n", __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}
	if (system(cmd) == -1)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] (%d): system() call return value is -1\n", __func__, __LINE__);

	return WAPP_SUCCESS;
}
void wdev_he_cap_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_he_cap *he_cap_rcvd)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_he_cap *he_cap;
		he_cap = &ap->he_cap;
		os_memcpy(he_cap, he_cap_rcvd, sizeof(wdev_he_cap));
		DBGPRINT_RAW(RT_DEBUG_OFF,
				"he_cap (%u):\n"
				"\t tx_stream = %u, rx_stream = %u, he_160 = %u, he_8080 = %u, he_mcs_len = %u\n",
				ifindex,
				he_cap->tx_stream,
				he_cap->rx_stream,
				he_cap->he_160,
				he_cap->he_8080,
				he_cap->he_mcs_len)
	}
}

void wdev_tx_pwr_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_tx_power *tx_pwr_rcvd)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_tx_power *tx_pwr;
		tx_pwr = &ap->pwr;
		os_memcpy(tx_pwr, tx_pwr_rcvd, sizeof(wdev_tx_power));

		DBGPRINT_RAW(RT_DEBUG_OFF,
				"op_class: (%u)\n"
				"\t num_of_op_class = %u \n",
				ifindex,
				tx_pwr->num_of_op_class);

		int i = 0;
		for(i = 0; i < tx_pwr->num_of_op_class; i++) {
			printf("\t opClass = %u, power = %u\n", tx_pwr->tx_pwr_limit[i].op_class, tx_pwr->tx_pwr_limit[i].max_pwr);
		}
	}
}

#ifdef MAP_R3_WF6
void print_wf6(wdev_wf6_cap_roles *wf6_cap) {
#if 0
	int i = 0;
	printf("WF6:WAPPD: wf6_cap->role_supp = %d\n", wf6_cap->role_supp);
	for(;i<wf6_cap->role_supp;i++) {
		DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].tx_stream = %d\n", wf6_cap->wf6_role[i].tx_stream);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPDD: wf6_cap->wf6_role[i].rx_stream = %d\n", wf6_cap->wf6_role[i].rx_stream);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].he_mcs_len = %d\n", wf6_cap->wf6_role[i].he_mcs_len);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].su_bf_cap = %d\n", wf6_cap->wf6_role[i].su_bf_cap);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].mu_bf_cap = %d\n", wf6_cap->wf6_role[i].mu_bf_cap);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].dl_mu_mimo_ofdma_cap = %d\n", wf6_cap->wf6_role[i].dl_mu_mimo_ofdma_cap);
		DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].ul_mu_mimo_ofdma_cap = %d\n", wf6_cap->wf6_role[i].ul_mu_mimo_ofdma_cap);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].ul_mu_mimo_cap = %d\n", wf6_cap->wf6_role[i].ul_mu_mimo_cap);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].agent_role = %d\n", wf6_cap->wf6_role[i].agent_role);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPDD: wf6_cap->wf6_role[i].su_beamformee_status = %d\n", wf6_cap->wf6_role[i].su_beamformee_status);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].beamformee_sts_less80 = %d\n", wf6_cap->wf6_role[i].beamformee_sts_less80);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].beamformee_sts_more80 = %d\n", wf6_cap->wf6_role[i].beamformee_sts_more80);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].max_user_dl_tx_mu_mimo = %d\n", wf6_cap->wf6_role[i].max_user_dl_tx_mu_mimo);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].max_user_ul_rx_mu_mimo = %d\n", wf6_cap->wf6_role[i].max_user_ul_rx_mu_mimo);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].max_user_dl_tx_ofdma = %d\n", wf6_cap->wf6_role[i].max_user_dl_tx_ofdma);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].max_user_ul_rx_ofdma =%d\n", wf6_cap->wf6_role[i].max_user_ul_rx_ofdma);
		DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].rts_status = %d\n", wf6_cap->wf6_role[i].rts_status);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].mu_rts_status = %d\n", wf6_cap->wf6_role[i].mu_rts_status);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].m_bssid_status = %d\n", wf6_cap->wf6_role[i].m_bssid_status);
		DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].mu_edca_status = %d\n", wf6_cap->wf6_role[i].mu_edca_status);
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].twt_requester_status = %d\n", wf6_cap->wf6_role[i].twt_requester_status);
		DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].twt_responder_status = %d\n", wf6_cap->wf6_role[i].twt_responder_status);
		DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].ul_ofdma_cap = %d\n", wf6_cap->wf6_role[i].ul_ofdma_cap);
		DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: wf6_cap->wf6_role[i].dl_ofdma_cap = %d\n",wf6_cap->wf6_role[i].dl_ofdma_cap);
	}
#endif
}

void wdev_wf6_cap_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_wf6_cap_roles *wf6_cap_rcvd)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "WF6:WAPPD: DATA coming from driver\n");
	print_wf6(wf6_cap_rcvd);
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	DBGPRINT(RT_DEBUG_TRACE, "WF6:WAPPD:%s handle cap response from driver\n", __func__);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_wf6_cap_roles *wf6_cap;
		wf6_cap = &ap->wf6_cap;
		os_memcpy(wf6_cap, wf6_cap_rcvd, sizeof(wdev_wf6_cap_roles));
#if 0
		DBGPRINT_RAW(RT_DEBUG_OFF,
				"he_cap (%u):\n"
				"\t tx_stream = %u, rx_stream = %u, he_160 = %u, he_8080 = %u, he_mcs_len = %u\n",
				ifindex,
				wf6_cap->tx_stream,
				wf6_cap->rx_stream,
				wf6_cap->he_160,
				wf6_cap->he_8080,
				wf6_cap->he_mcs_len)
#endif
        DBGPRINT(RT_DEBUG_OFF,DPP_MAP_PREX"WF6:WAPPD: Data interprate by WAPP\n");
		print_wf6(&ap->wf6_cap);
	}
}
#endif /*MAP_R3_WF6*/

void wdev_handle_cac_period(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char* buf = NULL;

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	send_pkt_len = sizeof(struct _wapp_cac_info);
	buf = os_zalloc(send_pkt_len);
	if (buf) {
		os_memcpy(buf, &event_data->cac_info, send_pkt_len);
		DBGPRINT(RT_DEBUG_TRACE, "%s cac_enable:%d \n", __func__, event_data->cac_info.ret);
		wapp_send_1905_msg(wapp, WAPP_CAC_PERIOD_ENABLE, send_pkt_len, buf);
		os_free(buf);
	}
}

#ifdef LOW_POWER_SUPPORT
void wdev_handle_no_sta_connect_timeout(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct low_power_status low_power;
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	low_power.status = WAPP_LOW_POWER_NO_STA_CONNECT_TIMEOUT;

	wapp_send_1905_msg(
		wapp,
		WAPP_LOW_POWER_NOTIF,
		sizeof (struct low_power_status),
		(char *) &low_power);
}

void wdev_handle_no_traffic_timeout(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct low_power_status low_power;
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	low_power.status = WAPP_LOW_POWER_NO_DATA_TRAFFIC_TIMEOUT;

	wapp_send_1905_msg(
		wapp,
		WAPP_LOW_POWER_NOTIF,
		sizeof (struct low_power_status),
		(char *) &low_power);
}

void wdev_handle_wifi_open(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct low_power_status low_power;
	if (!wapp)
		return;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	low_power.status = WAPP_LOW_POWER_WIFI_ON;

	wapp_send_1905_msg(
		wapp,
		WAPP_LOW_POWER_NOTIF,
		sizeof (struct low_power_status),
		(char *) &low_power);
}

void wdev_handle_wifi_close(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct low_power_status low_power;
	if (!wapp)
		return;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	low_power.status = WAPP_LOW_POWER_WIFI_OFF;

	wapp_send_1905_msg(
		wapp,
		WAPP_LOW_POWER_NOTIF,
		sizeof (struct low_power_status),
		(char *) &low_power);
}
#endif

#ifdef MAP_R4_SPT
void wdev_handle_uplink_traffic_event(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
#ifdef MAP_R3
	struct os_time now, diff;
#endif /* MAP_R3 */

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
	if(ap == NULL)
		return;
	struct wapp_mesh_sr_info *spt_reuse_info;
	struct uplink_traffic_status up_traffic_status;
	spt_reuse_info = &ap->sr_info;
	if (spt_reuse_info->sr_mode == 0)
		return;

	spt_reuse_info->ul_traffic_status = event_data->mesh_sr_info.ul_traffic_status;
	up_traffic_status.band = *wdev->radio->radio_band;
	up_traffic_status.status = event_data->mesh_sr_info.ul_traffic_status;
	DBGPRINT(RT_DEBUG_TRACE,GRN("handle uplink event")"ifindex %d"
				"band %d status %d \n", ifindex,
				up_traffic_status.band,up_traffic_status.status);
#ifdef MAP_R3
	os_get_time(&now);
	os_time_sub(&now, &wdev->last_sr_time, &diff);
	/* Only send event to map after 1 second */
	if (diff.sec >= 1) {
		wdev->last_sr_time = now;
		DBGPRINT(RT_DEBUG_TRACE, "Sending type=d1 to mapd\n");
		wapp_send_1905_msg(
				wapp,
				WAPP_UPLINK_TRAFFIC_STATUS,
				sizeof(struct uplink_traffic_status),
				(char *) &up_traffic_status);
	}
#endif /* MAP_R3 */
}

void wdev_handle_self_srg_bm_event(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
	if(ap == NULL)
		return;
	struct wapp_mesh_sr_info *spt_reuse_info;
	spt_reuse_info = &ap->sr_info;

	if (spt_reuse_info->sr_mode == 0)
		return;

	os_memcpy(&spt_reuse_info->bm_info, &event_data->mesh_sr_info.bm_info, sizeof(struct
				wapp_srg_bitmap));

	DBGPRINT_RAW(RT_DEBUG_OFF,GRN("wapp bm")\
			"ifindex %u Color:[63_32][0x%4x]-[31_0 [0x%4x] \
			PBssid::[63_32][0x%4x]-[31_0][0x%4x]",
			ifindex,
			spt_reuse_info->bm_info.color_63_32,
			spt_reuse_info->bm_info.color_31_0,
			spt_reuse_info->bm_info.bssid_63_32,
			spt_reuse_info->bm_info.bssid_31_0);

	if(wapp->map->my_map_dev_role == DEVICE_ROLE_CONTROLLER)
		map_send_ch_select_req_info(wapp, wdev);
	else
		map_operating_channel_info(wapp);
}

void wdev_spt_reuse_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, struct wapp_mesh_sr_info *spl_reuse_rcvd)
{
	struct wapp_dev *wdev = NULL;

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		if(ap == NULL)
			return;
		struct wapp_mesh_sr_info *mesh_sr_info;
		mesh_sr_info = &ap->sr_info;
		os_memcpy(mesh_sr_info, spl_reuse_rcvd, sizeof(struct wapp_mesh_sr_info));
#if 1
		DBGPRINT_RAW(RT_DEBUG_TRACE,
				"\n"RED("[color from driver [in wapp]]")"ifindex %u "
				BLUE("Color:[63_32][0x%4X]-[31_0 [0x%4X] ")
				"PBssid::[63_32][0x%4X]-[31_0][0x%4X]\n",
				ifindex,
				mesh_sr_info->bm_info.color_63_32,
				mesh_sr_info->bm_info.color_31_0,
				mesh_sr_info->bm_info.bssid_63_32,
				mesh_sr_info->bm_info.bssid_31_0);

#endif
	}
}
#endif

