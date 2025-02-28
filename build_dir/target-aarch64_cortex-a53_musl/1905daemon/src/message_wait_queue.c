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
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <assert.h>
#include "message_wait_queue.h"
#include "multi_ap.h"
#include "cmdu.h"
#include "debug.h"


LIST_HEAD(list_head_msg_waitq, msg_wait_list) msg_waitq_head;


/*initialize the list head of message wait queue.*/
void init_message_wait_queue()
{
    LIST_INIT(&msg_waitq_head);
}

void handle_message_wait_queue(struct p1905_managerd_ctx *ctx, unsigned char *buf)
{
    struct msg_wait_list *list = NULL, *list_temp = NULL;

    list = LIST_FIRST(&msg_waitq_head);
    while (list) {
        list_temp = LIST_NEXT(list, wait_entry);
		map_event_handler(ctx, (char *)list->data, list->data_len, 0, NULL, 0);
		process_cmdu_txq(ctx, buf);

		debug(DEBUG_TRACE, "handle event(%04x), delete it from waitq\n", list->mtype);
		LIST_REMOVE(list, wait_entry);
        free(list->data);
        free(list);
        list = list_temp;
    }
}

void insert_message_wait_queue(unsigned short mtype, unsigned char *data,
	unsigned short length)
{
    struct msg_wait_list *list = NULL;

	/* every elemnet of the retry queue is unique
	  * mid is monotonous increasing for every 1905 device
	  */
    list = (struct msg_wait_list *)malloc(sizeof(struct msg_wait_list));
	if (!list) {
		debug(DEBUG_ERROR, "alloc list fail\n");
		return;
	}
	memset(list, 0, sizeof(struct msg_wait_list));
	list->mtype = mtype;
	if (length == 0) {
		debug(DEBUG_ERROR, "invalid length(0)\n");
		free(list);
		return;
	}
	list->data = (unsigned char*)malloc(length);
	if (!list->data) {
		debug(DEBUG_ERROR, "alloc list->data fail\n");
		free(list);
		return;
	}
    memcpy(list->data, data, length);
	list->data_len = length;

    debug(DEBUG_TRACE, "insert message wait queue:\n");
    debug(DEBUG_TRACE, "event type(%04x)\n", mtype);
    LIST_INSERT_HEAD(&msg_waitq_head, list, wait_entry);
}

void delete_message_wait_queue_all()
{
    struct msg_wait_list *list, *list_temp;

    list = LIST_FIRST(&msg_waitq_head);
    while(list) {
        list_temp = LIST_NEXT(list, wait_entry);

        debug(DEBUG_TRACE, "event type(%04x)\n", list->mtype);
        LIST_REMOVE(list, wait_entry);

        free(list->data);
        free(list);
        list = list_temp;
    }
}

void uninit_message_wait_queue()
{
   delete_message_wait_queue_all();
}

