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
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stddef.h>
#include "interface.h"
#include "data_def.h"

#include "_1905_interface_ctrl.h"
#include "../inc/_1905_lib_internal.h"
#include "../inc/common.h"
#include "debug.h"

#ifdef CONFIG_MASK_MACADDR
#define PRINT_MAC(a) a[0],a[3],a[4],a[5]
#else
#define PRINT_MAC(a) a[0],a[1],a[2],a[3],a[4],a[5]
#endif


#define CMD_SUC_SYNC_EVENT 0
#define CMD_SUC_ASYNC_EVENT 1
#define CMD_PROCESS_FAIL 2

#ifndef BIT
#define BIT(x) (1U << (x))
#endif


#define TOPOLOGY_DISCOVERY      0x0000
#define TOPOLOGY_NOTIFICATION   0x0001
#define TOPOLOGY_QUERY          0x0002
#define TOPOLOGY_RESPONSE       0x0003
#define VENDOR_SPECIFIC         0x0004
#define LINK_METRICS_QUERY      0x0005
#define LINK_METRICS_RESPONSE   0x0006
#define AP_AUTOCONFIG_SEARCH    0x0007
#define AP_AUTOCONFIG_RESPONSE  0x0008
#define AP_AUTOCONFIG_WSC       0x0009
#define AP_AUTOCONFIG_RENEW     0x000A
#define P1905_PB_EVENT_NOTIFY   0x000B
#define P1905_PB_JOIN_NOTIFY    0x000C
#define SCLIENT_CAPABILITY_QUERY 0x000D
#define ICHANNEL_SELECTION_REQUEST 0x000E
#define Ack_1905 0x8000
#define AP_CAPABILITY_QUERY 0x8001
#define AP_CAPABILITY_REPORT 0x8002
#define MAP_POLICY_CONFIG_REQUEST 0x8003
#define CHANNEL_PREFERENCE_QUERY 0x8004
#define CHANNEL_PREFERENCE_REPORT 0x8005
#define CHANNEL_SELECTION_REQUEST 0x8006
#define CHANNEL_SELECTION_RESPONSE 0x8007
#define OPERATING_CHANNEL_REPORT 0x8008
#define CLIENT_CAPABILITY_QUERY 0x8009
#define CLIENT_CAPABILITY_REPORT 0x800A
#define AP_LINK_METRICS_QUERY 0x800B
#define AP_LINK_METRICS_RESPONSE 0x800C
#define ASSOC_STA_LINK_METRICS_QUERY 0x800D
#define ASSOC_STA_LINK_METRICS_RESPONSE 0x800E
#define UNASSOC_STA_LINK_METRICS_QUERY 0x800F
#define UNASSOC_STA_LINK_METRICS_RESPONSE 0x8010
#define BEACON_METRICS_QUERY 0x8011
#define BEACON_METRICS_RESPONSE 0x8012
#define COMBINED_INFRASTRUCTURE_METRICS 0x8013
#define CLIENT_STEERING_REQUEST 0x8014
#define CLIENT_STEERING_BTM_REPORT 0x8015
#define CLIENT_ASSOC_CONTROL_REQUEST 0x8016
#define CLIENT_STEERING_COMPLETED 0x8017
#define HIGHER_LAYER_DATA_MESSAGE 0x8018
#define BACKHAUL_STEERING_REQUEST 0x8019
#define BACKHAUL_STEERING_RESPONSE 0x801A
/*channel scan feature*/
#define CHANNEL_SCAN_REQUEST 0x801B
#define CHANNEL_SCAN_REPORT 0x801C
/*  DPP CCE Indication */
#define DPP_CCE_INDICATION_MESSAGE 0x801D

/* DFS CAC */
#define CAC_REQUEST 0x8020
#define CAC_TERMINATION 0x8021
#define CLIENT_DISASSOCIATION_STATS 0x8022

#define ASSOCIATION_STATUS_NOTIFICATION 0x8025
#define TUNNELED_MESSAGE 0x8026
#define BACKHAUL_STA_CAP_QUERY_MESSAGE 0x8027
#define BACKHAUL_STA_CAP_REPORT_MESSAGE 0x8028
#define PROXIED_ENCAP_DPP_MESSAGE 0x8029
#define DIRECT_ENCAP_DPP_MESSAGE 0x802A
#define BSS_RECONFIGURATION_TRIGGER_MESSAGE 0x802B
#define BSS_CONFIGURATION_REQUEST_MESSAGE 0x802C
#define BSS_CONFIGURATION_RESPONSE_MESSAGE 0x802D
#define BSS_CONFIGURATION_RESULT_MESSAGE 0x802E
#define CHIRP_NOTIFICATION_MESSAGE 0x802F
#define _1905_ENCAP_EAPOL 0x8030
#define DPP_BOOTSTRAPING_URI_NOTIFICATION 0x8031
#define DPP_BOOTSTRAPING_URI_QUERY 0x8032
#define FAILED_CONNECTION_MESSAGE 0x8033
#define DPP_URI_NOTIFICATION_MESSAGE 0x8034
#define DEV_SEND_1905_REQUEST 0x9000

#define STA_LEAVE 0
#define STA_JOIN (1 << 7)


unsigned char lib_debug_level = MSG_INFO;

#define LIB_DEBUG(Level, fmt, args...)   \
{\
	if(Level >= lib_debug_level)\
	{\
		printf("[%s]", __FUNCTION__);	\
		printf( fmt, ## args);          \
		printf("\n");\
	}\
}

unsigned char _1905_multicast_address[6]
                    = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x13 };

#define MAX_LIBBUF_LEN 15360
unsigned char cmdu_buf[MAX_LIBBUF_LEN] = {0};

int _1905_interface_ctrl_send(struct _1905_context *ctrl, const char *cmd, size_t cmd_len)
{
	struct os_time started_at;
	const char *_cmd;
	size_t _cmd_len;
	int _error = 0;

	_cmd = cmd;
	_cmd_len = cmd_len;
	started_at.sec = 0;
	started_at.usec = 0;
retry_send:
	if (send(ctrl->s, _cmd, _cmd_len, 0) < 0) {
		_error = errno;
		if (errno == EAGAIN || errno == EBUSY || errno == EWOULDBLOCK) {
			/*
			 * Must be a non-blocking socket... Try for a bit
			 * longer before giving up.
			 */
			if (started_at.sec == 0) {
				os_get_time(&started_at);
			} else {
				struct os_time n;
				os_get_time(&n);
				/* Try for a few seconds. */
				if (os_reltime_expired(&n, &started_at, 5)) {
					LIB_DEBUG(MSG_ERROR, "send error(%d-%s)\n",_error, strerror(_error));
					return -2;
				}
			}
			os_sleep(0, 10000);
			goto retry_send;
		}
		LIB_DEBUG(MSG_ERROR, "send error(%d-%s)\n",_error, strerror(_error));
		return -1;
	}
	return 0;
}


int _1905_interface_ctrl_request(struct _1905_context *ctrl, const char *cmd, size_t cmd_len) {
	int ret = 0;
	unsigned short buf_len = 0;
	struct timeval tv;
	unsigned char recv_buf[50] = {0};
	size_t rev_buf_len = sizeof(recv_buf);
	struct msg *recv_event = NULL, *send_event = NULL;
	unsigned char rsp = 0;
	unsigned short change_buf_len = 0;
	struct os_time now;
	struct os_time started;

	os_memset(&now, 0, sizeof(struct os_time));
	os_memset(&started, 0, sizeof(struct os_time));

	if(!ctrl || !cmd || !cmd_len) {
		LIB_DEBUG(MSG_ERROR, "invalid input parameters \n");
		return -1;
	}

	if (strncmp(cmd, "ATTACH:", strlen("ATTACH:")) == 0 || strncmp(cmd, "DETACH", strlen("DETACH")) == 0) {
		ret = _1905_interface_ctrl_send(ctrl, cmd, cmd_len);
		return ret;
	}
	buf_len = ctrl->peer_recv_buf_len;
	send_event = (struct msg *)cmd;

	if(cmd_len > buf_len) {
		change_buf_len = cmd_len;
	} else if(ctrl->default_peer_recv_len < buf_len && cmd_len <= ctrl->default_peer_recv_len)
		change_buf_len = ctrl->default_peer_recv_len;

	if(change_buf_len == 0) {
		ret = _1905_interface_ctrl_send(ctrl, cmd, cmd_len);
	} else {
		LIB_DEBUG(MSG_ERROR, "send msg to notify 1905 change recv buffer size:%d (input cmd len:%zu, peer:%d, default:%d)\n",
			change_buf_len, cmd_len, ctrl->peer_recv_buf_len, ctrl->default_peer_recv_len);
		ret  = _1905_Sync_Recv_Buf_Size(ctrl, change_buf_len, ctrl->own_recv_buf_len);
		if(ret < 0) {
			LIB_DEBUG(MSG_ERROR, "notify 1905 change recv buffer size msg fail\n");
			return ret;
		}
		tv.tv_sec = 3;
		tv.tv_usec = 0;
		os_get_time(&started);
		while (1) {
			if (_1905_interface_ctrl_pending(ctrl, &tv)) {
				if (_1905_Receive(ctrl, (char *)recv_buf, &rev_buf_len) < 0) {
					LIB_DEBUG(MSG_ERROR, "recv fail! %s\n", strerror(errno));
					return -1;
				}
				recv_event = (struct msg *)recv_buf;

				if (recv_event->type == _1905_SYNC_RECV_BUF_SIZE_RSP_EVENT) {
					change_buf_len = 0;
					memcpy(&change_buf_len, recv_event->buffer, sizeof(change_buf_len));
					rsp = recv_event->buffer[sizeof(change_buf_len)];
					LIB_DEBUG(MSG_ERROR, "receive the _1905_SYNC_RECV_BUF_SIZE_RSP_EVENT from 1905\n");
					LIB_DEBUG(MSG_ERROR, "change buf size success len:%d, rsp :%d\n", change_buf_len, rsp);
					if (rsp == 1) {
						if (!ctrl->peer_recv_buf_len && !ctrl->default_peer_recv_len)
							ctrl->default_peer_recv_len = change_buf_len;
						ctrl->peer_recv_buf_len = change_buf_len;
						LIB_DEBUG(MSG_ERROR, "send cmd(type:%d,len:%zu)to 1905 success after adjust recv sock buf len\n",
							send_event->type, cmd_len);
						ret = _1905_interface_ctrl_send(ctrl, cmd, cmd_len);
						return ret;
					} else {
						LIB_DEBUG(MSG_ERROR, "adjust recv sock buf len :%d fail, drop msg (type:%d)\n",
							change_buf_len, send_event->type);
						return -1;
					}
				} else {
					LIB_DEBUG(MSG_ERROR, "receive wrong msg (type:%d), drop\n", recv_event->type);
				}
			} else {
				LIB_DEBUG(MSG_ERROR, "%ld s timeout! no change buf len rsp from 1905daemon\n", tv.tv_sec);
				return -2;
			}

			os_get_time(&now);
			if (os_reltime_expired(&now, &started, (os_time_t)tv.tv_sec)) {
				LIB_DEBUG(MSG_ERROR, "wait for _1905_SYNC_RECV_BUF_SIZE_RSP_EVENT timeout, drop msg (type:%d)\n",
					send_event->type);
				return -2;
			}
		}
	}
	return ret;
}

int _1905_interface_ctrl_attach_helper(struct _1905_context *ctrl, int attach, char* daemon)
{
	char buf[10];
	int ret;
	size_t len = 10;
	struct timeval tv;
	char request_buf[64];

	memset(request_buf, 0, sizeof(request_buf));
	memset(buf, 0, sizeof(buf));
	ret = snprintf(request_buf, sizeof(request_buf), attach ? "ATTACH:%s" : "DETACH:%s", daemon);
	if (os_snprintf_error(sizeof(request_buf), ret)) {
		LIB_DEBUG(MSG_ERROR, "[%d][%s]snprintf fail!\n", __LINE__, __func__);
		return -1;
	}
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	ret = _1905_interface_ctrl_request(ctrl, request_buf, strlen(request_buf));
	if (ret < 0) {
		LIB_DEBUG(MSG_ERROR, "send fail! %s\n", strerror(errno));
		return ret;
	}
	if (_1905_interface_ctrl_pending(ctrl, &tv)) {
		if(_1905_Receive(ctrl, buf, &len) < 0) {
			LIB_DEBUG(MSG_ERROR, "recv fail! %s\n", strerror(errno));
			return -1;
		}
	} else {
		LIB_DEBUG(MSG_ERROR, "3s timeout! no rsp from 1905daemon\n");
	}
	if (len == 3 && memcmp(buf, "OK\n", 3) == 0)
		return 0;

	return -1;
}


int _1905_interface_ctrl_attach(struct _1905_context *ctrl, char* daemon)
{
#define INIT_RETRY_TIME 10
	int i = 0;

	for (i = 0; i < INIT_RETRY_TIME; i++) {
		if (_1905_interface_ctrl_attach_helper(ctrl, 1, daemon) == 0)
			break;
		os_sleep(1, 0);
	}

	if (i >= INIT_RETRY_TIME)
		return -1;

	return 0;
}



int _1905_interface_ctrl_detach(struct _1905_context *ctrl)
{
	return _1905_interface_ctrl_attach_helper(ctrl, 0, "");
}

struct _1905_context * _1905_Init(const char *local_path)
{
	struct _1905_context *ctrl;
	static int counter = 0;
	int nameLen = 0;
	int tries = 0;
	int flags;
	int path_len = 0;
	int nSndBufLen = 0,nRcvBufLen = 0;
	socklen_t optlen = sizeof(int);
	char *daemon = NULL;
	int ret;

	daemon = malloc(strlen(local_path) + 1);
	if (!daemon)
		return NULL;
	ret = snprintf(daemon, strlen(local_path) + 1, "%s", local_path);
	if (os_snprintf_error(strlen(local_path) + 1, ret)) {
		LIB_DEBUG(MSG_ERROR, "[%d][%s]snprintf fail!\n", __LINE__, __func__);
		free(daemon);
		return NULL;
	}

	ctrl = (struct _1905_context *)malloc(sizeof(*ctrl));
	if (ctrl == NULL) {
		LIB_DEBUG(MSG_ERROR, "%s, alloc memory fail\n", __func__);
		free(daemon);
		return NULL;
	}
	memset(ctrl, 0, sizeof(*ctrl));

	ctrl->s = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (ctrl->s < 0) {
		free(ctrl);
		free(daemon);
		return NULL;
	}

	/* set send buf as 1M */
	if (getsockopt(ctrl->s, SOL_SOCKET, SO_SNDBUF, (void *)&nSndBufLen, &optlen) < 0) {
		nSndBufLen = 1024*1024;
	}
	else {
		nSndBufLen = nSndBufLen * 2;
	}

	/* set recv buf as 1M */
	if (getsockopt(ctrl->s, SOL_SOCKET, SO_RCVBUF, (void *)&nRcvBufLen, &optlen) < 0) {
		nRcvBufLen = 1024*1024;
	}
	else {
		nRcvBufLen = nRcvBufLen * 2;
	}

	/* set send buf as 1M */
	if (setsockopt(ctrl->s, SOL_SOCKET, SO_SNDBUF, (const char*)&nSndBufLen, sizeof(int)) < 0) {
		LIB_DEBUG(MSG_ERROR, "warning: %s set send buffer size failed, %s\n", __func__, strerror(errno));
	} else {
		LIB_DEBUG(MSG_DEBUG, "%s set send buffer size %d\n", __func__, nSndBufLen);
	}

	/* set recv buf as 1M */
	if (setsockopt(ctrl->s, SOL_SOCKET, SO_RCVBUF, (const char*)&nRcvBufLen, sizeof(int)) < 0) {
		LIB_DEBUG(MSG_ERROR, "%s set recv buffer size failed, %s\n", __func__, strerror(errno));
	} else {
		LIB_DEBUG(MSG_DEBUG, "%s set recv buffer size %d\n", __func__, nRcvBufLen);
	}

	ctrl->local.sun_family = AF_UNIX;
	counter++;
try_again:
	nameLen = strlen(local_path);
    if (nameLen >= (int) sizeof(ctrl->local.sun_path) -1) { /* too long? */
		close(ctrl->s);
		free(ctrl);
		LIB_DEBUG(MSG_ERROR, "nameLen(local_path) too long\n");
		free(daemon);
		return NULL;
    }
    ctrl->local.sun_path[0] = '\0';  /* abstract namespace */
	ret = snprintf(ctrl->local.sun_path + 1, sizeof(ctrl->local.sun_path) - 1, "%s", local_path);
	if (os_snprintf_error(sizeof(ctrl->local.sun_path) - 1, ret)) {
		LIB_DEBUG(MSG_ERROR, "[%d][%s]snprintf fail!\n", __LINE__, __func__);
		close(ctrl->s);
		free(ctrl);
		free(daemon);
		return NULL;
	}
    path_len = 1 + nameLen + offsetof(struct sockaddr_un, sun_path);
	tries++;
	if (bind(ctrl->s, (struct sockaddr *) &ctrl->local,
		    path_len) < 0) {
		if (errno == EADDRINUSE && tries < 2) {
			/*
			 * getpid() returns unique identifier for this instance
			 * of wpa_ctrl, so the existing socket file must have
			 * been left by unclean termination of an earlier run.
			 * Remove the file and try again.
			 */
			/*for abstract socket path, no need to unlink it*/
//			unlink(ctrl->local.sun_path);
			goto try_again;
		}
		close(ctrl->s);
		free(ctrl);
		LIB_DEBUG(MSG_ERROR, "%s, bind fail\n", __func__);
		free(daemon);
		return NULL;
	}

	ctrl->dest.sun_family = AF_UNIX;
	nameLen = strlen("1905_daemon");
    if (nameLen >= (int) sizeof(ctrl->dest.sun_path) -1) {
		close(ctrl->s);
		free(ctrl);
		LIB_DEBUG(MSG_ERROR, "nameLen too long\n");
		free(daemon);
		return NULL;
    }
    ctrl->dest.sun_path[0] = '\0';  /* abstract namespace */
	ret = snprintf(ctrl->dest.sun_path + 1, sizeof(ctrl->dest.sun_path) - 1, "%s", "1905_daemon");
	if (os_snprintf_error(sizeof(ctrl->dest.sun_path) - 1, ret)) {
		LIB_DEBUG(MSG_ERROR, "[%d][%s]snprintf fail!\n", __LINE__, __func__);
		close(ctrl->s);
		free(ctrl);
		free(daemon);
		return NULL;
	}
    path_len = 1 + nameLen + offsetof(struct sockaddr_un, sun_path);

	if (connect(ctrl->s, (struct sockaddr *) &ctrl->dest,
		    path_len) < 0) {
		close(ctrl->s);
		/*for abstract socket path, no need to unlink it*/
//		unlink(ctrl->local.sun_path);
		free(ctrl);
		LIB_DEBUG(MSG_ERROR, "%s, connect fail, %s\n", __func__, strerror(errno));
		free(daemon);
		return NULL;
	}

	/*
	 * Make socket non-blocking so that we don't hang forever if
	 * target dies unexpectedly.
	 */
	flags = fcntl(ctrl->s, F_GETFL);
	if (flags >= 0) {
		flags |= O_NONBLOCK;
		if (fcntl(ctrl->s, F_SETFL, flags) < 0) {
			perror("fcntl(ctrl->s, O_NONBLOCK)");
			/* Not fatal, continue on.*/
		}
	}

	ctrl->own_recv_buf_len = 0;
	ctrl->default_own_recv_len = 0;
	ctrl->peer_recv_buf_len = 0;
	ctrl->default_peer_recv_len = 0;
	ctrl->s_buf = NULL;

	if (_1905_interface_ctrl_attach(ctrl, daemon)) {
		LIB_DEBUG(MSG_ERROR, "attach failed\n");
		close(ctrl->s);
		free(ctrl);
		free(daemon);
		return NULL;
	}
	free(daemon);
	return ctrl;
}

void _1905_Deinit(struct _1905_context *ctrl)
{
	if (ctrl == NULL)
		return;
	if (_1905_interface_ctrl_detach(ctrl)) {
		LIB_DEBUG(MSG_ERROR, "detach failed\n");
	}
//	unlink(ctrl->local.sun_path);
	if (ctrl->s >= 0)
		close(ctrl->s);
	free(ctrl);
}

int _1905_Receive(struct _1905_context *ctrl, char *reply, size_t *reply_len)
{
	int res;

	res = recv(ctrl->s, reply, *reply_len, 0);
	if (res < 0)
		return res;
	*reply_len = res;
	return 0;
}


int _1905_interface_ctrl_pending(struct _1905_context *ctrl, struct timeval *tv)
{
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(ctrl->s, &rfds);
	select(ctrl->s + 1, &rfds, NULL, NULL, tv);
	return FD_ISSET(ctrl->s, &rfds);
}


int _1905_interface_ctrl_get_fd(struct _1905_context *ctrl)
{
	return ctrl->s;
}

int _1905_Set_Role(IN struct _1905_context *ctx, enum MAP_ROLE role)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct manage_cmd *_manage_cmd = (struct manage_cmd*)cmd->buffer;
	struct timeval tv;
	size_t len = sizeof(cmdu_buf);

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	memset(cmdu_buf, 0, sizeof(cmdu_buf));
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	cmd->type = MANAGEMENT_1905_COMMAND;
	cmd->length = sizeof(struct manage_cmd) + 1;
	_manage_cmd->cmd_id = MANAGE_SET_ROLE;
	_manage_cmd->len = 1;
	_manage_cmd->content[0] = (unsigned char)role;

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		if (_1905_interface_ctrl_pending(ctx, &tv)) {
			if (_1905_Receive(ctx, (char*)cmdu_buf, &len) < 0) {
				return -1;
			}
		}
		if (len == 3 && memcmp(cmdu_buf, "OK\n", 3) == 0)
			return 0;
		LIB_DEBUG(MSG_ERROR, "set role fail role(%d)\n", role);
		return -1;
	}
}

#define ENOUGH_CHECK(org, cur, step, total_len){	\
	if((unsigned short)((cur - org) + (step)) > (total_len)) { \
		LIB_DEBUG(MSG_ERROR, "not enough buffer at line %d, (cur - org)=%d step=%d total_len=%d", __LINE__, (int)(cur - org), step, total_len);	\
		return -1;\
	}	\
}

int _1905_Get_Local_Devinfo(IN struct _1905_context *ctx, OUT struct device_info *dev_info,
	OUT struct bridge_cap *br_cap, OUT struct supported_srv *srv)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct manage_cmd *_manage_cmd = (struct manage_cmd*)cmd->buffer;
	struct timeval tv;
	size_t len = sizeof(cmdu_buf);
	unsigned char *p = NULL;
	unsigned short len_body = 0;
	unsigned char i = 0, j = 0;
	int charlen = sizeof(unsigned char);
	int shortlen = sizeof(unsigned short);

	if (ctx == NULL || dev_info == NULL) {
		LIB_DEBUG(MSG_ERROR, "%s NULL pointer", ctx == NULL ? "ctx" : "dev_info");
		goto fail;
	}

	if (br_cap == NULL || srv == NULL) {
		LIB_DEBUG(MSG_ERROR, "%s NULL pointer", br_cap == NULL ? "br_cap" : "srv");
		goto fail;
	}

	memset(cmdu_buf, 0, sizeof(cmdu_buf));
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	cmd->type = MANAGEMENT_1905_COMMAND;
	cmd->length = sizeof(struct manage_cmd);
	_manage_cmd->cmd_id = MANAGE_GET_LOCAL_DEVINFO;
	_manage_cmd->len = 0;

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		goto fail;
	} else {
		if (_1905_interface_ctrl_pending(ctx, &tv)) {
			if (_1905_Receive(ctx, (char*)cmdu_buf, &len) < 0) {
				goto fail;
			}
		}

		p = cmdu_buf;

		ENOUGH_CHECK(cmdu_buf, p, shortlen, (int)len);
		if (*(unsigned short*)p != MANAGE_GET_LOCAL_DEVINFO) {
			LIB_DEBUG(MSG_ERROR, "raw data from 1905 is wrong");
			goto fail;
		}
		p += sizeof(unsigned short);

		/*mid*/
		ENOUGH_CHECK(cmdu_buf, p, shortlen, (int)len);
		p += sizeof(unsigned short);

		ENOUGH_CHECK(cmdu_buf, p, shortlen, (int)len);
		len_body = *(unsigned short*)p;
		if (len_body != len - 6) {
			LIB_DEBUG(MSG_ERROR, "invalid length, len_body(%d) total len(%d)", len_body, (int)len);
			goto fail;
		}
		p += sizeof(unsigned short);

		/*dev info*/
		ENOUGH_CHECK(cmdu_buf, p, ETH_ALEN, (int)len);
		memcpy(dev_info->al_mac, p, ETH_ALEN);
		p += ETH_ALEN;
		LIB_DEBUG(MSG_DEBUG, "almac "MACSTR, PRINT_MAC(dev_info->al_mac));

		ENOUGH_CHECK(cmdu_buf, p, charlen, (int)len);
		dev_info->inf_num = *p++;
		LIB_DEBUG(MSG_DEBUG, "dev_info->inf_num=%d", dev_info->inf_num);

		ENOUGH_CHECK(cmdu_buf, p, (ETH_ALEN + (unsigned int)shortlen) * dev_info->inf_num, (int)len);
		for (i = 0; i < dev_info->inf_num; i++) {
			memcpy(dev_info->inf_info[i].mac, p, ETH_ALEN);
			p += ETH_ALEN;
			dev_info->inf_info[i].media_type = *(unsigned short*)p;
			p += sizeof(unsigned short);
			LIB_DEBUG(MSG_DEBUG, "interface mac "MACSTR,
				PRINT_MAC(dev_info->inf_info[i].mac));
			LIB_DEBUG(MSG_DEBUG, "interface media type %04x", dev_info->inf_info[i].media_type);
		}

		/*bridge info*/
		ENOUGH_CHECK(cmdu_buf, p, charlen, (int)len);
		br_cap->br_num = *p++;
		LIB_DEBUG(MSG_DEBUG, "br_cap->br_num=%d", br_cap->br_num);

		for (i = 0; i < br_cap->br_num; i++) {
			ENOUGH_CHECK(cmdu_buf, p, charlen, (int)len);
			unsigned char br_name_len = *p++;

			ENOUGH_CHECK(cmdu_buf, p, br_name_len, (int)len);
			if (br_name_len >= IFNAMSIZ - 1)
				br_name_len = IFNAMSIZ - 1;
			os_strncpy(br_cap->bridge_info[i].br_name, (char*)p, br_name_len);
			p += br_name_len;
			LIB_DEBUG(MSG_DEBUG, "\t(br_cap->bridge_info[i].br_name %s", br_cap->bridge_info[i].br_name);

			ENOUGH_CHECK(cmdu_buf, p, charlen, (int)len);
			br_cap->bridge_info[i].inf_num = *p++;
			LIB_DEBUG(MSG_DEBUG, "\tbr_cap->bridge_info[i].inf_num=%d", br_cap->bridge_info[i].inf_num);

			ENOUGH_CHECK(cmdu_buf, p, br_cap->bridge_info[i].inf_num * ETH_ALEN, (int)len);
			os_memcpy(br_cap->bridge_info[i].inf_mac, p, br_cap->bridge_info[i].inf_num * ETH_ALEN);
			p += br_cap->bridge_info[i].inf_num * ETH_ALEN;

			/*for (j = 0; j < br_cap->bridge_info[i].inf_num; j++) {
				LIB_DEBUG(MSG_DEBUG, "\t\t "MACSTR, br_cap->bridge_info[i].inf_mac[j * ETH_ALEN],
					br_cap->bridge_info[i].inf_mac[j * ETH_ALEN + 1],
					br_cap->bridge_info[i].inf_mac[j * ETH_ALEN + 2],
					br_cap->bridge_info[i].inf_mac[j * ETH_ALEN + 3],
					br_cap->bridge_info[i].inf_mac[j * ETH_ALEN + 4],
					br_cap->bridge_info[i].inf_mac[j * ETH_ALEN + 5]);
			}*/
		}

		/*supported service*/
		ENOUGH_CHECK(cmdu_buf, p, charlen, (int)len);
		srv->srv_num = *p++;
		LIB_DEBUG(MSG_DEBUG, "srv->srv_num=%d", srv->srv_num);

		ENOUGH_CHECK(cmdu_buf, p, srv->srv_num, (int)len);
		os_memcpy(srv->srv, p, srv->srv_num);
		for (j = 0; j < srv->srv_num; j++) {
			LIB_DEBUG(MSG_DEBUG, "srv->srv=%02x", srv->srv[j]);
		}
	}
	return 0;
fail:
	return -1;
}


int _1905_Set_Radio_Basic_Cap (IN struct _1905_context* ctx,
	IN struct ap_radio_basic_cap * cap)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	int len = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (cap == NULL) {
		LIB_DEBUG(MSG_ERROR, "cap NULL pointer");
		return -1;
	}
	cmd->type = LIB_RADIO_BASIC_CAP;
	len = sizeof(struct ap_radio_basic_cap) + cap->op_class_num * sizeof(struct radio_basic_cap);
	cmd->length = len;
	memcpy(cmd->buffer, (unsigned char*)cap, len);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	}else {
		return 0;
	}
}

int _1905_Set_Channel_Preference_Report_Info (IN struct _1905_context* ctx,
	IN int ch_prefer_cnt, IN struct ch_prefer_lib * ch_prefers,
	IN int restriction_cnt, IN struct restriction_lib *restrictions,
	IN struct cac_completion_report_lib  *cac_rep
#ifdef MAP_R2
	, IN struct cac_status_report_lib  *cac_status_rep
#endif
	, IN unsigned short mid
	)
{
	unsigned short len = 0;
	struct msg *cmd = (struct msg* )cmdu_buf;
	unsigned char* p = NULL;
	unsigned char* p_old = NULL;
	int i = 0, j = 0, k = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if ((ch_prefer_cnt > 0 && ch_prefers == NULL) || (restriction_cnt > 0 && restrictions == NULL)) {
		LIB_DEBUG(MSG_ERROR, "%s null pointer", (ch_prefers == NULL) ? "ch_prefers" : "restrictions");
		return -1;
	}
	p = cmd->buffer;
	cmd->type = WAPP_CHANNEL_PREFERENCE_REPORT_INFO;
	cmd->mid = mid;

	for (i = 0; i < ch_prefer_cnt; i++) {
		*p++ = 0x8B;

		p_old = p;
		p += 2;
		memcpy(p, ch_prefers->identifier, ETH_ALEN);
		p += ETH_ALEN;

		*p++ = ch_prefers->op_class_num;
		for (j = 0; j < ch_prefers->op_class_num; j++) {
			*p++ = ch_prefers->opinfo[j].op_class;
			*p++ = ch_prefers->opinfo[j].ch_num;
			memcpy(p, ch_prefers->opinfo[j].ch_list, ch_prefers->opinfo[j].ch_num);
			p += ch_prefers->opinfo[j].ch_num;
			*p++ = (((ch_prefers->opinfo[j].perference & 0x0F) << 4) | (ch_prefers->opinfo[j].reason & 0x0F));
		}
		len = (unsigned short)(p - p_old) - 2;
		*((unsigned short*)p_old) = host_to_be16(len);
		ch_prefers++;
	}

	for (i = 0; i < restriction_cnt; i++) {
		*p++ = 0x8C;

		p_old = p;
		p += 2;
		memcpy(p, restrictions->identifier, ETH_ALEN);
		p += ETH_ALEN;

		*p++ = restrictions->op_class_num;
		for (j = 0; j < restrictions->op_class_num; j++) {
			*p++ = restrictions->opinfo[j].op_class;
			*p++ = restrictions->opinfo[j].ch_num;
			for (k = 0; k < restrictions->opinfo[j].ch_num; k++) {
				*p++ = restrictions->opinfo[j].ch_list[k];
				*p++ = restrictions->opinfo[j].fre_separation[k];
			}
		}
		len = (unsigned short)(p - p_old) - 2;
		*((unsigned short*)p_old) = host_to_be16(len);
		restrictions += (sizeof(struct restriction_lib) + (restrictions->op_class_num * sizeof(struct restrict_info)));
	}

	if (cac_rep) {
		struct cac_completion_status_lib  *p_cac_status = NULL;
		struct cac_completion_report_opcap *p_opcap = NULL;

		*p++ = 0xAF;

		p_old = p;
		p += 2;
		*p++ = cac_rep->radio_num;

		p_cac_status = cac_rep->cac_completion_status;
		for (i = 0; i < cac_rep->radio_num; i++) {
			memcpy(p, p_cac_status->identifier, ETH_ALEN);
			p += ETH_ALEN;

			*p++ = p_cac_status->op_class;

			*p++ = p_cac_status->channel;

			*p++ = p_cac_status->cac_status;

			*p++ = p_cac_status->op_class_num;

			p_opcap = p_cac_status->opcap;
			for (j = 0; j <p_cac_status->op_class_num; j++) {
				*p++ = p_opcap->op_class;
				*p++ = p_opcap->ch_num;

				p_opcap++;
			}

			p_cac_status = (struct cac_completion_status_lib *)((char *)p_cac_status + sizeof(struct cac_completion_status_lib) +
				p_cac_status->op_class_num * sizeof(struct cac_completion_report_opcap));
		}
		len = (unsigned short)(p - p_old) - 2;
		*((unsigned short*)p_old) = host_to_be16(len);
	}
#ifdef MAP_R2
	if (cac_status_rep) {
		struct cac_allowed_channel *p_allowed_channel = NULL;
		struct cac_non_allowed_channel *p_non_allowed_channel = NULL;
		struct cac_ongoing_channel *p_ongoing_channel = NULL;
		/* CAC Status Report TLV. */
		*p++ = 0xB1;

		/* length */
		p_old = p;
		p += 2;

		/* Number of channels the Multi-AP Agent indicates is Available Channels */
		*p++ = cac_status_rep->allowed_channel_num;

		p_allowed_channel = cac_status_rep->allowed_channel;
		for (i = 0; i < cac_status_rep->allowed_channel_num; i++) {
			*p++ = p_allowed_channel->op_class;

			*p++ = p_allowed_channel->ch_num;

			*((unsigned short *)p) = host_to_be16(p_allowed_channel->cac_interval);
			p += 2;

			p_allowed_channel ++;
		}

		/* Number of class/channel pairs the Multi-AP Agent indicates
		** are on the non-occupancy list due to detection of radar
		*/
		*p++ = cac_status_rep->non_allowed_channel_num;

		p_non_allowed_channel = cac_status_rep->non_allowed_channel;
		for (i = 0; i < cac_status_rep->non_allowed_channel_num; i++) {
			*p++ = p_non_allowed_channel->op_class;

			*p++ = p_non_allowed_channel->ch_num;

			*((unsigned short *)p) = host_to_be16(p_non_allowed_channel->remain_interval);
			p += 2;

			p_non_allowed_channel++;
		}

		/* Number of class/channel pairs that have an active CAC ongoing */
		*p++ = cac_status_rep->ongoing_cac_channel_num;

		p_ongoing_channel = cac_status_rep->cac_ongoing_channel;
		for (i = 0; i < cac_status_rep->ongoing_cac_channel_num; i++) {
			*p++ = p_ongoing_channel->op_class;

			*p++ = p_ongoing_channel->ch_num;

			*p++ = p_ongoing_channel->remain_interval;
			*p++ = p_ongoing_channel->remain_interval >> 8;
			*p++ = p_ongoing_channel->remain_interval >> 16;

			p_ongoing_channel++;
		}

		len = (unsigned short)(p - p_old) - 2;
		*((unsigned short*)p_old) = host_to_be16(len);
	}
#endif //#ifdef MAP_R2


	cmd->length = (unsigned short)(p - cmd->buffer);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Channel_Selection_Rsp_Info (IN struct _1905_context* ctx,
	IN struct ch_sel_rsp_info *rsp_info, IN unsigned char rsp_cnt
#ifdef MAP_R4_SPT
	, IN struct ch_sel_rsp_info *spt_reuse_rsp_info, IN unsigned char spt_reuse_rsp_cnt
#endif
	, IN unsigned short mid
	)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	unsigned char *p = cmd->buffer;
	int i = 0;
	cmd->type = WAPP_CHANNEL_SELECTION_RSP_INFO;
	cmd->mid = mid;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if ((rsp_cnt == 0 || rsp_info == NULL)
#ifdef MAP_R4_SPT
		&&(spt_reuse_rsp_cnt == 0 || spt_reuse_rsp_info == NULL)
#endif
	) {
		LIB_DEBUG(MSG_ERROR, "%s", (rsp_cnt == 0) ? "rsp_cnt invalid value" : "rsp_info NULL pointer");
		return -1;
	}
#ifdef MAP_R4_SPT
	LIB_DEBUG(MSG_INFO, "rsp_cnt %d spt_reuse_rsp_cnt %d\n", rsp_cnt, spt_reuse_rsp_cnt);
#endif
	for (i = 0; i < rsp_cnt && rsp_info; i++) {
		/*Channel Sleection Response TLV*/
		*p++ = 0x8E;

		*p++ = 0;
		*p++ = 7;
		memcpy(p, rsp_info->radio_indentifier, ETH_ALEN);
		p += ETH_ALEN;
		*p++ = rsp_info->rsp_code;
		rsp_info++;
	}
#ifdef MAP_R4_SPT
	for (i = 0; i < spt_reuse_rsp_cnt && spt_reuse_rsp_info; i++) {
		/*spatial reuse config Response TLV*/
		/* DM: need to improve tlv no*/
		*p++ = 0xDA;

		*p++ = 0;
		*p++ = 7;
		memcpy(p, spt_reuse_rsp_info->radio_indentifier, ETH_ALEN);
		p += ETH_ALEN;
		*p++ = spt_reuse_rsp_info->rsp_code;
		spt_reuse_rsp_info++;
	}
#endif
	cmd->length = (unsigned short)(p - cmd->buffer);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}



int _1905_Set_Ap_Cap (IN struct _1905_context* ctx,
	IN struct ap_capability *cap)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (cap == NULL) {
		LIB_DEBUG(MSG_ERROR, "cap NULL pointer");
		return -1;
	}
	cmd->type = LIB_AP_CAPABILITY;
	cmd->length = sizeof(struct ap_capability);
	memcpy(cmd->buffer, (unsigned char*)cap, cmd->length);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Ap_Ht_Cap (IN struct _1905_context* ctx,
	IN struct ap_ht_capability *cap)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (cap == NULL) {
		LIB_DEBUG(MSG_ERROR, "cap NULL pointer");
		return -1;
	}
	cmd->type = LIB_AP_HT_CAPABILITY;
	cmd->length = sizeof(struct ap_ht_capability);
	memcpy(cmd->buffer, (unsigned char*)cap, cmd->length);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Ap_Vht_Cap (IN struct _1905_context* ctx,
	IN struct ap_vht_capability *cap)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (cap == NULL) {
		LIB_DEBUG(MSG_ERROR, "cap NULL pointer");
		return -1;
	}
	cmd->type = LIB_AP_VHT_CAPABILITY;
	cmd->length = sizeof(struct ap_vht_capability);
	memcpy(cmd->buffer, (unsigned char*)cap, cmd->length);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Ap_He_Cap (IN struct _1905_context* ctx,
	IN struct ap_he_capability *cap)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (cap == NULL) {
		LIB_DEBUG(MSG_ERROR, "cap NULL pointer");
		return -1;
	}
	cmd->type = LIB_AP_HE_CAPABILITY;
	cmd->length = sizeof(struct ap_he_capability);
	memcpy(cmd->buffer, (unsigned char*)cap, cmd->length);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Operbss_Cap(IN struct _1905_context* ctx,
	IN struct oper_bss_cap *cap)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (cap == NULL) {
		LIB_DEBUG(MSG_ERROR, "cap NULL pointer");
		return -1;
	}
	cmd->type = LIB_OPERBSS_REPORT;
	cmd->length = sizeof(struct oper_bss_cap) + cap->oper_bss_num * sizeof(struct op_bss_cap);
	memcpy(cmd->buffer, (unsigned char*)cap, cmd->length);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Cli_Steer_BTM_Report_Info (IN struct _1905_context* ctx,
	IN struct  cli_steer_btm_event *info)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (info == NULL) {
		LIB_DEBUG(MSG_ERROR, "info NULL pointer");
		return -1;
	}
	cmd->type = LIB_CLI_STEER_BTM_REPORT;
	cmd->length = sizeof(struct cli_steer_btm_event);
	memcpy(cmd->buffer, (unsigned char*)info, cmd->length);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Steering_Complete_Info (IN struct _1905_context* ctx)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	cmd->type = LIB_STEERING_COMPLETED;
	cmd->length = 0;
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg)) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Read_Bss_Conf_Request (IN struct _1905_context* ctx)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	cmd->type = LIB_1905_READ_BSS_CONF_REQUEST;
	cmd->length = 0;
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Read_Bss_Conf_and_Renew_legacy (IN struct _1905_context* ctx,
	IN unsigned char local_only, IN unsigned char version)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	cmd->type = LIB_1905_READ_BSS_CONF_AND_RENEW;
	cmd->length = 2;
	*(cmd->buffer) = local_only;
	*(cmd->buffer + 1) = version;
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Read_Bss_Conf_and_Renew (IN struct _1905_context *ctx, IN unsigned char local_only) {
	return _1905_Set_Read_Bss_Conf_and_Renew_legacy(ctx, local_only, 1);
}


int _1905_Set_Read_Bss_Conf_and_Renew_v2 (IN struct _1905_context *ctx, IN unsigned char local_only) {
	return _1905_Set_Read_Bss_Conf_and_Renew_legacy(ctx, local_only, 2);
}


int _1905_Set_Bss_Config(IN struct _1905_context* ctx, IN struct bss_config_info* info,
	IN enum BSS_CONFIG_OPERATION oper)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	struct manage_cmd *_manage_cmd = (struct manage_cmd*)cmd->buffer;
	unsigned char *p = NULL;
	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		goto fail;
	}

	if(info == NULL) {
		LIB_DEBUG(MSG_ERROR, "info NULL pointer");
		goto fail;
	}

	cmd->type = MANAGEMENT_1905_COMMAND;

	_manage_cmd->cmd_id = MANAGE_SET_BSS_CONF;
	_manage_cmd->len = 1 + sizeof(struct bss_config_info) + info->cnt * sizeof(struct config_info);
	p = _manage_cmd->content;

	*p++ = oper;

	memcpy(p, info, sizeof(struct bss_config_info) + info->cnt * sizeof(struct config_info));

	cmd->length = sizeof(struct manage_cmd) + _manage_cmd->len;

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		goto fail;
	}
	return 0;
fail:
	return -1;
}
void hex_dump_all_temp(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
	unsigned char *pt;
	int x;

	pt = pSrcBufVA;
	printf("%s: %p, len = %d\n",str,  pSrcBufVA, SrcBufLen);
	for (x=0; x<SrcBufLen; x++)
	{
		if (x % 16 == 0)
			printf("0x%04x : ", x);
		printf("%02x ", ((unsigned char)pt[x]));
		if (x%16 == 15) printf("\n");
	}
    printf("\n");
}

int _1905_Set_Ap_Metric_Rsp_Info (IN struct _1905_context* ctx,
	IN struct ap_metrics_info_lib *info, IN int ap_metrics_info_cnt,
	IN struct stat_info *sta_states, IN int sta_states_cnt,
	IN struct link_metrics *sta_metrics, IN int sta_metrics_cnt
#ifdef MAP_R2
	, IN struct ap_extended_metrics_lib *ap_extended_metrics, IN int ap_extended_metrics_cnt,
	IN struct radio_metrics_lib *radio_metrics, IN int radio_metrics_cnt,
	IN struct sta_extended_metrics_lib *sta_extended_metrics, IN int sta_extended_metrics_cnt,
	IN unsigned char *vs_tlv, IN unsigned int vs_len
#endif // #ifdef MAP_R2
#ifdef MAP_R3
	, IN struct assoc_wifi6_sta_status_tlv_lib *wifi6_sta
	, IN int wifi6_sta_cnt
#endif //#ifdef MAP_R3
	, IN unsigned short mid
	)
{
	unsigned short len = 0;
	struct msg *cmd = (struct msg* )cmdu_buf;
	unsigned char* p = NULL;
	unsigned char* p_old = NULL;
	unsigned char* p_old_2 = NULL;
	unsigned char ac[4] = {0x01, 0x00, 0x03, 0x02};
	int i = 0, j = 0, k=0;
	p = cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (ap_metrics_info_cnt > 0  && info == NULL) {
		LIB_DEBUG(MSG_ERROR, "info NULL pointer");
		return -1;
	}
	if ((sta_states_cnt > 0 && sta_states == NULL) || (sta_metrics_cnt > 0 && sta_metrics == NULL)) {
		LIB_DEBUG(MSG_ERROR, "%s NULL pointer", sta_states == NULL ? "stat_states" : "sta_metrics");
		return -1;
	}
	if (ap_metrics_info_cnt == 0 && sta_states_cnt == 0 && sta_metrics_cnt == 0) {
		LIB_DEBUG(MSG_ERROR, "invalid value, ap_metrics_info_cnt=0, sta_states_cnt=0, sta_metrics_cnt=0");
		return -1;
	}
	cmd->type = WAPP_AP_METRICS_RSP_INFO;
	cmd->mid = mid;
	if (ap_metrics_info_cnt < 1) {
		LIB_DEBUG(MSG_ERROR, "invalid value, ap_metrics_cnt at least 1");
		return -1;
	}
	/* one  or more ap metrics TLVs */
	for (k = 0; k < ap_metrics_info_cnt; k++) {
		/*AP metrics TLV*/
		*p++ = 0x94;
		p_old = p;

		p += 2;

		memcpy(p, info->bssid, ETH_ALEN);
		p += ETH_ALEN;
		*p++ = info->ch_util;

		*((unsigned short*)p) = host_to_be16(info->assoc_sta_cnt);
		p += sizeof(unsigned short);

		p_old_2 = p;
		*p_old_2 = 0x00;
		p++;
		for (i = 0; i < 4; i++) {
			for (j = 0; j < info->valid_esp_count; j++) {
				if (ac[i] == info->esp[j].ac) {
					*p_old_2 |= (0x01 << (7 - i));
					if (info->esp[j].ba_win_size > 7) {
						LIB_DEBUG(MSG_ERROR, "ba win size(%d) should not be larger than 7", info->esp[j].ba_win_size);
						return -1;
					}

					if (info->esp[j].format > 3) {
						LIB_DEBUG(MSG_ERROR, "format(%d) should not be larger than 3", info->esp[j].format);
						return -1;
					}
					*p++ = info->esp[j].ppdu_dur_target;
					*p++ = info->esp[j].e_air_time_fraction;
					*p++ = (info->esp[j].ba_win_size << 5) |
						(info->esp[j].format << 3) | info->esp[j].ac;

				}
			}
		}
		/*AC_BE is fixed*/
		if (((*p_old_2) & 0x80) == 0) {
			LIB_DEBUG(MSG_ERROR, "error, does not contain AC_BE %02x", *p_old_2);
			return -1;
		}
		len = (unsigned short)(p - p_old - 2);
		*((unsigned short*)p_old) = host_to_be16(len);
		info++;
	}



	/* zero or more associated sta traffic stats TLVs */
	if (sta_states != NULL) {
		for (i = 0; i < sta_states_cnt; i++) {
			/*Associated STA Traffic Stats TLV*/
			*p++ = 0xA2;
			p_old = p;

			p += 2;

			memcpy(p, sta_states[i].mac, ETH_ALEN);
			p += ETH_ALEN;


			*((unsigned int*)p) = host_to_be32(sta_states[i].bytes_sent);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(sta_states[i].bytes_received);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(sta_states[i].packets_sent);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(sta_states[i].packets_received);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(sta_states[i].tx_packets_errors);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(sta_states[i].rx_packets_errors);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(sta_states[i].retransmission_count);
			p += sizeof(unsigned int);

			len = (unsigned short)(p - p_old - 2);
			*((unsigned short*)p_old) = host_to_be16(len);
		}
	}

	/* zero or more associated sta link metrics TLVs */
	if (sta_metrics != NULL) {
		for (i = 0; i < sta_metrics_cnt; i++) {
			/*Associated STA link metrics TLV*/
			*p++ = 0x96;
			p_old = p;

			p += 2;

			memcpy(p, sta_metrics[i].mac, ETH_ALEN);
			p += ETH_ALEN;

			*p++ = 1;

			memcpy(p, sta_metrics[i].bssid, ETH_ALEN);
			p += ETH_ALEN;

			*((unsigned int*)p) = host_to_be32(sta_metrics[i].time_delta);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(sta_metrics[i].erate_downlink);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(sta_metrics[i].erate_uplink);
			p += sizeof(unsigned int);

			*p++ = sta_metrics[i].rssi_uplink;

			len = (unsigned short)(p - p_old - 2);
			*((unsigned short*)p_old) = host_to_be16(len);
		}
	}

#ifdef MAP_R2

	/* One or more AP Extended Metrics TLVs */
	if (ap_extended_metrics != NULL) {
		for (i = 0; i < ap_extended_metrics_cnt; i++) {
			/* AP Extended Metrics TLVs */
			*p++ = 0xC7;
			p_old = p;

			p += 2;

			memcpy(p, ap_extended_metrics[i].bssid, ETH_ALEN);
			p += ETH_ALEN;

			/* mapd guarantee this unit */
			*((unsigned int*)p) = host_to_be32(ap_extended_metrics[i].uc_tx);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(ap_extended_metrics[i].uc_rx);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(ap_extended_metrics[i].mc_tx);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(ap_extended_metrics[i].mc_rx);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(ap_extended_metrics[i].bc_tx);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(ap_extended_metrics[i].bc_rx);
			p += sizeof(unsigned int);

			len = (unsigned short)(p - p_old - 2);
			*((unsigned short*)p_old) = host_to_be16(len);

			//ap_extended_metrics ++;
		}
	}

	/* Zero or more Radio Metrics TLVs */
	if (radio_metrics != NULL) {
		for (i = 0; i < radio_metrics_cnt; i++) {
			/* Radio Metrics TLVs */
			*p++ = 0xC6;
			p_old = p;

			p += 2;

			memcpy(p, radio_metrics[i].identifier, ETH_ALEN);
			p += ETH_ALEN;


			*p++ = radio_metrics[i].noise;

			*p++ = radio_metrics[i].transmit;

			*p++ = radio_metrics[i].receive_self;

			*p++ = radio_metrics[i].receive_other;

			len = (unsigned short)(p - p_old - 2);
			*((unsigned short*)p_old) = host_to_be16(len);

			//radio_metrics ++;
		}
	}

	/* Zero or more Associated STA Extended Link Metrics TLVs */
	if (sta_extended_metrics) {
		struct extended_metrics_info *metric_info = NULL;
		for (i = 0; i < sta_extended_metrics_cnt; i++) {
			/* Associated STA Extended Link Metrics TLVs */
			*p++ = 0xC8;
			p_old = p;

			p += 2;

			memcpy(p, sta_extended_metrics->sta_mac, ETH_ALEN);
			p += ETH_ALEN;

			*p++ = sta_extended_metrics->extended_metric_cnt;

			metric_info = sta_extended_metrics->metric_info;
			for (j = 0; j < sta_extended_metrics->extended_metric_cnt; j ++) {
				memcpy(p, metric_info->bssid, ETH_ALEN);
				p += ETH_ALEN;

				*((unsigned int*)p) = host_to_be32(metric_info[j].last_data_ul_rate);
				p += sizeof(unsigned int);

				*((unsigned int*)p) = host_to_be32(metric_info[j].last_data_dl_rate);
				p += sizeof(unsigned int);

				*((unsigned int*)p) = host_to_be32(metric_info[j].utilization_rx);
				p += sizeof(unsigned int);

				*((unsigned int*)p) = host_to_be32(metric_info[j].utilization_tx);
				p += sizeof(unsigned int);

				//metric_info += 1;
			}

			sta_extended_metrics = (struct sta_extended_metrics_lib *)((char *)sta_extended_metrics +
				sizeof(struct sta_extended_metrics_lib) +
				sta_extended_metrics->extended_metric_cnt * sizeof(struct extended_metrics_info));

			len = (unsigned short)(p - p_old - 2);
			*((unsigned short*)p_old) = host_to_be16(len);

		}
	}

	if (vs_tlv && vs_len) {
		memcpy(p, vs_tlv, vs_len);
		p += vs_len;
	}
#endif // #ifdef MAP_R2

#ifdef MAP_R3
	/* zero or more associated wf6 sta status TLVs */
	if (wifi6_sta != NULL) {
		for (i = 0; i < wifi6_sta_cnt; i++) {
			/*Associated STA Traffic Stats TLV*/
			*p++ = 0xB0;
			p_old = p;

			p += 2;

			memcpy(p, wifi6_sta[i].mac, ETH_ALEN);
			p += ETH_ALEN;

			*p++ = wifi6_sta[i].tid_cnt;

			for(j = 0; j < 4; j++) {

				*p++ = wifi6_sta[i].status_tlv[j].tid;

				*p++ = wifi6_sta[i].status_tlv[j].tid_q_size;

			}

			len = (unsigned short)(p - p_old - 2);
			*((unsigned short*)p_old) = host_to_be16(len);
		}
	}
#endif // MAP_R3

	cmd->length = (unsigned short)(p - cmd->buffer);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, cmd->length + sizeof(struct msg)) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	}
	return 0;
}

int _1905_Set_Bh_Steer_Rsp_Info (IN struct _1905_context* ctx,
	IN struct backhaul_steer_rsp *info)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	unsigned char* p = NULL;
	unsigned short len = 13;
	unsigned short err_tlv_len = 7;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (info == NULL) {
 		LIB_DEBUG(MSG_ERROR, "info NULL pointer");
		return -1;
	}
	cmd->type = LIB_BACKHAUL_STEER_RSP;
	p = cmd->buffer;

	*p++ = 0x9F;

	*((unsigned short*)p) = host_to_be16(len);
	p += 2;

	memcpy(p, info->backhaul_mac, ETH_ALEN);
	p += ETH_ALEN;

	memcpy(p, info->target_bssid, ETH_ALEN);
	p += ETH_ALEN;

	*p++ = info->status;

	if (info->error) {/*add error code tlv*/
		*p++ = 0xA3;
		*((unsigned short*)p) = host_to_be16(err_tlv_len);
		p += 2;
		*p++ = info->error;
		memcpy(p, info->backhaul_mac, ETH_ALEN);
		p += ETH_ALEN;
	}

	cmd->length = p - cmd->buffer;
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Assoc_Sta_Link_Metric_Rsp_Info (IN struct _1905_context* ctx,
	IN unsigned char info_cnt, IN struct link_metrics *info,
	IN unsigned char *sta_mac, IN unsigned char reason
#ifdef MAP_R2
	, IN unsigned char sta_extended_metrics_cnt,
	IN struct sta_extended_metrics_lib *sta_extended_metrics
#endif
	, IN unsigned short mid
)
{
	unsigned char* p = NULL;
	unsigned char* old_p = NULL;
	unsigned short len = 0;
	int i = 0;
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (info_cnt == 0 || info == NULL) {
		LIB_DEBUG(MSG_ERROR, "%s", info_cnt <= 0 ? "info_cnt must be no less than 1" : "info NULL pointer");
		return -1;
	}
	cmd->type = LIB_ONE_ASSOC_STA_LINK_METRICS;
	cmd->mid = mid;
	p = cmd->buffer;

	/* One or more Associated STA Link Metrics TLVs */
	for (i = 0; i < info_cnt; i++) {
		/*associated sta link metrics tlv*/
		*p++ = 0x96;
		old_p = p;

		p += 2;

		memcpy(p, info[i].mac, ETH_ALEN);
		p += ETH_ALEN;

		if (reason == 0x02)
			*p++ = 0;
		else
			*p++ = 1;
		if (reason != 0x02) {
			memcpy(p, info[i].bssid, ETH_ALEN);
			p += ETH_ALEN;

			*((unsigned int*)p) = host_to_be32(info[i].time_delta);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(info[i].erate_downlink);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(info[i].erate_uplink);
			p += sizeof(unsigned int);

			*p++ = info[i].rssi_uplink;

			len = (unsigned short)(p - old_p - 2);

			*((unsigned short*)old_p) = host_to_be16(len);
		}
	}

#ifdef MAP_R2
	/* One or more Associated STA Extended Link Metrics TLVs */
	if (sta_extended_metrics) {
		int j = 0;
		struct extended_metrics_info *metric_info = NULL;
		for (i = 0; i < sta_extended_metrics_cnt; i++) {
			/* Associated STA Extended Link Metrics TLVs */
			*p++ = 0xC8;
			old_p = p;

			p += 2;

			memcpy(p, sta_extended_metrics->sta_mac, ETH_ALEN);
			p += ETH_ALEN;

			*p++ = sta_extended_metrics->extended_metric_cnt;

			metric_info = sta_extended_metrics->metric_info;
			for (j = 0; j < sta_extended_metrics->extended_metric_cnt; j ++) {
				memcpy(p, metric_info->bssid, ETH_ALEN);
				p += ETH_ALEN;

				*((unsigned int*)p) = host_to_be32(metric_info->last_data_ul_rate);
				p += sizeof(unsigned int);

				*((unsigned int*)p) = host_to_be32(metric_info->last_data_dl_rate);
				p += sizeof(unsigned int);

				*((unsigned int*)p) = host_to_be32(metric_info->utilization_rx);
				p += sizeof(unsigned int);

				*((unsigned int*)p) = host_to_be32(metric_info->utilization_tx);
				p += sizeof(unsigned int);

				metric_info += 1;
			}


			len = (unsigned short)(p - old_p - 2);
			*((unsigned short*)old_p) = host_to_be16(len);

			sta_extended_metrics = (struct sta_extended_metrics_lib *)metric_info;
		}
	}

	/* zero or one Error Code TLV (see section 17.2.36) */
	if (reason) {
		/* Error Code TLV */
		*p++ = 0xA3;

		*((unsigned short*)p) = host_to_be16(7);
		p += 2;

		*p++ =reason;

		if ((reason == 0x01 || reason == 0x02) && sta_mac) {
			memcpy(p, sta_mac, ETH_ALEN);
			p += ETH_ALEN;
		}
	}
#endif // #ifdef MAP_R2

	cmd->length = (unsigned short)(p - cmd->buffer);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Link_Metrics_Rsp_Info (IN struct _1905_context* ctx,
	IN int tx_metrics_cnt, IN struct tx_link_metrics *tx_metrics,
	IN int rx_metrics_cnt, IN struct rx_link_metrics *rx_metrics,
	IN unsigned short mid)
{
	unsigned char* p = NULL;
	unsigned char* old_p = NULL;
	unsigned short len = 0;
	int i = 0, j = 0;

	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	cmd->type = WAPP_LINK_METRICS_RSP_INFO;
	cmd->mid = mid;
	p = cmd->buffer;

	if ((tx_metrics_cnt > 0 && tx_metrics == NULL) || (rx_metrics_cnt > 0 && rx_metrics == NULL)) {
		LIB_DEBUG(MSG_ERROR, "%s NULL pointer", tx_metrics == NULL ? "tx_metrics" : "rx_metrics");
		return -1;
	}

	for (i = 0; i < tx_metrics_cnt; i++) {
		/*transmitter link metrics TLV*/
		*p++ = 0x09;

		old_p = p;

		/*skip tlvLength field*/
		p += 2;

		memcpy(p, tx_metrics->almac, ETH_ALEN);
		p += ETH_ALEN;

		memcpy(p, tx_metrics->neighbor_almac, ETH_ALEN);
		p += ETH_ALEN;

		for (j = 0; j < tx_metrics->link_pair_cnt; j++) {
			memcpy(p, tx_metrics->metrics[j].mac_address, ETH_ALEN);
			p += ETH_ALEN;
			memcpy(p, tx_metrics->metrics[j].mac_address_neighbor, ETH_ALEN);
			p += ETH_ALEN;

			*((unsigned short*)p) = host_to_be16(tx_metrics->metrics[j].intftype);
			p += sizeof(unsigned short);

			*p++ = tx_metrics->metrics[j].ieee80211_bridgeflg;

			*((unsigned int*)p) = host_to_be32(tx_metrics->metrics[j].packetErrors);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(tx_metrics->metrics[j].transmittedPackets);
			p += sizeof(unsigned int);

			*((unsigned short*)p) = host_to_be16(tx_metrics->metrics[j].macThroughputCapacity);
			p += sizeof(unsigned short);

			*((unsigned short*)p) = host_to_be16(tx_metrics->metrics[j].linkAvailability);
			p += sizeof(unsigned short);

			*((unsigned short*)p) = host_to_be16(tx_metrics->metrics[j].phyRate);
			p += sizeof(unsigned short);
		}
		len = (unsigned short)(p - old_p - 2);
		*((unsigned short*)old_p) = host_to_be16(len);
		tx_metrics++;
	}
	for (i = 0; i < rx_metrics_cnt; i++) {
		/*receiver link metrics TLV*/
		*p++ = 10;

		old_p = p;

		/*skip tlvLength field*/
		p += 2;

		memcpy(p, rx_metrics->almac, ETH_ALEN);
		p += ETH_ALEN;

		memcpy(p, rx_metrics->neighbor_almac, ETH_ALEN);
		p += ETH_ALEN;

		for (j = 0; j < rx_metrics->link_pair_cnt; j++) {
			memcpy(p, rx_metrics->metrics[j].mac_address, ETH_ALEN);
			p += ETH_ALEN;
			memcpy(p, rx_metrics->metrics[j].mac_address_neighbor, ETH_ALEN);
			p += ETH_ALEN;

			*((unsigned short*)p) = host_to_be16(rx_metrics->metrics[j].intftype);
			p += sizeof(unsigned short);

			*((unsigned int*)p) = host_to_be32(rx_metrics->metrics[j].packetErrors);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(rx_metrics->metrics[j].packetsReceived);
			p += sizeof(unsigned int);

			*p++ = rx_metrics->metrics[j].rssi;
		}
		len = (unsigned short)(p - old_p - 2);
		*((unsigned short*)old_p) = host_to_be16(len);
		rx_metrics++;
	}
	cmd->length = (unsigned short)(p - cmd->buffer);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Unassoc_Sta_Link_Metric_Rsp_Info (IN struct _1905_context* ctx,
	IN struct unlink_metrics_rsp *info)
{
	unsigned char* p = NULL;
	unsigned char* old_p = NULL;
	unsigned short len = 0;
	int i = 0;
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		goto error_exit;
	}
	if (info == NULL) {
		LIB_DEBUG(MSG_ERROR, "info NULL pointer");
		goto error_exit;
	}
	cmd->type = LIB_UNASSOC_STA_LINK_METRICS;
	p = cmd->buffer;

	/*unassociated sta link metrics response TLV*/
	*p++ = 0x98;
	old_p = p;

	p += 2;
	*p++ = info->oper_class;
	*p++ = info->sta_num;

	for (i = 0; i < info->sta_num; i++) {
		memcpy(p, info->info[i].mac, ETH_ALEN);
		p += ETH_ALEN;

		*p++ = info->info[i].ch;

		*((unsigned int*)p) = host_to_be32(info->info[i].time_delta);
		p += sizeof(unsigned int);

		*p++ = info->info[i].uplink_rssi;
	}
	len = (unsigned short)(p - old_p) - 2;
	*((unsigned short*)old_p) = host_to_be16(len);
	cmd->length =(unsigned short)(p - cmd->buffer);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}

error_exit:
	cmd->type = LIB_UNASSOC_STA_LINK_METRICS;
	cmd->length = 0;
	if (_1905_interface_ctrl_request(ctx, (char *)cmd, sizeof(struct msg) + cmd->length) < 0)
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");

	return -1;
}

int _1905_Set_Operating_Channel_Report_Info (IN struct _1905_context* ctx,
	IN struct channel_report *info
#ifdef MAP_R4_SPT
	, IN struct spt_reuse_report *spt_report
#endif
	)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (info == NULL) {
		LIB_DEBUG(MSG_ERROR, "info NULL pointer");
		return -1;
	}
#ifdef MAP_R4_SPT
	unsigned short len = 0;
#endif
	cmd->type = LIB_OPERATING_CHANNEL_REPORT;
	cmd->length = sizeof(struct channel_report) + info->ch_rep_num * sizeof(struct ch_rep_info);
	memcpy(cmd->buffer, (unsigned char*)info, cmd->length);

#ifdef MAP_R4_SPT
	if(spt_report != NULL) {
		len = sizeof(struct spt_reuse_report)+ spt_report->spt_rep_num* sizeof(struct ap_spt_reuse_resp);
		memcpy(cmd->buffer + cmd->length, (unsigned char*)spt_report, len);
		cmd->length += len;
	}
#endif
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Beacon_Metrics_Report_Info (IN struct _1905_context* ctx,
	IN struct beacon_metrics_rsp_lib *info)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	unsigned char* p = NULL;
	unsigned char* p_old = NULL;
	unsigned short len = 0;
	cmd->type = LIB_BEACON_METRICS_REPORT;
	p = cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (info == NULL) {
		LIB_DEBUG(MSG_ERROR, "info NULL pointer");
		return -1;
	}
	/*beacon metrics response tlv*/
	*p++ = 0x9A;
	p_old = p;

	p += 2;

	memcpy(p, info->sta_mac, ETH_ALEN);
	p += ETH_ALEN;

	*p++ = (info->status << 6);

	*p++ = info->bcn_rpt_num;

	memcpy(p, info->rpt, info->rpt_len);
	p += info->rpt_len;

	len = (unsigned short)(p - cmd->buffer) - 3;

	*((unsigned short*)p_old) = host_to_be16(len);

	cmd->length = len + 3;
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Sta_Notification_Info (IN struct _1905_context* ctx,
	IN struct client_association_event *info)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (info == NULL) {
		LIB_DEBUG(MSG_ERROR, "info NULL pointer");
		return -1;
	}
	cmd->type = LIB_CLIENT_NOTIFICATION;
	cmd->length = sizeof(struct client_association_event) + (info->map_assoc_evt.assoc_evt == STA_LEAVE ? 0 : info->map_assoc_evt.assoc_req_len);
	memcpy(cmd->buffer, (unsigned char*)info, cmd->length);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Bh_Ready (IN struct _1905_context* ctx, struct bh_link_info *info)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (info == NULL) {
		LIB_DEBUG(MSG_ERROR, "info NULL pointer");
		return -1;
	}
	cmd->type = LIB_MAP_BH_READY;
	cmd->length = sizeof(struct bh_link_info);
	os_memcpy(cmd->buffer, (unsigned char*)info, sizeof(struct bh_link_info));

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

#ifdef MAP_R2
int _1905_Set_Radio_Metric(IN struct _1905_context* ctx, struct radio_metrics_lib *info)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (info == NULL) {
		LIB_DEBUG(MSG_ERROR, "radio_metrics_lib NULL pointer");
		return -1;
	}
	cmd->type = LIB_RADIO_METRICS_INFO;
	cmd->length = sizeof(struct radio_metrics_lib);
	os_memcpy(cmd->buffer, (unsigned char*)info, sizeof(struct radio_metrics_lib));

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}
#endif

int _1905_Set_Wi_Bh_Link_Down (IN struct _1905_context* ctx, struct bh_link_info *info)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (info == NULL) {
		LIB_DEBUG(MSG_ERROR, "info NULL pointer");
		return -1;
	}
	cmd->type = WAPP_NOTIFY_WIRELESS_BH_LINK_DOWN;
	cmd->length = sizeof(struct bh_link_info);
	os_memcpy(cmd->buffer, (unsigned char*)info, sizeof(struct bh_link_info));

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_ch_bw_info (IN struct _1905_context* ctx, struct channel_bw_info *info)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (info == NULL) {
		LIB_DEBUG(MSG_ERROR, "info NULL pointer");
		return -1;
	}
	cmd->type = WAPP_NOTIFY_CH_BW_INFO;
	cmd->length = sizeof(struct channel_bw_info);
	os_memcpy(cmd->buffer, (unsigned char*)info, sizeof(struct channel_bw_info));

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Get_Own_Topo_Rsp (IN struct _1905_context* ctx)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	cmd->type = WAPP_GET_OWN_TOPOLOGY_RSP;
	cmd->length = 0;

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Get_Own_Switch_Status (IN struct _1905_context* ctx)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	cmd->type = GET_SWITCH_STATUS;
	cmd->length = 0;

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}


int _1905_Get_Wsc_Config (IN struct _1905_context* ctx, struct wps_get_config *config)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (config == NULL) {
		LIB_DEBUG(MSG_ERROR, "config NULL pointer");
		return -1;
	}
	cmd->type = LIB_GET_WSC_CONF;
	cmd->length = sizeof(struct wps_get_config) + sizeof(inf_info) * config->num_of_inf;
	memcpy(cmd->buffer, (char* )config,  cmd->length);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Read_1905_Tlv_Req (IN struct _1905_context* ctx, char* file_path, int len)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (file_path == NULL || len == 0) {
		LIB_DEBUG(MSG_ERROR, "%s", len == 0 ? "len must be no less than 1" : "file_path NULL pointer");
		return -1;
	}

	memset(cmdu_buf, 0, sizeof(cmdu_buf));
	cmd->type = LIB_1905_READ_1905_TLV_REQUEST;
	memcpy(cmd->buffer, file_path,  len);
	cmd->length = len;
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}


int _1905_Send_Topology_Query_Message (IN struct _1905_context* ctx, char* almac)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = TOPOLOGY_QUERY;
	request->len = 0;
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request)) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_Link_Metric_Query_Message (IN struct _1905_context* ctx,
	char* almac, unsigned char  neighbor,  char* neighbor_almac,  unsigned char metrics)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char zero_mac[ETH_ALEN] = {0};
	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (neighbor > 1) {
		LIB_DEBUG(MSG_ERROR, "neighbor should be less than 2");
		return -1;
	}
	if (metrics > 2) {
		LIB_DEBUG(MSG_ERROR, "metrics should be less than 3");
		return -1;
	}
	if (almac == NULL || (neighbor > 0 && neighbor_almac == NULL)) {
		LIB_DEBUG(MSG_ERROR, "%s NULL pointer", almac == NULL ? "almac" : "neighbor_almac");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = LINK_METRICS_QUERY;
	request->len = ETH_ALEN + sizeof(unsigned char);
	if (neighbor == 0x00) {
		memcpy(request->body, zero_mac, ETH_ALEN);
	} else {
		memcpy(request->body, neighbor_almac, ETH_ALEN);
	}
	memcpy(request->body + 6, &metrics, sizeof(unsigned char));
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + 7) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}

}

int _1905_Send_AP_Autoconfig_Search_Message(IN struct _1905_context* ctx,
	unsigned char band)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, _1905_multicast_address, ETH_ALEN);
	request->type = AP_AUTOCONFIG_SEARCH;
	request->len = sizeof(unsigned char);
	memcpy(request->body, &band, sizeof(unsigned char));
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + sizeof(unsigned char)) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_AP_autoconfig_Renew_Message (IN struct _1905_context* ctx,
	unsigned char band)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;
	request->type = AP_AUTOCONFIG_RENEW;
	request->len = sizeof(unsigned char);
	memcpy(request->body, &band, sizeof(unsigned char));
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + sizeof(unsigned char)) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}
int _1905_Send_Vendor_Specific_Message(IN struct _1905_context* ctx,
	char* almac,  char* vend_spec_tlv, unsigned short len)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (len == 0 || vend_spec_tlv == NULL) {
		LIB_DEBUG(MSG_ERROR, "%s", len == 0 ? "len must be no less than 1" : "vend_spec_tlv NULL pointer");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = VENDOR_SPECIFIC;
	request->len = len;
	memcpy(request->body, vend_spec_tlv, len);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}


int _1905_Send_AP_Capability_Query_Message (IN struct _1905_context* ctx, char* almac)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = AP_CAPABILITY_QUERY;
	request->len = 0;
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request)) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_MAP_Policy_Request_Message (IN struct _1905_context* ctx,
	char* almac, unsigned char steer_disallow_sta_cnt,  char *steer_disallow_sta_list,
	unsigned char btm_disallow_sta_cnt, char *btm_disallow_sta_list,  unsigned char radio_cnt_steer,
	struct lib_steer_radio_policy *steering_policy, unsigned char ap_rep_interval,
	unsigned char radio_cnt_metrics, struct lib_metrics_radio_policy *metrics_policy
#ifdef MAP_R2
	, unsigned char scan_rep_include, unsigned char scan_rep_policy,
	unsigned char unsuccess_assoc_policy_include, struct lib_unsuccess_assoc_policy *unsuccess_assoc_policy
#endif
	)
{
	unsigned short len = 0;
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;
	unsigned char *old_p = NULL;
	int j = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	if (steer_disallow_sta_cnt > 0 && steer_disallow_sta_list == NULL) {
		LIB_DEBUG(MSG_ERROR, "steer_disallow_sta_list NULL pointer");
		return -1;
	}
	if (btm_disallow_sta_cnt > 0 && btm_disallow_sta_list == NULL) {
		LIB_DEBUG(MSG_ERROR, "btm_disallow_sta_list NULL pointer");
		return -1;
	}
	if (radio_cnt_steer > 0 && steering_policy == NULL) {
		LIB_DEBUG(MSG_ERROR, "steering_policy NULL pointer");
		return -1;
	}
	if (radio_cnt_metrics > 0 && metrics_policy == NULL) {
		LIB_DEBUG(MSG_ERROR, "metrics_policy NULL pointer");
		return -1;
	}
	if (steer_disallow_sta_cnt == 0 && btm_disallow_sta_cnt == 0 && radio_cnt_steer == 0
		&& radio_cnt_metrics == 0) {
		LIB_DEBUG(MSG_ERROR, "invalid value, all cnt equal to 0");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = MAP_POLICY_CONFIG_REQUEST;

	if (steer_disallow_sta_cnt != 0 || btm_disallow_sta_cnt != 0 || radio_cnt_steer != 0) {
		/*steering policy tlv*/
		*p++ = 0x89;
		old_p = p;

		/*skip tlvLength field*/
		p += 2;

		*p++ = steer_disallow_sta_cnt;
		memcpy(p, steer_disallow_sta_list, steer_disallow_sta_cnt * ETH_ALEN);
		p += steer_disallow_sta_cnt * ETH_ALEN;

		*p++ = btm_disallow_sta_cnt;
		memcpy(p, btm_disallow_sta_list, btm_disallow_sta_cnt * ETH_ALEN);
		p += btm_disallow_sta_cnt * ETH_ALEN;

		*p++ = radio_cnt_steer;
		for (j = 0; j < radio_cnt_steer; j++) {
			memcpy(p, steering_policy->identifier, ETH_ALEN);
			p += ETH_ALEN;
			*p++ = steering_policy->steer_policy;
			*p++ = steering_policy->ch_util_thres;
			*p++ = steering_policy->rssi_thres;
			steering_policy++;
		}

		len = (unsigned short)(p - old_p - 2);
		*((unsigned short*)old_p) = host_to_be16(len);
	}

	if (ap_rep_interval != 0 || radio_cnt_metrics!= 0) {
		/*metric reporting policy TLV*/
		*p++ = 0x8A;

		old_p = p;

		/*skip tlvLength field*/
		p += 2;

		*p++ = ap_rep_interval;
		*p++ = radio_cnt_metrics;
		for (j = 0; j < radio_cnt_metrics; j++) {
			memcpy(p, metrics_policy->identifier, ETH_ALEN);
			p += ETH_ALEN;
			*p++ = metrics_policy->rssi_thres;
			*p++ = metrics_policy->rssi_margin;
			*p++ = metrics_policy->ch_util_thres;
			*p = ((metrics_policy->traffic_inclusion & 0x01) << 7);
			*p |= ((metrics_policy->metrics_inclusion& 0x01) << 6);
#ifdef MAP_R3
			*p |= ((metrics_policy->assoc_wf6_inclusion& 0x01) << 5);
#endif
			p++;
			metrics_policy++;
		}

		len = (unsigned short)(p - old_p - 2);
		*((unsigned short*)old_p) = host_to_be16(len);
	}

#ifdef MAP_R2
	if (scan_rep_include) {
		/*channel scan reporting policy TLV*/
		*p++ = 0xA4;
		old_p = p;

		/*skip tlvLength field*/
		p += 2;
		*p++ = scan_rep_policy;
		len = (unsigned short)(p - old_p - 2);
		*((unsigned short*)old_p) = host_to_be16(len);
	}
	if (unsuccess_assoc_policy_include) {
		/* Unsuccessful Association Policy TLV */
		*p++ = 0xC4;
		old_p = p;

		/*skip tlvLength field*/
		p += 2;

		/*   Report Unsuccessful Associations (bit 7)
		**	0: Do not report unsuccessful association attempts
		**	1: Report unsuccessful association attempts
		*/
		if (unsuccess_assoc_policy->report_switch)
			*p++ |= 0x80;

		/* report rate */
		*((unsigned int *)p) = host_to_be32(unsuccess_assoc_policy->report_rate);
		p += sizeof(unsigned int);

		len = (unsigned short)(p - old_p - 2);
		*((unsigned short*)old_p) = host_to_be16(len);
	}
#endif // #ifdef MAP_R2

	// TODO: add BSSID to Unique BSS Index mapping TLV (0x83)

	request->len = p - request->body;

	if(_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_Channel_Preference_Query_Message (IN struct _1905_context* ctx, char* almac)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = CHANNEL_PREFERENCE_QUERY;
	request->len = 0;
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request)) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_Channel_Selection_Request_Message (IN struct _1905_context* ctx,
	char* almac, unsigned char ch_prefer_cnt, struct ch_prefer_lib* prefer,
	unsigned char transmit_power_cnt, struct transmit_power_limit* power_limit
#ifdef MAP_R4_SPT
	,unsigned char spt_reuse_count, struct ap_spt_reuse_req *spt_reuse_cap
#endif
	)
{
	unsigned short len = 0;
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;
	unsigned char *old_p = NULL;
	int i = 0, j = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	if (ch_prefer_cnt > 0 && prefer == NULL) {
		LIB_DEBUG(MSG_ERROR, "invalid value ch_prefer_cnt=%d, prefer=NULL", ch_prefer_cnt);
		return -1;
	}
	if (transmit_power_cnt > 0 && power_limit == NULL) {
		LIB_DEBUG(MSG_ERROR, "invalid value transmit_power_cnt=%d, power_limit=NULL", transmit_power_cnt);
		return -1;
	}
	if (ch_prefer_cnt == 0 && transmit_power_cnt == 0
#ifdef MAP_R4_SPT
			&& spt_reuse_count == 0
#endif
	) {
		LIB_DEBUG(MSG_ERROR, "error, all cnt equal to 0");
		return -1;
	}

#ifdef MAP_R4_SPT
		if (spt_reuse_count > 0 && spt_reuse_cap == NULL) {
		LIB_DEBUG(MSG_ERROR, "invalid value spt_reuse_count=%d, spt_reuse_cap=NULL", transmit_power_cnt);
		return -1;
	}
	LIB_DEBUG(MSG_ERROR, "value spt_reuse_count=%d, prefer=NULL\n", spt_reuse_count);
#endif
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = CHANNEL_SELECTION_REQUEST;

	for (i = 0; i < ch_prefer_cnt; i++) {
		/*channel preference TLV*/
		*p++ = 0x8B;
		old_p = p;

		/*skip tlvLength field*/
		p += 2;
		memcpy(p, prefer->identifier, ETH_ALEN);
		p += ETH_ALEN;
		*p++ = prefer->op_class_num;
		for (j = 0; j < prefer->op_class_num; j++) {
			*p++ = prefer->opinfo[j].op_class;
			*p++ = prefer->opinfo[j].ch_num;
			memcpy(p, prefer->opinfo[j].ch_list, prefer->opinfo[j].ch_num);
			p += prefer->opinfo[j].ch_num;
			*p = 0;
			*p |= (unsigned char)((prefer->opinfo[j].perference & 0x0F) << 4);
			*p |= (unsigned char)(prefer->opinfo[j].reason & 0x0F);
			p++;
		}
		len = (unsigned short)(p - old_p - 2);
		*((unsigned short*)old_p) = host_to_be16(len);
		prefer++;

	}

	for (i = 0; i < transmit_power_cnt; i++) {
		/*transmit power limit TLV*/
		*p++ = 0x8D;

		*p++ = 0;
		*p++ = 0x07;

		memcpy(p, power_limit->identifier, ETH_ALEN);
		p += ETH_ALEN;

		*p++ = power_limit->transmit_power_limit;

		power_limit++;
	}
#ifdef MAP_R4_SPT
	for(i = 0; i < spt_reuse_count; i++)
	{/* Spatial Reuse Request TLV*/
		/* DM: need to improve tlv no*/
		*p++ = 0xD8;
		old_p = p;
		/*skip tlvLength field*/
		p += 2;

		memcpy(p, spt_reuse_cap->identifier, ETH_ALEN);
		p += ETH_ALEN;
		*p++ = spt_reuse_cap->bss_color & 0x3F;
		*p++ = (spt_reuse_cap->hesiga_spa_reuse_val_allowed? 0x10 : 0x00) |
				(spt_reuse_cap->srg_info_valid? 0x08 : 0x00) |
				(spt_reuse_cap->nonsrg_offset_valid ? 0x04 : 0x00) |
				(spt_reuse_cap->psr_disallowed ? 0x01 : 0x00);

		if(spt_reuse_cap->nonsrg_offset_valid)
			*p++ = spt_reuse_cap->nonsrg_obsspd_max_offset;
		else
			p++;

		if(spt_reuse_cap->srg_info_valid){
			*p++ = spt_reuse_cap->srg_obsspd_min_offset;
			*p++ = spt_reuse_cap->srg_obsspd_max_offset;
			memcpy(p, spt_reuse_cap->srg_bss_color_bitmap, 8);
			p += 8;
			memcpy(p, spt_reuse_cap->srg_partial_bssid_bitmap, 8);
			p += 8;
		}
		else
			p += 18;
		*p++ = 0;
		*p++ = 0;
		len = (unsigned short)(p - old_p - 2);
		*((unsigned short*)old_p) = host_to_be16(len);
		spt_reuse_cap++;
	}
#endif
	request->len = p - request->body;

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}

}

int _1905_Send_Client_Capability_Query_Message (IN struct _1905_context* ctx,
	char* almac, unsigned char *bssid, unsigned char *sta_mac)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	if (bssid == NULL || sta_mac == NULL) {
		LIB_DEBUG(MSG_ERROR, "%s NULL pointer", bssid == NULL ? "bssid" : "sta_mac");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = CLIENT_CAPABILITY_QUERY;
	request->len = 2 * ETH_ALEN;
	memcpy(request->body, bssid, ETH_ALEN);
	memcpy(request->body + ETH_ALEN, sta_mac, ETH_ALEN);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + 2 * ETH_ALEN) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_AP_Metrics_Query_Message (IN struct _1905_context* ctx,
	char* almac, unsigned char bssid_cnt, unsigned char *bssid_list
#ifdef MAP_R2
	, unsigned char radio_cnt, unsigned char *identifier_list
#endif
	)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	unsigned char* p = NULL, *p_old = NULL;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	if (bssid_cnt == 0 || bssid_list == NULL) {
		LIB_DEBUG(MSG_ERROR, "%s", bssid_cnt == 0 ? "bssid_cnt must be no less than 1" : "bssid_list NULL pointer");
		return -1;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = AP_LINK_METRICS_QUERY;

	p = request->body;

	/* One AP metric query TLV */
	*p++ = 0x93;

	p_old = p;
	p += 2;

	*p++ = bssid_cnt;

	memcpy(p, bssid_list, bssid_cnt * ETH_ALEN);
	p += bssid_cnt * ETH_ALEN;

	*((unsigned short*)p_old) = host_to_be16(p - p_old - 2);

#ifdef MAP_R2
	if (identifier_list){
		int i = 0;
		unsigned char *p_identifier_list = NULL;

		p_identifier_list = identifier_list;
		/* Zero or more AP Radio Identifier TLVs */
		for (i = 0; i < radio_cnt; i ++) {
			*p++ = 0x82;

			p_old = p;

			*(unsigned short *)p = ETH_ALEN;
			p += 2;

			memcpy(p, p_identifier_list, ETH_ALEN);
			p += ETH_ALEN;
			p_identifier_list += ETH_ALEN;

			*((unsigned short*)p_old) = host_to_be16(p - p_old - 2);
		}
	}
#endif


	request->len = (unsigned short)(p - request->body);


	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_Associated_STA_Link_Metrics_Query_Message (IN struct _1905_context* ctx,
	char* almac, unsigned char *sta_mac)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	unsigned char* p = NULL;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	if (sta_mac == NULL) {
		LIB_DEBUG(MSG_ERROR, "sta_mac NULL pointer");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = ASSOC_STA_LINK_METRICS_QUERY;
	p = request->body;

	/*sta mac address tlv*/
	*p++ = 0x95;

	*p++ = 0x00;
	*p++ = 0x06;

	memcpy(p, sta_mac, ETH_ALEN);
	request->len = 9;

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_Unassociated_STA_Link_Metrics_Query_Message (IN struct _1905_context* ctx,
	char* almac, struct unassoc_sta_link_metrics_query *query)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	unsigned char* p = NULL, *p_old = NULL;
	unsigned char i = 0;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	if (query == NULL) {
		LIB_DEBUG(MSG_ERROR, "query NULL pointer");
		return -1;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;

	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = UNASSOC_STA_LINK_METRICS_QUERY;

	p = request->body;

	/*unassociated sta link metrics query tlv*/
	*p++ = 0x97;

	p_old = p;
	p += 2;

	*p++ = query->op_class;
	*p++ = query->ch_num;

	for (i = 0; i < query->ch_num; i++) {
		*p++ = query->unassoc_link_query_sub[i].channel;
		*p++ = query->unassoc_link_query_sub[i].sta_num;
		memcpy(p, query->unassoc_link_query_sub[i].sta_mac, query->unassoc_link_query_sub[i].sta_num * ETH_ALEN);
		p += query->unassoc_link_query_sub[i].sta_num * ETH_ALEN;
	}

	request->len = (unsigned short)(p - request->body);

	*((unsigned short*)p_old) = host_to_be16((request->len - 3));

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}

}

int _1905_Send_Beacon_Metrics_Query_Message (IN struct _1905_context* ctx, char* almac,
	struct beacon_metrics_query *query)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	unsigned char* p = NULL, *p_old = NULL;
	unsigned char i = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	if (query == NULL) {
		LIB_DEBUG(MSG_ERROR, "query NULL pointer");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = BEACON_METRICS_QUERY;

	p = request->body;

	/*beacon metrics query tlv*/
	*p++ = 0x99;

	p_old = p;
	p += 2;

	memcpy(p, query->sta_mac, ETH_ALEN);
	p += ETH_ALEN;

	*p++ = query->oper_class;
	*p++ = query->ch;

	memcpy(p, query->bssid, ETH_ALEN);
	p += ETH_ALEN;

	*p++ = query->rpt_detail_val;
	*p++ = query->ssid_len;

	memcpy(p, query->ssid, query->ssid_len);
	p += query->ssid_len;

	if (query->ch != 255) {
		query->ap_ch_rpt_num = 0;
	}
	*p++ = query->ap_ch_rpt_num;
	for (i = 0; i < query->ap_ch_rpt_num; i++) {
		*p++ = query->rpt[i].ch_rpt_len;
		*p++ = query->rpt[i].oper_class;
		memcpy(p, query->rpt[i].ch_list, query->rpt[i].ch_rpt_len - 1);
		p += (query->rpt[i].ch_rpt_len - 1);
	}

	if (query->rpt_detail_val != 1) {
		query->elemnt_num = 0;
	}
	*p++ = query->elemnt_num;
	memcpy(p, query->elemnt_list, query->elemnt_num);
	p += query->elemnt_num;

	request->len = (unsigned short)(p - request->body);

	*((unsigned short*)p_old) = host_to_be16((request->len - 3));
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_Combined_Infrastructure_Metrics_Message (IN struct _1905_context* ctx,
	char* almac, unsigned char ap_metrics_cnt, struct GNU_PACKED ap_metrics_info_lib *metrics_info,
	unsigned char bh_link_cnt, struct tx_link_metrics* tx_metrics_bh_ap,
	struct tx_link_metrics *tx_metrics_bh_sta, struct rx_link_metrics* rx_metrics_bh_ap,
	struct rx_link_metrics *rx_metrics_bh_sta)
{
	unsigned short len = 0;
	struct msg* cmd = (struct msg*)cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;
	unsigned char *old_p = NULL;
	unsigned char *old_p_2 = NULL;
	unsigned char ac[4] = {0x01, 0x00, 0x03, 0x02};
	int i = 0, j = 0, k = 0;


	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	if (ap_metrics_cnt > 0 && metrics_info == NULL) {
		LIB_DEBUG(MSG_ERROR, "metrics_info NULL pointer");
		return -1;
	}
	if (bh_link_cnt > 0 && (tx_metrics_bh_ap == NULL || tx_metrics_bh_sta == NULL
		|| rx_metrics_bh_ap == NULL || rx_metrics_bh_sta == NULL)) {
		LIB_DEBUG(MSG_ERROR, "error,NULL pointer");
		return -1;
	}
	if (ap_metrics_cnt == 0 && bh_link_cnt == 0) {
		LIB_DEBUG(MSG_ERROR, "error, ap_metrics_cnt and bh_link_cnt could not be 0 at same time");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = COMBINED_INFRASTRUCTURE_METRICS;

	for (i = 0; i < ap_metrics_cnt; i++) {
		/*ap metrics TLV*/
		*p++ = 0x94;
		old_p = p;

		/*skip tlvLength field*/
		p += 2;
		memcpy(p, metrics_info->bssid, ETH_ALEN);
		p += ETH_ALEN;
		*p++ = metrics_info->ch_util;

		*((unsigned short*)p) = host_to_be16(metrics_info->assoc_sta_cnt);
		p += sizeof(unsigned short);

		old_p_2 = p;
		*old_p_2 = 0x00;
		p++;
		for (k = 0; k < 4; k++) {
			for (j = 0; j < metrics_info->valid_esp_count; j++) {
				if (ac[k] == metrics_info->esp[j].ac) {
					*old_p_2 |= (0x01 << (7 - k));
					if (metrics_info->esp[j].ba_win_size > 7) {
						LIB_DEBUG(MSG_ERROR, "ba win size(%d) should not be larger than 7", metrics_info->esp[j].ba_win_size);
						return -1;
					}

					if (metrics_info->esp[j].format > 3) {
						LIB_DEBUG(MSG_ERROR, "format(%d) should not be larger than 3", metrics_info->esp[j].format);
						return -1;
					}
					*p++ = metrics_info->esp[j].ppdu_dur_target;
					*p++ = metrics_info->esp[j].e_air_time_fraction;
					*p++ = (metrics_info->esp[j].ba_win_size << 5) |
						(metrics_info->esp[j].format << 3) | metrics_info->esp[j].ac;
				}
			}
		}
		/*AC_BE is fixed*/
		if (((*old_p_2) & 0x80) == 0) {
			LIB_DEBUG(MSG_ERROR, "error, does not contain AC_BE %02x", *old_p_2);
			return -1;
		}
		len = (unsigned short)(p - old_p - 2);
		*((unsigned short*)old_p) = host_to_be16(len);
		metrics_info++;
	}

	for (i = 0; i < bh_link_cnt; i++) {
		/*transmitter link metrics TLV*/
		*p++ = 0x09;

		old_p = p;

		/*skip tlvLength field*/
		p += 2;

		memcpy(p, tx_metrics_bh_ap->almac, ETH_ALEN);
		p += ETH_ALEN;

		memcpy(p, tx_metrics_bh_ap->neighbor_almac, ETH_ALEN);
		p += ETH_ALEN;

		for (j = 0; (j < tx_metrics_bh_ap->link_pair_cnt) && (j < MAX_LINK_NUM); j++) {
			memcpy(p, tx_metrics_bh_ap->metrics[j].mac_address, ETH_ALEN);
			p += ETH_ALEN;
			memcpy(p, tx_metrics_bh_ap->metrics[j].mac_address_neighbor, ETH_ALEN);
			p += ETH_ALEN;

			*((unsigned short*)p) = host_to_be16(tx_metrics_bh_ap->metrics[j].intftype);
			p += sizeof(unsigned short);

			*p++ = tx_metrics_bh_ap->metrics[j].ieee80211_bridgeflg;

			*((unsigned int*)p) = host_to_be32(tx_metrics_bh_ap->metrics[j].packetErrors);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(tx_metrics_bh_ap->metrics[j].transmittedPackets);
			p += sizeof(unsigned int);

			*((unsigned short*)p) = host_to_be16(tx_metrics_bh_ap->metrics[j].macThroughputCapacity);
			p += sizeof(unsigned short);

			if (tx_metrics_bh_ap->metrics[j].linkAvailability > 100) {
				LIB_DEBUG(MSG_ERROR, "tx_metrics_bh_ap linkAvailability(%d) should not be larger than 100", tx_metrics_bh_ap->metrics[j].linkAvailability);
				/*
				return -1;
				*/
			}
			*((unsigned short*)p) = host_to_be16(tx_metrics_bh_ap->metrics[j].linkAvailability);
			p += sizeof(unsigned short);

			*((unsigned short*)p) = host_to_be16(tx_metrics_bh_ap->metrics[j].phyRate);
			p += sizeof(unsigned short);
		}
		len = (unsigned short)(p - old_p - 2);
		*((unsigned short*)old_p) = host_to_be16(len);
		tx_metrics_bh_ap++;

		/*transmitter link metrics TLV*/
		*p++ = 0x09;

		old_p = p;

		/*skip tlvLength field*/
		p += 2;

		memcpy(p, tx_metrics_bh_sta->almac, ETH_ALEN);
		p += ETH_ALEN;

		memcpy(p, tx_metrics_bh_sta->neighbor_almac, ETH_ALEN);
		p += ETH_ALEN;

		for (j = 0; j < tx_metrics_bh_sta->link_pair_cnt; j++) {
			memcpy(p, tx_metrics_bh_sta->metrics[j].mac_address, ETH_ALEN);
			p += ETH_ALEN;
			memcpy(p, tx_metrics_bh_sta->metrics[j].mac_address_neighbor, ETH_ALEN);
			p += ETH_ALEN;

			*((unsigned short*)p) = host_to_be16(tx_metrics_bh_sta->metrics[j].intftype);
			p += sizeof(unsigned short);

			*p++ = tx_metrics_bh_sta->metrics[j].ieee80211_bridgeflg;

			*((unsigned int*)p) = host_to_be32(tx_metrics_bh_sta->metrics[j].packetErrors);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(tx_metrics_bh_sta->metrics[j].transmittedPackets);
			p += sizeof(unsigned int);

			*((unsigned short*)p) = host_to_be16(tx_metrics_bh_sta->metrics[j].macThroughputCapacity);
			p += sizeof(unsigned short);

			if (tx_metrics_bh_sta->metrics[j].linkAvailability > 100) {
				LIB_DEBUG(MSG_ERROR, "tx_metrics_bh_sta linkAvailability(%d) should not be larger than 100", tx_metrics_bh_sta->metrics[j].linkAvailability);
				/*
				return -1;
				*/
			}
			*((unsigned short*)p) = host_to_be16(tx_metrics_bh_sta->metrics[j].linkAvailability);
			p += sizeof(unsigned short);

			*((unsigned short*)p) = host_to_be16(tx_metrics_bh_sta->metrics[j].phyRate);
			p += sizeof(unsigned short);
		}
		len = (unsigned short)(p - old_p - 2);
		*((unsigned short*)old_p) = host_to_be16(len);
		tx_metrics_bh_sta++;

		/*receiver link metrics TLV*/
		*p++ = 10;

		old_p = p;

		/*skip tlvLength field*/
		p += 2;

		memcpy(p, rx_metrics_bh_ap->almac, ETH_ALEN);
		p += ETH_ALEN;

		memcpy(p, rx_metrics_bh_ap->neighbor_almac, ETH_ALEN);
		p += ETH_ALEN;

		for (j = 0; j < rx_metrics_bh_ap->link_pair_cnt; j++) {
			memcpy(p, rx_metrics_bh_ap->metrics[j].mac_address, ETH_ALEN);
			p += ETH_ALEN;
			memcpy(p, rx_metrics_bh_ap->metrics[j].mac_address_neighbor, ETH_ALEN);
			p += ETH_ALEN;

			*((unsigned short*)p) = host_to_be16(rx_metrics_bh_ap->metrics[j].intftype);
			p += sizeof(unsigned short);

			*((unsigned int*)p) = host_to_be32(rx_metrics_bh_ap->metrics[j].packetErrors);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(rx_metrics_bh_ap->metrics[j].packetsReceived);
			p += sizeof(unsigned int);

			*p++ = rx_metrics_bh_ap->metrics[j].rssi;
		}
		len = (unsigned short)(p - old_p - 2);
		*((unsigned short*)old_p) = host_to_be16(len);
		rx_metrics_bh_ap++;

		/*receiver link metrics TLV*/
		*p++ = 10;

		old_p = p;

		/*skip tlvLength field*/
		p += 2;

		memcpy(p, rx_metrics_bh_sta->almac, ETH_ALEN);
		p += ETH_ALEN;

		memcpy(p, rx_metrics_bh_sta->neighbor_almac, ETH_ALEN);
		p += ETH_ALEN;

		for (j = 0; j < rx_metrics_bh_sta->link_pair_cnt; j++) {
			memcpy(p, rx_metrics_bh_sta->metrics[j].mac_address, ETH_ALEN);
			p += ETH_ALEN;
			memcpy(p, rx_metrics_bh_sta->metrics[j].mac_address_neighbor, ETH_ALEN);
			p += ETH_ALEN;

			*((unsigned short*)p) = host_to_be16(rx_metrics_bh_sta->metrics[j].intftype);
			p += sizeof(unsigned short);

			*((unsigned int*)p) = host_to_be32(rx_metrics_bh_sta->metrics[j].packetErrors);
			p += sizeof(unsigned int);

			*((unsigned int*)p) = host_to_be32(rx_metrics_bh_sta->metrics[j].packetsReceived);
			p += sizeof(unsigned int);

			*p++ = rx_metrics_bh_sta->metrics[j].rssi;
		}
		len = (unsigned short)(p - old_p - 2);
		*((unsigned short*)old_p) = host_to_be16(len);
		rx_metrics_bh_sta++;
	}
	request->len = p - request->body;

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_Client_Steering_Request_Message (IN struct _1905_context* ctx,
	char* almac, struct lib_steer_request *request,  unsigned char tbss_cnt,
	struct lib_target_bssid_info *info
#ifdef MAP_R2
	, struct lib_steer_request_R2 *request_R2,
	unsigned char tbss_cnt_R2, struct lib_target_bssid_info_R2 *info_R2
#endif
	)

{
	unsigned char *p = NULL, *p_old = NULL;
	int i = 0;
	struct msg *cmd = (struct msg*)cmdu_buf;
	struct _1905_cmdu_request* cmdu_request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		goto fail;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		goto fail;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(cmdu_request->dest_al_mac, almac, ETH_ALEN);
	cmdu_request->type = CLIENT_STEERING_REQUEST;
	p = cmdu_request->body;

	/* One Steering Request TLV in R1 Agent */
	if (request) {
		if ((request->mode & 0xFE) || (request->disassoc_imminent & 0xFE) || (request->abridged & 0xFE)) {
			LIB_DEBUG(MSG_ERROR, "mode(%d) disassoc_imminent(%d) abridged(%d) all should not be larger than 1",
				request->mode, request->disassoc_imminent, request->abridged);
			goto fail;
		}

		/*steering request tlv*/
		*p++ = 0x9B;
		p_old = p;
		p += 2;

		memcpy(p, request->bssid, ETH_ALEN);
		p += ETH_ALEN;

		*p = 0;
		*p |= (request->mode << 7);
		*p |= (request->disassoc_imminent << 6);
		*p |= (request->abridged << 5);
		p++;
		/*
		if (request->mode == 0) {
			*(unsigned short*)p = host_to_be16(request->window);
			p += sizeof(unsigned short);
		}
		*/
		*(unsigned short*)p = host_to_be16(request->window);
		p += sizeof(unsigned short);

		*(unsigned short*)p = host_to_be16(request->timer);
		p += sizeof(unsigned short);

		*p++ = request->sta_cnt;

		memcpy(p, request->sta_list, request->sta_cnt * ETH_ALEN);
		p += request->sta_cnt * ETH_ALEN;

		if (request->mode == 0) {
			if (tbss_cnt != 0) {
				tbss_cnt = 0;
				LIB_DEBUG(MSG_WARNING, "tbss_cnt must be 0 when mode equal to 0");
			}
		} else if (request->mode == 1) {
			if (tbss_cnt == 0) {
				LIB_DEBUG(MSG_ERROR, "tbss_cnt(%d) must not to set 0 when mode equal to 1", tbss_cnt);
				goto fail;
			} else {
				if (tbss_cnt != 1 && (request->sta_cnt != tbss_cnt)) {
					LIB_DEBUG(MSG_ERROR, "tbss_cnt(%d) must be 1 or equal to sta_cnt(%d)", tbss_cnt, request->sta_cnt);
					goto fail;
				}

				if (info == NULL) {
					LIB_DEBUG(MSG_ERROR, "info NULL pointer");
					goto fail;
				}
			}
		}

		*p++ = tbss_cnt;
		if (tbss_cnt > 0) {
			for (i = 0; i < tbss_cnt; i++) {
				memcpy(p, info->bssid, ETH_ALEN);
				p += ETH_ALEN;
				*p++ = info->op_class;
				*p++ = info->channel;
				info++;
			}
		}

		/* update R1 len of steering request tlv */
		*((unsigned short*)p_old) = host_to_be16(p - p_old - 2);
	}

#ifdef MAP_R2

	/* Zero or one R2 Steering Request TLV */
	if (request_R2) {

	        if ((request_R2->mode & 0xFE) || (request_R2->disassoc_imminent & 0xFE) || (request_R2->abridged & 0xFE)) {
			LIB_DEBUG(MSG_ERROR, "mode(%d) disassoc_imminent(%d) abridged(%d) all should not be larger than 1",
				request_R2->mode, request_R2->disassoc_imminent, request_R2->abridged);
			goto fail;
	        }

		/*R2 Steering Request TLV*/
		*p++ = 0xC3;
		p_old = p;
		p += 2;

		memcpy(p, request_R2->bssid, ETH_ALEN);
		p += ETH_ALEN;

		*p = 0;
		*p |= (request_R2->mode << 7);
		*p |= (request_R2->disassoc_imminent << 6);
		*p |= (request_R2->abridged << 5);
		p++;
		/*
		if (request->mode == 0) {
			*(unsigned short*)p = host_to_be16(request->window);
			p += sizeof(unsigned short);
		}
		*/
		*(unsigned short*)p = host_to_be16(request_R2->window);
		p += sizeof(unsigned short);

		*(unsigned short*)p = host_to_be16(request_R2->timer);
		p += sizeof(unsigned short);

		*p++ = request_R2->sta_cnt;

		memcpy(p, request_R2->sta_list, request_R2->sta_cnt * ETH_ALEN);
		p += request_R2->sta_cnt * ETH_ALEN;

		if (request_R2->mode == 0) {
			if (tbss_cnt_R2 != 0) {
				tbss_cnt_R2 = 0;
				LIB_DEBUG(MSG_WARNING, "tbss_cnt_R2 must be 0 when mode equal to 0");
			}
		} else if (request_R2->mode == 1) {
			if (tbss_cnt_R2 == 0) {
				LIB_DEBUG(MSG_ERROR, "tbss_cnt_R2(%d) must not to set 0 when mode equal to 1", tbss_cnt_R2);
				goto fail;
			} else {
				if (tbss_cnt_R2 != 1 && (request_R2->sta_cnt != tbss_cnt_R2)) {
					LIB_DEBUG(MSG_ERROR, "tbss_cnt_R2(%d) must be 1 or equal to sta_cnt(%d)", tbss_cnt_R2, request_R2->sta_cnt);
					goto fail;
				}

				if (info_R2 == NULL) {
					LIB_DEBUG(MSG_ERROR, "info_R2 NULL pointer");
					goto fail;
				}
			}
		}

		*p++ = tbss_cnt_R2;
		if (tbss_cnt_R2 > 0) {
			for (i = 0; i < tbss_cnt_R2; i++) {
				memcpy(p, info_R2->bssid, ETH_ALEN);
				p += ETH_ALEN;
				*p++ = info_R2->op_class;
				*p++ = info_R2->channel;
				*p++ = info_R2->reason_code;
				info_R2++;
			}
		}

		/* update R2 len of steering request tlv */
		*((unsigned short*)p_old) = host_to_be16(p - p_old - 2);

	}
#endif // #ifdef MAP_R2

	cmdu_request->len = (unsigned short)(p - cmdu_request->body);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + cmdu_request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		goto fail;
	}
	return 0;
fail:
	return -1;
}

int _1905_Send_Client_Association_Control_Request_Message (IN struct _1905_context* ctx,
	char* almac, unsigned char *bssid,  unsigned char control,
	unsigned short valid_time, unsigned char sta_cnt, unsigned char *sta_list)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	unsigned char* p = NULL, *p_old = NULL;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	if (bssid == NULL) {
		LIB_DEBUG(MSG_ERROR, "bssid NULL pointer");
		return -1;
	}
	if(sta_cnt < 1 || (sta_cnt >= 1 && sta_list == NULL)) {
		LIB_DEBUG(MSG_ERROR, "%s", sta_cnt < 1 ? "sta_cnt must be no less than 1" : "sta_list NULL pointer");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = CLIENT_ASSOC_CONTROL_REQUEST;
	p = request->body;

	/*client assoc control request tlv*/
	*p++ = 0x9D;

	p_old = p;
	p += 2;

	memcpy(p, bssid, ETH_ALEN);
	p += ETH_ALEN;

	*p++ = control;
	valid_time = host_to_be16(valid_time);
	memcpy(p, &valid_time, sizeof(unsigned short));
	p += 2;
	*p++ = sta_cnt;
	memcpy(p, sta_list, sta_cnt * ETH_ALEN);

	p += sta_cnt * ETH_ALEN;

	request->len = (unsigned short)(p - request->body);

	*((unsigned short*)p_old) = host_to_be16((request->len - 3));

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_Higher_Layer_Data_Message (IN struct _1905_context* ctx, char* almac,
	unsigned char protocol, unsigned short len, unsigned char *payload)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	unsigned char* p = NULL, *p_old = NULL;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	if (len > 0 && payload == NULL) {
		LIB_DEBUG(MSG_ERROR, "payload NULL pointer");
		return -1;
	}
	if (len >= (MAX_LIBBUF_LEN - sizeof(struct msg) - sizeof(struct _1905_cmdu_request)) - 4) {
		LIB_DEBUG(MSG_ERROR, "payload len too long");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = HIGHER_LAYER_DATA_MESSAGE;
	p = request->body;

	/*higher layer data tlv*/
	*p++ = 0xA0;

	p_old = p;
	p += 2;

	*p++ = protocol;
	memcpy(p, payload, len);
	p += len;

	request->len = (unsigned short)(p - request->body);

	*((unsigned short*)p_old) = host_to_be16((request->len - 3));

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_Backhaul_Steering_Request_Message (IN struct _1905_context* ctx, char* almac,
	unsigned char *sta_mac, unsigned char *bssid, unsigned char opclass, unsigned char channel)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	unsigned char* p = NULL, *p_old = NULL;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	if (sta_mac == NULL || bssid == NULL) {
		LIB_DEBUG(MSG_ERROR, "%s NULL pointer", sta_mac == NULL ? "sta_mac" : "bssid");
		return -1;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = BACKHAUL_STEERING_REQUEST;
	p = request->body;

	/*backhaul steering request tlv*/
	*p++ = 0x9E;
	p_old = p;
	p += 2;

	memcpy(p, sta_mac, ETH_ALEN);
	p += ETH_ALEN;

	memcpy(p, bssid, ETH_ALEN);
	p += ETH_ALEN;

	*p++ = opclass;
	*p++ = channel;

	request->len = (unsigned short)(p - request->body);

	*((unsigned short*)p_old) = host_to_be16((request->len - 3));
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}


#ifdef MAP_R2

int _1905_Set_CAC_Cap (IN struct _1905_context* ctx,
	IN struct cac_capability_lib * cac_caps, IN int len)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (cac_caps == NULL) {
		LIB_DEBUG(MSG_ERROR, "cac_caps NULL pointer");
		return -1;
	}

	memset(cmdu_buf, 0, sizeof(cmdu_buf));
	cmd->type = LIB_CAC_CAPAB;
	cmd->length = len;

	memcpy(cmd->buffer, cac_caps,  len);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}



int _1905_Set_R2_AP_Cap(IN struct _1905_context* ctx, struct ap_r2_capability *info)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (info == NULL) {
		LIB_DEBUG(MSG_ERROR, "r2_cap NULL pointer");
		return -1;
	}
	cmd->type = LIB_R2_AP_CAP;
	cmd->length = sizeof(struct ap_r2_capability);
	os_memcpy(cmd->buffer, (unsigned char*)info, cmd->length);


	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}


int _1905_Set_Metric_collection_interval(IN struct _1905_context* ctx, unsigned int interval)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	cmd->type = LIB_METRIC_REP_INTERVAL_CAP;
	cmd->length = sizeof(interval);
	os_memcpy(cmd->buffer, (unsigned char *)&interval, sizeof(interval));

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}


int _1905_Set_AP_Extened_Metric(IN struct _1905_context* ctx, struct ap_extended_metrics_lib *info)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (info == NULL) {
		LIB_DEBUG(MSG_ERROR, "ap_extended_metrics NULL pointer");
		return -1;
	}
	cmd->type = LIB_AP_EXTENDED_METRICS_INFO;
	cmd->length = sizeof(struct ap_extended_metrics_lib);
	os_memcpy(cmd->buffer, (unsigned char*)info, sizeof(struct ap_extended_metrics_lib));

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_Assoc_Sta_Extended_Link_Metrics(IN struct _1905_context* ctx, struct sta_extended_metrics_lib *info)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (info == NULL) {
		LIB_DEBUG(MSG_ERROR, "sta_extended_metrics_lib NULL pointer");
		return -1;
	}
	cmd->type = LIB_ASSOC_STA_EXTENDED_LINK_METRICS;
	cmd->length = sizeof(struct sta_extended_metrics_lib) + info->extended_metric_cnt * sizeof(struct sta_extended_metrics_lib);
	os_memcpy(cmd->buffer, (unsigned char*)info, cmd->length);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}


/*channel scan feature*/
int _1905_Set_Channel_Scan_Cap (IN struct _1905_context* ctx,
	IN struct scan_capability_lib *scan_caps)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (scan_caps == NULL) {
		LIB_DEBUG(MSG_ERROR, "scan_caps=NULL");
		return -1;
	}

	//cmd->type = WAPP_CHANNEL_SCAN_CAP;
	cmd->type = LIB_CHANNEL_SCAN_CAPAB;
	cmd->length = sizeof(struct scan_capability_lib) + scan_caps->op_class_num * sizeof(struct scan_opcap);
	memcpy(cmd->buffer, (unsigned char*)scan_caps, cmd->length);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}


int _1905_Send_Channel_Scan_Request_Message (
	IN struct _1905_context* ctx, char* almac,
	IN unsigned char fresh_scan, IN int radio_cnt,
	IN struct scan_request_lib *scan_req)
{
	unsigned short len = 0;
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;
	unsigned char *old_p = NULL;
	int i = 0, j = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	if (radio_cnt > 0 && scan_req == NULL) {
		LIB_DEBUG(MSG_ERROR, "invalid value radio_cnt=%d, scan_req=NULL", radio_cnt);
		return -1;
	}
	if (radio_cnt == 0) {
		LIB_DEBUG(MSG_ERROR, "error, radio_cnt equal to 0");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = CHANNEL_SCAN_REQUEST;

	/*channel scan request tlv type*/
	*p++ = 0xA6;
	old_p = p;
	/*skip tlvLength field*/
	p += 2;
	/*fresh scan*/
	*p++ = fresh_scan;
	/*number of radio*/
	*p++ = radio_cnt;

	for (i = 0; i < radio_cnt; i++) {
		/*radio identifier*/
		memcpy(p, scan_req->identifier, ETH_ALEN);
		p += ETH_ALEN;
		/*number of operating classes*/
		*p++ = scan_req->op_class_num;
		for (j = 0; j < scan_req->op_class_num; j++) {
			/*operating class*/
			*p++ = scan_req->opcap[j].op_class;
			/*number of channels*/
			*p++ = scan_req->opcap[j].ch_num;
			/*channel list*/
			if (scan_req->opcap[j].ch_num)
				memcpy(p, scan_req->opcap[j].ch_list, scan_req->opcap[j].ch_num);
			p += scan_req->opcap[j].ch_num;
		}
		scan_req++;
	}

	len = (unsigned short)(p - old_p - 2);
	*((unsigned short*)old_p) = host_to_be16(len);
	request->len = p - request->body;

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}

}

int _1905_Send_Channel_Scan_Report_Message (
	IN struct _1905_context* ctx, char* almac,
	IN unsigned char ts_len, IN unsigned char *ts_str,
	IN int scan_res_cnt, IN unsigned char *scan_res,
	IN unsigned char *vs_tlv, IN unsigned int vs_len)
{
	int buf_len = 15360;
	unsigned char *buf = NULL;
	unsigned short len = 0;
	struct msg *cmd = NULL;
	struct _1905_cmdu_request *request = NULL;
	unsigned char *p = NULL;
	unsigned char *old_p = NULL, *p_neighbor_cnt = NULL, *p_neighbor = NULL;
	int i = 0, j = 0;
	unsigned char *psr = scan_res;
	unsigned char *psr_tmp = NULL;
	struct scan_result_lib *pscan_res_tmp = NULL, *pscan_tmp = NULL;
	unsigned short neighbor_cnt = 0;
	u32 total_len = 0, tmp = 0;

	buf = (unsigned char *)os_malloc(buf_len);
		if (buf == NULL)
			return -1;
	memset(buf, 0, buf_len);
	cmd = (struct msg *)buf;
	request = (struct _1905_cmdu_request *)cmd->buffer;
	p = request->body;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		os_free(buf);
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		os_free(buf);
		return -1;
	}
	if (ts_len > 0 && ts_str == NULL) {
		LIB_DEBUG(MSG_ERROR, "invalid value ts_len=%d, ts_str=NULL", ts_len);
		os_free(buf);
		return -1;
	}
	if (scan_res_cnt > 0 && scan_res == NULL) {
		LIB_DEBUG(MSG_ERROR, "invalid value scan_res_cnt=%d, scan_res=NULL", scan_res_cnt);
		os_free(buf);
		return -1;
	}
	if (scan_res_cnt == 0 || ts_len == 0) {
		LIB_DEBUG(MSG_ERROR, "error, scan_res_cnt or ts_len equal to 0");
		os_free(buf);
		return -1;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = CHANNEL_SCAN_REPORT;

	/*timestamp tlv type*/
	*p++ = 0xA8;
	old_p = p;
	/*skip tlvLength field*/
	p += 2;
	/*timestamp length*/
	*p++ = ts_len;
	/*timestamp*/
	memcpy(p, ts_str, ts_len);
	p += ts_len;
	len = (unsigned short)(p - old_p - 2);
	*((unsigned short*)old_p) = host_to_be16(len);

	psr_tmp = psr;
	for (i = 0; i < scan_res_cnt; i++) {
		pscan_tmp = (struct scan_result_lib *)psr_tmp;
		tmp = sizeof(struct scan_result_lib) + pscan_tmp->neighbor_cnt * sizeof(struct scan_neighbor);
		total_len += tmp;
		psr_tmp += tmp;
	}
	if (total_len > (buf_len - sizeof(struct msg) - 4 - ts_len)) {
		LIB_DEBUG(MSG_ERROR, "invalid input scan result");
		os_free(buf);
		return -1;
	}
	psr_tmp = scan_res;

	pscan_res_tmp = (struct scan_result_lib *)psr;
	for (i = 0; i < scan_res_cnt; i++) {
		j = 0;
		neighbor_cnt = 0;
next_roud:
		/*channel scan result tlv type*/
		*p++ = 0xA7;
		old_p = p;

		/*skip tlvLength field*/
		p += 2;
		/*radio identifier*/
		memcpy(p, pscan_res_tmp->identifier, ETH_ALEN);
		p += ETH_ALEN;
		/*operating class*/
		*p++ = pscan_res_tmp->op_class;
		/*channel*/
		*p++ = pscan_res_tmp->channel;
		/*scan status*/
		*p++ = pscan_res_tmp->scan_status;
		if (pscan_res_tmp->scan_status == 0) {
			/*timestamp length*/
			if (pscan_res_tmp->ts_len > TS_MAX) {
				LIB_DEBUG(MSG_ERROR, "ts_len bigger than TS_MAX!\n");
				os_free(buf);
				return -1;
			}
			*p++ = pscan_res_tmp->ts_len;
			/*timestamp*/
			memcpy(p, pscan_res_tmp->ts_str, pscan_res_tmp->ts_len);
			p += pscan_res_tmp->ts_len;
			/*utlization*/
			*p++ = pscan_res_tmp->utilization;
			/*Noise*/
			*p++ = pscan_res_tmp->noise;
			/*number of neighbors, fill it later*/
			p_neighbor_cnt = p;
			p += 2;
			for (; j < pscan_res_tmp->neighbor_cnt; j++) {
				p_neighbor = p;
				/*bssid*/
				memcpy(p, pscan_res_tmp->neighbor[j].bssid, ETH_ALEN);
				p += ETH_ALEN;
				/*ssid length*/
				if (pscan_res_tmp->neighbor[j].ssid_len > 32) {
					LIB_DEBUG(MSG_ERROR, "ssid_len bigger than 32!\n");
					os_free(buf);
					return -1;
				}
				*p++ = pscan_res_tmp->neighbor[j].ssid_len;
				/*ssid*/
				memcpy(p, pscan_res_tmp->neighbor[j].ssid, pscan_res_tmp->neighbor[j].ssid_len);
				p += pscan_res_tmp->neighbor[j].ssid_len;
				/*rssi*/
				*p++ = pscan_res_tmp->neighbor[j].rssi;
				/*channel bw length*/
				if (pscan_res_tmp->neighbor[j].bw_len > BW_MAX) {
					LIB_DEBUG(MSG_ERROR, "bw_len bigger than BW_MAX!\n");
					os_free(buf);
					return -1;
				}
				*p++ = pscan_res_tmp->neighbor[j].bw_len;
				/*channel bw*/
				memcpy(p, pscan_res_tmp->neighbor[j].bw, pscan_res_tmp->neighbor[j].bw_len);
				p += pscan_res_tmp->neighbor[j].bw_len;
				/*present*/
				*p++ = pscan_res_tmp->neighbor[j].present;
				/*neighbor channel utilization*/
				if (pscan_res_tmp->neighbor[j].present & 0x80)
					*p++ = pscan_res_tmp->neighbor[j].utilization;
				/*neighbor channel utilization*/
				if (pscan_res_tmp->neighbor[j].present & 0x40) {
					*((unsigned short *)p) = host_to_be16(pscan_res_tmp->neighbor[j].sta_cnt);
					p += 2;
				}

				/*if one tlv exceeds the tlv length limiation*/
				if ((p - old_p - 7) > 1467) {
					printf("try another tlv, %p %p\n", p, old_p);
					p = p_neighbor;
					*((unsigned int *)p) = host_to_be32(pscan_res_tmp->agg_scan_dur);
					p += 4;
					/*scan type*/
					*p++ = pscan_res_tmp->scan_type;
					*((unsigned short *)p_neighbor_cnt) = host_to_be16(neighbor_cnt);
					len = (unsigned short)(p - old_p - 2);
					*((unsigned short*)old_p) = host_to_be16(len);
					neighbor_cnt = 0;

					goto next_roud;
				}
				neighbor_cnt++;
			}
			*((unsigned short *)p_neighbor_cnt) = host_to_be16(neighbor_cnt);
			/*aggregate scan duration*/
			*((unsigned int *)p) = host_to_be32(pscan_res_tmp->agg_scan_dur);
			p += 4;
			/*scan type*/
			*p++ = pscan_res_tmp->scan_type;
		}

		len = (unsigned short)(p - old_p - 2);
		*((unsigned short*)old_p) = host_to_be16(len);
		psr_tmp += sizeof(struct scan_result_lib)+ pscan_res_tmp->neighbor_cnt*sizeof(struct scan_neighbor);

		pscan_res_tmp = (struct scan_result_lib *)psr_tmp;//(struct scan_result_lib *)(psr + len + 3);
	}

	if (vs_tlv && vs_len) {
		memcpy(p, vs_tlv, vs_len);
		p += vs_len;
	}

	request->len = p - request->body;
	printf("request->len %d\n", request->len);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		os_free(buf);
		return -1;
	} else {
		os_free(buf);
		return 0;
	}

}


int _1905_Send_Tunneled_Message (IN struct _1905_context* ctx, char* almac,
	IN struct tunneled_message_lib * tunneled_msg)
{
	unsigned short len = 0;
	struct msg *cmd = (struct msg*)cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;
	int i = 0;
	struct tunneled_msg_tlv *p_payload = NULL;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}
	if (tunneled_msg == NULL) {
		LIB_DEBUG(MSG_ERROR, "tunneled_msg NULL pointer");
		return -1;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = TUNNELED_MESSAGE;

	/* one Source Info TLV*/
	/* tlv type 0xC0 */
	*p++ = 0xC0;

	/* tlv lenth 2 octets, value 2 */
	len = ETH_ALEN;
	*((unsigned short*)p) = host_to_be16(len);
	p += 2;

	/* The MAC address of the device that generated the message */
	memcpy(p, tunneled_msg->sta_mac, ETH_ALEN);
	p += ETH_ALEN;

	/* One Tunneled Message Type TLV */
	/* tlv type 0xC1 */
	*p++ = 0xC1;

	/* tlv lenth 2 octets, value 1 */
	len = 1;
	*((unsigned short*)p) = host_to_be16(len);
	p += 2;

	/* Tunneled protocol type 1 octet */
	*p++ = tunneled_msg->proto_type;

	/* One or more Tunneled TLVs */
	p_payload = tunneled_msg->tunneled_msg_tlv;
	for (i = 0; i < tunneled_msg->num_tunneled_tlv; i ++) {
		/* tlv type 0xC2 */
		*p++ = 0xC2;

		/* tlv lenth 2 octets */
		len = (unsigned short)(p_payload->payload_len);
		*((unsigned short*)p) = host_to_be16(len);
		p += 2;

		memcpy(p ,p_payload->payload, p_payload->payload_len);
		p += p_payload->payload_len;
		p_payload = (struct tunneled_msg_tlv *)(p_payload->payload + p_payload->payload_len);
	}

	request->len = (unsigned short)(p - request->body);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}



int _1905_Send_Assoc_Status_Notification_Message (IN struct _1905_context* ctx,
	char* almac, IN struct assoc_notification_lib *assoc_notification)
{
	unsigned char all_agent_mac[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	unsigned short len = 0;
	struct msg *cmd = (struct msg*)cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;
	unsigned char *old_p = NULL;
	int i = 0, j = 0;
	struct assoc_notification_tlv *notify_tlv = NULL;
	struct assoc_status *p_assoc_status = NULL;


	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (assoc_notification == NULL) {
		LIB_DEBUG(MSG_ERROR, "assoc_notification NULL pointer");
		return -1;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;
	if (almac == NULL) {
		memcpy(request->dest_al_mac, all_agent_mac, ETH_ALEN);
	}
	else {
		memcpy(request->dest_al_mac, almac, ETH_ALEN);
	}
	request->type = ASSOCIATION_STATUS_NOTIFICATION;

	notify_tlv = assoc_notification->notification_tlv;

	/* one or more Association Status Notification TLVs */
	for (i = 0; i < assoc_notification->assoc_notification_tlv_num; i ++) {

		/* tlv type 0xBF */
		*p++ = 0xBF;
		old_p = p;
		p += 2;

		/* Number of BSSIDs and their statuses included in this TLV */
		*p++ = notify_tlv->bssid_num;
		p_assoc_status = notify_tlv->status;

		for (j = 0; j < notify_tlv->bssid_num; j ++) {

			memcpy(p, p_assoc_status->bssid, ETH_ALEN);
			p += ETH_ALEN;

			*p++ = p_assoc_status->status;

			p_assoc_status += 1;
		}

	    	len = (unsigned short)(p - old_p - 2);
	    	*((unsigned short*)old_p) = host_to_be16(len);

		notify_tlv = (struct assoc_notification_tlv *)((unsigned char *)notify_tlv->status + notify_tlv->bssid_num * sizeof(struct assoc_status));
	}

	request->len = (unsigned short)(p - request->body);



	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}




int _1905_Send_CAC_Request_Message (IN struct _1905_context* ctx, char* almac,
	IN struct cac_request_lib * cac_req)
{
	unsigned short len = 0;
	struct msg *cmd = (struct msg*)cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;
	unsigned char *old_p = NULL;
	int i = 0;
	struct cac_req *p_cac_req = NULL;


	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (cac_req == NULL) {
		LIB_DEBUG(MSG_ERROR, "cac_request_lib NULL pointer");
		return -1;
	}


	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = CAC_REQUEST;

	/* tlv type 0xAD */
	*p++ = 0xAD;
	old_p = p;
	p += 2;

	*p++ = cac_req->num_radio;

	/* one or more Association Status Notification TLVs */
	p_cac_req = cac_req->req;
	for (i = 0; i < cac_req->num_radio; i ++) {


		/* Number of BSSIDs and their statuses included in this TLV */
		memcpy(p, p_cac_req->identifier, ETH_ALEN);
		p += ETH_ALEN;

		*p++ = p_cac_req->op_class_num;

		*p++ = p_cac_req->ch_num;
		*p = 0;//initialize to 0
		*p |= (p_cac_req->cac_method << 5) & 0xE0;
		*p |= (p_cac_req->cac_action << 3) & 0x18;
		p++;


		p_cac_req += 1;
	}
	len = (unsigned short)(p - old_p - 2);
	*((unsigned short*)old_p) = host_to_be16(len);

	request->len = (unsigned short)(p - request->body);



	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}


int _1905_Send_CAC_Terminate_Message (IN struct _1905_context* ctx, char* almac,
	IN struct cac_terminate_lib * cac_term)
{
	unsigned short len = 0;
	struct msg *cmd = (struct msg*)cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;
	unsigned char *old_p = NULL;
	int i = 0;
	struct cac_term *p_cac_term = NULL;


	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (cac_term == NULL) {
		LIB_DEBUG(MSG_ERROR, "cac_term NULL pointer");
		return -1;
	}


	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = CAC_TERMINATION;

	/* tlv type 0xAE */
	*p++ = 0xAE;
	old_p = p;
	p += 2;
	*p++ = cac_term->num_radio;

	/* one or more Association Status Notification TLVs */
	p_cac_term = cac_term->term_tlv;
	for (i = 0; i < cac_term->num_radio; i ++) {


		/* Number of BSSIDs and their statuses included in this TLV */
		memcpy(p, p_cac_term->identifier, ETH_ALEN);
		p += ETH_ALEN;

		*p++ = p_cac_term->op_class_num;

		*p++ = p_cac_term->ch_num;

		p_cac_term ++;
	}
	len = (unsigned short)(p - old_p - 2);
	*((unsigned short*)old_p) = host_to_be16(len);

	request->len = (unsigned short)(p - request->body);



	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}




int _1905_Send_Client_Disassociation_Stats_Message(IN struct _1905_context* ctx,
	char* almac, unsigned short reason_code, struct stat_info *stat)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (stat == NULL) {
		LIB_DEBUG(MSG_ERROR, "stat_info NULL pointer");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = CLIENT_DISASSOCIATION_STATS;

	/* One STA MAC Address TLV */
	*p++ = 0x95;
	*(unsigned short *)p = host_to_be16(ETH_ALEN);
	p += sizeof(unsigned short);

	memcpy(p, stat->mac, ETH_ALEN);
	p += ETH_ALEN;

	/* One Disassociation Reason Code TLV */
	*p++ = 0xCA;
	*(unsigned short *)p = host_to_be16(sizeof(unsigned short));
	p += sizeof(unsigned short);

	*(unsigned short *)p = host_to_be16(reason_code);
	p += sizeof(unsigned short);

	/* One Associated STA Traffic Stats TLV */
	*p++ = 0xA2;
	*(unsigned short *)p = host_to_be16(sizeof(struct stat_info));
	p += sizeof(unsigned short);

	memcpy(p, stat->mac, ETH_ALEN);
	p += ETH_ALEN;

	*(unsigned int *)p = host_to_be32(stat->bytes_sent);
	p += sizeof(unsigned int);

	*(unsigned int *)p = host_to_be32(stat->bytes_received);
	p += sizeof(unsigned int);

	*(unsigned int *)p = host_to_be32(stat->packets_sent);
	p += sizeof(unsigned int);

	*(unsigned int *)p = host_to_be32(stat->packets_received);
	p += sizeof(unsigned int);

	*(unsigned int *)p = host_to_be32(stat->tx_packets_errors);
	p += sizeof(unsigned int);

	*(unsigned int *)p = host_to_be32(stat->rx_packets_errors);
	p += sizeof(unsigned int);

	*(unsigned int *)p = host_to_be32(stat->retransmission_count);
	p += sizeof(unsigned int);

	request->len = p - request->body;

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}


int _1905_Send_Failed_Connection_message(IN struct _1905_context *ctx,
	char* almac, char* sta_mac, unsigned short status, unsigned short reason
#ifdef MAP_R3
	, unsigned char *bssid
#endif
	)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (almac == NULL || sta_mac == NULL) {
		LIB_DEBUG(MSG_ERROR, "mac NULL pointer");
		return -1;
	}
	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = FAILED_CONNECTION_MESSAGE;

#ifdef MAP_R3
	/* One BSSID TLV */
	if (bssid) {
		*p++ = 0xB8;
		*(unsigned short *)p = host_to_be16(ETH_ALEN);
		p += sizeof(unsigned short);

		memcpy(p, bssid, ETH_ALEN);
		p += ETH_ALEN;
	}
#endif
	/* One STA MAC Address TLV */
	*p++ = 0x95;
	*(unsigned short *)p = host_to_be16(ETH_ALEN);
	p += sizeof(unsigned short);

	memcpy(p, sta_mac, ETH_ALEN);
	p += ETH_ALEN;

	/* One Association Status Code TLV */
	*p++ = 0xC9;
	*(unsigned short *)p = host_to_be16(sizeof(unsigned short));
	p += sizeof(unsigned short);

	*(unsigned short *)p = host_to_be16(status);
	p += sizeof(unsigned short);

	if (!status) {
		/* zero or One Reason Code TLV */
		*p++ = 0xCA;
		*(unsigned short *)p = host_to_be16(sizeof(unsigned short));
		p += sizeof(unsigned short);

		*(unsigned short *)p = host_to_be16(reason);
		p += sizeof(unsigned short);
	}


	request->len = p - request->body;


	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_BH_Sta_Cap_Query (IN struct _1905_context *ctx, char *almac)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = BACKHAUL_STA_CAP_QUERY_MESSAGE;
	request->len = 0;
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request)) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

#endif // MAP_R2

#ifdef MAP_R3
int _1905_set_connector(IN struct _1905_context* ctx, struct dpp_sec_cred *dpp_sec)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	unsigned char *p = cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (dpp_sec == NULL) {
		LIB_DEBUG(MSG_ERROR, "%s NULL pointer", __func__);
		return -1;
	}

	cmd->type = LIB_SET_1905_CONNECTOR;

	/* decrypt_threshold */
	*(unsigned short *)p = dpp_sec->decrypt_threshold;
	p += 2;

	/* connector len and connector */
	*(unsigned short *)p = dpp_sec->connector_len;
	p += 2;

	memcpy(p, dpp_sec->payload, dpp_sec->connector_len);
	p += dpp_sec->connector_len;

	/* CSignKeyLen and CSignKey */
	*(unsigned short *)p = dpp_sec->csign_len;
	p += 2;

	memcpy(p, dpp_sec->payload+dpp_sec->connector_len, dpp_sec->csign_len);
	p += dpp_sec->csign_len;

	/* netAccessKeyLen and netAccesskey */
	*(unsigned short *)p = dpp_sec->netaccesskey_len;
	p += 2;

	memcpy(p, dpp_sec->payload+dpp_sec->connector_len+dpp_sec->csign_len,
        dpp_sec->netaccesskey_len);
	p += dpp_sec->netaccesskey_len;

	/* netAccesskey expiry */
	*(unsigned int *)p = dpp_sec->netaccesskey_expiry;
	p += sizeof(unsigned int);

	cmd->length = (unsigned short)(p - cmd->buffer);

	//hex_dump("set 1905 connector", cmd->buffer, cmd->length);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}


int _1905_set_bss_connector(IN struct _1905_context* ctx, struct dpp_bss_cred *dpp_sec)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	unsigned char *p = cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (dpp_sec == NULL) {
		LIB_DEBUG(MSG_ERROR, "%s NULL pointer", __func__);
		return -1;
	}

	if (dpp_sec->payload_len != dpp_sec->bh_connect_len + dpp_sec->fh_connect_len) {
		LIB_DEBUG(MSG_ERROR, "%s len mismatch, total %d bh %d fh %d", __func__,
			dpp_sec->payload_len, dpp_sec->bh_connect_len, dpp_sec->fh_connect_len);
		return -1;
	}

	cmd->type = LIB_SET_BSS_CONNECTOR;

	/* enrollee apcli mac address */
	os_memcpy(p, dpp_sec->mac_apcli, ETH_ALEN);
	p += ETH_ALEN;

	/* enrollee almac address */
	os_memcpy(p, dpp_sec->almac, ETH_ALEN);
	p += ETH_ALEN;

	/* bh connector len */
	*(unsigned short *)p = dpp_sec->bh_connect_len;
	p += 2;

	/* fh connector len */
	*(unsigned short *)p = dpp_sec->fh_connect_len;
	p += 2;

	os_memcpy(p, dpp_sec->payload, dpp_sec->payload_len);
	p += dpp_sec->payload_len;

	cmd->length = (unsigned short)(p - cmd->buffer);

	//hex_dump("set bss connector", cmd->buffer, cmd->length);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}


int _1905_set_chirp_value(IN struct _1905_context* ctx,
	struct chirp_info *chirp)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	unsigned char *p = cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (chirp == NULL) {
		LIB_DEBUG(MSG_ERROR, "chirp NULL pointer");
		return -1;
	}

	if (chirp->chirp_cnt != 1) {
		LIB_DEBUG(MSG_ERROR, "chirp count %d is not eqaul to 1", chirp->chirp_cnt);
		return -1;
	}

	cmd->type = LIB_SET_CHIRP_VALUE;

	os_memcpy(p, chirp->almac, ETH_ALEN);
	p += ETH_ALEN;

	os_memcpy(p, chirp->item, sizeof(struct chirp_tlv_info));
	p+=sizeof(struct chirp_tlv_info);
	cmd->length = (unsigned short)(p - cmd->buffer);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_Proxied_Encap_DPP_Message (IN struct _1905_context* ctx, char* almac, struct dpp_msg *msg)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;
	unsigned char *p_bitmap = NULL, *p_len = NULL;
	int i = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}

	if (!msg->payload_len) {
		LIB_DEBUG(MSG_ERROR, "dpp_frame payload length zero");
		return -1;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = PROXIED_ENCAP_DPP_MESSAGE;

	/* One 1905 Encap DPP TLV */
	/* tlv type 0xCD */
	*p++ = 0xCD;

	p_len = p;
	p += 2;

	p_bitmap = p++;

	*p_bitmap = 0;

    /* Enrollee MAC Address Present */
	if (msg->dpp_info.enrollee_mac_flag) {
		*p_bitmap |= 0x80;

        /* Destination STA MAC Address  */
		memcpy(p, msg->dpp_info.enrollee_mac, ETH_ALEN);
		p += ETH_ALEN;
	}

    /* Channel List Present */
	if (msg->dpp_info.chn_list_flag) {
		*p_bitmap |= 0x40;

        /* Number of Operating Classes */
		*p++ = msg->dpp_info.opclass_num;

		for (i = 0; i < msg->dpp_info.opclass_num; i ++) {
            /* Operating Class */
			*p++ = msg->dpp_info.opclass[i].opclass;

            /* Number of Channels  */
			*p++ = msg->dpp_info.opclass[i].channel_num;

            /* Channel */
			memcpy(p, msg->dpp_info.opclass[i].channel, msg->dpp_info.opclass[i].channel_num);
			p += msg->dpp_info.opclass[i].channel_num;
		}
	}

	/* frame type:
	** if 1, indicate it's a GAS frame
	** if 0, Public action frame
	*/
    /* GAS frame, should set Frame Type as 0xFF */
	if (msg->dpp_frame_indicator == 1) {
		*p_bitmap |= 0x20;
        *p++ = 0xFF;
	}
    else {
        *p++ = msg->frame_type;
    }

    /* length of frame */
    *(unsigned short*)p = host_to_be16(msg->payload_len);
    p += 2;

    /* frame */
	memcpy(p, msg->payload, msg->payload_len);
    p += msg->payload_len;

    /* update encap dpp tlv len */
	*((unsigned short*)p_len) = host_to_be16(p - p_len -2);

    /* DPP Reconfig Authentication Request, shall not include a DPP Chirp Value TLV */
	/* zero or One Chirp Hash TLV */
    if (msg->chirp_tlv_present && msg->frame_type != 15) {
        /* tlv type 0xD3 */
        *p++ = 0xD3;

        /* len pointer */
        p_len = p;
        p += 2;

        *p = 0;

        /* Enrollee MAC Address Present field: reconfiguration purposes */
        if (msg->chirp_info.enrollee_mac_address_present)
            *p |= 0x80;

        /* hash type and hash validity bitmapt */
        if (msg->chirp_info.hash_validity == 1)
            *p |= 0x40;

        p++;

        if (msg->chirp_info.enrollee_mac_address_present) {
            os_memcpy(p, msg->chirp_info.enrollee_mac, ETH_ALEN);
            p += ETH_ALEN;
        }

        /* hash len: 1 byte */
        *(p++) = msg->chirp_info.hash_len;

        /* hash */
        os_memcpy(p, msg->chirp_info.hash_payload, msg->chirp_info.hash_len);

        p += msg->chirp_info.hash_len;

        *((unsigned short*)p_len) = host_to_be16(p - p_len - 2);
    }

	request->len = (unsigned short)(p - request->body);

	//hex_dump("proxied encap dpp", request->body, request->len);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}



int _1905_Send_Direct_Encap_DPP_Message (IN struct _1905_context* ctx, char* almac,
	unsigned short frame_len, unsigned char *frame)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}

	if (frame_len && frame == NULL) {
		LIB_DEBUG(MSG_ERROR, "dpp_frame NULL pointer");
		return -1;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = DIRECT_ENCAP_DPP_MESSAGE;

	/* One DPP Message */
	/* tlv type ???  */
	*p++ = 0xD1;

	/* len pointer */
	*(unsigned short *)p = host_to_be16(frame_len);
	p += 2;

	/* dpp frame */
	memcpy(p, frame, frame_len);

	p += frame_len;

	request->len = (unsigned short)(p - request->body);

	//hex_dump("direct encap dpp", request->body, request->len);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}



int _1905_Send_DPP_Bootstrap_URI_Notification_Message (
	IN struct _1905_context* ctx, char* almac,IN unsigned char *identifier,
	IN unsigned char *local_if_mac, IN unsigned char *peer_if_mac,
	IN unsigned char *uri, IN unsigned short uri_len)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;
	unsigned char *p_tlvlen = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = DPP_BOOTSTRAPING_URI_NOTIFICATION;

	/* One DPP Bootstrapping URI Notification TLV */
	*p++ = 0xCF;

	p_tlvlen = p;
	p += 2;

	/* Radio Unique Identifier of a radio */
	memcpy(p, identifier, ETH_ALEN);
	p += ETH_ALEN;

	/* MAC Address of Local Interface (equal to BSSID) */
	memcpy(p, local_if_mac, ETH_ALEN);
	p += ETH_ALEN;

	/* MAC Address of bSTA from which the URI was received */
	memcpy(p, peer_if_mac, ETH_ALEN);
	p += ETH_ALEN;

	/* DPP Bootstrapping URI received during PBC onboarding */
	memcpy(p, uri, uri_len);
	p += uri_len;

	*((unsigned short*)p_tlvlen) = host_to_be16(p - p_tlvlen - 2);

	request->len = (unsigned short)(p - request->body);

	//hex_dump("dpp bootstrap", request->body, request->len);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_DPP_Bootstrap_URI_Query_Message (IN struct _1905_context* ctx, char* almac)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = DPP_BOOTSTRAPING_URI_QUERY;

	request->len = 0;

    LIB_DEBUG(MSG_ERROR, "%s", __func__);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}



int _1905_Send_DPP_CCE_Indication_Message (IN struct _1905_context* ctx, char* almac,
	IN unsigned char indication)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
		LIB_DEBUG(MSG_ERROR, "almac NULL pointer");
		return -1;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;
	memcpy(request->dest_al_mac, almac, ETH_ALEN);
	request->type = DPP_CCE_INDICATION_MESSAGE;

	/* One CCE Indication tlv */
	/* tlv type 0xD2 */
	*p++ = 0xD2;

	*((unsigned short*)p) = host_to_be16(1);
	p += 2;

	*p++ = indication;
	request->len = (unsigned short)(p - request->body);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Send_Chirp_Notification_Message (IN struct _1905_context* ctx, char* almac,
	IN struct chirp_info *chirp)
{
	struct msg *cmd = (struct msg*)cmdu_buf;
	struct _1905_cmdu_request* request = (struct _1905_cmdu_request*)cmd->buffer;
	unsigned char *p = request->body;
	unsigned char i = 0;
	unsigned char *p_len = NULL;
	struct chirp_tlv_info *p_chirp_item = NULL;
    unsigned char all_agent_mac[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (almac == NULL) {
        os_memcpy(request->dest_al_mac, all_agent_mac, ETH_ALEN);
	}
	else {
		os_memcpy(request->dest_al_mac, almac, ETH_ALEN);
	}

	if (chirp == NULL) {
		LIB_DEBUG(MSG_ERROR, "chirp NULL pointer");
		return -1;
	}

	cmd->type = LIB_1905_CMDU_REQUEST;
	request->type = CHIRP_NOTIFICATION_MESSAGE;

	/* One or more DPP Chirp Value TLV */
	/* tlv type 0xD3 */
	p_chirp_item = chirp->item;
	for (i = 0; i < chirp->chirp_cnt; i ++) {

		(*p++) = 0xD3;
		p_len = p;
		p += 2;

		/*
		1. one byte
		   bit 7, Enrollee MAC Address Present field,
		    This field is only set to 1 for reconfiguration purposes
		   Bit 6: Hash Validity
			1: establish DPP authentication state pertaining to the hash value in this TLV
			0: purge any DPP authentication state pertaining to the hash value in this TLV

		2. Destination STA MAC Address
		3. hash length 1 octet
		4. hash value k octets
		*/

        *p = 0;
		if (p_chirp_item->enrollee_mac_address_present) {
			*p |= (1 << 7);
		}

		if (p_chirp_item->hash_validity) {
			*p |= (1 << 6);
		}
        p++;

        if (p_chirp_item->enrollee_mac_address_present) {
            os_memcpy(p, chirp->item[i].enrollee_mac, ETH_ALEN);
            p += ETH_ALEN;
        }

		*p++ = p_chirp_item->hash_len;

		/*
		4. hash value k octets
		*/
		memcpy(p, p_chirp_item->hash_payload, p_chirp_item->hash_len);
		p += chirp->item[i].hash_len;

		*((unsigned short *)p_len) = host_to_be16(p - p_len - 2);

		p_chirp_item ++;
	}

	request->len = (unsigned short)(p - request->body);

	//hex_dump("chirp notify", request->body, request->len);

	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + sizeof(struct _1905_cmdu_request) + request->len) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}




int _1905_Set_Ap_Wf6_Cap (IN struct _1905_context* ctx,
	IN struct ap_wf6_cap_roles *cap)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (cap == NULL) {
		LIB_DEBUG(MSG_ERROR, "cap NULL pointer");
		return -1;
	}
	cmd->type = LIB_AP_WF6_CAPABILITY;
	cmd->length = sizeof(struct ap_wf6_cap_roles);
	memcpy(cmd->buffer, (unsigned char*)cap, cmd->length);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}


int _1905_Set_GTK_Rekey_Interval (IN struct _1905_context* ctx, IN unsigned int interval)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	unsigned char *p = cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	cmd->type = LIB_SET_GTK_REKEY_INTERVAL;
	cmd->length = sizeof(unsigned int);
	*((unsigned int *)p) = interval;
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

int _1905_Set_R3_Onboarding_Type (IN struct _1905_context* ctx, unsigned char type)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	unsigned char *p = cmd->buffer;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	cmd->type = LIB_SET_R3_ONBOARDING_TYPE;
	cmd->length = sizeof(unsigned char);
	*p = type;
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}

#endif  // MAP_R3
#ifdef MAP_R3_DE
int _1905_Set_Dev_Inven_Tlv (IN struct _1905_context* ctx,
	IN struct dev_inven *de)
{
	struct msg *cmd = (struct msg* )cmdu_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (de == NULL) {
		LIB_DEBUG(MSG_ERROR, "de NULL pointer");
		return -1;
	}
	cmd->type = LIB_DEV_INVEN_TLV;
	cmd->length = sizeof(struct dev_inven);
	memcpy(cmd->buffer, (unsigned char*)de, cmd->length);
	if (_1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		return 0;
	}
}
#endif //MAP_R3_DE


int _1905_Set_Wireless_Interface_Info (IN struct _1905_context* ctx,
	IN struct interface_info_list_hdr *info)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	int ret = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	if (info == NULL) {
		LIB_DEBUG(MSG_ERROR, "info NULL pointer");
		return -1;
	}
	cmd->type = LIB_WIRELESS_INTERFACE_INFO;
	cmd->length = sizeof(struct interface_info_list_hdr) +
		info->interface_count * sizeof(struct interface_info);
	memcpy(cmd->buffer, (unsigned char *)info, cmd->length);
	ret = _1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length);
	if (ret < 0)
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");

	return ret;
}

int _1905_clear_switch_table (IN struct _1905_context* ctx)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	int ret = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	cmd->type = LIB_CLEAR_SWITCH_TABLE;
	cmd->length = 0;
	ret = _1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length);
	if (ret < 0)
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");

	return ret;
}

int _1905_Sync_Recv_Buf_Size (IN struct _1905_context* ctx, unsigned short length, unsigned short own_length)
{
	unsigned char recv_buf[50] = {0};
	struct msg *cmd = (struct msg* )recv_buf;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (length == 0) {
		LIB_DEBUG(MSG_ERROR, " length is 0, there is no need to nofity ");
		return -1;
	}
	cmd->type = LIB_SYNC_RECV_BUF_SIZE;
	cmd->length = sizeof(length) + sizeof(own_length);
	os_memcpy(cmd->buffer, &length, sizeof(length));
	os_memcpy(cmd->buffer+ sizeof(length), &own_length, sizeof(own_length));

	if (_1905_interface_ctrl_send(ctx, (char*)cmd, sizeof(struct msg) + cmd->length) < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		return -1;
	} else {
		LIB_DEBUG(MSG_ERROR, "LIB_SYNC_RECV_BUF_SIZE send to 1905 success (change len:%d, own len:%d)", length, own_length);
		return 0;
	}
}

int _1905_Sync_Recv_Buf_Size_Rsp (IN struct _1905_context* ctx, unsigned short length, unsigned char rsp)
{
	unsigned char tmp_buf[50] = {0};
	struct msg *cmd = (struct msg *) tmp_buf;
	int ret = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (length == 0) {
		LIB_DEBUG(MSG_ERROR, " length is 0, there is no need to nofity ");
		return -1;
	}
	cmd->type = LIB_SYNC_RECV_BUF_SIZE_RSP;
	cmd->length = sizeof(length) + sizeof(rsp);
	os_memcpy(cmd->buffer, &length, sizeof(length));
	os_memcpy(cmd->buffer + sizeof(length), &rsp, sizeof(rsp));

	ret = _1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length);
	if (ret < 0)
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");

	return ret;
}
#ifdef MAP_R3_SP
int _1905_sp_add_static_rule (char *rule_str, int str_len)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct sp_cmd *sp = (struct sp_cmd*)cmd->buffer;
	int ret = 0;
	struct _1905_context *ctx = NULL;
	size_t reply_len = sizeof(cmdu_buf);
	struct timeval tv;

	ctx = _1905_Init("r3_sp");
	if (!ctx) {
		LIB_DEBUG(MSG_ERROR, "!!!error, ctx == NULL");
		ret = -1;
		goto err2;
	}

	if (str_len > (sizeof(cmdu_buf) - sizeof(struct sp_cmd) - sizeof(struct msg))) {
		LIB_DEBUG(MSG_ERROR, "rule string too long to handle\n");
		ret = -1;
		goto err1;
	}

	memset(cmdu_buf, 0, sizeof(cmdu_buf));
	cmd->type = SERVICE_PRIORITIZATION_COMMAND;
	cmd->length = sizeof(struct sp_cmd) + str_len;

	sp->cmd_id = SP_CMD_ADD_RULE;
	sp->len = str_len;
	memcpy(sp->value, rule_str, str_len);

	ret = _1905_interface_ctrl_send(ctx, (char*)cmd, sizeof(struct msg) + cmd->length);
	if (ret < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		goto err1;
	}

	tv.tv_sec = 2;
	tv.tv_usec = 0;
	if (_1905_interface_ctrl_pending(ctx, &tv)) {
		ret = _1905_Receive(ctx, (char *)cmdu_buf, &reply_len);
		if (ret < 0) {
			LIB_DEBUG(MSG_ERROR, "fail to recv rsp or rev wrong rsp\n");
			goto err1;
		}

		if (reply_len > 0) {
			if (cmdu_buf[0] == 0x01)
				ret = 0;	/*success*/
			else
				ret = -1;	/*fail*/
		} else {
			LIB_DEBUG(MSG_ERROR, "reply_len=%d\n", (int)reply_len);
			goto err1;
		}
	}
	else {
		LIB_DEBUG(MSG_ERROR, "waiting rsp timeout\n");
		ret = -1;
	}

err1:
	if (ret < 0)
		LIB_DEBUG(MSG_ERROR, "recv error from 1905daemon");

	_1905_Deinit(ctx);
err2:
	return ret;
}

int _1905_sp_set_static_rule (IN struct _1905_context* ctx,
	char *rule_str, int str_len)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct sp_cmd *sp = (struct sp_cmd*)cmd->buffer;
	int ret = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	if (str_len > (sizeof(cmdu_buf) - sizeof(struct sp_cmd) - 1 - sizeof(struct msg))) {
		LIB_DEBUG(MSG_ERROR, "rule string too long to handle\n");
		return -1;
	}

	memset(cmdu_buf, 0, sizeof(cmdu_buf));
	cmd->type = SERVICE_PRIORITIZATION_COMMAND;
	cmd->length = sizeof(struct sp_cmd) + str_len + 1;

	sp->cmd_id = SP_CMD_SET_RULE;
	sp->len = str_len;
	memcpy(sp->value , rule_str, str_len);

	ret = _1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length);
	if (ret < 0)
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");

	return ret;
}

int _1905_sp_rm_static_rule (IN struct _1905_context* ctx,
	unsigned char rule_index)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct sp_cmd *sp = (struct sp_cmd*)cmd->buffer;
	int ret = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	memset(cmdu_buf, 0, sizeof(cmdu_buf));
	cmd->type = SERVICE_PRIORITIZATION_COMMAND;
	cmd->length = sizeof(struct sp_cmd) + 1;

	sp->cmd_id = SP_CMD_RM_RULE;
	sp->len = 1;
	sp->value[0] = rule_index;

	ret = _1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length);
	if (ret < 0)
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");

	return ret;
}

int _1905_sp_reorder_static_rule (IN struct _1905_context* ctx,
	unsigned char org_idx, unsigned char new_idx)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct sp_cmd *sp = (struct sp_cmd*)cmd->buffer;
	int ret = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	memset(cmdu_buf, 0, sizeof(cmdu_buf));
	cmd->type = SERVICE_PRIORITIZATION_COMMAND;
	cmd->length = sizeof(struct sp_cmd) + 2;

	sp->cmd_id = SP_CMD_REORDER_RULE;
	sp->len = 2;
	sp->value[0] = org_idx;
	sp->value[1] = new_idx;

	ret = _1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length);
	if (ret < 0)
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");

	return ret;
}

int _1905_sp_move_static_rule (IN struct _1905_context* ctx,
	unsigned char org_idx, unsigned char action)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct sp_cmd *sp = (struct sp_cmd*)cmd->buffer;
	int ret = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	memset(cmdu_buf, 0, sizeof(cmdu_buf));
	cmd->type = SERVICE_PRIORITIZATION_COMMAND;
	cmd->length = sizeof(struct sp_cmd) + 2;

	sp->cmd_id = SP_CMD_MOVE_RULE;
	sp->len = 2;
	sp->value[0] = org_idx;
	sp->value[1] = action;

	ret = _1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length);
	if (ret < 0)
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");

	return ret;
}

int _1905_sp_get_static_rule (unsigned char idx, char *rule_string, int *str_len)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct sp_cmd *sp = (struct sp_cmd*)cmd->buffer;
	int ret = 0;
	struct _1905_context *ctx = NULL;
	size_t reply_len = sizeof(cmdu_buf);
	struct timeval tv;

	ctx = _1905_Init("r3_sp");
	if (!ctx) {
		LIB_DEBUG(MSG_ERROR, "!!!error, ctx == NULL");
		ret = -1;
		goto err0;
	}

	memset(cmdu_buf, 0, sizeof(cmdu_buf));
	cmd->type = SERVICE_PRIORITIZATION_COMMAND;
	cmd->length = sizeof(struct sp_cmd) + 1;

	sp->cmd_id = SP_CMD_GET_RULE;
	sp->len = 1;
	sp->value[0] = idx;

	ret = _1905_interface_ctrl_send(ctx, (char*)cmd, sizeof(struct msg) + cmd->length);
	if (ret < 0) {
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");
		goto err1;
	}

	reply_len = sizeof(cmdu_buf);

	tv.tv_sec = 2;
	tv.tv_usec = 0;
	if (_1905_interface_ctrl_pending(ctx, &tv)) {
		ret = _1905_Receive(ctx, (char *)cmdu_buf, &reply_len);
		if (ret < 0) {
			LIB_DEBUG(MSG_ERROR, "fail to recv rsp or rev wrong rsp\n");
			goto err1;
		}
		if (reply_len > 0) {
			memcpy(rule_string, cmdu_buf, reply_len);
			*str_len = reply_len;
		} else {
			LIB_DEBUG(MSG_ERROR, "reply_len=%d\n", (int)reply_len);
			goto err1;
		}
	}
	else {
		LIB_DEBUG(MSG_ERROR, "waiting rsp timeout\n");
		ret = -1;
	}

err1:
	if (ret < 0)
		LIB_DEBUG(MSG_ERROR, "recv error from 1905daemon");

	_1905_Deinit(ctx);
err0:
	return ret;
}

int _1905_sp_enable_dynamic_rule (IN struct _1905_context* ctx,
	unsigned char enable, unsigned char prior)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct sp_cmd *sp = (struct sp_cmd*)cmd->buffer;
	int ret = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	memset(cmdu_buf, 0, sizeof(cmdu_buf));
	cmd->type = SERVICE_PRIORITIZATION_COMMAND;
	cmd->length = sizeof(struct sp_cmd) + 2;

	sp->cmd_id = SP_CMD_SET_DYNAMIC;
	sp->len = 2;
	sp->value[0] = enable;
	sp->value[1] = prior;

	ret = _1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length);
	if (ret < 0)
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");

	return ret;
}

int _1905_sp_set_dscp_tbl (IN struct _1905_context* ctx,
	char *dscp_tbl)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct sp_cmd *sp = (struct sp_cmd*)cmd->buffer;
	int ret = 0, len = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}
	len = strlen(dscp_tbl);
	if (len != 144) {
		LIB_DEBUG(MSG_ERROR, "error!! dscp_tble string len(%d) != 144", len);
		return -1;
	}

	memset(cmdu_buf, 0, sizeof(cmdu_buf));
	cmd->type = SERVICE_PRIORITIZATION_COMMAND;
	cmd->length = sizeof(struct sp_cmd) + len;

	sp->cmd_id = SP_CMD_SET_DSCP_TBL;
	sp->len = len;
	memcpy(sp->value, dscp_tbl, len);

	ret = _1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length);
	if (ret < 0)
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");

	return ret;
}

int _1905_sp_config_done (IN struct _1905_context* ctx)
{
	struct msg *cmd = (struct msg* )cmdu_buf;
	struct sp_cmd *sp = (struct sp_cmd*)cmd->buffer;
	int ret = 0;

	if (ctx == NULL) {
		LIB_DEBUG(MSG_ERROR, "ctx NULL pointer");
		return -1;
	}

	memset(cmdu_buf, 0, sizeof(cmdu_buf));
	cmd->type = SERVICE_PRIORITIZATION_COMMAND;
	cmd->length = sizeof(struct sp_cmd);

	sp->cmd_id = SP_CMD_CONFIG_DONE;
	sp->len = 0;

	ret = _1905_interface_ctrl_request(ctx, (char*)cmd, sizeof(struct msg) + cmd->length);
	if (ret < 0)
		LIB_DEBUG(MSG_ERROR, "_1905_interface_ctrl_request error");

	return ret;
}
#endif
