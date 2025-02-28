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

#ifndef P1905_AP_AUTOCONFIG_H
#define P1905_AP_AUTOCONFIG_H


typedef enum
{
    no_ap_autoconfig = 1,
    wait_4_send_ap_autoconfig_search, //not used
    wait_4_recv_ap_autoconfig_resp,  //not used
    wait_4_send_m1,
    wait_4_recv_m2,
    wait_4_set_config,

} enrolle_config_stage;

typedef enum {
	no_renew_bss = 1,
	wait_4_dump_topo,
	wait_4_pop_node,
	wait_4_send_renew,
	wait_4_check_renew_result,
	wait_4_apply_local,
	/* R3 */
	wait_4_send_bss_reconfig_trigger,
	wait_4_recv_bss_autoconfig_req,
	wait_4_send_bss_autoconfig_resp,
	wait_4_recv_bss_autoconfig_result,
}renew_bss_state;

typedef enum
{
	controller_search_idle = 0,
	bk_link_ready = 1,
	wait_4_send_controller_search,
	wait_4_recv_controller_search_rsp,
	controller_search_done,

} controller_search_stage;

typedef struct _ap_config_para
{
    unsigned char e_nonce[16];
    unsigned char r_nonce[16];
    unsigned char private_key[192];
    unsigned char public_key[192];
    unsigned char security_key[192];
    unsigned char peer_public_key[192];
    unsigned char enrolle_mac[6];

    unsigned char authKey[32];
    unsigned char keyWrapKey[16];
    unsigned short auth_type;
} ap_config_para;

typedef enum
{
    MAP_CONF_UNCONF,
	MAP_CONF_CONFED,
} radio_config_stage;

struct p1905_managerd_ctx;

void ap_controller_search_step(void *eloop_ctx, void *timeout_ctx);
void ap_autoconfig_enrolle_step(void *eloop_ctx, void *timeout_ctx);
void controller_renew_bss_step(void *eloop_ctx, void *timeout_ctx);
void agent_apply_new_config(void *eloop_ctx, void *timeout_ctx);
unsigned char get_left_unconfig_radio(struct p1905_managerd_ctx *ctx);
int cont_handle_autoconfig_wsc(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned char *if_name, unsigned short mid, struct agent_radio_info *r,
	unsigned char radio_cnt, unsigned char *radio_identifier);
int cont_handle_autoconfig_renew(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned char *if_name, unsigned char band);
int cont_handle_autoconfig_response(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
unsigned char *if_name, unsigned char band, unsigned char role, unsigned char profile,
	unsigned char kibmib);
int cont_handle_autoconfig_search(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned char *if_name, unsigned char profile, unsigned short mid);
int cont_handle_autoconfig_wsc_ack(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	unsigned short mid);
WIFI_UTILS_STATUS set_wsc_config(void *pctx);
WIFI_UTILS_STATUS get_wsc_config(void *pctx, WSC_CONFIG* wsc,
	unsigned char *wfa_vendor_extension, struct agent_radio_info *agent_radio);
WIFI_UTILS_STATUS flash_wsc_config(void *pctx);

#endif /* P1905_AP_AUTOCONFIG_H */
