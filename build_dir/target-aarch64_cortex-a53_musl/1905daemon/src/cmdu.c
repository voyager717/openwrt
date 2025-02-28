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
#include <asm/byteorder.h>
#include <fcntl.h>
#include <unistd.h>

#include "p1905_managerd.h"
#include "topology.h"
#include "cmdu.h"
#include "cmdu_retry_message.h"
#include "message_wait_queue.h"
#include "debug.h"
#include "topology.h"
#include "ethernet_layer.h"
#include "common.h"
#include "file_io.h"
#include "service_prioritization.h"
#include "mapfilter_if.h"
#ifdef MAP_R3
#include "security_engine.h"
#include "map_dpp.h"
#include "wpa_extern.h"
#endif
extern int debug_level;
extern int find_al_mac_address(struct p1905_managerd_ctx *ctx,
	unsigned char *mac_addr, unsigned char *al_mac_addr);
extern int set_opt_not_forward_dest(unsigned char *inputmac);
extern void init_fragment_queue();
extern void uninit_fragment_queue();
extern int _1905_write_mid(char *name, unsigned short mid);
//extern int get_cmdu_tlv_length(unsigned char *buf);

int makeAddr(const char *name, struct sockaddr_un *pAddr, socklen_t *pSockLen)
{
	int ret;
    int nameLen = strlen(name);
    if (nameLen >= (int) sizeof(pAddr->sun_path) -1)  /* too long? */
        return -1;
    pAddr->sun_path[0] = '\0';  /* abstract namespace */
	ret = snprintf(pAddr->sun_path+1, sizeof(pAddr->sun_path) - 1, "%s", name);
	if (os_snprintf_error(sizeof(pAddr->sun_path) - 1, ret)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		return -1;
	}
    pAddr->sun_family = AF_UNIX;
    *pSockLen = 1 + nameLen + offsetof(struct sockaddr_un, sun_path);
    return 0;
}

TAILQ_HEAD(list_head_txq, txq_list) cmdu_txq_head;
extern unsigned char p1905_multicast_address[6];

void delete_cmdu_txq_all(void);
int cmdu_send(struct p1905_managerd_ctx *ctx, unsigned char *dmac,
                     unsigned char *smac, msgtype mtype, unsigned short mid,
                     unsigned char *buffer, unsigned char *ifname);
int cmdu_bridge_relay(struct p1905_managerd_ctx *ctx,
                             unsigned char *buf, int len, unsigned int if_index);
int cmdu_bridge_relay_send(struct p1905_managerd_ctx *ctx,
            unsigned char *smac, unsigned char *buf, int len);




/**
 * check destination mac address of received CMDU. if this CMDU is not for us
 * , we need to ignore this CMDU
 *
 * \param  ctx  p1905_managerd_ctx context.
 * \param  da   pointer of destination MAC.
 * \return  1: for us 0: not for us.
 */
 unsigned char cmdu_dst_mac_check(struct p1905_managerd_ctx *ctx,
        unsigned char *da)
{
    int i = 0;

    if((!memcmp(da, ctx->p1905_al_mac_addr, ETH_ALEN)) ||
        (!memcmp(da, p1905_multicast_address, ETH_ALEN)))
        return 1;

    for(i=0; i<ITF_NUM; i++) {
        if(!memcmp(da, ctx->itf[i].mac_addr, ETH_ALEN))
            return 1;
    }

	return 0;
}


/**
 * check source mac address of received CMDU. if this received CMDU is send by us, drop it
 *
 * \param  ctx  p1905_managerd_ctx context.
 * \param  da   pointer of source MAC.
 * \return  1: from us 0: not from us.
 */
unsigned char cmdu_src_mac_check(struct p1905_managerd_ctx *ctx,
        unsigned char *sa)
{
    int i= 0;

    if((!memcmp(sa, ctx->p1905_al_mac_addr, ETH_ALEN)))
		return 1;

	for(i = 0; i < ctx->itf_number; i++) {
		if(!memcmp(sa, ctx->itf[i].mac_addr, ETH_ALEN))
			return 1;
	}

    return 0;
}

int cmdu_recv_intf_check(struct p1905_managerd_ctx *ctx)
{
	if (!(ctx->itf[ctx->recent_cmdu_rx_if_number].trx_config & (RX_MUL|RX_UNI))) {
		debug(DEBUG_ERROR, "recv intf %s trx_config(%02x) has no receiving cap; drop!\n",
			ctx->itf[ctx->recent_cmdu_rx_if_number].if_name,
			ctx->itf[ctx->recent_cmdu_rx_if_number].trx_config);
		return -1;
	}

	return 0;
}

/**
 *  bridge init
 *
 * \param  ctx  p1905_managerd_ctx context.
 * \return  error code.
 */
/*int bridge_init(struct p1905_managerd_ctx *ctx)
{
    int f, s;
    struct ifreq ifr;

    if ((f=socket(PF_PACKET, SOCK_RAW, host_to_be16(ETH_P_ALL)))<0)
        return -1;

    strcpy(ifr.ifr_name, (char *)ctx->br_name);
    if ((s = ioctl(f, SIOCGIFFLAGS, &ifr))<0){
      close(f);
      return-1;
    }

    ifr.ifr_flags |= IFF_PROMISC;
    if ((s = ioctl(f, SIOCSIFFLAGS, &ifr)) < 0){
		close(f);
		return -1;
    }
    debug(DEBUG_OFF, "Setting interface ::: %s ::: to promisc\n", ifr.ifr_name);
	close(f);

    return 0;
}*/

/**
 * init cmdu tx queue.
 *
 */
void cmdu_txq_init(void)
{
    TAILQ_INIT(&cmdu_txq_head);
}

#ifdef SUPPORT_CONTROL_SOCKET
int _1905_ctrl_sock_init(struct p1905_managerd_ctx *ctx)
{
	struct sockaddr_un addr;
	socklen_t server_len = 0;

	os_memset(&addr, 0, sizeof(struct sockaddr_un));
	/* Initialize control interface */
	ctx->ctrl_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (ctx->ctrl_sock < 0) {
		debug(DEBUG_ERROR, "open ctrl_sock fail\n");
		return -1;
	}

	makeAddr("/tmp/1905ctrl", &addr, &server_len);

	if (bind(ctx->ctrl_sock, (struct sockaddr *)&addr, server_len) < 0) {
		debug(DEBUG_ERROR, "bind addr to ctrl_sock fail(%s)\n", strerror(errno));
		unlink(addr.sun_path);
		close(ctx->ctrl_sock);
		return -1;
	}

	if(listen(ctx->ctrl_sock, 1) < 0)
	{
		unlink(addr.sun_path);
		close(ctx->ctrl_sock);
		debug(DEBUG_ERROR, "listen ctrl_sock fail(%s)\n", strerror(errno));
		return -1;
	}

	return 0;
}
#ifdef MAP_R2
extern void map_rm_hnat_session();
#endif
void _1905_ctrl_interface_recv_and_parse(struct p1905_managerd_ctx *ctx,
		char *buf, int len)
{
	struct sockaddr_un from;
	int lenfrom = sizeof(from);
	int recv_fd = 0;
	char *file = NULL;
	int total_len = 0;

	recv_fd = accept(ctx->ctrl_sock, (struct sockaddr *)&from, (socklen_t *)&lenfrom);
	if (recv_fd < 0) {
		debug(DEBUG_ERROR, "accept ctrl_sock fail\n");
		return;
	}

	total_len = recv(recv_fd, buf, len - 1, 0);
	if (total_len <= 0) {
		debug(DEBUG_ERROR, "recv ctrl_sock fail\n");
		close(recv_fd);
		return;
	}

	buf[total_len] = '\0';

	if (os_strncmp(buf, "version", strlen("version")) == 0) {
		show_1905_mapfilter_version(ctx);
	} else if (os_strncmp(buf, "dev_send_1905", os_strlen("dev_send_1905")) == 0) {
		file = buf + os_strlen("dev_send_1905") + 1;
		send_1905_raw_data(ctx, file);
	} else if (os_strncmp(buf, "dev_set_config", os_strlen("dev_set_config")) == 0) {
		if (ctx->role == CONTROLLER) {
			file = buf + os_strlen("dev_set_config") + 1;
			read_1905_bss_config(ctx, file);
		} else {
			debug(DEBUG_ERROR, "agent mode, no need hanle dev_set_config\n");
		}
	} else if (os_strncmp(buf, "dump_topology_info", os_strlen("dump_topology_info")) == 0) {
		dump_topology_info(ctx);
	} else if (os_strncmp(buf, "dump_topology_tree", os_strlen("dump_topology_tree")) == 0) {
		dump_topology_tree(ctx);
		eth_layer_port_clients_dump();
	} else if (os_strncmp(buf, "dump_radio_info", os_strlen("dump_radio_info")) == 0) {
		dump_radio_info(ctx);
	} else if (os_strncmp(buf, "log_level", os_strlen("log_level")) == 0) {
		int level = 0;
		char *p = buf + os_strlen("log_level") + 1;

		level = atoi(p);
		set_debug_level(level);
	} else if (os_strncmp(buf, "get_eth_port_type", os_strlen("get_eth_port_type")) == 0) {
		test_eth_layer_get_port_type();
	}  else if (os_strncmp(buf, "get_eth_entry", os_strlen("get_eth_entry")) == 0) {
		test_eth_layer_get_client_entry();
	}  else if (os_strncmp(buf, "find_eth_entry", os_strlen("find_eth_entry")) == 0) {
		unsigned char mac[ETH_ALEN];
		if(!hwaddr_aton(buf + os_strlen("find_eth_entry") + 1, mac))
			test_eth_layer_search_table_entry(mac);
	} else if (os_strncmp(buf, "get_eth_port_status", os_strlen("get_eth_port_status")) == 0) {
		int port_index = 0;
		char *p = buf + os_strlen("get_eth_port_status") + 1;
		port_index = atoi(p);
		test_eth_layer_get_port_status(port_index);
	} else if (os_strncmp(buf, "set_eth_ts_vlan_id", os_strlen("set_eth_ts_vlan_id")) == 0) {
		char *p = buf + os_strlen("set_eth_ts_vlan_id") + 1;
		char *str_vlan = NULL;
		unsigned short vids[256];
		unsigned char vid_num = 0;

		while(1) {
			str_vlan = strsep(&p, ",;");
			if (!str_vlan)
				break;
			if (*str_vlan == '\0')
				continue;

			vids[vid_num++] = atoi(str_vlan);
		}

		test_eth_layer_set_vlan(vids, vid_num, 0);
#ifdef MAP_R2
		map_rm_hnat_session();
#endif
	}  else if (os_strncmp(buf, "set_eth_trans_vlan_id", os_strlen("set_eth_trans_vlan_id")) == 0) {
		char *p = buf + os_strlen("set_eth_trans_vlan_id") + 1;
		char *str_vlan = NULL;
		unsigned short vids[256];
		unsigned char vid_num = 0;

		while(1) {
			str_vlan = strsep(&p, ",;");
			if (!str_vlan)
				break;
			if (*str_vlan == '\0')
				continue;

			vids[vid_num++] = atoi(str_vlan);
		}

		test_eth_layer_set_vlan(vids, vid_num, 1);
	} else if (os_strncmp(buf, "clear_switch_table", os_strlen("clear_switch_table")) == 0) {
		test_eth_layer_clear_switch_table();
	} else if (os_strncmp(buf, "mapfilter_debug", os_strlen("mapfilter_debug")) == 0) {
		mapfilter_dump_debug_info();
	} else if (os_strncmp(buf, "mapfilter_set_primary", os_strlen("mapfilter_set_primary")) == 0) {
		unsigned char mac[ETH_ALEN] = {0};

		debug(DEBUG_ERROR, "%s\n", buf);
		if(!hwaddr_aton(buf + os_strlen("mapfilter_set_primary") + 1, mac)) {
			struct local_interface itf;
			int primary = INF_UNKNOWN;
			char *p = NULL;

			os_memset(&itf, 0, sizeof(struct local_interface));
			os_memcpy(itf.mac, mac, ETH_ALEN);
			itf.dev_type = APCLI;
			p = buf + os_strlen("mapfilter_set_primary") + 1 + 18;
			primary = atoi(p);

			debug(DEBUG_ERROR, "mapfilter_set_primary "MACSTR" %02x\n", PRINT_MAC(mac), primary);
			mapfilter_set_primary_interface(&itf, primary);
		}
	} else if (os_strncmp(buf, "mapfilter_set_up_path", os_strlen("mapfilter_set_up_path")) == 0) {
		unsigned char mac[ETH_ALEN] = {0}, mac2[ETH_ALEN] = {0};

		debug(DEBUG_ERROR, "%s\n", buf);
		if(!hwaddr_aton(buf + os_strlen("mapfilter_set_up_path") + 1, mac)) {
			struct local_interface in, out;
			char *p = NULL;

			os_memset(&in, 0, sizeof(struct local_interface));
			os_memset(&out, 0, sizeof(struct local_interface));
			os_memcpy(in.mac, mac, ETH_ALEN);

			p = buf + os_strlen("mapfilter_set_up_path") + 1 + 18;
			if (!hwaddr_aton(p, mac2)) {
				os_memcpy(out.mac, mac2, ETH_ALEN);
				debug(DEBUG_ERROR, "mapfilter_set_up_path "MACSTR"->"MACSTR"\n",
					PRINT_MAC(mac), PRINT_MAC(mac2));
				mapfilter_set_uplink_path(&in, &out);
			}
		}
	} else if (os_strncmp(buf, "show_PON_dev", os_strlen("show_PON_dev")) == 0) {
		debug(DEBUG_ERROR, "%s\n", buf);
		show_PON_dev(ctx);
	} else if (strncmp(buf, "set_debug_level", strlen("set_debug_level")) == 0) {
		debug(DEBUG_OFF, "buf %s\n", buf);
		debug_level = atoi(buf + strlen("set_debug_level") + 1);
		debug(DEBUG_OFF, "change debug level to %d\n", debug_level);
	}
#ifdef MAP_R2
	else if (os_strncmp(buf, "set_ts_config", strlen("set_ts_config")) == 0){

		char *tmp = buf + os_strlen("set_ts_config") + 1;
		if (strncmp(tmp, "pvlan", strlen("pvlan")) == 0) {
			cmd_set_ts_pvlan(ctx, tmp + os_strlen("pvlan") + 1);
		} else if (strncmp(tmp, "policy", strlen("policy")) == 0) {
			tmp = tmp + os_strlen("policy") + 1;
			if (strncmp(tmp, "clean", strlen("clean")) == 0)
				cmd_set_ts_policy_clear(ctx);
			else if (strncmp(tmp, "done", strlen("done")) == 0)
				cmd_set_ts_policy_done(ctx);
			else
				cmd_set_ts_policy(ctx, tmp);
		} else {
			debug(DEBUG_ERROR, "%s no such command\n", __func__);
		}
	} else if (os_strncmp(buf, "show_ts_info", strlen("show_ts_info")) == 0) {
		show_all_assoc_clients(ctx);
		show_all_operational_bss(ctx);
	} else if (os_strncmp(buf, "ts_onoff", strlen("ts_onoff")) == 0) {
		debug(DEBUG_OFF, "buf %s\n", buf);
		char onoff = atoi(buf + os_strlen("ts_onoff") + 1);
		debug(DEBUG_OFF, "change ts to %d\n", onoff);
		mapfilter_ts_onoff(onoff);
	}
#endif// #ifdef MAP_R2
#ifdef MAP_R3_SP
	else if (os_strncmp(buf, "show_sp", strlen("show_sp")) == 0){
		debug(DEBUG_ERROR, "[static rule list]\n");
		show_rule_list_dptbl(&ctx->sp_rule_static_list, NULL);
		debug(DEBUG_ERROR, "[dynamic rule list]\n");
		show_rule_list_dptbl(&ctx->sp_rule_dynamic_list, NULL);
		debug(DEBUG_ERROR, "[dscp mapping table]\n");
		show_rule_list_dptbl(NULL, &ctx->sp_dp_table);
		debug(DEBUG_ERROR, "[learn complete rule list]\n");
		show_rule_list_dptbl(&ctx->sp_rule_learn_complete_list, NULL);
	}
#endif
#ifdef MAP_R3
	else if (strncmp(buf, "dump_security_info", strlen("dump_security_info")) == 0) {
		char *reply = NULL;
		const int reply_size = 10240;
		int reply_len = 0;

		reply = os_zalloc(reply_size);
		if (reply == NULL) {
			debug(DEBUG_ERROR, "[%d]dump_security_info reply zalloc fail!\n", __LINE__);
			close(recv_fd);
			return;
		}

		reply_len = dump_security_info(reply, reply_size);
		if (reply_len < 0 || !reply) {
			debug(DEBUG_ERROR, "[%d]dump_security_info construct reply buf fail!\n", __LINE__);
			os_free(reply);
			close(recv_fd);
			return;
		}

		if (send(recv_fd, reply, reply_len, 0) != reply_len)
			debug(DEBUG_ERROR, "[errno = %d][%d]dump_security_info reply send fail!\n", errno, __LINE__);

		os_free(reply);

	} else if (strncmp(buf, "set_key", strlen("set_key")) == 0) {
		cmd_set_key(buf + os_strlen("set_key") + 1);
	} else if (strncmp(buf, "set_sec_log_level", strlen("set_sec_log_level")) == 0) {
		int level = 0;
		char *p = buf + os_strlen("set_sec_log_level") + 1;

		level = atoi(p);
		set_security_log_level(level);
	} else if (strncmp(buf, "set_sec_config", strlen("set_sec_config")) == 0) {
		cmd_set_security_config(buf + os_strlen("set_sec_config") + 1);
	}
    else if (strncmp(buf, "dev_start_buffer", strlen("dev_start_buffer")) == 0) {
        if ((ctx->map_version == MAP_PROFILE_R3) && ctx->MAP_Cer)
		    cmd_dev_start_buffer(ctx, buf + os_strlen("dev_start_buffer") + 1);
	} else if (strncmp(buf, "dev_stop_buffer", strlen("dev_stop_buffer")) == 0) {
	    if ((ctx->map_version == MAP_PROFILE_R3) && ctx->MAP_Cer)
		    cmd_dev_stop_buffer(ctx);
	} else if (strncmp(buf, "dev_get_frame_info", strlen("dev_get_frame_info")) == 0) {
	    if ((ctx->map_version == MAP_PROFILE_R3) && ctx->MAP_Cer)
		    cmd_dev_get_frame_info(ctx, buf + os_strlen("dev_get_frame_info") + 1);
	} else if (strncmp(buf, "send_bss_reconfig_trigger", strlen("send_bss_reconfig_trigger")) == 0) {
		if ((ctx->map_version == MAP_PROFILE_R3) && (ctx->role == CONTROLLER))
			cmd_send_bss_reconfiguration_trigger(ctx, buf + os_strlen("send_bss_reconfig_trigger") + 1);
	}
#endif // MAP_R3
	else {
		debug(DEBUG_ERROR, "%s no such command\n", __func__);
	}
	close(recv_fd);
}

void  _1905_ctrl_sock_deinit(struct p1905_managerd_ctx *ctx)
{
	close(ctx->ctrl_sock);
	debug(DEBUG_OFF, "\n");
}
#endif

/**
 * Initialize all parameters for CMDU.
 *
 * \param  ctx  p1905_managerd_ctx context.
 * \return  error code.
 */
int cmdu_init(struct p1905_managerd_ctx *ctx)
{
	struct sockaddr_ll sll;
	struct ifreq ifr;
	int nSndBufLen = 0,nRcvBufLen = 0;
	socklen_t optlen = sizeof(int);
	int ret;

	ctx->mid = 0;
	ctx->need_relay = 0;

	if(0 > (ctx->sock_br = socket(AF_PACKET, SOCK_RAW, ETH_P_1905)))
	{
	    debug(DEBUG_ERROR, "cannot open socket on %s (%s)", ctx->br_name,
	           strerror(errno));
	    return -1;
	}

	ret = snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ctx->br_name);
	if (os_snprintf_error(sizeof(ifr.ifr_name), ret)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		close(ctx->sock_br);
		return -1;
	}

	if(-1 == (ioctl(ctx->sock_br, SIOCGIFINDEX, &ifr)))
	{
	    debug(DEBUG_ERROR, "cannot get interface %s index (%s)\n", ctx->br_name,
	           strerror(errno));
	    close(ctx->sock_br);
	    return -1;
	}

	if (getsockopt(ctx->sock_br, SOL_SOCKET, SO_SNDBUF, (void *)&nSndBufLen, &optlen) < 0) {
		nSndBufLen = 1024*1024;
	}
	else {
		nSndBufLen = nSndBufLen * 2;
	}

	if (getsockopt(ctx->sock_br, SOL_SOCKET, SO_RCVBUF, (void *)&nRcvBufLen, &optlen) < 0) {
		nRcvBufLen = 1024*1024;
	}
	else {
		nRcvBufLen = nRcvBufLen * 2;
	}

	if (setsockopt(ctx->sock_br, SOL_SOCKET, SO_SNDBUF, (const char*)&nSndBufLen, sizeof(int)) < 0) {
		printf("warning: %s set send buffer size failed, %s\n", __func__, strerror(errno));
	} else {
		printf("%s set send buffer size %d\n", __func__, nSndBufLen);
	}

	if (setsockopt(ctx->sock_br, SOL_SOCKET, SO_RCVBUF, (const char*)&nRcvBufLen, sizeof(int)) < 0) {
		printf("warning: %s set recv buffer size failed, %s\n", __func__, strerror(errno));
	} else {
		printf("%s set recv buffer size %d\n", __func__, nRcvBufLen);
	}

	/*Create a receive connection of CMDU on bridge interface*/
	memset(&sll, 0, sizeof(sll));
	sll.sll_family = AF_PACKET;
	sll.sll_ifindex = ifr.ifr_ifindex;
	sll.sll_protocol = host_to_be16(ETH_P_1905);
	if(-1 == (bind(ctx->sock_br, (struct sockaddr *)&sll,
	          sizeof(struct sockaddr_ll))))
	{
	    debug(DEBUG_ERROR, "cannot bind raw socket to interface %s (%s)",
	           ctx->br_name, strerror(errno));
	    close(ctx->sock_br);
	    return -1;
	}

    /*Get br mac address*/
    if(-1 == (ioctl(ctx->sock_br, SIOCGIFHWADDR, &ifr))) {
        debug(DEBUG_ERROR, "cannot get interface %s mac address (%s)",
               ctx->br_name, strerror(errno));
        close(ctx->sock_br);
        return -1;
    }
    memcpy(ctx->br_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	debug(DEBUG_ERROR, "br mac addr "MACSTR"",
		   PRINT_MAC(ctx->br_addr));

    /* Let bridge do not forward if dest address is local AL MAC address.
     * It is for 1905.1 unicast message usage.
     */
	if (set_opt_not_forward_dest(ctx->p1905_al_mac_addr) < 0) {
		close(ctx->sock_br);
		return -1;
	}


    /*initialize the cmdu transmit queue */
    cmdu_txq_init();

    /* initialize the cmdu fragment queue. It will queue fragment cmdu until
     * all fragmenets received
     */
    init_fragment_queue();

	/*
	 * initialization retry queue. it will queue message which needs receive 1905_ack until
	 * 1905_ack received or timeout
	*/
	 init_retry_queue();

	/*
	 * initialization message wait queue. it will queue unsolicited event while waiting for
	 * some specific event.
	*/
	init_message_wait_queue();

    return 0;
}

/**
 * uninit cmdu, it will be called when exit this process.
 *
 * \param  ctx  p1905_managerd_ctx context.
 */
void cmdu_uninit(struct p1905_managerd_ctx *ctx)
{
	unsigned int i = 0;

	close(ctx->sock_br);
	for (i = 0; i < ctx->itf_number; i++)
		close(ctx->sock_inf_recv_1905[i]);
    /*delete all message queue in TX queue*/
#if 0
	concurrent_sock_deinit(ctx);
#endif
#ifdef SUPPORT_CONTROL_SOCKET
	_1905_ctrl_sock_deinit(ctx);
#endif
    delete_cmdu_txq_all();

    /*delete all fragment message queue in fragment queue*/

    uninit_fragment_queue();
    /*delete all retry message in retry message queue*/
	uninit_retry_queue();

	uninit_message_wait_queue();
}

/**
 *  for cmdu message sniffer.
 *
 * \param  ctx  p1905_managerd_ctx context.
 * \param  buf  pointer of buffer for tx.
 * \param  len  transmit length.
 * \return error code
 */
int cmdu_eth0_send(struct p1905_managerd_ctx *ctx,
                             unsigned char *buf, int len)
{
    int length;

    length = send(ctx->sock_eth0, buf, len, 0);

    if(0 > length)
    {
        debug(DEBUG_WARN, "relay failed on %s (%s)", ETH0_IFNAME,
               strerror(errno));
        return -1;
    }

    return 0;
}

int sniffer_init(struct p1905_managerd_ctx *ctx)
{
#if 0
    struct ifreq ifr;

    /*disable sniffer function when initailization*/
    ctx->sniffer_enable = 0;

    if(0 > (ctx->sock_eth0 = socket(AF_PACKET, SOCK_RAW, ETH_P_1905)))
    {
        syslog(LOG_WARNING, "cannot open socket on %s (%s)", ETH0_IFNAME,
               strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, (char*)ETH0_IFNAME, IFNAMSIZ);

    if(-1 == (ioctl(ctx->sock_eth0, SIOCGIFINDEX, &ifr)))
    {
	    syslog(LOG_WARNING, "cannot get interface %s index (%s)", ETH0_IFNAME,
               strerror(errno));
	    close(ctx->sock_eth0);
	    return -1;
    }

    ctx->eth0_sll.sll_family = AF_PACKET;
    ctx->eth0_sll.sll_ifindex = ifr.ifr_ifindex;
    ctx->eth0_sll.sll_protocol = htons(ETH_P_1905);

    /*Bind eth0 socket to this interface*/
    if(-1 == (bind(ctx->sock_eth0, (struct sockaddr *)&ctx->eth0_sll,
          sizeof(struct sockaddr_ll))))
    {
        syslog(LOG_WARNING, "cannot bind raw socket to interface %s (%s)",
            ETH0_IFNAME, strerror(errno));
        close(ctx->sock_eth0);
        return -1;
    }
#endif

	return 0;
}

void sniffer_uninit(struct p1905_managerd_ctx *ctx)
{
#if 0
    close(ctx->sock_eth0);
#endif
}

/**
 *  get the cmdu message in tx queue and send to bridge.
 *
 * \param  ctx  p1905_managerd_ctx context.
 * \param  buffer  tx buffer.
 * \return  error code.
 */
int process_cmdu_txq(struct p1905_managerd_ctx *ctx, unsigned char *buffer)
{
    struct txq_list *tlist;
    int result = 0;
	unsigned int i = 0;
	unsigned char *ifname = NULL;

    if(TAILQ_EMPTY(&cmdu_txq_head))
    {
       debug(DEBUG_INFO, "txq empty\n");
       return 0;
    }

    TAILQ_FOREACH(tlist, &cmdu_txq_head, cmdu_txq_entry)
    {
        debug(DEBUG_TRACE, "txq send:\n");
        debug(DEBUG_TRACE, "dst "MACSTR"\n", PRINT_MAC(tlist->dmac));
        debug(DEBUG_TRACE, "src "MACSTR"\n", PRINT_MAC(tlist->smac));
        debug(DEBUG_TRACE, "mtype 0x%04x\n", tlist->mtype);
        debug(DEBUG_TRACE, "mid 0x%04x\n",tlist->mid);

		for (i = 0; i < ctx->itf_number; i++) {
			if (!memcmp(tlist->ifname, ctx->itf[i].if_name, strlen((char *)tlist->ifname))) {
				ifname = ctx->itf[i].if_name;
				break;
			}
		}
		if (i >= ctx->itf_number) {
			debug(DEBUG_TRACE, "txq unkonwn inf name %s, tx by br\n", tlist->ifname);
			debug(DEBUG_TRACE, "mtype 0x%04x\n", tlist->mtype);
			ifname = ctx->br_name;
		} else {
			/*drop this packet that does not allow to send multicast*/
			if (!(ctx->itf[i].trx_config & TX_MUL) && !os_memcmp(tlist->dmac, p1905_multicast_address, ETH_ALEN)) {
				debug(DEBUG_TRACE, "drop tx multicast packet on %s; this intf has no tx multicast cap\n", ctx->itf[i].if_name);
				continue;
			}

			if (!(ctx->itf[i].trx_config & TX_UNI) && os_memcmp(tlist->dmac, p1905_multicast_address, ETH_ALEN)) {
				debug(DEBUG_TRACE, "drop tx unicast packet on %s; this intf has no tx unicast cap\n", ctx->itf[i].if_name);
				continue;
			}
		}

        result = cmdu_send(ctx, tlist->dmac, tlist->smac,
                 tlist->mtype, tlist->mid, buffer, ifname);
        if(0 > result)
        {
           debug(DEBUG_ERROR, "txq send fail\n");
           return -1;
        }
		if(tlist->need_update_mid)
		{
			if(_1905_write_mid("/tmp/msg_id.txt", tlist->mid) < 0)
			{
				debug(DEBUG_ERROR, "update mid=0x%04x to dev_send_1905_mid.txt fail\n", tlist->mid);
			}
			else
			{
				debug(DEBUG_TRACE, "update mid=0x%04x to dev_send_1905_mid.txt success\n", tlist->mid);
			}
		}
    }
    delete_cmdu_txq_all();
	if (ctx->sta_notifier)
		ctx->sta_notifier = 0;
    return 0;
}

/**
 *  insert cmdu message into tx queue.
 *
 * \param  dmac  destination mac address.
 * \param  smac  source mac addr, it awii affect the forwarding of bridge.
 * \param  mtype  cmdu message type, defined in 1905.1 spec.
 * \param  mid  message id, defined in 1905.1 spec
 */
void insert_cmdu_txq(unsigned char *dmac, unsigned char *smac, msgtype mtype,
                    unsigned short mid, unsigned char* ifname, unsigned char need_update_mid)
{
	int ret;
    struct txq_list *tlist;

    tlist = (struct txq_list*)malloc(sizeof(struct txq_list));
    if (!tlist)
        return;
    memset(tlist, 0, sizeof(struct txq_list));
    memcpy(tlist->dmac, dmac, 6);
    memcpy(tlist->smac, smac, 6);
	ret = snprintf((char *)tlist->ifname, sizeof(tlist->ifname), "%s", ifname);
	if (os_snprintf_error(sizeof(tlist->ifname), ret) < 0)
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
    tlist->mtype = mtype;
    tlist->mid = mid;
	tlist->need_update_mid = need_update_mid;
    TAILQ_INSERT_TAIL(&cmdu_txq_head, tlist, cmdu_txq_entry);
}

/**
 *  delete all cmdu message in tx queue.
 *
 */
void delete_cmdu_txq_all(void)
{
    struct txq_list *tlist, *tlist_temp;

    tlist = TAILQ_FIRST(&cmdu_txq_head);
    while(tlist)
    {
        tlist_temp = TAILQ_NEXT(tlist, cmdu_txq_entry);
        TAILQ_REMOVE(&cmdu_txq_head, tlist, cmdu_txq_entry);
        free(tlist);
        tlist = tlist_temp;
    }
}

/**
 *  Fragment tx. Use this funciton when CMDU message size exceed max size.
 *
 * \param  ctx  p1905_managerd_ctx context.
 * \param  tlv  pointer of tlv start position.
 * \param  msg_hdr  pointer of message header start position.
 * \param  buffer   pointer of start position of whole message.
 * \return error code
 */
int cmdu_tx_fragment(struct p1905_managerd_ctx *ctx,
	unsigned char *buffer, unsigned short buflen, unsigned char *ifname)
{
    unsigned short offset = 0;
    unsigned short tmp_length = 0;
	cmdu_message_header *msg_hdr = NULL;
    unsigned char *tlv_buf_raw = NULL, *tlv_buf = NULL;
    unsigned char *temp_buf = NULL;
	unsigned char *tlv = NULL;
    unsigned short tlv_len = 0, total_tlv_len = 0;
    int len = 0, org_len = 0, send_len = 0;
	int sock = 0;
	unsigned int i = 0;

	for (i = 0; i < ctx->itf_number; i++) {
		if (!memcmp(ifname, ctx->itf[i].if_name, strlen((char *)ifname))) {
			sock = ctx->sock_inf_recv_1905[i];
			break;
		}
	}
	if (i >= ctx->itf_number)
		sock = ctx->sock_br;

	msg_hdr = (cmdu_message_header *)(buffer + ETH_HLEN);
	tlv = buffer + ETH_HLEN + CMDU_HLEN;
	total_tlv_len = buflen - ETH_HLEN - CMDU_HLEN;
	tlv_buf = (unsigned char *)malloc(total_tlv_len);
	if (!tlv_buf) {
		debug(DEBUG_ERROR, "alloc tlv_buf fail\n");
		return -1;
	}
	tlv_buf_raw = tlv_buf;

	memcpy(tlv_buf, tlv, total_tlv_len);
	temp_buf = tlv_buf;

	i = 0;
	for (;;) {
		if ((*temp_buf) == END_OF_TLV_TYPE) {
			/* if last fragment is larger than MAX_TLVS_LENGTH,
			* we need to seperate this fragment into two fragments
			* the total length of end of tlv is 3
			*/
			if ((offset + 3) > MAX_TLVS_LENGTH) {
#ifdef MAP_R3
				memcpy(tlv, tlv_buf, MAX_TLVS_LENGTH);
				msg_hdr->fragment_id = i;
				msg_hdr->last_fragment_indicator = 0;
				len = MAX_TLVS_LENGTH + CMDU_HLEN + ETH_HLEN;
#else
				memcpy(tlv, tlv_buf, offset);
				msg_hdr->fragment_id = i;
				msg_hdr->last_fragment_indicator = 0;
				len = offset + CMDU_HLEN + ETH_HLEN;
#endif
				org_len = len;
				send_len = send(sock, buffer, len, 0);
				if (0 > send_len) {
					debug(DEBUG_ERROR,"err1 offset(%d) len(%d) fragment_id(%d)\n", offset, len, i);
					goto error;
				}

				debug(DEBUG_TRACE,"1msgtype=0x%04x, interface=%s fid=%d mid=%d lf=%d\n",
					be_to_host16(msg_hdr->message_type), ifname,
					msg_hdr->fragment_id, be_to_host16(msg_hdr->message_id),
					msg_hdr->last_fragment_indicator);
				/*add for fragment id*/
				i++;

				memset(tlv, 0, MIN_TLVS_LENGTH);
				msg_hdr->fragment_id = i;
				msg_hdr->last_fragment_indicator = 1;

				len = MIN_TLVS_LENGTH + CMDU_HLEN + ETH_HLEN;
				org_len = len;
				send_len = send(sock, buffer, len, 0);
				if(0 > send_len) {
					debug(DEBUG_ERROR,"err2 len(%d) fragment_id(%d)\n",len, i);
				    goto error;
				}

				debug(DEBUG_TRACE,"2msgtype=0x%04x, interface=%s fid=%d mid=%d lf=%d\n",
					be_to_host16(msg_hdr->message_type), ifname,
					msg_hdr->fragment_id, be_to_host16(msg_hdr->message_id),
					msg_hdr->last_fragment_indicator);
			} else {
				/* It is the last fragment so we set last fragment ind to 1.
				* And send this buffer
				*/
				msg_hdr->fragment_id = i;
				msg_hdr->last_fragment_indicator = 1;

				memcpy(tlv, tlv_buf, (offset + 3));

				if ((offset + 3) < MIN_TLVS_LENGTH) {
					memset((tlv+(offset + 3)), 0, MIN_TLVS_LENGTH-(offset + 3));
					tmp_length = MIN_TLVS_LENGTH;
				} else {
					tmp_length = (offset + 3);
				}
				len = tmp_length + CMDU_HLEN + ETH_HLEN;
				org_len = len;

				send_len = send(sock, buffer, len, 0);
				if(0 > send_len) {
					debug(DEBUG_ERROR,"err3 offset(%d) len(%d) fragment_id(%d)\n", offset, len, i);
					goto error;
				}
				debug(DEBUG_TRACE, "3msgtype=0x%04x, interface=%s fid=%d mid=%d lf=%d\n",
					be_to_host16(msg_hdr->message_type), ifname,
					msg_hdr->fragment_id, be_to_host16(msg_hdr->message_id),
					msg_hdr->last_fragment_indicator);
			}
			goto out;
		}

		/* below is non-last fragment case.
		* We should go here first when fragment tx happen.
		*/
		tlv_len = get_cmdu_tlv_length(temp_buf);
		if (tlv_len > total_tlv_len) {
			debug(DEBUG_ERROR, "tlv_len %d is bigger than total_tlv_len %d\n",
				tlv_len, total_tlv_len);
			goto error;
		}
		debug(DEBUG_TRACE, "tlv_type=0x%02x, tlv_len=%d offset=%d tlv_len=%d\n",
			*temp_buf, tlv_len, offset, tlv_len);
		/*shift buffer to correct position*/
		temp_buf += tlv_len;
#ifdef MAP_R3
		/* one encrypted tlv may bigger than 0x3FFF for R3
		** no one tlv could exceed 0x3FFF for R1 and R2
		*/
		if (tlv_len > MAX_TLVS_LENGTH) {
			while ((offset + tlv_len) > MAX_TLVS_LENGTH) {
				memcpy(tlv, tlv_buf, MAX_TLVS_LENGTH);

				/*shift to start position of not sent cmdu payload */
				tlv_buf += MAX_TLVS_LENGTH;
				tlv_len -= (MAX_TLVS_LENGTH - offset);
				offset = 0;

				/* set fragment id & last fragment ind in cmdu message header.
				* In this case, last fragment ind must be 0 because no end_of_tlv.
				*/
				msg_hdr->fragment_id = i;
				msg_hdr->last_fragment_indicator = 0;

				len = MAX_TLVS_LENGTH + CMDU_HLEN + ETH_HLEN;
				org_len = len;
				send_len = send(sock, buffer, len, 0);
				if(0 > send_len)
					goto error;
				debug(DEBUG_TRACE, "frag_cmdu%d, msgtype=0x%04x, interface=%s fid=%d mid=%d lf=%d, len:%d\n",
					i, be_to_host16(msg_hdr->message_type), ifname,
					msg_hdr->fragment_id, be_to_host16(msg_hdr->message_id),
					msg_hdr->last_fragment_indicator, len);

				i++;
				}

			offset += tlv_len;
		}else
#endif

        if ((offset + tlv_len) > MAX_TLVS_LENGTH) {
            memcpy(tlv, tlv_buf, offset);

            /* set fragment id & last fragment ind in cmdu message header.
             * In this case, last fragment ind must be 0 because no end_of_tlv.
             */
            msg_hdr->fragment_id = i;
            msg_hdr->last_fragment_indicator = 0;

            len = offset + CMDU_HLEN + ETH_HLEN;
			org_len = len;
            send_len = send(sock, buffer, len, 0);
			debug(DEBUG_TRACE, "frag_tlv_len=%d send_len(%d)\n", offset, send_len);
            if(0 > send_len) {
				debug(DEBUG_ERROR,"err4 offset(%d) len(%d) fragment_id(%d)\n", offset, len, i);
                goto error;
            }

			debug(DEBUG_TRACE, "4msgtype=0x%04x, interface=%s fid=%d mid=%d lf=%d\n",
				be_to_host16(msg_hdr->message_type), ifname,
				msg_hdr->fragment_id, be_to_host16(msg_hdr->message_id),
				msg_hdr->last_fragment_indicator);
            /*shift to start position of not sent cmdu payload */
            tlv_buf += offset;

            /* record the tlv_len, which was gotten by get_cmdu_tlv_length
             * Don't worry about buffer address, because we will shift buffer
             * to correct position in the end of this for loop.
             */
            offset = tlv_len;

            /*for fragment id use*/
            i++;
        } else {
            offset += tlv_len;
        }
    }

out:
	free(tlv_buf_raw);
	return 0;
error:
	debug(DEBUG_ERROR, "error msgtype=0x%04x, interface=%s fid=%d mid=%d lf=%d errno(%d-%s), len=%d\n",
		be_to_host16(msg_hdr->message_type), ifname,
		msg_hdr->fragment_id, be_to_host16(msg_hdr->message_id),
		msg_hdr->last_fragment_indicator, errno, strerror(errno), org_len);
	hex_dump_all("error send frag",tlv_buf_raw,total_tlv_len);
	free(tlv_buf_raw);
	return -1;
}

struct _config_sync {
	unsigned char al_mac[ETH_ALEN];
	unsigned char band;
};

int check_reliable_msg(unsigned short mtype)
{
    if ((mtype == TOPOLOGY_NOTIFICATION) ||
        (mtype == ASSOCIATION_STATUS_NOTIFICATION))
        return 1;

    return 0;
}


/**
 *  create & send cmdu message to the corresponding interface.
 *
 * \param  ctx  p1905_managerd_ctx context.
 * \param  dmac  destination mac address.
 * \param  smac  source mac address.
 * \param  mtype   cmdu message type.
 * \param  mid   cmdu message id.
 * \param  buffer   cmdu buffer for sending.
 * \return error code
 */
int cmdu_send(struct p1905_managerd_ctx *ctx, unsigned char *dmac,
                     unsigned char *smac, msgtype mtype, unsigned short mid,
                     unsigned char *buffer, unsigned char *ifname)
{
    struct ethhdr *eth_hdr;
    cmdu_message_header *msg_hdr;
    unsigned char *msg;
    unsigned short msg_leng = 0;
    unsigned short len;
    unsigned char *tlv;
	struct agent_list_db *agent = NULL;
	unsigned char *pmac = NULL;
	int sock = 0;
	unsigned int i = 0;
	unsigned char zero_mac[ETH_ALEN] = {0};

	for (i = 0; i < ctx->itf_number; i++) {
		if (!memcmp(ifname, ctx->itf[i].if_name, strlen((char *)ifname))) {
			sock = ctx->sock_inf_recv_1905[i];
			pmac = ctx->itf[i].mac_addr;
			break;
		}
	}
	if (i >= ctx->itf_number) {
		sock = ctx->sock_br;
		pmac = (unsigned char*)ctx->br_addr;
	}

    eth_hdr = (struct ethhdr*)buffer;
    memcpy(eth_hdr->h_dest, dmac, ETH_ALEN);
    memcpy(eth_hdr->h_source, smac, ETH_ALEN);
    eth_hdr->h_proto = host_to_be16(ETH_P_1905);

    /*start position of message header*/
    msg = buffer + ETH_HLEN;
    msg_hdr = (cmdu_message_header*)(msg);

    /*start position of tlvs */
    tlv = msg + CMDU_HLEN;

    switch(mtype)
    {
			case e_topology_discovery:
				msg_leng = create_topology_discovery_message(ctx, msg,
					ctx->p1905_al_mac_addr, pmac, 0, NULL);
				break;
			case e_topology_discovery_with_vendor_ie:
				{
					unsigned char vs_info[16] = {0};
					unsigned short vs_len = 0;

					vs_len = create_vs_info_for_specific_discovery(vs_info);

					msg_leng = create_topology_discovery_message(ctx, msg,
                           ctx->p1905_al_mac_addr, pmac, vs_len, vs_info);
					if (msg_leng == 0)
						return 0;
				}
				break;
			case e_vendor_specific_topology_discovery:
				{
					unsigned char vs_info[16] = {0};
					unsigned short vs_len = 0;

					vs_len = create_vs_info_for_specific_discovery(vs_info);

					msg_leng = create_vendor_specific_topology_discovery_message(ctx, msg,
						ctx->p1905_al_mac_addr, pmac, vs_len, vs_info);
					if (msg_leng == 0)
						return 0;
				}
                break;
			case e_topology_notification:
				msg_leng = create_topology_notification_message(ctx, msg,
					ctx->p1905_al_mac_addr, &ctx->sinfo.sassoc_evt,
					ctx->sta_notifier, 0, NULL);
				break;
			case e_topology_notification_with_vendor_ie:
				{
					unsigned char vs_info[32] = {0};
					unsigned short vs_len = 0;

					vs_len = create_vs_info_for_specific_notification(ctx, vs_info);
					msg_leng = create_topology_notification_message(ctx, msg,
						ctx->p1905_al_mac_addr, &ctx->sinfo.sassoc_evt,
						ctx->sta_notifier, vs_len, vs_info);
				}
                break;
			case e_topology_query:
				msg_leng = create_topology_query_message(msg, ctx);
                break;
            case e_topology_response:
                msg_leng = create_topology_response_message(msg,
                           ctx->p1905_al_mac_addr, ctx, ctx->br_cap,
                           ctx->p1905_neighbor_dev, ctx->non_p1905_neighbor_dev,
                           &(ctx->topology_entry.tprdb_head), ctx->service,
                           &(ctx->ap_cap_entry.oper_bss_head),
                           &(ctx->ap_cap_entry.assoc_clients_head), ctx->cnt);
                break;
            case e_vendor_specific:
                msg_leng = create_vendor_specific_message(msg, ctx);
                break;
		case e_vendor_specific_wts_content:
		{
			msg_leng = create_vendor_specific_wts_content_message(msg, ctx);
			debug(DEBUG_TRACE, "e_vendor_specific_wts_content msg_leng=%d mid(%04x)\n", msg_leng, mid);

			if (!msg_leng)
				return 0;
		}
		break;
			case e_link_metric_query:
				msg_leng = create_link_metrics_query_message(ctx, msg,
					ctx->link_metric_query_para.target,
					ctx->link_metric_query_para.type);
				break;
            case e_link_metric_response:
                msg_leng = create_link_metrics_response_message(msg,
                            ctx->link_metric_response_para.target,
                            ctx->link_metric_response_para.type,
                            ctx->p1905_al_mac_addr,
                            ctx->itf, ctx->p1905_neighbor_dev,
                            &(ctx->topology_entry.tpddb_head), ctx);
                break;
            case e_ap_autoconfiguration_search:
                msg_leng = ap_autoconfiguration_search_message(msg, ctx);
                break;
            case e_ap_autoconfiguration_wsc_m1:
                msg_leng = ap_autoconfiguration_wsc_message(msg, ctx, dmac, MESSAGE_TYPE_M1);
                break;
            case e_ap_autoconfiguration_response:
                msg_leng = ap_autoconfiguration_response_message(msg, dmac, ctx);
                break;
            case e_ap_autoconfiguration_wsc_m2:
				msg_leng = ap_autoconfiguration_wsc_message(msg, ctx, dmac, MESSAGE_TYPE_M2);
                break;
            case e_ap_autoconfiguration_renew:
                msg_leng = ap_autoconfiguration_renew_message(msg, ctx);
                break;
			case e_ap_capability_query:
				msg_leng = ap_capability_query_message(msg, ctx);
				break;
			case e_channel_preference_query:
				msg_leng = channel_preference_query_message(msg, ctx);
				break;
			case e_channel_selection_request:
				if (ctx->role == CONTROLLER) {
					find_agent_info(ctx, dmac, &agent);
					if (!agent && !ctx->send_tlv_len) {
						debug(DEBUG_ERROR, "error! no agent info exist && send_tlv_len==0\n");
						break;
					}
					msg_leng = channel_selection_request_message(msg, agent, ctx);
			}
				break;
			case e_combined_infrastructure_metrics:
				if (ctx->role == CONTROLLER)
					msg_leng = combined_infrastructure_metrics_message(msg, ctx, dmac);
				else
					reset_send_tlv(ctx);
				break;
			case e_ap_capability_report:
				msg_leng = ap_capability_report_message(msg, ctx);
				break;
			case e_cli_capability_query:
				msg_leng = cli_capability_query_message(msg, ctx);
				break;
			case e_cli_capability_report:
				msg_leng = cli_capability_report_message(msg, ctx);
				break;
			case e_channel_selection_response:
				msg_leng = channel_selection_response_message(msg, ctx);
				break;
			case e_channel_preference_report:
				msg_leng = channel_preference_report_message(msg, ctx);
                break;
			case e_operating_channel_report:
				msg_leng = operating_channel_report_message(msg, ctx);
				break;
			case e_client_steering_btm_report:
				msg_leng = client_steering_btm_report_message(msg, ctx);
				break;
			case e_steering_completed:
				msg_leng = client_steering_completed_message(msg, ctx);
				break;
			case e_multi_ap_policy_config_request:
				msg_leng = map_policy_config_request_message(msg, ctx, dmac);
				break;
			case e_client_association_control_request:
				msg_leng = client_association_control_request_message(msg, ctx);
				break;
			case e_ap_metrics_query:
				msg_leng = ap_metrics_query_message(msg, ctx);
				break;
			case e_ap_metrics_response:
				msg_leng = ap_metrics_response_message(msg, ctx);
				break;
			case e_associated_sta_link_metrics_query:
				msg_leng = associated_sta_link_metrics_query_message(msg, ctx);
				break;
			case e_associated_sta_link_metrics_response:
				msg_leng = associated_sta_link_metrics_response_message(msg, ctx);
				break;
			case e_unassociated_sta_link_metrics_query:
				msg_leng = unassociated_sta_link_metrics_query_message(msg, ctx);
				break;
			case e_unassociated_sta_link_metrics_response:
				msg_leng = unassociated_sta_link_metrics_response_message(msg, ctx);
				break;
			case e_beacon_metrics_query:
				msg_leng = beacon_metrics_query_message(msg, ctx);
				break;
			case e_beacon_metrics_response:
				msg_leng = beacon_metrics_response_message(msg, ctx);
				break;
			case e_backhaul_steering_response:
				msg_leng = backhaul_steering_response_message(msg, ctx);
				break;
			case e_higher_layer_data:
				msg_leng = high_layer_data_message(msg, ctx);
				break;
			case e_1905_ack:
				msg_leng = _1905_ack_message(msg, ctx);
				break;
			case e_dev_send_1905_request:
				msg_leng = dev_send_1905_msg(msg, ctx);
				break;
			case e_client_steering_request:
				msg_leng = client_steering_request_message(msg, ctx);
				break;
			case e_backhaul_steering_request:
				msg_leng= backhaul_steering_request_message(msg, ctx);
				break;

#ifdef MAP_R2
			/*channel scan feature*/
			case e_channel_scan_request:
				if (ctx->role == CONTROLLER)
					msg_leng = channel_scan_request_message(msg, ctx);
				break;
			case e_channel_scan_report:
				msg_leng = channel_scan_report_message(msg, ctx);
				break;

			case e_tunneled_message:
				msg_leng = tunneled_message(msg, ctx);
				break;

			case e_association_status_notification:
				msg_leng = assoc_status_notification_message(msg, ctx);
				break;

			case e_cac_request:
				msg_leng = cac_request_message(msg, ctx);
				break;

			case e_cac_termination:
				msg_leng = cac_terminate_message(msg, ctx);
				break;

			case e_client_disassociation_stats:
				msg_leng = client_disassciation_stats_message(msg, ctx);
				break;
			case e_backhual_sta_cap_query_message:
					msg_leng = backhual_sta_cap_query_message(msg, ctx);
					break;
			case e_backhual_sta_cap_report_message:
					msg_leng = backhual_sta_cap_report_message(msg, ctx);
					break;
			case e_failed_connection_message:
				msg_leng = failed_connection_message(msg, ctx);
				break;
#endif // #ifdef MAP_R2

#ifdef MAP_R3
			case e_proxied_encap_dpp:
				msg_leng = proxied_encap_dpp_message(msg, ctx);
				break;

			case e_direct_encap_dpp:
				msg_leng = direct_encap_dpp_message(msg, ctx);
				break;

			case e_1905_encap_eapol:
				msg_leng = _1905_encap_eapol_message(msg, ctx);
				break;

			case e_dpp_bootstrapping_uri_notification:
				msg_leng = dpp_bootstrap_uri_notification_message(msg, ctx);
				break;

			case e_dpp_bootstrapping_uri_query:
				msg_leng = dpp_bootstrap_uri_query_message(msg, ctx);
				break;

			case e_dpp_cce_indication:
				msg_leng = dpp_cce_indication_message(msg, ctx);
				break;

            /* Agent may at any time send a BSS Configuration Request */
			case e_bss_configuration_request:
				msg_leng = bss_configuration_request_message(msg, ctx);
				break;

			case e_bss_configuration_response:
				msg_leng = bss_configuration_response_message(msg, ctx, dmac);
				if (msg_leng == 0) {
					/* trigger topology qeury at once as no bss connector match by almac */
					debug(DEBUG_ERROR, "trigger topology query at once to "MACSTR"\n", MAC2STR(dmac));
					insert_cmdu_txq(dmac, ctx->p1905_al_mac_addr, e_topology_query, ++ctx->mid, ifname, 0);
					return 0;
				}
				/* reset position of message header*/
				buffer = ctx->trx_buf.buf;
				msg = buffer + ETH_HLEN;
				msg_hdr = (cmdu_message_header *)(msg);
				break;

			case e_bss_configuration_result:
				msg_leng = bss_configuration_result_message(msg, ctx, dmac);
				break;

			case e_ptk_rekey_request:
				msg_leng = _1905_rekey_request_message(msg, ctx);
				break;

            case e_decryption_failure:
				msg_leng = _1905_decryption_failure_message(ctx, msg, dmac);
				break;

            /*
            ** 1. this msg should be sent by ucc in raw data
            ** 2. this msg should be send after bss config changed while turnkey
            */
            case e_bss_reconfiguration_trigger:
				msg_leng = bss_reconfiguration_trigger_message(msg, ctx);
				break;

            case e_agent_list:
                msg_leng = agent_list_message(msg, ctx, dmac);
                break;

			case e_chirp_notification:
                msg_leng = chirp_notification_message(msg, ctx);
                break;
#endif // MAP_R3
#ifdef MAP_R3_SP
	case e_service_prioritization_request:
		msg_leng = create_sp_request_message(get_tlv_buffer(ctx),
			msg, &ctx->sp_rule_static_list, &ctx->sp_rule_dynamic_list,
			&ctx->sp_rule_del_list, &ctx->sp_dp_table, ctx->sp_req_conf.action, dmac);
		break;
	case e_service_prioritization_request_4_learning_comp:
		msg_leng = create_sp_request_message(get_tlv_buffer(ctx),
			msg, NULL, &ctx->sp_rule_learn_complete_list,
			NULL, NULL, ADD_RULE, dmac);
		break;
#endif
            default:
                break;
    }

	/* reset header as buffer maybe realloced */
	eth_hdr = (struct ethhdr *)buffer;
	os_memcpy(eth_hdr->h_dest, dmac, ETH_ALEN);
	os_memcpy(eth_hdr->h_source, smac, ETH_ALEN);
	eth_hdr->h_proto = host_to_be16(ETH_P_1905);

	/*start position of tlvs */
	tlv = msg + CMDU_HLEN;

	/*move tlv_temp to buffer*/
	os_memcpy(tlv, get_tlv_buffer(ctx), msg_leng);
	len = msg_leng + CMDU_HLEN + ETH_HLEN;
#ifdef MAP_R3
	/* testbed need buffer message[include all tlvs] which ucc specified */
	if (ctx->MAP_Cer && (MAP_PROFILE_R3 == ctx->map_version))
		dev_buffer_frame(ctx, be_to_host16(msg_hdr->message_type), mid, tlv, msg_leng);
#endif

    msg_hdr->message_id = host_to_be16(mid);
    /* temporarily set the fragment id & last fragment ind
     	  * these two variables will be updated.
        */
    msg_hdr->fragment_id = 0;
    msg_hdr->last_fragment_indicator = 1;

	/*recheck relay indicator*/
	if (msg_hdr->relay_indicator == 0x1) {
		if (memcmp(dmac, p1905_multicast_address, ETH_ALEN)) {
			msg_hdr->relay_indicator = 0x0;
		}
	}

	/*insert message into retry queue to monitor if 1905_ack is received from the peer*/
	if (memcmp(dmac, zero_mac, ETH_ALEN)) {
		if (exist_in_retry_message_list(be_to_host16(msg_hdr->message_type))) {
			insert_retry_message_queue(dmac, be_to_host16(msg_hdr->message_id),
				be_to_host16(msg_hdr->message_type), RETRY_CNT,
				sock, ifname, buffer, len);
		}
	}

#ifdef MAP_R3
	/* not perform encryption procedure for the following messages
	**	1905 AP-Autoconfiguration Search
	**	1905 AP-AutoconfigurationResponse
	**	Direct Encap DPP or 1905 Encap EAPOL*/
	if (ctx->map_version == MAP_PROFILE_R3) {
		unsigned char code = 0;
		struct security_entry *entry = NULL;
		if ((be_to_host16(msg_hdr->message_type) != _1905_ENCAP_EAPOL) &&
			(be_to_host16(msg_hdr->message_type) != DIRECT_ENCAP_DPP_MESSAGE) &&
			(be_to_host16(msg_hdr->message_type) != AP_AUTOCONFIG_SEARCH) &&
			(be_to_host16(msg_hdr->message_type) != AP_AUTOCONFIG_RESPONSE)) {
			/* buffer: whole msg
			** len total len include eth hdr and cmdu hdr
			*/
			entry = sec_entry_lookup(dmac);
			if (entry) {
				code = sec_encrypt(dmac, smac, buffer, &len, 0);
				if (code != SUC) {
					debug(DEBUG_ERROR, "encry msg (0x%04x) error: %s to "MACSTR"\n",
						be_to_host16(msg_hdr->message_type), code2string(code), PRINT_MAC(dmac));
					return 0;
				}
				if (is_security_on(dmac))
					security_hex_dump(SEC_DEBUG_TRACE, "send msg", buffer, len);
				msg_leng = len - CMDU_HLEN - ETH_HLEN;
			}
		}
	}
#endif // #ifdef MAP_R3

	/*exceed the max frame size, do need to fragment TX*/
	if (msg_leng > MAX_TLVS_LENGTH) {
		debug(DEBUG_TRACE, "mtype(0x%04x) mlen(%d) on %s\n",
			be_to_host16(msg_hdr->message_type), msg_leng, ifname);
		if (cmdu_tx_fragment(ctx, buffer, len, ifname) < 0) {
			debug(DEBUG_ERROR, "send fragment failed on %s (%s)\n", ifname, strerror(errno));
			return -1;
		}
	} else {
		/*send to local interface*/
		if (send(sock, buffer, len, 0) < 0) {
			if (errno == ENETDOWN) {
				debug(DEBUG_ERROR, "send failed on %s (%s)\n", ifname, strerror(errno));
				return 0;
			}
			return -1;
		}
		debug(DEBUG_TRACE,"msgtype=0x%04x, msg_hdr->message_id=0x%04x interface=%s\n",
			be_to_host16(msg_hdr->message_type), be_to_host16(msg_hdr->message_id), ifname);
#if 0
		if (be_to_host16(msg_hdr->message_type) == CHANNEL_PREFERENCE_REPORT)
			hex_dump_all("CHANNEL_PREFERENCE_REPORT", buffer, len);
#endif
	}

	return 0;
}




/**
 *  for cmdu message relay.
 *
 * \param  ctx  p1905_managerd_ctx context.
 * \param  buf  pointer of buffer for tx.
 * \param  len  transmit length.
 * \return error code
 */
int cmdu_bridge_relay(struct p1905_managerd_ctx *ctx,
                             unsigned char *buf, int len, unsigned int if_index)
{
    int length = 0;

	length = send(ctx->sock_inf_recv_1905[if_index], buf, len, 0);

    if (0 > length) {
		if (errno == ENETDOWN) {
			debug(DEBUG_ERROR, "send failed on %s (%s)\n", (char *)ctx->itf[if_index].if_name,
				strerror(errno));
			return 0;
		}
        return -1;
    }

    return 0;

}

/**
 *  for cmdu message relay.
 *
 * \param  ctx  p1905_managerd_ctx context.
 * \param  buf  pointer of buffer for tx.
 * \param  len  transmit length.
 * \return error code
 */
int cmdu_bridge_relay_send(struct p1905_managerd_ctx *ctx,
            unsigned char *smac, unsigned char *buf, int len)
{
    unsigned int i = 0;

	for (i = 0; i < ctx->itf_number; i++) {
        /*do not relay to original receive interface*/
        if (i != ctx->recent_cmdu_rx_if_number) {
            if (0 > cmdu_bridge_relay(ctx, buf, len, i))
                return -1;
        }
    }

    return 0;
}

/**
 *  receive cmdu message from virtual interface.
 *
 * \param  ctx  p1905_managerd_ctx context.
 * \param  buf  pointer of buffer for rx.
 * \param  len  max receive length.
 * \return error code
 */
int cmdu_virtual_if_receive(struct p1905_managerd_ctx *ctx, int sock, unsigned char *buf, int len)
{
	struct sockaddr_un from;
	int lenfrom = sizeof(from);
	int recv_fd = 0;

    recv_fd = accept(ctx->sock_inf_recv_1905[FIRST_VITUAL_ITF], (struct sockaddr *)&from, (socklen_t *)&lenfrom);
    if(recv_fd < 0)
    {
        debug(DEBUG_ERROR, "accept fail\n");
        return -1;
    }

	len = recv(recv_fd, buf, len, 0);

    if(0 >= len)
    {
        debug(DEBUG_ERROR, "recv fail\n");
        close(recv_fd);
        return -1;
    }
	close(recv_fd);

	buf[len] = '\0';

    return len;
}

/**
 *  receive cmdu message from socket.
 *
 * \param  ctx  p1905_managerd_ctx context.
 * \param  buf  pointer of buffer for rx.
 * \param  len  max receive length.
 * \return error code
 */
int cmdu_bridge_receive(struct p1905_managerd_ctx *ctx, int sock, unsigned char *buf, int len)
{

    len = recv(sock, buf, len, 0);
    if(0 >= len)
    {
        debug(DEBUG_WARN, "receive failed on %s (%s)", ctx->br_name,
               strerror(errno));
        return -1;
    }

    return len;
}

/**
 *  parse cmdu.
 *
 * \param  ctx  p1905_managerd_ctx context.
 * \param  buf  pointer of buffer for rx.
 * \param  len  receive length.
 * \return error code
 */
int cmdu_parse(struct p1905_managerd_ctx *ctx, unsigned char *buf, int len)
{
	struct ethhdr *eth_hdr;
	unsigned char *temp_buf;

	eth_hdr = (struct ethhdr *)buf;

	if (cmdu_recv_intf_check(ctx) < 0)
		return -1;

	/* examine the dst mac address to decide if we need to parse this packet */
	if (!cmdu_dst_mac_check(ctx, eth_hdr->h_dest))
		return -1;

	/* examine the received cmdu is from us, then drop it */
	if (cmdu_src_mac_check(ctx, eth_hdr->h_source))
		return -1;

	/* shift to cmdu message start position */
	temp_buf = buf;
	temp_buf += ETH_HLEN;

	if (parse_cmdu_message(ctx, temp_buf, eth_hdr->h_dest, eth_hdr->h_source, len) < 0) {
		debug(DEBUG_ERROR, "parse_cmdu_message fail\n");
		return -1;
	}

	if (ctx->need_relay) {
		ctx->need_relay = 0;
		/*change to our almac, because some device may drop multicast not sending by neighbor*/
		memcpy(eth_hdr->h_source, ctx->p1905_al_mac_addr, ETH_ALEN);
#ifdef MAP_R3
		if (ctx->map_version == MAP_PROFILE_R3) {
			unsigned char code = 0;
			unsigned short length = 0;
			struct security_entry *entry = NULL;

			length = len;

			entry = sec_entry_lookup(eth_hdr->h_dest);
			if (entry) {
				code = sec_encrypt(eth_hdr->h_dest, eth_hdr->h_source,
									buf, &length, 0);
				if (code != SUC) {
					debug(DEBUG_ERROR, "encry error: %s to "MACSTR"\n",
					code2string(code), PRINT_MAC(eth_hdr->h_dest));
					return -1;
				}
				len = length;
				if (is_security_on(eth_hdr->h_dest))
					security_hex_dump(SEC_DEBUG_TRACE, "send relay msg", buf, len);
			}
		}
#endif
		if (cmdu_bridge_relay_send(ctx, eth_hdr->h_source, buf, len) < 0) {
			debug(DEBUG_ERROR, "cmdu_bridge_relay_send fail\n");
			return -1;
		}
	}

	return 0;
}

