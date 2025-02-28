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
#include <syslog.h>
#include <assert.h>
#include <linux/if_ether.h>
#include <asm/byteorder.h>
#include <stdlib.h>
#include "p1905_managerd.h"
#include "cmdu_message.h"
#include "cmdu_tlv_parse.h"
#include "cmdu.h"
#include "os.h"
#include "common.h"
#include "debug.h"
#include "_1905_lib_io.h"
#include "file_io.h"

#include "topology.h"
#include "multi_ap.h"
#include "eloop.h"
#ifdef MAP_R3
#include "security_engine.h"
#include "wpa_extern.h"
#include "map_dpp.h"
#endif

struct _find_leaf {
	unsigned char al_mac[ETH_ALEN];
	unsigned char find_result;
};

struct leaf *create_leaf(unsigned char *al_mac)
{
	struct leaf *node = NULL;

	node = (struct leaf*)os_malloc(sizeof(struct leaf));
	if (!node) {
		debug(DEBUG_ERROR, "allocate leaf memory fail for ("MACSTR")\n",
			PRINT_MAC(al_mac));
		return NULL;
	}

	dl_list_init(&node->children);
	os_memcpy(node->al_mac, al_mac, ETH_ALEN);
	os_get_time(&node->create_time);
	node->renew_send_round = 0;
	return node;
}


void free_leaf(struct p1905_managerd_ctx *ctx, struct leaf *del_leaf)
{
	struct agent_list_db *agent_info = NULL;
#ifdef MAP_R3
	unsigned int wait_time = 30;
	struct r3_member *peer = NULL;

	if (ctx->MAP_Cer) {
		wait_time = 300;
		/* don't reset tx/rx counter in turnkey
		** tx rx counter may mismatch while plug in/out cable frequently
		*/
		ptk_peer_reset_encrypt_txrx_counter(del_leaf->al_mac);
	}
	peer = get_r3_member(del_leaf->al_mac);
	if (peer) {
		debug(DEBUG_ERROR, "del R3 sec info after %d sec\n", wait_time);
		eloop_cancel_timeout(timeout_delete_r3_member,
			(void *)ctx, (void *)peer);
		eloop_register_timeout(wait_time, 0,
			timeout_delete_r3_member, (void *)ctx, (void *)peer);
	}
#endif

	find_agent_info(ctx, del_leaf->al_mac, &agent_info);
	if (agent_info) {
		/*reset the band configured status of agent which leaves from current network*/
		debug(DEBUG_ERROR, TOPO_PREX"reset agent("MACSTR") config states\n",
			PRINT_MAC(agent_info->almac));
		clear_agent_all_radio_config_stat(agent_info);
	}
	dl_list_del(&del_leaf->entry);
	os_free(del_leaf);
}

struct topology_response_db *lookup_tprdb_by_almac(struct p1905_managerd_ctx *ctx, unsigned char *al_mac)
{
	struct topology_response_db *rpdb = NULL;

	SLIST_FOREACH(rpdb, &ctx->topology_entry.tprdb_head, tprdb_entry) {
		if (!os_memcmp(rpdb->al_mac_addr, al_mac, ETH_ALEN))
			break;
	}

	return rpdb;
}

unsigned char is_neighbor(struct topology_response_db *rpdb, unsigned char *al_mac)
{
	struct device_info_db *dev_info = NULL;
	struct p1905_neighbor_device_db *neighbor_db = NULL;

	SLIST_FOREACH(dev_info, &rpdb->devinfo_head, devinfo_entry) {
		SLIST_FOREACH(neighbor_db, &dev_info->p1905_nbrdb_head, p1905_nbrdb_entry) {
			if (!os_memcmp(neighbor_db->p1905_neighbor_al_mac, al_mac, ETH_ALEN))
				return 1;
		}
	}

	return 0;
}

struct leaf *lookup_childleaf_by_almac(struct leaf *current_leaf, unsigned char *almac)
{
	struct leaf *child_leaf = NULL;
	unsigned char found = 0;

	dl_list_for_each(child_leaf, &current_leaf->children, struct leaf, entry) {
		if (!os_memcmp(child_leaf->al_mac, almac, ETH_ALEN)) {
			found = 1;
			break;
		}
	}
	if (found == 0)
		child_leaf = NULL;

	return child_leaf;
}

void find_leaf_handler(void *context, void* parent_leaf, void *current_leaf, void *data)
{
	struct leaf *cur_leaf = (struct leaf *)current_leaf;
	struct _find_leaf *find_data = (struct _find_leaf *)data;

	if (find_data->find_result == 1)
		return;

	if (!os_memcmp(cur_leaf->al_mac, find_data->al_mac, ETH_ALEN))
		find_data->find_result = 1;
}

unsigned char is_leaf_exist(struct p1905_managerd_ctx * ctx, struct leaf *current_leaf, unsigned char *almac)
{
	struct _find_leaf find;

	os_memset(&find, 0, sizeof(struct _find_leaf));
	os_memcpy(find.al_mac, almac, ETH_ALEN);

	trace_topology_tree_cb(ctx, current_leaf, (topology_tree_cb_func)find_leaf_handler, (void*)&find);
	return find.find_result;
}

unsigned char build_tree_from_current_leaf(struct p1905_managerd_ctx *ctx, struct topology_response_db *current_rsp, struct leaf *current_leaf, struct leaf *parent_leaf)
{
	struct device_info_db *dev_info = NULL;
	struct p1905_neighbor_device_db *neighbor_db = NULL;
	struct topology_response_db *child_rpdb = NULL;
	struct leaf *child_leaf = NULL;

	SLIST_FOREACH(dev_info, &current_rsp->devinfo_head, devinfo_entry) {
		SLIST_FOREACH(neighbor_db, &dev_info->p1905_nbrdb_head, p1905_nbrdb_entry) {
			/*1. if neighbor exist in topology response db
			   2. if neighbor's 1905 neighbor tlv match the current devices's al mac and no leaf exist in current node
			       then create the child leaf
			   2.5 check whether this leaf exist, if yes, do not add this one until the exist one has deleted
			   3. and from this new/existed child leaf, buld all its child leaves*/
			child_rpdb = NULL;
			child_leaf = NULL;
			/*skip the parent leaf*/
			if (!os_memcmp(neighbor_db->p1905_neighbor_al_mac, parent_leaf->al_mac, ETH_ALEN))
				continue;
			/*1.*/
			child_rpdb = lookup_tprdb_by_almac(ctx, neighbor_db->p1905_neighbor_al_mac);
			if (!child_rpdb) {
				debug(DEBUG_TRACE, TOPO_PREX"almac("MACSTR") not exist in my topology rsp db\n",
					PRINT_MAC(neighbor_db->p1905_neighbor_al_mac));
				continue;
			}

			/*2.*/
			if (!is_neighbor(child_rpdb, current_leaf->al_mac)) {
				debug(DEBUG_TRACE, TOPO_PREX"child dev("MACSTR") is not a neighbor of dev("MACSTR")\n",
					PRINT_MAC(child_rpdb->al_mac_addr),
					PRINT_MAC(current_leaf->al_mac));
				continue;
			}

			child_leaf = lookup_childleaf_by_almac(current_leaf, child_rpdb->al_mac_addr);
			if (!child_leaf) {
				/*2.5*/
				if (is_leaf_exist(ctx, ctx->root_leaf, child_rpdb->al_mac_addr)) {
					debug(DEBUG_TRACE, TOPO_PREX"current leaf("MACSTR") exist, do not create\n",
						PRINT_MAC( child_rpdb->al_mac_addr));
					continue;
				}

				child_leaf = create_leaf(child_rpdb->al_mac_addr);

				if(!child_leaf)
					continue;
				debug(DEBUG_ERROR, TOPO_PREX"leaf add!current(%p)("MACSTR")----->child(%p)("MACSTR")\n",
					current_leaf, PRINT_MAC(current_leaf->al_mac),
					child_leaf, PRINT_MAC(child_leaf->al_mac));
				dl_list_add(&current_leaf->children, &child_leaf->entry);
			}

			/*3.*/
			build_tree_from_current_leaf(ctx, child_rpdb, child_leaf, current_leaf);
		}
	}

	return 0;
}

int delete_tree_from_current_leaf(struct p1905_managerd_ctx *ctx, struct leaf *current_leaf)
{
	struct leaf *child_leaf = NULL, *child_leaf_next = NULL;

	if (!current_leaf)
		return 0;

	dl_list_for_each_safe(child_leaf, child_leaf_next, &current_leaf->children, struct leaf, entry) {
		debug(DEBUG_ERROR, TOPO_PREX"leaf delete from current(%p)("MACSTR")\n",
				child_leaf, PRINT_MAC(child_leaf->al_mac));
		delete_tree_from_current_leaf(ctx, child_leaf);

		free_leaf(ctx, child_leaf);
	}

	return 0;
}

int detect_deleted_leaf_from_current_leaf(struct p1905_managerd_ctx *ctx, struct leaf *current_leaf)
{
	struct leaf *child_leaf = NULL, *child_leaf_next = NULL;
	struct topology_response_db *current_rsp = NULL, *child_rpdb = NULL;

	current_rsp = lookup_tprdb_by_almac(ctx, current_leaf->al_mac);

	if (!current_rsp) {
		debug(DEBUG_ERROR, TOPO_PREX"BUG here!!!current leaf ("MACSTR") not exist in my tpr db\n",
			PRINT_MAC(current_leaf->al_mac));
		return -1;
	}

	dl_list_for_each_safe(child_leaf, child_leaf_next, &current_leaf->children, struct leaf, entry) {
		child_rpdb = lookup_tprdb_by_almac(ctx, child_leaf->al_mac);
		/*maybe this condition should be add more, like check neighbor of child rsp db to see if it is current device*/
		if (child_rpdb && is_neighbor(current_rsp, child_leaf->al_mac)) {
			detect_deleted_leaf_from_current_leaf(ctx, child_leaf);
		} else {
			/*if child leaf is not my neighbor, should delete this child leaf and all its children leaves of this child leaf*/
			debug(DEBUG_ERROR, TOPO_PREX"leaf delete from current(%p)("MACSTR")\n",
				child_leaf, PRINT_MAC(child_leaf->al_mac));
			delete_tree_from_current_leaf(ctx, child_leaf);
			free_leaf(ctx, child_leaf);
		}
	}

	return 0;
}

/*init the root leaf of the tree, the root leaf always should represent current device*/
int create_topology_tree(struct p1905_managerd_ctx* ctx)
{
	struct leaf *root_leaf = NULL;

	if (ctx->root_leaf) {
		debug(DEBUG_ERROR, "root leaf already exist\n");
		goto fail;
	}

	root_leaf = create_leaf(ctx->p1905_al_mac_addr);
	if (!root_leaf)
		goto fail;

	ctx->root_leaf = root_leaf;
	return 0;
fail:
	return -1;
}

int update_topology_tree(struct p1905_managerd_ctx* ctx)
{
	struct p1905_neighbor_info *neighor_info = NULL;
	struct topology_response_db *child_rpdb = NULL;
	struct leaf *child_leaf = NULL, *child_leaf_next = NULL;
	int i = 0;
	unsigned char found = 0;

	if (!ctx->root_leaf) {
		debug(DEBUG_ERROR, TOPO_PREX"root leaf not exist, could not update topology tree\n");
		goto fail;
	}

	for (i = 0; i < ctx->itf_number; i++) {
		/*check the leaves that should be added to topology tree*/
		LIST_FOREACH(neighor_info, &ctx->p1905_neighbor_dev[i].p1905nbr_head, p1905nbr_entry) {
			child_rpdb = NULL;
			child_leaf = NULL;
			/*1. if neighbor exist in topology response db
			   2. if neighbor's 1905 neighbor tlv match the current devices's al mac and no leaf exist in current node
			       then create the child leaf
			   2.5 check whether this leaf exist, if yes, do not add this one until the exist one has deleted
			   3. and from this new/existed child leaf, rebuld its child leaves*/
			/*1.*/
			child_rpdb = lookup_tprdb_by_almac(ctx, neighor_info->al_mac_addr);
			if (!child_rpdb) {
				debug(DEBUG_TRACE, TOPO_PREX"neighbor dev("MACSTR") not exist in my topology rsp db\n",
					PRINT_MAC(neighor_info->al_mac_addr));
				continue;
			}

			/*2.*/
			if (!is_neighbor(child_rpdb, ctx->root_leaf->al_mac)) {
				debug(DEBUG_TRACE, TOPO_PREX"neighbor dev("MACSTR") has no neighbor of me\n",
					PRINT_MAC(child_rpdb->al_mac_addr));
				continue;
			}

			child_leaf = lookup_childleaf_by_almac(ctx->root_leaf, neighor_info->al_mac_addr);
			if (!child_leaf) {
				/*2.5*/
				if (is_leaf_exist(ctx, ctx->root_leaf, neighor_info->al_mac_addr)) {
					debug(DEBUG_TRACE, TOPO_PREX"current leaf("MACSTR") exist, do not create!\n",
						PRINT_MAC(neighor_info->al_mac_addr));
					continue;
				}

				child_leaf = create_leaf(neighor_info->al_mac_addr);
				if (!child_leaf)
					continue;
				debug(DEBUG_ERROR, TOPO_PREX"leaf add! root(%p)("MACSTR")----->child(%p)("MACSTR")\n",
					ctx->root_leaf, PRINT_MAC(ctx->root_leaf->al_mac),
					child_leaf, PRINT_MAC(child_leaf->al_mac));
				dl_list_add(&ctx->root_leaf->children, &child_leaf->entry);
			}
			/*3.*/
			build_tree_from_current_leaf(ctx, child_rpdb, child_leaf, ctx->root_leaf);
		}
	}

	/*check the leaves that should be deleted from topology tree*/
	dl_list_for_each_safe(child_leaf, child_leaf_next, &ctx->root_leaf->children, struct leaf, entry) {
		found = 0;
		for (i = 0; i < ctx->itf_number; i++) {
			LIST_FOREACH(neighor_info, &ctx->p1905_neighbor_dev[i].p1905nbr_head, p1905nbr_entry) {
				if (!os_memcmp(child_leaf->al_mac, neighor_info->al_mac_addr, ETH_ALEN)) {
					found = 1;
					break;
				}
			}
			if (found)
				break;
		}

		child_rpdb = lookup_tprdb_by_almac(ctx, child_leaf->al_mac);

		if (!found || !child_rpdb) {
			debug(DEBUG_ERROR, TOPO_PREX"leaf delete from current(%p)("MACSTR")\n",
				child_leaf, PRINT_MAC(child_leaf->al_mac));
			delete_tree_from_current_leaf(ctx, child_leaf);
			free_leaf(ctx, child_leaf);
		} else {
			detect_deleted_leaf_from_current_leaf(ctx, child_leaf);
		}
	}

	return 0;
fail:
	return -1;
}

int clear_topology_tree(struct p1905_managerd_ctx* ctx)
{
	delete_tree_from_current_leaf(ctx, ctx->root_leaf);
	os_free(ctx->root_leaf);
	ctx->root_leaf = NULL;

	return 0;
}

int trace_topology_tree_cb(struct p1905_managerd_ctx* ctx, struct leaf *current_leaf, topology_tree_cb_func cb, void *data)
{
	struct leaf *child_leaf = NULL, *child_leaf_next = NULL;

	if (!current_leaf)
		return 0;

	dl_list_for_each_safe(child_leaf, child_leaf_next, &current_leaf->children, struct leaf, entry) {
		cb((void*)ctx, (void*)current_leaf, (void*)child_leaf, data);
		trace_topology_tree_cb(ctx, child_leaf, cb, data);
	}

	return 0;
}

int topology_tree_travel_preorder(struct p1905_managerd_ctx* ctx, struct leaf *current_leaf)
{
	struct leaf *child_leaf = NULL, *child_leaf_next = NULL;
	leaf_info leaf_in;
	struct agent_list_db *agent_info = NULL;

	if (!current_leaf)
		return 0;

	os_memset(&leaf_in, 0, sizeof(leaf_in));

	/*push to stack*/
	debug(DEBUG_OFF, "push to stack:("MACSTR")\n",
		PRINT_MAC(current_leaf->al_mac));
	os_memcpy(leaf_in.al_mac, current_leaf->al_mac, ETH_ALEN);
#ifdef MAP_R3
		struct r3_member *peer = NULL;
		dl_list_for_each(peer, &r3_member_head, struct r3_member, entry) {
			if (!os_memcmp(peer->al_mac, current_leaf->al_mac, ETH_ALEN)) {
				if (peer->security_1905) {
					leaf_in.profile = MAP_PROFILE_R3;
					debug(DEBUG_OFF, "it's a r3 device\n");
				}
			}
		}
#endif
	if (os_memcmp(current_leaf->al_mac, ctx->p1905_al_mac_addr, ETH_ALEN)) {
		find_agent_info(ctx, current_leaf->al_mac, &agent_info);
		if (agent_info)
			clear_agent_all_radio_config_stat(agent_info);
	}
	push(&ctx->topo_stack, leaf_in);

	dl_list_for_each_safe(child_leaf, child_leaf_next, &current_leaf->children, struct leaf, entry) {
		topology_tree_travel_preorder(ctx, child_leaf);
	}

	return 0;
}

