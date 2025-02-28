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
#include <sys/socket.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "lldp.h"
#include "cmdu.h"
#include "common.h"
#include "debug.h"
#include "eloop.h"

extern void delete_lldp_queue_all();
extern void init_lldp_rx_queue();


int lldpdu_init(struct p1905_managerd_ctx *ctx)
{
    struct ifreq ifr;
	int i = 0;

	for (i = 0; i < ctx->itf_number; i++) {
	    /*Create a receive connection of lldpdu on bridge interface*/
	    if (0 > (ctx->sock_lldp[i] = socket(AF_PACKET, SOCK_RAW, ETH_P_LLDP))) {
	        debug(DEBUG_ERROR, "cannot open lldpdu socket %s\n",
	               strerror(errno));
	        return -1;
	    }

	    /*Prepare lldpdu socket interface index*/
		(void)snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ctx->itf[i].if_name);
	    if (-1 == (ioctl(ctx->sock_lldp[i], SIOCGIFINDEX, &ifr))) {
	        debug(DEBUG_ERROR, "cannot get interface %s index (%s) for lldpdu\n",
	               ctx->itf[i].if_name, strerror(errno));
	        close(ctx->sock_lldp[i]);
	        return -1;
	    }

	    ctx->lldp_sll.sll_family = AF_PACKET;
	    ctx->lldp_sll.sll_ifindex = ifr.ifr_ifindex;
	    ctx->lldp_sll.sll_protocol = host_to_be16(ETH_P_LLDP);

	    /*Bind lldpdu socket to this interface*/
	    if (-1 == (bind(ctx->sock_lldp[i], (struct sockaddr *)&ctx->lldp_sll,
	              sizeof(struct sockaddr_ll)))) {
	        debug(DEBUG_ERROR, "cannot bind raw socket to interface %s (%s)\n",
	               ctx->itf[i].if_name, strerror(errno));
	        close(ctx->sock_lldp[i]);
	        return -1;
	    }
		 debug(DEBUG_ERROR, "create lldpdu on %s-sock(%d) success\n",
		 	ctx->itf[i].if_name, ctx->sock_lldp[i]);

		eloop_register_read_sock(ctx->sock_lldp[i], lldp_process, (void *)ctx, NULL);
	}
    /*init lldp rx queue*/
    init_lldp_rx_queue();

    return 0;
}

void lldpdu_uninit(struct p1905_managerd_ctx *ctx)
{
	int i = 0;

    delete_lldp_queue_all();
	for (i = 0; i < ctx->itf_number; i++)
   	 close(ctx->sock_lldp[i]);
}

int send_802_1_bridge_discovery_msg(struct p1905_managerd_ctx *ctx,
                    unsigned char *dmac, unsigned char *smac,
                    unsigned char *buffer, int if_index)
{
    struct ethhdr *eth_hdr;
    unsigned char *temp_buf;
    int total_len;

    temp_buf = buffer;

    eth_hdr = (struct ethhdr*)buffer;
    memcpy(eth_hdr->h_dest, dmac, ETH_ALEN);
    memcpy(eth_hdr->h_source, smac, ETH_ALEN);
    eth_hdr->h_proto = host_to_be16(ETH_P_LLDP);

    /*shift to start position of lldpdu */
    temp_buf += ETH_HLEN;

    total_len =
    create_802_1_bridge_discovery_message(temp_buf,ctx->p1905_al_mac_addr,smac);

    /*total length = lldpdu length + ether header length*/
    total_len += ETH_HLEN;

    total_len = send(ctx->sock_lldp[if_index], buffer, total_len, 0);
    if(0 > total_len)
    {
		if (errno == ENETDOWN) {
			debug(DEBUG_ERROR, "error reason(%s)\n", strerror(errno));
			return 0;
		}
        return -1;
    }

    return total_len;
}

int receive_802_1_bridge_discovery_msg(struct p1905_managerd_ctx *ctx, int sock,
                            unsigned char *buf, int len)
{
    len = recv(sock, buf, len, 0);

    if(0 >= len)
    {
        debug(DEBUG_WARN, "lldp receive failed on %d (%s)", sock,
               strerror(errno));
        return -1;
    }
    return len;
}
