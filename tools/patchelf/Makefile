# 
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=patchelf
<<<<<<< HEAD
PKG_VERSION:=0.18.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/NixOS/patchelf/releases/download/$(PKG_VERSION)
PKG_HASH:=1952b2a782ba576279c211ee942e341748fdb44997f704dd53def46cd055470b
=======

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/NixOS/patchelf.git
PKG_SOURCE_VERSION:=f34751b88bd07d7f44f5cd3200fb4122bf916c7e
PKG_SOURCE_DATE:=2020-12-07
PKG_MIRROR_HASH:=ac746930b919b97da40f259cfc9ab7bbd48a0c9cbf2eebd8cee5ae19a94356fd
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

HOST_BUILD_PARALLEL:=1
HOST_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/src/patchelf $(STAGING_DIR_HOST)/bin/patchelf
endef

<<<<<<< HEAD
define Host/Clean
	rm -rf $(STAGING_DIR_HOST)/bin/patchelf
endef

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
$(eval $(call HostBuild))
