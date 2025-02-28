# Copyright (c) 2013 The Linux Foundation. All rights reserved.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=ipq806x
BOARDNAME:=Qualcomm Atheros IPQ806X
<<<<<<< HEAD
FEATURES:=squashfs fpu ramdisk
CPU_TYPE:=cortex-a15
CPU_SUBTYPE:=neon-vfpv4
SUBTARGETS:=generic chromium

KERNEL_PATCHVER:=6.6
=======
FEATURES:=squashfs nand fpu ramdisk
CPU_TYPE:=cortex-a15
CPU_SUBTYPE:=neon-vfpv4
SUBTARGETS:=generic

KERNEL_PATCHVER:=5.4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

KERNELNAME:=zImage Image dtbs

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
<<<<<<< HEAD
	kmod-leds-gpio kmod-gpio-button-hotplug \
	kmod-ata-ahci kmod-ata-ahci-platform \
	kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport \
	kmod-phy-qcom-ipq806x-usb kmod-usb3 kmod-usb-dwc3-qcom \
	kmod-ath10k-ct wpad-basic-mbedtls \
=======
	kmod-leds-gpio kmod-gpio-button-hotplug swconfig \
	kmod-ata-ahci kmod-ata-ahci-platform \
	kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport \
	kmod-phy-qcom-ipq806x-usb kmod-usb3 kmod-usb-dwc3-qcom \
	kmod-ath10k-ct wpad-basic-wolfssl \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	uboot-envtools

$(eval $(call BuildTarget))
