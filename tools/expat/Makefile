#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=expat
<<<<<<< HEAD
PKG_CPE_ID:=cpe:/a:libexpat:libexpat
PKG_VERSION:=2.6.3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=17aa6cfc5c4c219c09287abfc10bc13f0c06f30bb654b28bfe6f567ca646eb79
=======
PKG_CPE_ID:=cpe:/a:libexpat:expat
PKG_VERSION:=2.2.10

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_HASH:=b2c160f1b60e92da69de8e12333096aeb0c3bf692d41c60794de278af72135a5
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_SOURCE_URL:=https://github.com/libexpat/libexpat/releases/download/R_$(subst .,_,$(PKG_VERSION))

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

<<<<<<< HEAD
HOSTCC := $(HOSTCC_NOCACHE)

HOST_CONFIGURE_ARGS += \
	--disable-shared \
	--without-docbook \
	--without-tests \
	--with-pic

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
=======
HOST_CONFIGURE_ARGS += \
	--without-docbook

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) install
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call HostBuild))
