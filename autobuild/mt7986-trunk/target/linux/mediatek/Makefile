# Copyright (c) 2015 OpenWrt.org
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=mediatek
BOARDNAME:=MediaTek Ralink ARM
SUBTARGETS:=mt7622 mt7623 mt7629 mt7986
FEATURES:=squashfs nand ramdisk fpu dt-overlay

KERNEL_PATCHVER:=5.10
KERNEL_TESTING_PATCHVER:=5.15

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-gpio-button-hotplug

$(eval $(call BuildTarget))
