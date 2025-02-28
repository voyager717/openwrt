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
#include <stdlib.h>
#include <string.h>
#include "1905_ctrl.h"
#include "1905_cli.h"
#include "os.h"

static int _1905_set_debug_level(struct _1905_ctrl *ctrl, int argc, char *argv[]);
static int _1905_dump_security_info(struct _1905_ctrl *ctrl, int argc, char *argv[]);

static struct _1905_ctrl *ctrl_conn;
static struct _1905_cli_cmd cli_cmds[] = {
	{"help", _1905_cli_cmd_help, "command usage"},		
	{"version", _1905_mapfilter_version, "show 1905 and mapfilter version"},
	{"dev_send_1905", _1905_cli_send_raw_data, "send 1905 raw data to 1905 daemon"},
	{"dev_set_config", _1905_cli_set_config, "set bss info to 1905 controller"},
	{"dump_topology_info", _1905_dump_topology_info, "dump topology info"},
	{"dump_topology_tree", _1905_dump_topology_tree, "dump topology tree"},
	{"dump_radio_info", _1905_dump_radio_info, "dump topology tree"},
	{"log_level", _1905_set_log_level, "set log level"},
	{"get_eth_port_type", _1905_eth_common_cmd, "get types of all ports"},
	{"get_eth_entry", _1905_eth_common_cmd, "get all ethernet entries"},
	{"find_eth_entry", _1905_eth_common_cmd, "find a specific entry from ethernet entry table"},	
	{"get_eth_port_status", _1905_eth_common_cmd, "get status of a specific eth port"},
	{"set_eth_ts_vlan_id", _1905_eth_common_cmd, "set traffic separation vlan to switch"},
	{"set_eth_trans_vlan_id", _1905_eth_common_cmd, "set transparent vlan to switch"},
	{"clear_switch_table", _1905_eth_common_cmd, "clear switch table"},
	{"mapfilter_debug", _1905_eth_common_cmd, "get status of a specific eth port"},
	{"mapfilter_set_primary", _1905_mapfilter_common_cmd, "get status of a specific eth port"},
	{"mapfilter_set_up_path", _1905_mapfilter_common_cmd, "get status of a specific eth port"},
	{"show_PON_dev", _1905_show_PON_dev, "show device connect to the PON"},
	{"dump_security_info", _1905_dump_security_info, "dump security_info"},
	{"set_key", _1905_set_key, "set key"},
	{"set_sec_config", _1905_set_security_config, "set security config, 0x01-enable multicast encrypt, 0x02-enable unicast encrypt"},
	{"set_sec_log_level", _1905_set_sec_log_level, "set security log level"},
	{"sec_log_level", _1905_set_debug_level, "set security log level"},
	{"set_debug_level", _1905_set_debug_level, "set debug level"},
	{"set_ts_config", _1905_ts_config, "set primary vlan and traffic separation policy"},
	{"show_ts_info", _1905_show_ts_info, "show traffic separation information"},
	{"ts_onoff", _1905_ts_onoff, "enable or disable traffic separation in mapfilter"},
	{"dev_start_buffer", _1905_start_buffer, "start to buffer 1905 packets"},
    {"dev_stop_buffer", _1905_stop_buffer, "stop to buffer 1905 packets"},
    {"dev_get_frame_info", _1905_get_frame_info, "get specified packets or tlvs from buffered packets"},
	{"show_sp", _1905_show_sp, "show service prioritization information in 1905daemon"},
	{"send_bss_reconfig_trigger", _1905_cli_send_bss_reconfig_trigger, "send bss reconfig trigger msg to Agent(at most 3 agents)"},
};


static int _1905_cli_open_connection(const char *ctrl_path)
{
	ctrl_conn = _1905_ctrl_open(ctrl_path);

	if (!ctrl_conn) {
		printf("_1905_cli_open fail\n");
		return -1;
	}

	return 0;
}

static void _1905_cli_close_connection(void)
{
	_1905_ctrl_close(ctrl_conn);
	ctrl_conn = NULL;
}

static int _1905_cli_cmd_help(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	struct _1905_cli_cmd *cmd;
	int cmd_num = sizeof(cli_cmds) / sizeof(struct _1905_cli_cmd);
	int i;

	cmd = cli_cmds;
	printf("1905ctrl [role] [cmd] [args]\n");
	printf("Command Usage:\n");

	for (i = 0; i < cmd_num; i++, cmd++) {
			printf("  %-60s %-50s\n", cmd->cmd, cmd->usage);
	}

	return 0;
}

static int _1905_mapfilter_version(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret = 0;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, CLI_CMD_LEN);

	ret = os_snprintf(&cmd[i], len, "%s", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;

}


static int _1905_cli_send_raw_data(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret = 0;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, len);

	ret = os_snprintf(&cmd[i], len, "%s ", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	i += strlen(argv[0]) + 1;
	len -= ret;
	ret = os_snprintf(&cmd[i], len, "%s", argv[1]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_cli_set_config(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret = 0;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, len);

	ret = os_snprintf(&cmd[i], len, "%s ", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	i += strlen(argv[0]) + 1;
	len -= ret;
	ret = os_snprintf(&cmd[i], len, "%s", argv[1]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_dump_topology_info(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret = 0;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, len);

	ret = os_snprintf(&cmd[i], len, "%s", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_dump_topology_tree(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret = 0;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, len);

	ret = os_snprintf(&cmd[i], len, "%s", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_dump_radio_info(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret = 0;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, len);

	ret = os_snprintf(&cmd[i], len, "%s", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}


static int _1905_set_log_level(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret = 0;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, len);

	ret = os_snprintf(&cmd[i], len, "%s ", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	i += strlen(argv[0]) + 1;
	len -= ret;
	ret = os_snprintf(&cmd[i], len, "%s", argv[1]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_eth_common_cmd(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret = 0;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, len);

	ret = os_snprintf(&cmd[i], len, "%s", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	i += strlen(argv[0]);
	len -= ret;
	if (argv[1]) {
		ret = os_snprintf(&cmd[i], len, "%s", argv[1]);
		if (os_snprintf_error(len, ret))
			return -1;
	}

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_mapfilter_common_cmd(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret = 0;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, CLI_CMD_LEN);

	ret = os_snprintf(&cmd[i], len, "%s", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	i += strlen(argv[0]);
	len -= ret;
	if (argv[1]) {
		ret = os_snprintf(&cmd[i], len, " %s", argv[1]);
		if (os_snprintf_error(len, ret))
			return -1;

		i += strlen(argv[1]) + 1;
		len -= ret;
	}
	if (argv[2]) {
		ret = os_snprintf(&cmd[i], len, " %s", argv[2]);
		if (os_snprintf_error(len, ret))
			return -1;
	}

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_show_PON_dev(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret = 0;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, CLI_CMD_LEN);

	ret = os_snprintf(&cmd[i], len, "%s", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	i += strlen(argv[0]);
	len -= ret;
	if (argv[1]) {
		ret = os_snprintf(&cmd[i], len, " %s", argv[1]);
		if (os_snprintf_error(len, ret))
			return -1;
		i += strlen(argv[1]) + 1;
		len -= ret;
	}
	if (argv[2]) {
		ret = os_snprintf(&cmd[i], len, " %s", argv[2]);
		if (os_snprintf_error(len, ret))
			return -1;
	}

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_dump_security_info(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret = 0;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, CLI_CMD_LEN);

	ret = os_snprintf(&cmd[i], len, "%s", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	if (ret == 0) {
		struct timeval tv;
		int res;
		fd_set rfds;
		char *reply = NULL;
		const int reply_len = 10240;

		errno = 0;
		reply = malloc(reply_len);
		if (!reply)
			return -1;
		memset(reply, 0, reply_len);

		for ( ; ; ) {
			tv.tv_sec = 10;
			tv.tv_usec = 0;
			FD_ZERO(&rfds);
			FD_SET(ctrl->s, &rfds);
			res = select(ctrl->s + 1, &rfds, NULL, NULL, &tv);
			if (res < 0 && errno == EINTR)
				continue;
			if (res < 0) {
				free(reply);
				return res;
			}
			if (FD_ISSET(ctrl->s, &rfds)) {
				res = recv(ctrl->s, reply, reply_len, 0);
				if (res < 0) {
					free(reply);
					return res;
				}
				if (res > 0) {
					if (res == reply_len)
						res = reply_len - 1;
					reply[res] = '\0';
					printf("%s\n", reply);
					free(reply);
					break;
				}
			}
		}
	}

	return ret;
}

static int _1905_set_key(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret = 0;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, CLI_CMD_LEN);

	ret = os_snprintf(&cmd[i], len, "%s ", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	i += strlen(argv[0]) + 1;
	len -= ret;
	ret = os_snprintf(&cmd[i], len, "%s ", argv[1]); /*almac*/
	if (os_snprintf_error(len, ret))
		return -1;

	i += strlen(argv[1]) + 1;
	len -= ret;
	ret = os_snprintf(&cmd[i], len, "%s ", argv[2]);  /*key*/
	if (os_snprintf_error(len, ret))
		return -1;

	i += strlen(argv[2]) + 1;
	len -= ret;
	ret = os_snprintf(&cmd[i], len, "%s", argv[3]);  /*key*/
	if (os_snprintf_error(len, ret))
		return -1;


	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

/*1905ctrl agent/controller _1905_set_security_config 0-disabled,1-multicast enable,2-unicast enable,3-both enable*/
static int _1905_set_security_config(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret = 0;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, CLI_CMD_LEN);

	ret = os_snprintf(&cmd[i], sizeof(cmd), "%s %s", argv[0], argv[1]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_set_sec_log_level(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN];
	int i = 0, ret;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, len);

	ret = os_snprintf(&cmd[i], len, "%s %s", argv[0], argv[1]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}


static int _1905_set_debug_level(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, CLI_CMD_LEN);

	ret = os_snprintf(&cmd[i], len, "%s %s", argv[0], argv[1]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

/*
**	1. set_ts_config pvlan <pvid>
**	2. set_ts_config policy {<ssid> <vid>} |
				<clean> |
				<done>
*/
static int _1905_ts_config(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, CLI_CMD_LEN);

	ret = os_snprintf(&cmd[i], len, "%s ", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	i += strlen(argv[0]) + 1;
	len -= ret;
	if (argc < 3)
		return -1;

	if (!strcmp(argv[1], "pvlan")) {
		/* string "primary vlan" */
		ret = os_snprintf(&cmd[i], len, "%s ", argv[1]);
		if (os_snprintf_error(len, ret))
			return -1;

		i += strlen(argv[1]) + 1;
		len -= ret;
		/* primary vlan id */
		ret = os_snprintf(&cmd[i], len, "%s ", argv[2]);
		if (os_snprintf_error(len, ret))
			return -1;

		i += strlen(argv[2]) + 1;
		len -= ret;
	} else if (!strcmp(argv[1], "policy")) {
		/* string "policy" */
		ret = os_snprintf(&cmd[i], len, "%s ", argv[1]);
		if (os_snprintf_error(len, ret))
			return -1;

		i += strlen(argv[1]) + 1;
		len -= ret;
		if ((!strcmp(argv[2], "clean")) ||
			(!strcmp(argv[2], "done"))) {
			/* string "policy" */
			ret = os_snprintf(&cmd[i], len, "%s ", argv[2]);
			if (os_snprintf_error(len, ret))
				return -1;

			i += strlen(argv[2]) + 1;
			len -= ret;
		}
		else {
			/* ssid */
			ret = os_snprintf(&cmd[i], len, "%s ", argv[2]);
			if (os_snprintf_error(len, ret))
				return -1;

			i += strlen(argv[2]) + 1;
			len -= ret;
			/* vlan id */
			ret = os_snprintf(&cmd[i], len, "%s ", argv[3]);
			if (os_snprintf_error(len, ret))
				return -1;

			i += strlen(argv[3]) + 1;
		}
	} else {
		printf("not supported ts cmd %s\n", __func__);
	}

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_show_ts_info(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN] = {0};
	int i = 0, ret;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, CLI_CMD_LEN);

	ret = os_snprintf(&cmd[i], len, "%s", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_show_sp(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN];
	int i = 0, ret;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, len);

	ret = os_snprintf(&cmd[i], len, "%s", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_ts_onoff(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN];
	int i = 0, ret;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, len);

	ret = os_snprintf(&cmd[i], len, "%s %s", argv[0], argv[1]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}
static int _1905_start_buffer(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN];
	int i = 0, ret;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, len);

	ret = os_snprintf(&cmd[i], len, "%s %s", argv[0], argv[1]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_stop_buffer(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN];
	int i = 0, ret;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, len);

	ret = os_snprintf(&cmd[i], sizeof(cmd), "%s", argv[0]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}


static int _1905_get_frame_info(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN];
	int ret = -1;
	unsigned int len = CLI_CMD_LEN;

	if (argc < 6)
		return ret;

	memset(cmd, 0, len);

	ret = os_snprintf(cmd, len, "%s %s %s %s %s %s", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
	if (os_snprintf_error(len, ret))
		return -1;

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_cli_send_bss_reconfig_trigger(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[CLI_CMD_LEN];
	int i = 0, offset = 0, ret;
	unsigned int len = CLI_CMD_LEN;

	memset(cmd, 0, len);

	if (argc > 4) {
		argc = 4;
		printf("%s support at most 3 agents\n", __func__);
	}

	for (i = 0; i < argc; i ++) {
		ret = os_snprintf(&cmd[offset], len - offset, "%s ", argv[i]);
		if (os_snprintf_error(len, ret))
			return -1;
		offset += strlen(argv[i]) + 1;
	}

	printf("%s cmdline %s\n", __func__, cmd);

	ret = _1905_cli_command(ctrl, cmd, strlen(cmd));

	return ret;
}

static int _1905_cli_request(struct _1905_ctrl *ctrl, int argc, char *argv[])
{
	struct _1905_cli_cmd *cmd, *match = NULL;
	int ret = 0;

	cmd = cli_cmds;

	while (cmd->cmd) {
		if (strncmp(cmd->cmd, argv[0], strlen(argv[0])) == 0) {
			match = cmd;
			break;
		}
		cmd++;
	}

	if (match) {
		ret = match->cmd_handler(ctrl, argc, &argv[0]);
	} else {
		printf("Unknown command\n");
		ret = -1;
	}

	return ret;
}

int optind = 2;
int main(int argc, char *argv[])
{
	int ret = 0;
	char socket_path[64]={0};

	ret = os_snprintf(socket_path, 64, "%s", "/tmp/1905ctrl");
	if (os_snprintf_error(64, ret))
		return -1;

	ret = _1905_cli_open_connection(socket_path);
	if (ret < 0)
		return ret;

	ret = _1905_cli_request(ctrl_conn, argc - optind, &argv[optind]);

	_1905_cli_close_connection();

	return ret;
}
