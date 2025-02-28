/*
 * wpa_supplicant/hostapd / Internal implementation of OS specific functions
 * Copyright (c) 2005-2006, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 *
 * This file is an example of operating system specific  wrapper functions.
 * This version implements many of the functions internally, so it can be used
 * to fill in missing functions from the target system C libraries.
 *
 * Some of the functions are using standard C library calls in order to keep
 * this file in working condition to allow the functions to be tested on a
 * Linux target. Please note that OS_NO_C_LIB_DEFINES needs to be defined for
 * this file to work correctly. Note that these implementations are only
 * examples and are not optimized for speed.
 */

#include "os.h"

void os_sleep(os_time_t sec, os_time_t usec)
{
	if (sec)
		sleep(sec);
	if (usec)
		usleep(usec);
}


int os_get_time(struct os_time *t)
{
    struct timespec times = {0, 0};

    clock_gettime(CLOCK_MONOTONIC, &times);
    t->sec = times.tv_sec;
    t->usec = times.tv_nsec / 1000;
    return 0;
}

int os_reltime_expired(struct os_time *now,
				     struct os_time *ts,
				     os_time_t timeout_secs)
{
	struct os_time age;

	os_time_sub(now, ts, &age);
	return (age.sec > timeout_secs) ||
	       (age.sec == timeout_secs && age.usec > 0);
}


int os_mktime(int year, int month, int day, int hour, int min, int sec,
	      os_time_t *t)
{
	struct tm tm;

	if (year < 1970 || month < 1 || month > 12 || day < 1 || day > 31 ||
	    hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 ||
	    sec > 60)
		return -1;

	os_memset(&tm, 0, sizeof(tm));
	tm.tm_year = year - 1900;
	tm.tm_mon = month - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = min;
	tm.tm_sec = sec;

	*t = (os_time_t) mktime(&tm);
	if (*t < 0) {
		printf("[%s] mktime fail\n", __func__);
		return -1;
	}
	return 0;
}

int os_gmtime(os_time_t t, struct os_tm *tm)
{
        struct tm *tm2;
        time_t t2 = t;

        tm2 = gmtime(&t2);
        if (tm2 == NULL)
                return -1;
        tm->sec = tm2->tm_sec;
        tm->min = tm2->tm_min;
        tm->hour = tm2->tm_hour;
        tm->day = tm2->tm_mday;
        tm->month = tm2->tm_mon + 1;
        tm->year = tm2->tm_year + 1900;
        return 0;
}

int os_daemonize(const char *pid_file)
{
	int ret = 0;
	if (daemon(0, 0)) {
		perror("daemon");
		return -1;
	}

	if (pid_file) {
		FILE *f = fopen(pid_file, "w");
		if (f) {
			ret = fprintf(f, "%u\n", getpid());
			if (ret < 0) {
				printf("[%s] fprintf fail\n", __func__);
				ret = fclose(f);
				if (ret != 0)
					printf("[%s] fclose fail\n", __func__);
				return -1;
			}
			ret = fclose(f);
			if (ret != 0) {
				printf("[%s] fclose fail\n", __func__);
				return -1;
			}
		}
	}

	return 0;
}


void os_daemonize_terminate(const char *pid_file)
{
	if (pid_file)
		unlink(pid_file);
}


int os_get_random(unsigned char *buf, size_t len)
{
	FILE *f;
	size_t rc;
	int ret = 0;

	f = fopen("/dev/urandom", "rb");
	if (f == NULL) {
		printf("Could not open /dev/urandom.\n");
		return -1;
	}

	rc = fread(buf, 1, len, f);
	ret = fclose(f);
	if (ret != 0) {
		printf("[%s] fclose fail\n", __func__);
		return -1;
	}

	return rc != len ? -1 : 0;
}


unsigned long os_random(void)
{
	static int initialized;
	int fd = 0;
	unsigned long seed = 0;

	if (!initialized) {
		fd = open("/dev/urandom", 0);
		if (fd < 0 || read(fd, &seed, sizeof(seed)) < 0) {
			printf("[%s]:Could not load seed from /dev/urandom: %s\n", __func__, strerror(errno));
			seed = time(0);
		}
		if (fd >= 0)
			close(fd);
		srand(seed);
		initialized++;
	}

	return rand();

}


int os_program_init(void)
{
	return 0;
}


void os_program_deinit(void)
{
}


int os_setenv(const char *name, const char *value, int overwrite)
{
	return setenv(name, value, overwrite);
}


int os_unsetenv(const char *name)
{
#if defined(__FreeBSD__) || defined(__NetBSD__)
	unsetenv(name);
	return 0;
#else
	return unsetenv(name);
#endif
}


char * os_readfile(const char *name, size_t *len)
{
	FILE *f = NULL;
	char *buf = NULL;
	long int ret = 0;
	int tmp = 0;

	f = fopen(name, "rb");
	if (f == NULL)
		return NULL;

	ret = fseek(f, 0, SEEK_END);
	if (ret != 0) {
		printf("[%s] fseek fail\n", __func__);
		goto error;
	}
	ret = ftell(f);
	if(ret < 0) {
		printf("[%s] ftell fail\n", __func__);
		goto error;
	}
	*len = ret;
	ret = fseek(f, 0, SEEK_SET);
	if (ret != 0) {
		printf("[%s] fseek fail\n", __func__);
		goto error;
	}

	buf = os_malloc(*len);
	if (buf == NULL) {
		printf("[%s] os_malloc fail\n", __func__);
		goto error;
	}

	if (fread(buf, 1, *len, f) != *len) {
		printf("[%s] fread fail\n", __func__);
		goto error;
	}

	tmp = fclose(f);
	if (tmp != 0) {
		printf("[%s] fclose fail\n", __func__);
		os_free(buf);
		return NULL;
	}

	return buf;
error:
	if (buf)
		os_free(buf);
	tmp = fclose(f);
	if (tmp != 0)
		printf("[%s] fclose fail\n", __func__);

	return NULL;
}


void * os_zalloc(size_t size)
{
	void *n = os_malloc(size);
	if (n)
		os_memset(n, 0, size);
	else
		printf("%s fail, size:%lu\n", __func__, (unsigned long)size);
	return n;
}

