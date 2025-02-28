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
 * sp_ut.c, it's used to support unit test for service_prioritization.c
 *
 * Author: Zheng Zhou <Zheng.Zhou@mediatek.com>
 */

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <assert.h>
#include <linux/if_ether.h>
#include <asm/byteorder.h>
#include <stdlib.h>
#include <pthread.h>
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
#include "interface.h"
#include "data_def.h"

#include "list.h"
#include "common.h"
#include "debug.h"
#include "service_prioritization.h"
#include "mapfilter_if.h"
#include "_1905_interface_ctrl.h"

unsigned char tlv_temp[15360] = { 0 };

struct p1905_managerd_ctx {
	int dummy;
};

struct topology_response_db *lookup_tprdb_by_almac(struct p1905_managerd_ctx *ctx, unsigned char *al_mac)
{
	return NULL;
}

void insert_cmdu_txq(unsigned char *dmac, unsigned char *smac, int mtype,
		     unsigned short mid, unsigned char *ifname, unsigned char need_update_mid)
{
}

int process_cmdu_txq(struct p1905_managerd_ctx *ctx, unsigned char *buffer)
{
	return 0;
}

#define DEBUG_ERROR		LOG_ALERT

int debug_level = DEBUG_ERROR;

unsigned char *get_tlv_buffer(void)
{
	return tlv_temp;
}

int get_tlv_len(unsigned char *buf)
{
	unsigned char *temp_buf = buf;
	unsigned short length = 0;

	temp_buf += 1;		/* shift to length field */

	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	return length;
}

int get_cmdu_tlv_length(unsigned char *buf)
{
	unsigned char *temp_buf = buf;
	unsigned short length = 0;

	temp_buf += 1;		/* shift to length field */

	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	return (length + 3);
}

typedef void (*topology_tree_cb_func) (void *context, void *parent_leaf, void *current_leaf, void *data);
struct leaf {
	struct dl_list dummy;
};

int trace_topology_tree_cb(struct p1905_managerd_ctx *ctx, struct leaf *current_leaf, topology_tree_cb_func cb,
			   void *data)
{
	return 0;
}

unsigned short append_end_of_tlv(unsigned char *pkt)
{
	unsigned short total_length = 0;

	*pkt++ = 0;
	total_length += 1;

	*(unsigned short *)(pkt) = host_to_be16(0);
	pkt += 2;
	total_length += 2;

	return total_length;
}

struct sp_test_ctx {
	struct dl_list static_list;
	struct dl_list dynamic_list;
	struct dl_list del_list;
	struct dl_list learn_comp_list;
	struct sp_dscp_pcp_mapping_table dptbl;
};

#define STRESS_TEST_TIME 1000

#define ut_assert(cond) \
{\
	if (!(cond)) {\
		printf("!!!!!!!ASSERT!!!!!!,%s %d\n", __func__,  __LINE__);\
	} \
}
#define PRINT_IP_RAW(ip) ((ip) & 0x000000ff), (((ip) & 0x0000ff00) >> 8), (((ip) & 0x00ff0000) >> 16), (((ip) & 0xff000000) >> 24)
#define PRINT_MAC_RAW(a) a[0], a[1], a[2], a[3], a[4], a[5]
#define MACSTR_RAW "%02x:%02x:%02x:%02x:%02x:%02x"
#define IPSTR_RAW "%d.%d.%d.%d"

void hex_dump_ut(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
	unsigned char *pt;
	int x;

	pt = pSrcBufVA;
	printf("%s: %p, len = %d\n", str, pSrcBufVA, SrcBufLen);
	for (x = 0; x < SrcBufLen; x++) {
		if (x % 16 == 0)
			printf("0x%04x : ", x);
		printf("%02x ", ((unsigned char)pt[x]));
		if (x % 16 == 15)
			printf("\n");
	}
	printf("\n");
}

void test_case0(void)
{
	const char *ip[] = { "192.168.1.200",
		"2.168..200",
		"192..1.200",
		"192.168.1.",
		"192.133.1."
	};
	int i = 0;
	unsigned int ip_addr = 0;
	int ret = 0;

	for (i = 0; i < 5; i++) {
		ret = ipv4_aton(ip[i], &ip_addr);
		if (i == 0) {
			ut_assert(ret == 0);
			printf(IPSTR_RAW "\n", PRINT_IP_RAW(ip_addr));
		} else
			ut_assert(ret == -1);
	}
}

/*local read config file*/
void test_case1(void)
{
	int ret = 0;
	int i = 0;
	struct sp_test_ctx sp;

	/*init test contex */
	os_memset(&sp, 0, sizeof(struct sp_test_ctx));
	dl_list_init(&sp.static_list);
	dl_list_init(&sp.dynamic_list);
	dl_list_init(&sp.del_list);

	/*case 1: read rule config file
	 * expected results: should succed
	 */
	ret = sp_init_rule_from_profile(&sp.static_list, &sp.dptbl, "/etc/map/case1.cfg");

	ut_assert(ret == 0);

	show_rule_list_dptbl(&sp.static_list, &sp.dptbl);

	clear_rule_list(&sp.static_list);

	/*case 2: read rule config file
	 *expected results: should fail
	 */
	ret = sp_init_rule_from_profile(&sp.static_list, &sp.dptbl, "/etc/map/case2.cfg");

	ut_assert(ret == -1);

	show_rule_list_dptbl(&sp.static_list, &sp.dptbl);

	clear_rule_list(&sp.static_list);

	/*case 3: stress read rule config file
	 *expected results: should fail
	 */
	for (i = 0; i < STRESS_TEST_TIME; i++) {
		ret = sp_init_rule_from_profile(&sp.static_list, &sp.dptbl, "/etc/map/case3.cfg");

		ut_assert(ret == 0);

		show_rule_list_dptbl(&sp.static_list, &sp.dptbl);

		clear_rule_list(&sp.static_list);
	}
}

unsigned char buf[1024] = { 0x00 };

void test_case_1_5(void)
{
	char content[] = { "1.game_session:[]+[]+[]+[]+[]+[]+[]+[]=7\
.game_session:[00:0c:43:21:22:23]+[]+[]+[]+[]+[]+[]+[]=7\
3.game_session:[]+[multi_ap_5GH]+[]+[]+[]+[]+[]+[]=1\
4.game_session:[]+[]+[07:06:FE:Ac:34:Ab]+[]+[]+[]+[]+[]=2\
5.game_session:[]+[]+[]+[TCP]+[]+[]+[]+[]=3\
6.game_session:[]+[]+[]+[]+[8.8.9.3]+[]+[]+[]=4\
7.game_session:[]+[]+[]+[]+[]+[192.168.12.206]+[]+[]=5\
8.game_session:[]+[]+[]+[]+[]+[]+[43]+[]=6\
9.game_session:[]+[]+[]+[]+[]+[]+[]+[80]=7\
10.game_session:[]+[]+[07:06:FE:Ac:34:ac]+[TCP]+[]+[192.168.1.200]+[]+[8080]=7\
11.game_session:[]+[multi_ap_5GH]+[07:06:FE:Ac:34:Ab]+[TCP]+[10.18.12.123]+[192.168.1.200]+[]+[43]=7\
26.game_session:[]+[multi_ap_5GH]+[]+[]+[]+[192.168.1.200]+[8080]+[8080]=7\
DSCP_PCP_MAPPING=0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7" };
	char content1[] = { "1.game_session:[]+[]+[]+[]+[]+[]+[]+[]=7\
2.game_session:[00:0c:43:21:22:23]+[]+[]+[]+[]+[]+[]+[]=7\
3.game_session:[]+[multi_ap_5GH]+[]+[]+[]+[]+[]+[]=1\
4.game_session:[]+[]+[07:06:FE:Ac:34:Ab]+[]+[]+[]+[]+[]=2\
5.game_session:[]+[]+[]+[TCP]+[]+[]+[]+[]=3\
6.game_session:[]+[]+[]+[]+[8.8.9.3]+[]+[]+[]=4\
7.game_session:[]+[]+[]+[]+[]+[192.168.12.206]+[]+[]=5\
8.game_session:[]+[]+[]+[]+[]+[]+[43]+[]=6\
9.game_session:[]+[]+[]+[]+[]+[]+[]+[80]=7\
10.game_session:[]+[]+[07:06:FE:Ac:34:ac]+[TCP]+[]+[192.168.1.200]+[]+[8080]=7\
11.game_session:[]+[multi_ap_5GH]+[07:06:FE:Ac:34:Ab]+[TCP]+[10.18.12.123]+[192.168.1.200]+[]+[43]=7\
26.game_session:[]+[multi_ap_5GH]+[]+[]+[]+[192.168.1.200]+[8080]+[8080]=7" };
	char *pos = NULL;
	struct sp_test_ctx sp_send;

	os_memset(&sp_send, 0, sizeof(struct sp_test_ctx));

	dl_list_init(&sp_send.static_list);
	dl_list_init(&sp_send.dynamic_list);
	dl_list_init(&sp_send.del_list);

	pos = content;

	while ((pos = parse_rule_from_str(&sp_send.static_list, &sp_send.dptbl, pos)))
		;

	printf("%s %d, [rule list]\n", __func__, __LINE__);
	show_rule_list_dptbl(&sp_send.static_list, &sp_send.dptbl);

	pos = content1;

	while ((pos = parse_rule_from_str(&sp_send.static_list, &sp_send.dptbl, pos)))
		;

	printf("%s %d, [rule list]\n", __func__, __LINE__);
	show_rule_list_dptbl(&sp_send.static_list, &sp_send.dptbl);
}

void test_case2(void)
{
	int ret = 0;
	int i = 0, j = 0;
	struct sp_test_ctx sp_send, sp_recv;
	unsigned short send_len = 0;
	unsigned char peer_al_mac[6] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
	struct sp_rule_element *r1 = NULL, *r2 = NULL;
	int rule_num1 = 0, rule_num2 = 0;
	char content[] = { "1.game_session:[]+[]+[]+[]+[]+[]+[]+[]=7\
2.game_session:[00:0c:43:21:22:23]+[]+[]+[]+[]+[]+[]+[]=7\
13.game_session:[00:01:02:03:04:05]+[]+[]+[]+[]+[]+[]+[]=7\
3.game_session:[]+[multi_ap_5GH]+[]+[]+[]+[]+[]+[]=1\
4.game_session:[]+[]+[07:06:FE:Ac:34:Ab]+[]+[]+[]+[]+[]=2\
5.game_session:[]+[]+[]+[TCP]+[]+[]+[]+[]=3\
6.game_session:[]+[]+[]+[]+[8.8.9.3]+[]+[]+[]=4\
7.game_session:[]+[]+[]+[]+[]+[192.168.12.206]+[]+[]=5\
8.game_session:[]+[]+[]+[]+[]+[]+[43]+[]=6\
9.game_session:[]+[]+[]+[]+[]+[]+[]+[80]=7\
10.game_session:[]+[]+[07:06:FE:Ac:34:ac]+[TCP]+[]+[192.168.1.200]+[]+[8080]=7\
11.game_session:[]+[multi_ap_5GH]+[07:06:FE:Ac:34:Ab]+[TCP]+[10.18.12.123]+[192.168.1.200]+[]+[43]=7\
26.game_session:[]+[multi_ap_5GH]+[]+[]+[]+[192.168.1.200]+[8080]+[8080]=7" };
	char *pos = NULL;
	/*init test contex */

	os_memset(&sp_send, 0, sizeof(struct sp_test_ctx));
	os_memset(&sp_recv, 0, sizeof(struct sp_test_ctx));

	dl_list_init(&sp_send.static_list);
	dl_list_init(&sp_send.dynamic_list);
	dl_list_init(&sp_send.del_list);
	dl_list_init(&sp_send.learn_comp_list);

	dl_list_init(&sp_recv.static_list);
	dl_list_init(&sp_recv.dynamic_list);
	dl_list_init(&sp_recv.del_list);
	dl_list_init(&sp_send.learn_comp_list);

	pos = content;

	while ((pos = parse_rule_from_str(&sp_send.static_list, &sp_send.dptbl, pos)))
		;
	/*
	** printf("%s %d, [SEND rule list]\n", __func__, __LINE__);
	** show_rule_list_dptbl(&sp_send.static_list);
	*/

	/*ADD rule */
	/*simulate controller creates sp request */
	send_len = create_sp_request_message(get_tlv_buffer(), buf, &sp_send.static_list,
					     &sp_send.dynamic_list, &sp_send.del_list, &sp_send.dptbl, ADD_RULE,
					     peer_al_mac);

	/*simulate agent parses sp request */
	ret = parse_sp_request_message(&sp_recv.static_list, &sp_recv.dynamic_list, &sp_recv.del_list,
				       &sp_recv.learn_comp_list, &sp_recv.dptbl, peer_al_mac, get_tlv_buffer(),
				       send_len);

	ut_assert(ret == 0);
	/*
	** printf("%s %d, [RECEIVE rule list]\n", __func__, __LINE__);
	** show_rule_list_dptbl(&sp_recv.static_list);
	*/

	/*compare the static rule list on controller and agent */
	rule_num1 = dl_list_len(&sp_send.static_list);
	rule_num2 = dl_list_len(&sp_recv.static_list);

	ut_assert(rule_num1 == rule_num2);

	dl_list_for_each(r1, &sp_send.static_list, struct sp_rule_element, entry) {
		j = 0;
		dl_list_for_each(r2, &sp_send.static_list, struct sp_rule_element, entry) {
			if (j++ == i)
				break;
		}
		ut_assert(!os_memcmp(&r1->rule, &r2->rule, sizeof(struct sp_rule)));
		i++;
	}
	/*clear rule */

	/*simulate controller creates sp request */
	send_len = create_sp_request_message(get_tlv_buffer(), buf, &sp_send.static_list,
					     &sp_send.dynamic_list, &sp_send.del_list, &sp_send.dptbl, REMOVE_RULE,
					     peer_al_mac);

	/*simulate agent parses sp request */
	ret = parse_sp_request_message(&sp_recv.static_list, &sp_recv.dynamic_list, &sp_recv.del_list,
				       &sp_recv.learn_comp_list, &sp_recv.dptbl, peer_al_mac, get_tlv_buffer(),
				       send_len);

	ut_assert(ret == 0);

	/*compare the static rule list on controller and agent */
	rule_num2 = dl_list_len(&sp_recv.static_list);

	ut_assert(rule_num2 == 0);
}

/*
** test interface between 1905daemon and mapfilter
** steps and results:
**		1. add multiple sp rules; [result] rule list in 1905daemon and mapfilter should be same.
**		2. add same few sp rules; [result] no duplicated rule in rule list,
**		   1905daemon and mapfilter should be same.
**		3. remove one sp rule;    [result] rule list in 1905daemon and mapfilter should be same.
**		4. clear sp rules;        [result] rule list should be empty.
*/
void test_case3(void)
{
	int ret = 0;
	struct sp_test_ctx sp_send;
	struct sp_rule_element *r = NULL;
	struct dl_list del_list;
	int index = 0, i = 0;

	char content[] = { "1.game_session:[]+[]+[]+[]+[]+[]+[]+[]=7\
2.game_session:[00:0c:43:21:22:23]+[]+[]+[]+[]+[]+[]+[]=7\
13.game_session:[00:01:02:03:04:05]+[]+[]+[]+[]+[]+[]+[]=7\
3.game_session:[]+[multi_ap_5GH]+[]+[]+[]+[]+[]+[]=1\
4.game_session:[]+[]+[07:06:FE:Ac:34:Ab]+[]+[]+[]+[]+[]=2\
5.game_session:[]+[]+[]+[TCP]+[]+[]+[]+[]=3\
6.game_session:[]+[]+[]+[]+[8.8.9.3]+[]+[]+[]=4\
7.game_session:[]+[]+[]+[]+[]+[192.168.12.206]+[]+[]=5\
8.game_session:[]+[]+[]+[]+[]+[]+[43]+[]=6\
9.game_session:[]+[]+[]+[]+[]+[]+[]+[80]=7\
10.:[]+[]+[07:06:FE:Ac:34:ac]+[TCP]+[]+[192.168.1.200]+[]+[8080]=7\
11.game_session:[]+[multi_ap_5GH]+[07:06:FE:Ac:34:Ab]+[TCP]+[10.18.12.123]+[192.168.1.200]+[]+[43]=7\
26.game_session:[]+[multi_ap_5GH]+[]+[]+[]+[192.168.1.200]+[8080]+[8080]=7\
DSCP_PCP_MAPPING=0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7" };
	char *pos = NULL;
	/*init test contex */

	os_memset(&sp_send, 0, sizeof(struct sp_test_ctx));

	dl_list_init(&sp_send.static_list);
	dl_list_init(&sp_send.dynamic_list);
	dl_list_init(&sp_send.del_list);
	dl_list_init(&del_list);

	if (mapfilter_netlink_init(getpid()) < 0)
		debug(DEBUG_ERROR, "mapfilter_netlink_init init fail\n");

	pos = content;

	while ((pos = parse_rule_from_str(&sp_send.static_list, &sp_send.dptbl, pos)))
		;

	/*1. apply all rules to mapfilter */
	ret = apply_sp_rules_to_local(&sp_send.static_list, NULL, sp_send.dptbl.dptbl);

	ut_assert(ret == 0);

	show_rule_list_dptbl(&sp_send.static_list, &sp_send.dptbl);

	if (dl_list_len(&sp_send.static_list) == 0) {
		ut_assert(0);
		return;
	}
	/*2. remove one rule */
	index = os_random() % dl_list_len(&sp_send.static_list);
	dl_list_for_each(r, &sp_send.static_list, struct sp_rule_element, entry) {
		if (i++ > index) {
			dl_list_del(&r->entry);
			break;
		}
	}

	if (r) {
		dl_list_add(&del_list, &r->entry);
		debug(DEBUG_ERROR, "del list entry\n");
		show_rule_list_dptbl(&del_list, NULL);

		ret = remove_sp_rules_of_local(&del_list);
		ut_assert(ret == 0);
	}
}

/*
 *test mapfilter sp_function
 * topology:
 * PC<--------[eth]--------->MAP_device<-----------[wifi]------------->station
 * 1. 8 different rules can be selected, single match condition is set
 * 2. capture wireshark log to chceck dscp value.
 */
void test_case4(int option)
{
	int ret = 0;
	struct sp_test_ctx sp_send;

	char *content[] = { "1.al_mac:[00:0c:43:21:22:23]+[]+[]+[]+[]+[]+[]+[]=6",
		"1.ssid:[]+[Multi-AP-5LG-1]+[]+[]+[]+[]+[]+[]=5",
		"1.client_mac:[]+[]+[14:3c:c3:29:3b:e3]+[]+[]+[]+[]+[]=7",
		"1.protocol_IGMP:[]+[]+[]+[IGMP]+[]+[]+[]+[]=7",
		"1.protocol_TCP:[]+[]+[]+[TCP]+[]+[]+[]+[]=7",
		"1.dst_IP:[]+[]+[]+[]+[192.168.1.106]+[]+[]+[]=7",
		"1.src_IP:[]+[]+[]+[]+[]+[192.168.1.200]+[]+[]=7",
		"1.dst_port:[]+[]+[]+[]+[]+[]+[80]+[]=7",
		"1.src_port:[]+[]+[]+[]+[]+[]+[]+[8080]=7"
	};
	char *pos = NULL;
	/*init test contex */

	os_memset(&sp_send, 0, sizeof(struct sp_test_ctx));

	dl_list_init(&sp_send.static_list);
	dl_list_init(&sp_send.dynamic_list);
	dl_list_init(&sp_send.del_list);

	if (mapfilter_netlink_init(getpid()) < 0)
		debug(DEBUG_ERROR, "mapfilter_netlink_init init fail\n");

	pos = content[option];

	while ((pos = parse_rule_from_str(&sp_send.static_list, &sp_send.dptbl, pos)))
		;

	/*1. apply all rules to mapfilter */
	ret = apply_sp_rules_to_local(&sp_send.static_list, NULL, sp_send.dptbl.dptbl);

	ut_assert(ret == 0);

	show_rule_list_dptbl(&sp_send.static_list, &sp_send.dptbl);

}

/*
 *test 1905library Service Prioritization APIs
 */
void test_case5(int option)
{
	int ret = 0, i = 0, len = 0, j = 0;

	char *content[] = { "1.al_mac:[00:0c:43:21:22:23]+[]+[]+[]+[]+[]+[]+[]=6",
		"2.ssid:[]+[Multi-AP-5LG-1]+[]+[]+[]+[]+[]+[]=5",
		"3.client_mac:[]+[]+[14:3c:c3:29:3b:e3]+[]+[]+[]+[]+[]=7",
		"4.protocol_IGMP:[]+[]+[]+[IGMP]+[]+[]+[]+[]=7",
		"5.protocol_TCP:[]+[]+[]+[TCP]+[]+[]+[]+[]=7",
		"6.dst_IP:[]+[]+[]+[]+[192.168.1.106]+[]+[]+[]=7",
		"7.src_IP:[]+[]+[]+[]+[]+[192.168.1.200]+[]+[]=7",
		"8.dst_port:[]+[]+[]+[]+[]+[]+[80]+[]=7",
		"9.src_port:[]+[]+[]+[]+[]+[]+[]+[8080]=7"
	};

	char *content_new[] = { "9.al_mac:[00:0c:43:21:22:23]+[]+[]+[]+[]+[]+[]+[]=6",
		"8.ssid:[]+[Multi-AP-5LG-1]+[]+[]+[]+[]+[]+[]=5",
		"7.client_mac:[]+[]+[14:3c:c3:29:3b:e3]+[]+[]+[]+[]+[]=7",
		"6.protocol_IGMP:[]+[]+[]+[IGMP]+[]+[]+[]+[]=7",
		"5.protocol_TCP:[]+[]+[]+[TCP]+[]+[]+[]+[]=7",
		"4.dst_IP:[]+[]+[]+[]+[192.168.1.106]+[]+[]+[]=7",
		"3.src_IP:[]+[]+[]+[]+[]+[192.168.1.200]+[]+[]=7",
		"2.dst_port:[]+[]+[]+[]+[]+[]+[80]+[]=7",
		"1.src_port:[]+[]+[]+[]+[]+[]+[]+[8080]=7"
	};

	char *dscp_tbl =
	    "DSCP_PCP_MAPPING=0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7.0.1.2.3.4.0.6.7";
	char tmp[1024] = { 0 };
	struct _1905_context *ctx = NULL;

	/*init test contex */

	/*1. add rule */
	for (i = 0; i < 9; i++) {
		memset(tmp, 0, sizeof(tmp));
		if (strlen(content[i]) > sizeof(tmp)) {
			ut_assert(0);
			return;
		}
		strncpy(tmp, content[i], strlen(content[i]));
		ret = _1905_sp_add_static_rule(tmp, strlen(tmp));
		ut_assert(ret == 0);
	}

	memset(tmp, 0, sizeof(tmp));

	/*2. get all rules */
	ret = _1905_sp_get_static_rule(0, tmp, &len);
	ut_assert(ret == 0);

	/*get single rule and check if it equals to original one */
	for (i = 0; i < 9; i++) {
		memset(tmp, 0, sizeof(tmp));
		ret = _1905_sp_get_static_rule(i + 1, tmp, &len);
		ut_assert(ret == 0);

		/*
		** printf("recv:%s len=%d\ncontent=%s content_len=%d\n", tmp, len,
		** content[i], strlen(content[i]));
		*/
		ut_assert(memcmp(tmp, content[i], len - 1) == 0)
	}

	ctx = _1905_Init("sp_ut");
	ut_assert(ctx != NULL);

	/*3. set rule */
	for (i = 0; i < 9; i++) {
		/*set rule */
		ret = _1905_sp_set_static_rule(ctx, content_new[i], strlen(content_new[i]));
		ut_assert(ret == 0);

		/*get rule and compare */
		memset(tmp, 0, sizeof(tmp));
		ret = _1905_sp_get_static_rule(9 - i, tmp, &len);
		ut_assert(ret == 0)

		    ut_assert(memcmp(tmp, content_new[i], len - 1) == 0);
	}
#if 0
	memset(tmp, 0, sizeof(tmp));
	ret = _1905_sp_get_static_rule(0, tmp, &len);
	ut_assert(ret == 0);
	printf("all static rule 3:%s\n", tmp);
#endif
	/*4. reorder rule */
	for (i = 0; i < 9; i++) {
		ret = _1905_sp_reorder_static_rule(ctx, 9, i + 1);
		ut_assert(ret == 0);
	}

	for (i = 0; i < 9; i++) {
		memset(tmp, 0, sizeof(tmp));
		ret = _1905_sp_get_static_rule(i + 1, tmp, &len);
		ut_assert(ret == 0);
		ut_assert(memcmp(tmp, content[i], len - 1) == 0);
	}

	/*4.5.1 move rule backward */
	for (i = 0; i < 9; i++) {
		for (j = 1; j < 9 - i; j++) {
			ret = _1905_sp_move_static_rule(ctx, j, MOVE_BACKWARD);
			ut_assert(ret == 0);
		}
	}

	for (i = 0; i < 9; i++) {
		memset(tmp, 0, sizeof(tmp));
		ret = _1905_sp_get_static_rule(i + 1, tmp, &len);
		ut_assert(ret == 0);
		ut_assert(memcmp(tmp, content_new[8 - i], len - 1) == 0);
	}

	/*4.5.2 move rule forward */
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 8 - i; j++) {
			ret = _1905_sp_move_static_rule(ctx, 9 - j, MOVE_FORWARD);
			ut_assert(ret == 0);
		}
	}

	for (i = 0; i < 9; i++) {
		memset(tmp, 0, sizeof(tmp));
		ret = _1905_sp_get_static_rule(i + 1, tmp, &len);
		ut_assert(ret == 0);
		ut_assert(memcmp(tmp, content[i], len - 1) == 0);
	}

#if 0
	memset(tmp, 0, sizeof(tmp));
	ret = _1905_sp_get_static_rule(0, tmp, &len);
	ut_assert(ret == 0);
	printf("all static rule 4:%s\n", tmp);
#endif
	/*5. remove rule */
	i = os_random() % 9;
	memset(tmp, 0, sizeof(tmp));
	ret = _1905_sp_rm_static_rule(ctx, i + 1);
	ut_assert(ret == 0);

	ret = _1905_sp_get_static_rule(i + 1, tmp, &len);
	ut_assert(ret == -1);

	printf("remove rule index=%d\n", i + 1);

	/*6. set dscptble and config done */
	ret = _1905_sp_set_dscp_tbl(ctx, dscp_tbl);
	ut_assert(ret == 0);

	ret = _1905_sp_config_done(ctx);
	ut_assert(ret == 0);

	_1905_Deinit(ctx);

}

/*test learn complete event create rule mechanisim*/
void test_learn_complete_tlv(void)
{
#define CONTROLLER 0
#define AGENT 1
	struct sp_learn_complete_event comp_event[5] = {
		{6, 0x04, 0x0f, .src_addr.v4_addr = 0x6401A8C0, .dst_addr.v4_addr = 0xC0A80164, 100, 200},
		{5, 0x04, 0x0e, .src_addr.v4_addr = 0x6501A8C0, .dst_addr.v4_addr = 0xC0A80165, 101, 201},
		{4, 0x04, 0x0d, .src_addr.v4_addr = 0x6601A8C0, .dst_addr.v4_addr = 0xC0A80166, 102, 202},
		{3, 0x04, 0x0b, .src_addr.v4_addr = 0x6701A8C0, .dst_addr.v4_addr = 0xC0A80167, 103, 203},
		{2, 0x04, 0x07, .src_addr.v4_addr = 0x6801A8C0, .dst_addr.v4_addr = 0xC0A80168, 104, 204}
	};
	unsigned char *learn_comp_evet_tlv = NULL;
	unsigned short tlv_len = 0, rule_mask = 0;
	struct sp_test_ctx sp_c1, sp_c2;
	int i = 0, ret = 0, event_num = 0, count = 0;
	unsigned char own_mac[6] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
	struct sp_rule_element *rule_1 = NULL, *rule_2 = NULL;
	unsigned int integrity = 0;
	struct dl_list static_list_tmp;
	struct dl_list dynamic_list_tmp;
	struct dl_list learn_complete_list_tmp;

	learn_comp_evet_tlv = malloc(2048);
	ut_assert(learn_comp_evet_tlv);

	if (!learn_comp_evet_tlv)
		return;

	os_memset(&sp_c1, 0, sizeof(struct sp_test_ctx));
	os_memset(&sp_c2, 0, sizeof(struct sp_test_ctx));

	dl_list_init(&sp_c1.static_list);
	dl_list_init(&sp_c1.dynamic_list);
	dl_list_init(&sp_c1.del_list);
	dl_list_init(&sp_c1.learn_comp_list);

	dl_list_init(&sp_c2.static_list);
	dl_list_init(&sp_c2.dynamic_list);
	dl_list_init(&sp_c2.del_list);
	dl_list_init(&sp_c2.learn_comp_list);

	dl_list_init(&static_list_tmp);
	dl_list_init(&dynamic_list_tmp);
	dl_list_init(&learn_complete_list_tmp);

	memset(learn_comp_evet_tlv, 0, 2048);

	event_num = sizeof(comp_event) / sizeof(struct sp_learn_complete_event);

	for (i = 0; i < event_num; i++) {
		/*1. simulate create sp rule by lrarn complete event from mapfilter on Controller */
		ret = sp_process_learn_complete_event(CONTROLLER, (unsigned char *)&comp_event[i],
			&sp_c1.learn_comp_list, NULL, &tlv_len);
		ut_assert(ret == 0);
		/*2. simulate create learn complete notification tlv by learn complete event from mapfilter on Agent */
		ret = sp_process_learn_complete_event(AGENT, (unsigned char *)&comp_event[i], NULL,
			learn_comp_evet_tlv, &tlv_len);
		ut_assert(tlv_len != 0 && ret >= 0);

		/*3. simulate parse learn complete notification tlv and create sp rule on Controller */
		if (parse_sp_vendor_tlv(learn_comp_evet_tlv, 2048,
				    own_mac, &integrity, &static_list_tmp,
				    &dynamic_list_tmp, &learn_complete_list_tmp) < 0)
			goto error;

		if (integrity & BIT(0)) {
			debug(DEBUG_ERROR, SP_PREX"sync rule for static list\n");
			sync_rule_between_list(&sp_c2.static_list, &static_list_tmp);
			debug(DEBUG_ERROR, SP_PREX"sync rule for dynamic list\n");
			sync_rule_between_list(&sp_c2.dynamic_list, &dynamic_list_tmp);
		}
		if (integrity & BIT(2)) {
			debug(DEBUG_ERROR, SP_PREX"free all rule in static and dynamic list\n");
			clear_rule_list(&sp_c2.static_list);
			clear_rule_list(&sp_c2.dynamic_list);
		}
		if (integrity & BIT(3)) {
			debug(DEBUG_ERROR, SP_PREX"sync rule for learn complete list\n");
			sync_rule_between_list(&sp_c2.learn_comp_list, &learn_complete_list_tmp);
		}
	}

	/*4. check rule list */
	ut_assert(dl_list_len(&sp_c1.learn_comp_list) == event_num);
	ut_assert(dl_list_len(&sp_c2.learn_comp_list) == event_num);
	ut_assert(dl_list_len(&sp_c2.static_list) == 0);
	ut_assert(dl_list_len(&sp_c2.dynamic_list) == 0);

	/*5. check rule detail one by one */
	for (i = 0; i < event_num; i++) {
		rule_mask = 0;
		count = i + 1;
		dl_list_for_each(rule_1, &sp_c1.learn_comp_list, struct sp_rule_element, entry) {
			count--;
			if (count == 0)
				break;
		}

		count = i + 1;
		dl_list_for_each(rule_2, &sp_c2.learn_comp_list, struct sp_rule_element, entry) {
			count--;
			if (count == 0)
				break;
		}

		ut_assert(comp_event[i].ip_ver == rule_1->rule.tcpip.ip_ver);
		ut_assert(comp_event[i].ip_ver == rule_2->rule.tcpip.ip_ver);

		ut_assert(comp_event[i].up == rule_1->rule.rule_output);
		ut_assert(comp_event[i].up == rule_2->rule.rule_output);

		if (comp_event[i].mask & BIT(3)) {
			ut_assert(comp_event[i].pkt_dst_port == rule_1->rule.tcpip.dst_port);
			ut_assert(comp_event[i].pkt_dst_port == rule_2->rule.tcpip.dst_port);
			rule_mask |= RULE_TCPIP_DST_PORT;
		}

		if (comp_event[i].mask & BIT(2)) {
			ut_assert(comp_event[i].pkt_src_port == rule_1->rule.tcpip.src_port);
			ut_assert(comp_event[i].pkt_src_port == rule_2->rule.tcpip.src_port);
			rule_mask |= RULE_TCPIP_SRC_PORT;
		}

		if (comp_event[i].ip_ver == 0x04) {
			if (comp_event[i].mask & BIT(0)) {
				ut_assert(comp_event[i].src_addr.v4_addr == rule_1->rule.tcpip.src_addr.v4_addr);
				ut_assert(comp_event[i].src_addr.v4_addr == rule_2->rule.tcpip.src_addr.v4_addr);
				rule_mask |= RULE_TCPIP_SRC_IP;
			}

			if (comp_event[i].mask & BIT(1)) {
				ut_assert(comp_event[i].dst_addr.v4_addr == rule_1->rule.tcpip.dst_addr.v4_addr);
				ut_assert(comp_event[i].dst_addr.v4_addr == rule_2->rule.tcpip.dst_addr.v4_addr);
				rule_mask |= RULE_TCPIP_DST_IP;
			}
		} else if (comp_event[i].ip_ver == 0x06) {
			if (comp_event[i].mask & BIT(0)) {
				ut_assert(memcmp
					  (comp_event[i].src_addr.v6_addr, rule_1->rule.tcpip.src_addr.v6_addr,
					   16) == 0)
				    ut_assert(memcmp
					      (comp_event[i].src_addr.v6_addr, rule_2->rule.tcpip.src_addr.v6_addr,
					       16) == 0)
				    rule_mask |= RULE_TCPIP_SRC_IP;
			}
			if (comp_event[i].mask & BIT(1)) {
				ut_assert(memcmp
					  (comp_event[i].dst_addr.v6_addr, rule_1->rule.tcpip.dst_addr.v6_addr,
					   16) == 0)
				    ut_assert(memcmp
					      (comp_event[i].dst_addr.v6_addr, rule_2->rule.tcpip.dst_addr.v6_addr,
					       16) == 0)
				    rule_mask |= RULE_TCPIP_DST_IP;
			}
		} else {
		}

		ut_assert(rule_1->rule.mask == rule_mask);
		ut_assert(rule_2->rule.mask == rule_mask);
	}

	show_rule_list_dptbl(&sp_c1.learn_comp_list, NULL);
	show_rule_list_dptbl(&sp_c2.learn_comp_list, NULL);
error:
	clear_rule_list(&static_list_tmp);
	clear_rule_list(&dynamic_list_tmp);
	clear_rule_list(&learn_complete_list_tmp);
	if (learn_comp_evet_tlv)
		free(learn_comp_evet_tlv);
}

int main(int argc, char *argv[])
{
	debug(DEBUG_ERROR, SP_PREX "UT START\n");
	unsigned char index = 0;

	if (argv[1][0] == '0')
		test_case0();
	if (argv[1][0] == '1')
		test_case1();
	else if (argv[1][0] == '2')
		test_case_1_5();
	else if (argv[1][0] == '3')
		test_case2();
	else if (argv[1][0] == '4')
		test_case3();
	else if (argv[1][0] == '5') {
		index = argv[2][0] - '0';
		test_case4((int)index);
	} else if (argv[1][0] == '6') {
		test_case5(0);
	} else if (argv[1][0] == '7') {
		test_learn_complete_tlv();
	}
	debug(DEBUG_ERROR, SP_PREX "UT END\n");
	return 0;
}
