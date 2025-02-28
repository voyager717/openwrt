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
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h> // close function
#include <sys/socket.h>
#ifdef SUPPORT_COREDUMP
#include <sys/resource.h>
#endif
#ifdef SUPPORT_BACKTRACE
#include <execinfo.h>
#endif
#ifdef DISABLE_SWITCH_POLLING
#include <netlink/genl/genl.h>
#endif

#include "p1905_managerd.h"
#include "lldp_message_parse.h"
#include "lldp.h"
#include "cmdu.h"
#ifdef SUPPORT_WIFI
#include "wifi_utils.h"
#endif
#include "cmdu_retry_message.h"
#include "message_wait_queue.h"
#include "os.h"
#include "_1905_lib_io.h"
#include "debug.h"
#include "file_io.h"
#include "topology.h"
#include "eloop.h"
#include "ethernet_layer.h"
#include "netlink_event.h"
#include "worker_task.h"
#include "mapfilter_if.h"
#include "genl_netlink.h"

#ifdef MAP_R3
#include "common.h"
#include "wpa_extern.h"
#include "map_dpp.h"
#include "byteorder.h"
#include "security_engine.h"
#endif

#define MAP_CFG_FILE "/etc/map/1905d.cfg"
#define MAP_WTS_BSS_CFG_FILE "/etc/map/wts_bss_info_config"
#define MAP_WIFI_FILE "/etc/wifi_info.txt"

extern int _1905_interface_init(struct p1905_managerd_ctx *ctx);
extern int _1905_interface_process(struct p1905_managerd_ctx *ctx, struct sockaddr_un* from,
					socklen_t fromlen, char *buf, size_t buf_len, char** reply, size_t *resp_len);
extern void _1905_interface_deinit(struct p1905_managerd_ctx *ctx);
extern void _1905_interface_receive_process(int sock, void *eloop_ctx,
					      void *sock_ctx);
void cmdu_process(int sock, void *eloop_ctx, void *sock_ctx);
void library_cmd_process(int sock, void *eloop_ctx, void *sock_ctx);
void ctrl_cmd_process(int sock, void *eloop_ctx, void *sock_ctx);
void netlink_event_process(int sock, void *eloop_ctx, void *sock_ctx);
void discovery_at_boot_up(void *eloop_data, void *user_ctx);
void worker_task_event_process(int sock, void *eloop_ctx, void *sock_ctx);

#define IEEE802_11_band_2P4G 0x00
#define IEEE802_11_band_5GL  0x01
#define IEEE802_11_band_5GH  0x02

#ifdef CHANGE_BR_AGEING_TIME
#define BRIDGE_AGEING_TIME 150//seconds
#endif

#define P1905_TIMER_INTERVAL_2_SEC 3

#define PERIODIC_TIMER_SEC	5
#define P1905_TIMER_INTERVAL_SEC 1   	/*1 * 5s = 5s*/
#define TOPOLOGY_DISCOVERY_COUNT 11     /*11 * 5s = 55s*/
#define DUMP_INFO_COUNT 700
#define SWITCH_TABLE_DUMP_TIME	45
#define ONE_SECOND_CNT 60
#define DROP_FRAGMENT_CNT 15
#define DECRYPT_FAULURE_THRESHOLD 100

#ifdef SUPPORT_ALME
#define ALME_WAIT_LINK_METRIC_RSP_CNT 2
#endif

unsigned char p1905_multicast_address[6]
                    = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x13 };
unsigned char nearest_bridge_group_address[6]
                    = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e };
unsigned char dev_send_1905_buf[3072*2] = {0};
int debug_level = DEBUG_ERROR;
volatile sig_atomic_t timer_expired = 0;
volatile sig_atomic_t exit_signal = 0;
volatile sig_atomic_t hpav_info_has_changed = 0;
#define DISC_RETRY_TIME 3

void ethernet_plug_in_handler(void *context, void *data)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)context;
	struct port_status_info *port_info = (struct port_status_info *)data;
	int j = 0, i = 0;
	if (port_info->status == 0)
		return;
	usleep(20000);
	ctx->mid++;
	for (j = 0; j < ctx->itf_number; j++) {
		/*send multicast cmdu to all the ethenet interface */
		if (ctx->itf[j].media_type == IEEE802_3_GROUP) {
			for(i = 0; i < DISC_RETRY_TIME; i++) {
				insert_cmdu_txq(p1905_multicast_address, ctx->p1905_al_mac_addr,
					e_topology_discovery_with_vendor_ie, ctx->mid, ctx->itf[j].if_name, 0);
			}
			debug(DEBUG_ERROR, TOPO_PREX"tx multicast discovery(mid-%04x) with vs tlv on %s\n",
				ctx->mid, ctx->itf[j].if_name);
		}
	}
	eloop_register_timeout(1, 0, discovery_at_boot_up, (void *)ctx, NULL);
}

void ethernet_pull_out_handler(void *context, void *data)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)context;
	struct port_status_info *port_info = (struct port_status_info *)data;
	int i = 0, cnt = 0;

	if (port_info->status == 1)
		return;
	cnt = delete_topo_disc_db_by_port(ctx, port_info->port, NULL);
	if (cnt > 0) {
		report_own_topology_rsp(ctx, ctx->p1905_al_mac_addr, ctx->br_cap,
			ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
			ctx->service, &ctx->ap_cap_entry.oper_bss_head,
			&ctx->ap_cap_entry.assoc_clients_head, ctx->cnt);
		ctx->mid++;
		debug(DEBUG_ERROR, TOPO_PREX"tx multicast notification(mid-%04x) on all intf\n",
			ctx->mid);
		for(i = 0; i < ctx->itf_number; i++) {
			insert_cmdu_txq((unsigned char*)p1905_multicast_address,
					(unsigned char*)ctx->p1905_al_mac_addr,
					e_topology_notification, ctx->mid,
					ctx->itf[i].if_name, 0);
#ifdef SUPPORT_CMDU_RELIABLE
			cmdu_reliable_send(ctx, e_topology_notification, ctx->mid, i);
#endif
		}
		/*vlan case to clear switch table of upper ap device*/
		debug(DEBUG_ERROR, TOPO_PREX"clear switch table port [%d]\n", port_info->port);
		eth_layer_clear_switch_table();
	}
	unmask_control_conn_port(ctx, port_info->port);
	delete_topo_response_db_by_port_interface(ctx, port_info->port);
	update_topology_tree(ctx);
}

unsigned char delete_non_p1905_neighbor_dev_info_byport(struct non_p1905_neighbor *non_p1905_dev, unsigned char port)
{
    int i = 0;
	unsigned char changed = 0;
    struct non_p1905_neighbor_info *dev_info, *dev_info_temp;

    for (i = 0; i < ITF_NUM; i++) {
        if (!LIST_EMPTY(&(non_p1905_dev->non_p1905nbr_head))) {
            dev_info = LIST_FIRST(&(non_p1905_dev->non_p1905nbr_head));
            while (dev_info) {
                dev_info_temp = LIST_NEXT(dev_info, non_p1905nbr_entry);
				if (dev_info->port_index == port) {
                	LIST_REMOVE(dev_info, non_p1905nbr_entry);
                	free(dev_info);
					changed = 1;
				}
                dev_info = dev_info_temp;
            }
        }
    }
	return changed;
}

unsigned char delete_non_p1905_neighbor_dev_info_bymac(struct non_p1905_neighbor *non_p1905_dev, unsigned char *mac)
{
    int i = 0;
	unsigned char changed = 0;
    struct non_p1905_neighbor_info *dev_info, *dev_info_temp;

    for (i = 0; i < ITF_NUM; i++) {
        if (!LIST_EMPTY(&(non_p1905_dev->non_p1905nbr_head))) {
            dev_info = LIST_FIRST(&(non_p1905_dev->non_p1905nbr_head));
            while (dev_info) {
                dev_info_temp = LIST_NEXT(dev_info, non_p1905nbr_entry);
				if (!os_memcmp(dev_info->itf_mac_addr, mac, ETH_ALEN)) {
                	LIST_REMOVE(dev_info, non_p1905nbr_entry);
                	free(dev_info);
					changed = 1;
				}
                dev_info = dev_info_temp;
            }
        }
    }
	return changed;
}

struct non_p1905_neighbor_info *get_non_p1905_neighbor_dev_info_bymac(struct non_p1905_neighbor *non_p1905_dev,
	unsigned char *mac, unsigned char port)
{
    struct non_p1905_neighbor_info *dev_info;

	LIST_FOREACH(dev_info, &(non_p1905_dev->non_p1905nbr_head), non_p1905nbr_entry) {
		if (!os_memcmp(dev_info->itf_mac_addr, mac, ETH_ALEN) && dev_info->port_index == port) {
			break;
		}
	}
    return dev_info;
}

void non_1905_neighbor_update(void *eloop_data, void *user_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_data;
	unsigned int i = 0, j = 0;
	struct topology_discovery_db *discovery_db = NULL;
	struct topology_response_db *rsp_db = NULL;
	struct ethernet_client_entry *client_entry = NULL;
	struct non_p1905_neighbor *non_1905_neighbor_list = NULL;
	struct non_p1905_neighbor_info *info = NULL;
	unsigned char topo_changed = 0;
	struct ethernet_port *peth_data = NULL;
	struct non_p1905_neighbor_info *dev_info, *dev_info_temp;

	if (eth_layer_port_client_update() < 0)
		return;

	for (j = 0; j < ctx->itf_number; j++) {
		if (ctx->itf[j].media_type != IEEE802_3_GROUP)
			continue;

		non_1905_neighbor_list = &ctx->non_p1905_neighbor_dev[j];

		for (i = 0; i < MAX_LAN_PORT_NUM; i++) {
			discovery_db = NULL;
			peth_data = eth_layer_get_eth_data_by_port_num(i);
			if(peth_data == NULL) {
				debug(DEBUG_ERROR, "peth_data null\n");
				return;
			}
			if (ctx->role == CONTROLLER && peth_data->wan_port)
				continue;
			dl_list_for_each(client_entry, &peth_data->clients, struct ethernet_client_entry, entry) {
				discovery_db = get_tpd_db_by_mac(ctx, client_entry->mac);

				/*to check any 1905 device exist*/
				if (discovery_db != NULL) {
					/*delete all non-1905 neighbor device belongs to this port*/
					debug(DEBUG_TRACE, TOPO_PREX"delete all non 1905.1 neighbors for 1905 topology discovery on port %d\n",
						peth_data->port_index);
					topo_changed = delete_non_p1905_neighbor_dev_info_byport(non_1905_neighbor_list, peth_data->port_index);
					break;
				}
			}
			/*if no direct 1905 device found with current port*/
			if (discovery_db == NULL) {
				dl_list_for_each(client_entry, &peth_data->clients, struct ethernet_client_entry, entry) {
					rsp_db = get_trsp_db_by_mac(ctx, client_entry->mac);
					if (rsp_db != NULL) {
						break;
					}
				}
				/*if this almac is found in topology response db, should not add it into non 1905 neighbor list*/
				if (rsp_db == NULL) {
					dl_list_for_each(client_entry, &peth_data->clients, struct ethernet_client_entry, entry) {
						info = get_non_p1905_neighbor_dev_info_bymac(non_1905_neighbor_list, client_entry->mac, peth_data->port_index);
						if (info == NULL) {
							info = (struct non_p1905_neighbor_info*)os_malloc(sizeof(struct non_p1905_neighbor_info));
							if (info == NULL)
								continue;
							info->port_index = peth_data->port_index;
							os_memcpy(info->itf_mac_addr, client_entry->mac, ETH_ALEN);
							debug(DEBUG_TRACE, TOPO_PREX"insert non 1905.1 dev("MACSTR") on inf("MACSTR") port(%d)\n",
								PRINT_MAC(info->itf_mac_addr), PRINT_MAC(non_1905_neighbor_list->local_mac_addr),
								peth_data->port_index);
							LIST_INSERT_HEAD(&non_1905_neighbor_list->non_p1905nbr_head, info, non_p1905nbr_entry);
							topo_changed = 1;
						}
					}
				} else {
					topo_changed = delete_non_p1905_neighbor_dev_info_bymac(non_1905_neighbor_list, rsp_db->al_mac_addr);
				}

				/*delete the non 1905 neighbor from list by checking the switch port data list*/
				dev_info = LIST_FIRST(&(non_1905_neighbor_list->non_p1905nbr_head));
	            while (dev_info) {
	                dev_info_temp = LIST_NEXT(dev_info, non_p1905nbr_entry);
					if (dev_info->port_index == peth_data->port_index) {
						if (!eth_layer_get_client_by_mac(peth_data, dev_info->itf_mac_addr)) {
							LIST_REMOVE(dev_info, non_p1905nbr_entry);
							debug(DEBUG_TRACE, TOPO_PREX"delete non 1905.1 dev("MACSTR") on inf("MACSTR") port(%d)\n",
								PRINT_MAC(dev_info->itf_mac_addr), PRINT_MAC(non_1905_neighbor_list->local_mac_addr),
								dev_info->port_index);
		                	os_free(dev_info);
							topo_changed = 1;
						}
					}
	                dev_info = dev_info_temp;
	            }
			}
		}
	}
	if (topo_changed) {
		report_own_topology_rsp(ctx, ctx->p1905_al_mac_addr, ctx->br_cap,
							ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
							ctx->service, &ctx->ap_cap_entry.oper_bss_head,
							&ctx->ap_cap_entry.assoc_clients_head, ctx->cnt);
		ctx->mid++;
		debug(DEBUG_ERROR, TOPO_PREX"topo change! tx multicast notification(mid-%04x) on all intf\n", ctx->mid);
		for (j = 0; j < ctx->itf_number; j++) {
			/*send multicast cmdu to all the interface */
			insert_cmdu_txq(p1905_multicast_address, ctx->p1905_al_mac_addr,
				e_topology_notification, ctx->mid, ctx->itf[j].if_name, 0);
#ifdef SUPPORT_CMDU_RELIABLE
			cmdu_reliable_send(ctx, e_topology_notification, ctx->mid, j);
#endif
		}
	}

	eloop_register_timeout(SWITCH_TABLE_DUMP_TIME, 0, non_1905_neighbor_update, (void *)ctx, NULL);
}


#define CONFIG_SYNC_TOLERATE_TIME 30
#define RENEW_ROUND 4
void config_sync_error_handler(void *context, void* parent_leaf, void *current_leaf, void *data)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)context;
	struct leaf *cur_leaf = (struct leaf *)current_leaf;
	struct os_time now;
	struct topology_response_db *rpdb = NULL;
	unsigned char band = 0;
	struct agent_list_db *agent = NULL;
	unsigned char *if_name = NULL;
	int i = 0;
#ifdef MAP_R3
	struct r3_member *peer = NULL;
	if (ctx->map_version == MAP_PROFILE_R3) {
		peer = get_r3_member(cur_leaf->al_mac);
	}
#endif
	/*doing renew, do not start config error sync*/
	if (ctx->renew_ctx.trigger_renew)
		return;

	os_get_time(&now);

	find_agent_info(ctx, cur_leaf->al_mac, &agent);
	if (!agent) {
		debug(DEBUG_ERROR, "!!agent info("MACSTR") not exist\n",
			PRINT_MAC(cur_leaf->al_mac));
		return;
	}

	/*it means only after autoconfig, can check whether should renew the deivce*/
	/*
	if (cur_leaf->band_cap == 0)
		return;
	*/
	if (now.sec - cur_leaf->create_time.sec > CONFIG_SYNC_TOLERATE_TIME) {
		/*to let send renew per about 20 seconds if peer device still can not be configured*/
		if (cur_leaf->renew_send_round > 0)
			cur_leaf->renew_send_round--;

		if (cur_leaf->renew_send_round == 0) {

			rpdb = lookup_tprdb_by_almac(ctx, cur_leaf->al_mac);
			if (!rpdb) {
				debug(DEBUG_ERROR, "BUG here!!!! rpdb almac("MACSTR") not exist\n",
					PRINT_MAC(cur_leaf->al_mac));
				return;
			}
			if_name = ctx->itf[rpdb->recv_ifid].if_name;
			cur_leaf->renew_send_round = RENEW_ROUND;

			if (!get_agent_all_radio_config_state(agent)) {
				debug(DEBUG_ERROR, "config push to almac("MACSTR")\n", PRINT_MAC(cur_leaf->al_mac));

#ifdef MAP_R3
				if (peer && peer->security_1905) {
					debug(DEBUG_ERROR, "send bss reconfig trigger to almac("MACSTR")\n", PRINT_MAC(cur_leaf->al_mac));
					insert_cmdu_txq(cur_leaf->al_mac, ctx->p1905_al_mac_addr,
							e_bss_reconfiguration_trigger, ++ctx->mid, if_name, 0);
					process_cmdu_txq(ctx, ctx->trx_buf.buf);
					return;
				}
#endif
				for (i = 0; i < agent->radio_num; i++) {
					if (!agent->ra_info[i].conf_ctx.config_status) {
						if (agent->ra_info[i].band & BAND_2G_CAP)
							band = 0x00;
						else if (agent->ra_info[i].band & (BAND_5G_CAP))
							band = 0x01;
						break;
					}
				}
				if (fill_send_tlv(ctx, &band, 1) < 0) {
					return;
				}
				insert_cmdu_txq(cur_leaf->al_mac, ctx->p1905_al_mac_addr,
						e_ap_autoconfiguration_renew, ++ctx->mid, if_name, 0);
				process_cmdu_txq(ctx, ctx->trx_buf.buf);
			}
		}
	}
}

void manage_cmd_process(struct p1905_managerd_ctx *ctx, struct manage_cmd *cmd)
{
#define REPLY_BUF_LEN 2048
	unsigned char *buf = NULL;
	size_t len = 0;
	unsigned short length = 0;

	buf = os_zalloc(REPLY_BUF_LEN);
	if (!buf) {
		debug(DEBUG_ERROR, "fail to allocate buf\n");
		return;
	}

	switch(cmd->cmd_id) {
		case MANAGE_SET_ROLE:
			{
				if (change_role_dynamic(ctx, cmd->content[0]) < 0) {
					os_strncpy((char*)buf, "FAIL\n", 5);
					len = 5;
				} else {
					os_strncpy((char*)buf, "OK\n", 3);
					len = 3;
				}
				_1905_notify_raw_data(ctx, buf, len);
			}
			break;
		case MANAGE_SET_LOG_LEVEL:
			break;
		case MANAGE_GET_LOCAL_DEVINFO:
			if (get_local_device_info(ctx, buf, REPLY_BUF_LEN, &length) < 0) {
				debug(DEBUG_ERROR, "error when get device info\n");
				break;
			}
			len = length;
			_1905_notify_raw_data(ctx, buf, len);
			eloop_register_timeout(1,0, attach_action, (void *)ctx, NULL);
			break;
		case MANAGE_SET_BSS_CONF:
			{
				unsigned char oper = cmd->content[0];
				set_bss_config(ctx, oper, (struct bss_config_info*)(&cmd->content[1]));
			}
			break;
		default:
			break;
	}

	os_free(buf);
}

void p1905_manage_terminate(int signum, void *signal_ctx)
{
	eloop_terminate();
}

#define NEIGHBOR_TOPOLOGY_QUERY_TIME 20
#define NEIGHBOR_TOPOLOGY_EXPIRE_TIME 40
#define TOPOLOGY_EXPIRE_TIME 70
#define TOPOLOGY_EXPIRE_DEL_TIME 90
void delete_exist_topology_response_database(struct p1905_managerd_ctx *ctx,
                                                unsigned char *al_mac);

static void check_topology_rsp_expired(struct p1905_managerd_ctx *ctx)
{
	struct os_time now;
	struct topology_response_db *rsp, *rsp_tmp = NULL;
	struct topology_discovery_db *disc = NULL, *disc_tmp = NULL;
	char query = 0, del = 0, del_rsp_cnt = 0;
	unsigned int i = 0;
	unsigned char *ifname = NULL;

	os_get_time(&now);

	/*fistly,  check whether it is necessary query all devices in topology response database*/
	disc = LIST_FIRST(&ctx->topology_entry.tpddb_head);
	while (disc) {
		query = 0;
		del = 0;
		disc_tmp = LIST_NEXT(disc, tpddb_entry);
		rsp = find_response_by_almac(ctx, disc->al_mac);
		if (rsp) {
			if ((now.sec - rsp->last_seen.sec) > NEIGHBOR_TOPOLOGY_EXPIRE_TIME) {
				del = 1;
			} else if ((now.sec - rsp->last_seen.sec) > NEIGHBOR_TOPOLOGY_QUERY_TIME) {
				query = 1;
			}
		} else {
			query = 1;
		}

		if (query) {
			/*get ifname by discovery*/
			for (i = 0; i < ctx->itf_number; i++) {
				if (!os_memcmp(ctx->itf[i].mac_addr, disc->receive_itf_mac, ETH_ALEN))
					break;
			}
			if (i < ctx->itf_number)
				ifname = ctx->itf[i].if_name;
			else
				ifname = ctx->br_name;
			insert_cmdu_txq(disc->al_mac, ctx->p1905_al_mac_addr,
					e_topology_query, ++ctx->mid, ifname, 0);
			debug(DEBUG_TRACE, TOPO_PREX"tx query(mid-%04x) to neighbor"MACSTR" on %s\n",
				ctx->mid, MAC2STR(disc->al_mac), ifname);
		}
		if (del) {
			debug(DEBUG_ERROR, TOPO_PREX"timeout!!! del neighbor("MACSTR")\n",
				PRINT_MAC(disc->al_mac));
			delete_p1905_neighbor_dev_info(ctx, disc->al_mac, disc->receive_itf_mac);
			report_own_topology_rsp(ctx, ctx->p1905_al_mac_addr, ctx->br_cap,
				ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
				ctx->service, &ctx->ap_cap_entry.oper_bss_head,
				&ctx->ap_cap_entry.assoc_clients_head, ctx->cnt);
			LIST_REMOVE(disc, tpddb_entry);
			if (find_discovery_by_almac(ctx, disc->al_mac) == NULL) {
				delete_exist_topology_response_database(ctx, disc->al_mac);
				del_rsp_cnt++;
				debug(DEBUG_ERROR, TOPO_PREX"timeout!!! no discovery in db; del rsp("MACSTR")\n",
					PRINT_MAC(disc->al_mac));
			}
			unmask_control_conn_port(ctx, disc->eth_port);
			free(disc);
		}
		disc = disc_tmp;
	}

	rsp = SLIST_FIRST(&ctx->topology_entry.tprdb_head);
	while (rsp) {
		query = 0;
		del = 0;
		rsp_tmp = SLIST_NEXT(rsp, tprdb_entry);
		if (find_discovery_by_almac(ctx, rsp->al_mac_addr) == NULL) {
			if ((now.sec - rsp->last_seen.sec > TOPOLOGY_EXPIRE_TIME) &&
				(now.sec - rsp->last_seen.sec < TOPOLOGY_EXPIRE_DEL_TIME) ) {
				query = 1;
			} else if (now.sec - rsp->last_seen.sec > TOPOLOGY_EXPIRE_DEL_TIME) {
				del = 1;
			}
		}

		if (query) {
			ifname = ctx->itf[rsp->recv_ifid].if_name;
			insert_cmdu_txq(rsp->al_mac_addr, ctx->p1905_al_mac_addr,
					e_topology_query, ++ctx->mid, ifname, 0);
			debug(DEBUG_TRACE, TOPO_PREX"tx query(mid-%04x) to non neighbor dev("MACSTR") on %s\n",
				ctx->mid, PRINT_MAC(rsp->al_mac_addr), ifname);
		}
		if (del) {
			delete_exist_topology_response_database(ctx, rsp->al_mac_addr);
			debug(DEBUG_ERROR, TOPO_PREX"timeout!!! del non neighbor dev("MACSTR")\n", PRINT_MAC(rsp->al_mac_addr));
			del_rsp_cnt++;
		}
		rsp = rsp_tmp;
	}

	/*secondly, check whether it is necessary to update my own topology response*/
	if (now.sec - ctx->own_topo_rsp_update_time.sec > TOPOLOGY_EXPIRE_TIME) {
		debug(DEBUG_TRACE, TOPO_PREX"update my own topology rsp to mapd\n");
		report_own_topology_rsp(ctx, ctx->p1905_al_mac_addr, ctx->br_cap,
			ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
			ctx->service, &ctx->ap_cap_entry.oper_bss_head,
			&ctx->ap_cap_entry.assoc_clients_head, ctx->cnt);
	}

	if (del_rsp_cnt)
		mark_valid_topo_rsp_node(ctx);
}


/*1905 one sec timer*/
void p1905_managerd_periodic(void *eloop_data, void *user_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)eloop_data;
	static unsigned int cnt = 0;
	int i = 0;
	struct buf_info *buf = &ctx->trx_buf;

	cnt += PERIODIC_TIMER_SEC;

    /*1. update LLDP database, if exceed time to live , delete*/
    update_lldp_queue_ttl(PERIODIC_TIMER_SEC);

    /*2. delete 1905.1 neighbor if not receiving new topology discovery*/
    if (delete_not_exist_p1905_neighbor_device(ctx, PERIODIC_TIMER_SEC)) {
    	ctx->mid++;
		debug(DEBUG_ERROR, TOPO_PREX"topo change! tx multicast notification(mid-%04x) on all intf\n",
			ctx->mid);
		for(i = 0; i < ctx->itf_number; i++) {
        	insert_cmdu_txq(p1905_multicast_address, ctx->p1905_al_mac_addr,
                        e_topology_notification, ctx->mid, ctx->itf[i].if_name, 0);
#ifdef SUPPORT_CMDU_RELIABLE
			cmdu_reliable_send(ctx, e_topology_notification, ctx->mid, i);
#endif
		}
    }
	/*3 delete 1905 topology device*/
	delete_not_exist_p1905_topology_device(ctx, PERIODIC_TIMER_SEC);
	check_topology_rsp_expired(ctx);
	/*update topology tree*/
	update_topology_tree(ctx);
	/*config sync error handling*/
	if (ctx->role == CONTROLLER && !ctx->MAP_Cer)
		trace_topology_tree_cb(ctx, ctx->root_leaf, (topology_tree_cb_func)config_sync_error_handler, NULL);

#ifdef MAP_R3_SP
	if (ctx->role == CONTROLLER && !ctx->MAP_Cer) {
		trace_topology_tree_cb(ctx, ctx->root_leaf, (topology_tree_cb_func)sp_rule_sync_handler, NULL);
		if ((cnt % ONE_SECOND_CNT) == 0) {
			sp_sync_learn_complete_rule((void*)ctx);
		}
	}
#endif

	if ((cnt % DROP_FRAGMENT_CNT) == 0) {
		/*increment fragment timeout counter*/
		add_fragment_cnt();
		if (get_fragment_cnt() >= 2)
			delete_fragment_queue_all();
	}
	clear_expired_dm_buffer(ctx);
	/*check if any message need resend*/
	handle_retry_message_queue(ctx);
	common_process(ctx, buf);
	eloop_cancel_timeout(p1905_managerd_periodic, (void *)ctx, NULL);
	eloop_register_timeout(PERIODIC_TIMER_SEC, 0, p1905_managerd_periodic, (void *)ctx, NULL);
}

void send_discovery_periodic(void *eloop_data, void *user_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)eloop_data;
	int i = 0;
	struct buf_info *buf = &ctx->trx_buf;

	ctx->mid++;
	debug(DEBUG_TRACE, TOPO_PREX"periodically! tx multicast discovery(mid-%04x) on all intf\n",
		ctx->mid);
	for (i = 0; i < ctx->itf_number; i++)
		insert_cmdu_txq(p1905_multicast_address, ctx->p1905_al_mac_addr,
			e_topology_discovery, ctx->mid, ctx->itf[i].if_name, 0);

     /*according to IEEE P1905.1 spec, we need to send 802.1 bridge
         *detection message after sending topology discovery message
         *so we implement this functionality here
         */
	for (i = 0; i < ctx->itf_number; i++) {
		if (0 > send_802_1_bridge_discovery_msg(ctx,
			nearest_bridge_group_address, ctx->itf[i].mac_addr, buf->buf, i))
			debug(DEBUG_ERROR, "send lldpdu fail on %s\n", ctx->itf[i].if_name);
	}

	common_process(ctx, buf);
	eloop_register_timeout(ctx->discovery_cnt * PERIODIC_TIMER_SEC, 0,
		send_discovery_periodic, (void *)ctx, NULL);
}


void p1905_database_init(struct p1905_managerd_ctx *ctx)
{
	ctx->service = ctx->role;
	ctx->mid = (unsigned short)os_random();
    LIST_INIT(&ctx->topology_entry.tpddb_head);
    SLIST_INIT(&ctx->topology_entry.tprdb_head);
    SLIST_INIT(&ctx->topodev_entry.tpdevdb_head);
	SLIST_INIT(&ctx->ap_cap_entry.ap_ht_cap_head);
	SLIST_INIT(&ctx->ap_cap_entry.ap_vht_cap_head);
	SLIST_INIT(&ctx->ap_cap_entry.oper_bss_head);
	SLIST_INIT(&ctx->ap_cap_entry.assoc_clients_head);
	SLIST_INIT(&ctx->ap_cap_entry.ch_prefer_head);
	SLIST_INIT(&ctx->ap_cap_entry.oper_restrict_head);
	SLIST_INIT(&ctx->map_policy.spolicy.local_disallow_head);
	SLIST_INIT(&ctx->map_policy.spolicy.btm_disallow_head);
	SLIST_INIT(&ctx->map_policy.spolicy.radio_policy_head);
	SLIST_INIT(&ctx->map_policy.mpolicy.policy_head);
	SLIST_INIT(&ctx->metric_entry.metrics_query_head);
	SLIST_INIT(&ctx->metric_entry.metrics_rsp_head);
	SLIST_INIT(&ctx->metric_entry.traffic_stats_head);
	SLIST_INIT(&ctx->metric_entry.link_metrics_head);
	SLIST_INIT(&ctx->metric_entry.unlink_info.unlink_metrics_head);
	SLIST_INIT(&ctx->agent_head);
	SLIST_INIT(&ctx->query_neighbor_head);
	SLIST_INIT(&ctx->steer_cntrl.policy_head);
#ifdef MAP_R2
	SLIST_INIT(&ctx->ap_cap_entry.ch_scan_cap_head);
	SLIST_INIT(&ctx->ap_cap_entry.radio_cac_cap.radio_cac_capab_head);
	SLIST_INIT(&ctx->metric_entry.radio_metrics_head);
	SLIST_INIT(&ctx->metric_entry.assoc_sta_extended_link_metrics_head);
	SLIST_INIT(&ctx->metric_entry.radio_identifier_head);
	SLIST_INIT(&ctx->map_policy.tpolicy.policy_head);
	SLIST_INIT(&ctx->map_policy.epolicy.policy_head);
	SLIST_INIT(&ctx->map_policy.fpolicy.bssid_head);
#endif
}

extern 	int debug_level;

void clear_topology_db(struct p1905_topology_db *topo_entry)
{
	struct topology_discovery_db *pddb = NULL, *pddb_tmp = NULL;
	struct topology_response_db *prdb = NULL, *prdb_tmp = NULL;
	struct device_info_db *pdev_info_db = NULL, *pdev_info_db_tmp = NULL;
	struct p1905_neighbor_device_db *neighbor_dev_db = NULL, *neighbor_dev_db_tmp = NULL;
	struct non_p1905_neighbor_device_list_db *non_neighbor_dev_db = NULL;
	struct non_p1905_neighbor_device_list_db *non_neighbor_dev_db_tmp = NULL;
	struct device_bridge_capability_db *pbrcap_db = NULL, *pbrcap_db_tmp = NULL;

	/*free list tpddb_head*/
	pddb = LIST_FIRST(&topo_entry->tpddb_head);
	while (pddb != NULL) {
		pddb_tmp = LIST_NEXT(pddb, tpddb_entry);
		debug(DEBUG_TRACE, "free topology_discovery_db(al="MACSTR")\n",
			PRINT_MAC(pddb->al_mac));
		free(pddb);
		pddb = pddb_tmp;
	}
	LIST_INIT(&topo_entry->tpddb_head);

	/*free list tprdb_head*/
	prdb = SLIST_FIRST(&topo_entry->tprdb_head);
	while (prdb != NULL) {
		prdb_tmp = SLIST_NEXT(prdb, tprdb_entry);
		debug(DEBUG_TRACE, "[%s]free topology_response_db(al="MACSTR")\n",
			__func__, PRINT_MAC(prdb->al_mac_addr));

		/*free devinfo_head*/
		pdev_info_db = SLIST_FIRST(&prdb->devinfo_head);
		while (pdev_info_db != NULL) {
			pdev_info_db_tmp = SLIST_NEXT(pdev_info_db, devinfo_entry);
			debug(DEBUG_TRACE, "[%s]free device_info_db(mac="MACSTR")\n",
				__func__, PRINT_MAC(pdev_info_db->mac_addr));

			/*free p1905_neighbor_device_db*/
			neighbor_dev_db = SLIST_FIRST(&pdev_info_db->p1905_nbrdb_head);
			while (neighbor_dev_db != NULL) {
				neighbor_dev_db_tmp = SLIST_NEXT(neighbor_dev_db, p1905_nbrdb_entry);
				debug(DEBUG_TRACE, "[%s]free neighbor_dev_db(al="MACSTR")\n",
					__func__, PRINT_MAC(neighbor_dev_db->p1905_neighbor_al_mac));
				free(neighbor_dev_db);
				neighbor_dev_db = neighbor_dev_db_tmp;
			}
			SLIST_INIT(&pdev_info_db->p1905_nbrdb_head);

			/*free non_p1905_neighbor_device_list_db*/
			non_neighbor_dev_db = SLIST_FIRST(&pdev_info_db->non_p1905_nbrdb_head);
			while (non_neighbor_dev_db != NULL) {
				non_neighbor_dev_db_tmp = SLIST_NEXT(non_neighbor_dev_db, non_p1905_nbrdb_entry);
				debug(DEBUG_TRACE, "[%s]free non neighbor_dev_db(al="MACSTR")\n",
					__func__, PRINT_MAC(non_neighbor_dev_db->non_p1905_device_interface_mac));
				free(non_neighbor_dev_db);
				non_neighbor_dev_db = non_neighbor_dev_db_tmp;
			}
			SLIST_INIT(&(pdev_info_db->non_p1905_nbrdb_head));

			if(pdev_info_db->vs_info_len)
				free(pdev_info_db->vs_info);

			free(pdev_info_db);
			pdev_info_db = pdev_info_db_tmp;
		}
		SLIST_INIT(&prdb->devinfo_head);

		/*free device_bridge_capability_db*/
		pbrcap_db = LIST_FIRST(&prdb->brcap_head);
		while (pbrcap_db != NULL) {
			pbrcap_db_tmp = LIST_NEXT(pbrcap_db, brcap_entry);
			if(pbrcap_db->interface_amount != 0)
				free(pbrcap_db->interface_mac_tuple);
			free(pbrcap_db);
			pbrcap_db = pbrcap_db_tmp;
		}
		LIST_INIT(&(prdb->brcap_head));

		free(prdb);
		prdb = prdb_tmp;
	}
	SLIST_INIT(&topo_entry->tprdb_head);

}

void clear_topology_neighbor_db(struct p1905_neighbor *neighbor, unsigned char itf_number)
{
	struct p1905_neighbor_info *pinfo = NULL, *pinfo_tmp = NULL;
	int i = 0;

	for(i = 0; i < itf_number; i++) {
		pinfo = LIST_FIRST(&neighbor[i].p1905nbr_head);
		while (pinfo != NULL) {
			pinfo_tmp = LIST_NEXT(pinfo, p1905nbr_entry);
			debug(DEBUG_TRACE, "[%s]neighbor[%d]free p1905_neighbor_info(al="MACSTR")\n",
				__func__,i, PRINT_MAC(pinfo->al_mac_addr));
			free(pinfo);
			pinfo = pinfo_tmp;
		}
		LIST_INIT(&neighbor[i].p1905nbr_head);
	}
}

void clear_topology_device_db(struct p1905_topodevice_db *topodev_entry)
{
	struct topology_device_db *pdev = NULL, *pdev_tmp = NULL;

	pdev = SLIST_FIRST(&topodev_entry->tpdevdb_head);
	while (pdev != NULL) {
		pdev_tmp = SLIST_NEXT(pdev, tpdev_entry);
		debug(DEBUG_TRACE, "[%s]free topology device(al="MACSTR")\n",
			__func__, PRINT_MAC(pdev->aldev_mac));
		free(pdev);
		pdev = pdev_tmp;
	}
	SLIST_INIT(&topodev_entry->tpdevdb_head);
}

int clear_all_exist_radio_basic_capability(
	struct p1905_managerd_ctx *ctx)
{
	int i = 0;
	struct radio_info *r = NULL;
	struct operating_class *opc = NULL, *opc_tmp = NULL;

	for (i = 0; i < ctx->radio_number; i++) {
		r = &ctx->rinfo[i];
		dl_list_for_each_safe(opc, opc_tmp, &r->basic_cap.op_class_list, struct operating_class, entry) {
			dl_list_del(&opc->entry);
			os_free(opc);
		}
	}
	return 0;
}

void clear_ap_capability_db(struct ap_capability_db *apcap_entry)
{
	struct ap_ht_cap_db *pht_cap_db = NULL, *pht_cap_db_tmp = NULL;
	struct ap_vht_cap_db *pvht_cap_db = NULL, *pvht_cap_db_tmp = NULL;
	struct operational_bss_db *oper_bss_db = NULL, *oper_bss_db_tmp = NULL;
	struct op_bss_db *bss_db = NULL, *bss_db_tmp = NULL;
	struct associated_clients_db *assoc_client_db = NULL, *assoc_client_db_tmp = NULL;
	struct clients_db *cli_db = NULL, *cli_db_tmp = NULL;
	struct ch_prefer_db *ch_pre_db	= NULL, *ch_pre_db_tmp = NULL;
	struct prefer_info_db *pre_db = NULL, *pre_db_tmp = NULL;
	struct oper_restrict_db *restriction  = NULL, *restriction_tmp = NULL;
	struct restrict_db *resdb = NULL, *resdb_tmp = NULL;

	/*free ap_ht_cap_head*/
	pht_cap_db = SLIST_FIRST(&apcap_entry->ap_ht_cap_head);
	while (pht_cap_db != NULL) {
		pht_cap_db_tmp = SLIST_NEXT(pht_cap_db, ap_ht_cap_entry);
		debug(DEBUG_TRACE, "free ht_cap_db(id="MACSTR")\n",
			PRINT_MAC(pht_cap_db->identifier));
		free(pht_cap_db);
		pht_cap_db = pht_cap_db_tmp;
	}
	SLIST_INIT(&apcap_entry->ap_ht_cap_head);

	/*free ap_vht_cap_head*/
	pvht_cap_db = SLIST_FIRST(&apcap_entry->ap_vht_cap_head);
	while (pvht_cap_db != NULL) {
		pvht_cap_db_tmp = SLIST_NEXT(pvht_cap_db, ap_vht_cap_entry);
		debug(DEBUG_TRACE, "free vht_cap_db(id="MACSTR")\n",
			PRINT_MAC(pvht_cap_db->identifier));
		free(pvht_cap_db);
		pvht_cap_db = pvht_cap_db_tmp;
	}
	SLIST_INIT(&apcap_entry->ap_vht_cap_head);

	/*free oper_bss_head*/
	oper_bss_db = SLIST_FIRST(&apcap_entry->oper_bss_head);
	while (oper_bss_db != NULL) {
		oper_bss_db_tmp = SLIST_NEXT(oper_bss_db, oper_bss_entry);
		debug(DEBUG_TRACE, "free operational_bss_db(id="MACSTR"))\n",
			PRINT_MAC(oper_bss_db->identifier));
		/*free basic_cap_db*/
		bss_db = SLIST_FIRST(&oper_bss_db->op_bss_head);
		while (bss_db != NULL) {
			bss_db_tmp = SLIST_NEXT(bss_db, op_bss_entry);
			debug(DEBUG_TRACE, "free bss_db bssid["MACSTR"]\n",
				PRINT_MAC(bss_db->bssid));
			free(bss_db);
			bss_db = bss_db_tmp;
		}
		SLIST_INIT(&oper_bss_db->op_bss_head);
		free(oper_bss_db);
		oper_bss_db = oper_bss_db_tmp;
	}
	SLIST_INIT(&apcap_entry->oper_bss_head);

	/*free assoc_clients_head*/
	assoc_client_db = SLIST_FIRST(&apcap_entry->assoc_clients_head);
	while (assoc_client_db != NULL) {
		assoc_client_db_tmp = SLIST_NEXT(assoc_client_db, assoc_clients_entry);
		debug(DEBUG_TRACE, "free associated_clients_db(bssid="MACSTR"))\n",
			PRINT_MAC(assoc_client_db->bssid));
		/*free basic_cap_db*/
		cli_db = SLIST_FIRST(&assoc_client_db->clients_head);
		while (cli_db != NULL) {
			cli_db_tmp = SLIST_NEXT(cli_db, clients_entry);
			debug(DEBUG_TRACE, "free cli_db mac["MACSTR"]\n",
				PRINT_MAC(cli_db->mac));
			free(cli_db);
			cli_db = cli_db_tmp;
		}
		SLIST_INIT(&assoc_client_db->clients_head);
		free(assoc_client_db);
		assoc_client_db = assoc_client_db_tmp;
	}
	SLIST_INIT(&apcap_entry->assoc_clients_head);

	/*free ch_prefer_head*/
	ch_pre_db = SLIST_FIRST(&apcap_entry->ch_prefer_head);
	while (ch_pre_db != NULL) {
		ch_pre_db_tmp = SLIST_NEXT(ch_pre_db, ch_prefer_entry);
		debug(DEBUG_TRACE, "free prefer_info_entry(id="MACSTR"))\n",
			PRINT_MAC(ch_pre_db->identifier));
		/*free pre_db*/
		pre_db = SLIST_FIRST(&ch_pre_db->prefer_info_head);
		while (pre_db != NULL) {
			pre_db_tmp = SLIST_NEXT(pre_db, prefer_info_entry);
			debug(DEBUG_TRACE, "free pre_db oper_class=%d\n", pre_db->op_class);
			free(pre_db);
			pre_db = pre_db_tmp;
		}
		SLIST_INIT(&ch_pre_db->prefer_info_head);
		free(ch_pre_db);
		ch_pre_db = ch_pre_db_tmp;
	}
	SLIST_INIT(&apcap_entry->ch_prefer_head);

	/*free oper_restrict_head*/
	restriction = SLIST_FIRST(&apcap_entry->oper_restrict_head);
	while (restriction != NULL) {
		restriction_tmp = SLIST_NEXT(restriction, oper_restrict_entry);
		debug(DEBUG_TRACE, "free oper_restrict_db(id="MACSTR"))\n",
			PRINT_MAC(restriction->identifier));
		/*free restrict_db*/
		resdb = SLIST_FIRST(&restriction->restrict_head);
		while (resdb != NULL) {
			resdb_tmp = SLIST_NEXT(resdb, restrict_entry);
			debug(DEBUG_TRACE, "free pre_db oper_class=%d\n", resdb->op_class);
			free(resdb);
			resdb = resdb_tmp;
		}
		SLIST_INIT(&restriction->restrict_head);
		free(restriction);
		restriction = restriction_tmp;
	}
	SLIST_INIT(&apcap_entry->oper_restrict_head);

	apcap_entry->ap_cap.rssi_steer = 0;
	apcap_entry->ap_cap.sta_report_not_cop = 0;
	apcap_entry->ap_cap.sta_report_on_cop = 0;
}

void clear_metric_db(struct metrics_info *metrics_entry)
{
	struct bss_db *bss = NULL, *bss_tmp = NULL;
	struct mrsp_db *mrsp = NULL, *mrsp_tmp = NULL;
	struct esp_db *esp = NULL, *esp_tmp = NULL;
	struct traffic_stats_db *traffic_stats = NULL, *traffic_stats_tmp = NULL;
	struct stats_db *stats = NULL, *stats_tmp = NULL;
	struct link_metrics_db *link_metrics = NULL, *link_metrics_tmp = NULL;
	struct metrics_db *metrics = NULL, *metrics_tmp = NULL;

	/*metrics query*/
	bss = SLIST_FIRST(&metrics_entry->metrics_query_head);
	while (bss != NULL) {
		bss_tmp = SLIST_NEXT(bss, bss_entry);
		debug(DEBUG_TRACE, "free bss_db(id="MACSTR"))\n",
			PRINT_MAC(bss->bssid));
		free(bss);
		bss = bss_tmp;
	}
	SLIST_INIT(&metrics_entry->metrics_query_head);
	metrics_entry->metrics_query_cnt = 0;

	/*metrics response*/
	mrsp = SLIST_FIRST(&metrics_entry->metrics_rsp_head);
	while (mrsp != NULL) {
		mrsp_tmp = SLIST_NEXT(mrsp, mrsp_entry);
		debug(DEBUG_TRACE, "free mrsp_db(id="MACSTR"))\n",
			PRINT_MAC(mrsp->bssid));
		/*free esp_db*/
		esp = SLIST_FIRST(&mrsp->esp_head);
		while (esp != NULL) {
			esp_tmp = SLIST_NEXT(esp, esp_entry);
			debug(DEBUG_TRACE, "free esp_db ac=%d\n", esp->ac);
			free(esp);
			esp = esp_tmp;
		}
		SLIST_INIT(&mrsp->esp_head);
		free(mrsp);
		mrsp = mrsp_tmp;
	}
	SLIST_INIT(&metrics_entry->metrics_rsp_head);
	metrics_entry->metrics_rsp_cnt = 0;

	/*associated sta traffic stats*/
	traffic_stats = SLIST_FIRST(&metrics_entry->traffic_stats_head);
	while (traffic_stats != NULL) {
		traffic_stats_tmp = SLIST_NEXT(traffic_stats, traffic_stats_entry);
		debug(DEBUG_TRACE, "free traffic_stats_db(id="MACSTR"))\n",
			PRINT_MAC(traffic_stats->identifier));
		/*free stats_db*/
		stats = SLIST_FIRST(&traffic_stats->stats_head);
		while (stats != NULL) {
			stats_tmp = SLIST_NEXT(stats, stats_entry);
			debug(DEBUG_TRACE, "free stats_db bytes_sent=%d\n", stats->bytes_sent);
			free(stats);
			stats = stats_tmp;
		}
		SLIST_INIT(&traffic_stats->stats_head);
		free(traffic_stats);
		traffic_stats = traffic_stats_tmp;
	}
	SLIST_INIT(&metrics_entry->traffic_stats_head);

	/*associated sta link metrics*/
	link_metrics = SLIST_FIRST(&metrics_entry->link_metrics_head);
	while(link_metrics != NULL) {
		link_metrics_tmp = SLIST_NEXT(link_metrics, link_metrics_entry);
		debug(DEBUG_TRACE, "free link_metrics_db(id="MACSTR"))\n",
			PRINT_MAC(link_metrics->identifier));
		/*free metrics_db*/
		metrics = SLIST_FIRST(&link_metrics->metrics_head);
		while (metrics != NULL) {
			metrics_tmp = SLIST_NEXT(metrics, metrics_entry);
			debug(DEBUG_TRACE, "free metrics_db erate_downlink=%d\n", metrics->erate_downlink);
			free(metrics);
			metrics = metrics_tmp;
		}
		SLIST_INIT(&link_metrics->metrics_head);
		free(link_metrics);
		link_metrics = link_metrics_tmp;
	}
	SLIST_INIT(&metrics_entry->link_metrics_head);

	memset(metrics_entry->assoc_sta, 0, ETH_ALEN);
	memset(&metrics_entry->assoc_sta_link_metrics, 0, sizeof(struct metrics_db));

	/*unassociated sta link metrics*/
	delete_exist_unlink_metrics_rsp(&metrics_entry->unlink_info);
	if (metrics_entry->unlink_query) {
		free(metrics_entry->unlink_query);
		metrics_entry->unlink_query = NULL;
	}
	if (metrics_entry->bcn_query) {
		free(metrics_entry->bcn_query);
		metrics_entry->bcn_query = NULL;
	}
	if (metrics_entry->bcn_rsp) {
		free(metrics_entry->bcn_rsp);
		metrics_entry->bcn_rsp = NULL;
	}
}

/*
 *When a process terminates, all of its memory is returned to the system,
 *including heap memory allocated by functions in the malloc package.
 *In programs that allocate memory and continue using it until program termination,
 *it is common to omit calls to free(), relying on this behavior to automatically free the memory.
 *This can be especially useful in programs that allocate many blocks of memory,
 *since adding multiple calls to free() could be expensive in terms of CPU time,
 *as well as perhaps being complicated to code
*/
void p1905_database_deinit(struct p1905_managerd_ctx *ctx)
{
	clear_topology_db(&ctx->topology_entry);
	/*free neighbor dev info*/
	clear_topology_neighbor_db(ctx->p1905_neighbor_dev, ctx->itf_number);
	clear_topology_device_db(&ctx->topodev_entry);
	clear_all_exist_radio_basic_capability(ctx);
	clear_ap_capability_db(&ctx->ap_cap_entry);
	/*free map cpnfiguration info*/
	delete_exist_steering_policy(&ctx->map_policy.spolicy);
	delete_exist_metrics_policy(ctx, &ctx->map_policy.mpolicy);
	/*free metric info*/
	clear_metric_db(&ctx->metric_entry);
	/*free buffer*/
	if (ctx->ch_set) {
		free(ctx->ch_set);
		ctx->ch_set = NULL;
	}
	if (ctx->ch_stat) {
		free(ctx->ch_stat);
		ctx->ch_stat = NULL;
	}
	if (ctx->ch_rep) {
		free(ctx->ch_rep);
		ctx->ch_rep = NULL;
	}
	if (ctx->cli_steer_req) {
		free(ctx->cli_steer_req);
		ctx->cli_steer_req = NULL;
	}

	/*free all the agents info*/
	free_all_the_agents_info(&ctx->agent_head);

	debug(DEBUG_OFF, "==>\n");
}

void p1905_interface_init(struct p1905_managerd_ctx *ctx)
{
	unsigned int i = 0;
#ifdef SUPPORT_WIFI
	for(i = 0; i < ctx->itf_number; i++) {
		if(ctx->itf[i].media_type & IEEE802_11_GROUP) {
		    ctx->itf[i].vs_info_length = 10;
		    ctx->itf[i].vs_info = malloc(ctx->itf[i].vs_info_length);
		    memset(ctx->itf[i].vs_info, 0, ctx->itf[i].vs_info_length);
		    memcpy(ctx->itf[i].vs_info, ctx->itf[i].mac_addr, 6);
			if (ctx->itf[i].is_wifi_sta) {
				*(ctx->itf[i].vs_info + 6) = 0x40;    //non-ap station
				memset(ctx->itf[i].vs_info, 0, 6);
			}
		}
	}
#endif

    /*init local bridge capability */
	ctx->br_cap[0].interface_num = ctx->itf_number;
	ctx->br_cap[0].itf_mac_list=malloc(ETH_ALEN * ctx->br_cap[0].interface_num);
	for (i = 0; i < ctx->br_cap[0].interface_num; i++)
		memcpy(ctx->br_cap[0].itf_mac_list + i * ETH_ALEN, ctx->itf[i].mac_addr, ETH_ALEN);

	/*init local non-p1905.1 neighbor device information*/
	for(i = 0; i < ctx->itf_number; i++)
	{
		memcpy(ctx->non_p1905_neighbor_dev[i].local_mac_addr, ctx->itf[i].mac_addr, ETH_ALEN);
	    LIST_INIT(&(ctx->non_p1905_neighbor_dev[i].non_p1905nbr_head));
	}

	/*init local p1905.1 neighbor device information*/
	for(i = 0; i < ctx->itf_number; i++)
	{
		memcpy(ctx->p1905_neighbor_dev[i].local_mac_addr, ctx->itf[i].mac_addr, ETH_ALEN);
		LIST_INIT(&(ctx->p1905_neighbor_dev[i].p1905nbr_head));
	}
}

void p1905_interface_uninit(struct p1905_managerd_ctx *ctx)
{
    int i = 0;

    for(i=0;i<ITF_NUM;i++)
    {
        if(ctx->itf[i].vs_info_length > 0)
            free(ctx->itf[i].vs_info);
    }

    for(i=0;i<BRIDGE_NUM;i++)
    {
        free(ctx->br_cap[i].itf_mac_list);
    }
}

int update_mapfilter_all_eth_interfaces(struct p1905_managerd_ctx *ctx)
{
	int i = 0, ret = 0;
	unsigned char *buf = NULL;
	struct local_itfs *itf = NULL;
	int buf_len = 0;

	buf_len = sizeof(struct local_itfs) + ctx->itf_number * sizeof(struct local_interface);
	buf = os_zalloc(buf_len);

	if (!buf)
		return -1;

	itf = (struct local_itfs*)buf;

	for (i = 0; i < ctx->itf_number; i++) {
		if (ctx->itf[i].media_type == IEEE802_3_GROUP && ctx->itf[i].is_veth != 1) {
			itf->num++;
			os_memcpy(itf->inf[i].name, ctx->itf[i].if_name, IFNAMSIZ);
			os_memcpy(itf->inf[i].mac, ctx->itf[i].mac_addr, ETH_ALEN);
			itf->inf[i].dev_type = ETH;
		}
	}

	if (itf->num)
		ret = mapfilter_set_all_interface(itf);

	os_free(buf);
	return ret;
}

#ifdef MAP_R2
int update_mapfilter_wan_tag(struct p1905_managerd_ctx *ctx, unsigned char status)
{
	int i = 0, ret = 0;

	if (ctx->map_version == DEV_TYPE_R1)
		return ret;

	for (i = 0; i < ctx->itf_number; i++) {
		if (ctx->itf[i].is_wan == 1) {
			ret = mapfilter_set_wan_tag(ctx->itf[i].mac_addr, status);
			debug(DEBUG_ERROR, "set wan(%s) to enable ts vlan tag check\n",
				ctx->itf[i].if_name);
			break;
		}
	}

	return ret;
}
#endif


extern unsigned char dev_send_1905_buf[3072*2];
void discovery_at_boot_up(void *eloop_data, void *user_ctx);


void update_ethenet_port_status_at_bootup(void *eloop_data, void *user_ctx)
{
	int i = 0, status =0;
	struct port_status_info port_status;

	for (i = 0; i < MAX_LAN_PORT_NUM; i++) {
		status = eth_layer_update_eth_port_status(i);
		if (status < 0) {
			debug(DEBUG_ERROR, "get status fail on port(%d) status(%d)\n", i, status);
			continue;
		}
		port_status.port = i;
		port_status.status = status;
		eth_layer_port_data_update_and_notify(eloop_data, (void *)&port_status);
	}
}


#ifdef MAP_R3
void dev_get_frame_info_init(struct p1905_managerd_ctx *ctx)
{
    ctx->frame_buff_switch = 0;
    dl_list_init(&ctx->frame_buff_head);
    debug(DEBUG_ERROR, "success\n");
}


void dev_get_frame_info_deinit(struct p1905_managerd_ctx *ctx)
{
	struct frame_buff_struct *msg_info_entry, *msg_info_entry_next;
    struct frame_list_struct *msg_list_entry, *msg_list_entry_next;

    /* delete all the msg type info and packet list first */
    dl_list_for_each_safe(msg_info_entry, msg_info_entry_next,
        &ctx->frame_buff_head, struct frame_buff_struct, frame_info_list) {

        dl_list_for_each_safe(msg_list_entry, msg_list_entry_next,
            &msg_info_entry->frame_data_list, struct frame_list_struct, data_list) {
            debug(DEBUG_ERROR, "rm frame data from frame list for msg type 0x%04x\n",
                msg_info_entry->msg_type);
            os_free(msg_list_entry->data);
            dl_list_del(&msg_list_entry->data_list);
            os_free(msg_list_entry);
        }

        debug(DEBUG_ERROR, "rm frame info for msg type 0x%04x\n",
            msg_info_entry->msg_type);
		dl_list_del(&msg_info_entry->frame_info_list);
		os_free(msg_info_entry);
    }

    ctx->frame_buff_switch = 0;
    debug(DEBUG_ERROR, "success\n");
}
#endif // MAP_R3

int p1905_managerd_init(struct p1905_managerd_ctx *ctx)
{
	struct ethernet_port_notifier noti;
	int sock = 0;

    assert(ctx != NULL);

	ctx->trx_buf.buf = (unsigned char *)os_malloc(MAX_PKT_LEN);
	if (!ctx->trx_buf.buf) {
		debug(DEBUG_ERROR, "allocate trx buffer fail\n");
		return -1;
	}
	ctx->trx_buf.buf_len = MAX_PKT_LEN;

	ctx->tlv_buf.buf = (unsigned char *)os_zalloc(MAX_PKT_LEN);
	if (!ctx->tlv_buf.buf) {
		debug(DEBUG_ERROR, "allocate tlv buffer fail\n");
		goto fail;
	}
	ctx->tlv_buf.buf_len = MAX_PKT_LEN;

	ctx->reassemble_buf.buf = (unsigned char *)os_zalloc(MAX_PKT_LEN);
	if (!ctx->reassemble_buf.buf) {
		debug(DEBUG_ERROR, "allocate reassembly buffer fail\n");
		goto fail;
	}
	ctx->reassemble_buf.buf_len = MAX_PKT_LEN;

	ctx->revd_tlv.buf  = (unsigned char *)os_malloc(MAX_PKT_LEN);
	if (!ctx->revd_tlv.buf) {
		debug(DEBUG_ERROR, "allocate revd_tlv buffer fail\n");
		goto fail;
	}
	ctx->revd_tlv.buf_len = MAX_PKT_LEN;

	if (cmdu_init(ctx) == -1) {
		debug(DEBUG_ERROR, "cmdu_init fail\n");
		goto fail;
	}

#ifdef SUPPORT_CONTROL_SOCKET
	if (_1905_ctrl_sock_init(ctx) == -1) {
		debug(DEBUG_ERROR, "_1905_ctrl_sock_init fail\n");
		goto fail;
	}
#endif
	p1905_database_init(ctx);

	if (_1905_interface_init(ctx) < 0) {
		debug(DEBUG_ERROR, "_1905_interface_init fail\n");
		goto fail;
	}

	/*if ((ctx->sock_netlink = netlink_init(getpid())) < 0) {
		debug(DEBUG_TRACE, "netlink init fail\n");
	}*/

	sock = mapfilter_netlink_init(getpid());
	if (sock < 0) {
		debug(DEBUG_ERROR, "mapfilter_netlink_init init fail\n");
		goto fail;
	}

    if (ap_autoconfig_init(ctx)) {
		debug(DEBUG_ERROR, "fail to ap_autoconfig_init\n");
		goto fail;
    }

	noti.down_handler = ethernet_pull_out_handler;
	noti.up_handler = ethernet_plug_in_handler;

	if (eth_layer_init((void *)ctx, &noti) < 0) {
		goto fail;
	}

#ifdef DISABLE_SWITCH_POLLING
	ctx->genl_sock = gen_netlink_init();
	ctx->switch_polling = 0;
	if (!ctx->genl_sock) {
		debug(DEBUG_ERROR, "generic netlink init fail; using polling mechanism\n");
		/*init switch port status polling thread*/
		ctx->sock_internal = worker_task_receiver_sock_init("internal_server");
		if (ctx->sock_internal < 0) {
			debug(DEBUG_ERROR, "worker task receiver sock init fail\n");
			goto fail;
		} else {
			if (worker_task_init() < 0) {
				debug(DEBUG_ERROR, "worker_task_init init fail\n");
				goto fail;
			}
		}
		ctx->switch_polling = 1;
	}
	debug(DEBUG_ERROR, "switch_polling(%d)\n", ctx->switch_polling);
#else
	ctx->sock_internal = worker_task_receiver_sock_init("internal_server");
	if (ctx->sock_internal < 0) {
		debug(DEBUG_ERROR, "worker task receiver sock init fail\n");
		goto fail;
	} else {
		if (worker_task_init() < 0) {
			debug(DEBUG_ERROR, "worker_task_init init fail\n");
			goto fail;
		}
	}
#endif

	if (eloop_init()) {
		debug(DEBUG_ERROR, "Failed to initialize event loop");
		goto fail;
	}

	if(update_mapfilter_all_eth_interfaces(ctx)) {
		debug(DEBUG_ERROR, "Error, fail to update all ethernet interface for mapfilter\n");
	}

	mapfilter_ts_onoff(0);

#ifdef MAP_R2
	if (ctx->role == CONTROLLER)
		update_mapfilter_wan_tag(ctx, 1);
	else
		update_mapfilter_wan_tag(ctx, 0);
	traffic_separation_init(ctx);
#ifdef MAP_R3_SP
	if(service_prioritization_init((void*)ctx))
		goto fail;
#endif
#endif

#ifdef MAP_R3
	ctx->encr_buf.buf = (unsigned char *)os_zalloc(MAX_PKT_LEN);
	if (!ctx->encr_buf.buf) {
		debug(DEBUG_ERROR, "allocate encryption buffer fail\n");
		goto fail;
	}
	ctx->encr_buf.buf_len = MAX_PKT_LEN;
	init_akm_capability(ctx);
	security_engine_init(ctx->encr_buf.buf, ctx->encr_buf.buf_len);
	map_dpp_init(ctx);
	if (ctx->MAP_Cer)
		dev_get_frame_info_init(ctx);

    /* init wpa, according to local device role */
	if ((ctx->wpa = wpa_init((void *)ctx, ctx->role)) == NULL) {
		debug(DEBUG_ERROR, "wpa_init failed \n");
		goto fail;
	}
#endif

	if (ctx->role == CONTROLLER)
		if (_1905_read_set_config(ctx, ctx->wts_bss_cfg_file,
			ctx->bss_config, MAX_SET_BSS_INFO_NUM, &ctx->bss_config_num) < 0)
			debug(DEBUG_ERROR, "_1905_read_set_config fail\n");

	dm_init(ctx);

	eloop_register_signal_terminate(p1905_manage_terminate, NULL);

	/*set all socket to eloop*/
	eloop_register_read_sock(ctx->sock_br, cmdu_process, (void *)ctx, NULL);
	eloop_register_read_sock(ctx->_1905_ctrl.sock, library_cmd_process, (void *)ctx, NULL);
	eloop_register_read_sock(ctx->ctrl_sock, ctrl_cmd_process, (void *)ctx, NULL);
	if (ctx->sock_netlink > 0)
		eloop_register_read_sock(ctx->sock_netlink, netlink_event_process, (void *)ctx, NULL);
	if (!ctx->MAP_Cer) {
		eloop_register_timeout(SWITCH_TABLE_DUMP_TIME, 0, non_1905_neighbor_update, (void *)ctx, NULL);
	}
	eloop_register_timeout(PERIODIC_TIMER_SEC, 0, p1905_managerd_periodic, (void *)ctx, NULL);
	eloop_register_timeout(PERIODIC_TIMER_SEC, 0, send_discovery_periodic, (void *)ctx, NULL);
#ifdef DISABLE_SWITCH_POLLING
	if (ctx->switch_polling) {
		eloop_register_read_sock(ctx->sock_internal, worker_task_event_process, (void *)ctx, NULL);
	} else {
		eloop_register_read_sock(ctx->genl_sock->s_fd, genl_netlink_event_process, (void *)ctx, NULL);
		eloop_register_timeout(1, 0, update_ethenet_port_status_at_bootup, (void *)ctx, NULL);
	}
#else
	eloop_register_read_sock(ctx->sock_internal, worker_task_event_process, (void *)ctx, NULL);
#endif
	if (!ctx->MAP_Cer)
		eloop_register_read_sock(sock, mapfilter_event_process, (void *)ctx, NULL);

    return 0;

fail:
	if (ctx->trx_buf.buf) {
		os_free(ctx->trx_buf.buf);
		ctx->trx_buf.buf = NULL;
		ctx->trx_buf.buf_len = 0;
	}
	if (ctx->tlv_buf.buf) {
		os_free(ctx->tlv_buf.buf);
		ctx->tlv_buf.buf = NULL;
		ctx->tlv_buf.buf_len = 0;
	}
	if (ctx->reassemble_buf.buf) {
		os_free(ctx->reassemble_buf.buf);
		ctx->reassemble_buf.buf = NULL;
		ctx->reassemble_buf.buf_len = 0;
	}
#ifdef MAP_R3
	if (ctx->encr_buf.buf) {
		os_free(ctx->encr_buf.buf);
		ctx->encr_buf.buf = NULL;
		ctx->encr_buf.buf_len = 0;
	}
#endif
	if (ctx->revd_tlv.buf) {
		os_free(ctx->revd_tlv.buf);
		ctx->revd_tlv.buf = NULL;
		ctx->revd_tlv.buf_len = 0;
	}
	return -1;
}

#define RECV_FROM_BR      0x01
#define RECV_FROM_VIRTUAL 0x02

int p1905_managerd_run(struct p1905_managerd_ctx *ctx)
{
	eloop_run();
	return 0;
}

int common_process(struct p1905_managerd_ctx *ctx, struct buf_info *buf)
{
	/*if any cmdu need to be transmited , it will be sent here*/
	if (process_cmdu_txq(ctx, buf->buf) < 0) {
		debug(DEBUG_ERROR, "process_cmdu_txq fail\n");
		return -1;
	}

	/*handle message wait queue*/
	handle_message_wait_queue(ctx, buf->buf);

	return 0;
}

void cmdu_process(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;
	struct buf_info *buf = &ctx->trx_buf;
	struct ethhdr *eth_hdr = NULL;
	int len = MAX_PKT_LEN;
	unsigned int j = 0;
	unsigned char intf_mac[6];

	if (sock == ctx->sock_br) {
		len = cmdu_bridge_receive(ctx, ctx->sock_br, buf->buf, MAX_PKT_LEN);
		if (len < 0) {
			debug(DEBUG_ERROR, "cmdu receive failed on %s (%s)\n", ctx->br_name, strerror(errno));
			return;
		}
		eth_hdr = (struct ethhdr *)buf->buf;
		if (get_receive_port_addr(ctx->br_name, eth_hdr->h_source, intf_mac) < 0) {
			debug(DEBUG_ERROR, "get_receive_port_addr fail, source="MACSTR"\n",
				PRINT_MAC(eth_hdr->h_source));
			return;
		}
		for (j = 0; j < ctx->itf_number; j++) {
			if (!os_memcmp(ctx->itf[j].mac_addr, intf_mac, sizeof(intf_mac))) {
				/*add this to tell which inf recv the cmdu*/
				ctx->recent_cmdu_rx_if_number = j;
				debug(DEBUG_TRACE, "recv cmdu from %s\n", ctx->itf[j].if_name);
				break;
			}
		}
		if (j >= ctx->itf_number) {
			debug(DEBUG_ERROR, "no any local interface match the mac("MACSTR")\n",
				PRINT_MAC(intf_mac));
			return;
		}
	}

	if (cmdu_parse(ctx, buf->buf, len) < 0) {
		debug(DEBUG_INFO, "parsing cmdu fail!\n");
		return;
	}

	common_process(ctx, buf);
}

void lldp_process(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;
	struct buf_info *buf = &ctx->trx_buf;
	int length = 0;

	/*reveive LLDPDU, process here*/
	debug(DEBUG_TRACE, "receive lldp message\n");
	length = receive_802_1_bridge_discovery_msg(ctx, sock, buf->buf, MAX_PKT_LEN);
	if (length < 0) {
		debug(DEBUG_ERROR, "receive 802.1 bridge discovery fail\n");
		return;
	}

	if (parse_802_1_bridge_discovery_msg(ctx, sock, buf->buf) < 0) {
		debug(DEBUG_ERROR, "parse 802.1 bridge discovery fail\n");
		return;
	}

	common_process(ctx, buf);
}


void library_cmd_process(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;

	_1905_interface_receive_process(sock, ctx, 0);

	common_process(ctx, &ctx->trx_buf);
}

void mapfilter_event_process(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;
	unsigned char *buf = ctx->trx_buf.buf;
	int recv_len = 0;

	buf = mapfilter_read_nlmsg(buf, MAX_PKT_LEN, &recv_len);

#ifdef MAP_R3_SP
	if (buf) {
		sp_process_learn_complete_event(ctx->role, buf, &ctx->sp_rule_learn_complete_list, ctx->send_tlv,
			&ctx->send_tlv_len);
		if (ctx->role == AGENT) {
			insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
				e_vendor_specific, ++ctx->mid, ctx->cinfo.local_ifname, 0);
			process_cmdu_txq(ctx, ctx->trx_buf.buf);
		}
	}
#endif
}

void ctrl_cmd_process(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;
	struct buf_info *buf = &ctx->trx_buf;

	_1905_ctrl_interface_recv_and_parse(ctx, (char *)buf->buf, MAX_PKT_LEN);

	common_process(ctx, buf);
}

void netlink_event_process(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;
	struct buf_info *buf = &ctx->trx_buf;
	int length = 0;

	length = netlink_event_recv(ctx->sock_netlink, buf->buf, MAX_PKT_LEN);
	if (0 > length) {
       debug(DEBUG_ERROR, "netlink event recv fail\n");
       return;
    }
	buf->buf = get_netlink_data(buf->buf, &length);
//	hex_dump_all("netlink data", buf, length);
	/*add netlink event handler here*/
	netlink_event_handler(eloop_ctx, (struct _1905_netlink_message *)buf->buf, length);

	common_process(ctx, buf);
	return;
}

#ifdef DISABLE_SWITCH_POLLING
void genl_netlink_event_process(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;
	char buf[32] = {0xff};
	struct port_status_info *status = NULL;
	int ret = 0;

	ret = genl_netlink_recv(ctx->genl_sock, buf);
	if (!ret) {
		 debug(DEBUG_TRACE, "genl_netlink_recv success\n");
		 status = (struct port_status_info *)buf;
		 if (status->port != 0xff && status->status != 0xff)
			eth_layer_port_data_update_and_notify(eloop_ctx, (void *)status);
	}
}
#endif

void worker_task_event_process(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_ctx;
	struct buf_info *buf = &ctx->trx_buf;
	int length = 0;

	length = worker_task_receiver_recv(ctx->sock_internal, buf->buf, MAX_PKT_LEN);
	if (0 > length) {
       debug(DEBUG_ERROR, "worker task receiver event recv fail\n");
       return;
    }
	worker_task_receiver_event_handler(eloop_ctx, (struct worker_task_message *)buf->buf, length);

	common_process(ctx, buf);
	return;
}

/*void update_ethenet_port_status_at_bootup(void *eloop_data, void *user_ctx)
{
	static int i = 0;

	debug(DEBUG_ERROR, "get status of switch port[%d]\n", i);
	if(update_switch_port_status_by_netlink(i) < 0) {
		debug(DEBUG_ERROR, "get status of switch port[%d] fail\n", i);
	}
	i++;
	if (i >= MAX_LAN_PORT_NUM)
		return;
	eloop_register_timeout(1, 0, update_ethenet_port_status_at_bootup, eloop_data, NULL);
	return;
}*/

void discovery_at_boot_up(void *eloop_data, void *user_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx*)eloop_data;
	static int i = DISC_RETRY_TIME;
	int j = 0;

	ctx->mid++;
	for (j = 0; j < ctx->itf_number; j++) {
		/*send multicast cmdu to all the ethenet interface */
		if (ctx->itf[j].media_type == IEEE802_3_GROUP) {
			insert_cmdu_txq(p1905_multicast_address, ctx->p1905_al_mac_addr,
				e_topology_discovery_with_vendor_ie, ctx->mid, ctx->itf[j].if_name, 0);
			debug(DEBUG_ERROR, TOPO_PREX"tx multicast discovery(mid-%04x) with vs tlv on %s\n",
				ctx->mid, ctx->itf[j].if_name);
		}
	}
	i--;

	if (i == 0) {
		i = DISC_RETRY_TIME;
		return;
	}

	eloop_register_timeout(3, 0, discovery_at_boot_up, (void *)ctx, NULL);

	return;
}

#ifdef MAP_R3
void buf_huge(
	struct buf_info *buf,
	unsigned int pkt_len,
	unsigned char bcopy)
{
	unsigned char *new_buf = NULL;

	if (pkt_len > MAX_PKT_LEN) {
		/* pkt_len > 20k, buf_len < pkt_len, will malloc 100K */
		if (buf->buf_len < PKT_LEN_100K) {
			/* store cmdu hdr before free */
			new_buf = os_zalloc(PKT_LEN_100K);
			if (new_buf) {
				/* store new buf and len */
				if (bcopy)
					/* copy cmdu hdr + tlv if bcopy*/
					os_memcpy(new_buf, buf->buf, buf->buf_len);
				os_free(buf->buf);
				buf->buf = new_buf;
				buf->buf_len = PKT_LEN_100K;
			}
		}
	}
}

/* reset buf to 20k size */
void buf_reset(struct buf_info *buf)
{
	unsigned char *new_buf = NULL;

	new_buf = os_zalloc(MAX_PKT_LEN);
	if (new_buf) {
		/* new buf, change buf_len */
		os_free(buf->buf);
		buf->buf_len = MAX_PKT_LEN;
		buf->buf = new_buf;
	}
}


void tm_reset_buf_size(void *eloop_data, void *user_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)eloop_data;

	debug(DEBUG_ERROR, "reset 20k buf\n");
	buf_reset(&ctx->trx_buf);
	buf_reset(&ctx->encr_buf);
	buf_reset(&ctx->tlv_buf);
	buf_reset(&ctx->reassemble_buf);
	sec_update_buf(ctx->encr_buf.buf, ctx->encr_buf.buf_len);
}
#endif

void p1905_managerd_uninit(struct p1905_managerd_ctx *ctx)
{
	int i = 0;

    assert(ctx != NULL);

	//netlink_deinit(ctx->sock_netlink);
#ifdef DISABLE_SWITCH_POLLING
	if (ctx->switch_polling) {
		worker_task_deinit();
		worker_task_receiver_sock_deinit(ctx->sock_internal);
	} else {
		eloop_unregister_read_sock(ctx->genl_sock->s_fd);
	}
#else
	worker_task_deinit();
	worker_task_receiver_sock_deinit(ctx->sock_internal);
#endif
	dm_deinit(ctx);
	eth_layer_fini();
#ifdef MAP_R2
	ts_traffic_separation_deinit(ctx);
	update_mapfilter_wan_tag(ctx, 0);
#ifdef MAP_R3_SP
	service_prioritization_deinit((void*)ctx);
#endif
#endif

#ifdef MAP_R3
	if (ctx->MAP_Cer)
		dev_get_frame_info_deinit(ctx);
	eloop_cancel_timeout(tm_reset_buf_size, (void *)ctx, NULL);
	eloop_cancel_timeout(r3_wait_for_dpp_auth_req, (void *)ctx, NULL);
	eloop_cancel_timeout(periodic_send_autoconfig_search, (void *)ctx, NULL);
	eloop_cancel_timeout(r3_config_sm_step, (void *)ctx, NULL);
	eloop_cancel_timeout(r3_set_config, (void *)ctx, NULL);
	eloop_cancel_timeout(map_r2_notify_ts_config, (void *)ctx, NULL);
	eloop_cancel_timeout(map_notify_transparent_vlan_setting, (void *)ctx, NULL);
	eloop_cancel_timeout(cmd_dev_stop_buffer_timeout, (void *)ctx, NULL);
	deinit_akm_capability(ctx);
	security_engine_deinit(ctx);
	if (ctx->encr_buf.buf != NULL) {
		os_free(ctx->encr_buf.buf);
		ctx->encr_buf.buf = NULL;
	}
	map_dpp_deinit(ctx);
	wpa_deinit();
#endif // MAP_R3
	/*deinit all socket from eloop*/
	eloop_unregister_read_sock(ctx->sock_br);
	for (i = 0; i < ctx->itf_number; i++)
		eloop_unregister_read_sock(ctx->sock_lldp[i]);
	eloop_unregister_read_sock(ctx->_1905_ctrl.sock);
	eloop_unregister_read_sock(ctx->ctrl_sock);
	eloop_unregister_read_sock(ctx->sock_netlink);
#ifdef DISABLE_SWITCH_POLLING
	if (ctx->switch_polling)
		eloop_unregister_read_sock(ctx->sock_internal);
	else
		eloop_unregister_read_sock(ctx->genl_sock->s_fd);
#else
	eloop_unregister_read_sock(ctx->sock_internal);
#endif
	eloop_cancel_timeout(ap_controller_search_step, (void *)ctx, NULL);
	eloop_cancel_timeout(ap_autoconfig_enrolle_step, (void *)ctx, NULL);
	eloop_cancel_timeout(metrics_report_timeout, (void *)ctx, NULL);
	eloop_cancel_timeout(p1905_managerd_periodic, (void *)ctx, NULL);

	eloop_destroy();

    p1905_interface_uninit(ctx);
    cmdu_uninit(ctx);
    lldpdu_uninit(ctx);
	p1905_database_deinit(ctx);
	/*deinit topology tree*/
	clear_topology_tree(ctx);
	_1905_interface_deinit(ctx);

	if (ctx->trx_buf.buf != NULL) {
		os_free(ctx->trx_buf.buf);
		ctx->trx_buf.buf = NULL;
	}
	if (ctx->revd_tlv.buf != NULL) {
		os_free(ctx->revd_tlv.buf);
		ctx->revd_tlv.buf = NULL;
	}
	if (ctx->reassemble_buf.buf != NULL) {
		os_free(ctx->reassemble_buf.buf);
		ctx->reassemble_buf.buf = NULL;
	}
	if (ctx->tlv_buf.buf != NULL) {
		os_free(ctx->tlv_buf.buf);
		ctx->tlv_buf.buf = NULL;
	}
	if (ctx->last_tx_data) {
		os_free(ctx->last_tx_data);
		ctx->last_tx_data = NULL;
	}
	if (ctx->last_rx_data) {
		os_free(ctx->last_rx_data);
		ctx->last_rx_data = NULL;
	}
}

unsigned char BtoH(char ch)
{
    if (ch >= '0' && ch <= '9') return (ch - '0');        /* Handle numerals*/
    if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 0xA);  /* Handle capitol hex digits*/
    if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 0xA);  /* Handle small hex digits*/
    return(255);
}


void AtoH(char *src, char *dest, int destlen)
{
    char *srcptr;
    char *destTemp;

    srcptr = src;
    destTemp = dest;

    while(destlen--)
    {
        *destTemp = BtoH(*srcptr++) << 4;    /* Put 1st ascii byte in upper nibble.*/
        *destTemp += BtoH(*srcptr++);      /* Add 2nd ascii byte to above.*/
        destTemp++;
    }
}

char * map_config_get_line(char *s,
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

#ifdef MAP_R2
int parse_transparent_vid(struct p1905_managerd_ctx *ctx, char *str_vids)
{
	char *token = NULL;
	unsigned char i = 0;
	int vid = 0;

	token = strtok(str_vids, ";");

	while (token != NULL) {
		vid = atoi(token);
		debug(DEBUG_OFF, "\transparent vid=%d\n", vid);
		token = strtok(NULL, ";");

		if (vid >= INVALID_VLAN_ID || vid < 1) {
			debug(DEBUG_ERROR, "invalid vlan id %d\n", vid);
			continue;
		}

		ctx->map_policy.trans_vlan.transparent_vids[i++] = vid;
		if (i >= MAX_NUM_TRANSPARENT_VID)
			break;
	}

	ctx->map_policy.trans_vlan.num = i;
	ctx->map_policy.trans_vlan.updated = 1;
	debug(DEBUG_ERROR, "transparent vlan id number=%d\n", ctx->map_policy.trans_vlan.num);

	return 0;
}
#endif

int map_read_config_file(struct p1905_managerd_ctx *ctx)
{
	FILE *file;
	char buf[256], *pos, *token, *token1;
	char tmpbuf[256];
	int line = 0, i = 0;
	unsigned char intf_idx = 0;
	int ret = 0;

	if (strlen(ctx->map_cfg_file) == 0) {
		ret = snprintf(ctx->map_cfg_file, sizeof(ctx->map_cfg_file), "%s", MAP_CFG_FILE);
		if (os_snprintf_error(sizeof(ctx->map_cfg_file), ret)) {
			debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
			return -1;
		}
	}

	file = fopen(ctx->map_cfg_file, "r");
	if (!file) {
		debug(DEBUG_ERROR, "open MAP cfg file (%s) fail\n", ctx->map_cfg_file);
		return -1;
	}
	os_memset(buf, 0, 256);
	os_memset(tmpbuf, 0, 256);

	while (map_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
		if (strlen(pos) < 256) {
			ret = snprintf(tmpbuf, sizeof(tmpbuf), "%s", pos);
			if (os_snprintf_error(sizeof(tmpbuf), ret)) {
				debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
				goto error;
			}
		}
		token = strtok(pos, "=");
		if (token != NULL) {
			if (os_strcmp(token, "map_controller_alid") == 0) {
				if (ctx->role == CONTROLLER) {
					token = strtok(NULL, "");
					if (!token) {
						debug(DEBUG_ERROR, "no map_controller_alid content\n");
						goto error;
					}
					i = 0;
					token1 = strtok(token, ":");
					while (token1 != NULL) {
						AtoH(token1, (char *)&ctx->p1905_al_mac_addr[i], 1);
						i++;
						if (i >= ETH_ALEN)
							break;
						token1 = strtok(NULL, ":");
					}
					debug(DEBUG_OFF, "map_controller_alid = "MACSTR"\n",
						PRINT_MAC(ctx->p1905_al_mac_addr));
				}
			} else if (os_strcmp(token, "map_agent_alid") == 0) {
				if (ctx->role == AGENT) {
					token = strtok(NULL, "");
					if (!token) {
						debug(DEBUG_ERROR, "no map_agent_alid content\n");
						goto error;
					}
					i = 0;
					token1 = strtok(token, ":");
					while (token1 != NULL) {
						AtoH(token1, (char *)&ctx->p1905_al_mac_addr[i], 1);
						i++;
						if (i >= ETH_ALEN)
							break;
						token1 = strtok(NULL, ":");
					}
					debug(DEBUG_OFF, "map_agent_alid = "MACSTR"\n",
						PRINT_MAC(ctx->p1905_al_mac_addr));
				}
			} else if (os_strcmp(token, "br_inf") == 0) {
				token = strtok(NULL, "");
				if (!token) {
					debug(DEBUG_ERROR, "no br_inf content\n");
					goto error;
				}
				ret = snprintf((char *)ctx->br_name, sizeof(ctx->br_name), "%s", token);
				if (os_snprintf_error(sizeof(ctx->br_name), ret)) {
					debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
					goto error;
				}
				debug(DEBUG_OFF, "br_inf = %s\n", ctx->br_name);
			} else if (os_strcmp(token, "lan") == 0) {
				intf_idx = ctx->itf_number;
				token = strtok(NULL, "");
				if (!token)
					continue;
				ret = snprintf((char *)ctx->itf[intf_idx].if_name,
					sizeof(ctx->itf[intf_idx].if_name), "%s", token);
				if (os_snprintf_error(sizeof(ctx->itf[intf_idx].if_name), ret)) {
					debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
					goto error;
				}
				debug(DEBUG_OFF, "lan = %s\n", ctx->itf[intf_idx].if_name);
				ctx->itf[intf_idx].media_type = IEEE802_3_GROUP;
				ctx->itf[intf_idx].is_wifi_sta = 0;
				ctx->itf_number++;
			} else if (os_strcmp(token, "wan") == 0) {
				intf_idx = ctx->itf_number;
				token = strtok(NULL, "");
				if (!token)
					continue;
				ret = snprintf((char *)ctx->itf[intf_idx].if_name,
					sizeof(ctx->itf[intf_idx].if_name), "%s", token);
				if (os_snprintf_error(sizeof(ctx->itf[intf_idx].if_name), ret)) {
					debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
					goto error;
				}
				debug(DEBUG_OFF, "wan = %s\n", ctx->itf[intf_idx].if_name);
				ctx->itf[intf_idx].media_type = IEEE802_3_GROUP;
				ctx->itf[intf_idx].is_wifi_sta = 0;
				ctx->itf[intf_idx].is_wan = 1;
				ctx->itf_number++;
			} else if(os_strcmp(token, "veth") == 0) {
				intf_idx = ctx->itf_number;
				token = strtok(NULL, "");
				if (!token)
					continue;
				ret = snprintf((char *)ctx->itf[intf_idx].if_name,
					sizeof(ctx->itf[intf_idx].if_name), "%s", token);
				if (os_snprintf_error(sizeof(ctx->itf[intf_idx].if_name), ret)) {
					debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
					goto error;
				}
				debug(DEBUG_OFF, "veth = %s\n", ctx->itf[intf_idx].if_name);
				ctx->itf[intf_idx].media_type = IEEE802_3_GROUP;
				ctx->itf[intf_idx].is_wifi_sta = 0;
				ctx->itf[intf_idx].is_veth = 1;
				ctx->itf_number++;
			}
#ifdef MAP_R2
 			else if (os_strcmp(token, "map_ver") == 0) {
 				token = strtok(NULL, "");
				if (!token) {
					debug(DEBUG_ERROR, "no map_ver content\n");
					goto error;
				}
				if (os_strcmp(token, "R1") == 0)
					ctx->map_version = DEV_TYPE_R1;
				else
					ctx->map_version = DEV_TYPE_R2;

#ifdef MAP_R3
				if (os_strcmp(token, "R3") == 0)
					ctx->map_version = DEV_TYPE_R3;
#endif


				debug(DEBUG_OFF, "ctx->map_version = %d\n", ctx->map_version);
			} else if (os_strcmp(token, "transparent_vids") == 0) {
				if (ctx->role == CONTROLLER) {
					token = strtok(NULL, "");
					if (!token)
						continue;
					parse_transparent_vid(ctx, token);
				}
			}
#endif
			else if (os_strcmp(token, "al_inf") == 0) {
				token = strtok(NULL, "");
				if (!token)
					continue;
				os_memset(ctx->al_inf, 0, IFNAMSIZ);
				ret = snprintf((char *)ctx->al_inf, sizeof(ctx->al_inf), "%s", token);
				if (os_snprintf_error(sizeof(ctx->al_inf), ret)) {
					debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
					goto error;
				}
				debug(DEBUG_OFF, "al_inf = %s\n", ctx->al_inf);
			}
			else if (os_strcmp(token, "discovery_cnt") == 0) {
				token = strtok(NULL, "");
				if (!token)
					continue;
				ctx->discovery_cnt = atoi(token);
				debug(DEBUG_OFF, "discovery_cnt = %d\n", ctx->discovery_cnt);
			} else if (os_strcmp(token, "bss_config_priority") == 0) {
				token = strtok(NULL, "");
				if (!token)
					continue;
				if (os_strlen(token) >= (MAX_RADIO_NUM * (IFNAMSIZ + 1) * MAX_BSS_NUM)) {
					debug(DEBUG_ERROR, "bss_config_priority string too long\n");
					continue;
				}
				ret = snprintf(ctx->bss_priority_str, sizeof(ctx->bss_priority_str), "%s", token);
				if (os_snprintf_error(sizeof(ctx->bss_priority_str), ret)) {
					debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
					goto error;
				}
				debug(DEBUG_OFF, "bss_config_priority: %s\n", token);
			}
#ifdef MAP_R3
			else if (os_strcmp(token, "decrypt_fail_threshold") == 0) {
				token = strtok(NULL, "");
				if (!token) {
					debug(DEBUG_ERROR, "[%d]no decrypt_fail_threshold!\n", __LINE__);
					goto error;
				}
				ctx->decrypt_fail_threshold = atoi(token);
				if (ctx->decrypt_fail_threshold <= 0) {
					ctx->decrypt_fail_threshold = DECRYPT_FAULURE_THRESHOLD;
				}
				debug(DEBUG_OFF, "decrypt_fail_threshold = %d\n", ctx->decrypt_fail_threshold);
			}
			else if (os_strcmp(token, "gtk_rekey_interval") == 0) {
				int interal = 0;
				token = strtok(NULL, "");
				if (!token) {
					debug(DEBUG_ERROR, "[%d]no gtk_rekey_interval!\n", __LINE__);
					goto error;
				}

				interal = atoi(token);
				if (interal <= 0) {
					interal = 100;
				}

				ctx->gtk_rekey_interval = (unsigned int)interal;
			}
#endif
		}
	}

	(void)fclose(file);

	if (ctx->discovery_cnt <= 0 || ctx->discovery_cnt > 60) {
		ctx->discovery_cnt = TOPOLOGY_DISCOVERY_COUNT;
		debug(DEBUG_OFF, "set discovery_cnt to defaut value(%d)\n",
			ctx->discovery_cnt);
	}

	return 0;

error:
	(void)fclose(file);
	return -1;
}


int init_global_var_by_role(struct p1905_managerd_ctx *ctx)
{

	/*init config info*/
	if (map_read_config_file(ctx) != 0) {
		debug(DEBUG_ERROR, "map_read_config_file fail\n");
		return -1;
	}

	return 0;
}

#ifdef SUPPORT_BACKTRACE
void _1905_sigsegv_handler(int sig, void *signal_ctx)
{
	void *array[20];
	size_t size;

	debug(DEBUG_OFF, "sig(%d)!!!!!!!!!!\n",sig);

	// get void*'s for all entries on the stack
	size = backtrace(array, 20);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

void _1905_enable_backtrack()
{
	eloop_register_signal(SIGSEGV, _1905_sigsegv_handler, NULL);
}
#else
void _1905_enable_backtrack() { }
#endif

#ifdef SUPPORT_COREDUMP
void _1905_enable_core_dump()
{
	struct rlimit limit;
	debug(DEBUG_OFF, "enabling core dump");
	limit.rlim_cur = 1024 * 1024 * 500;
	limit.rlim_max = 1024 * 1024 * 500;
	setrlimit(RLIMIT_CORE, &limit);
}
#else
void _1905_enable_core_dump() { }
#endif


/**
 * Main function
 *
 * \param  argc  number of arguments.
 * \param  argv  arguments pointer.
 * \return  error code.
 */
int main(int argc, char *argv[])
{
	struct p1905_managerd_ctx *ctx = NULL;
	unsigned int ctx_len = 0;
    int result = 0;
	int c = 0;
	int ret = 0;

	debug(DEBUG_OFF, "Current Version %s\n", VERSION_1905);

	ctx_len = sizeof(*ctx);
	ctx = os_zalloc(ctx_len);
	if (!ctx) {
		debug(DEBUG_ERROR, "alloc ctx fail\n");
		return -1;
	}
	debug(DEBUG_ERROR, "alloc ctx size(%d)\n", ctx_len);
	 //Open syslog file
    openlog("p1905_managerd", LOG_PID, LOG_DAEMON);

	while((c = getopt(argc, argv, "d:i:r:BGMf:F:")) != -1)
	{
		switch(c)
		{
			case 'd':
			{
				debug_level = optarg[0] - '0';
				debug(DEBUG_OFF, "debug level = %d\n", debug_level);
				break;
			}
			case 'r':
			{
				ctx->role = optarg[0] - '0';
				if (ctx->role > 1) {
					debug(DEBUG_ERROR, "role must be 0(controller) or 1(agent) fail!!!!\n");
					result = -1;
					goto error;
				} else {
					debug(DEBUG_OFF, "set 1905 role %s\n", (ctx->role == 0)?"controller":"agent");
					break;
				}
			}
			case 'B':
			{
				ctx->backtrace = 1;
				debug(DEBUG_OFF, "use backtrace\n");
				break;
			}
			case 'G':
			{
				ctx->core_dump= 1;
				debug(DEBUG_OFF, "use gdb to parse core_dump\n");
				break;
			}
			case 'f':
			{
				ret = snprintf(ctx->map_cfg_file, sizeof(ctx->map_cfg_file), "%s", optarg);
				if (os_snprintf_error(sizeof(ctx->map_cfg_file), ret)) {
					debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
					result = -1;
					goto error;
				}
				debug(DEBUG_OFF, "map cfg file %s\n", ctx->map_cfg_file);
			}
				break;

			case 'F':
			{
				ret = snprintf(ctx->wts_bss_cfg_file, sizeof(ctx->wts_bss_cfg_file), "%s", optarg);
				if (os_snprintf_error(sizeof(ctx->wts_bss_cfg_file), ret)) {
					debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
					result = -1;
					goto error;
				}
				debug(DEBUG_OFF, "wts bss cfg file %s\n", ctx->wts_bss_cfg_file);
			}
				break;
			case 'M':
			{
				ctx->MAP_Cer = 1;
				debug(DEBUG_OFF, "MAP Certification mode\n");
			}
				break;
			default:
				break;
		}
	}


	if (ctx->core_dump) {
		_1905_enable_core_dump();
	} else if (ctx->backtrace) {
		_1905_enable_backtrack();
	}

	if (0 == os_strlen(ctx->map_cfg_file)) {
		ret = os_snprintf(ctx->map_cfg_file, sizeof(ctx->map_cfg_file), MAP_CFG_FILE);
		if (os_snprintf_error(sizeof(ctx->map_cfg_file), ret)) {
			debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
			result = -1;
			goto error;
		}
		debug(DEBUG_OFF, "map cfg file %s\n", ctx->map_cfg_file);
	}
	if (0 == os_strlen(ctx->wts_bss_cfg_file)) {
		ret = os_snprintf(ctx->wts_bss_cfg_file, sizeof(ctx->wts_bss_cfg_file), MAP_WTS_BSS_CFG_FILE);
		if (os_snprintf_error(sizeof(ctx->wts_bss_cfg_file), ret)) {
			debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
			result = -1;
			goto error;
		}
		debug(DEBUG_OFF, "wts bss cfg file %s\n", ctx->wts_bss_cfg_file);
	}

	if (map_read_config_file(ctx) != 0) {
		debug(DEBUG_ERROR, "map_read_config_file fail\n");
		result = -1;
		goto error;
	}

    //Initialize manager daemon
    if(0 != p1905_managerd_init(ctx))
    {
        debug(DEBUG_ERROR, "p1905_managerd_init fail!\n");
        result = -1;
        goto error;
    }

    debug(DEBUG_OFF, "p1905_Manager Daemon Running\n");

    //Start manager daemon core
    p1905_managerd_run(ctx);
    debug(DEBUG_OFF, "p1905_Manager Daemon exiting\n");


    //Uninitialize p1905_manager daemon
    p1905_managerd_uninit(ctx);
error:
    //close syslog file
    closelog();
	if (ctx)
		free(ctx);
    return result;
}
