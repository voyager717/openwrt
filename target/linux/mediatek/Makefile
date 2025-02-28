# Copyright (c) 2015 OpenWrt.org
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=mediatek
<<<<<<< HEAD
BOARDNAME:=MediaTek ARM
SUBTARGETS:=filogic mt7622 mt7623 mt7629
FEATURES:=dt-overlay emmc fpu gpio nand pci pcie rootfs-part separate_ramdisk squashfs usb

KERNEL_PATCHVER:=6.6
=======
BOARDNAME:=MediaTek Ralink ARM
SUBTARGETS:=mt7622 mt7623 mt7629 mt7986 mt7981
FEATURES:=squashfs nand ramdisk fpu

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-gpio-button-hotplug

$(eval $(call BuildTarget))
