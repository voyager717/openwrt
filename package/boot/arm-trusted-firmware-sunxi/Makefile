#
# Copyright (C) 2017 Hauke Mehrtens
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

<<<<<<< HEAD
PKG_VERSION:=2.10
PKG_RELEASE:=1

PKG_HASH:=88215a62291b9ba87da8e50b077741103cdc08fb6c9e1ebd34dfaace746d3201
=======
PKG_NAME:=arm-trusted-firmware-sunxi
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/ARM-software/arm-trusted-firmware
PKG_SOURCE_DATE:=2018-10-02
PKG_SOURCE_VERSION:=dbc8d9496ead9ecdd7c2a276b542a4fbbbf64027
PKG_MIRROR_HASH:=c81521a27b86f606e927b4e346286540b862828c8d49292eae1f5c6adfc24001
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=license.md

PKG_MAINTAINER:=Hauke Mehrtens <hauke@hauke-m.de>

<<<<<<< HEAD
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk

define Trusted-Firmware-A/Default
	BUILD_TARGET:=sunxi
	BUILD_SUBTARGET:=cortexa53
endef

define Trusted-Firmware-A/sunxi-a64
	NAME:=Allwinner A64
	PLAT:=sun50i_a64
endef

define Trusted-Firmware-A/sunxi-h6
	NAME:=Allwinner H6
	PLAT:=sun50i_h6
endef

define Trusted-Firmware-A/sunxi-h616
	NAME:=Allwinner H616
	PLAT:=sun50i_h616
endef

TFA_TARGETS:= \
	sunxi-a64 \
	sunxi-h6 \
	sunxi-h616

TFA_MAKE_FLAGS+= \
	$(if $(CONFIG_BINUTILS_VERSION_2_37)$(CONFIG_BINUTILS_VERSION_2_38),,LDFLAGS="-no-warn-rwx-segments")

define Package/trusted-firmware-a/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/$(PLAT)/release/bl31.bin $(STAGING_DIR_IMAGE)/bl31_$(BUILD_VARIANT).bin
endef

$(eval $(call BuildPackage/Trusted-Firmware-A))
=======
include $(INCLUDE_DIR)/package.mk


define Package/arm-trusted-firmware-sunxi
    SECTION:=boot
    CATEGORY:=Boot Loaders
    TITLE:=ARM Trusted Firmware for Allwinner
    DEPENDS:=@TARGET_sunxi_cortexa53
endef

export GCC_HONOUR_COPTS=s

MAKE_VARS = \
	CROSS_COMPILE="$(TARGET_CROSS)"

MAKE_FLAGS += \
	PLAT=sun50i_a64 \
	bl31

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/build/sun50i_a64/release/bl31.bin $(STAGING_DIR_IMAGE)/bl31.bin
endef

define Package/arm-trusted-firmware-sunxi/install
endef

$(eval $(call BuildPackage,arm-trusted-firmware-sunxi))
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
