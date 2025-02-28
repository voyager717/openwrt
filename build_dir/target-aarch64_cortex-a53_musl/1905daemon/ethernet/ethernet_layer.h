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

#ifndef PLATFORM_UTILS_H
#define PLATFORM_UTILS_H

//#include "common.h"

#include "list.h"

#define IN
#define OUT

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#if defined (CONFIG_RALINK_MT7621)
#define MAX_LAN_PORT_NUM 5
#define MAX_PHY_PORT	5
#endif
#ifndef MAX_LAN_PORT_NUM
#define MAX_LAN_PORT_NUM 7
#endif

#ifndef MAX_PHY_PORT
#define MAX_PHY_PORT 7
#endif

#define ETH_SUCCESS					0
#define ETH_ERROR_FAIL				-1
#define ETH_ERROR_NOT_SUPPORT		-2

typedef void (*switch_port_status_change_handler)(void *context, void *data);

struct ethernet_client_entry {
	struct dl_list entry;
	unsigned char port_index;
	unsigned char mac[ETH_ALEN];
	unsigned char age;
};

struct ethernet_port {
	unsigned char port_index;
	int port_valid;
	unsigned char wan_port;
	struct dl_list clients;
};

/*the notifier stucture*/
struct ethernet_port_notifier {
	void (*up_handler)(void *context, void *data);
	void (*down_handler)(void *context, void *data);
};

/*switch port status structure*/
struct port_status_info {
    int port;		// port number
    int status;		//1up ,down
};

struct arl_setting {
	unsigned short lan_vid_num;
	unsigned short *lan_vid;
	unsigned short wan_vid_num;
	unsigned short *wan_vid;
	unsigned short cpu_port_num;
	unsigned short *cpu_port;
};

/*
 * This structure is registered by different switch chips
 * All the operations should return 0 on success, otherwise return -1.
 * @init: inialization procedure before any switch operations. Note that
 *	this operation should guarantee that current device would not foward any
 *	1905 neighbor multicast cmuds(dest mac as 01:80:c2:00:00:13).
 * @get_port_type: get the types of each switch port. @lport_map 
 * 	indicates the lan port index, e.g. 0x07=0B00001111 means port 0, 1, 2, 3
 *	are the lan port, @wport_map indicates the wan port index, e.g. 0x10=
 *	0x10=0B00010000 means port 4 is the wan port.
 * @get_table_entry: get all the entry from switch table. The variable 
 * 	@entry_list is the head of the entry list, the element of entry list should
 *	be the structure ethernet_client_entry. 
 * @search_table:  find the specific entry by mac address. This operation is
 *	optional.
 * @get_port_status: get the status of a switch port. @status should be 1
 *	if Link Up, 0 if Link Down
 * @deinit: deinitialization procedure.
*/

struct eth_ops {
	int (*init)();
	int (*get_port_type)(OUT unsigned int *lport_map, OUT unsigned int *wport_map);
	int (*get_table_entry)(OUT struct dl_list *entry_list);
	int (*search_table)(IN unsigned char *mac, OUT struct ethernet_client_entry *entry);
	int (*get_port_status)(IN unsigned char index, OUT int *status);
	int (*set_switch_ts_vlan)(unsigned short vid[], 
		unsigned short vid_cnt, unsigned char active);
	int (*set_switch_transparent_vlan)(unsigned short vid[], 
		unsigned short vid_cnt, unsigned char active);
	int (*clear_table)();
	int (*deinit)();
};

int eth_layer_init(void *context, struct ethernet_port_notifier *pnotifier);
void eth_layer_fini(void);
int eth_layer_port_client_update();
void eth_layer_port_clients_dump();
struct ethernet_client_entry* eth_layer_get_client_with_largest_age(struct ethernet_port *port);
struct ethernet_client_entry* eth_layer_get_client_by_mac(struct ethernet_port *port, unsigned char *mac);
int eth_layer_get_client_port_by_mac(unsigned char *mac);
int eth_layer_port_data_update_and_notify(void *context, void *data);
int eth_layer_update_eth_port_status(int port_index);
int eth_layer_get_eth_port_status(int port_index);
struct ethernet_port *eth_layer_get_eth_data_by_port_num(int port_num);
int eth_layer_set_traffic_separation_vlan(unsigned short vid[], unsigned char num);
int eth_layer_set_switch_trasnparent_vlan(unsigned short vid[], unsigned char num);

/*below functions are used to test the eth_ops functions, note that the eth_ops should be correctly registered before using*/
int test_eth_layer_get_port_type(void);
int test_eth_layer_get_client_entry(void);
int test_eth_layer_search_table_entry(unsigned char *mac);
int test_eth_layer_get_port_status(int port_index);
int eth_layer_clear_switch_table();
int test_eth_layer_set_vlan(unsigned short vid[], unsigned char vid_num,
	unsigned char type);
int test_eth_layer_clear_switch_table();

#endif
