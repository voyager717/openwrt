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
#include <syslog.h>
#include <errno.h>
#include "multi_ap.h"
#include "p1905_managerd.h"
#include "crypt_hmac.h"
#include "crypt_sha2.h"
#include "crypt_aes.h"
#include "_1905_lib_io.h"
#include "common.h"
#include "debug.h"
#include "eloop.h"

#define LFSR_MASK                   0x80000057

unsigned char WPS_DH_G_VALUE[1] = {0x02};
unsigned char WPS_DH_P_VALUE[192] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
    0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
    0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
    0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
    0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
    0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
    0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
    0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
    0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
    0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
    0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
    0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
    0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
    0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
    0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
    0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A,
    0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
    0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96,
    0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
    0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
    0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
    0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x23, 0x73, 0x27,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

unsigned char RandomByte()
{
	unsigned long i;
	unsigned char R, Result;
	unsigned int ShiftReg;
	R = 0;

	ShiftReg = os_random();

	for (i = 0; i < 8; i++)
	{
		if (ShiftReg & 0x00000001)
		{
			ShiftReg = ((ShiftReg ^ LFSR_MASK) >> 1) | 0x80000000;
			Result = 1;
		}
		else
		{
			ShiftReg = ShiftReg >> 1;
			Result = 0;
		}
		R = (R << 1) | Result;
	}

	return R;
}

/**
 *  get uuid for ap-auto configure use.
 *
 * \param uuid output uuid
 * \return  error code.
 */
WIFI_UTILS_STATUS get_uuid(unsigned char uuid[])
{
	return	wifi_utils_error;
}

static unsigned char Wsc_Personal_String[] =  "Wi-Fi Easy and Secure Key Derivation";
#define SHA256_BLOCK_SIZE   64	/* 512 bits = 64 bytes */
#define SHA256_DIGEST_SIZE  32	/* 256 bits = 32 bytes */

int WscDeriveKey (
    unsigned char *kdk, unsigned int kdk_len,
    unsigned char *prsnlString, unsigned int str_len,
    unsigned char *key, unsigned int keyBits )
{
    unsigned int i = 0, iterations = 0;
    unsigned char input[64], output[128]={0};
    unsigned char hmac[32];
    unsigned int temp;

	debug(DEBUG_TRACE, "WscDeriveKey: Deriving a key of %d bits\n", keyBits);
    iterations = ((keyBits/8) + 32 - 1)/32;

    /*Prepare the input buffer. During the iterations, we need only replace the */
    /*value of i at the start of the buffer. */
    temp = host_to_be32(i);
    memcpy(input, &temp, 4);
    memcpy(input+4, prsnlString, str_len);

    temp = host_to_be32(keyBits);
    memcpy(input+4+str_len, &temp, 4);

    for(i = 0; i < iterations; i++)
    {
        /*Set the current value of i at the start of the input buffer */
        temp = host_to_be32(i+1); /*i should start at 1 */
        memcpy(input,&temp,4);
        RT_HMAC_SHA256(kdk, kdk_len, input, 4+str_len+4, hmac, SHA256_DIGEST_SIZE);
        memcpy(output+i*32, hmac, 32);
    }

    /*Sanity check */
    if(keyBits/8 > (32*iterations))
    {
        debug(DEBUG_ERROR, "WscDeriveKey: Key derivation generated less bits "
                              "than asked\n");
        return 1; /*failed */
    }

    memcpy(key, output, 80);

    return 0; /*success */

}


/***
 *  create Diffie-Hellman public key & private key for ap-auto configure use.
 *
 * \param  pdh  pointer of DH_TABLE.
 * \return  error code.
 */

WIFI_UTILS_STATUS generate_DH_pub_priv_key(DH_TABLE *pdh)
{
	int i = 0;
	unsigned int length = 192;

	debug(DEBUG_INFO, "my private key = ");
	for(i = 0; i < 192; i++)
	{
		pdh->priv_key[i] = RandomByte();
		debugbyte(DEBUG_INFO, "%02x", pdh->priv_key[i]);
	}
	debugbyte(DEBUG_INFO, "\n");
	DH_PublicKey_Generate(WPS_DH_G_VALUE, sizeof(WPS_DH_G_VALUE),
            	    	WPS_DH_P_VALUE, sizeof(WPS_DH_P_VALUE),
            	    	pdh->priv_key, sizeof(pdh->priv_key),
            	    	pdh->pub_key, &length);

	return wifi_utils_success;
}

/**
 *  create Diffie-Hellman security key for ap-auto configure use.
 *
 * \param  pdh  pointer of DH_TABLE.
 * \return  error code.
 */

WIFI_UTILS_STATUS generate_DH_secu_key(DH_TABLE *pdh)
{
	unsigned int secu_key_len = 192;

	DH_SecretKey_Generate(pdh->pub_key, sizeof(pdh->pub_key),
						WPS_DH_P_VALUE, sizeof(WPS_DH_P_VALUE),
						pdh->priv_key, sizeof(pdh->priv_key),
						pdh->secu_key, &secu_key_len);

	return wifi_utils_success;
}

/**
 *  create auth & keywrap keys for ap-auto configure use.
 *
 * \param  pkdk_kdf  pointer of KDK_KDF_TABLE.
 * \return  error code.
 */

WIFI_UTILS_STATUS generate_auth_keywrap_key(KDK_KDF_TABLE *pkdk_kdf)
{
	unsigned char	DHKey[32],KdkInput[38],KDK[32],KdfKey[80] = { 0 };
	memset(DHKey, 0, sizeof(DHKey));
	memset(KdkInput, 0, sizeof(KdkInput));
	memset(KDK, 0, sizeof(KDK));

	RT_SHA256(&pkdk_kdf->DH_Secu_Key[0], 192, &DHKey[0]);
	/* Create KDK input data */
	memcpy(&KdkInput[0], pkdk_kdf->E_Nonce, 16);

	memcpy(&KdkInput[16], pkdk_kdf->E_Mac_Addr, 6);

	memcpy(&KdkInput[22], pkdk_kdf->R_Nonce, 16);

	/* Generate the KDK */
	RT_HMAC_SHA256(DHKey, 32,  KdkInput, 38, KDK, SHA256_DIGEST_SIZE);

	/* KDF */
	WscDeriveKey(KDK, 32, Wsc_Personal_String, (sizeof(Wsc_Personal_String) - 1), KdfKey, 640);

	/* Assign Key from KDF */
//	NdisMoveMemory(pkdk_kdf->AuthKey, &KdfKey[0], 32);
//	NdisMoveMemory(pkdk_kdf->KeyWrapKey, &KdfKey[32], 16);
//	NdisMoveMemory(pkdk_kdf->Emsk, &KdfKey[48], 32);
	memcpy(pkdk_kdf->AuthKey, &KdfKey[0], 32);
	memcpy(pkdk_kdf->KeyWrapKey, &KdfKey[32], 16);
	memcpy(pkdk_kdf->Emsk, &KdfKey[48], 32);

	//hexdump
	return wifi_utils_success;
}

/**
 *  create KWA for ap-auto configure use.
 *
 * \param  pkwa  pointer of KDK_KDF_TABLE.
 * \return  error code.
 */

WIFI_UTILS_STATUS generate_kwa(KWA_TABLE *pkwa)
{
	unsigned char TB[256];
	memset(TB, 0, sizeof(TB));
	RT_HMAC_SHA256(pkwa->AuthKey, 32, pkwa->EncrptData, pkwa->EncrptDataLen,
		TB, SHA256_DIGEST_SIZE);
	memcpy(pkwa->KWA, TB, 8);
	return wifi_utils_success;
}

/**
 *  AES encrypt for ap-auto configure use.
 *
 * \param  enc  pointer of AES_TABLE.
 * \return  error code.
 */

WIFI_UTILS_STATUS generate_AES_CBC_encrypt_value(AES_TABLE *enc)
{
	AES_CBC_Encrypt(enc->PlainText, enc->PlainTextLen, enc->KeyWrapKey,sizeof(enc->KeyWrapKey),
		enc->IV, 16, enc->CipherText, &enc->CipherTextLen);

	return wifi_utils_success;
}

/**
 *  AES decrypt for ap-auto configure use.
 *
 * \param  dec  pointer of AES_TABLE.
 * \return  error code.
 */

WIFI_UTILS_STATUS generate_AES_CBC_decrypt_value(AES_TABLE *dec)
{
	debug(DEBUG_TRACE, "PlainTextLen=%u\n", dec->PlainTextLen);
	AES_CBC_Decrypt(dec->CipherText, dec->CipherTextLen, dec->KeyWrapKey, sizeof(dec->KeyWrapKey),
					dec->IV, sizeof(dec->IV), dec->PlainText, &dec->PlainTextLen);
	debug(DEBUG_TRACE, "PlainTextLen=%u\n", dec->PlainTextLen);
	return wifi_utils_success;
}
