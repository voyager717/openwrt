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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>
#include "p1905_managerd.h"
#include "p1905_ap_autoconfig.h"
#include "cmdu_message.h"
#include "cmdu.h"
#include "multi_ap.h"
#include "_1905_lib_io.h"
#include "debug.h"
#include "eloop.h"
#include "topology.h"
#ifdef MAP_R3
#include "common.h"
#include "map_dpp.h"
#endif

extern const unsigned char p1905_multicast_address[6];
void trigger_auto_config_flow(struct p1905_managerd_ctx *ctx);
void reset_radio_config_state(struct p1905_managerd_ctx *ctx);

int ap_autoconfig_init(struct p1905_managerd_ctx *ctx)
{
	if (ctx->role == AGENT) {
		/*set defalut index = -1*/
		ctx->current_autoconfig_info.radio_index = -1;
	    ctx->enrolle_state = no_ap_autoconfig;
	    ctx->current_rx_data = NULL;
	    ctx->is_authenticator_exist_in_M2 = 0;
	    ctx->is_in_encrypt_settings = 0;
	    ctx->get_config_attr_kind = 0;
	}

	ctx->last_rx_data = os_malloc(10240);
	if (!ctx->last_rx_data) {
		debug(DEBUG_ERROR, "failed to allocate last_rx_data\n");
		goto fail3;
	}
	ctx->last_rx_buf_len = 10240;
	ctx->last_rx_length = 0;

	ctx->last_tx_data = os_malloc(10240);
	if (!ctx->last_tx_data) {
		debug(DEBUG_ERROR, "failed to allocate last_tx_data\n");
		goto fail2;
	}
	ctx->last_tx_buf_len = 10240;
	ctx->last_tx_length = 0;


    ctx->is_ap_config_by_eth = 0;
    //get_uuid(ctx->uuid);

	return 0;
fail2:
	os_free(ctx->last_rx_data);
fail3:
	return -1;
}

void ap_autoconfig_search(struct p1905_managerd_ctx* ctx, unsigned short mid)
{
	int i = 0;
	unsigned char multicast_address[ETH_ALEN] = {0};

	memcpy(multicast_address, p1905_multicast_address, ETH_ALEN);
	debug(DEBUG_ERROR, AUTO_CONFIG_PREX"tx multicast search (mid-%04x) on all intf\n",
		mid);
	for (i = 0; i < ctx->itf_number; i++) {
		insert_cmdu_txq(multicast_address, ctx->p1905_al_mac_addr,
			e_ap_autoconfiguration_search, mid, ctx->itf[i].if_name, 0);
#ifdef SUPPORT_CMDU_RELIABLE
		cmdu_reliable_send(ctx, e_ap_autoconfiguration_search, mid, i);
#endif
	}
}

#ifdef MAP_R3
void periodic_send_autoconfig_search(void *eloop_ctx, void *timeout_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;
	debug(DEBUG_INFO, AUTO_CONFIG_PREX"periodic_send_autoconfig_search on R3 device while R1|R2 onboarding\n");
	ctx->controller_search_cnt++;
	ctx->autoconfig_search_mid = ++ctx->mid;
	reset_send_tlv(ctx);
	ap_autoconfig_search(ctx, ctx->mid);

	eloop_cancel_timeout(periodic_send_autoconfig_search, (void *)ctx, NULL);
	eloop_register_timeout(30, 0, periodic_send_autoconfig_search, (void *)ctx, NULL);
}
#endif


void ap_controller_search_step(void *eloop_ctx, void *timeout_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;

	multi_ap_controller_search_sm(ctx);

	common_process(ctx, &ctx->trx_buf);
}

void multi_ap_controller_search_sm(struct p1905_managerd_ctx* ctx)
{
	switch(ctx->controller_search_state) {
	case bk_link_ready:
	case wait_4_send_controller_search:
		if (ctx->controller_search_state == bk_link_ready) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"[bk_link_ready] send search\n");
		}
		else {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"timer to send search\n");
		}

	    ctx->autoconfig_search_mid = ++ctx->mid;
		reset_send_tlv(ctx);
		ap_autoconfig_search(ctx, ctx->mid);
	    ctx->controller_search_state = wait_4_recv_controller_search_rsp;
		eloop_register_timeout(RECV_CONFIG_RSP_TIME, 0, ap_controller_search_step,
			(void *)ctx, NULL);
		break;
	case wait_4_recv_controller_search_rsp:
		/*re-send autoconfiguration search*/
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"[wait_4_recv_controller_search_rsp] retry search\n");
		ctx->controller_search_cnt++;
		ctx->autoconfig_search_mid = ++ctx->mid;
		reset_send_tlv(ctx);
		ap_autoconfig_search(ctx, ctx->mid);
		eloop_register_timeout(3, 0, ap_controller_search_step, (void *)ctx, NULL);
		break;
	case controller_search_done:
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"[controller_search_done]\n");
		_1905_notify_controller_found(ctx);
		ctx->controller_search_state = controller_search_idle;
		eloop_cancel_timeout(ap_controller_search_step, (void *)ctx, NULL);
#ifdef MAP_R3
		if (ctx->map_version == MAP_PROFILE_R3 && ctx->r3_oboard_ctx.active) {
			/*r3 onboarding*/
			if (ctx->r3_oboard_ctx.bh_type == MAP_BH_WIFI) {
				if ((!ctx->r3_dpp.onboarding_type)&&ctx->r3_dpp.connector_len) {
					struct r3_member *peer = NULL;
					debug(DEBUG_ERROR, AUTO_CONFIG_PREX"WIFI BH and dpp onboarding type, "
						"trigger dpp introduction with Controller("
						MACSTR")\n", MAC2STR(ctx->r3_oboard_ctx.peer_almac));

					peer = get_r3_member(ctx->r3_oboard_ctx.peer_almac);
					if (!peer) {
						debug(DEBUG_ERROR, AUTO_CONFIG_PREX"R3 member("MACSTR") not found\n",
								MAC2STR(ctx->r3_oboard_ctx.peer_almac));
						break;
					}

					if (peer->profile < MAP_PROFILE_R3) {
						debug(DEBUG_ERROR, AUTO_CONFIG_PREX"peer("MACSTR") not R3 dev\n",
								MAC2STR(peer->al_mac));
						break;
					}
                    /*trigger 1905 dpp introduction with Controller first*/
					peer->r3_info.cur_dpp_state = dpp_idle;
					map_cancel_onboarding_timer(ctx, peer);

					map_dpp_trigger_member_dpp_intro(ctx, peer);
                }
				else {
					if (ctx->r3_dpp.onboarding_type)
						debug(DEBUG_ERROR, AUTO_CONFIG_PREX"WIFI BH, but PBC case, trigger WSC\n");
					if (!ctx->r3_dpp.connector_len)
						debug(DEBUG_ERROR, AUTO_CONFIG_PREX"WIFI BH, no connector set, trigger WSC\n");
					reset_radio_config_state(ctx);
					trigger_auto_config_flow(ctx);
				}
			}
			else if (ctx->r3_oboard_ctx.bh_type == MAP_BH_ETH) {
				unsigned int wait_auth_time = 10;
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX"ETH BH, wait dpp auth req for 10 sec\n");

				/*start 10s timer to wait for Direct Encap DPP MSG*/
				eloop_cancel_timeout(r3_wait_for_dpp_auth_req, (void *)ctx, NULL);
				/* R3 testbed, QC Controller reply dpp auth req right after 10 sec
				** so need wait more time
				*/
				if (ctx->MAP_Cer)
					wait_auth_time = 20;
				eloop_register_timeout(wait_auth_time, 0, r3_wait_for_dpp_auth_req, (void *)ctx, NULL);
			}
			break;
		}
#endif
		if (!ctx->renew_ctx.is_renew_ongoing) {
			reset_radio_config_state(ctx);
			trigger_auto_config_flow(ctx);
		} else {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"renew is ongoing; skip autoconfiguration by self\n");
		}
		break;
	default:
		break;
	}
}


void ap_autoconfig_enrolle_step(void *eloop_ctx, void *timeout_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;

	ap_autoconfig_enrolle_sm(ctx);

	common_process(ctx, &ctx->trx_buf);
}

void ap_autoconfig_enrolle_sm(struct p1905_managerd_ctx *ctx)
{
	static unsigned int search_retry_count = 0, m1_retry_count = 0;
	unsigned char radio_index = 0, bss_idx = 0;

    switch(ctx->enrolle_state)
    {
        case wait_4_send_ap_autoconfig_search:
            /* if PLC is not authenticated and
             * if ctx->is_ap_config_by_eth = 1 and ETH doesn't connect
             * break!!!
             * this function will be called periodically by timer handler
             * so system will check PLC and ETH status periodically
             */
            ctx->autoconfig_search_mid = ++ctx->mid;
			reset_send_tlv(ctx);
			ap_autoconfig_search(ctx, ctx->mid);

            ctx->enrolle_state = wait_4_recv_ap_autoconfig_resp;
			eloop_register_timeout(RECV_CONFIG_RSP_TIME, 0,
				ap_autoconfig_enrolle_step, (void *)ctx, NULL);
            break;

        case wait_4_recv_ap_autoconfig_resp:
                /* ap auto config timeout. Set state to no_ap_autoconfig
                   re-trigger ap autoconfig search process.
                 */
				if(search_retry_count < 2)
				{
					debug(DEBUG_OFF, "timeout, retry ap autoconfig search\n");
					search_retry_count++;
					ctx->autoconfig_search_mid = ++ctx->mid;
					reset_send_tlv(ctx);
					ap_autoconfig_search(ctx, ctx->mid);
					eloop_register_timeout(RECV_CONFIG_RSP_TIME, 0,
						ap_autoconfig_enrolle_step, (void *)ctx, NULL);
				}
				else
				{
					debug(DEBUG_OFF, "cannot find any controller!!!\n");
					ctx->enrolle_state = no_ap_autoconfig;
					auto_configuration_done(ctx);
					search_retry_count = 0;
				}
            break;
		case wait_4_send_m1:
			insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
				e_ap_autoconfiguration_wsc_m1, ++ctx->mid,
				ctx->itf[ctx->cinfo.recv_ifid].if_name, 0);
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"tx WSC M1(mid-%04x) to ("MACSTR") on %s\n",
				ctx->mid, PRINT_MAC(ctx->cinfo.almac), ctx->itf[ctx->cinfo.recv_ifid].if_name);

	        ctx->enrolle_state = wait_4_recv_m2;
			os_memset(&ctx->ap_config_data, 0, sizeof(WSC_CONFIG));
			eloop_cancel_timeout(ap_autoconfig_enrolle_step, (void *)ctx, NULL);
			eloop_register_timeout(RECV_WSC_M2_TIME, 0, ap_autoconfig_enrolle_step,
				(void *)ctx, NULL);
			break;
        case wait_4_recv_m2:
            /*check timeout*/
            /*if timeout happen, need to free ctx->ap_config_data*/
			search_retry_count = 0;

			if (m1_retry_count < M1_RETRY_CNT) {
				ctx->enrolle_state = wait_4_send_m1;
				debug(DEBUG_OFF, AUTO_CONFIG_PREX"timeout, retry m1\n");
				m1_retry_count++;
				eloop_register_timeout(0, 0, ap_autoconfig_enrolle_step, (void *)ctx, NULL);
			} else {
				debug(DEBUG_OFF, AUTO_CONFIG_PREX"timeout, stop retry m1 m1_retry_count(%d)\n",
					m1_retry_count);
				ctx->enrolle_state = no_ap_autoconfig;
				m1_retry_count = 0;
				auto_configuration_done(ctx);
			}

            break;

        case wait_4_set_config:
		{
			search_retry_count = 0;
			m1_retry_count = 0;
        	unsigned char map_vendor_extension = 0, radio_left = 0;
			int i = 0;

			for (i = 0; i < ctx->current_autoconfig_info.config_number; i++)
				map_vendor_extension |= ctx->current_autoconfig_info.config_data[i].map_vendor_extension;
			/*MAP controller let agent tear down*/
			if (map_vendor_extension & BIT_TEAR_DOWN) {
				if (ctx->current_autoconfig_info.radio_index != -1) {
					radio_index = (unsigned char)ctx->current_autoconfig_info.radio_index;
					debug(DEBUG_ERROR, AUTO_CONFIG_PREX"current radio("MACSTR") will tear down\n",
						PRINT_MAC(ctx->rinfo[radio_index].identifier));
					ctx->rinfo[radio_index].teared_down = 1;
					for (bss_idx = 0; bss_idx < ctx->rinfo[radio_index].bss_number; bss_idx++)
						ctx->rinfo[radio_index].bss[bss_idx].config_status = 0;
					delete_exist_operational_bss(ctx, ctx->rinfo[radio_index].identifier);
					/*to do tear down this radio*/
					_1905_set_radio_tear_down(ctx, ctx->rinfo[radio_index].identifier);
				} else {
					debug(DEBUG_ERROR, AUTO_CONFIG_PREX"error!!! radio_index  == -1!!!\n");
				}
			} else if (wifi_utils_success != set_wsc_config((void *)ctx)) {
                debug(DEBUG_ERROR, AUTO_CONFIG_PREX"set wsc config error for radio("MACSTR")\n",
					PRINT_MAC(ctx->rinfo[radio_index].identifier));
            }

			ctx->enrolle_state = no_ap_autoconfig;
			auto_configuration_done(ctx);
			trigger_auto_config_flow(ctx);

			if (ctx->renew_ctx.is_renew_ongoing) {
				radio_left = get_left_unconfig_radio(ctx);
				eloop_cancel_timeout(agent_apply_new_config, (void *)ctx, NULL);

				if (!radio_left) {
					/*all radios have been configured successfully, apply its wireless setting immediately*/
					eloop_register_timeout(0, 0,
							agent_apply_new_config, (void *)ctx, NULL);
				} else {
					/*wait for left unconfiguring process*/
					eloop_register_timeout(radio_left * ONE_RADIO_CONFIG_TIME_MAX, 0,
							agent_apply_new_config, (void *)ctx, NULL);
				}
			}
#ifdef MAP_R3
			if (ctx->map_version == MAP_PROFILE_R3 &&
				(ctx->r3_oboard_ctx.bh_type == MAP_BH_ETH)) {
				/* R1|R2 onboarding on R3 device, need send autoconfig search periodicly */
				eloop_cancel_timeout(periodic_send_autoconfig_search, (void *)ctx, NULL);
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX"R1|R2 onboarding on R3 dev,register 30s timer to send autoconfig search periodicly\n");
				eloop_register_timeout(30, 0, periodic_send_autoconfig_search, (void *)ctx, NULL);
			}
#endif
        }
            break;
		default:
			break;
    }
}

int pop_node_from_stack(struct p1905_managerd_ctx *ctx)
{
	leaf_info *leaf_top = NULL;
#ifdef MAP_R3
	struct r3_member *peer = NULL;
#endif

	if (get_top(&ctx->topo_stack, &leaf_top) < 0) {
		debug(DEBUG_ERROR, "controller_renew_bss: topo stack empty\n");
		ctx->renew_ctx.trigger_renew = 0;
		return -1;
	}

	debug(DEBUG_OFF, "pop_node_from_stack, pop out_mac("MACSTR")\n",
			PRINT_MAC(leaf_top->al_mac));

	if (!os_memcmp(ctx->p1905_al_mac_addr, leaf_top->al_mac, ETH_ALEN)) {
		debug(DEBUG_OFF, "do not renew to controller almac("MACSTR")\n",
			PRINT_MAC(leaf_top->al_mac));
		ctx->renew_state = wait_4_apply_local;
		eloop_register_timeout(0, 0, controller_renew_bss_step,
					(void *)ctx, NULL);
		return 0;
	}

#ifdef MAP_R3
	if (ctx->map_version == MAP_PROFILE_R3) {
		peer = get_r3_member(leaf_top->al_mac);
	}

	if (peer && peer->security_1905) {
		ctx->renew_state = wait_4_send_bss_reconfig_trigger;
		debug(DEBUG_OFF, "peer almac("MACSTR") is R3 onboarded\n",
			PRINT_MAC(leaf_top->al_mac));
	}
	else
#endif
		ctx->renew_state = wait_4_send_renew;

	leaf_top->renew_retry_cnt = 3;
	eloop_register_timeout(0, 0, controller_renew_bss_step, (void *)ctx, NULL);

	return 0;
}

int controller_send_renew(struct p1905_managerd_ctx *ctx)
{
	struct topology_response_db *rpdb = NULL;
	leaf_info *leaf_top = NULL;
	int ret = 0;
	struct agent_list_db *agent_info = NULL;
	unsigned char *if_name = NULL;
	unsigned char freq_band = 0x00;
	int i = 0;

	ret = get_top(&ctx->topo_stack, &leaf_top);
	if (ret < 0) {
		debug(DEBUG_ERROR, "BUG here!!!! get top node from stack error\n");
		ctx->renew_ctx.trigger_renew = 0;
		return -1;
	}

	rpdb = lookup_tprdb_by_almac(ctx, leaf_top->al_mac);
	if (!rpdb) {
		debug(DEBUG_ERROR, "BUG here!!!! rpdb almac("MACSTR") not exist\n",
			PRINT_MAC(leaf_top->al_mac));
		ctx->renew_ctx.trigger_renew = 0;
		return -1;
	}
	if_name = ctx->itf[rpdb->recv_ifid].if_name;

	find_agent_info(ctx, leaf_top->al_mac, &agent_info);
	if (!agent_info) {
		debug(DEBUG_ERROR, "cannot find agent info"MACSTR"\n", PRINT_MAC(leaf_top->al_mac));
		return -1;
	}

	/*in order to be compatible with older version, change the renew freq band*/
	for (i = 0; i < agent_info->radio_num; i++) {
		if (!agent_info->ra_info[i].conf_ctx.config_status) {
			if (agent_info->ra_info[i].band & BAND_2G_CAP)
				freq_band = 0x00;
			else if (agent_info->ra_info[i].band & BAND_5G_CAP)
				freq_band = 0x01;
			break;
		}
	}

	if (fill_send_tlv(ctx, &freq_band, 1) < 0) {
		ctx->renew_ctx.trigger_renew = 0;
		return -1;
	}
	debug(DEBUG_OFF, "controller_send_renew ++ freq_band=%02x\n", freq_band);

	insert_cmdu_txq(leaf_top->al_mac, ctx->p1905_al_mac_addr,
		e_ap_autoconfiguration_renew, ++ctx->mid, if_name, 0);

	if (!ctx->MAP_Cer) {
		if (agent_info && agent_info->wts_syn_done == 1) {
			debug(DEBUG_TRACE, "reset agent wts sync state\n");
			agent_info->wts_syn_done = 0;
		}
	}

	ctx->renew_state = wait_4_check_renew_result;
	eloop_register_timeout(RECV_WSC_M1_TIME, 0, controller_renew_bss_step, (void *)ctx, NULL);

	return 0;

}

void controller_check_agent_renew_state(struct p1905_managerd_ctx *ctx)
{
	leaf_info *leaf_top = NULL;
	int ret = 0;
	struct agent_list_db *agent_info = NULL;

	ret = get_top(&ctx->topo_stack, &leaf_top);
	if (ret < 0) {
		debug(DEBUG_ERROR, "BUG here!!!! get top node from stack error\n");
		ctx->renew_ctx.trigger_renew = 0;
		return;
	}

	find_agent_info(ctx, leaf_top->al_mac, &agent_info);
	if (!agent_info) {
		debug(DEBUG_ERROR, "cannot find agent info"MACSTR"\n", PRINT_MAC(leaf_top->al_mac));
		ctx->renew_ctx.trigger_renew = 0;
		return;
	}

	if (get_agent_all_radio_config_state(agent_info)) {
		/*all radio if agent have been configured, renew next agent*/
		pop_node(&ctx->topo_stack);
		ctx->renew_state = wait_4_pop_node;
		debug(DEBUG_ERROR,
			"all radios have been configured of agent "MACSTR", renew next one\n",
			PRINT_MAC(agent_info->almac));
	} else {
		leaf_top->renew_retry_cnt--;
		if (leaf_top->renew_retry_cnt > 0) {
			/*not all radio if agent have been configured, retry renew*/
			ctx->renew_state = wait_4_send_renew;
			debug(DEBUG_OFF, "send renew but get M1/M2_ack timeout, retry\n");
		}
		else {
			/*disappointing agent, renew next agent*/
			pop_node(&ctx->topo_stack);
			ctx->renew_state = wait_4_pop_node;
			debug(DEBUG_OFF, "send renew but get M1/M2_ack timeout, renew next agent\n");
		}
	}

	eloop_register_timeout(0, 0, controller_renew_bss_step, (void *)ctx, NULL);

}

#ifdef MAP_R3
int r3_send_bss_reconfig_trigger(struct p1905_managerd_ctx *ctx)
{
	struct topology_response_db *rpdb = NULL;
	leaf_info *leaf_top = NULL;
	int ret = 0;
	unsigned char *if_name = NULL;

	ret = get_top(&ctx->topo_stack, &leaf_top);
	if (ret < 0) {
		debug(DEBUG_ERROR, "BUG here!!!! get top node from stack error\n");
		ctx->renew_ctx.trigger_renew = 0;
		return -1;
	}

	rpdb = lookup_tprdb_by_almac(ctx, leaf_top->al_mac);
	if (!rpdb) {
		debug(DEBUG_ERROR, "BUG here!!!! rpdb almac("MACSTR") not exist\n",
			PRINT_MAC(leaf_top->al_mac));
		ctx->renew_ctx.trigger_renew = 0;
		return -1;
	}
	if_name = ctx->itf[rpdb->recv_ifid].if_name;

	insert_cmdu_txq(leaf_top->al_mac, ctx->p1905_al_mac_addr,
		e_bss_reconfiguration_trigger, ++ctx->mid, if_name, 0);

	ctx->renew_state = wait_4_recv_bss_autoconfig_req;
	eloop_cancel_timeout(controller_renew_bss_step, (void *)ctx, NULL);
	eloop_register_timeout(5, 0, controller_renew_bss_step, (void *)ctx, NULL);
	debug(DEBUG_ERROR, "[BSS Re-Config] send bss reconfig trigger message to "MACSTR"\n", MAC2STR(leaf_top->al_mac));

	return 0;
}
#endif

void controller_renew_bss_step(void *eloop_ctx, void *timeout_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;

	controller_renew_bss_sm(ctx);

	common_process(ctx, &ctx->trx_buf);
}

void controller_renew_bss_sm(struct p1905_managerd_ctx *ctx)
{
#ifdef MAP_R3
	leaf_info *leaf_top = NULL;
#endif

	switch(ctx->renew_state)
	{
		case wait_4_dump_topo:
		{
			empty_stack(&ctx->topo_stack);
			/*travel topology tree and dump the topo*/
			topology_tree_travel_preorder(ctx, ctx->root_leaf);

			ctx->renew_state = wait_4_pop_node;
			eloop_register_timeout(0, 0, controller_renew_bss_step, (void *)ctx, NULL);
		}
		break;

		case wait_4_pop_node:
		{
			pop_node_from_stack(ctx);
		}
		break;

		case wait_4_send_renew:
		{
			controller_send_renew(ctx);
		}
		break;

		case wait_4_check_renew_result:
		{
			controller_check_agent_renew_state(ctx);
		}
		break;

		case wait_4_apply_local:
		{
			int i = 0;
			debug(DEBUG_ERROR, "apply bss config locally\n")
			for (i = 0; i < ctx->radio_number; i++) {
				_1905_update_bss_info_per_radio(ctx, &ctx->rinfo[i]);
			}
			ctx->renew_ctx.trigger_renew = 0;
#ifdef MAP_R2
			eloop_cancel_timeout(map_r2_notify_ts_config, (void *)ctx, NULL);
			eloop_cancel_timeout(map_notify_transparent_vlan_setting, (void *)ctx, NULL);
			eloop_register_timeout(0, 0, map_r2_notify_ts_config, (void *)ctx, NULL);
			eloop_register_timeout(0, 0, map_notify_transparent_vlan_setting, (void *)ctx, NULL);
#endif
		}
		break;
#ifdef MAP_R3
	case wait_4_send_bss_reconfig_trigger:
		r3_send_bss_reconfig_trigger(ctx);
		break;
	case wait_4_recv_bss_autoconfig_req:
		/* wait for bss config req timeout*/
		if (get_top(&ctx->topo_stack, &leaf_top) < 0) {
			debug(DEBUG_ERROR, "BUG here!!!! get top node from stack error\n");
			ctx->renew_ctx.trigger_renew = 0;
			return;
		}
		debug(DEBUG_OFF, "send bss reconfig trigger but get bss config req timeout\n");
		leaf_top->renew_retry_cnt--;

		if (leaf_top->renew_retry_cnt <= 0) {
			//renew next agent
			pop_node(&ctx->topo_stack);
			ctx->renew_state = wait_4_pop_node;
		} else {
			ctx->renew_state = wait_4_send_bss_reconfig_trigger;
		}

		eloop_cancel_timeout(controller_renew_bss_step, (void *)ctx, NULL);
		eloop_register_timeout(0, 0, controller_renew_bss_step, (void *)ctx, NULL);
		break;
#endif
	default:
			break;

	}
}


void agent_apply_new_config(void *eloop_ctx, void *timeout_ctx) {
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;
	debug(DEBUG_ERROR, "agent_apply_new_config ++\n");

	ctx->renew_ctx.is_renew_ongoing = 0;
	flash_wsc_config(ctx);
}

void reset_radio_config_state(struct p1905_managerd_ctx *ctx)
{
	int i = 0;

	if (ctx->role == CONTROLLER)
		return;

	for (i = 0; i < ctx->radio_number; i++)
		ctx->rinfo[i].config_status = MAP_CONF_UNCONF;

	ctx->current_autoconfig_info.radio_index = -1;
}

/*get config state of all radios
 *return value: number of left radios that are not configured
 */
unsigned char get_left_unconfig_radio(struct p1905_managerd_ctx *ctx)
{
	int i = 0;
	unsigned char left = 0;

	for (i = 0; i < ctx->radio_number; i++) {
		if (ctx->rinfo[i].config_status == MAP_CONF_UNCONF)
			left++;
	}

	return left;
}

void trigger_auto_config_flow(struct p1905_managerd_ctx *ctx)
{
	int i = 0;
	int ret =0;

	if(ctx == NULL)
		return;
	if (ctx->role == CONTROLLER)
		return;

	for (i = 0; i < ctx->radio_number; i++) {
		if (ctx->rinfo[i].config_status == MAP_CONF_UNCONF)
			break;
	}

	if (i >= ctx->radio_number) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"auto-configuration done\n");
		return;
	}

#if 0
	ctx->authenticated = 1;
#endif

	ret= set_radio_autoconf_trriger(ctx, i, 1);
	if (ret == -2) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"should trigger next radio\n");
		ctx->rinfo[i].config_status = MAP_CONF_CONFED;
		trigger_auto_config_flow(ctx);
	} else if (ret == 0) {
		triger_autoconfiguration(ctx);
	}
}

int cont_handle_autoconfig_wsc(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned char *if_name, unsigned short mid, struct agent_radio_info *r,
	unsigned char radio_cnt, unsigned char *radio_identifier)
{
	if (ctx->role == AGENT) {
		struct radio_info *own_radio = NULL, *current_config_radio = NULL;

		if (ctx->current_autoconfig_info.radio_index == -1) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"Error!!!no ongoing auto config radio"
				", drop this M2\n");
			goto fail;
		}

		own_radio = get_rainfo_by_id(ctx, radio_identifier);
		if (own_radio == NULL) {
			debug(DEBUG_ERROR, "[%d]get_rainfo_by_id fail!\n", __LINE__);
			goto fail;
		}
		current_config_radio = &ctx->rinfo[ctx->current_autoconfig_info.radio_index];

		if (own_radio != current_config_radio) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"Error! M2 back radio("MACSTR") "
				"ongoing radio("MACSTR") not match, drop this M2\n",
				PRINT_MAC(own_radio->identifier),
				PRINT_MAC(current_config_radio->identifier));
			goto fail;
		}
		/*we got correct M2, so enter the set config state*/
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"recv WSC M2(mid-%04x) from ("MACSTR")\n",
			mid, PRINT_MAC(al_mac));
		ctx->enrolle_state = wait_4_set_config;
		eloop_cancel_timeout(ap_autoconfig_enrolle_step, (void *)ctx, NULL);
		eloop_register_timeout(0, 0, ap_autoconfig_enrolle_step, (void *)ctx, NULL);
		if (ctx->renew_ctx.is_renew_ongoing) {
			/*send private m2 ack for agent*/
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr,
				e_1905_ack, mid, if_name, 0);
			debug(DEBUG_TRACE, "respond ack(%04x) to ("MACSTR") on %s\n",
				mid, PRINT_MAC(al_mac), if_name);
			process_cmdu_txq(ctx, ctx->trx_buf.buf);
		}
	} else {
		struct agent_list_db *agent_info = NULL;
		struct agent_radio_info *radio = NULL;
		unsigned short m2_mid = 0;
		/* Registrar got correct M1. Need to response M2.(Unicast)
		 * It has a strange behavior about mid. Spec does not specify we
		 * need to use same mid for M1 and M2. So I create a new mid for M2
		 */
		find_agent_info(ctx, al_mac, &agent_info);
		if (!agent_info)
			goto fail;
		if (radio_cnt != 1) {
			debug(DEBUG_ERROR, "Error! number of ap radio basic cap tlv in M1 is incorrect(%d)\n",
				radio_cnt);
			goto fail;
		}
		update_agent_radio_info(agent_info, r);

		radio = find_agent_radio_info(agent_info, r->identifier);
		if (!radio) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"BUG here!!!cannot find radio "MACSTR" for agent "MACSTR"\n",
				PRINT_MAC(r->identifier), PRINT_MAC(al_mac));
			goto fail;
		}
		set_agent_wsc_doing_radio(agent_info, radio->identifier);
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"receive M1 on agent("MACSTR"),radio id is "MACSTR"\n",
				PRINT_MAC(al_mac),PRINT_MAC(radio->identifier));

		insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr,
			e_ap_autoconfiguration_wsc_m2, ++ctx->mid,
			if_name, 0);
		m2_mid = ctx->mid;
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"prepare M2(mid-%04x) to "MACSTR" on %s\n",
			m2_mid, PRINT_MAC(al_mac), if_name);

		/*
		* After Registrar send out M2, it will send out a Vendor Specific
		* Message including Registrar's wts content.
		* And Enrollee will update itself's wts file.
		*/
		if (!ctx->MAP_Cer) {
			if (agent_info->wts_syn_done == 0) {
				insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_vendor_specific_wts_content, ++ctx->mid,
					if_name, 0);
				agent_info->wts_syn_done = 1;
			}
		}

		if (ctx->renew_ctx.trigger_renew) {
			leaf_info *leaf_top = NULL;

			if (get_top(&ctx->topo_stack, &leaf_top) < 0) {
				debug(DEBUG_ERROR, "BUG here!!!! get top node from stack error\n");
				goto fail;
			}
			if (!os_memcmp(leaf_top->al_mac, al_mac, ETH_ALEN)) {
				eloop_cancel_timeout(controller_renew_bss_step, (void *)ctx, NULL);
				radio->conf_ctx.m2_mid = m2_mid;
				ctx->renew_state = wait_4_check_renew_result;
				eloop_register_timeout(RECV_WSC_M2_ACK_TIME, 0,
					controller_renew_bss_step, (void *)ctx, NULL);

				debug(DEBUG_TRACE, "m2_ack_mid(%04x)\n",
					radio->conf_ctx.m2_mid);
			}
		} else {
			/*
			 * for non-renew case, e.g config error handling process, assuming config success
			 * after sending M2.
			 */
			set_agent_radio_config_stat(agent_info, radio->identifier, 1);
		}
	}
	return 0;
fail:
	return -1;
}

int cont_handle_autoconfig_renew(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned char *if_name, unsigned char band)
{
	int i = 0;
	int ret = 0;

	if (ctx->role != AGENT)
		goto fail;

#ifdef MAP_R3
	if ((ctx->map_version >= MAP_PROFILE_R3) &&
		(ctx->r3_oboard_ctx.onboarding_stage != R3_ONBOARDING_STAGE_INVALID)) {
		debug(DEBUG_ERROR, "R3 onboarding is ongoing, skip autoconfig renew\n");
		goto end;
	}
#endif
	ctx->renew_ctx.is_renew_ongoing = 1;

	debug(DEBUG_ERROR, "recv renew message from "MACSTR"\n", PRINT_MAC(al_mac));
	os_memcpy(ctx->cinfo.almac, al_mac, ETH_ALEN);
	ctx->cinfo.supported_freq = band;
	ret = os_snprintf((char *)ctx->cinfo.local_ifname, sizeof(ctx->cinfo.local_ifname), "%s", if_name);
	if (os_snprintf_error(sizeof(ctx->cinfo.local_ifname), ret)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		goto fail;
	}
	ctx->cinfo.recv_ifid = ctx->recent_cmdu_rx_if_number;
	debug(DEBUG_TRACE, "recv conroller with interface=%s, ifidx=%u\n",
		 ctx->cinfo.local_ifname, ctx->cinfo.recv_ifid);

	/*
	If a Multi-AP Agent receives an AP-Autoconfiguration Renew message,
	then it shall respond within one second by sending one AP-Autoconfiguration
	WSC message per section 17.1.3 for each of its radios, irrespective of the
	value specified in the SupportedFreqBand TLV in the AP-Autoconfiguration Renew
	message.
	*/
	for (i = 0; i < ctx->radio_number; i++) {
		ctx->rinfo[i].teared_down = 0;
	}
	reset_radio_config_state(ctx);
	trigger_auto_config_flow(ctx);
#ifdef MAP_R3
end:
#endif
	return 0;
fail:
	return -1;
}

int cont_handle_autoconfig_response(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned char *if_name, unsigned char band, unsigned char role, unsigned char profile, unsigned char kibmib)
{
#ifdef MAP_R3
	struct r3_member *peer = NULL;
#endif
	int ret = 0;

	ctx->is_authenticator_exist_in_M2 = 0;
	debug(DEBUG_TRACE, "set ctx->is_authenticator_exist_in_M2 = 0\n");

	os_memcpy(ctx->cinfo.almac, al_mac, ETH_ALEN);
	ctx->cinfo.supported_freq = band;
	ctx->cinfo.supported_role_registrar = role;
	ctx->cinfo.profile = profile;
	ctx->cinfo.kibmib = kibmib;
	ret = os_snprintf((char *)ctx->cinfo.local_ifname, sizeof(ctx->cinfo.local_ifname), "%s", if_name);
	if (os_snprintf_error(sizeof(ctx->cinfo.local_ifname), ret)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		goto end;
	}
	ctx->cinfo.recv_ifid = ctx->recent_cmdu_rx_if_number;
	debug(DEBUG_TRACE, "find the conroller with supperted_freq=%d, role=%d\n", band, role);
#ifdef MAP_R3
	if (NULL == (peer = get_r3_member(al_mac)) && ctx->map_version == MAP_PROFILE_R3) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"create R3 member for "MACSTR"\n", MAC2STR(al_mac));
		peer = create_r3_member(al_mac);
	}

	if (peer) {
		peer->profile = profile;
		/* peer->role change to Controller after receive autoconfig resp */
		peer->role = CONTROLLER;
	}
#endif
	debug(DEBUG_TRACE, "recv conroller with interface=%s, ifidx=%u\n",
		 ctx->cinfo.local_ifname, ctx->cinfo.recv_ifid);
	debug(DEBUG_OFF, AUTO_CONFIG_PREX"find the conroller al_mac="MACSTR"\n",
		PRINT_MAC(al_mac));


	if (ctx->enrolle_state == no_ap_autoconfig && ctx->controller_search_state == controller_search_idle) {
		_1905_notify_autoconfig_rsp_event(ctx, al_mac);
		goto end;
	}
    if(((ctx->enrolle_state != wait_4_recv_ap_autoconfig_resp) &&
        ctx->controller_search_state == controller_search_idle)) {
       debug(DEBUG_ERROR, "enrolle_state mismatch\n");
       goto end;
    }

	if (ctx->controller_search_state != controller_search_idle) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"recv the autoconfig search rsp, notify the wapp search done\n");
		ctx->controller_search_state = controller_search_done;
#ifdef MAP_R3
		if (ctx->map_version == MAP_PROFILE_R3 &&
			profile == MAP_PROFILE_R3 && peer) {
            /*
            ** wifi case: if connector was set, launch DPP Onboarding
            **            else, WSC onboarding
            ** eth case:  still run DPP onboarding state machine
            **            if not receive DPP Auth in 10 sec, will start WSC
            */
            if ((ctx->r3_dpp.connector_len &&
                ctx->r3_oboard_ctx.bh_type == MAP_BH_WIFI) ||
                ctx->r3_oboard_ctx.bh_type == MAP_BH_ETH) {
                /* it is r3 onboarding */
                ctx->r3_oboard_ctx.active = 1;
                ctx->r3_oboard_ctx.peer_profile = profile;
                /* 1 means peer is controller */
                ctx->r3_oboard_ctx.peer_service_type = role;
                os_memcpy(ctx->r3_oboard_ctx.peer_almac, al_mac, ETH_ALEN);
				peer->r3_info.cur_dpp_state = dpp_idle;

                debug(DEBUG_ERROR, AUTO_CONFIG_PREX"R3 onboard active\n");
            } else {
                debug(DEBUG_ERROR, AUTO_CONFIG_PREX"R3 testbed [BH-WIFI], but "
					"connector not set, start WSC process\n");
            }
        }
#endif
		eloop_cancel_timeout(ap_controller_search_step, (void *)ctx, NULL);
		eloop_register_timeout(0, 0, ap_controller_search_step, (void *)ctx, NULL);
	}
end:
	return 0;
}

int cont_handle_autoconfig_search(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned char *if_name, unsigned char profile, unsigned short mid)
{
	struct agent_list_db *agent_info = NULL;

	if (ctx->role == CONTROLLER) {
#ifdef MAP_R3
		if (ctx->map_version == MAP_PROFILE_R3)
			create_r3_member(al_mac);
#endif
		/*insert agent entry*/
		agent_info = insert_agent_info(ctx, al_mac);
		if (agent_info)
			agent_info->profile = profile;
        /*send ap auto config response message, unicast*/
		insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr,
			e_ap_autoconfiguration_response, mid, if_name, 0);
#ifdef MAP_R3
        if (ctx->map_version == MAP_PROFILE_R3 &&
            profile == MAP_PROFILE_R3) {
            _1905_notify_autoconfig_search_event(ctx, al_mac);
        }
#endif
	} else {
		if(ctx->enrolle_state == no_ap_autoconfig &&
			ctx->controller_search_state == controller_search_idle)
			_1905_notify_autoconfig_search_event(ctx, al_mac);
	}

	return 0;
}

int cont_handle_autoconfig_wsc_ack(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned short mid)
{
	leaf_info *leaf_top = NULL;
	struct agent_list_db *agent_info = NULL;
	struct agent_radio_info *radio = NULL;

	if (ctx->role == AGENT)
		return 0;

	if (!ctx->renew_ctx.trigger_renew)
		return 0;

	if (get_top(&ctx->topo_stack, &leaf_top) < 0) {
		debug(DEBUG_ERROR, "BUG here!!!! get top node from stack error\n");
		goto error;
	}
	find_agent_info(ctx, al_mac, &agent_info);

	if (!agent_info) {
		debug(DEBUG_ERROR, "no agent info\n");
		goto error;
	}

	radio = find_agent_wsc_doing_radio(agent_info);

	if (!radio) {
		debug(DEBUG_ERROR, "no radio info\n");
		goto error;
	}

	if (radio->conf_ctx.m2_mid != mid) {
		debug(DEBUG_ERROR, "radio->conf_ctx.m2_mid(%d), mid(%d) mismatch\n",
			radio->conf_ctx.m2_mid, mid);
		goto error;
	}

	eloop_cancel_timeout(controller_renew_bss_step, (void *)ctx, NULL);
	set_agent_radio_config_stat(agent_info, radio->identifier, 1);
	debug(DEBUG_ERROR,
		"get correct m2_ack from agent "MACSTR", radio "MACSTR" configured successfully\n",
		PRINT_MAC(agent_info->almac), PRINT_MAC(radio->identifier));

	if (get_agent_all_radio_config_state(agent_info)) {
		/*if all radios have been configured, renew next one*/
		pop_node(&ctx->topo_stack);
		ctx->renew_state = wait_4_pop_node;
		eloop_register_timeout(0, 0,
			controller_renew_bss_step, (void *)ctx, NULL);
		debug(DEBUG_ERROR,
			"all radios have been configured of agent "MACSTR", renew next one\n",
			PRINT_MAC(agent_info->almac));
	} else {
		/*wait for m1 for another radio*/
		ctx->renew_state = wait_4_check_renew_result;
		eloop_register_timeout(RECV_WSC_M1_TIME, 0,
			controller_renew_bss_step, (void *)ctx, NULL);
	}

	return 0;
error:
	return -1;
}


/**
 *  get WSC config for ap-auto configure use.
 *
 * \param  wsc  pointer of  wsc_config.
 * \return  error code.
 */
WIFI_UTILS_STATUS get_wsc_config(void *pctx, WSC_CONFIG* wsc,
	unsigned char *wfa_vendor_extension, struct agent_radio_info *agent_radio)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)pctx;
	unsigned char band = 0;

	memset(wsc, 0, sizeof(WSC_CONFIG));

	band = determin_band_config(ctx, ctx->ap_config.enrolle_mac, agent_radio->band);
	debug(DEBUG_ERROR, AUTO_CONFIG_PREX"determin band=%s(%02x)\n", band_2_string(band), band);
	if (band == BAND_INVALID_CAP) {
		*wfa_vendor_extension = 0x10;
		return wifi_utils_success;
	} else {
		return config_bss_by_band(ctx, band, wsc, wfa_vendor_extension);
	}
}

/**
 *  set WSC config for ap-auto configure use.
 *
 * \param  wsc  pointer of  wsc_config.
 * \return  error code.
 */
WIFI_UTILS_STATUS set_wsc_config(void *pctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)pctx;

	if(_1905_set_wireless_setting(ctx) == wapp_utils_error)
		return wifi_utils_error;
#ifdef MAP_R2
	eloop_cancel_timeout(map_r2_notify_ts_config, (void *)ctx, NULL);
	eloop_cancel_timeout(map_notify_transparent_vlan_setting, (void *)ctx, NULL);
	eloop_register_timeout(0, 0, map_r2_notify_ts_config, (void *)ctx, NULL);
	eloop_register_timeout(0, 0, map_notify_transparent_vlan_setting, (void *)ctx, NULL);
#endif

	return wifi_utils_success;
}

WIFI_UTILS_STATUS flash_wsc_config(void *pctx) {
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)pctx;

	debug(DEBUG_OFF, "flash_wsc_config ++ \n");
	_1905_flash_out_config(ctx);

#ifdef MAP_R2
	eloop_cancel_timeout(map_r2_notify_ts_config, (void *)ctx, NULL);
	eloop_cancel_timeout(map_notify_transparent_vlan_setting, (void *)ctx, NULL);
	eloop_register_timeout(0, 0, map_r2_notify_ts_config, (void *)ctx, NULL);
	eloop_register_timeout(0, 0, map_notify_transparent_vlan_setting, (void *)ctx, NULL);
#endif

	return wifi_utils_success;
}


