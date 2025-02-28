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

#ifndef __1905_cli_H__
#define __1905_cli_H__

#include "1905_ctrl.h"

#define CLI_CMD_LEN 256

struct _1905_cli_cmd {
	const char *cmd;
	int (*cmd_handler)(struct _1905_ctrl *ctrl, int argc, char *argv[]);
	const char *usage;
};

static int _1905_cli_cmd_help(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_mapfilter_version(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_cli_send_raw_data(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_cli_set_config(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_dump_topology_info(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_dump_topology_tree(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_dump_radio_info(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_set_log_level(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_eth_common_cmd(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_mapfilter_common_cmd(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_show_PON_dev(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_dump_security_info(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_set_key(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_set_security_config(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_set_sec_log_level(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_set_debug_level(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_ts_config(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_show_ts_info(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_ts_onoff(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_start_buffer(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_stop_buffer(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_get_frame_info(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_cli_send_bss_reconfig_trigger(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_show_sp(struct _1905_ctrl *ctrl, int argc, char *argv[]);
#endif /* __1905_cli_H__ */
