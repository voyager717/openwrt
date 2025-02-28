/*
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2018, Mediatek, Inc.
 * *
 * * All rights reserved. Mediatek's source code is an unpublished work and the
 * * use of a copyright notice does not imply otherwise. This source code
 * * contains confidential trade secret material of Ralink Tech. Any attemp
 * * or participation in deciphering, decoding, reverse engineering or in any
 * * way altering the source code is stricitly prohibited, unless the prior
 * * written consent of Mediatek, Inc. is obtained.
 * ***************************************************************************
 *
 *  Module Name:
 *  Channel planning R2*
 *  Abstract:
 *  Channel Planning
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  * */

#include "includes.h"
#ifdef __linux__
#include <fcntl.h>
#endif				/* __linux__ */

#include "common.h"
#include <sys/un.h>
#include "interface.h"
#include "data_def.h"
#include "1905_map_interface.h"
#include "client_db.h"
#include "mapd_i.h"
#include "topologySrv.h"
#include "eloop.h"
#include "tlv_parsor.h"
#include "1905_if.h"
#include "wapp_if.h"
#include "mapd_debug.h"
#include "chan_mon.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "apSelection.h"
#include "ch_planning.h"

extern struct oper_class_map global_op_class[];
extern u8 grp_bw_160[MAX_BW_160_BLOCK][8];
extern u8 grp_bw_80[MAX_BW_80_BLOCK][4];
extern u8 grp_bw_40[MAX_BW_40_BLOCK][2];

#ifdef MAP_R2 
void ch_planning_R2_bootup_handling_restart(
	void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *ctx = eloop_ctx;
	uint8_t radio_idx = 0;
#ifndef MAP_6E_SUPPORT
	struct mapd_radio_info *radio_info = NULL;
#endif
	err(CH_PLANING_PREX"Restart Bootup Chnl Plan");
	for (radio_idx = 0; radio_idx < MAX_NUM_OF_RADIO; radio_idx++) {
#ifndef MAP_6E_SUPPORT
		radio_info = &ctx->dev_radio_info[radio_idx];
		if (radio_info->radio_idx == (uint8_t)-1)
			continue;
		if (radio_info->bootup_run == BOOTUP_SCAN_ERROR)
			radio_info->bootup_run = BOOTUP_SCAN_NEEDED;
#else
		if (ctx->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run == BOOTUP_SCAN_ERROR)
			ctx->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run = BOOTUP_SCAN_NEEDED;
#endif
	}
}
u8 find_radio_num(
	struct own_1905_device *ctx,
	struct radio_info_db *radio)
{
	u8 radio_num = 0;
	for(radio_num = 0;radio_num < MAX_NUM_OF_RADIO; radio_num++) {
		if (radio->band == ctx->ch_planning_R2.ch_plan_thres[radio_num].band)
			break;
	}
	return radio_num;
}

void ch_planning_get_avg (struct radio_info_db *radio)
{
	struct dev_ch_monitor *monitor_info =
		&radio->dev_ch_plan_info.dev_ch_monitor_info;
	if ((monitor_info->count_cu_util < MIN_SAMPLE_COUNT)
		||((monitor_info->count_radio_metric < MIN_SAMPLE_COUNT))) {
		err(CH_PLANING_PREX" count less than min sample count avoid this radio in analysis");
		monitor_info->avg_cu_monitor = 0;
		monitor_info->avg_edcca = 0;
		monitor_info->avg_obss_load = 0;
		monitor_info->avg_myTxAirtime = 0;
		monitor_info->avg_myRxAirtime = 0;
		return;
	}
	if (monitor_info->count_cu_util > 0) {
	monitor_info->avg_cu_monitor =
		(monitor_info->avg_cu_monitor /
		monitor_info->count_cu_util);
	} else {
		monitor_info->avg_cu_monitor = 0; 
		err(CH_PLANING_PREX" avg_cu_monitor=0 as count_cu_util is %d",monitor_info->count_cu_util);
	}
	if (monitor_info->count_edcca_cu_tlv > 0){
	monitor_info->avg_edcca =
		(monitor_info->avg_edcca /
			monitor_info->count_edcca_cu_tlv);
	} else {
		monitor_info->avg_edcca= 0;
		err(CH_PLANING_PREX" avg_edcca=0 as count_edcca_cu_tlv is %d", monitor_info->count_edcca_cu_tlv);
	}
	if (monitor_info->count_radio_metric > 0){

	monitor_info->avg_obss_load =
		(monitor_info->avg_obss_load /
			monitor_info->count_radio_metric);

	monitor_info->avg_myTxAirtime =
		(monitor_info->avg_myTxAirtime /
			monitor_info->count_radio_metric);

	monitor_info->avg_myRxAirtime =
		(monitor_info->avg_myRxAirtime /
			monitor_info->count_radio_metric);
	} else {
		monitor_info->avg_obss_load = 0;
		monitor_info->avg_myTxAirtime = 0;
		monitor_info->avg_myRxAirtime = 0;
		err(CH_PLANING_PREX" avg_myTxAirtime,avg_obss_load,avg_myRxAirtime = 0 as count_radio_metric is %d", monitor_info->count_radio_metric);
	}
}
s32 CalculateCompareOBSS(
	struct own_1905_device *ctx,
	struct _1905_map_device *_1905_device,
	struct monitor_ch_info *ch_info)
{
	u32 TotalMapTxRxAirtime = 0;
	s32 CompareObss = 0;
	struct radio_info_db *temp_radio = NULL;
	struct _1905_map_device *temp_1905_device = NULL;
	temp_1905_device = topo_srv_get_1905_device(ctx, NULL);
	while(temp_1905_device) {
		if(temp_1905_device == _1905_device) {
			temp_1905_device = topo_srv_get_next_1905_device(ctx, temp_1905_device);
			continue;
		}
		temp_radio = topo_srv_get_radio_by_channel(temp_1905_device, ch_info->channel_num);
		if(temp_radio) {
			TotalMapTxRxAirtime = TotalMapTxRxAirtime +
			(temp_radio->dev_ch_plan_info.dev_ch_monitor_info.avg_myTxAirtime +
				temp_radio->dev_ch_plan_info.dev_ch_monitor_info.avg_myRxAirtime);
			err("for dev "MACSTR" avg_myTxAirtime %d avg_myRxAirtime %d",MAC2STR(temp_1905_device->_1905_info.al_mac_addr),
				temp_radio->dev_ch_plan_info.dev_ch_monitor_info.avg_myTxAirtime,
				temp_radio->dev_ch_plan_info.dev_ch_monitor_info.avg_myRxAirtime);
		}
		
		temp_1905_device = topo_srv_get_next_1905_device(ctx,temp_1905_device);
	}
	err("for all dev  TotalMapTxRxAirtime %d", TotalMapTxRxAirtime);
	temp_radio = topo_srv_get_radio_by_channel(_1905_device,ch_info->channel_num);
	if(temp_radio) {
		err("avg_obss_load %d",temp_radio->dev_ch_plan_info.dev_ch_monitor_info.avg_obss_load);
		CompareObss = (temp_radio->dev_ch_plan_info.dev_ch_monitor_info.avg_obss_load
				- TotalMapTxRxAirtime);
	}
	return CompareObss;
}
void dump_ch_planning_update_data(struct radio_info_db *radio)
{
	struct dev_ch_monitor *dev_monitor_info =
			&radio->dev_ch_plan_info.dev_ch_monitor_info;
	always("avg_cu_monitor %d\n",dev_monitor_info->avg_cu_monitor);
	always("avg_edcca %d\n",dev_monitor_info->avg_edcca);
	always("avg_obss_load %d\n",dev_monitor_info->avg_obss_load);
	always("avg_myTxAirtime %d\n",dev_monitor_info->avg_myTxAirtime);
	always("avg_myRxAirtime %d\n",dev_monitor_info->avg_myRxAirtime);
	always("count ch_util %d\n",dev_monitor_info->count_cu_util);
	always("count edcca tlv %d\n",dev_monitor_info->count_edcca_cu_tlv);
	always("count radio metric tlv %d\n",dev_monitor_info->count_radio_metric);
}

void ch_planning_all_dev_avg(struct own_1905_device *ctx,
	struct monitor_ch_info *ch_info)
{
	struct _1905_map_device *_1905_device = NULL;
	struct radio_info_db *radio = NULL;

	_1905_device = topo_srv_get_1905_device(ctx, NULL);
	while(_1905_device) {
		if (!_1905_device->in_network) {
			_1905_device = topo_srv_get_next_1905_device(ctx, _1905_device);
			continue;
		}
		radio =
			topo_srv_get_radio_by_channel(_1905_device, ch_info->channel_num);
		if(radio) {
			ch_planning_get_avg(radio);
			err("For dev "MACSTR"", MAC2STR(_1905_device->_1905_info.al_mac_addr));
			dump_ch_planning_update_data(radio);
		}
	_1905_device = topo_srv_get_next_1905_device(ctx, _1905_device);
	}
	

}
u32 ch_planning_get_max_edcca_airtime(
	struct own_1905_device *ctx,
	struct monitor_ch_info *ch_info)
{
	struct _1905_map_device *_1905_device = NULL;
	struct radio_info_db *radio = NULL;
	u32 Max_EdccaAirtime = 0;
	_1905_device = topo_srv_get_1905_device(ctx, NULL);
	while(_1905_device) {
		radio = topo_srv_get_radio_by_channel(_1905_device, ch_info->channel_num);
		if(radio) {
			if(radio->dev_ch_plan_info.dev_ch_monitor_info.avg_edcca > Max_EdccaAirtime)
				Max_EdccaAirtime = radio->dev_ch_plan_info.dev_ch_monitor_info.avg_edcca;
		}
		_1905_device = topo_srv_get_next_1905_device(ctx, _1905_device);
	}
	return Max_EdccaAirtime;
}

u8 ch_planning_is_trigger_req(
	struct own_1905_device *ctx,
	struct monitor_ch_info *ch_info)
{
	struct _1905_map_device *_1905_device = NULL;
	u8 need_trigger = TRIGGER_FALSE;
	u32 Max_EdccaAirtime = 0;
	struct radio_info_db *radio = NULL;
	u32 EdccaThreshold = EDCCA_THRESHOLD;
	u32 CompareEdcca = 0;
	s32 ObssThreshold = OBSS_THRESHOLD;
	s32 CompareObss = 0 ;
	s32 BkLoadThreshold = BKLOAD_THRESHOLD, CompareBkLoad = 0;
	u8 skip = ctx->ch_planning_R2.skip_edcca_check;
	u8 radio_num = 0;
	_1905_device = topo_srv_get_1905_device(ctx, NULL);
	radio = topo_srv_get_radio_by_channel(_1905_device,ch_info->channel_num);
	if(!radio){
		err(CH_PLANING_PREX"radio is NULL");
	}
	else {
		radio_num = find_radio_num(ctx, radio);
		EdccaThreshold = ctx->ch_planning_R2.ch_plan_thres[radio_num].edcca_threshold;
		ObssThreshold = (s32)ctx->ch_planning_R2.ch_plan_thres[radio_num].obss_load_threshold;
		
	}
	err(CH_PLANING_PREX"Trigger thresholds are EDCCA %d,OBSS %d",EdccaThreshold, ObssThreshold);
	debug(CH_PLANING_PREX"skip %d",skip);
	if (skip == 0) {
		//sonal test later when correct edcca value comes
		Max_EdccaAirtime = ch_planning_get_max_edcca_airtime(ctx, ch_info);
		CompareEdcca = Max_EdccaAirtime;
		if(CompareEdcca > EdccaThreshold) {
			err(CH_PLANING_PREX" There is strong non-WIFI interference, launch scan mandate");
			need_trigger = TRIGGER_TRUE;
			return need_trigger;
		}
	}

	while(_1905_device) {
		CompareObss = CalculateCompareOBSS(ctx, _1905_device, ch_info);
		if(CompareObss < 0) {
			err(CH_PLANING_PREX"for this DEV ASSUMPTION that ObssAirtime_A2 > (MyTxAirtime_A1 + MyRxAirtime_A1) is wrong");
		}
		CompareBkLoad = CompareEdcca + CompareObss;
		err(CH_PLANING_PREX"CompareObss %d CompareBkLoad %d", CompareObss, CompareBkLoad);
		/*If any agent exceeds threshold ,
		then also we will go for trigger ch planning on that radio*/
		if(CompareObss > ObssThreshold) {
			err(CH_PLANING_PREX"There is large WiFi traffic not belongs to MyMAP Network");
			need_trigger = TRIGGER_TRUE;
		} else if ((CompareBkLoad > BkLoadThreshold) && (skip == 0)) {
			err(CH_PLANING_PREX"There is large background load not belongs to MyMAP Network");
			need_trigger = TRIGGER_TRUE;
		}
		if(need_trigger)
			break;
		_1905_device = topo_srv_get_next_1905_device(ctx, _1905_device);
	}
	return need_trigger;
}
u8 ch_planning_check_trigger(
	struct own_1905_device *ctx,
	struct monitor_ch_info *ch_info)
{
	u8 need_trigger = TRIGGER_FALSE;

	/*Check TRIGGER is to be made PENDING*/
	if((ctx->network_optimization.network_opt_state != NETOPT_STATE_IDLE) ||
			(ctx->ch_planning_R2.ch_plan_state >= CHPLAN_STATE_SCAN_ONGOING) ||
			(ctx->user_triggered_scan == 1)) {
		err(CH_PLANING_PREX"state mismatch!!! DEFER CH PLANNING");
		need_trigger = TRIGGER_PENDING;
		return need_trigger;
	}
	/*Check TRIGGER is to be TRUE or FALSE*/
	/*Part 1 -> Take AVG of all devices , monitored radio*/
	err(CH_PLANING_PREX"Take Monitored value AVG");
	ch_planning_all_dev_avg(ctx, ch_info);

	err(CH_PLANING_PREX"Compare Monitor Avg with Threshold");
	/*Part 2 ->Calculate and compare with thresholds*/
	need_trigger = ch_planning_is_trigger_req(ctx, ch_info);
	err(CH_PLANING_PREX"need_trigger %d",need_trigger);
	return need_trigger;
}
u32 get_scan_wait_time(
	struct _1905_map_device *_1905_device,
	struct radio_info_db *radio)
{
/*can't trigger scan if last scan for any agent was earlier than its min_scan_interval*/
	u32 scan_wait_time = 0;
	struct os_time now;
	os_get_time(&now);
	if(now.sec > (radio->dev_ch_plan_info.last_report_timestamp.sec
				+ radio->radio_scan_params.min_scan_interval)) {
		scan_wait_time = 0;
	} else {
		scan_wait_time = radio->radio_scan_params.min_scan_interval
				- (now.sec - radio->dev_ch_plan_info.last_report_timestamp.sec);
	}
	return scan_wait_time;
}
void ch_planning_scan_wait_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct _1905_map_device *dev = (struct _1905_map_device *)timeout_ctx;
	struct own_1905_device *ctx = (struct own_1905_device *)eloop_ctx;
	struct radio_info_db *temp_radio = NULL;
	struct monitor_ch_info *ch_info = NULL, *t_ch_info = NULL;

	SLIST_FOREACH_SAFE(ch_info, &ctx->ch_planning_R2.first_monitor_ch, next_monitor_ch, t_ch_info) {
		temp_radio = topo_srv_get_radio_by_channel(dev, ch_info->channel_num);
		if (temp_radio)
			break;
	}
	if(temp_radio)
		ch_planning_send_scan_req(ctx, dev, temp_radio);

}

void ch_planning_mark_boot_ch_state(
	struct own_1905_device *own_dev,
	struct monitor_ch_info *ch_info)
{
	if (ch_info->channel_num > 14)
		own_dev->scan_done_5g = 1;
	else if (ch_info->channel_num <= 14)
		own_dev->scan_done_2g = 1;
}

void ch_scan_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct monitor_ch_info *ch_info = (struct monitor_ch_info *)timeout_ctx;
	struct own_1905_device *own_dev = (struct own_1905_device *)eloop_ctx;
	struct mapd_global *global = (struct mapd_global *)own_dev->back_ptr;

	/*check if more than 50% agents have sent scan report.
	If yes, then trigger channel planning logic. find the agent count that support this radio
	number of agents that support that radio compare with total num of scan results
	*/
	if (ch_planning_check_scan_result(own_dev, ch_info)) {
		err(CH_PLANING_PREX"HALF scan reports received go for best channel selection");
		os_get_time(&global->dev.channel_planning_last_scan_timestamp);
		ch_planning_mark_boot_ch_state(own_dev, ch_info);
		ch_planning_select_best_channel_R2(own_dev->back_ptr, ch_info);
		/*still some agents have not sent scan results , so clean complete scan list entry */
		ch_planning_remove_scan_list(own_dev);
		own_dev->ch_planning_R2.retry_count = 0;
	} else {
		if (own_dev->ch_planning_R2.retry_count == 1) {
			/* If we reach here it means we have not received more than 50% result
			after two attempts, need to proceed with whatever result we have */
			os_get_time(&global->dev.channel_planning_last_scan_timestamp);
			ch_planning_mark_boot_ch_state(own_dev, ch_info);
			ch_planning_select_best_channel_R2(own_dev->back_ptr, ch_info);
			ch_planning_remove_scan_list(own_dev);
			own_dev->ch_planning_R2.retry_count = 0;
		} else {
			err(CH_PLANING_PREX" re-trigger channel monitor on ch %d", ch_info->channel_num);
			/* re-trigger channel monitor.*/
			own_dev->ch_planning_R2.retry_count++;
			own_dev->ch_planning_R2.ch_plan_state = CHPLAN_STATE_MONITOR;
			ch_planning_update_all_dev_state((u8)CHPLAN_STATE_MONITOR,ch_info->channel_num,own_dev);
			/* if 5min scan timedout and we have not received 50% of scan result
			then retrigger scan immediately only one more time */
			eloop_register_timeout(0, 0, channel_monitor_timeout, own_dev, ch_info);
			/*change policy for AP metric reporting for all devices on that channel*/
			ch_planning_update_ap_metric_policy(own_dev, ch_info, 0);
		}
	}
}

void ch_scan_timeout_mtk(void *eloop_ctx, void *timeout_ctx)
{
	struct monitor_ch_info *ch_info = (struct monitor_ch_info *)timeout_ctx;
	struct own_1905_device *own_dev = (struct own_1905_device *)eloop_ctx;

	/*check if more than 50% agents have sent scan report.
	If yes, then trigger channel planning logic. find the agent count that support this radio
	number of agents that support that radio compare with total num of scan results
	*/
	if (ch_planning_check_scan_result(own_dev, ch_info)) {
		err(CH_PLANING_PREX"HALF scan reports received go for best channel selection");
		ch_planning_select_best_channel_R2(own_dev->back_ptr, ch_info);
		/*still some agents have not sent scan results , so clean complete scan list entry */
		ch_planning_mark_boot_ch_state(own_dev, ch_info);
		ch_planning_remove_scan_list(own_dev);
		eloop_cancel_timeout(ch_scan_timeout, own_dev, ch_info);
		own_dev->ch_planning_R2.retry_count = 0;
	}

}

void ch_planning_remove_radio_scan_results(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{
	struct scan_result_tlv *scan_list_entry = NULL;
	struct nb_info *neigh = NULL;
	struct radio_info_db *radio = NULL, *t_radio = NULL;
	/*If user triggered scan is true remove all scan results ,
	if ch planning algo triggers then only delete for particular radio*/
	SLIST_FOREACH_SAFE(radio, &dev->first_radio, next_radio, t_radio) {
		if(ctx->user_triggered_scan == FALSE &&
			radio->dev_ch_plan_info.dev_ch_plan_state != CHPLAN_STATE_SCAN_ONGOING)
			continue;
		while (!SLIST_EMPTY(&(radio->first_scan_result))) {
			scan_list_entry = SLIST_FIRST(&(radio->first_scan_result));
			while (!SLIST_EMPTY(&(scan_list_entry->first_neighbor_info))) {
				neigh = SLIST_FIRST(&(scan_list_entry->first_neighbor_info));
				SLIST_REMOVE_HEAD(&(scan_list_entry->first_neighbor_info), next_neighbor_info);
				os_free(neigh);
			}
			SLIST_REMOVE_HEAD(&(radio->first_scan_result), next_scan_result);
			os_free(scan_list_entry);
		}
		if (SLIST_EMPTY(&(radio->first_scan_result))) {
			SLIST_INIT(&(radio->first_scan_result));
		}
	}
}

void ch_planning_remove_all_scan_results(
	struct _1905_map_device *dev)
{
	struct scan_result_tlv *scan_list_entry = NULL;
	struct nb_info *neigh = NULL;
	struct radio_info_db *radio = NULL, *t_radio = NULL;
	SLIST_FOREACH_SAFE(radio, &dev->first_radio, next_radio, t_radio) {
		while (!SLIST_EMPTY(&(radio->first_scan_result))) {
			scan_list_entry = SLIST_FIRST(&(radio->first_scan_result));
			while (!SLIST_EMPTY(&(scan_list_entry->first_neighbor_info))) {
				neigh = SLIST_FIRST(&(scan_list_entry->first_neighbor_info));
				SLIST_REMOVE_HEAD(&(scan_list_entry->first_neighbor_info), next_neighbor_info);
				os_free(neigh);
			}
			SLIST_REMOVE_HEAD(&(radio->first_scan_result), next_scan_result);
			os_free(scan_list_entry);
		}
	}
}
void ch_planning_remove_scanlist_entry(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
	struct scan_list_info *scan_list, *t_scan_list = NULL;
	SLIST_FOREACH_SAFE(scan_list, &ctx->ch_planning_R2.first_scan_list, next_scan_list, t_scan_list) {
		if(os_memcmp(scan_list->al_mac, dev->_1905_info.al_mac_addr, ETH_ALEN) == 0 &&
			os_memcmp(scan_list->radio_identifier, radio->identifier, ETH_ALEN) == 0) {
			SLIST_REMOVE(&(ctx->ch_planning_R2.first_scan_list),
				scan_list,
				scan_list_info,
				next_scan_list);
			os_free(scan_list);
			break;
		}
	}
}
void ch_planning_insert_scanlist_entry(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
	struct scan_list_info *scan_list = os_zalloc(sizeof(struct scan_list_info));
	if (scan_list == NULL) {
		err("alloc memory fail");
		return;
	}
	os_memcpy(scan_list->al_mac, dev->_1905_info.al_mac_addr, ETH_ALEN);
	os_memcpy(scan_list->radio_identifier, radio->identifier, ETH_ALEN);
	scan_list->trigger_done = 0;
	SLIST_INSERT_HEAD(&ctx->ch_planning_R2.first_scan_list,
		scan_list, next_scan_list);
}

void ch_planning_update_state_scan_done(
	struct own_1905_device *own_dev,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
	if(radio->dev_ch_plan_info.dev_ch_plan_state == CHPLAN_STATE_SCAN_ONGOING) {
		os_get_time(&radio->dev_ch_plan_info.last_report_timestamp);
		radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_SCAN_COMPLETE;
		ch_planning_remove_scanlist_entry(own_dev, dev, radio);
	}
}
void ch_planning_clear_score_table(
	struct own_1905_device *own_dev,
	struct monitor_ch_info *ch_info)
{
	struct score_info *cumulative_score = NULL, *t_cumm_score = NULL;
	struct radio_info_db *radio_monitor = NULL, *radio = NULL;
	struct _1905_map_device *_1905_dev = NULL;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
	radio_monitor = topo_srv_get_radio_by_band(_1905_dev,ch_info->channel_num);
	if(radio_monitor == NULL) {
		err(CH_PLANING_PREX"radio monitor is NULL , return");
		return;
	}
	SLIST_FOREACH_SAFE(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score,next_ch_score, t_cumm_score) {
		radio = topo_srv_get_radio_by_band(_1905_dev,cumulative_score->channel);
		if(radio == NULL) {
			continue;
		}
		if(radio->band != radio_monitor->band){
			continue;
		}
		cumulative_score->total_score = 0;
		cumulative_score->dev_count = 0;
		cumulative_score->avg_score = 0;
		cumulative_score->ch_rank = 0;
	}
}
void ch_planning_remove_agg_score_table(
	struct own_1905_device *own_dev)
{
	struct score_info *cumulative_score = NULL;

	while (!SLIST_EMPTY(&(own_dev->ch_planning_R2.first_ch_score))) {
		cumulative_score = SLIST_FIRST(&(own_dev->ch_planning_R2.first_ch_score));
		SLIST_REMOVE_HEAD(&(own_dev->ch_planning_R2.first_ch_score),
			next_ch_score);
		os_free(cumulative_score);
	}
}
void ch_planning_remove_selective_ch_monitor_info(
	struct own_1905_device *own_dev,
	struct radio_info_db *reset_radio)
{
	struct monitor_ch_info *ch_info = NULL, *t_ch_info = NULL;
	struct affected_agent_info *agent = NULL;
	SLIST_FOREACH_SAFE(ch_info,
		&(own_dev->ch_planning_R2.first_monitor_ch), next_monitor_ch, t_ch_info) {
		if (ch_info->band != reset_radio->band)
			continue;

		if(ch_info->trigger_status == TRIGGER_TRUE) {
			err(CH_PLANING_PREX"scan timeout cancel for ch  %d", ch_info->channel_num);
			eloop_cancel_timeout(ch_scan_timeout, own_dev, ch_info);
			eloop_cancel_timeout(ch_scan_timeout_mtk, own_dev, ch_info);
		}
		eloop_cancel_timeout(channel_monitor_timeout, own_dev, ch_info);
		while (!SLIST_EMPTY(&(ch_info->first_affected_agent))) {
			agent = SLIST_FIRST(&(ch_info->first_affected_agent));
			SLIST_REMOVE_HEAD(&(ch_info->first_affected_agent),
				next_affected_agent);
			os_free(agent);
		}
		SLIST_REMOVE(&(own_dev->ch_planning_R2.first_monitor_ch),
				ch_info,
				monitor_ch_info,
				next_monitor_ch);
		os_free(ch_info);
		if(SLIST_EMPTY(&(own_dev->ch_planning_R2.first_monitor_ch)))
			break;
	}
}

void ch_planning_remove_ch_monitor_info_list(
	struct own_1905_device *own_dev,
	struct radio_info_db *reset_radio)
{
	struct monitor_ch_info *ch_info = NULL;
	struct affected_agent_info *agent = NULL;
	if(reset_radio != NULL) {
		ch_planning_remove_selective_ch_monitor_info(own_dev,reset_radio);
		return;
	}

	err(CH_PLANING_PREX"reset_radio null! remove all the ch_info from r2 monitor_ch");
	while (!SLIST_EMPTY(&(own_dev->ch_planning_R2.first_monitor_ch))) {
		ch_info = SLIST_FIRST(&(own_dev->ch_planning_R2.first_monitor_ch));
		if(ch_info->trigger_status == TRIGGER_TRUE) {
			err(CH_PLANING_PREX"scan timeout cancel for ch  %d", ch_info->channel_num);
			eloop_cancel_timeout(ch_scan_timeout, own_dev, ch_info);
			eloop_cancel_timeout(ch_scan_timeout_mtk, own_dev, ch_info);
		}
		eloop_cancel_timeout(channel_monitor_timeout, own_dev, ch_info);
		while (!SLIST_EMPTY(&(ch_info->first_affected_agent))) {
			agent = SLIST_FIRST(&(ch_info->first_affected_agent));
			SLIST_REMOVE_HEAD(&(ch_info->first_affected_agent),
				next_affected_agent);
			os_free(agent);
		}
		err(CH_PLANING_PREX"remove ch(%d) from r2 monitor_ch", ch_info->channel_num);
		SLIST_REMOVE_HEAD(&(own_dev->ch_planning_R2.first_monitor_ch),
			next_monitor_ch);
		os_free(ch_info);
	}
}
void ch_planning_remove_scan_list(
	struct own_1905_device *own_dev)
{
	struct scan_list_info *scan_list = NULL;
	while (!SLIST_EMPTY(&(own_dev->ch_planning_R2.first_scan_list))) {
		scan_list = SLIST_FIRST(&(own_dev->ch_planning_R2.first_scan_list));
		SLIST_REMOVE_HEAD(&(own_dev->ch_planning_R2.first_scan_list),
			next_scan_list);
		os_free(scan_list);
	}
}

void ch_planning_insert_ch_score_table(
	struct own_1905_device *own_dev, u8 channel)
{
	struct score_info *cumulative_score, *entry, *t_entry = NULL;
	/*insert only if not already inserted*/
	SLIST_FOREACH_SAFE(entry, &(own_dev->ch_planning_R2.first_ch_score), next_ch_score, t_entry) {
		if (entry->channel == channel) {
			debug("no need to insert ch already present");
			return;
		}
	}
	cumulative_score = os_zalloc(sizeof(struct score_info));
	if (cumulative_score == NULL) {
		err("alloc memory fail");
		assert(0);
		return ;
	}
	cumulative_score->channel = channel;
	cumulative_score->total_score = 0;
	cumulative_score->dev_count = 0;
	cumulative_score->ch_rank = 0;
	SLIST_INSERT_HEAD(&(own_dev->ch_planning_R2.first_ch_score),
		cumulative_score, next_ch_score);
}

void ch_planning_remove_ch_score_table(
	struct own_1905_device *own_dev, u8 channel)
{
	struct score_info *cumulative_score, *t_cumm_score = NULL;
	SLIST_FOREACH_SAFE(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score, t_cumm_score) {
		if(cumulative_score->channel == channel) {
			SLIST_REMOVE(&(own_dev->ch_planning_R2.first_ch_score),
				cumulative_score,
				score_info,
				next_ch_score);
			os_free(cumulative_score);
			return;
		}
	}
}
struct score_info* ch_planning_find_ch_max_score(
	struct own_1905_device *own_dev)
{
	struct score_info *ch_score_temp = NULL, *max_ch_score = NULL, *t_ch_score_tmp = NULL;
	SLIST_FOREACH_SAFE(ch_score_temp,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score, t_ch_score_tmp) {
		if(!max_ch_score)
			max_ch_score = ch_score_temp;
		if(ch_score_temp->total_score > max_ch_score->total_score)
			max_ch_score = ch_score_temp;
	}
	if(max_ch_score)
		err("max score channel %d, score %d",max_ch_score->channel, max_ch_score->total_score);
	return max_ch_score;
}
void dump_ch_planning_score_info(
	struct own_1905_device *own_dev)
{
	struct score_info *ch_score = NULL, *t_ch_score = NULL;
	always("******* Total Channel Score Table************\n");
	SLIST_FOREACH_SAFE(ch_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score, t_ch_score) {
		always("channel %d, total score %d, avg score %d Rank %d\n", ch_score->channel,
			ch_score->total_score, ch_score->avg_score, ch_score->ch_rank);
	}
}

struct score_info* ch_planning_find_current_ch_score_info(
	struct own_1905_device *own_dev,
	struct radio_info_db *current_radio )
{
	struct score_info *ch_score = NULL, *current_working_ch_info = NULL, *t_ch_score = NULL;
	SLIST_FOREACH_SAFE(ch_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score, t_ch_score){
		if(ch_score->channel == current_radio->channel[0]) {
			current_working_ch_info = ch_score;
			return current_working_ch_info;
		}
	}
	return NULL;
}

u16 ch_planning_calc_weight( u8 band, u16 neighbor_num)
{
	u16 weight = 0;
	u16 weight_5G[MAX_BSS_NUM_5G] = {
		0, 18, 28, 36, 42, 47, 51, 54, 57, 60,
		62, 65, 67, 69, 71, 72, 74, 75, 77, 78,
		79, 81, 82, 83, 84, 85, 86, 87, 88, 89,
		90, 91, 91, 92, 93, 94, 94, 95, 96, 96,
		97, 98, 98, 99, 100};
	u16 weight_2G[MAX_BSS_NUM_2G] = {0, 33, 52, 66, 77, 86, 93, 100};

	if (band == BAND_2G){
		if (neighbor_num >= MAX_BSS_NUM_2G) {
			weight = weight_2G[MAX_BSS_NUM_2G - 1];
		}else {
			weight = weight_2G[neighbor_num - 1];
		}
	} else{
		if (neighbor_num >= MAX_BSS_NUM_5G) {
			weight = weight_5G[MAX_BSS_NUM_5G - 1];
		}else {
			weight = weight_5G[neighbor_num - 1];
		}
	}
	return weight;
}
u16 ch_planning_calc_OBSS(struct own_1905_device *own_dev,
	struct _1905_map_device *_1905_dev,
	struct radio_info_db *radio,
	struct scan_result_tlv *scan_res)
{
/*need to remove the my MAP network contribution from OBSS time*/
/*Calculate ObssAirtime based on the following formula
	ObssTime-A1 = ObssTimeA1 - (Tx/Rx Time C +Tx/Rx Time A2 etc.)*/
	//struct _1905_map_device *other_map_dev = NULL;
	//struct radio_info_db *other_mapdev_radio = NULL;
	u8 OBSS = 0;
	//u32 OtherDevTxRxAirTime = 0;

	if (radio->channel[0] != scan_res->channel) {
		debug(CH_PLANING_PREX"My MAP Network is not on this channel , so can use obss time directly");
		OBSS = scan_res->utilization;
		return OBSS;
	}
	return scan_res->utilization;//return utilization for all channels
#if 0
	OBSS = radio->radio_metrics.cu_other;
	SLIST_FOREACH(other_map_dev, &own_dev->_1905_dev_head, next_1905_device) {
	/* Don't subtract your own tx rx air time as it is already not present in obss airtime*/
		if(other_map_dev == _1905_dev)
			continue;
		other_mapdev_radio =
			topo_srv_get_radio_by_band(other_map_dev,radio->channel[0]);
		if(!other_mapdev_radio)
			return 0;
		err("cu_tx %d,cu_rx %d",other_mapdev_radio->radio_metrics.cu_tx,
			other_mapdev_radio->radio_metrics.cu_rx);
		
		OtherDevTxRxAirTime = OtherDevTxRxAirTime +
			other_mapdev_radio->radio_metrics.cu_tx +
			other_mapdev_radio->radio_metrics.cu_rx;
	}
	if(OBSS > OtherDevTxRxAirTime) {
		OBSS = OBSS - OtherDevTxRxAirTime;
		err("OBSS %d", OBSS);
		return OBSS;
	}else {
		err("ASSUMPTION that ObssAirtime_A2 > (MyTxAirtime_A1 + MyRxAirtime_A1) is wrong");
		return scan_res->utilization;
	}
#endif
}

extern u8 RadarCh[MAX_DFS_CHANNEL];
void start_netopt_timer_R2(struct own_1905_device *own_dev, u8 channel, u8 dfs_status)
{
	struct cac_cap_db *cap = NULL, *t_cap = NULL;
	struct radio_info_db *radio = NULL;
	struct _1905_map_device *_1905_dev = NULL, *t_1905_dev = NULL;
	unsigned int total_cac_time = 0;
	Boolean is_radar_ch = FALSE;
	u8 count = 0, opclass = 0;
	int bw = BW_20, max_bw = -1;

	is_radar_ch = ch_planning_is_ch_dfs(own_dev, channel);

	if (is_radar_ch == FALSE) {
		err(CH_PLANING_PREX"channel (%u) is NON DFS, return", channel);
		return;
	}

	count = get_net_opt_dev_count((struct mapd_global *)own_dev->back_ptr);
	if (count <= 1)
		return;

	SLIST_FOREACH_SAFE(_1905_dev, &own_dev->_1905_dev_head, next_1905_device, t_1905_dev) {
		if(!_1905_dev->in_network) {
			debug(CH_PLANING_PREX"agent("MACSTR") not in network", MAC2STR(_1905_dev->_1905_info.al_mac_addr));
			continue;
		}
		radio = topo_srv_get_radio_by_band(_1905_dev, channel);
		if (!radio) {
			debug(CH_PLANING_PREX"agent("MACSTR") radio(%u) is NULL",
				MAC2STR(_1905_dev->_1905_info.al_mac_addr), channel);
			continue;
		}
		if (radio->dev_ch_plan_info.dev_ch_plan_state == CHPLAN_STATE_CAC_ONGOING) {
			SLIST_FOREACH_SAFE(cap, &radio->cac_cap.cac_capab_head, cac_cap_entry, t_cap) {
				if (is_channel_in_cac_cap(channel, cap, &bw, &opclass) == 1) {
					if (max_bw < bw) {
						max_bw = bw;
						total_cac_time =
							cap->cac_interval[0]*power(16,4) + cap->cac_interval[1]*power(16,2) + cap->cac_interval[2];
					}
				}
			}
			if (total_cac_time) {
				if (eloop_is_timeout_registered(trigger_net_opt,(void *)own_dev , NULL))
					eloop_cancel_timeout(trigger_net_opt, own_dev, NULL);
				eloop_register_timeout((own_dev->network_optimization.post_cac_trigger_time + total_cac_time),
						0, trigger_net_opt, own_dev, NULL);
				err("Channel is DFS, netopt timer : %d+%d",
					own_dev->network_optimization.post_cac_trigger_time, total_cac_time);
				return;
			}
		}
	}

	/* We came here means we could not find any Agent on which CAC has been triggered, 
	Check DFS STATUS if we have changed to the channel directly */
	_1905_dev = topo_srv_get_1905_device(own_dev, NULL);
	if (_1905_dev->map_version == DEV_TYPE_R2 && dfs_status == 0) {
		radio = topo_srv_get_radio_by_band(_1905_dev, channel);
		if (!radio) {
			err(CH_PLANING_PREX"radio is NULL for channel (%u)", channel);
			return;
		}
		t_cap = NULL;
		SLIST_FOREACH_SAFE(cap, &radio->cac_cap.cac_capab_head, cac_cap_entry, t_cap) {
			if (is_channel_in_cac_cap(channel, cap, &bw, &opclass) == 1) {
				if (max_bw < bw) {
					max_bw = bw;
					total_cac_time =
						cap->cac_interval[0]*power(16,4) + cap->cac_interval[1]*power(16,2) + cap->cac_interval[2];
				}
			}
		}
		if (total_cac_time) {
			if (eloop_is_timeout_registered(trigger_net_opt,(void *)own_dev , NULL))
				eloop_cancel_timeout(trigger_net_opt, own_dev, NULL);
			eloop_register_timeout((own_dev->network_optimization.post_cac_trigger_time + total_cac_time),
				0, trigger_net_opt, own_dev, NULL);
			err("Channel is DFS, netopt timer : %d+%d",
				own_dev->network_optimization.post_cac_trigger_time, total_cac_time);
			return;
		}
	} else if (_1905_dev->map_version == DEV_TYPE_R1) {
		start_netopt_timer(own_dev, channel);
	}
}

void trigger_chplan_post_cac_resp(void *eloop_ctx, void *timeout_ctx) {
	struct own_1905_device *own_dev = (struct own_1905_device *)eloop_ctx;

	own_dev->ch_planning_R2.ch_plan_state =	CHPLAN_STATE_CH_CHANGE_TRIGGERED;
	own_dev->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
	ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED, own_dev->trig_chan_post_cac_chplan,own_dev);
	err(CH_PLANING_PREX"own_dev->force_ch_change %d", own_dev->force_ch_change);
	if (own_dev->force_ch_change || own_dev->div_ch_planning) {
		err(CH_PLANING_PREX"R1 ch planning is %d, make it 1 here", own_dev->ch_planning.ch_planning_enabled);
		own_dev->ch_planning.ch_planning_enabled = 1;
	}
	mapd_restart_channel_plannig(own_dev->back_ptr);
	own_dev->trig_chan_post_cac_chplan = 0;

}


void channel_cac_timeout2(void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *own_dev = (struct own_1905_device *)eloop_ctx;
	struct _1905_map_device *_1905_dev, *t_1905_dev = NULL;
	struct radio_info_db *radio, *t_radio = NULL;
	struct mapd_global *global = own_dev->back_ptr;
	u8 channel = own_dev->ch_planning_R2.CAC_on_channel;
	u8 dfs_status;
	err(CH_PLANING_PREX"CAC_on_channel %d ", own_dev->ch_planning_R2.CAC_on_channel);

	if (!own_dev->ch_planning_R2.CAC_on_channel)
		return;

	/*Find the device on which CAC timed out and mark its state as CAC_TIMEOUT*/
	SLIST_FOREACH_SAFE(_1905_dev, &own_dev->_1905_dev_head, next_1905_device, t_1905_dev) {
		SLIST_FOREACH_SAFE(radio, &_1905_dev->first_radio, next_radio, t_radio) {
			if(radio->dev_ch_plan_info.dev_ch_plan_state == CHPLAN_STATE_CAC_ONGOING)
				radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CAC_TIMEOUT;
		}
	}

	/*Find a new agent to trigger CAC on*/
	dfs_status = ch_planning_check_dfs_by_bw_wrapper(global, channel);

	/* No more Agent avilable for doing CAC. */
	if((dfs_status == DIRECT_SWITCH_CAC) &&
	   (is_CAC_Success(global, channel) != 1) &&
	   !(global->dev.Restart_ch_planning_radar_on_agent)) {
		if (ch_planning_check_controller_cac_cap(own_dev, channel, DEDICATED_RADIO)) {
			err(CH_PLANING_PREX"trigger cac on cont radio(%d) mode(%d)", channel, DEDICATED_RADIO);
			ch_planning_trigger_cac_on_cont(own_dev->back_ptr, channel, 1, DEDICATED_RADIO);
		} else {
			err(CH_PLANING_PREX"trigger cac on cont radio(%d), mode(%d)", channel, CONTINUOUS);
			ch_planning_trigger_cac_on_cont(own_dev->back_ptr, channel, 0, CONTINUOUS);
		}
	} else if (dfs_status == DIRECT_SWITCH_CAC && (is_CAC_Success(global, channel) == TRUE)) {
		own_dev->ch_planning_R2.ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
		own_dev->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
		ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED, channel, own_dev);
		return;
	}

	if (dfs_status != NO_CAC)
		start_netopt_timer_R2(own_dev, channel, dfs_status);

}

void ch_planning_blacklist_ch(
	struct own_1905_device *own_dev, u8 channel, u8 op)
{
	struct score_info *cumulative_score, *t_cumm_score = NULL;
	unsigned char channel_block[MAX_CHANNEL_BLOCKS] = {0};

	int maxbw = BW_20, bw = BW_20;
	struct _1905_map_device *_1905_dev = NULL, *t_1905_dev = NULL;
	SLIST_FOREACH_SAFE(_1905_dev, &own_dev->_1905_dev_head, next_1905_device, t_1905_dev) {
		bw = ch_planning_get_dev_bw_from_channel(_1905_dev, channel);
		if (maxbw < bw)
			maxbw = bw;
	}
	ch_planning_get_channel_block(channel, channel_block, op, maxbw);
	//err("ch blk to blacklist is %d, %d, %d, %d", channel_block[0], channel_block[1], channel_block[2], channel_block[3]);
	SLIST_FOREACH_SAFE(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score, t_cumm_score) {
		if (op == 129) {
			if(cumulative_score->channel == channel_block[0] ||
				cumulative_score->channel == channel_block[1] ||
				cumulative_score->channel == channel_block[2] ||
				cumulative_score->channel == channel_block[3] ||
				cumulative_score->channel == channel_block[4] ||
				cumulative_score->channel == channel_block[5] ||
				cumulative_score->channel == channel_block[6] ||
				cumulative_score->channel == channel_block[7]) {
				cumulative_score->avg_score = -65535;
				cumulative_score->ch_rank = 0;
				err(CH_PLANING_PREX"blacklist channel %d due to radar",cumulative_score->channel);
				//return;
			}
		}
#ifdef MAP_320BW
		else if (op == 137) {
			if (cumulative_score->channel == channel_block[0] ||
				cumulative_score->channel == channel_block[1] ||
				cumulative_score->channel == channel_block[2] ||
				cumulative_score->channel == channel_block[3] ||
				cumulative_score->channel == channel_block[4] ||
				cumulative_score->channel == channel_block[5] ||
				cumulative_score->channel == channel_block[6] ||
				cumulative_score->channel == channel_block[7] ||
				cumulative_score->channel == channel_block[8] ||
				cumulative_score->channel == channel_block[9] ||
				cumulative_score->channel == channel_block[10] ||
				cumulative_score->channel == channel_block[11] ||
				cumulative_score->channel == channel_block[12] ||
				cumulative_score->channel == channel_block[13] ||
				cumulative_score->channel == channel_block[14] ||
				cumulative_score->channel == channel_block[15]) {
				cumulative_score->avg_score = -65535;
				cumulative_score->ch_rank = 0;
				err(CH_PLANING_PREX"blacklist channel %d due to radar", cumulative_score->channel);
			}
		}
#endif
		else {
			if(cumulative_score->channel == channel_block[0] ||
				cumulative_score->channel == channel_block[1] ||
				cumulative_score->channel == channel_block[2] ||
				cumulative_score->channel == channel_block[3]) {
				cumulative_score->avg_score = -65535;
				cumulative_score->ch_rank = 0;
				err(CH_PLANING_PREX"blacklist channel %d due to radar",cumulative_score->channel);
				//return;
			}
		}
	}
}

void ch_planning_radar_detected2(
	struct own_1905_device *own_dev,
	struct radio_info_db *radio)
{
	struct _1905_map_device *tmp_dev = topo_srv_get_1905_device(own_dev, NULL);

	ch_planning_blacklist_ch(own_dev,
		radio->cac_comp_status.channel, radio->operating_class);

	if (own_dev->ch_planning_R2.ch_plan_state == CHPLAN_STATE_CAC_ONGOING) {
		info("radio_ch band: %u, CAC_ch band: %u", get_band(tmp_dev, radio->channel[0]),
			get_band(tmp_dev, own_dev->ch_planning_R2.CAC_on_channel));
		if (get_band(tmp_dev, radio->channel[0]) == get_band(tmp_dev, own_dev->ch_planning_R2.CAC_on_channel)) {
			if (eloop_is_timeout_registered(trigger_net_opt,(void *)own_dev, NULL)) {
				eloop_cancel_timeout(trigger_net_opt, own_dev, NULL);
			}
		} else {
			own_dev->ch_planning_R2.need_cac_on_channel = own_dev->ch_planning_R2.CAC_on_channel;
			own_dev->ch_planning_R2.CAC_on_channel = 0;
			info("set need_cac_ch: %u", own_dev->ch_planning_R2.need_cac_on_channel);
		}
	}
	/*need to go back to ch planning R1 logic to find new best channel*/
	own_dev->ch_planning_R2.ch_plan_state =
		CHPLAN_STATE_CH_CHANGE_TRIGGERED;
	radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
	own_dev->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
	if (own_dev->div_ch_planning == 1)
		own_dev->ch_planning_R2.CAC_on_channel = 0;

}
void controller_ch_set(void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *ctx = (struct own_1905_device *)eloop_ctx;
	struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(ctx, NULL);
	struct _1905_map_device *tmp_dev = NULL, *t_tmp_dev = NULL;
	err(CH_PLANING_PREX"controller dfs ch select timeout");
	ch_planning_send_select(ctx,
		own_1905_node,
		ctx->ch_planning_R2.ch_prefer_count,
		ctx->ch_planning_R2.ch_prefer_for_ch_select);
	SLIST_FOREACH_SAFE(tmp_dev, &ctx->_1905_dev_head, next_1905_device, t_tmp_dev) {
		tmp_dev->ch_sel_req_given = 0;
	}
	os_free(ctx->ch_planning_R2.ch_prefer_for_ch_select);
	ctx->ch_planning_R2.ch_prefer_for_ch_select = NULL;
}

void ch_planning_make_ch_select_req(
	struct own_1905_device *ctx,
	unsigned char ch_prefer_count,
	struct ch_prefer_lib *ch_prefer)
{
	struct ch_prefer_lib *con_prefer = ctx->ch_planning_R2.ch_prefer_for_ch_select;
	u8 i = 0, j = 0, k = 0;

	err(CH_PLANING_PREX"prefer count %d ", ctx->ch_planning_R2.ch_prefer_count);
	while (i < ch_prefer_count) {
		u8 max_pref_index = ch_planning_find_max_pref_index(ctx, NULL, &ch_prefer[j]);
		err(CH_PLANING_PREX"Reason code: %d opclass %d channel %d", ch_prefer[j].opinfo[max_pref_index].reason,
				ch_prefer[j].opinfo[max_pref_index].op_class,
				ch_prefer[j].opinfo[max_pref_index].ch_list[0]);
		os_memcpy(con_prefer[i].identifier,ch_prefer[j].identifier,ETH_ALEN);
		con_prefer[i].op_class_num = ch_prefer[j].op_class_num;
		con_prefer[i].opinfo[0].op_class = ch_prefer[j].opinfo[max_pref_index].op_class;
		con_prefer[i].opinfo[0].ch_num = ch_prefer[j].opinfo[max_pref_index].ch_num;
		con_prefer[i].opinfo[0].ch_list[0] = ch_prefer[j].opinfo[max_pref_index].ch_list[0];
		con_prefer[i].opinfo[0].perference = ch_prefer[j].opinfo[max_pref_index].perference;
		con_prefer[i].opinfo[0].reason = ch_prefer[j].opinfo[max_pref_index].reason;
		/* Using i+1 index for storing primary CH info at con_prefer->opinfo */
		if ((ch_prefer->opinfo[max_pref_index].op_class == 128)
			|| (ch_prefer->opinfo[max_pref_index].op_class == 129)) {
			unsigned char next_channel;

			next_channel = ch_prefer->opinfo[max_pref_index+1].ch_list[0];
			if (is_valid_primary_ch_80M_160M(next_channel, ch_prefer->opinfo[max_pref_index].ch_list[0],
									ch_prefer->opinfo[max_pref_index].op_class)) {
				con_prefer[i].opinfo[k+1].ch_num = 1;
				con_prefer[i].opinfo[k+1].ch_list[0] = ch_prefer->opinfo[max_pref_index+1].ch_list[0];
				con_prefer[i].opinfo[k+1].op_class = ch_prefer->opinfo[max_pref_index+1].op_class;
				//os_memcpy(con_prefer->opinfo[i+1].identifier, ch_prefer->identifier, ETH_ALEN);
				con_prefer[i].opinfo[k+1].reason = ch_prefer->opinfo[max_pref_index+1].reason;
				err(CH_PLANING_PREX"DFS opclass %d primary channel %d",
					ch_prefer->opinfo[max_pref_index+1].op_class,
					ch_prefer->opinfo[max_pref_index+1].ch_list[0]);
				/* Since we just added one extra entry, increment the index by 1 to make sure it will not get overwritten in next iteration */
				i++;
				ch_prefer_count++;
			}
		}
		i++;
		j++;
	}
}

void ch_planning_R2_send_select_dfs(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned char ch_prefer_count,
	struct ch_prefer_lib *ch_prefer,
	u8 channel)
{
	struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(ctx, NULL);
	struct _1905_map_device *tmp_dev = NULL, *tmp_dev_2 = NULL, *t_tmp_dev = NULL, *t_tmp_dev_2 = NULL;
	struct ch_prefer_lib *ch_prefer_per_dev = NULL;
	int max_hop = -1;
	err(CH_PLANING_PREX"R2.ch_plan_enable %d, state %d, ctx->dedicated_radio %d",
		ctx->ch_planning_R2.ch_plan_enable,  ctx->ch_planning_R2.ch_plan_state, ctx->dedicated_radio);
	/*since our dev needs to jump to a dfs channel , so all the devices that are connected below
	it in the tree (or have max hop count more that the dev), need to first be switched to the dfs channel ,
	otherwise they may mistakenly perform CAC due to Channel switch announcement of their uplink*/
	if((ctx->ch_planning_R2.ch_plan_enable == TRUE || ctx->dedicated_radio)&&
		ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_CH_CHANGE_TRIGGERED) {
		ch_prefer_per_dev = os_zalloc(sizeof(struct ch_prefer_lib));
		if (!ch_prefer_per_dev) {
			err (CH_PLANING_PREX" CH Prefer is allocation failure");
			goto Error;
		}
		SLIST_FOREACH_SAFE(tmp_dev, &ctx->_1905_dev_head, next_1905_device, t_tmp_dev) {
			if (tmp_dev == own_1905_node)
				continue;
			max_hop = -1;
			SLIST_FOREACH_SAFE(tmp_dev_2, &ctx->_1905_dev_head, next_1905_device, t_tmp_dev_2) {
				if (tmp_dev_2->ch_sel_req_given == 1) {
					debug(CH_PLANING_PREX"Channel Selection Req already sent to: "MACSTR, MAC2STR(tmp_dev_2->_1905_info.al_mac_addr));
					continue;
				}
				if (max_hop <= tmp_dev_2->root_distance)
					max_hop = tmp_dev_2->root_distance;
			}
			t_tmp_dev_2 = NULL;
			SLIST_FOREACH_SAFE(tmp_dev_2, &ctx->_1905_dev_head, next_1905_device, t_tmp_dev_2) {
				if (tmp_dev_2 == own_1905_node) {
					debug(CH_PLANING_PREX"skip for controller");
					continue;
				}
				if (max_hop == tmp_dev_2->root_distance && tmp_dev_2->ch_sel_req_given == 0) {
					struct radio_info_db *temp_radio = topo_srv_get_radio_by_band(tmp_dev_2,channel);
					if (temp_radio && (temp_radio->channel[0] != channel)) {
						err(CH_PLANING_PREX"sending ch sel request to:"MACSTR,
								MAC2STR(tmp_dev_2->_1905_info.al_mac_addr));
						temp_radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
						ch_planning_ch_selection_prefer_data(ctx, channel,temp_radio,ch_prefer_per_dev);
						ch_planning_send_select(ctx, tmp_dev_2, ch_prefer_count, ch_prefer_per_dev);
						os_memset(ch_prefer_per_dev, 0, sizeof(struct ch_prefer_lib));
					}
				}
			}
		}
		if (!ctx->ch_planning_R2.ch_prefer_for_ch_select && !own_1905_node->channel_planning_completed) {
			ctx->ch_planning_R2.ch_prefer_count = ch_prefer_count;
			ctx->ch_planning_R2.ch_prefer_for_ch_select = os_zalloc(sizeof(struct ch_prefer_lib) * 3);
			if(!ctx->ch_planning_R2.ch_prefer_for_ch_select) {
				err(CH_PLANING_PREX"alloc fail for ctx->ch_planning_R2.ch_prefer_for_ch_select");
				goto Error;
			} else {
				ch_planning_make_ch_select_req(ctx, ch_prefer_count, ch_prefer);
			}

			eloop_register_timeout(3, 0,
				controller_ch_set, ctx, NULL);
		}
	}
Error:
	if (ch_prefer_per_dev)
		os_free(ch_prefer_per_dev);
	return;
}

void ch_planning_send_select_2G(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned char ch_prefer_count,
	struct ch_prefer_lib *ch_prefer,
	u8 channel)
{
	struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(ctx, NULL);
	struct _1905_map_device *tmp_dev = NULL, *tmp_dev_2 = NULL, *t_tmp_dev = NULL, *t_tmp_dev_2 = NULL;
	struct ch_prefer_lib *ch_prefer_per_dev = NULL;
	u8 ch_prefer_count_per_dev = 1;
	int max_hop = -1;
	debug(CH_PLANING_PREX"R1.ch_plan_enable %d, R2.ch_plan_enable %d, state %d, ctx->dedicated_radio %d",
		ctx->ch_planning.ch_planning_enabled, ctx->ch_planning_R2.ch_plan_enable, ctx->ch_planning_R2.ch_plan_state, ctx->dedicated_radio);
	if((((ctx->ch_planning.ch_planning_enabled == TRUE) && (ctx->map_version <= DEV_TYPE_R1))||
		(ctx->ch_planning_R2.ch_plan_enable == TRUE)) &&
		ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_CH_CHANGE_TRIGGERED) {
		ch_prefer_per_dev = os_zalloc(sizeof(struct ch_prefer_lib));
		if (!ch_prefer_per_dev) {
			err (CH_PLANING_PREX" CH Prefer is allocation failure");
			goto Error;
		}
		/*For 2G we need to explicitly set, */
		SLIST_FOREACH_SAFE(tmp_dev, &ctx->_1905_dev_head, next_1905_device, t_tmp_dev) {
			if (tmp_dev == own_1905_node)
				continue;
			max_hop = -1;
			SLIST_FOREACH_SAFE(tmp_dev_2, &ctx->_1905_dev_head, next_1905_device, t_tmp_dev_2) {
				if (tmp_dev_2->ch_sel_req_given == 1) {
					debug(CH_PLANING_PREX"Channel Selection Req already sent to: "MACSTR, MAC2STR(tmp_dev_2->_1905_info.al_mac_addr));
					continue;
				}
				if (max_hop <= tmp_dev_2->root_distance)
					max_hop = tmp_dev_2->root_distance;
			}
			t_tmp_dev_2 = NULL;
			SLIST_FOREACH_SAFE(tmp_dev_2, &ctx->_1905_dev_head, next_1905_device, t_tmp_dev_2) {
				if (tmp_dev_2 == own_1905_node) {
					debug(CH_PLANING_PREX"skip for controller");
					continue;
				}
				if (max_hop == tmp_dev_2->root_distance && tmp_dev_2->ch_sel_req_given == 0) {
					if (tmp_dev_2 != dev) {
						struct radio_info_db *temp_radio = topo_srv_get_radio_by_band(tmp_dev_2,channel);
					if (temp_radio && (temp_radio->channel[0] != channel)) {
						err(CH_PLANING_PREX"sending ch sel request to:"MACSTR,
								MAC2STR(tmp_dev_2->_1905_info.al_mac_addr));
						temp_radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
						ch_planning_ch_selection_prefer_data(ctx, channel, temp_radio, ch_prefer_per_dev);
						ch_planning_send_select(ctx, tmp_dev_2, ch_prefer_count_per_dev, ch_prefer_per_dev);
						os_memset(ch_prefer_per_dev, 0, sizeof(struct ch_prefer_lib));
					}

					} else {
						info(CH_PLANING_PREX"same device "MACSTR" for which the Channel planning is running\n",
								MAC2STR(tmp_dev_2->_1905_info.al_mac_addr));
						ch_planning_send_select(ctx, tmp_dev_2, ch_prefer_count, ch_prefer);
					}
				}
			}
		}

		if (!ctx->ch_planning_R2.ch_prefer_for_ch_select && !own_1905_node->channel_planning_completed) {
			ctx->ch_planning_R2.ch_prefer_count = ch_prefer_count;
			ctx->ch_planning_R2.ch_prefer_for_ch_select = os_zalloc(sizeof(struct ch_prefer_lib) * 3);
			if(!ctx->ch_planning_R2.ch_prefer_for_ch_select) {
				err(CH_PLANING_PREX"alloc fail for ctx->ch_planning_R2.ch_prefer_for_ch_select");
				goto Error;
			} else {
				ch_planning_make_ch_select_req(ctx, ch_prefer_count, ch_prefer);
			}

			eloop_register_timeout(3, 0,
				controller_ch_set, ctx, NULL);
		}
	}
Error:
	if (ch_prefer_per_dev)
		os_free(ch_prefer_per_dev);
	return;
}

void ch_planning_handle_cac_response2(
	struct own_1905_device *own_dev,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
	u8 dfs_status;
	u8 channel;
	if (own_dev->user_triggered_cac == 0) {
		if((own_dev->ch_planning_R2.ch_plan_state != CHPLAN_STATE_CAC_ONGOING) ||
			(radio->dev_ch_plan_info.dev_ch_plan_state != CHPLAN_STATE_CAC_ONGOING) ||
			(own_dev->ch_planning.ch_planning_state != CHANNEL_PLANNING_CAC_START))
		{
			err(CH_PLANING_PREX"state mismatch! r2_state(%d) radio_state(%d)",
				own_dev->ch_planning_R2.ch_plan_state,
				radio->dev_ch_plan_info.dev_ch_plan_state);
			return;
		}
	}
	if (radio->cac_comp_status.channel != own_dev->ch_planning_R2.CAC_on_channel) {
		err(CH_PLANING_PREX"Return, CAC req on %u, CAC success for %u\n", own_dev->ch_planning_R2.CAC_on_channel,
			radio->cac_comp_status.channel);
		return;
	}
	err(CH_PLANING_PREX"cancel cac timeout on ch %d",own_dev->ch_planning_R2.CAC_on_channel);
	eloop_cancel_timeout(channel_cac_timeout2,
		own_dev, NULL);

	if(radio->cac_comp_status.cac_status == RADAR_DETECTED) {
		err(CH_PLANING_PREX"Radar found on best channel(%u)", radio->cac_comp_status.channel);
		if (own_dev->user_triggered_cac == 0)
			ch_planning_radar_detected2(own_dev,radio);
	} else if (radio->cac_comp_status.cac_status == CAC_SUCCESSFUL) {
		err(CH_PLANING_PREX"set this channel %d",radio->cac_comp_status.channel);
		if (own_dev->user_triggered_cac == 0) {
			own_dev->trig_chan_post_cac_chplan = radio->channel[0];
			eloop_register_timeout(POST_CAC_CHPLAN_TRIGGER_TIME,
				0, trigger_chplan_post_cac_resp, own_dev, NULL);
		}
	} else {
		err(CH_PLANING_PREX"CAC failed trigger CAC on different agent.");
		/*current agent is not good ,mark it */
		if (own_dev->user_triggered_cac == 0) {
			radio->dev_ch_plan_info.dev_ch_plan_state =
				CHPLAN_STATE_CAC_TIMEOUT;

			channel = radio->cac_comp_status.channel;
			dfs_status = ch_planning_check_dfs_by_bw_wrapper(own_dev->back_ptr, channel);

			if((dfs_status == DIRECT_SWITCH_CAC) &&
					(is_CAC_Success(own_dev->back_ptr, channel) != 1) &&
					!(own_dev->Restart_ch_planning_radar_on_agent)) {
				if (ch_planning_check_controller_cac_cap(own_dev, channel, DEDICATED_RADIO)) {
					err(CH_PLANING_PREX"trigger cac on cont radio(%d) mode(%d)", channel, DEDICATED_RADIO);
					ch_planning_trigger_cac_on_cont(own_dev->back_ptr, channel, 1, DEDICATED_RADIO);
				} else {
					err(CH_PLANING_PREX"trigger cac on cont radio(%d), mode(%d)", channel, CONTINUOUS);
					ch_planning_trigger_cac_on_cont(own_dev->back_ptr, channel, 0, CONTINUOUS);
				}
			}

			if(dfs_status != NO_CAC)
				start_netopt_timer_R2(own_dev, channel, dfs_status);
		}
	}
}

u8 is_channel_in_cac_cap_by_bw(
	u8 channel,
	struct cac_cap_db *cap,
	int bandwidth,
	u8 *cac_op_class)
{
	struct cac_opcap_db *opcap = NULL, *t_opcap = NULL;
	u8 i = 0;
	int bw = BW_20;
	int ret = 0;
	SLIST_FOREACH_SAFE(opcap, &cap->cac_opcap_head, cac_opcap_entry, t_opcap) {
		bw = chan_mon_get_bw_from_op_class(opcap->op_class);

		for(i = 0; i < opcap->ch_num; i++) {
			if (bw <= BW_40) {
				if(opcap->ch_list[i] == channel) {
					if (bw == bandwidth) {
						ret = 1;
						if(cac_op_class != NULL)
							*cac_op_class = opcap->op_class;
					}
				}
			} else {
				if (opcap->ch_list[i] ==
						ch_planning_get_centre_freq_ch(channel, opcap->op_class)) {
					if (bw == bandwidth) {
						ret = 1;
						if(cac_op_class != NULL)
							*cac_op_class = opcap->op_class;
					}
				}
			}
		}
	}
	info(CH_PLANING_PREX"channel(%d) not found in cac cap", channel);
	return ret;
}


u8 is_channel_in_cac_cap(
	u8 channel,
	struct cac_cap_db *cap,
	int *bandwidth,
	u8 *opclass)
{
	struct cac_opcap_db *opcap = NULL, *t_opcap = NULL;
	u8 i = 0;
	int bw = BW_20;
	int max_bw = -1;
	SLIST_FOREACH_SAFE(opcap, &cap->cac_opcap_head, cac_opcap_entry, t_opcap) {
		if (opcap->op_class == 130) {
			debug(CH_PLANING_PREX"Continue as we dont support 80 plus 80");
			continue;
		}
		bw = chan_mon_get_bw_from_op_class(opcap->op_class);

		for(i = 0; i < opcap->ch_num; i++) {
			if (bw <= BW_40) {
				if(opcap->ch_list[i] == channel) {
					if (bw > max_bw) {
						max_bw = bw;
						*opclass = opcap->op_class;
					}
				}
			} else {
				if (opcap->ch_list[i] ==
						ch_planning_get_centre_freq_ch(channel, opcap->op_class)) {
					if (bw > max_bw) {
						max_bw = bw;
						*opclass = opcap->op_class;
					}
				}
			}
		}
	}
	if (bandwidth != NULL)
		*bandwidth = max_bw;
	debug(CH_PLANING_PREX"channel(%d) not found in cac cap", channel);
	if (max_bw >= BW_20)
		return 1;

	return 0;
}

void ch_planning_trigger_cac_msg2 (
	struct mapd_global *global,
	struct _1905_map_device *_1905_dev,
	u8 channel, unsigned int cac_method,
	u8 cac_op_class)
{
	struct own_1905_device *ctx = &global->dev;
	struct radio_info_db *radio = NULL;
	struct cac_request *req = NULL;
	unsigned int total_cac_time = 0;
	struct cac_cap_db *cap = NULL, *t_cap = NULL;
	int bw = BW_20, max_bw = -1;
	u8 opclass = 0;
	/*CAC is to be done on the new best channel*/
	req = os_zalloc(sizeof(struct cac_request) + sizeof(struct cac_tlv));
	if(!req) {
		err(CH_PLANING_PREX"malloc fail for req");
		return;
	}
	req->num_radio = 1;
	radio = topo_srv_get_radio_by_band(_1905_dev, channel);

	if (radio == NULL) {
		err(CH_PLANING_PREX"radio is NULL for channel(%d) on dev("MACSTR")",
			channel, MAC2STR(_1905_dev->_1905_info.al_mac_addr));
		os_free(req);
		return;
	}

	os_memcpy(req->tlv[0].identifier, radio->identifier, ETH_ALEN);
	req->tlv[0].cac_action = 0x01;
	req->tlv[0].cac_method = cac_method;
	req->tlv[0].ch_num = channel;
	req->tlv[0].op_class_num = cac_op_class;
	err(CH_PLANING_PREX"send cac req to agent with ALMAC "MACSTR,
		MAC2STR(_1905_dev->_1905_info.al_mac_addr));
	err(CH_PLANING_PREX"cac action: %d, cac method: %d, ch num: %d, op class: %d",
		req->tlv[0].cac_action, req->tlv[0].cac_method, req->tlv[0].ch_num, req->tlv[0].op_class_num);
	if (_1905_dev->map_version != DEV_TYPE_R2
#ifdef MAP_R3
		&& _1905_dev->map_version != DEV_TYPE_R3
#endif
	) {
		err(CH_PLANING_PREX"agent is not R2, return");
		os_free(req);
		return;
	}
	map_1905_Send_CAC_Request(
		global->_1905_ctrl,
		(char *)_1905_dev->_1905_info.al_mac_addr,
		req);
	radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CAC_ONGOING;
	global->dev.ch_planning_R2.ch_plan_state = CHPLAN_STATE_CAC_ONGOING;
	ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_CAC_START;
	os_free(req);
	global->dev.ch_planning_R2.CAC_on_channel = channel;
	SLIST_FOREACH_SAFE(cap, &radio->cac_cap.cac_capab_head, cac_cap_entry, t_cap) {
		if (is_channel_in_cac_cap(channel, cap, &bw, &opclass) == 1) {
			if (max_bw < bw) {
				max_bw = bw;
				total_cac_time =
					cap->cac_interval[0]*power(16,4) + cap->cac_interval[1]*power(16,2) + cap->cac_interval[2];
			}
		}
	}
	err(CH_PLANING_PREX"start cac timeout2 %u on channel %d", total_cac_time, channel);
	eloop_register_timeout(total_cac_time + 20,
			0,
			channel_cac_timeout2,
			ctx,
			NULL);
	return;
}

enum ch_set_by_bw
{
	BW_20_CH_SET = 0,
	BW_40_CH_SET = 2,
	BW_80_CH_SET = 4,
	BW_160_CH_SET = 8,
#ifdef MAP_320BW
	BW_320_CH_SET = 16,
#endif
};

u8 is_CAC_Success_by_bw(struct radio_info_db *radio, u8 channel)
{
	int bw = 0;
	u8 i = 0;
#ifdef MAP_320BW
	unsigned char channel_block[BW_320_CH_SET] = {0};
#else
	unsigned char channel_block[BW_160_CH_SET] = {0};
#endif
	info("CAC success op_class %d channel %d\n", radio->cac_comp_status.op_class, radio->cac_comp_status.channel);
	if (radio->cac_comp_status.channel == 0)
		return FALSE;

	bw = chan_mon_get_bw_from_op_class(radio->cac_comp_status.op_class);
	ch_planning_get_channel_block(radio->cac_comp_status.channel, channel_block, radio->cac_comp_status.op_class, bw);
#ifdef MAP_320BW
	for (i = 0; i < BW_320_CH_SET; i++) {
#else
	for (i = 0; i < BW_160_CH_SET; i++) {

#endif
		if(channel_block[i] == channel)
			return TRUE;
	}

	return FALSE;
}

u8 is_CAC_Success(
	struct mapd_global *global,
	u8 channel)
{
	struct own_1905_device *own_dev = &global->dev;
	struct _1905_map_device *_1905_dev = NULL, *t_1905_dev = NULL;
	struct radio_info_db *radio = NULL;
	u8 ret = FALSE;

	SLIST_FOREACH_SAFE(_1905_dev, &own_dev->_1905_dev_head, next_1905_device, t_1905_dev) {
		radio = topo_srv_get_radio_by_band(_1905_dev,channel);
		if (radio){
			if((radio->cac_comp_status.channel == channel) &&
				(radio->cac_comp_status.cac_status == CAC_SUCCESSFUL)) {
				return TRUE;
			} else if (radio->cac_comp_status.cac_status == CAC_SUCCESSFUL) {
				ret = is_CAC_Success_by_bw(radio, channel);
				if(ret == TRUE)
					return TRUE;
			}
		}
	}

	return FALSE;
}
u8 ch_planning_check_controller_cac_cap(
	struct own_1905_device *own_dev,
	u8 channel,
	u8 cac_mode)
{
	struct radio_info_db *radio = NULL;
	struct cac_cap_db *cap = NULL, *t_cap = NULL;
	struct _1905_map_device *_1905_dev = NULL;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);

	radio = topo_srv_get_radio_by_band(_1905_dev,channel);
	if(!radio) {
		err(CH_PLANING_PREX"radio(%d) not found", channel);
		return 0;
	}
	SLIST_FOREACH_SAFE(cap, &radio->cac_cap.cac_capab_head, cac_cap_entry, t_cap) {
		if(cap->cac_mode == cac_mode) {
			err(CH_PLANING_PREX"radio(%d) controller supports CAC type(%d)",
				radio->channel[0], cac_mode);
			return 1;
		}
	}
	debug(CH_PLANING_PREX"radio(%d) controller don't supports CAC type(%d)",
		radio->channel[0], cac_mode);

	return 0;
}
void ch_planning_handle_cac_failure(struct mapd_global * global, struct radio_info_db *radio,
						struct cac_completion_report * report, struct cac_status_report * status_report)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_map_device *dev = topo_srv_get_controller_device(ctx);

	if (dev == NULL)
		return;

	if (ctx->device_role == DEVICE_ROLE_CONTROLLER) {
		err(CH_PLANING_PREX"CAC_FAILURE on ch (%u)", report->cac_completion_status[0].channel);
		eloop_cancel_timeout(channel_cac_timeout2, ctx, NULL);
		radio->cac_comp_status.cac_status = CAC_FAILURE;
		ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
		ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_IDLE;
		dev->channel_planning_completed = TRUE;
		ch_planning_R2_reset(ctx, NULL);
	}
	if (ctx->device_role != DEVICE_ROLE_CONTROLLER) {
		err(CH_PLANING_PREX"agent send cac failure info to controller");
		if (ctx->map_version == DEV_TYPE_R2
#ifdef MAP_R3
		|| ctx->map_version == DEV_TYPE_R3
#endif
		)
			_1905_update_channel_pref_report(ctx, report, status_report);
		else
			_1905_update_channel_pref_report(ctx, report, NULL);

	};
}
void ch_planning_handle_cac_success_for_cont(struct mapd_global * global, struct radio_info_db *radio,
						struct cac_completion_report * report)
{
	struct own_1905_device *ctx = &global->dev;
	struct os_reltime rem_time = {0};

	err(CH_PLANING_PREX"radio ch state: %d, r2_ch_state: %d, R1_ch_state: %d\n",
		radio->dev_ch_plan_info.dev_ch_plan_state, ctx->ch_planning_R2.ch_plan_state, ctx->ch_planning.ch_planning_state);
	if ((global->dev.ch_planning_R2.ch_plan_state == CHPLAN_STATE_CAC_ONGOING) &&
		(radio->dev_ch_plan_info.dev_ch_plan_state == CHPLAN_STATE_CAC_ONGOING) &&
		((ctx->ch_planning.ch_planning_state == CHANNEL_PLANNING_CAC_START) || ctx->div_ch_planning == 1)) {

		if (report->cac_completion_status[0].channel != ctx->ch_planning_R2.CAC_on_channel) {
			err(CH_PLANING_PREX"Return, CAC req on %u, CAC success for %u\n", ctx->ch_planning_R2.CAC_on_channel,
				report->cac_completion_status[0].channel);
			return;
		}
		err(CH_PLANING_PREX"cancel cac timeouton ch %d",ctx->ch_planning_R2.CAC_on_channel);
		eloop_cancel_timeout(channel_cac_timeout2,ctx, NULL);

		ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
		ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
		ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED,radio->channel[0],&global->dev);

		debug(CH_PLANING_PREX"R1 ch planning is %d, make it 1 here", global->dev.ch_planning.ch_planning_enabled);
		global->dev.ch_planning.ch_planning_enabled = 1;
		mapd_restart_channel_plannig(global);

		radio->cac_enable = CAC_SUCCESSFUL;
		radio->cac_channel = 0;
		radio->cac_comp_status.cac_status = CAC_SUCCESSFUL;
		radio->cac_comp_status.channel = report->cac_completion_status[0].channel;//ctx->ch_planning_R2.CAC_on_channel;
		radio->cac_comp_status.op_class = report->cac_completion_status[0].op_class;//OP_CLASS;
	} else {

		radio->cac_enable = CAC_SUCCESSFUL;
		radio->cac_channel = 0;
		radio->cac_comp_status.cac_status = CAC_SUCCESSFUL;
		radio->cac_comp_status.channel = report->cac_completion_status[0].channel;//ctx->ch_planning_R2.CAC_on_channel;
		radio->cac_comp_status.op_class = report->cac_completion_status[0].op_class; //op_CLASS;
		/* Trigger NetOpt if cont is doing CAC(bootup CAC, CH PLAN might trigger direct switch CAC). So that after
		CAC NetOpt can bring the BH on 5G, This is useful in case of 605s of CAC */
		if (get_net_opt_dev_count(global) > 1) {
			eloop_get_remaining_timeout(&rem_time,trigger_net_opt,ctx,NULL);
			if (rem_time.sec == 0) {
				eloop_register_timeout(ctx->network_optimization.wait_time,
						0, trigger_net_opt, ctx, NULL);
			}
		}
	}
}

void ch_planning_trigger_cac_on_cont(struct mapd_global *global, u8 channel, unsigned char action, unsigned char cac_method)
{
	struct cac_request *cac = NULL;
	struct _1905_map_device *_1905_dev = NULL;
	struct own_1905_device *ctx = &global->dev;
	struct radio_info_db *radio = NULL;
	u8 *buff = NULL;
	u8 i = 0, opclass = 0;
	u16 length = 0;
	unsigned int total_cac_time = 0;
	struct cac_cap_db *cap = NULL, *t_cap = NULL;
	int bw = BW_20, max_bw = -1;

	buff = os_zalloc(sizeof(struct cac_request) + sizeof(struct cac_tlv));
	if(!buff) {
		err(CH_PLANING_PREX"malloc fail for buff");
		return;
	}

	_1905_dev = topo_srv_get_1905_device(ctx, NULL);
	radio = topo_srv_get_radio_by_band(_1905_dev, channel);

	if (radio == NULL) {
		err(CH_PLANING_PREX"radio is NULL for channel(%d) on current dev", channel);
		os_free(buff);
		return;
	}

	SLIST_FOREACH_SAFE(cap, &radio->cac_cap.cac_capab_head, cac_cap_entry, t_cap) {
		if (is_channel_in_cac_cap(channel, cap, &bw, &opclass) == 1) {
			if (max_bw < bw) {
				max_bw = bw;
				total_cac_time =
					cap->cac_interval[0]*power(16, 4) + cap->cac_interval[1]*power(16, 2) + cap->cac_interval[2];
			}
		}
	}

	cac = (struct cac_request *)buff;
	cac->num_radio = 1;

	for(i=0; i < cac->num_radio; i++) {
		os_memcpy(&cac->tlv[i].identifier,radio->identifier,ETH_ALEN);
		if (opclass == 0)
			cac->tlv[i].op_class_num = radio->operating_class;
		else
			cac->tlv[i].op_class_num = opclass;
		cac->tlv[i].ch_num = channel;
		cac->tlv[i].cac_method = cac_method;
		cac->tlv[i].cac_action = action;
		err(CH_PLANING_PREX"channel(%d) cac method(%d) cac action(%d) cac op(%u)", cac->tlv[i].ch_num,
			cac->tlv[i].cac_method, cac->tlv[i].cac_action, cac->tlv[i].op_class_num);
	}

	length = sizeof(struct cac_request) + cac->num_radio*sizeof(struct cac_tlv);
	wlanif_issue_wapp_command(global, WAPP_USER_SET_CAC_REQ,
			0, NULL, 0, buff, length, 0, 0, 0);

	radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CAC_ONGOING;
	global->dev.ch_planning_R2.ch_plan_state = CHPLAN_STATE_CAC_ONGOING;
	ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_CAC_START;
	global->dev.ch_planning_R2.CAC_on_channel = channel;

	err(CH_PLANING_PREX"start cac timeout2 for %d sec on channel %d", total_cac_time, channel);
	eloop_register_timeout(total_cac_time + 20,
		0,
		channel_cac_timeout2,
		ctx,
		NULL);
	os_free(buff);
}

void switch_temporary_channel(struct mapd_global *global)
{
	struct prefer_info_db *prefer_info = NULL, *t_prefer_info = NULL;
	struct channel_setting *setting = NULL;
	struct _1905_map_device *_1905_device = topo_srv_get_1905_device(&global->dev, NULL);
	int i = 0;
	struct radio_info_db *radio = topo_srv_get_radio_by_band(_1905_device, 36);
	if(!radio) {
		err("radio is NULL return");
		return;
	}
	setting = os_zalloc(512);
	if(!setting){
		err(CH_PLANING_PREX"alloc fail for setting");
		return;
	}

	err(CH_PLANING_PREX"on controller\n");
	SLIST_FOREACH_SAFE(prefer_info,
		&(radio->chan_preferance.prefer_info_head),
		prefer_info_entry, t_prefer_info) {
		int i =0;
		for (i = 0; i < prefer_info->ch_num; i++) {
			if (36 == prefer_info->ch_list[i])
				break;
		}
		if (36 == prefer_info->ch_list[i])
			break;
	}
	if(!prefer_info) {
		err(CH_PLANING_PREX"return, prefer_info is NULL");
		os_free(setting);
		return;
	}
	setting->ch_set_num = 1;
	setting->chinfo[i].channel = 36;
	//setting->chinfo[i].op_class = prefer_info->op_class;
	setting->chinfo[i].op_class = radio->operating_class;
	os_memcpy(setting->chinfo[i].identifier, radio->identifier, ETH_ALEN);
	setting->chinfo[i].reason_code = 0;

	wlanif_issue_wapp_command(global, WAPP_USER_SET_CHANNEL_SETTING, 0,
		NULL, NULL, setting, 512, 0, 0, 0);
	os_free(setting);
}

u8 ch_planning_dfs_rank_cac_mode(u8 cac_rank)
{
	switch(cac_rank)
	{
		case CAC_DEDICATED_RADIO_RANK:
			return DEDICATED_RADIO;
		case CAC_REDUCED_MIMO_RANK:
			return REDUCED_MIMO;
		case CAC_CONTINUOUS_RANK:
		default:
			return CONTINUOUS;
	}
}

u8 ch_planning_dfs_mode_cac_rank(u8 cac_mode)
{
	switch(cac_mode)
	{
		case DEDICATED_RADIO:
			return CAC_DEDICATED_RADIO_RANK;
		case REDUCED_MIMO:
			return CAC_REDUCED_MIMO_RANK;
		case CONTINUOUS:
		default:
			return CAC_CONTINUOUS_RANK;
	}
}

u8 ch_planning_check_max_bw_in_network (u8 num_dev_per_bw[])
{
	if (num_dev_per_bw[BW_160] != 0)
		return BW_160;
	else if (num_dev_per_bw[BW_8080] != 0)
		return BW_8080;
	else if (num_dev_per_bw[BW_80] != 0)
		return BW_80;
	else if (num_dev_per_bw[BW_40] != 0)
		return BW_40;
	else if (num_dev_per_bw[BW_20] != 0)
		return BW_20;
	else if (num_dev_per_bw[BW_10] != 0)
		return BW_10;
	else
		return BW_5;
}

u8 ch_planning_check_dfs_by_bw(
		struct own_1905_device *own_dev,
		u8 num_dev_per_bw[],
		u8 channel,
		u8 max_bw)
{
	struct _1905_map_device *_1905_dev = NULL, *t_1905_dev = NULL;
	struct _1905_map_device *_1905_dev_best = NULL;
	struct radio_info_db *radio = NULL;
	struct cac_cap_db *cap = NULL, *t_cap = NULL;
	int max_hop = -1;
	u8 cac_mode;
	u8 best_cac_mode = CAC_CONTINUOUS_RANK;
	u8 cac_method = CONTINUOUS;
	u8 cac_op_class = 0;
	u8 tmp_cac_op_class = 0;

	SLIST_FOREACH_SAFE(_1905_dev, &own_dev->_1905_dev_head, next_1905_device, t_1905_dev) {
		cac_mode = 0;
		if (_1905_dev->device_role != DEVICE_ROLE_AGENT)
			continue;

		if(_1905_dev->map_version != DEV_TYPE_R2
#ifdef MAP_R3
				&& _1905_dev->map_version != DEV_TYPE_R3
#endif
		  ) {
			err(CH_PLANING_PREX"agent("MACSTR") is not R2 cap , skip!!!", MAC2STR(_1905_dev->_1905_info.al_mac_addr));
			continue;
		}

		if(!_1905_dev->in_network) {
			debug(CH_PLANING_PREX"agent("MACSTR") not in network", MAC2STR(_1905_dev->_1905_info.al_mac_addr));
			continue;
		}

		radio = topo_srv_get_radio_by_band(_1905_dev,channel);
		if (radio == NULL)
			continue;

		if (radio->dev_ch_plan_info.dev_ch_plan_state == CHPLAN_STATE_CAC_TIMEOUT) {
			continue;
		}

		SLIST_FOREACH_SAFE(cap, &radio->cac_cap.cac_capab_head, cac_cap_entry, t_cap) {
			if (is_channel_in_cac_cap_by_bw(channel, cap, max_bw, &tmp_cac_op_class) == 0)
				continue;

			cac_mode = ch_planning_dfs_mode_cac_rank(cap->cac_mode);
			break;
		}

		if(cac_mode == 0 || best_cac_mode < cac_mode) {
			info(CH_PLANING_PREX"agent ("MACSTR")'s cac_mode %d does not have best_cac_mode %d",
					MAC2STR(_1905_dev->_1905_info.al_mac_addr), cac_mode, best_cac_mode);
			continue;
		}

		if (max_hop <= _1905_dev->root_distance) {
			best_cac_mode = cac_mode;
			max_hop = _1905_dev->root_distance;
			_1905_dev_best = _1905_dev;
			cac_method = ch_planning_dfs_rank_cac_mode(best_cac_mode);
			cac_op_class = tmp_cac_op_class;
			info(CH_PLANING_PREX"agent ("MACSTR") does not have best cac mode %d",
					MAC2STR(_1905_dev_best->_1905_info.al_mac_addr), best_cac_mode);
		}
	}

	if(_1905_dev_best == NULL) {
		err(CH_PLANING_PREX"No Agent found for CAC(%u)", channel);
		return DIRECT_SWITCH_CAC;
	} else {
		err(CH_PLANING_PREX"trigger cac on agent "MACSTR "",MAC2STR(_1905_dev_best->_1905_info.al_mac_addr));
		ch_planning_trigger_cac_msg2(own_dev->back_ptr, _1905_dev_best, channel, cac_method, cac_op_class);
		return CAC_ON_SELECT_DEV;
	}
}

void ch_planning_clear_old_cac(struct mapd_global *global, u8 channel)
{
	struct own_1905_device *own_dev = &global->dev;
	struct _1905_map_device *_1905_dev = NULL, *t_1905_dev = NULL;
	struct radio_info_db *radio = NULL;

	SLIST_FOREACH_SAFE(_1905_dev, &own_dev->_1905_dev_head, next_1905_device, t_1905_dev) {
		radio = topo_srv_get_radio_by_band(_1905_dev,channel);
		if (radio && (radio->cac_comp_status.cac_status == CAC_SUCCESSFUL)) {
			radio->cac_comp_status.cac_status = CAC_FAILURE;
			radio->cac_comp_status.channel = 0;
		}
	}
}

void ch_planning_check_old_cac_valid(struct mapd_global * global, u8 channel)
{
	struct own_1905_device *own_dev = &global->dev;
	if (ch_planning_is_ch_dfs(own_dev, channel) == FALSE) {
		ch_planning_clear_old_cac(global, channel);
	} else if (is_CAC_Success(global, channel) == FALSE) {
		ch_planning_clear_old_cac(global, channel);
	}
}


u8 ch_planning_check_dfs_by_bw_wrapper(struct mapd_global *global, u8 channel)
{
	struct _1905_map_device *_1905_dev = NULL, *t_1905_dev = NULL;
	u8 num_dev_per_bw[NUM_OF_BW] = {0};
	u8 max_bw = 0, radio_max_bw = 0;
	struct radio_info_db *radio = NULL;
	struct own_1905_device *own_dev = &global->dev;

	if(ch_planning_is_ch_dfs(&global->dev, channel)) {
		if (is_CAC_Success(global, channel) == 1) {
			err(CH_PLANING_PREX"CAC done on ch(%d), switch directly", channel);
			return DIRECT_SWITCH_CAC;
		} else if (global->dev.Restart_ch_planning_radar_on_agent) {
			//may cause link break
			err(CH_PLANING_PREX"Curr ch(%d) on cont is non-operable ,need to switch immediately",
					global->dev.Restart_ch_planning_radar_on_agent);
			return DIRECT_SWITCH_CAC;
		}
		if (global->dev.map_version == DEV_TYPE_R2
#ifdef MAP_R3
				|| global->dev.map_version == DEV_TYPE_R3
#endif
		   ) {
			/*looping to find out the number of devs per bw.*/
			SLIST_FOREACH_SAFE(_1905_dev, &own_dev->_1905_dev_head, next_1905_device, t_1905_dev) {
				if (_1905_dev->map_version != DEV_TYPE_R2
#ifdef MAP_R3
						&& _1905_dev->map_version != DEV_TYPE_R3
#endif
				   ) {
					err(CH_PLANING_PREX"agent("MACSTR") is not R2 cap , skip!!!", MAC2STR(_1905_dev->_1905_info.al_mac_addr));
					continue;
				}
				if (!_1905_dev->in_network) {
					debug(CH_PLANING_PREX"agent("MACSTR") not in network", MAC2STR(_1905_dev->_1905_info.al_mac_addr));
					continue;
				}

				radio = topo_srv_get_radio_by_band(_1905_dev, channel);
				if ((radio == NULL) || (radio->dev_ch_plan_info.dev_ch_plan_state == CHPLAN_STATE_CAC_TIMEOUT))
					continue;

				radio_max_bw = ch_planning_get_max_bw_1905dev_prefer_channel(own_dev, radio, _1905_dev, channel);

				num_dev_per_bw[radio_max_bw]++;
			}

			max_bw = ch_planning_check_max_bw_in_network(num_dev_per_bw);

			_1905_dev = NULL;
			radio = NULL;
			radio_max_bw = 0;

			_1905_dev = topo_srv_get_next_1905_device(own_dev, NULL);

			radio = topo_srv_get_radio_by_band(_1905_dev, channel);
			radio_max_bw = ch_planning_get_max_bw_1905dev_prefer_channel(own_dev, radio, _1905_dev, channel);
			if (max_bw == radio_max_bw) {
				info(CH_PLANING_PREX"max_bw:%d agent("MACSTR") bw:%d not max_bw of network",
						max_bw, MAC2STR(_1905_dev->_1905_info.al_mac_addr), radio_max_bw);

				if ((_1905_dev->device_role == DEVICE_ROLE_CONTROLLER) && own_dev->dedicated_radio) {
					err(CH_PLANING_PREX"trigger cac on cont radio(%d) mode(%d)", channel, DEDICATED_RADIO);
					ch_planning_trigger_cac_on_cont(own_dev->back_ptr, channel, 1, DEDICATED_RADIO);
					return CAC_ON_SELECT_DEV;
				}
			}

			/*looping to find the max_bw device.*/
			return (ch_planning_check_dfs_by_bw(own_dev, num_dev_per_bw, channel, max_bw));
		} else {
			if (ch_planning_check_controller_cac_cap(own_dev, channel, DEDICATED_RADIO)) {

				return CAC_ON_SELECT_DEV;
			} else {
				err(CH_PLANING_PREX"directly switch to cont ch(%u)", channel);
				return DIRECT_SWITCH_CAC;
			}
		}
	} else
		return NO_CAC;
}


u8 ch_planning_filter_channel(
	struct radio_info_db *radio,
	struct scan_result_tlv *scan_res)
{
	u8 band_max_bss;
	u8 band;
	band = get_band_from_channel(scan_res->channel);
	if (band == BAND_2G) {
		band_max_bss = MAX_BSS_NUM_2G;
	} else {
		band_max_bss = MAX_BSS_NUM_5G;
	}
	if(scan_res->neighbor_num > band_max_bss) {
		err(CH_PLANING_PREX"NeighboringBSSNum %d is over MaxBSSNum(%d)!!! skip",
			scan_res->neighbor_num, band_max_bss);
		return FILTER_OUT;
	}
	return FILTER_IN;
}
void ch_planning_agg_score(
	struct own_1905_device *own_dev,
	struct scan_result_tlv *scan_res)
{
	struct score_info *cumulative_score = NULL, *t_cumm_score = NULL;
	SLIST_FOREACH_SAFE(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score, t_cumm_score) {
		if(cumulative_score->channel == scan_res->channel &&
			cumulative_score->total_score != -65535) {
			cumulative_score->total_score =
				cumulative_score->total_score + scan_res->ch_score;
			cumulative_score->dev_count ++;
			break;
		}
	}
}
void ch_planning_avg_score(
	struct own_1905_device *own_dev)
{
	struct score_info *cumulative_score = NULL, *t_cumm_score = NULL;
	SLIST_FOREACH_SAFE(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score, t_cumm_score) {
		if(cumulative_score->dev_count !=0 )
			cumulative_score->avg_score =
				cumulative_score->total_score / cumulative_score->dev_count;
	}
}
u8 ch_planning_get_ch_rank(
	struct own_1905_device *own_dev,
	u8 channel)
{
	struct score_info *cumulative_score = NULL, *t_cumm_score = NULL;
	SLIST_FOREACH_SAFE(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score, t_cumm_score) {
		if(channel == cumulative_score->channel) {
			return cumulative_score->ch_rank;
		}
	}
	return 0;
}
u8 ch_planning_min_score_margin_check(
	struct own_1905_device *own_dev, 
	struct monitor_ch_info *ch_info)
{
	struct _1905_map_device *_1905_dev = NULL;
	struct radio_info_db *radio = NULL, *temp_radio = NULL;
	struct score_info *cumulative_score = NULL, *max_score_info = NULL, *current_op_ch_info = NULL, *t_cumm_score = NULL;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
	radio = topo_srv_get_radio_by_band(_1905_dev,ch_info->channel_num);
	u32 per_diff = 0;
	if(!radio) {
		err(CH_PLANING_PREX"radio(%d) not found!!!", ch_info->channel_num);
		return 0;
	}
	debug(CH_PLANING_PREX"check radio(%d)", radio->channel[0]);
	SLIST_FOREACH_SAFE(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score, t_cumm_score) {
		temp_radio = topo_srv_get_radio_by_band(_1905_dev,cumulative_score->channel);
		if(!temp_radio || temp_radio->band != radio->band)
			continue;
		if(!max_score_info)
			max_score_info = cumulative_score;
		if(cumulative_score->ch_rank > max_score_info->ch_rank) {
			max_score_info = cumulative_score;
		}
		if(cumulative_score->channel == radio->channel[0]) {
			current_op_ch_info = cumulative_score;
		}
	}
	if(!current_op_ch_info){
		debug(CH_PLANING_PREX"no current op ch info found in score table");
		return 0;
	}
	debug(CH_PLANING_PREX"max score info is ch %d, score %d, rank %d", max_score_info->channel, max_score_info->avg_score, max_score_info->ch_rank);
	debug(CH_PLANING_PREX"current ch score info is ch %d, score %d, rank %d", current_op_ch_info->channel, current_op_ch_info->avg_score, current_op_ch_info->ch_rank);
	if(current_op_ch_info->avg_score <= 0) {
		debug(CH_PLANING_PREX"current channel is not operable");
		return 0;
	}

	if(max_score_info->channel == current_op_ch_info->channel ||
		max_score_info->avg_score == current_op_ch_info->avg_score) {
		err(CH_PLANING_PREX"give higher rank to current channel(%d)", current_op_ch_info->channel);
		current_op_ch_info->ch_rank = max_score_info->ch_rank + 1;
		return 1;
	}
	per_diff = ((POSITIVE(max_score_info->avg_score - current_op_ch_info->avg_score))*100)/(current_op_ch_info->avg_score);
	err(CH_PLANING_PREX"percentage difference of max with current %d, margin %d", per_diff, own_dev->ch_planning_R2.min_score_inc);
	if(per_diff <= own_dev->ch_planning_R2.min_score_inc){
		err(CH_PLANING_PREX"still give highest rank to current channel(%d)", current_op_ch_info->channel);
		current_op_ch_info->ch_rank = max_score_info->ch_rank + 1;
		return 1;
	}
	return 0;
}
u8 ch_planning_modify_rank_DFS(
	struct own_1905_device *own_dev, 
	struct monitor_ch_info *ch_info)
{
	struct _1905_map_device *_1905_dev = NULL;
	struct radio_info_db *radio = NULL, *temp_radio = NULL;
	struct score_info *cumulative_score = NULL, *max_score_info_non_DFS = NULL, *max_score_info_DFS = NULL, *t_cumm_score = NULL;
	u8 check_DFS = 0;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
	radio = topo_srv_get_radio_by_band(_1905_dev,ch_info->channel_num);
	u32 per_diff = 0;
	if(!radio) {
		err(CH_PLANING_PREX"radio(%d) not found!!!", ch_info->channel_num);
		return 0;
	}
	debug(CH_PLANING_PREX"check radio(%d)", radio->channel[0]);
	/*find max non-DFS Ranked entry*/
	SLIST_FOREACH_SAFE(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score, t_cumm_score) {
		temp_radio = topo_srv_get_radio_by_band(_1905_dev,cumulative_score->channel);
		if(!temp_radio || temp_radio->band != radio->band)
			continue;
		check_DFS = 0;
		check_DFS = ch_planning_is_ch_dfs(own_dev, cumulative_score->channel);
		debug(CH_PLANING_PREX"check_DFS %d, cumulative_score->channel %d", check_DFS, cumulative_score->channel);
		if(!check_DFS) {
			if(!max_score_info_non_DFS)
				max_score_info_non_DFS = cumulative_score;
			if(cumulative_score->ch_rank > max_score_info_non_DFS->ch_rank) {
				max_score_info_non_DFS = cumulative_score;
			}
		} else {
			if(!max_score_info_DFS)
				max_score_info_DFS = cumulative_score;
			if(cumulative_score->ch_rank > max_score_info_DFS->ch_rank) {
				max_score_info_DFS = cumulative_score;
			}
		}
	}
	
	if(!max_score_info_DFS || !max_score_info_non_DFS){
		debug(CH_PLANING_PREX"no max info found");
		return 0;
	}
	debug(CH_PLANING_PREX"DFS max score info is ch %d, score %d, rank %d", max_score_info_DFS->channel, max_score_info_DFS->avg_score, max_score_info_DFS->ch_rank);
	debug(CH_PLANING_PREX"NON-DFS max score info is ch %d, score %d, rank %d", max_score_info_non_DFS->channel, max_score_info_non_DFS->avg_score, max_score_info_non_DFS->ch_rank);

	if(max_score_info_non_DFS->avg_score <= 0) {
		err(CH_PLANING_PREX"max non-DFS channel is not operable");
		return 0;
	}

	if(max_score_info_DFS->ch_rank < max_score_info_non_DFS->ch_rank) {
		err(CH_PLANING_PREX"non-DFS is already better than DFS , so no need for rank change");
		return 0;
	} 

	per_diff = ((POSITIVE(max_score_info_DFS->avg_score - max_score_info_non_DFS->avg_score))*100)/(max_score_info_non_DFS->avg_score);
	err(CH_PLANING_PREX"percentage difference of max with current %d", per_diff);
	if(per_diff <= own_dev->ch_planning_R2.min_score_inc){
		err(CH_PLANING_PREX"give highest rank to non-DFS channel");
		max_score_info_non_DFS->ch_rank = max_score_info_DFS->ch_rank + 1;
		return 1;
	}
	return 0;
}


void ch_planning_update_channel_rank(
	struct own_1905_device *own_dev, 
	struct monitor_ch_info *ch_info)
{
/*calculate the channel rank based on avg score Rank 1 is lowest */
	struct score_info *cumulative_score = NULL, *current_ch = NULL, *last_current_ch = NULL, *t_cumm_score = NULL;
	u8 current_rank = 1, is_current_ch_max = 0;
	
	while(1) {
		current_ch = NULL;
		SLIST_FOREACH_SAFE(cumulative_score,
					&own_dev->ch_planning_R2.first_ch_score, next_ch_score, t_cumm_score) {
			if(cumulative_score->ch_rank == 0){
				current_ch = cumulative_score;
				debug(CH_PLANING_PREX"curr ch %d",current_ch->channel);
				break;
			}
		}
		if(current_ch == NULL){
			err(CH_PLANING_PREX"all dev rank done\n");
			break;
		}
		t_cumm_score = NULL;
		SLIST_FOREACH_SAFE(cumulative_score,
			&own_dev->ch_planning_R2.first_ch_score, next_ch_score, t_cumm_score) {
			if(cumulative_score->ch_rank == 0){
				if (cumulative_score->avg_score <= current_ch->avg_score){
					current_ch = cumulative_score;
				}
			}
		}
		if(last_current_ch){
			if(last_current_ch->avg_score == current_ch->avg_score){
				current_ch->ch_rank = last_current_ch->ch_rank;
				debug(CH_PLANING_PREX"ch %d rank %d",current_ch->channel, current_ch->ch_rank);
				continue;
			}
		}
		last_current_ch = current_ch;
		current_ch->ch_rank = current_rank;
		debug(CH_PLANING_PREX"ch %d rank %d",current_ch->channel, current_ch->ch_rank);
		current_rank ++;
	}

/*means a new channel will be chosen , so in this case give more preference to non-DFS channel over DFS channel*/
	ch_planning_modify_rank_DFS(own_dev, ch_info);

/*give higher rank to current radio's channel if its score is not less that certain percentage from
the max rank score , so that there is no need to switch , as not much improvement*/
//find the channel with max rank , compare its score with current channel's score
	is_current_ch_max = ch_planning_min_score_margin_check(own_dev, ch_info);

}
#endif
int ch_planning_get_dev_bw_from_channel(
	struct _1905_map_device *_1905_dev,
	u8 channel)
{
	int contr_bw = BW_20;
	struct radio_info_db *radio = NULL;
	radio = topo_srv_get_radio_by_band(_1905_dev,channel);
	if (!radio) {
		err(CH_PLANING_PREX"radio(%d) not found skip grouping", channel);
		return -1;
	}
	contr_bw = chan_mon_get_bw_from_op_class(radio->operating_class);
	return contr_bw;
}
#ifdef MAP_R2
void dump_ch_group_by_bw_info(
	struct own_1905_device *own_dev)
{
	struct grp_score_info *grp_score = NULL, *t_grp_score = NULL;
	u8 i=0;
	err("******* Group Score Table************\n");
	SLIST_FOREACH_SAFE(grp_score,
		&own_dev->ch_planning_R2.first_grp_score, next_grp_score, t_grp_score) {
		for(i=0;i<grp_score->grp_channel_num;i++) {
			err("channel %d",grp_score->grp_channel_list[i]);
		}
		err("grp_total_avg_score %d, grp rank %d", grp_score->grp_total_avg_score,grp_score->grp_rank);
		debug("best ch %d", grp_score->best_ch);
	}
}
u8 ch_planning_find_best_ch_in_grp(
	struct own_1905_device *own_dev,
	u8 *grp_channel_list,
	u8 grp_channel_num)
{
	u8 best_ch = 0, max_rank = 1;
	u8 j = 0;
	struct score_info *cumulative_score = NULL, *t_cumm_score = NULL;
	if (!grp_channel_list) {
		err("ch list invalid");
		return 0;
	}
	best_ch = grp_channel_list[0];
	for(j = 0; j < grp_channel_num; j++) {
		SLIST_FOREACH_SAFE(cumulative_score,
			&own_dev->ch_planning_R2.first_ch_score,
			next_ch_score, t_cumm_score) {
			if(grp_channel_list[j] == cumulative_score->channel) {
				if(max_rank < cumulative_score->ch_rank) {
					max_rank = cumulative_score->ch_rank;
					best_ch = cumulative_score->channel;
				}
				break;
			}
		}
	}
	return best_ch;
}

void ch_planning_form_ch_grps(struct own_1905_device *own_dev)
{

//create groups based on BW
	struct grp_score_info *grp_score = NULL;
	u8 grp_num = MAX_BW_40_BLOCK, grp_channel_num = 2;
	u8 grp_bw_160[MAX_BW_160_BLOCK][8] = {
		 {36,40,44,48,52,56,60,64},
		 {100, 104, 108, 112, 116, 120, 124, 128},
		 {149, 153, 157, 161, 165, 169, 173, 177}
		};
	u8 grp_bw_80[MAX_BW_80_BLOCK][4] = {
		 {36,40,44,48},
		 {52,56,60,64},
		 {100,104,108,112},
		 {116,120,124,128},
		 {132,136,140,144},
		 {149, 153, 157, 161},
		 {165, 169, 173, 177}
		};
	u8 grp_bw_40[MAX_BW_40_BLOCK][2] = {
		 {36,40},
		 {44,48},
		 {52,56},
		 {60,64},
		 {100,104},
		 {108,112},
		 {116,120},
		 {124,128},
		 {132,136},
		 {140,144},
		 {149,153},
		 {157, 161},
		 {165, 169},
		 {173, 177}
		};
#ifdef MAP_320BW
	u8 grp_bw_320[MAX_BW_320_BLOCK][16] = {
		{1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61},
		{33, 37, 41, 45, 49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93},
		{65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125},
		{97, 101, 105, 109, 113, 117, 121, 125, 129, 133, 137, 141, 145, 149, 153, 157},
		{129, 133, 137, 141, 145, 149, 153, 157, 161, 165, 169, 173, 177, 181, 185, 189},
		{161, 165, 169, 173, 177, 181, 185, 189, 193, 197, 201, 205, 209, 213, 217, 221}
};
#endif
	u8 *grp_list = NULL;
	u8 i = 0, j = 0;
	struct score_info *cumulative_score = NULL, *t_cumm_score = NULL;
	if(own_dev->ch_planning_R2.grp_bw == BW_160) {
		grp_num = MAX_BW_160_BLOCK;
		grp_channel_num = 8;
		grp_list = &grp_bw_160[0][0];
	}else if (own_dev->ch_planning_R2.grp_bw == BW_80) {
		grp_num = MAX_BW_80_BLOCK;
		grp_channel_num = 4;
		grp_list = &grp_bw_80[0][0];
	}else if (own_dev->ch_planning_R2.grp_bw == BW_40) {
		grp_num = MAX_BW_40_BLOCK;
		grp_channel_num = 2;
		grp_list = &grp_bw_40[0][0];
	}
#ifdef MAP_320BW
	else if (own_dev->ch_planning_R2.grp_bw == BW_320) {
		grp_num = MAX_BW_40_BLOCK;
		grp_channel_num = 16;
		grp_list = &grp_bw_320[0][0];
	}
#endif
	debug(CH_PLANING_PREX"grp_num %d grp_channel_num %d grp_list %p grp_bw_40 %p grp_bw_80 %p, grp_bw_160 %p",
		grp_num, grp_channel_num, grp_list,
		grp_bw_40,
		grp_bw_80,
		grp_bw_160);
	for(i = 0; i < grp_num; i++) {
		grp_score = os_zalloc(sizeof(struct grp_score_info));
		if (grp_score == NULL) {
			err(CH_PLANING_PREX"alloc memory fail for grp_score");
			assert(0);
			return;
		}
		grp_score->grp_channel_num = grp_channel_num;
		grp_score->grp_rank = 0;
		grp_score->best_ch = 0;
		grp_score->best_ch_score = 0;
		if(grp_list == NULL) {
			err(CH_PLANING_PREX"grp_list is NULL");
			os_free(grp_score);
			return;
		}
		os_memcpy(&grp_score->grp_channel_list[0],(grp_list+(i*grp_channel_num)),grp_channel_num);
		debug(CH_PLANING_PREX"grp list ch[0] %d,ch[1] %d ch[2] %d ch[3] %d ", 
			grp_score->grp_channel_list[0],
			grp_score->grp_channel_list[1],
			grp_score->grp_channel_list[2],
			grp_score->grp_channel_list[3]);
		for(j = 0; j < grp_channel_num; j++) {
			SLIST_FOREACH_SAFE(cumulative_score,
				&own_dev->ch_planning_R2.first_ch_score,
				next_ch_score, t_cumm_score) {
				if(grp_score->grp_channel_list[j] == cumulative_score->channel) {
					grp_score->grp_total_avg_score =
						grp_score->grp_total_avg_score + cumulative_score->avg_score;
					debug(CH_PLANING_PREX"add score for ch %d total_avg_score %d",
						grp_score->grp_channel_list[j],
						grp_score->grp_total_avg_score);
					break;
				}	
				
			}
			
			if(!cumulative_score) {
				continue;
			}
			if (grp_score->best_ch == 0 ) {
				grp_score->best_ch = grp_score->grp_channel_list[j];
				grp_score->best_ch_score = cumulative_score->avg_score;
			} else if(grp_score->best_ch_score < cumulative_score->avg_score) {
				grp_score->best_ch = grp_score->grp_channel_list[j];
				grp_score->best_ch_score = cumulative_score->avg_score;
			} else if(grp_score->best_ch_score == cumulative_score->avg_score) {
				struct _1905_map_device *_1905_device = topo_srv_get_1905_device(own_dev,NULL);
				struct radio_info_db *temp_radio = topo_srv_get_radio_by_band(_1905_device,cumulative_score->channel);
				if(temp_radio) {
					if(temp_radio->channel[0] == cumulative_score->channel) {
						grp_score->best_ch = grp_score->grp_channel_list[j];
						grp_score->best_ch_score = cumulative_score->avg_score;
					}
				}
			}
		}
		debug(CH_PLANING_PREX"grp_score->grp_total_avg_score %d", grp_score->grp_total_avg_score);
		/*Update the best channel in the group based on the individual channel ranks calculated earlier*/
		grp_score->best_ch = ch_planning_find_best_ch_in_grp(own_dev,grp_score->grp_channel_list,grp_channel_num);
		err(CH_PLANING_PREX"grp_bw %d total grp avg score %d, best channel %d",
			own_dev->ch_planning_R2.grp_bw, grp_score->grp_total_avg_score, grp_score->best_ch);
		SLIST_INSERT_HEAD(&(own_dev->ch_planning_R2.first_grp_score),
			grp_score, next_grp_score);
	}
	
	//dump_ch_group_by_bw_info(own_dev);
}
void ch_planning_form_ch_grps_160_remaining(struct own_1905_device *own_dev, u8 band)
{
	struct grp_score_info *grp_score = NULL;
	u8 grp_num = 0,grp_channel_num = 0;
	u8 grp_bw_5gh_80_160[1][4] = {
		 {132, 136, 140, 144}
		};
	u8 *grp_list = NULL;
	u8 i = 0, j = 0;
	struct score_info *cumulative_score = NULL, *t_cumm_score = NULL;

	grp_num = 1;
	grp_channel_num = 4;
	grp_list = &grp_bw_5gh_80_160[0][0];

	err(CH_PLANING_PREX"grp_num(%u) band(%u) grp_ch_num(%u)", grp_num, band, grp_channel_num);
	for(i = 0; i < grp_num; i++) {
		grp_score = os_zalloc(sizeof(struct grp_score_info));
		if (grp_score == NULL) {
			err(CH_PLANING_PREX"alloc memory fail for grp_score");
			assert(0);
			return;
		}
		grp_score->grp_channel_num = grp_channel_num;
		grp_score->grp_rank = 0;
		grp_score->best_ch = 0;
		grp_score->best_ch_score = 0;
		if(grp_list == NULL) {
			err(CH_PLANING_PREX"grp_list is NULL");
			os_free(grp_score);
			return;
		}
		os_memcpy(&grp_score->grp_channel_list[0],(grp_list+(i*grp_channel_num)),grp_channel_num);
		debug(CH_PLANING_PREX"grp list ch[0] %d,ch[1] %d ch[2] %d ch[3] %d ", 
			grp_score->grp_channel_list[0],
			grp_score->grp_channel_list[1],
			grp_score->grp_channel_list[2],
			grp_score->grp_channel_list[3]);
		for(j = 0; j < grp_channel_num; j++) {
			SLIST_FOREACH_SAFE(cumulative_score,
				&own_dev->ch_planning_R2.first_ch_score,
				next_ch_score, t_cumm_score) {
				if(grp_score->grp_channel_list[j] == cumulative_score->channel) {
					grp_score->grp_total_avg_score =
						grp_score->grp_total_avg_score + cumulative_score->avg_score;
					debug(CH_PLANING_PREX"add score for ch %d total_avg_score %d",
						grp_score->grp_channel_list[j],
						grp_score->grp_total_avg_score);
					break;
				}
			}

			if(!cumulative_score) {
				continue;
			}
			if (grp_score->best_ch == 0) {
				grp_score->best_ch = grp_score->grp_channel_list[j];
				grp_score->best_ch_score = cumulative_score->avg_score;
			} else if(grp_score->best_ch_score < cumulative_score->avg_score) {
				grp_score->best_ch = grp_score->grp_channel_list[j];
				grp_score->best_ch_score = cumulative_score->avg_score;
			} else if(grp_score->best_ch_score == cumulative_score->avg_score) {
				struct _1905_map_device *_1905_device = topo_srv_get_1905_device(own_dev,NULL);
				struct radio_info_db *temp_radio = topo_srv_get_radio_by_band(_1905_device,cumulative_score->channel);
				if(temp_radio) {
					if(temp_radio->channel[0] == cumulative_score->channel) {
						grp_score->best_ch = grp_score->grp_channel_list[j];
						grp_score->best_ch_score = cumulative_score->avg_score;
						debug(CH_PLANING_PREX"radio ch: %u, best ch: %u, score: %d", temp_radio->channel[0], grp_score->best_ch, grp_score->best_ch_score);
					}
				}
			}
		}
		/*Update the best channel in the group based on the individual channel ranks calculated earlier*/
		grp_score->best_ch = ch_planning_find_best_ch_in_grp(own_dev,grp_score->grp_channel_list,grp_channel_num);
		debug(CH_PLANING_PREX"grp_score->grp_total_avg_score %d", grp_score->grp_total_avg_score);
		err(CH_PLANING_PREX"total grp avg score %d, best channel %d",
			grp_score->grp_total_avg_score, grp_score->best_ch);
		SLIST_INSERT_HEAD(&(own_dev->ch_planning_R2.first_grp_score),
			grp_score, next_grp_score);
	}
}


void ch_planning_form_ch_grps_triband(struct own_1905_device *own_dev, u8 band)
{

	/*create groups based on 5gl BW or 5gh BW*/
	struct grp_score_info *grp_score = NULL;
	u8 grp_num = 12,grp_channel_num = 2;
	u8 grp_bw_5gl_160[1][8] = {
		 {36,40,44,48,52,56,60,64},
		};
	u8 grp_bw_5gh_160[2][8] = {
		 {100, 104, 108, 112, 116, 120, 124, 128},
		 {149, 153, 157, 161, 165, 169, 173, 177},
		};
	u8 grp_bw_5gl_80[2][4] = {
		 {36,40,44,48},
		 {52,56,60,64},
		};
	u8 grp_bw_5gh_80[5][4] = {
		 {100,104,108,112},
		 {116,120,124,128},
		 {132,136,140,144},
		 {149, 153, 157, 161},
		 {165, 169, 173, 177}
		};
	u8 grp_bw_5gl_40[4][2] = {
		 {36,40},
		 {44,48},
		 {52,56},
		 {60,64},
		};
	u8 grp_bw_5gh_40[10][2] = {
		 {100,104},
		 {108,112},
		 {116,120},
		 {124,128},
		 {132,136},
		 {140,144},
		 {149,153},
		 {157, 161},
		 {165, 169},
		 {173, 177}
		};
#ifdef MAP_320BW
	u8 grp_bw_320[MAX_BW_320_BLOCK][16] = {
		{1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61},
		{33, 37, 41, 45, 49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93},
		{65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125},
		{97, 101, 105, 109, 113, 117, 121, 125, 129, 133, 137, 141, 145, 149, 153, 157},
		{129, 133, 137, 141, 145, 149, 153, 157, 161, 165, 169, 173, 177, 181, 185, 189},
		{161, 165, 169, 173, 177, 181, 185, 189, 193, 197, 201, 205, 209, 213, 217, 221}
};
#endif
	u8 *grp_list = NULL;
	u8 i = 0, j = 0;
	struct score_info *cumulative_score = NULL, *t_cumm_score = NULL;
	if(own_dev->ch_planning_R2.grp_bw == BW_160) {
		if (band == BAND_5GL) {
			grp_num = 1;
			grp_channel_num = 8;
			grp_list = &grp_bw_5gl_160[0][0];
		} else if (band == BAND_5GH) {
			grp_num = 1;
			grp_channel_num = 8;
			grp_list = &grp_bw_5gh_160[0][0];
		}
	} else if (own_dev->ch_planning_R2.grp_bw == BW_80) {
		if (band == BAND_5GL) {
			grp_num = 2;
			grp_channel_num = 4;
			grp_list = &grp_bw_5gl_80[0][0];
		} else if (band == BAND_5GH) {
			grp_num = 4;
			grp_channel_num = 4;
			grp_list = &grp_bw_5gh_80[0][0];
		}
	} else if (own_dev->ch_planning_R2.grp_bw == BW_40) {
		if (band == BAND_5GL) {
			grp_num = 4;
			grp_channel_num = 2;
			grp_list = &grp_bw_5gl_40[0][0];
		} else if (band == BAND_5GH) {
			grp_num = 8;
			grp_channel_num = 2;
			grp_list = &grp_bw_5gh_40[0][0];
		}
	}
#ifdef MAP_320BW
	else if (own_dev->ch_planning_R2.grp_bw == BW_320) {
		grp_num = MAX_BW_320_BLOCK;
		grp_channel_num = 16;
		grp_list = &grp_bw_320[0][0];
	}
#endif
	err(CH_PLANING_PREX"grp_num(%u) band(%u) grp_ch_num(%u)", grp_num, band, grp_channel_num);
	for(i = 0; i < grp_num; i++) {
		grp_score = os_zalloc(sizeof(struct grp_score_info));
		if (grp_score == NULL) {
			err(CH_PLANING_PREX"alloc memory fail for grp_score");
			assert(0);
			return;
		}
		grp_score->grp_channel_num = grp_channel_num;
		grp_score->grp_rank = 0;
		grp_score->best_ch = 0;
		grp_score->best_ch_score = 0;
		if(grp_list == NULL) {
			err(CH_PLANING_PREX"grp_list is NULL");
			os_free(grp_score);
			return;
		}
		os_memcpy(&grp_score->grp_channel_list[0],(grp_list+(i*grp_channel_num)),grp_channel_num);
		debug(CH_PLANING_PREX"grp list ch[0] %d,ch[1] %d ch[2] %d ch[3] %d ",
			grp_score->grp_channel_list[0],
			grp_score->grp_channel_list[1],
			grp_score->grp_channel_list[2],
			grp_score->grp_channel_list[3]);
		for(j = 0; j < grp_channel_num; j++) {
			SLIST_FOREACH_SAFE(cumulative_score,
				&own_dev->ch_planning_R2.first_ch_score,
				next_ch_score, t_cumm_score) {
				if(grp_score->grp_channel_list[j] == cumulative_score->channel) {
					grp_score->grp_total_avg_score =
						grp_score->grp_total_avg_score + cumulative_score->avg_score;
					debug(CH_PLANING_PREX"add score for ch %d total_avg_score %d",
						grp_score->grp_channel_list[j],
						grp_score->grp_total_avg_score);
					break;
				}
			}

			if(!cumulative_score) {
				continue;
			}
			if (grp_score->best_ch == 0) {
				grp_score->best_ch = grp_score->grp_channel_list[j];
				grp_score->best_ch_score = cumulative_score->avg_score;
			} else if(grp_score->best_ch_score < cumulative_score->avg_score) {
				grp_score->best_ch = grp_score->grp_channel_list[j];
				grp_score->best_ch_score = cumulative_score->avg_score;
			} else if(grp_score->best_ch_score == cumulative_score->avg_score) {
				struct _1905_map_device *_1905_device = topo_srv_get_1905_device(own_dev,NULL);
				struct radio_info_db *temp_radio = topo_srv_get_radio_by_band(_1905_device,cumulative_score->channel);
				if(temp_radio) {
					if(temp_radio->channel[0] == cumulative_score->channel) {
						grp_score->best_ch = grp_score->grp_channel_list[j];
						grp_score->best_ch_score = cumulative_score->avg_score;
						debug(CH_PLANING_PREX"radio ch: %u, best ch: %u, score: %d", temp_radio->channel[0], grp_score->best_ch, grp_score->best_ch_score);
					}
				}
			}
		}
		/*Update the best channel in the group based on the individual channel ranks calculated earlier*/
		grp_score->best_ch = ch_planning_find_best_ch_in_grp(own_dev,grp_score->grp_channel_list,grp_channel_num);
		debug(CH_PLANING_PREX"grp_score->grp_total_avg_score %d", grp_score->grp_total_avg_score);
		err(CH_PLANING_PREX"total grp avg score %d, best channel %d",
			grp_score->grp_total_avg_score, grp_score->best_ch);
		SLIST_INSERT_HEAD(&(own_dev->ch_planning_R2.first_grp_score),
			grp_score, next_grp_score);
	}
}


void ch_planning_remove_grp_score_table(
	struct own_1905_device *own_dev, u8 band, Boolean is_triband)
{
	struct grp_score_info *grp_score = NULL, *t_grp_score = NULL;

	if (!is_triband) {
		while (!SLIST_EMPTY(&(own_dev->ch_planning_R2.first_grp_score))) {
			grp_score = SLIST_FIRST(&(own_dev->ch_planning_R2.first_grp_score));
			SLIST_REMOVE_HEAD(&(own_dev->ch_planning_R2.first_grp_score),
				next_grp_score);
			os_free(grp_score);
		}
	} else if (is_triband) {
		SLIST_FOREACH_SAFE(grp_score,
			&own_dev->ch_planning_R2.first_grp_score, next_grp_score, t_grp_score) {
			if (band == BAND_5GL && grp_score->best_ch <= 64) {
				SLIST_REMOVE(&(own_dev->ch_planning_R2.first_grp_score),
					grp_score,
					grp_score_info,
					next_grp_score);
				os_free(grp_score);
			} else if (band == BAND_5GH && grp_score->best_ch >= 100) {
				SLIST_REMOVE(&(own_dev->ch_planning_R2.first_grp_score),
					grp_score,
					grp_score_info,
					next_grp_score);
				os_free(grp_score);
			}
		}
	}
}
u8 ch_planning_get_grp_rank(
	struct own_1905_device *own_dev,
	u8 channel)
{
	struct grp_score_info *grp_score = NULL, *t_grp_score = NULL;
	u8 i = 0;
	SLIST_FOREACH_SAFE(grp_score,
		&own_dev->ch_planning_R2.first_grp_score, next_grp_score, t_grp_score) {
		for(i = 0; i < grp_score->grp_channel_num; i++) {
			if(channel == grp_score->grp_channel_list[i]) {
				if(channel == grp_score->best_ch) {
					return (grp_score->grp_rank+1);
				}
				return grp_score->grp_rank;
			}
		}
	}
	return 0;
}
u8 ch_planning_all_grp_rank_done(
	struct own_1905_device *own_dev)
{
	struct grp_score_info *grp_score = NULL, *t_grp_score = NULL;
	SLIST_FOREACH_SAFE(grp_score,
			&own_dev->ch_planning_R2.first_grp_score, next_grp_score, t_grp_score) {
			err(CH_PLANING_PREX"dump grp wth ch %d, rank %d",grp_score->best_ch, grp_score->grp_rank);
		if(grp_score->grp_rank == 0)
			return 0;
	}
	return 1;
}
u8 ch_planning_grp_score_margin_check(
	struct own_1905_device *own_dev, 
	u8 channel, u8 band, Boolean is_triband)
{
	struct _1905_map_device *_1905_dev = NULL;
	struct radio_info_db *radio = NULL, *temp_radio = NULL;
	struct grp_score_info *grp_score = NULL, *max_score_info = NULL, *current_op_grp_info = NULL, *t_grp_score = NULL;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
	radio = topo_srv_get_radio_by_band(_1905_dev,channel);
	u32 per_diff = 0;
	u8 i = 0;
	if(!radio) {
		err(CH_PLANING_PREX"radio(%d) not found skip score cal", channel);
		return 0;
	}
	debug(CH_PLANING_PREX"check for radio with ch %d", radio->channel[0]);

	SLIST_FOREACH_SAFE(grp_score,
		&own_dev->ch_planning_R2.first_grp_score, next_grp_score, t_grp_score) {
		temp_radio = topo_srv_get_radio_by_band(_1905_dev,channel);
		if(!temp_radio || temp_radio->band != radio->band)
			continue;
		if (is_triband) {
			if (band == BAND_5GL && grp_score->best_ch > 64) {
				continue;
			} else if (band == BAND_5GH && grp_score->best_ch <= 64) {
				continue;
			}
		}

		if(!max_score_info)
			max_score_info = grp_score;
		if(grp_score->grp_rank > max_score_info->grp_rank) {
			max_score_info = grp_score;
		}
		for (i = 0; i < grp_score->grp_channel_num; i++) {
			if(grp_score->grp_channel_list[i] == radio->channel[0]) {
				current_op_grp_info = grp_score;
				break;
			}
		}
	}
	if(!current_op_grp_info){
		err(CH_PLANING_PREX"no current op grp info found in score table");
		return 0;
	}
	debug(CH_PLANING_PREX"max score info is ch %d, score %d, rank %d", max_score_info->grp_channel_list[0], max_score_info->grp_total_avg_score, max_score_info->grp_rank);
	debug(CH_PLANING_PREX"current grp score info is ch %d, score %d, rank %d", current_op_grp_info->grp_channel_list[0], current_op_grp_info->grp_total_avg_score, current_op_grp_info->grp_rank);
	if(current_op_grp_info->grp_total_avg_score <= 0) {
		err(CH_PLANING_PREX"current grp is not operable");
		return 0;
	}

	if(max_score_info->grp_channel_list[0]== current_op_grp_info->grp_channel_list[0] ||
		max_score_info->grp_total_avg_score == current_op_grp_info->grp_total_avg_score) {
		debug(CH_PLANING_PREX"give higher rank to current group");
		current_op_grp_info->grp_rank = max_score_info->grp_rank + 1;
		return 1;
	}
	per_diff = ((POSITIVE(max_score_info->grp_total_avg_score - current_op_grp_info->grp_total_avg_score))*100)/(current_op_grp_info->grp_total_avg_score);
	debug(CH_PLANING_PREX"percentage difference of max with current %d, margin %d", per_diff, own_dev->ch_planning_R2.min_score_inc);
	if(per_diff <= own_dev->ch_planning_R2.min_score_inc){
		debug(CH_PLANING_PREX"still give highest rank to current grp");
		current_op_grp_info->grp_rank = max_score_info->grp_rank + 1;
		return 1;
	}
	return 0;
}
u8 ch_planning_modify_grp_rank_DFS(
	struct own_1905_device *own_dev, 
	u8 channel, u8 band, Boolean is_triband)
{
	struct _1905_map_device *_1905_dev = NULL;
	struct radio_info_db *radio = NULL, *temp_radio = NULL;
	struct grp_score_info *cumulative_score = NULL, *max_score_info_non_DFS = NULL, *max_score_info_DFS = NULL, *t_cumm_score = NULL;
	u8 check_DFS = 0;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
	radio = topo_srv_get_radio_by_band(_1905_dev,channel);
	u32 per_diff = 0;
	if(!radio) {
		err(CH_PLANING_PREX"radio(%d) not found", channel);
		return 0;
	}
	debug(CH_PLANING_PREX"check for radio with ch %d", radio->channel[0]);
	/*find max non-DFS Ranked entry*/
	SLIST_FOREACH_SAFE(cumulative_score,
		&own_dev->ch_planning_R2.first_grp_score, next_grp_score, t_cumm_score) {
		temp_radio = topo_srv_get_radio_by_band(_1905_dev,cumulative_score->grp_channel_list[0]);
		if(!temp_radio || temp_radio->band != radio->band)
			continue;
		if (is_triband) {
			if (band == BAND_5GL && cumulative_score->best_ch > 64) {
				continue;
			} else if (band == BAND_5GH && cumulative_score->best_ch <= 64) {
				continue;
			}
		}
		check_DFS = 0;
		check_DFS = ch_planning_is_ch_dfs(own_dev, cumulative_score->grp_channel_list[0]);
		debug(CH_PLANING_PREX"check_DFS %d, cumulative_score->channel %d", check_DFS, cumulative_score->grp_channel_list[0]);
		if(!check_DFS) {
			if(!max_score_info_non_DFS)
				max_score_info_non_DFS = cumulative_score;
			if(cumulative_score->grp_rank > max_score_info_non_DFS->grp_rank) {
				max_score_info_non_DFS = cumulative_score;
			}
		} else {
			if(!max_score_info_DFS)
				max_score_info_DFS = cumulative_score;
			if(cumulative_score->grp_rank > max_score_info_DFS->grp_rank) {
				max_score_info_DFS = cumulative_score;
			}
		}
	}
	
	if(!max_score_info_DFS || !max_score_info_non_DFS){
		debug(CH_PLANING_PREX"no max info found");
		return 0;
	}
	debug(CH_PLANING_PREX"DFS max score info is ch %d, score %d, rank %d", max_score_info_DFS->grp_channel_list[0], max_score_info_DFS->grp_total_avg_score, max_score_info_DFS->grp_rank);
	debug(CH_PLANING_PREX"NON-DFS max score info is ch %d, score %d, rank %d", max_score_info_non_DFS->grp_channel_list[0], max_score_info_non_DFS->grp_total_avg_score, max_score_info_non_DFS->grp_rank);

	if(max_score_info_non_DFS->grp_total_avg_score <= 0) {
		debug(CH_PLANING_PREX"max non-DFS channel is not operable");
		return 0;
	}

	if(max_score_info_DFS->grp_rank < max_score_info_non_DFS->grp_rank) {
		debug(CH_PLANING_PREX"non-DFS is already better than DFS , so no need for rank change");
		return 0;
	} 

	per_diff = ((POSITIVE(max_score_info_DFS->grp_total_avg_score - max_score_info_non_DFS->grp_total_avg_score))*100)/(max_score_info_non_DFS->grp_total_avg_score);
	debug(CH_PLANING_PREX"percentage difference of max with current %d", per_diff);
	if(per_diff <= own_dev->ch_planning_R2.min_score_inc){
		debug(CH_PLANING_PREX"give highest rank to non-DFS channel");
		max_score_info_non_DFS->grp_rank = max_score_info_DFS->grp_rank + 1;
		return 1;
	}
	return 0;
}

void ch_planning_update_grp_rank(
	struct own_1905_device *own_dev, 
	u8 channel, u8 band, Boolean is_triband)
{
	struct grp_score_info *cumulative_score = NULL, *current_ch = NULL, *last_current_ch = NULL, *t_cumm_score = NULL;
	u8 current_rank = 1,is_current_grp_max = 0;;
	
	while(1) {
		current_ch = NULL;
		SLIST_FOREACH_SAFE(cumulative_score,
					&own_dev->ch_planning_R2.first_grp_score, next_grp_score, t_cumm_score) {
			if(cumulative_score->grp_rank == 0){
				if (is_triband) {
					if (band == BAND_5GL && cumulative_score->best_ch <= 64) {
 						current_ch = cumulative_score;
						break;
					} else if (band == BAND_5GH && cumulative_score->best_ch >= 100) {
						current_ch = cumulative_score;
						break;
					}
				} else {
					current_ch = cumulative_score;
					break;
				}
			}
		}
		if(current_ch == NULL){
			err(CH_PLANING_PREX"all dev rank done\n");
			break;
		}
		SLIST_FOREACH_SAFE(cumulative_score,
			&own_dev->ch_planning_R2.first_grp_score, next_grp_score, t_cumm_score) {
			if (cumulative_score->grp_rank == 0) {
				if (is_triband) {
					if (band == BAND_5GL && cumulative_score->best_ch <= 64) {
						if (cumulative_score->grp_total_avg_score <= current_ch->grp_total_avg_score){
							current_ch = cumulative_score;
						}
					} else if (band == BAND_5GH && cumulative_score->best_ch >= 100) {
						if (cumulative_score->grp_total_avg_score <= current_ch->grp_total_avg_score){
							current_ch = cumulative_score;
						}
					}
				} else {
					if (cumulative_score->grp_total_avg_score <= current_ch->grp_total_avg_score){
						current_ch = cumulative_score;
					}
				}
			}
		}
		if(last_current_ch){
			if(last_current_ch->grp_total_avg_score == current_ch->grp_total_avg_score){
				current_ch->grp_rank = last_current_ch->grp_rank;
				debug(CH_PLANING_PREX"ch %d rank %d",current_ch->best_ch, current_ch->grp_rank);
				continue;
			}
		}
		last_current_ch = current_ch;
		current_ch->grp_rank = current_rank;
		debug(CH_PLANING_PREX"ch %d rank %d",current_ch->best_ch, current_ch->grp_rank);
		current_rank ++;
	}


//after update group ranks dump
	//err("after update group rank, dump group by bw info");
	//dump_ch_group_by_bw_info(own_dev);
	ch_planning_modify_grp_rank_DFS(own_dev,channel, band, is_triband);
	is_current_grp_max = ch_planning_grp_score_margin_check(own_dev,channel, band, is_triband);
	debug(CH_PLANING_PREX"after score margin check dump group by bw info, is_current_grp_max %d", is_current_grp_max);

	//dump_ch_group_by_bw_info(own_dev);
return;
}

void ch_planning_group_ch_by_bw(
	struct own_1905_device *own_dev,
	struct monitor_ch_info *ch_info)
{
	struct _1905_map_device *_1905_dev = NULL;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
	Boolean is_triband = FALSE;
	struct radio_info_db *radio = NULL;

	if(ch_info->channel_num <=14) {
		err(CH_PLANING_PREX"this feature is not supported for 2.4G");
		return;
	}
	radio = topo_srv_get_radio_by_band(_1905_dev, ch_info->channel_num);
	if (!radio) {
		err(CH_PLANING_PREX"radio is NULL");
		return;
	}
	own_dev->ch_planning_R2.grp_bw = ch_planning_get_max_bw_1905dev_prefer(own_dev, radio, _1905_dev);
	err(CH_PLANING_PREX"BW based grouping TBD by BW %d", own_dev->ch_planning_R2.grp_bw);

	if(own_dev->ch_planning_R2.grp_bw == BW_20) {
		err(CH_PLANING_PREX"no need for BW based grouping as grp bw is 20MHz");
		return;
	}

	is_triband = check_is_triband(_1905_dev);
	//based on this BW make link list of score with bw
	//clean-up the existing list if any
	ch_planning_remove_grp_score_table(own_dev, ch_info->band, is_triband);
	if (SLIST_EMPTY(&(own_dev->ch_planning_R2.first_grp_score))) {
		SLIST_INIT(&own_dev->ch_planning_R2.first_grp_score);
	}
	//create channel groups based on BW
	if (is_triband) {
		ch_planning_form_ch_grps_triband(own_dev, ch_info->band);
		if (own_dev->ch_planning_R2.grp_bw == BW_160 && ch_info->band == BAND_5GH)
			ch_planning_form_ch_grps_160_remaining(own_dev, ch_info->band);
	} else {
		ch_planning_form_ch_grps(own_dev);
		if (own_dev->ch_planning_R2.grp_bw == BW_160)
			ch_planning_form_ch_grps_160_remaining(own_dev, ch_info->band);
	}

	//update the rank of each ch group
	ch_planning_update_grp_rank(own_dev, ch_info->channel_num, ch_info->band, is_triband);
}

u8 ch_planning_is_ch_switch_req(
	struct own_1905_device *own_dev,
	struct monitor_ch_info *ch_info,
	struct score_info *best_channel_info)
{
	struct radio_info_db *temp_radio= NULL;
	u16 min_score_inc = own_dev->ch_planning_R2.min_score_inc;
	struct score_info *current_working_ch_info = NULL;

	/*current working radio info*/
	temp_radio =
		topo_srv_get_radio_by_channel(
		SLIST_FIRST(&own_dev->_1905_dev_head),
		ch_info->channel_num);
	if(!temp_radio) {
		err(CH_PLANING_PREX"radio not found with channel %d",ch_info->channel_num);
		return 0;
	}
	debug(CH_PLANING_PREX"best_channel %d,radio->channel %d",
		best_channel_info->channel, temp_radio->channel[0]);
	if (best_channel_info->channel != temp_radio->channel[0]) {
		err(CH_PLANING_PREX"current channel is different from best channel ");
		current_working_ch_info =
			ch_planning_find_current_ch_score_info(own_dev, temp_radio);
		if(!current_working_ch_info) {
			err(CH_PLANING_PREX"current working ch_info not found ");
			return 0;
		}
		err(CH_PLANING_PREX"current_working ch %d, score %d", current_working_ch_info->channel,
			current_working_ch_info->total_score);
		/*check score margin*/
		if (best_channel_info->total_score >
			current_working_ch_info->total_score + min_score_inc) {
			err(CH_PLANING_PREX"need to trigger ch switch");
			return 1;
		} else {
			err(CH_PLANING_PREX"no need to trigger ch change as score margin not met");
			return 0;
		}
	} else {
		err(CH_PLANING_PREX"no need to trigger ch change as score of current ch is the best");
	}
	return 0;
}
void ch_planning_select_best_channel_R2(
	struct mapd_global *global,
	struct monitor_ch_info *ch_info)
{
	struct own_1905_device *own_dev = &global->dev;
	struct _1905_map_device *_1905_dev = NULL, *t_1905_dev = NULL;
	struct radio_info_db *radio = NULL;
	struct scan_result_tlv *scan_res = NULL, *t_scan_res = NULL;
	//struct score_info *best_channel_info = NULL;
	u8 filter_status;//, trigger_status;
	if(!ch_info) {
		err(CH_PLANING_PREX"ch_info is NULL!!!!!! ");
		_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
		radio = topo_srv_get_next_radio(_1905_dev, NULL);
		while (radio) {
			ch_planning_R2_reset(own_dev,radio);
			radio = topo_srv_get_next_radio(_1905_dev, radio);
		}
		handle_task_completion(own_dev);
		return;
	}
	ch_planning_clear_score_table(own_dev, ch_info);
	SLIST_FOREACH_SAFE(_1905_dev, &own_dev->_1905_dev_head, next_1905_device, t_1905_dev) {
		radio = topo_srv_get_radio_by_band(_1905_dev,ch_info->channel_num);
		if(!radio) {
			err(CH_PLANING_PREX"radio(%d) not found!!!", ch_info->channel_num);
			continue;
		}
		SLIST_FOREACH_SAFE(scan_res, &radio->first_scan_result, next_scan_result, t_scan_res) {
			/*ch_planning_filter_unqualified_channels*/
			filter_status = ch_planning_filter_channel(radio, scan_res);
			if (filter_status == FILTER_OUT) {
				scan_res->ch_score = 0;
				struct score_info *cumulative_score = NULL, *t_cumm_score = NULL;
				SLIST_FOREACH_SAFE(cumulative_score,
					&own_dev->ch_planning_R2.first_ch_score, next_ch_score, t_cumm_score) {
					if(cumulative_score->channel == scan_res->channel) {
						cumulative_score->total_score = -65535;
					}
				}
				continue;
			}
			/*channel score calculation*/
			ch_planning_calc_score(own_dev,_1905_dev,radio,scan_res);
			/*keep aggregating scores from all agents*/
			ch_planning_agg_score(own_dev,scan_res);
		} //all scan result loop
	}//all 1905 device loop

	ch_planning_avg_score(own_dev);
	ch_planning_update_channel_rank(own_dev, ch_info);
	//dump_ch_planning_score_info(own_dev);
	if(own_dev->ch_planning_R2.ch_plan_enable_bw){
		ch_planning_group_ch_by_bw(own_dev, ch_info);
	}
	own_dev->ch_planning_R2.ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
#ifndef MAP_6E_SUPPORT
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
	radio = topo_srv_get_radio_by_band(_1905_dev,ch_info->channel_num);
	if(radio)
		ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED,radio->channel[0],own_dev);
#else
	ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED,ch_info->channel_num,own_dev);
#endif
	own_dev->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
	mapd_restart_channel_plannig(global);
}

u8 ch_planning_check_scan_result(struct own_1905_device *own_dev,
			struct monitor_ch_info *ch_info)
{
	struct _1905_map_device *dev = NULL, *t_dev = NULL;
	struct radio_info_db *temp_radio = NULL;
	struct scan_list_info *list = NULL, *t_list = NULL;
	u8 count = 0, pending = 0, received = 0;

	err("for ch %d",ch_info->channel_num);
	SLIST_FOREACH_SAFE(dev, &own_dev->_1905_dev_head, next_1905_device, t_dev) {
		temp_radio = topo_srv_get_radio_by_channel(dev, ch_info->channel_num);
		if(temp_radio)
			count++;
	}

	SLIST_FOREACH_SAFE(list, &own_dev->ch_planning_R2.first_scan_list, next_scan_list, t_list) {
		pending ++;
	}
	/*find number of scan results received (count - pending)*/
	received = count - pending;
	err(CH_PLANING_PREX"Received %d, count %d, pending %d", received,count,pending);
	if(received >= (count/2)) {
		return 1;
	} else
		return 0;

}

void ch_planning_handle_ch_scan_rep(struct mapd_global *global)
{
	struct monitor_ch_info *ch_info, *tmp_ch_info, *t_ch_info = NULL, *t_tmp_ch_info = NULL;
	struct own_1905_device *ctx = &global->dev;
	struct os_reltime rem_time = {0};
	Boolean timedout = 0;

	if(global->dev.ch_planning_R2.ch_plan_state != CHPLAN_STATE_SCAN_ONGOING) {
		err(CH_PLANING_PREX"scan is not ongoing!!! return!!! r2_state(%d)",
			global->dev.ch_planning_R2.ch_plan_state);
		return;
	}
	if(SLIST_EMPTY(&global->dev.ch_planning_R2.first_scan_list)) {
		err(CH_PLANING_PREX"All scan report were received success");
		/* all scan report were received success*/
		global->dev.ch_planning_R2.ch_plan_state = CHPLAN_STATE_SCAN_COMPLETE;
		os_get_time(&global->dev.channel_planning_last_scan_timestamp);
		SLIST_FOREACH_SAFE(ch_info,
			&global->dev.ch_planning_R2.first_monitor_ch, next_monitor_ch, t_ch_info) {
			if(ch_info->trigger_status == TRIGGER_TRUE) {
				err(CH_PLANING_PREX"cancel ch(%d) scan timeout!!!", ch_info->channel_num);
				eloop_cancel_timeout(ch_scan_timeout, &global->dev, ch_info);
				eloop_cancel_timeout(ch_scan_timeout_mtk, &global->dev, ch_info);
				break;
			}
		}
	//	dump_ch_planning_info(&global->dev, 0);
		ch_planning_select_best_channel_R2(global, ch_info);
		if (ch_info)
			ch_planning_mark_boot_ch_state(ctx, ch_info);
	} else {
			/* If non mtk agent in the network then check
			ch_scan_timeout_mtk is timedout and we have more than 50% result */
			if (is_mixed_network(ctx, 0)) {
				SLIST_FOREACH_SAFE(tmp_ch_info,
						&global->dev.ch_planning_R2.first_monitor_ch, next_monitor_ch, t_tmp_ch_info) {
					if((tmp_ch_info->trigger_status == TRIGGER_TRUE)) {
						eloop_get_remaining_timeout(&rem_time,ch_scan_timeout_mtk,ctx,tmp_ch_info);
						timedout = (rem_time.sec == 0) ? 1 : 0;
						break;
					}
				}
				debug(CH_PLANING_PREX"rem time: %lu, timedout: %d", rem_time.sec, timedout);
				if (timedout == 1) {
					err(CH_PLANING_PREX"Mixed network and timedout");
					SLIST_FOREACH_SAFE(ch_info,
							&global->dev.ch_planning_R2.first_monitor_ch, next_monitor_ch, t_ch_info) {
						/* trigger is TRUE and we have received half of scan result */
						if((ch_info->trigger_status == TRIGGER_TRUE) &&
								ch_planning_check_scan_result(ctx, ch_info)) {
							err(CH_PLANING_PREX"Received Half Scan Results, Select best channel");
							os_get_time(&global->dev.channel_planning_last_scan_timestamp);
							ch_planning_mark_boot_ch_state(ctx, ch_info);
							global->dev.ch_planning_R2.ch_plan_state = CHPLAN_STATE_SCAN_COMPLETE;
							eloop_cancel_timeout(ch_scan_timeout, &global->dev, ch_info);
							ch_planning_select_best_channel_R2(global, ch_info);
							/*still some agents have not sent scan results , so clean complete scan list entry */
							ch_planning_remove_scan_list(ctx);
							break;
						}
					}
				}
			}
	}
}
void ch_planning_get_channel_list(
	struct own_1905_device *own_dev,
	u8 *channel_arr)
{
	struct score_info *entry, *t_entry = NULL;
	u8 i = 0;
	SLIST_FOREACH_SAFE(entry,&(own_dev->ch_planning_R2.first_ch_score),next_ch_score, t_entry) {
		channel_arr[i] = entry->channel;
		err(CH_PLANING_PREX"channel list %d", channel_arr[i]);
		i++;
	}
}

void ch_planning_send_scan_req_radio(
	struct mapd_global *global,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
	struct channel_scan_req *scan_req = NULL;
	struct prefer_info_db *prefer_db = NULL, *t_prefer_db = NULL;
	u8 buf[3000] = {0};
	u8 i, opidx = 0;
	u16 length = 0;

	scan_req = (struct channel_scan_req *)buf;
	scan_req->fresh_scan = 0x80;
	scan_req->radio_num = 0;
	os_memcpy(scan_req->body[0].radio_id, radio->identifier, ETH_ALEN);
	scan_req->body[0].oper_class_num = 0;

	SLIST_FOREACH_SAFE(prefer_db,
		&radio->chan_preferance.prefer_info_head, prefer_info_entry, t_prefer_db) {
		if (prefer_db->perference == 0)
			continue;
		/*we only support scan on 20MHz opclass */
		if((radio->band > BAND_2G) && (prefer_db->op_class != 118 &&
							prefer_db->op_class != 115 &&
							prefer_db->op_class != 125 &&
							prefer_db->op_class != 121))
			continue;
		if((radio->band == BAND_2G) && (prefer_db->op_class != 81 &&
							prefer_db->op_class != 82 ))
			continue;
		scan_req->body[0].ch_body[opidx].oper_class = prefer_db->op_class;
		scan_req->body[0].ch_body[opidx].ch_list_num = 0;
		for(i = 0; i < prefer_db->ch_num; i++) {
			if (prefer_db->ch_list[i] == 50 ||
				prefer_db->ch_list[i] == 114 ||
				prefer_db->ch_list[i] == 163) {
				debug("160 Mhz Channel, skip %d\n",
					prefer_db->ch_list[i]);
				continue;
			}
			scan_req->body[0].ch_body[opidx].ch_list_num++;
			scan_req->body[0].ch_body[opidx].ch_list[i] = prefer_db->ch_list[i];
			ch_planning_insert_ch_score_table(
				&global->dev,prefer_db->ch_list[i]);
		}
		scan_req->body[0].oper_class_num ++;
		opidx ++;
	}
	scan_req->radio_num++;

	length = sizeof(struct channel_scan_req) + scan_req->radio_num*sizeof(struct scan_body);
	mapd_hexdump(MSG_DEBUG, "MAPD SCAN_REQ", buf, length);

	if ((dev->device_role == DEVICE_ROLE_CONTROLLER) || (dev->device_role == DEVICE_ROLE_CONTRAGENT)) {
		scan_req->neighbour_only = 2;//NB_ALL;
		err(CH_PLANING_PREX"send WAPP_USER_SET_CHANNEL_SCAN_REQ to controller");
		map_get_info_from_wapp(&global->dev, WAPP_USER_SET_CHANNEL_SCAN_REQ, 0, NULL, NULL, (void *)buf, length);
	} else {
		err(CH_PLANING_PREX"Send scan req to dev "MACSTR, MAC2STR(dev->_1905_info.al_mac_addr));
		map_1905_Send_Channel_Scan_Request_Message(
			global->_1905_ctrl,
			(char *)dev->_1905_info.al_mac_addr,
			scan_req->fresh_scan,
			scan_req->radio_num,
			(unsigned char *)scan_req->body);
	}
}

void ch_planning_controller_fill_scan_results(
	struct own_1905_device *ctx,
	struct _1905_map_device *own_1905_device,
	struct net_opt_scan_report_event *scan_rep_evt)
{
	u8 i = 0, j = 0;
	struct radio_info_db *radio = NULL;
	u8 * buf = NULL;

	buf = (u8 *)scan_rep_evt->scan_result;
	debug(CH_PLANING_PREX"scan_result_num %d", scan_rep_evt->scan_result_num);
	for(i = 0; i < scan_rep_evt->scan_result_num; i++) {
		struct net_opt_scan_result_event *rep = (struct net_opt_scan_result_event *)buf;
		radio = topo_srv_get_radio(own_1905_device,rep->radio_id);
		if(!radio) {
			err(CH_PLANING_PREX"radio(%d) not found", rep->channel);
			continue;
		}
		struct scan_result_tlv *new_scan_result = os_zalloc(sizeof(struct scan_result_tlv));
		if (!new_scan_result) {
			err(CH_PLANING_PREX"alloc memory fail for new_scan_result");
			return;
		}
		os_memcpy(new_scan_result->radio_id,rep->radio_id, ETH_ALEN);
		new_scan_result->oper_class = rep->oper_class;
		new_scan_result->channel = rep->channel;
		new_scan_result->scan_status = rep->scan_status;
		debug(CH_PLANING_PREX"scan_res%d opclass(%d) ch(%d) scan_status(%d)",
			i, new_scan_result->oper_class,
			new_scan_result->channel,
			new_scan_result->scan_status);
		if (new_scan_result->scan_status != SCAN_SUCCESS){
			buf +=sizeof(struct net_opt_scan_result_event);
			if (new_scan_result->scan_status == OP_CLASS_CHAN_NOT_SUPP) {
				os_free(new_scan_result);
				err(CH_PLANING_PREX"continue!!!");
				continue;
			} else {
				err(CH_PLANING_PREX"return!!!");
				os_free(new_scan_result);
				ch_planning_scan_restart_due_to_failure(ctx->back_ptr);
				return;
			}
		}
		SLIST_INSERT_HEAD(&(radio->first_scan_result), new_scan_result, next_scan_result);
		SLIST_INIT(&(new_scan_result->first_neighbor_info));
		new_scan_result->timestamp_len = rep->timestamp_len;
		if (new_scan_result->timestamp_len <= TS_MAX)
			os_memcpy(new_scan_result->timestamp, rep->timestamp, new_scan_result->timestamp_len);
		new_scan_result->utilization = rep->utilization;
		new_scan_result->noise = rep->noise;
		if (new_scan_result->noise > 220)
			new_scan_result->noise = 0;
		new_scan_result->neighbor_num = rep->neighbor_num;
		debug(CH_PLANING_PREX"timestamp_len(%d) utilization(%d) noise(%d) neighbor_num(%d)",
			new_scan_result->timestamp_len, new_scan_result->utilization,
			new_scan_result->noise,
			new_scan_result->neighbor_num);
		if (new_scan_result->neighbor_num > MAX_LEN_OF_BSS_TABLE) {
			err(CH_PLANING_PREX"maximum scan result neighbour num(%u) exceeded than 256", new_scan_result->neighbor_num);
			continue;
		}

		for (j = 0; j < new_scan_result->neighbor_num; j++) {
			struct nb_info *nb_info = os_zalloc(sizeof(struct nb_info));
			struct neighbor_info *src_nb = &rep->nb_info[j];
			if(!nb_info) {
				err(CH_PLANING_PREX"alloc memory fail for nb_info");
				//os_free(new_scan_result);
				return;
			}
			SLIST_INSERT_HEAD(&(new_scan_result->first_neighbor_info), 
				nb_info, next_neighbor_info);
			os_memcpy(nb_info->bssid, src_nb->bssid, ETH_ALEN);
			nb_info->ssid_len = src_nb->ssid_len;
			if (nb_info->ssid_len <= MAX_SSID_LEN)
				os_memcpy(nb_info->ssid, src_nb->ssid, nb_info->ssid_len);
			nb_info->RCPI = src_nb->RCPI;
			nb_info->ch_bw_len = src_nb->ch_bw_len;
			if (nb_info->ch_bw_len <= MAX_CH_BW_LEN)
				os_memcpy(nb_info->ch_bw, src_nb->ch_bw, nb_info->ch_bw_len);
			nb_info->cu_stacnt_present = src_nb->cu_stacnt_present;
			if (nb_info->cu_stacnt_present & 0x80)
				nb_info->cu = src_nb->cu;
			if (nb_info->cu_stacnt_present & 0x40) {
				nb_info->sta_cnt = src_nb->sta_cnt;
			}

			debug(CH_PLANING_PREX"nb%d bssid("MACSTR") ssid_len(%d) ssid(%s) RCPI(%d)"
				"ch_bw_len(%d) ch_bw(%s) cu_stacnt_present(%d) cu(%d) sta_cnt(%d)",
				j, MAC2STR(nb_info->bssid), nb_info->ssid_len, nb_info->ssid,
				nb_info->RCPI, nb_info->ch_bw_len, nb_info->ch_bw, nb_info->cu_stacnt_present,
				nb_info->cu, nb_info->sta_cnt);
		}
		new_scan_result->cu_distribution.ch_num = rep->channel;
		new_scan_result->cu_distribution.edcca_airtime = rep->edcca;
		buf +=sizeof(struct net_opt_scan_result_event)+ (rep->neighbor_num*sizeof(struct neighbor_info));
		topo_srv_update_ch_plan_scan_done(ctx,own_1905_device,radio);
	}
}

void ch_planning_handle_controller_scan_result(
	struct own_1905_device *ctx,
	struct net_opt_scan_report_event *scan_rep_evt)
{
	struct _1905_map_device *own_1905_mapdevice = topo_srv_get_next_1905_device(ctx, NULL);

	ch_planning_remove_radio_scan_results(ctx, own_1905_mapdevice);
	ch_planning_controller_fill_scan_results(ctx, own_1905_mapdevice, scan_rep_evt);
	ch_planning_handle_ch_scan_rep(ctx->back_ptr);
}
void ch_planning_send_scan_req(
	struct own_1905_device *ctx,
	struct _1905_map_device *_1905_device,
	struct radio_info_db *radio)
{
	radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_SCAN_ONGOING;
	ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_SCAN_ONGOING;
	if(_1905_device->device_role == DEVICE_ROLE_CONTROLLER) {
		debug(CH_PLANING_PREX"cancel monitor timer for radio with channel %d", radio->channel[0]);
		/*cancel timer for own dev metric reporting from wapp */
		eloop_cancel_timeout(ch_planning_own_dev_get_metric_timeout, ctx, radio);
	} else {
		debug(CH_PLANING_PREX"restore policy as now going to trigger scan");
		/*Update policy to restore original setting after monitor timeout when scan is triggered*/
		steer_msg_update_policy_config(ctx->back_ptr, _1905_device);
	}

	/*Add check here to send scan req only to a MAP R2 dev that supports fresh scan
	otherwise update state of dev as scan done*/
	if((_1905_device->map_version != DEV_TYPE_R2
#ifdef MAP_R3
		&& _1905_device->map_version != DEV_TYPE_R3
#endif
		) ||
		(
#ifdef MAP_R3
		(
#endif
		_1905_device->map_version == DEV_TYPE_R2
#ifdef MAP_R3
		|| _1905_device->map_version == DEV_TYPE_R3
		)
#endif
		&& radio->radio_scan_params.boot_scan_only == 1) ||
		(!_1905_device->in_network)) {
		err(CH_PLANING_PREX"Scan is not allowed for "MACSTR"!!! map_version(%d), bootOnly(%d) in_network(%d)",
			MAC2STR(_1905_device->_1905_info.al_mac_addr), _1905_device->map_version, radio->radio_scan_params.boot_scan_only,
			_1905_device->in_network);
		radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_SCAN_COMPLETE;
		return;
	}
	ch_planning_insert_scanlist_entry(ctx, _1905_device, radio);
	ch_planning_send_scan_req_radio(ctx->back_ptr, _1905_device, radio);

}

void ch_planning_scan_trigger(
	struct own_1905_device *ctx,
	struct monitor_ch_info *ch_info)
{
	struct radio_info_db *radio = NULL, *c_radio = NULL;
	u32 scan_wait_timeout = 0;
	struct _1905_map_device *_1905_device = NULL, *t_1905_device = NULL, *cont_dev = NULL;

	err(CH_PLANING_PREX"scan trigger on ch(%d)", ch_info->channel_num);
	SLIST_FOREACH_SAFE( _1905_device,&ctx->_1905_dev_head,next_1905_device, t_1905_device) {
		radio = topo_srv_get_radio_by_band(_1905_device, ch_info->channel_num);
		if(!radio){
			err(CH_PLANING_PREX"radio not found!!! skip!!!");
			continue;
		}
		if(radio->dev_ch_plan_info.dev_ch_plan_state >=
			CHPLAN_STATE_SCAN_ONGOING) {
			err(CH_PLANING_PREX"radio state(%d) mismatch! r2 state(%d) defer scan!!!",
				radio->dev_ch_plan_info.dev_ch_plan_state, ctx->ch_planning_R2.ch_plan_state);
			return;
		}
		scan_wait_timeout = get_scan_wait_time(_1905_device, radio);
		if(scan_wait_timeout) {
			eloop_register_timeout(scan_wait_timeout,
				0,
				ch_planning_scan_wait_timeout,
				ctx,
				_1905_device);
			err(CH_PLANING_PREX"scan_wait_timeout %d", scan_wait_timeout);
			continue;
		} else {
			if (_1905_device->device_role == DEVICE_ROLE_CONTROLLER) {
				cont_dev = _1905_device;
				c_radio = radio;
				continue;
			}
			ch_planning_send_scan_req(ctx, _1905_device, radio);
		}
	}
	if (cont_dev && c_radio)
		ch_planning_send_scan_req(ctx, cont_dev, c_radio);

	/* Check for 3rd party agent, if yes then scan timeout will be 5min */
		if (!is_mixed_network(ctx, 0)) {
			debug(CH_PLANING_PREX"all mtk devices");
			eloop_register_timeout(10, 0, ch_scan_timeout, ctx, ch_info);
		} else {
			err(CH_PLANING_PREX"mixed devices");
			eloop_register_timeout(10, 0, ch_scan_timeout_mtk, ctx, ch_info);
			eloop_register_timeout(300, 0, ch_scan_timeout, ctx, ch_info);
		}
}

void ch_planning_remove_monitor_ch(
	struct own_1905_device *own_dev,
	struct monitor_ch_info *ch_info)
{
	struct monitor_ch_info *temp_ch_info, *t_tmp_ch_info = NULL;
	struct affected_agent_info *agent = NULL;
	SLIST_FOREACH_SAFE(temp_ch_info,
		&own_dev->ch_planning_R2.first_monitor_ch, next_monitor_ch, t_tmp_ch_info) {
		if(ch_info->channel_num == temp_ch_info->channel_num) {
			/*remove all the list of affected agents that we had saved earlier */
			while (!SLIST_EMPTY(&(temp_ch_info->first_affected_agent))) {
				agent = SLIST_FIRST(&(temp_ch_info->first_affected_agent));
				SLIST_REMOVE_HEAD(&(temp_ch_info->first_affected_agent),
					next_affected_agent);
				os_free(agent);
			}
			err(CH_PLANING_PREX"remove ch(%d) from r2 first_monitor_ch", ch_info->channel_num);
			SLIST_REMOVE(&(own_dev->ch_planning_R2.first_monitor_ch),
				temp_ch_info,
				monitor_ch_info,
				next_monitor_ch);
			os_free(temp_ch_info);
			return;
		}
	}
}

void channel_monitor_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct monitor_ch_info *ch_info = (struct monitor_ch_info *)timeout_ctx;
	struct own_1905_device *ctx = (struct own_1905_device *)eloop_ctx;

	err(CH_PLANING_PREX"Monitor timeout on channel(%d)", ch_info->channel_num);

	ctx->ch_planning_R2.skip_edcca_check = is_mixed_network(ctx, 1);

	if(ctx->ch_planning_R2.force_trigger == 1) {
		err(CH_PLANING_PREX"force trigger");
		ch_info->trigger_status = TRIGGER_TRUE;
	} else {
		ch_info->trigger_status = ch_planning_check_trigger(ctx, ch_info);
	}

	os_get_time(&ctx->ch_planning_R2.ch_monitor_start_ts);
	if(ch_info->trigger_status == TRIGGER_TRUE) {
		err(CH_PLANING_PREX"scan trigger!!!");
		ch_planning_scan_trigger(ctx, ch_info);
	} else if(ch_info->trigger_status == TRIGGER_FALSE) {
		err(CH_PLANING_PREX"cancel scan!!! remove monitor ch");
		ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_IDLE;
		ch_planning_update_all_dev_state((u8)CHPLAN_STATE_IDLE,ch_info->channel_num,ctx);
		struct _1905_map_device *_1905_device = NULL;
		struct radio_info_db *reset_radio = NULL;
		_1905_device = topo_srv_get_1905_device(ctx, NULL);
		reset_radio = topo_srv_get_radio_by_band(_1905_device, ch_info->channel_num);
		ch_planning_restore_policy(ctx, reset_radio);
		ch_planning_remove_monitor_ch(ctx, ch_info);
	} else if (ch_info->trigger_status == TRIGGER_PENDING) {
		err(CH_PLANING_PREX"scan pending!!! insert into task list");
		insert_into_task_list(ctx, TASK_CHANNEL_PLANNING_TRIGGER,
			ch_info, NULL, NULL);
	}
}
u8 ch_planning_search_monitor_ch(
	struct own_1905_device * ctx,
	struct radio_info_db *radio)
{
	struct monitor_ch_info *ch = NULL, *t_ch = NULL;
	if(!radio) {
		err(CH_PLANING_PREX"radio is NULL");
		return 0;
	}
	SLIST_FOREACH_SAFE(ch,
		&(ctx->ch_planning_R2.first_monitor_ch), next_monitor_ch, t_ch) {
		if (radio->channel[0] == ch->channel_num) {
			return 1;
		}
	}
	debug(CH_PLANING_PREX"this channel is not in monitor list %d", radio->channel[0]);
	return 0;
}
void ch_planning_own_dev_get_metric_timeout(
	void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *ctx = (struct own_1905_device *)eloop_ctx;
	struct radio_info_db *radio = (struct radio_info_db *)timeout_ctx;
	u8 metric_policy_interval =
		ctx->ch_planning_R2.ch_plan_metric_policy_interval;
	own_dev_get_metric_info(ctx->back_ptr, radio);
	eloop_register_timeout(metric_policy_interval,
		0, ch_planning_own_dev_get_metric_timeout, ctx, radio);
}

void ch_planning_update_ap_metric_policy(
	struct own_1905_device *ctx,
	struct monitor_ch_info *monitor_ch,
	u8 metric_policy_interval)
{
	/*change policy for AP metric reporting for all devices on that channel*/
	struct _1905_map_device *tmp_dev = NULL, *t_tmp_dev = NULL;
	struct radio_info_db *tmp_radio = NULL;
	struct lib_steer_radio_policy *radio_policy = NULL;
	struct lib_metrics_radio_policy *metrics_policy = NULL;
	struct mapd_global *global = ctx->back_ptr;
	if(metric_policy_interval == 0) {
		metric_policy_interval =
			ctx->ch_planning_R2.ch_plan_metric_policy_interval;
	}
	struct lib_unsuccess_assoc_policy *assoc_policy = os_zalloc(sizeof(struct lib_unsuccess_assoc_policy));

	radio_policy = os_zalloc(sizeof(struct lib_steer_radio_policy));
	metrics_policy = os_zalloc(sizeof(struct lib_metrics_radio_policy));
	if(metrics_policy == NULL || radio_policy == NULL) {
		if(metrics_policy)
			os_free(metrics_policy);
		if(radio_policy)
			os_free(radio_policy);
		err("memory alloc fail, can't update policy");
		if(assoc_policy)
			os_free(assoc_policy);
		return;
	}
	if(assoc_policy == NULL) {
		os_free(metrics_policy);
		os_free(radio_policy);
		mapd_ASSERT(0);
		return;
	}
	SLIST_FOREACH_SAFE(tmp_dev, &ctx->_1905_dev_head, next_1905_device, t_tmp_dev) {
		tmp_radio = topo_srv_get_radio_by_channel(tmp_dev, monitor_ch->channel_num);
		if(!tmp_radio) {
			err("failed to find radio on this temp dev ");
			continue;
		}
		if(tmp_dev->device_role == DEVICE_ROLE_CONTROLLER) {
			err("start metric rep timer for owndev tmp_radio ch  %d", tmp_radio->channel[0]);
			/*start timer for own_dev's info fetching from WAPP*/
			eloop_register_timeout(metric_policy_interval,
				0, ch_planning_own_dev_get_metric_timeout, ctx, tmp_radio);
			continue;
		}
		if(get_default_radio_policy(ctx, tmp_radio, radio_policy) != 0) {
			err("get radio_policy fail, can't update policy");
			if(metrics_policy)
				os_free(metrics_policy);
			if(assoc_policy)
				os_free(assoc_policy);
			return;
		}
		os_memcpy(metrics_policy->identifier, tmp_radio->identifier, ETH_ALEN);
		debug("send command to agent");
		assoc_policy->report_switch = 1;
		assoc_policy->report_rate = 10;
		/*no need to fill other metric policy contents, as reporting will be done as per interval*/
		if (tmp_dev->map_version == DEV_TYPE_R2
#ifdef MAP_R3
			|| tmp_dev->map_version == DEV_TYPE_R3
#endif
			) {
			map_store_policy_for_agent(
				&global->dev,
				(char *)tmp_dev->_1905_info.al_mac_addr,
				0,
				NULL,
				0,
				NULL,
				1,
				radio_policy,
				metric_policy_interval,
				1,
				metrics_policy,
				1,
				0,
				1,
				assoc_policy);
			map_1905_Send_MAP_Policy_Request_Message (
				global->_1905_ctrl,
				(char *)tmp_dev->_1905_info.al_mac_addr,
				0,
				NULL,
				0,
				NULL,
				1,
				radio_policy,
				metric_policy_interval,
				1,
				metrics_policy,
				1,
				0,
				1,
				assoc_policy);
		} else {
			map_store_policy_for_agent(
				&global->dev,
				(char *)tmp_dev->_1905_info.al_mac_addr,
				0,
				NULL,
				0,
				NULL,
				1,
				radio_policy,
				metric_policy_interval,
				1,
				metrics_policy,
				0,
				0,
				0,
				NULL);
			map_1905_Send_MAP_Policy_Request_Message (
				global->_1905_ctrl,
				(char *)tmp_dev->_1905_info.al_mac_addr,
				0,
				NULL,
				0,
				NULL,
				1,
				radio_policy,
				metric_policy_interval,
				1,
				metrics_policy,
				0,
				0,
				0,
				NULL);
		}
	}
	if(metrics_policy) {
		os_free(metrics_policy);
	}
	if(radio_policy){
		os_free(radio_policy);
	}
	if (assoc_policy)
		os_free(assoc_policy);
}

void ch_planning_add_monitor_ch(
	struct own_1905_device * ctx,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
/* check list if channel is already present ,
if not then add new list item and start timer for this channel's monitor
check list if affected agents has dev alid match , if not then add */
	struct monitor_ch_info *ch = NULL, *t_ch = NULL;
	struct monitor_ch_info *new_ch_info = NULL;
	struct affected_agent_info *affected_agent = NULL, *t_aff_agent = NULL;
	u8 new_ch = 1, new_agent = 1;
	if(!radio) {
		err(CH_PLANING_PREX"radio is NULL");
		return ;
	}
	if(ch_planning_search_monitor_ch(ctx,radio)) {
		new_ch = 0;
	}
	if(new_ch) {
	
		new_ch_info = os_zalloc(sizeof(struct monitor_ch_info));
		affected_agent = NULL;
		if (new_ch_info == NULL) {
			err(CH_PLANING_PREX"alloc memory fail");
			assert(0);
			return ;
		}
		new_ch_info->channel_num = radio->channel[0];
		new_ch_info->band = radio->band;
		SLIST_INSERT_HEAD(&(ctx->ch_planning_R2.first_monitor_ch),
			new_ch_info, next_monitor_ch);
		SLIST_INIT(&new_ch_info->first_affected_agent);
		err(CH_PLANING_PREX"Start Monitoring Channel %d", radio->channel[0]);

		affected_agent = os_zalloc(sizeof(struct affected_agent_info));
		if (affected_agent == NULL) {
			err(CH_PLANING_PREX"alloc memory fail");
			os_free(new_ch_info);
			assert(0);
			return ;
		}
		affected_agent->affected_dev = dev;
		SLIST_INSERT_HEAD(&new_ch_info->first_affected_agent,
			affected_agent, next_affected_agent);

		/*start timer for  this channel's monitor*/
		debug(CH_PLANING_PREX"start monitor timeout for channel %d ", radio->channel[0]);
		eloop_register_timeout(ctx->ch_planning_R2.ch_monitor_timeout, 0,
			channel_monitor_timeout, ctx, new_ch_info);

		ch_planning_update_all_dev_state((u8)CHPLAN_STATE_MONITOR,new_ch_info->channel_num,ctx);
		/*change policy for AP metric reporting for all devices on that channel*/
		ch_planning_update_ap_metric_policy(ctx, new_ch_info, 0);
		debug(CH_PLANING_PREX"update policy done");
	} else {
		affected_agent = NULL;
		new_agent = 1;
		SLIST_FOREACH_SAFE(ch, &(ctx->ch_planning_R2.first_monitor_ch), next_monitor_ch, t_ch) {
			if (ch->channel_num == radio->channel[0])
				break;
		}
		if (!ch) {
			err(CH_PLANING_PREX"monitor ch is null");
			return;
		}
		SLIST_FOREACH_SAFE(affected_agent,
			&(ch->first_affected_agent), next_affected_agent, t_aff_agent) {
			if (os_memcmp(affected_agent->affected_dev->_1905_info.al_mac_addr,
				dev->_1905_info.al_mac_addr, ETH_ALEN) == 0) {
				new_agent = 0;
				break;
			}
		}
		if(new_agent) {
			affected_agent = os_zalloc(sizeof(struct affected_agent_info));
			if (!affected_agent) {
				err(CH_PLANING_PREX"failed to allocate memory for affected_agent");
				assert(0);
				return;
			}
			affected_agent->affected_dev = dev;
			SLIST_INSERT_HEAD(&ch->first_affected_agent,
				affected_agent, next_affected_agent);
			debug(CH_PLANING_PREX"add new agent");
		}
	}
}
void ch_planning_monitor_threshold_init(struct own_1905_device *ctx)
{
	struct Ch_threshold *thresh = NULL;

	thresh = &ctx->ch_planning_R2.ch_plan_thres[0];
	thresh->band = BAND_2G;
	if(thresh->ch_util_threshold == 0)
		thresh->ch_util_threshold =
			CH_PLAN_DEFAULT_CH_UTIL_TH_2G;
	if(thresh->edcca_threshold == 0)
		thresh->edcca_threshold =
			CH_PLAN_DEFAULT_EDCCA_TH_2G;
	if(thresh->obss_load_threshold == 0)
		thresh->obss_load_threshold =
			CH_PLAN_DEFAULT_OBSS_TH_2G;

	thresh = &ctx->ch_planning_R2.ch_plan_thres[1];
		thresh->band = BAND_5GL;
		if(thresh->ch_util_threshold == 0)
			thresh->ch_util_threshold =
				CH_PLAN_DEFAULT_CH_UTIL_TH_5G;
		if(thresh->edcca_threshold == 0)
			thresh->edcca_threshold =
				CH_PLAN_DEFAULT_EDCCA_TH_5G;
		if(thresh->obss_load_threshold == 0)
			thresh->obss_load_threshold =
				CH_PLAN_DEFAULT_OBSS_TH_5G;

	thresh = &ctx->ch_planning_R2.ch_plan_thres[2];
	thresh->band = BAND_5GH;
	if(thresh->ch_util_threshold == 0)
		thresh->ch_util_threshold =
			CH_PLAN_DEFAULT_CH_UTIL_TH_5G;
	if(thresh->edcca_threshold == 0)
		thresh->edcca_threshold =
			CH_PLAN_DEFAULT_EDCCA_TH_5G;
	if(thresh->obss_load_threshold == 0)
		thresh->obss_load_threshold =
			CH_PLAN_DEFAULT_OBSS_TH_5G;
}

void ch_planning_R2_init(struct own_1905_device * ctx)
{

	ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_IDLE;
	if(ctx->ch_planning_R2.min_score_inc == 0)
		ctx->ch_planning_R2.min_score_inc = MIN_SCORE_INCREMENT_DEFAULT;

	if(ctx->ch_planning_R2.ch_plan_metric_policy_interval == 0)
		ctx->ch_planning_R2.ch_plan_metric_policy_interval =
			DEFAULT_METRIC_REPORTING_INTERVAL;

	if(ctx->ch_planning_R2.ch_monitor_timeout == 0)
		ctx->ch_planning_R2.ch_monitor_timeout =
			CHANNEL_MONITOR_TIMEOUT;
	if(ctx->ch_planning_R2.ch_monitor_prohibit_wait_time == 0)
		ctx->ch_planning_R2.ch_monitor_prohibit_wait_time =
			CHANNEL_MONITOR_PROHIBIT_TIME;

	ch_planning_monitor_threshold_init(ctx);
	if (SLIST_EMPTY(&(ctx->ch_planning_R2.first_scan_list))) {
		SLIST_INIT(&ctx->ch_planning_R2.first_scan_list);
	}
	if (SLIST_EMPTY(&(ctx->ch_planning_R2.first_ch_score))) {
		SLIST_INIT(&ctx->ch_planning_R2.first_ch_score);
	}
	if (SLIST_EMPTY(&(ctx->ch_planning_R2.first_monitor_ch))) {
		SLIST_INIT(&ctx->ch_planning_R2.first_monitor_ch);
	}
}
void ch_planning_remove_radio_scan_result(
	struct radio_info_db *radio)
{
	struct scan_result_tlv *scan_result = NULL;
	struct nb_info *neigh = NULL;
	while (!SLIST_EMPTY(&(radio->first_scan_result))) {
		scan_result = SLIST_FIRST(&(radio->first_scan_result));
		while (!SLIST_EMPTY(&(scan_result->first_neighbor_info))) {
			neigh = SLIST_FIRST(&(scan_result->first_neighbor_info));
			SLIST_REMOVE_HEAD(&(scan_result->first_neighbor_info),
				next_neighbor_info);
			os_free(neigh);
		}
		SLIST_REMOVE_HEAD(&(radio->first_scan_result), next_scan_result);
		os_free(scan_result);
	}
}
void ch_planning_remove_all_dev_radio_scan_results(
	struct own_1905_device * ctx,
	struct radio_info_db *reset_radio)
{
	struct _1905_map_device *_1905_device = NULL, *t_1905_dev = NULL;
	struct radio_info_db *radio = NULL, *t_radio = NULL;
	SLIST_FOREACH_SAFE(_1905_device, &(ctx->_1905_dev_head), next_1905_device, t_1905_dev) {
		SLIST_FOREACH_SAFE(radio, &(_1905_device->first_radio), next_radio, t_radio) {
			if((reset_radio && (radio->band == reset_radio->band)) ||
				(reset_radio == NULL)) {
				radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_IDLE;
				eloop_cancel_timeout(ch_planning_own_dev_get_metric_timeout, ctx, radio);
				os_memset(&radio->dev_ch_plan_info.dev_ch_monitor_info,
					0, sizeof(struct dev_ch_monitor));
			}
		}
	}
}
u8 ch_planning_num_monitor_ch(
	struct own_1905_device *own_dev)
{
	struct monitor_ch_info *temp_ch_info, *t_tmp_ch_info = NULL;
	u8 num_monitor_ch = 0;
	SLIST_FOREACH_SAFE(temp_ch_info,
		&own_dev->ch_planning_R2.first_monitor_ch, next_monitor_ch, t_tmp_ch_info) {
		num_monitor_ch++;
	}
	return num_monitor_ch;
}

void ch_planning_restore_policy(
	struct own_1905_device * ctx,
	struct radio_info_db *reset_radio)
{
	struct radio_info_db *radio = reset_radio, *temp_radio = NULL, *t_tmp_radio = NULL;
	struct _1905_map_device *_1905_device = NULL, *t_1905_device = NULL;
	struct mapd_global *global = (struct mapd_global *)ctx->back_ptr;
	u8 num_monitor_ch = ch_planning_num_monitor_ch(ctx);
	SLIST_FOREACH_SAFE(_1905_device, &(ctx->_1905_dev_head), next_1905_device, t_1905_device) {
		if(_1905_device->device_role == DEVICE_ROLE_CONTROLLER) {
			if(radio) {
				/*cancel timer for own dev metric reporting from wapp */
				eloop_cancel_timeout(ch_planning_own_dev_get_metric_timeout, ctx, radio);
			} else {
				SLIST_FOREACH_SAFE(temp_radio,&(_1905_device->first_radio),next_radio, t_tmp_radio){
					/*cancel timer for own dev metric reporting from wapp */
					eloop_cancel_timeout(ch_planning_own_dev_get_metric_timeout, ctx, temp_radio);
				}
			}
		} else if((num_monitor_ch < 2) && !global->params.Certification){
			debug(CH_PLANING_PREX"restore policy monitor ch(%u)", num_monitor_ch);
			/*Update policy to restore original setting after monitor timeout when scan is triggered*/
			steer_msg_update_policy_config(ctx->back_ptr, _1905_device);
		}
	}
}
#ifndef MAP_6E_SUPPORT
/* ToDo: Need to extend for 6E and remove this MACRO */
void ch_planning_remove_scan_pending(struct own_1905_device *ctx)
{
	struct _1905_map_device *_1905_dev = topo_srv_get_1905_device(ctx, NULL);

	struct radio_info_db *radio = NULL;

	struct mapd_radio_info *radio_info = NULL;

	if (_1905_dev == NULL)
		return;

	radio = topo_srv_get_next_radio(_1905_dev, radio);

	while (radio) {
		if ((ctx->scan_done_2g == 1) && (radio->band == BAND_2G)) {
			radio_info = get_radio_info_by_radio_id(ctx->back_ptr, radio->identifier);
			if (radio_info) {
				ctx->scan_done_2g = 0;
				if (radio_info->bootup_run == BOOTUP_SCAN_ONGOING)
					radio_info->bootup_run = BOOTUP_SCAN_COMPLETED;
			}
		} else if ((ctx->scan_done_5g == 1) &&
			((radio->band == BAND_5GL) || (radio->band == BAND_5GH))) {
			radio_info = get_radio_info_by_radio_id(ctx->back_ptr, radio->identifier);
			if (radio_info) {
				ctx->scan_done_5g = 0;
				if (radio_info->bootup_run == BOOTUP_SCAN_ONGOING)
					radio_info->bootup_run = BOOTUP_SCAN_COMPLETED;
			}
		}
		radio = topo_srv_get_next_radio(_1905_dev, radio);
	}
}
#endif
void ch_planning_R2_reset(
	struct own_1905_device * ctx,
	struct radio_info_db *reset_radio)
{
	struct _1905_map_device *tmp_dev = NULL, *t_tmp_dev = NULL;

	/*if reset radio is NULL , then reset for all radios*/
	if(!reset_radio){
		err(CH_PLANING_PREX"reset on all radio");
	} else {
		err(CH_PLANING_PREX"reset on radio(%d)", reset_radio->channel[0]);
	}

	ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_IDLE;
	//ch_planning_remove_agg_score_table(ctx);// make persistent
	ch_planning_restore_policy(ctx, reset_radio);
	ch_planning_remove_scan_list(ctx);
	ch_planning_remove_ch_monitor_info_list(ctx, reset_radio);
	ch_planning_remove_all_dev_radio_scan_results(ctx,reset_radio);
#ifndef MAP_6E_SUPPORT
	/* ToDo: Need to extend for 6E and remove this MACRO */
	ch_planning_remove_scan_pending(ctx);
#endif
	if(!reset_radio){
		find_and_remove_pending_task(ctx, TASK_CHANNEL_PLANNING_TRIGGER);
	} else {
		/*remove pending channel planning task on the radio*/
		find_and_remove_pending_task_by_radio(ctx, TASK_CHANNEL_PLANNING_TRIGGER, reset_radio);
	}
	if(ctx->ch_planning_R2.CAC_on_channel)
		eloop_cancel_timeout(channel_cac_timeout2,
			ctx, NULL);

	if (eloop_is_timeout_registered(controller_ch_set, ctx, NULL)) {
		eloop_cancel_timeout(controller_ch_set, ctx, NULL);
		if (ctx->ch_planning_R2.ch_prefer_for_ch_select) {
			os_free(ctx->ch_planning_R2.ch_prefer_for_ch_select);
			ctx->ch_planning_R2.ch_prefer_for_ch_select = NULL;
		}
	}

	if (eloop_is_timeout_registered(trigger_chplan_post_cac_resp, (void *)ctx , NULL))
		eloop_cancel_timeout(trigger_chplan_post_cac_resp, ctx, NULL);

	SLIST_FOREACH_SAFE(tmp_dev, &ctx->_1905_dev_head, next_1905_device, t_tmp_dev) {
		tmp_dev->ch_sel_req_given = 0;
	}
	ctx->trig_chan_post_cac_chplan = 0;
	ctx->ch_planning_R2.CAC_on_channel = 0;
	ctx->ch_planning_R2.force_trigger = 0;
	ctx->ch_planning.current_ch_planning_dev = NULL;
	ch_planning_R2_init(ctx);
	ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE; 
}

void ch_planning_update_data(
	struct bss_info_db *bss,
	struct radio_info_db *radio,
	u8 cu_tlv_update)
{
	struct dev_ch_monitor *dev_monitor_info = NULL;

	if(!radio) {
		err(CH_PLANING_PREX"radio is NULL");
		return;
	}

	dev_monitor_info = &radio->dev_ch_plan_info.dev_ch_monitor_info;
	debug(CH_PLANING_PREX"radio channel %d", radio->channel[0]);
	if(bss) {
		dev_monitor_info->avg_cu_monitor =
			(dev_monitor_info->avg_cu_monitor +
			bss->ch_util);
		dev_monitor_info->count_cu_util++;
		debug(CH_PLANING_PREX"add ch %d bss->ch_util %d, count %d",radio->channel[0], bss->ch_util, dev_monitor_info->count_cu_util);
	} else {
		if(cu_tlv_update == 1) {
			dev_monitor_info->avg_edcca =
				(dev_monitor_info->avg_edcca +
				radio->cu_distribution.edcca_airtime);
			dev_monitor_info->count_edcca_cu_tlv++;
			debug(CH_PLANING_PREX"add edcca %d, count %d", radio->cu_distribution.edcca_airtime, dev_monitor_info->count_edcca_cu_tlv);
		} else {
			dev_monitor_info->avg_obss_load =
				(dev_monitor_info->avg_obss_load +
				radio->radio_metrics.cu_other);

			dev_monitor_info->avg_myTxAirtime =
				(dev_monitor_info->avg_myTxAirtime +
				radio->radio_metrics.cu_tx);

			dev_monitor_info->avg_myRxAirtime =
				(dev_monitor_info->avg_myRxAirtime +
				radio->radio_metrics.cu_rx);
			dev_monitor_info->count_radio_metric++;
			debug(CH_PLANING_PREX"add cu_other %d, count %d", radio->radio_metrics.cu_other, dev_monitor_info->count_radio_metric);
		}
	}
	//dump_ch_planning_update_data(radio);
}
u8 ch_planning_compare_monitor_thresh(
	struct own_1905_device * ctx,
	struct bss_info_db *bss,
	struct radio_info_db *radio,
	u8 cu_tlv_update)
{
	u8 need_to_monitor = 0;
	u8 radio_num = 0;
	u32  util_per = 0;
	if(bss) {
		/*check based on channel util threshold*/
		radio_num = find_radio_num(ctx,bss->radio);
		debug(CH_PLANING_PREX"ch_util %d",bss->ch_util);
		util_per = (u32)(bss->ch_util) * 100;/*calculate percentage*/
		util_per = util_per / 255;
		if(util_per >
			ctx->ch_planning_R2.ch_plan_thres[radio_num].ch_util_threshold)
		{
			debug(CH_PLANING_PREX"CH monitoring on ch %d, bss->ch_util %d",bss->radio->channel[0], bss->ch_util);
			need_to_monitor = 1;
		}
		radio = bss->radio;
		debug(CH_PLANING_PREX"BSSID "MACSTR"bss radio ch %d",MAC2STR(bss->bssid), radio->channel[0]);
	} else if(radio && (cu_tlv_update == 1)) {
			/*check based on EDCCA thresh*/
			radio_num = find_radio_num(ctx, radio);
			debug(CH_PLANING_PREX"edcca_airtime %d radio ch %d",  radio->cu_distribution.edcca_airtime, radio->channel[0]);
			if(radio->cu_distribution.edcca_airtime >
				ctx->ch_planning_R2.ch_plan_thres[radio_num].edcca_threshold)
			{
				debug(CH_PLANING_PREX"moni trigger due to edcca bad %d", radio->cu_distribution.edcca_airtime);
				need_to_monitor = 1;
			}
	} else if(radio && (cu_tlv_update == 0)) {
		/*check based on OBSS thresh*/
		radio_num = find_radio_num(ctx, radio);
		debug(CH_PLANING_PREX"OBSS %d radio ch %d", radio->radio_metrics.cu_other, radio->channel[0]);
		if(radio->radio_metrics.cu_other >
			ctx->ch_planning_R2.ch_plan_thres[radio_num].obss_load_threshold)
		{
			debug(CH_PLANING_PREX"moni trigger due to obss bad %d", radio->radio_metrics.cu_other);
			need_to_monitor = 1;
		}
	}
	return need_to_monitor;
}

u8 ch_planning_is_MAP_net_idle(
	struct own_1905_device * ctx)
{
	u8 is_idle = 0;
	struct os_time now;
	struct ch_planning_cb *p_ch_planning = &ctx->ch_planning;

	if (p_ch_planning->last_high_byte_count_ts.sec == 0) {
		os_get_time(&p_ch_planning->last_high_byte_count_ts);
	}
	os_get_time(&now);
	if (now.sec - p_ch_planning->last_high_byte_count_ts.sec >
		p_ch_planning->ChPlanningIdleTime) {
		is_idle = 1;
	} else {
		is_idle = 0;
	}
	debug(CH_PLANING_PREX"is_idle %d", is_idle);
	return is_idle;
}

u8 ch_planning_is_monitor_prohibit_over(
	struct own_1905_device * ctx)
{
	struct os_time now;
	os_get_time(&now);
	if(ctx->ch_planning_R2.ch_monitor_start_ts.sec == 0) {
		err(CH_PLANING_PREX" initialize monitor timer");
		os_get_time(&ctx->ch_planning_R2.ch_monitor_start_ts);
	}
	if((now.sec > ctx->ch_planning_R2.ch_monitor_start_ts.sec +
			ctx->ch_planning_R2.ch_monitor_prohibit_wait_time)) {
		err(CH_PLANING_PREX"start monitor time update ");
		os_get_time(&ctx->ch_planning_R2.ch_monitor_start_ts);
		return 1;
	}
	return 0;
}
u8 ch_planning_need_monitor(
	struct own_1905_device * ctx,
	struct bss_info_db *bss,
	struct radio_info_db *radio,
	u8 cu_tlv_update)
{
/*	Need to monitor will be true only when 
	the monitor thresholds exceed 
	AND 
	Map network is in IDLE state
	AND
	MONITOR prohibit time has elapsed*/
	u8 need_to_monitor_1 = 0, need_to_monitor_2 = 0, need_to_monitor_3 = 0, need_to_monitor_4 = 0;

	/*Check if initial ch planning is completed or not*/
	need_to_monitor_4 = is_chan_plan_done_for_all_dev(ctx->back_ptr);
	if(!need_to_monitor_4)
			return 0;
	/*Compare with Thresholds*/
	need_to_monitor_1 = ch_planning_compare_monitor_thresh(ctx,bss,radio,cu_tlv_update);
	if(!need_to_monitor_1)
		return 0;
	/*Check Network is in IDLE state*/
	need_to_monitor_2 = ch_planning_is_MAP_net_idle(ctx);
	if(!need_to_monitor_2)
		return 0;
	/*Check Monitoring prohibit timer has expired*/
	need_to_monitor_3 = ch_planning_is_monitor_prohibit_over(ctx);
	if(!need_to_monitor_3)
		return 0;

	err(CH_PLANING_PREX"Monitoring Required");
	return 1;
}
void ch_planning_handle_metric_report(
	struct own_1905_device * ctx,
	struct _1905_map_device *dev,
	struct bss_info_db *bss,
	struct radio_info_db *radio,
	u8 cu_tlv_update,
	u8 force_ch_planning_monitor)
{
	u8 need_to_monitor = 0;
	debug(CH_PLANING_PREX"dev ALMAC"MACSTR" ",MAC2STR(dev->_1905_info.al_mac_addr));
	if(force_ch_planning_monitor)
		need_to_monitor = 1;
	else
		need_to_monitor = ch_planning_need_monitor(ctx,bss,radio,cu_tlv_update);
	if(!radio && bss){
		radio = bss->radio;
	}
	if(need_to_monitor) {
		if(ctx->ch_planning_R2.ch_plan_state < CHPLAN_STATE_MONITOR)
			ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_MONITOR;
		ch_planning_add_monitor_ch(ctx, dev, radio);
	}
	
	if(ch_planning_search_monitor_ch(ctx, radio)) {
		/*need to keep storing channel plan data per device*/
		ch_planning_update_data(bss, radio, cu_tlv_update);
	}
}
Boolean ch_planning_all_dev_select_done(
	struct own_1905_device *own_dev,
	struct radio_info_db *radio)
{
	struct radio_info_db *temp_radio = NULL;
	struct _1905_map_device *_1905_device = NULL, *t_1905_device = NULL;
	if(!radio) {
		err(CH_PLANING_PREX"radio is NULL");
		return FALSE;
	}
	SLIST_FOREACH_SAFE(_1905_device, &(own_dev->_1905_dev_head), next_1905_device, t_1905_device) {
		if (!(_1905_device->in_network))
			continue;
		temp_radio = topo_srv_get_radio_by_band(_1905_device,radio->channel[0]);
		if(temp_radio &&
			temp_radio->dev_ch_plan_info.dev_ch_plan_state != CHPLAN_STATE_IDLE) {
			err(CH_PLANING_PREX"FALSE  DEV("MACSTR") radio(%d) state(%d)",
				MAC2STR(_1905_device->_1905_info.al_mac_addr),
				radio->channel[0], temp_radio->dev_ch_plan_info.dev_ch_plan_state);
			return FALSE;
		}
	}
	err(CH_PLANING_PREX"TRUE");
	return TRUE;
}
void ch_planning_update_all_dev_state(
	u8 state,
	u8 channel,
	struct own_1905_device *own_dev)
{
	struct radio_info_db *temp_radio = NULL;
	struct _1905_map_device *_1905_device = NULL, *t_1905_device = NULL;

	SLIST_FOREACH_SAFE(_1905_device, &(own_dev->_1905_dev_head), next_1905_device, t_1905_device) {
		temp_radio = topo_srv_get_radio_by_band(_1905_device,channel);
		if(temp_radio)
			temp_radio->dev_ch_plan_info.dev_ch_plan_state = state;
	}
}
void ch_planning_handle_ch_selection_rsp(
	struct own_1905_device *own_dev,struct _1905_map_device *peer_1905)
{
	struct radio_info_db *radio = NULL, *t_radio = NULL;
#ifndef MAP_6E_SUPPORT
	struct mapd_radio_info *own_radio = NULL;
#endif
	struct _1905_map_device *tmp_dev = NULL, *t_tmp_dev = NULL;
	struct os_reltime rem_time = {0};
	u8 count = 0;

	SLIST_FOREACH_SAFE(radio,&(peer_1905->first_radio),next_radio, t_radio)
	{
		if (radio->dev_ch_plan_info.dev_ch_plan_state != CHPLAN_STATE_CH_CHANGE_TRIGGERED)
			continue;
		radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_IDLE;
		err(CH_PLANING_PREX"dev("MACSTR") planning done; set radio state to idle(0)",
			MAC2STR(peer_1905->_1905_info.al_mac_addr));
		if (ch_planning_all_dev_select_done(own_dev,radio)) {
#ifndef MAP_6E_SUPPORT
			struct radio_info_db *temp_radio = NULL;
			struct _1905_map_device *_1905_device = topo_srv_get_1905_device(own_dev,NULL);
			temp_radio = topo_srv_get_radio_by_band(_1905_device,radio->channel[0]);
			if(temp_radio)
				own_radio = mapd_get_radio_from_channel(own_dev->back_ptr,temp_radio->channel[0]);
			if(own_radio && own_radio->bootup_run == BOOTUP_SCAN_ONGOING){
				err(CH_PLANING_PREX"ownradio bootup run complete for ch %d", radio->channel[0]);
				own_radio->bootup_run = BOOTUP_SCAN_COMPLETED;
			}
#else
			u8 band = get_band(peer_1905,radio->channel[0]);
			u8 radio_idx = 0;
			for (radio_idx = 0; radio_idx < MAX_NUM_OF_RADIO; radio_idx++) {
				if ((own_dev->ch_planning_R2.bootup_scanstatus[radio_idx].band == band) &&
					(own_dev->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run == BOOTUP_SCAN_ONGOING))
					own_dev->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run = BOOTUP_SCAN_COMPLETED;
			}
#endif
			//dump_ch_planning_info(own_dev, 0);
			err(CH_PLANING_PREX"ch change done on all devices on radio(%d)", radio->channel[0]);
			SLIST_FOREACH_SAFE(tmp_dev, &own_dev->_1905_dev_head, next_1905_device, t_tmp_dev) {
				tmp_dev->ch_sel_req_given = 0;
			}
			count = get_net_opt_dev_count((struct mapd_global *)own_dev->back_ptr);
			if (count > 1) {
				eloop_get_remaining_timeout(&rem_time,trigger_net_opt,own_dev,NULL);
				if (rem_time.sec == 0) {
					eloop_register_timeout(own_dev->network_optimization.wait_time,
							0, trigger_net_opt, own_dev, NULL);
				}
			}
			ch_planning_R2_reset(own_dev,radio);
			handle_task_completion(own_dev);
		}
	}
}

void dump_ch_planning_info(
	struct own_1905_device *own_dev,
	u8 cmd_param)
{
	struct _1905_map_device *peer_1905 = NULL, *t_peer_1905 = NULL;
	struct radio_info_db *radio = NULL, *t_radio = NULL;
	struct monitor_ch_info *ch_info = NULL, *t_ch_info = NULL;
	u8 i;
	err("global ch_plan_enable %d", own_dev->ch_planning_R2.ch_plan_enable);
	err("global ch_plan_state %d", own_dev->ch_planning_R2.ch_plan_state);
	err("force trigger %d",own_dev->ch_planning_R2.force_trigger);
	debug("global Threshold information");
	for (i = 0; i< 2; i++)
	{
		debug("global ch_util_threshold for band %d is %d", i, own_dev->ch_planning_R2.ch_plan_thres[i].ch_util_threshold);
		debug("global edcca_threshold for band %d is %d", i, own_dev->ch_planning_R2.ch_plan_thres[i].edcca_threshold);
		debug("global obss_load_threshold for band %d is %d", i, own_dev->ch_planning_R2.ch_plan_thres[i].obss_load_threshold);
	}
	uint8_t radio_idx = 0;
	for (radio_idx = 0; radio_idx < MAX_NUM_OF_RADIO; radio_idx++) {
#ifndef MAP_6E_SUPPORT
		struct mapd_radio_info *radio_info = &own_dev->dev_radio_info[radio_idx];
		if (radio_info->radio_idx == (uint8_t)-1)
			continue;
		err("ownradio ch %d radio_info->bootup_run state %d",radio_info->channel, radio_info->bootup_run);
#else
		err("radio ch %d bootup_run state %d\n",
				own_dev->ch_planning_R2.bootup_scanstatus[radio_idx].channel,
				own_dev->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run);
#endif
	}

	err("************monitor info*************");
	SLIST_FOREACH_SAFE(ch_info, &own_dev->ch_planning_R2.first_monitor_ch, next_monitor_ch, t_ch_info) {
		err("chinfo channel_num %d", ch_info->channel_num);
		err("chinfo scan trigger_status %d", ch_info->trigger_status);
	}
	err("*********CHScoreTable****************");
	struct score_info *cumulative_score = NULL, *t_cumm_score = NULL;
	SLIST_FOREACH_SAFE(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score,next_ch_score, t_cumm_score) {
		err("CH %d , avg score %d, rank %d", cumulative_score->channel, cumulative_score->avg_score, cumulative_score->ch_rank);
	}
	if(own_dev->ch_planning_R2.ch_plan_enable_bw) {
		err("*********GroupScoreTable****************");
		struct grp_score_info *grp_score = NULL, *t_grp_score = NULL;
		SLIST_FOREACH_SAFE(grp_score,
			&own_dev->ch_planning_R2.first_grp_score,next_grp_score, t_grp_score) {
			err("CH grp [%d, %d, %d, %d, %d, %d, %d, %d], avg score %d, rank %d",
				grp_score->grp_channel_list[0],grp_score->grp_channel_list[1],
				grp_score->grp_channel_list[2],grp_score->grp_channel_list[3],
				grp_score->grp_channel_list[4],grp_score->grp_channel_list[5],
				grp_score->grp_channel_list[6],grp_score->grp_channel_list[7],
				grp_score->grp_total_avg_score,grp_score->grp_rank);
		}
		
	} 
	err("**************1905 dev info************");
	SLIST_FOREACH_SAFE(peer_1905, &(own_dev->_1905_dev_head), next_1905_device, t_peer_1905) {
		err("DEV Role %d, DEV ALMAC"MACSTR"",peer_1905->device_role, MAC2STR(peer_1905->_1905_info.al_mac_addr));
		SLIST_FOREACH_SAFE(radio, &(peer_1905->first_radio), next_radio, t_radio) {
			err("channel %d, state %d", radio->channel[0],
				radio->dev_ch_plan_info.dev_ch_plan_state);
		}
	}

	if(cmd_param == 2) {
		//want to dump all dev ch prefer info 
		err("**************1905 dev CH PREFER info************");
		struct radio_ch_prefer *ch_prefer = NULL;
		struct prefer_info_db *prefer_db = NULL, *t_prefer_db = NULL;
		u8 bw = 0, i = 0;
		t_peer_1905 = NULL;
		SLIST_FOREACH_SAFE(peer_1905, &(own_dev->_1905_dev_head), next_1905_device, t_peer_1905) {
			err("DEV Role %d, DEV ALMAC"MACSTR"",
				peer_1905->device_role, MAC2STR(peer_1905->_1905_info.al_mac_addr));
			t_radio = NULL;
			SLIST_FOREACH_SAFE(radio, &(peer_1905->first_radio), next_radio, t_radio) {
				err("current channel %d, opclass %d", radio->channel[0], radio->operating_class);
				ch_prefer = &radio->chan_preferance;
				SLIST_FOREACH_SAFE(prefer_db, &ch_prefer->prefer_info_head, prefer_info_entry, t_prefer_db) {
					bw = chan_mon_get_bw_from_op_class(prefer_db->op_class);
					err("opclass %d,bw %d pref %d", prefer_db->op_class,bw, prefer_db->perference);
					for(i=0; i<prefer_db->ch_num;i++) {
						err("ch %d",prefer_db->ch_list[i]);
					}
				}				
			}
		}
	}
	if(cmd_param != 1)
		return;
	struct scan_result_tlv *res = NULL, *t_res = NULL;
	err("*************Scan results dump***********");
	t_peer_1905 = NULL;
	SLIST_FOREACH_SAFE(peer_1905, &(own_dev->_1905_dev_head), next_1905_device, t_peer_1905) {
		err("^^^^^^DEV ALMAC^^^^^"MACSTR"",MAC2STR(peer_1905->_1905_info.al_mac_addr));
		t_radio = NULL;
		SLIST_FOREACH_SAFE(radio, &(peer_1905->first_radio), next_radio, t_radio) {
			err("------radio channel %d--------", radio->channel[0]);
			SLIST_FOREACH_SAFE(res, &(radio->first_scan_result), next_scan_result, t_res) {
				err("Scan results for channel %d", res->channel);
				err("Util %d, NBnum %d score %d", res->utilization, res->neighbor_num, res->ch_score);
				debug("EDCCA %d, ch %d",res->cu_distribution.edcca_airtime, res->cu_distribution.ch_num);
			}
		}
	}
	
}

void ch_planning_R2_force_trigger(
	struct mapd_global *global,
	u8 channel)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_map_device *_1905_dev = NULL;
	struct radio_info_db *radio = NULL;
	struct monitor_ch_info *new_ch_info = NULL;
#ifdef MAP_6E_SUPPORT
	struct _1905_map_device *t_1905_dev = NULL;
#endif
	global->dev.ch_planning_R2.ch_plan_enable = TRUE;
	ch_planning_R2_reset(&global->dev,NULL);
#ifndef MAP_6E_SUPPORT
	_1905_dev = topo_srv_get_1905_device(&global->dev, NULL);
	radio = topo_srv_get_radio_by_band(_1905_dev, channel);
	if (radio == NULL) {
		err("radio is NULL return");
		return;
	}
#endif
	/*Add channel to monitor list*/
	new_ch_info = os_zalloc(sizeof(struct monitor_ch_info));
	if (new_ch_info == NULL) {
		err(CH_PLANING_PREX"alloc memory fail for new_ch_info");
		assert(0);
		return ;
	}
#ifndef MAP_6E_SUPPORT
	new_ch_info->channel_num = radio->channel[0];
	new_ch_info->band = radio->band;
#else
	SLIST_FOREACH_SAFE(_1905_dev, &(global->dev._1905_dev_head), next_1905_device, t_1905_dev) {
		radio = topo_srv_get_radio_by_band(_1905_dev, channel);
		if (radio == NULL)
			continue;
		new_ch_info->channel_num = channel;
		new_ch_info->band = radio->band;
		break;
	}
#endif
	SLIST_INSERT_HEAD(&ctx->ch_planning_R2.first_monitor_ch,
		new_ch_info, next_monitor_ch);
	SLIST_INIT(&new_ch_info->first_affected_agent);

	/*set force trigger flag*/
	ctx->ch_planning_R2.force_trigger = 1;
	ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_MONITOR;
	ch_planning_update_all_dev_state((u8)CHPLAN_STATE_MONITOR,new_ch_info->channel_num,ctx);
	/*Go for scan start*/
	eloop_register_timeout(0, 0,
			channel_monitor_timeout, ctx, new_ch_info);

}
void ch_planning_R2_bootup_handling(
	struct own_1905_device *ctx)
{
	uint8_t radio_idx = 0;
	for (radio_idx = 0; radio_idx < MAX_NUM_OF_RADIO; radio_idx++) {
#ifndef MAP_6E_SUPPORT
		struct mapd_radio_info *radio_info = &ctx->dev_radio_info[radio_idx];
		if (radio_info->radio_idx == (uint8_t)-1)
			continue;
		if(radio_info->bootup_run != BOOTUP_SCAN_COMPLETED && radio_info->bootup_run != BOOTUP_SCAN_ERROR &&
			ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_IDLE &&
			ctx->user_triggered_scan == FALSE &&
			ctx->network_optimization.network_opt_state == NETOPT_STATE_IDLE) {
			err(CH_PLANING_PREX"cont bootup run on ch(%d)", radio_info->channel);
			radio_info->bootup_run = BOOTUP_SCAN_ONGOING;
			ch_planning_R2_force_trigger(ctx->back_ptr, radio_info->channel);
			if(ctx->ch_planning_R2.ch_monitor_start_ts.sec == 0)
				os_get_time(&ctx->ch_planning_R2.ch_monitor_start_ts);
		}
#else
		if(ctx->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run != BOOTUP_SCAN_COMPLETED &&
			ctx->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run != BOOTUP_SCAN_ERROR &&
			ctx->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run != BOOTUP_SCAN_NOT_NEEDED &&
			ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_IDLE &&
			ctx->user_triggered_scan == FALSE &&
			ctx->network_optimization.network_opt_state == NETOPT_STATE_IDLE) {
			err(CH_PLANING_PREX"cont bootup run on ch(%d)", ctx->ch_planning_R2.bootup_scanstatus[radio_idx].channel);
			ctx->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run = BOOTUP_SCAN_ONGOING;
			ch_planning_R2_force_trigger(ctx->back_ptr, 
				ctx->ch_planning_R2.bootup_scanstatus[radio_idx].channel);
			if(ctx->ch_planning_R2.ch_monitor_start_ts.sec == 0)
				os_get_time(&ctx->ch_planning_R2.ch_monitor_start_ts);
		}
#endif
	}
}

void ch_planning_handle_ongoing_cac(struct _1905_map_device *dev,
	struct own_1905_device *ctx,
	struct radio_info_db *radio)
{
	struct radio_info_db *tmp_radio = NULL;
	if (!radio || !dev) {
		err(CH_PLANING_PREX"%s is NULL", radio == NULL ? "radio" : "dev");
		return;
	}

	info("radio_ch band: %u, CAC_ch band: %u", get_band(dev, radio->channel[0]),
	get_band(dev, ctx->ch_planning_R2.CAC_on_channel));
	if (get_band(dev, radio->channel[0]) != get_band(dev, ctx->ch_planning_R2.CAC_on_channel)) {
		ctx->ch_planning_R2.need_cac_on_channel = ctx->ch_planning_R2.CAC_on_channel;
		ctx->ch_planning_R2.CAC_on_channel = 0;
		err(CH_PLANING_PREX"set need_cac_ch: %u", ctx->ch_planning_R2.need_cac_on_channel);
		/* Since radar has triggered on other band(5GL), 5GH radio of controller set to IDLE so that ch plan will run for 5GL first */
		tmp_radio = topo_srv_get_radio_by_band(dev, ctx->ch_planning_R2.need_cac_on_channel);
		if (tmp_radio && tmp_radio->dev_ch_plan_info.dev_ch_plan_state == CHPLAN_STATE_CH_CHANGE_TRIGGERED) {
			tmp_radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_IDLE;
		}
	} else {
		if (eloop_is_timeout_registered(trigger_net_opt,(void *)ctx , NULL)) {
			eloop_cancel_timeout(trigger_net_opt, ctx, NULL);
		}
	}
}

void reset_ongoing_CAC(struct own_1905_device *ctx, struct radio_info_db *radio)
{
	if (radio && radio->channel[0] == ctx->ch_planning_R2.CAC_on_channel) {
		err("Reset CAC on going");
		ctx->ch_planning_R2.CAC_on_channel = 0;
		ctx->ch_planning_R2.cac_ongoing = 0;
	}
	if(radio && radio->cac_channel != 0)
	{
		err("radio cac channel reset");
		radio->cac_channel = 0;
	}
}

Boolean check_ongoing_CAC(struct own_1905_device *ctx)
{
	int band;
	struct _1905_map_device *dev = NULL;
	struct radio_info_db *radio = NULL;
	struct bh_link_entry *bh_entry = NULL, *t_bh_entry = NULL;

	dev = topo_srv_get_1905_device(ctx, NULL);
	if (!dev) {
		err("Own 1905dev not found");
		return FALSE;
	}

	if (ctx->ch_planning_R2.cac_ongoing && ctx->ch_planning_R2.CAC_on_channel) {
		SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, t_bh_entry) {
			band = get_band(dev, ctx->ch_planning_R2.CAC_on_channel);
			radio = topo_srv_get_radio(dev, bh_entry->radio_identifier);
			if (radio) {
				err("CAC band:%d, radio_band:%u", band, radio->band);
			}
			if (radio && radio->band == band && bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED) {
				bh_entry->priority_info.priority_bkp = bh_entry->priority_info.priority;
				bh_entry->priority_info.priority = 0;
				return TRUE;
			}
		}
	}
	return FALSE;
}


void ch_planning_scan_restart_due_to_failure(struct mapd_global *global)
{
	struct own_1905_device *own_dev = &global->dev;
	struct _1905_map_device *dev = topo_srv_get_1905_device(own_dev,NULL);
	struct monitor_ch_info *ch_info = NULL, *t_ch_info = NULL;
	uint8_t radio_idx = 0;	
	if ((global->dev.user_triggered_scan == FALSE)) {
		for (radio_idx = 0; radio_idx < MAX_NUM_OF_RADIO; radio_idx++) {
#ifndef MAP_6E_SUPPORT
			struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[radio_idx];
			if (radio_info->radio_idx == (uint8_t)-1)
				continue;
			if (radio_info->bootup_run != BOOTUP_SCAN_COMPLETED)
				radio_info->bootup_run = BOOTUP_SCAN_ERROR; //Special condition to run scan after 150 seconds on particular radio
			SLIST_FOREACH_SAFE(ch_info,
				&global->dev.ch_planning_R2.first_monitor_ch, next_monitor_ch, t_ch_info) {
				if((ch_info->trigger_status == TRIGGER_TRUE)
					&& (get_band(dev, ch_info->channel_num) == get_band(dev, radio_info->channel))) {
					radio_info->bootup_run = BOOTUP_SCAN_ERROR;
					break;
				}
			}
#else
			if (own_dev->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run!= BOOTUP_SCAN_COMPLETED &&
				own_dev->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run != BOOTUP_SCAN_NOT_NEEDED)
				own_dev->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run = BOOTUP_SCAN_ERROR; //Special condition to run scan after 150 seconds on particular radio
			SLIST_FOREACH_SAFE(ch_info,
				&global->dev.ch_planning_R2.first_monitor_ch, next_monitor_ch, t_ch_info) {
				if((ch_info->trigger_status == TRIGGER_TRUE)
					&& (get_band(dev, ch_info->channel_num) == own_dev->ch_planning_R2.bootup_scanstatus[radio_idx].band)) {
					own_dev->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run = BOOTUP_SCAN_ERROR;
					break;
				}
			}
#endif
		}
	} else {
		global->dev.user_triggered_scan = FALSE;
		err (CH_PLANING_PREX"User triggered scan while agent is not ready. Retrigger command after 150 seconds")
	}
	ch_planning_R2_reset(&global->dev, NULL);
	eloop_register_timeout(CH_SCAN_RETRIGGER_TIMEOUT, 0, ch_planning_R2_bootup_handling_restart, (void *)&global->dev, NULL);
	return;
}

#endif/*MAP_R2*/



