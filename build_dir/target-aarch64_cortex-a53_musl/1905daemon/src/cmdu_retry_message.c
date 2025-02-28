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
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <assert.h>
#include <linux/if_ether.h>
#include <asm/byteorder.h>
#include "cmdu_retry_message.h"
#include "cmdu_message.h"
#include "cmdu.h"
#include "debug.h"
#include "common.h"


LIST_HEAD(list_head_retry, retry_list) retry_head;


/*initialize the list head of retry message queue.*/
void init_retry_queue()
{
    LIST_INIT(&retry_head);
}

void resend_message(struct p1905_managerd_ctx *ctx, struct retry_list *list)
{
	unsigned short tlv_len = 0;
	int len = 0;
	unsigned short *mid = NULL;

	/*need update mid every time retransmit the message*/
	list->mid = ++ctx->mid;
	mid = (unsigned short *)(list->data + ETH_HLEN + 4);
	*mid = host_to_be16(list->mid);
	debug(DEBUG_TRACE, "update mid=%d\n",list->mid);

	tlv_len = list->data_len - ETH_HLEN - CMDU_HLEN;
	/*exceed the max frame size, do need to fragment TX*/
    if (tlv_len > MAX_TLVS_LENGTH) {
        if (0 > cmdu_tx_fragment(ctx, list->data, list->data_len, list->ifname)) {
            debug(DEBUG_ERROR, "fragment failed on %s (%s)", list->ifname,
                strerror(errno));
            return;
        }
    } else {
		/*send to local interface*/
        len = send(list->sock, list->data, list->data_len, 0);
        if (0 > len) {
            debug(DEBUG_ERROR, "failed on %s (%s)", list->ifname,
                strerror(errno));
            return;
        }
    }
}

void handle_retry_message_queue(struct p1905_managerd_ctx *ctx)
{
    struct retry_list *list = NULL, *list_tmp = NULL;

	list = LIST_FIRST(&retry_head);
	while (list) {
		list_tmp = LIST_NEXT(list, retry_entry);
		if (list->retry_cnt > 0) {
			/*donot receive 1905_ack after timeout, need resend the message*/
			list->retry_cnt--;
			debug(DEBUG_ERROR, "resend_message %s to %s "
				"almac="MACSTR" "
				"premid=%d retry_cnt_left=%d\n",
				get_retry_mtype_str(list->mtype),
				list->ifname,
				PRINT_MAC(list->almac),
				list->mid, list->retry_cnt);
			resend_message(ctx, list);
        } else {
			/*donot receive 1905_ack after timeout, no need resend the message*/
			LIST_REMOVE(list, retry_entry);
			os_free(list->data);
			os_free(list);
			list = NULL;
        	}
		list = list_tmp;
	}
}

void insert_retry_message_queue(unsigned char *almac,
						  unsigned short mid, unsigned short mtype,
                          unsigned char retry_cnt, int sock,
                          unsigned char *ifname,
                          unsigned char *data, unsigned short length)
{
    struct retry_list *list = NULL;

	/* every elemnet of the retry queue is unique
	  * mid is monotonous increasing for every 1905 device
	  */
    list = (struct retry_list *)malloc(sizeof(struct retry_list));
	if (!list) {
		debug(DEBUG_ERROR, "alloc list fail\n");
		return;
	}
	memset(list, 0, sizeof(struct retry_list));
	memcpy(list->almac, almac, ETH_ALEN);
    list->mid = mid;
	list->mtype = mtype;
    list->retry_cnt = retry_cnt;
	list->sock = sock;
	list->ifname = ifname;
	if (length > 0) {
    	list->data = (unsigned char*)malloc(length);
		if (!list->data) {
			debug(DEBUG_ERROR, "alloc list->data fail\n");
			free(list);
			return;
		}
	} else {
		debug(DEBUG_ERROR, "invalid length(0)\n");
		free(list);
		return;
	}
    memcpy(list->data, data, length);
	list->data_len = length;

    debug(DEBUG_TRACE, "insert retry message queue:\n");
    debug(DEBUG_TRACE, "almac = "MACSTR" mid(%04x) mtype(%s)\n",
            PRINT_MAC(almac), mid, get_retry_mtype_str(mtype));
    LIST_INSERT_HEAD(&retry_head, list, retry_entry);
}

char * get_retry_mtype_str(unsigned short mtype)
{
	static char str[64];

	(void)snprintf((char *)str, sizeof(str), "Unknown:%04x", mtype);
	switch(mtype)
	{
		case MAP_POLICY_CONFIG_REQUEST:
			return "Map Policy Config Request";
		case CHANNEL_PREFERENCE_REPORT:
			return "Channel Preference Report";
		case CHANNEL_SELECTION_RESPONSE:
			return "Channel Selection Response";
		case OPERATING_CHANNEL_REPORT:
			return "Operating Channel Report";
		case UNASSOC_STA_LINK_METRICS_QUERY:
			return "Unassoc STA Link Metrics Query";
		case UNASSOC_STA_LINK_METRICS_RESPONSE:
			return "Unassoc STA Link Metrics Response";
		case BEACON_METRICS_QUERY:
			return "Beacon Metrics Query";
		case BEACON_METRICS_RESPONSE:
			return "Beacon Metrics Response";
		case COMBINED_INFRASTRUCTURE_METRICS:
			return "Combined Infrastructure Metrics";
		case CLIENT_STEERING_REQUEST:
			return "Client Steering Request";
		case CLIENT_ASSOC_CONTROL_REQUEST:
			return "Client Assoc Control Request";
		case CLIENT_STEERING_BTM_REPORT:
			return "Client Steering BTM Report";
		case CLIENT_STEERING_COMPLETED:
			return "Client Steering Completed";
		case e_higher_layer_data:
			return "High Layer Data";
		case BACKHAUL_STEERING_REQUEST:
			return "Backhaul Steering Request";
		case BACKHAUL_STEERING_RESPONSE:
			return "Backhaul Steering Response";
		default:
			return str;
	}
}

void delete_retry_message_queue(unsigned char *almac, unsigned short mid,
	unsigned char *ifname)
{
    struct retry_list *list = NULL;

    LIST_FOREACH(list, &retry_head, retry_entry)
    {
        if (!memcmp(list->almac, almac, ETH_ALEN) && (list->mid == mid)) {
			debug(DEBUG_TRACE, "ifname(%s) list->ifname(%s)\n", ifname, list->ifname);
			LIST_REMOVE(list, retry_entry);
			debug(DEBUG_TRACE, "%s receive 1905_ack no need retry anymore\n",
				get_retry_mtype_str(list->mtype));
			free(list->data);
			free(list);
			break;
        }
    }
}


unsigned char exist_in_retry_message_list(unsigned short mtype)
{
	unsigned char exist = 0;

	switch(mtype) {
	case MAP_POLICY_CONFIG_REQUEST:
	case CHANNEL_PREFERENCE_REPORT:
	case CHANNEL_SELECTION_RESPONSE:
	case OPERATING_CHANNEL_REPORT:
	case UNASSOC_STA_LINK_METRICS_QUERY:
	case UNASSOC_STA_LINK_METRICS_RESPONSE:
	case BEACON_METRICS_QUERY:
	case BEACON_METRICS_RESPONSE:
	case COMBINED_INFRASTRUCTURE_METRICS:
	case CLIENT_STEERING_REQUEST:
	case CLIENT_ASSOC_CONTROL_REQUEST:
	case CLIENT_STEERING_BTM_REPORT:
	case CLIENT_STEERING_COMPLETED:
	case e_higher_layer_data:
	case BACKHAUL_STEERING_REQUEST:
	case BACKHAUL_STEERING_RESPONSE:
#ifdef MAP_R2
	case CHANNEL_SCAN_REQUEST:
	case CHANNEL_SCAN_REPORT:
#endif
		exist = 1;
		break;
	default:
		break;
	}

	return exist;
}

void delete_retry_message_queue_all()
{
    struct retry_list *list, *list_temp;

    list = LIST_FIRST(&retry_head);
    while(list) {
        list_temp = LIST_NEXT(list, retry_entry);

        debug(DEBUG_TRACE, "almac("MACSTR") mtype(%s), mid = %04x\n",
                PRINT_MAC(list->almac), get_retry_mtype_str(list->mtype),
                list->mid);
        LIST_REMOVE(list, retry_entry);

        free(list->data);
        free(list);
        list = list_temp;
    }
}

void uninit_retry_queue()
{
   delete_retry_message_queue_all();
}

