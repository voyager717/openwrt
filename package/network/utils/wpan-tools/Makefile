<<<<<<< HEAD
# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2015 OpenWrt.org
#
=======
#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(TOPDIR)/rules.mk

PKG_NAME:=wpan-tools
<<<<<<< HEAD
PKG_VERSION:=0.9
PKG_RELEASE=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/linux-wpan/wpan-tools/releases/download/$(PKG_NAME)-$(PKG_VERSION)/$(PKG_NAME)-$(PKG_VERSION).tar.gz?
PKG_HASH:=fa76d9c1874220e4b1f91c226f42baf1e372ea8ccf4b892effaf0d164448f608
=======
PKG_VERSION:=0.7

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://wpan.cakelab.org/releases/
PKG_HASH:=c16de9d7861c2d9b6a4436a0fac730f9f545ee290b92bc770c538ec6a3f22309
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/package.mk

define Package/wpan-tools
  SECTION:=net
  CATEGORY:=Network
  TITLE:=cfg802154 interface configuration utility
<<<<<<< HEAD
  URL:=https://linux-wpan.org/wpan-tools.html
=======
  URL:=http://wpan.cakelab.org/
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  DEPENDS:= +libnl
endef

define Package/wpan-tools/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/iwpan $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wpan-ping/wpan-ping $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,wpan-tools))
