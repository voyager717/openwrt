<<<<<<< HEAD
# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2015 OpenWrt.org
#
=======
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(TOPDIR)/rules.mk

PKG_NAME:=nftables
<<<<<<< HEAD
PKG_VERSION:=1.1.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://netfilter.org/projects/$(PKG_NAME)/files
PKG_HASH:=6358830f3a64f31e39b0ad421d7dadcd240b72343ded48d8ef13b8faf204865a

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING
=======
PKG_VERSION:=0.9.6
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://netfilter.org/projects/$(PKG_NAME)/files
PKG_HASH:=68d6fdfe8ab02303e6b1f13968a4022da5b0120110eaee3233d806857937b66e
PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

<<<<<<< HEAD
PKG_BUILD_FLAGS:=lto

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
include $(INCLUDE_DIR)/package.mk

DISABLE_NLS:=

CONFIGURE_ARGS += \
        --disable-debug \
        --disable-man-doc \
        --with-mini-gmp \
        --without-cli \
        --disable-python

define Package/nftables/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE:=nftables userspace utility
  DEPENDS:=+kmod-nft-core +libnftnl
  URL:=http://netfilter.org/projects/nftables/
  PROVIDES:=nftables
endef

define Package/nftables-nojson
  $(Package/nftables/Default)
  TITLE+= no JSON support
  VARIANT:=nojson
  DEFAULT_VARIANT:=1
<<<<<<< HEAD
  CONFLICTS:=nftables-json
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Package/nftables-json
  $(Package/nftables/Default)
  TITLE+= with JSON support
  VARIANT:=json
  DEPENDS+=+jansson
endef

ifeq ($(BUILD_VARIANT),json)
  CONFIGURE_ARGS += --with-json
endif

<<<<<<< HEAD
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/nftables $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libnftables.pc \
		$(1)/usr/lib/pkgconfig/
endef
=======
TARGET_CFLAGS += -flto
TARGET_LDFLAGS += -flto
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Package/nftables/install/Default
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/nft $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

Package/nftables-nojson/install = $(Package/nftables/install/Default)
Package/nftables-json/install = $(Package/nftables/install/Default)

$(eval $(call BuildPackage,nftables-nojson))
$(eval $(call BuildPackage,nftables-json))
