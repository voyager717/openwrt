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
 * switch_legacy_ioctl.c, it's used for <7530 + raeth>
 *
 * Author: Sirui Zhao <Sirui.Zhao@mediatek.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <pthread.h>

#include "ra_ioctl.h"
#include "ethernet_layer.h"
#include "debug.h"
#include "netlink_event.h"
#include "common.h"

#ifndef CONFIG_SUPPORT_OPENWRT
#include <linux/autoconf.h>
#endif

static int esw_fd;
static char eth_dev_name[IFNAMSIZ] = {0x00};

#if defined (CONFIG_RALINK_MT7628)
/****************************************************
*
*Function name : reg_read_ioctl()
*Description   : register read for old switch chip like
				 7628 switch and so on
*Parameter	:
*		@offset : register address offset, see chip CR
*		@value  : output the register value
*Return:
*		0: Success, Other fail
*other:
*		for 7628 switch use
***************************************************/

int reg_read_ioctl(int offset, int *value) {
	struct ifreq ifr;
	struct esw_reg reg;

	if (value == NULL)
		return -1;
	reg.off = offset;
	strncpy(ifr.ifr_name, eth_dev_name, os_strlen(eth_dev_name) + 1);
	ifr.ifr_data = &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_REG_READ, &ifr)) {
		perror("error: ioctl: invalid ioctl cmd");
		close(esw_fd);
		return -1;
	}
	*value = reg.val;
	return 0;
}

/****************************************************
*
*Function name : reg_write_ioctl()
*Description   : register write for old switch chip like
				 7628 switch and so on
*Parameter	:
*		@offset : register address offset, see chip CR
*		@value  : input the register value
*Return:
*		0: Success, Other fail
*other:
*		for 7628 switch use
***************************************************/

int reg_write_ioctl(int offset, int value)
{
	struct ifreq ifr;
	struct esw_reg reg;

	reg.off = offset;
	reg.val = value;
	strncpy(ifr.ifr_name, eth_dev_name, os_strlen(eth_dev_name) + 1);
	ifr.ifr_data = &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_REG_WRITE, &ifr)) {
		perror("error: ioctl: invalid ioctl cmd");
		close(esw_fd);
		return -1;
	}

	return 0;
}

#else
int reg_read_ioctl(int offset, int *value)
{
	struct ifreq ifr;
	struct ra_mii_ioctl_data mii;

	strncpy(ifr.ifr_name, eth_dev_name, os_strlen(eth_dev_name) + 1);
	ifr.ifr_data = &mii;

	mii.phy_id = 0x1f;
	mii.reg_num = offset;

	if (-1 == ioctl(esw_fd, RAETH_MII_READ, &ifr)) {
		debug(DEBUG_ERROR, "ioctl fail");
		close(esw_fd);
		exit(0);
	}
	*value = mii.val_out;
	return 0;
}

int reg_write_ioctl(int offset, int value)
{
	struct ifreq ifr;
	struct ra_mii_ioctl_data mii;

	strncpy(ifr.ifr_name, eth_dev_name, os_strlen(eth_dev_name) + 1);
	ifr.ifr_data = &mii;

	mii.phy_id = 0x1f;
	mii.reg_num = offset;
	mii.val_in = value;

	if (-1 == ioctl(esw_fd, RAETH_MII_WRITE, &ifr)) {
		debug(DEBUG_ERROR, "ioctl fail");
		close(esw_fd);
		exit(0);
	}
	return 0;
}

#endif

/*host_bus access internal phy*/
int mii_cl22_read_phy(int phy_num, int phy_reg, int *status)
{
	int method = 0, ret = 0;
	struct ifreq ifr;
	struct ra_mii_ioctl_data mii;

	os_strncpy(ifr.ifr_name, eth_dev_name, os_strlen(eth_dev_name) + 1);
	ifr.ifr_data = &mii;

	mii.phy_id = phy_num;
	mii.reg_num = phy_reg;		/*default value*/
	method = RAETH_MII_READ;	/*read*/

	ret = ioctl(esw_fd, method, &ifr);
	if (ret < 0) {
		debug(DEBUG_ERROR, "mii_mgr_x: ioctl error\n");
		return -1;
	}
	*status = mii.val_out;

	return 0;
}

/*host_bus access external phy*/
int mii_cl45_read_phy(int phy_num, int dev, int phy_reg, int *status)
{
	int method = 0, ret = 0;
	struct ifreq ifr;
	struct ra_mii_ioctl_data mii;

	os_strncpy(ifr.ifr_name, eth_dev_name, os_strlen(eth_dev_name) + 1);
	ifr.ifr_data = &mii;

	mii.port_num = phy_num;
	mii.dev_addr = dev;
	mii.reg_addr = phy_reg;
	method = RAETH_MII_READ_CL45;
	ret = ioctl(esw_fd, method, &ifr);
	if (ret < 0) {
		debug(DEBUG_ERROR, "mii_mgr_cl45: ioctl error\n");
		return -1;
	}
	*status = mii.val_out;

	return 0;
}


int switch_ioctl_init(void *context)
{
	char *eth_name = (char*)context;
	os_memset(eth_dev_name, 0, sizeof(eth_dev_name));

	esw_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (esw_fd < 0) {
		perror("socket");
		return -1;
	}

	os_memset(eth_dev_name, 0, sizeof(eth_dev_name));

	if (eth_name) {
		if (os_strlen(eth_name) >= IFNAMSIZ) {
			debug(DEBUG_ERROR, "error, eth_name charactor length larger than IFNAMSIZ\n");
			return -1;
		}
		(void)snprintf(eth_dev_name, sizeof(eth_dev_name), "%s", eth_name);
	} else {
#ifndef CONFIG_SUPPORT_OPENWRT
		snprintf(eth_dev_name, sizeof(eth_dev_name), "%s", "eth2");
#else
		snprintf(eth_dev_name, sizeof(eth_dev_name), "%s", "eth0");
#endif
	}

	debug(DEBUG_TRACE, "ethernet dev name %s\n", eth_dev_name);
	return 0;
}

int switch_ioctl_deinit(void *context)
{
	os_memset(eth_dev_name, 0, sizeof(eth_dev_name));
	close(esw_fd);
	return 0;
}
