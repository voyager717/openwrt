/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <glob.h>
#include <dlfcn.h>

#include <blkid/blkid.h>
#include <libubox/ulog.h>
#include "boot_param.h"

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

#define BOOT_PARAM_STR_MAX_LEN			256

static struct {
	bool loaded;
	blkid_probe (*new_probe_from_filename)(const char *);
	int (*do_safeprobe)(blkid_probe);
	int (*probe_lookup_value)(blkid_probe, const char *, const char **, size_t *);
	void (*free_probe)(blkid_probe);
	int (*probe_enable_partitions)(blkid_probe, int);
	int (*probe_set_partitions_flags)(blkid_probe, int);
} libblkid = {};

bool read_boot_param_bool(const char *name)
{
	char path[BOOT_PARAM_STR_MAX_LEN], val;
	size_t len;
	FILE *f;

	snprintf(path, sizeof(path), "/sys/module/boot_param/parameters/%s",
		 name);

	f = fopen(path, "rb");
	if (!f)
		return false;

	len = fread(&val, 1, 1, f);
	fclose(f);

	if (len != 1)
		return false;

	return val == 'Y';
}

int read_boot_param_string(const char *name, char *val, size_t maxsize)
{
	char path[BOOT_PARAM_STR_MAX_LEN];
	size_t len;
	FILE *f;

	snprintf(path, sizeof(path), "/sys/module/boot_param/parameters/%s",
		 name);

	f = fopen(path, "rb");
	if (!f) {
		val[0] = 0;
		return -1;
	}

	len = fread(val, 1, maxsize, f);
	fclose(f);

	while (len > 0) {
		if (val[len - 1] != '\n' && val[len - 1] != '\r')
			break;

		len--;
	}

	if (len < maxsize)
		val[len] = 0;

	return len;
}

int write_boot_param_string(const char *name, const char *val)
{
	size_t wlen, len = strlen(val);
	char path[BOOT_PARAM_STR_MAX_LEN];
	FILE *f;

	if (len >= BOOT_PARAM_STR_MAX_LEN)
		return -1;

	snprintf(path, sizeof(path), "/sys/module/boot_param/parameters/%s",
		 name);

	f = fopen(path, "wb");
	if (!f)
		return -1;

	wlen = fwrite(val, 1, len, f);
	fclose(f);

	return wlen;
}

static bool load_libblkid(void)
{
	void *lib;

	if (libblkid.loaded)
		return true;

	lib = dlopen("libblkid.so", RTLD_GLOBAL);

	if (!lib)
		lib = dlopen("libblkid.so.1", RTLD_GLOBAL);

	if (!lib)
		return false;

	libblkid.new_probe_from_filename = dlsym(lib, "blkid_new_probe_from_filename");
	if (!libblkid.new_probe_from_filename)
		return false;

	libblkid.do_safeprobe = dlsym(lib, "blkid_do_safeprobe");
	if (!libblkid.do_safeprobe)
		return false;

	libblkid.probe_lookup_value = dlsym(lib, "blkid_probe_lookup_value");
	if (!libblkid.probe_lookup_value)
		return false;

	libblkid.free_probe = dlsym(lib, "blkid_free_probe");
	if (!libblkid.free_probe)
		return false;

	libblkid.probe_enable_partitions = dlsym(lib, "blkid_probe_enable_partitions");
	if (!libblkid.probe_enable_partitions)
		return false;

	libblkid.probe_set_partitions_flags = dlsym(lib, "blkid_probe_set_partitions_flags");
	if (!libblkid.probe_set_partitions_flags)
		return false;

	libblkid.loaded = true;
	return true;
}

static char *lookup_block_dev(const char *path, const char *key, bool is_uuid)
{
	int gl_flags = GLOB_NOESCAPE | GLOB_MARK;
	const char *type, *value;
	char *result = NULL;
	size_t len;
	glob_t gl;
	int i;

	if (glob(path, gl_flags, NULL, &gl) < 0)
		return NULL;

	type = is_uuid ? "PART_ENTRY_UUID" : "PART_ENTRY_NAME";

	for (i = 0; i < gl.gl_pathc; i++) {
		blkid_probe pr = libblkid.new_probe_from_filename(gl.gl_pathv[i]);
		if (!pr)
			continue;

		libblkid.probe_enable_partitions(pr, 1);
		libblkid.probe_set_partitions_flags(pr, BLKID_PARTS_ENTRY_DETAILS);

		if (libblkid.do_safeprobe(pr))
			goto free_pr;

		if (!libblkid.probe_lookup_value(pr, type, &value, &len)) {
			if (!strcmp(value, key))
				result = strdup(gl.gl_pathv[i]);
		}

	free_pr:
		libblkid.free_probe(pr);

		if (result)
			break;
	}

	globfree(&gl);

	return result;
}

static char *find_block_dev(const char *key, bool is_uuid)
{
	char *devpath = NULL;
	int i;

	static const char *block_pats[] = {
		"/dev/loop*",
		"/dev/mmcblk*",
		"/dev/sd*",
		"/dev/hd*",
		"/dev/md*",
		"/dev/nvme*",
		"/dev/vd*",
		"/dev/xvd*",
		"/dev/mapper/*",
	};

	if (!load_libblkid())
		return NULL;

	for (i = 0; i < ARRAY_SIZE(block_pats); i++) {
		devpath = lookup_block_dev(block_pats[i], key, is_uuid);
		if (devpath)
			break;
	}

	return devpath;
}

char *blockdev_parse(const char *name)
{
	char *e, *part_dev_path;
	struct stat st;

	if (!name)
		return NULL;

	e = strchr(name, '=');
	if (e) {
		*e = 0;
		e++;
	}

	if (!e) {
		if (stat(name, &st))
			return NULL;

		if (!S_ISBLK(st.st_mode))
			return NULL;

		part_dev_path = strdup(name);
	} else if (!strcmp(name, "PARTLABEL")) {
		part_dev_path = find_block_dev(e, false);
	} else if (!strcmp(name, "PARTUUID")) {
		if (strlen(e) != 36)
			return NULL;
		part_dev_path = find_block_dev(e, true);
	} else {
		return NULL;
	}

	return part_dev_path;
}

char *boot_param_get_dev(const char *name)
{
	char partkey[BOOT_PARAM_STR_MAX_LEN];

	read_boot_param_string(name, partkey, sizeof(partkey));

	if (!partkey[0])
		return NULL;

	return blockdev_parse(partkey);
}
