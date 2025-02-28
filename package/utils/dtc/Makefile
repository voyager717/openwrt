# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2016-2019 Yousong Zhou <yszhou4tech@gmail.com>

include $(TOPDIR)/rules.mk

PKG_NAME:=dtc
<<<<<<< HEAD
PKG_VERSION:=1.7.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=9532f10098455711a4da37816fd567dfc8523bb01f59ad6c44887a112e553d9e
PKG_SOURCE_URL:=@KERNEL/software/utils/dtc

PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=GPL
PKG_CPE_ID:=cpe:/a:dtc_project:dtc

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk
=======
PKG_VERSION:=1.6.0
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=10503b0217e1b07933e29e8d347a00015b2431bea5f59afe0bed3af30340c82d
PKG_SOURCE_URL:=@KERNEL/software/utils/dtc
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=GPL
PKG_INSTALL:=1
PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>

include $(INCLUDE_DIR)/package.mk

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Package/dtc
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Device Tree Compiler
  URL:=https://git.kernel.org/pub/scm/utils/dtc/dtc.git
endef

define Package/dtc/description
  Device Tree Compiler for Flat Device Trees Device Tree Compiler, dtc, takes
  as input a device-tree in a given format and outputs a device-tree in another
  format for booting kernels on embedded systems.
endef

<<<<<<< HEAD
define Package/dtc/config
	config DTC_STATIC_BUILD
		depends on PACKAGE_dtc
		bool "Build dtc as static binary"
		default n
		help
			Builds dtc as a static binary.
			This is usefull in order to export live DTS from a device running
			various vendor modified OpenWrt versions.
endef

define Package/dtc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dtc $(1)/usr/bin
=======
define Package/dtc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/bin/dtc $(1)/usr/bin
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef


# See Documentation/manual.txt for details about each utility
define Package/fdt-utils
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Flat Device Tree Utilities
  URL:=https://git.kernel.org/pub/scm/utils/dtc/dtc.git
  DEPENDS:=+libfdt
endef

define Package/fdt-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
<<<<<<< HEAD
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/convert-dtsv0 $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/fdtdump $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/fdtget $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/fdtput $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/fdtoverlay $(1)/usr/bin
=======
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/convert-dtsv0 $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/fdtdump $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/fdtget $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/fdtput $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/fdtoverlay $(1)/usr/bin
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef


define Package/libfdt
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=a utility library for reading and manipulating dtb files
  URL:=https://git.kernel.org/pub/scm/utils/dtc/dtc.git
endef

define Package/libfdt/description
  This is a library containing functions for manipulating Flat Device Trees.
endef

define Package/libfdt/install
	$(INSTALL_DIR) $(1)/usr/lib
<<<<<<< HEAD
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfdt*.so* $(1)/usr/lib
endef

MESON_ARGS += \
	-Dtests=false \
	-Dtools=true \
	-Dyaml=disabled \
	-Dvalgrind=disabled \
	-Dpython=disabled \
	-Dstatic-build=$(if $(CONFIG_DTC_STATIC_BUILD),true,false)
=======
	$(CP) $(PKG_INSTALL_DIR)/lib/libfdt*.so* $(1)/usr/lib
endef

# NO_PYTHON is for disabling pylibfdt
MAKE_FLAGS += \
	PREFIX= \
	NO_PYTHON=1 \
	NO_VALGRIND=1 \
	NO_YAML=1 \
	EXTRA_CFLAGS=$(EXTRA_CFLAGS) \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include
<<<<<<< HEAD
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
=======
	$(CP) $(PKG_INSTALL_DIR)/include/* $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call BuildPackage,dtc))
$(eval $(call BuildPackage,fdt-utils))
$(eval $(call BuildPackage,libfdt))
