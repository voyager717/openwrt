#
# Copyright (C) 2009-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtd-utils
<<<<<<< HEAD
PKG_VERSION:=2.2.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://infraroot.at/pub/mtd/
PKG_HASH:=f7ae20b2eb79ee83441468f0b99d897024cd96ff853eea59106fb1952065c803

PKG_INSTALL:=1

PKG_FLAGS:=nonshared
PKG_BUILD_FLAGS:=gc-sections
=======
PKG_VERSION:=2.1.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=ftp://ftp.infradead.org/pub/mtd-utils/
PKG_HASH:=8ad4c5f34716d40646aa28724a2f5616d325a6f119254f914e26976f1f76e9d6

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

PKG_FLAGS:=nonshared
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_BUILD_DEPENDS:=util-linux

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=
<<<<<<< HEAD
PKG_CPE_ID:=cpe:/a:mtd-utils_project:mtd-utils
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_MAINTAINER:=John Crispin <john@phrozen.org>

include $(INCLUDE_DIR)/package.mk

define Package/mtd-utils/Default
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=http://www.linux-mtd.infradead.org/
  DEPENDS:=@NAND_SUPPORT
endef

define Package/ubi-utils
 $(call Package/mtd-utils/Default)
  TITLE:=Utilities for ubi info/debug
endef

define Package/ubi-utils/description
  Utilities for manipulating memory technology devices.
endef

define Package/nand-utils
 $(call Package/mtd-utils/Default)
  TITLE:=Utilities for nand flash erase/read/write/test
endef

define Package/nand-utils/description
  Utilities for NAND devices.
endef

MAKE_FLAGS += LDLIBS+="$(LIBGCC_S)"

CONFIGURE_ARGS += \
<<<<<<< HEAD
	--enable-tests \
	--without-crypto \
	--without-xattr \
	--without-zstd \
	--without-lzo \
	--without-zlib

define Package/ubi-utils/conffiles
/etc/config/ubihealthd
endef
=======
	--disable-tests \
	--without-crypto \
	--without-xattr \
	--without-zstd \
	--without-lzo

TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Package/ubi-utils/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) \
<<<<<<< HEAD
		$(PKG_INSTALL_DIR)/usr/sbin/{ubiattach,ubicrc32,ubiblock,ubidetach,ubiformat,ubihealthd} $(1)/usr/sbin/
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/sbin/{ubimkvol,ubinfo,ubinize,ubirename,ubirmvol,ubirsvol,ubiupdatevol} $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ubihealthd.init $(1)/etc/init.d/ubihealthd
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/ubihealthd.defaults $(1)/etc/uci-defaults/ubihealthd
=======
		$(PKG_INSTALL_DIR)/usr/sbin/{ubiattach,ubicrc32,ubiblock,ubidetach,ubiformat,ubimkvol} $(1)/usr/sbin/
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/sbin/{ubinfo,ubinize,ubirename,ubirmvol,ubirsvol,ubiupdatevol} $(1)/usr/sbin/
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Package/nand-utils/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) \
<<<<<<< HEAD
	$(PKG_INSTALL_DIR)/usr/sbin/{flash_erase,nanddump,nandwrite,nandtest,mtdinfo} \
	$(PKG_INSTALL_DIR)/usr/lib/mtd-utils/nandbiterrs $(1)/usr/sbin/
=======
	$(PKG_INSTALL_DIR)/usr/sbin/{flash_erase,nanddump,nandwrite,nandtest,mtdinfo} $(1)/usr/sbin/
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call BuildPackage,ubi-utils))
$(eval $(call BuildPackage,nand-utils))
