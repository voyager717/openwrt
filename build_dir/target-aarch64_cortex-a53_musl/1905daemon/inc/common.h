/*
 * wpa_supplicant/hostapd / common helper functions, etc.
 * Copyright (c) 2002-2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef COMMON_H
#define COMMON_H

#include "os.h"

typedef unsigned long long UINT64;
typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;
typedef signed long long INT64;
typedef signed int INT32;
typedef signed short INT16;
typedef signed char INT8;

typedef UINT64 u64;
typedef UINT32 u32;
typedef UINT16 u16;
typedef UINT8 u8;
typedef INT64 s64;
typedef INT32 s32;
typedef INT16 s16;
typedef INT8 s8;

#ifdef __CHECKER__
#define __force __attribute__((force))
#undef __bitwise
#define __bitwise __attribute__((bitwise))
#else
#define __force
#undef __bitwise
#define __bitwise
#endif

#ifndef __must_check
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#define __must_check __attribute__((__warn_unused_result__))
#else
#define __must_check
#endif /* __GNUC__ */
#endif /* __must_check */


typedef u16 __bitwise be16;
typedef u16 __bitwise le16;
typedef u32 __bitwise be32;
typedef u32 __bitwise le32;
typedef u64 __bitwise be64;
typedef u64 __bitwise le64;

static inline unsigned short wpa_swap_16(unsigned short v)
{
	return ((v & 0xff) << 8) | (v >> 8);
}

static inline unsigned int wpa_swap_32(unsigned int v)
{
	return ((v & 0xff) << 24) | ((v & 0xff00) << 8) |
		((v & 0xff0000) >> 8) | (v >> 24);
}

static inline unsigned long long wpa_swap_64(unsigned long long v)
{
	return ((v & 0xff) << 56) | ((v & 0xff00) << 40) |
		((v & 0xff0000) << 24) | ((v & 0xff000000) << 8) |
		((v & 0xff00000000) >> 8) | ((v & 0xff0000000000) >> 24) |
		((v & 0xff000000000000) >> 40) | (v >> 56);
}

#ifdef RT_BIG_ENDIAN
#define host_to_le64(x) wpa_swap_64((x))
#define le_to_host64(x) wpa_swap_64((x))
#define host_to_le32(x) wpa_swap_32((x))
#define le_to_host32(x) wpa_swap_32((x))
#define host_to_le16(x) wpa_swap_16((x))
#define le_to_host16(x) wpa_swap_16((x))
#define host_to_be64(x) ((UINT64)(x))
#define be_to_host64(x) ((UINT64)(x))
#define host_to_be32(x) ((UINT32)(x))
#define be_to_host32(x) ((UINT32)(x))
#define host_to_be16(x) ((UINT16)(x))
#define be_to_host16(x) ((UINT16)(x))
#else /* Little_Endian */
#define host_to_le64(x) ((UINT64)(x))
#define le_to_host64(x) ((UINT64)(x))
#define host_to_le32(x) ((UINT32)(x))
#define le_to_host32(x) ((UINT32)(x))
#define host_to_le16(x) ((UINT16)(x))
#define le_to_host16(x) ((UINT16)(x))
#define host_to_be64(x) wpa_swap_64((x))
#define be_to_host64(x) wpa_swap_64((x))
#define host_to_be32(x) wpa_swap_32((x))
#define be_to_host32(x) wpa_swap_32((x))
#define host_to_be16(x) wpa_swap_16((x))
#define be_to_host16(x) wpa_swap_16((x))
#endif /* RT_BIG_ENDIAN */

static inline u16 WPA_GET_BE16(const u8 *a)
{
	return (a[0] << 8) | a[1];
}

static inline void WPA_PUT_BE16(u8 *a, u16 val)
{
	a[0] = val >> 8;
	a[1] = val & 0xff;
}

static inline u16 WPA_GET_LE16(const u8 *a)
{
	return (a[1] << 8) | a[0];
}

static inline void WPA_PUT_LE16(u8 *a, u16 val)
{
	a[1] = val >> 8;
	a[0] = val & 0xff;
}

static inline u32 WPA_GET_BE24(const u8 *a)
{
	return (a[0] << 16) | (a[1] << 8) | a[2];
}

static inline void WPA_PUT_BE24(u8 *a, u32 val)
{
	a[0] = (val >> 16) & 0xff;
	a[1] = (val >> 8) & 0xff;
	a[2] = val & 0xff;
}

static inline u32 WPA_GET_BE32(const u8 *a)
{
	return ((u32) a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3];
}

static inline void WPA_PUT_BE32(u8 *a, u32 val)
{
	a[0] = (val >> 24) & 0xff;
	a[1] = (val >> 16) & 0xff;
	a[2] = (val >> 8) & 0xff;
	a[3] = val & 0xff;
}

static inline u32 WPA_GET_LE32(const u8 *a)
{
	return ((u32) a[3] << 24) | (a[2] << 16) | (a[1] << 8) | a[0];
}

static inline void WPA_PUT_LE32(u8 *a, u32 val)
{
	a[3] = (val >> 24) & 0xff;
	a[2] = (val >> 16) & 0xff;
	a[1] = (val >> 8) & 0xff;
	a[0] = val & 0xff;
}

static inline u64 WPA_GET_BE64(const u8 *a)
{
	return (((u64) a[0]) << 56) | (((u64) a[1]) << 48) |
		(((u64) a[2]) << 40) | (((u64) a[3]) << 32) |
		(((u64) a[4]) << 24) | (((u64) a[5]) << 16) |
		(((u64) a[6]) << 8) | ((u64) a[7]);
}

static inline void WPA_PUT_BE64(u8 *a, u64 val)
{
	a[0] = val >> 56;
	a[1] = val >> 48;
	a[2] = val >> 40;
	a[3] = val >> 32;
	a[4] = val >> 24;
	a[5] = val >> 16;
	a[6] = val >> 8;
	a[7] = val & 0xff;
}

static inline u64 WPA_GET_LE64(const u8 *a)
{
	return (((u64) a[7]) << 56) | (((u64) a[6]) << 48) |
		(((u64) a[5]) << 40) | (((u64) a[4]) << 32) |
		(((u64) a[3]) << 24) | (((u64) a[2]) << 16) |
		(((u64) a[1]) << 8) | ((u64) a[0]);
}

static inline void WPA_PUT_LE64(u8 *a, u64 val)
{
	a[7] = val >> 56;
	a[6] = val >> 48;
	a[5] = val >> 40;
	a[4] = val >> 32;
	a[3] = val >> 24;
	a[2] = val >> 16;
	a[1] = val >> 8;
	a[0] = val & 0xff;
}

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif
#ifndef ETH_HLEN
#define ETH_HLEN 14
#endif
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif
#ifndef ETH_P_ALL
#define ETH_P_ALL 0x0003
#endif
#ifndef ETH_P_IP
#define ETH_P_IP 0x0800
#endif
#ifndef ETH_P_80211_ENCAP
#define ETH_P_80211_ENCAP 0x890d /* TDLS comes under this category */
#endif
#ifndef ETH_P_PAE
#define ETH_P_PAE 0x888E /* Port Access Entity (IEEE 802.1X) */
#endif /* ETH_P_PAE */
#ifndef ETH_P_EAPOL
#define ETH_P_EAPOL ETH_P_PAE
#endif /* ETH_P_EAPOL */
#ifndef ETH_P_RSN_PREAUTH
#define ETH_P_RSN_PREAUTH 0x88c7
#endif /* ETH_P_RSN_PREAUTH */
#ifndef ETH_P_RRB
#define ETH_P_RRB 0x890D
#endif /* ETH_P_RRB */
#ifndef ETH_P_OUI
#define ETH_P_OUI 0x88B7
#endif /* ETH_P_OUI */


#ifdef __GNUC__
#define PRINTF_FORMAT(a,b) __attribute__ ((format (printf, (a), (b))))
#define STRUCT_PACKED __attribute__ ((packed))
#else
#define PRINTF_FORMAT(a,b)
#define STRUCT_PACKED
#endif
/* vsnprintf - only used for wpa_msg() in wpa_supplicant.c */
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

/* getopt - only used in main.c */
int getopt(int argc, char *const argv[], const char *optstring);
extern char *optarg;
extern int optind;


#ifndef MAC2STR
#ifdef CONFIG_MASK_MACADDR
#define MAC2STR(a) (a)[0], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:**:**:%02x:%02x:%02x"
#define PRINT_MAC(a) a[0],a[3],a[4],a[5]
#else
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define PRINT_MAC(a) a[0],a[1],a[2],a[3],a[4],a[5]
#endif
#endif

#ifndef BIT
#define BIT(x) (1U << (x))
#endif

#define SSID_MAX_LEN 32

struct wpa_ssid_value {
	u8 ssid[SSID_MAX_LEN];
	size_t ssid_len;
};

enum {
	MSG_EXCESSIVE, MSG_MSGDUMP, MSG_DEBUG, MSG_INFO, MSG_WARNING, MSG_ERROR,
};

int hwaddr_aton(const char *txt, u8 *addr);
int hwaddr_masked_aton(const char *txt, u8 *addr, u8 *mask, u8 maskable);
int hwaddr_compact_aton(const char *txt, u8 *addr);
int hwaddr_aton2(const char *txt, u8 *addr);
int ipv4_aton(const char *txt, unsigned int *ip_out);
int hex2byte(const char *hex);
int hexstr2bin(const char *hex, u8 *buf, size_t len);
void inc_byte_array(u8 *counter, size_t len);
void buf_shift_right(u8 *buf, size_t len, size_t bits);
void wpa_get_ntp_timestamp(u8 *buf);
int wpa_scnprintf(char *buf, size_t size, const char *fmt, ...);
int wpa_snprintf_hex_sep(char *buf, size_t buf_size, const u8 *data, size_t len,
			 char sep);
int wpa_snprintf_hex(char *buf, size_t buf_size, const u8 *data, size_t len);
int wpa_snprintf_hex_uppercase(char *buf, size_t buf_size, const u8 *data,
			       size_t len);

int hwaddr_mask_txt(char *buf, size_t len, const u8 *addr, const u8 *mask);
int ssid_parse(const char *buf, struct wpa_ssid_value *ssid);
void printf_encode(char *txt, size_t maxlen, const u8 *data, size_t len);
size_t printf_decode(u8 *buf, size_t maxlen, const char *str);
const char * wpa_ssid_txt(const u8 *ssid, size_t ssid_len);
char * wpa_config_parse_string(const char *value, size_t *len);
int is_hex(const u8 *data, size_t len);
int has_ctrl_char(const u8 *data, size_t len);
int has_newline(const char *str);
size_t merge_byte_arrays(u8 *res, size_t res_len,
			 const u8 *src1, size_t src1_len,
			 const u8 *src2, size_t src2_len);
char * dup_binstr(const void *src, size_t len);

static inline int is_zero_ether_addr(const u8 *a)
{
	return !(a[0] | a[1] | a[2] | a[3] | a[4] | a[5]);
}

static inline int is_broadcast_ether_addr(const u8 *a)
{
	return (a[0] & a[1] & a[2] & a[3] & a[4] & a[5]) == 0xff;
}

static inline int is_multicast_ether_addr(const u8 *a)
{
	return a[0] & 0x01;
}

int dec2num(char c);

#define broadcast_ether_addr (const u8 *) "\xff\xff\xff\xff\xff\xff"


struct wpa_freq_range_list {
	struct wpa_freq_range {
		unsigned int min;
		unsigned int max;
	} *range;
	unsigned int num;
};

int freq_range_list_includes(const struct wpa_freq_range_list *list,
			     unsigned int freq);
char * freq_range_list_str(const struct wpa_freq_range_list *list);

int int_array_len(const int *a);
void int_array_concat(int **res, const int *a);
void int_array_sort_unique(int *a);
void int_array_add_unique(int **res, int a);

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

void str_clear_free(char *str);
void bin_clear_free(void *bin, size_t len);

int random_mac_addr(u8 *addr);
int random_mac_addr_keep_oui(u8 *addr);

const char * cstr_token(const char *str, const char *delim, const char **last);
char * str_token(char *str, const char *delim, char **context);
size_t utf8_escape(const char *inp, size_t in_size,
		   char *outp, size_t out_size);
size_t utf8_unescape(const char *inp, size_t in_size,
		     char *outp, size_t out_size);
int is_ctrl_char(char c);

int str_starts(const char *str, const char *start);

u8 rssi_to_rcpi(int rssi);
char * get_param(const char *cmd, const char *param);

/*
 * gcc 4.4 ends up generating strict-aliasing warnings about some very common
 * networking socket uses that do not really result in a real problem and
 * cannot be easily avoided with union-based type-punning due to struct
 * definitions including another struct in system header files. To avoid having
 * to fully disable strict-aliasing warnings, provide a mechanism to hide the
 * typecast from aliasing for now. A cleaner solution will hopefully be found
 * in the future to handle these cases.
 */
void * __hide_aliasing_typecast(void *foo);
#define aliasing_hide_typecast(a,t) (t *) __hide_aliasing_typecast((a))
#endif /* COMMON_H */
