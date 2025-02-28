# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2012-2020 OpenWrt.org
# Copyright (C) 2017 LEDE project

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=bcm27xx
BOARDNAME:=Broadcom BCM27xx
<<<<<<< HEAD
FEATURES:=audio boot-part display ext4 fpu gpio rootfs-part rtc squashfs usb usbgadget
SUBTARGETS:=bcm2708 bcm2709 bcm2710 bcm2711 bcm2712

KERNEL_PATCHVER:=6.6
=======
FEATURES:=ext4 audio usb usbgadget display gpio fpu squashfs rootfs-part boot-part
SUBTARGETS:=bcm2708 bcm2709 bcm2710 bcm2711

KERNEL_PATCHVER=5.4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build firmware image for Broadcom BCM27xx SoC devices.
	Currently produces SD Card image for Raspberry Pi.
endef

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES := $(filter-out urngd,$(DEFAULT_PACKAGES))
DEFAULT_PACKAGES += \
<<<<<<< HEAD
	bcm27xx-gpu-fw bcm27xx-utils \
=======
	bcm27xx-gpu-fw \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	kmod-usb-hid \
	kmod-sound-core kmod-sound-arm-bcm2835 \
	kmod-fs-vfat kmod-nls-cp437 kmod-nls-iso8859-1 \
	partx-utils mkf2fs e2fsprogs

KERNELNAME:=Image dtbs

$(eval $(call BuildTarget))
