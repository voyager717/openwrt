/*
 * Copyright (C) 2016 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define _FILE_OFFSET_BITS 64

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include "libfstools.h"
#include "volume.h"
#include "../boot_param.h"

#include <linux/loop.h>

#define ROOTDEV_OVERLAY_ALIGN	(64ULL * 1024ULL)
#define F2FS_MINSIZE		(100ULL * 1024ULL * 1024ULL)

struct squashfs_super_block {
	uint32_t s_magic;
	uint32_t pad0[9];
	uint64_t bytes_used;
};

struct rootdev_volume {
	struct volume v;
	uint64_t offset;
	char loop_name[32];
	char *dev_path;
};

static const char *rootdev;
static struct driver rootdisk_driver;

static char *get_blockdev(dev_t dev)
{
	const char *dirname = "/dev";
	DIR *dir = opendir(dirname);
	struct dirent *d;
	struct stat st;
	static char buf[256];
	char *ret = NULL;

	if (!dir)
		return ret;

	while ((d = readdir(dir)) != NULL) {
		snprintf(buf, sizeof(buf), "%s/%s", dirname, d->d_name);

		if (lstat(buf, &st) != 0)
			continue;

		if (!S_ISBLK(st.st_mode))
			continue;

		if (st.st_rdev != dev)
			continue;

		ret = buf;
		break;
	}

	closedir(dir);
	return ret;
}

static char *get_rootdev(const char *dir)
{
	struct stat st;

	if (stat(dir, &st))
		return NULL;

	return get_blockdev(S_ISBLK(st.st_mode) ? st.st_rdev : st.st_dev);
}

static int get_squashfs(struct squashfs_super_block *sb)
{
	FILE *f;
	int len;

	f = fopen(rootdev, "r");
	if (!f)
		return -1;

	len = fread(sb, sizeof(*sb), 1, f);
	fclose(f);

	if (len != 1)
		return -1;

	return 0;
}

static bool rootdisk_use_f2fs(struct rootdev_volume *p)
{
	const char *dev = rootdev;
	uint64_t size = 0;
	bool ret = false;
	int fd;

	if (p->dev_path)
		dev = p->dev_path;

	fd = open(dev, O_RDONLY);
	if (ioctl(fd, BLKGETSIZE64, &size) == 0)
		ret = size - p->offset > F2FS_MINSIZE;
	close(fd);

	return ret;
}

static struct volume *find_existed_rootfs_data(void)
{
	struct rootdev_volume *p;
	char *rootfs_data_dev;

	rootfs_data_dev = boot_param_get_dev("rootfs_data_part");

	if (!rootfs_data_dev)
		return NULL;

	ULOG_NOTE("Using existed rootfs_data device %s\n", rootfs_data_dev);

	write_boot_param_string("rootfs_data_part", rootfs_data_dev);

	p = calloc(1, sizeof(*p));
	p->v.drv = &rootdisk_driver;
	p->v.name = "rootfs_data";

	p->offset = 0;
	p->dev_path = rootfs_data_dev;

	return &p->v;
}

static struct volume *rootdisk_volume_find(char *name)
{
	struct squashfs_super_block sb;
	struct rootdev_volume *p;

	if (strcmp(name, "rootfs_data") != 0)
		return NULL;

	if (read_boot_param_bool("no_split_rootfs_data"))
		return find_existed_rootfs_data();

	if (!rootdev)
		rootdev = get_rootdev("/");
	if (!rootdev)
		rootdev = get_rootdev("/rom");
	if (!rootdev)
		return NULL;

	if (strstr(rootdev, "mtdblock") ||
	    strstr(rootdev, "ubiblock"))
		return NULL;

	if (get_squashfs(&sb))
		return NULL;

	if (memcmp(&sb.s_magic, "hsqs", sizeof(sb.s_magic)) != 0)
		return NULL;

	p = calloc(1, sizeof(*p));
	p->v.drv = &rootdisk_driver;
	p->v.name = "rootfs_data";

	p->offset = le64_to_cpu(sb.bytes_used);
	p->offset = ((p->offset + (ROOTDEV_OVERLAY_ALIGN - 1)) &
		     ~(ROOTDEV_OVERLAY_ALIGN - 1));

	return &p->v;
}

static int rootdisk_volume_identify(struct volume *v)
{
	struct rootdev_volume *p = container_of(v, struct rootdev_volume, v);
	const char *dev = rootdev;
	int ret = FS_NONE;
	uint32_t magic = 0;
	size_t n;
	FILE *f;

	if (p->dev_path)
		dev = p->dev_path;

	f = fopen(dev, "r");
	if (!f)
		return ret;

	fseeko(f, p->offset + 0x400, SEEK_SET);
	n = fread(&magic, sizeof(magic), 1, f);
	if (n != 1)
		return -1;

	if (magic == cpu_to_le32(0xF2F52010))
		ret = FS_F2FS;

	magic = 0;
	fseeko(f, p->offset + 0x438, SEEK_SET);
	n = fread(&magic, sizeof(magic), 1, f);
	if (n != 1)
		return -1;
	if ((le32_to_cpu(magic) & 0xffff) == 0xef53)
		ret = FS_EXT4;

	fclose(f);

	return ret;
}

static int rootdisk_create_loop(struct rootdev_volume *p)
{
	struct loop_info64 info;
	int ret = -1;
	int fd = -1;
	int i, ffd;

	ffd = open(rootdev, O_RDWR);
	if (ffd < 0)
		return -1;

	for (i = 0; i < 8; i++) {
		snprintf(p->loop_name, sizeof(p->loop_name), "/dev/loop%d",
			 i);

		if (fd >= 0)
			close(fd);

		fd = open(p->loop_name, O_RDWR);
		if (fd < 0)
			continue;

		if (ioctl(fd, LOOP_GET_STATUS64, &info) == 0) {
			if (strcmp((char *) info.lo_file_name, rootdev) != 0)
				continue;
			if (info.lo_offset != p->offset)
				continue;
			ret = 0;
			break;
		}

		if (errno != ENXIO)
			continue;

		if (ioctl(fd, LOOP_SET_FD, ffd) != 0)
			continue;

		memset(&info, 0, sizeof(info));
		snprintf((char *) info.lo_file_name, sizeof(info.lo_file_name), "%s",
			 rootdev);
		info.lo_offset = p->offset;
		info.lo_flags |= LO_FLAGS_AUTOCLEAR;

		if (ioctl(fd, LOOP_SET_STATUS64, &info) != 0) {
			ioctl(fd, LOOP_CLR_FD, 0);
			continue;
		}

		/*
		 * Don't close fd. Leave it open until this process exits, to avoid
		 * the autoclear from happening too soon.
		 */
		fd = -1;

		ret = 0;
		break;
	}

	if (fd >= 0)
		close(fd);

	close(ffd);

	if (ret)
		p->loop_name[0] = 0;

	return ret;
}

static int rootdisk_volume_init(struct volume *v)
{
	struct rootdev_volume *p = container_of(v, struct rootdev_volume, v);
	char str[128];
	int ret = 0;

	if (p->dev_path) {
		/* Do not create loop device with no_split_rootfs_data set */
		v->type = BLOCKDEV;
		v->blk = p->dev_path;
		goto do_format;
	}

	if (!p->loop_name[0] && rootdisk_create_loop(p) != 0) {
		ULOG_ERR("unable to create loop device\n");
		return -1;
	}

	v->type = BLOCKDEV;
	v->blk = p->loop_name;

do_format:
	switch (rootdisk_volume_identify(v)) {
	case FS_NONE:
		ULOG_INFO("rootdisk overlay filesystem has not been formatted yet\n");
		if (rootdisk_use_f2fs(p))
			snprintf(str, sizeof(str), "mkfs.f2fs -q -l rootfs_data %s", v->blk);
		else
			snprintf(str, sizeof(str), "mkfs.ext4 -q -L rootfs_data %s", v->blk);
		ret = system(str);
		break;
	default:
		break;
	}
	return ret;
}

static struct driver rootdisk_driver = {
	.name = "rootdisk",
	.find = rootdisk_volume_find,
	.init = rootdisk_volume_init,
	.identify = rootdisk_volume_identify,
};

DRIVER(rootdisk_driver);
