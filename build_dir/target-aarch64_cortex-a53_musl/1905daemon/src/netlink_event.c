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
#include <linux/netlink.h>
#include <linux/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "os.h"
#include "debug.h"
#include "netlink_event.h"
#include "ethernet_layer.h"

int netlink_sock = 0;
int netlink_pid = 0;

int netlink_init(unsigned int pid)
{
    int sock;
    struct sockaddr_nl addr;
    int group = PORT_STATUS_GROUP;

    sock = socket(AF_NETLINK, SOCK_RAW, PORT_STATUS_NOTIFIER_PROTO);
    if (sock < 0) {
        debug(DEBUG_ERROR, "sock < 0.\n");
        return -1;
    }

    memset((void *) &addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = pid;
	addr.nl_groups = PORT_STATUS_GROUP;
    /* This doesn't work for some reason. See the setsockopt() below. */
    /* addr.nl_groups = MYMGRP; */

    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        debug(DEBUG_ERROR, "bind < 0.\n");
	    close(sock);
        return -1;
    }

    /*
     * 270 is SOL_NETLINK. See
     * http://lxr.free-electrons.com/source/include/linux/socket.h?v=4.1#L314
     * and
     * http://stackoverflow.com/questions/17732044/
     */
    if (setsockopt(sock, 270, NETLINK_ADD_MEMBERSHIP, &group, sizeof(group)) < 0) {
        debug(DEBUG_ERROR, "setsockopt < 0\n");
		close(sock);
        return -1;
    }
	netlink_sock = sock;
	netlink_pid = pid;
    return sock;
}

int netlink_deinit(int sock)
{
	close(sock);
	
	return 0;
}

int netlink_event_recv(int sock, unsigned char *buf, int len)
{
	struct sockaddr_nl nladdr;
	struct msghdr msg = { 0 };
	struct iovec iov;
	struct nlmsghdr *nlhdr = NULL;
	int recv_len = 0;
	int ret;

	iov.iov_base = (void *) buf;
	iov.iov_len = len;

	msg.msg_name = (void *) &(nladdr);
	msg.msg_namelen = sizeof(nladdr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	ret = recvmsg(sock, &msg, 0);
	if (ret < 0) {
		debug(DEBUG_ERROR, "ret < 0.\n");
		return -1;
	}
	nlhdr = (struct nlmsghdr*)buf;
	recv_len = nlhdr->nlmsg_len;
	
	return recv_len;
}

unsigned char *get_netlink_data(unsigned char *raw, int *len)
{	
	struct nlmsghdr *nlhdr = NULL;
	
//	hex_dump_all("raw netlink message", raw, *len);

	nlhdr = (struct nlmsghdr*)raw;
	*len = nlhdr->nlmsg_len - NLMSG_SPACE(0);
	
	return (unsigned char*)NLMSG_DATA(nlhdr);
}

int netlink_msg_send(const unsigned char *message, int len, unsigned int dst_group)
{
	struct nlmsghdr *nlh = NULL;
	struct sockaddr_nl dest_addr;
	struct iovec iov;
	struct msghdr msg;

	if(!message ) {
		return -1;
	}

	//create message
	nlh = (struct nlmsghdr *)os_malloc(NLMSG_SPACE(len));
	if (!nlh) {
		debug(DEBUG_ERROR, "allocate nlmsghdr fail\n");
		return -1;
	}
	nlh->nlmsg_len = NLMSG_SPACE(len);
	nlh->nlmsg_pid = netlink_pid;
	nlh->nlmsg_flags = 0;
	os_memcpy(NLMSG_DATA(nlh), message, len);

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	os_memset(&dest_addr, 0, sizeof(struct sockaddr_nl));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;
	dest_addr.nl_groups = dst_group;

	os_memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(struct sockaddr_nl);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	//send message
	if (sendmsg(netlink_sock, &msg, 0) < 0) {
		debug(DEBUG_ERROR, "netlink msg send fail\n");
		os_free(nlh);
		return -3;
	}

	os_free(nlh);
	return 0;
}

int netlink_event_handler(void *context, struct _1905_netlink_message *event, int length)
{
	switch(event->type) {
		case SWITCH_PORT_CHANGE:
			{
				struct port_status_info *status = NULL;
				if (length - sizeof(struct _1905_netlink_message) != event->len) {
					debug(DEBUG_ERROR, "invalid SWITCH_PORT_CHANGE event\n");
					break;
				}

				status = (struct port_status_info*)(event->event);
				eth_layer_port_data_update_and_notify(context, status);
			}
			break;
		default:
			debug(DEBUG_ERROR, "unknown netlink event %d\n", event->type);
			break;
	}
	return 0;
}

int update_switch_port_status_by_netlink(int port_index)
{
	unsigned char buf[64] = {0};
	struct _1905_netlink_message *cmd = (struct _1905_netlink_message *)buf;
	struct port_status_info *port_status = (struct port_status_info *)cmd->event;

	cmd->type = GET_SWITCH_PORT_STATUS;
	cmd->len = sizeof(struct port_status_info);

	port_status->port = port_index;

	if (netlink_msg_send(buf, cmd->len + sizeof(struct port_status_info), 0) < 0) {
		debug(DEBUG_ERROR, "netlink msg send fail\n");
		return -1;
	}

	return 0;
}

