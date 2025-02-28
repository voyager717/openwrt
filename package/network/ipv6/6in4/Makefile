#
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=6in4
<<<<<<< HEAD
PKG_RELEASE:=29
=======
PKG_RELEASE:=26
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/6in4
  SECTION:=net
  CATEGORY:=Network
<<<<<<< HEAD
  DEPENDS:=@IPV6 +kmod-sit +uclient-fetch +resolveip
=======
  DEPENDS:=@IPV6 +kmod-sit +uclient-fetch
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  TITLE:=IPv6-in-IPv4 configuration support
  MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
  PKGARCH:=all
endef

define Package/6in4/description
Provides support for 6in4 tunnels in /etc/config/network.
Refer to http://wiki.openwrt.org/doc/uci/network for
configuration details.
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/6in4/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/6in4.sh $(1)/lib/netifd/proto/6in4.sh
endef

$(eval $(call BuildPackage,6in4))
