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

/*this file include some function to test 1905 deamon*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h> // close function
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stddef.h>
#include "_1905_interface.h"
#include "p1905_managerd.h"
#include "common.h"
#include "debug.h"
#include "eloop.h"
#include "_1905_interface_ctrl.h"

extern int map_event_handler(struct p1905_managerd_ctx *ctx, char *buf, int len, unsigned char type,
	unsigned char *reply, int *reply_len);
extern int _1905_wait_parse_spec_event(struct p1905_managerd_ctx* ctx, unsigned char* buf, int len, unsigned char event_type, long sec, long usec);

int _1905_interface_open_sock(struct p1905_managerd_ctx *ctx, char* socket_patch);
extern void attach_action(void *eloop_data, void *user_ctx);
int _1905_interface_pending(struct p1905_managerd_ctx *ctx, struct timeval *tv);
int _1905_interface_recv(struct p1905_managerd_ctx *ctx, unsigned char* buf, int buf_len);
extern int _1905_send_sync_recv_buf_size(struct p1905_managerd_ctx* ctx, unsigned short change_len, unsigned short own_len);
int _1905_interface_set_sock_buf(struct p1905_managerd_ctx *ctx, unsigned short len);
void _1905_interface_deinit(struct p1905_managerd_ctx *ctx);

int _1905_interface_init(struct p1905_managerd_ctx *ctx)
{
	struct _1905_interface_ctrl* cli_ctrl = NULL;
	struct _config_buf_ctrl *buf_ctr = NULL;

	cli_ctrl = &ctx->_1905_ctrl;
	dl_list_init(&cli_ctrl->daemon_cli_list);

	if (_1905_interface_open_sock(ctx, "1905_daemon") < 0) {
		return -1;
	}

	buf_ctr = &ctx->config_buffer;
	dl_list_init(&buf_ctr->list);
	ctx->_1905_ctrl.default_own_recv_len = MAX_PKT_LEN;
	ctx->_1905_ctrl.own_recv_buf_len = ctx->_1905_ctrl.default_own_recv_len;
	ctx->_1905_ctrl.sock_buf = NULL;
	ctx->_1905_ctrl.peer_recv_buf_len = 0;
	ctx->_1905_ctrl.default_peer_recv_len = 0;
	if(_1905_interface_set_sock_buf(ctx, ctx->_1905_ctrl.own_recv_buf_len)  != 0) {
		_1905_interface_deinit(ctx);
		return -1;
	}

	return 0;
}


int _1905_interface_reinit(struct p1905_managerd_ctx *ctx)
{
	int res;
	struct _1905_interface_ctrl* cli_ctrl = NULL;

	cli_ctrl = &ctx->_1905_ctrl;
	if (cli_ctrl->sock <= 0)
		return -1;

	close(cli_ctrl->sock);
	cli_ctrl->sock = -1;

	res = _1905_interface_open_sock(ctx, "1905_daemon");
	if (res < 0)
		return -1;
	return cli_ctrl->sock;
}


void _1905_interface_deinit(struct p1905_managerd_ctx *ctx)
{
	struct _1905_interface_ctrl* cli_ctrl = NULL;
	struct _1905_interface_cli *dst, *prev;

	cli_ctrl = &ctx->_1905_ctrl;

	if (cli_ctrl->sock > -1) {
		if (!dl_list_empty(&cli_ctrl->daemon_cli_list)) {
			/*
			 * Wait before closing the control socket if
			 * there are any attached monitors in order to allow
			 * them to receive any pending messages.
			 */
			debug(DEBUG_OFF, "CTRL_IFACE wait for attached "
				   "monitors to receive messages");
			os_sleep(0, 100000);
		}
		close(cli_ctrl->sock);
		cli_ctrl->sock = -1;
		unlink(cli_ctrl->addr.sun_path);
	}

	dl_list_for_each_safe(dst, prev, &cli_ctrl->daemon_cli_list, struct _1905_interface_cli,
			      list)
	{
		free(dst);
	}

	ctx->_1905_ctrl.default_own_recv_len = 0;
	ctx->_1905_ctrl.own_recv_buf_len = 0;
	ctx->_1905_ctrl.peer_recv_buf_len = 0;
	ctx->_1905_ctrl.default_peer_recv_len = 0;
	if(ctx->_1905_ctrl.sock_buf) {
		free(ctx->_1905_ctrl.sock_buf);
		ctx->_1905_ctrl.sock_buf = NULL;
	}
}

int _1905_interface_attach(struct dl_list *ctrl_dst,
					    struct sockaddr_un *from,
					    socklen_t fromlen, char* daemon_name)
{
	struct _1905_interface_cli* dst, *dst_n;
	char addr_txt[200];

	/*firstly delete the previous same attached daemon*/
	dl_list_for_each_safe(dst, dst_n, ctrl_dst, struct _1905_interface_cli, list) {
		if(!memcmp(dst->daemon_name, daemon_name, strlen(daemon_name)))
		{
			debug(DEBUG_OFF, "delete the previous daemon(%s)\n", daemon_name);
			dl_list_del(&dst->list);
			free(dst);
		}
	}
	/*alloc an new client structure and add to the linklist*/
	dst = (struct _1905_interface_cli* )malloc(sizeof(*dst));
	if (dst == NULL)
		return -1;
	if (strlen(daemon_name) > sizeof(dst->daemon_name)) {
		os_free(dst);
		return -1;
	}

	memset(dst, 0, sizeof(*dst));
	memcpy(&dst->addr, from, sizeof(struct sockaddr_un));
	strncpy(dst->daemon_name, daemon_name, strlen(daemon_name));
	dst->addrlen = fromlen;
	dl_list_add(ctrl_dst, &dst->list);
	printf_encode(addr_txt, sizeof(addr_txt),
		      (unsigned char *) from->sun_path,
		      fromlen - offsetof(struct sockaddr_un, sun_path));
	debug(DEBUG_TRACE, "daemon %s attached %s\n", daemon_name, addr_txt);
	return 0;
}

int _1905_interface_detach(struct dl_list *ctrl_dst,
					    struct sockaddr_un *from,
					    socklen_t fromlen)
{
	struct _1905_interface_cli* dst, *dst_n;

	dl_list_for_each_safe(dst, dst_n, ctrl_dst, struct _1905_interface_cli, list) {
		if (fromlen == dst->addrlen &&
		    memcmp(from->sun_path, dst->addr.sun_path,
			      fromlen - offsetof(struct sockaddr_un, sun_path))
		    == 0) {
			char addr_txt[200];
			printf_encode(addr_txt, sizeof(addr_txt),
				      (unsigned char *) from->sun_path,
				      fromlen -
				      offsetof(struct sockaddr_un, sun_path));
			debug(DEBUG_TRACE, "daemon %s monitor detached %s\n",
				   dst->daemon_name, addr_txt);
			dl_list_del(&dst->list);
			free(dst);
			return 0;
		}
	}
	return -1;
}

int _1905_interface_process(struct p1905_managerd_ctx *ctx, struct sockaddr_un* from,
					socklen_t fromlen, char *buf, size_t buf_len, char** reply, size_t *resp_len)
{
#if 0
	struct _1905_interface_cli* dst;
#endif
	int reply_len = 0;
	char* reply_buf = NULL;
	int ret = 0;
#if 0
	unsigned short cmd_type = ((struct msg *)buf)->type;
	unsigned char valid_cli = 0;
#endif
	reply_buf = (char*)os_zalloc(MAX_PKT_LEN);
	if(reply_buf == NULL)
	{
		debug(DEBUG_ERROR, "alloc relay_buf fail!\n");
		goto error3;
	}
	/*it is not essential to check peer device, because 1905libary user is safe enough*/
#if 0

	dl_list_for_each(dst, &ctx->_1905_ctrl.daemon_cli_list, struct _1905_interface_cli, list) {
		if (fromlen == dst->addrlen &&
			memcmp(from->sun_path, dst->addr.sun_path,
				  fromlen - offsetof(struct sockaddr_un, sun_path))
			== 0) {
			debug(DEBUG_TRACE, "process cmd(%d) from daemon(%s)\n", cmd_type, dst->daemon_name);
			valid_cli = 1;
			break;
		}
	}
	/*check whether is the dst valid client*/
	if(valid_cli == 0)
	{
		debug(DEBUG_ERROR, "invalid daemon\n");
		goto error2;
	}
#endif
	/*add command handler here, now just add map handler*/
	/*if the command handler want to reply event, please fill the reply_buf and reply_len*/
	/*reply_len here represents the buffer size of reply_buf*/
	reply_len = MAX_PKT_LEN;
	ret = map_event_handler(ctx, buf, buf_len, 0, (unsigned char *)reply_buf, &reply_len);
	/*check reply len is changed or not*/
	reply_len = reply_len == MAX_PKT_LEN ? 0 : reply_len;

	*reply = reply_buf;
	*resp_len = reply_len;
	return ret;

error3:
	*reply = reply_buf;
	*resp_len = reply_len;
	return -1;
}

void socket_send(struct p1905_managerd_ctx *ctx, char* buf, size_t buf_len, struct _1905_interface_cli* dst)
{
	int _errno = 0;

retry_send:
	if (sendto(ctx->_1905_ctrl.sock, buf, buf_len, 0, (struct sockaddr *) &dst->addr,
	   dst->addrlen) >= 0)
	{
		dst->errors = 0;
		return;
	}

	_errno = errno;
	debug(DEBUG_ERROR, "sendto failed: %d - %s\n", _errno, strerror(_errno));

	if (_errno == ENOBUFS || _errno == EAGAIN) {
		/*
		 * The socket send buffer could be full. This
		 * may happen if client programs are not
		 * receiving their pending messages. Close and
		 * reopen the socket as a workaround to avoid
		 * getting stuck being unable to send any new
		 * responses.
		 */
		 dst->errors++;
		 if (dst->errors < 3) {
			debug(DEBUG_OFF, "send buffer might be full, wait for 1 sec and retry the %d time", dst->errors);
			os_sleep(1, 0);
			goto retry_send;
		}
		debug(DEBUG_OFF, "Failed to send msg from 1905 to mapd socket, drop msg");
	}
}

int send_event_dispatch(struct p1905_managerd_ctx *ctx, struct msg* event, size_t event_len, char* dst_daemon)
{
	struct _1905_interface_cli* dst, *dst_n;
	unsigned char daemon_cnt = 0;

	dl_list_for_each_safe(dst, dst_n,  &ctx->_1905_ctrl.daemon_cli_list, struct _1905_interface_cli, list)
	{
		socket_send(ctx, (char *)event, event_len, dst);
		daemon_cnt++;
	}

	return daemon_cnt;
}

int _1905_interface_send_int(struct p1905_managerd_ctx *ctx, unsigned char* buf, size_t buf_len, char *dst_dameon)
{
	unsigned char daemon_cnt = 0;
	daemon_cnt = send_event_dispatch(ctx, (struct msg*)buf, buf_len, dst_dameon);
	return daemon_cnt;
}

int _1905_interface_send(struct p1905_managerd_ctx *ctx, unsigned char* buf, size_t buf_len, char *dst_dameon)
{
	unsigned char daemon_cnt = 0;
	unsigned short peer_buf_len = 0;
	unsigned short change_buf_len = 0;
	unsigned char recv_buf[50] = {0};
	struct msg *send_event = NULL;

	if(!ctx || !buf || !buf_len) {
		debug(DEBUG_OFF, "invalid input parameters \n");
		return -1;
	}
	peer_buf_len = ctx->_1905_ctrl.peer_recv_buf_len;
	if(buf_len > peer_buf_len) {
		change_buf_len = buf_len;
	} else if(ctx->_1905_ctrl.default_peer_recv_len < peer_buf_len && buf_len <= ctx->_1905_ctrl.default_peer_recv_len)
		change_buf_len = ctx->_1905_ctrl.default_peer_recv_len;

	send_event = (struct msg *)buf;
	if(send_event->type == _1905_SYNC_RECV_BUF_SIZE_RSP_EVENT) // type
		change_buf_len = 0;

	if(change_buf_len == 0) {
		daemon_cnt = _1905_interface_send_int(ctx, buf, buf_len, dst_dameon);
	}else {
		debug(DEBUG_OFF, "send msg to notify mapd change recv buffer size:%d (input buf len:%zu, peer:%d, default:%d)\n",
			change_buf_len, buf_len, ctx->_1905_ctrl.peer_recv_buf_len, ctx->_1905_ctrl.default_peer_recv_len);
		if(_1905_send_sync_recv_buf_size(ctx, change_buf_len, ctx->_1905_ctrl.own_recv_buf_len)) {
			debug(DEBUG_OFF, "notify 1905 change recv buffer size msg fail, drop msg (type:%d)\n", send_event->type);
			return -1;
		}

		if(_1905_wait_parse_spec_event(ctx, recv_buf, sizeof(recv_buf), LIB_SYNC_RECV_BUF_SIZE_RSP, 3, 0) < 0)
		{
			debug(DEBUG_OFF, "adjust recv sock buf len :%d fail, drop msg (type:%d)\n",change_buf_len, send_event->type);
			return -1;
		} else {
			debug(DEBUG_OFF, "send cmd (type:%d, len:%zu) to mapd sucess after adjust recv sock buf len\n", send_event->type, buf_len);
			daemon_cnt = _1905_interface_send_int(ctx, buf, buf_len, dst_dameon);
		}
	}

	return daemon_cnt;
}


int _1905_interface_recv(struct p1905_managerd_ctx *ctx, unsigned char* buf, int buf_len)
{
	int res;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);

	res = recvfrom(ctx->_1905_ctrl.sock, buf, buf_len, 0,
		       (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		debug(DEBUG_ERROR, "recvfrom fail %s", strerror(errno));
	}
	return res;
}

void _1905_interface_receive_process(int sock, void *eloop_ctx,
					      void *sock_ctx)
{
#define RETRY_COUNT 3
	struct p1905_managerd_ctx *ctx = eloop_ctx;
	struct _1905_interface_ctrl* cli_ctrl = NULL;
	char *buf = ctx->_1905_ctrl.sock_buf;
	unsigned short buf_len = ctx->_1905_ctrl.own_recv_buf_len;
	char* daemon_name = NULL;
	int res;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);
	char *reply = NULL, *reply_buf = NULL;
	size_t reply_len = 0;
	int new_attached = 0;
	int retry = 0;
	
	cli_ctrl = &ctx->_1905_ctrl;
	if(buf == NULL)
	{
		debug(DEBUG_ERROR, "recv sock buf is null !\n");
		return;
	}
	memset(buf, 0, buf_len);

retry:
	res = recvfrom(sock, buf, buf_len - 1, 0,
		       (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		if (retry++ <= RETRY_COUNT) {
			debug(DEBUG_ERROR, "recvfrom fail %s, retry\n", strerror(errno));
			os_sleep(0, 200000);
			goto retry;
		}
		else {
			debug(DEBUG_ERROR, "recvfrom fail %s\n", strerror(errno));
			return;
		}
	}
	buf[res] = '\0';
	/*
	hex_dump_dbg("wapp recv cmd:", (unsigned char*)buf, res);
	*/
	if (strncmp(buf, "ATTACH:", strlen("ATTACH:")) == 0) {
		daemon_name = strchr(buf, ':');
		if (daemon_name != NULL) {
			daemon_name++;
			if (_1905_interface_attach(&cli_ctrl->daemon_cli_list, &from,
							     fromlen, daemon_name)) {
				reply = "FAIL\n";
				reply_len = 5;
			} else {
				new_attached = 1;
				reply = "OK\n";
				debug(DEBUG_TRACE, "ATTACH success \n");
				reply_len = 3;
			}
		}
	} else if (strncmp(buf, "DETACH", strlen("DETACH")) == 0) {
		if (_1905_interface_detach(&cli_ctrl->daemon_cli_list, &from,
						     fromlen)) {
			reply = "FAIL\n";
			reply_len = 5;
		}else {
			reply = "OK\n";
			debug(DEBUG_TRACE, "DETACH success \n");
			reply_len = 3;
		}
	} else {
		_1905_interface_process(ctx, &from, fromlen, buf, res,
							      &reply_buf, &reply_len);
		reply = reply_buf;
	}

	if (reply_len) {
		if (sendto(sock, reply, reply_len, 0, (struct sockaddr *) &from,
			   fromlen) < 0) {
			int _errno = errno;
			debug(DEBUG_ERROR, "ctrl_iface sendto failed: %d - %s\n",
				_errno, strerror(_errno));
			if (_errno == ENOBUFS || _errno == EAGAIN) {
				/*
				 * The socket send buffer could be full. This
				 * may happen if client programs are not
				 * receiving their pending messages. Close and
				 * reopen the socket as a workaround to avoid
				 * getting stuck being unable to send any new
				 * responses.
				 */
				sock = _1905_interface_reinit(ctx);
				if (sock < 0) {
					debug(DEBUG_ERROR, "Failed to reinitialize ctrl_iface socket");
				}
			}
			if (new_attached) {
				debug(DEBUG_ERROR, "Failed to send response to ATTACH - detaching");
				_1905_interface_detach(
					&cli_ctrl->daemon_cli_list, &from, fromlen);
			}
		}
	}
	if(reply_buf != NULL)
		free(reply_buf);
}

int _1905_interface_open_sock(struct p1905_managerd_ctx *ctx, char* socket_patch)
{
	int sock_len = 0;
	int flags;
	struct _1905_interface_ctrl* cli_ctrl = NULL;
	int nSndBufLen = 0,nRcvBufLen = 0;
	socklen_t optlen = sizeof(int);

	if (!socket_patch)
		goto fail;

	cli_ctrl = &ctx->_1905_ctrl;
	cli_ctrl->sock = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (cli_ctrl->sock < 0) {
		debug(DEBUG_ERROR, "open socket fail(%s)\n", strerror(errno));
		goto fail;
	}

	if (getsockopt(cli_ctrl->sock, SOL_SOCKET, SO_SNDBUF, (void *)&nSndBufLen, &optlen) < 0) {
		nSndBufLen = 1024*1024;
	}
	else {
		nSndBufLen = nSndBufLen * 2;
	}

	if (getsockopt(cli_ctrl->sock, SOL_SOCKET, SO_RCVBUF, (void *)&nRcvBufLen, &optlen) < 0) {
		nRcvBufLen = 1024*1024;
	}
	else {
		nRcvBufLen = nRcvBufLen * 2;
	}

	if (setsockopt(cli_ctrl->sock, SOL_SOCKET, SO_SNDBUF, (const char*)&nSndBufLen, sizeof(int)) < 0) {
		printf("warning: %s set send buffer size failed, %s\n", __func__, strerror(errno));
	} else {
		printf("%s set send buffer size %d\n", __func__, nSndBufLen);
	}

	if (setsockopt(cli_ctrl->sock, SOL_SOCKET, SO_RCVBUF, (const char*)&nRcvBufLen, sizeof(int)) < 0) {
		printf("warning: %s set recv buffer size failed, %s\n", __func__, strerror(errno));
	} else {
		printf("%s set recv buffer size %d\n", __func__, nRcvBufLen);
	}

	memset(&cli_ctrl->addr, 0, sizeof(cli_ctrl->addr));
	cli_ctrl->addr.sun_family = AF_UNIX;
	/*abstract path*/
	cli_ctrl->addr.sun_path[0] = '\0';
	sock_len = snprintf(cli_ctrl->addr.sun_path + 1, sizeof(cli_ctrl->addr.sun_path) - 1, "%s", socket_patch);
	sock_len += 1 + offsetof(struct sockaddr_un, sun_path);
	if (bind(cli_ctrl->sock, (struct sockaddr *) &cli_ctrl->addr, sock_len) < 0) {
		debug(DEBUG_ERROR, "bind socket fail(%s)\n", strerror(errno));
		goto fail;
	}

	/*
	 * Make socket non-blocking so that we don't hang forever if
	 * target dies unexpectedly.
	 */
	flags = fcntl(cli_ctrl->sock, F_GETFL);
	if (flags >= 0) {
		flags |= O_NONBLOCK;
		if (fcntl(cli_ctrl->sock, F_SETFL, flags) < 0) {
			debug(DEBUG_ERROR, "fcntl(ctrl, O_NONBLOCK): %s\n", strerror(errno));
			/* Not fatal, continue on.*/
		}
	}

	return 0;

fail:
	if (cli_ctrl && cli_ctrl->sock >= 0) {
		close(cli_ctrl->sock);
		cli_ctrl->sock = -1;
	}

	return -1;
}
int _1905_interface_pending(struct p1905_managerd_ctx *ctx, struct timeval *tv)
{
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(ctx->_1905_ctrl.sock, &rfds);
	select(ctx->_1905_ctrl.sock + 1, &rfds, NULL, NULL, tv);
	return FD_ISSET(ctx->_1905_ctrl.sock, &rfds);
}

int _1905_interface_set_sock_buf(struct p1905_managerd_ctx *ctx, unsigned short len)
{
	char *buf = NULL;
	if(ctx == NULL || len == 0)
		return -1;

	buf = (char*)malloc(len);
	if(buf == NULL)
	{
		debug(DEBUG_ERROR, "alloc recv buf fail!\n");
		return -1;
	}
	memset(buf, 0, len);
	if(ctx->_1905_ctrl.sock_buf) {
		free(ctx->_1905_ctrl.sock_buf);
		ctx->_1905_ctrl.sock_buf = NULL;
	}

	ctx->_1905_ctrl.sock_buf = buf;
	debug(DEBUG_ERROR, "alloc 1905 sock_buf :%d!\n", len);
	return 0;
}