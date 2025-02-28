#
# Copyright (C) 2012-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=odhcp6c
<<<<<<< HEAD
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/odhcp6c.git
PKG_SOURCE_DATE:=2024-09-25
PKG_SOURCE_VERSION:=b6ae9ffaeb0e18e9fa3d5be62faa8d3c9f340a58
PKG_MIRROR_HASH:=25499b6a0d403815e6b04869a02d2d5a32582348bff2bc2626559d297425a908
=======
PKG_RELEASE:=16

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/odhcp6c.git
PKG_SOURCE_DATE:=2021-01-09
PKG_SOURCE_VERSION:=53f07e90b7f1da6977143a488dd5cb73a33b233b
PKG_MIRROR_HASH:=ad3665b611d19177996771b11df28d92f066b0125484ea3bdfc0b3185c14f90e
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_MAINTAINER:=Hans Dedecker <dedeckeh@gmail.com>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += \
	-DUSE_LIBUBOX=on

ifneq ($(CONFIG_PACKAGE_odhcp6c_ext_cer_id),0)
  CMAKE_OPTIONS += -DEXT_CER_ID=$(CONFIG_PACKAGE_odhcp6c_ext_cer_id)
endif

define Package/odhcp6c
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Embedded DHCPv6-client for OpenWrt
  DEPENDS:=@IPV6 +libubox
endef

define Package/odhcp6c/config
  config PACKAGE_odhcp6c_ext_cer_id
    int "CER-ID Extension ID (0 = disabled)"
    depends on PACKAGE_odhcp6c
    default 0
endef

<<<<<<< HEAD
define Package/odhcp6c/conffiles
/etc/odhcp6c.user
/etc/odhcp6c.user.d/
endef

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
define Package/odhcp6c/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/odhcp6c $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/dhcpv6.sh $(1)/lib/netifd/proto/dhcpv6.sh
	$(INSTALL_BIN) ./files/dhcpv6.script $(1)/lib/netifd/
<<<<<<< HEAD
	$(INSTALL_DIR) $(1)/etc/odhcp6c.user.d/
	$(INSTALL_CONF) ./files/odhcp6c.user $(1)/etc/
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call BuildPackage,odhcp6c))
