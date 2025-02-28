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
 *
 * switch_layer.c: switch layer for MAP
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
#include <stdbool.h>

#include "ethernet_layer.h"
#include "debug.h"
#include "netlink_event.h"

#include "common.h"
#include "switch_mt753x_nl.h"
#include "switch_legacy_ioctl.h"
#include "switch_layer.h"

#define MT7622_7530_KT 1

bool nl_init_flag;
struct mt753x_attr *attres;
char *cfg_file = "/etc/ethernet_cfg.txt";

struct arl_setting arl;
static char eth_dev_name[IFNAMSIZ] = {0x00};
unsigned int lan_port_bitmap = 0;
unsigned int wan_port_bitmap = 0;
unsigned int ts_vlan_cnt = 0;
unsigned int transparent_vlan_cnt = 0;


TAILQ_HEAD(vlan_list, switch_vlan_list) switch_vlan_head;


char * config_get_line(char *s,
	int size, FILE *stream, int *line, char **_pos)
{
	char *pos, *end, *sstart;

    while (fgets(s, size, stream)) {
        (*line)++;
        s[size - 1] = '\0';
        pos = s;

        /* Skip white space from the beginning of line. */
        while (*pos == ' ' || *pos == '\t' || *pos == '\r')
            pos++;

        /* Skip comment lines and empty lines */
        if (*pos == '#' || *pos == '\n' || *pos == '\0')
            continue;

        /*
         * Remove # comments unless they are within a double quoted
         * string.
		 */
        sstart = os_strchr(pos, '"');
        if (sstart)
            sstart = os_strrchr(sstart + 1, '"');
        if (!sstart)
            sstart = pos;
        end = os_strchr(sstart, '#');
        if (end)
            *end-- = '\0';
        else
            end = pos + os_strlen(pos) - 1;

        /* Remove trailing white space. */
        while (end > pos &&
               (*end == '\n' || *end == ' ' || *end == '\t' ||
            *end == '\r'))
            *end-- = '\0';

        if (*pos == '\0')
            continue;

		if (_pos)
            *_pos = pos;
        return pos;
    }

    if (_pos)
        *_pos = NULL;
    return NULL;
}

int parse_arl_setting_by_str(struct arl_setting *arl, char *lan_vid_str,
				char *wan_vid_str, char *cpu_port_str)
{
	char *token = NULL;
	int vid = 0, cpu = 0, num = 0;
	unsigned short *tmp = NULL;

	if (lan_vid_str) {
		token = strtok(lan_vid_str, ";");

		while (token != NULL) {
			vid = atoi(token);
			if (vid > 0 && vid < 4095) {
				num++;
				tmp = (num == 1) ? os_malloc(sizeof(unsigned short)) :
					os_realloc(arl->lan_vid, sizeof(unsigned short) * num);
				if (!tmp) {
					debug(DEBUG_OFF, "alloc lan vid fail, num(%d)\n", num);
					free(arl->lan_vid);
					arl->lan_vid_num = 0;
					return -1;
				}
				arl->lan_vid = tmp;
				arl->lan_vid[num - 1] = vid;
				debug(DEBUG_OFF, "lan vid[%d]=%d\n", num - 1, vid);
				arl->lan_vid_num = num;
			} else {
				debug(DEBUG_OFF, "invalid vid(%d)\n", vid);
			}
			token = strtok(NULL, ";");
		}
	}

	if (wan_vid_str) {
		token = strtok(wan_vid_str, ";");

		num = 0;
		while (token != NULL) {
			vid = atoi(token);
			if (vid > 0 && vid < 4095) {
				num++;
				tmp = (num == 1) ? os_malloc(sizeof(unsigned short)) :
					os_realloc(arl->wan_vid, sizeof(unsigned short) * num);
				if (!tmp) {
					debug(DEBUG_OFF, "alloc wan vid fail, num(%d)\n", num);
					free(arl->wan_vid);
					arl->wan_vid_num = 0;
					return -1;
				}
				arl->wan_vid = tmp;
				arl->wan_vid[num - 1] = vid;
				debug(DEBUG_OFF, "wan vid[%d]=%d\n", num - 1, vid);
				arl->wan_vid_num = num;
			} else {
				debug(DEBUG_OFF, "invalid vid(%d)\n", vid);
			}
			token = strtok(NULL, ";");
		}
	}

	if (cpu_port_str) {
		token = strtok(cpu_port_str, ";");
		num = 0;
		while (token != NULL) {
			cpu = atoi(token);
			if (cpu == 5 || cpu == 6) {
				num++;
				tmp = (num == 1) ? os_malloc(sizeof(unsigned short)) :
					os_realloc(arl->cpu_port, sizeof(unsigned short) * num);
				if (!tmp) {
					debug(DEBUG_OFF, "alloc cpu vid fail, num(%d)\n", num);
					free(arl->cpu_port);
					arl->cpu_port_num = 0;
					return -1;
				}
				arl->cpu_port = tmp;
				arl->cpu_port[num - 1] = cpu;
				debug(DEBUG_OFF, "cpu_port[%d]=%d\n", num - 1, cpu);
				arl->cpu_port_num = num;
			} else {
				debug(DEBUG_OFF, "invalid cpu port(%d)\n", cpu);
			}
			token = strtok(NULL, ";");
		}
	}
	return 0;
}

int switch_read_config_file(char *file_path)
{
	FILE *file;
	char buf[256], *pos, *token;
	char tmpbuf[256];
	int line = 0;

	file = fopen(file_path, "r");

	if (!file) {
		debug(DEBUG_ERROR, "open ehternet cfg file (%s) fail, use default setting\n", file_path);
		return 0;
	}
	os_memset(buf, 0, 256);
	os_memset(tmpbuf, 0, 256);

	while (config_get_line(buf, sizeof(buf), file, &line, &pos)) {
		if (os_strlen(pos) >= sizeof(tmpbuf)) {
			debug(DEBUG_ERROR, "line(%s) too long! excced %d\n", pos, (int)sizeof(tmpbuf));
			continue;
		}
		(void)snprintf(tmpbuf, sizeof(tmpbuf), "%s", pos);
		token = strtok(pos, "=");

		if (token != NULL) {
			if (os_strcmp(token, "ethernet_dev_name") == 0) {
				token = strtok(NULL, "");
				if (token == NULL)
					continue;
				if (os_strlen(token) >= IFNAMSIZ) {
					debug(DEBUG_ERROR, "length of ethernet_deve_name exceeds IFNAMSIZ, use default value\n");
					continue;
				}
				os_strncpy(eth_dev_name, token, os_strlen(token));
				debug(DEBUG_OFF, "eth_dev_name: %s\n", token);
			} else if (os_strcmp(token, "lan_vid") == 0) {
				token = strtok(NULL, "");
				if (token == NULL)
					continue;
				debug(DEBUG_OFF, "lan_vid=%s\n", token);
				parse_arl_setting_by_str(&arl, token, NULL, NULL);
			} else if (os_strcmp(token, "wan_vid") == 0) {
				token = strtok(NULL, "");
				if (token == NULL)
					continue;
				debug(DEBUG_OFF, "wan_vid=%s\n", token);
				parse_arl_setting_by_str(&arl, NULL, token, NULL);
			} else if (os_strcmp(token, "cpu_port") == 0) {
				token = strtok(NULL, "");
				if (token == NULL)
					continue;
				debug(DEBUG_OFF, "cpu_port=%s\n", token);
				parse_arl_setting_by_str(&arl, NULL, NULL, token);
			}
		}
	}

	fclose(file);
	return 0;
}

int reg_read(int offset, int *value)
{
	int ret = 0;
	struct mt753x_attr attr;

	memset(&attr, 0, sizeof(attr));
	attr.dev_id = -1;
	attr.port_num = -1;
	attr.phy_dev = -1;

	if (nl_init_flag == true)
		ret = reg_read_nl(&attr, offset, value);
	else
		ret = reg_read_ioctl(offset, value);
	if (ret < 0)
		printf("switch read fail\n");

	return ret;
}

int reg_write(int offset, int value)
{
	int ret = 0;
	struct mt753x_attr attr;

	memset(&attr, 0, sizeof(attr));
	attr.dev_id = -1;
	attr.port_num = -1;
	attr.phy_dev = -1;

	if (nl_init_flag == true)
		ret = reg_write_nl(&attr, offset, value);
	else
		ret = reg_write_ioctl(offset, value);
	if (ret < 0)
		printf("switch write fail\n");

	return ret;
}

static int get_chip_name()
{
	int temp = 0;
#if defined(CONFIG_RALINK_MT7628)
	debug(DEBUG_OFF, "switch chip name is mt7628!\n");
	return 0x7628;
#endif
	/*judge 7530*/
	reg_read((0x7ffc), &temp);
	temp = temp >> 16;
	if (temp == 0x7530)
		return temp;
	/*judge 7531*/
	reg_read(0x781c, &temp);
	temp = temp >> 16;
	if (temp == 0x7531)
		return temp;
	return -1;
}

static int map_switch_defalut_setting(int chip_name)
{
	int i = 0, value = 0;

	if (chip_name == 0x7530) {
		if (arl.cpu_port_num > 1) {
			printf("mt7530 only supports one cpu port(5/6)\n");
			return -1;
		}
		if (arl.cpu_port[0] == 5)
			reg_write(0x10,0xffffffd0);
		else if (arl.cpu_port[0] == 6)
			reg_write(0x10,0xffffffe0);
		else {
			printf("invalid cpu port(%d)\n",arl.cpu_port[0]);
			return -1;
		}
	} else if (chip_name == 0x7531) {
		reg_read(0x4, &value);
		if (arl.cpu_port_num > 2) {
			printf("mt7531 only supports two cpu port(5/6) at most\n");
			return -1;
		}
		for (i = 0; i < arl.cpu_port_num; i++) {
			if (arl.cpu_port[i] != 5 && arl.cpu_port[i] != 6) {
				printf("invalid cpu port(%d)\n",arl.cpu_port[i]);
				return -1;
			}
			value |= (1 << arl.cpu_port[i]);
		}
		reg_write(0x4, value);
		printf("cpu forward control:0x%x\n",value);
	} else if (chip_name == 0x7628) {
		debug(DEBUG_OFF, "mt7628 switch UES SDK default setting!\n");
		return 0;
	}
	reg_write(0x34,0x8160816);
	reg_write(0xc,0x7181d);
	return 0;
}

int switch_layer_init()
{
	int err, chip_name;

	os_memset(&arl, 0, sizeof(struct arl_setting));

	TAILQ_INIT(&switch_vlan_head);

	err = switch_read_config_file(cfg_file);

	if (err)
		goto fail;

	/*set the defaut vid setting, lan_vid=1, wan_vid=2, cpu_port=6*/
	if (arl.lan_vid_num == 0 && arl.wan_vid_num == 0) {

		arl.lan_vid_num = 1;
		arl.lan_vid = os_malloc(sizeof(unsigned short));
		if (!arl.lan_vid)
			goto fail;
		arl.lan_vid[0] = 1;

		arl.wan_vid_num = 1;
		arl.wan_vid = os_malloc(sizeof(unsigned short));
		if (!arl.wan_vid)
			goto fail;
		arl.wan_vid[0] = 2;
		debug(DEBUG_ERROR, "use default vlan setting\n");
	}
	if (arl.cpu_port_num == 0) {

		arl.cpu_port_num = 1;
		arl.cpu_port = os_malloc(sizeof(unsigned short));
		if (!arl.cpu_port)
			goto fail;
		arl.cpu_port[0] = 6;

		debug(DEBUG_ERROR, "use default cpu port(6)\n");
	}

	err = switch_ioctl_init(os_strlen(eth_dev_name) > 0 ? (void*)eth_dev_name : NULL);
	if (mt753x_netlink_init() < 0) {

		nl_init_flag = false;
		if (err < 0) {
			debug(DEBUG_ERROR, "switch layer init fail\n");
			goto fail;
		} else
			goto suc;
   	}

   	nl_init_flag = true;

	attres = (struct mt753x_attr *)os_malloc(sizeof(struct mt753x_attr));
	if (!attres)
		goto fail;

	attres->dev_id = -1;
	attres->port_num = -1;
	attres->phy_dev = -1;
suc:
	chip_name = get_chip_name();
	if (chip_name < 0) {
		printf("no chip unsupport or chip id is invalid!\n");
		goto fail;
	}
	if (map_switch_defalut_setting(chip_name) < 0)
		goto fail;

	return err;
fail:
	if (arl.lan_vid)
		os_free(arl.lan_vid);
	if (arl.wan_vid)
		os_free(arl.wan_vid);
	if (arl.cpu_port)
		os_free(arl.cpu_port);
	return -1;
}

int switch_layer_deinit()
{
	int ret = 0;

	switch_ioctl_deinit(NULL);
	if (nl_init_flag == true) {
		free(attres);
		attres = NULL;
		ret = switch_nl_deinit();
	}
	if (arl.lan_vid)
		os_free(arl.lan_vid);
	if (arl.wan_vid)
		os_free(arl.wan_vid);
	if (arl.cpu_port)
		os_free(arl.cpu_port);

	return ret;
}


/*According to the mac address, searching the switch mac table*/
int _switch_port_find_by_mac(unsigned char *mac_addr, int vid, unsigned char *age)
{
	int i, j, value, value2 = 0;
	char tmpstr[9];

	//for (i = 0; i < ETH_ALEN - 2; i++)
	//	sprintf(tmpstr + i * 2, "%02x", mac_addr[i]);
	//tmpstr[8] = '\0';
	snprintf(tmpstr, sizeof(tmpstr), "%02x%02x%02x%02x",
			mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]);
	value = strtoul(tmpstr, NULL, 16);
	if (ERANGE == value) {
		debug(DEBUG_ERROR, "strtoul tmpstr fail\n");
		return -1;
	}
	reg_write(REG_ESW_WT_MAC_ATA1, value);
	//printf("REG_ESW_WT_MAC_ATA1 is 0x%x\n\r",value);

	//for (i = 4; i < ETH_ALEN; i++)
	//	snprintf(tmpstr + (i - 4) * 2, 2, "%02x", mac_addr[i]);
	//tmpstr[4] = '\0';
	(void)snprintf(tmpstr, sizeof(tmpstr), "%02x%02x", mac_addr[4], mac_addr[5]);
	value = strtoul(tmpstr, NULL, 16);
	value = (value << 16);
	value |= (1 << 15);//IVL=1

	j = vid;
	value |= j; //vid
	reg_write(REG_ESW_WT_MAC_ATA2, value);
	//printf("REG_ESW_WT_MAC_ATA2 is 0x%x\n\r",value);

	value = 0x8000;  //w_mac_cmd
	reg_write(REG_ESW_WT_MAC_ATC, value);
	//printf("REG_ESW_WT_MAC_ATC is 0x%x\n\r",value);
	usleep(1000);

	for (i = 0; i < 20; i++) {
		reg_read(REG_ESW_WT_MAC_ATC, &value);
		//printf("REG_ESW_WT_MAC_ATC is 0x%x\n\r",value);
		if ((value & 0x8000) == 0 ) { //mac address busy
			break;
		}
		usleep(1000);
	}
	if (i == 20) {
		debug(DEBUG_ERROR, "search timeout.\n");
		return -1;
	}

	if (value & 0x1000) {
		debug(DEBUG_WARN, "Address Entry is not valid:0x%x, and search no entry by vid=%d.\n", value,vid);
		return -1;
	}

	debug(DEBUG_INFO, "search done.\n");

	reg_read(REG_ESW_TABLE_ATRD, &value2);
	j = (value2 >> 4) & 0xff; //r_port_map

	*age = (value2 >> 24) & 0xff;//r_age_field

	return j;
}

int _switch_port_find_by_mac_fid(unsigned char *mac_addr, int fid, unsigned char *age)
{
	int i, j, value, value2 = 0;
	char tmpstr[9];

	//for (i = 0; i < ETH_ALEN - 2; i++)
	//	sprintf(tmpstr + i * 2, "%02x", mac_addr[i]);
	//tmpstr[8] = '\0';
	snprintf(tmpstr, sizeof(tmpstr), "%02x%02x%02x%02x",
			mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]);
	value = strtoul(tmpstr, NULL, 16);
	if (ERANGE == value) {
		debug(DEBUG_ERROR, "strtoul tmpstr fail\n");
		return -1;
	}
	reg_write(REG_ESW_WT_MAC_ATA1, value);
	//printf("REG_ESW_WT_MAC_ATA1 is 0x%x\n\r",value);

	//for (i = 4; i < ETH_ALEN; i++)
	//	snprintf(tmpstr + (i - 4) * 2, 2, "%02x", mac_addr[i]);
	//tmpstr[4] = '\0';
	(void)snprintf(tmpstr, sizeof(tmpstr), "%02x%02x", mac_addr[4], mac_addr[5]);
	value = strtoul(tmpstr, NULL, 16);
	value = (value << 16);
	value &= ~(1 << 15);//IVL=0

	j = fid;
	value |= (j << 12); //vid
	reg_write(REG_ESW_WT_MAC_ATA2, value);
	//printf("REG_ESW_WT_MAC_ATA2 is 0x%x\n\r",value);

	value = 0x8000;  //w_mac_cmd
	reg_write(REG_ESW_WT_MAC_ATC, value);
	//printf("REG_ESW_WT_MAC_ATC is 0x%x\n\r",value);
	usleep(1000);

	for (i = 0; i < 20; i++) {
		reg_read(REG_ESW_WT_MAC_ATC, &value);
		//printf("REG_ESW_WT_MAC_ATC is 0x%x\n\r",value);
		if ((value & 0x8000) == 0 ) { //mac address busy
			break;
		}
		usleep(1000);
	}
	if (i == 20) {
		debug(DEBUG_ERROR, "search timeout.\n");
		return -1;
	}

	if (value & 0x1000) {
		debug(DEBUG_WARN, "Address Entry is not valid:0x%x, and search no entry by fid=%d.\n", value,fid);
		return -1;
	}

	debug(DEBUG_INFO, "search done.\n");

	reg_read(REG_ESW_TABLE_ATRD, &value2);
	j = (value2 >> 4) & 0xff; //r_port_map

	*age = (value2 >> 24) & 0xff;//r_age_field

	return j;
}


/*create map ethernet table: port-mac-age*/
struct ethernet_client_entry * switch_create_table_entry(unsigned char port_index, unsigned char age,
	unsigned char* mac, unsigned char vid)
{
	struct ethernet_client_entry *entry = NULL;

	entry = (struct ethernet_client_entry*)os_malloc(sizeof(struct ethernet_client_entry));
	if (entry == NULL)
		return NULL;

	entry->port_index = port_index;
	entry->age = age;
	if (mac != NULL)
		os_memcpy(entry->mac, mac, ETH_ALEN);

	return entry;
}

/*clear map ethernet table */
int switch_clear_entry_list(struct dl_list *entry_list)
{
	struct ethernet_client_entry *entry, *entry_next;

	dl_list_for_each_safe(entry, entry_next, entry_list, struct ethernet_client_entry, entry) {
		dl_list_del(&entry->entry);
		os_free(entry);
	}
	return 0;
}
#if defined (CONFIG_RALINK_MT7628)
/****************************************************
*
*Function name : _get_7628_lan_wan_portmaps()
*Description   : input the vid , return it's portmaps
*	exp:	vid  fid  portmap
*  			 1    0  1111-111
*	vid: 1 portmap: should conert to uint.
*Parameter	:
*		@vid : input the lan_vid or wan_vid (user_cfg)
*		@port_map	: output the portmap (unsigned int)
*Return:
*		0: Success, Other fail
*other:
*		for 7628 switch use
***************************************************/

int _get_7628_lan_wan_portmaps(int vid, unsigned int *port_map)
{
	int i , j, vid_val, tmp_vid, portmap_val;
	unsigned int bit_offset1 = 1;
	unsigned int bit_offset2 = 1;
	printf("idx   vid  portmap\n");
	for (i = 0; i < 8; i++) {
		reg_read(REG_ESW_VLAN_ID_BASE + 4*i, &vid_val);
		reg_read(REG_ESW_VLAN_MEMB_BASE + 4*(i/2), &portmap_val);
		printf(" %2d  %4d  ", 2*i, vid_val & 0xfff);
		if (i%2 == 0) {
			printf("%c", (portmap_val & 0x00000001)? '1':'-');//p0
			printf("%c", (portmap_val & 0x00000002)? '1':'-');//p1
			printf("%c", (portmap_val & 0x00000004)? '1':'-');//p2
			printf("%c", (portmap_val & 0x00000008)? '1':'-');//p3
			printf("%c", (portmap_val & 0x00000010)? '1':'-');//p4
			printf("%c", (portmap_val & 0x00000020)? '1':'-');//p5
			printf("%c\n", (portmap_val & 0x00000040)? '1':'-');//p6
			bit_offset1 = 0;
		}
		else {
			printf("%c", (portmap_val & 0x00010000)? '1':'-');
			printf("%c", (portmap_val & 0x00020000)? '1':'-');
			printf("%c", (portmap_val & 0x00040000)? '1':'-');
			printf("%c", (portmap_val & 0x00080000)? '1':'-');
			printf("%c", (portmap_val & 0x00100000)? '1':'-');
			printf("%c", (portmap_val & 0x00200000)? '1':'-');
			printf("%c\n", (portmap_val & 0x00400000)? '1':'-');
			bit_offset1 = 16;
		}
		printf(" %2d  %4d  ", 2*i+1, ((vid_val & 0xfff000) >> 12));
		if (i%2 == 0) {
			printf("%c", (portmap_val & 0x00000100)? '1':'-');
			printf("%c", (portmap_val & 0x00000200)? '1':'-');
			printf("%c", (portmap_val & 0x00000400)? '1':'-');
			printf("%c", (portmap_val & 0x00000800)? '1':'-');
			printf("%c", (portmap_val & 0x00001000)? '1':'-');
			printf("%c", (portmap_val & 0x00002000)? '1':'-');
			printf("%c\n", (portmap_val & 0x00004000)? '1':'-');
			bit_offset2 = 8;
		}
		else {
			printf("%c", (portmap_val & 0x01000000)? '1':'-');
			printf("%c", (portmap_val & 0x02000000)? '1':'-');
			printf("%c", (portmap_val & 0x04000000)? '1':'-');
			printf("%c", (portmap_val & 0x08000000)? '1':'-');
			printf("%c", (portmap_val & 0x10000000)? '1':'-');
			printf("%c", (portmap_val & 0x20000000)? '1':'-');
			printf("%c\n", (portmap_val & 0x40000000)? '1':'-');
			bit_offset2 = 24;
		}

		tmp_vid = vid_val & 0xfff;
		debug(DEBUG_TRACE,"(%d) tmp_vid=%#x, vid=%#x, portmap_val=%#x\n",
		__LINE__, tmp_vid, vid,portmap_val);
		if (vid == tmp_vid && (portmap_val & 0x01) != 0) {
			for (j = 0; j < SWITCH_MAX_PORT; j++) {
				if (portmap_val & (BIT(j + bit_offset1)))
					*port_map |= (0x00000001 << j);
			}
			debug(DEBUG_INFO,"(%d) port_map=%#x, portmap_val=%#x\n",
		__LINE__,*port_map, portmap_val);
			break;
		}

		tmp_vid = ((vid_val & 0xfff000) >> 12);
		debug(DEBUG_TRACE,"(%d) tmp_vid=%#x, vid=%#x, portmap_val=%#x\n",
		__LINE__, tmp_vid, vid,portmap_val);
		if (vid == tmp_vid &&(portmap_val & 0x01) != 0) {
			for (j = 0; j < SWITCH_MAX_PORT; j++) {
				if (portmap_val & (BIT(j + bit_offset2)))
					*port_map |= (0x00000001 << j);
			}
			debug(DEBUG_TRACE,"(%d) port_map=%#x, portmap_val=%#x\n",
		__LINE__,*port_map, portmap_val);
			break;
		}

		usleep(1000);
	}

	if (j >= 8) {
		debug(DEBUG_ERROR, "get switch vlan info timeout\n");
		return -1;
	}
	debug(DEBUG_TRACE,"(%d) port_map=%#x\n",
		__LINE__,*port_map);
	return 0;
}

/****************************************************
*
*Function name : switch_layer_get_7628_table_entry()
*Description   :   register write for old switch chip like 7628 switch
*				to get sta mac entry.
*Parameter	:
*		@entry_list: ouput the sta mac entry to entry_list
*Return:
*		0: Success, Other fail
*other:
*		for 7628 switch use
***************************************************/

int switch_layer_get_7628_table_entry(struct dl_list *entry_list)
{
	int i, k, value, mac;
	int port_index = 0;
	unsigned char temp_mac[ETH_ALEN] = {0};
	int tmp_port_idx = 0;
	unsigned int tmp_age = 0;
	int vid[16] = {0};
	int err_cnt = 0;
	struct ethernet_client_entry *entry = NULL;

	if (entry_list == NULL) {
		debug(DEBUG_ERROR, "ERROR! input entry_list is null.");
		return -1;
	}

	for (i = 0; i < 8; i++) {
		reg_read(REG_ESW_VLAN_ID_BASE + 4*i, &value);
		vid[2 * i] = value & 0xfff;
		vid[2 * i + 1] = (value & 0xfff000) >> 12;
	}
	/*Init Dlist*/
	dl_list_init(entry_list);

	reg_write(REG_ESW_TABLE_SEARCH, 0x1);
	for (i = 0; i < 0x400; i++) {
		while(1) {
			reg_read(REG_ESW_TABLE_STATUS0, &value);
			if (value & 0x1) {
				/*search_rdy*/
				if ((value & 0x70) == 0) {
					err_cnt++;
					debug(DEBUG_ERROR,"found an unused entry (age = 3'b000), err_count(%d),please check!\n", err_cnt);
					break;
				}
				tmp_port_idx = (value >> 12) & 0x7f; //r_port_map
				tmp_age = (value >> 4) & 0x7;//age_time

				reg_read(REG_ESW_TABLE_STATUS2, &mac);
				/*int to mac addr array 0-3*/
				temp_mac[0] = (unsigned char) ((mac>>(3 * 8)) & 0xff);
				temp_mac[1] = (unsigned char) ((mac>>(2 * 8)) & 0xff);
				temp_mac[2] = (unsigned char) ((mac>>(1 * 8)) & 0xff);
				temp_mac[3] = (unsigned char) ((mac>>(0 * 8)) & 0xff);

				reg_read(REG_ESW_TABLE_STATUS1, &mac);
				/*int to mac addr array 4-5  */
				temp_mac[4] = (unsigned char) ((mac>>(1 * 8)) & 0xff);
				temp_mac[5] = (unsigned char) ((mac>>(0 * 8)) & 0xff);

				/*check the mac vid infos*/
					/*port_map infos*/
					for (k = 0; k < MAX_PHY_PORT; k++) {
					if (tmp_port_idx & (0x01 << k))
						break;
					}

					if (k >= MAX_PHY_PORT)
						break;

					if (k < MAX_PHY_PORT) {
						port_index = k;
						/*create the switch*/
						debug(DEBUG_TRACE,"port_index(%x):%d, age:%d, mac: "MACSTR"\n",
						tmp_port_idx,port_index, tmp_age, temp_mac[0],temp_mac[1],temp_mac[2],temp_mac[3],
						temp_mac[4],temp_mac[5]);
						entry = switch_create_table_entry(port_index, tmp_age, temp_mac, 0);
						if (entry == NULL) {
							debug(DEBUG_ERROR, "allocate switch client entry fail\n");
							break;
						}
						/*add entry to the dl*/
						dl_list_add(entry_list, &entry->entry);

					}else {
						debug(DEBUG_INFO,"Can't find the port index!\n");
						break;
					}

				if (value & 0x2) {
					debug(DEBUG_INFO, "end of table %d\n", i);
					goto succ;
				}
				break;
			}
			else if (value & 0x2) { //at_table_end
				debug(DEBUG_INFO, "found the last entry %d (not ready)\n", i);
				goto succ;
			}
			usleep(5000);
		}
		if (err_cnt > SWITCH_ERROR_COUNTER) {
			debug(DEBUG_ERROR,"abnormal case,ERR COUNT is over %d\n", SWITCH_ERROR_COUNTER);
			goto fail;
		}
		reg_write(REG_ESW_TABLE_SEARCH, 0x2); //search for next address
	}

fail:
	switch_clear_entry_list(entry_list);
	return -1;

succ:
	return 0;
}

/****************************************************
*
*Function name : _switch_port_find_by_mac()
*Description   : find the sepical mac vid of ageout time
*				 in switch dump table.
*Parameter	:
*		@mac_addr : input mac_addr
*		@vid      : input vid of want to know
*		@age      : output age out time of this mac
*Return:
*		-1: fail, other success return the port_map
*other:
*
***************************************************/

int _switch_7628_port_find_by_mac(unsigned char *mac_addr, int in_vid, unsigned char *age)
{
	int i, j, k, value, mac;
	unsigned char temp_mac[ETH_ALEN] = {0};
	unsigned int temp_vid =0;
	unsigned int temp_age = 0;
	int vid[16];

	for (i = 0; i < 8; i++) {
		reg_read(REG_ESW_VLAN_ID_BASE + 4*i, &value);
		vid[2 * i] = value & 0xfff;
		vid[2 * i + 1] = (value & 0xfff000) >> 12;
	}

	reg_write(REG_ESW_TABLE_SEARCH, 0x1);

	for (i = 0; i < 0x400; i++) {
		while(1) {
			reg_read(REG_ESW_TABLE_STATUS0, &value);
			if (value & 0x1) { //search_rdy
				if ((value & 0x70) == 0) {
					debug(DEBUG_ERROR,"found an unused entry (age = 3'b000), please check!\n");
					return -1;
				}
				j = (value >> 12) & 0x7f; //r_port_map
				temp_vid = vid[(value >> 7) & 0xf];//vid value
				temp_age = (value >> 4) & 0x7;

				reg_read(REG_ESW_TABLE_STATUS2, &mac);
				/*int to mac addr array 0-3*/
				temp_mac[0] = (unsigned char) ((mac>>(3 * 8)) & 0xff);
				temp_mac[1] = (unsigned char) ((mac>>(2 * 8)) & 0xff);
				temp_mac[2] = (unsigned char) ((mac>>(1 * 8)) & 0xff);
				temp_mac[3] = (unsigned char) ((mac>>(0 * 8)) & 0xff);

				reg_read(REG_ESW_TABLE_STATUS1, &mac);
				/*int to mac addr array 4-5  */
				temp_mac[4] = (unsigned char) ((mac>>(1 * 8)) & 0xff);
				temp_mac[5] = (unsigned char) ((mac>>(0 * 8)) & 0xff);
				/*check the mac vid infos*/
				if (!os_memcmp(temp_mac, mac_addr, ETH_ALEN)) {
					if (temp_vid == in_vid) {
						/*age time*/
						*age = temp_age;
						/*port_map infos*/
						for (k = 0; k < SWITCH_MAX_PORT; k++) {
							if (j & (0x01 << k)) {
								debug(DEBUG_TRACE,"find the mac_add: age: %d , port map(%#x): %d\n", *age, j, k);
								return j;
							}
						}
						if (k >= SWITCH_MAX_PORT) {
							debug(DEBUG_INFO,"Can't find the port map!");
							return -1;
						}
					}
				}

				if (value & 0x2) {
					debug(DEBUG_INFO,"end of table %d\n", i);
					return -1;
				}
				break;
			}
			else if (value & 0x2) { //at_table_end
				debug(DEBUG_INFO, "found the last entry %d (not ready)\n", i);
				return -1;
			}
			usleep(5000);
		}
		reg_write(REG_ESW_TABLE_SEARCH, 0x2); //search for next address
	}
	return -1;
}

#endif

int _get_lan_wan_portmaps(int vid, unsigned int *port_map)
{
	int j, value, value2;

	printf("  vid  fid	portmap    s-tag\n");
	value = (0x80000000 + vid);  //r_vid_cmd
	reg_write(REG_ESW_VLAN_VTCR, value);

	for (j = 0; j < 20; j++) {
		reg_read(REG_ESW_VLAN_VTCR, &value);
		if ((value & 0x80000000) == 0 ) {
			//vlan table is busy
			break;
		}
		usleep(1000);
	}
	if (j == 20) {
		debug(DEBUG_ERROR, "vlan dump timeout.\n");
		return -1;
	}

	reg_read(REG_ESW_VLAN_VAWD1, &value);
	reg_read(REG_ESW_VLAN_VAWD2, &value2);

	if ((value & 0x01) != 0) {
		printf(" %4d  ", vid);
		printf(" %2d ",((value & 0xe)>>1));
		printf(" %c", (value & 0x00010000)? '1':'-');	//p0
		printf("%c", (value & 0x00020000)? '1':'-');	//p1
		printf("%c", (value & 0x00040000)? '1':'-');	//p2
		printf("%c", (value & 0x00080000)? '1':'-');	//p3
		printf("%c", (value & 0x00100000)? '1':'-');	//p4
		printf("%c", (value & 0x00200000)? '1':'-');	//p5
		printf("%c", (value & 0x00400000)? '1':'-');	//p6
		printf("%c", (value & 0x00800000)? '1':'-');	//p7
		printf("	%4d\n", ((value & 0xfff0)>>4)) ; //service Tag

		for (j = 0; j < SWITCH_MAX_PORT; j++) {
			if (value & (BIT(j + BIT_OFFSET)))
				*port_map |= (0x00000001 << j);
		}
	} else  {
		/*print 16 vid for reference information*/
		if (vid<=16) {
			printf(" %4d  ", vid);
			printf(" %2d ",((value & 0xe)>>1));
			printf(" invalid\n");
		}
	}
	return 0;
}

int switch_layer_get_port_type(unsigned int *lport_map, unsigned int *wport_map)
{
	int i;

	*lport_map = 0;
	*wport_map = 0;

	for (i = 0; i < arl.lan_vid_num; i++) {
#if defined (CONFIG_RALINK_MT7628)
		_get_7628_lan_wan_portmaps((int)(arl.lan_vid[i]), lport_map);
#else
		_get_lan_wan_portmaps((int)(arl.lan_vid[i]), lport_map);
#endif
	}
	for (i = 0; i < arl.wan_vid_num; i++) {
#if defined (CONFIG_RALINK_MT7628)
		_get_7628_lan_wan_portmaps((int)(arl.wan_vid[i]), wport_map);
#else
		_get_lan_wan_portmaps((int)(arl.wan_vid[i]), wport_map);
#endif
	}
	if ((*lport_map == 0)  && (*wport_map ==0)) {
		debug(DEBUG_ERROR, "Please check switch vlan setting!\n");
	} else if (*wport_map == 0) {
		debug(DEBUG_ERROR, "wan connect to extrnal phy\n");
		*wport_map = 0x100;
	}

	lan_port_bitmap = *lport_map;
	wan_port_bitmap = *wport_map;
	return 0;
}

int switch_layer_get_table_entry(struct dl_list *entry_list)
{
	int i, j, k, value = 0, mac = 0, mac2 = 0, value2 = 0, counter = 0;
	struct ethernet_client_entry *entry = NULL;

	for (i = 0; i < 8; i++)
		reg_read(REG_ESW_VLAN_ID_BASE + 4*i, &value);

	if(entry_list == NULL)
		return -1;

	dl_list_init(entry_list);

	debug(DEBUG_INFO, "%d\n", __LINE__);
	reg_write(REG_ESW_WT_MAC_ATC, 0x8004);
	usleep(5);
	debug(DEBUG_INFO, "%d\n", __LINE__);
	for (i = 0; i < 0x800; i++) {
		while(1) {
			reg_read(REG_ESW_WT_MAC_ATC, &value);
			//printf("ATC =  0x%x\n", value);
			debug(DEBUG_INFO, "%d\n", __LINE__);
			//search_rdy and Address Table is not busy
			if ((value & (0x1 << 13)) && (((value >> 15) &0x1) == 0)) {
				usleep(5);
				counter = 0;
				reg_read(REG_ESW_TABLE_ATRD, &value2);
				j = (value2 >> 4) & 0xff; //r_port_map
				for (k = 0; k < MAX_PHY_PORT; k++) {
					if (j & (0x01 << k))
						break;
				}
				if (k >= MAX_PHY_PORT)
					break;

				if (k < MAX_PHY_PORT) {
					entry = switch_create_table_entry(k, 0, NULL, 0);
					if (entry == NULL) {
						debug(DEBUG_ERROR, "allocate switch client entry fail\n");
						break;
					}
				}
				usleep(5);
				debug(DEBUG_INFO, "%d\n", __LINE__);

				reg_read(REG_ESW_TABLE_TSRA2, &mac2);
				if (k < MAX_PHY_PORT) {
					entry->age = (value2 >> 24) & 0xff;
				}
				usleep(5);
				reg_read(REG_ESW_TABLE_TSRA1, &mac);
				if (k < MAX_PHY_PORT) {
					entry->mac[0] = (mac & 0xff000000) >> 24;
					entry->mac[1] = (mac & 0x00ff0000) >> 16;
					entry->mac[2] = (mac & 0x0000ff00) >> 8;
					entry->mac[3] = mac & 0x000000ff;
					entry->mac[4] = (((mac2 >> 16) & 0xffff) & 0x0000ff00) >> 8;
					entry->mac[5] = ((mac2 >> 16) & 0xffff) & 0x000000ff;

					dl_list_add(entry_list, &entry->entry);
				}

				debug(DEBUG_INFO, "%d\n", __LINE__);
				if ((value & 0x4000) && (((value >> 16) & 0xfff) == 0x7FF)){
					goto suc;
				}
				break;
			}
			else if ((value & 0x4000) && (((value >> 15) &0x1) == 0) && (((value >> 16) & 0xfff) == 0x7FF)){ //at_table_end
				debug(DEBUG_INFO, "%d\n", __LINE__);
				goto suc;
			}
			else {
				usleep(5);
				counter++;
				if (counter >= SWITCH_ERROR_COUNTER) {
					debug(DEBUG_ERROR, "abnormal case, exit%d\n", __LINE__);
					goto fail;
				}
			}
		}
		reg_write(REG_ESW_WT_MAC_ATC, 0x8005); //search for next address
		usleep(5);
	}

fail:
	switch_clear_entry_list(entry_list);
	return -1;
suc:
	debug(DEBUG_INFO, "%d\n", __LINE__);
	return 0;
}

int switch_layer_table_search(unsigned char *mac, struct ethernet_client_entry *entry)
{
	int port_map = 0, port_map_max = -1, i=0;
	unsigned char max_age = 0, age = 0;
	int port_map_cpu = 1 << MAX_LAN_PORT_NUM;
	if (entry == NULL)
		return -1;

	/*get the client with largest age*/
	for (i = 0; i < arl.lan_vid_num; i++) {
#if defined (CONFIG_RALINK_MT7628)
		port_map = _switch_7628_port_find_by_mac(mac, (int)(arl.lan_vid[i]), &age);
#else
		port_map = _switch_port_find_by_mac(mac, (int)(arl.lan_vid[i]), &age);
#endif
		/*only get lan port, don't get cpu port*/
		if (port_map < port_map_cpu && age > max_age) {
			max_age = age;
			port_map_max = port_map;
		}
	}

	for (i = 0; i < arl.wan_vid_num; i++) {
#if defined (CONFIG_RALINK_MT7628)
		port_map = _switch_7628_port_find_by_mac(mac, (int)(arl.wan_vid[i]), &age);
#else
		port_map = _switch_port_find_by_mac(mac, (int)(arl.wan_vid[i]), &age);
#endif
		/*only get wan port, don't get cpu port*/
		if (port_map < port_map_cpu && age > max_age) {
			max_age = age;
			port_map_max = port_map;
		}
	}
#if defined (CONFIG_RALINK_MT7628)
#else
	/*if vlan is not configed, use fid to search table*/
	port_map = _switch_port_find_by_mac_fid(mac, 0, &age);
	if (port_map < port_map_cpu && age > max_age) {
		max_age = age;
		port_map_max = port_map;
	}
#endif
	/*not find this entry*/
	if (port_map_max == -1)
		return -1;

	for (i = 0; i < MAX_PHY_PORT; i++) {
		if (port_map_max & (0x00000001 << i))
			break;
	}

	entry->age = max_age;
	entry->port_index = i;
	debug(DEBUG_WARN, "port_index=%d age=%d\n",entry->port_index, entry->age);

	return 0;
}

/*Get port phy link status*/
int switch_layer_get_port_status(unsigned char index, int *status)
{
	int ret = 0;
	int value;
	struct mt753x_attr attr;

	memset(&attr, 0, sizeof(attr));
	attr.dev_id = -1;
	attr.port_num = -1;
	attr.phy_dev = -1;

	if (index >= 32) {	        //host internal phy
		ret = mii_cl22_read_phy((index - 32), 1, &value);
		if (ret < 0) {
			debug(DEBUG_ERROR, "Get internel phy error\n");
			return -1;
		}
		*status = (value & 0x00000004) ? 1 : 0;

	} else if (index > 7) {			//extrnal phy
	/*Add your code according to third party phy*/
#ifdef  MT7622_7530_KT
		ret = mii_cl45_read_phy(index, 0x1, 0xE800, &value);
		if (ret < 0) {
			debug(DEBUG_ERROR, "access extrnal phy: ioctl error\n");
			return -1;
		}
		if ((value & 0x1) == 0) {
			ret = mii_cl45_read_phy(8, 0x4, 0xE812, &value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "access extrnal phy: ioctl error\n");
				return -1;
			}
			if (((value >> 13) & 0x1) == 0x1)
				*status = 1;    // link up
			else
				*status = 0;    // link down
		} else
			*status = (value & 0x1);
#endif

	} else {		//switch port phy
		if (nl_init_flag == true)
			ret = netlink_cl22_read_phy(&attr, index, 1, &value);
		else
			ret = mii_cl22_read_phy(index, 1, &value);
		if (ret < 0) {
			debug(DEBUG_ERROR, "Get switch port phy error\n");
			return -1;
		}
		*status = (value & 0x00000004) ? 1 : 0;
	}

	return ret;
}

int switch_layer_clear_table()
{
	int ret = 0;

#ifndef CONFIG_RALINK_MT7628
	int cnt = 100;

	/* switch clear */
	ret = reg_write(REG_ESW_WT_MAC_ATC, 0x8002);
	while (ret && cnt) {
		usleep(5000);
		ret = reg_write(REG_ESW_WT_MAC_ATC, 0x8002);
		cnt--;
	}

	if (cnt) {
		debug(DEBUG_OFF, "clear switch table success! retry remain(%d)\n", cnt);
	} else {
		debug(DEBUG_OFF, "clear switch table fail! retry %d times\n", cnt);
	}
#else
	debug(DEBUG_ERROR, "for 7628 swith, do not support\n");
	ret = -2;
#endif

	return ret;
}
/*
**	set ts vlan on wan port, wan cpu port, each lan port
**	vid[]:		vlan ids
**	vid_cnt:	num of vids
**	active:		1-active; 2-inactive
*/
int switch_vlan_set(unsigned short vid[], unsigned short vid_cnt, unsigned char active, vlan_type type)
{
	unsigned char ivl_en = 1;
	unsigned char fid = 0;
	unsigned short stag = 0;
	unsigned char portMap = 0;
	unsigned char tagPortMap = 0;
	struct switch_vlan_list *vlan_entry = NULL;
	struct switch_vlan_list *entry_temp = NULL;
	

	int value = 0;
	int value2 = 0;
	int reg = 0;
	int i;

	portMap = lan_port_bitmap;
	
	if (e_transparent_vlan == type) {
		for (i = 0; i <= SWITCH_MAX_PORT; i ++) {
			if (wan_port_bitmap & BIT(i) && i < 5) {
				/* but why wan port include 4,5 */
				portMap |= (0x01 << i);
			}
		}
	}

	tagPortMap = portMap;

	if (!vid_cnt)
		active = 0;

	value = (portMap << 16);
	value |= (stag << 4);
	value |= (ivl_en << 30);
	value |= (fid << 1);
	value |= (active ? 1 : 0);

	// total 7 ports
	for (i = 0; i < 7; i++) {
		if (tagPortMap & (1 << i))
			value2 |= 0x2 << (i * 2);
	}

	if (value2)
		value |= (1 << 28); // eg_tag

	reg = 0x98; // VAWD2
	reg_write(reg, value2);

	reg = 0x94; // VAWD1
	reg_write(reg, value);

	/* vid_cnt != 0, activate or inactivate switch vlan table according to active flag*/
	if (vid_cnt) {
		for (i = 0; i < vid_cnt; i ++) {

			reg = 0x90; // VTCR
			value = (0x80001000 + vid[i]);
			reg_write(reg, value);
			usleep(5000);

			vlan_entry = malloc(sizeof(struct switch_vlan_list));
			if (!vlan_entry) {
				printf("\n malloc vlan entry fail\n");
				return -1;
			}
			else {
				printf("%s vlan id %d added\n", 
					type==e_traffic_separation_vlan?
					"traffic separation":"transparent vlan", vid[i]);
				TAILQ_INSERT_TAIL(&switch_vlan_head, vlan_entry, entry);

				vlan_entry->vid = vid[i];
				vlan_entry->vid_type = type;
				vlan_entry->port_bitmap = portMap;
				vlan_entry->cpu_port = tagPortMap;
			}
		}
		
		/* cnt of any vlan type is bigger than 0, set to user mode */
		if (!ts_vlan_cnt && !transparent_vlan_cnt) {
			value = 0x81000000;
			printf("2n10 set to user mode 0x%0x\n", value);
			for (i = 0; i < 7; i++) {
				reg = 0x2010 + i * 0x100;
				reg_write(reg, value);
				usleep(5000);
			}
		}
		
		if (e_traffic_separation_vlan == type)
			ts_vlan_cnt = vid_cnt;
		else
			transparent_vlan_cnt = vid_cnt;
		
	}
	/* vid_cnt = 0, inactivate switch vlan talbe in switch */
	else{
		/* look up all the vlan entry from list and delete them */
		vlan_entry = TAILQ_FIRST(&switch_vlan_head);
		while(vlan_entry) {
			entry_temp = TAILQ_NEXT(vlan_entry, entry);
			if (vlan_entry->vid_type == type) {
				/* disable vlan in switch first */
				reg = 0x90; // VTCR
				value = (0x80001000 + vlan_entry->vid);
				reg_write(reg, value);
				usleep(5000);

				printf("%s vlan id %d removed\n", 
					type==e_traffic_separation_vlan?
					"traffic separation":"transparent vlan", vlan_entry->vid);

				/* rm entry from list */
				TAILQ_REMOVE(&switch_vlan_head, vlan_entry, entry);
				free(vlan_entry);
			}

			vlan_entry = entry_temp;
		}


		if (e_traffic_separation_vlan == type)
			ts_vlan_cnt = 0;
		else
			transparent_vlan_cnt = 0;
		
		/* if cnt of each type of vlan is 0, reset to transparent mode as golden branch  */
		if (!ts_vlan_cnt && !transparent_vlan_cnt) {
			value = 0x810000c0;
			printf("2n10 set to transparent mode 0x%0x\n", value);
			for (i = 0; i < 7; i++) {
				reg = 0x2010 + i * 0x100;
				reg_write(reg, value);
				usleep(5000);
			}
		}
	}
		

	reg = 0x90; // VTCR
	while (1) {
		reg_read(reg, &value);
		if ((value & 0x80000000) == 0){ //table busy
			break;
		}
	}

	/* switch clear */
	reg = 0x80;
	reg_write(reg, 0x8002);
	usleep(5000);
	reg_read(reg, &value);

	return 0;
	
}




/*
**	set ts vlan on wan port, wan cpu port, each lan port
**	vid[]:		vlan ids
**	vid_cnt:	num of vids
**	active:		1-active; 2-inactive
*/
int switch_ts_vlan_set(unsigned short vid[], unsigned short vid_cnt, unsigned char active)
{
	return switch_vlan_set(vid, vid_cnt, active, e_traffic_separation_vlan);
}

/*
**	set transparent vlan on wan port, lan cpu port, each lan port
**	vid[]:		vlan ids
**	vid_cnt:	num of vids
**	active:		1-active; 2-inactive
*/
int switch_transparent_vlan_set(unsigned short vid[], unsigned short vid_cnt, unsigned char active)
{
	return switch_vlan_set(vid, vid_cnt, active, e_transparent_vlan);
}

struct eth_ops switch_layer_ops = {
	.init = switch_layer_init,
	.get_port_type = switch_layer_get_port_type,
#if defined (CONFIG_RALINK_MT7628)
	.get_table_entry = switch_layer_get_7628_table_entry,
#else
	.get_table_entry = switch_layer_get_table_entry,
#endif
	.search_table = switch_layer_table_search,
	.get_port_status = switch_layer_get_port_status,
	.set_switch_ts_vlan = switch_ts_vlan_set,
	.set_switch_transparent_vlan = switch_transparent_vlan_set,
	.clear_table = switch_layer_clear_table,
	.deinit = switch_layer_deinit,
};
