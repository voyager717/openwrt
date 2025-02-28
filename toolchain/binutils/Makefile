#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=binutils
PKG_VERSION:=$(call qstrip,$(CONFIG_BINUTILS_VERSION))
BIN_VERSION:=$(PKG_VERSION)

PKG_SOURCE_URL:=@GNU/binutils/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
<<<<<<< HEAD
PKG_CPE_ID:=cpe:/a:gnu:binutils

TAR_OPTIONS += --exclude='*.rej'

ifeq ($(PKG_VERSION),2.40)
  PKG_HASH:=0f8a4c272d7f17f369ded10a4aca28b8e304828e95526da482b0ccc4dfc9d8e1
endif

ifeq ($(PKG_VERSION),2.42)
  PKG_HASH:=f6e4d41fd5fc778b06b7891457b3620da5ecea1006c6a4a41ae998109f85a800
endif

ifeq ($(PKG_VERSION),2.43.1)
  PKG_HASH:=13f74202a3c4c51118b797a39ea4200d3f6cfbe224da6d1d95bb938480132dfd
=======

TAR_OPTIONS += --exclude='*.rej'

ifeq ($(PKG_VERSION),2.32)
  PKG_HASH:=0ab6c55dd86a92ed561972ba15b9b70a8b9f75557f896446c82e8b36e473ee04
endif

ifeq ($(PKG_VERSION),2.34)
  PKG_HASH:=f00b0e8803dc9bab1e2165bd568528135be734df3fabf8d0161828cd56028952
endif

ifeq ($(PKG_VERSION),2.35.1)
  PKG_HASH:=3ced91db9bf01182b7e420eab68039f2083aed0a214c0424e257eae3ddee8607
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endif

HOST_BUILD_PARALLEL:=1

PATCH_DIR:=./patches/$(PKG_VERSION)

include $(INCLUDE_DIR)/toolchain-build.mk

<<<<<<< HEAD
ifdef CONFIG_GCC_USE_GRAPHITE
  GRAPHITE_CONFIGURE:= --with-isl=$(STAGING_DIR_HOST)
else
  GRAPHITE_CONFIGURE:= --without-isl --without-cloog
endif

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
HOST_CONFIGURE_ARGS = \
	--prefix=$(TOOLCHAIN_DIR) \
	--build=$(GNU_HOST_NAME) \
	--host=$(GNU_HOST_NAME) \
	--target=$(REAL_GNU_TARGET_NAME) \
	--with-sysroot=$(TOOLCHAIN_DIR) \
<<<<<<< HEAD
	--with-system-zlib \
	--with-zstd \
	--enable-deterministic-archives \
	--enable-plugins \
	--enable-lto \
	--disable-gprofng \
=======
	--enable-deterministic-archives \
	--enable-plugins \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	--disable-multilib \
	--disable-werror \
	--disable-nls \
	--disable-sim \
	--disable-gdb \
	$(GRAPHITE_CONFIGURE) \
	$(SOFT_FLOAT_CONFIG_OPTION) \
	$(call qstrip,$(CONFIG_EXTRA_BINUTILS_CONFIG_OPTIONS))

ifneq ($(CONFIG_SSP_SUPPORT),)
  HOST_CONFIGURE_ARGS+= \
		--enable-libssp
else
  HOST_CONFIGURE_ARGS+= \
		--disable-libssp
endif

ifneq ($(CONFIG_EXTRA_TARGET_ARCH),)
  HOST_CONFIGURE_ARGS+= \
		--enable-targets=$(call qstrip,$(CONFIG_EXTRA_TARGET_ARCH_NAME))-linux-$(TARGET_SUFFIX)
endif

<<<<<<< HEAD
HOST_CONFIGURE_VARS += \
	acx_cv_cc_gcc_supports_ada=false

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
define Host/Prepare
	$(call Host/Prepare/Default)
	ln -snf $(notdir $(HOST_BUILD_DIR)) $(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME)
	$(CP) $(SCRIPT_DIR)/config.{guess,sub} $(HOST_BUILD_DIR)/
<<<<<<< HEAD
=======
	$(SED) 's, " Linaro.*,,' $(HOST_BUILD_DIR)/bfd/version.h
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Host/Compile
	+$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR) all
endef

define Host/Install
<<<<<<< HEAD
	$(MAKE) -C $(HOST_BUILD_DIR) \
		install
	$(call FixupLibdir,$(TOOLCHAIN_DIR))
	$(CP) $(TOOLCHAIN_DIR)/bin/$(REAL_GNU_TARGET_NAME)-readelf $(HOST_BUILD_PREFIX)/bin/readelf
=======
	mkdir -p $(TOOLCHAIN_DIR)/initial
	$(MAKE) -C $(HOST_BUILD_DIR) \
		prefix=$(TOOLCHAIN_DIR)/initial \
		install
	$(MAKE) -C $(HOST_BUILD_DIR) \
		prefix=$(TOOLCHAIN_DIR) \
		install
	$(call FixupLibdir,$(TOOLCHAIN_DIR)/initial)
	$(RM) $(TOOLCHAIN_DIR)/initial/lib/libiberty.a
	$(CP) $(TOOLCHAIN_DIR)/bin/$(REAL_GNU_TARGET_NAME)-readelf $(HOST_BUILD_PREFIX)/bin/readelf
	# ARC gcc requires extlib.
	# If extlib is not available in "initial" folder
	# initial gcc will fail to build libc.
	if [ -d $(TOOLCHAIN_DIR)/extlib ]; then \
		$(CP) -r $(TOOLCHAIN_DIR)/extlib $(TOOLCHAIN_DIR)/initial/; \
	fi
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Host/Clean
	rm -rf \
		$(HOST_BUILD_DIR) \
		$(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME)
endef

$(eval $(call HostBuild))
