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

#ifndef __STACK_H__
#define __STACK_H__

#include "debug.h"

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#define MAX_AGENT_COUNT 100

typedef struct {
	unsigned char al_mac[ETH_ALEN];
	unsigned char renew_retry_cnt;
	unsigned short profile;
}leaf_info;

typedef struct {
	unsigned int top;
	leaf_info arr[MAX_AGENT_COUNT];
}stack;

static inline void stack_init(stack *s) {
	s->top = 0;

	return;
}

static inline int stack_empty(stack *s) {
	if (s->top == 0)
		return 1;
	else
		return 0;
}

static inline int push(stack *sp, leaf_info l) {
	sp->top += 1;
	sp->arr[sp->top] = l;

	return 0;
}

static inline int pop(stack *sp, leaf_info *pl) {
	if (sp->top == 0)
		return -1;

	*pl = sp->arr[sp->top];
	sp->top--;

	return 0;
}

static inline int get_top(stack *sp, leaf_info **pl) {
	if (sp->top == 0)
		return -1;

	*pl = &sp->arr[sp->top];

	return 0;
}

static inline int pop_node(stack *sp) {
	if (sp->top == 0) {
		debug(DEBUG_ERROR, "BUG here!!!! top == 0\n");
		return -1;
	}
	sp->top--;

	return 0;
}

static inline void empty_stack(stack *s) {
	s->top = 0;
}


#endif