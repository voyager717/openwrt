// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2020 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <getopt.h>
#include <fcntl.h>
#include <errno.h>

#include "ksmbdtools.h"
#include "version.h"

static void usage(void)
{
	fprintf(stderr, "Usage: ksmbd.control\n");
	fprintf(stderr, "\t-s | --shutdown\n");
	fprintf(stderr, "\t-d | --debug=all or [smb, auth, etc...]\n");
	fprintf(stderr, "\t-c | --ksmbd-version\n");
	fprintf(stderr, "\t-V | --version\n");

	exit(EXIT_FAILURE);
}

static const struct option opts[] = {
	{"shutdown",		no_argument,		NULL,	's' },
	{"debug",		required_argument,	NULL,	'd' },
	{"ksmbd-version",	no_argument,		NULL,	'c' },
	{"version",		no_argument,		NULL,	'V' },
	{NULL,			0,			NULL,	 0  }
};

static void show_version(void)
{
	printf("ksmbd-tools version : %s\n", KSMBD_TOOLS_VERSION);
	exit(EXIT_FAILURE);
}

static int ksmbd_control_shutdown(void)
{
	int fd, ret;

	terminate_ksmbd_daemon();

	fd = open("/sys/class/ksmbd-control/kill_server", O_WRONLY);
	if (fd < 0) {
		pr_err("open failed: %d\n", errno);
		return EXIT_FAILURE;
	}

	ret = write(fd, "hard", 4);
	close(fd);
	return ret != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int ksmbd_control_show_version(void)
{
	int fd, ret;
	char ver[255] = {0};

	fd = open("/sys/module/ksmbd/version", O_RDONLY);
	if (fd < 0) {
		pr_err("open failed: %d\n", errno);
		return EXIT_FAILURE;
	}

	ret = read(fd, ver, 255);
	close(fd);
	if (ret != -1)
		pr_info("ksmbd version : %s\n", ver);
	return ret != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int ksmbd_control_debug(char *comp)
{
	int fd, ret;
	char buf[255] = {0};

	fd = open("/sys/class/ksmbd-control/debug", O_RDWR);
	if (fd < 0) {
		pr_err("open failed: %d\n", errno);
		return EXIT_FAILURE;
	}

	ret = write(fd, comp, strlen(comp));
	if (ret < 0)
		goto out;
	ret = read(fd, buf, 255);
	if (ret < 0)
		goto out;

	pr_info("%s\n", buf);
out:
	close(fd);
	return ret != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	int c;

	set_logger_app_name("ksmbd.control");

	if (getuid() != 0) {
		pr_err("Please try it as root.\n");
		return ret;
	}

	opterr = 0;
	while ((c = getopt_long(argc, argv, "sd:cVh", opts, NULL)) != EOF)
		switch (c) {
		case 's':
			ret = ksmbd_control_shutdown();
			break;
		case 'd':
			ret = ksmbd_control_debug(optarg);
			break;
		case 'c':
			ret = ksmbd_control_show_version();
			break;
		case 'V':
			show_version();
			break;
		case '?':
		case 'h':
		default:
			usage();
	}

	if (argc < 2)
		usage();

	return ret;
}
