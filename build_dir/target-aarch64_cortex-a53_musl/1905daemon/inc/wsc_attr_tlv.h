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

#ifndef WSC_ATTR_TLV_H
#define WSC_ATTR_TLV_H

#include "linux/types.h"
#include "p1905_managerd.h"

#define ATTR_TYPE_FIELD_LENGTH               2
#define ATTR_LENGTH_FIELD_LENGTH             2
#define ATTR_HEADR_LENGTH   (ATTR_TYPE_FIELD_LENGTH + ATTR_LENGTH_FIELD_LENGTH)

/*define wsc attribute ID*/
#define ATTR_ASSOCIATION_STATE_ID            0x1002
#define ATTR_AUTHENTICATION_TYPE_ID          0x1003
#define ATTR_AUTH_TYPE_FLAG_ID               0x1004
#define ATTR_AUTHENTICATOR_ID                0x1005
#define ATTR_CONFIG_METHOD_ID                0x1008
#define ATTR_CONFIG_ERROR_ID                 0x1009
#define ATTR_CONNECTION_TYPE_FLAG_ID         0x100D
#define ATTR_ENCRYPTION_TYPE_ID              0x100F
#define ATTR_ENCRYPTION_TYPE_FLAG_ID         0x1010
#define ATTR_DEVICE_NAME_ID                  0x1011
#define ATTR_DEVICE_PWD_ID                   0x1012
#define ATTR_ENCRYPTED_SETTINGS_ID           0x1018
#define ATTR_ENROLLE_NONCE_ID                0x101A
#define ATTR_KEY_WRAP_AUTHENTICATOR_ID       0x101E
#define ATTR_MAC_ADDRESS_ID                  0x1020
#define ATTR_MANUFACTURER_ID                 0x1021
#define ATTR_MESSAGE_TYPE_ID                 0x1022
#define ATTR_MODEL_NAME_ID                   0x1023
#define ATTR_MODEL_NUMBER_ID                 0x1024
#define ATTR_NETWORK_KEY_ID                  0x1027
#define ATTR_OS_VERSION_ID                   0x102D
#define ATTR_PUBLIC_KEY_ID                   0x1032
#define ATTR_REGISTRAR_NONCE_ID              0x1039
#define ATTR_RF_BAND_ID                      0x103C
#define ATTR_SERIAL_NUMBER_ID                0x1042
#define ATTR_WSC_STATE_ID                    0x1044
#define ATTR_SSID_ID                         0x1045
#define ATTR_UUID_E_ID                       0x1047
#define ATTR_UUID_R_ID                       0x1048
#define ATTR_VENDOR_EXTENSION_ID             0x1049
#define ATTR_VERSION_ID                      0x104A
#define ATTR_PRIM_DEV_TYPE_ID                0x1054

/*define wsc attribute length*/
#define ATTR_VERSION_DATA_LENGTH             1
#define ATTR_MESSAGE_TYPE_DATA_LENGTH        1
#define ATTR_MAC_ADDRESS_DATA_LENGTH         6
#define ATTR_ENROLLE_NONCE_DATA_LENGTH       16
#define ATTR_REGISTRAR_NONCE_DATA_LENGTH     16
#define ATTR_PUBLIC_KEY_DATA_LENGTH          192
#define ATTR_KWA_LENGTH                      8
#define ATTR_AUTHENTICATION_TYPE_DATA_LENGTH 2
#define ATTR_ENCRYPTION_TYPE_DATA_LENGTH     2
#define ATTR_UUID_E_DATA_LENGTH              16
#define ATTR_UUID_R_DATA_LENGTH              16
#define ATTR_AUTHENTICATOR_DATA_LENGTH       8

/*define message type*/
#define MESSAGE_TYPE_M1                      0x04
#define MESSAGE_TYPE_M2                      0x05

/*define version*/
#define WSC_VERSION                          0x10

typedef enum
{
    wsc_attr_success = 0,
    wsc_attr_error,
    wsc_attr_not_find,
    wsc_attr_not_expect,
    wsc_attr_out_of_order,
    wsc_attr_not_get_all_tlv,

} WSC_ATTR_STATUS;

typedef struct
{
    __be16 type;
    __be16 length;
}__attribute__ ((__packed__)) WSC_TLV_HDR;

typedef struct _WSC_ATTR_FUNC
{
    unsigned short id;
    unsigned char M1_order;
    unsigned char M2_order;

    WSC_ATTR_STATUS (*create) (unsigned char *out, unsigned char *in, unsigned short *length);
    WSC_ATTR_STATUS (*parse) (unsigned char *in, void *ctx, unsigned short *length);

} WSC_ATTR_FUNC;

WSC_ATTR_STATUS append_wsc_attr_tlv(
    unsigned char *pkt, unsigned char *inbuf, unsigned short *length,
    unsigned short tlv_type);
WSC_ATTR_STATUS parse_wsc_attr_tlv(
   	unsigned char *pkt, void *pctx, unsigned short length);

#endif /* WSC_ATTR_TLV_H */
