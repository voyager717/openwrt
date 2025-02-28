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
#include "cmdu_tlv.h"
#include "p1905_utils.h"
#include "multi_ap.h"
#include "common.h"
#include "debug.h"

extern WSC_ATTR_STATUS create_wsc_msg_M1(
    struct p1905_managerd_ctx *ctx, unsigned char *pkt, unsigned short *length);
WSC_ATTR_STATUS create_wsc_msg_M2(
    struct p1905_managerd_ctx *ctx, unsigned char *pkt, unsigned short *length,
    WSC_CONFIG *config_data, unsigned char wfa_vendor_extension);

#define MAX_NEIGHBORS_IN_ONE_TLV 100

unsigned short append_1905_al_mac_addr_type_tlv(unsigned char *pkt,
    unsigned char *al_mac)
{
    unsigned short total_length = 0;

    *pkt++ = AL_MAC_ADDR_TLV_TYPE;
    total_length += 1;

	*(unsigned short *)(pkt) = host_to_be16(AL_MAC_ADDR_TLV_LENGTH);
	pkt += 2;
    total_length += 2;

    memcpy(pkt,al_mac,ETH_ALEN);

    total_length += AL_MAC_ADDR_TLV_LENGTH;
    return total_length;
}

unsigned short append_mac_addr_type_tlv(unsigned char *pkt,
    unsigned char *itf_mac)
{
    unsigned short total_length = 0;

    *pkt++ = MAC_ADDR_TLV_TYPE;
    total_length += 1;

	*(unsigned short *)(pkt) = host_to_be16(MAC_ADDR_TLV_LENGTH);
	pkt += 2;
    total_length += 2;

    memcpy(pkt,itf_mac,ETH_ALEN);

    total_length += MAC_ADDR_TLV_LENGTH;
    return total_length;
}

unsigned short append_device_info_type_tlv(unsigned char *pkt,
    unsigned char *al_mac,struct p1905_managerd_ctx *ctx)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;
	struct p1905_interface* itf_list = ctx->itf;
    int i =0;

    temp_buf = pkt;

    *temp_buf = DEVICE_INFO_TLV_TYPE;
    temp_buf +=1;
    total_length += 1;

    /* shift to tlv value ,we will calculate the tlv value length in the end
     *  of this function
     */
    temp_buf +=2;
    total_length += 2;

    /*al mac address*/
    memcpy(temp_buf,al_mac,ETH_ALEN);
    temp_buf +=6;
    total_length += 6;

    /*amount of local p1905.1 interface */
    *temp_buf = ctx->itf_number;
    temp_buf +=1;
    total_length += 1;

    /*info of every interface*/
    for (i = 0; i < ctx->itf_number; i++)
    {
        memcpy(temp_buf,itf_list[i].mac_addr,ETH_ALEN);
        temp_buf += ETH_ALEN;
        total_length += ETH_ALEN;

        *(unsigned short *)(temp_buf) = host_to_be16(itf_list[i].media_type);
        temp_buf += 2;
        total_length += 2;

        *temp_buf = itf_list[i].vs_info_length;
        temp_buf += 1;
        total_length += 1;

        if(itf_list[i].vs_info_length > 0)
        {
#ifdef SUPPORT_WIFI
            if(itf_list[i].media_type >= IEEE802_11_GROUP &&
               itf_list[i].media_type < IEEE1901_GROUP)
            {
                /* we use 802.11n device, dot11CurrentChannelCenterFrequencyIndex1 = 0*/
				*((itf_list[i].vs_info) + 7) = itf_list[i].channel_bw;
				*((itf_list[i].vs_info) + 8) = itf_list[i].channel_freq;
                 *((itf_list[i].vs_info) + 9) = 0;
            }
#endif
            memcpy(temp_buf,itf_list[i].vs_info,itf_list[i].vs_info_length);
            temp_buf += itf_list[i].vs_info_length;
            total_length += itf_list[i].vs_info_length;
        }
    }
    /*finally, fill the total tlv value length into tlv length field*/
	*(unsigned short *)(pkt + 1) = host_to_be16(total_length - 3);

    return total_length;
}

unsigned short append_device_bridge_capability_type_tlv(unsigned char *pkt,
    struct bridge_capabiltiy *br_cap_list)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;
    int i =0;

    temp_buf = pkt;

    *temp_buf = BRIDGE_CAPABILITY_TLV_TYPE;
    temp_buf +=1;
    total_length += 1;

    /* shift to tlv value ,we will calculate the tlv value length in the end
     * ofthis function
     */
    temp_buf +=2;
    total_length += 2;

    /*amount of internal bridge*/
    *temp_buf = BRIDGE_NUM;
    temp_buf +=1;
    total_length += 1;

    for(i=0;i<BRIDGE_NUM;i++)
    {
        *temp_buf = br_cap_list[i].interface_num;
        temp_buf +=1;
        total_length += 1;

        memcpy(temp_buf, br_cap_list[i].itf_mac_list,
               (ETH_ALEN*br_cap_list[i].interface_num));

        temp_buf += ETH_ALEN * br_cap_list[i].interface_num;
        total_length += ETH_ALEN * br_cap_list[i].interface_num;
    }

    /*finally, fill the total tlv value length into tlv length field*/
	*(unsigned short *)(pkt + 1) = host_to_be16(total_length - 3);

    return total_length;
}

unsigned short append_p1905_neighbor_device_type_tlv(unsigned char *pkt,
    struct p1905_neighbor *devlist, int num, unsigned char *seperate)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;
    int i = 0;
    struct p1905_neighbor_info *info;
    unsigned char exist = 0;
    unsigned char neighbor_num = 0;
    static unsigned char record_mac[ETH_ALEN] = {0};

    temp_buf = pkt;
    i = num;

  /*firstly shift to tlv value*/
    temp_buf += 3;
    total_length += 3;

    if(!LIST_EMPTY(&(devlist[i].p1905nbr_head)))
    {
        //exist = 1;
        memcpy(temp_buf, devlist[i].local_mac_addr, ETH_ALEN);
        temp_buf += ETH_ALEN;
        total_length += ETH_ALEN;

        LIST_FOREACH(info, &(devlist[i].p1905nbr_head), p1905nbr_entry)
        {
            if(*seperate)
            {
                if(memcmp(record_mac, info->al_mac_addr, ETH_ALEN))
                {
                    continue;
                }

                /*shift to next p1905.1 neighbor device database*/
                *seperate = 0;
                continue;
            }

            exist = 1;
            memcpy(temp_buf, info->al_mac_addr, ETH_ALEN);
            temp_buf += ETH_ALEN;
            total_length += ETH_ALEN;

            if(info->ieee802_1_bridge)
                (*temp_buf) = 0x80;
            else
                (*temp_buf) = 0x0;
            temp_buf += 1;
            total_length += 1;

            /* use neighbor_num to record how many neighbors in one tlv
             * we don't want the tlv length larger than max message length
             * so we restrict the neighbor number to MAX_NEIGHBORS_IN_ONE_TLV
             * Notice:  although we can fragmet the cmdu, but cmdu is protocol
             * tlv boundary. So one tlv cannot exceed max message length.
             */
            neighbor_num += 1;
            if(neighbor_num == MAX_NEIGHBORS_IN_ONE_TLV)
            {
                *seperate = 1;
                memcpy(record_mac, info->al_mac_addr, ETH_ALEN);
                break;
            }
         }
     }

    /*if really has the p1905.1 neighbor device, append type & length */
    if(exist)
    {
        (*pkt) = P1905_NEIGHBOR_DEV_TLV_TYPE;
		*(unsigned short *)(pkt + 1) = host_to_be16(total_length - 3);
        return total_length;
    }
    else
        return 0;
}

unsigned short append_non_p1905_neighbor_device_type_tlv(unsigned char *pkt,
    struct non_p1905_neighbor *devlist, int num, unsigned char *seperate)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;
    unsigned char exist = 0;
    struct non_p1905_neighbor_info *info;
    unsigned char neighbor_num = 0;
    static unsigned char record_mac[ETH_ALEN] = {0};

    temp_buf = pkt;

    /*shift to value field*/
    temp_buf += 3;
    total_length += 3;

    /*local interface mac field*/
    memcpy(temp_buf, devlist[num].local_mac_addr, ETH_ALEN);
    temp_buf += ETH_ALEN;
    total_length += ETH_ALEN;

    /*mac address of non-1905.1 neighbor device*/
    if(!LIST_EMPTY(&(devlist[num].non_p1905nbr_head)))
    {
        LIST_FOREACH(info, &(devlist[num].non_p1905nbr_head), non_p1905nbr_entry)
        {
            if(*seperate)
            {
                if(memcmp(record_mac, info->itf_mac_addr, ETH_ALEN))
                {
                    continue;
                }

                /*shift to next non-p1905.1 neighbor device database*/
                *seperate = 0;
                memset(record_mac, 0, ETH_ALEN);
                continue;
            }

            exist =1;
            memcpy(temp_buf, info->itf_mac_addr, ETH_ALEN);
            temp_buf += ETH_ALEN;
            total_length += ETH_ALEN;

            /* use neighbor_num to record how many neighbors in one tlv
             * we don't want the tlv length larger than max message length
             * so we restrict the neighbor number to MAX_NEIGHBORS_IN_ONE_TLV
             * Notice:  although we can fragmet the cmdu, but cmdu is protocol
             * tlv boundary. So one tlv cannot exceed max message length.
             */
            neighbor_num += 1;
            if(neighbor_num == MAX_NEIGHBORS_IN_ONE_TLV)
            {
                *seperate = 1;
                memcpy(record_mac, info->itf_mac_addr, ETH_ALEN);
                break;
            }
        }
    }

    /*if really has the non-1905.1 neighbor device, append type & length */
    if(exist)
    {
        (*pkt) = NON_P1905_NEIGHBOR_DEV_TLV_TYPE;
		*(unsigned short *)(pkt + 1) = host_to_be16(total_length - 3);
        return total_length;
    }
    else
        return 0;
}

unsigned short append_end_of_tlv(unsigned char *pkt)
{
    unsigned short total_length = 0;

    *pkt++ = END_OF_TLV_TYPE;
    total_length += 1;

	*(unsigned short *)(pkt) = host_to_be16(END_OF_TLV_LENGTH);
    total_length += 2;

    return total_length;
}

unsigned short append_vendor_specific_type_tlv(unsigned char *pkt,
                                               struct p1905_vs_info *vs_info)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;

    temp_buf = pkt;
    *pkt = VENDOR_SPECIFIC_TLV_TYPE;
    total_length += 1;
    temp_buf += 1;

    /*shift to OUI field*/
    temp_buf += 2;
    total_length += 2;

    memcpy(temp_buf, OUI_SPIDCOM, OUI_LEN);
    temp_buf += OUI_LEN;
    total_length += OUI_LEN;


    /*append AL MAC*/
	/*
    memcpy(temp_buf, vs_info->al_mac, ETH_ALEN);
    temp_buf += ETH_ALEN;
    total_length += ETH_ALEN;
	*/
    /*add more vendor specific information below*/

    /*.........................................*/
    /*add more vendor specific information above*/
    *(pkt + 1) =  (unsigned char)(((total_length - 3) >> 8) & 0xff);
    *(pkt + 2) =  (unsigned char)((total_length - 3) & 0xff);

    return total_length;
}

unsigned short append_link_metrics_query_type_tlv(unsigned char *pkt,
                                                  unsigned char *target_al_mac,
                                                  unsigned char type)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;
    unsigned char query_all_neighbor[ETH_ALEN] = {0,0,0,0,0,0};


    temp_buf = pkt;

    *temp_buf = LINK_METRICS_QUERY_TLV_TYPE;
    temp_buf +=1;
    total_length += 1;

    /* The length of link metrics query is a variable.
     * It depends on target & type, so shift to payload first
     */
    temp_buf +=2;
    total_length += 2;

    /*query link metrics of ALL neighbor or specific neighbor */
    if(!memcmp(target_al_mac, query_all_neighbor, ETH_ALEN))
    {
        *temp_buf = QUERY_ALL_NEIGHBOR;
        temp_buf += 1;
        total_length += 1;
    }
    else
    {
        *temp_buf = QUERY_SPECIFIC_NEIGHBOR;
        /*specific neighbor, append the AL MAC of neighbor in following field*/
        temp_buf += 1;
        total_length += 1;

        memcpy(temp_buf, target_al_mac, ETH_ALEN);
        temp_buf += ETH_ALEN;
        total_length += ETH_ALEN;
    }

    if(type == TX_METRICS_ONLY)
        *temp_buf = TX_METRICS_ONLY;
    else if(type == RX_METRICS_ONLY)
        *temp_buf = RX_METRICS_ONLY;
    else if(type == BOTH_TX_AND_RX_METRICS)
        *temp_buf = BOTH_TX_AND_RX_METRICS;

    temp_buf += 1;
    total_length += 1;

    /*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

    return total_length;
}

unsigned short append_link_metrics_result_code_tlv(unsigned char *pkt)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;

    temp_buf = pkt;

    *temp_buf = RESULT_CODE_TLV_TYPE;
    temp_buf +=1;
    total_length += 1;

    /*calculate totoal length & fill into the length field*/
	*(unsigned short *)(temp_buf) = host_to_be16(LINK_METRIC_RESULT_CODE_LENGTH);

    temp_buf +=2;
    total_length += 2;

    /*0x00: Invalid neighbor, other value: reserved*/
    *temp_buf = 0x0;
    temp_buf +=1;
    total_length += 1;
    return total_length;
}

unsigned short append_push_button_event_notification_tlv(unsigned char *pkt,
                    struct p1905_interface *itf_list)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;
    int i = 0, launch_itf_amount = 0;

    temp_buf = pkt;

    *temp_buf = PUSH_BUTTON_EVENT_NOTIFICATION_TYPE;
    temp_buf +=1;
    total_length += 1;

    /*shift the tlvLength field firstly*/
    temp_buf +=2;
    total_length += 2;

    for(i=0;i<ITF_NUM;i++)
    {
        if(((itf_list[i].media_type >= IEEE1901_GROUP) &&
            (itf_list[i].media_type < MOCA_GROUP)))
            launch_itf_amount++;
#ifdef SUPPORT_WIFI
        else if(((itf_list[i].media_type >= IEEE802_11_GROUP) &&
            (itf_list[i].media_type < IEEE1901_GROUP)))
        {
            /*if wifi interface is AP, need to launch PBC configuration*/
            if((*((itf_list[i].vs_info) + 6)) == 0)
                launch_itf_amount++;
            #ifdef SUPPORT_WIFI_STATION
            /* if wifi interface is station and no link, need to launch
             * PBC configuration, implement in future
             */
            #endif
        }
#endif
    }
    /*fill into number of media type*/
    *temp_buf = launch_itf_amount;
    temp_buf +=1;
    total_length += 1;


    for(i=0;i<ITF_NUM;i++)
    {
#ifdef SUPPORT_WIFI
        if(((itf_list[i].media_type >= IEEE802_11_GROUP) &&
                    (itf_list[i].media_type < IEEE1901_GROUP)))
        {
            /*media type field, 2 octets*/
            *temp_buf = (itf_list[i].media_type >> 8) & 0xFF;
            *(temp_buf + 1) = itf_list[i].media_type & 0xFF;
            temp_buf += 2;
            total_length += 2;

            /*media specific info length*/
            *temp_buf = 10;
            temp_buf += 1;
            total_length += 1;

            /* we use 802.11n device, dot11CurrentChannelCenterFrequencyIndex1 = 0*/
            *((itf_list[i].vs_info) + 9) = 0;

            memcpy(temp_buf,itf_list[i].vs_info,itf_list[i].vs_info_length);
            temp_buf += itf_list[i].vs_info_length;
            total_length += itf_list[i].vs_info_length;
        }
#endif
    }

    /*calculate totoal length & fill into the length field*/
    *(pkt + 1) =  (unsigned char)(((total_length - 3) >> 8) & 0xff);
    *(pkt + 2) =  (unsigned char)((total_length - 3) & 0xff);

    return total_length;
}

unsigned short append_push_button_join_notification_tlv(unsigned char *pkt,
                unsigned char *al_id, unsigned short mid,
                unsigned char *local_mac, unsigned char *new_device_mac)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;

    temp_buf = pkt;

    *temp_buf = PUSH_BUTTON_JOIN_NOTIFICATION_TYPE;
    temp_buf +=1;

    *(temp_buf) = (unsigned char)((PUSH_BUTTON_JOIN_NOTIFICATION_LENGTH & 0xFF00) >> 8);
    *(temp_buf + 1) = (unsigned char)(PUSH_BUTTON_JOIN_NOTIFICATION_LENGTH & 0x00FF);
    temp_buf += 2;
    total_length = PUSH_BUTTON_JOIN_NOTIFICATION_LENGTH + 3;

    /*fill into al id field*/
    memcpy(temp_buf, al_id, ETH_ALEN);
    temp_buf += ETH_ALEN;

    /*fill into mid field*/
    *(temp_buf) = (unsigned char)((mid & 0xFF00) >> 8);
    *(temp_buf + 1) = (unsigned char)(mid & 0x00FF);
    temp_buf += 2;

    /*fill into local plc mac*/
    memcpy(temp_buf, local_mac, ETH_ALEN);
    temp_buf += ETH_ALEN;

    /*fill into new joined plc mac*/
    memcpy(temp_buf, new_device_mac, ETH_ALEN);
    temp_buf += ETH_ALEN;

    return total_length;
}

unsigned short append_searched_role_tlv(
        unsigned char *pkt)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;

    temp_buf = pkt;

    *temp_buf = SEARCH_ROLE_TLV_TYPE;
    temp_buf +=1;

	*(unsigned short *)(temp_buf) = host_to_be16(SEARCH_ROLE_LENGTH);
    temp_buf += 2;
    total_length = SEARCH_ROLE_LENGTH + 3;

    *temp_buf = ROLE_REGISTRAR;
    return total_length;
}

unsigned short append_autoconfig_freq_band_tlv(
        unsigned char *pkt, unsigned char band)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;

    temp_buf = pkt;

    *temp_buf = AUTO_CONFIG_FREQ_BAND_TLV_TYPE;
    temp_buf +=1;

	*(unsigned short *)(temp_buf) = host_to_be16(AUTOCONFIG_FREQ_BAND_LENGTH);
    temp_buf += 2;
    total_length = AUTOCONFIG_FREQ_BAND_LENGTH + 3;

    /*0x00: 2.4G, 0x01:5G, 0x02:60G, no 60G case now*/
    *temp_buf = band;

    return total_length;
}

unsigned short append_supported_role_tlv(
        unsigned char *pkt)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;

    temp_buf = pkt;

    *temp_buf = SUPPORT_ROLE_TLV_TYPE;
    temp_buf +=1;

	*(unsigned short *)(temp_buf) = host_to_be16(SUPPORTED_ROLE_LENGTH);
    temp_buf += 2;
    total_length = SUPPORTED_ROLE_LENGTH + 3;

    *temp_buf = ROLE_REGISTRAR;
    return total_length;
}

unsigned short append_supported_freq_band_tlv(
        unsigned char *pkt, unsigned char band)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;

    temp_buf = pkt;

    *temp_buf = SUPPORT_FREQ_BAND_TLV_TYPE;
    temp_buf +=1;

	*(unsigned short *)(temp_buf) = host_to_be16(SUPPORTED_FREQ_BAND_LENGTH);
    temp_buf += 2;
    total_length = SUPPORTED_FREQ_BAND_LENGTH + 3;

    /*0x00: 2.4G, 0x01:5G, 0x02:60G, no 60G case now*/
    *temp_buf = band;

    return total_length;
}

unsigned short append_WSC_tlv(
        unsigned char *pkt, struct p1905_managerd_ctx *ctx, unsigned char wsc_type,
        WSC_CONFIG *config_data, unsigned char wfa_vendor_extension)
{
    unsigned short total_length = 0;
    unsigned char *temp_buf;
    WSC_ATTR_STATUS status = wsc_attr_success;
    unsigned short wsc_length = 0;

    temp_buf = pkt;

    *temp_buf = WSC_TLV_TYPE;
    temp_buf +=1;
    total_length += 1;

    /*shift the tlvLength field firstly*/
    temp_buf +=2;
    total_length += 2;

	if (ctx->role == AGENT) {
    	if (wsc_type == MESSAGE_TYPE_M1)
        	status = create_wsc_msg_M1(ctx, temp_buf, &wsc_length);
	} else {
    	if (wsc_type == MESSAGE_TYPE_M2)
        	status = create_wsc_msg_M2(ctx, temp_buf, &wsc_length, config_data, wfa_vendor_extension);
	}

    if (status != wsc_attr_success) {
        debug(DEBUG_ERROR, "create WSC TLV type %d fail\n", wsc_type);
        total_length = 0;
        goto end;
    }

    total_length += wsc_length;

    /*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = host_to_be16(total_length - 3);

end:
    return total_length;
}

unsigned short append_send_tlv(unsigned char *pkt, struct p1905_managerd_ctx *ctx)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;

	temp_buf = pkt;
	total_length = ctx->send_tlv_len;
	memcpy(temp_buf, ctx->send_tlv, ctx->send_tlv_len);

	ctx->send_tlv_len = 0;

	return total_length;
}

unsigned short append_send_tlv_relayed(unsigned char *pkt, struct p1905_managerd_ctx *ctx)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;

	temp_buf = pkt;
	total_length = ctx->send_tlv_len;
	memcpy(temp_buf, ctx->send_tlv, ctx->send_tlv_len);

	return total_length;
}

