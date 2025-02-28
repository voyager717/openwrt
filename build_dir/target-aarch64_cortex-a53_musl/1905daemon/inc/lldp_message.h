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

#ifndef LLDP_MESSAGE_H
#define LLDP_MESSAGE_H

#include <linux/if_ether.h>

#define LLDPDU_HLEN    2

#define END_OF_LLDPDU_TLV_TYPE  0
#define CHASSIS_ID_TLV_TYPE     1
#define PORT_ID_TLV_TYPE        2
#define TIME_TO_LIVE_TLV_TYPE   3

/*chassis id tlv subtype*/
#define C_CHASSIS_COMPONENT 1
#define C_INTERFACE_ALIAS   2
#define C_PORT_COMPONENT    3
#define C_MAC_ADDRESS       4
#define C_NETWORK_ADDRESS   5
#define C_INTERFACE_NAME    6
#define C_LOCAL_ASSIGN      7

/*port id tlv subtype*/
#define P_INTERFACE_ALIAS   1
#define P_PORT_COMPONENT    2
#define P_MAC_ADDRESS       3
#define P_NETWORK_ADDRESS   4
#define P_INTERFACE_NAME    5
#define P_AGENT_CIRCUIT_ID  6
#define P_LOCAL_ASSIGN      7

typedef union
{
    __be16 lldpdu_hdr;

    struct lldphdr
    {
        __be16 info_str_len:9;
        __be16 tlv_type:7;

    }lldpdu_msg;
}lldpdu_message_header;


unsigned short append_lldpdu_chassis_id_tlv(unsigned char *pkt,
                                            unsigned char *al_mac);

unsigned short append_lldpdu_port_id_tlv(unsigned char *pkt,
                                         unsigned char *itf_mac);

unsigned short append_lldpdu_time_to_live_tlv(unsigned char *pkt);

unsigned short append_lldpdu_end_of_lldpdu_tlv(unsigned char *pkt);

unsigned short create_802_1_bridge_discovery_message(unsigned char *buf,
                            unsigned char *al_mac, unsigned char *itf_mac);

#endif /* LLDP_MESSAGE_H */
