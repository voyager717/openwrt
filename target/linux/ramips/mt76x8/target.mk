#
# Copyright (C) 2009 OpenWrt.org
#

SUBTARGET:=mt76x8
BOARDNAME:=MT76x8 based boards
<<<<<<< HEAD
FEATURES+=usb ramdisk small_flash
CPU_TYPE:=24kc

DEFAULT_PACKAGES += kmod-mt7603 wpad-basic-mbedtls swconfig
=======
FEATURES+=usb ramdisk
CPU_TYPE:=24kc

DEFAULT_PACKAGES += kmod-mt7603 wpad-basic-wolfssl swconfig
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build firmware images for Ralink MT76x8 based boards.
endef

