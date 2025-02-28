/*
 * wpa_supplicant/hostapd / Debug prints
 * Copyright (c) 2002-2013, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "util.h"

#ifdef CONFIG_DEBUG_SYSLOG
#include <syslog.h>

int wpa_debug_syslog = 0;
#endif /* CONFIG_DEBUG_SYSLOG */

int wpa_debug_show_keys = 1;
extern int RTDebugLevel;

#if defined(DPP_AUTOTEST) || defined(MAP_R3)
void hex_str(unsigned char *inchar, unsigned int len, unsigned char *outtxt)
{
	unsigned char hbit,lbit;
	unsigned int i;
	for(i=0;i<len;i++)
	{
		hbit = (*(inchar+i)&0xf0)>>4;
		lbit = *(inchar+i)&0x0f;
		if (hbit>9)
			outtxt[2*i]='a'+hbit-10;
		else
			outtxt[2*i]='0'+hbit;

		if (lbit>9)
			outtxt[2*i+1]='a'+lbit-10;
		else
			outtxt[2*i+1]='0'+lbit;
	}
	outtxt[2*i] = 0;
}
#endif /* DPP_AUTOTEST || MAP_R3 */

/*
write key message into dppc_config.txt
*/
void wpa_write_config_file(char *filename, char *buf, int len) {
	FILE *pf;
	int ret = 0;

	pf = fopen(filename, "a+");
	if (!pf)
		return;

	ret = fseek(pf, 0, SEEK_END);
	if (ret != 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] fseek fail\n", __func__);
		goto error;
	}
	ret = fprintf(pf, "%s", buf);
	if (ret < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s] fprintf fail\n", __func__);
		goto error;
	}
	ret = fflush(pf);
	if (ret != 0)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] fflush fail\n", __func__);
error:
	ret = fclose(pf);
	if (ret != 0)
		DBGPRINT(RT_DEBUG_ERROR, "[%s] fclose fail\n", __func__);
	return;
}

/*
 * wpa_printf - conditional printf
 * @level: priority level (MSG_*) of the message
 * @fmt: printf format string, followed by optional arguments
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration.
 *
 * Note: New line '\n' is added to the end of the text when printing to stdout.
 */
void wpa_printf(int level, const char *fmt, ...)
{
	if (level > RTDebugLevel)
		return;
	va_list ap;

	va_start(ap, fmt);
		vprintf(fmt, ap);
		printf("\n");
	va_end(ap);
}

static void _wpa_hexdump(int level, const char *title, const u8 *buf,
			 size_t len, int show)
{
	size_t i;

	if (level > RTDebugLevel)
		return;
	printf("%s - hexdump(len=%lu):", title, (unsigned long) len);
	if (buf == NULL) {
		printf(" [NULL]");
	} else if (show) {
		for (i = 0; i < len; i++)
			printf(" %02x", buf[i]);
	} else {
		printf(" [REMOVED]");
	}
	printf("\n");
}

void wpa_hexdump(int level, const char *title, const void *buf, size_t len)
{
	_wpa_hexdump(level, title, buf, len, 1);
}


void wpa_hexdump_key(int level, const char *title, const void *buf, size_t len)
{
	_wpa_hexdump(level, title, buf, len, wpa_debug_show_keys);
}


static void _wpa_hexdump_ascii(int level, const char *title, const void *buf,
			       size_t len, int show)
{
	size_t i, llen;
	const u8 *pos = buf;
	const size_t line_len = 16;

	if (level > RTDebugLevel)
		return;
	if (buf == NULL) {
		printf("%s - hexdump_ascii(len=%lu): [NULL]\n",
		       title, (unsigned long) len);
		return;
	}
	printf("%s - hexdump_ascii(len=%lu):\n", title, (unsigned long) len);
	while (len) {
		llen = len > line_len ? line_len : len;
		printf("    ");
		for (i = 0; i < llen; i++)
			printf(" %02x", pos[i]);
		for (i = llen; i < line_len; i++)
			printf("   ");
		printf("   ");
		for (i = 0; i < llen; i++) {
			if (isprint(pos[i]))
				printf("%c", pos[i]);
			else
				printf("_");
		}
		for (i = llen; i < line_len; i++)
			printf(" ");
		printf("\n");
		pos += llen;
		len -= llen;
	}
}


void wpa_hexdump_ascii(int level, const char *title, const void *buf,
		       size_t len)
{
	_wpa_hexdump_ascii(level, title, buf, len, 1);
}


void wpa_hexdump_ascii_key(int level, const char *title, const void *buf,
			   size_t len)
{
	_wpa_hexdump_ascii(level, title, buf, len, wpa_debug_show_keys);
}


void wpa_dump(int level, const char *title, const unsigned int *buf, size_t len)
{
	size_t i;

	if (level > RTDebugLevel)
		return;
	printf("%s - hexdump(len=%lu):", title, (unsigned long) len);
	if (buf == NULL) {
		printf(" [NULL]");
	} else {
		for (i = 0; i < len; i++)
			printf(" %d", buf[i]);
	}
	printf("\n");
}


