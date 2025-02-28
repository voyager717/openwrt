# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2010 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=powerpc
BOARD:=mpc85xx
BOARDNAME:=Freescale MPC85xx
<<<<<<< HEAD
CPU_TYPE:=8548
FEATURES:=squashfs ramdisk nand
SUBTARGETS:=p1010 p1020 p2020

KERNEL_PATCHVER:=6.6
=======
CPU_TYPE:=8540
FEATURES:=squashfs ramdisk
SUBTARGETS:=p1010 p1020 p2020

KERNEL_PATCHVER:=5.4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

KERNELNAME:=zImage

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	kmod-input-core kmod-input-gpio-keys kmod-button-hotplug \
<<<<<<< HEAD
	kmod-leds-gpio swconfig kmod-ath9k wpad-basic-mbedtls kmod-usb2 \
	uboot-envtools kmod-crypto-hw-talitos
=======
	kmod-leds-gpio swconfig kmod-ath9k wpad-basic-wolfssl kmod-usb2
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

$(eval $(call BuildTarget))
