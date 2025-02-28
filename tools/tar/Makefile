#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=tar
PKG_CPE_ID:=cpe:/a:gnu:tar
<<<<<<< HEAD
PKG_VERSION:=1.35

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/tar
PKG_HASH:=14d55e32063ea9526e057fbf35fcabd53378e769787eff7919c3755b02d2b57e
=======
PKG_VERSION:=1.32

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/tar
PKG_HASH:=d0d3ae07f103323be809bc3eac0dcc386d52c5262499fe05511ac4788af1fdd8
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOSTCC := $(HOSTCC_NOCACHE)
HOSTCXX := $(HOSTCXX_NOCACHE)

HOST_CONFIGURE_ARGS += \
	--without-posix-acls \
	--without-selinux \
	--without-xattrs \
	--disable-acl \
	--disable-nls

<<<<<<< HEAD
define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
$(eval $(call HostBuild))
