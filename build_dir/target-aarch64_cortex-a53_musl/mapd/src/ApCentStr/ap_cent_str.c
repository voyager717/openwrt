#include "includes.h"
#include "common.h"
#include "list.h"
#include "client_db.h"
#include "mapd_i.h"
#include "steer_fsm.h"
#include "data_def.h"
#include "chan_mon.h"
#ifdef SUPPORT_MULTI_AP
#include "data_def.h"
#include "topologySrv.h"
#include "tlv_parsor.h"
#endif
#include "client_mon.h"
#include "steer_action.h"
#include "ap_est.h"
#include <assert.h>
#include "wapp_if.h"
#include "ap_roam_algo.h"
#ifdef SUPPORT_MULTI_AP
#include "apSelection.h"
#endif
#ifdef CENT_STR
#include "ap_cent_str.h"
#endif
#include "network_optimization.h"

#include "eloop.h"

extern u8 ZERO_MAC_ADDR[ETH_ALEN];


#ifdef SUPPORT_MULTI_AP
#ifdef CENT_STR


static int compare_cent_str_ol_ug(const void *a, const void *b)
{
	struct client *cli1 = *(struct client **)a;
	struct client *cli2 = *(struct client **)b;

	/* If there are two clients whose seering has been attempted in the past,
	 * then give lesser priority to client whose steering was attempted most
	 * recently */
	if (os_reltime_before(&cli1->steer_cand_ts, &cli2->steer_cand_ts))
		return -1;
	else if (os_reltime_before(&cli2->steer_cand_ts, &cli1->steer_cand_ts))
		return 1;

	if (cli1->cli_steer_method != cli2->cli_steer_method) {
		return cli1->cli_steer_method - cli2->cli_steer_method;
    }


	/* For OFFLOADING/ACTIVE, give priority to candidates with greater airtime */
	if ((cli1->cli_steer_method == OFFLOADING) ||
		(cli1->cli_steer_method == ACTIVE_STANDALONE_UG)) {
			return cli2->curr_air_time - cli1->curr_air_time;
	}

	/* For Rest, priority doesn't matter */
		return 0;
}

Boolean cent_str_bs_max_cnt_reached(struct client *cli, struct mapd_global *global)
{
	if (cli->dual_band == BAND_SUPPORT_SINGLE || cli->dual_band == BAND_SUPPORT_DUAL)
		return FALSE;
	if(!cli->steer_stats.steer_band_steer_success_cnt && cli->steer_stats.steer_band_steer_fail_cnt > global->dev.cli_steer_params.cent_str_max_bs_fail){
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"Client has been determined as single band "MACSTR,MAC2STR(cli->mac_addr));
		return TRUE;
	}
	return FALSE;
}

static s8 mapd_get_ap_steer_th_1905_radio(struct mapd_global *global,  struct _1905_map_device *_1905_device, struct radio_info_db *radio)
{
	if(!_1905_device) {
		err(CENT_STEER_PREX"1905 device is NULL \n");
		assert(0);
	}

	if(!radio) {
		err(CENT_STEER_PREX"1905 radio is NULL \n");
		assert(0);
	}


	if(_1905_device->device_role == DEVICE_ROLE_CONTROLLER) {
		return (global->dev.cli_steer_params.LowRSSIAPSteerEdge_root
						+ ap_est_get_noise_offset_by_1905_radio(global,radio));
	} else {
		return (global->dev.cli_steer_params.LowRSSIAPSteerEdge_RE
						+ ap_est_get_noise_offset_by_1905_radio(global,radio));
	}
}


static Boolean ap_roam_algo_channel_measurement_allow_cent_str(struct own_1905_device *own_device,
		u8 *channel_arr, u8 channel, u8 chan_util,
		struct client *map_client)
{
	u8 chan_ol_th = 0;
	u8 chan_safety_th = 0;

#if 0
	if(isChan5GH(channel)) {
		chan_ol_th = own_device->cli_steer_params.CUOverloadTh_5G_H;
		chan_safety_th = own_device->cli_steer_params.CUSafetyh_5G_H;
	}

	else if(isChan5GL(channel)) {
		chan_ol_th = own_device->cli_steer_params.CUOverloadTh_5G_L;
		chan_safety_th = own_device->cli_steer_params.CUSafetyh_5G_L;
	}

	else {
		chan_ol_th = own_device->cli_steer_params.CUOverloadTh_2G;
		chan_safety_th = own_device->cli_steer_params.CUSafetyTh_2G;
	}
#endif
#ifdef SUPPORT_MULTI_AP
	// try to find a better AP with better airtime efficiency
	if (channel_arr != NULL) {
		if(map_client->cli_steer_method == NOL_MULTIAP
			&& ((map_client->current_chan == channel_arr[0]) ||
			(map_client->current_chan == channel_arr[1]) ||
			(map_client->current_chan == channel_arr[2]) ||
			(map_client->current_chan == channel_arr[3])))
			return TRUE;
	} else if ((channel_arr == NULL) &&
		(map_client->cli_steer_method == NOL_MULTIAP)
		&& (map_client->current_chan == channel)) {
		return TRUE;
	}
#endif
	if (channel_arr != NULL) {
		ap_roam_algo_get_ch_ol_safety_th(own_device, channel_arr[0], &chan_ol_th, &chan_safety_th);
	} else {
		ap_roam_algo_get_ch_ol_safety_th(own_device, channel, &chan_ol_th, &chan_safety_th);
	}
	chan_util = (chan_util*100)/255;
	mapd_printf(MSG_DEBUG,CENT_STEER_PREX"channel =%d ch_util=%d ol_th=%d safety=%d",
					channel, chan_util, chan_ol_th, chan_safety_th);

	if (map_client->activity_state == 0) { // idle client
		if(chan_util < chan_ol_th)
			return TRUE;
	}
	else if(chan_util < chan_safety_th)
			return TRUE;


	return FALSE;
}


static Boolean ap_roam_algo_dg_cand_cent_str(struct mapd_global *global, struct client *cli)
{
	int8_t noise_offset = 0, steer_edge_dg = 0;
	uint32_t dl_mcs_th_dg = 0;
	struct _1905_map_device * own_device = NULL;
	struct associated_clients * client_dev = NULL;
	struct bss_info_db *curr_bss = NULL;
	struct radio_info_db * own_radio = NULL;
	struct radio_info_db * radio_tmp = NULL, *tmp_radio_tmp = NULL;
	struct bss_info_db * bss_tmp = NULL, *tmp_bss_tmp = NULL;
	mapd_printf(MSG_DEBUG, CENT_STEER_PREX"  ");

	if(!cli)
		return FALSE;

	if(os_memcmp(ZERO_MAC_ADDR,cli->bssid,ETH_ALEN) == 0)
		return FALSE;


	own_device = topo_srv_get_1905_by_bssid(&global->dev, cli->bssid);

	if(!own_device) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client connect bssid ("MACSTR ") dev is null", MAC2STR(cli->bssid));
		return FALSE;
	}

	curr_bss = topo_srv_get_bss_by_bssid(&global->dev,own_device,cli->bssid);

	if(!curr_bss) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client connect bss ("MACSTR ") is null", MAC2STR(cli->bssid));
		return FALSE;
	}
	own_radio = curr_bss->radio;

	if(!own_radio) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client connect bss ("MACSTR ") radio is null", MAC2STR(cli->bssid));
		return FALSE;
	}


	client_dev = topo_srv_get_associate_client(&global->dev,own_device, cli->mac_addr);

	if(!client_dev) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client ("MACSTR ") info is null", MAC2STR(cli->mac_addr));
		return FALSE;
	}

	/* Already on 2.4G */
	if (cli->current_chan <= 14) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"current channel is 2g");
		return FALSE;
	}

	if (global->dev.cli_steer_params.disable_active_dg && cli->activity_state) {
		mapd_printf(MSG_DEBUG,CENT_STEER_PREX"disable_active_ug && cli->activity_state");
		return FALSE;
	}

	if ((global->dev.cli_steer_params.disable_idle_dg) && (!cli->activity_state)) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"disable_idle_dg && !cli->activity_state");
		return FALSE;
	}

	if(cent_str_bs_max_cnt_reached(cli, global)){
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"Client has been determined as single band "MACSTR,MAC2STR(cli->mac_addr));
		return FALSE;
	}

	SLIST_FOREACH_SAFE(radio_tmp, &(own_device->first_radio), next_radio, tmp_radio_tmp) {
		if (radio_tmp->channel[0] == 0)
			continue;

		if (radio_tmp->channel[0] > 14)
			continue;
#if	0
		if (!is_chan_supported(&cli->known_channels[0], radio_tmp->channel[0])) {
				mapd_printf(MSG_DEBUG, "client doens't support channel=%d",
								radio_info->channel);
				return FALSE;
		}
#endif
		if (chan_mon_is_1905_radio_ol(global,radio_tmp)) {
			cli->DG_fail_2gOL = 1;
				mapd_printf(MSG_DEBUG,CENT_STEER_PREX"Channel = %d is OL", radio_tmp->channel[0]);
			return FALSE;
		}

		if (cli->activity_state) {
			u8 channel_safety_th = 0;

			if (chan_mon_is_1905_radio_safety_ol(global,radio_tmp)) {
				cli->DG_fail_2gOL = 1;
				mapd_printf(MSG_DEBUG,CENT_STEER_PREX"Prospective channel %d util is"
								" above Safety Th %d", own_radio->channel[0],
								channel_safety_th);
				continue;
			}

			/* Serving */
			noise_offset = ap_est_get_noise_offset_by_1905_radio(global, own_radio);
			steer_edge_dg = noise_offset + global->dev.cli_steer_params.RSSICrossingThreshold_DG;
			dl_mcs_th_dg =  global->dev.cli_steer_params.MCSCrossingThreshold_DG;

			mapd_printf(MSG_DEBUG,CENT_STEER_PREX"noise_offset=%d steer_edge_dg=%d"
							" dl_mcs_th_dg=%d UL_RSSI=%d", noise_offset,
							steer_edge_dg, dl_mcs_th_dg,
							client_dev->rssi_uplink);
			if (((signed char)client_dev->rssi_uplink >= steer_edge_dg) &&
					(client_dev->erate_downlink * 1000  >=  dl_mcs_th_dg))
				return FALSE; //No need to try next channel
		} else {
			/* Non-serving for Idle clients */
			int8_t uplink_rssi_non_serving = 0;

			noise_offset = ap_est_get_noise_offset_by_1905_radio(global, radio_tmp);
			if (chan_mon_is_1905_radio_ol(global,radio_tmp))
				steer_edge_dg = noise_offset + global->dev.cli_steer_params.MinRSSIOverload;
			else
				steer_edge_dg = noise_offset + global->dev.cli_steer_params.RSSISteeringEdge_DG;

			uplink_rssi_non_serving = ap_est_update_non_serving_rssi_cent_str(global,cli,own_radio->band,radio_tmp->band);

			mapd_printf(MSG_DEBUG,CENT_STEER_PREX"noise_offset=%d steer_edge_dg=%d"
							" NON_serving_UL_RSSI[Channel=%d]=%d",
							noise_offset, steer_edge_dg, radio_tmp->channel[0],
							uplink_rssi_non_serving);

			if(uplink_rssi_non_serving == -110)
				continue;
			if (uplink_rssi_non_serving >= steer_edge_dg)
				continue; //Try next 2G channel
		}
		SLIST_FOREACH_SAFE(bss_tmp, &(own_device->first_bss), next_bss, tmp_bss_tmp) {
			if (bss_tmp->radio == radio_tmp ) {
				if (!os_strcmp((const char *)bss_tmp->ssid, (const char *)curr_bss->ssid)) {
						mapd_printf(MSG_INFO,CENT_STEER_PREX"Alternate BSS exists --> "
										MACSTR, MAC2STR(bss_tmp->bssid));
						return TRUE;
				}
			}

		}
	}

	return FALSE;
}


static Boolean ap_roam_algo_nol_map_cand_cent_str(struct mapd_global *global, struct client *cli,
				int steer_edge_multiap)
{
	struct _1905_map_device *_1905_device = NULL;
	int num_1905_devs = 0;
	int num_potential_alt_bss = 0;
	struct _1905_map_device * own_device = NULL;
	struct associated_clients * client_dev = NULL;
	struct bss_info_db *own_bss = NULL;

	mapd_printf(MSG_DEBUG, CENT_STEER_PREX" ");

	if(!cli)
		return FALSE;

	if(os_memcmp(ZERO_MAC_ADDR,cli->bssid,ETH_ALEN) == 0)
		return FALSE;

	own_device = topo_srv_get_1905_by_bssid(&global->dev, cli->bssid);

	if(!own_device) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client connect bssid ("MACSTR ") dev is null", MAC2STR(cli->bssid));
		return FALSE;
	}

	own_bss = topo_srv_get_bss_by_bssid(&global->dev,own_device,cli->bssid);

	if(!own_bss) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client connect bss ("MACSTR ") is null", MAC2STR(cli->bssid));
		return FALSE;
	}

	client_dev = topo_srv_get_associate_client(&global->dev,own_device, cli->mac_addr);

	if(!client_dev) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client mac("MACSTR ") info is null", MAC2STR(cli->mac_addr));
		return FALSE;
	}

	if (global->dev.cli_steer_params.disable_nolmultiap) {
		mapd_printf(MSG_DEBUG,CENT_STEER_PREX"disable_nolmultiap ");
		return FALSE;
	}

	num_1905_devs = topo_srv_get_1905_dev_count(&global->dev);

	if (num_1905_devs <= 1) {
		mapd_printf(MSG_DEBUG,CENT_STEER_PREX"num_1905_devs:%d ", num_1905_devs);
		return FALSE;
	}

	if ((signed char)client_dev->rssi_uplink > steer_edge_multiap) {
		mapd_printf(MSG_DEBUG,CENT_STEER_PREX"rssi:%d, steer_edge_multiap:%d", (signed char)client_dev->rssi_uplink, steer_edge_multiap);
		return FALSE;
	}

	if((cli->csbc_data.btm_state == CSBC_BTM_DISALLOWED && cli->csbc_data.force_str_state == CSBC_FORCED_ALLOWED )
		&& ((signed char)client_dev->rssi_uplink > global->dev.cli_steer_params.force_roam_rssi_th))
		return FALSE;


	_1905_device = topo_srv_get_next_1905_device(&global->dev, NULL);
	_1905_device = topo_srv_get_next_1905_device(&global->dev, _1905_device);

    if (_1905_device == NULL) {
        mapd_printf(MSG_ERROR,CENT_STEER_PREX"Multiple devices not present. Can't reach here");
		return FALSE;
	}

	while (_1905_device) {
		   struct bss_info_db *bss = NULL;
		   while ((_1905_device->in_network) &&
				  ((bss = topo_srv_get_next_bss(_1905_device, bss)) != NULL)) {
					mapd_printf(MSG_DEBUG,CENT_STEER_PREX"BSS " MACSTR " chan=%d", MAC2STR(bss->bssid),
						bss->radio->channel[0]);
/*
				if (!global->params.Certification &&
					!is_chan_supported(cli->known_channels,
						bss->radio->channel[0]))
					  continue;

					mapd_printf(MSG_DEBUG, "Client supports this channel");
*/
				if (bss->radio->channel[0] != cli->current_chan &&
					bss->radio->channel[1] != cli->current_chan &&
					bss->radio->channel[2] != cli->current_chan &&
					bss->radio->channel[3] != cli->current_chan &&
					bss->radio->channel[4] != cli->current_chan &&
					bss->radio->channel[5] != cli->current_chan &&
					bss->radio->channel[6] != cli->current_chan &&
					bss->radio->channel[7] != cli->current_chan &&
#ifdef MAP_320BW
					bss->radio->channel[8] != cli->current_chan &&
					bss->radio->channel[9] != cli->current_chan &&
					bss->radio->channel[10] != cli->current_chan &&
					bss->radio->channel[11] != cli->current_chan &&
					bss->radio->channel[12] != cli->current_chan &&
					bss->radio->channel[13] != cli->current_chan &&
					bss->radio->channel[14] != cli->current_chan &&
					bss->radio->channel[15] != cli->current_chan &&
#endif
					ap_roam_algo_channel_measurement_allow_cent_str(&global->dev,
						bss->radio->channel, 0, bss->ch_util, cli) == FALSE)
					continue;

				  if (!SSID_EQUAL(bss->ssid, bss->ssid_len, own_bss->ssid, own_bss->ssid_len))
					  continue;
					mapd_printf(MSG_DEBUG,CENT_STEER_PREX"Current and Potential target SSID are same");
				  if ((!(cli->capab & CAP_11K_SUPPORTED)) &&
					(bss->radio->channel[0] != cli->current_chan &&
					bss->radio->channel[1] != cli->current_chan &&
					bss->radio->channel[2] != cli->current_chan &&
					bss->radio->channel[3] != cli->current_chan &&
					bss->radio->channel[4] != cli->current_chan &&
					bss->radio->channel[5] != cli->current_chan &&
					bss->radio->channel[6] != cli->current_chan &&
#ifdef MAP_320BW
					bss->radio->channel[7] != cli->current_chan &&
					bss->radio->channel[8] != cli->current_chan &&
					bss->radio->channel[9] != cli->current_chan &&
					bss->radio->channel[10] != cli->current_chan &&
					bss->radio->channel[11] != cli->current_chan &&
					bss->radio->channel[12] != cli->current_chan &&
					bss->radio->channel[13] != cli->current_chan &&
					bss->radio->channel[14] != cli->current_chan &&
					bss->radio->channel[15] != cli->current_chan))
#else
					bss->radio->channel[7] != cli->current_chan))
#endif
					continue;
				mapd_printf(MSG_DEBUG,CENT_STEER_PREX"Client is capable for NOL_MULTIAP to this BSS");
				/* Potential BSS for NOL_MULTIAP */
				num_potential_alt_bss ++;
		   }
		   _1905_device = topo_srv_get_next_1905_device(&global->dev,_1905_device);
	}
	if (num_potential_alt_bss == 0) {
		mapd_printf(MSG_DEBUG,CENT_STEER_PREX"num_potential_alt_bss is 0, no suitable bss");
		return FALSE;
	}

	mapd_printf(MSG_INFO,CENT_STEER_PREX " " MACSTR " is a NOL_MULTIAP cand(num_potential_alt_bss=%d)",
					MAC2STR(cli->mac_addr), num_potential_alt_bss);
	return TRUE;
}

static Boolean ap_roam_algo_ug_cand_cent_str(struct mapd_global *global, struct client *cli)
{
	int8_t noise_offset = 0, steer_edge_ug = 0;
	uint32_t dl_mcs_th_ug = 0;
	struct _1905_map_device * own_device = NULL;
	struct associated_clients * client_dev = NULL;
	struct bss_info_db *curr_bss = NULL;
	struct radio_info_db * own_radio = NULL;
	struct radio_info_db * radio_tmp = NULL, *tmp_radio_tmp = NULL;
	struct bss_info_db * bss_tmp = NULL, *tmp_bss_tmp = NULL;

	if(!cli)
		return FALSE;
	mapd_printf(MSG_DEBUG,CENT_STEER_PREX" " MACSTR, MAC2STR(cli->mac_addr));

	if(os_memcmp(ZERO_MAC_ADDR,cli->bssid,ETH_ALEN) == 0)
		return FALSE;

	own_device = topo_srv_get_1905_by_bssid(&global->dev, cli->bssid);

	if(!own_device) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client connect bssid ("MACSTR ") dev is null", MAC2STR(cli->bssid));
		return FALSE;
	}

	curr_bss = topo_srv_get_bss_by_bssid(&global->dev,own_device,cli->bssid);

	if(!curr_bss) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client connect bss ("MACSTR ") is null", MAC2STR(cli->bssid));
		return FALSE;
	}

	own_radio = curr_bss->radio;

	if(!own_radio) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client connect bss ("MACSTR ") radio is null", MAC2STR(cli->bssid));
		return FALSE;
	}

	client_dev = topo_srv_get_associate_client(&global->dev,own_device, cli->mac_addr);

	if(!client_dev) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client mac ("MACSTR ") info is null", MAC2STR(cli->mac_addr));
		return FALSE;
	}

	/* Already on 5G */
	if (cli->current_chan > 14) { //already on 5G
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"cli->current_chan(%d) is 5g", cli->current_chan);
		return FALSE;
	}
	if (global->dev.cli_steer_params.disable_active_ug && cli->activity_state) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"disable_active_ug && cli->activity_state");
		return FALSE;
	}

	if ((global->dev.cli_steer_params.disable_idle_ug) && (!cli->activity_state)) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"disable_idle_ug && !cli->activity_state");
		return FALSE;
	}

	if(cent_str_bs_max_cnt_reached(cli, global)) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"Client has been determined as single band "MACSTR,MAC2STR(cli->mac_addr));
		return FALSE;
	}


	/* Cannot upgrade Non-HT clients, if Phy Based is on*/
    if ((global->dev.cli_steer_params.PHYBasedSelection) &&
		((cli->phy_capab.phy_mode[BAND_5G_IDX] == LEGACY_MODE))) {
		mapd_printf(MSG_DEBUG,CENT_STEER_PREX"Non-HT and PhyBased Selection -- cannot upgrade");
		return FALSE;
	}

	SLIST_FOREACH_SAFE(radio_tmp, &(own_device->first_radio), next_radio, tmp_radio_tmp) {

		if (radio_tmp->channel[0] == 0)
			continue;

		if (radio_tmp->channel[0] <= 14)
			continue;
/*
		if (!is_chan_supported(&cli->known_channels[0], radio_info->channel)) {
				mapd_printf(MSG_DEBUG, "client doesn't support channel=%d",
								radio_info->channel);
				continue;
		}
*/
		if (chan_mon_is_1905_radio_ol(global,radio_tmp)) {
				mapd_printf(MSG_ERROR,CENT_STEER_PREX"Prospective channel = %d is OL",
								radio_tmp->channel[0]);
				continue;
		}
		if (cli->activity_state) {
			u8 channel_safety_th = 0;

			ap_roam_algo_get_ch_ol_safety_th(&global->dev, radio_tmp->channel[0],
							NULL, &channel_safety_th);
			if (chan_mon_is_1905_radio_safety_ol(global,radio_tmp)) {
				mapd_printf(MSG_ERROR,CENT_STEER_PREX"Prospective channel %d util is"
								" above Safety Th %d", radio_tmp->channel[0],
								channel_safety_th);
				continue;
			}

			/* Serving */
			noise_offset = ap_est_get_noise_offset_by_1905_radio(global, own_radio);
			steer_edge_ug = noise_offset + global->dev.cli_steer_params.RSSICrossingThreshold_UG;
			dl_mcs_th_ug =  global->dev.cli_steer_params.MCSCrossingThreshold_UG;

			mapd_printf(MSG_DEBUG,CENT_STEER_PREX"noise_offset=%d steer_edge_ug=%d"
							" dl_mcs_th_ug=%d UL_RSSI=%d", noise_offset,
							steer_edge_ug, dl_mcs_th_ug,
							client_dev->rssi_uplink);

			if ((client_dev->rssi_uplink <= steer_edge_ug) ||
					(client_dev->erate_downlink * 1000  <= dl_mcs_th_ug))
				return FALSE; //No need to loop over next channel
		} else {
		int8_t 	uplink_rssi_non_serving = 0;

			/* Non-serving for Idle clients */
			noise_offset = ap_est_get_noise_offset_by_1905_radio(global, radio_tmp);
			/* Different Thresholds depending upon current channel condition */
			if (chan_mon_is_1905_radio_ol(global,radio_tmp))
				steer_edge_ug = noise_offset +
						global->dev.cli_steer_params.MinRSSIOverload;
			else
				steer_edge_ug = noise_offset +
						global->dev.cli_steer_params.RSSISteeringEdge_UG;

			uplink_rssi_non_serving = ap_est_update_non_serving_rssi_cent_str(global,cli,own_radio->band,radio_tmp->band);


			mapd_printf(MSG_DEBUG,CENT_STEER_PREX"noise_offset=%d steer_edge_ug=%d"
							" NON_serving_UL_RSSI[Channel=%d]=%d",
							noise_offset, steer_edge_ug, radio_tmp->channel[0],
							uplink_rssi_non_serving);
			if(uplink_rssi_non_serving == -110)
				continue;

			if (uplink_rssi_non_serving <= steer_edge_ug)
				continue; //Try next 5G channel
		}

		/* Look for same ssid on this radio */

		SLIST_FOREACH_SAFE(bss_tmp, &(own_device->first_bss), next_bss, tmp_bss_tmp) {
			if (bss_tmp->radio == radio_tmp ) {
				if (!os_strcmp((const char *)bss_tmp->ssid, (const char *)curr_bss->ssid)) {
						mapd_printf(MSG_INFO,CENT_STEER_PREX"Alternate BSS exists --> "
										MACSTR, MAC2STR(bss_tmp->bssid));
						return TRUE;
				}
			}

		}
	}


	return FALSE;
}


static Boolean ap_roam_algo_offloading_cand_cent_str(struct mapd_global *pGlobal, struct client *cli)
{
	struct _1905_map_device *_1905_device = NULL;

	int num_potential_alt_bss = 0;
	struct bss_info_db *own_bss = NULL;
	struct _1905_map_device * own_device = NULL;
	struct bss_info_db *curr_bss = NULL;
	struct radio_info_db * own_radio = NULL;

	mapd_printf(MSG_DEBUG,CENT_STEER_PREX" ");

	if(!cli)
		return FALSE;

	if(os_memcmp(ZERO_MAC_ADDR,cli->bssid,ETH_ALEN) == 0)
		return FALSE;


	own_device = topo_srv_get_1905_by_bssid(&pGlobal->dev, cli->bssid);

	if(!own_device) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client connect bssid ("MACSTR ") dev is null", MAC2STR(cli->bssid));
		return FALSE;
	}
	curr_bss = topo_srv_get_bss_by_bssid(&pGlobal->dev,own_device,cli->bssid);
	if(!curr_bss) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client connect bss ("MACSTR ") is null", MAC2STR(cli->bssid));
		return FALSE;
	}
	own_radio = curr_bss->radio;

	if(!own_radio) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client connect bss ("MACSTR ") radio is null", MAC2STR(cli->bssid));
		return FALSE;
	}


	if (pGlobal->dev.cli_steer_params.disable_offloading) {
		mapd_printf(MSG_DEBUG,CENT_STEER_PREX"disable_offloading");
		return FALSE;
	}

	if (!cli->activity_state) {
		mapd_printf(MSG_DEBUG,CENT_STEER_PREX"!cli->activity_state");
		return FALSE;
	}

	if (!(cli->capab & CAP_11K_SUPPORTED)) {
		mapd_printf(MSG_DEBUG,CENT_STEER_PREX"cli do not support 11k ");
		return FALSE;
	}

	if(cent_str_bs_max_cnt_reached(cli, pGlobal)) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"Client has been determined as single band "MACSTR,MAC2STR(cli->mac_addr));
		return FALSE;
	}

	if (!chan_mon_is_1905_radio_ol(pGlobal,own_radio))
		return FALSE;

	own_bss = topo_srv_get_bss_by_bssid(&pGlobal->dev, NULL, cli->bssid);

	if (own_bss == NULL) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"Map client not connected to us");
		return FALSE;
	}

	_1905_device = topo_srv_get_next_1905_device(&pGlobal->dev, NULL);

	if (_1905_device == NULL) {
		mapd_ASSERT(0);
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"_1905_device is null");
		return FALSE;
	}

	while(_1905_device) {
		struct bss_info_db *bss = NULL;

		while ((_1905_device->in_network) &&
			   ((bss = topo_srv_get_next_bss(_1905_device, bss)) != NULL)) {
/*				if (!is_chan_supported(cli->known_channels,
				bss->radio->channel[0], get_client_max_bw(bss->radio->channel[0], cli)))
					continue;
*/
				if (ap_roam_algo_channel_measurement_allow_cent_str(&pGlobal->dev,
						bss->radio->channel, 0, bss->ch_util, cli) == FALSE)
					continue;
				if (!SSID_EQUAL(bss->ssid, bss->ssid_len, own_bss->ssid, own_bss->ssid_len))
					continue;
				num_potential_alt_bss ++;
		}
		_1905_device = topo_srv_get_next_1905_device(&pGlobal->dev,_1905_device);
	}

	if (num_potential_alt_bss == 0) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"num_potential_alt_bss is 0");
		return FALSE;
	}

	mapd_printf(MSG_INFO, MACSTR CENT_STEER_PREX"is a OFFLOADING cand(num_potential_alt_bss=%d)",
					MAC2STR(cli->mac_addr), num_potential_alt_bss);
	return TRUE;
}


static int ap_roam_algo_post_assoc_phy_based_sel_cent_str(struct mapd_global *global, struct client *cli)
{
	struct bss_info_db *bss_tmp = NULL, *tmp_bss_tmp = NULL;
	int ret = NONE;
	struct _1905_map_device * own_device = NULL;
	struct bss_info_db *curr_bss = NULL;
	struct radio_info_db * own_radio = NULL;
	struct radio_info_db * radio_tmp = NULL, *tmp_radio_tmp = NULL;

	if(!cli)
		return FALSE;

	if(os_memcmp(ZERO_MAC_ADDR,cli->bssid,ETH_ALEN) == 0)
		return FALSE;

	own_device = topo_srv_get_1905_by_bssid(&global->dev, cli->bssid);

	if(!own_device){
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client connect bssid ("MACSTR ") dev is null", MAC2STR(cli->bssid));
		return NONE;
	}
	curr_bss = topo_srv_get_bss_by_bssid(&global->dev,own_device,cli->bssid);

	if(!curr_bss){
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client connect bss ("MACSTR ") is null", MAC2STR(cli->bssid));
		return NONE;
	}
	own_radio = curr_bss->radio;

	if(!own_radio)
		return NONE;

	if(cent_str_bs_max_cnt_reached(cli, global))
		return FALSE;

	SLIST_FOREACH_SAFE(radio_tmp, &(own_device->first_radio), next_radio, tmp_radio_tmp) {

		if (radio_tmp->channel[0] == 0)
			continue;
		if (radio_tmp->channel[0] <= 14)
			continue;
/*		if (!is_chan_supported(&cli->known_channels[0], radio_info->channel))
			continue;
*/

		if (chan_mon_is_1905_radio_ol(global,radio_tmp))
			continue;
		if (cli->activity_state) {
#if 0
			if (radio_info->ch_util >
					(isChan5GL(radio_info->channel) ?
					 global->dev.cli_steer_params.CUSafetyTh_5G_L :
					 global->dev.cli_steer_params.CUSafetyh_5G_H))
				continue;
#endif
		}
		if ((cli->phy_capab.phy_mode[BAND_5G_IDX] == VHT_MODE) &&
				(isChan5GL(cli->current_chan) && isChan5GH(radio_tmp->channel[0])))
			ret = (cli->activity_state ? ACTIVE_5GL_TO_5GH : IDLE_5GL_TO_5GH);
		else if ((cli->phy_capab.phy_mode[BAND_5G_IDX] != VHT_MODE) &&
				isChan5GH(cli->current_chan) && isChan5GL(radio_tmp->channel[0]))
			ret = (cli->activity_state ? ACTIVE_5GH_TO_5GL : IDLE_5GH_TO_5GL);
		if (ret != NONE) {
			SLIST_FOREACH_SAFE(bss_tmp, &(own_device->first_bss), next_bss, tmp_bss_tmp) {
				if (bss_tmp->radio == radio_tmp ) {
					if (!os_strcmp((const char *)bss_tmp->ssid, (const char *)curr_bss->ssid)) {
							mapd_printf(MSG_INFO,CENT_STEER_PREX"Alternate BSS exists --> "
											MACSTR, MAC2STR(bss_tmp->bssid));
							return TRUE;
					}
				}

			}

		}
	}

	return NONE;
}


struct cent_steer_fail_cands *get_client_from_cent_steer_fail_cand_list(
	struct own_1905_device *ctx,
	struct client *cli)
{
	struct cent_steer_fail_cands *f_cand = NULL;
	if(!cli) {
		err(CENT_STEER_PREX"cli is NULL");
		return NULL;
	}
	if (STAILQ_EMPTY(&(ctx->cent_steer_fail_cands_head)))
		return NULL;
	f_cand = STAILQ_FIRST(&(ctx->cent_steer_fail_cands_head));
	while(f_cand != NULL) {
		if(os_memcmp(cli->mac_addr,f_cand->fail_steer_cand->mac_addr,ETH_ALEN) == 0)
			return f_cand;
		f_cand = STAILQ_NEXT(f_cand, next_cand);
	}
	return NULL;
}
void insert_entry_fail_candidate_list(
	struct client *cli, 
	struct own_1905_device *ctx)
{
	struct cent_steer_fail_cands *cand = NULL;
	if(!cli) {
		err(CENT_STEER_PREX"cli is NULL");
		return;
	}
	cand = get_client_from_cent_steer_fail_cand_list(ctx, cli);
	if(cand!=NULL) {
		err("avoid making duplicate entry ");
		return;
	}
	cand = os_zalloc(sizeof(*cand));
	if(!cand) {
		err("alloc fail");
		return;
	}
	cand->fail_steer_cand = cli;
	cand->fail_steer_cand->str_reattempt_cnt = 0;
	STAILQ_INSERT_TAIL(&ctx->cent_steer_fail_cands_head, cand, next_cand);
	mapd_printf(MSG_INFO,CENT_STEER_PREX"client (" MACSTR ") on agent is a %s fail candidate",
		MAC2STR(cli->mac_addr), str_method_str(cli->cli_steer_method));
}


void remove_entry_fail_candidate_list(
	struct client *fail_cli, 
	struct own_1905_device *ctx)
{
	struct cent_steer_fail_cands *fail_cand = NULL;
	if(!fail_cli) {
		err(CENT_STEER_PREX"cli is NULL");
		return;
	}
	fail_cand = get_client_from_cent_steer_fail_cand_list(ctx,fail_cli);
	if(!fail_cand){
		info("no candidate found in fail list return");
		return;
	}
	mapd_printf(MSG_INFO,CENT_STEER_PREX"REMOVE client (" MACSTR ") from DG steer fail candidate list",
		MAC2STR(fail_cli->mac_addr));
	fail_cand->fail_steer_cand->str_reattempt_cnt = 0;
	fail_cand->fail_steer_cand = NULL;
	STAILQ_REMOVE(&(ctx->cent_steer_fail_cands_head), fail_cand, cent_steer_fail_cands, next_cand);
	os_free(fail_cand);
}

void remove_all_fail_candidate_list(struct mapd_global *global)
{
	struct cent_steer_fail_cands *cand = NULL, *temp_cand = NULL;
	struct own_1905_device *ctx = &global->dev;

	if(!STAILQ_EMPTY(&ctx->cent_steer_fail_cands_head)) {
		cand = STAILQ_FIRST(&ctx->cent_steer_fail_cands_head);
		while (cand) {
			temp_cand = STAILQ_NEXT(cand, next_cand);
			cand->fail_steer_cand->str_reattempt_cnt = 0;
			cand->fail_steer_cand = NULL;
			STAILQ_REMOVE(&ctx->cent_steer_fail_cands_head, cand, cent_steer_fail_cands,next_cand);
			os_free(cand);
			cand = NULL;
			cand = temp_cand;
		}
	}
}
void cent_str_reattempt_on_demand_steer(struct mapd_global *global)
{
	struct own_1905_device *ctx = &global->dev;
	struct cent_steer_cands *cand = NULL;
	struct cent_steer_fail_cands *fail_cand = NULL, *temp_cand = NULL;
	struct _1905_map_device *tmp_dev = NULL;
	struct radio_info_db * radio_tmp = NULL, *tmp_radio_tmp = NULL;
	struct _1905_map_device * _1905_owndev = NULL;
	u8 ret = 0;

	if(!STAILQ_EMPTY(&ctx->cent_steer_fail_cands_head)) {
		fail_cand = STAILQ_FIRST(&ctx->cent_steer_fail_cands_head);
		while (fail_cand) {
			temp_cand = STAILQ_NEXT(fail_cand, next_cand);
			ret = 1;

			if(fail_cand->fail_steer_cand->str_reattempt_cnt == MAX_STR_REATTEMPT_CNT) {
				remove_entry_fail_candidate_list(fail_cand->fail_steer_cand,ctx);
				fail_cand = temp_cand;
				continue;
			}

			/* check if cli is not already present in the steer cands list */
			cand = get_client_from_cent_steer_cand_list(ctx,fail_cand->fail_steer_cand);
			if(cand != NULL) {
				info("failed cli is already in the steer cands list , so no need attempt again");
				ret = 0;
				//need delete cli from failed list  and return
				remove_entry_fail_candidate_list(fail_cand->fail_steer_cand,ctx);
				fail_cand = temp_cand;
				continue;
			}

			/*Check if 2.4G radio is still OL */
			_1905_owndev = topo_srv_get_1905_by_bssid(&global->dev, fail_cand->fail_steer_cand->bssid);
			if(_1905_owndev == NULL) {
				remove_entry_fail_candidate_list(fail_cand->fail_steer_cand,ctx);
				fail_cand = temp_cand;
				continue;
			}
			SLIST_FOREACH_SAFE(radio_tmp, &(_1905_owndev->first_radio), next_radio, tmp_radio_tmp) {
				if (radio_tmp->channel[0] == 0)
					continue;
				if (radio_tmp->channel[0] > 14)
					continue;
				if (chan_mon_is_1905_radio_ol(global,radio_tmp)) {
					info("2.4G is still OL ");
					return;
				}
			}

			/* send trigger to correct 1905dev on which CLI is connected*/
			if(ret) {
				tmp_dev = topo_srv_get_1905_by_bssid(ctx, fail_cand->fail_steer_cand->bssid);
				if(!tmp_dev) {
					err("tmp dev for retrigger on demand not found");
					return;
				}

				fail_cand->fail_steer_cand->str_reattempt_cnt ++;
				mapd_cent_str_assoc_sta_link_metric_query(global, tmp_dev, fail_cand);
			}
			fail_cand = temp_cand;
		}
	}
}

void client_mon_chk_post_assoc_cent_str(struct mapd_global *global)
{
	struct cent_steer_cands *cand = NULL, *tmp_cand = NULL;
	struct own_1905_device *ctx = &global->dev;
	int cand_cnt = 0;

	if(global->dev.SetSteer==STEER_DISABLE){
		mapd_printf(MSG_INFO,CENT_STEER_PREX"Steering is disabled");
		return;
	}

	/* Check if post-assoc steering is disabled */
	if (global->dev.cli_steer_params.disable_post_assoc_strng)
		return;

	/* Steering already in progress */
	cand = NULL;
	cent_str_reattempt_on_demand_steer(global);
#ifdef SUPPORT_MULTI_AP
	STAILQ_FOREACH_SAFE(cand, &ctx->cent_steer_cands_head, next_cand, tmp_cand) {
	struct client *cli = cand->steer_cand;


	if(cli->cli_steer_state != CLI_STATE_IDLE)
		continue;


	if(cand_cnt >= ctx->cli_steer_params.cent_str_max_steer_cand){
		err(CENT_STEER_PREX"Max Steer Cand reached %d",cand_cnt);
		break;
	}

	mapd_printf(MSG_ERROR,CENT_STEER_PREX"select Client(%d) " MACSTR" do steer \n", cli->client_id, MAC2STR(cli->mac_addr));
	steer_fsm_trigger(global, cli->client_id,
					mapd_get_trigger_from_steer_method(global,
							cli->cli_steer_method), NULL);
	cand_cnt++;
	if (STAILQ_EMPTY(&(ctx->cent_steer_cands_head))){
		return;
	}
	else {
		continue;
	}

	}
#endif

}

void cent_str_select_on_demand_str_method(struct own_1905_device *ctx,struct client *cli) {

struct mapd_global *pglobal = ctx->back_ptr;
struct _1905_map_device *curr_dev = NULL;
struct bss_info_db *curr_bss = NULL;
struct cent_steer_cands *cand = NULL;
int8_t steer_edge_dg = 0;
uint8_t enqueue = 0;
s8 steer_edge_multiap = 0;

	if (!cli){
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"Client is null");
		return;
	}

	curr_dev = topo_srv_get_1905_by_bssid(ctx,cli->bssid);

	if (!curr_dev){
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"curr_dev is null");
		return;
	}

	curr_bss = topo_srv_get_bss_by_bssid(ctx,curr_dev,cli->bssid);

	if (!curr_bss){
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"curr_bss is null");
		return;
	}

	steer_edge_dg = pglobal->dev.cli_steer_params.RSSISteeringEdge_DG + ap_est_get_noise_offset_by_1905_radio(pglobal,curr_bss->radio);

	/*Skip this client as it is already present in client steering list*/
	if(get_client_from_cent_steer_cand_list(ctx, cli)){
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client already present in the cent str queue");
		return;
	}
#ifdef SUPPORT_MULTI_AP

if (topo_srv_get_rssi_th_by_policy_1905_radio(ctx, curr_bss->radio, &steer_edge_multiap) != 0)
	/* Get it from Steer Params */
	steer_edge_multiap = mapd_get_ap_steer_th_1905_radio(pglobal, curr_dev ,curr_bss->radio);
#endif

	if (cli->cli_steer_state != CLI_STATE_IDLE) {
		/* Only one at a time */
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"Parallel Steering is not allowed:"
						"Connected to me but not IDLE state, STATE: %d", cli->cli_steer_method);
		return;
	}

	if(!((cli->cli_steer_state == CLI_STATE_IDLE) &&
		(steer_action_is_steer_allowed(pglobal, cli->client_id))))
		return;



if ((steer_edge_multiap >= steer_edge_dg) &&
	(ap_roam_algo_nol_map_cand_cent_str(pglobal, cli, steer_edge_multiap))) {
	 /* Attempt MAP steering first */
	cli->cli_steer_method = NOL_MULTIAP;
	 enqueue = 1;
} else if (ap_roam_algo_dg_cand_cent_str(pglobal, cli)){
	cli->cli_steer_method = (cli->activity_state ? ACTIVE_STANDALONE_DG : IDLE_STANDALONE_DG);
	enqueue = 1;
}
	/*Add the client db to the ctx->cent_steer_cands_head.*/
if(enqueue) {
	cand = os_zalloc(sizeof(*cand));
	cand->steer_cand = cli;
	os_memcpy(cand->steer_cand_home_bssid, cli->bssid, ETH_ALEN);
	STAILQ_INSERT_TAIL(&ctx->cent_steer_cands_head, cand, next_cand);
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client (" MACSTR ") on agent(" MACSTR ")  is a %s candidate",
			MAC2STR(cli->mac_addr), MAC2STR(curr_dev->_1905_info.al_mac_addr), str_method_str(cli->cli_steer_method));
	}
else {
		if(cli->DG_fail_2gOL ) {
			if(curr_dev->device_role == DEVICE_ROLE_AGENT)
				insert_entry_fail_candidate_list(cli,ctx);
			cli->DG_fail_2gOL = 0;
		}
	}

	return;
}


static void ap_roam_ol_steer_cand_selection_cent_str(struct own_1905_device *ctx, struct _1905_map_device *curr_dev)
{
	struct mapd_global *pglobal = ctx->back_ptr;
	struct associated_clients *client = NULL, *tmp_client = NULL;
	struct client *arr_cand_list[MAX_STA_SEEN];
	uint32_t total_cand = 0;
	struct cent_steer_cands *cand = NULL;


	if (!curr_dev) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"curr_dev is NULL ");
		return;
	}


	SLIST_FOREACH_SAFE(client, &(curr_dev->assoc_clients), next_client, tmp_client) {
		struct client * cli = client_db_get_client_from_sta_mac(ctx->back_ptr,client->client_addr);

		if(client->is_bh_link)
			continue;

		if(!cli) {
			mapd_printf(MSG_ERROR,CENT_STEER_PREX"client db entry for "MACSTR"not found", MAC2STR(client->client_addr));
			continue;
		}

		if (cli->cli_steer_state != CLI_STATE_IDLE) {
			/* Only one at a time */
			mapd_printf(MSG_ERROR,CENT_STEER_PREX"Parallel Steering is not allowed:"
							"Connected to me but not IDLE state, STATE: %d", cli->cli_steer_method);
			continue;
		}


		if(!((cli->cli_steer_state == CLI_STATE_IDLE) &&
			(steer_action_is_steer_allowed(pglobal, cli->client_id))))
			continue;



		if (ap_roam_algo_offloading_cand_cent_str(ctx->back_ptr,cli)) {
			/*Skip client if it is already part of cand list*/
			if(get_client_from_cent_steer_cand_list(ctx,cli))
				continue;
			cli->cli_steer_method = OFFLOADING;
			arr_cand_list[total_cand++] = cli;
		}
	}

	/* Sort global list*/
	if (total_cand > 0) {
		/* The comparator function takes care of internal prioritization and
		   conflict resolution
		*/
		qsort(&arr_cand_list[0], total_cand, sizeof(struct client *), compare_cent_str_ol_ug);
		/*Add client to str cand list*/
		cand = os_zalloc(sizeof(*cand));
		cand->steer_cand = arr_cand_list[0];
		os_memcpy(cand->steer_cand_home_bssid, arr_cand_list[0]->bssid, ETH_ALEN);
		STAILQ_INSERT_TAIL(&ctx->cent_steer_cands_head, cand, next_cand);
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"client (" MACSTR ") on agent(" MACSTR ") is a %s candidate",
			MAC2STR(cand->steer_cand->mac_addr), MAC2STR(curr_dev->_1905_info.al_mac_addr),
			str_method_str(cand->steer_cand->cli_steer_method));
		return;
	}
	else {
		return;
	}

}


static void ap_roam_ug_steer_cand_selection_cent_str(struct own_1905_device *ctx, struct _1905_map_device *curr_dev)
{
	struct mapd_global *pglobal = ctx->back_ptr;
	struct associated_clients *client = NULL, *tmp_client = NULL;
	struct client *arr_cand_list[MAX_STA_SEEN];
	uint32_t total_cand = 0;
	struct cent_steer_cands *cand = NULL;

	if (!curr_dev) {
		mapd_printf(MSG_ERROR, CENT_STEER_PREX"curr_dev is NULL ");
		return;
	}

	SLIST_FOREACH_SAFE(client, &(curr_dev->assoc_clients), next_client, tmp_client) {
		struct client * cli = client_db_get_client_from_sta_mac(ctx->back_ptr,client->client_addr);

		if(client->is_bh_link)
			continue;

		if(!cli) {
			mapd_printf(MSG_ERROR,CENT_STEER_PREX"client db entry for "MACSTR"not found", MAC2STR(client->client_addr));
			continue;
		}

		if (cli->cli_steer_state != CLI_STATE_IDLE) {
			/* Only one at a time */
			mapd_printf(MSG_ERROR,CENT_STEER_PREX"Parallel Steering is not allowed:"
							"Connected to me but not IDLE state, STATE: %d", cli->cli_steer_method);
			continue;
		}

		if(!((cli->cli_steer_state == CLI_STATE_IDLE) &&
			(steer_action_is_steer_allowed(pglobal, cli->client_id))))
			continue;

		if (ap_roam_algo_ug_cand_cent_str(ctx->back_ptr,cli)){
			/*Skip client if it is already part of cand list*/
			if(get_client_from_cent_steer_cand_list(ctx,cli)){
				continue;
			}
			cli->cli_steer_method = (cli->activity_state ? ACTIVE_STANDALONE_UG : IDLE_STANDALONE_UG);
			arr_cand_list[total_cand++] = cli;
		}
	}
	/* Sort global list*/
	if (total_cand > 0) {
		/* The comparator function takes care of internal prioritization and
		   conflict resolution
		*/
		qsort(&arr_cand_list[0], total_cand, sizeof(struct client *), compare_cent_str_ol_ug);
		/*Add client to str cand list*/
		cand = os_zalloc(sizeof(*cand));
		cand->steer_cand = arr_cand_list[0];
		os_memcpy(cand->steer_cand_home_bssid, arr_cand_list[0]->bssid, ETH_ALEN);
		STAILQ_INSERT_TAIL(&ctx->cent_steer_cands_head, cand, next_cand);
		mapd_printf(MSG_ERROR, CENT_STEER_PREX"client (" MACSTR ") on agent(" MACSTR ") is a %s candidate",
			MAC2STR(cand->steer_cand->mac_addr), MAC2STR(curr_dev->_1905_info.al_mac_addr),
			str_method_str(cand->steer_cand->cli_steer_method));
		return;
	} else {
		return;
	}


}

static void ap_roam_phy_based_steer_cand_selection_cent_str(struct own_1905_device *ctx, struct _1905_map_device *curr_dev)
{
	struct mapd_global *pglobal = ctx->back_ptr;
	struct associated_clients *client = NULL, *tmp_client = NULL;
	struct client *arr_cand_list[MAX_STA_SEEN];
	uint32_t total_cand = 0;
	struct cent_steer_cands *cand = NULL;


	if (!curr_dev) {
		mapd_printf(MSG_ERROR,CENT_STEER_PREX"curr_dev is NULL ");
		return;
	}


	SLIST_FOREACH_SAFE(client, &(curr_dev->assoc_clients), next_client, tmp_client) {
		struct client * cli = client_db_get_client_from_sta_mac(ctx->back_ptr,client->client_addr);

			if(!cli) {
				mapd_printf(MSG_ERROR,CENT_STEER_PREX"client db entry for "MACSTR" not found", MAC2STR(client->client_addr));
				continue;
			}

			if(client->is_bh_link)
				continue;


			if (cli->cli_steer_state != CLI_STATE_IDLE) {
				/* Only one at a time */
				mapd_printf(MSG_ERROR,CENT_STEER_PREX"Parallel Steering is not allowed:"
								"Connected to me but not IDLE state, STATE: %d", cli->cli_steer_method);
				continue;
			}

			if(!((cli->cli_steer_state == CLI_STATE_IDLE) &&
				(steer_action_is_steer_allowed(pglobal, cli->client_id))))
				continue;



			if(ctx->cli_steer_params.PHYBasedSelection) {
				int ret;
				ret = ap_roam_algo_post_assoc_phy_based_sel_cent_str(ctx->back_ptr, cli);
				if (ret != NONE ){
					if(get_client_from_cent_steer_cand_list(ctx,cli))
						continue;
					cli->cli_steer_method = ret;
					arr_cand_list[total_cand++] = cli;
				}
			}
	}

	/* Sort global list*/
	if (total_cand > 0) {
		/* The comparator function takes care of internal prioritization and
		   conflict resolution
		*/
		qsort(&arr_cand_list[0], total_cand, sizeof(struct client *), compare_cent_str_ol_ug);
		/*Add client to str cand list*/
		cand = os_zalloc(sizeof(*cand));
		cand->steer_cand = arr_cand_list[0];
		os_memcpy(cand->steer_cand_home_bssid, arr_cand_list[0]->bssid, ETH_ALEN);
		STAILQ_INSERT_TAIL(&ctx->cent_steer_cands_head, cand, next_cand);
		mapd_printf(MSG_ERROR, CENT_STEER_PREX"client (" MACSTR ") on agent(" MACSTR ") is a %s candidate",
			MAC2STR(cand->steer_cand->mac_addr), MAC2STR(curr_dev->_1905_info.al_mac_addr),
			str_method_str(cand->steer_cand->cli_steer_method));
		return;
	} else {
		return;
	}


}


void cent_str_rr_steer_cand_selection(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = eloop_ctx;
	int i = 0;

	if (global->dev.device_role == DEVICE_ROLE_AGENT)
		return;

	/* controller and auto role*/
	if (global->dev.cent_str_en)
		eloop_register_timeout(CENT_STR_1_MIN, 0,
			cent_str_rr_steer_cand_selection, global, NULL);

	if (global->dev.device_role != DEVICE_ROLE_CONTROLLER)
		return;

	/*Select agent in round robin fashion*/
	global->dev.p_cent_str_curr_1905_rr = topo_srv_get_next_1905_device(&global->dev,global->dev.p_cent_str_curr_1905_rr);

	if(global->dev.p_cent_str_curr_1905_rr == NULL)
		global->dev.p_cent_str_curr_1905_rr = topo_srv_get_next_1905_device(&global->dev,global->dev.p_cent_str_curr_1905_rr);


	mapd_printf(MSG_ERROR, CENT_STEER_PREX"RR almac:"MACSTR,MAC2STR(global->dev.p_cent_str_curr_1905_rr->_1905_info.al_mac_addr));
	while (i < global->dev.cli_steer_params.cent_str_max_ug_steer_cand) {
	ap_roam_ug_steer_cand_selection_cent_str(&global->dev,global->dev.p_cent_str_curr_1905_rr);
		i++;
	}
	ap_roam_phy_based_steer_cand_selection_cent_str(&global->dev,global->dev.p_cent_str_curr_1905_rr);

}


Boolean is_chplan_netopt_ongoing(struct mapd_global *global) {

	struct own_1905_device *dev = &global->dev;
	struct os_reltime rem_time;
	u32 net_opt_trigger_rem_time = 0;

	if(eloop_is_timeout_registered(retrigger_ch_planning_post_radar,(void *)global , NULL)
		|| (dev->Restart_ch_planning_radar)){
		debug(CENT_STEER_PREX"retrigger_ch_planning_post_radar Restart_ch_planning_radar:%d", dev->Restart_ch_planning_radar);
		return TRUE;
	}

	os_memset((u8 *)&rem_time, 0, sizeof(struct os_reltime));
	eloop_get_remaining_timeout(&rem_time, trigger_net_opt,
					(void *)dev, NULL);

	net_opt_trigger_rem_time = rem_time.sec ? rem_time.sec : ((rem_time.usec ? 1 : 0));
	/*If a DFS channel is selected after channel planning the netopt will be triggered
	  for a non-weather channel  in (65 +post_cac_trigger_time) seconds,but for weather channel the
	  netopt will be triggered in (605 + post_cac_trigger_time) seconds which is more than 10 mins.
	  So in the case of DFS weather channel we start to disallow steering when netopt timer has (65 +post_cac_trigger_time) seconds
	  left to expire
	*/
	if((eloop_is_timeout_registered(trigger_net_opt,(void *)dev , NULL)
			&& ((net_opt_trigger_rem_time > 0 && net_opt_trigger_rem_time <= dev->network_optimization.post_cac_trigger_time + 65)))
			|| (dev->network_optimization.network_opt_state != NETOPT_STATE_IDLE)
			|| (dev->network_optimization.NetOptReason != REASON_NOT_REQUIRED )){
		debug(CENT_STEER_PREX"trigger_net_opt,NetOptReason:%d",dev->network_optimization.NetOptReason);
		return TRUE;
	}
#ifdef MAP_R2
	if ((dev->ch_planning_R2.ch_plan_enable == TRUE)
				&&	(dev->ch_planning_R2.ch_plan_state > CHPLAN_STATE_MONITOR)){
		err(CENT_STEER_PREX"R2 ch_plan_state:%d",dev->ch_planning_R2.ch_plan_state);
		return TRUE;
	}
#endif
	if((dev->ch_planning.ch_planning_enabled == TRUE)
				&& (dev->ch_planning.ch_planning_state != CHANNEL_PLANNING_IDLE)){
		debug(CENT_STEER_PREX"R1 ch_plan_state:%d",dev->ch_planning.ch_planning_state);
		return TRUE;
	}

	return FALSE;


}

void steer_handle_chan_plan_net_opt_trigger(struct mapd_global *global){
	struct cent_steer_cands *cand = NULL, *tmp_cand = NULL;
	struct own_1905_device *ctx = &global->dev;


	if(!STAILQ_EMPTY(&ctx->cent_steer_cands_head) && is_chplan_netopt_ongoing(global)) {
		STAILQ_FOREACH_SAFE(cand, &ctx->cent_steer_cands_head, next_cand, tmp_cand) {
			struct client *cli = cand->steer_cand;


			if(cli->cli_steer_state == CLI_STATE_IDLE){
				cand->steer_cand = NULL;
				STAILQ_REMOVE(&(ctx->cent_steer_cands_head), cand, cent_steer_cands, next_cand);
				os_free(cand);
				if(STAILQ_EMPTY(&ctx->cent_steer_cands_head))
					break;
			} else {
				/*Client should be in decision state i.e channel measurement ongoing*/
				steer_fsm_trigger(global, cli->client_id, CHAN_MEASUREMENT_DISALLOWED, NULL);

			}

		}

	}
	if(global->dev.device_role == DEVICE_ROLE_CONTROLLER && is_chplan_netopt_ongoing(global)) {
		remove_all_fail_candidate_list(global);
	}
}


Boolean cent_str_check_cu_chan_list(struct own_1905_device * ctx, u8 channel){
	Boolean ret = FALSE;
	struct cent_str_cu_mon_ch_info *ch_info	= NULL, *tmp_ch_info = NULL;

	if(!ctx)
		return ret;

	SLIST_FOREACH_SAFE(ch_info,
			&(ctx->cent_str_cu_mon_ch_list), next_mon_ch, tmp_ch_info) {

		if(channel == ch_info->channel_num){
			ret = TRUE;
			break;
		}
	}

	return ret;
}

struct cent_str_cu_mon_ch_info * cent_str_get_cu_chan_list(
	struct own_1905_device * ctx, struct radio_info_db *radio)
{
	struct cent_str_cu_mon_ch_info *ch_info	= NULL, *tmp_ch_info = NULL;

	if(!ctx)
		return NULL;
	if(SLIST_EMPTY(&(ctx->cent_str_cu_mon_ch_list))){
		return NULL;	
	}

	SLIST_FOREACH_SAFE(ch_info,
			&(ctx->cent_str_cu_mon_ch_list), next_mon_ch, tmp_ch_info) {

		if(radio == ch_info->radio){
			return ch_info;
		}
	}

	return NULL;
}

void cent_str_cu_monitor_prohibit_timeout(void *eloop_ctx, void *timeout_ctx) {
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct own_1905_device *dev = (struct own_1905_device *)&global->dev;
	struct cent_str_cu_mon_ch_info *ch_info =  (struct cent_str_cu_mon_ch_info *)timeout_ctx;
	struct cent_str_cu_mon_ch_info *tmp_ch_info = NULL, *tmp_tmp_ch_info = NULL;
	u8 valid_cu_mon_entry = 0;

	if(ch_info)
		debug(CENT_STEER_PREX"CU monitor prohibit timeout for channel:%d",ch_info->channel_num);

	if(ch_info){
		SLIST_FOREACH_SAFE(tmp_ch_info,
			&(dev->cent_str_cu_mon_ch_list), next_mon_ch, tmp_tmp_ch_info) {
			if(tmp_ch_info->radio == ch_info->radio && tmp_ch_info->channel_num== ch_info->channel_num){
				valid_cu_mon_entry = 1;
				break;
			}

		}
		if(!valid_cu_mon_entry) {
			err(CENT_STEER_PREX"CU monitor prohibit timeout for channel:%d does not in monitor list",ch_info->channel_num);
			return;
		}
		SLIST_REMOVE(&(dev->cent_str_cu_mon_ch_list),
					ch_info,
					cent_str_cu_mon_ch_info,
					next_mon_ch);
		os_free(ch_info);
	}

	return;
}


void cent_str_cu_mon_add_chan_list(struct own_1905_device * ctx, struct radio_info_db *radio){

	struct cent_str_cu_mon_ch_info *new_ch_info, *trav_ch_info, *prev_ch_info, *tmp_trav_ch_info = NULL;

	new_ch_info = trav_ch_info = prev_ch_info = NULL;

	if(!radio)
		return;


	new_ch_info = os_zalloc(sizeof(struct cent_str_cu_mon_ch_info));

	if (new_ch_info == NULL) {
		err(CENT_STEER_PREX"alloc memory fail");
		assert(0);
		return ;
	}
	new_ch_info->channel_num = radio->channel[0];
	new_ch_info->radio = radio;

	if(SLIST_EMPTY(&(ctx->cent_str_cu_mon_ch_list))){
		SLIST_INSERT_HEAD(&(ctx->cent_str_cu_mon_ch_list),
			new_ch_info, next_mon_ch);
	} else {
		/*Add to tail*/
		SLIST_FOREACH_SAFE(trav_ch_info,
				&(ctx->cent_str_cu_mon_ch_list), next_mon_ch, tmp_trav_ch_info) {
			prev_ch_info = trav_ch_info;
		}
		if(prev_ch_info != NULL){
			SLIST_INSERT_AFTER(prev_ch_info,
				new_ch_info,
					next_mon_ch);
		}
	}

	/*start timer for  this channel's monitor*/
	eloop_register_timeout(ctx->cli_steer_params.cent_str_cu_mon_time, 0,
		cent_str_cu_monitor_timeout, ctx->back_ptr, radio);


}

void cent_str_cu_mon_remove_chan_list(
	struct own_1905_device *own_dev)
{
	struct cent_str_cu_mon_ch_info *ch_info = NULL,*temp_ch_info = NULL, *t_ch_info = NULL;

	SLIST_FOREACH_SAFE(ch_info,
			&(own_dev->cent_str_cu_mon_ch_list), next_mon_ch, t_ch_info) {


	eloop_cancel_timeout(cent_str_cu_monitor_timeout, own_dev->back_ptr, ch_info->radio);

	if (ch_info
		&& eloop_is_timeout_registered(
		cent_str_cu_monitor_prohibit_timeout,(void *)own_dev->back_ptr , ch_info))
		eloop_cancel_timeout(cent_str_cu_monitor_prohibit_timeout, own_dev->back_ptr, ch_info);

	ch_info->radio->cent_str_avg_cu_monitor = 0;
	ch_info->radio->cent_str_count_cu_util = 0;
	temp_ch_info = SLIST_NEXT(ch_info, next_mon_ch);
	SLIST_REMOVE(&(own_dev->cent_str_cu_mon_ch_list),
		ch_info,
		cent_str_cu_mon_ch_info,
		next_mon_ch);
		os_free(ch_info);
		ch_info = temp_ch_info;
	}

	return;

}



void cent_str_cu_monitor_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct own_1905_device *dev = (struct own_1905_device *)&global->dev;
	struct radio_info_db *radio = (struct radio_info_db *)timeout_ctx;
	struct cent_str_cu_mon_ch_info *ch_info = NULL, *tmp_ch_info = NULL;
	struct _1905_map_device	*curr_dev = NULL;
	struct radio_info_db *curr_radio = NULL;
	int avg_cu = 0;
	u8 valid_cu_mon_entry = 0, ol_th = 0;



	/*Get the entry from cent_str_cu_mon_ch_list */
	SLIST_FOREACH_SAFE(ch_info,
				&(dev->cent_str_cu_mon_ch_list), next_mon_ch, tmp_ch_info) {
			if(ch_info->radio == radio){
				valid_cu_mon_entry = 1;
				break;
			}

	}

	if(!valid_cu_mon_entry){
		err(CENT_STEER_PREX"CU Montior timeout expired but the radio not part of CU monitor list");
		return;
	}


	if(ch_info){

		ol_th = chan_mon_get_ol_th_by_chan(global,ch_info->channel_num);


		/*Calculate the average cu over monitor time */
		if (radio->cent_str_count_cu_util > 0) {
			avg_cu = radio->cent_str_avg_cu_monitor;
			avg_cu /= radio->cent_str_count_cu_util;
		} else
			avg_cu = 0;

		debug(CENT_STEER_PREX"CU Sum:%d Avg CU:%d for %d samples", radio->cent_str_avg_cu_monitor, avg_cu, radio->cent_str_count_cu_util);
		if(avg_cu > ol_th){
			/*Trigger OL steering on all devices having a  radio configured to same channel*/
			curr_dev = topo_srv_get_1905_device(&global->dev,NULL);
			while(curr_dev) {
				curr_radio = topo_srv_get_radio_by_channel(curr_dev, radio->channel[0]);
				if(curr_radio) {
					int i = 0;

					while(i < dev->cli_steer_params.cent_str_max_ol_steer_cand){
						ap_roam_ol_steer_cand_selection_cent_str(dev,curr_dev);
						i++;
					}


				}
				curr_dev = topo_srv_get_next_1905_device(&global->dev, curr_dev);
			}


		}
		radio->cent_str_avg_cu_monitor = 0;
		radio->cent_str_count_cu_util = 0;


		err(CENT_STEER_PREX"cu mon chan:%d", radio->channel[0]);
		eloop_register_timeout(dev->cli_steer_params.cent_str_cu_mon_prohibit_time, 0,
			cent_str_cu_monitor_prohibit_timeout, global, ch_info);


	}



}





void cent_str_cu_monitor(
	struct own_1905_device * ctx,
	struct bss_info_db *bss)
{
	u8 ol_th = 0,update_cu = 0;
	uint32_t ch_util = 0;
	u8 is_timer_ongoing = 0;
	struct radio_info_db *radio = NULL;
	struct cent_str_cu_mon_ch_info *ch_info = NULL;


	if(!bss)
		return;

	if(!bss->radio)
		return;

	radio = bss->radio;

	/*Check is CU mon prohibited for this radio*/
	ch_info = cent_str_get_cu_chan_list(ctx,radio);

	if(ch_info
		&& eloop_is_timeout_registered(
			cent_str_cu_monitor_prohibit_timeout,(void *)ctx->back_ptr , ch_info)){
		debug(CENT_STEER_PREX"CU monitor prohibited for this radio %d",ch_info->channel_num);
		return;
	}

	ol_th = chan_mon_get_ol_th_by_chan(ctx->back_ptr,radio->channel[0]);

	ch_util = bss->ch_util *100;
	ch_util /= 255;


	/*Check threshold*/
	if(ch_util > ol_th) {
		update_cu = 1;
	}


	if(eloop_is_timeout_registered(cent_str_cu_monitor_timeout,(void *)ctx->back_ptr , radio))
		is_timer_ongoing = 1;


	if(update_cu && !is_timer_ongoing && !cent_str_check_cu_chan_list(ctx,radio->channel[0])) {
		if(is_chplan_netopt_ongoing(ctx->back_ptr))
			return;
		/* Start cu monitor on a new channel */
		debug(CENT_STEER_PREX"Start CU Monitor for channel %d------>",radio->channel[0]);
		cent_str_cu_mon_add_chan_list(ctx, radio);

		/* Init cu avg stats */
		radio->cent_str_avg_cu_monitor = ch_util;
		radio->cent_str_count_cu_util++;

	} else if(is_timer_ongoing){
		debug(CENT_STEER_PREX"Start Update CU mon avg  for channel %d--->",radio->channel[0]);
		radio->cent_str_avg_cu_monitor += ch_util;
		radio->cent_str_count_cu_util++;
	}

	return;
}


#endif /*CENT_STR*/
#endif /*SUPPORT_MULTI_AP*/


