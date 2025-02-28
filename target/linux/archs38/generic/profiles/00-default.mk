# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2016 OpenWrt.org

define Profile/Default
	NAME:=Default Profile (all drivers)
<<<<<<< HEAD
	PACKAGES:= kmod-usb2 kmod-ath9k-htc wpad-basic-mbedtls
=======
	PACKAGES:= kmod-usb2 kmod-ath9k-htc wpad-basic-wolfssl
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Profile/Default/Description
	Default package set compatible with most boards.
endef
$(eval $(call Profile,Default))
