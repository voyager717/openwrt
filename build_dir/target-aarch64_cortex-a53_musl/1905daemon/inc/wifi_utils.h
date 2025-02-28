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

#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

#define MAX_SSID_LEN 33

/*WSC Encryption type, defined in wsc 2.0.2 p.114*/
#define ENCRYP_NONE 0x0001
#define ENCRYP_WEP 0x0002
#define ENCRYP_TKIP 0x0004
#define ENCRYP_AES 0x0008

/*WSC Authentication type, defined in wsc 2.0.2 p.105*/
#define AUTH_OPEN 0x0001
#define AUTH_WPA_PERSONAL 0x0002
#define AUTH_SHARED 0x0004
#define AUTH_WPA_ENTERPRISE 0x0008
#define AUTH_WPA2_ENTERPRISE 0x0010
#define AUTH_WPA2_PERSONAL 0x0020
#define AUTH_SAE_PERSONAL 0x0040
#define AUTH_DPP_ONLY 0x0080

/*WSC connection type*/
#define ESS 0x1
#define IBSS 0x2

/*WSC configured status*/
#define AP_NOT_CONFIGURED 0x1
#define AP_ALREADY_CONFIGURED     0x2

#define WIFI_MAX_STATION_NUM 16

#define RADIO1 "ra0"
#define RADIO2 "rax0"

typedef enum
{
    wifi_utils_success = 0,
    wifi_utils_error,



} WIFI_UTILS_STATUS;


typedef enum
{
    rf_band_2p4G = 1,
    rf_band_5G,
} RF_BAND;


typedef struct _DH_TABLE
{
	unsigned char pub_key[192];
	unsigned char priv_key[192];
    unsigned char secu_key[192];
} DH_TABLE, *PDH_TABLE;

typedef struct _KDK_KDF_TABLE {
    unsigned char E_Nonce[16];
    unsigned char R_Nonce[16];
    unsigned char E_Mac_Addr[6];

    unsigned char AuthKey[32];
    unsigned char KeyWrapKey[16];
    unsigned char Emsk[32];

    unsigned char DH_Secu_Key[192];
} KDK_KDF_TABLE, *PKDK_KDF_TABLE;

typedef struct _KWA_TABLE {
    unsigned char AuthKey[32];
    unsigned char *EncrptData;
    unsigned int EncrptDataLen;
    unsigned char KWA[8];
} KWA_TABLE, *PKWA_TABLE;

typedef struct _AES_TABLE {
    unsigned char *PlainText;
    unsigned int PlainTextLen;
    unsigned char IV[16];
    unsigned char *CipherText;
    unsigned int CipherTextLen;

    unsigned char KeyWrapKey[16];
} AES_TABLE, *PAES_TABLE;

typedef struct _WSC_CONFIG{
    unsigned char	Ssid[MAX_SSID_LEN];
    unsigned short AuthMode;
    unsigned short	EncrypType;
    unsigned char	WPAKey[64 + 1];
	unsigned char map_vendor_extension;    /*store MAP controller's Muiti-AP Vendor Extesion value in M2*/
	unsigned char hidden_ssid;
#ifdef MAP_R3
	/*
	** cred_len is for BH, ext_cred_len is for FH, which support both FH and BH
	** cred_len is for BH or FH which support only FH or BH
	*/
	unsigned short cred_len;
	unsigned short ext_cred_len;
	unsigned char cred[1024];
	unsigned char ext_cred[1024];
#endif
} WSC_CONFIG, *PWSC_CONFIG;

WIFI_UTILS_STATUS generate_AES_CBC_encrypt_value(AES_TABLE *enc);
WIFI_UTILS_STATUS generate_kwa(KWA_TABLE *pkwa);
WIFI_UTILS_STATUS generate_DH_secu_key(DH_TABLE *pdh);
WIFI_UTILS_STATUS generate_auth_keywrap_key(KDK_KDF_TABLE *pkdk_kdf);
WIFI_UTILS_STATUS generate_AES_CBC_decrypt_value(AES_TABLE *dec);
WIFI_UTILS_STATUS generate_DH_pub_priv_key(DH_TABLE *pdh);
WIFI_UTILS_STATUS get_uuid(unsigned char uuid[]);
#endif /* WIFI_UTILS_H */
