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
#include <syslog.h>
#include <errno.h>
#include "wifi_utils.h"
#include "wsc_attr_tlv.h"
#include <asm/byteorder.h>
#include "multi_ap.h"
#include "common.h"
#include "debug.h"

enum
{
    HAS_SSID_ATTR = 0,
    HAS_AUTHENTICATION_TYPE_ATTR,
    HAS_ENCRYPTION_TYPE_ATTR,
    HAS_NETWORK_KEY_ATTR,
    HAS_MAC_ADDR_ATTR,
//max is 7, because I use a char to record
};

WSC_ATTR_STATUS create_encrypt_settings_filed(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_enrolle_nonce_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_mac_address_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_message_type_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_public_key_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_registrar_nonce_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_uuid_e_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_uuid_r_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_version_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_ssid_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_kwa_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_authentication_type_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_encryption_type_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_network_key_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_auth_type_flag_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_encrypt_type_flag_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_conn_type_flag_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_config_method_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_wsc_state_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_manufacturer_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_model_name_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_model_number_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_serial_number_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_prim_dev_type_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_device_name_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_rf_band_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_association_state_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_device_pwd_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_config_error_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_os_version_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_authenticator_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);
WSC_ATTR_STATUS create_vendor_extension_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length);


WSC_ATTR_STATUS parse_version_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_message_type_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_encrypt_settings_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_enrolle_nonce_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_mac_address_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_public_key_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_registrar_nonce_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_uuid_e_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_uuid_r_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_kwa_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_ssid_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_authentication_type_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_encryption_type_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_network_key_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_auth_type_flag_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_encrypt_type_flag_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_conn_type_flag_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_config_method_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_wsc_state_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_manufacturer_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_model_name_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_model_number_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_serial_number_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_prim_dev_type_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_device_name_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_rf_band_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_association_state_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_device_pwd_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_config_error_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_os_version_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_authenticator_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);
WSC_ATTR_STATUS parse_vendor_extension_attr(
        unsigned char *pkt, void *ctx, unsigned short *length);


/* Notice!! if you want to add new function into wsc_func, please insert new
 * function in sequence. The id value is from small to big for binary search
 * use.
 */
WSC_ATTR_FUNC wsc_func[] =
{
    {ATTR_ASSOCIATION_STATE_ID,      19,  18,  create_association_state_field,   parse_association_state_attr},
    {ATTR_AUTHENTICATION_TYPE_ID,    255, 255, create_authentication_type_field, parse_authentication_type_attr},
    {ATTR_AUTH_TYPE_FLAG_ID,         7,   7,   create_auth_type_flag_field,      parse_auth_type_flag_attr},
    {ATTR_AUTHENTICATOR_ID,          255, 255, create_authenticator_field,       parse_authenticator_attr},
    {ATTR_CONFIG_METHOD_ID,          10,  10,  create_config_method_field,       parse_config_method_attr},
    {ATTR_CONFIG_ERROR_ID,           21,  19,  create_config_error_field,        parse_config_error_attr},
    {ATTR_CONNECTION_TYPE_FLAG_ID,   9,   9,   create_conn_type_flag_field,      parse_conn_type_flag_attr},
    {ATTR_ENCRYPTION_TYPE_ID,        255, 255, create_encryption_type_field,     parse_encryption_type_attr},
    {ATTR_ENCRYPTION_TYPE_FLAG_ID,   8,   8,   create_encrypt_type_flag_field,   parse_encrypt_type_flag_attr},
    {ATTR_DEVICE_NAME_ID,            17,  16,  create_device_name_field,         parse_device_name_attr},
    {ATTR_DEVICE_PWD_ID,             20,  20,  create_device_pwd_field,          parse_device_pwd_attr},
    {ATTR_ENCRYPTED_SETTINGS_ID,     255, 255, create_encrypt_settings_filed,    parse_encrypt_settings_attr},
    {ATTR_ENROLLE_NONCE_ID,          5,   3,   create_enrolle_nonce_field,       parse_enrolle_nonce_attr},
    {ATTR_KEY_WRAP_AUTHENTICATOR_ID, 255, 255, create_kwa_field,                 parse_kwa_attr},
    {ATTR_MAC_ADDRESS_ID,            4,   255, create_mac_address_field,         parse_mac_address_attr},
    {ATTR_MANUFACTURER_ID,           12,  11,  create_manufacturer_field,        parse_manufacturer_attr},
    {ATTR_MESSAGE_TYPE_ID,           2,   2,   create_message_type_field,        parse_message_type_attr},
    {ATTR_MODEL_NAME_ID,             13,  12,  create_model_name_field,          parse_model_name_attr},
    {ATTR_MODEL_NUMBER_ID,           14,  13,  create_model_number_field,        parse_model_number_attr},
    {ATTR_NETWORK_KEY_ID,            255, 255, create_network_key_field,         parse_network_key_attr},
    {ATTR_OS_VERSION_ID,             22,  21,  create_os_version_field,          parse_os_version_attr},
    {ATTR_PUBLIC_KEY_ID,             6,   6,   create_public_key_field,          parse_public_key_attr},
    {ATTR_REGISTRAR_NONCE_ID,        255, 4,   create_registrar_nonce_field,     parse_registrar_nonce_attr},
    {ATTR_RF_BAND_ID,                18,  17,  create_rf_band_field,             parse_rf_band_attr},
    {ATTR_SERIAL_NUMBER_ID,          15,  14,  create_serial_number_field,       parse_serial_number_attr},
    {ATTR_WSC_STATE_ID,              11,  255, create_wsc_state_field,           parse_wsc_state_attr},
    {ATTR_SSID_ID,                   255, 255, create_ssid_field,                parse_ssid_attr},
    {ATTR_UUID_E_ID,                 3,   255, create_uuid_e_field,              parse_uuid_e_attr},
    {ATTR_UUID_R_ID,                 255, 5,   create_uuid_r_field,              parse_uuid_r_attr},
    {ATTR_VENDOR_EXTENSION_ID,       255, 255, create_vendor_extension_field,    parse_vendor_extension_attr},
    {ATTR_VERSION_ID,                1,   1,   create_version_field,             parse_version_attr},
    {ATTR_PRIM_DEV_TYPE_ID,          16,  15,  create_prim_dev_type_field,       parse_prim_dev_type_attr},
};

/* Return index of wsc_func.
 * If return value is -1 ==> cannot find any matched item
 */
int search_match_function_by_id(unsigned short tlv_type)
{
    /*binary search*/
    unsigned short first = 0;
    unsigned short last = (unsigned short)(sizeof(wsc_func)/sizeof(WSC_ATTR_FUNC)) - 1;
    unsigned int index = 0;

    while(first <= last)
    {
        index = (first + last)/2;
        if(wsc_func[index].id < tlv_type)
            first = index + 1;
        else if(wsc_func[index].id > tlv_type)
            last = index -1;
        else
            return index;
    }
    return -1;
}

/* =========================================================================*/
/* WSC attribute TLVs create functions                                       */
/* =========================================================================*/

WSC_ATTR_STATUS create_encrypt_settings_filed(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    AES_TABLE *enc;
    unsigned char *temp;

    enc = (AES_TABLE *)inbuf;
    temp = outbuf;

    memcpy(temp, enc->IV, 16);
    temp += 16;

    if(wifi_utils_success != generate_AES_CBC_encrypt_value(enc))
        return wsc_attr_error;

    memcpy(temp, enc->CipherText, enc->CipherTextLen);
    (*length) = enc->CipherTextLen + 16;//cipher text length + IV length

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_enrolle_nonce_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    if((*length) == ATTR_ENROLLE_NONCE_DATA_LENGTH)
        memcpy(outbuf, inbuf, ATTR_ENROLLE_NONCE_DATA_LENGTH);
    else
        return wsc_attr_error;

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_mac_address_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    if((*length) == ATTR_MAC_ADDRESS_DATA_LENGTH)
        memcpy(outbuf, inbuf, ATTR_MAC_ADDRESS_DATA_LENGTH);
    else
        return wsc_attr_error;

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_public_key_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
	if((*length) == ATTR_PUBLIC_KEY_DATA_LENGTH)
		memcpy(outbuf, inbuf, (*length));
	else
		return wsc_attr_error;

	hex_dump_info("create_public_key",outbuf,ATTR_PUBLIC_KEY_DATA_LENGTH);
    return wsc_attr_success;
}

WSC_ATTR_STATUS create_registrar_nonce_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    if((*length) == ATTR_REGISTRAR_NONCE_DATA_LENGTH)
        memcpy(outbuf, inbuf, ATTR_REGISTRAR_NONCE_DATA_LENGTH);
    else
        return wsc_attr_error;

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_ssid_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    memcpy(outbuf, inbuf, (*length));
    return wsc_attr_success;
}

WSC_ATTR_STATUS create_uuid_e_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    memcpy(outbuf, inbuf, (*length));
    return wsc_attr_success;
}

WSC_ATTR_STATUS create_uuid_r_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    memcpy(outbuf, inbuf, (*length));
    return wsc_attr_success;
}

WSC_ATTR_STATUS create_version_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    *outbuf = *inbuf;
    *length = ATTR_VERSION_DATA_LENGTH;
    return wsc_attr_success;
}

WSC_ATTR_STATUS create_message_type_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    *outbuf = *inbuf;
    *length = ATTR_MESSAGE_TYPE_DATA_LENGTH;
    return wsc_attr_success;
}

WSC_ATTR_STATUS create_kwa_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    KWA_TABLE *kwa;

    kwa = (KWA_TABLE *)inbuf;
    if(wifi_utils_success != generate_kwa(kwa))
        return wsc_attr_error;

    memcpy(outbuf, kwa->KWA, (*length));
    return wsc_attr_success;
}

WSC_ATTR_STATUS create_authentication_type_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    unsigned short authmode;

    authmode = *(unsigned short *)inbuf;
	*(unsigned short *)outbuf = host_to_be16(authmode);

    debug(DEBUG_TRACE, "create authmode 0x%02x%02x\n",*outbuf, *(outbuf + 1));

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_encryption_type_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    unsigned short encryptype;

    encryptype = *(unsigned short *)inbuf;
	*(unsigned short *)outbuf = host_to_be16(encryptype);

    debug(DEBUG_TRACE, "create encryptype 0x%02x%02x\n",*outbuf, *(outbuf + 1));

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_network_key_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    if((*length) == 0)
        return wsc_attr_success;
    if((*length) > 64)
        return wsc_attr_error;

    memcpy(outbuf, inbuf, (*length));

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_auth_type_flag_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    unsigned short flag = 0;

//    flag = (AUTH_OPEN | AUTH_WPA_PERSONAL | AUTH_SHARED | AUTH_WPA2_PERSONAL);
    //modify for only support wsc 2.0 above
  /* To Add the support for WPA3 in MAP_R1*/
   flag = (AUTH_OPEN | AUTH_WPA_PERSONAL | AUTH_WPA2_PERSONAL
	|AUTH_SAE_PERSONAL
	);
	*(unsigned short *)outbuf = host_to_be16(flag);

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_encrypt_type_flag_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    unsigned short flag = 0;

    flag = (ENCRYP_NONE | ENCRYP_WEP | ENCRYP_TKIP | ENCRYP_AES);
	*(unsigned short *)outbuf = host_to_be16(flag);

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_conn_type_flag_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    *outbuf = ESS;
    return wsc_attr_success;
}

WSC_ATTR_STATUS create_config_method_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    //unsigned short method = 0x238C;//temp refer to WSC 2.0.2
    /*modify for support push button/virtual push button/physical push button*/
    unsigned short method = 0x0680;//temp refer to WSC 2.0.2

	*(unsigned short *)outbuf = host_to_be16(method);

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_wsc_state_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    /*set reserve for enrollee*/
    *outbuf = 0x00;

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_manufacturer_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    /* Need to get manufacturer attribute.
     * I left this item for futher implementation. set all 0
     */
	/*max length 64*/
	*length = strlen("mediatek");
	memcpy(outbuf, "mediatek", *length);

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_model_name_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    /* Need to get model name attribute.
     * I left this item for futher implementation. set all 0
     */
	/*max length 32*/
	*length = strlen("MTK 0000");
	memcpy(outbuf, "MTK 0000", *length);

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_model_number_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    /* Need to get model number attribute.
     * I left this item for futher implementation. set all 0
     */
	/*max length 32*/
	*length = strlen(VERSION_1905);
	memcpy(outbuf, VERSION_1905, *length);

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_serial_number_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    /* Need to get serial number attribute.
     * I left this item for futher implementation. set all 0
     */
	/*max length 32*/
	*length = strlen("223333-768367373646");
	memcpy(outbuf, "223333-768367373646", *length);

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_prim_dev_type_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    unsigned char *temp;
	unsigned short cid = 0x6;
	unsigned short scid = 0x1;
	unsigned int oui = 0x0050f204;

    temp = outbuf;

    /*category ID, 2 bytes*/
    /*set category ID to 0x6(network infrastructure), refert to WSC 2.0.2*/
	*(unsigned short *)temp = host_to_be16(cid);
	temp += 2;

    /*OUI, use WIFI Alliance OUI 00 50 F2 04, 4 bytes*/
	*(unsigned int *)temp = host_to_be32(oui);
    temp += 4;

    /*set sub category ID to 0x1(AP), 2 bytes*/
	*(unsigned short *)temp = host_to_be16(scid);
    temp += 2;

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_device_name_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{

	if (*inbuf == CONTROLLER) {
		memcpy(outbuf, "REGISTRAR", strlen("REGISTRAR"));
		*length = strlen("REGISTRAR");
	} else {
		memcpy(outbuf, "ENROLEE", strlen("ENROLEE"));
		*length = strlen("ENROLEE");
	}
    return wsc_attr_success;
}

WSC_ATTR_STATUS create_rf_band_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
	*outbuf = *inbuf;
	debug(DEBUG_TRACE, AUTO_CONFIG_PREX"create rf band for %s\n", *inbuf == 1 ? "2.4g" : "5g");

	return wsc_attr_success;
}

WSC_ATTR_STATUS create_association_state_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
	unsigned short assoc = 0x0001;

    /*set state to connection success*/
	*(unsigned short *)outbuf = host_to_be16(assoc);

	return wsc_attr_success;
}

WSC_ATTR_STATUS create_device_pwd_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
	unsigned short pbc = 0x0004;

    /*set device password ID to 0x0004(push button), WSC 2.0.2*/
	*(unsigned short *)outbuf = host_to_be16(pbc);

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_config_error_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    /*set configure error to 0x0000(no error), WSC 2.0.2*/
	*(unsigned short *)outbuf = 0x0;

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_os_version_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
	unsigned int osv = 0x80000000;
    /*the most significant bit must be set to 1, WSC 2.0.2*/
	*(unsigned int *)outbuf = host_to_be32(osv);

    return wsc_attr_success;
}

WSC_ATTR_STATUS create_authenticator_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
    KWA_TABLE *kwa;

    kwa = (KWA_TABLE *)inbuf;
	if(wifi_utils_success != generate_kwa(kwa))
		return wsc_attr_error;

    memcpy(outbuf, kwa->KWA, (*length));
    return wsc_attr_success;
}

WSC_ATTR_STATUS create_vendor_extension_field(
        unsigned char *outbuf, unsigned char *inbuf, unsigned short *length)
{
	memcpy(outbuf, inbuf, *length);

    return wsc_attr_success;

}

/* =========================================================================*/
/* WSC attribute TLVs parse functions                                       */
/* =========================================================================*/

WSC_ATTR_STATUS parse_kwa_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    /*need to kwa check*/
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_version_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    unsigned char *temp;

    temp = pkt;
    if(((*length) != ATTR_VERSION_DATA_LENGTH) || ((*temp) != WSC_VERSION))
        return wsc_attr_error;

    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_message_type_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    unsigned char *temp;
    struct p1905_managerd_ctx *ctx_temp = (struct p1905_managerd_ctx *)ctx;

    temp = pkt;
	if (ctx_temp->role == CONTROLLER) {
		if(((*length) != ATTR_MESSAGE_TYPE_DATA_LENGTH) || ((*temp) != MESSAGE_TYPE_M1))
			return wsc_attr_error;
	} else {
		if(((*length) != ATTR_MESSAGE_TYPE_DATA_LENGTH) || ((*temp) != MESSAGE_TYPE_M2))
	        return wsc_attr_error;
	}

    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_enrolle_nonce_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    unsigned char *temp;
    struct p1905_managerd_ctx *ctx_temp = (struct p1905_managerd_ctx *)ctx;
    temp = pkt;

	if((*length) != ATTR_ENROLLE_NONCE_DATA_LENGTH)
		return wsc_attr_error;

	if (ctx_temp->role == CONTROLLER) {
		memcpy(ctx_temp->ap_config.e_nonce, temp, ATTR_ENROLLE_NONCE_DATA_LENGTH);
	} else {
		if(memcmp(ctx_temp->ap_config.e_nonce, temp, ATTR_ENROLLE_NONCE_DATA_LENGTH))
			return wsc_attr_error;
	}

    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_registrar_nonce_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    unsigned char *temp;
    struct p1905_managerd_ctx *ctx_temp = (struct p1905_managerd_ctx *)ctx;

	if (ctx_temp->role == AGENT) {
	    if ((*length) != ATTR_REGISTRAR_NONCE_DATA_LENGTH)
	        return wsc_attr_error;

	    temp = pkt;
	    memcpy(ctx_temp->ap_config.r_nonce, temp, ATTR_REGISTRAR_NONCE_DATA_LENGTH);
	}

    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_public_key_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    unsigned char *temp;
    struct p1905_managerd_ctx *ctx_temp = (struct p1905_managerd_ctx *)ctx;
    DH_TABLE *dh_key;
    KDK_KDF_TABLE *kdk;

    if ((*length) != ATTR_PUBLIC_KEY_DATA_LENGTH)
        return wsc_attr_error;

    temp = pkt;
    memcpy(ctx_temp->ap_config.peer_public_key, temp, ATTR_PUBLIC_KEY_DATA_LENGTH);

	if (ctx_temp->role == AGENT) {
	    /*if we are enrolle, need to calculate Diffie-Helman security key here*/
	    dh_key = (DH_TABLE *)malloc(sizeof(DH_TABLE));
		if (dh_key == NULL)
			return wsc_attr_error;
		memset(dh_key, 0, sizeof(DH_TABLE));
	    memcpy(dh_key->priv_key, ctx_temp->ap_config.private_key, 192);
	    memcpy(dh_key->pub_key, ctx_temp->ap_config.peer_public_key, 192);

	    if (wifi_utils_success != generate_DH_secu_key(dh_key)) {
	        free(dh_key);
	        return wsc_attr_error;
	    }
	    memcpy(ctx_temp->ap_config.security_key, dh_key->secu_key, 192);
	    free(dh_key);

	    /*if we are enrolle, need to calculate keywrapkey & authkey here*/
	    kdk = (KDK_KDF_TABLE *)malloc(sizeof(KDK_KDF_TABLE));
		if (kdk == NULL)
			return wsc_attr_error;
		memset(kdk, 0, sizeof(KDK_KDF_TABLE));
	    memcpy(kdk->DH_Secu_Key, ctx_temp->ap_config.security_key, 192);
	    memcpy(kdk->E_Nonce, ctx_temp->ap_config.e_nonce, 16);
	    memcpy(kdk->R_Nonce, ctx_temp->ap_config.r_nonce, 16);
	    memcpy(kdk->E_Mac_Addr, ctx_temp->ap_config.enrolle_mac, 6);
	    if (wifi_utils_success != generate_auth_keywrap_key(kdk)) {
	        free(kdk);
	        return wsc_attr_error;
	    }
	    memcpy(ctx_temp->ap_config.keyWrapKey, kdk->KeyWrapKey, 16);
	    memcpy(ctx_temp->ap_config.authKey, kdk->AuthKey, 32);
	    free(kdk);
	}

    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_ssid_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    unsigned char *temp;
    struct p1905_managerd_ctx *ctx_temp = (struct p1905_managerd_ctx *)ctx;

	if (*length > SSID_MAX_LEN) {
	    debug(DEBUG_ERROR, "length check fail! msg_len(%d) > max_len(32)parse fail\n", *length);
	    return wsc_attr_error;
	}

	if(ctx_temp->role == AGENT && ctx_temp->is_in_encrypt_settings) {
	    temp = pkt;

	    memset(ctx_temp->ap_config_data.Ssid, 0, SSID_MAX_LEN+1);
	    memcpy(ctx_temp->ap_config_data.Ssid, temp, (*length));
	    ctx_temp->get_config_attr_kind |= (1 << HAS_SSID_ATTR);
	}

    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_uuid_e_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{

    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_uuid_r_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_mac_address_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{

    struct p1905_managerd_ctx *ctx_temp = (struct p1905_managerd_ctx *)ctx;

	if((*length) != ATTR_MAC_ADDRESS_DATA_LENGTH)
		return wsc_attr_error;
	if (ctx_temp->role == CONTROLLER) {
		memcpy(ctx_temp->ap_config.enrolle_mac, pkt, ATTR_MAC_ADDRESS_DATA_LENGTH);
	} else {
		if(ctx_temp->is_in_encrypt_settings)
		    ctx_temp->get_config_attr_kind |= (1 << HAS_MAC_ADDR_ATTR);
	}

    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_encrypt_settings_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    WSC_ATTR_STATUS status = wsc_attr_success;
    unsigned char *temp;
    struct p1905_managerd_ctx *ctx_temp = (struct p1905_managerd_ctx *)ctx;
    AES_TABLE *dec = NULL;
    WSC_TLV_HDR *hdr;
    KWA_TABLE kwa;
    unsigned char *temp_buf, *temp1_buf;
    unsigned short type = 0, tlv_len = 0, total_len = 0;
    int index;

	if (ctx_temp->role == CONTROLLER)
		return status;

    ctx_temp->is_in_encrypt_settings = 1;
    ctx_temp->get_config_attr_kind = 0;

	memset(&kwa, 0, sizeof(kwa));
    temp = pkt;
    kwa.EncrptData = (unsigned char *)malloc(512);
	if (!kwa.EncrptData) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"alloc EncrptData fail\n");
		status = wsc_attr_error;
		goto end;
	}
	memset(kwa.EncrptData, 0, 512);

    dec = (AES_TABLE *)malloc(sizeof(AES_TABLE));
	if (!dec) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"alloc dec fail\n");
		status = wsc_attr_error;
		goto end;
	}
	memset(dec, 0, sizeof(AES_TABLE));

    dec->PlainText = malloc(512);
	if (!dec->PlainText) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"alloc PlainText fail\n");
		status = wsc_attr_error;
		goto end;
	}
	memset(dec->PlainText, 0, 512);
    dec->PlainTextLen = 512;
    memcpy(dec->KeyWrapKey, ctx_temp->ap_config.keyWrapKey, 16);

    /*get IV and shift to encrypt data start position*/
    memcpy(dec->IV, temp, 16);
    temp += 16;

    /*get cipher text*/
    dec->CipherText = malloc(512);
	if (!dec->CipherText) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"alloc CipherText fail\n");
		status = wsc_attr_error;
		goto end;
	}
	memset(dec->CipherText, 0, 512);

	if (*length < 16 || *length > 512 + 16 ) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"length check fail! \n");
		status = wsc_attr_error;
		goto end;
	}
    dec->CipherTextLen = (*length) - 16;
    memcpy(dec->CipherText, temp, dec->CipherTextLen);
    if(wifi_utils_success != generate_AES_CBC_decrypt_value(dec))
    {
        status = wsc_attr_error;
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"generate_AES_CBC_decrypt_value fail\n");
        goto end;
    }
    /*start to parse plain text, check KWA first*/
    /*1. calculate KWA for compare*/
    temp_buf = dec->PlainText;
    temp1_buf = dec->PlainText;

    if (((unsigned int)(dec->PlainTextLen - ATTR_KWA_LENGTH - ATTR_HEADR_LENGTH)) > 512) {
	    debug(DEBUG_ERROR, AUTO_CONFIG_PREX"PlainTextLen %d illegal\n", dec->PlainTextLen);
	    goto end;
    }
    memcpy(kwa.EncrptData, temp1_buf,
        (dec->PlainTextLen - ATTR_KWA_LENGTH - ATTR_HEADR_LENGTH));
    kwa.EncrptDataLen = (dec->PlainTextLen - ATTR_KWA_LENGTH - ATTR_HEADR_LENGTH);

    memcpy(kwa.AuthKey, ctx_temp->ap_config.authKey, 32);
    if(wifi_utils_success != generate_kwa(&kwa))
    {
        status = wsc_attr_error;
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"generate_kwa fail\n");
        goto end;
    }

    /*2. shift to start position of received KWA tlv*/
    temp1_buf += (dec->PlainTextLen - ATTR_KWA_LENGTH - ATTR_HEADR_LENGTH);
    hdr = (WSC_TLV_HDR *)temp1_buf;
    if((be_to_host16(hdr->type) != ATTR_KEY_WRAP_AUTHENTICATOR_ID) ||
        (be_to_host16(hdr->length) != ATTR_KWA_LENGTH))
    {
    	debug(DEBUG_ERROR, AUTO_CONFIG_PREX"kwa error, it is not the kwa attribute\n");
        status = wsc_attr_error;
        goto end;
    }
    temp1_buf += ATTR_HEADR_LENGTH;
    if(memcmp(temp1_buf, kwa.KWA, ATTR_KWA_LENGTH))
    {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"err, kwa not match\n");
        status = wsc_attr_error;
        goto end;
    }

    /*3. parse encrypt settings tlv*/
    while((dec->PlainTextLen - ATTR_KWA_LENGTH - ATTR_HEADR_LENGTH) > total_len)
    {
        hdr = (WSC_TLV_HDR *)temp_buf;

        type = be_to_host16(hdr->type);
        tlv_len = be_to_host16(hdr->length);

        debug(DEBUG_TRACE, AUTO_CONFIG_PREX"get encrypt settings tlv type 0x%04x\n", type);
        debug(DEBUG_TRACE, AUTO_CONFIG_PREX"get encrypt settings length 0x%04x\n", tlv_len);

        index = search_match_function_by_id(type);
        if(index >= 0)
        {
            temp_buf += ATTR_HEADR_LENGTH;//shift to data field
            status = wsc_func[index].parse(temp_buf, ctx, &tlv_len);
            if(status != wsc_attr_success)
            {
               debug(DEBUG_ERROR, AUTO_CONFIG_PREX"encrypt settings error, type = 0x%04x\n", type);
                goto end;
            }
        }
        else
        {
            debug(DEBUG_ERROR, AUTO_CONFIG_PREX"not matched item in encrypt settings, type = 0x%04x\n", type);
            status = wsc_attr_not_find;
            goto end;
        }
        temp_buf += tlv_len;
        total_len += (tlv_len + ATTR_HEADR_LENGTH);
    }

end:
	if (kwa.EncrptData)
    	free(kwa.EncrptData);
	if (dec) {
		if (dec->CipherText)
   			free(dec->CipherText);
		if (dec->PlainText)
	    	free(dec->PlainText);
	    free(dec);
	}
    ctx_temp->is_in_encrypt_settings = 0;

    return status;
}

WSC_ATTR_STATUS parse_authentication_type_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    unsigned char *temp = NULL;
    struct p1905_managerd_ctx *ctx_temp = (struct p1905_managerd_ctx *)ctx;
    unsigned short authmode;

	if (ctx_temp->role == AGENT && ctx_temp->is_in_encrypt_settings) {
	    temp = pkt;

	    if((*length) != ATTR_AUTHENTICATION_TYPE_DATA_LENGTH)
	        return wsc_attr_error;

	    authmode = *(unsigned short *)temp;
	    authmode = be_to_host16(authmode);

	    debug(DEBUG_TRACE, "parse authmode = 0x%04x\n", authmode);
		ctx_temp->ap_config_data.AuthMode = authmode;
	    ctx_temp->get_config_attr_kind |= (1 << HAS_AUTHENTICATION_TYPE_ATTR);
	}

    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_encryption_type_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    unsigned char *temp = NULL;
    struct p1905_managerd_ctx *ctx_temp = (struct p1905_managerd_ctx *)ctx;
    unsigned short encryptype;

	if (ctx_temp->role == AGENT && ctx_temp->is_in_encrypt_settings) {
	    if ((*length) != ATTR_ENCRYPTION_TYPE_DATA_LENGTH)
	        return wsc_attr_error;

	    temp = pkt;
	    encryptype = *(unsigned short *)temp;
	    encryptype = be_to_host16(encryptype);

	    debug(DEBUG_TRACE, "parse encryptype = 0x%04x\n", encryptype);
		ctx_temp->ap_config_data.EncrypType = encryptype;
	    ctx_temp->get_config_attr_kind |= (1 << HAS_ENCRYPTION_TYPE_ATTR);
	}

    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_network_key_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    unsigned char *temp;
    struct p1905_managerd_ctx *ctx_temp = (struct p1905_managerd_ctx *)ctx;

	if (ctx_temp->role == AGENT && ctx_temp->is_in_encrypt_settings) {
	    if ((*length) == 0) {
	        ctx_temp->get_config_attr_kind |= (1 << HAS_NETWORK_KEY_ATTR);
	        return wsc_attr_success;
	    }

	    temp = pkt;
	if (*length > 64) {
		debug(DEBUG_ERROR, "length check fail! msg_len(%d) > max_len(64) parse fail\n", *length);
		return wsc_attr_error;
	}
        memset(ctx_temp->ap_config_data.WPAKey, 0, 64+1);
        memcpy(ctx_temp->ap_config_data.WPAKey, temp, (*length));

    	ctx_temp->get_config_attr_kind |= (1 << HAS_NETWORK_KEY_ATTR);
	}

    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_auth_type_flag_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    /*Add WPA3 Support with MAP_R1*/
        unsigned char *temp = NULL;
	struct p1905_managerd_ctx *ctx_temp = (struct p1905_managerd_ctx *)ctx;
	unsigned short authmode;

	if (ctx_temp->role == CONTROLLER) {
		temp = pkt;

		authmode = *(unsigned short *)temp;
		authmode = be_to_host16(authmode);
		debug(DEBUG_TRACE, "parse authmode = 0x%04x\n", authmode);
			ctx_temp->ap_config.auth_type = authmode;
   }
	return wsc_attr_success;
}

WSC_ATTR_STATUS parse_encrypt_type_flag_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_conn_type_flag_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_config_method_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_wsc_state_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_manufacturer_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_model_name_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_model_number_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_serial_number_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_prim_dev_type_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_device_name_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_rf_band_attr(unsigned char *pkt, void *ctx, unsigned short *length)
{
	struct p1905_managerd_ctx* tmp_ctx = (struct p1905_managerd_ctx*)ctx;
	unsigned char radio_index = 0;

	if (tmp_ctx->role == AGENT) {
		if (tmp_ctx->current_autoconfig_info.radio_index != -1) {
			radio_index = (unsigned char)tmp_ctx->current_autoconfig_info.radio_index;
			debug(DEBUG_TRACE, AUTO_CONFIG_PREX"recv controller rf_band(%02x), current rf_band_cap(%02x)\n",
				(*pkt - 1), tmp_ctx->rinfo[radio_index].band);

			return wsc_attr_success;
		} else {
			return wsc_attr_success;
		}
	} else {
		debug(DEBUG_TRACE, AUTO_CONFIG_PREX"agent rf band=%s\n", (*pkt) == 1 ? "2.4G" : "5G");
		return wsc_attr_success;
	}
}

WSC_ATTR_STATUS parse_association_state_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_device_pwd_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_config_error_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_os_version_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    return wsc_attr_success;
}

WSC_ATTR_STATUS parse_authenticator_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
    WSC_ATTR_STATUS status = wsc_attr_success;
    unsigned char *temp;
    struct p1905_managerd_ctx *ctx_temp = (struct p1905_managerd_ctx *)ctx;
    KWA_TABLE *authenticator = NULL;

	if (ctx_temp->role == CONTROLLER)
		return status;

    temp = pkt;

    /*1. get (M1 || M2*) and calculated hash value by HMAC-SHA-256*/
    authenticator = (KWA_TABLE *)malloc(sizeof(KWA_TABLE));
	if (authenticator == NULL) {
		status = wsc_attr_error;
		goto end;
	}
    authenticator->EncrptData = malloc(ctx_temp->last_tx_length + ctx_temp->current_rx_length);
	if (authenticator->EncrptData == NULL) {
		status = wsc_attr_error;
		goto end;
	}
    authenticator->EncrptDataLen = (ctx_temp->last_tx_length + ctx_temp->current_rx_length);
    memcpy(authenticator->EncrptData, ctx_temp->last_tx_data, ctx_temp->last_tx_length);
    memcpy((authenticator->EncrptData + ctx_temp->last_tx_length),
            ctx_temp->current_rx_data, ctx_temp->current_rx_length);
    memcpy(authenticator->AuthKey, ctx_temp->ap_config.authKey, 32);
    if (wifi_utils_success != generate_kwa(authenticator)) {
        status = wsc_attr_error;
        goto end;
    }
    /* 2. compare the calculated value and authenticator data value from M2*/
    if (memcmp(authenticator->KWA, temp, 8)) {
        debug(DEBUG_ERROR, "authenticaor error\n");
        /*different value, we got a error packet*/
        status = wsc_attr_error;
        goto end;
    }

end:
	if (authenticator) {
		if (authenticator->EncrptData != NULL) {
			free(authenticator->EncrptData);
			authenticator->EncrptData = NULL;
		}
    	free(authenticator);
	}

    if (status == wsc_attr_success) {
    	ctx_temp->is_authenticator_exist_in_M2 = 1;

		debug(DEBUG_TRACE, "set ctx->is_authenticator_exist_in_M2 = 1\n");
    }

    return status;
}

WSC_ATTR_STATUS parse_vendor_extension_attr(
        unsigned char *pkt, void *ctx, unsigned short *length)
{
	struct p1905_managerd_ctx* temp_ctx = (struct p1905_managerd_ctx*)ctx;
	unsigned char WFA_vendor_ext_attr_id[3] = {0x00, 0x37, 0x2A};
    unsigned char MTK_vendor_ext_attr_id[3] = {0x00, 0x0C, 0xE7};
	struct wfa_subelements_attr *attr = NULL;
	unsigned int attr_len = *length;

	if (temp_ctx->role == CONTROLLER)
		return wsc_attr_success;
	if (attr_len < sizeof(WFA_vendor_ext_attr_id)) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX" current total len (%d) is less than attr id len (%d)\n", attr_len, (unsigned int)sizeof(WFA_vendor_ext_attr_id));
		return wsc_attr_success;
	}

	if (!memcmp(pkt, WFA_vendor_ext_attr_id, sizeof(WFA_vendor_ext_attr_id))) {
		pkt += 3; /*skip Vendor ID 0x00372A*/
		attr_len -= 3;
		while (attr_len) {
			attr = (struct wfa_subelements_attr *)pkt;
			if (attr_len < (sizeof(*attr) + attr->attribute_length)) {
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX" current total len (%d) is less than attr len (%d)\n", attr_len, (unsigned int)(sizeof(*attr) + attr->attribute_length));
				break;
			}

			if(attr->attribute == 0x06) {/*MAP extension attribute*/
				debug(DEBUG_TRACE, AUTO_CONFIG_PREX"get Muilt-AP Entension Attibute, value=%02x\n", attr->attribute_value[0]);
				temp_ctx->ap_config_data.map_vendor_extension = attr->attribute_value[0];
			} else if (attr->attribute == 0x00) {
				debug(DEBUG_TRACE, AUTO_CONFIG_PREX"get version2, value=%02x\n", attr->attribute_value[0]);
			} else {
				debug(DEBUG_TRACE, AUTO_CONFIG_PREX"unparsed WFA vendor extension subelements(%02x)\n", attr->attribute);
			}
			attr_len -= (sizeof(*attr) + attr->attribute_length);
			pkt += (sizeof(*attr) + attr->attribute_length);
		}
	} else if (!memcmp(pkt, MTK_vendor_ext_attr_id, sizeof(MTK_vendor_ext_attr_id))) {
		pkt += 3; /*skip Vendor ID 0x000CE7*/
		attr_len -= 3;
		while (attr_len) {
			attr = (struct wfa_subelements_attr *)pkt;
			if (attr_len < (sizeof(*attr) + attr->attribute_length)) {
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX" current total (%d) is less than attr len (%d)\n", attr_len, (unsigned int)(sizeof(*attr) + attr->attribute_length));
				break;
			}
			if(attr->attribute == 0x00) {/*hidden ssid attribute*/
				debug(DEBUG_TRACE, AUTO_CONFIG_PREX"get MTK Hidden Ssid Attibute, value=%02x\n", attr->attribute_value[0]);
				temp_ctx->ap_config_data.hidden_ssid= attr->attribute_value[0];
			}  else {
				debug(DEBUG_TRACE, AUTO_CONFIG_PREX"unparsed MTK vendor extension subelements(%02x)\n", attr->attribute);
			}
			attr_len -= (sizeof(*attr) + attr->attribute_length);
			pkt += (sizeof(*attr) + attr->attribute_length);
		}
	} else {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"unknown vendor id 0x%02x%02x%02x\n", pkt[0], pkt[1], pkt[2]);
	}

    return wsc_attr_success;
}

/* =========================================================================*/
/* common WSC attribute TLVs append function                                */
/* =========================================================================*/

WSC_ATTR_STATUS append_wsc_attr_tlv(
        unsigned char *pkt, unsigned char *inbuf, unsigned short *length,
        unsigned short tlv_type)
{
    unsigned char *temp_buf;
    WSC_TLV_HDR *hdr;
    unsigned short data_length = (*length);
    int index = 0;
    WSC_ATTR_STATUS status = wsc_attr_success;

    temp_buf = pkt;
    hdr = (WSC_TLV_HDR *)temp_buf;

    /*fill into tlv type*/
    hdr->type = host_to_be16(tlv_type);
    /*shift to data field*/
    temp_buf += ATTR_HEADR_LENGTH;

    /*use tlv_type to find matched fucntion, then create data field*/
    index = search_match_function_by_id(tlv_type);
    if(index >= 0)
    {
        status = wsc_func[index].create(temp_buf, inbuf, &data_length);
        if(status != wsc_attr_success)
        {
            debug(DEBUG_ERROR, "wsc_function error, type = 0x%04x\n", tlv_type);
            goto end;
        }
    }
    else
    {
        debug(DEBUG_ERROR, "not matched item in wsc_func, type = 0x%04x\n", tlv_type);
        status = wsc_attr_not_find;
        goto end;
    }

    /*fill into tlv length*/
    hdr->length = host_to_be16(data_length);

    *length = data_length + ATTR_HEADR_LENGTH;

end:
    return status;
}

/* =========================================================================*/
/* common WSC attribute TLVs parse function                                 */
/* =========================================================================*/
//extern int debug_level;
WSC_ATTR_STATUS parse_wsc_attr_tlv(
        unsigned char *pkt, void *pctx, unsigned short length)
{
    unsigned char *temp_buf;
    WSC_TLV_HDR *hdr;
    WSC_ATTR_STATUS status = wsc_attr_success;
    int index = 0;
    unsigned short total_len = 0, tlv_len = 0;
    unsigned short type = 0;
    unsigned char tlv_sequence_num = 0;
    unsigned char total_receive_tlv_num = 0;
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)pctx;
	unsigned char total_requested_tlv = 0;

    temp_buf = pkt;


    if (ctx->role == AGENT)
		total_requested_tlv = 21;
     else
	 	total_requested_tlv =22;

    /* if enrollee, need to parse authenticator to check integrity of
     * (M1 || M2*). so need to store M2*.(WSC 2.0.2 ch7.2)
     */
    if (ctx->role == AGENT) {
	    /* 1. calculate M2* length*/
	    ctx->current_rx_length = length - ATTR_AUTHENTICATOR_DATA_LENGTH - ATTR_HEADR_LENGTH;
	    /* 2. store M2* data to ctx->current_rx_data*/
	    ctx->current_rx_data = pkt;
    }

    /* the input parameter length will introduce the tlv length gotten from
     * cmdu tlv length field
     */
    while (length > total_len) {
        hdr = (WSC_TLV_HDR *)temp_buf;

        type = be_to_host16(hdr->type);
        tlv_len = be_to_host16(hdr->length);

    	debug(DEBUG_TRACE, AUTO_CONFIG_PREX"get wsc tlv type 0x%04x\n", type);
    	debug(DEBUG_TRACE, AUTO_CONFIG_PREX"get wsc tlv length 0x%04x\n", tlv_len);
        tlv_sequence_num++;

        index = search_match_function_by_id(type);
        if (index >= 0) {
			if (ctx->role == CONTROLLER) {
	            if ((wsc_func[index].M1_order != tlv_sequence_num) &&
	                (total_receive_tlv_num < total_requested_tlv)) {
	                debug(DEBUG_ERROR, AUTO_CONFIG_PREX"stop parse wsc because out of order\n");
	                status = wsc_attr_out_of_order;
	                goto end;
	            }
			} else {
				if ((wsc_func[index].M2_order != tlv_sequence_num) &&
					(total_receive_tlv_num < total_requested_tlv))
				{
					debug(DEBUG_ERROR, AUTO_CONFIG_PREX"stop parse wsc because out of order\n");
					status = wsc_attr_out_of_order;
					goto end;
				}
			}

            total_receive_tlv_num++;

            temp_buf += ATTR_HEADR_LENGTH;//shift to data field
            status = wsc_func[index].parse(temp_buf, ctx, &tlv_len);
            if (status != wsc_attr_success) {
                debug(DEBUG_ERROR, AUTO_CONFIG_PREX"wsc_function error, type = 0x%04x\n",type);
                goto end;
            }
        } else {
            debug(DEBUG_ERROR, AUTO_CONFIG_PREX"not matched item in parse wsc, type = 0x%04x\n",type);
            temp_buf += ATTR_HEADR_LENGTH;
            if (total_receive_tlv_num < total_requested_tlv) {
                debug(DEBUG_ERROR, AUTO_CONFIG_PREX"stop parse wsc because out of order\n");
                status = wsc_attr_out_of_order;
                goto end;
            }
        }
        temp_buf += tlv_len;
        total_len += (tlv_len + ATTR_HEADR_LENGTH);
    }

    /*enrollee need to check existence of authenticator tlv*/
	if (ctx->role == AGENT) {
	    if ((!ctx->is_authenticator_exist_in_M2) ||
	        (ctx->get_config_attr_kind !=((1<<HAS_SSID_ATTR)|(1<<HAS_AUTHENTICATION_TYPE_ATTR)|
	        (1<<HAS_ENCRYPTION_TYPE_ATTR)|(1<<HAS_NETWORK_KEY_ATTR)|(1<<HAS_MAC_ADDR_ATTR))))
	    {
	    	if (ctx->ap_config_data.map_vendor_extension & BIT_TEAR_DOWN) {
		        debug(DEBUG_TRACE, AUTO_CONFIG_PREX"tear down bit in encrypt setting, not check other encrypt info\n");
	    	} else {
	        	debug(DEBUG_ERROR, AUTO_CONFIG_PREX"is_authenticator_exist_in_M2 %d\n",ctx->is_authenticator_exist_in_M2);
	        	debug(DEBUG_ERROR, AUTO_CONFIG_PREX"get_config_attr_kind 0x%02x\n",ctx->get_config_attr_kind);
	        	status = wsc_attr_not_get_all_tlv;
	        	goto end;
	    	}
	    }
	}

end:
    return status;
}


