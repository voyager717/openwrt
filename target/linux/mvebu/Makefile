# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2012-2015 OpenWrt.org

include $(TOPDIR)/rules.mk

BOARD:=mvebu
BOARDNAME:=Marvell EBU Armada
<<<<<<< HEAD
FEATURES:=fpu usb pci pcie gpio nand squashfs ramdisk boot-part rootfs-part legacy-sdcard targz
SUBTARGETS:=cortexa9 cortexa53 cortexa72

KERNEL_PATCHVER:=6.6
=======
FEATURES:=fpu usb pci pcie gpio nand squashfs ramdisk boot-part rootfs-part
SUBTARGETS:=cortexa9 cortexa53 cortexa72

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += uboot-envtools kmod-gpio-button-hotplug

$(eval $(call BuildTarget))
