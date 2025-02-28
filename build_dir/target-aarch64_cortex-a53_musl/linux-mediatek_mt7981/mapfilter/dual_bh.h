/*
 * dual_bh, it's used to support MAP dual backhaul
 *
 * Author: Zheng Zhou <Zheng.Zhou@mediatek.com>
 */
#ifndef _DUAL_BH_H_
#define _DUAL_BH_H_
#include <linux/init.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_bridge.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>
#include <net/netlink.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/kobject.h>
#include <linux/version.h>
#include <linux/jhash.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

void dual_bh_init(void);
void dual_bh_deinit(void);
#endif
