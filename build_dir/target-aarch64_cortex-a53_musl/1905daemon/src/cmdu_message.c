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
#include<stdlib.h>
#include <string.h>
#include <net/if.h>
#include <syslog.h>
#include <assert.h>
#include <linux/if_ether.h>
#include <asm/byteorder.h>
#include <pthread.h>
#include "cmdu_message.h"
#include "multi_ap.h"
#include "common.h"
#include "debug.h"
#include "_1905_lib_io.h"
#include "eloop.h"
#include "cmdu.h"
#ifdef MAP_R3
#include "map_dpp.h"
#endif

extern unsigned char p1905_multicast_address[];



extern unsigned short append_vendor_specific_type_tlv(unsigned char *pkt,
    struct p1905_vs_info *vs_info);
extern unsigned short append_link_metrics_query_type_tlv(unsigned char *pkt,
   unsigned char *target_al_mac, unsigned char type);
extern unsigned short append_push_button_event_notification_tlv(unsigned char *pkt,
   struct p1905_interface *itf_list);
extern unsigned short append_push_button_join_notification_tlv(unsigned char *pkt,
	unsigned char *al_id, unsigned short mid,
	unsigned char *local_mac, unsigned char *new_device_mac);
extern unsigned short append_link_metrics_result_code_tlv(unsigned char *pkt);


unsigned char *get_tlv_buffer(struct p1905_managerd_ctx *ctx)
{
	return ctx->tlv_buf.buf;
}


/**
 *  delete all non-1905.1 neighbor device.
 *
 * \param  non_p1905_dev  pointer of non_1905.1 database
 * \param  retun error code
 */
void delete_non_p1905_neighbor_dev_info(struct non_p1905_neighbor *non_p1905_dev)
{
    int i = 0;
    struct non_p1905_neighbor_info *dev_info, *dev_info_temp;

    for(i=0;i<ITF_NUM;i++)
    {
        if(!LIST_EMPTY(&(non_p1905_dev[i].non_p1905nbr_head)))
        {
            dev_info = LIST_FIRST(&(non_p1905_dev[i].non_p1905nbr_head));
            while(dev_info)
            {
                dev_info_temp = LIST_NEXT(dev_info, non_p1905nbr_entry);
                LIST_REMOVE(dev_info, non_p1905nbr_entry);
                free(dev_info);
                dev_info = dev_info_temp;
            }
        }
    }
}

/**
 *  create topology discovery message.
 *
 * \param  buf  pointer to cmdu message header start position.
 * \param  al_mac  pointer of local abstraction layer mac address.
 * \param  itf_mac  pointer of transmitting ibterface mac address.
 * \return length of total tlvs included in this message
 */
unsigned short create_topology_discovery_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned char *al_mac, unsigned char *itf_mac,
	unsigned short vs_len, unsigned char *vs_info)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;

    msg_hdr = (cmdu_message_header*)buf;

    /*fill into tlvs*/
    length = append_1905_al_mac_addr_type_tlv(tlv_temp_buf,al_mac);
    total_tlvs_length += length;
    tlv_temp_buf += length;

    length = append_mac_addr_type_tlv(tlv_temp_buf,itf_mac);
    total_tlvs_length += length;
    tlv_temp_buf += length;

	/*add vendor specific info if necessary*/
	if (vs_len != 0 && vs_info != NULL) {
		os_memcpy(tlv_temp_buf, vs_info, vs_len);
		total_tlvs_length += vs_len;
    	tlv_temp_buf += vs_len;
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
    msg_hdr->message_type = host_to_be16(TOPOLOGY_DISCOVERY);
    msg_hdr->relay_indicator = 0x0;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

/**
 *  create topology discovery message.
 *
 * \param  buf  pointer to cmdu message header start position.
 * \param  al_mac  pointer of local abstraction layer mac address.
 * \param  itf_mac  pointer of transmitting ibterface mac address.
 * \return length of total tlvs included in this message
 */
unsigned short create_vendor_specific_topology_discovery_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned char *al_mac, unsigned char *itf_mac,
	unsigned short vs_len, unsigned char *vs_info)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;

    msg_hdr = (cmdu_message_header*)buf;

	/*add vendor specific info if necessary*/
	if (vs_len != 0 && vs_info != NULL) {
		os_memcpy(tlv_temp_buf, vs_info, vs_len);
		total_tlvs_length += vs_len;
    	tlv_temp_buf += vs_len;
	} else {
		return 0;
	}

    /*fill into tlvs*/
    length = append_1905_al_mac_addr_type_tlv(tlv_temp_buf,al_mac);
    total_tlvs_length += length;
    tlv_temp_buf += length;

    length = append_mac_addr_type_tlv(tlv_temp_buf,itf_mac);
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
    msg_hdr->message_type = host_to_be16(VENDOR_SPECIFIC);
    msg_hdr->relay_indicator = 0x0;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

/**
 *  create topology query message.
 *
 * \param  buf  pointer to cmdu message header start position.
 * \return length of total tlvs included in this message
 */
unsigned short create_topology_query_message(unsigned char *buf,  struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;

	msg_hdr = (cmdu_message_header *)buf;
	length = append_map_version_tlv(tlv_temp_buf, ctx->map_version);
	total_tlvs_length += length;
	tlv_temp_buf += length;

#ifdef MAP_R2
	length = append_r2_cap_tlv(tlv_temp_buf, &(ctx->ap_cap_entry.ap_r2_cap));
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
    msg_hdr->message_type = host_to_be16(TOPOLOGY_QUERY);
    msg_hdr->relay_indicator = 0x0;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

/**
 *  create topology response message.
 *
 * \param  buf  pointer to cmdu message header start position.
 * \param  al_mac  pointer of local abstraction layer mac address.
 * \param  itf_list  pointer of local interfaces information.
 * \param  br_cap_list  pointer of local internal bridge information.
 * \param  p1905_dev  pointer of 1905.1 neighbor devices information.
 * \param  non_p1905_dev poniter of non-1905.1 device information
 * \param  tpgr_list  list head of received topology response database
 * \return length of total tlvs included in this message
 */
unsigned short create_topology_response_message(unsigned char *buf,
    unsigned char *al_mac, struct p1905_managerd_ctx* ctx,
    struct bridge_capabiltiy *br_cap_list, struct p1905_neighbor *p1905_dev,
    struct non_p1905_neighbor *non_p1905_dev, struct list_head_tprdb *tpgr_head,
    unsigned char service, struct list_head_oper_bss *opbss_head,
    struct list_head_assoc_clients *asscli_head, unsigned int cnt)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
    cmdu_message_header *msg_hdr;
    unsigned short length = 0;
    unsigned short total_tlvs_length =0;
    int i = 0;
    unsigned char seperate = 0;

    msg_hdr = (cmdu_message_header*)buf;

    length = append_device_info_type_tlv(tlv_temp_buf, al_mac, ctx);
    total_tlvs_length += length;
    tlv_temp_buf += length;

    length = append_device_bridge_capability_type_tlv(tlv_temp_buf,
                                                      br_cap_list);
    total_tlvs_length += length;
    tlv_temp_buf += length;

    for (i=0; i<ctx->itf_number; i++) {
       do {
	       length = append_p1905_neighbor_device_type_tlv(tlv_temp_buf, p1905_dev,
	                    i, &seperate);
	        total_tlvs_length += length;
	        tlv_temp_buf += length;
	       } while(seperate);
    }

    seperate = 0;
    /*append non-p1905.1 device tlv*/
    for(i=0; i<ctx->itf_number; i++)
    {
        do
        {
            length = append_non_p1905_neighbor_device_type_tlv(tlv_temp_buf,
                     non_p1905_dev,i, &seperate);
            total_tlvs_length += length;
            tlv_temp_buf += length;
        }while(seperate);
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

	length = append_map_version_tlv(tlv_temp_buf, ctx->map_version);
	total_tlvs_length += length;
	tlv_temp_buf += length;

#ifdef MAP_R3
	if ((ctx->map_version == (unsigned char)MAP_PROFILE_R3) &&
		(r3_autoconfig_done == ctx->r3_oboard_ctx.r3_config_state)) {
		debug(DEBUG_TRACE, "R3 autoconfig done, append bss config report tlv\n");
		length = append_bss_config_report_tlv(tlv_temp_buf, ctx);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}
#endif // MAP_R3

	reset_stored_tlv(ctx);
	store_revd_tlvs(ctx, ctx->tlv_buf.buf, total_tlvs_length);
	os_get_time(&ctx->own_topo_rsp_update_time);
	_1905_notify_topology_rsp_event(ctx, ctx->p1905_al_mac_addr, ctx->p1905_al_mac_addr);

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
    msg_hdr->message_type = host_to_be16(TOPOLOGY_RESPONSE);
    msg_hdr->relay_indicator = 0x0;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

/**
 *  create topology notification message.
 *
 * \param  buf  pointer to cmdu message header start position.
 * \param  al_mac  pointer of local abstraction layer mac address.
 * \return length of total tlvs included in this message
 */
unsigned short create_topology_notification_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned char *al_mac, struct map_client_association_event *assoc_evt,
	unsigned char notifier, unsigned short vs_len, unsigned char *vs_info)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;

    msg_hdr = (cmdu_message_header*)buf;

    length = append_1905_al_mac_addr_type_tlv(tlv_temp_buf,al_mac);
    total_tlvs_length += length;
    tlv_temp_buf += length;
	if (notifier) {
		length = append_client_assoc_event_tlv(tlv_temp_buf, assoc_evt);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}

	/*add vendor specific info if necessary*/
	if (vs_len != 0 && vs_info != NULL) {
		os_memcpy(tlv_temp_buf, vs_info, vs_len);
		total_tlvs_length += vs_len;
    	tlv_temp_buf += vs_len;
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
    msg_hdr->message_type = host_to_be16(TOPOLOGY_NOTIFICATION);
    /* topology notification is a relay multicast message,
     * so set relay_indicator
     */
    msg_hdr->relay_indicator = 0x1;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

/**
 *  create vendor specific message.
 *
 * \param  buf  pointer to cmdu message header start position.
 * \param  vs_info  vendor specific information.
 * \return length of total tlvs included in this message
 */
unsigned short create_vendor_specific_message(unsigned char *buf,
                        struct p1905_managerd_ctx *ctx)
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
    msg_hdr->message_type = host_to_be16(VENDOR_SPECIFIC);
    msg_hdr->relay_indicator = 0x0;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

/**
 *  create link metric query message.
 *
 * \param  buf  pointer to cmdu message header start position.
 * \param  target_al_mac  0: all neighbors, otherwise, mac address of target.
 * \param  type  TX_METRICS_ONLY/RX_METRICS_ONLY/BOTH_TX_AND_RX_METRICS.
 * \return length of total tlvs included in this message
 */
unsigned short create_link_metrics_query_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned char *target_al_mac, unsigned char type)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;

    msg_hdr = (cmdu_message_header*)buf;

    //fill into tlvs
    length = append_link_metrics_query_type_tlv(tlv_temp_buf, target_al_mac, type);
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
    msg_hdr->message_type = host_to_be16(LINK_METRICS_QUERY);
    msg_hdr->relay_indicator = 0x0;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

/**
 *  create link metric response message.
 *
 * \param  buf  pointer to cmdu message header start position.
 * \param  target_al_mac  0: all neighbors, otherwise, mac address of target.
 * \param  type  TX_METRICS_ONLY/RX_METRICS_ONLY/BOTH_TX_AND_RX_METRICS.
 * \param  local_al_mac  local AL mac.
 * \param  itf_list  pointer of local interfaces information.
 * \param  tpd_head  list head of topology discovery database .
 * \return length of total tlvs included in this message
 */
unsigned short create_link_metrics_response_message(unsigned char *buf,
        unsigned char *target_al_mac, unsigned char type,
        unsigned char *local_al_mac, struct p1905_interface *itf_list,
        struct p1905_neighbor *devlist, struct list_head_tpddb *tpd_head,
        struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
    cmdu_message_header *msg_hdr;
    unsigned short length = 0;
    unsigned short total_tlvs_length =0;

    msg_hdr = (cmdu_message_header*)buf;

	length = append_send_tlv(tlv_temp_buf, ctx);
	tlv_temp_buf += length;
	total_tlvs_length += length;

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
    msg_hdr->message_type = host_to_be16(LINK_METRICS_RESPONSE);
    msg_hdr->relay_indicator = 0x0;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

/**
 *  create push button event notification message.
 *
 * \param  buf  pointer to cmdu message header start position.
 * \param  plc_mac  mac address of PLC interface.
 * \param  al_mac  local AL mac address.
 * \return length of total tlvs included in this message
 */
unsigned short push_button_event_notification_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned char *al_mac, struct p1905_interface *itf_list)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;

    msg_hdr = (cmdu_message_header*)buf;

    //fill into tlvs
    length = append_1905_al_mac_addr_type_tlv(tlv_temp_buf, al_mac);
    total_tlvs_length += length;
    tlv_temp_buf += length;

    length = append_push_button_event_notification_tlv(tlv_temp_buf, itf_list);
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
    msg_hdr->message_type = host_to_be16(P1905_PB_EVENT_NOTIFY);
    /*push button event notification is a multicast relay message*/
    msg_hdr->relay_indicator = 0x1;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

/**
 *  create push button join notification message.
 *
 * \param  buf  pointer to cmdu message header start position.
 * \param  plc_mac  mac address of PLC interface.
 * \param  al_mac  local AL mac address.
 * \param  pbc_p        instance of push_button_param
 * \return length of total tlvs included in this message
 */
unsigned short push_button_join_notification_message(
	struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned char *local_mac, unsigned char *al_mac,
	push_button_param *pbc_p, unsigned char is_plc,
	unsigned char role)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;

    msg_hdr = (cmdu_message_header*)buf;

    //fill into tlvs
    length = append_1905_al_mac_addr_type_tlv(tlv_temp_buf, al_mac);
    total_tlvs_length += length;
    tlv_temp_buf += length;

    if(is_plc) {
        length = append_push_button_join_notification_tlv(tlv_temp_buf,
            pbc_p->al_id, pbc_p->mid, local_mac, pbc_p->info.new_station);
    } else {
    	if (role == CONTROLLER)
			length = append_push_button_join_notification_tlv(tlv_temp_buf,
            	pbc_p->al_id, pbc_p->mid, local_mac, pbc_p->wifi_info.new_station);
    }
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
    msg_hdr->message_type = host_to_be16(P1905_PB_JOIN_NOTIFY);
    /*push button join notification is a multicast relay message*/
    msg_hdr->relay_indicator = 0x1;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

unsigned short ap_autoconfiguration_search_message(
        unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;
	unsigned char service = ctx->service;
	unsigned char band = IEEE802_11_band_5GL;
	unsigned char radio_index = 0;

    msg_hdr = (cmdu_message_header*)buf;

    length = append_1905_al_mac_addr_type_tlv(tlv_temp_buf, ctx->p1905_al_mac_addr);
    total_tlvs_length += length;
    tlv_temp_buf += length;

    length = append_searched_role_tlv(tlv_temp_buf);
    total_tlvs_length += length;
    tlv_temp_buf += length;

	/*find controller state*/
	if (ctx->send_tlv_len != 0) {
		band = ctx->send_tlv[0];
	} else {
		if (ctx->current_autoconfig_info.radio_index == -1)
			band = os_random() % 2;
		else {
			radio_index = (unsigned char)ctx->current_autoconfig_info.radio_index;
			if (ctx->rinfo[radio_index].band & BAND_2G_CAP)
				band = IEEE802_11_band_2P4G;
			else if (ctx->rinfo[radio_index].band & BAND_5G_CAP)
				band = IEEE802_11_band_5GL;
			else if (ctx->rinfo[radio_index].band & BAND_6G_CAP)
				band = 0xFF;
		}	
	}
    length = append_autoconfig_freq_band_tlv(tlv_temp_buf, band);
    total_tlvs_length += length;
    tlv_temp_buf += length;

	length = append_supported_service_tlv(tlv_temp_buf, service);
    total_tlvs_length += length;
    tlv_temp_buf += length;

	length = append_searched_service_tlv(tlv_temp_buf);
    total_tlvs_length += length;
    tlv_temp_buf += length;

	length = append_map_version_tlv(tlv_temp_buf, ctx->map_version);
	total_tlvs_length += length;
	tlv_temp_buf += length;

#ifdef MAP_R2
	length = append_r2_cap_tlv(tlv_temp_buf, &(ctx->ap_cap_entry.ap_r2_cap));
	total_tlvs_length += length;
	tlv_temp_buf += length;
#endif

#ifdef MAP_R3
	if ((ctx->map_version == (unsigned char)MAP_PROFILE_R3) && (!ctx->r3_dpp.onboarding_type)) {
		/* only append chirp tlv while eth case in R4 spec
		** but alwasy append chirp tlv in both eth and wifi case in R3 spec
		** workaround here to make sure R3 cert can pass
		** need add map_ver=R4 in future
		*/
		if (ctx->MAP_Cer || (ctx->r3_oboard_ctx.bh_type == MAP_BH_ETH)) {
			/* include a DPP Chirp Value TLV
			** Hash Validity bit to 1 and Enrollee MAC Address Present field to 0
			*/
			struct chirp_tlv_info chirp;
			struct hash_value_item *hv_item = NULL;

			hv_item = dl_list_first(&ctx->r3_dpp.hash_value_list, struct hash_value_item, member);
			if (hv_item && hv_item->hashLen > 0) {
				chirp.enrollee_mac_address_present = 0;
				chirp.hash_validity = 1;
				chirp.hash_len = hv_item->hashLen;
				os_memcpy(chirp.hash_payload, hv_item->hashValue, hv_item->hashLen);
				length = append_dpp_chirp_value_tlv(tlv_temp_buf, &chirp);
				total_tlvs_length += length;
				tlv_temp_buf += length;
			}
		}
	}
#endif // MAP_R3

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
    msg_hdr->message_type = host_to_be16(AP_AUTOCONFIG_SEARCH);
    /*AP autoconfiguration search message is a multicast relay message*/
    msg_hdr->relay_indicator = 0x1;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

unsigned short ap_autoconfiguration_response_message(
        unsigned char *buf, unsigned char *dmac, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;
	unsigned char band = 0;
	unsigned char service = 0;
	unsigned char kibmib = 1;
	u8 map_ver = 0;
	struct agent_list_db *agent_info = NULL;

	find_agent_info(ctx, dmac, &agent_info);
	msg_hdr = (cmdu_message_header *)buf;

    length = append_supported_role_tlv(tlv_temp_buf);
    total_tlvs_length += length;
    tlv_temp_buf += length;

	band = ctx->peer_search_band;
    length = append_supported_freq_band_tlv(tlv_temp_buf, band);
    total_tlvs_length += length;
    tlv_temp_buf += length;

	length = append_supported_service_tlv(tlv_temp_buf, service);
    total_tlvs_length += length;
    tlv_temp_buf += length;

	length = append_controller_cap_tlv(tlv_temp_buf, kibmib);
	total_tlvs_length += length;
	tlv_temp_buf += length;

	if (agent_info) {
		if (agent_info->profile == MAP_PROFILE_R1 ||
			agent_info->profile == MAP_PROFILE_REVD) {
			map_ver = MAP_PROFILE_R1;
		} else if (agent_info->profile == MAP_PROFILE_R2) {
			map_ver = MAP_PROFILE_R2;
		} else if (agent_info->profile >= MAP_PROFILE_R3) {
			map_ver = MAP_PROFILE_R3;
		}
		length = append_map_version_tlv(tlv_temp_buf, map_ver);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}

#ifdef MAP_R3
	struct r3_member *peer = NULL;
	peer = get_r3_member(dmac);
	if (peer) {

		if ((ctx->map_version == (unsigned char)MAP_PROFILE_R3) && (!ctx->r3_dpp.onboarding_type)) {
	        /* Hash Validity bit to 1 and Enrollee MAC Address Present field to 0 */
	        struct chirp_tlv_info chirp;
			struct hash_value_item *hv_item = NULL, *hv_item_nxt = NULL;

			dl_list_for_each_safe(hv_item, hv_item_nxt,
				&ctx->r3_dpp.hash_value_list, struct hash_value_item, member) {
				if ((os_memcmp(hv_item->almac, peer->al_mac, ETH_ALEN) == 0)
					&& (hv_item->hashLen > 0)) {
					hex_dump_info("item hash", hv_item->hashValue, sizeof(hv_item->hashValue));
					debug(DEBUG_ERROR, "append dpp chirp value tlv\n");
					chirp.enrollee_mac_address_present = 0;
					chirp.hash_validity = 1;
					chirp.hash_len = hv_item->hashLen;
					os_memcpy(chirp.hash_payload, hv_item->hashValue, hv_item->hashLen);
					length = append_dpp_chirp_value_tlv(tlv_temp_buf, &chirp);
					total_tlvs_length += length;
					tlv_temp_buf += length;
					break;
				}
			}

	        length = append_1905_layer_security_tlv(tlv_temp_buf);
	        total_tlvs_length += length;
	        tlv_temp_buf += length;
	    }
	}
	else {
		debug(DEBUG_ERROR, "can't find r3 member by "MACSTR"\n", MAC2STR(dmac));
	}
#endif // MAP_R3

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
    msg_hdr->message_type = host_to_be16(AP_AUTOCONFIG_RESPONSE);
    msg_hdr->relay_indicator = 0x0;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

unsigned short ap_autoconfiguration_renew_message(
        unsigned char *buf, struct p1905_managerd_ctx *ctx)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;
	unsigned char band = 0;

    msg_hdr = (cmdu_message_header*)buf;

    length = append_1905_al_mac_addr_type_tlv(tlv_temp_buf, ctx->p1905_al_mac_addr);
    total_tlvs_length += length;
    tlv_temp_buf += length;

    length = append_supported_role_tlv(tlv_temp_buf);
    total_tlvs_length += length;
    tlv_temp_buf += length;
	if(ctx->send_tlv_len != 0)
	{
		band = ctx->send_tlv[0];
		debug(DEBUG_TRACE, "insert renew message for band(%s)\n", band ? "5g" : "2g");
	}

    length = append_supported_freq_band_tlv(tlv_temp_buf, band);
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
    msg_hdr->message_type = host_to_be16(AP_AUTOCONFIG_RENEW);
    msg_hdr->relay_indicator = 0x1;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

#define WSC_TLV_LENGTH 1024

struct wsc_context {
	struct p1905_managerd_ctx *ctx;
	unsigned char *pkt;
	unsigned char wsc_type;
	WSC_CONFIG config_data;
	unsigned char wfa_vendor_extension;
	unsigned short out_len;
};

void *create_m2_thread_func(void *arg)
{
	struct wsc_context *wsc_ctx = (struct wsc_context *)arg;
	wsc_ctx->out_len = append_WSC_tlv(wsc_ctx->pkt, wsc_ctx->ctx, wsc_ctx->wsc_type,
		&wsc_ctx->config_data, wsc_ctx->wfa_vendor_extension);

	return NULL;
}

int create_all_m2s(struct p1905_managerd_ctx *ctx, unsigned char *buf,
	unsigned char num, unsigned short *total_len, struct agent_radio_info *agent_radio)
{
	unsigned char i = 0, j = 0;
	int ret = 0;
	pthread_t m2_thread_id[16];
	unsigned short len = 0;
	unsigned char *m2_buf = NULL;
	WIFI_UTILS_STATUS status = wifi_utils_success;
	struct wsc_context *wsc = NULL;

	wsc = (struct wsc_context *)os_malloc(16 * sizeof(struct wsc_context));
	if (wsc == NULL)
		return -1;

	*total_len = 0;

	m2_buf = os_malloc(WSC_TLV_LENGTH * num);
	if (!m2_buf) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"failed to allocate m2 buffer on controller\n");
		goto fail;
	}

	/*before building m2, get the corresponding wsc config_data firstly*/
	for (i = 0; i < num; i++) {
		memset(&wsc[j].config_data, 0, sizeof(WSC_CONFIG));

	    status = get_wsc_config((void *)ctx, &wsc[j].config_data,
			&wsc[j].wfa_vendor_extension, agent_radio);
		if (status == wifi_utils_success)
			j++;
	}

	/*number of j represents that only j configuration data have been get successfully*/
	num = j;

	for (i = 0; i < num; i++) {
		wsc[i].ctx = ctx;
		wsc[i].pkt = m2_buf + i * WSC_TLV_LENGTH;
		wsc[i].out_len = 0;
		wsc[i].wsc_type = MESSAGE_TYPE_M2;
		ret = pthread_create(&m2_thread_id[i], NULL, create_m2_thread_func, &wsc[i]);
		if (ret < 0) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"worker task init fail\n");
		}
	}

	for (i = 0; i < num; i++) {
		if (pthread_join(m2_thread_id[i], NULL)) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"error join thread.\n");
		}
	}

	for (i = 0; i < num; i++) {
		if (wsc[i].out_len > WSC_TLV_LENGTH) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"error!!!length(%d) of one wsc message is larger than 1024 bytes.\n",
				wsc[i].out_len);
			len = 0;
			break;
		}
		os_memcpy(buf + len, wsc[i].pkt, wsc[i].out_len);
		len += wsc[i].out_len;
	}

	*total_len = len;

	os_free(m2_buf);
	os_free(wsc);
	return 1;
fail:
	os_free(wsc);
	return -1;

}

unsigned short ap_autoconfiguration_wsc_message(
        unsigned char *buf, struct p1905_managerd_ctx *ctx, unsigned char *al_mac, unsigned char wsc_type)
{
	unsigned char *tlv_temp_buf = ctx->tlv_buf.buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;
	unsigned char radio_index = 0;
	unsigned char total_bss_config_num = 0;
	struct radio_info *r = NULL;
	msg_hdr = (cmdu_message_header *)buf;

	if (ctx->role == AGENT && wsc_type == MESSAGE_TYPE_M1) {
		/*1. WSC tlv*/
		length = append_WSC_tlv(tlv_temp_buf, ctx, wsc_type, NULL, 0);
		total_tlvs_length += length;
		tlv_temp_buf += length;

		/*2. AP radio basic capa tlv*/
		if (ctx->current_autoconfig_info.radio_index != -1) {
			radio_index = (unsigned char)ctx->current_autoconfig_info.radio_index;
			r = &ctx->rinfo[radio_index];
			length = append_ap_radio_basic_capability_tlv(ctx, tlv_temp_buf, r);
			total_tlvs_length += length;
			tlv_temp_buf += length;

#ifdef MAP_R2
			if (ctx->map_version >= DEV_TYPE_R2) {
				struct ap_radio_advan_cap cap;
				os_memcpy(cap.radioid, r->identifier, ETH_ALEN);
				cap.flag = 0xc0;

				/*4. AP radio advanced tlv*/
				length = append_ap_radio_advan_tlv(tlv_temp_buf, &cap);
				total_tlvs_length += length;
				tlv_temp_buf += length;
			}
#endif
		} else {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"error radio_index == -1!!!\n");
		}
#ifdef MAP_R2
		if (ctx->map_version >= DEV_TYPE_R2) {
			/*5. Map R2 capability tlv*/
			length = append_r2_cap_tlv(tlv_temp_buf, &(ctx->ap_cap_entry.ap_r2_cap));
			total_tlvs_length += length;
			tlv_temp_buf += length;
		}
#endif
	} else if (ctx->role == CONTROLLER && wsc_type == MESSAGE_TYPE_M2) {
		struct agent_list_db *agent = NULL;
		struct agent_radio_info *agent_radio = NULL;

		/*clear bss configed bitmap to prepare to determine bss config for each M2*/
		os_memset(ctx->bss_config_bitmap, 0, sizeof(ctx->bss_config_bitmap));
		find_agent_info(ctx, al_mac, &agent);
		if (agent) {
			agent_radio = find_agent_wsc_doing_radio(agent);
		}

		if (!agent_radio) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX"!!!!!no wsc doing radio on agent"MACSTR"\n",
				PRINT_MAC(al_mac));
			return 0;
		}

		/*1. WSC tlv*/
		total_bss_config_num = agent_radio->max_bss_num < ctx->bss_config_num ?
			agent_radio->max_bss_num : ctx->bss_config_num;
		/*send tear down on this radio*/
		if (total_bss_config_num == 0)
			total_bss_config_num = 1;
		create_all_m2s(ctx, tlv_temp_buf, total_bss_config_num, &length, agent_radio);

		total_tlvs_length += length;
		tlv_temp_buf += length;

		/*2. AP radio identifier tlv*/
		length = append_ap_radio_identifier_tlv(tlv_temp_buf, agent_radio->identifier);
		total_tlvs_length += length;
		tlv_temp_buf += length;

#ifdef MAP_R2
		if (ctx->map_version >= DEV_TYPE_R2) {
			/*3. 802Q TLV*/
			length = append_default_8021Q_tlv(ctx, tlv_temp_buf, al_mac,
				ctx->bss_config_num, ctx->bss_config);
			total_tlvs_length += length;
			tlv_temp_buf += length;

			/*4. traffic separation policy TLV*/
			length = append_traffic_separation_tlv(ctx, tlv_temp_buf, al_mac,
				ctx->bss_config_num, ctx->bss_config);
			total_tlvs_length += length;
			tlv_temp_buf += length;

			/*5. vendor specific transparent vlan TLV*/
			/*decide if we need to add transparent vlan tlv*/
			if (ctx->map_policy.trans_vlan.num > 0) {
				length = append_transparent_vlan_tlv(ctx, tlv_temp_buf);
				total_tlvs_length += length;
				tlv_temp_buf += length;
			}
		}
#endif
	}

	length = append_map_version_tlv(tlv_temp_buf, ctx->map_version);
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
    msg_hdr->message_type = host_to_be16(AP_AUTOCONFIG_WSC);
    msg_hdr->relay_indicator = 0x0;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

/*this is for GPON feature*/
void detect_neighbor_existence(struct p1905_managerd_ctx *ctx,
	unsigned char *neth_almac, unsigned char *sta_mac)
{
	struct topology_discovery_db *tpg_db = NULL;
	struct neighbor_list_db *ndb = NULL;
	int i = 0, j = 0;
	unsigned char remote_almac[ETH_ALEN] = {0};
	unsigned char zero_mac[ETH_ALEN] = {0};
	unsigned char meida = 0;

	if (neth_almac) {
		os_memcpy(remote_almac, neth_almac, ETH_ALEN);
		meida = 0;
	} else if (sta_mac) {
		find_neighbor_almac_by_intf_mac(ctx, sta_mac, remote_almac);
		meida = 1;
	} else {
		debug(DEBUG_ERROR, TOPO_PREX"unhandle event\n");
		return;
	}

	if (!os_memcmp(remote_almac, zero_mac, ETH_ALEN))
		return;

	debug(DEBUG_OFF, TOPO_PREX"detect %s neighbor("MACSTR")\n",
		(meida ? "wifi" : "eth"), PRINT_MAC(remote_almac));

	LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry) {
		if (os_memcmp(tpg_db->al_mac, remote_almac, ETH_ALEN))
			continue;
		SLIST_FOREACH(ndb, &ctx->query_neighbor_head, neighbor_entry) {
			if (!os_memcmp(tpg_db->al_mac, ndb->nalmac, ETH_ALEN) &&
				!os_memcmp(tpg_db->itf_mac, ndb->nitf_mac, ETH_ALEN))
				break;
		}
		if (ndb)
			continue;
		ndb = (struct neighbor_list_db *)malloc(sizeof(struct neighbor_list_db));
		if (ndb == NULL) {
			debug(DEBUG_ERROR, TOPO_PREX"allocate neighbor_list_db fail\n");
			return;
		}
	    os_memcpy(ndb->nalmac, tpg_db->al_mac, ETH_ALEN);
		os_memcpy(ndb->nitf_mac, tpg_db->itf_mac, ETH_ALEN);
	    SLIST_INSERT_HEAD(&ctx->query_neighbor_head, ndb, neighbor_entry);
		for(i = 0; i < ctx->itf_number; i++) {
			if (os_memcmp(tpg_db->receive_itf_mac, ctx->itf[i].mac_addr, ETH_ALEN))
				continue;
			for(j = 0; j < 3; j++) {
				ctx->mid++;
				insert_cmdu_txq(p1905_multicast_address, ctx->p1905_al_mac_addr,
					e_topology_discovery_with_vendor_ie, ctx->mid, ctx->itf[i].if_name, 0);
			}
			debug(DEBUG_OFF, TOPO_PREX"tx discovery_with_vendor_ie(last_mid-(%04x) to "MACSTR" on %s",
				ctx->mid - 1, PRINT_MAC(remote_almac), ctx->itf[i].if_name);
			eloop_register_timeout(DETECT_NEIGHBOR_TIME, 0, recv_neighbor_disc_timeout,
				(void *)ctx, (void *)ndb);
			break;
		}
	}
}

void recv_neighbor_disc_timeout(void *eloop_data, void *user_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_data;
	struct neighbor_list_db *ndb = (struct neighbor_list_db *)user_ctx;
	struct topology_discovery_db *tpg_db = NULL, *tpg_db_tmp = NULL;
    struct topology_response_db *tpgr_db = NULL;
	unsigned char del_rsp_cnt = 0;

	tpg_db = LIST_FIRST(&ctx->topology_entry.tpddb_head);
	while (tpg_db) {
		tpg_db_tmp = LIST_NEXT(tpg_db, tpddb_entry);
		if (!os_memcmp(tpg_db->al_mac, ndb->nalmac, ETH_ALEN) &&
			!os_memcmp(tpg_db->itf_mac, ndb->nitf_mac, ETH_ALEN)) {
			LIST_REMOVE(tpg_db, tpddb_entry);
			debug(DEBUG_OFF, RED("del neighbor("MACSTR")"
				" itf("MACSTR")\n"),
				PRINT_MAC(tpg_db->al_mac), PRINT_MAC(tpg_db->itf_mac));
			delete_p1905_neighbor_dev_info(ctx, tpg_db->al_mac,
				(unsigned char *)tpg_db->receive_itf_mac);
			if (find_discovery_by_almac(ctx, tpg_db->al_mac) == NULL) {
				debug(DEBUG_OFF, "delete topology rsp ("MACSTR")\n",
					PRINT_MAC(tpg_db->al_mac));
				/*topo C--ETH--A1--ETH--A2
				*reboot A1; during A1 get start, A2 will get C topology discovery by ETH
				*PON with MAP logic will run and drop A1 discovery
				*Add this logic to solve the issue mentioned above
				*/
				tpgr_db = find_topology_rsp_by_almac(ctx, tpg_db->al_mac);
				if (tpgr_db) {
					if (tpgr_db->support_service == 0 || tpgr_db->support_service == 2) {
						if (ctx->conn_port.is_set) {
							ctx->conn_port.is_set = 0;
							debug(DEBUG_OFF, RED("clear conn_port=%d\n"), ctx->conn_port.port_num);
						}
					}
					delete_exist_topology_response_database(ctx, tpg_db->al_mac);
					del_rsp_cnt++;
				}
			}
			os_free(tpg_db);
		}
		tpg_db = tpg_db_tmp;
	}

	report_own_topology_rsp(ctx, ctx->p1905_al_mac_addr, ctx->br_cap,
		ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
       	ctx->service, &ctx->ap_cap_entry.oper_bss_head,
        &ctx->ap_cap_entry.assoc_clients_head, ctx->cnt);

	if (del_rsp_cnt)
		mark_valid_topo_rsp_node(ctx);

	SLIST_REMOVE(&ctx->query_neighbor_head, ndb, neighbor_list_db, neighbor_entry);
	os_free(ndb);
}

void check_neighbor_discovery(struct p1905_managerd_ctx *ctx,
	unsigned char *al_mac, unsigned char *itf_mac)
{
	struct neighbor_list_db *ndb = NULL;
	int ret = 0;

	SLIST_FOREACH(ndb, &ctx->query_neighbor_head, neighbor_entry) {
		if (!os_memcmp(ndb->nalmac, al_mac, ETH_ALEN) &&
			!os_memcmp(ndb->nitf_mac, itf_mac, ETH_ALEN)) {
			break;
		}
	}
	if (!ndb)
		return;

	ret= eloop_is_timeout_registered(recv_neighbor_disc_timeout, ctx, ndb);
	if (ret) {
		debug(DEBUG_ERROR, TOPO_PREX"[neighbor detect]receive discovery("MACSTR") "
			"peer intf("MACSTR")\n",
			PRINT_MAC(al_mac),
			PRINT_MAC(itf_mac));
		eloop_cancel_timeout(recv_neighbor_disc_timeout, ctx, ndb);
		SLIST_REMOVE(&ctx->query_neighbor_head, ndb, neighbor_list_db, neighbor_entry);
		os_free(ndb);
	}
}

