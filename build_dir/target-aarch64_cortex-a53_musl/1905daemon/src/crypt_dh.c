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

/****************************************************************************
    Module Name:
    DH

    Abstract:
    RFC 2631: Diffie-Hellman Key Agreement Method

    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2009/01/21      Create Diffie-Hellman
***************************************************************************/
#include <stdio.h>
#include "crypt_biginterger.h"
#include "crypt_hmac.h"
#include "crypt_sha2.h"
#include "crypt_aes.h"
#include "debug.h"

/*
========================================================================
Routine Description:
	Diffie-Hellman public key generation

Arguments:
	GValue			 Array in UINT8
	GValueLength	 The length of G in bytes
	PValue			 Array in UINT8
	PValueLength	 The length of P in bytes
	PrivateKey		 Private key
	PrivateKeyLength The length of Private key in bytes

Return Value:
	PublicKey		Public key
	PublicKeyLength The length of public key in bytes

Note:
	Reference to RFC2631
	PublicKey = G^PrivateKey (mod P)
========================================================================
*/
void DH_PublicKey_Generate (
	IN UINT8 GValue[],
	IN UINT GValueLength,
	IN UINT8 PValue[],
	IN UINT PValueLength,
	IN UINT8 PrivateKey[],
	IN UINT PrivateKeyLength,
	OUT UINT8 PublicKey[],
	INOUT UINT *PublicKeyLength)
{
	PBIG_INTEGER pBI_G = NULL;
	PBIG_INTEGER pBI_P = NULL;
	PBIG_INTEGER pBI_PrivateKey = NULL;
	PBIG_INTEGER pBI_PublicKey = NULL;

	/*
	 * 1. Check the input parameters
	 *	  - GValueLength, PValueLength and PrivateLength must be large than zero
	 *	  - PublicKeyLength must be large or equal than PValueLength
	 *	  - PValue must be odd
	 *
	 *	  - PValue must be prime number (no implement)
	 *	  - GValue must be greater than 0 but less than the PValue (no implement)
	 */
	if (GValueLength == 0) {
		debug(DEBUG_ERROR, "DH_PublicKey_Generate: G length is (%d)\n", GValueLength);
		return;
	} /* End of if */
	if (PValueLength == 0) {
		debug(DEBUG_ERROR, "DH_PublicKey_Generate: P length is (%d)\n", PValueLength);
		return;
	} /* End of if */
	if (PrivateKeyLength == 0) {
		debug(DEBUG_ERROR, "DH_PublicKey_Generate: private key length is (%d)\n", PrivateKeyLength);
		return;
	} /* End of if */
	if (*PublicKeyLength < PValueLength) {
		debug(DEBUG_ERROR, "DH_PublicKey_Generate: public key length(%d) must be large or equal than P length(%d)\n",
			*PublicKeyLength, PValueLength);
		return;
	} /* End of if */
	if (!(PValue[PValueLength - 1] & 0x1)) {
		debug(DEBUG_ERROR, "DH_PublicKey_Generate: P value must be odd\n");
		return;
	} /* End of if */

	/*
	 * 2. Transfer parameters to BigInteger structure
	 */
	BigInteger_Init(&pBI_G);
	BigInteger_Init(&pBI_P);
	BigInteger_Init(&pBI_PrivateKey);
	BigInteger_Init(&pBI_PublicKey);
	BigInteger_Bin2BI(GValue, GValueLength, &pBI_G);
	BigInteger_Bin2BI(PValue, PValueLength, &pBI_P);
	BigInteger_Bin2BI(PrivateKey, PrivateKeyLength, &pBI_PrivateKey);
	/*
	 * 3. Calculate PublicKey = G^PrivateKey (mod P)
	 *	  - BigInteger Operation
	 *	  - Montgomery reduction
	 */
	BigInteger_Montgomery_ExpMod(pBI_G, pBI_PrivateKey, pBI_P, &pBI_PublicKey);

	/*
	 * 4. Transfer BigInteger structure to char array
	 */
	BigInteger_BI2Bin(pBI_PublicKey, PublicKey, PublicKeyLength);

	BigInteger_Free(&pBI_G);
	BigInteger_Free(&pBI_P);
	BigInteger_Free(&pBI_PrivateKey);
	BigInteger_Free(&pBI_PublicKey);
} /* End of DH_PublicKey_Generate */


/*
========================================================================
Routine Description:
	Diffie-Hellman secret key generation

Arguments:
	PublicKey		 Public key
	PublicKeyLength  The length of Public key in bytes
	PValue			 Array in UINT8
	PValueLength	 The length of P in bytes
	PrivateKey		 Private key
	PrivateKeyLength The length of Private key in bytes

Return Value:
	SecretKey		 Secret key
	SecretKeyLength  The length of secret key in bytes

Note:
	Reference to RFC2631
	SecretKey = PublicKey^PrivateKey (mod P)
========================================================================
*/
void DH_SecretKey_Generate (
	IN UINT8 PublicKey[],
	IN UINT PublicKeyLength,
	IN UINT8 PValue[],
	IN UINT PValueLength,
	IN UINT8 PrivateKey[],
	IN UINT PrivateKeyLength,
	OUT UINT8 SecretKey[],
	INOUT UINT *SecretKeyLength)
{
	PBIG_INTEGER pBI_P = NULL;
	PBIG_INTEGER pBI_SecretKey = NULL;
	PBIG_INTEGER pBI_PrivateKey = NULL;
	PBIG_INTEGER pBI_PublicKey = NULL;

	/*
	 * 1. Check the input parameters
	 *	  - PublicKeyLength, PValueLength and PrivateLength must be large than zero
	 *	  - SecretKeyLength must be large or equal than PValueLength
	 *	  - PValue must be odd
	 *
	 *	  - PValue must be prime number (no implement)
	 */
	if (PublicKeyLength == 0) {
		debug(DEBUG_ERROR, "DH_SecretKey_Generate: public key length is (%d)\n", PublicKeyLength);
		return;
	} /* End of if */
	if (PValueLength == 0) {
		debug(DEBUG_ERROR, "DH_SecretKey_Generate: P length is (%d)\n", PValueLength);
		return;
	} /* End of if */
	if (PrivateKeyLength == 0) {
		debug(DEBUG_ERROR, "DH_SecretKey_Generate: private key length is (%d)\n", PrivateKeyLength);
		return;
	} /* End of if */
	if (*SecretKeyLength < PValueLength) {
		debug(DEBUG_ERROR, "DH_SecretKey_Generate: secret key length(%d) must be large or equal than P length(%d)\n",
			*SecretKeyLength, PValueLength);
		return;
	} /* End of if */
	if (!(PValue[PValueLength - 1] & 0x1)) {
		debug(DEBUG_ERROR, "DH_SecretKey_Generate: P value must be odd\n");
		return;
	} /* End of if */

	/*
	 * 2. Transfer parameters to BigInteger structure
	 */
	BigInteger_Init(&pBI_P);
	BigInteger_Init(&pBI_PrivateKey);
	BigInteger_Init(&pBI_PublicKey);
	BigInteger_Init(&pBI_SecretKey);

	BigInteger_Bin2BI(PublicKey, PublicKeyLength, &pBI_PublicKey);
	BigInteger_Bin2BI(PValue, PValueLength, &pBI_P);
	BigInteger_Bin2BI(PrivateKey, PrivateKeyLength, &pBI_PrivateKey);

	/*
	 * 3. Calculate SecretKey = PublicKey^PrivateKey (mod P)
	 *	  - BigInteger Operation
	 *	  - Montgomery reduction
	 */
	BigInteger_Montgomery_ExpMod(pBI_PublicKey, pBI_PrivateKey, pBI_P, &pBI_SecretKey);

	/*
	 * 4. Transfer BigInteger structure to char array
	 */
	BigInteger_BI2Bin(pBI_SecretKey, SecretKey, SecretKeyLength);

	BigInteger_Free(&pBI_P);
	BigInteger_Free(&pBI_PrivateKey);
	BigInteger_Free(&pBI_PublicKey);
	BigInteger_Free(&pBI_SecretKey);
} /* End of DH_SecretKey_Generate */


