#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=relayd
PKG_RELEASE:=1

PKG_SOURCE_URL=$(PROJECT_GIT)/project/relayd.git
PKG_SOURCE_PROTO:=git
<<<<<<< HEAD
PKG_SOURCE_DATE:=2023-01-28
PKG_SOURCE_VERSION:=f646ba40489371e69f624f2dee2fc4e19ceec00e
PKG_MIRROR_HASH:=658a09a1a947c148c2827335c7bbe126f4bcf361d2de6d9de7cca432c9fe04d3
=======
PKG_SOURCE_DATE:=2020-04-25
PKG_SOURCE_VERSION:=f4d759be54ceb37714e9a6ca320d5b50c95e9ce9
PKG_MIRROR_HASH:=b1ff6e99072867be0975ba0be52ba9da3a876c8b8da893d68301e8238243a51e
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/relayd
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Routing and Redirection
  TITLE:=Transparent routing / relay daemon
  DEPENDS:=+libubox
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include

define Package/relayd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/relayd $(1)/usr/sbin/relayd
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/relay.init $(1)/etc/init.d/relayd
endef

$(eval $(call BuildPackage,relayd))
