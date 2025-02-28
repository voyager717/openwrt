// SPDX-License-Identifier: LGPL-2.1
/*
 * Example: parse TLV example
 *
 * Copyright (C) 2021 MediaTek Inc. All Rights Reserved.
 *
 * Author: Cheery Chen <Cheery.Chen@mediatek.com>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define be_to_host16(n) (n)
#define le_to_host16(n) (n)

/* TLV1: 1byte, TLV2: 2bytes, TLV3: 3bytes */
#define SIMPLE_TLV1_TYPE 0x01
#define SIMPLE_TLV2_TYPE 0x02
#define SIMPLE_TLV3_TYPE 0x03

/* TLV1: 11byte, TLV2: 12bytes, TLV3: 13bytes */
#define COMPLEX_TLV1_TYPE 0x11
#define COMPLEX_TLV2_TYPE 0x12
#define COMPLEX_TLV3_TYPE 0x13

#define MAX_VID 128

/* tlv buffer */
char buf[1000] = {0};

/* use structure to wrap TL */
struct cmdu_header_tlv {
	char type;
	unsigned short len;
	/* followed by data of length len */
}__attribute__((__packed__));

/* example simple tlv's data1 */
struct tlv_simple_data {
	char vid1;
	char vid2[2];
	char vid3[3];
};

/* example complex tlv's data2 */
struct tlv_complex_data {
	char *tlv1;
	int tlv1_len;
	char *tlv2;
	int tlv2_len;
	char *tlv3;
	int tlv3_len;
};

int cmdu_sanity_check(char *buf, int len);
int cmdu_simple_parse(char *buf, size_t buf_len, struct tlv_simple_data *tlv_data);
int cmdu_complex_parse(char *buf, size_t buf_len, struct tlv_complex_data *tlv_data);

void dump_buf(char *buf, size_t len);
int make_tlv(char **pos, char t, unsigned short l, char v);

void main()
{
	char *pos = buf;
	struct tlv_simple_data simple_data;
	struct tlv_complex_data complex_data;

	int i = 0;
	int buf_len = 0;
	int res = 0;

	// Create simple TLV
	// TVL1
	buf_len += make_tlv(&pos, SIMPLE_TLV1_TYPE, 0x01, 0x01);
	printf("buf_len len: 0x%04x\n", buf_len);
	// TVL2
	buf_len += make_tlv(&pos, SIMPLE_TLV2_TYPE, 0x02, 0x02);
	printf("buf_len len: 0x%04x\n", buf_len);
	// TVL3
	buf_len += make_tlv(&pos, SIMPLE_TLV3_TYPE, 0x03, 0x05);
	printf("buf_len len: 0x%04x\n", buf_len);

	// Create complex TLV
	// TVL4
	buf_len += make_tlv(&pos, COMPLEX_TLV1_TYPE, 0x01, 0x01);
	printf("buf_len len: 0x%04x\n", buf_len);
	// TVL5
	buf_len += make_tlv(&pos, COMPLEX_TLV2_TYPE, 0x02, 0x02);
	printf("buf_len len: 0x%04x\n", buf_len);
	// TVL6
	buf_len += make_tlv(&pos, COMPLEX_TLV3_TYPE, 0x03, 0x05);
	printf("buf_len len: 0x%04x\n", buf_len);
	// END TVL
	buf_len += make_tlv(&pos, 0x00, 0x00, 0x00);
	printf("buf_len len: 0x%04x\n", buf_len);

	// Sanity Check
	dump_buf(buf, buf_len);
	res = cmdu_sanity_check(buf, buf_len);
	if (res < 0) {
		printf("%s: TestFail:%d\n", __func__, res);
	} else {
		printf("%s: Sanity check Pass!\n", __func__);
	}

	/* simple parse */
	res = cmdu_simple_parse(buf, buf_len, &simple_data);
	if (res < 0) {
		printf("%s: Simple Parse fail:%d\n", __func__, res);
	} else {
		printf("%s: Simple Parse Success!\n", __func__);
	}
	dump_buf((char *)&simple_data, sizeof(simple_data));

	/* complex parse */
	res = cmdu_complex_parse(buf, buf_len, &complex_data);
	if (res < 0) {
		printf("%s: Complex Parse fail:%d\n", __func__, res);
	} else {
		printf("%s: Complex Parse Success!\n", __func__);
	}

	// parse sub TLV here
	if (complex_data.tlv1) {
		printf("parse tlv1, len(%d)\n", complex_data.tlv1_len);
		dump_buf(complex_data.tlv1, complex_data.tlv1_len);
	}
	if (complex_data.tlv2) {
		printf("parse tlv2, len(%d)\n", complex_data.tlv2_len);
		dump_buf(complex_data.tlv2, complex_data.tlv2_len);
	}
	if (complex_data.tlv3) {
		printf("parse tlv3, len(%d)\n", complex_data.tlv3_len);
		dump_buf(complex_data.tlv3, complex_data.tlv3_len);
	}
}

int get_cmdu_tlv_length(unsigned char *buf)
{
	unsigned char *temp_buf = buf;
	unsigned short length = 0;

	temp_buf +=1;//shift to length field

	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	return (length+3);
}

/* sanity check TLV length, if total TVL's L is not equal with buffer length, drop it*/
int cmdu_sanity_check(char *buf, int len)
{
	unsigned char *pos = buf;
	unsigned int tlv_total_len = 0;

	while (tlv_total_len < len) {
		/* end tlv */
		if (*(pos + tlv_total_len) == 0) {
			tlv_total_len += get_cmdu_tlv_length(pos + tlv_total_len);
			if (tlv_total_len == len || (tlv_total_len < 38 && len == 38))
				return 0;
			else
				return -1;
		}
		tlv_total_len += get_cmdu_tlv_length(pos + tlv_total_len);
	}
	return -2;
}

/* simple parse TLV */
int cmdu_simple_parse(char *buf, size_t buf_len, struct tlv_simple_data *tlv_data)
{
	struct cmdu_header_tlv *h = NULL;
	char *pos = buf;
	size_t left = buf_len;
	int vid2_idx = 0;
	int vid3_idx = 0;
	/* tlv */
	char type = 0;
	size_t len = 0;
	char vid = 0;

	memset(tlv_data, 0, sizeof(*tlv_data));

	/* if allow left = 3, fill '>=' here */
	while (left > sizeof(*h)) {
		h = (struct cmdu_header_tlv *)pos;
		left -= sizeof(*h);
		pos += sizeof(*h);

		/* tlv parse */
		type = h->type;
		len = be_to_host16(h->len);

		/* length check */
		if (left < len) {
			printf("left length: %d is less than tlv len: %d, drop it\n", left, len);
			return -1;
		}

		/* check end tlv */
		if (type == 0) {
			printf("Parse Finished!\n");
			continue;
		}

		/* combine type and length to check */
		if (type != SIMPLE_TLV1_TYPE && type != SIMPLE_TLV2_TYPE && type != SIMPLE_TLV3_TYPE) {
			printf("%s: Unknown TLV! type(0x%02x): err handling, Continue!\n", __func__, type);
			left -= len;
			pos += len;
			continue;
		}

		if (type == SIMPLE_TLV1_TYPE && len != 1) {
			printf("type(0x%02x), invalid length(%d)\n", type, len);
			return -1;
		}

		if (type == SIMPLE_TLV2_TYPE && len != 2) {
			printf("type(0x%02x), invalid length(%d)\n", type, len);
			return -1;
		}

		if (type == SIMPLE_TLV3_TYPE && len != 3) {
			printf("type(0x%02x), invalid length(%d)\n", type, len);
			return -1;
		}

		while (len >= sizeof(char)) {
			vid = *pos;
			pos += sizeof(char);
			left -= sizeof(char);
			len -= sizeof(char);

			/* vid data sanity check: valid value 0-128 */
			if (vid < 0 || vid > MAX_VID) {
				printf("vid(%d) is invalid \n", vid);
				continue;
			}

			if (type == SIMPLE_TLV1_TYPE)
				tlv_data->vid1 = vid;

			if (type == SIMPLE_TLV2_TYPE && vid2_idx < 2) {
				tlv_data->vid2[vid2_idx] = vid;
				vid2_idx++;
			}

			if (type == SIMPLE_TLV3_TYPE && vid3_idx < 3) {
				tlv_data->vid3[vid3_idx] = vid;
				vid3_idx++;
			}
		}
		left -= len;
		pos += len;
	}

	return 0;
}

/* complex parse TLV */
int cmdu_complex_parse_sub(struct tlv_complex_data *tlv_data, char type, char *pos, size_t len)
{
	switch (type) {
	case COMPLEX_TLV1_TYPE:
		if (tlv_data->tlv1) {
			printf("More than one COMPLEX_TLV1_TYPE TLV in the message");
			return -2;
		}
		tlv_data->tlv1 = pos;
		tlv_data->tlv1_len = len;
		break;
	case COMPLEX_TLV2_TYPE:
		if (tlv_data->tlv3) {
			printf("More than one COMPLEX_TLV2_TYPE TLV in the message");
			return -2;
		}
		tlv_data->tlv2 = pos;
		tlv_data->tlv2_len = len;
		break;
	case COMPLEX_TLV3_TYPE:
		if (tlv_data->tlv3) {
			printf("More than one COMPLEX_TLV3_TYPE TLV in the message");
			return -2;
		}
		tlv_data->tlv3 = pos;
		tlv_data->tlv3_len = len;
		break;
	default:
		/* Unknown TLV */
		return -1;
	}
	return 0;
}

int cmdu_complex_parse(char *buf, size_t buf_len, struct tlv_complex_data *tlv_data)
{
	struct cmdu_header_tlv *h = NULL;
	char *pos = buf;
	size_t left = buf_len;
	int res = 0;
	/* tlv */
	char type = 0;
	size_t len = 0;

	memset(tlv_data, 0, sizeof(*tlv_data));

	/* if allow left = 3, fill '>=' here */
	while (left > sizeof(*h)) {
		h = (struct cmdu_header_tlv *)pos;
		left -= sizeof(*h);
		pos += sizeof(*h);

		/* tlv parse */
		type = h->type;
		len = be_to_host16(h->len);

		/* length check */
		if (left < len) {
			printf("left length: %d is less than tlv len: %d, drop it\n", left, len);
			return -1;
		}

		/* parse sub tlv */
		res = cmdu_complex_parse_sub(tlv_data, type, pos, len);
		if (res == -2) {
			// Big Err: printf("repeat type: type(%d) more than one\n", type);
			break;
		}
		if (res < 0) {
			printf("%s: Unknown TLV! type(0x%02x): err handling, Continue!\n", __func__, type);
			left -= len;
			pos += len;
			continue;
		}
		left -= len;
		pos += len;
	}
	return 0;
}

/* create a fake tlv, 'v' is the same data */
int make_tlv(char **pos, char t, unsigned short l, char v)
{
	struct cmdu_header_tlv *h;
	int i = 0;
	char *type = NULL;
	unsigned short *len = NULL;
	char *value = NULL;

	//T
	type = *pos;
	*type = t;
	//L
	*pos += 1;
	len = (unsigned short *)*pos;
	*len = l;
	//V
	*pos += 2;
	value = *pos;
	for(i = 0; i < *len; i++)
		*(value+i) = v;
	*pos += *len;

	return sizeof(*h) + l;
}

void dump_buf(char *buf, size_t len)
{
	char i = 0;

	printf("dump_buf:");
	for (i = 0; i < len; i++) {
		printf("%02x,", buf[i]);
	}
	printf("\n");
}

