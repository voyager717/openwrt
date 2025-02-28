<<<<<<< HEAD
# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2016 OpenWrt.org
#
=======
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(TOPDIR)/rules.mk

PKG_NAME:=arptables
<<<<<<< HEAD
PKG_VERSION:=0.0.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://ftp.netfilter.org/pub/arptables
PKG_HASH:=4f9a0656ce5c90868f551cd4deeb2d04f33899667e1fb2818b64e432fe8f629c

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/arptables-legacy
=======
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://git.netfilter.org/arptables
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2015-05-20
PKG_SOURCE_VERSION:=f4ab8f63f11a72f14687a6646d04ae1bae3fa45f
PKG_MIRROR_HASH:=84bc660be4c9f70be91046acfd87785add930eceab7c543036058e1a9de2e9d9

PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/arptables
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE:=ARP firewalling software
  DEPENDS:=+kmod-arptables
  URL:=https://git.netfilter.org/arptables/
<<<<<<< HEAD
  PROVIDES:=arptables
  ALTERNATIVES:=\
    200:/usr/sbin/arptables:/usr/sbin/arptables-legacy
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

MAKE_FLAGS += \
	COPT_FLAGS="$(TARGET_CFLAGS) -D__OPTIMIZE__=1" \
	KERNEL_DIR="$(LINUX_DIR)"

<<<<<<< HEAD
define Package/arptables-legacy/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/arptables-legacy $(1)/usr/sbin/arptables-legacy
endef

$(eval $(call BuildPackage,arptables-legacy))
=======
define Package/arptables/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,arptables))
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
