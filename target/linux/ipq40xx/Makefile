include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=ipq40xx
BOARDNAME:=Qualcomm Atheros IPQ40XX
<<<<<<< HEAD
FEATURES:=squashfs fpu ramdisk
CPU_TYPE:=cortex-a7
CPU_SUBTYPE:=neon-vfpv4
SUBTARGETS:=generic chromium mikrotik

KERNEL_PATCHVER:=6.6
=======
FEATURES:=squashfs fpu ramdisk nand
CPU_TYPE:=cortex-a7
CPU_SUBTYPE:=neon-vfpv4
SUBTARGETS:=generic mikrotik

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

KERNELNAME:=zImage Image dtbs

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-usb-dwc3-qcom \
<<<<<<< HEAD
	kmod-leds-gpio kmod-gpio-button-hotplug \
	kmod-ath10k-ct wpad-basic-mbedtls \
=======
	kmod-leds-gpio kmod-gpio-button-hotplug swconfig \
	kmod-ath10k-ct wpad-basic-wolfssl \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	kmod-usb3 kmod-usb-dwc3 ath10k-firmware-qca4019-ct \
	uboot-envtools

$(eval $(call BuildTarget))
