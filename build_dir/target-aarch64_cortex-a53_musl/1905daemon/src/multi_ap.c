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
#include <string.h>
#include <syslog.h>
#include <assert.h>
#include <linux/if_ether.h>
#include <asm/byteorder.h>
#include <stdlib.h>
#include <pthread.h>
#include <regex.h>
#include <sys/ioctl.h>
#include "p1905_database.h"
#include "multi_ap.h"
#include "cmdu_message.h"
#include "cmdu_tlv_parse.h"
#include "cmdu.h"
#include "os.h"
#include "common.h"
#include "debug.h"
#include "_1905_lib_io.h"
#include "file_io.h"
#include "message_wait_queue.h"
#include "topology.h"
#include "eloop.h"
#include "mapfilter_if.h"
#include "ethernet_layer.h"
#include "lldp.h"
#ifdef MAP_R3
#include "security_engine.h"
#include "json.h"
#include "dpp.h"
#include "map_dpp.h"
#include "wpabuf.h"
#include "wpa_extern.h"
#include "map_dpp.h"


unsigned char OUI_WPA2_AKM_8021X[4] = {0x00, 0x0F, 0xAC, 0x01};
unsigned char OUI_WPA2_AKM_PSK[4] = {0x00, 0x0F, 0xAC, 0x02};
unsigned char OUI_WPA2_AKM_FT_8021X[4] = {0x00, 0x0F, 0xAC, 0x03};
unsigned char OUI_WPA2_AKM_FT_PSK[4] = {0x00, 0x0F, 0xAC, 0x04};
unsigned char OUI_WPA2_AKM_8021X_SHA256[4] = {0x00, 0x0F, 0xAC, 0x05};
unsigned char OUI_WPA2_AKM_PSK_SHA256[4] = {0x00, 0x0F, 0xAC, 0x06};
unsigned char OUI_WPA2_AKM_TDLS[4] = {0x00, 0x0F, 0xAC, 0x07};
unsigned char OUI_WPA2_AKM_SAE_SHA256[4] = {0x00, 0x0F, 0xAC, 0x08};
unsigned char OUI_WPA2_AKM_FT_SAE_SHA256[4] = {0x00, 0x0F, 0xAC, 0x09};
unsigned char OUI_WPA2_AKM_SUITEB_SHA256[4] = {0x00, 0x0F, 0xAC, 0x0B};
unsigned char OUI_WPA2_AKM_SUITEB_SHA384[4] = {0x00, 0x0F, 0xAC, 0x0C};
unsigned char OUI_WPA2_AKM_FT_8021X_SHA384[4] = {0x00, 0x0F, 0xAC, 0x0D};
unsigned char OUI_WPA2_AKM_OWE[4] = {0x00, 0x0F, 0xAC, 0x12/*d'18*/};
unsigned char OUI_AKM_DPP[4] = {0x50, 0x6F, 0x9A, 0x02};
/* temporary memory to store r3 onboarding info */
struct r3_onboarding_info r3_ctx_tmp;
#endif

extern unsigned char dev_send_1905_buf[3072*2];
extern int _1905_read_dev_send_1905(struct p1905_managerd_ctx* ctx,
	char* name, unsigned char *almac, unsigned short* _type,
	unsigned short *tlv_len, unsigned char *pay_load);
extern int set_opt_not_forward_dest(unsigned char *inputmac);
struct assoc_client *get_assoc_cli_by_mac(struct p1905_managerd_ctx *ctx, unsigned char mac[]);
struct assoc_client *create_assoc_cli(struct p1905_managerd_ctx *ctx, unsigned char mac[],
	unsigned char *al_mac);
struct assoc_client *add_new_assoc_cli(struct dl_list *clients_table, unsigned char mac[], unsigned char *al_mac);
struct assoc_client *get_cli_by_mac(struct dl_list *clients_table, unsigned char mac[]);
int _1905_interface_set_sock_buf(struct p1905_managerd_ctx *ctx, unsigned short len);

struct global_oper_class
{
	unsigned char opclass;			/* regulatory class */
	unsigned char channel_num;
	unsigned char channel_set[59];	/* max 59 channels, use 0 as terminator */
};

/*Global operating classes*/
 struct global_oper_class oper_class[] = {
	{0, 0, {0}},			/* Invlid entry */
	{81, 13, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}},
	{82, 1, {14}},
	{83, 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
	{84, 9, {5, 6, 7, 8, 9, 10, 11, 12, 13}},
	{94, 2, {133, 137}},
	{95, 4, {132, 134, 136, 138}},
	{96, 8, {131, 132, 133, 134, 135, 136, 137, 138}},
	{101, 2, {21, 25}},
	{102, 5, {11, 13, 15, 17, 19}},
	{103, 10, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
	{104, 2, {184, 192}},
	{105, 2, {188, 196}},
	{106, 2, {191, 195}},
	{107, 5, {189, 191, 193, 195, 197}},
	{108, 10, {188, 189, 190, 191, 192, 193, 194, 195, 196, 197}},
	{109, 4, {184, 188, 192, 196}},
	{110, 7, {183, 184, 185, 186, 187, 188, 189}},
	{111, 8, {182, 183, 184, 185, 186, 187, 188, 189}},
	{112, 3, {8, 12, 16}},
	{113, 5, {7, 8, 9, 10, 11}},
	{114, 6, {6, 7, 8, 9, 10, 11}},
	{115, 4, {36, 40, 44, 48} },
	{116, 2, {36, 44}},
	{117, 2, {40, 48}},
	{118, 4, {52, 56, 60, 64}},
	{119, 2, {52, 60}},
	{120, 2, {56, 64}},
	{121, 12, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144}},
	{122, 6, {100, 108, 116, 124, 132, 140}},
	{123, 6, {104, 112, 120, 128, 136, 144}},
	{124, 4, {149, 153, 157, 161}},
	{125, 6, {149, 153, 157, 161, 165, 169}},
	{126, 2, {149, 157}},
	{127, 2, {153, 161}},
	{128, 6, {42, 58, 106, 122, 138, 155}},
	{129, 2, {50, 114}},
	{130, 6, {42, 58, 106, 122, 138, 155}},
	{131, 59, {1, 5, 9, 13, 17, 21, 25, 29, 33, 37,
				41, 45, 49, 53, 57, 61, 65, 69, 73, 77,
				81, 85, 89, 93, 97, 101, 105, 109, 113, 117,
				121, 125, 129, 133, 137, 141, 145, 149, 153, 157,
				161, 165, 169, 173, 177, 181, 185, 189, 193, 197,
				201, 205, 209, 213, 217, 221, 225, 229, 233}},
	{132, 29, {3, 11, 19, 27, 35, 43, 51, 59, 67, 75,
				83, 91, 99, 107, 115, 123, 131, 139, 147, 155,
				163, 171, 179, 187, 195, 203, 211, 219, 227}},
	{133, 14, {7, 23, 39, 55, 71, 87, 103, 119, 135, 151, 167, 183, 199, 215}},
	{134, 7, {15, 47, 79, 111, 143, 175, 207}},
	{135, 14, {7, 23, 39, 55, 71, 87, 103, 119, 135, 151, 167, 183, 199, 215}},
	{0, 0, {0}}			/* end */
};

unsigned char opclass_2g[] = {
	81, 82, 83, 84, 101, 102, 103, 112, 113, 114, 0
};

unsigned char opclass_5gh[] = {
	94, 95, 96, 104, 105, 106, 107, 108, 109, 110, 111, 121, 122,
	123, 124, 125, 126, 127, 0
};

unsigned char opclass_5gl[] = {
	115, 116, 117, 118, 119, 120, 0
};


int set_bss_config_priority_by_str(struct p1905_managerd_ctx *ctx, char *prio_str)
{
	char *token = NULL;
	unsigned char priority = 1, i = 0;

	token = strtok(prio_str, ";");

	while (token != NULL) {
		for (i = 0; i < ctx->itf_number; i++) {
			if (os_strcmp((char *)ctx->itf[i].if_name, token) == 0) {
				ctx->itf[i].config_priority = priority;
				break;
			}
		}
		if (i >= ctx->itf_number)
			debug(DEBUG_ERROR, "bss intf(%s) not exist in local interface\n", token);

		debug(DEBUG_OFF, "\tbss intf(%s) priority=%d\n", token, priority);
		token = strtok(NULL, ";");
		priority++;
	}

	return 0;
}


int triger_autoconfiguration(struct p1905_managerd_ctx *ctx)
{
	int i = 0;
	unsigned char radio_id = 0;

	if (ctx->current_autoconfig_info.radio_index != -1) {
		radio_id = ctx->current_autoconfig_info.radio_index;
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"another radio("MACSTR") wait it done\n",
			PRINT_MAC(ctx->rinfo[radio_id].identifier));
		return -1;
	}

	for (i = 0; i < ctx->radio_number; i++) {
		if (ctx->rinfo[i].trrigerd_autoconf == 1) {
			ctx->current_autoconfig_info.radio_index = i;
			ctx->enrolle_state = wait_4_send_m1;
			eloop_register_timeout(0, 0, ap_autoconfig_enrolle_step, (void *)ctx, NULL);
			break;
		}
	}
	if (i >= ctx->radio_number) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"auto-configuration done on the radio\n");
		return 2;
	} else {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"trigger radio("MACSTR") to do auto-configuration\n",
			PRINT_MAC(ctx->rinfo[i].identifier));
		return 0;
	}
}
void auto_configuration_done(struct p1905_managerd_ctx *ctx)
{
	unsigned char radio_index = 0;

	if (ctx->current_autoconfig_info.radio_index != -1) {
		radio_index = (unsigned char)ctx->current_autoconfig_info.radio_index;
	} else {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"error radio_index == -1!!!\n");
		return;
	}

	/*set to 0 to stop sm*/
	set_radio_autoconf_trriger(ctx, radio_index, 0);
	ctx->rinfo[radio_index].config_status = MAP_CONF_CONFED;
	ctx->current_autoconfig_info.radio_index = -1;

	triger_autoconfiguration(ctx);
}

int fill_send_tlv(struct p1905_managerd_ctx* ctx, unsigned char* buf, unsigned short len)
{
	if (len >= sizeof(ctx->send_tlv)) {
		debug(DEBUG_ERROR, "send tlv len(%d) > ctx->send_tlv len", len);
		return -1;
	}

	if (ctx->send_tlv_len != 0) {
		debug(DEBUG_ERROR, "receive new send tlv, replace old one(%lu)\n", ctx->send_tlv_cookie);
	}
	ctx->send_tlv_cookie = os_random();
	debug(DEBUG_TRACE, "send_tlv_cookie = %ld, len=%d\n", ctx->send_tlv_cookie, len);
	memcpy(ctx->send_tlv, buf, len);
	ctx->send_tlv_len = len;
	return 0;
}

int reset_send_tlv(struct p1905_managerd_ctx *ctx)
{
	ctx->send_tlv_len = 0;
	return 0;
}

int reset_stored_tlv(struct p1905_managerd_ctx* ctx)
{
	ctx->revd_tlv.buf_len = 0;
	return 0;
}

int store_revd_tlvs(struct p1905_managerd_ctx* ctx, unsigned char *revd_buf, unsigned short len)
{
	if ((ctx->revd_tlv.buf_len + len) > 10240)
		return -1;
	else {
			memcpy(ctx->revd_tlv.buf + ctx->revd_tlv.buf_len, revd_buf, len);
			ctx->revd_tlv.buf_len += len;
			return 0;
		}
}
int get_tlv_len_by_tlvtype(unsigned char *ptlvtype)
{
	unsigned short tlvlen;

	/*jump to tlv len field*/
	tlvlen = *(unsigned short *)(ptlvtype + 1);
	/*endian change*/
	tlvlen = be_to_host16(tlvlen);
	/*add byte of tlytyle & tlvlength*/
	tlvlen += 3;

	return tlvlen;
}

struct radio_info *get_rainfo_by_id(struct p1905_managerd_ctx *ctx, unsigned char* identifier)
{
	int i = 0;

	for (i = 0; i < ctx->radio_number; i++) {
		if (!os_memcmp(ctx->rinfo[i].identifier, identifier, ETH_ALEN))
			return &ctx->rinfo[i];
	}

	return NULL;
}

int delete_exist_radio_basic_capability(
	struct p1905_managerd_ctx *ctx, unsigned char* identifier)
{
	struct radio_info *r = NULL;
	struct operating_class *opc = NULL, *opc_tmp = NULL;

	r = get_rainfo_by_id(ctx, identifier);

	if (!r) {
		debug(DEBUG_ERROR, "radio not found "MACSTR, PRINT_MAC(identifier));
		return -1;
	}

	dl_list_for_each_safe(opc, opc_tmp, &r->basic_cap.op_class_list, struct operating_class, entry) {
		dl_list_del(&opc->entry);
		os_free(opc);
	}

	r->basic_cap.max_bss_num = 0;
	r->basic_cap.op_class_num = 0;

	return 0;
}

int update_radio_basic_capability(struct p1905_managerd_ctx *ctx, struct ap_radio_basic_cap *bcap,
	unsigned short cap_len)
{
	struct radio_info *r = NULL;
	unsigned char i = 0;
	struct operating_class *opc = NULL;

	/*check bcap and its length*/
	if (cap_len != sizeof(struct ap_radio_basic_cap) +
		bcap->op_class_num * sizeof(struct radio_basic_cap)) {
		debug(DEBUG_ERROR, "cap buffer length(%d) is not eqaul to real length(%u)\n",
			cap_len, (unsigned int)(sizeof(struct ap_radio_basic_cap) +
			bcap->op_class_num * sizeof(struct radio_basic_cap)));
		return -1;
	}

	r = get_rainfo_by_id(ctx, bcap->identifier);

	if (!r) {
		debug(DEBUG_ERROR, "radio not found "MACSTR"\n", PRINT_MAC(bcap->identifier));
		return -1;
	}

	r->band = 0;
	r->basic_cap.max_bss_num = bcap->max_bss_num;
	r->basic_cap.op_class_num = bcap->op_class_num;

	for (i = 0; i < bcap->op_class_num; i++) {
		opc = (struct operating_class *)os_zalloc(sizeof(struct operating_class));

		if (!opc) {
			debug(DEBUG_ERROR, "fail to allocate operating class\n");
			continue;
		}
		opc->class_number = bcap->opcap[i].op_class;
		opc->max_tx_pwr = bcap->opcap[i].max_tx_pwr;
		opc->non_operch_num = bcap->opcap[i].non_operch_num;
		os_memcpy(opc->non_operch_list, bcap->opcap[i].non_operch_list, MAX_CH_NUM);
		dl_list_add_tail(&r->basic_cap.op_class_list, &opc->entry);

		r->band |= get_bandcap(ctx, opc->class_number, opc->non_operch_num, opc->non_operch_list);
	}

	debug(DEBUG_TRACE, "radio_band_cap("MACSTR"):%02x\n", PRINT_MAC(r->identifier), r->band);

	return 0;

}

int delete_exist_operational_bss(
	struct p1905_managerd_ctx *ctx, unsigned char *identifier)
{
	 struct operational_bss_db *opcap = NULL;
	 struct op_bss_db *opbss = NULL, *opbss_tmp = NULL;

	debug(DEBUG_TRACE, "delete_exist_operational_bss\n");

	if(!SLIST_EMPTY(&(ctx->ap_cap_entry.oper_bss_head)))
    {
        SLIST_FOREACH(opcap, &(ctx->ap_cap_entry.oper_bss_head), oper_bss_entry)
        {
            if(!memcmp(opcap->identifier, identifier, ETH_ALEN))
            {
                if(!SLIST_EMPTY(&(opcap->op_bss_head)))
                {
                    opbss = SLIST_FIRST(&(opcap->op_bss_head));
                    while(opbss)
                    {
                    	debug(DEBUG_TRACE, "delete_exist struct op_bss_db\n");
						debug(DEBUG_TRACE, "ssid:%s, bssid="MACSTR"\n",
							opbss->ssid, PRINT_MAC(opbss->bssid));
                        opbss_tmp = SLIST_NEXT(opbss, op_bss_entry);
                        SLIST_REMOVE(&(opcap->op_bss_head), opbss,
                                    op_bss_db, op_bss_entry);
                        free(opbss);
                        opbss = opbss_tmp;
                    }
                }
				opcap->oper_bss_num = 0;
				debug(DEBUG_TRACE, "delete_exist struct operational_bss_db\n");
				debug(DEBUG_TRACE, "identifier("MACSTR")\n",
					PRINT_MAC(opcap->identifier));
				debug(DEBUG_TRACE, "oper_bss_num:%d, band:%d\n",
					opcap->oper_bss_num, opcap->band);
                break;
            }
        }
    }

	return wapp_utils_success;
}


int check_sta_in_oper_bss(struct list_head_oper_bss *opbss_head,
		struct map_client_association_event *assc_evt)
{
	struct op_bss_db *bss_info = NULL;
	struct operational_bss_db *operbss_info = NULL;

	SLIST_FOREACH(operbss_info, opbss_head, oper_bss_entry) {
		SLIST_FOREACH(bss_info, &operbss_info->op_bss_head, op_bss_entry)
		{
			if (!memcmp(bss_info->bssid, assc_evt->bssid, ETH_ALEN))
				return 0;
		}
	}
	return -1;
}



int insert_new_operational_bss(
	struct p1905_managerd_ctx *ctx, struct oper_bss_cap *opcap)
{
	struct op_bss_cap *opbss_cap = NULL;
	struct operational_bss_db *opcap_db = NULL;
	struct op_bss_db *opbss_db = NULL;
	int oper_bss_num = 0;
	int i = 0;
#ifdef MAP_R2
	struct operational_bss *bss = NULL;
#endif

	SLIST_FOREACH(opcap_db, &ctx->ap_cap_entry.oper_bss_head, oper_bss_entry)
    {
    	if(!memcmp(opcap_db->identifier, opcap->identifier, ETH_ALEN)) {
			break;
    	}
    }

	if (!opcap_db) {
		opcap_db = (struct operational_bss_db *)malloc(sizeof(struct operational_bss_db));
		if (!opcap_db) {
			debug(DEBUG_ERROR, "alloc struct operational_bss_db fail\n");
			return wapp_utils_error;
		}
		memcpy(opcap_db->identifier, opcap->identifier, ETH_ALEN);
		SLIST_INSERT_HEAD(&ctx->ap_cap_entry.oper_bss_head, opcap_db, oper_bss_entry);
	}

	opcap_db->oper_bss_num = opcap->oper_bss_num;

	debug(DEBUG_TRACE, "identifier("MACSTR")\n",
		PRINT_MAC(opcap->identifier));
	debug(DEBUG_TRACE, "oper_bss_num:%d\n",opcap->oper_bss_num);

	SLIST_INIT(&(opcap_db->op_bss_head));
	oper_bss_num = opcap_db->oper_bss_num;
	for(i = 0; i < oper_bss_num; i++) {
		opbss_cap = &opcap->cap[i];
		opbss_db = (struct op_bss_db *)malloc(sizeof(*opbss_db));
		if (!opbss_db) {
			debug(DEBUG_ERROR, "alloc struct op_bss_db fail\n");
			return wapp_utils_error;
		}
		memset(opbss_db, 0, sizeof(*opbss_db));
		memcpy(opbss_db->bssid, opbss_cap->bssid, ETH_ALEN);
		opbss_db->ssid_len = opbss_cap->ssid_len;
		(void)snprintf((char *)opbss_db->ssid, sizeof(opbss_db->ssid), "%s", opbss_cap->ssid);
		SLIST_INSERT_HEAD(&opcap_db->op_bss_head, opbss_db, op_bss_entry);

		debug(DEBUG_TRACE, "insert struct op_bss_db\n");
		debug(DEBUG_TRACE, "ssid(len=%d):%s, bssid="MACSTR"\n",
				opbss_db->ssid_len, opbss_db->ssid, PRINT_MAC(opbss_db->bssid));
#ifdef MAP_R2
		bss = get_bss_by_bssid(ctx, opbss_cap->bssid);
		if (!bss) {
			bss = create_oper_bss(ctx, opbss_cap->bssid, (char *)opbss_cap->ssid, opbss_db->ssid_len,
				ctx->p1905_al_mac_addr, &ctx->bss_head);

			if (!bss) {
				debug(DEBUG_ERROR, "fail to allocate memory for opertational bss\n");
				continue;
			}

		} else {
			if (os_strncmp(bss->ssid, (char *)opbss_cap->ssid, opbss_db->ssid_len) ||
				opbss_db->ssid_len != os_strlen(bss->ssid)) {
				os_memset(bss->ssid, 0, sizeof(bss->ssid));
				os_strncpy(bss->ssid, (char *)opbss_cap->ssid, opbss_db->ssid_len);
				debug(DEBUG_ERROR, "update ssid to %s for bss("MACSTR")\n",
					opbss_cap->ssid, PRINT_MAC(opbss_cap->bssid));
			}
		}
#endif
	}

#ifdef MAP_R2
	update_operation_bss_vlan(ctx, &ctx->bss_head, &ctx->map_policy.tpolicy,
		ctx->p1905_al_mac_addr);
#endif
	return wapp_utils_success;

}

void delete_legacy_assoc_clients_db(struct p1905_managerd_ctx *ctx)
{
	struct associated_clients_db *assoc_cli_db = NULL, *assoc_cli_db_tmp = NULL;
	struct operational_bss_db *opcap_db = NULL;
	struct op_bss_db *bss_info = NULL;
	struct clients_db *cli_db = NULL, *cli_db_tmp = NULL;
	char bss_find = 0;

	assoc_cli_db = SLIST_FIRST(&ctx->ap_cap_entry.assoc_clients_head);
	while (assoc_cli_db) {
		bss_find = 0;
		assoc_cli_db_tmp = SLIST_NEXT(assoc_cli_db, assoc_clients_entry);
		SLIST_FOREACH(opcap_db, &ctx->ap_cap_entry.oper_bss_head, oper_bss_entry) {
			SLIST_FOREACH(bss_info, &opcap_db->op_bss_head, op_bss_entry) {
				if (!os_memcmp(bss_info->bssid, assoc_cli_db->bssid, ETH_ALEN)) {
					bss_find = 1;
					break;
				}
			}
			if (bss_find)
				break;
		}
		if (!bss_find) {
			SLIST_REMOVE(&ctx->ap_cap_entry.assoc_clients_head, assoc_cli_db,
			associated_clients_db, assoc_clients_entry);
			debug(DEBUG_OFF, "free assoc_cli_db("MACSTR")\n",
				PRINT_MAC(assoc_cli_db->bssid));
			cli_db = SLIST_FIRST(&assoc_cli_db->clients_head);
			while (cli_db != NULL) {
				cli_db_tmp = SLIST_NEXT(cli_db, clients_entry);
				debug(DEBUG_TRACE, "free cli_db mac["MACSTR"]\n",
					PRINT_MAC(cli_db->mac));
				os_free(cli_db);
				cli_db = cli_db_tmp;
			}
			os_free(assoc_cli_db);
		}
		assoc_cli_db = assoc_cli_db_tmp;
	}
}

int update_assoc_sta_info(struct p1905_managerd_ctx *ctx,
	struct map_client_association_event *cinfo)
{
	struct associated_clients_db *bss_db = NULL;
	struct associated_clients_db *bss_db_tmp = NULL;
	struct clients_db *sta_db = NULL;

	SLIST_FOREACH(bss_db, &(ctx->ap_cap_entry.assoc_clients_head), assoc_clients_entry)
	{
		if(!memcmp(bss_db->bssid, cinfo->bssid, ETH_ALEN))
			break;
	}
	if(bss_db == NULL)
	{
		debug(DEBUG_TRACE, "bss("MACSTR") not exist, create new one\n",
			PRINT_MAC(cinfo->bssid));
		bss_db = (struct associated_clients_db*)malloc(sizeof(struct associated_clients_db));
		if(bss_db == NULL)
		{
			debug(DEBUG_ERROR, "alloc memory fail\n");
			return -1;
		}
		memset(bss_db, 0, sizeof(struct associated_clients_db));
		memcpy(bss_db->bssid, cinfo->bssid, ETH_ALEN);
		SLIST_INIT(&(bss_db->clients_head));
		SLIST_INSERT_HEAD(&(ctx->ap_cap_entry.assoc_clients_head), bss_db, assoc_clients_entry);
	}
	bss_db_tmp = bss_db;
	bss_db = NULL;

	SLIST_FOREACH(bss_db, &(ctx->ap_cap_entry.assoc_clients_head), assoc_clients_entry)
	{
		if (os_memcmp(bss_db->bssid, cinfo->bssid, ETH_ALEN))
			continue;
		SLIST_FOREACH(sta_db, &(bss_db->clients_head), clients_entry)
		{
			if(!memcmp(sta_db->mac, cinfo->sta_mac, ETH_ALEN))
			{
				/*free all stainfo for assoc req frame len may be different*/
				SLIST_REMOVE(&(bss_db->clients_head), sta_db, clients_db, clients_entry);
				free(sta_db);
				bss_db->assco_clients_num--;
				debug(DEBUG_TRACE, "delete sta("MACSTR") in bss("MACSTR")\n",
					PRINT_MAC(cinfo->sta_mac), PRINT_MAC(bss_db->bssid));
				break;
			}
		}
	}

	if(cinfo->assoc_evt == 0x80) /*station connect to bss*/
	{
		sta_db = (struct clients_db *)malloc(sizeof(struct clients_db) + cinfo->assoc_req_len);
		if (!sta_db) {
			debug(DEBUG_ERROR, "fail to allock memory for sta_db!\n");
			return -1;
		}
		memcpy(sta_db->mac, cinfo->sta_mac, ETH_ALEN);
		sta_db->time = cinfo->assoc_time;
		sta_db->frame_len = cinfo->assoc_req_len;
		memcpy(sta_db->frame, cinfo->assoc_req, cinfo->assoc_req_len);

		debug(DEBUG_TRACE, "insert sta("MACSTR") in bss("MACSTR")\n",
			PRINT_MAC(cinfo->sta_mac), PRINT_MAC(cinfo->bssid));

		SLIST_INSERT_HEAD(&(bss_db_tmp->clients_head), sta_db, clients_entry);
		bss_db_tmp->assco_clients_num++;
	}

	SLIST_FOREACH(bss_db, &ctx->ap_cap_entry.assoc_clients_head, assoc_clients_entry) {
		if (!os_memcmp(bss_db->bssid, cinfo->bssid, ETH_ALEN) && bss_db->assco_clients_num == 0) {
			SLIST_REMOVE(&ctx->ap_cap_entry.assoc_clients_head, bss_db, associated_clients_db, assoc_clients_entry);
			debug(DEBUG_OFF, "free bss_db("MACSTR")\n", PRINT_MAC(bss_db->bssid));
			os_free(bss_db);
			break;
		}
	}

	return 0;
}

/**
  * @service: 1 for controller only; 2 for agent only; 3 for both controller and agent
  */
unsigned short append_supported_service_tlv(
        unsigned char *pkt, unsigned char service)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;

    temp_buf = pkt;

    *temp_buf = SUPPORTED_SERVICE_TLV_TYPE;
    temp_buf +=1;
	/**
	  * need communicate with WAPP to get supported service
	  * or just write hard code to Multi-AP Agent
	  */
	if (service == 0) {
		*(unsigned short *)(temp_buf) = host_to_be16(SUPPORTED_SERVICE_LENGTH);
    	temp_buf += 2;
		*temp_buf++ = 1;
    	*temp_buf = SERVICE_CONTROLLER;
    	total_length = SUPPORTED_SERVICE_LENGTH + 3;
	} else if (service == 1) {
		*(unsigned short *)(temp_buf) = host_to_be16(SUPPORTED_SERVICE_LENGTH);
    	temp_buf += 2;
		*temp_buf++ = 1;
    	*temp_buf = SERVICE_AGENT;
    	total_length = SUPPORTED_SERVICE_LENGTH + 3;
	} else if (service == 2) {
		*(unsigned short *)(temp_buf) = host_to_be16(SUPPORTED_SERVICE2_LENGTH);
    	temp_buf += 2;
		*temp_buf++ = 2;
    	*temp_buf++ = SERVICE_CONTROLLER;
		*temp_buf = SERVICE_AGENT;
    	total_length = SUPPORTED_SERVICE2_LENGTH + 3;
	} else {
		debug(DEBUG_ERROR, "unvalid service\n");
	}

    return total_length;
}

unsigned short append_controller_cap_tlv(
		unsigned char *pkt, unsigned char kibmib)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;

	temp_buf = pkt;

	*temp_buf = CONTROLLER_CAPABILITY_TYPE;
	temp_buf += 1;

	temp_buf += 2;

	if (kibmib)
		*temp_buf = 0x80;
	else
		*temp_buf = 0x00;
	temp_buf += 1;

	/* Reserved for future expansion */
	*temp_buf = 0;
	temp_buf += 1;

	total_length = temp_buf - pkt;
	*(unsigned short *)(pkt + 1) = host_to_be16(total_length - 3);
	return total_length;
}

unsigned short append_searched_service_tlv(
        unsigned char *pkt)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;

    temp_buf = pkt;

    *temp_buf = SEARCHED_SERVICE_TLV_TYPE;
    temp_buf +=1;

	*(unsigned short *)(temp_buf) = host_to_be16(SEARCHED_SERVICE_LENGTH);
    temp_buf += 2;
	*temp_buf++ = 1;
    *temp_buf = SERVICE_CONTROLLER;

    total_length = SEARCHED_SERVICE_LENGTH + 3;

    return total_length;
}
/**
  * @identifier: 6 byte of Radio Unique Identifier of a radio of the Agent
  */
unsigned short append_ap_radio_identifier_tlv(
        unsigned char *pkt, unsigned char *identifier)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;

    temp_buf = pkt;

    *temp_buf = AP_RADIO_IDENTIFIER_TYPE;
    temp_buf +=1;

	*(unsigned short *)(temp_buf) = host_to_be16(AP_RADIO_IDENTIFIER_LENGTH);
    temp_buf += 2;
	memcpy(temp_buf,identifier,ETH_ALEN);

    total_length = AP_RADIO_IDENTIFIER_LENGTH + 3;

    return total_length;
}

unsigned short append_ap_radio_basic_capability_tlv(struct p1905_managerd_ctx* ctx, unsigned char *pkt,
	struct radio_info *r)
{
	unsigned short total_length = 0, tmp_len = 0;
    unsigned char *temp_buf;
	struct operating_class *opc = NULL;

	temp_buf = pkt;
	*temp_buf = AP_RADIO_BASIC_CAPABILITY_TYPE;
    temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, r->identifier, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;


	temp_buf[0] = r->bss_number;
	temp_buf += 1;
	total_length += 1;

	temp_buf[0] = r->basic_cap.op_class_num;
	temp_buf += 1;
	total_length += 1;

	dl_list_for_each(opc, &r->basic_cap.op_class_list, struct operating_class, entry) {
		tmp_len = 3 + opc->non_operch_num;
		memcpy(temp_buf, opc, tmp_len);
		temp_buf += tmp_len;
		total_length += tmp_len;
	}

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

	return total_length;
}


/**
  * @cap1(0 or 1): cap of unassociated sta link metrics reporting on the channels its BSSs are
  * currently operating on
  * @cap2(0 or 1): cap of unassociated sta link metrics reporting on the channels its BSSs are
  * not currently operating on
  * @cap3(0 or 1): cap of Agent-initiated RSSI-based Steering
  */
unsigned short append_ap_capability_tlv(
        unsigned char *pkt, struct ap_capability *ap_cap)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;

    temp_buf = pkt;

    *temp_buf = AP_CAPABILITY_TYPE;
    temp_buf +=1;

	*(unsigned short *)(temp_buf) = host_to_be16(AP_CAPABILITY_LENGTH);
    temp_buf += 2;

	/*if big endian is correct*/
	*temp_buf = (ap_cap->sta_report_on_cop << 7) | (ap_cap->sta_report_not_cop << 6) |
		(ap_cap->rssi_steer << 5);

    total_length = AP_CAPABILITY_LENGTH + 3;

    return total_length;
}

unsigned short append_ap_ht_capability_tlv(
        unsigned char *pkt, struct ap_ht_cap_db* ht_cap)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;

    temp_buf = pkt;

    *temp_buf++ = AP_HT_CAPABILITY_TYPE;

	*(unsigned short *)(temp_buf) = host_to_be16(AP_HT_CAPABILITY_LENGTH);
    temp_buf += 2;
	memcpy(temp_buf, ht_cap->identifier, ETH_ALEN);
	temp_buf += ETH_ALEN;

	*temp_buf = (ht_cap->tx_stream << 6) | (ht_cap->rx_stream << 4) |
		(ht_cap->sgi_20 << 3) | (ht_cap->sgi_40 << 2) | (ht_cap->ht_40 << 1);

    total_length = AP_HT_CAPABILITY_LENGTH + 3;

    return total_length;
}

unsigned short append_ap_vht_capability_tlv(
        unsigned char *pkt, struct ap_vht_cap_db *vht_cap)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf = NULL;

    temp_buf = pkt;

    *temp_buf = AP_VHT_CAPABILITY_TYPE;
    temp_buf += 1;

	*(unsigned short *)(temp_buf) = host_to_be16(AP_VHT_CAPABILITY_LENGTH);
    temp_buf += 2;
	memcpy(temp_buf, vht_cap->identifier, ETH_ALEN);
	temp_buf += ETH_ALEN;
	*(unsigned short *)(temp_buf) = host_to_be16(vht_cap->vht_tx_mcs);
	temp_buf += 2;
	*(unsigned short *)(temp_buf) = host_to_be16(vht_cap->vht_rx_mcs);
	temp_buf += 2;
	*temp_buf = (vht_cap->tx_stream << 5) | (vht_cap->rx_stream << 2) |
		(vht_cap->sgi_80 << 1) | (vht_cap->sgi_160);
	temp_buf++;
	*temp_buf = (vht_cap->vht_8080 << 7) | (vht_cap->vht_160 << 6) |
		(vht_cap->su_beamformer << 5) | (vht_cap->mu_beamformer << 4);

    total_length = AP_VHT_CAPABILITY_LENGTH + 3;

    return total_length;
}

unsigned short append_ap_he_capability_tlv(
        unsigned char *pkt, struct ap_he_cap_db *he_cap)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf = NULL;

    temp_buf = pkt;

    *temp_buf++ = AP_HE_CAPABILITY_TYPE;

	*(unsigned short *)(temp_buf) = host_to_be16(9 + he_cap->he_mcs_len);
    temp_buf += 2;
	memcpy(temp_buf, he_cap->identifier, ETH_ALEN);
	temp_buf += ETH_ALEN;

	*temp_buf++ = he_cap->he_mcs_len;
	os_memcpy(temp_buf, he_cap->he_mcs, he_cap->he_mcs_len);
	temp_buf += he_cap->he_mcs_len;
	*temp_buf++ = (he_cap->tx_stream << 5) | (he_cap->rx_stream << 2) |
		(he_cap->he_8080 << 1) | (he_cap->he_160);
	*temp_buf++ = (he_cap->su_bf_cap << 7) | (he_cap->mu_bf_cap << 6) |
		(he_cap->ul_mu_mimo_cap << 5) | (he_cap->ul_mu_mimo_ofdma_cap << 4) |
		(he_cap->dl_mu_mimo_ofdma_cap << 3) | (he_cap->ul_ofdma_cap << 2) |
		(he_cap->dl_ofdma_cap << 1);

    total_length = 12 + he_cap->he_mcs_len;

    return total_length;
}

unsigned short append_client_steering_request_tlv(unsigned char *pkt)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;
	unsigned char BSSID[6] = {0x00,0x0C,0x43,0x26,0x60,0x98};
	unsigned char TBSSID[6] = {0x00,0x0C,0x43,0x26,0x60,0x60};
	unsigned char STA[6] = {0x38,0xBC,0x1A,0xC1,0xD3,0x40};

    temp_buf = pkt;

    *temp_buf++ = STEERING_REQUEST_TYPE;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, BSSID, 6);
	temp_buf += 6;
	total_length += 6;

	*temp_buf++ = 0xe0;
	total_length += 1;

	*temp_buf++ = 0x00;
	*temp_buf++ = 0x05;
	total_length += 2;

	/*sta num & sta mac*/
	*temp_buf++ = 1;
	total_length += 1;
	memcpy(temp_buf, STA, 6);
	temp_buf += 6;
	total_length += 6;

	/*bssid num & bssid mac, opclass, channel*/
	*temp_buf++ = 1;
	total_length += 1;
	memcpy(temp_buf, TBSSID, 6);
	temp_buf += 6;
	total_length += 6;
	*temp_buf++ = 115;
	*temp_buf++ = 36;
	total_length += 2;

	/*calculate totoal length & fill into the length field*/
    *(pkt + 1) =  (unsigned char)(((total_length - 3) >> 8) & 0xff);
    *(pkt + 2) =  (unsigned char)((total_length - 3) & 0xff);

    return total_length;
}

unsigned short append_client_steering_btm_report_tlv(unsigned char *pkt,
	struct cli_steer_btm_event *btm_evt)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;
	unsigned char zero_bssid[ETH_ALEN] = {0};

    temp_buf = pkt;

    *temp_buf++ = STEERING_BTM_REPORT_TYPE;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, btm_evt->bssid, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	memcpy(temp_buf, btm_evt->sta_mac, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	*temp_buf++ = btm_evt->status;
	total_length += 1;

	if (memcmp(zero_bssid, btm_evt->tbssid, ETH_ALEN)) {
		memcpy(temp_buf, btm_evt->tbssid, ETH_ALEN);
		temp_buf += ETH_ALEN;
		total_length += ETH_ALEN;
	}

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

    return total_length;
}

unsigned short append_map_policy_config_request_tlv(unsigned char *pkt)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;
	unsigned char BSSID[ETH_ALEN] = {0x00,0x0C,0x43,0x26,0x60,0x98};

	temp_buf = pkt;

	*temp_buf++ = STEERING_POLICY_TYPE;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	*temp_buf++ = 0;
	total_length += 1;

	*temp_buf++ = 0;
	total_length += 1;

	*temp_buf++ = 1;
	total_length += 1;
	memcpy(temp_buf, BSSID, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;
	*temp_buf++ = 0x01;
	*temp_buf++ = 0xff;
	*temp_buf++ = 20;
	total_length += 3;

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

	return total_length;
}

unsigned short append_cli_assoc_cntrl_request_tlv(unsigned char *pkt)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;
	unsigned char BSSID[ETH_ALEN] = {0x00,0x0C,0x43,0x26,0x60,0x98};
	unsigned char STA[ETH_ALEN] = {0x38,0xBC,0x1A,0xC1,0xD3,0x40};

	temp_buf = pkt;

	*temp_buf++ = CLI_ASSOC_CONTROL_REQUEST_TYPE;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, BSSID, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	*temp_buf++ = 0;
	total_length += 1;

	*temp_buf++ = 0;
	*temp_buf++ = 30;
	total_length += 2;

	*temp_buf++ = 1;
	total_length += 1;

	memcpy(temp_buf, STA, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;


	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

	return total_length;
}

unsigned short append_channel_preference_tlv(unsigned char *pkt,
	struct ch_prefer_db *prefer_db)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;
	struct prefer_info_db *prefer_info = NULL;

	if (!prefer_db) {
		debug(DEBUG_ERROR, "NULL pointer prefer_db!\n");
		return 0;
	}

	temp_buf = pkt;

  /*firstly shift to tlv value*/
	temp_buf += 3;
	total_length += 3;

	memcpy(temp_buf, prefer_db->identifier, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	*temp_buf++ = prefer_db->op_class_num;
	total_length += 1;
	if (!SLIST_EMPTY(&prefer_db->prefer_info_head)) {
		SLIST_FOREACH(prefer_info, &prefer_db->prefer_info_head, prefer_info_entry)
		{
			/*operating class*/
			*temp_buf++ = prefer_info->op_class;
			total_length += 1;
			/*channel num*/
			*temp_buf++ = prefer_info->ch_num;
			total_length += 1;
			/*channel list*/
			if (prefer_info->ch_num) {
				memcpy(temp_buf, prefer_info->ch_list, prefer_info->ch_num);
				temp_buf += prefer_info->ch_num;
				total_length += prefer_info->ch_num;
			}
			/*preference & reason*/
			*temp_buf++ = (prefer_info->perference << 4) | prefer_info->reason;
			total_length += 1;
		}
	} else {
		debug(DEBUG_ERROR, "error! prefer_info_head==NULL\n");
		return 0;
	}

	/*if really has the p1905.1 neighbor device, append type & length */
	*pkt = CH_PREFERENCE_TYPE;
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

	return total_length;
}

unsigned short append_transmit_power_limit_tlv(unsigned char *pkt,
	unsigned char *identifier, signed char tx_power_limit)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;

	temp_buf = pkt;

	*temp_buf++ = TRANSMIT_POWER_LIMIT_TYPE;
	total_length += 1;

	*(unsigned short *)(temp_buf) = host_to_be16(TRANSMIT_POWER_LIMIT_LENGTH);
	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, identifier, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	*temp_buf++ = tx_power_limit;
	total_length += 1;

	return total_length;
}

unsigned short append_radio_operation_restriction_tlv(unsigned char *pkt,
	struct oper_restrict_db *restriction_db)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;
	struct restrict_db *restrict_info = NULL;
	int i = 0;

	if (!restriction_db) {
		debug(DEBUG_ERROR, "NULL pointer restriction_db!\n");
		return 0;
	}

	temp_buf = pkt;

  /*firstly shift to tlv value*/
	temp_buf += 3;
	total_length += 3;

	memcpy(temp_buf, restriction_db->identifier, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	*temp_buf++ = restriction_db->op_class_num;
	total_length += 1;

	if (!SLIST_EMPTY(&restriction_db->restrict_head)) {
		SLIST_FOREACH(restrict_info, &restriction_db->restrict_head, restrict_entry)
		{
			*temp_buf++ = restrict_info->op_class;
			total_length += 1;
			*temp_buf++ = restrict_info->ch_num;
			total_length += 1;
			for (i = 0; i < restrict_info->ch_num; i++) {
				*temp_buf++ = restrict_info->ch_list[i];
				*temp_buf++ = restrict_info->min_fre_sep[i];
				total_length += 2;
			}
		}
	} else {
		debug(DEBUG_ERROR, "error! restrict_head==NULL\n");
		return 0;
	}

	/*if really has the p1905.1 neighbor device, append type & length */
	*pkt = RADIO_OPERATION_RESTRICTION_TYPE;
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

	return total_length;
}


unsigned short append_channel_selection_response_tlv(unsigned char *pkt,
	struct ch_stat_info *stat)
{
	unsigned short total_length = 0;
    unsigned char *temp_buf;

	temp_buf = pkt;

	*temp_buf = CH_SELECTION_RESPONSE_TYPE;
    temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, stat->identifier, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	*temp_buf = stat->code;
	temp_buf += 1;
	total_length += 1;

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

	return total_length;
}

unsigned short append_operating_channel_report_tlv(unsigned char *pkt,
	struct ch_rep_info *rep_info)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;

	temp_buf = pkt;

	*temp_buf = OPERATING_CHANNEL_REPORT_TYPE;
	temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, rep_info->identifier, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	*temp_buf = 1;
	temp_buf += 1;
	total_length += 1;

	*temp_buf = rep_info->op_class;
	temp_buf += 1;
	total_length += 1;

	*temp_buf = rep_info->channel;
	temp_buf += 1;
	total_length += 1;

	*temp_buf = rep_info->tx_power;
	temp_buf += 1;
	total_length += 1;

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

	return total_length;
}

unsigned short append_operating_channel_report_tlv_bw80(unsigned char *pkt,
	struct ch_rep_info *rep_info)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;

	temp_buf = pkt;

	*temp_buf = OPERATING_CHANNEL_REPORT_TYPE;
	temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, rep_info->identifier, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	*temp_buf = 2;
	temp_buf += 1;
	total_length += 1;

	*temp_buf = rep_info->op_class;
	temp_buf += 1;
	total_length += 1;

	*temp_buf = rep_info->channel;
	temp_buf += 1;
	total_length += 1;
	/*primary channel*/
	rep_info++;
	*temp_buf = rep_info->op_class;
	temp_buf += 1;
	total_length += 1;

	*temp_buf = rep_info->channel;
	temp_buf += 1;
	total_length += 1;

	rep_info--;
	*temp_buf = rep_info->tx_power;
	temp_buf += 1;
	total_length += 1;

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

	return total_length;
}

#ifdef MAP_R4_SPT
unsigned short append_spatial_report_tlv(unsigned char *pkt,
	struct ap_spt_reuse_resp *rep_info)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;

	temp_buf = pkt;

	*temp_buf = SPATIAL_REUSE_REPORT_TYPE;
	temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, rep_info->identifier, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	*temp_buf = (rep_info->partial_bss_color ? 0x40:0) | (rep_info->bss_color & 0x3F);
	temp_buf += 1;
	total_length += 1;

	*temp_buf = (rep_info->hesiga_spa_reuse_val_allowed? 0x10:0)|
				(rep_info->srg_info_valid? 0x08:0)|
				(rep_info->nonsrg_offset_valid? 0x04:0)|
				(rep_info->psr_disallowed? 0x01:0);
	temp_buf += 1;
	total_length += 1;

	if(rep_info->nonsrg_offset_valid)
	{
		*temp_buf = rep_info->nonsrg_obsspd_max_offset;
	}
	temp_buf += 1;
	total_length += 1;

	if(rep_info->srg_info_valid)
	{
		*temp_buf = rep_info->srg_obsspd_min_offset;
		temp_buf += 1;
		total_length += 1;

		*temp_buf = rep_info->srg_obsspd_max_offset;
		temp_buf += 1;
		total_length += 1;

		memcpy(temp_buf, rep_info->srg_bss_color_bitmap, 8);
		temp_buf += 8;
		total_length += 8;

		memcpy(temp_buf, rep_info->srg_partial_bssid_bitmap, 8);
		temp_buf += 8;
		total_length += 8;
	}
	else
	{
		temp_buf += 18;
		total_length += 18;
	}

	memcpy(temp_buf, rep_info->ngh_bss_color_in_bitmap, 8);
	temp_buf += 8;
	total_length += 8;

	*temp_buf = 0;
	temp_buf += 1;
	*temp_buf = 0;
	temp_buf += 1;

	total_length += 2;

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

	return total_length;
}
#endif

unsigned short channel_selection_response_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{

	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CHANNEL_SELECTION_RESPONSE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short channel_selection_request_message(unsigned char *buf,
	struct agent_list_db *agent, struct p1905_managerd_ctx *ctx)

{

	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;
	struct ch_prefer_db *chcap = NULL;

	msg_hdr = (cmdu_message_header*)buf;

	if(!ctx->send_tlv_len)
	{
		SLIST_FOREACH(chcap, &agent->ch_prefer_head, ch_prefer_entry)
		{
			length = append_channel_preference_tlv(tlv_temp_buf, chcap);
			total_tlvs_length += length;
			tlv_temp_buf += length;
			if (chcap->tx_power_limit) {
				length = append_transmit_power_limit_tlv(tlv_temp_buf,
					chcap->identifier, chcap->tx_power_limit);
				total_tlvs_length += length;
				tlv_temp_buf += length;
			}
		}
	}
	else
	{
		length = append_send_tlv(tlv_temp_buf, ctx);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}
	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CHANNEL_SELECTION_REQUEST);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short combined_infrastructure_metrics_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx, unsigned char *dalmac)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(COMBINED_INFRASTRUCTURE_METRICS);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short client_steering_request_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{

	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CLIENT_STEERING_REQUEST);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short client_steering_btm_report_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{

	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_client_steering_btm_report_tlv(tlv_temp_buf, &ctx->sinfo.sbtm_evt);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CLIENT_STEERING_BTM_REPORT);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short client_steering_completed_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{

	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CLIENT_STEERING_COMPLETED);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short map_policy_config_request_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx, unsigned char *al_mac)
{

	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

#ifdef MAP_R2
	if (ctx->MAP_Cer) {
		/* append default 8021Q setting tlv and ts policy tlv */
		length = append_default_8021Q_tlv(ctx, tlv_temp_buf, al_mac,ctx->bss_config_num, ctx->bss_config);
		total_tlvs_length += length;
		tlv_temp_buf += length;

		length = append_traffic_separation_tlv(ctx, tlv_temp_buf, al_mac,ctx->bss_config_num, ctx->bss_config);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}
#endif

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(MAP_POLICY_CONFIG_REQUEST);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short client_association_control_request_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{

	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CLIENT_ASSOC_CONTROL_REQUEST);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short channel_preference_report_message(
        unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;

    msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

    length = append_end_of_tlv(tlv_temp_buf);
    total_tlvs_length += length;
    tlv_temp_buf += length;

    /*tlvs size is less than (46(minimun ethernet frame payload size)
     *-8(cmdu header size)) ==>padding
     */
    if(total_tlvs_length < MIN_TLVS_LENGTH)
    {
	    memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
	    total_tlvs_length = MIN_TLVS_LENGTH;
    }

    //0x00: for this version of the specification
    //0x01~0xFF: Reserved Values
    msg_hdr->message_version = 0x0;

    //set reserve field to 0
    msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CHANNEL_PREFERENCE_REPORT);
    msg_hdr->relay_indicator = 0x0;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

unsigned int is_valid_primary_ch_80M_160M(unsigned char ch, unsigned char center_ch, unsigned char op)
{
	int offset = 0;

	offset = ch - center_ch;
	if (op == 128) {
		if ((abs(offset) == 6) || (abs(offset) == 2))
			return 1;
		else
			return 0;
	} else if (op == 129) {
		if ((abs(offset) == 14) || (abs(offset) == 10) ||
			(abs(offset) == 6) || (abs(offset) == 2)) {
			return 1;
		} else {
			return 0;
		}
	}
	return 0;
}


unsigned short operating_channel_report_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{

	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;
	int i = 0;

	msg_hdr = (cmdu_message_header*)buf;
	for (i = 0; i < ctx->ch_rep->ch_rep_num; i++) {
		if (ctx->ch_rep->info[i].channel) {
			if ((ctx->ch_rep->info[i].op_class == 128 || ctx->ch_rep->info[i].op_class == 129)
				&& (i < ctx->ch_rep->ch_rep_num - 1)) {
				if (is_valid_primary_ch_80M_160M(ctx->ch_rep->info[i+1].channel, ctx->ch_rep->info[i].channel,
								ctx->ch_rep->info[i].op_class)) {
					/*operating channel report for bw 80 case*/
					length = append_operating_channel_report_tlv_bw80(tlv_temp_buf, &ctx->ch_rep->info[i]);
					i++;
				} else {
					length = append_operating_channel_report_tlv(tlv_temp_buf, &ctx->ch_rep->info[i]);
				}
			} else {
				length = append_operating_channel_report_tlv(tlv_temp_buf, &ctx->ch_rep->info[i]);
			}
			total_tlvs_length += length;
			tlv_temp_buf += length;
		}
	}
#ifdef MAP_R4_SPT
	if(ctx->spt_report) {
		for (i = 0; i < ctx->spt_report->spt_rep_num; i++) {

				length = append_spatial_report_tlv(tlv_temp_buf, &ctx->spt_report->spt_reuse_report[i]);
				total_tlvs_length += length;
				tlv_temp_buf += length;
			debug(DEBUG_TRACE, GRN("[1905 bm index %d]")"bitmap "
					"0x%2x:%2x:%2x:%2x:%2x:%2x:%2x:%2x\n",i,
					ctx->spt_report->spt_reuse_report[i].srg_bss_color_bitmap[0],
					ctx->spt_report->spt_reuse_report[i].srg_bss_color_bitmap[1],
					ctx->spt_report->spt_reuse_report[i].srg_bss_color_bitmap[2],
					ctx->spt_report->spt_reuse_report[i].srg_bss_color_bitmap[3],
					ctx->spt_report->spt_reuse_report[i].srg_bss_color_bitmap[4],
					ctx->spt_report->spt_reuse_report[i].srg_bss_color_bitmap[5],
					ctx->spt_report->spt_reuse_report[i].srg_bss_color_bitmap[6],
					ctx->spt_report->spt_reuse_report[i].srg_bss_color_bitmap[7]
			);
		}
		free(ctx->spt_report);
		ctx->spt_report = NULL;
	}
#endif
	free(ctx->ch_rep);
	ctx->ch_rep = NULL;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(OPERATING_CHANNEL_REPORT);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}


unsigned short channel_preference_query_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{

	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CHANNEL_PREFERENCE_QUERY);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short ap_capability_query_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;

    msg_hdr = (cmdu_message_header*)buf;

    length = append_end_of_tlv(tlv_temp_buf);
    total_tlvs_length += length;
    tlv_temp_buf += length;

    /*tlvs size is less than (46(minimun ethernet frame payload size)
     *-8(cmdu header size)) ==>padding
     */
    if(total_tlvs_length < MIN_TLVS_LENGTH)
    {
	    memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
	    total_tlvs_length = MIN_TLVS_LENGTH;
    }

    //0x00: for this version of the specification
    //0x01~0xFF: Reserved Values
    msg_hdr->message_version = 0x0;

    //set reserve field to 0
    msg_hdr->reserved_field_0 = 0x0;
    msg_hdr->message_type = host_to_be16(AP_CAPABILITY_QUERY);
    msg_hdr->relay_indicator = 0x0;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

unsigned short ap_capability_report_message(
        unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;
	struct ap_ht_cap_db* pdb_ht = NULL;
	struct ap_vht_cap_db* pdb_vht = NULL;
	struct ap_he_cap_db* pdb_he = NULL;
#ifdef MAP_R3
	struct ap_wf6_role_db* pdb_wf6 = NULL;
#endif /*MAP_R3*/
	unsigned char radio_index = 0;
	struct radio_info *r = NULL;

    msg_hdr = (cmdu_message_header*)buf;

    length = append_ap_capability_tlv(tlv_temp_buf, &ctx->ap_cap_entry.ap_cap);
    total_tlvs_length += length;
    tlv_temp_buf += length;


	/*One or more AP HT Capabilities TLV from spec 17.1.7*/
	for (radio_index = 0; radio_index < ctx->radio_number; radio_index++) {
		r = &ctx->rinfo[radio_index];
		debug(DEBUG_TRACE, "radio identifier="MACSTR"\n", PRINT_MAC(r->identifier));

		length = append_ap_radio_basic_capability_tlv(ctx, tlv_temp_buf, r);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}
	/*Zero or more AP HT Capabilities TLV from spec 17.1.7*/
	SLIST_FOREACH(pdb_ht, &(ctx->ap_cap_entry.ap_ht_cap_head), ap_ht_cap_entry)
	{
		debug(DEBUG_TRACE, "pdb_ht->identifier="MACSTR"\n",
			PRINT_MAC(pdb_ht->identifier));
		length = append_ap_ht_capability_tlv(tlv_temp_buf, pdb_ht);
		total_tlvs_length += length;
		tlv_temp_buf += length;

	}
	/*Zero or more AP VHT Capabilities TLV from spec 17.1.7*/
	SLIST_FOREACH(pdb_vht, &(ctx->ap_cap_entry.ap_vht_cap_head), ap_vht_cap_entry)
	{
		debug(DEBUG_TRACE, "pdb_vht->identifier="MACSTR"\n", PRINT_MAC(pdb_vht->identifier));
		length = append_ap_vht_capability_tlv(tlv_temp_buf, pdb_vht);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}

	SLIST_FOREACH(pdb_he, &(ctx->ap_cap_entry.ap_he_cap_head), ap_he_cap_entry)
	{
#ifdef MAP_R2
		if (!(ctx->map_version == DEV_TYPE_R2 && ctx->MAP_Cer)) {
#endif
			debug(DEBUG_TRACE, "pdb_he->identifier="MACSTR"\n",
				PRINT_MAC(pdb_he->identifier));
			length = append_ap_he_capability_tlv(tlv_temp_buf, pdb_he);
			total_tlvs_length += length;
			tlv_temp_buf += length;
#ifdef MAP_R2
		}
#endif
	}

#ifdef MAP_R2
	if (ctx->map_version >= DEV_TYPE_R2) {
		struct ap_r2_capability ap_r2_cap = {0};
		/*one channel scan capability tlv*/
		length = append_channel_scan_capability_tlv(tlv_temp_buf, &(ctx->ap_cap_entry.ch_scan_cap_head));
		total_tlvs_length += length;
		tlv_temp_buf += length;

		/* One CAC Capabilities TLV */
		length = append_cac_capability_tlv(tlv_temp_buf, &(ctx->ap_cap_entry.radio_cac_cap));
		total_tlvs_length += length;
		tlv_temp_buf += length;

		/* One R2 AP Capability TLV  */
		os_memset(&ap_r2_cap, 0, sizeof(ap_r2_cap));
		os_memcpy(&ap_r2_cap, &(ctx->ap_cap_entry.ap_r2_cap), sizeof(ap_r2_cap));

		if ((ctx->cinfo.profile == MAP_PROFILE_R1) && (!ctx->cinfo.kibmib)) {
			ap_r2_cap.byte_counter_units = 0;
			debug(DEBUG_ERROR, "Controller is R1 or Controller not support KIBMIB, byte_counter_units change to bytes\n");
		}
		length = append_r2_cap_tlv(tlv_temp_buf, &ap_r2_cap);
		total_tlvs_length += length;
		tlv_temp_buf += length;

		/* One Metric Collection Interval TLV */
		length = append_metric_collection_interval_tlv(tlv_temp_buf, ctx->metric_entry.metric_collection_interval);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}
#endif // #ifdef MAP_R2

#ifdef MAP_R3
	if (ctx->map_version == DEV_TYPE_R3) {
		SLIST_FOREACH(pdb_wf6, &(ctx->ap_cap_entry.ap_wf6_cap_head), ap_wf6_role_entry) {
			debug(DEBUG_TRACE, "pdb_wf6->identifier="MACSTR"\n",
				PRINT_MAC(pdb_wf6->identifier));
			length = append_ap_wf6_capability_tlv(tlv_temp_buf, pdb_wf6);
			total_tlvs_length += length;
			tlv_temp_buf += length;
		}

		length = append_1905_layer_security_tlv(tlv_temp_buf);
		total_tlvs_length += length;
		tlv_temp_buf += length;
#ifdef MAP_R3_DE
		length = append_dev_inven_tlv(tlv_temp_buf, &(ctx->de));
		total_tlvs_length += length;
		tlv_temp_buf += length;
#endif //MAP_R3_DE
	}
#endif// MAP_R3


    length = append_end_of_tlv(tlv_temp_buf);
    total_tlvs_length += length;
    tlv_temp_buf += length;

    /*tlvs size is less than (46(minimun ethernet frame payload size)
     *-8(cmdu header size)) ==>padding
     */
    if(total_tlvs_length < MIN_TLVS_LENGTH)
    {
	    memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
	    total_tlvs_length = MIN_TLVS_LENGTH;
    }

    //0x00: for this version of the specification
    //0x01~0xFF: Reserved Values
    msg_hdr->message_version = 0x0;

    //set reserve field to 0
    msg_hdr->reserved_field_0 = 0x0;
    msg_hdr->message_type = host_to_be16(AP_CAPABILITY_REPORT);
    msg_hdr->relay_indicator = 0x0;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

int append_error_code_tlv(struct p1905_managerd_ctx* ctx, unsigned char *pkt,
		int error_code, unsigned char *mac)
{
	*pkt++ = ERROR_CODE_TYPE;

	*(unsigned short *)pkt = host_to_be16(ERROR_CODE_TLV_LENGTH);
	pkt += 2;

	*pkt++ = error_code;

	memcpy(pkt, mac, ETH_ALEN);

	return ERROR_CODE_TLV_LENGTH + 3;
}


unsigned short cli_capability_report_message(
        unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;
	int error_code = 0;

    msg_hdr = (cmdu_message_header*)buf;

    length = append_cli_info_tlv(tlv_temp_buf, &ctx->sinfo.cinfo);
    total_tlvs_length += length;
    tlv_temp_buf += length;
	length = append_cli_capability_report_tlv(ctx, tlv_temp_buf, &error_code);
    total_tlvs_length += length;
    tlv_temp_buf += length;

	length = append_error_code_tlv(ctx, tlv_temp_buf, error_code, ctx->sinfo.cinfo.sta_mac);
	total_tlvs_length += length;
	tlv_temp_buf += length;


    length = append_end_of_tlv(tlv_temp_buf);
    total_tlvs_length += length;
    tlv_temp_buf += length;

    /*tlvs size is less than (46(minimun ethernet frame payload size)
     *-8(cmdu header size)) ==>padding
     */
    if(total_tlvs_length < MIN_TLVS_LENGTH)
    {
	    memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
	    total_tlvs_length = MIN_TLVS_LENGTH;
    }

    //0x00: for this version of the specification
    //0x01~0xFF: Reserved Values
    msg_hdr->message_version = 0x0;

    //set reserve field to 0
    msg_hdr->reserved_field_0 = 0x0;
    msg_hdr->message_type = host_to_be16(CLIENT_CAPABILITY_REPORT);
    msg_hdr->relay_indicator = 0x0;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

unsigned short cli_capability_query_message(
        unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;
	struct client_info *cinfo = NULL;

	msg_hdr = (cmdu_message_header*)buf;
	if(ctx->send_tlv_len == 0)
	{
		return total_tlvs_length;
	}
	else
	{
		cinfo = (struct client_info*)ctx->send_tlv;
		reset_send_tlv(ctx);
	}
	length = append_cli_info_tlv(tlv_temp_buf, cinfo);
    total_tlvs_length += length;
    tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CLIENT_CAPABILITY_QUERY);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

int _1905_ack_message(unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

#ifdef MAP_R2
	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;
#endif

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(e_1905_ack);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}


int high_layer_data_message(unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(e_higher_layer_data);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

int dev_send_1905_msg(unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	memcpy(tlv_temp_buf, ctx->pcmdu_tx_buf, ctx->cmdu_tx_buf_len);
	length = ctx->cmdu_tx_buf_len;
	total_tlvs_length += length;
    tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(ctx->cmdu_tx_msg_type);
	if (ctx->cmdu_tx_msg_type == AP_AUTOCONFIG_RENEW || ctx->cmdu_tx_msg_type == TOPOLOGY_NOTIFICATION) {
		msg_hdr->relay_indicator = 0x1;
		debug(DEBUG_TRACE, "renew msg or notification msg,relay_indicator will be turn on\n");
	} else {
		msg_hdr->relay_indicator = 0x0;
	}
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

int parse_supported_service_tlv(unsigned char *service, unsigned short len, unsigned char *value)
{
	unsigned char *temp_buf = value;
	int left = len;

	/* 1 byte: list of supported service(s) */
	if (left < 1)
		return -1;
	left--;

	if (left == 1) {
		temp_buf++;
		*service = *temp_buf;
	} else if (left == 2) {
		temp_buf++;
		*service = 2;
	} else {
		return -1;
	}

	return 0;
}

#ifdef MAP_R3
int parse_controller_cap_tlv(unsigned char *buf, unsigned char *kibmib)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if ((*temp_buf) == CONTROLLER_CAPABILITY_TYPE)
		temp_buf++;
	else
		return -1;

	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);
	temp_buf += 2;

	if (*temp_buf & 0x80)
		*kibmib = 1;
	else
		*kibmib = 0;

	return (length+3);
}
#endif

int get_bandcap(struct p1905_managerd_ctx *ctx, unsigned char opclass,
	unsigned char non_opch_num, unsigned char *non_opch)
{
	unsigned char chnum = 0;
	int i = 0, j = 0, k = 0;
	struct global_oper_class *opcls = oper_class;
	unsigned char *chset = NULL;
	unsigned char cap = BAND_INVALID_CAP;

	do {
		if (opcls[i].opclass == opclass) {
			chnum = opcls[i].channel_num;
			chset = opcls[i].channel_set;
			if (non_opch_num == chnum) {
				debug(DEBUG_TRACE, AUTO_CONFIG_PREX"opclass(%d) all channel not used\n", opclass);
				hex_dump("non_opch", non_opch, non_opch_num);
				cap = BAND_INVALID_CAP;
			} else {
				switch (opclass) {
				case 81:
				case 82:
				case 83:
				case 84:
				case 101:
				case 102:
				case 103:
				case 112:
				case 113:
				case 114:
					cap = BAND_2G_CAP;
				break;
				case 94:
				case 95:
				case 96:
				case 104:
				case 105:
				case 106:
				case 107:
				case 108:
				case 109:
				case 110:
				case 111:
				case 121:
				case 122:
				case 123:
				case 124:
				case 125:
				case 126:
				case 127:
					cap = BAND_5GH_CAP;
				break;
				case 115:
				case 116:
				case 117:
				case 118:
				case 119:
				case 120:
					cap = BAND_5GL_CAP;
				break;
				case 128:
				case 129:
				case 130:
					hex_dump("chset",chset,chnum);
					hex_dump("non_opch",non_opch,non_opch_num);
					for (j = 0; j < chnum; j++) {
						for (k = 0; k < non_opch_num; k++) {
							if (chset[j] == non_opch[k])
								break;
						}
						if ((k == non_opch_num) && (!ctx->MAP_Cer)) {
							if (chset[j] < 100)
								cap |= BAND_5GL_CAP;
							else
								cap |= BAND_5GH_CAP;
							debug(DEBUG_TRACE, AUTO_CONFIG_PREX"opclass=%d channel(%d) support cap=%d\n", opclass, chset[j], cap);
						}
					}
					break;
				case 131:
				case 132:
				case 133:
				case 134:
				case 135:
					for (j = 0; j < chnum; j++) {
						for (k = 0; k < non_opch_num; k++) {
							if (chset[j] == non_opch[k])
								break;
						}
						if ((k == non_opch_num) && (!ctx->MAP_Cer)) {
							cap |= BAND_6G_CAP;
							debug(DEBUG_TRACE, AUTO_CONFIG_PREX"opclass=%d channel(%d) support cap=%d\n", opclass, chset[j], cap);
							break;
						}
					}
					break;
				default:
					debug(DEBUG_TRACE, AUTO_CONFIG_PREX"unknown opclass %d\n", opclass);
				break;

				}
				debug(DEBUG_TRACE, AUTO_CONFIG_PREX"opclass=%d band_cap=%d\n", opclass, cap);
			}
			break;
		}
		i++;
	} while (opcls[i].opclass != 0);

	return cap;
}


int parse_ap_radio_basic_cap_tlv(struct p1905_managerd_ctx *ctx, struct agent_radio_info *rinfo,
					unsigned short len, unsigned char *value)
{
	int left = len;
	unsigned char *temp_buf = value;
	unsigned char opnum = 0, i = 0, non_opch_num = 0;
	unsigned char opclass = 0;
	unsigned char cap = 0;

	if (left < ETH_ALEN + 2)
		return -1;
	left -= ETH_ALEN + 2;

	/*here only parse identifier of ap_radio_basic_cap_tlv*/
	os_memcpy(rinfo->identifier, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	rinfo->max_bss_num = *temp_buf++;
	/*Number of operating classes*/
	opnum = *temp_buf++;
	rinfo->op_class_num = opnum;

	if (opnum > MAX_OP_CLASS_NUM) {
		debug(DEBUG_ERROR, "opnum (%d) is large than MAX_OP_CLASS_NU)\n", opnum);
		return -1;
	}

	for (i = 0; i < opnum; i++) {
		if (left < 3) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n", __LINE__, left, 3);
			return -1;
		}
		left -= 3;

		opclass = *temp_buf;
		temp_buf += 2;
		non_opch_num = *temp_buf++;
		cap = get_bandcap(ctx, opclass, non_opch_num, temp_buf);
		rinfo->band |= cap;

		if (left < non_opch_num) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n", __LINE__, left, non_opch_num);
			return -1;
		}
		left -= non_opch_num;

		temp_buf += non_opch_num;
	}
	debug(DEBUG_TRACE, "radio_id("MACSTR") bandcap=%02x\n", PRINT_MAC(rinfo->identifier),
		rinfo->band);

	return 0;
}

int parse_ap_radio_identifier_tlv(struct p1905_managerd_ctx *ctx,
			OUT unsigned char *radio_identifier
#ifdef MAP_R2
			, unsigned char need_store
#endif
			, unsigned short len, unsigned char *val)
{
	if (len != ETH_ALEN)
		return -1;

	os_memcpy(radio_identifier, val, ETH_ALEN);
#ifdef MAP_R2
	if (need_store)
		add_new_radio_identifier(ctx, val);
#endif
	return 0;
}

int parse_client_assoc_event_tlv(struct p1905_managerd_ctx *ctx, unsigned char *mac,
	unsigned char *bssid, unsigned char *event, unsigned char *al_mac, unsigned short len,
	unsigned char *value, struct dl_list *clients_table)
{
	unsigned char *temp_buf = NULL;
#ifdef MAP_R2
	struct assoc_client *client = NULL;
	struct assoc_client *client_tmp = NULL;
#endif

	if (len != 13)
		return -1;

	temp_buf = value;

	debug(DEBUG_TRACE, "client("MACSTR") ", PRINT_MAC(temp_buf));
	os_memcpy(mac, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;
	os_memcpy(bssid, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;
	*event = *temp_buf;

	debug(DEBUG_TRACE, "sta("MACSTR")\n", PRINT_MAC(mac));
	debug(DEBUG_TRACE, "%s the ", (*event) ? "join" : "left");
	debug(DEBUG_TRACE, "bss("MACSTR")\n", PRINT_MAC(bssid));
#ifdef MAP_R2
	client = get_assoc_cli_by_mac(ctx, mac);

	if (client && (*event) == 0) {
		client_tmp = get_cli_by_mac(clients_table, mac);
		if (!client_tmp) {
			client_tmp = add_new_assoc_cli(clients_table, mac, al_mac);
			if (!client_tmp)
				return -1;
		}
		client_tmp->disassoc = 1;
	} else if (!client && *event) {
		client = add_new_assoc_cli(clients_table, mac, al_mac);
		if (client) {
			os_memcpy(client->bssid, bssid, ETH_ALEN);
		}
		debug(DEBUG_ERROR, "add sta("MACSTR") bssid("MACSTR")\n",
			PRINT_MAC(mac), PRINT_MAC(bssid));
	}
#endif
	return 0;

}

int parse_client_info_tlv(struct p1905_managerd_ctx *ctx, unsigned char *bssid, unsigned char *sta_mac,
			unsigned short len, unsigned char *value)
{
	unsigned char *temp_buf = value;

	if (len != 12) {
		debug(DEBUG_ERROR, "length(%d) error\n", len);
		return -1;
	}

	debug(DEBUG_TRACE, "BSSID ("MACSTR") ", PRINT_MAC(temp_buf));
	memcpy(bssid, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;
	debug(DEBUG_TRACE, "client ("MACSTR")\n", PRINT_MAC(temp_buf));
	memcpy(sta_mac, temp_buf, ETH_ALEN);

	return 0;
}

int append_operational_bss_tlv(unsigned char *pkt,
	struct list_head_oper_bss *opbss_head)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;
	int radio_num = 0;
	unsigned char exist = 0;
	struct op_bss_db *bss_info = NULL;
	struct operational_bss_db *operbss_info = NULL;

	temp_buf = pkt;

  /*firstly shift to tlv value*/
	temp_buf += 3;
	total_length += 3;

	temp_buf += 1;
	total_length += 1;

	if(!SLIST_EMPTY(opbss_head)) {
		SLIST_FOREACH(operbss_info, opbss_head, oper_bss_entry)
		{
			exist = 1;
			radio_num++;
			memcpy(temp_buf, operbss_info->identifier, ETH_ALEN);
			debug(DEBUG_TRACE, "identifier("MACSTR")\n",
						PRINT_MAC(operbss_info->identifier));
			temp_buf += ETH_ALEN;
			total_length += ETH_ALEN;

			*temp_buf = operbss_info->oper_bss_num;
			temp_buf += 1;
			total_length += 1;
			if (!SLIST_EMPTY(&operbss_info->op_bss_head)) {
				SLIST_FOREACH(bss_info, &operbss_info->op_bss_head, op_bss_entry)
				{
					debug(DEBUG_TRACE, "BSSID("MACSTR")\n",
						PRINT_MAC(bss_info->bssid));
					memcpy(temp_buf, bss_info->bssid, ETH_ALEN);
					temp_buf += ETH_ALEN;
					total_length += ETH_ALEN;
					*temp_buf = bss_info->ssid_len;
					temp_buf += 1;
					total_length += 1;
					memcpy(temp_buf, bss_info->ssid, bss_info->ssid_len);
					temp_buf += bss_info->ssid_len;
					total_length += bss_info->ssid_len;
				}
			} else {
				debug(DEBUG_TRACE, "interface of this radio are all off\n");
			}
		 }
	 }

	/*if really has the p1905.1 neighbor device, append type & length */
	if(exist) {
		(*pkt) = AP_OPERATIONAL_BSS_TYPE;
		*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);
		*(pkt + 3) = radio_num;
		debug(DEBUG_TRACE, "%s radio_num=%d\n",__func__,radio_num);
		return total_length;
	} else {
		return 0;
	}

}

int append_associated_clients_tlv(unsigned char *pkt,
	struct list_head_assoc_clients *asscli_head, unsigned int cnt)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf = NULL;
	int bss_num = 0, cli_num = 0;
	unsigned char bss_exist = 0;
	struct clients_db *clis_info = NULL;
	struct associated_clients_db *assc_clis_info = NULL;
	unsigned short time = 0;

	temp_buf = pkt;

  /*firstly shift to tlv value*/
	temp_buf += 3;
	total_length += 3;

	temp_buf += 1;
	total_length += 1;

	if(!SLIST_EMPTY(asscli_head)) {
		SLIST_FOREACH(assc_clis_info, asscli_head, assoc_clients_entry)
		{
			bss_exist = 1;
			bss_num++;
			memcpy(temp_buf, assc_clis_info->bssid, ETH_ALEN);
			temp_buf += ETH_ALEN;
			total_length += ETH_ALEN;
			debug(DEBUG_TRACE, "assco_clients_num=%d\n", assc_clis_info->assco_clients_num);
			*(unsigned short *)(temp_buf) = host_to_be16(assc_clis_info->assco_clients_num);
			temp_buf += 2;
			total_length += 2;

			if (!SLIST_EMPTY(&assc_clis_info->clients_head)) {
				cli_num = 0;
				SLIST_FOREACH(clis_info, &assc_clis_info->clients_head, clients_entry)
				{
					cli_num++;
					memcpy(temp_buf, clis_info->mac, ETH_ALEN);
					temp_buf += ETH_ALEN;
					total_length += ETH_ALEN;

					if ((cnt - clis_info->time) > 65534)
						time = 65535;
					else
						time = (unsigned short)(cnt - clis_info->time);
					debug(DEBUG_TRACE, "time=%d\n", time);
					*(unsigned short *)(temp_buf) = host_to_be16(time);
					temp_buf += 2;
					total_length += 2;
				}
				if (cli_num != assc_clis_info->assco_clients_num) {
					debug(DEBUG_ERROR, "error! assco_clients_num mismatch");
					debug(DEBUG_ERROR, "cli_num=%d, assco_clients_num=%d\n", cli_num,
						assc_clis_info->assco_clients_num);
					return 0;
				}
			} else {
				debug(DEBUG_TRACE, "no station assoc "MACSTR"\n", PRINT_MAC(assc_clis_info->bssid));
				continue;
			}
		 }
	 }

	/*if really has the p1905.1 neighbor device, append type & length */
	if(bss_exist) {
		(*pkt) = AP_ASSOCIATED_CLIENTS_TYPE;
		*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);
		*(pkt + 3) = bss_num;
		return total_length;
	} else {
		return 0;
	}
}



int append_client_assoc_event_tlv(unsigned char *pkt,
		struct map_client_association_event *assoc_evt)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf = NULL;

	*pkt++ = CLIENT_ASSOCIATION_EVENT_TYPE;
	total_length += 1;

	*(unsigned short *)(pkt) = host_to_be16(CLIENT_ASSOCIATION_EVENT_LENGTH);
	pkt += 2;
	total_length += 2;

	temp_buf = pkt;
	memcpy(temp_buf, assoc_evt->sta_mac, ETH_ALEN);
	temp_buf += ETH_ALEN;
	memcpy(temp_buf, assoc_evt->bssid, ETH_ALEN);
	temp_buf += ETH_ALEN;
	*temp_buf = assoc_evt->assoc_evt;

	total_length += CLIENT_ASSOCIATION_EVENT_LENGTH;
	return total_length;
}

int append_cli_info_tlv(unsigned char *pkt,
		struct client_info *info)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf = NULL;

	*pkt = CLIENT_INFO_TYPE;
	total_length += 1;

	*(unsigned short *)(pkt+1) = host_to_be16(CLIENT_INFO_LENGTH);
	total_length += 2;

	temp_buf = pkt + 3;

	memcpy(temp_buf, info->bssid, ETH_ALEN);
	temp_buf += ETH_ALEN;
	memcpy(temp_buf, info->sta_mac, ETH_ALEN);
	temp_buf += ETH_ALEN;

	total_length += CLIENT_INFO_LENGTH;
	return total_length;
}

int append_cli_capability_report_tlv(struct p1905_managerd_ctx* ctx, unsigned char *pkt,
		int *error_code)
{
	unsigned short total_length = 0;
	unsigned short length = 1;
	unsigned char *temp_buf = NULL;
	struct associated_clients_db* pbss_db = NULL;
	struct clients_db* sta_db = NULL;

	*pkt = CLIENT_CAPABILITY_REPORT_TYPE;
	total_length += 1;

	SLIST_FOREACH(pbss_db, &(ctx->ap_cap_entry.assoc_clients_head), assoc_clients_entry)
	{
		SLIST_FOREACH(sta_db, &(pbss_db->clients_head), clients_entry)
		{
			if(!memcmp(sta_db->mac, ctx->sinfo.cinfo.sta_mac, ETH_ALEN))
			{
				break;
			}
		}
		if(sta_db != NULL)
		{
			break;
		}
	}
	temp_buf = pkt + 3;
	if(!sta_db)
	{
		debug(DEBUG_TRACE, "station("MACSTR") not associated",
			PRINT_MAC(ctx->sinfo.cinfo.sta_mac));
		*temp_buf = 0x01;
		*error_code = 0x02; /*STA not associated with any BSS operated by the Multi-AP Agent*/
		total_length += 1;
	}
	else
	{
		*temp_buf = 0x00;
		total_length += 1;
		temp_buf += 1;

		memcpy(temp_buf, sta_db->frame, sta_db->frame_len);
		total_length += sta_db->frame_len;
		length += sta_db->frame_len;
	}

	*(unsigned short *)(pkt+1) = host_to_be16(length);
	total_length += 2;

	return total_length;
}

int update_ap_ht_cap(struct p1905_managerd_ctx* ctx, struct ap_ht_capability *pcap)
{
	struct ap_ht_cap_db *pdb;
	SLIST_FOREACH(pdb, &(ctx->ap_cap_entry.ap_ht_cap_head), ap_ht_cap_entry)
	{
		/*exist entry, update it*/
		if(!memcmp(pdb->identifier, pcap->identifier, ETH_ALEN))
		{
			debug(DEBUG_TRACE, "update ap ht cap for existing radio("MACSTR")\n", PRINT_MAC(pcap->identifier));
			pdb->tx_stream = pcap->tx_stream;
			pdb->rx_stream = pcap->rx_stream;
			pdb->sgi_20 = pcap->sgi_20;
			pdb->sgi_40 = pcap->sgi_40;
			pdb->ht_40 = pcap->ht_40;
			return 0;
		}
	}
	/*insert new one*/
	debug(DEBUG_TRACE, "insert new ap ht cap for radio("MACSTR")\n", PRINT_MAC(pcap->identifier));
	pdb = (struct ap_ht_cap_db*)malloc(sizeof(struct ap_ht_cap_db));
	if(pdb == NULL)
	{
		debug(DEBUG_ERROR, "allocate memory fail\n");
		return -1;
	}
	memcpy(pdb->identifier, pcap->identifier, ETH_ALEN);
	pdb->tx_stream = pcap->tx_stream;
	pdb->rx_stream = pcap->rx_stream;
	pdb->sgi_20 = pcap->sgi_20;
	pdb->sgi_40 = pcap->sgi_40;
	pdb->ht_40 = pcap->ht_40;
	SLIST_INSERT_HEAD(&(ctx->ap_cap_entry.ap_ht_cap_head), pdb, ap_ht_cap_entry);
	return 0;
}

int update_ap_vht_cap(struct p1905_managerd_ctx* ctx, struct ap_vht_capability *pcap)
{
	struct ap_vht_cap_db *pdb;
	SLIST_FOREACH(pdb, &(ctx->ap_cap_entry.ap_vht_cap_head), ap_vht_cap_entry)
	{
		/*exist entry, update it*/
		if(!memcmp(pdb->identifier, pcap->identifier, ETH_ALEN))
		{
			debug(DEBUG_TRACE, "update ap vht cap for existing radio("MACSTR")\n", PRINT_MAC(pcap->identifier));
			pdb->vht_tx_mcs = pcap->vht_tx_mcs;
			pdb->vht_rx_mcs = pcap->vht_rx_mcs;
			pdb->mu_beamformer = pcap->mu_beamformer;
			pdb->sgi_160 = pcap->sgi_160;
			pdb->sgi_80 = pcap->sgi_80;
			pdb->vht_160 = pcap->vht_160;
			pdb->vht_8080 = pcap->vht_8080;
			pdb->tx_stream = pcap->tx_stream;
			pdb->rx_stream = pcap->rx_stream;
			pdb->su_beamformer = pcap->su_beamformer;
			return 0;
		}
	}
	/*insert new one*/
	debug(DEBUG_TRACE, "insert new ap vht cap for radio("MACSTR")\n", PRINT_MAC(pcap->identifier));
	pdb = (struct ap_vht_cap_db*)malloc(sizeof(struct ap_vht_cap_db));
	if(pdb == NULL)
	{
		debug(DEBUG_ERROR, "[%s]allocate memory fail\n", __func__);
		return -1;
	}
	memcpy(pdb->identifier, pcap->identifier, ETH_ALEN);
	pdb->vht_tx_mcs = pcap->vht_tx_mcs;
	pdb->vht_rx_mcs = pcap->vht_rx_mcs;
	pdb->mu_beamformer = pcap->mu_beamformer;
	pdb->sgi_160 = pcap->sgi_160;
	pdb->sgi_80 = pcap->sgi_80;
	pdb->vht_160 = pcap->vht_160;
	pdb->vht_8080 = pcap->vht_8080;
	pdb->tx_stream = pcap->tx_stream;
	pdb->rx_stream = pcap->rx_stream;
	pdb->su_beamformer = pcap->su_beamformer;
	SLIST_INSERT_HEAD(&(ctx->ap_cap_entry.ap_vht_cap_head), pdb, ap_vht_cap_entry);
	return 0;
}

int update_ap_he_cap(struct p1905_managerd_ctx* ctx, struct ap_he_capability *pcap)
{
	struct ap_he_cap_db *pdb;

	if (pcap->he_mcs_len > 12 || pcap->he_mcs_len < 2) {
		debug(DEBUG_ERROR, "invalid he_mcs_len(%d)\n", pcap->he_mcs_len);
		return -1;
	}

	SLIST_FOREACH(pdb, &(ctx->ap_cap_entry.ap_he_cap_head), ap_he_cap_entry)
	{
		/*exist entry, update it*/
		if(!memcmp(pdb->identifier, pcap->identifier, ETH_ALEN))
		{
			debug(DEBUG_TRACE, "update ap he cap for existing radio("MACSTR")\n",
				PRINT_MAC(pcap->identifier));
			pdb->he_mcs_len = pcap->he_mcs_len;
			os_memcpy(pdb->he_mcs, pcap->he_mcs, pdb->he_mcs_len);
			pdb->tx_stream = pcap->tx_stream;
			pdb->rx_stream = pcap->rx_stream;
			pdb->he_8080 = pcap->he_8080;
			pdb->he_160 = pcap->he_160;
			pdb->su_bf_cap = pcap->su_bf_cap;
			pdb->mu_bf_cap = pcap->mu_bf_cap;
			pdb->ul_mu_mimo_cap = pcap->ul_mu_mimo_cap;
			pdb->ul_mu_mimo_ofdma_cap = pcap->ul_mu_mimo_ofdma_cap;
			pdb->dl_mu_mimo_ofdma_cap = pcap->dl_mu_mimo_ofdma_cap;
			pdb->ul_ofdma_cap = pcap->ul_ofdma_cap;
			pdb->dl_ofdma_cap = pcap->dl_ofdma_cap;
#ifdef MAP_R3
			pdb->agent_role = pcap->agent_role;
			pdb->su_beamformee_status = pcap->su_beamformee_status;
			pdb->beamformee_sts_less80 = pcap->beamformee_sts_less80;
			pdb->beamformee_sts_more80= pcap->beamformee_sts_more80;
			pdb->max_user_dl_tx_mu_mimo = pcap->max_user_dl_tx_mu_mimo;
			pdb->max_user_ul_rx_mu_mimo = pcap->max_user_ul_rx_mu_mimo;
			pdb->max_user_dl_tx_ofdma = pcap->max_user_dl_tx_ofdma;
			pdb->max_user_ul_rx_ofdma = pcap->max_user_ul_rx_ofdma;
			pdb->rts_status = pcap->rts_status;
			pdb->mu_rts_status = pcap->mu_rts_status;
			pdb->m_bssid_status = pcap->m_bssid_status;
			pdb->mu_edca_status = pcap->mu_edca_status;
			pdb->twt_requester_status = pcap->twt_requester_status;
			pdb->twt_responder_status = pcap->twt_responder_status;
#endif //MAP_R3
			return 0;
		}
	}
	/*insert new one*/
	debug(DEBUG_TRACE, "insert new ap vht cap for radio("MACSTR")\n",
		PRINT_MAC(pcap->identifier));
	pdb = (struct ap_he_cap_db*)malloc(sizeof(struct ap_he_cap_db));
	if(pdb == NULL)
	{
		debug(DEBUG_ERROR, "allocate memory fail\n");
		return -1;
	}
	memcpy(pdb->identifier, pcap->identifier, ETH_ALEN);
	pdb->he_mcs_len = pcap->he_mcs_len;
	os_memcpy(pdb->he_mcs, pcap->he_mcs, pdb->he_mcs_len);
	pdb->tx_stream = pcap->tx_stream;
	pdb->rx_stream = pcap->rx_stream;
	pdb->he_8080 = pcap->he_8080;
	pdb->he_160 = pcap->he_160;
	pdb->su_bf_cap = pcap->su_bf_cap;
	pdb->mu_bf_cap = pcap->mu_bf_cap;
	pdb->ul_mu_mimo_cap = pcap->ul_mu_mimo_cap;
	pdb->ul_mu_mimo_ofdma_cap = pcap->ul_mu_mimo_ofdma_cap;
	pdb->dl_mu_mimo_ofdma_cap = pcap->dl_mu_mimo_ofdma_cap;
	pdb->ul_ofdma_cap = pcap->ul_ofdma_cap;
	pdb->dl_ofdma_cap = pcap->dl_ofdma_cap;
#ifdef MAP_R3
	pdb->agent_role = pcap->agent_role;
	pdb->su_beamformee_status = pcap->su_beamformee_status;
	pdb->beamformee_sts_less80 = pcap->beamformee_sts_less80;
	pdb->beamformee_sts_more80= pcap->beamformee_sts_more80;
	pdb->max_user_dl_tx_mu_mimo = pcap->max_user_dl_tx_mu_mimo;
	pdb->max_user_ul_rx_mu_mimo = pcap->max_user_ul_rx_mu_mimo;
	pdb->max_user_dl_tx_ofdma = pcap->max_user_dl_tx_ofdma;
	pdb->max_user_ul_rx_ofdma = pcap->max_user_ul_rx_ofdma;
	pdb->rts_status = pcap->rts_status;
	pdb->mu_rts_status = pcap->mu_rts_status;
	pdb->m_bssid_status = pcap->m_bssid_status;
	pdb->mu_edca_status = pcap->mu_edca_status;
	pdb->twt_requester_status = pcap->twt_requester_status;
	pdb->twt_responder_status = pcap->twt_responder_status;
#endif //MAP_R3

	SLIST_INSERT_HEAD(&(ctx->ap_cap_entry.ap_he_cap_head), pdb, ap_he_cap_entry);
	return 0;
}

int delete_exist_ch_prefer_info(struct p1905_managerd_ctx *ctx,
	unsigned char *identifier)
{
	 struct ch_prefer_db *chcap = NULL;
	 struct prefer_info_db *prefer = NULL, *prefer_tmp = NULL;
	 int i = 0;

	if(!SLIST_EMPTY(&(ctx->ap_cap_entry.ch_prefer_head)))
    {
        SLIST_FOREACH(chcap, &(ctx->ap_cap_entry.ch_prefer_head), ch_prefer_entry)
        {
            if(!memcmp(chcap->identifier, identifier, ETH_ALEN))
            {
                if(!SLIST_EMPTY(&(chcap->prefer_info_head)))
                {
                    prefer = SLIST_FIRST(&(chcap->prefer_info_head));
                    while(prefer)
                    {
                    	debug(DEBUG_TRACE, "delete_exist struct prefer_info_db\n");
						debug(DEBUG_TRACE, "opclass=%d, ch_num=%d perference=%d, reason=%d\n",
							prefer->op_class, prefer->ch_num, prefer->perference, prefer->reason);
						debug(DEBUG_TRACE, "ch_list: ");
						for (i = 0; i < prefer->ch_num; i++) {
							debugbyte(DEBUG_TRACE, "%d ", prefer->ch_list[i]);
						}
						debugbyte(DEBUG_TRACE, "\n");
                        prefer_tmp = SLIST_NEXT(prefer, prefer_info_entry);
                        SLIST_REMOVE(&(chcap->prefer_info_head), prefer,
                                    prefer_info_db, prefer_info_entry);
                        free(prefer);
                        prefer = prefer_tmp;
                    }
                }
				debug(DEBUG_TRACE, "delete_exist struct ch_prefer_db\n");
				debug(DEBUG_TRACE, "identifier("MACSTR")\n",
					PRINT_MAC(chcap->identifier));
				debug(DEBUG_TRACE, "oper_bss_num:%d\n", chcap->op_class_num);
                SLIST_REMOVE(&(ctx->ap_cap_entry.ch_prefer_head), chcap,\
                            ch_prefer_db, ch_prefer_entry);
                free(chcap);
                break;
            }
        }
    }

	return wapp_utils_success;
}

int delete_all_ch_prefer_info(struct list_head_ch_prefer *ch_prefer_head)
{
	struct ch_prefer_db *chcap = NULL, *chcap_tmp = NULL;
	struct prefer_info_db *prefer = NULL, *prefer_tmp = NULL;
	int i = 0;

	chcap = SLIST_FIRST(ch_prefer_head);
	while (chcap != NULL) {
		chcap_tmp = SLIST_NEXT(chcap, ch_prefer_entry);
        prefer = SLIST_FIRST(&(chcap->prefer_info_head));
        while(prefer)
        {
        	debug(DEBUG_TRACE, "delete_exist struct prefer_info_db\n");
			debug(DEBUG_TRACE, "opclass=%d, ch_num=%d perference=%d, reason=%d\n",
				prefer->op_class, prefer->ch_num, prefer->perference, prefer->reason);
			debug(DEBUG_TRACE, "ch_list: ");
			for (i = 0; i < prefer->ch_num; i++) {
				debugbyte(DEBUG_TRACE, "%d ", prefer->ch_list[i]);
			}
			debugbyte(DEBUG_TRACE, "\n");
            prefer_tmp = SLIST_NEXT(prefer, prefer_info_entry);
            free(prefer);
            prefer = prefer_tmp;
        }
		debug(DEBUG_TRACE, "delete_exist struct ch_prefer_db\n");
		debug(DEBUG_TRACE, "identifier("MACSTR")\n",
			PRINT_MAC(chcap->identifier));
		debug(DEBUG_TRACE, "tx_power_limit=%d, oper_bss_num:%d\n",
			chcap->tx_power_limit, chcap->op_class_num);
		free(chcap);
		chcap = chcap_tmp;
    }
	SLIST_INIT(ch_prefer_head);

	return wapp_utils_success;
}

int insert_new_channel_prefer_info(struct p1905_managerd_ctx *ctx,
	struct ch_prefer *prefer)
{
	struct ch_prefer_db *chcap = NULL;
	struct prefer_info_db *prefer_db = NULL;
	struct prefer_info *info = NULL;
	int i = 0, j = 0;

	chcap = (struct ch_prefer_db *)malloc(sizeof(struct ch_prefer_db));
	if (!chcap) {
		debug(DEBUG_ERROR, "alloc struct ch_prefer_db fail\n");
		return wapp_utils_error;
	}
	memset(chcap, 0, sizeof(struct ch_prefer_db));
	memcpy(chcap->identifier, prefer->identifier, ETH_ALEN);
	chcap->op_class_num = prefer->op_class_num;
	SLIST_INIT(&(chcap->prefer_info_head));
	SLIST_INSERT_HEAD(&(ctx->ap_cap_entry.ch_prefer_head), chcap, ch_prefer_entry);

	debug(DEBUG_TRACE, "insert struct ch_prefer_db\n");
	debug(DEBUG_TRACE, "identifier("MACSTR")\n",
		PRINT_MAC(chcap->identifier));
	debug(DEBUG_TRACE, "op_class_num:%d\n", chcap->op_class_num);

	for(i = 0; i < chcap->op_class_num; i++) {
		info = &prefer->opinfo[i];
		prefer_db = (struct prefer_info_db *)malloc(sizeof(struct prefer_info_db));
		if (!prefer_db) {
			debug(DEBUG_ERROR, "alloc struct prefer_info_db fail\n");
			return wapp_utils_error;
		}
		prefer_db->op_class = info->op_class;
		prefer_db->ch_num= info->ch_num;
		memcpy(prefer_db->ch_list, info->ch_list, prefer_db->ch_num);
		prefer_db->perference= info->perference;
		prefer_db->reason= info->reason;
		SLIST_INSERT_HEAD(&(chcap->prefer_info_head), prefer_db, prefer_info_entry);

		debug(DEBUG_TRACE, "insert struct prefer_info_db\n");
		debug(DEBUG_TRACE, "opclass=%d, ch_num=%d perference=%d, reason=%d\n",
			prefer_db->op_class, prefer_db->ch_num, prefer_db->perference, prefer_db->reason);
		debug(DEBUG_TRACE, "ch_list: ");
		for (j = 0; j < prefer_db->ch_num; j++) {
			debug(DEBUG_TRACE, "%d ", prefer_db->ch_list[j]);
		}
		debug(DEBUG_TRACE, "\n");
	}

	return wapp_utils_success;

}

int delete_exist_ch_prefer(struct list_head_ch_prefer *ch_prefer_head, unsigned char *identifier)
{
	struct ch_prefer_db *chcap = NULL;
	struct ch_prefer_db *chcap_tmp = NULL;
	struct prefer_info_db *prefer = NULL;
	struct prefer_info_db *prefer_tmp = NULL;

	if (!ch_prefer_head)
		return wapp_utils_error;

	if (SLIST_EMPTY(ch_prefer_head))
		return wapp_utils_success;

	chcap = SLIST_FIRST(ch_prefer_head);
	while (chcap) {
		if (!os_memcmp(chcap->identifier, identifier, ETH_ALEN)) {
			if (!SLIST_EMPTY(&chcap->prefer_info_head)) {
				prefer = SLIST_FIRST(&chcap->prefer_info_head);
				while (prefer) {
					prefer_tmp = SLIST_NEXT(prefer, prefer_info_entry);
					SLIST_REMOVE(&(chcap->prefer_info_head), prefer, prefer_info_db, prefer_info_entry);
					free(prefer);
					prefer = prefer_tmp;
				}
			}

			SLIST_REMOVE(ch_prefer_head, chcap, ch_prefer_db, ch_prefer_entry);
			free(chcap);
			break;
		}
		chcap = chcap_tmp;
	}

	return wapp_utils_success;
}

void update_ch_prefer_list_info(struct list_head_ch_prefer *dest_ch_prefer_head, struct list_head_ch_prefer *src_ch_prefer_head)
{
	struct ch_prefer_db *chcap = NULL;
	struct ch_prefer_db *chcap_tmp = NULL;
	struct ch_prefer_db *dst_chcap = NULL;
	int exists = 0;

	if (!dest_ch_prefer_head || !src_ch_prefer_head)
		return;

	if (SLIST_EMPTY(src_ch_prefer_head))
		return;

	/* first check chcap whether exist in dest_ch_prefer_head */
	/*if true, should remove the same one in dest_ch_prefer_head, then add new*/

	chcap = SLIST_FIRST(src_ch_prefer_head);
	while (chcap != NULL) {
		chcap_tmp = SLIST_NEXT(chcap, ch_prefer_entry);
		SLIST_REMOVE(src_ch_prefer_head, chcap, ch_prefer_db, ch_prefer_entry);
		exists = 0;
		if (!SLIST_EMPTY(dest_ch_prefer_head)) {
			SLIST_FOREACH(dst_chcap, dest_ch_prefer_head, ch_prefer_entry) {
				if (!os_memcmp(dst_chcap->identifier, chcap->identifier, ETH_ALEN)) {
					exists = 1;
					break;
				}
			}

			if (exists && dst_chcap)
				delete_exist_ch_prefer(dest_ch_prefer_head, dst_chcap->identifier);
		}

		SLIST_INSERT_HEAD(dest_ch_prefer_head, chcap, ch_prefer_entry);
		chcap = chcap_tmp;
	}
}


void update_ch_prefer_info(struct p1905_managerd_ctx *ctx, struct list_head_ch_prefer *ch_prefer_head)
{
	struct ch_prefer_db *prefer = NULL;
	struct ch_prefer_db *prefer_tmp = NULL;

	if (!ctx || !ch_prefer_head)
		return;
	if (SLIST_EMPTY(ch_prefer_head))
		return;

	prefer = SLIST_FIRST(ch_prefer_head);
	while (prefer) {
		prefer_tmp = SLIST_NEXT(prefer, ch_prefer_entry);
		SLIST_REMOVE(ch_prefer_head, prefer, ch_prefer_db, ch_prefer_entry);
		delete_exist_ch_prefer_info(ctx, prefer->identifier);
		SLIST_INSERT_HEAD(&(ctx->ap_cap_entry.ch_prefer_head), prefer, ch_prefer_entry);
		prefer = prefer_tmp;
	}
}

int insert_new_ch_prefer_info(
	struct list_head_ch_prefer *ch_prefer_head, unsigned short len, unsigned char *val)
{
	struct ch_prefer_db *chcap = NULL;
	struct prefer_info_db *prefer = NULL;
	unsigned char *pos = val;
	unsigned short prefer_len = 0;
	unsigned char i = 0, j = 0, op_class = 0, ch_num = 0;
	int left = (int)len;

	SLIST_FOREACH(chcap, ch_prefer_head, ch_prefer_entry)
	{
		if (!memcmp(chcap->identifier, pos, ETH_ALEN)) {
			debug(DEBUG_TRACE, "exist chcap identifier("MACSTR")\n",
				PRINT_MAC(chcap->identifier));
			break;
		}
	}

	if (left - ETH_ALEN - 1 < 0) {
		debug(DEBUG_ERROR, "pre length check for chcap fail\n");
		goto err;
	}

	if (!chcap) {
		chcap = (struct ch_prefer_db *)malloc(sizeof(*chcap));
		if (!chcap) {
			debug(DEBUG_ERROR, "alloc struct ch_prefer_db fail\n");
			goto err;
		}
		memset(chcap, 0, sizeof(*chcap));
		memcpy(chcap->identifier, pos, ETH_ALEN);
		pos += ETH_ALEN;
		chcap->op_class_num = *pos;
		pos += 1;
		SLIST_INIT(&(chcap->prefer_info_head));
		SLIST_INSERT_HEAD(ch_prefer_head, chcap, ch_prefer_entry);
		debug(DEBUG_TRACE, "insert struct ch_prefer_db\n");
		debug(DEBUG_TRACE, "identifier("MACSTR") op_class_num=%d\n",
			PRINT_MAC(chcap->identifier), chcap->op_class_num);
	}else {
		pos += ETH_ALEN;
		chcap->op_class_num = *pos;
		pos += 1;
	}

	left -= (ETH_ALEN + 1);

	for(i = 0; i < chcap->op_class_num; i++) {
		if (left - 2 < 0) {
			debug(DEBUG_ERROR, "pre length check for op_class fail\n");
			goto err;
		}
		op_class = *pos;
		pos += 1;
		ch_num = *pos;
		pos += 1;
		left -= 2;

		if (left - ch_num < 0) {
			debug(DEBUG_ERROR, "pre length check for ch_num fail\n");
			goto err;
		}
		prefer = (struct prefer_info_db *)malloc(sizeof(struct prefer_info_db));
		if (!prefer) {
			debug(DEBUG_ERROR, "alloc struct prefer_info_db fail\n");
			goto err;
		}
		memset(prefer, 0 , prefer_len);
		prefer->op_class = op_class;
		prefer->ch_num = ch_num;
		if (prefer->ch_num) {
			memcpy(prefer->ch_list, pos, ch_num);
			pos += ch_num;
		}
		left -= ch_num;

		if (left - 1 < 0) {
			debug(DEBUG_ERROR, "pre length check for perference/reason fail\n");
			free(prefer);
			goto err;
		}
		/*bit 4~7*/
		prefer->perference = *pos >> 4;
		/*bit 0~3*/
		prefer->reason = *pos & 0x0f;
		pos += 1;
		SLIST_INSERT_HEAD(&(chcap->prefer_info_head), prefer, prefer_info_entry);
		debug(DEBUG_TRACE, "insert struct prefer_info_db\n");
		debug(DEBUG_TRACE, "opclass=%d, ch_num=%d perference=%d, reason=%d\n",
			prefer->op_class, prefer->ch_num, prefer->perference, prefer->reason);
		debug(DEBUG_TRACE, "ch_list: ");
		for (j = 0; j < prefer->ch_num; j++) {
			debugbyte(DEBUG_TRACE, "%d ", prefer->ch_list[j]);
		}
		debugbyte(DEBUG_TRACE, "\n");
		left -= 1;
	}

	return 0;
err:
	delete_all_ch_prefer_info(ch_prefer_head);
	return -1;
}

int delete_exist_restriction_info(struct p1905_managerd_ctx *ctx,
	unsigned char *identifier)
{
	 struct oper_restrict_db *restriction = NULL;
	 struct restrict_db *resdb = NULL, *resdb_tmp = NULL;
	 int i = 0;

	if(!SLIST_EMPTY(&(ctx->ap_cap_entry.oper_restrict_head)))
    {
        SLIST_FOREACH(restriction, &(ctx->ap_cap_entry.oper_restrict_head), oper_restrict_entry)
        {
            if(!memcmp(restriction->identifier, identifier, ETH_ALEN))
            {
                if(!SLIST_EMPTY(&(restriction->restrict_head)))
                {
                    resdb = SLIST_FIRST(&(restriction->restrict_head));
                    while(resdb)
                    {
                    	debug(DEBUG_TRACE, "delete_exist struct oper_restrict_db\n");
						debug(DEBUG_TRACE, "opclass=%d, ch_num=%d\n",
							resdb->op_class, resdb->ch_num);
						debug(DEBUG_TRACE, "ch_list: ");
						for (i = 0; i < resdb->ch_num; i++) {
							debug(DEBUG_TRACE, "%d ", resdb->ch_list[i]);
						}
						debug(DEBUG_TRACE, "\n");
						debug(DEBUG_TRACE, "min_fre_sep_list: ");
						for (i = 0; i < resdb->ch_num; i++) {
							debug(DEBUG_TRACE, "%d ", resdb->min_fre_sep[i]);
						}
						debug(DEBUG_TRACE, "\n");
                        resdb_tmp = SLIST_NEXT(resdb, restrict_entry);
                        SLIST_REMOVE(&(restriction->restrict_head), resdb,
                                    restrict_db, restrict_entry);
                        free(resdb);
                        resdb = resdb_tmp;
                    }
                }
				debug(DEBUG_TRACE, "delete_exist struct oper_restrict_db\n");
				debug(DEBUG_TRACE, "identifier("MACSTR")\n",
					PRINT_MAC(restriction->identifier));
				debug(DEBUG_TRACE, "oper_bss_num:%d\n", restriction->op_class_num);
                SLIST_REMOVE(&(ctx->ap_cap_entry.oper_restrict_head), restriction,
                            oper_restrict_db, oper_restrict_entry);
                free(restriction);
                break;
            }
        }
    }

	return wapp_utils_success;
}

int insert_new_restriction_info(struct p1905_managerd_ctx *ctx,
	struct restriction *op_restrict)
{
	struct oper_restrict_db *restriction_db = NULL;
	struct restrict_db *resdb = NULL;
	struct restrict_info *info = NULL;
	int i = 0, j = 0;

	restriction_db = (struct oper_restrict_db *)malloc(sizeof(struct oper_restrict_db));
	if (!restriction_db) {
		debug(DEBUG_ERROR, "alloc struct oper_restrict_db fail\n");
		return wapp_utils_error;
	}
	memset(restriction_db, 0, sizeof(struct oper_restrict_db));
	memcpy(restriction_db->identifier, op_restrict->identifier, ETH_ALEN);
	restriction_db->op_class_num = op_restrict->op_class_num;
	SLIST_INIT(&(restriction_db->restrict_head));
	SLIST_INSERT_HEAD(&(ctx->ap_cap_entry.oper_restrict_head),
		restriction_db, oper_restrict_entry);

	debug(DEBUG_TRACE, "insert struct oper_restrict_db\n");
	debug(DEBUG_TRACE, "identifier("MACSTR")\n",
		PRINT_MAC(restriction_db->identifier));
	debug(DEBUG_TRACE, "op_class_num:%d\n", restriction_db->op_class_num);

	for(i = 0; i < restriction_db->op_class_num; i++) {
		info = &op_restrict->opinfo[i];
		resdb = (struct restrict_db *)malloc(sizeof(struct restrict_db));
		if (!resdb) {
			debug(DEBUG_ERROR, "alloc struct restrict_db fail\n");
			return wapp_utils_error;
		}
		resdb->op_class = info->op_class;
		resdb->ch_num= info->ch_num;
		memcpy(resdb->ch_list, info->ch_list, resdb->ch_num);
		memcpy(resdb->min_fre_sep, info->fre_separation, resdb->ch_num);
		SLIST_INSERT_HEAD(&(restriction_db->restrict_head), resdb, restrict_entry);

		debug(DEBUG_TRACE, "insert struct restrict_db\n");
		debug(DEBUG_TRACE, "opclass=%d, ch_num=%d\n",
			resdb->op_class, resdb->ch_num);
		debug(DEBUG_TRACE, "ch_list: ");
		for (j = 0; j < resdb->ch_num; j++) {
			debug(DEBUG_TRACE, "%d ", resdb->ch_list[j]);
		}
		debug(DEBUG_TRACE, "\n");
		debug(DEBUG_TRACE, "min_fre_sep_list: ");
		for (j = 0; j < resdb->ch_num; j++) {
			debug(DEBUG_TRACE, "%d ", resdb->min_fre_sep[j]);
		}
		debug(DEBUG_TRACE, "\n");
	}

	return wapp_utils_success;

}

int parse_channel_preference_tlv(struct p1905_managerd_ctx *ctx,
					struct list_head_ch_prefer *ch_prefer_head,
					unsigned short len, unsigned char *value)
{
	if (ctx->MAP_Cer) {
		/*insert new channel preference info*/
		if (insert_new_ch_prefer_info(ch_prefer_head, len, value) < 0) {
			debug(DEBUG_ERROR, "insert_new_ch_prefer_info fail\n");
			return -1;
		}
	}

	return 0;
}

int update_tx_power_limit_info(struct p1905_managerd_ctx *ctx,
	unsigned char *identifier, signed char power)
{
	struct ch_prefer_db *chcap = NULL;

	if(!SLIST_EMPTY(&(ctx->ap_cap_entry.ch_prefer_head))) {
        SLIST_FOREACH(chcap, &(ctx->ap_cap_entry.ch_prefer_head), ch_prefer_entry)
        {
            if(!memcmp(chcap->identifier, identifier, ETH_ALEN)) {
				debug(DEBUG_TRACE, "find struct ch_prefer_db\n");
				debug(DEBUG_TRACE, "identifier("MACSTR")\n",
					PRINT_MAC(chcap->identifier));
				chcap->tx_power_limit = power;
				debug(DEBUG_TRACE, "tx_power_limit:%d\n", chcap->tx_power_limit);
                break;
            }
        }
    }

	if (chcap == NULL) {
		debug(DEBUG_ERROR, "identifier("MACSTR") not exist, create new one\n",
			PRINT_MAC(identifier));
		chcap = (struct ch_prefer_db *)malloc(sizeof(*chcap));
		if(chcap == NULL) {
			debug(DEBUG_ERROR, "alloc memory fail\n");
			return wapp_utils_error;
		}
		memset(chcap, 0, sizeof(*chcap));
		memcpy(chcap->identifier, identifier, ETH_ALEN);
		chcap->tx_power_limit = power;
		debug(DEBUG_TRACE, "tx_power_limit:%d\n", chcap->tx_power_limit);
		SLIST_INIT(&(chcap->prefer_info_head));
		SLIST_INSERT_HEAD(&(ctx->ap_cap_entry.ch_prefer_head), chcap, ch_prefer_entry);
	}

	return wapp_utils_success;
}

int parse_transmit_power_limit_tlv(struct p1905_managerd_ctx *ctx, unsigned short len, unsigned char *value)
{
	if (len != 7) {
		debug(DEBUG_ERROR, "length error not eaqual 7\n");
		return -1;
	}

	return 0;
}

int parse_channel_selection_request_message(struct p1905_managerd_ctx *ctx,
							unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	struct list_head_ch_prefer ch_prefer_head = {0};
	struct ch_prefer_db *prefer = NULL;
	struct ch_prefer_db *prefer_tmp = NULL;

	type = buf;
	reset_stored_tlv(ctx);
	SLIST_INIT(&ch_prefer_head);

	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = 0x%02x, left_tlv_length %d less than tlv_len %d\n",
				__LINE__, *type, left_tlv_len, tlv_len);
			goto fail;
		}

		if (*type == CH_PREFERENCE_TYPE) {
			ret = parse_channel_preference_tlv(ctx, &ch_prefer_head, len, value);
			if (ret < 0) {
				delete_all_ch_prefer_info(&ctx->ap_cap_entry.ch_prefer_head);
				debug(DEBUG_ERROR, "error channel preference tlv\n");
				goto fail;
			}
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == TRANSMIT_POWER_LIMIT_TYPE) {
			ret = parse_transmit_power_limit_tlv(ctx, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error transmit power limit tlv\n");
				goto fail;
			}
			store_revd_tlvs(ctx, type, tlv_len);
#ifdef MAP_R4_SPT
		} else if (*type == SPATIAL_REUSE_REQ_TYPE) {
			store_revd_tlvs(ctx, type, tlv_len);
#endif
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	update_ch_prefer_info(ctx, &ch_prefer_head);
	return 0;

fail:
	if (!SLIST_EMPTY(&ch_prefer_head)) {
		prefer = SLIST_FIRST(&ch_prefer_head);
		while (prefer) {
			prefer_tmp = SLIST_NEXT(prefer, ch_prefer_entry);
			SLIST_REMOVE(&ch_prefer_head, prefer, ch_prefer_db, ch_prefer_entry);
			free(prefer);
			prefer = prefer_tmp;
		}
	}
	return -1;
}

unsigned char check_invalid_channel(unsigned char op_class,
	unsigned char ch_num, unsigned char *ch_list)
{
	struct global_oper_class *opcls = oper_class;
	int i = 0, j = 0, k = 0;
	unsigned char opcls_found = 0;

	/*check if the channel is in oper_class*/
	do {
		if (opcls[i].opclass == op_class) {
			opcls_found = 1;
			for (k = 0; k < ch_num; k++) {
				for (j = 0; j < opcls[i].channel_num; j++) {
					if (opcls[i].channel_set[j] == ch_list[k])
						break;
				}
				if (j == opcls[i].channel_num) {
					return 0; /*invalid channel*/
				}
			}
			break;
		}
		i++;
	} while (opcls[i].opclass != 0);

	if (opcls_found)
		return 1;
	else
		return 0;
}


unsigned char find_oper_channel(unsigned char op_class,
	unsigned char ch_num, unsigned char *ch_list)
{
	struct global_oper_class *opcls = oper_class;
	int i = 1; /* skip Invlid entry */
	int j = 0, k = 0;
	unsigned char channel = 0;

	do {
		if (opcls[i].opclass == op_class) {
			for (j = 0; j < opcls[i].channel_num; j++) {
				channel = opcls[i].channel_set[j];
				for (k = 0; k < ch_num; k++) {
					if (channel == ch_list[k])
						break;
				}
				if (k == ch_num) {
					return channel;
				}
			}
			break;
		}
		i++;
	} while (opcls[i].opclass != 0);

	return 0;
}

unsigned char find_first_oper_channel(unsigned char op_class)
{
	struct global_oper_class *opcls = oper_class;
	int i = 1; /* skip Invlid entry */
	unsigned char channel = 0;

	do {
		if (opcls[i].opclass == op_class) {
			channel = opcls[i].channel_set[0];
			return channel;
		}
		i++;
	} while (opcls[i].opclass != 0);

	return 0;
}

int update_channel_report_info(struct p1905_managerd_ctx *ctx,
	struct channel_report *rep, unsigned char len)
{
#ifndef MAP_R4_SPT
	ctx->ch_rep = (struct channel_report *)malloc(len);
#else
//	hex_dump_all("DM: spt report", (unsigned char *)rep,len);
	unsigned char ch_rpt_len = sizeof(struct channel_report) + (rep->ch_rep_num* sizeof(struct ch_rep_info));
	ctx->ch_rep = (struct channel_report *)malloc(ch_rpt_len);
	if(ctx->ch_rep && len > ch_rpt_len)
	{
		ctx->spt_report = (struct spt_reuse_report *)malloc(len - ch_rpt_len);
		if (!ctx->spt_report) {
			free(ctx->ch_rep);
			ctx->ch_rep = NULL;
			debug(DEBUG_TRACE, "alloc struct channel_report mem fail\n");
			return -1;
		}
		memcpy(ctx->spt_report, rep + ch_rpt_len, len - ch_rpt_len);
		len = ch_rpt_len;
	}
#endif
	if (!ctx->ch_rep) {
		debug(DEBUG_TRACE, "alloc struct channel_report mem fail\n");
		return -1;
	}
	memcpy(ctx->ch_rep, rep, len);
#ifdef MAP_R4_SPT
	if (ctx->spt_report)
		debug(DEBUG_TRACE, "ctx->spt_report %x\n", ctx->spt_report->spt_rep_num);
#endif
	return 0;
}

int parse_steering_request_tlv(struct p1905_managerd_ctx *ctx,
	unsigned short len, unsigned char *val, struct err_sta_db *err_sta)
{
#ifdef MAP_R2
	unsigned char *temp_buf = val;

	struct assoc_client *client = NULL;
	struct err_sta_mac_db *err_mac = NULL;
	unsigned char sta_count = 0;
	unsigned char assoc_bssid[ETH_ALEN];
	unsigned char req_mode = 0;
	int i = 0;
	int left = len;

	if (left - ETH_ALEN - 6 < 0) {
		debug(DEBUG_ERROR, "pre length check for steering request fail\n");
		return -1;
	}
	memcpy(assoc_bssid, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	req_mode = (*temp_buf & 0x80) ? 1 : 0;
	if (req_mode == 0)
		return 0;
	temp_buf += 1;

	/*skip Steering Opportunity window*/
	temp_buf += 2;

	/*skip BTM Disassociation Timer*/
	temp_buf += 2;

	sta_count = *temp_buf++;
	left -= (ETH_ALEN + 6);


	if (left - sta_count * ETH_ALEN < 0) {
		debug(DEBUG_ERROR, "pre length check for steering request fail\n");
		return -1;
	}
	for (i = 0; i < sta_count; i++) {
		client = get_assoc_cli_by_mac(ctx, temp_buf);
		if (!client || client->disassoc || os_memcmp(client->bssid, assoc_bssid, ETH_ALEN)) {
			err_mac = (struct err_sta_mac_db *)os_malloc(sizeof(struct err_sta_mac_db));
			if (!err_mac) {
				debug(DEBUG_ERROR, "alloc err_sta_mac_db fail\n");
				continue;
			}
			os_memset(err_mac, 0, sizeof(struct err_sta_mac_db));
			os_memcpy(err_mac->mac_addr, temp_buf, ETH_ALEN);
			debug(DEBUG_OFF,
				"mac("MACSTR") error in STEERING_REQUEST_TYPE tlv, need send error tlv in 1905 ack\n",
				PRINT_MAC(temp_buf));
			err_sta->err_sta_cnt++;
			SLIST_INSERT_HEAD(&err_sta->err_sta_head, err_mac, err_sta_mac_entry);
		}
		temp_buf += ETH_ALEN;
	}
#endif

	return 0;
}

int delete_exist_steering_policy(struct steer_policy *spolicy)
{
	 struct sta_db *sta = NULL, *sta_tmp = NULL;
	 struct radio_policy_db *policy = NULL, *policy_tmp = NULL;

	 sta = SLIST_FIRST(&spolicy->local_disallow_head);
	 while(sta != NULL) {
	 	debug(DEBUG_TRACE, "local_disallow_head sta_mac("MACSTR")\n",
				PRINT_MAC(sta->mac));
	 	sta_tmp = SLIST_NEXT(sta, sta_entry);
		free(sta);
		sta = sta_tmp;
	}
	SLIST_INIT(&spolicy->local_disallow_head);
	spolicy->local_disallow_count = 0;

	sta = SLIST_FIRST(&spolicy->btm_disallow_head);
	while(sta != NULL) {
	 	debug(DEBUG_TRACE, "btm_disallow_head sta_mac("MACSTR")\n",
				PRINT_MAC(sta->mac));
	 	sta_tmp = SLIST_NEXT(sta, sta_entry);
		free(sta);
		sta = sta_tmp;
	}
	SLIST_INIT(&spolicy->btm_disallow_head);
	spolicy->btm_disallow_count = 0;

	policy = SLIST_FIRST(&spolicy->radio_policy_head);
	while(policy != NULL) {
		debug(DEBUG_TRACE, "radio_policy_head identifier("MACSTR")\n",
				PRINT_MAC(policy->identifier));
		debug(DEBUG_TRACE, "steer_policy=%d, ch_util_thres=%d, rssi_thres=%d\n",
			policy->steer_policy, policy->ch_util_thres, policy->rssi_thres);
		policy_tmp = SLIST_NEXT(policy, radio_policy_entry);
		free(policy);
		policy = policy_tmp;
	}
	SLIST_INIT(&spolicy->radio_policy_head);
	spolicy->radios = 0;

	return wapp_utils_success;
}


int parse_steering_policy_tlv(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == STEERING_POLICY_TYPE) {
		temp_buf++;
	}
	else {
		return -1;
	}

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	return (length+3);
}

void add_control_policy_info(struct control_policy *steer_cntrl, struct control_policy_db *new_policy)
{
	struct control_policy_db *policy = NULL;
	struct control_policy_db *policy_tmp = NULL;
	struct list_head_control_policy *policy_head = NULL;
	struct sta_db *sta = NULL;
	struct sta_db *sta_tmp = NULL;

	if (!steer_cntrl || !new_policy)
		return;
	policy_head = &(steer_cntrl->policy_head);

	if (SLIST_EMPTY(policy_head))
		return;

	policy = SLIST_FIRST(policy_head);
	while (policy) {
		policy_tmp = SLIST_NEXT(policy, policy_entry);
		if (!os_memcmp(policy->bssid, new_policy->bssid, ETH_ALEN)) {
			SLIST_REMOVE(policy_head, policy, control_policy_db, policy_entry);

			if (!SLIST_EMPTY(&policy->sta_head)) {
				sta = SLIST_FIRST(&policy->sta_head);
				while (policy) {
					sta_tmp = SLIST_NEXT(sta, sta_entry);
					free(sta);
					sta = sta_tmp;
				}
			}
			free(policy);
			steer_cntrl->policy_cnt--;
			break;
		}
		policy = policy_tmp;
	}
	SLIST_INSERT_HEAD(&(steer_cntrl->policy_head), new_policy, policy_entry);
	steer_cntrl->policy_cnt++;
}

void update_control_policy_info(struct control_policy *dst_steer_cntrl, struct control_policy *src_steer_cntrl)
{
	struct control_policy_db *policy = NULL;
	struct control_policy_db *policy_tmp = NULL;
	struct list_head_control_policy *src_policy_head = NULL;

	if (!dst_steer_cntrl || !src_steer_cntrl)
		return;
	src_policy_head = &(src_steer_cntrl->policy_head);

	if (SLIST_EMPTY(src_policy_head))
		return;

	policy = SLIST_FIRST(src_policy_head);
	while (policy) {
		policy_tmp = SLIST_NEXT(policy, policy_entry);
		add_control_policy_info(dst_steer_cntrl, policy);
		SLIST_REMOVE(src_policy_head, policy, control_policy_db, policy_entry);
		policy = policy_tmp;
	}
}


int parse_cli_assoc_control_request_tlv(struct p1905_managerd_ctx *ctx,
	unsigned short len, unsigned char *val, struct control_policy *steer_cntrl)
{
	unsigned char *temp_buf = val;
	struct control_policy_db *policy = NULL;
	struct sta_db *sta = NULL;
	unsigned int i = 0;
	int left = len;

	if (!steer_cntrl)
		goto err;
	if (left - ETH_ALEN - 4 < 0) {
		debug(DEBUG_ERROR, "pre length check for cli assoc control request fail\n");
		goto err;
	}
	policy = (struct control_policy_db *)malloc(sizeof(*policy));
	if (!policy) {
		debug(DEBUG_ERROR, "policy fail to alloc memory!\n");
		goto err;
	}
	memcpy(policy->bssid, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;
	policy->assoc_control = *temp_buf++;
	policy->valid_period = *(unsigned short *)temp_buf;
	policy->valid_period = be_to_host16(policy->valid_period);
	temp_buf += 2;
	policy->sta_list_count = *temp_buf++;
	SLIST_INSERT_HEAD(&steer_cntrl->policy_head, policy, policy_entry);
	steer_cntrl->policy_cnt++;
	left -= (ETH_ALEN + 4);

	SLIST_INIT(&policy->sta_head);
	if (left - policy->sta_list_count * ETH_ALEN < 0) {
		debug(DEBUG_ERROR, "pre length check for sta fail\n");
		goto err;
	}
	for(i = 0; i < policy->sta_list_count; i++) {
		sta = (struct sta_db *)malloc(sizeof(*sta));
		if (!sta) {
			debug(DEBUG_ERROR, "sta fail to alloc memory!\n");
			goto err;
		}
		memcpy(sta->mac, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;
		SLIST_INSERT_HEAD(&policy->sta_head, sta, sta_entry);
	}

	return 0;
err:
	return -1;

}

int parse_metric_reporting_policy_tlv(struct p1905_managerd_ctx *ctx,
	unsigned short len, unsigned char *val, struct metrics_policy *mpolicy)
{
	unsigned char *temp_buf = val;
	unsigned short i = 0;
	struct metric_policy_db *metric_policy = NULL;
	int left = len;

	if (left - 2 < 0) {
		debug(DEBUG_ERROR, "pre length check for metric report policy fail\n");
		return -1;
	}
	if (!mpolicy) {
		debug(DEBUG_ERROR, "input mpolicy is null\n");
		return -1;
	}
	mpolicy->report_interval = *temp_buf++;

	/* workaround for checking ap metrics response
	** ucc set report_interval as 5 sec
	** 1905 send ap metrics response after 5sec, sniffer fail to capture
	*/
	if (ctx->MAP_Cer &&
		(mpolicy->report_interval >= 5)) {
		/* ucc check ap metrics response in 5 sec */
		mpolicy->report_interval -= 1;
	}
	debug(DEBUG_TRACE, "report_interval(%d)\n", mpolicy->report_interval);

	mpolicy->radio_num = *temp_buf++;
	left -= 2;

	if (left - mpolicy->radio_num * 10 < 0) {
		debug(DEBUG_ERROR, "pre length check for radio fail\n");
		return -1;
	}
	for (i = 0; i < mpolicy->radio_num; i++) {
		metric_policy = (struct metric_policy_db *)malloc(sizeof(struct metric_policy_db));
		if (!metric_policy) {
			debug(DEBUG_ERROR, "alloc struct metric_policy_db fail\n");
			return -1;
		}
		memcpy(metric_policy->identifier, temp_buf, ETH_ALEN);

		debug(DEBUG_TRACE, "identifier("MACSTR")\n",
			PRINT_MAC(metric_policy->identifier));
		temp_buf += ETH_ALEN;
		metric_policy->rssi_thres = *temp_buf++;
		metric_policy->hysteresis_margin = *temp_buf++;
		metric_policy->ch_util_thres = *temp_buf++;
		metric_policy->sta_stats_inclusion = *temp_buf & 0x80;
		metric_policy->sta_metrics_inclusion = *temp_buf & 0x40;
#ifdef MAP_R3
		metric_policy->assoc_wf6_inclusion = *temp_buf & 0x20;
#endif
		debug(DEBUG_TRACE, "rssi_thres(%d) hysteresis_margin(%d) ch_util_thres(%d) "
			"sta_stats_inclusion(%d) sta_metrics_inclusion(%d)\n"
#ifdef MAP_R3
			"sta_wifi6_sta_status_inclusion(%d) "
#endif
			"\n",
			metric_policy->rssi_thres,
			metric_policy->hysteresis_margin,
			metric_policy->ch_util_thres,
			metric_policy->sta_stats_inclusion,
			metric_policy->sta_metrics_inclusion
#ifdef MAP_R3
			, metric_policy->assoc_wf6_inclusion
#endif
			);
		temp_buf += 1;
		SLIST_INSERT_HEAD(&mpolicy->policy_head, metric_policy, policy_entry);
	}

	return 0;
}

int parse_client_steering_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len, struct err_sta_db *err_sta)
{
	unsigned char *type = NULL;
	unsigned char *val = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;
	unsigned int right_integrity = 0x1;
	struct err_sta_db err_sta_list = {0};
	struct err_sta_mac_db *err_mac = NULL;
	struct err_sta_mac_db *err_mac_tmp = NULL;

	type = buf;
	reset_stored_tlv(ctx);
	SLIST_INIT(&err_sta_list.err_sta_head);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			goto fail;
		}
		val = type + 3;

		if ((*type == STEERING_REQUEST_TYPE)
#ifdef MAP_R2
		 || (*type == R2_STEERING_REQUEST_TYPE)
#endif
		 ) {
			integrity |= (1 << 0);

			ret = parse_steering_request_tlv(ctx, len, val, &err_sta_list);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error steering request tlv\n");
				goto fail;
			}
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != right_integrity) {
		debug(DEBUG_ERROR, "incomplete steering request 0x%x 0x%x\n",
			integrity, right_integrity);
		goto fail;
	}

	if (integrity&BIT(0) && !SLIST_EMPTY(&err_sta_list.err_sta_head)) {
		err_sta->err_sta_cnt = err_sta_list.err_sta_cnt;
		err_mac = SLIST_FIRST(&err_sta_list.err_sta_head);
		while (err_mac) {
			err_mac_tmp = SLIST_NEXT(err_mac, err_sta_mac_entry);
			SLIST_REMOVE(&err_sta_list.err_sta_head, err_mac, err_sta_mac_db, err_sta_mac_entry);
			SLIST_INSERT_HEAD(&err_sta->err_sta_head, err_mac, err_sta_mac_entry);
			err_mac = err_mac_tmp;
		}
	}

	return 0;

fail:
	if (integrity&BIT(0) && !SLIST_EMPTY(&err_sta_list.err_sta_head)) {
		err_mac = SLIST_FIRST(&err_sta_list.err_sta_head);
		while (err_mac) {
			err_mac_tmp = SLIST_NEXT(err_mac, err_sta_mac_entry);
			SLIST_REMOVE(&err_sta_list.err_sta_head, err_mac, err_sta_mac_db, err_sta_mac_entry);
			free(err_mac);
			err_mac = err_mac_tmp;
		}
	}
	return -1;
}

int delete_exist_metrics_policy(struct p1905_managerd_ctx *ctx, struct metrics_policy *mpolicy)
{
	struct metric_policy_db *policy = NULL, *policy_tmp = NULL;

	policy = SLIST_FIRST(&mpolicy->policy_head);
	while(policy != NULL) {
		debug(DEBUG_TRACE, "metric_policy_db identifier("MACSTR")\n",
				PRINT_MAC(policy->identifier));
		debug(DEBUG_TRACE, "rssi_thres=%d, hysteresis_margin=%d, ch_util_thres=%d"
			"rssi_thres=%d, hysteresis_margin=%d\n",
			policy->rssi_thres, policy->hysteresis_margin, policy->ch_util_thres,
			policy->sta_stats_inclusion, policy->sta_metrics_inclusion);
		policy_tmp = SLIST_NEXT(policy, policy_entry);
		free(policy);
		policy = policy_tmp;
	}
	SLIST_INIT(&mpolicy->policy_head);
	mpolicy->report_interval = 0;
	mpolicy->radio_num = 0;

	return wapp_utils_success;
}

int fill_metrics_policy(struct p1905_managerd_ctx *ctx, struct metrics_policy *mpolicy)
{
	struct metric_policy_db *policy = NULL, *policy_tmp = NULL;

	policy = SLIST_FIRST(&mpolicy->policy_head);
	while (policy != NULL) {

		policy_tmp = SLIST_NEXT(policy, policy_entry);
		SLIST_REMOVE(&(mpolicy->policy_head), policy, metric_policy_db, policy_entry);
		SLIST_INSERT_HEAD(&(ctx->map_policy.mpolicy.policy_head), policy, policy_entry);
		policy = policy_tmp;
	}
	ctx->map_policy.mpolicy.report_interval = mpolicy->report_interval;
	ctx->map_policy.mpolicy.radio_num = mpolicy->radio_num;
	mpolicy->report_interval = 0;
	mpolicy->radio_num = 0;

	return wapp_utils_success;
}

void update_metrics_policy(struct p1905_managerd_ctx *ctx, struct metrics_policy *mpolicy)
{
	/*delete the previously reserved metrics policy*/
	delete_exist_metrics_policy(ctx, &ctx->map_policy.mpolicy);
	fill_metrics_policy(ctx, mpolicy);
}

int parse_map_policy_config_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned char *val = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;
	unsigned int all_integrity = 0xFF;
	struct metrics_policy mpolicy = {0};
#ifdef MAP_R2
	struct vs_value vs_info;
	struct def_8021q_setting setting;
	struct traffics_policy tpolicy;
	struct traffic_separation_db *tpolicy_db = NULL;
	struct traffic_separation_db *tpolicy_db_nxt = NULL;
	int i = 0;

	os_memset(&setting, 0, sizeof(struct def_8021q_setting));
	os_memset(&tpolicy, 0, sizeof(struct traffics_policy));
	os_memset(&vs_info, 0, sizeof(struct vs_value));
	SLIST_INIT(&tpolicy.policy_head);
	vs_info.parse_ts = 1;
#endif

	type = buf;
	reset_stored_tlv(ctx);
	SLIST_INIT(&(mpolicy.policy_head));

	while(1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			goto fail;
		}
		val = type + 3;

		/* Zero or one Steering Policy TLV */
		if (*type == STEERING_POLICY_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* Zero or one Metric Reporting Policy TLV */
		else if (*type == METRIC_REPORTING_POLICY_TYPE) {
			integrity |= (1 << 1);
			ret = parse_metric_reporting_policy_tlv(ctx, len, val, &mpolicy);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error metric reporting policy tlv\n");
				goto fail;
			}
			store_revd_tlvs(ctx, type, tlv_len);

		}
#ifdef MAP_R2
		/* Zero or one Channel Scan Reporting Policy TLV */
		else if (*type == CHANNEL_SCAN_REPORTING_POLICY_TYPE) {
			integrity |= (1 << 2);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* Zero or one Default 802.1Q Settings TLV */
		else if (*type == DEFAULT_8021Q_SETTING_TYPE) {
			ret = parse_default_802_1q_setting_tlv(&setting, len, val);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error metric reporting policy tlv\n");
				goto fail;
			}
			integrity |= (1 << 3);
			setting.updated = 1;
		}
		/* Zero or one Traffic Separation Policy TLV */
		else if (*type == TRAFFIC_SEPARATION_POLICY_TYPE) {
			ret = parse_traffic_separation_policy_tlv(&tpolicy, len, val);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error trffic separation policy tlv\n");
				goto fail;
			}
			integrity |= (1 << 4);
			tpolicy.updated = 1;
		} else if (*type == VENDOR_SPECIFIC_TLV_TYPE) {
			ret = parse_vs_tlv(&vs_info, len, val);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error vs tlv\n");
				goto fail;
			}
		}
#if 0
		else if (*temp_buf == PACKET_FILTERING_POLICY_TYPE) {
			integrity |= (1 << 5);
			/*delete the previously reserved packet filtering policy*/
			delete_exist_pfilter_policy(ctx, &ctx->map_policy.fpolicy);

			length = parse_packet_filtering_policy_tlv(temp_buf, ctx);
			if(length < 0)
			{
				debug(DEBUG_ERROR, "%s error packet filtering policy tlv\n", __func__);
				return -1;
			}
			temp_buf += length;
		}
		/* Zero or one Ethernet Configuration Policy TLV */
		else if (*temp_buf == ETHERNET_CONFIGURATION_POLICY_TYPE) {
			integrity |= (1 << 6);
			/*delete the previously reserved eth config policy*/
			delete_exist_eth_config_policy(ctx, &ctx->map_policy.epolicy);

			length = parse_eth_config_policy_tlv(temp_buf, ctx);
			if(length < 0)
			{
				debug(DEBUG_ERROR, "%s error ether config policy tlv\n", __func__);
				return -1;
			}
			temp_buf += length;
		}
#endif
		/* Zero or one Unsuccessful Association Policy TLV */
		else if (*type == UNSUCCESSFUL_ASSOCIATION_POLICY_TYPE) {
			integrity |= (1 << 7);
			//length = parse_unsuccess_assoc_policy_tlv(temp_buf, ctx);
			store_revd_tlvs(ctx, type, tlv_len);
		}
#endif
		else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if ((integrity & all_integrity) == 0) {
		debug(DEBUG_ERROR, "incomplete policy config request 0x%x 0x%x\n",
			integrity, all_integrity);
		goto fail;
	}

	if (integrity & BIT(1))
		update_metrics_policy(ctx, &mpolicy);

#ifdef MAP_R2
	if (vs_info.trans_vlan.updated == 1 && vs_info.parse_ts) {
		ctx->map_policy.trans_vlan.num = vs_info.trans_vlan.num;
		debug(DEBUG_ERROR, "update transparent vlan number=%d\n", ctx->map_policy.trans_vlan.num);
		for (i = 0; i < vs_info.trans_vlan.num; i++) {
			ctx->map_policy.trans_vlan.transparent_vids[i] = vs_info.trans_vlan.transparent_vids[i];

			debug(DEBUG_ERROR, "parse transparent vlan id(%d)\n",
				ctx->map_policy.trans_vlan.transparent_vids[i]);
		}
		ctx->map_policy.trans_vlan.updated = 1;
	}
#endif

#ifdef MAP_R2
	if (ctx->role == AGENT) {
		if (ctx->MAP_Cer) {
			/*if no default 802.1q setting*/
			if (!(integrity & BIT(3))) {
				ctx->map_policy.setting.primary_vid = VLAN_N_VID;
				ctx->map_policy.setting.updated = 1;
			} else
				os_memcpy(&ctx->map_policy.setting, &setting, sizeof(struct def_8021q_setting));

			/*if no traffic separation policy tlv*/
			if (!(integrity & BIT(4))) {
				delete_exist_traffic_policy(ctx, &ctx->map_policy.tpolicy);
				ctx->map_policy.tpolicy.updated = 1;
			} else {
				/*delete the previously reserved traffic policy*/
				delete_exist_traffic_policy(ctx, &ctx->map_policy.tpolicy);

				ctx->map_policy.tpolicy.SSID_num = tpolicy.SSID_num;
				ctx->map_policy.tpolicy.updated = tpolicy.updated;


				tpolicy_db = SLIST_FIRST(&(tpolicy.policy_head));
				while (tpolicy_db) {
					tpolicy_db_nxt = SLIST_NEXT(tpolicy_db, policy_entry);
					SLIST_REMOVE(&(tpolicy.policy_head), tpolicy_db,
								traffic_separation_db, policy_entry);

					SLIST_INSERT_HEAD(&ctx->map_policy.tpolicy.policy_head, tpolicy_db, policy_entry);
					tpolicy_db = tpolicy_db_nxt;
				}
			}

			eloop_register_timeout(0, 0, map_r2_notify_ts_config, (void *)ctx, NULL);
		} else {
			if (integrity & BIT(3))
				os_memcpy(&ctx->map_policy.setting, &setting, sizeof(struct def_8021q_setting));

			if (integrity & BIT(4)) {
				/*delete the previously reserved traffic policy*/
				delete_exist_traffic_policy(ctx, &ctx->map_policy.tpolicy);

				ctx->map_policy.tpolicy.SSID_num = tpolicy.SSID_num;
				ctx->map_policy.tpolicy.updated = tpolicy.updated;


				tpolicy_db = SLIST_FIRST(&(tpolicy.policy_head));
				while (tpolicy_db) {
					tpolicy_db_nxt = SLIST_NEXT(tpolicy_db, policy_entry);
					SLIST_REMOVE(&(tpolicy.policy_head), tpolicy_db,
								traffic_separation_db, policy_entry);

					SLIST_INSERT_HEAD(&ctx->map_policy.tpolicy.policy_head, tpolicy_db, policy_entry);
					tpolicy_db = tpolicy_db_nxt;
				}
			}

			if ((integrity & BIT(3)) || (integrity & BIT(4)))
				eloop_register_timeout(0, 0, map_r2_notify_ts_config, (void *)ctx, NULL);
		}
	}
#endif

	return 0;

fail:
#ifdef MAP_R2
	tpolicy_db = SLIST_FIRST(&(tpolicy.policy_head));
	while (tpolicy_db) {
		tpolicy_db_nxt = SLIST_NEXT(tpolicy_db, policy_entry);
		SLIST_REMOVE(&(tpolicy.policy_head), tpolicy_db,
			traffic_separation_db, policy_entry);
		os_free(tpolicy_db);
		tpolicy_db = tpolicy_db_nxt;
	}
#endif
	return -1;

}

int append_reason_code_tlv(unsigned char *pkt,unsigned short reason)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf = NULL;

	temp_buf= pkt;
	*temp_buf++ = REASON_CODE_TYPE;

	*(unsigned short *)(temp_buf) = host_to_be16(2);
	temp_buf += 2;

	memcpy(temp_buf, &reason, 2);
	temp_buf += 2;

	total_length = (temp_buf - pkt);
	return total_length;
}


unsigned short failed_connection_message(unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(FAILED_CONNECTION_MESSAGE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}
int parse_cli_assoc_control_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned char *val = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;
	unsigned int right_integrity = 0x1;
	struct control_policy steer_cntrl = {0};
	struct control_policy_db *policy = NULL;
	struct control_policy_db *policy_tmp = NULL;

	type = buf;
	reset_stored_tlv(ctx);

	while(1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			goto fail;
		}
		val = type + 3;

		if (*type == CLI_ASSOC_CONTROL_REQUEST_TYPE) {
			integrity |= (1 << 0);
			ret = parse_cli_assoc_control_request_tlv(ctx, len, val, &steer_cntrl);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error cli assoc control request tlv\n");
				goto fail;
			}
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != right_integrity) {
		debug(DEBUG_ERROR, "incomplete client association control request 0x%x 0x%x\n",
			integrity, right_integrity);
		goto fail;
	}

	if (integrity&BIT(0))
		update_control_policy_info(&(ctx->steer_cntrl), &steer_cntrl);
	return 0;

fail:
	if ((integrity&BIT(0)) && !SLIST_EMPTY(&(steer_cntrl.policy_head))) {
		policy = SLIST_FIRST(&(steer_cntrl.policy_head));
		while (policy) {
			policy_tmp = SLIST_NEXT(policy, policy_entry);
			SLIST_REMOVE(&(steer_cntrl.policy_head), policy, control_policy_db, policy_entry);
			free(policy);
			policy = policy_tmp;
		}
	}
	return -1;
}

/*link metrics collection*/

void delete_ap_metrics_query_head(struct list_head_metrics_query *head)
{
	struct bss_db *bss = SLIST_FIRST(head);
	struct bss_db *bss_tmp = NULL;

	while(bss != NULL) {
		bss_tmp = SLIST_NEXT(bss, bss_entry);
		debug(DEBUG_TRACE, "free bss_db(id="MACSTR"))\n",
			PRINT_MAC(bss->bssid));
		free(bss);
		bss = bss_tmp;
	}
	SLIST_INIT(head);
}

int parse_ap_metrics_query_tlv(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == AP_METRICS_QUERY_TYPE) {
		temp_buf++;
	}
	else {
		return -1;
	}

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	return (length+3);
}

int parse_ap_metrics_query_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len,
	unsigned char *radio_identifier)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
#ifdef MAP_R2
	unsigned char *val = NULL;
	int ret = 0;
#endif
	unsigned short len = 0;
	unsigned char integrity = 0;
	unsigned int all_integrity = 0xff;
	unsigned char radio_if[ETH_ALEN] = {0};
	unsigned char zero_if[ETH_ALEN] = {0};

	type = buf;
	reset_stored_tlv(ctx);
#ifdef MAP_R2
	delete_exist_radio_identifier(ctx);
#endif
	while(1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}
#ifdef MAP_R2
		val = type + 3;
#endif
		/* One AP metric query TLV */
		if (*type == AP_METRICS_QUERY_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		}
#ifdef MAP_R2
		/* Zero or more AP Radio Identifier TLVs */
		else if (*type == AP_RADIO_IDENTIFIER_TYPE) {
			integrity |= (1 << 1);


			/* need save ap radio identifier for each radio */
			ret = parse_ap_radio_identifier_tlv(ctx, radio_if
#ifdef MAP_R2
			, 1
#endif
			, len, val);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error ap radio identifier tlv\n");
				return -1;
			}
			store_revd_tlvs(ctx, type, tlv_len);
		}
#endif
		else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if ((integrity & all_integrity) == 0) {
		debug(DEBUG_ERROR, "incomplete ap metrics query message 0x%x 0x%x\n",
			integrity, all_integrity);
		return -1;
	}

	if ((integrity&BIT(1)) && (os_memcmp(radio_if, zero_if, ETH_ALEN)))
		os_memcpy(radio_identifier, radio_if, ETH_ALEN);
	return 0;
}

int parse_associated_sta_link_metrics_query_tlv(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == STA_MAC_ADDRESS_TYPE) {
		temp_buf++;
	}
	else {
		return -1;
	}

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	return (length+3);
}

int parse_associated_sta_link_metrics_query_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while(1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == STA_MAC_ADDRESS_TYPE) {
			integrity |= 0x1;
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if(integrity != 0x1) {
		debug(DEBUG_ERROR, "no sta mac address tlv\n");
		return -1;
	}

	return 0;
}

int parse_associated_sta_link_metrics_rsp_tlv(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == ASSOC_STA_LINK_METRICS_TYPE)
		temp_buf++;
	else
		return -1;

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	return (length+3);
}

int parse_error_code_tlv(struct p1905_managerd_ctx *ctx, unsigned short len, unsigned char *value)
{
	if (len != 7)
		return -1;

	return 0;
}

int parse_associated_sta_link_metrics_rsp_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0, all_integrity = 0xff;

	type = buf;
	reset_stored_tlv(ctx);

	while(1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == ASSOC_STA_LINK_METRICS_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* Zero or one Error Code TLV */
		else if (*type == ERROR_CODE_TYPE) {
			integrity |= (1 << 1);
			store_revd_tlvs(ctx, type, tlv_len);
		}
#ifdef MAP_R2
		/* One or more Associated STA Extended Link Metrics TLVs */
		else if (*type == ASSOCIATED_STA_EXTENDED_LINK_METRIC_TYPE) {
			integrity |= (1 << 2);
			store_revd_tlvs(ctx, type, tlv_len);
		}
#endif

#ifdef MAP_R3
		else if (*type == 0xB0) {
			integrity |= (1 << 6);
			store_revd_tlvs(ctx, type, tlv_len);
		}
#endif
		else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if ((integrity & all_integrity) == 0) {
		debug(DEBUG_ERROR, "incomplete associated sta link metrics rsp message 0x%x\n",
			integrity);
		return -1;
	}

	return 0;
}

int parse_unassociated_sta_link_metrics_query_tlv(struct p1905_managerd_ctx *ctx,
	unsigned short len, unsigned char *val)
{
	unsigned char *temp_buf = val;
	unsigned short query_len = 0;
	unsigned char opclass = 0, num_sta = 0, num_ch = 0;
	unsigned char channels[MAX_CH_NUM] = { 0 };
	struct unlink_metrics_query *unlink_query = NULL;
	int left = len;

	if (left - 2 < 0) {
		debug(DEBUG_ERROR, "pre length check for opclass/ch num fail\n");
		goto err0;
	}
	opclass = *temp_buf++;
	num_ch = *temp_buf++;
	if (num_ch == 0 || num_ch > 13) {
		debug(DEBUG_ERROR, "invalid ch num(%d)\n", num_ch);
		goto err0;
	}
	left -= 2;

	if (left - num_ch < 0) {
		debug(DEBUG_ERROR, "pre length check for channels fail\n");
		goto err0;
	}
	memcpy(channels, temp_buf, num_ch);
	temp_buf += num_ch;
	left -= num_ch;

	if (left - 1 < 0) {
		debug(DEBUG_ERROR, "pre length check for sta num fail\n");
		goto err0;
	}
	num_sta = *temp_buf++;
	if (num_sta == 0) {
		debug(DEBUG_ERROR, "invalid sta num(0)\n");
		goto err0;
	}
	left -= 1;

	query_len = sizeof(struct unlink_metrics_query) + num_sta * ETH_ALEN;
	if (ctx->metric_entry.unlink_query) {
		free(ctx->metric_entry.unlink_query);
		ctx->metric_entry.unlink_query = NULL;
	}

	ctx->metric_entry.unlink_query = (struct unlink_metrics_query *)os_zalloc(query_len);
	if (!ctx->metric_entry.unlink_query) {
		debug(DEBUG_ERROR, "alloc ctx->metric_entry.unlink_query fail\n");
		goto err0;
	}
	unlink_query = ctx->metric_entry.unlink_query;
	unlink_query->oper_class = opclass;
	unlink_query->ch_num = num_ch;
	memcpy(unlink_query->ch_list, channels, num_ch);

	if (left - num_sta * ETH_ALEN < 0) {
		debug(DEBUG_ERROR, "pre length check for sta fail\n");
		goto err1;
	}
	unlink_query->sta_num = num_sta;
	memcpy(unlink_query->sta_list, temp_buf, unlink_query->sta_num * ETH_ALEN);

	return 0;

err1:
	if (ctx->metric_entry.unlink_query) {
		free(ctx->metric_entry.unlink_query);
		ctx->metric_entry.unlink_query = NULL;
	}
err0:
	return -1;
}

int parse_unassociated_sta_link_metrics_query_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned char *val = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while(1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}
		val = type + 3;

		if (*type == UNASSOC_STA_LINK_METRICS_QUERY_TYPE) {
			integrity |= 0x1;
			ret = parse_unassociated_sta_link_metrics_query_tlv(ctx, len, val);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error unassociated sta link metrics query tlv\n");
				return -1;
			}
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if(integrity != 0x1) {
		debug(DEBUG_ERROR, "no unassicated sta link metrics query tlv\n");
		return -1;
	}

	return 0;
}

int delete_exist_metrics_info(struct p1905_managerd_ctx *ctx,
	unsigned char *bssid)
{
	 struct mrsp_db *mrsp = NULL;
	 struct esp_db *esp = NULL, *esp_tmp = NULL;

    SLIST_FOREACH(mrsp, &(ctx->metric_entry.metrics_rsp_head), mrsp_entry)
    {
        if(!memcmp(mrsp->bssid, bssid, ETH_ALEN))
        {
        	debug(DEBUG_TRACE, "esp_cnt=%d\n", mrsp->esp_cnt);
            if(!SLIST_EMPTY(&(mrsp->esp_head)))
            {
                esp = SLIST_FIRST(&(mrsp->esp_head));
                while(esp)
                {
                	debug(DEBUG_TRACE, "delete_exist struct esp_db\n");
					debug(DEBUG_TRACE, "ac=%d, format=%d ba_win_size=%d\n",
						esp->ac, esp->format, esp->ba_win_size);
					debug(DEBUG_TRACE, "e_air_time_fraction=%d, ppdu_dur_target=%d\n",
						esp->e_air_time_fraction, esp->ppdu_dur_target);

                    esp_tmp = SLIST_NEXT(esp, esp_entry);
                    SLIST_REMOVE(&(mrsp->esp_head), esp, esp_db, esp_entry);
                    free(esp);
                    esp = esp_tmp;
                }
            }
			debug(DEBUG_TRACE, "delete_exist struct mrsp_db\n");
			debug(DEBUG_TRACE, "bssid("MACSTR")\n",
				PRINT_MAC(mrsp->bssid));
			debug(DEBUG_TRACE, "ch_uti=%d, assoc_sta_cnt=%d\n", mrsp->ch_util,
				mrsp->assoc_sta_cnt);
            SLIST_REMOVE(&(ctx->metric_entry.metrics_rsp_head), mrsp,
                        mrsp_db, mrsp_entry);
            free(mrsp);
            break;
        }
    }

	return wapp_utils_success;
}

int insert_new_metrics_info(struct p1905_managerd_ctx *ctx,
	struct ap_metrics_info *minfo)
{
	struct mrsp_db *mrsp = NULL;
	struct esp_db *esp = NULL;
	struct esp_info *info = NULL;
	int i = 0;

	mrsp = (struct mrsp_db *)malloc(sizeof(struct mrsp_db));
	if (!mrsp) {
		debug(DEBUG_ERROR, "alloc struct mrsp_db fail\n");
		return wapp_utils_error;
	}
	memset(mrsp, 0, sizeof(struct mrsp_db));
	memcpy(mrsp->bssid, minfo->bssid, ETH_ALEN);
	mrsp->ch_util = minfo->ch_util;
	mrsp->assoc_sta_cnt = minfo->assoc_sta_cnt;
	mrsp->esp_cnt = minfo->valid_esp_count;
	SLIST_INIT(&(mrsp->esp_head));
	SLIST_INSERT_HEAD(&(ctx->metric_entry.metrics_rsp_head), mrsp, mrsp_entry);

	debug(DEBUG_TRACE, "insert struct mrsp_db\n");
	debug(DEBUG_TRACE, "bssid("MACSTR")\n",
		PRINT_MAC(mrsp->bssid));
	debug(DEBUG_TRACE, "ch_uti=%d, assoc_sta_cnt=%d, esp_cnt=%d\n", mrsp->ch_util,
				mrsp->assoc_sta_cnt, mrsp->esp_cnt);

	for(i = 0; i < mrsp->esp_cnt; i++) {
		info = &minfo->esp[i];
		esp = (struct esp_db *)malloc(sizeof(struct esp_db));
		if (!esp) {
			debug(DEBUG_TRACE, "alloc struct esp_db fail\n");
			return wapp_utils_error;
		}
		esp->ac = info->ac;
		esp->format = info->format;
		esp->ba_win_size = info->ba_win_size;
		esp->e_air_time_fraction = info->e_air_time_fraction;
		esp->ppdu_dur_target = info->ppdu_dur_target;
		SLIST_INSERT_HEAD(&(mrsp->esp_head), esp, esp_entry);

		debug(DEBUG_TRACE, "insert struct esp_db\n");
		debug(DEBUG_TRACE, "ac=%d, format=%d ba_win_size=%d\n",
			esp->ac, esp->format, esp->ba_win_size);
		debug(DEBUG_TRACE, "e_air_time_fraction=%d, ppdu_dur_target=%d\n",
			esp->e_air_time_fraction, esp->ppdu_dur_target);
	}

	return wapp_utils_success;

}

unsigned short append_ap_metrics_tlv(
        unsigned char *pkt, struct mrsp_db *mrsp)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;
	struct esp_db *esp = NULL;
	unsigned char ac[4] = {1,0,3,2};
	unsigned char i = 0;
	unsigned char *esp_pos = NULL;

    temp_buf = pkt;

    *temp_buf++ = AP_METRICS_TYPE;
	total_length += 1;

    temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, mrsp->bssid, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	*temp_buf++ = mrsp->ch_util;
	total_length += 1;

	*(unsigned short *)(temp_buf) = host_to_be16(mrsp->assoc_sta_cnt);
	temp_buf += 2;
	total_length += 2;

	esp_pos = temp_buf;
	temp_buf += 1;
	total_length += 1;

	*esp_pos = 0;
	for (i = 0; i < 4; i++) {
		SLIST_FOREACH(esp, &(mrsp->esp_head), esp_entry)
	    {
	    	if (esp->ac == ac[i]) {
				*esp_pos |= 1 << (7 - i);
				*temp_buf++ = (esp->ba_win_size << 5) | (esp->format << 3) | esp->ac;
				*temp_buf++ = esp->e_air_time_fraction;
				*temp_buf++ = esp->ppdu_dur_target;
				total_length += 3;
				break;
	    	}
		}
	}

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

    return total_length;
}

unsigned short append_sta_traffic_stats_tlv(unsigned char *pkt, struct stats_db *stats
#ifdef MAP_R2
, unsigned char unit
#endif
)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;
	unsigned int bytes_sent = 0;
	unsigned int bytes_received = 0;

    temp_buf = pkt;

    *temp_buf++ = ASSOC_STA_TRAFFIC_STATS_TYPE;
	total_length += 1;

    temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, stats->mac, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	/* BytesSent && BytesReceived Units specified by R2 AP Capability TLV */
	/* bytes, default, mapd guarantee this unit */
	bytes_sent = stats->bytes_sent;
	bytes_received = stats->bytes_received;
#ifdef MAP_R2
	/*BytesSent*/
	if (unit == 1) {
		/* kibibytes (KiB) */
		bytes_sent = stats->bytes_sent/1024;
		bytes_received = stats->bytes_received/1024;
	}
	else if (unit == 2) {
		/* mebibytes (MiB) */
		bytes_sent = stats->bytes_sent/1024/1024;
		bytes_received = stats->bytes_received/1024/1024;
	}
#endif

	*(unsigned int *)(temp_buf) = host_to_be32(bytes_sent);
	temp_buf += 4;
	total_length += 4;

	/*BytesReceived*/
	*(unsigned int *)(temp_buf) = host_to_be32(bytes_received);
	temp_buf += 4;
	total_length += 4;

	/*PacketsSent*/
	*(unsigned int *)(temp_buf) = host_to_be32(stats->packets_sent);
	temp_buf += 4;
	total_length += 4;

	/*PacketsReceived*/
	*(unsigned int *)(temp_buf) = host_to_be32(stats->packets_received);
	temp_buf += 4;
	total_length += 4;


	/*TxPacketsErrors*/
	*(unsigned int *)(temp_buf) = host_to_be32(stats->tx_packets_errors);
	temp_buf += 4;
	total_length += 4;

	/*RxPacketsErrors*/
	*(unsigned int *)(temp_buf) = host_to_be32(stats->rx_packets_errors);
	temp_buf += 4;
	total_length += 4;

	/*RetransmissionCount*/
	*(unsigned int *)(temp_buf) = host_to_be32(stats->retransmission_count);
	temp_buf += 4;
	total_length += 4;

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

    return total_length;
}

unsigned short append_sta_link_metrics_tlv(
        unsigned char *pkt, struct metrics_db *metrics)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;

    temp_buf = pkt;

    *temp_buf++ = ASSOC_STA_LINK_METRICS_TYPE;
	total_length += 1;

    temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, metrics->mac, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	/*BSSID number*/
	*temp_buf++ = 1;
	total_length += 1;

	memcpy(temp_buf, metrics->bssid, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	/*TimeDelta*/
	*(unsigned int *)(temp_buf) = host_to_be32(metrics->time_delta);
	temp_buf += 4;
	total_length += 4;

	/*Estimated MAC Date Rate in Downlink*/
	*(unsigned int *)(temp_buf) = host_to_be32(metrics->erate_downlink);
	temp_buf += 4;
	total_length += 4;

	/*Estimated MAC Date Rate in Uplink*/
	*(unsigned int *)(temp_buf) = host_to_be32(metrics->erate_uplink);
	temp_buf += 4;
	total_length += 4;

	/*Measured Uplink RSSI*/
	*temp_buf++ = metrics->rssi_uplink & 0xff;
	total_length += 1;

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

    return total_length;
}

int delete_exist_traffic_stats_info(struct p1905_managerd_ctx *ctx,
	unsigned char *identifier)
{
	struct traffic_stats_db *traffic_stats = NULL;
	struct stats_db *stats = NULL, *stats_tmp = NULL;

    SLIST_FOREACH(traffic_stats, &ctx->metric_entry.traffic_stats_head, traffic_stats_entry)
    {
        if(!memcmp(traffic_stats->identifier, identifier, ETH_ALEN)) {
        	debug(DEBUG_TRACE, "sta_cnt=%d\n", traffic_stats->sta_cnt);

            stats = SLIST_FIRST(&traffic_stats->stats_head);
            while(stats) {
            	debug(DEBUG_TRACE, "delete_exist struct stats_db\n");
				debug(DEBUG_TRACE, "sta mac("MACSTR")\n", PRINT_MAC(stats->mac));
				debug(DEBUG_TRACE, "bytes_sent=%d, bytes_received=%d"
					"packets_sent=%d, packets_received=%d"
					"tx_packets_errors=%d, rx_packets_errors=%d"
					"retransmission_count=%d\n",
					stats->bytes_sent, stats->bytes_received,
					stats->packets_sent, stats->packets_received,
					stats->tx_packets_errors, stats->rx_packets_errors,
					stats->retransmission_count);

                stats_tmp = SLIST_NEXT(stats, stats_entry);
                SLIST_REMOVE(&traffic_stats->stats_head, stats, stats_db, stats_entry);
                free(stats);
                stats = stats_tmp;
            }
			debug(DEBUG_TRACE, "delete_exist struct traffic_stats_db\n");
			debug(DEBUG_TRACE, "identifier("MACSTR")\n",
				PRINT_MAC(traffic_stats->identifier));
            SLIST_REMOVE(&ctx->metric_entry.traffic_stats_head, traffic_stats,
                        traffic_stats_db, traffic_stats_entry);
            free(traffic_stats);
            break;
        }
    }

	return wapp_utils_success;
}

int insert_new_traffic_stats_info(struct p1905_managerd_ctx *ctx,
	struct sta_traffic_stats *traffic_stats)
{
	struct traffic_stats_db *tstats = NULL;
	struct stats_db *stats = NULL;
	struct stat_info *info = NULL;
	int i = 0;

	tstats = (struct traffic_stats_db *)malloc(sizeof(struct traffic_stats_db));
	if (!tstats) {
		debug(DEBUG_ERROR, "alloc struct traffic_stats_db fail\n");
		return wapp_utils_error;
	}
	memset(tstats, 0, sizeof(struct traffic_stats_db));
	memcpy(tstats->identifier, traffic_stats->identifier, ETH_ALEN);
	tstats->sta_cnt= traffic_stats->sta_cnt;
	SLIST_INIT(&tstats->stats_head);
	SLIST_INSERT_HEAD(&(ctx->metric_entry.traffic_stats_head), tstats, traffic_stats_entry);

	debug(DEBUG_TRACE, "insert struct traffic_stats_db\n");
	debug(DEBUG_TRACE, "identifier("MACSTR")\n",
		PRINT_MAC(tstats->identifier));
	debug(DEBUG_TRACE, "sta_cnt=%d\n", tstats->sta_cnt);

	for(i = 0; i < traffic_stats->sta_cnt; i++) {
		info = &traffic_stats->stats[i];
		stats = (struct stats_db *)malloc(sizeof(struct stats_db));
		if (!stats) {
			debug(DEBUG_ERROR, "alloc struct stats_db fail\n");
			return wapp_utils_error;
		}
		memcpy(stats->mac, info->mac, ETH_ALEN);
		stats->bytes_sent = info->bytes_sent;
		stats->bytes_received = info->bytes_received;
		stats->packets_sent = info->packets_sent;
		stats->packets_received = info->packets_received;
		stats->tx_packets_errors = info->tx_packets_errors;
		stats->rx_packets_errors = info->rx_packets_errors;
		stats->retransmission_count = info->retransmission_count;
		SLIST_INSERT_HEAD(&tstats->stats_head, stats, stats_entry);

		debug(DEBUG_TRACE, "insert struct stats_db\n");
		debug(DEBUG_TRACE, "sta mac("MACSTR")\n",
			PRINT_MAC(stats->mac));
		debug(DEBUG_TRACE, "bytes_sent=%d, bytes_received=%d packets_sent=%d"
			"packets_received=%d, tx_packets_errors=%d rx_packets_errors=%d"
			"retransmission_count=%d\n",
			stats->bytes_sent, stats->bytes_received, stats->packets_sent,
			stats->packets_received, stats->tx_packets_errors, stats->rx_packets_errors,
			stats->retransmission_count);
	}

	return wapp_utils_success;

}

int delete_exist_link_metrics_info(struct p1905_managerd_ctx *ctx,
	unsigned char *identifier)
{
	struct link_metrics_db *link_metrics = NULL;
	struct metrics_db *metrics = NULL, *metrics_tmp = NULL;

    SLIST_FOREACH(link_metrics, &ctx->metric_entry.link_metrics_head, link_metrics_entry)
    {
        if(!memcmp(link_metrics->identifier, identifier, ETH_ALEN)) {
        	debug(DEBUG_TRACE, "sta_cnt=%d\n", link_metrics->sta_cnt);

            metrics = SLIST_FIRST(&link_metrics->metrics_head);
            while(metrics) {
            	debug(DEBUG_TRACE, "delete_exist struct link_metrics_db\n");
				debug(DEBUG_TRACE, "sts mac("MACSTR")\n", PRINT_MAC(metrics->mac));
				debug(DEBUG_TRACE, "bssid("MACSTR")\n", PRINT_MAC(metrics->bssid));
				debug(DEBUG_TRACE, "time_delta=%d, erate_downlink=%d erate_uplink=%d, rssi_uplink=%d\n",
					metrics->time_delta, metrics->erate_downlink,
					metrics->erate_uplink, metrics->rssi_uplink);

                metrics_tmp = SLIST_NEXT(metrics, metrics_entry);
                SLIST_REMOVE(&link_metrics->metrics_head, metrics, metrics_db, metrics_entry);
                free(metrics);
                metrics = metrics_tmp;
            }
			debug(DEBUG_TRACE, "delete_exist struct link_metrics_db\n");
			debug(DEBUG_TRACE, "identifier("MACSTR")\n",
				PRINT_MAC(link_metrics->identifier));
            SLIST_REMOVE(&ctx->metric_entry.link_metrics_head, link_metrics,
                        link_metrics_db, link_metrics_entry);
            free(link_metrics);
            break;
        }
    }

	return wapp_utils_success;
}

int insert_new_link_metrics_info(struct p1905_managerd_ctx *ctx,
	struct sta_link_metrics *metrics_info)
{
	struct link_metrics_db *link_metrics_ctx = NULL;
	struct metrics_db *metrics_ctx = NULL;
	struct link_metrics *info = NULL;
	int i = 0;

	link_metrics_ctx = (struct link_metrics_db *)malloc(sizeof(struct link_metrics_db));
	if (!link_metrics_ctx) {
		debug(DEBUG_ERROR, "alloc struct link_metrics_db fail\n");
		return wapp_utils_error;
	}
	memset(link_metrics_ctx, 0, sizeof(struct link_metrics_db));
	memcpy(link_metrics_ctx->identifier, metrics_info->identifier, ETH_ALEN);
	link_metrics_ctx->sta_cnt= metrics_info->sta_cnt;
	SLIST_INIT(&link_metrics_ctx->metrics_head);
	SLIST_INSERT_HEAD(&ctx->metric_entry.link_metrics_head, link_metrics_ctx, link_metrics_entry);

	debug(DEBUG_TRACE, "insert struct link_metrics_db\n");
	debug(DEBUG_TRACE, "identifier("MACSTR")\n",
		PRINT_MAC(link_metrics_ctx->identifier));
	debug(DEBUG_TRACE, "sta_cnt=%d\n", link_metrics_ctx->sta_cnt);

	for(i = 0; i < link_metrics_ctx->sta_cnt; i++) {
		info = &metrics_info->info[i];
		metrics_ctx = (struct metrics_db *)malloc(sizeof(struct metrics_db));
		if (!metrics_ctx) {
			debug(DEBUG_ERROR, "alloc struct metrics_db fail\n");
			return wapp_utils_error;
		}
		memcpy(metrics_ctx->mac, info->mac, ETH_ALEN);
		memcpy(metrics_ctx->bssid, info->bssid, ETH_ALEN);
		metrics_ctx->time_delta = info->time_delta;
		metrics_ctx->erate_downlink = info->erate_downlink;
		metrics_ctx->erate_uplink = info->erate_uplink;
		metrics_ctx->rssi_uplink = info->rssi_uplink;
		SLIST_INSERT_HEAD(&link_metrics_ctx->metrics_head, metrics_ctx, metrics_entry);

		debug(DEBUG_TRACE, "insert struct metrics_db\n");
		debug(DEBUG_TRACE, "sta mac("MACSTR")\n",
			PRINT_MAC(metrics_ctx->mac));
		debug(DEBUG_TRACE, "bssid("MACSTR")\n",
			PRINT_MAC(metrics_ctx->bssid));
		debug(DEBUG_TRACE, "time_delta=%d, erate_downlink=%d erate_uplink=%d rssi_uplink=%d\n",
			metrics_ctx->time_delta, metrics_ctx->erate_downlink,
			metrics_ctx->erate_uplink, metrics_ctx->rssi_uplink);
	}

	return wapp_utils_success;

}

int update_one_sta_link_metrics_info(struct p1905_managerd_ctx *ctx,
	struct link_metrics *metrics)
{
	struct metrics_db *metrics_sta = &ctx->metric_entry.assoc_sta_link_metrics;

	memset(metrics_sta, 0, sizeof(struct metrics_db));
	memcpy(metrics_sta->mac, metrics->mac, ETH_ALEN);
	memcpy(metrics_sta->bssid, metrics->bssid, ETH_ALEN);
	metrics_sta->time_delta = metrics->time_delta;
	metrics_sta->erate_downlink = metrics->erate_downlink;
	metrics_sta->erate_uplink = metrics->erate_uplink;
	metrics_sta->rssi_uplink = metrics->rssi_uplink;

	debug(DEBUG_TRACE, "insert struct link_metrics_db\n");
	debug(DEBUG_TRACE, "sta mac("MACSTR")\n",
		PRINT_MAC(metrics_sta->mac));
	debug(DEBUG_TRACE, "bssid("MACSTR")\n",
		PRINT_MAC(metrics_sta->bssid));
	debug(DEBUG_TRACE, "time_delta=%d, erate_downlink=%d erate_uplink=%d rssi_uplink=%d\n",
			metrics_sta->time_delta, metrics_sta->erate_downlink,
			metrics_sta->erate_uplink, metrics_sta->rssi_uplink);

	return wapp_utils_success;

}

unsigned short ap_metrics_query_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(AP_LINK_METRICS_QUERY);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}


unsigned short ap_metrics_response_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	/* one AP Metrics TLV  */
	if(ctx->send_tlv_len != 0)
	{
		length = append_send_tlv(tlv_temp_buf, ctx);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(AP_LINK_METRICS_RESPONSE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short associated_sta_link_metrics_query_message(
	unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(ASSOC_STA_LINK_METRICS_QUERY);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short associated_sta_link_metrics_response_message(
	unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;


	if(ctx->send_tlv_len != 0)
	{
		length = append_send_tlv(tlv_temp_buf, ctx);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(ASSOC_STA_LINK_METRICS_RESPONSE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

int delete_exist_unlink_metrics_rsp(struct unlink_metrics_info *unlink_metrics)
{
	struct unlink_metrics_db *metrics = NULL, *metrics_tmp = NULL;

	unlink_metrics->oper_class = 0;
	unlink_metrics->sta_num = 0;
	metrics = SLIST_FIRST(&unlink_metrics->unlink_metrics_head);
	while(metrics != NULL)
	{
		metrics_tmp = SLIST_NEXT(metrics, unlink_metrics_entry);
		debug(DEBUG_TRACE, "sta mac="MACSTR"\n",
			PRINT_MAC(metrics->mac));
		debug(DEBUG_TRACE, "ch=%d, time_delta=%d, uplink_rssi=%d\n",
				metrics->ch, metrics->time_delta, metrics->uplink_rssi);
		free(metrics);
		metrics = metrics_tmp;
	}
	SLIST_INIT(&unlink_metrics->unlink_metrics_head);

	return wapp_utils_success;
}

int update_unlink_metrics_rsp(struct unlink_metrics_info *unlink_metrics_ctx,
	struct unlink_metrics_rsp *unlink_metrics)
{
	struct unlink_metrics_db *metrics = NULL;
	struct unlink_rsp_sta *info = NULL;
	int i = 0;

	delete_exist_unlink_metrics_rsp(unlink_metrics_ctx);

	unlink_metrics_ctx->oper_class = unlink_metrics->oper_class;
	unlink_metrics_ctx->sta_num = unlink_metrics->sta_num;
	debug(DEBUG_TRACE, "oper_class=%d, sta_num=%d\n",
		unlink_metrics_ctx->oper_class, unlink_metrics_ctx->sta_num);

	SLIST_INIT(&unlink_metrics_ctx->unlink_metrics_head);

	for (i = 0; i < unlink_metrics->sta_num; i++) {
		metrics = (struct unlink_metrics_db *)malloc(sizeof(struct unlink_metrics_db));
		if (!metrics) {
			debug(DEBUG_TRACE, "alloc struct unlink_metrics_db fail\n");
			return wapp_utils_error;
		}
		info = &unlink_metrics->info[i];
		memcpy(metrics->mac, info->mac, ETH_ALEN);
		metrics->ch = info->ch;
		metrics->time_delta= info->time_delta;
		metrics->uplink_rssi = info->uplink_rssi;
		SLIST_INSERT_HEAD(&unlink_metrics_ctx->unlink_metrics_head, metrics, unlink_metrics_entry);
		debug(DEBUG_TRACE, "sta mac="MACSTR"\n",
			PRINT_MAC(metrics->mac));
		debug(DEBUG_TRACE, "ch=%d, time_delta=%d, uplink_rssi=%d\n",
				metrics->ch, metrics->time_delta, metrics->uplink_rssi);
	}

	return wapp_utils_success;

}

unsigned short append_sta_unlink_metrics_response_tlv(
        unsigned char *pkt, struct unlink_metrics_info *metrics)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;
	struct unlink_metrics_db *unlink_metrics = NULL;

    temp_buf = pkt;

    *temp_buf++ = UNASSOC_STA_LINK_METRICS_RSP_TYPE;
	total_length += 1;

    temp_buf += 2;
	total_length += 2;

	/*Operating Class*/
	*temp_buf++ = metrics->oper_class;
	total_length += 1;

	/*STA number*/
	*temp_buf++ = metrics->sta_num;
	total_length += 1;

	SLIST_FOREACH(unlink_metrics, &metrics->unlink_metrics_head, unlink_metrics_entry)
    {
    	/*sta mac*/
    	memcpy(temp_buf, unlink_metrics->mac, ETH_ALEN);
		temp_buf += ETH_ALEN;
		total_length += ETH_ALEN;

		/*channel*/
		*temp_buf++ = unlink_metrics->ch;
		total_length += 1;

		/*TimeDelta*/
		*(unsigned int *)(pkt+1) = host_to_be32(unlink_metrics->time_delta);
		temp_buf += 4;
		total_length += 4;

		/*Measured Uplink RSSI*/
		*temp_buf++ = unlink_metrics->uplink_rssi & 0xff;
		total_length += 1;
	}

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

    return total_length;
}

unsigned short unassociated_sta_link_metrics_query_message(
	unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(UNASSOC_STA_LINK_METRICS_QUERY);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}


unsigned short unassociated_sta_link_metrics_response_message(
	unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	if(ctx->send_tlv_len != 0)
	{
		length = append_send_tlv(tlv_temp_buf, ctx);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(UNASSOC_STA_LINK_METRICS_RESPONSE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short append_beacon_metrics_response_tlv(
        unsigned char *pkt, struct beacon_metrics_rsp *beacon_rsp)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;

    temp_buf = pkt;

    *temp_buf++ = BEACON_METRICS_RESPONSE_TYPE;
	total_length += 1;

    temp_buf += 2;
	total_length += 2;

	/*MAC address of the associated STA for which the Beacon report information is requested*/
	memcpy(temp_buf, beacon_rsp->sta_mac, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	/*reserved field*/
	*temp_buf++ = beacon_rsp->reserved;
	total_length += 1;

	/*Number of measurement report elements*/
	*temp_buf++ = beacon_rsp->bcn_rpt_num;
	total_length += 1;
	memcpy(temp_buf, beacon_rsp->rpt, beacon_rsp->rpt_len);
	total_length += beacon_rsp->rpt_len;


	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

    return total_length;
}

unsigned short beacon_metrics_response_message(
	unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	if(ctx->send_tlv_len != 0)
	{
		length = append_send_tlv(tlv_temp_buf, ctx);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(BEACON_METRICS_RESPONSE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short beacon_metrics_query_message(
	unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(BEACON_METRICS_QUERY);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

int parse_beacon_metrics_query_tlv(struct p1905_managerd_ctx *ctx,
	unsigned short len, unsigned char *val)
{
	unsigned char *temp_buf = val;
	unsigned short query_len = 0;
	struct beacon_metrics_query *bcn_query = NULL;
	unsigned char bssid[ETH_ALEN] = {0};
	unsigned char sta_mac[ETH_ALEN] = {0};
	unsigned char ssid[33] = {0};
	unsigned char opclass = 0, ch = 0, rpt_detail = 0;
	unsigned char ssid_len = 0, num_chrep = 0;
	unsigned char i = 0;
	struct ap_chn_rpt *chn_rpt = NULL;
	int left = len;

	if (left - ETH_ALEN - 2 - ETH_ALEN - 2 < 0) {
		debug(DEBUG_ERROR, "pre length check for beacon metrics query fail\n");
		goto err0;
	}

	memcpy(sta_mac, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	/*opclass*/
	opclass = *temp_buf++;
	/*channel number*/
	ch = *temp_buf++;
	/*bssid*/
	memcpy(bssid, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;
	/*Reporting Detail value*/
	rpt_detail = *temp_buf++;
	/*ssid len & ssid*/
	ssid_len = *temp_buf++;
	left -= (ETH_ALEN + 2 + ETH_ALEN + 2);

	if (left - ssid_len - 1 < 0) {
		debug(DEBUG_ERROR, "pre length check for ssid/ch report fail\n");
		goto err0;
	}
	memcpy(ssid, temp_buf, ssid_len);
	temp_buf += ssid_len;
	/*Number of AP Channel Reports*/
	num_chrep = *temp_buf++;
	left -= (ssid_len + 1);

	query_len = sizeof(struct beacon_metrics_query) +
		num_chrep * sizeof(struct ap_chn_rpt);
	if (ctx->metric_entry.bcn_query) {
		free(ctx->metric_entry.bcn_query);
		ctx->metric_entry.bcn_query = NULL;
	}
	ctx->metric_entry.bcn_query = (struct beacon_metrics_query *)malloc(query_len);
	if (!ctx->metric_entry.bcn_query) {
		debug(DEBUG_ERROR, "alloc struct beacon_metrics_query fail\n");
		goto err0;
	}
	bcn_query = ctx->metric_entry.bcn_query;

	memset(bcn_query, 0, query_len);
	memcpy(bcn_query->sta_mac, sta_mac, ETH_ALEN);
	bcn_query->oper_class = opclass;
	bcn_query->ch = ch;
	memcpy(bcn_query->bssid, bssid, ETH_ALEN);
	bcn_query->rpt_detail_val = rpt_detail;
	bcn_query->ssid_len = ssid_len;
	memcpy(bcn_query->ssid, ssid, ssid_len);
	bcn_query->ap_ch_rpt_num = num_chrep;
	chn_rpt = bcn_query->rpt;
	for (i = 0; i < num_chrep; i++) {
		if (left - 1 < 0) {
			debug(DEBUG_ERROR, "pre length check for ch_rpt len fail\n");
			goto err1;
		}
		/*ap channel report info*/
		chn_rpt->ch_rpt_len = *temp_buf++;
		if (chn_rpt->ch_rpt_len - 1 > MAX_CH_NUM) {
			debug(DEBUG_ERROR, "invalid ch count(%d)\n", chn_rpt->ch_rpt_len - 1);
			goto err1;
		}
		left -= 1;

		if (left - chn_rpt->ch_rpt_len < 0) {
			debug(DEBUG_ERROR, "pre length check for ch_rpt/opclass fail\n");
			goto err1;
		}
		chn_rpt->oper_class = *temp_buf++;
		memcpy(chn_rpt->ch_list, temp_buf, chn_rpt->ch_rpt_len - 1);
		temp_buf += chn_rpt->ch_rpt_len - 1;
		chn_rpt++;
		left -= chn_rpt->ch_rpt_len;
	}

	/*element IDs*/
	if (left - 1 < 0) {
		debug(DEBUG_ERROR, "pre length check for elemnt IDs fail\n");
		goto err1;
	}
	bcn_query->elemnt_num = *temp_buf++;
	left -= 1;

	if (left - bcn_query->elemnt_num < 0) {
		debug(DEBUG_ERROR, "pre length check for elemnt List fail\n");
		goto err1;
	}
	if (bcn_query->elemnt_num > MAX_ELEMNT_NUM)
		bcn_query->elemnt_num = MAX_ELEMNT_NUM;
	memcpy(bcn_query->elemnt_list, temp_buf, bcn_query->elemnt_num);

	return 0;
err1:
	if (ctx->metric_entry.bcn_query) {
		free(ctx->metric_entry.bcn_query);
		ctx->metric_entry.bcn_query = NULL;
	}
err0:
	return -1;
}

int parse_beacon_metrics_query_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned char *val = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while(1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}
		val = type + 3;

		if (*type == BEACON_METRICS_QUERY_TYPE) {
			integrity |= 0x1;
			ret = parse_beacon_metrics_query_tlv(ctx, len, val);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error beacon metrics query tlv\n");
				return -1;
			}
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 0x1) {
		debug(DEBUG_ERROR, "no beacon metrics query tlv\n");
		return -1;
	}

	return 0;
}

unsigned short append_backhaul_steer_response_tlv(
		unsigned char *pkt, struct backhaul_steer_rsp *steer_rsp)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;

	temp_buf = pkt;

	*temp_buf++ = BACKHAUL_STEERING_RESPONSE_TYPE;
	total_length += 1;

	*(unsigned short *)(temp_buf) = host_to_be16(BACKHAUL_STEERING_RESPONSE_LENGTH);
	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, steer_rsp->backhaul_mac, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	memcpy(temp_buf, steer_rsp->target_bssid, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	*temp_buf++ = steer_rsp->status;
	total_length += 1;

	return total_length;
}

unsigned short backhaul_steering_response_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(BACKHAUL_STEERING_RESPONSE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short backhaul_steering_request_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(BACKHAUL_STEERING_REQUEST);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

int parse_backhaul_steering_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while(1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == BACKHAUL_STEERING_REQUEST_TYPE) {
			integrity |= 0x1;
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 0x1) {
		debug(DEBUG_ERROR, "no backhaul steering request tlv\n");
		return -1;
	}

	return 0;
}

int parse_backhaul_steering_rsp_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while(1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == BACKHAUL_STEERING_RESPONSE_TYPE) {
			integrity |= 0x1;
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == ERROR_CODE_TYPE) {
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 0x1) {
		debug(DEBUG_ERROR, "no backhaul steering response tlv\n");
		return -1;
	}

	return 0;
}

int dev_send_1905(struct p1905_managerd_ctx* ctx, struct _1905_cmdu_request* request)
{
	struct p1905_neighbor_info* dev_info = NULL;
	struct topology_response_db *tpgr_db = NULL;
	int i = 0, j = 0, ifidx = -1;
	int is_mal = 0;
	unsigned char al_multi_address[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x13 };
	unsigned char need_update_mid = 1;
	unsigned char *if_name = NULL;

	if(memcmp(al_multi_address, request->dest_al_mac, ETH_ALEN)) {
		for(i = 0; i < ctx->itf_number; i++) {
			if(!LIST_EMPTY(&(ctx->p1905_neighbor_dev[i].p1905nbr_head))) {
				LIST_FOREACH(dev_info, &(ctx->p1905_neighbor_dev[i].p1905nbr_head), p1905nbr_entry) {
					if(!memcmp(dev_info->al_mac_addr, request->dest_al_mac, ETH_ALEN)) {
						ifidx = i;
						break;
					}
				}
			}
		}

		if (ifidx == -1) {
			SLIST_FOREACH(tpgr_db, &(ctx->topology_entry.tprdb_head), tprdb_entry) {
				if(!memcmp(request->dest_al_mac, tpgr_db->al_mac_addr, ETH_ALEN)) {
				   ifidx = tpgr_db->recv_ifid;
				}
			}
		}
	} else {
		is_mal = 1;
		debug(DEBUG_TRACE, "send 1905 broadcast cmdu\n");
	}

	if (-1 == ifidx) {
		if_name = ctx->br_name;
	} else if (ifidx >= 0 && ifidx < ctx->itf_number) {
		if_name = ctx->itf[ifidx].if_name;
	} else {
		debug(DEBUG_ERROR, "error ifidx(%d)\n", ifidx);
		return 0;
	}

	if(request->len > 0)
	{
		debug(DEBUG_TRACE, "DEV_SEND_1905 type=0x%04x, len=%d\n", request->type, request->len);
		ctx->mid++;
		if (is_mal == 1) {
			for(j = 0; j < ctx->itf_number; j++) {
	            insert_cmdu_txq(al_multi_address, ctx->p1905_al_mac_addr,
	            	e_dev_send_1905_request, ctx->mid, ctx->itf[j].if_name, need_update_mid);
#ifdef SUPPORT_CMDU_RELIABLE
				if (request->type == TOPOLOGY_NOTIFICATION ||
						request->type == AP_AUTOCONFIG_RENEW)
					cmdu_reliable_send(ctx, e_dev_send_1905_request, ctx->mid, j);
#endif
			}
		} else {
			debug(DEBUG_TRACE, "DEV_SEND_1905_REQUEST to "MACSTR"\n",
				PRINT_MAC(request->dest_al_mac));
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
				e_dev_send_1905_request, ctx->mid,
				if_name, need_update_mid);
		}
		ctx->pcmdu_tx_buf = request->body;
		ctx->cmdu_tx_msg_type = request->type;
		ctx->cmdu_tx_buf_len = request->len;
		return 0;
	}

	switch(request->type)
	{
		/*this is for test=====start*/
		case TOPOLOGY_DISCOVERY:
			debug(DEBUG_TRACE, "send topology discovery data\n");
			ctx->mid++;
			for(j = 0; j < ctx->itf_number; j++) {
	            insert_cmdu_txq(al_multi_address, ctx->p1905_al_mac_addr,
	            	e_topology_discovery, ctx->mid, ctx->itf[j].if_name, need_update_mid);
			}
			break;
		case TOPOLOGY_NOTIFICATION:
			debug(DEBUG_TRACE, "send topology notification data\n");
			ctx->mid++;
			for(j = 0; j < ctx->itf_number; j++) {
	            insert_cmdu_txq(al_multi_address, ctx->p1905_al_mac_addr,
	            	e_topology_notification, ctx->mid, ctx->itf[j].if_name, need_update_mid);
			}
			break;
		/*this is for test=====end*/
		case TOPOLOGY_QUERY:
			 /* if a AP want to send unicast cmdu to STA, it needs to use interface mac
		   	   * instead of AL mac address
		  	   */
	    	debug(DEBUG_TRACE, "send topology query to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));
	        insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_topology_query, ++ctx->mid,
	                if_name, need_update_mid);
			break;
		case AP_CAPABILITY_QUERY:
			 /* if a AP want to send unicast cmdu to STA, it needs to use interface mac
			   * instead of AL mac address
			   */
			debug(DEBUG_TRACE, "send ap capability query to "MACSTR"\n",
				PRINT_MAC(request->dest_al_mac));
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
					e_ap_capability_query, ++ctx->mid,
					if_name, need_update_mid);
			break;
		case CHANNEL_PREFERENCE_QUERY:
			debug(DEBUG_TRACE, "send channel preference query to "MACSTR"\n",
				PRINT_MAC(request->dest_al_mac));
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
				e_channel_preference_query, ++ctx->mid,
				if_name, need_update_mid);
			break;
		case CHANNEL_SELECTION_REQUEST:
			 /* if a AP want to send unicast cmdu to STA, it needs to use interface mac
		   	   * instead of AL mac address
		  	   */
	    	debug(DEBUG_TRACE, "send channel selection request to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));
			reset_send_tlv(ctx);
	        insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_channel_selection_request, ++ctx->mid,
	                if_name, need_update_mid);
			break;
		case COMBINED_INFRASTRUCTURE_METRICS:
			debug(DEBUG_TRACE, "send combined infrastructure metrics query event to mapd\n");
			_1905_notify_combined_infrastructure_metrics_query(ctx, ctx->p1905_al_mac_addr,
				request->dest_al_mac);

			break;
		default:
			debug(DEBUG_ERROR, "unknown type of message (0x%04x)\n", request->type);
			break;
	}
	return 0;
}


/*process event from 1905.1 library*/
int process_1905_request(struct p1905_managerd_ctx* ctx, struct _1905_cmdu_request* request)
{
	struct p1905_neighbor_info* dev_info = NULL;
	struct topology_response_db *tpgr_db = NULL;
	unsigned int i = 0, j = 0;
	int ifidx = -1;
	unsigned char al_multi_address[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x13 };
#ifdef MAP_R2
	unsigned char all_agent_mac[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
#endif
#ifdef MAP_R3
	struct agent_list_db *agent_info = NULL;
	struct r3_member *peer = NULL;
#endif
	unsigned char need_update_mid = 0;
	unsigned char zero_mac[6] = {0};
	unsigned char *if_name = NULL;
	struct radio_info *r = NULL;
	unsigned char band = 0;

	if (memcmp(al_multi_address, request->dest_al_mac, ETH_ALEN)) {
		for (i = 0; i < ctx->itf_number; i++) {
			if(!LIST_EMPTY(&(ctx->p1905_neighbor_dev[i].p1905nbr_head))) {
				LIST_FOREACH(dev_info, &(ctx->p1905_neighbor_dev[i].p1905nbr_head), p1905nbr_entry) {
					if(!memcmp(dev_info->al_mac_addr, request->dest_al_mac, ETH_ALEN)) {
						ifidx = i;
						break;
					}
				}
			}
		}

		if (ifidx == -1) {
			SLIST_FOREACH(tpgr_db, &(ctx->topology_entry.tprdb_head), tprdb_entry) {
				if(!memcmp(request->dest_al_mac, tpgr_db->al_mac_addr, ETH_ALEN)) {
				   ifidx = tpgr_db->recv_ifid;
				}
			}
		}
	} else {
		debug(DEBUG_TRACE, "send 1905 broadcast cmdu\n");
	}

	if (-1 == ifidx) {
		if_name = ctx->br_name;
	} else if(ifidx >= 0 && ifidx < ctx->itf_number) {
		if_name = ctx->itf[ifidx].if_name;
	} else {
		debug(DEBUG_ERROR, "error ifidx(%d)\n", ifidx);
		return -1;
	}

	/*below code for DEV_SEND_1905 length==0, so 1905 must build the content of 1905 cmdu*/
	debug(DEBUG_TRACE, "receive cmd(0x%04x) from library\n", request->type);
	switch(request->type)
	{
		case TOPOLOGY_DISCOVERY:
			ctx->mid++;
			for(j = 0; j < ctx->itf_number; j++) {
	            insert_cmdu_txq(al_multi_address, ctx->p1905_al_mac_addr,\
	            	e_topology_discovery, ctx->mid, ctx->itf[j].if_name, need_update_mid);
			}
			break;
		case TOPOLOGY_NOTIFICATION:
			ctx->mid++;
			for(j = 0; j < ctx->itf_number; j++) {
	            insert_cmdu_txq(al_multi_address, ctx->p1905_al_mac_addr,
	            	e_topology_notification, ctx->mid, ctx->itf[j].if_name, need_update_mid);
#ifdef SUPPORT_CMDU_RELIABLE
				cmdu_reliable_send(ctx, e_topology_notification, ctx->mid, j);
#endif
			}
			break;
		case TOPOLOGY_QUERY:
			 /* if a AP want to send unicast cmdu to STA, it needs to use interface mac
		   	   * instead of AL mac address
		  	   */
	    	debug(DEBUG_TRACE, "send topology query to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));
	        insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_topology_query, ++ctx->mid,
	                if_name, need_update_mid);
			break;
		case AP_CAPABILITY_QUERY:
			 /* if a AP want to send unicast cmdu to STA, it needs to use interface mac
			   * instead of AL mac address
			   */
			debug(DEBUG_TRACE, "send ap capability query to "MACSTR"\n",
				PRINT_MAC(request->dest_al_mac));
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
					e_ap_capability_query, ++ctx->mid,
					if_name, need_update_mid);
			break;
		case CHANNEL_PREFERENCE_QUERY:
			debug(DEBUG_TRACE, "send channel preference query to "MACSTR"\n",
				PRINT_MAC(request->dest_al_mac));
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
				e_channel_preference_query, ++ctx->mid,
				if_name, need_update_mid);
			break;
		case COMBINED_INFRASTRUCTURE_METRICS:
			debug(DEBUG_TRACE, "send combined infrastructure metrics message to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));
			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			need_update_mid = 1;
	        insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
			e_combined_infrastructure_metrics, ctx->dev_send_1905_mid,
	                if_name, need_update_mid);

			break;
		case LINK_METRICS_QUERY:
			debug(DEBUG_TRACE, "send link metrics query to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));
			memcpy(ctx->link_metric_query_para.target, request->body, ETH_ALEN);
			memcpy(&(ctx->link_metric_query_para.type), request->body + ETH_ALEN, sizeof(unsigned char));
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_link_metric_query, ++ctx->mid,
	                if_name, need_update_mid);
			break;
		case AP_AUTOCONFIG_SEARCH:
			debug(DEBUG_TRACE, "send ap autoconfig search to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));

			if(ctx->controller_search_state != controller_search_idle ||
				ctx->enrolle_state != no_ap_autoconfig)
			{
				debug(DEBUG_TRACE, "now current autoconfig is ongoing, drop this autoconfig search\n");
				break;
			}
#ifdef MAP_R3
			reset_send_tlv(ctx);
			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
#endif // MAP_R3
			ctx->autoconfig_search_mid = ++ctx->mid;
			for(j = 0; j < ctx->itf_number; j++) {
				if ((ctx->itf[j].is_wifi_sta && os_memcmp(ctx->itf[j].vs_info, zero_mac, 6)) ||
					(ctx->itf[j].media_type == IEEE802_3_GROUP)) {
					r = get_rainfo_by_id(ctx, ctx->itf[j].identifier);
					if (r) {
						if (r->band & BAND_2G_CAP)
							band = IEEE802_11_band_2P4G;
						else if (r->band & BAND_5G_CAP)
							band = IEEE802_11_band_5GL;
						else if (r->band & BAND_6G_CAP)
							band = 0xFF;
					} else
						band = IEEE802_11_band_2P4G;
					reset_send_tlv(ctx);
					if (fill_send_tlv(ctx, &band, 1) == -1)
						debug(DEBUG_ERROR, "[%d]fill_send_tlv fail!\n", __LINE__);
					insert_cmdu_txq(al_multi_address, ctx->p1905_al_mac_addr,
						e_ap_autoconfiguration_search, ctx->autoconfig_search_mid,
						ctx->itf[j].if_name, need_update_mid);
					debug(DEBUG_ERROR, "send search on %s mid %04x\n", ctx->itf[j].if_name,
						ctx->autoconfig_search_mid);
#ifdef SUPPORT_CMDU_RELIABLE
					cmdu_reliable_send(ctx, e_ap_autoconfiguration_search,
						ctx->autoconfig_search_mid, j);
#endif
					process_cmdu_txq(ctx, ctx->trx_buf.buf);
				}
			}
			break;
		case AP_AUTOCONFIG_RENEW:
			debug(DEBUG_TRACE, "send ap autoconfig renew to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));

			if (fill_send_tlv(ctx, request->body, request->len) < 0)
				break;

			ctx->mid++;
			for (j = 0; j < ctx->itf_number; j++) {
				insert_cmdu_txq(al_multi_address, ctx->p1905_al_mac_addr,
					e_ap_autoconfiguration_renew, ctx->mid, ctx->itf[j].if_name,
					need_update_mid);
#ifdef SUPPORT_CMDU_RELIABLE
				cmdu_reliable_send(ctx, e_ap_autoconfiguration_renew, ctx->mid, j);
#endif
			}
			break;
		case CLIENT_CAPABILITY_QUERY:
			{
				struct client_info* cinfo = NULL;
				debug(DEBUG_TRACE, "send client capability query to "MACSTR"\n",
				 	PRINT_MAC(request->dest_al_mac));
				if (ctx->revd_tlv.buf_len != 0)
					debug(DEBUG_TRACE, "a 1905 cmdu request exist, drop current one\n");
				cinfo = (struct client_info*)ctx->send_tlv;
				memcpy(cinfo->bssid, request->body, ETH_ALEN);
				memcpy(cinfo->sta_mac, request->body + ETH_ALEN, ETH_ALEN);
				ctx->send_tlv_len = sizeof(struct client_info);
				insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
		                e_cli_capability_query, ++ctx->mid,
		                if_name, need_update_mid);
			}
			break;
		case AP_LINK_METRICS_QUERY:
			{
				debug(DEBUG_TRACE, "send ap link metrics query to "MACSTR"\n",
				 	PRINT_MAC(request->dest_al_mac));

				if(fill_send_tlv(ctx, request->body, request->len) < 0)
				{
					break;
				}
				insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
		                e_ap_metrics_query, ++ctx->mid,
		                if_name, need_update_mid);
			}
			break;
		case ASSOC_STA_LINK_METRICS_QUERY:
			debug(DEBUG_TRACE, "send assoc sta link metrics query to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_associated_sta_link_metrics_query, ++ctx->mid,
	                if_name, need_update_mid);
			break;
		case UNASSOC_STA_LINK_METRICS_QUERY:
			debug(DEBUG_TRACE, "send unassoc sta link metrics query to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_unassociated_sta_link_metrics_query, ++ctx->mid,
	                if_name, need_update_mid);
			break;
		case BEACON_METRICS_QUERY:
			debug(DEBUG_TRACE, "send beacon metrics query to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_beacon_metrics_query, ++ctx->mid,
	                if_name, need_update_mid);
			break;
		case CLIENT_STEERING_REQUEST:
			debug(DEBUG_TRACE, "send client steering request to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_client_steering_request, ++ctx->mid,
	                if_name, need_update_mid);
			break;
		case CLIENT_ASSOC_CONTROL_REQUEST:
			debug(DEBUG_TRACE, "send client assoc control request to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_client_association_control_request, ++ctx->mid,
	                if_name, need_update_mid);
			break;
		case HIGHER_LAYER_DATA_MESSAGE:
			debug(DEBUG_TRACE, "send higher layer data message to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				debug(DEBUG_TRACE, "a 1905 cmdu request exist, drop current one\n");
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_higher_layer_data, ++ctx->mid,
	                if_name, need_update_mid);
			break;
		case BACKHAUL_STEERING_REQUEST:
			debug(DEBUG_TRACE, "send backhaul steering request to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_backhaul_steering_request, ++ctx->mid,
	                if_name, need_update_mid);
			break;
		case CHANNEL_SELECTION_REQUEST:
			debug(DEBUG_TRACE, "send channel select request to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_channel_selection_request, ++ctx->mid,
	                if_name, need_update_mid);
			break;
		case MAP_POLICY_CONFIG_REQUEST:
			debug(DEBUG_TRACE, "send map policy config request to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_multi_ap_policy_config_request, ++ctx->mid,
	                if_name, need_update_mid);
			break;
		case VENDOR_SPECIFIC:
			debug(DEBUG_TRACE, "send VENDOR_SPECIFIC request to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_vendor_specific, ++ctx->mid,
	                if_name, need_update_mid);
			break;

#ifdef MAP_R2
		/*channel scan feature*/
		case CHANNEL_SCAN_REQUEST:
			debug(DEBUG_TRACE, "send channel scan request to "MACSTR"\n",
			 	PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
	                e_channel_scan_request, ++ctx->mid,
	               if_name, need_update_mid);
			break;
		case CHANNEL_SCAN_REPORT:
			debug(DEBUG_TRACE, "send channel scan report to "MACSTR"\n",
				PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
					e_channel_scan_report, ++ctx->mid,
					if_name, need_update_mid);
			break;

		case TUNNELED_MESSAGE:
			debug(DEBUG_TRACE, "send tunneled message to "MACSTR"\n",
				PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
					e_tunneled_message, ++ctx->mid,
					if_name, need_update_mid);
			break;

		case ASSOCIATION_STATUS_NOTIFICATION:
			if (0 == memcmp(all_agent_mac, request->dest_al_mac, ETH_ALEN)) {
				debug(DEBUG_OFF, "send assoc status notification to all map device\n");
			} else {
				debug(DEBUG_TRACE, "send assoc status notification to "MACSTR"\n",
					    PRINT_MAC(request->dest_al_mac));
			}
			if(fill_send_tlv(ctx, request->body, request->len) < 0)
				break;
			ctx->mid++;
			if (0 == memcmp(all_agent_mac, request->dest_al_mac, ETH_ALEN)) {
				for(j = 0; j < ctx->itf_number; j++) {
					insert_cmdu_txq(al_multi_address, ctx->p1905_al_mac_addr,
						e_association_status_notification, ctx->mid, ctx->itf[j].if_name, need_update_mid);
#ifdef SUPPORT_CMDU_RELIABLE
					cmdu_reliable_send(ctx, e_association_status_notification, ctx->mid, j);
#endif
				}
			} else {
				insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
					e_association_status_notification, ctx->mid,
					if_name, need_update_mid);
			}
			break;

	    case CAC_REQUEST:
		    debug(DEBUG_TRACE, "send CAC_REQUEST message to "MACSTR"\n",
			    PRINT_MAC(request->dest_al_mac));

		    if(fill_send_tlv(ctx, request->body, request->len) < 0)
		    {
			    break;
		    }
		    insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
				    e_cac_request, ++ctx->mid,
				    if_name, need_update_mid);
		    break;

		case CAC_TERMINATION:
			debug(DEBUG_TRACE, "send CAC_TERMINATION message to "MACSTR"\n",
				PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
					e_cac_termination, ++ctx->mid,
					if_name, need_update_mid);
			break;

		case CLIENT_DISASSOCIATION_STATS:
			debug(DEBUG_TRACE, "send CLIENT_DISASSOCIATION_STATS message to "MACSTR"\n",
				PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
					e_client_disassociation_stats, ++ctx->mid,
					if_name, need_update_mid);
			break;

		case FAILED_CONNECTION_MESSAGE:
			debug(DEBUG_TRACE, "send FAILED_CONNECTION_MESSAGE message to "MACSTR"\n",
				PRINT_MAC(request->dest_al_mac));

			if(fill_send_tlv(ctx, request->body, request->len) < 0)
			{
				break;
			}
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
					e_failed_connection_message, ++ctx->mid,
					if_name, need_update_mid);
			break;
		case BACKHAUL_STA_CAP_QUERY_MESSAGE:
			debug(DEBUG_TRACE, "send BACKHAUL_STA_CAP_QUERY_MESSAGE message to "MACSTR"\n",
				PRINT_MAC(request->dest_al_mac));

			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
					e_backhual_sta_cap_query_message, ++ctx->mid,
					if_name, need_update_mid);
			break;

#endif // #ifdef MAP_R2

#ifdef MAP_R3
		case PROXIED_ENCAP_DPP_MESSAGE:
		{
            unsigned char send_flag= 0;
            debug(DEBUG_ERROR, "recv PROXIED_ENCAP_DPP_MESSAGE\n");
			if(fill_send_tlv(ctx, request->body, request->len) < 0)
				break;
			ctx->mid++;

			if (0 == memcmp(all_agent_mac, request->dest_al_mac, ETH_ALEN)) {
				SLIST_FOREACH(agent_info, &(ctx->agent_head), agent_entry)
				{
					peer = get_r3_member(agent_info->almac);
					/* controller send pkt to agent which onboarded */
					if (ctx->role == CONTROLLER) {

						if (get_agent_all_radio_config_state(agent_info)) {
							send_flag = 1;
							debug(DEBUG_ERROR, "peer R1/R2 onboarded\n");
						}

						if (peer && peer->r3_info.cur_dpp_state == dpp_disc_suc) {
							send_flag = 1;
							debug(DEBUG_ERROR, "peer R3 onboarded\n");
						}
					}
					else {
						/* agent, send directly */
						send_flag = 1;
					}

					if (send_flag) {
						debug(DEBUG_ERROR, "send PROXIED_ENCAP_DPP_MESSAGE to "MACSTR"\n",
									MAC2STR(agent_info->almac));
						insert_cmdu_txq(agent_info->almac, ctx->p1905_al_mac_addr,
							e_proxied_encap_dpp, ctx->mid,
							(ifidx != -1) ? ctx->itf[ifidx].if_name : ctx->br_name,
							need_update_mid);
					}
					else {
						debug(DEBUG_ERROR, "peer "MACSTR" not onboarded\n"
							", can not proxied encap dpp msg\n", MAC2STR(agent_info->almac));
					}
				}
			}
			else {
				find_agent_info(ctx, request->dest_al_mac, &agent_info);
				peer = get_r3_member(request->dest_al_mac);
				if (ctx->role == CONTROLLER) {
					debug(DEBUG_ERROR, "controller check to send the proxied encap dpp\n");
					if (agent_info && get_agent_all_radio_config_state(agent_info)) {
						send_flag = 1;
						debug(DEBUG_ERROR, "peer R1/R2 onboarded\n");
					}

					if (peer && peer->r3_info.cur_dpp_state == dpp_disc_suc) {
						send_flag = 1;
						debug(DEBUG_ERROR, "peer R3 onboarded\n");
					}
				}
				else {
					/* agent, send directly */
					send_flag = 1;
				}

				if (send_flag) {
					debug(DEBUG_ERROR, "send PROXIED_ENCAP_DPP_MESSAGE to "MACSTR"\n",
								MAC2STR(request->dest_al_mac));
					insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
						e_proxied_encap_dpp, ctx->mid,
						(ifidx != -1) ? ctx->itf[ifidx].if_name : ctx->br_name,
						need_update_mid);
				}
				else {
					debug(DEBUG_ERROR, "peer "MACSTR" not onboarded\n"
						", can not send proxied encap dpp msg\n", MAC2STR(request->dest_al_mac));
				}
			}

		}
		break;

		case DIRECT_ENCAP_DPP_MESSAGE:
		{
			debug(DEBUG_ERROR, "recv DIRECT_ENCAP_DPP_MESSAGE\n");

			reset_send_tlv(ctx);
			if(fill_send_tlv(ctx, request->body, request->len) < 0)
				break;
			ctx->mid++;

			if (0 == memcmp(all_agent_mac, request->dest_al_mac, ETH_ALEN)) {
				SLIST_FOREACH(agent_info, &(ctx->agent_head), agent_entry)
				{
					debug(DEBUG_ERROR, "send DIRECT_ENCAP_DPP_MESSAGE to "MACSTR"\n",
								MAC2STR(agent_info->almac));
					insert_cmdu_txq(agent_info->almac, ctx->p1905_al_mac_addr,
						e_direct_encap_dpp, ctx->mid,
						ctx->br_name, need_update_mid);
				}
			}
			else {
				debug(DEBUG_ERROR, "send DIRECT_ENCAP_DPP_MESSAGE to "MACSTR"\n",
							PRINT_MAC(request->dest_al_mac));
				insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
					e_direct_encap_dpp, ctx->mid,
					ctx->br_name, need_update_mid);
			}
		}
		break;

		case CHIRP_NOTIFICATION_MESSAGE:
		{
			reset_send_tlv(ctx);
			if(fill_send_tlv(ctx, request->body, request->len) < 0)
				break;
			if (0 == memcmp(all_agent_mac, request->dest_al_mac, ETH_ALEN)) {
				/* send to all agents */
				if (ctx->role == CONTROLLER) {
					SLIST_FOREACH(agent_info, &(ctx->agent_head), agent_entry) {
						debug(DEBUG_ERROR, "send CHIRP_NOTIFICATION_MESSAGE to "MACSTR"\n",
									MAC2STR(agent_info->almac));
						insert_cmdu_txq(agent_info->almac, ctx->p1905_al_mac_addr,
							e_chirp_notification, ++ctx->mid,
							(ifidx != -1) ? ctx->itf[ifidx].if_name : ctx->br_name, need_update_mid);
					}
				}
				/* send to controller */
				else {
					debug(DEBUG_ERROR, "send CHIRP_NOTIFICATION_MESSAGE to Controller "MACSTR"\n",
								MAC2STR(ctx->cinfo.almac));
					insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
						e_chirp_notification, ++ctx->mid,
						ctx->itf[ctx->cinfo.recv_ifid].if_name, need_update_mid);
				}
			}
			else {
				debug(DEBUG_ERROR, "send CHIRP_NOTIFICATION_MESSAGE to "MACSTR"\n",
							MAC2STR(request->dest_al_mac));
				insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
					e_chirp_notification, ++ctx->mid,
					(ifidx != -1) ? ctx->itf[ifidx].if_name : ctx->br_name, need_update_mid);
			}
		}
        break;

		case DPP_BOOTSTRAPING_URI_NOTIFICATION:
		{
			reset_send_tlv(ctx);
			if(fill_send_tlv(ctx, request->body, request->len) < 0)
				break;

			if (ctx->role != CONTROLLER) {
				debug(DEBUG_ERROR, "send CHIRP_NOTIFICATION_MESSAGE to Controller "MACSTR"\n",
							MAC2STR(ctx->cinfo.almac));
				insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
					e_dpp_bootstrapping_uri_notification, ++ctx->mid,
					ctx->itf[ctx->cinfo.recv_ifid].if_name, need_update_mid);
			}
		}
		break;

		case DPP_BOOTSTRAPING_URI_QUERY:
			debug(DEBUG_ERROR, "send DPP_BOOTSTRAPING_URI_QUERY message to "MACSTR"\n",
				MAC2STR(request->dest_al_mac));
			insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
				e_dpp_bootstrapping_uri_query, ++ctx->mid,
				(ifidx != -1) ? ctx->itf[ifidx].if_name : ctx->br_name, need_update_mid);
		break;

		case DPP_CCE_INDICATION_MESSAGE:
		{
			unsigned char send_flag = 0;

			debug(DEBUG_TRACE, "recv DPP_CCE_INDICATION_MESSAGE\n");
			reset_send_tlv(ctx);
			if(fill_send_tlv(ctx, request->body, request->len) < 0)
				break;
			ctx->mid++;

			if (0 == memcmp(all_agent_mac, request->dest_al_mac, ETH_ALEN)) {
				SLIST_FOREACH(agent_info, &(ctx->agent_head), agent_entry)
				{
					peer = get_r3_member(agent_info->almac);
					/* controller send pkt to agent which onboarded */
					if (ctx->role == CONTROLLER) {
						debug(DEBUG_TRACE, "controller check to send the DPP CCE indication\n");
						if (get_agent_all_radio_config_state(agent_info)) {
							send_flag = 0;
							debug(DEBUG_TRACE, "peer R1/R2 onboarded\n");
						}

						if (peer && peer->r3_info.cur_dpp_state == dpp_disc_suc) {
							send_flag = 1;
							debug(DEBUG_TRACE, "peer R3 onboarded\n");
						}
					}
					else {
						/* agent, send directly */
						send_flag = 1;
					}

					if (send_flag) {
						debug(DEBUG_TRACE, "send DPP_CCE_INDICATION_MESSAGE to "MACSTR"\n",
									MAC2STR(agent_info->almac));
						insert_cmdu_txq(agent_info->almac, ctx->p1905_al_mac_addr,
							e_dpp_cce_indication, ctx->mid,
							(ifidx != -1) ? ctx->itf[ifidx].if_name : ctx->br_name, need_update_mid);
					}
					else {
						debug(DEBUG_TRACE, "peer "MACSTR" not onboarded\n"
							", can not send dpp cce indication\n", MAC2STR(agent_info->almac));
					}
				}
			}
			else {
				find_agent_info(ctx, request->dest_al_mac, &agent_info);
				peer = get_r3_member(request->dest_al_mac);
				/* controller send pkt to agent which onboarded */
				if (ctx->role == CONTROLLER) {
					debug(DEBUG_TRACE, "controller check to send the DPP CCE indication\n");
					if (agent_info && get_agent_all_radio_config_state(agent_info)) {
						send_flag = 1;
						debug(DEBUG_TRACE, "peer R1/R2 onboarded\n");
					}

					if (peer && peer->r3_info.cur_dpp_state == dpp_disc_suc) {
						send_flag = 1;
						debug(DEBUG_TRACE, "peer R3 onboarded\n");
					}
				}
				else {
					/* agent, send directly */
					send_flag = 1;
				}

				if (send_flag) {
					debug(DEBUG_TRACE, "send DPP_CCE_INDICATION_MESSAGE to "MACSTR"\n",
								MAC2STR(request->dest_al_mac));
					insert_cmdu_txq(request->dest_al_mac, ctx->p1905_al_mac_addr,
						e_dpp_cce_indication, ctx->mid,
						(ifidx != -1) ? ctx->itf[ifidx].if_name : ctx->br_name, need_update_mid);
				}
				else {
					debug(DEBUG_TRACE, "peer "MACSTR" not onboarded\n"
						", can not send dpp cce indication\n", MAC2STR(request->dest_al_mac));
				}
			}
		}
		break;

#endif // MAP_R3
		default:
			debug(DEBUG_OFF, "unknown type of message\n");
			break;
	}
	return 0;
}

void update_radio_info(struct p1905_managerd_ctx* ctx, struct wps_get_config* info)
{
	unsigned char i = 0, j = 0;
	unsigned int k = 0;
	unsigned char add_num = 0;

	for(i = 0; i < ctx->radio_number; i++)
	{
		if(!memcmp(ctx->rinfo[i].identifier, info->identifier, ETH_ALEN))
		{
			ctx->rinfo[i].band = info->band;
			ctx->rinfo[i].dev_type = info->dev_type;

			debug(DEBUG_OFF, "add new radio("MACSTR") band=%d, bssnum=%d\n",
				PRINT_MAC(info->identifier), info->band, info->num_of_inf);
			for(k = 0; k < info->num_of_inf; k++)
			{
				for(j = 0; j < ctx->rinfo[i].bss_number; j++)
				{

					if(!memcmp(ctx->rinfo[i].bss[j].ifname, info->inf_data[k].ifname,
							strlen((char*)ctx->rinfo[i].bss[j].ifname)))
					{
						break;
					}
				}
				if(j >= ctx->rinfo[i].bss_number)
				{
					/*new bss, should add new one*/
					(void)snprintf((char *)ctx->rinfo[i].bss[ctx->rinfo[i].bss_number + add_num].ifname,
						sizeof(ctx->rinfo[i].bss[ctx->rinfo[i].bss_number + add_num].ifname),
						"%s", info->inf_data[k].ifname);
					memcpy(ctx->rinfo[i].bss[ctx->rinfo[i].bss_number + add_num].mac, info->inf_data[k].mac_addr, ETH_ALEN);
					ctx->rinfo[i].bss[ctx->rinfo[i].bss_number + add_num].config_status = 0;
					ctx->rinfo[i].bss[ctx->rinfo[i].bss_number + add_num].priority= 0;
					add_num++;
					debug(DEBUG_OFF, "readd new bss(%s) to existed radio("MACSTR")\n",
						info->inf_data[k].ifname, PRINT_MAC(info->inf_data[k].mac_addr));
				}
			}
			ctx->rinfo[i].bss_number += add_num;
			debug(DEBUG_OFF, "rbss_number(%d)\n", ctx->rinfo[i].bss_number);
			break;
		}
	}
	/*new radio, add new one*/
	if(i >= ctx->radio_number)
	{
		memcpy(ctx->rinfo[ctx->radio_number].identifier, info->identifier, ETH_ALEN);
		ctx->rinfo[ctx->radio_number].band = info->band;
		ctx->rinfo[ctx->radio_number].dev_type = info->dev_type;
		ctx->rinfo[ctx->radio_number].bss_number = info->num_of_inf;
		ctx->rinfo[ctx->radio_number].teared_down = 0;
		debug(DEBUG_OFF, "add new radio("MACSTR") band=%d, bssnum=%d\n",
			PRINT_MAC(info->identifier), info->band, info->num_of_inf);
		for(i = 0; i < info->num_of_inf; i++)
		{
			(void)snprintf((char *)ctx->rinfo[ctx->radio_number].bss[i].ifname,
				sizeof(ctx->rinfo[ctx->radio_number].bss[i].ifname),
				"%s", info->inf_data[i].ifname);
			memcpy(ctx->rinfo[ctx->radio_number].bss[i].mac, info->inf_data[i].mac_addr, ETH_ALEN);
			ctx->rinfo[ctx->radio_number].bss[i].config_status = 0;
			ctx->rinfo[i].bss[ctx->rinfo[i].bss_number + add_num].priority= 0;
			debug(DEBUG_OFF, "add new bss(%s) to above new radio\n", ctx->rinfo[ctx->radio_number].bss[i].ifname);
		}
		ctx->radio_number++;
	}

	/*set the priority to the bss that will be configured*/
	for (i = 0; i < ctx->radio_number; i++) {
		if (!os_memcmp(ctx->rinfo[i].identifier, info->identifier, ETH_ALEN)) {
			for (j = 0; j < ctx->rinfo[i].bss_number; j++) {
				for (k = 0; k < ctx->itf_number; k++) {
					/*if (os_strcmp((char*)ctx->rinfo[i].bss[j].ifname, (char*)ctx->itf[k].if_name) == 0) {*/
					if (os_memcmp((char*)ctx->rinfo[i].bss[j].mac, (char*)ctx->itf[k].mac_addr, ETH_ALEN) == 0) {
						ctx->rinfo[i].bss[j].priority = ctx->itf[k].config_priority;
					}
				}
			}
			break;
		}
	}
}

int set_radio_autoconf_prepare(struct p1905_managerd_ctx* ctx, unsigned int band, unsigned char set_value)
{
	int i = 0;

	for (i = 0; i < ctx->radio_number; i++) {
		if (ctx->rinfo[i].band == band) {
			ctx->rinfo[i].trrigerd_autoconf = set_value;
			debug(DEBUG_OFF, "radio("MACSTR") need %sto do autoconfig\n",
				PRINT_MAC(ctx->rinfo[i].identifier), set_value ? "" : "not ");
		}
	}

	return 0;
}

int set_radio_autoconf_trriger(struct p1905_managerd_ctx *ctx, unsigned char radio_id, unsigned char set_value)
{
	unsigned char radio_index = 0;

	if (radio_id >= ctx->radio_number) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"unknown radio id(%d) total_radio_number(%d)\n",
			radio_id, ctx->radio_number);
		return -1;
	}

	if (set_value== 1) {
		if (ctx->rinfo[radio_id].teared_down == 1) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"radio("MACSTR")teared down, need wait for renew\n",
				PRINT_MAC(ctx->rinfo[radio_id].identifier));
			return -2;
		}
		if (ctx->current_autoconfig_info.radio_index == radio_id) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"radio("MACSTR")ongoing; wait it done\n",
				PRINT_MAC(ctx->rinfo[radio_id].identifier));
			return -1;
		}
		if (ctx->current_autoconfig_info.radio_index != -1) {
			radio_index = ctx->current_autoconfig_info.radio_index;
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"cancel current radio("MACSTR")\n",
				PRINT_MAC(ctx->rinfo[radio_index].identifier));
			ctx->enrolle_state = no_ap_autoconfig;
			ctx->rinfo[radio_id].config_status = MAP_CONF_UNCONF;
		}
	}
	ctx->rinfo[radio_id].trrigerd_autoconf = set_value;
	debug(DEBUG_TRACE, AUTO_CONFIG_PREX"set radio("MACSTR") autoconfig %strrigered\n",
		PRINT_MAC(ctx->rinfo[radio_id].identifier), set_value ? "" : "un");

	return 0;
}

struct agent_list_db *insert_agent_info(struct p1905_managerd_ctx *ctx, unsigned char *almac)
{
	unsigned char is_found = 0;
	struct agent_list_db *agent_info = NULL;

	SLIST_FOREACH(agent_info, &(ctx->agent_head), agent_entry)
	{
		if (!memcmp(agent_info->almac, almac, ETH_ALEN)) {
			is_found = 1;
			break;
		}
	}

	if (is_found) {
		debug(DEBUG_ERROR, "this agent is alraedy insert agent list\n");
		return agent_info;
	}

	agent_info = (struct agent_list_db *)malloc(sizeof(struct agent_list_db));
	if (!agent_info) {
		debug(DEBUG_ERROR, "alloc struct agent_list_db fail\n");
		return NULL;
	}

	debug(DEBUG_TRACE, "insert new agent("MACSTR")\n",
		PRINT_MAC(almac));
	memset(agent_info, 0, sizeof(struct agent_list_db));
	memcpy(agent_info->almac, almac, ETH_ALEN);
	SLIST_INIT(&(agent_info->ch_prefer_head));
	SLIST_INIT(&(agent_info->oper_restrict_head));
	SLIST_INIT(&(agent_info->metrics_rsp_head));
	SLIST_INIT(&(agent_info->tx_metrics_head));
	SLIST_INIT(&(agent_info->rx_metrics_head));
#ifdef MAP_R2
	SLIST_INIT(&agent_info->ts_cap_head);
#endif
	SLIST_INSERT_HEAD(&(ctx->agent_head), agent_info, agent_entry);

	return agent_info;

}


struct agent_radio_info *insert_agent_radio_info(struct agent_list_db *agent, unsigned char *id)
{
	if (agent->radio_num == MAX_RADIO_NUM) {
		debug(DEBUG_ERROR, MACSTR" radio number exceeds maximum number!!!\n", PRINT_MAC(id));
		return NULL;
	} else {
		os_memset(&agent->ra_info[agent->radio_num], 0, sizeof(struct agent_radio_info));
		os_memcpy(agent->ra_info[agent->radio_num].identifier, id, ETH_ALEN);
		agent->radio_num++;
		return &agent->ra_info[agent->radio_num - 1];
	}
}

struct agent_radio_info *find_agent_radio_info(struct agent_list_db *agent, unsigned char *id)
{
	struct agent_radio_info *r = NULL;
	int i = 0;

	for (i = 0; i < agent->radio_num; i++) {
		if (!os_memcmp(agent->ra_info[i].identifier, id, ETH_ALEN)) {
			r = &agent->ra_info[i];
			break;
		}
	}

	return r;
}

int update_agent_radio_info(struct agent_list_db *agent, struct agent_radio_info *r)
{
	struct agent_radio_info *org = NULL;

	org = find_agent_radio_info(agent, r->identifier);
	if (!org)
		org = insert_agent_radio_info(agent, r->identifier);

	if (org) {
		org->band = r->band;
		org->max_bss_num = r->max_bss_num;
		org->op_class_num = r->op_class_num;
	}

	return 0;
}

void find_agent_info(struct p1905_managerd_ctx *ctx,
	unsigned char *almac, struct agent_list_db **agent)
{
	struct agent_list_db *agent_info = NULL;

	SLIST_FOREACH(agent_info, &(ctx->agent_head), agent_entry)
	{
		if (!memcmp(agent_info->almac, almac, ETH_ALEN)) {
			*agent = agent_info;
			break;
		}
	}

}

void set_agent_wsc_doing_radio(struct agent_list_db *agent, unsigned char *id)
{
	unsigned char i = 0;

	for (i = 0; i < agent->radio_num; i++) {
		if (!os_memcmp(agent->ra_info[i].identifier, id, ETH_ALEN))
			agent->ra_info[i].doing_wsc = 1;
		else
			agent->ra_info[i].doing_wsc = 0;
	}
}

struct agent_radio_info *find_agent_wsc_doing_radio(struct agent_list_db *agent)
{
	unsigned char i = 0;

	for (i = 0; i < agent->radio_num; i++) {
		if (agent->ra_info[i].doing_wsc)
			return &agent->ra_info[i];
	}

	return NULL;
}

void clear_agent_all_radio_config_stat(struct agent_list_db *agent)
{
	unsigned char i = 0;

	for (i = 0; i < agent->radio_num; i++)
		agent->ra_info[i].conf_ctx.config_status = 0;
}

void config_agent_all_radio_config_stat(struct agent_list_db *agent)
{
	unsigned char i = 0;

	for (i = 0; i < agent->radio_num; i++)
		agent->ra_info[i].conf_ctx.config_status = 1;
}

void set_agent_radio_config_stat(struct agent_list_db *agent,
	unsigned char *radio_id, unsigned char state)
{
	unsigned char i = 0;

	for (i = 0; i < agent->radio_num; i++) {
		if (!os_memcmp(agent->ra_info[i].identifier, radio_id, ETH_ALEN))
			agent->ra_info[i].conf_ctx.config_status = state;
	}
}

/*
 * get radio config status
 * return:0-not been configured 1-configured
 */
int get_agent_all_radio_config_state(struct agent_list_db *agent)
{
	int i = 0;

	for (i = 0; i < agent->radio_num; i++) {
		if (!agent->ra_info[i].conf_ctx.config_status)
			return 0;
	}

	return 1;
}

int delete_agent_ch_prefer_info(struct p1905_managerd_ctx *ctx,
	struct agent_list_db *agent)
{
	struct ch_prefer_db *chcap = NULL, *chcap_tmp = NULL;
	struct prefer_info_db *prefer = NULL, *prefer_tmp = NULL;
	struct oper_restrict_db *restriction = NULL, *restriction_tmp = NULL;;
	struct restrict_db *resdb = NULL, *resdb_tmp = NULL;
	int i = 0;

	debug(DEBUG_TRACE, "agent("MACSTR")\n", PRINT_MAC(agent->almac));
	debug(DEBUG_TRACE, "########delete channel preference########\n");
	chcap = SLIST_FIRST(&(agent->ch_prefer_head));
	while (chcap != NULL) {
		chcap_tmp = SLIST_NEXT(chcap, ch_prefer_entry);
  	  	prefer = SLIST_FIRST(&(chcap->prefer_info_head));
    	while(prefer) {
    		debug(DEBUG_TRACE, "delete_exist struct prefer_info_db\n");
			debug(DEBUG_TRACE, "opclass=%d, ch_num=%d perference=%d, reason=%d\n",
				prefer->op_class, prefer->ch_num, prefer->perference, prefer->reason);
			debug(DEBUG_TRACE, "ch_list: ");
			for (i = 0; i < prefer->ch_num; i++) {
				debugbyte(DEBUG_TRACE, "%d ", prefer->ch_list[i]);
			}
			debugbyte(DEBUG_TRACE, "\n");
        	prefer_tmp = SLIST_NEXT(prefer, prefer_info_entry);
       	 	free(prefer);
        	prefer = prefer_tmp;
    	}
		debug(DEBUG_TRACE, "delete_exist struct ch_prefer_db\n");
		debug(DEBUG_TRACE, "identifier("MACSTR")\n",
			PRINT_MAC(chcap->identifier));
		debug(DEBUG_TRACE, "tx_power_limit=%d, oper_bss_num:%d\n",
			chcap->tx_power_limit, chcap->op_class_num);
		free(chcap);
		chcap = chcap_tmp;
	}
	SLIST_INIT(&(agent->ch_prefer_head));

	debug(DEBUG_TRACE, "########delete radio operating restriction########\n");
	restriction = SLIST_FIRST(&(agent->oper_restrict_head));
	while (restriction != NULL) {
		restriction_tmp = SLIST_NEXT(restriction, oper_restrict_entry);
  	  	resdb = SLIST_FIRST(&(restriction->restrict_head));
    	while(resdb) {
    		debug(DEBUG_TRACE, "delete_exist struct restrict_db\n");
			debug(DEBUG_TRACE, "opclass=%d ch_num=%d\n",
				resdb->op_class, resdb->ch_num);
			debug(DEBUG_TRACE, "ch_list: ");
			for (i = 0; i < resdb->ch_num; i++) {
				debugbyte(DEBUG_TRACE, "%d ", resdb->ch_list[i]);
			}
			debugbyte(DEBUG_TRACE, "\n");
			debug(DEBUG_TRACE, "min_fre_sep_list: ");
			for (i = 0; i < resdb->ch_num; i++) {
				debugbyte(DEBUG_TRACE, "%d ", resdb->min_fre_sep[i]);
			}
			debugbyte(DEBUG_TRACE, "\n");
        	resdb_tmp = SLIST_NEXT(resdb, restrict_entry);
       	 	free(resdb);
        	resdb = resdb_tmp;
    	}
		debug(DEBUG_TRACE, "delete_exist struct oper_restrict_db\n");
		debug(DEBUG_TRACE, "identifier("MACSTR")\n",
			PRINT_MAC(restriction->identifier));
		debug(DEBUG_TRACE, "oper_bss_num:%d\n", restriction->op_class_num);
		free(restriction);
		restriction = restriction_tmp;
	}
	SLIST_INIT(&(agent->oper_restrict_head));

	return 0;
}

int delete_all_radio_oper_restrict_info(struct list_head_oper_restrict *oper_restrict_head)
{
	struct oper_restrict_db *restriction = NULL, *restriction_tmp = NULL;
	struct restrict_db *resdb = NULL, *resdb_tmp = NULL;
	int i = 0;

	restriction = SLIST_FIRST(oper_restrict_head);
	while (restriction != NULL) {
		restriction_tmp = SLIST_NEXT(restriction, oper_restrict_entry);
		resdb = SLIST_FIRST(&(restriction->restrict_head));
		while(resdb)
		{
			debug(DEBUG_TRACE, "delete_exist struct restrict_db\n");
			debug(DEBUG_TRACE, "opclass=%d ch_num=%d\n", resdb->op_class, resdb->ch_num);
			debug(DEBUG_TRACE, "ch_list: ");
			for (i = 0; i < resdb->ch_num; i++) {
				debugbyte(DEBUG_TRACE, "%d ", resdb->ch_list[i]);
			}
			debugbyte(DEBUG_TRACE, "\n");
			debug(DEBUG_TRACE, "min_fre_sep_list: ");
			for (i = 0; i < resdb->ch_num; i++) {
				debugbyte(DEBUG_TRACE, "%d ", resdb->min_fre_sep[i]);
			}
			debugbyte(DEBUG_TRACE, "\n");
			resdb_tmp = SLIST_NEXT(resdb, restrict_entry);
			free(resdb);
			resdb = resdb_tmp;
		}
		debug(DEBUG_TRACE, "delete_exist struct oper_restrict_db\n");
		debug(DEBUG_TRACE, "identifier("MACSTR")\n",
			PRINT_MAC(restriction->identifier));
		debug(DEBUG_TRACE, "oper_bss_num:%d\n", restriction->op_class_num);
		free(restriction);
		restriction = restriction_tmp;
	}
	SLIST_INIT(oper_restrict_head);

	return wapp_utils_success;
}


int insert_new_radio_oper_restrict(
	struct list_head_oper_restrict *oper_restrict_head, unsigned char *buf, unsigned short len)
{
	struct oper_restrict_db *restriction = NULL;
	struct restrict_db *resdb = NULL;
	 unsigned char *pos = buf;
	 unsigned short check_len = 0, prefer_len = 0;
	 unsigned char i = 0, j = 0, op_class = 0, ch_num = 0;

	if (len < 7) {
		debug(DEBUG_ERROR, "length error less than 7\n");
		return wapp_utils_error;
	}

	restriction = (struct oper_restrict_db *)malloc(sizeof(struct oper_restrict_db));
	if (!restriction) {
		debug(DEBUG_ERROR, "alloc struct oper_restrict_db fail\n");
		return wapp_utils_error;
	}
	memset(restriction, 0, sizeof(struct oper_restrict_db));
	memcpy(restriction->identifier, pos, ETH_ALEN);
	pos += ETH_ALEN;
	check_len += ETH_ALEN;
	restriction->op_class_num = *pos++;
	check_len += 1;
	SLIST_INIT(&(restriction->restrict_head));
	SLIST_INSERT_HEAD(oper_restrict_head, restriction, oper_restrict_entry);

	debug(DEBUG_TRACE, "insert struct oper_restrict_db\n");
	debug(DEBUG_TRACE, "identifier("MACSTR") op_class_num=%d\n",
		PRINT_MAC(restriction->identifier), restriction->op_class_num);

	for(i = 0; i < restriction->op_class_num; i++) {
		op_class = *pos++;
		check_len += 1;
		ch_num = *pos++;
		check_len += 1;
		resdb = (struct restrict_db *)malloc(sizeof(struct restrict_db));
		if (!resdb) {
			debug(DEBUG_ERROR, "alloc struct restrict_db fail\n");
			return wapp_utils_error;
		}
		memset(resdb, 0 , prefer_len);
		resdb->op_class = op_class;
		resdb->ch_num = ch_num;
		for (j = 0; j < resdb->ch_num; j++) {
			resdb->ch_list[j] = *pos++;
			resdb->min_fre_sep[j] = *pos++;
			check_len += 2;
		}
		SLIST_INSERT_HEAD(&restriction->restrict_head, resdb, restrict_entry);
		debug(DEBUG_TRACE, "insert struct restrict_db\n");
		debug(DEBUG_TRACE, "opclass=%d, ch_num=%d\n", resdb->op_class, resdb->ch_num);
		debug(DEBUG_TRACE, "ch_list: ");
		for (j = 0; j < resdb->ch_num; j++) {
			debugbyte(DEBUG_TRACE, "%d ", resdb->ch_list[j]);
		}
		debugbyte(DEBUG_TRACE, "\n");
		debug(DEBUG_TRACE, "min_fre_sep_list: ");
		for (j = 0; j < resdb->ch_num; j++) {
			debugbyte(DEBUG_TRACE, "%d ", resdb->min_fre_sep[j]);
		}
		debugbyte(DEBUG_TRACE, "\n");
	}

	if (len != check_len) {
		debug(DEBUG_ERROR, "length mismatch len(%d) != check_len(%d)\n",
			len, check_len);
		delete_all_radio_oper_restrict_info(oper_restrict_head);
		return wapp_utils_error;
	}

	return wapp_utils_success;

}

int parse_radio_operation_restriction_tlv(unsigned char *buf,
	struct list_head_oper_restrict *oper_restrict_head)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == RADIO_OPERATION_RESTRICTION_TYPE) {
		temp_buf++;
	}
	else {
		return -1;
	}

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	//shift to tlv value field
	temp_buf+=2;

	/*insert new channel preference info*/
	if (0 > insert_new_radio_oper_restrict(oper_restrict_head, temp_buf, length)) {
		debug(DEBUG_ERROR, "insert_new_radio_oper_restrict fail\n");
		return -1;
	}

	return (length+3);
}



int parse_cac_completion_report_type_tlv(struct p1905_managerd_ctx *ctx, unsigned char *buf)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == CAC_COMPLETION_REPORT_TYPE) {
	    debug(DEBUG_WARN, "cac completion report type:%02x \n", *temp_buf);
	    temp_buf++;
	}
	else {
	    return -1;
	}

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	temp_buf += 2;

	debug(DEBUG_WARN, "cac completion report type len:%d \n", length);

	return (length+3);
}

int parse_channel_preference_report_message(struct p1905_managerd_ctx *ctx,
	struct agent_list_db *agent, unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned char *val = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned short len = 0;
	struct list_head_ch_prefer ch_prefer_head = {0};
	struct ch_prefer_db *chcap = NULL;
	struct ch_prefer_db *chcap_tmp = NULL;

	type = buf;
	reset_stored_tlv(ctx);
	SLIST_INIT(&ch_prefer_head);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			goto fail;
		}
		val = type + 3;

		if (*type == CH_PREFERENCE_TYPE) {
			if (ctx->MAP_Cer) {
				ret = insert_new_ch_prefer_info(&ch_prefer_head, len, val);
				if (ret < 0) {
					delete_all_ch_prefer_info((struct list_head_ch_prefer *)&(agent->ch_prefer_head));
					debug(DEBUG_ERROR, "error CH PREFERENCE tlv\n");
					goto fail;
				}
			}
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == RADIO_OPERATION_RESTRICTION_TYPE) {
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == CAC_COMPLETION_REPORT_TYPE) {
			store_revd_tlvs(ctx, type, tlv_len);
		}
#ifdef MAP_R2
		else if (*type == CAC_STATUS_REPORT_TYPE) {
			store_revd_tlvs(ctx, type, tlv_len);
		}
#endif
		else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	update_ch_prefer_list_info((struct list_head_ch_prefer *)&(agent->ch_prefer_head), &ch_prefer_head);
	return 0;

fail:
	if (!SLIST_EMPTY(&ch_prefer_head)) {
		chcap = SLIST_FIRST(&ch_prefer_head);
		while (chcap != NULL) {
			chcap_tmp = SLIST_NEXT(chcap, ch_prefer_entry);
			free(chcap);
			chcap = chcap_tmp;
		}
	}
	return -1;
}

int delete_agent_ap_metrics_info(struct list_head_metrics_rsp_agent *metrics_head)
{
	 struct mrsp_db *mrsp = NULL, *mrsp_tmp = NULL;
	 struct esp_db *esp = NULL, *esp_tmp = NULL;

	mrsp = SLIST_FIRST(metrics_head);
	while (mrsp != NULL) {
		mrsp_tmp = SLIST_NEXT(mrsp, mrsp_entry);
        esp = SLIST_FIRST(&mrsp->esp_head);
        while(esp) {
        	debug(DEBUG_TRACE, "delete_exist struct esp_db\n");
			debug(DEBUG_TRACE, "ac=%d, format=%d ba_win_size=%d, e_air_time_fraction=%d, ppdu_dur_target=%d\n",
				esp->ac, esp->format, esp->ba_win_size, esp->e_air_time_fraction, esp->ppdu_dur_target);
            esp_tmp = SLIST_NEXT(esp, esp_entry);
            free(esp);
            esp = esp_tmp;
        }
		debug(DEBUG_TRACE, "delete_exist struct mrsp_db\n");
		debug(DEBUG_TRACE, "bssid("MACSTR")\n", PRINT_MAC(mrsp->bssid));
		debug(DEBUG_TRACE, "ch_util=%d, assoc_sta_cnt:%d\n",
			mrsp->ch_util, mrsp->assoc_sta_cnt);
		free(mrsp);
		mrsp = mrsp_tmp;
    }
	SLIST_INIT(metrics_head);

	return wapp_utils_success;
}


int insert_new_ap_metrics_info(
	struct list_head_metrics_rsp_agent *metrics_head, unsigned char *buf, unsigned short len)
{
	 unsigned char *pos = buf;
	 unsigned short check_len = 0;
	 struct mrsp_db *mrsp = NULL;
	 struct esp_db *esp = NULL;

	if (len != 13 && len != 22 && len != 16 && len != 19) {
		debug(DEBUG_ERROR, "length error(%d)\n", len);
		return wapp_utils_error;
	}

    SLIST_FOREACH(mrsp, metrics_head, mrsp_entry)
    {
        if(!memcmp(mrsp->bssid, buf, ETH_ALEN)) {
			debug(DEBUG_TRACE, "bss("MACSTR") found\n", PRINT_MAC(mrsp->bssid));
            break;
        }
    }

	if (!mrsp) {
		mrsp = (struct mrsp_db *)malloc(sizeof(*mrsp));
		if (!mrsp) {
			debug(DEBUG_ERROR, "alloc struct mrsp_db fail\n");
			return wapp_utils_error;
		}
		memset(mrsp, 0, sizeof(*mrsp));
		memcpy(mrsp->bssid, pos, ETH_ALEN);
		pos += ETH_ALEN;
		check_len += ETH_ALEN;
		mrsp->ch_util = *pos++;
		check_len += 1;
		mrsp->assoc_sta_cnt = *(unsigned short *)pos;
		mrsp->assoc_sta_cnt = be_to_host16(mrsp->assoc_sta_cnt);
		pos += 2;
		check_len += 2;
		/*skip esp indicator*/
		pos += 1;
		check_len += 1;
		SLIST_INIT(&mrsp->esp_head);
		SLIST_INSERT_HEAD(metrics_head, mrsp, mrsp_entry);

		debug(DEBUG_TRACE, "insert struct mrsp_db\n");
		debug(DEBUG_TRACE, "bssid("MACSTR") ch_util=%d assoc_sta_cnt=%d\n",
			PRINT_MAC(mrsp->bssid), mrsp->ch_util, mrsp->assoc_sta_cnt);
		while(check_len < len) {
			esp = (struct esp_db *)malloc(sizeof(struct esp_db));
			if (!esp) {
				debug(DEBUG_ERROR, "alloc struct esp_db fail\n");
				return wapp_utils_error;
			}
			memset(esp, 0 , sizeof(struct esp_db));
			esp->ac = (*pos) & 0x03;
			esp->format = (*pos) & 0x18;
			esp->ba_win_size = (*pos) & 0xe0;
			esp->e_air_time_fraction = *(pos + 1);
			esp->ppdu_dur_target = *(pos + 2);
			SLIST_INSERT_HEAD(&mrsp->esp_head, esp, esp_entry);
			pos += 3;
			check_len += 3;
			debug(DEBUG_TRACE, "insert struct esp_db\n");
			debug(DEBUG_TRACE, "ac=%d, format=%d ba_win_size=%d, e_air_time_fraction=%d, ",
				esp->ac, esp->format, esp->ba_win_size, esp->e_air_time_fraction);
			debug(DEBUG_TRACE, "ppdu_dur_target=%d\n", esp->ppdu_dur_target);
		}
	}else {
		debug(DEBUG_ERROR, "bssid exist\n");
		return wapp_utils_error;
	}

	return wapp_utils_success;

}

int parse_ap_metrics_tlv(struct p1905_managerd_ctx* ctx, unsigned char *buf,
	struct list_head_metrics_rsp_agent *metrics_head)
{
    unsigned char *temp_buf;
    unsigned short length = 0;

    temp_buf = buf;

    if((*temp_buf) == AP_METRICS_TYPE)
        temp_buf++;
    else
        return -1;

    //calculate tlv length
    length = *(unsigned short *)temp_buf;
    length = be_to_host16(length);

    return (length+3);
}

int parse_assoc_sta_traffic_stats_tlv(struct p1905_managerd_ctx* ctx, unsigned char *buf)
{
    unsigned char *temp_buf;
    unsigned short length = 0;

    temp_buf = buf;

    if((*temp_buf) == ASSOC_STA_TRAFFIC_STATS_TYPE) {
        temp_buf++;
    } else {
        return -1;
    }

    //calculate tlv length
    length = *(unsigned short *)temp_buf;
    length = be_to_host16(length);

	temp_buf += ETH_ALEN + 2;
	*((unsigned int*)temp_buf) = be_to_host32(*((unsigned int*)temp_buf));
	temp_buf += sizeof(unsigned int);
	*((unsigned int*)temp_buf) = be_to_host32(*((unsigned int*)temp_buf));
	temp_buf += sizeof(unsigned int);
	*((unsigned int*)temp_buf) = be_to_host32(*((unsigned int*)temp_buf));
	temp_buf += sizeof(unsigned int);
	*((unsigned int*)temp_buf) = be_to_host32(*((unsigned int*)temp_buf));
	temp_buf += sizeof(unsigned int);
	*((unsigned int*)temp_buf) = be_to_host32(*((unsigned int*)temp_buf));
	temp_buf += sizeof(unsigned int);
	*((unsigned int*)temp_buf) = be_to_host32(*((unsigned int*)temp_buf));
	temp_buf += sizeof(unsigned int);
	*((unsigned int*)temp_buf) = be_to_host32(*((unsigned int*)temp_buf));
	temp_buf += sizeof(unsigned int);
    return (length+3);
}

int parse_assoc_sta_link_metrics_tlv(struct p1905_managerd_ctx* ctx, unsigned char *buf)
{
    unsigned char *temp_buf;
    unsigned short length = 0;

    temp_buf = buf;

    if((*temp_buf) == ASSOC_STA_LINK_METRICS_TYPE) {
        temp_buf++;
    } else {
        return -1;
    }

    //calculate tlv length
    length = *(unsigned short *)temp_buf;
    length = be_to_host16(length);

    return (length+3);
}

int parse_ap_metrics_response_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;
	unsigned int all_integrity = 0xff;

	type = buf;
	reset_stored_tlv(ctx);

	while(1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		/* One or more AP mMetrics TLVs */
		if (*type == AP_METRICS_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* Zero or more Associated STA Traffic Stats TLVs */
		else if (*type == ASSOC_STA_TRAFFIC_STATS_TYPE) {
			integrity |= (1 << 1);
			parse_assoc_sta_traffic_stats_tlv(ctx, type);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* Zero or more Associated STA Link Metrics TLVs */
		else if (*type == ASSOC_STA_LINK_METRICS_TYPE) {
			integrity |= (1 << 2);
			store_revd_tlvs(ctx, type, tlv_len);
		}
#ifdef MAP_R2
		/* Zero or more Radio Metrics TLVs */
		else if (*type == RADIO_METRIC_TYPE) {
			integrity |= (1 << 3);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* One or more AP Extended Metrics TLVs */
		else if (*type == AP_EXTENDED_METRIC_TYPE) {
			integrity |= (1 << 4);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* Zero or more Associated STA Extended Link Metrics TLVs */
		else if (*type == ASSOCIATED_STA_EXTENDED_LINK_METRIC_TYPE) {
			integrity |= (1 << 5);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* Zero or more Associated STA Extended Link Metrics TLVs */
		else if (*type == VENDOR_SPECIFIC_TLV_TYPE) {
			integrity |= (1 << 6);
			store_revd_tlvs(ctx, type, tlv_len);
		}
#endif
		else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if ((integrity & all_integrity) == 0) {
		debug(DEBUG_ERROR, "incomplete ap metrics response message 0x%x 0x%x\n",
			integrity, all_integrity);
		return -1;
	}

	return 0;
}

int delete_agent_tx_link_metrics_info(struct list_head_tx_metrics_agent *tx_metrics_head)
{
	 struct tx_link_metric_db *tx_link_metric = NULL, *tx_link_metric_tmp = NULL;
	 struct tx_metric_db *tx_metric = NULL, *tx_metric_tmp = NULL;

	tx_link_metric = SLIST_FIRST(tx_metrics_head);
	while (tx_link_metric != NULL) {
		tx_link_metric_tmp = SLIST_NEXT(tx_link_metric, tx_link_metric_entry);
        tx_metric = SLIST_FIRST(&tx_link_metric->tx_metric_head);
        while(tx_metric) {
        	debug(DEBUG_TRACE, "delete_exist struct tx_metric_db\n");
			debug(DEBUG_TRACE, "own inf("MACSTR")\n",
				PRINT_MAC(tx_metric->mac));
			debug(DEBUG_TRACE, "neighbor connect inf("MACSTR")\n",
				PRINT_MAC(tx_metric->nmac));
			debug(DEBUG_TRACE, "intf_type=%d, bridge_flag=%d, error_packet=%d, tx_packets=%d, ",
				tx_metric->intf_type, tx_metric->bridge_flag, tx_metric->error_packet,
				tx_metric->tx_packets);
			debug(DEBUG_TRACE, "mac_tpcap=%d, linkavl=%d phyrate=%d\n",
				tx_metric->mac_tpcap, tx_metric->linkavl, tx_metric->phyrate);
            tx_metric_tmp = SLIST_NEXT(tx_metric, tx_metric_entry);
            free(tx_metric);
            tx_metric = tx_metric_tmp;
        }
		debug(DEBUG_TRACE, "delete_exist struct tx_link_metric_db\n");
		debug(DEBUG_TRACE, "own almac("MACSTR")\n",
			PRINT_MAC(tx_link_metric->almac));
		debug(DEBUG_TRACE, "neighbor almac("MACSTR")\n",
			PRINT_MAC(tx_link_metric->nalmac));
		tx_link_metric = tx_link_metric_tmp;
    }
	SLIST_INIT(tx_metrics_head);

	return wapp_utils_success;
}

int delete_agent_rx_link_metrics_info(struct list_head_rx_metrics_agent *rx_metrics_head)
{
	 struct rx_link_metric_db *rx_link_metric = NULL, *rx_link_metric_tmp = NULL;
	 struct rx_metric_db *rx_metric = NULL, *rx_metric_tmp = NULL;

	rx_link_metric = SLIST_FIRST(rx_metrics_head);
	while (rx_link_metric != NULL) {
		rx_link_metric_tmp = SLIST_NEXT(rx_link_metric, rx_link_metric_entry);
        rx_metric = SLIST_FIRST(&rx_link_metric->rx_metric_head);
        while(rx_metric) {
        	debug(DEBUG_TRACE, "delete_exist struct rx_metric_db\n");
			debug(DEBUG_TRACE, "own inf("MACSTR")\n",
				PRINT_MAC(rx_metric->mac));
			debug(DEBUG_TRACE, "neighbor connect inf("MACSTR")\n",
				PRINT_MAC(rx_metric->nmac));
			debug(DEBUG_TRACE, "intf_type=%d, error_packet=%d, rx_packets=%d, rssi=%d\n",
				rx_metric->intf_type, rx_metric->error_packet, rx_metric->rx_packets,
				rx_metric->rssi);
            rx_metric_tmp = SLIST_NEXT(rx_metric, rx_metric_entry);
            free(rx_metric);
            rx_metric = rx_metric_tmp;
        }
		debug(DEBUG_TRACE, "delete_exist struct rx_link_metric_db\n");
		debug(DEBUG_TRACE, "own almac("MACSTR")\n",
			PRINT_MAC(rx_link_metric->almac));
		debug(DEBUG_TRACE, "neighbor almac("MACSTR")\n",
			PRINT_MAC(rx_link_metric->nalmac));
		rx_link_metric = rx_link_metric_tmp;
    }
	SLIST_INIT(rx_metrics_head);

	return wapp_utils_success;
}
int insert_new_tx_link_metrics_info(
	struct list_head_tx_metrics_agent *tx_metrics_head, unsigned char *buf, unsigned short len)
{
	 unsigned char *pos = buf;
	 unsigned short check_len = 0;
	 struct tx_link_metric_db *tx_link_metric = NULL;
	 struct tx_metric_db *tx_metric = NULL;
	 unsigned char *neighbor_dev = buf + 6;

    SLIST_FOREACH(tx_link_metric, tx_metrics_head, tx_link_metric_entry)
    {
        if(!memcmp(tx_link_metric->nalmac, neighbor_dev, ETH_ALEN)) {
			debug(DEBUG_TRACE, "neighbor dev("MACSTR") found\n",
				PRINT_MAC(neighbor_dev));
            break;
        }
    }

	if (!tx_link_metric) {
		tx_link_metric = (struct tx_link_metric_db *)malloc(sizeof(*tx_link_metric));
		if (!tx_link_metric) {
			debug(DEBUG_ERROR, "alloc struct tx_link_metric_db fail\n");
			return wapp_utils_error;
		}
		memset(tx_link_metric, 0, sizeof(*tx_link_metric));
		memcpy(tx_link_metric->almac, pos, ETH_ALEN);
		pos += ETH_ALEN;
		check_len += ETH_ALEN;
		memcpy(tx_link_metric->nalmac, pos, ETH_ALEN);
		pos += ETH_ALEN;
		check_len += ETH_ALEN;
		SLIST_INIT(&tx_link_metric->tx_metric_head);
		SLIST_INSERT_HEAD(tx_metrics_head, tx_link_metric, tx_link_metric_entry);

		debug(DEBUG_TRACE, "insert struct tx_link_metric_db\n");
		debug(DEBUG_TRACE, "almac("MACSTR")\n",
			PRINT_MAC(tx_link_metric->almac));
		debug(DEBUG_TRACE, "nalmac("MACSTR")\n",
			PRINT_MAC(tx_link_metric->nalmac));
		while(check_len < len) {
			tx_metric = (struct tx_metric_db *)malloc(sizeof(struct tx_metric_db));
			if (!tx_metric) {
				debug(DEBUG_ERROR, "alloc struct tx_metric_db fail\n");
				return wapp_utils_error;
			}
			memcpy(tx_metric->mac, pos, ETH_ALEN);
			pos += ETH_ALEN;
			check_len += ETH_ALEN;
			memcpy(tx_metric->nmac, pos, ETH_ALEN);
			pos += ETH_ALEN;
			check_len += ETH_ALEN;
			tx_metric->intf_type = (*(unsigned short *)pos);
			tx_metric->intf_type = be_to_host16(tx_metric->intf_type);
			pos += 2;
			check_len += 2;
			tx_metric->bridge_flag = *pos;
			pos += 1;
			check_len += 1;
			tx_metric->error_packet = (*(unsigned int *)pos);
			tx_metric->error_packet = be_to_host32(tx_metric->error_packet);
			pos += 4;
			check_len += 4;
			tx_metric->tx_packets = (*(unsigned int *)pos);
			tx_metric->tx_packets = be_to_host32(tx_metric->tx_packets);
			pos += 4;
			check_len += 4;
			tx_metric->mac_tpcap = (*(unsigned short *)pos);
			tx_metric->mac_tpcap = be_to_host16(tx_metric->mac_tpcap);
			pos += 2;
			check_len += 2;
			tx_metric->linkavl = (*(unsigned short *)pos);
			tx_metric->linkavl = be_to_host16(tx_metric->linkavl);
			pos += 2;
			check_len += 2;
			tx_metric->phyrate = (*(unsigned short *)pos);
			tx_metric->phyrate = be_to_host16(tx_metric->phyrate);
			pos += 2;
			check_len += 2;
			SLIST_INSERT_HEAD(&tx_link_metric->tx_metric_head, tx_metric, tx_metric_entry);

			debug(DEBUG_TRACE, "insert struct tx_metric_db\n");
			debug(DEBUG_TRACE, "mac("MACSTR")\n",
				PRINT_MAC(tx_metric->mac));
			debug(DEBUG_TRACE, "connect mac("MACSTR")\n",
				PRINT_MAC(tx_metric->nmac));
			debug(DEBUG_TRACE, "intf_type=%d, bridge_flag=%d error_packet=%d, ",
				tx_metric->intf_type, tx_metric->bridge_flag, tx_metric->error_packet);
			debug(DEBUG_TRACE, "tx_packets=%d, mac_tpcap=%d linkavl=%d, phyrate=%d\n",
				tx_metric->tx_packets, tx_metric->mac_tpcap, tx_metric->linkavl, tx_metric->phyrate);
		}
	}else {
		debug(DEBUG_ERROR, "tx_link_metric exist\n");
		return wapp_utils_error;
	}

	return wapp_utils_success;

}


int parse_tx_link_metrics_tlv(struct p1905_managerd_ctx* ctx, unsigned char *buf,
	struct list_head_tx_metrics_agent *tx_metrics_head)
{
    unsigned char *temp_buf;
    unsigned short length = 0;

    temp_buf = buf;

    if((*temp_buf) == TRANSMITTER_LINK_METRIC_TYPE)
        temp_buf++;
    else
        return -1;

    //calculate tlv length
    length = *(unsigned short *)temp_buf;
    length = be_to_host16(length);

    return (length+3);
}

int parse_rx_link_metrics_tlv(struct p1905_managerd_ctx* ctx, unsigned char *buf,
	struct list_head_rx_metrics_agent *rx_metrics_head)
{
    unsigned char *temp_buf;
    unsigned short length = 0;

    temp_buf = buf;

    if((*temp_buf) == RECEIVER_LINK_METRIC_TYPE)
        temp_buf++;
    else
        return -1;

    //calculate tlv length
    length = *(unsigned short *)temp_buf;
    length = be_to_host16(length);

    return (length+3);
}

int parse_link_metrics_response_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while(1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == TRANSMITTER_LINK_METRIC_TYPE)
			store_revd_tlvs(ctx, type, tlv_len);
		else if (*type == RECEIVER_LINK_METRIC_TYPE)
			store_revd_tlvs(ctx, type, tlv_len);
		else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	return 0;
}

unsigned short append_tx_link_metric_tlv(
        unsigned char *pkt, struct tx_link_metric_db *tx_link_metric)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;
	struct tx_metric_db *tx_metric = NULL;

    temp_buf = pkt;

    *temp_buf = TRANSMITTER_LINK_METRIC_TYPE;
    temp_buf +=1;
    total_length += 1;

    /* The length of tx link metric tlv is a variable.
     * It depends on connetced interface number, shift to payload first
     */
    temp_buf +=2;
    total_length += 2;

    /*fill into local abstration layer mac addr*/
    memcpy(temp_buf, tx_link_metric->almac, ETH_ALEN);
    temp_buf += ETH_ALEN;
    total_length += ETH_ALEN;

    /*fill into neighbor abstration layer mac addr*/
    memcpy(temp_buf, tx_link_metric->nalmac, ETH_ALEN);
    temp_buf += ETH_ALEN;
    total_length += ETH_ALEN;

    SLIST_FOREACH(tx_metric, &tx_link_metric->tx_metric_head, tx_metric_entry)
    {
		/*fill into local interface mac addr*/
		memcpy(temp_buf, tx_metric->mac, ETH_ALEN);
		temp_buf += ETH_ALEN;
		total_length += ETH_ALEN;
		/*fill into neighbor interface mac addr*/
		memcpy(temp_buf, tx_metric->nmac, ETH_ALEN);
		temp_buf += ETH_ALEN;
		total_length += ETH_ALEN;
		/*fill into interface media type*/
		*(unsigned short *)(temp_buf) = host_to_be16(tx_metric->intf_type);
		temp_buf += 2;
		total_length += 2;
		/*fill into bridge flag*/
		*temp_buf = tx_metric->bridge_flag;
		temp_buf += 1;
		total_length += 1;
		/*fill into tx packets error*/
		*(unsigned int *)(temp_buf) = host_to_be32(tx_metric->error_packet);
		temp_buf += 4;
		total_length += 4;
		/*fill into total transmitted packets*/
		*(unsigned int *)(temp_buf) = host_to_be32(tx_metric->tx_packets);
		temp_buf += 4;
		total_length += 4;
		/*fill into max throughput capability*/
		*(unsigned short *)(temp_buf) = host_to_be16(tx_metric->mac_tpcap);
		temp_buf += 2;
		total_length += 2;
		/*fill into link availability field */
		*(unsigned short *)(temp_buf) = host_to_be16(tx_metric->linkavl);
		temp_buf += 2;
		total_length += 2;
		/* fill into phy rate*/
		*(unsigned short *)(temp_buf) = host_to_be16(tx_metric->phyrate);
		temp_buf += 2;
		total_length += 2;
    }

    /*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

    return total_length;
}

unsigned short append_rx_link_metric_tlv(
        unsigned char *pkt, struct rx_link_metric_db *rx_link_metric)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;
	struct rx_metric_db *rx_metric = NULL;

    temp_buf = pkt;

    *temp_buf = RECEIVER_LINK_METRIC_TYPE;
    temp_buf +=1;
    total_length += 1;

    /* The length of rx link metric tlv is a variable.
     * It depends on connetced interface number, shift to payload first
     */
    temp_buf +=2;
    total_length += 2;

    /*fill into local abstration layer mac addr*/
    memcpy(temp_buf, rx_link_metric->almac, ETH_ALEN);
    temp_buf += ETH_ALEN;
    total_length += ETH_ALEN;

    /*fill into neighbor abstration layer mac addr*/
    memcpy(temp_buf, rx_link_metric->nalmac, ETH_ALEN);
    temp_buf += ETH_ALEN;
    total_length += ETH_ALEN;

    SLIST_FOREACH(rx_metric, &rx_link_metric->rx_metric_head, rx_metric_entry)
    {
		/*fill into local interface mac addr*/
		memcpy(temp_buf, rx_metric->mac, ETH_ALEN);
		temp_buf += ETH_ALEN;
		total_length += ETH_ALEN;
		/*fill into neighbor interface mac addr*/
		memcpy(temp_buf, rx_metric->nmac, ETH_ALEN);
		temp_buf += ETH_ALEN;
		total_length += ETH_ALEN;
		/*fill into interface media type*/
		*(unsigned short *)(temp_buf) = host_to_be16(rx_metric->intf_type);
		temp_buf += 2;
		total_length += 2;
		/*fill into rx packets error*/
		*(unsigned int *)(temp_buf) = host_to_be32(rx_metric->error_packet);
		temp_buf += 4;
		total_length += 4;
		/*fill into total received packets*/
		*(unsigned int *)(temp_buf) = host_to_be32(rx_metric->rx_packets);
		temp_buf += 4;
		total_length += 4;
		/* fill rssi*/
		*temp_buf = rx_metric->rssi;
		temp_buf += 1;
		total_length += 1;
    }

    /*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

    return total_length;
}

int free_all_the_agents_info(struct list_head_agent *agent_head)
{
	struct agent_list_db *agent = SLIST_FIRST(agent_head);
	struct agent_list_db *agent_tmp = NULL;

	while(agent != NULL) {
		agent_tmp = SLIST_NEXT(agent, agent_entry);
		debug(DEBUG_TRACE, "free agent_list_db(almac="MACSTR"))\n",
			PRINT_MAC(agent->almac));

		delete_all_ch_prefer_info((struct list_head_ch_prefer *)&agent->ch_prefer_head);
		delete_all_radio_oper_restrict_info((struct list_head_oper_restrict *)&agent->oper_restrict_head);
		delete_agent_ap_metrics_info(&agent->metrics_rsp_head);
		delete_agent_tx_link_metrics_info(&agent->tx_metrics_head);
		delete_agent_rx_link_metrics_info(&agent->rx_metrics_head);
		os_free(agent);
		agent = agent_tmp;
	}

	return 0;
}
extern unsigned char p1905_multicast_address[6];
extern unsigned char dev_send_1905_buf[3072*2];

void show_1905_mapfilter_version(struct p1905_managerd_ctx* ctx)
{
	debug(DEBUG_ERROR, "Currrent 1905 verion %s\n", VERSION_1905);
	mapfilter_show_version();
}


void send_1905_raw_data(struct p1905_managerd_ctx* ctx, char *file)
{
	struct _1905_cmdu_request *request = (struct _1905_cmdu_request*)dev_send_1905_buf;

	if(_1905_read_dev_send_1905(ctx, file, request->dest_al_mac, &request->type,
			&request->len, request->body) < 0) {
		debug(DEBUG_ERROR, "parse raw data from %s fail\n", file);
		return;
	}

	dev_send_1905(ctx, request);
	process_cmdu_txq(ctx, ctx->trx_buf.buf);
}

void read_1905_bss_config(struct p1905_managerd_ctx* ctx, char *file)
{
	if (_1905_read_set_config(ctx, file, ctx->bss_config, MAX_SET_BSS_INFO_NUM, &ctx->bss_config_num) < 0)
		debug(DEBUG_ERROR, "load bss config fail from %s\n", file);

#ifdef MAP_R3
	if (ctx->map_version == MAP_PROFILE_R3 && ctx->role == CONTROLLER) {
		struct r3_member *peer = NULL;
		dl_list_for_each(peer, &r3_member_head, struct r3_member, entry) {
			if (peer->security_1905) {
				debug(DEBUG_ERROR, "send bss reconfig trigger to "MACSTR"\n", MAC2STR(peer->al_mac));
				insert_cmdu_txq(peer->al_mac, ctx->p1905_al_mac_addr,
					e_bss_reconfiguration_trigger, ++ctx->mid, ctx->br_name, 0);
			}
		}
	}
#endif
}

#ifdef SUPPORT_CMDU_RELIABLE
void cmdu_reliable_send(struct p1905_managerd_ctx *ctx, unsigned short msg_type,
		unsigned short mid, unsigned int ifidx)
{
	struct topology_response_db *tpr_db = NULL;
	unsigned char *if_name = NULL;

	SLIST_FOREACH(tpr_db, &ctx->topology_entry.tprdb_head, tprdb_entry)
	{
		/*relay this unicast message to all devices in the same topology*/
		if (tpr_db->recv_ifid == ifidx) {
			if_name = ctx->itf[ifidx].if_name;
			insert_cmdu_txq(tpr_db->al_mac_addr, ctx->p1905_al_mac_addr,
					msg_type, mid, if_name, 0);
			debug(DEBUG_ERROR, TOPO_PREX"tx unicast %04x(mid-%04x) on intf(%s) to "MACSTR"\n",
				msg_type, mid, if_name, PRINT_MAC(tpr_db->al_mac_addr));
		}
	}
}
#endif

int map_event_handler(struct p1905_managerd_ctx *ctx, char *buf, int len, unsigned char type,
	unsigned char *reply, int *reply_len)
{
	struct msg *wapp_event = NULL;
	int if_num = 0;
	unsigned short mid = 0;

	wapp_event = (struct msg *)buf;

	if ((type != 0x00) && (wapp_event->type == type)) {
		if (wapp_event->length == 0) {
			debug(DEBUG_ERROR, "waittype(0x%04x) == wapp_event->type=(0x%04x); data is invalid\n",
				 	type, wapp_event->type);
			return -2;
		}
	}

	if (wapp_event->length == 0) {
		if (wapp_event->type != LIB_1905_CMDU_REQUEST &&
			wapp_event->type != LIB_STEERING_COMPLETED &&
			wapp_event->type != WAPP_GET_OWN_TOPOLOGY_RSP &&
			wapp_event->type != LIB_CLEAR_SWITCH_TABLE
			) {
			 debug(DEBUG_ERROR, "invalid data from wapp! type=0x%04x\n", wapp_event->type);
			return -1;
		}
	}

	if((type != 0x00) && wapp_event->type != type)
	{
		debug(DEBUG_TRACE, "waittype(0x%04x) != wapp_event->type=(0x%04x)\n", type, wapp_event->type);
		/*avoid to drop unsolicited message, insert to a waitq*/
		insert_message_wait_queue(wapp_event->type, (unsigned char *)buf, len);
		return -1;
	}

	debug(DEBUG_TRACE, "receive event(0x%04x)\n", wapp_event->type);

	switch (wapp_event->type) {
	case LIB_WIRELESS_INTERFACE_INFO:
	{
		struct interface_info_list_hdr *info = NULL;
		unsigned short real_len = 0;

		info = (struct interface_info_list_hdr *)wapp_event->buffer;
		real_len = sizeof(struct interface_info_list_hdr) + \
			info->interface_count * sizeof(struct interface_info);
		/*event length check*/
		if (wapp_event->length != real_len) {
			debug(DEBUG_ERROR, "length check fail for LIB_WIRELESS_INTERFACE_INFO! "
				"recv length(%u) real length(%u)\n", wapp_event->length, real_len);
			break;
		}
		init_wireless_interface(ctx, info);
		set_bss_config_priority_by_str(ctx, ctx->bss_priority_str);
		common_info_init(ctx);
#ifdef MAP_R3
		eloop_register_timeout(2, 0, report_akm_suit_cap_to_mapd, (void *)ctx, NULL);
		eloop_register_timeout(2, 0, report_1905_secure_cap_to_mapd, (void *)ctx, NULL);
		eloop_register_timeout(2, 0, report_sp_standard_rule_to_mapd_timeout, (void *)ctx, NULL);
#endif
	}
		break;
	case LIB_CLEAR_SWITCH_TABLE:
	{
		eth_layer_clear_switch_table();
	}
		break;
	case LIB_MAP_BH_READY:
	{
		struct bh_link_info *info = NULL;
		int i = 0;
		unsigned char notify = 0;
		int ret = 0, ret1 = 0;
		unsigned char almac[ETH_ALEN] = {0};

		info = (struct bh_link_info *)wapp_event->buffer;

		if (ctx->role == CONTROLLER) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"[LIB_MAP_BH_READY]error! controller recv link up event\n");
			break;
		}
#ifdef MAP_R3
		/* avoid r3 onboarding sm abnormality */
		os_memset(&ctx->r3_oboard_ctx, 0, sizeof(ctx->r3_oboard_ctx));
		if (info->type)
			ctx->r3_oboard_ctx.bh_type = MAP_BH_WIFI;
		else
			ctx->r3_oboard_ctx.bh_type = MAP_BH_ETH;

		/* cancel timer to stop sending autoconfig search periodicly */
		eloop_cancel_timeout(periodic_send_autoconfig_search, (void *)ctx, NULL);
#endif

		if (info->type == MAP_BH_ETH) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"ETH link up\n");
			ctx->controller_search_state = bk_link_ready;
		    ctx->controller_search_cnt = 0;
#ifdef MAP_R3
			int j = 0;
            for (j = 1; j < ctx->itf_number; j++) {
                /*send multicast cmdu to all the ethenet interface */
                if (ctx->itf[j].media_type == IEEE802_3_GROUP) {
                    for(i = 0; i < 3; i++) {
                        ctx->mid++;
                        insert_cmdu_txq(p1905_multicast_address, ctx->p1905_al_mac_addr,
                            e_topology_discovery_with_vendor_ie, ctx->mid, ctx->itf[j].if_name, 0);
                    }
                }
            }
#endif
			eloop_cancel_timeout(ap_controller_search_step, (void *)ctx, NULL);
			eloop_register_timeout(0, 0, ap_controller_search_step, (void *)ctx, NULL);
			break;
		}

		for (i = 0; i < ctx->itf_number; i++) {
			if (!os_memcmp(info->mac_addr, ctx->itf[i].mac_addr, ETH_ALEN)) {
				if (ctx->itf[i].is_wifi_sta != 1) {
					debug(DEBUG_ERROR, AUTO_CONFIG_PREX"[LIB_MAP_BH_READY]error! not apcli interface("MACSTR")\n",
						PRINT_MAC(info->mac_addr));
				}
				break;
			}
		}
		if (i >= ctx->itf_number) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"[LIB_MAP_BH_READY]can't find apcli interface("MACSTR")\n",
				PRINT_MAC(info->mac_addr));
			break;
		}

		debug(DEBUG_OFF, AUTO_CONFIG_PREX"[LIB_MAP_BH_READY] apcli(%s-"MACSTR") connect to ("MACSTR")\n",
			ctx->itf[i].if_name, PRINT_MAC(info->mac_addr), PRINT_MAC(info->bssid));
		eloop_register_timeout(0, 0, discovery_at_interface_link_up,
			(void *)ctx, (void *)ctx->itf[i].if_name);
		memcpy(ctx->itf[i].vs_info, info->bssid, ETH_ALEN);
		ctx->itf[i].trx_config = TX_MUL | RX_MUL | TX_UNI | RX_UNI;

		/*check if bssid in this event is operating on one of neighbor MAP device*/
		ret = find_almac_by_connect_mac(ctx, info->bssid, almac);
		if (ret == 1) {
			debug(DEBUG_ERROR, TOPO_PREX"[LIB_MAP_BH_READY]already receive discovery from neighbor("MACSTR") peer itf("MACSTR")\n",
					PRINT_MAC(almac), PRINT_MAC(info->bssid));
		} else if (ret == 2) {
			debug(DEBUG_ERROR, TOPO_PREX"[LIB_MAP_BH_READY]no discovery!!! recv response from neighbor("MACSTR") peer itf("MACSTR")\n",
					PRINT_MAC(almac), PRINT_MAC(info->bssid));
			debug(DEBUG_ERROR, TOPO_PREX"[LIB_MAP_BH_READY]add neighbor info and discovery to db");
			ret1 = insert_topology_discovery_database(ctx, almac,
					info->bssid, info->mac_addr);
			if (ret1 >= 0) {
				update_p1905_neighbor_dev_info(ctx, almac, info->bssid,
					info->mac_addr, &notify);
				/*report updated own topology response to mapd*/
				report_own_topology_rsp(ctx, ctx->p1905_al_mac_addr, ctx->br_cap,
					ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
					ctx->service, &ctx->ap_cap_entry.oper_bss_head,
					&ctx->ap_cap_entry.assoc_clients_head, ctx->cnt);
				update_topology_tree(ctx);
				/*send notification to notify other device my topology is changed*/
				ctx->mid++;
				debug(DEBUG_ERROR, TOPO_PREX"[LIB_MAP_BH_READY]tx multicast notification(mid-%04x) on all intf\n",
						ctx->mid);
				for(if_num = 0; if_num < ctx->itf_number; if_num++) {
					insert_cmdu_txq((unsigned char*)p1905_multicast_address,
						(unsigned char*)ctx->p1905_al_mac_addr,
						e_topology_notification, ctx->mid,
						ctx->itf[if_num].if_name, 0);
#ifdef SUPPORT_CMDU_RELIABLE
					cmdu_reliable_send(ctx, e_topology_notification, ctx->mid, if_num);
#endif
				}
			}
		} else {
			debug(DEBUG_ERROR, TOPO_PREX"[LIB_MAP_BH_READY]no topo msg from neighbor dev("MACSTR") with peer intf("MACSTR")\n",
					PRINT_MAC(almac), PRINT_MAC(info->bssid));
		}

		/*trigger_autconf will be set in first wifi link in wifi case or in eth case*/
		if (info->trigger_autconf) {
			ctx->controller_search_state = bk_link_ready;
		    ctx->controller_search_cnt = 0;
			eloop_register_timeout(0, 0, ap_controller_search_step, (void *)ctx, NULL);
		}
	}
		break;
	case WAPP_NOTIFY_WIRELESS_BH_LINK_DOWN:
	{
		struct bh_link_info *info = NULL;
		struct topology_discovery_db *db = NULL;
		int i = 0;
		unsigned char delete_almac[ETH_ALEN] = {0};
		int ret = 0;
		struct p1905_neighbor_info *dev_info, *dev_info_temp = NULL;

		info = (struct bh_link_info *)wapp_event->buffer;

		debug(DEBUG_OFF, "backhaul link down(%s-"MACSTR")\n",
				info->ifname, PRINT_MAC(info->mac_addr));

		if (info->type != MAP_BH_WIFI) {
			debug(DEBUG_ERROR, "not wifi link down event\n");
			break;
		}

		for (i = 0; i < ctx->itf_number; i++) {
			if (!os_memcmp(info->mac_addr, ctx->itf[i].mac_addr, ETH_ALEN)) {
				ret = 1;
				if (ctx->itf[i].is_wifi_sta != 1) {
					debug(DEBUG_ERROR, "not apcli interface\n");
					ret = -1;
				}
				break;
			}
		}
		if (ret != 1) {
			debug(DEBUG_ERROR, "wrong bh_link_info!!! ret(%d)\n", ret);
			break;
		}

		reset_controller_connect_ifname(ctx, ctx->itf[i].if_name);

		db = get_tpd_db_by_mac(ctx, ctx->itf[i].vs_info);
		os_memset(ctx->itf[i].vs_info, 0, ETH_ALEN);
		/*temporarily set sta interface not to send multicast cmdu which is not connected*/
		ctx->itf[i].trx_config = 0;
		if (!db) {
			debug(DEBUG_ERROR, TOPO_PREX"[WAPP_NOTIFY_WIRELESS_BH_LINK_DOWN]no discovery in db for peer intf("MACSTR")\n",
				PRINT_MAC(ctx->itf[i].vs_info));
			/*clear neighbor info for apcli for error handle*/
			dev_info = LIST_FIRST(&ctx->p1905_neighbor_dev[i].p1905nbr_head);
			while (dev_info) {
				dev_info_temp = LIST_NEXT(dev_info, p1905nbr_entry);
				LIST_REMOVE(dev_info, p1905nbr_entry);
				debug(DEBUG_ERROR, TOPO_PREX"[WAPP_NOTIFY_WIRELESS_BH_LINK_DOWN]del neighbor("MACSTR") on %s\n",
					PRINT_MAC(dev_info->al_mac_addr), ctx->itf[i].if_name);
				os_free(dev_info);
				dev_info = dev_info_temp;
			}
			break;
		}

		debug(DEBUG_ERROR, TOPO_PREX"[WAPP_NOTIFY_WIRELESS_BH_LINK_DOWN]del discovery("MACSTR") peer intf("MACSTR") recv intf("MACSTR")\n",
			PRINT_MAC(db->al_mac), PRINT_MAC(db->itf_mac), PRINT_MAC(db->receive_itf_mac));
		os_memcpy(delete_almac, db->al_mac, ETH_ALEN);
		delete_exist_topology_discovery_database(ctx, db->al_mac, db->itf_mac);
		/*cannot refer db, the memory is free*/
		report_own_topology_rsp(ctx, ctx->p1905_al_mac_addr, ctx->br_cap,
			ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
			ctx->service, &ctx->ap_cap_entry.oper_bss_head,
			&ctx->ap_cap_entry.assoc_clients_head, ctx->cnt);
		if (find_discovery_by_almac(ctx, delete_almac) == NULL) {
			debug(DEBUG_ERROR, TOPO_PREX"[WAPP_NOTIFY_WIRELESS_BH_LINK_DOWN]del topology rsp\n");
			delete_exist_topology_response_database(ctx, delete_almac);
			mark_valid_topo_rsp_node(ctx);
		}
		update_topology_tree(ctx);

		ctx->mid++;
		debug(DEBUG_ERROR, TOPO_PREX"[WAPP_NOTIFY_WIRELESS_BH_LINK_DOWN]tx multicast notification(mid-%04x) on all intf\n",
				ctx->mid);
		for(if_num = 0; if_num < ctx->itf_number; if_num++) {
			insert_cmdu_txq((unsigned char*)p1905_multicast_address, (unsigned char*)ctx->p1905_al_mac_addr,
            	e_topology_notification, ctx->mid, ctx->itf[if_num].if_name, 0);
#ifdef SUPPORT_CMDU_RELIABLE
			cmdu_reliable_send(ctx, e_topology_notification, ctx->mid, if_num);
#endif
		}
	}
		break;
	case WAPP_NOTIFY_CH_BW_INFO:
	{
		struct channel_bw_info *info = NULL;
		info = (struct channel_bw_info *)wapp_event->buffer;
		for(if_num = 0; if_num < ctx->itf_number; if_num++) {
			if (!memcmp(ctx->itf[if_num].mac_addr, info->iface_addr,
				ETH_ALEN)) {
				ctx->itf[if_num].channel_bw = info->channel_bw;
				ctx->itf[if_num].channel_freq = info->channel_num;
			}
		}
		break;
	}
	case WAPP_GET_OWN_TOPOLOGY_RSP:
	{
		debug(DEBUG_OFF, YLW("get my own topology rsp") "\n");
		report_own_topology_rsp(ctx, ctx->p1905_al_mac_addr, ctx->br_cap,
			ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
	       	ctx->service, &ctx->ap_cap_entry.oper_bss_head,
	        &ctx->ap_cap_entry.assoc_clients_head, ctx->cnt);
	}
		break;
	case GET_SWITCH_STATUS:
	{
		int i = 0, status_temp = 0;
		unsigned char status = 0;

		for (i = 0; i < MAX_LAN_PORT_NUM; i++) {
			if ((status_temp = eth_layer_get_eth_port_status(i)) < 0) {
				status = 0xff;
				break;
			} else {
				if (status_temp)
					status |= (1 << i);
			}
		}

		_1905_notify_switch_status(ctx, status);
		debug(DEBUG_OFF, "notify switch status %02x\n", status);
	}
		break;
	case LIB_RADIO_BASIC_CAP:
	{
		struct ap_radio_basic_cap *bcap = NULL;

		bcap = (struct ap_radio_basic_cap *)wapp_event->buffer;
		delete_exist_radio_basic_capability(ctx, bcap->identifier);
		update_radio_basic_capability(ctx, bcap, wapp_event->length);
	}
		break;
	case LIB_AP_CAPABILITY:
	{
		struct ap_capability *cap = NULL;

		cap = (struct ap_capability *)wapp_event->buffer;
		memcpy(&ctx->ap_cap_entry.ap_cap, cap, sizeof(*cap));
		debug(DEBUG_TRACE, "sta_report_on_cop=%d, sta_report_not_cop%d, rssi_steer=%d\n",
			cap->sta_report_on_cop, cap->sta_report_not_cop, cap->rssi_steer);
	}
		break;
	case LIB_AP_HT_CAPABILITY:
	{
		struct ap_ht_capability *cap = NULL;

		cap = (struct ap_ht_capability *)wapp_event->buffer;
		update_ap_ht_cap(ctx, cap);
		debug(DEBUG_TRACE, "tx_stream=%d, rx_stream=%d, sgi_20=%d, sgi_40=%d, ht_40=%d, band=%d\n",
			cap->tx_stream, cap->rx_stream, cap->sgi_20, cap->sgi_40, cap->ht_40, cap->band);
	}
		break;
	case LIB_AP_VHT_CAPABILITY:
	{
		struct ap_vht_capability *cap = NULL;

		cap = (struct ap_vht_capability *)wapp_event->buffer;
		update_ap_vht_cap(ctx, cap);
		debug(DEBUG_TRACE, "identifier("MACSTR")\n",
			PRINT_MAC(cap->identifier));
		debug(DEBUG_TRACE, "vht_tx_mcs=%x, vht_rx_mcs=%x, tx_stream=%d, rx_stream=%d\n",
			cap->vht_tx_mcs, cap->vht_rx_mcs, cap->tx_stream, cap->rx_stream);
		debug(DEBUG_TRACE, "sgi_80=%d, sgi_160=%d, vht_8080=%d, vht_160=%d\n",
			cap->sgi_80, cap->sgi_160, cap->vht_8080, cap->vht_160);
		debug(DEBUG_TRACE, "su_beamformer=%d, mu_beamformer=%d, band=%d\n",
			cap->su_beamformer, cap->mu_beamformer, cap->band);
	}
		break;
	case LIB_AP_HE_CAPABILITY:
	{
		struct ap_he_capability *cap = NULL;

		cap = (struct ap_he_capability *)wapp_event->buffer;
		update_ap_he_cap(ctx, cap);
		if (cap->he_mcs_len >= 2 && cap->he_mcs_len <=12)
			hex_dump("he_mcs", cap->he_mcs, cap->he_mcs_len);
		debug(DEBUG_TRACE, "tx_stream=%d, rx_stream=%d, he_8080=%d, he_160=%d\n",
			cap->tx_stream, cap->rx_stream, cap->he_8080, cap->he_160);
		debug(DEBUG_TRACE, "su_bf_cap=%d, mu_bf_cap=%d, ul_mu_mimo_cap=%d\n",
			cap->su_bf_cap, cap->mu_bf_cap, cap->ul_mu_mimo_cap);
		debug(DEBUG_TRACE, "ul_mu_mimo_ofdma_cap=%d, dl_mu_mimo_ofdma_cap=%d\n",
			cap->ul_mu_mimo_ofdma_cap, cap->dl_mu_mimo_ofdma_cap);
		debug(DEBUG_TRACE, "ul_ofdma_cap=%d, dl_ofdma_cap=%d\n",
			cap->ul_ofdma_cap, cap->dl_ofdma_cap);
	}
		break;
#ifdef MAP_R2
	/*channel scan feature*/
	//case WAPP_CHANNEL_SCAN_CAP:
	case LIB_CHANNEL_SCAN_CAPAB:
	{
		struct scan_capability_lib *scap = NULL;

		scap = (struct scan_capability_lib *)wapp_event->buffer;
		delete_exist_channel_scan_capability(ctx, scap->identifier);
		insert_new_channel_scan_capability(ctx, scap);
	}
		break;
#endif
	case LIB_CLI_CAPABILITY_REPORT:
	{
		struct client_capa_rep *cap = NULL;
		struct client_capa_rep *ctx_cap = NULL;

		cap = (struct client_capa_rep *)wapp_event->buffer;
		free(ctx->sinfo.pcli_rep);
		ctx->sinfo.pcli_rep = (struct client_capa_rep *)malloc(cap->length + 1);
		if(NULL == ctx->sinfo.pcli_rep) {
			debug(DEBUG_ERROR, "LIB_CLI_CAPABILITY_REPORT allocate memory fail (%s)\n",
				strerror(errno));
       		break;
    	}
		ctx_cap = ctx->sinfo.pcli_rep;
		ctx_cap->result = cap->result;
		ctx_cap->length = 0;
		if (ctx_cap->result == 0) {
			ctx_cap->length = cap->length;
			memcpy(ctx_cap->body, cap->body, ctx_cap->length);
			hex_dump("assoc req", ctx_cap->body, ctx_cap->length);
		}
	}
		break;
	case LIB_OPERBSS_REPORT:
	{
		struct oper_bss_cap *opcap = NULL;

		opcap = (struct oper_bss_cap *)wapp_event->buffer;
		delete_exist_operational_bss(ctx, opcap->identifier);
		insert_new_operational_bss(ctx, opcap);
		delete_legacy_assoc_clients_db(ctx);
	}
		break;
	case LIB_CHANNLE_PREFERENCE:
	{
		struct ch_prefer *prefer = NULL;

		prefer = (struct ch_prefer *)wapp_event->buffer;
		delete_exist_ch_prefer_info(ctx, prefer->identifier);
		insert_new_channel_prefer_info(ctx, prefer);
	}
		break;
	case WAPP_CHANNEL_PREFERENCE_REPORT_INFO:
	{
		if(fill_send_tlv(ctx, wapp_event->buffer, wapp_event->length) < 0)
		{
			break;
		}
		if (wapp_event->mid) {
			mid = wapp_event->mid;
			debug(DEBUG_TRACE, "send channel preference report to controller("MACSTR") with mid(%04x)\n",
				PRINT_MAC(ctx->cinfo.almac), mid);
		} else {
			mid = ++ctx->mid;
			debug(DEBUG_TRACE, "send unsolicited channel preference report to controller("MACSTR") with mid(%04x)\n",
				PRINT_MAC(ctx->cinfo.almac), mid);
		}
		insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
			e_channel_preference_report, mid, ctx->cinfo.local_ifname, 0);
	}
		break;
	case WAPP_CHANNEL_SELECTION_RSP_INFO:
	{
		if(fill_send_tlv(ctx, wapp_event->buffer, wapp_event->length) < 0)
		{
			break;
		}
		insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
			e_channel_selection_response, wapp_event->mid, ctx->cinfo.local_ifname, 0);
	}
		break;
	case LIB_RADIO_OPERATION_RESTRICTION:
	{
		struct restriction *resdb = NULL;

		resdb = (struct restriction *)wapp_event->buffer;
		delete_exist_restriction_info(ctx, resdb->identifier);
		insert_new_restriction_info(ctx, resdb);
	}
		break;
	case LIB_TX_LINK_STATISTICS:
	{
		struct tx_link_stat_rsp *rsp = NULL;

		rsp = (struct tx_link_stat_rsp *)wapp_event->buffer;
		memcpy(&ctx->link_stat.tx_link_stat, rsp,
			sizeof(struct tx_link_stat_rsp));
	}
		break;
	case LIB_RX_LINK_STATISTICS:
	{
		struct rx_link_stat_rsp *rsp = NULL;

		rsp = (struct rx_link_stat_rsp *)wapp_event->buffer;
		memcpy(&ctx->link_stat.rx_link_stat, rsp,
			sizeof(struct rx_link_stat_rsp));
	}
		break;
	case WAPP_AP_METRICS_RSP_INFO:
	{
		if(fill_send_tlv(ctx, wapp_event->buffer, wapp_event->length) < 0)
		{
			break;
		}
		if (wapp_event->mid) {
			mid = wapp_event->mid;
			debug(DEBUG_TRACE, "send ap metrics rsp to controller("MACSTR") with mid(%04x)\n",
				PRINT_MAC(ctx->cinfo.almac), mid);
		} else {
			mid = ++ctx->mid;
			debug(DEBUG_TRACE, "send unsolicited ap metrics rsp to controller("MACSTR") with mid(%04x)\n",
				PRINT_MAC(ctx->cinfo.almac), mid);
		}
		insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
			e_ap_metrics_response, mid, ctx->cinfo.local_ifname, 0);
	}
		break;
	case LIB_AP_METRICS_INFO:
	{
		struct ap_metrics_info *minfo = NULL;

		minfo = (struct ap_metrics_info *)wapp_event->buffer;
		delete_exist_metrics_info(ctx, minfo->bssid);
		insert_new_metrics_info(ctx, minfo);
	}
		break;
	case LIB_ALL_ASSOC_STA_TRAFFIC_STATS:
	{
		struct sta_traffic_stats *stats = NULL;

		stats = (struct sta_traffic_stats *)wapp_event->buffer;
		delete_exist_traffic_stats_info(ctx, stats->identifier);
		insert_new_traffic_stats_info(ctx, stats);
	}
		break;
	case LIB_ALL_ASSOC_STA_LINK_METRICS:
	{
		struct sta_link_metrics *metrics = NULL;

		metrics = (struct sta_link_metrics *)wapp_event->buffer;
		delete_exist_link_metrics_info(ctx, metrics->identifier);
		insert_new_link_metrics_info(ctx, metrics);
	}
		break;
	case LIB_ONE_ASSOC_STA_LINK_METRICS:
	{
		if(fill_send_tlv(ctx, wapp_event->buffer, wapp_event->length) < 0)
		{
			break;
		}
		if (wapp_event->mid) {
			mid = wapp_event->mid;
			debug(DEBUG_TRACE, "send assoc sta link metrics rsp to controller("MACSTR") with mid(%04x)\n",
				PRINT_MAC(ctx->cinfo.almac), mid);
		} else {
			mid = ++ctx->mid;
			debug(DEBUG_TRACE, "send unsolicited assoc sta link metrics rsp to controller("MACSTR") with mid(%04x)\n",
				PRINT_MAC(ctx->cinfo.almac), mid);
		}
		insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
					e_associated_sta_link_metrics_response,
					mid, ctx->cinfo.local_ifname, 0);
	}
		break;
	case LIB_UNASSOC_STA_LINK_METRICS:
	{
		struct delayed_message_buf dm_buf;

		memset(&dm_buf, 0, sizeof(dm_buf));
		debug(DEBUG_ERROR, "receive unassoc STA link metrics rsp info\n");
		if (!pop_dm_buffer(ctx, LIB_UNASSOC_STA_LINK_METRICS, dm_buf.al_mac, &dm_buf.mid, dm_buf.if_name)) {
			if (fill_send_tlv(ctx, wapp_event->buffer, wapp_event->length) < 0)
				break;
			insert_cmdu_txq(dm_buf.al_mac, ctx->p1905_al_mac_addr,
				e_unassociated_sta_link_metrics_response, ctx->mid++, (unsigned char *)dm_buf.if_name, 0);
		} else {
			debug(DEBUG_ERROR, "cannot find WAPP_UNASSOC_STA_LINK_METRICS"
				"buf entry with almac "MACSTR" mid(%04x)\n",
				MAC2STR(dm_buf.al_mac), dm_buf.mid);
		}
	}
		break;
	case LIB_OPERATING_CHANNEL_INFO:
		update_channel_report_info(ctx, (struct channel_report *)wapp_event->buffer,
			wapp_event->length);
		break;
	case LIB_CLIENT_NOTIFICATION:
	{
		struct client_association_event *evt = NULL;
		struct map_client_association_event *pevt = NULL;
		struct topology_discovery_db *db = NULL;
		unsigned char delete_almac[ETH_ALEN] = {0};
#ifdef MAP_R2
		struct assoc_client *client = NULL;
#endif

		evt = (struct client_association_event *)wapp_event->buffer;
		pevt = (struct map_client_association_event *)&evt->map_assoc_evt;
		memcpy(&ctx->sinfo.sassoc_evt.sta_mac, pevt->sta_mac, ETH_ALEN);
		memcpy(&ctx->sinfo.sassoc_evt.bssid, pevt->bssid, ETH_ALEN);
		ctx->sinfo.sassoc_evt.assoc_evt = pevt->assoc_evt;
		ctx->sta_notifier = 1;

		debug(DEBUG_ERROR, TOPO_PREX"[LIB_CLIENT_NOTIFICATION]sta("MACSTR") bssid("MACSTR") assoc(%d) is_APCLI(%d)\n",
			PRINT_MAC(pevt->sta_mac), PRINT_MAC(pevt->bssid), pevt->assoc_evt, pevt->is_APCLI);

		if (pevt->assoc_evt == 0) {
			db = get_tpd_db_by_mac(ctx, pevt->sta_mac);
			if (db) {
				if (!os_memcmp(db->receive_itf_mac, pevt->bssid, ETH_ALEN)) {
					os_memcpy(delete_almac, db->al_mac, ETH_ALEN);
					debug(DEBUG_ERROR, TOPO_PREX"[LIB_CLIENT_NOTIFICATION]del discovery("MACSTR") peer intf("MACSTR") recv intf("MACSTR")\n",
						PRINT_MAC(db->al_mac), PRINT_MAC(db->itf_mac), PRINT_MAC(db->receive_itf_mac));
					delete_exist_topology_discovery_database(ctx, db->al_mac, db->itf_mac);
					/*cannot refer db, the memory is free*/
					report_own_topology_rsp(ctx, ctx->p1905_al_mac_addr, ctx->br_cap,
						ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
				       	ctx->service, &ctx->ap_cap_entry.oper_bss_head,
				        &ctx->ap_cap_entry.assoc_clients_head, ctx->cnt);
					if (find_discovery_by_almac(ctx, delete_almac) == NULL) {
						debug(DEBUG_OFF, TOPO_PREX"[LIB_CLIENT_NOTIFICATION] no discovery! del rsp\n");
						delete_exist_topology_response_database(ctx, delete_almac);
						mark_valid_topo_rsp_node(ctx);
					}
					update_topology_tree(ctx);
				} else {
					debug(DEBUG_ERROR, TOPO_PREX"bssid not match bssid("MACSTR") receive_itf_mac("MACSTR")\n",
						PRINT_MAC(pevt->bssid), PRINT_MAC(db->receive_itf_mac));
				}
			}
		} else {
			detect_neighbor_existence(ctx, NULL, pevt->sta_mac);
		}

		if (pevt->assoc_evt) {
			if (check_sta_in_oper_bss(&(ctx->ap_cap_entry.oper_bss_head), pevt)) {
				debug(DEBUG_ERROR, "WARN! bss "MACSTR" which the sta "MACSTR" assoced is not operational\n",
					MAC2STR(pevt->bssid), MAC2STR(pevt->sta_mac));
				break;
			}
		}

		if(update_assoc_sta_info(ctx, pevt) < 0) {
			debug(DEBUG_ERROR, "update station info fail\n");
			break;
		}
#ifdef MAP_R2
		client = get_assoc_cli_by_mac(ctx, pevt->sta_mac);

		if (client && pevt->assoc_evt == 0) {
			client->disassoc = 1;
		} else if (!client && pevt->assoc_evt) {
			client = create_assoc_cli(ctx, pevt->sta_mac, ctx->p1905_al_mac_addr);
			if (client) {
				os_memcpy(client->bssid, pevt->bssid, ETH_ALEN);
			}
			debug(DEBUG_ERROR, "add sta("MACSTR") bssid("MACSTR")\n",
				PRINT_MAC(pevt->sta_mac), PRINT_MAC(pevt->bssid));
		}
		maintain_all_assoc_clients(ctx);
#endif

		/*send notification with assoc sta info to notify other device my own topology is changed*/
		ctx->mid++;
		debug(DEBUG_ERROR, TOPO_PREX"[LIB_CLIENT_NOTIFICATION]tx multicast notification(mid-%04x) on all intf\n",
			ctx->mid);
		for(if_num = 0; if_num < ctx->itf_number; if_num++) {
			insert_cmdu_txq(p1905_multicast_address, ctx->p1905_al_mac_addr,
	        	e_topology_notification, ctx->mid, ctx->itf[if_num].if_name, 0);
#ifdef SUPPORT_CMDU_RELIABLE
			cmdu_reliable_send(ctx, e_topology_notification, ctx->mid, if_num);
#endif
		}

		if (pevt->assoc_evt && pevt->is_APCLI) {
			for(if_num = 0; if_num < ctx->itf_number; if_num++) {
				if (!os_memcmp(ctx->itf[if_num].mac_addr, pevt->bssid, ETH_ALEN)) {
					debug(DEBUG_ERROR, TOPO_PREX"[LIB_CLIENT_NOTIFICATION]tx multicast discovery on"
						" %s for downstream MAP agent\n", ctx->itf[if_num].if_name);
					eloop_register_timeout(0, 0, discovery_at_interface_link_up,
						(void *)ctx, (void *)ctx->itf[if_num].if_name);
					break;
				}
			}
		}
	}
		break;
	case LIB_OPERATING_CHANNEL_REPORT:
		if (0 > update_channel_report_info(ctx,
					(struct channel_report *)wapp_event->buffer, wapp_event->length)) {
			debug(DEBUG_ERROR, "no need to respond this invalid msg\n");
			break;
		}

		debug(DEBUG_TRACE, "send e_operating_channel_report to "MACSTR"\n",
			PRINT_MAC(ctx->cinfo.almac));
		insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
			e_operating_channel_report, ++ctx->mid, ctx->cinfo.local_ifname, 0);
		break;
	case LIB_BEACON_METRICS_REPORT:
	{
		if(fill_send_tlv(ctx, wapp_event->buffer, wapp_event->length) < 0)
			break;
    	debug(DEBUG_TRACE, "send BEACON_METRICS_REPORT to "MACSTR"\n",
		 	PRINT_MAC(ctx->cinfo.almac));

        insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
        	e_beacon_metrics_response, ++ctx->mid, ctx->cinfo.local_ifname, 0);
	}
		break;
	case LIB_CLI_STEER_BTM_REPORT:
	{
		struct cli_steer_btm_event *evt = NULL;

		evt = (struct cli_steer_btm_event *)wapp_event->buffer;
		memcpy(ctx->sinfo.sbtm_evt.bssid, evt->bssid, ETH_ALEN);
		memcpy(ctx->sinfo.sbtm_evt.sta_mac, evt->sta_mac, ETH_ALEN);
		ctx->sinfo.sbtm_evt.status = evt->status;
		memcpy(ctx->sinfo.sbtm_evt.tbssid, evt->tbssid, ETH_ALEN);

    	debug(DEBUG_TRACE, "send CLI_STEER_BTM_REPORT to "MACSTR"\n",
		 	PRINT_MAC(ctx->cinfo.almac));
        insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
        	e_client_steering_btm_report, ++ctx->mid, ctx->cinfo.local_ifname, 0);
	}
		break;
	case LIB_STEERING_COMPLETED:
	{
		debug(DEBUG_TRACE, "send STEERING_COMPLETED to "MACSTR"\n",
			PRINT_MAC(ctx->cinfo.almac));
		insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
			e_steering_completed, ++ctx->mid, ctx->cinfo.local_ifname, 0);
	}
		break;
	case LIB_BACKHAUL_STEER_RSP:
	{
		if(fill_send_tlv(ctx, wapp_event->buffer, wapp_event->length) < 0)
			break;
    	debug(DEBUG_TRACE, "send BACKHAUL_STEER_RSP to "MACSTR"\n",
		 	PRINT_MAC(ctx->cinfo.almac));
        insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
        	e_backhaul_steering_response, ++ctx->mid, ctx->cinfo.local_ifname, 0);
	}
		break;
	/*below event is mainly from 1905.1 library*/
	case LIB_1905_CMDU_REQUEST:
		{
			struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)wapp_event->buffer;
			debug(DEBUG_TRACE, "[LIB CMDU REQUEST] dest al mac="MACSTR", type=%04x\n",
				PRINT_MAC(request->dest_al_mac), request->type);
			process_1905_request(ctx, request);
		}
		break;
	case LIB_1905_READ_BSS_CONF_REQUEST:
		{
			if (ctx->role != CONTROLLER)
				break;

			if(_1905_read_set_config(ctx, ctx->wts_bss_cfg_file, ctx->bss_config, MAX_SET_BSS_INFO_NUM, &ctx->bss_config_num) < 0)
			{
				debug(DEBUG_ERROR, "load bss config fail from %s\n", ctx->wts_bss_cfg_file);
			}
		}
		break;
	case LIB_1905_READ_BSS_CONF_AND_RENEW:
		{
			unsigned char local_only = 0;
			unsigned char version = 0;

			if (ctx->role != CONTROLLER)
				break;
			local_only = *(wapp_event->buffer);
			version = *(wapp_event->buffer + 1);

			/*read bss info to 1905*/
			if(version == 1) {
				if (read_bss_conf_and_renew(ctx, local_only) < 0) {
					debug(DEBUG_ERROR, "load bss config fail\n");
					break;
				}
			} else {
				if (read_bss_conf_and_renew_v2(ctx, local_only) < 0) {
					debug(DEBUG_ERROR, "load bss config fail\n");
					break;
				}
			}
		}
		break;
	case LIB_1905_REQ:
		break;
	case WAPP_LINK_METRICS_RSP_INFO:
		{
			struct delayed_message_buf dm_buf;

			debug(DEBUG_TRACE, "receive link metrics rsp info\n");
			os_memset(&dm_buf, 0, sizeof(struct delayed_message_buf));
			if (!pop_dm_buffer(ctx, WAPP_LINK_METRICS_RSP_INFO, dm_buf.al_mac, &dm_buf.mid,
				dm_buf.if_name)) {
				if (fill_send_tlv(ctx, wapp_event->buffer, wapp_event->length) < 0) {
					break;
				}
				insert_cmdu_txq(dm_buf.al_mac, ctx->p1905_al_mac_addr,
					e_link_metric_response, dm_buf.mid, (unsigned char*)dm_buf.if_name, 0);
				debug(DEBUG_TRACE, "tx link metrics rsp\n");
				process_cmdu_txq(ctx, ctx->trx_buf.buf);
			} else {
				debug(DEBUG_ERROR, "cannot find WAPP_LINK_METRICS_RSP_INFO delaying message buf entry\n");
			}
		}
		break;
	case MANAGEMENT_1905_COMMAND:
		{
			debug(DEBUG_TRACE, "receive MANAGEMENT_1905_COMMAND\n");
			manage_cmd_process(ctx, (struct manage_cmd*)(wapp_event->buffer));
		}
		break;
	case LIB_SYNC_RECV_BUF_SIZE:
		{
			unsigned short length = 0, peer_len = 0, own_len = 0;
			int ret = 0, rsp = 0;
			os_memcpy( &length, wapp_event->buffer, sizeof(length));
			os_memcpy( &peer_len, wapp_event->buffer + sizeof(length), sizeof(peer_len));
			debug(DEBUG_OFF, "receive LIB_SYNC_RECV_BUF_SIZE (notify len:%d, peer len:%d) from mapd, own real len:%d, default len:%d \n", length, peer_len, ctx->_1905_ctrl.own_recv_buf_len, ctx->_1905_ctrl.default_own_recv_len);
                     // change buffer size
            if(length > 0 && peer_len > 0) {
				if(ctx->_1905_ctrl.own_recv_buf_len != length) {
					if(length <= ctx->_1905_ctrl.default_own_recv_len) {
						own_len = ctx->_1905_ctrl.default_own_recv_len;
					} else
						own_len = length;
					ret = _1905_interface_set_sock_buf(ctx, own_len);
					if(ret == 0) {
						rsp = 1;
						ctx->_1905_ctrl.own_recv_buf_len = own_len;
						debug(DEBUG_OFF, " change own real len success:%d\n", ctx->_1905_ctrl.own_recv_buf_len);
					}
				}
				if(ctx->_1905_ctrl.peer_recv_buf_len != peer_len) {
					if(!ctx->_1905_ctrl.peer_recv_buf_len && !ctx->_1905_ctrl.default_peer_recv_len)
						ctx->_1905_ctrl.default_peer_recv_len = peer_len;
					ctx->_1905_ctrl.peer_recv_buf_len = peer_len;
					debug(DEBUG_OFF, " change peer len success:%d\n", ctx->_1905_ctrl.peer_recv_buf_len);
				}
			}
			_1905_send_sync_recv_buf_size_rsp(ctx, ctx->_1905_ctrl.own_recv_buf_len, rsp);
		}
		break;
	case LIB_SYNC_RECV_BUF_SIZE_RSP:
		{
			unsigned short change_buf_len = 0, rsp = 0;
			memcpy(&change_buf_len, wapp_event->buffer, sizeof(change_buf_len));
			rsp = wapp_event->buffer[sizeof(change_buf_len)];
			debug(DEBUG_OFF, "receive the LIB_SYNC_RECV_BUF_SIZE_RSP from mapd \n");

			if(rsp == 1) {
				if(!ctx->_1905_ctrl.peer_recv_buf_len && !ctx->_1905_ctrl.default_peer_recv_len)
						ctx->_1905_ctrl.default_peer_recv_len = change_buf_len;
				ctx->_1905_ctrl.peer_recv_buf_len = change_buf_len;
				debug(DEBUG_OFF, "change buf size success len:%d\n", change_buf_len);
				return 0;
			} else {
				debug(DEBUG_OFF, "change buf size fail:%d\n",change_buf_len);
				return -1;
			}
		}
		break;
#ifdef MAP_R2

	case LIB_CAC_CAPAB:
		{
			debug(DEBUG_TRACE, "receive LIB_CAC_CAPAB\n");

			insert_new_cac_capability(ctx, (struct cac_capability_lib *)wapp_event->buffer);
		}
		break;

	case LIB_R2_AP_CAP:
		{
			debug(DEBUG_TRACE, "receive LIB_R2_AP_CAP\n");

			update_r2_ap_capability(ctx, (struct ap_r2_capability *)wapp_event->buffer);
		}
		break;

	case LIB_METRIC_REP_INTERVAL_CAP:
		{
			debug(DEBUG_TRACE, "receive LIB_METRIC_REP_INTERVAL_CAP\n");
			unsigned int *interval = (unsigned int *)wapp_event->buffer;
			ctx->metric_entry.metric_collection_interval = *interval;
			debug(DEBUG_TRACE, "metric_rep_interval is %d\n", *interval);
		}
		break;

	case LIB_AP_EXTENDED_METRICS_INFO:
		{
			debug(DEBUG_TRACE, "receive LIB_AP_EXTENDED_METRICS_INFO\n");

			insert_new_ap_extended_capability(ctx, (struct ap_extended_metrics_lib *)wapp_event->buffer);
		}
		break;

	case LIB_RADIO_METRICS_INFO:
		{
			debug(DEBUG_TRACE, "receive LIB_RADIO_METRICS_INFO\n");

			insert_new_radio_metric(ctx, (struct radio_metrics_lib *)wapp_event->buffer);
		}
		break;

	case LIB_ASSOC_STA_EXTENDED_LINK_METRICS:
		{
			debug(DEBUG_TRACE, "receive LIB_ASSOC_STA_EXTENDED_LINK_METRICS\n");

			insert_assoc_sta_extended_link_metric(ctx, (struct sta_extended_metrics_lib *)wapp_event->buffer);
		}
		break;
#endif // #ifdef MAP_R2
#ifdef MAP_R3
	case LIB_SET_1905_CONNECTOR:
		{
			struct r3_dpp_information *dpp = &ctx->r3_dpp;
			unsigned char *p = wapp_event->buffer;
			unsigned short len = 0;
			struct r3_member *peer = NULL;

			debug(DEBUG_ERROR, "[LIB_SET_1905_CONNECTOR]\n");
			/* 2 bytes decrypt_fail_threshold */
			ctx->decrypt_fail_threshold = *((unsigned short *)p);
			p += sizeof(unsigned short);

			/* 2 bytes connector len */
			len = *((unsigned short *)p);
			dpp->connector_len = len;
			p += sizeof(unsigned short);

			/* connector */
			if (dpp->connector)
				os_free(dpp->connector);

			dpp->connector = os_zalloc(len + 1);
			if (!dpp->connector) {
				debug(DEBUG_ERROR, "malloc connector space fail\n");
				break;
			}
			os_memcpy(dpp->connector, p, len);
			p += len;
			hex_dump_info("connetor", (u8 *)dpp->connector, len);

			/* 2 bytes CsignKey len */
			len = *((unsigned short *)p);
			dpp->CsignKey_len = len;
			p += sizeof(unsigned short);

			/* CsignKey */
			if (dpp->CsignKey)
				os_free(dpp->CsignKey);

			dpp->CsignKey = os_zalloc(len + 1);
			if (!dpp->CsignKey) {
				debug(DEBUG_ERROR, "malloc CsignKey space fail\n");
				break;
			}
			os_memcpy(dpp->CsignKey, p, len);
			p += len;
			hex_dump_info("CsignKey", dpp->CsignKey, dpp->CsignKey_len);

			/* 2 bytes netAccessKey len */
			len = *((unsigned short *)p);
			dpp->netAccessKey_len = len;
			p += sizeof(unsigned short);

			/* netAccessKey */
			if (dpp->netAccessKey)
				os_free(dpp->netAccessKey);

			dpp->netAccessKey = os_zalloc(len + 1);
			if (!dpp->netAccessKey) {
				debug(DEBUG_ERROR, "malloc netAccessKey space fail\n");
				break;
			}
			os_memcpy(dpp->netAccessKey, p, len);
			p += len;
			hex_dump_info("netAccessKey", dpp->netAccessKey, dpp->netAccessKey_len);

			/* 4 bytes expiry */
			dpp->netAccessKey_expiry = *(unsigned int *)p;
			p += sizeof(unsigned int);

			map_dpp_save_keys(dpp);

			dpp->initialized = 1;

			/* cancel timer to stop periodic autoconfig search  */
			eloop_cancel_timeout(periodic_send_autoconfig_search, (void *)ctx, NULL);

			/*for ethernet onboarding, trigger dpp discovery when set connector which means
			  *  DPP auth/config procedure have been completed
			  */
			if (ctx->r3_oboard_ctx.active && ctx->r3_oboard_ctx.bh_type == MAP_BH_ETH) {
				debug(DEBUG_ERROR, "[LIB_SET_1905_CONNECTOR]ETH case,ready to start dpp introduction with Controller\n");
				peer = get_r3_member(ctx->r3_oboard_ctx.peer_almac);
				if (!peer) {
					debug(DEBUG_ERROR, "[LIB_SET_1905_CONNECTOR]fail to start dpp introduction, no R3 member found\n");
					break;
				}
				peer->r3_info.cur_dpp_state = dpp_idle;

				map_cancel_onboarding_timer(ctx, peer);

				map_dpp_trigger_member_dpp_intro(ctx, peer);
			}

		}
		break;

		case LIB_SET_BSS_CONNECTOR:
		{
			struct r3_dpp_information *dpp = &ctx->r3_dpp;
			unsigned char *p = wapp_event->buffer;
			struct bss_connector_item *bc_item = NULL, *bc_item_nxt = NULL;
			struct bss_connector_item bc = {0};
			unsigned short found = 0;
			unsigned char all_zero_mac[ETH_ALEN];

			os_memset(&bc, 0, sizeof(bc));
			os_memset(&all_zero_mac, 0, sizeof(all_zero_mac));

			debug(DEBUG_ERROR, "receive LIB_SET_BSS_CONNECTOR\n");
			/* mac of apcli */
			os_memcpy(bc.mac_apcli, p, ETH_ALEN);
			p += ETH_ALEN;

			/* almac */
			os_memcpy(bc.almac, p, ETH_ALEN);
			p += ETH_ALEN;

			/* 2 bytes bh connector len */
			bc.bh_connector_len = *((unsigned short *)p);
			p += sizeof(unsigned short);

			/* 2 bytes fh connector len */
			bc.fh_connector_len = *((unsigned short *)p);
			p += sizeof(unsigned short);

			bc.bh_connector = (char *)p;
			p += bc.bh_connector_len;
			bc.fh_connector = (char *)p;
			p += bc.fh_connector_len;

			dl_list_for_each_safe(bc_item, bc_item_nxt,
				&dpp->bss_connector_list, struct bss_connector_item, member) {
				if ((!os_memcmp(bc.mac_apcli, all_zero_mac, ETH_ALEN)) &&
					(!os_memcmp(bc.almac, all_zero_mac, ETH_ALEN))) {
					if ((!os_memcmp(bc_item->mac_apcli, bc.mac_apcli, ETH_ALEN)) &&
						(!os_memcmp(bc_item->almac, bc.almac, ETH_ALEN))) {
						debug(DEBUG_ERROR, "self case, update for self\n");
						found = 1;
						break;
					}
				}
				else if (os_memcmp(bc.mac_apcli, all_zero_mac, ETH_ALEN)) {
					if (!os_memcmp(bc_item->mac_apcli, bc.mac_apcli, ETH_ALEN)) {
						debug(DEBUG_ERROR, "update for wifi case\n");
						found = 1;
						break;
					}
				}
				else if (os_memcmp(bc.almac, all_zero_mac, ETH_ALEN)) {
					if (!os_memcmp(bc_item->almac, bc.almac, ETH_ALEN)) {
						debug(DEBUG_ERROR, "update for eth case\n");
						found = 1;
						break;
					}
				}
			}

			/* update */
			if (found) {
				bc_item->bh_connector_len = bc.bh_connector_len;
				bc_item->fh_connector_len = bc.fh_connector_len;

				if (bc_item->bh_connector) {
					os_free(bc_item->bh_connector);
				}
				if (bc_item->fh_connector) {
					os_free(bc_item->fh_connector);
				}

				bc_item->bh_connector = os_zalloc(bc.bh_connector_len + 1);
				if (!bc_item->bh_connector) {
					return 0;
				}
				bc_item->fh_connector = os_zalloc(bc.fh_connector_len + 1);
				if (!bc_item->fh_connector) {
					os_free(bc_item->bh_connector);
					return 0;
				}

				os_memcpy(bc_item->bh_connector, bc.bh_connector, bc.bh_connector_len);
				os_memcpy(bc_item->fh_connector, bc.fh_connector, bc.fh_connector_len);
			}
			/* add new one */
			else {
				bc_item = os_zalloc(sizeof(struct bss_connector_item));
				if (!bc_item) {
					return 0;
				}
				bc_item->bh_connector = os_zalloc(bc.bh_connector_len + 1);
				if (!bc_item->bh_connector) {
					os_free(bc_item);
					return 0;
				}
				bc_item->fh_connector = os_zalloc(bc.fh_connector_len + 1);
				if (!bc_item->fh_connector) {
					os_free(bc_item->bh_connector);
					os_free(bc_item);
					return 0;
				}

				os_memcpy(bc_item->mac_apcli, (unsigned char *)bc.mac_apcli, ETH_ALEN);
				os_memcpy(bc_item->almac, (unsigned char *)bc.almac, ETH_ALEN);
				bc_item->bh_connector_len = bc.bh_connector_len;
				bc_item->fh_connector_len = bc.fh_connector_len;
				os_memcpy(bc_item->bh_connector, bc.bh_connector, bc.bh_connector_len);
				os_memcpy(bc_item->fh_connector, bc.fh_connector, bc.fh_connector_len);

				dl_list_add(&dpp->bss_connector_list, &bc_item->member);
			}


			if ((!os_memcmp(bc_item->almac, all_zero_mac, ETH_ALEN)) &&
				os_memcmp(bc_item->mac_apcli, all_zero_mac, ETH_ALEN)) {
				debug(DEBUG_ERROR, "save later for apcli "MACSTR"\n", MAC2STR(bc_item->mac_apcli));
			}
			else {
				map_dpp_save_bss_connector(bc_item);
			}
			debug(DEBUG_ERROR, "bh connetor:\n %s\n\n", bc_item->bh_connector);
			debug(DEBUG_ERROR, "fh connetor:\n %s\n\n", bc_item->fh_connector);
		}
		break;

		case LIB_SET_CHIRP_VALUE:
		{
			struct chirp_tlv_info *p_chirp_tlv = NULL;

			unsigned char *p = wapp_event->buffer;
			unsigned char *mac1 = p;
			unsigned char all_zero_mac[ETH_ALEN];
			os_memset(all_zero_mac, 0, sizeof(all_zero_mac));

			p += ETH_ALEN;

			p_chirp_tlv = (struct chirp_tlv_info *)p;

			if ((!os_memcmp(p_chirp_tlv->enrollee_mac, all_zero_mac, ETH_ALEN)) &&
				(!os_memcmp(mac1, all_zero_mac, ETH_ALEN))) {
				if (ctx->role == CONTROLLER) {
					debug(DEBUG_ERROR, "no apcli mac and almac specified\n");
					break;
				}
			}

			add_chirp_hash_list(ctx, p_chirp_tlv, mac1);
		}
		break;


		case LIB_AP_WF6_CAPABILITY:
		{
			struct ap_wf6_cap_roles *cap = NULL;

			cap = (struct ap_wf6_cap_roles *)wapp_event->buffer;
			update_ap_wf6_cap(ctx, cap);
		}
		break;

		case LIB_SET_GTK_REKEY_INTERVAL:
		{
			unsigned char *p = wapp_event->buffer;
			unsigned int interval = *((unsigned int *)p);

			wpa_set_gtk_rekey_interval(interval);
		}
		break;

		case LIB_SET_R3_ONBOARDING_TYPE:
		{
			struct r3_dpp_information *dpp = &ctx->r3_dpp;

			dpp->onboarding_type = wapp_event->buffer[0];
		}
		break;

#endif	// MAP_R3
#ifdef MAP_R3_DE
		case LIB_DEV_INVEN_TLV:
		{
			struct dev_inven *de = NULL;

			de = (struct dev_inven *)wapp_event->buffer;
			update_ap_dev_inven(ctx, de);
		}
		break;
#endif //MAP_R3_DE
#ifdef MAP_R3_SP
		case SERVICE_PRIORITIZATION_COMMAND:
		{
#ifdef MAP_R3
			struct sp_cmd *sp = NULL;
#endif
			debug(DEBUG_TRACE, "receive SERVICE_PRIORITIZATION_COMMAND\n");
			sp_process_1905_lib_cmd((void *)ctx, wapp_event->buffer, wapp_event->length, reply, reply_len);
#ifdef MAP_R3
			sp = (struct sp_cmd*)wapp_event->buffer;
			if (sp->cmd_id == SP_CMD_CONFIG_DONE)
				report_sp_standard_rule_to_mapd(ctx);
#endif
		}
		break;
#endif
	default:
		debug(DEBUG_ERROR, "unknow command(0x%04x)\n", wapp_event->type);
		break;
	}
	return 0;
}

int parse_combined_infra_metrics_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while(1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == AP_METRICS_TYPE) {
			integrity |= 0x1;
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == TRANSMITTER_LINK_METRIC_TYPE) {
			integrity |= 0x1;
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == RECEIVER_LINK_METRIC_TYPE) {
			integrity |= 0x1;
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == ERROR_CODE_TYPE) {
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 0x1) {
		debug(DEBUG_ERROR, "integrity check fail\n");
		return -1;
	}

	return 0;
}

int change_role_dynamic(struct p1905_managerd_ctx* ctx, unsigned char role)
{
	int j = 0, i = 0;

	if (ctx->current_autoconfig_info.radio_index != -1) {
		debug(DEBUG_ERROR, "auto configuration is ongoing; reject this operation\n");
		return -1;
	}


	if (ctx->role == CONTROLLER) {
		debug(DEBUG_ERROR, "cannot set controller to other role\n");
		return -1;
	}

	if (ctx->role == role) {
		debug(DEBUG_ERROR, "already set the role\n");
		return -1;
	}

	ctx->role = role;
	ctx->service = ctx->role;
	debug(DEBUG_OFF, "change role to %s\n", role? "agent":"controller");

	if (ctx->role == CONTROLLER) {
		for(i = 0; i < 2; i++) {
			ctx->mid++;
			for (j = 0; j < ctx->itf_number; j++) {
				debug(DEBUG_OFF, "send topology notification by interface[%s]\n", ctx->itf[j].if_name);
				insert_cmdu_txq(p1905_multicast_address, ctx->p1905_al_mac_addr,
						e_topology_notification, ctx->mid, ctx->itf[j].if_name, 0);
#ifdef SUPPORT_CMDU_RELIABLE
				cmdu_reliable_send(ctx, e_topology_notification, ctx->mid, j);
#endif
			}
		}
	}

	return 0;
}

int get_local_device_info(struct p1905_managerd_ctx* ctx, unsigned char *buf,
	unsigned int buf_len, unsigned short *len)
{
	unsigned char *p = NULL, *old_p = NULL;
	int i = 0;
	unsigned int expected_len = 0;

	/*pre-check length of buf to avoid memory overwritten*/
	expected_len += 15 + 8 * ctx->itf_number;
	for (i = 0; i < BRIDGE_NUM; i++) {
		expected_len += 2 + os_strlen((char*)ctx->br_name) +
			ETH_ALEN * ctx->br_cap[i].interface_num;
	}

	if (expected_len > buf_len)
		return -1;

	p = buf;
	*(unsigned short*)p = MANAGE_GET_LOCAL_DEVINFO;
	p += sizeof(unsigned short);

	/*mid*/
	p += sizeof(unsigned short);

	old_p = p;

	p += sizeof(unsigned short);
	/*device info*/
	memcpy(p, ctx->p1905_al_mac_addr, ETH_ALEN);
	p += ETH_ALEN;

	*p++ = (unsigned char)ctx->itf_number;

	for (i = 0; i < ctx->itf_number; i++) {
		memcpy(p, ctx->itf[i].mac_addr, ETH_ALEN);
		p += ETH_ALEN;
		memcpy(p, &(ctx->itf[i].media_type), sizeof(unsigned short));
		p += sizeof(unsigned short);
	}

	/*bridge capa*/
	*p++ = BRIDGE_NUM;
	for (i = 0; i < BRIDGE_NUM; i++) {
		*p++ = os_strlen((char*)ctx->br_name);
		os_strncpy((char*)p, (char*)ctx->br_name, os_strlen((char*)ctx->br_name));
		p += os_strlen((char*)ctx->br_name);
		*p++ = ctx->br_cap[i].interface_num;
		memcpy(p, ctx->br_cap[i].itf_mac_list, ctx->br_cap[i].interface_num * ETH_ALEN);
		p += ctx->br_cap[i].interface_num * ETH_ALEN;
	}

	/*supported service*/
	*p++ = 1;
	*p++ = ctx->service;

	*(unsigned short*)old_p = (unsigned short)(p - old_p - 2);
	*len = (unsigned short)(p - buf);

	return 0;
}

int parse_ap_capability_tlv(struct p1905_managerd_ctx *ctx,
				unsigned short len, unsigned char *value)
{
	if (len != 1)
		return -1;

	/* mapd parse */

	return 0;
}

int parse_ap_ht_capability_tlv(struct p1905_managerd_ctx *ctx,
				unsigned short len, unsigned char *value)
{
	if (len != 7)
		return -1;

	/* mapd parse */

	return 0;
}

int parse_ap_vht_capability_tlv(struct p1905_managerd_ctx *ctx,
				unsigned short len, unsigned char *value)
{
	if (len != 12)
		return -1;

	/* mapd parse */

	return 0;
}

int parse_ap_capability_report_message(struct p1905_managerd_ctx *ctx,
	unsigned char *almac, unsigned char *buf, unsigned int left_tlv_len,
	OUT struct agent_radio_info *r, OUT unsigned char *radio_cnt)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned int integrity = 0, all_integrity = 0xff;

	type = buf;
	reset_stored_tlv(ctx);
	*radio_cnt = 0;

	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "[%d] type = %d, tlv length > left_tlv_length\n", __LINE__, *type);
			return -1;
		}

		/* One AP Capability TLV */
		if (*type == AP_CAPABILITY_TYPE) {
			ret = parse_ap_capability_tlv(ctx, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error ap capability tlv\n");
				return -1;
			}
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* One or more AP Radio Basic Capabilities TLV */
		else if (*type == AP_RADIO_BASIC_CAPABILITY_TYPE) {
			os_memset(r, 0, sizeof(struct agent_radio_info));
			*radio_cnt += 1;
			if (*radio_cnt > MAX_RADIO_NUM) {
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX"error!!!Exceeds MAX_RADIO_NUM"
					" in ap radio basic cap tlv\n");
				return -1;
			}
			ret = parse_ap_radio_basic_cap_tlv(ctx, r++, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error ap radio basic cap tlv\n");
				return -1;
			}
			integrity |= (1 << 1);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* Zero or more AP HT Capabilities TLV */
		else if (*type == AP_HT_CAPABILITY_TYPE) {
			ret = parse_ap_ht_capability_tlv(ctx, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error ap ht capability tlv\n");
				return -1;
			}
			integrity |= (1 << 2);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* Zero or more AP VHT Capabilities TLV */
		else if (*type == AP_VHT_CAPABILITY_TYPE) {
			ret = parse_ap_vht_capability_tlv(ctx, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error ap vht capability tlv\n");
				return -1;
			}

			integrity |= (1 << 3);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* Zero or more AP HE Capabilities TLV */
		else if (*type == AP_HE_CAPABILITY_TYPE) {
			integrity |= (1 << 4);
			store_revd_tlvs(ctx, type, tlv_len);
		}
#ifdef MAP_R2
		/* One Channel Scan Capabilities TLV */
		else if (*type == CHANNEL_SCAN_CAPABILITY_TYPE) {
			integrity |= (1 << 5);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* One CAC Capabilities TLV */
		else if (*type == CAC_CAPABILITIES_TYPE) {
			integrity |= (1 << 6);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* One R2 AP Capability TLV */
		else if (*type == R2_AP_CAPABILITY_TYPE) {
			integrity |= (1 << 7);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* One Metric Collection Interval TLV */
		else if (*type == METRIC_COLLECTION_INTERVAL_TYPE) {
			integrity |= (1 << 8);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* skip: One Device 1905 Layer Security Capability TLV
		else if (*temp_buf == METRIC_COLLECTION_INTERVAL_TYPE) {
			integrity |= (1 << 2);
			length = get_tlv_len_by_tlvtype(temp_buf);
			store_revd_tlvs(ctx, temp_buf, length);
			temp_buf += length;
		}*/
#endif
#ifdef MAP_R3
		/* One Device 1905 Layer Security Capability TLV */
		else if (*type == _1905_LAYER_SECURITY_CAPABILITY_TYPE) {
			unsigned char onboardingProto = 0, msgIntAlg = 0, msgEncAlg = 0;

			integrity |= (1 << 9);
			ret = parse_1905_layer_security_capability_tlv(ctx, type, &onboardingProto, &msgIntAlg, &msgEncAlg);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error 1905layer security cap tlv\n");
				return -1;
			}
		} else if (*type == 0xAA) {
			integrity |= (1 << 10);
			ret = parse_ap_wf6_capability_tlv(ctx, type);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error ap wf6 capability tlv\n");
				return -1;
			}
			store_revd_tlvs(ctx, type, tlv_len);
		}
#endif // MAP_R3
#ifdef MAP_R3_DE
		else if (*type == R3_DE_INVENTORY_TLV_TYPE) {
			integrity |= (1 << 11);
			store_revd_tlvs(ctx, type, tlv_len);
		}
#endif //MAP_R3_DE
		else if (*type == END_OF_TLV_TYPE) {
			break;
		}

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
	/*check integrity*/
	if ((integrity & all_integrity) == 0) {
		debug(DEBUG_ERROR, "error when check ap capability report tlvs\n");
		return -1;
	}

	return 0;
}

int parse_channel_selection_rsp_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == CH_SELECTION_RESPONSE_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
#ifdef MAP_R4_SPT
		} else if (*type == SPATIAL_REUSE_CONFIG_RESP_TYPE) {
			integrity |= (1 << 1);
			store_revd_tlvs(ctx, type, tlv_len);
#endif
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity < 1) {
		debug(DEBUG_ERROR, "error channel selection response tlvs\n");
		return -1;
	}

	return 0;
}

int parse_operating_channel_report_message(struct p1905_managerd_ctx *ctx,
		unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned int tlv_len = 0;
	unsigned int integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

#ifdef SINGLE_BAND_SUPPORT
	if (*type != OPERATING_CHANNEL_REPORT_TYPE) {
		debug(DEBUG_ERROR, " Wifi Radio not present on remote device\n");
		return 0;
	}
#endif

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "[%d] type = %d, tlv length > left_tlv_length\n", __LINE__, *type);
			return -1;
		}

		if (*type == OPERATING_CHANNEL_REPORT_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
#ifdef MAP_R4_SPT
		} else if (*type == SPATIAL_REUSE_REPORT_TYPE) {
			integrity |= (1 << 1);
			store_revd_tlvs(ctx, type, tlv_len);
#endif
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
	/* check integrity */
	if (integrity < 1) {
		debug(DEBUG_ERROR, "error when check operating channel report tlvs\n");
		return -1;
	}
	return 0;
}

int parse_client_capability_report_message(struct p1905_managerd_ctx *ctx,
						unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned int integrity = 0;
	unsigned char temp_bssid[ETH_ALEN] = {0};
	unsigned char temp_sta[ETH_ALEN] = {0};

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = 0x%02x, left_tlv_length %d less than tlv_len %d\n",
				__LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}

		if (*type == CLIENT_INFO_TYPE) {
			ret = parse_client_info_tlv(ctx, temp_bssid, temp_sta, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error client info tlv\n");
				return -1;
			}
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == CLIENT_CAPABILITY_REPORT_TYPE) {
			integrity |= (1 << 1);
			store_revd_tlvs(ctx, type, tlv_len);
			break;
		} else if (*type == ERROR_CODE_TYPE) {
			ret = parse_error_code_tlv(ctx, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error in error code tlv\n");
				return -1;
			}
			store_revd_tlvs(ctx, type, tlv_len);
			break;
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
	/* check integrity */
	if (integrity != 3) {
		debug(DEBUG_ERROR, "error when check operating channel report tlvs\n");
		return -1;
	}
	return 0;
}

int parse_unassociated_sta_link_metrics_rsp_tlv(struct p1905_managerd_ctx *ctx,
	unsigned char *buf)
{
    unsigned char *temp_buf;
    unsigned short length = 0;

    temp_buf = buf;

    if((*temp_buf) == UNASSOC_STA_LINK_METRICS_RSP_TYPE) {
        temp_buf++;
    }
    else {
        return -1;
    }

    //calculate tlv length
    length = *(unsigned short *)temp_buf;
    length = be_to_host16(length);

    return (length+3);
}

int parse_unassoc_sta_link_metrics_response_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == UNASSOC_STA_LINK_METRICS_RSP_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 1) {
		debug(DEBUG_ERROR, "error unassoc sta link metrics response tlvs\n");
		return -1;
	}
	return 0;
}

int parse_beacon_metrics_rsp_tlv(struct p1905_managerd_ctx *ctx,
	unsigned char *buf)
{
    unsigned char *temp_buf;
    unsigned short length = 0;

    temp_buf = buf;

    if((*temp_buf) == BEACON_METRICS_RESPONSE_TYPE) {
        temp_buf++;
    }
    else {
        return -1;
    }

    //calculate tlv length
    length = *(unsigned short *)temp_buf;
    length = be_to_host16(length);

    return (length+3);
}

int parse_beacon_metrics_response_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == BEACON_METRICS_RESPONSE_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 1) {
		debug(DEBUG_ERROR, "error check beacon metrics response tlvs\n");
		return -1;
	}

	return 0;
}

int parse_client_steering_btm_report_message(struct p1905_managerd_ctx *ctx,
		unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == STEERING_BTM_REPORT_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 1) {
		debug(DEBUG_ERROR, "error when check client steering btm report tlvs\n");
		return -1;
	}
	return 0;
}

int parse_higher_layer_data_message(struct p1905_managerd_ctx *ctx,
					unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned int tlv_len = 0;
	unsigned int integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = 0x%02x, left_tlv_length %d less than tlv_len %d\n",
				__LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}

		if (*type == HIGH_LAYER_DATA_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 1) {
		debug(DEBUG_ERROR, "error when check higher layer data tlvs\n");
		return -1;
	}
	return 0;
}

int set_bss_config(struct p1905_managerd_ctx *ctx, enum BSS_CONFIG_OPERATION operation,
	struct bss_config_info* info)
{
	unsigned char i = 0, j = 0, k = 0;
	unsigned char band_mask = 0x01;
	struct config_info* conf = NULL;
	char opclass[3][4] = {"8x", "11x", "12x"};
	if (operation == BSS_ADD) {
		j = ctx->bss_config_num;
	}

	conf = info->info;
	for (i = 0; i < info->cnt; i++) {
		for (k = 0; k < 3; k ++) {
			if ((conf->band & (band_mask << k)) == 0x00) {
				continue;
			}
			memset(&(ctx->bss_config[j]), 0, sizeof(struct set_config_bss_info));
			memcpy(ctx->bss_config[j].mac, conf->almac, ETH_ALEN);
			ctx->bss_config[j].authmode = conf->auth_mode;
			ctx->bss_config[j].encryptype = conf->encry_type;
			ctx->bss_config[j].wfa_vendor_extension = conf->wfa_vendor_extension;
			ctx->bss_config[j].hidden_ssid = conf->hidden_ssid;
			(void)snprintf(ctx->bss_config[j].ssid, sizeof(ctx->bss_config[j].ssid), "%s", conf->ssid);
			memcpy(ctx->bss_config[j].key, conf->key, 65);
			memcpy(ctx->bss_config[j].oper_class, opclass[k], 4);
			debug(DEBUG_OFF, "add bss, mac="MACSTR", opclass=%s,"
				" ssid=%s, authmode=%04x, encrytype=%04x, key=%s, bh_bss=%s, fh_bss=%s"
				"hidden_ssid=%d\n",
				PRINT_MAC(ctx->bss_config[j].mac), ctx->bss_config[j].oper_class,
				ctx->bss_config[j].ssid, ctx->bss_config[j].authmode, ctx->bss_config[j].encryptype,
				ctx->bss_config[j].key,
				ctx->bss_config[j].wfa_vendor_extension & BIT_BH_BSS ? "1" : "0",
				ctx->bss_config[j].wfa_vendor_extension & BIT_FH_BSS ? "1" : "0",
				ctx->bss_config[j].hidden_ssid);
			j++;
		}
		conf++;
	}
	ctx->bss_config_num = j;
	debug(DEBUG_OFF, "total bss config number %d\n", ctx->bss_config_num);

	return 0;
}

void build_ap_metric_query_tlv(struct p1905_managerd_ctx *ctx)
{
	unsigned char buf[512] = {0}, bssid[512] = {0};
	unsigned char i = 0, j = 0, bssid_num = 0;
	unsigned short length = 0;
	unsigned char *pos = buf, *bss_pos = bssid;

#ifdef MAP_R2
	struct radio_identifier_db *p_tmp = NULL;

	unsigned char identifier[512] = {0};
	unsigned char id_total_len = 0;
	unsigned char *identifier_pos = identifier;
#endif
	reset_stored_tlv(ctx);

	for (i = 0; i < MAX_RADIO_NUM; i++) {
		for (j = 0; j < ctx->rinfo[i].bss_number; j++) {
			if (ctx->rinfo[i].bss[j].config_status) {
				memcpy(bss_pos, ctx->rinfo[i].bss[j].mac, ETH_ALEN);
				bss_pos += ETH_ALEN;
				bssid_num++;
			}
		}
	}

	/*tlvType*/
	*pos++ = AP_METRICS_QUERY_TYPE;
	/*tlvLength*/
	length = 1 + ETH_ALEN * bssid_num;
	*(unsigned short *)(pos) = host_to_be16(length);
	pos += 2;
	/*Number of BSSIDs included in this TLV*/
	*pos++ = bssid_num;
	/*BSSID of a BSS*/
	memcpy(pos, bssid, length-1);

	store_revd_tlvs(ctx, buf, length + 3);
#ifdef MAP_R2
	/* Zero or more AP Radio Identifier TLVs */
	if(!SLIST_EMPTY(&ctx->metric_entry.radio_identifier_head))
	{
		p_tmp = SLIST_FIRST(&ctx->metric_entry.radio_identifier_head);
		while (p_tmp)
		{
			debug(DEBUG_TRACE, "radio identifier("MACSTR") \n", PRINT_MAC(p_tmp->identifier));

			/*tlvType*/
			*identifier_pos++ = AP_RADIO_IDENTIFIER_TYPE;
			/*tlvLength*/
			length = ETH_ALEN;
			*(unsigned short *)(identifier_pos) = host_to_be16(length);
			identifier_pos += 2;
			/*radio identifier*/
			memcpy(identifier_pos, p_tmp->identifier, ETH_ALEN);

			id_total_len += (length + 3);
			identifier_pos += ETH_ALEN;
			p_tmp = SLIST_NEXT(p_tmp, radio_identifier_entry);
		}
	}
	store_revd_tlvs(ctx, identifier, id_total_len);
#endif // MAP_R2
}

void report_own_topology_rsp(struct p1905_managerd_ctx *ctx,
	unsigned char *al_mac, struct bridge_capabiltiy *br_cap_list,
	struct p1905_neighbor *p1905_dev, struct non_p1905_neighbor *non_p1905_dev,
	unsigned char service, struct list_head_oper_bss *opbss_head,
    struct list_head_assoc_clients *asscli_head, unsigned int cnt)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;
	int i = 0;
	unsigned char separate = 0;

    length = append_device_info_type_tlv(tlv_temp_buf, al_mac, ctx);
    total_tlvs_length += length;
    tlv_temp_buf += length;

    length = append_device_bridge_capability_type_tlv(tlv_temp_buf, br_cap_list);
    total_tlvs_length += length;
    tlv_temp_buf += length;

	for (i = 0; i < ctx->itf_number; i++) {
		do {
			length = append_p1905_neighbor_device_type_tlv(tlv_temp_buf, p1905_dev, i, &separate);
			total_tlvs_length += length;
			tlv_temp_buf += length;
		} while (separate);
	}

	separate = 0;
	/*append non-p1905.1 device tlv*/
	for (i = 0; i < ctx->itf_number; i++) {
		do {
			length = append_non_p1905_neighbor_device_type_tlv(tlv_temp_buf,
				non_p1905_dev, i, &separate);
			total_tlvs_length += length;
			tlv_temp_buf += length;
		} while (separate);
	}

	length = append_supported_service_tlv(tlv_temp_buf, service);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_operational_bss_tlv(tlv_temp_buf, opbss_head);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_associated_clients_tlv(tlv_temp_buf, asscli_head, cnt);
	total_tlvs_length += length;
	tlv_temp_buf += length;

#ifdef MAP_R4_SPT
	if (ctx->map_version >= DEV_TYPE_R3) {
		length = append_map_version_tlv(tlv_temp_buf, ctx->map_version);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}
#endif

	if (total_tlvs_length > 10240) {
		debug(DEBUG_ERROR, "send length(%d) exceeds max revd_tlv len\n",
			total_tlvs_length);
		return;
	}
	reset_stored_tlv(ctx);
	store_revd_tlvs(ctx, ctx->tlv_buf.buf, total_tlvs_length);
	os_get_time(&ctx->own_topo_rsp_update_time);
	_1905_notify_topology_rsp_event(ctx, ctx->p1905_al_mac_addr, ctx->p1905_al_mac_addr);

}

int read_bss_conf_and_renew_v2(struct p1905_managerd_ctx *ctx, unsigned char local_only) {
	/*update bss setting into ctx->bss_config*/
	if (_1905_read_set_config(ctx, ctx->wts_bss_cfg_file, ctx->bss_config, MAX_SET_BSS_INFO_NUM, &ctx->bss_config_num) < 0 ||
		(ctx->bss_config_num == 0)) {
		debug(DEBUG_ERROR, "read wts_bss_info_config fail or no valid bss info\n");
		return -1;
	}
	ctx->renew_ctx.trigger_renew = 1;
	ctx->renew_state = wait_4_dump_topo;
	eloop_register_timeout(0, 0, controller_renew_bss_step, (void *)ctx, NULL);

	return 0;
}


int read_bss_conf_and_renew(struct p1905_managerd_ctx *ctx, unsigned char local_only)
{
	int i = 0;
	unsigned char band = 0;
	unsigned char buffer[1514] = {0};

	/*update bss setting into ctx->bss_config*/
	debug(DEBUG_OFF, "read wts_bss_info_config to ctx->bss_config, radio_number=%d\n",
		ctx->radio_number);
	if (_1905_read_set_config(ctx, ctx->wts_bss_cfg_file, ctx->bss_config, MAX_SET_BSS_INFO_NUM, &ctx->bss_config_num) < 0 ||
		(ctx->bss_config_num == 0)) {
		debug(DEBUG_ERROR, "read wts_bss_info_config fail or no valid bss info\n");
		return -1;
	}

	for (i = 0; i < ctx->radio_number; i++) {
		_1905_update_bss_info_per_radio(ctx, &ctx->rinfo[i]);
	}
#ifdef MAP_R2
	eloop_register_timeout(3, 0, map_r2_notify_ts_config, (void *)ctx, NULL);
	eloop_register_timeout(0, 0, map_notify_transparent_vlan_setting, (void *)ctx, NULL);
#endif


	if (!local_only) {
			band = 0;
			ctx->mid++;
			if (fill_send_tlv(ctx, &band, 1) < 0) {
				return 0;
			}

			for(i = 0; i < ctx->itf_number; i++) {
				insert_cmdu_txq(p1905_multicast_address, ctx->p1905_al_mac_addr,
					e_ap_autoconfiguration_renew, ctx->mid, ctx->itf[i].if_name, 0);
				debug(DEBUG_TRACE, "insert renew message for %s band(%s)\n",ctx->itf[i].if_name, "2g");
#ifdef SUPPORT_CMDU_RELIABLE
				cmdu_reliable_send(ctx, e_ap_autoconfiguration_renew, ctx->mid, i);
#endif
			}
			/*handle the txq before insert another band renew message*/
			process_cmdu_txq(ctx, buffer);

			band = 1;
			ctx->mid++;
			if (fill_send_tlv(ctx, &band, 1) < 0) {
				return 0;
			}

			for(i = 0; i < ctx->itf_number; i++) {
				insert_cmdu_txq(p1905_multicast_address, ctx->p1905_al_mac_addr,
					e_ap_autoconfiguration_renew, ctx->mid, ctx->itf[i].if_name, 0);
				debug(DEBUG_TRACE, "insert renew message for %s band(%s)\n",ctx->itf[i].if_name, "5g");
#ifdef SUPPORT_CMDU_RELIABLE
				cmdu_reliable_send(ctx, e_ap_autoconfiguration_renew, ctx->mid, i);
#endif
			}
			process_cmdu_txq(ctx, buffer);
	}
	return 0;
}

unsigned char determin_band_config(struct p1905_managerd_ctx *ctx, unsigned char *almac,
	unsigned char band_cap)
{
	unsigned char i = 0;
	unsigned char bss_cap = 0;
	unsigned char wild_card_mac[ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	for(i = 0; i < ctx->bss_config_num; i++) {
		if(!os_memcmp(almac, ctx->bss_config[i].mac, ETH_ALEN) ||
		   !os_memcmp(wild_card_mac, ctx->bss_config[i].mac, ETH_ALEN)) {
			bss_cap |= ctx->bss_config[i].band_support;
		}
	}
	debug(DEBUG_TRACE, AUTO_CONFIG_PREX"agent("MACSTR") bsscap = %02x\n",
		PRINT_MAC(almac), bss_cap);

	if (band_cap & bss_cap & BAND_2G_CAP) {
		return BAND_2G_CAP;
	} else if (band_cap & bss_cap & BAND_6G_CAP) {
		return BAND_6G_CAP;
	} else if (band_cap & bss_cap & BAND_5GL_CAP) {
		return BAND_5GL_CAP;
	} else if (band_cap & bss_cap & BAND_5GH_CAP) {
		return BAND_5GH_CAP;
	} else {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"bandcap=%02x, bsscap=%02x cannot decide which band to config\n",
			band_cap, bss_cap);
		return BAND_INVALID_CAP;
	}
}

WIFI_UTILS_STATUS config_bss_by_band(struct p1905_managerd_ctx *ctx, unsigned char band,
	WSC_CONFIG *wsc, unsigned char *wfa_vendor_extension)
{
	unsigned char i = 0;
	unsigned char wild_card_mac[ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	os_memset(wsc, 0, sizeof(WSC_CONFIG));
	for (i = 0; i < ctx->bss_config_num; i++) {
		debug(DEBUG_INFO, AUTO_CONFIG_PREX"enrolle_mac("MACSTR")\n",
			PRINT_MAC(ctx->ap_config.enrolle_mac));
		debug(DEBUG_INFO, AUTO_CONFIG_PREX"bss_mac("MACSTR")\n",
			PRINT_MAC(ctx->bss_config[i].mac));
		debug(DEBUG_INFO, AUTO_CONFIG_PREX"band=%02x, \n", band);
		/*2.4g/5g band*/
		if (!(ctx->bss_config_bitmap[i/(sizeof(int)*8)] & BIT(i%(sizeof(int)*8))) &&
			band == ctx->bss_config[i].band_support) {
			if (!os_memcmp(ctx->ap_config.enrolle_mac, ctx->bss_config[i].mac, ETH_ALEN) ||
				!os_memcmp(wild_card_mac, ctx->bss_config[i].mac, ETH_ALEN)) {
				ctx->bss_config_bitmap[i/(sizeof(int)*8)] |= BIT(i%(sizeof(int)*8));
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX"index=%d " MACSTR " ssid=%s\n", i,
					PRINT_MAC(ctx->bss_config[i].mac), ctx->bss_config[i].ssid);
				break;
			}
		}

	}

	if (i >= ctx->bss_config_num) {
		if (!ctx->bss_config_bitmap[0] && !ctx->bss_config_bitmap[1] &&
			!ctx->bss_config_bitmap[2] && !ctx->bss_config_bitmap[3]) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"[%d]can not get any wsc config(almac="MACSTR"), no bss config from file\n",
				i, PRINT_MAC(ctx->ap_config.enrolle_mac));
			debug(DEBUG_ERROR, "error!!! cannot happen!!! no bss info for band=%02x! send tear down bit in m2\n", band);
			*wfa_vendor_extension = 0x10;
			goto fail;
		} else {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"no enough bss info for band=%02x! not add extra m2 now\n", band);
			goto fail;
		}
	}

	memcpy(wsc->Ssid, ctx->bss_config[i].ssid, 32);
	wsc->EncrypType = ctx->bss_config[i].encryptype;
	wsc->AuthMode = ctx->bss_config[i].authmode;
	memcpy(wsc->WPAKey, ctx->bss_config[i].key, 64);
	*wfa_vendor_extension = ctx->bss_config[i].wfa_vendor_extension;
	wsc->hidden_ssid = ctx->bss_config[i].hidden_ssid;
	return wifi_utils_success;
fail:
	return wifi_utils_error;
}

void init_radio_info_by_intf(struct p1905_managerd_ctx* ctx, struct p1905_interface *itf)
{
	unsigned char i = 0, j = 0;
	unsigned int k = 0;

	for (i = 0; i < ctx->radio_number; i++) {
		if (!os_memcmp(ctx->rinfo[i].identifier, itf->identifier, ETH_ALEN)) {
			for (j = 0; j < ctx->rinfo[i].bss_number; j++) {
				if (!os_memcmp(ctx->rinfo[i].bss[j].ifname, itf->if_name, IFNAMSIZ)) {
					break;
				}
			}
			if (j >= ctx->rinfo[i].bss_number) {
				/*new bss, should add new one*/
				(void)os_snprintf((char *)ctx->rinfo[i].bss[ctx->rinfo[i].bss_number].ifname,
					sizeof(ctx->rinfo[i].bss[ctx->rinfo[i].bss_number].ifname),
					"%s", itf->if_name);
				os_memcpy(ctx->rinfo[i].bss[ctx->rinfo[i].bss_number].mac, itf->mac_addr, ETH_ALEN);
				ctx->rinfo[i].bss[ctx->rinfo[i].bss_number].priority= 0;
				debug(DEBUG_TRACE, "add new bss(%s) to existed radio("MACSTR")\n",
					itf->if_name, PRINT_MAC(itf->identifier));
				ctx->rinfo[i].bss_number++;
			}
			break;
		}
	}

	if (ctx->radio_number == MAX_RADIO_NUM) {
		debug(DEBUG_ERROR, "Error! radio number exceeds maximum(%d)("MACSTR")\n",
			MAX_RADIO_NUM, PRINT_MAC(itf->identifier));
		return;
	}

	/*new radio, add new one*/
	if (i >= ctx->radio_number) {
		os_memcpy(ctx->rinfo[ctx->radio_number].identifier, itf->identifier, ETH_ALEN);
		ctx->rinfo[ctx->radio_number].bss_number = 1;
		debug(DEBUG_TRACE, "add new radio("MACSTR")\n", PRINT_MAC(itf->identifier));
		(void)os_snprintf((char *)ctx->rinfo[ctx->radio_number].bss[0].ifname,
			sizeof(ctx->rinfo[ctx->radio_number].bss[0].ifname), "%s", itf->if_name);
		os_memcpy(ctx->rinfo[ctx->radio_number].bss[0].mac, itf->mac_addr, ETH_ALEN);
		ctx->rinfo[ctx->radio_number].bss[0].priority= 0;
		dl_list_init(&ctx->rinfo[ctx->radio_number].basic_cap.op_class_list);
		debug(DEBUG_TRACE, "add new bss(%s) to above new radio\n", ctx->rinfo[ctx->radio_number].bss[0].ifname);
		ctx->radio_number++;
	}

	/*set the priority to the bss that will be configured*/
	for (i = 0; i < ctx->radio_number; i++) {
		if (!os_memcmp(ctx->rinfo[i].identifier, itf->identifier, ETH_ALEN)) {
			for (j = 0; j < ctx->rinfo[i].bss_number; j++) {
				for (k = 0; k < ctx->itf_number; k++) {
					/*if (os_strcmp((char*)ctx->rinfo[i].bss[j].ifname, (char*)ctx->itf[k].if_name) == 0) {*/
					if (os_memcmp((char*)ctx->rinfo[i].bss[j].mac, (char*)ctx->itf[k].mac_addr, ETH_ALEN) == 0) {
						ctx->rinfo[i].bss[j].priority = ctx->itf[k].config_priority;
					}
				}
			}
			break;
		}
	}
}

void metrics_report_timeout(void *eloop_data, void *user_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)eloop_data;

	/*build ap metric query tlv*/
	debug(DEBUG_TRACE, "send automatic ap metrics response(report_interval) to "MACSTR"\n",
		PRINT_MAC(ctx->cinfo.almac));
	build_ap_metric_query_tlv(ctx);

	_1905_notify_ap_metrics_query(ctx, ctx->cinfo.almac, 0
#ifdef MAP_R2
	, 1
#endif
	);
	common_process(ctx, &ctx->trx_buf);

	eloop_register_timeout(ctx->map_policy.mpolicy.report_interval, 0,
		metrics_report_timeout, eloop_data, NULL);
}

unsigned short create_vs_info_for_specific_discovery(unsigned char *vs_info)
{
	/*1905 internal using vendor specific tlv format
	   type			1 byte			0x0b
	   length			2 bytes			0x, 0x
	   oui			3 bytes			0x00, 0x0C, 0xE7
	   function type	1 byte			0xff
	   suboui			3 bytes			0x00, 0x0C, 0xE7
	   sub func type	1 byte			0x
	*/
	unsigned char *p = vs_info;
	unsigned short len = 0;
	unsigned char mtk_oui[3] = {0x00, 0x0C, 0xE7};

	/*vendor specific tlv type*/
	*p++ = 11;

	/*vendor specific tlv length*/
	*((unsigned short*)p) = host_to_be16(8);
	p += 2;

	/*oui*/
	os_memcpy(p, mtk_oui, sizeof(mtk_oui));
	p += 3;

	/*function type*/
	*p++ = 0xff;

	/*additional OUI*/
	os_memcpy(p, mtk_oui, sizeof(mtk_oui));
	p += 3;

	/*1905 internal used vendor specific subtype*/
	*p++ = internal_vendor_discovery_message_subtype;

	len = (unsigned short)(p - vs_info);

	return len;
}

void discovery_at_interface_link_up(void *eloop_data, void *user_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_data;
	unsigned char *if_name = (unsigned char *)user_ctx;
	static int i = 3, j = 10;

	/*send multicast cmdu to all the ethenet interface */
	insert_cmdu_txq(p1905_multicast_address, ctx->p1905_al_mac_addr,
		e_topology_discovery_with_vendor_ie, ++ctx->mid, if_name, 0);
	debug(DEBUG_ERROR, TOPO_PREX"tx multicast discovery(mid-%04x) with vs tlv on %s\n",
		ctx->mid, if_name);

	/*in certification mode, cover TB Mar* issue
	* send more discovery to it and wait for it response
	*/
	if (ctx->MAP_Cer) {
		j--;
		if (j == 0) {
			j = 10;
			return;
		}
		eloop_register_timeout(3, 0, discovery_at_interface_link_up, (void *)ctx, (void *)if_name);
	} else {
		i--;
		if (i == 0) {
			i = 3;
			return;
		}
		eloop_register_timeout(3, 0, discovery_at_interface_link_up, (void *)ctx, (void *)if_name);
	}

	return;
}

void attach_action(void *eloop_data, void *user_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_data;
	struct topology_discovery_db *tpg_db = NULL;
	struct topology_response_db *tpgr_db = NULL;
	static int i = 2;

	report_own_topology_rsp(ctx, ctx->p1905_al_mac_addr, ctx->br_cap,
				ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
				ctx->service, &ctx->ap_cap_entry.oper_bss_head,
				&ctx->ap_cap_entry.assoc_clients_head, ctx->cnt);


    LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry) {
		insert_cmdu_txq(tpg_db->al_mac, ctx->p1905_al_mac_addr,
			e_topology_query, ++ctx->mid, ctx->br_name, 0);
    }

	SLIST_FOREACH(tpgr_db, &ctx->topology_entry.tprdb_head, tprdb_entry) {
		insert_cmdu_txq(tpgr_db->al_mac_addr, ctx->p1905_al_mac_addr,
			e_topology_query, ++ctx->mid, ctx->br_name, 0);
	}

	i--;

	if (i == 0) {
		i = 2;
		return;
	}

	eloop_register_timeout(1, 0, attach_action, (void *)ctx, NULL);
}


/*int check_send_rsp(struct p1905_managerd_ctx *ctx)
{
	int i = 0;
	unsigned char zero_mac[ETH_ALEN] = {0};
	struct topology_discovery_db *tpg_db = NULL;

	if (ctx->role == CONTROLLER)
		return 1;

	for (i = 1; i < ctx->itf_number; i++) {
		if ((ctx->itf[i].is_wifi_sta == 1) &&
			os_memcmp(ctx->itf[i].vs_info, zero_mac, ETH_ALEN)) {
			LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry)
			{
				if (!os_memcmp(tpg_db->itf_mac, ctx->itf[i].vs_info, ETH_ALEN))
					break;
			}
			if (!tpg_db) {
				debug(DEBUG_OFF, "apcli(%02x:%02x:%02x:%02x:%02x:%02x) "
					"connect to bssid(%02x:%02x:%02x:%02x:%02x:%02x)\n"
					"but not update neighbor! cannot tx rsp\n",
					PRINT_MAC(ctx->itf[i].mac_addr),
					PRINT_MAC(ctx->itf[i].vs_info));
				return 0;
			}
		}
	}

	return 1;
}


int check_valid_neighbor_rsp(struct p1905_managerd_ctx *ctx,
	struct topology_response_db *tpgr_db)
{
    struct device_info_db *dev = NULL;
	struct p1905_neighbor_device_db *neighbor = NULL;
	int rcv_id = ctx->recent_cmdu_rx_if_number;
	int i = 0, found_neighbor = 0, found_dev = 0;
	unsigned char zero_mac[ETH_ALEN] = {0};

	SLIST_FOREACH(dev, &tpgr_db->devinfo_head, devinfo_entry)
	{
		SLIST_FOREACH(neighbor, &dev->p1905_nbrdb_head, p1905_nbrdb_entry)
		{
			if (!os_memcmp(neighbor->p1905_neighbor_al_mac, ctx->p1905_al_mac_addr,
					ETH_ALEN)) {
				for (i = 1; i < ctx->itf_number; i++) {
					if ((ctx->itf[i].is_wifi_sta == 1) &&
						!os_memcmp(ctx->itf[i].vs_info, dev->mac_addr, ETH_ALEN))
						break;
				}
				if (i >= ctx->itf_number)
					return 0;
				break;
			}
		}
	}

	for (i = 1; i < ctx->itf_number; i++) {
		found_neighbor = 0;
		found_dev = 0;
		if ((ctx->itf[i].is_wifi_sta == 1) &&
			os_memcmp(ctx->itf[i].vs_info, zero_mac, ETH_ALEN)) {
			SLIST_FOREACH(dev, &tpgr_db->devinfo_head, devinfo_entry)
			{
				SLIST_FOREACH(neighbor, &dev->p1905_nbrdb_head, p1905_nbrdb_entry)
				{
					if (!os_memcmp(neighbor->p1905_neighbor_al_mac,
							ctx->p1905_al_mac_addr, ETH_ALEN)) {
							found_neighbor = 1;
							break;
					}
				}
				if (found_neighbor) {
					if (!os_memcmp(dev->mac_addr, ctx->itf[i].vs_info, ETH_ALEN)) {
						found_dev = 1;
						break;
					}
				}
			}
			if (!found_dev)
				return 0;
		}
	}

	return 1;
}

 struct topology_response_db * find_neighbor_topology_rsp(
 	struct p1905_managerd_ctx *ctx, unsigned char *al_mac)
{
	struct topology_discovery_db *tpg_db = NULL;
    struct topology_response_db *tpgr_db = NULL;

	LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry)
	{
		if (!os_memcmp(tpg_db->al_mac, al_mac ETH_ALEN))
			break;
    }
	if (!tpg_db) {
		return NULL;
	}

	SLIST_FOREACH(tpgr_db, &ctx->topology_entry.tprdb_head, tprdb_entry)
	{
		if (!os_memcmp(tpgr_db->al_mac_addr, al_mac, ETH_ALEN))
			return tpgr_db;
	}

	return NULL;
}

int update_own_neighbor_info_by_rsp(struct p1905_managerd_ctx *ctx,
	struct topology_response_db *tpgr_db)
{
	struct topology_discovery_db *tpg_db = NULL, *tpg_db_tmp;
    struct device_info_db *dev = NULL;
	struct p1905_neighbor_device_db *neighbor = NULL;
	unsigned char report = 0;

	LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry)
	{
		if (!os_memcmp(tpg_db->al_mac, tpgr_db->al_mac_addr, ETH_ALEN)) {
			SLIST_FOREACH(dev, &tpgr_db->devinfo_head, devinfo_entry)
			{
				SLIST_FOREACH(neighbor, &dev->p1905_nbrdb_head, p1905_nbrdb_entry)
				{
					if (!os_memcmp(neighbor->p1905_neighbor_al_mac,
							ctx->p1905_al_mac_addr, ETH_ALEN) &&
						!os_memcmp(dev->mac_addr, tpg_db->itf_mac, ETH_ALEN)) {
						break;
					}
				}
			}
			if (!dev) {
				tpg_db_tmp = LIST_NEXT(tpg_db, tpddb_entry);
				delete_p1905_neighbor_dev_info(ctx, tpg_db->al_mac,
					(unsigned char *)tpg_db->receive_itf_mac);
				LIST_REMOVE(tpg_db, tpddb_entry);
				free(tpg_db);
				tpg_db = tpg_db_tmp;
				report = 1;
			}
		}
	}

	if (report)
		report_own_topology_rsp(ctx, ctx->p1905_al_mac_addr, ctx->br_cap,
			ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
			ctx->service, &ctx->ap_cap_entry.oper_bss_head,
			&ctx->ap_cap_entry.assoc_clients_head, ctx->cnt);
}*/

unsigned short create_vs_info_for_specific_notification(
		struct p1905_managerd_ctx *ctx, unsigned char *vs_info)
{
	/*1905 internal using vendor specific tlv format
	   type 		1 byte			0x11
	   length			2 bytes 		0x, 0x
	   oui			3 bytes 		0x00, 0x0C, 0xE7
	   function type	1 byte			0xff
	   suboui			3 bytes 		0x00, 0x0C, 0xE7
	   sub func type	1 byte			0x01
	   sub len			2 byte
	   sub value		n byte
	*/
	unsigned char *p = vs_info;
	unsigned short len = 0;
	unsigned char mtk_oui[3] = {0x00, 0x0C, 0xE7};

	/*vendor specific tlv type*/
	*p++ = 11;

	/*vendor specific tlv length*/
	*((unsigned short*)p) = host_to_be16(16);
	p += 2;

	/*oui*/
	os_memcpy(p, mtk_oui, sizeof(mtk_oui));
	p += 3;

	/*function type*/
	*p++ = 0xff;

	/*additional OUI*/
	os_memcpy(p, mtk_oui, sizeof(mtk_oui));
	p += 3;

	/*1905 internal used vendor specific subtype*/
	*p++ = internal_vendor_notification_message_subtype;
	*((unsigned short*)p) = host_to_be16(ETH_ALEN);
	p += 2;
	os_memcpy(p, ctx->send_tlv, ETH_ALEN);
	p += ETH_ALEN;

	len = (unsigned short)(p - vs_info);

	return len;
}

void reset_controller_connect_ifname(struct p1905_managerd_ctx *ctx,
	unsigned char *link_down_ifname)
{
	int i = 0;

	if (ctx->role == CONTROLLER)
		return;

	if (os_memcmp(link_down_ifname, ctx->cinfo.local_ifname, IFNAMSIZ))
		return;

	for (i = 0; i < ctx->itf_number; i++) {
		if (ctx->itf[i].is_wifi_sta &&
			os_memcmp(ctx->itf[i].if_name, ctx->cinfo.local_ifname, IFNAMSIZ) &&
			ctx->itf[i].trx_config & (TX_MUL | TX_UNI)) {
			debug(DEBUG_OFF, "local_ifname[%s] link down use another one[%s]\n",
				ctx->cinfo.local_ifname, ctx->itf[i].if_name);
			os_memcpy(ctx->cinfo.local_ifname, ctx->itf[i].if_name, IFNAMSIZ);
			return;
		}
	}
}

int append_map_version_tlv(unsigned char *pkt, unsigned char version)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;

	temp_buf = pkt;
	total_length = 4;
	(*temp_buf) = MULTI_AP_VERSION_TYPE;
	*(unsigned short *)(temp_buf+1) = host_to_be16(total_length-3);
	*(temp_buf+3) = version;

	return total_length;
}

#ifdef MAP_R2
int parse_cac_status_report_type_tlv(struct p1905_managerd_ctx *ctx, unsigned char *buf)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == CAC_STATUS_REPORT_TYPE) {
	    debug(DEBUG_WARN, "cac status report type:%02x \n", *temp_buf);
	    temp_buf++;
	}
	else {
	    return -1;
	}

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	temp_buf += 2;

	debug(DEBUG_WARN, "cac status report type len:%d \n", length);

	return (length+3);
}



int append_r2_cap_tlv(unsigned char *pkt, struct ap_r2_capability *r2_cap)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;

	temp_buf = pkt;
	(*temp_buf) = R2_AP_CAPABILITY_TYPE;
	temp_buf += 3;

	/*
	1. Set the Max Number Service Prioritization Rules in the R2 AP Capability TLV to 6 or greater.
	2. If the Multi-AP R2 Agent Advanced Service Prioritization Capability fields are included,
		the Multi-AP R2 Agent shall set the Max Number Advance Service Prioritization Rules
		in the R2 AP Capability TLV to 0 or greater.
	3. Set the Max Number of Permitted Destination MAC Addresses field to 1 or greater and
		shall support Packet Filtering using up to that number of Permitted Destination MAC Addresses.
	4. Set the Byte Counter Units field to 0x01 (KiB (kibibytes)).
	5. Set the Max Total Number of VIDs field to 2 or greater.
	6. Set the Number of Ethernet Interfaces that can be configured as Edge field to 0 or
		greater and add an Interface ID field for each logical Ethernet interface that
		can be configured as Edge
	*/

	*temp_buf = r2_cap->max_total_num_sp_rules;
	temp_buf += 2;	/*sizeof(max_total_num_sp_rules) + sizeof(reserved1)*/

	/* Byte Counter Units: bits 7-6
	**	0: bytes
	**	1: kibibytes (KiB)
	**	2: mebibytes (MiB)
	**	3: reserved
	*/
	*temp_buf = 0;
	debug(DEBUG_OFF, "r2_cap->byte_counter_units %d\n", r2_cap->byte_counter_units);
	if (r2_cap->byte_counter_units == 1)
		*temp_buf |= 0x40;
	else if (r2_cap->byte_counter_units == 2)
		*temp_buf |= 0x80;

	if (r2_cap->sp_flag == 1)
		*temp_buf |= 0x20;
	if (r2_cap->dpp_flag == 1)
		*temp_buf |= 0x10;
	if (r2_cap->ts_flag == 1)
		*temp_buf |= 0x08;

	temp_buf += 1;

	*temp_buf++ = r2_cap->max_total_num_vid;

	total_length = (unsigned short)(temp_buf - pkt);

	*(unsigned short *)(pkt + 1) = host_to_be16(total_length - 3);

	return total_length;
}

int append_metric_collection_interval_tlv(unsigned char *pkt, unsigned int interval)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;

	temp_buf = pkt;
	total_length = sizeof(unsigned int) + 3;
	(*temp_buf) = METRIC_COLLECTION_INTERVAL_TYPE;
	*(unsigned short *)(temp_buf+1) = host_to_be16(sizeof(unsigned int));
	temp_buf += 3;
	*(unsigned int *)(temp_buf) = host_to_be32(interval);

	return total_length;
}

int append_default_8021Q_tlv(struct p1905_managerd_ctx *ctx,
	unsigned char *pkt, unsigned char *al_mac,
	unsigned char bss_num, struct set_config_bss_info *bss_info)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;
	unsigned char i = 0;
	unsigned short pvid = 0;
	unsigned char pcp = 0;

	if (ctx->MAP_Cer) {
		for (i = 0; i < MAX_SET_BSS_INFO_NUM; i ++) {
			if (memcmp(al_mac, bss_info[i].mac, ETH_ALEN))
				continue;

			if (bss_info[i].pvid)
				break;
		}

		if (i >= bss_num)
			return 0;

		pvid = bss_info[i].pvid;
		pcp = bss_info[i].pcp;
	} else {
		pvid = ctx->map_policy.setting.primary_vid;
		pcp = ctx->map_policy.setting.dft.PCP;
	}

	if (pvid == INVALID_VLAN_ID)
		return 0;

	temp_buf = pkt;
	*temp_buf++ = DEFAULT_8021Q_SETTING_TYPE;
	*(unsigned short *)temp_buf = host_to_be16(3);
	temp_buf += 2;

	*(unsigned short *)temp_buf = host_to_be16(pvid);
	temp_buf +=2;

	*temp_buf++ = (pcp << 5) & 0xE0;

	total_length = temp_buf - pkt;
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

	return total_length;
}


int append_traffic_separation_tlv(struct p1905_managerd_ctx *ctx,
	unsigned char *pkt, unsigned char *al_mac,
	unsigned char bss_num, struct set_config_bss_info *bss_info)
{
	unsigned char *temp_buf = pkt;
	unsigned short total_length = 0;
	unsigned char i = 0, ssid_num = 0;

	debug(DEBUG_TRACE, "al_mac "MACSTR"\n", PRINT_MAC(al_mac));

	temp_buf += 3;

	/* skip ssid num 1 byte */
	temp_buf ++;

	for (i = 0; i < ctx->bss_config_num; i++) {
		if (ctx->MAP_Cer) {
			if (memcmp(al_mac, bss_info[i].mac, ETH_ALEN))
				continue;
		}
		if (bss_info[i].vid == INVALID_VLAN_ID)
			continue;
		debug(DEBUG_OFF, "bss[%d] mac "MACSTR", vid %d, ssid %s, ssid_len %d\n"
			, i, PRINT_MAC(bss_info[i].mac), bss_info[i].vid, bss_info[i].ssid, bss_info[i].ssid_len);

		ssid_num ++;

		/* ssid len 1 byte */
		*temp_buf++ = bss_info[i].ssid_len;

		/* ssid */
		memcpy(temp_buf, bss_info[i].ssid, bss_info[i].ssid_len);
		temp_buf += bss_info[i].ssid_len;

		/* vlan id */
		*(unsigned short *)temp_buf = host_to_be16(bss_info[i].vid);
		temp_buf += 2;
	}

	if (ssid_num) {
		total_length = temp_buf - pkt;

		*pkt = TRAFFIC_SEPARATION_POLICY_TYPE;

		/* ts tlv len */
		*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

		/* ssid num */
		*(pkt+3) = ssid_num;


	}
	/* fix cross-vendor test issue with Broadcom. Broadcom will re-onboarding after policy config
	** Broadcom will remove there policy after receiving M2 with ssid num == 0
	** only use policy config req to remove policy
	** so if ssid num ==0, we don't need to add this tlv in M2
	*/
	else
		debug(DEBUG_OFF, "no ssid need to be configured, don't need to add TS tlv in M2\n");


	return total_length;
}

int append_transparent_vlan_tlv(struct p1905_managerd_ctx *ctx, unsigned char *pkt)
{
	/*1905 internal using vendor specific tlv format
	   type			1 byte			0x11
	   length			2 bytes			0x, 0x
	   oui			3 bytes			0x00, 0x0C, 0xE7
	   function type	1 byte			0xff
	   suboui			3 bytes			0x00, 0x0C, 0xE7
	   sub func type	1 byte			0x
	*/
	unsigned char *p = pkt, *p_old = NULL;
	unsigned short len = 0;
	unsigned char mtk_oui[3] = {0x00, 0x0C, 0xE7};
	unsigned char i = 0;

	/*vendor specific tlv type*/
	*p++ = 11;

	/*vendor specific tlv length*/
	*((unsigned short*)p) = host_to_be16(8);
	p += 2;

	/*oui*/
	os_memcpy(p, mtk_oui, sizeof(mtk_oui));
	p += 3;

	/*function type*/
	*p++ = 0xff;

	/*additional OUI*/
	os_memcpy(p, mtk_oui, sizeof(mtk_oui));
	p += 3;

	/*1905 internal used vendor specific subtype*/
	*p++ = transparent_vlan_message_subtype;

	/*skip transparency vlan number*/
	p_old = p;
	p++;

	for (i = 0; i < ctx->map_policy.trans_vlan.num; i++) {
		if (ctx->map_policy.trans_vlan.transparent_vids[i]== INVALID_VLAN_ID)
			break;

		*((unsigned short*)p) = host_to_be16(ctx->map_policy.trans_vlan.transparent_vids[i]);
		p += 2;
	}

	/*set transparent vlan number*/
	*p_old = i;

	len = (unsigned short)(p - pkt);

	*((unsigned short*)(pkt + 1)) = host_to_be16(len - 3);

	return len;
}

int append_ap_radio_advan_tlv(unsigned char *pkt, struct ap_radio_advan_cap *radio_adv_capab)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;
	struct ap_radio_advan_cap *radio_advan_capability;

	temp_buf = pkt;
	total_length = 10;
	(*temp_buf) = AP_RADIO_ADVANCED_CAPABILITIES_TYPE;
	*(unsigned short *)(temp_buf+1) = host_to_be16(total_length-3);
	radio_advan_capability = (struct ap_radio_advan_cap *)(temp_buf + 3);
	os_memcpy(radio_advan_capability->radioid,radio_adv_capab->radioid,6);
	radio_advan_capability->flag=radio_adv_capab->flag;

	return total_length;
}

void map_rm_hnat_session()
{
	int ret_len = 0;
	char cmd[1024];

	memset(cmd, 0, sizeof(cmd));
	/*
	**	read each line of hnat_entry
	**	get hnat_entry index
	**	if line_num > 0, rm the hnat_entry by index, and sleep 1 sec for each entry
	*/
	ret_len = snprintf(cmd, sizeof(cmd)-1,
		"cat /sys/kernel/debug/hnat/hnat_entry | while read line;"
		"do"
		"	line_num=$(echo $line | awk -F \"(\" '{print $2}' | awk -F \")\" '{print $1}');"
		"	[ -n \"$line_num\" ] && echo \"3 $line_num\">/sys/kernel/debug/hnat/hnat_entry && usleep 1000;"
		"done");
	if (os_snprintf_error(sizeof(cmd), ret_len)) {
		return;
	}

	printf("\nmap rm hnat session cmd:\n"
		"***************************\n"
		"%s\n"
		"***************************\n", cmd);

	if (system(cmd) == -1)
		debug(DEBUG_OFF, "system cmd fail:%s\n", cmd);
}

struct operational_bss *get_bss_by_bssid(struct p1905_managerd_ctx *ctx,
	unsigned char *mac)
{
	struct operational_bss *bss = NULL;

	dl_list_for_each(bss, &ctx->bss_head, struct operational_bss, entry) {
		if (!os_memcmp(bss->mac, mac, ETH_ALEN))
			return bss;
	}

	return NULL;
}


struct operational_bss *create_oper_bss(struct p1905_managerd_ctx *ctx,
	unsigned char *mac, char *ssid, unsigned char ssid_len, unsigned char *almac, struct dl_list *bss_list)
{
	struct operational_bss *bss = NULL;

	bss = (struct operational_bss*)os_malloc(sizeof(struct operational_bss));

	if (bss) {
		os_memset(bss, 0, sizeof(struct operational_bss));
		os_memcpy(bss->mac, mac, ETH_ALEN);
		os_strncpy(bss->ssid, ssid, ssid_len);
		os_memcpy(bss->almac, almac, ETH_ALEN);
		bss->vid = INVALID_VLAN_ID;
		debug(DEBUG_ERROR, "create operational bss("MACSTR")"
			"almac("MACSTR") ssid=%s\n",
			PRINT_MAC(mac), PRINT_MAC(almac), bss->ssid);
		dl_list_add(bss_list, &bss->entry);
	}

	return bss;
}

int parse_ap_operational_bss_tlv(struct p1905_managerd_ctx *ctx, unsigned char *almac,
		unsigned short len, unsigned char *value, struct dl_list *bss_list)
{
	unsigned char *temp_buf = value;
	int left = len;
	unsigned char radio_num = 0, bss_num = 0, ssid_len = 0;
	struct operational_bss *bss = NULL;
	unsigned char mac[ETH_ALEN] = {0x00};
	char ssid[33];

	if (left < 1)
		return -1;
	left--;

	radio_num = *temp_buf++;
	while (radio_num--) {
		if (left < 7) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n", __LINE__, left, 7);
			return -1;
		}
		left -= 7;

		temp_buf += ETH_ALEN;

		bss_num = *temp_buf++;
		while(bss_num--) {
			if (left < 7) {
				debug(DEBUG_ERROR, "%d: error left %d less than %d\n", __LINE__, left, 7);
				return -1;
			}
			left -= 7;

			os_memset(ssid, 0, sizeof(ssid));
			os_memcpy(mac, temp_buf, ETH_ALEN);
			temp_buf += ETH_ALEN;

			ssid_len = *temp_buf++;
			if (ssid_len > SSID_MAX_LEN) {
				debug(DEBUG_ERROR, "fail to parse ssid because ssid_len (%d) is bigger than ssid buf \n", ssid_len);
				return -1;
			}
			if (left < ssid_len) {
				debug(DEBUG_ERROR, "%d: error left %d less than %d\n", __LINE__, left, ssid_len);
				return -1;
			}
			left -= ssid_len;

			os_memcpy(ssid, temp_buf, ssid_len);
			temp_buf += ssid_len;

			bss = get_bss_by_bssid(ctx, mac);
			if (!bss) {
				bss = create_oper_bss(ctx, mac, ssid, ssid_len, almac, bss_list);

				if (!bss) {
					debug(DEBUG_ERROR, "fail to allocate memory for opertational bss\n");
					continue;
				}
			} else {
				if (os_strncmp(bss->ssid, ssid, ssid_len) ||
					ssid_len != os_strlen(bss->ssid)) {
					os_memset(bss->ssid, 0, sizeof(bss->ssid));
					os_strncpy(bss->ssid, ssid, ssid_len);
				}
			}
		}
	}

	return 0;
}

struct assoc_client *get_cli_by_mac(struct dl_list *clients_table, unsigned char mac[])
{
	struct assoc_client *client = NULL;

	if (dl_list_empty(clients_table))
		return NULL;

	dl_list_for_each(client, clients_table, struct assoc_client, entry) {
		if (!os_memcmp(client->mac, mac, ETH_ALEN))
			return client;
	}
	return NULL;
}

struct assoc_client *add_new_assoc_cli(struct dl_list *clients_table, unsigned char mac[], unsigned char *al_mac)
{
	struct assoc_client *client = NULL;

	client = (struct assoc_client *)os_malloc(sizeof(struct assoc_client));

	if (client) {
		os_memset(client, 0, sizeof(struct assoc_client));
		os_memcpy(client->mac, mac, ETH_ALEN);
		os_memcpy(client->al_mac, al_mac, ETH_ALEN);
		client->vlan_id = INVALID_VLAN_ID;
		dl_list_add(clients_table, &client->entry);
	}

	return client;
}

struct assoc_client *get_assoc_cli_by_mac(struct p1905_managerd_ctx *ctx, unsigned char mac[])
{
	struct assoc_client *client = NULL;

	unsigned char hash = MAC_ADDR_HASH_INDEX(mac);

	dl_list_for_each(client, &ctx->clients_table[hash], struct assoc_client, entry) {
		if (!os_memcmp(client->mac, mac, ETH_ALEN))
			return client;
	}

	return NULL;
}

struct assoc_client *create_assoc_cli(struct p1905_managerd_ctx *ctx, unsigned char mac[],
	unsigned char *al_mac)
{
	struct assoc_client *client = NULL;
	unsigned char hash = 0;

	client = (struct assoc_client*)os_malloc(sizeof(struct assoc_client));

	if (client) {
		os_memset(client, 0, sizeof(struct assoc_client));
		os_memcpy(client->mac, mac, ETH_ALEN);
		os_memcpy(client->al_mac, al_mac, ETH_ALEN);
		client->vlan_id = INVALID_VLAN_ID;

		hash = MAC_ADDR_HASH_INDEX(mac);
		dl_list_add(&ctx->clients_table[hash], &client->entry);
	}

	return client;
}

int parse_assoc_clients_tlv(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
		unsigned short len, unsigned char *value, struct dl_list *clients_table)
{
	unsigned char *temp_buf = value;
	int left = len;
	unsigned char bss_num = 0;
	unsigned short client_num = 0;
	struct assoc_client *p_client = NULL;
	unsigned char mac[ETH_ALEN], bssid[ETH_ALEN];

	if (left < 1)
		return -1;
	left--;

	bss_num = *temp_buf++;

	while(bss_num--) {
		if (left < ETH_ALEN + 2) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n", __LINE__, left, ETH_ALEN + 2);
			return -1;
		}
		left -= ETH_ALEN + 2;

		os_memcpy(bssid, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;

		client_num = be_to_host16(*(unsigned short*)temp_buf);
		temp_buf += 2;

		if (left < (ETH_ALEN + 2) * client_num) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n", __LINE__, left, (ETH_ALEN + 2) * client_num);
			return -1;
		}
		left -= (ETH_ALEN + 2) * client_num;

		while (client_num--) {
			os_memcpy(mac, temp_buf, ETH_ALEN);
			temp_buf += ETH_ALEN;

			temp_buf += 2;
			p_client = get_cli_by_mac(clients_table, mac);
			if (!p_client) {
				p_client = add_new_assoc_cli(clients_table, mac, al_mac);

				if (!p_client) {
					debug(DEBUG_ERROR, "fail to allocate memory for assoc client("MACSTR")\n",
						PRINT_MAC(mac));
					continue;
				}
			}
			os_memcpy(p_client->bssid, bssid, ETH_ALEN);
		}
	}

	return 0;
}

/***channel scan***/
int delete_exist_channel_scan_capability(
	struct p1905_managerd_ctx *ctx, unsigned char* identifier)
{
	 struct channel_scan_cap_db *scan_cap = NULL;
	 struct scan_opcap_db *scap = NULL, *scap_tmp = NULL;
	 int i = 0;

	if(!SLIST_EMPTY(&(ctx->ap_cap_entry.ch_scan_cap_head)))
    {
        SLIST_FOREACH(scan_cap, &(ctx->ap_cap_entry.ch_scan_cap_head), ch_scan_cap_entry)
        {
            if(!memcmp(scan_cap->identifier, identifier, ETH_ALEN))
            {
                if(!SLIST_EMPTY(&(scan_cap->scan_opcap_head)))
                {
                    scap = SLIST_FIRST(&(scan_cap->scan_opcap_head));
                    while(scap)
                    {
                    	debug(DEBUG_TRACE, "delete_exist struct scan_opcap_db\n");
						debug(DEBUG_TRACE, "opclass:%d, ch_num=%d\n", scap->op_class, scap->ch_num);
						debug(DEBUG_TRACE, "ch_list: \n");
						for (i = 0; i < scap->ch_num; i++)
							debug(DEBUG_TRACE, "%d ", scap->ch_list[i]);
						debug(DEBUG_TRACE, "\n");
                        scap_tmp = SLIST_NEXT(scap, scan_opcap_entry);
                        SLIST_REMOVE(&scan_cap->scan_opcap_head, scap,
                                    scan_opcap_db, scan_opcap_entry);
                        free(scap);
                        scap = scap_tmp;
                    }
                }
				debug(DEBUG_TRACE, "delete_exist struct channel_scan_cap_db\n");
				debug(DEBUG_TRACE, "identifier("MACSTR")\n",
					PRINT_MAC(scan_cap->identifier));
				debug(DEBUG_TRACE, "boot_only:%d, scan_impact:%d, min_scan_interval:%d op_class_num:%d\n",
					scan_cap->boot_only, scan_cap->scan_impact, scan_cap->min_scan_interval,
					scan_cap->op_class_num);
                SLIST_REMOVE(&(ctx->ap_cap_entry.ch_scan_cap_head), scan_cap,
                            channel_scan_cap_db, ch_scan_cap_entry);
                free(scan_cap);
                break;
            }
        }
    }

	return wapp_utils_success;
}

int insert_new_channel_scan_capability(
	struct p1905_managerd_ctx *ctx, struct scan_capability_lib *scap)
{
	struct channel_scan_cap_db *scap_db = NULL;
	struct scan_opcap_db *opcap_db = NULL;
	struct scan_opcap *opcap = NULL;
	int op_class_num = 0;
	int i = 0, j=0;

	scap_db = (struct channel_scan_cap_db *)malloc(sizeof(*scap_db));
	if (!scap_db) {
		debug(DEBUG_ERROR, "alloc scap_db fail\n");
		return wapp_utils_error;
	}
	memcpy(scap_db->identifier, scap->identifier, ETH_ALEN);
	scap_db->boot_only = scap->boot_only;
	scap_db->scan_impact = scap->scan_impact;
	scap_db->min_scan_interval = scap->min_scan_interval;
	scap_db->op_class_num = scap->op_class_num;
	SLIST_INSERT_HEAD(&(ctx->ap_cap_entry.ch_scan_cap_head), scap_db, ch_scan_cap_entry);

	debug(DEBUG_TRACE, "insert struct channel_scan_cap_db\n");
	debug(DEBUG_TRACE, "identifier("MACSTR")\n",
		PRINT_MAC(scap_db->identifier));
	debug(DEBUG_TRACE, "boot_only:%d, scan_impact:%d, min_scan_interval:%d op_class_num:%d\n",
		scap_db->boot_only, scap_db->scan_impact, scap_db->min_scan_interval,
		scap_db->op_class_num);

	SLIST_INIT(&scap_db->scan_opcap_head);
	op_class_num = scap_db->op_class_num;
	for(i = 0; i < op_class_num; i++) {
		opcap = &scap->opcap[i];
		opcap_db = (struct scan_opcap_db *)malloc(sizeof(*opcap_db));
		if (!opcap_db) {
			debug(DEBUG_ERROR, "alloc opcap_db fail\n");
			return wapp_utils_error;
		}
		memcpy(opcap_db, opcap, sizeof(*opcap));
		SLIST_INSERT_HEAD(&scap_db->scan_opcap_head, opcap_db, scan_opcap_entry);

		debug(DEBUG_TRACE, "insert struct scan_opcap_db\n");
		debug(DEBUG_TRACE, "opclass:%d, ch_num=%d\n", opcap_db->op_class, opcap_db->ch_num);
		debug(DEBUG_TRACE, "ch_list: \n");
		for (j = 0; j < opcap_db->ch_num; j++)
			debug(DEBUG_TRACE, "%d ", opcap_db->ch_list[j]);
		debug(DEBUG_TRACE, "\n");
	}

	return wapp_utils_success;

}

unsigned short append_channel_scan_capability_tlv(
        unsigned char *pkt, struct list_head_ch_scan_cap *shead)
{
    unsigned short total_length = 0, tlv_len = 0;;
    unsigned char *temp_buf = NULL;
	struct channel_scan_cap_db *pdb_scan = NULL;
	struct scan_opcap_db *pdb_op = NULL;
	unsigned char rnum = 0;

    temp_buf = pkt;

    *temp_buf++ = CHANNEL_SCAN_CAPABILITY_TYPE;
	/*skip tlv length field*/
    temp_buf += 2;
	/*skip radio number field*/
	temp_buf += 1;

	SLIST_FOREACH(pdb_scan, shead, ch_scan_cap_entry)
	{
		debug(DEBUG_TRACE, "pdb_scan->identifier="MACSTR"\n",
			PRINT_MAC(pdb_scan->identifier));
		rnum++;
		/*radio identifier*/
		memcpy(temp_buf, pdb_scan->identifier, ETH_ALEN);
		temp_buf += ETH_ALEN;
		/*boot only flag & scan impact*/
		*temp_buf++ = (pdb_scan->boot_only << 7) | (pdb_scan->scan_impact << 5);
		/*minimum scan interval*/
		*((unsigned int *)temp_buf) = host_to_be32(pdb_scan->min_scan_interval);
		temp_buf += 4;
		/*number of operating classes*/
		*temp_buf++ = pdb_scan->op_class_num;
		SLIST_FOREACH(pdb_op, &pdb_scan->scan_opcap_head, scan_opcap_entry)
		{
			/*operating class*/
			*temp_buf++ = pdb_op->op_class;
			/*number of channels*/
			*temp_buf++ = pdb_op->ch_num;
			/*channel list*/
			if (pdb_op->ch_num)
				memcpy(temp_buf, pdb_op->ch_list, pdb_op->ch_num);
			temp_buf += pdb_op->ch_num;
		}
	}

	/*number of radios*/
	*(pkt + 3) = rnum;

	total_length = (unsigned short)(temp_buf - pkt);
	tlv_len = total_length - 3;
	*((unsigned short *)(pkt + 1)) = host_to_be16(tlv_len);

    return total_length;
}


/***dfs cac***/
int delete_exist_cac_capability(
	struct p1905_managerd_ctx *ctx, unsigned char* identifier)
{
	struct radio_cac_capability_db *p_radio_cac_cap_db = NULL;
	struct cac_capability_db *p_cac_capab_db = NULL, *p_cac_capab_db_tmp = NULL;
	struct cac_cap_db *p_cac_cap_db = NULL, *p_cac_cap_db_tmp = NULL;
	struct cac_opcap_db *p_cac_opcab_db = NULL, *p_cac_opcab_db_tmp = NULL;


	p_radio_cac_cap_db = &ctx->ap_cap_entry.radio_cac_cap;

	if(!SLIST_EMPTY(&p_radio_cac_cap_db->radio_cac_capab_head))
	{
		//SLIST_FOREACH(p_cac_capab_db, &p_radio_cac_cap_db->radio_cac_capab_head, cac_capab_entry)
		p_cac_capab_db = SLIST_FIRST(&(p_radio_cac_cap_db->radio_cac_capab_head));
		while (p_cac_capab_db)
		{
			p_cac_capab_db_tmp = SLIST_NEXT(p_cac_capab_db, cac_capab_entry);
			if(!memcmp(p_cac_capab_db->identifier, identifier, ETH_ALEN))
			{
				debug(DEBUG_TRACE, "delete existed cac_capability for "MACSTR"\n",
					PRINT_MAC(p_cac_capab_db->identifier));

				if(!SLIST_EMPTY(&(p_cac_capab_db->cac_capab_head)))
				{
					p_cac_cap_db = SLIST_FIRST(&(p_cac_capab_db->cac_capab_head));
					while(p_cac_cap_db)
					{
						p_cac_cap_db_tmp = SLIST_NEXT(p_cac_cap_db, cac_cap_entry);

						/*
						debug(DEBUG_TRACE, "delete cac_cap_db\n");
						debug(DEBUG_TRACE, "cac_mode:%d\n", p_cac_cap_db->cac_mode);
						*/

						if(!SLIST_EMPTY(&(p_cac_cap_db->cac_opcap_head)))
						{
							p_cac_opcab_db = SLIST_FIRST(&(p_cac_cap_db->cac_opcap_head));
							while(p_cac_opcab_db) {
								/*
								debug(DEBUG_TRACE, "delete cac_opcap_db\n");
								debug(DEBUG_TRACE, "opclass:%d, ch_num=%d\n", p_cac_opcab_db->op_class, p_cac_opcab_db->ch_num);
								debug(DEBUG_TRACE, "ch_list: \n");
								*/
								p_cac_opcab_db_tmp = SLIST_NEXT(p_cac_opcab_db, cac_opcap_entry);
								SLIST_REMOVE(&(p_cac_cap_db->cac_opcap_head), p_cac_opcab_db,
									cac_opcap_db, cac_opcap_entry);

								free(p_cac_opcab_db);
								p_cac_opcab_db = p_cac_opcab_db_tmp;
							}
						}

						SLIST_REMOVE(&(p_cac_capab_db->cac_capab_head), p_cac_cap_db,
							cac_cap_db, cac_cap_entry);
						free(p_cac_cap_db);
						p_cac_cap_db = p_cac_cap_db_tmp;
					}
				}

				SLIST_REMOVE(&(p_radio_cac_cap_db->radio_cac_capab_head), p_cac_capab_db,
					cac_capability_db, cac_capab_entry);
				free(p_cac_capab_db);
			}
			p_cac_capab_db = p_cac_capab_db_tmp;
		}
	}

	return wapp_utils_success;
}



int insert_new_cac_capability(
	struct p1905_managerd_ctx *ctx, struct cac_capability_lib *cac_capab_in)
{
	struct radio_cac_capability_db *p_radio_cac_cap_db = &ctx->ap_cap_entry.radio_cac_cap;
	struct cac_capability_db *p_cac_capab_db = NULL;
	struct cac_cap_db *p_cac_cap_db = NULL;
	struct cac_opcap_db *p_cac_opcap_db = NULL;
	struct cac_cap *p_cac_cap_in = NULL;
	struct cac_type *p_cac_type_in = NULL;
	struct cac_opcap *p_cac_opcap_in = NULL;
	int i = 0, j=0, k = 0, n = 0;
	int offset_type = 0, offset_opcap = 0;
	char ch_str[64] = {0}, offset_str = 0;

	if (!ctx || !cac_capab_in)
		return wapp_utils_error;

	memcpy(p_radio_cac_cap_db->country_code, cac_capab_in->country_code, 2);
	p_radio_cac_cap_db->radio_num = cac_capab_in->radio_num;
	debug(DEBUG_TRACE, "insert cac cap \n");

	debug(DEBUG_TRACE, "  country_code %s, radio_num %d\n",
		p_radio_cac_cap_db->country_code, p_radio_cac_cap_db->radio_num);

	p_cac_cap_in = cac_capab_in->cap;
	for (i = 0; i < p_radio_cac_cap_db->radio_num; i ++) {

		delete_exist_cac_capability(ctx, p_cac_cap_in->identifier);

		p_cac_capab_db = (struct cac_capability_db *)malloc(sizeof(struct cac_capability_db));
		if (!p_cac_capab_db)
			goto error;

		memcpy(p_cac_capab_db->identifier, p_cac_cap_in->identifier, ETH_ALEN);
		p_cac_capab_db->cac_type_num = p_cac_cap_in->cac_type_num;


		debug(DEBUG_TRACE, "    new identifier "MACSTR", cac_type_num %d\n",
			PRINT_MAC(p_cac_capab_db->identifier), p_cac_capab_db->cac_type_num);

		SLIST_INIT(&p_cac_capab_db->cac_capab_head);

		SLIST_INSERT_HEAD(&(p_radio_cac_cap_db->radio_cac_capab_head), p_cac_capab_db, cac_capab_entry);

		p_cac_type_in = p_cac_cap_in->type;
		offset_type = 0;
		for (j = 0; j < p_cac_capab_db->cac_type_num; j ++) {
			p_cac_cap_db = (struct cac_cap_db *)malloc(sizeof(struct cac_cap_db));
			if (!p_cac_cap_db)
				goto error;

			p_cac_cap_db->cac_mode = p_cac_type_in->cac_mode;
			memcpy(p_cac_cap_db->cac_interval, p_cac_type_in->cac_interval, sizeof(p_cac_cap_db->cac_interval));
			p_cac_cap_db->op_class_num = p_cac_type_in->op_class_num;

			debug(DEBUG_TRACE, "      cac_mode %d, cac_interval 0x%02x,0x%02x,0x%02x, op_class_num %d\n",
				p_cac_cap_db->cac_mode, p_cac_cap_db->cac_interval[0], p_cac_cap_db->cac_interval[1],
				p_cac_cap_db->cac_interval[2], p_cac_cap_db->op_class_num);


			SLIST_INIT(&p_cac_cap_db->cac_opcap_head);

			SLIST_INSERT_HEAD(&p_cac_capab_db->cac_capab_head, p_cac_cap_db, cac_cap_entry);

			p_cac_opcap_in = p_cac_type_in->opcap;
			offset_opcap = 0;
			for (k = 0; k < p_cac_cap_db->op_class_num; k ++) {
				p_cac_opcap_db = (struct cac_opcap_db *)malloc(sizeof(struct cac_opcap_db));
				if (!p_cac_opcap_db)
					goto error;

				p_cac_opcap_db->op_class = p_cac_opcap_in->op_class;
				p_cac_opcap_db->ch_num = p_cac_opcap_in->ch_num;

				memcpy(p_cac_opcap_db->ch_list, p_cac_opcap_in->ch_list, p_cac_opcap_db->ch_num);

				debug(DEBUG_TRACE, "        op_class %d, ch_num %d\n",
					p_cac_opcap_db->op_class, p_cac_opcap_db->ch_num);

				offset_str = 0;
				memset(ch_str, 0, sizeof(ch_str));
				for (n = 0; n < p_cac_opcap_db->ch_num; n ++)
					offset_str += sprintf(ch_str + offset_str, "%02x,", p_cac_opcap_db->ch_list[n]);

				debug(DEBUG_TRACE, "          channel %s\n", ch_str);

				SLIST_INSERT_HEAD(&p_cac_cap_db->cac_opcap_head, p_cac_opcap_db, cac_opcap_entry);

				p_cac_opcap_in = (struct cac_opcap *)((char *)p_cac_opcap_in + sizeof(struct cac_opcap) + p_cac_opcap_db->ch_num);
				offset_opcap += sizeof(struct cac_opcap) + p_cac_opcap_db->ch_num;

			}

			p_cac_type_in = (struct cac_type *)((char *)p_cac_type_in + sizeof(struct cac_type) + offset_opcap);
			offset_type += sizeof(struct cac_type) + offset_opcap;
		}

		p_cac_cap_in = (struct cac_cap *)((char *)p_cac_cap_in + sizeof(struct cac_cap) + offset_type);
	}

	return wapp_utils_success;
error:
	return wapp_utils_error;

}

unsigned short append_cac_capability_tlv(
        unsigned char *pkt, struct radio_cac_capability_db *cachead)
{
	unsigned short total_length = 0, tlv_len = 0;;
	unsigned char *temp_buf = NULL;
	struct cac_capability_db *p_cac_capab = NULL;
	struct cac_cap_db *p_cac_cab = NULL;
	struct cac_opcap_db *p_cac_cab_opcap = NULL;

	temp_buf = pkt;

	*temp_buf++ = CAC_CAPABILITIES_TYPE;
	/*skip tlv length field*/
	temp_buf += 2;

	/* country code, 2 octets */
	memcpy(temp_buf, cachead->country_code, sizeof(cachead->country_code));
	temp_buf += 2;

	/* Number of radios, 1 octet */
	*temp_buf++ = cachead->radio_num;


	SLIST_FOREACH(p_cac_capab, &cachead->radio_cac_capab_head, cac_capab_entry)
	{
		/*radio identifier, 6 octets*/
		memcpy(temp_buf, p_cac_capab->identifier, ETH_ALEN);
		debug(DEBUG_TRACE, "identifier="MACSTR"\n",
			PRINT_MAC(p_cac_capab->identifier));
		temp_buf += ETH_ALEN;

		/*Number of types of CAC, 1 octet*/
		*temp_buf++ = p_cac_capab->cac_type_num;
		debug(DEBUG_TRACE, "cac_type_num %d\n", p_cac_capab->cac_type_num);

		SLIST_FOREACH(p_cac_cab, &p_cac_capab->cac_capab_head, cac_cap_entry)
		{
			/* CAC mode supported, 1 octet */
			*temp_buf++ = p_cac_cab->cac_mode;

			/* Number of seconds required to complete CAC, 3 octets */
			memcpy(temp_buf, p_cac_cab->cac_interval, 3);
			temp_buf += 3;

			/* Number of classes supported, 1 octet */
			*temp_buf++ = p_cac_cab->op_class_num;

			debug(DEBUG_TRACE, "  cac_mode %d\n", p_cac_cab->cac_mode);
			debug(DEBUG_TRACE, "  cac_interval 0x%02x,0x%02x,0x%02x\n",
				p_cac_cab->cac_interval[0], p_cac_cab->cac_interval[1], p_cac_cab->cac_interval[2]);
			debug(DEBUG_TRACE, "  op_class_num %d\n", p_cac_cab->op_class_num);
			SLIST_FOREACH(p_cac_cab_opcap, &p_cac_cab->cac_opcap_head, cac_opcap_entry)
			{
				/* perating class, 1 octet */
				*temp_buf++ = p_cac_cab_opcap->op_class;

				/* perating class, 1 octet */
				*temp_buf++ = p_cac_cab_opcap->ch_num;

				/*channel number, */
				memcpy(temp_buf, p_cac_cab_opcap->ch_list, p_cac_cab_opcap->ch_num);

				debug(DEBUG_TRACE, "    op_class %d\n", p_cac_cab_opcap->op_class);
				debug(DEBUG_TRACE, "    ch_num %d\n", p_cac_cab_opcap->ch_num);

				temp_buf += p_cac_cab_opcap->ch_num;
			}

		}

	}

	total_length = (unsigned short)(temp_buf - pkt);
	tlv_len = total_length - 3;
	*((unsigned short *)(pkt + 1)) = host_to_be16(tlv_len);

    return total_length;
}


int update_r2_ap_capability(
	struct p1905_managerd_ctx *ctx, struct ap_r2_capability *ap_r2_cap)
{
	debug(DEBUG_TRACE, "receive LIB_R2_AP_CAP\n");

	memcpy(&ctx->ap_cap_entry.ap_r2_cap, ap_r2_cap, sizeof(struct ap_r2_capability));

	debug(DEBUG_TRACE, "\tmax_total_num_sp_rules:%d\n\tbyte_counter_units:%d\n\tmax_total_num_vid %d\n",
		ap_r2_cap->max_total_num_sp_rules, ap_r2_cap->byte_counter_units, ap_r2_cap->max_total_num_vid);

	debug(DEBUG_TRACE, "\tsp_flag:%d\n\tdpp_flag:%d\n",
		ap_r2_cap->sp_flag, ap_r2_cap->dpp_flag);

	return wapp_utils_success;

}



int insert_new_ap_extended_capability(
	struct p1905_managerd_ctx *ctx, struct ap_extended_metrics_lib *metric)
{
	struct ap_ext_cap_db *p_new_db = NULL;

	p_new_db = (struct ap_ext_cap_db *)malloc(sizeof(struct ap_ext_cap_db));
	if (p_new_db == NULL) {
		debug(DEBUG_ERROR, "malloc ap_ext_cap_db failed\n");
		return wapp_utils_error;
	}
	memcpy(p_new_db->bssid, metric->bssid, ETH_ALEN);
	p_new_db->uc_tx = metric->uc_tx;
	p_new_db->uc_rx = metric->uc_rx;
	p_new_db->mc_tx = metric->mc_tx;
	p_new_db->mc_rx = metric->mc_rx;
	p_new_db->bc_tx = metric->bc_tx;
	p_new_db->bc_rx = metric->bc_rx;
	SLIST_INSERT_HEAD(&(ctx->ap_cap_entry.ap_ext_cap_head), p_new_db, ap_ext_cap_entry);

	debug(DEBUG_TRACE, "insert new ap extended capability for "MACSTR"\n",
		PRINT_MAC(metric->bssid));
	debug(DEBUG_TRACE, "uc tx %d, uc rx %d, mc tx %d, mc rx %d, bc tx %d, bc rx %d\n",
		metric->uc_tx, metric->uc_rx, metric->mc_tx, metric->mc_rx, metric->mc_tx, metric->mc_rx);


	return wapp_utils_success;

}




int insert_new_radio_metric(
	struct p1905_managerd_ctx *ctx, struct radio_metrics_lib *metric)
{
	struct radio_metrics_db *p_new_db = NULL;

	p_new_db = (struct radio_metrics_db *)malloc(sizeof(struct radio_metrics_db));
	if (p_new_db == NULL) {
		debug(DEBUG_ERROR, "malloc radio_metrics_db failed\n");
		return wapp_utils_error;
	}
	memcpy(p_new_db->identifier, metric->identifier, ETH_ALEN);
	p_new_db->noise= metric->noise;
	p_new_db->transmit= metric->transmit;
	p_new_db->receive_self= metric->receive_self;
	p_new_db->receive_other= metric->receive_other;
	SLIST_INSERT_HEAD(&(ctx->metric_entry.radio_metrics_head), p_new_db, radio_metrics_entry);

	debug(DEBUG_TRACE, "insert new ap radio metric for "MACSTR"\n",
		PRINT_MAC(metric->identifier));
	debug(DEBUG_TRACE, "noise %d, transmit %d, receive_self %d, receive_other %d\n",
		metric->noise, metric->transmit, metric->receive_self, metric->receive_other);


	return wapp_utils_success;

}


int delete_assoc_sta_extended_link_metric(
	struct p1905_managerd_ctx *ctx, struct sta_extended_metrics_lib *metric)
{
	struct sta_extended_metrics_db *p_tmp = NULL, *p_next = NULL;
	struct extended_metrics_db *p_tmp_metric = NULL, * p_next_metric = NULL;

	/* rm exsited extended_metrics_db for the sta */
	if(!SLIST_EMPTY(&ctx->metric_entry.assoc_sta_extended_link_metrics_head))
	{
		p_tmp = SLIST_FIRST(&ctx->metric_entry.assoc_sta_extended_link_metrics_head);
		while (p_tmp)
		{
			p_next = SLIST_NEXT(p_tmp, sta_extended_metrics_entry);
			if(!memcmp(p_tmp->sta_mac, metric->sta_mac, ETH_ALEN)) {
				p_tmp_metric = SLIST_FIRST(&(p_tmp->extended_metrics_head));
				while(p_tmp_metric)
				{
					p_next_metric = SLIST_NEXT(p_tmp_metric, metrics_entry);

					SLIST_REMOVE(&(p_tmp->extended_metrics_head), p_tmp_metric,
						extended_metrics_db, metrics_entry);
					free(p_tmp_metric);
					p_tmp_metric = p_next_metric;
				}
			}

			SLIST_REMOVE(&ctx->metric_entry.assoc_sta_extended_link_metrics_head, p_tmp,
				sta_extended_metrics_db, sta_extended_metrics_entry);
			free(p_tmp);
			p_tmp = p_next;
		}

	}
	return wapp_utils_success;
}



int insert_assoc_sta_extended_link_metric(
	struct p1905_managerd_ctx *ctx, struct sta_extended_metrics_lib *metric)
{
	struct sta_extended_metrics_db *p_new_db = NULL;
	struct extended_metrics_db *p_new_metric_db = NULL;
	struct extended_metrics_info *p_metric_in = NULL;
	int i = 0;

	delete_assoc_sta_extended_link_metric(ctx, metric);

	/* insert  */
	p_new_db = (struct sta_extended_metrics_db *)malloc(sizeof(struct sta_extended_metrics_db));
	if (p_new_db == NULL) {
		debug(DEBUG_ERROR, "malloc sta_extended_metrics_db failed\n");
		return wapp_utils_error;
	}
	memset(p_new_db, 0, sizeof(struct sta_extended_metrics_db));
	memcpy(p_new_db->sta_mac, metric->sta_mac, ETH_ALEN);
	p_new_db->extended_metric_cnt = metric->extended_metric_cnt;

	SLIST_INSERT_HEAD(&(ctx->metric_entry.assoc_sta_extended_link_metrics_head),
		p_new_db, sta_extended_metrics_entry);

	debug(DEBUG_TRACE, "insert new assoc sta link metric for "MACSTR"\n",
		PRINT_MAC(metric->sta_mac));

	p_metric_in = metric->metric_info;
	while (i < metric->extended_metric_cnt) {
		p_new_metric_db = (struct extended_metrics_db *)malloc(sizeof(struct extended_metrics_db));
		if (p_new_metric_db == NULL) {
			debug(DEBUG_ERROR, "malloc new_metric_db failed\n");
			return wapp_utils_error;
		}

		memcpy(p_new_metric_db->bssid, p_metric_in->bssid, ETH_ALEN);
		p_new_metric_db->last_data_ul_rate = p_metric_in->last_data_ul_rate;
		p_new_metric_db->last_data_dl_rate = p_metric_in->last_data_dl_rate;
		p_new_metric_db->utilization_rx = p_metric_in->utilization_rx;
		p_new_metric_db->utilization_tx= p_metric_in->utilization_tx;

		debug(DEBUG_TRACE, "insert new extended link metric for bssid "MACSTR"\n",
			PRINT_MAC(p_new_metric_db->bssid));

		SLIST_INSERT_HEAD(&(p_new_db->extended_metrics_head),
			p_new_metric_db, metrics_entry);

		p_metric_in += 1;
		i ++;
	}


	return wapp_utils_success;

}

int parse_channel_scan_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned int tlv_len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while(1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "[%d] type = %d, tlv len %d > left_tlv_len %d\n",
				__LINE__, *type, tlv_len, left_tlv_len);
			return -1;
		}

		if (*type == CHANNEL_SCAN_REQUEST_TYPE) {
			integrity |= 0x1;
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 0x1) {
		debug(DEBUG_ERROR, "no channel scan request tlv\n");
		return -1;
	}

	return 0;
}

int parse_channel_scan_report_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while(1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == TIMESTAMP_TYPE) {
			integrity |= 0x1;
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == CHANNEL_SCAN_RESULT_TYPE) {
			integrity |= 0x2;
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == VENDOR_SPECIFIC_TLV_TYPE) {
			integrity |= 0x2;
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 0x3) {
		debug(DEBUG_ERROR, "incomplete channel scan report 0x%x 0x3\n",
			integrity);
		return -1;
	}

	return 0;
}

unsigned short channel_scan_request_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CHANNEL_SCAN_REQUEST);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

unsigned short channel_scan_report_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CHANNEL_SCAN_REPORT);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}



unsigned short tunneled_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(TUNNELED_MESSAGE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}



unsigned short assoc_status_notification_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	/*for this kind of message, maybe we'll send more than one(for relayed multicast)
	*so we cannot call append_send_tlv which will set send_tlv_len to 0
	*/
	length = append_send_tlv_relayed(tlv_temp_buf, ctx);
		total_tlvs_length += length;
		tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(ASSOCIATION_STATUS_NOTIFICATION);
	msg_hdr->relay_indicator = 0x1;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}


unsigned short cac_request_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CAC_REQUEST);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}


unsigned short cac_terminate_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CAC_TERMINATION);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}


unsigned short client_disassciation_stats_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CLIENT_DISASSOCIATION_STATS);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}



unsigned short backhual_sta_cap_query_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(BACKHAUL_STA_CAP_QUERY_MESSAGE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}

unsigned short backhual_sta_cap_report_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;
	int i = 0, r = 0;

	msg_hdr = (cmdu_message_header*)buf;


	for (r = 0; r < ctx->radio_number; r++) {
		for (i = 0; i < ctx->itf_number; i++) {
			if (ctx->itf[i].is_wifi_sta &&
				!os_memcmp(ctx->rinfo[r].identifier, ctx->itf[i].identifier, ETH_ALEN)) {
				length = append_backhaul_sta_radio_cap_tlv(tlv_temp_buf, ctx->itf[i].identifier, ctx->itf[i].mac_addr);
				total_tlvs_length += length;
				tlv_temp_buf += length;
				break;
			}
		}
		if (i >= ctx->itf_number) {
			length = append_backhaul_sta_radio_cap_tlv(tlv_temp_buf, ctx->rinfo[r].identifier, NULL);
			total_tlvs_length += length;
			tlv_temp_buf += length;
		}
	}

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(BACKHAUL_STA_CAP_REPORT_MESSAGE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}


int delete_exist_traffic_policy(struct p1905_managerd_ctx *ctx, struct traffics_policy *tpolicy)
{
	struct traffic_separation_db *policy = NULL, *policy_tmp = NULL;

	policy = SLIST_FIRST(&tpolicy->policy_head);
	while(policy != NULL) {
		debug(DEBUG_OFF, "SSID=%s, vlan_id=%d\n",policy->SSID, policy->vid);
		free(policy->SSID);
		policy_tmp = SLIST_NEXT(policy, policy_entry);
		free(policy);
		policy = policy_tmp;
	}
	SLIST_INIT(&tpolicy->policy_head);
	tpolicy->SSID_num = 0;

	return wapp_utils_success;
}

int parse_traffic_separation_policy_tlv(
	struct traffics_policy *tpolicy,
	unsigned short len, unsigned char *val)
{
	unsigned char *temp_buf = val;
	unsigned short i = 0;
	struct traffic_separation_db *tpolicy_db = NULL;
	unsigned char ssidbuf[SSID_MAX_LEN + 1] = {0};
	int left = len;

	if (left - 1 < 0) {
		debug(DEBUG_ERROR, "pre length check for traffic separation policy fail\n");
		goto err0;
	}

	tpolicy->SSID_num = *temp_buf++;
	debug(DEBUG_ERROR, "SSID_num(%d)\n", tpolicy->SSID_num);
	left -= 1;

	for (i = 0; i < tpolicy->SSID_num; i++) {
		if (left - 1 < 0) {
			debug(DEBUG_ERROR, "pre length check for ssid len fail\n");
			goto err0;
		}
		tpolicy_db = (struct traffic_separation_db *)os_zalloc(sizeof(struct traffic_separation_db));
		if (!tpolicy_db) {
			debug(DEBUG_ERROR, "alloc struct traffic_separation_db fail\n");
			goto err0;
		}
		tpolicy_db->SSID_len = *temp_buf++;
		if (tpolicy_db->SSID_len > SSID_MAX_LEN) {
			debug(DEBUG_ERROR, "SSID_len(%d) is large than 32 bytes\n", tpolicy_db->SSID_len);
			goto err1;
		}
		debug(DEBUG_ERROR, "SSID_len(%d)\n", tpolicy_db->SSID_len);
		left -= 1;

		if (left - tpolicy_db->SSID_len < 0) {
			debug(DEBUG_ERROR, "pre length check for ssid fail\n");
			goto err1;
		}
		tpolicy_db->SSID = os_zalloc(tpolicy_db->SSID_len + 1);
		if (!tpolicy_db->SSID) {
			debug(DEBUG_ERROR, "kmalloc for trffic_policy->SSID fail\n");
			goto err1;
		}
		os_memset(tpolicy_db->SSID, 0, tpolicy_db->SSID_len + 1);
		os_memcpy(tpolicy_db->SSID, temp_buf, tpolicy_db->SSID_len);
		os_memset(ssidbuf, 0, SSID_MAX_LEN + 1);
		os_memcpy(ssidbuf, tpolicy_db->SSID, tpolicy_db->SSID_len);
		temp_buf += tpolicy_db->SSID_len;
		left -= tpolicy_db->SSID_len;

		if (left - 2 < 0) {
			debug(DEBUG_ERROR, "pre length check for vid fail\n");
			goto err1;
		}
		tpolicy_db->vid = *((unsigned short *)temp_buf);
		tpolicy_db->vid = be_to_host16(tpolicy_db->vid);
		temp_buf += 2;
		debug(DEBUG_ERROR, "parse:%s, %d\n", ssidbuf, tpolicy_db->vid);
		left -= 2;
		SLIST_INSERT_HEAD(&tpolicy->policy_head, tpolicy_db, policy_entry);
	}

	return 0;

err1:
	if (tpolicy_db) {
		if (tpolicy_db->SSID) {
			free(tpolicy_db->SSID);
			tpolicy_db->SSID = NULL;
		}
		free(tpolicy_db);
	}
err0:
	return -1;

}

int delete_exist_pfilter_policy(struct p1905_managerd_ctx *ctx, struct pfiltering_policy *fpolicy)
{
	struct pf_bssid_list *bssid = NULL, *bssid_tmp = NULL;
	struct dest_mac_list *dmac = NULL, *dmac_tmp = NULL;

	bssid = SLIST_FIRST(&fpolicy->bssid_head);
	while (bssid != NULL) {
		dmac = SLIST_FIRST(&bssid->dmac_head);
		while (dmac != NULL) {
			dmac_tmp = SLIST_NEXT(dmac, dmac_entry);
			free(dmac);
			dmac = dmac_tmp;
		}
		SLIST_INIT(&bssid->dmac_head);
		bssid->n = 0;

		bssid_tmp = SLIST_NEXT(bssid, bssid_entry);
		free(bssid);
		bssid = bssid_tmp;
	}
	SLIST_INIT(&fpolicy->bssid_head);
	fpolicy->k= 0;

	return wapp_utils_success;
}

int parse_packet_filtering_policy_tlv(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *temp_buf;
	unsigned short length = 0, i = 0, j = 0;
	struct pf_bssid_list *pfBSSID = NULL;
	struct dest_mac_list *dmac = NULL;

	struct pfiltering_policy *fpolicy = &ctx->map_policy.fpolicy;
	int total_tlv_length = 0;

	temp_buf = buf;
	if ((*temp_buf) == PACKET_FILTERING_POLICY_TYPE) {
		temp_buf++;
		total_tlv_length++;
	} else {
		return -1;
	}

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	//shift to tlv value field
	temp_buf += 2;
	total_tlv_length += (2+length);

	fpolicy->k = *temp_buf++;
	debug(DEBUG_ERROR, "pkt filtering policy, k=%d\n", fpolicy->k);
	for (i = 0; i < fpolicy->k; i++) {
		pfBSSID = (struct pf_bssid_list *)malloc(sizeof(struct pf_bssid_list));
		if (!pfBSSID) {
			debug(DEBUG_ERROR, "alloc struct pf_bssid_list fail\n");
			return -1;
		}
		memset(pfBSSID, 0, sizeof(struct pf_bssid_list));
		memcpy(pfBSSID->bssid, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;
		pfBSSID->n = *temp_buf++;

		debug(DEBUG_ERROR, "BSSID:"MACSTR", n=%d\n", PRINT_MAC(pfBSSID->bssid), pfBSSID->n);

		SLIST_INSERT_HEAD(&fpolicy->bssid_head, pfBSSID, bssid_entry);

		for (j = 0; j < pfBSSID->n; j ++) {
			dmac = (struct dest_mac_list *)malloc(sizeof(struct dest_mac_list));
			if (!dmac) {
				debug(DEBUG_ERROR, "alloc struct dest_mac_list fail\n");
				return -1;
			}
			memcpy(dmac->dest_addr, temp_buf, ETH_ALEN);
			temp_buf += ETH_ALEN;
			debug(DEBUG_ERROR, "\t\t dmac[%d]:"MACSTR"\n", j, PRINT_MAC(dmac->dest_addr));

			SLIST_INSERT_HEAD(&pfBSSID->dmac_head, dmac, dmac_entry);
		}
	}

	return total_tlv_length;
}

int parse_default_802_1q_setting_tlv(
	struct def_8021q_setting *setting,
	unsigned short len, unsigned char *val)
{
	unsigned char *temp_buf = val;
	unsigned short left = len;

	if (left < (sizeof(*setting)-1)) {
		debug(DEBUG_ERROR, "pre length check for default 802_1q fail\n");
		return -1;
	}
	memcpy(setting, temp_buf, sizeof(*setting));
	setting->primary_vid = be_to_host16(setting->primary_vid);
	debug(DEBUG_ERROR, "primary_vid:%d pcp:%d, length=%d\n",
				setting->primary_vid, setting->dft.PCP, len);

	return 0;
}

int delete_exist_eth_config_policy(struct p1905_managerd_ctx *ctx, struct ethernets_policy *epolicy)
{
	struct ethernet_config_db *policy = NULL, *policy_tmp = NULL;

	policy = SLIST_FIRST(&epolicy->policy_head);
	while(policy != NULL) {
		debug(DEBUG_TRACE, "intfid=%pM, type=%d\n",policy->intfid, policy->eth.type);
		policy_tmp = SLIST_NEXT(policy, policy_entry);
		free(policy);
		policy = policy_tmp;
	}
	SLIST_INIT(&epolicy->policy_head);
	epolicy->inf_num= 0;

	return wapp_utils_success;
}

int parse_eth_config_policy_tlv(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *temp_buf;
	unsigned short length = 0, i =0;
	struct ethernet_config_db *eth_policy = NULL;
	struct ethernets_policy *policy = &ctx->map_policy.epolicy;
	int total_tlv_length = 0;

	temp_buf = buf;

	if((*temp_buf) == ETHERNET_CONFIGURATION_POLICY_TYPE) {
		temp_buf++;
		total_tlv_length++;
	}
	else {
		return -1;
	}

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	//shift to tlv value field
	temp_buf += 2;
	total_tlv_length += (2+length);

	policy->inf_num = *temp_buf++;
	debug(DEBUG_TRACE, "inf_num(%d)\n", policy->inf_num);
	for (i = 0; i < policy->inf_num; i++) {
		eth_policy = (struct ethernet_config_db *)malloc(sizeof(struct ethernet_config_db));
		if (!eth_policy) {
			debug(DEBUG_ERROR, "alloc struct ethernet_config_db fail\n");
			return -1;
		}
		memcpy(eth_policy->intfid, temp_buf, ETH_ALEN);
		debug(DEBUG_TRACE, "intfid(%pM)\n", eth_policy->intfid);

		temp_buf += ETH_ALEN;

		memcpy(&eth_policy->eth, temp_buf, sizeof(eth_policy->eth));
		temp_buf += 1;

		SLIST_INSERT_HEAD(&policy->policy_head, eth_policy, policy_entry);
	}

	return total_tlv_length;
}


int parse_unsuccess_assoc_policy_tlv(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
	struct unsuccess_assoc_policy *policy = &ctx->map_policy.unsuccess_assoc_policy;
	int total_tlv_length = 0;
	unsigned int report_rate = 0;

	temp_buf = buf;

	if((*temp_buf) == UNSUCCESSFUL_ASSOCIATION_POLICY_TYPE) {
		temp_buf++;
		total_tlv_length++;
	}
	else {
		return -1;
	}

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	//shift to tlv value field
	temp_buf += 2;
	total_tlv_length += (2+length);

	if ((*(temp_buf++)) & 0x80) {
		policy->report_switch = 1;
	}
	else
		policy->report_switch = 0;

	report_rate = (*((unsigned int *)temp_buf));

	policy->report_rate = be_to_host32(report_rate);


	return total_tlv_length;
}

#if 0
void mapfilter_set_pfilter_policy(struct p1905_managerd_ctx *ctx, struct pfiltering_policy *fpolicy)
{
	unsigned char buf[520] = {0};
	unsigned char *msgbuf;
	struct nlmsg_cmd *msg = NULL;
	struct pf_bssid_list *pfBSSID = NULL;
	struct dest_mac_list *dmac = NULL;

	msg = (struct nlmsg_cmd *)buf;
	msg->cmd = NLMSG_PERMMIT_MAC;
	msg->num = fpolicy->k;
	msg->bufsize = 0;
	msgbuf = msg->buf;

	SLIST_FOREACH(pfBSSID, &(fpolicy->bssid_head), bssid_entry) {
		memcpy(msgbuf, pfBSSID->bssid, ETH_ALEN);
		msgbuf += ETH_ALEN;
		msg->bufsize += ETH_ALEN;

		memcpy(msgbuf, &pfBSSID->n, 1);
		msgbuf += 1;
		msg->bufsize += 1;

		msg->bufsize += pfBSSID->n*ETH_ALEN;
		SLIST_FOREACH(dmac, &(pfBSSID->dmac_head), dmac_entry) {
			memcpy(msgbuf, dmac->dest_addr, ETH_ALEN);
			msgbuf += ETH_ALEN;
		}
	}
	send_msg_to_mapfilter(msg);

}

void mapfilter_set_ethernet_policy(struct p1905_managerd_ctx *ctx, struct ethernets_policy *epolicy)
{
	unsigned char buf[280] = {0};
	unsigned char *msgbuf;
	struct nlmsg_cmd *msg = NULL;
	struct ethernet_config_db *policy_db = NULL;
	struct intf_type_info inf_info = {0};

	if (!epolicy->inf_num)
		return ;

	msg = (struct nlmsg_cmd *)buf;
	msg->cmd = NLMSG_ETH_INTF;
	msg->num = epolicy->inf_num;
	msg->bufsize = msg->num * sizeof(struct intf_type_info);
	msgbuf = msg->buf;

	SLIST_FOREACH(policy_db, &(epolicy->policy_head), policy_entry)
	{
		memcpy(inf_info.mac, policy_db->intfid, ETH_ALEN);
		inf_info.type = policy_db->eth.type;
		memcpy(msgbuf, &inf_info, sizeof(struct intf_type_info));
		msgbuf += sizeof(struct intf_type_info);
	}
	send_msg_to_mapfilter(msg);

}
#endif

void map_r2_notify_ts_config(void *eloop_ctx, void *timeout_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;
	struct ssid_2_vid_mapping *mapping = NULL, *mapping_org = NULL;
	struct traffics_policy *pts_policy = &ctx->map_policy.tpolicy;
	struct traffic_separation_db *policy_db = NULL;
	unsigned short vids[256] = {0};
	unsigned char vid_num = 0;

	/*ts policy does not update, do not notify it*/
	if (!ctx->map_policy.setting.updated && !pts_policy->updated)
		return;

	mapfilter_set_ts_default_8021q(ctx->map_policy.setting.primary_vid, ctx->map_policy.setting.dft.PCP);
	mapping = os_zalloc(pts_policy->SSID_num * sizeof(struct ssid_2_vid_mapping));
	if (!mapping)
		return;
	mapping_org = mapping;

	SLIST_FOREACH(policy_db, &(pts_policy->policy_head), policy_entry)
	{
		mapping->vlan_id = policy_db->vid;
		os_memcpy(mapping->ssid, policy_db->SSID, policy_db->SSID_len);
		debug(DEBUG_OFF, "set_traffic_policy:%s --> %d \n", mapping->ssid, mapping->vlan_id);
		vids[vid_num++] = policy_db->vid;
		mapping++;
	}

	mapfilter_set_ts_policy(pts_policy->SSID_num, mapping_org);

	/*notify the WiFi driver*/
	_1905_notify_ts_setting(ctx, pts_policy->SSID_num, mapping_org);

	os_free(mapping_org);

	/*fisrtly clear all vlan id and then re-set all vids included in traffic separation policy to switch*/
	eth_layer_set_traffic_separation_vlan(vids, 0);

	if (vid_num)
		eth_layer_set_traffic_separation_vlan(vids, vid_num);

	/* need rm hnat sessions after resetting the ts vlan */
	map_rm_hnat_session();

	/*reset policy*/
	ctx->map_policy.setting.updated = 0;
	pts_policy->updated = 0;

	update_operation_bss_vlan(ctx, &ctx->bss_head, pts_policy, NULL);
	maintain_all_assoc_clients(ctx);

}

void cmd_set_ts_pvlan(struct p1905_managerd_ctx *ctx, char* buf)
{
	struct def_8021q_setting *def_setting = &ctx->map_policy.setting;
	unsigned int pvid = 0;

	if (sscanf(buf, "%d", &pvid) < 0) {
		debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
		return;
	}
	def_setting->primary_vid = (unsigned short)pvid;
	debug(DEBUG_ERROR, "primary_vid(%d)\n", def_setting->primary_vid);
//	mapfilter_set_ts_default_8021q(ctx->map_policy.setting.primary_vid, ctx->map_policy.setting.dft.PCP);
}

void cmd_set_ts_policy(struct p1905_managerd_ctx *ctx, char* buf)
{
	unsigned int vid = 0;
	char ssid[64];
	struct traffic_separation_db *trffic_policy = NULL;
	struct traffics_policy *policy = &ctx->map_policy.tpolicy;
	memset(ssid, 0, sizeof(ssid));
	/* <ssid> <vid> */
	if (sscanf(buf, "%64s %d", ssid, &vid) < 0) {
		debug(DEBUG_ERROR, "[%d]sscanf fail\n", __LINE__);
		return;
	}

	trffic_policy = (struct traffic_separation_db *)os_malloc(sizeof(struct traffic_separation_db));
	if (!trffic_policy) {
		debug(DEBUG_ERROR, "alloc struct traffic_separation_db fail\n");
		return;
	}
	trffic_policy->SSID_len = os_strlen(ssid);
	debug(DEBUG_ERROR, "SSID_len(%d)\n", trffic_policy->SSID_len);

	trffic_policy->SSID = os_malloc(trffic_policy->SSID_len + 1);
	if (!trffic_policy->SSID) {
		debug(DEBUG_ERROR, "kmalloc for trffic_policy->SSID fail\n");
		free(trffic_policy);
		return;
	}

	os_memset(trffic_policy->SSID, 0, trffic_policy->SSID_len + 1);
	os_memcpy(trffic_policy->SSID, ssid, trffic_policy->SSID_len);
	trffic_policy->vid = (unsigned short)vid;
	debug(DEBUG_ERROR, "SSID(%s) VID(%d)\n", trffic_policy->SSID, trffic_policy->vid);

	policy->SSID_num ++;
	debug(DEBUG_ERROR, "current SSID_num(%d)\n", policy->SSID_num);

	SLIST_INSERT_HEAD(&policy->policy_head, trffic_policy, policy_entry);
}

void cmd_set_ts_policy_done(struct p1905_managerd_ctx *ctx)
{
	ctx->map_policy.setting.updated = 1;
	ctx->map_policy.tpolicy.updated = 1;
	map_r2_notify_ts_config((void*)ctx, NULL);
}

void cmd_set_ts_policy_clear(struct p1905_managerd_ctx *ctx)
{
	ctx->map_policy.setting.primary_vid = VLAN_N_VID;
	ctx->map_policy.setting.dft.PCP = 0;
	delete_exist_traffic_policy(ctx, &ctx->map_policy.tpolicy);
	ctx->map_policy.setting.updated = 1;
	ctx->map_policy.tpolicy.updated = 1;
	map_r2_notify_ts_config((void*)ctx, NULL);
}

int parse_tunneled_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == SOURCE_INFO_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == TUNNELED_MESSAGE_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		}  else if (*type == TUNNELED_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 1) {
		debug(DEBUG_ERROR, "error when check tunneled message tlvs\n");
		return -1;
	}
	return 0;
}

int parse_assoc_status_notification_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == ASSOCIATION_STATUS_NOTIFICATION_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 1) {
		debug(DEBUG_ERROR, "error when check tunneled message tlvs\n");
		return -1;
	}

	return 0;
}

int parse_cac_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == CAC_REQUEST_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 1) {
		debug(DEBUG_ERROR, "error when check cac request tlvs\n");
		return -1;
	}

	return 0;
}

int parse_cac_terminate_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == CAC_TERMINATION_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 1) {
		debug(DEBUG_ERROR, "error when check cac terminate tlvs\n");
		return -1;
	}

	return 0;
}


int parse_client_disassciation_stats_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == STA_MAC_ADDRESS_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == REASON_CODE_TYPE) {
			integrity |= (1 << 1);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == ASSOC_STA_TRAFFIC_STATS_TYPE) {
			integrity |= (1 << 2);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity != 0x7) {
		debug(DEBUG_ERROR, "error when check client disassciation stats tlvs\n");
		return -1;
	}

	return 0;
}

int parse_backhaul_sta_cap_report_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == BACKHAUL_STA_RADIA_CAPABILITY_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity == 0x0) {
		debug(DEBUG_ERROR, "error when check backhaul sta cap report tlvs\n");
		return -1;
	}
	return 0;
}

int parse_failed_connection_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned int tlv_len = 0;
	unsigned short len = 0;
	unsigned char integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			return -1;
		}

		if (*type == STA_MAC_ADDRESS_TYPE) {
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == ASSOCIATED_STATUS_CODE_TYPE) {
			integrity |= (1 << 1);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == REASON_CODE_TYPE) {
			integrity |= (1 << 2);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}
		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity < 0x3) {
		debug(DEBUG_ERROR, "error when check failed connection tlvs\n");
		return -1;
	}

	return 0;
}

void traffic_separation_init(struct p1905_managerd_ctx *ctx)
{
	int i = 0;

	ctx->map_policy.setting.primary_vid = INVALID_VLAN_ID;
	ctx->map_policy.setting.dft.PCP = 0;
	SLIST_INIT(&ctx->map_policy.tpolicy.policy_head);
	ctx->map_policy.setting.updated = 1;
	ctx->map_policy.tpolicy.updated = 1;

	/*this field has been assigned while reading 1905d_cfg*/
	/*ctx->map_policy.trans_vlan.num = 0;

	for (i = 0; i < MAX_NUM_TRANSPARENT_VID; i++) {
		ctx->map_policy.trans_vlan.transparent_vids[i] = INVALID_VLAN_ID;
	}
	*/
	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		dl_list_init(&ctx->clients_table[i]);
	}

	eloop_register_timeout(0, 0, map_r2_notify_ts_config, (void *)ctx, NULL);
	eloop_register_timeout(25, 0, map_notify_transparent_vlan_setting, (void *)ctx, NULL);

	dl_list_init(&ctx->bss_head);
	if (ctx->map_version >= DEV_TYPE_R2) {
		/*enable TS in mapfilter*/
		mapfilter_ts_onoff(1);
	}

}

void ts_traffic_separation_deinit(struct p1905_managerd_ctx *ctx)
{
	struct assoc_client *client = NULL, *client_tmp = NULL;
	struct operational_bss *bss = NULL, *bss_tmp = NULL;
	int i = 0;

	delete_exist_traffic_policy(ctx, &ctx->map_policy.tpolicy);
	ctx->map_policy.setting.primary_vid = INVALID_VLAN_ID;
	ctx->map_policy.setting.dft.PCP = 0;

	ctx->map_policy.setting.updated = 1;
	ctx->map_policy.tpolicy.updated = 1;

	map_r2_notify_ts_config((void*)ctx, NULL);

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		dl_list_for_each_safe(client, client_tmp, &ctx->clients_table[i],
			struct assoc_client, entry) {
			dl_list_del(&client->entry);
			mapfilter_update_client_vid(client->mac, client->vlan_id, STATION_LEAVE,
				client->ssid_len, client->ssid, client->al_mac);
			os_free(client);
		}
	}

	dl_list_for_each_safe(bss, bss_tmp, &ctx->bss_head, struct operational_bss, entry) {
		dl_list_del(&bss->entry);
		os_free(bss);
	}

	/*disable TS in mapfilter*/
	mapfilter_ts_onoff(0);
}

int update_traffic_separation_policy(struct p1905_managerd_ctx *ctx,
	struct set_config_bss_info bss_info[], unsigned char bss_num)
{
	unsigned char i = 0;
	struct traffic_separation_db *traffic_policy = NULL;
	struct traffics_policy *policy = &ctx->map_policy.tpolicy;

	for (i = 0; i < bss_num; i++) {
		if (bss_info[i].vid == INVALID_VLAN_ID)
			continue;

		traffic_policy = (struct traffic_separation_db*)os_malloc(sizeof(struct traffic_separation_db));
		if (!traffic_policy) {
			debug(DEBUG_ERROR, "fail to allocate memory\n");
			continue;
		}

		traffic_policy->vid = bss_info[i].vid;
		traffic_policy->SSID_len = bss_info[i].ssid_len;
		traffic_policy->SSID = os_malloc(bss_info[i].ssid_len + 1);
		if (!traffic_policy->SSID) {
			os_free(traffic_policy);
			debug(DEBUG_ERROR, "fail to allocate SSID\n");
			continue;
		}
		os_memset(traffic_policy->SSID, 0, bss_info[i].ssid_len + 1);
		os_memcpy(traffic_policy->SSID, bss_info[i].ssid, bss_info[i].ssid_len);
		SLIST_INSERT_HEAD(&policy->policy_head, traffic_policy, policy_entry);
		policy->SSID_num++;

		policy->updated = 1;
	}


	return 0;
}

void map_notify_transparent_vlan_setting(void *eloop_ctx, void *timeout_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;

	if (!ctx->map_policy.trans_vlan.updated)
		return;

	mapfilter_set_transparent_vid(ctx->map_policy.trans_vlan.transparent_vids,
		ctx->map_policy.trans_vlan.num);
	_1905_notify_transparent_vlan_setting(ctx);

	/*firstly cleart the original transparent vlan ids, and the re-set new ones to switch*/
	eth_layer_set_switch_trasnparent_vlan(ctx->map_policy.trans_vlan.transparent_vids,
		0);

	if (ctx->map_policy.trans_vlan.num)
		eth_layer_set_switch_trasnparent_vlan(ctx->map_policy.trans_vlan.transparent_vids,
			ctx->map_policy.trans_vlan.num);

	ctx->map_policy.trans_vlan.updated = 0;
}

struct operational_bss *get_operational_bss_by_bssid(struct dl_list *bss_head,
	unsigned char bssid[])
{
	struct operational_bss *bss = NULL;

	dl_list_for_each(bss, bss_head, struct operational_bss, entry) {
		if (!os_memcmp(bss->mac, bssid, ETH_ALEN))
			return bss;
	}

	return NULL;
}

unsigned short get_vlan_by_ssid_from_ts_policy(char *ssid, unsigned char ssid_len,
	struct traffics_policy *policy)
{
	unsigned short vlan = INVALID_VLAN_ID;
	struct traffic_separation_db *policy_db = NULL;

	SLIST_FOREACH(policy_db, &(policy->policy_head), policy_entry)
	{
		if (ssid_len == policy_db->SSID_len && !os_memcmp(policy_db->SSID, ssid, ssid_len)) {
			vlan = policy_db->vid;
			break;
		}
	}

	return vlan;
}

void update_operation_bss_vlan(struct p1905_managerd_ctx *ctx,struct dl_list *bss_head,
	struct traffics_policy *policy, unsigned char *al_mac)
{
	struct operational_bss *bss = NULL;
	unsigned short vid = INVALID_VLAN_ID;
	struct topology_response_db *tpgr_db = NULL;
	enum MAP_PROFILE profile = MAP_PROFILE_REVD;

	dl_list_for_each(bss, bss_head, struct operational_bss, entry) {

		/*if only update bss for a specific almac*/
		if (al_mac && os_memcmp(al_mac, bss->almac, ETH_ALEN))
			continue;

		profile = MAP_PROFILE_REVD;
		tpgr_db = find_topology_rsp_by_almac(ctx, bss->almac);
		if (tpgr_db) {
			profile = tpgr_db->profile;
		} else {
			if (!os_memcmp(bss->almac, ctx->p1905_al_mac_addr, ETH_ALEN))
				profile = ctx->map_version;
		}
		if (profile == MAP_PROFILE_REVD || profile == MAP_PROFILE_R1)
			continue;

		vid = get_vlan_by_ssid_from_ts_policy(bss->ssid, os_strlen(bss->ssid), policy);

		if (bss->vid != vid) {
			debug(DEBUG_ERROR, "update bss("MACSTR") %s vlan from %d to %d",
				PRINT_MAC(bss->mac), bss->ssid, bss->vid, vid);
			bss->vid = vid;
		}

	}
}

void maintain_all_assoc_clients(struct p1905_managerd_ctx *ctx)
{
	struct dl_list *clients = NULL;
	struct assoc_client *client = NULL, *client_temp = NULL;
	struct operational_bss *bss = NULL;
	int i = 0;

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		clients = &ctx->clients_table[i];
		dl_list_for_each_safe (client, client_temp, clients, struct assoc_client, entry) {
			if (client->disassoc) {
				debug(DEBUG_TRACE, "client delete "MACSTR"\n",
					PRINT_MAC(client->mac));
				dl_list_del(&client->entry);
				mapfilter_update_client_vid(client->mac, client->vlan_id, STATION_LEAVE,
					client->ssid_len, client->ssid, client->al_mac);
				os_free(client);
				continue;
			}
			bss = get_operational_bss_by_bssid(&ctx->bss_head, client->bssid);

			if (!bss)
				continue;

			/*if vid/ssid of bss has been changed, should reconfig client and set to mapfilter*/
			if (client->vlan_id != bss->vid || client->ssid_len != os_strlen(bss->ssid) ||
				os_strncmp(client->ssid, bss->ssid, client->ssid_len)) {
				client->vlan_id = bss->vid;
				if (os_strlen(bss->ssid) >= MAX_SSID_LEN)
					continue;

				client->ssid_len = os_strlen(bss->ssid);
				os_memset(client->ssid, 0, sizeof(client->ssid));
				os_strncpy(client->ssid, bss->ssid, client->ssid_len);

				client->apply_2_mapfilter = 0;
			}

			/*if (client->vlan_id != bss->vid) {*/
			if (!client->apply_2_mapfilter) {
				client->apply_2_mapfilter = 1;
				mapfilter_update_client_vid(client->mac, client->vlan_id, STATION_JOIN,
					client->ssid_len, client->ssid, client->al_mac);
			}
		}
	}
}

void show_all_assoc_clients(struct p1905_managerd_ctx *ctx)
{
	struct dl_list *clients = NULL;
	struct assoc_client *client = NULL;
	int i = 0;

	printf("index	mac	bssid	vid\n");
	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		clients = &ctx->clients_table[i];
		dl_list_for_each(client, clients, struct assoc_client, entry) {
			printf("[%d] "MACSTR" "MACSTR" %d\n",
				i, PRINT_MAC(client->mac), PRINT_MAC(client->bssid), client->vlan_id);
		}
	}
}

void show_all_operational_bss(struct p1905_managerd_ctx *ctx)
{
	struct operational_bss *bss = NULL;
	int i = 0;

	printf("show_all_operational_bss\n");
	printf("index\tmac\t\tssid\tvid\talmac\n");
	dl_list_for_each(bss, &ctx->bss_head, struct operational_bss, entry) {
		printf("[%d] "MACSTR" %s %d "MACSTR"\n", i,
			PRINT_MAC(bss->mac), bss->ssid, bss->vid, PRINT_MAC(bss->almac));
		i++;
	}
}

unsigned short append_backhaul_sta_radio_cap_tlv(unsigned char *pkt,
    unsigned char *rid, unsigned char *itf_mac)
{
    unsigned short total_length = 0;

    *pkt++ = BACKHAUL_STA_RADIA_CAPABILITY_TYPE;

	if (itf_mac) {
		*(unsigned short *)(pkt) = host_to_be16(13);
		total_length = 16;
	} else {
		*(unsigned short *)(pkt) = host_to_be16(7);
		total_length = 10;
	}
	pkt += 2;

	 /* Radio Unique Identifier */
	 memcpy(pkt,rid,ETH_ALEN);
	 pkt += ETH_ALEN;

	/* MAC address inlucde bit (bit 7)
	** 0: the MAC address is not included below
	** 1: the MAC address is included below
	*/
	if (itf_mac) {
		*pkt++ = 0x80;

		/* MAC address of the backhaul STA on this radio */
	    memcpy(pkt,itf_mac,ETH_ALEN);
	} else {
		*pkt++ = 0x0;
	}

    return total_length;
}


void delete_exist_radio_identifier(struct p1905_managerd_ctx *ctx)
{
	struct radio_identifier_db *p_tmp = NULL, *p_next = NULL;

	if(!SLIST_EMPTY(&ctx->metric_entry.radio_identifier_head))
	{
		p_tmp = SLIST_FIRST(&ctx->metric_entry.radio_identifier_head);
		while (p_tmp)
		{
			debug(DEBUG_TRACE, "remove radio identifier("MACSTR") \n", PRINT_MAC(p_tmp->identifier));

			p_next = SLIST_NEXT(p_tmp, radio_identifier_entry);

			SLIST_REMOVE(&ctx->metric_entry.radio_identifier_head, p_tmp,
				radio_identifier_db, radio_identifier_entry);
			free(p_tmp);
			p_tmp = p_next;
		}
	}
}

void add_new_radio_identifier(struct p1905_managerd_ctx *ctx, unsigned char *identifier)
{
	struct radio_identifier_db *p_new = NULL;

	p_new = (struct radio_identifier_db *)malloc(sizeof(struct radio_identifier_db));

	if (!p_new) {
		return;
	}

	memcpy(p_new->identifier, identifier, ETH_ALEN);
	debug(DEBUG_TRACE, "add radio identifier("MACSTR") \n", PRINT_MAC(p_new->identifier));

	SLIST_INSERT_HEAD(&ctx->metric_entry.radio_identifier_head, p_new, radio_identifier_entry);
}

int parse_ap_radio_advanced_capability_tlv(struct p1905_managerd_ctx *ctx,
		unsigned char *almac, struct ts_cap_db **ts_cap, unsigned short len, unsigned char *value)
{
	unsigned char *temp_buf = value;
	struct agent_list_db *agent_info = NULL;
	struct ts_cap_db *ts_cap_tmp = NULL;

	if (!temp_buf || !ctx || !almac || !ts_cap) {
		debug(DEBUG_ERROR, "params null!!! value(%p) ctx(%p) almac(%p) ts_cap(%p)\n",
			temp_buf, ctx, almac, ts_cap);
		return -1;
	}

	if (len != 7) {
		debug(DEBUG_ERROR, "tlv length error!!! length should be 7 but is %d\n", len);
		return -1;
	}

	/*fill in agent Profile-2 AP Capability*/
	find_agent_info(ctx, almac, &agent_info);
	if (!agent_info) {
		debug(DEBUG_ERROR, "error! no agent info exist try to insert a new.\n");
		agent_info = insert_agent_info(ctx, almac);
		if (!agent_info) {
			debug(DEBUG_ERROR, "insert new agent info failed.\n");
			return -1;
		}
	}

	SLIST_FOREACH(ts_cap_tmp, &agent_info->ts_cap_head, ts_cap_entry)
	{
		if (!os_memcmp(ts_cap_tmp->identifier, temp_buf, ETH_ALEN)) {
			temp_buf += ETH_ALEN;
			ts_cap_tmp->ts_combined_fh = *temp_buf & 0x80;
			ts_cap_tmp->ts_combined_bh = *temp_buf & 0x40;
			temp_buf++;
			break;
		}
	}
	if (!ts_cap_tmp) {
		ts_cap_tmp = (struct ts_cap_db *)os_zalloc(sizeof(struct ts_cap_db));
		if (!ts_cap_tmp) {
			debug(DEBUG_ERROR, "alloc struct ts_cap_db fail\n");
			return -1;
		}
		os_memcpy(ts_cap_tmp->identifier, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;
		ts_cap_tmp->ts_combined_fh = *temp_buf & 0x80;
		ts_cap_tmp->ts_combined_bh = *temp_buf & 0x40;
		temp_buf++;
		SLIST_INSERT_HEAD(&agent_info->ts_cap_head, ts_cap_tmp, ts_cap_entry);
	}

	*ts_cap=ts_cap_tmp;

	return 0;
}


int parse_r2_ap_capability_tlv(struct ap_r2_capability *r2_cap, unsigned char *almac,
				  unsigned short len, unsigned char *value)
{
#define R2_AP_CABABILITY_TLV_LEN 4
	unsigned char *temp_buf = value;

	if (len != R2_AP_CABABILITY_TLV_LEN)
		return -1;

	/*fill in agent Profile-2 AP Capability*/
	os_memset(r2_cap, 0, sizeof(struct ap_r2_capability));
	/* Max Total Number Service Prioritization Rules*/
	r2_cap->max_total_num_sp_rules = *temp_buf;
	temp_buf++;

	/*skip Reserved byte*/
	temp_buf++;

	/*parse capa bit map*/
	if (*temp_buf & 0x40)
		r2_cap->byte_counter_units = 1;
	else if (*temp_buf & 0x80)
		r2_cap->byte_counter_units = 2;

	if (*temp_buf & 0x20)
		r2_cap->sp_flag = 1;
	if (*temp_buf & 0x10)
		r2_cap->dpp_flag = 1;
	if (*temp_buf & 0x08)
		r2_cap->ts_flag = 1;
	temp_buf++;

	/*Max Total Number of VIDs.*/
	r2_cap->max_total_num_vid = *temp_buf;
	temp_buf++;

	debug(DEBUG_TRACE, "agent("MACSTR") sp_num: %d, dpp_flag:%d, sp_flag:%d, ts_flag:%d, byte_units:%d, num_vid:%d\n",
		PRINT_MAC(almac), r2_cap->max_total_num_sp_rules,
		r2_cap->dpp_flag, r2_cap->sp_flag, r2_cap->ts_flag,
		r2_cap->byte_counter_units, r2_cap->max_total_num_vid);

	return 0;
}

#endif // #ifdef MAP_R2

#ifdef MAP_R3

extern unsigned char p1905_multicast_address[6];

/* action frame type from DPP SPEC R2 */
char *actionFrame_typeNum2str(unsigned char type)
{
    //debug(DEBUG_ERROR, "type 0x%02x\n", type);
    switch (type) {
        case DPP_PA_AUTHENTICATION_REQ:
            return "Authentication Request";
        case DPP_PA_AUTHENTICATION_RESP:
            return "Authentication Response";
        case DPP_PA_AUTHENTICATION_CONFIRM:
            return "Authentication Confirm";
        case DPP_PA_PEER_DISCOVERY_REQ:
            return "Peer Discovery Request";
        case DPP_PA_PEER_DISCOVERY_RESP:
            return "Peer Discovery Response";
        case DPP_PA_PKEX_EXCHANGE_REQ:
            return "PKEX Exchange Request";
        case DPP_PA_PKEX_EXCHANGE_RESP:
            return "PKEX Exchange Response";
        case DPP_PA_PKEX_COMMIT_REVEAL_REQ:
            return "PKEX Commit-Reveal Request";
        case DPP_PA_PKEX_COMMIT_REVEAL_RESP:
            return "PKEX Commit-Reveal Response";
        case DPP_PA_CONFIGURATION_RESULT:
            return "Configuration Result";
        case DPP_PA_CONNECTION_STATUS_RESULT:
            return "Connection Status Result";
        case DPP_PA_PRESENCE_ANNOUNCEMENT:
            return "DPP Presence Announcement";
        case DPP_PA_RECONFIG_ANNOUNCEMENT:
            return "DPP Reconfig Announcement";
        case DPP_PA_RECONFIG_AUTH_REQ:
            return "DPP Reconfig Auth Req";
        case DPP_PA_RECONFIG_AUTH_RESP:
            return "DPP Reconfig Auth Resp";
        case DPP_PA_RECONFIG_AUTH_COMFIRM:
            return "DPP Reconfig Auth Confirm";
        default:
            return "unsupported type";
    }
}


char *GasFrame_actionNum2str(unsigned char action)
{
    debug(DEBUG_ERROR, "action 0x%02x\n", action);
    switch (action) {
        case 0x0A:
        case 0x0C:
            return "DPP Configuration Request";
        case 0x0B:
        case 0x0D:
            return "DPP Configuration Response";
        default:
            return "unsupported type";
    }
}

int dev_send_eapol(void *ctx, unsigned char *dst_almac, unsigned char *eapol_frame, unsigned short eapol_len)
{
	struct p1905_managerd_ctx *pctx = (struct p1905_managerd_ctx *)ctx;

	reset_send_tlv(pctx);
	if (fill_send_tlv(pctx, eapol_frame, eapol_len) == -1)
		debug(DEBUG_ERROR, "[%d]fill_send_tlv fail!\n", __LINE__);
	insert_cmdu_txq(dst_almac, pctx->p1905_al_mac_addr,
			e_1905_encap_eapol, ++pctx->mid, pctx->br_name, 0);
	process_cmdu_txq(pctx, eapol_frame);

	return 0;
}


unsigned short bss_reconfiguration_trigger_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(BSS_RECONFIGURATION_TRIGGER_MESSAGE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}



unsigned short _1905_rekey_request_message(unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	/*no content*/

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	*-8(cmdu header size)) ==>padding
	*/
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(_1905_REKEY_REQUEST);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}

/* If a Decryption Failure Counter exceeds the threshold
** (configured by the Multi-AP Controller),
**the Multi-AP Agent shall send a Decryption Failure message to the Multi-AP Controller
*/
unsigned short _1905_decryption_failure_message(
	struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned char *al_mac)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	/*One 1905.1 AL MAC address type TLV*/

	length = append_1905_al_mac_addr_type_tlv(tlv_temp_buf, al_mac);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	*-8(cmdu header size)) ==>padding
	*/
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(_1905_DECRYPTION_FAILURE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}


unsigned short proxied_encap_dpp_message(unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = ctx->send_tlv_len;
	memcpy(tlv_temp_buf, ctx->send_tlv, ctx->send_tlv_len);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(PROXIED_ENCAP_DPP_MESSAGE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}



unsigned short direct_encap_dpp_message(unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = ctx->send_tlv_len;
	memcpy(tlv_temp_buf, ctx->send_tlv, ctx->send_tlv_len);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(DIRECT_ENCAP_DPP_MESSAGE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}





unsigned short dpp_uri_notification_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned short reason)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_reason_code_tlv(tlv_temp_buf, reason);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(DPP_URI_NOTIFICATION_MESSAGE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}



unsigned short _1905_encap_eapol_message(unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_1905_encap_eapol_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(_1905_ENCAP_EAPOL);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}


unsigned short dpp_bootstrap_uri_notification_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_bootstrap_uri_notification_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(DPP_BOOTSTRAPING_URI_NOTIFICATION);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}

unsigned short dpp_bootstrap_uri_query_message(unsigned char *buf,
 	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(DPP_BOOTSTRAPING_URI_QUERY);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}

int append_dpp_cce_indication_tlv(unsigned char *pkt, struct p1905_managerd_ctx* ctx)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;

	temp_buf = pkt;
	(*temp_buf) = DPP_CCE_INDICATION_TYPE;
	temp_buf += 1;

	*(unsigned short *)temp_buf = host_to_be16(2);
	temp_buf += 2;

	/* 1: enable
	** 2: disable CCE indication
	*/
	*temp_buf++ = 1;

	total_length = (unsigned short)(temp_buf - pkt);
	return total_length;
}


unsigned short dpp_cce_indication_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = ctx->send_tlv_len;
	memcpy(tlv_temp_buf, ctx->send_tlv, ctx->send_tlv_len);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(DPP_CCE_INDICATION_MESSAGE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}


int append_akm_suite_cap_tlv(unsigned char *pkt, struct p1905_managerd_ctx* ctx)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;
	struct akm_suite_cap *p_akm = NULL;
    struct akm_suit * p_akm_suit = NULL;
    int i = 0;

	temp_buf = pkt;
	(*temp_buf) = AKM_SUITE_CAPABILITY_TYPE;
	temp_buf += 3;

	/* Num BH BSS AKM Suite Selectors	*/
	p_akm = &ctx->bh_akm;
	*temp_buf++ = p_akm->akm_suite_cnt;
    debug(DEBUG_ERROR, "Num BH AKM Suite %d\n", p_akm->akm_suite_cnt);
	/* where does the akm suits come frome */
	for (i = 0; i < p_akm->akm_suite_cnt; i ++) {
        p_akm_suit =  &p_akm->akm_suite_info[i];
		memcpy(temp_buf, p_akm_suit->oui, 3);
		temp_buf += 3;

		*temp_buf++ = p_akm_suit->akm_suit_type;

        debug(DEBUG_ERROR, "suite type %d, oui %02x%02x%02x\n",
            p_akm->akm_suite_cnt, p_akm_suit->oui[0], p_akm_suit->oui[1],
            p_akm_suit->oui[2]);
	}

	/* Num FH BSS AKM Suite Selectors	*/
	p_akm = &ctx->fh_akm;
	*temp_buf++ = p_akm->akm_suite_cnt;
    debug(DEBUG_ERROR, "Num FH AKM Suite %d\n", p_akm->akm_suite_cnt);
	/* where does the akm suits come frome */
	for (i = 0; i < p_akm->akm_suite_cnt; i ++) {
        p_akm_suit =  &p_akm->akm_suite_info[i];
		memcpy(temp_buf, p_akm_suit->oui, 3);
		temp_buf += 3;

		*temp_buf++ = p_akm_suit->akm_suit_type;

        debug(DEBUG_ERROR, "suite type %d, oui %02x%02x%02x\n",
            p_akm->akm_suite_cnt, p_akm_suit->oui[0], p_akm_suit->oui[1],
            p_akm_suit->oui[2]);
	}

	total_length = (unsigned short)(temp_buf - pkt);
	*(unsigned short *)(pkt + 1) = host_to_be16(total_length - 3);
	return total_length;
}





/* Once an Enrollee Multi-AP Agent has established a PMK and PTK with the
** Controller at 1905 layer,it shall request configuration for its fBSSs
** and bBSSs by sending a 1905 BSS Configuration Request message to the Controller
*/
unsigned short bss_configuration_request_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;
	struct ap_radio_advan_cap cap;
	unsigned char radio_index = 0;
	int i = 0;
	struct radio_info *r = NULL;

	msg_hdr = (cmdu_message_header*)buf;

	/* version */
	length = append_map_version_tlv(tlv_temp_buf, ctx->map_version);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/* one SupportedService TLV */
	length = append_supported_service_tlv(tlv_temp_buf, ctx->service);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/* One AKM Suite Capabilities TLV */
	length = append_akm_suite_cap_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	for (radio_index = 0; radio_index < ctx->radio_number; radio_index++) {
		r = &ctx->rinfo[radio_index];
		debug(DEBUG_ERROR, "identifier="MACSTR"\n", PRINT_MAC(r->identifier));
		length = append_ap_radio_basic_capability_tlv(ctx, tlv_temp_buf, r);
		total_tlvs_length += length;
		tlv_temp_buf += length;

		/* One or more AP Radio Advanced Capabilities TLV */
		os_memcpy(cap.radioid, r->identifier, ETH_ALEN);
		cap.flag = 0xc0;
		length = append_ap_radio_advan_tlv(tlv_temp_buf, &cap);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}

	/* Zero or more Backhaul STA Radio Capabilities TLV */
	for (i = 0; i < ctx->itf_number; i++) {
		if (ctx->itf[i].is_wifi_sta) {
			length = append_backhaul_sta_radio_cap_tlv(tlv_temp_buf, ctx->itf[i].identifier, ctx->itf[i].mac_addr);
			total_tlvs_length += length;
			tlv_temp_buf += length;
		}
	}

	/* One Profile-2 AP Capability TLV */
	length = append_r2_cap_tlv(tlv_temp_buf, &(ctx->ap_cap_entry.ap_r2_cap));
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_bss_configuration_request_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		os_memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(BSS_CONFIGURATION_REQUEST_MESSAGE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}

unsigned char *build_config_obj_attr(unsigned char *pkt, struct p1905_managerd_ctx* ctx,
	struct r3_member *peer, unsigned char *radio_identifier, unsigned char bss_index,
	unsigned char bss_found, unsigned char bss_type)
{
#define JSON_LEN 2048
	unsigned char *temp_buf = pkt;
	unsigned short per_obj_len = 0;
	unsigned short authmode = 0;
	char pass[63 * 6 + 1];
	char ssid[32 * 4 + 1];
	char identifier_str[20] = {0};
	struct wpabuf *buf;
	char json[JSON_LEN];
	char authStr[32 + 1];
	unsigned char akm_idx = 0;
	struct bss_connector_item *bc_item =NULL, *bc_item_nxt = NULL;
	int found = 0;
	int res = 0;

	per_obj_len = 0;

	/* update */
	dl_list_for_each_safe(bc_item, bc_item_nxt,
		&ctx->r3_dpp.bss_connector_list, struct bss_connector_item, member) {
		if (os_memcmp(bc_item->almac, peer->al_mac, ETH_ALEN) == 0) {
			found = 1;
			break;
		}
	}

	if (!found) {
		debug(DEBUG_ERROR, "not found bss connector by almac "MACSTR"\n", MAC2STR(peer->al_mac));
		return pkt;
	}

	os_memset(json, 0, sizeof(json));
	buf = wpabuf_alloc(512+bc_item->bh_connector_len);
	if (!buf) {
		debug(DEBUG_ERROR, "wpabuf_alloc failed\n");
		return pkt;
	}

	if (bss_found && bss_type & BIT_BH_BSS)
		/* wi-fi tech: map */
		per_obj_len += snprintf(json+per_obj_len, sizeof(json), "{\"wi-fi_tech\":\"map\",\"discovery\":");
	else
		/* wi-fi tech: inframap */
		per_obj_len += snprintf(json+per_obj_len, sizeof(json), "{\"wi-fi_tech\":\"inframap\",\"discovery\":");

	/* Radio identifier */
	wpa_snprintf_hex(identifier_str, sizeof(identifier_str), radio_identifier, ETH_ALEN);
	per_obj_len += snprintf(json+per_obj_len, sizeof(json) - per_obj_len, "{\"RUID\":\"%s\"", identifier_str);


	if (bss_found) {
		os_memset(ssid, 0, sizeof(ssid));
		json_escape_string(ssid, sizeof(ssid),
			(const char *)ctx->bss_config[bss_index].ssid,
			os_strlen(ctx->bss_config[bss_index].ssid));
		per_obj_len += snprintf(json+per_obj_len, sizeof(json) - per_obj_len, ",\"ssid\":\"%s\"", ssid);
	} else
		per_obj_len += snprintf(json+per_obj_len, sizeof(json) - per_obj_len, ",\"ssid\":\"\"");

	if (!ctx->MAP_Cer && ctx->bss_config[bss_index].hidden_ssid)
		per_obj_len += snprintf(json+per_obj_len, sizeof(json) - per_obj_len, ",\"hidden-ssid\":\"1\"");

	/*
	** BSSID value only used by reconfiguration
	** ignore here
	*/
	per_obj_len += snprintf(json+per_obj_len, sizeof(json) - per_obj_len, ",\"BSSID\":\"000000000000\"");
	per_obj_len += snprintf(json+per_obj_len, sizeof(json) - per_obj_len, "}");

	if (bss_found) {
		os_memset(authStr, 0, sizeof(authStr));
		/* CERT mode, authmode from wts file */
		if (ctx->MAP_Cer) {
			if (ctx->bss_config[bss_index].authmode & AUTH_DPP_ONLY)
				authmode |= AUTH_DPP_ONLY;
			if (ctx->bss_config[bss_index].authmode & AUTH_SAE_PERSONAL)
				authmode |= AUTH_SAE_PERSONAL;
			if (ctx->bss_config[bss_index].authmode & AUTH_WPA2_PERSONAL)
				authmode |= AUTH_WPA2_PERSONAL;
		}
		/* Turnkey mode, authmode from akm tlv in bss config request */
		else {
			/* turnkey need check akm in wts as well */
			for (akm_idx = 0; akm_idx < peer->bh_akm.akm_suite_cnt; akm_idx ++) {
				if ((!os_memcmp(peer->bh_akm.akm_suite_info[akm_idx].oui, OUI_AKM_DPP, 3)) &&
					(peer->bh_akm.akm_suite_info[akm_idx].akm_suit_type == OUI_AKM_DPP[3]) &&
					(ctx->bss_config[bss_index].authmode & AUTH_DPP_ONLY))
					authmode |= AUTH_DPP_ONLY;
				if ((!os_memcmp(peer->bh_akm.akm_suite_info[akm_idx].oui, OUI_WPA2_AKM_SAE_SHA256, 3)) &&
					(peer->bh_akm.akm_suite_info[akm_idx].akm_suit_type == OUI_WPA2_AKM_SAE_SHA256[3]) &&
					(ctx->bss_config[bss_index].authmode & AUTH_SAE_PERSONAL))
					authmode |= AUTH_SAE_PERSONAL;
				if ((!os_memcmp(peer->bh_akm.akm_suite_info[akm_idx].oui, OUI_WPA2_AKM_PSK, 3)) &&
					((peer->bh_akm.akm_suite_info[akm_idx].akm_suit_type == OUI_WPA2_AKM_PSK[3]) ||
					 (peer->bh_akm.akm_suite_info[akm_idx].akm_suit_type == OUI_WPA2_AKM_PSK_SHA256[3])) &&
					(ctx->bss_config[bss_index].authmode & AUTH_WPA2_PERSONAL))
					authmode |= AUTH_WPA2_PERSONAL;
			}
		}

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
		else if ((authmode & AUTH_SAE_PERSONAL) &&
				(authmode & AUTH_WPA2_PERSONAL)) {
			os_snprintf(authStr, sizeof(authStr), "sae+psk");
		}
		else if (authmode & AUTH_SAE_PERSONAL) {
			os_snprintf(authStr, sizeof(authStr), "sae");
		}
		else if (authmode & AUTH_WPA2_PERSONAL) {
			os_snprintf(authStr, sizeof(authStr), "psk");
		}

		/* akm:  */
		per_obj_len += snprintf(json+per_obj_len, sizeof(json) - per_obj_len, ",\"cred\":{\"akm\":\"%s\"", authStr);

		/* pass: WPA2 Passphrase */
		json_escape_string(pass, sizeof(pass),
			(const char *)ctx->bss_config[bss_index].key,
			os_strlen(ctx->bss_config[bss_index].key));

		if (os_strlen(pass) >= 8 && os_strlen(pass) <= 63)
			res = os_snprintf(json+per_obj_len, JSON_LEN - per_obj_len, ",\"pass\":\"%s\"", pass);
		else if (os_strlen(pass) == 64)
			res = os_snprintf(json+per_obj_len, JSON_LEN - per_obj_len, ",\"psk_hex\":\"%s\"", pass);
		else {
			debug(DEBUG_ERROR, "invalid passphrase length %d\n", (int)os_strlen(pass));
			goto fail;
		}

		if (os_snprintf_error(JSON_LEN - per_obj_len, res))
			goto fail;

		per_obj_len += res;

		/* add pre-shared key but no value
		per_obj_len += sprintf(json+per_obj_len, ",\"psk_hex\":\"\"");*/

		/* connector for BH or FH */
		if (bss_type & BIT_BH_BSS)
			per_obj_len += snprintf(json+per_obj_len, sizeof(json) - per_obj_len, ",%s", bc_item->bh_connector);
		else
			per_obj_len += snprintf(json+per_obj_len, sizeof(json) - per_obj_len, ",%s", bc_item->fh_connector);
		per_obj_len += snprintf(json+per_obj_len, sizeof(json) - per_obj_len, "}");
	}
	per_obj_len += snprintf(json+per_obj_len, sizeof(json) - per_obj_len, "}");
	debug(DEBUG_ERROR, "bss conf JSON:%s\n", json);

	/* configAttrib */
	wpabuf_put_le16(buf, DPP_ATTR_CONFIG_OBJ);
	wpabuf_put_le16(buf, per_obj_len);

	wpabuf_put_data(buf, json, per_obj_len);

	/* copy each tlv to pkt */
	os_memcpy(temp_buf, wpabuf_head(buf), wpabuf_len(buf));

	temp_buf += wpabuf_len(buf);

fail:
	wpabuf_free(buf);

	return temp_buf;
}

unsigned char *build_bss_config_obj_attr(unsigned char *pkt, struct p1905_managerd_ctx* ctx,
	struct r3_member *peer, unsigned char *radio_identifier, unsigned char bss_index, unsigned char bss_found)
{
	unsigned char *temp_buf = pkt;

	if (bss_found) {
		if (ctx->bss_config[bss_index].wfa_vendor_extension & BIT_BH_BSS)
			temp_buf = build_config_obj_attr(temp_buf, ctx, peer, radio_identifier, bss_index, bss_found, BIT_BH_BSS);
		if (ctx->bss_config[bss_index].wfa_vendor_extension & BIT_FH_BSS)
			temp_buf = build_config_obj_attr(temp_buf, ctx, peer, radio_identifier, bss_index, bss_found, BIT_FH_BSS);
	} else {
		temp_buf = build_config_obj_attr(temp_buf, ctx, peer, radio_identifier, 0, bss_found, 0);
	}
	return temp_buf;
}

int precheck_obj_num(struct p1905_managerd_ctx *ctx, unsigned char *al_mac)
{
	unsigned char radio_idx = 0;
	struct agent_list_db *agent = NULL;
	struct agent_radio_info *radio = NULL;
	int k = 0;
	unsigned char band = BAND_INVALID_CAP;
	unsigned char bss_num_per_radio = 0;
	unsigned char total_bss_num = 0;
	unsigned char wild_card_mac[ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	find_agent_info(ctx, al_mac, &agent);
	if (!agent) {
		debug(DEBUG_ERROR, "error!agent not found"MACSTR"\n",
			PRINT_MAC(al_mac));
		return 0;
	}
	while (radio_idx < agent->radio_num) {
		bss_num_per_radio = 0;
		radio = &agent->ra_info[radio_idx];

		band = determin_band_config(ctx, al_mac, radio->band);
		/* idx in wts should start from last one of the radio */
		for (k = 0; k < ctx->bss_config_num; k++) {
			if (band == ctx->bss_config[k].band_support &&
				(!os_memcmp(al_mac, ctx->bss_config[k].mac, ETH_ALEN) ||
				!os_memcmp(ctx->bss_config[k].mac, wild_card_mac, ETH_ALEN))) {
				if (bss_num_per_radio < radio->max_bss_num) {
					total_bss_num++;
					bss_num_per_radio++;
				}
			}
		}
		radio_idx++;
	}
	return total_bss_num;
}

/*
**	BSS Configuration Response TLV 1
**		DPP Configuration Object (16bss of radio 1)
**	BSS Configuration Response TLV 2
**		DPP Configuration Object (rest of 16bss of radio 1 )
**	BSS Configuration Response TLV 3
**		DPP Configuration Object (16bss of radio 2)
**	BSS Configuration Response TLV 4
**		DPP Configuration Object (rest of 16bss of radio 2 )
**	BSS Configuration Response TLV 5
**		DPP Configuration Object (16bss of radio 3)
**	BSS Configuration Response TLV 6
**		DPP Configuration Object (rest of 16bss of radio 3)
*/
int append_bss_config_response_tlv(unsigned char *pkt, struct p1905_managerd_ctx* ctx, unsigned char *al_mac)
{
	unsigned char *temp_buf = NULL;
	/* base pointer of each tlv */
	unsigned char *tlv_base_ptr = NULL;
	unsigned short total_length = 0;
	unsigned char *p_tlv_len = 0;
	/* bss num in one bss config response tlv */
	unsigned char bss_num_in_tlv = 0;
	/* bss num per radio */
	unsigned char bss_num_per_radio = 0;
	/* idx in wts file for each radio */
	unsigned char bss_conf_idx = 0;
	unsigned char radio_idx = 0;
	//unsigned char bss_cnt[MAX_RADIO_NUM];
	struct agent_list_db *agent = NULL;
	struct agent_radio_info *radio = NULL;
	struct r3_member *peer = NULL;
	int k = 0;
	unsigned char band = BAND_INVALID_CAP;
	unsigned char wild_card_mac[ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	tlv_base_ptr = temp_buf = pkt;

	find_agent_info(ctx, al_mac, &agent);
	if (!agent) {
		debug(DEBUG_ERROR, "error!agent not found"MACSTR"\n",
			PRINT_MAC(al_mac));
		return 0;
	}

	peer = get_r3_member(al_mac);

	if (!peer) {
		debug(DEBUG_ERROR, "peer info not found\n");
		return 0;
	}

	while (radio_idx < agent->radio_num) {
		radio = &agent->ra_info[radio_idx];

		/* build TLV once:
		**  1. first radio && first time
		**  2. bss num in the tlv bigger than 16
		*/
		if ((radio_idx == 0 && bss_num_in_tlv == 0) ||
			(bss_num_in_tlv >= BSSNUM_PER_BSS_RESPONSE_TLV)) {

			/* reset bss num in the tlv to 0
			** and set base pointer for next tlv
			** once tlv num in the tlv bigger than 16
			*/
			if (bss_num_in_tlv >= BSSNUM_PER_BSS_RESPONSE_TLV) {
				bss_num_in_tlv = 0;
				tlv_base_ptr = temp_buf;
			}

			/* build bss config response tlv */
			*temp_buf++ = BSS_CONFIG_RESPONSE_TYPE;
			p_tlv_len = temp_buf;
			temp_buf += 2;

			debug(DEBUG_ERROR, "begin build bss config response tlv\n");

		}

		band = determin_band_config(ctx, al_mac, radio->band);
		/* NULL-SSID config obj if:
		**	1. no bss info in wts
		**	2. invalid band cap from agent
		*/
		if (band == BAND_INVALID_CAP) {
			debug(DEBUG_ERROR, "invalide band 0x%02x for radio"MACSTR"\n",
				radio->band, PRINT_MAC(radio->identifier));
			debug(DEBUG_ERROR, "build NULL-SSID config obj for bind(%d)\n", radio_idx);

			temp_buf = build_bss_config_obj_attr(temp_buf, ctx, peer, radio->identifier, 0, 0);

			radio_idx++;
		} else {
			debug(DEBUG_ERROR, "band cap 0x%02x for radio %d\n", band, radio_idx);
			/* idx in wts should start from last one of the radio */
			for (k = bss_conf_idx; k < ctx->bss_config_num; k++) {
				if (band == ctx->bss_config[k].band_support &&
					(!os_memcmp(al_mac, ctx->bss_config[k].mac, ETH_ALEN) ||
					!os_memcmp(ctx->bss_config[k].mac, wild_card_mac, ETH_ALEN))) {
					if ((bss_num_per_radio < radio->max_bss_num) &&
							bss_num_in_tlv < BSSNUM_PER_BSS_RESPONSE_TLV) {
						debug(DEBUG_ERROR, "build config obj for bss(%d) band(%d)\n",
							bss_num_per_radio, radio_idx);
						temp_buf = build_bss_config_obj_attr(
							temp_buf, ctx, peer, radio->identifier,
							k, 1);

						/* increasement bss_num of current radio */
						bss_num_per_radio++;
						bss_num_in_tlv++;

						/* store idx in wts to the last + 1 */
						bss_conf_idx = k+1;
					}
				}
			}

			if (!bss_num_per_radio) {
				/* build null obj when bss num eqaul to 0 */
				debug(DEBUG_ERROR, "build NULL-SSID config obj for bind(%d)\n", radio_idx);
				temp_buf = build_bss_config_obj_attr(temp_buf, ctx, peer, radio->identifier, 0, 0);
			} else {
				/* build config obj with NULL-SSID for the rest bss of the radio */
				while ((bss_num_per_radio < radio->max_bss_num) &&
						bss_num_in_tlv < BSSNUM_PER_BSS_RESPONSE_TLV) {

					debug(DEBUG_ERROR,
						"build NULL-SSID config obj for bss index(%d) band(%d)\n",
						bss_num_per_radio, radio_idx);
					/* build config obj attr for this bss with NULL-SSID */
					temp_buf = build_bss_config_obj_attr(
						temp_buf, ctx, peer, radio->identifier, 0, 0);

					/* increasement bss_num of current radio */
					bss_num_per_radio++;
					bss_num_in_tlv++;
				}
			}

			if (bss_num_per_radio >= radio->max_bss_num) {
				/* bss num greater than or eqaul to
				**	max bss num of this radio
				*/
				radio_idx++;
				bss_num_per_radio = 0;
				bss_conf_idx = 0;
				debug(DEBUG_ERROR, "goto next radio[%d]\n", radio_idx);
			} else {
				debug(DEBUG_ERROR, "stay on current radio[%d]\n", radio_idx);
			}

		}

		/* update each tlv len here, avoid tlv len of last tlv is missing */
		*((unsigned short *)p_tlv_len) = host_to_be16(temp_buf - tlv_base_ptr - 3);
	}
	/* update each tlv len */
	total_length = (unsigned short)(temp_buf - pkt);
	return total_length;
}


unsigned short bss_configuration_response_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx, unsigned char *al_mac)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;
	struct bss_connector_item *bc_item = NULL, *bc_item_nxt = NULL;
	unsigned short found = 0;
	unsigned char bss_num = 0;
	unsigned int buf_size = 0;

	dl_list_for_each_safe(bc_item, bc_item_nxt,
		&ctx->r3_dpp.bss_connector_list, struct bss_connector_item, member) {
		if (os_memcmp(bc_item->almac, al_mac, ETH_ALEN) == 0) {
			found = 1;
			debug(DEBUG_ERROR, "found bss connector by almac "MACSTR"\n", MAC2STR(al_mac));
			break;
		}
	}

	if (!found) {
		debug(DEBUG_ERROR, "not found bss connector by almac "MACSTR"\n", MAC2STR(al_mac));
		debug(DEBUG_ERROR, "wait for another turn\n");
		return 0;
	}
	bss_num = precheck_obj_num(ctx, al_mac);
	if (bss_num > BSSNUM_PER_BSS_RESPONSE_TLV) {
		/* 1 more kbytes for other NULL-SSID obj
		** and 8021Q, ts
		*/
		buf_size = (bss_num+1) * 1024 + CMDU_HLEN + ETH_HLEN;

		if ((buf_size > MAX_PKT_LEN) &&
			((ctx->encr_buf.buf_len < PKT_LEN_100K) ||
			(ctx->tlv_buf.buf_len < PKT_LEN_100K) ||
			(ctx->trx_buf.buf_len < PKT_LEN_100K))) {
			debug(DEBUG_ERROR, "realloc 100k buf for encr buf, tlv buf, ctx rx_buf\n");
			/* realloc 100k buf for encr buf, tlv buf, ctx rx_buf */
			buf_huge(&ctx->encr_buf, buf_size, 0);
			buf_huge(&ctx->tlv_buf, buf_size, 0);
			buf_huge(&ctx->trx_buf, buf_size, 0);
			sec_update_buf(ctx->encr_buf.buf, ctx->encr_buf.buf_len);

			eloop_cancel_timeout(tm_reset_buf_size, (void *)ctx, NULL);
			eloop_register_timeout(60, 0, tm_reset_buf_size, (void *)ctx, NULL);

			if ((ctx->encr_buf.buf_len < PKT_LEN_100K) || (ctx->tlv_buf.buf_len < PKT_LEN_100K) ||
				(ctx->trx_buf.buf_len < PKT_LEN_100K)) {
				debug(DEBUG_ERROR, "one of encr buf, tlv buf, ctx trx_buf less than 100k, skip\n");
				return 0;
			}
		}
	}
	tlv_temp_buf = ctx->tlv_buf.buf;
	msg_hdr = (cmdu_message_header *)(ctx->trx_buf.buf + ETH_HLEN);

	length = append_bss_config_response_tlv(tlv_temp_buf, ctx, al_mac);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/* Zero or one Default 802.1Q Settings TLV */
	length = append_default_8021Q_tlv(ctx, tlv_temp_buf, al_mac,
			ctx->bss_config_num, ctx->bss_config);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/* Zero or one Traffic Separation Policy TLV */
	length = append_traffic_separation_tlv(ctx, tlv_temp_buf, al_mac,
			ctx->bss_config_num, ctx->bss_config);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(BSS_CONFIGURATION_RESPONSE_MESSAGE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}

/* spec is not ready, just keep as a reminder here */
int append_bss_config_report_tlv(unsigned char *pkt, struct p1905_managerd_ctx* ctx)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;
    int i = 0, j = 0;

	temp_buf = pkt;
	(*temp_buf) = BSS_CONFIG_REPORT_TYPE;
	temp_buf += 3;

	/*
	Number of radios reported.
	*/
	*temp_buf++ = ctx->radio_number;
    debug(DEBUG_TRACE, "radio num:%d\n", ctx->radio_number);

    for (i = 0; i < ctx->radio_number; i++) {
        /* Radio Unique Identifier of a radio */
        os_memcpy(temp_buf, ctx->rinfo[i].identifier, ETH_ALEN);
        debug(DEBUG_TRACE, "radio identifier "MACSTR"\n", MAC2STR(ctx->rinfo[i].identifier));
        temp_buf += ETH_ALEN;

        /* Number of BSS */
        *temp_buf++ = ctx->rinfo[i].bss_number;
        debug(DEBUG_TRACE, "radio %d bss num %d\n", i, ctx->rinfo[i].bss_number);

        for (j = 0; j < ctx->rinfo[i].bss_number; j++) {
            /* MAC Address of Local Interface (equal to BSSID) */
            os_memcpy(temp_buf, ctx->rinfo[i].bss[j].mac, ETH_ALEN);
            debug(DEBUG_TRACE, "radio %d bss %d mac "MACSTR"\n", i, j, MAC2STR(ctx->rinfo[i].bss[j].mac));
            temp_buf+= ETH_ALEN;

            *temp_buf = 0;

            /* bit7: Backhaul BSS */
            if (ctx->rinfo[i].bss[j].config.map_vendor_extension & BIT_BH_BSS) {
                *temp_buf |= 0x80;
                debug(DEBUG_TRACE, "FH in use\n");
            }

            /* bit6: Fronthaul BSS */
            if (ctx->rinfo[i].bss[j].config.map_vendor_extension & BIT_FH_BSS) {
                *temp_buf |= 0x40;
                debug(DEBUG_TRACE, "BH in use\n");
            }

            /* bit5: R1 disallowed status */
            if (ctx->rinfo[i].bss[j].config.map_vendor_extension & BIT_PROFILE1_DISALLOW) {
                *temp_buf |= 0x20;
                debug(DEBUG_TRACE, "R1 disallowed status: disallowed\n");
            }
            else {
                debug(DEBUG_TRACE, "R1 disallowed status: allowed\n");
            }

            /* bit4: R2 disallowed status */
            if (ctx->rinfo[i].bss[j].config.map_vendor_extension & BIT_PROFILE2_DISALLOW) {
                *temp_buf |= 0x10;
                debug(DEBUG_TRACE, "R2 disallowed status: allowed\n");
            }
            else {
                debug(DEBUG_TRACE, "R2 disallowed status: allowed\n");
            }

            /* bit3: Multiple BSSID Set */
            debug(DEBUG_TRACE, "Multiple BSSID Set: Non-transmitted\n");

            /* bit2: Transmitted BSSID */
            debug(DEBUG_TRACE, "Transmitted BSSID: Non-transmitted\n");

            /* bit1 bit0 reserved */
            temp_buf++;

            /* one more byte reseved */
            temp_buf++;

            /* SSID length: 1 byte */
            *temp_buf++ = 33;
            debug(DEBUG_TRACE, "ssid len:%d\n", 33);

            /* SSID */
            os_memcpy(temp_buf, ctx->rinfo[i].bss[j].config.Ssid, 33);
            debug(DEBUG_TRACE, "ssid:%s\n", ctx->rinfo[i].bss[j].config.Ssid);
            temp_buf += 33;
        }
    }

	total_length = (unsigned short)(temp_buf - pkt);

    //hex_dump_all("bss config report", pkt, total_length);

	*(unsigned short *)(pkt + 1) = host_to_be16(total_length - 3);

	return total_length;
}

unsigned short bss_configuration_result_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx, unsigned char *al_mac)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

    debug(DEBUG_ERROR, "append bss config report\n");
	/* one BSS Configuration report TLV */
	length = append_bss_config_report_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(BSS_CONFIGURATION_RESULT_MESSAGE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}



/* jie: rm bh_staxxx */

int append_dpp_chirp_value_tlv
(
	unsigned char *pkt,
	struct chirp_tlv_info *chirp
)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;

	temp_buf = pkt;
	(*temp_buf) = DPP_CHIRP_VALUE_TYPE;
	temp_buf += 3;

    /*
    1. one byte
       bit 7, Enrollee MAC Address Present field,
        This field is only set to 1 for reconfiguration purposes
       Bit 6: Hash Validity
        1: establish DPP authentication state pertaining to the hash value in this TLV
        0: purge any DPP authentication state pertaining to the hash value in this TLV
    2. Destination STA MAC Address
    3. hash length 1 octet
    4. hash value k octets
    */
    *temp_buf = 0;
    if (chirp->enrollee_mac_address_present)
        *temp_buf |= 0x80;

    /* hash type and hash validity bitmapt */
    if (chirp->hash_validity == 1)
        *temp_buf |= 0x40;

    temp_buf++;

    if (chirp->enrollee_mac_address_present) {
        os_memcpy(temp_buf, chirp->enrollee_mac, ETH_ALEN);
        temp_buf += ETH_ALEN;
    }

    /* hash len: 1 byte */
    *(temp_buf++) = chirp->hash_len;

    /* hash */
    os_memcpy(temp_buf, chirp->hash_payload, chirp->hash_len);

    temp_buf += chirp->hash_len;

	total_length = (unsigned short)(temp_buf - pkt);

	*(unsigned short *)(pkt + 1) = host_to_be16(total_length - 3);

	return total_length;
}

int append_1905_layer_security_tlv(unsigned char *pkt)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;

	temp_buf = pkt;
	(*temp_buf) = _1905_LAYER_SECURITY_CAPABILITY_TYPE;
	temp_buf += 1;

	/* len : 3 bytes */
	*((unsigned short *)temp_buf) = host_to_be16(3);
	temp_buf += 2;

	/* Onboarding protocols supported: 1905 Device Provisioning Protocol */
	(*temp_buf) = 0;
	temp_buf += 1;

	/* Message integrity algorithms supported: HMAC-SHA256 */
	(*temp_buf) = 0;
	temp_buf += 1;

	/* Message encryption algorithms supported: AES-SIV */
	(*temp_buf) = 0;
	temp_buf += 1;

	total_length = (unsigned short)(temp_buf - pkt);

	return total_length;
}

int append_1905_encap_eapol_tlv(unsigned char *pkt, struct p1905_managerd_ctx* ctx)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;

	temp_buf = pkt;
	(*temp_buf++) = _1905_ENCAP_EAPOL_MESSAGE_TYPE;

	*((unsigned short *)temp_buf) = host_to_be16(ctx->send_tlv_len);
	temp_buf += 2;

	memcpy(temp_buf, ctx->send_tlv, ctx->send_tlv_len);
	temp_buf += ctx->send_tlv_len;

	ctx->send_tlv_len = 0;

	total_length = (unsigned short)(temp_buf - pkt);

	return total_length;
}

int append_bootstrap_uri_notification_tlv(unsigned char *pkt, struct p1905_managerd_ctx* ctx)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;

	temp_buf = pkt;
	memcpy(temp_buf, ctx->send_tlv, ctx->send_tlv_len);
	temp_buf += ctx->send_tlv_len;

	ctx->send_tlv_len = 0;

	total_length = (unsigned short)(temp_buf - pkt);

	*(unsigned short *)(pkt + 1) = host_to_be16(total_length - 3);

	return total_length;
}


int parse_proxied_encap_dpp_type_tlv(unsigned char *buf, unsigned short len,
	unsigned char *frame_type, unsigned short *frame_offset, unsigned short *frame_len)
{
	unsigned short left = len;
	unsigned char *temp_buf;
	unsigned char *p_bitmap = NULL;
	unsigned char sta_mac[ETH_ALEN];

	temp_buf = buf;

	if (left < 1) {
		debug(DEBUG_ERROR, "[%d] Error left_tlv_length %d less than %d\n",
			  __LINE__, left, 1);
		return -1;
	}

	p_bitmap = temp_buf++;
	left -= 1;


	/* Enrollee MAC address.
	** (This field is present if the Final destination field is set to 0)
	*/
	if (*p_bitmap & 0x80) {
		if (left < ETH_ALEN) {
			debug(DEBUG_ERROR, "[%d] Error left_tlv_length %d less than %d\n",
				  __LINE__, left, ETH_ALEN);
			return -1;
		}
		memcpy(sta_mac, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;
		left -= ETH_ALEN;
	}

	if (left < 3) {
		debug(DEBUG_ERROR, "[%d] Error left_tlv_length %d less than %d\n",
			  __LINE__, left, 3);
		return -1;
	}
	*frame_type = *temp_buf++;
	*frame_len = *((unsigned short *)temp_buf);
	*frame_len = be_to_host16(*frame_len);
	temp_buf += 2;
	left -= 3;

	if (left < *frame_len) {
		debug(DEBUG_ERROR, "[%d] Error left_tlv_length %d less than %d\n",
			  __LINE__, left, *frame_len);
		return -1;
	}
	*frame_offset = temp_buf - buf;

	/*DPP Frame
	**
		0: DPP public action frame
		1: GAS frame
	*/
	if (*p_bitmap & 0x20 || *frame_type == 0xFF) {
		debug(DEBUG_OFF, "[dpp in 1905]GAS frame %s\n", GasFrame_actionNum2str(*temp_buf));
	} else {
		debug(DEBUG_OFF, "[dpp in 1905]Action Frame %s\n", actionFrame_typeNum2str(*(temp_buf+6)));
	}

	return 0;
}

int parse_dpp_chirp_value_tlv(
	unsigned char *buf, unsigned short len, struct chirp_tlv_info *chirp)
{
	unsigned short left = len;
	unsigned char *temp_buf;
	unsigned char bitmap = 0;

	temp_buf = buf;
	if (left < 1) {
		debug(DEBUG_ERROR, "[%d] Error left_tlv_length %d less than %d\n",
			  __LINE__, left, 1);
		return -1;
	}

	bitmap = *temp_buf++;
	left -= 1;

	/* Enrollee MAC Address Present field */
	if (bitmap & 0x80) {
		if (left < ETH_ALEN) {
			debug(DEBUG_ERROR, "[%d] Error left_tlv_length %d less than %d\n",
				  __LINE__, left, ETH_ALEN);
			return -1;
		}
		/* reconfiguration hash */
		chirp->enrollee_mac_address_present = 1;

		os_memcpy(chirp->enrollee_mac, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;
		left -= ETH_ALEN;
		debug(DEBUG_ERROR, "[dpp in 1905]enrollee mac present 1 \n");
		debug(DEBUG_ERROR, "[dpp in 1905]enrollee mac "MACSTR" \n", MAC2STR(chirp->enrollee_mac));
	} else {
		/* bootstrapping hash */
		chirp->enrollee_mac_address_present = 0;
		debug(DEBUG_ERROR, "[dpp in 1905]enrollee mac present 0\n");
	}

	/* hash len + hash value */
	if (left < 1+32) {
		debug(DEBUG_ERROR, "[%d] Error left_tlv_length %d less than %d\n",
			  __LINE__, left, 1+32);
		return -1;
	}
	/* Hash Validity Bit */
	if (bitmap & 0x40) {
		/* establish DPP authentication state pertaining to the hash value in this TLV */
		chirp->hash_validity = 1;
	} else {
		/* purge any DPP authentication state pertaining to the hash value in this TLV */
		chirp->hash_validity = 0;
	}
	debug(DEBUG_ERROR, "[dpp in 1905]hash_validity %d \n", chirp->hash_validity);

	chirp->hash_len = (unsigned short)(*(temp_buf++));

	os_memcpy(chirp->hash_payload, temp_buf, chirp->hash_len);

	return 0;
}





int parse_proxied_encap_dpp_message(
	struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len,
	unsigned char *al_mac,
	unsigned char *frame_type,
	unsigned short *frame_offset,
	unsigned short *frame_len)
{
	unsigned char *type = NULL;
	unsigned char *value = NULL;
	unsigned short len = 0;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned int integrity = 0;
	struct r3_member *peer = NULL;
	struct chirp_tlv_info chirp;

	os_memset(&chirp, 0, sizeof(struct chirp_tlv_info));

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = 0x%02x, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}
		if (*type == PROXIED_ENCAP_DPP_MESSAGE_TYPE) {
			/* only need to parse peer discovery,
			** other pkt pass to mapd directly
			*/
			ret = parse_proxied_encap_dpp_type_tlv(value, len,
				frame_type, frame_offset, frame_len);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error dpp message tlv\n");
				return -1;
			}
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == DPP_CHIRP_VALUE_TYPE) {
			ret = parse_dpp_chirp_value_tlv(value, len, &chirp);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error dpp message tlv\n");
				return -1;
			}
			integrity |= (1 << 1);
			if (*frame_type != DPP_DISCOVERY_REQUEST &&
				*frame_type != DPP_DISCOVERY_RESPONSE)
				store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
	/*check integrity*/
	if (integrity < 0x1) {
		debug(DEBUG_ERROR, "error when check dpp message tlvs\n");
		return -1;
	}

	if (integrity & BIT(1)) {
		peer = get_r3_member(al_mac);
		if (!peer)
			return -1;

		os_memcpy(&peer->chirp, &chirp, sizeof(struct chirp_tlv_info));
		add_chirp_hash_list(ctx, &chirp, al_mac);
	}
	return 0;
}


int parse_1905_encap_eapol_message(
	struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len,
	unsigned short *eapol_offset, unsigned short *eapol_len)
{
	unsigned char *type = NULL;
	unsigned char *value = NULL;
	unsigned short len = 0;
	unsigned int tlv_len = 0;
	unsigned int integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = 0x%02x, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}
		if (*type == _1905_ENCAP_EAPOL_MESSAGE_TYPE) {
			*eapol_offset = (unsigned short)(value - type);
			*eapol_len = len;
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
	/*check integrity*/
	if (integrity != 1) {
		debug(DEBUG_ERROR, "error when check eapol message tlvs\n");
		return -1;
	}
	return 0;
}


int parse_dpp_bootstraping_uri_notification_message(
	struct p1905_managerd_ctx *ctx,
	unsigned char *buf,
	unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned int tlv_len = 0;
	unsigned int integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = 0x%02x, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}
		if (*type == DPP_BOOTSTRAP_URI_NOTIFY_TYPE) {
			integrity |= (1 << 0);

			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
	/*check integrity*/
	if (integrity != 1) {
		debug(DEBUG_ERROR, "error when check dpp bootstraping uri notification tlvs\n");
		return -1;
	}
	return 0;
}


int parse_dpp_cce_indication_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned int tlv_len = 0;
	unsigned int integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = 0x%02x, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}
		if (*type == DPP_CCE_INDICATION_TYPE) {
			integrity |= (1 << 0);

			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
		/*check integrity*/
	if (integrity != 1) {
		debug(DEBUG_ERROR, "error when check dpp message tlvs\n");
		return -1;
	}
	return 0;
}

int parse_direct_encap_dpp_message_tlv(
	struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned short len,
	unsigned char *frame_type, unsigned short *frame_offset, unsigned short *frame_len,
	unsigned char *al_mac)
{
	unsigned short left = len;
	unsigned char *temp_buf;
	unsigned char action = 0;

	temp_buf = buf;

	if (left < 1) {
		debug(DEBUG_ERROR, "[%d] Error left_tlv_length %d less than %d\n",
			  __LINE__, left, 1);
		return -1;
	}
	action = *temp_buf;
	left -= 1;

	if (action == 0x09) {
		/* publica action frame: include dpp auth, peer discover */
		/* action frame start from Action field not Category
		** Action field:    1 octet,    value 0x09
		** OUI:             3 octets,   value 50 6F 9A
		** OUI type:        1 octet,    value 0x1A
		** Crypto Suite:    1 octet,    value *
		** DPP frame type:  1 octet,    value *
		*/
		if (left < 6) {
			debug(DEBUG_ERROR, "[%d] Error left_tlv_length %d less than %d\n",
				  __LINE__, left, 6);
			return -1;
		}
		left -= 6;
		/* 7:action hdr + 3: tlv hdr */
		*frame_offset = 7 + 3;
		*frame_type = *(temp_buf + 6);
		*frame_len = len - 7;

		if (left < *frame_len) {
			debug(DEBUG_ERROR, "[%d] Error left_tlv_length %d less than %d\n",
				  __LINE__, left, *frame_len);
			return -1;
		}

		debug(DEBUG_ERROR, "[dpp in 1905]public action frame %s\n", actionFrame_typeNum2str(*frame_type));
	} else {
		/* GAS frame: DPP configuration */
        /* 1905 don't handle this */
		*frame_type = 0xFF;

		/* pass whole raw data to mapd */
		debug(DEBUG_ERROR, "[dpp in 1905]GAS frame: %s\n", GasFrame_actionNum2str(*temp_buf));
	}
	return 0;
}



int parse_direct_encap_dpp_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned int left_tlv_len,
	unsigned char *frame_type, unsigned short *frame_offset, unsigned short *frame_len,
	unsigned char *al_mac)
{
	unsigned char *type = NULL;
	unsigned char *value = NULL;
	unsigned short len = 0;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned int integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = 0x%02x, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}
		if (*type == DIRECT_ENCAP_DPP_MESSAGE_TYPE) {
			ret = parse_direct_encap_dpp_message_tlv(ctx, value, len, frame_type,
				frame_offset, frame_len, al_mac);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error direct encap dpp message tlv\n");
				return -1;
			}
			integrity |= (1 << 0);

			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
	/*check integrity*/
	if (integrity != 1) {
		debug(DEBUG_ERROR, "error when check dpp message tlvs\n");
		return -1;
	}

	if (*frame_type == DPP_PA_AUTHENTICATION_REQ) {
		if (ctx->r3_oboard_ctx.bh_type == MAP_BH_ETH) {
			debug(DEBUG_ERROR, "[dpp in 1905]ETH BH,cancel timer of dpp auth req\n");
			eloop_cancel_timeout(r3_wait_for_dpp_auth_req, (void *)ctx, NULL);
		}
	} else if (*frame_type == DPP_PA_PEER_DISCOVERY_REQ ||
			*frame_type == DPP_PA_PEER_DISCOVERY_RESP) {
		map_dpp_handle_public_action_frame(ctx, *frame_type,
			(unsigned char *)(buf + *frame_offset), (size_t)*frame_len, al_mac);
	}
	return 0;
}

int parse_1905_rekey_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned int tlv_len = 0;
	unsigned int integrity = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = 0x%02x, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}
		if (*type != END_OF_TLV_TYPE)
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		else
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
	/*check integrity*/
	if (integrity != 0) {
		debug(DEBUG_ERROR, "error when check 1905 rekey request message tlvs\n");
		return -1;
	}
	return 0;
}

int parse_1905_decryption_failure_message(struct p1905_managerd_ctx *ctx,
			unsigned char *buf, unsigned int left_tlv_len, unsigned char *al_mac)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned int integrity = 0;
	unsigned char peer_al_mac[ETH_ALEN] = { 0 };
	unsigned char zero_mac[ETH_ALEN] = { 0 };

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "[%d] type = %d, tlv len %d > left_tlv_len %d\n",
				__LINE__, *type, tlv_len, left_tlv_len);
			return -1;
		}

		if (*type == AL_MAC_ADDR_TLV_TYPE) {
			/* only need to parse peer discovery,
			** other pkt pass to mapd directly
			*/
			ret = parse_al_mac_addr_type_tlv(al_mac, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error al mac tlv\n");
				return -1;
			}
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
	/*check integrity*/
	if (integrity < 1) {
		debug(DEBUG_ERROR, "error when check dpp message tlvs\n");
		return -1;
	}

	if ((integrity&BIT(0)) && (os_memcmp(peer_al_mac, zero_mac, ETH_ALEN)))
		os_memcpy(al_mac, peer_al_mac, ETH_ALEN);

	return 0;
}

int parse_chirp_notification_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len, unsigned char *al_mac)
{
	unsigned char *type = NULL;
	unsigned char *value = NULL;
	unsigned short len = 0;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned int integrity = 0;
	struct r3_member *peer = NULL;
	struct chirp_tlv_info chirp;

	os_memset(&chirp, 0, sizeof(struct chirp_tlv_info));

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = 0x%02x, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}
		if (*type == DPP_CHIRP_VALUE_TYPE) {

			ret = parse_dpp_chirp_value_tlv(value, len, &chirp);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error al mac tlv\n");
				return -1;
			}
			integrity |= (1 << 0);
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
	/*check integrity*/
	if (integrity != 1) {
		debug(DEBUG_ERROR, "error when check chirp notification tlvs\n");
		return -1;
	}

	if (integrity & BIT(0)) {
		peer = get_r3_member(al_mac);
		if (!peer)
			return -1;
		os_memcpy(&peer->chirp, &chirp, sizeof(struct chirp_tlv_info));
		add_chirp_hash_list(ctx, &chirp, al_mac);
	}
	return 0;
}



unsigned short chirp_notification_message(
	unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(CHIRP_NOTIFICATION_MESSAGE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;

}



int parse_dpp_bootstrapping_uri_query_message(struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned char *al_mac)
{
	int length =0;
	unsigned char *temp_buf;
	unsigned int integrity = 0;
	temp_buf = buf;
	reset_stored_tlv(ctx);

	while(1)
	{
		if(*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
		/*check integrity*/
	if(integrity != 0) {
		debug(DEBUG_ERROR, "error when check dpp uri query message\n");
		return -1;
	}
	return 0;
}

int parse_akm_suit_capability_tlv(
	unsigned char *buf,
	unsigned short len,
	struct r3_member *peer)
{
	int left = len;
	unsigned char *temp_buf;
	unsigned char i =0;
	struct akm_suite_cap *akm_cap = NULL;
	struct akm_suit *akm = NULL;

	if (!peer)
		return -1;

	temp_buf = buf;

	if (peer->fh_akm.akm_suite_info) {
		os_free(peer->fh_akm.akm_suite_info);
		peer->fh_akm.akm_suite_info = NULL;
	}
	if (peer->bh_akm.akm_suite_info) {
		os_free(peer->bh_akm.akm_suite_info);
		peer->bh_akm.akm_suite_info = NULL;
	}

	/* Num BH BSS AKM Suite Selectors	*/
	akm_cap = &peer->bh_akm;
	if (left < 1) {
		debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
			__LINE__, left, 1);
		goto fail;
	}
	akm_cap->akm_suite_cnt = *temp_buf++;
	left--;
	if (akm_cap->akm_suite_cnt > AKM_CNT_MAX) {
		debug(DEBUG_ERROR, "BH AKM Suite cnt(%d) too big\n", akm_cap->akm_suite_cnt);
		goto fail;
	}
	akm_cap->akm_suite_info = os_zalloc(akm_cap->akm_suite_cnt * sizeof(struct akm_suit));
	if (!akm_cap->akm_suite_info) {
		debug(DEBUG_ERROR, "malloc buf for akm BH fail\n");
		goto fail;
	}
	akm = akm_cap->akm_suite_info;
	debug(DEBUG_ERROR, "Num BH AKM Suite %d\n", akm_cap->akm_suite_cnt);
	for (i = 0; i < akm_cap->akm_suite_cnt; i ++) {
		if (left < sizeof(struct akm_suit)) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
				__LINE__, left, (int)sizeof(struct akm_suit));
			goto fail;
		}
		os_memcpy(akm->oui, temp_buf, 3);
		temp_buf += 3;
		akm->akm_suit_type = *temp_buf++;
		debug(DEBUG_ERROR, "suite type %d, oui %02x%02x%02x\n",
			akm->akm_suit_type, akm->oui[0], akm->oui[1], akm->oui[2]);

		akm ++;
		left -= sizeof(struct akm_suit);
	}

	/* Num FH BSS AKM Suite Selectors	*/
	akm_cap = &peer->fh_akm;
	if (left < 1) {
		debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
			__LINE__, left, 1);
		goto fail;
	}
	akm_cap->akm_suite_cnt = *temp_buf++;
	left--;
	if (akm_cap->akm_suite_cnt > AKM_CNT_MAX) {
		debug(DEBUG_ERROR, "FH AKM Suite cnt(%d) too big\n", akm_cap->akm_suite_cnt);
		goto fail;
	}
	akm_cap->akm_suite_info = os_zalloc(akm_cap->akm_suite_cnt * sizeof(struct akm_suit));
	if (!akm_cap->akm_suite_info) {
		debug(DEBUG_ERROR, "malloc buf for akm FH fail\n");
		goto fail;
	}
	akm = akm_cap->akm_suite_info;
	debug(DEBUG_ERROR, "Num FH AKM Suite %d\n", akm_cap->akm_suite_cnt);
	for (i = 0; i < akm_cap->akm_suite_cnt; i ++) {
		if (left < sizeof(struct akm_suit)) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
				__LINE__, left, (int)sizeof(struct akm_suit));
			goto fail;
		}
		os_memcpy(akm->oui, temp_buf, 3);
		temp_buf += 3;
		akm->akm_suit_type = *temp_buf++;
		debug(DEBUG_ERROR, "suite type %d, oui %02x%02x%02x\n",
			akm->akm_suit_type, akm->oui[0], akm->oui[1], akm->oui[2]);

		akm ++;
		left -= sizeof(struct akm_suit);
	}

	return 0;
fail:
	if (peer->fh_akm.akm_suite_info) {
		os_free(peer->fh_akm.akm_suite_info);
		peer->fh_akm.akm_suite_info = NULL;
	}
	if (peer->bh_akm.akm_suite_info) {
		os_free(peer->bh_akm.akm_suite_info);
		peer->bh_akm.akm_suite_info = NULL;
	}
	return -1;
}


int parse_bh_sta_radio_cap_tlv(unsigned char *buf, unsigned short len)
{
	int left = len;
	unsigned char *temp_buf;
	unsigned char identifier[ETH_ALEN];
	unsigned char bh_mac[ETH_ALEN];
	temp_buf = buf;

	if (left < ETH_ALEN + 1) {
		debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
			__LINE__, left, ETH_ALEN + 1);
		return -1;
	}

	/* identifier */
	os_memcpy(identifier, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;
	left -= ETH_ALEN;

	if ((*(temp_buf++)) & 0x80) {
		left -= 1;
		if (left < ETH_ALEN) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
				__LINE__, left, ETH_ALEN);
			return -1;
		}
		os_memcpy(bh_mac, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;
	}

	return 0;
}


int parse_bss_configuration_request_tlv(struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned short len)
{
	unsigned char *temp_buf;
	unsigned short obj_len = 0;
	struct json_token *root, *token;
	const unsigned char *conf_rsp_obj = NULL;
	temp_buf = buf;

	if (dpp_check_attrs(temp_buf, len) < 0) {
		debug(DEBUG_ERROR, "Invalid attribute in bss configuration response\n");
		goto fail;
	}

	conf_rsp_obj = dpp_get_attr(temp_buf, len, DPP_ATTR_CONFIG_ATTR_OBJ, &obj_len);
	if (!conf_rsp_obj) {
		debug(DEBUG_ERROR,
			      "Missing or invalid required configuration object\n");
		goto fail;
	}

	/* parse dpp configuration request object */
	root = json_parse((const char *) conf_rsp_obj, obj_len);
	if (!root)
		return -1;
	if (root->type != JSON_OBJECT) {
		debug(DEBUG_ERROR, "JSON root is not an object\n");
		goto fail;
	}

	token = json_get_member(root, "netRole");
	if (!token || token->type != JSON_STRING) {
		debug(DEBUG_ERROR, "No netRole string value found\n");
		goto fail;
	}
    debug(DEBUG_ERROR, "netRole %s\n", token->string);

	token = json_get_member(root, "wi-fi_tech");
	if (!token || token->type != JSON_STRING) {
		debug(DEBUG_ERROR, "No wi-fi_tech string value found\n");
		goto fail;
	}
    debug(DEBUG_ERROR, "wi-fi_tech %s\n", token->string);

	return 0;
fail:
	return -1;
}

int parse_1905_bss_configuration_request_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len, unsigned char *al_mac, OUT struct agent_radio_info *r,
	OUT unsigned char *radio_cnt)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned char service = 0;
	unsigned int integrity = 0;
	unsigned char map_version = 0;
	struct r3_member *peer = NULL;

	type = buf;
	reset_stored_tlv(ctx);
	peer = get_r3_member(al_mac);

	*radio_cnt = 0;

	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = 0x%02x, left_tlv_length %d less than tlv_len %d\n",
				__LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}

		if (*type == MULTI_AP_VERSION_TYPE) {
			ret = parse_map_version_tlv(type, al_mac, &map_version);

			if (ret < 0) {
				debug(DEBUG_ERROR, "error al mac tlv\n");
				return -1;
			}

			integrity |= (1 << 0);
		} else if (*type == SUPPORTED_SERVICE_TLV_TYPE) {
			ret = parse_supported_service_tlv(&service, len, value);

			if (ret < 0) {
				debug(DEBUG_ERROR, "error supported service tlv\n");
				return -1;
			}
			integrity |= (1 << 1);
		} else if (*type == AKM_SUITE_CAPABILITY_TYPE) {
			ret = parse_akm_suit_capability_tlv(value, len, peer);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error akm suite cap tlv\n");
				return -1;
			}
			integrity |= (1 << 2);
		} else if (*type == AP_RADIO_BASIC_CAPABILITY_TYPE) {
			if (!peer) {
				debug(DEBUG_ERROR, "peer not found\n");
				return -1;
			}
			os_memset(r, 0, sizeof(struct agent_radio_info));
			*radio_cnt += 1;
			if (*radio_cnt > MAX_RADIO_NUM) {
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX"error!!!Exceeds MAX_RADIO_NUM"
					" in ap radio basic cap tlv\n");
				return -1;
			}
			/*length = parse_ap_radio_basic_cap_tlv(ctx, temp_buf, peer->radio_basic_r3.identifier[radio_index],
				&peer->radio_basic_r3.bss_num[radio_index], &peer->radio_basic_r3.band_cap_r3[radio_index]);*/
			ret = parse_ap_radio_basic_cap_tlv(ctx, r++, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error ap radio basic cap tlv\n");
				return -1;
			}
			integrity |= (1 << 3);
		}
		/* One Profile-2 AP Capability TLV */
		else if (*type == R2_AP_CAPABILITY_TYPE) {
			integrity |= (1 << 4);
			ret = get_tlv_len_by_tlvtype(type);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* One or more AP Radio Advanced Capabilities TLV  */
		else if (*type == AP_RADIO_ADVANCED_CAPABILITIES_TYPE) {
			integrity |= (1 << 5);
			ret = get_tlv_len_by_tlvtype(type);
			store_revd_tlvs(ctx, type, tlv_len);
		}
		/* One or more BSS Configuration Request TLV */
		else if (*type == BSS_CONFIG_REQUEST_TYPE) {
			ret = parse_bss_configuration_request_tlv(ctx, value, len);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error bss configuration request tlv\n");
				return -1;
			}
			integrity |= (1 << 6);
		}
		/* Zero or more Backhaul STA Radio Capabilities TLV */
		else if (*type == BACKHAUL_STA_RADIA_CAPABILITY_TYPE) {
			debug(DEBUG_TRACE, " parse BACKHAUL_STA_RADIA_CAPABILITY_TYPE\n");
			ret = parse_bh_sta_radio_cap_tlv(value, len);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error bh sta radio cap tlv\n");
				return -1;
			}
			integrity |= (1 << 7);
		} else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
	/*check integrity*/
	if(integrity < 0x3F) {
		debug(DEBUG_ERROR, "error when check bss configuration request msg\n");
		return -1;
	}

	if ((integrity & BIT(0)) && peer)
		peer->profile = map_version;

	if (integrity & BIT(2))
		map_notify_akm_suit_cap(ctx, al_mac, type, tlv_len);

	return 0;
}

int parse_1905_layer_security_capability_tlv(struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned char *onboardingProto, unsigned char *msgIntAlg, unsigned char *msgEncAlg)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
	temp_buf = buf;

	if((*temp_buf) == _1905_LAYER_SECURITY_CAPABILITY_TYPE) {
		temp_buf++;
	} else {
		return -1;
	}

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);
	temp_buf += 2;

	*onboardingProto = *temp_buf++;

	*msgIntAlg = *temp_buf++;

	*msgEncAlg = *temp_buf++;

	return (length+3);
}




int r3_send_bss_config_req(struct p1905_managerd_ctx *ctx)
{
	int if_index = 0;

	if_index = ctx->cinfo.recv_ifid;
	insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
			e_bss_configuration_request, ++ctx->mid,
			ctx->itf[if_index].if_name, 0);
	return 0;
}

int transfer_r3_config_2_wireless_setting(struct p1905_managerd_ctx *ctx)
{
	struct r3_onboarding_info *r3_ctx = &ctx->r3_oboard_ctx;
	unsigned char i, index = 0, j = 0;
	struct r3_member *peer= NULL;

	/*step1. transfer r3 configuration data to r1/r2 autoconfiguration data*/
	i = ctx->current_autoconfig_info.radio_index;

	for (j = 0; j < r3_ctx->total_config_num; j++) {
		if (!os_memcmp(ctx->rinfo[i].identifier,
			r3_ctx->bss_config_data[j].radio_identifier, ETH_ALEN)) {
			os_memcpy(&ctx->current_autoconfig_info.config_data[index++],
				&r3_ctx->bss_config_data[j].config_data, sizeof(WSC_CONFIG));
		}
	}


	if (!index) {
		debug(DEBUG_ERROR, "tear down radio %d, identifier["MACSTR"]\n",
			i, MAC2STR(ctx->rinfo[i].identifier));
		_1905_set_radio_tear_down(ctx, ctx->rinfo[i].identifier);
	}
	else {
		ctx->current_autoconfig_info.config_number = index;

		/*step2. set wireless setting to mapd/wappd/driver*/
		set_wsc_config(ctx);
	}

	/*step3. check whether need we do this for another band*/
	if (++ctx->current_autoconfig_info.radio_index < ctx->radio_number) {
		eloop_cancel_timeout(r3_set_config, (void *)ctx, NULL);
		eloop_register_timeout(3, 0, r3_set_config, (void *)ctx, NULL);
		debug(DEBUG_ERROR, "trigger radio(%d) to do bss config\n",
			ctx->current_autoconfig_info.radio_index);
	}
	else {
		r3_set_config_state(ctx, r3_autoconfig_done);
		ctx->r3_oboard_ctx.onboarding_stage = R3_ONBOARDING_STAGE_DONE;
		debug(DEBUG_ERROR, "R3 onboarding finished, bss config done\n");
		ctx->current_autoconfig_info.radio_index = -1;

		peer = get_r3_member(r3_ctx->peer_almac);

		if (peer) {
			/* send bss configuration result message after configuration done */
			eloop_cancel_timeout(wait_send_bss_configuration_result, (void *)ctx, (void *)peer);
			eloop_register_timeout(3, 0, wait_send_bss_configuration_result, (void *)ctx, peer);
		}
	}

	return 0;
}

void r3_set_config(void *eloop_ctx, void *timeout_ctx)
{
	transfer_r3_config_2_wireless_setting((struct p1905_managerd_ctx *)eloop_ctx);
}

void r3_set_config_state(struct p1905_managerd_ctx *ctx,  unsigned char state)
{
	struct r3_onboarding_info *r3_info = &ctx->r3_oboard_ctx;

	r3_info->r3_config_state = state;
	debug(DEBUG_ERROR, "[BSS Config] state change to %d\n", state);
}

void r3_config_sm(struct p1905_managerd_ctx *ctx)
{
	struct r3_onboarding_info *r3_info = &ctx->r3_oboard_ctx;

	switch(r3_info->r3_config_state) {
		case wait_4_send_bss_autoconfig_req:
		{
			r3_send_bss_config_req(ctx);
			r3_set_config_state(ctx, wait_4_recv_bss_autoconfig_resp);
			eloop_cancel_timeout(r3_config_sm_step, (void *)ctx, NULL);
			eloop_register_timeout(5, 0, r3_config_sm_step, (void *)ctx, NULL);
		}
			break;
		case wait_4_recv_bss_autoconfig_resp:
		{
			if (r3_info->bss_req_retry_cnt < 4) {
				r3_info->bss_req_retry_cnt ++;
				debug(DEBUG_ERROR, "[BSS Config]retry %d times BSS Config procedure\n", r3_info->bss_req_retry_cnt);
				r3_set_config_state(ctx, wait_4_send_bss_autoconfig_req);
				eloop_cancel_timeout(r3_config_sm_step, (void *)ctx, NULL);
				eloop_register_timeout(5, 0, r3_config_sm_step, (void *)ctx, NULL);
			} else {
				r3_info->bss_req_retry_cnt = 0;
				ctx->r3_oboard_ctx.bss_renew = 0;
				r3_set_config_state(ctx, no_r3_autoconfig);
				ctx->r3_oboard_ctx.onboarding_stage = R3_ONBOARDING_STAGE_INVALID;
				eloop_cancel_timeout(r3_config_sm_step, (void *)ctx, NULL);
				//eloop_register_timeout(0, 0, r3_config_sm_step, (void *)ctx, NULL);
				debug(DEBUG_ERROR, "[BSS Config]retry timeout,start autoconfig search procedure\n");
				eloop_cancel_timeout(ap_controller_search_step, (void *)ctx, NULL);
				ctx->enrolle_state = wait_4_send_controller_search;
				eloop_register_timeout(RECV_CONFIG_RSP_TIME, 0, ap_controller_search_step, (void *)ctx, NULL);
			}
		}
			break;
		case wait_4_set_r3_config:
		{
			ctx->current_autoconfig_info.radio_index = 0;
			r3_set_config((void *)ctx, NULL);
		}
			break;
		default:
			break;
	}
}

void r3_config_sm_step(void *eloop_ctx, void *timeout_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;

	r3_config_sm(ctx);

	common_process(ctx, &ctx->trx_buf);
}


void wait_send_bss_configuration_result(void *eloop_ctx, void *timeout_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;
	struct r3_member *peer = timeout_ctx;

	if (!eloop_ctx || !timeout_ctx) {
		debug(DEBUG_ERROR, "NULL pointer\n");
		return;
	}

    debug(DEBUG_ERROR, "send bss config result to "MACSTR"\n",MAC2STR(peer->al_mac));

    insert_cmdu_txq(peer->al_mac, ctx->p1905_al_mac_addr,
        e_bss_configuration_result, ++ctx->mid, ctx->br_name, 0);
    return;
}

int init_akm_capability(struct p1905_managerd_ctx *ctx)
{
	ctx->bsta_akm.akm_suite_cnt = 4;
	ctx->bsta_akm.akm_suite_info = (struct akm_suit *)os_malloc(4 * sizeof(struct akm_suit));
	if (!ctx->bsta_akm.akm_suite_info)
		goto fail3;

	os_memcpy(&ctx->bsta_akm.akm_suite_info[0], OUI_WPA2_AKM_PSK, 4);
	os_memcpy(&ctx->bsta_akm.akm_suite_info[1], OUI_WPA2_AKM_PSK_SHA256, 4);
	os_memcpy(&ctx->bsta_akm.akm_suite_info[2], OUI_WPA2_AKM_SAE_SHA256, 4);
	os_memcpy(&ctx->bsta_akm.akm_suite_info[3], OUI_AKM_DPP, 4);

	ctx->bh_akm.akm_suite_cnt = 4;
	ctx->bh_akm.akm_suite_info = (struct akm_suit *)os_malloc(4 * sizeof(struct akm_suit));
	if (!ctx->bh_akm.akm_suite_info)
		goto fail2;

	os_memcpy(&ctx->bh_akm.akm_suite_info[0], OUI_WPA2_AKM_PSK, 4);
	os_memcpy(&ctx->bh_akm.akm_suite_info[1], OUI_WPA2_AKM_PSK_SHA256, 4);
	os_memcpy(&ctx->bh_akm.akm_suite_info[2], OUI_WPA2_AKM_SAE_SHA256, 4);
	os_memcpy(&ctx->bh_akm.akm_suite_info[3], OUI_AKM_DPP, 4);

	ctx->fh_akm.akm_suite_cnt = 6;
	ctx->fh_akm.akm_suite_info = (struct akm_suit *)os_malloc(6 * sizeof(struct akm_suit));
	if (!ctx->fh_akm.akm_suite_info)
		goto fail1;

	os_memcpy(&ctx->fh_akm.akm_suite_info[0], OUI_WPA2_AKM_PSK, 4);
	os_memcpy(&ctx->fh_akm.akm_suite_info[1], OUI_WPA2_AKM_FT_PSK, 4);
	os_memcpy(&ctx->fh_akm.akm_suite_info[2], OUI_WPA2_AKM_PSK_SHA256, 4);
	os_memcpy(&ctx->fh_akm.akm_suite_info[3], OUI_WPA2_AKM_SAE_SHA256, 4);
	os_memcpy(&ctx->fh_akm.akm_suite_info[4], OUI_WPA2_AKM_FT_SAE_SHA256, 4);
	os_memcpy(&ctx->fh_akm.akm_suite_info[5], OUI_AKM_DPP, 4);

	return 0;
fail1:
	os_free(ctx->bh_akm.akm_suite_info);
fail2:
	os_free(ctx->bsta_akm.akm_suite_info);
fail3:
	return -1;
}

void deinit_akm_capability(struct p1905_managerd_ctx *ctx)
{
	os_free(ctx->fh_akm.akm_suite_info);
	os_free(ctx->bh_akm.akm_suite_info);
	os_free(ctx->bsta_akm.akm_suite_info);
}

int parse_bss_configuration_response_message(struct p1905_managerd_ctx *ctx,
	unsigned char *buf, unsigned int left_tlv_len, unsigned char *al_mac)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned int integrity = 0;
	struct def_8021q_setting setting;
	struct traffics_policy tpolicy;
	struct traffic_separation_db *tpolicy_db = NULL;
	struct traffic_separation_db *tpolicy_db_nxt = NULL;

	os_memset(&setting, 0, sizeof(struct def_8021q_setting));
	os_memset(&tpolicy, 0, sizeof(struct traffics_policy));
	os_memset(&r3_ctx_tmp, 0, sizeof(struct r3_onboarding_info));

	SLIST_INIT(&tpolicy.policy_head);

	type = buf;
	reset_stored_tlv(ctx);
	ctx->r3_oboard_ctx.total_config_num = 0;
	while(1) {
		len = get_tlv_len(type);
		if ((len + 3) > left_tlv_len) {
			debug(DEBUG_ERROR, "tlv len > left_tlv_len\n");
			goto fail;
		}
		value = type + 3;
		tlv_len = len + 3;
		if (*type == BSS_CONFIG_RESPONSE_TYPE) {
			/* only need to parse peer discovery,
			** other pkt pass to mapd directly
			*/
			debug(DEBUG_TRACE, "BSS_CONFIG_RESPONSE_TLV\n");
			ret = parse_bss_configuration_response_tlv(value, len, &r3_ctx_tmp);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error bss configuration response tlv\n");
				goto fail;
			}
			integrity |= (1 << 0);
		} else if (*type == DEFAULT_8021Q_SETTING_TYPE) {
			ret = parse_default_802_1q_setting_tlv(&setting, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error default 802_1q setting tlv\n");
				goto fail;
			}
			integrity |= (1 << 1);
			setting.updated = 1;
		} else if (*type == TRAFFIC_SEPARATION_POLICY_TYPE) {
			ret = parse_traffic_separation_policy_tlv(&tpolicy, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "%s error trffic separation policy tlv\n", __func__);
				goto fail;
			}
			integrity |= (1 << 2);
			tpolicy.updated = 1;
		} else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
		/*check integrity*/
	if(integrity < 0x01) {
		debug(DEBUG_ERROR, "error when check dpp message tlvs\n");
		goto fail;
	}

	if (integrity & BIT(0)) {
		os_memset(ctx->r3_oboard_ctx.bss_config_data, 0,
			MAX_SET_BSS_INFO_NUM*sizeof(struct r3_config_data));
		ctx->r3_oboard_ctx.total_config_num = r3_ctx_tmp.total_config_num;
		os_memcpy(&ctx->r3_oboard_ctx.bss_config_data, &r3_ctx_tmp.bss_config_data,
			r3_ctx_tmp.total_config_num * sizeof(struct r3_config_data));
	}

	if (integrity & BIT(1))
		os_memcpy(&ctx->map_policy.setting, &setting, sizeof(struct def_8021q_setting));

	if (integrity & BIT(2)) {
		/*delete the previously reserved traffic policy*/
		delete_exist_traffic_policy(ctx, &ctx->map_policy.tpolicy);

		ctx->map_policy.tpolicy.SSID_num = tpolicy.SSID_num;
		ctx->map_policy.tpolicy.updated = tpolicy.updated;

		tpolicy_db = SLIST_FIRST(&(tpolicy.policy_head));
		while (tpolicy_db) {
			tpolicy_db_nxt = SLIST_NEXT(tpolicy_db, policy_entry);
			SLIST_REMOVE(&(tpolicy.policy_head), tpolicy_db,
						traffic_separation_db, policy_entry);

			SLIST_INSERT_HEAD(&ctx->map_policy.tpolicy.policy_head, tpolicy_db, policy_entry);
			tpolicy_db = tpolicy_db_nxt;
		}
	}

	return 0;

fail:
	tpolicy_db = SLIST_FIRST(&(tpolicy.policy_head));
	while (tpolicy_db) {
		tpolicy_db_nxt = SLIST_NEXT(tpolicy_db, policy_entry);
		SLIST_REMOVE(&(tpolicy.policy_head), tpolicy_db,
					traffic_separation_db, policy_entry);
		os_free(tpolicy_db);
		tpolicy_db = tpolicy_db_nxt;
	}
	return -1;
}

int append_bss_configuration_request_tlv(unsigned char *pkt, struct p1905_managerd_ctx* ctx)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;
	char json[512] = {0x00};
	struct wpabuf *msg = NULL;

	temp_buf = pkt;

	*temp_buf = BSS_CONFIG_REQUEST_TYPE;
	temp_buf += 3;

    /*
        DPP Configuration Request Object with the following content:
        "   netRole set to "mapAgent"
        "   wi-fi_tech set to "map"
    */

	(void)os_snprintf(json, sizeof(json),
		    "{"
		        "\"netRole\":\"mapAgent\","
		        "\"wi-fi_tech\":\"map\""
		    "}");

	debug(DEBUG_ERROR, "BSS configuration req obj: %s\n", json);

	msg = wpabuf_alloc(4 + os_strlen(json));

	if (!msg)
		goto end;

	/* configAttrib */
	wpabuf_put_le16(msg, DPP_ATTR_CONFIG_ATTR_OBJ);
	wpabuf_put_le16(msg, os_strlen(json));
	wpabuf_put_data(msg, json, os_strlen(json));

	os_memcpy(temp_buf, wpabuf_mhead(msg), wpabuf_len(msg));

	total_length = 3 + wpabuf_len(msg);
	*(unsigned short *)(pkt + 1) = host_to_be16(total_length - 3);
	if (msg)
		os_free(msg);
end:
	return total_length;
}

int parse_bss_configuration_response_tlv(
	unsigned char *buf, unsigned short len,
	struct r3_onboarding_info *r3_ctx)
{
	unsigned short left = len;
	unsigned char *temp_buf;
	struct json_token *root = NULL, *token, *discovery, *cred;
	struct json_token *akm_token = NULL;
	struct json_token *ruid_token = NULL;
	struct json_token *ssid_token = NULL;
	struct json_token *wifi_tech_token = NULL;
	unsigned char map_vendor_extension[MAX_SET_BSS_INFO_NUM];
	unsigned char mac[ETH_ALEN] = {0x00};
	unsigned char ruid[ETH_ALEN] = {0x00};
	unsigned char index = r3_ctx->total_config_num;
	enum dpp_akm akm;
	struct dpp_authentication auth;
	/* Cross vendor with NXP */
	const unsigned char *conf_rsp_obj = NULL;
	unsigned short obj_len = 0;
	unsigned char tear_down = 0;

	temp_buf = buf;
	os_memset(&auth, 0, sizeof(auth));
	os_memset(map_vendor_extension, 0, sizeof(map_vendor_extension));

	if (dpp_check_attrs(temp_buf, len) < 0) {
		debug(DEBUG_ERROR, "Invalid attribute in bss configuration response\n");
		goto fail;
	}

	while (temp_buf - buf < len) {
		tear_down = 0;
		conf_rsp_obj = dpp_get_attr(temp_buf, left, DPP_ATTR_CONFIG_OBJ,
					    &obj_len);
		if (!conf_rsp_obj) {
			debug(DEBUG_ERROR,
				      "Missing or invalid required configuration object\n");
			goto fail;
		}
		debug(DEBUG_TRACE, "attribute type: 0x100C\n");
		debug(DEBUG_TRACE, "attribute length: %d\n", obj_len);
		root = json_parse((const char *) conf_rsp_obj, obj_len);
		if (!root) {
			debug(DEBUG_ERROR, "Could not parse Config Response Object\n");
			goto fail;
		}

		wifi_tech_token = json_get_member(root, "wi-fi_tech");
		if (!wifi_tech_token || wifi_tech_token->type != JSON_STRING) {
			debug(DEBUG_ERROR, "No Config Attributes - wi-fi_tech\n");
			goto fail;
		}
		debug(DEBUG_TRACE, "wi-fi_tech= '%s'\n", wifi_tech_token->string);


		discovery = json_get_member(root, "discovery");
		if (!discovery || discovery->type != JSON_OBJECT) {
			debug(DEBUG_ERROR, "No discovery object in JSON\n");
			goto fail;
		}


		ssid_token = json_get_member(discovery, "ssid");
		if (!ssid_token || ssid_token->type != JSON_STRING) {
			debug(DEBUG_ERROR, "No discovery::ssid string value found\n");
			goto fail;
		}
		debug(DEBUG_ERROR, "ssid %s\n", ssid_token->string);
		if (os_strlen(ssid_token->string) > 32) {
			debug(DEBUG_ERROR, "Too long discovery::ssid string value\n");
			goto fail;
		}

		ruid_token = json_get_member(discovery, "RUID");

		if (!ruid_token || ruid_token->type != JSON_STRING) {
			debug(DEBUG_ERROR, "No Config Attributes - RUID\n");
			goto fail;
		}
		hwaddr_compact_aton(ruid_token->string, ruid);
		os_memset(&r3_ctx->bss_config_data[index], 0, sizeof(struct r3_config_data));
		if (ssid_token && os_strlen(ssid_token->string) && index) {
			/* bss with FH and BH if both ssid and ruid are the same as last obj */
			/* TODO: how about the same ssid on all interface???*/
			if ((!os_strncmp((const char *)r3_ctx->bss_config_data[index - 1].config_data.Ssid,
				(const char *)ssid_token->string, MAX_SSID_LEN - 1)) &&
				(!os_memcmp((const char *)r3_ctx->bss_config_data[index - 1].radio_identifier,
				(const char *)ruid, ETH_ALEN))) {
				index -= 1;
				debug(DEBUG_ERROR, "SSID same as last bss, reset index to %d\n", index);
			}
		}

		os_memcpy(r3_ctx->bss_config_data[index].radio_identifier, ruid, ETH_ALEN);
		debug(DEBUG_ERROR, "bss(%d) RUID "MACSTR"\n", index, MAC2STR(ruid));

		/* decide FH or BH by wi-fi_tech */
		if (!os_strcmp(wifi_tech_token->string, "map")) {
			map_vendor_extension[index] |= BIT_BH_BSS;
			debug(DEBUG_ERROR, "bss(%d) with BIT_BH_BSS\n", index);
		}
		else if (!os_strcmp(wifi_tech_token->string, "inframap")) {
			map_vendor_extension[index] |= BIT_FH_BSS;
			debug(DEBUG_ERROR, "bss(%d) with BIT_FH_BSS\n", index);
		}
		else {
			debug(DEBUG_ERROR, "invalide wifi tech info\n");
			goto fail;
		}

		token = json_get_member(discovery, "RUID");

		if (!token || token->type != JSON_STRING) {
			debug(DEBUG_ERROR, "No Config Attributes - RUID\n");
			goto fail;
		}

		if (os_strlen(ssid_token->string) == 0) {
			map_vendor_extension[index] = BIT_TEAR_DOWN;
			tear_down = 1;
			debug(DEBUG_ERROR, "index(%d) tear down this bss\n", index);
		}
		else {
			os_memset(r3_ctx->bss_config_data[index].config_data.Ssid, 0, 33);
			os_memcpy(r3_ctx->bss_config_data[index].config_data.Ssid,
				ssid_token->string, os_strlen(ssid_token->string));
		}

		token = json_get_member(discovery, "BSSID");
		if (!token || token->type != JSON_STRING) {
			debug(DEBUG_ERROR, "No discovery::BSSID string value found\n");
			goto fail;
		}
		debug(DEBUG_ERROR, "bss(%d) BSSID %s\n",index, token->string);

		if(!hwaddr_aton(token->string, mac)) {
			os_memset(r3_ctx->bss_config_data[index].bssid, 0, ETH_ALEN);
			os_memcpy(r3_ctx->bss_config_data[index].bssid, mac, ETH_ALEN);
		}
		r3_ctx->bss_config_data[index].config_data.map_vendor_extension = map_vendor_extension[index];

		if (!tear_down) {

			token = json_get_member(discovery, "hidden-ssid");

			if (!token || token->type != JSON_STRING) {
				debug(DEBUG_TRACE, "bss(%d) no hidden-ssid\n",index);
			}
			else {
				r3_ctx->bss_config_data[index].config_data.hidden_ssid = 1;
				debug(DEBUG_ERROR, "bss(%d) hidden-ssid\n",index);
			}


			cred = json_get_member(root, "cred");
			if (!cred || cred->type != JSON_OBJECT) {
				debug(DEBUG_ERROR, "No cred object in JSON\n");
				goto fail;
			}

			akm_token = json_get_member(cred, "akm");
			if (!akm_token || akm_token->type != JSON_STRING) {
				debug(DEBUG_ERROR, "No cred::akm string value found\n");
				goto fail;
			}

			debug(DEBUG_ERROR, "bss(%d) akm %s\n",index, akm_token->string);

			akm = dpp_akm_from_str(akm_token->string);

			if (dpp_akm_legacy(akm)) {
				if (dpp_parse_cred_legacy(&auth, cred) < 0)
					goto fail;

				if (akm == DPP_AKM_PSK_SAE)
					r3_ctx->bss_config_data[index].config_data.AuthMode = AUTH_WPA2_PERSONAL|AUTH_SAE_PERSONAL;
				else if (akm == DPP_AKM_PSK)
					r3_ctx->bss_config_data[index].config_data.AuthMode = AUTH_WPA2_PERSONAL;
				else if (dpp_akm_sae(akm))
					r3_ctx->bss_config_data[index].config_data.AuthMode = AUTH_SAE_PERSONAL;

				r3_ctx->bss_config_data[index].config_data.EncrypType = ENCRYP_AES;
			} else if (dpp_akm_dpp(akm)) {
				if(dpp_akm_ver2(akm)) {
					if (dpp_parse_cred_legacy(&auth, cred) < 0)
						goto fail;
				}
				if (akm == DPP_AKM_PSK_SAE_DPP)
					r3_ctx->bss_config_data[index].config_data.AuthMode = AUTH_DPP_ONLY|AUTH_WPA2_PERSONAL|AUTH_SAE_PERSONAL;
				else if (akm == DPP_AKM_SAE_DPP)
					r3_ctx->bss_config_data[index].config_data.AuthMode = AUTH_DPP_ONLY|AUTH_SAE_PERSONAL;
				else if (akm == DPP_AKM_PSK_DPP)
					r3_ctx->bss_config_data[index].config_data.AuthMode = AUTH_DPP_ONLY|AUTH_WPA2_PERSONAL;
				else
					r3_ctx->bss_config_data[index].config_data.AuthMode = AUTH_DPP_ONLY;

				r3_ctx->bss_config_data[index].config_data.EncrypType = ENCRYP_AES;

				if (obj_len > 1024) {
					debug(DEBUG_ERROR, "obj len too big, change to 1024\n");
					obj_len = 1024;
				}

				/* R3 ext Cred will be filled up while BSS is set to both BH and FH */
				if ((map_vendor_extension[index] & BIT_BH_BSS) &&
					(map_vendor_extension[index] & BIT_FH_BSS)) {
					debug(DEBUG_TRACE, "obj len %d\n", obj_len);

					os_memcpy(r3_ctx->bss_config_data[index].config_data.ext_cred,
						conf_rsp_obj, obj_len);
					r3_ctx->bss_config_data[index].config_data.ext_cred_len = obj_len;
					debug(DEBUG_TRACE, "map_vendor_extension are both FH and BH\n");
				}
				/* R3 Cred will be filled up while BSS is set to BH or FH */
				else {
					debug(DEBUG_TRACE, "map_vendor_extension is %s only\n",
						(r3_ctx->bss_config_data[index].config_data.map_vendor_extension & BIT_BH_BSS)?"BH":"FH");
					os_memcpy(r3_ctx->bss_config_data[index].config_data.cred, conf_rsp_obj, obj_len);
					r3_ctx->bss_config_data[index].config_data.cred_len = obj_len;
					debug(DEBUG_TRACE, "obj len %d\n", obj_len);
				}
			} else {
				debug(DEBUG_ERROR, "Unsupported akm: %s", akm_token->string);
				goto fail;
			}

			if (dpp_akm_legacy(akm) || dpp_akm_ver2(akm)) {
				if (auth.psk_set) {
					if (wpa_snprintf_hex((char *)r3_ctx->bss_config_data[index].config_data.WPAKey, 65,
						auth.psk, PMK_LEN) != 64) {
						debug(DEBUG_ERROR, "bss(%d) fail to copy psk\n", index);
						goto fail;
					}
					debug(DEBUG_ERROR, "psk set to bss bss(%d)\n", index);
				} else
					os_memcpy(r3_ctx->bss_config_data[index].config_data.WPAKey,
					auth.passphrase, sizeof(auth.passphrase));
			}
			index++;
			if (index >= MAX_SET_BSS_INFO_NUM) {
				debug(DEBUG_ERROR, "bss index(%d) equal to or bigger than %d\n",
					index, MAX_SET_BSS_INFO_NUM);
				goto fail;
			}
		}

		left -= (obj_len + 4);
		temp_buf += (obj_len + 4);
	}
	r3_ctx->total_config_num += index;
	return 0;
fail:
	return -1;
}

#ifdef PTK_REKEY
void resend_ptk_rekey_request_msg(void *eloop_ctx, void *timeout_ctx)
{
	struct wpa_entry_info *entry = eloop_ctx;
	struct p1905_managerd_ctx *ctx = timeout_ctx;
	struct topology_response_db *tpgr_db = NULL;
	int ifidx = -1;

	SLIST_FOREACH(tpgr_db, &(ctx->topology_entry.tprdb_head), tprdb_entry) {
		if(!memcmp(entry->peer_addr, tpgr_db->al_mac_addr, ETH_ALEN)) {
		   ifidx = tpgr_db->recv_ifid;
		}
	}

	insert_cmdu_txq(entry->peer_addr, entry->own_addr,
		e_ptk_rekey_request, ++ctx->mid,
		(ifidx != -1) ? ctx->itf[ifidx].if_name : ctx->br_name, 0);

	debug(DEBUG_ERROR, "to "MACSTR" on interface %s\n",MAC2STR(entry->peer_addr),
		(ifidx != -1) ? ctx->itf[ifidx].if_name : ctx->br_name);

	return;
}

void resend_ptk_rekey_request_timer(void *eloop_ctx, void *timeout_ctx)
{
	struct wpa_parameter *wpa_param = eloop_ctx;
	struct wpa_entry_info *entry, *entry_next;
	unsigned char peer_cnt = 0;

	dl_list_for_each_safe(entry, entry_next, &wpa_param->entry_list, struct wpa_entry_info, list) {
		/*  avoid creating a burst of control message traffic */
		debug(DEBUG_ERROR, "start timer %d(s) to send ptk rekey request to "MACSTR"\n",
			peer_cnt * 10, MAC2STR(entry->peer_addr));
		eloop_cancel_timeout(resend_ptk_rekey_request_msg, (void *)entry, (void *)wpa_param->p1905_ctx);
		eloop_register_timeout(peer_cnt * 5, 0,
			resend_ptk_rekey_request_msg, (void *)entry, wpa_param->p1905_ctx);
		peer_cnt ++;
	}

	eloop_cancel_timeout(resend_ptk_rekey_request_timer, (void *)wpa_param, NULL);
	eloop_register_timeout(wpa_param->wpa_conf.ptk_rekey_timer, 0,
		resend_ptk_rekey_request_timer, (void *)wpa_param, NULL);
}
#endif // PTK_REKEY

void resend_gtk_timer(void *eloop_ctx, void *timeout_ctx)
{
	struct wpa_parameter *wpa_param = eloop_ctx;
	struct wpa_entry_info *entry = NULL, *entry_next = NULL;

	/* GTK only can be update @ init or rekey stage */
	if (wpa_param->role == CONTROLLER) {
		wpa_gtk_update(&wpa_param->group);
	}

	dl_list_for_each_safe(entry, entry_next, &wpa_param->entry_list, struct wpa_entry_info, list) {
		wpa_send_group_msg_1(entry->wpa_sm);
	}

	eloop_cancel_timeout(resend_gtk_timer, (void *)wpa_param, NULL);
    eloop_register_timeout(wpa_param->wpa_conf.gtk_rekey_timer, 0,
		resend_gtk_timer, (void *)wpa_param, NULL);
}

int append_agent_list_tlv(unsigned char *pkt, struct p1905_managerd_ctx *ctx)
{
	unsigned char *temp_buf = NULL;
    unsigned char agent_cnt = 0;
	unsigned char *p_cnt = NULL;
	unsigned short total_length = 0;
	struct r3_member *peer = NULL;

	temp_buf = pkt;
	(*temp_buf) = AGENT_LIST_TLV_TYPE;
	temp_buf += 3;

    /* Number of Agents */
    p_cnt = temp_buf;
    temp_buf ++;
	dl_list_for_each(peer, &r3_member_head, struct r3_member, entry) {
        agent_cnt ++;

        /* AL MAC address of the Agent */
        os_memcpy(temp_buf, peer->al_mac, ETH_ALEN);
        temp_buf += ETH_ALEN;

        /* Latest profile supported by the Agent */
        *temp_buf++ = peer->profile;

		/* 0x01: 1905 Security enabled default */
        *temp_buf++ = peer->security_1905;

        debug(DEBUG_ERROR, "append peer "MACSTR" profile %d, 1905 security %d\n",
			MAC2STR(peer->al_mac), peer->profile, peer->security_1905);
	}

    *p_cnt = agent_cnt;

	total_length = (unsigned short)(temp_buf - pkt);

	*(unsigned short *)(pkt + 1) = host_to_be16(total_length - 3);

	return total_length;
}

unsigned short agent_list_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx, unsigned char *al_mac)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length =0;

	msg_hdr = (cmdu_message_header*)buf;

	/* one or more BSS Configuration response TLV */
	length = append_agent_list_tlv(tlv_temp_buf, ctx);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	 *-8(cmdu header size)) ==>padding
	 */
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(AGENT_LIST_MESSAGE);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}


int parse_agent_list_tlv(unsigned char *buf, unsigned short len,
	unsigned char *agent_cnt, struct r3_member *peer)
{
	unsigned short left = len;
	unsigned char *temp_buf;
	unsigned char i = 0;

	temp_buf = buf;

	if (left < 1) {
		debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
			__LINE__, left, 1);
		return -1;
	}

	*agent_cnt = *temp_buf++;
	left -= 1;

	if (*agent_cnt > 10) {
		debug(DEBUG_ERROR, "%d: error agent_cnt %d more than 10\n",
			__LINE__, *agent_cnt);
		return -1;
	}

	for (i = 0; i < *agent_cnt; i++) {
		if (left < 8) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
				__LINE__, left, 8);
			return -1;
		}
		os_memcpy(peer[i].al_mac, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;

		peer[i].profile = *temp_buf++;
		peer[i].security_1905 = *temp_buf++;
		left -= 8;
    }

	return 0;
}



int parse_agent_list_message(unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned int integrity = 0;
	unsigned char i = 0, agent_cnt = 0;
	struct r3_member peers[10];
	struct r3_member *peer = NULL;

	os_memset(peers, 0, sizeof(peers));

	type = buf;
	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = 0x%02x, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}
		if (*type == AGENT_LIST_TLV_TYPE) {
			ret = parse_agent_list_tlv(value, len, &agent_cnt, peers);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error al mac tlv\n");
				return -1;
			}
			integrity |= (1 << 0);
		} else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
		/*check integrity*/
	if (integrity < 0x01) {
		debug(DEBUG_ERROR, "error when check agent list message tlvs\n");
		return -1;
	}

	for (i = 0; i < agent_cnt; i++) {
		peer = create_r3_member(peers[i].al_mac);
		if (!peer)
			continue;

		peer->profile = peers[i].profile;
		debug(DEBUG_ERROR, "peer "MACSTR" profile %d, 1905 security status %d\n",
			MAC2STR(peer->al_mac), peer->profile, peer->security_1905);
	}
	return 0;
}


unsigned short append_ap_wf6_capability_tlv(
        unsigned char *pkt, struct ap_wf6_role_db *wf6_cap)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf = NULL;
	unsigned char *length = NULL;
	int i = 0;

	temp_buf = pkt;

	*temp_buf++ = 0xAA;

	length = temp_buf;
//	*(unsigned short *)(temp_buf) = host_to_be16(13 + he_cap->he_mcs_len);
	temp_buf += 2;

	memcpy(temp_buf, wf6_cap->identifier, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;

	*temp_buf++ = wf6_cap->role_supp;
	total_length++;

	for(i = 0; i < wf6_cap->role_supp; i++) {
		*temp_buf++ = (wf6_cap->wf6_role[i].agent_role << 6) | (wf6_cap->wf6_role[i].he_160 << 5) |
			(wf6_cap->wf6_role[i].he_8080 << 4 | wf6_cap->wf6_role[i].he_mcs_len);
		total_length++;

		os_memcpy(temp_buf, wf6_cap->wf6_role[i].he_mcs, wf6_cap->wf6_role[i].he_mcs_len);
		temp_buf += wf6_cap->wf6_role[i].he_mcs_len;
		total_length += wf6_cap->wf6_role[i].he_mcs_len;

		*temp_buf++ = (wf6_cap->wf6_role[i].su_bf_cap << 7) |(wf6_cap->wf6_role[i].su_beamformee_status << 6) |
			(wf6_cap->wf6_role[i].mu_bf_cap << 5) | (wf6_cap->wf6_role[i].beamformee_sts_less80 << 4) |
			(wf6_cap->wf6_role[i].beamformee_sts_more80 << 3) | (wf6_cap->wf6_role[i].ul_mu_mimo_cap << 2) |
			(wf6_cap->wf6_role[i].ul_ofdma_cap << 1) | wf6_cap->wf6_role[i].dl_ofdma_cap;
		total_length++;


		*temp_buf++ = (wf6_cap->wf6_role[i].max_user_dl_tx_mu_mimo << 4) |
			(wf6_cap->wf6_role[i].max_user_ul_rx_mu_mimo);
		total_length++;

		*temp_buf++ = wf6_cap->wf6_role[i].max_user_dl_tx_ofdma;
		total_length++;

		*temp_buf++ = wf6_cap->wf6_role[i].max_user_ul_rx_ofdma;
		total_length++;

		*temp_buf++ = (wf6_cap->wf6_role[i].rts_status << 7) | (wf6_cap->wf6_role[i].mu_rts_status << 6) |
			(wf6_cap->wf6_role[i].m_bssid_status << 5) | (wf6_cap->wf6_role[i].mu_edca_status << 4) |
			(wf6_cap->wf6_role[i].twt_requester_status <<3) | (wf6_cap->wf6_role[i].twt_responder_status <<2)
#ifdef MAP_R4_SPT
			| (wf6_cap->sr_mode << 1)
#endif
		;
		total_length++;

	//	total_length = 16 + he_cap->he_mcs_len;
	}

	*(unsigned short *)length = host_to_be16(total_length);

	return (total_length + 3);
}


int update_ap_wf6_cap(struct p1905_managerd_ctx* ctx, struct ap_wf6_cap_roles *pcap)
{
	struct ap_wf6_role_db *pdb;
	int i = 0;

	if (pcap->wf6_role[0].he_mcs_len > 12 || pcap->wf6_role[0].he_mcs_len < 4) {
		debug(DEBUG_ERROR, "invalid he_mcs_len(%d)\n", pcap->wf6_role[0].he_mcs_len);
		return -1;
	}

	SLIST_FOREACH(pdb, &(ctx->ap_cap_entry.ap_wf6_cap_head), ap_wf6_role_entry)
	{
		/*exist entry, update it*/
		if(!memcmp(pdb->identifier, pcap->identifier, ETH_ALEN))
		{
			debug(DEBUG_TRACE, "update ap he cap for existing radio("MACSTR")\n",
				PRINT_MAC(pcap->identifier));
			pdb->role_supp = pcap->role_supp;
#ifdef MAP_R4_SPT
			pdb->sr_mode = pcap->sr_mode;
#endif
			for(i = 0; i < pcap->role_supp; i++) {
				pdb->wf6_role[i].he_mcs_len = pcap->wf6_role[i].he_mcs_len;
				os_memcpy(pdb->wf6_role[i].he_mcs, pcap->wf6_role[i].he_mcs, pdb->wf6_role[i].he_mcs_len);
				pdb->wf6_role[i].tx_stream = pcap->wf6_role[i].tx_stream;
				pdb->wf6_role[i].rx_stream = pcap->wf6_role[i].rx_stream;
				pdb->wf6_role[i].he_8080 = pcap->wf6_role[i].he_8080;
				pdb->wf6_role[i].he_160 = pcap->wf6_role[i].he_160;
				pdb->wf6_role[i].su_bf_cap = pcap->wf6_role[i].su_bf_cap;
				pdb->wf6_role[i].mu_bf_cap = pcap->wf6_role[i].mu_bf_cap;
				pdb->wf6_role[i].ul_mu_mimo_cap = pcap->wf6_role[i].ul_mu_mimo_cap;
				pdb->wf6_role[i].ul_mu_mimo_ofdma_cap = pcap->wf6_role[i].ul_mu_mimo_ofdma_cap;
				pdb->wf6_role[i].dl_mu_mimo_ofdma_cap = pcap->wf6_role[i].dl_mu_mimo_ofdma_cap;
				pdb->wf6_role[i].ul_ofdma_cap = pcap->wf6_role[i].ul_ofdma_cap;
				pdb->wf6_role[i].dl_ofdma_cap = pcap->wf6_role[i].dl_ofdma_cap;
				pdb->wf6_role[i].agent_role = pcap->wf6_role[i].agent_role;
				pdb->wf6_role[i].su_beamformee_status = pcap->wf6_role[i].su_beamformee_status;
				pdb->wf6_role[i].beamformee_sts_less80 = pcap->wf6_role[i].beamformee_sts_less80;
				pdb->wf6_role[i].beamformee_sts_more80= pcap->wf6_role[i].beamformee_sts_more80;
				pdb->wf6_role[i].max_user_dl_tx_mu_mimo = pcap->wf6_role[i].max_user_dl_tx_mu_mimo;
				pdb->wf6_role[i].max_user_ul_rx_mu_mimo = pcap->wf6_role[i].max_user_ul_rx_mu_mimo;
				pdb->wf6_role[i].max_user_dl_tx_ofdma = pcap->wf6_role[i].max_user_dl_tx_ofdma;
				pdb->wf6_role[i].max_user_ul_rx_ofdma = pcap->wf6_role[i].max_user_ul_rx_ofdma;
				pdb->wf6_role[i].rts_status = pcap->wf6_role[i].rts_status;
				pdb->wf6_role[i].mu_rts_status = pcap->wf6_role[i].mu_rts_status;
				pdb->wf6_role[i].m_bssid_status = pcap->wf6_role[i].m_bssid_status;
				pdb->wf6_role[i].mu_edca_status = pcap->wf6_role[i].mu_edca_status;
				pdb->wf6_role[i].twt_requester_status = pcap->wf6_role[i].twt_requester_status;
				pdb->wf6_role[i].twt_responder_status = pcap->wf6_role[i].twt_responder_status;
			}

			return 0;
		}
	}
	/*insert new one*/
	debug(DEBUG_TRACE, "insert new ap vht cap for radio("MACSTR")\n",
		PRINT_MAC(pcap->identifier));
	pdb = (struct ap_wf6_role_db*)malloc(sizeof(struct ap_wf6_role_db));
	if(pdb == NULL)
	{
		debug(DEBUG_ERROR, "allocate memory fail ap_wf6_role_db\n");
		return -1;
	}
	memcpy(pdb->identifier, pcap->identifier, ETH_ALEN);
	pdb->role_supp = pcap->role_supp;
#ifdef MAP_R4_SPT
	pdb->sr_mode = pcap->sr_mode;
#endif
	for(i = 0; i < pcap->role_supp; i++) {
		pdb->wf6_role[i].he_mcs_len = pcap->wf6_role[i].he_mcs_len;
		os_memcpy(pdb->wf6_role[i].he_mcs, pcap->wf6_role[i].he_mcs, pdb->wf6_role[i].he_mcs_len);
		pdb->wf6_role[i].tx_stream = pcap->wf6_role[i].tx_stream;
		pdb->wf6_role[i].rx_stream = pcap->wf6_role[i].rx_stream;
		pdb->wf6_role[i].he_8080 = pcap->wf6_role[i].he_8080;
		pdb->wf6_role[i].he_160 = pcap->wf6_role[i].he_160;
		pdb->wf6_role[i].su_bf_cap = pcap->wf6_role[i].su_bf_cap;
		pdb->wf6_role[i].mu_bf_cap = pcap->wf6_role[i].mu_bf_cap;
		pdb->wf6_role[i].ul_mu_mimo_cap = pcap->wf6_role[i].ul_mu_mimo_cap;
		pdb->wf6_role[i].ul_mu_mimo_ofdma_cap = pcap->wf6_role[i].ul_mu_mimo_ofdma_cap;
		pdb->wf6_role[i].dl_mu_mimo_ofdma_cap = pcap->wf6_role[i].dl_mu_mimo_ofdma_cap;
		pdb->wf6_role[i].ul_ofdma_cap = pcap->wf6_role[i].ul_ofdma_cap;
		pdb->wf6_role[i].dl_ofdma_cap = pcap->wf6_role[i].dl_ofdma_cap;
		pdb->wf6_role[i].agent_role = pcap->wf6_role[i].agent_role;
		pdb->wf6_role[i].su_beamformee_status = pcap->wf6_role[i].su_beamformee_status;
		pdb->wf6_role[i].beamformee_sts_less80 = pcap->wf6_role[i].beamformee_sts_less80;
		pdb->wf6_role[i].beamformee_sts_more80= pcap->wf6_role[i].beamformee_sts_more80;
		pdb->wf6_role[i].max_user_dl_tx_mu_mimo = pcap->wf6_role[i].max_user_dl_tx_mu_mimo;
		pdb->wf6_role[i].max_user_ul_rx_mu_mimo = pcap->wf6_role[i].max_user_ul_rx_mu_mimo;
		pdb->wf6_role[i].max_user_dl_tx_ofdma = pcap->wf6_role[i].max_user_dl_tx_ofdma;
		pdb->wf6_role[i].max_user_ul_rx_ofdma = pcap->wf6_role[i].max_user_ul_rx_ofdma;
		pdb->wf6_role[i].rts_status = pcap->wf6_role[i].rts_status;
		pdb->wf6_role[i].mu_rts_status = pcap->wf6_role[i].mu_rts_status;
		pdb->wf6_role[i].m_bssid_status = pcap->wf6_role[i].m_bssid_status;
		pdb->wf6_role[i].mu_edca_status = pcap->wf6_role[i].mu_edca_status;
		pdb->wf6_role[i].twt_requester_status = pcap->wf6_role[i].twt_requester_status;
		pdb->wf6_role[i].twt_responder_status = pcap->wf6_role[i].twt_responder_status;
	}
	SLIST_INSERT_HEAD(&(ctx->ap_cap_entry.ap_wf6_cap_head), pdb, ap_wf6_role_entry);
	return 0;
}

int parse_ap_wf6_capability_tlv(struct p1905_managerd_ctx *ctx,
	unsigned char *buf)
{
    unsigned char *temp_buf;
    unsigned short length = 0;

    temp_buf = buf;

    if((*temp_buf) == 0xAA) {
        temp_buf++;
    }
    else {
        return -1;
    }

    //calculate tlv length
    length = *(unsigned short *)temp_buf;
    length = be_to_host16(length);

    return (length+3);
}

void cmd_send_bss_reconfiguration_trigger(struct p1905_managerd_ctx *ctx, char *buf)
{
	int i = 0;
	unsigned char al_mac[ETH_ALEN] = {0};
	char *pos1=buf, *pos2=NULL;
	unsigned int mac_int[ETH_ALEN];
	struct r3_member *peer = NULL;

	if (_1905_read_set_config(ctx, ctx->wts_bss_cfg_file, ctx->bss_config,
		MAX_SET_BSS_INFO_NUM, &ctx->bss_config_num) < 0) {
		debug(DEBUG_ERROR, "load bss config fail from %s\n", ctx->wts_bss_cfg_file);
		return;
	}

	if (buf && (os_strlen(buf) > 0)) {
		while (NULL != (pos2 = strstr(pos1, " "))) {

			if (sscanf(pos1, "%02x:%02x:%02x:%02x:%02x:%02x", mac_int,
				(mac_int + 1), (mac_int + 2), (mac_int + 3),
				(mac_int + 4), (mac_int + 5)) < 0) {
				debug(DEBUG_ERROR, "[%d]sscanf fail\n", __LINE__);
				return;
			}

			pos1 = pos2+1;
			for (i = 0; i < ETH_ALEN; i++)
				al_mac[i] = (unsigned char)mac_int[i];

			peer = get_r3_member(al_mac);
			if (peer && peer->security_1905) {
				debug(DEBUG_ERROR, "send bss reconfiguration trigger msg to special Agent "MACSTR"\n", MAC2STR(al_mac));
				insert_cmdu_txq(peer->al_mac, ctx->p1905_al_mac_addr,
					e_bss_reconfiguration_trigger, ++ctx->mid, ctx->br_name, 0);
			}
			else {
				debug(DEBUG_ERROR, ""MACSTR" is not R3 onboarding, skip\n", MAC2STR(al_mac));
			}
		}
	}
	else {
		dl_list_for_each(peer, &r3_member_head, struct r3_member, entry) {
			if (peer->security_1905) {
				debug(DEBUG_ERROR, "send bss reconfiguration trigger to "MACSTR" in R3 member list\n", MAC2STR(peer->al_mac));
				insert_cmdu_txq(peer->al_mac, ctx->p1905_al_mac_addr,
					e_bss_reconfiguration_trigger, ++ctx->mid, ctx->br_name, 0);
			}
		}
	}

	return;
}

void add_chirp_hash_list(struct p1905_managerd_ctx *ctx,
	struct chirp_tlv_info *chirp_tlv, unsigned char *almac)
{
	struct hash_value_item *hv_item = NULL, *hv_item_nxt = NULL;
	int found = 0;
	unsigned char all_zero_mac[ETH_ALEN];
	os_memset(all_zero_mac, 0, sizeof(all_zero_mac));

	dl_list_for_each_safe(hv_item, hv_item_nxt,
		&ctx->r3_dpp.hash_value_list, struct hash_value_item, member) {
		if (!os_memcmp(hv_item->hashValue, chirp_tlv->hash_payload, chirp_tlv->hash_len)) {
			debug(DEBUG_ERROR, "update for "MACSTR"\n", MAC2STR(hv_item->almac));
			found = 1;
			break;
		}
	}

	if (!found) {
		hv_item = os_zalloc(sizeof(struct hash_value_item));
		if (!hv_item) {
			return;
		}
		os_memcpy(hv_item->almac, (unsigned char *)almac, ETH_ALEN);
		hv_item->hashLen = chirp_tlv->hash_len;
		os_memcpy(hv_item->hashValue, chirp_tlv->hash_payload, chirp_tlv->hash_len);
		debug(DEBUG_ERROR, "new for "MACSTR"\n", MAC2STR(hv_item->almac));

		dl_list_add(&ctx->r3_dpp.hash_value_list, &hv_item->member);
	}

	if ((os_memcmp(almac, all_zero_mac, ETH_ALEN))) {
		os_memcpy(hv_item->almac, (unsigned char *)almac, ETH_ALEN);
		debug(DEBUG_ERROR, "for almac "MACSTR"\n", MAC2STR(hv_item->almac));
	}
	if ((os_memcmp(chirp_tlv->enrollee_mac, all_zero_mac, ETH_ALEN))) {
		os_memcpy(hv_item->mac_apcli, chirp_tlv->enrollee_mac, ETH_ALEN);
		debug(DEBUG_ERROR, "for apcli mac "MACSTR"\n", MAC2STR(hv_item->mac_apcli));
	}
}

void report_akm_suit_cap_to_mapd(void *eloop_ctx, void *timeout_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;
	unsigned char tlv_buf[128] = {0};
	unsigned short tlv_len = 0;

	tlv_len = append_akm_suite_cap_tlv(tlv_buf, ctx);
	map_notify_akm_suit_cap(ctx, ctx->p1905_al_mac_addr, tlv_buf, tlv_len);
}

void report_1905_secure_cap_to_mapd(void *eloop_ctx, void *timeout_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;
	unsigned char tlv_buf[64] = {0};
	unsigned short tlv_len = 0;

	tlv_len = append_1905_layer_security_tlv(tlv_buf);
	map_notify_1905_secure_cap(ctx, ctx->p1905_al_mac_addr, tlv_buf, tlv_len);
}

void report_sp_standard_rule_to_mapd_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;

	report_sp_standard_rule_to_mapd(ctx);
}



void report_sp_standard_rule_to_mapd(struct p1905_managerd_ctx *ctx)
{
	ctx->revd_tlv.buf_len = get_standard_sp_rule(ctx->revd_tlv.buf, MAX_PKT_LEN,
						&ctx->sp_rule_static_list, &ctx->sp_dp_table,
						ADD_RULE, ctx->p1905_al_mac_addr);
	if (ctx->revd_tlv.buf_len)
		map_notify_standard_sp_rules(ctx, ctx->p1905_al_mac_addr,
			ctx->revd_tlv.buf, ctx->revd_tlv.buf_len);
}


#endif // MAP_R3

#ifdef MAP_R3_DE

unsigned short append_dev_inven_tlv(
		unsigned char *pkt, struct dev_inven *de)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf = NULL;
	unsigned char *length = NULL;
	int i = 0;

	temp_buf = pkt;

	*temp_buf++ = R3_DE_INVENTORY_TLV_TYPE;

	length = temp_buf;
	temp_buf += 2;

	*temp_buf++ = de->ser_num_len;
	total_length++;

	os_memcpy(temp_buf, de->ser_num, de->ser_num_len);
	temp_buf += de->ser_num_len;
	total_length += de->ser_num_len;

	*temp_buf++ = de->sw_ver_len;
	total_length++;

	os_memcpy(temp_buf, de->sw_ver, de->sw_ver_len);
	temp_buf += de->sw_ver_len;
	total_length += de->sw_ver_len;

	*temp_buf++ = de->exec_env_len;
	total_length++;

	os_memcpy(temp_buf, de->exec_env, de->exec_env_len);
	temp_buf += de->exec_env_len;
	total_length += de->exec_env_len;

	*temp_buf++ = de->num_radio;
	total_length ++;

	for(i = 0; i < de->num_radio; i++)
	{
		os_memcpy(temp_buf, de->ruid[i].identifier, ETH_ALEN);
		temp_buf += ETH_ALEN;
		total_length += ETH_ALEN;

		*temp_buf++ = de->ruid[i].chip_ven_len;
		total_length++;

		os_memcpy(temp_buf, de->ruid[i].chip_ven, de->ruid[i].chip_ven_len);
		temp_buf += de->ruid[i].chip_ven_len;
		total_length += de->ruid[i].chip_ven_len;

	}

	*(unsigned short *)length = host_to_be16(total_length);

	return total_length + 3;
}



int update_ap_dev_inven(struct p1905_managerd_ctx* ctx, struct dev_inven *de)
{
	int i = 0;

	os_memset(&ctx->de, '\0', sizeof(struct dev_inven));

	struct dev_inven_ruid *de_ruid;
	struct dev_inven_ruid *ctx_ruid;


	os_memcpy(ctx->de.sw_ver, de->sw_ver, de->sw_ver_len);
	ctx->de.sw_ver_len = de->sw_ver_len;

	os_memcpy(ctx->de.exec_env, de->exec_env, de->exec_env_len);
	ctx->de.exec_env_len = de->exec_env_len;

	os_memcpy(ctx->de.ser_num, de->ser_num, ETH_ALEN);
	ctx->de.ser_num_len = de->ser_num_len;

	ctx->de.num_radio = de->num_radio;

	for(i = 0; i < ctx->de.num_radio; i++)
	{
		de_ruid = &de->ruid[i];
		ctx_ruid = &ctx->de.ruid[i];

		os_memcpy(ctx_ruid->identifier, de_ruid->identifier, ETH_ALEN);

		os_memcpy(ctx_ruid->chip_ven, de_ruid->chip_ven, de_ruid->chip_ven_len);
		ctx_ruid->chip_ven_len = de_ruid->chip_ven_len;
	}

	return 0;
}

#endif //MAP_R3_DE


int init_interface_socket(struct p1905_managerd_ctx *ctx)
{
	unsigned int i = 0;
	struct sockaddr_ll sll;
	struct ifreq ifr;
	int ret = 0;

	for (i = 0; i < ctx->itf_number; i++) {
		ctx->sock_inf_recv_1905[i] = socket(AF_PACKET, SOCK_RAW, ETH_P_1905);
		if (ctx->sock_inf_recv_1905[i] < 0) {
			debug(DEBUG_ERROR, "cannot open socket on %s (%s)", ctx->itf[i].if_name,
				strerror(errno));
			goto fail;
		}

		os_memset(&ifr, 0, sizeof(ifr));
		ret = snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ctx->itf[i].if_name);
		if (os_snprintf_error(sizeof(ifr.ifr_name), ret)) {
			debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
			goto fail;
		}
		if ((ioctl(ctx->sock_inf_recv_1905[i], SIOCGIFINDEX, &ifr)) == -1) {
			debug(DEBUG_ERROR, "cannot get interface %s index (%s) i=%d\n",
				ctx->itf[i].if_name, strerror(errno), i);
			goto fail;
		}

		os_memset(&sll, 0, sizeof(sll));
		sll.sll_family = AF_PACKET;
		sll.sll_ifindex = ifr.ifr_ifindex;
		sll.sll_protocol = host_to_be16(ETH_P_1905);

		/*Get interface mac address*/
		if ((ioctl(ctx->sock_inf_recv_1905[i], SIOCGIFHWADDR, &ifr)) == -1) {
			debug(DEBUG_ERROR, "cannot get interface %s mac address (%s)",
				ctx->itf[i].if_name, strerror(errno));
			goto fail;
		}
		ether_addr_copy(ctx->itf[i].mac_addr, ifr.ifr_hwaddr.sa_data);

		if ((bind(ctx->sock_inf_recv_1905[i], (struct sockaddr *)&sll,
			sizeof(struct sockaddr_ll))) == -1) {
			debug(DEBUG_ERROR, "cannot bind raw socket to interface %s (%s)",
				ctx->itf[i].if_name, strerror(errno));
			goto fail;
		}

		ctx->itf[i].trx_config = 0;
	}

#ifdef SINGLE_BAND_SUPPORT
	ctx->wifi_present = 1;
#endif
	return 0;
fail:
	if (ctx->sock_inf_recv_1905[i] > 0) {
		close(ctx->sock_inf_recv_1905[i]);
		ctx->sock_inf_recv_1905[i] = -1;
	}
#ifdef SINGLE_BAND_SUPPORT
	ctx->wifi_present = 0;
	return 0;
#endif
	return -1;
}

int init_wireless_interface(struct p1905_managerd_ctx *ctx,
	struct interface_info_list_hdr *info)
{
	int i = 0;
	unsigned int itf_idx = 0;

	if (info->interface_count > ITF_NUM-5) {
		debug(DEBUG_ERROR, "intf num [%d] bigger than maxmum [%d]\n",
			info->interface_count, ITF_NUM-5);
		return -1;
	}
	for (i = 0; i < info->interface_count; i++) {

		itf_idx = ctx->itf_number;

		debug(DEBUG_ERROR, "itf name = %s\n", info->if_info[i].if_name);
		os_strncpy((char *)ctx->itf[itf_idx].if_name, (char *)info->if_info[i].if_name, IFNAMSIZ);
		ether_addr_copy(ctx->itf[itf_idx].mac_addr, info->if_info[i].if_mac_addr);

		/* role */
		if (os_strcmp((char *)info->if_info[i].if_role, "wiap") == 0)
			ctx->itf[itf_idx].is_wifi_ap = 1;
		else
			ctx->itf[itf_idx].is_wifi_sta = 1;

		debug(DEBUG_TRACE, "role[%s] = %s\n", info->if_info[i].if_name,
			info->if_info[i].if_role);
		/* media type */
		if (info->if_info[i].if_ch <= 14 && os_strcmp((char *)info->if_info[i].if_phymode, "B") == 0)
			ctx->itf[itf_idx].media_type = IEEE802_11B_24G_GROUP;
		else if (info->if_info[i].if_ch <= 14 && os_strcmp((char *)info->if_info[i].if_phymode, "G") == 0)
			ctx->itf[itf_idx].media_type = IEEE802_11G_24G_GROUP;
		else if (info->if_info[i].if_ch <= 14 && os_strcmp((char *)info->if_info[i].if_phymode, "N") == 0)
			ctx->itf[itf_idx].media_type = IEEE802_11N_24G_GROUP;
		else if (info->if_info[i].if_ch <= 14 && os_strcmp((char *)info->if_info[i].if_phymode, "AX") == 0)
			ctx->itf[itf_idx].media_type = IEEE802_11AX_GROUP;
		else if (info->if_info[i].if_ch > 14 && os_strcmp((char *)info->if_info[i].if_phymode, "A") == 0)
			ctx->itf[itf_idx].media_type = IEEE802_11A_5G_GROUP;
		else if (info->if_info[i].if_ch > 14 && os_strcmp((char *)info->if_info[i].if_phymode, "N") == 0)
			ctx->itf[itf_idx].media_type = IEEE802_11N_5G_GROUP;
		else if (info->if_info[i].if_ch > 14 && os_strcmp((char *)info->if_info[i].if_phymode, "AC") == 0)
			ctx->itf[itf_idx].media_type = IEEE802_11AC_5G_GROUP;
		else if (info->if_info[i].if_ch > 14 && os_strcmp((char *)info->if_info[i].if_phymode, "AX") == 0)
			ctx->itf[itf_idx].media_type = IEEE802_11AX_GROUP;
		else {
			debug(DEBUG_ERROR, "%s band[%d] & phymode[%s] mismatch\n",
				info->if_info[i].if_name, info->if_info[i].if_ch,
				info->if_info[i].if_phymode);
			return -1;
		}
		debug(DEBUG_TRACE, "media_type[%s] = %d\n", info->if_info[i].if_name,
			ctx->itf[itf_idx].media_type);
		/* radio indentifer */
		ether_addr_copy(ctx->itf[itf_idx].identifier, info->if_info[i].identifier);
		debug(DEBUG_TRACE, "identifier[%s] = "MACSTR"\n",
			info->if_info[i].if_name, PRINT_MAC(info->if_info[i].identifier));
		/* init vs_info */
		ctx->itf[itf_idx].vs_info_length = 10;
		ctx->itf[itf_idx].vs_info = os_zalloc(ctx->itf[itf_idx].vs_info_length);
		if (!ctx->itf[itf_idx].vs_info) {
			debug(DEBUG_ERROR, "cannot alloc vs_info for %s\n",
				info->if_info[i].if_name);
			return -1;
		}
		ether_addr_copy(ctx->itf[itf_idx].vs_info, ctx->itf[itf_idx].mac_addr);
		if (ctx->itf[itf_idx].is_wifi_sta) {
			/* non-ap station */
			*(ctx->itf[itf_idx].vs_info + 6) = 0x40;
			os_memset(ctx->itf[itf_idx].vs_info, 0, 6);
		}

		ctx->itf_number++;
	}

	if (init_interface_socket(ctx)) {
		debug(DEBUG_ERROR, "interface socket init fail\n");
		return -1;
	}

	if (lldpdu_init(ctx)) {
		debug(DEBUG_ERROR, "lldpdu_init fail\n");
		return -1;
	}

	return 0;
}


int common_info_init(struct p1905_managerd_ctx *ctx)
{
	unsigned int i = 0;

	if (!ctx->MAP_Cer) {
		/*reset 1905almac to ap interface mac*/
		for (i = 0; i < ctx->itf_number; i++) {
			/*(ctx->itf[i].media_type == IEEE802_3_GROUP && !ctx->itf[i].is_veth)*/
			if (!os_strlen((char *)ctx->al_inf)) {
				if ((ctx->itf[i].is_wifi_ap)
					&& os_memcmp(ctx->itf[i].mac_addr, ctx->br_addr, ETH_ALEN)) {
					os_memcpy(ctx->p1905_al_mac_addr, ctx->itf[i].mac_addr, ETH_ALEN);
					debug(DEBUG_OFF, "reset almac to "MACSTR"\n",
						PRINT_MAC(ctx->p1905_al_mac_addr));
					/*reset to 1905 config file*/
					if (write_almac_to_1905_cfg_file(ctx->map_cfg_file,
							ctx->p1905_al_mac_addr)) {
						debug(DEBUG_ERROR, "write almac to 1905 cfg file fail\n");
						return -1;
					}
					if (set_opt_not_forward_dest(ctx->p1905_al_mac_addr) < 0) {
						debug(DEBUG_ERROR, "set almac to forward module fail\n");
						return -1;
					}
					break;
				}
			} else {
				/*find al_inf , chose it's mac as al_mac*/
				if (!os_memcmp(ctx->al_inf, ctx->itf[i].if_name, IFNAMSIZ) &&
					(ctx->itf[i].is_wifi_ap) &&
					os_memcmp(ctx->itf[i].mac_addr, ctx->br_addr, ETH_ALEN)) {
					os_memcpy(ctx->p1905_al_mac_addr, ctx->itf[i].mac_addr, ETH_ALEN);
					debug(DEBUG_OFF, "al_inf(%s), reset almac to "MACSTR"\n",
						ctx->al_inf, PRINT_MAC(ctx->p1905_al_mac_addr));
					/*reset to 1905 config file*/
					if (write_almac_to_1905_cfg_file(ctx->map_cfg_file,
							ctx->p1905_al_mac_addr)) {
						debug(DEBUG_ERROR, "write almac to 1905 cfg file fail\n");
						return -1;
					}
					if (set_opt_not_forward_dest(ctx->p1905_al_mac_addr) < 0) {
						debug(DEBUG_ERROR, "set almac to forward module fail\n");
						return -1;
					}
					break;
				}
			}
		}
		if (i == ctx->itf_number)
			debug(DEBUG_ERROR, "cannot find suitable almac, use orginal one\n");
	}

	create_topology_tree(ctx);
	ctx->br_cap[0].interface_num = ctx->itf_number;
	ctx->br_cap[0].itf_mac_list= os_malloc(ETH_ALEN * ctx->br_cap[0].interface_num);
	if (!ctx->br_cap[0].itf_mac_list) {
		debug(DEBUG_ERROR, "cannot alloc br_cap itf_mac_list\n");
		return -1;
	}
	for(i = 0; i < ctx->br_cap[0].interface_num; i++)
		os_memcpy(ctx->br_cap[0].itf_mac_list + i * ETH_ALEN, ctx->itf[i].mac_addr, ETH_ALEN);

	for (i = 0; i < ctx->itf_number; i++) {
		os_memcpy(ctx->non_p1905_neighbor_dev[i].local_mac_addr, ctx->itf[i].mac_addr, ETH_ALEN);
	    LIST_INIT(&(ctx->non_p1905_neighbor_dev[i].non_p1905nbr_head));
		os_memcpy(ctx->p1905_neighbor_dev[i].local_mac_addr, ctx->itf[i].mac_addr, ETH_ALEN);
		LIST_INIT(&(ctx->p1905_neighbor_dev[i].p1905nbr_head));
		if (ctx->itf[i].is_wifi_ap)
			init_radio_info_by_intf(ctx, &ctx->itf[i]);
		if (ctx->itf[i].media_type == IEEE802_3_GROUP) {
			ctx->itf[i].trx_config = TX_MUL | RX_MUL | TX_UNI | RX_UNI;
			debug(DEBUG_ERROR, "%s is eth interface; enable all tx rx\n" ,ctx->itf[i].if_name);
		}
	}

	return 0;
}

int write_almac_to_1905_cfg_file(char *cfg_file_name, unsigned char *almac)
{
	FILE *file;
	char *cpos = NULL, *apos = NULL;
	char almac_str[32];
	int cpos_cnt = 0, apos_cnt = 0, i = 0;
	char file_name[MAX_FILE_PATH_LENGTH] = {0};
	char content[1024] = {0};
	signed char ch = 0;
	int tmp = 0;

	tmp = snprintf(file_name, sizeof(file_name), "%s", cfg_file_name);
	if (os_snprintf_error(sizeof(file_name), tmp)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		return -1;
	}

	file = fopen(file_name, "r");
	if (!file) {
		debug(DEBUG_ERROR, "open MAP cfg file (%s) fail\n", file_name);
		return -1;
	}
	do {
		tmp = fgetc(file);
		if (tmp == EOF)
			break;
		ch = (signed char)tmp;
		content[i++] = ch;
	}while (EOF != ch && i < (sizeof(content) - 1));
	(void)fclose(file);

	cpos = strstr(content, "map_controller_alid");
	if (!cpos) {
		debug(DEBUG_ERROR, "cannot find map_controller_alid str\n");
		return -1;
	}
	apos = strstr(content, "map_agent_alid");
	if (!apos) {
		debug(DEBUG_ERROR, "cannot find map_agent_alid str\n");
		return -1;
	}

	cpos_cnt = cpos - content + os_strlen("map_controller_alid");
	apos_cnt = apos - content + os_strlen("map_agent_alid");

	file = fopen(file_name, "r+");
	if (!file) {
		debug(DEBUG_ERROR, "open MAP cfg file (%s) fail at second time\n", file_name);
		return -1;
	}
	if (fseek(file, 0, SEEK_SET) != 0) {
		debug(DEBUG_ERROR, "[%d]feek fail!\n", __LINE__);
		(void)fclose(file);
		return -1;
	}
	if (fseek(file, cpos_cnt, SEEK_SET) != 0) {
		debug(DEBUG_ERROR, "[%d]feek fail!\n", __LINE__);
		(void)fclose(file);
		return -1;
	}
	do {
		tmp = fgetc(file);
		if (tmp == EOF)
			break;
		ch = (signed char)tmp;
		if (ch != '=' && ch != ' ')
			break;
	} while (ch != '\n' && ch != '\r');
	os_memset(almac_str, 0, sizeof(almac_str));
	tmp = snprintf(almac_str, sizeof(almac_str), ""MACSTR"", PRINT_MAC(almac));
	if (os_snprintf_error(sizeof(almac_str), tmp)) {
		(void)fclose(file);
		return -1;
	}

	if (fseek(file, -1, SEEK_CUR) != 0) {
		debug(DEBUG_ERROR, "[%d]feek fail!\n", __LINE__);
		(void)fclose(file);
		return -1;
	}
	if (fputs(almac_str, file) == EOF) {
		debug(DEBUG_ERROR, "[%d]fputs fail", __LINE__);
		(void)fclose(file);
		return -1;
	}
	if (fseek(file, 0, SEEK_SET) != 0) {
		debug(DEBUG_ERROR, "[%d]feek fail!\n", __LINE__);
		(void)fclose(file);
		return -1;
	}
	if (fseek(file, apos_cnt, SEEK_SET) != 0) {
		debug(DEBUG_ERROR, "[%d]feek fail!\n", __LINE__);
		(void)fclose(file);
		return -1;
	}
	do {
		tmp = fgetc(file);
		if (tmp == EOF)
			break;
		ch = (signed char)tmp;
		if (ch != '=' && ch != ' ')
			break;
	} while (ch != '\n' && ch != '\r');
	os_memset(almac_str, 0, sizeof(almac_str));
	tmp = snprintf(almac_str, sizeof(almac_str), ""MACSTR"", PRINT_MAC(almac));
	if (os_snprintf_error(sizeof(almac_str), tmp)) {
		debug(DEBUG_ERROR, "[%d]snprinft fail!\n", __LINE__);
		(void)fclose(file);
		return -1;
	}

	if (fseek(file, -1, SEEK_CUR) != 0) {
		debug(DEBUG_ERROR, "[%d]feek fail!\n", __LINE__);
		(void)fclose(file);
		return -1;
	}
	if (fputs(almac_str, file) == EOF) {
		debug(DEBUG_ERROR, "[%d]fputs fail", __LINE__);
		(void)fclose(file);
		return -1;
	}
	(void)fclose(file);

	return 0;
}


void mark_valid_topo_rsp_node(struct p1905_managerd_ctx *ctx)
{
    struct p1905_neighbor_info *p1905_neigbor = NULL;
    struct topology_response_db *tpgr = NULL;
	int i = 0;

	SLIST_FOREACH(tpgr, &ctx->topology_entry.tprdb_head, tprdb_entry) {
		debug(DEBUG_ERROR, TOPO_PREX"mark dev("MACSTR") invalid\n",
			PRINT_MAC(tpgr->al_mac_addr));
		tpgr->valid = 0;
	}

	for(i = 0; i < ctx->itf_number; i++) {
		LIST_FOREACH(p1905_neigbor, &(ctx->p1905_neighbor_dev[i].p1905nbr_head), p1905nbr_entry) {
			tpgr = find_topology_rsp_by_almac(ctx, p1905_neigbor->al_mac_addr);
			if (tpgr && !tpgr->valid) {
				tpgr->valid = 1;
				debug(DEBUG_ERROR, TOPO_PREX"mark dev("MACSTR") valid\n",
					PRINT_MAC(tpgr->al_mac_addr));
				update_topology_info(ctx, tpgr);
			}
		}
	}
}

void update_topology_info(
	struct p1905_managerd_ctx *ctx, struct topology_response_db *tpgr)
{
	struct device_info_db *dev_info = NULL;
	struct p1905_neighbor_device_db *p1905_neighbor = NULL;
    struct topology_response_db *tpgr_child = NULL;

	SLIST_FOREACH(dev_info, &tpgr->devinfo_head, devinfo_entry) {
		SLIST_FOREACH(p1905_neighbor, &dev_info->p1905_nbrdb_head, p1905_nbrdb_entry) {
			tpgr_child = find_topology_rsp_by_almac(ctx,
							p1905_neighbor->p1905_neighbor_al_mac);
			if (tpgr_child && !tpgr_child->valid) {
				tpgr_child->valid = 1;
				debug(DEBUG_ERROR, TOPO_PREX"mark dev("MACSTR") valid\n",
					PRINT_MAC(tpgr_child->al_mac_addr));
				update_topology_info(ctx, tpgr_child);
			}
		}
	}
}

struct topology_response_db *find_topology_rsp_by_almac(
	struct p1905_managerd_ctx *ctx, unsigned char *almac)
{
    struct topology_response_db *tpgr_db = NULL;

	SLIST_FOREACH(tpgr_db, &ctx->topology_entry.tprdb_head, tprdb_entry) {
		if (!os_memcmp(tpgr_db->al_mac_addr, almac, ETH_ALEN))
			return tpgr_db;
	}

	return tpgr_db;
}

unsigned short append_wts_content_vendor_tlv(unsigned char *pkt,
	FILE *f, int offset, int size)
{
	/*1905 internal using vendor specific tlv format
	   type			1 byte			0x11
	   length			2 bytes			0x, 0x
	   oui			3 bytes			0x00, 0x0C, 0xE7
	   function type	1 byte			0xff
	   suboui			3 bytes			0x00, 0x0C, 0xE7
	   sub func type	1 byte			0x
	*/
	unsigned char *p = pkt;
	unsigned short count = 0;
	unsigned char mtk_oui[3] = {0x00, 0x0C, 0xE7};
	int ret = 0;

	/*vendor specific tlv type*/
	*p++ = VENDOR_SPECIFIC_TLV_TYPE;

	/*vendor specific tlv length*/
	*((unsigned short*)(p)) = host_to_be16(8 + size);
	p += 2;

	/*oui*/
	os_memcpy(p, mtk_oui, sizeof(mtk_oui));
	p += 3;

	/*function type*/
	*p++ = 0xff;

	/*additional OUI*/
	os_memcpy(p, mtk_oui, sizeof(mtk_oui));
	p += 3;

	/*1905 internal used vendor specific subtype*/
	*p++ = internal_vendor_wts_content_message_subtype;

	/*add content*/
	ret = fseek(f, offset, SEEK_SET);
	if (ret) {
		debug(DEBUG_ERROR, "fseek file fail, offset(%d)\n", offset);
		return 0;
	}

    count = fread(p, 1, size, f);
	if (count != size) {
		debug(DEBUG_OFF, "file read error count(%d) size(%d) offset(%d)\n",
			count, size, offset);
		return 0;
	}

	return 11 + count;
}


/**
 *  create topology discovery message.
 *
 * \param  buf  pointer to cmdu message header start position.
 * \param  al_mac  pointer of local abstraction layer mac address.
 * \param  itf_mac  pointer of transmitting ibterface mac address.
 * \return length of total tlvs included in this message
 */
unsigned short create_vendor_specific_wts_content_message(unsigned char *buf,
	struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;
	int file_len = 0, i = 0;
	int file_cut_size = 1000;
	FILE *f;

	debug(DEBUG_TRACE, "==>\n");

	/*open file*/
	f = fopen(ctx->wts_bss_cfg_file, "r");
	if (f == NULL) {
		debug(DEBUG_ERROR, "open file %s fail\n", ctx->wts_bss_cfg_file);
		return 0;
	}

	/*get file size*/
	if (fseek(f, 0, SEEK_END) != 0) {
		debug(DEBUG_ERROR, "[%d]fseek fail!\n", __LINE__);
		(void)fclose(f);
		return 0;
	}
	file_len = ftell(f);
	if (file_len >= MAX_PKT_LEN - CMDU_HLEN - ETH_HLEN - 3) {
		debug(DEBUG_ERROR, "file %s too large to send(size-%d)\n",
			ctx->wts_bss_cfg_file, file_len);
		(void)fclose(f);
		return 0;
	}

	msg_hdr = (cmdu_message_header*)buf;

	/*fill wts content info*/

	for (i = 0; i <= file_len / file_cut_size; i++) {
		if (i < file_len / file_cut_size)
			length = append_wts_content_vendor_tlv(tlv_temp_buf, f, i * file_cut_size, file_cut_size);
		else
			length = append_wts_content_vendor_tlv(tlv_temp_buf, f, i * file_cut_size, file_len % file_cut_size);
		if (!length) {
			(void)fclose(f);
			return 0;
		}
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}

	(void)fclose(f);

	length = append_end_of_tlv(tlv_temp_buf);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	/*tlvs size is less than (46(minimun ethernet frame payload size)
	*-8(cmdu header size)) ==>padding
	*/
	if(total_tlvs_length < MIN_TLVS_LENGTH)
	{
		memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
		total_tlvs_length = MIN_TLVS_LENGTH;
	}

	//0x00: for this version of the specification
	//0x01~0xFF: Reserved Values
	msg_hdr->message_version = 0x0;

	//set reserve field to 0
	msg_hdr->reserved_field_0 = 0x0;
	msg_hdr->message_type = host_to_be16(VENDOR_SPECIFIC);
	msg_hdr->relay_indicator = 0x0;
	//set reserve field to 0
	msg_hdr->reserve_field_1 = 0x0;

	return total_tlvs_length;
}

int delete_exist_steer_cntrl_db(struct p1905_managerd_ctx *ctx)
{
	struct control_policy_db *con_policy = NULL, *con_policy_tmp = NULL;
	struct sta_db *sta = NULL, *sta_tmp = NULL;

    con_policy = SLIST_FIRST(&ctx->steer_cntrl.policy_head);
	while(con_policy) {
		sta = SLIST_FIRST(&con_policy->sta_head);
		while (sta) {
	        sta_tmp = SLIST_NEXT(sta, sta_entry);
	        SLIST_REMOVE(&con_policy->sta_head, sta,
	                   sta_db, sta_entry);
	        free(sta);
	        sta = sta_tmp;
		}
		con_policy_tmp = SLIST_NEXT(con_policy, policy_entry);
		SLIST_REMOVE(&ctx->steer_cntrl.policy_head, con_policy,
		            control_policy_db, policy_entry);
		free(con_policy);
		con_policy = con_policy_tmp;
		ctx->steer_cntrl.policy_cnt--;
    }
	SLIST_INIT(&ctx->steer_cntrl.policy_head);
	ctx->steer_cntrl.policy_cnt = 0;

	return wapp_utils_success;
}



int check_add_error_code_tlv(struct p1905_managerd_ctx *ctx, unsigned char *pkt,
	unsigned char max_sta_cnt, unsigned short msgtype)
{
	struct associated_clients_db *bss_db = NULL;
	struct clients_db *cli = NULL;
	struct unlink_metrics_query *unlink_query = NULL;
	struct beacon_metrics_query *bcn_query = NULL;
	struct control_policy_db *policy = NULL;
	struct sta_db *sta = NULL;
	unsigned char *pos =NULL;
	unsigned int i = 0, ack_len = 0, err_sta_cnt = 0;

	pos = pkt;
	switch (msgtype) {
	case UNASSOC_STA_LINK_METRICS_QUERY:
		unlink_query = ctx->metric_entry.unlink_query;
		if (!unlink_query) {
			debug(DEBUG_ERROR, "no valid sta in unassoc metrics query msg\n");
			return -1;
		}
		for (i = 0; i < unlink_query->sta_num; i++) {
			SLIST_FOREACH(bss_db, &ctx->ap_cap_entry.assoc_clients_head, assoc_clients_entry)
			{
				SLIST_FOREACH(cli, &bss_db->clients_head, clients_entry)
				{
					if (!memcmp(cli->mac, &(unlink_query->sta_list[i * ETH_ALEN]), ETH_ALEN)) {
						/*add error code tlv logic here*/
						*pos++ = ERROR_CODE_TYPE;
						*pos++ = 0x00;
						*pos++ = 0x07;
						*pos++ = 0x01;
						os_memcpy(pos, cli->mac, ETH_ALEN);
						pos += ETH_ALEN;
						ack_len += 10;
						err_sta_cnt++;
						if (err_sta_cnt > max_sta_cnt) {
							debug(DEBUG_ERROR, "too many error sta\n");
							return -1;
						}
						break;
					}
				}
				if (cli)
					break;
			}
		}
	break;
	case BEACON_METRICS_QUERY:
		bcn_query = ctx->metric_entry.bcn_query;
		if (!bcn_query) {
			debug(DEBUG_ERROR, "no valid sta in bcn metrics query msg\n");
			return -1;
		}
		SLIST_FOREACH(bss_db, &ctx->ap_cap_entry.assoc_clients_head, assoc_clients_entry)
		{
			SLIST_FOREACH(cli, &bss_db->clients_head, clients_entry)
			{
				if (!memcmp(cli->mac, bcn_query->sta_mac, ETH_ALEN))
					break;
			}
			if (cli)
				break;
		}
		if (!cli) {
			/*add error code tlv logic here*/
			*pos++ = ERROR_CODE_TYPE;
			*pos++ = 0x00;
			*pos++ = 0x07;
			*pos++ = 0x02;
			os_memcpy(pos, bcn_query->sta_mac, ETH_ALEN);
			pos += ETH_ALEN;
			ack_len += 10;
			err_sta_cnt++;
		}
	break;
	case CLIENT_ASSOC_CONTROL_REQUEST:
		SLIST_FOREACH(policy, &ctx->steer_cntrl.policy_head, policy_entry)
		{
			SLIST_FOREACH(sta, &policy->sta_head, sta_entry)
			{
				SLIST_FOREACH(bss_db, &ctx->ap_cap_entry.assoc_clients_head, assoc_clients_entry)
				{
					SLIST_FOREACH(cli, &bss_db->clients_head, clients_entry)
					{
						if (!memcmp(cli->mac, sta->mac, ETH_ALEN) &&
							!memcmp(bss_db->bssid, policy->bssid, ETH_ALEN) &&
							policy->assoc_control == 0x00) {
							/*add error code tlv logic here*/
							*pos++ = ERROR_CODE_TYPE;
							*pos++ = 0x00;
							*pos++ = 0x07;
							*pos++ = 0x01;
							os_memcpy(pos, cli->mac, ETH_ALEN);
							pos += ETH_ALEN;
							ack_len += 10;
							err_sta_cnt++;
							if (err_sta_cnt > max_sta_cnt) {
								debug(DEBUG_ERROR, "too many error sta\n");
								return -1;
							}
							break;
						}
					}
					if (cli)
						break;
				}
			}
		}
	break;
	default:
	break;
	}

	if (ack_len) {
		reset_send_tlv(ctx);
		if (fill_send_tlv(ctx, pkt, ack_len) < 0) {
			debug(DEBUG_ERROR, "fill_send_tlv fail\n");
			return -1;
		}
	}

	return 0;
}

int check_C_and_A_concurrent_by_topology_rsp(struct p1905_managerd_ctx *ctx, struct topology_response_db *tpgr_db)
{
	struct topology_discovery_db *ctpg_db = NULL, *atpg_db = NULL, *tpg_db = NULL;

	if (tpgr_db->support_service != 1) {
		ctpg_db = find_discovery_by_almac(ctx, tpgr_db->al_mac_addr);
		if (ctpg_db) {
			LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry)
			{
				if (!os_memcmp(ctpg_db->itf_mac, tpg_db->itf_mac, ETH_ALEN) &&
					os_memcmp(ctpg_db->al_mac, tpg_db->al_mac, ETH_ALEN)) {
					debug(DEBUG_ERROR, "BCM case!!! C("MACSTR") & A("MACSTR") almac different but same intf_mac("MACSTR")!!! del discovery("MACSTR")\n",
						MAC2STR(ctpg_db->al_mac), MAC2STR(tpg_db->al_mac),
						MAC2STR(tpg_db->itf_mac), MAC2STR(tpg_db->al_mac));
					delete_exist_topology_discovery_database(ctx, tpg_db->al_mac, tpg_db->itf_mac);
					delete_exist_topology_response_database(ctx, tpg_db->al_mac);
					/*report own topology rsp to update db in mapd*/
					report_own_topology_rsp(ctx, ctx->p1905_al_mac_addr, ctx->br_cap,
						ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
						ctx->service, &ctx->ap_cap_entry.oper_bss_head,
						&ctx->ap_cap_entry.assoc_clients_head, ctx->cnt);
				}
			}
		}
	} else {
		atpg_db = find_discovery_by_almac(ctx, tpgr_db->al_mac_addr);
		if (atpg_db) {
			LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry)
			{
				if (!os_memcmp(atpg_db->itf_mac, tpg_db->itf_mac, ETH_ALEN) &&
					os_memcmp(atpg_db->al_mac, tpg_db->al_mac, ETH_ALEN)) {
					debug(DEBUG_ERROR, "BCM case!!! C("MACSTR") & A("MACSTR") almac different but same intf_mac("MACSTR")!!! del discovery("MACSTR")\n",
						MAC2STR(tpg_db->al_mac), MAC2STR(atpg_db->al_mac),
						MAC2STR(tpg_db->itf_mac), MAC2STR(atpg_db->al_mac));
					delete_exist_topology_discovery_database(ctx, atpg_db->al_mac, atpg_db->itf_mac);
					delete_exist_topology_response_database(ctx, atpg_db->al_mac);
					/*report own topology rsp to update db in mapd*/
					report_own_topology_rsp(ctx, ctx->p1905_al_mac_addr, ctx->br_cap,
						ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
						ctx->service, &ctx->ap_cap_entry.oper_bss_head,
						&ctx->ap_cap_entry.assoc_clients_head, ctx->cnt);
					return -1;
				}
			}
		}
	}

	return 0;
}

int check_C_and_A_concurrent__by_discovery(struct p1905_managerd_ctx *ctx, unsigned char *al_mac_addr, unsigned char *mac_addr)
{
	struct topology_response_db *tpgr_db = NULL;
	struct device_info_db *dev_info = NULL;

	SLIST_FOREACH(tpgr_db, &ctx->topology_entry.tprdb_head, tprdb_entry) {
		SLIST_FOREACH(dev_info, &tpgr_db->devinfo_head, devinfo_entry)
		{
			if (!os_memcmp(dev_info->mac_addr, mac_addr, ETH_ALEN) &&
				tpgr_db->support_service != 1 &&
				os_memcmp(tpgr_db->al_mac_addr, al_mac_addr, ETH_ALEN)) {
					debug(DEBUG_ERROR, "BCM case!!! C("MACSTR") & A("MACSTR") almac different but same intf_mac("MACSTR")!!! drop discovery("MACSTR")\n",
						MAC2STR(tpgr_db->al_mac_addr), MAC2STR(al_mac_addr),
						MAC2STR(mac_addr), MAC2STR(al_mac_addr));
					return -1;
			}
		}
	}

	return 0;
}

void set_trx_config_for_bss(struct p1905_managerd_ctx*ctx, unsigned char map_vendor_extension, unsigned char *bss_mac)
{
	unsigned int i = 0;

	for (i = 0; i < ctx->itf_number; i++) {
		if (!os_memcmp(ctx->itf[i].mac_addr, bss_mac, ETH_ALEN) &&
			ctx->itf[i].is_wifi_ap) {
			if (!(map_vendor_extension & BIT_BH_BSS))
				ctx->itf[i].trx_config = 0;
			else
				ctx->itf[i].trx_config = TX_MUL | RX_MUL | TX_UNI | RX_UNI;
			debug(DEBUG_ERROR, "%s is %s bss; %s all tx rx for this interface\n",
				ctx->itf[i].if_name,
				map_vendor_extension & BIT_BH_BSS ? "bh" : "fh",
				map_vendor_extension & BIT_BH_BSS ? "enable" : "disable"
			);
			return;
		}
	}
}

/*delaying meesage handling function-init*/
int dm_init(struct p1905_managerd_ctx *ctx)
{
	dl_list_init(&ctx->dm_ctx.buf_list_head);

	return 0;
}

struct delayed_message_buf *get_dm_buf_entry(struct dl_list *list_head, IN unsigned char *almac,
	IN unsigned char event, IN char *if_name)
{
	struct delayed_message_buf *dm_buf = NULL;

	dl_list_for_each_reverse(dm_buf, list_head, struct delayed_message_buf, entry) {
		if (dm_buf->wait_for_event == event && !os_memcmp(almac, dm_buf->al_mac, ETH_ALEN) &&
			!os_memcmp(if_name, dm_buf->if_name, IFNAMSIZ))
			return dm_buf;
	}

	return NULL;
}

/*delaying meesage handling function-store*/
int store_dm_buffer(IN struct p1905_managerd_ctx *ctx, IN unsigned char *almac, IN unsigned char event,
	IN unsigned short mid, IN char *if_name)
{
#define DM_BUF_TIMEOUT 1
	struct delayed_message_buf *dm_buf = NULL;
	struct os_time now;

	if (os_strlen(if_name) >= IFNAMSIZ)
		return -1;

	dm_buf = get_dm_buf_entry(&ctx->dm_ctx.buf_list_head, almac, event, if_name);

	if (dm_buf) {
		if (dm_buf->mid == mid){
			debug(DEBUG_ERROR, "do not store this dm_buffer since mid is same\n");
			return -1;
		}

		os_get_time(&now);

		if (now.sec <= dm_buf->time.sec+ DM_BUF_TIMEOUT) {
			debug(DEBUG_ERROR, "do not store this dm_buffer to prevent malicious attacks mid=%d\n",
				mid);
			return -1;
		}
	}

	dm_buf = (struct delayed_message_buf*)os_zalloc(sizeof(struct delayed_message_buf));

	if (!dm_buf) {
		debug(DEBUG_ERROR, "failing to allocate dm_buf\n");
		return -1;
	}

	os_memcpy(dm_buf->al_mac, almac, ETH_ALEN);
	dm_buf->wait_for_event = event;
	dm_buf->mid = mid;
	os_strncpy(dm_buf->if_name, if_name, os_strlen(if_name));
	os_get_time(&dm_buf->time);
	debug(DEBUG_TRACE, "almac="MACSTR" ifname=%s, mid=%d\n", PRINT_MAC(almac),
					if_name, mid);

	dl_list_add_tail(&ctx->dm_ctx.buf_list_head, &dm_buf->entry);

	return 0;
}

/*delaying meesage handling function-pop*/
int pop_dm_buffer(IN struct p1905_managerd_ctx *ctx, IN unsigned char event, OUT unsigned char *almac,
	OUT unsigned short *mid, OUT char *if_name)
{
	struct delayed_message_buf *dm_buf = NULL;

	dl_list_for_each(dm_buf, &ctx->dm_ctx.buf_list_head, struct delayed_message_buf, entry) {
		if (dm_buf->wait_for_event == event) {
			os_memcpy(almac, dm_buf->al_mac, ETH_ALEN);
			*mid = dm_buf->mid;
			os_strncpy(if_name, dm_buf->if_name,os_strlen(dm_buf->if_name));
			debug(DEBUG_TRACE, "almac="MACSTR" ifname=%s, mid=%d\n", PRINT_MAC(almac),
				if_name, *mid);

			dl_list_del(&dm_buf->entry);
			os_free(dm_buf);
			return 0;
		}
	}

	return -1;
}

/*delaying meesage handling function-clear_expired*/
int clear_expired_dm_buffer(IN struct p1905_managerd_ctx *ctx)
{
#define DM_BUF_EXPIRED 10

	struct os_time now;
	struct delayed_message_buf *dm_buf = NULL, *tmp = NULL;

	os_get_time(&now);
	dl_list_for_each_safe(dm_buf, tmp, &ctx->dm_ctx.buf_list_head, struct delayed_message_buf, entry) {
		if (now.sec >= dm_buf->time.sec + DM_BUF_EXPIRED) {
			debug(DEBUG_ERROR, "almac="MACSTR" ifname=%s, mid=%d\n", PRINT_MAC(dm_buf->al_mac),
				dm_buf->if_name, dm_buf->mid);
			dl_list_del(&dm_buf->entry);
			os_free(dm_buf);
		}
	}

	return 0;
}

/*delaying meesage handling function-deinit*/
int dm_deinit(struct p1905_managerd_ctx *ctx)
{
	struct delayed_message_buf *dm_buf = NULL, *tmp = NULL;

	dl_list_for_each_safe(dm_buf, tmp, &ctx->dm_ctx.buf_list_head, struct delayed_message_buf, entry) {
		dl_list_del(&dm_buf->entry);
		os_free(dm_buf);
	}

	return 0;
}

unsigned char find_band_by_opclass(char *opclass)
{
	if (!os_memcmp(opclass, "8x", 2))
		return BAND_2G_CAP;
	else if (!os_memcmp(opclass, "11x", 3))
		return BAND_5GL_CAP;
	else if (!os_memcmp(opclass, "12x", 3))
		return BAND_5GH_CAP;
	else if (!os_memcmp(opclass, "13x", 3))
		return BAND_6G_CAP;
	else
		return BAND_INVALID_CAP;
}

int cont_handle_agent_ap_radio_basic_capability(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	struct agent_radio_info *r, unsigned char radio_cnt)
{
	struct agent_list_db *agent_info = NULL;
	unsigned char i = 0;

	if (ctx->role != CONTROLLER)
		goto end;

	find_agent_info(ctx, al_mac, &agent_info);
	if (!agent_info) {
		debug(DEBUG_ERROR, "error! no agent info exist inset it\n");
		agent_info = insert_agent_info(ctx, al_mac);
	}

	if (!agent_info) {
		debug(DEBUG_ERROR, "error! fail to allocate agent\n");
		goto end;
	}

	for (i = 0; i < radio_cnt; i++)
		update_agent_radio_info(agent_info, &r[i]);
end:
	return 0;
}

#ifdef MAP_R3
int cont_handle_bss_configuration_request(struct p1905_managerd_ctx *ctx, unsigned char *al_mac,
	struct agent_radio_info *r, unsigned char radio_cnt)
{
	if (ctx->role != CONTROLLER)
		goto end;

	cont_handle_agent_ap_radio_basic_capability(ctx, al_mac, r, radio_cnt);

	insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr,
		e_bss_configuration_response, ++ctx->mid, ctx->itf[ctx->recent_cmdu_rx_if_number].if_name, 0);
	common_process(ctx, &ctx->trx_buf);

	/* renew state on controller */
	if (ctx->renew_ctx.trigger_renew) {
		debug(DEBUG_ERROR, "bss configuration renewing, pop another leaf\n");
		pop_node(&ctx->topo_stack);
		ctx->renew_state = wait_4_pop_node;
		eloop_cancel_timeout(controller_renew_bss_step, (void *)ctx, NULL);
		eloop_register_timeout(0, 0, controller_renew_bss_step, (void *)ctx, NULL);
	}
end:
	return 0;
}
#endif

const char *band_2_string(unsigned char band)
{
	switch (band) {
		case BAND_INVALID_CAP:
			return "INVALID_BAND";
		case BAND_2G_CAP:
			return "24G";
		case BAND_5GL_CAP:
			return "5GL";
		case BAND_5GH_CAP:
			return "5GH";
		case BAND_6G_CAP:
			return "6G";
		default:
			return "Unknown band";
	}
}
