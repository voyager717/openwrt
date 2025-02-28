#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mkimage
<<<<<<< HEAD
PKG_VERSION:=2025.01
=======
PKG_VERSION:=2021.01
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_SOURCE:=u-boot-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	https://mirror.cyberbits.eu/u-boot \
	https://ftp.denx.de/pub/u-boot \
	ftp://ftp.denx.de/pub/u-boot
<<<<<<< HEAD
PKG_HASH:=cdef7d507c93f1bbd9f015ea9bc21fa074268481405501945abc6f854d5b686f
=======
PKG_HASH:=b407e1510a74e863b8b5cb42a24625344f0e0c2fc7582d8c866bd899367d0454
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/u-boot-$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk

<<<<<<< HEAD
define Host/Configure
	$(MAKE) -C $(HOST_BUILD_DIR) \
		HOSTCFLAGS="$(HOST_CFLAGS)" \
		HOSTLDFLAGS="$(HOST_LDFLAGS)" \
		PKG_CONFIG_EXTRAARGS="--static" \
		V=$(if $(findstring c,$(OPENWRT_VERBOSE)),1) \
		tools-only_config

	sed -i 's/CONFIG_TOOLS_MKEFICAPSULE=y/# CONFIG_TOOLS_MKEFICAPSULE is not set/' $(HOST_BUILD_DIR)/.config
=======
define Host/Prepare
	$(Host/Prepare/Default)
	mkdir -p $(HOST_BUILD_DIR)/include/config
	touch $(HOST_BUILD_DIR)/include/config/auto.conf
	mkdir -p $(HOST_BUILD_DIR)/include/generated/
	touch $(HOST_BUILD_DIR)/include/generated/autoconf.h
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) \
		HOSTCFLAGS="$(HOST_CFLAGS)" \
		HOSTLDFLAGS="$(HOST_LDFLAGS)" \
<<<<<<< HEAD
		PKG_CONFIG_EXTRAARGS="--static" \
		V=$(if $(findstring c,$(OPENWRT_VERBOSE)),1) \
=======
		no-dot-config-targets=tools-only \
		CONFIG_MKIMAGE_DTC_PATH=dtc \
		CONFIG_FIT=y \
		CONFIG_FIT_SIGNATURE=y \
		CONFIG_FIT_SIGNATURE_MAX_SIZE=0x10000000 \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
		tools-only
endef

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/tools/mkimage $(STAGING_DIR_HOST)/bin/
	$(CP) $(HOST_BUILD_DIR)/tools/mkenvimage $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/mkimage
	rm -f $(STAGING_DIR_HOST)/bin/mkenvimage
endef

$(eval $(call HostBuild))
