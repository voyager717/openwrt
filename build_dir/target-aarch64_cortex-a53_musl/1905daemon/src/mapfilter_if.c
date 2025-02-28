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
#include <net/if.h>

#include "os.h"
#include "multi_ap.h"
#include "mapfilter_if.h"

static int netlink_sock = 0;
static int netlink_pid = 0;

unsigned char *cmd_buf = NULL;

int mapfilter_netlink_init(unsigned int pid)
{
	int sock;
	struct sockaddr_nl addr;
	int group = 1;

	cmd_buf = os_malloc(2048);
	if(!cmd_buf) {
		printf("failed to allocate memory\n");
		return -1;
	}
	sock = socket(AF_NETLINK, SOCK_RAW, MAP_NETLINK);
	if (sock < 0) {
		printf("sock < 0.\n");
		os_free(cmd_buf);
		return -1;
	}

	memset((void *) &addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = pid;
	addr.nl_groups = 1;
	/* This doesn't work for some reason. See the setsockopt() below. */
	/* addr.nl_groups = MYMGRP; */

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		printf("bind < 0.\n");
		close(sock);
		os_free(cmd_buf);
		return -1;
	}

	if (setsockopt(sock, 270, NETLINK_ADD_MEMBERSHIP, &group, sizeof(group)) < 0) {
		printf("setsockopt < 0\n");
		close(sock);
		os_free(cmd_buf);
		return -1;
	}

	netlink_sock = sock;
	netlink_pid = pid;

	return sock;
}

int mapfilter_netlink_deinit(int sock)
{
	close(sock);
	os_free(cmd_buf);
	return 0;
}

static int mapfilter_netlink_msg_send(const unsigned char *message, int len, unsigned int dst_group)
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
		printf("allocate nlmsghdr fail\n");
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
		printf("netlink msg send fail\n");
		os_free(nlh);
		return -3;
	}

	os_free(nlh);
	return 0;
}

int mapfilter_set_all_interface(struct local_itfs *itf)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	int len = sizeof(struct local_itfs) + itf->num * sizeof(struct local_interface);
	int ret = 0;

	msg->type = UPDATE_MAP_NET_DEVICE;
	msg->len = len;
	os_memcpy(msg->event, (unsigned char*)itf, len);

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + len, 0);
	return ret;
}

int mapfilter_set_primary_interface(struct local_interface *itf, unsigned char primary)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	struct primary_itf_setting *setting = (struct primary_itf_setting*)msg->event;
	int len = sizeof(struct primary_itf_setting);
	int ret = 0;

	msg->type = SET_PRIMARY_INTERFACE;
	os_memcpy(&setting->inf, itf, sizeof(struct local_interface));
	setting->primary = primary;
	msg->len = len;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + len, 0);
	return ret;
}

int mapfilter_set_uplink_path(struct local_interface *in, struct local_interface *out)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	struct up_link_path_setting *setting = (struct up_link_path_setting*)msg->event;
	int len = 0, ret = 0;

	msg->type = SET_UPLINK_PATH_ENTRY;
	os_memcpy(&setting->in, in, sizeof(struct local_interface));
	os_memcpy(&setting->out, out, sizeof(struct local_interface));
	len = sizeof(struct up_link_path_setting);
	msg->len = len;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + len, 0);
	return ret;
}

int mapfilter_dump_debug_info()
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;

	int ret = 0;

	msg->type = DUMP_DEBUG_INFO;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message), 0);
	return ret;
}

int mapfilter_show_version()
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;

	int ret = 0;

	msg->type = SHOW_VERSION;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message), 0);
	return ret;
}


int mapfilter_ts_onoff(char onoff)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	int ret = 0;

	msg->type = TRAFFIC_SEPARATION_ONOFF;
	msg->len = 1;
	msg->event[0] = onoff;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + msg->len, 0);
	return ret;
}

unsigned char *mapfilter_read_nlmsg(unsigned char *rx_buf, int buf_len, int *reply_len)
{
	struct sockaddr_nl nladdr;
	struct msghdr msg;
	struct iovec iov;
	struct nlmsghdr *nlh = (struct nlmsghdr *)rx_buf;
	int ret = 0;

	iov.iov_base = (void *)nlh;
	iov.iov_len = buf_len;

	os_memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *)&(nladdr);
	msg.msg_namelen = sizeof(nladdr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	ret = recvmsg(netlink_sock, &msg, 0);
	if (ret<0) {
	    return NULL;
	}

	*reply_len = nlh->nlmsg_len - NLMSG_SPACE(0);

	return NLMSG_DATA(nlh);
}

#ifdef MAP_R2
int mapfilter_set_ts_default_8021q(unsigned short primary_vid, unsigned char default_pcp)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	struct ts_default_8021q *setting = (struct ts_default_8021q*)msg->event;
	int len = 0, ret = 0;

	msg->type = SET_TRAFFIC_SEPARATION_DEFAULT_8021Q;
	setting->primary_vid = primary_vid;
	setting->default_pcp = default_pcp;
	len = sizeof(struct ts_default_8021q);
	msg->len = len;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + len, 0);
	return ret;
}

int mapfilter_set_ts_policy(unsigned char num, struct ssid_2_vid_mapping *mapping)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	struct ts_policy *setting = (struct ts_policy*)msg->event;
	int len = 0, ret = 0;

	msg->type = SET_TRAFFIC_SEPARATION_POLICY;
	setting->num = num;

	if (mapping)
		os_memcpy(setting->ssid_2_vid, mapping, num * sizeof(struct ssid_2_vid_mapping));

	len = sizeof(struct ts_policy) + num * sizeof(struct ssid_2_vid_mapping);
	msg->len = len;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + len, 0);
	return ret;
}

int mapfilter_set_transparent_vid(unsigned short vids[], unsigned char vid_num)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	struct transparent_vids *setting = (struct transparent_vids*)msg->event;
	int len = 0, ret = 0;
	unsigned char i = 0;

	msg->type = SET_TRANSPARENT_VID;

	printf("transparent vid number=%d\n", vid_num);

	for (i = 0; i < vid_num; i++) {
		setting->vids[i] = vids[i];
		printf("transparent vid=%d\n", vids[i]);
	}

	setting->num = vid_num;

	len = sizeof(struct transparent_vids) + vid_num * sizeof(unsigned short);
	msg->len = len;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + len, 0);
	return ret;
}

int mapfilter_update_client_vid(unsigned char mac[], unsigned short vid,
	unsigned char status, unsigned char ssid_len, char *ssid, 
	unsigned char *al_mac)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	struct client_vid *setting = (struct client_vid*)msg->event;
	int len = 0, ret = 0;

	msg->type = UPDATE_CLIENT_VID;

	os_memset(setting, 0, sizeof(struct client_vid));
	os_memcpy(setting->client_mac, mac, ETH_ALEN);
	setting->vid = vid;
	setting->status = status;
	setting->ssid_len = ssid_len;
	os_memcpy(setting->ssid, ssid, ssid_len);
	os_memcpy(setting->al_mac, al_mac, ETH_ALEN);

	len = sizeof(struct client_vid);
	msg->len = len;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + len, 0);
	return ret;
}

int mapfilter_set_wan_tag(unsigned char *wan_mac, unsigned char status)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	int ret = 0;

	msg->type = WAN_TAG_FLAG_ON_DEV;
	msg->len = ETH_ALEN + 1;
	os_memcpy(msg->event, (unsigned char*)wan_mac, ETH_ALEN);
	msg->event[ETH_ALEN] = status;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + msg->len, 0);
	return ret;
}
#endif

#ifdef MAP_R3_SP
int mapfilter_set_sp_rules(struct dl_list *static_list, struct dl_list *dynamic_list)
{
	struct map_netlink_message *msg = NULL;
	int ret = 0, num = 0, i = 0;
	struct sp_rule_element *r_ele = NULL;
	struct sp_rules_config *r = NULL;

	if (static_list)
		num += dl_list_len(static_list);

	if (dynamic_list)
		num += dl_list_len(dynamic_list);

	if (!num || (!static_list && !dynamic_list)) {
		ret = 0;
		goto err;
	}

	msg = os_malloc(sizeof(struct map_netlink_message) + sizeof(struct sp_rules_config) +
		num * sizeof(struct sp_rule));

	if (!msg) {
		ret = -1;
		debug(DEBUG_ERROR, SP_PREX"failed to allocate memory\n");
		goto err;
	}

	msg->type = SERVICE_PRIORITIZATION_RULE;
	msg->len = sizeof(struct sp_rules_config) + num * sizeof(struct sp_rule);

	r = (struct sp_rules_config *)msg->event;
	
	if (static_list) {
		dl_list_for_each(r_ele, static_list, struct sp_rule_element, entry) {
			if (r_ele->conf_status == RULE_NOT_CONFIGED_2_MAPFILTER)
				r->rules[i++] = r_ele->rule;
		}
	}

	if (dynamic_list) {
		dl_list_for_each(r_ele, dynamic_list, struct sp_rule_element, entry) {
			if (r_ele->conf_status == RULE_NOT_CONFIGED_2_MAPFILTER)
				r->rules[i++] = r_ele->rule;
		}
	}
	debug(DEBUG_ERROR, "rule_count = %d , total_num=%d\n", i, num);
	r->number = i;

	ret = mapfilter_netlink_msg_send((unsigned char *)msg, sizeof(struct map_netlink_message) + msg->len, 0);

	if (!ret) {
		/*set all rule config status to RULE_CONFIGED_2_MAPFILTER*/
		if (static_list) {
			dl_list_for_each(r_ele, static_list, struct sp_rule_element, entry)
				r_ele->conf_status = RULE_CONFIGED_2_MAPFILTER;
		}
		if (dynamic_list) {
			dl_list_for_each(r_ele, dynamic_list, struct sp_rule_element, entry)
				r_ele->conf_status = RULE_CONFIGED_2_MAPFILTER;
		}
	}

	os_free(msg);
err:
	return ret;
}

int mapfilter_set_sp_dscp_mapping_tbl(unsigned char *dptbl)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	int ret = 0;
	struct sp_dscp_mapping_tbl_config *tbl = (struct sp_dscp_mapping_tbl_config *)msg->event;

	if (!dptbl) {
		ret = -1;
		goto err;
	}

	msg->type = SERVICE_PRIORITIZATION_DSCP_MAPPING_TBL;
	msg->len = sizeof(struct sp_dscp_mapping_tbl_config);

	os_memcpy(tbl->dptbl, dptbl, 64);

	ret = mapfilter_netlink_msg_send((unsigned char *)msg, sizeof(struct map_netlink_message) + msg->len, 0);
err:
	return ret;
}

int mapfilter_set_sp_clear_rules()
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	int ret = 0;

	msg->type = SERVICE_PRIORITIZATION_CLEAR_RULE;
	msg->len = 0;

	ret = mapfilter_netlink_msg_send((unsigned char *)msg, sizeof(struct map_netlink_message) + msg->len, 0);

	return ret;
}

int mapfilter_set_sp_rm_one_rule(struct sp_rule *rule)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	struct sp_rule *r = (struct sp_rule *)msg->event;
	int ret = 0;

	msg->type = SERVICE_PRIORITIZATION_REMOVE_ONE_RULE;
	msg->len = sizeof(struct sp_rule);
	*r = *rule;

	ret = mapfilter_netlink_msg_send((unsigned char *)msg, sizeof(struct map_netlink_message) + msg->len, 0);

	return ret;
}

#endif

