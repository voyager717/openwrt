#
# Copyright (C) 2011 OpenWrt.org
#

SUBTARGET:=rt3883
BOARDNAME:=RT3662/RT3883 based boards
FEATURES+=usb pci small_flash
CPU_TYPE:=74kc

<<<<<<< HEAD
DEFAULT_PACKAGES += kmod-rt2800-pci kmod-rt2800-soc wpad-basic-mbedtls swconfig
=======
DEFAULT_PACKAGES += kmod-rt2800-pci kmod-rt2800-soc wpad-basic-wolfssl swconfig
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build firmware images for Ralink RT3662/RT3883 based boards.
endef

