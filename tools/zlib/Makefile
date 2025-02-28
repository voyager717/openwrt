#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=zlib
<<<<<<< HEAD
PKG_VERSION:=1.3.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/madler/zlib
PKG_SOURCE_VERSION:=51b7f2abdade71cd9bb0e7a373ef2610ec6f9daf
PKG_MIRROR_HASH:=6558577038f4839057fad93afb295bf32e84dc9bd2c33512d40a5eab6d4889ef

PKG_LICENSE:=Zlib
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:zlib:zlib

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOSTCC := $(HOSTCC_NOCACHE)
HOST_CFLAGS += $(HOST_FPIC)

HOST_CONFIGURE_ARGS = \
	--prefix=$(STAGING_DIR_HOST) \
	--sysconfdir=$(STAGING_DIR_HOST)/etc \
	--localstatedir=$(STAGING_DIR_HOST)/var \
	--libdir=$(STAGING_DIR_HOST)/lib \
	--includedir=$(STAGING_DIR_HOST)/include \
	--static

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
=======
PKG_VERSION:=1.2.11
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@SF/libpng http://www.zlib.net
PKG_HASH:=4ff941449631ace0d4d203e3483be9dbc9da454084111f97ea0a2114e19bf066

PKG_LICENSE:=Zlib
PKG_LICENSE_FILES:=README
PKG_CPE_ID:=cpe:/a:gnu:zlib

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

HOST_CFLAGS +=-fPIC

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/libz.a $(STAGING_DIR_HOST)/lib/
	$(CP) $(HOST_BUILD_DIR)/zconf.h $(STAGING_DIR_HOST)/include/
	$(CP) $(HOST_BUILD_DIR)/zlib.h $(STAGING_DIR_HOST)/include/
	$(CP) $(HOST_BUILD_DIR)/zlib.pc $(STAGING_DIR_HOST)/lib/pkgconfig/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/lib/libz.a
	rm -f $(STAGING_DIR_HOST)/include/zconf.h
	rm -f $(STAGING_DIR_HOST)/include/zlib.h
	rm -f $(STAGING_DIR_HOST)/lib/pkgconfig//zlib.pc
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call HostBuild))
