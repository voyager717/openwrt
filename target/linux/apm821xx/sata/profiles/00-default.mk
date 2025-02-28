# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2011 OpenWrt.org

define Profile/Default
  NAME:=Default Profile
  PRIORITY:=1
<<<<<<< HEAD
  PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport kmod-usb-storage kmod-fs-vfat wpad-basic-mbedtls
=======
  PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport kmod-usb-storage kmod-fs-vfat wpad-basic-wolfssl
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Profile/Default/Description
	Default package set
endef

$(eval $(call Profile,Default))
