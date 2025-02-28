#
# Copyright 2021 Daniel Golle
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=arm-trusted-firmware-tools
<<<<<<< HEAD
PKG_VERSION:=2.12
PKG_RELEASE:=1
PKG_HASH:=b4c047493cac1152203e1ba121ae57267e4899b7bf56eb365e22a933342d31c9
=======
PKG_VERSION:=2.4
PKG_RELEASE:=1
PKG_HASH:=bf3eb3617a74cddd7fb0e0eacbfe38c3258ee07d4c8ed730deef7a175cc3d55b
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_HOST_ONLY:=1

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/trusted-firmware-a-$(PKG_VERSION)

include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/arm-trusted-firmware-tools
  SECTION:=boot
  CATEGORY:=Boot Loaders
  TITLE:=ARM Trusted Firmware tools
  URL:=https://www.trustedfirmware.org
  BUILDONLY:=1
endef

define Host/Compile
	$(MAKE) -C \
		$(HOST_BUILD_DIR)/tools/fiptool \
		CPPFLAGS="$(HOST_CFLAGS)" \
<<<<<<< HEAD
		LDFLAGS="$(HOST_LDFLAGS)" \
		OPENSSL_DIR="$(STAGING_DIR_HOST)"
=======
		LDFLAGS="$(HOST_LDFLAGS)"
	$(MAKE) -C \
		$(HOST_BUILD_DIR)/tools/sptool \
		CPPFLAGS="$(HOST_CFLAGS)" \
		LDFLAGS="$(HOST_LDFLAGS)"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/tools/fiptool/fiptool $(STAGING_DIR_HOST)/bin/
<<<<<<< HEAD
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/tools/sptool/sptool.py $(STAGING_DIR_HOST)/bin/
=======
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/tools/sptool/sptool $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/tools/sptool/sp_mk_generator.py $(STAGING_DIR_HOST)/bin/
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/fiptool
<<<<<<< HEAD
	rm -f $(STAGING_DIR_HOST)/bin/sptool.py $(STAGING_DIR_HOST)/bin/sptool
=======
	rm -f $(STAGING_DIR_HOST)/bin/sptool
	rm -f $(STAGING_DIR_HOST)/bin/sp_mk_generator.py
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call BuildPackage,arm-trusted-firmware-tools))
$(eval $(call HostBuild))
