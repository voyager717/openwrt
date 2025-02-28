#
# Copyright (C) 2009-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=isl
<<<<<<< HEAD
PKG_VERSION:=0.26

PKG_SOURCE_URL:=https://libisl.sourceforge.io/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=b10473024cbf17d7db85323121eff0e50f03de14342a03738b4d384b587ce212
=======
PKG_VERSION:=0.20

PKG_SOURCE_URL:=https://libisl.sourceforge.io/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=a5596a9fb8a5b365cb612e4b9628735d6e67e9178fae134a816ae195017e77aa
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

unexport CFLAGS

HOST_CONFIGURE_ARGS += \
	--enable-static \
	--disable-shared \
	--with-gmp-prefix=$(STAGING_DIR_HOST)

<<<<<<< HEAD
define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
$(eval $(call HostBuild))
