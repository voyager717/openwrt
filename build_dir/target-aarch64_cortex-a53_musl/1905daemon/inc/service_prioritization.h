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
 * service_prioritization.h, it's used to support MAP R3 Service Prioritization
 *
 * Author: Zheng Zhou <Zheng.Zhou@mediatek.com>
 */

#ifndef _SERVICE_PRIORITIZATION_H
#define _SERVICE_PRIORITIZATION_H

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

#define SP_RULE_FILE_PATH "/etc/map/sp_rule.cfg"

enum {
	RULE_INDEX_IDX = 1,
	RULE_NAME_IDX,
	RULE_AL_MAC_IDX,
	RULE_SSID_IDX,
	RULE_MAC_IDX,
	RULE_TCPIP_PRO_IDX,
	RULE_TCPIP_DST_IP_IDX,
	RULE_TCPIP_SRC_IP_IDX,
	RULE_TCPIP_DST_PORT_IDX,
	RULE_TCPIP_SRC_PORT_IDX,
	RULE_OUTPUT
};

enum {
	RULE_TYPE_STATIC = 0,
	RULE_TYPE_DYNAMIC
};

#define IP_V4 0x04
#define IP_V6 0x06

#define RULE_AL_MAC BIT(0)
#define RULE_SSID	BIT(1)
#define RULE_MAC	BIT(2)
#define RULE_TCPIP_PRO	BIT(3)
#define RULE_TCPIP_DST_IP	BIT(4)
#define RULE_TCPIP_SRC_IP	BIT(5)
#define RULE_TCPIP_DST_PORT	BIT(6)
#define RULE_TCPIP_SRC_PORT	BIT(7)

#define SP_RULE_MAX_VALUE 0x09
#define SP_RULE_MIN_VALUE 0x00
#define SP_RULE_DSCP_PCP_ENABLE 0x08

#define SP_RULE_TLV_LENGTH				0x08
#define SP_DSCP_MAPPING_TABLE_LENGTH	64
enum {
	SP_VENDOR_TYPE_RULE = 0,
	SP_VENDOR_TYPE_DYNAMIC_SWITCH,
	SP_VENDOR_TYPE_CLEAR_RULE,
	SP_VENDOR_TYPE_LEARNING_COMPELETE_NOTIFICATION
};


#define STANDARD_SP_RULE(r) (!((r)->mask ^ (RULE_AL_MAC)))
#define SP_RULE_ALWAYS_MATCH BIT(7)

enum rule_action {
	REMOVE_RULE = 0,
	ADD_RULE	
};

enum move_action {
	MOVE_FORWARD = 0,
	MOVE_BACKWARD
};

struct ip_proto {
	char proto_name[16];
	unsigned char value;
};

struct GNU_PACKED tcpip_5_tuple {
	unsigned char protocol;
	union {
		unsigned int v4_addr;
		unsigned char v6_addr[16];
	} src_addr;
	union {
		unsigned int v4_addr;
		unsigned char v6_addr[16];
	} dst_addr;
	unsigned short dst_port;
	unsigned short src_port;
	unsigned char ip_ver;
};

#define RULE_NAME_SIZE 32

struct GNU_PACKED sp_rule {
	unsigned char index;
	unsigned char name_len;
	char rule_name[RULE_NAME_SIZE + 1];
	unsigned short mask;
	unsigned char rule_type;
	unsigned char al_mac[ETH_ALEN];
	unsigned char ssid_len;
	char ssid[SSID_MAX_LEN + 1];
	unsigned char client_mac[ETH_ALEN];
	struct tcpip_5_tuple tcpip;
	unsigned char rule_output;
	unsigned char rule_identifier[4]; /*indentified in standard rule tlv*/
};

struct GNU_PACKED sp_dscp_pcp_tbl {
	unsigned char dscp_2_pcp[64];
};

/*indicating whether the rule bas been synced to other agents*/
enum rule_sync_status {
	RULE_UNSYNC = 0,
	RULE_SYNC
};

/*indicating whether the rule has been configed into mapfilter*/
enum rule_config_status {
	RULE_NOT_CONFIGED_2_MAPFILTER = 0,
	RULE_CONFIGED_2_MAPFILTER
};

struct sp_rule_element {
	struct dl_list entry;
	enum rule_config_status conf_status;
	struct sp_rule rule;
};

struct GNU_PACKED sp_dscp_pcp_mapping_table {
	unsigned char enable;
	unsigned char dptbl[64];
};

struct sp_request_send_config {
	enum rule_action action;
};

struct GNU_PACKED sp_learn_complete_event
{
	unsigned char up;
	unsigned char ip_ver;
	unsigned char mask;
	union {
		unsigned int v4_addr;
		unsigned char v6_addr[16];
	} src_addr;
	union {
		unsigned int v4_addr;
		unsigned char v6_addr[16];
	} dst_addr;
	unsigned short pkt_src_port;
	unsigned short pkt_dst_port;
};

char *parse_rule_from_str(struct dl_list *list, struct sp_dscp_pcp_mapping_table *dp_tbl, char *str);

int apply_sp_rules_to_local(struct dl_list *list1, struct dl_list *list2,
	unsigned char *dptbl);
int clear_sp_rules_of_local(struct dl_list *static_list, struct dl_list *dynamic_list);
int remove_sp_rules_of_local(struct dl_list *list_del);
void show_rule_list_dptbl(struct dl_list *rule_list, struct sp_dscp_pcp_mapping_table *dptbl);
void insert_rule_to_rule_list(struct dl_list *rule_list, struct sp_rule_element *r);
void remove_rule_from_rule_list(struct dl_list *list, unsigned char *rule_id,
	struct dl_list *list_del);
int clear_rule_list(struct dl_list *rule_list);
int sp_init_rule_from_profile(struct dl_list *list, struct sp_dscp_pcp_mapping_table *dp_tbl, char *path);
int sp_rule_write_back_2_profile(struct dl_list *list, struct sp_dscp_pcp_mapping_table *dp_tbl, char *path);
int service_prioritization_init(void *context);
unsigned short append_sp_rule_tlv(struct sp_rule_element *r, unsigned char *tlv,
	enum rule_action action, unsigned char *peer_al_mac);
unsigned short append_dscp_mapping_table_tlv(unsigned char *dptbl, unsigned char *tlv);
unsigned short append_sp_vendor_rule_tlv(struct sp_rule_element *r, unsigned char *tlv);
unsigned short append_sp_vendor_clear_rule_tlv(unsigned char *tlv);
unsigned short create_sp_request_message(unsigned char *tlv_buf,
	unsigned char *buf, struct dl_list *static_list,
	struct dl_list *dynamic_list, struct dl_list *del_list,
	struct sp_dscp_pcp_mapping_table *dp_tbl,
	enum rule_action action, unsigned char *peer_al_mac);
int parse_sp_rule_tlv(unsigned char *tlv, unsigned short len,
	unsigned char *own_almac, struct dl_list *list, struct dl_list *del_list);
int parse_sp_dscp_mapping_tbl_tlv(unsigned char *tlv,
	unsigned short len, struct sp_dscp_pcp_mapping_table *dptbl);
int check_sp_vendor_tlvs(unsigned char *tlv, unsigned char *sp_action_type);
int parse_sp_vendor_rule_tlv(unsigned char *buf, unsigned short len,
	struct dl_list *static_list, struct dl_list *dynamic_list);
int parse_sp_vendor_tlv(unsigned char *tlv, unsigned short len,
	unsigned char *own_almac, unsigned int *sub_integrity,
	struct dl_list *static_list, struct dl_list *dynamic_list,
	struct dl_list *learn_complete_list);
int parse_sp_request_message(struct dl_list *static_list, struct dl_list *dynamic_list,
	struct dl_list *del_list, struct dl_list *learn_complete_list,
	struct sp_dscp_pcp_mapping_table *dptbl, unsigned char *own_almac,
	unsigned char *buf, unsigned int left_tlv_len);

void sp_rule_sync_handler(void *context, void* parent_leaf, void *current_leaf, void *data);
void sp_rule_config_done_action(void *context);
int service_prioritization_deinit(void *context);
int sp_process_1905_lib_cmd(void *context, unsigned char *rcv_buf,
	unsigned short len, unsigned char *reply_buf, int *replay_len);
int sp_process_learn_complete_event(unsigned char own_role, unsigned char *event_buf,
	struct dl_list *target_list, unsigned char *back_buf, unsigned short *back_buf_len);
int sp_sync_learn_complete_rule(void *context);
int parse_sp_learn_complete_tlv(
	unsigned char *buf, unsigned short len, struct dl_list *list);

unsigned short get_standard_sp_rule(unsigned char *buf, unsigned short buf_len,
	struct dl_list *static_list, struct sp_dscp_pcp_mapping_table *dscp_tbl,
	enum rule_action action, unsigned char *peer_al_mac);
void sync_rule_between_list(struct dl_list *list_d, struct dl_list *list_s);

#endif
