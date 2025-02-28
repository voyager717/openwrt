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


#ifndef __CRYPT_AES_H__
#define __CRYPT_AES_H__

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

/* AES definition & structure */
#define AES_STATE_ROWS 4     /* Block size: 4*4*8 = 128 bits */
#define AES_STATE_COLUMNS 4
#define AES_BLOCK_SIZES AES_STATE_ROWS*AES_STATE_COLUMNS
#define AES_KEY_ROWS 4
#define AES_KEY_COLUMNS 8    /*Key length: 4*{4,6,8}*8 = 128, 192, 256 bits */
#define AES_KEY128_LENGTH 16
#define AES_KEY192_LENGTH 24
#define AES_KEY256_LENGTH 32
#define AES_CBC_IV_LENGTH 16

typedef struct {
    UINT8 State[AES_STATE_ROWS][AES_STATE_COLUMNS];
    UINT8 KeyWordExpansion[AES_KEY_ROWS][AES_KEY_ROWS*((AES_KEY256_LENGTH >> 2) + 6 + 1)];
} AES_CTX_STRUC, *PAES_CTX_STRUC;

/* AES operations */
VOID RT_AES_KeyExpansion (
    IN UINT8 Key[],
    IN UINT KeyLength,
    INOUT AES_CTX_STRUC *paes_ctx);

VOID RT_AES_Encrypt (
    IN UINT8 PlainBlock[],
    IN UINT PlainBlockSize,
    IN UINT8 Key[],
    IN UINT KeyLength,
    OUT UINT8 CipherBlock[],
    INOUT UINT *CipherBlockSize);

VOID RT_AES_Decrypt (
    IN UINT8 CipherBlock[],
    IN UINT CipherBlockSize,
    IN UINT8 Key[],
    IN UINT KeyLength,
    OUT UINT8 PlainBlock[],
    INOUT UINT *PlainBlockSize);

/* AES Counter with CBC-MAC operations */
VOID AES_CCM_MAC (
    IN UINT8 Payload[],
    IN UINT  PayloadLength,
    IN UINT8 Key[],
    IN UINT  KeyLength,
    IN UINT8 Nonce[],
    IN UINT  NonceLength,
    IN UINT8 AAD[],
    IN UINT  AADLength,
    IN UINT  MACLength,
    OUT UINT8 MACText[]);

INT AES_CCM_Encrypt (
    IN UINT8 PlainText[],
    IN UINT  PlainTextLength,
    IN UINT8 Key[],
    IN UINT  KeyLength,
    IN UINT8 Nonce[],
    IN UINT  NonceLength,
    IN UINT8 AAD[],
    IN UINT  AADLength,
    IN UINT  MACLength,
    OUT UINT8 CipherText[],
    INOUT UINT *CipherTextLength);

INT AES_CCM_Decrypt (
    IN UINT8 CipherText[],
    IN UINT  CipherTextLength,
    IN UINT8 Key[],
    IN UINT  KeyLength,
    IN UINT8 Nonce[],
    IN UINT  NonceLength,
    IN UINT8 AAD[],
    IN UINT  AADLength,
    IN UINT  MACLength,
    OUT UINT8 PlainText[],
    INOUT UINT *PlainTextLength);

/* AES-CMAC operations */
VOID AES_CMAC_GenerateSubKey (
    IN UINT8 Key[],
    IN UINT KeyLength,
    OUT UINT8 SubKey1[],
    OUT UINT8 SubKey2[]);

VOID AES_CMAC (
    IN UINT8 PlainText[],
    IN UINT PlainTextLength,
    IN UINT8 Key[],
    IN UINT KeyLength,
    OUT UINT8 MACText[],
    INOUT UINT *MACTextLength);



/* AES-CBC operations */
VOID AES_CBC_Encrypt (
    IN UINT8 PlainText[],
    IN UINT PlainTextLength,
    IN UINT8 Key[],
    IN UINT KeyLength,
    IN UINT8 IV[],
    IN UINT IVLength,
    OUT UINT8 CipherText[],
    INOUT UINT *CipherTextLength);

VOID AES_CBC_Decrypt (
    IN UINT8 CipherText[],
    IN UINT CipherTextLength,
    IN UINT8 Key[],
    IN UINT KeyLength,
    IN UINT8 IV[],
    IN UINT IVLength,
    OUT UINT8 PlainText[],
    INOUT UINT *PlainTextLength);

/* AES key wrap operations */
INT AES_Key_Wrap (
    IN UINT8 PlainText[],
    IN UINT  PlainTextLength,
    IN UINT8 Key[],
    IN UINT  KeyLength,
    OUT UINT8 CipherText[],
    OUT UINT *CipherTextLength);

INT AES_Key_Unwrap (
    IN UINT8 CipherText[],
    IN UINT  CipherTextLength,
    IN UINT8 Key[],
    IN UINT  KeyLength,
    OUT UINT8 PlainText [],
    OUT UINT *PlainTextLength);


#endif /* __CRYPT_AES_H__ */


