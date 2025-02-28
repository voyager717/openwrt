# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2007-2013 OpenWrt.org

define Profile/Broadcom-b44-b43
  NAME:=Broadcom SoC, b44 Ethernet, BCM43xx WiFi (b43, default)
<<<<<<< HEAD
  PACKAGES:=kmod-b44 kmod-b43
=======
  PACKAGES:=kmod-b44 kmod-b43 kmod-b43legacy
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Profile/Broadcom-b44-b43/Description
	Package set compatible with hardware older Broadcom BCM47xx or BCM535x
	SoC with Broadcom BCM43xx Wifi cards using the mac80211, b43 and
<<<<<<< HEAD
	b44 Ethernet driver.
=======
	b43legacy drivers and b44 Ethernet driver.
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call Profile,Broadcom-b44-b43))

