<<<<<<< HEAD
#
=======
# 
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=autoconf
<<<<<<< HEAD
PKG_VERSION:=2.72

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/autoconf
PKG_HASH:=ba885c1319578d6c94d46e9b0dceb4014caafe2490e437a0dbca3f270a223f5a

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_VARS += \
	EMACS="no" \
	PERL="/usr/bin/env perl"

define Host/Compile
	-$(call Host/Compile/Default,--touch install-man1) # disable building of man pages
	$(call Host/Compile/Default)
endef

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
=======
PKG_VERSION:=2.69

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/autoconf
PKG_HASH:=64ebcec9f8ac5b2487125a86a7760d2591ac9e1d3dbd59489633f9de62a57684

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--datarootdir=$(STAGING_DIR_HOST)/share

HOST_CONFIGURE_VARS += \
	PERL="/usr/bin/env perl"

define Host/Compile
	export SHELL="$(BASH)"; $(MAKE) -C $(HOST_BUILD_DIR)
endef

define Host/Install
	export SHELL="$(BASH)"; $(MAKE) -C $(HOST_BUILD_DIR) install
endef

define Host/Clean
	-export SHELL="$(BASH)"; $(MAKE) -C $(HOST_BUILD_DIR) uninstall
	$(call Host/Clean/Default)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call HostBuild))
