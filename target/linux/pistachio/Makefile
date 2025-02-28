# SPDX-License-Identifier: GPL-2.0-only
#
<<<<<<< HEAD
# Copyright (C) 2017-2021 OpenWrt.org
=======
# Copyright (C) 2017 OpenWrt.org
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=pistachio
BOARDNAME:=MIPS pistachio
FEATURES:=fpu usb usbgadget squashfs targz nand
CPU_TYPE:=24kc
CPU_SUBTYPE:=24kf
<<<<<<< HEAD
SUBTARGETS:=generic

KERNEL_PATCHVER:=6.6
=======

KERNEL_PATCHVER:=5.4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=vmlinux dtbs

DEFAULT_PACKAGES += \
	kmod-gpio-button-hotplug \
	uboot-envtools

$(eval $(call BuildTarget))
