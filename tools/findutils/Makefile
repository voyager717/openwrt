# 
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=findutils
PKG_CPE_ID:=cpe:/a:gnu:findutils
<<<<<<< HEAD
PKG_VERSION:=4.10.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=1387e0b67ff247d2abde998f90dfbf70c1491391a59ddfecb8ae698789f0a4f5

PKG_PROGRAMS:=find locate updatedb xargs
=======
PKG_VERSION:=4.7.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=c5fefbdf9858f7e4feb86f036e1247a54c79fc2d8e4b7064d5aaa1f47dfa789a
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

<<<<<<< HEAD
ifeq ($(HOST_OS),Darwin)
	HOST_CFLAGS +=-D__nonnull\\(params\\)=
endif

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
HOST_CONFIGURE_ARGS += \
	--enable-threads=pth \
	--disable-rpath \
	--disable-nls \
	--without-selinux

<<<<<<< HEAD
define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
$(eval $(call HostBuild))
