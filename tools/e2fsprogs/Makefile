#
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=e2fsprogs
PKG_CPE_ID:=cpe:/a:e2fsprogs_project:e2fsprogs
<<<<<<< HEAD
PKG_VERSION:=1.47.2
PKG_HASH:=08242e64ca0e8194d9c1caad49762b19209a06318199b63ce74ae4ef2d74e63c
=======
PKG_VERSION:=1.45.6
PKG_HASH:=ffa7ae6954395abdc50d0f8605d8be84736465afc53b8938ef473fcf7ff44256
PKG_RELEASE:=1
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/kernel/people/tytso/e2fsprogs/v$(PKG_VERSION)/

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

ifneq ($(shell $(HOSTCC) --version | grep clang),)
  HOST_CFLAGS += -D__GNUC_PREREQ\(...\)=0 -Dloff_t=off_t
endif
<<<<<<< HEAD
HOST_CFLAGS += $(HOST_FPIC)

HOST_CONFIGURE_ARGS += \
	--disable-elf-shlibs \
	--disable-libuuid \
	--disable-tls \
	--disable-nls \
	--enable-threads=pth \
	--disable-fuse2fs \
	--with-crond-dir=no

# The following uses pkg-config the wrong way around. Just override it.
HOST_CONFIGURE_VARS += \
	ac_cv_lib_uuid_uuid_generate=yes
=======
HOST_CFLAGS += $(FPIC)

HOST_CONFIGURE_ARGS += \
	--disable-elf-shlibs \
	--enable-libuuid \
	--disable-tls \
	--disable-nls \
	--enable-threads=pth
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Host/Prepare
	$(call Host/Prepare/Default)
	rm -rf $(HOST_BUILD_DIR)/doc
endef

define Host/Install
<<<<<<< HEAD
	$(call Host/Install/Default)
	$(SED) 's|^DIR=.*|DIR=$$$${STAGING_DIR_HOST}/share/et|' $(STAGING_DIR_HOST)/bin/compile_et
	$(SED) 's|^DIR=.*|DIR=$$$${STAGING_DIR_HOST}/share/ss|' $(STAGING_DIR_HOST)/bin/mk_cmds
endef

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
=======
	$(Host/Install/Default)
	$(MAKE) -C $(HOST_BUILD_DIR)/lib/uuid install
	mkdir -p $(STAGING_DIR_HOST)/include/e2fsprogs
	$(CP) $(STAGING_DIR_HOST)/include/uuid $(STAGING_DIR_HOST)/include/e2fsprogs/
	rm -rf $(STAGING_DIR_HOST)/include/uuid
	$(INSTALL_DATA) $(HOST_BUILD_DIR)/lib/uuid/libuuid.a $(STAGING_DIR_HOST)/lib/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/e2fsck
	rm -f $(STAGING_DIR_HOST)/bin/tune2fs
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call HostBuild))
