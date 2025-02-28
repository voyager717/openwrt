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

#ifndef _MAPFILTER_IF_H
#define _MAPFILTER_IF_H

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

/*---------------------------------->
consistent in both kernel prog and user prog
*/

#define MAP_NETLINK 26
#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

enum DEVICE_TYPE {
	AP = 0,
	APCLI,
	ETH,
};

#define INF_UNKNOWN		0x00
#define INF_PRIMARY		0x01
#define INF_NONPRIMARY	0x02

#define _24G 0x01
#define _5GL 0x02
#define _5GH 0x04
#define _5G	0x06

enum MAP_NETLINK_EVENT_TYPE {
	UPDATE_MAP_NET_DEVICE = 0,
	SET_PRIMARY_INTERFACE,
	SET_UPLINK_PATH_ENTRY,
	DUMP_DEBUG_INFO,
	UPDATE_APCLI_LINK_STATUS,
	UPDATE_CHANNEL_UTILIZATION,
	DYNAMIC_LOAD_BALANCE,
	SET_DROP_SPECIFIC_IP_PACKETS_STATUS,
	SET_TRAFFIC_SEPARATION_DEFAULT_8021Q,
	SET_TRAFFIC_SEPARATION_POLICY,
	SET_TRANSPARENT_VID,
	UPDATE_CLIENT_VID,
	TRAFFIC_SEPARATION_ONOFF,
	WAN_TAG_FLAG_ON_DEV,	
	SHOW_VERSION,
	SERVICE_PRIORITIZATION_RULE,
	SERVICE_PRIORITIZATION_DSCP_MAPPING_TBL,
	SERVICE_PRIORITIZATION_CLEAR_RULE,
	SERVICE_PRIORITIZATION_REMOVE_ONE_RULE
};

struct GNU_PACKED map_netlink_message {
	unsigned char type;
	unsigned short len;
	unsigned char event[0];
};

struct GNU_PACKED local_interface {
	char name[IFNAMSIZ];
	unsigned char mac[ETH_ALEN];
	enum DEVICE_TYPE dev_type;
	unsigned char band;
};

struct GNU_PACKED local_itfs {
	unsigned char num;
	struct local_interface inf[0];
};

struct GNU_PACKED primary_itf_setting {
	struct local_interface inf;
	unsigned char primary;
};

struct GNU_PACKED up_link_path_setting {
	struct local_interface in;
	struct local_interface out;
};

struct GNU_PACKED apcli_link_status {
	struct local_interface in;
	unsigned char link_status;
};


#ifdef MAP_R2

#define STATION_JOIN 1
#define STATION_LEAVE 0

struct GNU_PACKED ssid_2_vid_mapping {
	char ssid[32];
	unsigned short vlan_id;
};

struct GNU_PACKED ts_policy {
	unsigned char num;
	struct ssid_2_vid_mapping ssid_2_vid[0];
};

struct GNU_PACKED ts_default_8021q {
	unsigned short primary_vid;
	unsigned char default_pcp;
};

struct GNU_PACKED transparent_vids {
	unsigned char num;
	unsigned short vids[0];
};

struct GNU_PACKED client_vid {
	unsigned char client_mac[ETH_ALEN];
	unsigned short vid;
	unsigned char status;
	unsigned char ssid_len;
	char ssid[33];
	unsigned al_mac[ETH_ALEN];
};

#endif

#ifdef MAP_R3_SP
#include "common.h"
#include "service_prioritization.h"

struct GNU_PACKED sp_rules_config {
	unsigned char number;
	struct sp_rule rules[0];
};

struct GNU_PACKED sp_dscp_mapping_tbl_config {
	unsigned char dptbl[64];
};
#endif

int mapfilter_netlink_init(unsigned int pid);
int mapfilter_netlink_deinit(int sock);
int mapfilter_set_all_interface(struct local_itfs *itf);
int mapfilter_set_primary_interface(struct local_interface *itf, unsigned char primary);
int mapfilter_set_uplink_path(struct local_interface *in, struct local_interface *out);
int mapfilter_dump_debug_info();
int mapfilter_ts_onoff(char onoff);
int mapfilter_show_version();
unsigned char *mapfilter_read_nlmsg(unsigned char *rx_buf, int buf_len, int *reply_len);
#ifdef MAP_R2
int mapfilter_set_ts_default_8021q(unsigned short primary_vid, unsigned char default_pcp);
int mapfilter_set_ts_policy(unsigned char num, struct ssid_2_vid_mapping *mapping);
int mapfilter_set_transparent_vid(unsigned short vids[], unsigned char vid_num);
int mapfilter_update_client_vid(unsigned char mac[], unsigned short vid,
	unsigned char status, unsigned char ssid_len, char *ssid, 
	unsigned char *al_mac);
int mapfilter_set_wan_tag(unsigned char *wan_mac, unsigned char status);
#endif
#ifdef MAP_R3_SP
int mapfilter_set_sp_rules(struct dl_list *static_list, struct dl_list *dynamic_list);
int mapfilter_set_sp_dscp_mapping_tbl(unsigned char *dptbl);
int mapfilter_set_sp_clear_rules();
int mapfilter_set_sp_rm_one_rule(struct sp_rule *rule);
#endif

#endif
