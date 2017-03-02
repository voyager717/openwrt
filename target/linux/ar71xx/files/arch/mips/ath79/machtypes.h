/*
 *  Atheros AR71XX/AR724X/AR913X machine type definitions
 *
 *  Copyright (C) 2008-2010 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _ATH79_MACHTYPE_H
#define _ATH79_MACHTYPE_H

#include <asm/mips_machine.h>

enum ath79_mach_type {
	ATH79_MACH_GENERIC_OF = -1,	/* Device tree board */
	ATH79_MACH_GENERIC = 0,
	ATH79_MACH_A40,				/* OpenMesh A40 */
	ATH79_MACH_A60,				/* OpenMesh A60 */
	ATH79_MACH_ALFA_AP120C,			/* ALFA Network AP120C board */
	ATH79_MACH_ALFA_AP96,			/* ALFA Network AP96 board */
	ATH79_MACH_ALFA_NX,			/* ALFA Network N2/N5 board */
	ATH79_MACH_ALL0258N,			/* Allnet ALL0258N */
	ATH79_MACH_ALL0305,			/* Allnet ALL0305 */
	ATH79_MACH_ALL0315N,			/* Allnet ALL0315N */
	ATH79_MACH_ANTMINER_S1,			/* Antminer S1 */
	ATH79_MACH_ANTMINER_S3,			/* Antminer S3 */
	ATH79_MACH_ANTROUTER_R1,		/* Antrouter R1 */
	ATH79_MACH_AP121,			/* Atheros AP121 reference board */
	ATH79_MACH_AP121_MINI,			/* Atheros AP121-MINI reference board */
	ATH79_MACH_AP132,			/* Atheros AP132 reference board */
	ATH79_MACH_AP135_020,			/* Atheros AP135-020 reference board */
	ATH79_MACH_AP136_010,			/* Atheros AP136-010 reference board */
	ATH79_MACH_AP136_020,			/* Atheros AP136-020 reference board */
	ATH79_MACH_AP143,			/* Atheros AP143 reference board */
	ATH79_MACH_AP147_010,			/* Atheros AP147-010 reference board */
	ATH79_MACH_AP152,			/* Atheros AP152 reference board */
	ATH79_MACH_AP531B0,			/* Rockeetech AP531B0 */
	ATH79_MACH_AP90Q,			/* YunCore AP90Q */
	ATH79_MACH_AP96,			/* Atheros AP96 */
	ATH79_MACH_ARCHER_C5,			/* TP-LINK Archer C5 board */
	ATH79_MACH_ARCHER_C59_V1,		/* TP-LINK Archer C59 V1 board */
	ATH79_MACH_ARCHER_C60_V1,		/* TP-LINK Archer C60 V1 board */
	ATH79_MACH_ARCHER_C7,			/* TP-LINK Archer C7 board */
	ATH79_MACH_ARCHER_C7_V2,		/* TP-LINK Archer C7 V2 board */
	ATH79_MACH_ARDUINO_YUN,			/* Yun */
	ATH79_MACH_AW_NR580,			/* AzureWave AW-NR580 */
	ATH79_MACH_BHR_4GRV2,			/* Buffalo BHR-4GRV2 */
	ATH79_MACH_BHU_BXU2000N2_A1,		/* BHU BXU2000n-2 A1 */
	ATH79_MACH_BSB,				/* Smart Electronics Black Swift board */
	ATH79_MACH_C55,				/* AirTight Networks C-55 */
	ATH79_MACH_C60,				/* AirTight Networks C-60 */
	ATH79_MACH_CAP324,			/* PowerCloud CAP324 */
	ATH79_MACH_CAP4200AG,			/* Senao CAP4200AG */
	ATH79_MACH_CARAMBOLA2,			/* 8devices Carambola2 */
	ATH79_MACH_CF_E316N_V2,			/* COMFAST CF-E316N v2 */
	ATH79_MACH_CF_E320N_V2,			/* COMFAST CF-E320N v2 */
	ATH79_MACH_CF_E380AC_V1,		/* COMFAST CF-E380AC v1 */
	ATH79_MACH_CF_E380AC_V2,		/* COMFAST CF-E380AC v2 */
	ATH79_MACH_CF_E520N,			/* COMFAST CF-E520N */
	ATH79_MACH_CF_E530N,			/* COMFAST CF-E530N */
	ATH79_MACH_CPE210,			/* TP-LINK CPE210 */
	ATH79_MACH_CPE510,			/* TP-LINK CPE510 */
	ATH79_MACH_CPE830,			/* YunCore CPE830 */
	ATH79_MACH_CPE870,			/* YunCore CPE870 */
	ATH79_MACH_CR3000,			/* PowerCloud CR3000 */
	ATH79_MACH_CR5000,			/* PowerCloud CR5000 */
	ATH79_MACH_DAP_2695_A1,			/* D-Link DAP-2695 rev. A1 */
	ATH79_MACH_DB120,			/* Atheros DB120 reference board */
	ATH79_MACH_DGL_5500_A1,			/* D-link DGL-5500 rev. A1 */
	ATH79_MACH_DHP_1565_A1,			/* D-Link DHP-1565 rev. A1 */
	ATH79_MACH_DIR_505_A1,			/* D-Link DIR-505 rev. A1 */
	ATH79_MACH_DIR_600_A1,			/* D-Link DIR-600 rev. A1 */
	ATH79_MACH_DIR_615_C1,			/* D-Link DIR-615 rev. C1 */
	ATH79_MACH_DIR_615_E1,			/* D-Link DIR-615 rev. E1 */
	ATH79_MACH_DIR_615_E4,			/* D-Link DIR-615 rev. E4 */
	ATH79_MACH_DIR_615_I1,			/* D-Link DIR-615 rev. I1 */
	ATH79_MACH_DIR_825_B1,			/* D-Link DIR-825 rev. B1 */
	ATH79_MACH_DIR_825_C1,			/* D-Link DIR-825 rev. C1 */
	ATH79_MACH_DIR_835_A1,			/* D-Link DIR-835 rev. A1 */
	ATH79_MACH_DIR_869_A1,			/* D-Link DIR-869 rev. A1 */
	ATH79_MACH_DLAN_HOTSPOT,		/* devolo dLAN Hotspot */
	ATH79_MACH_DLAN_PRO_1200_AC,		/* devolo dLAN pro 1200+ WiFi ac*/
	ATH79_MACH_DLAN_PRO_500_WP,		/* devolo dLAN pro 500 Wireless+ */
	ATH79_MACH_DOMYWIFI_DW33D,		/* DomyWifi DW33D */
	ATH79_MACH_DR344,			/* Wallys DR344 */
	ATH79_MACH_DR531,			/* Wallys DR531 */
	ATH79_MACH_DRAGINO2,			/* Dragino Version 2 */
	ATH79_MACH_EAP120,			/* TP-LINK EAP120 */
	ATH79_MACH_EAP300V2,			/* EnGenius EAP300 v2 */
	ATH79_MACH_EAP7660D,			/* Senao EAP7660D */
	ATH79_MACH_EBR_2310_C1,			/* D-link EBR-2310 rev. C1 */
	ATH79_MACH_EL_M150,			/* EasyLink EL-M150 */
	ATH79_MACH_EL_MINI,			/* EasyLink EL-MINI */
	ATH79_MACH_EPG5000,			/* EnGenius EPG5000 */
	ATH79_MACH_ESR1750,			/* EnGenius ESR1750 */
	ATH79_MACH_ESR900,			/* EnGenius ESR900 */
	ATH79_MACH_EW_DORIN,			/* embedded wireless Dorin Platform */
	ATH79_MACH_EW_DORIN_ROUTER,		/* embedded wireless Dorin Router Platform */
	ATH79_MACH_F9K1115V2,			/* Belkin AC1750DB */
	ATH79_MACH_GL_AR150,			/* GL-AR150 support */
	ATH79_MACH_GL_AR300,			/* GL-AR300 */
	ATH79_MACH_GL_AR300M,			/* GL-AR300M */
	ATH79_MACH_GL_DOMINO,			/* Domino */
	ATH79_MACH_GL_INET,			/* GL-CONNECT GL-INET */
	ATH79_MACH_GL_MIFI,			/* GL-MIFI support */
	ATH79_MACH_GS_MINIBOX_V1,		/* Gainstrong MiniBox V1.0 */
	ATH79_MACH_GS_OOLITE,			/* GS OOLITE V1.0 */
	ATH79_MACH_HIWIFI_HC6361,		/* HiWiFi HC6361 */
	ATH79_MACH_HORNET_UB,			/* ALFA Networks Hornet-UB */
	ATH79_MACH_JA76PF,			/* jjPlus JA76PF */
	ATH79_MACH_JA76PF2,			/* jjPlus JA76PF2 */
	ATH79_MACH_JWAP003,			/* jjPlus JWAP003 */
	ATH79_MACH_JWAP230,			/* jjPlus JWAP230 */
	ATH79_MACH_LIMA,			/* 8devices Lima */
	ATH79_MACH_MC_MAC1200R,			/* MERCURY MAC1200R */
	ATH79_MACH_MR12,			/* Cisco Meraki MR12 */
	ATH79_MACH_MR16,			/* Cisco Meraki MR16 */
	ATH79_MACH_MR1750,			/* OpenMesh MR1750 */
	ATH79_MACH_MR1750V2,			/* OpenMesh MR1750v2 */
	ATH79_MACH_MR18,			/* Cisco Meraki MR18 */
	ATH79_MACH_MR600,			/* OpenMesh MR600 */
	ATH79_MACH_MR600V2,			/* OpenMesh MR600v2 */
	ATH79_MACH_MR900,			/* OpenMesh MR900 */
	ATH79_MACH_MR900v2,			/* OpenMesh MR900v2 */
	ATH79_MACH_MYNET_N600,			/* WD My Net N600 */
	ATH79_MACH_MYNET_N750,			/* WD My Net N750 */
	ATH79_MACH_MYNET_REXT,			/* WD My Net Wi-Fi Range Extender */
	ATH79_MACH_MZK_W04NU,			/* Planex MZK-W04NU */
	ATH79_MACH_MZK_W300NH,			/* Planex MZK-W300NH */
	ATH79_MACH_NBG460N,			/* Zyxel NBG460N/550N/550NH */
	ATH79_MACH_NBG6616,			/* Zyxel NBG6616 */
	ATH79_MACH_NBG6716,			/* Zyxel NBG6716 */
	ATH79_MACH_OM2P,			/* OpenMesh OM2P */
	ATH79_MACH_OM2Pv2,			/* OpenMesh OM2Pv2 */
	ATH79_MACH_OM2Pv4,			/* OpenMesh OM2Pv4 */
	ATH79_MACH_OM2P_HS,			/* OpenMesh OM2P-HS */
	ATH79_MACH_OM2P_HSv2,			/* OpenMesh OM2P-HSv2 */
	ATH79_MACH_OM2P_HSv3,			/* OpenMesh OM2P-HSv3 */
	ATH79_MACH_OM2P_HSv4,			/* OpenMesh OM2P-HSv4 */
	ATH79_MACH_OM2P_LC,			/* OpenMesh OM2P-LC */
	ATH79_MACH_OM5P,			/* OpenMesh OM5P */
	ATH79_MACH_OM5P_AC,			/* OpenMesh OM5P-AC */
	ATH79_MACH_OM5P_ACv2,			/* OpenMesh OM5P-ACv2 */
	ATH79_MACH_OM5P_AN,			/* OpenMesh OM5P-AN */
	ATH79_MACH_OMY_G1,			/* OMYlink OMY-G1 */
	ATH79_MACH_OMY_X1,			/* OMYlink OMY-X1 */
	ATH79_MACH_ONION_OMEGA,			/* ONION OMEGA */
	ATH79_MACH_PB42,			/* Atheros PB42 */
	ATH79_MACH_PB44,			/* Atheros PB44 reference board */
	ATH79_MACH_PQI_AIR_PEN,			/* PQI Air Pen */
	ATH79_MACH_QIHOO_C301,			/* Qihoo 360 C301 */
	ATH79_MACH_R6100,			/* NETGEAR R6100 */
	ATH79_MACH_RB_2011G,			/* Mikrotik RouterBOARD 2011UAS-2HnD */
	ATH79_MACH_RB_2011L,			/* Mikrotik RouterBOARD 2011L */
	ATH79_MACH_RB_2011R5,			/* Mikrotik RouterBOARD 2011UiAS(-2Hnd) */
	ATH79_MACH_RB_2011US,			/* Mikrotik RouterBOARD 2011UAS */
	ATH79_MACH_RB_411,			/* MikroTik RouterBOARD 411/411A/411AH */
	ATH79_MACH_RB_411U,			/* MikroTik RouterBOARD 411U */
	ATH79_MACH_RB_433,			/* MikroTik RouterBOARD 433/433AH */
	ATH79_MACH_RB_433U,			/* MikroTik RouterBOARD 433UAH */
	ATH79_MACH_RB_435G,			/* MikroTik RouterBOARD 435G */
	ATH79_MACH_RB_450,			/* MikroTik RouterBOARD 450 */
	ATH79_MACH_RB_450G,			/* MikroTik RouterBOARD 450G */
	ATH79_MACH_RB_493,			/* Mikrotik RouterBOARD 493/493AH */
	ATH79_MACH_RB_493G,			/* Mikrotik RouterBOARD 493G */
	ATH79_MACH_RB_711GR100,			/* Mikrotik RouterBOARD 911/912 boards */
	ATH79_MACH_RB_750,			/* MikroTik RouterBOARD 750 */
	ATH79_MACH_RB_750G_R3,			/* MikroTik RouterBOARD 750GL */
	ATH79_MACH_RB_750UPR2,			/* MikroTik RouterBOARD 750UP r2 */
	ATH79_MACH_RB_751,			/* MikroTik RouterBOARD 751 */
	ATH79_MACH_RB_751G,			/* Mikrotik RouterBOARD 751G */
	ATH79_MACH_RB_922GS,			/* Mikrotik RouterBOARD 911/922GS boards */
	ATH79_MACH_RB_941,			/* MikroTik RouterBOARD 941-2nD */
	ATH79_MACH_RB_951G,			/* Mikrotik RouterBOARD 951G */
	ATH79_MACH_RB_951U,			/* Mikrotik RouterBOARD 951Ui-2HnD */
	ATH79_MACH_RB_952,			/* MikroTik RouterBOARD 951Ui-2nD */
	ATH79_MACH_RB_CAP,			/* Mikrotik RouterBOARD cAP2nD */
	ATH79_MACH_RB_MAP,			/* Mikrotik RouterBOARD mAP2nD */
	ATH79_MACH_RB_MAPL,			/* Mikrotik RouterBOARD mAP L-2nD */
	ATH79_MACH_RB_WAP,			/* Mikrotik RouterBOARD wAP2nD */
	ATH79_MACH_RB_SXTLITE2ND,		/* Mikrotik RouterBOARD SXT Lite 2nD */
	ATH79_MACH_RB_SXTLITE5ND,		/* Mikrotik RouterBOARD SXT Lite 5nD */
	ATH79_MACH_RE450,			/* TP-LINK RE450 */
	ATH79_MACH_RW2458N,			/* Redwave RW2458N */
	ATH79_MACH_SC1750,			/* Abicom SC1750 */
	ATH79_MACH_SC300M,			/* Abicom SC300M */
	ATH79_MACH_SC450,			/* Abicom SC450 */
	ATH79_MACH_SMART_300,			/* NC-LINK SMART-300 */
	ATH79_MACH_SOM9331,			/* OpenEmbed SOM9331 */
	ATH79_MACH_SR3200,			/* YunCore SR3200 */
	ATH79_MACH_TELLSTICK_ZNET_LITE,		/* TellStick ZNet Lite */
	ATH79_MACH_TEW_632BRP,			/* TRENDnet TEW-632BRP */
	ATH79_MACH_TEW_673GRU,			/* TRENDnet TEW-673GRU */
	ATH79_MACH_TEW_712BR,			/* TRENDnet TEW-712BR */
	ATH79_MACH_TEW_732BR,			/* TRENDnet TEW-732BR */
	ATH79_MACH_TEW_823DRU,			/* TRENDnet TEW-823DRU */
	ATH79_MACH_TL_MR10U,			/* TP-LINK TL-MR10U */
	ATH79_MACH_TL_MR11U,			/* TP-LINK TL-MR11U */
	ATH79_MACH_TL_MR13U,			/* TP-LINK TL-MR13U */
	ATH79_MACH_TL_MR3020,			/* TP-LINK TL-MR3020 */
	ATH79_MACH_TL_MR3040,			/* TP-LINK TL-MR3040 */
	ATH79_MACH_TL_MR3040_V2,		/* TP-LINK TL-MR3040 v2 */
	ATH79_MACH_TL_MR3220,			/* TP-LINK TL-MR3220 */
	ATH79_MACH_TL_MR3220_V2,		/* TP-LINK TL-MR3220 v2 */
	ATH79_MACH_TL_MR3420,			/* TP-LINK TL-MR3420 */
	ATH79_MACH_TL_MR3420_V2,		/* TP-LINK TL-MR3420 v2 */
	ATH79_MACH_TL_WA701ND_V2,		/* TP-LINK TL-WA701ND v2 */
	ATH79_MACH_TL_WA7210N_V2,		/* TP-LINK TL-WA7210N v2 */
	ATH79_MACH_TL_WA750RE,			/* TP-LINK TL-WA750RE */
	ATH79_MACH_TL_WA7510N_V1,		/* TP-LINK TL-WA7510N v1 */
	ATH79_MACH_TL_WA801ND_V2,		/* TP-LINK TL-WA801ND v2 */
	ATH79_MACH_TL_WA801ND_V3,		/* TP-LINK TL-WA801ND v3 */
	ATH79_MACH_TL_WA830RE_V2,		/* TP-LINK TL-WA830RE v2 */
	ATH79_MACH_TL_WA850RE,			/* TP-LINK TL-WA850RE */
	ATH79_MACH_TL_WA850RE_V2,		/* TP-LINK TL-WA850RE v2 */
	ATH79_MACH_TL_WA860RE,			/* TP-LINK TL-WA860RE */
	ATH79_MACH_TL_WA901ND,			/* TP-LINK TL-WA901ND */
	ATH79_MACH_TL_WA901ND_V2,		/* TP-LINK TL-WA901ND v2 */
	ATH79_MACH_TL_WA901ND_V3,		/* TP-LINK TL-WA901ND v3 */
	ATH79_MACH_TL_WA901ND_V4,		/* TP-LINK TL-WA901ND v4 */
	ATH79_MACH_TL_WDR3320_V2,		/* TP-LINK TL-WDR3320 v2 */
	ATH79_MACH_TL_WDR3500,			/* TP-LINK TL-WDR3500 */
	ATH79_MACH_TL_WDR4300,			/* TP-LINK TL-WDR4300 */
	ATH79_MACH_TL_WDR4900_V2,		/* TP-LINK TL-WDR4900 v2 */
	ATH79_MACH_TL_WDR6500_V2,		/* TP-LINK TL-WDR6500 v2 */
	ATH79_MACH_TL_WPA8630,			/* TP-Link TL-WPA8630 */
	ATH79_MACH_TL_WR1041N_V2,		/* TP-LINK TL-WR1041N v2 */
	ATH79_MACH_TL_WR1043ND,			/* TP-LINK TL-WR1043ND */
	ATH79_MACH_TL_WR1043ND_V2,		/* TP-LINK TL-WR1043ND v2 */
	ATH79_MACH_TL_WR1043ND_V4,		/* TP-LINK TL-WR1043ND v4 */
	ATH79_MACH_TL_WR2543N,			/* TP-LINK TL-WR2543N/ND */
	ATH79_MACH_TL_WR703N,			/* TP-LINK TL-WR703N */
	ATH79_MACH_TL_WR710N,			/* TP-LINK TL-WR710N */
	ATH79_MACH_TL_WR720N_V3,		/* TP-LINK TL-WR720N v3/v4 */
	ATH79_MACH_TL_WR741ND,			/* TP-LINK TL-WR741ND */
	ATH79_MACH_TL_WR741ND_V4,		/* TP-LINK TL-WR741ND v4 */
	ATH79_MACH_TL_WR802N_V1,		/* TP-LINK TL-WR802N v1 */
	ATH79_MACH_TL_WR802N_V2,		/* TP-LINK TL-WR802N v2 */
	ATH79_MACH_TL_WR810N,			/* TP-LINK TL-WR810N */
	ATH79_MACH_TL_WR841N_V1,		/* TP-LINK TL-WR841N v1 */
	ATH79_MACH_TL_WR841N_V11,		/* TP-LINK TL-WR841N/ND v11 */
	ATH79_MACH_TL_WR841N_V7,		/* TP-LINK TL-WR841N/ND v7 */
	ATH79_MACH_TL_WR841N_V8,		/* TP-LINK TL-WR841N/ND v8 */
	ATH79_MACH_TL_WR841N_V9,		/* TP-LINK TL-WR841N/ND v9 */
	ATH79_MACH_TL_WR842N_V2,		/* TP-LINK TL-WR842N/ND v2 */
	ATH79_MACH_TL_WR842N_V3,		/* TP-LINK TL-WR842N/ND v3 */
	ATH79_MACH_TL_WR941ND,			/* TP-LINK TL-WR941ND */
	ATH79_MACH_TL_WR941ND_V5,		/* TP-LINK TL-WR941ND v5 */
	ATH79_MACH_TL_WR941ND_V6,		/* TP-LINK TL-WR941ND v6 */
	ATH79_MACH_TL_WR940N_V4,		/* TP-LINK TL-WR940N v4 */
	ATH79_MACH_TUBE2H,			/* Alfa Network Tube2H */
	ATH79_MACH_UBNT_AIRGW,			/* Ubiquiti AirGateway */
	ATH79_MACH_UBNT_AIRGWP,			/* Ubiquiti AirGateway Pro */
	ATH79_MACH_UBNT_AIRROUTER,		/* Ubiquiti AirRouter */
	ATH79_MACH_UBNT_BULLET_M,		/* Ubiquiti Bullet M */
	ATH79_MACH_UBNT_LOCO_M_XW,		/* Ubiquiti Loco M XW */
	ATH79_MACH_UBNT_LSSR71,			/* Ubiquiti LS-SR71 */
	ATH79_MACH_UBNT_LSX,			/* Ubiquiti LSX */
	ATH79_MACH_UBNT_NANO_M,			/* Ubiquiti NanoStation M */
	ATH79_MACH_UBNT_NANO_M_XW,		/* Ubiquiti NanoStation M XW */
	ATH79_MACH_UBNT_ROCKET_M,		/* Ubiquiti Rocket M */
	ATH79_MACH_UBNT_ROCKET_M_TI,		/* Ubiquiti Rocket M TI */
	ATH79_MACH_UBNT_ROCKET_M_XW,		/* Ubiquiti Rocket M XW */
	ATH79_MACH_UBNT_RS,			/* Ubiquiti RouterStation */
	ATH79_MACH_UBNT_RSPRO,			/* Ubiquiti RouterStation Pro */
	ATH79_MACH_UBNT_UAP_PRO,		/* Ubiquiti UniFi AP Pro */
	ATH79_MACH_UBNT_UNIFI,			/* Ubiquiti Unifi */
	ATH79_MACH_UBNT_UNIFIAC_LITE,		/* Ubiquiti Unifi AC LITE/LR */
	ATH79_MACH_UBNT_UNIFIAC_PRO,		/* Ubiquiti Unifi AC PRO */
	ATH79_MACH_UBNT_UNIFI_OUTDOOR,		/* Ubiquiti UnifiAP Outdoor */
	ATH79_MACH_UBNT_UNIFI_OUTDOOR_PLUS,	/* Ubiquiti UnifiAP Outdoor+ */
	ATH79_MACH_UBNT_XM,			/* Ubiquiti Networks XM board rev 1.0 */
	ATH79_MACH_WBS210,			/* TP-LINK WBS210 */
	ATH79_MACH_WBS510,			/* TP-LINK WBS510 */
	ATH79_MACH_WEIO,			/* WeIO board */
	ATH79_MACH_WHR_G301N,			/* Buffalo WHR-G301N */
	ATH79_MACH_WHR_HP_G300N,		/* Buffalo WHR-HP-G300N */
	ATH79_MACH_WHR_HP_GN,			/* Buffalo WHR-HP-GN */
	ATH79_MACH_WLAE_AG300N,			/* Buffalo WLAE-AG300N */
	ATH79_MACH_WLR8100,			/* SITECOM WLR-8100 */
	ATH79_MACH_WNDAP360,			/* NETGEAR WNDAP360 */
	ATH79_MACH_WNDR3700,			/* NETGEAR WNDR3700/WNDR3800/WNDRMAC */
	ATH79_MACH_WNDR3700_V4,			/* NETGEAR WNDR3700v4 */
	ATH79_MACH_WNDR4300,			/* NETGEAR WNDR4300 */
	ATH79_MACH_WNR1000_V2,			/* NETGEAR WNR1000 v2 */
	ATH79_MACH_WNR2000,			/* NETGEAR WNR2000 */
	ATH79_MACH_WNR2000_V3,			/* NETGEAR WNR2000 v3 */
	ATH79_MACH_WNR2000_V4,			/* NETGEAR WNR2000 v4 */
	ATH79_MACH_WNR2200,			/* NETGEAR WNR2200 */
	ATH79_MACH_WNR612_V2,			/* NETGEAR WNR612 v2 */
	ATH79_MACH_WP543,			/* Compex WP543 */
	ATH79_MACH_WPE72,			/* Compex WPE72 */
	ATH79_MACH_WPJ342,			/* Compex WPJ342 */
	ATH79_MACH_WPJ344,			/* Compex WPJ344 */
	ATH79_MACH_WPJ531,			/* Compex WPJ531 */
	ATH79_MACH_WPJ558,			/* Compex WPJ558 */
	ATH79_MACH_WPN824N,			/* NETGEAR WPN824N */
	ATH79_MACH_WRT160NL,			/* Linksys WRT160NL */
	ATH79_MACH_WRT400N,			/* Linksys WRT400N */
	ATH79_MACH_WRTNODE2Q,			/* WRTnode2Q */
	ATH79_MACH_WZR_450HP2,			/* Buffalo WZR-450HP2 */
	ATH79_MACH_WZR_HP_AG300H,		/* Buffalo WZR-HP-AG300H */
	ATH79_MACH_WZR_HP_G300NH,		/* Buffalo WZR-HP-G300NH */
	ATH79_MACH_WZR_HP_G300NH2,		/* Buffalo WZR-HP-G300NH2 */
	ATH79_MACH_WZR_HP_G450H,		/* Buffalo WZR-HP-G450H */
	ATH79_MACH_XD3200,			/* YunCore XD3200 */
	ATH79_MACH_Z1,				/* Cisco Meraki Z1 */
	ATH79_MACH_ZBT_WE1526,			/* Zbtlink ZBT-WE1526 */
	ATH79_MACH_ZCN_1523H_2,			/* Zcomax ZCN-1523H-2-xx */
	ATH79_MACH_ZCN_1523H_5,			/* Zcomax ZCN-1523H-5-xx */
};

#endif /* _ATH79_MACHTYPE_H */
