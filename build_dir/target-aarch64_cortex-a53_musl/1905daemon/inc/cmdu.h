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

#ifndef CMDU_H
#define CMDU_H
#include <sys/queue.h>

#include "p1905_managerd.h"
#include "cmdu_message.h"
#include "cmdu_message_parse.h"
#include "cmdu_tlv_parse.h"
#include "cmdu_fragment.h"
#include "multi_ap.h"

/** Interfaces name */
#define LO_IFNAME    "lo"
#define ETH0_IFNAME  "eth0"
#define PLC0_IFNAME  "plc0"
#define WIFI0_IFNAME  "ra0"

#define ETH_P_1905	(0x893a)
#ifndef ETH_P_LLDP
#define ETH_P_LLDP  (0x88cc)
#endif

struct txq_list
{
	unsigned char ifname[IFNAMSIZ];
    unsigned char dmac[6];
    unsigned char smac[6];
    msgtype mtype;
    unsigned short mid;
	unsigned char need_update_mid;
    TAILQ_ENTRY(txq_list) cmdu_txq_entry;
};

int cmdu_init(struct p1905_managerd_ctx *ctx);
void cmdu_uninit(struct p1905_managerd_ctx *ctx);
int process_cmdu_txq(struct p1905_managerd_ctx *ctx,unsigned char *buffer);
void insert_cmdu_txq(unsigned char *dmac, unsigned char *smac,msgtype mtype,
	unsigned short mid, unsigned char* ifname, unsigned char band);
int cmdu_bridge_receive(struct p1905_managerd_ctx *ctx, int sock, unsigned char *buf,int len);
int cmdu_parse(struct p1905_managerd_ctx *ctx,unsigned char *buf,int len);
int sniffer_init(struct p1905_managerd_ctx *ctx);
int cmdu_virtual_if_receive(struct p1905_managerd_ctx *ctx, int sock, unsigned char *buf, int len);
void sniffer_uninit(struct p1905_managerd_ctx *ctx);
int cmdu_tx_fragment(struct p1905_managerd_ctx *ctx,
	unsigned char *buffer, unsigned short buflen, unsigned char *ifname);
int concurrent_sock_send(struct p1905_managerd_ctx *ctx,
	char *buffer_send, int length);
#ifdef SUPPORT_CONTROL_SOCKET
int _1905_ctrl_sock_init(struct p1905_managerd_ctx *ctx);
void _1905_ctrl_interface_recv_and_parse(struct p1905_managerd_ctx *ctx,
		char *buf, int len);
void  _1905_ctrl_sock_deinit(struct p1905_managerd_ctx *ctx);
#endif
#ifdef MAP_R3
int check_reliable_msg(unsigned short mtype);
#endif


#endif /* CMDU_H */
