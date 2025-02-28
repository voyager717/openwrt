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
 * Copyright  (C) 2020-2021  MediaTek Inc. All rights reserved.
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

#ifndef _QOS_H_
#define _QOS_H_




#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

#define QOS_NETLINK_EXT 27
#define MAX_MSCS_STA_CNT 20
#define MAX_EXCEPT_CNT	21
#define MAX_SCS_STA_CNT 255
#define _802_11_HEADER 24

#define MAX_FILTER_LEN	32

/*Action Frame Category*/
#define CATEGORY_QOS			1
#define CATEGORY_RAVS                   19
#ifdef QOS_R2
#define CATEGORY_VEND_SPECIFIC_PROTECTED	0x7E
#define CATEGORY_VEND_SPECIFIC				0x7F
#endif

/*Action Frame Action field*/
#ifdef QOS_R2
#define ACT_SCS_REQ	0
#define ACT_SCS_RSP	1
#endif
#define ACT_MSCS_REQ	4
#define ACT_MSCS_RSP	5

#define ACT_QOSMAP_CONFIG	4

#ifdef QOS_R2
#define IE_TCLAS_ELEMENT_ID		14  /*0x0e*/
#define IE_TCLAS_PROCESSING_ID		44  /*0x2c*/
#define IE_INTRA_ACCESS_CATE_PRI_ID	184 /*0xb8*/
#define IE_SCS_DESCRIPTOR_ELEMENT_ID	185 /*0xb9*/

#define QOS_MGMT_ELEMENT_ID	0xDD
#define MAX_TCLAS_NUM		5
#define MAX_DOMAIN_NAME_LEN	64
#define MAX_POLICY_NUM		10

/*Attribute ID*/
#define ATTR_ID_PORT_RANGE	1
#define ATTR_ID_DSCP_POLICY	2
#define ATTR_ID_TCLAS		3
#define ATTR_ID_DOMAIN_NAME	4

/*OUI Type*/
#define QOS_ACT_FRM_OUI_TYPE	0x1A
#define QOS_MGMT_IE_OUI_TYPE	0x22

/*OUI Subtype*/
#define DSCP_POLICY_QRY		0
#define DSCP_POLICY_REQ		1
#define DSCP_POLICY_RSP		2

#endif
#define IE_EXTENSION_ID_MSCS_DESC	88
#define IE_EXTENSION_ID_TCLAS_MASK	89
#define IE_WLAN_EXTENSION 255

enum {
	NL_SET_MSCS_CONFIG = 1,
	NL_SET_QOS_MAP,
	NL_SET_VEND_SPECIFIC_CONFIG,
	NL_VEND_SPECIFIC_CONFIG_EXPIRED,
	NL_RESET_QOS_CONFIG,
	NL_SET_SCS_CONFIG,
};

enum MSCS_STATUS_CODE {
	STA_SUCCESS = 0,
	STA_REQUEST_DECLINED = 37,
	STA_INSUFFICIENT_RESOURCES = 57,
	STA_REQUESTED_NOT_SUPPORTED = 80,
	STA_RESOURCES_EXHAUSTED = 81,
	STA_TERMINATED = 97,
	STA_TERMINATED_INSUFFICIENT_QOS = 128,
	STA_TERMINATED_POLICY_CONFLICT = 129,
};

enum {
	REQUEST_TYPE_ADD = 0,
	REQUEST_TYPE_REMOVE = 1,
	REQUEST_TYPE_CHANGE = 2,
	REQUEST_TYPE_UNKNOWN
};

enum {
	CLASSIFIER_TYPE0 = 0,
	CLASSIFIER_TYPE1 = 1,
	CLASSIFIER_TYPE2 = 2,
	CLASSIFIER_TYPE3 = 3,
	CLASSIFIER_TYPE4 = 4,
	CLASSIFIER_TYPE5 = 5,
	CLASSIFIER_TYPE10 = 10,
};

enum {
	UP_0 = 0,
	UP_1,
	UP_2,
	UP_3,
	UP_4,
	UP_5,
	UP_6,
	UP_7,
	UP_MAX
};

enum {
	VER_IPV4 = 4,
	VER_IPV6 = 6,
};

enum {
	BD_24G = 1,
	BD_5GL,
	BD_5GH,
};

typedef enum _RT_802_11_PHY_MODE {
	PHY_11BG_MIXED = 0,
	PHY_11B = 1,
	PHY_11A = 2,
	PHY_11ABG_MIXED = 3,
	PHY_11G = 4,
	PHY_11ABGN_MIXED = 5,	/* both band   5 */
	PHY_11N_2_4G = 6,		/* 11n-only with 2.4G band      6 */
	PHY_11GN_MIXED = 7,		/* 2.4G band      7 */
	PHY_11AN_MIXED = 8,		/* 5G  band       8 */
	PHY_11BGN_MIXED = 9,	/* if check 802.11b.      9 */
	PHY_11AGN_MIXED = 10,	/* if check 802.11b.      10 */
	PHY_11N_5G = 11,		/* 11n-only with 5G band                11 */
	PHY_11VHT_N_ABG_MIXED = 12, /* 12 -> AC/A/AN/B/G/GN mixed */
	PHY_11VHT_N_AG_MIXED = 13, /* 13 -> AC/A/AN/G/GN mixed  */
	PHY_11VHT_N_A_MIXED = 14, /* 14 -> AC/AN/A mixed in 5G band */
	PHY_11VHT_N_MIXED = 15, /* 15 -> AC/AN mixed in 5G band */
	PHY_11AX_24G = 16,
	PHY_11AX_5G = 17,
	PHY_11AX_6G = 18,
	PHY_11AX_24G_6G = 19,
	PHY_11AX_5G_6G = 20,
	PHY_11AX_24G_5G_6G = 21,
	PHY_MODE_MAX,
} RT_802_11_PHY_MODE;

struct GNU_PACKED classifier_header {
	u8 cs_type;	/*Classifier Type*/
	u8 cs_mask;	/*Classifier Mask*/
};

struct GNU_PACKED classifier_type3 {
	u8 cs_type;	/*Classifier Type*/
	u8 cs_mask;	/*Classifier Mask*/
	u16	filterOffset;
	u8	filterlen;
	u8	filterValue[MAX_FILTER_LEN];
	u8	filterMask[MAX_FILTER_LEN];
};

#ifdef QOS_R2
struct GNU_PACKED classifier_type4 {
	u8 cs_type;	/*Classifier Type*/
	u8 cs_mask;	/*Classifier Mask*/
	u8 version;
	union {
		u32	ipv4;
		u16	ipv6[8];
	} srcIp;
	union {
		u32	ipv4;
		u16	ipv6[8];
	} destIp;
	u16	srcPort;
	u16	destPort;
	u8	DSCP;
	union {
		u8	protocol;
		u8	nextheader;
	}u;
	u8	flowLabel[3];
};
#endif

struct GNU_PACKED classifier_type10 {
	u8 cs_type;	/*Classifier Type*/
	u8	protocolInstance;
	union {
		u8	protocol;
		u8	nextheader;
	}u;
	u8	filterlen;
	u8	filterValue[MAX_FILTER_LEN];
	u8	filterMask[MAX_FILTER_LEN];
};

struct GNU_PACKED classifier_parameter {
	u8 sta_mac[MAC_ADDR_LEN];
	u8 request_type;
	u8 up_bitmap;
	u8 up_limit;
	u32 timeout;
	union {
		struct classifier_header header;
		struct classifier_type3 type3;
		struct classifier_type10 type10;
	}cs;
};

#ifdef QOS_R2
struct GNU_PACKED scs_status {
	u8 scsid;
	u16 stacode;
};

typedef union GNU_PACKED _tclas_element {
	struct classifier_header header;
	struct classifier_type4 type4;
	struct classifier_type10 type10;
} tclas_element, *ptclas_element;

struct GNU_PACKED scs_classifier_parameter {
	u8 sta_mac[MAC_ADDR_LEN];
	u8 scsid;
	u8 request_type;
	u8 up;
	u8 alt_queue;
	u8 drop_elig;
	u8 processing;

	u8 tclas_num;
	tclas_element tclas_elem[MAX_TCLAS_NUM];
};

struct GNU_PACKED scs_configuration {
	struct dl_list list;
	struct scs_classifier_parameter scsparam;
};

struct GNU_PACKED qos_management_element {
	u8 elementid;
	u8 length;
	u8 oui[3];
	u8 ouitype;
};

struct GNU_PACKED attribute_header {
	u8 attrid;
	u8 length;
};

struct GNU_PACKED dscp_policy_attribute {
	u8 attrid;
	u8 length;
	u8 policyid;
	u8 request_type;
	u8 dscp;
};

struct GNU_PACKED tclas_attribute_ipv4 {
	u8 attrid;
	u8 length;
	u8 cs_type;	/*Classifier Type*/
	u8 cs_mask;	/*Classifier Mask*/
	u8 version;
	u32 srcIp;
	u32 dstIp;
	u16 srcPort;
	u16 dstPort;
	u8  DSCP;
	u8  protocol;
	u8  reserved;
};

struct GNU_PACKED tclas_attribute_ipv6 {
	u8 attrid;
	u8 length;
	u8 cs_type;	/*Classifier Type*/
	u8 cs_mask;	/*Classifier Mask*/
	u8 version;
	u16 srcIp[8];
	u16 dstIp[8];
	u16 srcPort;
	u16 dstPort;
	u8  DSCP;
	u8  nextheader;
	u8  flowLabel[3];
};

struct GNU_PACKED domain_name_attribute {
	u8 attrid;
	u8 length;
	char  domainname[MAX_DOMAIN_NAME_LEN];
};

struct GNU_PACKED port_range_attribute {
	u8 attrid;
	u8 length;
	u16 startp;
	u16 endp;
};

struct GNU_PACKED dscp_policy_db {
	struct dl_list list;
	struct dscp_policy_attribute policyattri;
	union {
		struct attribute_header header;
		struct tclas_attribute_ipv4 ipv4;
		struct tclas_attribute_ipv6 ipv6;
	} tclasattri;
	struct domain_name_attribute domainattri;
	struct port_range_attribute portrange;
};

struct GNU_PACKED dscp_policy_req {
	u8 id_num;
	u8 policyids[MAX_POLICY_NUM];
	u8 token;
	u8 moreflag;
	u8 resetflag;
};

#endif

struct GNU_PACKED wapp_qos_action_frame {
	u8 src[MAC_ADDR_LEN];
	u32 frmid;
	u32 chan;
	u32 frm_len;
	u8 frm[0];
};

struct GNU_PACKED qos_netlink_message {
	unsigned char type;
	unsigned short len;
	unsigned char data[0];
};

struct GNU_PACKED mscs_configuration {
	struct dl_list list;
	struct classifier_parameter csparam;
};

struct GNU_PACKED dscp_exception {
	u8 dscp;
	u8 up;
};

struct GNU_PACKED dscp_range {
	u8 low;
	u8 high;
};

struct GNU_PACKED qos_map {
	u8 num;	/*dscp exception number*/
	struct dscp_exception dscp_ex[MAX_EXCEPT_CNT];
	struct dscp_range dscp_rg[UP_MAX];
};

struct qos_wifi_config {
	u8 update;
	u8 channel;
	u8 mode;
	char ssid[32];
	char keymgnt[16];
	char psk[16];
	u8 qosmap_set;
	u8 stamac[MAC_ADDR_LEN];
#ifdef QOS_R2
	u8 RequestCtrlReset;
	u8 dscppolicycapa;
	u8 qosmapcapa;
#endif
};

struct GNU_PACKED wapp_vend_spec_classifier_para_report {
	u32 id;
	u8 sta_mac[MAC_ADDR_LEN];
	u8 request_type;
	u8 up;
	u8 delay_bound;
	u8 protocol;
	u32 timeout;
	unsigned long expired;
	u8 version;
	union {
		u32	ipv4;
		u8	ipv6[16];
	} srcIp;
	union {
		u32	ipv4;
		u8	ipv6[16];
	} destIp;
	u16	srcPort;
	u16	destPort;

	char	ifname[IFNAMSIZ];
};

int qos_cmd_reset_default(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int qos_cmd_set_config(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int qos_cmd_commit_config(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int qos_ctrl_interface_cmd_handle(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int qos_cmd_show_help(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int qos_cmd_show_config(struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_drv_qos_init(struct wifi_app *wapp);
int wapp_drv_qos_deinit(struct wifi_app *wapp);
int wapp_ctrl_iface_cmd_qos(struct wifi_app *wapp, const char *iface,
		char *param_value_pair, char *reply, size_t *reply_len);
void wapp_handle_qos_action_frame(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data);
void wapp_handle_mscs_descriptor_element(struct wifi_app *wapp,
		u32 ifindex, wapp_event_data *event_data);
void wdev_handle_vend_spec_up_tuple_event(struct wifi_app *wapp,
		u32 ifindex, wapp_event_data *event_data);

void wapp_send_qos_map_configure_frame(struct wifi_app *wapp, const char *ifname,
	unsigned int channel, const u8 *dst, struct dscp_exception *dscp_ex, u8 ex_cnt, struct dscp_range *dscp_rg);


#endif /* #ifndef _QOS_H_ */

