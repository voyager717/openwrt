# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2013 OpenWrt.org

define Profile/Broadcom-tg3-none
  NAME:=Broadcom SoC, tg3 Ethernet, no WiFi
<<<<<<< HEAD
  PACKAGES:=-wpad-basic-mbedtls kmod-tg3
=======
  PACKAGES:=-wpad-basic-wolfssl kmod-tg3
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Profile/Broadcom-tg3-none/Description
	Package set compatible with hardware Broadcom BCM4705/BCM4785
	SoC without any Wifi cards and tg3 Ethernet driver.
endef
$(eval $(call Profile,Broadcom-tg3-none))

