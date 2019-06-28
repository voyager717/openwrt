/*
 *  MTD splitter for MikroTik NOR devices
 *
 *  Copyright (C) 2017 Thibaut VARENE <varenet@parisc-linux.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  The rootfs is expected at erase-block boundary due to the use of
 *  mtd_find_rootfs_from(). We use a trimmed down version of the yaffs header
 *  for two main reasons:
 *  - the original header uses weakly defined types (int, enum...) which can
 *    vary in length depending on build host (and the struct is not packed),
 *    and the name field can have a different total length depending on
 *    whether or not the yaffs code was _built_ with unicode support.
 *  - the only field that could be of real use here (file_size_low) contains
 *    invalid data in the header generated by kernel2minor, so we cannot use
 *    it to infer the exact position of the rootfs and do away with
 *    mtd_find_rootfs_from() (and thus have non-EB-aligned rootfs).
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/string.h>
#include <linux/of.h>

#include "mtdsplit.h"

#define YAFFS_OBJECT_TYPE_FILE	0x1
#define YAFFS_OBJECTID_ROOT	0x1
#define YAFFS_SUM_UNUSED	0xFFFF
#define YAFFS_NAME		"kernel"

#define MINOR_NR_PARTS		2

/*
 * This structure is based on yaffs_obj_hdr from yaffs_guts.h
 * The weak types match upstream. The fields have cpu-endianness
 */
struct minor_header {
	int yaffs_type;
	int yaffs_obj_id;
	u16 yaffs_sum_unused;
	char yaffs_name[sizeof(YAFFS_NAME)];
};

static int mtdsplit_parse_minor(struct mtd_info *master,
				const struct mtd_partition **pparts,
				struct mtd_part_parser_data *data)
{
	struct minor_header hdr;
	size_t hdr_len, retlen;
	size_t rootfs_offset;
	struct mtd_partition *parts;
	int err;

	hdr_len = sizeof(hdr);
	err = mtd_read(master, 0, hdr_len, &retlen, (void *) &hdr);
	if (err)
		return err;

	if (retlen != hdr_len)
		return -EIO;

	/* match header */
	if (hdr.yaffs_type != YAFFS_OBJECT_TYPE_FILE)
		return -EINVAL;

	if (hdr.yaffs_obj_id != YAFFS_OBJECTID_ROOT)
		return -EINVAL;

	if (hdr.yaffs_sum_unused != YAFFS_SUM_UNUSED)
		return -EINVAL;

	if (memcmp(hdr.yaffs_name, YAFFS_NAME, sizeof(YAFFS_NAME)))
		return -EINVAL;

	err = mtd_find_rootfs_from(master, master->erasesize, master->size,
				   &rootfs_offset, NULL);
	if (err)
		return err;

	parts = kzalloc(MINOR_NR_PARTS * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	parts[0].name = KERNEL_PART_NAME;
	parts[0].offset = 0;
	parts[0].size = rootfs_offset;

	parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = master->size - rootfs_offset;

	*pparts = parts;
	return MINOR_NR_PARTS;
}

static const struct of_device_id mtdsplit_minor_of_match_table[] = {
	{ .compatible = "mikrotik,minor" },
	{},
};
MODULE_DEVICE_TABLE(of, mtdsplit_minor_of_match_table);

static struct mtd_part_parser mtdsplit_minor_parser = {
	.owner = THIS_MODULE,
	.name = "minor-fw",
	.of_match_table = mtdsplit_minor_of_match_table,
	.parse_fn = mtdsplit_parse_minor,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int __init mtdsplit_minor_init(void)
{
	register_mtd_parser(&mtdsplit_minor_parser);

	return 0;
}

subsys_initcall(mtdsplit_minor_init);
