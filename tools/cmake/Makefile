#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=cmake
<<<<<<< HEAD
PKG_VERSION:=3.31.4
PKG_VERSION_MAJOR:=$(word 1,$(subst ., ,$(PKG_VERSION))).$(word 2,$(subst ., ,$(PKG_VERSION)))
=======
PKG_VERSION:=3.19.1
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_RELEASE:=1
PKG_CPE_ID:=cpe:/a:kitware:cmake

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/Kitware/CMake/releases/download/v$(PKG_VERSION)/ \
<<<<<<< HEAD
		https://cmake.org/files/v$(PKG_VERSION_MAJOR)/
PKG_HASH:=a6130bfe75f5ba5c73e672e34359f7c0a1931521957e8393a5c2922c8b0f7f25
=======
		https://cmake.org/files/v3.19/
PKG_HASH:=1d266ea3a76ef650cdcf16c782a317cb4a7aa461617ee941e389cb48738a3aba
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

HOST_BUILD_PARALLEL:=1
HOST_CONFIGURE_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_VARS += \
	CC="$(HOSTCC_NOCACHE)" \
	CXX="$(HOSTCXX_NOCACHE)" \
	MAKEFLAGS="$(HOST_JOBS)" \
<<<<<<< HEAD
	CXXFLAGS="$(HOST_CFLAGS)" \
	MAKE="$(STAGING_DIR_HOST)/bin/ninja"

HOST_CONFIGURE_ARGS := \
	--no-debugger \
	$(if $(MAKE_JOBSERVER),--parallel="$(MAKE_JOBSERVER)") \
	--prefix="$(STAGING_DIR_HOST)" \
	--system-expat \
	--system-liblzma \
	--system-zlib \
	--system-zstd \
	--generator=Ninja

define Host/Compile/Default
	+$(NINJA) -C $(HOST_BUILD_DIR) $(1)
endef

define Host/Install/Default
	+$(NINJA) -C $(HOST_BUILD_DIR) install
endef

define Host/Uninstall/Default
	+$(NINJA) -C $(HOST_BUILD_DIR) uninstall
endef
=======
	CXXFLAGS="$(HOST_CFLAGS)"

HOST_CONFIGURE_ARGS := \
	$(if $(MAKE_JOBSERVER),--parallel="$(MAKE_JOBSERVER)") \
	--prefix=$(STAGING_DIR_HOST)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

ifneq ($(findstring c,$(OPENWRT_VERBOSE)),)
  HOST_MAKE_FLAGS += VERBOSE=1
endif

$(eval $(call HostBuild))
