#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
<<<<<<< HEAD

PKG_NAME:=ccache
PKG_VERSION:=4.10.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/ccache/ccache/releases/download/v$(PKG_VERSION)
PKG_HASH:=108100960bb7e64573ea925af2ee7611701241abb36ce0aae3354528403a7d87
=======
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=ccache
PKG_VERSION:=4.2.1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/ccache/ccache/releases/download/v$(PKG_VERSION)
PKG_HASH:=9d6ba1cdefdc690401f404b747d81a9a1802b17af4235815866b7620d980477e

HOST_BUILD_PARALLEL:=1
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_HOST_OPTIONS += \
	-DCMAKE_C_COMPILER_LAUNCHER="" \
	-DCMAKE_CXX_COMPILER_LAUNCHER="" \
	-DCMAKE_SKIP_RPATH=FALSE \
	-DCMAKE_INSTALL_RPATH="${STAGING_DIR_HOST}/lib" \
<<<<<<< HEAD
	-DENABLE_DOCUMENTATION=OFF \
	-DREDIS_STORAGE_BACKEND=OFF \
	-DENABLE_TESTING=OFF
=======

ifneq (docs-$(CONFIG_BUILD_DOCUMENTATION),docs-y)
CMAKE_HOST_OPTIONS += -DENABLE_DOCUMENTATION=OFF
endif

define Host/Install/ccache
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin/
	$(CP) ./files/* $(STAGING_DIR_HOST)/bin/
endef

define Host/Install
	$(call Host/Install/Default)
	$(call Host/Install/ccache)
endef
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

$(eval $(call HostBuild))
