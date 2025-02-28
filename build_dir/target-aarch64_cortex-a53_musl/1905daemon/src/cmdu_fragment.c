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
#include "cmdu_fragment.h"
#include "cmdu_tlv.h"
#include "cmdu_tlv_parse.h"
#include "multi_ap.h"
#include "debug.h"
#include "eloop.h"
#include "cmdu_message.h"
#ifdef MAP_R3
#include "security_engine.h"
#endif

LIST_HEAD(list_head_fragment, fragment_list) fragment_head;

static unsigned char clear_fragment_cnt = 0;

void reset_fragment_cnt(void)
{
    clear_fragment_cnt = 0;
}

void add_fragment_cnt(void)
{
    clear_fragment_cnt++;
}

unsigned char get_fragment_cnt(void)
{
    return clear_fragment_cnt;
}


/**
 *  initialize the list head of rx fragment queue.
 *
 */
void init_fragment_queue(void)
{
    LIST_INIT(&fragment_head);
}

/**
 *  delete fragment queue by message type and message id.
 *
 * \param  mtype  cmdu message type.
 * \param  mid  cmdu message id.
 */
void delete_fragment_queue(unsigned short mtype, unsigned short mid)
{
    struct fragment_list *flist, *flist_temp;

    flist = LIST_FIRST(&fragment_head);
    while(flist)
    {
        flist_temp = LIST_NEXT(flist, fragment_entry);

        if(flist->mtype == mtype && flist->mid == mid)
        {
            debug(DEBUG_TRACE, "mtype = %d, mid = %d\n", mtype, mid);
            LIST_REMOVE(flist, fragment_entry);

            if(flist->length)
                free(flist->data);
            free(flist);
        }
        flist = flist_temp;
    }
}

/**
 *  delete all rxfragment queue.
 *
 */
void delete_fragment_queue_all(void)
{
	struct fragment_list *flist, *flist_temp;

	flist = LIST_FIRST(&fragment_head);
	while (flist) {
		flist_temp = LIST_NEXT(flist, fragment_entry);

		debug(DEBUG_TRACE, "mtype = %d, mid = %d\n",
			flist->mtype, flist->mid);
		LIST_REMOVE(flist, fragment_entry);

		if (flist->length)
			free(flist->data);
		free(flist);
		flist = flist_temp;
	}
}

/**
 *  insert rx fragment into queue.
 *
 * \param  mtype  cmdu message type.
 * \param  mid  cmdu message id.
 * \param  fid  cmdu fragment id.
 * \param  lastfragment  cmdu last fragment indication.
 * \param  length  length of this fragment.
 * \param  buffer  tlv start address of rx cmdu message.
 */
void insert_fragment_queue(
	unsigned short mtype, unsigned short mid,
	unsigned char fid, unsigned char lastfragment,
	unsigned short length, unsigned char *buffer)
{
	struct fragment_list *flist;
	unsigned char new_db = 1;

	/* serach fragment queue to know this fragment is existed or not */
	if (!LIST_EMPTY(&fragment_head)) {
		LIST_FOREACH(flist, &fragment_head, fragment_entry) {
			if (flist->mtype == mtype && flist->mid == mid) {
				/* if already in queue, do not store it */
				if (fid == flist->fid) {
					new_db = 0;
					break;
				}
			}
		}
	}

	if (new_db) {
		flist = (struct fragment_list *)malloc(sizeof(struct fragment_list));
		if (flist == NULL)
			return;
		flist->fid = fid;
		flist->lastfragment = lastfragment;
		flist->mid = mid;
		flist->mtype = mtype;
		flist->length = length;

		if (length) {
			flist->data = (unsigned char *)malloc(length);
			if (flist->data == NULL) {
				free(flist);
				return;
			}
			memcpy(flist->data, buffer, length);
		}

		debug(DEBUG_TRACE, "fid = %d last = %d mid = %d mtype = %d, len = %d\n",
				fid, lastfragment, mid, mtype, length);
		LIST_INSERT_HEAD(&fragment_head, flist, fragment_entry);

		/*if new fragment inserted into queue, reset fragment timeout counter*/
		reset_fragment_cnt();
	}
}

/**
 *  reassembly the fragment in rx fragment queue.
 *
 * \param  rxbuf  buffer for store reassembly fragments.
 * \param  mtype  cmdu message type.
 * \param  mid  cmdu message id.
 * \return  total length of reassembly fragments
 */
unsigned short reassembly_fragment_queue(
	struct p1905_managerd_ctx *ctx,
	unsigned short mtype, unsigned short mid)
{
	unsigned char find = 0;
	unsigned char fcnt = 0;
	struct fragment_list *flist, *temp_flist;
	int i = 0;
	unsigned char *temp_rxbuf;
	unsigned short total_length = 0;

	temp_rxbuf = ctx->reassemble_buf.buf  + CMDU_HLEN;

	LIST_FOREACH(flist, &fragment_head, fragment_entry) {
		/*check if the last fragment is existed in fragment queue*/
		if ((flist->mtype == mtype) && (flist->mid == mid) &&
			(flist->lastfragment == 1)) {
			find = 1;
			break;
		}
	}

	/* find == 1 ==> has last fragment in queue*/
	if (find) {
		/* Calculate total amount of this kind fragment. After calculation,
		* we store the result to fcnt
		* Don't worry about the duplicated fragment, because it has already
		* been filterd in Insert stage.
		*/
		LIST_FOREACH(temp_flist, &fragment_head, fragment_entry) {
			if (temp_flist->mid == mid && temp_flist->mtype == mtype)
				fcnt++;
		}

		/* fcnt ==> total fragment amount
		* flist->fid ==> fragment id of last fragment
		* fcnt == (flist->fid + 1) ==> represents we got all fragments,
		* so start to assembly.
		*/
		if (fcnt == (flist->fid + 1)) {
			for (i = 0; i < fcnt; i++) {
				LIST_FOREACH(temp_flist, &fragment_head, fragment_entry) {
					/* ok , we need to get fragments from queue in sequence.
					* so use fragment id to get.
					* then fill these fragments into buffer in sequence.
					*/
					if (temp_flist->fid == i && temp_flist->mid == mid
					&& temp_flist->mtype == mtype) {
						total_length += temp_flist->length;
#ifdef MAP_R3
						if ((total_length + CMDU_HLEN > MAX_PKT_LEN) &&
							((ctx->reassemble_buf.buf_len < PKT_LEN_100K) ||
							(ctx->encr_buf.buf_len < PKT_LEN_100K))) {
							debug(DEBUG_ERROR, "realloc 100k buf for reassembly, encr buf\n");
							buf_huge(&ctx->reassemble_buf, total_length+CMDU_HLEN, 1);
							buf_huge(&ctx->encr_buf, total_length+CMDU_HLEN, 0);
							sec_update_buf(ctx->encr_buf.buf, ctx->encr_buf.buf_len);

							eloop_cancel_timeout(tm_reset_buf_size, (void *)ctx, NULL);
							eloop_register_timeout(60, 0, tm_reset_buf_size, (void *)ctx, NULL);
							if ((ctx->reassemble_buf.buf_len < PKT_LEN_100K) ||
								(ctx->encr_buf.buf_len < PKT_LEN_100K)) {
								total_length = 0;
								debug(DEBUG_ERROR, "one of reassembly, encr buf less than 100k, skip\n");
								break;
							}
							temp_rxbuf = ctx->reassemble_buf.buf;
						}
#endif
						memcpy(temp_rxbuf, temp_flist->data, temp_flist->length);
						temp_rxbuf += temp_flist->length;

					}
				}
			}
            delete_fragment_queue(mtype, mid);
		} else
			return 0;
	} else
		return 0;

	return total_length;
}

/**
 *  un-init rx fragment queue.
 *
 */
void uninit_fragment_queue(void)
{
   delete_fragment_queue_all();
}

