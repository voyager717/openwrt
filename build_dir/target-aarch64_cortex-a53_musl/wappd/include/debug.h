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

#define NIC_DBG_STRING      ("")

#define RT_DEBUG_OFF        0
#define RT_DEBUG_ERROR      1
#define RT_DEBUG_WARN       2
#define RT_DEBUG_TRACE      3
#define RT_DEBUG_INFO       4

#define TOPO_PREX "\033[1;31m[TOPO]\033[0m"
#define AUTO_CONFIG_PREX "\033[1;31m[AUTO_CONFIG]\033[0m"
#define DPP_MAP_PREX "\033[1;31m[DPP ]\033[0m"


extern int RTDebugLevel;

#define DBGPRINT(Level, fmt, args...)   \
{                                       \
    if (Level <= RTDebugLevel)          \
    {                                   \
		printf("[wappd][%s]", __func__);	\
        printf( fmt, ## args);          \
    }                                   \
}

#define DBGPRINT_RAW(Level, fmt, args...)   \
{       	                                \
    if (Level <= RTDebugLevel)          	\
    {                                   	\
        printf( fmt, ## args);          	\
    }                                   	\
}

void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);
void hex_dump_dbg(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);

#ifdef MASK_PARTIAL_MACADDR
#define MAC2STR(a) (a)[0], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:**:**:%02x:%02x:%02x"
#else
	/* Debug print format string for the MAC Address */
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
	/* Debug print argument for the MAC Address */
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif /* MASK_PARTIAL_MACADDR */

#endif /* __DEBUG_H__ */
