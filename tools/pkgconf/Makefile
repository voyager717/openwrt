#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=pkgconf
<<<<<<< HEAD
PKG_VERSION:=2.2.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://distfiles.dereferenced.org/pkgconf
PKG_HASH:=28f8dfc279a10ef66148befa3f6eb266e5f3570316600208ed50e9781c7269d8

PKG_CPE_ID:=cpe:/a:pkgconf:pkgconf

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/meson.mk
=======
PKG_VERSION:=1.7.3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://distfiles.dereferenced.org/pkgconf
PKG_HASH:=b846aea51cf696c3392a0ae58bef93e2e72f8e7073ca6ad1ed8b01c85871f9c0

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

unexport PKG_CONFIG

HOSTCC := $(HOSTCC_NOCACHE)

<<<<<<< HEAD
MESON_HOST_ARGS += \
	-Ddefault_library=static \
	-Dtests=disabled

define Host/Install
	$(call Host/Install/Meson)
=======
define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) install
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	mv $(STAGING_DIR_HOST)/bin/pkgconf $(STAGING_DIR_HOST)/bin/pkg-config.real
	$(INSTALL_BIN) ./files/pkg-config $(STAGING_DIR_HOST)/bin/pkg-config
endef

define Host/Clean
<<<<<<< HEAD
	rm -f $(STAGING_DIR_HOST)/bin/pkg-config.real $(STAGING_DIR_HOST)/bin/pkg-config
	$(call Host/Clean/Meson)
=======
	-$(MAKE) -C $(HOST_BUILD_DIR) uninstall
	$(call Host/Clean/Default)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call HostBuild))
