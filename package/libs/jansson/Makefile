#
# Copyright (C) 2011-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=jansson
<<<<<<< HEAD
PKG_VERSION:=2.14
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/akheron/$(PKG_NAME)/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=c739578bf6b764aa0752db9a2fdadcfe921c78f1228c7ec0bb47fa804c55d17b

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:jansson_project:jansson

PKG_BUILD_FLAGS:=lto
=======
PKG_VERSION:=2.13.1
PKG_RELEASE:=2
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://www.digip.org/jansson/releases/
PKG_HASH:=ee90a0f879d2b7b7159124ff22b937a2a9a8c36d3bb65d1da7dd3f04370a10bd
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_INSTALL:=1
CMAKE_OPTIONS += \
	-DJANSSON_BUILD_DOCS:BOOL=OFF \
	-DJANSSON_BUILD_SHARED_LIBS:BOOL=ON \
	-DJANSSON_EXAMPLES:BOOL=OFF \
	-DJANSSON_WITHOUT_TESTS:BOOL=ON

define Package/jansson
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Jansson library
  URL:=http://www.digip.org/jansson/
  ABI_VERSION:=4
endef

define Package/jansson/description
  Jansson is a C library for encoding, decoding and manipulating JSON data
endef

<<<<<<< HEAD
define Package/jansson/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libjansson*so.* $(1)/usr/lib/
=======
TARGET_CFLAGS += -flto
TARGET_LDFLAGS += -flto

define Package/jansson/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libjansson*so* $(1)/usr/lib/
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call BuildPackage,jansson))
