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
 *  AP selection
 *
 *  Abstract:
 *  AP selection
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Kapil.Gupta 2018/05/02    First implementation of the AP selection
 * */
#include "includes.h"
#ifdef __linux__
#include <fcntl.h>
#endif /* __linux__ */

#include "common.h"
#include <sys/un.h>

#include "interface.h"
#include "data_def.h"
#include "client_db.h"
#include "mapd_i.h"
#include "eloop.h"
#include "wapp_if.h"
#include "tlv_parsor.h"
#include "topologySrv.h"
#include "1905_if.h"
#include "apSelection.h"
#include "chan_mon.h"
#include <net/if.h>
#include "1905_map_interface.h"
#include "ch_planning.h"
#include "network_optimization.h"
#include "mapfilter_if.h"
#include "ap_est.h"

#define VHT_RSSI_LEVEL 11
extern u8 ZERO_MAC_ADDR[ETH_ALEN];

unsigned long power(unsigned long x, unsigned long y)
{
    if (y == 0)
        return 1;
    else if (y % 2 == 0)
        return power(x, y / 2) * power(x, y / 2);
    else
        return x * power(x, y / 2) * power(x, y / 2);
}

static const int rssi_to_phyrate_he_ss1_20[VHT_RSSI_LEVEL][2] =
{
    {-67, 1147},
    {-69, 1032},
    {-71, 860},
    {-73, 774},
    {-76, 688},
    {-78, 516},
    {-82, 344},
    {-84, 258},
    {-87, 172},
    {-89, 86},
    {-127, 60}
};

static const int rssi_to_phyrate_he_ss1_40[VHT_RSSI_LEVEL][2] =
{
    {-67, 2294},
    {-69, 2065},
    {-71, 1721},
    {-73, 1549},
    {-76, 1376},
    {-78, 1032},
    {-82, 688},
    {-84, 516},
    {-87, 344},
    {-89, 172},
    {-127, 60}
};

static const int rssi_to_phyrate_he_ss1_80[VHT_RSSI_LEVEL][2] =
{
    {-67, 4804},
    {-69, 4324},
    {-71, 3603},
    {-73, 3243},
    {-76, 2882},
    {-78, 2162},
    {-82, 1441},
    {-84, 1081},
    {-87, 721},
    {-89, 360},
    {-127, 60},
};

static const int rssi_to_phyrate_he_ss1_160[VHT_RSSI_LEVEL][2] =
{
    {-67, 9607},
    {-69, 8647},
    {-71, 7206},
    {-73, 6485},
    {-76, 5765},
    {-78, 4324},
    {-82, 2882},
    {-84, 2162},
    {-87, 1441},
    {-89, 721},
    {-127, 60}
};

static const int rssi_to_phyrate_he_ss2_20[VHT_RSSI_LEVEL][2] =
{
    {-67, 2294},
    {-69, 2065},
    {-71, 1721},
    {-73, 1549},
    {-76, 1376},
    {-78, 1032},
    {-82, 688},
    {-84, 516},
    {-87, 344},
    {-89, 172},
    {-127, 60}
};

static const int rssi_to_phyrate_he_ss2_40[VHT_RSSI_LEVEL][2] =
{
    {-67, 4588},
    {-69, 4129},
    {-71, 3441},
    {-73, 3097},
    {-76, 2753},
    {-78, 2065},
    {-82, 1376},
    {-84, 1032},
    {-87, 688},
    {-89, 344},
    {-127, 60}
};

static const int rssi_to_phyrate_he_ss2_80[VHT_RSSI_LEVEL][2] =
{
    {-67, 9607},
    {-69, 8647},
    {-71, 7206},
    {-73, 6485},
    {-76, 5765},
    {-78, 4324},
    {-82, 2882},
    {-84, 2162},
    {-87, 1441},
    {-89, 721},
    {-127, 60},
};

static const int rssi_to_phyrate_he_ss2_160[VHT_RSSI_LEVEL][2] =
{
    {-67, 19215},
    {-69, 17294},
    {-71, 14412},
    {-73, 12971},
    {-76, 11529},
    {-78, 8647},
    {-82, 5765},
    {-84, 4324},
    {-87, 2882},
    {-89, 1441},
    {-127, 60}
};


static const int rssi_to_phyrate_he_ss3_20[VHT_RSSI_LEVEL][2] =
{
    {-67, 3441},
    {-69, 3097},
    {-71, 2581},
    {-73, 2323},
    {-76, 2065},
    {-78, 1549},
    {-82, 1032},
    {-84, 774},
    {-87, 516},
    {-89, 258},
    {-127, 60}
};

static const int rssi_to_phyrate_he_ss3_40[VHT_RSSI_LEVEL][2] =
{
    {-67, 6882},
    {-69, 6194},
    {-71, 5162},
    {-73, 4646},
    {-76, 4129},
    {-78, 3097},
    {-82, 2065},
    {-84, 1549},
    {-87, 1032},
    {-89, 516},
    {-127, 60}
};

static const int rssi_to_phyrate_he_ss3_80[VHT_RSSI_LEVEL][2] =
{
    {-67, 14412},
    {-69, 12971},
    {-71, 10809},
    {-73,  9728},  //should be same rate??
    {-76,  8647},
    {-78,  6485},
    {-82,  4324},
    {-84,  3243},
    {-87, 2162},
    {-89, 1081},
    {-127, 60},
};

static const int rssi_to_phyrate_he_ss3_160[VHT_RSSI_LEVEL][2] =
{
    {-67, 28824},
    {-69, 25942},
    {-71, 21618},
    {-73, 19456},
    {-76, 17294},
    {-78, 12971},
    {-82,  8647},
    {-84,  6485},
    {-87, 4324},
    {-89, 2162},
    {-127, 60}
};

static const int rssi_to_phyrate_he_ss4_20[VHT_RSSI_LEVEL][2] =
{
    {-67, 4588},
    {-69, 4129},
    {-71, 3441},
    {-73, 3097},
    {-76, 2753},
    {-78, 2065},
    {-82, 1376},
    {-84, 1032},
    {-87, 688},
    {-89, 344},
    {-127, 60}
};

static const int rssi_to_phyrate_he_ss4_40[VHT_RSSI_LEVEL][2] =
{
    {-67, 9176},
    {-69, 8259},
    {-71, 6882},
    {-73, 6194},
    {-76, 5506},
    {-78, 4129},
    {-82, 2753},
    {-84, 2065},
    {-87, 1376},
    {-89, 688},
    {-127, 60}
};
static const int rssi_to_phyrate_he_ss4_80[VHT_RSSI_LEVEL][2] =
{
    {-67, 19215},
    {-69, 17294},
    {-71, 14412},
    {-73, 12971},
    {-76, 11529},
    {-78,  8647},
    {-82,  5765},
    {-84,  4324},
    {-87,  2882},
    {-89,  1441},
    {-127, 60},
};

static const int rssi_to_phyrate_he_ss4_160[VHT_RSSI_LEVEL][2] =
{
    {-67, 38431},
    {-69, 34588},
    {-71, 28824},
    {-73, 25941},
    {-76, 23059},
    {-78, 17294},
    {-82, 11529},
    {-84,  8647},
    {-87, 5765},
    {-89, 2882},
    {-127, 60}
};

static const int16_t rssi_to_phyrate_vht_ss1_20[VHT_RSSI_LEVEL][2] =
{
    {-67, 780}, //TODO
    {-69, 780},
    {-71, 650},
    {-73, 585},
    {-76, 520},
    {-78, 390},
    {-82, 260},
    {-84, 195},
    {-87, 130},
    {-89, 65},
    {-127, 60}
};

static const int16_t rssi_to_phyrate_vht_ss1_40[VHT_RSSI_LEVEL][2] =
{
    {-67, 1800},
    {-69, 1620},
    {-71, 1350},
    {-73, 1215},
    {-76, 1080},
    {-78, 810},
    {-82, 540},
    {-84, 400},
    {-87, 270},
    {-89, 130},
    {-127, 60}
};

static const int16_t rssi_to_phyrate_vht_ss1_80[VHT_RSSI_LEVEL][2] =
{
    {-67, 3900},
    {-69, 3510},
    {-71, 2925},
    {-73, 2633},
    {-76, 2340},
    {-78, 1755},
    {-82, 1170},
    {-84, 878},
    {-87, 585},
    {-89, 293},
    {-127, 60},
};

static const int16_t rssi_to_phyrate_vht_ss1_160[VHT_RSSI_LEVEL][2] =
{
    {-67, 7800},
    {-69, 7020},
    {-71, 5850},
    {-73, 5265},
    {-76, 4680},
    {-78, 3510},
    {-82, 2340},
    {-84, 1755},
    {-87, 1170},
    {-89, 585},
    {-127, 60}
};

static const int16_t rssi_to_phyrate_vht_ss2_20[VHT_RSSI_LEVEL][2] =
{
    {-67, 1560},
    {-69, 1560},
    {-71, 1300},
    {-73, 1170},
    {-76, 1040},
    {-78, 780},
    {-82, 520},
    {-84, 390},
    {-87, 130},
    {-89, 65},
    {-127, 60}
};

static const int16_t rssi_to_phyrate_vht_ss2_40[VHT_RSSI_LEVEL][2] =
{
    {-67, 3600},
    {-69, 3240},
    {-71, 2700},
    {-73, 2430},
    {-76, 2160},
    {-78, 1620},
    {-82, 1080},
    {-84, 810},
    {-87, 270},
    {-89, 130},
    {-127, 60}
};

static const int16_t rssi_to_phyrate_vht_ss2_80[VHT_RSSI_LEVEL][2] =
{
    {-67, 7800},
    {-69, 7020},
    {-71, 5850},
    {-73, 5260},
    {-76, 4680},
    {-78, 3510},
    {-82, 2340},
    {-84, 1755},
    {-87, 585},
    {-89, 293},
    {-127, 60},
};

static const int16_t rssi_to_phyrate_vht_ss2_160[VHT_RSSI_LEVEL][2] =
{
    {-67, 15600},
    {-69, 14040},
    {-71, 11700},
    {-73, 10530},
    {-76, 9360},
    {-78, 7020},
    {-82, 4680},
    {-84, 3510},
    {-87, 1170},
    {-89, 585},
    {-127, 60}
};

static const int16_t rssi_to_phyrate_vht_ss3_20[VHT_RSSI_LEVEL][2] =
{
    {-67, 2600},
    {-69, 2340},
    {-71, 1950},
    {-73, 1755},
    {-76, 1560},
    {-78, 1170},
    {-82, 780},
    {-84, 585},
    {-87, 130},
    {-89, 65},
    {-127, 60}
};

static const int16_t rssi_to_phyrate_vht_ss3_40[VHT_RSSI_LEVEL][2] =
{
    {-67, 5400},
    {-69, 4860},
    {-71, 4050},
    {-73, 3645},
    {-76, 3240},
    {-78, 2430},
    {-82, 1620},
    {-84, 1215},
    {-87, 270},
    {-89, 130},
    {-127, 60}
};

static const int16_t rssi_to_phyrate_vht_ss3_80[VHT_RSSI_LEVEL][2] =
{
    {-67, 11700},
    {-69, 10530},
    {-71,  8775},
    {-73,  8775},  //should be same rate??
    {-76,  7020},
    {-78,  5265},
    {-82,  3510},
    {-84,  2633},
    {-87, 585},
    {-89, 293},
    {-127, 60},
};

static const int16_t rssi_to_phyrate_vht_ss3_160[VHT_RSSI_LEVEL][2] =
{
    {-67, 21060},
    {-69, 21060},
    {-71, 17550},
    {-73, 15795},
    {-76, 14040},
    {-78, 10530},
    {-82,  7020},
    {-84,  5265},
    {-87, 1170},
    {-89, 585},
    {-127, 60}
};

static const int16_t rssi_to_phyrate_vht_ss4_20[VHT_RSSI_LEVEL][2] =
{
    {-67, 3120},
    {-69, 3120},
    {-71, 2600},
    {-73, 2340},
    {-76, 2080},
    {-78, 1560},
    {-82, 1040},
    {-84,  780},
    {-87, 130},
    {-89, 65},
    {-127, 60}
};

static const int16_t rssi_to_phyrate_vht_ss4_40[VHT_RSSI_LEVEL][2] =
{
    {-67, 7200},
    {-69, 6480},
    {-71, 5400},
    {-73, 4860},
    {-76, 4320},
    {-78, 3240},
    {-82, 2160},
    {-84, 1620},
    {-87, 270},
    {-89, 130},
    {-127, 60}
};

static const int16_t rssi_to_phyrate_vht_ss4_80[VHT_RSSI_LEVEL][2] =
{
    {-67, 15600},
    {-69, 14040},
    {-71, 11700},
    {-73, 10530},  //should be same rate??
    {-76,  9360},
    {-78,  7020},
    {-82,  4680},
    {-84,  3510},
    {-87, 585},
    {-89, 293},
    {-127, 60},
};

static const int16_t rssi_to_phyrate_vht_ss4_160[VHT_RSSI_LEVEL][2] =
{
    {-67, 31200},
    {-69, 28080},
    {-71, 23400},
    {-73, 21060},
    {-76, 18720},
    {-78, 14040},
    {-82,  9360},
    {-84,  7020},
    {-87, 1170},
    {-89, 585},
    {-127, 60}
};

#define HT_RSSI_LEVEL 10
static const int16_t rssi_to_phyrate_ht_ss1_20[HT_RSSI_LEVEL][2] =
{
    {-72, 650},
    {-74, 585},
    {-77, 520},
    {-79, 390},
    {-81, 260},
    {-83, 195},
    {-86, 130},
    {-90,  65},
    {-94,  20},
    {-127, 10}
};


static const int16_t rssi_to_phyrate_ht_ss1_40[HT_RSSI_LEVEL][2] =
{
    {-72, 1350},
    {-74, 1215},
    {-77, 1080},
    {-79,  810},
    {-81,  540},
    {-83,  405},
    {-86,  270},
    {-90,  135},
    {-94,  20},
    {-127, 10}
};

/* defined but not used [-Werror=unused-const-variable=]
static const int16_t rssi_to_phyrate_ht_ss2_20[HT_RSSI_LEVEL][2] =
{
    {-72, 1300},
    {-74, 1170},
    {-77, 1040},
    {-79,  780},
    {-81,  520},
    {-83,  390},
    {-86,  130},
    {-90,   65},
    {-94,   20},
    {-127,  10}
};*/

static const int16_t rssi_to_phyrate_ht_ss2_40[HT_RSSI_LEVEL][2] =
{
    {-72, 2700},
    {-74, 2430},
    {-77, 2160},
    {-79, 1620},
    {-81, 1080},
    {-83,  810},
    {-86,  270},
    {-90,  135},
    {-94,  20},
    {-127, 10}
};

static const int16_t rssi_to_phyrate_ht_ss3_20[HT_RSSI_LEVEL][2] =
{
    {-72, 1950},
    {-74, 1755},
    {-77, 1560},
    {-79, 1170},
    {-81,  780},
    {-83,  585},
    {-86,  130},
    {-90,   65},
    {-94,   20},
    {-127,  10}
};

static const int16_t rssi_to_phyrate_ht_ss3_40[HT_RSSI_LEVEL][2] =
{
    {-72, 4050},
    {-74, 3645},
    {-77, 3240},
    {-79, 2430},
    {-81, 1620},
    {-83, 1215},
    {-86,  270},
    {-90,  135},
    {-94,  20},
    {-127, 10}
};

static const int16_t rssi_to_phyrate_ht_ss4_20[HT_RSSI_LEVEL][2] =
{
    {-72, 2600},
    {-74, 2340},
    {-77, 2080},
    {-79, 1560},
    {-81, 1040},
    {-83,  780},
    {-86,  130},
    {-90,   65},
    {-94,   20},
    {-127,  10}
};

static const int16_t rssi_to_phyrate_ht_ss4_40[HT_RSSI_LEVEL][2] =
{
    {-72, 4050},
    {-74, 3645},
    {-77, 3240},
    {-79, 2430},
    {-81, 1620},
    {-83, 1215},
    {-86,  270},
    {-90,  135},
    {-94,  20},
    {-127, 10}
};

#define BG_RSSI_LEVEL 8
static const int16_t rssi_to_phyrate_bg[BG_RSSI_LEVEL][2] =
{
    {-72, 540},
    {-76, 480},
    {-80, 360},
    {-84, 240},
    {-87, 180},
    {-89, 120},
    {-92,  60},
    {-127, 10}
};

#define G_RSSI_LEVEL 8
static const int16_t rssi_to_phyrate_g[G_RSSI_LEVEL][2] =
{
    {-72, 540},
    {-76, 480},
    {-80, 360},
    {-84, 240},
    {-87, 180},
    {-89, 120},
    {-92,  90},
    {-127, 60}
};

#define B_RSSI_LEVEL 4
static const int16_t rssi_to_phyrate_b[B_RSSI_LEVEL][2] =
{
    {-85, 110},
    {-87, 55},
    {-90, 20},
    {-127, 10}
};

/**
* @brief Fn to check if profile is matching
*
* @param ctx own 1905 device ctx
* @param info bh link infor
* @param bssid bssid of ap
*
* @return
*/
int is_profile_match(struct own_1905_device *ctx, struct bh_link_entry *info, unsigned char *bssid)
{
	//TODO
	return TRUE;
}
#ifdef MAP_R3
static int32_t min(int32_t a, int32_t b)
{
        return ((a > b) ? b : a) ;
}
#endif

/**
* @brief Fn to get estimated rate for 11g
*
* @param bss bss pointer
*
* @return rate
*/
int get_estimate_rate_11g(signed char Rssi)
{
	int i;

	for (i = 0; i < G_RSSI_LEVEL; i++) {
		if (rssi_to_phyrate_g[i][0] < Rssi)
			return rssi_to_phyrate_g[i][1];
	}
	return -1;
}

/**
* @brief Fn to get estimated rate for 11bg
*
* @param bss bss pointer
*
* @return rate
*/
int get_estimate_rate_11bg(struct bss_info *bss)
{
	int i;

	for (i = 0; i < BG_RSSI_LEVEL; i++) {
		if (rssi_to_phyrate_bg[i][0] < bss->Rssi)
			return rssi_to_phyrate_bg[i][1];
	}
	return -1;
}

/**
* @brief Fn to get estimated rate for 11b
*
* @param bss bss pointer
*
* @return rate
*/
int get_estimate_rate_11b(signed char Rssi)
{
	int i;

	for (i = 0; i < B_RSSI_LEVEL; i++) {
		if (rssi_to_phyrate_b[i][0] < Rssi)
			return rssi_to_phyrate_b[i][1];
	}
	return -1;
}

/**
* @brief Fn to get 11ac rate of a bss
*
* @param bss bss info
* @param stream number of stream
* @param bw bw supported
*
* @return rate
*/
int get_estimate_rate_11ax(signed char Rssi, int stream, int bw)
{
	int i;
	const int **final_rate_table = NULL;
	const int *rate_table = NULL;

	if ((stream == 4) && (bw == BW_160))
		rate_table = rssi_to_phyrate_he_ss4_160[0];
	else if ((stream == 4) && (bw == BW_80))
		rate_table = rssi_to_phyrate_he_ss4_80[0];
	else if ((stream == 4) && (bw == BW_40))
		rate_table = rssi_to_phyrate_he_ss4_40[0];
	else if ((stream == 4) && (bw == BW_20))
		rate_table = rssi_to_phyrate_he_ss4_20[0];
	else if ((stream == 3) && (bw == BW_160))
		rate_table = rssi_to_phyrate_he_ss3_160[0];
	else if ((stream == 3) && (bw == BW_80))
		rate_table = rssi_to_phyrate_he_ss3_80[0];
	else if ((stream == 3) && (bw == BW_40))
		rate_table = rssi_to_phyrate_he_ss3_40[0];
	else if ((stream == 3) && (bw == BW_20))
		rate_table = rssi_to_phyrate_he_ss3_20[0];
	else if ((stream == 2) && (bw == BW_160))
		rate_table = rssi_to_phyrate_he_ss2_160[0];
	else if ((stream == 2) && (bw == BW_80))
		rate_table = rssi_to_phyrate_he_ss2_80[0];
	else if ((stream == 2) && (bw == BW_40))
		rate_table = rssi_to_phyrate_he_ss2_40[0];
	else if ((stream == 2) && (bw == BW_20))
		rate_table = rssi_to_phyrate_he_ss2_20[0];
	else if ((stream == 1) && (bw == BW_160))
		rate_table = rssi_to_phyrate_he_ss1_160[0];
	else if ((stream == 1) && (bw == BW_80))
		rate_table = rssi_to_phyrate_he_ss1_80[0];
	else if ((stream == 1) && (bw == BW_40))
		rate_table = rssi_to_phyrate_he_ss1_40[0];
	else if ((stream == 1) && (bw == BW_20))
		rate_table = rssi_to_phyrate_he_ss1_20[0];


	if (!rate_table) {
		err("failed to get value from he rate table Rssi[%d], Stream[%d], BW[%d]\n", Rssi, stream, bw);
		return -1;
	}

	final_rate_table = &rate_table;
	for (i = 0; i < VHT_RSSI_LEVEL; i++) {
		if ((*(*final_rate_table + (i * 2) + 0)) < Rssi) {
			return (*(*final_rate_table + (i * 2) + 1));
		}
	}

	return -1;
}

int get_estimate_rate_11ac(signed char Rssi, int stream, int bw)
{
	int i;
	const int16_t **final_rate_table = NULL;
	const int16_t *rate_table = NULL;

	if ((stream == 4) && (bw == BW_160))
		rate_table = rssi_to_phyrate_vht_ss4_160[0];
	else if ((stream == 4) && (bw == BW_80))
		rate_table = rssi_to_phyrate_vht_ss4_80[0];
	else if ((stream == 4) && (bw == BW_40))
		rate_table = rssi_to_phyrate_vht_ss4_40[0];
	else if ((stream == 4) && (bw == BW_20))
		rate_table = rssi_to_phyrate_vht_ss4_20[0];
	else if ((stream == 3) && (bw == BW_160))
		rate_table = rssi_to_phyrate_vht_ss3_160[0];
	else if ((stream == 3) && (bw == BW_80))
		rate_table = rssi_to_phyrate_vht_ss3_80[0];
	else if ((stream == 3) && (bw == BW_40))
		rate_table = rssi_to_phyrate_vht_ss3_40[0];
	else if ((stream == 3) && (bw == BW_20))
		rate_table = rssi_to_phyrate_vht_ss3_20[0];
	else if ((stream == 2) && (bw == BW_160))
		rate_table = rssi_to_phyrate_vht_ss2_160[0];
	else if ((stream == 2) && (bw == BW_80))
		rate_table = rssi_to_phyrate_vht_ss2_80[0];
	else if ((stream == 2) && (bw == BW_40))
		rate_table = rssi_to_phyrate_vht_ss2_40[0];
	else if ((stream == 2) && (bw == BW_20))
		rate_table = rssi_to_phyrate_vht_ss2_20[0];
	else if ((stream == 1) && (bw == BW_160))
		rate_table = rssi_to_phyrate_vht_ss1_160[0];
	else if ((stream == 1) && (bw == BW_80))
		rate_table = rssi_to_phyrate_vht_ss1_80[0];
	else if ((stream == 1) && (bw == BW_40))
		rate_table = rssi_to_phyrate_vht_ss1_40[0];
	else if ((stream == 1) && (bw == BW_20))
		rate_table = rssi_to_phyrate_vht_ss1_20[0];


	if (!rate_table) {
		err("failed to get value from vht rate table Rssi[%d], Stream[%d], BW[%d]\n", Rssi, stream, bw);
		return -1;
	}

	final_rate_table = &rate_table;
	for (i = 0; i < VHT_RSSI_LEVEL; i++) {
		if ((*(*final_rate_table + (i * 2) + 0)) < Rssi) {
			return (*(*final_rate_table + (i * 2) + 1));
		}
	}

	return -1;
}

/**
* @brief Fn to get 11n rate of a bss
*
* @param bss bss info
* @param stream number of stream
* @param bw bw supported
*
* @return rate
*/
int get_estimate_rate_11n(signed char Rssi, int stream, int bw)
{
	int i;
	const int16_t **final_rate_table = NULL;
	const int16_t *rate_table = NULL;

	if ((stream == 4) && (bw == BW_40))
		rate_table = rssi_to_phyrate_ht_ss4_40[0];
	else if ((stream == 3) && (bw == BW_40))
		rate_table = rssi_to_phyrate_ht_ss3_40[0];
	else if ((stream == 2) && (bw == BW_40))
		rate_table = rssi_to_phyrate_ht_ss2_40[0];
	else if ((stream == 1) && (bw == BW_40))
		rate_table = rssi_to_phyrate_ht_ss1_40[0];
	else if ((stream == 4) && (bw == BW_20))
		rate_table = rssi_to_phyrate_ht_ss4_20[0];
	else if ((stream == 3) && (bw == BW_20))
		rate_table = rssi_to_phyrate_ht_ss3_20[0];
	else if ((stream == 2) && (bw == BW_20))
		rate_table = rssi_to_phyrate_ht_ss3_20[0];
	else if ((stream == 1) && (bw == BW_20))
		rate_table = rssi_to_phyrate_ht_ss1_20[0];


	if (!rate_table) {
		err("failed to get value from rate table Rssi[%d], Stream[%d], BW[%d]\n", Rssi, stream, bw);
		return 0;
	}

	final_rate_table = &rate_table;
	for (i = 0; i < HT_RSSI_LEVEL; i++) {
		if ((*(*final_rate_table + (i * 2) + 0)) < Rssi)
			return (*(*final_rate_table + (i * 2) + 1));
	}

	return -1;
}

int get_phyrate(int wireless_mode, signed char Rssi, int stream, int bw)
{
	if (wireless_mode == MODE_HE) {
		return get_estimate_rate_11ax(Rssi, stream, bw);
#ifdef MAP_320BW /*TODO fucntion need to update according to 11be and spatial stream*/
	} else if (wireless_mode == MODE_EHT) {
		return get_estimate_rate_11ax(Rssi, stream, bw);
#endif
	} else if (wireless_mode == MODE_VHT) {
		return get_estimate_rate_11ac(Rssi, stream, bw);
	} else if ((wireless_mode == MODE_HTMIX) || (MODE_HTGREENFIELD == wireless_mode)) {
		return get_estimate_rate_11n(Rssi, stream, bw);
	} else if (wireless_mode == MODE_OFDM) {
		return get_estimate_rate_11g(Rssi);
	} else if (wireless_mode == MODE_CCK) {
		return get_estimate_rate_11b(Rssi);
	}
	return -1;
}

int check_blacklist_ap(struct blacklisted_ap_list *bl_ap,
	struct own_1905_device *ctx,
	struct scan_bss_list *list_bss)
{
	struct os_time now;
	struct blacklisted_ap_list *tmp_bl_ap = NULL;
	SLIST_FOREACH_SAFE(bl_ap, &ctx->bl_ap_list, next_bl_ap, tmp_bl_ap) {
		os_get_time(&now);
		err(BST_AP_SLCT_PREX" For Blacklisted AP: ("MACSTR")", MAC2STR(bl_ap->bssid));
		if (!os_memcmp(bl_ap->bssid, list_bss->bss.Bssid, ETH_ALEN) && bl_ap->fail_cnt > MIN_BL_FAIL_CNT) {
			if (now.sec - bl_ap->time.sec < ctx->bl_timeout) {
				err(BST_AP_SLCT_PREX" Still in block phase");
				return 1;
			} else {
				err(BST_AP_SLCT_PREX" Block phase removed");
				bl_ap->fail_cnt = 0;
				SLIST_REMOVE(&ctx->bl_ap_list, bl_ap, blacklisted_ap_list, next_bl_ap);
				os_free(bl_ap);
				return 0;
			}
		}
	}
	return 0;
}


/**
* @brief Fn to issue connect to a backhaul AP
*
* @param ctx own 1905 device ctx
* @param info backhaul info
* @param bssid bssid of the AP
*
* @return 0 if success else error
*/
signed int get_matching_bss_profile(
	struct own_1905_device *ctx, struct bss_info *bss)
{
	int i = 0;
	for (i = 0; i < ctx->bh_config_count; i++)
	{
		if (SSID_EQUAL(ctx->bh_configs[i].ssid,
			ctx->bh_configs[i].SsidLen,
			bss->Ssid, bss->SsidLen) &&
			(ctx->bh_configs[i].EncrType & bss->EncrypType))
		{
			if(ctx->bh_configs[i].AuthType & bss->AuthMode) {
				return i;
			} else if(bss->AuthMode & WSC_AUTHTYPE_SAE) {
				ctx->bh_configs[i].AuthType = WSC_AUTHTYPE_SAE;
				return i;
			}
		}
	}
	return -1;
}

struct blacklisted_ap_list *lookup_for_bl_ap(struct own_1905_device *ctx)
{
	struct blacklisted_ap_list *bl_ap = NULL, *tmp_bl_ap = NULL;
	SLIST_FOREACH_SAFE(bl_ap, &ctx->bl_ap_list, next_bl_ap, tmp_bl_ap) {
		debug(BST_AP_SLCT_PREX" Blacklisted AP: ("MACSTR")", MAC2STR(bl_ap->bssid));
		if (!os_memcmp(bl_ap->bssid, ctx->conn_attempted_mac, ETH_ALEN)) {
			return bl_ap;
		}
	}
	return NULL;
}

void issue_connect_timeout_handle(void *global_ctx, void *timer_ctx)
{
	struct own_1905_device *ctx = (struct own_1905_device *)global_ctx;
	struct blacklisted_ap_list *bl_ap = NULL;
	struct os_time now;
	struct bh_link_entry *bh_entry = NULL, *tmp_bh_entry = NULL;
	struct scan_bss_list *list_bss, *tmp_list_bss = NULL;
	int count = 0;

	err(BST_AP_SLCT_PREX" *");

#ifdef MAP_R3
	if (ctx->ReconfigTrigger == FALSE) 
	{
#endif
	
		if (is_mbh_conn_triggered(ctx)) {
			mapd_printf(MSG_OFF, "Connect failed for Duplicate Link %s; Retrigger Scan", ctx->bh_dup_entry->ifname);
			ap_selection_reset_scan(ctx);
			eloop_register_timeout(1, 0, ap_selection_retrigger_scan, ctx, NULL);
			ctx->current_bh_substate = BH_SUBSTATE_IDLE;
			return;
		}
		if(ctx->ThirdPartyConnection) {
			ctx->ConnectThirdPartyVend = 0;
		}

		SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, tmp_bh_entry) {
			if (!bh_entry->priority_info.attempt_allowed)
				continue;

			SLIST_FOREACH_SAFE(list_bss, &(bh_entry->scan_bss_list_head), next_bss, tmp_list_bss) {
				count++;
			}
		}

		bl_ap = lookup_for_bl_ap(ctx);
		os_get_time(&now);
		if (bl_ap == NULL) {
			bl_ap = (struct blacklisted_ap_list *)os_zalloc(sizeof(struct blacklisted_ap_list));
			if (!bl_ap) {
				err(BST_AP_SLCT_PREX" Mem alloc failed");
				return;
			}
			os_memcpy(bl_ap->bssid, ctx->conn_attempted_mac, ETH_ALEN);
			bl_ap->time = now;
			err(BST_AP_SLCT_PREX" Added Blacklisted AP: ("MACSTR")", MAC2STR(bl_ap->bssid));
			SLIST_INSERT_HEAD(&ctx->bl_ap_list, bl_ap, next_bl_ap);
			bl_ap->fail_cnt++;
		} else {
			bl_ap->time = now;
			bl_ap->fail_cnt++;
		}
		/*Disable APCli*/
		wlanif_disconnect_apcli((struct mapd_global *)ctx->back_ptr, NULL);
		ctx->last_connected_bh_entry = NULL;
		/*Do Full Channel Scan*/
		ctx->current_bh_state = BH_STATE_WIFI_BOOTUP;
		ctx->current_bh_substate = BH_SUBSTATE_IDLE;
		eloop_register_timeout(1, 0, ap_selection_retrigger_scan, ctx, NULL);
#ifdef MAP_R3
	}
#endif
}

#ifdef MAP_6E_SUPPORT
/* Remove this func once 6E TK is ready, this HACK is used to force conn on 6G */
signed int get_matching_bss_profile_6g(
	struct own_1905_device *ctx, char *Ssid, unsigned char SsidLen,
	unsigned short EncrypType, unsigned short AuthMode)
{
	int i = 0;

	for (i = 0; i < ctx->bh_config_count; i++) {
		if (SSID_EQUAL(ctx->bh_configs[i].ssid,
			ctx->bh_configs[i].SsidLen,
			Ssid, SsidLen) &&
			(ctx->bh_configs[i].EncrType & EncrypType)) {
			if (ctx->bh_configs[i].AuthType & AuthMode)
				return i;
			else if (AuthMode & WSC_AUTHTYPE_SAE) {
				ctx->bh_configs[i].AuthType = WSC_AUTHTYPE_SAE;
				return i;
			}
		}
	}
	return -1;
}
/* Remove this func once 6E TK is ready, this HACK is used to force conn on 6G */
void issue_connect_6g(struct own_1905_device *ctx)
{
	struct backhaul_connect_request bconnect_req;
	struct wsc_apcli_config *bh_config = NULL;
	char ssid[32] = "Multi-AP-1";
	unsigned char Bssid[ETH_ALEN] = {0xC2, 0x0C, 0x43, 0x69, 0xA6, 0xFC};
	unsigned char sta_mac[ETH_ALEN] = {0xCA, 0x0C, 0x43, 0x49, 0xA6, 0xD8};
	unsigned char ssidlen = os_strlen(ssid);
	unsigned short EncrypType = 0x0008;
	unsigned short AuthMode = WSC_AUTHTYPE_SAE;
	signed int profile_idx = get_matching_bss_profile_6g(ctx, ssid, ssidlen, EncrypType, AuthMode);

	err("profile_idx:%d", profile_idx);
	if (profile_idx < 0 || profile_idx >= MAX_NUM_OF_RADIO) {
		err(BST_AP_SLCT_PREX" profile_idx invalid %d", profile_idx);
		return;
	}

	bh_config = &ctx->bh_configs[profile_idx];
	ctx->bh_ready_expected = TRUE;
	os_memset(&bconnect_req, 0, sizeof(bconnect_req));
	os_memcpy(bconnect_req.target_bssid, Bssid, ETH_ALEN);
	os_memcpy(bconnect_req.target_ssid, ssid, ssidlen);
	bconnect_req.AuthType = AuthMode;
	bconnect_req.EncrType = EncrypType;
	bconnect_req.channel = 37;

	os_memcpy(bconnect_req.backhaul_mac, sta_mac, ETH_ALEN);
	os_memcpy(bconnect_req.Key, bh_config->Key, bh_config->KeyLength);
	bconnect_req.Key[bh_config->KeyLength] = '\0';
	ctx->current_bh_substate = BH_SUBSTATE_CONNECT_WAIT;

	err(BST_AP_SLCT_PREX" issuing conn on 6G");
	err("Bssid "MACSTR"BH MAC" MACSTR, MAC2STR(Bssid), MAC2STR(sta_mac));
	err("Key %s", bconnect_req.Key);
	eloop_cancel_timeout(issue_connect_timeout_handle, ctx, NULL);
	wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_BH_CONNECT_REQUEST,
				0, sta_mac, 0, &bconnect_req, sizeof(struct backhaul_connect_request), 0, 0, 0);

	eloop_register_timeout(30, 0, issue_connect_timeout_handle, ctx, NULL);
}
#endif

void ap_selection_fill_bconnec_req(struct own_1905_device *ctx,
	struct bh_link_entry *info,
	struct scan_bss_list *selected_bss,
	struct backhaul_connect_request *bconnect_req,
	struct wsc_apcli_config *bh_config)
{
	unsigned char central_ch, vht_bw, ht_bw;

#ifdef MAP_6E_SUPPORT
	if ((IS_MAP_CH_5G(selected_bss->bss.Channel) && IS_BAND_5G(info->bh_band)) ||
		(IS_MAP_CH_6G(selected_bss->bss.Channel) && IS_BAND_6G(info->bh_band)))
#else
	if (selected_bss->bss.Channel > 14)
#endif
	{
		if (selected_bss->bss.vht_cap.vht_160 || selected_bss->bss.he_cap.he_160) {
#ifdef MAP_6E_SUPPORT
			central_ch = ch_planning_get_centre_freq_ch_by_band(selected_bss->bss.Channel, info->bh_band, BW_160);
			vht_bw = ctx->max_vht_bw_6g;
			ht_bw = ctx->max_ht_bw_6g;
#else
			central_ch = ch_planning_get_centre_freq_ch_by_bw(selected_bss->bss.Channel, BW_160);
			vht_bw = ctx->max_vht_bw_5gh;
			ht_bw = ctx->max_ht_bw_5gh;
#endif
			if (central_ch == selected_bss->bss.CentralChannel && vht_bw == bw_160)
				bconnect_req->bw = BW_160;
			else if (vht_bw >= bw_80)
				bconnect_req->bw = BW_80;
			else if (ht_bw == bw_40)
				bconnect_req->bw = BW_40;
			else
				bconnect_req->bw = BW_20;
		} else if (selected_bss->bss.ht_cap.ht_40) {
#ifdef MAP_6E_SUPPORT
			central_ch = ch_planning_get_centre_freq_ch_by_band(selected_bss->bss.Channel, info->bh_band, BW_80);
			vht_bw = ctx->max_vht_bw_6g;
			ht_bw = ctx->max_ht_bw_6g;
#else
			central_ch = ch_planning_get_centre_freq_ch_by_bw(selected_bss->bss.Channel, BW_80);
			vht_bw = ctx->max_vht_bw_5gh;
			ht_bw = ctx->max_ht_bw_5gh;
#endif
			if (central_ch == selected_bss->bss.CentralChannel &&  vht_bw >= bw_80)
				bconnect_req->bw = BW_80;
			else if (ht_bw == bw_40)
				bconnect_req->bw = BW_40;
			else
				bconnect_req->bw = BW_20;
		} else
			bconnect_req->bw = BW_20;
	} else {
		 if (selected_bss->bss.ht_cap.ht_40 && ctx->max_ht_bw_2gh == bw_40)
			bconnect_req->bw = BW_40;
		 else
			bconnect_req->bw = BW_20;

	}

#ifdef MAP_R3
	bconnect_req->AuthType = min(selected_bss->bss.AuthMode, bh_config->AuthType);
#else
	bconnect_req->AuthType = bh_config->AuthType;
#endif
	bconnect_req->EncrType = bh_config->EncrType;
	bconnect_req->channel = selected_bss->bss.Channel;
}

int issue_connect(struct own_1905_device *ctx,
	struct bh_link_entry *info,
	struct scan_bss_list *selected_bss)
{
	struct backhaul_connect_request bconnect_req;
	signed int profile_idx = get_matching_bss_profile(ctx,
		&selected_bss->bss);
	struct wsc_apcli_config *bh_config = NULL;

	if(profile_idx < 0 || profile_idx >= MAX_NUM_OF_RADIO) {
		err(BST_AP_SLCT_PREX" profile_idx invalid %d", profile_idx);
		return -1;
	}
	
	bh_config = &ctx->bh_configs[profile_idx];

	ctx->bh_ready_expected = TRUE;
	os_memcpy(&info->bss, &selected_bss->bss, sizeof(struct bss_info));
	os_memset(&bconnect_req, 0, sizeof(bconnect_req));
	os_memcpy(bconnect_req.target_bssid, selected_bss->bss.Bssid, ETH_ALEN);
	if (selected_bss->bss.SsidLen <= MAX_SSID_LEN)
		os_memcpy(bconnect_req.target_ssid,
			selected_bss->bss.Ssid, selected_bss->bss.SsidLen);
	else		
		os_memcpy(bconnect_req.target_ssid,
			selected_bss->bss.Ssid, MAX_SSID_LEN);
	os_memcpy(bconnect_req.backhaul_mac, info->mac_addr, ETH_ALEN);
	os_memcpy(bconnect_req.Key, bh_config->Key, bh_config->KeyLength);
	bconnect_req.Key[bh_config->KeyLength] = '\0';

	info(BST_AP_SLCT_PREX" connection BSSID = "MACSTR"\n",
		MAC2STR(selected_bss->bss.Bssid));
	info(BST_AP_SLCT_PREX" connection SSID = %s\n", selected_bss->bss.Ssid);
	info(BST_AP_SLCT_PREX" central channel = %d\n", selected_bss->bss.CentralChannel);
	err(BST_AP_SLCT_PREX" connection ht_40 = %d vht_160 %d vht_8080 %d he_160 %d\n"
												, selected_bss->bss.ht_cap.ht_40
												, selected_bss->bss.vht_cap.vht_160
												, selected_bss->bss.vht_cap.vht_8080
												, selected_bss->bss.he_cap.he_160);
	err(BST_AP_SLCT_PREX" max bw vht_5g = %d ht_5g %d ht_2g %d\n"
											, ctx->max_vht_bw_5gh
											, ctx->max_ht_bw_5gh
											, ctx->max_ht_bw_2gh);

	ap_selection_fill_bconnec_req(ctx, info, selected_bss, &bconnect_req, bh_config);

	ctx->current_bh_substate = BH_SUBSTATE_CONNECT_WAIT;

	err(BST_AP_SLCT_PREX" connection interface is %s", info->ifname);
	eloop_cancel_timeout(issue_connect_timeout_handle, ctx, NULL);
	wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_BH_CONNECT_REQUEST,
				0, info->mac_addr, 0, &bconnect_req, sizeof(struct backhaul_connect_request), 0, 0, 0);

	eloop_register_timeout(30, 0, issue_connect_timeout_handle, ctx, NULL);
	//wappctrl rax0 wapp set_ch rax0 36
	return 0;
}

char * get_iface_from_index(unsigned int index, char *ifname)
{
	return if_indextoname(index, ifname);
}

unsigned char bss_profile_matches(
	struct own_1905_device *ctx, struct bss_info *bss)
{
	int i = 0;
	for (i = 0; i < ctx->bh_config_count; i++)
	{
		if (SSID_EQUAL(ctx->bh_configs[i].ssid,
					ctx->bh_configs[i].SsidLen,
					bss->Ssid, bss->SsidLen) &&
				(ctx->bh_configs[i].EncrType & bss->EncrypType))
		{
			if(ctx->bh_configs[i].AuthType & bss->AuthMode) {
				if ((bss->AuthMode & WSC_AUTHTYPE_SAE)
#ifdef MAP_R3
						&& !(ctx->bh_configs[i].AuthType & WSC_AUTHTYPE_DPP)
#endif
				   ) {
					ctx->bh_configs[i].AuthType = WSC_AUTHTYPE_SAE;
				}
				return TRUE;
			} else if((bss->AuthMode & WSC_AUTHTYPE_SAE)
#ifdef MAP_R3
					&& !(ctx->bh_configs[i].AuthType & WSC_AUTHTYPE_DPP)
#endif
				 ) {
				ctx->bh_configs[i].AuthType = WSC_AUTHTYPE_SAE;
				return TRUE;
			} else {
				err(BST_AP_SLCT_PREX" Profile not matched due to BSS AuthMode 0x%x SSID ----> %s", bss->AuthMode, bss->Ssid);
				if (ctx->bh_configs[i].AuthType == WSC_AUTHTYPE_SAE) {
					ctx->bh_configs[i].AuthType = bss->AuthMode;
					err("cxt AuthMode is WSC_AUTHTYPE_SAE, but BSS AuthMode is Not WSC_AUTHTYPE_SAE yet, change ctx AuthMode to 0x%x", ctx->bh_configs[i].AuthType);
					return TRUE;
				}
			}
		} else {
			info(BST_AP_SLCT_PREX" Profile not matched SSID ----> %s", bss->Ssid);
		}
	}
	return FALSE;
}

void send_bh_steering_fail(void *eloop_ctx, void *timeout_ctx)
{
	struct bh_link_entry *bh_entry = NULL, *tmp_bh_entry = NULL;
	struct own_1905_device *ctx = (struct own_1905_device *) eloop_ctx;
	struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;
	err(BH_STEER_PREX" Unable to connect to Target BSSID");
	memcpy(ctx->bsteer_rsp.backhaul_mac, ctx->bsteer_req.backhaul_mac, ETH_ALEN);
	memcpy(ctx->bsteer_rsp.target_bssid, ctx->bsteer_req.target_bssid, ETH_ALEN);
	ctx->bsteer_rsp.status = 0x01;
	map_1905_Set_Bh_Steer_Rsp_Info(mapd_ctx->_1905_ctrl, &ctx->bsteer_rsp);

	/* Reset states and bh varables before fresh connection with existing 1905 dev */
	ctx->current_bh_substate = BH_SUBSTATE_LINKDOWN_WAIT;
	SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, tmp_bh_entry) {
		if (bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED)
		{
			err(BH_STEER_PREX" BH Entry ifname %s Bssid"MACSTR, bh_entry->ifname, MAC2STR(bh_entry->bss.Bssid));
			ctx->current_bh_state = BH_STATE_WIFI_LINKUP;
			break;
		}
	}
	wlanif_disconnect_apcli(mapd_ctx, NULL);
	ctx->current_bh_state = BH_STATE_WIFI_LINK_FAIL;
}
#if MAP_6E_SUPPORT
Boolean ap_selection_is_scan_chan_valid(struct _1905_map_device *_1905dev,
	unsigned char bss_channel, unsigned char bh_channel, unsigned char bh_band)
{
	if (IS_MAP_CH_5G(bss_channel) && IS_MAP_CH_5G(bh_channel) && IS_BAND_5G(bh_band))
		return TRUE;
	else if (IS_MAP_CH_6G(bss_channel) && IS_MAP_CH_6G(bh_channel) && IS_BAND_6G(bh_band))
		return TRUE;
	else if (IS_MAP_CH_24G(bss_channel) && IS_MAP_CH_24G(bh_channel) && IS_BAND_24G(bh_band))
		return TRUE;
	else
		return FALSE;
}
#endif
/**
* @brief Fn to parse scan results from wapp
*
* @param ctx own 1905 device ctx
* @param info wapp scan info
*
* @return 0
*/
int ap_selection_parse_scan_result(struct own_1905_device *ctx, struct wapp_scan_info *info)
{
	int i;
	struct os_time now;
	char iface_name[64];
	struct bh_link_entry *bh_entry = NULL, *tmp_bh_entry = NULL;
	struct _1905_map_device * own_1905_map_device = topo_srv_get_1905_device(ctx, NULL);

	if (ctx->device_role == DEVICE_ROLE_CONTROLLER && ctx->ThirdPartyConnection == 0)
		return -1;

	os_get_time(&now);
	get_iface_from_index(info->interface_index, iface_name);
	SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, tmp_bh_entry) {
		if (strcmp((const char *)bh_entry->ifname, iface_name) == 0) {
			break;
		}
	}

	if (bh_entry == NULL)
	{
		err(BST_AP_SLCT_PREX" scan_result from unknown interface\n");
		return -1;
	}
	/* Periodic exe should kick out old bss */
	info("Num of bss from driver %d\n", info->bss_count);
	for (i = 0; i < info->bss_count; i++) {
		info("bss address is "MACSTR"\n", MAC2STR(info->bss[i].Bssid));
#ifdef MAP_6E_SUPPORT
		if (ap_selection_is_scan_chan_valid(own_1905_map_device, info->bss[i].Channel,
			bh_entry->bh_channel, bh_entry->bh_band) == FALSE)
			continue;
#else
		if(check_is_triband(own_1905_map_device) &&
			isChan5GL(info->bss[i].Channel) &&
			isChan5GL(bh_entry->bh_channel)){
		} else if (check_is_triband(own_1905_map_device) &&
				 isChan5GH(info->bss[i].Channel) &&
				 isChan5GH(bh_entry->bh_channel)) {
		} else if ((bh_entry->bh_channel > 14) && (info->bss[i].Channel > 14)) {
		} else if ((bh_entry->bh_channel <= 14) && (info->bss[i].Channel <= 14)) {
		} else {
			continue;
		}
#endif
		if (bss_profile_matches(ctx, &info->bss[i])) {
			struct scan_bss_list *bss_entry = os_zalloc(sizeof(struct scan_bss_list));
			bss_entry->bh_entry = bh_entry;
			bss_entry->time = now;
			os_memcpy(&bss_entry->bss, &info->bss[i], sizeof(struct bss_info));
			SLIST_INSERT_HEAD(&bh_entry->scan_bss_list_head, bss_entry, next_bss);
			info("bss_entry map_vendor IE found %d\n", bss_entry->bss.map_vendor_ie_found);

			info("bss_entry controller connectivity %d\n", bss_entry->bss.map_info.connectivity_to_controller);
		}
	}
	return 0;
}

struct local_interface * get_map_filter_iface(struct own_1905_device *dev, u8 *mac_addr)
{
	int i = 0;

	for (i = 0; i < dev->num_eth_itfs; i++) {
		if (!os_memcmp(dev->eth_itfs[i].mac, mac_addr, ETH_ALEN))
			return &dev->eth_itfs[i];
	}
	for(i = 0; i < dev->num_wifi_itfs; i++) {
		struct local_interface *wifi_if = dev->wifi_itfs[i];
		if (!os_memcmp(wifi_if->mac, mac_addr, ETH_ALEN))
			return wifi_if;

	}
	return NULL;
}

void mbh_handle_link_change(struct own_1905_device *dev)
{
	struct bh_link_entry *link_5g_up = NULL;
	struct bh_link_entry *link_2g_up = NULL;
	struct bh_link_entry *bh_entry = NULL, *tmp_bh_entry = NULL;
	struct bh_link_entry *primary_link_down = NULL;
	struct local_interface *temp = NULL;
	int i = 0;

	mapd_printf(MSG_OFF, "ApCli Link State change; Change Primary Link Info");

	SLIST_FOREACH_SAFE(bh_entry, &(dev->bh_link_head), next_bh_link, tmp_bh_entry) {
			if (bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED) {
				if (bh_entry->bh_channel > 14)
					link_5g_up = bh_entry;
				else
					link_2g_up = bh_entry;
				temp = get_map_filter_iface(dev, bh_entry->mac_addr);
				if (temp != NULL)
					if (mapfilter_set_apcli_link_status(temp, 1) < 0)
						err("error in callig func=%d\n", __LINE__);
			} else if (bh_entry->bh_assoc_state == WAPP_APCLI_DISASSOCIATED) {
				if (bh_entry == dev->primary_link)
					primary_link_down = bh_entry;
				temp = get_map_filter_iface(dev, bh_entry->mac_addr);
				if (temp != NULL)
					if (mapfilter_set_apcli_link_status(temp, 0) < 0)
						err("error in callig func=%d\n", __LINE__);
			}
	}

	/* Set Primary Link */
	if (primary_link_down) {
		/* Existing Primary Link has gone down */
		mapd_printf(MSG_OFF, "Primary link %s has gone down", primary_link_down->ifname);
		mapfilter_set_primary_interface(get_map_filter_iface(dev, primary_link_down->mac_addr), 0);
		dev->primary_link = NULL;
	}
	if (!dev->primary_link || (link_5g_up && dev->primary_link != link_5g_up)) {
		/* Give Priority to 5G */
		if (link_5g_up) {
			mapfilter_set_primary_interface(get_map_filter_iface(dev, link_5g_up->mac_addr), 1);
			dev->primary_link = link_5g_up;
		} else if(link_2g_up) {
			mapfilter_set_primary_interface(get_map_filter_iface(dev, link_2g_up->mac_addr), 1);
			dev->primary_link = link_2g_up;
		}
	}
#if 0
	/* 5G Primary */
	if (link_5g_up && link_2g_up) {
		if (link_5g_up != dev->primary_link)
			mapfilter_set_primary_interface(get_map_filter_iface(dev, link_5g_up->mac_addr), 1);
	} else {
			/* Atmost 1 link up */
			if (link_5g_up && (link_5g_up != dev->primary_link)) {
					mapfilter_set_primary_interface(get_map_filter_iface(dev, link_5g_up->mac_addr), 1);
					dev->primary_link = link_5g_up;
			} else if (link_2g_up && (link_2g_up != dev->primary_link)) {
					mapfilter_set_primary_interface(get_map_filter_iface(dev, link_2g_up->mac_addr), 1);
					dev->primary_link = link_2g_up;
			}
	}
#endif
	if (dev->primary_link) {
		mapd_printf(MSG_OFF, "Primarly Link Setup done-->%s", dev->primary_link->ifname);
	} else {
		mapd_printf(MSG_OFF, "Primarly Link Setup done-->None(No Active Links)");
	}

	/* Dual BH */
	if (link_5g_up && link_2g_up) {
		/* ETH to 5G */
		for (i = 0; i < dev->num_eth_itfs; i++) {
			mapd_printf(MSG_OFF, "Setting uplink information %s %s", dev->eth_itfs[i].name, link_5g_up->ifname);
			mapfilter_set_uplink_path(&dev->eth_itfs[i], get_map_filter_iface(dev, link_5g_up->mac_addr));
		}
		for (i = 0; i < dev->num_wifi_itfs; i++) {
			if (dev->wifi_itfs[i]->dev_type != AP)
				continue;
			/* 5G to 5G */
			/* 2G to 2G */
			if (dev->wifi_itfs[i]->band == _24G)
				mapfilter_set_uplink_path(dev->wifi_itfs[i], get_map_filter_iface(dev, link_2g_up->mac_addr));
			else
				mapfilter_set_uplink_path(dev->wifi_itfs[i], get_map_filter_iface(dev, link_5g_up->mac_addr));
		}
	} else {
		/* Only 1 up */
		for (i = 0; i < dev->num_eth_itfs; i++) {
			mapd_printf(MSG_OFF, "Setting uplink information %s NULL", dev->eth_itfs[i].name);
			mapfilter_set_uplink_path(&dev->eth_itfs[i], NULL);
		}
		for (i = 0; i < dev->num_wifi_itfs; i++) {
			if (dev->wifi_itfs[i]->dev_type != AP)
				continue;
			if (dev->wifi_itfs[i]->band == _24G) {
				if (link_2g_up)
					mapfilter_set_uplink_path(dev->wifi_itfs[i], get_map_filter_iface(dev, link_2g_up->mac_addr));
				else
					mapfilter_set_uplink_path(dev->wifi_itfs[i], NULL);
			} else {
				if (link_5g_up)
					mapfilter_set_uplink_path(dev->wifi_itfs[i], get_map_filter_iface(dev, link_5g_up->mac_addr));
				else
					mapfilter_set_uplink_path(dev->wifi_itfs[i], NULL);
			}
		}
	}
}

Boolean is_mbh_conn_triggered(struct own_1905_device *ctx)
{
	if ((ctx->dual_bh_en) &&
        (ctx->current_bh_state == BH_STATE_WIFI_LINKUP) &&
        (ctx->bh_dup_entry))
		return TRUE;
	return FALSE;
}

int get_mbh_state(struct own_1905_device *ctx, struct bh_link_entry **uncon,
				int *num_act_links)
{
	struct bh_link_entry *bh_entry = NULL, *tmp_bh_entry = NULL;
	int total_links = 0, num_pending_links = 0, num_blocked_links = 0;

	*uncon = NULL;
	/* Not Enabled */
	if (!ctx->dual_bh_en)
		return MBH_DISABLED;

	if (ctx->current_bh_state != BH_STATE_WIFI_LINKUP)
		return MBH_NOT_ALLOWED;

	/* Get Individual Link */
	SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, tmp_bh_entry) {
			total_links ++;
			if (bh_entry->mbh_blocked == 1) {
				num_blocked_links ++;
				continue;
			}
			if (bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED) {
				(*num_act_links)++;
				continue;
			}
			if (bh_entry->bh_assoc_state == WAPP_APCLI_DISASSOCIATED) {
				*uncon = bh_entry;
				num_pending_links ++; // This could already be triggered
				continue;
			}
	}

	/* Triggered(Only 1 is triggered at a time)  */
	if (ctx->bh_dup_entry) {
		*uncon = ctx->bh_dup_entry;
		return MBH_TRIGGERED;
	}

	if (total_links == *num_act_links) {
		ctx->sec_link_scan_cnt = 0;
		ctx->sec_bh_link_restore = FALSE;
		eloop_cancel_timeout(restore_sec_bh_link, ctx, NULL);
		return MBH_COMPLETED;
	} else if (num_pending_links > 0)
		return MBH_PENDING;
	else if (total_links == num_blocked_links)
		return MBH_NOT_ALLOWED;
	else
		mapd_ASSERT(0);

	return MBH_NOT_ALLOWED;
}
/* Return success if BH Steer is triggered by Network Optimization or Band Switching or BH Steer*/
int is_bh_steer_triggered(struct own_1905_device *ctx)
{

	if(	ctx->current_bh_state == BH_STATE_WIFI_BH_STEER ||
		ctx->current_bh_state == BH_STATE_WIFI_BAND_SWITCHED) {
		return 1;
	} else {
		return 0;
	}
}

struct bh_link_entry *get_connected_bh_entry(struct own_1905_device *ctx)
{
	struct bh_link_entry *bh_entry = NULL, *tmp_bh_entry = NULL;

	SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, tmp_bh_entry) {
		if (bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED) {
			debug("Last Connected BSS" MACSTR,MAC2STR(bh_entry->bssid));
			return bh_entry;
		}
	}
	return bh_entry;
}

/*
	Scan in Connected state
	Select BSS to be selected. If BSS found Link Down
	After Link Down Event From Driver issue connect.
*/
int issue_scan_connect_for_backhaul(struct own_1905_device *ctx)
{
	struct new_bh *new_bh_info = &ctx->new_bh_info;
	struct bh_link_entry *bh_entry;
	struct bh_link_entry *selected_bh_entry = NULL;
	struct scan_bss_list *selected_bss = NULL;
	struct mapd_global *pglobal;
	unsigned char target_band_same = 0;
	pglobal = (struct mapd_global *)(ctx->back_ptr);

	bh_entry = NULL;
	/* In case of Network Optimization only disocnnect here and wait for LinkDown*/
	if(ctx->current_bh_substate == BH_SUBSTATE_SCAN_DONE) {
		selected_bh_entry = ap_selection_find_best_ap(ctx, &selected_bss);

		if(selected_bh_entry){
			bh_entry = get_connected_bh_entry(ctx);
			if(bh_entry) {
				err(BST_AP_SLCT_PREX" curr intf %s target intf %s",bh_entry->ifname, selected_bh_entry->ifname);
				if(os_memcmp(bh_entry->ifname, selected_bh_entry->ifname , sizeof(bh_entry->ifname))== 0){
					target_band_same = 1;
				}
			} else {
				err(BST_AP_SLCT_PREX" Bh Entry Not Found");
			}
			err(BST_AP_SLCT_PREX" target band %d dual_bh_en = %d Selected Bss " MACSTR,
							target_band_same, ctx->dual_bh_en,
							MAC2STR(selected_bss->bss.Bssid));
			if(target_band_same || ctx->dual_bh_en) {
				ctx->last_connected_bh_entry = NULL;
				/*If target band is same band backup the selected bh entry and BSS for connection after link down*/
				/* Disconnect Link here and wait for Link Down*/
				new_bh_info->new_selected_bh_entry= selected_bh_entry;
				os_memcpy(&new_bh_info->new_selected_bss,selected_bss,sizeof(struct scan_bss_list));
				err(BST_AP_SLCT_PREX" Trigger All Links Down");
				wlanif_disconnect_apcli(pglobal, NULL);
				ctx->current_bh_substate = BH_SUBSTATE_LINKDOWN_WAIT;
			} else {
				/*If target band is not same first connect and then break the existing link*/
				err(BST_AP_SLCT_PREX" issue connect Connect Wait");
				ctx->last_connected_bh_entry = get_connected_bh_entry(ctx);
				issue_connect(ctx, selected_bh_entry, selected_bss);
			}
		} else {
				err(BST_AP_SLCT_PREX" BSS Not Found For BH Steer");
				ctx->current_bh_substate = BH_SUBSTATE_IDLE;
				return 1;
		}
	}
	else if(ctx->current_bh_substate == BH_SUBSTATE_LINKDOWN_WAIT) {
		err(BST_AP_SLCT_PREX" Issue Connect For Network Opt");
		if(new_bh_info->new_selected_bh_entry) {
			issue_connect(ctx, new_bh_info->new_selected_bh_entry, &new_bh_info->new_selected_bss);
		} else {
			err (BST_AP_SLCT_PREX" Issue connect fail BH Entry NULL");
			new_bh_info->new_selected_bh_entry = NULL;
			os_memset(&new_bh_info->new_selected_bss,0,sizeof(struct scan_bss_list));
			ctx->current_bh_substate = BH_SUBSTATE_IDLE;
		}
	}

	return 0;
}

Boolean ap_selection_is_rssi_operable(struct own_1905_device *ctx,
	struct bh_link_entry *selected_bh_entry,
	struct scan_bss_list *selected_bss,
	unsigned char is_channel_operable)
{
#ifdef MAP_6E_SUPPORT
	if (IS_MAP_CH_5G(selected_bh_entry->bh_channel) && IS_BAND_5G(selected_bh_entry->bh_band) &&
		is_channel_operable && (selected_bss->bss.Rssi > ctx->rssi_threshold_5g))
		return TRUE;
	else if (IS_MAP_CH_24G(selected_bh_entry->bh_channel) && IS_BAND_24G(selected_bh_entry->bh_band) &&
		(selected_bss->bss.Rssi > ctx->rssi_threshold_2g) && is_channel_operable)
		return TRUE;
	else if (IS_MAP_CH_6G(selected_bh_entry->bh_channel) && IS_BAND_6G(selected_bh_entry->bh_band) &&
		is_channel_operable && (selected_bss->bss.Rssi > ctx->rssi_threshold_6g))
		return TRUE;
#else
	if (selected_bh_entry->bh_channel > 14 && is_channel_operable &&
		selected_bss->bss.Rssi > ctx->rssi_threshold_5g) {
		return TRUE;
	} else if (selected_bh_entry->bh_channel <= 14 && selected_bss->bss.Rssi > ctx->rssi_threshold_2g
			&& is_channel_operable) {
		return TRUE;
	}
#endif
	else {
		err(BST_AP_SLCT_PREX" Do Not issue connect Ch:[%d] Rssi:[%d] is_channel_operable %d",
			selected_bh_entry->bh_channel, selected_bss->bss.Rssi, is_channel_operable);
		return FALSE;
	}
}
/**
* @brief Fn to find best ap and issue connect to that
*
* @param ctx own 1905 device ctx
* @param cookie scan cookie
*
* @return 0
*/


int ap_selection_issue_connect(struct own_1905_device *ctx)
{
	struct bh_link_entry *selected_bh_entry = NULL;
	struct scan_bss_list *selected_bss = NULL;
	struct scan_bss_list *list_bss = NULL, *tmp_list_bss = NULL;
	unsigned char is_channel_operable = 1;

	/* Connect on duplicate link */
	if (is_mbh_conn_triggered(ctx)) {
		struct _1905_map_device *peer_map_device = NULL;
		struct _1905_map_device *own_map_device = topo_srv_get_1905_device(ctx, NULL);

		SLIST_FOREACH_SAFE(list_bss, &(ctx->bh_dup_entry->scan_bss_list_head), next_bss, tmp_list_bss) {
			err(BST_AP_SLCT_PREX" peer MAC --->"MACSTR"\n",
							MAC2STR(list_bss->bss.Bssid));
#ifdef NOT_POSS
	/* Not possible because topology doesn't have the BH BSS information */
			if (os_memcmp(ctx->bh_dup_bssid, list_bss->bss.Bssid, ETH_ALEN)) {
				continue;
			}
#endif
#ifdef WIFI_MD_COEX_SUPPORT
			if (!ch_planning_check_channel_for_dev_operable_wrapper(ctx, topo_srv_get_1905_device(ctx, NULL),
				list_bss->bss.Channel)) {
				err(BST_AP_SLCT_PREX" bss" MACSTR " channel=%d is not supported!",
					MAC2STR(list_bss->bss.Bssid), list_bss->bss.Channel);
				continue;
			}
#endif
			peer_map_device = topo_srv_get_1905_by_bssid(ctx, list_bss->bss.Bssid);

			if (!peer_map_device)
				continue;
			if (peer_map_device != own_map_device->upstream_device)
				continue;

			selected_bss = list_bss;
			mapd_printf(MSG_OFF, BST_AP_SLCT_PREX" Issue connect to dup BSS " MACSTR,
							MAC2STR(selected_bss->bss.Bssid));
			break;
		}
		if (selected_bss)
			return issue_connect(ctx, ctx->bh_dup_entry, selected_bss);
		else
			return 1;
	}

	/*  Backhaul Steer /Backhaul Switch / Network optimization will be handled in this function*/
	if(is_bh_steer_triggered(ctx)) {
		return issue_scan_connect_for_backhaul(ctx);
	}

	selected_bh_entry = ap_selection_find_best_ap(ctx, &selected_bss);
	if (selected_bh_entry != NULL)
	{
		if (ctx->ThirdPartyConnection && ctx->device_role == DEVICE_ROLE_CONTROLLER)
			is_channel_operable = ch_planning_check_channel_operable_wrapper(ctx, selected_bss->bss.Channel);

		if (selected_bh_entry->priority_info.need_rssi_consider) {
			if (ap_selection_is_rssi_operable(ctx, selected_bh_entry, selected_bss, is_channel_operable) == TRUE)
				issue_connect(ctx, selected_bh_entry, selected_bss);
			else
				return 1;
		} else
			if (selected_bss->bss.Rssi > -88 && is_channel_operable)
				issue_connect(ctx, selected_bh_entry, selected_bss);
			else {
				err(BST_AP_SLCT_PREX" Do Not issue connect Rssi[%d] is_channel_operable %d",
					selected_bss->bss.Rssi, is_channel_operable);
				return 1;
			}
		os_memcpy(ctx->conn_attempted_mac, selected_bss->bss.Bssid, ETH_ALEN);
	} else {
		err(BST_AP_SLCT_PREX" Do not issue connect, no best AP found");
		return 1;
	}

	return 0;
}


/**
* @brief Fn to issue scan on an interface
*
* @param ctx own 1905 device ctx
* @param info backhaul info
*
* @return 0 if success else error code
*/
void ap_selection_reset_scan(struct own_1905_device *ctx)
{
	struct bh_link_entry *bh_entry = NULL, *tmp_bh_entry = NULL;
	struct scan_bss_list *scan, *tmp_scan;
	SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, tmp_bh_entry) {
		bh_entry->priority_info.attempt_allowed = FALSE;
		bh_entry->priority_info.scan_triggered = FALSE;
		if (bh_entry->priority_info.need_rssi_consider == 0)
			bh_entry->priority_info.need_rssi_consider = 1;
		else
			bh_entry->priority_info.need_rssi_consider = 0;
		if (!SLIST_EMPTY(&(bh_entry->scan_bss_list_head))) {
			scan = SLIST_FIRST(&(bh_entry->scan_bss_list_head));
			while(scan) {
				tmp_scan = SLIST_NEXT(scan, next_bss);
				SLIST_REMOVE(&bh_entry->scan_bss_list_head, scan, scan_bss_list, next_bss);
				free(scan);
				scan = tmp_scan;
			}
		}
		SLIST_INIT(&bh_entry->scan_bss_list_head);
	}
	ctx->current_connect_priority = MAX_POSSIBLE_BH_PRIORITY;

	/*This is temporary fix will properly fix this during optimization activity*/
	info(BST_AP_SLCT_PREX" current bh state %d sub state %d", ctx->current_bh_state, ctx->current_bh_substate);
	if ((ctx->current_bh_state != BH_STATE_ETHERNET_PLUGGED) &&
		(ctx->current_bh_state != BH_STATE_WIFI_LINKUP)) {
		if (ctx->link_fail_single_channel_scan_count) {
			err(BST_AP_SLCT_PREX" Decrement Single Channel Scan Count %d ",ctx->link_fail_single_channel_scan_count);
			ctx->link_fail_single_channel_scan_count--;

			if (ctx->dhcp_ctl_enable
				&& ctx->link_fail_single_channel_scan_count == 0
				&& (ctx->current_bh_state == BH_STATE_WIFI_LINK_FAIL
				|| ctx->current_bh_state == BH_STATE_ETHERNET_UPLUGGED
				|| ctx->current_bh_state == BH_STATE_WIFI_BOOTUP)) {
				/*
				3rd connetion enbale & deivce role is controller, dhcp server should in 3rd-AP,  when wifi disconnected,
				controller should not enable dhcp server in it. because may cause diff-subnet issue of agents.
				*/
				if (ctx->ThirdPartyConnection == 1 && ctx->device_role == DEVICE_ROLE_CONTROLLER) {
					if (!(0 == ctx->dhcp_req.dhcp_server_enable && 1 == ctx->dhcp_req.dhcp_client_enable)) {
						mapd_printf(MSG_OFF,BST_AP_SLCT_PREX" reconn retry timeout,3rd conn enable,enable dhcp client for recon!\n");
						ctx->dhcp_req.dhcp_server_enable = 0;
						ctx->dhcp_req.dhcp_client_enable = 1;
						wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_DHCP_CTL_REQUEST,
						0, NULL, NULL, &ctx->dhcp_req, sizeof(struct dhcp_ctl_req), 0, 0, 0);
						/*get ip timeout, timer*/
						map_register_dhcp_timer(ctx);
					}
				} else {
					if (!(1 == ctx->dhcp_req.dhcp_server_enable && 0 == ctx->dhcp_req.dhcp_client_enable)) {
						mapd_printf(MSG_OFF,BST_AP_SLCT_PREX" reconn retry timeout,enable_dhcp_server!\n");
						ctx->dhcp_req.dhcp_server_enable = 1;
						ctx->dhcp_req.dhcp_client_enable = 0;
						wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_DHCP_CTL_REQUEST,
						0, NULL, NULL, &ctx->dhcp_req, sizeof(struct dhcp_ctl_req), 0, 0, 0);
					}
				}
			}
		}
	}
	ctx->scan_triggered = FALSE;
	eloop_cancel_timeout(ap_selection_retrigger_scan, ctx, NULL);
}


void ap_selection_get_uplink_ap_channel(struct own_1905_device *ctx,
	unsigned char *scan_channel_list,
	struct bh_link_entry *bh_entry)
{
	struct _1905_map_device * own_1905_map_device = topo_srv_get_1905_device(ctx, NULL);
	struct _1905_map_device * upstream_device = own_1905_map_device->upstream_device;

#ifdef MAP_6E_SUPPORT
	if (upstream_device) {
		if (IS_MAP_CH_5G(bh_entry->bh_channel) && IS_BAND_5G(bh_entry->bh_band)) {
			if (check_is_triband(own_1905_map_device)) {
				if (IS_MAP_CH_5GL(bh_entry->bh_channel))
					topo_srv_get_5g_bh_ap_channel_by_band(ctx, upstream_device, scan_channel_list, BAND_5GL);
				else
					topo_srv_get_5g_bh_ap_channel_by_band(ctx, upstream_device, scan_channel_list, BAND_5GH);
			} else
				topo_srv_get_5g_bh_ap_channel(ctx, upstream_device, scan_channel_list);
		} else if (IS_MAP_CH_6G(bh_entry->bh_channel) && IS_BAND_6G(bh_entry->bh_band))
			topo_srv_get_6g_bh_ap_channel(ctx, upstream_device, scan_channel_list);
		else
			topo_srv_get_2g_bh_ap_channel(ctx, upstream_device, scan_channel_list);
	} else {
		mapd_printf(MSG_OFF, BST_AP_SLCT_PREX" OMG: No upstream device");
	}
#else
	if (upstream_device) {
		if (isChan5GL(bh_entry->bh_channel) || isChan5GH(bh_entry->bh_channel)) {
			if (check_is_triband(own_1905_map_device)) {
				if(isChan5GL(bh_entry->bh_channel))
					topo_srv_get_5g_bh_ap_channel_by_band(ctx, upstream_device, scan_channel_list, BAND_5GL);
				else
					topo_srv_get_5g_bh_ap_channel_by_band(ctx, upstream_device, scan_channel_list, BAND_5GH);
			} else 
					topo_srv_get_5g_bh_ap_channel(ctx, upstream_device, scan_channel_list);
		} else
			topo_srv_get_2g_bh_ap_channel(ctx, upstream_device, scan_channel_list);
	} else {
		mapd_printf(MSG_OFF, BST_AP_SLCT_PREX" OMG: No upstream device");
	}
#endif
}
void ap_selection_select_scan_channels(struct own_1905_device *ctx,
	struct bh_link_entry *bh_entry,
	struct scan_BH_ssids *scan_ssids)
{

	//! if we are here for BH steering, update bh_steer sub state to
	//! BH_SUBSTATE_SCAN_DONE, this will be used while making connection
	if(is_bh_steer_triggered(ctx)) {
		err(BH_STEER_PREX "SUB State Change");
		ctx->current_bh_substate = BH_SUBSTATE_SCAN_DONE;
	}
	info(BST_AP_SLCT_PREX" Current BH State = %d", ctx->current_bh_state);
	switch(ctx->current_bh_state)
	{
		case BH_STATE_WIFI_LINKUP:
			//! if we are in wifi link up state and reached this function,
			//! it means we are here for duplicate connection in MBH
			//! handle accordingly
			if (is_mbh_conn_triggered(ctx)) {
				//! get channel list on which target uplink device will be present
				ap_selection_get_uplink_ap_channel(ctx,
								scan_ssids->scan_channel_list, ctx->bh_dup_entry);
				if (!scan_ssids->scan_channel_list[0]) {
					scan_ssids->scan_channel_count = 0;
					mapd_printf(MSG_OFF," MBH Full SCAN");
				} else {
					mapd_printf(MSG_OFF," MBH SCAN on channel = %d,%d,%d,%d",
									scan_ssids->scan_channel_list[0], scan_ssids->scan_channel_list[1], scan_ssids->scan_channel_list[2],scan_ssids->scan_channel_list[3]);
					scan_ssids->scan_channel_count = 1;
					if (scan_ssids->scan_channel_list[1])
						scan_ssids->scan_channel_count++;
					if (scan_ssids->scan_channel_list[2])
						scan_ssids->scan_channel_count++;
					if (scan_ssids->scan_channel_list[3])
						scan_ssids->scan_channel_count++;
				
				}
			}
			break;
		case BH_STATE_WIFI_BOOTUP:
			//! we have just booted up with known WiFi profiles
			//! scan for all channels
			//! no need to be selective
			//! For WiFi profiles through WPS 3 times opprotunity to scan on selective channel on which linkdown due to WPS was received
			if (bh_entry->selective_channel_scan) {
				bh_entry->selective_channel_scan--;
				scan_ssids->scan_channel_list[0] = bh_entry->bh_channel;
				scan_ssids->scan_channel_count = 1;
				mapd_printf(MSG_OFF, BST_AP_SLCT_PREX" SCAN on channel = %d",
						scan_ssids->scan_channel_list[0])
			}
			break;
		case BH_STATE_ETHERNET_UPLUGGED:
		case BH_STATE_WIFI_BAND_SWITCHED:
			//! ethernet link got disconnected or we received band switch command
			//! attempt connection to same uplink agetnt
			if (ctx->link_fail_single_channel_scan_count) {
				ap_selection_get_uplink_ap_channel(ctx,
					scan_ssids->scan_channel_list, bh_entry);
				if (scan_ssids->scan_channel_list[0])
					scan_ssids->scan_channel_count = 1;

				mapd_printf(MSG_OFF, BST_AP_SLCT_PREX" SCAN on channel = %d,%d,%d,%d",
                                                                        scan_ssids->scan_channel_list[0], scan_ssids->scan_channel_list[1], scan_ssids->scan_channel_list[2],scan_ssids->scan_channel_list[3]);

				if (scan_ssids->scan_channel_list[1])
					scan_ssids->scan_channel_count++;
				if (scan_ssids->scan_channel_list[2])
					scan_ssids->scan_channel_count++;
				if (scan_ssids->scan_channel_list[3])
					scan_ssids->scan_channel_count++;

					
			}
			break;
		case BH_STATE_WIFI_BH_STEER:
			//! we received BH steer command
			//! we should scan on requested channel only
			scan_ssids->scan_channel_count = 1;
			scan_ssids->scan_channel_list[0] = ctx->bh_steer_channel;
			break;
		case BH_STATE_WIFI_LINK_FAIL:
			//! we entered link fail, trigger single channel scan on current channel
			if (ctx->link_fail_single_channel_scan_count) {
				scan_ssids->scan_channel_count = 1;
				scan_ssids->scan_channel_list[0] = bh_entry->bh_channel;
			}
			break;
	}
}
#ifdef MAP_6E_SUPPORT
int check_ch_overload_by_band(struct own_1905_device *ctx, u8 channel, u8 band)
{
	uint8_t ra_idx = 0;
	uint8_t ol_th = 0;
	uint8_t ra_band;

	for (ra_idx = 0; ra_idx < MAX_NUM_OF_RADIO; ra_idx++) {
		struct mapd_radio_info *ra_info = NULL;

		ra_info = &ctx->dev_radio_info[ra_idx];
		if (ra_info->radio_idx == (uint8_t)-1)
			continue;

		if (ra_info->channel == 0)
			continue;

		ra_band = get_band_from_chan_op(ra_info->channel, ra_info->op_class);

		if ((ra_info->channel  != channel) && (ra_band != band))
			continue;

		if (IS_MAP_CH_24G(ra_info->channel) && IS_OP_CLASS_24G(ra_info->op_class))
			ol_th = ctx->cli_steer_params.CUOverloadTh_2G;
		else if (IS_MAP_CH_5GL(ra_info->channel) && IS_OP_CLASS_5GL(ra_info->op_class))
			ol_th = ctx->cli_steer_params.CUOverloadTh_5G_L;
		else if (IS_MAP_CH_5GH(ra_info->channel) && IS_OP_CLASS_5GH(ra_info->op_class))
			ol_th = ctx->cli_steer_params.CUOverloadTh_5G_H;
		else
			ol_th = ctx->cli_steer_params.CUOverloadTh_6G;

		if (ra_info->ch_util > ol_th)
			return 1;
	}
	return 0;
}
#endif
int check_ch_overload_by_channel(struct own_1905_device *ctx, u8 channel)
{
	uint8_t ra_idx = 0;
	uint8_t ol_th = 0;

	for (ra_idx = 0; ra_idx < MAX_NUM_OF_RADIO; ra_idx++) {
		struct mapd_radio_info *ra_info = NULL;
		ra_info = &ctx->dev_radio_info[ra_idx];
		if (ra_info->radio_idx == (uint8_t)-1)
			continue;

		if (ra_info->channel  != channel) {
			continue;
		}

		 if (ra_info->channel >= 1 && ra_info->channel <= 14)
			ol_th = ctx->cli_steer_params.CUOverloadTh_2G;
		 else if(ra_info->channel >=36 && ra_info->channel <= 64)
			ol_th = ctx->cli_steer_params.CUOverloadTh_5G_L;
		 else
			ol_th = ctx->cli_steer_params.CUOverloadTh_5G_H;

		if(ra_info->ch_util > ol_th)
			return 1;
	}
	return 0;
}

/*
	The fucntion is starting point
	for every BH-STA connection procedure.
	Objective is to identify appropriate BH-interface
	and trigger scan on it.
*/
void ap_selection_issue_scan(struct own_1905_device *ctx)
{
	struct bh_link_entry *bh_entry, *tmp_bh_entry = NULL;
	unsigned char bh_with_priority_found = FALSE;
	struct scan_BH_ssids *scan_ssids;
	unsigned int profile_idx = 0;
	struct radio_info_db *radio = NULL;
	struct _1905_map_device *own_dev = topo_srv_get_1905_device(ctx, NULL);

	//! there is no need to trigger scan if we
	//! do not have any BH profiles available
	if (ctx->bh_config_count == 0) {
		err(BST_AP_SLCT_PREX" BH configs not available");
		return;
	}

	//! do not scan if ethernet BH connected
	if (ctx->current_bh_state == BH_STATE_ETHERNET_PLUGGED)
	{
		err(BST_AP_SLCT_PREX" Ethernet plugged, no need to scan");
		return;
	}

	//! do not scan if we are controller and we are
	//! not going to connect to any non MAP device
	if ((ctx->device_role == DEVICE_ROLE_CONTROLLER || ctx->auto_role_detect == 2)
		&& (ctx->ThirdPartyConnection == 0))
	{
		err(BST_AP_SLCT_PREX" Current Role Controller/unconfigured, do not scan");
		return;
	}
	if (ctx->device_role == DEVICE_ROLE_CONTROLLER && (ctx->ThirdPartyConnection == 1) &&
		(is_chplan_netopt_ongoing(ctx->back_ptr) == TRUE)) {
		info(BST_AP_SLCT_PREX" controller Third party do not scan, since ch plan , net opt ongoing");
		ap_selection_reset_scan(ctx);
		eloop_register_timeout(10, 0, ap_selection_retrigger_scan, ctx, NULL);
		return;
	}

	//! Keep looping unless we get appropriate exit condition
	while(1) {
		bh_entry = NULL;

		//! check for multi-BH
		if (is_mbh_conn_triggered(ctx)) {
			 if(ctx->sec_link_scan_cnt < ctx->max_allowed_scan) {
				 bh_entry = ctx->bh_dup_entry;
				 if (!bh_entry)
					 mapd_ASSERT(0);
				 mapd_printf(MSG_OFF," Scan for MBH on %s", bh_entry->ifname);
				 bh_with_priority_found = TRUE;
				 bh_entry->priority_info.attempt_allowed = TRUE;
				 if (bh_entry->priority_info.scan_triggered == TRUE)
				 	ctx->sec_link_scan_cnt++;
			 } else {
				if ( ctx->sec_bh_link_restore == FALSE) {
					ctx->sec_bh_link_restore = TRUE;
					eloop_register_timeout(SEC_BH_LINK_RESTORE, 0, restore_sec_bh_link, ctx, NULL);
				}
			}
		} else {
			//! run loop for each BH interface
			SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, tmp_bh_entry) {
			//! check if current BH's priority matches current scan
			//! the priority
			if ((bh_entry->priority_info.priority ==
				ctx->current_connect_priority)) {
				//! if we are here due to link fail,
				//! scan should be triggered only for the BH which gets
				//! disconnected.
				//! if CAC channel is present then do full scan on all bands
#ifdef MAP_6E_SUPPORT
				radio = topo_srv_get_radio_by_band_type(own_dev, bh_entry->bh_band);
#else
				radio = topo_srv_get_radio_by_band(own_dev, bh_entry->bh_channel);
#endif
				if ((bh_entry == ctx->failed_link_bh_entry) &&
					(radio && radio->cac_channel != 0))
				{
					ctx->link_fail_single_channel_scan_count = 0;
					continue;
				}
				
				//! this should continue as long as single channel scan count
				//! is non zero
				if (ctx->current_bh_state == BH_STATE_WIFI_LINK_FAIL &&
					ctx->link_fail_single_channel_scan_count &&
					(bh_entry != ctx->failed_link_bh_entry))
				{
					continue;
				}

				//! if we are here dueto BH steer command from controller
				//! we should scan only for the BH link for which Steer
				//! command is received
				if ((ctx->current_bh_state == BH_STATE_WIFI_BH_STEER) &&
					(ctx->bh_steer_bh_entry != bh_entry))
				{
					continue;
				}

				if (ctx->bh_cu_params.bh_switch_cu_en) {
#ifdef MAP_6E_SUPPORT
					if (check_ch_overload_by_band(ctx, bh_entry->bh_channel, bh_entry->bh_band) &&
						(ctx->current_bh_state == BH_STATE_WIFI_BAND_SWITCHED))
#else
					if (check_ch_overload_by_channel(ctx, bh_entry->bh_channel) &&
						(ctx->current_bh_state == BH_STATE_WIFI_BAND_SWITCHED))
#endif
					{
						err(BST_AP_SLCT_PREX" channel %d util overload",bh_entry->bh_channel);
						continue;
					}
				}

				info(BST_AP_SLCT_PREX" current priority = %d",
					ctx->current_connect_priority);
				info(BST_AP_SLCT_PREX" %s matches current priority\n",
					bh_entry->ifname);
				bh_entry->priority_info.attempt_allowed = TRUE;
				bh_with_priority_found = TRUE;
			}
			}
		}

		//! we found a BH on which we are supposed to scan in
		//! current iteration
		if (bh_with_priority_found)
		{
			bh_entry = NULL;
			tmp_bh_entry = NULL;
			SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, tmp_bh_entry) {
				//! we find a BH on which connection attempt is allowed
				//! and scan is not troiggered yet
				if (bh_entry->priority_info.attempt_allowed &&
					!bh_entry->priority_info.scan_triggered) {
						info(BST_AP_SLCT_PREX" Trigger Scan For %s\n", bh_entry->ifname);
						bh_entry->priority_info.scan_triggered = TRUE;
				//! prepare scan request message
				scan_ssids = os_zalloc(sizeof(struct scan_BH_ssids));//max Profile count 4 possible
				if (!scan_ssids) {
					info(BST_AP_SLCT_PREX" %s alloc scan_BH_ssids fail\n", __func__);
					return;
				}
				if (os_get_random((unsigned char *)&scan_ssids->scan_cookie, sizeof(unsigned long)) < 0) {
					if (scan_ssids)
						os_free(scan_ssids);
					err("random cookie failed");
					return;
				}
				if(scan_ssids->scan_cookie == 0)
					scan_ssids->scan_cookie = 1;
				scan_ssids->profile_cnt = ctx->bh_config_count;
				err(BST_AP_SLCT_PREX" Total Profiles: %d", scan_ssids->profile_cnt);
				//! fll in the SSIDs for which we should trigger scan
				while (profile_idx < scan_ssids->profile_cnt)
				{
					scan_ssids->scan_SSID_val[profile_idx].SsidLen = ctx->bh_configs[profile_idx].SsidLen;
					os_memset(&scan_ssids->scan_SSID_val[profile_idx].ssid,0,MAX_SSID_LEN+1);
					os_memcpy(&scan_ssids->scan_SSID_val[profile_idx].ssid,
						&ctx->bh_configs[profile_idx].ssid,scan_ssids->scan_SSID_val[profile_idx].SsidLen);
					err(BST_AP_SLCT_PREX" Ssid to Scan %s", scan_ssids->scan_SSID_val[profile_idx].ssid);
					profile_idx++;
				}

				//! select channels for which we want to trigger scan
				ap_selection_select_scan_channels(ctx, bh_entry, scan_ssids);
				ctx->scan_triggered = TRUE;
				wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_ISSUE_SCAN_REQ,
									0, bh_entry->mac_addr, 0, scan_ssids, sizeof(struct scan_BH_ssids), 0, 0, 0);

				free(scan_ssids);
				return;
				}
			}

			//! we reached here means we do not have any other BH interface
			//! with current priority to be scanned. We need to process all scan results
			//! and proceed with connection
			if (ap_selection_issue_connect(ctx))
			{
				if (is_mbh_conn_triggered(ctx)) {
					mapd_printf(MSG_OFF, "Could not find dup BSS; Re-triggering scan for dup link");
					ap_selection_reset_scan(ctx);
					eloop_register_timeout(2, 0, ap_selection_retrigger_scan, ctx, NULL);
				} else {
						err(BST_AP_SLCT_PREX "connection attempt on currrent priority failed\n");
						goto next_priority_bh;
				}
			} else {
				//! connection attempt on current priority was succesful
				//! reset scan state machine and exit scan process
				//! ap_selection_issue_connect function has triggered the
				//! connection process
				ap_selection_reset_scan(ctx);
				return;
			}
		}

		//! scan and connect attempt on current BH was unsuccesfull
		//! we need to attempt connection on next priority BH now
next_priority_bh:
		ctx->current_connect_priority++;
		bh_with_priority_found = FALSE;

		//! connection attempt on all priorities failed
		//! reset scan state machine and attempt right from scratch again
		if (ctx->current_connect_priority >
			MIN_POSSIBLE_BH_PRIORITY)
		{
			ap_selection_reset_scan(ctx);
			if (ctx->device_role == DEVICE_ROLE_CONTROLLER && (ctx->ThirdPartyConnection == 1))
				eloop_register_timeout(10, 0, ap_selection_retrigger_scan, ctx, NULL);
			else
				eloop_register_timeout(2, 0, ap_selection_retrigger_scan, ctx, NULL);
			return;
		}
	}
}

void restore_sec_bh_link(void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *ctx = eloop_ctx;
	err("*");
	ctx->sec_bh_link_restore = FALSE;
	ctx->sec_link_scan_cnt = 0;
}

void ap_selection_retrigger_scan(void *global_ctx, void *timer_ctx)
{
	struct own_1905_device *ctx = (struct own_1905_device *)global_ctx;
	if (ctx->scan_triggered) {
		err(BST_AP_SLCT_PREX" scan already going on, return");
		return;
	}
	ap_selection_issue_scan(global_ctx);
}

void ap_selection_reconnection_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *ctx = eloop_ctx;
	err(BST_AP_SLCT_PREX" *");
	if (ctx->current_bh_state != BH_STATE_WIFI_LINKUP) {
		err(BST_AP_SLCT_PREX " Current State not link up, do not start reconnection");
		return;
	}
	wlanif_disconnect_apcli(ctx->back_ptr, NULL);
	ctx->current_bh_state = BH_STATE_WIFI_BAND_SWITCHED;
	eloop_cancel_timeout(ap_selection_reconnection_timeout, ctx, NULL);
}

void register_clear_switch_table(void * eloop_ctx, void *user_ctx)
{
	err(BST_AP_SLCT_PREX" clear switch table");
	if (system("sh /usr/bin/brsw-del.sh 1 0") == -1)
		err(BST_AP_SLCT_PREX " system() call return value is equal to -1");
}

void send_garp_req_to_wapp_on_intf_for_sta(struct own_1905_device *ctx, unsigned char *sta_mac)
{
	struct garp_req_s *garp_req = NULL;
	struct mapd_global *mapd_ctx = NULL;
	struct _1905_map_device *dev = NULL;
	struct bss_info_db *bss = NULL, *tbss = NULL;
	struct bh_link_entry *bh_entry = NULL, *tbh_entry = NULL;

	if (!ctx || !sta_mac)
		return;

	mapd_ctx = (struct mapd_global *)ctx->back_ptr;
	if (!mapd_ctx || mapd_ctx->params.Certification) {
		err(BST_AP_SLCT_PREX " Return when Cert is ON");
		return;
	}

	dev = topo_srv_get_1905_device(ctx, NULL);
	if (!dev) {
		err(BST_AP_SLCT_PREX " get own dev fail");
		return;
	}

	garp_req = os_zalloc(sizeof(struct garp_req_s) + sizeof(struct garp_src_addr));
	if (!garp_req) {
		err(BST_AP_SLCT_PREX " alloc memory for garp_req fail");
		return;
	}

	os_memcpy(garp_req->mac_addr_list[0].addr, sta_mac, ETH_ALEN);
	garp_req->sta_count = 1;

	/*send garp at bss as bh bss */
	SLIST_FOREACH_SAFE(bss, &dev->first_bss, next_bss, tbss) {
		os_memset(garp_req->dev_addr_list[0].addr, 0, ETH_ALEN);
		garp_req->dev_cnt = 0;
		if (bss->map_vendor_extn & BSS_BH) {
			os_memcpy(garp_req->dev_addr_list[0].addr, bss->bssid, ETH_ALEN);
			garp_req->dev_cnt = 1;
			err(" send garp req for sta: "MACSTR" at ra interface:" MACSTR, MAC2STR(sta_mac), MAC2STR(bss->bssid));
			wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_GARP_REQUEST,
			0, NULL, 0, garp_req, sizeof(struct garp_req_s) + ETH_ALEN * garp_req->sta_count, 0, 0, 0);
		}
	}

	/* send garp at bh interface*/
	if (ctx->device_role != DEVICE_ROLE_CONTROLLER) {
		SLIST_FOREACH_SAFE(bh_entry, &ctx->bh_link_head, next_bh_link, tbh_entry) {
			os_memset(garp_req->dev_addr_list[0].addr, 0, ETH_ALEN);
			garp_req->dev_cnt = 0;
			if (bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED) {
				os_memcpy(garp_req->dev_addr_list[0].addr, bh_entry->mac_addr, ETH_ALEN);
				garp_req->dev_cnt = 1;
				err("send garp req for sta: "MACSTR" at apcli interface:" MACSTR,
					MAC2STR(sta_mac), MAC2STR(bh_entry->mac_addr));
				wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_GARP_REQUEST,
				0, NULL, 0, garp_req, sizeof(struct garp_req_s) + ETH_ALEN * garp_req->sta_count, 0, 0, 0);
			}
		}
	}
	os_free(garp_req);
}

void send_garp_req_to_wapp_on_intf(struct own_1905_device *ctx, unsigned char *mac)
{
	struct garp_req_s *garp_req = NULL;
	unsigned int garp_req_cnt = 0;
	struct connected_clients *conn_client = NULL, *tmp_conn_client = NULL;
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;

	if (mapd_ctx->params.Certification) {
		err(BST_AP_SLCT_PREX " Return when Cert is ON");
		return ;
	}

	SLIST_FOREACH_SAFE(conn_client, &dev->wlan_clients, next_client, tmp_conn_client) {
		garp_req_cnt++;
	}

	conn_client = NULL;
	garp_req = os_zalloc(sizeof(struct garp_req_s) + sizeof(struct garp_src_addr) * garp_req_cnt);
	if (!garp_req) {
		err(BST_AP_SLCT_PREX " alloc memory for garp_req fail");
		return;
	}

	os_memcpy(garp_req->dev_addr_list[0].addr, mac, ETH_ALEN);
	garp_req->dev_cnt = 1;

	garp_req_cnt = 0;
	tmp_conn_client = NULL;
	SLIST_FOREACH_SAFE(conn_client, &dev->wlan_clients, next_client, tmp_conn_client) {
		if (!conn_client->is_APCLI) {
			os_memcpy(garp_req->mac_addr_list[garp_req_cnt].addr,
				conn_client->client_addr, 6);
			err(BST_AP_SLCT_PREX" sending garp for "MACSTR, MAC2STR(conn_client->client_addr));
			garp_req_cnt++;
		}
	}
	err(BST_AP_SLCT_PREX" garp req count %d",garp_req_cnt);
	garp_req->sta_count = garp_req_cnt;
	wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_GARP_REQUEST,
	0, NULL, 0, garp_req, sizeof(struct garp_req_s) + ETH_ALEN * garp_req->sta_count, 0, 0, 0);
	os_free(garp_req);
}


int ap_selection_handle_cli_state_change(struct own_1905_device *ctx, struct wapp_apcli_association_info *assoc)
{
	char iface_name[64];
	struct bh_link_entry *bh_entry = NULL, *tmp_bh_entry = NULL;
	struct bh_link_entry *bh_entry_local = NULL;
	int num_active_links = 0;
	struct bh_link_entry *linkup_bh_entry = NULL;
	struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;
	get_iface_from_index(assoc->interface_index, iface_name);
	unsigned char need_run_ap_selection = TRUE;
	struct bh_link_info bh_info = {0};
	struct blacklisted_ap_list *bl_ap = NULL;
	struct _1905_map_device *dev = NULL;
	struct iface_info *iface = NULL, *tmp_iface = NULL;
	struct os_time now;
	unsigned char avoid_reconnection = 0;

	if (mapd_ctx->params.Certification) {
		err(BST_AP_SLCT_PREX " ap_selection_handle_cli_state_change: Return when Cert is ON");
		return 0;
	}

	dev = topo_srv_get_1905_device(ctx, NULL);

	err(BST_AP_SLCT_PREX " bh state %d sub state %d",
		ctx->current_bh_state,
		ctx->current_bh_substate);
	if(ctx->bh_cu_params.bh_switch_cu_en) {
		if (ctx->current_bh_substate != BH_SUBSTATE_DUP_LINKDOWN_WAIT)
			eloop_cancel_timeout(ap_selection_reconnection_timeout, ctx, NULL);
	} else {
		eloop_cancel_timeout(ap_selection_reconnection_timeout, ctx, NULL);
	}

	SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, tmp_bh_entry) {
		if (strcmp((const char *)bh_entry->ifname, iface_name) == 0) {
			if (assoc->apcli_assoc_state == WAPP_APCLI_DISASSOCIATED) {
				err(BST_AP_SLCT_PREX " *****************************************received LinkDown For %s", bh_entry->ifname);
				if (bh_entry->bh_assoc_state == WAPP_APCLI_DISASSOCIATED) {
					err(BST_AP_SLCT_PREX " already in link down, recieved down again");
					if (ctx->current_bh_state == BH_STATE_WIFI_BOOTUP
#ifdef MAP_R3
						&& (ctx->ReconfigTrigger == FALSE)
#endif
					) {
						/*WPS --> get credentials-->BH_STATE_WIFI_BOOTUP-->enter here--> set the retry to 3*/
						bh_entry->selective_channel_scan = 3;
						err(BST_AP_SLCT_PREX " POST WPS SCAN TRIGGER");
						/* Check for Black List AP and increase fail cnt */
						bl_ap = lookup_for_bl_ap(ctx);
					if (bl_ap) {
						bl_ap->fail_cnt++;
						warn(BST_AP_SLCT_PREX " Fail CNT increase for BL AP");
					}
						eloop_cancel_timeout(ap_selection_retrigger_scan, ctx, NULL);
						eloop_register_timeout(0,0,
									ap_selection_retrigger_scan,
									ctx,NULL);
					}

					if (os_memcmp(ctx->conn_attempted_mac, ZERO_MAC_ADDR, ETH_ALEN)
#ifdef MAP_R3
						&& (ctx->ReconfigTrigger == FALSE)
#endif
					) {
						bl_ap = lookup_for_bl_ap(ctx);
						os_get_time(&now);
						if (bl_ap == NULL) {
							bl_ap = (struct blacklisted_ap_list *)os_zalloc(sizeof(struct blacklisted_ap_list));
							if (!bl_ap) {
									err(BST_AP_SLCT_PREX " Mem alloc failed");
									return -1;
							}
							os_memcpy(bl_ap->bssid, ctx->conn_attempted_mac, ETH_ALEN);
							bl_ap->time = now;
							err(BST_AP_SLCT_PREX " Added Blacklisted AP: ("MACSTR")", MAC2STR(bl_ap->bssid));
							SLIST_INSERT_HEAD(&ctx->bl_ap_list, bl_ap, next_bl_ap);
						} else {
								bl_ap->time = now;
						}
					}
					return 0; // Timeout should handle
				}
				/*Cancel network opt scan*/
				eloop_cancel_timeout(trigger_net_opt_scan, ctx, NULL);

				bh_entry->bh_assoc_state = WAPP_APCLI_DISASSOCIATED;
				bh_entry->bh_channel = assoc->current_channel;
				update_apcli_info_in1905(mapd_ctx);
				bh_info.type = 1;
				os_memcpy(bh_info.ifname, bh_entry->ifname, os_strlen((const char *)bh_entry->ifname));
				os_memcpy(bh_info.mac_addr,bh_entry->mac_addr, ETH_ALEN);

				SLIST_FOREACH_SAFE(iface, &(dev->_1905_info.first_iface), next_iface, tmp_iface) {
					if (!os_memcmp(iface->iface_addr, bh_entry->mac_addr, ETH_ALEN)) {
						os_memcpy(iface->uplink_bss, ZERO_MAC_ADDR, ETH_ALEN);
						err(BST_AP_SLCT_PREX"uplink bss "MACSTR" for iface "MACSTR,
							MAC2STR(iface->uplink_bss), MAC2STR(iface->iface_addr));
					}
				}
				/* Links changed: Trigger Primary Link Change */
				if (ctx->dual_bh_en)
					mbh_handle_link_change(ctx);

				if (ctx->ThirdPartyConnection) {
					if(DEVICE_ROLE_AGENT == ctx->device_role) {
						map_1905_Set_Wi_Bh_Link_Down(mapd_ctx->_1905_ctrl, &bh_info);
					}
					ctx->ConnectThirdPartyVend=0;
				} else {
					map_1905_Set_Wi_Bh_Link_Down(mapd_ctx->_1905_ctrl, &bh_info);
				}
				err(BST_AP_SLCT_PREX " bh entry iface %s Channel %d", bh_entry->ifname, bh_entry->bh_channel);

				/* Link Down during LOOP_DETECTED; This is a deliberate disconnection */
				if (ctx->bh_loop_state == BH_SUBSTATE_LOOP_LINK_DOWN_WAIT) {
					err(BST_AP_SLCT_PREX " apcli disconnected in LOOP detected State");
					return 0;
				}

				if ((ctx->current_bh_state == BH_STATE_WIFI_LINKUP) &&
					get_mbh_state(ctx, &bh_entry_local, &num_active_links) != MBH_DISABLED) {
					if (num_active_links > 0) {
						mapd_printf(MSG_OFF, "Still an active link present: Stay in WIFI_LINKUP State");
						mapd_printf(MSG_OFF, "One Link gone down. Send Unsolicited Chan Pref Report to Controller");
#ifdef MAP_R2
						topo_srv_get_cac_statusinfo(&mapd_ctx->dev);
#else
						_1905_update_channel_pref_report(ctx, NULL);
#endif
						return 0;
					}
				}

				if (ctx->current_bh_state != BH_STATE_ETHERNET_PLUGGED) {
					ctx->failed_link_bh_entry = bh_entry;

					/*Ignore Link Down of Duplicate Link*/
					if (ctx->last_connected_bh_entry &&
						(ctx->current_bh_substate == BH_SUBSTATE_DUP_LINKDOWN_WAIT) &&
						(strcmp((const char *)bh_entry->ifname, (const char *)ctx->last_connected_bh_entry->ifname) == 0)) {
							err(BST_AP_SLCT_PREX" Link Down Recvd For Dup Link");
							ctx->last_connected_bh_entry = NULL;
							ctx->current_bh_substate = BH_SUBSTATE_IDLE;
							return 0;
					}

					ctx->link_fail_single_channel_scan_count = 3;
					if (ctx->current_bh_state == BH_STATE_WIFI_LINKUP) {
						err(BST_AP_SLCT_PREX" State change to Link Down");
						ctx->current_bh_state = BH_STATE_WIFI_LINK_FAIL;
						/* Clean-up MBH State */
						ctx->bh_dup_entry = NULL;
					}
					if (ctx->bh_loop_state != BH_SUBSTATE_E_LINK_C)
						topo_srv_update_radio_config_status(ctx, NULL, FALSE);

					if (is_bh_steer_triggered(ctx)){
						if (ctx->current_bh_substate == BH_SUBSTATE_LINKDOWN_WAIT){
							if (get_mbh_state(ctx, &bh_entry_local, &num_active_links) != MBH_DISABLED) {
								if (num_active_links > 0) {
									mapd_printf(MSG_OFF, BH_STEER_PREX" BH Steer/NOP Triggered but "
												"Still an active link present: Wait for its Link Down");
									return 0;
								}
							}
							err(BH_STEER_PREX" issue connect after Link Down");
							/*XXX: Why not move out of BH_SUBSTATE_LINKDOWN_WAIT here itself? */
							ap_selection_issue_connect(ctx);
							return 0;
						}
						if (ctx->current_bh_substate == BH_SUBSTATE_CONNECT_WAIT){
							err(BH_STEER_PREX" BH_SUBSTATE_CONNECT_WAIT so Skip Link down ");
							return 0;
						}
					}
					tmp_bh_entry = NULL;
					SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, tmp_bh_entry) {
						if (bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED)
						{
							err(BST_AP_SLCT_PREX" BH Entry ifname %s Bssid"MACSTR, bh_entry->ifname, MAC2STR(bh_entry->bss.Bssid));
							need_run_ap_selection = FALSE;
							break;
						}
					}
					if (need_run_ap_selection
#ifdef MAP_R3
						&& ((ctx->map_version <= DEV_TYPE_R2) ||((ctx->map_version == 3) && !assoc->dpp_onb_ongoing))
#endif /* MAP_R3 */
					){
						ctx->bh_ready_expected = FALSE;
						err(BST_AP_SLCT_PREX" Issue Scan");
						/*clean sub states here*/
						//reset_ntwrk_opt_states(ctx);
						ctx->current_bh_substate = BH_SUBSTATE_IDLE;
						if(ctx->wsc_save_bh_profile == FALSE)
							ap_selection_issue_scan(ctx);
						ctx->wsc_save_bh_profile = FALSE;
					}
					if (ctx->dhcp_ctl_enable
						&& ((ctx->bh_config_count == 0) || (! need_run_ap_selection))
                                        	&& (ctx->current_bh_state == BH_STATE_WIFI_LINK_FAIL)) {
							/*
							3rd connetion enbale & deivce role is controller, dhcp server should in 3rd-AP,  when link disconnected,
							controller should not enable dhcp server in it. because may cause diff-subnet issue of agents.
							*/
							if (ctx->ThirdPartyConnection == 1 && ctx->device_role == DEVICE_ROLE_CONTROLLER) {
								if (!(0 == ctx->dhcp_req.dhcp_server_enable && 1 == ctx->dhcp_req.dhcp_client_enable)) {
									mapd_printf(MSG_OFF,BST_AP_SLCT_PREX" link fail,3rd conn enable,enable dhcp cli for recon!\n");
									ctx->dhcp_req.dhcp_server_enable = 0;
									ctx->dhcp_req.dhcp_client_enable = 1;
									wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_DHCP_CTL_REQUEST,
									0, NULL, NULL, &ctx->dhcp_req, sizeof(struct dhcp_ctl_req), 0, 0, 0);
									/*get ip timeout timer*/
									map_register_dhcp_timer(ctx);
								}
							} else {
								if (!(1 == ctx->dhcp_req.dhcp_server_enable && 0 == ctx->dhcp_req.dhcp_client_enable)) {
									mapd_printf(MSG_OFF,BST_AP_SLCT_PREX" Agent Link down,enable_dhcp_server!\n");
									ctx->dhcp_req.dhcp_server_enable = 1;
									ctx->dhcp_req.dhcp_client_enable = 0;
									wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_DHCP_CTL_REQUEST,
									0, NULL, NULL, &ctx->dhcp_req, sizeof(struct dhcp_ctl_req), 0, 0, 0);
								}
							}
						}

				}
				break;
			} else {

				err(BST_AP_SLCT_PREX" XXXXXXXXXXXXXXXXXXXXXXXXXXX received LinkUp For %s\n", bh_entry->ifname);
				eloop_cancel_timeout(issue_connect_timeout_handle, ctx, NULL);
				eloop_cancel_timeout(ap_selection_retrigger_scan, ctx, NULL);
				if(is_bh_steer_triggered(ctx) &&
					ctx->current_bh_substate == BH_SUBSTATE_CONNECT_WAIT){
					if(ctx->last_connected_bh_entry) {
						err(BST_AP_SLCT_PREX" state change Dup Link down wait");
						wlanif_disconnect_apcli(mapd_ctx, ctx->last_connected_bh_entry->ifname);
						ctx->current_bh_substate = BH_SUBSTATE_DUP_LINKDOWN_WAIT;
					}
				}

				if(ctx->current_bh_substate == BH_SUBSTATE_CONNECT_WAIT) {
					err(BST_AP_SLCT_PREX" State change IDLE");
					ctx->current_bh_substate = BH_SUBSTATE_IDLE;
				}
				err(BST_AP_SLCT_PREX" Current BH Sub state %d",ctx->current_bh_substate);
				bh_entry->bh_channel = assoc->current_channel;
				linkup_bh_entry = bh_entry;
				if (ctx->current_bh_state == BH_STATE_WIFI_BH_STEER) {
					memcpy(ctx->bsteer_rsp.backhaul_mac, ctx->bsteer_req.backhaul_mac, ETH_ALEN);
					memcpy(ctx->bsteer_rsp.target_bssid, ctx->bsteer_req.target_bssid, ETH_ALEN);
					ctx->bsteer_rsp.status = 0x00;//success
					if(ctx->bh_cu_params.bh_switch_cu_en && !ctx->dual_bh_en)
						avoid_reconnection = 1;
					map_1905_Set_Bh_Steer_Rsp_Info(mapd_ctx->_1905_ctrl, &ctx->bsteer_rsp);
					eloop_cancel_timeout(send_bh_steering_fail, ctx, NULL);
				}

				ctx->current_bh_state = BH_STATE_WIFI_LINKUP;

				if (ctx->bh_dup_entry &&
					(ctx->bh_dup_entry == bh_entry)) {
					mapd_printf(MSG_OFF, BST_AP_SLCT_PREX" Duplicate Link Up");
				}

				ctx->failed_link_bh_entry = NULL;

				bh_entry->bh_assoc_state = WAPP_APCLI_ASSOCIATED;
				bh_entry->rssi = assoc->rssi;
				bh_entry->bh_channel = bh_entry->bss.Channel;
				tmp_iface = NULL;
				SLIST_FOREACH_SAFE(iface, &(dev->_1905_info.first_iface), next_iface, tmp_iface) {
					if (!os_memcmp(iface->iface_addr, bh_entry->mac_addr, ETH_ALEN)) {
						os_memcpy(iface->uplink_bss, bh_entry->bss.Bssid, ETH_ALEN);
						err(BST_AP_SLCT_PREX"uplink bss "MACSTR" for iface "MACSTR,
							MAC2STR(iface->uplink_bss), MAC2STR(iface->iface_addr));
					}
				}
				update_apcli_info_in1905(mapd_ctx);

				os_memcpy(ctx->conn_attempted_mac, ZERO_MAC_ADDR, ETH_ALEN);
				/* Links changed: Trigger Primary Link Change */
				if (ctx->dual_bh_en)
					mbh_handle_link_change(ctx);
				if (ctx->dhcp_ctl_enable) {
					/*dhcp service*/
					if (ctx->ThirdPartyConnection == 1 && ctx->device_role == DEVICE_ROLE_CONTROLLER) {
					mapd_printf(MSG_OFF,BST_AP_SLCT_PREX" dhcp_server_enable(%d) dhcp_client_enable(%d)!\n",
						ctx->dhcp_req.dhcp_server_enable, ctx->dhcp_req.dhcp_client_enable);
						if (!(0 == ctx->dhcp_req.dhcp_server_enable && 1 == ctx->dhcp_req.dhcp_client_enable)) {
							mapd_printf(MSG_OFF,BST_AP_SLCT_PREX" wifi link up,3rd connection,enable dhcp cli!\n");
							ctx->dhcp_req.dhcp_server_enable = 0;
							ctx->dhcp_req.dhcp_client_enable = 1;
							wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_DHCP_CTL_REQUEST,
							0, NULL, NULL, &ctx->dhcp_req, sizeof(struct dhcp_ctl_req), 0, 0, 0);
							/*get ip timeout timer*/
							map_register_dhcp_timer(ctx);
						}
					}
				}
				if (ctx->ThirdPartyConnection == 1 && ctx->device_role == DEVICE_ROLE_CONTROLLER) {
					info("Restart net opt due to Third party connection");
					if (eloop_is_timeout_registered(trigger_net_opt, (void *)ctx, NULL))
						eloop_cancel_timeout(trigger_net_opt, ctx, NULL);
					eloop_register_timeout(ctx->network_optimization.wait_time,
						0, trigger_net_opt, ctx, NULL);
				}
			}

			if (linkup_bh_entry && (assoc->peer_map_enable || !ctx->non_map_ap_enable)) {
				u32 rem_ol_time = 0;
				ap_selection_restore_conn_priority(ctx);
				/* Populate own Force Steer Unfriendly Time */
				if(ctx->bh_cu_params.bh_switch_cu_en && !ctx->dual_bh_en) {
					struct os_reltime rem_time = {0};
					rem_time.sec = 0; rem_time.usec = 0;
					eloop_get_remaining_timeout(&rem_time, band_switch_by_cu_timeout,
							(void *)ctx, NULL);
					rem_ol_time = rem_time.sec ? rem_time.sec : ((rem_time.usec ? 1 : 0));
					err("ol remain time %d",rem_ol_time);
					if (ctx->band_switch_time) {
						bh_entry = NULL;
						tmp_bh_entry = NULL;
						SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, tmp_bh_entry) {
							if (bh_entry->priority_info.priority &&
								(bh_entry->priority_info.priority <
								linkup_bh_entry->priority_info.priority) && (avoid_reconnection != 1)) {
								err("Trigger timeout : %d\n", ctx->band_switch_time + rem_ol_time);
								eloop_register_timeout(( ctx->band_switch_time + rem_ol_time),0,
										ap_selection_reconnection_timeout,
										ctx, NULL);
								break;
							}
						}
					}
				}
			}
			return 0;
		}
	}

	return -1;
}


/**
* @brief Fn to start link monitor service for an backhaul link
*
* @param ctx own 1905 device ctx
* @param info backhaul link info
*
* @return 0
*/
int start_link_monitor_service(struct own_1905_device *ctx, struct bh_link_info *info)
{
	struct bh_link_entry *bh = NULL, *tmp_bh = NULL;
	u8 found = 0;
	/*
	 * Check the configuration which will monitor
	 * the link based on rssi and disconnection
	 */
	if (SLIST_EMPTY(&(ctx->bh_link_head))) {
		SLIST_INIT(&ctx->bh_link_head);
	}
	/* find if the entry is present for this */
	SLIST_FOREACH_SAFE(bh, &(ctx->bh_link_head), next_bh_link, tmp_bh) {
		if (os_memcmp(bh->mac_addr, info->mac_addr, ETH_ALEN) == 0) {
			found = 1;
			break;
		}
	}
	if (found == 0) {
		bh = malloc(sizeof(struct bh_link_entry));
		if (bh == NULL) {
			err("%s allock memory fail\n", __FUNCTION__);
			return -1;
		}
		os_memset(bh, 0, sizeof(struct bh_link_entry));
		SLIST_INIT(&(bh->scan_bss_list_head));
	}
	bh->type = info->type;
	os_memcpy(bh->ifname, info->ifname, IFNAMSIZ);
	os_memcpy(bh->mac_addr, info->mac_addr, ETH_ALEN);
	os_memcpy(bh->bssid, info->bssid, ETH_ALEN);
	/* TODO for future */
	bh->rssi_monitor = 0;

	/* add this entry in list only when it is not in this list*/
	if (found == 0)
		SLIST_INSERT_HEAD(&ctx->bh_link_head, bh, next_bh_link);
	return 0;
}

/**
* @brief Fn to check whether a link is available or not
*
* @param ctx own 1905 device ctx
* @param bh_entry backhaul link info
*
* @return TRUE/FALSE
*/
int is_link_available(struct own_1905_device *ctx, struct bh_link_entry *bh_entry)
{
	if (bh_entry->rssi_monitor) {
		if (ctx->cli_steer_params.cli_rssi_threshold >= bh_entry->rssi)
			return FALSE; //ap_selection_issue_scan(ctx, bh_entry);
	}
	return TRUE;
}

/**
* @brief Fn to monitor link for a backhaul CLI
*
* @param ctx own 1905 device ctx
*
* @return 0
*/
int ap_selection_monitor_link(struct own_1905_device *ctx)
{
	struct bh_link_entry *bh_entry, *tmp_bh_entry = NULL;
	int link_available;

	if (SLIST_EMPTY(&(ctx->bh_link_head))) {
		/* link is not ready yet */
		return 0;
	}

	/* check if any of the backhaul link is failing */
	SLIST_FOREACH_SAFE(bh_entry, &(ctx->bh_link_head), next_bh_link, tmp_bh_entry) {
		if (bh_entry->type == 0)
			continue;
		wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_APCLI_RSSI_REQ,
					WAPP_APCLI_UPLINK_RSSI, bh_entry->bssid, bh_entry->mac_addr, NULL, 0, 0, 0, 0);
		link_available = is_link_available(ctx, bh_entry);
		if (!link_available) {
			//ap_selection_issue_scan(ctx);
		}
	}
	return 0;
}


int mapd_trigger_bh_switch(struct mapd_global *global, unsigned int priority_2g, unsigned int priority_5gl, unsigned int priority_5gh)
{
	struct bh_link_entry *bh_entry = NULL, *tmp_bh_entry = NULL;
	struct own_1905_device *ctx = &global->dev;
	unsigned char trigger_flag = 0;
	//err("enter mapd_trigger_bh_switch");
	SLIST_FOREACH_SAFE(bh_entry, &(global->dev.bh_link_head), next_bh_link, tmp_bh_entry) {
		if (bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED ) {
			trigger_flag = 1;
		}

		if (bh_entry->bh_channel <= 14) {
			bh_entry->priority_info.priority = priority_2g;
		} else if (bh_entry->bh_channel > 14 && bh_entry->bh_channel < 100) {
			bh_entry->priority_info.priority = priority_5gl;
		} else if (bh_entry->bh_channel >= 100) {
			bh_entry->priority_info.priority = priority_5gh;
		}
	}

	eloop_cancel_timeout(ap_selection_reconnection_timeout, ctx, NULL);
	if(trigger_flag) {
		ctx->link_fail_single_channel_scan_count = 3;
		ctx->current_bh_substate = BH_SUBSTATE_IDLE;
		ctx->current_bh_state = BH_STATE_WIFI_BAND_SWITCHED;
		err("Block Network Optimization");
		ap_selection_issue_scan(ctx);
	}

	return 0;
}

void band_switch_by_cu_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *ctx = eloop_ctx;

	mapd_printf(MSG_ERROR, "Prohibited band switch by cu is time out");
	eloop_cancel_timeout(band_switch_by_cu_timeout, ctx, NULL);
}

Boolean is_bh_switch_allow_by_cu(struct mapd_global *global, struct mapd_radio_info *radio)
{
	struct own_1905_device *own_device = &global->dev;
	struct bh_link_entry *bh_entry = NULL;

	if (own_device->device_role == DEVICE_ROLE_CONTROLLER) {
		return 0;
	}

	if (own_device->dual_bh_en) {
		mapd_printf(MSG_INFO, "dual bh enabled");
		return 0;
	}

	// check whether band switch by cu is Prohibited
	if (eloop_is_timeout_registered(band_switch_by_cu_timeout,
							(void *)own_device, NULL)) {
		mapd_printf(MSG_INFO, "band switch by cu is Prohibited for this Device");
		return 0;
	}

       // already trigger band link change
	if (own_device->current_bh_state != BH_STATE_WIFI_LINKUP) {
		mapd_printf(MSG_INFO, "already trigger band link change");
		return 0;
	}

	if(chan_mon_is_radio_ol(global, radio) == 0) {
		mapd_printf(MSG_INFO, "Found obss not obss OL now");
		return 0;
	}

	bh_entry = get_connected_bh_entry(own_device);
	if(!bh_entry) {
		return 0;
	}

	if(bh_entry->bh_channel != radio->channel) {
		mapd_printf(MSG_INFO, "bh_band != current_band");
		return 0;
	}

	return 1;
}

/**
* @brief Fn to calculate phyrate of a bss
*
* @param ctx own 1905 device ctx
* @param radio radio info
* @param bss candidate bss
*
* @return calculate rate
*/
int estimate_radio_phyrate_by_radio_info(struct own_1905_device *ctx, struct radio_info_db *radio, struct radio_info_db *uplink_radio, signed char est_rssi
#ifdef MAP_R3
	, struct _1905_map_device *dev
#endif
)
{
	int stream = 0;
	int bw = 0;
	int wireless_mode;
	int rate = 0;

	if (!radio) {
		err("%s: radio not found", __func__);
		return 0;
	}
	wireless_mode = topo_srv_get_wireless_mode(radio->wireless_mode);

	info("wireless mode is %d", wireless_mode);
	if (wireless_mode == MODE_VHT) {
		bw = radio->radio_capability.vht_cap.vht_160?BW_160:BW_80;
		stream = radio->radio_capability.vht_cap.tx_stream + 1;
	} else if ((wireless_mode == MODE_HTMIX) || (MODE_HTGREENFIELD == wireless_mode)) {
		bw = radio->radio_capability.ht_cap.ht_40?BW_40:BW_20;
		stream = radio->radio_capability.ht_cap.rx_stream + 1;
	}

    estimate_radio_phyrate(ctx, uplink_radio, est_rssi, wireless_mode, stream, bw
#ifdef MAP_R3
		, dev
#endif
		);
	err("wireles_mode=%d, stream=%d bw=%d rate=%d RSSI=%d", wireless_mode, stream, bw, rate, est_rssi);

	return rate;
}

void bh_switch_check_by_cu(struct mapd_global *global, u8 radio_idx)
{
	struct mapd_radio_info *cur_radio = &global->dev.dev_radio_info[radio_idx];
	struct own_1905_device *own_device = &global->dev;
	struct _1905_map_device * own_1905_device = topo_srv_get_1905_device(own_device, NULL);
	struct _1905_map_device * upstream_device = own_1905_device->upstream_device;
	struct mapd_radio_info *cand_radio[MAX_NUM_OF_RADIO];
	struct mapd_radio_info *best_radio = NULL;
	struct bh_link_entry *bh_entry = NULL, *tmp_bh_entry = NULL;
	struct backhaul_link_info *bh_info = NULL;
       uint8_t cur_band = 0, cand_band= 0;
	uint8_t ra_idx = 0, cand_idx  = 0;
	int8_t  est_rssi = 0;
	uint16_t phyrate = 0;
	uint16_t air_time;
	int8_t min_cu= 100, est_cu = 0;
	uint8_t is_up_band_find = 0;

	if(!is_bh_switch_allow_by_cu(global, cur_radio))
		return;

	bh_entry = get_connected_bh_entry(own_device);
	if(!bh_entry) {
		return;
	}

	if (cur_radio->channel >= 1 && cur_radio->channel <= 14) {
		cur_band = BAND_2G;
	} else if (cur_radio->channel >=36 && cur_radio->channel <= 64) {
		cur_band = BAND_5GL;
	} else {
		cur_band = BAND_5GH;
	}
	cur_radio->cu_ol_count++;

	if (cur_radio->cu_ol_count < own_device->bh_cu_params.BHOLSteerCountTh) {

		mapd_printf(MSG_INFO, "cu ol count :%d < th (%d)", cur_radio->cu_ol_count, own_device->bh_cu_params.BHOLSteerCountTh);
		return;
	}

	cur_radio->cu_ol_count = own_device->bh_cu_params.BHOLSteerCountTh;

	if(!upstream_device) {
		mapd_printf(MSG_ERROR, "upstream_device is null");
		return;
	}

       // get the bh link metrics
	bh_info = topo_srv_get_bh_uplink_metrics_info(own_device, bh_entry);

	if(!bh_info) {
		mapd_printf(MSG_INFO, " can not find bh entry info");
		return;
	}

	/* Find NOL candidate Radios on the device */
	for (ra_idx = 0; ra_idx < MAX_NUM_OF_RADIO; ra_idx++) {
		struct mapd_radio_info *ra_info = NULL;
		ra_info = &global->dev.dev_radio_info[ra_idx];
		if (ra_info->radio_idx == (uint8_t)-1)
			continue;

		if (ra_info->channel  == bh_entry->bh_channel) {
			continue;
		}

		if (chan_mon_is_radio_ol(global, ra_info)) {
			mapd_printf(MSG_INFO, "radio  overload");
			continue;
		}

		cand_radio[cand_idx] = ra_info;
		cand_idx ++;
	}

       // select best band
	if (cand_idx != 0) {
		for (ra_idx = 0; ra_idx < cand_idx; ra_idx++) {
			struct mapd_radio_info *ra_info = cand_radio[ra_idx];
			struct bss_info_db *bss = NULL, *tmp_bss = NULL;

			if (ra_info->channel  == bh_entry->bh_channel) {
				continue;
			}

			if (ra_info->channel >= 1 && ra_info->channel <= 14) {
				cand_band = BAND_2G;
			} else if (ra_info->channel >=36 && ra_info->channel <= 64) {
				cand_band = BAND_5GL;
			} else if  (ra_info->channel >= 64) {
				cand_band = BAND_5GH;
			} else
				continue;

			// est rssi, think cur apcli interface as a sta
			bh_entry->rssi = bh_info->rx.rssi;
			est_rssi = ap_est_update_non_serving_rssi_for_bh(global, bh_entry, cur_band, cand_band);
			mapd_printf(MSG_ERROR, "bh entry rssi:%d,  est rssi:%d", bh_entry->rssi, est_rssi);
			if (est_rssi >= 0) {
				mapd_printf(MSG_ERROR, "something wrong, bh_entry->rssi =%d, est rssi =%d", bh_entry->rssi, est_rssi);
				return;
			}
			// est the phyrate
			// get upllink candidate bss and radio info
			SLIST_FOREACH_SAFE(bss, &upstream_device->first_bss, next_bss, tmp_bss) {
				if (!bss->radio) {
					err("radio for bss not found");
					return;
				}
				if (get_band_from_channel(bss->radio->channel[0]) == cand_band) {
					is_up_band_find = 1;
					break;
				}
			}

			if(!is_up_band_find)
				continue;
			phyrate = estimate_radio_phyrate_by_radio_info(own_device, topo_srv_get_radio_by_band(own_1905_device, bh_entry->bh_channel), bss->radio, est_rssi
#ifdef MAP_R3
				,own_1905_device
#endif
				);
			// est  airtime
			mapd_printf(MSG_ERROR, "bh_info->tx.tx_tp:%d, bh_info->rx.rx_tp:%d", bh_info->tx.tx_tp, bh_info->rx.rx_tp);
			air_time = ap_est_get_airtime(global, (uint16_t)(bh_info->tx.tx_tp >>17), (uint16_t)(bh_info->rx.rx_tp >>17), phyrate);
			mapd_printf(MSG_ERROR, "est phyrate:%d, est airtime:%d",  phyrate, air_time);

			mapd_printf(MSG_ERROR, "ch util on cand link:%d, ch util thread:%d", (air_time + ra_info->ch_util),  chan_mon_get_safety_th(global, ra_info->radio_idx));
			// should not make candidate band overload on both own device and upstream device
			est_cu = air_time + ra_info->ch_util;
			if ((air_time + ra_info->ch_util) > chan_mon_get_safety_th(global, ra_info->radio_idx) )
				continue;

			// find  the best one for band switch
			if(min_cu >= est_cu) {
				 if (min_cu ==  est_cu) {
					if (best_radio && (best_radio->channel < ra_info->channel))
						best_radio = ra_info;
				 } else
					best_radio = ra_info;
				 min_cu= est_cu;
			}
		}
		mapd_printf(MSG_ERROR, " find best radio done, min cu:%d", min_cu);
		cur_radio->cu_ol_count = 0;

		// trigger band switch
		if(best_radio) {
			int priority_2g = 0, priority_5gl = 0, priority_5gh = 0;
			// set band prority
			if (best_radio->channel >= 1 && best_radio->channel <= 14) {
				priority_2g = 1;
				priority_5gl = priority_5gh = 0;
			} else if (best_radio->channel >=36 && best_radio->channel <= 64) {
				priority_5gl = 1;
				priority_2g = priority_5gh = 0;
			} else if (best_radio->channel >= 64){
				priority_5gh = 1;
				priority_5gl = priority_2g = 0;
			}

			bh_entry = NULL;
			SLIST_FOREACH_SAFE(bh_entry, &(own_device->bh_link_head), next_bh_link, tmp_bh_entry) {
				bh_entry->priority_info.priority_bkp = bh_entry->priority_info.priority;
			}
			mapd_printf(MSG_ERROR, "trigger bh switch, priority_2g:%d, priority_5gl:%d, priority_5gh:%d",  priority_2g, priority_5gl, priority_5gh);
			// trigger band switch
			mapd_trigger_bh_switch(global, priority_2g, priority_5gl, priority_5gh);
			eloop_register_timeout( own_device->bh_cu_params.BHOLForbidTime, 0, band_switch_by_cu_timeout, own_device, NULL);
		} else {
			mapd_printf(MSG_ERROR, " can not find best ap");
			return;
		}
	} else {
		mapd_printf(MSG_INFO, "cand_idx ==0");
		return;
	}
}
