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
#include "multi_ap.h"
#include "wifi_utils.h"
#include "p1905_database.h"
#include "p1905_managerd.h"
#include "cmdu_tlv.h"
#include "debug.h"
#include "_1905_interface_ctrl.h"
#include "topology.h"
#include "os.h"
#include "file_io.h"
#ifdef MAP_R3
#include "json.h"
#endif

#define IEEE802_3_GROUP   0x0000
#define IEEE802_11_GROUP  0x0100
#define IEEE1901_GROUP    0x0200
#define MOCA_GROUP        0x0300

extern int _1905_interface_send_int(struct p1905_managerd_ctx *ctx, unsigned char* buf, size_t buf_len, char *dst_dameon);
extern int _1905_interface_send(struct p1905_managerd_ctx *ctx, unsigned char* buf, size_t buf_len, char *dst_dameon);
extern int _1905_interface_recv(struct p1905_managerd_ctx *ctx, unsigned char* buf, int buf_len);
extern void _1905_interface_receive_process(int sock, void *eloop_ctx,
					      void *sock_ctx);
extern int _1905_interface_pending(struct p1905_managerd_ctx *ctx, struct timeval *tv);

unsigned char buf_io[MAX_PKT_LEN];

/*get the bss with same priority or the bss with least priority which is larger that priority(argument 2)*/
struct config_bss_info *get_config_bss_by_priority(struct radio_info *rinfo, unsigned char priority)
{
	unsigned char i = 0, found = 0;
	unsigned char min_prio = 0xFF;
	unsigned char min_prio_index = 0;

	/*get the bss with same priority*/
	for (i = 0; i < rinfo->bss_number; i++) {
		if (rinfo->bss[i].priority == priority) {
			found = 1;
			break;
		}
	}

	if (found == 1)
		return &rinfo->bss[i];

	/*get the bss with least priority which is larger that priority*/
	for (i = 0; i < rinfo->bss_number; i++) {
		if (rinfo->bss[i].priority > priority && rinfo->bss[i].priority < min_prio) {
			min_prio = rinfo->bss[i].priority;
			min_prio_index = i;
			found = 1;
		}
	}

	if (found == 0)
		return NULL;

	return &rinfo->bss[min_prio_index];
}

unsigned char *fill_bss_config(unsigned char *setting, WSC_CONFIG *config, struct config_bss_info *bss)
{
	unsigned char *p = setting;

	memcpy(p, bss->mac, ETH_ALEN);
	p += ETH_ALEN;

	memcpy(p, config->Ssid, 33);
	p += 33;

	*(unsigned short*)p = config->AuthMode;
	p += 2;

	*(unsigned short*)p = config->EncrypType;
	p += 2;

	memcpy(p, config->WPAKey, 65);
	p += 65;

	*p++ = config->map_vendor_extension;

	*p++ = config->hidden_ssid;
#ifdef MAP_R3
	*(unsigned short *)p = config->cred_len;
	p += 2;
	*(unsigned short *)p = config->ext_cred_len;
	p += 2;

	if (config->cred_len) {
		os_memcpy(p, config->cred, config->cred_len);
		debug(DEBUG_TRACE, "cred len %d, JSON:%s\n", config->cred_len, config->cred);
	}
	p += config->cred_len;

	if (config->ext_cred_len) {
		os_memcpy(p, config->ext_cred, config->ext_cred_len);
		debug(DEBUG_TRACE, "ext cred len %d, JSON:%s\n", config->ext_cred_len, config->ext_cred);
	}
	p += config->ext_cred_len;
#endif

	bss->config_status = 1;
#ifdef MAP_R2
	bss->config = *config;
#endif
	return p;
}

int _1905_set_wireless_setting(struct p1905_managerd_ctx* ctx)
{
	unsigned char i = 0, j = 0;
	unsigned char radio_index = 0;
	unsigned char *p = buf_io;
	unsigned char auto_config_num = 0;
	unsigned char priority = 1;
	int len = 0;
	unsigned char *p_content_len = NULL;
	struct config_bss_info *bss = NULL;
	WSC_CONFIG *config = NULL;
	struct _config_buf_ctrl *dst, *dst_n;
	char itf_buff_exist = 0;

	os_memset(buf_io, 0, sizeof(buf_io));

	if (ctx->current_autoconfig_info.radio_index != -1)
		radio_index = (unsigned char)ctx->current_autoconfig_info.radio_index;
	else
		return wapp_utils_error;

	/*reset config_status*/
	for (i = 0; i < ctx->rinfo[radio_index].bss_number; i++)
		ctx->rinfo[radio_index].bss[i].config_status = 0;

	auto_config_num = (ctx->current_autoconfig_info.config_number <=
		ctx->rinfo[radio_index].bss_number) ?
		ctx->current_autoconfig_info.config_number :
		ctx->rinfo[radio_index].bss_number;
	debug(DEBUG_TRACE, AUTO_CONFIG_PREX"total auto-configuration num=%d\n", auto_config_num);

	*(unsigned short*)p = _1905_SET_WIRELESS_SETTING_EVENT;
	p += 2;

	/*mid*/
	p += 2;

	*(unsigned short*)p = sizeof(struct wsc_config);
	p_content_len = p;
	debug(DEBUG_TRACE, "total len %d line %d\n", *(unsigned short*)p_content_len, __LINE__);

	p += 2;
	*(unsigned char*)p = (unsigned char)auto_config_num;
	p += 1;

	/*first fill the bss config by the priority*/
	i = 0;
	while ((bss = get_config_bss_by_priority(&ctx->rinfo[radio_index], priority)) != NULL) {
		priority = bss->priority + 1;
		config = &ctx->current_autoconfig_info.config_data[i];
		p = fill_bss_config(p, config, bss);
		*(unsigned short*)p_content_len += sizeof(struct wireless_setting);
		set_trx_config_for_bss(ctx, config->map_vendor_extension, bss->mac);
		debug(DEBUG_TRACE, "total len %d line %d\n", *(unsigned short*)p_content_len, __LINE__);
#ifdef MAP_R3
		*(unsigned short*)p_content_len += config->cred_len + config->ext_cred_len;
#endif
		debug(DEBUG_TRACE, "total len %d line %d\n", *(unsigned short*)p_content_len, __LINE__);
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"[priority]interface=%s priority=%d config_SSID=%s hidden_ssid=%d\n",
			bss->ifname, bss->priority, config->Ssid, config->hidden_ssid);
		i++;

		if (i >= auto_config_num)
			break;
	}

	/*then fill the rest bss with no priority*/
	j = i;
	if (j < auto_config_num) {
		for(i = 0; i < ctx->rinfo[radio_index].bss_number; i++)
		{
			bss = &ctx->rinfo[radio_index].bss[i];
			if (bss->priority != 0)
				continue;
			config = &ctx->current_autoconfig_info.config_data[j++];

			p = fill_bss_config(p, config, bss);
			*(unsigned short*)p_content_len += sizeof(struct wireless_setting);
			set_trx_config_for_bss(ctx, config->map_vendor_extension, bss->mac);
			debug(DEBUG_TRACE, "total len %d line %d\n", *(unsigned short*)p_content_len, __LINE__);
#ifdef MAP_R3
			*(unsigned short*)p_content_len += config->cred_len + config->ext_cred_len;
#endif
			debug(DEBUG_TRACE, "total len %d line %d\n", *(unsigned short*)p_content_len, __LINE__);
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"[w/o priority]interface=%s priority=%d config_SSID=%s hidden_ssid=%d\n",
				bss->ifname, bss->priority, config->Ssid, config->hidden_ssid);
			if (j >= auto_config_num)
				break;
		}
	}
	len = (int)(p - buf_io);
	debug(DEBUG_INFO, "total len %d line %d\n", *(unsigned short*)p_content_len, __LINE__);

	if (!bss)
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"err!!! no bss, auto_config_num(%d) "
			"band(%d) bss_number(%d)\n",
			auto_config_num,
			ctx->rinfo[radio_index].band,
			ctx->rinfo[radio_index].bss_number);

	if(ctx->renew_ctx.is_renew_ongoing && bss) {
		dl_list_for_each_safe(dst, dst_n, &ctx->config_buffer.list, struct _config_buf_ctrl, list) {
			if(!memcmp(bss->ifname, dst->itf_name, strlen((char *)dst->itf_name))) {
				debug(DEBUG_TRACE, AUTO_CONFIG_PREX"found the same interface in buffer mode, replace it\n");
				itf_buff_exist = 1;
				free(dst->config_buff);
				dst->config_buff = (unsigned char *)malloc(len);
				if (!dst->config_buff) {
					debug(DEBUG_ERROR, AUTO_CONFIG_PREX"alloc dst->config_buff fail\n");
					return wapp_utils_error;
				}
				memset(dst->config_buff, 0, len);
				memcpy(dst->config_buff, buf_io, len);
				dst->len = len;
				break;
			}
		}

		if(!itf_buff_exist) {
			/*alloc a new node and link to the list*/
			dst = (struct _config_buf_ctrl *)malloc(sizeof(*dst));
			if (!dst) {
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX"alloc _config_buf_ctrl fail\n");
				return wapp_utils_error;
			}
			memset(dst, 0, sizeof(*dst));
			dst->itf_name = bss->ifname;
			dst->config_buff = (unsigned char *)malloc(len);
			if (!dst->config_buff) {
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX"alloc config_buff fail\n");
				free(dst);
				return wapp_utils_error;
			}
			memset(dst->config_buff, 0, len);
			memcpy(dst->config_buff, buf_io, len);
			dst->len = len;

			dl_list_add(&ctx->config_buffer.list, &dst->list);
		}
	}else {
		_1905_interface_send(ctx, buf_io, (size_t)len, NULL);
	}

    return wapp_utils_success;
}

int _1905_flash_out_config(struct p1905_managerd_ctx* ctx) {
	struct _config_buf_ctrl *dst, *dst_n;
	debug(DEBUG_OFF, "_1905_flash_out_config ++\n");

	dl_list_for_each_safe(dst, dst_n, &ctx->config_buffer.list, struct _config_buf_ctrl, list) {
		debug(DEBUG_OFF, "itf_name: %s\n", dst->itf_name);
		_1905_interface_send(ctx, dst->config_buff, (size_t)dst->len, NULL);
		free(dst->config_buff);
		dl_list_del(&dst->list);
		free(dst);
		os_sleep(1, 0);
	}

	return wapp_utils_success;
}

int _1905_wait_parse_spec_event(struct p1905_managerd_ctx* ctx, unsigned char* buf, int len, unsigned char event_type, long sec, long usec)
{

	struct timeval tv;
	int ret = 0;
	int res = 0;
	struct os_time now;
	struct os_time started;

	os_memset(&now, 0, sizeof(struct os_time));
	os_memset(&started, 0, sizeof(struct os_time));
	os_get_time(&started);

	tv.tv_sec = sec;
	tv.tv_usec = usec;
	while(1)
	{
		ret = _1905_interface_pending(ctx, &tv);
		if(ret == 1)
		{
			if(_1905_interface_recv(ctx, buf, len) < 0)
			{
				debug(DEBUG_ERROR, "_1905_interface_recv fail\n");
			    return wapp_utils_error;
			}
			else
			{
				res = map_event_handler(ctx, (char* )buf, len, event_type, NULL, 0);
				if (res == 0) {
					return wapp_utils_success;
				} else if (res == -2) {
					return wapp_utils_error;
				}
			}
		}
		else
		{
			debug(DEBUG_ERROR, "wait for event timeout\n");
			return wapp_utils_error;
		}
		os_get_time(&now);
		if (os_reltime_expired(&now, &started, (os_time_t)sec)) {
			debug(DEBUG_ERROR, "wait for event %d timeout\n", event_type);
			return -1;
		}
	}
}


int _1905_event_notify(struct p1905_managerd_ctx *ctx, unsigned short event_id,
	unsigned char *peer_al_mac, unsigned short mid)
{
	unsigned char *p = buf_io;
	int len = 0;
	unsigned short notification_len = 0;

	memset(p, 0, MAX_PKT_LEN);
	*(unsigned short*)p = event_id;
	p += 2;

	*(unsigned short *)p = mid;
	p += 2;

	notification_len = ctx->revd_tlv.buf_len;
	if(peer_al_mac != NULL)
	{
		notification_len += ETH_ALEN;
	}

	*(unsigned short*)p = notification_len;
	p += 2;

	if(peer_al_mac != NULL)
	{
		memcpy(p, peer_al_mac, ETH_ALEN);
		p += ETH_ALEN;
	}

	memcpy(p, ctx->revd_tlv.buf, ctx->revd_tlv.buf_len);
	p += ctx->revd_tlv.buf_len;

	len = (int)(p - buf_io);
	if (_1905_interface_send(ctx, buf_io, len, NULL))
   		return wapp_utils_success;
	else
		return wapp_utils_error;

}

void _1905_set_steering_setting(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_STEERING_REQUEST_EVENT, al_mac, 0);
}

void _1905_set_client_assoc_ctrl(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_CLIENT_ASSOC_CNTRL_SETTING_EVENT, al_mac, 0);
}

void _1905_set_map_policy_config(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_POLICY_CONFIG_REQUEST_EVENT, al_mac, 0);
}

int _1905_notify_ap_metrics_query(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned short mid
#ifdef MAP_R2
, unsigned char periodic
#endif
)
{
	unsigned short event_id = 0;

#ifdef MAP_R2
	if (periodic && ctx->map_version >= DEV_TYPE_R2)
		event_id = _1905_RECV_AP_METRICS_QUERY_PERIODIC_EVENT;
	else
#endif
		event_id = _1905_RECV_AP_METRICS_QUERY_EVENT;

	_1905_event_notify(ctx, event_id, al_mac, mid);

	return 0;
}

int _1905_notify_bh_steering_req(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_BACKHAUL_STEER_REQ_EVENT, al_mac, 0);

	return 0;
}

int _1905_notify_assoc_sta_link_metrics_query(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned short mid)
{
	_1905_event_notify(ctx, _1905_RECV_ASSOC_STA_LINK_METRICS_QUERY_EVENT, al_mac, mid);

	return 0;
}

int _1905_notify_link_metrics_query(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned short mid)
{
	if (_1905_event_notify(ctx, _1905_RECV_LINK_METRICS_QUERY, al_mac, mid) < 0) {
		debug(DEBUG_ERROR, "no daemon attached! no need wait WAPP_LINK_METRICS_RSP_INFO\n");
		return -1;
	}

	return 0;
}

int _1905_notify_unassoc_sta_metrics_query(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	if (_1905_event_notify(ctx, _1905_RECV_UNASSOC_STA_LINK_METRICS_QUERY_EVENT, al_mac, 0) < 0) {
		debug(DEBUG_ERROR, "no daemon attached! no need wait LIB_UNASSOC_STA_LINK_METRICS\n");
		return -1;
	}

	return 0;
}

int _1905_set_radio_tear_down(struct p1905_managerd_ctx* ctx, unsigned char* radio_identifier)
{
	unsigned char buf[256];
	unsigned char* p = buf;
	*(unsigned short*)p = _1905_SET_RADIO_TEARED_DOWN_EVENT;
	p += 2;

	/*mid*/
	p += 2;

	*(unsigned short*)p = 6;
	p += 2;

	memcpy(p, radio_identifier, ETH_ALEN);
	p += ETH_ALEN;
	_1905_interface_send(ctx, buf, (size_t)(p - buf), NULL);
	return 0;
}

void _1905_notify_beacon_metrics_query(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_BEACON_METRICS_QUERY_EVENT, al_mac, 0);
}

void _1905_notify_controller_found(struct p1905_managerd_ctx* ctx)
{
	unsigned char buf[256];
	unsigned char* p = buf;
	int i = 0;

	*(unsigned short*)p = _1905_MAP_CONTROLLER_FOUND_EVENT;
	p += 2;

	/*mid*/
	p += 2;

	*(unsigned short*)p = 24;
	p += 2;

	*p++ = ctx->cinfo.supported_freq;

	for(i = 0; i < ctx->itf_number; i++)
	{
		if(!memcmp(ctx->cinfo.local_ifname, ctx->itf[i].if_name, IFNAMSIZ))
		{
			if(ctx->itf[i].media_type == IEEE802_3_GROUP)
			{
				*p++ = 0;
			}
			else if((ctx->itf[i].media_type & IEEE802_11_GROUP))
			{
				*p++ = 1;
			}
			memcpy(p, ctx->itf[i].if_name, IFNAMSIZ);
			p += IFNAMSIZ;
			memcpy(p, ctx->itf[i].mac_addr, ETH_ALEN);
			p += ETH_ALEN;
			break;
		}
	}
	_1905_interface_send(ctx, buf, (size_t)(p - buf), NULL);
}

int _1905_notify_channel_selection_req(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned short mid)
{

	_1905_event_notify(ctx, _1905_RECV_CHANNEL_SELECTION_REQ_EVENT, al_mac, mid);

	return 0;
}

int _1905_notify_channel_preference_query(struct p1905_managerd_ctx *ctx,
	unsigned char *al_mac, unsigned short mid)
{
	unsigned char *p = buf_io;
	unsigned char i = 0;
	*(unsigned short*)p = _1905_RECV_CHANNEL_PREFERENCE_QUERY_EVENT;
	p += 2;

	*(unsigned short *)p = mid;
	p += 2;

	*(unsigned short*)p = ETH_ALEN + 1 + ctx->radio_number * ETH_ALEN;
	p += 2;

	memcpy(p, al_mac, ETH_ALEN);
	p += ETH_ALEN;

	*p++ = ctx->radio_number;

	for(i = 0; i < ctx->radio_number; i++)
	{
		memcpy(p, ctx->rinfo[i].identifier, ETH_ALEN);
		p += ETH_ALEN;
	}
	if (_1905_interface_send(ctx, buf_io, (size_t)(p - buf_io), NULL) == 0) {
		debug(DEBUG_ERROR, "no daemon attached! no need wait WAPP_CHANNEL_PREFERENCE_REPORT_INFO\n");
		return -1;
	}

	return 0;
}

void _1905_notify_channel_preference_report(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_CHANNEL_PREFERENCE_REPORT_EVENT, al_mac, 0);
}

void _1905_notify_client_steering_btm_report(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_CLI_STEER_BTM_REPORT_EVENT, al_mac, 0);
}

void _1905_notify_steering_complete(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_STEER_COMPLETE_EVENT, al_mac, 0);
}

void _1905_notify_ap_metrics_response(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_AP_METRICS_RSP_EVENT, al_mac, 0);
}

void _1905_notify_link_metrics_response(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_LINK_METRICS_RSP_EVENT, al_mac, 0);
}

void _1905_notify_assoc_sta_link_metric_rsp(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_ASSOC_STA_LINK_METRICS_RSP_EVENT, al_mac, 0);
}

void _1905_notify_unassoc_sta_link_metric_rsp(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_UNASSOC_STA_LINK_METRICS_RSP_EVENT, al_mac, 0);
}

void _1905_notify_bcn_metric_rsp(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_BCN_METRICS_RSP_EVENT, al_mac, 0);
}

void _1905_notify_bh_steering_rsp(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_BACKHAUL_STEER_RSP_EVENT, al_mac, 0);
}

void _1905_notify_combined_infra_metrics(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_COMBINED_INFRASTRUCTURE_METRICS_EVENT, al_mac, 0);
}

void _1905_notify_vendor_specific_message(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_VENDOR_SPECIFIC_MESSAGE_EVENT, al_mac, 0);
}

void _1905_notify_topology_rsp_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac, unsigned char* local_if_mac)
{
	/*insert the local interface mac address to the almac*/
	memcpy(buf_io, ctx->revd_tlv.buf, ctx->revd_tlv.buf_len);
	memcpy(ctx->revd_tlv.buf, local_if_mac, ETH_ALEN);
	memcpy(ctx->revd_tlv.buf + ETH_ALEN, buf_io, ctx->revd_tlv.buf_len);
	ctx->revd_tlv.buf_len += ETH_ALEN;
	_1905_event_notify(ctx, _1905_RECV_TOPOLOGY_RSP_EVENT, al_mac, 0);
}

void _1905_notify_renew_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac, unsigned char renew_band)
{
	unsigned char *p = buf_io;

	*((unsigned short*)p) = _1905_AUTOCONFIG_RENEW_EVENT;
	p += 2;

	/*mid*/
	p += 2;

	*((unsigned short*)p) = 7;
	p += 2;

	memcpy(p, al_mac, ETH_ALEN);
	p += ETH_ALEN;

	*p++ = renew_band;

	_1905_interface_send(ctx, buf_io, (size_t)(p - buf_io), NULL);
}

void _1905_notify_autoconfig_search_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_AUTOCONFIG_SEARCH_EVENT, al_mac, 0);
}

void _1905_notify_autoconfig_rsp_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_AUTOCONFIG_RSP_EVENT, al_mac, 0);
}

void _1905_notify_topology_notification_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_TOPOLOGY_NOTIFICATION_EVENT, al_mac, 0);
}

void _1905_notify_ap_capability_report_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_AP_CAPABILITY_REPORT_EVENT, al_mac, 0);
}

void _1905_notify_ch_selection_rsp_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_CH_SELECTION_RSP_EVENT, al_mac, 0);
}

void _1905_notify_operating_channel_report_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_OPERATING_CH_REPORT_EVENT, al_mac, 0);
}

void _1905_notify_client_capability_report_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_CLIENT_CAPABILITY_REPORT_EVENT, al_mac, 0);
}

void _1905_notify_higher_layer_data_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_HIGHER_LAYER_DATA_EVENT, al_mac, 0);
}

void _1905_notify_raw_data(struct p1905_managerd_ctx *ctx, unsigned char *buf, size_t len)
{
	_1905_interface_send(ctx, buf, len, NULL);
}

void _1905_notify_combined_infrastructure_metrics_query(struct p1905_managerd_ctx* ctx,
	unsigned char* al_mac, unsigned char *peer_almac)
{
	unsigned char *p = buf_io;

	*((unsigned short*)p) = _1905_RECV_COMBINED_INFRASTRUCTURE_METRICS_QUERY_EVENT;
	p += 2;

	/*mid*/
	p += 2;

	*((unsigned short*)p) = 12;
	p += 2;

	memcpy(p, al_mac, ETH_ALEN);
	p += ETH_ALEN;

	memcpy(p, peer_almac, ETH_ALEN);
	p += ETH_ALEN;

	_1905_interface_send(ctx, buf_io, (size_t)(p - buf_io), NULL);

	if (ctx->MAP_Cer) {
		ctx->dev_send_1905_mid = ctx->mid + 1;

		if (_1905_write_mid("/tmp/msg_id.txt", ctx->dev_send_1905_mid) < 0) {
			debug(DEBUG_ERROR, "update mid=0x%04x to dev_send_1905_mid.txt fail\n", ctx->dev_send_1905_mid);
		} else {
			debug(DEBUG_TRACE, "update mid=0x%04x to dev_send_1905_mid.txt success\n", ctx->dev_send_1905_mid);
		}

		if (_1905_wait_parse_spec_event(ctx, buf_io, sizeof(buf_io), LIB_1905_CMDU_REQUEST, 5, 1) < 0) {
			debug(DEBUG_ERROR, "wait event LIB_1905_CMDU_REQUEST fail\n");
			return;
	    }
	}
}

void _1905_notify_switch_status(struct p1905_managerd_ctx* ctx, unsigned char status)
{
	unsigned char *p = buf_io;

	*((unsigned short*)p) = _1905_SWITCH_STATUS;
	p += 2;

	/*mid*/
	p += 2;

	*((unsigned short*)p) = 1;
	p += 2;

	*p++ = status;

	_1905_interface_send(ctx, buf_io, (size_t)(p - buf_io), NULL);
}

int _1905_notify_error_code_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_ERROR_CODE_EVENT, al_mac, 0);

	return 0;
}

extern	struct config_bss_info *get_config_bss_by_priority(struct radio_info *rinfo, unsigned char priority);
extern 	unsigned char *fill_bss_config(unsigned char *setting, WSC_CONFIG *config, struct config_bss_info *bss);

#ifdef MAP_R3
unsigned short build_config_obj_attr_per_radio(struct p1905_managerd_ctx* ctx,
	struct radio_info *rinfo, unsigned char bss_index, char *json, unsigned char bss_type, int json_len)
{
	unsigned short per_obj_len = 0;
	unsigned short authmode = 0;
	char pass[63 * 6 + 1];
	char ssid[32 + 1];
	char identifier_str[20] = {0};
	char authStr[32 + 1];
	unsigned char bss_found = 0;
	struct bss_connector_item *bc_item =NULL, *bc_item_nxt = NULL;
	int connector_found = 0;
	unsigned char all_zero_mac[ETH_ALEN] = {0};

	if (!ctx || !rinfo || !json) {
		debug(DEBUG_ERROR, "NULL pointer\n");
		return 0;
	}

	os_memset(all_zero_mac, 0, sizeof(all_zero_mac));

	dl_list_for_each_safe(bc_item, bc_item_nxt,
		&ctx->r3_dpp.bss_connector_list, struct bss_connector_item, member) {
		if ((!os_memcmp(bc_item->almac, all_zero_mac, ETH_ALEN)) &&
			(!os_memcmp(bc_item->mac_apcli, all_zero_mac, ETH_ALEN))) {
			connector_found = 1;
			break;
		}
	}

	if (!connector_found) {
		debug(DEBUG_ERROR, "not found self bss connector\n");
		return 0;
	}


	per_obj_len = 0;

	if (os_strlen(ctx->bss_config[bss_index].ssid))
		bss_found = 1;

	if (bss_type & BIT_BH_BSS) {
		/* wi-fi tech: map */
		per_obj_len += snprintf(json + per_obj_len, json_len - per_obj_len, "{\"wi-fi_tech\":\"map\",\"discovery\":");
		debug(DEBUG_TRACE, "build config obj for BH\n");
	}
	else {
		/* wi-fi tech: inframap */
		per_obj_len += snprintf(json+per_obj_len, json_len - per_obj_len,
								"{\"wi-fi_tech\":\"inframap\",\"discovery\":");
		debug(DEBUG_TRACE, "build config obj for FH\n");
	}

	/* Radio identifier */
	wpa_snprintf_hex(identifier_str, sizeof(identifier_str), rinfo->identifier, ETH_ALEN);
	per_obj_len += snprintf(json+per_obj_len, json_len - per_obj_len, "{\"RUID\":\"%s\"", identifier_str);

	json_escape_string(ssid, sizeof(ssid),
		(const char *)ctx->bss_config[bss_index].ssid,
		os_strlen(ctx->bss_config[bss_index].ssid));

	if (bss_found)
		per_obj_len += snprintf(json+per_obj_len, json_len - per_obj_len, ",\"ssid\":\"%s\"", ssid);
	else
		per_obj_len += snprintf(json+per_obj_len, json_len - per_obj_len, ",\"ssid\":\"\"");

	/*
	** BSSID value only used by reconfiguration
	** ignore here
	*/
	per_obj_len += snprintf(json+per_obj_len, json_len - per_obj_len, ",\"BSSID\":\"000000000000\"");
	per_obj_len += snprintf(json+per_obj_len, json_len - per_obj_len, "}");

	os_memset(authStr, 0, sizeof(authStr));
	/* CERT mode, authmode from wts file */
	if (ctx->bss_config[bss_index].authmode & AUTH_DPP_ONLY)
		authmode |= AUTH_DPP_ONLY;
	if (ctx->bss_config[bss_index].authmode & AUTH_SAE_PERSONAL)
		authmode |= AUTH_SAE_PERSONAL;
	if (ctx->bss_config[bss_index].authmode & AUTH_WPA2_PERSONAL)
		authmode |= AUTH_WPA2_PERSONAL;

	if ((authmode & AUTH_DPP_ONLY) &&
		(authmode & AUTH_SAE_PERSONAL) &&
		(authmode & AUTH_WPA2_PERSONAL)) {
		os_snprintf(authStr, sizeof(authStr), "dpp+psk+sae");
	}
	else if ((authmode & AUTH_DPP_ONLY) &&
			 (authmode & AUTH_SAE_PERSONAL)) {
		os_snprintf(authStr, sizeof(authStr), "dpp+sae");
	}
	else if ((authmode & AUTH_DPP_ONLY) &&
			 (authmode & AUTH_WPA2_PERSONAL)) {
		os_snprintf(authStr, sizeof(authStr), "dpp+psk");
	}
	else if (authmode & AUTH_DPP_ONLY) {
		os_snprintf(authStr, sizeof(authStr), "dpp");
	}
	else if (authmode & AUTH_SAE_PERSONAL) {
		os_snprintf(authStr, sizeof(authStr), "sae");
	}
	else if (authmode & AUTH_WPA2_PERSONAL) {
		os_snprintf(authStr, sizeof(authStr), "psk");
	}

	/* akm:  */
	per_obj_len += snprintf(json+per_obj_len, json_len - per_obj_len, ",\"cred\":{\"akm\":\"%s\"", authStr);

	/* pass: WPA2 Passphrase */
	json_escape_string(pass, sizeof(pass),
		(const char *)ctx->bss_config[bss_index].key,
		os_strlen(ctx->bss_config[bss_index].key));
	per_obj_len += snprintf(json+per_obj_len, json_len - per_obj_len, ",\"pass\":\"%s\"", pass);

	/* add pre-shared key but no value
	per_obj_len += sprintf(json+per_obj_len, ",\"psk_hex\":\"\"");*/

	/* connector for BH or FH */
	if (bss_type & BIT_BH_BSS) {
		per_obj_len += snprintf(json+per_obj_len, json_len - per_obj_len, ",%s", bc_item->bh_connector);
	}
	else {
		per_obj_len += snprintf(json+per_obj_len, json_len - per_obj_len, ",%s", bc_item->fh_connector);
	}
	per_obj_len += snprintf(json+per_obj_len, json_len - per_obj_len, "}");
	per_obj_len += snprintf(json+per_obj_len, json_len - per_obj_len, "}");

	if (per_obj_len > 1024)
		per_obj_len = 1024;

	return per_obj_len;
}
#endif

#ifdef MAP_R3
void update_r3_cred(struct p1905_managerd_ctx *ctx, struct radio_info *rinfo,
	unsigned char bss_index, PWSC_CONFIG config)
{
	char obj_json[2048] = {0};
	int obj_json_len = sizeof(obj_json);
	unsigned short obj_len = 0;
	unsigned char bss_type = 0;
	if ((ctx->map_version == MAP_PROFILE_R3) && (config->AuthMode & AUTH_DPP_ONLY)) {
		obj_len = 0;
		os_memset(obj_json, 0, sizeof(obj_json));
		if ((config->map_vendor_extension & BIT_BH_BSS) &&
			(config->map_vendor_extension & BIT_FH_BSS)) {

			debug(DEBUG_TRACE, "build config obj for both BH and FH\n");
			obj_len = build_config_obj_attr_per_radio(ctx, rinfo, bss_index, obj_json, BIT_BH_BSS, obj_json_len);
			if (obj_len) {
				config->cred_len = obj_len;
				os_memcpy(config->cred, obj_json, obj_len);
				debug(DEBUG_TRACE, "cred len %d\n", obj_len);
			}
			obj_len = 0;
			os_memset(obj_json, 0, sizeof(obj_json));
			obj_len = build_config_obj_attr_per_radio(ctx, rinfo, bss_index, obj_json, BIT_FH_BSS, obj_json_len);
			if (obj_len) {
				config->ext_cred_len = obj_len;
				os_memcpy(config->ext_cred, obj_json, obj_len);
				debug(DEBUG_TRACE, "ext cred len %d\n", obj_len);
			}
		}
		else {
			if (config->map_vendor_extension & BIT_BH_BSS) {
				bss_type |= BIT_BH_BSS;
				debug(DEBUG_TRACE, "only build config obj for BH\n");
			}
			else {
				bss_type |= BIT_FH_BSS;
				debug(DEBUG_TRACE, "only build config obj for FH\n");
			}
			obj_len = build_config_obj_attr_per_radio(ctx, rinfo, bss_index, obj_json, bss_type, obj_json_len);
			if (obj_len) {
				config->cred_len = obj_len;
				os_memcpy(config->cred, obj_json, obj_len);
				debug(DEBUG_TRACE, "cred len %d\n", obj_len);
			}
		}
	}
}
#endif

int _1905_update_bss_info_per_radio(struct p1905_managerd_ctx *ctx,
	struct radio_info *rinfo)
{
	unsigned char i = 0, j = 0, ji = 0, sum = 0;
	unsigned char config_intf_num_per_radio = 0, bss_config_num_per_radio = 0, auto_config_num = 0;
	unsigned char *p = buf_io;
	unsigned char *p_len = NULL;
	int len = 0;
	unsigned char wild_card_mac[ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char band = BAND_INVALID_CAP;
	struct config_bss_info *bss;
	unsigned char priority = 1;
	WSC_CONFIG config;
	int ret;

	os_memset(&config, 0, sizeof(config));
	config_intf_num_per_radio = rinfo->bss_number;

	debug(DEBUG_TRACE, "band_cap=%02x\n", rinfo->band);

	if (rinfo->band == BAND_INVALID_CAP) {
		debug(DEBUG_ERROR, "invalid radio cap; no need config this radio\n");
		return wapp_utils_error;
	}

	band = determin_band_config(ctx, ctx->p1905_al_mac_addr, rinfo->band);
	if (band == BAND_INVALID_CAP) {
		debug(DEBUG_OFF, "no bss config info for this radio."
			"need send tear down event to this radio("MACSTR")\n", PRINT_MAC(rinfo->identifier));
		_1905_set_radio_tear_down(ctx, rinfo->identifier);
		return wapp_utils_success;
	}

	debug(DEBUG_TRACE, "band=%d\n", band);
	for (i = 0; i < ctx->bss_config_num; i++) {
		if (band == ctx->bss_config[i].band_support &&
			(!memcmp(ctx->p1905_al_mac_addr, ctx->bss_config[i].mac, ETH_ALEN) ||
				!memcmp(ctx->bss_config[i].mac, wild_card_mac, ETH_ALEN))) {
				bss_config_num_per_radio++;
		}
	}
	debug(DEBUG_TRACE, "bss_config_num_per_radio=%d\n", bss_config_num_per_radio);

	if (bss_config_num_per_radio == 0) {
		debug(DEBUG_OFF, "no bss config info for this device on this radio."
			" need send tear down event to this radio\n");
		_1905_set_radio_tear_down(ctx, rinfo->identifier);
		return wapp_utils_success;
	}

	auto_config_num = (config_intf_num_per_radio <= bss_config_num_per_radio) ?
		config_intf_num_per_radio : bss_config_num_per_radio;
	debug(DEBUG_TRACE, "total autoconf num=%d\n", auto_config_num);

	*(unsigned short*)p = _1905_SET_WIRELESS_SETTING_EVENT;
	p += 2;

	/*mid*/
	p += 2;

	*(unsigned short*)p = sizeof(struct wsc_config);
	p_len = p;
	debug(DEBUG_TRACE, "total len %d line %d\n", *(unsigned short*)p_len, __LINE__);

	p += 2;
	*(unsigned char*)p = (unsigned char)auto_config_num;
	p += 1;

	/*first fill the bss config by the priority*/
	i = 0;
	while ((bss = get_config_bss_by_priority(rinfo, priority)) != NULL) {
		priority = bss->priority + 1;
		for (j = ji; j < ctx->bss_config_num; j++) {
			if (band == ctx->bss_config[j].band_support &&
				(!memcmp(ctx->p1905_al_mac_addr, ctx->bss_config[j].mac, ETH_ALEN) ||
					!memcmp(ctx->bss_config[j].mac, wild_card_mac, ETH_ALEN))) {
				ji = j + 1;
				os_memset(&config, 0, sizeof(config));
				config.AuthMode = ctx->bss_config[j].authmode;
#ifdef MAP_R2
				if (ctx->map_version < MAP_PROFILE_R3)
					config.AuthMode &= ~AUTH_DPP_ONLY;
#endif
				config.EncrypType = ctx->bss_config[j].encryptype;
				config.map_vendor_extension = ctx->bss_config[j].wfa_vendor_extension;
				config.hidden_ssid = ctx->bss_config[j].hidden_ssid;
				ret = os_snprintf((char *)config.Ssid, sizeof(config.Ssid), "%s", ctx->bss_config[j].ssid);
				if (os_snprintf_error(sizeof(config.Ssid), ret)) {
					debug(DEBUG_ERROR, "[%d]snprintf error!\n", __LINE__);
					return wapp_utils_error;
				}
				os_memcpy(config.WPAKey, ctx->bss_config[j].key, 65);
#ifdef MAP_R3
				update_r3_cred(ctx, rinfo, j, &config);
				*(unsigned short*)p_len += config.cred_len + config.ext_cred_len;
				debug(DEBUG_TRACE, "total len %d line %d\n", *(unsigned short*)p_len, __LINE__);
#endif
				*(unsigned short*)p_len += sizeof(struct wireless_setting);
				debug(DEBUG_TRACE, "total len %d line %d\n", *(unsigned short*)p_len, __LINE__);
				break;
			}
		}
		p = fill_bss_config(p, &config, bss);
		set_trx_config_for_bss(ctx, config.map_vendor_extension, bss->mac);
		debug(DEBUG_TRACE, "interface=%s priority=%d config_SSID=%s, hidden_ssid=%d\n",
			bss->ifname, bss->priority, config.Ssid, config.hidden_ssid);
		i++;

		if (i >= auto_config_num)
			break;
	}

	/*then fill the rest bss with no priority*/
	sum = i;
	if (sum < auto_config_num) {
		for(i = 0; i < rinfo->bss_number; i++)
		{
			bss = &rinfo->bss[i];
			if (bss->priority != 0)
				continue;
			for (j = ji; j < ctx->bss_config_num; j++) {
				if (band == ctx->bss_config[j].band_support &&
					(!memcmp(ctx->p1905_al_mac_addr, ctx->bss_config[j].mac, ETH_ALEN) ||
					!memcmp(ctx->bss_config[j].mac, wild_card_mac, ETH_ALEN))) {
						ji = j + 1;
						os_memset(&config, 0, sizeof(config));
						config.AuthMode = ctx->bss_config[j].authmode;
#ifdef MAP_R2
						if (ctx->map_version < MAP_PROFILE_R3)
							config.AuthMode &= ~AUTH_DPP_ONLY;
#endif
						config.EncrypType = ctx->bss_config[j].encryptype;
						config.map_vendor_extension = ctx->bss_config[j].wfa_vendor_extension;
						config.hidden_ssid = ctx->bss_config[j].hidden_ssid;
						ret = snprintf((char *)config.Ssid, sizeof(config.Ssid), "%s", ctx->bss_config[j].ssid);
						if (os_snprintf_error(sizeof(config.Ssid), ret)) {
							debug(DEBUG_ERROR, "[%d]snprintf error!\n", __LINE__);
							return wapp_utils_error;
						}
						os_memcpy(config.WPAKey, ctx->bss_config[j].key, 65);
#ifdef MAP_R3
						update_r3_cred(ctx, rinfo, j, &config);
						*(unsigned short*)p_len += config.cred_len + config.ext_cred_len;
						debug(DEBUG_TRACE, "total len %d line %d\n", *(unsigned short*)p_len, __LINE__);
#endif
						*(unsigned short*)p_len += sizeof(struct wireless_setting);
						debug(DEBUG_TRACE, "total len %d line %d\n", *(unsigned short*)p_len, __LINE__);
						break;
				}
			}
			sum++;
			p = fill_bss_config(p, &config, bss);
			set_trx_config_for_bss(ctx, config.map_vendor_extension, bss->mac);
			debug(DEBUG_TRACE, "interface=%s priority=%d config_SSID=%s, hidden_ssid=%d\n",
				bss->ifname, bss->priority, config.Ssid, config.hidden_ssid);
			if (sum >= auto_config_num)
				break;
		}
	}

	debug(DEBUG_TRACE, "config bss number(%d)\n", auto_config_num);
	debug(DEBUG_TRACE, "total len %d line %d\n", *(unsigned short*)p_len, __LINE__);

	len = (int)(p - buf_io);
	_1905_interface_send(ctx, buf_io, (size_t)len, NULL);

	return wapp_utils_success;
}

#ifdef MAP_R2

/*channel scan feature*/
int _1905_notify_ch_scan_req(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_CHANNEL_SCAN_REQ_EVENT, al_mac, 0);

	return 0;
}

int _1905_notify_ch_scan_rep(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_CHANNEL_SCAN_REP_EVENT, al_mac, 0);

	return 0;
}


int _1905_notify_tunneled_msg(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_TUNNELED_MESSAGE_EVENT, al_mac, 0);

	return 0;
}


int _1905_notify_assoc_status_notification_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_ASSOCIATION_STATUS_NOTIFICATION_EVENT, al_mac, 0);

	return 0;
}


int _1905_notify_cac_request_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_CAC_REQUEST_EVENT, al_mac, 0);

	return 0;
}


int _1905_notify_cac_terminate_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_CAC_TERMINATE_EVENT, al_mac, 0);

	return 0;
}


int _1905_notify_client_disassociation_stats_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_CLIENT_DISASSOCIATION_STATS_EVENT, al_mac, 0);

	return 0;
}

int _1905_notify_backhaul_sta_cap_report_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_BACKHAUL_STA_CAP_REPORT_EVENT, al_mac, 0);

	return 0;
}


int _1905_notify_failed_connection_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_FAILED_CONNECTION_EVENT, al_mac, 0);

	return 0;
}

#define VLAN_N_VID		4095
unsigned short get_vid_from_ssid(char *ssid, unsigned char num,
	struct ssid_2_vid_mapping *mapping)
{
	unsigned char i = 0;

	for (i = 0; i < num && mapping; i++) {
		if (!os_strncmp(ssid, mapping[i].ssid, 32))
			return mapping[i].vlan_id;
	}

	return VLAN_N_VID;

}
void _1905_notify_ts_setting(struct p1905_managerd_ctx *ctx, unsigned char num,
	struct ssid_2_vid_mapping *mapping)
{
	struct ts_setting *setting = NULL;
	unsigned char *p = buf_io;
	unsigned char i = 0, j = 0, fh_count = 0;
	struct config_bss_info *bss = NULL;
	unsigned short vid = 0;
	unsigned short len = 0;

	os_memset(buf_io, 0, sizeof(buf_io));
	*(unsigned short*)p = _1905_SET_TRAFFIC_SEPARATION_SETTING_EVENT;
	p += 2;

	/*mid*/
	p += 2;

	/*add common traffic separation common setting: primary vid/pcp, all policy*/
	setting = (struct ts_setting*)(p + 2);
	setting->common_setting.primary_vid = ctx->map_policy.setting.primary_vid;
	setting->common_setting.primary_pcp = ctx->map_policy.setting.dft.PCP;

	for (i = 0; i < num && mapping; i++)
		setting->common_setting.policy_vids[i] = mapping[i].vlan_id;
	setting->common_setting.policy_vid_num = num;

	/*add fh vid for a specific fh bss*/
	for (i = 0; i < ctx->radio_number; i++) {
		for (j = 0; j < ctx->rinfo[i].bss_number; j++) {
			bss = &ctx->rinfo[i].bss[j];

			debug(DEBUG_ERROR, "%s bss->config_status=%d\n", bss->ifname, bss->config_status);
			if (bss->config_status == 0)
				continue;

			vid = get_vid_from_ssid((char*)bss->config.Ssid, num, mapping);
//			if (vid == VLAN_N_VID)
//				continue;

			/*fronthaul*/
			if (bss->config.map_vendor_extension & BIT_FH_BSS) {
				os_memcpy(setting->fh_bss_setting.fh_configs[fh_count].itf_mac, bss->mac, ETH_ALEN);
				setting->fh_bss_setting.fh_configs[fh_count].vid = vid;
				debug(DEBUG_ERROR, "%s vid=%d\n", bss->ifname, vid);
				fh_count++;
			}
		}
	}
	setting->fh_bss_setting.itf_num = fh_count;
	debug(DEBUG_ERROR, "fh_count=%d\n", fh_count);
	len = sizeof(struct ts_setting) + fh_count * sizeof(struct ts_fh_config);
	*(unsigned short*)p = len;

	_1905_interface_send(ctx, buf_io, len + 4, NULL);
}

void _1905_notify_transparent_vlan_setting(struct p1905_managerd_ctx *ctx)
{
	struct trans_vlan_setting *trans_setting = NULL;
	unsigned char *p = buf_io;
	unsigned char i = 0, if_cnt = 0;

	os_memset(buf_io, 0, sizeof(buf_io));
	*(unsigned short*)p = _1905_SET_TRANSPARENT_VLAN_SETTING_EVENT;
	p += 2;

	/*mid*/
	p += 2;

	trans_setting = (struct trans_vlan_setting*)(p + 2);

	trans_setting->trans_vlan_configs.trans_vid_num = ctx->map_policy.trans_vlan.num;
	os_memcpy(trans_setting->trans_vlan_configs.vids,
		ctx->map_policy.trans_vlan.transparent_vids, sizeof(ctx->map_policy.trans_vlan.transparent_vids));
	debug(DEBUG_ERROR, "transparent vid number=%d\n", trans_setting->trans_vlan_configs.trans_vid_num);
	for (i = 0; i < ctx->itf_number; i++) {
		if (ctx->itf[i].media_type >= IEEE802_11_GROUP &&
			ctx->itf[i].media_type <= IEEE802_11AX_GROUP) {
			os_memcpy(&trans_setting->apply_itf_mac[if_cnt * ETH_ALEN], ctx->itf[i].mac_addr, ETH_ALEN);
			if_cnt++;
		}
	}

	if (if_cnt == 0) {
		debug(DEBUG_ERROR, "interface count equals to 0, return!!\n");
		return;
	}

	trans_setting->apply_itf_num = if_cnt;
	debug(DEBUG_ERROR, "transparent vid applied interface number=%d\n", trans_setting->apply_itf_num);
	*(unsigned short*)p = sizeof(struct trans_vlan_setting) + trans_setting->apply_itf_num * ETH_ALEN;

	_1905_interface_send(ctx, buf_io,
		sizeof(struct trans_vlan_setting) + trans_setting->apply_itf_num * ETH_ALEN + 4, NULL);
}

void map_r2_notify_ts_cap(struct p1905_managerd_ctx *ctx, unsigned char *almac,
	struct ts_cap_db *ts_cap)
{
	unsigned char *p = buf_io;

	if (!ctx || !almac || !ts_cap) {
		debug(DEBUG_ERROR, "invalid param! ctx(%p) almac(%p) ts_cap(%p) is null\n",
			ctx, almac, ts_cap);
		return;
	}

	os_memset(buf_io, 0, sizeof(buf_io));
	*(unsigned short*)p = _1905_SET_AP_RADIO_ADVANCED_CAPABILITIES_EVENT;
	p += 2;

	/*mid*/
	p += 2;

	*(unsigned short*)p = 8;
	p += 2;

	os_memcpy(p, almac, ETH_ALEN);
	p += ETH_ALEN;
	os_memcpy(p, ts_cap->identifier, ETH_ALEN);
	p += ETH_ALEN;
	*p++ = ts_cap->ts_combined_fh;
	*p++ = ts_cap->ts_combined_bh;

	_1905_interface_send(ctx, buf_io, 12, NULL);
}
#endif // #ifdef MAP_R2

#ifdef MAP_R3
void map_notify_standard_sp_rules(struct p1905_managerd_ctx *ctx,
	unsigned char *almac, unsigned char *buf, unsigned short len)
{
	unsigned char *p = buf_io;

	if (!ctx || !almac || !buf) {
		debug(DEBUG_ERROR, "invalid param! ctx(%p) almac(%p) buf(%p) null\n",
			ctx, almac, buf);
		return;
	}

	os_memset(buf_io, 0, sizeof(buf_io));
	*(unsigned short*)p = _1905_SET_SP_STANDARD_RULE;
	p += 2;

	/*mid*/
	p += 2;

	*(unsigned short*)p = len + ETH_ALEN;
	p += 2;

	os_memcpy(p, almac, ETH_ALEN);
	p += ETH_ALEN;
	os_memcpy(p, buf, len);
	p += len;

	_1905_interface_send(ctx, buf_io, len + ETH_ALEN + 4, NULL);

}

void map_notify_akm_suit_cap(struct p1905_managerd_ctx *ctx, unsigned char *almac,
	unsigned char *buf, unsigned short len)
{
	unsigned char *p = buf_io;

	if (!ctx || !almac || !buf) {
		debug(DEBUG_ERROR, "invalid param! ctx(%p) almac(%p) buf(%p) null\n",
			ctx, almac, buf);
		return;
	}

	os_memset(buf_io, 0, sizeof(buf_io));
	*(unsigned short*)p = _1905_SET_AKM_SUIT_CAP_EVENT;
	p += 2;

	/*mid*/
	p += 2;

	*(unsigned short*)p = len + ETH_ALEN;
	p += 2;

	os_memcpy(p, almac, ETH_ALEN);
	p += ETH_ALEN;
	os_memcpy(p, buf, len);
	p += len;

	_1905_interface_send(ctx, buf_io, len + ETH_ALEN + 4, NULL);
}

void map_notify_1905_secure_cap(struct p1905_managerd_ctx *ctx, unsigned char *almac,
	unsigned char *buf, unsigned short len) {
	unsigned char *p = buf_io;

	if (!ctx || !almac || !buf) {
		debug(DEBUG_ERROR, "invalid param! ctx(%p) almac(%p) buf(%p) null\n",
			ctx, almac, buf);
		return;
	}

	os_memset(buf_io, 0, sizeof(buf_io));
	*(unsigned short*)p = _1905_SET_1905_SECURE_CAP_EVENT;
	p += 2;

	/*mid*/
	p += 2;

	*(unsigned short*)p = len + ETH_ALEN;
	p += 2;

	os_memcpy(p, almac, ETH_ALEN);
	p += ETH_ALEN;
	os_memcpy(p, buf, len);
	p += len;

	_1905_interface_send(ctx, buf_io, len + ETH_ALEN + 4, NULL);
}


int _1905_notify_direct_encap_dpp_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_DIRECT_ENCAP_DPP_EVENT, al_mac, 0);

	return 0;
}

int _1905_notify_proxied_encap_dpp_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_PROXIED_ENCAP_DPP_EVENT, al_mac, 0);

	return 0;
}



int _1905_notify_dpp_bootstraping_uri_query_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_DPP_BOOTSTRAP_URI_QUERY_EVENT, al_mac, 0);

	return 0;
}


int _1905_notify_dpp_bootstraping_uri_notification_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_DPP_BOOTSTRAP_URI_NOTIFY_EVENT, al_mac, 0);

	return 0;
}


int _1905_notify_dpp_cce_indication_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_DPP_CCE_INDICATION_EVENT, al_mac, 0);

	return 0;
}

int _1905_notify_chirp_notification_event(struct p1905_managerd_ctx* ctx, unsigned char* al_mac)
{
	_1905_event_notify(ctx, _1905_RECV_CHIRP_NOTIFICATION_EVENT, al_mac, 0);

	return 0;
}


void _1905_notify_1905_security(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned char security_1905)
{
	unsigned char *p = buf_io;
	unsigned char *p_len = NULL;
	int len = 0;

	os_memset(buf_io, 0, sizeof(buf_io));
	*(unsigned short*)p = _1905_RECV_1905_SECURITY;
	p += 2;

	/*mid*/
	p += 2;

	p_len  = p;
	p += 2;

	os_memcpy(p, al_mac, ETH_ALEN);
	p += ETH_ALEN;

	*p++ = security_1905;

	len = (int)(p - buf_io);

	*(unsigned short*)p_len = ETH_ALEN + 1;

	hex_dump_info("1905 security notify", buf_io, len);

	_1905_interface_send(ctx, buf_io, len, NULL);
}


void _1905_notify_onboarding_result(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned char result, unsigned char *hash, unsigned short hash_len)
{
	unsigned char *p = buf_io;
	unsigned char *p_len = NULL;
	int len = 0;

	os_memset(buf_io, 0, sizeof(buf_io));
	*(unsigned short*)p = _1905_SET_ONBOARDING_RESULT_EVENT;
	p += 2;

	/*mid*/
	p += 2;

	p_len  = p;
	p += 2;

	os_memcpy(p, al_mac, ETH_ALEN);
	p += ETH_ALEN;

	*p++ = result;

	*p++ = hash_len;

	os_memcpy(p, hash, hash_len);
	p += hash_len;

	len = (int)(p - buf_io);

	*(unsigned short*)p_len = ETH_ALEN+2+hash_len;

	_1905_interface_send(ctx, buf_io, len, NULL);
}

#endif // #ifdef MAP_R3

int _1905_send_sync_recv_buf_size_rsp(struct p1905_managerd_ctx* ctx, unsigned short change_len, unsigned char rsp)
{
	unsigned char buf[50] = {0};
	struct msg *recv_event = (struct msg *)buf;
	int len = 0;
	int ret = 0;

	/*type*/
	recv_event->type = _1905_SYNC_RECV_BUF_SIZE_RSP_EVENT;
	len += 2;

	/*mid*/
	len += 2;

	/*len*/
	recv_event->length = sizeof(rsp) + sizeof(change_len);
	len += 2;

	/*buf*/
	memcpy(recv_event->buffer, &change_len, sizeof(change_len));
	len += sizeof(change_len);
	memcpy(recv_event->buffer + sizeof(change_len) , &rsp, sizeof(rsp));
	len += sizeof(rsp);

	ret = _1905_interface_send_int(ctx, buf, (size_t)len, NULL);

	if(ret > 0) {
		debug(DEBUG_OFF, "_1905_SYNC_RECV_BUF_SIZE_RSP_EVENT send to mapd success, change_len:%d, rsp:%d\n", change_len, rsp);
		return wapp_utils_success;
	} else {
		return wapp_utils_error;
	}
}

int _1905_send_sync_recv_buf_size(struct p1905_managerd_ctx* ctx, unsigned short change_len, unsigned short own_len)
{
	unsigned char buf[50] = {0};
	struct msg *recv_event = (struct msg *)buf;
	int len = 0;
	int ret = 0;

	/*type*/
	recv_event->type = _1905_SYNC_RECV_BUF_SIZE_EVENT;
	len += 2;

	/*mid*/
	len += 2;

	/*len*/
	recv_event->length = sizeof(change_len) + sizeof(own_len);
	len += 2;

	/*buf*/
	memcpy(recv_event->buffer, &change_len, sizeof(change_len));
	len += sizeof(change_len);
	memcpy(recv_event->buffer + sizeof(change_len), &own_len, sizeof(own_len));
	len += sizeof(own_len);
	ret = _1905_interface_send_int(ctx, buf, (size_t)len, NULL);

	if(ret > 0) {
		debug(DEBUG_OFF, "_1905_SYNC_RECV_BUF_SIZE_EVENT send to mapd success, change_len:%d, own_len:%d\n", change_len, own_len);
		return wapp_utils_success;
	} else {
		return wapp_utils_error;
	}
}
