#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef CONFIG_MASK_MACADDR
#define MAC2STR "%02x:**:**:%02x:%02x:%02x"
#define PRINT_MAC(a) a[0],a[3],a[4],a[5]
#define IP2STR "***.%d.%d.%d"
#define PRINT_IP(ip) (((ip) & 0x0000ff00) >> 8), (((ip) & 0x00ff0000) >> 16), (((ip) & 0xff000000) >> 24)
#else
#define MAC2STR "%02x:%02x:%02x:%02x:%02x:%02x"
#define PRINT_MAC(a) a[0], a[1], a[2], a[3], a[4], a[5]
#define IP2STR "%d.%d.%d.%d"
#define PRINT_IP(ip) ((ip) & 0x000000ff), (((ip) & 0x0000ff00) >> 8), (((ip) & 0x00ff0000) >> 16), (((ip) & 0xff000000) >> 24)
#endif

#define DBGPRINT(fmt, args...)   \
{                                       \
	printf("[%s]", __FUNCTION__);	\
    printf( fmt, ## args);          \
}

static inline int snprintf_error(size_t size, int res)
{
	return res < 0 || (unsigned int) res >= size;
}

#endif
