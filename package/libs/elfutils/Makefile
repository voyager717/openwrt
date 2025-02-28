#
# Copyright (C) 2010-2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=elfutils
<<<<<<< HEAD
PKG_VERSION:=0.192
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://sourceware.org/$(PKG_NAME)/ftp/$(PKG_VERSION) \
                https://mirrors.kernel.org/sourceware/$(PKG_NAME)/$(PKG_VERSION)
PKG_HASH:=616099beae24aba11f9b63d86ca6cc8d566d968b802391334c91df54eab416b4
=======
PKG_VERSION:=0.180
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://sourceware.org/$(PKG_NAME)/ftp/$(PKG_VERSION)
PKG_HASH:=b827b6e35c59d188ba97d7cf148fa8dc6f5c68eb6c5981888dfdbb758c0b569d
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_MAINTAINER:=Luiz Angelo Daros de Luca <luizluca@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING COPYING-GPLV2 COPYING-LGPLV3
PKG_CPE_ID:=cpe:/a:elfutils_project:elfutils

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
<<<<<<< HEAD
PKG_BUILD_DEPENDS:=!USE_GLIBC:argp-standalone

include $(INCLUDE_DIR)/package.mk
=======
PKG_USE_MIPS16:=0
PKG_BUILD_DEPENDS:=!USE_GLIBC:argp-standalone

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
include $(INCLUDE_DIR)/nls.mk

define Package/elfutils/Default
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=ELF manipulation libraries
  URL:=https://fedorahosted.org/elfutils/
  ABI_VERSION:=1
endef

define Package/libasm
  $(call Package/elfutils/Default)
  TITLE+= (libasm)
  DEPENDS:=+libelf +libdw
endef

define Package/libdw
  $(call Package/elfutils/Default)
<<<<<<< HEAD
  DEPENDS:=+libelf +libbz2 +USE_MUSL:musl-fts
=======
  DEPENDS:=+libelf +libbz2
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  TITLE+= (libdw)
endef

define Package/libelf
  $(call Package/elfutils/Default)
  DEPENDS:=$(INTL_DEPENDS) +zlib
  TITLE+= (libelf)
  PROVIDES:=libelf1
endef

ifeq ($(CONFIG_BUILD_NLS),y)
TARGET_LDFLAGS += -lintl
else
CONFIGURE_ARGS += --disable-nls
endif

<<<<<<< HEAD
HOST_CONFIGURE_ARGS += \
	--disable-shared \
	--disable-nls \
	--disable-debuginfod \
	--disable-libdebuginfod \
	--without-bzlib \
	--without-lzma \
	--without-zstd

CONFIGURE_ARGS += \
	--program-prefix=eu- \
	--disable-debuginfod \
	--disable-libdebuginfod \
	--without-bzlib \
	--without-lzma \
	--without-zstd

HOST_CONFIGURE_VARS += \
	ac_cv_search__obstack_free=yes

CONFIGURE_VARS += \
	ac_cv_search__obstack_free=yes

TARGET_CFLAGS += \
	-D_GNU_SOURCE \
	-Wno-unused-result \
	-Wno-format-nonliteral

ifneq ($(CONFIG_GCC_USE_VERSION_11),y)
TARGET_CFLAGS += \
	-Wno-error=use-after-free
endif
=======
CONFIGURE_ARGS += \
	--program-prefix=eu- \
	--disable-debuginfod \
	--without-lzma

TARGET_CFLAGS += -D_GNU_SOURCE -Wno-unused-result -Wno-format-nonliteral
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libasm*.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdw*.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelf*.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libelf.pc $(1)/usr/lib/pkgconfig/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libdw.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libasm/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libasm{-*.so,*.so.*} $(1)/usr/lib/
endef

define Package/libdw/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdw{-*.so,*.so.*} $(1)/usr/lib/
endef

define Package/libelf/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelf{-*.so,*.so.*} $(1)/usr/lib/
endef

# these lines need to be ordered by dependency because of ABI versioning
$(eval $(call BuildPackage,libelf))
$(eval $(call BuildPackage,libdw))
$(eval $(call BuildPackage,libasm))
<<<<<<< HEAD
=======
$(eval $(call HostBuild))
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
