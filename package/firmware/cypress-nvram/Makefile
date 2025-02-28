<<<<<<< HEAD
# SPDX-License-Identifier: GPL-2.0-or-later
=======
#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(TOPDIR)/rules.mk

PKG_NAME:=cypress-nvram
<<<<<<< HEAD
PKG_RELEASE:=7

PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>

=======
PKG_SOURCE_DATE:=2019-09-03
PKG_SOURCE_VERSION:=e7b78df22f2a0c5f56abb7b5880661611de35e5f
PKG_MIRROR_HASH:=1cb20a749696852be0a512d51961365dd9c031362af0af1a2b9f5a3fb894885f
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/openwrt/cypress-nvram.git

PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>

PKG_FLAGS:=nonshared

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
include $(INCLUDE_DIR)/package.mk

define Package/cypress-nvram-default
  SECTION:=firmware
  CATEGORY:=Firmware
<<<<<<< HEAD
=======
  URL:=https://community.cypress.com/community/linux
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Build/Compile
	true
endef

<<<<<<< HEAD
# Cypress 4339 SDIO NVRAM
define Package/cypress-nvram-4339-sdio
  $(Package/cypress-nvram-default)
  TITLE:=BCM4339 SDIO NVRAM
  CONFLICTS:=brcmfmac-nvram-4339-sdio
endef

define Package/cypress-nvram-4339-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		./files/brcmfmac4339-sdio.AP6335.txt \
		$(1)/lib/firmware/brcm/
	$(LN) \
		brcmfmac4339-sdio.AP6335.txt \
		$(1)/lib/firmware/brcm/brcmfmac4339-sdio.technexion,imx7d-pico-pi.txt
endef

$(eval $(call BuildPackage,cypress-nvram-4339-sdio))
=======
# Cypress 43430 SDIO Raspberry Pi 3B NVRAM
define Package/cypress-nvram-43430-sdio-rpi-3b
  $(Package/cypress-nvram-default)
  TITLE:=CYW43430 NVRAM for Raspberry Pi 3B
  DEPENDS:=@TARGET_bcm27xx
  PROVIDES:=brcmfmac-firmware-43430-sdio-rpi-3b
  CONFLICTS:=brcmfmac-firmware-43430-sdio-rpi-3b
endef

define Package/cypress-nvram-43430-sdio-rpi-3b/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcmfmac43430-sdio.raspberrypi,3-model-b.txt \
		$(1)/lib/firmware/brcm/brcmfmac43430-sdio.raspberrypi,3-model-b.txt
endef

$(eval $(call BuildPackage,cypress-nvram-43430-sdio-rpi-3b))

# Cypress 43430 SDIO Raspberry Pi Zero W NVRAM
define Package/cypress-nvram-43430-sdio-rpi-zero-w
  $(Package/cypress-nvram-default)
  TITLE:=CYW43430 NVRAM for Raspberry Pi Zero W
  DEPENDS:=@TARGET_bcm27xx
  PROVIDES:=brcmfmac-firmware-43430-sdio-rpi-zero-w
  CONFLICTS:=brcmfmac-firmware-43430-sdio-rpi-zero-w
endef

define Package/cypress-nvram-43430-sdio-rpi-zero-w/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcmfmac43430-sdio.raspberrypi,model-zero-w.txt \
		$(1)/lib/firmware/brcm/brcmfmac43430-sdio.raspberrypi,model-zero-w.txt
endef

$(eval $(call BuildPackage,cypress-nvram-43430-sdio-rpi-zero-w))

# Cypress 43455 SDIO Raspberry Pi 3B+ NVRAM
define Package/cypress-nvram-43455-sdio-rpi-3b-plus
  $(Package/cypress-nvram-default)
  TITLE:=CYW43455 NVRAM for Raspberry Pi 3B+
  DEPENDS:=@TARGET_bcm27xx
  PROVIDES:=brcmfmac-firmware-43455-sdio-rpi-3b-plus
  CONFLICTS:=brcmfmac-firmware-43455-sdio-rpi-3b-plus
endef

define Package/cypress-nvram-43455-sdio-rpi-3b-plus/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcmfmac43455-sdio.raspberrypi,3-model-b-plus.txt \
		$(1)/lib/firmware/brcm/brcmfmac43455-sdio.raspberrypi,3-model-b-plus.txt
endef

$(eval $(call BuildPackage,cypress-nvram-43455-sdio-rpi-3b-plus))

# Cypress 43455 SDIO Raspberry Pi 4B NVRAM
define Package/cypress-nvram-43455-sdio-rpi-4b
  $(Package/cypress-nvram-default)
  TITLE:=CYW43455 NVRAM for Raspberry Pi 4B
  DEPENDS:=@TARGET_bcm27xx
  PROVIDES:=brcmfmac-firmware-43455-sdio-rpi-4b
  CONFLICTS:=brcmfmac-firmware-43455-sdio-rpi-4b
endef

define Package/cypress-nvram-43455-sdio-rpi-4b/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcmfmac43455-sdio.raspberrypi,4-model-b.txt \
		$(1)/lib/firmware/brcm/brcmfmac43455-sdio.raspberrypi,4-model-b.txt
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcmfmac43455-sdio.raspberrypi,4-model-b.txt \
		$(1)/lib/firmware/brcm/brcmfmac43455-sdio.raspberrypi,4-compute-module.txt
endef

$(eval $(call BuildPackage,cypress-nvram-43455-sdio-rpi-4b))
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
