#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=strace
<<<<<<< HEAD
PKG_VERSION:=6.12
=======
PKG_VERSION:=5.10
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://strace.io/files/$(PKG_VERSION)
<<<<<<< HEAD
PKG_HASH:=c47da93be45b6055f4dc741d7f20efaf50ca10160a5b100c109b294fd9c0bdfe
=======
PKG_HASH:=fe3982ea4cd9aeb3b4ba35f6279f0b577a37175d3282be24b9a5537b56b8f01c
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING
<<<<<<< HEAD
PKG_CPE_ID:=cpe:/a:strace_project:strace
=======
PKG_CPE_ID:=cpe:/a:paul_kranenburg:strace
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

PKG_CONFIG_DEPENDS := \
	CONFIG_STRACE_LIBDW \
	CONFIG_STRACE_LIBUNWIND

include $(INCLUDE_DIR)/package.mk
<<<<<<< HEAD
include $(INCLUDE_DIR)/nls.mk
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

HOST_CFLAGS += -I$(LINUX_DIR)/user_headers/include

CONFIGURE_VARS+= \
	LDFLAGS_FOR_BUILD="$(HOST_LDFLAGS)" \
	CPPFLAGS_FOR_BUILD="$(HOST_CPPFLAGS)" \
	CFLAGS_FOR_BUILD="$(HOST_CFLAGS)" \
	CC_FOR_BUILD="$(HOST_CC)"

define Package/strace
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=System call tracer
  URL:=https://strace.io/
  DEPENDS:=+STRACE_LIBDW:libdw +STRACE_LIBUNWIND:libunwind
endef

define Package/strace/description
  A useful diagnostic, instructional, and debugging tool. Allows you to track what
  system calls a program makes while it is running.
endef

define Package/strace/config
choice
	prompt "stack tracing support"
	default STRACE_NONE

	config STRACE_NONE
		bool "None"

	config STRACE_LIBDW
		bool "libdw"

	config STRACE_LIBUNWIND
		bool "libunwind (experimental)"
endchoice
endef

CONFIGURE_ARGS += \
	--with-libdw=$(if $(CONFIG_STRACE_LIBDW),yes,no) \
	--with-libunwind=$(if $(CONFIG_STRACE_LIBUNWIND),yes,no) \
<<<<<<< HEAD
	--enable-mpers=no \
	--without-libselinux
=======
	--enable-mpers=no
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

MAKE_FLAGS := \
	CCOPT="$(TARGET_CFLAGS)"

define Package/strace/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/strace $(1)/usr/bin/
endef

$(eval $(call BuildPackage,strace))
