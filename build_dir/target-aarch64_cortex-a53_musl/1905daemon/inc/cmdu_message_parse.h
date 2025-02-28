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

#ifndef CMDU_MESSAGE_PARSE_H
#define CMDU_MESSAGEPARSE_H

#include <linux/if_ether.h>
#include "p1905_managerd.h"
#include "cmdu_tlv_parse.h"
#include "p1905_utils.h"

int parse_cmdu_message(struct p1905_managerd_ctx *ctx, unsigned char *buf,
    unsigned char *dmac, unsigned char *smac, int len);
char *get_mtype_str_from_msg_hdr(unsigned char *buf);
struct topology_discovery_db *get_tpd_db_by_mac(struct p1905_managerd_ctx *ctx, unsigned char *mac);
struct topology_response_db *get_trsp_db_by_mac(struct p1905_managerd_ctx *ctx, unsigned char *mac);
int delete_not_exist_p1905_neighbor_device(struct p1905_managerd_ctx *ctx,
	int sec);
void delete_not_exist_p1905_topology_device(struct p1905_managerd_ctx *ctx,
	int sec);
void delete_exist_topology_discovery_database(struct p1905_managerd_ctx *ctx,
	unsigned char *al_mac, unsigned char *itf_mac);

void delete_topo_response_db_by_port_interface(struct p1905_managerd_ctx *ctx,
	int port);

int delete_topo_disc_db_by_port(struct p1905_managerd_ctx *ctx,
	int port, unsigned char *exclude_almac);

int delete_p1905_neighbor_dev_info(struct p1905_managerd_ctx *ctx,
	unsigned char *al_mac, unsigned char *recvif_mac);
struct topology_response_db * delete_exist_topology_response_content(
	struct p1905_managerd_ctx *ctx, unsigned char *al_mac);

int insert_topology_discovery_database(struct p1905_managerd_ctx *ctx,
	unsigned char *al_mac, unsigned char *itf_mac, unsigned char *recv_itf_mac);

struct topology_discovery_db * find_discovery_by_almac(
	struct p1905_managerd_ctx *ctx, unsigned char *al_mac);
void find_neighbor_almac_by_intf_mac(
	struct p1905_managerd_ctx *ctx, unsigned char *itf_mac, unsigned char *almac);

struct topology_response_db * find_response_by_almac(
	struct p1905_managerd_ctx *ctx, unsigned char *al_mac);
int find_almac_by_connect_mac(struct p1905_managerd_ctx *ctx,
	unsigned char *mac, unsigned char *almac);
void mask_control_conn_port(struct p1905_managerd_ctx *ctx,
	struct topology_response_db *tpgr_db);
void unmask_control_conn_port(struct p1905_managerd_ctx *ctx, int port);
int parse_1905_ack_message(struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned int left_tlv_len);
int parse_map_version_tlv(unsigned char *buf, unsigned char *almac, unsigned char *profile);

#ifdef MAP_R3
void cmd_dev_start_buffer(struct p1905_managerd_ctx *ctx, char* buf);
void cmd_dev_stop_buffer(struct p1905_managerd_ctx *ctx);
void cmd_dev_stop_buffer_timeout(void *eloop_data, void *user_ctx);
void cmd_dev_get_frame_info(struct p1905_managerd_ctx *ctx, char* buf);
void dev_buffer_frame(struct p1905_managerd_ctx *ctx, unsigned short mtype,
    unsigned short mid, unsigned char *tlv_pos, int tlv_len);
int parse_controller_cap_tlv(unsigned char *buf, unsigned char *kibmib);
#endif
#endif /* CMDU_MESSAGEPARSE_H */
