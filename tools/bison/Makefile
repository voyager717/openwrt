#
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=bison
<<<<<<< HEAD
PKG_VERSION:=3.8.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=06c9e13bdf7eb24d4ceb6b59205a4f67c2c7e7213119644430fe82fbd14a0abb
PKG_CPE_ID:=cpe:/a:gnu:bison
=======
PKG_VERSION:=3.7.4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=a3b5813f48a11e540ef26f46e4d288c0c25c7907d9879ae50e430ec49f63c010
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += --enable-threads=posix --disable-nls

<<<<<<< HEAD
=======
define Host/Clean
	-$(MAKE) -C $(HOST_BUILD_DIR) uninstall
	$(call Host/Clean/Default)
endef

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
define Host/Install
	$(call Host/Install/Default)
	$(INSTALL_BIN) ./scripts/yacc $(STAGING_DIR_HOST)/bin/yacc
endef

<<<<<<< HEAD
define Host/Uninstall
	rm -f $(STAGING_DIR_HOST)/bin/yacc
	-$(call Host/Compile/Default,uninstall)
endef

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
$(eval $(call HostBuild))
