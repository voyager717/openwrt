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
#include "crypt_hmac.h"
#include "crypt_sha2.h"

typedef unsigned long long UINT64;
typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned int UINT;
typedef unsigned char UINT8;



/*
========================================================================
Routine Description:
    HMAC using SHA1 hash function

Arguments:
    key             Secret key
    key_len         The length of the key in bytes
    message         Message context
    message_len     The length of message in bytes
    macLen          Request the length of message authentication code

Return Value:
    mac             Message authentication code

Note:
    None
========================================================================
*/
VOID RT_HMAC_SHA1 (
    IN  const UINT8 Key[],
    IN  UINT KeyLen,
    IN  const UINT8 Message[],
    IN  UINT MessageLen,
    OUT UINT8 MAC[],
    IN  UINT MACLen)
{
    SHA1_CTX_STRUC sha_ctx1;
    SHA1_CTX_STRUC sha_ctx2;
    UINT8 K0[SHA1_BLOCK_SIZE];
    UINT8 Digest[SHA1_DIGEST_SIZE];
    UINT index;

//    NdisZeroMemory(&sha_ctx1, sizeof(SHA1_CTX_STRUC));
	memset(&sha_ctx1, 0, sizeof(SHA1_CTX_STRUC));

//    NdisZeroMemory(&sha_ctx2, sizeof(SHA1_CTX_STRUC));
	memset(&sha_ctx2, 0, sizeof(SHA1_CTX_STRUC));

    /*
     * If the length of K = B(Block size): K0 = K.
     * If the length of K > B: hash K to obtain an L byte string,
     * then append (B-L) zeros to create a B-byte string K0 (i.e., K0 = H(K) || 00...00).
     * If the length of K < B: append zeros to the end of K to create a B-byte string K0
     */
//    NdisZeroMemory(K0, SHA1_BLOCK_SIZE);
	memset(K0, 0, SHA1_BLOCK_SIZE);
    if (KeyLen <= SHA1_BLOCK_SIZE)
//        NdisMoveMemory(K0, Key, KeyLen);
		memcpy(K0, Key, KeyLen);
    else
        RT_SHA1(Key, KeyLen, K0);
    /* End of if */

    /* Exclusive-Or K0 with ipad */
    /* ipad: Inner pad; the byte x・36・ repeated B times. */
    for (index = 0; index < SHA1_BLOCK_SIZE; index++)
        K0[index] ^= 0x36;
        /* End of for */

    RT_SHA1_Init(&sha_ctx1);
    /* H(K0^ipad) */
    RT_SHA1_Append(&sha_ctx1, K0, sizeof(K0));
    /* H((K0^ipad)||text) */
    RT_SHA1_Append(&sha_ctx1, Message, MessageLen);
    RT_SHA1_End(&sha_ctx1, Digest);

    /* Exclusive-Or K0 with opad and remove ipad */
    /* opad: Outer pad; the byte x・5c・ repeated B times. */
    for (index = 0; index < SHA1_BLOCK_SIZE; index++)
        K0[index] ^= 0x36^0x5c;
        /* End of for */

    RT_SHA1_Init(&sha_ctx2);
    /* H(K0^opad) */
    RT_SHA1_Append(&sha_ctx2, K0, sizeof(K0));
    /* H( (K0^opad) || H((K0^ipad)||text) ) */
    RT_SHA1_Append(&sha_ctx2, Digest, SHA1_DIGEST_SIZE);
    RT_SHA1_End(&sha_ctx2, Digest);

    if (MACLen > SHA1_DIGEST_SIZE)
//        NdisMoveMemory(MAC, Digest, SHA1_DIGEST_SIZE);
		memcpy(MAC, Digest, SHA1_DIGEST_SIZE);
    else
//        NdisMoveMemory(MAC, Digest, MACLen);
		memcpy(MAC, Digest, MACLen);
} /* End of RT_HMAC_SHA1 */

/*
========================================================================
Routine Description:
    HMAC using SHA256 hash function

Arguments:
    key             Secret key
    key_len         The length of the key in bytes
    message         Message context
    message_len     The length of message in bytes
    macLen          Request the length of message authentication code

Return Value:
    mac             Message authentication code

Note:
    None
========================================================================
*/
VOID RT_HMAC_SHA256 (
    IN  const UINT8 Key[],
    IN  UINT KeyLen,
    IN  const UINT8 Message[],
    IN  UINT MessageLen,
    OUT UINT8 MAC[],
    IN  UINT MACLen)
{
    SHA256_CTX_STRUC sha_ctx1;
    SHA256_CTX_STRUC sha_ctx2;
    UINT8 K0[SHA256_BLOCK_SIZE];
    UINT8 Digest[SHA256_DIGEST_SIZE];
    UINT index;

//    NdisZeroMemory(&sha_ctx1, sizeof(SHA256_CTX_STRUC));
	memset(&sha_ctx1, 0, sizeof(SHA256_CTX_STRUC));
//    NdisZeroMemory(&sha_ctx2, sizeof(SHA256_CTX_STRUC));
	memset(&sha_ctx2, 0, sizeof(SHA256_CTX_STRUC));
    /*
     * If the length of K = B(Block size): K0 = K.
     * If the length of K > B: hash K to obtain an L byte string,
     * then append (B-L) zeros to create a B-byte string K0 (i.e., K0 = H(K) || 00...00).
     * If the length of K < B: append zeros to the end of K to create a B-byte string K0
     */
//    NdisZeroMemory(K0, SHA256_BLOCK_SIZE);
	memset(K0, 0, SHA256_BLOCK_SIZE);
    if (KeyLen <= SHA256_BLOCK_SIZE) {
//        NdisMoveMemory(K0, Key, KeyLen);
		memcpy(K0, Key, KeyLen);
    } else {
        RT_SHA256(Key, KeyLen, K0);
    }

    /* Exclusive-Or K0 with ipad */
    /* ipad: Inner pad; the byte x・36・ repeated B times. */
    for (index = 0; index < SHA256_BLOCK_SIZE; index++)
        K0[index] ^= 0x36;
        /* End of for */

    RT_SHA256_Init(&sha_ctx1);
    /* H(K0^ipad) */
    RT_SHA256_Append(&sha_ctx1, K0, sizeof(K0));
    /* H((K0^ipad)||text) */
    RT_SHA256_Append(&sha_ctx1, Message, MessageLen);
    RT_SHA256_End(&sha_ctx1, Digest);

    /* Exclusive-Or K0 with opad and remove ipad */
    /* opad: Outer pad; the byte x・5c・ repeated B times. */
    for (index = 0; index < SHA256_BLOCK_SIZE; index++)
        K0[index] ^= 0x36^0x5c;
        /* End of for */

    RT_SHA256_Init(&sha_ctx2);
    /* H(K0^opad) */
    RT_SHA256_Append(&sha_ctx2, K0, sizeof(K0));
    /* H( (K0^opad) || H((K0^ipad)||text) ) */
    RT_SHA256_Append(&sha_ctx2, Digest, SHA256_DIGEST_SIZE);
    RT_SHA256_End(&sha_ctx2, Digest);

    if (MACLen > SHA256_DIGEST_SIZE)
//        NdisMoveMemory(MAC, Digest,SHA256_DIGEST_SIZE);
		memcpy(MAC, Digest,SHA256_DIGEST_SIZE);
    else
//        NdisMoveMemory(MAC, Digest, MACLen);
		memcpy(MAC, Digest, MACLen);

} /* End of RT_HMAC_SHA256 */

/* End of crypt_hmac.c */


