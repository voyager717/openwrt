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
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <pthread.h>

#include "common.h"
#include "ethernet_layer.h"
#include "debug.h"
#include "netlink_event.h"


struct ethernet_port eth_data[MAX_LAN_PORT_NUM];
struct ethernet_port_notifier notifier;


#ifdef CONFIG_MASK_MACADDR
#define PRINT_MAC(a) a[0],a[3],a[4],a[5]
#else
#define PRINT_MAC(a) a[0],a[1],a[2],a[3],a[4],a[5]
#endif

struct eth_ops *_eth_ops = NULL;

#include "chips_comb.h"

static inline int eth_init(struct eth_ops *ops, void *context)
{
	int ret = ETH_ERROR_NOT_SUPPORT;

	if (ops && ops->init) {
		ret = ops->init();
		if (ret != ETH_SUCCESS)
			debug(DEBUG_ERROR, "fail to init\n");
	}
	return ret;
}

static inline int eth_deinit(struct eth_ops *ops, void *context)
{
	int ret = ETH_ERROR_NOT_SUPPORT;

	if (ops && ops->deinit) {
		ret = ops->deinit();
		if (ret != ETH_SUCCESS)
			debug(DEBUG_ERROR, "fail to deinit\n");
	}
	return ret;
}

static inline int eth_get_port_type(struct eth_ops *ops, unsigned int *lport_map, unsigned int *wport_map)
{
	int ret = ETH_ERROR_NOT_SUPPORT;

	if (ops && ops->get_port_type) {
		ret = ops->get_port_type(lport_map, wport_map);
		if (ret != ETH_SUCCESS)
			debug(DEBUG_ERROR, "fail to get_port_type\n");
	}
	return ret;
}

static inline int eth_get_table_entry(struct eth_ops *ops, struct dl_list *entry_list)
{
	int ret = ETH_ERROR_NOT_SUPPORT;

	if (ops && ops->get_table_entry) {
		ret = ops->get_table_entry(entry_list);
		if (ret != ETH_SUCCESS)
			debug(DEBUG_ERROR, "fail to get_table_entry\n");
	}
	return ret;
}

static inline int eth_search_table_entry(struct eth_ops *ops, unsigned char *mac, struct ethernet_client_entry *entry)
{
	int ret = ETH_ERROR_NOT_SUPPORT;

	if (ops && ops->search_table) {
		ret = ops->search_table(mac, entry);
		if (ret != ETH_SUCCESS)
			debug(DEBUG_ERROR, "fail to search_table mac("MACSTR")\n",
				PRINT_MAC(mac));
	}
	return ret;
}

static inline int eth_get_port_status(struct eth_ops *ops, unsigned char index, int *status)
{
	int ret = ETH_ERROR_NOT_SUPPORT;

	if (ops && ops->get_port_status) {
		ret = ops->get_port_status(index, status);
		if (ret != ETH_SUCCESS)
			debug(DEBUG_ERROR, "fail to get_port_status index(%d)\n", index);
	}
	return ret;
}

static inline int eth_clear_switch_table(struct eth_ops *ops)
{
	int ret = ETH_ERROR_NOT_SUPPORT;

	if (ops && ops->clear_table) {
		ret = ops->clear_table();
		if (ret != ETH_SUCCESS)
			debug(DEBUG_ERROR, "fail to clear switch table\n");
	}
	return ret;
}


int eth_layer_port_init(struct ethernet_port_notifier *pnotifier);

int eth_layer_init(void *context, struct ethernet_port_notifier *pnotifier)
{
	int ret = ETH_SUCCESS;

	ret = hook_eth_ops();
	if (ret == ETH_SUCCESS)
		ret = eth_init(_eth_ops, context);

	/*if eth init fail, unregister eth_ops*/
	if (ret == ETH_ERROR_FAIL) {
		_eth_ops = NULL;
		debug(DEBUG_ERROR, "ethernet layer function is not supported\n");
	}

	eth_layer_port_init(pnotifier);

	return ETH_SUCCESS;
}

void eth_layer_fini(void)
{
	eth_deinit(_eth_ops, NULL);
}

static inline void eth_layer_insert_entry(struct ethernet_port *port, struct ethernet_client_entry *client)
{
	dl_list_add(&port->clients, &client->entry);
}

static inline void eth_layer_port_clients_clear(struct ethernet_port *port)
{
	struct ethernet_client_entry *entry, *entry_next;

	dl_list_for_each_safe(entry, entry_next, &port->clients, struct ethernet_client_entry, entry) {
		dl_list_del(&entry->entry);
		os_free(entry);
	}
}

void eth_layer_port_clients_dump()
{
	int i = 0;
	struct ethernet_client_entry *entry = NULL;

	for(i = 0; i < MAX_LAN_PORT_NUM; i++) {
		printf("PORT %d :\n", eth_data[i].port_index);
		dl_list_for_each(entry, &eth_data[i].clients, struct ethernet_client_entry, entry) {
			printf("\t "MACSTR" %d\n", PRINT_MAC(entry->mac), entry->age);
		}
	}
}

int eth_layer_port_init(struct ethernet_port_notifier *pnotifier)
{
	int i = 0, j = 0, ret = ETH_SUCCESS;
	unsigned int lan_port = 0, wan_port = 0;
	int lan_finished = 0;


	debug(DEBUG_ERROR, "MAX_LAN_PORT_NUM=%d\n", MAX_LAN_PORT_NUM);

	for(i = 0; i < MAX_LAN_PORT_NUM; i++) {
		/*fisrtly assumes that the port is valid, for that not sure if the switch port plugs in or not when the 1905 daemon starts up*/
		eth_data[i].port_index = i;
		eth_data[i].port_valid = -1;
		debug(DEBUG_ERROR, "init eth_data[%d]=%d\n", i, i);
		dl_list_init(&eth_data[i].clients);
	}

	if (pnotifier != NULL) {
		notifier.up_handler = pnotifier->up_handler;
		notifier.down_handler = pnotifier->down_handler;
	}

	/*to get the correct lan port index for each eth_data*/
	ret = eth_get_port_type(_eth_ops, &lan_port, &wan_port);

	if (ret != ETH_SUCCESS) {
		goto end;
	}

	debug(DEBUG_ERROR, "lan_port=%08x, wan_port=%08x\n", lan_port, wan_port);
	//lan_port=0000004f wan_port=00000030
	for (i = 0; i < MAX_LAN_PORT_NUM; i++) {
		if (lan_port & (0x00000001 << i)) {
			eth_data[j].port_index = i;
			debug(DEBUG_ERROR, "assign eth_data[%d]=%d\n", j, i);
			j++;
			if (j >= MAX_LAN_PORT_NUM) {
				debug(DEBUG_ERROR, "lan_finished=1 i=%d j=%d\n", i, j);
				lan_finished = 1;
				break;
			}
		}
	}
	if (!lan_finished) {
		debug(DEBUG_ERROR, "lan_finished=0 j=%d\n", j);
		for (i = 0; i < MAX_LAN_PORT_NUM; i++) {
			if (wan_port & (0x00000001 << i)) {
				eth_data[j].port_index = i;
				eth_data[j].wan_port= 1;
				debug(DEBUG_ERROR, "assign eth_data[%d]=%d wan_port = true\n", j, i);
				j++;
				if (j >= MAX_LAN_PORT_NUM)
					break;
			}
		}
	}
end:
	return ret;
}

struct ethernet_client_entry* eth_layer_get_client_with_largest_age(struct ethernet_port *port)
{
	struct ethernet_client_entry *entry = NULL, *max = NULL;

	dl_list_for_each(entry, &port->clients, struct ethernet_client_entry, entry) {
		if (max == NULL)
			max = entry;
		if (max->age < entry->age)
			max = entry;
	}

	return max;
}

struct ethernet_client_entry* eth_layer_get_client_by_mac(struct ethernet_port *port, unsigned char *mac)
{
	struct ethernet_client_entry *entry = NULL;

	dl_list_for_each(entry, &port->clients, struct ethernet_client_entry, entry) {
		if (!os_memcmp(entry->mac, mac, ETH_ALEN))
			return entry;
	}

	return NULL;
}

/*get switch data by port index*/
struct ethernet_port *eth_layer_get_eth_data_by_port_index(int port_index)
{
	int i = 0;
	for (i = 0; i < MAX_LAN_PORT_NUM; i++) {
		if (eth_data[i].port_index == port_index)
			return &eth_data[i];
	}

	return NULL;
}

/*get switch data by port index*/
struct ethernet_port *eth_layer_get_eth_data_by_port_num(int port_num)
{
	if (port_num >= 0 && port_num < MAX_LAN_PORT_NUM)
		return &eth_data[port_num];

	return NULL;
}

/*get client switch port Fn*/
int eth_layer_get_client_port_by_mac(unsigned char *mac)
{
	int port = ETH_ERROR_NOT_SUPPORT, ret = ETH_SUCCESS, i = 0;
	struct ethernet_client_entry entry;
	struct ethernet_client_entry *client = NULL;
	struct ethernet_port *pport = NULL;

	debug(DEBUG_TRACE, "start to search entry\n");

	os_memset(&entry, 0, sizeof(struct ethernet_client_entry));
	ret = eth_search_table_entry(_eth_ops, mac, &entry);

	if (ret == ETH_SUCCESS) {
		pport = eth_layer_get_eth_data_by_port_index(entry.port_index);
		if (pport)
			port = pport->port_index;
		else
			port = ETH_ERROR_FAIL;
	} else if (ret == ETH_ERROR_FAIL) {
		port = ETH_ERROR_FAIL;
	} else if (ret == ETH_ERROR_NOT_SUPPORT) {
		ret = eth_layer_port_client_update();
		if (ret != ETH_SUCCESS) {
			port = ret;
			goto end;
		}
		debug(DEBUG_ERROR, "Quick switch table seraching not support, try to search by sofware\n");
		for (i = 0; i < MAX_LAN_PORT_NUM; i++) {
			client = eth_layer_get_client_by_mac(&eth_data[i], mac);
			if (client) {
				port = client->port_index;
				break;
			}
		}

		if (i >= MAX_LAN_PORT_NUM)
			port = ETH_ERROR_FAIL;
	}
	debug(DEBUG_TRACE, "search entry done\n");
end:
	return port;
}


int eth_layer_port_data_update_and_notify(void *context, void *data)
{
	struct port_status_info *port_status = (struct port_status_info *)data;
	int port_index = port_status->port;
	unsigned char changed = 0;
	struct ethernet_port *pport = NULL;

	pport = eth_layer_get_eth_data_by_port_index(port_index);
	if (pport == NULL) {
		debug(DEBUG_ERROR, "switch data not found by port index(%d)\n", port_index);
		return ETH_ERROR_FAIL;
	}

	if (pport->port_valid != port_status->status) {
		pport->port_valid = port_status->status;
		changed = 1;
	}

	debug(DEBUG_ERROR, TOPO_PREX"port [%d] %s\n", port_status->port, port_status->status == 1 ? "UP" : "DOWN");

	if (port_status->status == 0)
		eth_layer_port_clients_clear(pport);

	if (changed == 0)
		return 0;

	if (port_status->status == 0) {
		if (notifier.down_handler)
			notifier.down_handler(context, (void *)port_status);
	}
	else if (port_status->status == 1) {
		if (notifier.up_handler)
			notifier.up_handler(context, (void *)port_status);
	}
	return 0;
}

int eth_layer_get_eth_port_status(int port_index)
{
	if (port_index < 0 || port_index >= MAX_LAN_PORT_NUM)
		return -1;

	return eth_data[port_index].port_valid;
}

int eth_layer_update_eth_port_status(int port_index)
{
	int ret = 0, status = 0;

	ret = eth_get_port_status(_eth_ops, port_index, &status);
	if (!ret)
		return status;

	return ETH_ERROR_FAIL;
}

int eth_layer_set_traffic_separation_vlan(unsigned short vid[], unsigned char num)
{
	/*int ret = ETH_ERROR_NOT_SUPPORT;

	if (_eth_ops->set_switch_ts_vlan) {
		ret = _eth_ops->set_switch_ts_vlan(vid, num, 1);
		if (ret != ETH_SUCCESS) {
			debug(DEBUG_ERROR, "fail to set traffic separation vlan id to ethernet\n");
		}
	}

	return ret;*/
	return 0;
}

int eth_layer_set_switch_trasnparent_vlan(unsigned short vid[], unsigned char num)
{
	/*int ret = ETH_ERROR_NOT_SUPPORT;
	if ( _eth_ops->set_switch_transparent_vlan) {
		ret = _eth_ops->set_switch_transparent_vlan(vid, num, 1);
		if (ret != ETH_SUCCESS) {
			debug(DEBUG_ERROR, "fail to set transparent vlan id to ethernet\n");
		}
	}

	return ret;*/
	return 0;
}

int eth_layer_port_client_update()
{
	int ret = ETH_SUCCESS, i = 0;
	struct dl_list entry_list = {NULL, NULL};
	struct ethernet_client_entry *table_entry = NULL, *table_entry_next = NULL;
	struct ethernet_port *pport = NULL;

	debug(DEBUG_TRACE, "start to update entries\n");

	/*clear the original eth data*/
	for (i = 0; i < MAX_LAN_PORT_NUM; i++)
		eth_layer_port_clients_clear(&eth_data[i]);

	dl_list_init(&entry_list);
	ret = eth_get_table_entry(_eth_ops, &entry_list);

	if (ret != ETH_SUCCESS) {
		goto end;
	}

	/*update the eth data*/
	dl_list_for_each_safe(table_entry, table_entry_next, &entry_list, struct ethernet_client_entry, entry) {
		pport = eth_layer_get_eth_data_by_port_index(table_entry->port_index);
		if (pport && pport->port_valid == 1) {
			dl_list_del(&table_entry->entry);
			eth_layer_insert_entry(pport, table_entry);
		}
	}

	/*destory the rest entries in entry list*/
	dl_list_for_each_safe(table_entry, table_entry_next, &entry_list, struct ethernet_client_entry, entry) {
		dl_list_del(&table_entry->entry);
		os_free(table_entry);
	}
end:
	debug(DEBUG_TRACE, "updating entries done\n");
	return ret;
}

/*
below functions are used to test the eth_ops functions,
note that the eth_ops should be correctly registered before using
*/

int test_eth_layer_get_port_type(void)
{
	int ret = ETH_SUCCESS;
	unsigned int lan_port = 0, wan_port = 0;

	/*to get the correct lan port index for each eth_data*/
	ret = eth_get_port_type(_eth_ops, &lan_port, &wan_port);

	if (ret != ETH_SUCCESS) {
		goto end;
	}

	debug(DEBUG_ERROR, "lan_port=%08x, wan_port=%08x\n", lan_port, wan_port);
end:
	return ret;
}

int test_eth_layer_get_client_entry(void)
{
	int ret = ETH_SUCCESS, i = 0;
	struct dl_list entry_list;
	struct ethernet_client_entry *table_entry = NULL, *table_entry_next = NULL;

	memset(&entry_list, 0, sizeof(struct dl_list));
	ret = eth_get_table_entry(_eth_ops, &entry_list);

	if (ret != ETH_SUCCESS) {
		goto end;
	}

	printf("NO.\t MAC\t\t\tPORT\tage\n");
	dl_list_for_each(table_entry, &entry_list, struct ethernet_client_entry, entry) {
		printf("%d\t"MACSTR"\t%d\t%d\n", i++, PRINT_MAC(table_entry->mac),
			table_entry->port_index, table_entry->age);
	}

	/*destory the rest entries in entry list*/
	dl_list_for_each_safe(table_entry, table_entry_next, &entry_list, struct ethernet_client_entry, entry) {
		dl_list_del(&table_entry->entry);
		os_free(table_entry);
	}
end:
	return ret;
}

int test_eth_layer_search_table_entry(unsigned char *mac)
{
	int ret = ETH_SUCCESS;
	struct ethernet_port *pport = NULL;
	struct ethernet_client_entry entry;

	debug(DEBUG_TRACE, "start to search entry\n");
	memset(&entry, 0, sizeof(struct ethernet_client_entry));
	ret = eth_search_table_entry(_eth_ops, mac, &entry);

	if(ret != ETH_SUCCESS) {
		goto end;
	}

	pport = eth_layer_get_eth_data_by_port_index(entry.port_index);
	if (pport)
		debug(DEBUG_ERROR, "find "MACSTR" in port(%d)\n",
			PRINT_MAC(mac), entry.port_index)
	else
		debug(DEBUG_ERROR, "not found "MACSTR"\n", PRINT_MAC(mac));
end:
	return ret;
}

int test_eth_layer_get_port_status(int port_index)
{
	int ret = ETH_SUCCESS, status = 0;

	ret = eth_get_port_status(_eth_ops, port_index, &status);
	if (ret) {
		goto end;
	}
	debug(DEBUG_ERROR, "Port[%d] %s\n", port_index, status ? "LINK UP" : "LINK DOWN");
end:
	return ret;
}

int eth_layer_clear_switch_table()
{
	int ret = 0, status = 0;

	ret = eth_clear_switch_table(_eth_ops);
	if (!ret)
		return status;

	return ETH_ERROR_FAIL;
}

int test_eth_layer_clear_switch_table()
{
	int ret = 0, status = 0;

	ret = eth_clear_switch_table(_eth_ops);
	if (!ret)
		return status;

	return ETH_ERROR_FAIL;
}

int test_eth_layer_set_vlan(unsigned short vid[], unsigned char vid_num,
	unsigned char type)
{
	int ret = ETH_SUCCESS;
	unsigned char i = 0;

	debug(DEBUG_ERROR, "set %s to ethernet, number(%d) vids:",
		type == 0 ? "traffic separation vlan" : "transparent vlan", vid_num);
	for (i = 0; i < vid_num; i++) {
		printf("%d ", vid[i]);
	}
	printf("\n");

	if (type == 0) {
		eth_layer_set_traffic_separation_vlan(vid, 0);
		ret = eth_layer_set_traffic_separation_vlan(vid, vid_num);
	} else if (type == 1) {
		eth_layer_set_switch_trasnparent_vlan(vid, 0);
		ret = eth_layer_set_switch_trasnparent_vlan(vid, vid_num);
	}

	return ret;
}
