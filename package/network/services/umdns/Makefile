#
<<<<<<< HEAD
# Copyright (C) 2014-2023 OpenWrt.org
=======
# Copyright (C) 2014-2021 OpenWrt.org
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=umdns
<<<<<<< HEAD
PKG_RELEASE:=1

PKG_SOURCE_URL=$(PROJECT_GIT)/project/mdnsd.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2025-02-10
PKG_SOURCE_VERSION:=c5ca22a71b2ecb83a2a28b46391dac443be315c2
PKG_MIRROR_HASH:=2d595fa1e58f028a7ff6813b110ee6bcbc8e6ccce81b2792200508c6db9c276a
=======
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_URL=$(PROJECT_GIT)/project/mdnsd.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2021-01-26
PKG_SOURCE_VERSION:=78aa36b0e9808e801c527c6dc47320e593309522
PKG_MIRROR_HASH:=241833f2bf2f3366f356703159be386862ef747d9b253af6c13555f252cc970d
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_MAINTAINER:=John Crispin <john@phrozen.org>
PKG_LICENSE:=LGPL-2.1

include $(INCLUDE_DIR)/package-seccomp.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/umdns
  SECTION:=net
  CATEGORY:=Network
  TITLE:=OpenWrt Multicast DNS Daemon
<<<<<<< HEAD
  DEPENDS:=+libubox +libubus +libblobmsg-json +libudebug
=======
  DEPENDS:=+libubox +libubus +libblobmsg-json
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include

define Package/umdns/conffiles
/etc/config/umdns
endef

define Package/umdns/install
<<<<<<< HEAD
	$(INSTALL_DIR) \
		$(1)/usr/sbin \
		$(1)/etc/init.d \
		$(1)/etc/config \
		$(1)/usr/share/ucode/cli/modules
	$(INSTALL_DATA) ./files/mdns.uc $(1)/usr/share/ucode/cli/modules
=======
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d $(1)/etc/config
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/umdns $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/umdns.init $(1)/etc/init.d/umdns
	$(INSTALL_CONF) ./files/umdns.config $(1)/etc/config/umdns
	$(call InstallSeccomp,$(1),./files/umdns.json)
endef

$(eval $(call BuildPackage,umdns))
