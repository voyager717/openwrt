# 
# Copyright (C) 2008-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=m4
PKG_CPE_ID:=cpe:/a:gnu:m4
PKG_VERSION:=1.4.19

<<<<<<< HEAD
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=3be4a26d825ffdfda52a56fc43246456989a3630093cced3fbddf4771ee58a70
=======
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=63aede5c6d33b6d9b13511cd0be2cac046f2e70fd0a07aa9573a04a82783af96
PKG_CAT:=xzcat
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_VARS += gl_cv_func_strstr_linear=no

<<<<<<< HEAD
define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
=======
define Host/Clean
	-$(MAKE) -C $(HOST_BUILD_DIR) uninstall
	$(call Host/Clean/Default)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call HostBuild))
