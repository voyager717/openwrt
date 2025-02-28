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
#include <unistd.h>
#include <ctype.h>
#include "wapp_cmm.h"
#ifdef DPP_SUPPORT
#include "dpp_wdev.h"
#endif /*DPP_SUPPORT*/
#ifdef OPENWRT_SUPPORT
#include <libdatconf.h>
#endif


/*
========================================================================
IAPP
========================================================================
*/

#include "rt_config.h"
#include "rtmpiapp.h"
#include "iappdefs.h"

extern int wapp_iface_init(struct wifi_app *wapp);
VOID IAPP_Usage(
	VOID)
{
	printf("\tUSAGE:\t\tralinkiappd <-e eth_if_name> <-w wireless_if_name>\n");
	printf("\t\t\t\t<-k security_key> <-d debug level>\n");
	printf("\tDefault:\tralinkiappd -e br0 -w ra0 -k 12345678 -d 3\n");
}

/*
========================================================================
HOTSPOT
========================================================================
*/

#include "hotspot.h"
extern struct hotspot_event_ops hs_event_ops;
extern void wapp_iface_deinit(struct wifi_app *wapp);

int hs_usage()
{

	DBGPRINT(RT_DEBUG_OFF, "hotspot [-f <hotspot configuration file>] [-m <hotspot mode>] [-i <hotspot ipc type>] [-d <debug level>] [-I <ineterface name>]\n");
	DBGPRINT(RT_DEBUG_OFF, "-f <hotspot configuration file>\n");
	DBGPRINT(RT_DEBUG_OFF, "-m <hotspot mode> (OPMODE_STA, OPMODE_AP)\n");
	DBGPRINT(RT_DEBUG_OFF, "-i <hotspot ipc type> (RA_WEXT, RA_NETLINK)\n");
	DBGPRINT(RT_DEBUG_OFF, "-d <hotspot debug level>\n");
	DBGPRINT(RT_DEBUG_OFF, "-h help\n");
	return 0;
}

/*
========================================================================
MBO
========================================================================
*/
#ifdef MAP_SUPPORT
int process_options(int argc, char *argv[], char *filename,
					int *opmode, int *drv_mode, int *debug_level, int *version, char *iface
					,char *map_cfg, char *map_user_cfg
					, RTMP_IAPP *pCtrlBK
					)
#else
int process_options(int argc, char *argv[], char *filename,
					int *opmode, int *drv_mode, int *debug_level, int *version, char *iface
					,RTMP_IAPP *pCtrlBK
					)
#endif
{
	int c;
	char *cvalue = NULL;
	int i = 0;

	/* IAPP init */
	strncpy(pCtrlBK->IfNameEth, FT_KDP_DEFAULT_IF_ETH, IFNAMSIZ - 1);
	strncpy(pCtrlBK->IfNameWlan, FT_KDP_DEFAULT_IF_WLAN, IFNAMSIZ - 1);
	strncpy(pCtrlBK->IfNameWlanIoctl[0], FT_KDP_DEFAULT_IF_WLAN_IOCTL, IFNAMSIZ - 1);

	strncpy(pCtrlBK->CommonKey, FT_KDP_DEFAULT_PTK, IAPP_ENCRYPT_KEY_MAX_SIZE);


	opterr = 0;

	while ((c = getopt(argc, argv, "m:f:i:d:v:e:w:k:c:F:u:")) != -1) {
		switch (c) {
		case 'd':
			cvalue = optarg;
			if (os_strcmp(cvalue, "0") == 0)
				*debug_level = RT_DEBUG_OFF;
			else if (os_strcmp(cvalue, "1") == 0)
				*debug_level = RT_DEBUG_ERROR;
			else if (os_strcmp(cvalue, "2") == 0)
				*debug_level = RT_DEBUG_WARN;
			else if (os_strcmp(cvalue, "3") == 0)
				*debug_level = RT_DEBUG_TRACE;
			else if (os_strcmp(cvalue, "4") == 0)
				*debug_level = RT_DEBUG_INFO;
			else {
				DBGPRINT(RT_DEBUG_ERROR, "-d option does not have this debug_level %s\n", cvalue);
				return - 1;
			}
			break;
		case 'f':
			cvalue = optarg;
			os_strncpy(filename, cvalue,  sizeof(filename)-1);
			break;
		case 'm':
			cvalue = optarg;
			if (os_strcmp(cvalue, "OPMODE_STA") == 0)
				*opmode = OPMODE_STA;
			else if (os_strcmp(cvalue, "OPMODE_AP") == 0)
				*opmode = OPMODE_AP;
			else {
				DBGPRINT(RT_DEBUG_ERROR, "-m option does not have this mode %s\n", cvalue);
				return -1;
			}
			break;
		case 'i':
			cvalue = optarg;
			if (os_strcmp(cvalue, "RA_WEXT") == 0)
				*drv_mode = RA_WEXT;
			else if (os_strcmp(cvalue, "RA_NETLINK") == 0)
				*drv_mode = RA_NETLINK;
			else {
				DBGPRINT(RT_DEBUG_OFF, "-i option does not have this type %s\n", cvalue);
				return -1;
			}
			break;
		case 'v':
			cvalue = optarg;
           	*version = atoi(cvalue);
 			break;
		case 'h':
			cvalue = optarg;
			hs_usage();
			IAPP_Usage();
			break;
		case 'e':
			cvalue = optarg;
			os_strncpy(pCtrlBK->IfNameEth, cvalue, sizeof(pCtrlBK->IfNameEth)-1);
			break;
		case 'w':
			cvalue = optarg;
			os_strncpy(pCtrlBK->IfNameWlan, cvalue, sizeof(pCtrlBK->IfNameWlan)-1);
			break;
		case 'k':
			cvalue = optarg;
			if (strlen(cvalue) > IAPP_ENCRYPT_KEY_MAX_SIZE)
			{
				cvalue[IAPP_ENCRYPT_KEY_MAX_SIZE] = 0x00;

				DBGPRINT(RT_DEBUG_TRACE, "iapp> key length can not be larger than %d!",
						IAPP_ENCRYPT_KEY_MAX_SIZE);
			}
			strncpy(pCtrlBK->CommonKey, cvalue, sizeof(pCtrlBK->CommonKey)-1);
			break;
		case 'c': //original IAPP -wi : wlan ioctl interface
			cvalue = optarg;
			strncpy(pCtrlBK->IfNameWlanIoctl[pCtrlBK->IfNameWlanCount++], cvalue,
			sizeof(pCtrlBK->IfNameWlanIoctl[pCtrlBK->IfNameWlanCount])-1);
			break;
		case '?':
			if (optopt == 'f') {
				DBGPRINT(RT_DEBUG_OFF, "Option -%c requires an argument\n", optopt);
			} else if (optopt == 'm') {
				DBGPRINT(RT_DEBUG_OFF, "Option -%c requires an argument\n", optopt);
			} else if (optopt == 'd') {
				DBGPRINT(RT_DEBUG_OFF, "Option -%c requires an argument\n", optopt);
			} else if (optopt == 'i') {
				DBGPRINT(RT_DEBUG_OFF, "Option -%c requires an argument\n", optopt);
			} else if (isprint(optopt)) {
				DBGPRINT(RT_DEBUG_OFF, "Unknow options -%c\n", optopt);
			} else {

			}
			return -1;
			break;
#ifdef MAP_SUPPORT
		case 'F':
			cvalue = optarg;
			if (strlen(cvalue) != 0) {
				strncpy(map_cfg, cvalue,  sizeof(map_cfg)-1);
			}
			break;
		case 'u':
			cvalue = optarg;
			if (strlen(cvalue) != 0) {
				strncpy(map_user_cfg, cvalue, sizeof(map_user_cfg)-1);
			}
			break;
#endif
		}
	}

	if (strlen(iface) == 0)
	{
		os_strncpy(iface, DEFAULT_IFNAME, IFNAMSIZ - 1);
		DBGPRINT(RT_DEBUG_OFF, "Default interface: %s\n", iface);
	} else {
		DBGPRINT(RT_DEBUG_OFF, "Interface: %s\n", iface);
	}

//label_exit:
	if (pCtrlBK->IfNameWlanCount == 0)
		pCtrlBK->IfNameWlanCount = 1;
	DBGPRINT(RT_DEBUG_TRACE, "iapp> -e=%s, -w=%s",
			pCtrlBK->IfNameEth, pCtrlBK->IfNameWlan);

	for (i = 0; i < pCtrlBK->IfNameWlanCount; i++) {
		DBGPRINT(RT_DEBUG_TRACE, ", -wi=%s",
			pCtrlBK->IfNameWlanIoctl[i]);
	}
	DBGPRINT(RT_DEBUG_TRACE, ", IfNameWlanCount = %d\n", pCtrlBK->IfNameWlanCount);
	return 0;

}

int wapp_read_wts_map_config(struct wifi_app *wapp, char *name,
		struct set_config_bss_info bss_info[], unsigned char max_bss_num,
		unsigned char *config_bss_num)
{
	FILE* f;
	int index = 1, resv1, resv2, i, j;
	unsigned int mac_int[6] = {0};
	unsigned char mac[6] = {0};
	char op_class[4] = {0};
	unsigned char ssid[33] = {0};
	int authmode = 0, encrytype = 0;
	unsigned char key[65] = {0};
	char content[4096] = {0};
	char *pos1 = NULL, *pos2 = NULL;
	int ch = 0, sch = 0;
	unsigned char hidden_ssid = 0;
	unsigned char hidden_ssid_exist = 0;
	unsigned char fh_bss_unhidden = 0;
	unsigned char op_8x = 0, op_11x = 0, op_12x = 0;
#ifdef MAP_6E_SUPPORT
	unsigned char op_13x = 0;
#endif
	char backslash = 0x5C;
	char space = 0x20;
	char SUB = 0x1A;
	int ret = 0;
	int tmp = 0;

	f = fopen(name, "r");
	if (f == NULL) {
		DBGPRINT(RT_DEBUG_TRACE, "open file %s fail\n", name);
		return -1;
	}

	for (i = 0; i < max_bss_num; i++) {
		memset(&bss_info[i], 0, sizeof(struct set_config_bss_info));
	}

	i = 0;
	do {
		ch = fgetc(f);
		if (ferror(f)) {
			DBGPRINT(RT_DEBUG_TRACE, "fgetc error");
			break;
		}
	/* If ssid is a space, need add \ as ESC infront of it in wts file. As space is regard as
	* a separator. If ssid is a \, need add \ as ESC in front of it. when parsing wts file,
	"\space"need remove \ and replace space to SUB. "\\"need remove \*/
		if (ch == backslash) {
			sch = fgetc(f);
			if (ferror(f)) {
				DBGPRINT(RT_DEBUG_ERROR, "[%s] fgetc fail\n", __func__);
				break;
			}
			if (sch == space) {
				ch = SUB;
			} else if (sch == backslash) {
				ch = sch;
			} else {
				content[i++] = ch;
				ch = sch;
			}
		}
		content[i++] = ch;
	}while (EOF != ch);

	i = 0;
	pos1 = content;
	while (1) {
		/*index*/
		pos2 = strchr(pos1, ',');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "index not found\n");
			break;
		}
		*pos2 = '\0';
		ret = sscanf(pos1, "%d", &index);
		if (ret == EOF) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] sscanf fail\n", __func__);
			continue;
		}
		pos2++;
		pos1 = pos2;

		/*mac*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "mac not found\n");
			break;
		}
		*pos2 = '\0';
		ret = sscanf(pos1, "%02x:%02x:%02x:%02x:%02x:%02x", mac_int,
				(mac_int + 1), (mac_int + 2), (mac_int + 3),
				(mac_int + 4), (mac_int + 5));
		if (ret == EOF) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] sscanf fail\n", __func__);
			continue;
		}
		for (j = 0; j < 6; j++)
			mac[j] = (unsigned char)mac_int[j];

		pos2++;
		pos1 = pos2;

		/*opclass*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "opclass not found\n");
			break;
		}
		*pos2 = '\0';

		ret = sscanf(pos1, "%s", op_class);
		if (ret == EOF) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] sscanf fail\n", __func__);
			continue;
		}
		DBGPRINT(RT_DEBUG_TRACE, "opclass %s\n", op_class);

		if (0 == memcmp(op_class, "8", 1)) {
			op_8x ++;
			bss_info[i].operating_chan = RADIO_24G;
		}
		else if (0 == memcmp(op_class, "11", 2)) {
			op_11x ++;
			bss_info[i].operating_chan = RADIO_5GL;
		}
		else if (0 == memcmp(op_class, "12", 2)) {
			op_12x ++;
			bss_info[i].operating_chan = RADIO_5GH;
		}
#ifdef MAP_6E_SUPPORT
		else if (memcmp(op_class, "13", 2) == 0) {
			op_13x++;
			bss_info[i].operating_chan = RADIO_6G;
		}
#endif

		pos2++;
		pos1 = pos2;

		/*ssid*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "ssid not found\n");
			break;
		}
		*pos2 = '\0';
		ret = sscanf(pos1, "%s", ssid);
		if (ret == EOF) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] sscanf fail\n", __func__);
			continue;
		}
		for (j = 0; j < 32; j++) {
			if (ssid[j] == SUB)
				ssid[j] = space;
		}
		pos2++;
		pos1 = pos2;

		/*authmode*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "authmode not found\n");
			break;
		}
		*pos2 = '\0';
		ret = sscanf(pos1, "0x%04x", &authmode);
		if (ret == EOF) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] sscanf fail\n", __func__);
			continue;
		}
		pos2++;
		pos1 = pos2;

		/*encrytype*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "encrytype not found\n");
			break;
		}
		*pos2 = '\0';
		ret = sscanf(pos1, "0x%04x", &encrytype);
		if (ret == EOF) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] sscanf fail\n", __func__);
			continue;
		}
		pos2++;
		pos1 = pos2;

		/*key*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "key not found\n");
			break;
		}
		*pos2 = '\0';
		ret = sscanf(pos1, "%s", key);
		if (ret == EOF) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] sscanf fail\n", __func__);
			continue;
		}
		pos2++;
		pos1 = pos2;

		/*resv1*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "resv1 not found\n");
			break;
		}
		*pos2 = '\0';
		ret = sscanf(pos1, "%d", &resv1);
		if (ret == EOF) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] sscanf fail\n", __func__);
			continue;
		}
		pos2++;
		pos1 = pos2;
		/*resv2*/
		pos2 = strstr(pos1, "hidden-");
		if (pos2 != NULL) {
			DBGPRINT(RT_DEBUG_TRACE, "hidden SSID exist\n");
			hidden_ssid_exist = 1;
		} else {
			DBGPRINT(RT_DEBUG_TRACE, "no hidden SSID\n");
			hidden_ssid_exist = 0;
			hidden_ssid = 'N';
		}

		if (!hidden_ssid_exist) {
			/*resv2*/
			pos2 = strchr(pos1, '\n');
			if (pos2 == NULL) {
				DBGPRINT(RT_DEBUG_TRACE, "resv2 not found\n");
				break;
			}
			*pos2 = '\0';
			ret = sscanf(pos1, "%d", &resv2);
			if (ret == EOF) {
				DBGPRINT(RT_DEBUG_ERROR, "[%s] sscanf fail\n", __func__);
				continue;
			}
			pos2++;
			pos1 = pos2;
		} else {
			/*resv2*/
			pos2 = strchr(pos1, ' ');
			if (pos2 == NULL) {
				DBGPRINT(RT_DEBUG_TRACE, "resv2 not found\n");
				break;
			}
			*pos2 = '\0';
			ret = sscanf(pos1, "%d", &resv2);
			if (ret == EOF) {
				DBGPRINT(RT_DEBUG_ERROR, "[%s] sscanf fail\n", __func__);
				continue;
			}
			pos2++;
			pos1 = pos2;

			/*hidden ssid*/
			pos2 = strchr(pos1, '\n');
			if (pos2 == NULL) {
				DBGPRINT(RT_DEBUG_TRACE, "hidden ssid not found\n");
				break;
			}
			*pos2 = '\0';
			if (sscanf(pos1, "hidden-%c", &hidden_ssid) == 1) {
				pos2++;
				pos1 = pos2;
			} else {
				DBGPRINT(RT_DEBUG_TRACE, "wrong format hidden ssid\n");
				break;
			}
		}
		bss_info[i].authmode = authmode;
		bss_info[i].encryptype = encrytype;
		memcpy(bss_info[i].mac, mac, 6);
		memcpy(bss_info[i].key, key, sizeof(key));
		memcpy(bss_info[i].oper_class, op_class, sizeof(op_class));
		memcpy(bss_info[i].ssid, ssid, sizeof(ssid));
		bss_info[i].wfa_vendor_extension |= (resv1 & 0x01) << 6;
		bss_info[i].wfa_vendor_extension |= (resv2 & 0x01) << 5;
		if (hidden_ssid == 'Y')
			bss_info[i].hidden_ssid = 1;
		else if (hidden_ssid == 'N')
			bss_info[i].hidden_ssid = 0;
		else
			bss_info[i].hidden_ssid = 0;
		DBGPRINT(RT_DEBUG_OFF, "set bss index=%d, mac="MACSTR
				", opclass=%s, ssid=%s, authmode=%04x, encrytype=%04x, key=%s, "
				"bh_bss=%s, fh_bss=%s hidden_ssid=%d\n",
				index, MAC2STR(mac), op_class, ssid, authmode, encrytype, key,
				bss_info[i].wfa_vendor_extension & BIT_BH_BSS ? "1" : "0",
				bss_info[i].wfa_vendor_extension & BIT_FH_BSS ? "1" : "0", bss_info[i].hidden_ssid);
		i++;
		if (i >= max_bss_num) {
			DBGPRINT(RT_DEBUG_TRACE, "too much bss wireless setting info\n");
			i--;
			break;
		}
	}

	*config_bss_num = i;
	DBGPRINT(RT_DEBUG_OFF, "config_bss_num=%d\n", *config_bss_num);
	tmp = fclose(f);
	if (tmp != 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] fclose fail\n", __func__);
		return -1;
	}


	if (0 == op_8x) {
		DBGPRINT(RT_DEBUG_TRACE, RED("!!!2G Band BSS Configuration is missing!!!\n"));
	}
	if (0 == op_11x) {
		DBGPRINT(RT_DEBUG_TRACE, RED("!!!5G Low Band BSS Configuration is missing!!!\n"));
	}
	if (0 == op_12x) {
		DBGPRINT(RT_DEBUG_TRACE, RED("!!!5G High Band BSS Configuration is missing!!!\n"));
	}
#ifdef MAP_6E_SUPPORT
	if (op_13x == 0)
		DBGPRINT(RT_DEBUG_TRACE, RED("!!!6G High Band BSS Configuration is missing!!!\n"));
#endif

	fh_bss_unhidden = 0;
	for (i = 0; i < *config_bss_num; i++) {
		if ((bss_info[i].oper_class[0] == '8') &&
				(bss_info[i].wfa_vendor_extension & BIT_FH_BSS) &&
				(bss_info[i].hidden_ssid == 0)) {
			fh_bss_unhidden = 1;
			break;
		}
	}
	if (fh_bss_unhidden == 0) {
		DBGPRINT(RT_DEBUG_TRACE, "all fronthaul bss of radio 24G is hidden! wps may fail!\n");
	}

	fh_bss_unhidden = 0;
	for (i = 0; i < *config_bss_num; i++) {
		if ((bss_info[i].oper_class[1] == '1') &&
				(bss_info[i].wfa_vendor_extension & BIT_FH_BSS) &&
				(bss_info[i].hidden_ssid == 0)) {
			fh_bss_unhidden = 1;
			break;
		}
	}
	if (fh_bss_unhidden == 0) {
		DBGPRINT(RT_DEBUG_TRACE, "all fronthaul bss of radio 5GL is hidden! wps may fail!\n");
	}

	fh_bss_unhidden = 0;
	for (i = 0; i < *config_bss_num; i++) {
		if ((bss_info[i].oper_class[1] == '2') &&
				(bss_info[i].wfa_vendor_extension & BIT_FH_BSS) &&
				(bss_info[i].hidden_ssid == 0)) {
			fh_bss_unhidden = 1;
			break;
		}
	}
	if (fh_bss_unhidden == 0) {
		DBGPRINT(RT_DEBUG_TRACE, "all fronthaul bss of radio 5GH is hidden! wps may fail!\n");
	}
	return 0;
}
#ifdef DPP_SUPPORT
void wapp_dpp_config_init(struct wifi_app *wapp)
{
#ifdef DPP_R2_SUPPORT
	int dpp_ret = 0;
#endif /* DPP_R2_SUPPORT */

#ifndef MAP_R3
	if (wapp_dpp_init(wapp) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed to init dpp\n");
	}
#endif /* MAP_R3 */
	if (wapp_dpp_gas_server_init(wapp) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed to init hostapd dpp\n");
	}
	DBGPRINT(RT_DEBUG_OFF, "initialized dpp\n");
	dpp_read_config_file(wapp->dpp);
	if (wapp->dpp->dpp_configurator_supported
#ifdef MAP_R3
	 && !is_str_null(wapp->dpp->dpp_private_key)
#endif /* MAP_R3 */
	)
		wapp_dpp_configurator_add(wapp->dpp);
#ifndef MAP_R3
	else
		wapp->dpp->dpp_allowed_roles = DPP_CAPAB_ENROLLEE;
#endif /* MAP_R3 */

	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"key %s\n", wapp->dpp->dpp_private_key);
#ifdef MAP_R3
	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"mac_addr %s\n", wapp->dpp->dpp_macaddr_key);
#endif /* MAP_R3 */

#ifdef DPP_R2_SUPPORT

#ifdef MAP_R3
	dpp_ret = dpp_bootstrap_gen_at_bootup(wapp->dpp, (char *)wapp->dpp->dpp_private_key,
			(char *)wapp->dpp->dpp_macaddr_key, (char *)wapp->dpp->dpp_chan_list);
#else
	dpp_ret = dpp_bootstrap_gen_at_bootup(wapp->dpp, (char *)wapp->dpp->dpp_private_key, NULL, NULL);
#endif /* MAP_R3 */
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"dpp: dpp_configurator_supported :%d, gen_ret:%d\n", wapp->dpp->dpp_configurator_supported, dpp_ret);
#ifndef MAP_R3
	if(dpp_ret  != -1)
		wapp_dpp_presence_annouce(wapp, NULL);
#endif /* MAP_R3 */
#else
	dpp_bootstrap_gen_at_bootup(wapp->dpp, (char *)wapp->dpp->dpp_private_key, NULL, NULL);
#endif /* DPP_R2_SUPPORT */

	dl_list_init(&wapp->scan_results_list);
#ifdef MAP_R3
	if (wapp->dpp->is_map) {
		if (wapp->dpp->dpp_configurator_supported) {
			wapp_read_wts_map_config(wapp, MAPD_WTS_FILE,
					wapp->dpp->bss_config, MAX_SET_BSS_INFO_NUM, &wapp->dpp->bss_config_num);

			DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"Setting configurator\n");
			dpp_controller_start(wapp->dpp);
		}
	}
	dl_list_init(&wapp->map_devices);
#endif /* MAP_R3 */
#ifndef MAP_R3
#ifdef DPP_R2_SUPPORT
		/*set vendor beacon ie for DPP CCE*/
		if(wapp->dpp->dpp_configurator_supported)
			wapp_dpp_set_ie(wapp);
#endif /* DPP_R2_SUPPORT */
#endif /* MAP_R3 */
}
int get_dpp_parameters(struct map_info *map, char* param, char* value, size_t val_len)
{
#ifdef OPENWRT_SUPPORT
	const char *tmp_value = NULL;
	unsigned int len = 0;
	struct kvc_context *dat_ctx = NULL;
	const char *file = NULL;
	int ret = 0;

	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"%s param(%s)\n",__func__, param);

	os_memset(value, 0, val_len);
	file = DPP_CFG_FILE;

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"invalid file!!!\n");
		return -1;
	}

	dat_ctx = dat_load(file);
	if (!dat_ctx) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"load file(%s) fail\n", file);
		ret = -1;
		goto out;
	}

	tmp_value = kvc_get(dat_ctx, (const char *)param);
	if (!tmp_value) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"get param(%s) fail\n", param);
		ret = -1;
		goto out;
	}
	len = os_min(os_strlen(tmp_value), val_len - 1);
	os_memcpy(value, tmp_value, len);

	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"%s value(%s)\n",__func__, value);
out:
	//if (file)
	//	free_dat_path(file);
	if (dat_ctx)
		kvc_unload(dat_ctx);

	return ret;
#else
	char cmd_buffer[256] = {0};
	char tmp_buffer[350] = {0};
	char temp_file[] = "/tmp/system_command_output";
	FILE *fp;

	os_memset(value, 0, val_len);
	os_snprintf(cmd_buffer, sizeof(cmd_buffer),"nvram_get 2860 %s > %s &", param, temp_file);
	if ((fp = popen(temp_file, "r")) == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Error opening pipe!\n");
		return -1;
	}
	fscanf(fp, "%349s", tmp_buffer);

	os_memcpy(value, tmp_buffer, val_len);
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"get_dpp_parameter %s = %s\n", param, value);

	if(pclose(fp)) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Command not found or exited with error status\n");
		return -1;
	}

	return 0;
#endif
}
int dpp_read_config_file_for_connection(struct wifi_app *wapp)
{
	char param[64], value[1024] = {0};
	unsigned char buf_temp[1024] = {0};
	int k = 0;
	struct wapp_dev *wdev = NULL;
	struct dpp_config config[3];
	int ret = 0;
#ifdef MAP_R3
	struct dpp_sec_cred *cred = NULL;
	struct dpp_authentication auth;
	//struct dpp_bss_cred *bss_cred = NULL;
	u16 cred_len = 0, is_1905_conn = 0, is_normal = 0;
	struct dl_list *dev_list;
	dev_list = &wapp->dev_list;
	//char all_zero_mac[MAC_ADDR_LEN]={0};
#endif

	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"%s\n", __func__);

	if (!wapp) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"\033[1;31m %s, %u \033[0m\n", __FUNCTION__, __LINE__);  /* Haipin Debug Print (R)*/
		return WAPP_INVALID_ARG;
	}

	os_memset(config, 0, 3 * sizeof(struct dpp_config));
	ret = os_snprintf(param, sizeof(param), "_valid");
	if (os_snprintf_error(sizeof(param), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
		return -1;
	}
	get_dpp_parameters(wapp->map, param, value, sizeof(value));
	if (os_strcmp(value, "1"))
		is_normal = 0;
	else
		is_normal = 1;
	NdisZeroMemory(value, 1024);
	NdisZeroMemory(param, 64);
	ret = os_snprintf(param, sizeof(param), "_1905valid");
	if (os_snprintf_error(sizeof(param), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
		return -1;
	}
	get_dpp_parameters(wapp->map, param, value, sizeof(value));
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"%s _1905valid%s\n", __func__, value);
	if (os_strcmp(value, "1"))
		return -1;
	is_1905_conn = 1;
	if (is_normal) {
		NdisZeroMemory(value, 1024);
		NdisZeroMemory(param, 64);
		ret = os_snprintf(param, sizeof(param), "_ssid");
		if (os_snprintf_error(sizeof(param), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			return -1;
		}
		get_dpp_parameters(wapp->map, param, value, sizeof(value));
		config[k].ssid_len = os_strlen(value);
		config[k].ssid = os_zalloc(config[k].ssid_len + 1);
		if (!config[k].ssid)
			return -1;
		os_memcpy(config[k].ssid, value, os_strlen(value));
		config[k].ssid[os_strlen(value)] = '\0';
		NdisZeroMemory(value, 1024);
		NdisZeroMemory(param, 64);
		ret = os_snprintf(param, sizeof(param), "_akm");
		if (os_snprintf_error(sizeof(param), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
		get_dpp_parameters(wapp->map, param, value, sizeof(value));
		config[k].akm = atoi(value);
		NdisZeroMemory(value, 1024);
		NdisZeroMemory(param, 64);
		ret = os_snprintf(param, sizeof(param), "_passPhrase");
		if (os_snprintf_error(sizeof(param), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
		get_dpp_parameters(wapp->map, param, value, sizeof(value));
		os_memcpy(config[k].psk, value, 32);
		NdisZeroMemory(value, 1024);
		NdisZeroMemory(param, 64);
		ret = os_snprintf(param, sizeof(param), "_cSignKey");
		if (os_snprintf_error(sizeof(param), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
		get_dpp_parameters(wapp->map, param, value, sizeof(value));
		config[k].dpp_csign_len = os_strlen(value)/2;
		config[k].dpp_csign = os_zalloc(config[k].dpp_csign_len + 1);
		if (!config[k].dpp_csign) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_zalloc fail\n", __func__);
			goto error;
		}
		hexstr2bin(value, config[k].dpp_csign, config[k].dpp_csign_len);
		NdisZeroMemory(value, 1024);
		NdisZeroMemory(param, 64);
		ret = os_snprintf(param, sizeof(param), "_netAccessKey");
		if (os_snprintf_error(sizeof(param), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
		get_dpp_parameters(wapp->map, param, value, sizeof(value));
		config[k].dpp_netaccesskey_len = os_strlen(value)/2;
		config[k].dpp_netaccesskey = os_zalloc(config[k].dpp_netaccesskey_len + 1);
		if (!config[k].dpp_netaccesskey) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_zalloc fail\n", __func__);
			goto error;
		}
		hexstr2bin(value, config[k].dpp_netaccesskey, config[k].dpp_netaccesskey_len);
		NdisZeroMemory(value, 1024);
		NdisZeroMemory(param, 64);
		ret = os_snprintf(param, sizeof(param), "_connector");
		if (os_snprintf_error(sizeof(param), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
		get_dpp_parameters(wapp->map, param, value, sizeof(value));
		config[k].dpp_connector = os_zalloc(os_strlen(value) + 1);
		if (!config[k].dpp_connector) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_zalloc fail\n", __func__);
			goto error;
		}
		os_memcpy(config[k].dpp_connector, value, os_strlen(value));
		config[k].dpp_connector[os_strlen(value)] = '\0';
	}
	if (is_1905_conn) {
		NdisZeroMemory(value, 1024);
		NdisZeroMemory(param, 64);
		os_memset(&auth, '\0', sizeof(struct dpp_authentication));
		ret = os_snprintf(param, sizeof(param), "_netKeyExpiry");
		if (os_snprintf_error(sizeof(param), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
		get_dpp_parameters(wapp->map, param, value, sizeof(value));
		auth.net_access_key_expiry = atol(value);
		NdisZeroMemory(value, 1024);
		NdisZeroMemory(param, 64);
		ret = os_snprintf(param, sizeof(param), "_decryptThreshold");
		if (os_snprintf_error(sizeof(param), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
		get_dpp_parameters(wapp->map, param, value, sizeof(value));
		auth.decrypt_thresold = atoi(value);
		NdisZeroMemory(value, 1024);
		NdisZeroMemory(param, 64);
		ret = os_snprintf(param, sizeof(param), "_1905connector");
		if (os_snprintf_error(sizeof(param), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
		get_dpp_parameters(wapp->map, param, value, sizeof(value));
		auth.connector = os_zalloc(os_strlen(value)+1);
		if (!auth.connector) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
		os_memcpy(auth.connector, value, os_strlen(value));
		NdisZeroMemory(buf_temp, 1024);
		NdisZeroMemory(value, 1024);
		NdisZeroMemory(param, 64);
		ret = os_snprintf(param, sizeof(param), "_1905netAccessKey");
		if (os_snprintf_error(sizeof(param), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
		get_dpp_parameters(wapp->map, param, value, sizeof(value));
		hexstr2bin(value, buf_temp, os_strlen(value));
		auth.net_access_key = wpabuf_alloc(os_strlen(value));
		if (!auth.net_access_key) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
		NdisCopyMemory(auth.net_access_key->buf, buf_temp, os_strlen(value));
		auth.net_access_key->used = os_strlen(value);
		NdisZeroMemory(buf_temp, 1024);
		NdisZeroMemory(value, 1024);
		NdisZeroMemory(param, 64);
		ret = os_snprintf(param, sizeof(param), "_1905cSignKey");
		if (os_snprintf_error(sizeof(param), ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
		get_dpp_parameters(wapp->map, param, value, sizeof(value));
		NdisZeroMemory(buf_temp, 1024);
		NdisZeroMemory(value, 1024);
		NdisZeroMemory(param, 64);
		hexstr2bin(value, buf_temp, os_strlen(value));
		auth.c_sign_key = wpabuf_alloc(os_strlen(value));
		if (!auth.c_sign_key) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] wpabuf_alloc fail\n", __func__);
			goto error;
		}
		NdisCopyMemory(auth.c_sign_key->buf, buf_temp, os_strlen(value));
		auth.c_sign_key->used = os_strlen(value);
		cred = wapp_dpp_save_dpp_cred(&auth);
		if (!cred) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] wapp_dpp_save_dpp_cred fail\n", __func__);
			goto error;
		}
		cred_len = sizeof(*cred) + cred->payload_len;
		wpabuf_clear_free(auth.net_access_key);
		wpabuf_clear_free(auth.c_sign_key);
		os_free(auth.connector);
	}
#if 1
#ifdef MAP_R3_RECONFIG

	NdisZeroMemory(value, 1024);
	NdisZeroMemory(param, 64);
	ret = os_snprintf(param, sizeof(param), "_ppkey");
	if (os_snprintf_error(sizeof(param), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
		goto error;
	}
	get_dpp_parameters(wapp->map, param, value, sizeof(value));
	config[k].dpp_ppkey_len = os_strlen(value)/2;
	config[k].dpp_ppkey = os_zalloc(config[k].dpp_ppkey_len + 1);
	if (!config[k].dpp_ppkey)
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"ppKey not found in dpp_cfg.txt\n");
	hexstr2bin(value, config[k].dpp_ppkey, config[k].dpp_ppkey_len);

		printf("\n ppKey on rread frol dpp_cfg.txt - %s",config[k].dpp_ppkey);
#endif
#endif
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
			if (is_normal == 0 || is_1905_conn == 0)
				continue;
			if (wdev->config) {
				if (wdev->config->ssid) {
					os_free(wdev->config->ssid);
					wdev->config->ssid = NULL;
				}
				if (wdev->config->dpp_connector) {
					os_free(wdev->config->dpp_connector);
					wdev->config->dpp_connector = NULL;
				}
				if (wdev->config->dpp_csign) {
					os_free(wdev->config->dpp_csign);
					wdev->config->dpp_csign = NULL;
				}
				if (wdev->config->dpp_netaccesskey) {
					os_free(wdev->config->dpp_netaccesskey);
					wdev->config->dpp_netaccesskey = NULL;
				}
#if 1
#ifdef MAP_R3_RECONFIG
				if (wdev->config->dpp_ppkey) {
					os_free(wdev->config->dpp_ppkey);
					wdev->config->dpp_ppkey = NULL;
				}
#endif
#endif
			}
			if (!wdev->config)
				wdev->config = os_zalloc(sizeof(struct dpp_config));
			if (wdev->config) {
				os_memcpy(wdev->config, (char *)(&config[k]), sizeof(struct dpp_config));
				if (config[k].dpp_csign_len > 0) {
					wdev->config->dpp_csign = os_zalloc(config[k].dpp_csign_len);
					os_memcpy(wdev->config->dpp_csign, config[k].dpp_csign,config[k].dpp_csign_len);
#ifdef MAP_R3_RECONFIG
					if (wapp->dpp_csign) {
						os_free(wapp->dpp_csign);
						wapp->dpp_csign = NULL;
					}
					wapp->dpp_csign = os_zalloc(config[k].dpp_csign_len);
					if (!wapp->dpp_csign) {
						wpa_printf(MSG_ERROR, DPP_MAP_PREX
								"wapp->dpp_csign allocation failed");
						goto error;
					}
					os_memcpy(wapp->dpp_csign, config[k].dpp_csign,
							config[k].dpp_csign_len);
					wapp->dpp_csign_len = config[k].dpp_csign_len;

					wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "in process config cign key",
							wapp->dpp_csign, wapp->dpp_csign_len);
#endif /* MAP_R3_RECONFIG */
				}
				if (config[k].dpp_netaccesskey_len > 0) {
					wdev->config->dpp_netaccesskey = os_zalloc(config[k].dpp_netaccesskey_len);
					os_memcpy(wdev->config->dpp_netaccesskey, config[k].dpp_netaccesskey,config[k].dpp_netaccesskey_len);
				}
				if (config[k].ssid_len > 0) {
					wdev->config->ssid = os_zalloc(config[k].ssid_len + 1);
					os_memcpy(wdev->config->ssid, config[k].ssid,config[k].ssid_len);
					wdev->config->ssid[config[k].ssid_len] = '\0';
				}
				if (os_strlen(config[k].dpp_connector) > 0) {

					wdev->config->dpp_connector = os_zalloc(os_strlen(config[k].dpp_connector) + 1);
					os_memcpy(wdev->config->dpp_connector, config[k].dpp_connector,os_strlen(config[k].dpp_connector));
					wdev->config->dpp_connector[os_strlen(config[k].dpp_connector)] = '\0';

				}
#if 1
#ifdef MAP_R3_RECONFIG
				if (config[k].dpp_ppkey_len > 0) {
					wdev->config->dpp_ppkey = os_zalloc(config[k].dpp_ppkey_len);
					os_memcpy(wdev->config->dpp_ppkey, config[k].dpp_ppkey,config[k].dpp_ppkey_len);
				}
#endif
#endif
				wdev->config->key_mgmt = WPA_KEY_MGMT_DPP;
			}
		}
	}
	if (config[k].dpp_csign)
		os_free(config[k].dpp_csign);
	if (config[k].dpp_netaccesskey)
		os_free(config[k].dpp_netaccesskey);
	if (config[k].ssid)
		os_free(config[k].ssid);
	if (config[k].dpp_connector)
		os_free(config[k].dpp_connector);
#if 1
#ifdef MAP_R3_RECONFIG
	if (config[k].dpp_ppkey)
		os_free(config[k].dpp_ppkey);
#endif
#endif

#if 0
	cred_len = 0;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
		NdisZeroMemory(value, 1024);
		NdisZeroMemory(param, 64);
		os_snprintf(param, sizeof(param), "_bsscredlen");
		get_dpp_parameters(wapp->map, param, value, sizeof(value));
		cred_len = atoi(value);
		if (cred_len) {
			bss_cred = os_zalloc(cred_len);
			if (!bss_cred)
				goto fail;

			NdisZeroMemory(value, 1024);
			NdisZeroMemory(param, 64);
			os_memcpy(bss_cred->enrollee_mac, all_zero_mac, MAC_ADDR_LEN);
			os_memcpy(bss_cred->almac, all_zero_mac, MAC_ADDR_LEN);
			os_snprintf(param, sizeof(param), "_bhcredlen");
			get_dpp_parameters(wapp->map, param, value, sizeof(value));
			bss_cred->bh_connect_len = atoi(value);
			NdisZeroMemory(value, 1024);
			NdisZeroMemory(param, 64);
			os_snprintf(param, sizeof(param), "_fhcredlen");
			get_dpp_parameters(wapp->map, param, value, sizeof(value));
			bss_cred->fh_connect_len = atoi(value);
			bss_cred->payload_len = bss_cred->bh_connect_len + bss_cred->fh_connect_len;
			NdisZeroMemory(param, 64);
			os_snprintf(param, sizeof(param), "_bsscred");
			get_dpp_parameters(wapp->map, param, (char *)bss_cred->payload, bss_cred->payload_len+1);

			hex_dump(DPP_MAP_PREX"Reboot BSS_connectors= ",(UCHAR *)bss_cred, cred_len);
			if (wapp_send_1905_msg(wapp, WAPP_SEND_BSS_CONNECTOR,
				cred_len, (char *)bss_cred) < 0)
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"sending failed\n");

			os_free(bss_cred);
		} else
			continue;

		break;
	}
#endif

	/* Get QR code save conunt for */
	get_dpp_parameters(wapp->map, "qr_count", value, sizeof(value));
	if (wapp->dpp)
		wapp->dpp->qr_code_num = atoi(value);

//fail:
	if (cred)
		os_free(cred);

	return WAPP_SUCCESS;

error:
	if (auth.c_sign_key)
		wpabuf_clear_free(auth.c_sign_key);
	if (auth.net_access_key)
		wpabuf_clear_free(auth.net_access_key);
	if (auth.connector)
		os_free(auth.connector);
	if (config[k].dpp_connector)
		os_free(config[k].dpp_connector);
	if (config[k].dpp_netaccesskey)
		os_free(config[k].dpp_netaccesskey);
	if (config[k].dpp_csign)
		os_free(config[k].dpp_csign);
	if (config[k].ssid)
		os_free(config[k].ssid);
	if (cred)
		os_free(cred);
	return -1;

}


#endif /*DPP_SUPPORT*/

int main(int argc, char *argv[])
{
	int ret = 0;
	int opmode;
	int drv_mode;
	int debug_level = 0;
	int version = 2;
	char filename[256] = {0};
	struct wifi_app wapp_cfg;
	struct hotspot hs;
	struct mbo_cfg mbo;
	struct oce_cfg oce;
#ifdef MAP_SUPPORT
	struct map_info map;
	char value[10] = {0};
	char map_cfg[128]= {0};
	char map_user_cfg[128]= {0};
#endif
	struct _RTMP_IAPP IAPP_Ctrl_Block;
	struct wifi_app *wapp = &wapp_cfg;
	pid_t child_pid_hs,child_pid_iapp;

	/* default setting */
	opmode = OPMODE_AP;
	drv_mode = RA_WEXT;

	memset(wapp,0,sizeof(struct wifi_app));
	memset(&hs,0,sizeof(struct hotspot));
	memset(&mbo,0,sizeof(struct mbo_cfg));
	memset(&oce,0,sizeof(struct oce_cfg));
#ifdef MAP_SUPPORT
    memset(&map, 0, sizeof(struct map_info));
#endif
	memset(&IAPP_Ctrl_Block,0,sizeof(struct _RTMP_IAPP));

#ifdef MAP_SUPPORT
	ret = process_options(argc, argv, filename, &opmode, &drv_mode, &debug_level, &version, &wapp_cfg.iface[0], map_cfg, map_user_cfg, &IAPP_Ctrl_Block);
#else
	ret = process_options(argc, argv, filename, &opmode, &drv_mode, &debug_level, &version, &wapp_cfg.iface[0],&IAPP_Ctrl_Block);

#endif

	if(ret){
		hs_usage();
		return -1;
	}

	RTDebugLevel = debug_level;

	ret = wapp_cmm_init(wapp,drv_mode,opmode,version,&hs,&mbo, &oce, &map, &IAPP_Ctrl_Block);
	if (ret)
		goto error;

#ifdef MAP_SUPPORT
	if(strlen(map_cfg) == 0)
	{
		ret = os_snprintf(wapp->map->map_cfg, 128, "/etc/map/mapd_cfg");
		if (os_snprintf_error(128, ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
	}
	else
	{
		ret = os_snprintf(wapp->map->map_cfg, 128, "%s", map_cfg);
		if (os_snprintf_error(128, ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
	}

	if(strlen(map_user_cfg) == 0)
	{
		ret = os_snprintf(wapp->map->map_user_cfg, 128, "/etc/map/mapd_user.cfg");
		if (os_snprintf_error(128, ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
	}
	else
	{
		ret = os_snprintf(wapp->map->map_user_cfg, 128, "%s", map_user_cfg);
		if (os_snprintf_error(128, ret)) {
			DBGPRINT(RT_DEBUG_ERROR, "[%s] os_snprintf fail\n", __func__);
			goto error;
		}
	}
#endif

	DBGPRINT(RT_DEBUG_OFF, "Map cfg file: %s Map user cfg: %s\n", wapp->map->map_cfg, wapp->map->map_user_cfg);
	DBGPRINT(RT_DEBUG_OFF, "WAPP version: %s\nwapp cmm type version: %s\n", WAPP_VERSION, VERSION_WAPP_CMM);

#ifdef MAP_SUPPORT
		dl_list_init(&wapp->air_monitor_query_list);
		dl_list_init(&wapp->sta_mntr_list);
#endif
	setsid();
	child_pid_hs = fork();
	if (child_pid_hs == 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Initialize socket\n");
		ret = wapp_socket_and_ctrl_inf_init(wapp,drv_mode,opmode);
		ret += wapp_get_wireless_interfaces(wapp);
		/* Initialize control interface */
		wapp->w_ctrl_iface = wapp_ctrl_iface_init(wapp);
#ifdef MAP_SUPPORT
		wapp_iface_init(wapp);
#ifdef AUTOROLE_NEGO
		wapp_MapDevRoleNegotiation_init(wapp);
#endif // AUTOROLE_NEGO
#endif
#ifdef MAP_SUPPORT
	get_map_parameters(wapp->map, "Enable_WPS_toggle_5GL_5GH", value, NON_DRIVER_PARAM, sizeof(value));
	if (!strcmp(value,"1"))
		wapp->map->enable_wps_toggle_5GL_5GH = 1;
	else
		wapp->map->enable_wps_toggle_5GL_5GH = 0;
	get_map_parameters(wapp->map, "MAP_QuickChChange", value, NON_DRIVER_PARAM, sizeof(value));
	if (!strcmp(value,"1"))
		wapp->map->quick_ch_change = 1;
	else
		wapp->map->quick_ch_change = 0;
#ifdef MAP_R2
	get_map_parameters(wapp->map, "MetricRepIntv", value, NON_DRIVER_PARAM, sizeof(value));
	wapp->map->metric_rep_intv = atoi(value);
#endif
	get_map_parameters(wapp->map, "MaxStaAllowed", value, NON_DRIVER_PARAM, sizeof(value));
	wapp->map->max_client_cnt = atoi(value);
	if (wapp->map->max_client_cnt <= 0) {
		wapp->map->max_client_cnt = MAX_NUM_OF_CLIENT;
	}

		wapp->map->send_buffer = WAPP_EVT_SIZE;

		wapp->map_wapp_buffer_size = WAPP_EVT_SIZE;
		wapp->map_wapp_buffer = os_malloc(wapp->map_wapp_buffer_size);
		if(!wapp->map_wapp_buffer) {
			DBGPRINT(RT_DEBUG_ERROR, "Failed to allocate memory for map_wapp_buffer");
			goto error;
		}

		/*if MAP enable, create ARP socket for br & wireless interface*/
		get_map_parameters(wapp->map, "MapMode", value, DRIVER_PARAM, sizeof(value));
                if (!strcmp(value,"1"))
			ret += wapp_create_arp_socket(wapp);
#endif
		if (ret)
			goto error;

		if (os_strlen(filename) == 0) {
				DBGPRINT(RT_DEBUG_TRACE, "Use default configuration file /etc/wapp_ap.conf");
				snprintf(filename, 256, WAPP_CONF_PATH"/wapp_ap.conf");
		}

		ret = wapp_init_all_config(wapp, filename);

		if (ret) {
			DBGPRINT(RT_DEBUG_OFF, "Initial hotspot configuration file(%s) fail\n", filename);
			goto error0;
		}

		ret = hotspot_set_ap_ifaces_all_ies(wapp);

		if (ret)
			goto error1;

		/* Enable hotspot feature for all interfaces */
		ret = hotspot_onoff_all(wapp, 1);

		if (ret)
			DBGPRINT(RT_DEBUG_OFF, "Initial hotspot function retrun type error");
//			goto error2;
#ifdef DPP_SUPPORT
#ifdef MAP_R3
		if (wapp_dpp_init(wapp) < 0) {
			/* if dpp_init fails means wapp->dpp is not */
			/* ok so need to exit from this */
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failed to init dpp\n");
			goto error1;
		}
		get_map_parameters(wapp->map, "DeviceRole", value, NON_DRIVER_PARAM, sizeof(value));
		if (strcmp(value,"0") == 0 && wapp->map->TurnKeyEnable) {
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"DPP init afterwards");
		} else {
#endif
		wapp_dpp_config_init(wapp);
#ifdef MAP_R3
		get_map_parameters(wapp->map, "DppOnboardType", value, NON_DRIVER_PARAM, sizeof(value));
		if (!strcmp(value, "1"))
			wapp->dpp->onboarding_type = atoi(value);
		else
			wapp->dpp->onboarding_type = 0;
		}
#endif
#endif /*DPP_SUPPORT*/
#ifdef QOS_R1
		wapp_drv_qos_init(wapp);
#endif
		hotspot_run(wapp);

		/* Disable hotspot feature for all interfaces */
		hotspot_onoff_all(wapp, 0);
#ifdef MAP_320BW
	get_map_parameters(wapp->map, "HE_EXTCHA", value, NON_DRIVER_PARAM, sizeof(value));
	wapp->HE_EXTCHA = atoi(value);
#endif



	}
	else{
		child_pid_iapp = fork();
		if (child_pid_iapp == 0) {
			DBGPRINT(RT_DEBUG_OFF, "Initialize IAPP\n");
#ifdef IAPP_OS_LINUX
			IAPP_Task((VOID *)wapp->IAPP_Ctrl_Block);
#endif /* IAPP_OS_LINUX */
		}
		else{
			return 0;
		}

		return 0;
	}

// TODO: WAPP deinit

#ifdef QOS_R1
	wapp_drv_qos_deinit(wapp);
#endif

//error2:
//	hotspot_reset_all_ap_resource(wapp);
error1:
	wapp_deinit_all_config(wapp);
error0:
	hotspot_deinit(wapp);
#ifdef MAP_SUPPORT
	map_bss_table_release(wapp->map);
	wapp_iface_deinit(wapp);
#endif
error:
	exit(-1);
}

