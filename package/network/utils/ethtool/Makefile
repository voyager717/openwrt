#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ethtool
<<<<<<< HEAD
PKG_VERSION:=6.11
=======
PKG_VERSION:=5.10
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_RELEASE:=1

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/software/network/ethtool
<<<<<<< HEAD
PKG_HASH:=8d91f5c72ae3f25b7e88d4781279dcb320f71e30058914370b1c574c96b31202

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:kernel:ethtool
=======
PKG_HASH:=817d5396a9307b4c637b435d4c558b8f5f964a1464a035ca3c0180f4cc93cfcf

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

<<<<<<< HEAD
=======
PKG_CONFIG_DEPENDS:=CONFIG_ETHTOOL_PRETTY_DUMP

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
include $(INCLUDE_DIR)/package.mk

define Package/ethtool
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Display or change ethernet card settings
  URL:=http://www.kernel.org/pub/software/network/ethtool/
<<<<<<< HEAD
  VARIANT:=tiny
  CONFLICTS:=ethtool-full
endef

define Package/ethtool-full
  $(Package/ethtool)
  TITLE += (full)
  VARIANT:=full
  PROVIDES:=ethtool
  DEPENDS:=+libmnl
  CONFLICTS:=
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Package/ethtool/description
 ethtool is a small utility for examining and tuning your ethernet-based
 network interface
endef

<<<<<<< HEAD
Package/ethtool-full/description:=$(Package/ethtool/description)

ifeq ($(BUILD_VARIANT),full)
CONFIGURE_ARGS += --enable-netlink --enable-pretty-dump
else
CONFIGURE_ARGS += --disable-netlink --disable-pretty-dump
endif

# enable support for input_xfrm with kernels newer than 6.6
ifeq ($(CONFIG_LINUX_6_6),)
CONFIGURE_ARGS += --enable-rss-input-xfrm
=======
define Package/ethtool/config
	config ETHTOOL_PRETTY_DUMP
		depends on PACKAGE_ethtool
		bool "Enable pretty printing"
endef

CONFIGURE_ARGS += --disable-netlink

ifeq ($(CONFIG_ETHTOOL_PRETTY_DUMP),y)
CONFIGURE_ARGS += --enable-pretty-dump
else
CONFIGURE_ARGS += --disable-pretty-dump
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endif

define Package/ethtool/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ethtool $(1)/usr/sbin
endef

<<<<<<< HEAD
Package/ethtool-full/install=$(Package/ethtool/install)

$(eval $(call BuildPackage,ethtool))
$(eval $(call BuildPackage,ethtool-full))
=======
$(eval $(call BuildPackage,ethtool))
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
