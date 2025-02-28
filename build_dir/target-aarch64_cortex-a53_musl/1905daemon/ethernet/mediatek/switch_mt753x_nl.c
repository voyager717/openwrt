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

/*
 * switch_legacy.h, it's used for <<switch 753x + upstream driver>>
 * 
 * Author: Sirui Zhao <Sirui.Zhao@mediatek.com>
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <pthread.h>

#include "debug.h"
#include "switch_layer.h"
#include "switch_mt753x_nl.h"

static struct nl_sock *user_sock;
static struct nl_cache *cache;
static struct genl_family *family;
static struct nlattr *attrs[MT753X_ATTR_TYPE_MAX + 1];
static pthread_mutex_t lock;

static int wait_handler(struct nl_msg *msg, void *arg)
{
	int *finished = arg;

	*finished = 1;
	return NL_STOP;
}

static int construct_attrs(struct nl_msg *msg, void *arg)
{
	struct mt753x_attr *val = arg;
	int type = val->type;

	if (val->dev_id > -1)
		NLA_PUT_U32(msg, MT753X_ATTR_TYPE_DEV_ID, val->dev_id);

	if (val->op == 'r') {
		if (val->phy_dev != -1)
			NLA_PUT_U32(msg, MT753X_ATTR_TYPE_PHY_DEV, val->phy_dev);
		if (val->port_num >= 0)
			NLA_PUT_U32(msg, MT753X_ATTR_TYPE_PHY, val->port_num);
		NLA_PUT_U32(msg, type, val->reg);
	} else if (val->op == 'w') {
		if (val->phy_dev != -1)
			NLA_PUT_U32(msg, MT753X_ATTR_TYPE_PHY_DEV, val->phy_dev);
		if (val->port_num >= 0)
			NLA_PUT_U32(msg, MT753X_ATTR_TYPE_PHY, val->port_num);
		NLA_PUT_U32(msg, type, val->reg);
		NLA_PUT_U32(msg, MT753X_ATTR_TYPE_VAL, val->value);
	} else {
		printf("construct_attrs_message\n");
		NLA_PUT_STRING(msg, type, "hello");
	}
	return 0;

nla_put_failure:
	return -1;
}

static int spilt_attrs(struct nl_msg *msg, void *arg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct mt753x_attr *val = arg;
	char *str;

	if (nla_parse(attrs, MT753X_ATTR_TYPE_MAX, genlmsg_attrdata(gnlh, 0),
		      genlmsg_attrlen(gnlh, 0), NULL) < 0)
		goto done;

	if ((gnlh->cmd == MT753X_CMD_WRITE) || (gnlh->cmd == MT753X_CMD_READ)) {
		if (attrs[MT753X_ATTR_TYPE_MESG]) {
			str = nla_get_string(attrs[MT753X_ATTR_TYPE_MESG]);
			printf(" %s\n", str);
			if (!strncmp(str, "No", 2))
				goto done;
		}
		if (attrs[MT753X_ATTR_TYPE_REG]) {
			val->reg =
			    nla_get_u32(attrs[MT753X_ATTR_TYPE_REG]);
		}
		if (attrs[MT753X_ATTR_TYPE_VAL]) {
			val->value =
			    nla_get_u32(attrs[MT753X_ATTR_TYPE_VAL]);
		}
	}
	else
		goto done;

	return 0;
done:
	return NL_SKIP;
}

static int mt753x_request_callback(int cmd, int (*spilt)(struct nl_msg *, void *),
				   int (*construct)(struct nl_msg *, void *),
				   void *arg)
{
	struct nl_msg *msg;
	struct nl_cb *callback = NULL;
	int finished;
	int flags = 0;
	int err = 0;

	/*Allocate an netllink message buffer*/
	msg = nlmsg_alloc();
	if (!msg) {
		fprintf(stderr, "Failed to allocate netlink message\n");
		exit(1);
	}
	if (!construct) {
		if (cmd == MT753X_CMD_REQUEST)
			flags |= NLM_F_REQUEST;
		else
			flags |= NLM_F_DUMP;
	}
	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, genl_family_get_id(family),
		    0, flags, cmd, 0);

	/*Fill attaribute of netlink message by construct function*/
	if (construct) {
		if (construct(msg, arg) < 0) {
			fprintf(stderr, "attributes error\n");
			goto nal_put_failure;
		}
	}

	/*Allocate an new callback handler*/
	callback = nl_cb_alloc(NL_CB_CUSTOM);
	if (!callback) {
		fprintf(stderr, "Failed to allocate callback handler\n");
		exit(1);
	}

	/*Send netlink message*/
	err = nl_send_auto_complete(user_sock, msg);
	if (err < 0) {
		fprintf(stderr, "nl_send_auto_complete failied:%d\n", err);
		goto out;
	}
	finished = 0;
	if (spilt)
		nl_cb_set(callback, NL_CB_VALID, NL_CB_CUSTOM, spilt, arg);

	if (construct)
		nl_cb_set(callback, NL_CB_ACK, NL_CB_CUSTOM, wait_handler,
			  &finished);
	else
		nl_cb_set(callback, NL_CB_FINISH, NL_CB_CUSTOM, wait_handler,
			  &finished);

	/*receive message from kernel request*/
	err = nl_recvmsgs(user_sock, callback);
	if (err < 0)
		goto out;

	/*wait until an ACK is received for the latest not yet acknowledge*/
	if (!finished)
		err = nl_wait_for_ack(user_sock);
out:
	if (callback)
		nl_cb_put(callback);

nal_put_failure:
	nlmsg_free(msg);
	return err;
}

void mt753x_netlink_free(void)
{
	if (family)
		nl_object_put((struct nl_object *)family);
	if (cache)
		nl_cache_free(cache);
	if (user_sock)
		nl_socket_free(user_sock);
	user_sock = NULL;
	cache = NULL;
	family = NULL;
}

int mt753x_netlink_init(void)
{
	int ret = 0;

	user_sock = NULL;
	cache = NULL;
	family = NULL;

	pthread_mutex_init(&lock, NULL);
	/*Allocate an new netlink socket*/
	user_sock = nl_socket_alloc();



	if (!user_sock) {
		fprintf(stderr, "Failed to create user socket\n");
		printf("Failed to create user socket\n");
		goto err;
	}
	/*Connetct the genl controller*/
	if (genl_connect(user_sock)) {
		fprintf(stderr, "Failed to connetct to generic netlink\n");
		printf("Failed to connetct to generic netlink\n");
		goto err;
	}
	/*Allocate an new nl_cache*/
	ret = genl_ctrl_alloc_cache(user_sock, &cache);
	if (ret < 0) {
		fprintf(stderr, "Failed to allocate netlink cache\n");
		printf("Failed to allocate netlink cache\n");
		goto err;
	}

	/*Look up generic netlik family by "mt753x" in the provided cache*/
	family = genl_ctrl_search_by_name(cache, MT753X_GENL_NAME);
	if (!family) {
		//fprintf(stderr,"switch(mt753x) API not be prepared\n");
		printf("switch(mt753x) API not be prepared\n");
		goto err;
	}
	return 0;
err:
	mt753x_netlink_free();
	return -EINVAL;
}

static int mt753x_request(struct mt753x_attr *arg, int cmd)
{
	int err;

	pthread_mutex_lock(&lock);
	err = mt753x_request_callback(cmd, spilt_attrs, construct_attrs, arg);
	pthread_mutex_unlock(&lock);
	if (err < 0) {
		fprintf(stderr, "mt753x deal request error\n");
		return err;
	}
	return 0;
}

static int cr_operate_netlink(char op, struct mt753x_attr *arg, int port_num,
					int phy_dev, int offset, int *value)
{
	int ret = -1;
	struct mt753x_attr *attr = arg;

	if(!arg){
		debug(DEBUG_ERROR, "input arg is null error\n");
		return ret;
	}
	attr->port_num = port_num;
	attr->phy_dev = phy_dev;
	attr->reg = offset;
	attr->value = -1;
	attr->type = MT753X_ATTR_TYPE_REG;

	switch (op)
	{
		case 'r':
			attr->op = 'r';
			ret = mt753x_request(attr, MT753X_CMD_READ);
			*value = attr->value;
			break;
		case 'w':
			attr->op = 'w';
			attr->value = *value;
			ret = mt753x_request(attr, MT753X_CMD_WRITE);
			break;
		default:
			break;
	}

	return ret;
}

int reg_read_nl(struct mt753x_attr *arg, int offset, int *value)
{
	int ret;

	ret = cr_operate_netlink('r', arg, -1, -1, offset, value);
	if (ret < 0) {
		debug(DEBUG_ERROR, "switch read: netlink error\n");
		return -1;
	}
	return ret;
}

int reg_write_nl(struct mt753x_attr *arg, int offset, int value)
{
	int ret;

	ret = cr_operate_netlink('w', arg, -1, -1, offset, &value);
	if (ret < 0) {
		debug(DEBUG_ERROR, "switch write: netlink error\n");
		return -1;
	}
	return ret;
}

int netlink_cl22_read_phy(struct mt753x_attr *arg, int port_num, int phy_addr, int *value)
{
	int ret;

	ret = cr_operate_netlink('r', arg, port_num, -1, phy_addr, value);
	if (ret < 0) {
		debug(DEBUG_ERROR, "phy_cl22: netlink error\n");
		return -1;
	}
	return ret;
}


//reserve for external phy, which is connected witch mt753x switch
int netlink_cl45_read_phy(struct mt753x_attr *arg, int port_num, int phy_dev,
			 int phy_addr, int *value)
{
	int ret;

	ret = cr_operate_netlink('r', arg, port_num, phy_dev, phy_addr, value);
	if (ret < 0) {
		debug(DEBUG_ERROR, "phy_cl45: netlink error\n");
		return -1;
	}
	return ret;
}


int switch_nl_deinit()
{
	mt753x_netlink_free();
	return 0;
}




