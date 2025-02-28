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

#ifndef CMDU_TLV_H
#define CMDU_TLV_H

#include "p1905_managerd.h"

/*CMDU tlv type value*/
#define END_OF_TLV_TYPE 0
#define AL_MAC_ADDR_TLV_TYPE 1
#define MAC_ADDR_TLV_TYPE 2
#define DEVICE_INFO_TLV_TYPE 3
#define BRIDGE_CAPABILITY_TLV_TYPE 4
#define NON_P1905_NEIGHBOR_DEV_TLV_TYPE 6
#define P1905_NEIGHBOR_DEV_TLV_TYPE 7
#define LINK_METRICS_QUERY_TLV_TYPE 8
#define TRANSMITTER_LINK_METRIC_TYPE 9
#define RECEIVER_LINK_METRIC_TYPE 10
#define VENDOR_SPECIFIC_TLV_TYPE 11
#define RESULT_CODE_TLV_TYPE 12
#define SEARCH_ROLE_TLV_TYPE 13
#define AUTO_CONFIG_FREQ_BAND_TLV_TYPE 14
#define SUPPORT_ROLE_TLV_TYPE 15
#define SUPPORT_FREQ_BAND_TLV_TYPE 16
#define WSC_TLV_TYPE 17
#define PUSH_BUTTON_EVENT_NOTIFICATION_TYPE 18
#define PUSH_BUTTON_JOIN_NOTIFICATION_TYPE 19
#define CONTROLLER_CAPABILITY_TYPE 0XDD

/*CMDU tlv length(fixed part)*/
#define END_OF_TLV_LENGTH 0
#define AL_MAC_ADDR_TLV_LENGTH 6
#define MAC_ADDR_TLV_LENGTH 6
#define LINK_METRIC_RESULT_CODE_LENGTH 1
#define PUSH_BUTTON_JOIN_NOTIFICATION_LENGTH 20
#define BACKHAUL_STEERING_RESPONSE_LENGTH 13

#define SEARCH_ROLE_LENGTH 1
#define AUTOCONFIG_FREQ_BAND_LENGTH 1
#define SUPPORTED_ROLE_LENGTH 1
#define SUPPORTED_FREQ_BAND_LENGTH 1

/*link metrics query target*/
#define QUERY_ALL_NEIGHBOR 0
#define QUERY_SPECIFIC_NEIGHBOR 1

/*link metrics query type*/
#define TX_METRICS_ONLY 0
#define RX_METRICS_ONLY 1
#define BOTH_TX_AND_RX_METRICS 2



/*for searched role tlv and support role tlv use*/
#define ROLE_REGISTRAR 0x00

/*for auto freq band tlv and supported freq band tlv use*/
#define IEEE802_11_band_2P4G 0x00
#define IEEE802_11_band_5G   0x01
#define IEEE802_11_band_60G  0x02

typedef enum
{
    ieee_802_3_u = IEEE802_3_GROUP,
    ieee_802_3_ab,

    ieee_802_11_b = IEEE802_11_GROUP,
    ieee_802_11_g,
    ieee_802_11_a,
    ieee_802_11_n_2_4G,
    ieee_802_11_n_5G,
    ieee_802_11_ac,
    ieee_802_11_ad,
    ieee_802_11_af,

    ieee_1901_wavelet = IEEE1901_GROUP,
    ieee_1901_FFT,

    moca_v1_1 = MOCA_GROUP,
}itftype;

unsigned short append_1905_al_mac_addr_type_tlv(unsigned char *pkt,
    unsigned char *al_mac);

unsigned short append_mac_addr_type_tlv(unsigned char *pkt,
     unsigned char *itf_mac);

unsigned short append_device_info_type_tlv(unsigned char *pkt,
    unsigned char *al_mac,struct p1905_managerd_ctx* ctx);

unsigned short append_device_bridge_capability_type_tlv(unsigned char *pkt,
    struct bridge_capabiltiy *br_cap_list);

unsigned short append_p1905_neighbor_device_type_tlv(unsigned char *pkt,
    struct p1905_neighbor *devlist,int num, unsigned char *seperate);

unsigned short append_non_p1905_neighbor_device_type_tlv(unsigned char *pkt,
    struct non_p1905_neighbor *devlist, int num, unsigned char *seperate);

unsigned short append_end_of_tlv(unsigned char *pkt);
unsigned short append_supported_role_tlv(unsigned char *pkt);
unsigned short append_supported_freq_band_tlv(unsigned char *pkt, unsigned char band);
#endif /*CMDU_TLV_H*/
