#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gdb
<<<<<<< HEAD
PKG_VERSION:=15.2
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/gdb
PKG_HASH:=83350ccd35b5b5a0cba6b334c41294ea968158c573940904f00b92f76345314d
=======
PKG_VERSION:=10.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/gdb
PKG_HASH:=f82f1eceeec14a3afa2de8d9b0d3c91d5a3820e23e0a01bbb70ef9f0276b62c0
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_LICENSE:=GPL-3.0+
PKG_CPE_ID:=cpe:/a:gnu:gdb

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/gdb/Default
  SECTION:=devel
  CATEGORY:=Development
  DEPENDS:=+!USE_MUSL:libthread-db $(ICONV_DEPENDS) $(INTL_DEPENDS)
  URL:=https://www.gnu.org/software/gdb/
endef

define Package/gdb
$(call Package/gdb/Default)
  TITLE:=GNU Debugger
<<<<<<< HEAD
  DEPENDS+=+libreadline +libncurses +zlib +libgmp +libmpfr
endef

define Package/gdb/description
GDB, the GNU Project debugger, allows you to see what is going on 'inside'
=======
  DEPENDS+=+libreadline +libncurses +zlib
endef

define Package/gdb/description
GDB, the GNU Project debugger, allows you to see what is going on `inside'
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
another program while it executes -- or what another program was doing at the
moment it crashed.
endef

define Package/gdbserver
$(call Package/gdb/Default)
  TITLE:=Remote server for GNU Debugger
<<<<<<< HEAD
=======
  DEPENDS+=@!arc
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Package/gdbserver/description
GDBSERVER is a program that allows you to run GDB on a different machine than the
one which is running the program being debugged.
endef

# XXX: add --disable-werror to prevent build failure with arm
CONFIGURE_ARGS+= \
	--with-system-readline \
	--with-system-zlib \
	--without-expat \
	--without-lzma \
<<<<<<< HEAD
	--without-zstd \
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	--disable-unit-tests \
	--disable-ubsan \
	--disable-sim \
	--disable-werror \
<<<<<<< HEAD
	--disable-source-highlight \
	--without-isl \
	--without-xxhash \
	--with-libgmp-prefix=$(STAGING_DIR)/usr
=======
	--disable-source-highlight
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

CONFIGURE_VARS+= \
	ac_cv_search_tgetent="$(TARGET_LDFLAGS) -lncurses -lreadline"

TARGET_LDFLAGS+= \
	$(INTL_LDFLAGS) $(if $(INTL_FULL),-lintl) \
	-static-libstdc++ \
	-Wl,--gc-sections

define Build/Install
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		install-gdb install-gdbserver
endef

define Package/gdb/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gdb $(1)/usr/bin/
endef

define Package/gdbserver/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gdbserver $(1)/usr/bin/
endef

$(eval $(call BuildPackage,gdb))
$(eval $(call BuildPackage,gdbserver))
