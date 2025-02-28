#
# Copyright (C) 2006-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=gdb
<<<<<<< HEAD
PKG_VERSION:=15.2
=======
PKG_VERSION:=10.1
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/gdb
<<<<<<< HEAD
PKG_HASH:=83350ccd35b5b5a0cba6b334c41294ea968158c573940904f00b92f76345314d
PKG_CPE_ID:=cpe:/a:gnu:gdb
=======
PKG_HASH:=f82f1eceeec14a3afa2de8d9b0d3c91d5a3820e23e0a01bbb70ef9f0276b62c0
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
GDB_DIR:=$(PKG_NAME)-$(PKG_VERSION)

HOST_BUILD_DIR:=$(BUILD_DIR_TOOLCHAIN)/$(GDB_DIR)

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/toolchain-build.mk

<<<<<<< HEAD
export ZSTD_CFLAGS=-I$(STAGING_DIR_HOST)/include -pthread
export ZSTD_LIBS=-L$(STAGING_DIR_HOST)/lib -lzstd -lpthread

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
HOST_CONFIGURE_VARS += \
	acx_cv_cc_gcc_supports_ada=false \
	gdb_cv_func_sigsetjmp=yes

HOST_CONFIGURE_ARGS = \
	--prefix=$(TOOLCHAIN_DIR) \
	--build=$(GNU_HOST_NAME) \
	--host=$(GNU_HOST_NAME) \
	--target=$(REAL_GNU_TARGET_NAME) \
<<<<<<< HEAD
	--with-gmp=$(STAGING_DIR_HOST) \
	--with-mpfr=$(STAGING_DIR_HOST) \
	--with-mpc=$(STAGING_DIR_HOST) \
	--with-expat=$(STAGING_DIR_HOST) \
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	--disable-werror \
	--without-uiout \
	--enable-tui --disable-gdbtk --without-x \
	--without-included-gettext \
	--enable-threads \
<<<<<<< HEAD
=======
	--with-expat \
	--without-python \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	--disable-unit-tests \
	--disable-ubsan \
	--disable-binutils \
	--disable-ld \
	--disable-gas \
	--disable-sim

<<<<<<< HEAD
ifneq ($(CONFIG_GDB_PYTHON),)
  HOST_CONFIGURE_ARGS+= --with-python
else
  HOST_CONFIGURE_ARGS+= --without-python
endif

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
define Host/Install
	mkdir -p $(TOOLCHAIN_DIR)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/gdb/gdb $(TOOLCHAIN_DIR)/bin/$(TARGET_CROSS)gdb
	ln -fs $(TARGET_CROSS)gdb $(TOOLCHAIN_DIR)/bin/$(GNU_TARGET_NAME)-gdb
	strip $(TOOLCHAIN_DIR)/bin/$(TARGET_CROSS)gdb
<<<<<<< HEAD
	mkdir -p $(TOOLCHAIN_DIR)/share/gdb
	-cp -R $(HOST_BUILD_DIR)/gdb/data-directory/python $(TOOLCHAIN_DIR)/share/gdb/
	cp -R $(HOST_BUILD_DIR)/gdb/data-directory/syscalls $(TOOLCHAIN_DIR)/share/gdb/
	cp -R $(HOST_BUILD_DIR)/gdb/data-directory/system-gdbinit $(TOOLCHAIN_DIR)/share/gdb/
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Host/Clean
	rm -rf \
		$(HOST_BUILD_DIR) \
		$(TOOLCHAIN_DIR)/bin/$(TARGET_CROSS)gdb \
		$(TOOLCHAIN_DIR)/bin/$(GNU_TARGET_NAME)-gdb
endef

$(eval $(call HostBuild))
