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
 * switch_legacy.h, it's used for <<switch 753x + upstream driver>>
 * 
 * Author: Sirui Zhao <Sirui.Zhao@mediatek.com>
 */
#ifndef SWITCH_MT753X_NL_H
#define SWITCH_MT753X_NL_H

#define MT753X_GENL_NAME "mt753x"
#define MT753X_GENL_VERSION 0X1

/*add your cmd to here*/
enum {
	MT753X_CMD_UNSPEC = 0, /*Reserved*/
	MT753X_CMD_REQUEST,    /*user->kernelrequest/get-response*/
	MT753X_CMD_REPLY,      /*kernel->user event*/
	MT753X_CMD_READ,
	MT753X_CMD_WRITE,
	__MT753X_CMD_MAX,
};
#define MT753X_CMD_MAX (__MT753X_CMD_MAX - 1)

/*define attr types */
enum
{
	MT753X_ATTR_TYPE_UNSPEC = 0,
	MT753X_ATTR_TYPE_MESG, /*MT753X message*/
	MT753X_ATTR_TYPE_PHY,
	MT753X_ATTR_TYPE_PHY_DEV,
	MT753X_ATTR_TYPE_REG,
	MT753X_ATTR_TYPE_VAL,
	MT753X_ATTR_TYPE_DEV_NAME,
	MT753X_ATTR_TYPE_DEV_ID,
	__MT753X_ATTR_TYPE_MAX,
};
#define MT753X_ATTR_TYPE_MAX (__MT753X_ATTR_TYPE_MAX - 1)

struct mt753x_attr {
	int port_num;
	int phy_dev;
	int reg;
	int value;
	int type;
	char op;
	char *dev_info;
	int dev_name;
	int dev_id;
};

#if defined (CONFIG_MT753X_GSW)
int mt753x_netlink_init(void);
int reg_read_nl(struct mt753x_attr *arg, int offset, int *value);
int reg_write_nl(struct mt753x_attr *arg, int offset, int value);
int netlink_cl22_read_phy(struct mt753x_attr *arg, int port_num, int phy_addr, int *value);
int switch_nl_deinit();
#else
static int mt753x_netlink_init(void) { return -1; }
static int reg_read_nl(struct mt753x_attr *arg, int offset, int *value) { return -1; }
static int reg_write_nl(struct mt753x_attr *arg, int offset, int value) { return -1; }
static int netlink_cl22_read_phy(struct mt753x_attr *arg, int port_num, int phy_addr, int *value) { return -1; }
static int switch_nl_deinit() { return -1; }
#endif
#endif
