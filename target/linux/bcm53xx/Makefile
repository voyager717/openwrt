# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=bcm53xx
BOARDNAME:=Broadcom BCM47xx/53xx (ARM)
FEATURES:=squashfs nand usb pci pcie gpio pwm
CPU_TYPE:=cortex-a9
SUBTARGETS:=generic

<<<<<<< HEAD
KERNEL_PATCHVER:=6.6
=======
KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build firmware images for Broadcom based BCM47xx/53xx routers with ARM CPU, *not* MIPS.
endef

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=zImage dtbs

<<<<<<< HEAD
DEFAULT_PACKAGES += nvram \
=======
DEFAULT_PACKAGES += swconfig nvram \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	osafeloader oseama otrx \
	kmod-gpio-button-hotplug \
	kmod-leds-gpio

$(eval $(call BuildTarget))
