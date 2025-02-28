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

#ifndef CMDU_TLV_PARSE_H
#define CMDU_TLV_PARSE_H

#include "p1905_managerd.h"
#include "cmdu_tlv.h"

struct vs_value {
	unsigned char discovery;
	unsigned char itf_mac[ETH_ALEN];
	unsigned char neth_almac[ETH_ALEN];
#ifdef MAP_R2
	struct transparent_vlan_setting trans_vlan;
	int parse_ts;
#endif
};

int get_tlv_len(unsigned char *buf);
int get_cmdu_tlv_length(unsigned char *buf);
int cmdu_sanity_check(unsigned char *buf, int len);
int parse_al_mac_addr_type_tlv(unsigned char *al_mac, unsigned short len, unsigned char *value);
int parse_mac_addr_type_tlv(unsigned char *mac, unsigned short len, unsigned char *value);
int parse_vs_tlv(struct vs_value *vs_info, unsigned short len, unsigned char *value);
int parse_device_info_type_tlv(struct p1905_managerd_ctx *ctx,
				struct list_head_devinfo *devinfo_head, unsigned short len, unsigned char *value);
int parse_bridge_capability_type_tlv(struct list_head_brcap *brcap_head, unsigned short len, unsigned char *value);
int parse_p1905_neighbor_device_type_tlv(struct list_head_devinfo *devinfo_head,
					unsigned short len, unsigned char *value);
int parse_non_p1905_neighbor_device_type_tlv(struct list_head_devinfo *devinfo_head,
					unsigned short len, unsigned char *value);
int parse_supported_role_tlv(unsigned short len, unsigned char *value);
int parse_supported_freq_band_tlv(unsigned char *band, unsigned short len, unsigned char *value);
int parse_wsc_tlv(struct p1905_managerd_ctx *ctx, unsigned short len, unsigned char *value);
int parse_link_metric_query_type_tlv(unsigned char *target, unsigned char *type, unsigned short len, unsigned char *value);
int parse_push_button_event_notification_tlv(unsigned char *buf, iee802_11_info *wifi_info);

int parse_search_role_tlv(unsigned short len, unsigned char *value);
int parse_auto_config_freq_band_tlv(unsigned char *band, unsigned short len, unsigned char *value);

#endif /*CMDU_TLV_PARSE_H*/
