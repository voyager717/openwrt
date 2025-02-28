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

#include <sys/types.h>
#include <sys/socket.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <string.h>
#include <errno.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>

#ifndef __user
#define __user
#endif /* __user */

#include <linux/wireless.h>

#define FWDD_VERSION "2.0"

#define LOOP_DETECT_IN_DAEMON 0

#define BUFLEN 1024	/*1K Bytes*/

#define	IFNAMSIZ	16
#define MAX_TBL_SIZE	64
#define MAX_LINK_NUM	20
#define MAC_ADDR_LEN	6
#define MAX_MBSS_NUM	64

#define MAX_PAYLOAD	1024
#define NETLINK_EXT	25
#define PORT1		5001
#define MAGIC		"MAGIC\0"
#define MG_LEN		6

#define usec_5s	5000000 /*5s*/
#define sec_5s	5 	/*5s*/
#define sec_1s	1 	/*1s*/

#define LINK_POLL_INTERVAL	5 	/*5s*/
#define LOOP_DETECT_INTERVAL	30 	/*30s*/
#define RAETH_MII_READ			0x89F3

/* Maximum size of the ESSID and NICKN strings */
#define IW_ESSID_MAX_SIZE	32

#define MAX_ETH_IF_CNT		10

#define MAX(a, b) ((a > b) ? (a) : (b))
#define MIN(a, b) ((a < b) ? (a) : (b))

#define TIMER_VALID(t) 		((t.tv_sec) && (t.tv_usec))
#define TIME_AFTER(a, b)	((long)((b) - (a)) <= 0)
#define TIMER_ClEAN(t, s)	memset(t, 0, s)

#define MAC_ADDR_EQUAL(addr1, addr2) (!memcmp(addr1, addr2, MAC_ADDR_LEN))
#define STR_EQUAL(str1, str2)	(!strcmp(str1, str2))
#define os_snprintf_error(size, res) ((res < 0) || (res >= (size)))

/* Paths */
#ifndef IW_RESTRIC_ENUM
#define PROC_NET_PATH		"/proc/net/dev"
#else
#define PROC_NET_PATH		"/proc/net/wireless"
#endif

/* Debug Level */
#define DBG_LVL_OFF	0
#define DBG_LVL_ERROR	1
#define DBG_LVL_WARN	2
#define DBG_LVL_TRACE	3
#define DBG_LVL_INFO	4

int debug_level = DBG_LVL_ERROR;

#define DBGPRINT(Level, fmt, args...)			\
do {							\
	if (Level <= debug_level) {			\
		printf("[fwdd][%s]", __func__);		\
		printf(fmt, ## args);			\
	}						\
} while (0)

enum nl_msg_id {
	FWD_CMD_ADD_LINK = 1,
	FWD_CMD_DEL_LINK,

	FWD_CMD_ADD_TX_SRC = 3,
	FWD_CMD_DEL_TX_SRC,

	FWD_CMD_ADD_PATH_INFO,
	FWD_CMD_DEL_PATH_INFO,

	FWD_CMD_ADD_SESSION_ENTRY,
	FWD_CMD_DEL_SESSION_ENTRY,

	FWD_CMD_ADD_ETH_IF,

	FWD_CMD_MAX
};

enum interface_type {
	INTF_TYPE_UNKONW = 0,
	INTF_TYPE_AP,
	INTF_TYPE_APCLI,
	INTF_TYPE_ETH,
	INTF_TYPE_BRIDGE,
	INTF_TYPE_MAX
};

enum link_status {
	LINK_UNKNOWN = 0,
	LINK_UP,
	LINK_DOWN,
};

enum eth_traffic_band {
	BAND_2G = 2,
	BAND_5G = 5,
	BAND_UNKNOWN = 0xFF,
};

struct wifi_link{
	char name[IFNAMSIZ];
	char blk_mc;		/*mc pkt 0:not block, 1: block*/
};

struct mac_addr {
	unsigned char mac[MAC_ADDR_LEN];
};

struct session_key {
	unsigned int s_ip;
	unsigned int d_ip;
	union {
		unsigned short ports[2];
		unsigned int port;
	};
	unsigned char proto;
};

struct session_ctrl {
	struct session_key key;
	char d_if[IFNAMSIZ];
};

struct fwd_path {
	char s_if[IFNAMSIZ];
	char s_lk_stat;	/*0:unknown, 1:linkup, 2:link down*/
	char d_if[IFNAMSIZ];
	char d_lk_stat;	/*0:unknown, 1:linkup, 2:link down*/
};

struct fwd_path_tbl {
	unsigned char count;
	struct fwd_path p[MAX_TBL_SIZE];
};

struct network_link {
	char ifname[IFNAMSIZ];
	unsigned char mac[MAC_ADDR_LEN];
	char if_type;
	char state;		/*0:unknown, 1:linkup, 2:link down*/
	char blk_mc;		/*mc pkt 0:not block, 1: block*/
	int  magic;		/*magic number in bc pkt*/
	struct timeval timer;	/*time to send bc pkt for loop check*/
	char band;
};
struct network_link_tbl {
	int count;
	int triggered_link_cnt;
	struct network_link lk[MAX_LINK_NUM];
};

struct network_dev {
	char ifname[IFNAMSIZ];
	unsigned char mac[MAC_ADDR_LEN];
	char type;		/*2:IW_MODE_INFRA, 3:IW_MODE_MASTER*/
	char state;		/*0:unknown, 1:linkup, 2:link down*/
	char band;
};

struct	parse_msg
{
	unsigned short length;		/* number of fields or size in bytes */
	unsigned short flags;		/* Optional params */
};

struct ra_mii_ioctl_data {
	unsigned int  phy_id;
	unsigned int  reg_num;
	unsigned int  val_in;
	unsigned int  val_out;
	unsigned int  port_num;
	unsigned int  dev_addr;
	unsigned int  reg_addr;
};

struct ether_interface {
	char ifname[IFNAMSIZ];
	char lk_stat;	/*0:unknown, 1:link up, 2:link down*/
	char prefer_band;
	char notified;
};

/*------------------------------------------------------------------*/
/*-------------------------Global variables-------------------------*/
struct fwd_path_tbl	path_tbl;
struct network_link_tbl link_tbl;
struct network_dev	ap_tbl[MAX_MBSS_NUM];
struct network_dev 	dev_list[MAX_TBL_SIZE*2];
int			dev_cnt = 0, ap_cnt = 0;
char 			br_name[IFNAMSIZ] = {0};
char 			br_mac[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

int			ethintf_cnt;
struct ether_interface	ethintf[MAX_ETH_IF_CNT] = {0};


int daemon_terminated = 0;

int sock_send_nl_msg = -1;
int sock_send_bc = 0;
int sock_get_wf_lk = 0;
int sock_get_eth_lk = 0;
int sock_intf_event = 0;
int sock_rcv_bc = 0;

char zero_mac[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*----------------------Function declaration------------------------*/
int update_path_tbl(char *ifname, char lk_stat);
int update_link_tbl(char *ifname, char lk_stat, char if_type);
int update_mbss_tbl(char *ifname, char lk_stat, char if_type);
int send_ctrl_msg(char msgid, char *msg_data);
int getmacaddr(char* ifname, unsigned char *mac);
void add_path_table(char *s_ifname, char *d_ifname);
int get_ether_lk_status(char *ifname);


/*----------------------loop detect API-----------------------------*/
#if LOOP_DETECT_IN_DAEMON
void send_bc_pkt(struct network_link *lk)
{
	int status, addrlen;
	struct sockaddr_in sock_addr;
	char buf[100] = {0};

	struct ifreq netif;
	int buf_len = 0;
	int ret;

	struct timeval tpstart;

	addrlen = sizeof(struct sockaddr_in);
	memset(&sock_addr, 0, addrlen);

	memset(&netif, 0, sizeof(netif));
	memcpy(netif.ifr_name, lk->ifname, strlen(lk->ifname));

	ret = setsockopt(sock_send_bc, SOL_SOCKET, SO_BINDTODEVICE, (char*)&netif, sizeof(netif));
	if (ret < 0)
		DBGPRINT(DBG_LVL_ERROR, "setsockopt SO_BINDTODEVICE failed, ret=%d\n", ret);

	/* -1 = 255.255.255.255 this is a BROADCAST address,
	 a local broadcast address could also be used.
	 you can comput the local broadcat using NIC address and its NETMASK
	*/
	sock_addr.sin_addr.s_addr = htonl(-1); /* send message to 255.255.255.255 */
	sock_addr.sin_port = htons(PORT1); /* port number */
	sock_addr.sin_family = AF_INET;


	gettimeofday(&tpstart,NULL);
	srand(tpstart.tv_usec);
	lk->magic= (1+(int) (300.0*rand()/(RAND_MAX+1.0)));

	memset(buf, 0, 100);
	memcpy(&buf[buf_len], MAGIC, MG_LEN);
	buf_len += MG_LEN;
	memcpy(&buf[buf_len], (char *)&lk->magic, sizeof(lk->magic));
	buf_len += sizeof(lk->magic);
	memcpy(&buf[buf_len], (char *)&tpstart, sizeof(tpstart));
	buf_len += sizeof(tpstart);

	DBGPRINT(DBG_LVL_TRACE, "send_bc_pkt:%s, magic:%x\n", lk->ifname,  lk->magic);

	status = sendto(sock_send_bc, buf, buf_len, 0, (struct sockaddr *)&sock_addr, addrlen);
}

int is_loop_detect_triggered()
{
	return link_tbl.triggered_link_cnt;
}

void send_bc_on_triggered_link()
{
	int i = 0;
	struct timeval now;
	struct wifi_link lk;
	int d_t;

	gettimeofday(&now, NULL);

	for (i = 0; i < link_tbl.count; i++) {
		if (link_tbl.lk[i].if_type != INTF_TYPE_APCLI || !TIMER_VALID(link_tbl.lk[i].timer))
			continue;

		d_t = link_tbl.lk[i].timer.tv_sec - now.tv_sec;

		if (0 <= d_t && d_t < sec_5s) {
			memset(&lk, 0, sizeof(lk));
			memcpy(lk.name, link_tbl.lk[i].ifname, IFNAMSIZ);

			if(link_tbl.lk[i].blk_mc) {
				send_ctrl_msg(FWD_CMD_ADD_LINK, (char *)&lk);
				link_tbl.lk[i].blk_mc = 0;
			}
			send_bc_pkt(&link_tbl.lk[i]);
		} else if (d_t < 0){
			TIMER_ClEAN(&link_tbl.lk[i].timer, sizeof(struct timeval));
			link_tbl.triggered_link_cnt = MAX(link_tbl.triggered_link_cnt--, 0);
		}
	}
}


/*if loop detect, speed to detect next link.*/
void speed_loop_detect()
{
	int i = 0;
	struct timeval now;
	int d_t;

	gettimeofday(&now, NULL);
	for (i = 0; i < link_tbl.count; i++) {
		if (link_tbl.lk[i].if_type != INTF_TYPE_APCLI || !TIMER_VALID(link_tbl.lk[i].timer))
			continue;

		link_tbl.lk[i].timer.tv_sec -= sec_5s;

		if (TIME_AFTER(now.tv_sec, link_tbl.lk[i].timer.tv_sec)){
			TIMER_ClEAN(&link_tbl.lk[i].timer, sizeof(struct timeval));
			link_tbl.triggered_link_cnt = MAX(link_tbl.triggered_link_cnt--, 0);
		}
	}
}

/* this function is to detect loop & block mc on loop link.
 * mc pkt will be sent on all wifi link in interval of 5s,
 * it will be call on below two cases:
 * 1. mc link is down, start timer to find an existed link for mc pkt,
 * 2. eth link is up, start timer to check if loop was formed on other wifi link.
 */
void trigger_loop_detection()
{
	int i = 0;
	struct timeval now;

	gettimeofday(&now,NULL);

	link_tbl.triggered_link_cnt = 0;
	for (i = 0; i < link_tbl.count; i++) {
		if (link_tbl.lk[i].state == LINK_UP && link_tbl.lk[i].if_type == INTF_TYPE_APCLI) {
			link_tbl.lk[i].timer.tv_sec = now.tv_sec + (i+1)*sec_5s;
			link_tbl.lk[i].timer.tv_usec = now.tv_usec;
			link_tbl.triggered_link_cnt++;
		}
	}
}

void recv_bc_pkt(char *buf)
{
	int i = 0, is_loop = 0;
	int magic = *((int *)&buf[0]);

	for (i = 0; i < link_tbl.count; i++) {
		if (link_tbl.lk[i].state == LINK_UP && link_tbl.lk[i].magic == magic) {
			struct timeval now, *tm;
			int d_t;

			gettimeofday(&now,NULL);

			tm = (struct timeval*)&buf[sizeof(link_tbl.lk[i].magic)];
			d_t = (now.tv_sec - tm->tv_sec)*1000*1000 + (now.tv_usec - tm->tv_usec);

			/*receive self bc pkt in 5s, block tx/rx bc pkt in the inferface.*/
			if (0 <= d_t && d_t < usec_5s) {
				struct wifi_link lk;

				DBGPRINT(DBG_LVL_ERROR, "loop detected.\n");
				lk.blk_mc = 1;
				memcpy(lk.name, link_tbl.lk[i].ifname, IFNAMSIZ);
				send_ctrl_msg(FWD_CMD_ADD_LINK, (char *)&lk);
				link_tbl.lk[i].blk_mc = 1;
				link_tbl.lk[i].magic= 0;

				is_loop = 1;

			}
			break;
		}
	}
	if (is_loop && is_loop_detect_triggered)
		speed_loop_detect();
}
#endif

/*------------------------------------------------------------------*/
int create_sock()
{
	int ret;
	int bc = 1;
	struct sockaddr_nl addr_nl_msg;
	struct sockaddr_nl addr_intf_event;
#if LOOP_DETECT_IN_DAEMON
	struct sockaddr_in addr_rcv_bc;
#endif
	/*1. Create socket for send netlink msg*/
	sock_send_nl_msg = socket(AF_NETLINK, SOCK_RAW, NETLINK_EXT);
	if (sock_send_nl_msg == -1) {
		DBGPRINT(DBG_LVL_ERROR,
			"create sock_send_nl_msg socket error: %s\n", strerror(errno));
		return -1;
	}

	/*To prepare binding*/
	memset(&addr_nl_msg, 0, sizeof(addr_nl_msg));
	addr_nl_msg.nl_family = AF_NETLINK;
	addr_nl_msg.nl_pid = 100;
	addr_nl_msg.nl_groups = 0;

	/*Bind*/
	ret = bind(sock_send_nl_msg, (struct sockaddr *)&addr_nl_msg, sizeof(addr_nl_msg));
	if (ret < 0) {
		DBGPRINT(DBG_LVL_ERROR, "sock_send_nl_msg bind failed: %s\n", strerror(errno));
		close(sock_send_nl_msg);
		return -1;
	}

	/*2. Create socket for send broadcast msg*/
	sock_send_bc = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock_send_bc == -1) {
		DBGPRINT(DBG_LVL_ERROR, "create sock_send_bc socket error: %s\n", strerror(errno));
		return -1;
	}
	ret = setsockopt(sock_send_bc, SOL_SOCKET, SO_BROADCAST, &bc, sizeof(bc) );
	if (ret < 0) {
		DBGPRINT(DBG_LVL_ERROR, "sock_send_bc:setsockopt SO_BROADCAST failed, %s\n", strerror(errno));
		return -1;
	}

	/*3. Create socket for get wifi link status*/
	sock_get_wf_lk = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_get_wf_lk == -1) {
		DBGPRINT(DBG_LVL_ERROR, "create sock_get_wf_lk socket error: %s\n", strerror(errno));
		return -1;
	}

	/*4. Create socket for get ether link status*/
	sock_get_eth_lk = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_get_eth_lk < 0) {
		DBGPRINT(DBG_LVL_ERROR, "create sock_get_eth_lk socket error: %s\n", strerror(errno));
		return -1;
	}

	/*5. Create socket for interface state change msg*/
	sock_intf_event = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if(sock_intf_event == -1) {
		DBGPRINT(DBG_LVL_ERROR, "create sock_intf_event socket error: %s\n", strerror(errno));
		return -1;
	}
	bzero((char*) &addr_intf_event, sizeof(addr_intf_event));
	addr_intf_event.nl_family = AF_NETLINK;
	addr_intf_event.nl_groups = RTNLGRP_LINK;
	ret = bind(sock_intf_event, (struct sockaddr*)&addr_intf_event, sizeof(addr_intf_event));
	if(ret == -1) {
		perror("Error: bind sock_intf_event failed\n");
		return -1;
	}

#if LOOP_DETECT_IN_DAEMON
	/*6. Create socket for recv broadcast pkt*/
	sock_rcv_bc = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock_rcv_bc == -1) {
		DBGPRINT(DBG_LVL_ERROR, "create sock_rcv_bc socket error: %s\n", strerror(errno));
		return -1;
	}
	ret = setsockopt(sock_rcv_bc, SOL_SOCKET, SO_BROADCAST, &bc, sizeof(bc));
	if (ret < 0) {
		DBGPRINT(DBG_LVL_ERROR, "sock_rcv_bc:setsockopt SO_BROADCAST failed, %s\n", strerror(errno));
		return -1;
	}
	ret = setsockopt(sock_rcv_bc, SOL_SOCKET, SO_BINDTODEVICE, (char*)br_name, strlen(br_name));
	if (ret < 0) {
		DBGPRINT(DBG_LVL_ERROR, "sock_rcv_bc:setsockopt SO_BINDTODEVICE failed, %s\n", strerror(errno));
		return -1;
	}
	bzero((char*) &addr_rcv_bc, sizeof(addr_rcv_bc));
	/*Fill in sockaddr_in*/
	addr_rcv_bc.sin_family = AF_INET;
	addr_rcv_bc.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_rcv_bc.sin_port = htons(PORT1);

	/*Bind socket and listen for incoming bc pkt*/
	ret = bind(sock_rcv_bc, (struct sockaddr *) &addr_rcv_bc, sizeof(struct sockaddr));
	if(ret == -1) {
		perror("Error: bind sock_rcv_bc failed\n");
		return -1;
	}
#endif

	return 0;
}

void close_sock()
{
	close(sock_send_nl_msg);
	close(sock_send_bc);
	close(sock_get_wf_lk);
	close(sock_get_eth_lk);
	close(sock_intf_event);
#if LOOP_DETECT_IN_DAEMON
	close(sock_rcv_bc);
#endif
}

/*
 * get interface name from PROC_NET_PATH.
 */
static inline char *get_if_name(char *ifname, int size, char *buf)
{
	char *pos;

	/* strip spaces */
	while(isspace(*buf))
		buf++;

#ifndef IW_RESTRIC_ENUM
	pos = strrchr(buf, ':');
#else
	pos = strstr(buf, ": ");
#endif

	if((pos == NULL) || (((pos - buf) + 1) > size))
		return(NULL);

	memcpy(ifname, buf, (pos - buf));
	ifname[pos - buf] = '\0';

	return(pos);
}

static inline int io_get_cmd(int skfd, const char *ifname, int request, struct iwreq * pwrq)
{
	if (!ifname || !pwrq)
		return -1;

	memset(pwrq->ifr_name, 0, sizeof(pwrq->ifr_name));
	strncpy(pwrq->ifr_name, ifname, MIN((sizeof(pwrq->ifr_name)-1), strlen(ifname)));
	return (ioctl(skfd, request, pwrq));
}

static inline char *lks2str(char s)
{
	switch (s) {
		case LINK_DOWN:
			return "DOWN";
		case LINK_UP:
			return "UP";
		default:
			return "UNKNOWN";
	}
}

static inline char *lkt2str(char s)
{
	switch (s) {
		case INTF_TYPE_AP:
			return "ap";
		case INTF_TYPE_APCLI:
			return "apcli";
		case INTF_TYPE_ETH:
			return "eth";
		default:
			return "unknow";
	}
}

int getband(char *ifname)
{
	int channel;
	struct iwreq  wrq;

	if(io_get_cmd(sock_get_wf_lk, ifname, SIOCGIWFREQ, &wrq) < 0)
		return -1;

	channel = wrq.u.freq.m;
	if (0 < channel && channel <= 14)
		return BAND_2G;
	else if (channel > 14)
		return BAND_5G;
	else
		return BAND_UNKNOWN;
}

void wifi_lks_polling()
{
	int i = 0;
	char		buff[1024], *ret = NULL;
	FILE *		fh;
	char		essid[IW_ESSID_MAX_SIZE + 1];   /* ESSID (extended network) */

	struct iwreq	      wrq;

	memset(dev_list, 0, sizeof(dev_list));

	/* Check if /proc/net/dev or /proc/net/wireless is available */
	fh = fopen(PROC_NET_PATH, "r");

	if (fh != NULL) {
		/* Eat 2 lines of header */
		ret = fgets(buff, sizeof(buff), fh);
		if (!ret)
			DBGPRINT(DBG_LVL_ERROR, "%s(%d), fgets return NULL.\n", __func__, __LINE__);

		ret = fgets(buff, sizeof(buff), fh);
		if (!ret)
			DBGPRINT(DBG_LVL_ERROR, "%s(%d), fgets return NULL.\n", __func__, __LINE__);

		dev_cnt = 0;
		/* Read each device line */
		while (fgets(buff, sizeof(buff), fh)) {
			char	name[IFNAMSIZ + 1];
			char *s;

			/* Skip empty or almost empty lines. It seems that in some
			 * cases fgets return a line with only a newline. */
			if((buff[0] == '\0') || (buff[1] == '\0'))
				continue;

			memset(name, 0, sizeof(name));
			/* Extract interface name */
			s = get_if_name(name, sizeof(name), buff);

			/* eth didn't support the ioctl , maybe eth turn off the not support msg */
			if (strlen(name) && !strncmp("eth", name, 3))
				continue;

			if (s) {
				/* Get operation mode */
				if (io_get_cmd(sock_get_wf_lk, name, SIOCGIWMODE, &wrq) >= 0) {
					if (wrq.u.mode == IW_MODE_MASTER)
						dev_list[dev_cnt].type = INTF_TYPE_AP;
					else if (wrq.u.mode == IW_MODE_INFRA)
						dev_list[dev_cnt].type = INTF_TYPE_APCLI;
					else
						dev_list[dev_cnt].type = INTF_TYPE_UNKONW;
				} else
					continue;

				/* Get ESSID */
				memset(essid, 0, sizeof(essid));
				wrq.u.essid.pointer = (caddr_t)essid;
				wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
				wrq.u.essid.flags = 0;
				dev_list[dev_cnt].state = LINK_DOWN;
				if (io_get_cmd(sock_get_wf_lk, name, SIOCGIWESSID, &wrq) >= 0) {
					if (strlen(essid) > 0)
						dev_list[dev_cnt].state = LINK_UP;
				} else
					continue;

				memcpy(dev_list[dev_cnt].ifname, name, MIN(strlen(name), IFNAMSIZ));
			}
			dev_cnt++;
			if (dev_cnt >= MAX_TBL_SIZE*2) {
				DBGPRINT(DBG_LVL_ERROR, "Error: dev_list[%d] Out of bound.\n", dev_cnt);
				break;
			}
		}
		if (fclose(fh))
			DBGPRINT(DBG_LVL_ERROR, "%s(), fclose get error %s\n", __func__, strerror(errno));
	}

	for (i = 0; i < dev_cnt; i++) {
		if (dev_list[i].type == INTF_TYPE_AP) {
			update_path_tbl((char*)dev_list[i].ifname, dev_list[i].state);
			update_mbss_tbl((char*)dev_list[i].ifname, dev_list[i].state, INTF_TYPE_AP);
		}
		else if (dev_list[i].type == INTF_TYPE_APCLI) {
			update_link_tbl((char*)dev_list[i].ifname, dev_list[i].state, INTF_TYPE_APCLI);
			update_path_tbl((char*)dev_list[i].ifname, dev_list[i].state);
		}
	}
}

/* mii_mgr -g -p 0 -r 1 */
int get_ether_lk_status(char *ifname)
{
	return LINK_UP; /*Always on to fixed 7629 can't detect eth link status*/

#if 0
	int ret;
	struct ifreq ifr;
	struct ra_mii_ioctl_data mii;
	int method = 0;
	__u32 p = 0;
	char lk_s = LINK_UNKNOWN;

	if(strlen(ifname) <= 0) {
		DBGPRINT(DBG_LVL_ERROR, "get ether lk:Invalid ether_name.\n");
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_data = &mii;

	mii.reg_num = 1;
	method = RAETH_MII_READ;
	for (p = 0; p < 5; p++) {
		mii.phy_id = p;
		ret = ioctl(sock_get_eth_lk, method, &ifr);
		if (ret < 0) {
			DBGPRINT(DBG_LVL_ERROR, "get ether lk: ioctl error, ret=%d\n", ret);
			break;
		}
		if (mii.val_out & 0x00000004) {
			lk_s = LINK_UP;
			break;
		}else
			lk_s = LINK_DOWN;
	}
	return lk_s;
#endif
}

void eth_lks_polling()
{
	int i = 0;
	static int flag = -1;

	for (i = 0; i < ethintf_cnt; i++) {
		if (strlen(ethintf[i].ifname)) {
			ethintf[i].lk_stat = get_ether_lk_status(ethintf[i].ifname);

			if (!ethintf[i].notified) {
				flag = send_ctrl_msg(FWD_CMD_ADD_ETH_IF, ethintf[i].ifname);
				ethintf[i].notified = (flag < 0) ? 0 : 1;
			}
		}
	}
}

char *buildmsg(char msgid, char *msg_data, char *buff)
{
	if (msgid >= FWD_CMD_MAX) {
		DBGPRINT(DBG_LVL_ERROR, "Invalid msgid: %d\n", msgid);
		return NULL;
	}

	buff[0] = msgid;
	switch (msgid) {
		case FWD_CMD_ADD_LINK:
		case FWD_CMD_DEL_LINK:
		{
			struct wifi_link *lk = (struct wifi_link *)&buff[1];
			memcpy(lk, msg_data, sizeof(*lk));
			break;
		}
		case FWD_CMD_ADD_TX_SRC:
		case FWD_CMD_DEL_TX_SRC:
		{
			struct mac_addr *m = (struct mac_addr *)&buff[1];
			memcpy(m, msg_data, sizeof(*m));
			break;
		}
		case FWD_CMD_ADD_SESSION_ENTRY:
		case FWD_CMD_DEL_SESSION_ENTRY:
		{
			struct session_ctrl *s = (struct session_ctrl *)&buff[1];
			memcpy(s, msg_data, sizeof(*s));
			break;
		}
		case FWD_CMD_ADD_PATH_INFO:
		case FWD_CMD_DEL_PATH_INFO:
		{
			struct fwd_path *p = (struct fwd_path *)&buff[1];
			memcpy(p, msg_data, sizeof(*p));
			break;
		}
		case FWD_CMD_ADD_ETH_IF:
		{
			memcpy(&buff[1], msg_data, IFNAMSIZ);
			break;
		}
	}

	return buff;
}

int send_ctrl_msg(char msgid, char *msg_data)
{
	struct sockaddr_nl dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	struct msghdr msg;
	int state_smg = 0;
	char tmp[1024] = {0};

	if (sock_send_nl_msg < 0) {
		DBGPRINT(DBG_LVL_ERROR, "net link socket didn't created.\n");
		return -1;
	}

	/* To prepare create mssage */
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	if (!nlh) {
		DBGPRINT(DBG_LVL_ERROR, "malloc nlmsghdr error!\n");
		return -1;
	}

	memset(&dest_addr,0,sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;
	dest_addr.nl_groups = 0;
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = 100;
	nlh->nlmsg_flags = 0;
	if (!buildmsg(msgid, msg_data, tmp)) {
		DBGPRINT(DBG_LVL_ERROR, "%s(), build message failed.\n", __func__);
		free(nlh);
		return -1;
	}

	memcpy(NLMSG_DATA(nlh), tmp, 1024);
	iov.iov_base = (void *)nlh;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);

	/*Create mssage*/
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/*send message*/
	state_smg = sendmsg(sock_send_nl_msg, &msg, 0);
	if (state_smg == -1) {
		DBGPRINT(DBG_LVL_ERROR, "get error sendmsg = %s\n", strerror(errno));
		free(nlh);
		return -1;
	}

	free(nlh);
	return 0;
}

void update_device_mac()
{
	int i;
	struct mac_addr m;

	for (i = 0; i < ap_cnt; i++) {
		memset(&m, 0, sizeof(m));
		getmacaddr(ap_tbl[i].ifname, m.mac);
		if (!MAC_ADDR_EQUAL(m.mac, zero_mac) &&
			!MAC_ADDR_EQUAL(ap_tbl[i].mac, m.mac)) {
			if (send_ctrl_msg(FWD_CMD_ADD_TX_SRC, (char *)&m) == 0)
				memcpy(ap_tbl[i].mac, m.mac, MAC_ADDR_LEN);
		}
	}

	for (i = 0; i < link_tbl.count; i++) {
		memset(&m, 0, sizeof(m));
		getmacaddr(link_tbl.lk[i].ifname, m.mac);
		if (!MAC_ADDR_EQUAL(m.mac, zero_mac) &&
			!MAC_ADDR_EQUAL(link_tbl.lk[i].mac, m.mac)) {
			if (send_ctrl_msg(FWD_CMD_ADD_TX_SRC, (char *)&m) == 0)
				memcpy(link_tbl.lk[i].mac, m.mac, MAC_ADDR_LEN);
		}
	}
}

void update_bridge_mac()
{
	struct mac_addr m;

	if (strlen(br_name) > 0) {
		memset(&m, 0, sizeof(m));
		getmacaddr(br_name, m.mac);
		if (!MAC_ADDR_EQUAL(m.mac, zero_mac) && !MAC_ADDR_EQUAL(br_mac, m.mac)) {
			if (send_ctrl_msg(FWD_CMD_ADD_TX_SRC, (char *)&m) == 0) {
				memcpy(br_mac, m.mac, MAC_ADDR_LEN);
				DBGPRINT(DBG_LVL_TRACE, "update_bridge_mac: %s mac:%x:%x:%x:%x:%x:%x\n",
					br_name, m.mac[0], m.mac[1], m.mac[2], m.mac[3], m.mac[4], m.mac[5]);
			}
		}
	}
}

void usage()
{
	int ret = 0, offset = 0;
	char buf[512] = {0};

	memset(buf, 0, sizeof(buf));
	ret = snprintf(buf+offset, sizeof(buf)-offset, "%s\n", "Usage:");
	if (os_snprintf_error(sizeof(buf)-offset, ret))
		goto error;
	offset += ret;
	ret = snprintf(buf+offset, sizeof(buf)-offset, "%s\n",
		"fwdd [-d] <debug level> [-e] <ethernet interface> <traffic prefer band>");
	if (os_snprintf_error(sizeof(buf)-offset, ret))
		goto error;
	offset += ret;
	ret = snprintf(buf+offset, sizeof(buf)-offset, "%s\n",
		"   - <debug level> is a number between 0~3.");
	if (os_snprintf_error(sizeof(buf)-offset, ret))
		goto error;
	offset += ret;
	ret = snprintf(buf+offset, sizeof(buf)-offset, "%s\n",
		"   - <ethernet interface> is ethernet interface name, such as: eth0/lan0~lan3");
	if (os_snprintf_error(sizeof(buf)-offset, ret))
		goto error;
	offset += ret;
	ret = snprintf(buf+offset, sizeof(buf)-offset, "%s\n",
		"   - <traffic prefer band> ethernet traffic preferred band in DBDC mode, such as 2G/5G");
	if (os_snprintf_error(sizeof(buf)-offset, ret))
		goto error;
	offset += ret;
	ret = snprintf(buf+offset, sizeof(buf)-offset, "%s\n",
		"   example: fwdd -e eth0 5G -e lan0 5G -e lan1 5G -e lan2 5G -e lan3 5G&");
	if (os_snprintf_error(sizeof(buf)-offset, ret))
		goto error;
	offset += ret;

	DBGPRINT(DBG_LVL_ERROR, "%s\n", buf);
error:
	DBGPRINT(DBG_LVL_ERROR, "%s(), error happen, ret:%d\n", __func__, ret);
}

int parseparam(int argc, char **argv)
{
	int opt = 0;
	char options[] = "p:e:b:c:d:h:?";
	char *cvalue = NULL;

	if (argc <= 1)
		return -1;

	ethintf_cnt = 0;
	memset(&ap_tbl, 0, sizeof(ap_tbl));
	memset(&path_tbl, 0, sizeof(path_tbl));
	memset(&link_tbl, 0, sizeof(link_tbl));
	memset(ethintf, 0, sizeof(ethintf));

	while ((opt = getopt(argc, argv, options)) != -1) {
		switch (opt) {
			case 'p':
				add_path_table(optarg, argv[optind]);
				update_link_tbl(argv[optind], LINK_UNKNOWN, INTF_TYPE_APCLI);
				optind++;
				break;
			case 'e':
				if (ethintf_cnt >= MAX_ETH_IF_CNT) {
					DBGPRINT(DBG_LVL_ERROR,
						"ethernet interface count exceed %d.\n", MAX_ETH_IF_CNT);
					break;
				}

				memcpy(ethintf[ethintf_cnt].ifname, optarg, MIN(strlen(optarg), IFNAMSIZ));

				if(STR_EQUAL(argv[optind],"2G") || STR_EQUAL(argv[optind],"2g")) {
					ethintf[ethintf_cnt].prefer_band = BAND_2G;
					optind++;
				} else if (STR_EQUAL(argv[optind],"5G") || STR_EQUAL(argv[optind],"5g")) {
					ethintf[ethintf_cnt].prefer_band = BAND_5G;
					optind++;
				} else
					ethintf[ethintf_cnt].prefer_band = BAND_5G;

				DBGPRINT(DBG_LVL_TRACE,	"ethernet %s -> prefer_band:%dG\n",
					ethintf[ethintf_cnt].ifname, ethintf[ethintf_cnt].prefer_band);

				ethintf_cnt++;

				break;

			case 'b':
				memset(br_name, 0, sizeof(br_name));
				memcpy(br_name, optarg, MIN(strlen(optarg), IFNAMSIZ));
				break;

			case 'c':
				update_link_tbl(optarg, LINK_UNKNOWN, INTF_TYPE_APCLI);
				break;

			case 'd':
				cvalue = optarg;
				if (STR_EQUAL(cvalue, "0"))
					debug_level = DBG_LVL_OFF;
				else if (STR_EQUAL(cvalue, "1"))
					debug_level = DBG_LVL_ERROR;
				else if (STR_EQUAL(cvalue, "2"))
					debug_level = DBG_LVL_WARN;
				else if (STR_EQUAL(cvalue, "3"))
					debug_level = DBG_LVL_TRACE;
				else if (STR_EQUAL(cvalue, "4"))
					debug_level = DBG_LVL_INFO;
				else {
					DBGPRINT(DBG_LVL_ERROR,
						"-d option does not have this debuglevel %s\n", cvalue);
					return -1;
				}
				DBGPRINT(DBG_LVL_ERROR,	" debug_level is: %d\n", debug_level);
				break;

			case 'h':
			case '?':
				usage();
				return -1;
		}
	}

	return 0;
}

void add_path_table(char *s_ifname, char *d_ifname)
{
	int i = 0;

	if (!strlen(s_ifname))
		return;

	for (i = 0; i < path_tbl.count; i++) {
		if (STR_EQUAL(path_tbl.p[i].s_if, s_ifname) &&
			STR_EQUAL(path_tbl.p[i].d_if, d_ifname))
			return;
		else if (STR_EQUAL(path_tbl.p[i].s_if, s_ifname) &&
			!STR_EQUAL(path_tbl.p[i].d_if, d_ifname)) {
			DBGPRINT(DBG_LVL_TRACE, "update path_tbl[%d]: from %s --> %s to %s --> %s\n",
			i, s_ifname, path_tbl.p[i].d_if, s_ifname, d_ifname);
			memcpy(path_tbl.p[i].d_if, d_ifname, MIN(strlen(d_ifname), IFNAMSIZ));
			return;
		}
	}
	memcpy(path_tbl.p[path_tbl.count].s_if, s_ifname, MIN(strlen(s_ifname), IFNAMSIZ));
	memcpy(path_tbl.p[path_tbl.count].d_if, d_ifname, MIN(strlen(d_ifname), IFNAMSIZ));
	DBGPRINT(DBG_LVL_TRACE, "add path_tbl[%d]: %s --> %s\n", path_tbl.count, s_ifname, d_ifname);
	path_tbl.count++;
}

int update_path_tbl(char *ifname, char lk_stat)
{
	int i = 0;
	int lk_changed = 0;

	for(i = 0; i < path_tbl.count; i++) {
		if (STR_EQUAL(path_tbl.p[i].s_if, ifname)) {
			if(path_tbl.p[i].s_lk_stat != lk_stat){
				path_tbl.p[i].s_lk_stat = lk_stat;
				lk_changed = 1;
			}
		}
		else if(STR_EQUAL(path_tbl.p[i].d_if, ifname)) {
			if(path_tbl.p[i].d_lk_stat != lk_stat){
				path_tbl.p[i].d_lk_stat = lk_stat;
				lk_changed = 1;
			}
		}
		if (lk_changed) {
			if ((path_tbl.p[i].s_lk_stat == LINK_UP && path_tbl.p[i].d_lk_stat == LINK_UP))
				send_ctrl_msg(FWD_CMD_ADD_PATH_INFO, (char *)&path_tbl.p[i]);
			else if(path_tbl.p[i].d_lk_stat == LINK_DOWN)
				send_ctrl_msg(FWD_CMD_DEL_PATH_INFO, (char *)&path_tbl.p[i]);

			lk_changed = 0;
		}
	}
	return lk_changed;
}

void clean_fwd_tbl()
{
	int i = 0;
	struct wifi_link lk;

	for(i = 0; i < path_tbl.count; i++) {
		path_tbl.p[i].d_lk_stat = LINK_DOWN;
		send_ctrl_msg(FWD_CMD_DEL_PATH_INFO, (char *)&path_tbl.p[i]);
	}

	for (i = 0; i < link_tbl.count; i++) {
		memset(&lk, 0, sizeof(lk));
		memcpy(lk.name, link_tbl.lk[i].ifname, IFNAMSIZ);
		send_ctrl_msg(FWD_CMD_DEL_LINK, (char *)&lk);
	}
}

char check_interface_type(char *ifname)
{
	int i = 0;
	char type = INTF_TYPE_UNKONW;

	for (i = 0; i < dev_cnt; i++) {
		if (STR_EQUAL(dev_list[i].ifname, ifname)) {
			type = dev_list[i].type;
			break;
		}
	}
	return type;
}

int getmacaddr(char* ifname, unsigned char *mac)
{
	struct sockaddr *s_add;
	struct ifreq        ifr;

	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock < 0) {
		perror("socket error!\n");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	memcpy(ifr.ifr_name, ifname, MIN(sizeof(ifr.ifr_name), strlen(ifname)));

	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
		/*perror("ioctl error\n");*/
		close(sock);
		return -1;
	} else {
		s_add = (struct sockaddr *)&(ifr.ifr_hwaddr);
		memcpy(mac, s_add->sa_data, MAC_ADDR_LEN);
	}
	close(sock);
	return 0;
}

int update_mbss_tbl(char *ifname, char lk_stat, char if_type)
{
	int i = 0, idx = 0;
	int lk_changed = 0;

	for (i = 0; i < MAX_MBSS_NUM; i++) {
		if (STR_EQUAL(ap_tbl[i].ifname, ifname)) {
			if (ap_tbl[i].state != lk_stat) {
				ap_tbl[i].state = lk_stat;
				ap_tbl[i].band = getband(ifname);
				lk_changed = 1;
				idx = i;
				if (if_type < INTF_TYPE_MAX && if_type != INTF_TYPE_UNKONW)
					ap_tbl[i].type = if_type;
			}
			break;
		}
	}

	if (i >= ap_cnt && ap_cnt < (MAX_MBSS_NUM-1)) {
		memcpy(ap_tbl[ap_cnt].ifname, ifname, MIN(strlen(ifname), IFNAMSIZ));
		ap_tbl[ap_cnt].type = if_type;
		ap_tbl[ap_cnt].state = lk_stat;
		ap_tbl[ap_cnt].band = getband(ifname);
		idx = ap_cnt;
		lk_changed = 1;
		ap_cnt++;
	}

	if (lk_changed && lk_stat == LINK_UP) {
		struct mac_addr m;

		memset(&m, 0, sizeof(m));
		getmacaddr(ap_tbl[idx].ifname, m.mac);
		if (!MAC_ADDR_EQUAL(m.mac, zero_mac) &&
			!MAC_ADDR_EQUAL(ap_tbl[idx].mac, m.mac)) {
			if (send_ctrl_msg(FWD_CMD_ADD_TX_SRC, (char *)&m) == 0)
				memcpy(ap_tbl[idx].mac, m.mac, MAC_ADDR_LEN);
		}
	}

	return 0;
}

int update_link_tbl(char *ifname, char lk_stat, char if_type)
{
	int i = 0, idx = 0;
	int lk_changed = 0;

	for (i = 0; i < link_tbl.count; i++) {
		if (STR_EQUAL(link_tbl.lk[i].ifname, ifname)) {
			if (link_tbl.lk[i].state != lk_stat) {
				link_tbl.lk[i].state = lk_stat;
				link_tbl.lk[i].band = getband(ifname);
				lk_changed = 1;
				idx = i;
				if (if_type < INTF_TYPE_MAX && if_type != INTF_TYPE_UNKONW)
					link_tbl.lk[i].if_type = if_type;
			}
			break;
		}
	}
	if (i >= link_tbl.count && link_tbl.count < (MAX_LINK_NUM-1)) {
		memcpy(link_tbl.lk[link_tbl.count].ifname, ifname, MIN(strlen(ifname), IFNAMSIZ));
		link_tbl.lk[link_tbl.count].if_type = if_type;
		link_tbl.lk[link_tbl.count].state = lk_stat;
		link_tbl.lk[link_tbl.count].band = getband(ifname);
		idx = link_tbl.count;
		lk_changed = 1;

		link_tbl.count++;
	}
	if (lk_changed) {
		struct wifi_link lk;
		struct mac_addr m;

		memset(&lk, 0, sizeof(lk));
		memcpy(lk.name, ifname, MIN(strlen(ifname), IFNAMSIZ));

		memset(&m, 0, sizeof(m));
		getmacaddr(link_tbl.lk[idx].ifname, m.mac);
		if (!MAC_ADDR_EQUAL(m.mac, zero_mac) &&
			!MAC_ADDR_EQUAL(link_tbl.lk[idx].mac, m.mac)) {
			if (send_ctrl_msg(FWD_CMD_ADD_TX_SRC, (char *)&m) == 0)
				memcpy(link_tbl.lk[idx].mac, m.mac, MAC_ADDR_LEN);
		}

		if (lk_stat == LINK_UP) {
			send_ctrl_msg(FWD_CMD_ADD_LINK, (char *)&lk);
#if LOOP_DETECT_IN_DAEMON
			send_bc_pkt(&link_tbl.lk[idx]);
#endif
		} else if (lk_stat == LINK_DOWN) {
			send_ctrl_msg(FWD_CMD_DEL_LINK, (char *)&lk);
#if LOOP_DETECT_IN_DAEMON
			/*mc link was gone, set timer to find mc link.*/
			if (!link_tbl.lk[idx].blk_mc)
				trigger_loop_detection();
#endif
		}
		link_tbl.lk[idx].blk_mc = 0;
	}

	return lk_changed;
}

void handle_signal(int signo)
{
	DBGPRINT(DBG_LVL_ERROR, "handle_signal: terminated.\n");
	daemon_terminated = 1;
}

void get_link_ifname_by_band(char band, char *ifname, int len)
{
	int i = 0;

	if (ifname == NULL || len < 0)
		return;

	for (i = 0; i < link_tbl.count; i++) {
		if (link_tbl.lk[i].band == band) {
			memcpy(ifname, link_tbl.lk[i].ifname, MIN(len, strlen(link_tbl.lk[i].ifname)));
			break;
		}
	}
}

void fwd_path_auto_formation(void)
{
	static int cli_link_cnt;
	static int ap_dev_cnt;
	static int eth_dev_cnt;
	int flag = 0, i = 0;
	char cliifname[IFNAMSIZ] = {0};

	if (cli_link_cnt != link_tbl.count || ap_dev_cnt != ap_cnt || eth_dev_cnt != ethintf_cnt) {
		flag = 1;
		cli_link_cnt = link_tbl.count;
		ap_dev_cnt = ap_cnt;
		eth_dev_cnt = ethintf_cnt;
	}

	if (flag) {
		/* update traffic forward path on ap interface*/
		for (i = 0; i < ap_cnt; i++) {
			memset(cliifname, 0, sizeof(cliifname));
			get_link_ifname_by_band(ap_tbl[i].band, cliifname, sizeof(cliifname));
			if (strlen(cliifname))
				add_path_table(ap_tbl[i].ifname, cliifname);
		}
		/* update traffic forward path on ethernet interface*/
		for (i = 0; i < ethintf_cnt; i++) {
			memset(cliifname, 0, sizeof(cliifname));
			get_link_ifname_by_band(ethintf[i].prefer_band, cliifname, sizeof(cliifname));
			if (strlen(cliifname))
				add_path_table(ethintf[i].ifname, cliifname);
		}
	}
}

int main(int argc, char *argv[])
{
	int maxsock;
	int len;
	int ret = 0;
	char buf[BUFLEN] = {0};
	struct nlmsghdr *nh;
	struct ifinfomsg *ifinfo;
	struct rtattr *attr;
	fd_set fdsr;
	struct timeval tv;
	char *p = NULL;
	char link_stat = 0;
	struct timeval now, time1, time2, time3;
	struct parse_msg *rcv_msg;
	struct mac_addr m;

	if (parseparam(argc, argv) < 0) {
		usage();
		return -1;
	}

	if (create_sock() < 0)
		return -1;

	wifi_lks_polling();
	eth_lks_polling();

	update_device_mac();

	maxsock = MAX(sock_intf_event, sock_rcv_bc) + 1;

	if (signal(SIGTERM, handle_signal) == SIG_ERR) {
		DBGPRINT(DBG_LVL_ERROR, "init signal handler error.\n");
		goto exit;
	}

	gettimeofday(&time1, NULL);
	gettimeofday(&time2, NULL);
	gettimeofday(&time3, NULL);

	DBGPRINT(DBG_LVL_TRACE, "fwdd(ver:%s) started!!!\n", FWDD_VERSION);

	while (1) {
		if (daemon_terminated)
			break;

		FD_ZERO(&fdsr);
		FD_SET(sock_intf_event, &fdsr);
#if LOOP_DETECT_IN_DAEMON
		FD_SET(sock_rcv_bc, &fdsr);
#endif
		//timeout setting
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);
		if (ret < 0) {
			perror("select");
			break;
		}

		gettimeofday(&now, NULL);
		if (TIME_AFTER(now.tv_sec, time1.tv_sec)) {
			wifi_lks_polling();
			eth_lks_polling();
			update_bridge_mac();
			fwd_path_auto_formation();
			time1.tv_sec = now.tv_sec + LINK_POLL_INTERVAL;
		}

#if LOOP_DETECT_IN_DAEMON
		if (TIME_AFTER(now.tv_sec, time2.tv_sec)) {
			trigger_loop_detection();
			time2.tv_sec = now.tv_sec + LOOP_DETECT_INTERVAL;
		}

		if (TIME_AFTER(now.tv_sec, time3.tv_sec)) {
			if (is_loop_detect_triggered())
				send_bc_on_triggered_link();
			time3.tv_sec = now.tv_sec + sec_1s;
		}
#endif
		if (ret == 0) /*select timeout*/
			continue;

		memset(buf, 0, BUFLEN);
		if (FD_ISSET(sock_intf_event, &fdsr)) {
			static int r_len;
			struct iw_event *iwe;

			r_len = read(sock_intf_event, buf, BUFLEN);
			for (nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, r_len); nh = NLMSG_NEXT(nh, r_len)) {
				if (nh->nlmsg_type == NLMSG_DONE)
					break;
				else if (nh->nlmsg_type == NLMSG_ERROR)
					return -1;
				else if (nh->nlmsg_type != RTM_NEWLINK)
					continue;

				ifinfo = NLMSG_DATA(nh);
				link_stat = (ifinfo->ifi_flags & IFF_LOWER_UP) ? LINK_UP : LINK_DOWN;

				attr = (struct rtattr*)(((char*)nh) + NLMSG_SPACE(sizeof(*ifinfo)));
				len = nh->nlmsg_len - NLMSG_SPACE(sizeof(*ifinfo));

				while (RTA_OK(attr, len)) {
					if (attr->rta_type == IFLA_IFNAME) {
						p = (char*)RTA_DATA(attr);
						if (check_interface_type(p) == INTF_TYPE_APCLI)
							update_link_tbl(p, link_stat, INTF_TYPE_APCLI);
						update_path_tbl(p, link_stat);
					} else if (attr->rta_type == IFLA_WIRELESS) {
						iwe = (struct iw_event *)RTA_DATA(attr);
						if (iwe->cmd == IWEVCUSTOM) {
							rcv_msg = (struct parse_msg *)(((char *)iwe) + IW_EV_LCP_LEN);
							if ((rcv_msg->flags == FWD_CMD_ADD_TX_SRC ||
								rcv_msg->flags == FWD_CMD_DEL_TX_SRC) &&
								rcv_msg->length == MAC_ADDR_LEN) {
								p = (char *)(((char *)iwe) + IW_EV_POINT_LEN);
								memcpy(m.mac, p, MAC_ADDR_LEN);
								send_ctrl_msg((char)rcv_msg->flags, (char *)&m);
							}
						}
					}
					attr = RTA_NEXT(attr, len);
				}
			}
		}
#if LOOP_DETECT_IN_DAEMON
		else if (FD_ISSET(sock_rcv_bc, &fdsr)) {
			static int recv_len;
			static struct sockaddr_in s_addr;
			static unsigned int s_addr_len;

			recv_len = recvfrom(sock_rcv_bc, buf, BUFLEN, 0, (struct sockaddr*) &s_addr, &s_addr_len);
			if (recv_len == -1)
				perror("Error: recvfrom call failed");
			if (recv_len >= BUFLEN)
				DBGPRINT(DBG_LVL_ERROR, "recv buffer full!!!!!\n");

			if (STR_EQUAL(&buf[0], MAGIC)) {
				recv_bc_pkt(&buf[MG_LEN]);
			}
		}
#endif
	}

exit:
	clean_fwd_tbl();
	close_sock();

	DBGPRINT(DBG_LVL_TRACE, "fwd daemon terminated!!!\n");

	return 0;
}
