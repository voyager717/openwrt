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

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <syslog.h>

#define DEBUG_OFF 		LOG_EMERG
#define DEBUG_ERROR		LOG_ALERT
#define DEBUG_WARN		LOG_CRIT
#define DEBUG_TRACE		LOG_ERR
#define DEBUG_INFO		LOG_WARNING

extern int debug_level;

#define debug(level,...)   \
{                                       \
	if (level <= debug_level)          \
	{                                   \
		printf("[1905Daemon][%s] ", __func__);	\
		printf(__VA_ARGS__);          \
	}                                   \
}

#define debugbyte(Level, fmt, args...)   \
{                                       \
	if (Level <= debug_level)          \
	{                                   \
		printf( fmt, ## args);          \
	}                                   \
}

#define RED(_text)  "\033[1;31m"_text"\033[0m"
#define GRN(_text)  "\033[1;32m"_text"\033[0m"
#define YLW(_text)  "\033[1;33m"_text"\033[0m"
#define BLUE(_text) "\033[1;36m"_text"\033[0m"

void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);
void hex_dump_info(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);
void hex_dump_all(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);
void set_debug_level(int level);

#define TOPO_PREX "\033[1;31m[TOPO]\033[0m"
#define AUTO_CONFIG_PREX "\033[1;31m[AUTO_CONFIG]\033[0m"

#ifdef MAP_R3_SP
#define SP_PREX "\033[1;31m[R3_SP]\033[0m"
#endif

#endif /* __DEBUG_H__ */

