// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _BOOT_PARAM_H_
#define _BOOT_PARAM_H_

#include <stddef.h>
#include <stdbool.h>

bool read_boot_param_bool(const char *name);
int read_boot_param_string(const char *name, char *val, size_t maxsize);
int write_boot_param_string(const char *name, const char *val);

char *blockdev_parse(const char *name);
char *boot_param_get_dev(const char *name);

#endif /* _BOOT_PARAM_H_ */
