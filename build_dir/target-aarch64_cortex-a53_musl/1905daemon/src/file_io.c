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
#include <syslog.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stddef.h>
#include <unistd.h>
#include "p1905_managerd.h"
#include "cmdu_tlv.h"
#include "cmdu_message.h"
#include "multi_ap.h"
#include "debug.h"



int _1905_write_mid(char* name, unsigned short mid)
{
	FILE *f;
	int fd;
	f = fopen(name, "w");
	if (f == NULL)
	{
		debug(DEBUG_ERROR, "Failed to open '%s' for writing\n", name);
		return -1;
	}
	if (fprintf(f, "mid 0x%04x\n", mid) < 0) {
		debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
		if (fclose(f) < 0)
			debug(DEBUG_ERROR, "[%d]fclose fail!\n", __LINE__);
		return -1;
	}
	fd = fileno(f);
	fsync(fd);
	if (fclose(f) < 0)
		debug(DEBUG_ERROR, "[%d]fclose fail!\n", __LINE__);
	return 0;
}

int _1905_read_set_config(struct p1905_managerd_ctx *ctx, char *name,
	struct set_config_bss_info bss_info[], unsigned char max_bss_num,
	unsigned char *config_bss_num)
{
	FILE* f;
	int index, resv1, resv2, i, j;
	unsigned int mac_int[6] = {0};
	unsigned char mac[6] = {0};
	char op_class[4] = {0};
	unsigned char ssid[MAX_SSID_LEN] = {0};
#ifdef MAP_R2
	unsigned char ssid_tlv[33];
	unsigned int pvid = 0, vid = 0, value = 0, pcp = 0;
#endif
	int authmode = 0, encrytype = 0;
	unsigned char key[65] = {0};
	char content[4096] = {0};
	char *pos1 = NULL, *pos2 = NULL, *end_line = NULL;
#ifdef MAP_R2
	char *pos3 = NULL;
#endif
	signed char ch = 0, sch = 0;
	unsigned char hidden_ssid = 0;
	//unsigned char hidden_ssid_exist = 0;
	unsigned char fh_bss_unhidden = 0;
	unsigned int ssid_len = 0;
	char backslash = 0x5C;
	char space = 0x20;
	char SUB = 0x1A;
	int tmp = 0;


	f = fopen(name, "r");
	if (f == NULL) {
		debug(DEBUG_ERROR, "open file %s fail\n", name);
		return -1;
	}

	for (i = 0; i < max_bss_num; i++) {
		memset(&bss_info[i], 0, sizeof(struct set_config_bss_info));
	}

	i = 0;
	while ((ch = (signed char)fgetc(f)) != EOF) {
		/* If ssid is a space, need add \ as ESC infront of it in wts file. As space is regard as
		* a separator. If ssid is a \, need add \ as ESC in front of it. when parsing wts file,
		"\space"need remove \ and replace space to SUB. "\\"need remove \*/
		if (ch == backslash) {
			tmp = fgetc(f);
			if (tmp == EOF)
				break;
			sch = (signed char)tmp;
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
		debugbyte(DEBUG_TRACE, "%c", ch);
	}
	debugbyte(DEBUG_TRACE, "\n");

	i = 0;
	pos1 = content;
	while (1) {
		memset(ssid, 0, sizeof(ssid));
#ifdef MAP_R2
		pvid = INVALID_VLAN_ID;
		pcp = 0;
		vid = INVALID_VLAN_ID;
#endif
		hidden_ssid = 0;

		/*index*/
		pos2 = strchr(pos1, ',');
		if (pos2 == NULL) {
			debug(DEBUG_TRACE, "index not found\n");
			break;
		}
		*pos2 = '\0';
		if (sscanf(pos1, "%d", &index) < 0) {
			debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
			goto err;
		}
		pos2++;
		pos1 = pos2;

		/*mac*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			debug(DEBUG_ERROR, "mac not found\n");
			break;
		}
		*pos2 = '\0';
		if (sscanf(pos1, "%02x:%02x:%02x:%02x:%02x:%02x", mac_int,
			(mac_int + 1), (mac_int + 2), (mac_int + 3),
			(mac_int + 4), (mac_int + 5)) < 0) {
			debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
			goto err;
		}
		for (j = 0; j < 6; j++)
			mac[j] = (unsigned char)mac_int[j];

		pos2++;
		pos1 = pos2;

		/*opclass*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			debug(DEBUG_ERROR, "opclass not found\n");
			break;
		}
		*pos2 = '\0';

		memset(op_class, 0, sizeof(op_class));
		tmp = sscanf(pos1, "%s", op_class);
		if (tmp < 0 || tmp > sizeof(op_class)) {
			debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
			goto err;
		}
		op_class[sizeof(op_class) - 1] = '\0';
		debug(DEBUG_TRACE, "opclass %s\n", op_class);

		pos2++;
		pos1 = pos2;

		/*ssid*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			debug(DEBUG_ERROR, "ssid not found\n");
			break;
		}
		*pos2 = '\0';
		if (sscanf(pos1, "%32s", ssid) < 0) {
			debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
			goto err;
		}
		ssid[MAX_SSID_LEN  - 1] = '\0';
		/*replace SUB to space for ssid*/
		for (j = 0; j < 32; j++) {
			if (ssid[j] == SUB)
				ssid[j] = space;
		}
		pos2++;
		pos1 = pos2;

		/*authmode*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			debug(DEBUG_ERROR, "authmode not found\n");
			break;
		}
		*pos2 = '\0';
		if (sscanf(pos1, "0x%04x", &authmode) < 0) {
			debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
			goto err;
		}
		pos2++;
		pos1 = pos2;

		/*encrytype*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			debug(DEBUG_ERROR, "encrytype not found\n");
			break;
		}
		*pos2 = '\0';
		if (sscanf(pos1, "0x%04x", &encrytype) < 0) {
			debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
			goto err;
		}
		pos2++;
		pos1 = pos2;

		/*key*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			debug(DEBUG_ERROR, "key not found\n");
			break;
		}
		*pos2 = '\0';
		if (sscanf(pos1, "%64s", key) < 0) {
			debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
			goto err;
		}
		key[64] = '\0';
		/*replace SUB to space for passphase*/
		for (j = 0; j < 64; j++) {
			if (key[j] == SUB)
				key[j] = space;
		}
		pos2++;
		pos1 = pos2;

		/*resv1*/
		pos2 = strchr(pos1, ' ');
		if (pos2 == NULL) {
			debug(DEBUG_ERROR, "resv1 not found\n");
			break;
		}
		*pos2 = '\0';
		if (sscanf(pos1, "%d", &resv1) < 0) {
			debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
			goto err;
		}
		pos2++;
		pos1 = pos2;

		/*resv2*/
		//*pos2 and pos2 point to bh;
		if (sscanf(pos1, "%d", &resv2) < 0) {
			debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
			goto err;
		}

		end_line = strchr(pos1, '\n');
		if (!end_line)
			break;

		// no ts, no hiddden-ssid, to next line
		if (end_line - pos2 == 1) {
			pos2 += 2;
			pos1 = pos2;
		}
#ifdef MAP_R2
		if (ctx->MAP_Cer) {
			/* default vlan policy 0xB5*/
			pos2 = strstr(pos1, "B5");
			if (pos2 == NULL || pos2 >= end_line) {
				debug(DEBUG_ERROR, "default vlan tlv not found\n");
			} else {
				pos1 = pos2;

				/* move to tlv length */
				pos2 = strstr(pos1, "0x");
				if (pos2 == NULL) {
					debug(DEBUG_ERROR, "default vlan tlv len not found\n");
					break;
				}
				pos2 += 2;
				pos1 = pos2;
				if (sscanf(pos1, "%04x", &value) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto err;
				}

				/* move to vlan */
				pos2 = strstr(pos1, "0x");
				if (pos2 == NULL) {
					debug(DEBUG_ERROR, "default vlan not found\n");
					break;
				}
				pos2 += 2;
				pos1 = pos2;
				if (sscanf(pos1, "%04x", &pvid) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto err;
				}

				/* move to pcp */
				pos2 = strstr(pos1, "0x");
				if (pos2 == NULL) {
					debug(DEBUG_ERROR, "default pcp not found\n");
					break;
				}
				pos2 += 2;
				pos1 = pos2;
				if (sscanf(pos1, "%02x", &pcp) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto err;
				}
			}

			/* traffic separation policy */
			pos2 = strstr(pos1, "B6");
			if (pos2 == NULL || pos2 >= end_line) {
				debug(DEBUG_ERROR, "traffic seperation tlv not found\n");
			} else {
				pos1 = pos2;

				/* move to tlv length */
				pos2 = strstr(pos1, "0x");
				if (pos2 == NULL) {
					debug(DEBUG_ERROR, "traffic separation length not found\n");
					break;
				}
				pos2 += 2;
				pos1 = pos2;
				if (sscanf(pos1, "%04x", &value) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto err;
				}

				/* move to num ssid */
				pos2 = strstr(pos1, "0x");
				if (pos2 == NULL) {
					debug(DEBUG_ERROR, "num of ssid not found\n");
					break;
				}
				pos2 += 2;
				pos1 = pos2;
				if (sscanf(pos1, "%02x", &value) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto err;
				}

				/* move to ssid len */
				pos2 = strstr(pos1, "0x");
				if (pos2 == NULL) {
					debug(DEBUG_ERROR, "ssid len not found\n");
					break;
				}
				pos2 += 2;
				pos1 = pos2;
				if (sscanf(pos1, "%02x", &ssid_len) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto err;
				}

				/* ssid */
				pos2 = strchr(pos1, ' ');
				if (pos2 == NULL) {
					debug(DEBUG_ERROR, "ssid not found\n");
					break;
				}
				pos2 ++;
				pos1 = pos2;

				if (sscanf(pos1, "%32s", ssid_tlv) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto err;
				}
				ssid_tlv[sizeof(ssid_tlv) - 1] = '\0';
				debug(DEBUG_ERROR, "ssid tlv  %s\n",ssid_tlv);

				/* move to vlan */
				pos2 = strstr(pos1, "0x");
				if (pos2 == NULL) {
					debug(DEBUG_ERROR, "vlan not found\n");
					break;
				}
				pos2 += 2;
				pos1 = pos2;
				if (sscanf(pos1, "%04x", &vid) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto err;
				}

				/* end of vlan */
				pos2 = strchr(pos1, '\n');
				if (!pos2)
					break;
				else
					pos2 ++;
				pos1 = pos2;
			}
		}

#endif
		pos2 = strstr(pos1, "hidden-");
		if (pos2 == NULL || pos2 >= end_line) {
			debug(DEBUG_TRACE, "no hidden SSID exist\n");
		} else {
			pos1 = pos2;
			/*hidden ssid*/
			if (sscanf(pos1, "hidden-%c", &hidden_ssid) == 1) {
				pos2++;
				pos1 = pos2;
			} else {
				debug(DEBUG_ERROR, "wrong format hidden ssid\n");
				break;
			}
		}
#ifdef MAP_R2
		if (!ctx->MAP_Cer) {
			pos2 = strchr(pos1, ' ');
			pos3 = strchr(pos1, ',');
			if (pos2 == NULL || (pos3 < pos2 && pos3 != NULL)) {
				if (pos2 == NULL) {
					debug(DEBUG_ERROR, "vlan id not found\n");
				} else {
					debug(DEBUG_ERROR, "%p, %p\n", pos2, pos3);
				}
			} else {
				pos2++;
				pos1 = pos2;
				if (sscanf(pos1, "%d", &vid) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto err;
				}
				pos2++;
				pos1 = pos2;

				pos2 = strstr(pos1, "pvid");
				if (pos2 != NULL && (pos2 - pos1) < 10) {
					debug(DEBUG_ERROR, "default vlan found\n");
					ctx->map_policy.setting.primary_vid = vid;
					pvid = vid;
					pos1 = pos2;

					pos2 = strstr(pos1, " ");
					if (pos2 == NULL) {
						debug(DEBUG_ERROR, "default pcp not found\n");
						break;
					}
					pos2++;
					pos1 = pos2;
					if (sscanf(pos1, "%d", &pcp) < 0) {
						debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
						goto err;
					}
					ctx->map_policy.setting.dft.PCP = pcp;
					pos2++;
					pos1 = pos2;

					ctx->map_policy.tpolicy.updated = 1;
				} else {
					debug(DEBUG_TRACE, "pvid not found or too far\n");
				}
			}
		}
#endif
		bss_info[i].authmode = authmode;
		bss_info[i].encryptype = encrytype;
		memcpy(bss_info[i].mac, mac, 6);
		memcpy(bss_info[i].key, key, sizeof(key));
		memcpy(bss_info[i].oper_class, op_class, sizeof(op_class));
		bss_info[i].band_support = find_band_by_opclass(bss_info[i].oper_class);
		tmp = snprintf(bss_info[i].ssid, sizeof(bss_info[i].ssid), "%s", ssid);
		if (os_snprintf_error(sizeof(bss_info[i].ssid), tmp)) {
			debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
			goto err;
		}
		bss_info[i].wfa_vendor_extension |= (resv1 & 0x01) << 6;
		bss_info[i].wfa_vendor_extension |= (resv2 & 0x01) << 5;
#ifdef MAP_R2
		bss_info[i].pvid = pvid;
		bss_info[i].pcp = pcp;
		bss_info[i].vid = vid;
#endif
		if (!ctx->MAP_Cer)
			ssid_len = os_strlen(bss_info[i].ssid);
		bss_info[i].ssid_len = ssid_len;
		if (hidden_ssid == 'Y')
			bss_info[i].hidden_ssid = 1;
		else if (hidden_ssid == 'N')
			bss_info[i].hidden_ssid = 0;
		else
			bss_info[i].hidden_ssid = 0;
		debug(DEBUG_OFF, "set bss index=%d, mac="MACSTR","
			" opclass=%s, ssid=%s, ssid_len %d, authmode=%04x, encrytype=%04x, key=%s, "
			"bh_bss=%s, fh_bss=%s pvid %d, pcp %d, vlan %d hidden_ssid=%d\n",
			index, PRINT_MAC(mac), op_class, ssid,ssid_len,  authmode, encrytype, key,
			bss_info[i].wfa_vendor_extension & BIT_BH_BSS ? "1" : "0",
			bss_info[i].wfa_vendor_extension & BIT_FH_BSS ? "1" : "0",
			bss_info[i].pvid, bss_info[i].pcp, bss_info[i].vid,
			bss_info[i].hidden_ssid);

		if ((bss_info[i].wfa_vendor_extension & BIT_FH_BSS) &&
			(bss_info[i].hidden_ssid == 0))
				fh_bss_unhidden |= bss_info[i].band_support;

		i++;
		if (i >= max_bss_num) {
			debug(DEBUG_ERROR, "too much bss wireless setting info\n");
			i--;
			break;
		}
	}

	*config_bss_num = i;
	debug(DEBUG_OFF, "config_bss_num=%d\n", *config_bss_num);
	if (fclose(f) < 0) {
		debug(DEBUG_ERROR, "[%d]fclose fail!\n", __LINE__);
		return -1;
	}

	if (!(fh_bss_unhidden & BAND_2G_CAP)) {
		debug(DEBUG_OFF, "2G bss is hidden or empty! wps may fail!\n");
	}

	if (!(fh_bss_unhidden & BAND_5GL_CAP)) {
		debug(DEBUG_ERROR, "5GL bss is hidden or empty! wps may fail!\n");
	}

	if (!(fh_bss_unhidden & BAND_5GH_CAP)) {
		debug(DEBUG_ERROR, "5GH bss is hidden or empty! wps may fail!\n");
	}

	if (!(fh_bss_unhidden & BAND_6G_CAP)) {
		debug(DEBUG_ERROR, "6G bss is hidden or empty! wps may fail!\n");
	}
#ifdef MAP_R2
	delete_exist_traffic_policy(ctx, &ctx->map_policy.tpolicy);
	update_traffic_separation_policy(ctx, bss_info, *config_bss_num);
#endif
	return 0;
err:
	if (fclose(f) < 0)
		debug(DEBUG_ERROR, "[%d]fclose fail!\n", __LINE__);
	return -1;
}

char* get_raw_data_end(char *content)
{
	char flag_ch[5] = {'_', 0x0a, 0x0d, '\0', ' '};
	int i = 0;
	char* temp = NULL;
	char* temp_min = NULL;

	for(i = 0; i < 5; i++)
	{
		temp = strchr(content, flag_ch[i]);
		if(temp == NULL)
		{
			continue;
		}
		if((temp_min != NULL && temp_min > temp) || temp_min == NULL)
		{
			temp_min = temp;
		}
	}
	return temp_min;

}
char* str_locate(char *org_string, char *t_string)
{
	int i = 0;
	int org_len = 0, t_len = 0;

	org_len = strlen(org_string);
	t_len = strlen(t_string);

	for(i = 0; i <= org_len - t_len; i++)
	{
		if(!strncmp(org_string + i, t_string, t_len))
		{
			return (char* )(org_string + i);
		}
	}
	return NULL;
}
int _1905_read_dev_send_1905(struct p1905_managerd_ctx* ctx,
	char* name, unsigned char *almac, unsigned short* _type, unsigned short *tlv_len, unsigned char *pay_load)
{
	FILE* f;
	int i;
	char *content = NULL;
	unsigned short pay_load_len = 0;
	unsigned int value;
	char* pos1, *pos2;
	unsigned int mac_int[ETH_ALEN];
	unsigned char al_mac[ETH_ALEN];
	int msg_type = 0;
	signed char ch;
	unsigned char byte_value[3];
	unsigned char ts_tlv_flag =  0, ts_tlv_count = 0;
	unsigned char k = 0, num;
	unsigned char ssid[64] = {0};
#ifdef MAP_R3
	unsigned char sa_mac[ETH_ALEN] = {0}, da_mac[ETH_ALEN] = {0};
	unsigned int flag_mask = 0, add_rm_flag = 0;;
#endif

	f = fopen(name, "r");
	if (f == NULL)
	{
		debug(DEBUG_ERROR, "open file %s fail\n", name);
		return -1;
	}

	/*alloc more memory to support parse jumbo TLV from file.*/
	content = os_malloc(25*1024);
	if (!content) {
		debug(DEBUG_ERROR, "alloc memory for fail\n");
		goto error1;
	}

	os_memset(content, 0, 25*1024);

	i = 0;

	while ((ch = (signed char)fgetc(f)) != EOF)
		content[i++] = ch;
	debug(DEBUG_TRACE, "\n");

	hex_dump("raw content:", (unsigned char *)content, i);

	pos1 = content;
	/*al mac*/
	pos2 = strchr(pos1, ' ');
	if(pos2 == NULL)
	{
		debug(DEBUG_ERROR, "white space not found\n");
		goto error1;
	}
	*pos2 = '\0';
	if (sscanf(pos1, "%02x:%02x:%02x:%02x:%02x:%02x", mac_int, mac_int + 1,
			mac_int + 2, mac_int + 3, mac_int + 4, mac_int + 5) < 0) {
		debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
		goto error1;
	}
	for(i = 0; i < 6; i++)
	{
		al_mac[i] = (unsigned char)mac_int[i];
	}
	pos2++;
	pos1 = pos2;
	debug(DEBUG_TRACE, "almac:"MACSTR"\n", PRINT_MAC(al_mac));


	/*msg type*/
	pos2 = strchr(pos1, '\n');
	if(pos2 == NULL)
	{
		debug(DEBUG_ERROR, "\\n not  found\n");
		goto error1;
	}
	*pos2 = '\0';
	if (sscanf(pos1, "0x%04x", &msg_type) < 0) {
		debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
		goto error1;
	}
	pos2++;
	pos1 = pos2;
	debug(DEBUG_TRACE, "msg_type=0x%04x\n", msg_type);

	while(1)
	{
		pos1 = str_locate(pos1, "0x");
		/*end of data*/
		if(pos1 == NULL)
		{
			break;
		}
		/*skip '0x'*/
		pos1 += 2;

		pos2 = get_raw_data_end(pos1);

		if(pos2 == NULL)
		{
			debug(DEBUG_ERROR, "error find raw data end\n");
			goto error1;
		}
		if(pos2 < pos1)
		{
			debug(DEBUG_ERROR, "pos2 must larger than pos1\n");
			goto error1;
		}
		if(((int)(pos2 - pos1)) % 2 != 0)
		{
			pos1--;
			*pos1 = '0';
		}
		while(1)
		{
			memset(byte_value, 0, sizeof(byte_value));
			memcpy(byte_value, pos1, 2);
			if (sscanf((char *)byte_value, "%02x", &value) < 0) {
				debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
				goto error1;
			}
			pay_load[pay_load_len++] = (unsigned char)value;
			pos1 += 2;
			if(pos1 >= pos2)
			{
				break;
			}
		}

		/* TS TLV will appear as:
		 * 00:0c:43:48:98:FC 0x8003
		 * 0xB5 0x0003 0x000A 0x00 0xB6 0x0014 0x02 0x0E Multi-AP-5GL-1 0x000A 0x0E Multi-AP-5GL-2 0x0014
		 */

		if (value == TRAFFIC_SEPARATION_POLICY_TYPE)
			ts_tlv_flag = 1;

		if (ts_tlv_flag == 1)
			ts_tlv_count++;

		if (ts_tlv_count == 3) {
			num = value;
			for (k = 0; k < num; k++) {
				pos1 = str_locate(pos1, "0x");
				/*skip '0x', parse ssid length*/
				pos1 += 2;

				memset(byte_value, 0, sizeof(byte_value));
				memcpy(byte_value, pos1, 2);
				if (sscanf((char *)byte_value, "%02x", &value) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto error1;
				}
				pay_load[pay_load_len++] = (unsigned char)value;
				pos1 += 2;

				pos1 += 1; /*skip '_'*/
				pos2 = get_raw_data_end(pos1);
				memset(ssid, 0, 64);
				if (sscanf((char *)pos1, "%32s", ssid) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto error1;
				}
				memcpy(&pay_load[pay_load_len], ssid, value);
				pay_load_len += value;

				debug(DEBUG_ERROR, "ssid len=%d, ssid:%s\n", value, ssid);

				/*vlan id*/
				pos1 = str_locate(pos1, "0x");
				/*skip '0x'*/
				pos1 += 2;
				pos2 = get_raw_data_end(pos1);
				do {
					memset(byte_value, 0, sizeof(byte_value));
					memcpy(byte_value, pos1, 2);
					if (sscanf((char *)byte_value, "%02x", &value) < 0) {
						debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
						goto error1;
					}
					pay_load[pay_load_len++] = (unsigned char)value;
					pos1 += 2;
					if(pos1 >= pos2)
					{
						break;
					}
				}while(1);
				debug(DEBUG_ERROR, "vlanid:%u\n",
					(unsigned short)((pay_load[pay_load_len-2]<<8)|pay_load[pay_load_len-1]));
			}
			ts_tlv_flag = 0;
			ts_tlv_count = 0;
		}
#ifdef MAP_R3
		/* SP TLV will appear as:
		 * 00:0c:43:48:a0:6f, 0x8023,
		 * 0xB9,0x0009,0x00000000 0x80 0xFE 0x05 0x20 0x03,
		 * 0xB9,0x0009,0x00000001 0x80 0xFD 0x04 0x20 0x01,
		 * 0xB9,0x000E,0x00000002 0x80 0xFC 0x02 0x02 5c:80:b6:0f:c5:c1,
		 * 0xB9,0x0014,0x00000003 0x80 0xFB 0x01 0x0A a4:c3:f0:ec:8e:1b 5c:80:b6:0f:c5:c1,
		 * 0xB9,0x000E,0x00000004 0x80 0xFA 0x03 0x08 a4:c3:f0:ec:8e:11
		*/
		if (value == SERVICE_PRIORITIZATION_TULE_TYPE) {
			/*skip '0x', tlv length*/
			pos1 = str_locate(pos1, "0x");
			pos1 += 2;
			for (i = 0; i < 2; i ++) {
				memset(byte_value, 0, sizeof(byte_value));
				memcpy(byte_value, pos1, 2);
				if (sscanf((char *)byte_value, "%02x", &value) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto error1;
				}
				pay_load[pay_load_len++] = (unsigned char)value;
				pos1 += 2;
			}

			/*skip '0x', rule ID*/
			pos1 = str_locate(pos1, "0x");
			pos1 += 2;
			for (i = 0; i < 4; i ++) {
				memset(byte_value, 0, sizeof(byte_value));
				memcpy(byte_value, pos1, 2);
				if (sscanf((char *)byte_value, "%02x", &value) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto error1;
				}
				pay_load[pay_load_len++] = (unsigned char)value;
				pos1 += 2;
			}
			debug(DEBUG_OFF, "ruleid:0x%02x\n", value);

			/*skip '0x', add-remove filter*/
			pos1 = str_locate(pos1, "0x");
			pos1 += 2;
			memset(byte_value, 0, sizeof(byte_value));
			memcpy(byte_value, pos1, 2);
			if (sscanf((char *)byte_value, "%02x", &value) < 0) {
				debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
				goto error1;
			}
			pay_load[pay_load_len++] = (unsigned char)value;
			pos1 += 2;
			add_rm_flag = value;
			debug(DEBUG_OFF, "add-remove:0x%02x\n", value);

			/*skip '0x', precedence*/
			if (!add_rm_flag) {
				/* there is no any more data when add_rm_flag equals 0 */
				/* continue to check if any other rule need to be handled */
				debug(DEBUG_OFF, "no more data for this rule, continue to check other rules\n");
				continue;
			}

			pos1 = str_locate(pos1, "0x");
			pos1 += 2;
			memset(byte_value, 0, sizeof(byte_value));
			memcpy(byte_value, pos1, 2);
			if (sscanf((char *)byte_value, "%02x", &value) < 0) {
				debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
				goto error1;
			}
			pay_load[pay_load_len++] = (unsigned char)value;
			pos1 += 2;
			debug(DEBUG_OFF, "precedence:0x%02x\n", value);

			/*skip '0x', output*/
			pos1 = str_locate(pos1, "0x");
			pos1 += 2;
			memset(byte_value, 0, sizeof(byte_value));
			memcpy(byte_value, pos1, 2);
			if (sscanf((char *)byte_value, "%02x", &value) < 0) {
				debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
				goto error1;
			}
			pay_load[pay_load_len++] = (unsigned char)value;
			pos1 += 2;
			debug(DEBUG_OFF, "output:0x%02x\n", value);

			/*skip '0x', parse flag_mask*/
			pos1 = str_locate(pos1, "0x");
			pos1 += 2;
			memset(byte_value, 0, sizeof(byte_value));
			memcpy(byte_value, pos1, 2);
			if (sscanf((char *)byte_value, "%02x", &flag_mask) < 0) {
				debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
				goto error1;
			}
			pay_load[pay_load_len++] = (unsigned char)flag_mask;
			pos1 += 2;
			debug(DEBUG_OFF, "flag_mask:0x%02x\n", flag_mask);

			if (flag_mask & 0x20) {
				/*skip '0x', up*/
				pos1 = str_locate(pos1, "0x");
				pos1 += 2;
				memset(byte_value, 0, sizeof(byte_value));
				memcpy(byte_value, pos1, 2);
				if (sscanf((char *)byte_value, "%02x", &value) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto error1;
				}
				pay_load[pay_load_len++] = (unsigned char)value;
				pos1 += 2;
				debug(DEBUG_OFF, "up:0x%02x\n", value);
			}

			if (flag_mask & 0x08) {
				/*find source mac, smac*/
				pos1 = str_locate(pos1, ":");
				pos1 -= 2;
				if (sscanf(pos1, "%02x:%02x:%02x:%02x:%02x:%02x", mac_int, mac_int + 1,
					mac_int + 2, mac_int + 3, mac_int + 4, mac_int + 5) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto error1;
				}
				for(i = 0; i < 6; i++)
					sa_mac[i] = (unsigned char)mac_int[i];
				pos1 += 17;
				memcpy(&pay_load[pay_load_len], sa_mac, ETH_ALEN);
				pay_load_len += ETH_ALEN;
				debug(DEBUG_OFF, "SA: "MACSTR"\n", MAC2STR(sa_mac));
			}

			if (flag_mask & 0x02) {
				/*find dest mac, dmac*/
				pos1 = str_locate(pos1, ":");
				pos1 -= 2;
				if (sscanf(pos1, "%02x:%02x:%02x:%02x:%02x:%02x", mac_int, mac_int + 1,
					mac_int + 2, mac_int + 3, mac_int + 4, mac_int + 5) < 0) {
					debug(DEBUG_ERROR, "[%d]sscanf fail!\n", __LINE__);
					goto error1;
				}
				for(i = 0; i < 6; i++)
					da_mac[i] = (unsigned char)mac_int[i];
				pos1 += 17;
				memcpy(&pay_load[pay_load_len], da_mac, ETH_ALEN);
				pay_load_len += ETH_ALEN;
				debug(DEBUG_OFF, "DA: "MACSTR"\n", MAC2STR(da_mac));
			}
		}
#endif
	}
	hex_dump("dev_send_1905", pay_load, pay_load_len);
	*tlv_len = pay_load_len;
	*_type = (unsigned short)msg_type;
	memcpy(almac, al_mac, 6);
	if (fclose(f) < 0)
		debug(DEBUG_ERROR, "[%d]fclose fail!\n", __LINE__);
	os_free(content);
	return 0;
error1:
	if (fclose(f) < 0)
		debug(DEBUG_ERROR, "[%d]fclose fail!\n", __LINE__);
	if (content)
		os_free(content);
	return -1;


}

int _1905_write_wts_file_content(char *buf, char *name)
{
	FILE *f = NULL;
	size_t result;

	debug(DEBUG_OFF, "%s\n", __func__);

	f = fopen(name, "w+");
	if (f == NULL) {
		debug(DEBUG_ERROR, "%s: open file %s fail\n", __func__, name);
		return -1;
	}

	result = fwrite(buf, 1, strlen(buf), f);
	if (result != strlen(buf)) {
		debug(DEBUG_ERROR, "%s: write error", __func__);
		if (fclose(f) < 0)
			debug(DEBUG_ERROR, "[%d]fclose fail!\n", __LINE__);
		return -1;
	}

	if (fclose(f) < 0)
		debug(DEBUG_ERROR, "[%d]fclose fail!\n", __LINE__);
	return 0;
}

