/*
 * DPP functionality shared between hostapd and wpa_supplicant
 * Copyright (c) 2017, Qualcomm Atheros, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See BSD_LICENSE for more details.
 */

#include "types.h"
#include <openssl/opensslv.h>
#include <openssl/err.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/evp.h>
#include "crypto/evp.h"
#include "wapp_cmm.h"

#ifdef OPENWRT_SUPPORT
#include <libdatconf.h>
#endif

#include "gas.h"
#include "gas/gas_query.h"
#include "crypto/crypto.h"
#include "crypto/random.h"
#include "crypto/aes.h"
#include "crypto/aes_siv.h"
#include "crypto/sha384.h"
#include "crypto/sha512.h"
#include "dpp.h"
#include "dpp_wdev.h"
#include "debug.h"
#include "utils/wpabuf.h"
#include "utils/base64.h"
#include "wpa_debug.h"
#include "util.h"
#include "utils/json.h"
#include "utils/ip_addr.h"

#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
	(defined(LIBRESSL_VERSION_NUMBER) && \
	 LIBRESSL_VERSION_NUMBER < 0x20700000L)
/* Compatibility wrappers for older versions. */

#define DPP_CHIRP_TLV 0xD3
#ifdef MAP_R3
extern u8 obj_count;
#endif /* MAP_R3*/


static int ECDSA_SIG_set0(ECDSA_SIG *sig, BIGNUM *r, BIGNUM *s)
{
	sig->r = r;
	sig->s = s;
	return 1;
}


static void ECDSA_SIG_get0(const ECDSA_SIG *sig, const BIGNUM **pr,
			   const BIGNUM **ps)
{
	if (pr)
		*pr = sig->r;
	if (ps)
		*ps = sig->s;
}

#endif


const struct dpp_curve_params dpp_curves[] = {
	/* The mandatory to support and the default NIST P-256 curve needs to
	 * be the first entry on this list. */
	{ "prime256v1", 32, 32, 16, 32, "P-256", 19, "ES256" },
	{ "secp384r1", 48, 48, 24, 48, "P-384", 20, "ES384" },
	{ "secp521r1", 64, 64, 32, 66, "P-521", 21, "ES512" },
	{ "brainpoolP256r1", 32, 32, 16, 32, "BP-256", 28, "BS256" },
	{ "brainpoolP384r1", 48, 48, 24, 48, "BP-384", 29, "BS384" },
	{ "brainpoolP512r1", 64, 64, 32, 64, "BP-512", 30, "BS512" },
	{ NULL, 0, 0, 0, 0, NULL, 0, NULL }
};


/* Role-specific elements for PKEX */

/* NIST P-256 */
static const u8 pkex_init_x_p256[32] = {
	0x56, 0x26, 0x12, 0xcf, 0x36, 0x48, 0xfe, 0x0b,
	0x07, 0x04, 0xbb, 0x12, 0x22, 0x50, 0xb2, 0x54,
	0xb1, 0x94, 0x64, 0x7e, 0x54, 0xce, 0x08, 0x07,
	0x2e, 0xec, 0xca, 0x74, 0x5b, 0x61, 0x2d, 0x25
 };
static const u8 pkex_init_y_p256[32] = {
	0x3e, 0x44, 0xc7, 0xc9, 0x8c, 0x1c, 0xa1, 0x0b,
	0x20, 0x09, 0x93, 0xb2, 0xfd, 0xe5, 0x69, 0xdc,
	0x75, 0xbc, 0xad, 0x33, 0xc1, 0xe7, 0xc6, 0x45,
	0x4d, 0x10, 0x1e, 0x6a, 0x3d, 0x84, 0x3c, 0xa4
 };
static const u8 pkex_resp_x_p256[32] = {
	0x1e, 0xa4, 0x8a, 0xb1, 0xa4, 0xe8, 0x42, 0x39,
	0xad, 0x73, 0x07, 0xf2, 0x34, 0xdf, 0x57, 0x4f,
	0xc0, 0x9d, 0x54, 0xbe, 0x36, 0x1b, 0x31, 0x0f,
	0x59, 0x91, 0x52, 0x33, 0xac, 0x19, 0x9d, 0x76
};
static const u8 pkex_resp_y_p256[32] = {
	0xd9, 0xfb, 0xf6, 0xb9, 0xf5, 0xfa, 0xdf, 0x19,
	0x58, 0xd8, 0x3e, 0xc9, 0x89, 0x7a, 0x35, 0xc1,
	0xbd, 0xe9, 0x0b, 0x77, 0x7a, 0xcb, 0x91, 0x2a,
	0xe8, 0x21, 0x3f, 0x47, 0x52, 0x02, 0x4d, 0x67
};

/* NIST P-384 */
static const u8 pkex_init_x_p384[48] = {
	0x95, 0x3f, 0x42, 0x9e, 0x50, 0x7f, 0xf9, 0xaa,
	0xac, 0x1a, 0xf2, 0x85, 0x2e, 0x64, 0x91, 0x68,
	0x64, 0xc4, 0x3c, 0xb7, 0x5c, 0xf8, 0xc9, 0x53,
	0x6e, 0x58, 0x4c, 0x7f, 0xc4, 0x64, 0x61, 0xac,
	0x51, 0x8a, 0x6f, 0xfe, 0xab, 0x74, 0xe6, 0x12,
	0x81, 0xac, 0x38, 0x5d, 0x41, 0xe6, 0xb9, 0xa3
};
static const u8 pkex_init_y_p384[48] = {
	0x76, 0x2f, 0x68, 0x84, 0xa6, 0xb0, 0x59, 0x29,
	0x83, 0xa2, 0x6c, 0xa4, 0x6c, 0x3b, 0xf8, 0x56,
	0x76, 0x11, 0x2a, 0x32, 0x90, 0xbd, 0x07, 0xc7,
	0x37, 0x39, 0x9d, 0xdb, 0x96, 0xf3, 0x2b, 0xb6,
	0x27, 0xbb, 0x29, 0x3c, 0x17, 0x33, 0x9d, 0x94,
	0xc3, 0xda, 0xac, 0x46, 0xb0, 0x8e, 0x07, 0x18
};
static const u8 pkex_resp_x_p384[48] = {
	0xad, 0xbe, 0xd7, 0x1d, 0x3a, 0x71, 0x64, 0x98,
	0x5f, 0xb4, 0xd6, 0x4b, 0x50, 0xd0, 0x84, 0x97,
	0x4b, 0x7e, 0x57, 0x70, 0xd2, 0xd9, 0xf4, 0x92,
	0x2a, 0x3f, 0xce, 0x99, 0xc5, 0x77, 0x33, 0x44,
	0x14, 0x56, 0x92, 0xcb, 0xae, 0x46, 0x64, 0xdf,
	0xe0, 0xbb, 0xd7, 0xb1, 0x29, 0x20, 0x72, 0xdf
};
static const u8 pkex_resp_y_p384[48] = {
	0xab, 0xa7, 0xdf, 0x52, 0xaa, 0xe2, 0x35, 0x0c,
	0xe3, 0x75, 0x32, 0xe6, 0xbf, 0x06, 0xc8, 0x7c,
	0x38, 0x29, 0x4c, 0xec, 0x82, 0xac, 0xd7, 0xa3,
	0x09, 0xd2, 0x0e, 0x22, 0x5a, 0x74, 0x52, 0xa1,
	0x7e, 0x54, 0x4e, 0xfe, 0xc6, 0x29, 0x33, 0x63,
	0x15, 0xe1, 0x7b, 0xe3, 0x40, 0x1c, 0xca, 0x06
};

/* NIST P-521 */
static const u8 pkex_init_x_p521[66] = {
	0x00, 0x16, 0x20, 0x45, 0x19, 0x50, 0x95, 0x23,
	0x0d, 0x24, 0xbe, 0x00, 0x87, 0xdc, 0xfa, 0xf0,
	0x58, 0x9a, 0x01, 0x60, 0x07, 0x7a, 0xca, 0x76,
	0x01, 0xab, 0x2d, 0x5a, 0x46, 0xcd, 0x2c, 0xb5,
	0x11, 0x9a, 0xff, 0xaa, 0x48, 0x04, 0x91, 0x38,
	0xcf, 0x86, 0xfc, 0xa4, 0xa5, 0x0f, 0x47, 0x01,
	0x80, 0x1b, 0x30, 0xa3, 0xae, 0xe8, 0x1c, 0x2e,
	0xea, 0xcc, 0xf0, 0x03, 0x9f, 0x77, 0x4c, 0x8d,
	0x97, 0x76
};
static const u8 pkex_init_y_p521[66] = {
	0x00, 0xb3, 0x8e, 0x02, 0xe4, 0x2a, 0x63, 0x59,
	0x12, 0xc6, 0x10, 0xba, 0x3a, 0xf9, 0x02, 0x99,
	0x3f, 0x14, 0xf0, 0x40, 0xde, 0x5c, 0xc9, 0x8b,
	0x02, 0x55, 0xfa, 0x91, 0xb1, 0xcc, 0x6a, 0xbd,
	0xe5, 0x62, 0xc0, 0xc5, 0xe3, 0xa1, 0x57, 0x9f,
	0x08, 0x1a, 0xa6, 0xe2, 0xf8, 0x55, 0x90, 0xbf,
	0xf5, 0xa6, 0xc3, 0xd8, 0x52, 0x1f, 0xb7, 0x02,
	0x2e, 0x7c, 0xc8, 0xb3, 0x20, 0x1e, 0x79, 0x8d,
	0x03, 0xa8
};
static const u8 pkex_resp_x_p521[66] = {
	0x00, 0x79, 0xe4, 0x4d, 0x6b, 0x5e, 0x12, 0x0a,
	0x18, 0x2c, 0xb3, 0x05, 0x77, 0x0f, 0xc3, 0x44,
	0x1a, 0xcd, 0x78, 0x46, 0x14, 0xee, 0x46, 0x3f,
	0xab, 0xc9, 0x59, 0x7c, 0x85, 0xa0, 0xc2, 0xfb,
	0x02, 0x32, 0x99, 0xde, 0x5d, 0xe1, 0x0d, 0x48,
	0x2d, 0x71, 0x7d, 0x8d, 0x3f, 0x61, 0x67, 0x9e,
	0x2b, 0x8b, 0x12, 0xde, 0x10, 0x21, 0x55, 0x0a,
	0x5b, 0x2d, 0xe8, 0x05, 0x09, 0xf6, 0x20, 0x97,
	0x84, 0xb4
};
static const u8 pkex_resp_y_p521[66] = {
	0x00, 0x46, 0x63, 0x39, 0xbe, 0xcd, 0xa4, 0x2d,
	0xca, 0x27, 0x74, 0xd4, 0x1b, 0x91, 0x33, 0x20,
	0x83, 0xc7, 0x3b, 0xa4, 0x09, 0x8b, 0x8e, 0xa3,
	0x88, 0xe9, 0x75, 0x7f, 0x56, 0x7b, 0x38, 0x84,
	0x62, 0x02, 0x7c, 0x90, 0x51, 0x07, 0xdb, 0xe9,
	0xd0, 0xde, 0xda, 0x9a, 0x5d, 0xe5, 0x94, 0xd2,
	0xcf, 0x9d, 0x4c, 0x33, 0x91, 0xa6, 0xc3, 0x80,
	0xa7, 0x6e, 0x7e, 0x8d, 0xf8, 0x73, 0x6e, 0x53,
	0xce, 0xe1
};

/* Brainpool P-256r1 */
static const u8 pkex_init_x_bp_p256r1[32] = {
	0x46, 0x98, 0x18, 0x6c, 0x27, 0xcd, 0x4b, 0x10,
	0x7d, 0x55, 0xa3, 0xdd, 0x89, 0x1f, 0x9f, 0xca,
	0xc7, 0x42, 0x5b, 0x8a, 0x23, 0xed, 0xf8, 0x75,
	0xac, 0xc7, 0xe9, 0x8d, 0xc2, 0x6f, 0xec, 0xd8
};
static const u8 pkex_init_y_bp_p256r1[32] = {
	0x93, 0xca, 0xef, 0xa9, 0x66, 0x3e, 0x87, 0xcd,
	0x52, 0x6e, 0x54, 0x13, 0xef, 0x31, 0x67, 0x30,
	0x15, 0x13, 0x9d, 0x6d, 0xc0, 0x95, 0x32, 0xbe,
	0x4f, 0xab, 0x5d, 0xf7, 0xbf, 0x5e, 0xaa, 0x0b
};
static const u8 pkex_resp_x_bp_p256r1[32] = {
	0x90, 0x18, 0x84, 0xc9, 0xdc, 0xcc, 0xb5, 0x2f,
	0x4a, 0x3f, 0x4f, 0x18, 0x0a, 0x22, 0x56, 0x6a,
	0xa9, 0xef, 0xd4, 0xe6, 0xc3, 0x53, 0xc2, 0x1a,
	0x23, 0x54, 0xdd, 0x08, 0x7e, 0x10, 0xd8, 0xe3
};
static const u8 pkex_resp_y_bp_p256r1[32] = {
	0x2a, 0xfa, 0x98, 0x9b, 0xe3, 0xda, 0x30, 0xfd,
	0x32, 0x28, 0xcb, 0x66, 0xfb, 0x40, 0x7f, 0xf2,
	0xb2, 0x25, 0x80, 0x82, 0x44, 0x85, 0x13, 0x7e,
	0x4b, 0xb5, 0x06, 0xc0, 0x03, 0x69, 0x23, 0x64
};

/* Brainpool P-384r1 */
static const u8 pkex_init_x_bp_p384r1[48] = {
	0x0a, 0x2c, 0xeb, 0x49, 0x5e, 0xb7, 0x23, 0xbd,
	0x20, 0x5b, 0xe0, 0x49, 0xdf, 0xcf, 0xcf, 0x19,
	0x37, 0x36, 0xe1, 0x2f, 0x59, 0xdb, 0x07, 0x06,
	0xb5, 0xeb, 0x2d, 0xae, 0xc2, 0xb2, 0x38, 0x62,
	0xa6, 0x73, 0x09, 0xa0, 0x6c, 0x0a, 0xa2, 0x30,
	0x99, 0xeb, 0xf7, 0x1e, 0x47, 0xb9, 0x5e, 0xbe
};
static const u8 pkex_init_y_bp_p384r1[48] = {
	0x54, 0x76, 0x61, 0x65, 0x75, 0x5a, 0x2f, 0x99,
	0x39, 0x73, 0xca, 0x6c, 0xf9, 0xf7, 0x12, 0x86,
	0x54, 0xd5, 0xd4, 0xad, 0x45, 0x7b, 0xbf, 0x32,
	0xee, 0x62, 0x8b, 0x9f, 0x52, 0xe8, 0xa0, 0xc9,
	0xb7, 0x9d, 0xd1, 0x09, 0xb4, 0x79, 0x1c, 0x3e,
	0x1a, 0xbf, 0x21, 0x45, 0x66, 0x6b, 0x02, 0x52
};
static const u8 pkex_resp_x_bp_p384r1[48] = {
	0x03, 0xa2, 0x57, 0xef, 0xe8, 0x51, 0x21, 0xa0,
	0xc8, 0x9e, 0x21, 0x02, 0xb5, 0x9a, 0x36, 0x25,
	0x74, 0x22, 0xd1, 0xf2, 0x1b, 0xa8, 0x9a, 0x9b,
	0x97, 0xbc, 0x5a, 0xeb, 0x26, 0x15, 0x09, 0x71,
	0x77, 0x59, 0xec, 0x8b, 0xb7, 0xe1, 0xe8, 0xce,
	0x65, 0xb8, 0xaf, 0xf8, 0x80, 0xae, 0x74, 0x6c
};
static const u8 pkex_resp_y_bp_p384r1[48] = {
	0x2f, 0xd9, 0x6a, 0xc7, 0x3e, 0xec, 0x76, 0x65,
	0x2d, 0x38, 0x7f, 0xec, 0x63, 0x26, 0x3f, 0x04,
	0xd8, 0x4e, 0xff, 0xe1, 0x0a, 0x51, 0x74, 0x70,
	0xe5, 0x46, 0x63, 0x7f, 0x5c, 0xc0, 0xd1, 0x7c,
	0xfb, 0x2f, 0xea, 0xe2, 0xd8, 0x0f, 0x84, 0xcb,
	0xe9, 0x39, 0x5c, 0x64, 0xfe, 0xcb, 0x2f, 0xf1
};

/* Brainpool P-512r1 */
static const u8 pkex_init_x_bp_p512r1[64] = {
	0x4c, 0xe9, 0xb6, 0x1c, 0xe2, 0x00, 0x3c, 0x9c,
	0xa9, 0xc8, 0x56, 0x52, 0xaf, 0x87, 0x3e, 0x51,
	0x9c, 0xbb, 0x15, 0x31, 0x1e, 0xc1, 0x05, 0xfc,
	0x7c, 0x77, 0xd7, 0x37, 0x61, 0x27, 0xd0, 0x95,
	0x98, 0xee, 0x5d, 0xa4, 0x3d, 0x09, 0xdb, 0x3d,
	0xfa, 0x89, 0x9e, 0x7f, 0xa6, 0xa6, 0x9c, 0xff,
	0x83, 0x5c, 0x21, 0x6c, 0x3e, 0xf2, 0xfe, 0xdc,
	0x63, 0xe4, 0xd1, 0x0e, 0x75, 0x45, 0x69, 0x0f
};
static const u8 pkex_init_y_bp_p512r1[64] = {
	0x50, 0xb5, 0x9b, 0xfa, 0x45, 0x67, 0x75, 0x94,
	0x44, 0xe7, 0x68, 0xb0, 0xeb, 0x3e, 0xb3, 0xb8,
	0xf9, 0x99, 0x05, 0xef, 0xae, 0x6c, 0xbc, 0xe3,
	0xe1, 0xd2, 0x51, 0x54, 0xdf, 0x59, 0xd4, 0x45,
	0x41, 0x3a, 0xa8, 0x0b, 0x76, 0x32, 0x44, 0x0e,
	0x07, 0x60, 0x3a, 0x6e, 0xbe, 0xfe, 0xe0, 0x58,
	0x52, 0xa0, 0xaa, 0x8b, 0xd8, 0x5b, 0xf2, 0x71,
	0x11, 0x9a, 0x9e, 0x8f, 0x1a, 0xd1, 0xc9, 0x99
};
static const u8 pkex_resp_x_bp_p512r1[64] = {
	0x2a, 0x60, 0x32, 0x27, 0xa1, 0xe6, 0x94, 0x72,
	0x1c, 0x48, 0xbe, 0xc5, 0x77, 0x14, 0x30, 0x76,
	0xe4, 0xbf, 0xf7, 0x7b, 0xc5, 0xfd, 0xdf, 0x19,
	0x1e, 0x0f, 0xdf, 0x1c, 0x40, 0xfa, 0x34, 0x9e,
	0x1f, 0x42, 0x24, 0xa3, 0x2c, 0xd5, 0xc7, 0xc9,
	0x7b, 0x47, 0x78, 0x96, 0xf1, 0x37, 0x0e, 0x88,
	0xcb, 0xa6, 0x52, 0x29, 0xd7, 0xa8, 0x38, 0x29,
	0x8e, 0x6e, 0x23, 0x47, 0xd4, 0x4b, 0x70, 0x3e
};
static const u8 pkex_resp_y_bp_p512r1[64] = {
	0x80, 0x1f, 0x43, 0xd2, 0x17, 0x35, 0xec, 0x81,
	0xd9, 0x4b, 0xdc, 0x81, 0x19, 0xd9, 0x5f, 0x68,
	0x16, 0x84, 0xfe, 0x63, 0x4b, 0x8d, 0x5d, 0xaa,
	0x88, 0x4a, 0x47, 0x48, 0xd4, 0xea, 0xab, 0x7d,
	0x6a, 0xbf, 0xe1, 0x28, 0x99, 0x6a, 0x87, 0x1c,
	0x30, 0xb4, 0x44, 0x2d, 0x75, 0xac, 0x35, 0x09,
	0x73, 0x24, 0x3d, 0xb4, 0x43, 0xb1, 0xc1, 0x56,
	0x56, 0xad, 0x30, 0x87, 0xf4, 0xc3, 0x00, 0xc7
};


static void dpp_debug_print_point(const char *title, const EC_GROUP *group,
				  const EC_POINT *point)
{
	BIGNUM *x, *y;
	BN_CTX *ctx;
	char *x_str = NULL, *y_str = NULL;

	ctx = BN_CTX_new();
	x = BN_new();
	y = BN_new();
	if (!ctx || !x || !y ||
	    EC_POINT_get_affine_coordinates_GFp(group, point, x, y, ctx) != 1)
		goto fail;

	x_str = BN_bn2hex(x);
	y_str = BN_bn2hex(y);
	if (!x_str || !y_str)
		goto fail;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX DPP_MAP_PREX "%s (%s,%s)", title, x_str, y_str);

fail:
	OPENSSL_free(x_str);
	OPENSSL_free(y_str);
	BN_free(x);
	BN_free(y);
	BN_CTX_free(ctx);
}


static int dpp_hash_vector(const struct dpp_curve_params *curve,
			   size_t num_elem, const u8 *addr[], const size_t *len,
			   u8 *mac)
{
	if (curve->hash_len == 32)
		return sha256_vector(num_elem, addr, len, mac);
	if (curve->hash_len == 48)
		return sha384_vector(num_elem, addr, len, mac);
	if (curve->hash_len == 64)
		return sha512_vector(num_elem, addr, len, mac);
	return -1;
}


int dpp_hkdf_expand(size_t hash_len, const u8 *secret, size_t secret_len,
			   const char *label, u8 *out, size_t outlen)
{
	if (hash_len == 32)
		return hmac_sha256_kdf(secret, secret_len, NULL,
				       (const u8 *) label, os_strlen(label),
				       out, outlen);
	if (hash_len == 48)
		return hmac_sha384_kdf(secret, secret_len, NULL,
				       (const u8 *) label, os_strlen(label),
				       out, outlen);
	if (hash_len == 64)
		return hmac_sha512_kdf(secret, secret_len, NULL,
				       (const u8 *) label, os_strlen(label),
				       out, outlen);
	return -1;
}


static int dpp_hmac_vector(size_t hash_len, const u8 *key, size_t key_len,
			   size_t num_elem, const u8 *addr[],
			   const size_t *len, u8 *mac)
{
	if (hash_len == 32)
		return hmac_sha256_vector(key, key_len, num_elem, addr, len,
					  mac);
	if (hash_len == 48)
		return hmac_sha384_vector(key, key_len, num_elem, addr, len,
					  mac);
	if (hash_len == 64)
		return hmac_sha512_vector(key, key_len, num_elem, addr, len,
					  mac);
	return -1;
}


int dpp_hmac(size_t hash_len, const u8 *key, size_t key_len,
		    const u8 *data, size_t data_len, u8 *mac)
{
	if (hash_len == 32)
		return hmac_sha256(key, key_len, data, data_len, mac);
	if (hash_len == 48)
		return hmac_sha384(key, key_len, data, data_len, mac);
	if (hash_len == 64)
		return hmac_sha512(key, key_len, data, data_len, mac);
	return -1;
}


int dpp_bn2bin_pad(const BIGNUM *bn, u8 *pos, size_t len)
{
	int num_bytes, offset;

	num_bytes = BN_num_bytes(bn);
	if ((size_t) num_bytes > len)
		return -1;
	offset = len - num_bytes;
	os_memset(pos, 0, offset);
	BN_bn2bin(bn, pos + offset);
	return 0;
}


struct wpabuf * dpp_get_pubkey_point(EVP_PKEY *pkey, int prefix)
{
	int len, res;
	EC_KEY *eckey;
	struct wpabuf *buf;
	unsigned char *pos;

	eckey = EVP_PKEY_get1_EC_KEY(pkey);
	if (!eckey)
		return NULL;
	EC_KEY_set_conv_form(eckey, POINT_CONVERSION_UNCOMPRESSED);
	len = i2o_ECPublicKey(eckey, NULL);
	if (len <= 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DDP: Failed to determine public key encoding length");
		EC_KEY_free(eckey);
		return NULL;
	}

	buf = wpabuf_alloc(len);
	if (!buf) {
		EC_KEY_free(eckey);
		return NULL;
	}

	pos = wpabuf_put(buf, len);
	res = i2o_ECPublicKey(eckey, &pos);
	EC_KEY_free(eckey);
	if (res != len) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DDP: Failed to encode public key (res=%d/%d)",
			   res, len);
		wpabuf_free(buf);
		return NULL;
	}

	if (!prefix) {
		/* Remove 0x04 prefix to match DPP definition */
		pos = wpabuf_mhead(buf);
		os_memmove(pos, pos + 1, len - 1);
		buf->used--;
	}

	return buf;
}


EVP_PKEY * dpp_set_pubkey_point_group(const EC_GROUP *group,
					     const u8 *buf_x, const u8 *buf_y,
					     size_t len)
{
	EC_KEY *eckey = NULL;
	BN_CTX *ctx;
	EC_POINT *point = NULL;
	BIGNUM *x = NULL, *y = NULL;
	EVP_PKEY *pkey = NULL;

	ctx = BN_CTX_new();
	if (!ctx) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Out of memory");
		return NULL;
	}

	point = EC_POINT_new(group);
	x = BN_bin2bn(buf_x, len, NULL);
	y = BN_bin2bn(buf_y, len, NULL);
	if (!point || !x || !y) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Out of memory");
		goto fail;
	}

	if (!EC_POINT_set_affine_coordinates_GFp(group, point, x, y, ctx)) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: OpenSSL: EC_POINT_set_affine_coordinates_GFp failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	if (!EC_POINT_is_on_curve(group, point, ctx) ||
	    EC_POINT_is_at_infinity(group, point)) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Invalid point");
		goto fail;
	}
	dpp_debug_print_point("DPP: dpp_set_pubkey_point_group", group, point);

	eckey = EC_KEY_new();
	if (!eckey ||
	    EC_KEY_set_group(eckey, group) != 1 ||
	    EC_KEY_set_public_key(eckey, point) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to set EC_KEY: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	EC_KEY_set_asn1_flag(eckey, OPENSSL_EC_NAMED_CURVE);

	pkey = EVP_PKEY_new();
	if (!pkey || EVP_PKEY_set1_EC_KEY(pkey, eckey) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Could not create EVP_PKEY");
		goto fail;
	}

out:
	BN_free(x);
	BN_free(y);
	EC_KEY_free(eckey);
	EC_POINT_free(point);
	BN_CTX_free(ctx);
	return pkey;
fail:
	EVP_PKEY_free(pkey);
	pkey = NULL;
	goto out;
}


EVP_PKEY * dpp_set_pubkey_point(EVP_PKEY *group_key,
				       const u8 *buf, size_t len)
{
	EC_KEY *eckey;
	const EC_GROUP *group;
	EVP_PKEY *pkey = NULL;

	if (len & 1)
		return NULL;

	eckey = EVP_PKEY_get1_EC_KEY(group_key);
	if (!eckey) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Could not get EC_KEY from group_key");
		return NULL;
	}

	group = EC_KEY_get0_group(eckey);
	if (group)
		pkey = dpp_set_pubkey_point_group(group, buf, buf + len / 2,
						  len / 2);
	else
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Could not get EC group");

	EC_KEY_free(eckey);
	return pkey;
}


void dpp_auth_fail(struct dpp_authentication *auth, const char *txt)
{
	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX" %s [%s] \n", txt, __func__);
#ifdef MAP_R3
	conn_fail_reason *info_to_mapd = os_zalloc(sizeof(conn_fail_reason));
	if (info_to_mapd == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"[%s] info malloc failed\n", __func__);
		return;
	}
	if (!auth) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"[%s] Auth instance not found\n", __func__);
		os_free(info_to_mapd);
		return;
	}
	os_memcpy(info_to_mapd->reason, txt, sizeof(info_to_mapd->reason));
	wapp_send_1905_msg(
			auth->msg_ctx,
			WAPP_SEND_CONN_FAIL_NOTIF,
			sizeof(conn_fail_reason),
			(char *)info_to_mapd);
	os_free(info_to_mapd);
	return;
#endif
}


struct wpabuf * dpp_alloc_msg(enum dpp_public_action_frame_type type,
			      size_t len)
{
	struct wpabuf *msg;

	msg = wpabuf_alloc(8 + len);
	if (!msg)
		return NULL;
	wpabuf_put_u8(msg, WLAN_ACTION_PUBLIC);
	wpabuf_put_u8(msg, WLAN_PA_VENDOR_SPECIFIC);
	wpabuf_put_be24(msg, OUI_WFA);
	wpabuf_put_u8(msg, DPP_OUI_TYPE);
	wpabuf_put_u8(msg, 1); /* Crypto Suite */
	wpabuf_put_u8(msg, type);
	return msg;
}

const u8 * dpp_get_config_object(const u8 *buf, size_t len, u16 count, u16 *ret_len)
{
	u16 id, alen;
	const u8 *pos = buf, *end = buf + len;
	int local_count = 1;

	while (end - pos >= 4) {
		id = WPA_GET_LE16(pos);
		pos += 2;
		alen = WPA_GET_LE16(pos);
		pos += 2;
		if (alen > end - pos)
			return NULL;
		if (id == DPP_ATTR_CONFIG_OBJ) {
			if (count == local_count) {
				*ret_len = alen;
				return pos;
			} else
				local_count++;
		}
		pos += alen;
	}

	return NULL;
}

int dpp_get_config_objects_count(const u8 *buf, size_t len)
{
	const u8 *pos, *end;
	int count = 0;

	pos = buf;
	end = buf + len;
	while (end - pos >= 4) {
		u16 id, alen;

		id = WPA_GET_LE16(pos);
		pos += 2;
		alen = WPA_GET_LE16(pos);
		pos += 2;
		wpa_printf(MSG_MSGDUMP, "DPP: Attribute ID %04x len %u",
				id, alen);
		if (id == DPP_ATTR_CONFIG_OBJ)
			count++;
		pos += alen;
	}

	return count;
}

const u8 * dpp_get_attr(const u8 *buf, size_t len, u16 req_id, u16 *ret_len)
{
	u16 id, alen;
	const u8 *pos = buf, *end = buf + len;

	while (end - pos >= 4) {
		id = WPA_GET_LE16(pos);
		pos += 2;
		alen = WPA_GET_LE16(pos);
		pos += 2;
		if (alen > end - pos)
			return NULL;
		if (id == req_id) {
			*ret_len = alen;
			return pos;
		}
		pos += alen;
	}

	return NULL;
}


int dpp_check_attrs(const u8 *buf, size_t len)
{
	const u8 *pos, *end;
	int wrapped_data = 0;

	pos = buf;
	end = buf + len;
	while (end - pos >= 4) {
		u16 id, alen;

		id = WPA_GET_LE16(pos);
		pos += 2;
		alen = WPA_GET_LE16(pos);
		pos += 2;
		wpa_printf(MSG_MSGDUMP, "DPP: Attribute ID %04x len %u",
			   id, alen);
		if (alen > end - pos) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Truncated message - not enough room for the attribute - dropped");
			return -1;
		}
		if (wrapped_data) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: An unexpected attribute included after the Wrapped Data attribute");
			return -1;
		}
		if (id == DPP_ATTR_WRAPPED_DATA)
			wrapped_data = 1;
		pos += alen;
	}

	if (end != pos) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Unexpected octets (%d) after the last attribute",
			   (int) (end - pos));
		return -1;
	}

	return 0;
}


void dpp_bootstrap_info_free(struct dpp_bootstrap_info *info)
{
	if (!info)
		return;
	os_free(info->uri);
	os_free(info->info);
	EVP_PKEY_free(info->pubkey);
	os_free(info);
}


const char * dpp_bootstrap_type_txt(enum dpp_bootstrap_type type)
{
	switch (type) {
	case DPP_BOOTSTRAP_QR_CODE:
		return "QRCODE";
	case DPP_BOOTSTRAP_PKEX:
		return "PKEX";
	}
	return "??";
}


static int dpp_uri_valid_info(const char *info)
{
	while (*info) {
		unsigned char val = *info++;

		if (val < 0x20 || val > 0x7e || val == 0x3b)
			return 0;
	}

	return 1;
}


static int dpp_clone_uri(struct dpp_bootstrap_info *bi, const char *uri)
{
	bi->uri = os_strdup(uri);
	return bi->uri ? 0 : -1;
}


int dpp_parse_uri_chan_list(struct dpp_bootstrap_info *bi,
			    const char *chan_list
#ifdef MAP_R3
				, struct dpp_global *dpp
#endif
)
{
	const char *pos = chan_list,*pos2;
	int opclass = 0, channel = 0;

	while (pos && *pos && *pos != ';') {
		pos2 = pos;
                while (*pos2 >= '0' && *pos2 <= '9')
                        pos2++;
                if (*pos2 == '/') {
                        opclass = atoi(pos);
                        pos = pos2 + 1;
                }
                if (opclass <= 0)
                        goto fail;
		channel = atoi(pos);
#ifdef MAP_R3
		if (!wapp_dpp_ch_validation((struct wifi_app *)dpp->msg_ctx, channel)) {
			user_fail_reason *info_to_mapd = os_zalloc(sizeof(user_fail_reason));
			u8 event[64] = {0};
			if (info_to_mapd == NULL)
				return -1;
			DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX" [%s] \n", __func__);
			info_to_mapd->reason_id = CH_NOT_VALID;
			os_snprintf((char *)event, sizeof(event), "Channel %d is not operable", channel);
			os_memcpy(info_to_mapd->reason, event, os_strlen((const char *)event));
			wapp_send_1905_msg(
					dpp->msg_ctx,
					WAPP_SEND_USER_FAIL_NOTIF,
					sizeof(user_fail_reason),
					(char *)info_to_mapd);
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" %s [%s] \n", info_to_mapd->reason, __func__);
			os_free(info_to_mapd);
			while (*pos >= '0' && *pos <= '9')
				pos++;
			if (*pos == ';' || *pos == '\0')
				break;
			if (*pos != ',')
				goto fail;
			pos++;
			continue;
		}
#endif
		if (channel <= 0)
			goto fail;
		while (*pos >= '0' && *pos <= '9')
			pos++;
		if (bi->num_chan == DPP_BOOTSTRAP_MAX_FREQ) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Too many channels in URI channel-list - ignore list");
			bi->num_chan = 0;
			break;
		} else {
			bi->chan[bi->num_chan++] = channel;
		}

		if (*pos == ';' || *pos == '\0')
			break;
		if (*pos != ',')
			goto fail;
		pos++;
	}

	return 0;
fail:
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Invalid URI channel-list");
	return -1;
}


int dpp_parse_uri_mac(struct dpp_bootstrap_info *bi, const char *mac)
{
	if (!mac)
		return 0;

	if (hwaddr_aton2(mac, bi->mac_addr) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Invalid URI mac");
		return -1;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: URI mac: " MACSTR, MAC2STR(bi->mac_addr));

	return 0;
}


int dpp_parse_uri_info(struct dpp_bootstrap_info *bi, const char *info)
{
	const char *end;

	if (!info)
		return 0;

	end = os_strchr(info, ';');
	if (!end)
		end = info + os_strlen(info);
	bi->info = os_malloc(end - info + 1);
	if (!bi->info)
		return -1;
	os_memcpy(bi->info, info, end - info);
	bi->info[end - info] = '\0';
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: URI(information): %s", bi->info);
	if (!dpp_uri_valid_info(bi->info)) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Invalid URI information payload");
		return -1;
	}

	return 0;
}


static const struct dpp_curve_params *
dpp_get_curve_oid(const ASN1_OBJECT *poid)
{
	ASN1_OBJECT *oid;
	int i;

	for (i = 0; dpp_curves[i].name; i++) {
		oid = OBJ_txt2obj(dpp_curves[i].name, 0);
		if (oid && OBJ_cmp(poid, oid) == 0)
			return &dpp_curves[i];
	}
	return NULL;
}


static const struct dpp_curve_params * dpp_get_curve_nid(int nid)
{
	int i, tmp;

	if (!nid)
		return NULL;
	for (i = 0; dpp_curves[i].name; i++) {
		tmp = OBJ_txt2nid(dpp_curves[i].name);
		if (tmp == nid)
			return &dpp_curves[i];
	}
	return NULL;
}

int dpp_parse_uri_version(struct dpp_bootstrap_info *bi, const char *version)
{
#ifdef CONFIG_DPP2
	if (!version || DPP_VERSION < 2) {
#ifdef MAP_R3
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Invalid URI version not present");
		return -1;
#else
		return 0;
#endif
	}

	if (*version == '1')
		bi->version = 1;
	else if (*version == '2')
		bi->version = 2;
	else
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unknown URI version");

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: URI version: %d", bi->version);
#endif /* CONFIG_DPP2 */

	return 0;
}

static int dpp_parse_uri_pk(struct dpp_bootstrap_info *bi, const char *info)
{
	const char *end;
	u8 *data;
	size_t data_len;
	EVP_PKEY *pkey;
	const unsigned char *p;
	int res;
	X509_PUBKEY *pub = NULL;
	ASN1_OBJECT *ppkalg;
	const unsigned char *pk;
	int ppklen;
	X509_ALGOR *pa;
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
	ASN1_OBJECT *pa_oid;
#else
	const ASN1_OBJECT *pa_oid;
#endif
	const void *pval;
	int ptype;
	const ASN1_OBJECT *poid;
	char buf[100];

	end = os_strchr(info, ';');
	if (!end)
		return -1;

	data = base64_decode(info, end - info,
			     &data_len);
	if (!data) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Invalid base64 encoding on URI public-key");
		return -1;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Base64 decoded URI public-key",
		    data, data_len);

	if (sha256_vector(1, (const u8 **) &data, &data_len,
			  bi->pubkey_hash) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to hash public key");
		os_free(data);
		return -1;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Public key hash",
		    bi->pubkey_hash, SHA256_MAC_LEN);

	/* DER encoded ASN.1 SubjectPublicKeyInfo
	 *
	 * SubjectPublicKeyInfo  ::=  SEQUENCE  {
	 *      algorithm            AlgorithmIdentifier,
	 *      subjectPublicKey     BIT STRING  }
	 *
	 * AlgorithmIdentifier  ::=  SEQUENCE  {
	 *      algorithm               OBJECT IDENTIFIER,
	 *      parameters              ANY DEFINED BY algorithm OPTIONAL  }
	 *
	 * subjectPublicKey = compressed format public key per ANSI X9.63
	 * algorithm = ecPublicKey (1.2.840.10045.2.1)
	 * parameters = shall be present and shall be OBJECT IDENTIFIER; e.g.,
	 *       prime256v1 (1.2.840.10045.3.1.7)
	 */

	p = data;
	pkey = d2i_PUBKEY(NULL, &p, data_len);
	os_free(data);

	if (!pkey) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Could not parse URI public-key SubjectPublicKeyInfo");
		return -1;
	}

	if (EVP_PKEY_type(EVP_PKEY_id(pkey)) != EVP_PKEY_EC) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: SubjectPublicKeyInfo does not describe an EC key");
		EVP_PKEY_free(pkey);
		return -1;
	}

	res = X509_PUBKEY_set(&pub, pkey);
	if (res != 1) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Could not set pubkey");
		goto fail;
	}

	res = X509_PUBKEY_get0_param(&ppkalg, &pk, &ppklen, &pa, pub);
	if (res != 1) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Could not extract SubjectPublicKeyInfo parameters");
		goto fail;
	}
	res = OBJ_obj2txt(buf, sizeof(buf), ppkalg, 0);
	if (res < 0 || (size_t) res >= sizeof(buf)) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Could not extract SubjectPublicKeyInfo algorithm");
		goto fail;
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: URI subjectPublicKey algorithm: %s", buf);
	if (os_strcmp(buf, "id-ecPublicKey") != 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Unsupported SubjectPublicKeyInfo algorithm");
		goto fail;
	}

	X509_ALGOR_get0(&pa_oid, &ptype, (void *) &pval, pa);
	if (ptype != V_ASN1_OBJECT) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: SubjectPublicKeyInfo parameters did not contain an OID");
		goto fail;
	}
	poid = pval;
	res = OBJ_obj2txt(buf, sizeof(buf), poid, 0);
	if (res < 0 || (size_t) res >= sizeof(buf)) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Could not extract SubjectPublicKeyInfo parameters OID");
		goto fail;
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: URI subjectPublicKey parameters: %s", buf);
	bi->curve = dpp_get_curve_oid(poid);
	if (!bi->curve) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Unsupported SubjectPublicKeyInfo curve: %s",
			   buf);
		goto fail;
	}

	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: URI subjectPublicKey", pk, ppklen);

	X509_PUBKEY_free(pub);
	bi->pubkey = pkey;
#ifdef DPP_R2_SUPPORT
	dpp_chirp_key_hash(bi);
#endif /* DPP_R2_SUPPORT */
	return 0;
fail:
	X509_PUBKEY_free(pub);
	EVP_PKEY_free(pkey);
	return -1;
}


static struct dpp_bootstrap_info * dpp_parse_uri(const char *uri
#ifdef MAP_R3
	, struct dpp_global *dpp
#endif
	)
{
	const char *pos = uri;
	const char *end;
	const char *chan_list = NULL, *mac = NULL, *info = NULL, *pk = NULL;
	const char *version = NULL;
	struct dpp_bootstrap_info *bi;

	wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: URI", uri, os_strlen(uri));

	if (os_strncmp(pos, "DPP:", 4) != 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Not a DPP URI");
		return NULL;
	}
	pos += 4;

	for (;;) {
		end = os_strchr(pos, ';');
		if (!end)
			break;

		if (end == pos) {
			/* Handle terminating ";;" and ignore unexpected ";"
			 * for parsing robustness. */
			pos++;
			continue;
		}

		if (pos[0] == 'C' && pos[1] == ':' && !chan_list)
			chan_list = pos + 2;
		else if (pos[0] == 'M' && pos[1] == ':' && !mac)
			mac = pos + 2;
		else if (pos[0] == 'I' && pos[1] == ':' && !info)
			info = pos + 2;
		else if (pos[0] == 'K' && pos[1] == ':' && !pk)
			pk = pos + 2;
		else if (pos[0] == 'V' && pos[1] == ':' && !version)
			version = pos + 2;
		else
			wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX
					  "DPP: Ignore unrecognized URI parameter",
					  pos, end - pos);
		pos = end + 1;
	}

	if (!pk) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: URI missing public-key");
		return NULL;
	}

	bi = os_zalloc(sizeof(*bi));
	if (!bi)
		return NULL;

	/*set the default version to 1, in case of no version in URI*/
	bi->version = 1;

	if (dpp_clone_uri(bi, uri) < 0 ||
	    dpp_parse_uri_chan_list(bi, chan_list
#ifdef MAP_R3
		, dpp
#endif
		) < 0 ||
	    dpp_parse_uri_mac(bi, mac) < 0 ||
	    dpp_parse_uri_info(bi, info) < 0 ||
	    dpp_parse_uri_version(bi, version) < 0 ||
	    dpp_parse_uri_pk(bi, pk) < 0) {
		dpp_bootstrap_info_free(bi);
		bi = NULL;
	}

	return bi;
}


struct dpp_bootstrap_info * dpp_parse_qr_code(const char *uri
#ifdef MAP_R3
		, struct dpp_global *dpp
#endif
		)
{
	struct dpp_bootstrap_info *bi;

	bi = dpp_parse_uri(uri
#ifdef MAP_R3
			, dpp
#endif
		);
	if (bi)
		bi->type = DPP_BOOTSTRAP_QR_CODE;
	return bi;
}

#if defined(DPP_AUTOTEST) || defined(MAP_R3)
static void dpp_gen_privkey_from_pubkey(EVP_PKEY *key, unsigned char **privtemp)
{
	EC_KEY *eckey;
	BIO *out;
	size_t rlen;
	char *txt;
	int res;
	unsigned char *der = NULL;
	int der_len;

	out = BIO_new(BIO_s_mem());
	if (!out)
		return;

	EVP_PKEY_print_private(out, key, 0, NULL);
	rlen = BIO_ctrl_pending(out);
	txt = os_malloc(rlen + 1);
	if (txt) {
		res = BIO_read(out, txt, rlen);
		if (res > 0) {
			txt[res] = '\0';
		}
		os_free(txt);
	}
	BIO_free(out);

	eckey = EVP_PKEY_get1_EC_KEY(key);
	if (!eckey)
		return;

	der_len = i2d_ECPrivateKey(eckey, &der);
	if (der_len > 0) {
		*privtemp = os_zalloc(der_len * 2 + 1);
		if(*privtemp) {
			hex_str(der, der_len, *privtemp);
		}
	}
	OPENSSL_free(der);

	EC_KEY_free(eckey);
}
#endif /* DPP_AUTOTEST || MAP_R3 */

void dpp_debug_print_key(const char *title, EVP_PKEY *key)
{
	EC_KEY *eckey;
	BIO *out;
	size_t rlen;
	char *txt;
	int res;
	unsigned char *der = NULL;
	int der_len;
	const EC_GROUP *group;
	const EC_POINT *point;

	out = BIO_new(BIO_s_mem());
	if (!out)
		return;

	EVP_PKEY_print_private(out, key, 0, NULL);
	rlen = BIO_ctrl_pending(out);
	txt = os_malloc(rlen + 1);
	if (txt) {
		res = BIO_read(out, txt, rlen);
		if (res > 0) {
			txt[res] = '\0';
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX "%s: %s", title, txt);
		}
		os_free(txt);
	}
	BIO_free(out);

	eckey = EVP_PKEY_get1_EC_KEY(key);
	if (!eckey)
		return;

	group = EC_KEY_get0_group(eckey);
	point = EC_KEY_get0_public_key(eckey);
	if (group && point)
		dpp_debug_print_point(title, group, point);

	der_len = i2d_ECPrivateKey(eckey, &der);
	if (der_len > 0)
		wpa_hexdump_key(MSG_INFO1, DPP_MAP_PREX "DPP: ECPrivateKey", der, der_len);
	OPENSSL_free(der);
	if (der_len <= 0) {
		der = NULL;
		der_len = i2d_EC_PUBKEY(eckey, &der);
		if (der_len > 0)
			wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: EC_PUBKEY", der, der_len);
		OPENSSL_free(der);
	}

	EC_KEY_free(eckey);
}


EVP_PKEY * dpp_gen_keypair(const struct dpp_curve_params *curve)
{
	EVP_PKEY_CTX *kctx = NULL;
	EC_KEY *ec_params;
	EVP_PKEY *params = NULL, *key = NULL;
	int nid;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Generating a keypair");

	nid = OBJ_txt2nid(curve->name);
	if (nid == NID_undef) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Unsupported curve %s", curve->name);
		return NULL;
	}

	ec_params = EC_KEY_new_by_curve_name(nid);
	if (!ec_params) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to generate EC_KEY parameters");
		goto fail;
	}
	EC_KEY_set_asn1_flag(ec_params, OPENSSL_EC_NAMED_CURVE);
	params = EVP_PKEY_new();
	if (!params || EVP_PKEY_set1_EC_KEY(params, ec_params) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to generate EVP_PKEY parameters");
		goto fail;
	}

	kctx = EVP_PKEY_CTX_new(params, NULL);
	if (!kctx ||
	    EVP_PKEY_keygen_init(kctx) != 1 ||
	    EVP_PKEY_keygen(kctx, &key) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to generate EC key");
		goto fail;
	}

	dpp_debug_print_key("Own generated key: ", key);

	EVP_PKEY_free(params);
	EVP_PKEY_CTX_free(kctx);
	return key;
fail:
	EVP_PKEY_CTX_free(kctx);
	EVP_PKEY_free(params);
	return NULL;
}


static const struct dpp_curve_params *
dpp_get_curve_name(const char *name)
{
	int i;

	for (i = 0; dpp_curves[i].name; i++) {
		if (os_strcmp(name, dpp_curves[i].name) == 0 ||
		    (dpp_curves[i].jwk_crv &&
		     os_strcmp(name, dpp_curves[i].jwk_crv) == 0))
			return &dpp_curves[i];
	}
	return NULL;
}


static const struct dpp_curve_params *
dpp_get_curve_jwk_crv(const char *name)
{
	int i;

	for (i = 0; dpp_curves[i].name; i++) {
		if (dpp_curves[i].jwk_crv &&
		    os_strcmp(name, dpp_curves[i].jwk_crv) == 0)
			return &dpp_curves[i];
	}
	return NULL;
}


EVP_PKEY * dpp_set_keypair(const struct dpp_curve_params **curve,
				  const u8 *privkey, size_t privkey_len)
{
	EVP_PKEY *pkey;
	EC_KEY *eckey;
	const EC_GROUP *group;
	int nid;

	pkey = EVP_PKEY_new();
	if (!pkey)
		return NULL;
	eckey = d2i_ECPrivateKey(NULL, &privkey, privkey_len);
	if (!eckey) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: OpenSSL: d2i_ECPrivateKey() failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		EVP_PKEY_free(pkey);
		return NULL;
	}
	group = EC_KEY_get0_group(eckey);
	if (!group) {
		EC_KEY_free(eckey);
		EVP_PKEY_free(pkey);
		return NULL;
	}
	nid = EC_GROUP_get_curve_name(group);
	*curve = dpp_get_curve_nid(nid);
	if (!*curve) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Unsupported curve (nid=%d) in pre-assigned key",
			   nid);
		EC_KEY_free(eckey);
		EVP_PKEY_free(pkey);
		return NULL;
	}

	if (EVP_PKEY_assign_EC_KEY(pkey, eckey) != 1) {
		EC_KEY_free(eckey);
		EVP_PKEY_free(pkey);
		return NULL;
	}
	return pkey;
}


typedef struct {
	/* AlgorithmIdentifier ecPublicKey with optional parameters present
	 * as an OID identifying the curve */
	X509_ALGOR *alg;
	/* Compressed format public key per ANSI X9.63 */
	ASN1_BIT_STRING *pub_key;
} DPP_BOOTSTRAPPING_KEY;

ASN1_SEQUENCE(DPP_BOOTSTRAPPING_KEY) = {
	ASN1_SIMPLE(DPP_BOOTSTRAPPING_KEY, alg, X509_ALGOR),
	ASN1_SIMPLE(DPP_BOOTSTRAPPING_KEY, pub_key, ASN1_BIT_STRING)
} ASN1_SEQUENCE_END(DPP_BOOTSTRAPPING_KEY);

IMPLEMENT_ASN1_FUNCTIONS(DPP_BOOTSTRAPPING_KEY);


struct wpabuf * dpp_bootstrap_key_der(EVP_PKEY *key)
{
	unsigned char *der = NULL;
	int der_len;
	EC_KEY *eckey;
	struct wpabuf *ret = NULL;
	size_t len;
	const EC_GROUP *group;
	const EC_POINT *point;
	BN_CTX *ctx;
	DPP_BOOTSTRAPPING_KEY *bootstrap = NULL;
	int nid;

	ctx = BN_CTX_new();
	eckey = EVP_PKEY_get1_EC_KEY(key);
	if (!ctx || !eckey)
		goto fail;

	group = EC_KEY_get0_group(eckey);
	point = EC_KEY_get0_public_key(eckey);
	if (!group || !point)
		goto fail;
	dpp_debug_print_point("DPP: bootstrap public key", group, point);
	nid = EC_GROUP_get_curve_name(group);

	bootstrap = DPP_BOOTSTRAPPING_KEY_new();
	if (!bootstrap ||
	    X509_ALGOR_set0(bootstrap->alg, OBJ_nid2obj(EVP_PKEY_EC),
			    V_ASN1_OBJECT, (void *) OBJ_nid2obj(nid)) != 1)
		goto fail;

	len = EC_POINT_point2oct(group, point, POINT_CONVERSION_COMPRESSED,
				 NULL, 0, ctx);
	if (len == 0)
		goto fail;

	der = OPENSSL_malloc(len);
	if (!der)
		goto fail;
	len = EC_POINT_point2oct(group, point, POINT_CONVERSION_COMPRESSED,
				 der, len, ctx);

	OPENSSL_free(bootstrap->pub_key->data);
	bootstrap->pub_key->data = der;
	der = NULL;
	bootstrap->pub_key->length = len;
	/* No unused bits */
	bootstrap->pub_key->flags &= ~(ASN1_STRING_FLAG_BITS_LEFT | 0x07);
	bootstrap->pub_key->flags |= ASN1_STRING_FLAG_BITS_LEFT;

	der_len = i2d_DPP_BOOTSTRAPPING_KEY(bootstrap, &der);
	if (der_len <= 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DDP: Failed to build DER encoded public key");
		goto fail;
	}

	ret = wpabuf_alloc_copy(der, der_len);
fail:
	DPP_BOOTSTRAPPING_KEY_free(bootstrap);
	OPENSSL_free(der);
	EC_KEY_free(eckey);
	BN_CTX_free(ctx);
	return ret;
}


int dpp_bootstrap_key_hash(struct dpp_bootstrap_info *bi)
{
	struct wpabuf *der;
	int res;
	const u8 *addr[1];
	size_t len[1];

	der = dpp_bootstrap_key_der(bi->pubkey);
	if (!der)
		return -1;
	wpa_hexdump_buf(MSG_DEBUG, "DPP: Compressed public key (DER)",
			der);

	addr[0] = wpabuf_head(der);
	len[0] = wpabuf_len(der);
	res = sha256_vector(1, addr, len, bi->pubkey_hash);
	if (res < 0)
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to hash public key");
	else
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Public key hash", bi->pubkey_hash,
			    SHA256_MAC_LEN);
	wpabuf_free(der);
	return res;
}


char * dpp_keygen(struct dpp_bootstrap_info *bi, const char *curve,
		  const u8 *privkey, size_t privkey_len)
{
	char *base64 = NULL;
	char *pos, *end;
	size_t len;
	struct wpabuf *der = NULL;
	const u8 *addr[1];
	int res;

	if (!curve) {
		bi->curve = &dpp_curves[0];
	} else {
		bi->curve = dpp_get_curve_name(curve);
		if (!bi->curve) {
			wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Unsupported curve: %s",
				   curve);
			return NULL;
		}
	}
	if (privkey)
		bi->pubkey = dpp_set_keypair(&bi->curve, privkey, privkey_len);
	else
		bi->pubkey = dpp_gen_keypair(bi->curve);
	if (!bi->pubkey)
		goto fail;
	bi->own = 1;

	der = dpp_bootstrap_key_der(bi->pubkey);
	if (!der)
		goto fail;
	wpa_hexdump_buf(MSG_DEBUG, "DPP: Compressed public key (DER)",
			der);

	addr[0] = wpabuf_head(der);
	len = wpabuf_len(der);
	res = sha256_vector(1, addr, &len, bi->pubkey_hash);
	if (res < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to hash public key");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Public key hash", bi->pubkey_hash,
		    SHA256_MAC_LEN);

	base64 = base64_encode(wpabuf_head(der), wpabuf_len(der), &len);
	wpabuf_free(der);
	der = NULL;
	if (!base64)
		goto fail;
	pos = (char *) base64;
	end = pos + len;
	for (;;) {
		pos = os_strchr(pos, '\n');
		if (!pos)
			break;
		os_memmove(pos, pos + 1, end - pos);
	}
#ifdef DPP_R2_SUPPORT
	dpp_chirp_key_hash(bi);
#endif /* DPP_R2_SUPPORT */
	return (char *) base64;
fail:
	os_free(base64);
	wpabuf_free(der);
	return NULL;
}


static int dpp_derive_k1(const u8 *Mx, size_t Mx_len, u8 *k1,
			 unsigned int hash_len)
{
	u8 salt[DPP_MAX_HASH_LEN], prk[DPP_MAX_HASH_LEN];
	const char *info = "first intermediate key";
	int res;

	/* k1 = HKDF(<>, "first intermediate key", M.x) */

	/* HKDF-Extract(<>, M.x) */
	os_memset(salt, 0, hash_len);
	if (dpp_hmac(hash_len, salt, hash_len, Mx, Mx_len, prk) < 0)
		return -1;
	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: PRK = HKDF-Extract(<>, IKM=M.x)",
			prk, hash_len);

	/* HKDF-Expand(PRK, info, L) */
	res = dpp_hkdf_expand(hash_len, prk, hash_len, info, k1, hash_len);
	os_memset(prk, 0, hash_len);
	if (res < 0)
		return -1;

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: k1 = HKDF-Expand(PRK, info, L)",
			k1, hash_len);
	return 0;
}


static int dpp_derive_k2(const u8 *Nx, size_t Nx_len, u8 *k2,
			 unsigned int hash_len)
{
	u8 salt[DPP_MAX_HASH_LEN], prk[DPP_MAX_HASH_LEN];
	const char *info = "second intermediate key";
	int res;

	/* k2 = HKDF(<>, "second intermediate key", N.x) */

	/* HKDF-Extract(<>, N.x) */
	os_memset(salt, 0, hash_len);
	res = dpp_hmac(hash_len, salt, hash_len, Nx, Nx_len, prk);
	if (res < 0)
		return -1;
	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: PRK = HKDF-Extract(<>, IKM=N.x)",
			prk, hash_len);

	/* HKDF-Expand(PRK, info, L) */
	res = dpp_hkdf_expand(hash_len, prk, hash_len, info, k2, hash_len);
	os_memset(prk, 0, hash_len);
	if (res < 0)
		return -1;

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: k2 = HKDF-Expand(PRK, info, L)",
			k2, hash_len);
	return 0;
}


static int dpp_derive_ke(struct dpp_authentication *auth, u8 *ke,
			 unsigned int hash_len)
{
	size_t nonce_len;
	u8 nonces[2 * DPP_MAX_NONCE_LEN];
	const char *info_ke = "DPP Key";
	u8 prk[DPP_MAX_HASH_LEN];
	int res;
	const u8 *addr[3];
	size_t len[3];
	size_t num_elem = 0;

	if (!auth->Mx_len || !auth->Nx_len) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Mx/Nx not available - cannot derive ke");
		return -1;
	}

	/* ke = HKDF(I-nonce | R-nonce, "DPP Key", M.x | N.x [| L.x]) */

	/* HKDF-Extract(I-nonce | R-nonce, M.x | N.x [| L.x]) */
	nonce_len = auth->curve->nonce_len;
	os_memcpy(nonces, auth->i_nonce, nonce_len);
	os_memcpy(&nonces[nonce_len], auth->r_nonce, nonce_len);
	addr[num_elem] = auth->Mx;
	len[num_elem] = auth->Mx_len;
	num_elem++;
	addr[num_elem] = auth->Nx;
	len[num_elem] = auth->Nx_len;
	num_elem++;
	if (auth->peer_bi && auth->own_bi) {
		if (!auth->Lx_len) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Lx not available - cannot derive ke");
			return -1;
		}
		addr[num_elem] = auth->Lx;
		len[num_elem] = auth->secret_len;
		num_elem++;
	}
	res = dpp_hmac_vector(hash_len, nonces, 2 * nonce_len,
			      num_elem, addr, len, prk);
	if (res < 0)
		return -1;
	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: PRK = HKDF-Extract(<>, IKM)",
			prk, hash_len);

	/* HKDF-Expand(PRK, info, L) */
	res = dpp_hkdf_expand(hash_len, prk, hash_len, info_ke, ke, hash_len);
	os_memset(prk, 0, hash_len);
	if (res < 0)
		return -1;

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: ke = HKDF-Expand(PRK, info, L)",
			ke, hash_len);
	return 0;
}


void dpp_build_attr_status(struct wpabuf *msg,
				  enum dpp_status_error status)
{
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Status %d", status);
	wpabuf_put_le16(msg, DPP_ATTR_STATUS);
	wpabuf_put_le16(msg, 1);
	wpabuf_put_u8(msg, status);
}


static void dpp_build_attr_r_bootstrap_key_hash(struct wpabuf *msg,
						const u8 *hash)
{
	if (hash) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: R-Bootstrap Key Hash");
		wpabuf_put_le16(msg, DPP_ATTR_R_BOOTSTRAP_KEY_HASH);
		wpabuf_put_le16(msg, SHA256_MAC_LEN);
		wpabuf_put_data(msg, hash, SHA256_MAC_LEN);
	}
}


static void dpp_build_attr_i_bootstrap_key_hash(struct wpabuf *msg,
						const u8 *hash)
{
	if (hash) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: I-Bootstrap Key Hash");
		wpabuf_put_le16(msg, DPP_ATTR_I_BOOTSTRAP_KEY_HASH);
		wpabuf_put_le16(msg, SHA256_MAC_LEN);
		wpabuf_put_data(msg, hash, SHA256_MAC_LEN);
	}
}


static struct wpabuf * dpp_auth_build_req(struct dpp_authentication *auth,
					  const struct wpabuf *pi,
					  size_t nonce_len,
					  const u8 *r_pubkey_hash,
					  const u8 *i_pubkey_hash,
					  unsigned int neg_chan)
{
	struct wpabuf *msg;
	u8 clear[4 + DPP_MAX_NONCE_LEN + 4 + 1];
	u8 wrapped_data[4 + DPP_MAX_NONCE_LEN + 4 + 1 + AES_BLOCK_SIZE];
	u8 *pos;
	const u8 *addr[2];
	size_t len[2], siv_len, attr_len;
	u8 *attr_start, *attr_end;

	/* Build DPP Authentication Request frame attributes */
	attr_len = 2 * (4 + SHA256_MAC_LEN) + 4 + (pi ? wpabuf_len(pi) : 0) +
		4 + sizeof(wrapped_data);
	if (neg_chan > 0)
		attr_len += 4 + 2;
#ifdef CONFIG_DPP2
	attr_len += 5;
#endif /* CONFIG_DPP2 */

	msg = dpp_alloc_msg(DPP_PA_AUTHENTICATION_REQ, attr_len);
	if (!msg)
		return NULL;

	attr_start = wpabuf_put(msg, 0);

	/* Responder Bootstrapping Key Hash */
	dpp_build_attr_r_bootstrap_key_hash(msg, r_pubkey_hash);

	/* Initiator Bootstrapping Key Hash */
	dpp_build_attr_i_bootstrap_key_hash(msg, i_pubkey_hash);

	/* Initiator Protocol Key */
	if (pi) {
		wpabuf_put_le16(msg, DPP_ATTR_I_PROTOCOL_KEY);
		wpabuf_put_le16(msg, wpabuf_len(pi));
		wpabuf_put_buf(msg, pi);
	}

	/* Channel */
	if (neg_chan > 0) {
		u8 op_class = 0; //TODO kapil add this
		wpabuf_put_le16(msg, DPP_ATTR_CHANNEL);
		wpabuf_put_le16(msg, 2);
		wpabuf_put_u8(msg, op_class);
		wpabuf_put_u8(msg, neg_chan);
	}

#ifdef CONFIG_DPP2
	/* Protocol Version */
	wpabuf_put_le16(msg, DPP_ATTR_PROTOCOL_VERSION);
	wpabuf_put_le16(msg, 1);
	wpabuf_put_u8(msg, DPP_VERSION);
#endif /* CONFIG_DPP2 */

	/* Wrapped data ({I-nonce, I-capabilities}k1) */
	pos = clear;

	/* I-nonce */
	WPA_PUT_LE16(pos, DPP_ATTR_I_NONCE);
	pos += 2;
	WPA_PUT_LE16(pos, nonce_len);
	pos += 2;
	os_memcpy(pos, auth->i_nonce, nonce_len);
	pos += nonce_len;

	/* I-capabilities */
	WPA_PUT_LE16(pos, DPP_ATTR_I_CAPABILITIES);
	pos += 2;
	WPA_PUT_LE16(pos, 1);
	pos += 2;
	auth->i_capab = auth->allowed_roles;
	*pos++ = auth->i_capab;

	attr_end = wpabuf_put(msg, 0);

	/* OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = wpabuf_head_u8(msg) + 2;
	len[0] = 3 + 1 + 1 + 1;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);

	/* Attributes before Wrapped Data */
	addr[1] = attr_start;
	len[1] = attr_end - attr_start;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);

	siv_len = pos - clear;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext", clear, siv_len);
	if (aes_siv_encrypt(auth->k1, auth->curve->hash_len, clear, siv_len,
			    2, addr, len, wrapped_data) < 0) {
		wpabuf_free(msg);
		return NULL;
	}
	siv_len += AES_BLOCK_SIZE;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped_data, siv_len);

	wpabuf_put_le16(msg, DPP_ATTR_WRAPPED_DATA);
	wpabuf_put_le16(msg, siv_len);
	wpabuf_put_data(msg, wrapped_data, siv_len);

	wpa_hexdump_buf(MSG_DEBUG,
			"DPP: Authentication Request frame attributes", msg);

	return msg;
}


static struct wpabuf * dpp_auth_build_resp(struct dpp_authentication *auth,
					   enum dpp_status_error status,
					   const struct wpabuf *pr,
					   size_t nonce_len,
					   const u8 *r_pubkey_hash,
					   const u8 *i_pubkey_hash,
					   const u8 *r_nonce, const u8 *i_nonce,
					   const u8 *wrapped_r_auth,
					   size_t wrapped_r_auth_len,
					   const u8 *siv_key)
{
	struct wpabuf *msg;
#define DPP_AUTH_RESP_CLEAR_LEN 2 * (4 + DPP_MAX_NONCE_LEN) + 4 + 1 + \
		4 + 4 + DPP_MAX_HASH_LEN + AES_BLOCK_SIZE
	u8 clear[DPP_AUTH_RESP_CLEAR_LEN];
	u8 wrapped_data[DPP_AUTH_RESP_CLEAR_LEN + AES_BLOCK_SIZE];
	const u8 *addr[2];
	size_t len[2], siv_len, attr_len;
	u8 *attr_start, *attr_end, *pos;

	auth->current_state = DPP_STATE_AUTH_CONF_WAITING;
	auth->auth_resp_tries = 0;

	/* Build DPP Authentication Response frame attributes */
	attr_len = 4 + 1 + 2 * (4 + SHA256_MAC_LEN) +
		4 + (pr ? wpabuf_len(pr) : 0) + 4 + sizeof(wrapped_data);
#ifdef CONFIG_DPP2
	attr_len += 5;
#endif /* CONFIG_DPP2 */
	msg = dpp_alloc_msg(DPP_PA_AUTHENTICATION_RESP, attr_len);
	if (!msg)
		return NULL;

	attr_start = wpabuf_put(msg, 0);

	/* DPP Status */
	if (status != 255)
		dpp_build_attr_status(msg, status);

	/* Responder Bootstrapping Key Hash */
	dpp_build_attr_r_bootstrap_key_hash(msg, r_pubkey_hash);

	/* Initiator Bootstrapping Key Hash (mutual authentication) */
	dpp_build_attr_i_bootstrap_key_hash(msg, i_pubkey_hash);

	/* Responder Protocol Key */
	if (pr) {
		wpabuf_put_le16(msg, DPP_ATTR_R_PROTOCOL_KEY);
		wpabuf_put_le16(msg, wpabuf_len(pr));
		wpabuf_put_buf(msg, pr);
	}

#ifdef CONFIG_DPP2
	/* Protocol Version */
	if (auth->peer_version >= 2) {
		wpabuf_put_le16(msg, DPP_ATTR_PROTOCOL_VERSION);
		wpabuf_put_le16(msg, 1);
		wpabuf_put_u8(msg, DPP_VERSION);
	}
#endif /* CONFIG_DPP2 */

	attr_end = wpabuf_put(msg, 0);

	/* Wrapped data ({R-nonce, I-nonce, R-capabilities, {R-auth}ke}k2) */
	pos = clear;

	if (r_nonce) {
		/* R-nonce */
		WPA_PUT_LE16(pos, DPP_ATTR_R_NONCE);
		pos += 2;
		WPA_PUT_LE16(pos, nonce_len);
		pos += 2;
		os_memcpy(pos, r_nonce, nonce_len);
		pos += nonce_len;
	}

	if (i_nonce) {
		/* I-nonce */
		WPA_PUT_LE16(pos, DPP_ATTR_I_NONCE);
		pos += 2;
		WPA_PUT_LE16(pos, nonce_len);
		pos += 2;
		os_memcpy(pos, i_nonce, nonce_len);
		pos += nonce_len;
	}

	/* R-capabilities */
	WPA_PUT_LE16(pos, DPP_ATTR_R_CAPABILITIES);
	pos += 2;
	WPA_PUT_LE16(pos, 1);
	pos += 2;
	auth->r_capab = auth->configurator ? DPP_CAPAB_CONFIGURATOR :
		DPP_CAPAB_ENROLLEE;
	*pos++ = auth->r_capab;

	if (wrapped_r_auth) {
		/* {R-auth}ke */
		WPA_PUT_LE16(pos, DPP_ATTR_WRAPPED_DATA);
		pos += 2;
		WPA_PUT_LE16(pos, wrapped_r_auth_len);
		pos += 2;
		os_memcpy(pos, wrapped_r_auth, wrapped_r_auth_len);
		pos += wrapped_r_auth_len;
	}

	/* OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = wpabuf_head_u8(msg) + 2;
	len[0] = 3 + 1 + 1 + 1;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);

	/* Attributes before Wrapped Data */
	addr[1] = attr_start;
	len[1] = attr_end - attr_start;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);

	siv_len = pos - clear;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext", clear, siv_len);
	if (aes_siv_encrypt(siv_key, auth->curve->hash_len, clear, siv_len,
			    2, addr, len, wrapped_data) < 0) {
		wpabuf_free(msg);
		return NULL;
	}
	siv_len += AES_BLOCK_SIZE;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped_data, siv_len);

	wpabuf_put_le16(msg, DPP_ATTR_WRAPPED_DATA);
	wpabuf_put_le16(msg, siv_len);
	wpabuf_put_data(msg, wrapped_data, siv_len);

	wpa_hexdump_buf(MSG_DEBUG,
			"DPP: Authentication Response frame attributes", msg);
	return msg;
}

static int chan_included(const unsigned int chans[], unsigned int num,
			 unsigned int chan)
{
	while (num > 0) {
		if (chans[--num] == chan)
			return 1;
	}
	return 0;
}

static int dpp_channel_intersect(struct dpp_authentication *auth)
{
	struct dpp_bootstrap_info *peer_bi = auth->peer_bi;
	unsigned int i, chan;
	struct wifi_app *wapp = auth->msg_ctx;

	if (!wapp) {
		DBGPRINT(RT_DEBUG_ERROR,DPP_MAP_PREX"failed to find wapp, auth not initialized\n");
		return -1;
	}
	for (i = 0; i < peer_bi->num_chan; i++) {
		chan = peer_bi->chan[i];
		if (chan_included(auth->chan, auth->num_chan, chan))
			continue;
	}
	if (!auth->num_chan) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: No available channels for initiating DPP Authentication");
		return -1;
	}
	auth->curr_chan = auth->chan[0];
	return 0;
}

unsigned int channel_list_2g[] = {1, 6, 11};
unsigned int channel_list_5gh[] = {149, 153, 157, 161};
unsigned int channel_list_5gl[] = {36, 40, 44, 48};
unsigned int channel_list_5g[] = {36, 40, 44, 48, 149, 153, 157, 161};

static int dpp_channel_local_list(struct dpp_authentication *auth)
{
	struct wifi_app *wapp = auth->msg_ctx;
	int i, ch_len = 0;
	unsigned int *chanlist = NULL;

	auth->num_chan = 0;

	if (!wapp) {
		DBGPRINT(RT_DEBUG_ERROR,DPP_MAP_PREX"failed to find wapp, auth not initialized\n");
		return -1;
	}

	if (!auth->wdev) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: DPP interface is not initialized");
		return -1;
	}

	if (IS_MAP_CH_5GL(auth->wdev->radio->op_ch))
		chanlist = channel_list_5gl;
	else if (IS_MAP_CH_5GH(auth->wdev->radio->op_ch))
		chanlist = channel_list_5gh;
	else if (IS_MAP_CH_24G(auth->wdev->radio->op_ch))
		chanlist = channel_list_2g;
	if (wapp->radio_count == 2 && IS_MAP_CH_5G(auth->wdev->radio->op_ch))
		chanlist = channel_list_5g;

	if (!chanlist) {
		wpa_printf(MSG_INFO,
			   "DPP: something is wrong in this code");
		return -1;
	}
	ch_len = sizeof(*chanlist)/sizeof(chanlist[0]);
	for (i = 0; i < ch_len; i++) {
		auth->chan[i] = chanlist[i];
		auth->num_chan++;
	}

	return (auth->num_chan == 0) ? -1 : 0;
}


int dpp_prepare_channel_list(struct dpp_authentication *auth)
{
	int res;
	char chans[DPP_BOOTSTRAP_MAX_FREQ * 6 + 10], *pos, *end;
	unsigned int i;

	if (auth->peer_bi->num_chan > 0)
		res = dpp_channel_intersect(auth);
	else
		res = dpp_channel_local_list(auth);
	if (res < 0)
		return res;

	auth->chan_idx = 0;
	auth->curr_chan = auth->chan[0];
	pos = chans;
	end = pos + sizeof(chans);
	for (i = 0; i < auth->num_chan; i++) {
		res = os_snprintf(pos, end - pos, " %u", auth->chan[i]);
		if (os_snprintf_error(end - pos, res))
			break;
		pos += res;
	}
	*pos = '\0';
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Possible channels for initiating:%s",
		   chans);
	return 0;
}


static int dpp_autogen_bootstrap_key(struct dpp_authentication *auth)
{
	struct dpp_bootstrap_info *bi;
	char *pk = NULL;
	size_t len;

	if (auth->own_bi)
		return 0; /* already generated */

	bi = os_zalloc(sizeof(*bi));
	if (!bi)
		return -1;
	bi->type = DPP_BOOTSTRAP_QR_CODE;
	pk = dpp_keygen(bi, auth->peer_bi->curve->name, NULL, 0);
	if (!pk)
		goto fail;

	len = 4; /* "DPP:" */
#ifdef CONFIG_DPP2
	len += 4; /* V:2; */
#endif
	len += 4 + os_strlen(pk);
	bi->uri = os_malloc(len + 1);
	if (!bi->uri)
		goto fail;
	os_snprintf(bi->uri, len + 1, "DPP:%sK:%s;;", DPP_VERSION == 2 ? "V:2;" : "", pk);
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		   "DPP: Auto-generated own bootstrapping key info: URI %s",
		   bi->uri);

	auth->tmp_own_bi = auth->own_bi = bi;

	os_free(pk);

	return 0;
fail:
	os_free(pk);
	dpp_bootstrap_info_free(bi);
	return -1;
}

#ifdef MAP_R3
struct dpp_authentication * dpp_auth_init_relay(void *msg_ctx,
					  unsigned char *almac,
					  unsigned char *sta_addr,
					  unsigned char *chirp_buf,
					  unsigned char *auth_buf,
					  int len, int rem_buf_len)
{
	struct dpp_authentication *auth = NULL;
	u8 old_auth = 0;
	struct wifi_app *wapp = (struct wifi_app *)msg_ctx;
	u8 dpp_allowed_roles = DPP_CAPAB_PROXYAGENT;
	unsigned char *chirp_tmp_buf = chirp_buf;
	unsigned short chirp_len = 0;

#if 0	
	if (chirp_tmp_buf != NULL) {
			if (chirp_tmp_buf[0] == DPP_CHIRP_TLV) {
				chirp_tmp_buf += 2; //points to length
				//hash_len = *chirp_tmp_buf;				
			}
	}
#endif 
	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"sta_mac.."MACSTR"\n",MAC2STR(sta_addr));
	auth = wapp_dpp_get_auth_from_peer_mac(wapp, sta_addr);
	if (auth) {
			old_auth = 1;
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"got the auth instrance \n");
	} else {
		auth = os_zalloc(sizeof(*auth));
		if (!auth)
			return NULL;
		auth->msg_ctx = msg_ctx;
		auth->initiator = 1;
		auth->current_state = DPP_STATE_AUTH_RESP_WAITING;
		auth->allowed_roles = dpp_allowed_roles;
		auth->configurator = 0;
		os_memcpy(auth->peer_mac_addr, sta_addr,
			  MAC_ADDR_LEN);
		os_memcpy(auth->relay_mac_addr, almac,
			  MAC_ADDR_LEN);
		auth->is_map_connection = 1;
		auth->is_wired = 0;
		auth->auth_req_ack = 1;

		if(!auth->msg_out)
			wpabuf_free(auth->msg_out);

		auth->msg_out_pos = 0;
		auth->msg_out = wpabuf_alloc(len + 1);
		if (!auth->msg_out)
	                {
                          dpp_auth_deinit(auth);
		          return NULL;
                         }
		wpabuf_put_u8(auth->msg_out, WLAN_ACTION_PUBLIC);
		wpabuf_put_data(auth->msg_out, auth_buf, len);
		//auth->wdev = wdev; //wdev will be updated after presence announce frame.
	}


	if(!old_auth && wapp_dpp_auth_list_insert(wapp, auth) < 0) {
		//dpp_auth_deinit(auth);
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"auth_insert_failed..\n");
		goto fail;
	}

	//Store Chirp TLV
	chirp_tmp_buf = chirp_buf;
	if (chirp_tmp_buf != NULL) {
		if (chirp_tmp_buf[0] == DPP_CHIRP_TLV) {
			chirp_tmp_buf++; //points to length
			chirp_len = *(unsigned short *)chirp_tmp_buf;
			chirp_len = be2cpu16(chirp_len);
			if (rem_buf_len < chirp_len) {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s:buffer length %d less than TLV length %d for proxy agent\n"
					, __func__, rem_buf_len, chirp_len);
				return NULL;
			}

			chirp_tmp_buf += 2; //Starts TLV
			rem_buf_len = rem_buf_len - 2;
			auth->chirp_tlv.enrollee_mac_address_present = (*chirp_tmp_buf & 0x80);
			auth->chirp_tlv.hash_validity = (*chirp_tmp_buf & 0x40);
			chirp_tmp_buf++;
			rem_buf_len = rem_buf_len - 1;
			if(auth->chirp_tlv.enrollee_mac_address_present){
				os_memcpy(auth->chirp_tlv.enrollee_mac, chirp_tmp_buf, MAC_ADDR_LEN);
				chirp_tmp_buf += 6;  //Enrolle MAC address
				rem_buf_len = rem_buf_len - 6;
			}


			auth->chirp_tlv.hash_len = *chirp_tmp_buf;
			if (rem_buf_len < auth->chirp_tlv.hash_len) {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s:Buffer length %d less than hash length %d for proxy agent\n"
					, __func__, rem_buf_len, auth->chirp_tlv.hash_len);
				return NULL;
			}

			chirp_tmp_buf++; //hash_payload
			os_memcpy(auth->chirp_tlv.hash_payload, chirp_tmp_buf, auth->chirp_tlv.hash_len); //hash_payload
			hex_dump_dbg(DPP_MAP_PREX"chirp frame ",(u8 *)auth->chirp_tlv.hash_payload , auth->chirp_tlv.hash_len);
		}

	} 

	return auth;

fail:
	dpp_auth_deinit(auth);
	auth = NULL;
	return auth;

}
#endif /* MAP_R3 */

struct dpp_authentication * dpp_auth_init(void *msg_ctx,
					  struct wapp_dev *wdev,
					  struct dpp_bootstrap_info *peer_bi,
					  struct dpp_bootstrap_info *own_bi,
					  u8 dpp_allowed_roles,
					  unsigned int neg_chan)
{
	struct dpp_authentication *auth;
	size_t nonce_len;
	EVP_PKEY_CTX *ctx = NULL;
	size_t secret_len;
	struct wpabuf *pi = NULL;
	const u8 *r_pubkey_hash, *i_pubkey_hash;
	struct wifi_app *wapp = NULL;
	auth = os_zalloc(sizeof(*auth));
	if (!auth)
		return NULL;
	auth->msg_ctx = msg_ctx;
	auth->initiator = 1;
	auth->current_state = DPP_STATE_AUTH_RESP_WAITING;
	auth->allowed_roles = dpp_allowed_roles;
	auth->configurator = !!(dpp_allowed_roles & DPP_CAPAB_CONFIGURATOR);
	auth->peer_bi = peer_bi;
	auth->own_bi = own_bi;
	auth->curve = peer_bi->curve;
	auth->wdev = wdev;
	auth->own_version = 1;
	wapp = (struct wifi_app *)msg_ctx;
	if(wapp)
		auth->own_version = wapp->dpp->version_ctrl;
	auth->peer_version = peer_bi->version;
#ifdef CONFIG_DPP2
	auth->send_conn_status = 0;
	auth->waiting_conn_status_result = 0;
	auth->conn_status_requested = 0;
#endif /* CONFIG_DPP2 */
#ifdef MAP_R3
	auth->bss_conf = 0;
#endif  /* MAP_R3 */

	if(wapp && wapp->dpp->is_map){
		if(dpp_autogen_bootstrap_key(auth) < 0)
			goto fail;
	} else {
		if (dpp_autogen_bootstrap_key(auth) < 0 ||
		    dpp_prepare_channel_list(auth) < 0)
			goto fail;
	}

	nonce_len = auth->curve->nonce_len;
	if (random_get_bytes(auth->i_nonce, nonce_len)) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to generate I-nonce");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: I-nonce", auth->i_nonce, nonce_len);

	auth->own_protocol_key = dpp_gen_keypair(auth->curve);
	if (!auth->own_protocol_key)
		goto fail;

	pi = dpp_get_pubkey_point(auth->own_protocol_key, 0);
	if (!pi)
		goto fail;

	/* ECDH: M = pI * BR */
	ctx = EVP_PKEY_CTX_new(auth->own_protocol_key, NULL);
	if (!ctx ||
	    EVP_PKEY_derive_init(ctx) != 1 ||
	    EVP_PKEY_derive_set_peer(ctx, auth->peer_bi->pubkey) != 1 ||
	    EVP_PKEY_derive(ctx, NULL, &secret_len) != 1 ||
	    secret_len > DPP_MAX_SHARED_SECRET_LEN ||
	    EVP_PKEY_derive(ctx, auth->Mx, &secret_len) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to derive ECDH shared secret: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	auth->secret_len = secret_len;
	EVP_PKEY_CTX_free(ctx);
	ctx = NULL;

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: ECDH shared secret (M.x)",
			auth->Mx, auth->secret_len);
	auth->Mx_len = auth->secret_len;

	if (dpp_derive_k1(auth->Mx, auth->secret_len, auth->k1,
			  auth->curve->hash_len) < 0)
		goto fail;

	r_pubkey_hash = auth->peer_bi->pubkey_hash;
	i_pubkey_hash = auth->own_bi->pubkey_hash;

	auth->req_msg = dpp_auth_build_req(auth, pi, nonce_len, r_pubkey_hash,
					   i_pubkey_hash, neg_chan);
	if (!auth->req_msg)
		goto fail;
out:
#ifdef MAP_R3
	if (wapp && wapp->map && wapp->map->map_version == DEV_TYPE_R3 && wapp->map->TurnKeyEnable)
			wapp->map->dpp_after_scan = 1;
#endif
	wpabuf_free(pi);
	EVP_PKEY_CTX_free(ctx);
	return auth;
fail:
	dpp_auth_deinit(auth);
	auth = NULL;
	goto out;
}


static struct wpabuf * dpp_build_conf_req_attr(struct dpp_authentication *auth,
					       const char *json)
{
	size_t nonce_len;
	size_t json_len, clear_len;
	struct wpabuf *clear = NULL, *msg = NULL;
	u8 *wrapped;
	size_t attr_len;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Build configuration request");

	nonce_len = auth->curve->nonce_len;
	if (random_get_bytes(auth->e_nonce, nonce_len)) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to generate E-nonce");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: E-nonce", auth->e_nonce, nonce_len);
	json_len = os_strlen(json);
	wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: configAttr JSON", json, json_len);

	/* { E-nonce, configAttrib }ke */
	clear_len = 4 + nonce_len + 4 + json_len;
	clear = wpabuf_alloc(clear_len);
	attr_len = 4 + clear_len + AES_BLOCK_SIZE;
	msg = wpabuf_alloc(attr_len);
	if (!clear || !msg)
		goto fail;

	/* E-nonce */
	wpabuf_put_le16(clear, DPP_ATTR_ENROLLEE_NONCE);
	wpabuf_put_le16(clear, nonce_len);
	wpabuf_put_data(clear, auth->e_nonce, nonce_len);

	/* configAttrib */
	wpabuf_put_le16(clear, DPP_ATTR_CONFIG_ATTR_OBJ);
	wpabuf_put_le16(clear, json_len);
	wpabuf_put_data(clear, json, json_len);

	wpabuf_put_le16(msg, DPP_ATTR_WRAPPED_DATA);
	wpabuf_put_le16(msg, wpabuf_len(clear) + AES_BLOCK_SIZE);
	wrapped = wpabuf_put(msg, wpabuf_len(clear) + AES_BLOCK_SIZE);

	/* No AES-SIV AD */
	wpa_hexdump_buf(MSG_DEBUG, "DPP: AES-SIV cleartext", clear);
	if (aes_siv_encrypt(auth->ke, auth->curve->hash_len,
			    wpabuf_head(clear), wpabuf_len(clear),
			    0, NULL, NULL, wrapped) < 0)
		goto fail;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped, wpabuf_len(clear) + AES_BLOCK_SIZE);

	wpa_hexdump_buf(MSG_DEBUG,
			"DPP: Configuration Request frame attributes", msg);
	wpabuf_free(clear);
	return msg;

fail:
	wpabuf_free(clear);
	wpabuf_free(msg);
	return NULL;
}

void dpp_write_adv_proto(struct wpabuf *buf, u8 build_req)
{
	/* Advertisement Protocol IE */
	wpabuf_put_u8(buf, WLAN_EID_ADV_PROTO);
	wpabuf_put_u8(buf, 8); /* Length */
	if(!build_req)
		wpabuf_put_u8(buf, 0x7f);
	else
		wpabuf_put_u8(buf, 0x00);
	wpabuf_put_u8(buf, WLAN_EID_VENDOR_SPECIFIC);
	wpabuf_put_u8(buf, 5);
	wpabuf_put_be24(buf, OUI_WFA);
	wpabuf_put_u8(buf, DPP_OUI_TYPE);
	wpabuf_put_u8(buf, 0x01);
}

void dpp_write_gas_query(struct wpabuf *buf, struct wpabuf *query)
{
	/* GAS Query */
	wpabuf_put_le16(buf, wpabuf_len(query));
	wpabuf_put_buf(buf, query);
}


struct wpabuf * dpp_build_conf_req(struct dpp_authentication *auth,
				   const char *json)
{
	struct wpabuf *buf, *conf_req;
	u8 build_req = 1;

	conf_req = dpp_build_conf_req_attr(auth, json);
	if (!conf_req) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: No configuration request data available");
		return NULL;
	}

	buf = gas_build_initial_req(0, 10 + 2 + wpabuf_len(conf_req));
	if (!buf) {
		wpabuf_free(conf_req);
		return NULL;
	}

	dpp_write_adv_proto(buf, build_req);
	dpp_write_gas_query(buf, conf_req);
	wpabuf_free(conf_req);
	wpa_hexdump_buf(MSG_MSGDUMP, "DPP: GAS Config Request", buf);

	return buf;
}


void dpp_auth_success(struct dpp_authentication *auth)
{
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		   "DPP: Authentication success - clear temporary keys");
	os_memset(auth->Mx, 0, sizeof(auth->Mx));
	auth->Mx_len = 0;
	os_memset(auth->Nx, 0, sizeof(auth->Nx));
	auth->Nx_len = 0;
	os_memset(auth->Lx, 0, sizeof(auth->Lx));
	auth->Lx_len = 0;
	os_memset(auth->k1, 0, sizeof(auth->k1));
	os_memset(auth->k2, 0, sizeof(auth->k2));

	auth->auth_success = 1;
}


static int dpp_gen_r_auth(struct dpp_authentication *auth, u8 *r_auth)
{
	struct wpabuf *pix, *prx, *bix, *brx;
	const u8 *addr[7];
	size_t len[7];
	size_t i, num_elem = 0;
	size_t nonce_len;
	u8 zero = 0;
	int res = -1;

	/* R-auth = H(I-nonce | R-nonce | PI.x | PR.x | [BI.x |] BR.x | 0) */
	nonce_len = auth->curve->nonce_len;

	if (auth->initiator) {
		pix = dpp_get_pubkey_point(auth->own_protocol_key, 0);
		prx = dpp_get_pubkey_point(auth->peer_protocol_key, 0);
		if (auth->own_bi)
			bix = dpp_get_pubkey_point(auth->own_bi->pubkey, 0);
		else
			bix = NULL;
		brx = dpp_get_pubkey_point(auth->peer_bi->pubkey, 0);
	} else {
		pix = dpp_get_pubkey_point(auth->peer_protocol_key, 0);
		prx = dpp_get_pubkey_point(auth->own_protocol_key, 0);
		if (auth->peer_bi)
			bix = dpp_get_pubkey_point(auth->peer_bi->pubkey, 0);
		else
			bix = NULL;
		brx = dpp_get_pubkey_point(auth->own_bi->pubkey, 0);
	}
	if (!pix || !prx || !brx)
		goto fail;

	addr[num_elem] = auth->i_nonce;
	len[num_elem] = nonce_len;
	num_elem++;

	addr[num_elem] = auth->r_nonce;
	len[num_elem] = nonce_len;
	num_elem++;

	addr[num_elem] = wpabuf_head(pix);
	len[num_elem] = wpabuf_len(pix) / 2;
	num_elem++;

	addr[num_elem] = wpabuf_head(prx);
	len[num_elem] = wpabuf_len(prx) / 2;
	num_elem++;

	if (bix) {
		addr[num_elem] = wpabuf_head(bix);
		len[num_elem] = wpabuf_len(bix) / 2;
		num_elem++;
	}

	addr[num_elem] = wpabuf_head(brx);
	len[num_elem] = wpabuf_len(brx) / 2;
	num_elem++;

	addr[num_elem] = &zero;
	len[num_elem] = 1;
	num_elem++;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: R-auth hash components");
	for (i = 0; i < num_elem; i++)
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: hash component", addr[i], len[i]);
	res = dpp_hash_vector(auth->curve, num_elem, addr, len, r_auth);
	if (res == 0)
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: R-auth", r_auth,
			    auth->curve->hash_len);
fail:
	wpabuf_free(pix);
	wpabuf_free(prx);
	wpabuf_free(bix);
	wpabuf_free(brx);
	return res;
}


static int dpp_gen_i_auth(struct dpp_authentication *auth, u8 *i_auth)
{
	struct wpabuf *pix = NULL, *prx = NULL, *bix = NULL, *brx = NULL;
	const u8 *addr[7];
	size_t len[7];
	size_t i, num_elem = 0;
	size_t nonce_len;
	u8 one = 1;
	int res = -1;

	/* I-auth = H(R-nonce | I-nonce | PR.x | PI.x | BR.x | [BI.x |] 1) */
	nonce_len = auth->curve->nonce_len;

	if (auth->initiator) {
		pix = dpp_get_pubkey_point(auth->own_protocol_key, 0);
		prx = dpp_get_pubkey_point(auth->peer_protocol_key, 0);
		if (auth->own_bi)
			bix = dpp_get_pubkey_point(auth->own_bi->pubkey, 0);
		else
			bix = NULL;
		if (!auth->peer_bi)
			goto fail;
		brx = dpp_get_pubkey_point(auth->peer_bi->pubkey, 0);
	} else {
		pix = dpp_get_pubkey_point(auth->peer_protocol_key, 0);
		prx = dpp_get_pubkey_point(auth->own_protocol_key, 0);
		if (auth->peer_bi)
			bix = dpp_get_pubkey_point(auth->peer_bi->pubkey, 0);
		else
			bix = NULL;
		if (!auth->own_bi)
			goto fail;
		brx = dpp_get_pubkey_point(auth->own_bi->pubkey, 0);
	}
	if (!pix || !prx || !brx)
		goto fail;

	addr[num_elem] = auth->r_nonce;
	len[num_elem] = nonce_len;
	num_elem++;

	addr[num_elem] = auth->i_nonce;
	len[num_elem] = nonce_len;
	num_elem++;

	addr[num_elem] = wpabuf_head(prx);
	len[num_elem] = wpabuf_len(prx) / 2;
	num_elem++;

	addr[num_elem] = wpabuf_head(pix);
	len[num_elem] = wpabuf_len(pix) / 2;
	num_elem++;

	addr[num_elem] = wpabuf_head(brx);
	len[num_elem] = wpabuf_len(brx) / 2;
	num_elem++;

	if (bix) {
		addr[num_elem] = wpabuf_head(bix);
		len[num_elem] = wpabuf_len(bix) / 2;
		num_elem++;
	}

	addr[num_elem] = &one;
	len[num_elem] = 1;
	num_elem++;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: I-auth hash components");
	for (i = 0; i < num_elem; i++)
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: hash component", addr[i], len[i]);
	res = dpp_hash_vector(auth->curve, num_elem, addr, len, i_auth);
	if (res == 0)
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: I-auth", i_auth,
			    auth->curve->hash_len);
fail:
	wpabuf_free(pix);
	wpabuf_free(prx);
	wpabuf_free(bix);
	wpabuf_free(brx);
	return res;
}


static int dpp_auth_derive_l_responder(struct dpp_authentication *auth)
{
	const EC_GROUP *group;
	EC_POINT *l = NULL;
	EC_KEY *BI = NULL, *bR = NULL, *pR = NULL;
	const EC_POINT *BI_point;
	BN_CTX *bnctx;
	BIGNUM *lx, *sum, *q;
	const BIGNUM *bR_bn, *pR_bn;
	int ret = -1;

	/* L = ((bR + pR) modulo q) * BI */

	bnctx = BN_CTX_new();
	sum = BN_new();
	q = BN_new();
	lx = BN_new();
	if (!bnctx || !sum || !q || !lx)
		goto fail;
	BI = EVP_PKEY_get1_EC_KEY(auth->peer_bi->pubkey);
	if (!BI)
		goto fail;
	BI_point = EC_KEY_get0_public_key(BI);
	group = EC_KEY_get0_group(BI);
	if (!group)
		goto fail;

	bR = EVP_PKEY_get1_EC_KEY(auth->own_bi->pubkey);
	pR = EVP_PKEY_get1_EC_KEY(auth->own_protocol_key);
	if (!bR || !pR)
		goto fail;
	bR_bn = EC_KEY_get0_private_key(bR);
	pR_bn = EC_KEY_get0_private_key(pR);
	if (!bR_bn || !pR_bn)
		goto fail;
	if (EC_GROUP_get_order(group, q, bnctx) != 1 ||
	    BN_mod_add(sum, bR_bn, pR_bn, q, bnctx) != 1)
		goto fail;
	l = EC_POINT_new(group);
	if (!l ||
	    EC_POINT_mul(group, l, NULL, BI_point, sum, bnctx) != 1 ||
	    EC_POINT_get_affine_coordinates_GFp(group, l, lx, NULL,
						bnctx) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "OpenSSL: failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	if (dpp_bn2bin_pad(lx, auth->Lx, auth->secret_len) < 0)
		goto fail;
	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: L.x", auth->Lx, auth->secret_len);
	auth->Lx_len = auth->secret_len;
	ret = 0;
fail:
	EC_POINT_clear_free(l);
	EC_KEY_free(BI);
	EC_KEY_free(bR);
	EC_KEY_free(pR);
	BN_clear_free(lx);
	BN_clear_free(sum);
	BN_free(q);
	BN_CTX_free(bnctx);
	return ret;
}


static int dpp_auth_derive_l_initiator(struct dpp_authentication *auth)
{
	const EC_GROUP *group;
	EC_POINT *l = NULL, *sum = NULL;
	EC_KEY *bI = NULL, *BR = NULL, *PR = NULL;
	const EC_POINT *BR_point, *PR_point;
	BN_CTX *bnctx;
	BIGNUM *lx;
	const BIGNUM *bI_bn;
	int ret = -1;

	/* L = bI * (BR + PR) */

	bnctx = BN_CTX_new();
	lx = BN_new();
	if (!bnctx || !lx)
		goto fail;
	BR = EVP_PKEY_get1_EC_KEY(auth->peer_bi->pubkey);
	PR = EVP_PKEY_get1_EC_KEY(auth->peer_protocol_key);
	if (!BR || !PR)
		goto fail;
	BR_point = EC_KEY_get0_public_key(BR);
	PR_point = EC_KEY_get0_public_key(PR);

	bI = EVP_PKEY_get1_EC_KEY(auth->own_bi->pubkey);
	if (!bI)
		goto fail;
	group = EC_KEY_get0_group(bI);
	bI_bn = EC_KEY_get0_private_key(bI);
	if (!group || !bI_bn)
		goto fail;
	sum = EC_POINT_new(group);
	l = EC_POINT_new(group);
	if (!sum || !l ||
	    EC_POINT_add(group, sum, BR_point, PR_point, bnctx) != 1 ||
	    EC_POINT_mul(group, l, NULL, sum, bI_bn, bnctx) != 1 ||
	    EC_POINT_get_affine_coordinates_GFp(group, l, lx, NULL,
						bnctx) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "OpenSSL: failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	if (dpp_bn2bin_pad(lx, auth->Lx, auth->secret_len) < 0)
		goto fail;
	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: L.x", auth->Lx, auth->secret_len);
	auth->Lx_len = auth->secret_len;
	ret = 0;
fail:
	EC_POINT_clear_free(l);
	EC_POINT_clear_free(sum);
	EC_KEY_free(bI);
	EC_KEY_free(BR);
	EC_KEY_free(PR);
	BN_clear_free(lx);
	BN_CTX_free(bnctx);
	return ret;
}


static int dpp_auth_build_resp_ok(struct dpp_authentication *auth)
{
	size_t nonce_len;
	EVP_PKEY_CTX *ctx = NULL;
	size_t secret_len;
	struct wpabuf *msg, *pr = NULL;
	u8 r_auth[4 + DPP_MAX_HASH_LEN];
	u8 wrapped_r_auth[4 + DPP_MAX_HASH_LEN + AES_BLOCK_SIZE], *w_r_auth;
	size_t wrapped_r_auth_len;
	int ret = -1;
	const u8 *r_pubkey_hash, *i_pubkey_hash, *r_nonce, *i_nonce;
	enum dpp_status_error status = DPP_STATUS_OK;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Build Authentication Response");
	if (!auth->own_bi)
		return -1;

	nonce_len = auth->curve->nonce_len;
	if (random_get_bytes(auth->r_nonce, nonce_len)) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to generate R-nonce");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: R-nonce", auth->r_nonce, nonce_len);

	auth->own_protocol_key = dpp_gen_keypair(auth->curve);
	if (!auth->own_protocol_key)
		goto fail;

	pr = dpp_get_pubkey_point(auth->own_protocol_key, 0);
	if (!pr)
		goto fail;

	/* ECDH: N = pR * PI */
	ctx = EVP_PKEY_CTX_new(auth->own_protocol_key, NULL);
	if (!ctx ||
	    EVP_PKEY_derive_init(ctx) != 1 ||
	    EVP_PKEY_derive_set_peer(ctx, auth->peer_protocol_key) != 1 ||
	    EVP_PKEY_derive(ctx, NULL, &secret_len) != 1 ||
	    secret_len > DPP_MAX_SHARED_SECRET_LEN ||
	    EVP_PKEY_derive(ctx, auth->Nx, &secret_len) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to derive ECDH shared secret: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	EVP_PKEY_CTX_free(ctx);
	ctx = NULL;

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: ECDH shared secret (N.x)",
			auth->Nx, auth->secret_len);
	auth->Nx_len = auth->secret_len;

	if (dpp_derive_k2(auth->Nx, auth->secret_len, auth->k2,
			  auth->curve->hash_len) < 0)
		goto fail;

	if (auth->own_bi && auth->peer_bi) {
		/* Mutual authentication */
		if (dpp_auth_derive_l_responder(auth) < 0)
			goto fail;
	}

	if (dpp_derive_ke(auth, auth->ke, auth->curve->hash_len) < 0)
		goto fail;

	/* R-auth = H(I-nonce | R-nonce | PI.x | PR.x | [BI.x |] BR.x | 0) */
	WPA_PUT_LE16(r_auth, DPP_ATTR_R_AUTH_TAG);
	WPA_PUT_LE16(&r_auth[2], auth->curve->hash_len);
	if (dpp_gen_r_auth(auth, r_auth + 4) < 0)
		goto fail;
	if (aes_siv_encrypt(auth->ke, auth->curve->hash_len,
			    r_auth, 4 + auth->curve->hash_len,
			    0, NULL, NULL, wrapped_r_auth) < 0)
		goto fail;
	wrapped_r_auth_len = 4 + auth->curve->hash_len + AES_BLOCK_SIZE;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: {R-auth}ke",
		    wrapped_r_auth, wrapped_r_auth_len);
	w_r_auth = wrapped_r_auth;

	r_pubkey_hash = auth->own_bi->pubkey_hash;
	if (auth->peer_bi)
		i_pubkey_hash = auth->peer_bi->pubkey_hash;
	else
		i_pubkey_hash = NULL;

	i_nonce = auth->i_nonce;
	r_nonce = auth->r_nonce;

	msg = dpp_auth_build_resp(auth, status, pr, nonce_len,
				  r_pubkey_hash, i_pubkey_hash,
				  r_nonce, i_nonce,
				  w_r_auth, wrapped_r_auth_len,
				  auth->k2);
	if (!msg)
		goto fail;
	wpabuf_free(auth->resp_msg);
	auth->resp_msg = msg;
	ret = 0;
fail:
	wpabuf_free(pr);
	return ret;
}


static int dpp_auth_build_resp_status(struct dpp_authentication *auth,
				      enum dpp_status_error status)
{
	struct wpabuf *msg;
	const u8 *r_pubkey_hash, *i_pubkey_hash, *i_nonce;

	if (!auth->own_bi)
		return -1;
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Build Authentication Response");

	r_pubkey_hash = auth->own_bi->pubkey_hash;
	if (auth->peer_bi)
		i_pubkey_hash = auth->peer_bi->pubkey_hash;
	else
		i_pubkey_hash = NULL;

	i_nonce = auth->i_nonce;

	msg = dpp_auth_build_resp(auth, status, NULL, auth->curve->nonce_len,
				  r_pubkey_hash, i_pubkey_hash,
				  NULL, i_nonce, NULL, 0, auth->k1);
	if (!msg)
		return -1;
	wpabuf_free(auth->resp_msg);
	auth->resp_msg = msg;
	return 0;
}


struct dpp_authentication *
dpp_auth_req_rx(void *msg_ctx, u8 dpp_allowed_roles, int qr_mutual,
		struct dpp_bootstrap_info *peer_bi,
		struct dpp_bootstrap_info *own_bi,
		unsigned int chan, const u8 *hdr, const u8 *attr_start,
		size_t attr_len)
{
	EVP_PKEY *pi = NULL;
	EVP_PKEY_CTX *ctx = NULL;
	size_t secret_len;
	const u8 *addr[2];
	size_t len[2];
	u8 *unwrapped = NULL;
	size_t unwrapped_len = 0;
	u8 neg_chan;
	const u8 *wrapped_data, *i_proto, *i_nonce, *i_capab, *i_bootstrap,
		*channel;
	u16 wrapped_data_len, i_proto_len, i_nonce_len, i_capab_len,
		i_bootstrap_len, channel_len;
	struct dpp_authentication *auth = NULL;
#ifdef CONFIG_DPP2
	const u8 *version;
	u16 version_len;
#endif /* CONFIG_DPP2 */
	struct wifi_app *wapp = NULL;
	wrapped_data = dpp_get_attr(attr_start, attr_len, DPP_ATTR_WRAPPED_DATA,
				    &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		return NULL;
	}
	wpa_hexdump(MSG_MSGDUMP, DPP_MAP_PREX "DPP: Wrapped Data",
		    wrapped_data, wrapped_data_len);
	attr_len = wrapped_data - 4 - attr_start;

	auth = os_zalloc(sizeof(*auth));
	if (!auth)
		goto fail;
	auth->msg_ctx = msg_ctx;
	auth->peer_bi = peer_bi;
	auth->own_bi = own_bi;
	auth->curve = own_bi->curve;
	auth->curr_chan = chan;
	auth->own_version = 1;
#ifdef MAP_R3
	auth->bss_conf = 0;
#endif  /* MAP_R3 */
	wapp = (struct wifi_app *) msg_ctx;
	if(wapp)
		auth->own_version = wapp->dpp->version_ctrl;
	auth->peer_version = 1; /* default to the first version */

#ifdef CONFIG_DPP2
	version = dpp_get_attr(attr_start, attr_len, DPP_ATTR_PROTOCOL_VERSION,
			       &version_len);
	if (version && DPP_VERSION > 1) {
		if (version_len < 1 || version[0] == 0) {
			dpp_auth_fail(auth,
				      "Invalid Protocol Version attribute");
			goto fail;
		}
		auth->peer_version = version[0];
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Peer protocol version %u",
			   auth->peer_version);
	}
#endif /* CONFIG_DPP2 */

	channel = dpp_get_attr(attr_start, attr_len, DPP_ATTR_CHANNEL,
			       &channel_len);
	if (channel) {

		if (channel_len < 2) {
			dpp_auth_fail(auth, "Too short Channel attribute");
			goto fail;
		}

		neg_chan = channel[1];
		//TODO do we actually need operating class??
		if (auth->curr_chan != (unsigned int) neg_chan) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Changing negotiation channel from %u to %u",
				   chan, neg_chan);
			auth->curr_chan = neg_chan;
		}
	}

	i_proto = dpp_get_attr(attr_start, attr_len, DPP_ATTR_I_PROTOCOL_KEY,
			       &i_proto_len);
	if (!i_proto) {
		dpp_auth_fail(auth,
			      "Missing required Initiator Protocol Key attribute");
		goto fail;
	}
	wpa_hexdump(MSG_MSGDUMP, DPP_MAP_PREX "DPP: Initiator Protocol Key",
		    i_proto, i_proto_len);

	/* M = bR * PI */
	pi = dpp_set_pubkey_point(own_bi->pubkey, i_proto, i_proto_len);
	if (!pi) {
		dpp_auth_fail(auth, "Invalid Initiator Protocol Key");
		goto fail;
	}
	dpp_debug_print_key("Peer (Initiator) Protocol Key", pi);

	ctx = EVP_PKEY_CTX_new(own_bi->pubkey, NULL);
	if (!ctx ||
	    EVP_PKEY_derive_init(ctx) != 1 ||
	    EVP_PKEY_derive_set_peer(ctx, pi) != 1 ||
	    EVP_PKEY_derive(ctx, NULL, &secret_len) != 1 ||
	    secret_len > DPP_MAX_SHARED_SECRET_LEN ||
	    EVP_PKEY_derive(ctx, auth->Mx, &secret_len) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to derive ECDH shared secret: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		dpp_auth_fail(auth, "Failed to derive ECDH shared secret");
		goto fail;
	}
	auth->secret_len = secret_len;
	EVP_PKEY_CTX_free(ctx);
	ctx = NULL;

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: ECDH shared secret (M.x)",
			auth->Mx, auth->secret_len);
	auth->Mx_len = auth->secret_len;

	if (dpp_derive_k1(auth->Mx, auth->secret_len, auth->k1,
			  auth->curve->hash_len) < 0)
		goto fail;

	addr[0] = hdr;
	len[0] = DPP_HDR_LEN;
	addr[1] = attr_start;
	len[1] = attr_len;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped_data, wrapped_data_len);
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped = os_malloc(unwrapped_len);
	if (!unwrapped)
		goto fail;
	if (aes_siv_decrypt(auth->k1, auth->curve->hash_len,
			    wrapped_data, wrapped_data_len,
			    2, addr, len, unwrapped) < 0) {
		dpp_auth_fail(auth, "AES-SIV decryption failed");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext",
		    unwrapped, unwrapped_len);

	if (dpp_check_attrs(unwrapped, unwrapped_len) < 0) {
		dpp_auth_fail(auth, "Invalid attribute in unwrapped data");
		goto fail;
	}

	i_nonce = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_I_NONCE,
			       &i_nonce_len);
	if (!i_nonce || i_nonce_len != auth->curve->nonce_len) {
		dpp_auth_fail(auth, "Missing or invalid I-nonce");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: I-nonce", i_nonce, i_nonce_len);
	os_memcpy(auth->i_nonce, i_nonce, i_nonce_len);

	i_capab = dpp_get_attr(unwrapped, unwrapped_len,
			       DPP_ATTR_I_CAPABILITIES,
			       &i_capab_len);
	if (!i_capab || i_capab_len < 1) {
		dpp_auth_fail(auth, "Missing or invalid I-capabilities");
		goto fail;
	}
	auth->i_capab = i_capab[0];
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: I-capabilities: 0x%02x", auth->i_capab);

	bin_clear_free(unwrapped, unwrapped_len);
	unwrapped = NULL;

	switch (auth->i_capab & DPP_CAPAB_ROLE_MASK) {
	case DPP_CAPAB_ENROLLEE:
		if (!(dpp_allowed_roles & DPP_CAPAB_CONFIGURATOR)) {
			wpa_printf(MSG_ERROR, DPP_MAP_PREX
				   "DPP: Local policy does not allow Configurator role");
			goto not_compatible;
		}
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Acting as Configurator");
		auth->configurator = 1;
		break;
	case DPP_CAPAB_CONFIGURATOR:
		if (!(dpp_allowed_roles & DPP_CAPAB_ENROLLEE)) {
			wpa_printf(MSG_ERROR, DPP_MAP_PREX
				   "DPP: Local policy does not allow Enrollee role");
			goto not_compatible;
		}
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Acting as Enrollee");
		auth->configurator = 0;
		break;
	case DPP_CAPAB_CONFIGURATOR | DPP_CAPAB_ENROLLEE:
		if (dpp_allowed_roles & DPP_CAPAB_ENROLLEE) {
			wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Acting as Enrollee");
			auth->configurator = 0;
		} else if (dpp_allowed_roles & DPP_CAPAB_CONFIGURATOR) {
			wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Acting as Configurator");
			auth->configurator = 1;
		} else {
			wpa_printf(MSG_ERROR, DPP_MAP_PREX
				   "DPP: Local policy does not allow Configurator/Enrollee role");
			goto not_compatible;
		}
		break;
	default:
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Unexpected role in I-capabilities");
		goto fail;
	}

	auth->peer_protocol_key = pi;
	pi = NULL;
	if (qr_mutual && !peer_bi && own_bi->type == DPP_BOOTSTRAP_QR_CODE) {
		char hex[SHA256_MAC_LEN * 2 + 1];

		wpa_printf(MSG_INFO1, DPP_MAP_PREX
			   "DPP: Mutual authentication required with QR Codes, but peer info is not yet available - request more time");
		if (dpp_auth_build_resp_status(auth,
					       DPP_STATUS_RESPONSE_PENDING) < 0)
			goto fail;
		i_bootstrap = dpp_get_attr(attr_start, attr_len,
					   DPP_ATTR_I_BOOTSTRAP_KEY_HASH,
					   &i_bootstrap_len);
		if (i_bootstrap && i_bootstrap_len == SHA256_MAC_LEN) {
			auth->response_pending = 1;
			os_memcpy(auth->waiting_pubkey_hash,
				  i_bootstrap, i_bootstrap_len);
			os_snprintf_hex(hex, sizeof(hex), i_bootstrap,
					 i_bootstrap_len);
		} else {
			hex[0] = '\0';
		}

		return auth;
	}
	if (dpp_auth_build_resp_ok(auth) < 0)
		goto fail;

	return auth;

not_compatible:
	if (dpp_allowed_roles & DPP_CAPAB_CONFIGURATOR)
		auth->configurator = 1;
	else
		auth->configurator = 0;
	auth->peer_protocol_key = pi;
	pi = NULL;
	if (dpp_auth_build_resp_status(auth, DPP_STATUS_NOT_COMPATIBLE) < 0)
		goto fail;

	auth->remove_on_tx_status = 1;
	return auth;
fail:
	bin_clear_free(unwrapped, unwrapped_len);
	EVP_PKEY_free(pi);
	EVP_PKEY_CTX_free(ctx);
	dpp_auth_deinit(auth);
	return NULL;
}


int dpp_notify_new_qr_code(struct dpp_authentication *auth,
			   struct dpp_bootstrap_info *peer_bi)
{
	if (!auth || !auth->response_pending ||
	    os_memcmp(auth->waiting_pubkey_hash, peer_bi->pubkey_hash,
		      SHA256_MAC_LEN) != 0)
		return 0;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		   "DPP: New scanned QR Code has matching public key that was needed to continue DPP Authentication exchange with "
		   MACSTR, MAC2STR(auth->peer_mac_addr));
	auth->peer_bi = peer_bi;

	if (dpp_auth_build_resp_ok(auth) < 0)
		return -1;

	return 1;
}


static struct wpabuf * dpp_auth_build_conf(struct dpp_authentication *auth,
					   enum dpp_status_error status)
{
	struct wpabuf *msg;
	u8 i_auth[4 + DPP_MAX_HASH_LEN];
	size_t i_auth_len;
	u8 r_nonce[4 + DPP_MAX_NONCE_LEN];
	size_t r_nonce_len;
	const u8 *addr[2];
	size_t len[2], attr_len;
	u8 *wrapped_i_auth;
	u8 *wrapped_r_nonce;
	u8 *attr_start, *attr_end;
	const u8 *r_pubkey_hash, *i_pubkey_hash;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Build Authentication Confirmation");

	i_auth_len = 4 + auth->curve->hash_len;
	r_nonce_len = 4 + auth->curve->nonce_len;
	/* Build DPP Authentication Confirmation frame attributes */
	attr_len = 4 + 1 + 2 * (4 + SHA256_MAC_LEN) +
		4 + i_auth_len + r_nonce_len + AES_BLOCK_SIZE;
	msg = dpp_alloc_msg(DPP_PA_AUTHENTICATION_CONF, attr_len);
	if (!msg)
		goto fail;

	attr_start = wpabuf_put(msg, 0);

	r_pubkey_hash = auth->peer_bi->pubkey_hash;
	if (auth->own_bi)
		i_pubkey_hash = auth->own_bi->pubkey_hash;
	else
		i_pubkey_hash = NULL;

	/* DPP Status */
	dpp_build_attr_status(msg, status);

	/* Responder Bootstrapping Key Hash */
	dpp_build_attr_r_bootstrap_key_hash(msg, r_pubkey_hash);

	/* Initiator Bootstrapping Key Hash (mutual authentication) */
	dpp_build_attr_i_bootstrap_key_hash(msg, i_pubkey_hash);

	attr_end = wpabuf_put(msg, 0);

	/* OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = wpabuf_head_u8(msg) + 2;
	len[0] = 3 + 1 + 1 + 1;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);

	/* Attributes before Wrapped Data */
	addr[1] = attr_start;
	len[1] = attr_end - attr_start;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);

	if (status == DPP_STATUS_OK) {
		/* I-auth wrapped with ke */
		wpabuf_put_le16(msg, DPP_ATTR_WRAPPED_DATA);
		wpabuf_put_le16(msg, i_auth_len + AES_BLOCK_SIZE);
		wrapped_i_auth = wpabuf_put(msg, i_auth_len + AES_BLOCK_SIZE);

		/* I-auth = H(R-nonce | I-nonce | PR.x | PI.x | BR.x | [BI.x |]
		 *	      1) */
		WPA_PUT_LE16(i_auth, DPP_ATTR_I_AUTH_TAG);
		WPA_PUT_LE16(&i_auth[2], auth->curve->hash_len);
		if (dpp_gen_i_auth(auth, i_auth + 4) < 0)
			goto fail;

		if (aes_siv_encrypt(auth->ke, auth->curve->hash_len,
				    i_auth, i_auth_len,
				    2, addr, len, wrapped_i_auth) < 0)
			goto fail;
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: {I-auth}ke",
			    wrapped_i_auth, i_auth_len + AES_BLOCK_SIZE);
	} else {
		/* R-nonce wrapped with k2 */
		wpabuf_put_le16(msg, DPP_ATTR_WRAPPED_DATA);
		wpabuf_put_le16(msg, r_nonce_len + AES_BLOCK_SIZE);
		wrapped_r_nonce = wpabuf_put(msg, r_nonce_len + AES_BLOCK_SIZE);

		WPA_PUT_LE16(r_nonce, DPP_ATTR_R_NONCE);
		WPA_PUT_LE16(&r_nonce[2], auth->curve->nonce_len);
		os_memcpy(r_nonce + 4, auth->r_nonce, auth->curve->nonce_len);

		if (aes_siv_encrypt(auth->k2, auth->curve->hash_len,
				    r_nonce, r_nonce_len,
				    2, addr, len, wrapped_r_nonce) < 0)
			goto fail;
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: {R-nonce}k2",
			    wrapped_r_nonce, r_nonce_len + AES_BLOCK_SIZE);
	}

	wpa_hexdump_buf(MSG_DEBUG,
			"DPP: Authentication Confirmation frame attributes",
			msg);
	if (status == DPP_STATUS_OK)
		dpp_auth_success(auth);

	return msg;

fail:
	wpabuf_free(msg);
	return NULL;
}


static void
dpp_auth_resp_rx_status(struct dpp_authentication *auth, const u8 *hdr,
			const u8 *attr_start, size_t attr_len,
			const u8 *wrapped_data, u16 wrapped_data_len,
			enum dpp_status_error status)
{
	const u8 *addr[2];
	size_t len[2];
	u8 *unwrapped = NULL;
	size_t unwrapped_len = 0;
	const u8 *i_nonce, *r_capab;
	u16 i_nonce_len, r_capab_len;

	if (status == DPP_STATUS_NOT_COMPATIBLE) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Responder reported incompatible roles");
	} else if (status == DPP_STATUS_RESPONSE_PENDING) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Responder reported more time needed");
	} else {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Responder reported failure (status %d)",
			   status);
		dpp_auth_fail(auth, "Responder reported failure");
		return;
	}

	addr[0] = hdr;
	len[0] = DPP_HDR_LEN;
	addr[1] = attr_start;
	len[1] = attr_len;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped_data, wrapped_data_len);
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped = os_malloc(unwrapped_len);
	if (!unwrapped)
		goto fail;
	if (aes_siv_decrypt(auth->k1, auth->curve->hash_len,
			    wrapped_data, wrapped_data_len,
			    2, addr, len, unwrapped) < 0) {
		dpp_auth_fail(auth, "AES-SIV decryption failed");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext",
		    unwrapped, unwrapped_len);

	if (dpp_check_attrs(unwrapped, unwrapped_len) < 0) {
		dpp_auth_fail(auth, "Invalid attribute in unwrapped data");
		goto fail;
	}

	i_nonce = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_I_NONCE,
			       &i_nonce_len);
	if (!i_nonce || i_nonce_len != auth->curve->nonce_len) {
		dpp_auth_fail(auth, "Missing or invalid I-nonce");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: I-nonce", i_nonce, i_nonce_len);
	if (os_memcmp(auth->i_nonce, i_nonce, i_nonce_len) != 0) {
		dpp_auth_fail(auth, "I-nonce mismatch");
		goto fail;
	}

	r_capab = dpp_get_attr(unwrapped, unwrapped_len,
			       DPP_ATTR_R_CAPABILITIES,
			       &r_capab_len);
	if (!r_capab || r_capab_len < 1) {
		dpp_auth_fail(auth, "Missing or invalid R-capabilities");
		goto fail;
	}
	auth->r_capab = r_capab[0];
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: R-capabilities: 0x%02x", auth->r_capab);
	if (status == DPP_STATUS_NOT_COMPATIBLE) {
	} else if (status == DPP_STATUS_RESPONSE_PENDING) {
		u8 role = auth->r_capab & DPP_CAPAB_ROLE_MASK;

		if ((auth->configurator && role != DPP_CAPAB_ENROLLEE) ||
		    (!auth->configurator && role != DPP_CAPAB_CONFIGURATOR)) {
		} else {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Continue waiting for full DPP Authentication Response");
		}
	}
fail:
	bin_clear_free(unwrapped, unwrapped_len);
}


struct wpabuf *
dpp_auth_resp_rx(struct dpp_authentication *auth, const u8 *hdr,
		 const u8 *attr_start, size_t attr_len)
{
	EVP_PKEY *pr;
	EVP_PKEY_CTX *ctx = NULL;
	size_t secret_len;
	const u8 *addr[2];
	size_t len[2];
	u8 *unwrapped = NULL, *unwrapped2 = NULL;
	size_t unwrapped_len = 0, unwrapped2_len = 0;
	const u8 *r_bootstrap, *i_bootstrap, *wrapped_data, *status, *r_proto,
		*r_nonce, *i_nonce, *r_capab, *wrapped2, *r_auth;
	u16 r_bootstrap_len, i_bootstrap_len, wrapped_data_len, status_len,
		r_proto_len, r_nonce_len, i_nonce_len, r_capab_len,
		wrapped2_len, r_auth_len;
	u8 r_auth2[DPP_MAX_HASH_LEN];
	u8 role;
#ifdef CONFIG_DPP2
	const u8 *version;
	u16 version_len;
#endif /* CONFIG_DPP2 */

	if (!auth->initiator || !auth->peer_bi) {
		dpp_auth_fail(auth, "Unexpected Authentication Response");
		return NULL;
	}

	wrapped_data = dpp_get_attr(attr_start, attr_len, DPP_ATTR_WRAPPED_DATA,
				    &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		dpp_auth_fail(auth,
			      "Missing or invalid required Wrapped Data attribute");
		return NULL;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Wrapped data",
		    wrapped_data, wrapped_data_len);

	attr_len = wrapped_data - 4 - attr_start;

	r_bootstrap = dpp_get_attr(attr_start, attr_len,
				   DPP_ATTR_R_BOOTSTRAP_KEY_HASH,
				   &r_bootstrap_len);
	if (!r_bootstrap || r_bootstrap_len != SHA256_MAC_LEN) {
		dpp_auth_fail(auth,
			      "Missing or invalid required Responder Bootstrapping Key Hash attribute");
		return NULL;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Responder Bootstrapping Key Hash",
		    r_bootstrap, r_bootstrap_len);
	if (os_memcmp(r_bootstrap, auth->peer_bi->pubkey_hash,
		      SHA256_MAC_LEN) != 0) {
		dpp_auth_fail(auth,
			      "Unexpected Responder Bootstrapping Key Hash value");
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX
			    "DPP: Expected Responder Bootstrapping Key Hash",
			    auth->peer_bi->pubkey_hash, SHA256_MAC_LEN);
		return NULL;
	}

	i_bootstrap = dpp_get_attr(attr_start, attr_len,
				   DPP_ATTR_I_BOOTSTRAP_KEY_HASH,
				   &i_bootstrap_len);
	if (i_bootstrap) {
		if (i_bootstrap_len != SHA256_MAC_LEN) {
			dpp_auth_fail(auth,
				      "Invalid Initiator Bootstrapping Key Hash attribute");
			return NULL;
		}
		wpa_hexdump(MSG_MSGDUMP, DPP_MAP_PREX
			    "DPP: Initiator Bootstrapping Key Hash",
			    i_bootstrap, i_bootstrap_len);
		if (!auth->own_bi ||
		    os_memcmp(i_bootstrap, auth->own_bi->pubkey_hash,
			      SHA256_MAC_LEN) != 0) {
			dpp_auth_fail(auth,
				      "Initiator Bootstrapping Key Hash attribute did not match");
			return NULL;
		}
	} else if (auth->own_bi && auth->own_bi->type == DPP_BOOTSTRAP_PKEX) {
		/* PKEX bootstrapping mandates use of mutual authentication */
		dpp_auth_fail(auth,
			      "Missing Initiator Bootstrapping Key Hash attribute");
		return NULL;
	}

	auth->peer_version = 1; /* default to the first version */


#ifdef CONFIG_DPP2
	version = dpp_get_attr(attr_start, attr_len, DPP_ATTR_PROTOCOL_VERSION,
			       &version_len);
	if (version && DPP_VERSION > 1) {
		if (version_len < 1 || version[0] == 0) {
			dpp_auth_fail(auth,
				      "Invalid Protocol Version attribute");
			return NULL;
		}
		auth->peer_version = version[0];
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Peer protocol version %u",
			   auth->peer_version);
	}
#endif /* CONFIG_DPP2 */

	status = dpp_get_attr(attr_start, attr_len, DPP_ATTR_STATUS,
			      &status_len);
	if (!status || status_len < 1) {
		dpp_auth_fail(auth,
			      "Missing or invalid required DPP Status attribute");
		return NULL;
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Status %u", status[0]);
	auth->auth_resp_status = status[0];
	if (status[0] != DPP_STATUS_OK) {
		dpp_auth_resp_rx_status(auth, hdr, attr_start,
					attr_len, wrapped_data,
					wrapped_data_len, status[0]);
		return NULL;
	}

	if (!i_bootstrap && auth->own_bi) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Responder decided not to use mutual authentication");
		auth->own_bi = NULL;
	}

	r_proto = dpp_get_attr(attr_start, attr_len, DPP_ATTR_R_PROTOCOL_KEY,
			       &r_proto_len);
	if (!r_proto) {
		dpp_auth_fail(auth,
			      "Missing required Responder Protocol Key attribute");
		return NULL;
	}
	wpa_hexdump(MSG_MSGDUMP, DPP_MAP_PREX "DPP: Responder Protocol Key",
		    r_proto, r_proto_len);

	/* N = pI * PR */
	pr = dpp_set_pubkey_point(auth->own_protocol_key, r_proto, r_proto_len);
	if (!pr) {
		dpp_auth_fail(auth, "Invalid Responder Protocol Key");
		return NULL;
	}
	dpp_debug_print_key("Peer (Responder) Protocol Key", pr);

	ctx = EVP_PKEY_CTX_new(auth->own_protocol_key, NULL);
	if (!ctx ||
	    EVP_PKEY_derive_init(ctx) != 1 ||
	    EVP_PKEY_derive_set_peer(ctx, pr) != 1 ||
	    EVP_PKEY_derive(ctx, NULL, &secret_len) != 1 ||
	    secret_len > DPP_MAX_SHARED_SECRET_LEN ||
	    EVP_PKEY_derive(ctx, auth->Nx, &secret_len) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to derive ECDH shared secret: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		dpp_auth_fail(auth, "Failed to derive ECDH shared secret");
		goto fail;
	}
	EVP_PKEY_CTX_free(ctx);
	ctx = NULL;
	auth->peer_protocol_key = pr;
	pr = NULL;

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: ECDH shared secret (N.x)",
			auth->Nx, auth->secret_len);
	auth->Nx_len = auth->secret_len;

	if (dpp_derive_k2(auth->Nx, auth->secret_len, auth->k2,
			  auth->curve->hash_len) < 0)
		goto fail;

	addr[0] = hdr;
	len[0] = DPP_HDR_LEN;
	addr[1] = attr_start;
	len[1] = attr_len;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped_data, wrapped_data_len);
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped = os_malloc(unwrapped_len);
	if (!unwrapped)
		goto fail;
	if (aes_siv_decrypt(auth->k2, auth->curve->hash_len,
			    wrapped_data, wrapped_data_len,
			    2, addr, len, unwrapped) < 0) {
		dpp_auth_fail(auth, "AES-SIV decryption failed");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext",
		    unwrapped, unwrapped_len);

	if (dpp_check_attrs(unwrapped, unwrapped_len) < 0) {
		dpp_auth_fail(auth, "Invalid attribute in unwrapped data");
		goto fail;
	}

	r_nonce = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_R_NONCE,
			       &r_nonce_len);
	if (!r_nonce || r_nonce_len != auth->curve->nonce_len) {
		dpp_auth_fail(auth, "DPP: Missing or invalid R-nonce");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: R-nonce", r_nonce, r_nonce_len);
	os_memcpy(auth->r_nonce, r_nonce, r_nonce_len);

	i_nonce = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_I_NONCE,
			       &i_nonce_len);
	if (!i_nonce || i_nonce_len != auth->curve->nonce_len) {
		dpp_auth_fail(auth, "Missing or invalid I-nonce");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: I-nonce", i_nonce, i_nonce_len);
	if (os_memcmp(auth->i_nonce, i_nonce, i_nonce_len) != 0) {
		dpp_auth_fail(auth, "I-nonce mismatch");
		goto fail;
	}

	if (auth->own_bi) {
		/* Mutual authentication */
		if (dpp_auth_derive_l_initiator(auth) < 0)
			goto fail;
	}

	r_capab = dpp_get_attr(unwrapped, unwrapped_len,
			       DPP_ATTR_R_CAPABILITIES,
			       &r_capab_len);
	if (!r_capab || r_capab_len < 1) {
		dpp_auth_fail(auth, "Missing or invalid R-capabilities");
		goto fail;
	}
	auth->r_capab = r_capab[0];
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: R-capabilities: 0x%02x", auth->r_capab);
	role = auth->r_capab & DPP_CAPAB_ROLE_MASK;
	if ((auth->allowed_roles ==
	     (DPP_CAPAB_CONFIGURATOR | DPP_CAPAB_ENROLLEE)) &&
	    (role == DPP_CAPAB_CONFIGURATOR || role == DPP_CAPAB_ENROLLEE)) {
		/* Peer selected its role, so move from "either role" to the
		 * role that is compatible with peer's selection. */
		auth->configurator = role == DPP_CAPAB_ENROLLEE;
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Acting as %s",
			   auth->configurator ? "Configurator" : "Enrollee");
	} else if ((auth->configurator && role != DPP_CAPAB_ENROLLEE) ||
		   (!auth->configurator && role != DPP_CAPAB_CONFIGURATOR)) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Incompatible role selection");
		if (role != DPP_CAPAB_ENROLLEE &&
		    role != DPP_CAPAB_CONFIGURATOR)
			goto fail;
		bin_clear_free(unwrapped, unwrapped_len);
		auth->remove_on_tx_status = 1;
		return dpp_auth_build_conf(auth, DPP_STATUS_NOT_COMPATIBLE);
	}

	wrapped2 = dpp_get_attr(unwrapped, unwrapped_len,
				DPP_ATTR_WRAPPED_DATA, &wrapped2_len);
	if (!wrapped2 || wrapped2_len < AES_BLOCK_SIZE) {
		dpp_auth_fail(auth,
			      "Missing or invalid Secondary Wrapped Data");
		goto fail;
	}

	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped2, wrapped2_len);

	if (dpp_derive_ke(auth, auth->ke, auth->curve->hash_len) < 0)
		goto fail;

	unwrapped2_len = wrapped2_len - AES_BLOCK_SIZE;
	unwrapped2 = os_malloc(unwrapped2_len);
	if (!unwrapped2)
		goto fail;
	if (aes_siv_decrypt(auth->ke, auth->curve->hash_len,
			    wrapped2, wrapped2_len,
			    0, NULL, NULL, unwrapped2) < 0) {
		dpp_auth_fail(auth, "AES-SIV decryption failed");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext",
		    unwrapped2, unwrapped2_len);

	if (dpp_check_attrs(unwrapped2, unwrapped2_len) < 0) {
		dpp_auth_fail(auth,
			      "Invalid attribute in secondary unwrapped data");
		goto fail;
	}

	r_auth = dpp_get_attr(unwrapped2, unwrapped2_len, DPP_ATTR_R_AUTH_TAG,
			       &r_auth_len);
	if (!r_auth || r_auth_len != auth->curve->hash_len) {
		dpp_auth_fail(auth,
			      "Missing or invalid Responder Authenticating Tag");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Received Responder Authenticating Tag",
		    r_auth, r_auth_len);
	/* R-auth' = H(I-nonce | R-nonce | PI.x | PR.x | [BI.x |] BR.x | 0) */
	if (dpp_gen_r_auth(auth, r_auth2) < 0)
		goto fail;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Calculated Responder Authenticating Tag",
		    r_auth2, r_auth_len);
	if (os_memcmp(r_auth, r_auth2, r_auth_len) != 0) {
		dpp_auth_fail(auth, "Mismatching Responder Authenticating Tag");
		bin_clear_free(unwrapped, unwrapped_len);
		bin_clear_free(unwrapped2, unwrapped2_len);
		auth->remove_on_tx_status = 1;
		return dpp_auth_build_conf(auth, DPP_STATUS_AUTH_FAILURE);
	}

	bin_clear_free(unwrapped, unwrapped_len);
	bin_clear_free(unwrapped2, unwrapped2_len);

	return dpp_auth_build_conf(auth, DPP_STATUS_OK);

fail:
	bin_clear_free(unwrapped, unwrapped_len);
	bin_clear_free(unwrapped2, unwrapped2_len);
	EVP_PKEY_free(pr);
	EVP_PKEY_CTX_free(ctx);
	return NULL;
}


static int dpp_auth_conf_rx_failure(struct dpp_authentication *auth,
				    const u8 *hdr,
				    const u8 *attr_start, size_t attr_len,
				    const u8 *wrapped_data,
				    u16 wrapped_data_len,
				    enum dpp_status_error status)
{
	const u8 *addr[2];
	size_t len[2];
	u8 *unwrapped = NULL;
	size_t unwrapped_len = 0;
	const u8 *r_nonce;
	u16 r_nonce_len;

	/* Authentication Confirm failure cases are expected to include
	 * {R-nonce}k2 in the Wrapped Data attribute. */

	addr[0] = hdr;
	len[0] = DPP_HDR_LEN;
	addr[1] = attr_start;
	len[1] = attr_len;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped_data, wrapped_data_len);
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped = os_malloc(unwrapped_len);
	if (!unwrapped) {
		dpp_auth_fail(auth, "Authentication failed");
		goto fail;
	}
	if (aes_siv_decrypt(auth->k2, auth->curve->hash_len,
			    wrapped_data, wrapped_data_len,
			    2, addr, len, unwrapped) < 0) {
		dpp_auth_fail(auth, "AES-SIV decryption failed");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext",
		    unwrapped, unwrapped_len);

	if (dpp_check_attrs(unwrapped, unwrapped_len) < 0) {
		dpp_auth_fail(auth, "Invalid attribute in unwrapped data");
		goto fail;
	}

	r_nonce = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_R_NONCE,
			       &r_nonce_len);
	if (!r_nonce || r_nonce_len != auth->curve->nonce_len) {
		dpp_auth_fail(auth, "DPP: Missing or invalid R-nonce");
		goto fail;
	}
	if (os_memcmp(r_nonce, auth->r_nonce, r_nonce_len) != 0) {
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Received R-nonce",
			    r_nonce, r_nonce_len);
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Expected R-nonce",
			    auth->r_nonce, r_nonce_len);
		dpp_auth_fail(auth, "R-nonce mismatch");
		goto fail;
	}

	if (status == DPP_STATUS_NOT_COMPATIBLE)
		dpp_auth_fail(auth, "Peer reported incompatible R-capab role");
	else if (status == DPP_STATUS_AUTH_FAILURE)
		dpp_auth_fail(auth, "Peer reported authentication failure)");

fail:
	bin_clear_free(unwrapped, unwrapped_len);
	return -1;
}


int dpp_auth_conf_rx(struct dpp_authentication *auth, const u8 *hdr,
		     const u8 *attr_start, size_t attr_len)
{
	const u8 *r_bootstrap, *i_bootstrap, *wrapped_data, *status, *i_auth;
	u16 r_bootstrap_len, i_bootstrap_len, wrapped_data_len, status_len,
		i_auth_len;
	const u8 *addr[2];
	size_t len[2];
	u8 *unwrapped = NULL;
	size_t unwrapped_len = 0;
	u8 i_auth2[DPP_MAX_HASH_LEN];

	if (auth->initiator || !auth->own_bi) {
		dpp_auth_fail(auth, "Unexpected Authentication Confirm");
		return -1;
	}

	wrapped_data = dpp_get_attr(attr_start, attr_len, DPP_ATTR_WRAPPED_DATA,
				    &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		dpp_auth_fail(auth,
			      "Missing or invalid required Wrapped Data attribute");
		return -1;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Wrapped data",
		    wrapped_data, wrapped_data_len);

	attr_len = wrapped_data - 4 - attr_start;

	r_bootstrap = dpp_get_attr(attr_start, attr_len,
				   DPP_ATTR_R_BOOTSTRAP_KEY_HASH,
				   &r_bootstrap_len);
	if (!r_bootstrap || r_bootstrap_len != SHA256_MAC_LEN) {
		dpp_auth_fail(auth,
			      "Missing or invalid required Responder Bootstrapping Key Hash attribute");
		return -1;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Responder Bootstrapping Key Hash",
		    r_bootstrap, r_bootstrap_len);
	if (os_memcmp(r_bootstrap, auth->own_bi->pubkey_hash,
		      SHA256_MAC_LEN) != 0) {
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX
			    "DPP: Expected Responder Bootstrapping Key Hash",
			    auth->peer_bi->pubkey_hash, SHA256_MAC_LEN);
		dpp_auth_fail(auth,
			      "Responder Bootstrapping Key Hash mismatch");
		return -1;
	}

	i_bootstrap = dpp_get_attr(attr_start, attr_len,
				   DPP_ATTR_I_BOOTSTRAP_KEY_HASH,
				   &i_bootstrap_len);
	if (i_bootstrap) {
		if (i_bootstrap_len != SHA256_MAC_LEN) {
			dpp_auth_fail(auth,
				      "Invalid Initiator Bootstrapping Key Hash attribute");
			return -1;
		}
		wpa_hexdump(MSG_MSGDUMP, DPP_MAP_PREX
			    "DPP: Initiator Bootstrapping Key Hash",
			    i_bootstrap, i_bootstrap_len);
		if (!auth->peer_bi ||
		    os_memcmp(i_bootstrap, auth->peer_bi->pubkey_hash,
			      SHA256_MAC_LEN) != 0) {
			dpp_auth_fail(auth,
				      "Initiator Bootstrapping Key Hash mismatch");
			return -1;
		}
	} else if (auth->peer_bi) {
		/* Mutual authentication and peer did not include its
		 * Bootstrapping Key Hash attribute. */
		dpp_auth_fail(auth,
			      "Missing Initiator Bootstrapping Key Hash attribute");
		return -1;
	}

	status = dpp_get_attr(attr_start, attr_len, DPP_ATTR_STATUS,
			      &status_len);
	if (!status || status_len < 1) {
		dpp_auth_fail(auth,
			      "Missing or invalid required DPP Status attribute");
		return -1;
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Status %u", status[0]);
	if (status[0] == DPP_STATUS_NOT_COMPATIBLE ||
	    status[0] == DPP_STATUS_AUTH_FAILURE)
		return dpp_auth_conf_rx_failure(auth, hdr, attr_start,
						attr_len, wrapped_data,
						wrapped_data_len, status[0]);

	if (status[0] != DPP_STATUS_OK) {
		dpp_auth_fail(auth, "Authentication failed");
		return -1;
	}

	addr[0] = hdr;
	len[0] = DPP_HDR_LEN;
	addr[1] = attr_start;
	len[1] = attr_len;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped_data, wrapped_data_len);
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped = os_malloc(unwrapped_len);
	if (!unwrapped)
		return -1;
	if (aes_siv_decrypt(auth->ke, auth->curve->hash_len,
			    wrapped_data, wrapped_data_len,
			    2, addr, len, unwrapped) < 0) {
		dpp_auth_fail(auth, "AES-SIV decryption failed");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext",
		    unwrapped, unwrapped_len);

	if (dpp_check_attrs(unwrapped, unwrapped_len) < 0) {
		dpp_auth_fail(auth, "Invalid attribute in unwrapped data");
		goto fail;
	}

	i_auth = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_I_AUTH_TAG,
			      &i_auth_len);
	if (!i_auth || i_auth_len != auth->curve->hash_len) {
		dpp_auth_fail(auth,
			      "Missing or invalid Initiator Authenticating Tag");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Received Initiator Authenticating Tag",
		    i_auth, i_auth_len);
	/* I-auth' = H(R-nonce | I-nonce | PR.x | PI.x | BR.x | [BI.x |] 1) */
	if (dpp_gen_i_auth(auth, i_auth2) < 0)
		goto fail;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Calculated Initiator Authenticating Tag",
		    i_auth2, i_auth_len);
	if (os_memcmp(i_auth, i_auth2, i_auth_len) != 0) {
		dpp_auth_fail(auth, "Mismatching Initiator Authenticating Tag");
		goto fail;
	}

	bin_clear_free(unwrapped, unwrapped_len);
	dpp_auth_success(auth);
	return 0;
fail:
	dpp_auth_deinit(auth);
	bin_clear_free(unwrapped, unwrapped_len);
	return -1;
}


static int bin_str_eq(const char *val, size_t len, const char *cmp)
{
	return os_strlen(cmp) == len && os_memcmp(val, cmp, len) == 0;
}


struct dpp_configuration * dpp_configuration_alloc(const char *type)
{
	struct dpp_configuration *conf;
	const char *end;
	size_t len;

	conf = os_zalloc(sizeof(*conf));
	if (!conf)
		goto fail;

	end = os_strchr(type, ' ');
	if (end)
		len = end - type;
	else
		len = os_strlen(type);

	if (bin_str_eq(type, len, "psk"))
		conf->akm = DPP_AKM_PSK;
	else if (bin_str_eq(type, len, "sae"))
		conf->akm = DPP_AKM_SAE;
	else if (bin_str_eq(type, len, "psk-sae") ||
		 bin_str_eq(type, len, "psk+sae"))
		conf->akm = DPP_AKM_PSK_SAE;
	else if (bin_str_eq(type, len, "sae-dpp") ||
		 bin_str_eq(type, len, "dpp+sae"))
		conf->akm = DPP_AKM_SAE_DPP;
	else if (bin_str_eq(type, len, "psk-sae-dpp") ||
		 bin_str_eq(type, len, "dpp+psk+sae"))
		conf->akm = DPP_AKM_PSK_SAE_DPP;
	else if (bin_str_eq(type, len, "dpp"))
		conf->akm = DPP_AKM_DPP;
	else
		goto fail;

	return conf;
fail:
	dpp_configuration_free(conf);
	return NULL;
}


int dpp_akm_psk(enum dpp_akm akm)
{
	return akm == DPP_AKM_PSK || akm == DPP_AKM_PSK_SAE ||
		 akm == DPP_AKM_PSK_DPP || akm == DPP_AKM_PSK_SAE_DPP;
}


int dpp_akm_sae(enum dpp_akm akm)
{
	return akm == DPP_AKM_SAE || akm == DPP_AKM_PSK_SAE ||
		akm == DPP_AKM_SAE_DPP || akm == DPP_AKM_PSK_SAE_DPP;
}


int dpp_akm_legacy(enum dpp_akm akm)
{
	return akm == DPP_AKM_PSK || akm == DPP_AKM_PSK_SAE ||
		akm == DPP_AKM_SAE;
}


int dpp_akm_dpp(enum dpp_akm akm)
{
	return akm == DPP_AKM_DPP || akm == DPP_AKM_PSK_DPP ||
		 akm == DPP_AKM_SAE_DPP || akm == DPP_AKM_PSK_SAE_DPP;
}


int dpp_akm_ver2(enum dpp_akm akm)
{
	return  akm == DPP_AKM_PSK_DPP || akm == DPP_AKM_SAE_DPP
		 || akm == DPP_AKM_PSK_SAE_DPP;
}


int dpp_configuration_valid(const struct dpp_configuration *conf)
{
	if (conf->ssid_len == 0)
		return 0;
	if (dpp_akm_psk(conf->akm) && !conf->passphrase && !conf->psk_set)
		return 0;
	if (dpp_akm_sae(conf->akm) && !conf->passphrase)
		return 0;
	return 1;
}


void dpp_configuration_free(struct dpp_configuration *conf)
{
	if (!conf)
		return;

	str_clear_free(conf->passphrase);
	os_free(conf->group_id);
	bin_clear_free(conf, sizeof(*conf));
}

#ifdef DPP_R2_MUOBJ
void dpp_free_config_info(struct dpp_configuration *conf, unsigned int start, unsigned int end) 
{
	unsigned int i = start;

	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"start:%d, end:%d\n", start, end);
	if(!conf || !end || start >= end)
		return;

	while( i < end) {
		if(conf[i].passphrase) {
			free(conf[i].passphrase);
			conf[i].passphrase = NULL;
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"passphrase free index %d\n", i);
		}

		if(conf[i].group_id) {
			free(conf[i].group_id);
			conf[i].group_id = NULL;
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"group_id free index %d\n", i);
		}
		i ++;
	}
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"%s done\n", __func__);
}

void dpp_conf_obj_free(struct dpp_config_obj *conf)
{
	if (!conf || !conf->connector)
		return;

	os_free(conf->connector);
}

static int dpp_configuration_parse(struct dpp_global *dpp,
				   struct dpp_authentication *auth,
				   const char *cmd)
{
	const char *pos = NULL, *end = NULL;
	struct dpp_configuration *conf_sta = NULL, *conf_ap = NULL;
	struct dpp_configuration *conf = NULL;
	int idx = 0;

	if (cmd)
		pos = os_strstr(cmd, " conf=sta-");
	if (pos) {
		conf_sta = dpp_configuration_alloc(pos + 10);
		if (!conf_sta)
			goto fail;
		conf = conf_sta;
	}

	if (cmd)
		pos = os_strstr(cmd, " conf=ap-");
	if (pos) {
		conf_ap = dpp_configuration_alloc(pos + 9);
		if (!conf_ap)
			goto fail;
		conf = conf_ap;
	}

	if (!conf && dpp->dpp_configurator_supported) {
		if (dpp->dpp_conf_sta_num) {
			for(idx = 0; idx < dpp->dpp_conf_sta_num; idx++) {
				int pass_len = os_strlen(dpp->config_sta[idx].passphrase);
				int id_len = os_strlen(dpp->config_sta[idx].group_id);
				auth->config_sta[idx] = os_zalloc(sizeof(*conf));
				if (!auth->config_sta[idx]) {
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Error malloc failed\n");
					goto fail;
				}
				os_memcpy(auth->config_sta[idx], &(dpp->config_sta[idx]), sizeof(*conf));
				auth->config_sta[idx]->passphrase = os_zalloc(pass_len + 1);
				if (!auth->config_sta[idx]->passphrase) {
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Error malloc failed\n");
					goto fail;
				}
				os_strlcpy((char *)auth->config_sta[idx]->passphrase, dpp->config_sta[idx].passphrase,
					sizeof(auth->config_sta[idx]->passphrase));
				auth->config_sta[idx]->group_id = os_zalloc(id_len + 1);
				if (!auth->config_sta[idx]->group_id) {
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Error malloc failed\n");
					goto fail;
				}
				os_strlcpy((char *)auth->config_sta[idx]->group_id, dpp->config_sta[idx].group_id,
					sizeof(auth->config_sta[idx]->group_id));

				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"sta (idx:%d) ssid:%s, pass :%s, group id:%s \n", idx,
					auth->config_sta[idx]->ssid, auth->config_sta[idx]->passphrase, auth->config_sta[idx]->group_id);
			}
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"sta num:%d, idx:%d\n", dpp->dpp_conf_sta_num, idx);
		}
		if (dpp->dpp_conf_ap_num) {
			for(idx = 0; idx < dpp->dpp_conf_ap_num; idx++) {
				int pass_len = os_strlen(dpp->config_ap[idx].passphrase);
				int id_len = os_strlen(dpp->config_ap[idx].group_id);
				auth->config_ap[idx] = os_zalloc(sizeof(*conf));
				if (!auth->config_ap[idx]) {
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Error malloc failed\n");
					goto fail;
				}
				os_memcpy(auth->config_ap[idx], &(dpp->config_ap[idx]), sizeof(*conf));
				auth->config_ap[idx]->passphrase = os_zalloc(pass_len + 1);
				if (!auth->config_ap[idx]->passphrase) {
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Error malloc failed\n");
					goto fail;
				}
				os_strlcpy((char *)auth->config_ap[idx]->passphrase, dpp->config_ap[idx].passphrase,
					sizeof(auth->config_ap[idx]->passphrase));
				auth->config_ap[idx]->group_id = os_zalloc(id_len + 1);
				if (!auth->config_ap[idx]->group_id) {
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Error malloc failed\n");
					goto fail;
				}
				os_strlcpy((char *)auth->config_ap[idx]->group_id, dpp->config_ap[idx].group_id,
					sizeof(auth->config_ap[idx]->group_id));

				DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"ap (idx:%d) ssid:%s, pass :%s, group id:%s \n", idx, 
					auth->config_ap[idx]->ssid, auth->config_sta[idx]->passphrase, auth->config_sta[idx]->group_id);
			}
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"ap num:%d, idx:%d\n", dpp->dpp_conf_ap_num, idx);
		}

		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"setting global config from dpp cfg file\n");
		return 0;
	}

	if (cmd)
		pos = os_strstr(cmd, " ssid=");
	if (pos) {
		pos += 6;
		end = os_strchr(pos, ' ');
		conf->ssid_len = end ? (size_t) (end - pos) : os_strlen(pos);
		conf->ssid_len /= 2;
		if (conf->ssid_len > sizeof(conf->ssid) ||
		    hexstr2bin(pos, conf->ssid, conf->ssid_len) < 0)
			goto fail;
	} else {
		goto fail;
	}

	if (cmd)
		pos = os_strstr(cmd, " pass=");
	if (pos) {
		size_t pass_len;

		pos += 6;
		end = os_strchr(pos, ' ');
		pass_len = end ? (size_t) (end - pos) : os_strlen(pos);
		pass_len /= 2;
		if (pass_len > 63 || pass_len < 8)
			goto fail;
		conf->passphrase = os_zalloc(pass_len + 1);
		if (!conf->passphrase ||
		    hexstr2bin(pos, (u8 *) conf->passphrase, pass_len) < 0)
			goto fail;
	}

	if (cmd)
		pos = os_strstr(cmd, " psk=");
	if (pos) {
		pos += 5;
		if (hexstr2bin(pos, conf->psk, PMK_LEN) < 0)
			goto fail;
		conf->psk_set = 1;
	}

	if (cmd)
		pos = os_strstr(cmd, " group_id=");
	if (pos) {
		size_t group_id_len;

		pos += 10;
		end = os_strchr(pos, ' ');
		group_id_len = end ? (size_t) (end - pos) : os_strlen(pos);
		conf->group_id = os_malloc(group_id_len + 1);
		if (!conf->group_id)
			goto fail;
		os_memcpy(conf->group_id, pos, group_id_len);
		conf->group_id[group_id_len] = '\0';
	}

	if (cmd)
		pos = os_strstr(cmd, " expiry=");
	if (pos) {
		long int val;

		pos += 8;
		val = strtol(pos, NULL, 0);
		if (val <= 0)
			goto fail;
		conf->netaccesskey_expiry = val;
	}

	if (!dpp_configuration_valid(conf))
		goto fail;

	auth->config_sta[0] = conf_sta;
	auth->config_ap[0] = conf_ap;
	return 0;

fail:
	for(idx = 0; idx < DPP_CONF_OBJ_MAX; idx++) {
		dpp_configuration_free( auth->config_sta[idx]);
		auth->config_sta[idx] = NULL;
	}

	for(idx = 0; idx < DPP_CONF_OBJ_MAX; idx++) {
		dpp_configuration_free( auth->config_ap[idx]);
		auth->config_ap[idx] = NULL;
	}

	return -1;
}
#else
static int dpp_configuration_parse(struct dpp_global *dpp,
				   struct dpp_authentication *auth,
				   const char *cmd)
{
	const char *pos = NULL, *end = NULL;
	struct dpp_configuration *conf_sta = NULL, *conf_ap = NULL;
	struct dpp_configuration *conf = NULL;

	if (cmd)
		pos = os_strstr(cmd, " conf=sta-");
	if (pos) {
		conf_sta = dpp_configuration_alloc(pos + 10);
		if (!conf_sta)
			goto fail;
		conf = conf_sta;
	}

	if (cmd)
		pos = os_strstr(cmd, " conf=ap-");
	if (pos) {
		conf_ap = dpp_configuration_alloc(pos + 9);
		if (!conf_ap)
			goto fail;
		conf = conf_ap;
	}

	if (!conf && dpp->dpp_configurator_supported) {
		if (dpp->conf_sta) {
			int pass_len = os_strlen(dpp->conf_sta->passphrase);
			int id_len = os_strlen(dpp->conf_sta->group_id);
			auth->conf_sta = os_zalloc(sizeof(*conf));
			if (!auth->conf_sta) {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s: Error malloc failed\n", __func__);
				return -1;
			}
			os_memcpy(auth->conf_sta, dpp->conf_sta, sizeof(*conf));
			auth->conf_sta->passphrase = os_zalloc(pass_len + 1);
			os_strlcpy((char *)auth->conf_sta->passphrase, dpp->conf_sta->passphrase,
				pass_len + 1);
			auth->conf_sta->group_id = os_zalloc(id_len + 1);
			os_strlcpy((char *)auth->conf_sta->group_id, dpp->conf_sta->group_id,
				id_len + 1);
		}
		if (dpp->conf_ap) {
			int pass_len = os_strlen(dpp->conf_ap->passphrase);
			int id_len = os_strlen(dpp->conf_ap->group_id);
			auth->conf_ap = os_zalloc(sizeof(*conf));
			if (!auth->conf_ap){
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s: Error malloc failed\n", __func__);
				return -1;
			}
			os_memcpy(auth->conf_ap, dpp->conf_ap, sizeof(*conf));
			auth->conf_ap->passphrase = os_zalloc(pass_len + 1);
			os_strlcpy((char *)auth->conf_ap->passphrase, dpp->conf_ap->passphrase,
				pass_len + 1);
			auth->conf_ap->group_id = os_zalloc(id_len + 1);
			os_strlcpy((char *)auth->conf_ap->group_id, dpp->conf_ap->group_id,
				id_len + 1);
		}

		DBGPRINT(RT_DEBUG_INFO, DPP_MAP_PREX"setting global config from dpp cfg file\n");
		return 0;
	}

	if (cmd)
		pos = os_strstr(cmd, " ssid=");
	if (pos) {
		pos += 6;
		end = os_strchr(pos, ' ');
		conf->ssid_len = end ? (size_t) (end - pos) : os_strlen(pos);
		conf->ssid_len /= 2;
		if (conf->ssid_len > sizeof(conf->ssid) ||
		    hexstr2bin(pos, conf->ssid, conf->ssid_len) < 0)
			goto fail;
	} else {
		goto fail;
	}

	if (cmd)
		pos = os_strstr(cmd, " pass=");
	if (pos) {
		size_t pass_len;

		pos += 6;
		end = os_strchr(pos, ' ');
		pass_len = end ? (size_t) (end - pos) : os_strlen(pos);
		pass_len /= 2;
		if (pass_len > 63 || pass_len < 8)
			goto fail;
		conf->passphrase = os_zalloc(pass_len + 1);
		if (!conf->passphrase ||
		    hexstr2bin(pos, (u8 *) conf->passphrase, pass_len) < 0)
			goto fail;
	}

	if (cmd)
		pos = os_strstr(cmd, " psk=");
	if (pos) {
		pos += 5;
		if (hexstr2bin(pos, conf->psk, PMK_LEN) < 0)
			goto fail;
		conf->psk_set = 1;
	}

	if (cmd)
		pos = os_strstr(cmd, " group_id=");
	if (pos) {
		size_t group_id_len;

		pos += 10;
		end = os_strchr(pos, ' ');
		group_id_len = end ? (size_t) (end - pos) : os_strlen(pos);
		conf->group_id = os_malloc(group_id_len + 1);
		if (!conf->group_id)
			goto fail;
		os_memcpy(conf->group_id, pos, group_id_len);
		conf->group_id[group_id_len] = '\0';
	}

	if (cmd)
		pos = os_strstr(cmd, " expiry=");
	if (pos) {
		long int val;

		pos += 8;
		val = strtol(pos, NULL, 0);
		if (val <= 0)
			goto fail;
		conf->netaccesskey_expiry = val;
	}

	if (!dpp_configuration_valid(conf))
		goto fail;

	auth->conf_sta = conf_sta;
	auth->conf_ap = conf_ap;
	return 0;

fail:
	dpp_configuration_free(conf_sta);
	dpp_configuration_free(conf_ap);
	return -1;
}
#endif /* DPP_R2_MUOBJ */

struct dpp_configurator *
dpp_configurator_get_id(struct dpp_global *dpp, unsigned int id)
{
	struct dpp_configurator *conf;

	if (!dpp)
		return NULL;

	dl_list_for_each(conf, &dpp->configurator,
			 struct dpp_configurator, list) {
		if (conf->id == id)
			return conf;
	}
	return NULL;
}
extern u8 obj_count;

int dpp_set_configurator(struct dpp_global *dpp, void *msg_ctx,
			 struct dpp_authentication *auth,
			 const char *cmd)
{
	const char *pos = NULL;

	if (cmd) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Set configurator parameters: %s", cmd);
		pos = os_strstr(cmd, "configurator=");
	}

	if (pos) {
		pos += 13;
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"got configurator id is %d\n", atoi(pos));
		auth->conf = dpp_configurator_get_id(dpp, atoi(pos));
		if (!auth->conf) {
			DBGPRINT(RT_DEBUG_ERROR,
				   DPP_MAP_PREX"DPP: Could not find the specified configurator");
			return -1;
		}
	} else {
		auth->conf = dpp_configurator_get_id(dpp, 1);
		/* default config mode not found, let's work as enrollee mode */

		if (!auth->conf) {
			DBGPRINT(RT_DEBUG_ERROR,
				   DPP_MAP_PREX"DPP: Could not find configurator id");
			return 0;
		}
	}

	if (dpp_configuration_parse(dpp, auth, cmd) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"failing in configuration parsing \n");
		return -1;
	}
	return 0;
}

#ifdef DPP_R2_MUOBJ
void dpp_auth_deinit(struct dpp_authentication *auth)
{
	int idx = 0;
	if (!auth)
		return;
#ifdef MAP_R3
	struct wifi_app *wapp = auth->msg_ctx;
	if (wapp && wapp->map && wapp->map->map_version == DEV_TYPE_R3 && wapp->map->TurnKeyEnable)
		wapp->map->dpp_after_scan = 0;
#endif

#ifdef CONFIG_DPP2
	if (auth->sock >= 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Close Controller socket %d",
			   auth->sock);
		eloop_unregister_sock(auth->sock, EVENT_TYPE_READ);
		eloop_unregister_sock(auth->sock, EVENT_TYPE_WRITE);
		close(auth->sock);
	}
	wpabuf_free(auth->msg);
	wpabuf_free(auth->msg_out);
#endif /* CONFIG_DPP2 */
#ifdef MAP_R3
	auth->ethernetTrigger = FALSE;
	auth->is_1905_connector = FALSE;
#ifdef MAP_R3_RECONFIG
	auth->reconfigTrigger = FALSE;
#endif
#endif
	if(auth->list.next != NULL && auth->list.prev != NULL)
		wapp_dpp_auth_list_remove(auth);
	for(idx = 0; idx < DPP_CONF_OBJ_MAX; idx++) {
		dpp_configuration_free( auth->config_sta[idx]);
		auth->config_sta[idx] = NULL;
	}

	for(idx = 0; idx < DPP_CONF_OBJ_MAX; idx++) {
		dpp_configuration_free( auth->config_ap[idx]);
		auth->config_ap[idx] = NULL;
	}

	EVP_PKEY_free(auth->own_protocol_key);
	EVP_PKEY_free(auth->peer_protocol_key);
	wpabuf_free(auth->req_msg);
	wpabuf_free(auth->resp_msg);
	wpabuf_free(auth->conf_req);
#ifdef DPP_R2_RECONFIG
	EVP_PKEY_free(auth->own_connector_key);
	EVP_PKEY_free(auth->peer_connector_key);
	EVP_PKEY_free(auth->reconfig_old_protocol_key);
	wpabuf_free(auth->reconfig_resp_msg);
#endif /* DPP_R2_RECONFIG */
	for(idx = 0; idx < DPP_CONF_OBJ_MAX; idx++) {
		dpp_conf_obj_free( &(auth->conf_obj[idx]));
	}
	//os_free(auth->connector);
	wpabuf_free(auth->net_access_key);
	wpabuf_free(auth->c_sign_key);
	dpp_bootstrap_info_free(auth->tmp_own_bi);
	bin_clear_free(auth, sizeof(*auth));
	auth = NULL;
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"%s \n", __func__);
}
#else

void dpp_auth_deinit(struct dpp_authentication *auth)
{
	if (!auth) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"auth instance not found..\n");
		return;
	}

#ifdef MAP_R3
	struct wifi_app *wapp = (struct wifi_app *)auth->msg_ctx;
	if (!wapp) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"wapp instance not found..\n");
		return;
	}

	if (wapp && wapp->map && wapp->map->map_version == DEV_TYPE_R3 && wapp->map->TurnKeyEnable)
		wapp->map->dpp_after_scan = 0;
#endif

#ifdef CONFIG_DPP2
	if (auth->sock >= 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Close Controller socket %d",
			   auth->sock);
		eloop_unregister_sock(auth->sock, EVENT_TYPE_READ);
		eloop_unregister_sock(auth->sock, EVENT_TYPE_WRITE);
		close(auth->sock);
	}
	wpabuf_free(auth->msg);
	wpabuf_free(auth->msg_out);
#endif /* CONFIG_DPP2 */
#ifdef MAP_R3
	auth->ethernetTrigger = FALSE;
	auth->is_1905_connector = FALSE;
#ifdef MAP_R3_RECONFIG
	auth->reconfigTrigger = FALSE;
	auth->reconfig_bhconfig_done = FALSE;
	auth->is_wired = 0;
	wapp->dpp->radar_detect_ind = 0;
#endif
	obj_count = 0;
	if(!auth->self_sign_auth) {
/*
		//No need to delete key now
		if(wapp->dpp->dpp_allowed_roles == DPP_CAPAB_CONFIGURATOR) {
			dpp_bootstrap_remove(wapp->dpp, "2");
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"came here to delete key..\n");
		}
*/
		if(wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE) {
			//wapp->dpp->dpp_allowed_roles = DPP_CAPAB_PROXYAGENT;
			wapp->dpp->dpp_onboard_ongoing = 0;
			wapp->dpp->dpp_wifi_onboard_ongoing = FALSE;
			wapp->dpp->annouce_enrolle.pre_status = DPP_PRE_STATUS_INIT;
			//DBGPRINT(RT_DEBUG_INFO, DPP_MAP_PREX"Role changed to Proxy Agent..\n");
		}
	}
	auth->self_sign_auth = FALSE;
	wapp->dpp->config_done = 0;
	wapp->is_eth_onboard = FALSE;
#endif
	if(auth->list.next != NULL && auth->list.prev != NULL)
		wapp_dpp_auth_list_remove(auth);
	dpp_configuration_free(auth->conf_ap);
	dpp_configuration_free(auth->conf_sta);
	if (auth->own_connector_key)
		EVP_PKEY_free(auth->own_protocol_key);
	if (auth->peer_connector_key)
		EVP_PKEY_free(auth->peer_protocol_key);
#ifdef DPP_R2_RECONFIG
	if (auth->own_connector_key)
		EVP_PKEY_free(auth->own_connector_key);
	if (auth->peer_connector_key)
		EVP_PKEY_free(auth->peer_connector_key);
	wapp->reconfigTrigger = FALSE;
#ifdef MAP_R3_RECONFIG
	if (!auth->timeout_recon)
#endif /* MAP_R3_RECONFIG */
		wapp->wdev_backup = NULL;
	wapp->dpp->cce_driver_scan_ongoing = 0;
#endif /* DPP_R2_RECONFIG */

#ifndef MAP_R4
	wpabuf_free(auth->req_msg);
	auth->req_msg = NULL;
	wpabuf_free(auth->conf_req);
	auth->conf_req = NULL;
	if (wapp->dpp->dpp_allowed_roles != DPP_CAPAB_CONFIGURATOR)
#endif /* MAP_R4 */
	wpabuf_free(auth->resp_msg);
	auth->resp_msg = NULL;
	if (auth->connector)
		os_free(auth->connector);
	wpabuf_free(auth->net_access_key);
	wpabuf_free(auth->c_sign_key);
	dpp_bootstrap_info_free(auth->tmp_own_bi);
	bin_clear_free(auth, sizeof(*auth));
	auth = NULL;
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"%s", __func__);
}

#endif /* DPP_R2_MUOBJ */

static struct wpabuf *
dpp_build_conf_start(struct dpp_authentication *auth,
		     struct dpp_configuration *conf, size_t tailroom, enum dpp_config_type type)
{
	struct wpabuf *buf;
	char ssid[6 * sizeof(conf->ssid) + 1];
#ifdef MAP_R3
	int i = 0;
	unsigned short thresold_val = 5;
#endif

	buf = wpabuf_alloc(400 + tailroom);
	if (!buf)
		return NULL;
	if ((type == INFRA_STA) || type == INFRA_AP)
		wpabuf_put_str(buf, "{\"wi-fi_tech\":\"infra\",\"discovery\":{");
#ifdef MAP_R3
	else if ((type == MAP_BACKHAUL_AP) || (type == MAP_BACKHAUL_STA)) {
		wpabuf_put_str(buf, "{\"wi-fi_tech\":\"map\",\"discovery\":{");
		if (type == MAP_BACKHAUL_AP) {
			wpabuf_put_str(buf, "\"UBINDX\":\"");
			wpabuf_put_str(buf, (char *) base64_url_encode(&auth->bss_index, 1, NULL, 0));
			wpabuf_put_str(buf, "\",");
			auth->bss_index++;
			wpabuf_put_str(buf, "\"RUID\":\"");
			wpabuf_put_str(buf, (char *) base64_url_encode(auth->radio[i].identifier, 6,
						NULL, 0));
			wpabuf_put_str(buf, "\"");
		}
	} else if (type == MAP_FRONTHAUL_AP) {
		wpabuf_put_str(buf, "{\"wi-fi_tech\":\"inframap\",\"discovery\":{");
		wpabuf_put_str(buf, "\"UBINDX\":\"");
		wpabuf_put_str(buf, (char *) base64_url_encode(&auth->bss_index, 1, NULL, 0));
		wpabuf_put_str(buf, "\",");
		auth->bss_index++;
		wpabuf_put_str(buf, "\"RUID\":\"");
		wpabuf_put_str(buf, (char *) base64_url_encode(auth->radio[i].identifier, 6,
						NULL, 0));
		wpabuf_put_str(buf, "\"");
	} else if ((type == MAP_1905) || (type == MAP_1905_CONT)){
		wpabuf_put_str(buf, "{\"wi-fi_tech\":\"map\",");
		json_add_int(buf, "dfCounterThreshold", thresold_val);
		wpabuf_put_str(buf, ",");
	}
#endif /* MAP_R3 */
#ifdef DPP_R2_RECONFIG	
	else if (type == INFRA_CONFIGURATOR) {
		wpabuf_put_str(buf, "{\"wi-fi_tech\":\"infra\",\"discovery\":{");
	}
#endif  /* DPP_R2_RECONFIG */
	else {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"invalid option..\n");
		return NULL;
	}

#ifdef MAP_R3
	if ((type != MAP_1905) && (type != MAP_1905_CONT)) {
		if (type == MAP_FRONTHAUL_AP || type == MAP_BACKHAUL_AP)
			wpabuf_put_str(buf, ",");
#endif /* MAP_R3 */
		wpabuf_put_str(buf, "\"ssid\":\"");
		json_escape_string(ssid, sizeof(ssid),
				(const char *) conf->ssid, conf->ssid_len);
		wpabuf_put_str(buf, ssid);
		wpabuf_put_str(buf, "\"");
		wpabuf_put_str(buf, "},");
#ifdef MAP_R3
	}
#endif /* MAP_R3 */

	return buf;
}

int dpp_build_jwk(struct wpabuf *buf, const char *name, EVP_PKEY *key,
			 const char *kid, const struct dpp_curve_params *curve)
{
	struct wpabuf *pub;
	const u8 *pos;
	char *x = NULL, *y = NULL;
	int ret = -1;

	pub = dpp_get_pubkey_point(key, 0);
	if (!pub)
		goto fail;
	pos = wpabuf_head(pub);
	x = (char *) base64_url_encode(pos, curve->prime_len, NULL, 0);
	pos += curve->prime_len;
	y = (char *) base64_url_encode(pos, curve->prime_len, NULL, 0);
	if (!x || !y)
		goto fail;

	wpabuf_put_str(buf, "\"");
	wpabuf_put_str(buf, name);
	wpabuf_put_str(buf, "\":{\"kty\":\"EC\",\"crv\":\"");
	wpabuf_put_str(buf, curve->jwk_crv);
	wpabuf_put_str(buf, "\",\"x\":\"");
	wpabuf_put_str(buf, x);
	wpabuf_put_str(buf, "\",\"y\":\"");
	wpabuf_put_str(buf, y);
	if (kid) {
		wpabuf_put_str(buf, "\",\"kid\":\"");
		wpabuf_put_str(buf, kid);
	}
	wpabuf_put_str(buf, "\"}");
	ret = 0;
fail:
	wpabuf_free(pub);
	os_free(x);
	os_free(y);
	return ret;
}


static void dpp_build_legacy_cred_params(struct wpabuf *buf,
					 struct dpp_configuration *conf)
{
	if (conf->passphrase && os_strlen(conf->passphrase) < 64) {
		char pass[63 * 6 + 1];

		json_escape_string(pass, sizeof(pass), conf->passphrase,
				   os_strlen(conf->passphrase));
		wpabuf_put_str(buf, "\"pass\":\"");
		wpabuf_put_str(buf, pass);
		wpabuf_put_str(buf, "\"");
		os_memset(pass, 0, sizeof(pass));
	} else if (conf->psk_set) {
		char psk[2 * sizeof(conf->psk) + 1];

		os_snprintf_hex(psk, sizeof(psk),
				 conf->psk, sizeof(conf->psk));
		wpabuf_put_str(buf, "\"psk_hex\":\"");
		wpabuf_put_str(buf, psk);
		wpabuf_put_str(buf, "\"");
		os_memset(psk, 0, sizeof(psk));
	}
}

#ifdef MAP_R3
struct wpabuf *
dpp_build_conf_obj_bss(struct dpp_authentication *auth, enum dpp_config_type type,
		       struct dpp_configuration *conf)
{
	struct wpabuf *buf = NULL;
	char *signed1 = NULL, *signed2 = NULL, *signed3 = NULL;
	size_t tailroom;
	const struct dpp_curve_params *curve;
	char jws_prot_hdr[100];
	size_t signed1_len, signed2_len, signed3_len;
	struct wpabuf *dppcon = NULL;
	unsigned char *signature = NULL;
	const unsigned char *p;
	size_t signature_len;
	EVP_MD_CTX *md_ctx = NULL;
	ECDSA_SIG *sig = NULL;
	char *dot = ".";
	const EVP_MD *sign_md;
	const BIGNUM *r, *s;
	size_t extra_len = 1000;

	if (!auth->conf) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: No configurator specified - cannot generate DPP config object");
		goto fail;
	}
	curve = auth->conf->curve;
	if (curve->hash_len == SHA256_MAC_LEN) {
		sign_md = EVP_sha256();
	} else if (curve->hash_len == SHA384_MAC_LEN) {
		sign_md = EVP_sha384();
	} else if (curve->hash_len == SHA512_MAC_LEN) {
		sign_md = EVP_sha512();
	} else {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unknown signature algorithm");
		goto fail;
	}

	if (conf->group_id)
		extra_len += os_strlen(conf->group_id);
	/* Connector (JSON dppCon object) */
	dppcon = wpabuf_alloc(extra_len + 2 * auth->curve->prime_len * 4 / 3);
	if (!dppcon)
		goto fail;
	wpabuf_printf(dppcon, "{\"groups\":[{\"groupId\":\"%s\",",conf->group_id);
	if (type == MAP_BACKHAUL_AP)
		wpabuf_printf(dppcon, "\"netRole\":\"%s\"}],", "mapBackhaulBss");
	else if (type == MAP_FRONTHAUL_AP)
		wpabuf_printf(dppcon, "\"netRole\":\"%s\"}],", "ap");
	else if (type == INFRA_AP)
		wpabuf_printf(dppcon, "\"netRole\":\"%s\"}],", "ap");
	else if (type == INFRA_STA)
		wpabuf_printf(dppcon, "\"netRole\":\"%s\"}],", "sta");

	if (dpp_build_jwk(dppcon, "netAccessKey", auth->peer_protocol_key, NULL,
			  auth->curve) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to build netAccessKey JWK");
		goto fail;
	}
	if (conf->netaccesskey_expiry) {
		struct os_tm tm;

		if (os_gmtime(conf->netaccesskey_expiry, &tm) < 0) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Failed to generate expiry string");
			goto fail;
		}
		wpabuf_printf(dppcon,
			      ",\"expiry\":\"%04u-%02u-%02uT%02u:%02u:%02uZ\"",
			      tm.year, tm.month, tm.day,
			      tm.hour, tm.min, tm.sec);
	}
	wpabuf_put_u8(dppcon, '}');
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: dppCon: %s",
		   (const char *) wpabuf_head(dppcon));

	os_snprintf(jws_prot_hdr, sizeof(jws_prot_hdr),
		    "{\"typ\":\"dppCon\",\"kid\":\"%s\",\"alg\":\"%s\"}",
		    auth->conf->kid, curve->jws_alg);
	signed1 = (char *) base64_url_encode((unsigned char *) jws_prot_hdr,
					     os_strlen(jws_prot_hdr),
					     &signed1_len, 0);
	signed2 = (char *) base64_url_encode(wpabuf_head(dppcon),
					     wpabuf_len(dppcon),
					     &signed2_len, 0);
	if (!signed1 || !signed2)
		goto fail;

	md_ctx = EVP_MD_CTX_create();
	if (!md_ctx)
		goto fail;

	ERR_clear_error();
	if (EVP_DigestSignInit(md_ctx, NULL, sign_md, NULL,
			       auth->conf->csign) != 1) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: EVP_DigestSignInit failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	if (EVP_DigestSignUpdate(md_ctx, signed1, signed1_len) != 1 ||
	    EVP_DigestSignUpdate(md_ctx, dot, 1) != 1 ||
	    EVP_DigestSignUpdate(md_ctx, signed2, signed2_len) != 1) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: EVP_DigestSignUpdate failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	if (EVP_DigestSignFinal(md_ctx, NULL, &signature_len) != 1) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: EVP_DigestSignFinal failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	signature = os_malloc(signature_len);
	if (!signature)
		goto fail;
	if (EVP_DigestSignFinal(md_ctx, signature, &signature_len) != 1) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: EVP_DigestSignFinal failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: signedConnector ECDSA signature (DER)",
		    signature, signature_len);
	/* Convert to raw coordinates r,s */
	p = signature;
	sig = d2i_ECDSA_SIG(NULL, &p, signature_len);
	if (!sig)
		goto fail;
	ECDSA_SIG_get0(sig, &r, &s);
	if (dpp_bn2bin_pad(r, signature, curve->prime_len) < 0 ||
	    dpp_bn2bin_pad(s, signature + curve->prime_len,
			   curve->prime_len) < 0)
		goto fail;
	signature_len = 2 * curve->prime_len;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: signedConnector ECDSA signature (raw r,s)",
		    signature, signature_len);
	signed3 = (char *) base64_url_encode(signature, signature_len,
					     &signed3_len, 0);
	if (!signed3)
		goto fail;

	tailroom = 1000;
	tailroom += 2 * curve->prime_len * 4 / 3 + os_strlen(auth->conf->kid);
	tailroom += signed1_len + signed2_len + signed3_len;
	buf = wpabuf_alloc(tailroom);

	wpabuf_put_str(buf, "\"signedConnector\":\"");
	wpabuf_put_str(buf, signed1);
	wpabuf_put_u8(buf, '.');
	wpabuf_put_str(buf, signed2);
	wpabuf_put_u8(buf, '.');
	wpabuf_put_str(buf, signed3);
	wpabuf_put_str(buf, "\",");
	if (dpp_build_jwk(buf, "csign", auth->conf->csign, auth->conf->kid,
			  curve) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to build csign JWK");
		goto fail;
	}

	if (auth->peer_version >= 2 && auth->conf->pp_key) {
                json_value_sep(buf);
                if (dpp_build_jwk(buf, "ppKey", auth->conf->pp_key, NULL,
                                  curve) < 0) {
                        wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to build ppKey JWK");
                        goto fail;
                }
        }

	wpa_hexdump_ascii_key(MSG_DEBUG, DPP_MAP_PREX "DPP: Configuration Object",
			      wpabuf_head(buf), wpabuf_len(buf));

out:
	EVP_MD_CTX_destroy(md_ctx);
	ECDSA_SIG_free(sig);
	os_free(signed1);
	os_free(signed2);
	os_free(signed3);
	os_free(signature);
	wpabuf_free(dppcon);
	return buf;
fail:
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to build configuration object");
	wpabuf_free(buf);
	buf = NULL;
	goto out;
}
#endif /* MAP_R3 */

static struct wpabuf *
dpp_build_conf_obj_dpp(struct dpp_authentication *auth, enum dpp_config_type type,
		       struct dpp_configuration *conf)
{
	struct wpabuf *buf = NULL;
	char *signed1 = NULL, *signed2 = NULL, *signed3 = NULL;
	size_t tailroom;
	const struct dpp_curve_params *curve;
	char jws_prot_hdr[100];
	size_t signed1_len, signed2_len, signed3_len;
	struct wpabuf *dppcon = NULL;
	unsigned char *signature = NULL;
	const unsigned char *p;
	size_t signature_len;
	EVP_MD_CTX *md_ctx = NULL;
	ECDSA_SIG *sig = NULL;
	char *dot = ".";
	const EVP_MD *sign_md;
	const BIGNUM *r, *s;
	size_t extra_len = 1000;
	int incl_legacy;
	enum dpp_akm akm;

	if (!auth->conf) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: No configurator specified - cannot generate DPP config object");
		goto fail;
	}
	curve = auth->conf->curve;
	if (curve->hash_len == SHA256_MAC_LEN) {
		sign_md = EVP_sha256();
	} else if (curve->hash_len == SHA384_MAC_LEN) {
		sign_md = EVP_sha384();
	} else if (curve->hash_len == SHA512_MAC_LEN) {
		sign_md = EVP_sha512();
	} else {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unknown signature algorithm");
		goto fail;
	}

	akm = conf->akm;

	if ((auth->peer_version < 2)
#ifndef MAP_R3
	&& dpp_akm_ver2(akm)
#endif /* MAP_R3 */
	) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Convert DPP+legacy credential to DPP-only for peer that does not support version 2");
		akm = DPP_AKM_DPP;
	}
	if (conf->group_id)
		extra_len += os_strlen(conf->group_id);
	/* Connector (JSON dppCon object) */
	dppcon = wpabuf_alloc(extra_len + 2 * auth->curve->prime_len * 4 / 3);
	if (!dppcon)
		goto fail;
	wpabuf_printf(dppcon, "{\"groups\":[{\"groupId\":\"%s\",",conf->group_id);
	if (type == INFRA_AP)
		wpabuf_printf(dppcon, "\"netRole\":\"%s\"}],", "ap");
	else if (type == INFRA_STA)
		wpabuf_printf(dppcon, "\"netRole\":\"%s\"}],", "sta");
	else if (type == MAP_1905)
		wpabuf_printf(dppcon, "\"netRole\":\"%s\"}],", "mapAgent");
	else if (type == MAP_1905_CONT)
		wpabuf_printf(dppcon, "\"netRole\":\"%s\"}],", "mapController");
	else if (type == MAP_BACKHAUL_AP)
		wpabuf_printf(dppcon, "\"netRole\":\"%s\"}],", "mapBackhaulBss");
	else if (type == MAP_BACKHAUL_STA)
		wpabuf_printf(dppcon, "\"netRole\":\"%s\"}],", "mapBackhaulSta");
	else if (type == MAP_FRONTHAUL_AP)
		wpabuf_printf(dppcon, "\"netRole\":\"%s\"}],", "ap");
	else if (type == INFRA_CONFIGURATOR)
		wpabuf_printf(dppcon, "\"netRole\":\"%s\"}],", "configurator");


	if (
#ifdef DPP_R2_RECONFIG
		!auth->peer_protocol_key ||
#endif
		dpp_build_jwk(dppcon, "netAccessKey", auth->peer_protocol_key, NULL,
			  auth->curve) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to build netAccessKey JWK");
		goto fail;
	}
	if (conf->netaccesskey_expiry) {
		struct os_tm tm;

		if (os_gmtime(conf->netaccesskey_expiry, &tm) < 0) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Failed to generate expiry string");
			goto fail;
		}
		wpabuf_printf(dppcon,
			      ",\"expiry\":\"%04u-%02u-%02uT%02u:%02u:%02uZ\"",
			      tm.year, tm.month, tm.day,
			      tm.hour, tm.min, tm.sec);
	}
	wpabuf_put_u8(dppcon, '}');
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: dppCon: %s",
		   (const char *) wpabuf_head(dppcon));

	os_snprintf(jws_prot_hdr, sizeof(jws_prot_hdr),
		    "{\"typ\":\"dppCon\",\"kid\":\"%s\",\"alg\":\"%s\"}",
		    auth->conf->kid, curve->jws_alg);
	signed1 = (char *) base64_url_encode((unsigned char *) jws_prot_hdr,
					     os_strlen(jws_prot_hdr),
					     &signed1_len, 0);
	signed2 = (char *) base64_url_encode(wpabuf_head(dppcon),
					     wpabuf_len(dppcon),
					     &signed2_len, 0);
	if (!signed1 || !signed2)
		goto fail;

	md_ctx = EVP_MD_CTX_create();
	if (!md_ctx)
		goto fail;

	ERR_clear_error();
	if (EVP_DigestSignInit(md_ctx, NULL, sign_md, NULL,
			       auth->conf->csign) != 1) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: EVP_DigestSignInit failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	if (EVP_DigestSignUpdate(md_ctx, signed1, signed1_len) != 1 ||
	    EVP_DigestSignUpdate(md_ctx, dot, 1) != 1 ||
	    EVP_DigestSignUpdate(md_ctx, signed2, signed2_len) != 1) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: EVP_DigestSignUpdate failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	if (EVP_DigestSignFinal(md_ctx, NULL, &signature_len) != 1) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: EVP_DigestSignFinal failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	signature = os_malloc(signature_len);
	if (!signature)
		goto fail;
	if (EVP_DigestSignFinal(md_ctx, signature, &signature_len) != 1) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: EVP_DigestSignFinal failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: signedConnector ECDSA signature (DER)",
		    signature, signature_len);
	/* Convert to raw coordinates r,s */
	p = signature;
	sig = d2i_ECDSA_SIG(NULL, &p, signature_len);
	if (!sig)
		goto fail;
	ECDSA_SIG_get0(sig, &r, &s);
	if (dpp_bn2bin_pad(r, signature, curve->prime_len) < 0 ||
	    dpp_bn2bin_pad(s, signature + curve->prime_len,
			   curve->prime_len) < 0)
		goto fail;
	signature_len = 2 * curve->prime_len;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: signedConnector ECDSA signature (raw r,s)",
		    signature, signature_len);
	signed3 = (char *) base64_url_encode(signature, signature_len,
					     &signed3_len, 0);
	if (!signed3)
		goto fail;

	incl_legacy = dpp_akm_psk(akm) || dpp_akm_sae(akm);
	tailroom = 1000;
	tailroom += 2 * curve->prime_len * 4 / 3 + os_strlen(auth->conf->kid);
	tailroom += signed1_len + signed2_len + signed3_len;
	if (incl_legacy)
		tailroom += 1000;
	buf = dpp_build_conf_start(auth, conf, tailroom, type);
	if (!buf)
		goto fail;

	wpabuf_printf(buf, "\"cred\":{\"akm\":\"%s\",", dpp_akm_str(akm));
	if (incl_legacy) {
		dpp_build_legacy_cred_params(buf, conf);
		wpabuf_put_str(buf, ",");
	}
	wpabuf_put_str(buf, "\"signedConnector\":\"");
	wpabuf_put_str(buf, signed1);
	wpabuf_put_u8(buf, '.');
	wpabuf_put_str(buf, signed2);
	wpabuf_put_u8(buf, '.');
	wpabuf_put_str(buf, signed3);
	wpabuf_put_str(buf, "\",");
	if (dpp_build_jwk(buf, "csign", auth->conf->csign, auth->conf->kid,
			  curve) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to build csign JWK");
		goto fail;
	}

#ifdef MAP_R3
	if (auth->peer_version >= 2 && auth->conf->pp_key) {
                json_value_sep(buf);
                if (dpp_build_jwk(buf, "ppKey", auth->conf->pp_key, NULL,
                                  curve) < 0) {
                        wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to build ppKey JWK");
                        goto fail;
                }
        }
#endif /* MAP_R3 */

	wpabuf_put_str(buf, "}}");

	wpa_hexdump_ascii_key(MSG_DEBUG, DPP_MAP_PREX "DPP: Configuration Object",
			      wpabuf_head(buf), wpabuf_len(buf));

out:
	EVP_MD_CTX_destroy(md_ctx);
	ECDSA_SIG_free(sig);
	os_free(signed1);
	os_free(signed2);
	os_free(signed3);
	os_free(signature);
	wpabuf_free(dppcon);
	return buf;
fail:
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to build configuration object");
	wpabuf_free(buf);
	buf = NULL;
	goto out;
}

static struct wpabuf *
dpp_build_conf_obj_legacy(struct dpp_authentication *auth, enum dpp_config_type type,
			  struct dpp_configuration *conf)
{
	struct wpabuf *buf;

	buf = dpp_build_conf_start(auth, conf, 1000, type);
	if (!buf)
		return NULL;

	wpabuf_printf(buf, "\"cred\":{\"akm\":\"%s\",", dpp_akm_str(conf->akm));
	if (conf->passphrase) {
		char pass[63 * 6 + 1];

		if (os_strlen(conf->passphrase) > 63) {
			wpabuf_free(buf);
			return NULL;
		}

		json_escape_string(pass, sizeof(pass), conf->passphrase,
				   os_strlen(conf->passphrase));
		wpabuf_put_str(buf, "\"pass\":\"");
		wpabuf_put_str(buf, pass);
		wpabuf_put_str(buf, "\"");
	} else {
		char psk[2 * sizeof(conf->psk) + 1];

		os_snprintf_hex(psk, sizeof(psk),
				 conf->psk, sizeof(conf->psk));
		wpabuf_put_str(buf, "\"psk_hex\":\"");
		wpabuf_put_str(buf, psk);
		wpabuf_put_str(buf, "\"");
	}
	wpabuf_put_str(buf, "}}");

	wpa_hexdump_ascii_key(MSG_DEBUG, DPP_MAP_PREX "DPP: Configuration Object (legacy)",
			      wpabuf_head(buf), wpabuf_len(buf));

	return buf;
}

#ifdef MAP_R3
static void dpp_map_create_auth(struct set_config_bss_info *bss_config, struct dpp_configuration *conf)
{
	/* for 1905 */
	if (!bss_config) {
		if (conf->passphrase) {
			free(conf->passphrase);
			conf->passphrase = NULL;
		}

		conf->akm = DPP_AKM_DPP;
		return;
	}
	conf->ssid_len = strlen(bss_config->ssid);
	os_memcpy(conf->ssid, bss_config->ssid, conf->ssid_len);
	if (conf->passphrase) {
		free(conf->passphrase);
		conf->passphrase = NULL;
	}

#ifndef MAP_R3
	if ((bss_config->authmode & WPS_AUTH_SAE) &&
			(bss_config->authmode & WPS_AUTH_WPA2PSK)) {
		conf->akm = DPP_AKM_PSK_SAE;
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"akm is set to PSK+SAE from controller\n");
	}
	else if (bss_config->authmode & WPS_AUTH_WPA2PSK) {
		conf->akm = DPP_AKM_PSK;
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"akm is set to PSK from controller\n");
	}
	else if (bss_config->authmode & WPS_AUTH_SAE) {
		conf->akm = DPP_AKM_SAE;
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"akm is set to SAE from controller\n");
	}
	else {
		//TODO: Add for more security handling
		conf->akm = DPP_AKM_PSK;
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"need to add handling for more security setting PSK for now\n");
	}
#endif /* MAP_R3 */
	if(dpp_akm_psk(conf->akm) || dpp_akm_sae(conf->akm))
	{
		conf->passphrase = os_zalloc(strlen(bss_config->key)+1);
		os_memcpy(conf->passphrase, bss_config->key, strlen(bss_config->key));
		conf->passphrase[strlen(bss_config->key)] = '\0';
	}
}
#endif /* MAP_R3*/

#ifdef DPP_R2_MUOBJ

void dpp_build_conf_obj(struct dpp_authentication *auth, int ap, int is_map, int *conf_count, struct wpabuf **conf_map
#ifdef MAP_R3
		, struct json_token * akm, struct json_token * channel
#endif /* MAP_R3*/
)
{
	struct dpp_configuration *config[DPP_CONF_OBJ_MAX] = {NULL};
	struct dpp_configuration *conf = NULL;
	struct wifi_app *wapp;
	//int i, radio_cnt, bhsta_added = 0;
#ifdef MAP_R3
	struct wpabuf *msg1 = NULL;
	//struct peer_radio_info *radio;
	//int type = 0;
	//char zero_mac[6] = {0,0,0,0,0,0};
	u8 i,operating_chan=0;
	unsigned int chan[10];
	undigned int ch_count;
#endif /* MAP_R# */
	int max_obj = 0;

	if(ap)
		os_memcpy(config, auth->config_ap, sizeof(config[0]) * DPP_CONF_OBJ_MAX);
	else
		os_memcpy(config, auth->config_sta, sizeof(config[0]) * DPP_CONF_OBJ_MAX);	

	if ( !config[0] && !is_map) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: No configuration available for Enrollee(%s) - reject configuration request",
			   ap ? "ap" : "sta");
		return;
	}

	if(auth->peer_version >= 2)
		max_obj = DPP_CONF_OBJ_MAX;
	else
		max_obj = 1;

	if (!is_map) {
		int idx = 0;
		for (idx = 0; idx < max_obj; idx++) {
			if(!config[idx])
				break;
			if (dpp_akm_dpp(config[idx]->akm) || (auth->peer_version >= 2 && auth->conf))
				conf_map[idx] = dpp_build_conf_obj_dpp(auth, ap, config[idx]);
			else
				conf_map[idx] = dpp_build_conf_obj_legacy(auth, ap, config[idx]);
		}
			*conf_count = idx;
		wpa_printf(MSG_INFO1, DPP_MAP_PREX  "DPP: conf_count: %d \n", idx);
		return;
	}

	wapp = (struct wifi_app *) auth->msg_ctx;
	//radio_cnt = wapp->radio_count;

	if (!wapp) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: wapp is null??");
		return;
	}

#ifdef MAP_R3
	/* Check how many bss config we can give */
	if (!wapp->dpp->bss_config_num) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"wts config is not set\n");
		return;
	}

	conf = os_zalloc(sizeof(*conf));
	for (i = 0; i < wapp->dpp->bss_config_num; i++) {
		conf->akm = dpp_akm_from_str(akm->string);
		ch_count = get_chan_from_ch_list_str(channel->string, chan);
		operating_chan = wapp_op_band_frm_ch(wapp, chan);
		if ((wapp->dpp->bss_config[i].operating_chan == operating_chan)
				&& (wapp->dpp->bss_config[i].wfa_vendor_extension & BIT_BH_BSS)) {
			dpp_map_create_auth(&wapp->dpp->bss_config[i], conf);

			if (dpp_akm_dpp(conf->akm))
				msg1 = dpp_build_conf_obj_dpp(auth, MAP_BACKHAUL_STA, conf);
			else
				msg1 = dpp_build_conf_obj_legacy(auth, MAP_BACKHAUL_STA, conf);
			conf_map[*conf_count] = msg1;
			(*conf_count)++;
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" wapp->dpp->bss_config[%d].operating_chan =%d, operating_chan =%d\n",
					i,wapp->dpp->bss_config[i].operating_chan, operating_chan);
		}
	}
//#endif /* MAP_R3 */
	dpp_map_create_auth(NULL, conf);
	msg1 = dpp_build_conf_obj_dpp(auth, MAP_1905, conf);
	conf_map[*conf_count] = msg1;
	(*conf_count)++;
#endif /* MAP_R3 */
	free(conf);
}
#else

void dpp_build_conf_obj(struct dpp_authentication *auth, int ap, int is_map, int *conf_count, struct wpabuf **conf_map
#ifdef MAP_R3
		,struct json_token *akm, struct json_token *channel, u8 bsta_present, u8 self_sign
#ifdef MAP_R3_RECONFIG
		,u8 b1905Config, u8 breconfigconn
#endif /* MAP_R3*/
#endif
)
{
	struct dpp_configuration *conf = NULL;
	struct wifi_app *wapp=NULL;
	//int i, radio_cnt, bhsta_added = 0;
	//struct peer_radio_info *radio;
	//int type = 0;
	//char zero_mac[6] = {0,0,0,0,0,0};
#ifdef MAP_R3
	struct wpabuf *msg1 = NULL;
	u8 i;//,operating_chan=0;
	unsigned int chan[10] = {0};
	unsigned int ch_count;
	char conf_grpid_str[20] = "MAP_GRP_ID";	
	size_t group_id_len;
	u8 id_map = 0;
#endif /* MAP_R3 */
#ifdef MAP_R3
	unsigned short enrollee_hex_akm=0x00;
#endif /* MAP_R3 */

	conf = ap ? auth->conf_ap : auth->conf_sta;

#ifndef MAP_R3
	if (!conf && !is_map) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: No configuration available for Enrollee(%s) - reject configuration request",
			   ap ? "ap" : "sta");
		return;
	}
#endif /* MAP_R3 */

#ifdef MAP_R3_RECONFIG
	if (!breconfigconn)
	{	
#endif	
		wapp = (struct wifi_app *) auth->msg_ctx;
		//radio_cnt = wapp->radio_count;

	if (!wapp
#ifdef MAP_R3
	|| !wapp->dpp || !wapp->map
#endif /* MAP_R3 */
	) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: wapp or dpp/map is null");
		return;
	}

#ifdef MAP_R3
	if (wapp->dpp->is_map && wapp->dpp->dpp_configurator_supported) {
		DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"updating the BSS configurations from UCC");
		wapp_read_wts_map_config(wapp, MAPD_WTS_FILE,
		wapp->dpp->bss_config, MAX_SET_BSS_INFO_NUM, &wapp->dpp->bss_config_num);
	}
#endif /* MAP_R3 */

	/* Check how many bss config we can give */
	if (!wapp->dpp->bss_config_num) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"wts config is not set\n");
		return;
	}


	conf = os_zalloc(sizeof(*conf));
	if(conf == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" conf alloc failed\n");
		return;
	}

#if 1 //TODO check the group id string properly
	group_id_len = os_strlen(conf_grpid_str);
	conf->group_id = os_zalloc(group_id_len + 1);
	if(conf->group_id == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" conf group ID alloc failed\n");
		os_free(conf);
		return;
	}
	os_memcpy(conf->group_id, conf_grpid_str, group_id_len);
#endif
#ifdef MAP_R3_RECONFIG
	}
#endif

	if (!is_map && wapp  && wapp->dpp) {
#ifdef MAP_R3
#ifdef MAP_R3_RECONFIG
	if (!breconfigconn)
#endif
	{
		for (i = 0; i < wapp->dpp->bss_config_num; i++) {
			//conf->akm = dpp_akm_from_str(akm->string);
			//TODO: Add channel list handling here for turnkey
			//operating_chan = wapp_op_band_frm_ch(wapp, chan[0]);
			if(!is_broadcast_ether_addr(wapp->dpp->bss_config[i].mac)) {
				DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"mac addr given in wts file\n");
#if 0
				if ((wapp->dpp->bss_config[i].operating_chan == operating_chan)
#endif
				if((wapp->dpp->bss_config[i].wfa_vendor_extension & BIT_FH_BSS)
					&& (os_memcmp(wapp->dpp->bss_config[i].mac, wapp->dpp->al_mac, ETH_ALEN) == 0)) {
					dpp_map_create_auth(&wapp->dpp->bss_config[i], conf);

					if (dpp_akm_dpp(conf->akm) || auth->peer_version >= 2)
					msg1 = dpp_build_conf_obj_dpp(auth, MAP_BACKHAUL_STA, conf);
					else
					msg1 = dpp_build_conf_obj_legacy(auth, MAP_BACKHAUL_STA, conf);
					conf_map[id_map] = msg1;
					id_map++;
					DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX" wapp->dpp->bss_config[%d].operating_chan =%d, config num =%d\n",
							i,wapp->dpp->bss_config[i].operating_chan, i);
					break;
				}
				else{
					DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"mac addr does not match with proxy almac\n");
				}
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"mac addr not given in wts file\n");
#if 0
				if ((wapp->dpp->bss_config[i].operating_chan == operating_chan)
#endif
				if((wapp->dpp->bss_config[i].wfa_vendor_extension & BIT_FH_BSS)) {
					dpp_map_create_auth(&wapp->dpp->bss_config[i], conf);

					if (dpp_akm_dpp(conf->akm) || auth->peer_version >= 2)
					msg1 = dpp_build_conf_obj_dpp(auth, MAP_BACKHAUL_STA, conf);
					else
					msg1 = dpp_build_conf_obj_legacy(auth, MAP_BACKHAUL_STA, conf);
					conf_map[id_map] = msg1;
					id_map++;
					DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX" wapp->dpp->bss_config[%d].operating_chan =%d, config num =%d\n",
							i,wapp->dpp->bss_config[i].operating_chan, i);
					break;
				}
			}
		}
	}
#ifdef MAP_R3_RECONFIG
	else 
#endif
	{
		if (dpp_akm_dpp(conf->akm))
			conf_map[0] = dpp_build_conf_obj_dpp(auth, ap, conf);
		else
			conf_map[0] = dpp_build_conf_obj_legacy(auth, ap, conf);	
	}
#else
		if (dpp_akm_dpp(conf->akm))
			conf_map[0] = dpp_build_conf_obj_dpp(auth, ap, conf);
		else
			conf_map[0] = dpp_build_conf_obj_legacy(auth, ap, conf);
#endif /* MAP_R3 */
		if (conf_map[0])
			*conf_count = 1;
		if(conf->group_id)
			os_free(conf->group_id);
		if(conf->passphrase)
			os_free(conf->passphrase);
		os_free(conf);
		return;
	}

#if 0 //old code commented
	for (radio_cnt = 0; radio_cnt < 3; radio_cnt++) {
		radio = &auth->radio[radio_cnt];
		DBGPRINT(RT_DEBUG_ERROR,"radio identifier: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(radio->identifier));
		if (os_memcmp(radio->identifier, zero_mac, 6) == 0)
			continue;
		for (i = 0; i < wapp->dpp->bss_config_num; i++) {
			/* from file */

			if (radio->max_bss == 0)
				break;

			if (wapp->dpp->bss_config[i].is_used)
				continue;

			if (!bhsta_added &&radio->is_bh_sta_supported && (wapp->dpp->bss_config[i].operating_chan == radio->operating_chan)) {
				dpp_map_create_auth(&wapp->dpp->bss_config[i], conf);
				bhsta_added = 1;
				if (conf->akm == DPP_AKM_DPP)
					msg1 = dpp_build_conf_obj_dpp(auth, MAP_BACKHAUL_STA, conf);
				else
					msg1 = dpp_build_conf_obj_legacy(auth, MAP_BACKHAUL_STA, conf);
				conf_map[*conf_count] = msg1;
				(*conf_count)++;
				DBGPRINT(RT_DEBUG_ERROR,"bhsta_added=%d radio->is_bh_sta_supported=%d wapp->dpp->bss_config[i].operating_chan =%d, radio->operating_chan=%d\n", bhsta_added, radio->is_bh_sta_supported, wapp->dpp->bss_config[i].operating_chan, radio->operating_chan);
			}
			if ((wapp->dpp->bss_config[i].operating_chan == radio->operating_chan) && radio->max_bss > 0) {
				dpp_map_create_auth(&wapp->dpp->bss_config[i], conf);
				if (wapp->dpp->bss_config[i].wfa_vendor_extension & BIT_BH_BSS)
					type = MAP_BACKHAUL_AP;
				if (wapp->dpp->bss_config[i].wfa_vendor_extension & BIT_FH_BSS)
					type = MAP_FRONTHAUL_AP;
				bhsta_added = 1;
				radio->max_bss--;
				wapp->dpp->bss_config[i].is_used = 1;
			DBGPRINT(RT_DEBUG_ERROR,"wapp->dpp->bss_config[i].operating_chan=%d, radio->operating_chan=%d, radio->max_bss=%d\n",
				wapp->dpp->bss_config[i].operating_chan, radio->operating_chan, radio->max_bss);
				if (!(type & MAP_BACKHAUL_AP & MAP_FRONTHAUL_AP)) {
					if (conf->akm == DPP_AKM_DPP)
						msg1 = dpp_build_conf_obj_dpp(auth, type, conf);
					else
						msg1 = dpp_build_conf_obj_legacy(auth, type, conf);
				conf_map[*conf_count] = msg1;
				(*conf_count)++;
				} else {
					if (conf->akm == DPP_AKM_DPP)
						msg1 = dpp_build_conf_obj_dpp(auth, MAP_BACKHAUL_AP, conf);
					else
						msg1 = dpp_build_conf_obj_legacy(auth, MAP_BACKHAUL_AP, conf);
				conf_map[*conf_count] = msg1;
				(*conf_count)++;
					auth->bss_index--;
					if (conf->akm == DPP_AKM_DPP)
						msg1 = dpp_build_conf_obj_dpp(auth, MAP_FRONTHAUL_AP, conf);
					else
						msg1 = dpp_build_conf_obj_legacy(auth, MAP_FRONTHAUL_AP, conf);
				conf_map[*conf_count] = msg1;
				(*conf_count)++;
				}
			}
		}
	}
#endif
#ifdef MAP_R3


#ifdef MAP_R3_RECONFIG
	if (breconfigconn) {
#endif
		conf = os_zalloc(sizeof(*conf));
		if(conf == NULL) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" conf alloc failed\n");
			return;
		}

#if 1 //TODO check the group id string properly
		group_id_len = os_strlen(conf_grpid_str);
		conf->group_id = os_zalloc(group_id_len + 1);
		if(conf->group_id == NULL) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" conf group ID alloc failed\n");
			os_free(conf);
			return;
		}
		os_memcpy(conf->group_id, conf_grpid_str, group_id_len);
#endif
#ifdef MAP_R3_RECONFIG
	}
#endif

//NXP
#if 1
        dpp_map_create_auth(NULL, conf);
        if(self_sign)
                msg1 = dpp_build_conf_obj_dpp(auth, MAP_1905_CONT, conf);
        else {
#ifdef MAP_R3_RECONFIG
                if (auth->reconfigTrigger != TRUE)
#endif
                        msg1 = dpp_build_conf_obj_dpp(auth, MAP_1905, conf);
#ifdef MAP_R3_RECONFIG
                else if (auth->reconfigTrigger==TRUE && b1905Config)
                        msg1 = dpp_build_conf_obj_dpp(auth, MAP_1905, conf);
#endif
        }

#ifdef MAP_R3_RECONFIG
        if (auth->reconfigTrigger != TRUE ||
                (auth->reconfigTrigger == TRUE && b1905Config)) {
#endif
                conf_map[id_map] = msg1;
                id_map++;
#ifdef MAP_R3_RECONFIG
        }
#endif
#endif

	if (bsta_present && wapp && wapp->dpp)
	{
		//test NXP
		if (channel !=NULL) {
			DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"channel string :%s\n",channel->string);
			ch_count = get_chan_from_ch_list_str(channel->string, chan);
			DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"channel count in array is:%d\n",ch_count);
		}
		for (i = 0; i < wapp->dpp->bss_config_num; i++) {
			//conf->akm = dpp_akm_from_str(akm->string);
			//TODO: Add channel list handling here for turnkey
			//operating_chan = wapp_op_band_frm_ch(wapp, chan[0]);
#ifdef MAP_R3
                       conf->akm = dpp_akm_from_str(akm->string);
		
			// Hex value calculate for AKM
			if (conf->akm == DPP_AKM_PSK_SAE_DPP)
				enrollee_hex_akm = AUTH_DPP_ONLY | WPS_AUTH_SAE | WPS_AUTH_WPA2PSK;
			else if (conf->akm == DPP_AKM_SAE_DPP)
				enrollee_hex_akm = AUTH_DPP_ONLY | WPS_AUTH_SAE;
			else if (conf->akm == DPP_AKM_PSK_DPP)
				enrollee_hex_akm = AUTH_DPP_ONLY | WPS_AUTH_WPA2PSK;
			else if (conf->akm == DPP_AKM_PSK_SAE)
				enrollee_hex_akm = WPS_AUTH_SAE | WPS_AUTH_WPA2PSK;
			else if (conf->akm == DPP_AKM_DPP)
				enrollee_hex_akm = AUTH_DPP_ONLY;
			else if (conf->akm == DPP_AKM_SAE)
				enrollee_hex_akm = WPS_AUTH_SAE;
			else if (conf->akm == DPP_AKM_PSK)
				enrollee_hex_akm = WPS_AUTH_WPA2PSK;

			/* Get intersection value of authmode from WTS and value coming from Agent config request */
			enrollee_hex_akm &= wapp->dpp->bss_config[i].authmode;
			if (enrollee_hex_akm == 0x00) {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"akm match fail with WTS value and recived config request akm");
				continue;
			}
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"Value of intersect AKM is: 0x%x\n",enrollee_hex_akm);
			conf->akm = dpp_map_r3_akm_from_inter_val(enrollee_hex_akm);
#endif /* MAP_R3 */
			if(!is_broadcast_ether_addr(wapp->dpp->bss_config[i].mac)) {
				DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"mac addr given in wts file\n");
#if 0
				if ((wapp->dpp->bss_config[i].operating_chan == operating_chan)
#endif
				if((wapp->dpp->bss_config[i].wfa_vendor_extension & BIT_BH_BSS)
					&& ((os_memcmp(wapp->dpp->bss_config[i].mac, wapp->dpp->al_mac, ETH_ALEN) == 0)
					|| (os_memcmp(wapp->dpp->bss_config[i].mac, wapp->map->ctrl_alid, ETH_ALEN) == 0))) {
					dpp_map_create_auth(&wapp->dpp->bss_config[i], conf);

					if (dpp_akm_dpp(conf->akm) || auth->peer_version >= 2)
					msg1 = dpp_build_conf_obj_dpp(auth, MAP_BACKHAUL_STA, conf);
					else
					msg1 = dpp_build_conf_obj_legacy(auth, MAP_BACKHAUL_STA, conf);
					conf_map[id_map] = msg1;
					id_map++;
					DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX" wapp->dpp->bss_config[%d].operating_chan =%d, config num =%d\n",
							i,wapp->dpp->bss_config[i].operating_chan, i);
					/* Come out from for loop when one backhaul config is found */
					break;
				}
				else{
					DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"mac addr does not match with proxy almac\n");
				}
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"mac addr not given in wts file\n");
#if 0
				if ((wapp->dpp->bss_config[i].operating_chan == operating_chan)
#endif
				if((wapp->dpp->bss_config[i].wfa_vendor_extension & BIT_BH_BSS)) {
					dpp_map_create_auth(&wapp->dpp->bss_config[i], conf);

					if (dpp_akm_dpp(conf->akm) || auth->peer_version >= 2)
					msg1 = dpp_build_conf_obj_dpp(auth, MAP_BACKHAUL_STA, conf);
					else
					msg1 = dpp_build_conf_obj_legacy(auth, MAP_BACKHAUL_STA, conf);
					conf_map[id_map] = msg1;
					id_map++;
					DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX" wapp->dpp->bss_config[%d].operating_chan =%d, config num =%d\n",
							i,wapp->dpp->bss_config[i].operating_chan, i);
					/* Come out from for loop when one backhaul config is found */
					break;
				}
			}
		}
		if (i == wapp->dpp->bss_config_num) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Max number %u of BSS checked in WTS\n",wapp->dpp->bss_config_num);
			if (id_map != 0) {
					for (i = 0; i < id_map; i++) {
					wpabuf_free(conf_map[i]);
				}
				*conf_count = 0;
			}
			if (conf->group_id) {
				os_free(conf->group_id);
				conf->group_id = NULL;
			}
			if (conf->passphrase) {
				os_free(conf->passphrase);
				conf->passphrase = NULL;
			}
			free(conf);
			conf = NULL;
			return;
		}
	}
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"number of object in the conf obj:%u\n",id_map);
	*conf_count = id_map;
#if 0 //NXP
	dpp_map_create_auth(NULL, conf);
	if(self_sign)
		msg1 = dpp_build_conf_obj_dpp(auth, MAP_1905_CONT, conf);
	else {
#ifdef MAP_R3_RECONFIG
		if (auth->reconfigTrigger != TRUE)
#endif
			msg1 = dpp_build_conf_obj_dpp(auth, MAP_1905, conf);
#ifdef MAP_R3_RECONFIG
		else if (auth->reconfigTrigger==TRUE && b1905Config)
			msg1 = dpp_build_conf_obj_dpp(auth, MAP_1905, conf);
#endif
	}	

#ifdef MAP_R3_RECONFIG
	if (auth->reconfigTrigger != TRUE ||
		(auth->reconfigTrigger == TRUE && b1905Config)) {
#endif
		conf_map[*conf_count] = msg1;
		(*conf_count)++;
#ifdef MAP_R3_RECONFIG
	}
#endif
#endif
#endif /* MAP_R3 */
	if(conf->group_id)
		os_free(conf->group_id);
	if(conf->passphrase)
		os_free(conf->passphrase);
	os_free(conf);
}
#endif /* DPP_R2_MUOBJ */


static struct wpabuf *
dpp_build_conf_resp(struct dpp_authentication *auth, const u8 *e_nonce,
		    u16 e_nonce_len, int ap, int is_map
#ifdef MAP_R3
			,struct json_token *akm, struct json_token *channel, u8 bsta_present
#ifdef MAP_R3_RECONFIG
			, u8 b1905Config, u8 bSTAConfig
#endif /* MAP_R3 */			
#endif
)
{
	struct wpabuf *conf[20] = {0};
	int conf_count = 0, i;
	size_t clear_len, attr_len;
	struct wpabuf *clear = NULL, *msg = NULL;
	u8 *wrapped;
	const u8 *addr[1];
	size_t len[1];
	enum dpp_status_error status;

	dpp_build_conf_obj(auth, ap, is_map, &conf_count, conf
#ifdef MAP_R3
		, akm, channel, bsta_present, 0
#ifdef MAP_R3_RECONFIG
		, b1905Config, bSTAConfig
#endif
#endif /* MAP_R3*/
	);
	if (conf_count) {
		for (i = 0; i < conf_count; i++)
		wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: configurationObject JSON",
				  wpabuf_head(conf[i]), wpabuf_len(conf[i]));
	}
	status = conf_count ? DPP_STATUS_OK : DPP_STATUS_CONFIGURE_FAILURE;
	auth->conf_resp_status = status;

	/* { E-nonce, configurationObject}ke */
	clear_len = 4 + e_nonce_len;
	if (conf_count) {
		for (i = 0; i < conf_count; i++)
			clear_len += 4 + wpabuf_len(conf[i]);
	}
#ifdef MAP_R3
	if (auth->peer_bi)
               os_memset(auth->peer_bi->mac_addr, 0, ETH_ALEN);
#endif
#ifdef CONFIG_DPP2
	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: sendConnStatus peer_version:%d ", auth->peer_version);
	if ((auth->peer_version >= 2) && (ap == INFRA_STA)
#ifdef MAP_R3
	&& (auth->ethernetTrigger == FALSE)
#endif /* MAP_R3 */
       )
		auth->send_conn_status = 1;
	if (auth->peer_version >= 2 && ap == INFRA_STA && auth->send_conn_status)
		clear_len += 4;
#endif /* CONFIG_DPP2 */

	clear = wpabuf_alloc(clear_len);
	attr_len = 4 + 1 + 4 + clear_len + AES_BLOCK_SIZE;
	msg = wpabuf_alloc(attr_len);
	if (!clear || !msg)
		goto fail;


	/* E-nonce */
	wpabuf_put_le16(clear, DPP_ATTR_ENROLLEE_NONCE);
	wpabuf_put_le16(clear, e_nonce_len);
	wpabuf_put_data(clear, e_nonce, e_nonce_len);

	if (conf_count) {
		for (i = 0; i < conf_count; i++) {
		wpabuf_put_le16(clear, DPP_ATTR_CONFIG_OBJ);
		wpabuf_put_le16(clear, wpabuf_len(conf[i]));
		wpabuf_put_buf(clear, conf[i]);
		}
	}

#ifdef CONFIG_DPP2
	if (auth->peer_version >= 2 && ap == INFRA_STA && auth->send_conn_status) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: sendConnStatus");
		wpabuf_put_le16(clear, DPP_ATTR_SEND_CONN_STATUS);
		wpabuf_put_le16(clear, 0);
	}
#endif /* CONFIG_DPP2 */

	/* DPP Status */
	dpp_build_attr_status(msg, status);

	addr[0] = wpabuf_head(msg);
	len[0] = wpabuf_len(msg);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD", addr[0], len[0]);

	wpabuf_put_le16(msg, DPP_ATTR_WRAPPED_DATA);
	wpabuf_put_le16(msg, wpabuf_len(clear) + AES_BLOCK_SIZE);
	wrapped = wpabuf_put(msg, wpabuf_len(clear) + AES_BLOCK_SIZE);

	wpa_hexdump_buf(MSG_DEBUG, "DPP: AES-SIV cleartext", clear);
	if (aes_siv_encrypt(auth->ke, auth->curve->hash_len,
			    wpabuf_head(clear), wpabuf_len(clear),
			    1, addr, len, wrapped) < 0)
		goto fail;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped, wpabuf_len(clear) + AES_BLOCK_SIZE);

	wpa_hexdump_buf(MSG_DEBUG,
			"DPP: Configuration Response attributes", msg);
out:
	for (i = 0; i < conf_count; i++) {
		wpabuf_free(conf[i]);
	}
	wpabuf_free(clear);

	return msg;
fail:
	wpabuf_free(msg);
	msg = NULL;
	goto out;
}

#if 0
int get_cmdu_tlv_length(unsigned char *buf)
{
	unsigned char *temp_buf = buf;
	int length;

	temp_buf += 1;          //shift to length field

	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));

	return (length + 3);
}
#define MAP_VERSION2 2
#define MAP_VERSION3 3
int parse_map_verson_tlv(unsigned char *buf, struct dpp_authentication *auth)
{
        unsigned char *temp_buf;
        unsigned short length = 0;

        temp_buf = buf;

	DBGPRINT(RT_DEBUG_ERROR,"%s: %d\n", __func__, __LINE__);
        if((*temp_buf) == MULTI_AP_VERSION_TYPE) {
                temp_buf++;
        } else {
                DBGPRINT(RT_DEBUG_ERROR,"should not go here\n");
                return -1;
        }

        //calculate tlv length
        length = (*temp_buf);
        length = (length << 8) & 0xFF00;
        length = length |(*(temp_buf+1));

        //shift to tlv value field
        temp_buf += 2;

	if (*temp_buf != MAP_VERSION3)
	{
		DBGPRINT(RT_DEBUG_ERROR,"error, version is %d\n", *temp_buf);
	}

	return 0;
}
int parse_map_akm_suite_tlv(unsigned char *buf, struct dpp_authentication *auth)
{
        unsigned char *temp_buf;
        unsigned short length = 0;

        temp_buf = buf;

        if((*temp_buf) == AKM_SUITE_TLV_TYPE) {
                temp_buf++;
        } else {
                DBGPRINT(RT_DEBUG_ERROR,"should not go here\n");
                return -1;
        }

        //calculate tlv length
        length = (*temp_buf);
        length = (length << 8) & 0xFF00;
        length = length |(*(temp_buf+1));

        //shift to tlv value field
        temp_buf += 2;

	// TODO later
	return 0;
}
int parse_supported_service_tlv(unsigned char *buf, struct dpp_authentication *auth)
{
	return 0;
}
int parse_backhaul_station_radio_cap_tlv(unsigned char *buf, struct dpp_authentication *auth)
{
        unsigned char *temp_buf;
        unsigned short length = 0, i;

        temp_buf = buf;

        if((*temp_buf) == BACKHAUL_STATION_RADIO_CAP_TYPE) {
                temp_buf++;
        } else {
                DBGPRINT(RT_DEBUG_ERROR,"should not go here\n");
                return -1;
        }

        //calculate tlv length
        length = (*temp_buf);
        length = (length << 8) & 0xFF00;
        length = length |(*(temp_buf+1));

        //shift to tlv value field
        temp_buf += 2;

	for (i = 0; i < 3; i++) {
		if (os_memcmp(auth->radio[i].identifier, temp_buf, ETH_ALEN) == 0)
			auth->radio[i].is_bh_sta_supported = 1;
	}
        temp_buf += ETH_ALEN;
	// sta mac address later
	return 0;
}
int parse_r2_cap_tlv(unsigned char *buf, struct dpp_authentication *auth)
{
	//TODO later
	return 0;
}
int parse_ap_radio_advance_cap_tlv(unsigned char *buf, struct dpp_authentication *auth)
{
	// TODO later
	return 0;
}
int parse_basic_radio_cap_tlv(unsigned char *buf, struct dpp_authentication *auth)
{
        unsigned char *temp_buf;
        unsigned short length = 0;
        int i;
	u8 zero_mac[6] = {0,0,0,0,0,0};
	struct peer_radio_info *radio = NULL;
	int max_op_class = 0, op_class_num, non_operch_num;

        temp_buf = buf;

        if((*temp_buf) == AP_RADIO_BASIC_CAPABILITY_TYPE) {
                temp_buf++;
        } else {
                DBGPRINT(RT_DEBUG_ERROR,"should not go here\n");
                return -1;
        }

        //calculate tlv length
        length = (*temp_buf);
        length = (length << 8) & 0xFF00;
        length = length |(*(temp_buf+1));

        //shift to tlv value field
        temp_buf += 2;

	for (i = 0; i < 3; i++) {
		if (memcmp(auth->radio[i].identifier, zero_mac, 6) == 0) {
			radio = &auth->radio[i];
			memcpy(radio->identifier,temp_buf, ETH_ALEN); 
        		temp_buf += ETH_ALEN;
			break;
		}
	}

	if (radio == NULL)
		return -1;

        radio->max_bss = *temp_buf;
        temp_buf++;
        op_class_num = *temp_buf;
        temp_buf++;
        for (i = 0; i < op_class_num; i++) {
                max_op_class = max_op_class < *temp_buf ? *temp_buf: max_op_class;
                temp_buf++;
		// tx power
                temp_buf++;
		// non oper
                non_operch_num = *temp_buf;
                temp_buf++;
		/*list */
                temp_buf += non_operch_num;
        }
	if (max_op_class <= 84) {
		radio->operating_chan = RADIO_24G;
	} else if (max_op_class <= 120)
		radio->operating_chan = RADIO_5GL;
	else if (max_op_class <= 129)
		radio->operating_chan = RADIO_5GH;
	else
		radio->operating_chan = RADIO_24G;

	return 0;
}

static void dpp_parse_map_tlv(struct dpp_authentication *auth, struct wpabuf *map_tlv)
{
	unsigned char *buf = map_tlv->buf;
	int length;
	map_tlv->used = 0;

	DBGPRINT(RT_DEBUG_ERROR,"%s: %d\n", __func__, __LINE__);
	while (1) {
		if (map_tlv->used == map_tlv->size)
			break;

		if (*buf == SUPPORTED_SERVICE_TLV_TYPE) {
			parse_supported_service_tlv(buf, auth);
                        length = get_cmdu_tlv_length(buf);
                        buf += length;
			map_tlv->used += length;
		} else if (*buf == MULTI_AP_VERSION_TYPE) {
			parse_map_verson_tlv(buf, auth);
                        length = get_cmdu_tlv_length(buf);
                        buf += length;
			map_tlv->used += length;
		} else if (*buf == AKM_SUITE_TLV_TYPE) {
			parse_map_akm_suite_tlv(buf, auth);
                        length = get_cmdu_tlv_length(buf);
                        buf += length;
			map_tlv->used += length;
		} else if (*buf == AP_RADIO_BASIC_CAPABILITY_TYPE) {
			parse_basic_radio_cap_tlv(buf, auth);
                        length = get_cmdu_tlv_length(buf);
                        buf += length;
			map_tlv->used += length;
		} else if (*buf == BACKHAUL_STATION_RADIO_CAP_TYPE) {
			parse_backhaul_station_radio_cap_tlv(buf, auth);
                        length = get_cmdu_tlv_length(buf);
                        buf += length;
			map_tlv->used += length;
		} else if (*buf == R2_CAP_TLV_TYPE) {
			parse_r2_cap_tlv(buf, auth);
                        length = get_cmdu_tlv_length(buf);
                        buf += length;
			map_tlv->used += length;
		} else if (*buf == AP_RADIO_ADVANCE_CAP_TLV) {
			parse_ap_radio_advance_cap_tlv(buf, auth);
                        length = get_cmdu_tlv_length(buf);
                        buf += length;
			map_tlv->used += length;
		} else {
			DBGPRINT(RT_DEBUG_ERROR,"invalid tlv in config request, ignorning\n");
                        length = get_cmdu_tlv_length(buf);
                        buf += length;
			map_tlv->used += length;
		}
	}
}
#endif

struct wpabuf *
dpp_conf_req_rx(struct dpp_authentication *auth, const u8 *attr_start,
		size_t attr_len)
{
	const u8 *wrapped_data, *e_nonce, *config_attr;
	u16 wrapped_data_len, e_nonce_len, config_attr_len;
	u8 *unwrapped = NULL;
	size_t unwrapped_len = 0;
	struct wpabuf *resp = NULL;
	struct json_token *root = NULL, *token;
#ifdef MAP_R3
	struct json_token *bsta;
	struct json_token *netrole;
	struct json_token *akm = NULL;
	struct json_token *channel = NULL;
	u8 bsta_present = 0;
#ifdef MAP_R3_RECONFIG
	 u8 bSTAConfig=0,b1905Config=0;
#endif
#endif /* MAP_R3 */
	int ap= 0, map = 0;
	//struct wpabuf *map_tlv = NULL;

	if (dpp_check_attrs(attr_start, attr_len) < 0) {
		dpp_auth_fail(auth, "Invalid attribute in config request");
		return NULL;
	}

	wrapped_data = dpp_get_attr(attr_start, attr_len, DPP_ATTR_WRAPPED_DATA,
				    &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		dpp_auth_fail(auth,
			      "Missing or invalid required Wrapped Data attribute");
		return NULL;
	}

	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped_data, wrapped_data_len);
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped = os_malloc(unwrapped_len);
	if (!unwrapped)
		return NULL;
	if (aes_siv_decrypt(auth->ke, auth->curve->hash_len,
			    wrapped_data, wrapped_data_len,
			    0, NULL, NULL, unwrapped) < 0) {
		dpp_auth_fail(auth, "AES-SIV decryption failed");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext",
		    unwrapped, unwrapped_len);

	if (dpp_check_attrs(unwrapped, unwrapped_len) < 0) {
		dpp_auth_fail(auth, "Invalid attribute in unwrapped data");
		goto fail;
	}

	e_nonce = dpp_get_attr(unwrapped, unwrapped_len,
			       DPP_ATTR_ENROLLEE_NONCE,
			       &e_nonce_len);
	if (!e_nonce || e_nonce_len != auth->curve->nonce_len) {
		dpp_auth_fail(auth,
			      "Missing or invalid Enrollee Nonce attribute");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Enrollee Nonce", e_nonce, e_nonce_len);
	os_memcpy(auth->e_nonce, e_nonce, e_nonce_len);

	config_attr = dpp_get_attr(unwrapped, unwrapped_len,
				   DPP_ATTR_CONFIG_ATTR_OBJ,
				   &config_attr_len);
	if (!config_attr) {
		dpp_auth_fail(auth,
			      "Missing or invalid Config Attributes attribute");
		goto fail;
	}
	wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: Config Attributes",
			  config_attr, config_attr_len);

	root = json_parse((const char *) config_attr, config_attr_len);
	if (!root) {
		dpp_auth_fail(auth, "Could not parse Config Attributes");
		goto fail;
	}

	token = json_get_member(root, "name");
	if (!token || token->type != JSON_STRING) {
		dpp_auth_fail(auth, "No Config Attributes - name");
		goto fail;
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Enrollee name = '%s'", token->string);

	token = json_get_member(root, "wi-fi_tech");
	if (!token || token->type != JSON_STRING) {
		dpp_auth_fail(auth, "No Config Attributes - wi-fi_tech");
		goto fail;
	}
	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: wi-fi_tech = '%s'", token->string);
#if 0
	if (os_strcmp(token->string, "infra") != 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unsupported wi-fi_tech '%s'",
			   token->string);
		dpp_auth_fail(auth, "Unsupported wi-fi_tech");
		goto fail;
	}
#else
	if ((os_strcmp(token->string, "infra") != 0) &&
			(os_strcmp(token->string, "map") != 0) &&
			(os_strcmp(token->string, "inframap") != 0)) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unsupported wi-fi_tech '%s'",
				token->string);
		dpp_auth_fail(auth, "Unsupported wi-fi_tech");
		goto fail;
	}
#endif

#ifdef MAP_R3
	if (os_strcmp(token->string, "map") == 0) {
		//Setting the map flag if the wifi-tech is set to map
		map = 1;
	}
#endif /* MAP_R3 */
	token = json_get_member(root, "netRole");
	if (!token || token->type != JSON_STRING) {
#ifdef MAP_R3_RECONFIG
		if (auth->reconfigTrigger != TRUE)
#endif
		{
			dpp_auth_fail(auth, "No Config Attributes - netRole");
			goto fail;
		}
	}
#ifdef MAP_R3_RECONFIG
	if (token)
	{
#endif
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: netRole = '%s'", token->string);
		if (os_strcmp(token->string, "sta") == 0) {
			ap = 0;
		} else if (os_strcmp(token->string, "ap") == 0) {
			ap = 1;
		} else if (os_strcmp(token->string, "mapAgent") == 0) {
			DBGPRINT(RT_DEBUG_INFO, DPP_MAP_PREX"netrole is set for MAP\n");
		} else {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unsupported netRole '%s'",
				   token->string);
#ifdef MAP_R3_RECONFIG
			if (auth->reconfigTrigger != TRUE)
#endif
			{
				dpp_auth_fail(auth, "Unsupported netRole");
				goto fail;
			}
		}
#ifdef MAP_R3_RECONFIG
	}
#endif

#ifdef MAP_R3
	if(!map) {
		//handling Standalone DPP support from this
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"map flag is not set.\n");
		goto skip_bsta;
	}

	if (map && token && (os_strcmp(token->string, "mapAgent") == 0)) {
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"map flag is set.\n");
#ifdef MAP_R3_RECONFIG
		b1905Config = 1;
#endif
	}
#ifdef MAP_R3_RECONFIG
	else if (auth->reconfigTrigger == TRUE) {
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"map flag is set.\n");
	}
#endif
	else {
		dpp_auth_fail(auth, "Invalid Config Attributes - netRole or wifi_tech");
		goto fail;
	}

	bsta = json_get_member(root, "bSTAList");
	if (!bsta || bsta->type != JSON_ARRAY) {
		dpp_auth_fail(auth, "No Config Attributes - bSTAList");
	}
	else{
		bsta_present = 1;
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX" bsta_object_present [%s] \n", __func__);
	}

	if (bsta_present){	
		for (token = bsta->child; token; token = token->sibling) {

			netrole = json_get_member(token, "netRole");
			if (!netrole || netrole->type != JSON_STRING){
				dpp_auth_fail(auth, "No Config Attributes - netrole");
				goto fail;
			}

			//if (os_strcmp(netrole->string, "mapbackhaulSta") == 0) {
			if (os_strcasecmp(netrole->string, "mapbackhaulSta") == 0) {
				wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Correct netRole in object '%s'",
						netrole->string);
			} else {
				wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unsupported netRole '%s'",
						netrole->string);
				dpp_auth_fail(auth, "Unsupported netRole");
				goto fail;
			}

			akm = json_get_member(token, "akm");
			if (!akm || akm->type != JSON_STRING) {
				dpp_auth_fail(auth, "No Config Attributes - akm");
				goto fail;
			}

			channel = json_get_member(token, "channelList");
			if (!channel || channel->type != JSON_STRING) {
				dpp_auth_fail(auth, "No Config Attributes - channellist");
				goto fail;
			}
		}
	}

skip_bsta:
#endif /* MAP_R3*/

#ifdef DPP_R2_SUPPORT
	if(!map) {
		token = json_get_member(root, "mudurl");
		if (!token || token->type != JSON_STRING) {
			dpp_auth_fail(auth, "No Config Attributes - mudurl");
			goto fail;
		}
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: mudurl = %s", token->string);
		if (token->string) {
			FILE *file = NULL;

			file = fopen(DPP_MUD_URL_FILE, "w");
			if (!file) {
				wpa_printf(MSG_ERROR, DPP_MAP_PREX "open DPP_MUD_URL_FILE fail");
				goto fail;
			} else {
				fprintf(file, "Peer_MUD_URL=%s\n", token->string);
				fclose(file);
			}
		}
	}
#endif /* DPP_R2_SUPPORT */

#if 0
	if (0) {
		/* read map tlv */
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: mapTLVBlob = '%s'", token->string);
		map_tlv = json_get_member_base64url(root, "mapTLVBlob");
		if (!map_tlv) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No map_tlv string value found");
			goto fail;
		}
		wpa_hexdump_buf(MSG_DEBUG, "DPP: JWS Protected Header map_tlv (decoded)",
				map_tlv);

		dpp_parse_map_tlv(auth, map_tlv);
	}
#endif
	resp = dpp_build_conf_resp(auth, e_nonce, e_nonce_len, ap, map
#ifdef MAP_R3
		, akm, channel, bsta_present
#ifdef MAP_R3_RECONFIG
		, b1905Config, bSTAConfig
#endif
#endif /* MAP_R3*/	
	);

fail:
	json_free(root);
	os_free(unwrapped);
	return resp;
}


static struct wpabuf *
dpp_parse_jws_prot_hdr(const struct dpp_curve_params *curve,
		       const u8 *prot_hdr, u16 prot_hdr_len,
		       const EVP_MD **ret_md)
{
	struct json_token *root, *token;
	struct wpabuf *kid = NULL;

	root = json_parse((const char *) prot_hdr, prot_hdr_len);
	if (!root) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: JSON parsing failed for JWS Protected Header");
		goto fail;
	}

	if (root->type != JSON_OBJECT) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: JWS Protected Header root is not an object");
		goto fail;
	}

	token = json_get_member(root, "typ");
	if (!token || token->type != JSON_STRING) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No typ string value found");
		goto fail;
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: JWS Protected Header typ=%s",
		   token->string);
	if (os_strcmp(token->string, "dppCon") != 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Unsupported JWS Protected Header typ=%s",
			   token->string);
		goto fail;
	}

	token = json_get_member(root, "alg");
	if (!token || token->type != JSON_STRING) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No alg string value found");
		goto fail;
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: JWS Protected Header alg=%s",
		   token->string);
	if (os_strcmp(token->string, curve->jws_alg) != 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Unexpected JWS Protected Header alg=%s (expected %s based on C-sign-key)",
			   token->string, curve->jws_alg);
		goto fail;
	}
	if (os_strcmp(token->string, "ES256") == 0 ||
	    os_strcmp(token->string, "BS256") == 0)
		*ret_md = EVP_sha256();
	else if (os_strcmp(token->string, "ES384") == 0 ||
		 os_strcmp(token->string, "BS384") == 0)
		*ret_md = EVP_sha384();
	else if (os_strcmp(token->string, "ES512") == 0 ||
		 os_strcmp(token->string, "BS512") == 0)
		*ret_md = EVP_sha512();
	else
		*ret_md = NULL;
	if (!*ret_md) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Unsupported JWS Protected Header alg=%s",
			   token->string);
		goto fail;
	}

	kid = json_get_member_base64url(root, "kid");
	if (!kid) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No kid string value found");
		goto fail;
	}
	wpa_hexdump_buf(MSG_DEBUG, "DPP: JWS Protected Header kid (decoded)",
			kid);

fail:
	json_free(root);
	return kid;
}

#ifdef DPP_R2_MUOBJ
static int dpp_parse_cred_legacy(struct dpp_config_obj *conf,
				 struct json_token *cred)
{
	struct json_token *pass, *psk_hex;

	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Legacy akm=psk credential");

	pass = json_get_member(cred, "pass");
	psk_hex = json_get_member(cred, "psk_hex");

	if (pass && pass->type == JSON_STRING) {
		size_t len = os_strlen(pass->string);

		wpa_hexdump_ascii_key(MSG_INFO1, DPP_MAP_PREX"DPP: Legacy passphrase",
				      pass->string, len);
		if (len < 8 || len > 63)
			return -1;
		os_strlcpy(conf->passphrase, pass->string,
			   sizeof(conf->passphrase));
	} else if (psk_hex && psk_hex->type == JSON_STRING) {
		if (dpp_akm_sae(conf->akm) && !dpp_akm_psk(conf->akm)) {
			wpa_printf(MSG_INFO1, DPP_MAP_PREX
				   "DPP: Unexpected psk_hex with akm=sae");
			return -1;
		}
		if (os_strlen(psk_hex->string) != PMK_LEN * 2 ||
		    hexstr2bin(psk_hex->string, conf->psk, PMK_LEN) < 0) {
			wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Invalid psk_hex encoding");
			return -1;
		}
		wpa_hexdump_key(MSG_INFO1, DPP_MAP_PREX "DPP: Legacy PSK",
				conf->psk, PMK_LEN);
		conf->psk_set = 1;
	} else {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: No pass or psk_hex strings found");
		return -1;
	}

	if (dpp_akm_sae(conf->akm) && !conf->passphrase[0]) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: No pass for sae found");
		return -1;
	}
	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Legacy akm=psk credential done");
	return 0;
}

#else

static int dpp_parse_cred_legacy(struct dpp_authentication *auth,
				 struct json_token *cred)
{
	struct json_token *pass, *psk_hex;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Legacy akm=psk credential");

	pass = json_get_member(cred, "pass");
	psk_hex = json_get_member(cred, "psk_hex");

	if (pass && pass->type == JSON_STRING) {
		size_t len = os_strlen(pass->string);

		wpa_hexdump_ascii_key(MSG_DEBUG, DPP_MAP_PREX "DPP: Legacy passphrase",
				      pass->string, len);
		if (len < 8 || len > 63)
			return -1;
		os_strlcpy(auth->passphrase, pass->string,
			   sizeof(auth->passphrase));
	} else if (psk_hex && psk_hex->type == JSON_STRING) {
		if (dpp_akm_sae(auth->akm) && !dpp_akm_psk(auth->akm)) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Unexpected psk_hex with akm=sae");
			return -1;
		}
		if (os_strlen(psk_hex->string) != PMK_LEN * 2 ||
		    hexstr2bin(psk_hex->string, auth->psk, PMK_LEN) < 0) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Invalid psk_hex encoding");
			return -1;
		}
		wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: Legacy PSK",
				auth->psk, PMK_LEN);
		auth->psk_set = 1;
	} else {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No pass or psk_hex strings found");
		return -1;
	}

	if (dpp_akm_sae(auth->akm) && !auth->passphrase[0]) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No pass for sae found");
		return -1;
	}

	return 0;
}
#endif /* DPP_R2_MUOBJ */

EVP_PKEY * dpp_parse_jwk(struct json_token *jwk, const struct dpp_curve_params **key_curve)
{
	struct json_token *token;
	const struct dpp_curve_params *curve;
	struct wpabuf *x = NULL, *y = NULL;
	EC_GROUP *group;
	EVP_PKEY *pkey = NULL;

	token = json_get_member(jwk, "kty");
	if (!token || token->type != JSON_STRING) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No kty in JWK");
		goto fail;
	}
	if (os_strcmp(token->string, "EC") != 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unexpected JWK kty '%s'",
			   token->string);
		goto fail;
	}

	token = json_get_member(jwk, "crv");
	if (!token || token->type != JSON_STRING) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No crv in JWK");
		goto fail;
	}
	curve = dpp_get_curve_jwk_crv(token->string);
	if (!curve) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unsupported JWK crv '%s'",
			   token->string);
		goto fail;
	}

	x = json_get_member_base64url(jwk, "x");
	if (!x) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No x in JWK");
		goto fail;
	}
	wpa_hexdump_buf(MSG_DEBUG, "DPP: JWK x", x);
	if (wpabuf_len(x) != curve->prime_len) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Unexpected JWK x length %u (expected %u for curve %s)",
			   (unsigned int) wpabuf_len(x),
			   (unsigned int) curve->prime_len, curve->name);
		goto fail;
	}

	y = json_get_member_base64url(jwk, "y");
	if (!y) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No y in JWK");
		goto fail;
	}
	wpa_hexdump_buf(MSG_DEBUG, "DPP: JWK y", y);
	if (wpabuf_len(y) != curve->prime_len) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Unexpected JWK y length %u (expected %u for curve %s)",
			   (unsigned int) wpabuf_len(y),
			   (unsigned int) curve->prime_len, curve->name);
		goto fail;
	}

	group = EC_GROUP_new_by_curve_name(OBJ_txt2nid(curve->name));
	if (!group) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Could not prepare group for JWK");
		goto fail;
	}

	pkey = dpp_set_pubkey_point_group(group, wpabuf_head(x), wpabuf_head(y),
					  wpabuf_len(x));
	*key_curve = curve;

fail:
	wpabuf_free(x);
	wpabuf_free(y);

	return pkey;
}


int dpp_key_expired(const char *timestamp, os_time_t *expiry)
{
	struct os_time now;
	unsigned int year, month, day, hour, min, sec;
	os_time_t utime;
	const char *pos;

	/* ISO 8601 date and time:
	 * <date>T<time>
	 * YYYY-MM-DDTHH:MM:SSZ
	 * YYYY-MM-DDTHH:MM:SS+03:00
	 */
	if (os_strlen(timestamp) < 19) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Too short timestamp - assume expired key");
		return 1;
	}
	if (sscanf(timestamp, "%04u-%02u-%02uT%02u:%02u:%02u",
		   &year, &month, &day, &hour, &min, &sec) != 6) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Failed to parse expiration day - assume expired key");
		return 1;
	}

	if (os_mktime(year, month, day, hour, min, sec, &utime) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Invalid date/time information - assume expired key");
		return 1;
	}

	pos = timestamp + 19;
	if (*pos == 'Z' || *pos == '\0') {
		/* In UTC - no need to adjust */
	} else if (*pos == '-' || *pos == '+') {
		int items;

		/* Adjust local time to UTC */
		items = sscanf(pos + 1, "%02u:%02u", &hour, &min);
		if (items < 1) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Invalid time zone designator (%s) - assume expired key",
				   pos);
			return 1;
		}
		if (*pos == '-')
			utime += 3600 * hour;
		if (*pos == '+')
			utime -= 3600 * hour;
		if (items > 1) {
			if (*pos == '-')
				utime += 60 * min;
			if (*pos == '+')
				utime -= 60 * min;
		}
	} else {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Invalid time zone designator (%s) - assume expired key",
			   pos);
		return 1;
	}
	if (expiry)
		*expiry = utime;

	if (os_get_time(&now) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Cannot get current time - assume expired key");
		return 1;
	}

	if (now.sec > utime) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Key has expired (%lu < %lu)",
			   utime, now.sec);
		return 1;
	}

	return 0;
}


static int dpp_parse_connector(struct dpp_authentication *auth,
			       const unsigned char *payload,
			       u16 payload_len)
{
	struct json_token *root, *groups, *netkey, *token;
	int ret = -1;
	EVP_PKEY *key = NULL;
	const struct dpp_curve_params *curve;
	unsigned int rules = 0;

	root = json_parse((const char *) payload, payload_len);
	if (!root) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: JSON parsing of connector failed");
		goto fail;
	}

	groups = json_get_member(root, "groups");
	if (!groups || groups->type != JSON_ARRAY) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No groups array found");
		goto skip_groups;
	}
	for (token = groups->child; token; token = token->sibling) {
		struct json_token *id, *role;

		id = json_get_member(token, "groupId");
		if (!id || id->type != JSON_STRING) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Missing groupId string");
			goto fail;
		}

		role = json_get_member(token, "netRole");
		if (!role || role->type != JSON_STRING) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Missing netRole string");
			goto fail;
		}
#ifdef MAP_R3
		if ((os_strcasecmp(role->string, "mapAgent") == 0)
			|| (os_strcasecmp(role->string, "mapController") == 0)){
			DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"setting 1905 connector flag here..\n");
			auth->is_1905_connector = 1;
		}
#endif /* MAP_R3 */
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: connector group: groupId='%s' netRole='%s'",
			   id->string, role->string);
		rules++;
	}
skip_groups:

	if (!rules) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Connector includes no groups");
		goto fail;
	}

	token = json_get_member(root, "expiry");
	if (!token || token->type != JSON_STRING) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: No expiry string found - connector does not expire");
	} else {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: expiry = %s", token->string);
		if (dpp_key_expired(token->string,
				    &auth->net_access_key_expiry)) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Connector (netAccessKey) has expired");
			goto fail;
		}
	}

#ifdef MAP_R3
	if(!auth->bss_conf) {
#endif  /* MAP_R3 */
		netkey = json_get_member(root, "netAccessKey");
		if (!netkey || netkey->type != JSON_OBJECT) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No netAccessKey object found");
			goto fail;
		}

		key = dpp_parse_jwk(netkey, &curve);
		if (!key)
			goto fail;
		dpp_debug_print_key("DPP: Received netAccessKey", key);

		if (EVP_PKEY_cmp(key, auth->own_protocol_key) != 1) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
					"DPP: netAccessKey in connector does not match own protocol key");
			goto fail;
		}
#ifdef MAP_R3
	} else {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX
				"DPP: Ignore net acesskey key check for BSS configurations");
	}
#endif  /* MAP_R3 */

	ret = 0;
fail:
	EVP_PKEY_free(key);
	json_free(root);
	return ret;
}


static int dpp_check_pubkey_match(EVP_PKEY *pub, struct wpabuf *r_hash)
{
	struct wpabuf *uncomp;
	int res;
	u8 hash[SHA256_MAC_LEN];
	const u8 *addr[1];
	size_t len[1];

	if (wpabuf_len(r_hash) != SHA256_MAC_LEN)
		return -1;
	uncomp = dpp_get_pubkey_point(pub, 1);
	if (!uncomp)
		return -1;
	addr[0] = wpabuf_head(uncomp);
	len[0] = wpabuf_len(uncomp);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Uncompressed public key",
		    addr[0], len[0]);
	res = sha256_vector(1, addr, len, hash);
	wpabuf_free(uncomp);
	if (res < 0)
		return -1;
	if (os_memcmp(hash, wpabuf_head(r_hash), SHA256_MAC_LEN) != 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Received hash value does not match calculated public key hash value");
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Calculated hash",
			    hash, SHA256_MAC_LEN);
		return -1;
	}
	return 0;
}


static void dpp_copy_csign(struct dpp_authentication *auth, EVP_PKEY *csign)
{
	unsigned char *der = NULL;
	int der_len;

	der_len = i2d_PUBKEY(csign, &der);
	if (der_len <= 0)
		return;
	wpabuf_free(auth->c_sign_key);
	auth->c_sign_key = wpabuf_alloc_copy(der, der_len);
	OPENSSL_free(der);
}

#ifdef MAP_R3
enum dpp_akm dpp_map_r3_akm_from_inter_val(unsigned short akm_val)
{
	if(akm_val == (AUTH_DPP_ONLY | WPS_AUTH_SAE | WPS_AUTH_WPA2PSK))
		return DPP_AKM_PSK_SAE_DPP;
	else if (akm_val == (AUTH_DPP_ONLY | WPS_AUTH_SAE))
		return DPP_AKM_SAE_DPP;
	else if (akm_val == (AUTH_DPP_ONLY | WPS_AUTH_WPA2PSK))
		return DPP_AKM_PSK_DPP;
	else if (akm_val == (WPS_AUTH_SAE | WPS_AUTH_WPA2PSK))
		return DPP_AKM_PSK_SAE;
	else if (akm_val == AUTH_DPP_ONLY)
		return DPP_AKM_DPP;
	else if (akm_val == WPS_AUTH_SAE)
		return DPP_AKM_SAE;
	else if (akm_val == WPS_AUTH_WPA2PSK)
		return DPP_AKM_PSK;
	return DPP_AKM_UNKNOWN;
}

static void dpp_map_copy_csign(struct dpp_config *wdev_config, EVP_PKEY *csign)
{
	unsigned char *der = NULL;
	int der_len;

	der_len = i2d_PUBKEY(csign, &der);
	if (der_len <= 0)
		return;
	if(wdev_config->dpp_connector)
		os_free(wdev_config->dpp_connector);
	wdev_config->dpp_csign = os_malloc((size_t)der_len);
	if (!wdev_config->dpp_csign) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"C-sign Malloc failed\n");
		OPENSSL_free(der);
		return;
	}
	os_memcpy(wdev_config->dpp_csign, der, der_len);
	wdev_config->dpp_csign_len = (size_t)der_len;
	DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"dpp c-sign key copied: %d\n",(int)wdev_config->dpp_csign_len);

	OPENSSL_free(der);
}

static int dpp_map_copy_netaccess_frm_cfg(struct wifi_app *wapp, struct dpp_config *wdev_config)
{
	char param[64] = {0};
	char value[512] = {0};
	int ret;
	unsigned char buf_temp[1024] = {0};

	ret = os_snprintf(param, sizeof(param), "_1905valid");
	if (os_snprintf_error(sizeof(param), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s %d os_snprintf error1\n", __func__, __LINE__);
		return 0;
	}

	ret = 0;
	get_dpp_parameters(wapp->map, param, value, sizeof(value));
	if(os_strcmp(value,"1") != 0) {
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"no config found for 1905\n");
		return 0;
	}

	DBGPRINT(RT_DEBUG_INFO, DPP_MAP_PREX"1905 config found for wdev:%s\n", wdev->ifname);
	ret = os_snprintf(param, sizeof(param), "_1905netAccessKey");
	if (os_snprintf_error(sizeof(param), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s %d os_snprintf error2\n", __func__, __LINE__);
		return 0;
	}

	ret = 0;
	get_dpp_parameters(wapp->map, param, value, sizeof(value));
	hexstr2bin(value, buf_temp, os_strlen(value));
	wdev_config->dpp_netaccesskey_len = os_strlen(value) / 2;
	wdev_config->dpp_netaccesskey = os_zalloc(wdev_config->dpp_netaccesskey_len + 1);
	if(!wdev_config->dpp_netaccesskey) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"net access key malloc falied\n");
		return 0;
	}
	os_memcpy(wdev_config->dpp_netaccesskey, buf_temp, wdev_config->dpp_netaccesskey_len);
	ret = 1;

	return ret;
}

static void dpp_copy_ppkey(struct dpp_authentication *auth, EVP_PKEY *ppkey)
{
	unsigned char *der = NULL;
	int der_len;

	der_len = i2d_PUBKEY(ppkey, &der);
	if (der_len <= 0)
		return;
	wpabuf_free(auth->pp_key);
	auth->pp_key = wpabuf_alloc_copy(der, der_len);
	OPENSSL_free(der);
}
void dpp_auth_fail_wrapper(struct wifi_app *wapp, const char *txt)
{
	if (wapp) {
#ifdef MAP_R3
		conn_fail_reason *info_to_mapd = os_zalloc(sizeof(conn_fail_reason));
		if (info_to_mapd == NULL)
			return;
		os_memcpy(info_to_mapd->reason, txt, sizeof(info_to_mapd->reason)-1);
		wapp_send_1905_msg(
				wapp,
				WAPP_SEND_CONN_FAIL_NOTIF,
				sizeof(conn_fail_reason),
				(char *)info_to_mapd);
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" %s [%s] \n", info_to_mapd->reason, __func__);
		os_free(info_to_mapd);
		return;
#endif
	}
}
#endif /* MAP_R3 */

static void dpp_copy_netaccesskey(struct dpp_authentication *auth)
{
	unsigned char *der = NULL;
	int der_len;
	EC_KEY *eckey;

	eckey = EVP_PKEY_get1_EC_KEY(auth->own_protocol_key);
	if (!eckey)
		return;

	der_len = i2d_ECPrivateKey(eckey, &der);
	if (der_len <= 0) {
		EC_KEY_free(eckey);
		return;
	}
	wpabuf_free(auth->net_access_key);
	auth->net_access_key = wpabuf_alloc_copy(der, der_len);
	OPENSSL_free(der);
	EC_KEY_free(eckey);
}

#if 0
struct dpp_signed_connector_info {
	unsigned char *payload;
	size_t payload_len;
};
#endif
enum dpp_status_error
dpp_process_signed_connector(struct dpp_signed_connector_info *info,
			     EVP_PKEY *csign_pub, const char *connector)
{
	enum dpp_status_error ret = 255;
	const char *pos, *end, *signed_start, *signed_end;
	struct wpabuf *kid = NULL;
	unsigned char *prot_hdr = NULL, *signature = NULL;
	size_t prot_hdr_len = 0, signature_len = 0;
	const EVP_MD *sign_md = NULL;
	unsigned char *der = NULL;
	int der_len;
	int res;
	EVP_MD_CTX *md_ctx = NULL;
	ECDSA_SIG *sig = NULL;
	BIGNUM *r = NULL, *s = NULL;
	const struct dpp_curve_params *curve;
	EC_KEY *eckey;
	const EC_GROUP *group;
	int nid;

	eckey = EVP_PKEY_get1_EC_KEY(csign_pub);
	if (!eckey)
		goto fail;
	group = EC_KEY_get0_group(eckey);
	if (!group)
		goto fail;
	nid = EC_GROUP_get_curve_name(group);
	curve = dpp_get_curve_nid(nid);
	if (!curve)
		goto fail;
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: C-sign-key group: %s", curve->jwk_crv);
	os_memset(info, 0, sizeof(*info));

	signed_start = pos = connector;
	end = os_strchr(pos, '.');
	if (!end) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Missing dot(1) in signedConnector");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}
	prot_hdr = base64_url_decode((const char *) pos,
				     end - pos, &prot_hdr_len);
	if (!prot_hdr) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to base64url decode signedConnector JWS Protected Header");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}
	wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX
			  "DPP: signedConnector - JWS Protected Header",
			  prot_hdr, prot_hdr_len);
	kid = dpp_parse_jws_prot_hdr(curve, prot_hdr, prot_hdr_len, &sign_md);
	if (!kid) {
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}
	if (wpabuf_len(kid) != SHA256_MAC_LEN) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Unexpected signedConnector JWS Protected Header kid length: %u (expected %u)",
			   (unsigned int) wpabuf_len(kid), SHA256_MAC_LEN);
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}

	pos = end + 1;
	end = os_strchr(pos, '.');
	if (!end) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Missing dot(2) in signedConnector");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}
	signed_end = end - 1;
	info->payload = base64_url_decode((const char *) pos,
					  end - pos, &info->payload_len);
	if (!info->payload) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to base64url decode signedConnector JWS Payload");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}
	wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX
			  "DPP: signedConnector - JWS Payload",
			  info->payload, info->payload_len);
	pos = end + 1;
	signature = base64_url_decode((const char *) pos,
				      os_strlen(pos), &signature_len);
	if (!signature) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to base64url decode signedConnector signature");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
		}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: signedConnector - signature",
		    signature, signature_len);

	if (dpp_check_pubkey_match(csign_pub, kid) < 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Public key mismatch");
		ret = DPP_STATUS_NO_MATCH;
		goto fail;
	}

	if (signature_len & 0x01) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Unexpected signedConnector signature length (%d)",
			   (int) signature_len);
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}

	/* JWS Signature encodes the signature (r,s) as two octet strings. Need
	 * to convert that to DER encoded ECDSA_SIG for OpenSSL EVP routines. */
	r = BN_bin2bn(signature, signature_len / 2, NULL);
	s = BN_bin2bn(signature + signature_len / 2, signature_len / 2, NULL);
	sig = ECDSA_SIG_new();
	if (!r || !s || !sig || ECDSA_SIG_set0(sig, r, s) != 1)
		goto fail;
	r = NULL;
	s = NULL;

	der_len = i2d_ECDSA_SIG(sig, &der);
	if (der_len <= 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Could not DER encode signature");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: DER encoded signature", der, der_len);
	md_ctx = EVP_MD_CTX_create();
	if (!md_ctx)
		goto fail;

	ERR_clear_error();
	if (EVP_DigestVerifyInit(md_ctx, NULL, sign_md, NULL, csign_pub) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: EVP_DigestVerifyInit failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	if (EVP_DigestVerifyUpdate(md_ctx, signed_start,
				   signed_end - signed_start + 1) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: EVP_DigestVerifyUpdate failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	res = EVP_DigestVerifyFinal(md_ctx, der, der_len);
	if (res != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: EVP_DigestVerifyFinal failed (res=%d): %s",
			   res, ERR_error_string(ERR_get_error(), NULL));
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}

	ret = DPP_STATUS_OK;
fail:
	EC_KEY_free(eckey);
	EVP_MD_CTX_destroy(md_ctx);
	os_free(prot_hdr);
	wpabuf_free(kid);
	os_free(signature);
	ECDSA_SIG_free(sig);
	BN_free(r);
	BN_free(s);
	OPENSSL_free(der);
	return ret;
}

#ifdef DPP_R2_MUOBJ
static int dpp_parse_cred_dpp(struct dpp_authentication *auth, struct dpp_config_obj *conf,
			      struct json_token *cred)
{
	struct dpp_signed_connector_info info;
	struct json_token *token, *csign;
	int ret = -1;
	EVP_PKEY *csign_pub = NULL;
	const struct dpp_curve_params *key_curve = NULL;
	const char *signed_connector;

	os_memset(&info, 0, sizeof(info));

	if (dpp_akm_psk(conf->akm) || dpp_akm_sae(conf->akm)) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Legacy credential included in Connector credential");
		if (dpp_parse_cred_legacy(conf, cred) < 0)
			return -1;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Connector credential");

	csign = json_get_member(cred, "csign");
	if (!csign || csign->type != JSON_OBJECT) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No csign JWK in JSON");
		goto fail;
	}

	csign_pub = dpp_parse_jwk(csign, &key_curve);
	if (!csign_pub) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to parse csign JWK");
		goto fail;
	}
	dpp_debug_print_key("DPP: Received C-sign-key", csign_pub);

	token = json_get_member(cred, "signedConnector");
	if (!token || token->type != JSON_STRING) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No signedConnector string found");
		goto fail;
	}
	wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: signedConnector",
			  token->string, os_strlen(token->string));
	signed_connector = token->string;

	if (os_strchr(signed_connector, '"') ||
	    os_strchr(signed_connector, '\n')) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Unexpected character in signedConnector");
		goto fail;
	}

	if (dpp_process_signed_connector(&info, csign_pub,
					 signed_connector) != DPP_STATUS_OK)
		goto fail;

	if (dpp_parse_connector(auth, info.payload, info.payload_len) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to parse connector");
		goto fail;
	}

	os_free(conf->connector);
	conf->connector = os_strdup(signed_connector);

	dpp_copy_csign(auth, csign_pub);
	if (dpp_akm_dpp(conf->akm) || auth->peer_version >= 2)
		dpp_copy_netaccesskey(auth);

	ret = 0;
fail:
	EVP_PKEY_free(csign_pub);
	os_free(info.payload);
	return ret;
}
#else

static int dpp_parse_cred_dpp(struct dpp_authentication *auth,
			      struct json_token *cred)
{
	struct dpp_signed_connector_info info;
	struct json_token *token, *csign;
	int ret = -1;
	EVP_PKEY *csign_pub = NULL;
	const struct dpp_curve_params *key_curve = NULL;
	const char *signed_connector;
#ifdef MAP_R3
	struct json_token *ppkey;
	EVP_PKEY *pp_pub = NULL;
	const struct dpp_curve_params *pp_curve = NULL;
#endif /* MAP_R3 */

	os_memset(&info, 0, sizeof(info));

	if (dpp_akm_psk(auth->akm) || dpp_akm_sae(auth->akm)) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Legacy credential included in Connector credential");
		if (dpp_parse_cred_legacy(auth, cred) < 0)
			return -1;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Connector credential");

	csign = json_get_member(cred, "csign");
	if (!csign || csign->type != JSON_OBJECT) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No csign JWK in JSON");
		goto fail;
	}

	csign_pub = dpp_parse_jwk(csign, &key_curve);
	if (!csign_pub) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to parse csign JWK");
		goto fail;
	}
	dpp_debug_print_key("DPP: Received C-sign-key", csign_pub);

#ifdef MAP_R3
	ppkey = json_get_member(cred, "ppKey");
        if (ppkey && ppkey->type == JSON_OBJECT) {
                pp_pub = dpp_parse_jwk(ppkey, &pp_curve);
                if (!pp_pub) {
                        wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to parse ppKey JWK");
                        goto fail;
                }
                dpp_debug_print_key("DPP: Received ppKey", pp_pub);
                if (key_curve != pp_curve) {
                        wpa_printf(MSG_ERROR, DPP_MAP_PREX
                                   "DPP: C-sign-key and ppKey do not use the same curve");
                        goto fail;
                }
        }
	else {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX
				"DPP: No PPKey JWK in JSON");
	}
#endif /* MAP_R3 */

	token = json_get_member(cred, "signedConnector");
	if (!token || token->type != JSON_STRING) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No signedConnector string found");
		goto fail;
	}
	wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: signedConnector",
			  token->string, os_strlen(token->string));
	signed_connector = token->string;

	if (os_strchr(signed_connector, '"') ||
	    os_strchr(signed_connector, '\n')) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Unexpected character in signedConnector");
		goto fail;
	}

	if (dpp_process_signed_connector(&info, csign_pub,
					 signed_connector) != DPP_STATUS_OK)
		goto fail;

	if (dpp_parse_connector(auth, info.payload, info.payload_len) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to parse connector");
		goto fail;
	}

	os_free(auth->connector);
	auth->connector = os_strdup(signed_connector);

	dpp_copy_csign(auth, csign_pub);
	dpp_copy_netaccesskey(auth);
#ifdef MAP_R3
	if (pp_pub)
                dpp_copy_ppkey(auth, pp_pub);
#endif /* MAP_R3 */

	ret = 0;
fail:
	EVP_PKEY_free(csign_pub);
	os_free(info.payload);
	return ret;
}

#endif /*DPP_R2_MUOBJ */

const char * dpp_akm_str(enum dpp_akm akm)
{
	switch (akm) {
	case DPP_AKM_DPP:
		return "dpp";
	case DPP_AKM_PSK:
		return "psk";
	case DPP_AKM_SAE:
		return "sae";
	case DPP_AKM_PSK_SAE:
		return "psk+sae";
	case DPP_AKM_PSK_DPP:
		return "dpp+psk";
	case DPP_AKM_SAE_DPP:
		return "dpp+sae";
	case DPP_AKM_PSK_SAE_DPP:
		return "dpp+psk+sae";
	default:
		return "??";
	}
}


enum dpp_akm dpp_akm_from_str(const char *akm)
{
	if (os_strcmp(akm, "psk") == 0)
		return DPP_AKM_PSK;
	if (os_strcmp(akm, "sae") == 0)
		return DPP_AKM_SAE;
	if ((os_strcmp(akm, "psk+sae") == 0)
		|| (os_strcmp(akm, "sae+psk") == 0))
		return DPP_AKM_PSK_SAE;
	if (os_strcmp(akm, "dpp") == 0)
		return DPP_AKM_DPP;
	if ((os_strcmp(akm, "dpp+psk") == 0)
		|| (os_strcmp(akm, "psk+dpp") == 0))
		return DPP_AKM_PSK_DPP;
	if ((os_strcmp(akm, "dpp+sae") == 0)
		|| (os_strcmp(akm, "sae+dpp") == 0))
		return DPP_AKM_SAE_DPP;
	if ((os_strcmp(akm, "dpp+psk+sae") == 0)
		|| (os_strcmp(akm, "dpp+sae+psk") == 0))
		return DPP_AKM_PSK_SAE_DPP;
	return DPP_AKM_UNKNOWN;
}

#ifdef DPP_R2_MUOBJ
static int dpp_parse_conf_obj(struct dpp_authentication *auth,
			      const u8 *conf_obj, u16 conf_obj_len)
{
	int ret = -1;
	struct json_token *root, *token, *discovery, *cred;
	struct dpp_config_obj     *conf = NULL;
	int legacy;

#ifdef MAP_R3
	u8 is_1905 = 0;
#endif /* MAP_R3 */
	if( !auth|| !conf_obj)
		return -1;

	root = json_parse((const char *) conf_obj, conf_obj_len);
	if (!root)
		return -1;
	if (root->type != JSON_OBJECT) {
		dpp_auth_fail(auth, "JSON root is not an object");
		goto fail;
	}

	token = json_get_member(root, "wi-fi_tech");
	if (!token || token->type != JSON_STRING) {
		dpp_auth_fail(auth, "No wi-fi_tech string value found");
		goto fail;
	}

	if ((os_strcmp(token->string, "infra") != 0) &&
			(os_strcmp(token->string, "map") != 0) &&
			(os_strcmp(token->string, "mapBackhaulBss") != 0) &&
			(os_strcmp(token->string, "inframap") != 0)) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unsupported wi-fi_tech '%s'",
				token->string);
		dpp_auth_fail(auth, "Unsupported wi-fi_tech");
		goto fail;
	}
#if 0
	if (os_strcmp(token->string, "infra") != 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unsupported wi-fi_tech value: '%s'",
			   token->string);
		dpp_auth_fail(auth, "Unsupported wi-fi_tech value");
		goto fail;
	}
#endif

	discovery = json_get_member(root, "discovery");
	if (!discovery || discovery->type != JSON_OBJECT) {
		dpp_auth_fail(auth, "No discovery object in JSON");
#ifdef MAP_R3
		auth->is_1905_connector = 1;
		is_1905 = 1;
		goto skip_discovery;
#else
		goto fail;
#endif /* MAP_R3 */
	}

	if (auth->conf_obj_num == DPP_CONF_OBJ_MAX) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: No room for this many Config Objects - ignore this one");
		ret = 0;
		goto fail;
	}
	conf = &auth->conf_obj[auth->conf_obj_num++];
	
	token = json_get_member(discovery, "ssid");
	if (!token || token->type != JSON_STRING) {
		dpp_auth_fail(auth, "No discovery::ssid string value found");
	} else {
		wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: discovery::ssid",
				token->string, os_strlen(token->string));
		if (os_strlen(token->string) > SSID_MAX_LEN) {
			dpp_auth_fail(auth, "Too long discovery::ssid string value");
			goto fail;
		}
		conf->ssid_len = os_strlen(token->string);
		os_memcpy(conf->ssid, token->string, conf->ssid_len);
	}
#ifdef MAP_R3
skip_discovery:
	//Applying configuration to existing wdev for other than 1905 configs
	if (!is_1905)
#endif /* MAP_R3 */
		auth->config_wdev = auth->wdev;

	cred = json_get_member(root, "cred");
	if (!cred || cred->type != JSON_OBJECT) {
		dpp_auth_fail(auth, "No cred object in JSON");
		goto fail;
	}

	token = json_get_member(cred, "akm");
	if (!token || token->type != JSON_STRING) {
		dpp_auth_fail(auth, "No cred::akm string value found");
		goto fail;
	}
	conf->akm = dpp_akm_from_str(token->string);

	legacy = dpp_akm_legacy(conf->akm);
	if(legacy && auth->peer_version >= 2) {/*log print version number*/
		struct json_token *csign, *s_conn;

		csign = json_get_member(cred, "csign");
		s_conn = json_get_member(cred, "signedConnector");
		if (csign && csign->type == JSON_OBJECT &&
		    s_conn && s_conn->type == JSON_STRING)
			legacy = 0;
	}

	if (legacy) {
		if (dpp_parse_cred_legacy(conf, cred) < 0)
			goto fail;
	} else if (dpp_akm_dpp(conf->akm) ||
		   (auth->peer_version >= 2 && dpp_akm_legacy(conf->akm))) {
		if (dpp_parse_cred_dpp(auth, conf, cred) < 0)
			goto fail;
	} else {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unsupported akm: %s",
			   token->string);
		dpp_auth_fail(auth, "Unsupported akm");
		goto fail;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: JSON parsing completed successfully");
	ret = 0;
fail:
	json_free(root);
	return ret;
}

#else
static int dpp_parse_conf_obj(struct dpp_authentication *auth,
			      const u8 *conf_obj, u16 conf_obj_len)
{
	int ret = -1;
	struct json_token *root, *token, *discovery, *cred;
	int legacy;
#ifdef MAP_R3
	u8 is_1905 = 0;
	struct json_token *dec_count;
#endif /* MAP_R3 */

	root = json_parse((const char *) conf_obj, conf_obj_len);
	if (!root)
		return -1;
	if (root->type != JSON_OBJECT) {
		dpp_auth_fail(auth, "JSON root is not an object");
		goto fail;
	}

	token = json_get_member(root, "wi-fi_tech");
	if (!token || token->type != JSON_STRING) {
		dpp_auth_fail(auth, "No wi-fi_tech string value found");
		goto fail;
	}
	if ((os_strcmp(token->string, "infra") != 0) &&
			(os_strcmp(token->string, "map") != 0) &&
			(os_strcmp(token->string, "mapBackhaulBss") != 0) &&
			(os_strcmp(token->string, "inframap") != 0)) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unsupported wi-fi_tech '%s'",
				token->string);
		dpp_auth_fail(auth, "Unsupported wi-fi_tech");
		goto fail;
	}
#if 0
	if (os_strcmp(token->string, "infra") != 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unsupported wi-fi_tech value: '%s'",
			   token->string);
		dpp_auth_fail(auth, "Unsupported wi-fi_tech value");
		goto fail;
	}
#endif

#ifdef MAP_R3
	dec_count = json_get_member(root, "dfCounterThreshold");
	if(!dec_count) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "No decryption failure counter object");
	}
	else {
		if (dec_count->type != JSON_NUMBER) {
			dpp_auth_fail(auth, "Invalid decryption failure counter value found");
			goto fail;
		}
		auth->decrypt_thresold = (unsigned short)dec_count->number;
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "decryption failure counter value: %d",auth->decrypt_thresold);
	}
#endif /* MAP_R3 */

	discovery = json_get_member(root, "discovery");
	if (!discovery || discovery->type != JSON_OBJECT) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "No discovery object in JSON");
#ifdef MAP_R3
		is_1905 = 1;
		goto skip_discovery;  //Skip discovery for 1905 configurations
#else
		goto fail;
#endif /* MAP_R3 */
	}

	token = json_get_member(discovery, "ssid");
	if (!token || token->type != JSON_STRING) {
		dpp_auth_fail(auth, "No discovery::ssid string value found");
	} else {
		wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: discovery::ssid",
				token->string, os_strlen(token->string));
		if (os_strlen(token->string) > SSID_MAX_LEN) {
			dpp_auth_fail(auth, "Too long discovery::ssid string value");
			goto fail;
		}
		auth->ssid_len = os_strlen(token->string);
		os_memcpy(auth->ssid, token->string, auth->ssid_len);
	}
#ifdef MAP_R3
skip_discovery:
	//Applying configuration to existing wdev for other than 1905 configs
	if (!is_1905)
#endif /* MAP_R3 */
		auth->config_wdev = auth->wdev;

#if 0
	if (auth->is_map_connection) {
		ruid = json_get_member(discovery, "RUID");
		if (!ruid || ruid->type != JSON_STRING) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX "No RUID in JSON");
			goto fail;
		}
		size_t out_len;
		unsigned char *radio_id = base64_url_decode((const unsigned char *)ruid->string, strlen(ruid->string), &out_len);
		DBGPRINT(RT_DEBUG_ERROR,"outlen=%zu\n", out_len);
		DBGPRINT(RT_DEBUG_ERROR,"ruid is %02x%02x%02x%02x%02x%02x \n",PRINT_MAC(radio_id));
		ub_idx = json_get_member(discovery, "UBINDX");
		if (!ub_idx || ub_idx->type != JSON_STRING) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX "No UBINDX object in JSON");
			goto fail;
		}
		unsigned char *idx = base64_url_decode((const unsigned char *)ub_idx->string, strlen(ruid->string), &out_len);
		DBGPRINT(RT_DEBUG_ERROR,"ubidex is %u\n", *idx);
		//TODO this code
		auth->config_wdev = wapp_dev_list_lookup_by_radio_nid(wapp, (char *)radio_id, *idx); 
	}
#endif

	cred = json_get_member(root, "cred");
	if (!cred || cred->type != JSON_OBJECT) {
		dpp_auth_fail(auth, "No cred object in JSON");
		goto fail;
	}
	token = json_get_member(cred, "akm");
	if (!token || token->type != JSON_STRING) {
		dpp_auth_fail(auth, "No cred::akm string value found");
		goto fail;
	}
	auth->akm = dpp_akm_from_str(token->string);

	legacy = dpp_akm_legacy(auth->akm);
	if(legacy && auth->peer_version >= 2) {/*log print version number*/
		struct json_token *csign, *s_conn;

		csign = json_get_member(cred, "csign");
		s_conn = json_get_member(cred, "signedConnector");
		if (csign && csign->type == JSON_OBJECT &&
		    s_conn && s_conn->type == JSON_STRING)
			legacy = 0;
	}

	if (legacy) {
		if (dpp_parse_cred_legacy(auth, cred) < 0)
			goto fail;
	} else if (dpp_akm_dpp(auth->akm) ||
		   (auth->peer_version >= 2 && dpp_akm_legacy(auth->akm))) {
		if (dpp_parse_cred_dpp(auth, cred) < 0)
			goto fail;
	} else {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unsupported akm: %s",
			   token->string);
		dpp_auth_fail(auth, "Unsupported akm");
		goto fail;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: JSON parsing completed successfully");
	ret = 0;
fail:
	json_free(root);
	return ret;
}
#endif /* DPP_R2_MUOBJ */

#ifdef MAP_R3
static int dpp_parse_map_cred_dpp(struct wifi_app * wapp, struct dpp_config *wdev_config,
			      struct json_token *cred)
{
	struct dpp_signed_connector_info info;
	struct json_token *token, *csign, *root;
	int ret = -1;
	EVP_PKEY *csign_pub = NULL;
	const struct dpp_curve_params *key_curve = NULL;
	const char *signed_connector;
	struct dpp_authentication *auth = NULL;
	u16 connector_len = 0;

	os_memset(&info, 0, sizeof(info));

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Connector credential");

	csign = json_get_member(cred, "csign");
	if (!csign || csign->type != JSON_OBJECT) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No csign JWK in JSON");
		goto fail;
	}

	csign_pub = dpp_parse_jwk(csign, &key_curve);
	if (!csign_pub) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to parse csign JWK");
		goto fail;
	}
	dpp_debug_print_key("DPP: Received C-sign-key", csign_pub);

	token = json_get_member(cred, "signedConnector");
	if (!token || token->type != JSON_STRING) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No signedConnector string found");
		goto fail;
	}
	wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: signedConnector",
			  token->string, os_strlen(token->string));
	signed_connector = token->string;

	if (os_strchr(signed_connector, '"') ||
	    os_strchr(signed_connector, '\n')) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Unexpected character in signedConnector");
		goto fail;
	}

	if (dpp_process_signed_connector(&info, csign_pub,
					 signed_connector) != DPP_STATUS_OK)
		goto fail;

	wdev_config->dpp_netaccesskey_expiry = 0;

	root = json_parse((const char *) info.payload, info.payload_len);
	if (!root) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: JSON parsing of connector failed");
		goto fail;
	}

	token = json_get_member(root, "expiry");
	if (!token || token->type != JSON_STRING) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: No expiry string found - connector does not expire");
	} else {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: expiry = %s", token->string);
		if (dpp_key_expired(token->string,
				    (os_time_t *)&wdev_config->dpp_netaccesskey_expiry)) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Connector (netAccessKey) has expired");
			goto fail;
		}
	}
#if 0
	if (dpp_parse_connector(auth, info.payload, info.payload_len) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Failed to parse connector");
		goto fail;
	}
#endif

	dpp_map_copy_csign(wdev_config, csign_pub);

	ret = dpp_map_copy_netaccess_frm_cfg(wapp, wdev_config);
	if(ret == 0) {
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"%s Net accee key not found in file, take it out from auth\n", __func__);
		auth = wapp_dpp_get_first_auth(wapp);
		if(!auth) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s auth instance not found\n", __func__);
			goto fail;
		}

		dpp_copy_netaccesskey(auth);
		if (auth->net_access_key) {
			if (wdev_config->dpp_netaccesskey)
				os_free(wdev_config->dpp_netaccesskey);
			wdev_config->dpp_netaccesskey =
				os_malloc(wpabuf_len(auth->net_access_key));
			if (!wdev_config->dpp_netaccesskey)
				goto fail;
			os_memcpy(wdev_config->dpp_netaccesskey,
					wpabuf_head(auth->net_access_key),
					wpabuf_len(auth->net_access_key));
			wdev_config->dpp_netaccesskey_len = wpabuf_len(auth->net_access_key);
			wdev_config->dpp_netaccesskey_expiry = auth->net_access_key_expiry;
		}
	}

	if(wdev_config->dpp_connector)
		os_free(wdev_config->dpp_connector);
	connector_len = os_strlen(signed_connector);
	wdev_config->dpp_connector = os_malloc(connector_len + 1);
	if (!wdev_config->dpp_connector) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"Connector Malloc failed\n");
		goto fail;
	}
	os_memcpy(wdev_config->dpp_connector, signed_connector,
			connector_len);
	wdev_config->dpp_connector[connector_len] = '\0';

	if(wdev_config->dpp_connector)
	wpa_printf(MSG_INFO1, DPP_MAP_PREX DPP_EVENT_CONNECTOR "After: %s",
		wdev_config->dpp_connector);

	if(wdev_config->dpp_csign) {
		char *hex;
		size_t hexlen;

		hexlen = 2 * wdev_config->dpp_csign_len + 1;
		hex = os_malloc(hexlen);
		if (hex) {
			os_snprintf_hex(hex, hexlen,
				wdev_config->dpp_csign, wdev_config->dpp_csign_len);
			wpa_printf(MSG_INFO1, DPP_MAP_PREX
				DPP_EVENT_C_SIGN_KEY "%s", hex);
			os_free(hex);
		}
	}

	if(wdev_config->dpp_netaccesskey) {
		char *hex;
		size_t hexlen;

		hexlen = 2 * wdev_config->dpp_netaccesskey_len + 1;
		hex = os_malloc(hexlen);
		if (hex) {
			os_snprintf_hex(hex, hexlen,
				wdev_config->dpp_netaccesskey, wdev_config->dpp_netaccesskey_len);
			wpa_printf(MSG_INFO1, DPP_MAP_PREX
				DPP_EVENT_NET_ACCESS_KEY "%s", hex);
			os_free(hex);
		}
	}
	ret = 0;
fail:
	EVP_PKEY_free(csign_pub);
	os_free(info.payload);
	return ret;
}

int dpp_parse_map_conf_obj(struct dpp_config *wdev_config,
			      struct wifi_app *wapp, const u8 *conf_obj, u16 conf_obj_len)
{
	int ret = -1;
	struct json_token *root, *token, *cred;

	root = json_parse((const char *) conf_obj, conf_obj_len);
	if (!root)
		return -1;
	if (root->type != JSON_OBJECT) {
		dpp_auth_fail_wrapper(wapp, "JSON root is not an object");
		goto fail;
	}

	cred = json_get_member(root, "cred");
	if (!cred || cred->type != JSON_OBJECT) {
		dpp_auth_fail_wrapper(wapp, "No cred object in JSON");
		goto fail;
	}
	wdev_config->map_bss_akm = 0;
	token = json_get_member(cred, "akm");
	if (!token || token->type != JSON_STRING) {
		dpp_auth_fail_wrapper(wapp, "No cred::akm string value found");
		goto fail;
	}
	wdev_config->map_bss_akm = dpp_akm_from_str(token->string);

	if (dpp_akm_dpp(wdev_config->map_bss_akm) ||
		   ( dpp_akm_legacy(wdev_config->map_bss_akm))) {
		if (dpp_parse_map_cred_dpp(wapp, wdev_config, cred) < 0)
			goto fail;
	} else {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Unsupported akm: %s",
			   token->string);
		dpp_auth_fail_wrapper(wapp, "Unsupported akm");
		goto fail;
	}

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: JSON parsing completed successfully");
	ret = 0;
fail:
	json_free(root);
	return ret;
}
#endif /* MAP_R3 */

int dpp_conf_resp_rx(struct dpp_authentication *auth,
		     const struct wpabuf *resp)
{
	const u8 *wrapped_data, *e_nonce, *status, *conf_obj;
	u16 wrapped_data_len, e_nonce_len, status_len, conf_obj_len;
	const u8 *addr[1];
	size_t len[1];
	u8 *unwrapped = NULL;
	size_t unwrapped_len = 0;
	int ret = -1, count = 0, i;
	auth->conf_resp_status = 255;

	if (dpp_check_attrs(wpabuf_head(resp), wpabuf_len(resp)) < 0) {
		dpp_auth_fail(auth, "Invalid attribute in config response");
		return -1;
	}

	wrapped_data = dpp_get_attr(wpabuf_head(resp), wpabuf_len(resp),
				    DPP_ATTR_WRAPPED_DATA,
				    &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		dpp_auth_fail(auth,
			      "Missing or invalid required Wrapped Data attribute");
		return -1;
	}

	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped_data, wrapped_data_len);
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped = os_malloc(unwrapped_len);
	if (!unwrapped)
		return -1;

	addr[0] = wpabuf_head(resp);
	len[0] = wrapped_data - 4 - (const u8 *) wpabuf_head(resp);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD", addr[0], len[0]);

	if (aes_siv_decrypt(auth->ke, auth->curve->hash_len,
			    wrapped_data, wrapped_data_len,
			    1, addr, len, unwrapped) < 0) {
		dpp_auth_fail(auth, "AES-SIV decryption failed");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext",
		    unwrapped, unwrapped_len);

	if (dpp_check_attrs(unwrapped, unwrapped_len) < 0) {
		dpp_auth_fail(auth, "Invalid attribute in unwrapped data");
		goto fail;
	}

	e_nonce = dpp_get_attr(unwrapped, unwrapped_len,
			       DPP_ATTR_ENROLLEE_NONCE,
			       &e_nonce_len);
	if (!e_nonce || e_nonce_len != auth->curve->nonce_len) {
		dpp_auth_fail(auth,
			      "Missing or invalid Enrollee Nonce attribute");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Enrollee Nonce", e_nonce, e_nonce_len);
	if (os_memcmp(e_nonce, auth->e_nonce, e_nonce_len) != 0) {
		dpp_auth_fail(auth, "Enrollee Nonce mismatch");
		goto fail;
	}

	status = dpp_get_attr(wpabuf_head(resp), wpabuf_len(resp),
			      DPP_ATTR_STATUS, &status_len);
	if (!status || status_len < 1) {
		dpp_auth_fail(auth,
			      "Missing or invalid required DPP Status attribute");
		goto fail;
	}
	auth->conf_resp_status = status[0];
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Status %u", status[0]);
	if (status[0] != DPP_STATUS_OK) {
		dpp_auth_fail(auth, "Configurator rejected configuration");
		goto fail;
	}

	count = dpp_get_config_objects_count(unwrapped, unwrapped_len);
	if (!count) {
		dpp_auth_fail(auth,
				"Missing required Configuration Object attribute");
		goto fail;
	}

#ifdef CONFIG_DPP2
	status = dpp_get_attr(unwrapped, unwrapped_len,
			      DPP_ATTR_SEND_CONN_STATUS, &status_len);
	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: conf resp rx peer_version:%d", auth->peer_version);
	if (status) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Configurator requested connection status result");
		auth->conn_status_requested = 1;
	}
#endif /* CONFIG_DPP2 */

	for (i = 1; i <= count; i++) {
		conf_obj = dpp_get_config_object(unwrapped, unwrapped_len,
				i, &conf_obj_len);
		if (!conf_obj) {
			goto fail;
		}
		wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: configurationObject JSON",
				conf_obj, conf_obj_len);
		if (dpp_parse_conf_obj(auth, conf_obj, conf_obj_len) < 0)
			goto fail;
#ifndef DPP_R2_MUOBJ
		ret = wapp_dpp_handle_config_obj((struct wifi_app *) auth->msg_ctx, auth);
		if (ret < 0)
			goto fail;
#endif /* DPP_R2_MUOBJ */
	}
	
#ifdef DPP_R2_MUOBJ
	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: conf resp rx config count:%d", count);
	ret = wapp_dpp_handle_config_obj((struct wifi_app *) auth->msg_ctx, auth, &auth->conf_obj[0]);
	if (ret < 0)
		goto fail;
#endif /* DPP_R2_MUOBJ */

	ret = 0;

fail:
	os_free(unwrapped);
	return ret;
}


#ifdef DPP_R2_SUPPORT
enum dpp_status_error dpp_conf_result_rx(struct dpp_authentication *auth,
					 const u8 *hdr,
					 const u8 *attr_start, size_t attr_len)
{
	const u8 *wrapped_data, *status, *e_nonce;
	u16 wrapped_data_len, status_len, e_nonce_len;
	const u8 *addr[2];
	size_t len[2];
	u8 *unwrapped = NULL;
	size_t unwrapped_len = 0;
	enum dpp_status_error ret = 256;

	wrapped_data = dpp_get_attr(attr_start, attr_len, DPP_ATTR_WRAPPED_DATA,
				    &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		dpp_auth_fail(auth,
			      "Missing or invalid required Wrapped Data attribute");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Wrapped data",
		    wrapped_data, wrapped_data_len);

	attr_len = wrapped_data - 4 - attr_start;

	addr[0] = hdr;
	len[0] = DPP_HDR_LEN;
	addr[1] = attr_start;
	len[1] = attr_len;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped_data, wrapped_data_len);
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped = os_malloc(unwrapped_len);
	if (!unwrapped)
		goto fail;
	if (aes_siv_decrypt(auth->ke, auth->curve->hash_len,
			    wrapped_data, wrapped_data_len,
			    2, addr, len, unwrapped) < 0) {
		dpp_auth_fail(auth, "AES-SIV decryption failed");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext",
		    unwrapped, unwrapped_len);

	if (dpp_check_attrs(unwrapped, unwrapped_len) < 0) {
		dpp_auth_fail(auth, "Invalid attribute in unwrapped data");
		goto fail;
	}

	e_nonce = dpp_get_attr(unwrapped, unwrapped_len,
			       DPP_ATTR_ENROLLEE_NONCE,
			       &e_nonce_len);
	if (!e_nonce || e_nonce_len != auth->curve->nonce_len) {
		dpp_auth_fail(auth,
			      "Missing or invalid Enrollee Nonce attribute");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Enrollee Nonce", e_nonce, e_nonce_len);
	if (os_memcmp(e_nonce, auth->e_nonce, e_nonce_len) != 0) {
		dpp_auth_fail(auth, "Enrollee Nonce mismatch");
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Expected Enrollee Nonce",
			    auth->e_nonce, e_nonce_len);
		goto fail;
	}

	status = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_STATUS,
			      &status_len);
	if (!status || status_len < 1) {
		dpp_auth_fail(auth,
			      "Missing or invalid required DPP Status attribute");
		goto fail;
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Status %u", status[0]);
	ret = status[0];

fail:
	bin_clear_free(unwrapped, unwrapped_len);
	return ret;
}
#endif /* DPP_R2_SUPPORT */

struct wpabuf * dpp_build_conf_result(struct dpp_authentication *auth,
				      enum dpp_status_error status)
{
	struct wpabuf *msg = NULL;
	struct wpabuf *clear = NULL;
	size_t nonce_len, clear_len, attr_len;
	const u8 *addr[2];
	size_t len[2];
	u8 *wrapped;

	nonce_len = auth->curve->nonce_len;
	clear_len = 5 + 4 + nonce_len;
	attr_len = 4 + clear_len + AES_BLOCK_SIZE;
	clear = wpabuf_alloc(clear_len);
	msg = dpp_alloc_msg(DPP_PA_CONFIGURATION_RESULT, attr_len);
	if (!clear || !msg)
		goto fail;

	/* DPP Status */
	dpp_build_attr_status(clear, status);

	/* E-nonce */
	wpabuf_put_le16(clear, DPP_ATTR_ENROLLEE_NONCE);
	wpabuf_put_le16(clear, nonce_len);
	wpabuf_put_data(clear, auth->e_nonce, nonce_len);

	/* OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = wpabuf_head_u8(msg) + 2;
	len[0] = 3 + 1 + 1 + 1;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);

	/* Attributes before Wrapped Data (none) */
	addr[1] = wpabuf_put(msg, 0);
	len[1] = 0;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);

	/* Wrapped Data */
	wpabuf_put_le16(msg, DPP_ATTR_WRAPPED_DATA);
	wpabuf_put_le16(msg, wpabuf_len(clear) + AES_BLOCK_SIZE);
	wrapped = wpabuf_put(msg, wpabuf_len(clear) + AES_BLOCK_SIZE);

	wpa_hexdump_buf(MSG_DEBUG, "DPP: AES-SIV cleartext", clear);
	if (aes_siv_encrypt(auth->ke, auth->curve->hash_len,
			    wpabuf_head(clear), wpabuf_len(clear),
			    2, addr, len, wrapped) < 0)
		goto fail;

	wpa_hexdump_buf(MSG_DEBUG, "DPP: Configuration Result attributes", msg);
	wpabuf_free(clear);
	return msg;
fail:
	if (clear)
		wpabuf_free(clear);
	if (msg)
		wpabuf_free(msg);
	return NULL;
}


void dpp_configurator_free(struct dpp_configurator *conf)
{
	if (!conf)
		return;
	EVP_PKEY_free(conf->csign);
	os_free(conf->kid);
	os_free(conf);
}


int dpp_configurator_get_key(const struct dpp_configurator *conf, char *buf,
			     size_t buflen)
{
	EC_KEY *eckey;
	int key_len, ret = -1;
	unsigned char *key = NULL;

	if (!conf->csign)
		return -1;

	eckey = EVP_PKEY_get1_EC_KEY(conf->csign);
	if (!eckey)
		return -1;

	key_len = i2d_ECPrivateKey(eckey, &key);
	if (key_len > 0)
		ret = os_snprintf_hex(buf, buflen, key, key_len);

	EC_KEY_free(eckey);
	OPENSSL_free(key);
	return ret;
}


struct dpp_configurator *
dpp_keygen_configurator(const char *curve, const u8 *privkey,
			size_t privkey_len
#ifdef MAP_R3
			, const u8 *pp_key, size_t pp_key_len
#endif /* MAP_R3 */
)
{
	struct dpp_configurator *conf;
	struct wpabuf *csign_pub = NULL;
	u8 kid_hash[SHA256_MAC_LEN];
	const u8 *addr[1];
	size_t len[1];

	conf = os_zalloc(sizeof(*conf));
	if (!conf)
		return NULL;

	if (!curve) {
		conf->curve = &dpp_curves[0];
	} else {
		conf->curve = dpp_get_curve_name(curve);
		if (!conf->curve) {
			wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Unsupported curve: %s",
				   curve);
			os_free(conf);
			return NULL;
		}
	}
	if (privkey)
		conf->csign = dpp_set_keypair(&conf->curve, privkey,
					      privkey_len);
	else
		conf->csign = dpp_gen_keypair(conf->curve);

#ifdef MAP_R3
	if (pp_key)
		conf->pp_key = dpp_set_keypair(&conf->curve, pp_key,
					      pp_key_len);
	else
		conf->pp_key = dpp_gen_keypair(conf->curve);
#endif /* MAP_R3 */

	if (!conf->csign
#ifdef MAP_R3
		|| !conf->pp_key
#endif /* MAP_R3 */
	)
		goto fail;
	conf->own = 1;

	csign_pub = dpp_get_pubkey_point(conf->csign, 1);
	if (!csign_pub) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to extract C-sign-key");
		goto fail;
	}

	/* kid = SHA256(ANSI X9.63 uncompressed C-sign-key) */
	addr[0] = wpabuf_head(csign_pub);
	len[0] = wpabuf_len(csign_pub);
	if (sha256_vector(1, addr, len, kid_hash) < 0) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: Failed to derive kid for C-sign-key");
		goto fail;
	}

	conf->kid = (char *) base64_url_encode(kid_hash, sizeof(kid_hash),
					       NULL, 0);
	if (!conf->kid)
		goto fail;
out:
	wpabuf_free(csign_pub);
	return conf;
fail:
	dpp_configurator_free(conf);
	conf = NULL;
	goto out;
}

#ifdef DPP_R2_MUOBJ

int dpp_configurator_own_config(struct dpp_authentication *auth,
				const char *curve, int ap
#ifdef MAP_R3
	, int is_map
#endif /* MAP_R3 */
)
{
	struct wpabuf *conf_obj[DPP_CONF_OBJ_MAX];
	int ret = -1, count, i = 0;

	if (!auth->conf) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: No configurator specified");
		return -1;
	}

	if (!curve) {
		auth->curve = &dpp_curves[0];
	} else {
		auth->curve = dpp_get_curve_name(curve);
		if (!auth->curve) {
			wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Unsupported curve: %s",
				   curve);
			return -1;
		}
	}
	wpa_printf(MSG_INFO1, DPP_MAP_PREX
		   "DPP: Building own configuration/connector with curve %s",
		   auth->curve->name);

	auth->own_protocol_key = dpp_gen_keypair(auth->curve);

	if (!auth->own_protocol_key)
		return -1;

	dpp_copy_netaccesskey(auth);
	auth->peer_protocol_key = auth->own_protocol_key;
	dpp_copy_csign(auth, auth->conf->csign);
	dpp_build_conf_obj(auth, ap, 0, &count, conf_obj
#ifdef MAP_R3
		, NULL, NULL
#endif /* MAP_R3*/
	);

	if (!conf_obj[0])
		goto fail;
	ret = dpp_parse_conf_obj(auth, wpabuf_head(conf_obj[0]),
				 wpabuf_len(conf_obj[0]));

fail:
	//wpabuf_free(conf_obj);
		for (i = 0; i < count; i++) {
		wpabuf_free(conf_obj[i]);
	}
	auth->peer_protocol_key = NULL;
	return ret;
}
#else
int dpp_configurator_own_config(struct dpp_authentication *auth,
				const char *curve, int ap
#ifdef MAP_R3
	, int is_map
#endif /* MAP_R3 */
)
{
	struct wpabuf *conf_obj=NULL;
	int ret = -1, count;
#ifdef MAP_R3
	u8 self_sign = 1;
#endif /* MAP_R3 */

	if (!auth->conf) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: No configurator specified");
		return -1;
	}

	if (!curve) {
		auth->curve = &dpp_curves[0];
	} else {
		auth->curve = dpp_get_curve_name(curve);
		if (!auth->curve) {
			wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Unsupported curve: %s",
				   curve);
			return -1;
		}
	}
	wpa_printf(MSG_INFO1, DPP_MAP_PREX
		   "DPP: Building own configuration/connector with curve %s",
		   auth->curve->name);

	auth->own_protocol_key = dpp_gen_keypair(auth->curve);
	if (!auth->own_protocol_key)
		return -1;
	dpp_copy_netaccesskey(auth);
	auth->peer_protocol_key = auth->own_protocol_key;
	dpp_copy_csign(auth, auth->conf->csign);
#ifdef MAP_R3_RECONFIG
	if (auth->conf->pp_key)
                dpp_copy_ppkey(auth, auth->conf->pp_key);
#endif /* MAP_R3 */


#ifdef MAP_R3
	dpp_build_conf_obj(auth, ap, is_map, &count, &conf_obj
		, NULL, NULL, 0, self_sign 
#ifdef MAP_R3_RECONFIG
		, 0, 0
#endif
	);
#else
	dpp_build_conf_obj(auth, ap, 0, &count, &conf_obj);
#endif /* MAP_R3*/

	if (!conf_obj)
		goto fail;
	ret = dpp_parse_conf_obj(auth, wpabuf_head(conf_obj),
				 wpabuf_len(conf_obj));
fail:
	wpabuf_free(conf_obj);
#ifndef MAP_R3
	auth->peer_protocol_key = NULL;
#endif
	return ret;
}

#endif /* DPP_R2_MUOBJ*/

static int dpp_compatible_netrole(const char *role1, const char *role2)
{
#ifdef MAP_R3
	//return (os_strcmp(role1, "mapBackhaulSta") == 0 && os_strcmp(role2, "mapBackhaulBss") == 0) ||
		//(os_strcmp(role1, "mapBackhaulBss") == 0 && os_strcmp(role2, "mapBackhaulSta") == 0);
	return (os_strcasecmp(role1, "mapBackhaulSta") == 0 && os_strcmp(role2, "mapBackhaulBss") == 0) ||
		(os_strcmp(role1, "mapBackhaulBss") == 0 && os_strcasecmp(role2, "mapBackhaulSta") == 0) ||
		(os_strcmp(role1, "ap") == 0 && os_strcmp(role2, "sta") == 0);
#else
	return (os_strcmp(role1, "sta") == 0 && os_strcmp(role2, "ap") == 0) ||
		(os_strcmp(role1, "ap") == 0 && os_strcmp(role2, "sta") == 0);
#endif /* MAP_R3 */
}


static int dpp_connector_compatible_group(struct json_token *root,
					  const char *group_id,
					  const char *net_role
#ifdef DPP_R2_RECONFIG					  
					  , BOOLEAN reconfig
#endif					  
					  )
{
	struct json_token *groups, *token;

	groups = json_get_member(root, "groups");
	if (!groups || groups->type != JSON_ARRAY)
		return 0;

	for (token = groups->child; token; token = token->sibling) {
		struct json_token *id, *role;

		id = json_get_member(token, "groupId");
		if (!id || id->type != JSON_STRING)
			continue;

		role = json_get_member(token, "netRole");
		if (!role || role->type != JSON_STRING)
			continue;

		if (os_strcmp(id->string, "*") != 0 &&
		    os_strcmp(group_id, "*") != 0 &&
		    os_strcmp(id->string, group_id) != 0)
			continue;
#ifdef DPP_R2_RECONFIG
		if (reconfig && os_strcmp(net_role, "configurator") == 0)
			return 1;
#endif
		if (
#ifdef DPP_R2_RECONFIG
		!reconfig && 
#endif
		 dpp_compatible_netrole(role->string, net_role))
			return 1;
	}

	return 0;
}


int dpp_connector_match_groups(struct json_token *own_root,
				      struct json_token *peer_root
#ifdef DPP_R2_RECONFIG
					  , BOOLEAN reconfig
#endif
					  )
{
	struct json_token *groups, *token;

	groups = json_get_member(peer_root, "groups");
	if (!groups || groups->type != JSON_ARRAY) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: No peer groups array found");
		return 0;
	}

	for (token = groups->child; token; token = token->sibling) {
		struct json_token *id, *role;

		id = json_get_member(token, "groupId");
		if (!id || id->type != JSON_STRING) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Missing peer groupId string");
			continue;
		}

		role = json_get_member(token, "netRole");
		if (!role || role->type != JSON_STRING) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Missing peer groups::netRole string");
			continue;
		}
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: peer connector group: groupId='%s' netRole='%s'",
			   id->string, role->string);
		if (dpp_connector_compatible_group(own_root, id->string,
						   role->string
#ifdef DPP_R2_RECONFIG
						   , reconfig
#endif												   
						   )) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Compatible group/netRole in own connector");
			return 1;
		}
	}

	return 0;
}


static int dpp_derive_pmk(const u8 *Nx, size_t Nx_len, u8 *pmk,
			  unsigned int hash_len)
{
	u8 salt[DPP_MAX_HASH_LEN], prk[DPP_MAX_HASH_LEN];
	const char *info = "DPP PMK";
	int res;

	/* PMK = HKDF(<>, "DPP PMK", N.x) */

	/* HKDF-Extract(<>, N.x) */
	os_memset(salt, 0, hash_len);
	if (dpp_hmac(hash_len, salt, hash_len, Nx, Nx_len, prk) < 0)
		return -1;
	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: PRK = HKDF-Extract(<>, IKM=N.x)",
			prk, hash_len);

	/* HKDF-Expand(PRK, info, L) */
	res = dpp_hkdf_expand(hash_len, prk, hash_len, info, pmk, hash_len);
	os_memset(prk, 0, hash_len);
	if (res < 0)
		return -1;

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: PMK = HKDF-Expand(PRK, info, L)",
			pmk, hash_len);
	return 0;
}


static int dpp_derive_pmkid(const struct dpp_curve_params *curve,
			    EVP_PKEY *own_key, EVP_PKEY *peer_key, u8 *pmkid)
{
	struct wpabuf *nkx, *pkx;
	int ret = -1, res;
	const u8 *addr[2];
	size_t len[2];
	u8 hash[SHA256_MAC_LEN];

	/* PMKID = Truncate-128(H(min(NK.x, PK.x) | max(NK.x, PK.x))) */
	nkx = dpp_get_pubkey_point(own_key, 0);
	pkx = dpp_get_pubkey_point(peer_key, 0);
	if (!nkx || !pkx)
		goto fail;
	addr[0] = wpabuf_head(nkx);
	len[0] = wpabuf_len(nkx) / 2;
	addr[1] = wpabuf_head(pkx);
	len[1] = wpabuf_len(pkx) / 2;
	if (len[0] != len[1])
		goto fail;
	if (os_memcmp(addr[0], addr[1], len[0]) > 0) {
		addr[0] = wpabuf_head(pkx);
		addr[1] = wpabuf_head(nkx);
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: PMKID hash payload 1", addr[0], len[0]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: PMKID hash payload 2", addr[1], len[1]);
	res = sha256_vector(2, addr, len, hash);
	if (res < 0)
		goto fail;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: PMKID hash output", hash, SHA256_MAC_LEN);
	os_memcpy(pmkid, hash, PMKID_LEN);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: PMKID", pmkid, PMKID_LEN);
	ret = 0;
fail:
	wpabuf_free(nkx);
	wpabuf_free(pkx);
	return ret;
}


enum dpp_status_error
dpp_peer_intro(struct dpp_introduction *intro, const char *own_connector,
	       const u8 *net_access_key, size_t net_access_key_len,
	       const u8 *csign_key, size_t csign_key_len,
	       const u8 *peer_connector, size_t peer_connector_len,
	       os_time_t *expiry)
{
	struct json_token *root = NULL, *netkey, *token;
	struct json_token *own_root = NULL;
	enum dpp_status_error ret = 255, res;
	EVP_PKEY *own_key = NULL, *peer_key = NULL;
	struct wpabuf *own_key_pub = NULL;
	const struct dpp_curve_params *curve, *own_curve;
	struct dpp_signed_connector_info info;
	const unsigned char *p;
	EVP_PKEY *csign = NULL;
	char *signed_connector = NULL;
	const char *pos, *end;
	unsigned char *own_conn = NULL;
	size_t own_conn_len;
	EVP_PKEY_CTX *ctx = NULL;
	size_t Nx_len;
	u8 Nx[DPP_MAX_SHARED_SECRET_LEN];

	os_memset(intro, 0, sizeof(*intro));
	os_memset(&info, 0, sizeof(info));
	if (expiry)
		*expiry = 0;

	p = csign_key;
	csign = d2i_PUBKEY(NULL, &p, csign_key_len);
	if (!csign) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to parse local C-sign-key information");
		goto fail;
	}

	own_key = dpp_set_keypair(&own_curve, net_access_key,
				  net_access_key_len);
	if (!own_key) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to parse own netAccessKey");
		goto fail;
	}

	pos = os_strchr(own_connector, '.');
	if (!pos) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Own connector is missing the first dot (.)");
		goto fail;
	}
	pos++;
	end = os_strchr(pos, '.');
	if (!end) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Own connector is missing the second dot (.)");
		goto fail;
	}
	own_conn = base64_url_decode((const char *) pos,
				     end - pos, &own_conn_len);
	if (!own_conn) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to base64url decode own signedConnector JWS Payload");
		goto fail;
	}

	own_root = json_parse((const char *) own_conn, own_conn_len);
	if (!own_root) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to parse local connector");
		goto fail;
	}

	wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: Peer signedConnector",
			  peer_connector, peer_connector_len);
	signed_connector = os_malloc(peer_connector_len + 1);
	if (!signed_connector)
		goto fail;
	os_memcpy(signed_connector, peer_connector, peer_connector_len);
	signed_connector[peer_connector_len] = '\0';

	res = dpp_process_signed_connector(&info, csign, signed_connector);
	if (res != DPP_STATUS_OK) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to process peer connector with:%d",res);
		ret = res;
		goto fail;
	}

	root = json_parse((const char *) info.payload, info.payload_len);
	if (!root) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: JSON parsing of connector failed");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}

	if (!dpp_connector_match_groups(own_root, root
#ifdef DPP_R2_RECONFIG	
	, FALSE 
#endif	
	)) {
		wpa_printf(MSG_ERROR,
			   "DPP: Peer connector does not include compatible group netrole with own connector");
		ret = DPP_STATUS_NO_MATCH;
		goto fail;
	}

	token = json_get_member(root, "expiry");
	if (!token || token->type != JSON_STRING) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: No expiry string found - connector does not expire");
	} else {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: expiry = %s", token->string);
		if (dpp_key_expired(token->string, expiry)) {
			wpa_printf(MSG_ERROR, DPP_MAP_PREX
				   "DPP: Connector (netAccessKey) has expired");
			ret = DPP_STATUS_INVALID_CONNECTOR;
			goto fail;
		}
	}

	netkey = json_get_member(root, "netAccessKey");
	if (!netkey || netkey->type != JSON_OBJECT) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: No netAccessKey object found");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}

	peer_key = dpp_parse_jwk(netkey, &curve);
	if (!peer_key) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: No peer key found");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}
	dpp_debug_print_key("DPP: Received netAccessKey", peer_key);

	if (own_curve != curve) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Mismatching netAccessKey curves (%s != %s)",
			   own_curve->name, curve->name);
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}

	/* ECDH: N = nk * PK */
	ctx = EVP_PKEY_CTX_new(own_key, NULL);
	if (!ctx ||
	    EVP_PKEY_derive_init(ctx) != 1 ||
	    EVP_PKEY_derive_set_peer(ctx, peer_key) != 1 ||
	    EVP_PKEY_derive(ctx, NULL, &Nx_len) != 1 ||
	    Nx_len > DPP_MAX_SHARED_SECRET_LEN ||
	    EVP_PKEY_derive(ctx, Nx, &Nx_len) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to derive ECDH shared secret: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: ECDH shared secret (N.x)",
			Nx, Nx_len);

	/* PMK = HKDF(<>, "DPP PMK", N.x) */
	if (dpp_derive_pmk(Nx, Nx_len, intro->pmk, curve->hash_len) < 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to derive PMK");
		goto fail;
	}
	intro->pmk_len = curve->hash_len;

	/* PMKID = Truncate-128(H(min(NK.x, PK.x) | max(NK.x, PK.x))) */
	if (dpp_derive_pmkid(curve, own_key, peer_key, intro->pmkid) < 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to derive PMKID");
		goto fail;
	}

	ret = DPP_STATUS_OK;
fail:
	if (ret != DPP_STATUS_OK)
		os_memset(intro, 0, sizeof(*intro));
	os_memset(Nx, 0, sizeof(Nx));
	EVP_PKEY_CTX_free(ctx);
	os_free(own_conn);
	os_free(signed_connector);
	os_free(info.payload);
	EVP_PKEY_free(own_key);
	wpabuf_free(own_key_pub);
	EVP_PKEY_free(peer_key);
	EVP_PKEY_free(csign);
	json_free(root);
	json_free(own_root);
	return ret;
}


static EVP_PKEY * dpp_pkex_get_role_elem(const struct dpp_curve_params *curve,
					 int init)
{
	EC_GROUP *group;
	size_t len = curve->prime_len;
	const u8 *x, *y;

	switch (curve->ike_group) {
	case 19:
		x = init ? pkex_init_x_p256 : pkex_resp_x_p256;
		y = init ? pkex_init_y_p256 : pkex_resp_y_p256;
		break;
	case 20:
		x = init ? pkex_init_x_p384 : pkex_resp_x_p384;
		y = init ? pkex_init_y_p384 : pkex_resp_y_p384;
		break;
	case 21:
		x = init ? pkex_init_x_p521 : pkex_resp_x_p521;
		y = init ? pkex_init_y_p521 : pkex_resp_y_p521;
		break;
	case 28:
		x = init ? pkex_init_x_bp_p256r1 : pkex_resp_x_bp_p256r1;
		y = init ? pkex_init_y_bp_p256r1 : pkex_resp_y_bp_p256r1;
		break;
	case 29:
		x = init ? pkex_init_x_bp_p384r1 : pkex_resp_x_bp_p384r1;
		y = init ? pkex_init_y_bp_p384r1 : pkex_resp_y_bp_p384r1;
		break;
	case 30:
		x = init ? pkex_init_x_bp_p512r1 : pkex_resp_x_bp_p512r1;
		y = init ? pkex_init_y_bp_p512r1 : pkex_resp_y_bp_p512r1;
		break;
	default:
		return NULL;
	}

	group = EC_GROUP_new_by_curve_name(OBJ_txt2nid(curve->name));
	if (!group)
		return NULL;
	return dpp_set_pubkey_point_group(group, x, y, len);
}


static EC_POINT * dpp_pkex_derive_Qi(const struct dpp_curve_params *curve,
				     const u8 *mac_init, const char *code,
				     const char *identifier, BN_CTX *bnctx,
				     const EC_GROUP **ret_group)
{
	u8 hash[DPP_MAX_HASH_LEN];
	const u8 *addr[3];
	size_t len[3];
	unsigned int num_elem = 0;
	EC_POINT *Qi = NULL;
	EVP_PKEY *Pi = NULL;
	EC_KEY *Pi_ec = NULL;
	const EC_POINT *Pi_point;
	BIGNUM *hash_bn = NULL;
	const EC_GROUP *group = NULL;
	EC_GROUP *group2 = NULL;

	/* Qi = H(MAC-Initiator | [identifier |] code) * Pi */

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: MAC-Initiator: " MACSTR, MAC2STR(mac_init));
	addr[num_elem] = mac_init;
	len[num_elem] = ETH_ALEN;
	num_elem++;
	if (identifier) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: code identifier: %s",
			   identifier);
		addr[num_elem] = (const u8 *) identifier;
		len[num_elem] = os_strlen(identifier);
		num_elem++;
	}
	wpa_hexdump_ascii_key(MSG_DEBUG, DPP_MAP_PREX "DPP: code", code, os_strlen(code));
	addr[num_elem] = (const u8 *) code;
	len[num_elem] = os_strlen(code);
	num_elem++;
	if (dpp_hash_vector(curve, num_elem, addr, len, hash) < 0)
		goto fail;
	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX
			"DPP: H(MAC-Initiator | [identifier |] code)",
			hash, curve->hash_len);
	Pi = dpp_pkex_get_role_elem(curve, 1);
	if (!Pi)
		goto fail;
	dpp_debug_print_key("DPP: Pi", Pi);
	Pi_ec = EVP_PKEY_get1_EC_KEY(Pi);
	if (!Pi_ec)
		goto fail;
	Pi_point = EC_KEY_get0_public_key(Pi_ec);

	group = EC_KEY_get0_group(Pi_ec);
	if (!group)
		goto fail;
	group2 = EC_GROUP_dup(group);
	if (!group2)
		goto fail;
	Qi = EC_POINT_new(group2);
	if (!Qi) {
		EC_GROUP_free(group2);
		goto fail;
	}
	hash_bn = BN_bin2bn(hash, curve->hash_len, NULL);
	if (!hash_bn ||
	    EC_POINT_mul(group2, Qi, NULL, Pi_point, hash_bn, bnctx) != 1)
		goto fail;
	if (EC_POINT_is_at_infinity(group, Qi)) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Qi is the point-at-infinity");
		goto fail;
	}
	dpp_debug_print_point("DPP: Qi", group, Qi);
out:
	EC_KEY_free(Pi_ec);
	EVP_PKEY_free(Pi);
	BN_clear_free(hash_bn);
	if (ret_group)
		*ret_group = group2;
	return Qi;
fail:
	EC_POINT_free(Qi);
	Qi = NULL;
	goto out;
}


static EC_POINT * dpp_pkex_derive_Qr(const struct dpp_curve_params *curve,
				     const u8 *mac_resp, const char *code,
				     const char *identifier, BN_CTX *bnctx,
				     const EC_GROUP **ret_group)
{
	u8 hash[DPP_MAX_HASH_LEN];
	const u8 *addr[3];
	size_t len[3];
	unsigned int num_elem = 0;
	EC_POINT *Qr = NULL;
	EVP_PKEY *Pr = NULL;
	EC_KEY *Pr_ec = NULL;
	const EC_POINT *Pr_point;
	BIGNUM *hash_bn = NULL;
	const EC_GROUP *group = NULL;
	EC_GROUP *group2 = NULL;

	/* Qr = H(MAC-Responder | | [identifier | ] code) * Pr */

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: MAC-Responder: " MACSTR, MAC2STR(mac_resp));
	addr[num_elem] = mac_resp;
	len[num_elem] = ETH_ALEN;
	num_elem++;
	if (identifier) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: code identifier: %s",
			   identifier);
		addr[num_elem] = (const u8 *) identifier;
		len[num_elem] = os_strlen(identifier);
		num_elem++;
	}
	wpa_hexdump_ascii_key(MSG_DEBUG, DPP_MAP_PREX "DPP: code", code, os_strlen(code));
	addr[num_elem] = (const u8 *) code;
	len[num_elem] = os_strlen(code);
	num_elem++;
	if (dpp_hash_vector(curve, num_elem, addr, len, hash) < 0)
		goto fail;
	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX
			"DPP: H(MAC-Responder | [identifier |] code)",
			hash, curve->hash_len);
	Pr = dpp_pkex_get_role_elem(curve, 0);
	if (!Pr)
		goto fail;
	dpp_debug_print_key("DPP: Pr", Pr);
	Pr_ec = EVP_PKEY_get1_EC_KEY(Pr);
	if (!Pr_ec)
		goto fail;
	Pr_point = EC_KEY_get0_public_key(Pr_ec);

	group = EC_KEY_get0_group(Pr_ec);
	if (!group)
		goto fail;
	group2 = EC_GROUP_dup(group);
	if (!group2)
		goto fail;
	Qr = EC_POINT_new(group2);
	if (!Qr) {
		EC_GROUP_free(group2);
		goto fail;
	}
	hash_bn = BN_bin2bn(hash, curve->hash_len, NULL);
	if (!hash_bn ||
	    EC_POINT_mul(group2, Qr, NULL, Pr_point, hash_bn, bnctx) != 1)
		goto fail;
	if (EC_POINT_is_at_infinity(group, Qr)) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Qr is the point-at-infinity");
		goto fail;
	}
	dpp_debug_print_point("DPP: Qr", group, Qr);
out:
	EC_KEY_free(Pr_ec);
	EVP_PKEY_free(Pr);
	BN_clear_free(hash_bn);
	if (ret_group)
		*ret_group = group2;
	return Qr;
fail:
	EC_POINT_free(Qr);
	Qr = NULL;
	goto out;
}

static struct wpabuf * dpp_pkex_build_exchange_req(struct dpp_pkex *pkex)
{
	EC_KEY *X_ec = NULL;
	const EC_POINT *X_point;
	BN_CTX *bnctx = NULL;
	const EC_GROUP *group;
	EC_POINT *Qi = NULL, *M = NULL;
	struct wpabuf *M_buf = NULL;
	BIGNUM *Mx = NULL, *My = NULL;
	struct wpabuf *msg = NULL;
	size_t attr_len;
	const struct dpp_curve_params *curve = pkex->own_bi->curve;

	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Build PKEX Exchange Request");

	/* Qi = H(MAC-Initiator | [identifier |] code) * Pi */
	bnctx = BN_CTX_new();
	if (!bnctx)
		goto fail;
	Qi = dpp_pkex_derive_Qi(curve, pkex->own_mac, pkex->code,
				pkex->identifier, bnctx, &group);
	if (!Qi)
		goto fail;

	/* Generate a random ephemeral keypair x/X */
	pkex->x = dpp_gen_keypair(curve);
	if (!pkex->x)
		goto fail;

	/* M = X + Qi */
	X_ec = EVP_PKEY_get1_EC_KEY(pkex->x);
	if (!X_ec)
		goto fail;
	X_point = EC_KEY_get0_public_key(X_ec);
	if (!X_point)
		goto fail;
	dpp_debug_print_point("DPP: X", group, X_point);
	M = EC_POINT_new(group);
	Mx = BN_new();
	My = BN_new();
	if (!M || !Mx || !My ||
	    EC_POINT_add(group, M, X_point, Qi, bnctx) != 1 ||
	    EC_POINT_get_affine_coordinates_GFp(group, M, Mx, My, bnctx) != 1)
		goto fail;
	dpp_debug_print_point("DPP: M", group, M);

	/* Initiator -> Responder: group, [identifier,] M */
	attr_len = 4 + 2;
	if (pkex->identifier)
		attr_len += 4 + os_strlen(pkex->identifier);
	attr_len += 4 + 2 * curve->prime_len;
	msg = dpp_alloc_msg(DPP_PA_PKEX_EXCHANGE_REQ, attr_len);
	if (!msg)
		goto fail;

	/* Finite Cyclic Group attribute */
	wpabuf_put_le16(msg, DPP_ATTR_FINITE_CYCLIC_GROUP);
	wpabuf_put_le16(msg, 2);
	wpabuf_put_le16(msg, curve->ike_group);

	/* Code Identifier attribute */
	if (pkex->identifier) {
		wpabuf_put_le16(msg, DPP_ATTR_CODE_IDENTIFIER);
		wpabuf_put_le16(msg, os_strlen(pkex->identifier));
		wpabuf_put_str(msg, pkex->identifier);
	}

	/* M in Encrypted Key attribute */
	wpabuf_put_le16(msg, DPP_ATTR_ENCRYPTED_KEY);
	wpabuf_put_le16(msg, 2 * curve->prime_len);

	if (dpp_bn2bin_pad(Mx, wpabuf_put(msg, curve->prime_len),
			   curve->prime_len) < 0 ||
	    dpp_bn2bin_pad(Mx, pkex->Mx, curve->prime_len) < 0 ||
	    dpp_bn2bin_pad(My, wpabuf_put(msg, curve->prime_len),
			   curve->prime_len) < 0)
		goto fail;

out:
	wpabuf_free(M_buf);
	EC_KEY_free(X_ec);
	EC_POINT_free(M);
	EC_POINT_free(Qi);
	BN_clear_free(Mx);
	BN_clear_free(My);
	BN_CTX_free(bnctx);
	return msg;
fail:
	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Failed to build PKEX Exchange Request");
	wpabuf_free(msg);
	msg = NULL;
	goto out;
}


static void dpp_pkex_fail(struct dpp_pkex *pkex, const char *txt)
{
}


struct dpp_pkex * dpp_pkex_init(void *msg_ctx, struct dpp_bootstrap_info *bi,
				const u8 *own_mac,
				const char *identifier,
				const char *code)
{
	struct dpp_pkex *pkex;

	pkex = os_zalloc(sizeof(*pkex));
	if (!pkex)
		return NULL;
	pkex->msg_ctx = msg_ctx;
	pkex->initiator = 1;
	pkex->own_bi = bi;
	os_memcpy(pkex->own_mac, own_mac, ETH_ALEN);
	if (identifier) {
		pkex->identifier = os_strdup(identifier);
		if (!pkex->identifier)
			goto fail;
	}
	pkex->code = os_strdup(code);
	if (!pkex->code)
		goto fail;
	pkex->exchange_req = dpp_pkex_build_exchange_req(pkex);
	if (!pkex->exchange_req)
		goto fail;
	return pkex;
fail:
	dpp_pkex_free(pkex);
	return NULL;
}


static struct wpabuf *
dpp_pkex_build_exchange_resp(struct dpp_pkex *pkex,
			     enum dpp_status_error status,
			     const BIGNUM *Nx, const BIGNUM *Ny)
{
	struct wpabuf *msg = NULL;
	size_t attr_len;
	const struct dpp_curve_params *curve = pkex->own_bi->curve;

	/* Initiator -> Responder: DPP Status, [identifier,] N */
	attr_len = 4 + 1;
	if (pkex->identifier)
		attr_len += 4 + os_strlen(pkex->identifier);
	attr_len += 4 + 2 * curve->prime_len;
	msg = dpp_alloc_msg(DPP_PA_PKEX_EXCHANGE_RESP, attr_len);
	if (!msg)
		goto fail;

	/* DPP Status */
	dpp_build_attr_status(msg, status);

	/* Code Identifier attribute */
	if (pkex->identifier) {
		wpabuf_put_le16(msg, DPP_ATTR_CODE_IDENTIFIER);
		wpabuf_put_le16(msg, os_strlen(pkex->identifier));
		wpabuf_put_str(msg, pkex->identifier);
	}

	if (status != DPP_STATUS_OK)
		goto skip_encrypted_key;

	/* N in Encrypted Key attribute */
	wpabuf_put_le16(msg, DPP_ATTR_ENCRYPTED_KEY);
	wpabuf_put_le16(msg, 2 * curve->prime_len);

	if (dpp_bn2bin_pad(Nx, wpabuf_put(msg, curve->prime_len),
			   curve->prime_len) < 0 ||
	    dpp_bn2bin_pad(Nx, pkex->Nx, curve->prime_len) < 0 ||
	    dpp_bn2bin_pad(Ny, wpabuf_put(msg, curve->prime_len),
			   curve->prime_len) < 0)
		goto fail;

skip_encrypted_key:
	if (status == DPP_STATUS_BAD_GROUP) {
		/* Finite Cyclic Group attribute */
		wpabuf_put_le16(msg, DPP_ATTR_FINITE_CYCLIC_GROUP);
		wpabuf_put_le16(msg, 2);
		wpabuf_put_le16(msg, curve->ike_group);
	}

	return msg;
fail:
	wpabuf_free(msg);
	return NULL;
}


static int dpp_pkex_derive_z(const u8 *mac_init, const u8 *mac_resp,
			     const u8 *Mx, size_t Mx_len,
			     const u8 *Nx, size_t Nx_len,
			     const char *code,
			     const u8 *Kx, size_t Kx_len,
			     u8 *z, unsigned int hash_len)
{
	u8 salt[DPP_MAX_HASH_LEN], prk[DPP_MAX_HASH_LEN];
	int res;
	u8 *info, *pos;
	size_t info_len;

	/* z = HKDF(<>, MAC-Initiator | MAC-Responder | M.x | N.x | code, K.x)
	 */

	/* HKDF-Extract(<>, IKM=K.x) */
	os_memset(salt, 0, hash_len);
	if (dpp_hmac(hash_len, salt, hash_len, Kx, Kx_len, prk) < 0)
		return -1;
	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: PRK = HKDF-Extract(<>, IKM)",
			prk, hash_len);
	info_len = 2 * ETH_ALEN + Mx_len + Nx_len + os_strlen(code);
	info = os_malloc(info_len);
	if (!info)
		return -1;
	pos = info;
	os_memcpy(pos, mac_init, ETH_ALEN);
	pos += ETH_ALEN;
	os_memcpy(pos, mac_resp, ETH_ALEN);
	pos += ETH_ALEN;
	os_memcpy(pos, Mx, Mx_len);
	pos += Mx_len;
	os_memcpy(pos, Nx, Nx_len);
	pos += Nx_len;
	os_memcpy(pos, code, os_strlen(code));

	/* HKDF-Expand(PRK, info, L) */
	if (hash_len == 32)
		res = hmac_sha256_kdf(prk, hash_len, NULL, info, info_len,
				      z, hash_len);
	else if (hash_len == 48)
		res = hmac_sha384_kdf(prk, hash_len, NULL, info, info_len,
				      z, hash_len);
	else if (hash_len == 64)
		res = hmac_sha512_kdf(prk, hash_len, NULL, info, info_len,
				      z, hash_len);
	else
		res = -1;
	os_free(info);
	os_memset(prk, 0, hash_len);
	if (res < 0)
		return -1;

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: z = HKDF-Expand(PRK, info, L)",
			z, hash_len);
	return 0;
}


struct dpp_pkex * dpp_pkex_rx_exchange_req(void *msg_ctx,
					   struct dpp_bootstrap_info *bi,
					   const u8 *own_mac,
					   const u8 *peer_mac,
					   const char *identifier,
					   const char *code,
					   const u8 *buf, size_t len)
{
	const u8 *attr_group, *attr_id, *attr_key;
	u16 attr_group_len, attr_id_len, attr_key_len;
	const struct dpp_curve_params *curve = bi->curve;
	u16 ike_group;
	struct dpp_pkex *pkex = NULL;
	EC_POINT *Qi = NULL, *Qr = NULL, *M = NULL, *X = NULL, *N = NULL;
	BN_CTX *bnctx = NULL;
	const EC_GROUP *group;
	BIGNUM *Mx = NULL, *My = NULL;
	EC_KEY *Y_ec = NULL, *X_ec = NULL;;
	const EC_POINT *Y_point;
	BIGNUM *Nx = NULL, *Ny = NULL;
	u8 Kx[DPP_MAX_SHARED_SECRET_LEN];
	size_t Kx_len;
	int res;
	EVP_PKEY_CTX *ctx = NULL;

	if (bi->pkex_t >= PKEX_COUNTER_T_LIMIT) {
		return NULL;
	}

	attr_id_len = 0;
	attr_id = dpp_get_attr(buf, len, DPP_ATTR_CODE_IDENTIFIER,
			       &attr_id_len);
	if (!attr_id && identifier) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: No PKEX code identifier received, but expected one");
		return NULL;
	}
	if (attr_id && identifier &&
	    (os_strlen(identifier) != attr_id_len ||
	     os_memcmp(identifier, attr_id, attr_id_len) != 0)) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: PKEX code identifier mismatch");
		return NULL;
	}

	attr_group = dpp_get_attr(buf, len, DPP_ATTR_FINITE_CYCLIC_GROUP,
				  &attr_group_len);
	if (!attr_group || attr_group_len != 2) {
		return NULL;
	}
	ike_group = WPA_GET_LE16(attr_group);
	if (ike_group != curve->ike_group) {
		pkex = os_zalloc(sizeof(*pkex));
		if (!pkex)
			goto fail;
		pkex->own_bi = bi;
		pkex->failed = 1;
		pkex->exchange_resp = dpp_pkex_build_exchange_resp(
			pkex, DPP_STATUS_BAD_GROUP, NULL, NULL);
		if (!pkex->exchange_resp)
			goto fail;
		return pkex;
	}

	/* M in Encrypted Key attribute */
	attr_key = dpp_get_attr(buf, len, DPP_ATTR_ENCRYPTED_KEY,
				&attr_key_len);
	if (!attr_key || attr_key_len & 0x01 || attr_key_len < 2 ||
	    attr_key_len / 2 > DPP_MAX_SHARED_SECRET_LEN) {
		return NULL;
	}

	/* Qi = H(MAC-Initiator | [identifier |] code) * Pi */
	bnctx = BN_CTX_new();
	if (!bnctx)
		goto fail;
	Qi = dpp_pkex_derive_Qi(curve, peer_mac, code, identifier, bnctx,
				&group);
	if (!Qi)
		goto fail;

	/* X' = M - Qi */
	X = EC_POINT_new(group);
	M = EC_POINT_new(group);
	Mx = BN_bin2bn(attr_key, attr_key_len / 2, NULL);
	My = BN_bin2bn(attr_key + attr_key_len / 2, attr_key_len / 2, NULL);
	if (!X || !M || !Mx || !My ||
	    EC_POINT_set_affine_coordinates_GFp(group, M, Mx, My, bnctx) != 1 ||
	    EC_POINT_is_at_infinity(group, M) ||
	    !EC_POINT_is_on_curve(group, M, bnctx) ||
	    EC_POINT_invert(group, Qi, bnctx) != 1 ||
	    EC_POINT_add(group, X, M, Qi, bnctx) != 1 ||
	    EC_POINT_is_at_infinity(group, X) ||
	    !EC_POINT_is_on_curve(group, X, bnctx)) {
		bi->pkex_t++;
		goto fail;
	}
	dpp_debug_print_point("DPP: M", group, M);
	dpp_debug_print_point("DPP: X'", group, X);

	pkex = os_zalloc(sizeof(*pkex));
	if (!pkex)
		goto fail;
	pkex->t = bi->pkex_t;
	pkex->msg_ctx = msg_ctx;
	pkex->own_bi = bi;
	os_memcpy(pkex->own_mac, own_mac, ETH_ALEN);
	os_memcpy(pkex->peer_mac, peer_mac, ETH_ALEN);
	if (identifier) {
		pkex->identifier = os_strdup(identifier);
		if (!pkex->identifier)
			goto fail;
	}
	pkex->code = os_strdup(code);
	if (!pkex->code)
		goto fail;

	os_memcpy(pkex->Mx, attr_key, attr_key_len / 2);

	X_ec = EC_KEY_new();
	if (!X_ec ||
	    EC_KEY_set_group(X_ec, group) != 1 ||
	    EC_KEY_set_public_key(X_ec, X) != 1)
		goto fail;
	pkex->x = EVP_PKEY_new();
	if (!pkex->x ||
	    EVP_PKEY_set1_EC_KEY(pkex->x, X_ec) != 1)
		goto fail;

	/* Qr = H(MAC-Responder | | [identifier | ] code) * Pr */
	Qr = dpp_pkex_derive_Qr(curve, own_mac, code, identifier, bnctx, NULL);
	if (!Qr)
		goto fail;

	/* Generate a random ephemeral keypair y/Y */
	pkex->y = dpp_gen_keypair(curve);
	if (!pkex->y)
		goto fail;

	/* N = Y + Qr */
	Y_ec = EVP_PKEY_get1_EC_KEY(pkex->y);
	if (!Y_ec)
		goto fail;
	Y_point = EC_KEY_get0_public_key(Y_ec);
	if (!Y_point)
		goto fail;
	dpp_debug_print_point("DPP: Y", group, Y_point);
	N = EC_POINT_new(group);
	Nx = BN_new();
	Ny = BN_new();
	if (!N || !Nx || !Ny ||
	    EC_POINT_add(group, N, Y_point, Qr, bnctx) != 1 ||
	    EC_POINT_get_affine_coordinates_GFp(group, N, Nx, Ny, bnctx) != 1)
		goto fail;
	dpp_debug_print_point("DPP: N", group, N);

	pkex->exchange_resp = dpp_pkex_build_exchange_resp(pkex, DPP_STATUS_OK,
							   Nx, Ny);
	if (!pkex->exchange_resp)
		goto fail;

	/* K = y * X' */
	ctx = EVP_PKEY_CTX_new(pkex->y, NULL);
	if (!ctx ||
	    EVP_PKEY_derive_init(ctx) != 1 ||
	    EVP_PKEY_derive_set_peer(ctx, pkex->x) != 1 ||
	    EVP_PKEY_derive(ctx, NULL, &Kx_len) != 1 ||
	    Kx_len > DPP_MAX_SHARED_SECRET_LEN ||
	    EVP_PKEY_derive(ctx, Kx, &Kx_len) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to derive ECDH shared secret: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: ECDH shared secret (K.x)",
			Kx, Kx_len);

	/* z = HKDF(<>, MAC-Initiator | MAC-Responder | M.x | N.x | code, K.x)
	 */
	res = dpp_pkex_derive_z(pkex->peer_mac, pkex->own_mac,
				pkex->Mx, curve->prime_len,
				pkex->Nx, curve->prime_len, pkex->code,
				Kx, Kx_len, pkex->z, curve->hash_len);
	os_memset(Kx, 0, Kx_len);
	if (res < 0)
		goto fail;

	pkex->exchange_done = 1;

out:
	EVP_PKEY_CTX_free(ctx);
	BN_CTX_free(bnctx);
	EC_POINT_free(Qi);
	EC_POINT_free(Qr);
	BN_free(Mx);
	BN_free(My);
	BN_free(Nx);
	BN_free(Ny);
	EC_POINT_free(M);
	EC_POINT_free(N);
	EC_POINT_free(X);
	EC_KEY_free(X_ec);
	EC_KEY_free(Y_ec);
	return pkex;
fail:
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: PKEX Exchange Request processing failed");
	dpp_pkex_free(pkex);
	pkex = NULL;
	goto out;
}

static struct wpabuf *
dpp_pkex_build_commit_reveal_req(struct dpp_pkex *pkex,
				 const struct wpabuf *A_pub, const u8 *u)
{
	const struct dpp_curve_params *curve = pkex->own_bi->curve;
	struct wpabuf *msg = NULL;
	size_t clear_len, attr_len;
	struct wpabuf *clear = NULL;
	u8 *wrapped;
	u8 octet;
	const u8 *addr[2];
	size_t len[2];

	/* {A, u, [bootstrapping info]}z */
	clear_len = 4 + 2 * curve->prime_len + 4 + curve->hash_len;
	clear = wpabuf_alloc(clear_len);
	attr_len = 4 + clear_len + AES_BLOCK_SIZE;
	msg = dpp_alloc_msg(DPP_PA_PKEX_COMMIT_REVEAL_REQ, attr_len);
	if (!clear || !msg)
		goto fail;

	/* A in Bootstrap Key attribute */
	wpabuf_put_le16(clear, DPP_ATTR_BOOTSTRAP_KEY);
	wpabuf_put_le16(clear, wpabuf_len(A_pub));
	wpabuf_put_buf(clear, A_pub);

	/* u in I-Auth tag attribute */
	wpabuf_put_le16(clear, DPP_ATTR_I_AUTH_TAG);
	wpabuf_put_le16(clear, curve->hash_len);
	wpabuf_put_data(clear, u, curve->hash_len);

	addr[0] = wpabuf_head_u8(msg) + 2;
	len[0] = DPP_HDR_LEN;
	octet = 0;
	addr[1] = &octet;
	len[1] = sizeof(octet);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);

	wpabuf_put_le16(msg, DPP_ATTR_WRAPPED_DATA);
	wpabuf_put_le16(msg, wpabuf_len(clear) + AES_BLOCK_SIZE);
	wrapped = wpabuf_put(msg, wpabuf_len(clear) + AES_BLOCK_SIZE);

	wpa_hexdump_buf(MSG_DEBUG, "DPP: AES-SIV cleartext", clear);
	if (aes_siv_encrypt(pkex->z, curve->hash_len,
			    wpabuf_head(clear), wpabuf_len(clear),
			    2, addr, len, wrapped) < 0)
		goto fail;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped, wpabuf_len(clear) + AES_BLOCK_SIZE);

out:
	wpabuf_free(clear);
	return msg;

fail:
	wpabuf_free(msg);
	msg = NULL;
	goto out;
}

struct wpabuf * dpp_pkex_rx_exchange_resp(struct dpp_pkex *pkex,
					  const u8 *peer_mac,
					  const u8 *buf, size_t buflen)
{
	const u8 *attr_status, *attr_id, *attr_key, *attr_group;
	u16 attr_status_len, attr_id_len, attr_key_len, attr_group_len;
	const EC_GROUP *group;
	BN_CTX *bnctx = NULL;
	struct wpabuf *msg = NULL, *A_pub = NULL, *X_pub = NULL, *Y_pub = NULL;
	const struct dpp_curve_params *curve = pkex->own_bi->curve;
	EC_POINT *Qr = NULL, *Y = NULL, *N = NULL;
	BIGNUM *Nx = NULL, *Ny = NULL;
	EVP_PKEY_CTX *ctx = NULL;
	EC_KEY *Y_ec = NULL;
	size_t Jx_len, Kx_len;
	u8 Jx[DPP_MAX_SHARED_SECRET_LEN], Kx[DPP_MAX_SHARED_SECRET_LEN];
	const u8 *addr[4];
	size_t len[4];
	u8 u[DPP_MAX_HASH_LEN];
	int res;

	if (pkex->failed || pkex->t >= PKEX_COUNTER_T_LIMIT || !pkex->initiator)
		return NULL;

	os_memcpy(pkex->peer_mac, peer_mac, ETH_ALEN);

	attr_status = dpp_get_attr(buf, buflen, DPP_ATTR_STATUS,
				   &attr_status_len);
	if (!attr_status || attr_status_len != 1) {
		dpp_pkex_fail(pkex, "No DPP Status attribute");
		return NULL;
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Status %u", attr_status[0]);

	if (attr_status[0] == DPP_STATUS_BAD_GROUP) {
		attr_group = dpp_get_attr(buf, buflen,
					  DPP_ATTR_FINITE_CYCLIC_GROUP,
					  &attr_group_len);
		if (attr_group && attr_group_len == 2) {
			return NULL;
		}
	}

	if (attr_status[0] != DPP_STATUS_OK) {
		dpp_pkex_fail(pkex, "PKEX failed (peer indicated failure)");
		return NULL;
	}

	attr_id_len = 0;
	attr_id = dpp_get_attr(buf, buflen, DPP_ATTR_CODE_IDENTIFIER,
			       &attr_id_len);
	if (!attr_id && pkex->identifier) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: No PKEX code identifier received, but expected one");
		return NULL;
	}
	if (attr_id && pkex->identifier &&
	    (os_strlen(pkex->identifier) != attr_id_len ||
	     os_memcmp(pkex->identifier, attr_id, attr_id_len) != 0)) {
		dpp_pkex_fail(pkex, "PKEX code identifier mismatch");
		return NULL;
	}

	/* N in Encrypted Key attribute */
	attr_key = dpp_get_attr(buf, buflen, DPP_ATTR_ENCRYPTED_KEY,
				&attr_key_len);
	if (!attr_key || attr_key_len & 0x01 || attr_key_len < 2) {
		dpp_pkex_fail(pkex, "Missing Encrypted Key attribute");
		return NULL;
	}

	/* Qr = H(MAC-Responder | [identifier |] code) * Pr */
	bnctx = BN_CTX_new();
	if (!bnctx)
		goto fail;
	Qr = dpp_pkex_derive_Qr(curve, pkex->peer_mac, pkex->code,
				pkex->identifier, bnctx, &group);
	if (!Qr)
		goto fail;

	/* Y' = N - Qr */
	Y = EC_POINT_new(group);
	N = EC_POINT_new(group);
	Nx = BN_bin2bn(attr_key, attr_key_len / 2, NULL);
	Ny = BN_bin2bn(attr_key + attr_key_len / 2, attr_key_len / 2, NULL);
	if (!Y || !N || !Nx || !Ny ||
	    EC_POINT_set_affine_coordinates_GFp(group, N, Nx, Ny, bnctx) != 1 ||
	    EC_POINT_is_at_infinity(group, N) ||
	    !EC_POINT_is_on_curve(group, N, bnctx) ||
	    EC_POINT_invert(group, Qr, bnctx) != 1 ||
	    EC_POINT_add(group, Y, N, Qr, bnctx) != 1 ||
	    EC_POINT_is_at_infinity(group, Y) ||
	    !EC_POINT_is_on_curve(group, Y, bnctx)) {
		dpp_pkex_fail(pkex, "Invalid Encrypted Key value");
		pkex->t++;
		goto fail;
	}
	dpp_debug_print_point("DPP: N", group, N);
	dpp_debug_print_point("DPP: Y'", group, Y);

	pkex->exchange_done = 1;

	/* ECDH: J = a * Y??*/
	Y_ec = EC_KEY_new();
	if (!Y_ec ||
	    EC_KEY_set_group(Y_ec, group) != 1 ||
	    EC_KEY_set_public_key(Y_ec, Y) != 1)
		goto fail;
	pkex->y = EVP_PKEY_new();
	if (!pkex->y ||
	    EVP_PKEY_set1_EC_KEY(pkex->y, Y_ec) != 1)
		goto fail;
	ctx = EVP_PKEY_CTX_new(pkex->own_bi->pubkey, NULL);
	if (!ctx ||
	    EVP_PKEY_derive_init(ctx) != 1 ||
	    EVP_PKEY_derive_set_peer(ctx, pkex->y) != 1 ||
	    EVP_PKEY_derive(ctx, NULL, &Jx_len) != 1 ||
	    Jx_len > DPP_MAX_SHARED_SECRET_LEN ||
	    EVP_PKEY_derive(ctx, Jx, &Jx_len) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to derive ECDH shared secret: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: ECDH shared secret (J.x)",
			Jx, Jx_len);

	/* u = HMAC(J.x,  MAC-Initiator | A.x | Y??x | X.x ) */
	A_pub = dpp_get_pubkey_point(pkex->own_bi->pubkey, 0);
	Y_pub = dpp_get_pubkey_point(pkex->y, 0);
	X_pub = dpp_get_pubkey_point(pkex->x, 0);
	if (!A_pub || !Y_pub || !X_pub)
		goto fail;
	addr[0] = pkex->own_mac;
	len[0] = ETH_ALEN;
	addr[1] = wpabuf_head(A_pub);
	len[1] = wpabuf_len(A_pub) / 2;
	addr[2] = wpabuf_head(Y_pub);
	len[2] = wpabuf_len(Y_pub) / 2;
	addr[3] = wpabuf_head(X_pub);
	len[3] = wpabuf_len(X_pub) / 2;
	if (dpp_hmac_vector(curve->hash_len, Jx, Jx_len, 4, addr, len, u) < 0)
		goto fail;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: u", u, curve->hash_len);

	/* K = x * Y??*/
	EVP_PKEY_CTX_free(ctx);
	ctx = EVP_PKEY_CTX_new(pkex->x, NULL);
	if (!ctx ||
	    EVP_PKEY_derive_init(ctx) != 1 ||
	    EVP_PKEY_derive_set_peer(ctx, pkex->y) != 1 ||
	    EVP_PKEY_derive(ctx, NULL, &Kx_len) != 1 ||
	    Kx_len > DPP_MAX_SHARED_SECRET_LEN ||
	    EVP_PKEY_derive(ctx, Kx, &Kx_len) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to derive ECDH shared secret: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: ECDH shared secret (K.x)",
			Kx, Kx_len);

	/* z = HKDF(<>, MAC-Initiator | MAC-Responder | M.x | N.x | code, K.x)
	 */
	res = dpp_pkex_derive_z(pkex->own_mac, pkex->peer_mac,
				pkex->Mx, curve->prime_len,
				attr_key /* N.x */, attr_key_len / 2,
				pkex->code, Kx, Kx_len,
				pkex->z, curve->hash_len);
	os_memset(Kx, 0, Kx_len);
	if (res < 0)
		goto fail;

	msg = dpp_pkex_build_commit_reveal_req(pkex, A_pub, u);
	if (!msg)
		goto fail;

out:
	wpabuf_free(A_pub);
	wpabuf_free(X_pub);
	wpabuf_free(Y_pub);
	EC_POINT_free(Qr);
	EC_POINT_free(Y);
	EC_POINT_free(N);
	BN_free(Nx);
	BN_free(Ny);
	EC_KEY_free(Y_ec);
	EVP_PKEY_CTX_free(ctx);
	BN_CTX_free(bnctx);
	return msg;
fail:
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: PKEX Exchange Response processing failed");
	goto out;
}


static struct wpabuf *
dpp_pkex_build_commit_reveal_resp(struct dpp_pkex *pkex,
				  const struct wpabuf *B_pub, const u8 *v)
{
	const struct dpp_curve_params *curve = pkex->own_bi->curve;
	struct wpabuf *msg = NULL;
	const u8 *addr[2];
	size_t len[2];
	u8 octet;
	u8 *wrapped;
	struct wpabuf *clear = NULL;
	size_t clear_len, attr_len;

	/* {B, v [bootstrapping info]}z */
	clear_len = 4 + 2 * curve->prime_len + 4 + curve->hash_len;
	clear = wpabuf_alloc(clear_len);
	attr_len = 4 + clear_len + AES_BLOCK_SIZE;
	msg = dpp_alloc_msg(DPP_PA_PKEX_COMMIT_REVEAL_RESP, attr_len);
	if (!clear || !msg)
		goto fail;

	/* B in Bootstrap Key attribute */
	wpabuf_put_le16(clear, DPP_ATTR_BOOTSTRAP_KEY);
	wpabuf_put_le16(clear, wpabuf_len(B_pub));
	wpabuf_put_buf(clear, B_pub);

	/* v in R-Auth tag attribute */
	wpabuf_put_le16(clear, DPP_ATTR_R_AUTH_TAG);
	wpabuf_put_le16(clear, curve->hash_len);
	wpabuf_put_data(clear, v, curve->hash_len);

	addr[0] = wpabuf_head_u8(msg) + 2;
	len[0] = DPP_HDR_LEN;
	octet = 1;
	addr[1] = &octet;
	len[1] = sizeof(octet);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);

	wpabuf_put_le16(msg, DPP_ATTR_WRAPPED_DATA);
	wpabuf_put_le16(msg, wpabuf_len(clear) + AES_BLOCK_SIZE);
	wrapped = wpabuf_put(msg, wpabuf_len(clear) + AES_BLOCK_SIZE);

	wpa_hexdump_buf(MSG_DEBUG, "DPP: AES-SIV cleartext", clear);
	if (aes_siv_encrypt(pkex->z, curve->hash_len,
			    wpabuf_head(clear), wpabuf_len(clear),
			    2, addr, len, wrapped) < 0)
		goto fail;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped, wpabuf_len(clear) + AES_BLOCK_SIZE);

out:
	wpabuf_free(clear);
	return msg;

fail:
	wpabuf_free(msg);
	msg = NULL;
	goto out;
}


struct wpabuf * dpp_pkex_rx_commit_reveal_req(struct dpp_pkex *pkex,
					      const u8 *hdr,
					      const u8 *buf, size_t buflen)
{
	const struct dpp_curve_params *curve = pkex->own_bi->curve;
	EVP_PKEY_CTX *ctx = NULL;
	size_t Jx_len, Lx_len;
	u8 Jx[DPP_MAX_SHARED_SECRET_LEN];
	u8 Lx[DPP_MAX_SHARED_SECRET_LEN];
	const u8 *wrapped_data, *b_key, *peer_u;
	u16 wrapped_data_len, b_key_len, peer_u_len = 0;
	const u8 *addr[4];
	size_t len[4];
	u8 octet;
	u8 *unwrapped = NULL;
	size_t unwrapped_len = 0;
	struct wpabuf *msg = NULL, *A_pub = NULL, *X_pub = NULL, *Y_pub = NULL;
	struct wpabuf *B_pub = NULL;
	u8 u[DPP_MAX_HASH_LEN], v[DPP_MAX_HASH_LEN];

	if (!pkex->exchange_done || pkex->failed ||
	    pkex->t >= PKEX_COUNTER_T_LIMIT || pkex->initiator)
		goto fail;

	wrapped_data = dpp_get_attr(buf, buflen, DPP_ATTR_WRAPPED_DATA,
				    &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		dpp_pkex_fail(pkex,
			      "Missing or invalid required Wrapped Data attribute");
		goto fail;
	}

	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped_data, wrapped_data_len);
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped = os_malloc(unwrapped_len);
	if (!unwrapped)
		goto fail;

	addr[0] = hdr;
	len[0] = DPP_HDR_LEN;
	octet = 0;
	addr[1] = &octet;
	len[1] = sizeof(octet);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);

	if (aes_siv_decrypt(pkex->z, curve->hash_len,
			    wrapped_data, wrapped_data_len,
			    2, addr, len, unwrapped) < 0) {
		dpp_pkex_fail(pkex,
			      "AES-SIV decryption failed - possible PKEX code mismatch");
		pkex->failed = 1;
		pkex->t++;
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext",
		    unwrapped, unwrapped_len);

	if (dpp_check_attrs(unwrapped, unwrapped_len) < 0) {
		dpp_pkex_fail(pkex, "Invalid attribute in unwrapped data");
		goto fail;
	}

	b_key = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_BOOTSTRAP_KEY,
			     &b_key_len);
	if (!b_key || b_key_len != 2 * curve->prime_len) {
		dpp_pkex_fail(pkex, "No valid peer bootstrapping key found");
		goto fail;
	}
	pkex->peer_bootstrap_key = dpp_set_pubkey_point(pkex->x, b_key,
							b_key_len);
	if (!pkex->peer_bootstrap_key) {
		dpp_pkex_fail(pkex, "Peer bootstrapping key is invalid");
		goto fail;
	}
	dpp_debug_print_key("DPP: Peer bootstrap public key",
			    pkex->peer_bootstrap_key);

	/* ECDH: J' = y * A' */
	ctx = EVP_PKEY_CTX_new(pkex->y, NULL);
	if (!ctx ||
	    EVP_PKEY_derive_init(ctx) != 1 ||
	    EVP_PKEY_derive_set_peer(ctx, pkex->peer_bootstrap_key) != 1 ||
	    EVP_PKEY_derive(ctx, NULL, &Jx_len) != 1 ||
	    Jx_len > DPP_MAX_SHARED_SECRET_LEN ||
	    EVP_PKEY_derive(ctx, Jx, &Jx_len) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to derive ECDH shared secret: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: ECDH shared secret (J.x)",
			Jx, Jx_len);

	/* u' = HMAC(J'.x, MAC-Initiator | A'.x | Y.x | X'.x) */
	A_pub = dpp_get_pubkey_point(pkex->peer_bootstrap_key, 0);
	Y_pub = dpp_get_pubkey_point(pkex->y, 0);
	X_pub = dpp_get_pubkey_point(pkex->x, 0);
	if (!A_pub || !Y_pub || !X_pub)
		goto fail;
	addr[0] = pkex->peer_mac;
	len[0] = ETH_ALEN;
	addr[1] = wpabuf_head(A_pub);
	len[1] = wpabuf_len(A_pub) / 2;
	addr[2] = wpabuf_head(Y_pub);
	len[2] = wpabuf_len(Y_pub) / 2;
	addr[3] = wpabuf_head(X_pub);
	len[3] = wpabuf_len(X_pub) / 2;
	if (dpp_hmac_vector(curve->hash_len, Jx, Jx_len, 4, addr, len, u) < 0)
		goto fail;

	peer_u = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_I_AUTH_TAG,
			      &peer_u_len);
	if (!peer_u || peer_u_len != curve->hash_len ||
	    os_memcmp(peer_u, u, curve->hash_len) != 0) {
		dpp_pkex_fail(pkex, "No valid u (I-Auth tag) found");
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Calculated u'",
			    u, curve->hash_len);
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Received u", peer_u, peer_u_len);
		pkex->t++;
		goto fail;
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Valid u (I-Auth tag) received");

	/* ECDH: L = b * X' */
	EVP_PKEY_CTX_free(ctx);
	ctx = EVP_PKEY_CTX_new(pkex->own_bi->pubkey, NULL);
	if (!ctx ||
	    EVP_PKEY_derive_init(ctx) != 1 ||
	    EVP_PKEY_derive_set_peer(ctx, pkex->x) != 1 ||
	    EVP_PKEY_derive(ctx, NULL, &Lx_len) != 1 ||
	    Lx_len > DPP_MAX_SHARED_SECRET_LEN ||
	    EVP_PKEY_derive(ctx, Lx, &Lx_len) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to derive ECDH shared secret: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: ECDH shared secret (L.x)",
			Lx, Lx_len);

	/* v = HMAC(L.x, MAC-Responder | B.x | X'.x | Y.x) */
	B_pub = dpp_get_pubkey_point(pkex->own_bi->pubkey, 0);
	if (!B_pub)
		goto fail;
	addr[0] = pkex->own_mac;
	len[0] = ETH_ALEN;
	addr[1] = wpabuf_head(B_pub);
	len[1] = wpabuf_len(B_pub) / 2;
	addr[2] = wpabuf_head(X_pub);
	len[2] = wpabuf_len(X_pub) / 2;
	addr[3] = wpabuf_head(Y_pub);
	len[3] = wpabuf_len(Y_pub) / 2;
	if (dpp_hmac_vector(curve->hash_len, Lx, Lx_len, 4, addr, len, v) < 0)
		goto fail;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: v", v, curve->hash_len);

	msg = dpp_pkex_build_commit_reveal_resp(pkex, B_pub, v);
	if (!msg)
		goto fail;

out:
	EVP_PKEY_CTX_free(ctx);
	os_free(unwrapped);
	wpabuf_free(A_pub);
	wpabuf_free(B_pub);
	wpabuf_free(X_pub);
	wpabuf_free(Y_pub);
	return msg;
fail:
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX
		   "DPP: PKEX Commit-Reveal Request processing failed");
	goto out;
}


int dpp_pkex_rx_commit_reveal_resp(struct dpp_pkex *pkex, const u8 *hdr,
				   const u8 *buf, size_t buflen)
{
	const struct dpp_curve_params *curve = pkex->own_bi->curve;
	const u8 *wrapped_data, *b_key, *peer_v;
	u16 wrapped_data_len, b_key_len, peer_v_len = 0;
	const u8 *addr[4];
	size_t len[4];
	u8 octet;
	u8 *unwrapped = NULL;
	size_t unwrapped_len = 0;
	int ret = -1;
	u8 v[DPP_MAX_HASH_LEN];
	size_t Lx_len;
	u8 Lx[DPP_MAX_SHARED_SECRET_LEN];
	EVP_PKEY_CTX *ctx = NULL;
	struct wpabuf *B_pub = NULL, *X_pub = NULL, *Y_pub = NULL;

	if (!pkex->exchange_done || pkex->failed ||
	    pkex->t >= PKEX_COUNTER_T_LIMIT || !pkex->initiator)
		goto fail;

	wrapped_data = dpp_get_attr(buf, buflen, DPP_ATTR_WRAPPED_DATA,
				    &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		dpp_pkex_fail(pkex,
			      "Missing or invalid required Wrapped Data attribute");
		goto fail;
	}

	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped_data, wrapped_data_len);
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped = os_malloc(unwrapped_len);
	if (!unwrapped)
		goto fail;

	addr[0] = hdr;
	len[0] = DPP_HDR_LEN;
	octet = 1;
	addr[1] = &octet;
	len[1] = sizeof(octet);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);

	if (aes_siv_decrypt(pkex->z, curve->hash_len,
			    wrapped_data, wrapped_data_len,
			    2, addr, len, unwrapped) < 0) {
		dpp_pkex_fail(pkex,
			      "AES-SIV decryption failed - possible PKEX code mismatch");
		pkex->t++;
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext",
		    unwrapped, unwrapped_len);

	if (dpp_check_attrs(unwrapped, unwrapped_len) < 0) {
		dpp_pkex_fail(pkex, "Invalid attribute in unwrapped data");
		goto fail;
	}

	b_key = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_BOOTSTRAP_KEY,
			     &b_key_len);
	if (!b_key || b_key_len != 2 * curve->prime_len) {
		dpp_pkex_fail(pkex, "No valid peer bootstrapping key found");
		goto fail;
	}
	pkex->peer_bootstrap_key = dpp_set_pubkey_point(pkex->x, b_key,
							b_key_len);
	if (!pkex->peer_bootstrap_key) {
		dpp_pkex_fail(pkex, "Peer bootstrapping key is invalid");
		goto fail;
	}
	dpp_debug_print_key("DPP: Peer bootstrap public key",
			    pkex->peer_bootstrap_key);

	/* ECDH: L' = x * B' */
	ctx = EVP_PKEY_CTX_new(pkex->x, NULL);
	if (!ctx ||
	    EVP_PKEY_derive_init(ctx) != 1 ||
	    EVP_PKEY_derive_set_peer(ctx, pkex->peer_bootstrap_key) != 1 ||
	    EVP_PKEY_derive(ctx, NULL, &Lx_len) != 1 ||
	    Lx_len > DPP_MAX_SHARED_SECRET_LEN ||
	    EVP_PKEY_derive(ctx, Lx, &Lx_len) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to derive ECDH shared secret: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: ECDH shared secret (L.x)",
			Lx, Lx_len);

	/* v' = HMAC(L.x, MAC-Responder | B'.x | X.x | Y'.x) */
	B_pub = dpp_get_pubkey_point(pkex->peer_bootstrap_key, 0);
	X_pub = dpp_get_pubkey_point(pkex->x, 0);
	Y_pub = dpp_get_pubkey_point(pkex->y, 0);
	if (!B_pub || !X_pub || !Y_pub)
		goto fail;
	addr[0] = pkex->peer_mac;
	len[0] = ETH_ALEN;
	addr[1] = wpabuf_head(B_pub);
	len[1] = wpabuf_len(B_pub) / 2;
	addr[2] = wpabuf_head(X_pub);
	len[2] = wpabuf_len(X_pub) / 2;
	addr[3] = wpabuf_head(Y_pub);
	len[3] = wpabuf_len(Y_pub) / 2;
	if (dpp_hmac_vector(curve->hash_len, Lx, Lx_len, 4, addr, len, v) < 0)
		goto fail;

	peer_v = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_R_AUTH_TAG,
			      &peer_v_len);
	if (!peer_v || peer_v_len != curve->hash_len ||
	    os_memcmp(peer_v, v, curve->hash_len) != 0) {
		dpp_pkex_fail(pkex, "No valid v (R-Auth tag) found");
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Calculated v'",
			    v, curve->hash_len);
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Received v", peer_v, peer_v_len);
		pkex->t++;
		goto fail;
	}
	wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Valid v (R-Auth tag) received");

	ret = 0;
out:
	wpabuf_free(B_pub);
	wpabuf_free(X_pub);
	wpabuf_free(Y_pub);
	EVP_PKEY_CTX_free(ctx);
	os_free(unwrapped);
	return ret;
fail:
	goto out;
}


void dpp_pkex_free(struct dpp_pkex *pkex)
{
	if (!pkex)
		return;

	os_free(pkex->identifier);
	os_free(pkex->code);
	EVP_PKEY_free(pkex->x);
	EVP_PKEY_free(pkex->y);
	EVP_PKEY_free(pkex->peer_bootstrap_key);
	wpabuf_free(pkex->exchange_req);
	wpabuf_free(pkex->exchange_resp);
	os_free(pkex);
}


/* PFS should be done in driver */
#ifdef CONFIG_DPP2
#if 0
struct dpp_pfs * dpp_pfs_init(const u8 *net_access_key,
			      size_t net_access_key_len)
{
	struct wpabuf *pub = NULL;
	EVP_PKEY *own_key;
	struct dpp_pfs *pfs;

	pfs = os_zalloc(sizeof(*pfs));
	if (!pfs)
		return NULL;

	own_key = dpp_set_keypair(&pfs->curve, net_access_key,
				  net_access_key_len);
	if (!own_key) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to parse own netAccessKey");
		goto fail;
	}
	EVP_PKEY_free(own_key);

	pfs->ecdh = crypto_ecdh_init(pfs->curve->ike_group);
	if (!pfs->ecdh)
		goto fail;

	pub = crypto_ecdh_get_pubkey(pfs->ecdh, 0);
	pub = wpabuf_zeropad(pub, pfs->curve->prime_len);
	if (!pub)
		goto fail;

	pfs->ie = wpabuf_alloc(5 + wpabuf_len(pub));
	if (!pfs->ie)
		goto fail;
	wpabuf_put_u8(pfs->ie, WLAN_EID_EXTENSION);
	wpabuf_put_u8(pfs->ie, 1 + 2 + wpabuf_len(pub));
	wpabuf_put_u8(pfs->ie, WLAN_EID_EXT_OWE_DH_PARAM);
	wpabuf_put_le16(pfs->ie, pfs->curve->ike_group);
	wpabuf_put_buf(pfs->ie, pub);
	wpabuf_free(pub);
	wpa_hexdump_buf(MSG_DEBUG, "DPP: Diffie-Hellman Parameter element",
			pfs->ie);

	return pfs;
fail:
	wpabuf_free(pub);
	dpp_pfs_free(pfs);
	return NULL;
}


int dpp_pfs_process(struct dpp_pfs *pfs, const u8 *peer_ie, size_t peer_ie_len)
{
	if (peer_ie_len < 2)
		return -1;
	if (WPA_GET_LE16(peer_ie) != pfs->curve->ike_group) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Peer used different group for PFS");
		return -1;
	}

	pfs->secret = crypto_ecdh_set_peerkey(pfs->ecdh, 0, peer_ie + 2,
					      peer_ie_len - 2);
	pfs->secret = wpabuf_zeropad(pfs->secret, pfs->curve->prime_len);
	if (!pfs->secret) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: Invalid peer DH public key");
		return -1;
	}
	wpa_hexdump_buf_key(MSG_DEBUG, "DPP: DH shared secret", pfs->secret);
	return 0;
}


void dpp_pfs_free(struct dpp_pfs *pfs)
{
	if (!pfs)
		return;
	crypto_ecdh_deinit(pfs->ecdh);
	wpabuf_free(pfs->ie);
	wpabuf_clear_free(pfs->secret);
	os_free(pfs);
}
#endif
#endif /* CONFIG_DPP2 */

static unsigned int dpp_next_id(struct dpp_global *dpp)
{
	struct dpp_bootstrap_info *bi;
	unsigned int max_id = 0;

	dl_list_for_each(bi, &dpp->bootstrap, struct dpp_bootstrap_info, list) {
		if (bi->id > max_id)
			max_id = bi->id;
	}
	return max_id + 1;
}
int dpp_delete_parameter_from_file(struct wifi_app *wapp, const char *param)
{
  #ifdef OPENWRT_SUPPORT
	struct kvc_context *dat_ctx = NULL;
	char *ifparam;
	int ret = 0;

	os_alloc_mem(NULL, (UCHAR**)&ifparam, os_strlen(param)+1);
	if(ifparam == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s, mem alloc fail \n", __func__);
		goto out;
	}
	NdisZeroMemory(ifparam, os_strlen(param));
	strncat(ifparam, param,os_strlen(param));
	dat_ctx = dat_load(DPP_CFG_FILE);
	if (!dat_ctx) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"load file(%s) fail\n", DPP_CFG_FILE);
		ret = -1;
		goto out;
	}
	ret = kvc_unset(dat_ctx, (const char *)ifparam);
	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"un-set param(%s) fail\n", param);
		goto out;
	}
	ret = kvc_commit(dat_ctx);
	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"write param(%s) fail\n", param);
		goto out;
	}
	os_free_mem(NULL, ifparam);
	ifparam=NULL;
out:
	if (dat_ctx)
		kvc_unload(dat_ctx);
	if(ifparam)
		os_free_mem(NULL, ifparam);
	if (ret)
		   return -1;
#endif /* OPENWRT_SUPPORT */
		return 0;
}

#ifdef MAP_R3
int  dpp_reset_dpp_config_file(struct wifi_app *wapp)
{

   	char param[50]="0";
	char qr_param[20] = "0";
	char value[150] = "0";
	char param_str[15] = "agt_qr_code";
	char uri_num = 1;
	int uri_cnt = 0;
	int res = 0;
	
	char required_parameters[20][30]={"_akm","dpp_private_key","_1905valid","_1905connector",
        "_1905netAccessKey","_1905cSignKey","_netKeyExpiry","_decryptThreshold","_ppkey","_bsscred",
	"_bsscredlen","_bhcredlen","_fhcredlen","_valid","_ssid","_passPhrase","_connector",
	"_netAccessKey","_cSignKey","qr_count"};

	get_dpp_parameters(wapp->map, "qr_count", value, sizeof(value));
	uri_cnt = atoi(value);
	if(uri_cnt != 0) {
		for(uri_num = 1; uri_num <= uri_cnt; uri_num++) {
			os_memset(qr_param, 0, 20);
			res = os_snprintf(qr_param, sizeof(qr_param), "%s_%u", param_str, uri_num);
			if (os_snprintf_error(sizeof(qr_param), res)) {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s %d os_snprintf error\n", __func__, __LINE__);
				continue;
			}

			dpp_delete_parameter_from_file(wapp, (const char *)qr_param);
		}
	}

	for( int i=0;i<20;i++)
	{
		os_memset(param, 0, 50);
		os_strlcpy(param, required_parameters[i], os_strlen(required_parameters[i])+1);

#ifdef OPENWRT_SUPPORT
		struct kvc_context *dat_ctx = NULL;
		char *ifparam;
		int ret = 0;

		os_alloc_mem(NULL, (UCHAR**)&ifparam, os_strlen(param)+1);
		if(ifparam == NULL) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s, mem alloc fail \n", __func__);
			goto out;
		}
		NdisZeroMemory(ifparam, os_strlen(param));

		strncat(ifparam, param,os_strlen(param));
		dat_ctx = dat_load(DPP_CFG_FILE);
		if (!dat_ctx) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"load file(%s) fail\n", DPP_CFG_FILE);
			ret = -1;
			goto out;
		}

		ret = kvc_unset(dat_ctx, (char *)ifparam);
		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"un-set param(%s) fail\n", param);
			goto out;
		}

		ret = kvc_commit(dat_ctx);
		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"write param(%s) fail\n", param);
			goto out;
		}
		os_free_mem(NULL, ifparam);
		ifparam=NULL;
out:
		if (dat_ctx)
			kvc_unload(dat_ctx);
		if(ifparam)
			os_free_mem(NULL,ifparam);
		if (ret)
			    return -1;
#endif /* OPENWRT_SUPPORT */
	}
	return dpp_reset_mapd_user_config_file(wapp);
}


int  dpp_reset_mapd_user_config_file(struct wifi_app *wapp)
{

	char param[50] = "0";
	int i;
	char required_parameters[18][30] = {"BhProfile0Ssid", "BhProfile0AuthMode", "BhProfile0EncrypType", "BhProfile0WpaPsk",
		"BhProfile0Valid", "BhProfile0RaID", "BhProfile1Ssid", "BhProfile1AuthMode", "BhProfile1EncrypType", "BhProfile1WpaPsk",
		"BhProfile1Valid", "BhProfile1RaID", "BhProfile2Ssid", "BhProfile2AuthMode", "BhProfile2EncrypType", "BhProfile2WpaPsk",
		"BhProfile2Valid", "BhProfile2RaID"};


	for (i = 0; i < 18; i++)	{
		os_memset(param, 0, 50);
		os_strlcpy(param, required_parameters[i], os_strlen(required_parameters[i])+1);

#ifdef OPENWRT_SUPPORT
		struct kvc_context *dat_ctx = NULL;
		char *ifparam;
		int ret = 0;

		os_alloc_mem(NULL, (UCHAR **)&ifparam, os_strlen(param)+1);
		if (ifparam == NULL) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s, mem alloc fail\n", __func__);
			goto out;
		}
		NdisZeroMemory(ifparam, os_strlen(param));

		strncat(ifparam, param, os_strlen(param));
		dat_ctx = dat_load(MAPD_USER_CFG_FILE);
		if (!dat_ctx) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"load file(%s) fail\n", MAPD_USER_CFG_FILE);
			ret = -1;
			goto out;
		}

		ret = kvc_unset(dat_ctx, (char *)ifparam);
		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"un-set param(%s) fail\n", param);
			goto out;
		}

		ret = kvc_commit(dat_ctx);
		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"write param(%s) fail\n", param);
			goto out;
		}
		os_free_mem(NULL, ifparam);
		ifparam = NULL;
out:
		if (dat_ctx)
			kvc_unload(dat_ctx);
		if (ifparam)
			os_free_mem(NULL, ifparam);
		if (ret)
			return -1;
#endif /* OPENWRT_SUPPORT */
			}
return 0;
}



#endif /*MAP R3 */
void  dpp_remove_uri_from_file(struct wifi_app *wapp, unsigned int id , const char *uri_temp)
{
    
    char qr_param[20] = "0";
	char value[150] = "0";
	char param_str[15] = "agt_qr_code";
	char uri_num = 1;
	int uri_cnt = 0, ret = 0;

	char *strbuf = NULL;
   
	os_alloc_mem(NULL, (UCHAR**)&strbuf, 10);
	if (strbuf == NULL) {
		DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"Error: Memory allocation failed \n");
		return;
	}
	
	NdisZeroMemory(strbuf, 10);

	get_dpp_parameters(wapp->map, "qr_count", value, sizeof(value));
	uri_cnt = atoi(value);
	
	int found =0;
	//If passed URI found , overwritng it with succesive URI 
	//Also updating the successive URIs in the same manner
	if(uri_cnt != 0) {
		for(uri_num = 1; uri_num <= uri_cnt; uri_num++)
			{
			  
			os_memset(qr_param, 0, 20);
			os_memset(value, 0, 150);
			ret = os_snprintf(qr_param, sizeof(qr_param), "%s_%u", param_str, uri_num);
			if (os_snprintf_error(sizeof(qr_param), ret))
				DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"%s %d os_snprintf error1\n", __func__, __LINE__);

			 get_dpp_parameters(wapp->map, qr_param, value, sizeof(value));
			
			 if(memcmp(uri_temp,value,strlen(value))==0)
			 	{
			 	  DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"strings matched : going to update qr codes \n");
			 	found=1;
			for (uri_num = uri_num+1; uri_num <= uri_cnt; uri_num++) {
			os_memset(qr_param, 0, 20);
			os_memset(value, 0, 150);
			ret = os_snprintf(qr_param, sizeof(qr_param), "%s_%u", param_str, uri_num);
			if (os_snprintf_error(sizeof(qr_param), ret))
				DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"%s %d os_snprintf error2\n", __func__, __LINE__);

			get_dpp_parameters(wapp->map, qr_param, value, sizeof(value));
			if(!is_str_null(value))
			
			{
				os_memset(qr_param, 0, 20);
				ret = os_snprintf(qr_param, sizeof(qr_param), "%s_%u", param_str, (uri_num-1));
				if (os_snprintf_error(sizeof(qr_param), ret))
					DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"%s %d os_snprintf error3\n", __func__, __LINE__);

				dpp_save_config(wapp, (const char *)qr_param, (const char *)value, NULL);
				os_memset(value, 0, 150);
				get_dpp_parameters(wapp->map, qr_param, value, sizeof(value));
			}
			
		  	}
		 	}
			 if(found==1)
			 	break;
			
		}
		}
	 
     //updating/deleting last qr parameter 
     if(found ==1){
		os_memset(qr_param, 0, 20);
		ret = os_snprintf(qr_param, sizeof(qr_param), "%s_%u", param_str, uri_cnt);
		if (os_snprintf_error(sizeof(qr_param), ret))
			DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"%s %d os_snprintf error4\n", __func__, __LINE__);

		dpp_delete_parameter_from_file(wapp, (const char *)qr_param);

         //updating the QR count
		 wapp->dpp->qr_code_num--;
		ret = os_snprintf(strbuf, 10, "%u", wapp->dpp->qr_code_num);
		if (os_snprintf_error(10, ret))
			DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"%s %d os_snprintf error5\n", __func__, __LINE__);

		dpp_save_config(wapp, "qr_count", strbuf, NULL);
		NdisZeroMemory(strbuf, 10);

		
      	}
	    if (strbuf)
		   os_free_mem(NULL, strbuf);
		
		return;
}


static int dpp_bootstrap_del(struct dpp_global *dpp, unsigned int id)
{
	struct dpp_bootstrap_info *bi, *tmp;
	int found = 0;

	if (!dpp)
		return -1;

	dl_list_for_each_safe(bi, tmp, &dpp->bootstrap,
			      struct dpp_bootstrap_info, list) {
		if (id && bi->id != id)
			continue;
		found = 1;
		dl_list_del(&bi->list);
		dpp_bootstrap_info_free(bi);
	}
  
	if (id == 0)
		return 0; /* flush succeeds regardless of entries found */
	return found ? 0 : -1;
}

/* This function validates the URI received from the user with the
 * the ending string ';;' whose ascii is 0x3b and copies valid string
 * into uri buffer for further comparing valid uri with the uri
 * stored in list.
*/
static int validate_input_uri(const char *uri, char *buf)
{
	int flag = 0, i = 0;

	if (strlen(uri) > DPP_MAX_URI_LEN) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" Invalid URI len!\n");
		return -1;
	}

	while (uri[i] != '\0') {
		if (uri[i] == 0x3b && uri[i] == uri[i + 1]) {
			buf[i] = 0x3b;   /* ; ending char */
			buf[i + 1] = 0x3b;  /* ; ending char */
			i = i + 2;
			flag = 1;
			break;
		}

		buf[i] = uri[i];
		i++;
	}

	if (!flag) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" [%s]:Invalid URI %s!\n", __func__, uri);
		return -1;
	}

	buf[i] = '\0';
	return 0;
}

struct dpp_bootstrap_info * dpp_add_qr_code(struct dpp_global *dpp,
					    const char *uri)
{
 	struct dpp_bootstrap_info *bi;
#ifdef MAP_R3
	struct dpp_agent_info *agnt_info = NULL;
	struct dpp_bootstrap_info*temp = NULL;
	struct dpp_bootstrap_info* tra_bi = NULL;
	char uri_buf[DPP_MAX_URI_LEN] = {0};
#endif /* MAP_R3 */
	if (!dpp)
		return NULL;
	/* DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX" Received QR code ! %s\n", uri); */

	if (validate_input_uri(uri, uri_buf) != 0) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" Invalid Input URI %s!\n", uri);
		return NULL;
	}

	//check for duplicate URI 
		 dl_list_for_each_safe(tra_bi, temp, &dpp->bootstrap, struct dpp_bootstrap_info, list) {
			if (!tra_bi) {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"No URI in list\n");
				 break;
			}

		   /* printf( "Traversing :URI's already present  : %s\n", tra_bi->uri); */
			if (os_strncmp(tra_bi->uri, uri_buf, strlen(uri_buf)) == 0) {
				DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" Error : URI already exists in file\n");
				return NULL;
			}
		}

	bi = dpp_parse_qr_code(uri
#ifdef MAP_R3
			, dpp
#endif
		);
    
	if (!bi)
		return NULL;

	bi->id = dpp_next_id(dpp);
	/* Kapil, first one will be our own */
	if (bi->id == 1)
		bi->own = 1;

#ifdef MAP_R3
	agnt_info = os_zalloc(sizeof(*agnt_info));
	os_memcpy(agnt_info->resp_pubkey_hash, bi->pubkey_hash ,SHA256_MAC_LEN);
	os_memcpy(agnt_info->chirp_hash, bi->chirp_hash ,SHA256_MAC_LEN);
	agnt_info->agent_state = DPP_AGT_STATE_INIT;
	dl_list_add(&dpp->dpp_agent_list, &agnt_info->list);
#endif /* MAP_R3 */

	dl_list_add(&dpp->bootstrap, &bi->list);
	return bi;
}
		

int dpp_bootstrap_gen_at_bootup(struct dpp_global *dpp, char *key, char *mac, char *chan)
{
	char *info = NULL, *pk = NULL, *curve = NULL;
	u8 *privkey = NULL;
	size_t privkey_len = 0;
	size_t len;
	int ret = -1;
	struct dpp_bootstrap_info *bi;

	if (!dpp)
		return -1;

	bi = os_zalloc(sizeof(*bi));
	if (!bi)
		goto fail;

	bi->type = DPP_BOOTSTRAP_QR_CODE;

	if (key) {
		privkey_len = os_strlen(key) / 2;
		privkey = os_malloc(privkey_len);
		if (!privkey ||
		    hexstr2bin(key, privkey, privkey_len) < 0)
			goto fail;
	}

	pk = dpp_keygen(bi, curve, privkey, privkey_len);
	if (!pk)
		goto fail;

	len = 4; /* "DPP:" */

#ifdef MAP_R3
	if(is_str_null(mac))
		mac = NULL;
	if(is_str_null(chan))
		chan = NULL;
#endif /* MAP_R3 */

	if (chan) {
			if (dpp_parse_uri_chan_list(bi, chan
#ifdef MAP_R3
				, dpp
#endif
			) < 0)
			goto fail;
		len += 3 + os_strlen(chan); /* C:...; */
	}

	if (mac) {
		if (dpp_parse_uri_mac(bi, mac) < 0)
			goto fail;
		len += 3 + os_strlen(mac); /* M:...; */
	}
#if 0
	if (info) {
		if (dpp_parse_uri_info(bi, info) < 0)
			goto fail;
		len += 3 + os_strlen(info); /* I:...; */
	}
#endif
#ifdef CONFIG_DPP2
	len += 4; /* V:2; */
#endif
	len += 4 + os_strlen(pk);
	bi->uri = os_malloc(len + 1);
	if (!bi->uri)
		goto fail;
	ret = os_snprintf(bi->uri, len + 1, "DPP:%s%s%s%s%s%s%s%s%s%sK:%s;;",
		    chan ? "C:" : "", chan ? chan : "", chan ? ";" : "",
		    mac ? "M:" : "", mac ? mac : "", mac ? ";" : "",
		    info ? "I:" : "", info ? info : "", info ? ";" : "",
		    DPP_VERSION == 2 ? "V:2;" : "",
		    pk);
	if (os_snprintf_error(len + 1, ret))
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"%s %d os_snprintf error\n", __func__, __LINE__);

	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"URI : %s\n", bi->uri);
	bi->id = dpp_next_id(dpp);
	dl_list_add(&dpp->bootstrap, &bi->list);
	ret = bi->id;
	bi = NULL;
fail:
	os_free(curve);
	os_free(pk);
	os_free(info);
	bin_clear_free(privkey, privkey_len);
	dpp_bootstrap_info_free(bi);
	return ret;
}

int dpp_bootstrap_gen(struct dpp_global *dpp, const char *cmd)
{
	char *chan = NULL, *mac = NULL, *info = NULL, *pk = NULL, *curve = NULL;
	char *key = NULL;
	u8 *privkey = NULL;
	size_t privkey_len = 0;
	size_t len;
	int ret = -1;
	struct dpp_bootstrap_info *bi;
#ifdef DPP_AUTOTEST
	char buf[2048] = {0};
#endif
#if defined(DPP_AUTOTEST) || defined(MAP_R3)
	unsigned char *priv_temp = NULL;
#endif

	if (!dpp) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"DPP not init yet\n");
		return -1;
	}

#ifdef MAP_R3
	struct wifi_app *wapp = dpp->msg_ctx;
	struct dpp_bootstrap_info *own_bi = NULL;
	own_bi = dpp_get_own_bi(dpp);
	if(own_bi && dpp->chirp_ongoing) {
		DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"chirp already ongoing with existing QR code, stop chirp first\n");
		return 0;
	}
	if(own_bi) {
		/* Deleting the own URI if presenti in enrolle mode */
		if(wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE)
			dpp_bootstrap_del(dpp, own_bi->id);
		else if(wapp->dpp->dpp_allowed_roles == DPP_CAPAB_CONFIGURATOR) {
			DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX"URI already generated on controller\n");
			return 0;
		}
	}
#endif /* MAP_R3 */

	bi = os_zalloc(sizeof(*bi));
	if (!bi)
		goto fail;

	if (os_strstr(cmd, "type=qrcode"))
		bi->type = DPP_BOOTSTRAP_QR_CODE;
	else if (os_strstr(cmd, "type=pkex"))
		bi->type = DPP_BOOTSTRAP_PKEX;
	else
		goto fail;

	chan = get_param(cmd, " chan=");
	mac = get_param(cmd, " mac=");
	info = get_param(cmd, " info=");
	curve = get_param(cmd, " curve=");
	key = get_param(cmd, " key=");

	if (key) {
		privkey_len = os_strlen(key) / 2;
		privkey = os_malloc(privkey_len);
		if (!privkey ||
		    hexstr2bin(key, privkey, privkey_len) < 0)
			goto fail;
	}

	pk = dpp_keygen(bi, curve, privkey, privkey_len);
	if (!pk)
		goto fail;

	len = 4; /* "DPP:" */
	if (chan) {
		if (dpp_parse_uri_chan_list(bi, chan
#ifdef MAP_R3
			, dpp
#endif
			) < 0)
			goto fail;
		len += 3 + os_strlen(chan); /* C:...; */
	}
	if (mac) {
		if (dpp_parse_uri_mac(bi, mac) < 0)
			goto fail;
		len += 3 + os_strlen(mac); /* M:...; */
	}
	if (info) {
		if (dpp_parse_uri_info(bi, info) < 0)
			goto fail;
		len += 3 + os_strlen(info); /* I:...; */
	}
#ifdef CONFIG_DPP2
	len += 4; /* V:2; */
#endif
	len += 4 + os_strlen(pk);
	bi->uri = os_malloc(len + 1);
	if (!bi->uri)
		goto fail;
	ret = os_snprintf(bi->uri, len + 1, "DPP:%s%s%s%s%s%s%s%s%s%sK:%s;;",
		    chan ? "C:" : "", chan ? chan : "", chan ? ";" : "",
		    mac ? "M:" : "", mac ? mac : "", mac ? ";" : "",
		    info ? "I:" : "", info ? info : "", info ? ";" : "",
		    DPP_VERSION == 2 ? "V:2;" : "",
		    pk);
	if (os_snprintf_error(len + 1, ret))
		DBGPRINT(RT_DEBUG_ERROR, "%s %d os_snprintf error\n", __func__, __LINE__);

#ifdef MAP_R3
	dpp_gen_privkey_from_pubkey(bi->pubkey, &priv_temp);

	if(priv_temp != NULL) {
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"dpp_private_key=%s\n",priv_temp);
		dpp_save_config(wapp, "dpp_private_key", (const char *)priv_temp, NULL);
		//wpa_write_config_file(DPP_CFG_FILE, buf, strlen(buf));
		/* Saving in the buffer for future use */
		os_strlcpy(dpp->dpp_private_key, (const char *)priv_temp, sizeof(dpp->dpp_private_key));
		os_free(priv_temp);
		priv_temp = NULL;
	}

	if(chan)
		dpp_save_config(wapp, "dpp_chan_list", (const char *)chan, NULL);

	if(mac)
		dpp_save_config(wapp, "dpp_macaddr_key", (const char *)mac, NULL);
	/* For WPS PBC using the newly created URI from generation
	 * and also update to 1905 for new URI */
	if (bi && (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_ENROLLEE)
			&& (wapp->map && wapp->map->map_version == DEV_TYPE_R3 )) {
		wapp_dpp_agnt_uri(wapp, bi->uri);
		ChirpTLV_1905_send(wapp, bi);
	}
#endif /* MAP_R3 */

#ifdef DPP_AUTOTEST
	dpp_gen_privkey_from_pubkey(bi->pubkey, &priv_temp);

	if(priv_temp != NULL) {
		snprintf(buf,(sizeof(buf)), "dpp_private_key=%s\nqrcode=%s\n",priv_temp, bi->uri);
		wpa_write_config_file(DPP_CFG_FILE, buf, strlen(buf));
		os_free(priv_temp);
		priv_temp = NULL;
	}
#else
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"URI : %s\n", bi->uri);
#endif

	bi->id = dpp_next_id(dpp);
	dl_list_add(&dpp->bootstrap, &bi->list);
	ret = bi->id;
	bi = NULL;
fail:
	os_free(curve);
	os_free(pk);
	os_free(chan);
	os_free(mac);
	os_free(info);
	str_clear_free(key);
	bin_clear_free(privkey, privkey_len);
	dpp_bootstrap_info_free(bi);
	return ret;
}


struct dpp_bootstrap_info *
dpp_bootstrap_get_id(struct dpp_global *dpp, unsigned int id)
{
	struct dpp_bootstrap_info *bi;

	if (!dpp)
		return NULL;

	dl_list_for_each(bi, &dpp->bootstrap, struct dpp_bootstrap_info, list) {
		if (bi->id == id)
			return bi;
	}
	return NULL;
}


int dpp_bootstrap_remove(struct wifi_app *wapp,struct dpp_global *dpp, const char *id)
{
	unsigned int id_val;

	if (os_strcmp(id, "*") == 0) {
		id_val = 0;
	} else {
		id_val = atoi(id);
		if (id_val == 0)
			return -1;
	}
	
    
	char uri_temp2[150]={0};
	const char*uri = dpp_bootstrap_get_uri(wapp->dpp,id_val);
	if (id_val != 0) {
		if (uri)
			os_strlcpy(uri_temp2, uri, sizeof(uri_temp2));
	}
	int result = dpp_bootstrap_del(dpp, id_val);
		if(result ==-1)
			{
			printf("URI not present at this index \n");
			return result;
			}
	if (id_val != 0)
		dpp_remove_uri_from_file(wapp, id_val, (const char *)uri_temp2);

	return result;
   
}


struct dpp_bootstrap_info *
dpp_pkex_finish(struct dpp_global *dpp, struct dpp_pkex *pkex, const u8 *peer,
		unsigned int chan)
{
	struct dpp_bootstrap_info *bi;

	bi = os_zalloc(sizeof(*bi));
	if (!bi)
		return NULL;
	bi->id = dpp_next_id(dpp);
	bi->type = DPP_BOOTSTRAP_PKEX;
	os_memcpy(bi->mac_addr, peer, ETH_ALEN);
	bi->num_chan = 1;
	bi->chan[0] = chan;
	bi->curve = pkex->own_bi->curve;
	bi->pubkey = pkex->peer_bootstrap_key;
	pkex->peer_bootstrap_key = NULL;
	if (dpp_bootstrap_key_hash(bi) < 0) {
		dpp_bootstrap_info_free(bi);
		return NULL;
	}
	dpp_pkex_free(pkex);
	dl_list_add(&dpp->bootstrap, &bi->list);
	return bi;
}

const char * dpp_bootstrap_get_uri(struct dpp_global *dpp, unsigned int id)
{
	struct dpp_bootstrap_info *bi;
	char buf[2048] = {0};
	int ret;

	bi = dpp_bootstrap_get_id(dpp, id);
	if (!bi)
		return NULL;

	ret = os_snprintf(buf, (sizeof(buf)), "qrcode=%s\n", bi->uri);
	if (os_snprintf_error(sizeof(buf), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "%s %d snprintf error\n", __func__, __LINE__);
		return NULL;
	}

	FILE *fp = fopen(DPP_CFG_FILE_TEST, "w+");

	if (fp != NULL && fclose(fp) != 0) {
		DBGPRINT(RT_DEBUG_ERROR, "%s %d fclose or invalid fp error\n", __func__, __LINE__);
		return NULL;
	}

	wpa_write_config_file(DPP_CFG_FILE_TEST, buf, strlen(buf));

	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"URI : %s\n", bi->uri);
	return bi->uri;
}


int dpp_bootstrap_info(struct dpp_global *dpp, int id,
		       char *reply, int reply_size)
{
	struct dpp_bootstrap_info *bi;
	char pkhash[2 * SHA256_MAC_LEN + 1];

	bi = dpp_bootstrap_get_id(dpp, id);
	if (!bi)
		return -1;
	os_snprintf_hex(pkhash, sizeof(pkhash), bi->pubkey_hash,
			 SHA256_MAC_LEN);
	return os_snprintf(reply, reply_size, "type=%s\n"
			   "mac_addr=" MACSTR "\n"
			   "info=%s\n"
			   "num_chan=%u\n"
			   "curve=%s\n"
			   "pkhash=%s\n"
			   "version=%d\n",
			   dpp_bootstrap_type_txt(bi->type),
			   MAC2STR(bi->mac_addr),
			   bi->info ? bi->info : "",
			   bi->num_chan,
			   bi->curve->name,
			   pkhash,
			   bi->version);
}


void dpp_bootstrap_find_pair(struct dpp_global *dpp, const u8 *i_bootstrap,
			     const u8 *r_bootstrap,
			     struct dpp_bootstrap_info **own_bi,
			     struct dpp_bootstrap_info **peer_bi)
{
	struct dpp_bootstrap_info *bi;

	*own_bi = NULL;
	*peer_bi = NULL;
	if (!dpp)
		return;

	dl_list_for_each(bi, &dpp->bootstrap, struct dpp_bootstrap_info, list) {
		wpa_hexdump(MSG_MSGDUMP, DPP_MAP_PREX "DPP: Kapil pubkey hash ",
		    bi->pubkey_hash, 32);
		if (!*own_bi && bi->own &&
		    os_memcmp(bi->pubkey_hash, r_bootstrap,
			      SHA256_MAC_LEN) == 0) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Found matching own bootstrapping information");
			*own_bi = bi;
		}

		if (!*peer_bi && !bi->own &&
		    os_memcmp(bi->pubkey_hash, i_bootstrap,
			      SHA256_MAC_LEN) == 0) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Found matching peer bootstrapping information");
			*peer_bi = bi;
		}

		if (*own_bi && *peer_bi)
			break;
	}

}


static unsigned int dpp_next_configurator_id(struct dpp_global *dpp)
{
	struct dpp_configurator *conf;
	unsigned int max_id = 0;

	dl_list_for_each(conf, &dpp->configurator, struct dpp_configurator,
			 list) {
		if (conf->id > max_id)
			max_id = conf->id;
	}
	return max_id + 1;
}

int wapp_dpp_configurator_add(struct dpp_global *dpp)
{
	u8 *privkey = NULL;
	size_t privkey_len = 0;
	int ret = -1;
	struct dpp_configurator *conf = NULL;

	privkey_len = os_strlen(dpp->dpp_private_key) / 2;
	privkey = os_malloc(privkey_len);
	if (!privkey ||
			hexstr2bin(dpp->dpp_private_key, privkey, privkey_len) < 0)
		goto fail;

	conf = dpp_keygen_configurator(dpp->curve_name, privkey, privkey_len
#ifdef MAP_R3
		, NULL, 0
#endif /* MAP_R3 */
	);
	if (!conf)
		goto fail;

	conf->id = dpp_next_configurator_id(dpp);
	dl_list_add(&dpp->configurator, &conf->list);
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX"added one configurator with id=%d\n", conf->id);
	ret = conf->id;
	conf = NULL;
fail:
	dpp_configurator_free(conf);
	bin_clear_free(privkey, privkey_len);
	return ret;
}

int dpp_configurator_add(struct dpp_global *dpp, const char *cmd)
{
	char *curve = NULL;
	char *key = NULL;
	u8 *privkey = NULL;
	size_t privkey_len = 0;
	int ret = -1;
	struct dpp_configurator *conf = NULL;
#ifdef MAP_R3
	char *ppkey = NULL;
        u8 *pp_key = NULL;
        size_t pp_key_len = 0;
#endif /* MAP_R3 */

	curve = get_param(cmd, " curve=");
	key = get_param(cmd, " key=");
#ifdef MAP_R3
	ppkey = get_param(cmd, " ppkey=");
#endif /* MAP_R3 */

	if (key) {
		privkey_len = os_strlen(key) / 2;
		privkey = os_malloc(privkey_len);
		if (!privkey ||
		    hexstr2bin(key, privkey, privkey_len) < 0)
			goto fail;
	}

#ifdef MAP_R3
	if (ppkey) {
		pp_key_len = os_strlen(ppkey) / 2;
		pp_key = os_malloc(pp_key_len);
		if (!pp_key ||
			hexstr2bin(ppkey, pp_key, pp_key_len) < 0)
                        goto fail;
	}
#endif /* MAP_R3 */

	conf = dpp_keygen_configurator(curve, privkey, privkey_len
#ifdef MAP_R3
		, pp_key, pp_key_len
#endif /* MAP_R3 */
	);
	if (!conf)
		goto fail;

	conf->id = dpp_next_configurator_id(dpp);
	dl_list_add(&dpp->configurator, &conf->list);
	ret = conf->id;
	conf = NULL;
fail:
	os_free(curve);
	str_clear_free(key);
	bin_clear_free(privkey, privkey_len);
	str_clear_free(ppkey);
        bin_clear_free(pp_key, pp_key_len);
	dpp_configurator_free(conf);
	return ret;
}


static int dpp_configurator_del(struct dpp_global *dpp, unsigned int id)
{
	struct dpp_configurator *conf, *tmp;
	int found = 0;

	if (!dpp)
		return -1;

	dl_list_for_each_safe(conf, tmp, &dpp->configurator,
			      struct dpp_configurator, list) {
		if (id && conf->id != id)
			continue;
		found = 1;
		dl_list_del(&conf->list);
		dpp_configurator_free(conf);
	}

	if (id == 0)
		return 0; /* flush succeeds regardless of entries found */
	return found ? 0 : -1;
}


int dpp_configurator_remove(struct dpp_global *dpp, const char *id)
{
	unsigned int id_val;

	if (os_strcmp(id, "*") == 0) {
		id_val = 0;
	} else {
		id_val = atoi(id);
		if (id_val == 0)
			return -1;
	}

	return dpp_configurator_del(dpp, id_val);
}


int dpp_configurator_get_key_id(struct dpp_global *dpp, unsigned int id,
				char *buf, size_t buflen)
{
	struct dpp_configurator *conf;

	conf = dpp_configurator_get_id(dpp, id);
	if (!conf)
		return -1;

	return dpp_configurator_get_key(conf, buf, buflen);
}


#ifdef CONFIG_DPP2

static void dpp_tcp_init_flush(struct dpp_global *dpp)
{
	struct dpp_authentication *auth, *tmp;

	dl_list_for_each_safe(auth, tmp, &dpp->tcp_init, struct dpp_authentication,
			      list)
		dpp_auth_deinit(auth);
}


static void dpp_relay_controller_free(struct dpp_relay_controller *ctrl)
{
	struct dpp_authentication *auth, *tmp;

	dl_list_for_each_safe(auth, tmp, &ctrl->auth, struct dpp_authentication,
			      list)
		dpp_auth_deinit(auth);
	os_free(ctrl);
}

static void dpp_relay_flush_controllers(struct dpp_global *dpp)
{
	struct dpp_relay_controller *ctrl, *tmp;

	if (!dpp)
		return;

	dl_list_for_each_safe(ctrl, tmp, &dpp->controllers,
			      struct dpp_relay_controller, list) {
		dl_list_del(&ctrl->list);
		dpp_relay_controller_free(ctrl);
	}
}

#endif /* CONFIG_DPP2 */


int wapp_dpp_init(struct wifi_app *wapp)
{
	struct dpp_global *dpp;

	dpp = os_zalloc(sizeof(*dpp));
	if (!dpp)
		return -1;
	dpp->msg_ctx = wapp;
#ifdef CONFIG_DPP2
	dpp->cb_ctx = wapp;
#endif /* CONFIG_DPP2 */

	dl_list_init(&dpp->bootstrap);
	dl_list_init(&dpp->configurator);
#ifdef CONFIG_DPP2
	dl_list_init(&dpp->controllers);
	dl_list_init(&dpp->tcp_init);
#endif /* CONFIG_DPP2 */
	dl_list_init(&dpp->dpp_auth_list);
	dpp->version_ctrl = DPP_VERSION;
	wapp->dpp = dpp;
	if (!wapp->dpp)
		return -1;

	wapp->dpp->gas_query_ctx = gas_query_init(wapp);
#ifdef MAP_R3
	wapp->dpp->dpp_allowed_roles = 0;
#else
	wapp->dpp->dpp_allowed_roles = DPP_CAPAB_CONFIGURATOR | DPP_CAPAB_ENROLLEE;
#endif /* MAP_R3 */
	wapp->dpp->max_remain_on_chan = 2000;
	dl_list_init(&wapp->dpp->dpp_txstatus_pending_list);
	wapp->dpp->dpp_frame_seq_no = 1011; //TF
#ifndef MAP_R3
	wapp->dpp->default_5gh_iface = wapp_dev_list_lookup_by_ifname(wapp, DEFAULT_5GH_IFACE);
	wapp->dpp->default_5gl_iface = wapp_dev_list_lookup_by_ifname(wapp, DEFAULT_5GL_IFACE);
	wapp->dpp->default_2g_iface = wapp_dev_list_lookup_by_ifname(wapp, DEFAULT_2G_IFACE);
#endif /* MAP_R3 */
	os_strlcpy(dpp->curve_name, "prime256v1", sizeof(dpp->curve_name));
#ifdef DPP_R2_SUPPORT
	os_memset(&(dpp->scan_ch), 0, sizeof(struct dpp_scan_channel));
	os_memset(&(dpp->annouce_enrolle), 0, sizeof(struct dpp_pre_annouce_info));
	dpp->annouce_enrolle.is_enable = 0;
#endif /* DPP_R2_SUPPORT */
#ifdef DPP_R2_MUOBJ
	os_memset(&(dpp->config_ap), 0, sizeof(struct dpp_configuration) * DPP_CONF_OBJ_MAX);
	os_memset(&(dpp->config_sta), 0, sizeof(struct dpp_configuration) * DPP_CONF_OBJ_MAX);
#endif /* DPP_R2_MUOBJ */
#ifdef DPP_R2_RECONFIG
	os_memset(&(dpp->reconfig_annouce), 0, sizeof(struct dpp_annouce_info));
	dpp->reconfig_annouce.is_enable = 1;
#endif /* DPP_R2_RECONFIG */
#ifdef MAP_R3
	os_memset(dpp->dpp_macaddr_key, 0, sizeof(dpp->dpp_macaddr_key));
	wapp->conf_op_bnd = 0;
	wapp->dpp->dpp_chirp_handling = 1;
	wapp->dpp->config_done = 0;
	wapp->dpp->dpp_max_connection_tries = 4;
	os_memset(wapp->dpp->almac_cont, 0, MAC_ADDR_LEN);
	os_memset(wapp->dpp->relay_almac_addr, 0, MAC_ADDR_LEN);
	wapp->dpp->cce_scan_ongoing = 0;
	wapp->dpp->cce_driver_scan_ongoing = 0;
	wapp->dpp->cce_driver_scan_done = 0;
	wapp->dpp->chirp_ongoing = 0;
	wapp->dpp->cce_rsp_rcvd_cnt = 0;
	wapp->dpp->map_sec_done = FALSE;
	wapp->dpp->dpp_map_cont_self = FALSE;
	wapp->dpp->dpp_eth_conn_ind = FALSE;
	wapp->dpp->dpp_onboard_ongoing = FALSE;
	wapp->dpp->wsc_profile_cnt = 0;
	wapp->dpp->wsc_onboard_done = 0;
	wapp->dpp->qr_code_num = 0;
	wapp->cce_scan_count = 0;
	wapp->dpp->qr_cmd = 1;
	wapp->dpp->conf_res_received = 0;
	wapp->dpp->prev_chan_detected = 0;
	wapp->dpp->chirp_stop_done = 0;
	os_memcpy(wapp->dpp->band_priority,"5G",2);
	dl_list_init(&dpp->dpp_agent_list);
#endif /* MAP_R3 */
#ifdef MAP_R3_RECONFIG
	wapp->dpp->radar_detect_ind = 0;
	wapp->dpp->dpp_reconf_announce_try = DPP_RECONF_CH_TRY_MIN;
#endif /* MAP_R3_RECONFIG */
	return 0;
}


void dpp_global_clear(struct dpp_global *dpp)
{
	if (!dpp)
		return;
#ifdef DPP_R2_MUOBJ
	dpp_free_config_info(&(dpp->config_ap[0]), 0, dpp->dpp_conf_ap_num);
	dpp_free_config_info(&(dpp->config_sta[0]), 0, dpp->dpp_conf_sta_num);
#endif /* DPP_R2_MUOBJ */
	dpp_bootstrap_del(dpp, 0);
	dpp_configurator_del(dpp, 0);
#ifdef CONFIG_DPP2
	dpp_tcp_init_flush(dpp);
	dpp_relay_flush_controllers(dpp);
	dpp_controller_stop(dpp);
#endif /* CONFIG_DPP2 */
}


void dpp_global_deinit(struct dpp_global *dpp)
{
	dpp_global_clear(dpp);
	os_free(dpp);
}

struct dpp_bootstrap_info *dpp_get_own_bi(struct dpp_global *dpp)
{
	struct dpp_bootstrap_info *bi = NULL;
	
	if (!dpp)
		return NULL;

	dl_list_for_each(bi, &dpp->bootstrap, struct dpp_bootstrap_info, list) {
		if (bi->id == 1) 
		{
			DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX" found own bi \n");
			return bi;
		}
	}
	return NULL;
}


void sort_upgrade(int *arr, unsigned int len)
{
	unsigned int i = 0, j = 0, temp = 0;

	if( !arr || !len)
		return;

	for( i= 0;i < len-1; i++) {
		for(j = i+1; j < len; j++) {
			if(arr[i] > arr[j]) {
				temp = arr[i];
				arr[i] = arr[j];
				arr[j] = temp;
			}
		}
	}
}

int del_dup(int *arr, unsigned int len)
{
	unsigned int to = 1;
	unsigned int from = 1;
	unsigned int pre = 0;

	if( !arr || !len)
		return 0;
	pre = arr[0];
	for(from = 1; from < len; ++from) {
		if(arr[from] !=pre) {
			arr[to] = arr[from];
			pre = arr[from];
			++to;
		}
	}
	return to;
}

#ifdef MAP_R3
int del_dup_array(int *arr, unsigned int len)
{
	unsigned int i=0,j=0,k=0;
	unsigned int size = 0;

	size = len;

	if( !arr || !len)
		return 0;
	/*
	 * Find duplicate elements in array
	 */
	for(i=0; i<size; i++)
	{
		for(j=i+1; j<size; j++)
		{
			if(arr[i] == arr[j])
			{
				for(k=j; k<size; k++)
				{
					arr[k] = arr[k + 1];
				}
				size--;
				j--;
			}
		}
	}
	return size;
}
#endif /* MAP_R3 */

int dpp_handle_presence_channel_dup(int *chan_list, unsigned int len)
{
	unsigned int  real_len = 0;
	if( !chan_list || !len)
		return 0;

	if(len > 1) {
		sort_upgrade(chan_list, len);
		real_len = del_dup(chan_list, len);
	} else
		real_len = 1;

	return real_len;
}

#ifdef DPP_R2_SUPPORT
#ifdef DPP_R2_RECONFIG
#ifndef OPENSSL_NO_EC
EC_KEY * EVP_PKEY_get0_EC_KEY(EVP_PKEY *pkey)
{
	if (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC) {
		wpa_printf(MSG_ERROR, "DPP: Could not create EVP_PKEY_base_id");
		return NULL;
	}

	return pkey->pkey.ec;
}
#endif /* OPENSSL_NO_EC */

struct dpp_reconfig_id * dpp_gen_reconfig_id(const u8 *csign_key,
					     size_t csign_key_len,
					     const u8 *pp_key,
					     size_t pp_key_len)
{
	const unsigned char *p;
	EVP_PKEY *csign = NULL, *ppkey = NULL;
	struct dpp_reconfig_id *id = NULL;
	BN_CTX *ctx = NULL;
	BIGNUM *bn = NULL, *q = NULL;
	const EC_KEY *eckey;
	const EC_GROUP *group;
	EC_POINT *e_id = NULL;

	p = csign_key;
	csign = d2i_PUBKEY(NULL, &p, csign_key_len);
	if (!csign)
		goto fail;

	if (!pp_key)
		goto fail;
	p = pp_key;
	ppkey = d2i_PUBKEY(NULL, &p, pp_key_len);
	if (!ppkey)
		goto fail;

	eckey = EVP_PKEY_get0_EC_KEY(csign);
	if (!eckey)
		goto fail;
	group = EC_KEY_get0_group(eckey);
	if (!group)
		goto fail;

	e_id = EC_POINT_new(group);
	ctx = BN_CTX_new();
	bn = BN_new();
	q = BN_new();
	if (!e_id || !ctx || !bn || !q ||
	    !EC_GROUP_get_order(group, q, ctx) ||
	    !BN_rand_range(bn, q) ||
	    !EC_POINT_mul(group, e_id, bn, NULL, NULL, ctx))
		goto fail;

	dpp_debug_print_point("DPP: Generated random point E-id", group, e_id);

	id = os_zalloc(sizeof(*id));
	if (!id)
		goto fail;
	id->group = group;
	id->e_id = e_id;
	e_id = NULL;
	id->csign = csign;
	csign = NULL;
	id->pp_key = ppkey;
	ppkey = NULL;
fail:
	EC_POINT_free(e_id);
	EVP_PKEY_free(csign);
	EVP_PKEY_free(ppkey);
	BN_clear_free(bn);
	BN_CTX_free(ctx);
        if (q)
		BN_clear_free(q);
	
	return id;
}

static EVP_PKEY * dpp_pkey_from_point(const EC_GROUP *group,
				      const EC_POINT *point)
{
	EC_KEY *eckey;
	EVP_PKEY *pkey = NULL;

	eckey = EC_KEY_new();
	if (!eckey ||
	    EC_KEY_set_group(eckey, group) != 1 ||
	    EC_KEY_set_public_key(eckey, point) != 1) {
		wpa_printf(MSG_ERROR,
			   "DPP: Failed to set EC_KEY: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	EC_KEY_set_asn1_flag(eckey, OPENSSL_EC_NAMED_CURVE);

	pkey = EVP_PKEY_new();
	if (!pkey || EVP_PKEY_set1_EC_KEY(pkey, eckey) != 1) {
		wpa_printf(MSG_ERROR, "DPP: Could not create EVP_PKEY");
		EVP_PKEY_free(pkey);
		pkey = NULL;
		goto fail;
	}

fail:
	EC_KEY_free(eckey);
	return pkey;
}


int dpp_update_reconfig_id(struct dpp_reconfig_id *id)
{
	BN_CTX *ctx = NULL;
	BIGNUM *bn = NULL, *q = NULL;
	EC_POINT *e_prime_id = NULL, *a_nonce = NULL;
	int ret = -1;
	const EC_KEY *pp;
	const EC_POINT *pp_point;

	pp = EVP_PKEY_get0_EC_KEY(id->pp_key);
	if (!pp)
		goto fail;
	pp_point = EC_KEY_get0_public_key(pp);
	e_prime_id = EC_POINT_new(id->group);
	a_nonce = EC_POINT_new(id->group);
	ctx = BN_CTX_new();
	bn = BN_new();
	q = BN_new();
	/* Generate random 0 <= a-nonce < q
	 * A-NONCE = a-nonce * G
	 * E'-id = E-id + a-nonce * P_pk */
	if (!pp_point || !e_prime_id || !a_nonce || !ctx || !bn || !q ||
	    !EC_GROUP_get_order(id->group, q, ctx) ||
	    !BN_rand_range(bn, q) || /* bn = a-nonce */
	    !EC_POINT_mul(id->group, a_nonce, bn, NULL, NULL, ctx) ||
	    !EC_POINT_mul(id->group, e_prime_id, NULL, pp_point, bn, ctx) ||
	    !EC_POINT_add(id->group, e_prime_id, id->e_id, e_prime_id, ctx))
		goto fail;

	dpp_debug_print_point("DPP: Generated A-NONCE", id->group, a_nonce);
	dpp_debug_print_point("DPP: Encrypted E-id to E'-id",
			      id->group, e_prime_id);

	EVP_PKEY_free(id->a_nonce);
	EVP_PKEY_free(id->e_prime_id);
	id->a_nonce = dpp_pkey_from_point(id->group, a_nonce);
	id->e_prime_id = dpp_pkey_from_point(id->group, e_prime_id);
	if (!id->a_nonce || !id->e_prime_id)
		goto fail;

	ret = 0;

fail:
	EC_POINT_free(e_prime_id);
	EC_POINT_free(a_nonce);
	BN_clear_free(bn);
	BN_CTX_free(ctx);
	if (q)
		BN_clear_free(q);
	return ret;
}

void dpp_free_reconfig_id(struct dpp_reconfig_id *id)
{
	if (id) {
		EC_POINT_clear_free(id->e_id);
		EVP_PKEY_free(id->csign);
		EVP_PKEY_free(id->a_nonce);
		EVP_PKEY_free(id->e_prime_id);
		EVP_PKEY_free(id->pp_key);
		os_free(id);
	}
}

EC_POINT * dpp_decrypt_e_id(EVP_PKEY *ppkey, EVP_PKEY *a_nonce,
			    EVP_PKEY *e_prime_id)
{
	const EC_KEY *pp_ec, *a_nonce_ec, *e_prime_id_ec;
	const BIGNUM *pp_bn;
	const EC_GROUP *group;
	EC_POINT *e_id = NULL;
	const EC_POINT *a_nonce_point, *e_prime_id_point;
	BN_CTX *ctx = NULL;

	if (!ppkey)
		return NULL;

	/* E-id = E'-id - s_C * A-NONCE */
	pp_ec = EVP_PKEY_get0_EC_KEY(ppkey);
	a_nonce_ec = EVP_PKEY_get0_EC_KEY(a_nonce);
	e_prime_id_ec = EVP_PKEY_get0_EC_KEY(e_prime_id);
	if (!pp_ec || !a_nonce_ec || !e_prime_id_ec)
		return NULL;
	pp_bn = EC_KEY_get0_private_key(pp_ec);
	group = EC_KEY_get0_group(pp_ec);
	a_nonce_point = EC_KEY_get0_public_key(a_nonce_ec);
	e_prime_id_point = EC_KEY_get0_public_key(e_prime_id_ec);
	ctx = BN_CTX_new();
	if (!pp_bn || !group || !a_nonce_point || !e_prime_id_point || !ctx)
		goto fail;
	e_id = EC_POINT_new(group);
	if (!e_id ||
	    !EC_POINT_mul(group, e_id, NULL, a_nonce_point, pp_bn, ctx) ||
	    !EC_POINT_invert(group, e_id, ctx) ||
	    !EC_POINT_add(group, e_id, e_prime_id_point, e_id, ctx)) {
		EC_POINT_clear_free(e_id);
		goto fail;
	}

	dpp_debug_print_point("DPP: Decrypted E-id", group, e_id);

fail:
	BN_CTX_free(ctx);
	return e_id;
}
#endif
struct wpabuf * dpp_build_annouce_frame(struct dpp_global *dpp, struct dpp_bootstrap_info *dpp_bi)
{
	struct wpabuf *msg =  NULL;
	size_t attr_len = 0;
	struct dpp_bootstrap_info *bi = NULL;

	if (!dpp)
		return NULL;
	
	/* Build DPP Authentication Request frame attributes */
	attr_len = 4 + SHA256_MAC_LEN;

	msg = dpp_alloc_msg(DPP_PA_PRESENCE_ANNOUNCEMENT, attr_len);
	if (!msg)
		return NULL;

	if(!dpp_bi) {
		/* gen bootstrap key hash */
		bi = dpp_get_own_bi(dpp);
		if (!bi) {
			dpp_bootstrap_gen_at_bootup(dpp, (char *)dpp->dpp_private_key, NULL, NULL);
			bi = dpp_get_own_bi(dpp);
			if (!bi)
				goto fail;
		}
	} else
		bi = dpp_bi;

	/* responser Bootstrapping Key Hash */
	dpp_build_attr_r_bootstrap_key_hash(msg, bi->chirp_hash);

	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX" build frame done \n");
	return msg;
fail:
	DBGPRINT(RT_DEBUG_ERROR, DPP_MAP_PREX" build frame fail\n");
	os_free(msg);
	return NULL;
}

int dpp_check_valid_channel(struct dpp_global *dpp, unsigned int chan)
{
	int ret = 0;
	struct wapp_dev * wdev=  NULL;

	if (IS_MAP_CH_5GH(chan)) {
		wdev = dpp->default_5gh_iface;
	} else if (IS_MAP_CH_5GL(chan)) {
		wdev = dpp->default_5gl_iface;
	} else if (IS_MAP_CH_24G(chan)) {
		wdev = dpp->default_2g_iface;
	}

	if(wdev)
		ret = 1;

	return ret;
}

#ifdef MAP_R3
void wapp_dpp_ch_prefer_list_prepare(struct wifi_app *wapp,struct dpp_pre_annouce_info * annouce_info,
						unsigned int * real_num)
{
	unsigned int pref_cnt = 0, cce_num = 0;
	unsigned int pref_chan[DPP_PRESENCE_CH_MAX] = {0};
	unsigned int ch_list_cnt = 0;

	ch_list_cnt = *real_num;

	/* Add the ch on which reconf triggered first */
	if (wapp->map->TurnKeyEnable && (annouce_info->reconf_ch != 0))
		pref_chan[pref_cnt++] = annouce_info->reconf_ch;

	if(wapp->radio_count == MAX_RADIO_DBDC) {
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"band priority value:%s\n",wapp->dpp->band_priority);
		if ((os_strcasecmp(wapp->dpp->band_priority, "5g") == 0)
				|| (os_strcasecmp(wapp->dpp->band_priority, "5gl") == 0)
				|| (os_strcasecmp(wapp->dpp->band_priority, "5gh") == 0)) {
			int ch = 0;
			for(ch = 0; ch < ch_list_cnt; ch++) {
				if(IS_MAP_CH_5G(annouce_info->chan[ch])) {
					pref_chan[pref_cnt++] = annouce_info->chan[ch];
				}
			}
		}
		else {
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX" Band priority is set to 24g no need to modify priority\n");
		}
	}
	if(wapp->radio_count == MAP_MAX_RADIO) {
		DBGPRINT(RT_DEBUG_WARN, DPP_MAP_PREX"band priority value:%s\n",wapp->dpp->band_priority);
		if ((os_strcasecmp(wapp->dpp->band_priority, "5g") == 0)
				|| (os_strcasecmp(wapp->dpp->band_priority, "5gl") == 0)) {
			int ch = 0;
			for(ch = 0; ch < ch_list_cnt; ch++) {
				if(IS_MAP_CH_5GL(annouce_info->chan[ch])) {
					pref_chan[pref_cnt++] = annouce_info->chan[ch];
				}
				if(IS_MAP_CH_5GH(annouce_info->chan[ch])) {
					pref_chan[pref_cnt++] = annouce_info->chan[ch];
				}
			}
		}
		else if ((os_strcasecmp(wapp->dpp->band_priority, "5gh") == 0)) {
			int ch = 0;
			for(ch = 0; ch < ch_list_cnt; ch++) {
				if(IS_MAP_CH_5GH(annouce_info->chan[ch])) {
					pref_chan[pref_cnt++] = annouce_info->chan[ch];
				}
				if(IS_MAP_CH_5GL(annouce_info->chan[ch])) {
					pref_chan[pref_cnt++] = annouce_info->chan[ch];
				}
			}
		}
		else {
			DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX" Band priority is set to 24g no need to modify priority\n");
		}
	}
	if(pref_cnt != 0) {
		/* copy the whole channel list first and then delete the duplicate ones */
		os_memcpy(pref_chan + pref_cnt, annouce_info->chan , ch_list_cnt *sizeof(pref_chan[0]));
		cce_num = ch_list_cnt + pref_cnt;
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: Pref chan list ", pref_chan, cce_num);
		ch_list_cnt = del_dup_array((int *)pref_chan, cce_num);
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP:Pref chan list  after duplicate delete", pref_chan, ch_list_cnt);
		os_memset(annouce_info->chan, 0, DPP_PRESENCE_CH_MAX-1);
		os_memcpy(annouce_info->chan, pref_chan , ch_list_cnt *sizeof(pref_chan[0]));
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: Pref chan list in announce", pref_chan, ch_list_cnt);
		*real_num = ch_list_cnt;
	}
}

void wapp_dpp_parse_ch_list_agnt_uri(struct wifi_app *wapp, struct dpp_bootstrap_info *bi)
{
	const char *chan_list = NULL;
	const char *pos = bi->uri;
	const char *end;

	if (os_strncmp(pos, "DPP:", 4) != 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Not a DPP URI");
		return;
	}
	pos += 4;

	for (;;) {
		end = os_strchr(pos, ';');
		if (!end)
			break;

		if (end == pos) {
			/* Handle terminating ";;" and ignore unexpected ";"
			 * for parsing robustness. */
			pos++;
			continue;
		}

		if (pos[0] == 'C' && pos[1] == ':' && !chan_list)
			chan_list = pos + 2;
		pos = end + 1;
	}

	if(chan_list) {
		dpp_parse_uri_chan_list(bi, chan_list
#ifdef MAP_R3
		, wapp->dpp
#endif
		);
	}
}

int wapp_dpp_ch_list_chirp_del(int *ch_list, unsigned int ch,
					unsigned int tot_num)
{
	int i, k = 0;
	int total_channel[64] = {0};
	if (!ch_list)
		return 0;
	os_memcpy(total_channel, ch_list, tot_num);
	/*os_free(ch_list);
	ch_list = os_zalloc(tot_num - 1);
	if (!ch_list)
		return 0;*/
	os_memset(ch_list, 0, tot_num);
	for (i = 0; i < tot_num; i++) {
		if (ch != total_channel[i]) {
			ch_list[k++] = total_channel[i];
		}
	}
	return k;
}

int dpp_map_wsc_done_chirp_channel_list(struct wifi_app *wapp, struct dpp_pre_annouce_info *annouce_info)
{
	unsigned int ch_cnt = 0;
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list;

	if (!wapp || !annouce_info) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "failed to find dpp, not initialized\n");
		return 0;
	}

	// reset all
	os_memset(annouce_info->chan, 0, DPP_PRESENCE_CH_MAX-1);
	annouce_info->ch_num = 0;

	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX"%s\n", __func__);
	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
			annouce_info->chan[ch_cnt] = wdev->radio->op_ch;
			ch_cnt++;
		}
	}

	if(ch_cnt == 0) {
		return ch_cnt;
	}

#ifdef MAP_R3
	wapp_dpp_ch_prefer_list_prepare(wapp, annouce_info, &ch_cnt);
#endif /* MAP_R3 */
	annouce_info->ch_num = ch_cnt;
	annouce_info->cur_presence_chan_id = 0;
	annouce_info->presence_retry = 0;
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX" gen channel list: total num %d \n", ch_cnt);
#ifdef MAP_6E_SUPPORT
	annouce_info->cur_presence_chan_id_6g = 0;
#endif
	return annouce_info->ch_num;
}
#endif /* MAP_R3 */

int dpp_gen_presence_annouce_channel_list(struct dpp_global *dpp, struct dpp_pre_annouce_info *annouce_info)
{
	unsigned int  i = 0, cce_num = 0, real_num = 0;
	struct dpp_bootstrap_info *bi = NULL;
	u8 k = 0;
#ifdef MAP_6E_SUPPORT
	u8 real_num_6g = 0;
#endif

	if (!dpp || !annouce_info) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "failed to find dpp, not initialized\n");
		return 0;
	}

#ifdef MAP_R3
	u8 def_ch_num = 0, adj_chan = 0;
	int var = 0, j = 0;
	struct wifi_app *wapp = (struct wifi_app *)dpp->msg_ctx;
	if (!wapp) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "failed to find wapp, not initialized\n");
		return 0;
	}
#endif /* MAP_R3 */

	// reset all
	os_memset(annouce_info->chan, 0, DPP_CCE_CH_MAX-1);
	annouce_info->ch_num = 0;

#ifdef MAP_R3
	/* As part of optimization do not add default and URI channela in channel list */
	if(wapp->map->TurnKeyEnable == 0) {
#endif /* MAP_R3 */
		/* prefer chan list */
		if(dpp->default_2g_iface) {
			annouce_info->chan[i] = 6;
			i++;
		}

		if(dpp->default_5gl_iface) {
#ifdef MAP_R3
			def_ch_num = 44;
			if(wapp_dpp_ch_validation(wapp, (u8)def_ch_num)) {
				annouce_info->chan[i] = 44;
				i++;
			}

			if(wapp->radio_count == MAX_RADIO_DBDC) {
				def_ch_num = 149;
				if(wapp_dpp_ch_validation(wapp, (u8)def_ch_num)) {
					annouce_info->chan[i] = 149;
					i++;
				}
			}
#else
			annouce_info->chan[i] = 44;
			i++;
#endif /* MAP_R3 */
		}

		if(dpp->default_5gh_iface
#ifdef MAP_R3
				&& (wapp->radio_count == MAP_MAX_RADIO)
#endif /* MAP_R3 */
		  ) {
#ifdef MAP_R3
			def_ch_num = 149;
			if(wapp_dpp_ch_validation(wapp, (u8)def_ch_num)) {
				annouce_info->chan[i] = 149;
				i++;
			}

#else
			annouce_info->chan[i] = 149;
			i++;
#endif /* MAP_R3 */
		}
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: basic chanel list ", annouce_info->chan, i);
		// get URI channel list
		bi = dpp_get_own_bi(dpp);
		if (bi && bi->num_chan) {
			int tmp_num = 0;
			unsigned tmp_ch[DPP_BOOTSTRAP_MAX_FREQ] = {0};
			DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX" cce channel len: %d [%d] \n", i, bi->num_chan);
			cce_num = 0;
			for(cce_num = 0; cce_num <  bi->num_chan; cce_num++) {
#ifdef MAP_R3
				if (wapp_dpp_ch_chirp_valid(wapp, (u8)bi->chan[cce_num]) != -1)
#else
				if (dpp_check_valid_channel(dpp, bi->chan[cce_num]))
#endif /* MAP_R3 */
					{
						tmp_ch[tmp_num] = bi->chan[cce_num];
						tmp_num ++;
					}
				}
				//
				cce_num = tmp_num;
				if(cce_num > 0) {
					if((i + tmp_num) >= DPP_BOOTSTRAP_MAX_FREQ)
						cce_num = DPP_BOOTSTRAP_MAX_FREQ - i - 1;

					os_memcpy(annouce_info->chan + i, tmp_ch, cce_num *sizeof(tmp_ch[0]));
					i = i+cce_num;
					wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: add bi info chanel list ", annouce_info->chan, i);
				}

			}
#ifdef MAP_R3
	}
#endif /* MAP_R3 */

	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX" gen cce channel len: %d [%d, %d, %d]\n", i,
		dpp->cce_ch.cce_2g.ch_num, dpp->cce_ch.cce_5gl.ch_num, dpp->cce_ch.cce_5gh.ch_num);
	// get CCE chan list
	if(dpp->default_2g_iface && dpp->cce_ch.cce_2g.ch_num) {
		cce_num = dpp->cce_ch.cce_2g.ch_num;
		if((i + dpp->cce_ch.cce_2g.ch_num) >= DPP_CCE_CH_MAX)
			cce_num = DPP_CCE_CH_MAX - i - 1;
#ifdef MAP_R3
                /* handle 2.4G adjacent channels on every iterations */
                if(wapp->map->TurnKeyEnable && (cce_num > 1)) {
                        sort_upgrade((int *)dpp->cce_ch.cce_2g.chan, dpp->cce_ch.cce_2g.ch_num);
                        int chnl_num = 0;
			if(dpp->prev_chan_detected != 0) {
				for(k = 0; k < dpp->prev_chan_detected; k++) {
					for(chnl_num = 0; chnl_num < cce_num; chnl_num++) {
							if((dpp->cce_ch.cce_2g.chan[chnl_num] == (dpp->cce_ch.cce_2g.chan[chnl_num+1] - 1)
							|| (chnl_num > 0 && (dpp->cce_ch.cce_2g.chan[chnl_num] == (dpp->cce_ch.cce_2g.chan[chnl_num-1] + 1)))) &&
							(dpp->cce_ch.cce_2g.chan[chnl_num] == dpp->prev_chan[k]))
								dpp->cce_ch.cce_2g.chan[chnl_num] = 0;
					}
				}
				os_memset(dpp->prev_chan, 0, dpp->prev_chan_detected);
				dpp->prev_chan_detected = 0;
			}
			else {
				for(chnl_num = 1; chnl_num < cce_num; chnl_num++) {
					if(dpp->cce_ch.cce_2g.chan[chnl_num] == (dpp->cce_ch.cce_2g.chan[chnl_num-1] + 1)) {
						dpp->cce_ch.cce_2g.chan[chnl_num] = 0;
					}
				}
			}
			for(chnl_num = 0; chnl_num < cce_num; chnl_num++) {
				if(dpp->cce_ch.cce_2g.chan[chnl_num] == 0) {
					for(var=chnl_num; var < cce_num; var++) {
						dpp->cce_ch.cce_2g.chan[chnl_num] = dpp->cce_ch.cce_2g.chan[chnl_num+1];
					}
					cce_num--;
					adj_chan = 1;
				}
			}
			if(adj_chan) {
				for(chnl_num = 0; chnl_num < cce_num; chnl_num++) {
					dpp->prev_chan[j++] = dpp->cce_ch.cce_2g.chan[chnl_num];
					dpp->prev_chan_detected++;
				}
			}
		}
#endif /* MAP_R3 */

		os_memcpy(annouce_info->chan + i, dpp->cce_ch.cce_2g.chan, cce_num * sizeof(dpp->cce_ch.cce_2g.chan[0]));
		i = i+cce_num;
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: add 2g cce info chanel list ", annouce_info->chan, i);
#ifdef MAP_R3
		/* Reset the ch num after update to announce info */
		dpp->cce_ch.cce_2g.ch_num = 0;
#endif /* MAP_R3 */
	}

	if(dpp->default_5gl_iface && dpp->cce_ch.cce_5gl.ch_num) {
		cce_num = dpp->cce_ch.cce_5gl.ch_num;
		if((i + dpp->cce_ch.cce_5gl.ch_num) >= DPP_CCE_CH_MAX)
			cce_num = DPP_CCE_CH_MAX - i - 1;

		os_memcpy(annouce_info->chan + i, dpp->cce_ch.cce_5gl.chan, cce_num * sizeof(bi->chan[0]));
		i = i+cce_num;
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: add 5gl cce info chanel list ", annouce_info->chan, i);
#ifdef MAP_R3
		/* Reset the ch num after update to announce info */
		dpp->cce_ch.cce_5gl.ch_num = 0;
#endif /* MAP_R3 */
	}

	if(dpp->default_5gh_iface && dpp->cce_ch.cce_5gh.ch_num) {
		cce_num = dpp->cce_ch.cce_5gh.ch_num;
		if((i + dpp->cce_ch.cce_5gh.ch_num) >= DPP_CCE_CH_MAX)
			cce_num = DPP_CCE_CH_MAX - i - 1;

		os_memcpy(annouce_info->chan + i, dpp->cce_ch.cce_5gh.chan, cce_num * sizeof(bi->chan[0]));
		i = i+cce_num;
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: add 5gh cce info chanel list ", annouce_info->chan, i);
#ifdef MAP_R3
		/* Reset the ch num after update to announce info */
		dpp->cce_ch.cce_5gh.ch_num = 0;
#endif /* MAP_R3 */
	}
#ifdef MAP_6E_SUPPORT
	if (dpp->cce_ch.cce_6g.ch_num > 0) {
		cce_num = dpp->cce_ch.cce_6g.ch_num;
		if ((k + dpp->cce_ch.cce_6g.ch_num) >= DPP_CCE_CH_MAX)
			cce_num = DPP_CCE_CH_MAX - k - 1;

		os_memcpy(annouce_info->chan_6g + k, dpp->cce_ch.cce_6g.chan, cce_num * sizeof(bi->chan[0]));
		k = k+cce_num;
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: add 6G cce info channel list ", annouce_info->chan_6g, k);
#ifdef MAP_R3
		/* Reset the ch num after update to announce info */
		dpp->cce_ch.cce_6g.ch_num = 0;
#endif /* MAP_R3 */
	}
#endif

#ifdef MAP_R3
	/* Adding the user configured ch list in presence announcement */
	if(dpp->chirp_add_ch_num > 0)
	{
		u8 ch_add = 0;
		for(ch_add = 0; ch_add < (unsigned int)dpp->chirp_add_ch_num; ch_add++) {
			annouce_info->chan[i++] = (unsigned int)dpp->chirp_list_add[ch_add];
		}
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP:Updated ch list after adding user ch list ", annouce_info->chan, i);
	}

	/* Deleting the user configured ch list from presence announcement */
	if(dpp->chirp_del_ch_num > 0) {
		u8 ch_del = 0,ch_list = 0, up_done = 0, loop_ch = 0;

		for(ch_del = 0; ch_del < (unsigned int)dpp->chirp_del_ch_num; ch_del++) {
			for(ch_list = 0; ch_list < i; ch_list++) {
				if((unsigned int )dpp->chirp_list_del[ch_del] == annouce_info->chan[ch_list]) {
					up_done++;
					loop_ch = ch_list;
					while(loop_ch < i) {
						if (loop_ch == (DPP_CCE_CH_MAX - 1)) {
							wpa_printf(MSG_ERROR,
							"DPP: %s %d Invalid ,loop_ch is DPP_CCE_CH_MAX -1", __func__, __LINE__);
							break;
						}

						annouce_info->chan[loop_ch] = annouce_info->chan[loop_ch+1];
						loop_ch ++;
					}
				}
				else
					continue;
			}
		}
		if(up_done != 0) {
			i = i - up_done;
		}
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP:Updated ch list after deleting user ch list ", annouce_info->chan, i);
	}

#endif /* MAP_R3 */

	//DBGPRINT(RT_DEBUG_TRACE," gen all channel len[%d]\n", i);
	wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: dup before chanel list ", annouce_info->chan, i);

	//remove dup
	real_num = dpp_handle_presence_channel_dup ((int* )annouce_info->chan, i);
#ifdef MAP_6E_SUPPORT
	real_num_6g = dpp_handle_presence_channel_dup((int *)annouce_info->chan_6g, k);
#endif
#ifdef MAP_R3
	wapp_dpp_ch_prefer_list_prepare(wapp, annouce_info, &real_num);
#endif /* MAP_R3 */

	wpa_dump(MSG_INFO1, DPP_MAP_PREX "DPP: dup after chanel list ", annouce_info->chan, real_num);

	annouce_info->ch_num = real_num;
	annouce_info->cur_presence_chan_id = 0;
	annouce_info->presence_retry = 0;
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX" gen channel list: total num %d \n", real_num);
#ifdef MAP_6E_SUPPORT
	annouce_info->ch_num_6g = real_num_6g;
	annouce_info->cur_presence_chan_id_6g = 0;
	return annouce_info->ch_num + annouce_info->ch_num_6g;
#endif
	return annouce_info->ch_num;
}

int dpp_gen_reconfig_annouce_channel_list(struct dpp_global *dpp, struct dpp_annouce_info *annouce_info
			, u8 wdev_ch)
{
        unsigned int  i = 0, cce_num = 0, real_num = 0;
        struct dpp_bootstrap_info *bi = NULL;

        if (!dpp || !annouce_info) {
                wpa_printf(MSG_INFO1, DPP_MAP_PREX "failed to find dpp, not initialized\n");
                return 0;
        }

#ifdef MAP_R3
        u8 def_ch_num = 0;
        struct wifi_app *wapp = (struct wifi_app *)dpp->msg_ctx;
        if (!wapp) {
                wpa_printf(MSG_INFO1, DPP_MAP_PREX "failed to find wapp, not initialized\n");
                return 0;
        }
#endif /* MAP_R3 */

        // reset all
        os_memset(annouce_info->chan, 0, DPP_CCE_CH_MAX-1);
        annouce_info->ch_num = 0;

	/* Do not include default channels in the list for TurnKey mode */
	if (!wapp->map->TurnKeyEnable) {
		/* prefer chan list */
		if (dpp->default_2g_iface) {
			annouce_info->chan[i] = 6;
			i++;
		}

		if (dpp->default_5gl_iface) {
#ifdef MAP_R3
			def_ch_num = 44;
			if (wapp_dpp_ch_validation(wapp, (u8)def_ch_num)) {
				annouce_info->chan[i] = 44;
				i++;
			}

			if (wapp->radio_count == MAX_RADIO_DBDC) {
				def_ch_num = 149;
				if (wapp_dpp_ch_validation(wapp, (u8)def_ch_num)) {
					annouce_info->chan[i] = 149;
					i++;
				}
			}
#else
			annouce_info->chan[i] = 44;
			i++;
#endif /* MAP_R3 */
		}

		if (dpp->default_5gh_iface
#ifdef MAP_R3
				&& (wapp->radio_count == MAP_MAX_RADIO)
#endif /* MAP_R3 */
		  ) {
#ifdef MAP_R3
			def_ch_num = 149;
			if (wapp_dpp_ch_validation(wapp, (u8)def_ch_num)) {
				annouce_info->chan[i] = 149;
				i++;
			}

#else
			annouce_info->chan[i] = 149;
			i++;
#endif /* MAP_R3 */
		}
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: basic chanel list ", annouce_info->chan, i);
	}

/* Do not include URI channels in the list as per EasyconnectV2.0 spec */
#if 0
	/* get URI channel list */
	bi = dpp_get_own_bi(dpp);
	if (bi && bi->num_chan) {
		int tmp_num = 0;
		unsigned tmp_ch[DPP_BOOTSTRAP_MAX_FREQ] = {0};

		DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX" cce channel len: %d [%d]\n", i, bi->num_chan);
		cce_num = 0;
		for (cce_num = 0; cce_num <  bi->num_chan; cce_num++) {
#ifdef MAP_R3
			if (wapp_dpp_ch_validation(wapp, (u8)bi->chan[cce_num]))
#else
				if (dpp_check_valid_channel(dpp, bi->chan[cce_num]))
#endif /* MAP_R3 */
				{
					tmp_ch[tmp_num] = bi->chan[cce_num];
					tmp_num++;
				}
		}
		cce_num = tmp_num;
		if (cce_num > 0) {
			if ((i + tmp_num) > DPP_PRESENCE_CH_MAX)
				cce_num = DPP_PRESENCE_CH_MAX - i;

			os_memcpy(annouce_info->chan + i, tmp_ch, cce_num * sizeof(tmp_ch[0]));
			i = i+cce_num;
			wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: add bi info chanel list ", annouce_info->chan, i);
		}

	}
#endif

	/* Do not include default channels in the list for TurnKey mode */
	if (wapp->map->TurnKeyEnable)
		annouce_info->reconf_ch = wdev_ch;
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX "DPP: reconf chanel %d\n", annouce_info->reconf_ch);

	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX" gen cce channel len: %d [%d, %d, %d]\n", i,
			dpp->cce_ch.cce_2g.ch_num, dpp->cce_ch.cce_5gl.ch_num, dpp->cce_ch.cce_5gh.ch_num);
	/* get CCE chan list */
	if (dpp->default_2g_iface && dpp->cce_ch.cce_2g.ch_num) {
		cce_num = dpp->cce_ch.cce_2g.ch_num;
		if ((i + dpp->cce_ch.cce_2g.ch_num) >= DPP_CCE_CH_MAX)
			cce_num = DPP_CCE_CH_MAX - i - 1;

		os_memcpy(annouce_info->chan + i, dpp->cce_ch.cce_2g.chan, cce_num * sizeof(dpp->cce_ch.cce_2g.chan[0]));
		i = i+cce_num;
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: add 2g cce info chanel list ", annouce_info->chan, i);
#ifdef MAP_R3
		/* Reset the ch num after update to announce info */
		dpp->cce_ch.cce_2g.ch_num = 0;
#endif /* MAP_R3 */
	}

	if (dpp->default_5gl_iface && dpp->cce_ch.cce_5gl.ch_num) {
		cce_num = dpp->cce_ch.cce_5gl.ch_num;
		if ((i + dpp->cce_ch.cce_5gl.ch_num) >= DPP_CCE_CH_MAX)
			cce_num = DPP_CCE_CH_MAX - i - 1;

		os_memcpy(annouce_info->chan + i, dpp->cce_ch.cce_5gl.chan, cce_num * sizeof(bi->chan[0]));
		i = i+cce_num;
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: add 5gl cce info chanel list ", annouce_info->chan, i);
#ifdef MAP_R3
		/* Reset the ch num after update to announce info */
		dpp->cce_ch.cce_5gl.ch_num = 0;
#endif /* MAP_R3 */
	}

	if (dpp->default_5gh_iface && dpp->cce_ch.cce_5gh.ch_num) {
		cce_num = dpp->cce_ch.cce_5gh.ch_num;
		if ((i + dpp->cce_ch.cce_5gh.ch_num) >= DPP_CCE_CH_MAX)
			cce_num = DPP_CCE_CH_MAX - i - 1;

		os_memcpy(annouce_info->chan + i, dpp->cce_ch.cce_5gh.chan, cce_num * sizeof(bi->chan[0]));
		i = i+cce_num;
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: add 5gh cce info chanel list ", annouce_info->chan, i);
#ifdef MAP_R3
		/* Reset the ch num after update to announce info */
		dpp->cce_ch.cce_5gh.ch_num = 0;
#endif /* MAP_R3 */
	}

#ifdef MAP_R3
	/* Adding the user configured ch list in presence announcement */
	if (dpp->chirp_add_ch_num > 0) {
		u8 ch_add = 0;

		for (ch_add = 0; ch_add < (unsigned int)dpp->chirp_add_ch_num; ch_add++)
			annouce_info->chan[i++] = (unsigned int)dpp->chirp_list_add[ch_add];

		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP:Updated ch list after adding user ch list ", annouce_info->chan, i);
	}

	/* Deleting the user configured ch list from presence announcement */
	if (dpp->chirp_del_ch_num > 0) {
		u8 ch_del = 0, ch_list = 0, up_done = 0, loop_ch = 0;

		for (ch_del = 0; ch_del < (unsigned int)dpp->chirp_del_ch_num; ch_del++) {
			for (ch_list = 0; ch_list < i; ch_list++) {
				if ((unsigned int)dpp->chirp_list_del[ch_del] == annouce_info->chan[ch_list]) {
					up_done++;
					loop_ch = ch_list;
					while (loop_ch < (i - 1)) {
						annouce_info->chan[loop_ch] = annouce_info->chan[loop_ch+1];
						loop_ch++;
					}
				} else
					continue;
			}
		}

		if (up_done != 0)
			i = i - up_done;

		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP:Updated ch list after deleting user ch list ", annouce_info->chan, i);
	}

#endif /* MAP_R3 */

	/* DBGPRINT(RT_DEBUG_TRACE," gen all channel len[%d]\n", i); */
	wpa_dump(MSG_INFO1, DPP_MAP_PREX "DPP: dup before chanel list ", annouce_info->chan, i);

	/* remove dup */
	real_num = dpp_handle_presence_channel_dup((int *)annouce_info->chan, i);

#ifdef MAP_R3
        wapp_dpp_ch_prefer_list_prepare(wapp, (struct dpp_pre_annouce_info *)annouce_info, &real_num);
#endif /* MAP_R3 */

        wpa_dump(MSG_INFO1, DPP_MAP_PREX "DPP: dup after chanel list ", annouce_info->chan, real_num);
	annouce_info->ch_num = real_num;
        annouce_info->cur_an_chan_id = 0;
	annouce_info->an_retry = 0;
        DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX" gen channel list: total num %d \n", real_num);
        return annouce_info->ch_num;
}


#if 0
int dpp_gen_reconfig_annouce_channel_list(struct dpp_global *dpp, struct dpp_annouce_info *annouce_info)
{
	unsigned int  i = 0, cce_num = 0, real_num = 0;
	struct dpp_bootstrap_info *bi = NULL;

	if (!dpp || !annouce_info) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "failed to find dpp, not initialized\n");
		return 0;
	}

#ifdef MAP_R3
	u8 def_ch_num = 0;
	struct wifi_app *wapp = (struct wifi_app *)dpp->msg_ctx;
	if (!wapp) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "failed to find wapp, not initialized\n");
		return 0;
	}
#endif /* MAP_R3 */

	// reset all
	os_memset(annouce_info->chan, 0, DPP_PRESENCE_CH_MAX);
	annouce_info->ch_num = 0;

	/* prefer chan list */
	if(dpp->default_2g_iface) {
		annouce_info->chan[i] = 6;
		i++;
	}

	if(dpp->default_5gl_iface) {
#ifdef MAP_R3
		def_ch_num = 44;
		if(wapp_dpp_ch_validation(wapp, (u8)def_ch_num)) {
			annouce_info->chan[i] = 44;
			i++;
		}

		if(wapp->radio_count == MAX_RADIO_DBDC) {
			def_ch_num = 149;
			if(wapp_dpp_ch_validation(wapp, (u8)def_ch_num)) {
				annouce_info->chan[i] = 149;
				i++;
			}
		}
#else
		annouce_info->chan[i] = 44;
		i++;
#endif /* MAP_R3 */
	}

	if(dpp->default_5gh_iface
#ifdef MAP_R3
		&& (wapp->radio_count == MAP_MAX_RADIO)
#endif /* MAP_R3 */
	  ) {
#ifdef MAP_R3
		def_ch_num = 149;
		if(wapp_dpp_ch_validation(wapp, (u8)def_ch_num)) {
			annouce_info->chan[i] = 149;
			i++;
		}

#else
		annouce_info->chan[i] = 149;
		i++;
#endif /* MAP_R3 */
	}
	wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: basic chanel list ", annouce_info->chan, i);
#if 0
	// get URI channel list
	bi = dpp_get_own_bi(dpp);
	if (bi && bi->num_chan) {
		int tmp_num = 0;
		unsigned tmp_ch[DPP_BOOTSTRAP_MAX_FREQ] = {0};
		DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX" cce channel len: %d [%d] \n", i, bi->num_chan);
		cce_num = 0;
		for(cce_num = 0; cce_num <  bi->num_chan; cce_num++) {
#ifdef MAP_R3
			if(wapp_dpp_ch_validation(wapp, (u8)bi->chan[cce_num]))
#else
			if(dpp_check_valid_channel(dpp, bi->chan[cce_num]))
#endif /* MAP_R3 */
			{
				tmp_ch[tmp_num] = bi->chan[cce_num];
				tmp_num ++;
			}
		}
		//
		cce_num = tmp_num;
		if(cce_num > 0) {
			if((i + tmp_num) > DPP_PRESENCE_CH_MAX)
				cce_num = DPP_PRESENCE_CH_MAX - i;

			os_memcpy(annouce_info->chan + i, tmp_ch, cce_num *sizeof(tmp_ch[0]));
			i = i+cce_num;
			wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: add bi info chanel list ", annouce_info->chan, i);
		}

	}
#endif

	DBGPRINT(RT_DEBUG_TRACE, DPP_MAP_PREX" gen cce channel len: %d [%d, %d, %d]\n", i,
		dpp->cce_ch.cce_2g.ch_num, dpp->cce_ch.cce_5gl.ch_num, dpp->cce_ch.cce_5gh.ch_num);
	// get CCE chan list
	if(dpp->default_2g_iface && dpp->cce_ch.cce_2g.ch_num) {
		cce_num = dpp->cce_ch.cce_2g.ch_num;
		if((i + dpp->cce_ch.cce_2g.ch_num) > DPP_PRESENCE_CH_MAX)
			cce_num = DPP_PRESENCE_CH_MAX - i;

		os_memcpy(annouce_info->chan + i, dpp->cce_ch.cce_2g.chan, cce_num * sizeof(dpp->cce_ch.cce_2g.chan[0]));
		i = i+cce_num;
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: add 2g cce info chanel list ", annouce_info->chan, i);
	}

	if(dpp->default_5gl_iface && dpp->cce_ch.cce_5gl.ch_num) {
		cce_num = dpp->cce_ch.cce_5gl.ch_num;
		if((i + dpp->cce_ch.cce_5gl.ch_num) > DPP_PRESENCE_CH_MAX)
			cce_num = DPP_PRESENCE_CH_MAX - i;

		os_memcpy(annouce_info->chan + i, dpp->cce_ch.cce_5gl.chan, cce_num * sizeof(bi->chan[0]));
		i = i+cce_num;
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: add 5gl cce info chanel list ", annouce_info->chan, i);
	}

	if(dpp->default_5gl_iface && dpp->cce_ch.cce_5gh.ch_num) {
		cce_num = dpp->cce_ch.cce_5gh.ch_num;
		if((i + dpp->cce_ch.cce_5gh.ch_num) > DPP_PRESENCE_CH_MAX)
			cce_num = DPP_PRESENCE_CH_MAX - i;

		os_memcpy(annouce_info->chan + i, dpp->cce_ch.cce_5gh.chan, cce_num * sizeof(bi->chan[0]));
		i = i+cce_num;
		wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: add 5gh cce info chanel list ", annouce_info->chan, i);
	}

#ifdef MAP_R3
	/* Adding the user configured ch list in presence announcement */
	if (dpp->chirp_add_ch_num > 0)
	{
		u8 ch_add = 0;
		for(ch_add = 0; ch_add < (unsigned int)dpp->chirp_add_ch_num; ch_add++) {
			annouce_info->chan[i++] = (unsigned int)dpp->chirp_list_add[ch_add];
		}
	}
#endif /* MAP_R3 */

	//DBGPRINT(RT_DEBUG_TRACE," gen all channel len[%d]\n", i);
	wpa_dump(MSG_DEBUG, DPP_MAP_PREX "DPP: dup before chanel list ", annouce_info->chan, i);

	//remove dup
	real_num = dpp_handle_presence_channel_dup ((int* )annouce_info->chan, i);

	wpa_dump(MSG_INFO1, DPP_MAP_PREX "DPP: dup after chanel list ", annouce_info->chan, real_num);

#ifdef MAP_R3
	/* Deleting the user configured ch list from presence announcement */
	if (dpp->chirp_del_ch_num > 0) {
		u8 ch_del = 0;
		for(ch_del = 0; ch_del < (unsigned int)dpp->chirp_add_ch_num; ch_del++) {
			real_num = wapp_dpp_ch_list_chirp_del((int* )annouce_info->chan, 
					(unsigned int)dpp->chirp_list_add[ch_del], real_num);
		}
	}
	wpa_dump(MSG_INFO1, DPP_MAP_PREX "DPP: after deleting user configure channel list ", annouce_info->chan, real_num);
#endif /* MAP_R3 */

	annouce_info->ch_num = real_num;
	annouce_info->cur_an_chan_id = 0;
	annouce_info->an_retry = 0;
	DBGPRINT(RT_DEBUG_OFF, DPP_MAP_PREX" gen channel list: total num %d \n", real_num);
	return annouce_info->ch_num;
}
#endif


int dpp_chirp_key_hash(struct dpp_bootstrap_info *bi)
{
	int res = 0;;
	struct wpabuf *der, *prefix = NULL;

	const u8 *addr[2];
	size_t len[2];
	const char chirp[] = "chirp";

	if (!bi) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: input bi is null \n");
		return 0;
	}
	prefix = wpabuf_alloc_copy(chirp, os_strlen(chirp));
	if (!prefix) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Prefix alloc failure\n");
		return -1;
	}
	der = dpp_bootstrap_key_der(bi->pubkey);
	if (!der) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: der alloc failure\n");
		wpabuf_free(prefix);
		return -1;
	}

	addr[0] = wpabuf_head(prefix);
	len[0] = wpabuf_len(prefix);
	addr[1] = wpabuf_head(der);
	len[1] = wpabuf_len(der);

	res = sha256_vector(2, addr, len, bi->chirp_hash);
	if (res < 0)
		wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: Failed to hash public key");
	else
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: chirp key hash", bi->chirp_hash,
			    SHA256_MAC_LEN);
	wpabuf_free(der);
	wpabuf_free(prefix);

	return res;
}

struct dpp_bootstrap_info * dpp_chirp_find_pair(struct dpp_global *dpp, const u8 *r_bootstrap)
{
	struct dpp_bootstrap_info *bi = NULL;
	struct dpp_bootstrap_info *peer_bi = NULL;

	if (!dpp || !r_bootstrap)
		return peer_bi;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: presence chirp key hash", r_bootstrap,
			    SHA256_MAC_LEN);
	dl_list_for_each(bi, &dpp->bootstrap, struct dpp_bootstrap_info, list) {
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: each chirp key hash", bi->chirp_hash,
			    SHA256_MAC_LEN);
		if (!peer_bi && !bi->own) {
			if (os_memcmp(bi->chirp_hash, r_bootstrap,  SHA256_MAC_LEN) == 0) {
				wpa_printf(MSG_INFO1, DPP_MAP_PREX
					   "DPP: Found matching peer presence chirp key success");
				peer_bi = bi;
				break;
			}
		}
	}

	return peer_bi; 
}

static int valid_channel_list(const char *val)
{
	while (*val) {
		if (!((*val >= '0' && *val <= '9') ||
		      *val == '/' || *val == ','))
			return 0;
		val++;
	}

	return 1;
}

enum dpp_status_error dpp_conn_status_result_rx(struct dpp_authentication *auth,
						const u8 *hdr, const u8 *attr_start, size_t attr_len,
						u8 *ssid,  size_t *ssid_len, char **channel_list)
{
	const u8 *wrapped_data, *status, *e_nonce;
	u16 wrapped_data_len, status_len, e_nonce_len;
	const u8 *addr[2];
	size_t len[2];
	u8 *unwrapped = NULL;
	size_t unwrapped_len = 0;
	enum dpp_status_error ret = 256;
	struct json_token *root = NULL, *token;
	struct wpabuf *ssid64;

	*ssid_len = 0;
	*channel_list = NULL;

	if(!auth || !hdr || !attr_start)
		return ret;

	wrapped_data = dpp_get_attr(attr_start, attr_len, DPP_ATTR_WRAPPED_DATA,
				    &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		dpp_auth_fail(auth,
			      "Missing or invalid required Wrapped Data attribute");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Wrapped data",
		    wrapped_data, wrapped_data_len);

	attr_len = wrapped_data - 4 - attr_start;

	addr[0] = hdr;
	len[0] = DPP_HDR_LEN;
	addr[1] = attr_start;
	len[1] = attr_len;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped_data, wrapped_data_len);
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped = os_malloc(unwrapped_len);
	if (!unwrapped)
		goto fail;
	if (aes_siv_decrypt(auth->ke, auth->curve->hash_len,
			    wrapped_data, wrapped_data_len,
			    2, addr, len, unwrapped) < 0) {
		dpp_auth_fail(auth, "AES-SIV decryption failed");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext",
		    unwrapped, unwrapped_len);

	if (dpp_check_attrs(unwrapped, unwrapped_len) < 0) {
		dpp_auth_fail(auth, "Invalid attribute in unwrapped data");
		goto fail;
	}

	e_nonce = dpp_get_attr(unwrapped, unwrapped_len,
			       DPP_ATTR_ENROLLEE_NONCE,
			       &e_nonce_len);
	if (!e_nonce || e_nonce_len != auth->curve->nonce_len) {
		dpp_auth_fail(auth,
			      "Missing or invalid Enrollee Nonce attribute");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Enrollee Nonce", e_nonce, e_nonce_len);
	if (os_memcmp(e_nonce, auth->e_nonce, e_nonce_len) != 0) {
		dpp_auth_fail(auth, "Enrollee Nonce mismatch");
		wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: Expected Enrollee Nonce",
			    auth->e_nonce, e_nonce_len);
		goto fail;
	}

	status = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_CONN_STATUS,
			      &status_len);
	if (!status) {
		dpp_auth_fail(auth,
			      "Missing required DPP Connection Status attribute");
		goto fail;
	}
	wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: connStatus JSON",
			  status, status_len);

	root = json_parse((const char *) status, status_len);
	if (!root) {
		dpp_auth_fail(auth, "Could not parse connStatus");
		goto fail;
	}

	ssid64 = json_get_member_base64url(root, "ssid64");
	if (ssid64 && wpabuf_len(ssid64) <= SSID_MAX_LEN) {
		*ssid_len = wpabuf_len(ssid64);
		os_memcpy(ssid, wpabuf_head(ssid64), *ssid_len);
	}
	wpabuf_free(ssid64);

	token = json_get_member(root, "channelList");
	if (token && token->type == JSON_STRING &&
	    valid_channel_list(token->string))
		*channel_list = os_strdup(token->string);

	token = json_get_member(root, "result");
	if (!token || token->type != JSON_NUMBER) {
		dpp_auth_fail(auth, "No connStatus - result");
		goto fail;
	}
	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: connStatus result %d", token->number);
	ret = token->number;

fail:
	json_free(root);
	bin_clear_free(unwrapped, unwrapped_len);
	return ret;
}

struct wpabuf * dpp_build_conn_status_result(struct dpp_authentication *auth,
					     enum dpp_status_error result,
					     const u8 *ssid, size_t ssid_len,
					     const char *channel_list)
{
	struct wpabuf *msg = NULL, *clear = NULL, *json;
	size_t nonce_len, clear_len, attr_len;
	const u8 *addr[2];
	size_t len[2];
	u8 *wrapped;

	json = wpabuf_alloc(1000);
	if (!json)
		return NULL;
	json_start_object(json, NULL);
	json_add_int(json, "result", result);
	if (ssid) {
		json_value_sep(json);
		if (json_add_base64url(json, "ssid64", ssid, ssid_len) < 0)
			goto fail;
	}

	if (channel_list) {
		json_value_sep(json);
		json_add_string(json, "channelList", channel_list);
	}

	json_end_object(json);
	wpa_hexdump_ascii(MSG_INFO1, DPP_MAP_PREX "DPP: connStatus JSON",
			  wpabuf_head(json), wpabuf_len(json));

	if(auth->curve) {
		nonce_len = auth->curve->nonce_len;
	} else {
		nonce_len = 0;
	}

	clear_len = 5 + 4 + nonce_len + 4 + wpabuf_len(json);
	attr_len = 4 + clear_len + AES_BLOCK_SIZE;
	clear = wpabuf_alloc(clear_len);
	msg = dpp_alloc_msg(DPP_PA_CONNECTION_STATUS_RESULT, attr_len);
	if (!clear || !msg)
		goto fail;

	/* E-nonce */
	wpabuf_put_le16(clear, DPP_ATTR_ENROLLEE_NONCE);
	wpabuf_put_le16(clear, nonce_len);
	wpabuf_put_data(clear, auth->e_nonce, nonce_len);

	/* DPP Connection Status */
	wpabuf_put_le16(clear, DPP_ATTR_CONN_STATUS);
	wpabuf_put_le16(clear, wpabuf_len(json));
	wpabuf_put_buf(clear, json);

	/* OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = wpabuf_head_u8(msg) + 2;
	len[0] = 3 + 1 + 1 + 1;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[0]", addr[0], len[0]);

	/* Attributes before Wrapped Data (none) */
	addr[1] = wpabuf_put(msg, 0);
	len[1] = 0;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DDP: AES-SIV AD[1]", addr[1], len[1]);

	/* Wrapped Data */
	wpabuf_put_le16(msg, DPP_ATTR_WRAPPED_DATA);
	wpabuf_put_le16(msg, wpabuf_len(clear) + AES_BLOCK_SIZE);
	wrapped = wpabuf_put(msg, wpabuf_len(clear) + AES_BLOCK_SIZE);

	wpa_hexdump_buf(MSG_DEBUG, "DPP: AES-SIV cleartext", clear);
	if (auth->curve && aes_siv_encrypt(auth->ke, auth->curve->hash_len,
			    wpabuf_head(clear), wpabuf_len(clear),
			    2, addr, len, wrapped) < 0)
		goto fail;

	wpa_hexdump_buf(MSG_INFO1, "DPP: Connection Status Result attributes",
			msg);
	wpabuf_free(json);
	wpabuf_free(clear);
	return msg;
fail:
	wpabuf_free(json);
	wpabuf_free(clear);
	wpabuf_free(msg);
	return NULL;
}
#endif /* DPP_R2_SUPPORT */
struct wpabuf * dpp_build_conn_status(enum dpp_status_error result,
				      const u8 *ssid, size_t ssid_len,
				      const char *channel_list)
{
	struct wpabuf *json;

	json = wpabuf_alloc(1000);
	if (!json)
		return NULL;
	json_start_object(json, NULL);
	json_add_int(json, "result", result);
	if (ssid) {
		json_value_sep(json);
		if (json_add_base64url(json, "ssid64", ssid, ssid_len) < 0) {
			wpabuf_free(json);
			return NULL;
		}
	}
	if (channel_list) {
		json_value_sep(json);
		json_add_string(json, "channelList", channel_list);
	}
	json_end_object(json);
	wpa_hexdump_ascii(MSG_DEBUG, "DPP: connStatus JSON",
			  wpabuf_head(json), wpabuf_len(json));

	return json;
}



#ifdef DPP_R2_RECONFIG


struct dpp_authentication *
dpp_alloc_auth(struct dpp_global *dpp, void *msg_ctx)
{
	struct dpp_authentication *auth;

	auth = os_zalloc(sizeof(*auth));
	if (!auth)
		return NULL;
	auth->global = dpp;
	auth->msg_ctx = msg_ctx;
	auth->conf_resp_status = 255;
	return auth;
}

static char *
dpp_build_conn_signature(struct dpp_configurator *conf,
			 const char *signed1, size_t signed1_len,
			 const char *signed2, size_t signed2_len,
			 size_t *signed3_len)
{
	const struct dpp_curve_params *curve;
	char *signed3 = NULL;
	unsigned char *signature = NULL;
	const unsigned char *p;
	size_t signature_len;
	EVP_MD_CTX *md_ctx = NULL;
	ECDSA_SIG *sig = NULL;
	char *dot = ".";
	const EVP_MD *sign_md;
	const BIGNUM *r, *s;

	curve = conf->curve;
	if (curve->hash_len == SHA256_MAC_LEN) {
		sign_md = EVP_sha256();
	} else if (curve->hash_len == SHA384_MAC_LEN) {
		sign_md = EVP_sha384();
	} else if (curve->hash_len == SHA512_MAC_LEN) {
		sign_md = EVP_sha512();
	} else {
		wpa_printf(MSG_DEBUG, "DPP: Unknown signature algorithm");
		goto fail;
	}

	md_ctx = EVP_MD_CTX_create();
	if (!md_ctx)
		goto fail;

	ERR_clear_error();
	if (EVP_DigestSignInit(md_ctx, NULL, sign_md, NULL, conf->csign) != 1) {
		wpa_printf(MSG_DEBUG, "DPP: EVP_DigestSignInit failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	if (EVP_DigestSignUpdate(md_ctx, signed1, signed1_len) != 1 ||
	    EVP_DigestSignUpdate(md_ctx, dot, 1) != 1 ||
	    EVP_DigestSignUpdate(md_ctx, signed2, signed2_len) != 1) {
		wpa_printf(MSG_DEBUG, "DPP: EVP_DigestSignUpdate failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	if (EVP_DigestSignFinal(md_ctx, NULL, &signature_len) != 1) {
		wpa_printf(MSG_DEBUG, "DPP: EVP_DigestSignFinal failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	signature = os_malloc(signature_len);
	if (!signature)
		goto fail;
	if (EVP_DigestSignFinal(md_ctx, signature, &signature_len) != 1) {
		wpa_printf(MSG_DEBUG, "DPP: EVP_DigestSignFinal failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, "DPP: signedConnector ECDSA signature (DER)",
		    signature, signature_len);
	/* Convert to raw coordinates r,s */
	p = signature;
	sig = d2i_ECDSA_SIG(NULL, &p, signature_len);
	if (!sig)
		goto fail;
	ECDSA_SIG_get0(sig, &r, &s);
	if (dpp_bn2bin_pad(r, signature, curve->prime_len) < 0 ||
	    dpp_bn2bin_pad(s, signature + curve->prime_len,
			   curve->prime_len) < 0)
		goto fail;
	signature_len = 2 * curve->prime_len;
	wpa_hexdump(MSG_DEBUG, "DPP: signedConnector ECDSA signature (raw r,s)",
		    signature, signature_len);
	signed3 = base64_url_encode(signature, signature_len, signed3_len, 0);
fail:
	EVP_MD_CTX_destroy(md_ctx);
	ECDSA_SIG_free(sig);
	os_free(signature);
	return signed3;
}


static char *
dpp_build_jws_prot_hdr(struct dpp_configurator *conf, size_t *signed1_len)
{
	struct wpabuf *jws_prot_hdr;
	char *signed1;

	jws_prot_hdr = wpabuf_alloc(100);
	if (!jws_prot_hdr)
		return NULL;
	json_start_object(jws_prot_hdr, NULL);
	json_add_string(jws_prot_hdr, "typ", "dppCon");
	json_value_sep(jws_prot_hdr);
	json_add_string(jws_prot_hdr, "kid", conf->kid);
	json_value_sep(jws_prot_hdr);
	json_add_string(jws_prot_hdr, "alg", conf->curve->jws_alg);
	json_end_object(jws_prot_hdr);
	signed1 = base64_url_encode(wpabuf_head(jws_prot_hdr),
				    wpabuf_len(jws_prot_hdr),
				    signed1_len, 0);
	wpabuf_free(jws_prot_hdr);
	return signed1;
}

char * dpp_sign_connector(struct dpp_configurator *conf,
			  const struct wpabuf *dppcon)
{
	char *signed1 = NULL, *signed2 = NULL, *signed3 = NULL;
	char *signed_conn = NULL, *pos;
	size_t signed1_len, signed2_len, signed3_len;

	signed1 = dpp_build_jws_prot_hdr(conf, &signed1_len);
	signed2 = base64_url_encode(wpabuf_head(dppcon), wpabuf_len(dppcon),
				    &signed2_len, 0);
	if (!signed1 || !signed2)
		goto fail;

	signed3 = dpp_build_conn_signature(conf, signed1, signed1_len,
					   signed2, signed2_len, &signed3_len);
	if (!signed3)
		goto fail;

	signed_conn = os_malloc(signed1_len + signed2_len + signed3_len + 3);
	if (!signed_conn)
		goto fail;
	pos = signed_conn;
	os_memcpy(pos, signed1, signed1_len);
	pos += signed1_len;
	*pos++ = '.';
	os_memcpy(pos, signed2, signed2_len);
	pos += signed2_len;
	*pos++ = '.';
	os_memcpy(pos, signed3, signed3_len);
	pos += signed3_len;
	*pos = '\0';

fail:
	os_free(signed1);
	os_free(signed2);
	os_free(signed3);
	return signed_conn;
}
#if 0
enum dpp_status_error
dpp_process_signed_connector(struct dpp_signed_connector_info *info,
			     EVP_PKEY *csign_pub, const char *connector)
{
	enum dpp_status_error ret = 255;
	const char *pos, *end, *signed_start, *signed_end;
	struct wpabuf *kid = NULL;
	unsigned char *prot_hdr = NULL, *signature = NULL;
	size_t prot_hdr_len = 0, signature_len = 0;
	const EVP_MD *sign_md = NULL;
	unsigned char *der = NULL;
	int der_len;
	int res;
	EVP_MD_CTX *md_ctx = NULL;
	ECDSA_SIG *sig = NULL;
	BIGNUM *r = NULL, *s = NULL;
	const struct dpp_curve_params *curve;
	const EC_KEY *eckey;
	const EC_GROUP *group;
	int nid;

	eckey = EVP_PKEY_get0_EC_KEY(csign_pub);
	if (!eckey)
		goto fail;
	group = EC_KEY_get0_group(eckey);
	if (!group)
		goto fail;
	nid = EC_GROUP_get_curve_name(group);
	curve = dpp_get_curve_nid(nid);
	if (!curve)
		goto fail;
	wpa_printf(MSG_DEBUG, "DPP: C-sign-key group: %s", curve->jwk_crv);
	os_memset(info, 0, sizeof(*info));

	signed_start = pos = connector;
	end = os_strchr(pos, '.');
	if (!end) {
		wpa_printf(MSG_DEBUG, "DPP: Missing dot(1) in signedConnector");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}
	prot_hdr = base64_url_decode(pos, end - pos, &prot_hdr_len);
	if (!prot_hdr) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Failed to base64url decode signedConnector JWS Protected Header");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}
	wpa_hexdump_ascii(MSG_DEBUG,
			  "DPP: signedConnector - JWS Protected Header",
			  prot_hdr, prot_hdr_len);
	kid = dpp_parse_jws_prot_hdr(curve, prot_hdr, prot_hdr_len, &sign_md);
	if (!kid) {
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}
	if (wpabuf_len(kid) != SHA256_MAC_LEN) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Unexpected signedConnector JWS Protected Header kid length: %u (expected %u)",
			   (unsigned int) wpabuf_len(kid), SHA256_MAC_LEN);
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}

	pos = end + 1;
	end = os_strchr(pos, '.');
	if (!end) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Missing dot(2) in signedConnector");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}
	signed_end = end - 1;
	info->payload = base64_url_decode(pos, end - pos, &info->payload_len);
	if (!info->payload) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Failed to base64url decode signedConnector JWS Payload");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}
	wpa_hexdump_ascii(MSG_DEBUG,
			  "DPP: signedConnector - JWS Payload",
			  info->payload, info->payload_len);
	pos = end + 1;
	signature = base64_url_decode(pos, os_strlen(pos), &signature_len);
	if (!signature) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Failed to base64url decode signedConnector signature");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
		}
	wpa_hexdump(MSG_DEBUG, "DPP: signedConnector - signature",
		    signature, signature_len);

	if (dpp_check_pubkey_match(csign_pub, kid) < 0) {
		ret = DPP_STATUS_NO_MATCH;
		goto fail;
	}

	if (signature_len & 0x01) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Unexpected signedConnector signature length (%d)",
			   (int) signature_len);
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}

	/* JWS Signature encodes the signature (r,s) as two octet strings. Need
	 * to convert that to DER encoded ECDSA_SIG for OpenSSL EVP routines. */
	r = BN_bin2bn(signature, signature_len / 2, NULL);
	s = BN_bin2bn(signature + signature_len / 2, signature_len / 2, NULL);
	sig = ECDSA_SIG_new();
	if (!r || !s || !sig || ECDSA_SIG_set0(sig, r, s) != 1)
		goto fail;
	r = NULL;
	s = NULL;

	der_len = i2d_ECDSA_SIG(sig, &der);
	if (der_len <= 0) {
		wpa_printf(MSG_DEBUG, "DPP: Could not DER encode signature");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, "DPP: DER encoded signature", der, der_len);
	md_ctx = EVP_MD_CTX_create();
	if (!md_ctx)
		goto fail;

	ERR_clear_error();
	if (EVP_DigestVerifyInit(md_ctx, NULL, sign_md, NULL, csign_pub) != 1) {
		wpa_printf(MSG_DEBUG, "DPP: EVP_DigestVerifyInit failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	if (EVP_DigestVerifyUpdate(md_ctx, signed_start,
				   signed_end - signed_start + 1) != 1) {
		wpa_printf(MSG_DEBUG, "DPP: EVP_DigestVerifyUpdate failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	res = EVP_DigestVerifyFinal(md_ctx, der, der_len);
	if (res != 1) {
		wpa_printf(MSG_DEBUG,
			   "DPP: EVP_DigestVerifyFinal failed (res=%d): %s",
			   res, ERR_error_string(ERR_get_error(), NULL));
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}

	ret = DPP_STATUS_OK;
fail:
	EVP_MD_CTX_destroy(md_ctx);
	os_free(prot_hdr);
	wpabuf_free(kid);
	os_free(signature);
	ECDSA_SIG_free(sig);
	BN_free(r);
	BN_free(s);
	OPENSSL_free(der);
	return ret;
}
#endif

enum dpp_status_error
dpp_check_signed_connector(struct dpp_signed_connector_info *info,
			   const u8 *csign_key, size_t csign_key_len,
			   const u8 *peer_connector, size_t peer_connector_len)
{
	const unsigned char *p;
	EVP_PKEY *csign = NULL;
	char *signed_connector = NULL;
	enum dpp_status_error res = DPP_STATUS_INVALID_CONNECTOR;

	p = csign_key;
	csign = d2i_PUBKEY(NULL, &p, csign_key_len);
	if (!csign) {
		wpa_printf(MSG_ERROR,
			   "DPP: Failed to parse local C-sign-key information");
		goto fail;
	}

	wpa_hexdump_ascii(MSG_DEBUG, "DPP: Peer signedConnector",
			  peer_connector, peer_connector_len);
	signed_connector = os_malloc(peer_connector_len + 1);
	if (!signed_connector)
		goto fail;
	os_memcpy(signed_connector, peer_connector, peer_connector_len);
	signed_connector[peer_connector_len] = '\0';
	res = dpp_process_signed_connector(info, csign, signed_connector);
fail:
	os_free(signed_connector);
	EVP_PKEY_free(csign);
	return res;
}


#if 0
void dpp_debug_print_key(const char *title, EVP_PKEY *key)
{
	EC_KEY *eckey;
	BIO *out;
	size_t rlen;
	char *txt;
	int res;
	unsigned char *der = NULL;
	int der_len;
	const EC_GROUP *group;
	const EC_POINT *point;

	out = BIO_new(BIO_s_mem());
	if (!out)
		return;

	EVP_PKEY_print_private(out, key, 0, NULL);
	rlen = BIO_ctrl_pending(out);
	txt = os_malloc(rlen + 1);
	if (txt) {
		res = BIO_read(out, txt, rlen);
		if (res > 0) {
			txt[res] = '\0';
			wpa_printf(MSG_DEBUG, "%s: %s", title, txt);
		}
		os_free(txt);
	}
	BIO_free(out);

	eckey = EVP_PKEY_get1_EC_KEY(key);
	if (!eckey)
		return;

	group = EC_KEY_get0_group(eckey);
	point = EC_KEY_get0_public_key(eckey);
	if (group && point)
		dpp_debug_print_point(title, group, point);

	der_len = i2d_ECPrivateKey(eckey, &der);
	if (der_len > 0)
		wpa_hexdump_key(MSG_DEBUG, "DPP: ECPrivateKey", der, der_len);
	OPENSSL_free(der);
	if (der_len <= 0) {
		der = NULL;
		der_len = i2d_EC_PUBKEY(eckey, &der);
		if (der_len > 0)
			wpa_hexdump(MSG_DEBUG, "DPP: EC_PUBKEY", der, der_len);
		OPENSSL_free(der);
	}

	EC_KEY_free(eckey);
}
#endif


const struct dpp_curve_params * dpp_get_curve_ike_group(u16 group)
{
	int i;

	for (i = 0; dpp_curves[i].name; i++) {
		if (dpp_curves[i].ike_group == group)
			return &dpp_curves[i];
	}
	return NULL;
}


#if 0
struct wpabuf * dpp_build_conn_status(struct dpp_authentication *auth,
					     enum dpp_status_error result,
					     const u8 *ssid, size_t ssid_len,
					     const char *channel_list)
{
	struct wpabuf *json = NULL;

	if(!auth || !ssid ||! ssid_len)
		return NULL;

	json = wpabuf_alloc(1000);
	if (!json)
		return NULL;
	json_start_object(json, NULL);
#ifdef MAP_R3_RECONFIG
	result = DPP_STATUS_AUTH_FAILURE;
#endif
	json_add_int(json, "result", result);
	if (ssid) {
		json_value_sep(json);
		if (json_add_base64url(json, "ssid64", ssid, ssid_len) < 0) {
			wpabuf_free(json);
			return NULL;
		}
	}

	if (channel_list) {
		json_value_sep(json);
		json_add_string(json, "channelList", channel_list);
	}

	json_end_object(json);
	wpa_hexdump_ascii(MSG_INFO1, DPP_MAP_PREX "DPP: connStatus JSON for reconfig",
			  wpabuf_head(json), wpabuf_len(json));
	return json;
}
#endif
//#ifdef RECONFIG_OLD
#if 1
static void dpp_build_attr_r_csign_key_hash(struct wpabuf *msg,
						const u8 *hash)
{
	if (hash) {
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX "DPP: R-csign Key Hash");
		wpabuf_put_le16(msg, DPP_ATTR_C_SIGN_KEY_HASH);
		wpabuf_put_le16(msg, SHA256_MAC_LEN);
		wpabuf_put_data(msg, hash, SHA256_MAC_LEN);
	}
}
#endif
#ifdef MAP_R3_RECONFIG
EVP_PKEY * dpp_build_csign_hash(const u8 *csign_key, size_t csign_key_len)
{
	EVP_PKEY *csign = NULL;
	const unsigned char *p = NULL;
	
	if (!csign_key || !csign_key_len)
		return NULL;

	p = csign_key;
	csign = d2i_PUBKEY(NULL, &p, csign_key_len);
	if (!csign) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to parse local C-sign-key information in reconfig annouce frame");
		goto fail;
	}

	return csign;
fail:
	return NULL;
}

#endif
struct wpabuf * dpp_build_reconfig_annouce_frame(struct dpp_global *dpp, const u8 *csign_key, size_t csign_key_len)
{
	struct wpabuf *msg =  NULL;
	size_t attr_len = 0;
	EVP_PKEY *csign = NULL;
	const unsigned char *p = NULL;
	struct wpabuf *csign_pub = NULL;
	u8 csign_hash[SHA256_MAC_LEN];
	const u8 *addr[1];
	size_t len[1];
	int res = 0;
	
	if (!dpp || !csign_key || !csign_key_len)
		return NULL;
	
	/* Build DPP Authentication Request frame attributes */
	attr_len = 4 + SHA256_MAC_LEN;

	msg = dpp_alloc_msg(DPP_PA_RECONFIG_ANNOUNCEMENT, attr_len);
	if (!msg)
		return NULL;
	p = csign_key;
	csign = d2i_PUBKEY(NULL, &p, csign_key_len);
	if (!csign) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to parse local C-sign-key information in reconfig annouce frame");
		goto fail;
	}

	csign_pub = dpp_get_pubkey_point(csign, 1);
	if (!csign_pub) {
		wpa_printf(MSG_INFO, "DPP: Failed to extract C-sign-key in reconfig annouce frame");
		goto fail;
	}

	/* kid = SHA256(ANSI X9.63 uncompressed C-sign-key) */
	addr[0] = wpabuf_head(csign_pub);
	len[0] = wpabuf_len(csign_pub);
	res = sha256_vector(1, addr, len, csign_hash);
	wpabuf_free(csign_pub);
	if (res < 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to derive kid for C-sign-key in reconfig annouce frame");
		goto fail;
	}
	
	/* responser Bootstrapping Key Hash */
	dpp_build_attr_r_csign_key_hash(msg, csign_hash);
	return msg;
fail:
	os_free(msg);
	return NULL;
}

int dpp_parse_reconfig_annouce(struct wifi_app *wapp, const u8 *buf, size_t buf_len
#ifdef MAP_R3_RECONFIG
	, struct wapp_dev *wdev
#endif
)
{
	const u8 *r_csign_key  = NULL;
	u16 r_csignkey_len = 0;
	struct wpabuf *r_hash= NULL;
#ifdef MAP_R3_RECONFIG
	struct dpp_configurator *conf = NULL;
#endif
	if (!wapp ||!wapp->dpp || !buf || !buf_len)
		return -1;
#ifdef MAP_R3_RECONFIG
	if (wapp->dpp->dpp_allowed_roles != DPP_CAPAB_PROXYAGENT) {
		conf = dpp_configurator_get_id(wapp->dpp, 1);
		if(!conf)
			return -1;
	}
#endif
	/* get c sign key */ 
	r_csign_key = dpp_get_attr(buf, buf_len, DPP_ATTR_C_SIGN_KEY_HASH,
				   &r_csignkey_len);
	if (!r_csign_key || r_csignkey_len != SHA256_MAC_LEN) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX 
			"Missing or invalid required Responder c sign  Key Hash attribute");
		return -1;
	}

	/* Try to compare configurator c sign key hash matches based on the
	 * received hash values */
	r_hash = wpabuf_alloc(r_csignkey_len);
	if(!r_hash)
		return -1;
	//wpabuf_put_le16(r_hash, SHA256_MAC_LEN);
	wpabuf_put_data(r_hash, r_csign_key, SHA256_MAC_LEN);
#ifdef MAP_R3_RECONFIG
	if (wapp->dpp->dpp_allowed_roles == DPP_CAPAB_PROXYAGENT) {
		EVP_PKEY *csign=NULL;

		csign = dpp_build_csign_hash(wapp->dpp_csign, wapp->dpp_csign_len);
		
		if (dpp_check_pubkey_match(csign, r_hash) < 0) {
			wpabuf_free(r_hash);
			wpa_printf(MSG_ERROR, DPP_MAP_PREX 
				"dpp_check_pubkey_match c sign  Key Hash fail");
			return -1;
		}

	}
	else {
		if (dpp_check_pubkey_match(conf->csign, r_hash) < 0)  {
			wpabuf_free(r_hash);
			wpa_printf(MSG_ERROR, DPP_MAP_PREX 
				"dpp_check_pubkey_match c sign  Key Hash fail");
			return -1;
		}
	}
#else
#ifdef MAP_R3_RECONFIG	
		if (dpp_check_pubkey_match(conf->csign, r_hash) < 0)  
		{
			wpabuf_free(r_hash);
			wpa_printf(MSG_ERROR, DPP_MAP_PREX 
				"dpp_check_pubkey_match c sign  Key Hash fail");
			return -1;
		}
#endif
#endif
	wpabuf_free(r_hash);
	return 0;
}
#ifdef RECONFIG_OLD
int dpp_gen_reconfig_annouce_channel_list(struct dpp_global *dpp, struct dpp_annouce_info *annouce_info)
{
	unsigned int  i = 0, cce_num = 0, real_num = 0;
	struct dpp_bootstrap_info *bi = NULL;

	if (!dpp || !annouce_info) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "failed to find dpp, not initialized\n");
		return 0;
	}
	// reset all
	os_memset(annouce_info->chan, 0, DPP_PRESENCE_CH_MAX-1);
	annouce_info->ch_num = 0;

	/* prefer chan list */
	if(dpp->default_2g_iface) {
		annouce_info->chan[i] = 6;
		i++;
	}

	if(dpp->default_5gl_iface) {
		annouce_info->chan[i] = 44;
		i++;
	}

	if(dpp->default_5gh_iface) {
		annouce_info->chan[i] = 149;
		i++;
	}

	// get scan channel list
	if (dpp->scan_ch.ch_num) { 
		wpa_printf(MSG_ERROR, DPP_MAP_PREX " scan channel len: %d [%d] \n", i, dpp->scan_ch.ch_num);
		cce_num = dpp->scan_ch.ch_num;
		if((i + dpp->scan_ch.ch_num) > DPP_PRESENCE_CH_MAX)
			cce_num = DPP_PRESENCE_CH_MAX - i;

		os_memcpy(annouce_info->chan + i, dpp->scan_ch.chan, cce_num *sizeof(dpp->scan_ch.chan[0]));
		i = i+cce_num;
	}

	wpa_printf(MSG_ERROR, DPP_MAP_PREX " gen cce channel len: %d [%d, %d, %d]\n", i,
		dpp->cce_ch.cce_2g.ch_num, dpp->cce_ch.cce_5gl.ch_num, dpp->cce_ch.cce_5gh.ch_num);
	// get CCE chan list
	if(dpp->default_2g_iface && dpp->cce_ch.cce_2g.ch_num) {
		cce_num = dpp->cce_ch.cce_2g.ch_num;
		if((i + dpp->cce_ch.cce_2g.ch_num) > DPP_PRESENCE_CH_MAX)
			cce_num = DPP_PRESENCE_CH_MAX - i;

		os_memcpy(annouce_info->chan + i, dpp->cce_ch.cce_2g.chan, cce_num * sizeof(dpp->cce_ch.cce_2g.chan[0]));
		i = i+cce_num;
	}

	if(dpp->default_5gl_iface && dpp->cce_ch.cce_5gl.ch_num) {
		cce_num = dpp->cce_ch.cce_5gl.ch_num;
		if((i + dpp->cce_ch.cce_5gl.ch_num) > DPP_PRESENCE_CH_MAX)
			cce_num = DPP_PRESENCE_CH_MAX - i;

		os_memcpy(annouce_info->chan + i, dpp->cce_ch.cce_5gl.chan, cce_num * sizeof(bi->chan[0]));
		i = i+cce_num;
	}

	if(dpp->default_5gl_iface && dpp->cce_ch.cce_5gh.ch_num) {
		cce_num = dpp->cce_ch.cce_5gh.ch_num;
		if((i + dpp->cce_ch.cce_5gh.ch_num) > DPP_PRESENCE_CH_MAX)
			cce_num = DPP_PRESENCE_CH_MAX - i;

		os_memcpy(annouce_info->chan + i, dpp->cce_ch.cce_5gh.chan, cce_num * sizeof(bi->chan[0]));
		i = i+cce_num;
	}

	wpa_printf(MSG_ERROR, DPP_MAP_PREX " gen all channel len[%d]\n", i);
	//remove dup
	real_num = dpp_handle_presence_channel_dup ((int* )annouce_info->chan, i);

	annouce_info->ch_num = real_num;
	annouce_info->cur_an_chan_id = 0;
	annouce_info->an_retry = 0;
	wpa_printf(MSG_ERROR, DPP_MAP_PREX " gen channel list: total num %d \n", real_num);
	return annouce_info->ch_num;
}

struct dpp_authentication * dpp_reconfig_auth_init(void *msg_ctx,
					  struct wapp_dev *wdev,
					  struct dpp_bootstrap_info *peer_bi,
					  struct dpp_bootstrap_info *own_bi,
					  u8 dpp_allowed_roles,
					  unsigned int neg_chan)
{
	struct dpp_authentication *auth;
	size_t nonce_len;
	auth = os_zalloc(sizeof(*auth));
	if (!auth)
		return NULL;
	os_memset(auth, 0, sizeof(struct dpp_authentication));
	auth->msg_ctx = msg_ctx;
	auth->initiator = 1;
	auth->current_state = DPP_STATE_AUTH_RESP_WAITING;
	auth->allowed_roles = dpp_allowed_roles;
	auth->configurator = !!(dpp_allowed_roles & DPP_CAPAB_CONFIGURATOR);
	auth->peer_bi = peer_bi;
	auth->own_bi = own_bi;
	auth->curve = &dpp_curves[0];
	auth->wdev = wdev;
	auth->own_version = 1;
	auth->peer_version = 1;
#ifdef DPP_R2_MUOBJ
	auth->conf_obj_num = 0;
#endif /* DPP_R2_MUOBJ */
#ifdef DPP_R2_STATUS
	auth->send_conn_status = 0;
	auth->waiting_conn_status_result = 0;
	auth->conn_status_requested = 0;
#endif /* DPP_R2_STATUS */

	nonce_len = auth->curve->nonce_len;
	if (random_get_bytes(auth->i_nonce, nonce_len)) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to generate I-nonce");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: I-nonce", auth->i_nonce, nonce_len);

	auth->own_protocol_key = dpp_gen_keypair(auth->curve);
	if (!auth->own_protocol_key)
		goto fail;

	auth->peer_protocol_key = auth->own_protocol_key;
	return auth;
fail:
	dpp_auth_deinit(auth);
	return NULL;
}
#endif

struct wpabuf * dpp_build_reconf_connetor(struct dpp_authentication *auth)
{
	struct wpabuf *conf_obj[DPP_CONF_OBJ_MAX];
	int count = 0, i = 0;
	struct wpabuf *buf = NULL;

	if (!auth->conf) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: No configurator specified - cannot generate DPP config object");
		return buf;
	}

	dpp_copy_csign(auth, auth->conf->csign);
	dpp_build_conf_obj(auth, INFRA_CONFIGURATOR, 0, &count, conf_obj
#ifdef MAP_R3
		, NULL, NULL, 0, 0
#ifdef MAP_R3_RECONFIG
		, 0, 1
#endif
#endif /* MAP_R3*/
	);

	wpa_printf(MSG_INFO1, DPP_MAP_PREX  "DPP:dpp_build_reconf_connetor count:%d \n ", count);
	if (!count)
		return NULL;
	if (count) {
		for (i = 0; i < count; i++) {
			wpa_hexdump_ascii(MSG_INFO1, DPP_MAP_PREX "DPP: configurationObject JSON",
				  wpabuf_head(conf_obj[i]), wpabuf_len(conf_obj[i]));
			dpp_parse_conf_obj(auth, wpabuf_head(conf_obj[i]),
				 wpabuf_len(conf_obj[i]));
		}
	}
#ifdef DPP_R2_MUOBJ
	if(auth->conf_obj_num) {
		for(i = 0; i< auth->conf_obj_num; i++) {
			if(auth->conf_obj[i].connector && auth->conf_obj[i].akm == DPP_AKM_DPP) {
				buf = wpabuf_alloc(os_strlen(auth->conf_obj[0].connector));
				if(buf) {
					wpabuf_put_str(buf, auth->conf_obj[0].connector);
					wpa_printf(MSG_INFO1, DPP_MAP_PREX  "DPP: Find the connector for reconfig auth req\n");
				}
				break;
			}
		}
	}
#else
	if(auth->connector) {
		buf = wpabuf_alloc(os_strlen(auth->connector));
		if(buf) {
			wpabuf_put_str(buf, auth->connector);
			wpa_printf(MSG_INFO1, DPP_MAP_PREX  "DPP: Find the connector for reconfig auth req  signle obj  \n");
		}
	}
#endif /* DPP_R2_MUOBJ */

	for (i = 0; i < count; i++) {
		wpabuf_free(conf_obj[i]);
	}
	return buf;
}

static size_t dpp_auth_derive_secret_len(EVP_PKEY *own_key, EVP_PKEY *peer_key)
{
	size_t secret_len = 0;
	u8 tx[DPP_MAX_SHARED_SECRET_LEN];
	EVP_PKEY_CTX *ctx = NULL;
	if(!own_key || !peer_key)
		return secret_len;

	ctx = EVP_PKEY_CTX_new(own_key, NULL);
	if (!ctx ||
	    EVP_PKEY_derive_init(ctx) != 1 ||
	    EVP_PKEY_derive_set_peer(ctx, peer_key) != 1 ||
	    EVP_PKEY_derive(ctx, NULL, &secret_len) != 1 ||
	    secret_len > DPP_MAX_SHARED_SECRET_LEN ||
	    EVP_PKEY_derive(ctx, tx, &secret_len) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to derive  shared secret: %s, len:%zu",
			   ERR_error_string(ERR_get_error(), NULL), secret_len);
		secret_len = 0;
	}

	EVP_PKEY_CTX_free(ctx);
	ctx = NULL;
	wpa_printf(MSG_ERROR, DPP_MAP_PREX   "DPP: derive secret_len :%zu\n", secret_len);
	return secret_len;
}

static int dpp_auth_derive_m_I(struct dpp_authentication *auth)
{
	const EC_GROUP *group;
	EC_POINT *M = NULL, *sum = NULL;
	EC_KEY *cI = NULL, *CR = NULL, *PR = NULL;
	const EC_POINT *CR_point = NULL, *PR_point = NULL;
	BN_CTX *bnctx = NULL;
	BIGNUM *mx = NULL;
	const BIGNUM *cI_bn = NULL;
	int ret = -1;

	/* M = cI * (CR + PR) */
	if(!auth || !auth->peer_connector_key || !auth->peer_protocol_key || !auth->own_connector_key)
		return -1;
	bnctx = BN_CTX_new();
	mx = BN_new();
	if (!bnctx || !mx)
		goto fail;

	CR = EVP_PKEY_get1_EC_KEY(auth->peer_connector_key);
	PR = EVP_PKEY_get1_EC_KEY(auth->peer_protocol_key);
	if (!CR || !PR)
		goto fail;

	CR_point = EC_KEY_get0_public_key(CR);
	PR_point = EC_KEY_get0_public_key(PR);
	cI = EVP_PKEY_get1_EC_KEY(auth->own_connector_key);
	if (!cI)
		goto fail;

	group = EC_KEY_get0_group(cI);
	cI_bn = EC_KEY_get0_private_key(cI);
	if (!group || !cI_bn)
		goto fail;

	sum = EC_POINT_new(group);
	M = EC_POINT_new(group);
	if (!sum || !M ||
	    EC_POINT_add(group, sum, CR_point, PR_point, bnctx) != 1 ||
	    EC_POINT_mul(group, M, NULL, sum, cI_bn, bnctx) != 1 ||
	    EC_POINT_get_affine_coordinates_GFp(group, M, mx, NULL,
						bnctx) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "OpenSSL: failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	auth->secret_len = dpp_auth_derive_secret_len(auth->own_connector_key, auth->peer_protocol_key);
	wpa_printf(MSG_INFO1, DPP_MAP_PREX  "DPP: derive secret len: %zu",
			    auth->secret_len);
	if (dpp_bn2bin_pad(mx, auth->Mx, auth->secret_len) < 0)
		goto fail;
	wpa_hexdump_key(MSG_INFO1, DPP_MAP_PREX "DPP: M.x", auth->Mx, auth->secret_len);
	auth->Mx_len = auth->secret_len;
	ret = 0;
fail:
	EC_POINT_clear_free(M);
	EC_POINT_clear_free(sum);
	EC_KEY_free(cI);
	EC_KEY_free(CR);
	EC_KEY_free(PR);
	BN_clear_free(mx);
	BN_CTX_free(bnctx);
	return ret;
}

static int dpp_auth_derive_m_R(struct dpp_authentication *auth)
{
	const EC_GROUP *group = NULL;
	EC_POINT *M = NULL;
	EC_KEY *CI = NULL, *cR = NULL, *pR = NULL;
	const EC_POINT *CI_point = NULL;
	BN_CTX *bnctx = NULL;
	BIGNUM *mx = NULL, *sum = NULL, *q = NULL;
	const BIGNUM *cR_bn = NULL, *pR_bn = NULL;
	int ret = -1;

	if(!auth || !auth->peer_connector_key || !auth->own_connector_key || !auth->own_protocol_key)
		return -1;
	/* M = (cR + pR) * CI */
	bnctx = BN_CTX_new();
	sum = BN_new();
	q = BN_new();
	mx = BN_new();
	if (!bnctx || !sum || !q || !mx)
		goto fail;

	CI = EVP_PKEY_get1_EC_KEY(auth->peer_connector_key);
	if (!CI)
		goto fail;
	CI_point = EC_KEY_get0_public_key(CI);
	group = EC_KEY_get0_group(CI);
	if (!group)
		goto fail;

	cR = EVP_PKEY_get1_EC_KEY(auth->own_connector_key);
	pR = EVP_PKEY_get1_EC_KEY(auth->own_protocol_key);
	if (!cR || !pR )
		goto fail;

	cR_bn = EC_KEY_get0_private_key(cR);
	pR_bn = EC_KEY_get0_private_key(pR);
	if (!cR_bn || !pR_bn)
		goto fail;

	if (EC_GROUP_get_order(group, q, bnctx) != 1 ||
	    BN_mod_add(sum, cR_bn, pR_bn, q, bnctx) != 1)
		goto fail;

	M = EC_POINT_new(group);

	if (!M ||
	    EC_POINT_mul(group, M, NULL, CI_point, sum, bnctx) != 1 ||
	    EC_POINT_get_affine_coordinates_GFp(group, M, mx, NULL,
						bnctx) != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "OpenSSL: failed: %s",
			   ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	auth->secret_len = dpp_auth_derive_secret_len(auth->own_protocol_key, auth->peer_connector_key);
	wpa_printf(MSG_INFO1, DPP_MAP_PREX  "DPP: derive secret len: %zu", auth->secret_len);
	if (dpp_bn2bin_pad(mx, auth->Mx, auth->secret_len) < 0)
		goto fail;
	wpa_hexdump_key(MSG_INFO1, DPP_MAP_PREX "DPP: M.x", auth->Mx, auth->secret_len);
	auth->Mx_len = auth->secret_len;
	ret = 0;
fail:
	EC_POINT_clear_free(M);
	EC_KEY_free(CI);
	EC_KEY_free(cR);
	EC_KEY_free(pR);
	BN_clear_free(mx);
	BN_clear_free(sum);
	BN_free(q);
	BN_CTX_free(bnctx);
	return ret;
}

static int dpp_derive_reconfig_ke(struct dpp_authentication *auth, u8 *ke,
			 unsigned int hash_len)
{
	size_t nonce_len;
	u8 nonces[DPP_MAX_NONCE_LEN];
	const char *info_ke = "dpp reconfig key";
	u8 prk[DPP_MAX_HASH_LEN];
	int res;
	const u8 *addr[1];
	size_t len[1];
	size_t num_elem = 0;

	if (!auth->Mx_len) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Mx not available - cannot derive ke");
		return -1;
	}

	/* ke = HKDF(I-nonce, "dpp reconfig key", M.x) */
	/* HKDF-Extract(I-nonce, M.x) */
	nonce_len = auth->curve->nonce_len;
	os_memcpy(nonces, auth->i_nonce, nonce_len);
	addr[num_elem] = auth->Mx;
	len[num_elem] = auth->Mx_len;
	num_elem++;
	res = dpp_hmac_vector(hash_len, nonces, nonce_len,
			      num_elem, addr, len, prk);
	if (res < 0)
		return -1;
	wpa_hexdump_key(MSG_DEBUG, DPP_MAP_PREX "DPP: PRK = HKDF-Extract(<>, IKM)",
			prk, hash_len);

	/* HKDF-Expand(PRK, info, L) */
	res = dpp_hkdf_expand(hash_len, prk, hash_len, info_ke, ke, hash_len);
	os_memset(prk, 0, hash_len);
	if (res < 0)
		return -1;

	wpa_hexdump_key(MSG_INFO1, DPP_MAP_PREX "DPP: ke = HKDF-Expand(PRK, info, L)",
			ke, hash_len);
	return 0;
}

struct wpabuf * dpp_build_reconfig_auth_req(
#ifdef MAP_R3_RECONFIG
		struct wifi_app *wapp,
#endif
	struct dpp_authentication *auth, u8 trans_id)
{
	struct wpabuf *msg = NULL;
	struct wpabuf *connector  = NULL;
	int lenmsg = 0;

	if(!auth)
		return msg;

	wpa_printf(MSG_INFO1, DPP_MAP_PREX
			   "DPP: dpp_build_reconfig_auth_req() enter\n");
	// get connector
	connector = dpp_build_reconf_connetor(auth);
	if(!connector) {

		return msg;
	}

	// build msg
	msg = dpp_alloc_msg(DPP_PA_RECONFIG_AUTH_REQ,
			    5 + 5 + 4 + wpabuf_len(connector) + 4 + auth->curve->nonce_len);

	lenmsg = (5 + 5 + 4 + wpabuf_len(connector) + 4 + auth->curve->nonce_len);

	if (!msg) {
		if (connector)
			wpabuf_free(connector);
		return msg;
	}

	/* Transaction ID */
	wpabuf_put_le16(msg, DPP_ATTR_TRANSACTION_ID);
	wpabuf_put_le16(msg, 1);
	wpabuf_put_u8(msg, trans_id);

	/* Protocol Version */
	wpabuf_put_le16(msg, DPP_ATTR_PROTOCOL_VERSION);
	wpabuf_put_le16(msg, 1);
	wpabuf_put_u8(msg, auth->own_version);

	/* DPP Connector */
	wpabuf_put_le16(msg, DPP_ATTR_CONNECTOR);
	wpabuf_put_le16(msg, wpabuf_len(connector));
#ifdef MAP_R3_RECONFIG
	wpabuf_put_buf(msg,connector);
#else
	wpabuf_put_str(msg, wpabuf_head(connector));
#endif
	
	/* I-nonce */
	wpabuf_put_le16(msg, DPP_ATTR_I_NONCE);
	wpabuf_put_le16(msg, auth->curve->nonce_len);
	wpabuf_put_data(msg, auth->i_nonce, auth->curve->nonce_len);

	if (connector)
		wpabuf_free(connector);
	return msg;
}

int dpp_connector_get_netrole( struct json_token *peer_root)
{
	struct json_token *groups, *token;

	groups = json_get_member(peer_root, "groups");
	if (!groups || groups->type != JSON_ARRAY) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: No peer groups array found");
		return 0;
	}

	for (token = groups->child; token; token = token->sibling) {
		struct json_token *id, *role;

		id = json_get_member(token, "groupId");
		if (!id || id->type != JSON_STRING) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Missing peer groupId string");
			continue;
		}

		role = json_get_member(token, "netRole");
		if (!role || role->type != JSON_STRING) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Missing peer groups::netRole string");
			continue;
		}
		wpa_printf(MSG_DEBUG, DPP_MAP_PREX
			   "DPP: peer connector group: groupId='%s' netRole='%s'",
			   id->string, role->string);

#ifdef MAP_R3_RECONFIG
		if (os_strcmp( role->string, "mapController") == 0 ) {
			wpa_printf(MSG_ERROR, DPP_MAP_PREX
				   "DPP: Compatible group/netRole in own connector");
			return 1;
		}
		if (os_strcasecmp( role->string, "mapBackhaulSta") == 0 ) {
			wpa_printf(MSG_ERROR, DPP_MAP_PREX
				   "DPP: Compatible group/netRole in own connector");
			return 1;
		}
#endif
		if (os_strcmp( role->string, "configurator") == 0 ) {
			wpa_printf(MSG_DEBUG, DPP_MAP_PREX
				   "DPP: Compatible group/netRole in own connector");
			return 1;
		}
	}

	return 0;
}

struct json_token * dpp_parse_own_connector(const char *own_connector)
{
	unsigned char *own_conn;
	size_t own_conn_len;
	const char *pos, *end;
	struct json_token *own_root;

	pos = os_strchr(own_connector, '.');
	if (!pos) {
		wpa_printf(MSG_DEBUG, "DPP: Own connector is missing the first dot (.)");
		return NULL;
	}
	pos++;
	end = os_strchr(pos, '.');
	if (!end) {
		wpa_printf(MSG_DEBUG, "DPP: Own connector is missing the second dot (.)");
		return NULL;
	}
	own_conn = base64_url_decode(pos, end - pos, &own_conn_len);
	if (!own_conn) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Failed to base64url decode own signedConnector JWS Payload");
		return NULL;
	}

	own_root = json_parse((const char *) own_conn, own_conn_len);
	os_free(own_conn);
	if (!own_root)
		wpa_printf(MSG_DEBUG, "DPP: Failed to parse local connector");

	return own_root;
}


#ifdef RECONFIG_OLD
EVP_PKEY * dpp_check_valid_connector(const u8 *csign_key,
	size_t csign_key_len, const u8 *peer_connector,
	size_t peer_connector_len, int is_enrolle, int *result)
{
	struct json_token *root = NULL, *netkey = NULL;
	enum dpp_status_error ret = 255, res;
	const struct dpp_curve_params *own_curve;
	struct dpp_signed_connector_info info;
	const unsigned char *p;
	EVP_PKEY *csign = NULL, *peer_connetor_key = NULL, *ckey = NULL;
	char *signed_connector = NULL;

	os_memset(&info, 0, sizeof(info));
	p = csign_key;
	csign = d2i_PUBKEY(NULL, &p, csign_key_len);
	if (!csign) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Failed to parse local C-sign-key information");
		goto fail;
	}

	wpa_hexdump_ascii(MSG_DEBUG, DPP_MAP_PREX "DPP: Peer signedConnector",
			  peer_connector, peer_connector_len);
	signed_connector = os_malloc(peer_connector_len + 1);
	if (!signed_connector)
		goto fail;
	os_memcpy(signed_connector, peer_connector, peer_connector_len);
	signed_connector[peer_connector_len] = '\0';

	res = dpp_process_signed_connector(&info, csign, signed_connector);
	if (res != DPP_STATUS_OK) {
		ret = res;
		goto fail;
	}

	root = json_parse((const char *) info.payload, info.payload_len);
	if (!root) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: JSON parsing of connector failed");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}

	if (is_enrolle && !dpp_connector_get_netrole( root)) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: reconfig auth req connector does not include compatible group netrole is not configurator");
		ret = DPP_STATUS_NO_MATCH;
		goto fail;
	}

	netkey = json_get_member(root, "netAccessKey");
	if (!netkey || netkey->type != JSON_OBJECT) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: No netAccessKey object found");
		goto fail;
	}

	peer_connetor_key = dpp_parse_jwk(netkey, &own_curve);
	if (!peer_connetor_key) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: get  peer_connetor_key fail \n");
		goto fail;
	}
	dpp_debug_print_key("DPP: Received netAccessKey", peer_connetor_key);

	ckey = peer_connetor_key;
	ret  = DPP_STATUS_OK;
fail:
	wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: check valid connector ret :%d \n", ret);
	os_free(signed_connector);
	os_free(info.payload);
	EVP_PKEY_free(csign);
	json_free(root);
	*result = ret;
	return ckey;
}
#else
struct dpp_authentication * dpp_check_valid_connector(struct wifi_app *wapp, struct wapp_dev *wdev,
	const u8 *peer_connector, size_t peer_connector_len, const u8 *src, unsigned int chan, struct dpp_bootstrap_info *own_bi,
	const u8 *c_nonce, u16 c_nonce_len, const u8 *trans_id, const u8 *version)
{
	struct json_token *root = NULL, *token = NULL, *own_root = NULL;
	enum dpp_status_error ret = 255, res;
	//const struct dpp_curve_params *own_curve;
	struct dpp_signed_connector_info info;
	const unsigned char *p;
	const u8 *csign_key = NULL;
	char  *own_connector = NULL;
	size_t csign_key_len;
	struct wpabuf *msg = NULL, *conn_result = NULL;
	struct dpp_authentication *auth  = NULL;
	char *channel_list = NULL;
	char *channel_list_buf = NULL;

	EVP_PKEY *csign = NULL; /* *ckey = NULL; */
	char *signed_connector = NULL;

	os_memset(&info, 0, sizeof(info));
	if (wdev == NULL) {
		wpa_printf(MSG_ERROR,
			"DPP: %s %d wdev is NULL", __func__, __LINE__);
		goto fail;
	}

	csign_key = wdev->config->dpp_csign;
	csign_key_len = wdev->config->dpp_csign_len;
	own_connector = wdev->config->dpp_connector;
	
	p = csign_key;
	csign = d2i_PUBKEY(NULL, &p, csign_key_len);
	if (!csign) {
		wpa_printf(MSG_ERROR,
			   "DPP: Failed to parse local C-sign-key information");
		goto fail;
	}

	wpa_hexdump_ascii(MSG_DEBUG, "DPP: Peer signedConnector",
			  peer_connector, peer_connector_len);
	signed_connector = os_malloc(peer_connector_len + 1);
	if (!signed_connector)
		goto fail;
	os_memcpy(signed_connector, peer_connector, peer_connector_len);
	signed_connector[peer_connector_len] = '\0';

	res = dpp_process_signed_connector(&info, csign, signed_connector);
	if (res != DPP_STATUS_OK) {
		ret = res;
		goto fail;
	}

	root = json_parse((const char *) info.payload, info.payload_len);
	if (!root) {
		wpa_printf(MSG_ERROR, "DPP: JSON parsing of connector failed");
		ret = DPP_STATUS_INVALID_CONNECTOR;
		goto fail;
	}

	own_root = dpp_parse_own_connector(own_connector);
	if (!root || !own_root ||
	    !dpp_connector_match_groups(own_root, root, TRUE)) {
		wpa_printf(MSG_DEBUG,
			   "DPP: I-Connector does not include compatible group netrole with own connector");
		goto fail;
	}

	token = json_get_member(root, "expiry");
	if (token && token->type == JSON_STRING &&
	    dpp_key_expired(token->string, NULL)) {
		wpa_printf(MSG_DEBUG,
			   "DPP: I-Connector (netAccessKey) has expired");
		goto fail;
	}

	token = json_get_member(root, "netAccessKey");
	if (!token || token->type != JSON_OBJECT) {
		wpa_printf(MSG_ERROR, "DPP: No netAccessKey object found");
		goto fail;
	}

	auth = os_zalloc(sizeof(*auth));
	if (!auth) {
		goto fail;
	}

	os_memset(auth, 0, sizeof(struct dpp_authentication));
	auth->msg_ctx = (void *)wapp;
	auth->peer_bi = NULL;
	auth->own_bi = own_bi;
	auth->curve = own_bi->curve;
	auth->curr_chan = chan;
	auth->own_version = 1;
#ifdef MAP_R3_RECONFIG
	auth->reconfigTrigger = TRUE;
#endif
	if(wapp)
		auth->own_version = wapp->dpp->version_ctrl;
	auth->peer_version = 1; /* default to the first version */
	auth->wdev = wdev;

	auth->reconfig = 1;
	//auth->allowed_roles = DPP_CAPAB_ENROLLEE;
	//if (dpp_prepare_channel_list(auth, freq, NULL, 0) < 0)
		//goto fail;

	auth->transaction_id = trans_id[0];

	auth->peer_version = version[0];
	wpa_printf(MSG_DEBUG, "DPP: Peer protocol version %u",
		   auth->peer_version);
#if 0
	if(ckey) {
		if(auth->peer_connector_key)
			EVP_PKEY_free(auth->peer_connector_key);
		auth->peer_connector_key = ckey;
	}
#endif /* #if 0 */
	if (!is_zero_ether_addr(src))
		os_memcpy(auth->peer_mac_addr, src, MAC_ADDR_LEN);

	os_memcpy(auth->c_nonce, c_nonce, c_nonce_len);
	/* build connect status  */
	if (wapp && wapp->dpp && wapp->dpp->scan_ch.ch_num) {
		channel_list_buf = wapp_dpp_get_scan_channel_list(wapp);
		channel_list = channel_list_buf;
	}


	if (dpp_reconfig_derive_ke_responder(auth, wdev->config->dpp_netaccesskey,
					     wdev->config->dpp_netaccesskey_len, token) < 0)
		goto fail;


	if (c_nonce_len != auth->curve->nonce_len) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Unexpected C-nonce length %u (curve nonce len %zu)",
			   c_nonce_len, auth->curve->nonce_len);
		goto fail;
	}

	/* Build Connection Status object */
	/* TODO: Get appropriate result value */
	/* TODO: ssid64 and channelList */
	conn_result = dpp_build_conn_status(DPP_STATUS_NO_AP, auth->wdev->config->ssid, 
		auth->wdev->config->ssid_len, (const char *)channel_list);

	if (!conn_result)
		goto fail;

	msg = dpp_reconfig_build_resp(auth, own_connector, conn_result);

	if (!msg) {
		dpp_auth_deinit(auth);
		DBGPRINT(RT_DEBUG_OFF,  "DPP: the build reconfig auth resp msg is null \n");
		goto fail;
	}

	if (dpp_set_configurator(wapp->dpp, wapp, auth, " ") < 0) {
		dpp_auth_deinit(auth);
		DBGPRINT(RT_DEBUG_OFF, "dpp_set_configurator \n");
		goto fail;
	}

	if(wapp_dpp_auth_list_insert(wapp, auth) < 0) {
		dpp_auth_deinit(auth);
			DBGPRINT(RT_DEBUG_OFF, "dpp auth list insert fail \n");
		goto fail;
	}
	DBGPRINT(RT_DEBUG_OFF, "cur ch:%d, op_ch:%d \n", auth->curr_chan, wdev->radio->op_ch);
	if (auth->curr_chan != wdev->radio->op_ch) {
		auth->wdev = wdev;
		/* Since we are initiator, move to responder's channel */
		wapp_cancel_remain_on_channel(wapp, auth->wdev);  
		wdev_set_quick_ch(wapp, auth->wdev, auth->curr_chan);
	}

	//send the reconfig auth resp
	wpa_printf(MSG_INFO1, DPP_EVENT_TX "dst=" MACSTR ", type=%d",
		MAC2STR(auth->peer_mac_addr), DPP_PA_RECONFIG_AUTH_RESP);
	wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0, auth->peer_mac_addr,
				wpabuf_head(msg), wpabuf_len(msg));
	auth->current_state = DPP_STATE_RECONFIG_AUTH_CONF_WAITING;
	/* set register to wait the auth firm */
	eloop_cancel_timeout(wapp_dpp_reconfig_auth_confirm_wait_timeout, wapp, auth->peer_mac_addr);
	eloop_register_timeout(5, 0, wapp_dpp_reconfig_auth_confirm_wait_timeout, wapp, auth->peer_mac_addr);

out:
	os_free(signed_connector);
	os_free(info.payload);
	EVP_PKEY_free(csign);
	json_free(root);
	json_free(own_root);
	wpabuf_free(conn_result);
	wpabuf_free(msg);
	if (channel_list_buf != NULL) {
		os_free (channel_list_buf);
		channel_list_buf = NULL;
		channel_list = NULL;
	}
	return auth;
fail:
	dpp_auth_deinit(auth);
	auth = NULL;
	goto out;
}
#endif

struct wpabuf * dpp_build_reconfig_auth_resp(struct wifi_app *wapp, struct dpp_authentication *auth,
				struct dpp_config *config, u8 trans_id,  struct wpabuf *conn_result)
{
	struct wpabuf *msg = NULL, *tmp = NULL, *pr = NULL;
	char *connector  = NULL;
	size_t nonce_len; //, len[1];
	u8 *wrapped;
	//const u8 *addr[1];
	const struct dpp_curve_params *own_curve;

	if (!wapp  || !auth || !config || !conn_result)
		return msg;
	nonce_len = auth->curve->nonce_len;

	auth->own_connector_key = dpp_set_keypair(&own_curve, config->dpp_netaccesskey,
				  config->dpp_netaccesskey_len);
	if (!auth->own_connector_key) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to parse local C-connector-key information");
		goto fail;
	}

	if (random_get_bytes(auth->r_nonce, nonce_len)) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to generate R-nonce");
		goto fail;
	}
	wpa_hexdump(MSG_INFO1, DPP_MAP_PREX "DPP: gen reconfig R-nonce", auth->r_nonce, nonce_len);
	auth->own_protocol_key = dpp_gen_keypair(auth->curve);
	if (!auth->own_protocol_key)
		goto fail;

	pr = dpp_get_pubkey_point(auth->own_protocol_key, 0);
	if (!pr)
		goto fail;

	// get responder connector
	connector = config->dpp_connector;

	if(!connector) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Failed to get connector \n");
		goto fail;
	}

	// calc M = { cR + pR } * CI
	if (dpp_auth_derive_m_R(auth) < 0)
		goto fail;

       // calc ke,   ke = HKDF(I-nonce, "dpp reconfig key", M.x)
       if (dpp_derive_reconfig_ke(auth, auth->ke, auth->curve->hash_len) < 0)
		goto fail;

	nonce_len = auth->curve->nonce_len;

	// build msg
	tmp = wpabuf_alloc(4 + nonce_len + 4 + nonce_len + 4 + wpabuf_len(conn_result));
	if (!tmp)
		goto fail;

	/* I-nonce */
	wpabuf_put_le16(tmp, DPP_ATTR_I_NONCE);
	wpabuf_put_le16(tmp, nonce_len);
	wpabuf_put_data(tmp, auth->i_nonce, nonce_len);

	/* R-nonce */
	wpabuf_put_le16(tmp, DPP_ATTR_R_NONCE);
	wpabuf_put_le16(tmp, nonce_len);
	wpabuf_put_data(tmp, auth->r_nonce, nonce_len);

	wpabuf_put_le16(tmp, DPP_ATTR_CONN_STATUS);
	wpabuf_put_le16(tmp, wpabuf_len(conn_result));
	//wpabuf_put_data(tmp, wpabuf_head(conn_result), wpabuf_len(conn_result) );
	wpabuf_put_buf(tmp, conn_result);
	// build msg
	msg = dpp_alloc_msg(DPP_PA_RECONFIG_AUTH_RESP,
			    5 + 5 + 4 + os_strlen(connector) + 4+wpabuf_len(pr) + 4 + wpabuf_len(tmp) + AES_BLOCK_SIZE );

	if (!msg)
		goto fail;

	/* Transaction ID */
	wpabuf_put_le16(msg, DPP_ATTR_TRANSACTION_ID);
	wpabuf_put_le16(msg, 1);
	wpabuf_put_u8(msg, trans_id);

	/* Protocol Version */
	wpabuf_put_le16(msg, DPP_ATTR_PROTOCOL_VERSION);
	wpabuf_put_le16(msg, 1);
	wpabuf_put_u8(msg, auth->own_version);

	/* DPP Responder Connector */
	wpabuf_put_le16(msg, DPP_ATTR_CONNECTOR);
	wpabuf_put_le16(msg, os_strlen(connector));
	wpabuf_put_str(msg, connector);

	/* Responder Protocol Key */
	wpabuf_put_le16(msg, DPP_ATTR_R_PROTOCOL_KEY);
	wpabuf_put_le16(msg, wpabuf_len(pr));
	wpabuf_put_buf(msg, pr);

	wpabuf_put_le16(msg, DPP_ATTR_WRAPPED_DATA);
	wpabuf_put_le16(msg, wpabuf_len(tmp) + AES_BLOCK_SIZE);
	wrapped = wpabuf_put(msg, wpabuf_len(tmp) + AES_BLOCK_SIZE);

	wpa_hexdump_buf(MSG_INFO1, "DPP: AES-SIV cleartext", tmp);
	if (aes_siv_encrypt(auth->ke, auth->curve->hash_len,
			    wpabuf_head(tmp), wpabuf_len(tmp),
			    0, NULL, NULL, wrapped) < 0)
		goto fail;
	wpa_hexdump(MSG_INFO1, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped, wpabuf_len(tmp) + AES_BLOCK_SIZE);

	wpabuf_free(pr);
	wpabuf_free(tmp);
	return msg;
fail:
	EVP_PKEY_free(auth->own_connector_key);
	EVP_PKEY_free(auth->own_protocol_key);
	//EVP_PKEY_free(auth->peer_connector_key);
	wpabuf_free(pr);
	wpabuf_free(tmp);
	wpabuf_free(msg);
	return NULL;
}


 int dpp_parse_reconfig_auth_resp(struct wifi_app *wapp, struct dpp_authentication *auth,
				const u8 *buf, size_t buf_len, int *conn_value)
{
	const u8 *i_nonce = NULL, *r_nonce = NULL, *wrapped_data = NULL, *status = NULL;
	u16 i_nonce_len = 0, r_nonce_len = 0, wrapped_data_len = 0, status_len = 0;
	//const u8 *addr[1], *r_proto = NULL;
	const u8 *r_proto = NULL;
	u16 r_proto_len = 0;
	u8 *unwrapped = NULL;
	size_t unwrapped_len = 0;
	//size_t len[1], unwrapped_len = 0;
	struct json_token *root = NULL, *token = NULL;
	EVP_PKEY *pr= NULL;

	r_proto = dpp_get_attr(buf, buf_len, DPP_ATTR_R_PROTOCOL_KEY,
			       &r_proto_len);
	if (!r_proto) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Missing required Responder Protocol Key attribute");
		goto fail;
	}
	wpa_hexdump(MSG_MSGDUMP, DPP_MAP_PREX "DPP: Responder Protocol Key",
		    r_proto, r_proto_len);

	pr = dpp_set_pubkey_point(auth->own_protocol_key, r_proto, r_proto_len);
	if (!pr) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Invalid Responder Protocol Key");
		goto fail;
	}
	dpp_debug_print_key("Peer (Responder) Protocol Key", pr);
	if(auth->peer_protocol_key) {
		EVP_PKEY_free(auth->peer_protocol_key);
		auth->peer_protocol_key = NULL;
	}

	auth->peer_protocol_key = pr;

	// calc M = cI * { CR + PR }
	auth->own_connector_key = auth->own_protocol_key;
	if (dpp_auth_derive_m_I(auth) < 0) {
		auth->own_connector_key = NULL;
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: derive reconfig Mx fail");
		goto fail;
	}
	auth->own_connector_key = NULL;
       // calc ke,  ke = HKDF(I-nonce, "dpp reconfig key", M.x)
       if (dpp_derive_reconfig_ke(auth, auth->ke, auth->curve->hash_len) < 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: derive reconfig ke fail");
		goto fail;
	}

	wrapped_data = dpp_get_attr(buf, buf_len, DPP_ATTR_WRAPPED_DATA,
				    &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Missing or invalid required Wrapped Data attribute");
		goto fail;
	}

	wpa_hexdump(MSG_INFO1, DPP_MAP_PREX "DPP: AES-SIV ciphertext", wrapped_data, wrapped_data_len);
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped = os_malloc(unwrapped_len);
	if (!unwrapped)
		goto fail;

	if (aes_siv_decrypt(auth->ke, auth->curve->hash_len,
			    wrapped_data, wrapped_data_len,
			    0, NULL, NULL, unwrapped) < 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: AES-SIV decryption failed");
		goto fail;
	}
	wpa_hexdump(MSG_ERROR, DPP_MAP_PREX "DPP: AES-SIV cleartext",
		    unwrapped, unwrapped_len);

	// get I nonce
	i_nonce = dpp_get_attr(unwrapped, unwrapped_len,  DPP_ATTR_I_NONCE,  &i_nonce_len);
	if (!i_nonce || i_nonce_len != auth->curve->nonce_len) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Missing or invalid I Nonce attribute");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: I Nonce", i_nonce,i_nonce_len);
	if (os_memcmp(i_nonce, auth->i_nonce, i_nonce_len) != 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: I Nonce mismatch");
		goto fail;
	}
	
	/* get R nonce */
	r_nonce = dpp_get_attr(unwrapped, unwrapped_len,
			       DPP_ATTR_R_NONCE,
			       &r_nonce_len);
	if (!r_nonce || r_nonce_len != auth->curve->nonce_len) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "Missing or invalid Enrollee Nonce attribute");
		goto fail;
	}
	wpa_hexdump(MSG_ERROR, DPP_MAP_PREX "DPP: receive reconfig R Nonce", r_nonce, r_nonce_len);
	os_memcpy(auth->r_nonce, r_nonce, r_nonce_len);

	status = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_CONN_STATUS,
			      &status_len);
	if (!status) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Missing required DPP Connection Status attribute");
		goto fail;
	}
	wpa_hexdump_ascii(MSG_INFO1, DPP_MAP_PREX "DPP: connStatus JSON",
			  status, status_len);

	root = json_parse((const char *) status, status_len);
	if (!root) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Could not parse connStatus");
		goto fail;
	}

	token = json_get_member(root, "result");
	if (!token || token->type != JSON_NUMBER) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: No connStatus - result");
		goto fail;
	}
	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: previous connStatus result %d", token->number);
	*conn_value = token->number;
	json_free(root);
	bin_clear_free(unwrapped, unwrapped_len);
	return 0;
fail:
	json_free(root);
	bin_clear_free(unwrapped, unwrapped_len);
	return -1;
}

 struct wpabuf * dpp_build_reconfig_auth_confirm(struct wifi_app *wapp, 
				struct dpp_authentication *auth, int conn_value,  u8 trans_id)
{
	struct wpabuf *msg = NULL, *tmp = NULL;
	size_t nonce_len = 0, json_len = 0;
	u8 *wrapped;
	char json[50];
	int connectorKey = 0;
	int ret;
	if (!wapp  || !auth)
		return msg;

      //  get reconfig flags obj;
	if(conn_value == DPP_STATUS_INVALID_CONNECTOR
#ifdef MAP_R3_RECONFIG
		|| conn_value == DPP_STATUS_NO_AP	
#endif
	) {
		connectorKey = DPP_CONFIG_REUSEKEY;
		if(auth->peer_protocol_key)
			EVP_PKEY_free(auth->peer_protocol_key);
		auth->peer_protocol_key = auth->peer_connector_key;
		auth->peer_connector_key = NULL;
	} else if (conn_value == DPP_STATUS_NO_MATCH) {
		connectorKey = DPP_CONFIG_REPLACEKEY;
	} else {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: input invalid connet value %d", conn_value);
		goto fail;
	}

	ret = os_snprintf(json, sizeof(json),
				"{\"connectorKey\":%d}",
				connectorKey);
	if (os_snprintf_error(sizeof(json), ret)) {
		DBGPRINT(RT_DEBUG_ERROR, "%s %d os_snprintf error\n", __func__, __LINE__);
		goto fail;
	}

	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: connectorKey json: %s", json);
	json_len = os_strlen(json);
	nonce_len = auth->curve->nonce_len;
	// build msg
	tmp = wpabuf_alloc(5 + 5+ 4 + nonce_len + 4+ nonce_len + 4+json_len);
	if (!tmp)
		goto fail;

	/* Transaction ID */
	wpabuf_put_le16(tmp, DPP_ATTR_TRANSACTION_ID);
	wpabuf_put_le16(tmp, 1);
	wpabuf_put_u8(tmp, trans_id);

	/* Protocol Version */
	wpabuf_put_le16(tmp, DPP_ATTR_PROTOCOL_VERSION);
	wpabuf_put_le16(tmp, 1);
	wpabuf_put_u8(tmp, auth->own_version);
	/* I-nonce */
	wpabuf_put_le16(tmp, DPP_ATTR_I_NONCE);
	wpabuf_put_le16(tmp, nonce_len);
	wpabuf_put_data(tmp, auth->i_nonce, nonce_len);

	/* R-nonce */
	wpabuf_put_le16(tmp, DPP_ATTR_R_NONCE);
	wpabuf_put_le16(tmp, nonce_len);
	wpabuf_put_data(tmp, auth->r_nonce, nonce_len);
	wpa_hexdump(MSG_INFO1, DPP_MAP_PREX "DPP: build reconfig R Nonce", auth->r_nonce, nonce_len);
	/* reconfig flags */
	wpabuf_put_le16(tmp, DPP_ATTR_RECONFIG_FLAGS);
	wpabuf_put_le16(tmp, json_len);
	wpabuf_put_data(tmp, json, json_len);

	// build msg
	msg = dpp_alloc_msg(DPP_PA_RECONFIG_AUTH_CONF,
			    4 + wpabuf_len(tmp) + AES_BLOCK_SIZE );
	if (!msg)
		goto fail;

	wpabuf_put_le16(msg, DPP_ATTR_WRAPPED_DATA);
	wpabuf_put_le16(msg, wpabuf_len(tmp) + AES_BLOCK_SIZE);
	wrapped = wpabuf_put(msg, wpabuf_len(tmp) + AES_BLOCK_SIZE);

	wpa_hexdump_buf(MSG_DEBUG, "DPP: AES-SIV cleartext", tmp);

	if (aes_siv_encrypt(auth->ke, auth->curve->hash_len,
			    wpabuf_head(tmp), wpabuf_len(tmp),
			    0, NULL, NULL, wrapped) < 0)
		goto fail;
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext",
		    wrapped, wpabuf_len(tmp) + AES_BLOCK_SIZE);

	wpabuf_free(tmp);
	dpp_auth_success(auth);
	return msg;
fail:
	wpabuf_free(tmp);
	wpabuf_free(msg);
	return NULL;
 }


#ifdef RECONFIG_OLD 
 int dpp_parse_reconfig_auth_confirm(struct wifi_app *wapp, struct dpp_authentication *auth,
				const u8 *buf, size_t buf_len, int *reconfig_value)
{
	const u8 *i_nonce = NULL, *r_nonce = NULL, *wrapped_data = NULL;
	u16 i_nonce_len = 0, r_nonce_len = 0, wrapped_data_len = 0;
	const u8 *reconfig = NULL, *trans_id = NULL, *version = NULL;
	u8* unwrapped = NULL;
	u16 reconfig_len = 0, trans_id_len = 0, version_len = 0;
	size_t unwrapped_len = 0;
	struct json_token *root = NULL, *token;

	wrapped_data = dpp_get_attr(buf, buf_len, DPP_ATTR_WRAPPED_DATA,
				    &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			      "Missing or invalid required Wrapped Data attribute");
		goto fail;
	}

	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV ciphertext", wrapped_data, wrapped_data_len);
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped = os_malloc(unwrapped_len);
	if (!unwrapped)
		goto fail;

	if (aes_siv_decrypt(auth->ke, auth->curve->hash_len,
			    wrapped_data, wrapped_data_len,
			    0, NULL, NULL, unwrapped) < 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: AES-SIV decryption failed");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: AES-SIV cleartext",
		    unwrapped, unwrapped_len);

	if (dpp_check_attrs(unwrapped, unwrapped_len) < 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Invalid attribute in unwrapped data");
		goto fail;
	}

	/* get transaction id */ 
	trans_id = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_TRANSACTION_ID,
			       &trans_id_len);
	if (!trans_id || trans_id_len != 1) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX
			   "DPP: Peer did not include Transaction ID");
		goto fail;
	}

	/* get version */
	version = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_PROTOCOL_VERSION,
			       &version_len);
	if (version) {
		if (version_len < 1 || version[0] == 0 || auth->peer_version != version[0]) {
			wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Invalid Protocol Version attribute");
			goto fail;
		}
	}

	// get I nonce
	i_nonce = dpp_get_attr(unwrapped, unwrapped_len,  DPP_ATTR_I_NONCE,  &i_nonce_len);
	if (!i_nonce || i_nonce_len != auth->curve->nonce_len) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Missing or invalid i Nonce attribute");
		goto fail;
	}
	wpa_hexdump(MSG_DEBUG, DPP_MAP_PREX "DPP: I Nonce", i_nonce,i_nonce_len);

	if (os_memcmp(i_nonce, auth->i_nonce, i_nonce_len) != 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: I Nonce mismatch");
		goto fail;
	}
	/* get R nonce */
	r_nonce = dpp_get_attr(unwrapped, unwrapped_len,
			       DPP_ATTR_R_NONCE,
			       &r_nonce_len);
	if (!r_nonce || r_nonce_len != auth->curve->nonce_len) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Missing or invalid Enrollee Nonce attribute");
		goto fail;
	}
	wpa_hexdump(MSG_INFO1, DPP_MAP_PREX "DPP: receive reconfig R Nonce", r_nonce, r_nonce_len);

	if (os_memcmp(r_nonce, auth->r_nonce, r_nonce_len) != 0) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: R Nonce mismatch");
		goto fail;
	}

	reconfig = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_RECONFIG_FLAGS,
			      &reconfig_len);
	if (!reconfig) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Missing required DPP Connection Status attribute");
		goto fail;
	}
	wpa_hexdump_ascii(MSG_INFO1, DPP_MAP_PREX "DPP: received connectkey JSON",
			  reconfig, reconfig_len);

	root = json_parse((const char *) reconfig, reconfig_len);
	if (!root) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: Could not parse connectkey");
		goto fail;
	}

	token = json_get_member(root, "connectorKey");
	if (!token || token->type != JSON_NUMBER) {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: No connectorKey");
		goto fail;
	}
	wpa_printf(MSG_INFO1, DPP_MAP_PREX "DPP: reconfig connectorKey %d", token->number);
	*reconfig_value = token->number;
	if(token->number == DPP_CONFIG_REUSEKEY) {
		if(auth->own_protocol_key)
			EVP_PKEY_free(auth->own_protocol_key);
		auth->own_protocol_key = auth->own_connector_key;
		auth->own_connector_key = NULL;
		wpa_printf(MSG_INFO1, DPP_MAP_PREX
			   "DPP: connectorKey value is DPP_CONFIG_REUSEKEY");
	} else if(token->number == DPP_CONFIG_REPLACEKEY) {
		wpa_printf(MSG_INFO1, DPP_MAP_PREX
			   "DPP: connectorKey value is DPP_CONFIG_REPLACEKEY");
	} else {
		wpa_printf(MSG_ERROR, DPP_MAP_PREX "DPP: connectorKey value invalid");
		goto fail;
	}
	bin_clear_free(unwrapped, unwrapped_len);
	json_free(root);
	dpp_auth_success(auth);
	return 0;
fail:
	json_free(root);
	bin_clear_free(unwrapped, unwrapped_len);
	return -1;
}
#endif
#endif /* DPP_R2_RECONFIG */


