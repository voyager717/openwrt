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
#include "p1905_managerd.h"
#include "debug.h"
#include "multi_ap.h"


#define ENCRYPT_LENGTH 512

WSC_ATTR_STATUS create_wsc_msg_M1(
    struct p1905_managerd_ctx *ctx, unsigned char *pkt, unsigned short *length)
{
    unsigned char *temp_buf, *p;
    unsigned char *inbuf = NULL;
    unsigned short temp_length = 0;
    int i = 0;
    WSC_ATTR_STATUS status = wsc_attr_success;
    DH_TABLE *dh_key = NULL;
    unsigned char WFA_vendor_ext_attr_id[3] = {0x00, 0x37, 0x2A};
	unsigned char band = 0;

    temp_buf = pkt;
    inbuf = malloc(256);
	if(inbuf == NULL) {
        status = wsc_attr_error;
        goto end0;
    }
	memset(inbuf, 0, 256);

	memset(&ctx->ap_config, 0, sizeof(ap_config_para));

    *inbuf = WSC_VERSION;
    temp_length = ATTR_VERSION_DATA_LENGTH;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_VERSION_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    *inbuf = MESSAGE_TYPE_M1;
    temp_length = ATTR_MESSAGE_TYPE_DATA_LENGTH;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_MESSAGE_TYPE_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    memcpy(inbuf, ctx->uuid, ATTR_UUID_E_DATA_LENGTH);
    temp_length = ATTR_UUID_E_DATA_LENGTH;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_UUID_E_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    memcpy(ctx->ap_config.enrolle_mac, ctx->p1905_al_mac_addr, 6);
    memcpy(inbuf, ctx->p1905_al_mac_addr, ATTR_MAC_ADDRESS_DATA_LENGTH);
    temp_length = ATTR_MAC_ADDRESS_DATA_LENGTH;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_MAC_ADDRESS_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    for(i=0;i<16;i++)
        ctx->ap_config.e_nonce[i] = ((os_random() % 255) + 1);
    memcpy(inbuf, ctx->ap_config.e_nonce, ATTR_ENROLLE_NONCE_DATA_LENGTH);
    temp_length = ATTR_ENROLLE_NONCE_DATA_LENGTH;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_ENROLLE_NONCE_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*calculate Diffie-Hellman key begin*/
    dh_key = (DH_TABLE *)malloc(sizeof(DH_TABLE));
	if (dh_key == NULL) {
		status = wsc_attr_error;
        goto end1;
	}
	memset(dh_key, 0, sizeof(DH_TABLE));
    if(wifi_utils_success != generate_DH_pub_priv_key(dh_key))
    {
        status = wsc_attr_error;
        goto end2;
    }
    memcpy(ctx->ap_config.private_key, dh_key->priv_key, 192);
    memcpy(ctx->ap_config.public_key , dh_key->pub_key, 192);

    memcpy(inbuf, dh_key->pub_key, ATTR_PUBLIC_KEY_DATA_LENGTH);
    temp_length = ATTR_PUBLIC_KEY_DATA_LENGTH;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_PUBLIC_KEY_ID);
    temp_buf += temp_length;
    (*length) += temp_length;
    /*calculate Diffie-Hellman key end*/

    /*. create authentication type flag*/
    temp_length = 2;//authentication type flag tlv has fixed length 2 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_AUTH_TYPE_FLAG_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create encryption type flag*/
    temp_length = 2;//encryption type flag tlv has fixed length 2 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_ENCRYPTION_TYPE_FLAG_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create connection type flag*/
    temp_length = 1;//connection type flag tlv has fixed length 1 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_CONNECTION_TYPE_FLAG_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create configuration method*/
    temp_length = 2;//configuration method tlv has fixed length 2 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_CONFIG_METHOD_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create WiFi simple configuration state*/
    temp_length = 1;//WiFi simple configuration state tlv has fixed length 1 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_WSC_STATE_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create manufacturer*/
    temp_length = 64;//manufacturer has max length 64 Bytes, implementation specified
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_MANUFACTURER_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create model name*/
    temp_length = 32;//model name has max length 64 Bytes, implementation specified
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_MODEL_NAME_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create model number*/
    temp_length = 32;//model number has max length 64 Bytes, implementation specified
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_MODEL_NUMBER_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create serial number*/
    temp_length = 32;//serial number has max length 64 Bytes, implementation specified
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_SERIAL_NUMBER_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create primary device type*/
    temp_length = 8;//primary device type has fixed length 8 bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_PRIM_DEV_TYPE_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create device name*/
    temp_length = 32;//device name has max length 64 Bytes, implementation specified
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_DEVICE_NAME_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create rf band*/
    temp_length = 1;//rf band tlv has fixed length 2 Bytes
    if (ctx->current_autoconfig_info.radio_index != -1) {
		band = ctx->rinfo[ctx->current_autoconfig_info.radio_index].band;
		if (band & BAND_2G_CAP)
			*inbuf = 0x01;
		else if ((band & BAND_5G_CAP))
			*inbuf = 0x02;
		else if ((band & BAND_6G_CAP))
			*inbuf = 0xff;
    } else {
		*inbuf = 0;
	}
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_RF_BAND_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create association state*/
    temp_length = 2;//association state tlv has fixed length 2 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_ASSOCIATION_STATE_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create device password id*/
    temp_length = 2;//device password id tlv has fixed length 2 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_DEVICE_PWD_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create configuration error*/
    temp_length = 2;//configuration error tlv has fixed length 2 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_CONFIG_ERROR_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create OS version*/
    temp_length = 4;//OS version tlv has fixed length 2 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_OS_VERSION_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create version2 (WFA vendor extension)*/
	temp_length = 0;
	memcpy(inbuf, WFA_vendor_ext_attr_id, 3);
	temp_length += 3;
	inbuf[temp_length++] = 0x00;
	inbuf[temp_length++] = 0x01;
	inbuf[temp_length++] = 0x20;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_VENDOR_EXTENSION_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*store M1 content to ctx->last_tx_data for authenticator tlv parsing*/
	if ((*length) > ctx->last_tx_buf_len) {
		p = os_malloc((*length));

		if (p) {
			os_free(ctx->last_tx_data);
			ctx->last_tx_data = p;
			ctx->last_tx_length = (*length);
			ctx->last_tx_buf_len = (*length);
		} else
			ctx->last_tx_length = 0;
	} else
		ctx->last_tx_length = (*length);

	os_memcpy(ctx->last_tx_data, pkt, ctx->last_tx_length);

end2:
	if (dh_key) {
    	free(dh_key);
	}
end1:
	if (inbuf) {
    	free(inbuf);
	}
end0:
    return status;
}

WSC_ATTR_STATUS create_wsc_msg_M2(
    struct p1905_managerd_ctx *ctx, unsigned char *pkt, unsigned short *length,
    WSC_CONFIG *config_data, unsigned char wfa_vendor_extension)
{
    unsigned char *temp_buf = NULL;
    unsigned char *inbuf = NULL;
    unsigned short temp_length = 0, encrypt_length = 0;
    int i = 0;
    WSC_ATTR_STATUS status = wsc_attr_success;
    DH_TABLE *dh_key = NULL;
    KDK_KDF_TABLE *kdk = NULL;
    KWA_TABLE *kwa = NULL, *authenticator = NULL;
    AES_TABLE *enc = NULL;
    unsigned char WFA_vendor_ext_attr_id[3] = {0x00, 0x37, 0x2A};
    unsigned char MTK_vendor_ext_attr_id[3] = {0x00, 0x0C, 0xE7};
	unsigned char vendor_buf[32] = {0};
	unsigned char vendor_len = 0;
	unsigned char r_nonce[16];
	struct agent_list_db *agent_info = NULL;
	struct agent_radio_info *agent_radio = NULL;
	unsigned char peer_rf_band = 0;

	find_agent_info(ctx, ctx->ap_config.enrolle_mac, &agent_info);
	if (!agent_info) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"Fail to find agent info\n");
		return wsc_attr_error;
	}

	agent_radio = find_agent_wsc_doing_radio(agent_info);
	if (!agent_radio) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"no ongoing radio for agent"MACSTR"\n",
			PRINT_MAC(agent_info->almac));
		return wsc_attr_error;
	}

	if (agent_radio->band & BAND_2G_CAP)
		peer_rf_band = 0x01;
	else if (agent_radio->band & BAND_5G_CAP)
		peer_rf_band = 0x02;
	else if (agent_radio->band & BAND_6G_CAP)
		peer_rf_band = 0xff;

    temp_buf = pkt;
    inbuf = (unsigned char *)malloc(ENCRYPT_LENGTH);
	if (inbuf == NULL) {
		return wsc_attr_error;
	}
	memset(inbuf, 0, ENCRYPT_LENGTH);

    *inbuf = WSC_VERSION;
    temp_length = ATTR_VERSION_DATA_LENGTH;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_VERSION_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    *inbuf = MESSAGE_TYPE_M2;
    temp_length = ATTR_MESSAGE_TYPE_DATA_LENGTH;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_MESSAGE_TYPE_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /* because registrar must send M2 after receive M1, so the ap_config has
     * already been allocated.
     */
    memcpy(inbuf, ctx->ap_config.e_nonce, ATTR_ENROLLE_NONCE_DATA_LENGTH);
    temp_length = ATTR_ENROLLE_NONCE_DATA_LENGTH;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_ENROLLE_NONCE_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

	for (i = 0; i < 16; i++)
		r_nonce[i] = ((os_random() % 255) + 1);
    memcpy(inbuf, r_nonce, ATTR_REGISTRAR_NONCE_DATA_LENGTH);
    temp_length = ATTR_REGISTRAR_NONCE_DATA_LENGTH;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_REGISTRAR_NONCE_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    memcpy(inbuf, ctx->uuid, ATTR_UUID_R_DATA_LENGTH);
    temp_length = ATTR_UUID_R_DATA_LENGTH;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_UUID_R_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*calculate Diffie-Hellman key begin*/
    dh_key = (DH_TABLE *)malloc(sizeof(DH_TABLE));
	if (dh_key == NULL) {
		status = wsc_attr_error;
        goto end;
	}
	memset(dh_key, 0, sizeof(DH_TABLE));

    if(wifi_utils_success != generate_DH_pub_priv_key(dh_key))
    {
        debug(DEBUG_ERROR, AUTO_CONFIG_PREX"generate DH public private key fail\n");
        status = wsc_attr_error;
        goto end0;
    }

    memcpy(inbuf, dh_key->pub_key, ATTR_PUBLIC_KEY_DATA_LENGTH);
    temp_length = ATTR_PUBLIC_KEY_DATA_LENGTH;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_PUBLIC_KEY_ID);
    temp_buf += temp_length;
    (*length) += temp_length;
    /*calculate security key*/
    memcpy(dh_key->pub_key, ctx->ap_config.peer_public_key, 192);
    if(wifi_utils_success != generate_DH_secu_key(dh_key))
    {
        debug(DEBUG_ERROR, AUTO_CONFIG_PREX"generate DH secure key fail\n");
        status = wsc_attr_error;
        goto end0;
    }
    /*calculate Diffie-Hellman key end*/

    /*calculate keywrap key & auth key by KDK begin*/
    kdk = (KDK_KDF_TABLE *)malloc(sizeof(KDK_KDF_TABLE));
	if (kdk == NULL) {
		status = wsc_attr_error;
        goto end0;
	}
	memcpy(kdk->DH_Secu_Key, dh_key->secu_key, 192);
	memcpy(kdk->E_Nonce, ctx->ap_config.e_nonce, 16);
	memcpy(kdk->R_Nonce, r_nonce, 16);
	memcpy(kdk->E_Mac_Addr, ctx->ap_config.enrolle_mac, 6);
	memset(dh_key, 0, sizeof(KDK_KDF_TABLE));
    if(wifi_utils_success != generate_auth_keywrap_key(kdk))
    {
    	debug(DEBUG_ERROR, AUTO_CONFIG_PREX"generate key wrap key fail\n");
        status = wsc_attr_error;
        goto end1;
    }

    /*calculate keywrap key & auth key by KDK end*/

    /*. create authentication type flag*/
    temp_length = 2;//authentication type flag tlv has fixed length 2 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_AUTH_TYPE_FLAG_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create encryption type flag*/
    temp_length = 2;//encryption type flag tlv has fixed length 2 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_ENCRYPTION_TYPE_FLAG_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create connection type flag*/
    temp_length = 1;//connection type flag tlv has fixed length 1 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_CONNECTION_TYPE_FLAG_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create configuration method*/
    temp_length = 2;//configuration method tlv has fixed length 2 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_CONFIG_METHOD_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create manufacturer*/
    temp_length = 64;//manufacturer has max length 64 Bytes, implementation specified
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_MANUFACTURER_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create model name*/
    temp_length = 32;//model name has max length 64 Bytes, implementation specified
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_MODEL_NAME_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create model number*/
    temp_length = 32;//model number has max length 64 Bytes, implementation specified
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_MODEL_NUMBER_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create serial number*/
    temp_length = 32;//serial number has max length 64 Bytes, implementation specified
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_SERIAL_NUMBER_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create primary device type*/
    temp_length = 8;//primary device type has fixed length 8 bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_PRIM_DEV_TYPE_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create device name*/
    temp_length = 32;//device name has max length 64 Bytes, implementation specified
    *inbuf = ctx->role;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_DEVICE_NAME_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create rf band*/
    temp_length = 1;//rf band tlv has fixed length 2 Bytes
	*inbuf = peer_rf_band;
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_RF_BAND_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create association state*/
    temp_length = 2;//association state tlv has fixed length 2 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_ASSOCIATION_STATE_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create configuration error*/
    temp_length = 2;//configuration error tlv has fixed length 2 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_CONFIG_ERROR_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create device password id*/
    temp_length = 2;//device password id tlv has fixed length 2 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_DEVICE_PWD_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*. create OS version*/
    temp_length = 4;//OS version tlv has fixed length 2 Bytes
    append_wsc_attr_tlv(temp_buf, inbuf, &temp_length, ATTR_OS_VERSION_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

    /*prepare content of encrypt settings tlv begin*/
    /*2. create ssid tlv*/
    /*need to check more, ASCII case/Hex case ......*/
    temp_length = strlen((char *)config_data->Ssid);
    append_wsc_attr_tlv(inbuf, config_data->Ssid, &temp_length, ATTR_SSID_ID);
    encrypt_length += temp_length;
    inbuf += temp_length;

    /*3. create authentication type tlv*/
    temp_length = ATTR_AUTHENTICATION_TYPE_DATA_LENGTH;

	/*insert agent entry*/

	if (agent_info->profile == MAP_PROFILE_R2 || (agent_info->profile == MAP_PROFILE_R1 && ctx->ap_config.auth_type))
		config_data->AuthMode &= ctx->ap_config.auth_type;  /*Mask peer authmode */
	append_wsc_attr_tlv(inbuf,(unsigned char *) &(config_data->AuthMode), &temp_length, ATTR_AUTHENTICATION_TYPE_ID);
    encrypt_length += temp_length;
    inbuf += temp_length;

    /*4. create encryption type tlv*/
    temp_length = ATTR_ENCRYPTION_TYPE_DATA_LENGTH;
    append_wsc_attr_tlv(inbuf,(unsigned char *) &(config_data->EncrypType), &temp_length, ATTR_ENCRYPTION_TYPE_ID);
    encrypt_length += temp_length;
    inbuf += temp_length;

    /*5. create network key tlv*/
    if(config_data->AuthMode == AUTH_OPEN && config_data->EncrypType == ENCRYP_NONE)
        temp_length = 0;
    else //we need to check more about Hex network key
        temp_length = strlen((char *)config_data->WPAKey);

    append_wsc_attr_tlv(inbuf, config_data->WPAKey, &temp_length, ATTR_NETWORK_KEY_ID);
    encrypt_length += temp_length;
    inbuf += temp_length;

    /*6. create mac address tlv*/
    temp_length = ATTR_MAC_ADDRESS_DATA_LENGTH;
    append_wsc_attr_tlv(inbuf, ctx->ap_config.enrolle_mac, &temp_length, ATTR_MAC_ADDRESS_ID);
    encrypt_length += temp_length;
    inbuf += temp_length;

	/* wfa vendor extension data*/
	os_memcpy(vendor_buf, WFA_vendor_ext_attr_id, 3);
	vendor_len += 3;
	vendor_buf[vendor_len++] = 0x06;
	vendor_buf[vendor_len++] = 0x01;
	vendor_buf[vendor_len++] = wfa_vendor_extension;
	vendor_buf[vendor_len++] = 0x00;
	vendor_buf[vendor_len++] = 0x01;
	vendor_buf[vendor_len++] = 0x20;
	temp_length = vendor_len;
    append_wsc_attr_tlv(inbuf, vendor_buf, &temp_length, ATTR_VENDOR_EXTENSION_ID);
	encrypt_length += temp_length;
    inbuf += temp_length;
	debug(DEBUG_ERROR, AUTO_CONFIG_PREX"create wfa_vendor_extension(%02x) for ssid(%s)\n", wfa_vendor_extension,config_data->Ssid);
	hex_dump("multi-ap ie", (inbuf - temp_length), temp_length);

	/* mtk vendor extension data*/
	if (config_data->hidden_ssid == 1) {
		vendor_len = 0;
		os_memcpy(vendor_buf, MTK_vendor_ext_attr_id, 3);
		vendor_len += 3;
		vendor_buf[vendor_len++] = 0x00;
		vendor_buf[vendor_len++] = 0x01;
		vendor_buf[vendor_len++] = config_data->hidden_ssid;
		temp_length = vendor_len;
		append_wsc_attr_tlv(inbuf, vendor_buf, &temp_length, ATTR_VENDOR_EXTENSION_ID);
		encrypt_length += temp_length;
	    inbuf += temp_length;
	}

    /*7. create key wrap authenticator tlv*/
    /*back to inbuf begining to calculate KWA*/
    inbuf -= encrypt_length;
    kwa = (KWA_TABLE *)malloc(sizeof(KWA_TABLE));
	if (kwa == NULL) {
		status = wsc_attr_error;
		goto end3;
	}
	memset(kwa, 0, sizeof(KWA_TABLE));
    kwa->EncrptData = malloc(encrypt_length);
	if (kwa->EncrptData == NULL) {
		status = wsc_attr_error;
		goto end3;
	}
	memset(kwa->EncrptData, 0, encrypt_length);
    memcpy(kwa->EncrptData, inbuf, encrypt_length);
    kwa->EncrptDataLen = encrypt_length;
    memcpy(kwa->AuthKey, kdk->AuthKey, 32);
    temp_length = ATTR_KWA_LENGTH;
    /*return to tail of inbuf. KWA tlv needs to be appended*/
    inbuf += encrypt_length;
    append_wsc_attr_tlv(inbuf, (unsigned char *)kwa, &temp_length,
                        ATTR_KEY_WRAP_AUTHENTICATOR_ID);
    inbuf += temp_length;
    encrypt_length += temp_length;

    /*8. create encrypt settings tlv*/
    inbuf -= encrypt_length; //back to inbuf begining to encrypt
    enc = (AES_TABLE *)malloc(sizeof(AES_TABLE));
	if (enc == NULL) {
		status = wsc_attr_error;
		goto end3;
	}
	memset(enc, 0, sizeof(AES_TABLE));
    enc->PlainText = inbuf;
    enc->PlainTextLen = encrypt_length;

    for(i=0;i<16;i++)
        enc->IV[i] = ((os_random() % 255) + 1);
    enc->CipherText = malloc(ENCRYPT_LENGTH -16);
	if (enc->CipherText == NULL) {
		status = wsc_attr_error;
		goto end3;
	}
    enc->CipherTextLen = ENCRYPT_LENGTH -16;
	memset(enc->CipherText, 0, enc->CipherTextLen);
    memcpy(enc->KeyWrapKey, kdk->KeyWrapKey, 16);
    temp_length = 0;
    if(wsc_attr_success != append_wsc_attr_tlv(temp_buf, (unsigned char *)enc, &temp_length,
                        ATTR_ENCRYPTED_SETTINGS_ID))
    {
    	status = wsc_attr_error;
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX"append encryption setting fail\n");
        goto end3;
	}
    temp_buf += temp_length;
    (*length) += temp_length;
    /*prepare content of encrypt settings tlv end*/
	hex_dump_info("PlainText",enc->PlainText,enc->PlainTextLen);
	hex_dump_info("CipherText",enc->CipherText,enc->CipherTextLen);
	hex_dump_info("IV",enc->IV,16);
	debug(DEBUG_TRACE, AUTO_CONFIG_PREX"wireless setting added\n");

    /*create authenticator tlv*/
    /* 1. create (M1 || M2*) */
    authenticator = (KWA_TABLE *)malloc(sizeof(KWA_TABLE));
	if (authenticator == NULL) {
		status = wsc_attr_error;
		goto end3;
	}
	memset(authenticator, 0, sizeof(KWA_TABLE));
    authenticator->EncrptData = malloc(ctx->last_rx_length + (*length));
	if (authenticator->EncrptData == NULL) {
		status = wsc_attr_error;
		goto end4;
	}
	memset(authenticator->EncrptData, 0, (ctx->last_rx_length + (*length)));
    memcpy(authenticator->EncrptData, ctx->last_rx_data, ctx->last_rx_length);
    authenticator->EncrptData += ctx->last_rx_length;
    memcpy(authenticator->EncrptData, pkt, (*length));
    authenticator->EncrptData -= ctx->last_rx_length;
    authenticator->EncrptDataLen = ctx->last_rx_length + (*length);
    /* 2. default key is auth key */
    memcpy(authenticator->AuthKey, kdk->AuthKey, 32);
    /* 3. HMAC-SHA-256(M1 || M2*), key is auth key, then append*/
    temp_length = ATTR_AUTHENTICATOR_DATA_LENGTH;
    append_wsc_attr_tlv(temp_buf, (unsigned char *)authenticator, &temp_length,
                        ATTR_AUTHENTICATOR_ID);
    temp_buf += temp_length;
    (*length) += temp_length;

end4:
	if (authenticator) {
		if (authenticator->EncrptData)
    		free(authenticator->EncrptData);
    	free(authenticator);
	}
end3:
	if (enc) {
		if (enc->CipherText)
    		free(enc->CipherText);
    	free(enc);
	}
	if (kwa) {
		if (kwa->EncrptData)
    		free(kwa->EncrptData);
    	free(kwa);
	}
end1:
	if (kdk) {
    	free(kdk);
	}
end0:
	if (dh_key) {
    	free(dh_key);
	}
end:
	if (inbuf) {
    	free(inbuf);
	}
    return status;
}

WSC_ATTR_STATUS parse_wsc_msg(
    struct p1905_managerd_ctx *ctx, unsigned char *pkt, unsigned short length)
{
    /* if AP is registrar, ctx->ap_config will be allocated here.
     * we need to prepare ctx->ap_config to store exchaged data from M1.
     */
	 unsigned char *p = NULL;

	if (ctx->role == CONTROLLER) {
		os_memset(&ctx->ap_config, 0, sizeof(ap_config_para));
	    /*
	    	for authenticator tlv use, we need to store M1 content for future use.
	     	*/

		if (length > ctx->last_rx_buf_len) {
			p = os_realloc(ctx->last_rx_data, length);
			if (p) {
				ctx->last_rx_data = p;
				ctx->last_rx_length = length;
				ctx->last_rx_buf_len = length;
			} else
				ctx->last_rx_length = 0;
		} else {
			ctx->last_rx_length = length;
		}
		os_memcpy(ctx->last_rx_data, pkt, ctx->last_rx_length);
	}

    if (wsc_attr_success != parse_wsc_attr_tlv(pkt, (void *)ctx, length)) {
        debug(DEBUG_ERROR, AUTO_CONFIG_PREX"parse_wsc_attr_tlv error\n");
        return wsc_attr_error;
    }

    return wsc_attr_success;
}


