#
# Copyright (C) 2013-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2020.04
<<<<<<< HEAD
PKG_RELEASE:=6
=======
PKG_RELEASE:=1
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_HASH:=fe732aaf037d9cc3c0909bad8362af366ae964bbdac6913a34081ff4ad565372

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define U-Boot/Default
  BUILD_TARGET:=mxs
<<<<<<< HEAD
  BUILD_SUBTARGET:=generic
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  UBOOT_IMAGE:=u-boot.sb
  DEFAULT:=y
  HIDDEN:=1
endef

define U-Boot/mx23_olinuxino
  NAME:=Olinuxino i.MX233
<<<<<<< HEAD
  BUILD_DEVICES:=olinuxino_maxi olinuxino_micro
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define U-Boot/duckbill
  NAME:=I2SE Duckbill
<<<<<<< HEAD
  BUILD_DEVICES:=i2se_duckbill
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

UBOOT_TARGETS := \
	mx23_olinuxino \
	duckbill

UBOOT_MAKE_FLAGS += $(UBOOT_IMAGE)

<<<<<<< HEAD
define Build/InstallDev
	$(foreach device,$(BUILD_DEVICES), \
		$(INSTALL_DIR) $(STAGING_DIR_IMAGE)/$(device)
	)
	$(foreach device,$(BUILD_DEVICES), \
		$(CP) $(patsubst %,$(PKG_BUILD_DIR)/%,$(UBOOT_IMAGE)) $(STAGING_DIR_IMAGE)/$(device)/
	)
=======
Build/Exports:=$(Host/Exports)

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-$(UBOOT_IMAGE)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call BuildPackage/U-Boot))
