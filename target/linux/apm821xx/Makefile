# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

ARCH:=powerpc
BOARD:=apm821xx
BOARDNAME:=AppliedMicro APM821xx
CPU_TYPE:=464fp
FEATURES:=fpu dt gpio ramdisk squashfs usb
SUBTARGETS:=nand sata

<<<<<<< HEAD
KERNEL_PATCHVER:=6.6
=======
KERNEL_PATCHVER:=5.4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build images for AppliedMicro APM821xx based boards.
endef

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=uImage

DEFAULT_PACKAGES += \
<<<<<<< HEAD
	kmod-leds-gpio kmod-i2c-core kmod-gpio-button-hotplug uboot-envtools \
	kmod-hw-crypto-4xx
=======
	kmod-leds-gpio kmod-i2c-core kmod-gpio-button-hotplug
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

$(eval $(call BuildTarget))
