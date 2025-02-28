<<<<<<< HEAD
# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013-2021 OpenWrt.org
=======
#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(TOPDIR)/rules.mk

PKG_NAME:=zram-swap
<<<<<<< HEAD
PKG_RELEASE:=32
=======
PKG_RELEASE:=8
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/zram-swap
  SECTION:=utils
  CATEGORY:=Base system
<<<<<<< HEAD
  DEPENDS:= \
	+@BUSYBOX_CONFIG_FEATURE_SWAPON_DISCARD \
	+@BUSYBOX_CONFIG_FEATURE_SWAPON_PRI \
	+@BUSYBOX_CONFIG_MKSWAP \
	+@BUSYBOX_CONFIG_SWAPOFF \
	+@BUSYBOX_CONFIG_SWAPON \
	+kmod-zram
=======
  DEPENDS:=+kmod-zram +@BUSYBOX_CONFIG_MKSWAP +@BUSYBOX_CONFIG_SWAPON +@BUSYBOX_CONFIG_FEATURE_SWAPON_DISCARD +@BUSYBOX_CONFIG_FEATURE_SWAPON_PRI +@BUSYBOX_CONFIG_SWAPOFF
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  TITLE:=ZRAM swap scripts
  PKGARCH:=all
endef

define Package/zram-swap/description
<<<<<<< HEAD
 A script to activate swaping on a compressed zram partition. This
 could be used to increase the available memory, by using compressed
=======
 A script to activate swaping on a compressed zram partition. This 
 could be used to increase the available memory, by using compressed 
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
 memory.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/zram-swap/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/zram.init $(1)/etc/init.d/zram
endef

$(eval $(call BuildPackage,zram-swap))
