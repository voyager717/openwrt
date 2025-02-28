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
#include <syslog.h>
#include <assert.h>
#include <linux/if_ether.h>
#include <asm/byteorder.h>
#include "cmdu_tlv_parse.h"
#include "cmdu_message.h"
#include "common.h"
#include "debug.h"
#include "p1905_utils.h"
#include "multi_ap.h"
#ifdef MAP_R3
#include "map_dpp.h"
#endif

unsigned char mtk_oui[3] = {0x00, 0x0C, 0xE7};

#define TOTAL_AL_MAC_ADDR_TLV_LENGTH 9
#define TOTAL_MAC_ADDR_TLV_LENGTH 9

extern WSC_ATTR_STATUS parse_wsc_msg(
    struct p1905_managerd_ctx *ctx, unsigned char *pkt, unsigned short length);

int get_tlv_len(unsigned char *buf)
{
	unsigned char *temp_buf = buf;
	unsigned short length = 0;

	temp_buf += 1; /* shift to length field */

	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	return length;
}

int get_cmdu_tlv_length(unsigned char *buf)
{
	unsigned char *temp_buf = buf;
	unsigned short length = 0;

	temp_buf += 1; /* shift to length field */

	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	return (length+3);
}

/* sanity check TLV length, if sum TLV length is not equal with buffer length, drop it*/
int cmdu_sanity_check(unsigned char *buf, int len)
{
	unsigned char *pos = buf;
	unsigned int tlv_total_len = 0;

	while (1) {
		/* end tlv */
		if (*(pos + tlv_total_len) == END_OF_TLV_TYPE) {
			tlv_total_len += get_cmdu_tlv_length(pos + tlv_total_len);
			if (tlv_total_len == len || (tlv_total_len < MIN_TLVS_LENGTH && len == MIN_TLVS_LENGTH))
				return 0;
			else
				return -1;
		}
		tlv_total_len += get_cmdu_tlv_length(pos + tlv_total_len);
		/* not found end tlv */
		if (tlv_total_len >= len)
			return -2;
	}
}

int parse_al_mac_addr_type_tlv(unsigned char *al_mac, unsigned short len, unsigned char *value)
{
	if (len != ETH_ALEN)
		return -1;

	memcpy(al_mac, value, ETH_ALEN);

	return 0;
}


int parse_mac_addr_type_tlv(unsigned char *mac, unsigned short len, unsigned char *value)
{
	if (len != 0x6)
		return -1;

	memcpy(mac, value, ETH_ALEN);

	return 0;
}

int parse_vs_tlv(struct vs_value *vs_info, unsigned short len, unsigned char *value)
{
	int left = len;
	unsigned char *temp_buf = NULL;
	unsigned short sublen = 0;
	unsigned char func_type = 0, sub_type = 0;
	unsigned char suboui[3] = {0x00, 0x0C, 0xE7};

	if (left < sizeof(mtk_oui) + 1) {
		debug(DEBUG_ERROR, "%d: left %d less than %d\n", __LINE__,
			left, (unsigned int)(sizeof(mtk_oui) + 1));
		return -1;
	}
	left -= sizeof(mtk_oui) + 1;

	temp_buf = value;

	if (os_memcmp(temp_buf, mtk_oui, 3)) {
		debug(DEBUG_ERROR, "OUI not mtk_oui; do not handle!\n");
		return 0;
	}
	temp_buf += 3;

	func_type = *temp_buf++;

	switch(func_type) {
		case 0: /*mac sub type*/
			if (left != ETH_ALEN + 2) {
				debug(DEBUG_ERROR, "error vs mac tlv(%d)!\n", len);
				return -1;
			}
			sublen = *(unsigned short *)temp_buf;
			sublen = be_to_host16(sublen);
			if (sublen != ETH_ALEN) {
				debug(DEBUG_ERROR, "error mac len!\n");
				return -1;
			}
			temp_buf += 2;
			memcpy(vs_info->itf_mac, temp_buf, ETH_ALEN);
			debug(DEBUG_TRACE, "mac("MACSTR")!\n",
				PRINT_MAC(vs_info->itf_mac));
			break;
		case 0xff:
			if (left < sizeof(suboui) + 1) {
				debug(DEBUG_ERROR, "%d: left %d less than %d\n", __LINE__,
					left, (unsigned int)(sizeof(suboui) + 1));
				return -1;
			}
			left -= sizeof(suboui) + 1;

			if (os_memcmp(suboui, temp_buf, sizeof(suboui))) {
				debug(DEBUG_ERROR, "error vendor specific tlv charactor_value\n");
				return -1;
			}
			temp_buf += sizeof(suboui);
			sub_type = *temp_buf++;

			switch (sub_type) {
				case internal_vendor_discovery_message_subtype:
					if (left != 0) {
						debug(DEBUG_ERROR, "error vs discovery tlv(%d)!\n", len);
						return -1;
					}
					vs_info->discovery = 1;
					break;
				case internal_vendor_notification_message_subtype:
					if (left != ETH_ALEN + 2) {
						debug(DEBUG_ERROR, "error vs notification tlv(%d)!\n", len);
						return -1;
					}
					sublen = *(unsigned short *)temp_buf;
					sublen = be_to_host16(sublen);
					temp_buf += 2;
					if (sublen != ETH_ALEN) {
						debug(DEBUG_ERROR, "error almac len!\n");
						return -1;
					}
					os_memcpy(vs_info->neth_almac, temp_buf, ETH_ALEN);
					break;
#ifdef MAP_R2
				case transparent_vlan_message_subtype:
				{
					unsigned char i = 0, num = 0;

					if (!vs_info->parse_ts)
						break;
					if (left < 1) {
						debug(DEBUG_ERROR, "%d: left %d less than %d\n", __LINE__, left, 1);
						return -1;
					}
					left--;

					num = *temp_buf++;
					if (num > MAX_NUM_TRANSPARENT_VID) {
						debug(DEBUG_TRACE, "num(%d) is larger than MAX_NUM_TRANSPARENT_VID\n", num);
						return -1;
					}

					if (left < 2 * num) {
						debug(DEBUG_ERROR, "%d: left %d less than %d\n", __LINE__, left, 2 * num);
						return -1;
					}
					left -= 2 * num;

					vs_info->trans_vlan.num = num;
					debug(DEBUG_ERROR, "parse transparent vlan number=%d\n", num);
					for (i = 0; i < num; i++) {
						vs_info->trans_vlan.transparent_vids[i] =
							be_to_host16(*((unsigned short *)temp_buf));
						temp_buf += 2;
						debug(DEBUG_ERROR, "parse transparent vlan id(%d)\n",
							vs_info->trans_vlan.transparent_vids[i]);
					}
					vs_info->trans_vlan.updated = 1;
				}
				break;
#endif
				default:
					break;
			}
			break;
		default:
			break;
	}

	return 0;
}


int parse_device_info_type_tlv(struct p1905_managerd_ctx *ctx,
				struct list_head_devinfo *devinfo_head, unsigned short len, unsigned char *value)
{
	unsigned char *temp_buf = value;
	int left = len;
	unsigned char itfs_num = 0;
	unsigned char vs_info_len = 0;
	int i = 0;
	struct device_info_db *dev;
	unsigned char mac[ETH_ALEN];
	unsigned char almac[ETH_ALEN] = {0};
	unsigned char topo_bss_num = 0;
#ifdef MAP_R3
	unsigned char all_zero_mac[ETH_ALEN] = {0};
	struct bss_connector_item *bc_item = NULL, *bc_item_nxt = NULL;
	struct hash_value_item *hv_item = NULL, *hv_item_nxt = NULL;
#endif

	if (left < 7)
		return -1;
	left -= 7;

	/* skip AL MAC address field */
	memcpy(almac, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	/* get the amount of local interface */
	itfs_num = *temp_buf;
	temp_buf += 1;

	for (i = 0; i < itfs_num; i++) {
		if (left < 9) {
			debug(DEBUG_ERROR, "%d:error left %d less than %d\n", __LINE__, left, 9);
			return -1;
		}
		left -= 9;

		memcpy(mac, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;

#ifdef MAP_R3
		if (ctx->map_version == MAP_PROFILE_R3 && ctx->role == CONTROLLER) {
			dl_list_for_each_safe(bc_item, bc_item_nxt,
				&ctx->r3_dpp.bss_connector_list, struct bss_connector_item, member) {
				if (os_memcmp(bc_item->almac, all_zero_mac, ETH_ALEN))
					continue;
				if (!os_memcmp(bc_item->mac_apcli, mac, ETH_ALEN)) {
					debug(DEBUG_ERROR, "update almac "MACSTR" for bss connector\n", MAC2STR(almac));
					os_memcpy(bc_item->almac, almac, ETH_ALEN);

					map_dpp_save_bss_connector(bc_item);
					break;
				}
			}

			dl_list_for_each_safe(hv_item, hv_item_nxt,
				&ctx->r3_dpp.hash_value_list, struct hash_value_item, member) {
				if (os_memcmp(hv_item->almac, all_zero_mac, ETH_ALEN))
					continue;
				if (!os_memcmp(hv_item->mac_apcli, mac, ETH_ALEN)) {
					debug(DEBUG_ERROR, "update almac "MACSTR" for hash value\n", MAC2STR(almac));
					os_memcpy(hv_item->almac, almac, ETH_ALEN);
					break;
				}
			}
		}
#endif
		vs_info_len = *(temp_buf + 2);
		if (left < vs_info_len) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n", __LINE__, left, vs_info_len);
			return -1;
		}
		left -= vs_info_len;

		dev = (struct device_info_db *)malloc(sizeof(struct device_info_db));
		if (dev == NULL)
			return -1;
		memcpy(dev->mac_addr, mac, ETH_ALEN);
		/*init the list head of p1905.1 neighbor device list*/
		SLIST_INIT(&(dev->p1905_nbrdb_head));
		/*init the list head of non-p1905.1 neighbor device list*/
		SLIST_INIT(&(dev->non_p1905_nbrdb_head));
		SLIST_INSERT_HEAD(devinfo_head, dev, devinfo_entry);

		dev->media_type = (*(unsigned short *)temp_buf);
		dev->media_type = host_to_be16(dev->media_type);
		temp_buf += 2;

		dev->vs_info_len = vs_info_len;
		temp_buf += 1;

		if (vs_info_len > 0) {
			dev->vs_info = (unsigned char *)malloc(vs_info_len);
			if (dev->vs_info == NULL)
				return -1;
			memcpy(dev->vs_info, temp_buf, vs_info_len);
			temp_buf += vs_info_len;

			if (vs_info_len == 10 && (*(dev->vs_info + 6) == 0x00))
				topo_bss_num++;
		}
	}

	return 0;
}

int parse_bridge_capability_type_tlv(struct list_head_brcap *brcap_head,
					unsigned short len, unsigned char *value)
{
	unsigned char *temp_buf = value;
	int left = len;
	unsigned char br_num = 0;
	int i = 0;
	unsigned char br_itfs_num = 0;
	struct device_bridge_capability_db *br;

	if (left < 1)
		return -1;
	left--;

	/* the field of total number of bridge tuple */
	br_num = *temp_buf;
	temp_buf++;

	/* loop br_num times to store the bridge tuple information */
	for (i = 0; i < br_num; i++) {
		if (left < 1) {
			debug(DEBUG_ERROR, "%d: left %d less than %d\n", __LINE__, left, 1);
			return -1;
		}
		left--;

		br_itfs_num = *temp_buf;
		temp_buf++;

		br = (struct device_bridge_capability_db *)malloc(sizeof(struct device_bridge_capability_db));
		if (br == NULL)
			return -1;
		br->interface_amount = br_itfs_num;

		if (br_itfs_num > 0) {
			if (left < br_itfs_num * ETH_ALEN) {
				debug(DEBUG_ERROR, "%d: left %d less than %d\n",
					__LINE__, left, br_itfs_num * ETH_ALEN);
				free(br);
				return -1;
			}
			left -= br_itfs_num * ETH_ALEN;

			br->interface_mac_tuple = (unsigned char *)malloc(br_itfs_num * ETH_ALEN);
			if (br->interface_mac_tuple == NULL) {
				free(br);
				return -1;
			}
			memcpy(br->interface_mac_tuple, temp_buf, (br_itfs_num * ETH_ALEN));
		}

		LIST_INSERT_HEAD(brcap_head, br, brcap_entry);

		temp_buf += br_itfs_num * ETH_ALEN;
	}

	return 0;
}

int parse_p1905_neighbor_device_type_tlv(struct list_head_devinfo *devinfo_head,
						unsigned short len, unsigned char *value)
{
	unsigned char *temp_buf = value;
	int left = len;
	struct p1905_neighbor_device_db *dev;
	struct device_info_db *dev_info;
	unsigned char local_mac[ETH_ALEN];
	unsigned char al_mac[ETH_ALEN];
	unsigned char exist = 0;
	unsigned char new_db = 1;
	int tuple_num = 0;
	int i = 0;

	if (left < ETH_ALEN)
		return -1;
	left -= ETH_ALEN;

	memcpy(local_mac, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	/* tlv value has one local mac address field(6 bytes) and several
	* (al mac address field(6 bytes) + 802.1 exist field(1 byte)),
	* so tlv (value length - 6 ) % 7 must be zero
	*/
	if ((left % 7) == 0)
		tuple_num = left / 7;
	else
		return -1;

	SLIST_FOREACH(dev_info, devinfo_head, devinfo_entry) {
		if (!memcmp(local_mac, dev_info->mac_addr, ETH_ALEN)) {
			exist = 1;
			break;
		}
	}

	if (!exist)
		return 0;

	if (left < 7 * tuple_num) {
		debug(DEBUG_ERROR, "%d: left %d less than %d\n", __LINE__, left, 7 * tuple_num);
		return -1;
	}
	left -= 7 * tuple_num;

	for (i = 0; i < tuple_num; i++) {
		memcpy(al_mac, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;

		new_db = 1;

		/* maybe we don't needt to do this check because the old topology
		 * response db will be deleted when we get a new topology response
		 * message.
		 */
		SLIST_FOREACH(dev, &(dev_info->p1905_nbrdb_head), p1905_nbrdb_entry) {
			if (!memcmp(al_mac, dev->p1905_neighbor_al_mac, ETH_ALEN)) {
				new_db = 0;
				break;
			}
		}

		if (new_db) {
			dev = (struct p1905_neighbor_device_db *)malloc(sizeof(struct p1905_neighbor_device_db));
			if (dev == NULL)
				return -1;
			memcpy(dev->p1905_neighbor_al_mac, al_mac, ETH_ALEN);
			SLIST_INSERT_HEAD(&(dev_info->p1905_nbrdb_head), dev, p1905_nbrdb_entry);
		}

		if (((*temp_buf) & 0x80) == 0x80)
			dev->ieee_802_1_bridge_exist = 1;
		else
			dev->ieee_802_1_bridge_exist = 0;

		temp_buf += 1;
	}
	return 0;
}

int parse_non_p1905_neighbor_device_type_tlv(struct list_head_devinfo *devinfo_head,
						unsigned short len, unsigned char *value)
{
	unsigned char *temp_buf = value;
	int left = len;
	int tuple_num = 0;
	unsigned char local_mac[ETH_ALEN];
	unsigned char neighbor_mac[ETH_ALEN];
	struct device_info_db *dev_info;
	struct non_p1905_neighbor_device_list_db *dev;

	unsigned char exist = 0;
	unsigned char new_db = 1;
	int i = 0;

	if (left < ETH_ALEN)
		return -1;
	left -= ETH_ALEN;

	/*get local interface field*/
	memcpy(local_mac, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	/*calculate how many non-1905.1 device*/
	if ((left % 6) == 0)
		tuple_num = left / 6;
	else
		return -1;

	SLIST_FOREACH(dev_info, devinfo_head, devinfo_entry) {
		if (!memcmp(local_mac, dev_info->mac_addr, ETH_ALEN)) {
			exist = 1;
			break;
		}
	}

	if (!exist)
		return 0;

	if (left < tuple_num * ETH_ALEN) {
		debug(DEBUG_ERROR, "%d: left %d less than %d\n", __LINE__, left, tuple_num * ETH_ALEN);
		return -1;
	}
	left -= tuple_num * ETH_ALEN;

	for (i = 0; i < tuple_num; i++) {
		memcpy(neighbor_mac, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;

		new_db = 1;
		/* maybe we don't need to do this check because the old topology
		* response db will be deleted when we get a new topology response
		* message.
		*/
		SLIST_FOREACH(dev, &(dev_info->non_p1905_nbrdb_head), non_p1905_nbrdb_entry) {
			if (!memcmp(neighbor_mac, dev->non_p1905_device_interface_mac, ETH_ALEN)) {
				new_db = 0;
				break;
			}
		}
		if (new_db) {
			dev = (struct non_p1905_neighbor_device_list_db *)
				malloc(sizeof(struct non_p1905_neighbor_device_list_db));
			if (dev == NULL)
				return -1;
			memcpy(dev->non_p1905_device_interface_mac, neighbor_mac, ETH_ALEN);
			SLIST_INSERT_HEAD(&(dev_info->non_p1905_nbrdb_head), dev, non_p1905_nbrdb_entry);
		}
	}
	return 0;
}

int parse_link_metric_query_type_tlv(unsigned char *target, unsigned char *type, unsigned short len, unsigned char *value)
{
	unsigned char *temp_buf = value;

	/* set target mac = 0 if query all neighbors */
	if (*temp_buf == QUERY_ALL_NEIGHBOR) {
		memset(target, 0, ETH_ALEN);
		temp_buf += 1;

		/* add for IOT issue with BRCM */
		if (len == 8)
			temp_buf += ETH_ALEN;
	} else if (*temp_buf == QUERY_SPECIFIC_NEIGHBOR) {
		temp_buf += 1;
		memcpy(target, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;
	} else {
		debug(DEBUG_ERROR, "reserve valus %d\n", *temp_buf);
		return -1;
	}

	if (*temp_buf == TX_METRICS_ONLY)
		*type = TX_METRICS_ONLY;
	else if (*temp_buf == RX_METRICS_ONLY)
		*type = RX_METRICS_ONLY;
	else if (*temp_buf == BOTH_TX_AND_RX_METRICS)
		*type = BOTH_TX_AND_RX_METRICS;

	return 0;
}

#ifdef SUPPORT_ALME
int parse_transmitter_link_metrics_type_tlv(unsigned char *buf,
            unsigned char *local_al_mac, ALME_GET_METRIC_RSP *metric_rsp)
{
    unsigned char *temp_buf;
    int length = 0;
    unsigned char response_al_mac[6];
    unsigned short link_tuple_num = 0;
    int i = 0;
    unsigned int value;


    temp_buf = buf;

    if((*temp_buf) == TRANSMITTER_LINK_METRIC_TYPE)
        temp_buf++;
    else
    {
        return -1;
    }

    /*calculate tlv length*/
    length = (*temp_buf);
    length = (length << 8) & 0xFF00;
    length = length |(*(temp_buf+1));
    /*shift to tlv value field*/
    temp_buf+=2;

    /*transmit device's al mac field*/
    memcpy(response_al_mac, temp_buf, 6);
    temp_buf += 6;

    /*shift to neighbor al mac field*/
    /*if the neighbor al mac is not local, return fail*/
    if(memcmp(local_al_mac, temp_buf, 6))
    {
        debug(DEBUG_ERROR, "got a wrong target tx link metrics tlv\n");
        return -1;
    }
    temp_buf += 6;

    if(((length - 12) % 29) != 0)
    {
        debug(DEBUG_ERROR, "got a wrong length tx link metrics tlv\n");
        return -1;
    }

    link_tuple_num = (length - 12) /29;

    if(link_tuple_num > MAX_1905_LINK_NUM)
    {
        debug(DEBUG_ERROR, "got a wrong link num tx link metrics tlv\n");
        return -1;
    }

    metric_rsp->descriptor_num = link_tuple_num;
    for(i=0;i<link_tuple_num;i++)
    {
        /*fill into neighbor al mac field first*/
        memcpy(metric_rsp->descriptorlist[i].neighbor_al_mac, response_al_mac, 6);

        /*shift to interface mac addr of neighbor field*/
        temp_buf += 6;
        /*fill into local interface field in alme*/
        memcpy(metric_rsp->descriptorlist[i].local_itf_mac, temp_buf, 6);
        temp_buf += 6;

        /*intf type*/
        value = *(temp_buf);
        value = ((value << 8) & 0x0000FF00)| (*(temp_buf + 1));
        metric_rsp->descriptorlist[i].link_metrics.itf_type = (unsigned short)value;
        temp_buf += 2;

        /*ieee802.1 bridge*/
        metric_rsp->descriptorlist[i].ieee802_1_bridge = *(temp_buf);
        temp_buf += 1;

        /*packet error*/
        value = *(temp_buf);
        value = ((value << 8) & 0x0000FF00)| (*(temp_buf + 1));
        value = ((value << 8) & 0x00FFFF00)| (*(temp_buf + 2));
        value = ((value << 8) & 0xFFFFFF00)| (*(temp_buf + 3));
        metric_rsp->descriptorlist[i].link_metrics.tx_packet_error = value;
        temp_buf += 4;

        /*transmitted packets*/
        value = *(temp_buf);
        value = ((value << 8) & 0x0000FF00)| (*(temp_buf + 1));
        value = ((value << 8) & 0x00FFFF00)| (*(temp_buf + 2));
        value = ((value << 8) & 0xFFFFFF00)| (*(temp_buf + 3));
        metric_rsp->descriptorlist[i].link_metrics.tx_total_packet = value;
        temp_buf += 4;

        /*mac throghput capacity*/
        value = *(temp_buf);
        value = ((value << 8) & 0x0000FF00)| (*(temp_buf + 1));
        metric_rsp->descriptorlist[i].link_metrics.max_throughput_capacity = (unsigned short)value;
        temp_buf += 2;

        /*link availability*/
        value = *(temp_buf);
        value = ((value << 8) & 0x0000FF00)| (*(temp_buf + 1));
        metric_rsp->descriptorlist[i].link_metrics.link_availability = (unsigned short)value;
        temp_buf += 2;

        /*phy rate*/
        value = *(temp_buf);
        value = ((value << 8) & 0x0000FF00)| (*(temp_buf + 1));
        metric_rsp->descriptorlist[i].link_metrics.phy_rate = (unsigned short)value;
        temp_buf += 2;
    }

    return (length+3);
}

int parse_receiver_link_metrics_type_tlv(unsigned char *buf,
                unsigned char *local_al_mac, ALME_GET_METRIC_RSP *metric_rsp)
{
    unsigned char *temp_buf;
    int length = 0;
    unsigned char response_al_mac[6];
    unsigned short link_tuple_num = 0;
    int i = 0, j = 0;
    unsigned int value;

    temp_buf = buf;

    if((*temp_buf) == RECEIVER_LINK_METRIC_TYPE)
        temp_buf++;
    else
    {
        return -1;
    }

    /*calculate tlv length*/
    length = (*temp_buf);
    length = (length << 8) & 0xFF00;
    length = length |(*(temp_buf+1));
    /*shift to tlv value field*/
    temp_buf+=2;

    /*transmit device's al mac field*/
    memcpy(response_al_mac, temp_buf, 6);
    temp_buf += 6;

    /*shift to neighbor al mac field*/
    /*if the neighbor al mac is not local, return fail*/
    if(memcmp(local_al_mac, temp_buf, 6))
    {
        debug(DEBUG_ERROR, "got a wrong target rx link metrics tlv\n");
        return -1;
    }
    temp_buf += 6;

    if(((length - 12) % 23) != 0)
    {
        debug(DEBUG_ERROR, "got a wrong length rx link metrics tlv\n");
        return -1;
    }

    link_tuple_num = (length - 12) /23;

    /*compare link tuple num with tx and max link number*/
    if((link_tuple_num > MAX_1905_LINK_NUM) || (link_tuple_num != metric_rsp->descriptor_num))
    {
        debug(DEBUG_ERROR, "got a wrong link num rx link metrics tlv %d\n",link_tuple_num);
        return -1;
    }

    for(i=0;i<link_tuple_num;i++)
    {
        /*shift to interface mac addr of neighbor field*/
        temp_buf += 6;
        /*get correct position in metric_rsp*/
        for(j=0;j<link_tuple_num;j++)
        {
            if(!memcmp(temp_buf, metric_rsp->descriptorlist[j].local_itf_mac, 6))
                break;
        }
        if(j == link_tuple_num)
        {
            debug(DEBUG_ERROR, "got un-matched neighbor interface in rx link metric tlv\n");
            return -1;
        }
        temp_buf += 6;
        /*ignore iftype field*/
        temp_buf += 2;
        /*packet error*/
        value = *(temp_buf);
        value = ((value << 8) & 0x0000FF00)| (*(temp_buf + 1));
        value = ((value << 8) & 0x00FFFF00)| (*(temp_buf + 2));
        value = ((value << 8) & 0xFFFFFF00)| (*(temp_buf + 3));
        metric_rsp->descriptorlist[j].link_metrics.rx_packet_error = value;
        temp_buf += 4;
        /*total packet*/
        value = *(temp_buf);
        value = ((value << 8) & 0x0000FF00)| (*(temp_buf + 1));
        value = ((value << 8) & 0x00FFFF00)| (*(temp_buf + 2));
        value = ((value << 8) & 0xFFFFFF00)| (*(temp_buf + 3));
        metric_rsp->descriptorlist[j].link_metrics.rx_total_packet = value;
        temp_buf += 4;
        /*RSSI*/
        metric_rsp->descriptorlist[i].link_metrics.rssi = *(temp_buf);
        temp_buf += 1;
    }

    return (length+3);
}

int parse_link_metrics_result_code_type_tlv(unsigned char *buf)
{
    unsigned char *temp_buf;
    int length = 0;

    temp_buf = buf;
    /*shift to length field*/
    temp_buf++;

    /*calculate tlv length*/
    length = (*temp_buf);
    length = (length << 8) & 0xFF00;
    length = length |(*(temp_buf+1));
    /*shift to tlv value field*/
    temp_buf+=2;

    /*get result of invalid neighbor*/
    if((*temp_buf) == 0)
        return -1;

    return (length + 3);
}
#endif

int parse_push_button_event_notification_tlv(unsigned char *buf,
            iee802_11_info *wifi_info)
{
    unsigned char *temp_buf;
    int length = 0, i = 0;
    unsigned char num_media = 0;
    unsigned short media_type;
    unsigned char media_info_len = 0;

    temp_buf = buf;
    /*shift to length field*/
    temp_buf++;

    /*calculate tlv length*/
    length = (*temp_buf);
    length = (length << 8) & 0xFF00;
    length = length |(*(temp_buf+1));
    /*shift to tlv value field*/
    temp_buf+=2;

    num_media = *temp_buf;
    temp_buf++;

    if(num_media > 0)
    {
        for(i=0;i<num_media;i++)
        {
            media_type = *temp_buf;
            media_type = ((media_type << 8) & 0xFF00)| (*(temp_buf + 1));
            temp_buf += 2;

            debug(DEBUG_TRACE, "media_type in pb event notify = 0x%x\n",media_type);
            media_info_len = *temp_buf;
            temp_buf += 1;
            /* AP is a registrar, and the received push button event notification
             * carried the wifi media information, do not launch PBC sequece
             * on wifi AP
             */
            if((media_type >= IEEE802_11_GROUP) && (media_type < IEEE1901_GROUP))
            {
                wifi_info->no_need_start_pbc = 1;
            }
            temp_buf += media_info_len;
        }
    }

    return (length + 3);
}

int parse_search_role_tlv(unsigned short len, unsigned char *value)
{
	if (len != SEARCH_ROLE_LENGTH)
		return -1;

	if ((*value) != ROLE_REGISTRAR)
		return -1;

	return 0;
}

int parse_auto_config_freq_band_tlv(unsigned char *band, unsigned short len, unsigned char *value)
{
	if (len != AUTOCONFIG_FREQ_BAND_LENGTH)
		return -1;

	*band = *value;

	return 0;
}

int parse_supported_role_tlv(unsigned short len, unsigned char *value)
{
	if (len != SUPPORTED_ROLE_LENGTH)
		return -1;

	if ((*value) != ROLE_REGISTRAR)
		return -1;
	debug(DEBUG_TRACE, "support ROLE_REGISTRAR\n");

	return 0;
}

int parse_supported_freq_band_tlv(unsigned char *band, unsigned short len, unsigned char *value)
{
	if (len != SUPPORTED_FREQ_BAND_LENGTH)
		return -1;

	*band = *value;
	debug(DEBUG_TRACE, "support band %s\n", (*band) ? "5g":"2g");

	return 0;
}

int parse_wsc_tlv(struct p1905_managerd_ctx *ctx, unsigned short len, unsigned char *value)
{
	if (wsc_attr_success != parse_wsc_msg(ctx, value, len))
		return -1;

	return 0;
}
