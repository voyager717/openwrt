#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vti
<<<<<<< HEAD
PKG_RELEASE:=5
=======
PKG_RELEASE:=3
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/vti/Default
<<<<<<< HEAD
endef

define Package/vti
  SECTION:=net
  CATEGORY:=Network
  MAINTAINER:=Andre Valentin <avalentin@marcant.net>
  TITLE:=Virtual IPsec Tunnel Interface config support
  DEPENDS:=+kmod-ip-vti +IPV6:kmod-ip6-vti
  PROVIDES:=vtiv4 vtiv6
  PKGARCH:=all
endef

=======
  SECTION:=net
  CATEGORY:=Network
  MAINTAINER:=Andre Valentin <avalentin@marcant.net>
  PKGARCH:=all
endef

define Package/vti
$(call Package/vti/Default)
  TITLE:=Virtual IPsec Tunnel Interface config support
endef

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
define Package/vti/description
 Virtual IPsec Tunnel Interface config support (IPv4 and IPv6) in /etc/config/network.
endef

<<<<<<< HEAD
=======
define Package/vtiv4
$(call Package/vti/Default)
  TITLE:=Virtual IPsec Tunnel Interface (IPv4) config support
  DEPENDS:=@(PACKAGE_vti) +kmod-ip-vti
endef

define Package/vtiv4/description
 Virtual IPsec Tunnel Interface config support (IPv4) in /etc/config/network.
endef

define Package/vtiv6
$(call Package/vti/Default)
  TITLE:=Virtual IPsec Tunnel Interface (IPv6) config support
  DEPENDS:=@(PACKAGE_vti) @IPV6 +kmod-ip6-vti
endef

define Package/vtiv6/description
 Virtual IPsec Tunnel Interface config support (IPv6) in /etc/config/network.
endef

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
define Build/Compile
endef

define Build/Configure
endef

define Package/vti/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/vti.sh $(1)/lib/netifd/proto/vti.sh
endef

<<<<<<< HEAD
$(eval $(call BuildPackage,vti))
=======
define Package/vtiv4/install
	:
endef

define Package/vtiv6/install
	:
endef

$(eval $(call BuildPackage,vti))
$(eval $(call BuildPackage,vtiv4))
$(eval $(call BuildPackage,vtiv6))
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
