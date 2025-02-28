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
#include <net/if.h>
#include <syslog.h>
#include <assert.h>
#include <linux/if_ether.h>
#include <asm/byteorder.h>
#include <lldp_message.h>

#define LLDP_TIME_TO_LIVE 180 //180 seconds

unsigned short append_lldpdu_chassis_id_tlv(unsigned char *pkt,
                                unsigned char *al_mac)
{
    lldpdu_message_header msg_hdr;
    unsigned char *temp_buf;

    temp_buf = pkt;

    msg_hdr.lldpdu_msg.tlv_type = CHASSIS_ID_TLV_TYPE;
    /*sting length = cahssis id subtype(1 octect) + mac addr(6 otects)*/
    msg_hdr.lldpdu_msg.info_str_len = 7;

    *temp_buf = (unsigned char)(msg_hdr.lldpdu_hdr >> 8);
    *(temp_buf + 1) = (unsigned char)(msg_hdr.lldpdu_hdr & 0xff);


    /*shift to chassis id subtype field*/
    temp_buf += LLDPDU_HLEN;
    /*set subtype to 4 ==> MAC ADDRESS*/
    (*temp_buf) = 4;

    /*shift to chassis id field*/
    temp_buf++;

    /*set chassis id to p1905.1 AL MAC ADDR*/
    memcpy(temp_buf,al_mac,ETH_ALEN);

    /*return total length*/
    return (LLDPDU_HLEN + 7);
}

unsigned short append_lldpdu_port_id_tlv(unsigned char *pkt,
                                         unsigned char *itf_mac)
{
    lldpdu_message_header msg_hdr;
    unsigned char *temp_buf;

    temp_buf = pkt;

    msg_hdr.lldpdu_msg.tlv_type = PORT_ID_TLV_TYPE;
    /*sting length = port ID subtype(1 octect) + mac addr(6 otects)*/
    msg_hdr.lldpdu_msg.info_str_len = 7;

    *temp_buf = (unsigned char)(msg_hdr.lldpdu_hdr >> 8);
    *(temp_buf + 1) = (unsigned char)(msg_hdr.lldpdu_hdr & 0xff);

    /*shift to port id subtype field*/
    temp_buf += LLDPDU_HLEN;
    /*set port id subtype to 3 ==> MAC ADDRESS*/
    (*temp_buf) = 3;

    /*shift to port id field*/
    temp_buf++;

    /*set port id to MAC ADDR of interface*/
    memcpy(temp_buf,itf_mac,ETH_ALEN);

    /*return total length*/
    return (LLDPDU_HLEN + 7);
}

unsigned short append_lldpdu_time_to_live_tlv(unsigned char *pkt)
{
    lldpdu_message_header msg_hdr;
    unsigned char *temp_buf;

    temp_buf = pkt;

    msg_hdr.lldpdu_msg.tlv_type = TIME_TO_LIVE_TLV_TYPE;
    /*sting length = 2*/
    msg_hdr.lldpdu_msg.info_str_len = 2;

    *temp_buf = (unsigned char)(msg_hdr.lldpdu_hdr >> 8);
    *(temp_buf + 1) = (unsigned char)(msg_hdr.lldpdu_hdr & 0xff);

    /*shift to  tlv information field*/
    temp_buf += LLDPDU_HLEN;

    /*set TTL to 180 seconds*/
    *temp_buf = 0;
    *(temp_buf + 1) = LLDP_TIME_TO_LIVE;

    /*return total length*/
    return (LLDPDU_HLEN + 2);
}

unsigned short append_lldpdu_end_of_lldpdu_tlv(unsigned char *pkt)
{
    lldpdu_message_header msg_hdr;
    unsigned char *temp_buf;

    temp_buf = pkt;

    msg_hdr.lldpdu_msg.tlv_type = END_OF_LLDPDU_TLV_TYPE;
    /*sting length = 0*/
    msg_hdr.lldpdu_msg.info_str_len = 0;

    *temp_buf = (unsigned char)(msg_hdr.lldpdu_hdr >> 8);
    *(temp_buf + 1) = (unsigned char)(msg_hdr.lldpdu_hdr & 0xff);

    return LLDPDU_HLEN;
}

unsigned short create_802_1_bridge_discovery_message(unsigned char *buf,
                                                     unsigned char *al_mac,
                                                     unsigned char *itf_mac)
{
    unsigned short total_len = 0;
    unsigned char *temp_buf;
    unsigned short len = 0;

    temp_buf = buf;

    len = append_lldpdu_chassis_id_tlv(temp_buf,al_mac);
    temp_buf += len;
    total_len += len;

    len = append_lldpdu_port_id_tlv(temp_buf,itf_mac);
    temp_buf += len;
    total_len += len;

    len = append_lldpdu_time_to_live_tlv(temp_buf);
    temp_buf += len;
    total_len += len;

    len = append_lldpdu_end_of_lldpdu_tlv(temp_buf);
    temp_buf += len;
    total_len += len;

    /*padding 0 if payload size less than 46 bytes*/
    if(total_len < 46)
    {
        memset(temp_buf, 0, (46 - total_len));
        total_len = 46;
    }

    return total_len;
}
