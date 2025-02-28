#
# Copyright (C) 2007-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tcpdump
<<<<<<< HEAD
PKG_VERSION:=4.99.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.tcpdump.org/release/
PKG_HASH:=8c75856e00addeeadf70dad67c9ff3dd368536b2b8563abf6854d7c764cd3adb
=======
PKG_VERSION:=4.9.3
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.tcpdump.org/release/
PKG_HASH:=2cd47cb3d460b6ff75f4a9940f594317ad456cfbf2bd2c8e5151e16559db6410
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=BSD-3-Clause
PKG_CPE_ID:=cpe:/a:tcpdump:tcpdump

PKG_INSTALL:=1
<<<<<<< HEAD
PKG_BUILD_FLAGS:=gc-sections
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/tcpdump/default
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libpcap
  TITLE:=Network monitoring and data acquisition tool
  URL:=http://www.tcpdump.org/
endef

define Package/tcpdump
  $(Package/tcpdump/default)
  VARIANT:=full
endef

define Package/tcpdump-mini
  $(Package/tcpdump/default)
  TITLE+= (minimal version)
  VARIANT:=mini
endef

CONFIGURE_ARGS += \
	--without-cap-ng \
	--without-crypto \
	$(call autoconf_bool,CONFIG_IPV6,ipv6)

<<<<<<< HEAD
=======
TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
ifeq ($(BUILD_VARIANT),mini)
  TARGET_CFLAGS += -DTCPDUMP_MINI
  CONFIGURE_ARGS += --disable-smb
  MAKE_FLAGS += TCPDUMP_MINI=1
endif

define Package/tcpdump/install
<<<<<<< HEAD
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/tcpdump $(1)/usr/bin/
=======
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/tcpdump $(1)/usr/sbin/
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

Package/tcpdump-mini/install = $(Package/tcpdump/install)

$(eval $(call BuildPackage,tcpdump))
$(eval $(call BuildPackage,tcpdump-mini))
