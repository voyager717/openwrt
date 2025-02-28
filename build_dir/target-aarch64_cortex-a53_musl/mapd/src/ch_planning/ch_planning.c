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
 *  Channel planning *
 *  Abstract:
 *  Channel Planning
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Hasan 2018/05/02    First implementation channel planning * */

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
extern u8 ZERO_MAC_ADDR[ETH_ALEN];

u8 grp_bw_160[MAX_BW_160_BLOCK][8] = {
	{36, 40, 44, 48, 52, 56, 60, 64},
	{100, 104, 108, 112, 116, 120, 124, 128},
	 {149, 153, 157, 161, 165, 169, 173, 177}
};
u8 grp_bw_80[MAX_BW_80_BLOCK][4] = {
	{36, 40, 44, 48},
	{52, 56, 60, 64},
	{100, 104, 108, 112},
	{116, 120, 124, 128},
	{132, 136, 140, 144},
	{149, 153, 157, 161},
	{165, 169, 173, 177}
};
u8 grp_bw_40[MAX_BW_40_BLOCK][2] = {
	{36, 40},
	{44, 48},
	{52, 56},
	{60, 64},
	{100, 104},
	{108, 112},
	{116, 120},
	{124, 128},
	{132, 136},
	{140, 144},
	{149, 153},
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

u8 RadarCh[MAX_DFS_CHANNEL] = {52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144};
struct vht_ch_layout {
       u8 ch_low_bnd;
       u8 ch_up_bnd;
       u8 cent_freq_idx;
};

static struct vht_ch_layout vht_ch_80M[] = {
	{36, 48, 42},
	{52, 64, 58},
	{100, 112, 106},
	{116, 128, 122},
	{132, 144, 138},
	{149, 161, 155},
	{165, 177, 171},
	{0, 0, 0},
};

static struct vht_ch_layout vht_ch_40M[] = {
	{36, 40, 38},
	{44, 48, 46},
	{52, 56, 54},
	{60, 64, 62},
	{100, 104, 102},
	{108, 112, 110},
	{116, 120, 118},
	{124, 128, 126},
	{132, 136, 134},
	{140, 144, 142},
	{149, 153, 151},
	{157, 161, 159},
	{165, 169, 167},
	{173, 177, 175},
	{0, 0, 0},
};
#ifdef MAP_6E_SUPPORT
static struct vht_ch_layout SixE_ch_40M[] = {
	{1, 5, 3},
	{9, 13, 11},
	{17, 21, 19},
	{25, 29, 27},
	{33, 37, 35},
	{41, 45, 43},
	{49, 53, 51},
	{57, 61, 59},
	{65, 69, 67},
	{73, 77, 75},
	{81, 85, 83},
	{89, 93, 91},
	{97, 101, 99},
	{105, 109, 107},
	{113, 117, 115},
	{121, 125, 123},
	{129, 133, 131},
	{137, 141, 139},
	{145, 149, 147},
	{153, 157, 155},
	{161, 165, 163},
	{169, 173, 171},
	{177, 181, 179},
	{185, 189, 187},
	{193, 197, 195},
	{201, 205, 203},
	{209, 213, 211},
	{217, 221, 219},
	{225, 229, 227},
	{0, 0, 0},
};
#endif
#if 0
#ifdef MAP_320BW
static struct vht_ch_layout eht_ch_320M_6G[] = {
	{1, 61, 31},
	{33, 93, 63},
	{65, 125, 95},
	{97, 157, 127},
	{129, 189, 159},
	{161, 221, 191},
	{0, 0, 0},
};
#endif
#endif

unsigned char get_primary_channel(unsigned char channel)
{
       int i, ch_size;
       struct vht_ch_layout *vht;

       ch_size = sizeof(vht_ch_80M) / sizeof(struct vht_ch_layout);
       for (i = 0; i < ch_size; i++) {
               vht = &vht_ch_80M[i];
               if (vht->cent_freq_idx == channel) {
                       return vht->ch_low_bnd;
               }
       }
       return channel;
}

unsigned int is_valid_primary_ch_80M_160M(unsigned char ch, unsigned char center_ch, unsigned char op)
{
	int offset = 0;

	offset = ch - center_ch;
	if ((op == 128)
#ifdef MAP_6E_SUPPORT
		|| (op == 133)
#endif
		) {
		if ((abs(offset) == 6) || (abs(offset) == 2))
			return 1;
		else
			return 0;
	} else if ((op == 129)
#ifdef MAP_6E_SUPPORT
		|| (op == 134)
#endif
		) {
		if ((abs(offset) == 14) || (abs(offset) == 10) ||
			(abs(offset) == 6) || (abs(offset) == 2)) {
			return 1;
		} else {
			return 0;
		}
	}
#ifdef MAP_320BW
	else if (op == 137) {
		if ((abs(offset) == 30) || (abs(offset) == 26) ||
				(abs(offset) == 22) || (abs(offset) == 18) || (abs(offset) == 14) ||
				(abs(offset) == 10) || (abs(offset) == 6) || (abs(offset) == 2)) {
			return 1;
		} else {
			return 0;
		}
	}
#endif

	return 0;
}

u8 is_mixed_network(struct own_1905_device *ctx, Boolean ignore_edcca)
{
	struct _1905_map_device *dev = NULL, *t_dev = NULL;

	if (ignore_edcca)
		return 1;//Hard coded as edcca value is coming 0 always right now
	else {
		SLIST_FOREACH_SAFE(dev, &ctx->_1905_dev_head, next_1905_device, t_dev) {
			if (!dev->in_network)
				continue;
			if(dev->vendor != VENDOR_MEDIATEK)
				return 1;
		}
	}
	return 0;
}

/**
* @brief Fn to get number of radios on an operating channel
* @param ch_distribution chanel_planning global struct
* @param channel taregt channel number
*/

unsigned char ch_planning_get_num_radio_on_operating_channel(
	struct ch_distribution_cb *ch_distribution,
	unsigned char channel)
{
	struct operating_ch_cb *operating_ch= NULL, *t_op_ch = NULL;
	SLIST_FOREACH_SAFE(operating_ch, &(ch_distribution->first_operating_ch), next_operating_ch, t_op_ch) {
		if (operating_ch->ch_num == channel)
		{
			return operating_ch->radio_count;
		}
	}
	return 0;
}

/**
* @brief Fn to get number of radios on an prefered channel
* @param ch_distribution chanel_planning global struct
* @param channel taregt channel number
*/
unsigned char ch_planning_get_num_radio_on_prefered_channel(
	struct own_1905_device *ctx,
	unsigned char channel)
{
	//err("%s\n", __FUNCTION__);
	struct ch_distribution_cb *ch_distribution = NULL;
	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct prefered_ch_cb *prefered_ch= NULL, *t_prefered_ch = NULL;

	if (channel > 14)
	{
		//err("ch_ditribution_5g\n");
		ch_distribution = &ch_planning->ch_ditribution_5g;
	} else {
		//err("ch_ditribution_2g\n");
		ch_distribution = &ch_planning->ch_ditribution_2g;
	}
	SLIST_FOREACH_SAFE(prefered_ch, &(ch_distribution->first_prefered_ch), next_prefered_ch, t_prefered_ch) {
		if (prefered_ch->ch_num == channel)
		{
			return prefered_ch->radio_count;
		}
	}
	return 0;
}

/**
* @brief Fn to get max bw of a entire network
* @param ch_distribution chanel_planning global struct
* @param channel taregt channel number
*/
unsigned char ch_planning_get_max_bw_in_network (
	struct own_1905_device *ctx, unsigned char channel){

	int maxbw = BW_20, bw = BW_20;
	struct _1905_map_device *_1905_dev = NULL, *t_1905_dev = NULL;

	SLIST_FOREACH_SAFE(_1905_dev, &ctx->_1905_dev_head, next_1905_device, t_1905_dev) {
			if (_1905_dev->in_network) {
			bw = ch_planning_get_dev_bw_from_channel(_1905_dev, channel);
			if (maxbw < bw)
				maxbw = bw;
			}
	}

	return maxbw;
}

u8 is_best_channel_is_present(struct prefer_info_db *prefer_info, u8 channel)
{
	struct prefer_info_db *prefer_info_tmp = prefer_info;
	int bw = BW_20;
	int i = 0;

	bw = chan_mon_get_bw_from_op_class(prefer_info_tmp->op_class);

	for (i = 0; i < prefer_info_tmp->ch_num; i++) {
		if (bw <= BW_40) {
			if (prefer_info_tmp->ch_list[i] == channel)
				return 1;
		} else {
			if (prefer_info_tmp->ch_list[i] ==
				ch_planning_get_centre_freq_ch(channel, prefer_info_tmp->op_class))
				return 1;
		}
	}

	return 0;
}

unsigned char ch_planning_get_max_bw_1905dev_prefer_channel(
	struct own_1905_device *ctx, struct radio_info_db *radio, struct _1905_map_device *_1905_dev,
	u8 channel)
{
	struct prefer_info_db *prefer_info = NULL;
	int max_bw = BW_20;

	if (!radio)
		return 0;

	if (radio->channel[0] > 14) {
		SLIST_FOREACH(prefer_info, &(radio->chan_preferance.prefer_info_head), prefer_info_entry) {
			if ((prefer_info->op_class == 129) && is_best_channel_is_present(prefer_info, channel)) {
				if (max_bw < BW_160)
					max_bw = BW_160;

			} else if ((prefer_info->op_class == 128) && is_best_channel_is_present(prefer_info, channel)) {
				if (max_bw < BW_80)
					max_bw = BW_80;
#ifdef MAP_320BW
			} else if ((prefer_info->op_class == 137) && is_best_channel_is_present(prefer_info, channel)) {
				if (max_bw < BW_320)
					max_bw = BW_320;
#endif
			} else if ((prefer_info->op_class == 115 || prefer_info->op_class == 118 ||
				prefer_info->op_class == 121 || prefer_info->op_class == 124 ||
				prefer_info->op_class == 125) && is_best_channel_is_present(prefer_info, channel)) {
				if (max_bw <= BW_20)
					max_bw = BW_20;

			} else if ((prefer_info->op_class == 116 || prefer_info->op_class == 117 ||
				prefer_info->op_class == 119 || prefer_info->op_class == 122 ||
				prefer_info->op_class == 126 || prefer_info->op_class == 120 ||
				prefer_info->op_class == 123) && is_best_channel_is_present(prefer_info, channel)) {
				if (max_bw < BW_40)
					max_bw = BW_40;

			}
		}
	} else {
		SLIST_FOREACH(prefer_info, &(radio->chan_preferance.prefer_info_head), prefer_info_entry) {
			if (prefer_info->op_class == 83 || prefer_info->op_class == 84) {
				if (max_bw < BW_40)
					max_bw = BW_40;

			} else if (prefer_info->op_class == 81) {
				if (max_bw <= BW_20)
					max_bw = BW_20;

			}
		}
	}

	return max_bw;
}


unsigned char ch_planning_get_max_bw_1905dev_prefer (
	struct own_1905_device *ctx, struct radio_info_db *radio, struct _1905_map_device *_1905_dev)
{
	struct prefer_info_db *prefer_info = NULL;
	int max_bw = BW_20;

	if (!radio)
		return 0;

	if (radio->channel[0] > 14) {
		SLIST_FOREACH(prefer_info, &(radio->chan_preferance.prefer_info_head), prefer_info_entry) {
			if (prefer_info->op_class == 129) {
				if (max_bw < BW_160) {
					max_bw = BW_160;
				}
#ifdef MAP_320BW
			} else if (prefer_info->op_class == 137) {
				if (max_bw < BW_320) {
					max_bw = BW_320;
				}
#endif
			} else if (prefer_info->op_class == 128) {
				if (max_bw < BW_80) {
					max_bw = BW_80;
				}
			} else if (prefer_info->op_class == 115 || prefer_info->op_class == 118 ||
				prefer_info->op_class == 121 || prefer_info->op_class == 124 ||
				prefer_info->op_class == 125) {
				if (max_bw <= BW_20) {
					max_bw = BW_20;
				}
			} else if (prefer_info->op_class == 116 || prefer_info->op_class == 117 ||
				prefer_info->op_class == 119 || prefer_info->op_class == 122 ||
				prefer_info->op_class == 126 || prefer_info->op_class == 120 ||
				prefer_info->op_class == 123) {
				if (max_bw < BW_40) {
					max_bw = BW_40;
				}
			}
		}
	} else {
		SLIST_FOREACH(prefer_info, &(radio->chan_preferance.prefer_info_head), prefer_info_entry) {
			if (prefer_info->op_class == 83 || prefer_info->op_class == 84) {
				if (max_bw < BW_40) {
					max_bw = BW_40;
				}
			} else if (prefer_info->op_class == 81) {
				if (max_bw <= BW_20) {
					max_bw = BW_20;
				}
			}
		}
	}

	return max_bw;
}


/**
* @brief Fn to insert a radio in operating channel list
* @param ch_distribution chanel_planning global struct
* @param channel taregt channel number
*/

void ch_planning_insert_into_ch_operating(
	struct own_1905_device *ctx,
	struct operating_ch_cb *operating_ch,
	struct ch_distribution_cb *ch_distribution)
{
	struct operating_ch_cb *operating_ch_temp = NULL, *t_op_ch_tmp = NULL;
	struct operating_ch_cb *previous_operating_ch= NULL;

	SLIST_FOREACH_SAFE(operating_ch_temp, &(ch_distribution->first_operating_ch), next_operating_ch, t_op_ch_tmp) {
		if (operating_ch_temp->radio_count > operating_ch->radio_count)
		{
			debug(CH_PLANING_PREX"Radio count on Channel %d is higher than on Channel %d\n",
				operating_ch_temp->ch_num, operating_ch->ch_num);
		} else {
			if (operating_ch_temp->radio_count <
				operating_ch->radio_count)
			{
				if (previous_operating_ch != NULL) {
					SLIST_INSERT_AFTER(previous_operating_ch,
						operating_ch,
						next_operating_ch);
					debug(CH_PLANING_PREX"Add after Channel number = %d\n",
					previous_operating_ch->ch_num);
				} else {
					debug(CH_PLANING_PREX"Insert into head\n");
					SLIST_INSERT_HEAD(
						&(ch_distribution->first_operating_ch),
						operating_ch,
						next_operating_ch);
				}
				break;
			}
		}
		previous_operating_ch = operating_ch_temp;
	}
	if (operating_ch_temp == NULL)
	{
		debug(CH_PLANING_PREX"Channel %d not inserted yet, insert in the tail\n",
			operating_ch->ch_num);
		if (previous_operating_ch != NULL) {
			debug(CH_PLANING_PREX"Insert after %d\n", previous_operating_ch->ch_num);
			SLIST_INSERT_AFTER(previous_operating_ch,
				operating_ch,
				next_operating_ch);
		} else {
			debug(CH_PLANING_PREX"Insert in the head\n");
			SLIST_INSERT_HEAD(
				&(ch_distribution->first_operating_ch),
				operating_ch,
				next_operating_ch);
		}
	}

}
#ifdef MAP_R2
void ch_planning_allow_ch_sync(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{
	struct radio_info_db *radio = NULL, *t_radio = NULL;
#ifndef MAP_6E_SUPPORT
	struct mapd_global *global = ctx->back_ptr;
#endif
	//struct _1905_map_device *_1905_device = topo_srv_get_1905_device(ctx, NULL);
	/*if any radio's bootup run is yet incomplete then wait before allowing sync*/
	uint8_t op_channel_idx;
	for (op_channel_idx = 0; op_channel_idx < MAX_NUM_OF_RADIO; op_channel_idx++) {
#ifndef MAP_6E_SUPPORT
		struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[op_channel_idx];
		if (radio_info->radio_idx == (uint8_t)-1)
			continue;
		if(radio_info->bootup_run != BOOTUP_SCAN_COMPLETED)
#else
		if (ctx->ch_planning_R2.bootup_scanstatus[op_channel_idx].bootup_run != BOOTUP_SCAN_COMPLETED &&
			ctx->ch_planning_R2.bootup_scanstatus[op_channel_idx].bootup_run != BOOTUP_SCAN_NOT_NEEDED)

#endif
			return;
	}

	if(ctx->ch_planning_R2.force_trigger == 1 ||
		ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_CH_CHANGE_TRIGGERED) {
		err(CH_PLANING_PREX"no need sync state for dev("MACSTR")", MAC2STR(dev->_1905_info.al_mac_addr));
		return;
	}

	/*Otherwise allow sync , it indicates that a new device has entered network*/
	err(CH_PLANING_PREX"r2_state(%d) sync trigger state for all radio on dev"MACSTR"",
		ctx->ch_planning_R2.ch_plan_state, MAC2STR(dev->_1905_info.al_mac_addr));
	SLIST_FOREACH_SAFE(radio, &(dev->first_radio), next_radio, t_radio) {
		radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
		err(CH_PLANING_PREX"set CHPLAN_STATE_CH_CHANGE_TRIGGERED to radio(%d)", radio->channel[0]);
	}
}
#endif
/**
* @brief Fn to select device for channel planning
* @param ctx own global structure
*/
struct _1905_map_device *ch_planning_get_target_dev(struct own_1905_device *ctx)
{
	struct _1905_map_device *dev = NULL, *t_dev = NULL;
	struct os_time current_time;
	struct mapd_global *mapd_global_ptr = ctx->back_ptr;
#ifdef MAP_R2
	struct radio_info_db *radio, *t_radio = NULL;
	signed int best_channel = 0;
	unsigned char ch_prefer_count = 0, channel_planning_req = FALSE;
	struct ch_prefer_lib *ch_prefer = os_zalloc(sizeof(struct ch_prefer_lib) * 3);

	if (!ch_prefer) {
		err(CH_PLANING_PREX"alloc memory fail for ch_prefer");
		return NULL;
	}
#endif

	/* wait for 2 mins before starting channel planning */
	os_get_time(&current_time);
	SLIST_FOREACH_SAFE(dev, &(ctx->_1905_dev_head), next_1905_device, t_dev) {

#ifdef SINGLE_BAND_SUPPORT
		if (SLIST_EMPTY(&dev->first_radio)) {
			if (!dev->channel_planning_completed) {
				err(CH_PLANING_PREX"Skip dev("MACSTR") since it has no radio.", MAC2STR(dev->_1905_info.al_mac_addr));
				dev->channel_planning_completed = TRUE;
			}
			continue;
		}
#endif

		if (dev->in_network &&
			!dev->ch_preference_available)
		{
			map_1905_Send_Channel_Preference_Query_Message(mapd_global_ptr->_1905_ctrl,
				(char *)dev->_1905_info.al_mac_addr);
		}

#ifdef MAP_R2
		if (dev->in_network && dev->device_role == DEVICE_ROLE_AGENT) {
			SLIST_FOREACH_SAFE(radio, &(dev->first_radio), next_radio, t_radio) {
				if (radio->channel[0] == 0) {
					best_channel = find_controller_channel_from_agent_pref(ctx, radio);
					err(CH_PLANING_PREX"Need to sync channel on agent.");
					err(CH_PLANING_PREX"Send mandatory CSR for Ch(%d)", best_channel);
					if (best_channel > 0) {
						/*! prepare channel selection request data for current radio*/
						ch_planning_ch_selection_prefer_data(ctx, best_channel,
							radio, &ch_prefer[ch_prefer_count]);
						ch_prefer_count++;
						channel_planning_req = TRUE;
					}
				}
			}
			if (channel_planning_req) {
				ch_planning_send_select(ctx, dev, ch_prefer_count, ch_prefer);
				channel_planning_req = FALSE;
				continue;
			}
		}
#endif

		if (dev->in_network &&
			dev->ch_preference_available &&
			dev->radio_mapping_completed &&
			((current_time.sec - dev->first_seen.sec)
			> ctx->channel_planning_initial_timeout) &&
			!dev->channel_planning_completed)
		{
#ifdef MAP_R2
			if(ctx->ch_planning_R2.ch_plan_enable == TRUE) {
				ch_planning_allow_ch_sync(ctx, dev);
			}
			if (ch_prefer)
				os_free(ch_prefer);
#endif
			return dev;
		}
	}

#ifdef MAP_R2
	if (ch_prefer)
		os_free(ch_prefer);
#endif

	return NULL;
}
#ifdef MAP_6E_SUPPORT
unsigned char ch_planning_get_centre_freq_ch_by_band(unsigned char channel, unsigned char band, unsigned char bw)
{
	if (bw == BW_160) {
		if (IS_MAP_CH_5G(channel) && IS_BAND_5G(band)) {
			if (channel >= 36 && channel <= 64)
				return 50;
			else if (channel >= 100 && channel <= 128)
				return 114;
			else if (channel >= 149 && channel <= 177)
				return 163;
		} else if (IS_MAP_CH_6G(channel) && IS_BAND_6G(band)) {
			if (channel >= 1 && channel <= 29)
				return 15;
			else if (channel >= 33 && channel <= 61)
				return 47;
			else if (channel >= 65 && channel <= 93)
				return 79;
			else if (channel >= 97 && channel <= 125)
				return 111;
			else if (channel >= 129 && channel <= 157)
				return 143;
			else if (channel >= 161 && channel <= 189)
				return 175;
			else if (channel >= 193 && channel <= 221)
				return 207;
		}
	} else if (bw == BW_80) {
		if (IS_MAP_CH_5G(channel) && IS_BAND_5G(band)) {
			if (channel >= 36 && channel <= 48)
				return 42;
			else if (channel >= 52 && channel <= 64)
				return 58;
			else if (channel >= 100 && channel <= 112)
				return 106;
			else if (channel >= 116 && channel <= 128)
				return 122;
			else if (channel >= 132 && channel <= 144)
				return 138;
			else if (channel >= 149 && channel <= 161)
				return 155;
			else if (channel >= 165 && channel <= 177)
				return 171;
		} else if (IS_MAP_CH_6G(channel) && IS_BAND_6G(band)) {
			if (channel > 1 && channel <= 13)
				return 7;
			else if (channel >= 17 && channel <= 29)
				return 23;
			else if (channel >= 33 && channel <= 45)
				return 39;
			else if (channel >= 49 && channel <= 61)
				return 55;
			else if (channel >= 65 && channel <= 77)
				return 71;
			else if (channel >= 81 && channel <= 93)
				return 87;
			else if (channel >= 97 && channel <= 109)
				return 103;
			else if (channel >= 113 && channel <= 125)
				return 119;
			else if (channel >= 129 && channel <= 141)
				return 135;
			else if (channel >= 145 && channel <= 157)
				return 151;
			else if (channel >= 161 && channel <= 173)
				return 167;
			else if (channel >= 177 && channel <= 189)
				return 183;
			else if (channel >= 193 && channel <= 205)
				return 199;
			else if (channel >= 209 && channel <= 221)
				return 215;
		}
	}
	return 0;
}
#endif
unsigned char ch_planning_get_centre_freq_ch_by_bw(unsigned char channel, unsigned char bw)
{
	if (bw == 3) {
		if (channel >= 36 && channel <= 64)
			return 50;
		else if (channel >= 100 && channel <= 128)
			return 114;
		else if (channel >= 149 && channel <= 177)
			return 163;
	} else if (bw == 2) {
		if (channel >= 36 && channel <= 48)
			return 42;
		else if (channel >= 52 && channel <= 64)
			return 58;
		else if (channel >= 100 && channel <= 112)
			return 106;
		else if (channel >= 116 && channel <= 128)
			return 122;
		else if (channel >= 132 && channel <= 144)
			return 138;
		else if (channel >= 149 && channel <= 161)
			return 155;
		else if (channel >= 165 && channel <= 177)
			return 171;
	}
#ifdef MAP_320BW
	else if (bw == BW_320) {
		if (channel >= 1 && channel <= 61) {
			if ((!HE_EXTCHA) || channel <= 29)
				return 31;
			else
				return 63;
		} else if (channel >= 33 && channel <= 93) {
			if (!HE_EXTCHA)
				return 63;
			else
				return 95;
		} else if (channel >= 65 && channel <= 125) {
			if (!HE_EXTCHA)
				return 95;
			else
				return 127;
		} else if (channel >= 97 && channel <= 157) {
			if (!HE_EXTCHA)
				return 127;
			else
				return 159;
		} else if (channel >= 129 && channel <= 189) {
			if (!HE_EXTCHA)
				return 159;
			else
				return 191;
		} else if (channel >= 161 && channel <= 221) {
			return 191;
		}
	}
#endif
	return 0;
}

unsigned char ch_planning_get_centre_freq_ch(unsigned char channel, unsigned char op)
{
	if (op == 129) {
		if (channel >= 36 && channel <= 64)
			return 50;
		else if (channel >= 100 && channel <= 128)
			return 114;
		else if (channel >= 149 && channel <= 177)
			return 163;
	}
#ifdef MAP_6E_SUPPORT
	else if (op == 133) {/* BW80 case */
		if (channel > 1 && channel <= 13)
			return 7;
		else if (channel >= 17 && channel <= 29)
			return 23;
		else if (channel >= 33 && channel <= 45)
			return 39;
		else if (channel >= 49 && channel <= 61)
			return 55;
		else if (channel >= 65 && channel <= 77)
			return 71;
		else if (channel >= 81 && channel <= 93)
			return 87;
		else if (channel >= 97 && channel <= 109)
			return 103;
		else if (channel >= 113 && channel <= 125)
			return 119;
		else if (channel >= 129 && channel <= 141)
			return 135;
		else if (channel >= 145 && channel <= 157)
			return 151;
		else if (channel >= 161 && channel <= 173)
			return 167;
		else if (channel >= 177 && channel <= 189)
			return 183;
		else if (channel >= 193 && channel <= 205)
			return 199;
		else if (channel >= 209 && channel <= 221)
			return 215;
	} else if (op == 134) {
		if (channel >= 1 && channel <= 29)
			return 15;
		else if (channel >= 33 && channel <= 61)
			return 47;
		else if (channel >= 65 && channel <= 93)
			return 79;
		else if (channel >= 97 && channel <= 125)
			return 111;
		else if (channel >= 129 && channel <= 157)
			return 143;
		else if (channel >= 161 && channel <= 189)
			return 175;
		else if (channel >= 193 && channel <= 221)
			return 207;
	}
#ifdef MAP_320BW
	else if (op == 137) {
		if (channel >= 1 && channel <= 61) {
			if ((!HE_EXTCHA) || channel <= 29)
				return 31;
			else
				return 63;
		} else if (channel >= 33 && channel <= 93) {
			if (!HE_EXTCHA)
				return 63;
			else
				return 95;
		} else if (channel >= 65 && channel <= 125) {
			if (!HE_EXTCHA)
				return 95;
			else
				return 127;
		} else if (channel >= 97 && channel <= 157) {
			if (!HE_EXTCHA)
				return 127;
			else
				return 159;
		} else if (channel >= 129 && channel <= 189) {
			if (!HE_EXTCHA)
				return 159;
			else
				return 191;
		} else if (channel >= 161 && channel <= 221) {
			return 191;
		}
	}
#endif
#endif
	else {
		if (channel >= 36 && channel <= 48)
			return 42;
		else if (channel >= 52 && channel <= 64)
			return 58;
		else if (channel >= 100 && channel <= 112)
			return 106;
		else if (channel >= 116 && channel <= 128)
			return 122;
		else if (channel >= 132 && channel <= 144)
			return 138;
		else if (channel >= 149 && channel <= 161)
			return 155;
		else if (channel >= 165 && channel <= 177)
			return 171;
	}
	return 0;
}

void ch_planning_get_channel_block(unsigned char channel, unsigned char channel_block[MAX_CHANNEL_BLOCKS], unsigned char op, int maxbw)
{
	u8 *grp_list = NULL;
	int i = 0;

	if (maxbw == BW_160 || op == 129) {
		grp_list = &grp_bw_160[0][0];
		if (channel >= 36 && channel <= 64) {
			os_memcpy(&channel_block[0], (grp_list), 8);
		} else if (channel >= 100 && channel <= 128) {
			os_memcpy(&channel_block[0], (grp_list + 8), 8);
		}
	}
#ifdef MAP_320BW
	else if (maxbw == BW_320 || op == 137) {
		grp_list = &grp_bw_320[0][0];
		if (channel >= 1 && channel <= 61) {
			if ((!HE_EXTCHA) || channel <= 29)
				os_memcpy(&channel_block[0], (grp_list), 16);
			else
				 os_memcpy(&channel_block[0], (grp_list + 16), 16);
		} else if (channel >= 33 && channel <= 93) {
			if (!HE_EXTCHA)
				os_memcpy(&channel_block[0], (grp_list + 16), 16);
			else
				os_memcpy(&channel_block[0], (grp_list + 32), 16);
		} else if (channel >= 65 && channel <= 125) {
			if (!HE_EXTCHA)
				os_memcpy(&channel_block[0], (grp_list + 32), 16);
			else
				os_memcpy(&channel_block[0], (grp_list + 48), 16);
		} else if (channel >= 97 && channel <= 157) {
			if (!HE_EXTCHA)
				os_memcpy(&channel_block[0], (grp_list + 48), 16);
			else
				os_memcpy(&channel_block[0], (grp_list + 64), 16);
		} else if (channel >= 129 && channel <= 189) {
			if (!HE_EXTCHA)
				os_memcpy(&channel_block[0], (grp_list + 64), 16);
			else
				os_memcpy(&channel_block[0], (grp_list + 80), 16);
		} else if (channel >= 161 && channel <= 221) {
			if (!HE_EXTCHA)
				os_memcpy(&channel_block[0], (grp_list + 80), 16);
			else
				os_memcpy(&channel_block[0], (grp_list + 80), 16);
		}
	}
#endif
	else {
		if (maxbw == BW_80) {
			for (i = 0;i < MAX_BW_80_BLOCK;i++) {
				if (channel == grp_bw_80[i][0] || channel == grp_bw_80[i][1]
					|| channel == grp_bw_80[i][2] || channel == grp_bw_80[i][3]) {
					channel_block[0] = grp_bw_80[i][0];
					channel_block[1] = grp_bw_80[i][1];
					channel_block[2] = grp_bw_80[i][2];
					channel_block[3] = grp_bw_80[i][3];
					break;
				}
			}
		} else if(maxbw == BW_40) {
			for (i = 0;i < MAX_BW_40_BLOCK;i++) {
				if (channel == grp_bw_40[i][0] || channel == grp_bw_40[i][1]) {
					channel_block[0] = grp_bw_40[i][0];
					channel_block[1] = grp_bw_40[i][1];
					break;
				}
			}
		} else {
			channel_block[0] = channel;
		}
	}
}

unsigned int ch_planning_check_channel_available(struct own_1905_device *ctx,
	unsigned char channel)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	struct radio_info_db *radio_info = NULL;
	struct prefer_info_db *prefer_db = NULL, *t_prefer_db = NULL;
	int i = 0;

	while (dev != NULL) {
		if (dev->in_network) {
			radio_info = topo_srv_get_radio(dev, NULL);
				while (radio_info) {
					SLIST_FOREACH_SAFE(prefer_db,
						&radio_info->chan_preferance.prefer_info_head, prefer_info_entry, t_prefer_db) {
						for(i = 0; i < prefer_db->ch_num; i++)
						{
							if (prefer_db->ch_list[i] == channel) {
								return TRUE;
							}
						}
					}
					radio_info = topo_srv_get_next_radio(dev,
						radio_info);
				}
		}
		dev = topo_srv_get_next_1905_device(ctx, dev);
	}
	return FALSE;
}

unsigned int ch_planning_get_operable_blocks(struct own_1905_device *ctx,
	unsigned char channel)
{
	unsigned int ret = 0;
	unsigned char channel_block[MAX_CHANNEL_BLOCKS] = {0};
	int i = 0, loop = MAX_CHANNEL_BLOCKS;
	struct _1905_map_device *dev = NULL;
	struct radio_info_db *radio_info = NULL;
	int maxbw = BW_20, bw = BW_20;
	struct _1905_map_device *_1905_dev = NULL, *t_1905_dev = NULL;
	SLIST_FOREACH_SAFE(_1905_dev, &ctx->_1905_dev_head, next_1905_device, t_1905_dev) {
		bw = ch_planning_get_dev_bw_from_channel(_1905_dev, channel);
		if (maxbw < bw)
			maxbw = bw;
	}

	dev = topo_srv_get_1905_device(ctx, NULL);
	radio_info = topo_srv_get_radio_by_band(dev, channel);
	if (radio_info && channel > 14) {
		ch_planning_get_channel_block(channel, channel_block, radio_info->operating_class, maxbw);
		for (i = 0; i < loop; i++) {
			if (channel_block[i] == 0)
				continue;

			if (ch_planning_check_channel_available(ctx, channel_block[i]))
				ret++;
		}
	} else {
		ret = 2;
	}
	return ret;
}

#ifdef WIFI_MD_COEX_SUPPORT
unsigned int ch_planning_check_channel_for_dev_operable_wrapper(struct own_1905_device *ctx,
	struct _1905_map_device *dev, unsigned char channel)
{
	unsigned int ret = TRUE;
	unsigned char channel_block[4] = {0};
	int i = 0;
	struct radio_info_db *radio_info = NULL;
	int maxbw = BW_20, bw = BW_20;
	struct _1905_map_device *_1905_dev = NULL, *t_1905_dev = NULL;

	SLIST_FOREACH_SAFE(_1905_dev, &ctx->_1905_dev_head, next_1905_device, t_1905_dev) {
		bw = ch_planning_get_dev_bw_from_channel(_1905_dev, channel);
		if (maxbw < bw)
			maxbw = bw;
	}

	radio_info = topo_srv_get_radio_by_band(dev, channel);

	if (!radio_info)
		return ret;

	if (channel > 14) {
		ch_planning_get_channel_block(channel, channel_block, radio_info->operating_class, maxbw);
		for (i = 0; i < 4; i++) {
			//! check individual block
			ret = ch_planning_check_channel_operable_for_dev(dev, channel_block[i]);
			if (ret == FALSE)
				break;
		}
	} else {
		ret = ch_planning_check_channel_operable_for_dev(dev, channel);
	}
	return ret;
}


unsigned int ch_planning_check_channel_operable_for_dev(struct _1905_map_device *dev,
	unsigned char channel)
{
	struct radio_info_db *radio_info = NULL;
	struct prefer_info_db *prefer_db = NULL, *t_prefer_db = NULL;
	int i = 0;
	//! loop for each device
	if (dev->in_network) {
		radio_info = topo_srv_get_radio(dev, NULL);
			//!  check each radio
			while (radio_info) {
				SLIST_FOREACH_SAFE(prefer_db,
					&radio_info->chan_preferance.prefer_info_head, prefer_info_entry, t_prefer_db) {
					for(i = 0; i < prefer_db->ch_num; i++)
					{
						//! channel is present and preference is set to 0
						//! channel is not operable
						if (prefer_db->ch_list[i] == channel &&
							prefer_db->perference == 0)
							return FALSE;
					}
				}
				radio_info = topo_srv_get_next_radio(dev,
					radio_info);
			}
	}
	//! either channel is not present in preference list or not
	//! marked as 0 preference by any device
	return TRUE;
}
#endif

int ch_planning_check_skip_list(u8 ch_num, struct own_1905_device *ctx)
{
	int i = 0;
	debug("checking skip list while channel planning\n");
	for(i = 0; i < ctx->AutoChannelSkipListNum; i++) {
		debug("print values while searching channel=%d in skiplist ctx->AutoChannelSkipListNum[%d]=%d\n",
				ch_num , i, ctx->AutoChannelSkipList[i]);
		if(ch_num == ctx->AutoChannelSkipList[i]) {
			debug("found a match in skiplist\n");
			return 1;
		}
	}
	return 0;
}

Boolean check_if_operable_20M(struct radio_info_db *radio_info, u8 channel)
{
	struct prefer_info_db *prefer_db = NULL, *t_prefer_db = NULL;
	u8 i = 0;

	SLIST_FOREACH_SAFE(prefer_db,
		&radio_info->chan_preferance.prefer_info_head, prefer_info_entry, t_prefer_db) {

		if (prefer_db->op_class != 115)
			continue;

		for (i = 0; i < prefer_db->ch_num; i++) {
			/* ! channel is present and preference is set to 0 */
			if (prefer_db->ch_list[i] == channel && prefer_db->perference == 0)
				return FALSE;
		}
	}

	return TRUE;
}

int check_skiplist_channel_operable(struct own_1905_device *ctx, struct radio_info_db *radio_info, int maxbw,
		unsigned int user_prefer, unsigned char channel, unsigned char actual_channel)
{
	struct prefer_info_db *prefer_db = NULL, *t_prefer_db = NULL;
	int i = 0;

	SLIST_FOREACH_SAFE(prefer_db,
		&radio_info->chan_preferance.prefer_info_head, prefer_info_entry, t_prefer_db) {
		for (i = 0; i < prefer_db->ch_num; i++) {
			/* channel is present and preference is set to 0*/
			if (maxbw >= chan_mon_get_bw_from_op_class(prefer_db->op_class) &&
				prefer_db->ch_list[i] == channel &&
				(prefer_db->perference == 0 ||
				((user_prefer == 0)
				&& ch_planning_check_skip_list(channel, ctx)))) {
				/* 36 to 48 might be operable on 80M as these are NON DFS channel */
				if (actual_channel >= 36 && actual_channel <= 48 &&
					!(ch_planning_check_skip_list(channel, ctx))) {
					if (check_if_operable_20M(radio_info, actual_channel))
						return TRUE;
				}
				return FALSE;
			}
		}
	}

	return -1;
}

unsigned int ch_planning_check_skiplist_channel_operable(struct own_1905_device *ctx,
		unsigned char channel, unsigned char actual_channel, int maxbw, unsigned int user_prefer)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	struct radio_info_db *radio_info = NULL;
	int ret = 0;

	while (dev != NULL) {
		/* loop for each device & check each radio */
		if (dev->in_network) {
			radio_info = topo_srv_get_radio(dev, NULL);
			while (radio_info) {
				ret = check_skiplist_channel_operable(ctx, radio_info, maxbw, user_prefer, channel, actual_channel);
				if (ret == -1)
					radio_info = topo_srv_get_next_radio(dev, radio_info);
				else
					return ret;
			}
		}
		dev = topo_srv_get_next_1905_device(ctx, dev);
	}
	/* either channel is not present in preference list or marked as 0 preference by any device */
	return TRUE;
}



unsigned int ch_planning_check_skiplist_channel_operable_wrapper(struct own_1905_device *ctx,
		unsigned char channel, unsigned int user_prefer)
{
	unsigned int ret = TRUE;
	unsigned char channel_block[8] = {0};
	int i = 0, loop = 8;
	struct _1905_map_device *dev = NULL;
	struct radio_info_db *radio_info = NULL;
	int maxbw = BW_20, bw = BW_20;
	struct _1905_map_device *_1905_dev = NULL, *t_1905_dev = NULL;
	SLIST_FOREACH_SAFE(_1905_dev, &ctx->_1905_dev_head, next_1905_device, t_1905_dev) {
		bw = ch_planning_get_dev_bw_from_channel(_1905_dev, channel);
		if (maxbw < bw)
			maxbw = bw;
	}

	dev = topo_srv_get_1905_device(ctx, NULL);
	radio_info = topo_srv_get_radio_by_band(dev, channel);

	//! if it is a 5G channel then all 20Mhz blocks should be operable
	if (radio_info && channel > 14) {
		ch_planning_get_channel_block(channel, channel_block, radio_info->operating_class, maxbw);
		for (i = 0; i < loop; i++) {

			if (channel_block[i] == 0)
				continue;

			//! check individual block
			ret = ch_planning_check_skiplist_channel_operable(ctx, channel_block[i], channel, maxbw, user_prefer);
			if (ret == FALSE)
				break;
		}
	}
	return ret;
}

int check_channel_operable(struct radio_info_db *radio_info, unsigned char channel,
		unsigned char actual_channel, int maxbw)
{
	struct prefer_info_db *prefer_db = NULL, *t_prefer_db = NULL;
	int i = 0;

	SLIST_FOREACH_SAFE(prefer_db,
		&radio_info->chan_preferance.prefer_info_head, prefer_info_entry, t_prefer_db) {
		for (i = 0; i < prefer_db->ch_num; i++) {
			/* channel is present and preference is set to 0 */
			if (maxbw >= chan_mon_get_bw_from_op_class(prefer_db->op_class) &&
				prefer_db->ch_list[i] == channel &&
				prefer_db->perference == 0) {
				debug(CH_PLANING_PREX"prefer ch nop: %d", prefer_db->ch_list[i]);
				/* 36 to 48 might be operable on 80M as these are NON DFS channel */
				if (actual_channel >= 36 && actual_channel <= 48) {
					if (check_if_operable_20M(radio_info, actual_channel))
						return TRUE;
				}
				return FALSE;
			}
		}
	}
	return -1;
}

unsigned int ch_planning_check_channel_operable_wrapper(struct own_1905_device *ctx,
	unsigned char channel)
{
	unsigned int ret = TRUE;
	unsigned char channel_block[MAX_CHANNEL_BLOCKS] = {0};
	int loop = MAX_CHANNEL_BLOCKS;
	int i = 0;
	struct _1905_map_device *dev = NULL;
	struct radio_info_db *radio_info = NULL;
	int maxbw = BW_20, bw = BW_20;
		struct _1905_map_device *_1905_dev = NULL, *t_1905_dev = NULL;
		SLIST_FOREACH_SAFE(_1905_dev, &ctx->_1905_dev_head, next_1905_device, t_1905_dev) {
			bw = ch_planning_get_dev_bw_from_channel(_1905_dev, channel);
			if (maxbw < bw)
				maxbw = bw;
	}

	dev = topo_srv_get_1905_device(ctx, NULL);
	radio_info = topo_srv_get_radio_by_band(dev, channel);

	//! if it is a 5G channel then all 20Mhz blocks should be operable
	if (radio_info && channel > 14) {
		ch_planning_get_channel_block(channel, channel_block, radio_info->operating_class, maxbw);
		for (i = 0; i < loop; i++) {

			if (channel_block[i] == 0)
				continue;

			//! check individual block
			ret = ch_planning_check_channel_operable(ctx, channel_block[i], channel, maxbw);
			if (ret == FALSE)
				break;
		}
	}
	return ret;
}

unsigned int ch_planning_check_channel_operable(struct own_1905_device *ctx,
	unsigned char channel, unsigned char actual_channel, int maxbw)
{
	int ret;
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	struct radio_info_db *radio_info = NULL;

	while (dev != NULL) {
		/* loop for each device & check each radio */
		if (dev->in_network) {
			radio_info = topo_srv_get_radio(dev, NULL);
			while (radio_info) {
				ret = check_channel_operable(radio_info, channel, actual_channel, maxbw);
				if (ret == -1)
					radio_info = topo_srv_get_next_radio(dev, radio_info);
				else
					return ret;
			}
		}
		dev = topo_srv_get_next_1905_device(ctx, dev);
	}
	/* either channel is not present in preference list or marked as 0 preference by any device */
	return TRUE;
}

/**
* @brief Fn to find best channel for a radio
* @param ch_distribution chanel_planning global struct
* @param radio
* @return best channel for the radio
* 	return value 0 means radio is already on best possible channel
*	return value negative means channel planning cannot be performed for the radio currently
*	return type should always be signed int to avoid treating channel > 128 as negative value
*/
signed int ch_planning_select_best_channel(
	struct own_1905_device *ctx,
	struct radio_info_db *radio,
	struct _1905_map_device *dev
	)
{
	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct ch_distribution_cb *ch_distribution = NULL;
	struct prefered_ch_cb *prefered_channel = NULL;
	struct prefered_ch_cb *current_prefered_channel = NULL, *t_curr_pref_ch = NULL;
	struct prefered_ch_cb *new_prefered_channel = NULL;
	struct prefered_ch_radio_info_db *prefered_ch_radio = NULL, *t_pref_ch_radio = NULL;
	struct _1905_map_device *cnt_dev = NULL;
	struct radio_info_db *cnt_radio = NULL;
	unsigned char centre_freq_ch = 0;
	signed int cntr_ch_score = 0;
	u8 cntr_ch = 0;

	if (radio->channel[0] == 0)
	{
		err(CH_PLANING_PREX"Operating channel not known yet");
		return -1;
	}
	if (radio->chan_preferance.op_class_num == 0)
	{
		err(CH_PLANING_PREX"Channel preference not known yet");
		return -1;
	}


	if (radio->channel[0] > 14)
	{
		ch_distribution = &ch_planning->ch_ditribution_5g;
	} else {
		ch_distribution = &ch_planning->ch_ditribution_2g;
	}
	ch_planning_update_ch_score(ctx, ch_distribution);
	SLIST_FOREACH_SAFE(current_prefered_channel,
		&(ch_distribution->first_prefered_ch),
		next_prefered_ch, t_curr_pref_ch)
	{
		if (current_prefered_channel->ch_num == radio->channel[0])
			break;
	}

	if (current_prefered_channel == NULL)
	{
		goto bail_out;
	}
	if (!ch_planning_check_channel_operable_wrapper(ctx,current_prefered_channel->ch_num)) {
		current_prefered_channel->ch_score = -1;
	}
	debug(CH_PLANING_PREX"160Mhz central channel: 50 114 will be skipped");
	debug(CH_PLANING_PREX"80Mhz central channel: 42 58 106 138 155 will be skipped");
	t_curr_pref_ch = NULL;
	SLIST_FOREACH_SAFE(prefered_channel,
		&(ch_distribution->first_prefered_ch),
		next_prefered_ch, t_curr_pref_ch) {
		if (!ch_planning_check_channel_operable_wrapper(ctx,prefered_channel->ch_num)) {
			prefered_channel->ch_score = -1;
		}
		if (prefered_channel->ch_num == 50 ||
			prefered_channel->ch_num == 114 ||
			prefered_channel->ch_num == 163)
		{
			debug(CH_PLANING_PREX"prefered channel(%d) is the central channel of 160 Mhz, skip",
				prefered_channel->ch_num);
			continue;
		}
#ifdef MAP_320BW
		if (prefered_channel->ch_num == 31 ||
			prefered_channel->ch_num == 63 ||
			prefered_channel->ch_num == 95 ||
			prefered_channel->ch_num == 127 ||
			prefered_channel->ch_num == 159 ||
			prefered_channel->ch_num == 191) {
			debug(CH_PLANING_PREX"prefered channel(%d) is the central channel of 320 Mhz, skip",
				prefered_channel->ch_num);
			continue;
		}
#endif
		centre_freq_ch = ch_planning_get_centre_freq_ch(prefered_channel->ch_num, radio->operating_class);
		if (prefered_channel->ch_num == centre_freq_ch)
		{
			debug(CH_PLANING_PREX"Not suitable candidate as  %d is a centre freq in 80Mhz",
				prefered_channel->ch_num);
			continue;
		}
		if (radio->operating_class == 129) {
			centre_freq_ch = ch_planning_get_centre_freq_ch(prefered_channel->ch_num, 128);
			if (prefered_channel->ch_num == centre_freq_ch) {
				err(CH_PLANING_PREX"Not suitable candidate as  %d is a centre freq in 80/160Mhz",
					prefered_channel->ch_num);
				continue;
			}
		}

		if (current_prefered_channel == prefered_channel)
		{
			continue;
		}
		if (prefered_channel->ch_score == -1)
		{
			err(CH_PLANING_PREX"Continue as ch %d is not operable",
				prefered_channel->ch_num);
			continue;
		}
		if (current_prefered_channel->ch_score >= prefered_channel->ch_score)
		{
			continue;
		}
		SLIST_FOREACH_SAFE(prefered_ch_radio,
			&(prefered_channel->first_radio),
			next_pref_ch_radio, t_pref_ch_radio) {
			if ((prefered_ch_radio->radio == radio))
			{
				if (radio->operating_channel == NULL ||
					radio->chan_preferance.op_class_num == 0) {
					err(CH_PLANING_PREX"Channel Distribution not complete yet");
					return -1;
				}
				if (new_prefered_channel == NULL ||
					new_prefered_channel->ch_score < prefered_channel->ch_score)
				{
					new_prefered_channel = prefered_channel;
					debug(CH_PLANING_PREX"Radio on ch %d, score = %x,possible better ch %d, score = %x",
						radio->channel[0], current_prefered_channel->ch_score,
						prefered_channel->ch_num,
						new_prefered_channel->ch_score);
				}
				break;
			}
		}
	}


	cnt_dev = topo_srv_get_1905_device(ctx, NULL);
	if (cnt_dev)
		cnt_radio = topo_srv_get_radio_by_band(cnt_dev, radio->channel[0]);
	if (cnt_radio) {
		if (cnt_radio->channel[0] != cnt_radio->prev_channel) {
			cntr_ch_score = ch_planning_get_ch_score(ctx, ch_distribution, cnt_radio->prev_channel);
			cntr_ch = cnt_radio->prev_channel;
		} else {
			cntr_ch_score = ch_planning_get_ch_score(ctx, ch_distribution, cnt_radio->channel[0]);
			cntr_ch = cnt_radio->channel[0];
		}

	if (new_prefered_channel) {
		if (dev->device_role == DEVICE_ROLE_AGENT) {
			if (cntr_ch_score == new_prefered_channel->ch_score) {
				err(CH_PLANING_PREX"controller current channel score equals to new_prefered_channel!"
					" choose controller channel! prev_channel(%d) cur_op_ch(%d)",
					cnt_radio->prev_channel,cnt_radio->channel[0]);
				return cntr_ch;
			}
		} else {
			cnt_radio->prev_channel = new_prefered_channel->ch_num;
			err(CH_PLANING_PREX"record controller prev_channel to %d",
				cnt_radio->prev_channel);
		}
		err(CH_PLANING_PREX"find new channel(%d) score(%x)",
			new_prefered_channel->ch_num, new_prefered_channel->ch_score);
		return new_prefered_channel->ch_num;
	} else {
		if (dev->device_role == DEVICE_ROLE_AGENT) {
			if (cntr_ch_score == current_prefered_channel->ch_score &&
				current_prefered_channel->ch_num != cntr_ch) {
				err(CH_PLANING_PREX"controller current channel score equals to current_prefered_channel!"
					" choose controller channel! prev_channel(%d) cur_op_ch(%d)",
					cnt_radio->prev_channel, cnt_radio->channel[0]);
				return cntr_ch;
			}
		}
	}
	}
	err(CH_PLANING_PREX"Radio already on best possible ch");

	
	return 0;
bail_out:
	err(CH_PLANING_PREX"Prefered ch list not present for the device yet");
	return -1;
}


unsigned int ch_planning_check_chan_oper_for_dev(struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned char channel)
{
	struct radio_info_db *radio_info = NULL;
	struct prefer_info_db *prefer_db = NULL, *t_prefer_db = NULL;
	int i = 0;
	int maxbw = BW_20;

	if(!dev)
		return FALSE;

	if (!dev->in_network)
		return FALSE;

	maxbw = ch_planning_get_max_bw_in_network(ctx, channel);



	//! loop for each device

	radio_info = topo_srv_get_radio(dev, NULL);

	//!  check each radio
	while (radio_info) {
		SLIST_FOREACH_SAFE(prefer_db,
			&radio_info->chan_preferance.prefer_info_head, prefer_info_entry, t_prefer_db) {
			for(i = 0; i < prefer_db->ch_num; i++)
			{
				//! channel is present and preference is set to 0
				//! channel is not operable
				if (maxbw >= chan_mon_get_bw_from_op_class(prefer_db->op_class) &&
					prefer_db->ch_list[i] == channel
					&&	prefer_db->perference == 0){
					debug(CH_PLANING_PREX"prefer ch op: %d", prefer_db->ch_list[i]);
					return FALSE;
				}

				if (prefer_db->ch_list[i] == channel){
					return TRUE;
			}
		}
		}
		radio_info = topo_srv_get_next_radio(dev,
			radio_info);
	}


	//! either channel is not present in preference list or not
	//! marked as 0 preference by any device
	return FALSE;
}

u8 ch_planning_all_pref_done(
              struct ch_prefer_lib *ch_prefer)
{
       u8 j=0;
       for (j = 0; j < ch_prefer->op_class_num; j++) {
               if(ch_prefer->opinfo[j].perference == 0)
                       return 0;
       }
       return 1;
}
void dump_ch_prefer_info(
	struct ch_prefer_lib *ch_prefer)
{
	u8 i = 0;
	for (i = 0; i < ch_prefer->op_class_num; i++) {
		 err(CH_PLANING_PREX"opclass: %u, chnum: %u,ch_list %u, pref: %u",
		 		ch_prefer->opinfo[i].op_class,
                ch_prefer->opinfo[i].ch_num,
                ch_prefer->opinfo[i].ch_list[0],
                ch_prefer->opinfo[i].perference);
	}

}
/*Re-define the func of ch_planning_ch_selection_prefer_data.
This is because there is IOT issue of Channel Preference TLV with Qualcomm Agent.
According to R1 spec, we should not only insert the preferred Channel but also need insert all other Channels with lower preference value*/

/*
@ [input] channel: the best channel which Multi-AP Agent is asked to operated on
@ [input] radio: the data struct of radio of Multi-AP Agent
@ [output] ch_prefer: Channel Preference data which is used to construct the Channel Preference TLV.
*/
void ch_planning_ch_selection_prefer_data(
	struct own_1905_device *ctx,
	unsigned char channel,
	struct radio_info_db *radio,
	struct ch_prefer_lib *ch_prefer)
{
	struct prefer_info_db *prefer_info = NULL, *t_prefer_info = NULL;
	unsigned char num_of_op_class = 0; //num of operating class withing one radio unique
	int i = 0, j = 0, offset = 0;
	unsigned char op_class_match = FALSE; //boolean value

	os_memcpy(ch_prefer->identifier, radio->identifier, ETH_ALEN);

	SLIST_FOREACH_SAFE(prefer_info, &(radio->chan_preferance.prefer_info_head), prefer_info_entry, t_prefer_info) {
	//This loop is insert the preferred channel with higher preference value including 20M, 40M, 80M of channel with op class

		if (prefer_info->op_class <= 127)
		{
			for (i = 0; i < prefer_info->ch_num; i++)
			{
				if (channel	== prefer_info->ch_list[i])
				{
					ch_prefer->opinfo[num_of_op_class].op_class = prefer_info->op_class;
					ch_prefer->opinfo[num_of_op_class].ch_num = 1;
					ch_prefer->opinfo[num_of_op_class].ch_list[0] = channel;
					if(prefer_info->perference == 15)
						ch_prefer->opinfo[num_of_op_class].perference = 14;
					else
						ch_prefer->opinfo[num_of_op_class].perference = prefer_info->perference;//14;
#ifdef MAP_R2
					u8 check_cac = is_CAC_Success((struct mapd_global *)ctx->back_ptr, channel);
					if(check_cac) {
						ch_prefer->opinfo[num_of_op_class].reason = DFS_CH_CLEAR_INDICATION;
						err(CH_PLANING_PREX"CAC succes, not to do CAC %d", channel);
					}
#endif

					num_of_op_class++;
					if(num_of_op_class == MAX_OP_CLASS_NUM)
					{
						err(CH_PLANING_PREX"[ERROR]number of OP class is too big, can't insert Channel Preference anymore.");
						goto finish;
					}
				}
			}
		}
		else if(prefer_info->op_class == 128 && radio->band != BAND_2G)
		{
			for (i = 0; i < prefer_info->ch_num; i++)
			{
				offset = channel - prefer_info->ch_list[i];
				if (abs(offset) <= 6) //the channel is within the 80M coverage
				{
					ch_prefer->opinfo[num_of_op_class].op_class = prefer_info->op_class;
					ch_prefer->opinfo[num_of_op_class].ch_num = 1;
					ch_prefer->opinfo[num_of_op_class].ch_list[0] = prefer_info->ch_list[i];
					if(prefer_info->perference == 15)
						ch_prefer->opinfo[num_of_op_class].perference = 14;
					else
						ch_prefer->opinfo[num_of_op_class].perference = prefer_info->perference;//14;
#ifdef MAP_R2
					u8 check_cac = is_CAC_Success((struct mapd_global *)ctx->back_ptr, channel);
					if(check_cac) {
						ch_prefer->opinfo[num_of_op_class].reason = DFS_CH_CLEAR_INDICATION;
						err(CH_PLANING_PREX"CAC succes, not to CAC %d", channel);
					}
#endif
					num_of_op_class++;
					ch_prefer->opinfo[num_of_op_class].op_class = chan_mon_get_op_class_frm_channel(channel, BW_20);
					ch_prefer->opinfo[num_of_op_class].ch_num = 1;
					ch_prefer->opinfo[num_of_op_class].ch_list[0] = channel;
					ch_prefer->opinfo[num_of_op_class].reason = ch_prefer->opinfo[num_of_op_class - 1].reason;
					ch_prefer->opinfo[num_of_op_class].perference = ch_prefer->opinfo[num_of_op_class - 1].perference;

					num_of_op_class++;
					if(num_of_op_class == MAX_OP_CLASS_NUM)
					{
						err(CH_PLANING_PREX"[ERROR]number of OP class is too big, can't insert Channel Preference anymore.");
						goto finish;
					}
				}
			}

		}
		else if(prefer_info->op_class == 129 && radio->band != BAND_2G) {
			for (i = 0; i < prefer_info->ch_num; i++)
			{
				offset = channel - prefer_info->ch_list[i];
				if (abs(offset) <= 14) //the channel is within the 160M coverage
				{
					ch_prefer->opinfo[num_of_op_class].op_class = prefer_info->op_class;
					ch_prefer->opinfo[num_of_op_class].ch_num = 1;
					ch_prefer->opinfo[num_of_op_class].ch_list[0] = prefer_info->ch_list[i];
					if(prefer_info->perference == 15)
						ch_prefer->opinfo[num_of_op_class].perference = 14;
					else
						ch_prefer->opinfo[num_of_op_class].perference = prefer_info->perference;//14;
#ifdef MAP_R2
					u8 check_cac = is_CAC_Success((struct mapd_global *)ctx->back_ptr, channel);
					if(check_cac) {
						ch_prefer->opinfo[num_of_op_class].reason = DFS_CH_CLEAR_INDICATION;
						err(CH_PLANING_PREX"CAC succes, not to CAC %d", channel);
					}
#endif
					num_of_op_class++;
					ch_prefer->opinfo[num_of_op_class].op_class = chan_mon_get_op_class_frm_channel(channel, BW_20);
					ch_prefer->opinfo[num_of_op_class].ch_num = 1;
					ch_prefer->opinfo[num_of_op_class].ch_list[0] = channel;
					ch_prefer->opinfo[num_of_op_class].reason = ch_prefer->opinfo[num_of_op_class - 1].reason;
					ch_prefer->opinfo[num_of_op_class].perference = ch_prefer->opinfo[num_of_op_class - 1].perference;

					num_of_op_class++;
					if(num_of_op_class == MAX_OP_CLASS_NUM)
					{
						err(CH_PLANING_PREX"[ERROR]number of OP class is too big, can't insert Channel Preference anymore.");
						goto finish;
					}
				}
			}
		}
#ifdef MAP_320BW
		else if (prefer_info->op_class == 137 && radio->band != BAND_2G && radio->band != BAND_5GH && radio->band != BAND_5GL) {
			for (i = 0; i < prefer_info->ch_num; i++) {
				offset = channel - prefer_info->ch_list[i];
				if (abs(offset) <= 30) {
					ch_prefer->opinfo[num_of_op_class].op_class = prefer_info->op_class;
					ch_prefer->opinfo[num_of_op_class].ch_num = 1;
					ch_prefer->opinfo[num_of_op_class].ch_list[0] = prefer_info->ch_list[i];
					if (prefer_info->perference == 15)
						ch_prefer->opinfo[num_of_op_class].perference = 14;
					else
						ch_prefer->opinfo[num_of_op_class].perference = prefer_info->perference;
#ifdef MAP_R2
					u8 check_cac = is_CAC_Success((struct mapd_global *)ctx->back_ptr, channel);

					if (check_cac) {
						ch_prefer->opinfo[num_of_op_class].reason = DFS_CH_CLEAR_INDICATION;
						err(CH_PLANING_PREX"CAC succes, not to CAC %d", channel);
					}
#endif
					num_of_op_class++;
					ch_prefer->opinfo[num_of_op_class].op_class = chan_mon_get_op_class_frm_channel(channel, BW_20);
					ch_prefer->opinfo[num_of_op_class].ch_num = 1;
					ch_prefer->opinfo[num_of_op_class].ch_list[0] = channel;
					ch_prefer->opinfo[num_of_op_class].reason = ch_prefer->opinfo[num_of_op_class - 1].reason;
					ch_prefer->opinfo[num_of_op_class].perference = ch_prefer->opinfo[num_of_op_class - 1].perference;

					num_of_op_class++;
					if (num_of_op_class == MAX_OP_CLASS_NUM) {
						err(CH_PLANING_PREX"[ERROR]number of OP class is too big, can't insert");
						goto finish;
					}
				}
			}
		}
#endif
	} // end of SLIST_FOREACH(prefer_info, &(radio->chan_preferance.prefer_info_head), prefer_info_entry)
	t_prefer_info = NULL;
	SLIST_FOREACH_SAFE(prefer_info, &(radio->chan_preferance.prefer_info_head), prefer_info_entry, t_prefer_info) {
	//This loop is insert all the channels except preferred channel to assign lower preference value

		op_class_match = FALSE;
		if (prefer_info->op_class <= 127)
		{
			op_class_match = TRUE;
			ch_prefer->opinfo[num_of_op_class].op_class = prefer_info->op_class;
			ch_prefer->opinfo[num_of_op_class].ch_num = prefer_info->ch_num;
			if(radio->band == BAND_2G)
				ch_prefer->opinfo[num_of_op_class].perference = 0;
			else
				ch_prefer->opinfo[num_of_op_class].perference = 1; //should at least one non-DFS channel as operable

			if(prefer_info->reason){
				if(prefer_info->perference == 15)
					ch_prefer->opinfo[num_of_op_class].perference = 14;
				else
					ch_prefer->opinfo[num_of_op_class].perference = prefer_info->perference;
				ch_prefer->opinfo[num_of_op_class].reason = prefer_info->reason;
			}

			i = 0; //index for prefer_info->ch_list[]
			j = 0; //index for ch_prefer->opinfo[].ch_list[]
			while (i < prefer_info->ch_num)
			{	//ch_list should not contain the input channel
				if (channel	!= prefer_info->ch_list[i])
				{
					ch_prefer->opinfo[num_of_op_class].ch_list[j] = prefer_info->ch_list[i];
					j++;
				}
				else
				{
					ch_prefer->opinfo[num_of_op_class].ch_num--;
				}

				i++;
			}
		}
		else if(prefer_info->op_class == 128 && radio->band != BAND_2G)
		{
			op_class_match = TRUE;
			ch_prefer->opinfo[num_of_op_class].op_class = prefer_info->op_class;
			ch_prefer->opinfo[num_of_op_class].ch_num = prefer_info->ch_num;
			ch_prefer->opinfo[num_of_op_class].perference = 1;
			if(prefer_info->reason){
				if(prefer_info->perference == 15)
					ch_prefer->opinfo[num_of_op_class].perference = 14;
				else
					ch_prefer->opinfo[num_of_op_class].perference = prefer_info->perference;
				ch_prefer->opinfo[num_of_op_class].reason = prefer_info->reason;
			}

			i = 0; //index for prefer_info->ch_list[]
			j = 0; //index for ch_prefer->opinfo[].ch_list[]
			while (i < prefer_info->ch_num)
			{
				offset = channel - prefer_info->ch_list[i];
				if (abs(offset) > 6) //the channel is within the 80M coverage
				{
					ch_prefer->opinfo[num_of_op_class].ch_list[j] = prefer_info->ch_list[i];
					j++;
				}
				else
				{
					ch_prefer->opinfo[num_of_op_class].ch_num--;
				}
				i++;
			}
		}
		else if(prefer_info->op_class == 129 && radio->band != BAND_2G)
		{
			op_class_match = TRUE;
			ch_prefer->opinfo[num_of_op_class].op_class = prefer_info->op_class;
			ch_prefer->opinfo[num_of_op_class].ch_num = prefer_info->ch_num;
			ch_prefer->opinfo[num_of_op_class].perference = 1;
			if(prefer_info->reason){
				if(prefer_info->perference == 15)
					ch_prefer->opinfo[num_of_op_class].perference = 14;
				else
					ch_prefer->opinfo[num_of_op_class].perference = prefer_info->perference;
				ch_prefer->opinfo[num_of_op_class].reason = prefer_info->reason;
			}

			i = 0; //index for prefer_info->ch_list[]
			j = 0; //index for ch_prefer->opinfo[].ch_list[]
			while (i < prefer_info->ch_num)
			{
				offset = channel - prefer_info->ch_list[i];
				if (abs(offset) > 14) //the channel is within the 160M coverage
				{
					ch_prefer->opinfo[num_of_op_class].ch_list[j] = prefer_info->ch_list[i];
					j++;
				}
				else
				{
					ch_prefer->opinfo[num_of_op_class].ch_num--;
				}
				i++;
			}
		}
#ifdef MAP_320BW
		else if (prefer_info->op_class == 137 && radio->band != BAND_2G && radio->band != BAND_5GL && radio->band != BAND_5GH) {
			op_class_match = TRUE;
			ch_prefer->opinfo[num_of_op_class].op_class = prefer_info->op_class;
			ch_prefer->opinfo[num_of_op_class].ch_num = prefer_info->ch_num;
			ch_prefer->opinfo[num_of_op_class].perference = 1;
			if (prefer_info->reason) {
				if (prefer_info->perference == 15)
					ch_prefer->opinfo[num_of_op_class].perference = 14;
				else
					ch_prefer->opinfo[num_of_op_class].perference = prefer_info->perference;
				ch_prefer->opinfo[num_of_op_class].reason = prefer_info->reason;
			}

			i = 0;
			j = 0;
			while (i < prefer_info->ch_num) {
				offset = channel - prefer_info->ch_list[i];
				if (abs(offset) > 30) {
					ch_prefer->opinfo[num_of_op_class].ch_list[j] = prefer_info->ch_list[i];
					j++;
				} else
					ch_prefer->opinfo[num_of_op_class].ch_num--;
				i++;
			}
		}
#endif
		if(op_class_match){
			num_of_op_class++;
			if(num_of_op_class == MAX_OP_CLASS_NUM)
			{
				err(CH_PLANING_PREX"[ERROR]number of OP class is too big, can't insert Channel Preference anymore.");
				goto finish;
			}
		}
	} // end of SLIST_FOREACH(prefer_info, &(radio->chan_preferance.prefer_info_head), prefer_info_entry)
finish:
	ch_prefer->op_class_num = num_of_op_class;

	//dump_ch_prefer_info(ch_prefer);
}



/**
* @brief Fn timeout handler for channel selection request
*/

void ch_planning_timeout_handler(void * eloop_ctx, void *user_ctx)
{
	struct own_1905_device *ctx = eloop_ctx;
	struct _1905_map_device *dev = user_ctx;

	if (dev != NULL && dev->ch_sel_req_given) {
		err(CH_PLANING_PREX"ch_planning_timeout for "MACSTR"\n",
			MAC2STR(dev->_1905_info.al_mac_addr));
		ctx->ch_planning.current_ch_planning_dev = NULL;
		ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
		dev->ch_sel_req_given = 0;
	}
}

signed int ch_planning_get_ch_score(
	struct own_1905_device *ctx,
	struct ch_distribution_cb *ch_distribution,
	u8 channel)
{
	struct prefered_ch_cb *prefered_ch = NULL, *t_pref_ch = NULL;
	
	if (!ctx || !ch_distribution) {
		err("invalid params! ctx(%p) or ch_distribution(%p) null",
			ctx, ch_distribution);
		return 0;
	}

	SLIST_FOREACH_SAFE(prefered_ch, &ch_distribution->first_prefered_ch, next_prefered_ch, t_pref_ch){
		if (prefered_ch->ch_num == channel)
			return prefered_ch->ch_score;
	}

	return 0;
}


/**
* @brief Fn decide channel score
* @param ch_distribution_cb channel planning global
* @param channel target channel
*/

void ch_planning_update_ch_score(
	struct own_1905_device *ctx,
	struct ch_distribution_cb *ch_distribution)
{
	char operating_count = 0;
	struct prefered_ch_cb *prefered_ch, *t_pref_ch = NULL;
	unsigned int channel_operable = 0;
	signed int operable_blocks = 0;
	SLIST_FOREACH_SAFE(prefered_ch, &ch_distribution->first_prefered_ch, next_prefered_ch, t_pref_ch){

		if((ch_distribution->user_prefered_ch != prefered_ch->ch_num)
				&& ch_planning_check_skip_list(prefered_ch->ch_num, ctx)) {
			debug("Selected preferred channel=%d is in skiplist\n", prefered_ch->ch_num);
			prefered_ch->in_skiplist = 1;
		}

	}
	t_pref_ch = NULL;
	SLIST_FOREACH_SAFE(prefered_ch, &ch_distribution->first_prefered_ch, next_prefered_ch, t_pref_ch){

		if((ch_distribution->user_prefered_ch != prefered_ch->ch_num)
				&& ch_planning_check_skip_list(prefered_ch->ch_num, ctx)) {
			debug("Selected preferred channel=%d is in skiplist\n", prefered_ch->ch_num);
			/* For 20 MHz BW cases in skiplist*/
			prefered_ch->ch_score = -1;
			continue;
		}

		channel_operable = ch_planning_check_skiplist_channel_operable_wrapper(ctx, prefered_ch->ch_num,
				((ch_distribution->user_prefered_ch != prefered_ch->ch_num)? 0 : 1));

		//! radar is not present, score = user_prefered << 24| prefered_count << 16 | operating_count << 8 | operable blocks
		if (channel_operable) {
#ifdef MAP_R2
			if(ctx->ch_planning_R2.ch_plan_enable == TRUE) {
				if (ctx->ch_planning_R2.ch_plan_enable_bw &&
					((prefered_ch->ch_num > 14) && (ctx->ch_planning_R2.grp_bw != BW_20))) {
					/*If map R2 and the feature is enabled then use ch group Rank instead of operating count*/
					operating_count = ch_planning_get_grp_rank(ctx, prefered_ch->ch_num);
					debug(CH_PLANING_PREX"channel %d grp rank %d", prefered_ch->ch_num, operating_count);
				} else {
					/*If map R2 then use channel Rank instead of operating count*/
					operating_count = ch_planning_get_ch_rank(ctx, prefered_ch->ch_num);
				}
			} else {
				operating_count = ch_planning_get_num_radio_on_operating_channel(ch_distribution,
				prefered_ch->ch_num);
			}
#else
			operating_count = ch_planning_get_num_radio_on_operating_channel(ch_distribution,
				prefered_ch->ch_num);
#endif
			operable_blocks = ch_planning_get_operable_blocks(ctx, prefered_ch->ch_num);
			prefered_ch->ch_score = (prefered_ch->radio_count << 8) | operating_count;
			if (prefered_ch->ch_num == ch_distribution->user_prefered_ch) {
				debug(CH_PLANING_PREX"update normal score MSB set %d\n",prefered_ch->ch_num);
				prefered_ch->ch_score |= 1 << 16;
			}
			else if(prefered_ch->ch_num == ch_distribution->user_prefered_ch_HighBand) {
				debug(CH_PLANING_PREX"update high band score MSB set %d\n",prefered_ch->ch_num);
				prefered_ch->ch_score |= 1 << 16;
			}
			prefered_ch->ch_score = (prefered_ch->ch_score << 8) | operable_blocks;
		} else {
			//! radar found, score is -1, reset user_pref_ch
			if (prefered_ch->ch_num == ch_distribution->user_prefered_ch) {
				ch_distribution->user_prefered_ch = 0;
			} else if (prefered_ch->ch_num == ch_distribution->user_prefered_ch_HighBand) {
				ch_distribution->user_prefered_ch_HighBand = 0;
			}
			prefered_ch->ch_score = -1;
		}
	}
	//ch_planning_show_ch_distribution(ctx);

}

/**
* @brief Fn to add radio to prefered channel list
* @param ch_distribution_cb channel planning global
* @param radio target radio
* @param prefered_ch element
*/

void ch_planning_add_radio_to_prefered_ch(
	struct own_1905_device *ctx,
	struct ch_distribution_cb *ch_distribution,
	struct radio_info_db *radio,
	struct prefered_ch_cb *prefered_ch)
{
	struct prefered_ch_radio_info_db *prefered_ch_radio =NULL;
	prefered_ch_radio =
		os_zalloc(sizeof(struct prefered_ch_radio_info_db));
	prefered_ch_radio->radio = radio;
	SLIST_INSERT_HEAD(
		&(prefered_ch->first_radio),
		prefered_ch_radio,
		next_pref_ch_radio);
	prefered_ch->radio_count++;
	//ch_planning_update_ch_score(ctx, ch_distribution, prefered_ch->ch_num);
}

/**
* @brief Fn to add a channel to prefered list
* @param ctx own 1905 global
* @param channel target channel
* @param radio target radio
* @param band
* @param preference
* @param reason for preference
*/

void ch_planning_add_ch_to_prefered_list(
	struct own_1905_device *ctx,
	unsigned char channel,
	struct radio_info_db *radio,
	unsigned char band,
	unsigned char preference,
	unsigned char reason,
	unsigned char op_class)
{

	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct ch_distribution_cb *ch_distribution = NULL;
	struct prefered_ch_cb *prefered_ch= NULL, *t_pref_ch = NULL;
	struct prefered_ch_radio_info_db *prefered_ch_radio =NULL, *t_pref_ch_radio = NULL;

#ifdef MAP_6E_SUPPORT
	if (radio == NULL)
		return;

	if (IS_MAP_CH_5G(channel) && IS_OP_CLASS_5G(radio->operating_class))
		ch_distribution = &ch_planning->ch_ditribution_5g;
	else if (IS_MAP_CH_24G(channel) && IS_OP_CLASS_24G(radio->operating_class))
		ch_distribution = &ch_planning->ch_ditribution_2g;
	else if (IS_MAP_CH_6G(channel) && IS_OP_CLASS_6G(radio->operating_class))
		ch_distribution = &ch_planning->ch_ditribution_6g;
#else
	if (band)
	{
		//err("ch_ditribution_5g\n");
		ch_distribution = &ch_planning->ch_ditribution_5g;
	} else {
		//err("ch_ditribution_2g\n");
		ch_distribution = &ch_planning->ch_ditribution_2g;
	}
#endif
	SLIST_FOREACH_SAFE(prefered_ch, &(ch_distribution->first_prefered_ch), next_prefered_ch, t_pref_ch) {
		if (prefered_ch->ch_num == channel)
		{
			SLIST_FOREACH_SAFE(prefered_ch_radio,
				&(prefered_ch->first_radio), next_pref_ch_radio, t_pref_ch_radio) {
				if (prefered_ch_radio->radio == radio) {
					break;
				}
			}
			if (prefered_ch_radio == NULL) {
				ch_planning_add_radio_to_prefered_ch(ctx, ch_distribution,
					radio, prefered_ch);
			}
			break;
		}
	}
	if (prefered_ch == NULL)
	{
		//err("This channel not currently predent in prefered list\n");
		prefered_ch = os_zalloc(sizeof(struct prefered_ch_cb));
		SLIST_INIT(&(prefered_ch->first_radio));
		//err("Allocate a new prefered channel and add radio to it\n");
		prefered_ch->ch_num = channel;
		prefered_ch->possible_op_class = op_class;
		if (ch_distribution->prefered_ch_count == 0)
		{
			SLIST_INIT(&(ch_distribution->first_prefered_ch));
		}
		SLIST_INSERT_HEAD(
			&(ch_distribution->first_prefered_ch),
			prefered_ch,
			next_prefered_ch);
		ch_planning_add_radio_to_prefered_ch(ctx, ch_distribution,
			radio, prefered_ch);
		ch_distribution->prefered_ch_count++;
	}
	//ch_planning_update_ch_score(ctx, ch_distribution, channel);
}
/**
* @brief Fn to remove channel from prefered list
* @param ctx own 1905 global
* @param channel target channel
* @param radio target radio
* @param band
*/

void ch_planning_remove_ch_from_prefered_list(
	struct own_1905_device *ctx,
	unsigned char channel,
	struct radio_info_db *radio,
	unsigned char band)
{
	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct ch_distribution_cb *ch_distribution = NULL;
	struct prefered_ch_cb *prefered_ch= NULL, *t_pref_ch = NULL;
	struct prefered_ch_cb *prefered_ch_temp= NULL;
	struct prefered_ch_radio_info_db *prefered_ch_radio_temp = NULL, *t_pref_ch_radio_tmp = NULL;

#ifdef MAP_6E_SUPPORT
	if (IS_MAP_CH_5G(channel) && IS_OP_CLASS_5G(radio->operating_class))
		ch_distribution = &ch_planning->ch_ditribution_5g;
	else if (IS_MAP_CH_24G(channel) && IS_OP_CLASS_24G(radio->operating_class))
		ch_distribution = &ch_planning->ch_ditribution_2g;
	else if (IS_MAP_CH_6G(channel) && IS_OP_CLASS_6G(radio->operating_class))
		ch_distribution = &ch_planning->ch_ditribution_6g;
#else
	if (band)
	{
		//err("ch_ditribution_5g\n");
		ch_distribution = &ch_planning->ch_ditribution_5g;
	} else {
		//err("ch_ditribution_2g\n");
		ch_distribution = &ch_planning->ch_ditribution_2g;
	}
#endif
	SLIST_FOREACH_SAFE(prefered_ch, &(ch_distribution->first_prefered_ch), next_prefered_ch, t_pref_ch) {
		if (prefered_ch->ch_num == channel)
		{
			//err("Channel %d is present in prefered list\n", channel);
			SLIST_FOREACH_SAFE(prefered_ch_radio_temp,
				&(prefered_ch->first_radio),
				next_pref_ch_radio, t_pref_ch_radio_tmp) {
				if (prefered_ch_radio_temp->radio == radio)
				{
					//err("remove radio from the list\n");
					SLIST_REMOVE(&(prefered_ch->first_radio),
					prefered_ch_radio_temp,
					prefered_ch_radio_info_db,
					next_pref_ch_radio);
					os_free(prefered_ch_radio_temp);
					prefered_ch->radio_count--;
					break;
				}
			}
			if (prefered_ch->radio_count == 0) {
				//err("No more radios prefer channel %d\n", prefered_ch->ch_num);
				prefered_ch_temp = SLIST_NEXT(prefered_ch, next_prefered_ch);
				SLIST_REMOVE(&(ch_distribution->first_prefered_ch),
				prefered_ch,
				prefered_ch_cb,
				next_prefered_ch);
				os_free(prefered_ch);
				prefered_ch = prefered_ch_temp;
				ch_distribution->prefered_ch_count--;
				if (prefered_ch == NULL)
				{
					break;
				}
			}
		}
	}

	//ch_planning_update_ch_score(ctx, ch_distribution, channel);
}
/**
* @brief Fn to remove channel from operating list
* @param ctx own 1905 global
* @param channel target channel
* @param radio target radio
* @param band
*/

void ch_planning_remove_ch_from_operating_list(
	struct own_1905_device *ctx,
	unsigned char channel,
	struct radio_info_db *radio,
	unsigned char band)
{
	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct ch_distribution_cb *ch_distribution = NULL;
	struct operating_ch_cb *operating_ch= NULL, *t_op_ch = NULL;
	struct operating_ch_cb *operating_ch_temp = NULL;
	struct radio_info_db *radio_tmp = NULL, *t_radio_tmp = NULL;

	if (band)
	{
		//err("ch_ditribution_5g\n");
		ch_distribution = &ch_planning->ch_ditribution_5g;
	} else {
		//err("ch_ditribution_2g\n");
		ch_distribution = &ch_planning->ch_ditribution_2g;
	}
	SLIST_FOREACH_SAFE(operating_ch,
		&(ch_distribution->first_operating_ch),
		next_operating_ch, t_op_ch) {
		if (operating_ch->ch_num == channel)
		{
			err(CH_PLANING_PREX"Channel %d is present in operating list\n",
				channel);
			SLIST_FOREACH_SAFE(radio_tmp,
				&(operating_ch->first_radio),
				next_co_ch_radio, t_radio_tmp) {
				if (radio_tmp == radio)
				{
					err(CH_PLANING_PREX"remove radio from the list\n");
					SLIST_REMOVE(&(operating_ch->first_radio),
					radio_tmp,
					radio_info_db,
					next_co_ch_radio);
					operating_ch->radio_count--;
					//ch_planning_update_ch_score(ctx, ch_distribution, channel);
					break;
				}
			}
			if (operating_ch->radio_count == 0) {
				err(CH_PLANING_PREX"No more radios prefer channel %d\n",
					operating_ch->ch_num);
				operating_ch_temp = SLIST_NEXT(operating_ch, next_operating_ch);
				SLIST_REMOVE(&(ch_distribution->first_operating_ch),
				operating_ch,
				operating_ch_cb,
				next_operating_ch);
				os_free(operating_ch);
				operating_ch = operating_ch_temp;
				ch_distribution->prefered_ch_count--;
				ch_distribution->operating_ch_count--;
				if (operating_ch == NULL)
				{
					break;
				}
			}
		}
	}
	if (operating_ch != NULL)
	{
		err(CH_PLANING_PREX"Channel %d still exist, remove from list temporarily\n",
			operating_ch->ch_num);
		SLIST_REMOVE(&ch_distribution->first_operating_ch,
		operating_ch,
		operating_ch_cb,
		next_operating_ch);
		ch_planning_insert_into_ch_operating(ctx,
			operating_ch,
			ch_distribution);
	}
}
Boolean channel_validInList(struct own_1905_device *ctx,struct radio_info_db *radio, unsigned int channel)
{
	struct prefer_info_db *prefer_db, *t_pref_db = NULL;
	int j;
	if (radio->operating_channel == NULL ||
		radio->chan_preferance.op_class_num == 0) {
		err(CH_PLANING_PREX"Channel Distribution not complete yet\n");
		return FALSE;
	}
	SLIST_FOREACH_SAFE(prefer_db,
		&radio->chan_preferance.prefer_info_head,
		prefer_info_entry, t_pref_db) {
		for(j = 0; j < prefer_db->ch_num; j++)
		{
			if(channel==prefer_db->ch_list[j])
			{
				return TRUE;
			}
		}
	}
	err(CH_PLANING_PREX"channel %d not found in preferred list of radio %d",channel,radio->band);
	return FALSE;
}
/**
* @brief Fn to execute channel planning
* @param ctx own 1905 global
* @param dev target dev
*/

u8 ch_planning_find_max_pref_index(
	struct own_1905_device *ctx,
	struct _1905_map_device * _1905_device,
	struct ch_prefer_lib *ch_prefer)
{
	//aim is to keep the BW of device same ( so choose opclass accordingly)

	u8 max_pref_index = 0, j = 0;
	u8 max_opclass_pref = 0;
	u8 curr_opclass_pref = 0;
	u8 curr_bw = BW_20, max_bw = BW_20, radio_max_bw = BW_20;
	struct _1905_map_device * _1905_dev = NULL;

	struct radio_info_db *radio = NULL;

	if(_1905_device == NULL)
		_1905_dev = topo_srv_get_1905_device(ctx,NULL);
	else
		_1905_dev = _1905_device;

	radio = topo_srv_get_radio(_1905_dev, ch_prefer->identifier);
	if(!radio){
		err(CH_PLANING_PREX"radio is NULL for id("MACSTR")", MAC2STR(ch_prefer->identifier));
	} else {
		err(CH_PLANING_PREX"("MACSTR") current op %d, ch %d",
			MAC2STR(ch_prefer->identifier), radio->operating_class, radio->channel[0]);
	}
	radio_max_bw = ch_planning_get_max_bw_1905dev_prefer(ctx, radio, _1905_dev);
	 for (j = 0; j < ch_prefer->op_class_num; j++) {
		if (ch_prefer->opinfo[j].perference == 0) {
			debug("Preference is ZERO for op(%u)", ch_prefer->opinfo[j].op_class);
			continue;
		}
		 curr_opclass_pref = ch_prefer->opinfo[j].perference;
		 if (curr_opclass_pref < max_opclass_pref) {
		 	continue;
		 } else {
			curr_bw = chan_mon_get_bw_from_op_class(ch_prefer->opinfo[j].op_class);
			if (radio) {
				if (curr_bw >= max_bw && curr_bw <= radio_max_bw) {
					max_opclass_pref = ch_prefer->opinfo[j].perference;
					max_pref_index = j;
					max_bw = curr_bw;
					debug(CH_PLANING_PREX"max_bw %d  op_class(%d) ch(%d) orignal_bw(%d), radio_max_bw(%u)",
						   max_bw, ch_prefer->opinfo[j].op_class, ch_prefer->opinfo[j].ch_list[0], radio->orignal_bw, radio_max_bw);
				}
			} else {
				if (curr_bw >= max_bw ) {
					max_opclass_pref = ch_prefer->opinfo[j].perference;
					max_pref_index = j;
					max_bw = curr_bw;
					debug(CH_PLANING_PREX"max_bw %d  op_class(%d) ch(%d).",
						   max_bw, ch_prefer->opinfo[j].op_class, ch_prefer->opinfo[j].ch_list[0]);
				}
			}
		 }
	 }
	 err(CH_PLANING_PREX"found new ch to set %d, opclass(%d)",
	 	ch_prefer->opinfo[max_pref_index].ch_list[0],
	 	ch_prefer->opinfo[max_pref_index].op_class);
	return max_pref_index;
}
void ch_planning_trigger_net_opt_post_ch_plan(
	struct own_1905_device *ctx)
{
	struct _1905_map_device *dev = NULL, *t_dev = NULL;
	u8 pending = 0;
	u8 count = 0;
	SLIST_FOREACH_SAFE(dev, &(ctx->_1905_dev_head), next_1905_device, t_dev) {
		debug(CH_PLANING_PREX"dev role %d dev->in_network %d,dev->channel_planning_completed %d ",
			dev->device_role,
			dev->in_network,
			dev->channel_planning_completed);
		if (dev->in_network &&
			!dev->channel_planning_completed){
				pending = 1;
				break;
			}
	}
	if(pending == 0) {
		count = get_net_opt_dev_count((struct mapd_global *)ctx->back_ptr);
		if (count > 1) {
			err(CH_PLANING_PREX"all dev R1 channel planning is complete , safe to run network opt");
				eloop_register_timeout(ctx->network_optimization.wait_time,
					0, trigger_net_opt, ctx, NULL);
		}

	}

}
#ifdef MAP_R2
void ch_planning_send_select(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned char ch_prefer_count,
	struct ch_prefer_lib *ch_prefer)
{
	struct mapd_global *global = ctx->back_ptr;
	struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(ctx, NULL);
#ifdef MAP_R4_SPT
	struct ap_spt_reuse_req *spt_roles = NULL;
	unsigned char spt_role_len = 0;
#endif
	dev->ch_sel_req_given = 1;
	if (own_1905_node != dev) {
		err(CH_PLANING_PREX"send ch sel req to agent("MACSTR")\n",
			MAC2STR(dev->_1905_info.al_mac_addr));
		map_1905_Send_Channel_Selection_Request_Message(
			global->_1905_ctrl,
			(char *)dev->_1905_info.al_mac_addr,
			ch_prefer_count,
			ch_prefer,
			0,
			NULL
#ifdef MAP_R4_SPT
			,spt_role_len,
			spt_roles
#endif
		);
		eloop_register_timeout(5, 0, ch_planning_timeout_handler,
			ctx,dev);
	} else {
		err(CH_PLANING_PREX"set chnl on cont\n");
		struct channel_setting *setting = NULL;
		int i = 0, j = 0;
		unsigned char count = 0;
		setting = os_zalloc(512);
		if(!setting){
			err(CH_PLANING_PREX"alloc fail for setting");
			return;
		}

		setting->ch_set_num = 0;
		count = ch_prefer_count;
		u8 max_pref_index = ch_planning_find_max_pref_index(ctx, NULL, ch_prefer);
		while (i < count) {
			setting->ch_set_num++;
			setting->chinfo[j].channel = ch_prefer[i].opinfo[max_pref_index].ch_list[0];
			setting->chinfo[j].op_class = ch_prefer[i].opinfo[max_pref_index].op_class;
			os_memcpy(setting->chinfo[j].identifier, ch_prefer[i].identifier, ETH_ALEN);
			setting->chinfo[j].reason_code = ch_prefer[i].opinfo[max_pref_index].reason;
			err(CH_PLANING_PREX"Rsn code: %d op %d ch %d  prefer=%d\n", ch_prefer[i].opinfo[max_pref_index].reason,
				ch_prefer[i].opinfo[max_pref_index].op_class,
				ch_prefer[i].opinfo[max_pref_index].ch_list[0],
				ch_prefer[i].opinfo[max_pref_index].perference);

			/* Using i+1 index for storing primary CH info at setting->chinfo */
			if ((ch_prefer[i].opinfo[max_pref_index].op_class == 128)
					|| ch_prefer[i].opinfo[max_pref_index].op_class == 129
#ifdef MAP_320BW
					|| ch_prefer[i].opinfo[max_pref_index].op_class == 137
#endif
			) {
				unsigned char next_channel;

				next_channel = ch_prefer[i].opinfo[max_pref_index+1].ch_list[0];
				if (is_valid_primary_ch_80M_160M(next_channel, ch_prefer[i].opinfo[max_pref_index].ch_list[0],
										ch_prefer[i].opinfo[max_pref_index].op_class)) {
					setting->chinfo[j+1].channel = next_channel;
					setting->chinfo[j+1].op_class = ch_prefer[i].opinfo[max_pref_index+1].op_class;
					os_memcpy(setting->chinfo[j+1].identifier, ch_prefer[i].identifier, ETH_ALEN);
					setting->chinfo[j+1].reason_code = ch_prefer[i].opinfo[max_pref_index+1].reason;
					err(CH_PLANING_PREX"Primary ch Rsn code: %d op %d ch %d pref=%d\n",
						ch_prefer[i].opinfo[max_pref_index+1].reason,
						ch_prefer[i].opinfo[max_pref_index+1].op_class,
						ch_prefer[i].opinfo[max_pref_index+1].ch_list[0],
						ch_prefer[i].opinfo[max_pref_index+1].perference);
					/* Since we just added one extra entry, increment the index by 1 to make sure it will not get overwritten in next iteration */
					i++;
					setting->ch_set_num++;
					count++;
				}
			}
			i++;
			wlanif_issue_wapp_command(ctx->back_ptr, WAPP_USER_SET_CHANNEL_SETTING, 0,
				NULL, NULL, setting, 512, 0, 0, 0);
			os_memset((u8 *)setting, '\0', 512);
		}

		os_free(setting);
		dev->channel_planning_completed = TRUE;
		ctx->Restart_ch_planning_radar = 0;
		ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
		ctx->ch_planning.current_ch_planning_dev = NULL;
		if(ctx->ch_planning_R2.ch_plan_enable == TRUE)
			ch_planning_handle_ch_selection_rsp(ctx,dev);
		else
			ch_planning_trigger_net_opt_post_ch_plan(ctx);
	}
}
#endif


signed int ch_planning_select_channel_post_radar(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
	struct prefered_ch_cb *prefered_channel = NULL, *t_pref_ch = NULL;
	struct prefered_ch_cb *current_prefered_channel = NULL, *t_curr_pref_ch = NULL;
	struct prefered_ch_cb *new_prefered_channel = NULL;
	struct prefered_ch_cb *best_prefered_channel_dfs = NULL;
	struct prefered_ch_cb *best_prefered_channel_non_dfs = NULL;
	struct prefered_ch_radio_info_db *prefered_ch_radio = NULL, *t_pref_ch_radio = NULL;
	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	unsigned char centre_freq_ch = 0;
	unsigned char num_1905_devs = 0;
	struct ch_distribution_cb *ch_distribution = NULL;
	struct _1905_map_device *cnt_dev = NULL;
	struct radio_info_db *cnt_radio = NULL;
	struct prefered_ch_cb *cont_prefered_channel = NULL, *t_cont_pref_ch = NULL;

	if (!radio) {
		err(CH_PLANING_PREX"Radio is NULL");
		return -1;
	}

	if (radio->channel[0] == 0)
	{
		err(CH_PLANING_PREX"Operating channel not known yet\n");
		return -1;
	}
	if (radio->chan_preferance.op_class_num == 0)
	{
		err(CH_PLANING_PREX"Channel preference not known yet\n");
		return -1;
	}


	if(radio->channel[0] < 14)
		return -1;


	ch_distribution = &ch_planning->ch_ditribution_5g;

	num_1905_devs = topo_srv_get_1905_dev_count(ctx);


	/*Find current controller channel if channel planning already completed for controller*/
	if (dev->device_role == DEVICE_ROLE_AGENT) {
		cnt_dev = topo_srv_get_1905_device(ctx, NULL);

		if (cnt_dev && cnt_dev->channel_planning_completed){
			cnt_radio = topo_srv_get_radio_by_band(cnt_dev, radio->channel[0]);
			if (cnt_radio){
				SLIST_FOREACH_SAFE(cont_prefered_channel,
					&(ch_distribution->first_prefered_ch),
					next_prefered_ch, t_cont_pref_ch)
				{

					if (cont_prefered_channel->ch_num == cnt_radio->prev_channel)
						break;
				}
			}
		}
	}




	SLIST_FOREACH_SAFE(current_prefered_channel,
		&(ch_distribution->first_prefered_ch),
		next_prefered_ch, t_curr_pref_ch)
	{
		if (current_prefered_channel->ch_num == radio->channel[0])
			break;
	}

	if (current_prefered_channel == NULL)
		return -1;

	if (!ch_planning_check_channel_operable_wrapper(ctx, current_prefered_channel->ch_num)) {
		current_prefered_channel->ch_score = -1;
	}


	SLIST_FOREACH_SAFE(prefered_channel,
			&(ch_distribution->first_prefered_ch),
			next_prefered_ch, t_pref_ch) {
			if (!ch_planning_check_channel_operable_wrapper(ctx, prefered_channel->ch_num)) {
				prefered_channel->ch_score = -1;
			}
			/*Skip if not common for all 1905 devices in the network*/
			if (prefered_channel->radio_count != num_1905_devs){
				debug(CH_PLANING_PREX"Skip Ch num:%d,radio cnt:%d,1905_dev:%d", prefered_channel->ch_num,
					prefered_channel->radio_count,
					num_1905_devs);
				continue;
			}
			if (prefered_channel->ch_num == 50 ||
				prefered_channel->ch_num == 114 ||
				prefered_channel->ch_num == 163)
			{
				debug(CH_PLANING_PREX"160 Mhz Channel, skip %d\n",
					prefered_channel->ch_num);
				continue;
			}
			centre_freq_ch = ch_planning_get_centre_freq_ch(prefered_channel->ch_num, radio->operating_class);
			if (prefered_channel->ch_num == centre_freq_ch)
			{
				debug(CH_PLANING_PREX"Not suitable candidate as  %d is a centre freq in 80Mhz\n",
					prefered_channel->ch_num);
				continue;
			}
			if (radio->operating_class == 129) {
				centre_freq_ch = ch_planning_get_centre_freq_ch(prefered_channel->ch_num, 128);
				if (prefered_channel->ch_num == centre_freq_ch) {
					err(CH_PLANING_PREX"Not suitable candidate as  %d is a centre freq in 80/160Mhz",
						prefered_channel->ch_num);
					continue;
				}
			}

			if (current_prefered_channel == prefered_channel)
			{
				continue;
			}
			if (prefered_channel->ch_score == -1)
			{
				err(CH_PLANING_PREX"Continue as ch %d is not operable\n",
					prefered_channel->ch_num);
				continue;
			}


			if (current_prefered_channel->ch_score > prefered_channel->ch_score)
				continue;


			if (ch_planning_is_ch_dfs(ctx, prefered_channel->ch_num)) {
				SLIST_FOREACH_SAFE(prefered_ch_radio,
					&(prefered_channel->first_radio),
					next_pref_ch_radio, t_pref_ch_radio) {
					if ((prefered_ch_radio->radio == radio))
					{
						if (radio->operating_channel == NULL ||
							radio->chan_preferance.op_class_num == 0) {
							err(CH_PLANING_PREX"Channel Distribution not complete yet\n");
							return -1;
						}
						if (best_prefered_channel_dfs == NULL ||
							best_prefered_channel_dfs->ch_score < prefered_channel->ch_score)
						{
							debug(CH_PLANING_PREX"Radio ch %d, score = %x,possible better channel %d, score = %x\n",
								radio->channel[0], current_prefered_channel->ch_score,
								prefered_channel->ch_num,
								prefered_channel->ch_score);

							best_prefered_channel_dfs = prefered_channel;

							//return (signed int)(prefered_channel->ch_num);
						}
						break;
					}
				}

			}else {
				t_pref_ch_radio = NULL;
				SLIST_FOREACH_SAFE(prefered_ch_radio,
					&(prefered_channel->first_radio),
					next_pref_ch_radio, t_pref_ch_radio) {
					if ((prefered_ch_radio->radio == radio))
					{
						if (radio->operating_channel == NULL ||
							radio->chan_preferance.op_class_num == 0) {
							err(CH_PLANING_PREX"Channel Distribution not complete yet\n");
							return -1;
						}
						if (best_prefered_channel_non_dfs == NULL ||
							best_prefered_channel_non_dfs->ch_score < prefered_channel->ch_score)
						{
							best_prefered_channel_non_dfs = prefered_channel;
							debug(CH_PLANING_PREX"Radio ch %d, score = %x,possible better channel %d, score = %x\n",
								radio->channel[0], current_prefered_channel->ch_score,
								prefered_channel->ch_num,
								best_prefered_channel_non_dfs->ch_score);
							//return (signed int)(prefered_channel->ch_num);
						}
						break;
					}
				}
			}
		}

		if (best_prefered_channel_non_dfs) {
			new_prefered_channel = best_prefered_channel_non_dfs;
			debug(CH_PLANING_PREX"Non dfs best_prefered_channel_non_dfs:%d", new_prefered_channel->ch_num);

			if (new_prefered_channel &&
				(new_prefered_channel->ch_score > current_prefered_channel->ch_score)) {
				debug(CH_PLANING_PREX"Best possible option = %d", new_prefered_channel->ch_num);
				if (dev->device_role == DEVICE_ROLE_CONTROLLER) {
					cnt_dev = topo_srv_get_1905_device(ctx, NULL);
					if (cnt_dev) {
						cnt_radio = topo_srv_get_radio_by_band(cnt_dev, radio->channel[0]);
						if (cnt_radio)
							cnt_radio->prev_channel = new_prefered_channel->ch_num;
					}
				}
				return new_prefered_channel->ch_num;
			} else if (new_prefered_channel &&
				(new_prefered_channel->ch_score == current_prefered_channel->ch_score) &&
				dev->device_role == DEVICE_ROLE_AGENT) {
				cnt_dev = topo_srv_get_1905_device(ctx, NULL);
				if (cnt_dev) {
					cnt_radio = topo_srv_get_radio_by_band(cnt_dev, radio->channel[0]);
					u8 channel = 0;
					if (cnt_radio) {
						if (cnt_radio->channel[0] == radio->channel[0]){
							err(CH_PLANING_PREX"Radio already on best possible channel %d\n", cnt_radio->channel[0]);
							channel = 0;
					} else if  ((cnt_radio->channel[0] != cnt_radio->prev_channel) &&
						(ch_planning_check_channel_operable_wrapper(ctx, cnt_radio->prev_channel))) {
						channel = cnt_radio->prev_channel;
					} else {
							debug(CH_PLANING_PREX"New radio ch: %u", cnt_radio->channel[0]);
							channel = cnt_radio->channel[0];
						}
					}
					return channel;
				}
			} else {
			   err(CH_PLANING_PREX"Radio already on best possible ch\n");
			   return 0;
			}
		}
		else if (!best_prefered_channel_non_dfs && cont_prefered_channel
			&& !ch_planning_is_ch_dfs(ctx, cont_prefered_channel->ch_num)
			&& cont_prefered_channel->radio_count == num_1905_devs
			&& cnt_radio
			&& dev->device_role == DEVICE_ROLE_AGENT) {
			/* if better common non dfs channel doesnt exist then check if
			controller already has selected common non dfs channel*/

			if(cnt_radio->channel[0] == radio->channel[0]){
				err(CH_PLANING_PREX"Radio already on best possible ch %d\n", cnt_radio->channel[0]);
				return 0;
			} else if ((cnt_radio->channel[0] != cnt_radio->prev_channel) &&
					(ch_planning_check_channel_operable_wrapper(ctx, cnt_radio->prev_channel))) {
				return cnt_radio->prev_channel;
			} else {

				err(CH_PLANING_PREX"New radio ch: %u", cnt_radio->channel[0]);

				return cnt_radio->channel[0];
			}

		}
		else if (best_prefered_channel_dfs) {
/*If common non dfs channel doesnt exist then find common dfs channel with highest score*/
			new_prefered_channel = best_prefered_channel_dfs;
			debug(CH_PLANING_PREX"dfs best_prefered_channel_dfs:%d", new_prefered_channel->ch_num);

			if (new_prefered_channel &&
				(new_prefered_channel->ch_score > current_prefered_channel->ch_score)) {
				if (dev->device_role == DEVICE_ROLE_CONTROLLER) {
					cnt_dev = topo_srv_get_1905_device(ctx, NULL);
					if (cnt_dev) {
						cnt_radio = topo_srv_get_radio_by_band(cnt_dev, radio->channel[0]);
						if (cnt_radio)
							cnt_radio->prev_channel = new_prefered_channel->ch_num;
					}
				}
				err(CH_PLANING_PREX"Best possible option = %d", new_prefered_channel->ch_num);
				return new_prefered_channel->ch_num;
			} else if (new_prefered_channel &&
				(new_prefered_channel->ch_score == current_prefered_channel->ch_score) &&
				dev->device_role == DEVICE_ROLE_AGENT) {
				cnt_dev = topo_srv_get_1905_device(ctx, NULL);
				if (cnt_dev) {
					cnt_radio = topo_srv_get_radio_by_band(cnt_dev, radio->channel[0]);
					u8 channel = 0;
					if (cnt_radio) {
						if(cnt_radio->channel[0] == radio->channel[0]){
							err(CH_PLANING_PREX"Radio already on best possible ch %d\n", cnt_radio->channel[0]);
							channel = 0;
					} else if ((cnt_radio->channel[0] != cnt_radio->prev_channel) &&
						(ch_planning_check_channel_operable_wrapper(ctx, cnt_radio->prev_channel))) {
						channel = cnt_radio->prev_channel;
					} else{
						err(CH_PLANING_PREX"New radio ch: %u", cnt_radio->channel[0]);
						channel = cnt_radio->channel[0];
						}
					}
					return channel;
				}
			} else {
			   err(CH_PLANING_PREX"Radio already on best possible ch\n");
			   return 0;
			}
		}
		else if (!best_prefered_channel_dfs && cont_prefered_channel
			&& ch_planning_is_ch_dfs(ctx, cont_prefered_channel->ch_num)
			&& cont_prefered_channel->radio_count == num_1905_devs
			&& cnt_radio
			&& dev->device_role == DEVICE_ROLE_AGENT) {
			/* if better common dfs channel doesnt exist then check if
			controller already has selected common dfs channel*/

			if(cnt_radio->channel[0] == radio->channel[0]){
				err(CH_PLANING_PREX"Radio already on best possible ch %d\n", cnt_radio->channel[0]);
				return 0;
			} else if ((cnt_radio->channel[0] != cnt_radio->prev_channel) &&
					(ch_planning_check_channel_operable_wrapper(ctx, cnt_radio->prev_channel))) {
				return cnt_radio->prev_channel;
			} else{
				err(CH_PLANING_PREX"New radio ch: %u", cnt_radio->channel[0]);
				return cnt_radio->channel[0];
			}


		}
	/*No common non dfs and no common dfs channel found*/
		else {
	/*choose non dfs channel with highest score that is supported by the 1905device
	    and if no non dfs channel present then choose dfs channel with highest score supported by the 1905device    */
	   	debug(CH_PLANING_PREX"No common non/dfs channel found");
		t_pref_ch = NULL;
		SLIST_FOREACH_SAFE(prefered_channel,
				&(ch_distribution->first_prefered_ch),
				next_prefered_ch, t_pref_ch) {
				if (!ch_planning_check_channel_operable_wrapper(ctx, prefered_channel->ch_num)) {
					prefered_channel->ch_score = -1;
				}

				if (!ch_planning_check_chan_oper_for_dev(ctx, dev, prefered_channel->ch_num)) {
					err(CH_PLANING_PREX"Channel %d not supported by this radio",prefered_channel->ch_num);
					continue;
				}
				if (prefered_channel->ch_num == 50 ||
					prefered_channel->ch_num == 114 ||
					prefered_channel->ch_num == 163)
				{
					debug(CH_PLANING_PREX"160 Mhz Channel, skip %d\n",
						prefered_channel->ch_num);
					continue;
				}
			centre_freq_ch = ch_planning_get_centre_freq_ch(prefered_channel->ch_num, radio->operating_class);
			if (prefered_channel->ch_num == centre_freq_ch)
			{
				debug(CH_PLANING_PREX"Not suitable candidate as  %d is a centre freq in 80Mhz\n",
					prefered_channel->ch_num);
				continue;
			}
			if (radio->operating_class == 129) {
				centre_freq_ch = ch_planning_get_centre_freq_ch(prefered_channel->ch_num, 128);
				if (prefered_channel->ch_num == centre_freq_ch) {
					err(CH_PLANING_PREX"Not suitable candidate as  %d is a centre freq in 80/160Mhz",
						prefered_channel->ch_num);
					continue;
				}
			}

			if (current_prefered_channel == prefered_channel)
			{
				continue;
			}
			if (prefered_channel->ch_score == -1)
			{
				err(CH_PLANING_PREX"Continue as ch %d is not operable\n",
					prefered_channel->ch_num);
				continue;
			}

			if (ch_planning_is_ch_dfs(ctx, prefered_channel->ch_num)) {
				t_pref_ch_radio = NULL;
				SLIST_FOREACH_SAFE(prefered_ch_radio,
					&(prefered_channel->first_radio),
					next_pref_ch_radio, t_pref_ch_radio) {
					if ((prefered_ch_radio->radio == radio))
					{
						if (radio->operating_channel == NULL ||
							radio->chan_preferance.op_class_num == 0) {
							err(CH_PLANING_PREX"Channel Distribution not complete yet\n");
							return -1;
						}
						if (best_prefered_channel_dfs == NULL ||
							best_prefered_channel_dfs->ch_score < prefered_channel->ch_score)
						{
							best_prefered_channel_dfs = prefered_channel;
							err(CH_PLANING_PREX"Radio ch %d, score = %x,possible better channel %d, score = %x\n",
								radio->channel[0], current_prefered_channel->ch_score,
								prefered_channel->ch_num,
								best_prefered_channel_dfs->ch_score);
							//return (signed int)(prefered_channel->ch_num);
						}
						break;
					}
				}
			}else {
				t_pref_ch_radio = NULL;
				SLIST_FOREACH_SAFE(prefered_ch_radio,
					&(prefered_channel->first_radio),
					next_pref_ch_radio, t_pref_ch_radio) {
					if ((prefered_ch_radio->radio == radio))
					{
						if (radio->operating_channel == NULL ||
							radio->chan_preferance.op_class_num == 0) {
							err(CH_PLANING_PREX"Channel Distribution not complete yet\n");
							return -1;
						}
						if (best_prefered_channel_non_dfs == NULL ||
							best_prefered_channel_non_dfs->ch_score < prefered_channel->ch_score)
						{
							best_prefered_channel_non_dfs = prefered_channel;
							err(CH_PLANING_PREX"Radio ch %d, score = %x,possible better channel %d, score = %x\n",
								radio->channel[0], current_prefered_channel->ch_score,
								prefered_channel->ch_num,
								best_prefered_channel_non_dfs->ch_score);
							//return (signed int)(prefered_channel->ch_num);
						}
						break;
					}
				}
			}
		}
		if (best_prefered_channel_non_dfs) {
			new_prefered_channel = best_prefered_channel_non_dfs;
			debug(CH_PLANING_PREX"best Non dfs best_prefered_channel_non_dfs:%d",new_prefered_channel->ch_num);

			if (new_prefered_channel &&
				(new_prefered_channel->ch_score > current_prefered_channel->ch_score)) {

				if (dev->device_role == DEVICE_ROLE_CONTROLLER) {
					cnt_dev = topo_srv_get_1905_device(ctx, NULL);
					if (cnt_dev) {
						cnt_radio = topo_srv_get_radio_by_band(cnt_dev, radio->channel[0]);
						if (cnt_radio)
							cnt_radio->prev_channel = new_prefered_channel->ch_num;
					}
				}
				err(CH_PLANING_PREX"Best possible option = %d", new_prefered_channel->ch_num);
				return new_prefered_channel->ch_num;
			} else {
			   err(CH_PLANING_PREX"Radio already on best possible ch\n");

			}
		}
		else if (best_prefered_channel_dfs) {
			new_prefered_channel = best_prefered_channel_dfs;
			debug(CH_PLANING_PREX"best dfs best_prefered_channel_dfs:%d",new_prefered_channel->ch_num);

			if (new_prefered_channel &&
				(new_prefered_channel->ch_score > current_prefered_channel->ch_score)) {
				if (dev->device_role == DEVICE_ROLE_CONTROLLER) {
					cnt_dev = topo_srv_get_1905_device(ctx, NULL);
					if (cnt_dev) {
						cnt_radio = topo_srv_get_radio_by_band(cnt_dev, radio->channel[0]);
						if (cnt_radio)
							cnt_radio->prev_channel = new_prefered_channel->ch_num;
					}
				}
				err(CH_PLANING_PREX"Best possible option = %d", new_prefered_channel->ch_num);
				return new_prefered_channel->ch_num;
			} else {
			   err(CH_PLANING_PREX"Radio already on best possible ch\n");

			}
		}


	}

	return 0;

}

#ifdef MAP_R2
signed int find_controller_channel_from_agent_pref(struct own_1905_device *ctx, struct radio_info_db *tmp_radio)
{
	int i = 0;
	struct prefer_info_db *prefer = NULL, *prefer_tmp = NULL;
	struct radio_info_db *own_radio;
	struct radio_ch_prefer *ch_prefer;

	ch_prefer = &tmp_radio->chan_preferance;

	if (tmp_radio->chan_preferance.op_class_num == 0) {
		err(CH_PLANING_PREX"Channel Preference not available yet\n");
		return -1;
	}

	prefer = SLIST_FIRST(&ch_prefer->prefer_info_head);
	while (prefer) {
		prefer_tmp = SLIST_NEXT(prefer, prefer_info_entry);
		for (i = 0; i < prefer->ch_num; i++) {
			own_radio = topo_srv_get_radio_by_band(topo_srv_get_1905_device(ctx, NULL), prefer->ch_list[i]);
			if (own_radio) {
				err("Own channel on corresponding radio : %d", own_radio->channel[0]);
				return own_radio->channel[0];
			}
		}
		prefer = prefer_tmp;
	}
	return -1;
}
#endif

//! channel array in this function as input helps to
//! helps to differentiate between convergent and
//! divergent ch planning.
//! for divergent planning, chnnl array will comprise of non zero values
//! for convergent these value will be all zeros,
//! best channel will be picked internally
void ch_planning_exec(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned int channel[])
{

#ifdef MAP_R2
	ch_planning_exec_R2(ctx, dev, channel);
#else
	struct radio_info_db *radio, *t_radio = NULL;
	unsigned char channel_planning_req = FALSE,avoid_ChPlanning_radio = FALSE;
	signed int best_channel = 0;
	unsigned int force_channel = 0;
	unsigned char ch_prefer_count = 0;
	struct ch_prefer_lib *ch_prefer;
	struct mapd_global *global = ctx->back_ptr;
	struct bh_link_entry *bh_entry = NULL, *t_bh_entry = NULL;
	u8	max_pref_index = 0;
	size_t size = 512; 
	ch_prefer = os_zalloc(sizeof(struct ch_prefer_lib) * 3);
	SLIST_FOREACH_SAFE(radio, &(dev->first_radio), next_radio, t_radio) {
		if (((dev->device_role == DEVICE_ROLE_CONTROLLER) || (dev->device_role == DEVICE_ROLE_CONTRAGENT)) &&
			(global->dev.ThirdPartyConnection) && (!ctx->Restart_ch_planning_radar)) {
			avoid_ChPlanning_radio = FALSE;
			SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, t_bh_entry) {
				if((bh_entry->bh_channel == radio->channel[0] &&
					bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED) ||
 					(ctx->current_bh_state != BH_STATE_WIFI_LINKUP && ctx->bh_config_count > 0 )) {
					//! it is a divergent ch planning case and current radio
					//! is a controller radio with CLI connected to
					//! third party AP. channel planning cannot be executed on it
					avoid_ChPlanning_radio = TRUE;
				}
			}
			if(avoid_ChPlanning_radio) {
				err(CH_PLANING_PREX"Avoid Ch planning on this radio");
				continue;
			}
		}
		//! channel planning is executed only on the radio, whose APCLI interface
		//! is not connected to uplink device
		if (!radio->uplink_bh_present){
			//! convergent channel planing case
			if(!(channel[0]||channel[1]||channel[2])){
				//! find best possible channel for the radio
				best_channel = ch_planning_select_best_channel(ctx, radio, dev);
				err(CH_PLANING_PREX"bestchannel %d", best_channel);
				if(best_channel < 0){
					os_free(ch_prefer);
					return;
				} else if (best_channel > 0 && ch_planning_is_ch_dfs(ctx, best_channel)) {
					start_netopt_timer(ctx, best_channel);
				}
			}
			//! divergent channel planning cases
			else if(channel_validInList(ctx,radio,channel[0])) {
					force_channel = channel[0];
					global->dev.ch_planning.ch_planning_enabled = 0;
			}
			else if(channel_validInList(ctx,radio,channel[1])) {
					force_channel = channel[1];
					global->dev.ch_planning.ch_planning_enabled = 0;
			}
			else if(channel_validInList(ctx,radio,channel[2])) {
					force_channel = channel[3];
					global->dev.ch_planning.ch_planning_enabled = 0;
			}
			if (best_channel > 0) {
				//! prepare channel selection request data for current radio
				ch_planning_ch_selection_prefer_data(ctx, best_channel,
					radio, &ch_prefer[ch_prefer_count]);
				ch_prefer_count++;
				channel_planning_req = TRUE;
			}
			if (force_channel>0) {
				//! prepare channel selection request data for current radio

				ch_planning_ch_selection_prefer_data(ctx, force_channel,
					radio, &ch_prefer[ch_prefer_count]);
				ch_prefer_count++;
				channel_planning_req = TRUE;
			}
		}
	}//radio loop ends here
	err(CH_PLANING_PREX"channel_planning_req(%u) best_channel(%d), force_channel(%u)",
		channel_planning_req, best_channel, force_channel);
	if (channel_planning_req)
	{
		ctx->ch_planning.ch_planning_state =
			CHANNEL_SELECTION_REQ_SENT;
		ctx->ch_planning.current_ch_planning_dev = dev;

		struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(ctx, NULL);
		if (own_1905_node != dev) {
			err(CH_PLANING_PREX"change on agent "MACSTR, MAC2STR(dev->_1905_info.al_mac_addr));
			map_1905_Send_Channel_Selection_Request_Message(
				global->_1905_ctrl,
				(char *)dev->_1905_info.al_mac_addr,
				ch_prefer_count,
				ch_prefer,
				0,
				NULL);
			eloop_register_timeout(3, 0, ch_planning_timeout_handler,
				ctx,dev);
		} else {
			err(CH_PLANING_PREX"Channel Planning on cont\n");
			struct channel_setting *setting = NULL;
			int i = 0;
			setting = (struct channel_setting *)os_zalloc(size);
			if (setting == NULL) {
				if (ch_prefer)
					os_free(ch_prefer);
				return;
			}

			setting->ch_set_num = ch_prefer_count;
			for (i = 0; i < setting->ch_set_num; i++) {
				max_pref_index = ch_planning_find_max_pref_index(ctx, NULL, ch_prefer);
				setting->chinfo[i].channel = ch_prefer[i].opinfo[max_pref_index].ch_list[0];
				setting->chinfo[i].op_class = ch_prefer[i].opinfo[max_pref_index].op_class;
				os_memcpy(setting->chinfo[i].identifier, ch_prefer[i].identifier, ETH_ALEN);
				setting->chinfo[i].reason_code = ch_prefer[i].opinfo[max_pref_index].reason;
				err(CH_PLANING_PREX"Reason code: %d opclass %d channel %d",
					ch_prefer[i].opinfo[max_pref_index].reason,
					ch_prefer[i].opinfo[max_pref_index].op_class,
					ch_prefer[i].opinfo[max_pref_index].ch_list[0]);

				/* Using i+1 index for primary CH info for setting->chinfo */
				if ((i < setting->ch_set_num-1) && ((ch_prefer[i].opinfo[max_pref_index].op_class == 128)
						|| (ch_prefer[i].opinfo[max_pref_index].op_class == 129)
#ifdef MAP_320BW
						|| (ch_prefer[i].opinfo[max_pref_index].op_class == 137)
#endif
				)) {
                     setting->chinfo[i+1].channel = ch_prefer[i].opinfo[max_pref_index+1].ch_list[0];
                                	setting->chinfo[i+1].op_class = ch_prefer[i].opinfo[max_pref_index+1].op_class;
                                	os_memcpy(setting->chinfo[i+1].identifier, ch_prefer[i].identifier, ETH_ALEN);
                                	setting->chinfo[i+1].reason_code = ch_prefer[i].opinfo[max_pref_index+1].reason;
                                	err(CH_PLANING_PREX"Primary CH Reason code: %d opclass %d channel %d",
                                        	ch_prefer[i].opinfo[max_pref_index+1].reason,
                                        	ch_prefer[i].opinfo[max_pref_index+1].op_class,
                                        	ch_prefer[i].opinfo[max_pref_index+1].ch_list[0]);
					i++;
					setting->ch_set_num++;
				}

			}
			wlanif_issue_wapp_command(ctx->back_ptr, WAPP_USER_SET_CHANNEL_SETTING, 0,
				NULL, NULL, setting, 512, 0, 0, 0);
			os_free(setting);

			dev->channel_planning_completed = TRUE;
			ctx->Restart_ch_planning_radar = 0;
			ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
			ctx->ch_planning.current_ch_planning_dev = NULL;
			ch_planning_trigger_net_opt_post_ch_plan(ctx);
		}

	} else if (best_channel == 0) {
		struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(ctx, NULL);

		dev->channel_planning_completed = TRUE;
		ctx->Restart_ch_planning_radar = 0;
		if (own_1905_node != dev)
			eloop_cancel_timeout(ch_planning_timeout_handler, global, dev);
		ch_planning_trigger_net_opt_post_ch_plan(ctx);
	}
	os_free(ch_prefer);
#endif
}
#ifdef MAP_R2
void ch_planning_exec_R2(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned int channel[])
{
	struct radio_info_db *radio, *t_radio = NULL;
	unsigned char channel_planning_req = FALSE,avoid_ChPlanning_radio = FALSE;
	signed int best_channel = 0, best_channel_2g_tmp = 0;
	int best_channel_5g = 0, best_channel_2g = 0;
	unsigned int force_channel = 0;
	unsigned char ch_prefer_count = 0;
	struct ch_prefer_lib *ch_prefer;
	struct mapd_global *global = ctx->back_ptr;
	struct bh_link_entry *bh_entry = NULL, *t_bh_entry = NULL;
#ifndef MAP_6E_SUPPORT
	struct mapd_radio_info *own_radio = NULL;
#endif
	u8 dfs_status = DIRECT_SWITCH_CAC;
	u8 channel_change_cond_2G = 0;
	ch_prefer = os_zalloc(sizeof(struct ch_prefer_lib) * 3);
	SLIST_FOREACH_SAFE(radio, &(dev->first_radio), next_radio, t_radio) {
		if(ctx->ch_planning_R2.ch_plan_enable == TRUE) {
			if ((dev->device_role == DEVICE_ROLE_CONTROLLER)
			|| (dev->device_role == DEVICE_ROLE_CONTRAGENT)) {
#ifndef MAP_6E_SUPPORT
				own_radio = mapd_get_radio_from_channel(global,radio->channel[0]);
#ifndef SINGLE_BAND_SUPPORT
				if (own_radio == NULL) {
					err(CH_PLANING_PREX"Error!! own_radio(%u) should not be NULL", radio->channel[0]);
					continue;
				}
				debug(CH_PLANING_PREX"channel %d, bootup_run %d", radio->channel[0], own_radio->bootup_run);
				if(own_radio->bootup_run == BOOTUP_SCAN_NEEDED && ctx->Restart_ch_planning_radar_on_agent == 0)
#else
				if (own_radio == NULL)
					err(CH_PLANING_PREX"No own radio on channel %d", radio->channel[0]);
				else
					debug(CH_PLANING_PREX"channel %d, bootup_run %d", radio->channel[0], own_radio->bootup_run);
				if (own_radio && own_radio->bootup_run == BOOTUP_SCAN_NEEDED
						&& ctx->Restart_ch_planning_radar_on_agent == 0)
#endif
#else
				if (ctx->ch_planning_R2.bootup_scanstatus[radio->band].bootup_run == BOOTUP_SCAN_NEEDED
					&& ctx->Restart_ch_planning_radar_on_agent == 0)
#endif
					continue;
			}
			debug(CH_PLANING_PREX"radio->ch %d, state %d,force_channel %d",
				radio->channel[0], radio->dev_ch_plan_info.dev_ch_plan_state, force_channel);
			if(radio->dev_ch_plan_info.dev_ch_plan_state != CHPLAN_STATE_CH_CHANGE_TRIGGERED){
				continue;
			}
		}
		err(CH_PLANING_PREX"perform channel planning on radio(%d) on dev("MACSTR")",
			radio->channel[0], MAC2STR(dev->_1905_info.al_mac_addr));
			if (((dev->device_role == DEVICE_ROLE_CONTROLLER)
			|| (dev->device_role == DEVICE_ROLE_CONTRAGENT)) &&
			(global->dev.ThirdPartyConnection) && (!ctx->Restart_ch_planning_radar)) {
			avoid_ChPlanning_radio = FALSE;
			SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, t_bh_entry) {
				if((bh_entry->bh_channel == radio->channel[0] &&
					bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED) ||
					(ctx->current_bh_state != BH_STATE_WIFI_LINKUP && ctx->bh_config_count > 0 )) {
					//! it is a divergent ch planning case and current radio
					//! is a controller radio with CLI connected to
					//! third party AP. channel planning cannot be executed on it
					/*! if radar on agent, execute ch planning*/
					if (!ctx->Restart_ch_planning_radar_on_agent)
						avoid_ChPlanning_radio = TRUE;
				}
			}
			if(avoid_ChPlanning_radio) {
				err(CH_PLANING_PREX"ThirdPartyConnection!!! Avoid Ch planning on this radio");
				continue;
			}
		}
		//! channel planning is executed only on the radio, whose APCLI interface
		//! is not connected to uplink device
		if (!radio->uplink_bh_present){
			err(CH_PLANING_PREX"ch planning on fh radio(%d)", radio->channel[0]);
			//! convergent channel planing case
			if(!(channel[0] || channel[1] || channel[2])){
				//! find best possible channel for the radio
				if (ctx->Restart_ch_planning_radar_on_agent
						&& (radio->channel[0] > 14)) {
					best_channel = ch_planning_select_channel_post_radar(ctx, dev, radio);
					if (best_channel == radio->channel[0]) {
						info("since radio->channel[0] %d, best_channel %d make best channel =0",
								radio->channel[0], best_channel);
						best_channel = 0;
					}
				} else
				best_channel = ch_planning_select_best_channel(ctx, radio, dev);
					err(CH_PLANING_PREX"bestchannel %d", best_channel);
			}
			//! divergent channel planning cases
			else if(channel_validInList(ctx,radio,channel[0])) {
				force_channel = channel[0];
				global->dev.ch_planning.ch_planning_enabled = 0;
			}
			else if(channel_validInList(ctx,radio,channel[1])) {
				force_channel = channel[1];
				global->dev.ch_planning.ch_planning_enabled = 0;
			}
			else if(channel_validInList(ctx,radio,channel[2])) {
				force_channel = channel[2];
				global->dev.ch_planning.ch_planning_enabled = 0;
			}

			u8 dedicated_radio =0;
			if (best_channel > 0){
				dedicated_radio = ch_planning_check_controller_cac_cap(&global->dev, best_channel, DEDICATED_RADIO);
			} else if (force_channel > 0) {
				dedicated_radio = ch_planning_check_controller_cac_cap(&global->dev, force_channel, DEDICATED_RADIO);
			}
			global->dev.dedicated_radio = dedicated_radio;
			err(CH_PLANING_PREX"dedicated_radio for CAC on controller is %d", dedicated_radio);
			if((ctx->ch_planning_R2.ch_plan_enable == TRUE || (global->dev.dedicated_radio)) &&
				(best_channel > 14 || force_channel > 14)) {
				/*check if ch is DFS*/
				if(best_channel > 0) {
					ch_planning_check_old_cac_valid(global, best_channel);
					dfs_status = ch_planning_check_dfs_by_bw_wrapper(global, best_channel);
				} else if (force_channel > 0) {
					ch_planning_check_old_cac_valid(global, force_channel);
					dfs_status = ch_planning_check_dfs_by_bw_wrapper(global, force_channel);
				}
				if(dfs_status == CAC_ON_SELECT_DEV) {
					if (best_channel > 0)
						start_netopt_timer_R2(&global->dev, best_channel, dfs_status);
					else if (force_channel > 0)
						start_netopt_timer_R2(&global->dev, force_channel, dfs_status);

					if (ctx->force_ch_change == 0) {
						os_free(ch_prefer);
						return;
					} else {
						err(CH_PLANING_PREX"dfs cac to be checked, moving to other radio");
						continue;
					}
				} else {
					debug(CH_PLANING_PREX"dfs_status %d", dfs_status);
				}
			} else {
				dfs_status = NO_CAC;
				if(!(best_channel > MAX_2G_CH) && (best_channel >= MIN_2G_CH)) {
					err(CH_PLANING_PREX"setting channel_change_cond_2G = %d, best_channel %d", channel_change_cond_2G, best_channel);
					channel_change_cond_2G = 1;
				}
			}

			if (!global->dev.dedicated_radio && dfs_status == NO_CAC) {
				if (best_channel > 14 && ch_planning_is_ch_dfs(&global->dev, best_channel) &&
					(is_CAC_Success(global, best_channel) == 0)) {
						start_netopt_timer_R2(&global->dev, best_channel, dfs_status);
				} else if (force_channel > 14 && ch_planning_is_ch_dfs(&global->dev, force_channel) &&
					(is_CAC_Success(global, force_channel) == 0)) {
						start_netopt_timer_R2(&global->dev, force_channel, dfs_status);
				}
			}

			if (best_channel > 0) {
				//! prepare channel selection request data for current radio
				ch_planning_ch_selection_prefer_data(ctx, best_channel,
					radio, &ch_prefer[ch_prefer_count]);
				ch_prefer_count++;
				channel_planning_req = TRUE;
			}
			if (force_channel > 0) {
				//! prepare channel selection request data for current radio
				ch_planning_ch_selection_prefer_data(ctx, force_channel,
					radio, &ch_prefer[ch_prefer_count]);
				ch_prefer_count++;
				channel_planning_req = TRUE;
			}
		} else if (!(radio->channel[0] > MAX_2G_CH)) {
			best_channel_2g_tmp = ch_planning_select_best_channel(ctx, radio, dev);
			err(CH_PLANING_PREX"ch planning on bh radio(%d)set channel_change_cond_2G if(bestchannel > 0), bestchannel %d"
				, radio->channel[0], best_channel_2g_tmp);

			if (best_channel_2g_tmp > 0) {
				best_channel = best_channel_2g_tmp;
				//! prepare channel selection request data for current radio
				ch_planning_ch_selection_prefer_data(ctx, best_channel,
						radio, &ch_prefer[ch_prefer_count]);
				ch_prefer_count++;
				channel_planning_req = TRUE;
				channel_change_cond_2G = 1;
				dfs_status = NO_CAC;
			}
		} else {
			err(CH_PLANING_PREX"ch planning on bh radio(%d), ch update to happen via CSA, clean state",
				radio->channel[0]);
		}
		/*Best_channel is one variable we should have 2 channel for 2 radios.*/
		/*Support Radio needs to be extended please add new variable as per need.*/
		if(radio->channel[0] > MAX_2G_CH)
			best_channel_5g = best_channel;
		else
			best_channel_2g = best_channel;
	}//radio loop ends here
	err(CH_PLANING_PREX"channel_change_cond_2G %d, channel_planning_req %d, best_channel_2g %d, best_channel_5g %d force_channel %d dfs_status %u",
		channel_change_cond_2G, channel_planning_req, best_channel_2g, best_channel_5g, force_channel, dfs_status);
	if (channel_planning_req)
	{
		ctx->ch_planning.ch_planning_state = CHANNEL_SELECTION_REQ_SENT;
		ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
		ctx->ch_planning.current_ch_planning_dev = dev;
		if((dfs_status == NO_CAC) && (channel_change_cond_2G != 1))
		{
			err(CH_PLANING_PREX"since it is not DFS ch so jump directly.");
			ch_planning_send_select(ctx, dev, ch_prefer_count, ch_prefer);
			if(global->dev.force_ch_change) {
				global->dev.ch_planning.ch_planning_enabled = 0;
				global->dev.force_ch_change = 0;
			}
		} else if((dfs_status == NO_CAC) && (channel_change_cond_2G == 1)) {
			if(best_channel_2g > 0) {
				err(CH_PLANING_PREX"since it is 2G ch bestchannel %d so jump directly.", best_channel_2g);
				ch_planning_send_select_2G(ctx, dev, ch_prefer_count, ch_prefer, best_channel_2g);
			}
		} else if(dfs_status == DIRECT_SWITCH_CAC) {
			err(CH_PLANING_PREX"send select dfs");
			if(best_channel_5g > 0) {
				ch_planning_R2_send_select_dfs(ctx, dev, ch_prefer_count, ch_prefer, best_channel_5g);
				if (!(is_CAC_Success(global, best_channel_5g)))
					start_netopt_timer_R2(&global->dev, best_channel_5g, dfs_status);
			} else if(force_channel > 0) {
				err(CH_PLANING_PREX"global->dev.force_ch_change %d", global->dev.force_ch_change);
				ch_planning_R2_send_select_dfs(ctx, dev, ch_prefer_count, ch_prefer, force_channel);
				if(global->dev.force_ch_change) {
					global->dev.ch_planning.ch_planning_enabled = 0;
					global->dev.force_ch_change = 0;
				}
			}
		}
	} else if (best_channel_2g == 0 && best_channel_5g == 0){
		struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(ctx, NULL);

		if (own_1905_node != dev)
			eloop_cancel_timeout(ch_planning_timeout_handler, global, dev);

		dev->channel_planning_completed = TRUE;
		ctx->Restart_ch_planning_radar = 0;

		if (ctx->ch_planning_R2.ch_plan_enable == TRUE) {
			ch_planning_handle_ch_selection_rsp(ctx,dev);
		} else {
			ch_planning_trigger_net_opt_post_ch_plan(ctx);
		}
	}
	os_free(ch_prefer);
}
#endif

void ch_planning_show_ch_distribution(
	struct own_1905_device *ctx)
{
	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct ch_distribution_cb *ch_distribution = NULL;
	struct operating_ch_cb *operating_ch = NULL, *t_op_ch = NULL;
	struct prefered_ch_cb *prefered_ch = NULL, *t_pref_ch = NULL;
	struct prefered_ch_radio_info_db *prefered_ch_radio = NULL, *t_pref_ch_radio = NULL;
	struct radio_info_db *radio = NULL, *t_radio = NULL;

	if (ctx->device_role != DEVICE_ROLE_CONTROLLER) {
		err("only show this information at controller side");
		return;
	}

	err("*************channel planning distribution***********");
	always("Operating Channel distribution on 5G\n");
	ch_distribution = &ch_planning->ch_ditribution_5g;
	always("\tNumber of Channels = %d\n",
		ch_distribution->operating_ch_count);
	SLIST_FOREACH_SAFE(operating_ch,
		&(ch_distribution->first_operating_ch),
		next_operating_ch, t_op_ch) {
		always("\t\tChannel Number = %d\n", operating_ch->ch_num);
		always("\t\tNumber of Radios = %d\n", operating_ch->radio_count);
		SLIST_FOREACH_SAFE(radio,
			&(operating_ch->first_radio),
			next_co_ch_radio, t_radio) {
			always("\t\t\tDev ALMAC = "MACSTR"\n",
				MAC2STR(radio->parent_1905->_1905_info.al_mac_addr));
			always("\t\t\tRadio ID = "MACSTR"\n",
				MAC2STR(radio->identifier));
		}
	}
	always("Operating Channel distribution on 2G\n");
	ch_distribution = &ch_planning->ch_ditribution_2g;
	always("\tNumber of Channels = %d\n",
		ch_distribution->operating_ch_count);
	t_op_ch = NULL;
	SLIST_FOREACH_SAFE(operating_ch,
		&(ch_distribution->first_operating_ch),
		next_operating_ch, t_op_ch) {
		always("\t\tChannel Number = %d\n", operating_ch->ch_num);
		always("\t\tNumber of Radios = %d\n", operating_ch->radio_count);
		SLIST_FOREACH_SAFE(radio,
			&(operating_ch->first_radio),
			next_co_ch_radio, t_radio) {
			always("\t\t\tDev ALMAC = "MACSTR"\n",
				MAC2STR(radio->parent_1905->_1905_info.al_mac_addr));
			always("\t\t\tRadio ID = "MACSTR"\n",
				MAC2STR(radio->identifier));
		}
	}

	always("Prefered Channel distribution on 5G\n");
	ch_distribution = &ch_planning->ch_ditribution_5g;
	always("\tNumber of Channels = %d\n",
		ch_distribution->prefered_ch_count);
	SLIST_FOREACH_SAFE(prefered_ch,
		&(ch_distribution->first_prefered_ch),
		next_prefered_ch, t_pref_ch) {
		always("\t\tChannel Number = %d\n", prefered_ch->ch_num);
		always("\t\tchannel Score = %x\n", prefered_ch->ch_score);
		SLIST_FOREACH_SAFE(prefered_ch_radio,
			&(prefered_ch->first_radio),
			next_pref_ch_radio, t_pref_ch_radio) {
			always("\t\t\tDev ALMAC = "MACSTR"\n",
				MAC2STR(prefered_ch_radio->radio->parent_1905->_1905_info.al_mac_addr));
		}
	}

	always("Prefered Channel distribution on 2G\n");
	ch_distribution = &ch_planning->ch_ditribution_2g;
	always("\tNumber of Channels = %d\n",
		ch_distribution->prefered_ch_count);
	t_pref_ch = NULL;
	SLIST_FOREACH_SAFE(prefered_ch,
		&(ch_distribution->first_prefered_ch),
		next_prefered_ch, t_pref_ch) {
		always("\t\tChannel Number = %d\n", prefered_ch->ch_num);
		always("\t\tchannel Score = %x\n", prefered_ch->ch_score);
		t_pref_ch_radio = NULL;
		SLIST_FOREACH_SAFE(prefered_ch_radio,
			&(prefered_ch->first_radio),
			next_pref_ch_radio, t_pref_ch_radio) {
			always("\t\t\tDev ALMAC = "MACSTR"\n",
				MAC2STR(prefered_ch_radio->radio->parent_1905->_1905_info.al_mac_addr));
		}
	}

	always(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
}
/**
* @brief Fn to operating channel distribution
* @param ctx own 1905 global
* @param dev 1905 map deice
* @param radio target radio
* @param channel*/

void ch_planning_update_ch_ditribution(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct radio_info_db *radio,
	unsigned char channel,
 	unsigned char op_class
)
{
	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct ch_distribution_cb *ch_distribution = NULL;
	struct operating_ch_cb *operating_ch, *t_op_ch = NULL;


	if (ctx->device_role != DEVICE_ROLE_CONTROLLER)
	{
		return;
	}
#ifdef MAP_6E_SUPPORT
	if (IS_MAP_CH_24G(radio->channel[0]) && IS_OP_CLASS_24G(radio->operating_class))
		ch_distribution = &ch_planning->ch_ditribution_2g;
	else if (IS_MAP_CH_5G(radio->channel[0]) && IS_OP_CLASS_5G(radio->operating_class))
		ch_distribution = &ch_planning->ch_ditribution_5g;
	else if (IS_MAP_CH_6G(radio->channel[0]) && IS_OP_CLASS_6G(radio->operating_class))
		ch_distribution = &ch_planning->ch_ditribution_6g;
#else
	if (radio->channel[0] > 14)
	{
		ch_distribution = &ch_planning->ch_ditribution_5g;
	} else {
		ch_distribution = &ch_planning->ch_ditribution_2g;
	}
#endif
	/*dump channel distribution to see if radio still exist in operating list, when radio->operating_channel = NULL*/
	if (radio->operating_channel == NULL) {
		debug(CH_PLANING_PREX"radio(%d) operating channel NULL, dev("MACSTR")\n",
			radio->channel[0], MAC2STR(dev->_1905_info.al_mac_addr));
		//ch_planning_show_ch_distribution(ctx);
	}

	if (radio->operating_channel != NULL) {
		//err("Unique channel was previousy allocated to device\n");
		operating_ch = radio->operating_channel;
		if (operating_ch->ch_num == channel
#ifdef MAP_6E_SUPPORT
			&& (operating_ch->opclass == op_class)
#endif
			)
		{
			//err("already on previously announced channel\n");
			return;
		} else {
			debug(CH_PLANING_PREX"remove radio from previous unique channel\n");
			operating_ch->radio_count--;
			//ch_planning_update_ch_score(ctx, ch_distribution, operating_ch->ch_num);
			SLIST_REMOVE(&operating_ch->first_radio,
			radio,
			radio_info_db,
			next_co_ch_radio);
			if (operating_ch->radio_count == 0)
			{
				debug(CH_PLANING_PREX"No more radios in current unique list\n");
				debug(CH_PLANING_PREX"remove from channel distribution\n");

				SLIST_REMOVE(&ch_distribution->first_operating_ch,
				operating_ch,
				operating_ch_cb,
				next_operating_ch);
				os_free(operating_ch);
				ch_distribution->operating_ch_count--;
			}
		}
	}
	operating_ch = NULL;
	SLIST_FOREACH_SAFE(operating_ch,
		&(ch_distribution->first_operating_ch),
		next_operating_ch, t_op_ch) {
		if ((operating_ch->ch_num == channel)
#ifdef MAP_6E_SUPPORT
			&& (operating_ch->opclass == op_class)
#endif
			)
		{
			debug(CH_PLANING_PREX"A unique channel exist for current channel req\n");
			SLIST_INSERT_HEAD(
				&(operating_ch->first_radio),
				radio,
				next_co_ch_radio);
			radio->operating_channel = operating_ch;
			operating_ch->radio_count++;
			//ch_planning_update_ch_score(ctx, ch_distribution, operating_ch->ch_num);
			SLIST_REMOVE(&(ch_distribution->first_operating_ch),
			operating_ch,
			operating_ch_cb,
			next_operating_ch);
			ch_planning_insert_into_ch_operating(ctx,
				operating_ch,
				ch_distribution);
			break;
		}
	}
	if (operating_ch == NULL)
	{
		debug(CH_PLANING_PREX"allocate a new unique channel, channel NUM = %d, opclass %d\n", channel, op_class);
		operating_ch = os_zalloc(sizeof(struct operating_ch_cb));
		if (operating_ch) {
			operating_ch->ch_num = channel;
#ifdef MAP_6E_SUPPORT
			operating_ch->opclass = op_class;
#endif
			SLIST_INIT(&(operating_ch->first_radio));
			SLIST_INSERT_HEAD(
				&(operating_ch->first_radio),
				radio,
				next_co_ch_radio);
			operating_ch->radio_count++;
			radio->operating_channel = operating_ch;
			if (ch_distribution->operating_ch_count == 0)
				SLIST_INIT(&(ch_distribution->first_operating_ch));
			ch_planning_insert_into_ch_operating(ctx,
				operating_ch,
				ch_distribution);
			ch_distribution->operating_ch_count++;
		}
		//ch_planning_update_ch_score(ctx, ch_distribution, operating_ch->ch_num);
	}
	//ch_planning_show_ch_distribution(ctx);
}
#ifdef MAP_R4_SPT
unsigned char *ch_planning_handle_sr_req_tlv(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned char *buff, unsigned char *bm_change)
{
	unsigned char *curr_pointer = buff;
	unsigned short tlv_len = 0;

	curr_pointer += 1;//increment a byte for TLV type

	tlv_len = (*curr_pointer);
	tlv_len = (tlv_len << 8) & 0xFF00;
	tlv_len = tlv_len |(*(curr_pointer+1));
	curr_pointer += 2;

	unsigned char * radio_id = curr_pointer;

	struct radio_info_db *radio =
		topo_srv_get_radio(dev, radio_id);

	if (radio == NULL)
	{
		radio = os_zalloc(sizeof(*radio));
		if (!radio) {
			err(CH_PLANING_PREX"mem allocation failed for radio");
			return NULL;
		}
		os_memcpy(radio->identifier, radio_id, ETH_ALEN);
		radio->parent_1905 = dev;
		debug(CH_PLANING_PREX"create new radio %p, parent device = %p\n",
			radio, radio->parent_1905);
		SLIST_INSERT_HEAD(&(dev->first_radio), radio, next_radio);
		SLIST_INIT(&(radio->link_estimate_cb_head));
		SLIST_INIT(&radio->chan_preferance.prefer_info_head);
		SLIST_INIT(&radio->chan_restrict.restrict_head);
#ifdef MAP_R2
		SLIST_INIT(&radio->first_scan_result);
		SLIST_INIT(&radio->cac_cap.cac_capab_head);
		SLIST_INIT(&radio->cac_comp_status.cac_completion_opcap_head);
#endif
		radio->bh_priority = 1;
	}

	if((os_memcmp(curr_pointer+11, radio->spt_reuse.srg_bss_color_bitmap, 8) == 0) &&
		(os_memcmp(curr_pointer+19, radio->spt_reuse.srg_partial_bssid_bitmap, 8) == 0)){
		info("bitmap is not changed No need to send Ch selection req len %d", tlv_len);
		return curr_pointer+tlv_len;
	}
	*bm_change |= *bm_change<<1;
	os_memcpy(radio->spt_reuse.identifier, curr_pointer, ETH_ALEN);
	curr_pointer += ETH_ALEN;

//	radio->spt_reuse.partial_bss_color = (*curr_pointer & 0x40) ? 1:0;
	radio->spt_reuse.valid = 1;
	radio->spt_reuse.bss_color = *curr_pointer++ & 0x3F;
	radio->spt_reuse.hesiga_spa_reuse_val_allowed = (*curr_pointer & 0x10) ? 1:0;
	radio->spt_reuse.srg_info_valid = (*curr_pointer & 0x08) ? 1:0;
	radio->spt_reuse.nonsrg_offset_valid= (*curr_pointer & 0x04) ? 1:0;
	radio->spt_reuse.psr_disallowed= (*curr_pointer++ & 0x01) ? 1:0;

	if(radio->spt_reuse.nonsrg_offset_valid)
		radio->spt_reuse.nonsrg_obsspd_max_offset = *curr_pointer;
	curr_pointer ++;

	if(radio->spt_reuse.srg_info_valid)
	{
		radio->spt_reuse.srg_obsspd_min_offset = *curr_pointer++;
		radio->spt_reuse.srg_obsspd_max_offset = *curr_pointer++;
		os_memcpy(radio->spt_reuse.srg_bss_color_bitmap, curr_pointer, 8);
		curr_pointer += 8;
		os_memcpy(radio->spt_reuse.srg_partial_bssid_bitmap, curr_pointer, 8);
		curr_pointer += 8;
	}
	else
	{
		curr_pointer += 18;
	}
	curr_pointer += 10;//! we do not need the legth for now.

	info("[bitmap] 0x%2x%2x%2x%2x%2x%2x%2x%2x\n",
			radio->spt_reuse.srg_bss_color_bitmap[0],
			radio->spt_reuse.srg_bss_color_bitmap[1],
			radio->spt_reuse.srg_bss_color_bitmap[2],
			radio->spt_reuse.srg_bss_color_bitmap[3],
			radio->spt_reuse.srg_bss_color_bitmap[4],
			radio->spt_reuse.srg_bss_color_bitmap[5],
			radio->spt_reuse.srg_bss_color_bitmap[6],
			radio->spt_reuse.srg_bss_color_bitmap[7]
	);
	return curr_pointer;
}

void create_and_update_mesh_srg_bm(struct own_1905_device *ctx)
{
	struct radio_info_db *radio = NULL, *tmp_radio = NULL, *t_radio = NULL;
	struct mapd_global * global = (struct mapd_global * )ctx->back_ptr;
	struct _1905_map_device *tmp_dev, *own_dev, *t_dev = NULL;
	struct ap_spt_reuse_req spt_reuse_role[3];
	struct map_spt_reuse_req *wapp_sr_data;
	int idx = 0;

	os_memset(spt_reuse_role, 0, 3*sizeof(struct ap_spt_reuse_req));

	own_dev = topo_srv_get_1905_device(ctx, NULL);
	SLIST_FOREACH_SAFE(radio, &own_dev->first_radio, next_radio, t_radio){

		os_memcpy(spt_reuse_role[idx].identifier, radio->identifier, ETH_ALEN);
		os_memcpy(spt_reuse_role[idx].srg_bss_color_bitmap, 
						radio->spt_reuse.srg_bss_color_bitmap, 8);
		debug("\033[1;31m[bitmap 0]\033[0m "MACSTR" 0x%2x%2x%2x%2x%2x%2x%2x%2x\n",
			MAC2STR(radio->identifier),
			spt_reuse_role[idx].srg_bss_color_bitmap[0],
			spt_reuse_role[idx].srg_bss_color_bitmap[1],
			spt_reuse_role[idx].srg_bss_color_bitmap[2],
			spt_reuse_role[idx].srg_bss_color_bitmap[3],
			spt_reuse_role[idx].srg_bss_color_bitmap[4],
			spt_reuse_role[idx].srg_bss_color_bitmap[5],
			spt_reuse_role[idx].srg_bss_color_bitmap[6],
			spt_reuse_role[idx].srg_bss_color_bitmap[7]
		);
		os_memcpy(spt_reuse_role[idx].srg_partial_bssid_bitmap, 
						radio->spt_reuse.srg_partial_bssid_bitmap, 8);
		spt_reuse_role[idx].srg_info_valid = 1;

		SLIST_FOREACH_SAFE(tmp_dev, &ctx->_1905_dev_head, next_1905_device, t_dev) {
			if(tmp_dev == own_dev)
				continue;
			if(tmp_dev->in_network){
				tmp_radio = topo_srv_get_radio(tmp_dev, radio->identifier);
				if (!tmp_radio)
					continue;
				if(tmp_radio->spt_reuse.valid){
					debug("\033[1;31m[bitmap 1]\033[0m ""0x%2x%2x%2x%2x%2x%2x%2x%2x\n",
						tmp_radio->spt_reuse.srg_bss_color_bitmap[0],
						tmp_radio->spt_reuse.srg_bss_color_bitmap[1],
						tmp_radio->spt_reuse.srg_bss_color_bitmap[2],
						tmp_radio->spt_reuse.srg_bss_color_bitmap[3],
						tmp_radio->spt_reuse.srg_bss_color_bitmap[4],
						tmp_radio->spt_reuse.srg_bss_color_bitmap[5],
						tmp_radio->spt_reuse.srg_bss_color_bitmap[6],
						tmp_radio->spt_reuse.srg_bss_color_bitmap[7]
					);
					for(int i=0; i<8; i++){
						spt_reuse_role[idx].srg_bss_color_bitmap[i] |=
							tmp_radio->spt_reuse.srg_bss_color_bitmap[i];
						spt_reuse_role[idx].srg_partial_bssid_bitmap[i] |=
							tmp_radio->spt_reuse.srg_partial_bssid_bitmap[i];
					}
				}
			}
		}
		info("\033[1;31m[bitmap F]\033[0m "" 0x%2x%2x%2x%2x%2x%2x%2x%2x\n",
			spt_reuse_role[idx].srg_bss_color_bitmap[0],
			spt_reuse_role[idx].srg_bss_color_bitmap[1],
			spt_reuse_role[idx].srg_bss_color_bitmap[2],
			spt_reuse_role[idx].srg_bss_color_bitmap[3],
			spt_reuse_role[idx].srg_bss_color_bitmap[4],
			spt_reuse_role[idx].srg_bss_color_bitmap[5],
			spt_reuse_role[idx].srg_bss_color_bitmap[6],
			spt_reuse_role[idx].srg_bss_color_bitmap[7]
		);
		idx++;
	}

	SLIST_FOREACH_SAFE(tmp_dev, &ctx->_1905_dev_head, next_1905_device, t_dev) {
		if (own_dev == tmp_dev){
			int len = sizeof(struct map_spt_reuse_req)+ idx*sizeof(struct ap_spt_reuse_req);
			wapp_sr_data = (struct map_spt_reuse_req *)os_zalloc(len);
			if(!wapp_sr_data){
				err("memory allocation failed");
				continue;
			}
			wapp_sr_data->spt_req_cnt = idx;
			os_memcpy(wapp_sr_data->spt_reuse_report, spt_reuse_role, idx*sizeof(struct
			ap_spt_reuse_req));

			wlanif_issue_wapp_command(ctx->back_ptr, WAPP_USER_SET_AP_SR_BM, 0,
						NULL, NULL, wapp_sr_data, len, 0, 0, 0);

			os_free(wapp_sr_data);
			continue;
		}
		if(tmp_dev->map_version < DEV_TYPE_R3)
			continue;
		err("sending sr channel selection to "MACSTR"",MAC2STR(tmp_dev->_1905_info.al_mac_addr));
		map_1905_Send_Channel_Selection_Request_Message(
				global->_1905_ctrl,
				(char *)tmp_dev->_1905_info.al_mac_addr,
				0,
				NULL,
				0,
				NULL,
				idx,
				spt_reuse_role
				);
	}
}
#endif

/**
* @brief Fn to handle operating channel report
* @param ctx own 1905 global
* @param dev target 1905 map device
* @param buff
*/
unsigned char * ch_planning_handle_oper_ch_tlv(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned char *buff)
{
	unsigned char * radio_id = buff;
	unsigned char *curr_pointer = buff +ETH_ALEN;
	unsigned char i = 0, radio_count = 0;
	unsigned char num_oper_class = *curr_pointer;
	unsigned char oper_class = 0;
	unsigned char oper_channel = 0;
	struct radio_info_db *radio =
		topo_srv_get_radio(dev, radio_id);
	struct radio_info_db *tmp_radio = NULL, *t_tmp_radio = NULL;
	unsigned char oper_bw = 0;

#if 0
	mapd_hexdump(MSG_ERROR, "ch_planning_handle_operating_channel_report",
		buff, 32);
#endif
	if (radio == NULL)
	{
		radio = os_zalloc(sizeof(*radio));
		if (!radio) {
			err(CH_PLANING_PREX"mem allocation failed for radio");
			return NULL;
		}
		os_memcpy(radio->identifier, radio_id, ETH_ALEN);
		radio->parent_1905 = dev;
		err(CH_PLANING_PREX"create new radio %p, parent device = %p\n",
			radio, radio->parent_1905);
		SLIST_INSERT_HEAD(&(dev->first_radio), radio, next_radio);
		SLIST_INIT(&(radio->link_estimate_cb_head));
		SLIST_INIT(&radio->chan_preferance.prefer_info_head);
		SLIST_INIT(&radio->chan_restrict.restrict_head);
#ifdef MAP_R2
		SLIST_INIT(&radio->first_scan_result);
		SLIST_INIT(&radio->cac_cap.cac_capab_head);
		SLIST_INIT(&radio->cac_comp_status.cac_completion_opcap_head);
#endif
		radio->bh_priority = 1;
	}
	curr_pointer++;
	radio->operating_class = 0;
	for (i = 0; i < num_oper_class; i++)
	{
		oper_class = *curr_pointer;
		curr_pointer++;
		oper_channel = *curr_pointer;
		curr_pointer++;
		err(CH_PLANING_PREX"Channel = %d, OperClass = %d oper_bw %d dev("MACSTR")",
				oper_channel, oper_class, oper_bw, MAC2STR(dev->_1905_info.al_mac_addr));
		if(radio->operating_class == 0)
		{
			radio->operating_class = oper_class;
			oper_bw = chan_mon_get_bw_from_op_class(oper_class);
			radio->orignal_bw = oper_bw;
		}
		os_memset(radio->channel, 0,sizeof(radio->channel));
		radio->channel[0] = oper_channel;
		/* getting Primary CH for 80MHz BW */
#ifdef MAP_320BW
		if (((oper_class == 128) || (oper_class == 129) || (oper_class == 137)) && (i < (num_oper_class-1))) {
#else
		if (((oper_class == 128) || (oper_class == 129)) && (i < (num_oper_class-1))) {
#endif
			unsigned char next_channel;

			next_channel = *(curr_pointer + 1);
			if (is_valid_primary_ch_80M_160M(next_channel, oper_channel, oper_class)) {
				radio->channel[0] = next_channel;
				err(CH_PLANING_PREX"Primary Ch %d, op: %u",radio->channel[0], oper_class);
				curr_pointer+=2;
				i++;
			}
		}
		mapd_fill_secondary_channels(radio->channel, oper_class, 0);
		SLIST_FOREACH_SAFE(tmp_radio, &(dev->first_radio), next_radio, t_tmp_radio) {
			radio_count++;
		}
		if (radio_count == 1 || radio_count == 2) {
			radio->band = get_band_from_channel_dual_band(radio->channel[0]);
		} else if (radio_count == 3) {
			radio->band = get_band_from_channel(radio->channel[0]);
		}
#ifdef MAP_6E_SUPPORT
		if (ctx->ch_planning_R2.bootup_scanstatus[radio->band].bootup_run == BOOTUP_SCAN_NOT_NEEDED) {
			ctx->ch_planning_R2.bootup_scanstatus[radio->band].bootup_run = BOOTUP_SCAN_NEEDED;
			ctx->ch_planning_R2.bootup_scanstatus[radio->band].band = radio->band;
			ctx->ch_planning_R2.bootup_scanstatus[radio->band].channel = radio->channel[0];
		}
		err("Bootup run %d, band %d",
			ctx->ch_planning_R2.bootup_scanstatus[radio->band].bootup_run,
			ctx->ch_planning_R2.bootup_scanstatus[radio->band].band);
#endif
		ch_planning_update_ch_ditribution(ctx, dev,
			radio, radio->channel[0], oper_class);
	}
	return curr_pointer;
}
void ch_planning_handle_operating_channel_report(
	struct own_1905_device *ctx,
	unsigned char *buff,
	int left_tlv_len)
{
	unsigned short len = 0;
	unsigned int tlv_len = 0;
	unsigned char *peer_al_mac = buff;
	unsigned char *curr_pointer = buff + ETH_ALEN;
	left_tlv_len -= ETH_ALEN;
	struct _1905_map_device *dev = topo_srv_get_1905_device(
		ctx, peer_al_mac);

#ifdef MAP_R4_SPT
	unsigned char sr_bm_change = 1;
#endif
	while (dev)
	{
		len = get_tlv_len(curr_pointer);
		tlv_len = len + 3;
		if (left_tlv_len < tlv_len) {
			err("[%d] Error TLV len, type = %d, left_tlv_length %d less than tlv_len %d\n",
			      __LINE__, *curr_pointer, left_tlv_len, tlv_len);
			return;
		}
		if (*curr_pointer == OPERATING_CHANNEL_REPORT_TYPE)
		{
			if (validate_tlv_length_specific_pattern(len,
			OPERATING_CHANNEL_REPORT_TLV_CONST_LEN,
			OPERATING_CHANNEL_REPORT_TLV_REPEATED_LEN) == FALSE) {
				err("err in length");
				return;
			}
			curr_pointer += 1;//increment a byte for TLV type
			curr_pointer += 2;//! we do not need the legth for now.

			curr_pointer = ch_planning_handle_oper_ch_tlv(ctx, dev, curr_pointer);
			if (curr_pointer == NULL)
				return;
			curr_pointer += 1; //! we are not handling TX power
		}
#ifdef MAP_R4_SPT
		else if(*curr_pointer == SPATIAL_REUSE_REPORT_TYPE)
		{
			if (check_fixed_length_tlv(SR_REPORT_TLV_LEN, curr_pointer, left_tlv_len) == FALSE) {
				err("Error TLV len, type = %d, left_tlv_length %d less than tlv_len %d\n",
								*curr_pointer, left_tlv_len, tlv_len);
				return;
			}

			curr_pointer = ch_planning_handle_sr_req_tlv(ctx, dev, curr_pointer, &sr_bm_change);
			if (curr_pointer == NULL)
				return;
		}
#endif
		else {
			break;
		}
		left_tlv_len -= tlv_len;
	}

#ifdef MAP_R4_SPT
	if(sr_bm_change != 1){
		create_and_update_mesh_srg_bm(ctx);
	}
#endif

	if (dev && ctx->ch_planning.current_ch_planning_dev == dev)
	{
		err(CH_PLANING_PREX"Operating Channel Report Received, planning completed for " MACSTR " ",
			MAC2STR(dev->_1905_info.al_mac_addr));
		ctx->ch_planning.current_ch_planning_dev = NULL;
		ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
	}

	if (dev != NULL) {
#ifdef MAP_R2
		dev->ch_sel_req_given = 0;
#endif
	}
}

/**
* @brief Fn periodic function for channel planning
* @param ctx own 1905 global
*/

void mapd_perform_channel_planning(struct own_1905_device *ctx)
{
	struct _1905_map_device *dev = NULL;
	unsigned int channel[3]={0,0,0};
	Boolean prefer_avail;
#ifdef MAP_R2
	/*Do not restart channel planning based on 30 min idle time if R2 ch planning is enabled*/
	if(ctx->ch_planning_R2.ch_plan_enable == FALSE) {
		if(ch_planning_is_MAP_net_idle(ctx)){
			mapd_restart_channel_plannig(ctx->back_ptr);
			struct ch_planning_cb *p_ch_planning = &ctx->ch_planning;
			os_get_time(&p_ch_planning->last_high_byte_count_ts);
		}
	}
#else
	struct os_time now;
	struct ch_planning_cb *p_ch_planning = &ctx->ch_planning;
	if (p_ch_planning->last_high_byte_count_ts.sec == 0)
	{
		os_get_time(&p_ch_planning->last_high_byte_count_ts);
	}
	os_get_time(&now);

	if (now.sec - p_ch_planning->last_high_byte_count_ts.sec >
		p_ch_planning->ChPlanningIdleTime)
	{
		err(CH_PLANING_PREX"ChPlanningIdleTime(%lu) expired!!! restart ch planning!!!",
			p_ch_planning->ChPlanningIdleTime);
		mapd_restart_channel_plannig(ctx->back_ptr);
		os_get_time(&p_ch_planning->last_high_byte_count_ts);
	}
#endif

#ifdef MAP_R2
	if (ctx->device_role == DEVICE_ROLE_CONTROLLER
			&& ctx->ch_planning_R2.ch_plan_enable == TRUE) {
		struct os_time current_time;
		uint8_t radio_idx = 0;
		uint32_t scan_time_diff = 0;

		os_get_time(&current_time);	
		scan_time_diff = current_time.sec - ctx->channel_planning_last_scan_timestamp.sec;
		if((ctx->channel_planning_last_scan_timestamp.sec > 0)
				&&( ctx->channel_planning_scan_valid_time && (scan_time_diff > ctx->channel_planning_scan_valid_time))) {
			err("Scan results valid time %d has been exceeded, last scan occurred %d secs ago ------>",
					ctx->channel_planning_scan_valid_time ,scan_time_diff);
			for (radio_idx = 0; radio_idx < MAX_NUM_OF_RADIO; radio_idx++) {
#ifndef MAP_6E_SUPPORT
				struct mapd_radio_info *radio_info = &ctx->dev_radio_info[radio_idx];

				if (radio_info->radio_idx == (uint8_t)-1)
					continue;

				if (radio_info->bootup_run == BOOTUP_SCAN_COMPLETED) {
					radio_info->bootup_run = BOOTUP_SCAN_NEEDED;
					ch_planning_R2_reset(ctx, NULL);
				}
#else
				if (ctx->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run == BOOTUP_SCAN_COMPLETED)
					ctx->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run = BOOTUP_SCAN_NEEDED;

#endif
			}
		}		
	}
#endif


#ifdef MAP_R2
	if (ctx->device_role == DEVICE_ROLE_CONTROLLER &&
		(ctx->ch_planning_R2.ch_plan_enable == TRUE)){
		struct os_time current_time;
		os_get_time(&current_time);
		dev = topo_srv_get_1905_device(ctx,NULL);//controller bootup
		if((current_time.sec - dev->first_seen.sec)
			> ctx->channel_planning_initial_timeout){
			ch_planning_R2_bootup_handling(ctx);
		}
	}

	if (ctx->device_role == DEVICE_ROLE_CONTROLLER &&
		is_chan_plan_done_for_all_dev(ctx->back_ptr)
		&& ctx->Restart_ch_planning_radar_on_agent) {
		err("Reset Restart_ch_planning_radar_on_agent");
		ctx->Restart_ch_planning_radar_on_agent = 0;
	}
	if (ctx->device_role == DEVICE_ROLE_CONTROLLER &&
		ctx->ch_planning_R2.ch_plan_enable == TRUE &&
		ctx->ch_planning_R2.need_cac_on_channel &&
		ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_IDLE) {
		err("need to retrigger ch plan on ch(%u)", ctx->ch_planning_R2.need_cac_on_channel);
		ctx->ch_planning_R2.CAC_on_channel = ctx->ch_planning_R2.need_cac_on_channel;
		ctx->ch_planning_R2.need_cac_on_channel = 0;
		if (eloop_is_timeout_registered(channel_cac_timeout2, (void *)ctx, NULL)) {
			/* Possibility that Cont gets radar on 5GL and then agent got radar at 5GH(on going CAC)
			Cont first save "need_cac_on_channel" to handle cac response but actually we
			wont get cac response, so better not to wait for it
			*/
			if (ch_planning_check_channel_operable_wrapper(ctx, ctx->ch_planning_R2.CAC_on_channel)) {
				err("Ch %u is Operable", ctx->ch_planning_R2.CAC_on_channel);
				ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_CAC_ONGOING;
				ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_CAC_START;
			} else {
				/* Channel not Operable */
				err("Ch not Operable");
				eloop_cancel_timeout(channel_cac_timeout2, ctx, NULL);
				ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
				ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
				ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED, ctx->ch_planning_R2.CAC_on_channel, ctx);
				mapd_restart_channel_plannig(ctx->back_ptr);
			}
		}
	}


#endif

	prefer_avail = is_all_dev_ch_pref_available(ctx);
	if ((ctx->device_role == DEVICE_ROLE_CONTROLLER) &&
		(ctx->ch_planning.current_ch_planning_dev == NULL) &&
		(prefer_avail == TRUE) &&
		(ctx->ch_planning.ch_planning_state == CHANNEL_PLANNING_IDLE) &&
#ifdef MAP_R2
		(ctx->ch_planning_R2.ch_plan_enable == TRUE) &&
#endif
		(ctx->ch_planning.ch_planning_enabled == TRUE))
	{
		//while(1)
		{
			dev = ch_planning_get_target_dev(ctx);
			if (dev != NULL)
			{
				err(CH_PLANING_PREX"prepare to perform channel planning on "MACSTR"\n",
					MAC2STR(dev->_1905_info.al_mac_addr));
#ifdef MAP_R2
				/*To Prevent the old channel planning logic to run simultaneously when
				  ch planing R2 algo is running*/
				if((ctx->ch_planning_R2.ch_plan_enable == TRUE) &&
						(ctx->ch_planning_R2.ch_plan_state != CHPLAN_STATE_IDLE &&
						 ctx->ch_planning_R2.ch_plan_state != CHPLAN_STATE_CH_CHANGE_TRIGGERED)) {
					err(CH_PLANING_PREX"Ch planning R2 ongoing(r2 state %d), wait it done(r2 state changes to 0 or 6)",
							ctx->ch_planning_R2.ch_plan_state);
					return;
				}
				if (ctx->div_ch_planning == 1) {
					ctx->ch_planning.ch_planning_enabled = 0;
					if (ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_CH_CHANGE_TRIGGERED && ctx->ch_planning_R2.CAC_on_channel) {
						channel[0] = ctx->ch_planning_R2.CAC_on_channel;
						ctx->force_ch_change = 1;
					}
				}
#endif
				ch_planning_exec(ctx, dev, channel);
			}
		}
	}
}

#ifdef MAP_R2
void ch_planning_reset_user_preff_ch(struct mapd_global *global)
{
	struct own_1905_device *ctx = &global->dev;
	struct ch_distribution_cb *ch_distribution;
	ch_distribution = &ctx->ch_planning.ch_ditribution_5g;
	ch_distribution->user_prefered_ch_HighBand = 0;
	ch_distribution->user_prefered_ch = 0;
	ch_distribution = &ctx->ch_planning.ch_ditribution_2g;
	ch_distribution->user_prefered_ch_HighBand = 0;
	ch_distribution->user_prefered_ch = 0;
}
#endif
int ch_planning_set_user_preff_ch(struct mapd_global *global,
	u8 channel)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_map_device *dev = topo_srv_get_next_1905_device(ctx, NULL);
	struct ch_distribution_cb *ch_distribution;
	if (channel > 14)
		ch_distribution = &ctx->ch_planning.ch_ditribution_5g;
	else
		ch_distribution = &ctx->ch_planning.ch_ditribution_2g;
	if(check_is_triband(dev)&&(isChan5GH(channel))) {
		ch_distribution->user_prefered_ch_HighBand = channel;
	} else {
		ch_distribution->user_prefered_ch = channel;
		//ch_planning_update_ch_score(ctx,ch_distribution, prev_prefered_ch);
	}

	//ch_planning_update_ch_score(ctx, ch_distribution, channel);
	err(" channel %d", channel);
	mapd_restart_channel_plannig(global);
	return 0;
}

/**
* @brief Fn to set txpower percentage for a given MAP device
* @param ctx own 1905 global
* @param dev target dev
* @param unsigned char band
* @param unsigned char txpower_percentage
*/
void ch_planning_set_txpower_percentage(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned char band,
	unsigned char txpower_percentage)
{
	struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(ctx, NULL);
	struct tx_power_percentage_setting *tx_power_setting = NULL;

	err("Set Tx Power percentage\n");

	if(txpower_percentage > 100) {
		err("Invalid txpower_percentage setting %d\n", txpower_percentage);
		return;
	}
	if(!((band == BAND_24G) || (band == BAND_5G))) {
		err("Invalid band setting %d!!\n", band);
		return;
	}
	if (own_1905_node != dev) {
		err("Set Tx Power percentage on agent\n");
		ch_planning_send_txpower_percentage_msg(ctx, dev->_1905_info.al_mac_addr, band, txpower_percentage);
	} else {
		err("Set Tx Power percentage on controller\n");
		tx_power_setting = os_zalloc(sizeof(struct tx_power_percentage_setting));
		if(tx_power_setting == NULL) {
			err("tx_power setting:Memory allocation failed\n");
			return;
		}
		tx_power_setting->bandIdx = band;
		tx_power_setting->tx_power_percentage = txpower_percentage;

		wlanif_issue_wapp_command(ctx->back_ptr, WAPP_USER_SET_TX_POWER_PERCENTAGE, 0,
			NULL, NULL, tx_power_setting, sizeof(struct tx_power_percentage_setting), 0, 0, 0);
		os_free(tx_power_setting);
	}

	return;
}

/**
* @brief Fn to send txpower percentage for a given MAP device
* @param ctx own 1905 global
* @param unsigned char *al_mac_addr
* @param unsigned char bandIdx
* @param unsigned char txpower_percentage
*/
void ch_planning_send_txpower_percentage_msg(
	struct own_1905_device *ctx,
	unsigned char *al_mac_addr,
	unsigned char bandIdx,
	unsigned char txpower_percentage)
{
	struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;
	struct tx_power_percentage_tlv *txpower_percent_tlv = NULL;

	if (txpower_percentage > 100){
		err("Invalid txpower_percentage setting %d\n",txpower_percentage);
		return;
	}
	if(!((bandIdx == BAND_24G) || (bandIdx == BAND_5G))) {
		err("Invalid band setting %d!!\n", bandIdx);
		return;
	}
	debug("sending vendor tx power percentage tlv");
	txpower_percent_tlv = os_zalloc(sizeof(struct tx_power_percentage_tlv));

	if(txpower_percent_tlv == NULL) {
		err("txpower_percent_tlv is NULL\n");
		return;
	}

	txpower_percent_tlv->tlv_type = TLV_802_11_VENDOR_SPECIFIC;
	txpower_percent_tlv->tlv_len = host_to_be16(TX_POWER_PERCENTAGE_TLV_LEN);
	os_memcpy(txpower_percent_tlv->mtk_oui, MTK_OUI, OUI_LEN);
	txpower_percent_tlv->func_type = FUNC_VENDOR_SET_TX_POWER_PERCENTAGE;
	txpower_percent_tlv->bandIdx = bandIdx;
	txpower_percent_tlv->tx_power_percentage = txpower_percentage;

	map_1905_Send_Vendor_Specific_Message(mapd_ctx->_1905_ctrl, (char *)al_mac_addr,
						(char *)txpower_percent_tlv, sizeof(struct tx_power_percentage_tlv));
	os_free(txpower_percent_tlv);

	return;
}

/**
* @brief Fn to handle txpower percentage from the controller
* @param ctx own 1905 global
* @param tx_power_percentage_tlv *txpower_percent_tlv
*/
void ch_planning_handle_tx_power_percentage_msg(
	struct mapd_global *pGlobal_dev,
	struct tx_power_percentage_tlv *txpower_percent_tlv)
{
	struct tx_power_percentage_setting *tx_power_setting = NULL;
	struct own_1905_device *ctx = &pGlobal_dev->dev;

	if(ctx == NULL) {
		mapd_printf(MSG_ERROR,"own dev ctx is NULL");
		return;
	}

	if(txpower_percent_tlv == NULL) {
		mapd_printf(MSG_ERROR,"txpower_percent_tlv is null");
		return;
	}

	tx_power_setting = os_zalloc(sizeof(struct tx_power_percentage_setting));
	if(tx_power_setting == NULL) {
		mapd_printf(MSG_ERROR,"tx_power setting:Memory allocation failed\n");
		return;
	}

	tx_power_setting->bandIdx = txpower_percent_tlv->bandIdx;
	tx_power_setting->tx_power_percentage = txpower_percent_tlv->tx_power_percentage;
	always("%s:tx_power_setting->bandIdx = %d, tx_power_setting->tx_power_percentage = %d\n",__func__, tx_power_setting->bandIdx, tx_power_setting->tx_power_percentage);
	wlanif_issue_wapp_command(ctx->back_ptr, WAPP_USER_SET_TX_POWER_PERCENTAGE, 0,
		NULL, NULL, tx_power_setting, sizeof(struct tx_power_percentage_setting), 0, 0, 0);
	os_free(tx_power_setting);

	return;
}
int off_ch_scan_exec(struct own_1905_device *ctx,
					char *buf,
					unsigned char *reply, unsigned char bandwidth)
{
	unsigned char almac[6]= {0};
	int i = 0;
	int reply_len;
	unsigned char band = 0;
	unsigned char mode = 0;
	unsigned char bw = 0;
	unsigned char scan_ch_list[MAX_OFF_CH_SCAN_CH] = {0};
	struct _1905_map_device *target_1905=NULL;
	char * ptmp = NULL;
	char * pvalue = NULL;
	//! ptmp points to comand string
	ptmp = strtok_r(buf, " ", &buf);
	//! ptmp points to ALMAC
	ptmp = strtok_r(buf, " ", &buf);
	pvalue = strtok_r(ptmp, ":", &ptmp);
	for(i = 0;(pvalue && (i< ETH_ALEN)); i++, pvalue = strtok_r(ptmp, ":", &ptmp)) {
		almac[i] = strtol(pvalue, &pvalue, 0x10);
	}
	bw = bandwidth;
	ptmp = strtok_r(buf, " ", &buf);
	if (!ptmp) {
		err("missed mode\n");
		return -1;
	}
	mode= strtol(ptmp, &ptmp, 0x10);
	if (mode == SCAN_MODE_BAND) {
		//! ptmp points to band
		ptmp = strtok_r(buf, " ", &buf);
		if(!ptmp)  {
			err("miss the specified band\n");
			return -1;
		}
		band = strtol(ptmp, &ptmp, 0x10);
		if (errno == ERANGE)
			err("strtol cmd error=%d\n", __LINE__);
	} else if (mode == SCAN_MODE_CH){
		ptmp = strtok_r(buf, " ", &buf);
		i = 0;
		while (ptmp && i < sizeof(scan_ch_list)) {
			scan_ch_list[i] = strtol(ptmp, &ptmp, 10);
			ptmp = strtok_r(buf, " ", &buf);
			i++;
		}
	}
	target_1905 = topo_srv_get_1905_device(ctx, almac);
	if (!target_1905) {
		err("device with given almac not found\n");
		reply_len=-1;
	} else if((!target_1905->in_network)) {
		err("device is not connected\n");
		reply_len=-1;
	} else {
		os_memcpy(reply, "OK\n", 3);
		reply_len = 3;
	}

	if (target_1905) {
		if (target_1905->off_ch_scan_report) {
			os_free(target_1905->off_ch_scan_report);
			target_1905->off_ch_scan_report = NULL;
		}
		send_off_ch_scan_req(ctx->back_ptr, target_1905, mode, band, scan_ch_list, bw, 1);
	}
	return reply_len;
}

int map_is_valid_bssid(char *mac)
{

	int i = 0;
	int s = 0;
	char *tmp = mac;

	while (*tmp) {
		if (isxdigit(*tmp)) {
			i++;
		}
		else if (*tmp == ':' || *tmp == '-') {

			if (i == 0 || i/2 - 1 != s)
				break;

			++s;
		}
		else {
			s = -1;
		}

		++tmp;
	}
	return (i == 12 && (s == 5 || s == 0));
}

int mapd_cmd_steer_restrict_bss(struct own_1905_device *ctx, char *buf)
{
	char bss_id[6]= {0};
	struct _1905_map_device *target_1905=NULL;
	char *pcmd = NULL;
	//! ptmp points to ALMAC
	struct _1905_map_device *dev = NULL;
	int reply_len;
	pcmd = strtok_r(buf, " ", &buf);

	debug(CENT_STEER_PREX"called %s\n", __func__);
	dev = topo_srv_get_controller_device(ctx);
	if (dev == NULL)
		return -1;

	if(dev->device_role != DEVICE_ROLE_CONTROLLER) {
		err("Controller only command!");
		return -1;
	}

	if(hwaddr_aton(buf, (u8 *)bss_id) < 0) { 
		err("bss_id not rcv'd");
		return -1;
	}

	err(CENT_STEER_PREX"bss_id rcv'd ("MACSTR")\n", MAC2STR(bss_id));
		
	target_1905 = topo_srv_get_1905_device(ctx, (unsigned char *)bss_id);
	if (!target_1905) {

		err(CENT_STEER_PREX"device with given almac not found\n");
		reply_len=-1;

	} else if((!target_1905->in_network)) {

		err(CENT_STEER_PREX"device is not connected\n");
		reply_len=-1;

	} else {

		err("OK\n");
		reply_len = 3;

	}

	if (!strcmp(pcmd, "ADD")) {

		if(topo_srv_cli_update_steer_restrict_bss(ctx, bss_id)) {
			debug(CENT_STEER_PREX"bss_id ("MACSTR") is added in the restrict list\n", MAC2STR(bss_id));
			reply_len = 1;
		}

	} else if (!strcmp(pcmd, "REMOVE")) {

		if(topo_srv_del_steer_restrict_bss(ctx, bss_id)) {
			debug(CENT_STEER_PREX"bss_id ("MACSTR") is removed from the restrict list\n", MAC2STR(bss_id));
			reply_len = 1;
		}

	} else if (!strcmp(pcmd, "FLUSH")) {

		if(!topo_srv_flush_steer_restrict_bss(ctx)) {
			debug(CENT_STEER_PREX"complete restrict list flushed from DB\n");
			reply_len = 1;
		}

	} else if (!strcmp(pcmd, "PRINT")) {

		if(!topo_srv_print_steer_restrict_bss(ctx)) {
			debug(CENT_STEER_PREX"Operation complete\n");
			reply_len = 1;
		}

	} else
		debug(CENT_STEER_PREX"Please provide correct options\n");

	return reply_len;
}


void mapd_fill_secondary_channels(unsigned char *channel,
	unsigned char op_class, unsigned char bw)
{
	int i = 0;
	int j = 1;
	unsigned char centre_freq = 0;
	Boolean is6G = FALSE;

#ifdef MAP_6E_SUPPORT
	if (IS_OP_CLASS_6G(op_class))
		is6G = TRUE;
#endif

	if ((op_class == 129) || (bw == 3)
#ifdef MAP_6E_SUPPORT
		|| (op_class == 134)
#endif
		) {
		if (op_class == 0) {
			centre_freq = ch_planning_get_centre_freq_ch_by_bw(channel[0], bw);
		} else {
			centre_freq = ch_planning_get_centre_freq_ch(channel[0], op_class);
		}

		if (centre_freq == 0)
			return;

		for (i = 0; i < 8; i++) {
			if (channel[0] == ((centre_freq - 14) + (i*4))) {
				continue;
			} else {
				channel[j] = ((centre_freq - 14) + (i*4));
				j++;
			}
		}
	} else if ((op_class == 128) || (bw == 2)
#ifdef MAP_6E_SUPPORT
		|| (op_class == 133)
#endif
		) {
		if (op_class == 0) {
			centre_freq = ch_planning_get_centre_freq_ch_by_bw(channel[0], bw);
		} else {
			centre_freq = ch_planning_get_centre_freq_ch(channel[0], op_class);
		}
		if (centre_freq == 0)
			return;

		for (i = 0; i < 4; i++) {
			if (channel[0] == ((centre_freq - 6) + (i*4))) {
				continue;
			} else {
				channel[j] = ((centre_freq - 6) + (i*4));
				j++;
			}
		}
	} else if (bw == 1 && channel[0] > 14 && is6G == FALSE) {
		while (vht_ch_40M[i].cent_freq_idx != 0) {
			if (channel[0] >= vht_ch_40M[i].ch_low_bnd &&
				channel[0] <= vht_ch_40M[i].ch_up_bnd) {
				channel[0] = vht_ch_40M[i].ch_low_bnd;
				channel[1] = vht_ch_40M[i].ch_up_bnd;
				return;
			}
			i++;
		}
	}
#ifdef MAP_6E_SUPPORT
	else if ((bw == 1) && IS_MAP_CH_6G(channel[0]) && (is6G == TRUE)) {
		while (SixE_ch_40M[i].cent_freq_idx != 0) {
			if (channel[0] >= SixE_ch_40M[i].ch_low_bnd &&
				channel[0] <= SixE_ch_40M[i].ch_up_bnd) {
				channel[0] = SixE_ch_40M[i].ch_low_bnd;
				channel[1] = SixE_ch_40M[i].ch_up_bnd;
				return;
			}
			i++;
		}
	}
#endif
#ifdef MAP_320BW
	else if ((op_class == 137) || (bw == BW_320)) {
		if (op_class == 0)
			centre_freq = ch_planning_get_centre_freq_ch_by_bw(channel[0], bw);
		else
			centre_freq = ch_planning_get_centre_freq_ch(channel[0], op_class);
		if (centre_freq == 0)
			return;

		for (i = 0; i < 16; i++) {
			if (channel[0] == ((centre_freq - 30) + (i*4))) {
				continue;
			} else {
				channel[j] = ((centre_freq - 30) + (i*4));
				j++;
			}
		}
	}
#endif

}
void mapd_fill_secondary_channels_for_1905_dev(struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{
	struct radio_info_db *radio = topo_srv_get_next_radio(
		dev, NULL);
	struct bss_info_db *bss_info;
	struct iface_info *iface;
	while (radio) {
		bss_info = NULL;
		iface = NULL;
		bss_info = topo_srv_get_next_bss(dev, NULL);
		while (bss_info) {
			if (bss_info->radio == radio) {
				break;
			}
			bss_info = topo_srv_get_next_bss(dev, bss_info);
		}

		if (bss_info) {
			iface = topo_srv_get_next_iface(dev, NULL);
			while (iface) {
				if (!os_memcmp(iface->iface_addr, bss_info->bssid,
					ETH_ALEN)) {
					break;
				}
				iface = topo_srv_get_next_iface(dev, iface);
			}
		}
		if (iface) {
			os_memset(radio->channel, 0,
				sizeof(radio->channel));
			radio->channel[0] = iface->channel;
			radio->operating_class = iface->op_class;
			mapd_fill_secondary_channels(radio->channel, radio->operating_class, iface->bw);
#ifdef MAP_6E_SUPPORT
			radio->band = get_band_from_chan_op(radio->channel[0], radio->operating_class);
#else
			radio->band = get_band_from_channel(radio->channel[0]);
#endif
			info("Filled ch(%u) Op(%u) for "MACSTR, radio->channel[0], radio->operating_class,
				MAC2STR(dev->_1905_info.al_mac_addr));
		}
		radio = topo_srv_get_next_radio(dev, radio);
	}
}

void start_netopt_timer(struct own_1905_device *own_dev,u8 channel)
{
	u8 i;
	u8 count = 0;
	for (i = 0; i < 16; i++) {
		debug("ch %d pref 0x%x",own_dev->dfs_info_ch_list[i].channel,
			own_dev->dfs_info_ch_list[i].pref);
		if(channel == own_dev->dfs_info_ch_list[i].channel) {
			if(own_dev->dfs_info_ch_list[i].pref &(OP_DISALLOWED_DUE_TO_DFS)) {
				count = get_net_opt_dev_count((struct mapd_global *)own_dev->back_ptr);
				if (count > 1) {
					if (eloop_is_timeout_registered(trigger_net_opt,(void *)own_dev , NULL))
						eloop_cancel_timeout(trigger_net_opt, own_dev, NULL);
					eloop_register_timeout((own_dev->network_optimization.post_cac_trigger_time + own_dev->dfs_info_ch_list[i].cac_timer),
							0, trigger_net_opt, own_dev, NULL);
					err("Channel is DFS, netopt timer : %d+%d",
						own_dev->network_optimization.post_cac_trigger_time, own_dev->dfs_info_ch_list[i].cac_timer);
				}
			}
		}
	}
}

Boolean is_ch_operable_on_opclass(struct radio_info_db *radio, u8 channel, u8 op)
{
	struct prefer_info_db *prefer_info = NULL, *t_pref_info = NULL;
	u8 i = 0;

	SLIST_FOREACH_SAFE(prefer_info, &(radio->chan_preferance.prefer_info_head), prefer_info_entry, t_pref_info) {
		if (prefer_info->op_class == op) {
			for (i = 0; i < prefer_info->ch_num; i++) {
				if (prefer_info->ch_list[i] == channel && prefer_info->perference == 0)
					return FALSE;
			}
		}
	}

	return TRUE;
}

Boolean ch_planning_is_ch_dfs(
	struct own_1905_device *own_dev,u8 channel)
{
	u8 i = 0;
	struct ch_planning_cb *ch_planning = NULL;
	struct prefered_ch_cb *prefered_ch = NULL, *t_pref_ch = NULL;
	struct ch_distribution_cb *ch_distribution = NULL;
	int maxbw = BW_20, bw = BW_20;
	struct _1905_map_device *_1905_dev = NULL, *t_1905_dev = NULL;
	struct radio_info_db *radio = NULL;

	ch_planning = &own_dev->ch_planning;
	if (!ch_planning) {
		err(CH_PLANING_PREX"Error!! channel planning is NULL");
		return FALSE;
	}

	if (channel > 14)
		ch_distribution = &ch_planning->ch_ditribution_5g;
	else {
		err("Channel (%u) is not 5G channel", channel);
		return FALSE;
	}

	SLIST_FOREACH_SAFE(_1905_dev, &own_dev->_1905_dev_head, next_1905_device, t_1905_dev) {
		radio = topo_srv_get_radio_by_band(_1905_dev,channel);
		bw = ch_planning_get_max_bw_1905dev_prefer(own_dev, radio, _1905_dev);
		if (maxbw < bw)
			maxbw = bw;
	}

	if ((maxbw == BW_160) && (channel >= 36 && channel <= 48)) {
		SLIST_FOREACH_SAFE(_1905_dev, &own_dev->_1905_dev_head, next_1905_device, t_1905_dev) {
			radio = topo_srv_get_radio_by_band(_1905_dev, channel);
			if (!radio)
				continue;

			/* If central channel 50 is NOP then no need CAC on 160M */
			if (is_ch_operable_on_opclass(radio, 50, 129) == FALSE)
				return FALSE;
		}
	}
	SLIST_FOREACH_SAFE(prefered_ch, &ch_distribution->first_prefered_ch, next_prefered_ch, t_pref_ch) {
		if (prefered_ch->ch_num == channel) {
			for (i = 0; i < MAX_DFS_CHANNEL; i++) {
				if (prefered_ch->ch_num == RadarCh[i]
					|| ((maxbw == BW_160) && (prefered_ch->ch_num >= 36 && prefered_ch->ch_num <= 48))) {
					debug(CH_PLANING_PREX"ch-%u is DFS channel", channel);
					return TRUE;
				}
			}
		}
	}

	debug(CH_PLANING_PREX"ch-%u is NON DFS channel", channel);
	return FALSE;
}

#ifdef MAP_R2
void ch_scan_req_timeout(void * eloop_ctx, void *user_ctx)
{
	struct mapd_global *ctx = eloop_ctx;
	struct own_1905_device *own_dev = &ctx->dev;

	if(own_dev->user_triggered_scan == 1) {
		own_dev->user_triggered_scan = 0;
		handle_task_completion(own_dev);
	}
}
#endif


enum ch_type {
	CH_STATE = (0x1 << 0),
	CH_CAC_CAP = (0x1 << 1),
	CH_PREFERENCE = (0x1 << 2),
	CH_DISTRIBUTION = (0x1 << 3),
	CH_SCORE = (0x1 << 4),
	CH_SCAN_MONITOR = (0x1 << 5),
	CH_ALL = CH_STATE | CH_CAC_CAP | CH_PREFERENCE | CH_DISTRIBUTION | CH_SCORE | CH_SCAN_MONITOR,
};

int show_channel_planning_state(struct own_1905_device *own_dev, char *reply, int reply_len)
{
	struct _1905_map_device *dev = NULL, *t_dev = NULL;
	struct radio_info_db *radio = NULL, *t_radio = NULL;
#ifdef MAP_R2
#ifndef MAP_6E_SUPPORT
	struct mapd_radio_info *radio_info = NULL;
#endif
	struct monitor_ch_info *ch_info = NULL, *t_ch_info = NULL;
	uint8_t radio_idx = 0;
#endif
	struct os_time current_time;
	char *pos = reply;
	int len = 0, total_len = 0;

	if (!reply || reply_len <= 0)
		return 0;

	if (own_dev->device_role != DEVICE_ROLE_CONTROLLER) {
		len = os_snprintf(pos+total_len, reply_len-total_len, "only show this information at controller side\n");
		if (len < 0)
			err("os_snprintf cmd error=%d\n", __LINE__);
		total_len += len;
		return total_len;
	}

	len = os_snprintf(pos+total_len, reply_len-total_len, "**************channel planning state************\n");
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "channel planning R1(%d)\n",
		own_dev->ch_planning.ch_planning_enabled);
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
#ifdef MAP_R2
	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "channel planning R2(%d)\n",
		own_dev->ch_planning_R2.ch_plan_enable);
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
#endif
	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "\n");
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
	if (!own_dev->ch_planning.ch_planning_enabled) {
		if (reply_len-total_len <= 0)
			goto end;
		len = os_snprintf(pos+total_len, reply_len-total_len, "channel planning disable no need print more info\n");
		if (len < 0)
			err("os_snprintf cmd error=%d\n", __LINE__);
		total_len += len;
		return total_len;
	}
	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "ch_r1_state(%d)\n",
			own_dev->ch_planning.ch_planning_state);
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
#ifdef MAP_R2
	if (own_dev->ch_planning_R2.ch_plan_enable) {
		if (reply_len-total_len <= 0)
			goto end;
		len = os_snprintf(pos+total_len, reply_len-total_len, "ch_r2_state(%d)\n",
				own_dev->ch_planning_R2.ch_plan_state);
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
		total_len += len;
	}
#endif
	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "\n");
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
	if (own_dev->ch_planning.current_ch_planning_dev) {
		dev = (struct _1905_map_device *)own_dev->ch_planning.current_ch_planning_dev;
		if (reply_len-total_len <= 0)
			goto end;
		len = os_snprintf(pos+total_len, reply_len-total_len, "current_ch_planning_dev("MACSTR")\n",
				MAC2STR(dev->_1905_info.al_mac_addr));
		if (len < 0)
			err("os_snprintf cmd error=%d\n", __LINE__);
		total_len += len;
		if (reply_len-total_len <= 0)
			goto end;
		len = os_snprintf(pos+total_len, reply_len-total_len, "channel planning is underway\n");
		if (len < 0)
			err("os_snprintf cmd error=%d\n", __LINE__);
		total_len += len;
	} else {
		if (reply_len-total_len <= 0)
			goto end;
		len = os_snprintf(pos+total_len, reply_len-total_len, "current_ch_planning_dev null\n");
		if (len < 0)
			err("os_snprintf cmd error=%d\n", __LINE__);
		total_len += len;
	}
	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "\n");
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "dump all the dev info\n");
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "channel_planning_initial_timeout(%d)\n",
			own_dev->channel_planning_initial_timeout);
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
	SLIST_FOREACH_SAFE(dev, &own_dev->_1905_dev_head, next_1905_device, t_dev) {
		if (!dev->in_network)
			continue;
		os_get_time(&current_time);
		if (reply_len-total_len <= 0)
			goto end;
		len = os_snprintf(pos+total_len, reply_len-total_len,
				"dev("MACSTR") in_network(%d) ch_preference_available(%d)"
				"radio_mapping_completed(%d) sec_past(%ld) channel_planning_completed(%d)\n",
				MAC2STR(dev->_1905_info.al_mac_addr),
				dev->in_network,
				dev->ch_preference_available,
				dev->radio_mapping_completed,
				(long)(current_time.sec - dev->first_seen.sec),
				dev->channel_planning_completed);
		if (len < 0)
			err("os_snprintf cmd error=%d\n", __LINE__);
		total_len += len;
		if (current_time.sec - dev->first_seen.sec > 120) {
			if (reply_len-total_len <= 0)
				goto end;
			len = os_snprintf(pos+total_len, reply_len-total_len, "channel planning delay time for current dev past\n");
			if (len < 0)
				err("os_snprintf cmd error=%d\n", __LINE__);
			total_len += len;
		} else {
			if (reply_len-total_len <= 0)
				goto end;
			len = os_snprintf(pos+total_len, reply_len-total_len, "channel planning delay time for current dev not past\n");
			if (len < 0)
				err("os_snprintf cmd error=%d\n", __LINE__);
			total_len += len;
		}
		SLIST_FOREACH_SAFE(radio, &dev->first_radio, next_radio, t_radio) {
#ifdef MAP_R2
			if (reply_len-total_len <= 0)
				goto end;
			len = os_snprintf(pos+total_len, reply_len-total_len,
					"rid("MACSTR") state(%d) channel(%d) opclass(%d)\n",
					MAC2STR(radio->identifier),
					radio->dev_ch_plan_info.dev_ch_plan_state,
					radio->channel[0],
					radio->operating_class);
			if (len < 0)
				err("os_snprintf cmd error=%d\n", __LINE__);
			total_len += len;
#else
			if (reply_len-total_len <= 0)
				goto end;
			len = os_snprintf(pos+total_len, reply_len-total_len,
					"rid("MACSTR") channel(%d) opclass(%d)\n",
					MAC2STR(radio->identifier),
					radio->channel[0],
					radio->operating_class);
			if (len < 0)
				err("os_snprintf cmd error=%d\n", __LINE__);
			total_len += len;
#endif
		}
	}

	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "\n");
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
#ifdef MAP_R2
	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "************radio bootup run info*************\n");
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;

	for (radio_idx = 0; radio_idx < MAX_NUM_OF_RADIO; radio_idx++) {
#ifndef MAP_6E_SUPPORT
		radio_info = &own_dev->dev_radio_info[radio_idx];
		if (radio_info->radio_idx != (uint8_t)-1) {
			if (reply_len-total_len <= 0)
				goto end;
			len = os_snprintf(pos+total_len, reply_len-total_len,
					"own radio ch %d radio_info->bootup_run state %d\n",radio_info->channel, radio_info->bootup_run);
			if (len < 0)
				err("os_snprintf cmd error=%d\n", __LINE__);
			total_len += len;
		}
#else
		len = os_snprintf(pos+total_len, reply_len-total_len,
				"radio ch %d bootup_run state %d\n",
				own_dev->ch_planning_R2.bootup_scanstatus[radio_idx].channel,
				own_dev->ch_planning_R2.bootup_scanstatus[radio_idx].bootup_run);
			if (len < 0)
				err("os_snprintf cmd error=%d\n", __LINE__);
		total_len += len;

#endif
	}

	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "\n");
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "************monitor info*************\n");
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
	SLIST_FOREACH_SAFE(ch_info, &own_dev->ch_planning_R2.first_monitor_ch, next_monitor_ch, t_ch_info) {
		if (reply_len-total_len <= 0)
			goto end;
		len = os_snprintf(pos+total_len, reply_len-total_len, "monitor channel_num %d\n", ch_info->channel_num);
		if (len < 0)
			err("os_snprintf cmd error=%d\n", __LINE__);
		total_len += len;
		if (reply_len-total_len <= 0)
			goto end;
		len = os_snprintf(pos+total_len, reply_len-total_len, "monitor channel scan trigger_status %d\n",
				ch_info->trigger_status);
		if (len < 0)
			err("os_snprintf cmd error=%d\n", __LINE__);
		total_len += len;
	}

	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "\n");
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "************network optimization info*************\n");
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "network_opt_enable(%d) network_opt_state(%d)\n",
		own_dev->network_optimization.network_optimization_enabled,
		own_dev->network_optimization.network_opt_state);
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
#endif
	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "\n\n");
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;

end:
	return total_len;
}

#ifdef MAP_R2
void show_channel_planning_CAC_capability(struct own_1905_device *own_dev)
{
	struct _1905_map_device *dev = NULL, *t_dev = NULL;
	struct radio_info_db *radio = NULL, *t_radio = NULL;
	struct cac_cap_db *cap = NULL, *t_cap = NULL;
	struct cac_opcap_db *opcap = NULL, *t_op_cap = NULL;
	struct chnList *ch_list;
	unsigned char i = 0;

	if (own_dev->device_role != DEVICE_ROLE_CONTROLLER) {
		err("only show this information at controller side");
		return;
	}

	err("**************1905 dev CAC cap************");
	SLIST_FOREACH_SAFE(dev, &own_dev->_1905_dev_head, next_1905_device, t_dev) {
		if (!dev->in_network)
			continue;
		SLIST_FOREACH_SAFE(radio, &dev->first_radio, next_radio, t_radio) {
			err("dev("MACSTR") id(%02x:%02x:%02x:%02x:%02x:%02x) channel(%d) opclass(%d)",
			PRINT_MAC(dev->_1905_info.al_mac_addr), MAC2STR(radio->identifier),
			radio->channel[0], radio->operating_class);
			SLIST_FOREACH_SAFE(cap, &radio->cac_cap.cac_capab_head, cac_cap_entry, t_cap) {
				err("cac mode(%d) 0:Continuous CAC; 1:Continuous with dedicated radio;"
					" 2:MIMO dimension reduced; 3:Time sliced CAC", cap->cac_mode);
				err("Duration(%d-%d-%d)", cap->cac_interval[0], cap->cac_interval[1], cap->cac_interval[2]);
				SLIST_FOREACH_SAFE(opcap, &cap->cac_opcap_head, cac_opcap_entry, t_op_cap) {
					err("opclass(%d) ch list:", opcap->op_class);
					for (i = 0; i < opcap->ch_num; i++)
						printf("%d ", opcap->ch_list[i]);
					printf("\n\n");
				}
			}
		}
	}
	err("\n\n");

	err("**************own dev dfs_info_ch_list************");
	for (i = 0; i <16; i++) {
		ch_list = &own_dev->dfs_info_ch_list[i];
		err("ch(%d) pref(%02x) cac_timer(%d)", ch_list->channel,
			ch_list->pref, ch_list->cac_timer);
	}


	t_dev = NULL;
	t_radio = NULL;
	err("\n\n");
	err("************CAC complete status************");
	SLIST_FOREACH_SAFE(dev, &own_dev->_1905_dev_head, next_1905_device, t_dev) {
		if (!dev->in_network)
			continue;
		SLIST_FOREACH_SAFE(radio, &dev->first_radio, next_radio, t_radio) {
			err("dev("MACSTR") id(%02x:%02x:%02x:%02x:%02x:%02x) channel(%d) opclass(%d)",
				PRINT_MAC(dev->_1905_info.al_mac_addr), MAC2STR(radio->identifier),
				radio->channel[0], radio->operating_class);
			err("CAC sucessed on op_class(%d) channel(%d) status(%d)",
				radio->cac_comp_status.op_class, radio->cac_comp_status.channel,
				radio->cac_comp_status.cac_status);
		}
	}

	err("\n\n");
}

void show_channel_planning_scores(struct own_1905_device *own_dev)
{
	struct score_info *cumulative_score = NULL, *t_cumm_score = NULL;
	struct grp_score_info *grp_score = NULL, *t_grp_score = NULL;

	if (own_dev->device_role != DEVICE_ROLE_CONTROLLER) {
		err("only show this information at controller side");
		return;
	}

	err("**************CHScoreTable************");
	SLIST_FOREACH_SAFE(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score,next_ch_score, t_cumm_score) {
		err("CH %d , avg score %d, rank %d",
			cumulative_score->channel, cumulative_score->avg_score, cumulative_score->ch_rank);
	}
	if(own_dev->ch_planning_R2.ch_plan_enable_bw) {
		err("*********GroupScoreTable****************");
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
	err("\n\n");
}

void show_channel_planning_scan_info(struct own_1905_device *own_dev)
{
	struct _1905_map_device *dev = NULL, *t_dev = NULL;
	struct radio_info_db *radio = NULL, *t_radio = NULL;
	struct scan_result_tlv *res = NULL, *t_res = NULL;

	err("*************Scan results dump***********");
	SLIST_FOREACH_SAFE(dev, &own_dev->_1905_dev_head, next_1905_device, t_dev) {
		if (!dev->in_network)
			continue;
		SLIST_FOREACH_SAFE(radio, &dev->first_radio, next_radio, t_radio) {
			err("dev("MACSTR") rid("MACSTR") channel(%d) opclass(%d)",
				MAC2STR(dev->_1905_info.al_mac_addr), MAC2STR(radio->identifier),
				radio->channel[0], radio->operating_class);
			SLIST_FOREACH_SAFE(res, &radio->first_scan_result, next_scan_result, t_res) {
				err("Scan results for channel %d", res->channel);
				err("Util %d, NBnum %d score %d", res->utilization, res->neighbor_num, res->ch_score);
				err("EDCCA %d, ch %d",res->cu_distribution.edcca_airtime, res->cu_distribution.ch_num);
			}
		}
	}
	err("\n\n");
}
#endif

int show_channel_preference_database(struct own_1905_device *own_dev, char *reply, int reply_len)
{
	struct _1905_map_device *dev = NULL, *t_dev = NULL;
	struct radio_info_db *radio = NULL, *t_radio = NULL;
	struct radio_ch_prefer *ch_prefer = NULL;
	struct prefer_info_db *prefer_db = NULL, *t_prefer_db = NULL;
	u8 bw = 0, i = 0;
	char *pos = reply;
	int len = 0, total_len = 0;

	if (!reply || reply_len <= 0)
		return 0;

	len = os_snprintf(pos+total_len, reply_len-total_len, "1905 dev CH PREFER info on %s\n",
				own_dev->device_role == DEVICE_ROLE_CONTROLLER ? "controller" : "agent");
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;
	SLIST_FOREACH_SAFE(dev, &own_dev->_1905_dev_head, next_1905_device, t_dev) {
		if (!dev->in_network)
			continue;
		SLIST_FOREACH_SAFE(radio, &dev->first_radio, next_radio, t_radio) {
			if (reply_len-total_len <= 0)
				goto end;
			len = os_snprintf(pos+total_len, reply_len-total_len,
					"dev("MACSTR") id("MACSTR") channel(%d) opclass(%d)\n",
					MAC2STR(dev->_1905_info.al_mac_addr), MAC2STR(radio->identifier),
					radio->channel[0], radio->operating_class);
			if (len < 0)
				err("os_snprintf cmd error=%d\n", __LINE__);
			total_len += len;
			ch_prefer = &radio->chan_preferance;
			SLIST_FOREACH_SAFE(prefer_db, &ch_prefer->prefer_info_head, prefer_info_entry, t_prefer_db) {
				bw = chan_mon_get_bw_from_op_class(prefer_db->op_class);
				if (reply_len-total_len <= 0)
					goto end;
				len = os_snprintf(pos+total_len, reply_len-total_len,
					"opclass %d, bw %d, pref %d, reason %d\n",
					prefer_db->op_class,bw, prefer_db->perference, prefer_db->reason);
				if (len < 0)
					err("os_snprintf cmd error=%d\n", __LINE__);
				total_len += len;

				
				if (reply_len-total_len <= 0)
					goto end;
				len = os_snprintf(pos+total_len, reply_len-total_len, "ch list: ");
				if (len < 0)
					err("os_snprintf cmd error=%d\n", __LINE__);
				total_len += len;
				for(i=0; i<prefer_db->ch_num; i++) {
					if (reply_len-total_len <= 0)
						goto end;
					len = os_snprintf(pos+total_len, reply_len-total_len, "%d ", prefer_db->ch_list[i]);
					if (len < 0)
						err("os_snprintf cmd error=%d\n", __LINE__);
					total_len += len;
				}
				if (reply_len-total_len <= 0)
					goto end;
				len = os_snprintf(pos+total_len, reply_len-total_len, "\n");
				if (len < 0)
					err("os_snprintf cmd error=%d\n", __LINE__);
				total_len += len;
			}
		}
	}
	if (reply_len-total_len <= 0)
		goto end;
	len = os_snprintf(pos+total_len, reply_len-total_len, "\n\n");
	if (len < 0)
		err("os_snprintf cmd error=%d\n", __LINE__);
	total_len += len;

end:
	return total_len;
}

void show_channel_planning_distribution(struct own_1905_device *own_dev)
{
	struct _1905_map_device *dev = NULL, *t_dev = NULL;
	struct radio_info_db *radio = NULL, *t_radio = NULL;
	struct radio_ch_prefer *ch_prefer = NULL;
	struct prefer_info_db *prefer_db = NULL, *t_prefer_db = NULL;
	u8 bw = 0, i = 0;

	err("**************1905 dev CH PREFER info on %s************",
		own_dev->device_role == DEVICE_ROLE_CONTROLLER ? "controller" : "agent");
	SLIST_FOREACH_SAFE(dev, &own_dev->_1905_dev_head, next_1905_device, t_dev) {
		SLIST_FOREACH_SAFE(radio, &dev->first_radio, next_radio, t_radio) {
			err("dev("MACSTR") id("MACSTR") channel(%d) opclass(%d)",
			MAC2STR(dev->_1905_info.al_mac_addr), MAC2STR(radio->identifier),
			radio->channel[0], radio->operating_class);
			ch_prefer = &radio->chan_preferance;
			SLIST_FOREACH_SAFE(prefer_db, &ch_prefer->prefer_info_head, prefer_info_entry, t_prefer_db) {
				bw = chan_mon_get_bw_from_op_class(prefer_db->op_class);
				err("opclass %d, bw %d, pref %d, reason %d",
					prefer_db->op_class,bw, prefer_db->perference, prefer_db->reason);
				printf("ch list: ");
				for(i=0; i<prefer_db->ch_num; i++)
					printf("%d ", prefer_db->ch_list[i]);
				printf("\n");
			}
		}
	}
	err("\n\n");
}


int show_channel_planning_information(struct mapd_global *global, int val, char *reply, int reply_size)
{
	struct own_1905_device *own_dev = &global->dev;
	int total_len=0;

	if (!reply)
		return 0;

	if (val & CH_STATE)
		total_len += show_channel_planning_state(own_dev, reply, reply_size);
	if (val & CH_CAC_CAP)
#ifdef MAP_R2
		show_channel_planning_CAC_capability(own_dev);
#else
		err("MAP_R1 don't support CAC cap check");
#endif
	if (val & CH_PREFERENCE)
		total_len += show_channel_preference_database(own_dev, reply+total_len, reply_size-total_len);
	if (val & CH_DISTRIBUTION)
		ch_planning_show_ch_distribution(own_dev);
	if (val & CH_SCORE)
#ifdef MAP_R2
		show_channel_planning_scores(own_dev);
#else
		err("MAP_R1 don't support SCORE check");
#endif
	if (val & CH_SCAN_MONITOR)
#ifdef MAP_R2
		show_channel_planning_scan_info(own_dev);
#else
		err("MAP_R1 don't support SCAN INFO check");
#endif

	return total_len;

}

