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
    Abstract:
    FIPS 180-2: Secure Hash Standard (SHS)

    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2008/11/24      Create SHA1
    Eddy        2008/07/23      Create SHA256
**************************************************************************
*/

#ifndef __CRYPT_SHA2_H__
#define __CRYPT_SHA2_H__

/* Algorithm options */
#define SHA1_SUPPORT
#define SHA256_SUPPORT

typedef unsigned long long UINT64;
typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned int UINT;
typedef int INT;
typedef unsigned char UINT8;
#define IN
#define OUT
#define INOUT
#define VOID void


#define SHA1_BLOCK_SIZE    64	/* 512 bits = 64 bytes */
#define SHA1_DIGEST_SIZE   20	/* 160 bits = 20 bytes */
typedef struct _SHA1_CTX_STRUC {
	UINT32 HashValue[5];	/* 5 = (SHA1_DIGEST_SIZE / 32) */
	UINT64 MessageLen;	/* total size */
	UINT8 Block[SHA1_BLOCK_SIZE];
	UINT BlockLen;
} SHA1_CTX_STRUC, *PSHA1_CTX_STRUC;

VOID RT_SHA1_Init(
	IN SHA1_CTX_STRUC * pSHA_CTX);
VOID RT_SHA1_Hash(
	IN SHA1_CTX_STRUC * pSHA_CTX);
VOID RT_SHA1_Append(
	IN SHA1_CTX_STRUC * pSHA_CTX,
	IN const UINT8 Message[],
	IN UINT MessageLen);
VOID RT_SHA1_End(
	IN SHA1_CTX_STRUC * pSHA_CTX,
	OUT UINT8 DigestMessage[]);
VOID RT_SHA1(
	IN const UINT8 Message[],
	IN UINT MessageLen,
	OUT UINT8 DigestMessage[]);



#define SHA256_BLOCK_SIZE   64	/* 512 bits = 64 bytes */
#define SHA256_DIGEST_SIZE  32	/* 256 bits = 32 bytes */
typedef struct _SHA256_CTX_STRUC {
	UINT32 HashValue[8];	/* 8 = (SHA256_DIGEST_SIZE / 32) */
	UINT64 MessageLen;	/* total size */
	UINT8 Block[SHA256_BLOCK_SIZE];
	UINT BlockLen;
} SHA256_CTX_STRUC, *PSHA256_CTX_STRUC;

VOID RT_SHA256_Init(
	IN SHA256_CTX_STRUC * pSHA_CTX);
VOID RT_SHA256_Hash(
	IN SHA256_CTX_STRUC * pSHA_CTX);
VOID RT_SHA256_Append(
	IN SHA256_CTX_STRUC * pSHA_CTX,
	IN const UINT8 Message[],
	IN UINT MessageLen);
VOID RT_SHA256_End(
	IN SHA256_CTX_STRUC * pSHA_CTX,
	OUT UINT8 DigestMessage[]);
VOID RT_SHA256(
	IN const UINT8 Message[],
	IN UINT MessageLen,
	OUT UINT8 DigestMessage[]);

#endif /* __CRYPT_SHA2_H__ */

