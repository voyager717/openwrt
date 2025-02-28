#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ds-lite
<<<<<<< HEAD
PKG_RELEASE:=9
=======
PKG_RELEASE:=8
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/ds-lite
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=@IPV6 +kmod-ip6-tunnel +resolveip
<<<<<<< HEAD
  TITLE:=IPv4 over IPv6 (RFC2473 and DS-Lite) configuration support
=======
  TITLE:=Dual-Stack Lite (DS-Lite) configuration support
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  MAINTAINER:=Steven Barth <steven@midlink.org>
  PKGARCH:=all
endef

define Package/ds-lite/description
<<<<<<< HEAD
Provides support for IPv4 over IPv6 (RFC2473 and DS-Lite) in /etc/config/network.
=======
Provides support for Dual-Stack Lite in /etc/config/network.
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
Refer to http://wiki.openwrt.org/doc/uci/network for
configuration details.
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/ds-lite/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/dslite.sh $(1)/lib/netifd/proto/dslite.sh
endef

$(eval $(call BuildPackage,ds-lite))
