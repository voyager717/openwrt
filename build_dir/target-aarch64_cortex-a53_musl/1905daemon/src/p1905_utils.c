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
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <syslog.h>
#include <assert.h>
#include <linux/if_ether.h>
#include "p1905_utils.h"
#include "cmdu_tlv.h"
#include <sys/queue.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <netinet/in.h>
#include "debug.h"
#include "topology.h"

#define MAX_FDB_ENTRIES 350
#define MAX_FDB_ENTRIES_ONE_TIME 256

#define PATH_PLC0_PORT_NO "/sys/class/net/plc0/brport/port_no"
#define PATH_ETH0_PORT_NO "/sys/class/net/eth0/brport/port_no"
#ifdef SUPPORT_WIFI
#define PATH_WIFI0_PORT_NO "/sys/class/net/ra0/brport/port_no"
#endif

#define BRCTL_GET_RECV_PORT_ADDR 19
#define BRCTL_SET_BRIDGE_NOT_FORWARD_DEST 20
#define MAP_SO_BASE 1905


int get_receive_port_addr(unsigned char *brname, unsigned char *inputmac,unsigned char *outputmac)
{
	int fd = 0, ret = 0;
	int size = ETH_ALEN;
	unsigned char mac[ETH_ALEN] = {0};

	fd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
	if (fd < 0) {
		debug(DEBUG_ERROR, "socket create fail\n");
		return -1;
	}
	memcpy(mac, inputmac, ETH_ALEN);
	ret = getsockopt(fd, IPPROTO_IP, MAP_SO_BASE, mac, (socklen_t *)&size);
	close(fd);

	if (ret < 0) {
		debug(DEBUG_ERROR, "fail! ret(%d)\n", ret);
		return -1;
	}
	memcpy(outputmac, mac, ETH_ALEN);

	return 0;
}

int set_opt_not_forward_dest(unsigned char *inputmac)
{
	int fd = 0, ret = 0;
	int size = ETH_ALEN;
	unsigned char mac[ETH_ALEN] = {0};

	fd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
	if (fd < 0) {
		debug(DEBUG_ERROR, "create socket error\n");
		return -1;
	}
	memcpy(mac, inputmac, ETH_ALEN);
	debug(DEBUG_TRACE, "not forward mac("MACSTR")\n",
		PRINT_MAC(mac));
	ret = setsockopt(fd, IPPROTO_IP, MAP_SO_BASE, mac, (socklen_t)size);
	close(fd);

	if (ret < 0) {
		debug(DEBUG_ERROR, "fail! ret(%d)\n", ret);
		return -1;
	}

	return 0;
}

void dump_topology_info(struct p1905_managerd_ctx *ctx)
{
	struct topology_discovery_db *tpg_db;
	struct topology_response_db  *tpr_db;
	struct device_info_db *dev_info;
	struct p1905_neighbor_device_db *p1905_dev;
	struct non_p1905_neighbor_device_list_db *non_p1905_dev;
	struct device_bridge_capability_db *br_cap;
	unsigned char *mac = NULL;
	struct agent_list_db *agent_info = NULL;
	int i = 0;
	int j = 0;
	int z = 0;
	int a = 0, b = 0;

	debug(DEBUG_OFF, "====================topology discovery====================\n");
	if (!LIST_EMPTY(&ctx->topology_entry.tpddb_head)) {
	    LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry)
	    {
	        i++;
	        debug(DEBUG_OFF, "neighbor%d AL MAC: "MACSTR"\n", i, PRINT_MAC(tpg_db->al_mac));
	       	debug(DEBUG_OFF, "neighbor%d con-intf MAC: "MACSTR"\n", i, PRINT_MAC(tpg_db->itf_mac));
	        debug(DEBUG_OFF, "neighbor%d recv_intf MAC: "MACSTR" eth_port[%d]\n",
				i, PRINT_MAC(tpg_db->receive_itf_mac), tpg_db->eth_port);
	    }
	} else {
	    debug(DEBUG_OFF, "!!!no topology discovery\n");
	}

	debug(DEBUG_OFF, "====================topology response====================\n");
	if (!SLIST_EMPTY(&ctx->topology_entry.tprdb_head)) {
	    i = 0;
	    SLIST_FOREACH(tpr_db, &ctx->topology_entry.tprdb_head, tprdb_entry)
	    {
	        i++;
			debug(DEBUG_OFF, "-------------------------------\n");
	        debug(DEBUG_OFF, "device%d AL MAC("MACSTR") service(%d)\n",
				i, PRINT_MAC(tpr_db->al_mac_addr), tpr_db->support_service);
#ifdef MAP_R2
			debug(DEBUG_OFF, "multi-ap profile=%02x\n", tpr_db->profile);
#endif
	        if (!SLIST_EMPTY(&tpr_db->devinfo_head)) {
	            SLIST_FOREACH(dev_info, &tpr_db->devinfo_head, devinfo_entry)
	            {
	                j++;
					debug(DEBUG_OFF, "-------------------------------\n");
					debug(DEBUG_OFF, "interface%d("MACSTR") media type(0x%04x)\n",
						j, PRINT_MAC(dev_info->mac_addr), dev_info->media_type);
	                if (dev_info->vs_info_len > 0)
						hex_dump_all("media specific info", dev_info->vs_info, dev_info->vs_info_len);
					debug(DEBUG_OFF, "-------------------------------\n");
					debug(DEBUG_OFF, "p1905 neighbor device:\n");
	                if (!SLIST_EMPTY(&dev_info->p1905_nbrdb_head)) {
	                    SLIST_FOREACH(p1905_dev, &dev_info->p1905_nbrdb_head, p1905_nbrdb_entry)
	                    {
	                        z++;
							debug(DEBUG_OFF, "neighbor%d("MACSTR") 802.1 bridge exist(%d)\n",
								z, PRINT_MAC(p1905_dev->p1905_neighbor_al_mac),
								p1905_dev->ieee_802_1_bridge_exist);
	                    }
	                    z = 0;

	                }
					debug(DEBUG_OFF, "-------------------------------\n");
					debug(DEBUG_OFF, "p1905 non neighbor device:\n");
	                if (!SLIST_EMPTY(&dev_info->non_p1905_nbrdb_head)) {
	                    SLIST_FOREACH(non_p1905_dev, &dev_info->non_p1905_nbrdb_head, non_p1905_nbrdb_entry)
	                    {
	                        z++;
							debug(DEBUG_OFF, "non neighbor%d("MACSTR")\n",
								z, PRINT_MAC(non_p1905_dev->non_p1905_device_interface_mac));
	                    }
	                    z = 0;
	                }
	            }
	            j = 0;

	        }
			debug(DEBUG_OFF, "-------------------------------\n");
	        if (!LIST_EMPTY(&tpr_db->brcap_head)) {
	            LIST_FOREACH(br_cap, &tpr_db->brcap_head, brcap_entry)
	            {
	                a++;
	                debug(DEBUG_OFF, "bridge%d interface amount(%d)\n", a, br_cap->interface_amount);
	                if (br_cap->interface_amount > 0) {
	                    for(b = 0; b < br_cap->interface_amount; b++) {
							mac = br_cap->interface_mac_tuple + b * ETH_ALEN;
							debug(DEBUG_OFF, "interface_mac%d("MACSTR")\n",b, MAC2STR(mac));
	                    }
	                }
	            }
#ifdef MAP_R2
				debug(DEBUG_OFF, "multi-ap profile=%02x\n", tpr_db->profile);
#endif
	        }
	    }
	} else {
	    debug(DEBUG_OFF, "!!!no topology response\n");
	}
	debug(DEBUG_OFF, "====================agent list db====================\n");
	SLIST_FOREACH(agent_info, &(ctx->agent_head), agent_entry)
	{
		debug(DEBUG_OFF, "AL MAC: "MACSTR", profile:%02x, radio number:%d\n",
			PRINT_MAC(agent_info->almac), agent_info->profile, agent_info->radio_num);
		for (i = 0; i < agent_info->radio_num; i++) {
			debug(DEBUG_OFF, "\tradio[%d]:"MACSTR", band:%02x, conf_state:%d, max_bss_num=%d,"
				" opclass_number:%d, doing_wsc=%d\n", i, PRINT_MAC(agent_info->ra_info[i].identifier),
				agent_info->ra_info[i].band, agent_info->ra_info[i].conf_ctx.config_status,
				agent_info->ra_info[i].max_bss_num, agent_info->ra_info[i].op_class_num,
				agent_info->ra_info[i].doing_wsc);
		}
	}
	debug(DEBUG_OFF, "\n\n");
}


void print_tree_leaf_handler(void *context, void* parent_leaf, void *current_leaf, void *data)
{
	struct leaf *parent = (struct leaf*)parent_leaf;
	struct leaf *current = (struct leaf*)current_leaf;
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)context;

	if (ctx->role == CONTROLLER) {
		 debug(DEBUG_OFF, "\t\033[1m\033[40;31m (%p)("MACSTR")------>[current](%p)("MACSTR")\033[0m\n",
		 	parent, PRINT_MAC(parent->al_mac), current, PRINT_MAC(current->al_mac));

	} else {
		debug(DEBUG_OFF, "\t\033[1m\033[40;31m (%p)("MACSTR")------>(%p)("MACSTR")\033[0m\n",
			parent, PRINT_MAC(parent->al_mac), current, PRINT_MAC(current->al_mac));
	}
}

void dump_topology_tree(struct p1905_managerd_ctx *ctx)
{
	trace_topology_tree_cb(ctx,ctx->root_leaf, print_tree_leaf_handler, NULL);
}

void dump_radio_info(struct p1905_managerd_ctx *ctx)
{
	unsigned char radio_index = 0, bss_index = 0, if_index = 0, ch_index = 0;
	struct radio_info *r = NULL;
	struct p1905_interface *itf = NULL;
	struct operating_class *opc = NULL;

	debug(DEBUG_OFF, "====================ALMAC====================\n");
	debug(DEBUG_OFF, "My almac "MACSTR"\n", PRINT_MAC(ctx->p1905_al_mac_addr));
	debug(DEBUG_OFF, "\n\n");


	debug(DEBUG_OFF, "====================RADIO INFO====================\n");
	debug(DEBUG_OFF, "total radio number: %d\n", ctx->radio_number);
	for (radio_index = 0; radio_index < ctx->radio_number; radio_index++) {
		r = &ctx->rinfo[radio_index];
		debug(DEBUG_OFF, "radio%d:      "MACSTR"\n",radio_index, PRINT_MAC(r->identifier));
		debug(DEBUG_OFF, "----------attribut:\n");
		debug(DEBUG_OFF, "teared_down: %d   trrigerd_autoconf: %d\n", r->teared_down, r->trrigerd_autoconf);
		debug(DEBUG_OFF, "band: %d   config_status: %d   bss_number: %d\n", r->band, r->config_status, r->bss_number);
		debug(DEBUG_OFF, "----------bss list:\n");
		for (bss_index = 0; bss_index < r->bss_number; bss_index++) {
			debug(DEBUG_OFF, "name: %s   mac: "MACSTR"   config_status(%d)   priority(%d)\n",
				r->bss[bss_index].ifname,
				PRINT_MAC(r->bss[bss_index].mac),
				r->bss[bss_index].config_status,
				r->bss[bss_index].priority
				);
#ifdef MAP_R2
			debug(DEBUG_OFF, "ssid: %s   AuthMode: %d   EncrypType: %d\n",
				r->bss[bss_index].config.Ssid,
				r->bss[bss_index].config.AuthMode,
				r->bss[bss_index].config.EncrypType
			);
			debug(DEBUG_OFF, "WPAKey: %s role: %02x   hidden_ssid: %d\n",
				r->bss[bss_index].config.WPAKey,
				r->bss[bss_index].config.map_vendor_extension,
				r->bss[bss_index].config.hidden_ssid
			);
#endif
		}
		debug(DEBUG_OFF, "----------radio basic capability: max bss number:%d operating class number:%d\n",
			r->basic_cap.max_bss_num, r->basic_cap.op_class_num);
		dl_list_for_each(opc, &r->basic_cap.op_class_list, struct operating_class, entry) {
			debug(DEBUG_OFF, "opclass:%d   max_tx_pwr=%d   non_operch_num=%d\n",
				opc->class_number, opc->max_tx_pwr, opc->non_operch_num);
			debug(DEBUG_OFF, "\tnon_operch_list:\n");
			for (ch_index = 0; ch_index < opc->non_operch_num; ch_index++)
				debugbyte(DEBUG_OFF, "%d \n", opc->non_operch_list[ch_index]);
			debug(DEBUG_OFF, "\n");
		}
	}
	debug(DEBUG_OFF, "\n\n");
	debug(DEBUG_OFF, "====================INTERFACE INFO====================\n");
	debug(DEBUG_OFF, "total interface number: %d\n", ctx->itf_number);
	for (if_index = 0; if_index < ctx->itf_number; if_index++) {
		itf = &ctx->itf[if_index];
		debug(DEBUG_OFF, "interface%d:   mac_addr: "MACSTR"   if_name: %s   media_type: %04x\n",
			if_index, PRINT_MAC(itf->mac_addr), itf->if_name, itf->media_type);
		if (itf->is_wifi_ap || itf->is_wifi_sta) {
			debug(DEBUG_OFF, "channel_freq: %d   channel_bw: %d\n",
				itf->channel_freq, itf->channel_bw);
		}
		if (itf->is_wifi_sta) {
			debug(DEBUG_OFF, "BSSID: "MACSTR"\n", PRINT_MAC(itf->vs_info));
		}
		if (itf->is_wan) {
			debug(DEBUG_OFF, "WAN interface\n");
		}
		if (itf->is_veth) {
			debug(DEBUG_OFF, "VETH interface\n");
		}
	}
	debug(DEBUG_OFF, "\n\n");
	debug(DEBUG_OFF, "====================DESCRIPTION====================\n");
	debug(DEBUG_OFF, "----------total radio cap:\n");
	debug(DEBUG_OFF, "#define RADIO_2G_CAP         0x01\n");
	debug(DEBUG_OFF, "#define RADIO_5GL_CAP        0x02\n");
	debug(DEBUG_OFF, "#define RADIO_5GH_CAP        0x04\n");
	debug(DEBUG_OFF, "#define RADIO_5G_CAP         0x08\n");

	debug(DEBUG_OFF, "----------AuthMode:\n");
	debug(DEBUG_OFF, "#define AUTH_OPEN            0x0001\n");
	debug(DEBUG_OFF, "#define AUTH_WPA_PERSONAL    0x0002\n");
	debug(DEBUG_OFF, "#define AUTH_WPA2_PERSONAL   0x0020\n");
	debug(DEBUG_OFF, "#define AUTH_SAE_PERSONAL    0x0040\n");

	debug(DEBUG_OFF, "----------EncrypType:\n");
	debug(DEBUG_OFF, "#define ENCRYP_NONE          0x0001\n");
	debug(DEBUG_OFF, "#define ENCRYP_TKIP          0x0004\n");
	debug(DEBUG_OFF, "#define ENCRYP_AES           0x0008\n");

	debug(DEBUG_OFF, "----------role:\n");
	debug(DEBUG_OFF, "#define BIT_BH_STA           BIT(7)\n");
	debug(DEBUG_OFF, "#define BIT_BH_BSS           BIT(6)\n");
	debug(DEBUG_OFF, "#define BIT_FH_BSS           BIT(5)\n");
	debug(DEBUG_OFF, "#define BIT_TEAR_DOWN        BIT(4)\n");

	debug(DEBUG_OFF, "----------media_type:\n");
	debug(DEBUG_OFF, "#define IEEE802_3_GROUP      0x0000\n");
	debug(DEBUG_OFF, "#define IEEE802_11_GROUP     0x0100\n");
}




int is_in_exclude_port_list(int port, int *exclude_port, int num)
{
	int i = 0;

	for (i =0; i < num; i++) {
		if (port == exclude_port[i])
			return 1;
	}

	return 0;
}

/*debug cmd.*/
void show_PON_dev(struct p1905_managerd_ctx *ctx)
{
	struct topology_discovery_db *tpg_db = NULL;
	struct topology_discovery_db *tpg_db_tmp = NULL, *ntpg_db_tmp = NULL;
	int i = 0, j = 0;
	int exclude_port[7] = {-1};

	if (ctx->role != CONTROLLER) {
		debug(DEBUG_OFF, "need input this cmd on controller\n");
		return;
	}

    LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry)
    {
		i = 0;
		if (tpg_db->eth_port != -1 &&
			!is_in_exclude_port_list(tpg_db->eth_port, exclude_port, 7)) {
			tpg_db_tmp = LIST_NEXT(tpg_db, tpddb_entry);
			while(tpg_db_tmp) {
				ntpg_db_tmp = LIST_NEXT(tpg_db_tmp, tpddb_entry);
				if (tpg_db_tmp->eth_port == tpg_db->eth_port)
					i++;
				tpg_db_tmp = ntpg_db_tmp;
			}
			if (i >= 1) {
				exclude_port[j++] = tpg_db->eth_port;
				tpg_db_tmp = LIST_FIRST(&ctx->topology_entry.tpddb_head);
				while(tpg_db_tmp) {
					ntpg_db_tmp = LIST_NEXT(tpg_db_tmp, tpddb_entry);
					if (tpg_db_tmp->eth_port == tpg_db->eth_port) {
						 debug(DEBUG_OFF, "DEV("MACSTR")"
						 	"conncet to PON\n", PRINT_MAC(tpg_db_tmp->al_mac));
					}
					tpg_db_tmp = ntpg_db_tmp;
				}
				debug(DEBUG_OFF, "DEV("MACSTR")"
						 	"conncet to PON\n", PRINT_MAC(ctx->p1905_al_mac_addr));
				debug(DEBUG_OFF, "controller conncet port(%d)\n", tpg_db->eth_port);
			}
		}
    }
}

