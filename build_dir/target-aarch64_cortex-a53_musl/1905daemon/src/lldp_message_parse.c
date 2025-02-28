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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <syslog.h>
#include <assert.h>
#include <linux/if_ether.h>
#include <asm/byteorder.h>
#include "lldp_message.h"
#include "p1905_managerd.h"
#include "cmdu_message.h"
#include "lldp_message_parse.h"
#include "p1905_utils.h"
#include "debug.h"
#include "common.h"

#ifndef ETH_P_LLDP
#define ETH_P_LLDP  (0x88cc)
#endif

extern const unsigned char p1905_multicast_address[6];

LIST_HEAD(list_head_lldprx, lldpdu_rx) lldpdu_head;

int init_lldp_rx_queue(void)
{
    LIST_INIT(&lldpdu_head);
    return 0;
}

int insert_lldp_rx_queue(unsigned char *receive_itf_mac,
                         unsigned char *port_id_mac, int ttl)
{
    unsigned char new_db = 1;
    struct lldpdu_rx *lldpdu;

	debug(DEBUG_TRACE, "port id mac "MACSTR"\n",
		PRINT_MAC(port_id_mac));

    if(!LIST_EMPTY(&lldpdu_head))
    {
        LIST_FOREACH(lldpdu, &lldpdu_head, lldpdu_entry)
        {
            if(!memcmp(lldpdu->port_id_mac, port_id_mac, ETH_ALEN) &&
                !memcmp(lldpdu->receiving_local_itf_mac, receive_itf_mac, ETH_ALEN))
            {
                debug(DEBUG_TRACE, "lldpdu is already existed\n");
                lldpdu->ttl = (ttl + 10);
                new_db = 0;
                break;
            }
        }
    }

    if(new_db)
    {
        lldpdu = (struct lldpdu_rx*)malloc(sizeof(struct lldpdu_rx));
        memcpy(lldpdu->port_id_mac, port_id_mac, ETH_ALEN);
        memcpy(lldpdu->receiving_local_itf_mac, receive_itf_mac, ETH_ALEN);
        lldpdu->ttl = (ttl + 10);
        LIST_INSERT_HEAD(&lldpdu_head, lldpdu, lldpdu_entry);
    }
    return 0;
}

void update_lldp_queue_ttl(int sec)
{
    struct lldpdu_rx *lldpdu, *lldpdu_temp;

    if(!LIST_EMPTY(&lldpdu_head))
    {
        lldpdu = LIST_FIRST(&lldpdu_head);
        while(lldpdu)
        {
            lldpdu_temp = LIST_NEXT(lldpdu, lldpdu_entry);
            lldpdu->ttl -= sec;
            if(lldpdu->ttl <= 0)
            {
                debug(DEBUG_TRACE, "delete lldp database\n");
                LIST_REMOVE(lldpdu, lldpdu_entry);
                free(lldpdu);
            }
            lldpdu = lldpdu_temp;
        }
    }
}

void delete_lldp_queue_all(void)
{
    struct lldpdu_rx *lldpdu, *lldpdu_temp;

    if(!LIST_EMPTY(&lldpdu_head))
    {
        lldpdu = LIST_FIRST(&lldpdu_head);
        while(lldpdu)
        {
            lldpdu_temp = LIST_NEXT(lldpdu, lldpdu_entry);
            LIST_REMOVE(lldpdu, lldpdu_entry);
            free(lldpdu);
            lldpdu = lldpdu_temp;
        }
    }
}

int find_lldp_by_port_id_mac(unsigned char *receiving_mac,
                             unsigned char *port_id_mac)
{
    struct lldpdu_rx *lldpdu;

    if(!LIST_EMPTY(&lldpdu_head))
    {
        LIST_FOREACH(lldpdu, &lldpdu_head, lldpdu_entry)
        {
            if((!memcmp(lldpdu->port_id_mac, port_id_mac, ETH_ALEN)) &&\
              (!memcmp(lldpdu->receiving_local_itf_mac, receiving_mac, ETH_ALEN)))
                return 1;//find same mac in LLDP storage, return 1
        }
    }

    /*not match in LLDP storage, return 0*/
    return 0;
}
int lldp_update_802_1_bridge_exist(struct p1905_managerd_ctx *ctx, int sock,
                             unsigned char *smac, unsigned char *chassis_mac)
{
    unsigned char local_itf_mac[ETH_ALEN];
    int i = 0;
    struct p1905_neighbor_info *dev_info;

    for (i = 0; i < ctx->itf_number; i++) {
		if (ctx->sock_lldp[i] == sock) {
    		memcpy(local_itf_mac, ctx->itf[i].mac_addr, ETH_ALEN);
			break;
		}
	}
	if (i >= ctx->itf_number) {
		 debug(DEBUG_ERROR, "local_itf_mac found fail\n");
         return -1;
	}
    debug(DEBUG_TRACE, "receive lldp by intf "MACSTR" src="MACSTR"\n",
		PRINT_MAC(local_itf_mac), PRINT_MAC(smac));

    LIST_FOREACH(dev_info, &(ctx->p1905_neighbor_dev[i].p1905nbr_head), p1905nbr_entry)
    {
        if(!memcmp(dev_info->al_mac_addr, chassis_mac, ETH_ALEN))
        {
            /* receive LLDP & topology discover message in same interface
             * and the mac addr in chassis id tlv is same as the AL mac addr
             * in local p1905.1 neighbor device information
             */
            if(dev_info->ieee802_1_bridge)
            {
                debug(DEBUG_TRACE, "ieee802.1 bridge not exist\n");
                dev_info->ieee802_1_bridge = 0;
            }
            return 0;
        }
    }
    return -1;
}

unsigned short get_lldpdu_hdr(unsigned char *buf)
{
    unsigned short hdr;

    hdr = *(buf);
    hdr = (hdr << 8) & 0xFF00;
    hdr = hdr |(*(buf+1));

    return hdr;
}

int parse_chassis_id_tlv(unsigned char *buf,unsigned char *al_mac)
{
    unsigned short msg_hdr = 0;
    unsigned char *temp_buf;
    unsigned short length = 0;
    unsigned char subtype = 0;

    temp_buf = buf;
    msg_hdr = get_lldpdu_hdr(buf);

    if(!(((msg_hdr >> 9) & 0x7f) == CHASSIS_ID_TLV_TYPE))
    {
        debug(DEBUG_ERROR, "wrong chassis id tlv type\n");
        return -1;
    }
    else
    {
        debug(DEBUG_TRACE, "receive lldp chassis id tlv\n");
    }

    length = (msg_hdr & 0x01ff);
    debug(DEBUG_TRACE, "chassis id info string len = %d\n", length);
    temp_buf += LLDPDU_HLEN;

    /*chassis id tlv subtype field*/
    subtype = *temp_buf;
    temp_buf++;

    if(subtype != C_MAC_ADDRESS)
    {
        debug(DEBUG_TRACE, "chassis id tlv subtype = %d\n",subtype);
        return 0;
    }
    memcpy(al_mac, temp_buf, ETH_ALEN);
    return (length + LLDPDU_HLEN);
}

int parse_port_id_tlv(unsigned char *buf,unsigned char *itf_mac)
{
    unsigned short msg_hdr = 0;
    unsigned char *temp_buf;
    unsigned short length = 0;
    unsigned char subtype = 0;

    temp_buf = buf;
    msg_hdr = get_lldpdu_hdr(buf);

    if(!(((msg_hdr >> 9) & 0x7f) == PORT_ID_TLV_TYPE))
    {
        debug(DEBUG_ERROR, "wrong port id tlv type\n");
        return -1;
    }
    else
    {
        debug(DEBUG_TRACE, "receive lldp port id tlv\n");
    }

    length = (msg_hdr & 0x01ff);
    debug(DEBUG_TRACE, "port id info string len = %d\n",length);
    temp_buf += LLDPDU_HLEN;

    /*port id tlv subtype field*/
    subtype = *temp_buf;
    temp_buf++;

    if(subtype != P_MAC_ADDRESS)
    {
        debug(DEBUG_TRACE, "port id tlv subtype = %d\n",subtype);
        return 0;
    }

    memcpy(itf_mac, temp_buf, ETH_ALEN);
    return (length + LLDPDU_HLEN);
}

int parse_time_to_live_tlv(unsigned char *buf,unsigned short *ttl)
{
    unsigned short msg_hdr = 0;
    unsigned char *temp_buf;
    unsigned short length = 0;
    unsigned short time = 0;

    temp_buf = buf;
    msg_hdr = get_lldpdu_hdr(buf);

    if(!(((msg_hdr >> 9) & 0x7f) == TIME_TO_LIVE_TLV_TYPE))
    {
        debug(DEBUG_ERROR, "wrong time to live tlv type\n");
        return -1;
    }
    else
    {
        debug(DEBUG_TRACE, "receive lldp time to live tlv\n");
    }

    length = (msg_hdr & 0x01ff);
    if(length != 2)
    {
        debug(DEBUG_ERROR, "time to live tlv info string len = %d\n",length);
        return -1;
    }

    temp_buf += LLDPDU_HLEN;

    time = *(temp_buf);
    time = (time << 8) & 0xFF00;
    time = time |(*(temp_buf+1));

    *ttl = time;
    return (length + LLDPDU_HLEN);
}

int parse_802_1_bridge_discovery_msg(struct p1905_managerd_ctx *ctx, int sock,
                                     unsigned char *buf)
{
    unsigned char *temp_buf;
    struct ethhdr *eth_hdr;
    unsigned char chassis_mac[ETH_HLEN];
    unsigned char port_mac[ETH_HLEN];
    unsigned short ttl = 0;
    int len;
    //unsigned char exist_in_topology = 0;
    unsigned char need_record = 1;
	int i = 0;

    temp_buf = buf;
    eth_hdr = (struct ethhdr *)temp_buf;

    if(eth_hdr->h_proto != host_to_be16(ETH_P_LLDP))
    {
        debug(DEBUG_ERROR, "wrong LLDP ether type\n");
        return -1;
    }

    /*shift to LLDPDU start position*/
    temp_buf += ETH_HLEN;
    len = parse_chassis_id_tlv(temp_buf,chassis_mac);
    if(0 > len)
    {
        debug(DEBUG_ERROR, "parse chassis id tlv fail\n");
        return -1;
    }
    else if(0 == len)
    {
        /*if we get a chassis id tlv which we don't need(subtype != mac addr),
         *ignore this tlv
         */
        debug(DEBUG_TRACE, "do not need this chassis id tlv\n");
    }
    else
    {
        if(0 == lldp_update_802_1_bridge_exist(ctx, sock, eth_hdr->h_source, chassis_mac))
        {
            /*chassic mac already exist in p1905.1 neighbor info,
             *so we can reagrd this local interface without ieee802.1 bridge.
             *update the 802.1 bridge exist field in p1905.1 neighbor info
             */
            debug(DEBUG_TRACE, "this chassis mac is produced by p1905.1 device");
        }
    }
    temp_buf += len;

    len = parse_port_id_tlv(temp_buf,port_mac);
    if(0 > len)
    {
        debug(DEBUG_ERROR, "parse port id tlv fail\n");
        return -1;
    }
    else if(0 == len)
    {
        /*if we get a port id tlv which we don't need(subtype != MAC_ADDR),
         *ignore this tlv
         */
        need_record = 0;
         debug(DEBUG_TRACE, "do not need this port id tlv\n");
    }

    temp_buf += len;

    len = parse_time_to_live_tlv(temp_buf, &ttl);
    if (0 > len) {
        debug(DEBUG_ERROR, "parse time to live tlv fail\n");
        return -1;
    }

    if (need_record) {
      for (i = 0; i < ctx->itf_number; i++) {
        if (ctx->sock_lldp[i] == sock) {
          insert_lldp_rx_queue(ctx->itf[i].mac_addr, port_mac, ttl);
          debug(DEBUG_TRACE, "recv lldp on %s\n", ctx->itf[i].if_name);
          break;
        }
      }
    }

	return 0;
}

