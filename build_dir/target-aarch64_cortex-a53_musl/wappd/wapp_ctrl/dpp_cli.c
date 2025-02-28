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

#include "wapp_cmm.h"
#include "wapp_ctrl.h"
#include "wapp_cli.h"

static int wapp_cli_cmd(struct wapp_ctrl *ctrl, const char *src_cmd,
                           int min_args, int argc, char *argv[])
{
	char rsp[2048];
	size_t rsp_len = 0;
	char cmd[2048];
	int ret;
	int i = 0, j = 0,k=0;

	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"%s\n", __func__);
	os_memset(cmd, 0, 2048);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	ret = snprintf(&cmd[i], 2048, "interface=%s\n", argv[0]);
	if (os_snprintf_error(2048, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

	i += 11 + os_strlen(argv[0]);
	ret = snprintf(&cmd[i], 2048-i, "cmd=dpp %s ", src_cmd);
	if (os_snprintf_error(2048-i, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"argc is %d\n", argc);
	i += os_strlen(src_cmd) + 9;
	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"i %d\n", i);
	for (j = 3; j < argc; j++) {
		if(i<2048)
		{
		     k=(2048-i);
		     if (k>(os_strlen(argv[j]+1)))
		     {
			ret = snprintf(&cmd[i], 2048-i, "%s", argv[j]);
			if (os_snprintf_error(2048-i, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

			i += os_strlen(argv[j]);
			ret = snprintf(&cmd[i], 2048-i, "%s", " ");
			if (os_snprintf_error(2048-i, ret))
				DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);

			i++;
		     }
		}
	}
	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"cmd is = %s\n", cmd);

        if (argc < min_args) {
                DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Invalid %s command - at least %d argument%s required.\n",
                       cmd, min_args, min_args > 1 ? "s are" : " is");
                return -1;
        }
        return _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
}

static int wapp_cli_cmd_dpp_qr_code(struct wapp_ctrl *ctrl, int argc,
                                       char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_QR_CODE", 1, argc, argv);
}

static int wapp_cli_cmd_dpp_bootstrap_gen(struct wapp_ctrl *ctrl, int argc,
                                             char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_BOOTSTRAP_GEN", 1, argc, argv);
}


static int wapp_cli_cmd_dpp_bootstrap_remove(struct wapp_ctrl *ctrl, int argc,
                                                char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_BOOTSTRAP_REMOVE", 1, argc, argv);
}


static int wapp_cli_cmd_dpp_bootstrap_get_uri(struct wapp_ctrl *ctrl,
                                                 int argc, char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_BOOTSTRAP_GET_URI", 1, argc, argv);
}


static int wapp_cli_cmd_dpp_bootstrap_info(struct wapp_ctrl *ctrl, int argc,
                                              char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_BOOTSTRAP_INFO", 1, argc, argv);
}

static int wapp_cli_cmd_dpp_auth_init(struct wapp_ctrl *ctrl, int argc,
                                         char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_AUTH_INIT", 1, argc, argv);
}


static int wapp_cli_cmd_dpp_listen(struct wapp_ctrl *ctrl, int argc,
                                      char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_LISTEN", 1, argc, argv);
}


static int wapp_cli_cmd_dpp_stop_listen(struct wapp_ctrl *ctrl, int argc,
                                       char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_STOP_LISTEN", 1, argc, argv);
}


static int wapp_cli_cmd_dpp_configurator_add(struct wapp_ctrl *ctrl, int argc,
                                                char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_CONFIGURATOR_ADD", 0, argc, argv);
}


static int wapp_cli_cmd_dpp_configurator_remove(struct wapp_ctrl *ctrl,
                                                   int argc, char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_CONFIGURATOR_REMOVE", 1, argc, argv);
}

static int wapp_cli_cmd_dpp_configurator_sign(struct wapp_ctrl *ctrl,
                                                   int argc, char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_CONFIGURATOR_SIGN", 1, argc, argv);
}

static int wapp_cli_cmd_dpp_configurator_get_key(struct wapp_ctrl *ctrl,
                                                    int argc, char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_CONFIGURATOR_GET_KEY", 1, argc, argv);
}


static int wapp_cli_cmd_dpp_pkex_add(struct wapp_ctrl *ctrl, int argc,
                                        char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_PKEX_ADD", 1, argc, argv);
}

static int wapp_cli_cmd_dpp_controller_start(struct wapp_ctrl *ctrl, int argc,
                                           char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_CONTROLLER_START", 1, argc, argv);
}

static int wapp_cli_cmd_dpp_controller_stop(struct wapp_ctrl *ctrl, int argc,
                                           char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_CONTROLLER_STOP", 1, argc, argv);
}

static int wapp_cli_cmd_dpp_pkex_remove(struct wapp_ctrl *ctrl, int argc,
                                           char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_PKEX_REMOVE", 1, argc, argv);
}


#ifdef MAP_R3

static int wapp_cli_cmd_dpp_map_reset_default(struct wapp_ctrl *ctrl,int argc,
                                                char *argv[])
{

        return wapp_cli_cmd(ctrl, "DPP_RESET_DPP_CONFIG_FILE",0, argc, argv);
}
static int wapp_cli_cmd_dpp_mapd_user_config_reset(struct wapp_ctrl *ctrl, int argc,
						char *argv[])
{

	return wapp_cli_cmd(ctrl, "DPP_RESET_MAPD_USER_CONFIG_FILE", 0, argc, argv);
}


static int wapp_cli_cmd_dpp_enrollee_start(struct wapp_ctrl *ctrl, int argc,
                                           char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_START", 0, argc, argv);
}

static int wapp_cli_cmd_dpp_enrollee_stop(struct wapp_ctrl *ctrl, int argc,
                                           char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_STOP", 0, argc, argv);
}

static int wapp_cli_cmd_dpp_set_bh_wifi(struct wapp_ctrl *ctrl, int argc,
                                           char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_SET_BH", 0, argc, argv);
}

static int wapp_cli_cmd_dpp_set_cce_indication(struct wapp_ctrl *ctrl, int argc,
                                           char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_CCE_INDICATION", 1, argc, argv);
}

static int wapp_cli_cmd_dpp_dev_set_cfg(struct wapp_ctrl *ctrl, int argc,
                                           char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_DEV_SET_CFG", 0, argc, argv);
}
static int wapp_cli_cmd_chirp_en_disable(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	return wapp_cli_cmd(ctrl, "DPP_CHIRP_EN_DIS", 0, argc, argv);
}
static int wapp_cli_cmd_onboard_type(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	return wapp_cli_cmd(ctrl, "DPP_ONBOARD_TYPE", 0, argc, argv);
}

#endif

#ifdef DPP_R2_SUPPORT
static int wapp_cli_cmd_dpp_presence_enable(struct wapp_ctrl *ctrl, int argc,
                                           char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_PRESENCE_ENABLE", 0, argc, argv);
}
#endif /* DPP_R2_SUPPORT */

#ifdef DPP_R2_RECONFIG
static int wapp_cli_cmd_dpp_reconfig_enable(struct wapp_ctrl *ctrl, int argc,
                                           char *argv[])
{
        return wapp_cli_cmd(ctrl, "DPP_RECONFIG_ENABLE", 0, argc, argv);
}
#endif /* DPP_R2_RECONFIG */
static int wapp_cli_cmd_chirp_notif(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	return wapp_cli_cmd(ctrl, "DPP_CHIRP_NOTIF", 0, argc, argv);
}
#ifdef MAP_R3
struct wapp_cli_cmd dpp_specific_cmd[] = {
	{ "dpp_bootstrap_gen", wapp_cli_cmd_dpp_bootstrap_gen,
	"type=<qrcode> [chan=..] [mac=..] [info=..] [curve=..] [key=..] = generate DPP bootstrap information" },
	{ "dpp_bootstrap_get_uri", wapp_cli_cmd_dpp_bootstrap_get_uri,
	"<id> = get DPP bootstrap URI" },
	{ "dpp_bootstrap_remove", wapp_cli_cmd_dpp_bootstrap_remove,
	"*|<id> = remove DPP bootstrap information" },
	{ "dpp_qr_code", wapp_cli_cmd_dpp_qr_code,
	"report a scanned DPP URI from a QR Code" },
	{ "dpp_cce_indication_start", wapp_cli_cmd_dpp_set_cce_indication,
	"set cce indication for proxy agent"},
	{ "dpp_onboard_type", wapp_cli_cmd_onboard_type,
	"To set onboard type"},
	{ "dpp_start", wapp_cli_cmd_dpp_enrollee_start,
	"start dpp for enrollee"},
	{ "dpp_chirp_stop", wapp_cli_cmd_dpp_enrollee_stop,
	"stop dpp for enrollee"},
	{ "dpp_chirp_notif", wapp_cli_cmd_chirp_notif,
	"sending chirp notification to enrollee"},
	{ "dpp_chirp_ch_en", wapp_cli_cmd_chirp_en_disable,
	  "Enable disable chirp on channel list"},
	{ "dpp_reset_dpp_config_file", wapp_cli_cmd_dpp_map_reset_default,
	"reset dpp config text file to default" },
	{"help", wapp_dpp_cmd_show_help, "show this help"},
	{NULL, wapp_dpp_cmd_show_help,	NULL}
};
#endif

struct wapp_cli_cmd dpp_cmd[] = {
        { "dpp_qr_code", wapp_cli_cmd_dpp_qr_code,
          "report a scanned DPP URI from a QR Code" },
        { "dpp_bootstrap_gen", wapp_cli_cmd_dpp_bootstrap_gen,
          "type=<qrcode> [chan=..] [mac=..] [info=..] [curve=..] [key=..] = generate DPP bootstrap information" },
        { "dpp_bootstrap_remove", wapp_cli_cmd_dpp_bootstrap_remove,
          "*|<id> = remove DPP bootstrap information" },
        { "dpp_bootstrap_get_uri", wapp_cli_cmd_dpp_bootstrap_get_uri,
          "<id> = get DPP bootstrap URI" },
        { "dpp_bootstrap_info", wapp_cli_cmd_dpp_bootstrap_info,
          "<id> = show DPP bootstrap information" },
        { "dpp_auth_init", wapp_cli_cmd_dpp_auth_init,
          "peer=<id> [own=<id>] = initiate DPP bootstrapping" },
        { "dpp_listen", wapp_cli_cmd_dpp_listen, 
          "<freq in MHz> = start DPP listen" },
        { "dpp_stop_listen", wapp_cli_cmd_dpp_stop_listen,
          "= stop DPP listen" },
        { "dpp_configurator_add", wapp_cli_cmd_dpp_configurator_add,
          "[curve=..] [key=..] = add DPP configurator" },
        { "dpp_configurator_remove", wapp_cli_cmd_dpp_configurator_remove,
          "*|<id> = remove DPP configurator" },
        { "dpp_configurator_sign", wapp_cli_cmd_dpp_configurator_sign,
          "*|<id> = self configuration" },
        { "dpp_configurator_get_key", wapp_cli_cmd_dpp_configurator_get_key,
          "<id> = Get DPP configurator's private key" },
        { "dpp_pkex_add", wapp_cli_cmd_dpp_pkex_add,
          "add PKEX code" },
        { "dpp_pkex_remove", wapp_cli_cmd_dpp_pkex_remove,
          "*|<id> = remove DPP pkex information" },
	{ "dpp_controller_start", wapp_cli_cmd_dpp_controller_start,
	  "start gas controller"},
	{ "dpp_controller_stop", wapp_cli_cmd_dpp_controller_stop,
	  "stop gas controller"},
#ifdef MAP_R3	
	{ "dpp_reset_dpp_config_file", wapp_cli_cmd_dpp_map_reset_default,
	"reset dpp config text file to default" },
	{ "dpp_reset_mapd_user_config_file", wapp_cli_cmd_dpp_mapd_user_config_reset,
	"removes backhaul information from mapd user config file" },
	 { "dpp_start", wapp_cli_cmd_dpp_enrollee_start,
	  "start dpp for enrollee"},
	 { "dpp_set_bh_wifi", wapp_cli_cmd_dpp_set_bh_wifi,
	  "set the bh type to wifi"},
	 { "dpp_cce_indication_start", wapp_cli_cmd_dpp_set_cce_indication,
	  "set cce indication for proxy agent"},
	 { "dpp_dev_set_cfg", wapp_cli_cmd_dpp_dev_set_cfg,
	  "set configuration for dpp from map wts file"},
	 { "dpp_chirp_stop", wapp_cli_cmd_dpp_enrollee_stop,
	  "stop dpp for enrollee"},
#endif
#ifdef DPP_R2_SUPPORT
	 { "dpp_presence_enable", wapp_cli_cmd_dpp_presence_enable,
	  "enable dpp presence annouce for enrollee"},
#endif /* DPP_R2_SUPPORT*/
#ifdef DPP_R2_RECONFIG
	 { "dpp_reconfig_enable", wapp_cli_cmd_dpp_reconfig_enable,
	  "enable dpp reconfig for enrollee"},
#endif /* DPP_R2_RECONFIG*/
	{ "dpp_chirp_notif", wapp_cli_cmd_chirp_notif,
	  "sending chirp notification to enrollee"},
#ifdef MAP_R3
	{ "dpp_chirp_ch_en", wapp_cli_cmd_chirp_en_disable,
	  "Enable disable chirp on channel list"},
	{ "dpp_onboard_type", wapp_cli_cmd_onboard_type,
	  "To set onboard type"},
#endif
	{"help", wapp_dpp_cmd_show_help, "show this help"},
	{NULL, wapp_dpp_cmd_show_help,	NULL}
};

int wapp_dpp_cli_request(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	struct wapp_cli_cmd *cmd, *match = NULL;
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"%s\n", __func__);
	cmd = dpp_cmd;

	while (cmd->cmd) {
		if (os_strncmp(cmd->cmd, argv[2], os_strlen(argv[2])) == 0) {
			match = cmd;
			break;
		}
		cmd++;
	}

	if (match) {
		ret = match->cmd_handler(ctrl, argc, &argv[0]);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Unknown command\n");
		ret = -1;
	}

	return ret;
}

int wapp_dpp_cmd_show_help(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	u8 i = 0;

#ifdef MAP_R3
	struct wapp_cli_cmd *wapp_cmd = dpp_specific_cmd;
#else
	struct wapp_cli_cmd *wapp_cmd = dpp_cmd;
#endif /* MAP_R3 */
	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"%s\n", __func__);
	for(i=0;(wapp_cmd[i].cmd != NULL);i++){	
		DBGPRINT_RAW(RT_DEBUG_ERROR, "  %-60s %-50s\n",wapp_cmd[i].cmd,wapp_cmd[i].usage);
	}

	return WAPP_SUCCESS;
}

