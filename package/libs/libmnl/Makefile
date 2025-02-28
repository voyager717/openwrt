#
# Copyright (C) 2011-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmnl
<<<<<<< HEAD
PKG_VERSION:=1.0.5
PKG_RELEASE:=1
=======
PKG_VERSION:=1.0.4
PKG_RELEASE:=2
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	http://www.netfilter.org/projects/libmnl/files \
	ftp://ftp.netfilter.org/pub/libmnl
<<<<<<< HEAD
PKG_HASH:=274b9b919ef3152bfb3da3a13c950dd60d6e2bcd54230ffeca298d03b40d0525
=======
PKG_HASH:=171f89699f286a5854b72b91d06e8f8e3683064c5901fb09d954a9ab6f551f81
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_LICENSE:=LGPL-2.1+
<<<<<<< HEAD
PKG_CPE_ID:=cpe:/a:netfilter:libmnl
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/package.mk

define Package/libmnl
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Minimalistic user-space library for Netlink
  URL:=http://www.netfilter.org/projects/libmnl/
  ABI_VERSION:=0
endef

define Package/libmnl/description
 libmnl is a minimalistic user-space library oriented to Netlink developers.
 There are a lot of common tasks in parsing, validating, constructing of
 both the Netlink header and TLVs that are repetitive and easy to get wrong.
 This library aims to provide simple helpers that allows you to re-use code
 and to avoid re-inventing the wheel. The main features of this library are:
 .
 * Small: the shared library requires around 30KB for an x86-based computer.
 .
 * Simple: this library avoids complexity and elaborated abstractions that
   tend to hide Netlink details.
 .
 * Easy to use: the library simplifies the work for Netlink-wise developers.
   It provides functions to make socket handling, message building, validating,
   parsing and sequence tracking, easier.
 .
 * Easy to re-use: you can use the library to build your own abstraction layer
   on top of this library.
 .
 * Decoupling: the interdependency of the main bricks that compose the library
   is reduced, i.e. the library provides many helpers, but the programmer is not
   forced to use them.
endef

CONFIGURE_ARGS+= \
	--enable-shared \
	--enable-static \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libmnl $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmnl.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libmnl.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libmnl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmnl.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmnl))
