# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2013 OpenWrt.org

define Profile/Broadcom-b44-none
  NAME:=Broadcom SoC, b44 Ethernet, No WiFi
<<<<<<< HEAD
  PACKAGES:=-wpad-basic-mbedtls kmod-b44
=======
  PACKAGES:=-wpad-basic-wolfssl kmod-b44
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Profile/Broadcom-b44-none/Description
	Package set compatible with hardware older Broadcom BCM47xx or BCM535x
	SoC without any Wifi cards and b44 Ethernet driver.
endef
$(eval $(call Profile,Broadcom-b44-none))

