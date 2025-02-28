#
# Copyright (C) 2016-2017 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libressl
<<<<<<< HEAD
PKG_VERSION:=3.7.3
PKG_HASH:=7948c856a90c825bd7268b6f85674a8dcd254bae42e221781b24e3f8dc335db3
=======
PKG_VERSION:=3.4.3
PKG_HASH:=ff88bffe354818b3ccf545e3cafe454c5031c7a77217074f533271d63c37f08d
PKG_RELEASE:=1
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_CPE_ID:=cpe:/a:openbsd:libressl

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
<<<<<<< HEAD
PKG_SOURCE_URL:=https://mirror.ox.ac.uk/pub/OpenBSD/LibreSSL \
=======
PKG_SOURCE_URL:=http://mirror.ox.ac.uk/pub/OpenBSD/LibreSSL \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	http://ftp.jaist.ac.jp/pub/OpenBSD/LibreSSL \
	https://ftp.openbsd.org/pub/OpenBSD/LibreSSL

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOSTCC := $(HOSTCC_NOCACHE)
<<<<<<< HEAD

HOST_CFLAGS += $(HOST_FPIC)

HOST_CONFIGURE_ARGS += \
	--enable-static \
	--disable-shared \
	--disable-asm \
	--with-pic \
	--disable-tests

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef
=======
HOST_CONFIGURE_ARGS += --enable-static --disable-shared --disable-tests
HOST_CFLAGS += $(HOST_FPIC)

ifeq ($(GNU_HOST_NAME),x86_64-linux-gnux32)
HOST_CONFIGURE_ARGS += --disable-asm
endif
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

$(eval $(call HostBuild))
