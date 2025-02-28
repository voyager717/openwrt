#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

<<<<<<< HEAD
PKG_VERSION:=2024.10
PKG_RELEASE:=1

PKG_HASH:=b28daf4ac17e43156363078bf510297584137f6df50fced9b12df34f61a92fb0
=======
PKG_VERSION:=2021.01
PKG_RELEASE:=1

PKG_HASH:=b407e1510a74e863b8b5cb42a24625344f0e0c2fc7582d8c866bd899367d0454
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define U-Boot/Default
  BUILD_TARGET:=mvebu
  HIDDEN:=1
endef

define U-Boot/clearfog
  NAME:=SolidRun ClearFog A1
  BUILD_DEVICES:=solidrun_clearfog-base-a1 solidrun_clearfog-pro-a1
  BUILD_SUBTARGET:=cortexa9
<<<<<<< HEAD
  UBOOT_IMAGE:=u-boot-with-spl.kwb
=======
  UBOOT_IMAGE:=u-boot-spl.kwb
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define U-Boot/helios4
  NAME:=Kobol Helios 4
  BUILD_DEVICES:=kobol_helios4
  BUILD_SUBTARGET:=cortexa9
<<<<<<< HEAD
  UBOOT_IMAGE:=u-boot-with-spl.kwb
endef

define U-Boot/omnia
  NAME:=Turris Omnia
  BUILD_DEVICES:=cznic_turris-omnia
  BUILD_SUBTARGET:=cortexa9
  UBOOT_CONFIG:=turris_omnia
  UBOOT_IMAGE:=u-boot-with-spl.kwb
=======
  UBOOT_IMAGE:=u-boot-spl.kwb
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define U-Boot/espressobin
  NAME:=Marvell ESPRESSObin
  BUILD_SUBTARGET:=cortexa53
  UBOOT_CONFIG:=mvebu_espressobin-88f3720
endef

define U-Boot/uDPU
  NAME:=Methode uDPU
  BUILD_SUBTARGET:=cortexa53
endef

<<<<<<< HEAD
define U-Boot/eDPU
  NAME:=Methode eDPU
  BUILD_SUBTARGET:=cortexa53
endef

define U-Boot/rb5009
  NAME:=MikroTik RB5009
  BUILD_SUBTARGET:=cortexa72
  BUILD_DEVICES:=mikrotik_rb5009
  UBOOT_CONFIG:=mvebu_rb5009
  UBOOT_IMAGE:=u-boot.elf
endef

UBOOT_TARGETS:= \
	clearfog \
	helios4 \
	omnia \
	espressobin \
	uDPU \
	eDPU \
	rb5009

UBOOT_CUSTOMIZE_CONFIG := \
	--disable TOOLS_MKEFICAPSULE
=======
UBOOT_TARGETS:= \
	clearfog \
	helios4 \
	espressobin \
	uDPU

Build/Exports:=$(Host/Exports)

define Build/Configure
	# enable additional options beyond <device>_defconfig
	echo CONFIG_CMD_SETEXPR=y >> $(PKG_BUILD_DIR)/configs/$(UBOOT_CONFIG)_defconfig

	$(call Build/Configure/U-Boot)
endef
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Package/u-boot/install
	$(if $(findstring cortexa53,$(BUILD_SUBTARGET)),,$(Package/u-boot/install/default))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-$(UBOOT_IMAGE)
endef

$(eval $(call BuildPackage/U-Boot))
