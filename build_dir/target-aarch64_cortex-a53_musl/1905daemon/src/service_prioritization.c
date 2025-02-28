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
 * service_prioritization.c, it's used to support MAP R3 Service Prioritization
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
#include <regex.h>
#include "multi_ap.h"
#include "cmdu_message.h"
#include "cmdu_tlv_parse.h"
#include "cmdu.h"
#include "os.h"
#include "common.h"
#include "debug.h"
#include "mapfilter_if.h"
#include "cmdu_message.h"
#include "service_prioritization.h"
#include "topology.h"

#define RULE_REGEX_GROUP_NUMBER 12

#define PRINT_IP_RAW(ip) ((ip) & 0x000000ff), (((ip) & 0x0000ff00) >> 8), (((ip) & 0x00ff0000) >> 16), (((ip) & 0xff000000) >> 24)
#define PRINT_MAC_RAW(a) a[0],a[1],a[2],a[3],a[4],a[5]
#define MACSTR_RAW "%02x:%02x:%02x:%02x:%02x:%02x"
#define IPSTR_RAW "%d.%d.%d.%d"
#ifndef REG_OK
#define REG_OK 0
#endif
struct ip_proto IP_PROTO[4] = {
	{"ICMP", 0x01},
	{"IGMP", 0x02},
	{"TCP", 0x06},
	{"UDP", 0x11}
};

int apply_sp_rules_to_local(struct dl_list *list1, struct dl_list *list2,
	unsigned char *dptbl)
{
	int ret = 0;
	
	ret = mapfilter_set_sp_rules(list1, list2);

	if (ret < 0)
		debug(DEBUG_ERROR, SP_PREX"error to apply rules to mapfilter\n");

	if (dptbl)
		ret = mapfilter_set_sp_dscp_mapping_tbl(dptbl);

	if (ret < 0)
		debug(DEBUG_ERROR, SP_PREX"error to apply dscp mapping tbl to mapfilter\n");

	return ret;
}

int clear_sp_rules_of_local(struct dl_list *static_list, struct dl_list *dynamic_list)
{
	int ret = 0;
	struct sp_rule_element *r = NULL;

	if (static_list) {
		dl_list_for_each(r, static_list, struct sp_rule_element, entry) {
			r->conf_status = RULE_NOT_CONFIGED_2_MAPFILTER;
		}
	}

	if (dynamic_list) {
		dl_list_for_each(r, dynamic_list, struct sp_rule_element, entry) {
			r->conf_status = RULE_NOT_CONFIGED_2_MAPFILTER;
		}
	}

	ret = mapfilter_set_sp_clear_rules();

	return ret;
}

int remove_sp_rules_of_local(struct dl_list *list_del)
{
	int ret = 0;
	struct sp_rule_element *r = NULL;

	dl_list_for_each(r, list_del, struct sp_rule_element, entry) {
		mapfilter_set_sp_rm_one_rule(&r->rule);
	}

	ret = clear_rule_list(list_del);

	return ret;
}

int find_ip_proto(char *proto_name)
{
	int i = 0;

	for (i = 0; i < sizeof(IP_PROTO) / sizeof(struct ip_proto); i++) {
		if(!os_strcmp(proto_name, IP_PROTO[i].proto_name))
			return IP_PROTO[i].value;
	}

	return -1;
}

char *proto_2_str(unsigned char protocol) {
	int i = 0;

	for (i = 0; i < sizeof(IP_PROTO) / sizeof(struct ip_proto); i++) {
		if (protocol == IP_PROTO[i].value)
			return IP_PROTO[i].proto_name;
	}

	return "";
}

int str_2_rule_element(char *str_element, int str_len, struct sp_rule *rule,
	int idx, unsigned short *integrity)
{
	int ret = 0;
	switch(idx) {
		case RULE_INDEX_IDX:
			ret = atoi(str_element);
			if (ret >= 256 || ret < 0) {
				ret = -1;
				goto end;
			}
			rule->index = ret;
			/*only defined in standard rule tlv*/
			rule->rule_identifier[0] = ret;
			*integrity |= 1 << idx;
			break;
		case RULE_NAME_IDX:
			if (str_len > RULE_NAME_SIZE) {
				ret = -1;
				goto end;
			}
			rule->name_len = str_len;
			os_strncpy(rule->rule_name, str_element, str_len);
			break;
		case RULE_AL_MAC_IDX:
			ret = hwaddr_aton2(str_element, rule->al_mac);

			if (ret < 0)
				goto end;

			rule->mask |= RULE_AL_MAC;
			*integrity |= 1 << idx;
			break;
		case RULE_SSID_IDX:
			if (str_len > SSID_MAX_LEN) {
				ret = -1;
				goto end;
			}
			os_memcpy(rule->ssid, str_element, str_len);
			rule->ssid_len = str_len;
			rule->mask |= RULE_SSID;
			*integrity |= 1 << idx;
			break;
		case RULE_MAC_IDX:
			ret = hwaddr_aton2(str_element, rule->client_mac);

			if (ret < 0)
				goto end;

			rule->mask |= RULE_MAC;
			*integrity |= 1 << idx;
			break;
		case RULE_TCPIP_PRO_IDX:
			ret = find_ip_proto(str_element);
			if (ret < 0)
				goto end;
			rule->tcpip.protocol = ret;
			rule->mask |= RULE_TCPIP_PRO;
			*integrity |= 1 << idx;
			break;
		case RULE_TCPIP_DST_IP_IDX:
			ret = ipv4_aton(str_element, &rule->tcpip.dst_addr.v4_addr);
			if (ret < 0)
				goto end;

			/*firstly only support IPv4*/
			rule->tcpip.ip_ver = IP_V4;
			rule->mask |= RULE_TCPIP_DST_IP;
			*integrity |= 1 << idx;
			break;
		case RULE_TCPIP_SRC_IP_IDX:
			ret = ipv4_aton(str_element, &rule->tcpip.src_addr.v4_addr);
			if (ret < 0)
				goto end;
			
			/*firstly only support IPv4*/
			rule->tcpip.ip_ver = IP_V4;
			rule->mask |= RULE_TCPIP_SRC_IP;
			*integrity |= 1 << idx;
			break;
		case RULE_TCPIP_DST_PORT_IDX:
			ret = atoi(str_element);
			if (ret < 0)
				goto end;
			
			rule->tcpip.dst_port = ret;
			rule->mask |= RULE_TCPIP_DST_PORT;
			*integrity |= 1 << idx;
			break;
		case RULE_TCPIP_SRC_PORT_IDX:
			ret = atoi(str_element);
			if (ret < 0)
				goto end;

			rule->tcpip.src_port = ret;
			rule->mask |= RULE_TCPIP_SRC_PORT;
			*integrity |= 1 << idx;
			break;
		case RULE_OUTPUT:
			ret = atoi(str_element);
			if (ret < 0 || ret > 0x08) {
				ret = 0;
				goto end;
			}

			rule->rule_output = ret;
			*integrity |= 1 << idx;
			break;
		default:
			ret = -1;
			break;
	}
end:
	return ret;
}

struct sp_rule_element *create_sp_rule_from_regresult(char *content, regmatch_t matchptr[], const size_t nmatch)
{
	int i = 0, ret = 0, len = 0;;
	char buf[256] = {0x00};
	struct sp_rule rule;
	struct sp_rule_element *rule_ele = NULL;
	unsigned short integrity = 0;
	
	len = matchptr[0].rm_eo - matchptr[0].rm_so;

	if (len > sizeof(buf)) {
		debug(DEBUG_ERROR, SP_PREX"rule string length(%d) ecceds 256 bytes, drop it!\n", len);
		goto err;
	}
	os_memcpy(buf, content + matchptr[0].rm_so, len);
	os_memset(&rule, 0, sizeof(struct sp_rule));

	for (i = 1; i < nmatch; i++) {
		debug(DEBUG_TRACE, SP_PREX"rule=%s\n", buf);
		os_memset(buf, 0, sizeof(buf));
		len = matchptr[i].rm_eo - matchptr[i].rm_so;

		/*len==0 means this field is empty*/
		if (len == 0)
			continue;
		else if (len > sizeof(buf)) {
			debug(DEBUG_ERROR, SP_PREX"rule group[%d] length ecceds 256 bytes, drop it!\n", i);
			goto err;
		}
				
		os_memcpy(buf, content + matchptr[i].rm_so, len);
		ret = str_2_rule_element(buf, len, &rule, i, &integrity);
		if (ret < 0) {
			debug(DEBUG_ERROR, "invalid rule field %s\n", buf);
			goto err;
		}
	}
	
	rule.rule_type = RULE_TYPE_STATIC;
	
	if (integrity <= ((1 << RULE_INDEX_IDX) | (1 << RULE_OUTPUT))) {
		debug(DEBUG_ERROR, "incomplete rule %d %s integrity=%x\n", rule.index, rule.rule_name, integrity);
		goto err;
	}
	
	rule_ele = os_zalloc(sizeof(struct sp_rule_element));

	if (!rule_ele) {
		debug(DEBUG_ERROR, SP_PREX"failed to allocate rule_ele\n");
		goto err;
	}

	dl_list_init(&rule_ele->entry);
	rule_ele->rule = rule;
err:
	return rule_ele;
}

void show_rule_list_dptbl(struct dl_list *rule_list, struct sp_dscp_pcp_mapping_table *dptbl)
{
	struct sp_rule_element *r = NULL;
	int i = 0;

	if (rule_list) {
		debug(DEBUG_OFF, SP_PREX"rule_index\trule_name\tmask\ttype\tal_mac\t\tssid\tcli_mac\t\tproto\tdst_IP\t\tsrc_IP\t\tdst_port\tsrc_port\toutput\n")
		dl_list_for_each(r, rule_list, struct sp_rule_element, entry) {
			debug(DEBUG_OFF, SP_PREX"%d\t%s\t%04x\t%02x\t"MACSTR_RAW"\t%s\t"MACSTR_RAW"\t%d\t%d.%d.%d.%d\t%d.%d.%d.%d %d %d %d\n",
				r->rule.index, r->rule.rule_name, r->rule.mask, r->rule.rule_type,
				PRINT_MAC_RAW(r->rule.al_mac), r->rule.ssid, PRINT_MAC_RAW(r->rule.client_mac),
				r->rule.tcpip.protocol, PRINT_IP_RAW(r->rule.tcpip.dst_addr.v4_addr),
				PRINT_IP_RAW(r->rule.tcpip.src_addr.v4_addr), r->rule.tcpip.dst_port,
				r->rule.tcpip.src_port, r->rule.rule_output);
		}
	}
	if (dptbl && dptbl->enable) {
		debug(DEBUG_OFF, SP_PREX"DSCP_PCP_MAPPING_TBL=");
		for (i = 0; i < 64; i++) {
			printf("[%d]=0x%02x,", i, dptbl->dptbl[i]);
		}
		printf("\n");
	}
}

void insert_rule_to_rule_list(struct dl_list *rule_list, struct sp_rule_element *r)
{
	int found = 0, same = 0;
	struct sp_rule_element *prule = NULL;

	/*only static rule has order in list*/
	if (r->rule.rule_type == RULE_TYPE_STATIC) {
		/*search any rule index is greater than r->rule.index*/
		dl_list_for_each(prule, rule_list, struct sp_rule_element, entry) {
			/*if same rule id, drop the new one*/
			if (!os_memcmp(prule->rule.rule_identifier, r->rule.rule_identifier, 4)) {
				found = 1;
				same = 1;
				debug(DEBUG_ERROR, SP_PREX"same index or rule identifier, overwrite existing one\n");
				break;
			}
			if (prule->rule.index > r->rule.index) {
				found = 1;
				break;
			}
		}
	}

	if (found) {
		/*insert to list*/
		dl_list_add(prule->entry.prev, &r->entry);
		/*remove the old one*/
		if (same) {
			dl_list_del(&prule->entry);
			os_free(prule);
		}
	} else {
		if (dl_list_len(rule_list) < 255)
			dl_list_add_tail(rule_list, &r->entry);
		else {
			debug(DEBUG_ERROR, SP_PREX"cannot install rule since exceeds 255, rule index=%d\n", r->rule.index);
		}
	}

	return;
}

void remove_rule_from_rule_list(struct dl_list *list, unsigned char *rule_id,
	struct dl_list *list_del)
{
	struct sp_rule_element *r = NULL;

	dl_list_for_each(r, list, struct sp_rule_element, entry) {
		if (!os_memcmp(r->rule.rule_identifier, rule_id, 4)) {
			dl_list_del(&r->entry);
			/*add it to del list to do more things*/
			dl_list_add(list_del, &r->entry);
			break;
		}
	}
}

void sync_rule_between_list(struct dl_list *list_d, struct dl_list *list_s)
{
	struct sp_rule_element *r = NULL, *n = NULL;

	dl_list_for_each_safe(r, n, list_s, struct sp_rule_element, entry) {
		dl_list_del(&r->entry);
		if (dl_list_len(list_d) < 255)
			dl_list_add_tail(list_d, &r->entry);
		else {
			debug(DEBUG_ERROR, SP_PREX"cannot install rule since exceeds 255, rule index=%d\n",
				r->rule.index);
			os_free(r);
		}
	}
}

/*user may be confused about the logic of this function, so this function may
 *be abondoned.
 */
int reorder_rule_in_rule_list(struct dl_list *list, unsigned char org_idx,
	unsigned char new_idx)
{
	int ret = 0;
	int found = 0, check_same = 0;
	unsigned char idx = 0;

	struct sp_rule_element *r = NULL, *r_old = NULL;

	if (org_idx == new_idx)
		goto end;

	/*find original rule by org_idx*/
	dl_list_for_each(r, list, struct sp_rule_element, entry) {
		if (r->rule.index == org_idx) {
			found = 1;
			r_old = r;
			dl_list_del(&r->entry);			
			r_old->rule.index = new_idx;
			r_old->rule.rule_identifier[0] = new_idx;
			break;
		}
	}

	/*insert it into the new_idx*/
	if (found) {
		dl_list_for_each(r, list, struct sp_rule_element, entry) {
			if (r->rule.index >= new_idx) {
				dl_list_add(r->entry.prev, &r_old->entry);

				check_same = 1;
				break;
			} 
		}

		if (check_same) {
			dl_list_for_each(r, list, struct sp_rule_element, entry) {
				if (r->rule.index == idx) {
					r->rule.index++;
					r->rule.rule_identifier[0] = r->rule.index;
				}
				idx = r->rule.index;
			}
		} else {
			dl_list_add_tail(list, &r_old->entry);
		}
	} else {
		debug(DEBUG_ERROR, "no rule index equals to %d\n", org_idx);
	}

end:
	return ret;
}

int move_rule_in_rule_list(struct dl_list *list, unsigned char rule_idx, enum move_action action)
{
	int ret = 0;
	struct sp_rule_element *r = NULL, *r_t = NULL, tmp;

	dl_list_for_each(r, list, struct sp_rule_element, entry) {
		if (r->rule.index == rule_idx) {
			if (action == MOVE_FORWARD && r->entry.prev != list) {
				/*if move forward and current r is not the first one*/
				r_t = dl_list_entry(r->entry.prev, struct sp_rule_element, entry);
			} else if (action == MOVE_BACKWARD && r->entry.next != list) {
				/*if move backward and current r is not the last one*/
				r_t = dl_list_entry(r->entry.next, struct sp_rule_element, entry);
			}

			if (r_t) {
				/*fist swap the index and identifier*/
				tmp = *r;
				r->rule.index = r_t->rule.index;
				os_memcpy(r->rule.rule_identifier, r_t->rule.rule_identifier, 4);
				r_t->rule.index  = tmp.rule.index;
				os_memcpy(r_t->rule.rule_identifier, tmp.rule.rule_identifier, 4);

				/*then swap the whole thing include the index and identifier again*/
				tmp = *r;
				//r->conf_status = r_t->conf_status;
				r->conf_status = RULE_NOT_CONFIGED_2_MAPFILTER;
				r->rule = r_t->rule;
				//r_t->conf_status = tmp.conf_status;
				r_t->conf_status = RULE_NOT_CONFIGED_2_MAPFILTER;
				r_t->rule = tmp.rule;
			}
			break;
		}
	}

	return ret;
}

int remove_rule_from_rule_list_by_index(struct dl_list *list, unsigned char rule_idx)
{
	int ret = 0;
	struct sp_rule_element *r = NULL;

	dl_list_for_each(r, list, struct sp_rule_element, entry) {
		if (r->rule.index == rule_idx) {
			dl_list_del(&r->entry);
			os_free(r);
			break;
		}
	}

	return ret;
}

int get_rule_list(struct dl_list *list, unsigned char idx, char *str, int *len)
{
	int ret = 0, total_len = 0;
	struct sp_rule_element *r = NULL;

	
	dl_list_for_each(r, list, struct sp_rule_element, entry) {
		if (idx == r->rule.index || idx == 0) {
			ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, "%d.%s:[",
				r->rule.index, r->rule.rule_name);
			if (ret  < 0) 
				goto end;
			total_len += ret;

			/*al macc*/
			if (r->rule.mask & RULE_AL_MAC)
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, MACSTR_RAW"]+[",
					PRINT_MAC_RAW(r->rule.al_mac));
			else 
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, "]+[");
			
			if (ret  < 0) 
				goto end;
			total_len += ret;

			/*ssid*/
			if (r->rule.mask & RULE_SSID)
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, "%s]+[",
					r->rule.ssid);
			else 
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, "]+[");
			
			if (ret  < 0) 
				goto end;
			total_len += ret;

			/*client mac*/
			if (r->rule.mask & RULE_MAC)
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, MACSTR_RAW"]+[",
					PRINT_MAC_RAW(r->rule.client_mac));
			else 
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, "]+[");
			
			if (ret  < 0) 
				goto end;
			total_len += ret;

			/*protocol*/
			if (r->rule.mask & RULE_TCPIP_PRO)
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, "%s]+[",
					proto_2_str(r->rule.tcpip.protocol));
			else 
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, "]+[");
			
			if (ret  < 0) 
				goto end;
			total_len += ret;

			/*dst IP address*/
			if (r->rule.mask & RULE_TCPIP_DST_IP)
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, IPSTR_RAW"]+[",
					PRINT_IP_RAW(r->rule.tcpip.dst_addr.v4_addr));
			else 
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, "]+[");
			
			if (ret  < 0) 
				goto end;
			total_len += ret;

			/*src IP address*/
			if (r->rule.mask & RULE_TCPIP_SRC_IP)
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, IPSTR_RAW"]+[",
					PRINT_IP_RAW(r->rule.tcpip.src_addr.v4_addr));
			else 
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, "]+[");
			
			if (ret  < 0) 
				goto end;
			total_len += ret;

			/*dst port address*/
			if (r->rule.mask & RULE_TCPIP_DST_PORT)
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, "%d]+[",
					r->rule.tcpip.dst_port);
			else 
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, "]+[");
			
			if (ret  < 0) 
				goto end;
			total_len += ret;

			/*src port address*/
			if (r->rule.mask & RULE_TCPIP_SRC_PORT)
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, "%d]=",
					r->rule.tcpip.src_port);
			else 
				ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, "]=");
			
			if (ret  < 0) 
				goto end;
			total_len += ret;

			/*rule_output*/
			ret = os_snprintf(str + total_len, MAX_PKT_LEN - total_len, "%d\n",
				r->rule.rule_output);
			
			if (ret  < 0) 
				goto end;
			total_len += ret;

			if (idx == r->rule.index)
				break;
		}
	}

	*len = total_len;
end:
	return ret;
}

int clear_rule_list(struct dl_list *rule_list)
{
	int ret = 0;
	struct sp_rule_element *prule = NULL, *n = NULL;

	dl_list_for_each_safe(prule, n, rule_list, struct sp_rule_element, entry) {
		dl_list_del(&prule->entry);
		os_free(prule);
	}
	
	return ret;
}


int convert_str_2_dptbl(unsigned char *dp_tbl, char *str, int str_len)
{
	int i = 0, ret = 0, value = 0;

	if (str_len != 127) {
		debug(DEBUG_ERROR, SP_PREX"dptbl str_len(%d)!=127\n", str_len);
		ret = -1;
		goto err;
	}

	for (i = 0; i < 64; i++) {
		value = dec2num(*(str + i * 2));
		if (value < 0) {
			ret = -1;
			goto err;
		}
		dp_tbl[i] = value;
	}
		
err:
	return ret;
}

char *parse_dp_tble_from_str(struct sp_dscp_pcp_mapping_table *dp_tbl, char *str)
{
	regex_t preg; 
	char* pattern_format = "DSCP_PCP_MAPPING=([0-7\\.]{126}[0-7])";    
	char pattern[512] = {0x00};
	int success = 0, status = 0, ret = 0;
	char *pos_end = NULL;
	regmatch_t matchptr[2];
	const size_t nmatch = 2;

	success = regcomp(&preg, pattern_format, REG_EXTENDED|REG_ICASE);

	if (success) {
		debug(DEBUG_ERROR, SP_PREX"regex compile error\n");
		goto err;
	}
	
	status = regexec(&preg, str, nmatch, matchptr, 0);
	
	if (status == REG_NOMATCH) {
		/*regular static rule not match, do special rule matching*/
		debug(DEBUG_ERROR, SP_PREX"failed to reg match\n");
		goto err1;
	} else if (status == REG_OK) {
		pos_end = str + matchptr[0].rm_eo;
		memcpy(pattern, str + matchptr[1].rm_so, matchptr[1].rm_eo - matchptr[1].rm_so);
		ret = convert_str_2_dptbl(dp_tbl->dptbl, str + matchptr[1].rm_so,
			matchptr[1].rm_eo - matchptr[1].rm_so); 
		if (ret < 0)
			goto err1;

		dp_tbl->enable = 1;
	} else {
		os_memset(pattern, 0, sizeof(pattern));
		regerror(status, &preg, pattern, sizeof(pattern));
		debug(DEBUG_ERROR, SP_PREX"regex error:%s", pattern);
		goto err1;
	}
err1:
	regfree(&preg);
err:
	return pos_end;
}

char *parse_rule_from_str(struct dl_list *list, struct sp_dscp_pcp_mapping_table *dp_tbl, char *str)
{
	regex_t preg;
	const char *mac_pattern = "([0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2})";
	const char *ip_pattern = "([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})";
	char *pattern_format = "([0-9]+)\\.([^:\\+]*):\\[%s*\\]\\+\\[([^\\+]*)\\]\\+\\[%s*\\]\\+\\[([^\\+]*)\\]\\+\\[%s*\\]\\+\\[%s*\\]\\+\\[([0-9]{0,5})\\]\\+\\[([0-9]{0,5})\\]=([0-9])";
	char pattern[512] = {0x00};
	int success = 0, status = 0;
	char *pos_end = NULL;
	regmatch_t matchptr[RULE_REGEX_GROUP_NUMBER];
	const size_t nmatch = RULE_REGEX_GROUP_NUMBER;
	struct sp_rule_element *r = NULL;

	(void)os_snprintf(pattern, sizeof(pattern), pattern_format, mac_pattern, mac_pattern, ip_pattern, ip_pattern);

	success = regcomp(&preg, pattern, REG_EXTENDED|REG_ICASE);

	if (success) {
		debug(DEBUG_ERROR, SP_PREX"regex compile error\n");
		goto err;
	}

	status = regexec(&preg, str, nmatch, matchptr, 0);

	if (status == REG_NOMATCH) {
		/*regular static rule not match, do special rule matching*/
		pos_end = parse_dp_tble_from_str(dp_tbl, str);

		if (!pos_end) {
			debug(DEBUG_ERROR, SP_PREX"failed to reg match\n");
			goto err1;
		}
	} else if (status == REG_OK) {
		pos_end = str + matchptr[0].rm_eo;
		r = create_sp_rule_from_regresult(str, matchptr, nmatch);
		if (!r)
			goto err1;

		insert_rule_to_rule_list(list, r);
	} else {
		os_memset(pattern, 0, sizeof(pattern));
		regerror(status, &preg, pattern, sizeof(pattern));
		debug(DEBUG_ERROR, SP_PREX"regex error:%s", pattern);
		goto err1;
	}

err1:
	regfree(&preg);
err:
	return pos_end;
}

int sp_init_rule_from_profile(struct dl_list *list, struct sp_dscp_pcp_mapping_table *dp_tbl, char *path)
{
	FILE *f = NULL;
	int ret = 0;
	long fsize = 0, result = 0;
	char *content = NULL, *pos = NULL;

	f = fopen(path, "r");

	if (!f) {
		debug(DEBUG_ERROR, SP_PREX"failed to open sp rule profile\n");
		ret = -1;
		goto err;
	}

	if (fseek(f, 0, SEEK_END) != 0) {
		debug(DEBUG_ERROR, "[%d]fseek fail!", __LINE__);
		ret = -1;
		goto err1;
	}
	fsize = ftell(f);
	if (fsize < 0) {
		ret = -1;
		goto err1;
	}
	rewind(f);

	content = os_malloc(sizeof(char) * fsize);
	if (!content) {
		ret = -1;
		goto err1;
	}

	result = fread(content, 1, fsize, f);
	if (result != fsize) {
		debug(DEBUG_ERROR, SP_PREX"ERROR!result(%ld) != fsize(%ld)", result, fsize);
		ret = -1;
		os_free(content);
		goto err1;
	}

	content[fsize - 1] = '\0';
	pos = content;
	dp_tbl->enable = 0;

	while((pos = parse_rule_from_str(list, dp_tbl, pos))) {
		;
	}

	os_free(content);
err1:
	(void)fclose(f);
err:
	return ret;
}

int sp_rule_write_back_2_profile(struct dl_list *list, struct sp_dscp_pcp_mapping_table *dp_tbl, char *path)
{
	FILE *f = NULL;
	int ret = 0, i = 0, len = 0;
	char *rule_str = NULL;

	f = fopen(path, "w");

	if (!f) {
		debug(DEBUG_ERROR, SP_PREX"failed to open sp rule profile\n");
		ret = -1;
		goto err;
	}

	rule_str = os_zalloc(MAX_PKT_LEN);
	if (!rule_str) {
		debug(DEBUG_ERROR, SP_PREX"failed to allocate rule_str\n");
		ret = -1;
		goto err1;
	}
		
	ret = get_rule_list(list, 0, rule_str, &len);
	if (ret < 0) {
		debug(DEBUG_ERROR, SP_PREX"failed to create rule string\n");
		ret = -1;
		os_free(rule_str);
		goto err1;
	}

	if (fprintf(f, "%s", rule_str) < 0) {
		debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
		ret = -1;
		os_free(rule_str);
		goto err1;
	}

	if (dp_tbl->enable) {
		if (fputs("DSCP_PCP_MAPPING=", f) == EOF) {
			debug(DEBUG_ERROR, "[%d]fputs fail!\n", __LINE__);
			ret = -1;
			os_free(rule_str);
			goto err1;
		}
		for (i = 0; i < 63; i++) {
			if (fprintf(f, "%d.", dp_tbl->dptbl[i]) < 0) {
				debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
				ret = -1;
				os_free(rule_str);
				goto err1;
			}
		}
		if (fprintf(f, "%d\n", dp_tbl->dptbl[63]) < 0) {
			debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
			ret = -1;
			os_free(rule_str);
			goto err1;
		}
	}

	os_free(rule_str);
err1:
	(void)fclose(f);
err:
	return ret;
}

int service_prioritization_init(void *context)
{
	int ret = 0;
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)context;
	dl_list_init(&ctx->sp_rule_static_list);
	dl_list_init(&ctx->sp_rule_dynamic_list);
	dl_list_init(&ctx->sp_rule_del_list);
	dl_list_init(&ctx->sp_rule_learn_complete_list);

	/*only init static rule list from profile*/
	if (ctx->role == CONTROLLER)
		sp_init_rule_from_profile(&ctx->sp_rule_static_list, &ctx->sp_dp_table, SP_RULE_FILE_PATH);

	sp_rule_config_done_action(context);

	return ret;
}

int service_prioritization_deinit(void *context)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)context;

	/*1. clear rule lists*/
	clear_rule_list(&ctx->sp_rule_static_list);
	clear_rule_list(&ctx->sp_rule_dynamic_list);
	clear_rule_list(&ctx->sp_rule_del_list);
	clear_rule_list(&ctx->sp_rule_learn_complete_list);

	/*2. clear rule list of mapfilter*/
	clear_sp_rules_of_local(&ctx->sp_rule_static_list, &ctx->sp_rule_dynamic_list);

	return 0;
}

#define INDEX_2_PRECEDENCE(idx) (255 - (idx))
#define PRECEDENCE_2_INDEX(pre)	(255 - (pre))

unsigned short append_sp_rule_tlv(struct sp_rule_element *r, unsigned char *tlv,
	enum rule_action action, unsigned char *peer_al_mac)
{
	unsigned char p1905_multicast_address[6]
						= { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x13 };

    unsigned short total_length = 0;

	/*tlv type*/
    *tlv++ = SERVICE_PRIORITIZATION_TULE_TYPE;
    total_length += 1;

	/*tlv length*/
	*(unsigned short *)(tlv) = host_to_be16(SP_RULE_TLV_LENGTH);
	tlv += 2;
    total_length += 2;

	/*field:rule identifier*/
	os_memcpy(tlv, r->rule.rule_identifier, 4);
	tlv += 4;

	/*field:Add Remove*/
	if (action == ADD_RULE)
		*tlv |= 0x80;
	else
		*tlv &= 0x7f;
	tlv++;
	
	/*field:Precedence*/
	*tlv++ = INDEX_2_PRECEDENCE(r->rule.index);

	/*field:Output*/
	*tlv++ = r->rule.rule_output;

	/*field:Always Match*/
	/*if rule almac equals to peer al mac, it means Always Match flag is set*/
	if (!os_memcmp(peer_al_mac, r->rule.al_mac, ETH_ALEN) ||
		!os_memcmp(p1905_multicast_address, r->rule.al_mac, ETH_ALEN))
		*tlv |= SP_RULE_ALWAYS_MATCH;
	else
		*tlv &= ~SP_RULE_ALWAYS_MATCH;
	tlv++;
	
	total_length += 8;

    return total_length;
}

unsigned short append_dscp_mapping_table_tlv(unsigned char *dptbl, unsigned char *tlv)
{
    unsigned short total_length = 0;

	/*tlv type*/
    *tlv++ = DSCP_MAPPING_TABLE_TYPE;
    total_length += 1;

	/*tlv length*/
	*(unsigned short *)(tlv) = host_to_be16(SP_DSCP_MAPPING_TABLE_LENGTH);
	tlv += 2;
    total_length += 2;

	/*field:DSCP PCP mapping*/
	os_memcpy(tlv, dptbl, 64);
	tlv += 64;

	total_length += 64;

    return total_length;
}

unsigned short append_sp_vendor_rule_tlv(struct sp_rule_element *r, unsigned char *tlv)
{
    unsigned short total_length = 0, len = 0;
	/*1905 internal using vendor specific tlv format
	   type			1 byte			0x0b
	   length			2 bytes			0x, 0x
	   oui			3 bytes			0x00, 0x0C, 0xE7
	   function type	1 byte			0xff
	   suboui			3 bytes			0x00, 0x0C, 0xE7
	   sub func type	1 byte			0x
	*/
	unsigned char *p = NULL;
	unsigned char mtk_oui[3] = {0x00, 0x0C, 0xE7};

	/*tlv type*/
    *tlv++ = VENDOR_SPECIFIC_TLV_TYPE;
	
	/*tlv length*/
	p = tlv;
	tlv += 2;

	/*field:oui*/
	os_memcpy(tlv, mtk_oui, 3);
	tlv += 3;

	/*field:function type*/
	*tlv++ = 0xff;

	/*field:suboui*/
	os_memcpy(tlv, mtk_oui, 3);
	tlv += 3;

	/*field:subfunction type*/
	*tlv++ = service_prioritization_vendor_rule_message_subtype;

	/*field:setting rule*/
	*tlv++ = SP_VENDOR_TYPE_RULE;
	
	/*field:rule type*/
	*tlv++ = r->rule.rule_type;

	/*field:rule index*/
	*tlv++ = r->rule.index;

	/*field:rule output*/
	*tlv++ = r->rule.rule_output;

	/*field:rule name length*/
	*tlv++ = r->rule.name_len;

	/*filed:rule name*/
	os_memcpy(tlv, r->rule.rule_name, r->rule.name_len);
	tlv += r->rule.name_len;
	
	/*field:rule mask*/
	*((unsigned short *)tlv) = host_to_be16(r->rule.mask);
	tlv += 2;

	if (r->rule.mask & RULE_AL_MAC) {
		os_memcpy(tlv, r->rule.al_mac, ETH_ALEN);
		tlv += ETH_ALEN;
	}

	if (r->rule.mask & RULE_SSID) {
		*tlv++ = r->rule.ssid_len;
		os_memcpy(tlv, r->rule.ssid, r->rule.ssid_len);
		tlv += r->rule.ssid_len;
	}

	if (r->rule.mask & RULE_MAC) {
		os_memcpy(tlv, r->rule.client_mac, ETH_ALEN);
		tlv += ETH_ALEN;
	}

	if (r->rule.mask & RULE_TCPIP_PRO) {
		*tlv++ = r->rule.tcpip.protocol;
	}

	if (r->rule.mask & (RULE_TCPIP_DST_IP | RULE_TCPIP_SRC_IP)) {
		*tlv++ = r->rule.tcpip.ip_ver;
	}

	if (r->rule.mask & RULE_TCPIP_DST_IP) {
		/*fisrt support IPv4*/
		if (r->rule.tcpip.ip_ver == 0x04) {
			*((unsigned int*)tlv) = host_to_be32(r->rule.tcpip.dst_addr.v4_addr);
			tlv += 4;
		} else if (r->rule.tcpip.ip_ver == 0x06) {
			os_memcpy(tlv, r->rule.tcpip.dst_addr.v6_addr, 16);
			tlv += 16;
		}
	}

	if (r->rule.mask & RULE_TCPIP_DST_PORT) {
		*((unsigned short*)tlv) = host_to_be16(r->rule.tcpip.dst_port);
		tlv += 2;
	}

	if (r->rule.mask & RULE_TCPIP_SRC_IP) {
		/*fisrt support IPv4*/
		if (r->rule.tcpip.ip_ver == 0x04) {
			*((unsigned int*)tlv) = host_to_be32(r->rule.tcpip.src_addr.v4_addr);
			tlv += 4;
		} else if (r->rule.tcpip.ip_ver == 0x06) {
			os_memcpy(tlv, r->rule.tcpip.src_addr.v6_addr, 16);
			tlv += 16;
		}
	}

	if (r->rule.mask & RULE_TCPIP_SRC_PORT) {
		*((unsigned short*)tlv) = host_to_be16(r->rule.tcpip.src_port);
		tlv += 2;
	}

	len = (unsigned short)(tlv - p) - 2;

	*((unsigned short*)p) = host_to_be16(len);
	total_length = len + 3;
	
    return total_length;
}

unsigned short append_sp_vendor_clear_rule_tlv(unsigned char *tlv)
{
	unsigned short total_length = 0;
	/*1905 internal using vendor specific tlv format
	   type 		1 byte			0x0b
	   length			2 bytes 		0x, 0x
	   oui			3 bytes 		0x00, 0x0C, 0xE7
	   function type	1 byte			0xff
	   suboui			3 bytes 		0x00, 0x0C, 0xE7
	   sub func type	1 byte			0x
	*/
	unsigned char mtk_oui[3] = {0x00, 0x0C, 0xE7};

	/*tlv type*/
	*tlv++ = 0x0b;
	
	/*tlv length*/
	*((unsigned short *)tlv) = host_to_be16(0x09);
	tlv += 2;

	/*field:oui*/
	os_memcpy(tlv, mtk_oui, 3);
	tlv += 3;

	/*field:function type*/
	*tlv++ = 0xff;

	/*field:suboui*/
	os_memcpy(tlv, mtk_oui, 3);
	tlv += 3;

	/*field:subfunction type*/
	*tlv++ = service_prioritization_vendor_rule_message_subtype;

	/*field:clear sp rule*/
	*tlv++ = SP_VENDOR_TYPE_CLEAR_RULE;
	total_length = 12;
	
	return total_length;
}

unsigned short create_sp_request_message(
	unsigned char *tlv_buf,
	unsigned char *buf, struct dl_list *static_list,
	struct dl_list *dynamic_list, struct dl_list *del_list, struct sp_dscp_pcp_mapping_table *dp_tbl,
	enum rule_action action, unsigned char *peer_al_mac)
{
	unsigned char *tlv_temp_buf = tlv_buf;
	cmdu_message_header *msg_hdr;
	unsigned short length = 0;
	unsigned short total_tlvs_length = 0;
	struct sp_rule_element *r = NULL;
	
    msg_hdr = (cmdu_message_header*)buf;

	if (action == ADD_RULE) {
		if (static_list) {
			dl_list_for_each(r, static_list, struct sp_rule_element, entry) {
				/*for standard rule, add standard sp rule tlv in order to be compatible with other vendor product*/
				if (STANDARD_SP_RULE(&(r->rule))) {
					length = append_sp_rule_tlv(r,tlv_temp_buf, action, peer_al_mac);	/*standard sp rule tlv*/
					total_tlvs_length += length;
		    		tlv_temp_buf += length;
				}
				/*add sp vendor rule tlv for both standard and vendor rule*/
				length = append_sp_vendor_rule_tlv(r, tlv_temp_buf);	/*mtk vendor sp rule tlv*/
				total_tlvs_length += length;
		    	tlv_temp_buf += length;
			}
		}
		if (dp_tbl && dp_tbl->enable) {
			length = append_dscp_mapping_table_tlv(dp_tbl->dptbl, tlv_temp_buf);
			total_tlvs_length += length;
	    	tlv_temp_buf += length;
		}

		/*add dynamic sp vendor rule*/
		if (dynamic_list) {
			dl_list_for_each(r, dynamic_list, struct sp_rule_element, entry) {
				/*add sp vendor rule tlv for both standard and vendor rule*/
				length = append_sp_vendor_rule_tlv(r, tlv_temp_buf);	/*mtk vendor sp rule tlv*/
				total_tlvs_length += length;
		    	tlv_temp_buf += length;
			}
		}
	} else {
		if (del_list) {
			dl_list_for_each(r, del_list, struct sp_rule_element, entry) {
				if (STANDARD_SP_RULE(&(r->rule))) {
					length = append_sp_rule_tlv(r,tlv_temp_buf, action, peer_al_mac);	/*standard sp rule tlv*/
					total_tlvs_length += length;
		    		tlv_temp_buf += length;
				}
			}
		}
		length = append_sp_vendor_clear_rule_tlv(tlv_temp_buf);
		total_tlvs_length += length;
		tlv_temp_buf += length;
	}
	
    length = append_end_of_tlv(tlv_temp_buf);
    total_tlvs_length += length;
    tlv_temp_buf += length;

    /*tlvs size is less than (46(minimun ethernet frame payload size)
     *-8(cmdu header size)) ==>padding
     */
    if(total_tlvs_length < MIN_TLVS_LENGTH)
    {
	    memset(tlv_temp_buf, 0, (MIN_TLVS_LENGTH - total_tlvs_length));
	    total_tlvs_length = MIN_TLVS_LENGTH;
    }

    //0x00: for this version of the specification
    //0x01~0xFF: Reserved Values
    msg_hdr->message_version = 0x0;

    //set reserve field to 0
    msg_hdr->reserved_field_0 = 0x0;
    msg_hdr->message_type = host_to_be16(SERVICE_PRIORITIZATION_REQUEST);
    msg_hdr->relay_indicator = 0x0;
    //set reserve field to 0
    msg_hdr->reserve_field_1 = 0x0;

    return total_tlvs_length;
}

int parse_sp_rule_tlv(unsigned char *tlv, unsigned short len,
	unsigned char *own_almac, struct dl_list *list, struct dl_list *del_list)
{
	unsigned short left = len;
	unsigned char rule_id[4] = {0x00};
	unsigned char action = 0;
	unsigned char precedence = 0, output = 0, always_match = 0;
	struct sp_rule_element *r = NULL;
	struct dl_list *target_list = NULL;

	if (left < 8) {
		debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
			__LINE__, left, 1);
		return -1;
	}

	os_memcpy(rule_id, tlv, 4);
	tlv += 4;

	action = *tlv & 0x80 ? ADD_RULE : REMOVE_RULE;
	tlv++;

	precedence = *tlv++;
	output = *tlv++;
	always_match = *tlv++;

	/*do not install this rule*/
	if (!always_match)
		goto err;

	r = (struct sp_rule_element *)os_zalloc(sizeof(struct sp_rule_element));
	
	if (!r) {
		debug(DEBUG_ERROR, SP_PREX"failed to allocate memory for sp_rule_element\n");
		goto err;
	}
	os_memcpy(r->rule.rule_identifier, rule_id, 4);
	if (action == ADD_RULE) {
		r->rule.rule_type = RULE_TYPE_STATIC;
		r->rule.index = PRECEDENCE_2_INDEX(precedence);
		os_memcpy(r->rule.al_mac, own_almac, ETH_ALEN);
		r->rule.rule_output = output;
		r->rule.mask |= RULE_AL_MAC;
		target_list = list;
	} else
		target_list = del_list;

	insert_rule_to_rule_list(target_list, r);

	return 0;
err:
	return -1;
}

int parse_sp_dscp_mapping_tbl_tlv(unsigned char *tlv,
	unsigned short len, struct sp_dscp_pcp_mapping_table *dptbl)
{
	if (len < SP_DSCP_MAPPING_TABLE_LENGTH) {
		debug(DEBUG_ERROR, "[%d] Error left_tlv_length %d less than %d\n",
			  __LINE__, len, SP_DSCP_MAPPING_TABLE_LENGTH);
		return -1;
	}

	os_memcpy(dptbl->dptbl, tlv, SP_DSCP_MAPPING_TABLE_LENGTH);

	dptbl->enable = 1;

	return 0;
}


/*check*/
int check_sp_vendor_tlvs(unsigned char *tlv, unsigned char *sp_action_type)
{
	int ret = 0;
	unsigned char mtk_oui[3] = {0x00, 0x0C, 0xE7};

	if (os_memcmp(mtk_oui, tlv, 3)) {
		ret = -1;
		goto err;	/*it is not the mtk vendor ie*/
	}
	tlv += 3;

	if (*tlv++ != 0xff) {
		ret = -1;
		goto err;	/*it is not 1905daemon vendor function type*/
	}
	
	if (os_memcmp(mtk_oui, tlv, 3)) {
		ret = -1;
		goto err;	/*it is not the mtk sub vendor ie*/
	}
	tlv += 3;

	if (*tlv++ != service_prioritization_vendor_rule_message_subtype) {
		ret = -1;
		goto err;	/*it is not the sp vendor rule*/
	}

	*sp_action_type = *tlv;

err:
	return ret;
}

int parse_sp_vendor_rule_tlv(unsigned char *buf, unsigned short len,
	struct dl_list *static_list, struct dl_list *dynamic_list)
{
	unsigned short left = len;
	unsigned char *tlv = buf;
	struct sp_rule_element *r = NULL;
	struct dl_list *target_list = NULL;

	r = (struct sp_rule_element *)os_zalloc(sizeof(struct sp_rule_element));

	if (!r) {
		debug(DEBUG_ERROR, SP_PREX"failed to allocate memory\n");
		goto err;
	}

	if (left < 4) {
		debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
			__LINE__, left, 4);
		goto err;
	}
	r->rule.rule_type = *tlv++;
	r->rule.index = *tlv++;
	r->rule.rule_identifier[0] = r->rule.index;
	r->rule.rule_output = *tlv++;

	r->rule.name_len = *tlv++;
	left -= 4;
	if (r->rule.name_len > RULE_NAME_SIZE) {
		debug(DEBUG_ERROR, SP_PREX"rule name length(%d) > 32", r->rule.name_len);
		goto err;
	}


	if (left < (r->rule.name_len + 2)) {
		debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
			__LINE__, left, (int)(r->rule.name_len + 2));
		goto err;
	}
	os_memcpy(r->rule.rule_name, tlv, r->rule.name_len);
	tlv += r->rule.name_len;

	r->rule.mask = be_to_host16(*((unsigned short *)tlv));
	tlv += 2;
	left -= (r->rule.name_len + 2);

	if (r->rule.mask & RULE_AL_MAC) {
		if (left < ETH_ALEN) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
				__LINE__, left, ETH_ALEN);
			goto err;
		}
		os_memcpy(r->rule.al_mac, tlv, ETH_ALEN);
		tlv += ETH_ALEN;
		left -= ETH_ALEN;
	}

	if (r->rule.mask & RULE_SSID) {
		if (left < 1) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
				__LINE__, left, 1);
			goto err;
		}
		r->rule.ssid_len = *tlv++;
		left -= 1;
		if (r->rule.ssid_len >= MAX_SSID_LEN) {
			debug(DEBUG_ERROR, SP_PREX"ssid length(%d) > 32", r->rule.ssid_len);
			goto err;
		}
		if (left < r->rule.ssid_len) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
				__LINE__, left, r->rule.ssid_len);
			goto err;
		}
		os_memcpy(r->rule.ssid, tlv, r->rule.ssid_len);
		tlv += r->rule.ssid_len;
		left -= r->rule.ssid_len;
	}

	if (r->rule.mask & RULE_MAC) {
		if (left < ETH_ALEN) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
				__LINE__, left, ETH_ALEN);
			goto err;
		}
		os_memcpy(r->rule.client_mac, tlv, ETH_ALEN);
		tlv += ETH_ALEN;
		left -= ETH_ALEN;
	}

	if (r->rule.mask & RULE_TCPIP_PRO) {
		if (left < 1) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
				__LINE__, left, 1);
			goto err;
		}
		r->rule.tcpip.protocol = *tlv++;
		left -= 1;
	}

	if (r->rule.mask & (RULE_TCPIP_DST_IP | RULE_TCPIP_SRC_IP)) {
		if (left < 1) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
				__LINE__, left, 1);
			goto err;
		}
		r->rule.tcpip.ip_ver = *tlv++;
		left -= 1;
	}

	if (r->rule.mask & RULE_TCPIP_DST_IP) {
		if (r->rule.tcpip.ip_ver == 0x04) {
			if (left < 4) {
				debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
					__LINE__, left, 4);
				goto err;
			}
			r->rule.tcpip.dst_addr.v4_addr = be_to_host32(*((unsigned int*)tlv));
			tlv += 4;
			left -= 4;
		} else if (r->rule.tcpip.ip_ver == 0x06) {
			if (left < 16) {
				debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
					__LINE__, left, 4);
				goto err;
			}
			os_memcpy(r->rule.tcpip.dst_addr.v6_addr, tlv, 16);
			tlv += 16;
			left -= 16;
		}
	}

	if (r->rule.mask & RULE_TCPIP_DST_PORT) {
		if (left < 2) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
				__LINE__, left, 2);
			goto err;
		}
		r->rule.tcpip.dst_port = be_to_host16(*((unsigned short*)tlv));
		tlv += 2;
		left -= 2;
	}

	if (r->rule.mask & RULE_TCPIP_SRC_IP) {
		if (r->rule.tcpip.ip_ver == 0x04) {
			if (left < 4) {
				debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
					__LINE__, left, 4);
				goto err;
			}
			r->rule.tcpip.src_addr.v4_addr = be_to_host32(*((unsigned int*)tlv));
			tlv += 4;
			left -= 4;
		} else if (r->rule.tcpip.ip_ver == 0x06) {
			if (left < 16) {
				debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
					__LINE__, left, 16);
				goto err;
			}
			os_memcpy(r->rule.tcpip.src_addr.v6_addr, tlv, 16);
			tlv += 16;
			left -= 16;
		}
	}

	if (r->rule.mask & RULE_TCPIP_SRC_PORT) {
		if (left < 2) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
				__LINE__, left, 2);
			goto err;
		}
		r->rule.tcpip.src_port = be_to_host16(*((unsigned short*)tlv));
		tlv += 2;
		left -= 2;
	}
	target_list = r->rule.rule_type == RULE_TYPE_STATIC ? static_list : dynamic_list;
	insert_rule_to_rule_list(target_list, r);
	return 0;
err:
	if (r)
		os_free(r);

	return -1;
}

int parse_sp_vendor_tlv(unsigned char *tlv, unsigned short len,
	unsigned char *own_almac, unsigned int *sub_integrity,
	struct dl_list *static_list, struct dl_list *dynamic_list,
	struct dl_list *learn_complete_list)
{
	unsigned short left = len;
	/*1905 internal using vendor specific tlv format
	  type		   1 byte		   0x0b
	  length		   2 bytes		   0x, 0x
	  oui		   3 bytes		   0x00, 0x0C, 0xE7
	  function type    1 byte		   0xff
	  suboui		   3 bytes		   0x00, 0x0C, 0xE7
	  sub func type    1 byte		   0x
	*/
	unsigned char sp_action_type = 0;

	if (left < 8) {
		debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
			__LINE__, left, 1);
		return -1;
	}
	if (check_sp_vendor_tlvs(tlv, &sp_action_type) < 0) {
		goto err;	/*it is not the sp vendor tlv*/
	}

	tlv += 9;
	switch (sp_action_type) {
		case SP_VENDOR_TYPE_RULE:
			/*add sp vendor rule*/
			if (parse_sp_vendor_rule_tlv(tlv, len, static_list, dynamic_list) < 0)
				goto err;
			*sub_integrity |= BIT(0);
			break;
		case SP_VENDOR_TYPE_DYNAMIC_SWITCH:
			/*dynamic rule on/off switch*/
			*sub_integrity |= BIT(1);
			break;
		case SP_VENDOR_TYPE_CLEAR_RULE:
			/*clear sp vendor rule*/
			*sub_integrity |= BIT(2);
			break;
		case SP_VENDOR_TYPE_LEARNING_COMPELETE_NOTIFICATION:
			/*learning complete notification*/
			if (parse_sp_learn_complete_tlv(tlv, len, learn_complete_list) < 0)
				goto err;
			*sub_integrity |= BIT(3);
			break;
		default:
			break;
	}

	return 0;
err:
	return -1;
}

int parse_sp_request_message(struct dl_list *static_list, struct dl_list *dynamic_list,
	struct dl_list *del_list, struct dl_list *learn_complete_list,
	struct sp_dscp_pcp_mapping_table *dptbl, unsigned char *own_almac,
	unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	struct sp_dscp_pcp_mapping_table dptbl_tmp;
	unsigned int integrity = 0, sub_integrity = 0;
	struct dl_list static_list_tmp;
	struct dl_list dynamic_list_tmp;
	struct dl_list learn_complete_list_tmp;
	struct dl_list del_list_tmp;
	struct sp_rule_element *r = NULL, *n = NULL;
	int ret = 0;

	dl_list_init(&static_list_tmp);
	dl_list_init(&dynamic_list_tmp);
	dl_list_init(&learn_complete_list_tmp);
	dl_list_init(&del_list_tmp);
	os_memset(&dptbl_tmp, 0, sizeof(dptbl_tmp));

	type = buf;
	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = %d, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}

		if (*type == SERVICE_PRIORITIZATION_TULE_TYPE) {
			ret = parse_sp_rule_tlv(value, len,
				own_almac, &static_list_tmp, &del_list_tmp);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error! parse sp rule tlv\n");
				goto error;
			}

			integrity |= BIT(0);
		} else if (*type == VENDOR_SPECIFIC_TLV_TYPE) {
			ret = parse_sp_vendor_tlv(value, len, own_almac, &sub_integrity,
				&static_list_tmp, &dynamic_list_tmp, &learn_complete_list_tmp);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error! parse sp vendor tlv\n");
				goto error;
			}

			integrity |= BIT(1);
		} else if (*type == DSCP_MAPPING_TABLE_TYPE) {
			ret = parse_sp_dscp_mapping_tbl_tlv(value, len, &dptbl_tmp);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error! parse sp dscp mapping table tlv\n");
				goto error;
			}

			integrity |= BIT(2);
		} else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	if (integrity & BIT(0)) {
		sync_rule_between_list(static_list, &static_list_tmp);

		dl_list_for_each_safe(r, n, &del_list_tmp, struct sp_rule_element, entry) {
			remove_rule_from_rule_list(static_list, r->rule.rule_identifier, del_list);
		}
	}
	if (integrity & BIT(1)) {
		if (sub_integrity & BIT(2)) {
			clear_rule_list(static_list);
			clear_rule_list(dynamic_list);
		}
		if (sub_integrity & BIT(0)) {
			sync_rule_between_list(static_list, &static_list_tmp);
			sync_rule_between_list(dynamic_list, &dynamic_list_tmp);
		}
		if (sub_integrity & BIT(3))
			sync_rule_between_list(learn_complete_list, &learn_complete_list_tmp);
	}
	if (integrity & BIT(2))
		os_memcpy(dptbl, &dptbl_tmp, sizeof(struct sp_dscp_pcp_mapping_table));
	return 0;

error:
	clear_rule_list(&static_list_tmp);
	clear_rule_list(&dynamic_list_tmp);
	clear_rule_list(&learn_complete_list_tmp);
	clear_rule_list(&del_list_tmp);
	return -1;
}

/*to sync sp rules to other agents, now it is called by a timer on controller*/
void sp_rule_sync_handler(void *context, void* parent_leaf, void *current_leaf, void *data)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)context;
	struct leaf *cur_leaf = (struct leaf *)current_leaf;
	struct topology_response_db *rpdb = NULL;
	unsigned char *if_name = NULL;

	/*if sp rules has already synced to peer, do not sync again*/
	if (ctx->sp_req_conf.action == ADD_RULE) {
		if (cur_leaf->sp_rule_sync_status == RULE_SYNC)
			return;
	}
	rpdb = lookup_tprdb_by_almac(ctx, cur_leaf->al_mac);
	debug(DEBUG_ERROR, "sync sp rule to ("MACSTR")\n",
			PRINT_MAC(cur_leaf->al_mac));
	if (!rpdb) {
		debug(DEBUG_ERROR, "BUG here!!!! rpdb almac("MACSTR") not exist\n",
			PRINT_MAC(cur_leaf->al_mac));
		return;
	}

	debug(DEBUG_ERROR, "sync sp rule to ("MACSTR") profile=%02x\n",
		PRINT_MAC(cur_leaf->al_mac), rpdb->profile);

	if (rpdb->profile != MAP_PROFILE_R3)
		return;

	if_name = ctx->itf[rpdb->recv_ifid].if_name;

	insert_cmdu_txq(cur_leaf->al_mac, ctx->p1905_al_mac_addr,
		e_service_prioritization_request, ++ctx->mid, if_name, 0);
	process_cmdu_txq(ctx, ctx->trx_buf.buf);

	/*change its sync status*/
	if (ctx->sp_req_conf.action == ADD_RULE)
		cur_leaf->sp_rule_sync_status = RULE_SYNC;
	else if (ctx->sp_req_conf.action == REMOVE_RULE)
		cur_leaf->sp_rule_sync_status = RULE_UNSYNC;
}

/*only to sync sp learn complete rules to other agents, now it is called by a timer on controller*/
void sp_learn_complete_rule_sync_handler(void *context, void* parent_leaf, void *current_leaf,
	void *data)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)context;
	struct leaf *cur_leaf = (struct leaf *)current_leaf;
	struct topology_response_db *rpdb = NULL;
	unsigned char *if_name = NULL;

	rpdb = lookup_tprdb_by_almac(ctx, cur_leaf->al_mac);
	if (!rpdb) {
		debug(DEBUG_ERROR, "BUG here!!!! rpdb almac("MACSTR") not exist\n",
			PRINT_MAC(cur_leaf->al_mac));
		return;
	}
	if (rpdb->profile != MAP_PROFILE_R3)
		return;
	if_name = ctx->itf[rpdb->recv_ifid].if_name;

	insert_cmdu_txq(cur_leaf->al_mac, ctx->p1905_al_mac_addr,
		e_service_prioritization_request_4_learning_comp, ++ctx->mid, if_name, 0);
	process_cmdu_txq(ctx, ctx->trx_buf.buf);
}


/*to reconfig sp rules on controller*/
void sp_rule_config_done_action(void *context)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)context;

	/*1. delete all sp rule*/
	ctx->sp_req_conf.action = REMOVE_RULE;
	/*1.1 notify other agents to clear all rules*/
	trace_topology_tree_cb(ctx, ctx->root_leaf, (topology_tree_cb_func)sp_rule_sync_handler, NULL);
	/*1.2 clear own sp rules in mapfilter*/
	clear_sp_rules_of_local(&ctx->sp_rule_static_list, &ctx->sp_rule_dynamic_list);
	/*2. apply all sp_rules, note here just apply sp rules to local mapfilter
	 * in p1905_managerd_periodic, will sync sp rules to all other agnets
	 */
	ctx->sp_req_conf.action = ADD_RULE;
	apply_sp_rules_to_local(&ctx->sp_rule_static_list, &ctx->sp_rule_dynamic_list,
		ctx->sp_dp_table.dptbl);

	sp_rule_write_back_2_profile(&ctx->sp_rule_static_list, &ctx->sp_dp_table, SP_RULE_FILE_PATH);

}

int sp_process_1905_lib_cmd(void *context, unsigned char *rcv_buf,
	unsigned short len, unsigned char *reply_buf, int *replay_len)
{
	int ret = 0;
	struct sp_cmd *sp = (struct sp_cmd*)rcv_buf;
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)context;

	/*
	SP_CMD_ADD_RULE,
	SP_CMD_SET_RULE,
	SP_CMD_RM_RULE,
	SP_CMD_REORDER_RULE,
	SP_CMD_GET_RULE,
	SP_CMD_SET_DYNAMIC,
	SP_CMD_SET_DSCP_TBL,
	SP_CMD_CONFIG_DONE,
	*/

	if (ctx->role == AGENT && sp->cmd_id != SP_CMD_GET_RULE) {
		debug(DEBUG_ERROR, "agent cannot support current operation(%02x)\n", sp->cmd_id);
		*replay_len = 0;
		return 0;
	}

	switch(sp->cmd_id) {
		case SP_CMD_ADD_RULE:
			{
				if (parse_rule_from_str(&ctx->sp_rule_static_list, &ctx->sp_dp_table, (char *)sp->value)) {
					reply_buf[0] = 0x01; /*success*/
					debug(DEBUG_ERROR, "add rule successfully\n");
				} else {
					reply_buf[0] = 0x00; /*fail*/
					debug(DEBUG_ERROR, "fail to add rule\n");
				}

				*replay_len = 1;
			}
			//
			break;
		case SP_CMD_SET_RULE:
			{
				if (parse_rule_from_str(&ctx->sp_rule_static_list, &ctx->sp_dp_table, (char *)sp->value)) {
					debug(DEBUG_TRACE, "set rule successfully\n");
				} else {
					debug(DEBUG_ERROR, "fail to set rule\n");
				}
				*replay_len = 0;
			}
			break;
		case SP_CMD_RM_RULE:
			remove_rule_from_rule_list_by_index(&ctx->sp_rule_static_list, sp->value[0]);
			*replay_len = 0;
			break;
		case SP_CMD_REORDER_RULE:
			reorder_rule_in_rule_list(&ctx->sp_rule_static_list, sp->value[0], sp->value[1]);
			*replay_len = 0;
			break;
		case SP_CMD_MOVE_RULE:
			move_rule_in_rule_list(&ctx->sp_rule_static_list, sp->value[0], sp->value[1]);
			*replay_len = 0;
			break;
		case SP_CMD_GET_RULE:
			if (get_rule_list(&ctx->sp_rule_static_list, sp->value[0], (char *)reply_buf, replay_len) < 0) {
				debug(DEBUG_ERROR, "failed to get rule list\n");
				*replay_len = 0;
			}
			break;
		case SP_CMD_SET_DYNAMIC:
			break;
		case SP_CMD_SET_DSCP_TBL:
			{
				if (!parse_dp_tble_from_str(&ctx->sp_dp_table, (char *)sp->value)) {
					debug(DEBUG_ERROR, "failed to set dscp table\n");
				}
				*replay_len = 0;
			}
			break;
		case SP_CMD_CONFIG_DONE:
			{
				sp_rule_config_done_action((void*)ctx);
				*replay_len = 0;
			}
			break;
	}

	return ret;
}

struct sp_rule_element *sp_create_dynamic_rule_by_learn_complete_info(unsigned char up,
	unsigned char mask, unsigned char ip_ver, unsigned char *src_ip, unsigned char *dst_ip,
	unsigned short src_port, unsigned short dst_port)
{
	struct sp_rule_element *r = NULL;

	if (!mask) {
		debug(DEBUG_ERROR, SP_PREX"error!mask equals to 0 in learn complete event\n");
		goto end;
	}

	if (ip_ver != IP_V4 && ip_ver != IP_V6) {
		debug(DEBUG_ERROR, SP_PREX"invalid ip_ver=%02x\n", ip_ver);
		goto end;
	}

	r = (struct sp_rule_element *)os_zalloc(sizeof(struct sp_rule_element));

	if (!r) {
		debug(DEBUG_ERROR, SP_PREX"failed to allocate memory\n");
		goto end;
	}

	r->rule.rule_type = RULE_TYPE_DYNAMIC;
	r->rule.index = os_random() % 255;
	r->rule.rule_identifier[0] = r->rule.index;
	r->rule.rule_output = up;
	r->rule.tcpip.ip_ver = ip_ver;

	if (mask & BIT(0)) {
		r->rule.mask |= RULE_TCPIP_SRC_IP;
		if (ip_ver == IP_V4)
			r->rule.tcpip.src_addr.v4_addr = *((unsigned int*)src_ip);
		else if (ip_ver == IP_V6)
			os_memcpy(r->rule.tcpip.src_addr.v6_addr, src_ip, 16);
	}

	if (mask & BIT(1)) {
		r->rule.mask |= RULE_TCPIP_DST_IP;
		if (ip_ver == IP_V4)
			r->rule.tcpip.dst_addr.v4_addr = *((unsigned int*)dst_ip);
		else if (ip_ver == IP_V6)
			os_memcpy(r->rule.tcpip.dst_addr.v6_addr, dst_ip, 16);
	}

	if (mask & BIT(2)) {
		r->rule.mask |= RULE_TCPIP_SRC_PORT;
		r->rule.tcpip.src_port= src_port;
	}

	if (mask & BIT(3)) {
		r->rule.mask |= RULE_TCPIP_DST_PORT;
		r->rule.tcpip.dst_port= dst_port;
	}
end:
	return r;
}

unsigned short append_sp_learn_complete_tlv(unsigned char up,
	unsigned char mask, unsigned char ip_ver, unsigned char *src_ip, unsigned char *dst_ip,
	unsigned short src_port, unsigned short dst_port, unsigned char *tlv)
{
    unsigned short total_length = 0, len = 0;
	/*1905 internal using vendor specific tlv format
	   type			1 byte			0x0b
	   length			2 bytes			0x, 0x
	   oui			3 bytes			0x00, 0x0C, 0xE7
	   function type	1 byte			0xff
	   suboui			3 bytes			0x00, 0x0C, 0xE7
	   sub func type	1 byte			0x
	*/
	unsigned char *p = NULL;
	unsigned char mtk_oui[3] = {0x00, 0x0C, 0xE7};

	if (mask == 0)
		return 0;

	/*tlv type*/
	*tlv++ = VENDOR_SPECIFIC_TLV_TYPE;

	/*tlv length*/
	p = tlv;
	tlv += 2;

	/*field:oui*/
	os_memcpy(tlv, mtk_oui, 3);
	tlv += 3;

	/*field:function type*/
	*tlv++ = 0xff;

	/*field:suboui*/
	os_memcpy(tlv, mtk_oui, 3);
	tlv += 3;

	/*field:subfunction type*/
	*tlv++ = service_prioritization_vendor_rule_message_subtype;

	/*field:setting rule*/
	*tlv++ = SP_VENDOR_TYPE_LEARNING_COMPELETE_NOTIFICATION;

	/*field:user priority*/
	*tlv++ = up;

	/*field:mask*/
	*tlv++ = mask;

	/*field:ip version*/
	*tlv++ = ip_ver;

	if (mask & BIT(0)) {
		/*field:source ip*/
		if (ip_ver == IP_V4) {
			*((unsigned int*)tlv) = host_to_be32(*((unsigned int*)src_ip));
			tlv += 4;
		} else if (ip_ver == IP_V6) {
			os_memcpy(tlv, src_ip, 16);
			tlv += 16;
		}
	}

	if (mask & BIT(1)) {
		/*field:destination ip*/
		if (ip_ver == IP_V4) {
			*((unsigned int*)tlv) = host_to_be32(*((unsigned int*)dst_ip));
			tlv += 4;
		} else if (ip_ver == IP_V6) {
			os_memcpy(tlv, dst_ip, 16);
			tlv += 16;
		}
	}

	if (mask & BIT(2)) {
		/*field: dource port*/
		*((unsigned short*)tlv) = host_to_be16(src_port);
		tlv += 2;
	}

	if (mask & BIT(3)) {
		*((unsigned short*)tlv) = host_to_be16(dst_port);
		tlv += 2;
	}

	len = (unsigned short)(tlv - p) - 2;

	*((unsigned short*)p) = host_to_be16(len);
	total_length = len + 3;

    return total_length;
}

int parse_sp_learn_complete_tlv(
	unsigned char *buf, unsigned short len, struct dl_list *list)
{
	unsigned short left = len;
	unsigned char *tlv = buf;
	unsigned char up = 0, mask = 0;
	struct tcpip_5_tuple tuple;
	struct sp_rule_element *r = NULL;

	os_memset(&tuple, 0, sizeof(tuple));

	if (left < 3) {
		debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
			__LINE__, left, 3);
		return -1;
	}
	up = *tlv++;
	mask = *tlv++;
	tuple.ip_ver = *tlv++;
	left -= 3;

	if (mask == 0)
		return 0;

	if (mask & BIT(0)) {
		if (tuple.ip_ver == IP_V4) {
			if (left < 4) {
				debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
					__LINE__, left, 4);
				return -1;
			}
			tuple.src_addr.v4_addr = be_to_host32(*((unsigned int *)tlv));
			tlv += 4;
			left -= 4;
		} else if (tuple.ip_ver == IP_V6) {
			if (left < 16) {
				debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
					__LINE__, left, 16);
				return -1;
			}
			os_memcpy(tuple.src_addr.v6_addr, tlv, 16);
			tlv += 16;
			left -= 16;
		}
	}

	if (mask & BIT(1)) {
		if (tuple.ip_ver == IP_V4) {
			if (left < 4) {
				debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
					__LINE__, left, 4);
				return -1;
			}
			tuple.dst_addr.v4_addr = be_to_host32(*((unsigned int *)tlv));
			tlv += 4;
			left -= 4;
		} else if (tuple.ip_ver == IP_V6) {
			if (left < 16) {
				debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
					__LINE__, left, 16);
				return -1;
			}
			os_memcpy(tuple.dst_addr.v6_addr, tlv, 16);
			tlv += 16;
			left -= 16;
		}
	}

	if (mask & BIT(2)) {
		if (left < 2) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
				__LINE__, left, 2);
			return -1;
		}
		tuple.src_port = be_to_host16(*((unsigned short*)tlv));
		tlv += 2;
		left -= 2;
	}

	if (mask & BIT(3)) {
		if (left < 2) {
			debug(DEBUG_ERROR, "%d: error left %d less than %d\n",
				__LINE__, left, 2);
			return -1;
		}
		tuple.dst_port = be_to_host16(*((unsigned short*)tlv));
		tlv += 2;
		left -= 2;
	}

	r = sp_create_dynamic_rule_by_learn_complete_info(up, mask, tuple.ip_ver, (unsigned char *)&tuple.src_addr,
		(unsigned char *)&tuple.dst_addr, tuple.src_port, tuple.dst_port);
	if (r)
		insert_rule_to_rule_list(list, r);
	return 0;
}

int sp_process_learn_complete_event(unsigned char own_role, unsigned char *event_buf,
	struct dl_list *target_list, unsigned char *back_buf, unsigned short *back_buf_len)
{
	struct sp_learn_complete_event *comp_event = (struct sp_learn_complete_event*)event_buf;
	struct sp_rule_element *r = NULL;

	*back_buf_len = 0;
	if (own_role == CONTROLLER) {
		r = sp_create_dynamic_rule_by_learn_complete_info(comp_event->up, comp_event->mask,
			comp_event->ip_ver, (unsigned char*)&comp_event->src_addr, (unsigned char*)&comp_event->dst_addr, comp_event->pkt_src_port,
			comp_event->pkt_dst_port);
		if (r) {
			debug(DEBUG_ERROR, SP_PREX"create rule(%d) from learn complete event\n", r->rule.index);
			insert_rule_to_rule_list(target_list, r);
		}
	} else if (own_role == AGENT) {
		*back_buf_len = append_sp_learn_complete_tlv(comp_event->up, comp_event->mask,
			comp_event->ip_ver, (unsigned char*)&comp_event->src_addr, (unsigned char*)&comp_event->dst_addr, comp_event->pkt_src_port,
			comp_event->pkt_dst_port, back_buf);
	}

	return 0;
}

int sp_sync_learn_complete_rule(void *context)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)context;
	struct sp_rule_element *r = 0, *n = NULL;
	int ret = 0;
	int learn_complete_rule_num = 0;

	learn_complete_rule_num = dl_list_len(&ctx->sp_rule_learn_complete_list);
	if (learn_complete_rule_num) {
		debug(DEBUG_OFF, SP_PREX"lean complete rul number=%d, sync it now\n", learn_complete_rule_num);
		trace_topology_tree_cb(ctx, ctx->root_leaf, (topology_tree_cb_func)sp_learn_complete_rule_sync_handler, NULL);
		dl_list_for_each_safe(r, n, &ctx->sp_rule_learn_complete_list, struct sp_rule_element, entry) {
			/*1. remove rule from learn complete list*/
			dl_list_del(&r->entry);
			/*2. insert rule to dynamic rule list*/
			insert_rule_to_rule_list(&ctx->sp_rule_dynamic_list, r);
			ret = apply_sp_rules_to_local(&ctx->sp_rule_static_list,
				&ctx->sp_rule_dynamic_list, ctx->sp_dp_table.dptbl);
		}
	}

	return ret;
}
unsigned short get_standard_sp_rule(unsigned char *buf, unsigned short buf_len,
	struct dl_list *static_list, struct sp_dscp_pcp_mapping_table *dscp_tbl,
	enum rule_action action, unsigned char *peer_al_mac)
{
	struct sp_rule_element *r = NULL;
	unsigned char *pos = buf;
	unsigned short length = 0, total_length = 0;

	if (!buf || !static_list || (action != ADD_RULE)
		||!peer_al_mac || (buf_len < 1024) || !dscp_tbl)
		return 0;

	dl_list_for_each(r, static_list, struct sp_rule_element, entry) {
		/*for standard rule, add standard sp rule tlv in order to be compatible with other vendor product*/
		if (STANDARD_SP_RULE(&(r->rule))) {
			length = append_sp_rule_tlv(r, pos, action, peer_al_mac);	/*standard sp rule tlv*/
			total_length += length;
    		pos += length;
			if (total_length > 512) {
				debug(DEBUG_ERROR, "length for sp basic rule is too long!!! stop\n");
				return total_length;
			}
		}
	}

	if (dscp_tbl->enable) {
		length = append_dscp_mapping_table_tlv(dscp_tbl->dptbl, pos);
		total_length += length;
	}

	return total_length;

}

