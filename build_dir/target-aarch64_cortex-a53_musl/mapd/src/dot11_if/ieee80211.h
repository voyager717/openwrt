/*
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2011, Mediatek, Inc.
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
 *  mapd
 *
 *  Abstract:
 *  This file is having IEEE 802.11 related common frame structure information.
 *
 *  Revision History:
 *  Who                When          What
 *  --------          ----------    -----------------------------------------
 *  Vikram Chavan     2021/06/16    IEEE 802.11 related common information.
 * */

#ifndef IEEE80211_H
#define IEEE80211_H

#define IE_MEASUREMENT_REPORT           39
#define SUBTYPE_ASSOC_REQ				0

typedef struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
	u16 Order:1;		/* Strict order expected */
	u16 Wep:1;		/* Wep data */
	u16 MoreData:1;	/* More data bit */
	u16 PwrMgmt:1;	/* Power management bit */
	u16 Retry:1;		/* Retry status bit */
	u16 MoreFrag:1;	/* More fragment bit */
	u16 FrDs:1;		/* From DS indication */
	u16 ToDs:1;		/* To DS indication */
	u16 SubType:4;	/* MSDU subtype */
	u16 Type:2;		/* MSDU type */
	u16 Ver:2;		/* Protocol version */
#else
	u16 Ver:2;		/* Protocol version */
	u16 Type:2;		/* MSDU type, refer to FC_TYPE_XX */
	u16 SubType:4;	/* MSDU subtype, refer to  SUBTYPE_XXX */
	u16 ToDs:1;		/* To DS indication */
	u16 FrDs:1;		/* From DS indication */
	u16 MoreFrag:1;	/* More fragment bit */
	u16 Retry:1;		/* Retry status bit */
	u16 PwrMgmt:1;	/* Power management bit */
	u16 MoreData:1;	/* More data bit */
	u16 Wep:1;		/* Wep data */
	u16 Order:1;		/* Strict order expected */
#endif	/* !RT_BIG_ENDIAN */
} FRAME_CONTROL, *PFRAME_CONTROL;

typedef struct GNU_PACKED _HEADER_802_11 {
	FRAME_CONTROL   FC;
	u16         Duration;
	u8           Addr1[6];
	u8           Addr2[6];
	u8		Addr3[6];
#ifdef RT_BIG_ENDIAN
	u16		Sequence:12;
	u16		Frag:4;
#else
	u16		Frag:4;
	u16		Sequence:12;
#endif /* !RT_BIG_ENDIAN */
	u8		Octet[0];
} HEADER_802_11, *PHEADER_802_11;

typedef struct GNU_PACKED _FRAME_802_11 {
	HEADER_802_11   Hdr;
	u8            Octet[1];
} FRAME_802_11, *PFRAME_802_11;

typedef struct GNU_PACKED _EID_STRUCT {
	u8   Eid;
	u8   Len;
	u8   Octet[1];
} EID_STRUCT, *PEID_STRUCT, BEACON_EID_STRUCT, *PBEACON_EID_STRUCT;
#endif
