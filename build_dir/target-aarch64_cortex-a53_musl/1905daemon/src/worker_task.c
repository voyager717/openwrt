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
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stddef.h>

#include "os.h"
#include "worker_task.h"
#include "ethernet_layer.h"
#include "debug.h"
#include "genl_netlink.h"

pthread_t worker_task_id;
int worker_sock;
unsigned char worker_task_buf[512] = {0};
#define MAX_PHY_PORT	7

int worker_task_sock_init(const char *local_path)
{
	static int counter = 0;
	int nameLen = 0;
	int tries = 0;
	int flags;
	int path_len = 0;
	int sock = 0;
	struct sockaddr_un local;
	struct sockaddr_un dest;
	int ret = 0;

	sock = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (sock < 0) {
		return -1;
	}

	local.sun_family = AF_UNIX;
	counter++;
try_again:
	nameLen = strlen(local_path);
    if (nameLen >= (int) sizeof(local.sun_path) -1)  /* too long? */{
		close(sock);
		return -1;
	}

 	local.sun_path[0] = '\0';  /* abstract namespace */
	ret = snprintf(local.sun_path + 1, sizeof(local.sun_path) - 1, "%s", local_path);
	if (os_snprintf_error(sizeof(local.sun_path) - 1, ret)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		close(sock);
		return -1;
	}
    path_len = 1 + nameLen + offsetof(struct sockaddr_un, sun_path);
	tries++;
	if (bind(sock, (struct sockaddr *) &local,
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
		close(sock);
		debug(DEBUG_ERROR, "bind fail\n");
		return -1;
	}

	dest.sun_family = AF_UNIX;
	nameLen = strlen("internal_server");
	if (nameLen >= (int) sizeof(dest.sun_path) - 1) {  /* too long? */
		close(sock);
		return -1;
	}
    dest.sun_path[0] = '\0';  /* abstract namespace */
	ret = snprintf(dest.sun_path + 1, sizeof(dest.sun_path) - 1, "%s", "internal_server");
	if (os_snprintf_error(sizeof(dest.sun_path) - 1, ret)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		close(sock);
		return -1;
	}
    path_len = 1 + nameLen + offsetof(struct sockaddr_un, sun_path);

	if (connect(sock, (struct sockaddr *) &dest,
		    path_len) < 0) {
		close(sock);
		/*for abstract socket path, no need to unlink it*/
//		unlink(ctrl->local.sun_path);
		debug(DEBUG_ERROR, "connect fail, %s\n", strerror(errno));
		return -1;
	}

	/*
	 * Make socket non-blocking so that we don't hang forever if
	 * target dies unexpectedly.
	 */
	flags = fcntl(sock, F_GETFL);
	if (flags >= 0) {
		flags |= O_NONBLOCK;
		if (fcntl(sock, F_SETFL, flags) < 0) {
			perror("fcntl(ctrl->s, O_NONBLOCK)");
			/* Not fatal, continue on.*/
		}
	}

	return sock;
}

int work_message_send(unsigned char *buf, size_t len)
{
	struct os_time started_at;
	const char *_cmd;
	size_t _cmd_len;

	_cmd = (char*)buf;
	_cmd_len = len;
	errno = 0;
	started_at.sec = 0;
	started_at.usec = 0;
retry_send:
	if (send(worker_sock, _cmd, _cmd_len, 0) < 0) {
		if (errno == EAGAIN || errno == EBUSY || errno == EWOULDBLOCK) {
			/*
			 * Must be a non-blocking socket... Try for a bit
			 * longer before giving up.
			 */
			if (started_at.sec == 0)
				os_get_time(&started_at);
			else {
				struct os_time n;
				os_get_time(&n);
				/* Try for a few seconds. */
				if (os_reltime_expired(&n, &started_at, 5))
					goto send_err;
			}
			os_sleep(1, 0);
			goto retry_send;
		}
	send_err:
		return -1;
	}
	return 0;
}

/*
worker task main Fn
now the only things the worker task need to do is polling the mii mgr port status
*/
void *worker_task(void *arg)
{
	int mii_mgr_port_status[MAX_PHY_PORT] = {-1, -1, -1, -1, -1, -1, -1};
	struct worker_task_message *work_message = (struct worker_task_message*)worker_task_buf;
	struct port_status_info *port_info = (struct port_status_info *)(work_message->event);
	int i = 0;
	int status = -1;

	usleep(1000000);
	while(1) {
		/*polling the mii mgr port status*/
		for (i = 0; i < MAX_PHY_PORT; i++) {
			if ((status = eth_layer_update_eth_port_status(i)) < 0) {
				continue;
			}
//			printf("worker task status %d\n", status);
			/*mii mgr port status changed*/
			if (status != mii_mgr_port_status[i]) {
				work_message->type = MII_MGR_PORT_STATUS_CAHNGE;
				work_message->len = sizeof(struct port_status_info);
				port_info->port = i;
				port_info->status = status;

//				printf("worker task status change, send to receiver %d\n", status);
				if (work_message_send(worker_task_buf,
					sizeof(struct worker_task_message) + work_message->len) < 0) {
					debug(DEBUG_ERROR, "worker message(%d) send fail\n", work_message->type);
					continue;
				}
				mii_mgr_port_status[i] = status;
			}

		}

		/*sleep 1 second*/
		usleep(1000000);
	}
}

int worker_task_init()
{
	int ret=0;
	if ((worker_sock = worker_task_sock_init("internal_client")) < 0) {
		debug(DEBUG_ERROR, "internal sock client init fail\n");
		return -1;
	}

	ret = pthread_create(&worker_task_id, NULL, worker_task, NULL);
	if (ret < 0) {
		debug(DEBUG_ERROR, "worker task init fail\n");
		return -1;
	}

	return 0;
}

int worker_task_deinit()
{
	pthread_cancel(worker_task_id);
	close(worker_sock);

	return 0;
}

/*below Fn is used in receiver part*/
int worker_task_receiver_sock_init(char* socket_patch)
{
	int sock_len = 0;
	char *buf = NULL;
	int flags;
	int sock = 0;
	struct sockaddr_un addr;
	int ret = 0;

	buf = os_strdup(socket_patch);
	if (buf == NULL)
		goto fail;
	sock = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (sock < 0) {
		debug(DEBUG_ERROR, "open socket fail(%s)\n", strerror(errno));
		goto fail;
	}

	os_memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	/*abstract path*/
	addr.sun_path[0] = '\0';
	ret = snprintf(addr.sun_path + 1, sizeof(addr.sun_path) - 1, "%s", buf);
	if (os_snprintf_error(sizeof(addr.sun_path) - 1, ret)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		goto fail;
	}
	sock_len = 1 + os_strlen(buf) + offsetof(struct sockaddr_un, sun_path);
	if (bind(sock, (struct sockaddr *) &addr, sock_len) < 0) {
		debug(DEBUG_ERROR, "bind socket fail(%s)\n", strerror(errno));
		goto fail;
	}

	/*
	 * Make socket non-blocking so that we don't hang forever if
	 * target dies unexpectedly.
	 */
	flags = fcntl(sock, F_GETFL);
	if (flags >= 0) {
		flags |= O_NONBLOCK;
		if (fcntl(sock, F_SETFL, flags) < 0) {
			debug(DEBUG_ERROR, "fcntl(ctrl, O_NONBLOCK): %s\n", strerror(errno));
			/* Not fatal, continue on.*/
		}
	}

	os_free(buf);
	return sock;

fail:
	if (sock >= 0) {
		close(sock);
	}
	if (buf) {
		os_free(buf);
	}
	return -1;
}

int worker_task_receiver_sock_deinit(int sock)
{
	close(sock);

	return 0;
}

int worker_task_receiver_recv(int recv_sock, unsigned char* buf, int buf_len)
{
	int res;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);

	res = recvfrom(recv_sock, buf, buf_len, 0,
		       (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		debug(DEBUG_ERROR, "recvfrom fail %s", strerror(errno));
	}
	return res;
}

int worker_task_receiver_event_handler(void *context, struct worker_task_message *event, int length)
{
	switch(event->type) {
		case MII_MGR_PORT_STATUS_CAHNGE:
			{
				struct port_status_info *status = NULL;
				if (length - sizeof(struct worker_task_message) != event->len) {
					debug(DEBUG_ERROR, "invalid MII_MGR_PORT_STATUS_CAHNGE event\n");
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



