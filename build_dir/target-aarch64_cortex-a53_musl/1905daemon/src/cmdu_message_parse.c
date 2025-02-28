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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <syslog.h>
#include <assert.h>
#include <linux/if_ether.h>
#include <asm/byteorder.h>
#include "cmdu_message_parse.h"
#include "cmdu_message.h"
#include "cmdu_fragment.h"
#include "cmdu_retry_message.h"
#include "multi_ap.h"
#include "cmdu.h"
#include "_1905_lib_io.h"
#include "debug.h"
#include "common.h"
#include "topology.h"
#include "eloop.h"
#include "ethernet_layer.h"
#include "file_io.h"
#ifdef MAP_R3
#include "map_dpp.h"
#include "wpa_extern.h"
#include "security_engine.h"
#endif
#include "service_prioritization.h"

extern int find_lldp_by_port_id_mac(unsigned char *receiving_mac, unsigned char *port_id_mac);
extern void reset_radio_config_state(struct p1905_managerd_ctx *ctx);
extern void trigger_auto_config_flow(struct p1905_managerd_ctx *ctx);

extern const unsigned char p1905_multicast_address[6];

#ifdef MAP_R3

#define FRAME_INFO_FILE "/tmp/map_frameinfo.txt"
#define RETURN_CONTENT_TLV 1
#define RETURN_CONTENT_HEXDUMP 2
#define RETURN_CONTENT_CACMethod 3
#define FILTER_FLAG_FIRST 1
#define FILTER_FLAG_LAST 2
#define FILTER_FLAG_ALL  4

void cmd_dev_start_buffer(struct p1905_managerd_ctx *ctx, char *buf)
{
	char *pos1, *pos2;
	struct frame_buff_struct *msg_info_entry, *msg_info_entry_next;
	struct frame_list_struct *msg_list_entry, *msg_list_entry_next;
	/* support 10 msg type total */
	int msg_type[10];
	unsigned char i = 0;

	os_memset(msg_type, 0, sizeof(msg_type));

	ctx->frame_buff_switch = 1;
	debug(DEBUG_ERROR, "ctx->frame_buff_switch %d\n", ctx->frame_buff_switch);

	/* delete all the msg type info and packet list first */
	dl_list_for_each_safe(msg_info_entry, msg_info_entry_next,
			      &ctx->frame_buff_head, struct frame_buff_struct, frame_info_list) {

		dl_list_for_each_safe(msg_list_entry, msg_list_entry_next,
				      &msg_info_entry->frame_data_list, struct frame_list_struct, data_list) {
			os_free(msg_list_entry->data);
			dl_list_del(&msg_list_entry->data_list);
			os_free(msg_list_entry);
		}

		dl_list_del(&msg_info_entry->frame_info_list);
		os_free(msg_info_entry);
	}

	pos1 = buf;

	while (NULL != (pos2 = strstr(pos1, "0x"))) {
		if (sscanf(pos2, "0x%04x", &msg_type[i]) < 0)
			debug(DEBUG_ERROR, "[%d]Unexpected fail at sscanf\n", __LINE__);
		debug(DEBUG_ERROR, "msg type 0x%04x\n", msg_type[i]);
		pos1 = pos2 + 2;

		/* malloc buffer for msg info for each msg type */

		msg_info_entry = os_zalloc(sizeof(struct frame_buff_struct));
		if (!msg_info_entry) {
			debug(DEBUG_ERROR, "malloc msg error\n");
			break;
		}

		msg_info_entry->msg_type = msg_type[i];

		/* init packet list  */
		dl_list_init(&msg_info_entry->frame_data_list);

		/* add msg info to frame_buff_head */
		dl_list_add(&ctx->frame_buff_head, &msg_info_entry->frame_info_list);
	}

	/* wait 30 min at most to stop buffer
	 ** to avoid not receive stop buffer cmd from ucc
	 */
	eloop_register_timeout(30 * 60, 0, cmd_dev_stop_buffer_timeout, (void *)ctx, NULL);
	debug(DEBUG_ERROR, "success\n");
	return;
}

void cmd_dev_stop_buffer(struct p1905_managerd_ctx *ctx)
{
	ctx->frame_buff_switch = 0;
	debug(DEBUG_ERROR, "ctx->frame_buff_switch %d\n", ctx->frame_buff_switch);
	debug(DEBUG_ERROR, "success\n");
}

void cmd_dev_stop_buffer_timeout(void *eloop_data, void *user_ctx)
{
	struct p1905_managerd_ctx *ctx = (struct p1905_managerd_ctx *)eloop_data;

	if (ctx->frame_buff_switch) {
		debug(DEBUG_ERROR, "timeout, need stop buffering the frames\n");
		cmd_dev_stop_buffer(ctx);
	}
}

void get_tlv_data(struct p1905_managerd_ctx *ctx,
		  unsigned int msg_type,
		  unsigned int mid,
		  unsigned int tlv[], unsigned int flag, unsigned int pkt_num, unsigned int return_content)
{
	FILE *file = NULL;
	unsigned char *tmp = NULL;
	unsigned char *pdata = tmp;
	unsigned char msg_found = 0;
	unsigned char start_idx = 0, stop_idx = 0, cur_idx = 0;
	unsigned char cur_frame_idx = 0;
	unsigned short pkt_tlv_len = 0;
	unsigned short left_tlv_len = 0;
	struct frame_buff_struct *msg_info_entry, *msg_info_entry_next;
	struct frame_list_struct *msg_list_entry = NULL, *msg_list_entry_next = NULL;

	dl_list_for_each_safe(msg_info_entry, msg_info_entry_next,
			      &ctx->frame_buff_head, struct frame_buff_struct, frame_info_list) {
		if (msg_info_entry->msg_type == msg_type) {

			debug(DEBUG_ERROR, "Message Type 0x%04x found\n", msg_type);

			msg_found = 1;
			break;
		}
	}

	if (!msg_found) {
		debug(DEBUG_ERROR, "msg type 0x%04x not found\n", msg_type);
		return;
	}

	/* write MessageType to tmp file */
	file = fopen(FRAME_INFO_FILE, "w");
	if (!file) {
		debug(DEBUG_ERROR, "open file %s error\n", FRAME_INFO_FILE);
		return;
	}

	if (fprintf(file, "MessageType 0x%04x\n", msg_type) < 0) {
		debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
		goto end;
	}

	/* else, no tlv specified */
	if (flag == FILTER_FLAG_FIRST) {
		start_idx = 0;
		stop_idx = (pkt_num > msg_info_entry->msg_cnt) ? msg_info_entry->msg_cnt : pkt_num;
	} else if (flag == FILTER_FLAG_LAST) {
		start_idx = (msg_info_entry->msg_cnt > pkt_num) ? (msg_info_entry->msg_cnt - pkt_num) : 0;
		stop_idx = msg_info_entry->msg_cnt;
	} else {
		start_idx = 0;
		stop_idx = msg_info_entry->msg_cnt;
	}

	/* if mid specified, match the mid among all the frames buffered */
	if (mid > 0) {
		start_idx = 0;
		stop_idx = msg_info_entry->msg_cnt;
	}
	debug(DEBUG_ERROR, "start index %d, stop index %d\n", start_idx, stop_idx);

	if (tlv[0]) {
		/* TLVFieldValue,00 */
		if (fprintf(file, "TLVFieldValue ") < 0) {
			debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
			goto end;
		}
		debug(DEBUG_ERROR, "append TLVFieldValue\n");
	} else {
		/* TLVList,1_0xBD_0xBD_0xB5_0xB6_2_0xBD_0xB5 */
		if (return_content == RETURN_CONTENT_TLV) {
			if (fprintf(file, "TLVList ") < 0) {
				debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
				goto end;
			}
			debug(DEBUG_ERROR, "append TLVList\n");
		}
		/* MessageDump,1_B9081020304050607080BA03AABBCC */
		else {
			if (fprintf(file, "MessageDump ") < 0) {
				debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
				goto end;
			}
			debug(DEBUG_ERROR, "append MessageDump\n");
		}
	}

	dl_list_for_each_safe(msg_list_entry, msg_list_entry_next,
			      &msg_info_entry->frame_data_list, struct frame_list_struct, data_list) {
		if (cur_idx >= start_idx && cur_idx < stop_idx) {
			/* mid specified, */
			if (mid > 0) {
				/* msg id mismatch, skip current frame */
				if (mid != msg_list_entry->mid)
					continue;
				/* msg id match, continue to check  */
				else
					debug(DEBUG_ERROR, "frame mid 0x%04x found\n", msg_list_entry->mid);
			}

			cur_frame_idx++;
			debug(DEBUG_ERROR, "current frame idx %d\n", cur_frame_idx);
			tmp = msg_list_entry->data;
			left_tlv_len = msg_list_entry->len;

			/* not first pkt, append "_" before frame index */
			if (cur_frame_idx > 1)
				if (fprintf(file, "_") < 0) {
					debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
					goto end;
				}

			/* append frame index */
			if (fprintf(file, "%d", cur_frame_idx) < 0) {
				debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
				goto end;
			}

			/* there is "_" between two messages for hexdump */
			if (return_content == RETURN_CONTENT_HEXDUMP)
				if (fprintf(file, "_") < 0) {
					debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
					goto end;
				}

			/* return_content: TLV */
			/* return all the tlv types of all the packets */
			while (1) {
				/* end of tlv, skip */
				if (*tmp == 0)
					break;
				pdata = tmp;
				pkt_tlv_len = *((unsigned short *)(tmp + 1));
				pkt_tlv_len = be_to_host16(pkt_tlv_len);

				if (pkt_tlv_len > left_tlv_len) {
					debug(DEBUG_ERROR, "pkt_tlv_len %d bigger than left_tlv_len %d\n", pkt_tlv_len, left_tlv_len);
					goto end;
				}

				/* there is "_" between two tlv types */
				if (return_content == RETURN_CONTENT_TLV) {
					if (fprintf(file, "_0x%02X", *tmp) < 0) {
						debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
						goto end;
					}
					debug(DEBUG_ERROR, "*****************************\n");
					debug(DEBUG_ERROR, "tlv 0x%02X\n", *tmp);
					debug(DEBUG_ERROR, "*****************************\n");
				}
				/*  but no "_" between datas */
				else if (return_content == RETURN_CONTENT_HEXDUMP) {

					/* if tlv type specified, return data include [t l v] */
					if (tlv[0]) {
						if (tlv[0] == *tmp) {
							debug(DEBUG_ERROR, "frame tlv 0x%02x found\n", *tmp);
							while (pdata - tmp < (pkt_tlv_len + 3)) {
								if (fprintf(file, "%02X", *pdata) < 0) {
									debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
									goto end;
								}
								pdata++;
							}
						}

					} else {
						while (pdata - tmp < (pkt_tlv_len + 3)) {
							if (fprintf(file, "%02X", *pdata) < 0) {
								debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
								goto end;
							}
							pdata++;
						}
					}
				}

				/* next tlv */
				tmp += (3 + pkt_tlv_len);
				left_tlv_len -= (3 + pkt_tlv_len);
			}
		}
		cur_idx++;
	}
	if (fprintf(file, "\n") < 0) {
		debug(DEBUG_ERROR, "[%d]fprintf fail!\n", __LINE__);
		goto end;
	}
end:
	if (fclose(file) < 0)
		debug(DEBUG_ERROR, "[%d]fclose fail!\n", __LINE__);

	return;
}

void cmd_dev_get_frame_info(struct p1905_managerd_ctx *ctx, char *buf)
{
	unsigned int msg_type = 0;
	unsigned int tlv[8];
	unsigned int mid = 0;
	char *pos1 = NULL, *pos2 = NULL;
	unsigned int flag = 0, pkt_num = 0, return_content = 0;

	os_memset(tlv, 0, sizeof(tlv));

	debug(DEBUG_ERROR, "\n");
	/* message type */
	pos1 = buf;
	pos2 = os_strstr(pos1, " ");
	if (!pos2)
		return;

	if (sscanf(pos1, "0x%04x", &msg_type) < 0)
		debug(DEBUG_ERROR, "[%d]Unexpected fail at sscanf\n", __LINE__);
	debug(DEBUG_ERROR, "set message type 0x%04x\n", msg_type);

	/* tlv */
	pos1 = pos2 + 1;
	pos2 = os_strstr(pos1, " ");
	if (!pos2)
		return;
	if (sscanf(pos1, "0x%02x", &tlv[0]) < 0)
		debug(DEBUG_ERROR, "[%d]Unexpected fail at sscanf\n", __LINE__);
	debug(DEBUG_ERROR, "set tlv 0x%02x\n", tlv[0]);

	/* MID */
	pos1 = pos2 + 1;
	pos2 = os_strstr(pos1, " ");
	if (!pos2)
		return;
	if (sscanf(pos1, "0x%04x", &mid) < 0)
		debug(DEBUG_ERROR, "[%d]Unexpected fail at sscanf\n", __LINE__);
	debug(DEBUG_ERROR, "set mid 0x%04x\n", mid);

	/* Filter */
	pos1 = pos2 + 1;
	pos2 = os_strstr(pos1, " ");
	if (!pos2)
		return;
	if (os_strstr(pos1, "first") != NULL) {
		pkt_num = 1;
		flag = FILTER_FLAG_FIRST;
		debug(DEBUG_ERROR, "set First flag(First)\n");
	} else if (os_strstr(pos1, "last") != NULL) {
		flag = FILTER_FLAG_LAST;
		pkt_num = 1;
		debug(DEBUG_ERROR, "set Last flag(Last)\n");
	} else if (os_strstr(pos1, "all") != NULL) {
		pkt_num = 0xff;
		flag = FILTER_FLAG_ALL;
		debug(DEBUG_ERROR, "set All flag(All)\n");
	} else {
		if (sscanf(pos1, "%x", &pkt_num) < 0)
			debug(DEBUG_ERROR, "[%d]Unexpected fail at sscanf\n", __LINE__);
		debug(DEBUG_ERROR, "set pkt num %d\n", pkt_num);

		if (os_strstr(pos1, "_f") != NULL) {
			flag = FILTER_FLAG_FIRST;
			debug(DEBUG_ERROR, "set First flag(_F)\n");
		} else if (os_strstr(pos1, "_l") != NULL) {
			flag = FILTER_FLAG_LAST;
			debug(DEBUG_ERROR, "set Last flag(_L)\n");
		} else {
			debug(DEBUG_ERROR, "no last or first flag be set\n");
			return;
		}
	}

	/* ReturnContent */
	pos1 = pos2 + 1;
	if (os_strstr(pos1, "tlv") != NULL) {
		return_content = RETURN_CONTENT_TLV;
		debug(DEBUG_ERROR, "return content: TLV\n");
	} else if (os_strstr(pos1, "messagehexdump") != NULL) {
		return_content = RETURN_CONTENT_HEXDUMP;
		debug(DEBUG_ERROR, "return content: HEXDUMP\n");
	}

	get_tlv_data(ctx, msg_type, mid, tlv, flag, pkt_num, return_content);
	return;
}

void dev_buffer_frame(struct p1905_managerd_ctx *ctx, unsigned short mtype,
		      unsigned short mid, unsigned char *tlv_pos, int tlv_len)
{
	struct frame_buff_struct *msg_info_entry, *msg_info_entry_next;
	struct frame_list_struct *msg_list_entry = NULL;
	unsigned char *msg_data = NULL;

	if (ctx->frame_buff_switch) {
		dl_list_for_each_safe(msg_info_entry, msg_info_entry_next,
				      &ctx->frame_buff_head, struct frame_buff_struct, frame_info_list) {
			if (msg_info_entry->msg_type == mtype) {
				debug(DEBUG_ERROR, "store data for msg type 0x%04x\n", mtype);
				msg_list_entry = os_zalloc(sizeof(struct frame_list_struct));
				if (!msg_list_entry)
					break;
				msg_data = os_zalloc((size_t) tlv_len);
				if (!msg_data) {
					os_free(msg_list_entry);
					break;
				}

				/* will be free after next dev_start_buff cmd or daemon exit */
				os_memcpy(msg_data, tlv_pos, tlv_len);
				msg_list_entry->data = msg_data;
				msg_list_entry->len = tlv_len;
				msg_list_entry->mid = mid;
				msg_info_entry->msg_cnt++;
				dl_list_add(&msg_info_entry->frame_data_list, &msg_list_entry->data_list);
			}
		}
	}
}

#endif // MAP_R3
void topology_discovery_action(struct p1905_managerd_ctx *ctx, unsigned char *peer_al_mac,
			       unsigned char need_query, unsigned char need_notify)
{
	unsigned int j = 0;

	if (need_query) {
		insert_cmdu_txq(peer_al_mac, ctx->p1905_al_mac_addr, e_topology_query, ++ctx->mid,
				ctx->itf[ctx->recent_cmdu_rx_if_number].if_name, 0);
		debug(DEBUG_ERROR, TOPO_PREX "tx query(mid-%04x) on intf(%s) to " MACSTR "\n",
		      ctx->mid, ctx->itf[ctx->recent_cmdu_rx_if_number].if_name, PRINT_MAC(peer_al_mac));
	}
	if (need_notify == 1) {
		ctx->mid++;
		debug(DEBUG_ERROR, TOPO_PREX "tx multicast notification(mid-%04x) on all intf\n", ctx->mid);
		for (j = 0; j < ctx->itf_number; j++) {
			/*send multicast cmdu to all the interface */
			insert_cmdu_txq((unsigned char *)p1905_multicast_address, ctx->p1905_al_mac_addr,
					e_topology_notification, ctx->mid, ctx->itf[j].if_name, 0);
#ifdef SUPPORT_CMDU_RELIABLE
			cmdu_reliable_send(ctx, e_topology_notification, ctx->mid, j);
#endif
		}
	} else if (need_notify == 2) {
		ctx->mid++;
		if (fill_send_tlv(ctx, peer_al_mac, ETH_ALEN) == -1)
			debug(DEBUG_ERROR, "[%d]fill_send_tlv fail!\n", __LINE__);

		debug(DEBUG_ERROR, TOPO_PREX "tx multicast notification(with vendor tlv)(mid-%04x) on all intf\n",
		      ctx->mid);
		for (j = 0; j < ctx->itf_number; j++) {
			/*send multicast cmdu to all the interface */
			insert_cmdu_txq((unsigned char *)p1905_multicast_address,
					ctx->p1905_al_mac_addr,
					e_topology_notification_with_vendor_ie, ctx->mid, ctx->itf[j].if_name, 0);
#ifdef SUPPORT_CMDU_RELIABLE
			cmdu_reliable_send(ctx, e_topology_notification_with_vendor_ie, ctx->mid, j);
#endif
		}
	}
}

/**
 *  add/update 1905.1 neighbor device information.
 *
 * \param  ctx  1905.1 managerd context
 * \param  al_mac  neighbor al mac addr in topology discovery message.
 * \param  neighbor_itf_mac  neighbor device interface mac in topology discover.
 * \param  itf_mac_addr  the interface topology discovery received from.
 * \param  notify  if need to send topology notify, set this flag
 * \param  retun error code
 */
int update_p1905_neighbor_dev_info(struct p1905_managerd_ctx *ctx,
				   unsigned char *al_mac, unsigned char *neighbor_itf_mac,
				   unsigned char *itf_mac_addr, unsigned char *notify)
{
	int i = 0;
	struct p1905_neighbor_info *dev_info = NULL;

	for (i = 0; i < ctx->itf_number; i++) {
		if (!memcmp(ctx->p1905_neighbor_dev[i].local_mac_addr, itf_mac_addr, ETH_ALEN))
			break;
	}

	if (i >= ctx->itf_number) {
		debug(DEBUG_ERROR, TOPO_PREX "no this local interface(%s)\n", itf_mac_addr);
		return -1;
	}

	if (!LIST_EMPTY(&ctx->p1905_neighbor_dev[i].p1905nbr_head)) {
		LIST_FOREACH(dev_info, &ctx->p1905_neighbor_dev[i].p1905nbr_head, p1905nbr_entry) {
			if (!memcmp(dev_info->al_mac_addr, al_mac, ETH_ALEN)) {
				if (dev_info->ieee802_1_bridge == 0) {
					/* can not find any matched mac address in LLDP storage
					 * it means that local device just receive the topology
					 * discovery but no 802.1 bridge detection message.
					 * so a 802.1 bridge exist
					 */
					if (find_lldp_by_port_id_mac(itf_mac_addr, neighbor_itf_mac) == 0) {
						dev_info->ieee802_1_bridge = 1;
						*notify = 1;
					}
				}
				return 0;
			}
		}
	}

	dev_info = (struct p1905_neighbor_info *)malloc(sizeof(struct p1905_neighbor_info));
	if (!dev_info) {
		debug(DEBUG_ERROR, TOPO_PREX "alloc dev_info fail\n");
		return -1;
	}
	memcpy(dev_info->al_mac_addr, al_mac, ETH_ALEN);
	dev_info->ieee802_1_bridge = 1;
	LIST_INSERT_HEAD(&ctx->p1905_neighbor_dev[i].p1905nbr_head, dev_info, p1905nbr_entry);
	debug(DEBUG_OFF, TOPO_PREX "local intf(%s) add 1905 neighbor(" MACSTR ")\n",
	      ctx->itf[i].if_name, PRINT_MAC(al_mac));
	/*need to send topology notification message */
	*notify = 1;

	return 0;
}

/**
 *  delete 1905.1 topology response database.
 *
 * \param  ctx  1905.1 managerd context
 * \param  al_mac  input the target device al mac to delete.
 */
void delete_exist_topology_response_database(struct p1905_managerd_ctx *ctx, unsigned char *al_mac)
{
	struct topology_response_db *tpgr_db;
	struct device_info_db *dev_info, *dev_info_temp;
	struct p1905_neighbor_device_db *p1905_neighbor, *p1905_neighbor_temp;
	struct non_p1905_neighbor_device_list_db *non_p1905_neighbor, *non_p1905_neighbor_temp;
	struct device_bridge_capability_db *br_cap, *br_cap_temp;

	if (!SLIST_EMPTY(&(ctx->topology_entry.tprdb_head))) {
		SLIST_FOREACH(tpgr_db, &(ctx->topology_entry.tprdb_head), tprdb_entry) {
			if (!memcmp(tpgr_db->al_mac_addr, al_mac, ETH_ALEN)) {
				if (!SLIST_EMPTY(&(tpgr_db->devinfo_head))) {
					dev_info = SLIST_FIRST(&(tpgr_db->devinfo_head));
					while (dev_info) {
						dev_info_temp = SLIST_NEXT(dev_info, devinfo_entry);
						if (!SLIST_EMPTY(&(dev_info->p1905_nbrdb_head))) {
							/*delete all p1905.1 device in this interface */
							p1905_neighbor = SLIST_FIRST(&(dev_info->p1905_nbrdb_head));
							while (p1905_neighbor) {
								p1905_neighbor_temp =
								    SLIST_NEXT(p1905_neighbor, p1905_nbrdb_entry);

								SLIST_REMOVE(&(dev_info->p1905_nbrdb_head),
									     p1905_neighbor, p1905_neighbor_device_db,
									     p1905_nbrdb_entry);
								free(p1905_neighbor);
								p1905_neighbor = p1905_neighbor_temp;
							}
						}

						/*delete all non-p1905.1 device in this interface */
						if (!SLIST_EMPTY(&(dev_info->non_p1905_nbrdb_head))) {
							non_p1905_neighbor =
							    SLIST_FIRST(&(dev_info->non_p1905_nbrdb_head));
							while (non_p1905_neighbor) {
								non_p1905_neighbor_temp =
								    SLIST_NEXT(non_p1905_neighbor,
									       non_p1905_nbrdb_entry);

								SLIST_REMOVE(&(dev_info->non_p1905_nbrdb_head),
									     non_p1905_neighbor,
									     non_p1905_neighbor_device_list_db,
									     non_p1905_nbrdb_entry);
								free(non_p1905_neighbor);
								non_p1905_neighbor = non_p1905_neighbor_temp;
							}
						}
						/*remove this device info db from list */
						SLIST_REMOVE(&(tpgr_db->devinfo_head), dev_info,
							     device_info_db, devinfo_entry);
						/*delete this device info db */
						if (dev_info->vs_info_len)
							free(dev_info->vs_info);

						free(dev_info);
						dev_info = dev_info_temp;
					}
				}

				/*delete bridge capability info */
				if (!LIST_EMPTY(&(tpgr_db->brcap_head))) {
					br_cap = LIST_FIRST(&(tpgr_db->brcap_head));
					while (br_cap) {
						br_cap_temp = LIST_NEXT(br_cap, brcap_entry);
						LIST_REMOVE(br_cap, brcap_entry);

						if (br_cap->interface_amount)
							free(br_cap->interface_mac_tuple);

						free(br_cap);
						br_cap = br_cap_temp;
					}
				}
				/*finall, delete whole topology response with this AL MAC */
				SLIST_REMOVE(&(ctx->topology_entry.tprdb_head), tpgr_db,
					     topology_response_db, tprdb_entry);
				free(tpgr_db);
				return;
			}
		}
	}
}

void update_bridge_cap(struct list_head_brcap *dst_br_head, struct list_head_brcap *src_br_head)
{
	struct device_bridge_capability_db *br = NULL;
	struct device_bridge_capability_db *br_temp = NULL;

	if (!dst_br_head || !src_br_head)
		return;
	if (LIST_EMPTY(src_br_head))
		return;

	br = LIST_FIRST(src_br_head);
	while (br) {
		br_temp = LIST_NEXT(br, brcap_entry);
		LIST_REMOVE(br, brcap_entry);
		LIST_INSERT_HEAD(dst_br_head, br, brcap_entry);
		br = br_temp;
	}
}

void update_dev_info(struct list_head_devinfo *dst_devinfo_head, struct list_head_devinfo *src_devinfo_head)
{
	struct device_info_db *dev = NULL;
	struct device_info_db *dev_tmp = NULL;

	if (!dst_devinfo_head || !src_devinfo_head)
		return;
	if (SLIST_EMPTY(src_devinfo_head))
		return;

	dev = SLIST_FIRST(src_devinfo_head);
	while (dev) {
		dev_tmp = SLIST_NEXT(dev, devinfo_entry);
		SLIST_REMOVE(src_devinfo_head, dev, device_info_db, devinfo_entry);
		SLIST_INSERT_HEAD(dst_devinfo_head, dev, devinfo_entry);
		debug(DEBUG_TRACE, "insert struct dev info db\n");
		debug(DEBUG_TRACE, "dev mac("MACSTR") media_type=%d\n", PRINT_MAC(dev->mac_addr), dev->media_type);
		dev = dev_tmp;
	}
}

struct topology_response_db *delete_exist_topology_response_content(struct p1905_managerd_ctx *ctx,
								    unsigned char *al_mac)
{
	struct topology_response_db *tpgr_db = NULL;
	struct device_info_db *dev_info, *dev_info_temp;
	struct p1905_neighbor_device_db *p1905_neighbor, *p1905_neighbor_temp;
	struct non_p1905_neighbor_device_list_db *non_p1905_neighbor, *non_p1905_neighbor_temp;
	struct device_bridge_capability_db *br_cap, *br_cap_temp;

	SLIST_FOREACH(tpgr_db, &(ctx->topology_entry.tprdb_head), tprdb_entry) {
		if (os_memcmp(tpgr_db->al_mac_addr, al_mac, ETH_ALEN))
			continue;

		dev_info = SLIST_FIRST(&tpgr_db->devinfo_head);
		while (dev_info) {
			dev_info_temp = SLIST_NEXT(dev_info, devinfo_entry);
			/*delete all p1905.1 device in this interface */
			p1905_neighbor = SLIST_FIRST(&dev_info->p1905_nbrdb_head);
			while (p1905_neighbor) {
				p1905_neighbor_temp = SLIST_NEXT(p1905_neighbor, p1905_nbrdb_entry);
				SLIST_REMOVE(&dev_info->p1905_nbrdb_head,
					     p1905_neighbor, p1905_neighbor_device_db, p1905_nbrdb_entry);
				free(p1905_neighbor);
				p1905_neighbor = p1905_neighbor_temp;
			}
			/*delete all non-p1905.1 device in this interface */
			non_p1905_neighbor = SLIST_FIRST(&dev_info->non_p1905_nbrdb_head);
			while (non_p1905_neighbor) {
				non_p1905_neighbor_temp = SLIST_NEXT(non_p1905_neighbor, non_p1905_nbrdb_entry);
				SLIST_REMOVE(&dev_info->non_p1905_nbrdb_head,
					     non_p1905_neighbor, non_p1905_neighbor_device_list_db,
					     non_p1905_nbrdb_entry);
				free(non_p1905_neighbor);
				non_p1905_neighbor = non_p1905_neighbor_temp;
			}
			/*remove this device info db from list */
			SLIST_REMOVE(&tpgr_db->devinfo_head, dev_info, device_info_db, devinfo_entry);
			/*delete this device info db */
			if (dev_info->vs_info_len)
				free(dev_info->vs_info);

			free(dev_info);
			dev_info = dev_info_temp;
		}

		/*delete bridge capability info */
		br_cap = LIST_FIRST(&tpgr_db->brcap_head);
		while (br_cap) {
			br_cap_temp = LIST_NEXT(br_cap, brcap_entry);
			LIST_REMOVE(br_cap, brcap_entry);

			if (br_cap->interface_amount)
				free(br_cap->interface_mac_tuple);

			free(br_cap);
			br_cap = br_cap_temp;
		}
		return tpgr_db;
	}

	return tpgr_db;
}

void delete_exist_topology_discovery_database(struct p1905_managerd_ctx *ctx,
					      unsigned char *al_mac, unsigned char *itf_mac)
{
	struct topology_discovery_db *tpg_db = NULL;

	if (!LIST_EMPTY(&ctx->topology_entry.tpddb_head)) {
		LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry) {
			if (!memcmp(tpg_db->al_mac, al_mac, 6) && !memcmp(tpg_db->itf_mac, itf_mac, 6)) {
				delete_p1905_neighbor_dev_info(ctx, tpg_db->al_mac,
							       (unsigned char *)tpg_db->receive_itf_mac);
				/*delete the topology discovery database */
				LIST_REMOVE(tpg_db, tpddb_entry);
				free(tpg_db);
				return;
			}
		}
	}
}

/*
Fn to delete the topology rsp db by port by local interface mac address
	port == -1 delete by local interface mac address
	local_mac == NULL delete by port number index
*/
void delete_topo_response_db_by_port_interface(struct p1905_managerd_ctx *ctx, int port)
{
	struct topology_response_db *tpgr_db = NULL, *tpgr_db_tmp = NULL;
	unsigned char del_cnt = 0;

	if (port == -1) {
		debug(DEBUG_ERROR, TOPO_PREX "error port(-1)!!!\n");
		return;
	}

	tpgr_db = SLIST_FIRST(&ctx->topology_entry.tprdb_head);
	while (tpgr_db) {
		tpgr_db_tmp = SLIST_NEXT(tpgr_db, tprdb_entry);
		if (tpgr_db->eth_port == port) {
			delete_exist_topology_response_database(ctx, tpgr_db->al_mac_addr);
			del_cnt++;
		}
		tpgr_db = tpgr_db_tmp;
	}

	if (del_cnt)
		mark_valid_topo_rsp_node(ctx);
}

/*
Fn to delete the discovery db by port number or local interface mac address
	port == -1 delete by local interface mac address
	local_mac == NULL delete by port number index
*/
int delete_topo_disc_db_by_port(struct p1905_managerd_ctx *ctx, int port, unsigned char *exclude_almac)
{
	struct topology_discovery_db *db = NULL, *db_temp = NULL;
	int cnt = 0, del_rsp_cnt = 0;

	if (port == -1)
		return cnt;

	if (!LIST_EMPTY(&ctx->topology_entry.tpddb_head)) {
		db = LIST_FIRST(&ctx->topology_entry.tpddb_head);
		while (db) {
			db_temp = LIST_NEXT(db, tpddb_entry);
			if (db->eth_port == port) {
				if (exclude_almac && !os_memcmp(exclude_almac, db->al_mac, ETH_ALEN)) {
					db = db_temp;
					continue;
				}
				delete_p1905_neighbor_dev_info(ctx, db->al_mac, db->receive_itf_mac);
				LIST_REMOVE(db, tpddb_entry);
				cnt++;
				debug(DEBUG_ERROR,
				      TOPO_PREX "del discovery(" MACSTR ") peer intf(" MACSTR ") recv intf(" MACSTR
				      ") port(%d)\n", PRINT_MAC(db->al_mac), PRINT_MAC(db->itf_mac),
				      PRINT_MAC(db->receive_itf_mac), db->eth_port);
				if (find_discovery_by_almac(ctx, db->al_mac) == NULL) {
					debug(DEBUG_ERROR, TOPO_PREX "no discovery in db! del topology rsp\n");
					delete_exist_topology_response_database(ctx, db->al_mac);
					del_rsp_cnt++;
				}
				os_free(db);
			}
			db = db_temp;
		}
	}

	if (del_rsp_cnt)
		mark_valid_topo_rsp_node(ctx);

	return cnt;
}

/**
 *  delete one 1905.1 neighbor device.
 *
 * \param  ctx  1905.1 managerd context
 * \param  al_mac  input the neighbor device al mac to delete.
 * \param  retun error code
 */
int delete_p1905_neighbor_dev_info(struct p1905_managerd_ctx *ctx, unsigned char *al_mac, unsigned char *recvif_mac)
{
	int i = 0;
	struct p1905_neighbor_info *dev_info;
#ifdef MAP_R3
	struct r3_member *peer = NULL;
	unsigned int wait_time = 30;
#endif

	for (i = 0; i < ctx->itf_number; i++) {
		if (!memcmp(recvif_mac, ctx->itf[i].mac_addr, ETH_ALEN)) {
			LIST_FOREACH(dev_info, &ctx->p1905_neighbor_dev[i].p1905nbr_head, p1905nbr_entry) {
				if (!memcmp(dev_info->al_mac_addr, al_mac, ETH_ALEN)) {
					debug(DEBUG_ERROR, TOPO_PREX "del neighbor(" MACSTR ") on %s\n",
					      PRINT_MAC(al_mac), ctx->itf[i].if_name);
#ifdef MAP_R3
					if (ctx->MAP_Cer) {
						wait_time = 300;
						ptk_peer_reset_encrypt_txrx_counter(al_mac);
					}
					peer = get_r3_member(al_mac);
					if (peer) {
						debug(DEBUG_ERROR, "del R3 sec info after %d sec\n", wait_time);
						eloop_cancel_timeout(timeout_delete_r3_member,
							(void *)ctx, (void *)peer);
						eloop_register_timeout(wait_time, 0,
							timeout_delete_r3_member, (void *)ctx, (void *)peer);
					}
#endif
					LIST_REMOVE(dev_info, p1905nbr_entry);
					free(dev_info);
					return 1;
				}
			}
		}
	}

	return 0;
}

struct topology_discovery_db *get_tpd_db_by_mac(struct p1905_managerd_ctx *ctx, unsigned char *mac)
{
	struct topology_discovery_db *tpg_db = NULL;

	LIST_FOREACH(tpg_db, &(ctx->topology_entry.tpddb_head), tpddb_entry) {
		if (!os_memcmp(tpg_db->itf_mac, mac, ETH_ALEN)) {
			break;
		}
	}

	return tpg_db;
}

struct topology_response_db *get_trsp_db_by_mac(struct p1905_managerd_ctx *ctx, unsigned char *mac)
{
	struct topology_response_db *trsp_db = NULL;

	SLIST_FOREACH(trsp_db, &(ctx->topology_entry.tprdb_head), tprdb_entry) {
		if (!os_memcmp(trsp_db->al_mac_addr, mac, ETH_ALEN))
			break;
	}

	return trsp_db;
}

/**
 *  update the time to live for topology dicovery message.
 *  if value of time to live is less than 0, need to delete this record
 *
 *
 * \param  ctx  1905.1 managerd context
 * \param  sec  for 1905.1 managerd use.
 * \return error code
 */
int delete_not_exist_p1905_neighbor_device(struct p1905_managerd_ctx *ctx, int sec)
{
	struct topology_discovery_db *tpg_db, *tpg_db_temp;
	int result = 0;
	char del_rsp_cnt = 0;

	if (!LIST_EMPTY(&ctx->topology_entry.tpddb_head)) {
		tpg_db = LIST_FIRST(&ctx->topology_entry.tpddb_head);
		while (tpg_db) {
			tpg_db_temp = LIST_NEXT(tpg_db, tpddb_entry);
			tpg_db->time_to_live -= sec;
			if (tpg_db->time_to_live <= 0) {
				/*delete the local p1905.1 neighbor device info */
				debug(DEBUG_ERROR,
				      TOPO_PREX "timeout! del neighbor(" MACSTR ") peer tx intf(" MACSTR ") recv intf("
				      MACSTR ")\n", PRINT_MAC(tpg_db->al_mac), PRINT_MAC(tpg_db->itf_mac),
				      PRINT_MAC(tpg_db->receive_itf_mac));
				if (delete_p1905_neighbor_dev_info
				    (ctx, tpg_db->al_mac, (unsigned char *)tpg_db->receive_itf_mac))
					result = 1;
				/*delete the topology discovery database */
				LIST_REMOVE(tpg_db, tpddb_entry);
				if (find_discovery_by_almac(ctx, tpg_db->al_mac) == NULL) {
					debug(DEBUG_ERROR, TOPO_PREX "no discovery in db; del rsp\n");
					delete_exist_topology_response_database(ctx, tpg_db->al_mac);
					del_rsp_cnt++;
				}
				unmask_control_conn_port(ctx, tpg_db->eth_port);
				free(tpg_db);
			}
			tpg_db = tpg_db_temp;
		}
	}

	if (del_rsp_cnt)
		mark_valid_topo_rsp_node(ctx);

	return result;
}

void delete_not_exist_p1905_topology_device(struct p1905_managerd_ctx *ctx, int sec)
{
	struct topology_device_db *tpdev_db, *tpdev_db_temp;

	if (!SLIST_EMPTY(&ctx->topodev_entry.tpdevdb_head)) {
		tpdev_db = SLIST_FIRST(&ctx->topodev_entry.tpdevdb_head);
		while (tpdev_db) {
			tpdev_db_temp = SLIST_NEXT(tpdev_db, tpdev_entry);
			tpdev_db->time_to_live -= sec;
			if (tpdev_db->time_to_live <= 0) {
				SLIST_REMOVE(&ctx->topodev_entry.tpdevdb_head,
					     tpdev_db, topology_device_db, tpdev_entry);
				free(tpdev_db);
			}
			tpdev_db = tpdev_db_temp;
		}
	}
}

/**
 *  find remote device's al mac address
 *
 *
 * \param  ctx  1905.1 managerd context
 * \param  mac_addr  input, use the sa mac value we got from other unicast message .
 * \param  al_mac_addr  output, provide the al mac
 * \return error code
 */
int find_al_mac_address(struct p1905_managerd_ctx *ctx, unsigned char *mac_addr, unsigned char *al_mac_addr)
{
	struct topology_discovery_db *tpg_db;
	struct topology_response_db *tpgr_db;
	struct device_info_db *dev_db;
	int result = -1;

	/*search in topology discovery database */
	if (!LIST_EMPTY(&(ctx->topology_entry.tpddb_head))) {
		LIST_FOREACH(tpg_db, &(ctx->topology_entry.tpddb_head), tpddb_entry) {
			if (!memcmp(mac_addr, tpg_db->al_mac, ETH_ALEN)) {
				memcpy(al_mac_addr, mac_addr, ETH_ALEN);
				result = 0;
				goto find_finish;
			}
			if (!memcmp(mac_addr, tpg_db->itf_mac, ETH_ALEN)) {
				memcpy(al_mac_addr, tpg_db->al_mac, ETH_ALEN);
				result = 0;
				goto find_finish;
			}
		}
	}
	/* search in topology response database
	 * if no matched al mac in topology discovery databse, it means this
	 * device is not a neighbor. So we check the topology response database
	 * to find any matched device
	 */
	if (!SLIST_EMPTY(&(ctx->topology_entry.tprdb_head))) {
		SLIST_FOREACH(tpgr_db, &(ctx->topology_entry.tprdb_head), tprdb_entry) {
			if (!memcmp(mac_addr, tpgr_db->al_mac_addr, ETH_ALEN)) {
				memcpy(al_mac_addr, mac_addr, ETH_ALEN);
				result = 0;
				goto find_finish;
			}
			if (!SLIST_EMPTY(&(tpgr_db->devinfo_head))) {
				SLIST_FOREACH(dev_db, &(tpgr_db->devinfo_head), devinfo_entry) {
					if (!memcmp(mac_addr, dev_db->mac_addr, ETH_ALEN)) {
						memcpy(al_mac_addr, tpgr_db->al_mac_addr, ETH_ALEN);
						result = 0;
						goto find_finish;
					}
				}
			}
		}
	}

find_finish:
	return result;
}

/**
 *  get cmdu message version from header.
 *
 * \param  buf  input, cmdu message header start address
 * \retun  cmdu message version
 */
unsigned char get_mversion_from_msg_hdr(unsigned char *buf)
{
	return *buf;
}

/**
 *  get cmdu message type from header.
 *
 * \param  buf  input, cmdu message header start address
 * \retun  cmdu message type
 */
unsigned short get_mtype_from_msg_hdr(unsigned char *buf)
{
	unsigned short mtype = 0;

	mtype = *(unsigned short *)(buf + 2);
	mtype = be_to_host16(mtype);

	return mtype;
}

/**
 *  get cmdu message type string from header.
 *
 * \param  buf  input, cmdu message header start address
 * \retun  cmdu message type string
 */
char *get_mtype_str_from_msg_hdr(unsigned char *buf)
{
	static char str[64];
	unsigned short mtype = get_mtype_from_msg_hdr(buf);
	(void)snprintf((char *)str, sizeof(str), "Unknown:%04x", mtype);
	switch (mtype) {
	case TOPOLOGY_DISCOVERY:
		return "Topology Discovery";
	case TOPOLOGY_NOTIFICATION:
		return "Topology Notification";
	case TOPOLOGY_QUERY:
		return "Topology Query";
	case TOPOLOGY_RESPONSE:
		return "Topology Response";
	case VENDOR_SPECIFIC:
		return "Vendor Specific";
	case LINK_METRICS_QUERY:
		return "Link Metric Query";
	case LINK_METRICS_RESPONSE:
		return "Link Metric Response";
	case AP_AUTOCONFIG_SEARCH:
		return "AP-autoconfiguration Search";
	case AP_AUTOCONFIG_RESPONSE:
		return "AP-autoconfiguration Response";
	case AP_AUTOCONFIG_WSC:
		return "AP-autoconfiguration WSC";
	case AP_AUTOCONFIG_RENEW:
		return "AP-autoconfiguration Renew";
	case P1905_PB_EVENT_NOTIFY:
		return "1905.1 Push Button Event";
	case P1905_PB_JOIN_NOTIFY:
		return "1905.1 Push Button Join Notification";
	default:
		return str;
	}
}

/**
 *  get cmdu message id from header.
 *
 * \param  buf  input, cmdu message header start address
 * \retun  cmdu message id
 */
unsigned short get_mid_from_msg_hdr(unsigned char *buf)
{
	unsigned short mid = 0;

	mid = *(unsigned short *)(buf + 4);
	mid = be_to_host16(mid);

	return mid;
}

/**
 *  get cmdu fragment id from header.
 *
 * \param  buf  input, cmdu message header start address
 * \retun  cmdu fragment id
 */
unsigned char get_fid_from_msg_hdr(unsigned char *buf)
{
	return *(buf + 6);
}

/**
 *  get cmdu last fragment ind from header.
 *
 * \param  buf  input, cmdu message header start address
 * \retun  cmdu last fragment ind
 */
unsigned char get_last_fragment_ind_from_msg_hdr(unsigned char *buf)
{
	if (((*(buf + 7)) & 0x80) == 0x80)
		return 1;
	else
		return 0;
}

/**
 *  get cmdu relay ind from header.
 *
 * \param  buf  input, cmdu message header start address
 * \retun  cmdu relay ind
 */
unsigned char get_relay_ind_from_msg_hdr(unsigned char *buf)
{
	if (((*(buf + 7)) & 0x40) == 0x40)
		return 1;
	else
		return 0;
}

/**
 *  check whether receive a relay multicast message
 *
 *
 * \param  relay_ind  input, get from relay ind field of message header
 * \param  mtype  input, get from message type field og message header
 * \return -1: error  0: relay message
 */
int check_relay_message(unsigned char relay_ind, unsigned short mtype)
{
	if (relay_ind) {
		if ((mtype != TOPOLOGY_NOTIFICATION) && (mtype != VENDOR_SPECIFIC) &&
		    (mtype != AP_AUTOCONFIG_SEARCH) && (mtype != AP_AUTOCONFIG_RENEW) &&
		    (mtype != P1905_PB_EVENT_NOTIFY) && (mtype != P1905_PB_JOIN_NOTIFY)) {
			return -1;
		}
	}
	return 0;
}

/**
 *  parse topology discovery message
 *
 *
 * \param  ctx  1905.1 managerd context
 * \param  buf  input, tlvs start position of receive cmdu packet
 * \param  left_tlv_len  input, 1905 message's tlv length
 * \param  query  flag for sending topology query
 * \param  notify  flag for sending topology notification
 * \param  al_mac  output al mac address from this message
 * \return error code
 */
int parse_topology_discovery_message(struct p1905_managerd_ctx *ctx,
				     unsigned char *buf, unsigned int left_tlv_len,
				     unsigned char *query, unsigned char *notify,
				     unsigned char *discovery, unsigned char *al_mac, unsigned char *mac_addr,
				     unsigned char *smac)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned char al_mac_addr[ETH_ALEN];
	unsigned char itf_mac_addr[ETH_ALEN] = { 0 };
	unsigned char zero_mac[ETH_ALEN] = { 0 };
	unsigned char peer_itf_mac[ETH_ALEN] = { 0 };
	struct topology_discovery_db *tpg_db = NULL;
	unsigned char new_db = 1, if_sta = 0, if_eth = 0;
	unsigned int integrity = 0, ifid = 0;
	unsigned int right_integrity = 0x3;
	int eth_port = -1;
	struct vs_value vs_info;

	os_memset(&vs_info, 0, sizeof(struct vs_value));
	*discovery = 0;
	*query = 0;
	*notify = 0;
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

		if (*type == AL_MAC_ADDR_TLV_TYPE) {
			integrity |= (1 << 0);
			ret = parse_al_mac_addr_type_tlv(al_mac_addr, len, value);

			if (ret < 0) {
				debug(DEBUG_ERROR, "error al mac tlv\n");
				return -1;
			}
		} else if (*type == MAC_ADDR_TLV_TYPE) {
			integrity |= (1 << 1);
			ret = parse_mac_addr_type_tlv(peer_itf_mac, len, value);

			if (ret < 0) {
				debug(DEBUG_ERROR, "error mac tlv\n");
				return -1;
			}
		} else if (*type == VENDOR_SPECIFIC_TLV_TYPE) {
			ret = parse_vs_tlv(&vs_info, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error vs tlv\n");
				return -1;
			}
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}
	/*check integrity */
	if (integrity != right_integrity) {
		debug(DEBUG_ERROR, "incomplete topology discovery 0x%x 0x%x\n", integrity, right_integrity);
		return -1;
	}

	if ((integrity&BIT(1)) && os_memcmp(zero_mac, peer_itf_mac, ETH_ALEN))
		os_memcpy(mac_addr, peer_itf_mac, ETH_ALEN);

	/*sw solution for BCM */
	if (check_C_and_A_concurrent__by_discovery(ctx, al_mac_addr, mac_addr) < 0)
		return -1;

	*discovery = vs_info.discovery;
	os_memcpy(al_mac, al_mac_addr, ETH_ALEN);
	os_memcpy(itf_mac_addr, vs_info.itf_mac, ETH_ALEN);
	if (!os_memcmp(zero_mac, itf_mac_addr, ETH_ALEN)) {
		os_memcpy(itf_mac_addr, ctx->itf[ctx->recent_cmdu_rx_if_number].mac_addr, ETH_ALEN);
		debug(DEBUG_ERROR, TOPO_PREX "don't get recv intf from mapfilter. set to " MACSTR "\n",
		      PRINT_MAC(itf_mac_addr));
	} else if (os_memcmp(vs_info.itf_mac, ctx->itf[ctx->recent_cmdu_rx_if_number].mac_addr, ETH_ALEN)) {
		for (ifid = 0; ifid < ctx->itf_number; ifid++) {
			if (!os_memcmp(ctx->itf[ifid].mac_addr, vs_info.itf_mac, ETH_ALEN)) {
				ctx->recent_cmdu_rx_if_number = ifid;
				break;
			}
		}
	}

	ifid = ctx->recent_cmdu_rx_if_number;
	if (ctx->itf[ifid].is_wifi_sta) {
		if_sta = 1;
	} else if (ctx->itf[ifid].media_type == IEEE802_3_GROUP) {
		if_eth = 1;
		if (ctx->itf[ifid].is_veth == 1) {
			debug(DEBUG_ERROR, TOPO_PREX "%s is veth intf\n", ctx->itf[ifid].if_name);
			if_eth = 0;
		}
	}

	if (!ctx->MAP_Cer) {
		/*for eth device, we should get which switch port this topology discoery comes from */
		if (if_eth) {
			eth_port = eth_layer_get_client_port_by_mac(al_mac_addr);
			if (eth_port == ETH_ERROR_FAIL) {
				debug(DEBUG_ERROR, TOPO_PREX "eth_port(-1), drop discovery\n");
				return -1;
			}
		} else {
			eth_port = -1;
		}

		if (eth_port >= 0 && ctx->conn_port.is_set &&
		    ctx->conn_port.port_num == eth_port &&
		    os_memcmp(ctx->conn_port.filter_almac, al_mac_addr, ETH_ALEN)) {
			debug(DEBUG_ERROR, TOPO_PREX "GPON case!port_num(%d) drop discovery" MACSTR "\n",
			      eth_port, PRINT_MAC(al_mac_addr));
			return -1;
		}
	}

	if (if_sta && ctx->itf[ifid].vs_info_length >= ETH_ALEN
	    && os_memcmp(mac_addr, ctx->itf[ifid].vs_info, ETH_ALEN) != 0
	    && os_memcmp(ctx->itf[ifid].vs_info, zero_mac, ETH_ALEN) != 0) {
		debug(DEBUG_ERROR, TOPO_PREX "recv non-directly connected dev discovery! drop!"
		      "recv intf(%s) "
		      "connected bssid(" MACSTR ") "
		      "peer tx intf mac(" MACSTR ")\n",
		      ctx->itf[ifid].if_name, PRINT_MAC(ctx->itf[ifid].vs_info), PRINT_MAC(mac_addr));
		*query = 0;
		*notify = 0;
		return 0;
	}

	/*search the AL MAC ADDRESS exist or not */
	LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry) {
		/*compare the AL MAC ADDR and INF MAC ADDR with database */
		if (!memcmp(tpg_db->al_mac, al_mac_addr, ETH_ALEN) && !memcmp(tpg_db->itf_mac, mac_addr, ETH_ALEN)) {
			new_db = 0;
			break;
		}
	}

	if (new_db) {
		*query = 1;
		*notify = 1;
		/*no this AL MAC ADDR , add a new topology_discovery_db component in database */
		tpg_db = (struct topology_discovery_db *)malloc(sizeof(struct topology_discovery_db));
		if (!tpg_db) {
			debug(DEBUG_ERROR, TOPO_PREX "alloc tpg_db fail\n");
			return -1;
		}
		memset(tpg_db, 0, sizeof(*tpg_db));
		memcpy(tpg_db->al_mac, al_mac_addr, ETH_ALEN);
		memcpy(tpg_db->itf_mac, mac_addr, ETH_ALEN);
		memcpy(tpg_db->receive_itf_mac, itf_mac_addr, ETH_ALEN);
		tpg_db->time_to_live = TOPOLOGY_DISCOVERY_TTL;
		LIST_INSERT_HEAD(&ctx->topology_entry.tpddb_head, tpg_db, tpddb_entry);
	} else {
		tpg_db->time_to_live = TOPOLOGY_DISCOVERY_TTL;
	}

	if (tpg_db->eth_port != eth_port) {
		debug(DEBUG_ERROR, TOPO_PREX "port change!!! prev_port(%d) new_port(%d)\n", tpg_db->eth_port, eth_port);
		*query = 1;
	}
	tpg_db->eth_port = eth_port;

	if (0 > update_p1905_neighbor_dev_info(ctx, al_mac_addr, mac_addr, itf_mac_addr, notify)) {
		debug(DEBUG_ERROR, TOPO_PREX "update_p1905_neighbor_dev_info fail\n");
		return -1;
	}

	if (if_eth && eth_port >= 0 && new_db) {
		*notify = 2;
		debug(DEBUG_OFF, TOPO_PREX "new eth conncetion!\n");
	}

	if (*notify)
		report_own_topology_rsp(ctx, ctx->p1905_al_mac_addr, ctx->br_cap,
					ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
					ctx->service, &ctx->ap_cap_entry.oper_bss_head,
					&ctx->ap_cap_entry.assoc_clients_head, ctx->cnt);

	debug(DEBUG_ERROR, TOPO_PREX "recv discovery almac(" MACSTR ") "
	      "peer intf(" MACSTR ") "
	      "recv intf(%s) "
	      "new db(%d) eth_port(%d) "
	      "need_query(%d) need_notify(%d) need_discovery(%d)\n",
	      PRINT_MAC(tpg_db->al_mac),
	      PRINT_MAC(tpg_db->itf_mac), ctx->itf[ifid].if_name, new_db, eth_port, *query, *notify, *discovery);

	return 0;
}

/**
 *  parse topology notification message
 *
 *
 * \param  ctx  1905.1 managerd context
 * \param  buf  input, tlvs start position of receive cmdu packet
 * \param  left_tlv_len  input, 1905 message's tlv length
 * \param  al_mac  output al mac address from this message
 * \return error code
 */
int parse_topology_notification_message(struct p1905_managerd_ctx *ctx,
					unsigned char *buf, unsigned int left_tlv_len,
					unsigned char *al_mac, unsigned char *bssid,
					unsigned char *mac, unsigned char *event, unsigned char *neth_almac)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned char al_mac_addr[ETH_ALEN];
	unsigned int integrity = 0;
	unsigned int right_integrity = 0x1;
	struct vs_value vs_info;
	struct dl_list clients_table = {0};
#ifdef MAP_R2
	struct assoc_client *client = NULL;
	struct assoc_client *client_tmp = NULL;
	struct assoc_client *assoc_client = NULL;
#endif
	os_memset(&vs_info, 0, sizeof(struct vs_value));
	os_memset(al_mac_addr, 0, sizeof(al_mac_addr));
	type = buf;
	reset_stored_tlv(ctx);
	dl_list_init(&clients_table);

	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = %d, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left_tlv_len, tlv_len);
			goto fail;
		}

		if (*type == AL_MAC_ADDR_TLV_TYPE) {
			integrity |= (1 << 0);
			ret = parse_al_mac_addr_type_tlv(al_mac_addr, len, value);

			if (ret < 0) {
				debug(DEBUG_ERROR, "error al mac tlv\n");
				goto fail;
			}
			if (!memcmp(ctx->p1905_al_mac_addr, al_mac_addr, ETH_ALEN))
				goto fail;
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == CLIENT_ASSOCIATION_EVENT_TYPE) {
			ret = parse_client_assoc_event_tlv(ctx, mac, bssid, event, al_mac_addr, len, value, &clients_table);

			if (ret < 0) {
				debug(DEBUG_ERROR, "error client_assoc_event tlv\n");
				goto fail;
			}
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == VENDOR_SPECIFIC_TLV_TYPE) {
			ret = parse_vs_tlv(&vs_info, len, value);

			if (ret < 0) {
				debug(DEBUG_ERROR, "error vs tlv\n");
				goto fail;
			}
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		} else {
			debug(DEBUG_ERROR, "wrong TLV in topology notification message\n");
			/*ignore extra tlv */
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		}

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	/*check integrity */
	if (integrity != right_integrity) {
		debug(DEBUG_ERROR, "incomplete topology notification 0x%x 0x%x\n", integrity, right_integrity);
		goto fail;
	}

#ifdef MAP_R2
	if (!dl_list_empty(&clients_table)) {
		unsigned char hash = 0;

		dl_list_for_each_safe(client, client_tmp, &clients_table, struct assoc_client, entry) {
			dl_list_del(&client->entry);
			assoc_client = get_assoc_cli_by_mac(ctx, client->mac);
			if (assoc_client) {
				assoc_client->disassoc = client->disassoc;
				free(client);
				client = NULL;
			} else {
				hash = MAC_ADDR_HASH_INDEX(client->mac);
				dl_list_add(&ctx->clients_table[hash], &client->entry);
			}
		}
	}
#endif

#ifdef MAP_R2
	maintain_all_assoc_clients(ctx);
#endif
	os_memcpy(al_mac, al_mac_addr, ETH_ALEN);
	os_memcpy(neth_almac, vs_info.neth_almac, ETH_ALEN);
	return 0;
fail:

#ifdef MAP_R2
	if (!dl_list_empty(&clients_table)) {
		dl_list_for_each_safe(client, client_tmp, &clients_table, struct assoc_client, entry) {
			dl_list_del(&client->entry);
			free(client);
		}
	}
#endif
	return -1;
}

int parse_map_version_tlv(unsigned char *buf, unsigned char *almac, unsigned char *profile)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
#ifdef MAP_R3
	struct r3_member *peer = NULL;
#endif

	temp_buf = buf;
	if ((*temp_buf) == MULTI_AP_VERSION_TYPE)
		temp_buf++;
	else
		return -1;

	/* calculate tlv length */
	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);
	/* shift to tlv value field */
	temp_buf += 2;
	*profile = *temp_buf;
#ifdef MAP_R3
	peer = get_r3_member(almac);

	if (peer && peer->security_1905 && (peer->profile < MAP_PROFILE_R3)) {
		debug(DEBUG_ERROR, "disable security_1905 for " MACSTR "\n", MAC2STR(almac));
		peer->security_1905 = 0;
	}
#endif
	return (length+3);
}

#ifdef MAP_R2
int parse_topology_query_message(struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned int left_tlv_len,
				 unsigned char *almac)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned char map_profile = MAP_PROFILE_R1;

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

		if (*type == MULTI_AP_VERSION_TYPE) {
			ret = parse_map_version_tlv(type, almac, &map_profile);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error map version tlv\n");
				return -1;
			}
			break;
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	return 0;
}
#endif

/**
 *  parse topology notification message
 *
 *
 * \param  ctx  1905.1 managerd context
 * \param  buf  input, tlvs start position of receive cmdu packet
 * \return error code
 */
int parse_topology_response_message(struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned int left_tlv_len,
	unsigned char *almac, unsigned char *profile)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned int left = 0;
	unsigned char al_mac_addr[ETH_ALEN];
	struct topology_response_db *tpgr_db;
	unsigned int integrity = 0;
	unsigned int right_integrity = 0x3;
	unsigned char map_profile = MAP_PROFILE_R1;
	unsigned char support_service = 0xff;
#ifdef MAP_R2
	struct dl_list bss_head = {0};
	struct dl_list clients_table = {0};
	struct assoc_client *client = NULL;
	struct assoc_client *client_tmp = NULL;
	struct assoc_client *assoc_client = NULL;
	struct operational_bss *bss = NULL;
	struct operational_bss *bss_tmp = NULL;
#endif
	struct list_head_devinfo devinfo_list = {0};
	struct device_info_db *dev = NULL;
	struct device_info_db *dev_tmp = NULL;
	struct list_head_brcap brcap_list = {0};
	struct device_bridge_capability_db *br = NULL;
	struct device_bridge_capability_db *br_tmp = NULL;


	type = buf;
	left = left_tlv_len;

	/*we must get the AL MAC addr to get correct database pointer
	 * , so need to search device info tlv  firstly
	 */
	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (left < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = %d, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left, tlv_len);
			return -1;
		}

		if ((*type) == DEVICE_INFO_TLV_TYPE)
			integrity |= (1 << 0);
		else if (*type == SUPPORTED_SERVICE_TLV_TYPE)
			integrity |= (1 << 1);
		else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left -= tlv_len;
	}

	/*check integrity */
	if (integrity != right_integrity) {
		debug(DEBUG_ERROR, TOPO_PREX "incomplete topology response 0x%x 0x%x for dev(" MACSTR ")\n",
		      integrity, right_integrity, PRINT_MAC(almac));
		return -1;
	}

	type = buf;

	/* type +3 shift to AL MAC addr field in device information tlv */
	memcpy(al_mac_addr, type + 3, ETH_ALEN);
	memcpy(almac, al_mac_addr, ETH_ALEN);

	/*delete the database if already exist */
	tpgr_db = delete_exist_topology_response_content(ctx, al_mac_addr);
	if (!tpgr_db) {
		/*add a new topology_response_db component in database */
		tpgr_db = (struct topology_response_db *)os_zalloc(sizeof(struct topology_response_db));
		if (tpgr_db == NULL) {
			debug(DEBUG_ERROR, TOPO_PREX "allocate memory for tpgr_db fail for dev(" MACSTR ")\n",
			      PRINT_MAC(almac));
			return -1;
		}
		os_memcpy(tpgr_db->al_mac_addr, al_mac_addr, ETH_ALEN);
		SLIST_INSERT_HEAD(&(ctx->topology_entry.tprdb_head), tpgr_db, tprdb_entry);
	}
	os_get_time(&tpgr_db->last_seen);
	tpgr_db->recv_ifid = ctx->recent_cmdu_rx_if_number;
	tpgr_db->valid = 1;
	/*init its own device information list and bridge capacity list */
	SLIST_INIT(&tpgr_db->devinfo_head);
	LIST_INIT(&tpgr_db->brcap_head);
#ifdef MAP_R2
	dl_list_init(&bss_head);
	dl_list_init(&clients_table);
#endif
	SLIST_INIT(&devinfo_list);
	LIST_INIT(&brcap_list);

	/* until now, we get the correct pointer
	 * Move to the start of buf, then do tlvs parsing
	 */
	type = buf;
	left = left_tlv_len;
	reset_stored_tlv(ctx);
	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (left < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = %d, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left, tlv_len);
			goto fail;
		}

		if (*type == DEVICE_INFO_TLV_TYPE) {
			/*then shift back to start of device info tlv, parsing it first */
			ret = parse_device_info_type_tlv(ctx, &devinfo_list, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, TOPO_PREX "error device info tlv for dev(" MACSTR ")\n",
				      PRINT_MAC(almac));
				goto fail;
			}
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == BRIDGE_CAPABILITY_TLV_TYPE) {
			ret = parse_bridge_capability_type_tlv(&brcap_list, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, TOPO_PREX "error bridge capability tlv for dev(" MACSTR ")\n",
				      PRINT_MAC(almac));
				goto fail;
			}
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == P1905_NEIGHBOR_DEV_TLV_TYPE) {
			ret = parse_p1905_neighbor_device_type_tlv(&devinfo_list, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, TOPO_PREX "error p1905.1 neighbor device tlv for dev(" MACSTR ")\n",
				      PRINT_MAC(almac));
				goto fail;
			}
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == NON_P1905_NEIGHBOR_DEV_TLV_TYPE) {
			ret = parse_non_p1905_neighbor_device_type_tlv(&devinfo_list, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR,
				      TOPO_PREX "error non-p1905.1 neighbor device tlv for dev(" MACSTR ")\n",
				      PRINT_MAC(almac));
				goto fail;
			}
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == SUPPORTED_SERVICE_TLV_TYPE) {
			ret = parse_supported_service_tlv(&support_service, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, TOPO_PREX "error supported service tlv for dev(" MACSTR ")\n",
				      PRINT_MAC(almac));
				goto fail;
			}
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		}
#ifdef MAP_R2
		else if (*type == AP_OPERATIONAL_BSS_TYPE) {
			ret = parse_ap_operational_bss_tlv(ctx, al_mac_addr, len, value, &bss_head);
			if (ret < 0) {
				debug(DEBUG_ERROR, TOPO_PREX "error ap_operational_bss tlv for dev(" MACSTR ")\n",
				      PRINT_MAC(almac));
				goto fail;
			}
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == AP_ASSOCIATED_CLIENTS_TYPE) {
			ret = parse_assoc_clients_tlv(ctx, almac, len, value, &clients_table);
			if (ret < 0) {
				debug(DEBUG_ERROR, TOPO_PREX "error ap associated clients tlv for dev(" MACSTR ")\n",
				      PRINT_MAC(almac));
				goto fail;
			}
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		}
#endif /* #ifdef MAP_R2 */

		else if (*type == MULTI_AP_VERSION_TYPE) {
			ret = parse_map_version_tlv(type, almac, &map_profile);
			if (ret < 0) {
				debug(DEBUG_ERROR, TOPO_PREX "error steering request tlv for dev(" MACSTR ")\n",
				      PRINT_MAC(almac));
				goto fail;
			}
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
			tpgr_db->profile = map_profile;
			*profile = map_profile;
		}
		else if (*type == END_OF_TLV_TYPE)
			break;
		else {
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		}

		type += tlv_len;
		left -= tlv_len;
	}

	if (support_service != 0xff)
		tpgr_db->support_service = support_service;

	update_dev_info(&tpgr_db->devinfo_head, &devinfo_list);
	update_bridge_cap(&tpgr_db->brcap_head, &brcap_list);
#ifdef MAP_R2
	if (!dl_list_empty(&bss_head)) {
		dl_list_for_each_safe(bss, bss_tmp, &bss_head, struct operational_bss, entry) {
			dl_list_del(&bss->entry);
			dl_list_add(&ctx->bss_head, &bss->entry);
		}
	}

	if (!dl_list_empty(&clients_table)) {
		unsigned char hash = 0;

		dl_list_for_each_safe(client, client_tmp, &clients_table, struct assoc_client, entry) {
			dl_list_del(&client->entry);
			assoc_client = get_assoc_cli_by_mac(ctx, client->mac);
			if (assoc_client) {
				os_memcpy(assoc_client->bssid, client->bssid, ETH_ALEN);
				free(client);
				client = NULL;
			} else {
				hash = MAC_ADDR_HASH_INDEX(client->mac);
				dl_list_add(&ctx->clients_table[hash], &client->entry);
			}
		}
	}
#endif
	/*sw solution for BCM */
	if (check_C_and_A_concurrent_by_topology_rsp(ctx, tpgr_db) < 0)
		return -1;

	if (ctx->itf[ctx->recent_cmdu_rx_if_number].media_type == IEEE802_3_GROUP) {
		tpgr_db->eth_port = eth_layer_get_client_port_by_mac(al_mac_addr);
		if (tpgr_db->eth_port == ETH_ERROR_FAIL)
			debug(DEBUG_ERROR, TOPO_PREX "BUG!tpgr_db->eth_port should not be -1\n");
	} else
		tpgr_db->eth_port = -1;

	debug(DEBUG_TRACE, TOPO_PREX "revv rsp eth_port=%d\n", tpgr_db->eth_port);
	/*shall not enable PON with MAP feature in Certification case */
	if (!ctx->MAP_Cer)
		mask_control_conn_port(ctx, tpgr_db);
#ifdef MAP_R2
	update_operation_bss_vlan(ctx, &ctx->bss_head, &ctx->map_policy.tpolicy, al_mac_addr);
	maintain_all_assoc_clients(ctx);
#endif
	return 0;

fail:

	(void *)delete_exist_topology_response_content(ctx, al_mac_addr);
	if (!SLIST_EMPTY(&devinfo_list)) {
		dev = SLIST_FIRST(&devinfo_list);
		while (dev) {
			dev_tmp = SLIST_NEXT(dev, devinfo_entry);
			SLIST_REMOVE(&devinfo_list, dev, device_info_db, devinfo_entry);
			free(dev);
			dev = dev_tmp;
		}
	}

	if (!LIST_EMPTY(&brcap_list)) {
		br = LIST_FIRST(&brcap_list);
		while (br) {
			br_tmp = LIST_NEXT(br, brcap_entry);
			LIST_REMOVE(br, brcap_entry);
			free(br);
			br = br_tmp;
		}
	}
#ifdef MAP_R2
	if (!dl_list_empty(&bss_head)) {
		dl_list_for_each_safe(bss, bss_tmp, &bss_head, struct operational_bss, entry) {
			dl_list_del(&bss->entry);
			free(bss);
		}
	}

	if (!dl_list_empty(&clients_table)) {
		dl_list_for_each_safe(client, client_tmp, &clients_table, struct assoc_client, entry) {
			dl_list_del(&client->entry);
			free(client);
		}
	}
#endif
	return -1;
}

void query_neighbor_info(struct p1905_managerd_ctx *ctx, unsigned char *almac, unsigned int if_number)
{
	struct topology_response_db *tpgr = NULL, *tpgr_tmp = NULL;
	struct device_info_db *dev_info = NULL;
	struct p1905_neighbor_device_db *dev = NULL;
	unsigned char exist = 0;

	SLIST_FOREACH(tpgr, &ctx->topology_entry.tprdb_head, tprdb_entry) {
		if (!os_memcmp(tpgr->al_mac_addr, almac, 6))
			break;
	}

	if (!tpgr) {
		debug(DEBUG_ERROR, TOPO_PREX "something wrong!!! no 1905.1 dev " MACSTR "\n", PRINT_MAC(almac));
		return;
	}

	SLIST_FOREACH(dev_info, &tpgr->devinfo_head, devinfo_entry) {
		SLIST_FOREACH(dev, &dev_info->p1905_nbrdb_head, p1905_nbrdb_entry) {
			if (!os_memcmp(dev->p1905_neighbor_al_mac, ctx->p1905_al_mac_addr, ETH_ALEN))
				continue;

			exist = 0;
			SLIST_FOREACH(tpgr_tmp, &ctx->topology_entry.tprdb_head, tprdb_entry) {
				if (os_memcmp(tpgr_tmp->al_mac_addr, almac, ETH_ALEN)) {
					if (!os_memcmp(tpgr_tmp->al_mac_addr, dev->p1905_neighbor_al_mac, ETH_ALEN) &&
					    (tpgr_tmp->valid == 1)) {
						debug(DEBUG_INFO, TOPO_PREX "topo exist almac " MACSTR "\n",
						      PRINT_MAC(dev->p1905_neighbor_al_mac));
						exist = 1;
						break;
					}
				}
			}
			if (!exist) {
				insert_cmdu_txq(dev->p1905_neighbor_al_mac, ctx->p1905_al_mac_addr,
						e_topology_query, ++ctx->mid, ctx->itf[if_number].if_name, 0);
				debug(DEBUG_ERROR, TOPO_PREX "tx query(mid-%04x) to 1905.1 dev(" MACSTR ") on %s\n",
				      ctx->mid, PRINT_MAC(dev->p1905_neighbor_al_mac), ctx->itf[if_number].if_name);
			}
		}
	}
}

unsigned char is_internal_used_vendor_specific(unsigned char *vs_info)
{
	unsigned char charator_value[4] = { 0xFF, 0x00, 0x0C, 0xE7 };
	/*0xff, 0x00, 0x0C, 0xE7 */

	if (!os_memcmp(vs_info, charator_value, sizeof(charator_value)))
		return 1;
	else
		return 0;
}

/**
 *  parse vendor specific message. It for PC tool use.
 *
 *
 * \param  ctx  1905.1 managerd context
 * \param  vs_info  input
 * \return error code
 */
int parse_vendor_specific_message(struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned int left_tlv_len,
				  struct p1905_vs_info *vs_info, unsigned char *internal_vendor_message)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned int tlv_len = 0;
	unsigned int integrity = 0;
	unsigned int right_integrity = 0x1;

	type = buf;
	reset_stored_tlv(ctx);
	*internal_vendor_message = 0;
	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = %d, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}

		if (*type == VENDOR_SPECIFIC_TLV_TYPE) {
			integrity |= (1 << 0);
			if (tlv_len >= 8 && is_internal_used_vendor_specific(type + 6)) {
				*internal_vendor_message = 1;
				debug(DEBUG_OFF, "internal used vendor specific message\n");
			}
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}

		store_revd_tlvs(ctx, type, (unsigned short)tlv_len);

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	/*check integrity */
	if (integrity != right_integrity) {
		debug(DEBUG_ERROR, "incomplete vendor specific 0x%x 0x%x\n", integrity, right_integrity);
		return -1;
	}

	return 0;
}

/**
 *  parse 1905 internal using vendor specific message. It for PC tool use.
 *
 *
 * \param  ctx  1905.1 managerd context
 * \param  vs_info  input
 * \return error code
 */
int parse_1905_internal_vendor_specific_message(struct p1905_managerd_ctx *ctx, unsigned char *buf,
						unsigned int left_tlv_len)
{
	unsigned char *temp_buf;
	unsigned char need_query = 0, need_notify = 0, discvoery = 0;
	unsigned char peer_al_mac[ETH_ALEN] = { 0x00 };
	unsigned char peer_itf_mac[ETH_ALEN] = { 0x00 };
	int j = 0;
	unsigned char *p = NULL;
	int len = 0, total_len = 0;
	unsigned int left = left_tlv_len;

	/*1905 internal using vendor specific tlv format
	   type                 1 byte                  0x11
	   length                       2 bytes                 0x, 0x
	   oui                  3 bytes                 0x00, 0x0C, 0xE7
	   function type        1 byte                  0xff
	   suboui                       3 bytes                 0x00, 0x0C, 0xE7
	   sub func type        1 byte                  0x
	 */
	temp_buf = buf + 10;
	if (left < 10) {
		debug(DEBUG_ERROR, "[%d] Error TLV len, left_tlv_length %d less than len 10\n",
		      __LINE__, left);
		return -1;
	}
	left -= 10;

	debug(DEBUG_ERROR, "vendor specific internal message, subtype=%02x\n", *temp_buf);
	switch (*temp_buf) {
	case internal_vendor_discovery_message_subtype:
		{
			if (parse_topology_discovery_message(ctx, buf, left_tlv_len,
							     &need_query, &need_notify, &discvoery, peer_al_mac,
							     peer_itf_mac, NULL) < 0) {
				debug(DEBUG_ERROR, "receive error vendor specific topology discovery message\n");
				break;
			}

			for (j = 0; j < ctx->itf_number; j++) {
				/*send multicast cmdu to all the ethenet interface */
				if (ctx->itf[j].media_type == IEEE802_3_GROUP) {
					debug(DEBUG_OFF,
					      "receive specific discovery, reply topology discovery by interface[%s]\n",
					      ctx->itf[j].if_name);
					insert_cmdu_txq((unsigned char *)p1905_multicast_address,
							ctx->p1905_al_mac_addr, e_topology_discovery, ++ctx->mid,
							ctx->itf[j].if_name, 0);
				}
			}
			topology_discovery_action(ctx, peer_al_mac, need_query, need_notify);
			update_topology_tree(ctx);
		}
		break;
	case internal_vendor_wts_content_message_subtype:
		{
			p = buf;
			while (*p == VENDOR_SPECIFIC_TLV_TYPE) {
				len = get_cmdu_tlv_length(p);
				if (left < len) {
					debug(DEBUG_ERROR, "[%d] Error TLV len, left_tlv_length %d less than len %d\n",
					      __LINE__, left, len);
					return -1;
				}
				left -= len;

				p += len;
				total_len += len;
			}
			debug(DEBUG_OFF,
			      "receive internal_vendor_wts_content_message_subtype message total_len1(%d)\n",
			      total_len);
			p = buf;
			while (*p == VENDOR_SPECIFIC_TLV_TYPE) {
				len = get_cmdu_tlv_length(p);
				if (left < len) {
					debug(DEBUG_ERROR, "[%d] Error TLV len, left_tlv_length %d less than len %d\n",
					      __LINE__, left, len);
					return -1;
				}
				left -= len;

				os_memmove(p, p + 11, total_len - 11);
				p += (len - 11);
				total_len -= len;
				if (!total_len)
					break;
			}
			*p = 0;
			debug(DEBUG_OFF,
			      "receive internal_vendor_wts_content_message_subtype message total_len0(%d)\n",
			      total_len);
			if (_1905_write_wts_file_content((char *)buf, ctx->wts_bss_cfg_file))
				debug(DEBUG_ERROR, "write wts file fail\n");
		}
		break;
#ifdef MAP_R3_SP
	case service_prioritization_vendor_rule_message_subtype:
		{
			unsigned int integrity = 0;
			struct dl_list static_list_tmp;
			struct dl_list dynamic_list_tmp;
			struct dl_list learn_complete_list_tmp;

			dl_list_init(&static_list_tmp);
			dl_list_init(&dynamic_list_tmp);
			dl_list_init(&learn_complete_list_tmp);

			if (ctx->role != CONTROLLER)
				break;

			len = get_tlv_len(buf);
			if (parse_sp_vendor_tlv(buf+3, len,
				ctx->p1905_al_mac_addr, &integrity, &static_list_tmp,
				&dynamic_list_tmp, &learn_complete_list_tmp) < 0) {
				clear_rule_list(&static_list_tmp);
				clear_rule_list(&dynamic_list_tmp);
				clear_rule_list(&learn_complete_list_tmp);
				debug(DEBUG_ERROR, "error sp vendor rule tlv\n");
				return -1;
			}

			if (integrity & BIT(0)) {
				sync_rule_between_list(&ctx->sp_rule_static_list, &static_list_tmp);
				sync_rule_between_list(&ctx->sp_rule_dynamic_list, &dynamic_list_tmp);
			}
			if (integrity & BIT(2)) {
				clear_rule_list(&ctx->sp_rule_static_list);
				clear_rule_list(&ctx->sp_rule_dynamic_list);
			}
			if (integrity & BIT(3))
				sync_rule_between_list(&ctx->sp_rule_learn_complete_list, &learn_complete_list_tmp);
		}
		break;
#endif
	default:
		debug(DEBUG_ERROR, "un-supported internal vendor message subtype\n");
		break;
	}

	return 0;
}

/**
 *  parse link metric query message.
 *
 *
 * \param  ctx  1905.1 managerd context
 * \param  buf  input, tlvs start position of receive cmdu packet
 * \return error code
 */
int parse_link_metric_query_message(struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned int integrity = 0;
	unsigned int right_integrity = 0x1;
	unsigned char target_mac[ETH_ALEN] = {0};
	unsigned char metric_type = 0xff;
	unsigned char zero_mac[ETH_ALEN] = { 0 };

	type = buf;

	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = %d, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}

		if (*type == LINK_METRICS_QUERY_TLV_TYPE) {
			integrity |= (1 << 0);

			ret = parse_link_metric_query_type_tlv(target_mac,
							       &metric_type, len, value);

			if (ret < 0) {
				debug(DEBUG_ERROR, "error link metric query tlv\n");
				return -1;
			}
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	/*check integrity */
	if (integrity != right_integrity) {
		debug(DEBUG_ERROR, "incomplete link metrics query 0x%x 0x%x\n", integrity, right_integrity);
		return -1;
	}

	if (integrity&BIT(0)) {
		if (os_memcmp(ctx->p1905_al_mac_addr, zero_mac, ETH_ALEN))
			os_memcpy(ctx->link_metric_response_para.target, target_mac, ETH_ALEN);

		if (metric_type != 0xff)
			ctx->link_metric_response_para.type = metric_type;
	}

	return 0;
}

#ifdef SUPPORT_ALME
/**
 *  parse link metric response message.
 *
 *
 * \param  ctx  1905.1 managerd context
 * \param  buf  input, tlvs start position of receive cmdu packet
 * \param  metric_rsp  pointer of alme response buffer
 * \return error code
 */

int parse_link_metric_response_message(struct p1905_managerd_ctx *ctx,
				       unsigned char *buf, ALME_GET_METRIC_RSP *metric_rsp)
{
	int length = 0;
	unsigned char *temp_buf;
	unsigned char got_tx_link_tlv = 0, got_rx_link_tlv = 0;

	temp_buf = buf;

	/* in implementation, I use BOTH_TX_AND_RX_METRICS to query link metrics.
	 * so I must get both RX & TX metrics tlv in this message.
	 * Based on this concept, I parse tx link metrics tlv first to get the number
	 * of connecting interfaces and then parse rx link metrics tlv.
	 */
	while (1) {
		if (*temp_buf == TRANSMITTER_LINK_METRIC_TYPE) {
			length = parse_transmitter_link_metrics_type_tlv(temp_buf, ctx->p1905_al_mac_addr, metric_rsp);

			if (length < 0) {
				debug(DEBUG_ERROR, "error parse transmitted link metrics tlv type\n");
				return -1;
			}

			got_tx_link_tlv = 1;
			temp_buf += length;
		} else if (*temp_buf == RESULT_CODE_TLV_TYPE) {
			length = parse_link_metrics_result_code_type_tlv(temp_buf);

			if (length < 0) {
				debug(DEBUG_ERROR, "link metrics response with invalid neighbor\n");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == END_OF_TLV_TYPE)
			break;
		else {
			/*ignore other tlv */
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
	if (!got_tx_link_tlv) {
		debug(DEBUG_ERROR, "no tx link metrics tlv\n");
		return -1;
	}

	/*return to start position again to parse rx link metrics tlv */
	temp_buf = buf;
	while (1) {
		if (*temp_buf == RECEIVER_LINK_METRIC_TYPE) {
			length = parse_receiver_link_metrics_type_tlv(temp_buf, ctx->p1905_al_mac_addr, metric_rsp);

			if (length < 0) {
				debug(DEBUG_ERROR, "error parse receiver link metrics tlv type\n");
				return -1;
			}

			got_rx_link_tlv = 1;
			temp_buf += length;
		} else if (*temp_buf == END_OF_TLV_TYPE)
			break;
		else {
			/* ignore other tlv */
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
	if (!got_rx_link_tlv) {
		debug(DEBUG_ERROR, "no rx link metrics tlv\n");
		return -1;
	}

	/* no integrity check because we already used got_tx_link_tlv and
	 * got_rx_link_tlv to check
	 */

	return 0;
}
#endif

/**
 *  parse auto configuration search message.
 *
 *
 * \param  ctx  1905.1 managerd context
 * \param  buf  input, tlvs start position of receive cmdu packet
 * \param  al_mac output, gotten from the al mac type tlv
 * \return error code
 */
int parse_ap_autoconfig_search_message(struct p1905_managerd_ctx *ctx,
				       unsigned char *buf, unsigned int left_tlv_len, unsigned char *al_mac,
				       unsigned char *profile)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned char peer_search_band = 0;
	unsigned int integrity = 0;
	unsigned int right_integrity = 0x7;

#ifdef MAP_R3
	struct r3_member *peer = NULL;
	struct chirp_tlv_info chirp = {0};
#endif

	reset_stored_tlv(ctx);
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

		if (*type == AL_MAC_ADDR_TLV_TYPE) {
			ret = parse_al_mac_addr_type_tlv(al_mac, len, value);

			if (ret < 0) {
				debug(DEBUG_ERROR, "error al mac tlv\n");
				return -1;
			}
			if (!memcmp(ctx->p1905_al_mac_addr, al_mac, ETH_ALEN)) {
				debug(DEBUG_ERROR, "receive search sent by myself\n");
				return -1;
			}

			integrity |= (1 << 0);
		} else if (*type == SEARCH_ROLE_TLV_TYPE) {
			ret = parse_search_role_tlv(len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error search role tlv\n");
				return -1;
			}
			integrity |= (1 << 1);
		} else if (*type == AUTO_CONFIG_FREQ_BAND_TLV_TYPE) {
			ret = parse_auto_config_freq_band_tlv(&peer_search_band, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error freq tlv\n");
				return -1;
			}
			integrity |= (1 << 2);
		}
#ifdef MAP_R2
		else if (*type == MULTI_AP_VERSION_TYPE) {
			ret = parse_map_version_tlv(type, al_mac, profile);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error map version tlv\n");
				return -1;
			}
			integrity |= (1 << 3);
		}
#endif // (defined(MAP_R2)

#ifdef MAP_R3
		else if (*type == DPP_CHIRP_VALUE_TYPE) {
			os_memset(&chirp, 0, sizeof(struct chirp_tlv_info));
			ret = parse_dpp_chirp_value_tlv(value, len, &chirp);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error freq tlv\n");
				return -1;
			}
			integrity |= (1 << 4);
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		}
#endif
		else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	/*check integrity */
	if (integrity < right_integrity) {
		debug(DEBUG_ERROR, "incomplete ap auto config search 0x%x 0x%x\n", integrity, right_integrity);
		return -1;
	}

	if (integrity & BIT(2))
		ctx->peer_search_band = peer_search_band;

#ifdef MAP_R3
	peer = create_r3_member(al_mac);
	if (!peer) {
		debug(DEBUG_ERROR, "[%d]create_r3_member: " MACSTR " fail!\n", __LINE__, MAC2STR(al_mac));
		return -1;
	}

	if (integrity & BIT(3)) {
		peer->profile = *profile;
		debug(DEBUG_TRACE, "update " MACSTR " profile to %d\n", MAC2STR(al_mac), peer->profile);
	}

	if (integrity & BIT(4)) {
		if (ctx->role == CONTROLLER) {
			os_memcpy(&peer->chirp, &chirp, sizeof(struct chirp_tlv_info));
			add_chirp_hash_list(ctx, &chirp, al_mac);
		}
	}
#endif

	return 0;
}

/**
 *  parse auto configuration response message.
 *
 *
 * \param  ctx  1905.1 managerd context
 * \param  buf  input, tlvs start position of receive cmdu packet
 * \return error code
 */

int parse_ap_autoconfig_response_message(struct p1905_managerd_ctx *ctx,
					 unsigned char *buf, unsigned int left_tlv_len,
					 unsigned char *band, unsigned char *service,
					 unsigned char *role_register, unsigned char *al_mac, unsigned char *profile,
					 unsigned char *kibmib)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned int integrity = 0;
	unsigned int right_integrity = 0x7;
	unsigned char freq_band = 0xff;

#ifdef MAP_R3
	struct chirp_tlv_info chirp = { 0 };
#endif
	type = buf;
	reset_stored_tlv(ctx);
	while (1) {
		len = get_tlv_len(type);
		value = type + 3;
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = %d, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left_tlv_len, tlv_len);
			return -1;
		}

		if (*type == SUPPORT_ROLE_TLV_TYPE) {
			integrity |= (1 << 0);

			ret = parse_supported_role_tlv(len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error support role tlv\n");
				return -1;
			}
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == SUPPORT_FREQ_BAND_TLV_TYPE) {
			integrity |= (1 << 1);

			ret = parse_supported_freq_band_tlv(&freq_band, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error support freq tlv\n");
				return -1;
			}
			store_revd_tlvs(ctx, type, tlv_len);
		} else if (*type == SUPPORTED_SERVICE_TLV_TYPE) {
			integrity |= (1 << 2);

			ret = parse_supported_service_tlv(service, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error support service tlv\n");
				return -1;
			}
			store_revd_tlvs(ctx, type, tlv_len);
		}

		else if (*type == MULTI_AP_VERSION_TYPE) {
			integrity |= (1 << 3);
			ret = parse_map_version_tlv(type, al_mac, profile);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error steering request tlv\n");
				return -1;
			}
		}
#ifdef MAP_R3
		else if (*type == _1905_LAYER_SECURITY_CAPABILITY_TYPE) {
			unsigned char onboardingProto = 0, msgIntAlg = 0, msgEncAlg = 0;
			integrity |= (1 << 4);
			ret = parse_1905_layer_security_capability_tlv(ctx, type, &onboardingProto,
								     &msgIntAlg, &msgEncAlg);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error 1905layer security cap tlv\n");
				return -1;
			}
		} else if (*type == DPP_CHIRP_VALUE_TYPE) {
			integrity |= (1 << 5);

			ret = parse_dpp_chirp_value_tlv(value, len, &chirp);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error freq tlv\n");
				return -1;
			}
		} else if (*type == CONTROLLER_CAPABILITY_TYPE) {
			integrity |= (1 << 6);

			ret = parse_controller_cap_tlv(type, kibmib);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error support service tlv\n");
				return -1;
			}
			store_revd_tlvs(ctx, type, tlv_len);
		}
#endif /* MAP_R3 */
		else if (*type == END_OF_TLV_TYPE) {
			break;
		} else {
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		}

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	/*check integrity */
	if (integrity < right_integrity) {
		debug(DEBUG_ERROR, "incomplete ap auto config response 0x%x 0x%x\n", integrity, right_integrity);
		return -1;
	}

	if (integrity&BIT(0))
		*role_register = 1;
	if (integrity&BIT(1))
		*band = freq_band;
	return 0;
}

int parse_ap_autoconfig_renew_message(struct p1905_managerd_ctx *ctx,
				      unsigned char *buf, unsigned int left_tlv_len,
				      unsigned char *al_mac, unsigned char *band)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned int integrity = 0;
	unsigned int right_integrity = 0x7;
	unsigned char freq_band = 0xff;
	unsigned char peer_al_mac[ETH_ALEN] = { 0 };
	unsigned char zero_mac[ETH_ALEN] = { 0 };

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

		if (*type == AL_MAC_ADDR_TLV_TYPE) {
			integrity |= (1 << 0);

			ret = parse_al_mac_addr_type_tlv(peer_al_mac, len, value);

			if (ret < 0) {
				debug(DEBUG_ERROR, "error al mac tlv\n");
				return -1;
			}
			if (!os_memcmp(ctx->p1905_al_mac_addr, peer_al_mac, ETH_ALEN))
				return -1;
		} else if (*type == SUPPORT_ROLE_TLV_TYPE) {
			integrity |= (1 << 1);

			ret = parse_supported_role_tlv(len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error support role tlv\n");
				return -1;
			}
		} else if (*type == SUPPORT_FREQ_BAND_TLV_TYPE) {
			integrity |= (1 << 2);

			ret = parse_supported_freq_band_tlv(&freq_band, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error support freq tlv\n");
				return -1;
			}
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	/*check integrity */
	if (integrity != right_integrity) {
		debug(DEBUG_ERROR, "incomplete ap auto config renew 0x%x 0x%x\n", integrity, right_integrity);
		return -1;
	}

	if ((integrity&BIT(0)) && (os_memcmp(peer_al_mac, zero_mac, ETH_ALEN)))
		os_memcpy(al_mac, peer_al_mac, ETH_ALEN);

	if (integrity&BIT(2))
		*band = freq_band;

	return 0;
}

/**
 *  parse wsc M1/M2 message.
 *
 *
 * \param  ctx  1905.1 managerd context
 * \param  buf  input, tlvs start position of receive cmdu packet
 * \return error code
 */

int parse_ap_autoconfig_wsc_message(struct p1905_managerd_ctx *ctx,
				    unsigned char *almac, unsigned char *buf, unsigned int left_tlv_len,
				    OUT struct agent_radio_info *r, OUT unsigned char *radio_cnt,
				    OUT unsigned char *radio_identifier)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned int integrity = 0;
	unsigned int rc_integrity = 0x1F;
	unsigned int rm_integrity = 0x1F;
	unsigned int right_integrity = 0;
	int m2_num = 0;
	struct agent_list_db *agent_info = NULL;
	unsigned char radio_if[ETH_ALEN] = {0};
	unsigned char zero_if[ETH_ALEN] = {0};

#ifdef MAP_R2
	struct vs_value vs_info;
	struct def_8021q_setting setting;
	struct traffics_policy tpolicy;
	struct traffic_separation_db *tpolicy_db = NULL;
	struct traffic_separation_db *tpolicy_db_nxt = NULL;
	int i = 0;

	os_memset(&setting, 0, sizeof(struct def_8021q_setting));
	os_memset(&tpolicy, 0, sizeof(struct traffics_policy));
	os_memset(&vs_info, 0, sizeof(struct vs_value));
	SLIST_INIT(&tpolicy.policy_head);
	vs_info.parse_ts = 1;
#endif
	type = buf;

	right_integrity = (ctx->role == CONTROLLER) ? rm_integrity : rc_integrity;

	if (ctx->role == CONTROLLER) {
		find_agent_info(ctx, almac, &agent_info);
		if (!agent_info) {
			debug(DEBUG_ERROR, "error! no agent info exist inset it\n");
			agent_info = insert_agent_info(ctx, almac);
		}
		if (!agent_info) {
			debug(DEBUG_ERROR, "error! fail to allocate agent\n");
			return -1;
		}
	}

	*radio_cnt = 0;

	while (1) {
		len = get_tlv_len(type);
		if (left_tlv_len < (len + 3)) {
			debug(DEBUG_ERROR, "[%d] Error TLV len, type = %d, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *type, left_tlv_len, tlv_len);
			goto fail;
		}
		value = type + 3;
		tlv_len = len + 3;

		if (*type == WSC_TLV_TYPE) {
			integrity |= (1 << 0);
			if (ctx->role == AGENT)
				memset(&ctx->ap_config_data, 0, sizeof(WSC_CONFIG));
			ret = parse_wsc_tlv(ctx, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX "error wsc tlv\n");
				goto fail;
			}
			/*copy all setting from each M2 */
			if (ctx->role == AGENT) {
#ifdef MAP_R3
				if ((ctx->map_version == MAP_PROFILE_R3) &&
				    (ctx->ap_config_data.AuthMode & AUTH_DPP_ONLY)) {
					debug(DEBUG_ERROR, AUTO_CONFIG_PREX "R1|R2 onboarding, need clear AKM DPP\n");
					debug(DEBUG_ERROR, AUTO_CONFIG_PREX "Orig AKM 0x%04x\n",
					      ctx->ap_config_data.AuthMode);
					ctx->ap_config_data.AuthMode &= (~AUTH_DPP_ONLY);
					debug(DEBUG_ERROR, AUTO_CONFIG_PREX "new AKM 0x%04x\n",
					      ctx->ap_config_data.AuthMode);
				}
#endif
				memcpy(&(ctx->current_autoconfig_info.config_data[m2_num]),
				       &ctx->ap_config_data, sizeof(WSC_CONFIG));
				m2_num++;
			}
		} else if (*type == AP_RADIO_IDENTIFIER_TYPE) {
			integrity |= (1 << 1);
			ret = parse_ap_radio_identifier_tlv(ctx, radio_if
#ifdef MAP_R2
							    , 0
#endif
							    , len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX "error ap radio identifier tlv\n");
				goto fail;
			}
		} else if (*type == AP_RADIO_BASIC_CAPABILITY_TYPE) {
			integrity |= (1 << 2);
			os_memset(r, 0, sizeof(struct agent_radio_info));
			/*length = parse_ap_radio_basic_cap_tlv(ctx, temp_buf, ctx->identifier,
			 * &ctx->peer_bss_need_config, &ctx->band_cap);
			 */
			*radio_cnt = *radio_cnt + 1;
			if (*radio_cnt > 1) {
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX "error!!! more than 1 ap radio basic cap tlv\n");
				goto fail;
			}
			ret = parse_ap_radio_basic_cap_tlv(ctx, r, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX "error ap radio basic cap tlv\n");
				goto fail;
			}
		}
#ifdef MAP_R2
		else if (*type == DEFAULT_8021Q_SETTING_TYPE) {
			ret = parse_default_802_1q_setting_tlv(&setting, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, AUTO_CONFIG_PREX "error default_802_1q_setting_tlv\n");
				goto fail;
			}
			integrity |= BIT(3);
			setting.updated = 1;
		} else if (*type == TRAFFIC_SEPARATION_POLICY_TYPE) {
			ret = parse_traffic_separation_policy_tlv(&tpolicy, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "%s error trffic separation policy tlv\n", __func__);
				goto fail;
			}
			integrity |= BIT(4);
			tpolicy.updated = 1;
		} else if (*type == R2_AP_CAPABILITY_TYPE) {
			struct ap_r2_capability r2_cap;

			os_memset(&r2_cap, 0, sizeof(struct ap_r2_capability));
			ret = parse_r2_ap_capability_tlv(&r2_cap, almac, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error, r2 ap capability tlv\n");
				goto fail;
			}

			if (agent_info)
				agent_info->r2_cap = r2_cap;

			integrity |= (1 << 5);
		} else if (*type == VENDOR_SPECIFIC_TLV_TYPE) {
			debug(DEBUG_ERROR, "parse vendor specific tlv\n");
			ret = parse_vs_tlv(&vs_info, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error vs tlv\n");
				goto fail;
			}
		} else if (*type == AP_RADIO_ADVANCED_CAPABILITIES_TYPE) {
			struct ts_cap_db *ts_cap = NULL;

			integrity |= (1 << 6);
			ret = parse_ap_radio_advanced_capability_tlv(ctx, almac, &ts_cap, len, value);
			if (ret < 0) {
				debug(DEBUG_ERROR, "error ap radio advanced capability tlv\n");
				goto fail;
			}
			map_r2_notify_ts_cap(ctx, almac, ts_cap);
		}
#endif /* #ifdef MAP_R2 */
		else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	/*check integrity */
	if (!(integrity & right_integrity)) {
		debug(DEBUG_ERROR, AUTO_CONFIG_PREX "incomplete ap auto config WSC 0x%x 0x%x\n",
		      integrity, right_integrity);
		goto fail;
	}

	if ((integrity&BIT(1)) && (os_memcmp(radio_if, zero_if, ETH_ALEN)))
		os_memcpy(radio_identifier, radio_if, ETH_ALEN);

#ifdef MAP_R2
	if (vs_info.trans_vlan.updated == 1 && vs_info.parse_ts) {
		ctx->map_policy.trans_vlan.num = vs_info.trans_vlan.num;
		debug(DEBUG_ERROR, "update transparent vlan number=%d\n", ctx->map_policy.trans_vlan.num);
		for (i = 0; i < vs_info.trans_vlan.num; i++) {
			ctx->map_policy.trans_vlan.transparent_vids[i] = vs_info.trans_vlan.transparent_vids[i];
			debug(DEBUG_ERROR, "parse transparent vlan id(%d)\n",
				ctx->map_policy.trans_vlan.transparent_vids[i]);
		}
		ctx->map_policy.trans_vlan.updated = 1;
	}
#endif

#ifdef MAP_R2
	if (ctx->role == AGENT) {
		/*if no default 802.1q setting */
		if (!(integrity & BIT(3))) {
			ctx->map_policy.setting.primary_vid = VLAN_N_VID;
			ctx->map_policy.setting.updated = 1;
		} else
			os_memcpy(&ctx->map_policy.setting, &setting, sizeof(struct def_8021q_setting));

		/*if no traffic separation policy tlv */
		if (!(integrity & BIT(4))) {
			delete_exist_traffic_policy(ctx, &ctx->map_policy.tpolicy);
			ctx->map_policy.tpolicy.updated = 1;
		} else {
			/*delete the previously reserved traffic policy */
			delete_exist_traffic_policy(ctx, &ctx->map_policy.tpolicy);
			ctx->map_policy.tpolicy.SSID_num = tpolicy.SSID_num;
			ctx->map_policy.tpolicy.updated = tpolicy.updated;

			tpolicy_db = SLIST_FIRST(&(tpolicy.policy_head));
			while (tpolicy_db) {
				tpolicy_db_nxt = SLIST_NEXT(tpolicy_db, policy_entry);
				SLIST_REMOVE(&(tpolicy.policy_head), tpolicy_db,
							traffic_separation_db, policy_entry);

				SLIST_INSERT_HEAD(&ctx->map_policy.tpolicy.policy_head, tpolicy_db, policy_entry);
				tpolicy_db = tpolicy_db_nxt;
			}
		}
	}
#endif

	if (ctx->role == AGENT) {
		ctx->current_autoconfig_info.config_number = m2_num;
	}

	return 0;

fail:
#ifdef MAP_R2
	tpolicy_db = SLIST_FIRST(&(tpolicy.policy_head));
	while (tpolicy_db) {
		tpolicy_db_nxt = SLIST_NEXT(tpolicy_db, policy_entry);
		SLIST_REMOVE(&(tpolicy.policy_head), tpolicy_db,
			traffic_separation_db, policy_entry);
		os_free(tpolicy_db);
		tpolicy_db = tpolicy_db_nxt;
	}
#endif
	return -1;
}

int parse_client_capability_query_message(struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned char *value = NULL;
	unsigned int tlv_len = 0;
	int ret = 0;
	unsigned int integrity = 0;
	unsigned int right_integrity = 0x1;
	struct client_info client_info = {0};

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

		if (*type == CLIENT_INFO_TYPE) {
			integrity |= (1 << 0);

			ret = parse_client_info_tlv(ctx, client_info.bssid, client_info.sta_mac, len, value);

			if (ret < 0) {
				debug(DEBUG_ERROR, "error client info tlv\n");
				return -1;
			}
		} else if (*type == END_OF_TLV_TYPE) {
			break;
		}

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	/*check integrity */
	if (integrity != right_integrity) {
		debug(DEBUG_ERROR, "incomplete client capablilty query 0x%x 0x%x\n", integrity, right_integrity);
		return -1;
	}

	if (integrity & BIT(0)) {
		os_memcpy(ctx->sinfo.cinfo.bssid, client_info.bssid, ETH_ALEN);
		os_memcpy(ctx->sinfo.cinfo.sta_mac, client_info.sta_mac, ETH_ALEN);
	}
	return 0;
}

int check_recv_mid(struct p1905_managerd_ctx *ctx, unsigned char *salmac, unsigned short mid, unsigned short mtype)
{
	struct topology_device_db *tpdev_db = NULL, *exist_tpdev_db = NULL;
	unsigned short i = 0;

	if ((mtype == TOPOLOGY_RESPONSE) || (mtype == LINK_METRICS_RESPONSE) ||
	    (mtype == AP_AUTOCONFIG_RESPONSE) || (mtype == AP_CAPABILITY_REPORT) ||
	    (mtype == CHANNEL_PREFERENCE_REPORT) || (mtype == CHANNEL_SELECTION_RESPONSE) ||
	    (mtype == CLIENT_CAPABILITY_REPORT) || (mtype == AP_LINK_METRICS_RESPONSE) ||
	    (mtype == ASSOC_STA_LINK_METRICS_RESPONSE) || (mtype == Ack_1905)) {
		return 0;
	}

	if (!SLIST_EMPTY(&ctx->topodev_entry.tpdevdb_head)) {
		SLIST_FOREACH(tpdev_db, &ctx->topodev_entry.tpdevdb_head, tpdev_entry) {
			if (!memcmp(tpdev_db->aldev_mac, salmac, ETH_ALEN)) {
				exist_tpdev_db = tpdev_db;
				break;
			}
		}
	}
	if (!exist_tpdev_db) {
		exist_tpdev_db = (struct topology_device_db *)malloc(sizeof(struct topology_device_db));
		if (exist_tpdev_db == NULL)
			return -1;
		memset(exist_tpdev_db, 0, sizeof(struct topology_device_db));
		memcpy(exist_tpdev_db->aldev_mac, salmac, ETH_ALEN);
		exist_tpdev_db->time_to_live = TOPOLOGY_DEVICE_TTL;
		exist_tpdev_db->mid_list[0] = mid;
		exist_tpdev_db->mid_set = 0;
		exist_tpdev_db->recv_mid = mid;
		debug(DEBUG_TRACE, "first time recv mid(%u) aldev_mac(" MACSTR ") msgtype=0x%02x\n",
		      mid, PRINT_MAC(salmac), mtype);
		SLIST_INSERT_HEAD(&ctx->topodev_entry.tpdevdb_head, exist_tpdev_db, tpdev_entry);
	} else {
		for (i = 0; i < MAX_MID_QUEUE; i++) {
			if (mid == exist_tpdev_db->mid_list[i]) {
				debug(DEBUG_ERROR, "drop mid(%04x) aldev_mac(" MACSTR ") msgtype=0x%04x\n",
				      mid, PRINT_MAC(salmac), mtype);
				return -1;
			}
		}
		exist_tpdev_db->time_to_live = TOPOLOGY_DEVICE_TTL;
		exist_tpdev_db->recv_mid = mid;
		exist_tpdev_db->mid_set++;
		if (exist_tpdev_db->mid_set == MAX_MID_QUEUE)
			exist_tpdev_db->mid_set = 0;
		exist_tpdev_db->mid_list[exist_tpdev_db->mid_set] = mid;
	}

	return 0;
}

void relay_fragment_send(struct p1905_managerd_ctx *ctx,
			 unsigned char *tlv, unsigned int tlv_len, unsigned char *eth_hdr, unsigned char *almac)
{
	unsigned short msg_len = 0;
	unsigned char *msg = NULL;
	unsigned int i = 0;

	msg_len = ETH_HLEN + CMDU_HLEN + tlv_len;
	msg = (unsigned char *)malloc(msg_len);
	if (!msg) {
		debug(DEBUG_ERROR, "allco msg fail\n");
		return;
	}

	memcpy(msg, eth_hdr, (ETH_HLEN + CMDU_HLEN));
	memcpy((msg + ETH_ALEN), almac, ETH_ALEN);
	memcpy((msg + ETH_ALEN + CMDU_HLEN), tlv, tlv_len);

	for (i = 0; i < ctx->itf_number; i++) {
		/*do not relay to original receive interface */
		if (i != ctx->recent_cmdu_rx_if_number) {
			if (tlv_len > MAX_TLVS_LENGTH) {
				if (0 > cmdu_tx_fragment(ctx, msg, msg_len, ctx->itf[i].if_name)) {
					debug(DEBUG_ERROR, "fragment send failed on %s (%s)",
					      ctx->itf[i].if_name, strerror(errno));
					free(msg);
					return;
				}
			}
		}
	}

	free(msg);
}

#ifdef MAP_R3
int cmdu_r3_handle(struct p1905_managerd_ctx *ctx,
	unsigned char *dmac, unsigned char *smac,
	unsigned char *cmdu_hdr, unsigned short *cmdu_len, unsigned short mtype,
	unsigned char *vsinfo, unsigned short *vsinfo_len,
	unsigned char *tlv_buf, int tlv_total_len,
	unsigned char *restore_flag)
{
	unsigned char code = SUC;
	struct security_entry *entry = NULL;
	unsigned char *tlv_first = NULL, *pos_mic = NULL, *pos_vs = NULL;
	unsigned char mtk_oui[3] = { 0x00, 0x0C, 0xE7 };
	unsigned char *tlv_pos = NULL;
	unsigned short tlv_len = 0;
	struct r3_member *peer = NULL;

	peer = create_r3_member(smac);
	if (!peer)
		return -1;

	/*
	 * unicast: encrypt this frame only if both R3 device
	 * multicast: i'm R3, should encrypt the frame
	 */
	if (ctx->map_version == MAP_PROFILE_R3) {
		/* all unicast should be encrypted except reliable unicast */
		tlv_pos = tlv_first = tlv_buf;

		if (mtype == TOPOLOGY_DISCOVERY) {
			pos_mic = get_tlv_pos(tlv_first, MIC_TLV_TYPE, tlv_total_len);
			if (pos_mic) {
				while (1) {
					tlv_len = be_to_host16(*(unsigned short *)(tlv_pos + 1)) + 3;
					/* there maybe more than 1 vs info */
					if (*tlv_pos == VENDOR_SPECIFIC_TLV_TYPE) {
						pos_vs = tlv_pos;
						*vsinfo_len = tlv_len;
						if (*vsinfo_len > 16)
							*vsinfo_len = 16;

						if (!os_memcmp(&pos_vs[3], mtk_oui, 3) && (pos_vs[6] == 0x00)) {
							tlv_total_len -= *vsinfo_len;
							os_memcpy(vsinfo, pos_vs, *vsinfo_len);
							/* strip vs info added in mapfilter */
							os_memset(pos_vs, 0, 3);

							*restore_flag = 1;
						}
					} else if (*tlv_pos == END_OF_TLV_TYPE) {
						break;
					}
					tlv_pos += tlv_len;
				}
			}
		}

		/*
		 * not perform encryption procedure for the following messages
		 * 1905 AP-Autoconfiguration Search
		 * 1905 AP-AutoconfigurationResponse
		 * Direct Encap DPP or 1905 Encap EAPOL
		 */
		if ((mtype == _1905_ENCAP_EAPOL) ||
			(mtype == DIRECT_ENCAP_DPP_MESSAGE) ||
			(mtype == AP_AUTOCONFIG_SEARCH) ||
			(mtype == AP_AUTOCONFIG_RESPONSE))
			return 0;

		if (!os_memcmp(p1905_multicast_address, dmac, ETH_ALEN)) {
			unsigned char relay_ind = 0;
			unsigned char *pos_almac = NULL, *real_almac = NULL;
			unsigned short mid = 0;

			entry = sec_entry_lookup(dmac);

			/* check mid first for 1905 mc pkt */
			relay_ind = get_relay_ind_from_msg_hdr((unsigned char *)cmdu_hdr);
			mid = get_mid_from_msg_hdr((unsigned char *)cmdu_hdr);

			if (relay_ind) {
				pos_almac = get_tlv_pos(tlv_first, AL_MAC_ADDR_TLV_TYPE, tlv_total_len);
				if (*((unsigned short *)(pos_almac+1)) < MAC_ADDR_LEN) {
					debug(DEBUG_ERROR, "almac tlv len illegal, drop\n");
					return -1;
				}

				real_almac = pos_almac+3;
			} else
				real_almac = smac;

			if (check_recv_mid(ctx, real_almac, mid, mtype) < 0) {
				debug(DEBUG_ERROR, "recved illegal (mtype 0x%04x)(mid 0x%04x), drop\n", mtype, mid);
				ctx->need_relay = 0;
				return -1;
			}
		}
		else
			entry = sec_entry_lookup(smac);

		code = sec_decrypt(dmac, smac, cmdu_hdr, cmdu_len);
		if (code != SUC) {
			if ((code == NOT_ENCRYPTED) && (!entry))
				return 0;
			/* no key for R1/R2, and no enc/mic tlv in pkt, skip */
			if ((code == NO_KEY_FOUND) &&
				(!get_tlv_pos(tlv_buf, MIC_TLV_TYPE, tlv_total_len)) &&
				(!get_tlv_pos(tlv_buf, ENCRYPTED_TLV_TYPE, tlv_total_len)))
				return 0;

			if (dmac[0]&0x01)
				return 0;

			peer->dec_fail_cnt++;
			debug(DEBUG_ERROR,
				"(mtype 0x%04x) (error code %d) (almac:"MACSTR")\n",
				mtype, code, MAC2STR(smac));

			if (peer->dec_fail_cnt >= ctx->decrypt_fail_threshold) {
				debug(DEBUG_ERROR, "exceed decryption failure threshold 10\n");
				peer->dec_fail_cnt = 0;
				/* trigger dpp discovery again. */
				if (ctx->role != CONTROLLER) {
					/* send Decrypting failure msg to controller */
					insert_cmdu_txq(ctx->cinfo.almac, ctx->p1905_al_mac_addr,
						e_decryption_failure, ++ctx->mid, ctx->itf[ctx->cinfo.recv_ifid].if_name, 0);

					debug(DEBUG_ERROR,
						"send decryption failure to controller "MACSTR" with almac "MACSTR"\n",
						MAC2STR(ctx->cinfo.almac), MAC2STR(ctx->p1905_al_mac_addr));
					/* go on sending decryption failure msg */
					process_cmdu_txq(ctx, ctx->trx_buf.buf);
				}
				map_dpp_trigger_member_dpp_intro(ctx, peer);
			}
			return -1;
		}

	}
	return 0;
}
#endif

/**
 *  parse cmdu message.
 *
 *
 * \param  ctx  1905.1 managerd context
 * \param  buf  input, cmdu message start address(header + payload)
 * \param  dmac  input, dmac of receive packet
 * \param  smac  input, smac of receive packet
 * \param  len   input, length of received packet
 * \return error code
 */
int parse_cmdu_message(struct p1905_managerd_ctx *ctx, unsigned char *buf,
		       unsigned char *dmac, unsigned char *smac, int len)
{
	unsigned char *temp_buf;
	unsigned char mversion;
	unsigned short mtype;
	unsigned short mid = 0;
	unsigned char fid;
	unsigned char last_fragment_ind;
	unsigned char relay_ind = 0;
	unsigned char al_mac[ETH_ALEN] = { 0 };
	unsigned char itf_mac[ETH_ALEN] = { 0 };
	unsigned char need_query = 0, need_notify = 0, need_discovery = 0, internal_vendor_message = 0;
	int tlv_total_len = 0, temp_tlv_len = 0;
	unsigned short queue_len = 0;
	unsigned char *tlv_start;
	struct p1905_vs_info vs_info;
	unsigned char band = 0, role = 0, service = 0, kibmib = 0;
	unsigned char *cmdu_hdr = NULL;
	unsigned short cmdu_len = 0;
#ifdef MAP_R3
	struct r3_member *peer = NULL;
	struct hash_value_item *hv_item = NULL, *hv_item_nxt = NULL;
	unsigned char vsinfo[16] = { 0 };
	unsigned short vsinfo_len;
	unsigned char restore_flag = 0;
	unsigned char *pos_vs = NULL;
#endif
	unsigned char multicast_address[ETH_ALEN] = { 0 };
	unsigned char profile = MAP_PROFILE_R1;
	unsigned char *if_name = NULL;
	int sanity_check_flag = 0;
	struct agent_radio_info rinfo_tmp[MAX_RADIO_NUM];
	unsigned char radio_cnt = 0;
	unsigned char radio_identifier[ETH_ALEN];

	ctx->need_relay = 0;

	/* minial length check: ETH_HLEN(14) + cmdu_hd(8) + 38 */
	if (len < ETH_HLEN + sizeof(cmdu_message_header) + MIN_TLVS_LENGTH) {
		if (!ctx->MAP_Cer) {
			debug(DEBUG_ERROR, "length %d less than 60 bytes\n", len);
			return -1;
		}
	}

	if_name = ctx->itf[ctx->recent_cmdu_rx_if_number].if_name;

	memcpy(multicast_address, p1905_multicast_address, ETH_ALEN);
	/* get message info from message header */
	temp_buf = buf;
	cmdu_hdr = buf;
	tlv_start = buf + 8;
	mversion = get_mversion_from_msg_hdr(temp_buf);
	mtype = get_mtype_from_msg_hdr(temp_buf);
	mid = get_mid_from_msg_hdr(temp_buf);
	fid = get_fid_from_msg_hdr(temp_buf);
	last_fragment_ind = get_last_fragment_ind_from_msg_hdr(temp_buf);
	relay_ind = get_relay_ind_from_msg_hdr(temp_buf);

	debug(DEBUG_TRACE, "mtype(0x%04x) mid(0x%04x) fid(%d) lf_ind(%d)\n", mtype, mid, fid, last_fragment_ind);

	/* deal with message version, it must be 0x00 */
	if (mversion != MESSAGE_VERSION)
		return -1;

	/* check msg type: avoid cache invalid data to fragment buffer */
	if (!((mtype <= ICHANNEL_SELECTION_REQUEST) || (mtype >= Ack_1905 && mtype <= AGENT_LIST_MESSAGE))) {
		debug(DEBUG_ERROR, "mtype 0x%04x not support\n", mtype);
		return -1;
	}

	os_memcpy(al_mac, smac, ETH_ALEN);

	/* if relay indicator =1 but this is not relay multicast message, ignore */
	if (check_relay_message(relay_ind, mtype) < 0)
		return -1;
	if (relay_ind)
		ctx->need_relay = relay_ind;

	/*
	 * check the fragment relevant field
	 * if fragment, insert to fragment queue and try to reassembly
	 */
	temp_tlv_len = len - ETH_HLEN - sizeof(cmdu_message_header);
	tlv_total_len = temp_tlv_len;
	cmdu_len = (unsigned short)(tlv_total_len + CMDU_HLEN);
	if (last_fragment_ind == 0 || fid != 0) {
		/*
		 * because of receiving fragment cmdu, we need to record the total
		 * tlvs length of message.
		 * we use the received length reported by socket and to minus the
		 * ether header length & cmdu message header length
		 */
		debug(DEBUG_TRACE, "receive a fragment CMDU\n");

		/* store cmdu hdr */
		if (fid == 0) {
			os_memcpy(ctx->reassemble_buf.buf, buf, CMDU_HLEN);
			/* fragment id set to 0 */
			*(ctx->reassemble_buf.buf + 6) &= 0x00;
			/* last frag set to 1 */
			*(ctx->reassemble_buf.buf + 7) |= 0x80;
		}
		insert_fragment_queue(mtype, mid, fid, last_fragment_ind, temp_tlv_len, tlv_start);

		queue_len = reassembly_fragment_queue(ctx, mtype, mid);

		/* do not parse this message unless each fragment was received */
		if (queue_len == 0) {
			/* relay the whole packets instesd of fragments to avoid dual backhaul looping */
			if (relay_ind)
				ctx->need_relay = 0;
			return 0;
		}
		/* do relay in msg handle */
		ctx->need_relay = 0;

		tlv_total_len = queue_len;
		cmdu_len = (unsigned short)(tlv_total_len + CMDU_HLEN);
		cmdu_hdr = ctx->reassemble_buf.buf;
	}

	/*
	 * because relay multicast frame will change SA to TA ensure send by neighbor
	 * so its mid need check later
	 * in order to keep reliable for relay multicast message, these message will be
	 * transfered to unicast and sent again. its relay bit will be set to zero.
	 * for dual backhaul link, discovery message cannot drop so no need check mid
	 */
	if (relay_ind == 0 && mtype != TOPOLOGY_DISCOVERY &&
	    mtype != TOPOLOGY_NOTIFICATION && mtype != AP_AUTOCONFIG_SEARCH && mtype != AP_AUTOCONFIG_RENEW) {
		if (0 > check_recv_mid(ctx, al_mac, mid, mtype))
			return -1;
	}

	/*
	 * shift to payload, if queue_len > 0 ==> get message tlv from fragment
	 * queue. otherwise, get payload from original allocated buffer
	 */
	if (queue_len > 0)
		temp_buf = ctx->reassemble_buf.buf + CMDU_HLEN;
	else
		temp_buf = tlv_start;

#ifdef MAP_R3
	if (cmdu_r3_handle(ctx, dmac, smac, cmdu_hdr, &cmdu_len, mtype,
		vsinfo, &vsinfo_len, temp_buf, tlv_total_len, &restore_flag))
		return -1;
#endif /* MAP_R3 */

	temp_buf = cmdu_hdr + CMDU_HLEN;
	tlv_total_len = cmdu_len - CMDU_HLEN;

	/* check if receive 1905_ack for those message which need receive */
	if (mtype == Ack_1905)
		delete_retry_message_queue(al_mac, mid, if_name);

	reset_send_tlv(ctx);
#ifdef MAP_R3
	if (ctx->MAP_Cer && (MAP_PROFILE_R3 == ctx->map_version))
		dev_buffer_frame(ctx, mtype, mid, temp_buf, tlv_total_len);
#endif

	sanity_check_flag = cmdu_sanity_check(temp_buf, tlv_total_len);
	if (-1 > sanity_check_flag) {
		debug(DEBUG_ERROR,
		      "Sanity Check Fail! Err Flag:(%d), smac:(" MACSTR
		      "), mtype:(0x%04x), mid:(0x%04x), tlv_total_len:(0x%04x)\n", sanity_check_flag, PRINT_MAC(smac),
		      mtype, mid, tlv_total_len);
		return -1;
	}

#ifdef MAP_R3
	/* restore vs info added in mapfilter */
	if (restore_flag) {
		pos_vs = get_tlv_pos(temp_buf, END_OF_TLV_TYPE, cmdu_len - CMDU_HLEN);
		if (pos_vs) {
			/* vs info */
			os_memcpy(pos_vs, vsinfo, vsinfo_len);
			/* end of tlv */
			os_memset(pos_vs + vsinfo_len, 0, 3);

			tlv_total_len += vsinfo_len;
		}
	}
#endif /* MAP_R3 */

	switch (mtype) {
	case TOPOLOGY_DISCOVERY:
		/*
		 * need to do ==> check al mac addr tlv
		 * & MID to know whether this message is duplicated
		 */
		if (parse_topology_discovery_message(ctx, temp_buf, tlv_total_len,
						     &need_query, &need_notify, &need_discovery,
						     al_mac, itf_mac, smac) < 0) {
			debug(DEBUG_ERROR, TOPO_PREX "parse_topology_discovery_message fail\n");
			break;
		}

		if (need_discovery) {
			insert_cmdu_txq((unsigned char *)p1905_multicast_address,
					ctx->p1905_al_mac_addr, e_topology_discovery, ++ctx->mid, if_name, 0);
			debug(DEBUG_ERROR, TOPO_PREX "tx discovery(mid-%04x) on intf(%s) to " MACSTR "\n",
			      ctx->mid, if_name, PRINT_MAC(ctx->p1905_al_mac_addr));
		}
		topology_discovery_action(ctx, al_mac, need_query, need_notify);
		update_topology_tree(ctx);
		check_neighbor_discovery(ctx, al_mac, itf_mac);
		break;
	case TOPOLOGY_NOTIFICATION:
		{
			unsigned char sta_mac[ETH_ALEN] = { 0 };
			unsigned char neth_almac[ETH_ALEN] = { 0 };
			unsigned char bssid[ETH_ALEN] = { 0 };
			unsigned char event = 0;
			unsigned char zero_mac[ETH_ALEN] = { 0 };

			if (parse_topology_notification_message(ctx, temp_buf, tlv_total_len, al_mac,
								bssid, sta_mac, &event, neth_almac) < 0) {
				ctx->need_relay = 0;
				break;
			}
			/* need check mid for SA */
			if (check_recv_mid(ctx, al_mac, mid, mtype) < 0) {
				ctx->need_relay = 0;
				break;
			}
			_1905_notify_topology_notification_event(ctx, al_mac);
			/* send a topology query after receiving the topology notification message */
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_topology_query, ++ctx->mid, if_name, 0);

			debug(DEBUG_ERROR, TOPO_PREX "tx query(mid-%04x) on intf(%s) to " MACSTR " for notification\n",
			      ctx->mid, if_name, PRINT_MAC(al_mac));
			if (os_memcmp(neth_almac, zero_mac, ETH_ALEN))
				detect_neighbor_existence(ctx, neth_almac, NULL);
			else if (os_memcmp(sta_mac, zero_mac, ETH_ALEN) &&
				 os_memcmp(bssid, zero_mac, ETH_ALEN) && event)
				detect_neighbor_existence(ctx, NULL, sta_mac);
		}
		break;
	case TOPOLOGY_QUERY:
		insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_topology_response, mid, if_name, 0);
		debug(DEBUG_TRACE, TOPO_PREX "tx response(mid-%04x) on intf(%s) to " MACSTR " for query\n",
		      mid, if_name, PRINT_MAC(al_mac));
#ifdef MAP_R2
		if (parse_topology_query_message(ctx, temp_buf, tlv_total_len, al_mac) < 0) {
			debug(DEBUG_ERROR, "receive error topology response message\n");
			break;
		}
#endif
		break;
	case TOPOLOGY_RESPONSE:
		if (parse_topology_response_message(ctx, temp_buf, tlv_total_len, al_mac, &profile) < 0) {
			debug(DEBUG_ERROR, TOPO_PREX "receive error topology response message\n");
			break;
		}
#ifdef MAP_R3
		if (NULL == (peer = get_r3_member(al_mac))) {
			create_r3_member(al_mac);
		}
		if (peer) {
			peer->profile = profile;
		}
#endif
		debug(DEBUG_TRACE, TOPO_PREX "recv topology response(mid-%04x) on intf(%s)\n", mid,
		      ctx->itf[ctx->recent_cmdu_rx_if_number].if_name);
		_1905_notify_topology_rsp_event(ctx, al_mac, ctx->itf[ctx->recent_cmdu_rx_if_number].mac_addr);
		query_neighbor_info(ctx, al_mac, ctx->recent_cmdu_rx_if_number);
		update_topology_tree(ctx);
		break;
	case VENDOR_SPECIFIC:
		if (parse_vendor_specific_message(ctx, temp_buf, tlv_total_len, &vs_info, &internal_vendor_message) < 0) {
			debug(DEBUG_TRACE, "receive other vendor's vendor specific message\n");
			break;
		}
		/* some vendor message is internal used by 1905daemon, so do not notify it to upper layer */
		if (!internal_vendor_message) {
			_1905_notify_vendor_specific_message(ctx, al_mac);
		} else {
			debug(DEBUG_OFF, "handle 1905 internal using vendor specific message\n");
			parse_1905_internal_vendor_specific_message(ctx, temp_buf, tlv_total_len);
		}

		if (relay_ind && queue_len) {
			relay_fragment_send(ctx, ctx->reassemble_buf.buf,
				queue_len, (buf - ETH_HLEN), ctx->p1905_al_mac_addr);
		}
		break;
	case LINK_METRICS_QUERY:
		if (parse_link_metric_query_message(ctx, temp_buf, tlv_total_len) < 0) {
			debug(DEBUG_ERROR, "receive error LINK_METRICS_QUERY message\n");
			break;
		}

		if (!store_dm_buffer(ctx, al_mac, WAPP_LINK_METRICS_RSP_INFO, mid, (char *)if_name))
			_1905_notify_link_metrics_query(ctx, al_mac, mid);

		break;
	case AP_AUTOCONFIG_SEARCH:
		if (parse_ap_autoconfig_search_message(ctx, temp_buf, tlv_total_len, al_mac, &profile) < 0) {
			debug(DEBUG_ERROR, "no need to response this autoconfig search message\n");
			ctx->need_relay = 0;
			break;
		}

		if (0 > check_recv_mid(ctx, al_mac, mid, mtype)) {
			ctx->need_relay = 0;
			break;
		}
		if (cont_handle_autoconfig_search(ctx, al_mac, if_name, profile, mid) < 0) {
			debug(DEBUG_ERROR,
			      AUTO_CONFIG_PREX "fail to continue to handle auto config search (" MACSTR ")\n",
			      PRINT_MAC(al_mac));
			break;
		}
		break;
	case AP_AUTOCONFIG_RESPONSE:
		if (ctx->autoconfig_search_mid != mid) {
			debug(DEBUG_ERROR, "mid mismatch\n");
			break;
		}

		if (parse_ap_autoconfig_response_message
		    (ctx, temp_buf, tlv_total_len, &band, &service, &role, al_mac, &profile, &kibmib) < 0) {
			debug(DEBUG_ERROR, "no need to response this autoconfig response message\n");
			break;
		}

		if (cont_handle_autoconfig_response(ctx, al_mac, if_name, band, role, profile, kibmib) < 0) {
			debug(DEBUG_ERROR,
			      AUTO_CONFIG_PREX "fail to continue to handle auto config response (" MACSTR ")\n",
			      PRINT_MAC(al_mac));
			break;
		}
		break;
	case AP_AUTOCONFIG_RENEW:
#ifdef MAP_R3
		peer = get_r3_member(al_mac);
		if (peer && peer->security_1905) {
			debug(DEBUG_ERROR, "R3 onboarding, skip autoconfig renew\n");
			break;
		}
#endif
		if (parse_ap_autoconfig_renew_message(ctx, temp_buf, tlv_total_len, al_mac, &band) < 0) {
			debug(DEBUG_ERROR, "no need to response this autoconfig renew message\n");
			ctx->need_relay = 0;
			break;
		}

		if (check_recv_mid(ctx, al_mac, mid, mtype) < 0) {
			ctx->need_relay = 0;
			break;
		}

		if (cont_handle_autoconfig_renew(ctx, al_mac, if_name, band) < 0) {
			debug(DEBUG_ERROR,
			      AUTO_CONFIG_PREX "fail to continue to handle auto config renew (" MACSTR ")\n",
			      PRINT_MAC(al_mac));
			break;
		}
		break;
	case AP_AUTOCONFIG_WSC:
		if (ctx->role == AGENT) {
			if (ctx->enrolle_state != wait_4_recv_m2) {
				debug(DEBUG_ERROR,
				      AUTO_CONFIG_PREX "wrong state(%d) drop WSC M2(mid-%04x) from (" MACSTR ")\n",
				      ctx->enrolle_state, mid, PRINT_MAC(al_mac));
				break;
			}
		}

		if (parse_ap_autoconfig_wsc_message
		    (ctx, al_mac, temp_buf, tlv_total_len, rinfo_tmp, &radio_cnt, radio_identifier) < 0) {
			debug(DEBUG_ERROR,
			      AUTO_CONFIG_PREX "receive error AP_AUTOCONFIG_WSC(mid-%04x) message from (" MACSTR ")\n",
			      mid, PRINT_MAC(al_mac));
			break;
		}

		if (cont_handle_autoconfig_wsc(ctx, al_mac, if_name, mid, rinfo_tmp, radio_cnt, radio_identifier) < 0) {
			debug(DEBUG_ERROR, AUTO_CONFIG_PREX "fail to continue to handle auto config wsc (" MACSTR ")\n",
			      PRINT_MAC(al_mac));
			break;
		}
		break;
	case Ack_1905:
		if (parse_1905_ack_message(ctx, temp_buf, tlv_total_len) < 0) {
			debug(DEBUG_ERROR, "receive error Ack_1905 message\n");
			break;
		}
		if (ctx->revd_tlv.buf_len > 0)
			_1905_notify_error_code_event(ctx, al_mac);

		if (cont_handle_autoconfig_wsc_ack(ctx, al_mac, mid) < 0)
			debug(DEBUG_ERROR, "cont_handle_autoconfig_wsc_ack failed\n");

		break;
	case AP_CAPABILITY_QUERY:
		debug(DEBUG_TRACE, "send ap_capability_report to " MACSTR "\n", PRINT_MAC(al_mac));
		insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_ap_capability_report, mid, if_name, 0);
		break;
	case AP_CAPABILITY_REPORT:
		debug(DEBUG_TRACE, "receive ap_capability_report from " MACSTR "\n", PRINT_MAC(al_mac));
		if (parse_ap_capability_report_message(ctx, al_mac, temp_buf, tlv_total_len, rinfo_tmp, &radio_cnt) < 0) {
			debug(DEBUG_ERROR, "receive error ap capability report message\n");
			break;
		}
		_1905_notify_ap_capability_report_event(ctx, al_mac);
		if (cont_handle_agent_ap_radio_basic_capability(ctx, al_mac, rinfo_tmp, radio_cnt) < 0)
			break;
		break;
	case CLIENT_CAPABILITY_QUERY:
		if (parse_client_capability_query_message(ctx, temp_buf, tlv_total_len) < 0) {
			debug(DEBUG_ERROR, "receive error client capability query message, no need response\n");
			break;
		}
		debug(DEBUG_TRACE, "send client_capability_report to " MACSTR "\n", PRINT_MAC(al_mac));
		insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_cli_capability_report, mid, if_name, 0);
		break;
	case CLIENT_CAPABILITY_REPORT:
		debug(DEBUG_TRACE, "receive client capability report from " MACSTR "\n", PRINT_MAC(al_mac));
		if (parse_client_capability_report_message(ctx, temp_buf, tlv_total_len) < 0) {
			debug(DEBUG_ERROR, "receive error client capability report message\n");
			break;
		}
		_1905_notify_client_capability_report_event(ctx, al_mac);
		break;
	case CHANNEL_PREFERENCE_QUERY:
		reset_stored_tlv(ctx);
		_1905_notify_channel_preference_query(ctx, al_mac, mid);

		break;
	case CHANNEL_SELECTION_REQUEST:
		/* parse the selection request message */
		if (parse_channel_selection_request_message(ctx, temp_buf, tlv_total_len) < 0) {
			debug(DEBUG_ERROR, "error! no need to response this channel selection request message\n");
			break;
		}
		_1905_notify_channel_selection_req(ctx, al_mac, mid);
		break;
	case OPERATING_CHANNEL_REPORT:
		debug(DEBUG_TRACE, "receive operating channel report\n");
		debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
		insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

		if (parse_operating_channel_report_message(ctx, temp_buf, tlv_total_len) < 0) {
			debug(DEBUG_ERROR, "receive error operating channel report message\n");
			break;
		}
		_1905_notify_operating_channel_report_event(ctx, al_mac);
		break;
	case e_higher_layer_data:
		debug(DEBUG_TRACE, "receive higher layer data\n");
		debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
		insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

		if (parse_higher_layer_data_message(ctx, temp_buf, tlv_total_len) < 0) {
			debug(DEBUG_ERROR, "receive error high layer data message\n");
			break;
		}
		_1905_notify_higher_layer_data_event(ctx, al_mac);
		break;
	case CLIENT_STEERING_REQUEST:
		{

			struct err_sta_db err_sta;
			struct err_sta_mac_db *err_mac = NULL, *err_mac_next = NULL;
			unsigned char *buf_err = NULL, *pos = NULL;
			int len = 0;

			os_memset(&err_sta, 0, sizeof(err_sta));

			debug(DEBUG_TRACE, "receive client steering request\n");
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

			/* parse the selection request message */
			if (parse_client_steering_request_message(ctx, temp_buf, tlv_total_len, &err_sta) < 0) {
				debug(DEBUG_ERROR, "error! no need to response this client steering request message\n");
				break;
			}

			_1905_set_steering_setting(ctx, al_mac);

			/* prepare for error mac tlv */
			if (err_sta.err_sta_cnt) {
				len = err_sta.err_sta_cnt * 10;
				buf_err = (unsigned char *)os_malloc(len);
				if (!buf_err) {
					debug(DEBUG_ERROR, "alloc err sta len fail\n");
					break;
				}
				pos = buf_err;
				SLIST_FOREACH(err_mac, &err_sta.err_sta_head, err_sta_mac_entry) {
					*pos++ = ERROR_CODE_TYPE;
					*pos++ = 0x00;
					*pos++ = 0x07;
					*pos++ = 0x02;
					os_memcpy(pos, err_mac->mac_addr, ETH_ALEN);
					pos += ETH_ALEN;
				}
				err_mac = SLIST_FIRST(&err_sta.err_sta_head);
				while (err_mac) {
					err_mac_next = SLIST_NEXT(err_mac, err_sta_mac_entry);
					SLIST_REMOVE(&err_sta.err_sta_head, err_mac, err_sta_mac_db, err_sta_mac_entry);
					os_free(err_mac);
					err_mac = err_mac_next;
				}
				reset_send_tlv(ctx);
				if (fill_send_tlv(ctx, buf_err, len) < 0)
					debug(DEBUG_ERROR, "fill_send_tlv fail\n");
				os_free(buf_err);
			}
		}
		break;
	case MAP_POLICY_CONFIG_REQUEST:
		{
			if (ctx->role == AGENT) {
				unsigned char old_report_interval = ctx->map_policy.mpolicy.report_interval;

				debug(DEBUG_TRACE, "receive map policy config request\n");
				debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
				insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

				/* parse the map policy config request message */
				if (parse_map_policy_config_request_message(ctx, temp_buf, tlv_total_len) < 0) {
					debug(DEBUG_ERROR,
					      "error! no need to response this map policy config request message\n");
					break;
				}

				_1905_set_map_policy_config(ctx, al_mac);

				if (ctx->map_policy.mpolicy.report_interval == 0) {
					eloop_cancel_timeout(metrics_report_timeout, (void *)ctx, NULL);
				} else if (old_report_interval != ctx->map_policy.mpolicy.report_interval) {
					eloop_cancel_timeout(metrics_report_timeout, (void *)ctx, NULL);
					eloop_register_timeout(0, 0,
							       metrics_report_timeout, (void *)ctx, NULL);
				}
#ifdef MAP_R2
				/* Only update traffic separation policy in Certification mode */
				if (ctx->MAP_Cer) {
					eloop_register_timeout(4, 0, map_r2_notify_ts_config, (void *)ctx, NULL);
					eloop_register_timeout(0, 0, map_notify_transparent_vlan_setting, (void *)ctx,
							       NULL);
				}
#endif
			}
		}
		break;
	case CLIENT_ASSOC_CONTROL_REQUEST:
		{
			unsigned char ack_buf[560] = { 0 };

			debug(DEBUG_TRACE, "receive client assoc control request\n");

			delete_exist_steer_cntrl_db(ctx);
			/* parse the client association control request message */
			if (parse_cli_assoc_control_request_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR,
				      "error! no need to response this client association control request message\n");
				break;
			}

			/* check if need add error code tlv in 1905 ack message */
			check_add_error_code_tlv(ctx, ack_buf, 50, CLIENT_ASSOC_CONTROL_REQUEST);
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));

			_1905_set_client_assoc_ctrl(ctx, al_mac);
		}
		break;
		/* link metric collection */
	case AP_LINK_METRICS_QUERY:
		{
			/* parse AP_LINK_METRICS_QUERY msg */
			if (parse_ap_metrics_query_message(ctx, temp_buf, tlv_total_len, radio_identifier) < 0) {
				debug(DEBUG_ERROR, "error! no need to response this link metrics query message\n");
				break;
			}

			_1905_notify_ap_metrics_query(ctx, al_mac, mid
#ifdef MAP_R2
				, 0
#endif
			);
		}
		break;
	case AP_LINK_METRICS_RESPONSE:
		{
			if (ctx->role == CONTROLLER) {
				/* parse ap metrics response message for the agent */
				if (parse_ap_metrics_response_message(ctx, temp_buf, tlv_total_len) < 0)
					debug(DEBUG_ERROR, "error! parse ap metrics response message\n");
				_1905_notify_ap_metrics_response(ctx, al_mac);
			}
		}
		break;
	case LINK_METRICS_RESPONSE:
		{
			if (ctx->role == CONTROLLER) {
				debug(DEBUG_TRACE, "got LINK_METRICS_RESPONSE\n");
				/* parse ap metrics response message for the agent */
				if (parse_link_metrics_response_message(ctx, temp_buf, tlv_total_len) < 0)
					debug(DEBUG_ERROR, "error! parse link metrics response message\n");
				_1905_notify_link_metrics_response(ctx, al_mac);
			}
		}
		break;
	case ASSOC_STA_LINK_METRICS_QUERY:
		{
			/* parse AP_LINK_METRICS_QUERY msg */
			if (parse_associated_sta_link_metrics_query_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR,
				      "error! no need to response this associated sta link metrics query message\n");
				break;
			}

			_1905_notify_assoc_sta_link_metrics_query(ctx, al_mac, mid);
		}
		break;
	case ASSOC_STA_LINK_METRICS_RESPONSE:
		{
			if (parse_associated_sta_link_metrics_rsp_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "parse associated sta link metrics query message error\n");
				break;
			}
			_1905_notify_assoc_sta_link_metric_rsp(ctx, al_mac);
		}
		break;
	case UNASSOC_STA_LINK_METRICS_QUERY:
		{
			unsigned char ack_buf[1514] = { 0 };

			debug(DEBUG_TRACE, "receive unassoc sta link metrics query\n");

			free(ctx->metric_entry.unlink_query);
			ctx->metric_entry.unlink_query = NULL;
			if (parse_unassociated_sta_link_metrics_query_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR,
				      "error! no need to response this unassociated sta link metrics query message\n");
				break;
			}

			/* check if need add error code tlv in 1905 ack message */
			check_add_error_code_tlv(ctx, ack_buf, 50, UNASSOC_STA_LINK_METRICS_QUERY);
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
			process_cmdu_txq(ctx, ack_buf);

			if (!store_dm_buffer(ctx, al_mac, LIB_UNASSOC_STA_LINK_METRICS,
				mid, (char *)ctx->itf[ctx->recent_cmdu_rx_if_number].if_name))
				_1905_notify_unassoc_sta_metrics_query(ctx, al_mac);
		}
		break;
	case UNASSOC_STA_LINK_METRICS_RESPONSE:
		{
			debug(DEBUG_TRACE, "receive unassoc sta link metrics response\n");
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

			if (parse_unassoc_sta_link_metrics_response_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! unassoc sta link metrics response message\n");
				break;
			}

			_1905_notify_unassoc_sta_link_metric_rsp(ctx, al_mac);
		}
		break;
	case BEACON_METRICS_QUERY:
		{
			unsigned char ack_buf[128] = { 0 };

			debug(DEBUG_TRACE, "receive beacon metrics query\n");

			free(ctx->metric_entry.bcn_query);
			ctx->metric_entry.bcn_query = NULL;

			/* parse AP_LINK_METRICS_QUERY msg */
			if (parse_beacon_metrics_query_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! no need to response this beacon metrics query message\n");
				break;
			}

			/* check if need add error code tlv in 1905 ack message */
			check_add_error_code_tlv(ctx, ack_buf, 1, BEACON_METRICS_QUERY);
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));

			_1905_notify_beacon_metrics_query(ctx, al_mac);
		}
		break;
	case BEACON_METRICS_RESPONSE:
		{
			debug(DEBUG_TRACE, "receive beacon metrics response\n");
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

			if (parse_beacon_metrics_response_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! beacon metrics response message\n");
				break;
			}
			_1905_notify_bcn_metric_rsp(ctx, al_mac);
		}
		break;
	case CHANNEL_PREFERENCE_REPORT:
		{
			struct agent_list_db *agent_info = NULL;

			if (ctx->role == CONTROLLER) {
				debug(DEBUG_TRACE, "receive channel preference report\n");
				debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
				insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

				if (ctx->MAP_Cer) {
					find_agent_info(ctx, smac, &agent_info);
					if (!agent_info) {
						debug(DEBUG_ERROR, "error! no agent info exist\n");
						break;
					}
					/* delete all the stored channel preference information for the agent */
					delete_agent_ch_prefer_info(ctx, agent_info);
				}

				/* parse the channel preference report message for the agent */
				if (parse_channel_preference_report_message(ctx, agent_info, temp_buf, tlv_total_len) <
				    0) {
					debug(DEBUG_ERROR, "error! parse channel preference report message\n");
					break;
				}
				_1905_notify_channel_preference_report(ctx, al_mac);
			}
		}
		break;
	case CHANNEL_SELECTION_RESPONSE:
		{
			if (ctx->role == CONTROLLER) {
				debug(DEBUG_TRACE, "receive channel selection response\n");
				debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
				insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);
				if (parse_channel_selection_rsp_message(ctx, temp_buf, tlv_total_len) < 0) {
					debug(DEBUG_TRACE, "error! parse channel selection response message\n");
					break;
				}
				_1905_notify_ch_selection_rsp_event(ctx, al_mac);
			}
		}
		break;
	case CLIENT_STEERING_BTM_REPORT:
		{
			if (ctx->role == CONTROLLER) {
				debug(DEBUG_TRACE, "receive client steering btm report\n");
				debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
				insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

				if (parse_client_steering_btm_report_message(ctx, temp_buf, tlv_total_len) < 0) {
					debug(DEBUG_ERROR, "error! parse client steering btm report message\n");
					break;
				}

				_1905_notify_client_steering_btm_report(ctx, al_mac);
			}
		}
		break;
	case CLIENT_STEERING_COMPLETED:
		{
			if (ctx->role == CONTROLLER) {
				debug(DEBUG_TRACE, "receive client steering completed\n");
				debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
				insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);
				reset_stored_tlv(ctx);
				_1905_notify_steering_complete(ctx, al_mac);
			}

		}
		break;
	case BACKHAUL_STEERING_RESPONSE:
		{
			if (ctx->role == CONTROLLER) {
				debug(DEBUG_TRACE, "receive backhaul steering response\n");
				debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
				insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

				if (parse_backhaul_steering_rsp_message(ctx, temp_buf, tlv_total_len) < 0) {
					debug(DEBUG_ERROR, "error! parse backhual steering response message\n");
					break;
				}
				_1905_notify_bh_steering_rsp(ctx, al_mac);
			}
		}
		break;
		/* Backhaul optimization */
	case BACKHAUL_STEERING_REQUEST:
		{
			debug(DEBUG_TRACE, "receive backhaul steering request\n");
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

			/* parse BACKHAUL_STEERING_REQUEST msg */
			if (parse_backhaul_steering_request_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR,
				      "error! no need to response this backhaul steering request message\n");
				break;
			}
			_1905_notify_bh_steering_req(ctx, al_mac);
		}
		break;
	case COMBINED_INFRASTRUCTURE_METRICS:
		{
			debug(DEBUG_TRACE, "receive combined infrastructure metrics\n");
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

			if (parse_combined_infra_metrics_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR,
				      "error! no need to response this backhaul steering request message\n");
				break;
			}
			_1905_notify_combined_infra_metrics(ctx, al_mac);
		}
		break;
#ifdef MAP_R2
		/* channel scan feature */
	case CHANNEL_SCAN_REQUEST:
		{
			debug(DEBUG_TRACE, "receive channel scan request message\n");
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

			/* parse BACKHAUL_STEERING_REQUEST msg */
			if (parse_channel_scan_request_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! no need to response this channel scan request message\n");
				break;
			}

			_1905_notify_ch_scan_req(ctx, al_mac);
		}
		break;
	case CHANNEL_SCAN_REPORT:
		{
			debug(DEBUG_TRACE, "receive channel scan report message\n");
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

			/* parse BACKHAUL_STEERING_REQUEST msg */
			if (parse_channel_scan_report_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! no need to response this channel scan report message\n");
				break;
			}

			_1905_notify_ch_scan_rep(ctx, al_mac);
		}
		break;

	case TUNNELED_MESSAGE:
		{
			debug(DEBUG_TRACE, "receive TUNNELED_MESSAGE\n");
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

			/* parse tunneled msg */
			if (parse_tunneled_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! no need to send tunneled msg to mapd\n");
				break;
			}

			_1905_notify_tunneled_msg(ctx, al_mac);
		}
		break;
	case ASSOCIATION_STATUS_NOTIFICATION:
		{
			debug(DEBUG_TRACE, "receive ASSOCIATION_STATUS_NOTIFICATION\n");
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

			/* parse assoc status notification msg */
			if (parse_assoc_status_notification_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! no need to send assoc status notification msg to mapd\n");
				break;
			}

			_1905_notify_assoc_status_notification_event(ctx, al_mac);

		}
		break;
	case CAC_REQUEST:
		{
			debug(DEBUG_TRACE, "receive CAC_REQUEST\n");
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

			/* parse assoc status notification msg */
			if (parse_cac_request_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! no need to send cac request msg to mapd\n");
				break;
			}

			_1905_notify_cac_request_event(ctx, al_mac);

		}
		break;

	case CAC_TERMINATION:
		{
			debug(DEBUG_TRACE, "receive CAC_TERMINATION\n");
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

			/* parse assoc status notification msg */
			if (parse_cac_terminate_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! no need to send cac terminate msg to mapd\n");
				break;
			}

			_1905_notify_cac_terminate_event(ctx, al_mac);

		}
		break;

	case CLIENT_DISASSOCIATION_STATS:
		{
			debug(DEBUG_TRACE, "receive CLIENT_DISASSOCIATION_STATS\n");

			hex_dump("CLIENT_DISASSOCIATION_STATS", temp_buf, len - 25);
			/* parse client disassociation stats msg */
			if (parse_client_disassciation_stats_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! no need to send client disassoctiation stats to mapd\n");
				break;
			}

			_1905_notify_client_disassociation_stats_event(ctx, al_mac);
		}
		break;

	case BACKHAUL_STA_CAP_QUERY_MESSAGE:
		{
			debug(DEBUG_TRACE, "receive BACKHAUL_STA_CAP_QUERY_MESSAGE\n");

			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr,
					e_backhual_sta_cap_report_message, mid, if_name, 0);
		}
		break;
	case BACKHAUL_STA_CAP_REPORT_MESSAGE:
		{
			debug(DEBUG_TRACE, "receive BACKHAUL_STA_CAP_REPORT_MESSAGE\n");

			if (parse_backhaul_sta_cap_report_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! no need send backhaul sta cap report event to mapd\n");
				break;
			}

			_1905_notify_backhaul_sta_cap_report_event(ctx, al_mac);
		}
		break;

	case FAILED_CONNECTION_MESSAGE:
		{
			debug(DEBUG_TRACE, "receive FAILED_CONNECTION_MESSAGE\n");
			debug(DEBUG_TRACE, "send 1905 ack to " MACSTR "\n", PRINT_MAC(al_mac));
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr,
					e_1905_ack, mid, ctx->itf[ctx->recent_cmdu_rx_if_number].if_name, 0);

			/* parse assoc status notification msg */
			if (parse_failed_connection_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! no need to send failed connection to mapd\n");
				break;
			}

			_1905_notify_failed_connection_event(ctx, al_mac);
		}
		break;
#endif /* #ifdef MAP_R2 */

#ifdef MAP_R3
	case DIRECT_ENCAP_DPP_MESSAGE:
		{
			unsigned char dpp_frame_type = 0;
			unsigned short dpp_frame_offset = 0;
			unsigned short dpp_frame_len = 0;

			if (ctx->map_version < MAP_PROFILE_R3)
				break;

			if (parse_direct_encap_dpp_message(ctx, temp_buf, tlv_total_len,
				&dpp_frame_type, &dpp_frame_offset,
				&dpp_frame_len, al_mac) < 0) {
				debug(DEBUG_ERROR, "[dpp in 1905]error! fail to parse direct encap dpp msg\n");
				break;
			}

			/* DPP onboarding after R1|R2 onboarding */
			if ((ctx->r3_oboard_ctx.bh_type == MAP_BH_ETH) && (dpp_frame_type == DPP_AUTH_REQUEST)) {
				debug(DEBUG_ERROR,
				      "[dpp in 1905]activate dpp onboarding after receiving DPP Auth Req\n");
				ctx->r3_oboard_ctx.active = 1;
			}

			if (dpp_frame_type != DPP_DISCOVERY_REQUEST && dpp_frame_type != DPP_DISCOVERY_RESPONSE)
				_1905_notify_direct_encap_dpp_event(ctx, al_mac);
		}
		break;

	case PROXIED_ENCAP_DPP_MESSAGE:
		{
			unsigned char dpp_frame_type = 0;
			unsigned short dpp_frame_offset = 0;
			unsigned short dpp_frame_len = 0;

			if (ctx->map_version < MAP_PROFILE_R3)
				break;

			debug(DEBUG_ERROR, "[dpp in 1905]receive PROXIED_ENCAP_DPP_MESSAGE\n");
			/*
			 * If a Multi-AP Controller receives a Proxied Encap DPP Message,
			 * then it shall respond within one second with a 1905 Ack
			 */
			if (ctx->role == CONTROLLER) {
				insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr,
						e_1905_ack, mid, ctx->itf[ctx->recent_cmdu_rx_if_number].if_name, 0);
			}

			/* parse proxied encap dpp msg */
			if (parse_proxied_encap_dpp_message(ctx, temp_buf, tlv_total_len,
				al_mac, &dpp_frame_type, &dpp_frame_offset, &dpp_frame_len) < 0) {
				debug(DEBUG_ERROR, "[dpp in 1905]error! no need to proxid encap dpp msg to mapd\n");
				break;
			}

			if (dpp_frame_type != DPP_DISCOVERY_REQUEST && dpp_frame_type != DPP_DISCOVERY_RESPONSE)
				_1905_notify_proxied_encap_dpp_event(ctx, al_mac);

			if (ctx->r3_oboard_ctx.bh_type == MAP_BH_ETH && dpp_frame_type == DPP_AUTH_REQUEST) {
				debug(DEBUG_ERROR, "[dpp in 1905]ETH BH,cancel timer of dpp auth req\n");
				eloop_cancel_timeout(r3_wait_for_dpp_auth_req, (void *)ctx, NULL);
			}
		}
		break;

	case _1905_ENCAP_EAPOL:
		{
			unsigned char *eapol_frame = NULL;
			unsigned short eapol_offset = 0;
			unsigned short eapol_len = 0;
			int eapol_msg_type = EAPOL_MSG_INVALID;
			struct wpa_entry_info *entry = NULL;

			if (ctx->map_version < MAP_PROFILE_R3)
				break;

			debug(DEBUG_ERROR, "[1905_ENCAP_EAPOL]receive _1905_ENCAP_EAPOL from " MACSTR "\n",
			      PRINT_MAC(al_mac));

			/* parse eapol frame and length */
			if (parse_1905_encap_eapol_message(ctx, temp_buf, tlv_total_len,
				&eapol_offset, &eapol_len) < 0) {
				debug(DEBUG_ERROR, "[1905_ENCAP_EAPOL]parse encap eapol frame error\n");
				break;
			}

			eapol_frame = temp_buf + eapol_offset;
			eapol_msg_type = wpa_receive_from_1905(al_mac, eapol_frame, eapol_len);

			if (ctx->role == AGENT &&
			    ((eapol_msg_type == EAPOL_PAIR_MSG_1) || (eapol_msg_type == EAPOL_PAIR_MSG_2))) {

				if (!os_memcmp(al_mac, ctx->cinfo.almac, ETH_ALEN)) {
					ctx->r3_oboard_ctx.onboarding_stage = R3_ONBOARDING_STAGE_4_WAY_HS;
					debug(DEBUG_ERROR, "[1905_ENCAP_EAPOL] change stage to 4 way hs\n");
				}
			}

			entry = wpa_get_entry(al_mac);
			if (entry && entry->wpa_sm->ptk_sm == wpa_pair_suc) {
				peer = get_r3_member(al_mac);
				if (peer) {
					peer->security_1905 = 1;
					peer->dec_fail_cnt = 0;

					if (os_memcmp(al_mac, ctx->cinfo.almac, ETH_ALEN) == 0) {
						_1905_notify_1905_security(ctx, al_mac, 1);
					}
				}
			}

			/* make sure not trigger dpp peer discovery while GTK Rekey */
			if ((eapol_msg_type >= EAPOL_PAIR_MSG_3) && (eapol_msg_type <= EAPOL_PAIR_MSG_4)) {
				/* agent start bss configuration procedure */
				if (!os_memcmp(ctx->cinfo.almac, al_mac, ETH_ALEN)) {
					entry = wpa_get_entry(al_mac);
					/* check if PTK negotiation finished or not */
					if (entry && entry->wpa_sm->ptk_sm == wpa_pair_suc) {
						debug(DEBUG_ERROR,
						      "[1905_ENCAP_EAPOL] 4 way finished, start bss config procedure\n");
						if (ctx->r3_oboard_ctx.r3_config_state == no_r3_autoconfig
						    || ctx->r3_oboard_ctx.r3_config_state == r3_autoconfig_done) {
							r3_set_config_state(ctx, wait_4_send_bss_autoconfig_req);
							/* wait for 5 secs to start bss configuration */
							eloop_cancel_timeout(r3_config_sm_step, (void *)ctx, NULL);
							eloop_register_timeout(0, 0, r3_config_sm_step, (void *)ctx,
									       NULL);

							ctx->r3_oboard_ctx.onboarding_stage =
							    R3_ONBOARDING_STAGE_BSS_CONFIG;
							debug(DEBUG_ERROR, "[BSS Config] change stage to bss config\n");
						}
					} else {
						debug(DEBUG_ERROR,
						      "[1905_ENCAP_EAPOL] 4 way not finished, can't start bss config procedure\n");
					}
				}
			} else {
				debug(DEBUG_ERROR, "[1905_ENCAP_EAPOL] only start bss config after msg3|msg4\n");
			}
		}
		break;

	case DPP_BOOTSTRAPING_URI_QUERY:
		{
			debug(DEBUG_TRACE, "receive DPP_BOOTSTRAPING_URI_QUERY\n");

			/* if need ack??? */
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr,
					e_1905_ack, mid, ctx->itf[ctx->recent_cmdu_rx_if_number].if_name, 0);

			/* no tlvs in uri query msg, so do not need to parse it */
			_1905_notify_dpp_bootstraping_uri_query_event(ctx, al_mac);
		}
		break;

	case DPP_BOOTSTRAPING_URI_NOTIFICATION:
		{
			debug(DEBUG_TRACE, "receive DPP_BOOTSTRAPING_URI_NOTIFICATION\n");

			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr,
					e_1905_ack, mid, ctx->itf[ctx->recent_cmdu_rx_if_number].if_name, 0);

			/* parse dpp bootstraping uri notification msg */
			if (parse_dpp_bootstraping_uri_notification_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! no need to bootstraping uri notify msg to mapd\n");
				break;
			}

			_1905_notify_dpp_bootstraping_uri_notification_event(ctx, al_mac);
		}
		break;

	case DPP_CCE_INDICATION_MESSAGE:
		{
			debug(DEBUG_TRACE, "receive DPP_CCE_INDICATION_MESSAGE\n");

			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr,
					e_1905_ack, mid, ctx->itf[ctx->recent_cmdu_rx_if_number].if_name, 0);

			/* parse assoc status notification msg */
			if (parse_dpp_cce_indication_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! no need to send cce indication msg to mapd\n");
				break;
			}

			_1905_notify_dpp_cce_indication_event(ctx, al_mac);
		}
		break;

	case CHIRP_NOTIFICATION_MESSAGE:
		{
			debug(DEBUG_TRACE, "receive CHIRP_NOTIFICATION_MESSAGE\n");

			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr,
					e_1905_ack, mid, ctx->itf[ctx->recent_cmdu_rx_if_number].if_name, 0);

			/* parse chirp notification msg */
			if (parse_chirp_notification_message(ctx, temp_buf, tlv_total_len, al_mac) < 0) {
				debug(DEBUG_ERROR, "error! no need to send chirp notification msg to mapd\n");
				break;
			}

			_1905_notify_chirp_notification_event(ctx, al_mac);

		}
		break;

	case _1905_REKEY_REQUEST:
		{
			struct topology_response_db *tpgr_db = NULL;
			unsigned char peer_cnt = 0;

			debug(DEBUG_ERROR, "receive _1905_REKEY_REQUEST from " MACSTR "\n", MAC2STR(al_mac));

			/*
			 * receives a 1905 Rekey Request Message,
			 * respond within one second with a 1905 Ack message
			 */
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr,
					e_1905_ack, mid, ctx->itf[ctx->recent_cmdu_rx_if_number].if_name, 0);

			/* parse 1905 rekey request message */
			if (parse_1905_rekey_request_message(ctx, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "error! no need handle 1905 rekey request\n");
				break;
			}

			/* start 4-way handshake with each peer */
			SLIST_FOREACH(tpgr_db, &(ctx->topology_entry.tprdb_head), tprdb_entry) {
				debug(DEBUG_ERROR, "start timer %d(s) to init 4way with " MACSTR "\n",
				      peer_cnt * 10, MAC2STR(tpgr_db->al_mac_addr));
				eloop_cancel_timeout(wpa_start_4way_handshake, (void *)tpgr_db->al_mac_addr, NULL);
				eloop_register_timeout(peer_cnt * 2, 0, wpa_start_4way_handshake,
						       (void *)tpgr_db->al_mac_addr, NULL);
				peer_cnt++;
			}
		}
		break;

	case _1905_DECRYPTION_FAILURE:
		{
			unsigned char peer_al_mac[ETH_ALEN];

			debug(DEBUG_ERROR, "receive _1905_DECRYPTION_FAILURE from " MACSTR "\n", PRINT_MAC(al_mac));

			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr,
					e_1905_ack, mid, ctx->itf[ctx->recent_cmdu_rx_if_number].if_name, 0);

			/* parse 1905 rekey request message */
			if (parse_1905_decryption_failure_message(ctx, temp_buf, tlv_total_len, peer_al_mac) < 0)
				break;

			peer = get_r3_member(peer_al_mac);
			if (peer) {
				debug(DEBUG_ERROR, "trigger dpp intro with " MACSTR "again\n", PRINT_MAC(peer_al_mac));
				map_dpp_trigger_member_dpp_intro(ctx, peer);
			}
		}
		break;

	case BSS_CONFIGURATION_REQUEST_MESSAGE:
		{
			if (ctx->role != CONTROLLER)
				break;
			debug(DEBUG_TRACE, "receive BSS_CONFIGURATION_REQUEST_MESSAGE from " MACSTR "\n",
			      PRINT_MAC(al_mac));

			/* parse bss configuration request message */
			if (parse_1905_bss_configuration_request_message(ctx, temp_buf, tlv_total_len, al_mac,
									 rinfo_tmp, &radio_cnt) < 0) {
				debug(DEBUG_ERROR, "failed to parse bss config request message\n");
				break;
			}

			if (cont_handle_bss_configuration_request(ctx, al_mac, rinfo_tmp, radio_cnt) < 0)
				break;
		}
		break;

	case BSS_CONFIGURATION_RESPONSE_MESSAGE:
		{
			if (ctx->role != AGENT)
				break;
			ctx->r3_oboard_ctx.bss_renew = 0;
			debug(DEBUG_ERROR, "receive BSS_CONFIGURATION_RESPONSE_MESSAGE from " MACSTR "\n",
			      PRINT_MAC(al_mac));

			if (ctx->r3_oboard_ctx.r3_config_state != wait_4_recv_bss_autoconfig_resp) {
				debug(DEBUG_ERROR, "r3 config state mismatch\n");
				break;
			}
			if (parse_bss_configuration_response_message(ctx, temp_buf, tlv_total_len, al_mac) < 0) {
				debug(DEBUG_ERROR, "failed to parse bss config rsp message\n");
				break;
			}
			eloop_cancel_timeout(r3_config_sm_step, (void *)ctx, NULL);
			r3_set_config_state(ctx, wait_4_set_r3_config);
			eloop_register_timeout(0, 0, r3_config_sm_step, (void *)ctx, NULL);
		}
		break;

	case BSS_CONFIGURATION_RESULT_MESSAGE:
		{
			struct agent_list_db *agent = NULL;

			if (ctx->role != CONTROLLER)
				break;
			find_agent_info(ctx, al_mac, &agent);
			debug(DEBUG_TRACE, "receive BSS_CONFIGURATION_RESULT_MESSAGE from " MACSTR "\n",
			      PRINT_MAC(al_mac));
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_agent_list, mid,
					ctx->itf[ctx->recent_cmdu_rx_if_number].if_name, 0);

			/* all band configured */
			if (agent)
				config_agent_all_radio_config_stat(agent);

			peer = get_r3_member(al_mac);
			if (peer) {
				hex_dump("peer hash", (unsigned char *)peer->chirp.hash_payload,
					 sizeof(hv_item->hashValue));
				/* one chirp value for each agent on controller */
				dl_list_for_each_safe(hv_item, hv_item_nxt,
						      &ctx->r3_dpp.hash_value_list, struct hash_value_item, member) {
					hex_dump("item hash", hv_item->hashValue, sizeof(hv_item->hashValue));
					debug(DEBUG_TRACE, "item almac " MACSTR "\n", PRINT_MAC(hv_item->almac));
					if ((!(os_memcmp(hv_item->almac, al_mac, ETH_ALEN)))
					    && hv_item->hashLen) {
						_1905_notify_onboarding_result(ctx, al_mac, 1, hv_item->hashValue,
									       hv_item->hashLen);
						debug(DEBUG_TRACE, "notify onboarding result to mapd\n");
						break;
					}
				}

				if (peer->security_1905 && (!ctx->MAP_Cer)) {
					debug(DEBUG_TRACE, "trigger GTK REKEY for " MACSTR "\n", PRINT_MAC(al_mac));
					check_trigger_gtk_rekey();
				}
			}
		}
		break;

	case BSS_RECONFIGURATION_TRIGGER_MESSAGE:
		{
			if (ctx->role != AGENT)
				break;
			reset_stored_tlv(ctx);

			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr,
					e_1905_ack, mid, ctx->itf[ctx->recent_cmdu_rx_if_number].if_name, 0);

			if (ctx->r3_oboard_ctx.bss_renew) {
				debug(DEBUG_ERROR, "R3 BSS Config renew is ongoing, skip this one\n");
				break;
			}
			debug(DEBUG_TRACE, "received bss reconfig trigger msg,response with bss config request\n");

			r3_set_config_state(ctx, wait_4_send_bss_autoconfig_req);
			ctx->r3_oboard_ctx.bss_renew = 1;
			/* start bss config sm */
			eloop_cancel_timeout(r3_config_sm_step, (void *)ctx, NULL);
			eloop_register_timeout(0, 0, r3_config_sm_step, (void *)ctx, NULL);
		}
		break;

	case AGENT_LIST_MESSAGE:
		{
			debug(DEBUG_ERROR, "receive AGENT_LIST_MESSAGE from " MACSTR "\n", PRINT_MAC(al_mac));
			if (parse_agent_list_message(temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, "parse agent list message error\n");
				break;
			}
			map_dpp_trigger_all_member_dpp_intro(ctx);
		}
		break;
#endif /* #ifdef MAP_R3 */
#ifdef MAP_R3_SP
	case SERVICE_PRIORITIZATION_REQUEST:
		{
			debug(DEBUG_TRACE, "receive SERVICE_PRIORITIZATION_REQUEST\n");

			if (ctx->role == CONTROLLER) {
				debug(DEBUG_ERROR,
				      SP_PREX "BUG!! controller received SERVICE_PRIORITIZATION_REQUEST\n");
				break;
			}

			if (parse_sp_request_message(&ctx->sp_rule_static_list, &ctx->sp_rule_dynamic_list,
						     &ctx->sp_rule_del_list, &ctx->sp_rule_learn_complete_list,
						     &ctx->sp_dp_table, ctx->p1905_al_mac_addr, temp_buf, tlv_total_len) < 0) {
				debug(DEBUG_ERROR, SP_PREX "failed to parse service prioritization request message\n");
				break;
			}
			insert_cmdu_txq(al_mac, ctx->p1905_al_mac_addr, e_1905_ack, mid, if_name, 0);

			/*
			 * if all rule lists are empty, means controller has removed all sp rules,
			 * so clear sp rule in local mapfilter
			 */
			if (dl_list_len(&ctx->sp_rule_static_list) == 0 && dl_list_len(&ctx->sp_rule_dynamic_list) == 0) {
				if (clear_sp_rules_of_local(&ctx->sp_rule_static_list, &ctx->sp_rule_dynamic_list) < 0) {
					debug(DEBUG_ERROR, SP_PREX "failed to clear sp rules of local\n");
					break;
				}
			} else {
				/*
				 * if non-mtk controller want to remove some rules which are inlcude in standard rule tlv,
				 * just clear it; for mtk controller, all sp rules will be cleared in clear_sp_rules_of_local
				 * at above.
				 */
				if (remove_sp_rules_of_local(&ctx->sp_rule_del_list) < 0)
					debug(DEBUG_ERROR, SP_PREX "failed to remove sp rules of local\n");

				if (apply_sp_rules_to_local(&ctx->sp_rule_static_list, &ctx->sp_rule_dynamic_list,
							    ctx->sp_dp_table.dptbl) < 0) {
					debug(DEBUG_ERROR, SP_PREX "failed to apply sp rules of local\n");
					break;
				}

				if (sp_rule_write_back_2_profile(&ctx->sp_rule_static_list, &ctx->sp_dp_table,
								 SP_RULE_FILE_PATH) < 0) {
					debug(DEBUG_ERROR, SP_PREX "failed to save sp rules\n");
					break;
				}
			}
		}
		break;
#endif

	default:
		break;
	}
	return 0;
}

int insert_topology_discovery_database(struct p1905_managerd_ctx *ctx,
				       unsigned char *al_mac, unsigned char *itf_mac, unsigned char *recv_itf_mac)
{
	struct topology_discovery_db *tpg_db = NULL;

	LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry) {
		if (!os_memcmp(tpg_db->al_mac, al_mac, 6) && !os_memcmp(tpg_db->itf_mac, itf_mac, 6)) {
			return 0;
		}
	}

	if (!tpg_db) {
		tpg_db = (struct topology_discovery_db *)malloc(sizeof(struct topology_discovery_db));
		if (!tpg_db)
			return -1;
		os_memset(tpg_db, 0, sizeof(*tpg_db));
		os_memcpy(tpg_db->al_mac, al_mac, ETH_ALEN);
		os_memcpy(tpg_db->itf_mac, itf_mac, ETH_ALEN);
		os_memcpy(tpg_db->receive_itf_mac, recv_itf_mac, ETH_ALEN);
		tpg_db->time_to_live = TOPOLOGY_DISCOVERY_TTL;
		LIST_INSERT_HEAD(&ctx->topology_entry.tpddb_head, tpg_db, tpddb_entry);
		debug(DEBUG_ERROR,
		      TOPO_PREX "insert discovery db!!! al_mac(" MACSTR ") peer intf(" MACSTR ") recv intf(" MACSTR
		      ")\n", PRINT_MAC(al_mac), PRINT_MAC(itf_mac), PRINT_MAC(recv_itf_mac));
	}

	return 1;
}

struct topology_discovery_db *find_discovery_by_almac(struct p1905_managerd_ctx *ctx, unsigned char *al_mac)
{
	struct topology_discovery_db *tpg_db = NULL;

	LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry) {
		if (!os_memcmp(tpg_db->al_mac, al_mac, ETH_ALEN)) {
			return tpg_db;
		}
	}

	return tpg_db;
}

void find_neighbor_almac_by_intf_mac(struct p1905_managerd_ctx *ctx, unsigned char *itf_mac, unsigned char *almac)
{
	struct topology_discovery_db *tpg_db = NULL;
	struct topology_response_db *tpgr_db = NULL;
	struct device_info_db *dev_db = NULL;

	SLIST_FOREACH(tpgr_db, &ctx->topology_entry.tprdb_head, tprdb_entry) {
		SLIST_FOREACH(dev_db, &tpgr_db->devinfo_head, devinfo_entry) {
			if (!os_memcmp(dev_db->mac_addr, itf_mac, ETH_ALEN))
				break;
		}
		if (dev_db)
			break;
	}

	if (!tpgr_db) {
		debug(DEBUG_ERROR, "cannot find tpgr_db by itf_mac(" MACSTR ")\n", PRINT_MAC(itf_mac));
		return;
	}

	LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry) {
		if (!os_memcmp(tpg_db->al_mac, tpgr_db->al_mac_addr, ETH_ALEN)) {
			os_memcpy(almac, tpg_db->al_mac, ETH_ALEN);
			debug(DEBUG_OFF,
			      RED("neighbor(" MACSTR ") find based on sta(" MACSTR ")\n"),
			      PRINT_MAC(tpg_db->al_mac), PRINT_MAC(itf_mac));
			break;
		}
	}
}

struct topology_response_db *find_response_by_almac(struct p1905_managerd_ctx *ctx, unsigned char *al_mac)
{
	struct topology_response_db *tpgr_db = NULL;

	SLIST_FOREACH(tpgr_db, &ctx->topology_entry.tprdb_head, tprdb_entry) {
		if (!os_memcmp(tpgr_db->al_mac_addr, al_mac, ETH_ALEN)) {
			return tpgr_db;
		}
	}

	return tpgr_db;
}

int find_almac_by_connect_mac(struct p1905_managerd_ctx *ctx, unsigned char *mac, unsigned char *almac)
{
	struct topology_discovery_db *tpg_db = NULL;
	struct topology_response_db *tpgr_db = NULL;
	struct device_info_db *dev_info = NULL;

	LIST_FOREACH(tpg_db, &ctx->topology_entry.tpddb_head, tpddb_entry) {
		if (!os_memcmp(tpg_db->itf_mac, mac, ETH_ALEN)) {
			os_memcpy(almac, tpg_db->al_mac, ETH_ALEN);
			return 1;
		}
	}

	SLIST_FOREACH(tpgr_db, &ctx->topology_entry.tprdb_head, tprdb_entry) {
		SLIST_FOREACH(dev_info, &tpgr_db->devinfo_head, devinfo_entry) {
			if (!os_memcmp(dev_info->mac_addr, mac, ETH_ALEN)) {
				os_memcpy(almac, tpgr_db->al_mac_addr, ETH_ALEN);
				return 2;
			}
		}
	}

	return 0;
}

/*used for PON with MAP device feature
*need reform mesh topology to tree topology in agent side
*/
void mask_control_conn_port(struct p1905_managerd_ctx *ctx, struct topology_response_db *tpgr_db)
{
	struct topology_discovery_db *tpg_db = NULL;
	int rsp_port = 0, disc_port = 0;
	unsigned char role = 0;

	if (ctx->role == CONTROLLER)
		return;

	role = (tpgr_db->support_service == 0 || tpgr_db->support_service == 2) ? CONTROLLER : AGENT;
	rsp_port = tpgr_db->eth_port;

	if (role == CONTROLLER && rsp_port >= 0) {
		tpg_db = find_discovery_by_almac(ctx, tpgr_db->al_mac_addr);
		if (!tpg_db) {
			debug(DEBUG_TRACE, TOPO_PREX "cannot find discovery from Controller via port %d\n", rsp_port);
			return;
		}
		disc_port = tpg_db->eth_port;
		if (disc_port == rsp_port) {
			/*receive response & discovery from same eth port, need mark this port
			 *as controller-connect port. need drop all agents' MC message.
			 */
			debug(DEBUG_TRACE, TOPO_PREX "recv rsp & discovery from the same eth_port=%d\n", rsp_port);
			/*check if need set controller-conncet port */
			if (!ctx->conn_port.is_set) {
				debug(DEBUG_OFF, TOPO_PREX "set conn_port=%d\n", rsp_port);
				ctx->conn_port.is_set = 1;
				ctx->conn_port.port_num = rsp_port;
				os_memcpy(ctx->conn_port.filter_almac, tpgr_db->al_mac_addr, ETH_ALEN);
				/*delete existing neighbor info except for controller */
				delete_topo_disc_db_by_port(ctx, ctx->conn_port.port_num, ctx->conn_port.filter_almac);
			}
		} else {
			debug(DEBUG_ERROR,
			      TOPO_PREX "port different!!! recv sp from port(%d) receive discovery from port(%d)\n",
			      rsp_port, disc_port);
		}
	}
}

void unmask_control_conn_port(struct p1905_managerd_ctx *ctx, int port)
{
	if (ctx->conn_port.is_set && (port == ctx->conn_port.port_num)) {
		ctx->conn_port.is_set = 0;
		debug(DEBUG_ERROR, TOPO_PREX "[GPON]clear conn_port(%d)\n", port);
	}
}

int parse_1905_ack_message(struct p1905_managerd_ctx *ctx, unsigned char *buf, unsigned int left_tlv_len)
{
	unsigned char *type = NULL;
	unsigned short len = 0;
	unsigned int tlv_len = 0;

	type = buf;
	reset_stored_tlv(ctx);

	while (1) {
		len = get_tlv_len(type);
		tlv_len = len + 3;
		if (tlv_len > left_tlv_len) {
			debug(DEBUG_ERROR, "[%d] type = %d, tlv len %d > left_tlv_len %d\n",
			      __LINE__, *type, tlv_len, left_tlv_len);
			return -1;
		}

		if (*type == ERROR_CODE_TYPE)
			store_revd_tlvs(ctx, type, (unsigned short)tlv_len);
		else if (*type == END_OF_TLV_TYPE)
			break;

		type += tlv_len;
		left_tlv_len -= tlv_len;
	}

	return 0;
}
