#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=xz
<<<<<<< HEAD
PKG_VERSION:=5.6.4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/tukaani-project/xz/releases/download/v$(PKG_VERSION) \
		@SF/lzmautils \
		http://tukaani.org/xz
PKG_HASH:=176d510c30d80a23b8050bbc048f2ecaacb823ae48b6821727ed6591f0df9200
=======
PKG_VERSION:=5.2.5

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/lzmautils \
		http://tukaani.org/xz
PKG_HASH:=5117f930900b341493827d63aa910ff5e011e0b994197c3b71c08a20228a42df
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_CPE_ID:=cpe:/a:tukaani:xz

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOSTCC := $(HOSTCC_NOCACHE)
HOSTCXX := $(HOSTCXX_NOCACHE)

HOST_CONFIGURE_ARGS += \
	--enable-static=yes \
	--enable-shared=no \
	--disable-doc \
	--disable-nls \
	--with-pic

define Host/Install
	+$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR) install xzlinks="unxz xzcat"
endef

<<<<<<< HEAD
define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
$(eval $(call HostBuild))
