# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2014 OpenWrt.org

define Profile/Broadcom-mips74k-none
  NAME:=Broadcom SoC, No WiFi
<<<<<<< HEAD
  PACKAGES:=-wpad-basic-mbedtls
=======
  PACKAGES:=-wpad-basic-wolfssl
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Profile/Broadcom-mips74k-none/Description
	Package set for devices without a WiFi.
endef

$(eval $(call Profile,Broadcom-mips74k-none))

