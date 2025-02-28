# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2009-2015 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=kirkwood
BOARDNAME:=Marvell Kirkwood
<<<<<<< HEAD
FEATURES:=rtc usb nand squashfs ramdisk
CPU_TYPE:=xscale
SUBTARGETS:=generic

KERNEL_PATCHVER:=6.6
=======
FEATURES:=usb nand squashfs ramdisk
CPU_TYPE:=xscale

KERNEL_PATCHVER:=5.4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=zImage dtbs

DEFAULT_PACKAGES += uboot-envtools kmod-usb2

$(eval $(call BuildTarget))
