#
# Copyright (C) 2009 OpenWrt.org
#

SUBTARGET:=mt7620
BOARDNAME:=MT7620 based boards
FEATURES+=usb ramdisk
CPU_TYPE:=24kc

<<<<<<< HEAD
DEFAULT_PACKAGES += kmod-rt2800-soc wpad-basic-mbedtls swconfig
=======
DEFAULT_PACKAGES += kmod-rt2800-soc wpad-basic-wolfssl swconfig
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build firmware images for Ralink MT7620 based boards.
endef

