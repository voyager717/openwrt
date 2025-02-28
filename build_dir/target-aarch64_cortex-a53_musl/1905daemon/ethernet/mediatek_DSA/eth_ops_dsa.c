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
 * eth_ops_dsa.c: switch layer for MAP on DSA architecture
 * Author: Zheng Zhou<zheng.zhou@mediatek.com>
 */

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <pthread.h>
#include <stdbool.h>
#include "libbridge.h"

#include "ethernet_layer.h"
#include "debug.h"
#include "os.h"

#define MAX_INF_NUM	8
#define MAX_EXT_PHY_PORT_NUM 2
#define MTKETH_MII_READ 0x89F3
#define REG_NUM	1

struct dsa_inf {
	char if_name[IFNAMSIZ];
	unsigned int port_no;
	unsigned char real_index;
	unsigned char valid;
	int sock;
};

struct mtk_mii_ioctl_data {
	unsigned short phy_id;
	unsigned short reg_num;
	unsigned int val_in;
	unsigned int val_out;
};


#define IN
#define OUT

char br_lan[IFNAMSIZ] = "br-lan";

struct dsa_inf inf[MAX_INF_NUM];
unsigned char ext_phy_port[MAX_EXT_PHY_PORT_NUM];
int sk_fd;
int sk_fd_ext_phy;

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

int parse_eth_lan_interface(char *str)
{
	int lan_cnt = 0;
	char *token = NULL;
	int tmp_len = 0;

	token = strtok(str, " ");

	if (!token) {
		tmp_len = sizeof(inf[lan_cnt].if_name);
		os_strncpy(inf[lan_cnt++].if_name, str, tmp_len - 1);
		inf[lan_cnt - 1].if_name[tmp_len - 1] = '\0';
		debug(DEBUG_ERROR, "lan_interface[%d]=%s\n", lan_cnt - 1, inf[lan_cnt - 1].if_name);
		goto end;
	}

	while (token) {
		if (lan_cnt >= MAX_INF_NUM) {
			debug(DEBUG_ERROR, "lan interface count exceeds MAX ETH INTERFACE count(5)\n");
			goto end;
		}
		if (os_strlen(token) >= IFNAMSIZ)
			goto end;
		os_strncpy(inf[lan_cnt++].if_name, token, os_strlen(token));
		token = strtok(NULL, " ");
		debug(DEBUG_ERROR, "lan_interface[%d]=%s\n", lan_cnt - 1, inf[lan_cnt - 1].if_name);
	}

end:
	return 0;
}


int parse_ext_phy_port(char *str)
{
	int phy_port_cnt = 0;
	char *token = NULL;

	token = strtok(str, " ");
	if (!token) {
		ext_phy_port[phy_port_cnt++] = (unsigned char)atoi(str);
		goto end;
	}

	while (token) {
		if (phy_port_cnt >= MAX_EXT_PHY_PORT_NUM) {
			debug(DEBUG_ERROR, "ext phy port exceeds MAX count(2)\n");
			goto end;
		}
		/* port no should be less than 10 */
		if ((unsigned char)atoi(token) >= MAX_INF_NUM) {
			debug(DEBUG_ERROR, "port no[%s] bigger than %d\n", token, MAX_INF_NUM);
			goto end;
		}

		ext_phy_port[phy_port_cnt++] = (unsigned char)atoi(token);
		token = strtok(NULL, " ");
		debug(DEBUG_ERROR, "ext_phy_port[%d]=%d\n", phy_port_cnt - 1, ext_phy_port[phy_port_cnt - 1]);
	}

end:
	return 0;
}


int read_config_file(char *file_path)
{
	FILE *file;
	char buf[256], *pos, *token;
	char tmpbuf[256];
	int line = 0;
	int res = 0;

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
		res = snprintf(tmpbuf, sizeof(tmpbuf), "%s", pos);
		if (os_snprintf_error(sizeof(tmpbuf), res))
			debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		token = strtok(pos, "=");

		if (token != NULL) {
			if (os_strcmp(token, "lan") == 0) {
				token = strtok(NULL, "");
				if (token == NULL)
					continue;
				parse_eth_lan_interface(token);
			} else if (os_strcmp(token, "br_lan") == 0) {
				token = strtok(NULL, "");
				if (token == NULL)
					continue;
				if (os_strlen(token) >= IFNAMSIZ) {
					debug(DEBUG_ERROR, "br_lan name size error %s\n", token);
					continue;
				}
				os_strncpy(br_lan, token, os_strlen(token));
				debug(DEBUG_ERROR, "br_lan=%s\n", br_lan);
			}  else if (os_strcmp(token, "ext_phy_port") == 0) {
				debug(DEBUG_ERROR, "parse ext phy port\n");
				token = strtok(NULL, "");
				if (token == NULL)
					continue;
				parse_ext_phy_port(token);
			}
			else {
				debug(DEBUG_ERROR, "unknown config line %s\n", token);
			}
		}
	}

	if (fclose(file) < 0)
		debug(DEBUG_ERROR, "[%d]fclose fail!\n", __LINE__);
	return 0;
}

char *cfg_file = "/etc/ethernet_cfg.txt";

static int get_port_no(const char *br, const char *p,  void *arg)
{
	struct port_info pinfo;
	struct dsa_inf *intf = (struct dsa_inf*)arg;

	if (br_get_port_info(br, p, &pinfo)) {
		debug(DEBUG_ERROR, "Can't get info for %p",p);
		return 0;
	}

	if (os_strlen(p) != os_strlen(intf->if_name))
		return 0;

	if (os_strncmp(p, intf->if_name, os_strlen(intf->if_name)))
		return 0;

	intf->port_no = pinfo.port_no;

	debug(DEBUG_ERROR, "%s (%d)\n", p, pinfo.port_no);
	return 0;
}

unsigned int get_inf_portno_on_bridge(struct dsa_inf *interface)
{
	int err = 0;

	err = br_foreach_port(br_lan, get_port_no, (void*)interface);
	if (err < 0)
		debug(DEBUG_ERROR, "can't get ports: %s\n", strerror(-err));

	return err;
}

int sockets_open()
{
	int fd = 0;

//	fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	/* fd = socket(PF_UNIX, SOCK_DGRAM, 0); */
	fd = socket(PF_PACKET, SOCK_RAW, htobe16(ETH_P_ALL));
	if (fd > 0)
		return fd;
	else {
		debug(DEBUG_ERROR, "sockets_open error\n");
		return fd;
	}
}

int init()
{
	int ret = ETH_SUCCESS, i = 0;

	os_memset(inf, 0, sizeof(inf));
	os_memset(ext_phy_port, 0xff, sizeof(ext_phy_port));

	read_config_file(cfg_file);
	sk_fd = sockets_open();
	if (sk_fd < 0) {
		debug(DEBUG_ERROR, "sockets open failure!!\n");
		ret = ETH_ERROR_FAIL;
		goto fail;
	}

	sk_fd_ext_phy = socket(AF_INET, SOCK_DGRAM, 0);
	if (sk_fd_ext_phy < 0) {
		debug(DEBUG_ERROR, "fail to create socket for mii_mgr\n");
		ret = ETH_ERROR_FAIL;
		goto fail;
	}

	for (i = 0; i < MAX_INF_NUM; i++) {
		if (os_strlen(inf[i].if_name) > 0) {
			inf[i].real_index = i;
			get_inf_portno_on_bridge(&inf[i]);
			if (inf[i].port_no) {
				inf[i].valid = 1;
			}
		}
	}

	if (br_init()) {
		debug(DEBUG_ERROR, "sockets open failure!!\n");
		ret = ETH_ERROR_FAIL;
	}

	return ret;
fail:
	if (sk_fd > 0)
		close(sk_fd);
	if (sk_fd_ext_phy > 0)
		close(sk_fd_ext_phy);
	return ret;
}

int get_port_type(OUT unsigned int *lport_map, OUT unsigned int *wport_map)
{
	int ret = ETH_SUCCESS;

	*lport_map = 0x0000000F;
	*wport_map = 0x00000010;

	return ret;
}

struct dsa_inf *get_dsa_inf_by_portno(unsigned int portno)
{
	int i = 0;

	for (i = 0; i < MAX_INF_NUM; i++) {
		if (!inf[i].valid)
			continue;

		if (inf[i].port_no == portno)
			return &inf[i];
	}

	return NULL;
}

struct fdb_entry *get_all_fdb_entry(OUT int *num)
{
#define CHUNK 128
	struct fdb_entry *fdb = NULL;
	int offset = 0, n = 0;

	for (;;) {
		struct fdb_entry *fdb_tmp;

		fdb_tmp = os_realloc(fdb, (offset + CHUNK) * sizeof(struct fdb_entry));

		if (!fdb_tmp) {
			debug(DEBUG_ERROR, "Out of memory\n");
			os_free(fdb);
			return NULL;
		}

		fdb = fdb_tmp;
		if (offset == 0)
			os_memset(fdb, 0, CHUNK * sizeof(struct fdb_entry));
		else if (offset > 0)
			os_memset(fdb + CHUNK + offset - n, 0, n * sizeof(struct fdb_entry));
		n = br_read_fdb(br_lan, fdb+offset, offset, CHUNK);
		if (n == 0)
			break;

		if (n < 0) {
			debug(DEBUG_ERROR, "read of forward table failed: %s\n",
				strerror(errno));
			os_free(fdb);
			return NULL;
		}

		offset += n;
	}

	*num = offset;
	return fdb;
}

int get_table_entry(OUT struct dl_list *entry_list)
{
#define CHUNK 128
	int ret = ETH_SUCCESS, i = 0;
	struct ethernet_client_entry *entry = NULL;
	struct fdb_entry *fdb = NULL;
	int offset = 0;
	struct dsa_inf *pinf = NULL;

	dl_list_init(entry_list);

	fdb = get_all_fdb_entry(&offset);

	for (i = 0; i < offset; i++) {
		const struct fdb_entry *f = fdb + i;

		if (f->is_local)
			continue;

		pinf = get_dsa_inf_by_portno(f->port_no);

		if (!pinf)
			continue;

		entry = (struct ethernet_client_entry *)os_zalloc(sizeof(struct ethernet_client_entry));

		if (!entry)
			continue;

		entry->port_index = pinf->real_index;
		os_memcpy(entry->mac, f->mac_addr, ETH_ALEN);
		entry->age = 150 - f->ageing_timer_value.tv_sec / 2;
		dl_list_add(entry_list, &entry->entry);
	}

	if (fdb)
		os_free(fdb);

	return ret;
}

int search_table(IN unsigned char *mac, OUT struct ethernet_client_entry *entry)
{
	int i = 0;
	struct fdb_entry *fdb = NULL;
	int offset = 0;
	struct dsa_inf *pinf = NULL;

	fdb = get_all_fdb_entry(&offset);
	for (i = 0; i < offset; i++) {
		const struct fdb_entry *f = fdb + i;

		if (!os_memcmp(mac, f->mac_addr, ETH_ALEN)) {
			pinf = get_dsa_inf_by_portno(f->port_no);
			if (pinf) {
				entry->port_index = pinf->real_index;
				os_memcpy(entry->mac, f->mac_addr, ETH_ALEN);
				entry->age = 150 - f->ageing_timer_value.tv_sec / 2;
				os_free(fdb);
				return ETH_SUCCESS;
			}
		}
	}

	os_free(fdb);

	return ETH_ERROR_FAIL;
}


static void fill_mtk_mii_ioctl(
	struct mtk_mii_ioctl_data *mtk_mii, unsigned short phy_id,
	unsigned short reg_num, unsigned int *val)
{
	mtk_mii->phy_id  = phy_id;
	mtk_mii->reg_num = reg_num;
	mtk_mii->val_in  = *val;
	mtk_mii->val_out = 0;
}



static int get_phy_status_by_index(
	unsigned short index,
	unsigned int reg_num,
	unsigned int *val,

	unsigned short cmd)
{
	struct ifreq ifr;
	struct mtk_mii_ioctl_data mtk_mii;
	int err;

	strncpy(ifr.ifr_name, "eth0", sizeof(ifr.ifr_name));

	fill_mtk_mii_ioctl(&mtk_mii, index, reg_num, val);
	ifr.ifr_data = (char *)&mtk_mii;

	err = ioctl(sk_fd_ext_phy, cmd, &ifr);
	if (err)
		return -1;

	*val = (mtk_mii.val_out & 0x00000004) ? 1 : 0;
	return 0;
}


int get_port_status(IN unsigned char index, OUT int *status)
{
	int ret = ETH_SUCCESS, i = 0;
	struct ifreq ifr;
	unsigned int tmp_stat = 0;

	if (sk_fd <= 0)
		return ETH_ERROR_FAIL;

	for (i = 0; i < MAX_INF_NUM; i++) {
		if (inf[i].valid && inf[i].real_index == index) {
			os_memset(&ifr, 0, sizeof(ifr));
			os_strncpy(ifr.ifr_name, inf[i].if_name, os_strlen(inf[i].if_name));
			if (ioctl(sk_fd, SIOCGIFFLAGS, &ifr) < 0) {
				debug(DEBUG_ERROR, "ioctl error %s %s\n", strerror(errno), ifr.ifr_name);
				return ETH_ERROR_FAIL;
			}
		  	if (ifr.ifr_flags & IFF_RUNNING)
				*status = 1;
			else
				*status = 0;

			break;
		}
	}
	for (i = 0; i < MAX_EXT_PHY_PORT_NUM; i++) {
		if (ext_phy_port[i] == index) {
			ret = get_phy_status_by_index(index, REG_NUM, &tmp_stat, MTKETH_MII_READ);
			if (!ret)
				*status = (int)tmp_stat;
			break;
		}
	}

	return ret;
}

int clear_table()
{
	int ret = ETH_SUCCESS, i = 0;
	char cmd[1024] = {0};
	int res = 0;

	for (i = 0; i < MAX_INF_NUM; i++) {
		if (!inf[i].valid)
			continue;

		memset(cmd, 0, sizeof(cmd));
		res = snprintf(cmd, sizeof(cmd), "bridge fdb show dev %s"
			" | while read mac master br_name; do echo $mac; "
			"bridge fdb delete $mac dev lan1; done;", inf[i].if_name);
		if (os_snprintf_error(sizeof(cmd), res))
			debug(DEBUG_ERROR, "[%d] snprintf fail!\n", __LINE__);

		system(cmd);
	}

	return ret;
}

int deinit()
{
	close(sk_fd);
	close(sk_fd_ext_phy);
	br_shutdown();

	return ETH_SUCCESS;
}

struct eth_ops switch_layer_ops = {
	.init = init,
	.get_port_type = get_port_type,
	.get_table_entry = get_table_entry,
	.search_table = search_table,
	.get_port_status = get_port_status,
	.clear_table = clear_table,
	.deinit = deinit,
};
