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
#include <errno.h>
#include <stddef.h>
#include "1905_ctrl.h"

int makeAddr(const char *name, struct sockaddr_un *pAddr, socklen_t *pSockLen)
{
	int ret;
    int nameLen = strlen(name);
    if (nameLen >= (int) sizeof(pAddr->sun_path) -1)  /* too long? */
        return -1;
    pAddr->sun_path[0] = '\0';  /* abstract namespace */
	ret = snprintf(pAddr->sun_path+1, sizeof(pAddr->sun_path) - 1, "%s", name);
	if (ret < 0 || ret >= (sizeof(pAddr->sun_path) - 1)) {
		printf("[%d]snprintf fail!\n", __LINE__);
		return -1;
	}
    pAddr->sun_family = AF_UNIX;
    *pSockLen = 1 + nameLen + offsetof(struct sockaddr_un, sun_path);
    return 0;
}

struct _1905_ctrl *_1905_ctrl_open(const char *ctrl_path)
{
	struct _1905_ctrl *ctrl = NULL;
	socklen_t socklen = 0;
	int status = 0;

	ctrl = malloc(sizeof(*ctrl));
	if (!ctrl) {
		printf("memory is not available\n");
		return NULL;
	}
	memset(ctrl, 0, sizeof(*ctrl));

	ctrl->s = socket(AF_UNIX, SOCK_STREAM, 0);
	if (ctrl->s < 0) {
		printf("create socket for ctrl interface fail\n");
		free(ctrl);
		return NULL;
	}


	makeAddr("/tmp/1905ctrl_cli", &ctrl->local, &socklen);
	if (bind(ctrl->s, (struct sockaddr *)&ctrl->local, socklen) < 0) {
		printf("bind %s error(%s)", "/tmp/1905ctrl_cli", strerror(errno));
		goto error;
	}

	if (makeAddr(ctrl_path, &ctrl->dest, &socklen) < 0) {
		printf("makeAddr dst fail\n");
		goto error;
	}
	status = connect(ctrl->s, (struct sockaddr *)&ctrl->dest, socklen);
	if (status < 0) {
		printf("connect %s error(%s)", ctrl_path, strerror(errno));
		goto error;
	}

	return ctrl;

error:
	close(ctrl->s);
	free(ctrl);
	return NULL;
}

void _1905_ctrl_close(struct _1905_ctrl *ctrl)
{
	close(ctrl->s);
	free(ctrl);
}

int _1905_cli_command(struct _1905_ctrl *ctrl, const char *cmd, size_t cmd_len)
{
	if (send(ctrl->s, cmd, cmd_len, 0) < 0) {
		printf("send command to 1905 ctrl socket fail\n");
		return -1;
	}

	return 0;
}

