#
# Copyright 2019 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tfa-layerscape
<<<<<<< HEAD
PKG_VERSION:=6.6.23.2.0.0
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/nxp-qoriq/atf
PKG_SOURCE_VERSION:=lf-6.6.23-2.0.0
PKG_MIRROR_HASH:=628a95ba60a593ae0575ee9ede1154445ec3a86a07b18e4947e06e13c2b67859
PKG_BUILD_DEPENDS:=tfa-layerscape/host

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/trusted-firmware-a.mk
=======
PKG_VERSION:=LSDK-20.04-update-290520
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://source.codeaurora.org/external/qoriq/qoriq-components/atf
PKG_SOURCE_VERSION:=7d748e6f0ec652ba7c43733dc67a3d0b0217390a
PKG_MIRROR_HASH:=d209c9ad18aac9f18375450b98b8dab00f0382ccb485df14623bf9b72ea1dd9b
PKG_BUILD_DEPENDS:=tfa-layerscape/host

include $(INCLUDE_DIR)/host-build.mk
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
include $(INCLUDE_DIR)/package.mk

HOST_CFLAGS += -Wall -Werror -pedantic -std=c99
define Host/Compile
	$(MAKE) -C \
		$(HOST_BUILD_DIR)/tools/fiptool \
<<<<<<< HEAD
		PLAT_FIPTOOL_HELPER_MK="$(HOST_BUILD_DIR)/tools/fiptool/plat_fiptool/nxp/plat_fiptool.mk" \
		CFLAGS="$(HOST_CFLAGS)" \
		LDFLAGS="$(HOST_LDFLAGS)" \
		HOSTCCFLAGS="$(HOST_CFLAGS)"
	$(MAKE) -C \
		$(HOST_BUILD_DIR)/tools/nxp/create_pbl \
=======
		CFLAGS="$(HOST_CFLAGS)" \
		LDFLAGS="$(HOST_LDFLAGS)"
	$(MAKE) -C \
		$(HOST_BUILD_DIR)/plat/nxp/tools \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
		CFLAGS="$(HOST_CFLAGS)"
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/tools/fiptool/fiptool $(STAGING_DIR_HOST)/bin/fiptool-layerscape
<<<<<<< HEAD
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/tools/nxp/create_pbl/create_pbl $(STAGING_DIR_HOST)/bin/tfa-create-pbl
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/tools/nxp/create_pbl/byte_swap $(STAGING_DIR_HOST)/bin/tfa-byte-swap
endef

define Trusted-Firmware-A/Default
  BUILD_TARGET:=layerscape
  BUILD_SUBTARGET:=armv8_64b
  DEPENDS:=+layerscape-rcw +u-boot-fsl_$(1)
endef

define Trusted-Firmware-A/ls1012a-frdm
  NAME:=NXP LS1012AFRDM
=======
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/plat/nxp/tools/create_pbl $(STAGING_DIR_HOST)/bin/tfa-create-pbl
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/plat/nxp/tools/byte_swap $(STAGING_DIR_HOST)/bin/tfa-byte-swap
endef

define Package/tfa-generic
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_layerscape_armv8_64b +layerscape-rcw +u-boot-fsl_$(subst tfa-,,$(1))
  VARIANT:=$(subst tfa-,,$(1))
endef

define Package/tfa-ls1012a-frdm
  $(Package/tfa-generic)
  TITLE:=NXP LS1012AFRDM Trusted Firmware
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  PLAT:=ls1012afrdm
  BOOT_MODE:=qspi
endef

<<<<<<< HEAD
define Trusted-Firmware-A/ls1012a-rdb
  NAME:=NXP LS1012ARDB
=======
define Package/tfa-ls1012a-rdb
  $(Package/tfa-generic)
  TITLE:=NXP LS1012ARDB Trusted Firmware
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  PLAT:=ls1012ardb
  BOOT_MODE:=qspi
endef

<<<<<<< HEAD
define Trusted-Firmware-A/ls1012a-frwy-sdboot
  NAME:=NXP LS1012AFRWY
=======
define Package/tfa-ls1012a-frwy-sdboot
  $(Package/tfa-generic)
  TITLE:=NXP LS1012AFRWY Trusted Firmware
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  PLAT:=ls1012afrwy
  BOOT_MODE:=qspi
endef

<<<<<<< HEAD
define Trusted-Firmware-A/ls1028a-rdb
  TITLE:=NXP LS1028ARDB
  PLAT:=ls1028ardb
  BOOT_MODE:=flexspi_nor
endef

define Trusted-Firmware-A/ls1028a-rdb-sdboot
  TITLE:=NXP LS1028ARDB SD Boot
  PLAT:=ls1028ardb
  BOOT_MODE:=sd
endef

define Trusted-Firmware-A/ls1043a-rdb
  NAME:=NXP LS1043ARDB
=======
define Package/tfa-ls1043a-rdb
  $(Package/tfa-generic)
  TITLE:=NXP LS1043ARDB Trusted Firmware
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  PLAT:=ls1043ardb
  BOOT_MODE:=nor
endef

<<<<<<< HEAD
define Trusted-Firmware-A/ls1043a-rdb-sdboot
  NAME:=NXP LS1043ARDB SD Boot
=======
define Package/tfa-ls1043a-rdb-sdboot
  $(Package/tfa-generic)
  TITLE:=NXP LS1043ARDB SD Boot Trusted Firmware
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  PLAT:=ls1043ardb
  BOOT_MODE:=sd
endef

<<<<<<< HEAD
define Trusted-Firmware-A/ls1046a-frwy
  NAME:=NXP LS1046AFRWY
=======
define Package/tfa-ls1046a-frwy
  $(Package/tfa-generic)
  TITLE:=NXP LS1046AFRWY Trusted Firmware
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  PLAT:=ls1046afrwy
  BOOT_MODE:=qspi
endef

<<<<<<< HEAD
define Trusted-Firmware-A/ls1046a-frwy-sdboot
  NAME:=NXP LS1046AFRWY SD Boot
=======
define Package/tfa-ls1046a-frwy-sdboot
  $(Package/tfa-generic)
  TITLE:=NXP LS1046AFRWY SD Boot Trusted Firmware
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  PLAT:=ls1046afrwy
  BOOT_MODE:=sd
endef

<<<<<<< HEAD
define Trusted-Firmware-A/ls1046a-rdb
  NAME:=NXP LS1046ARDB
=======
define Package/tfa-ls1046a-rdb
  $(Package/tfa-generic)
  TITLE:=NXP LS1046ARDB Trusted Firmware
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  PLAT:=ls1046ardb
  BOOT_MODE:=qspi
endef

<<<<<<< HEAD
define Trusted-Firmware-A/ls1046a-rdb-sdboot
  NAME:=NXP LS1046ARDB SD Boot
=======
define Package/tfa-ls1046a-rdb-sdboot
  $(Package/tfa-generic)
  TITLE:=NXP LS1046ARDB SD Boot Trusted Firmware
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  PLAT:=ls1046ardb
  BOOT_MODE:=sd
endef

<<<<<<< HEAD
define Trusted-Firmware-A/ls1088a-rdb
  NAME:=NXP LS1088ARDB
=======
define Package/tfa-ls1088a-rdb
  $(Package/tfa-generic)
  TITLE:=NXP LS1088ARDB Trusted Firmware
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  PLAT:=ls1088ardb
  BOOT_MODE:=qspi
endef

<<<<<<< HEAD
define Trusted-Firmware-A/ls1088a-rdb-sdboot
  NAME:=NXP LS1088ARDB SD Boot
=======
define Package/tfa-ls1088a-rdb-sdboot
  $(Package/tfa-generic)
  TITLE:=NXP LS1088ARDB SD Boot Trusted Firmware
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  PLAT:=ls1088ardb
  BOOT_MODE:=sd
endef

<<<<<<< HEAD
define Trusted-Firmware-A/ls2088a-rdb
  NAME:=NXP LS2088ARDB
=======
define Package/tfa-ls2088a-rdb
  $(Package/tfa-generic)
  TITLE:=NXP LS2088ARDB Trusted Firmware
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  PLAT:=ls2088ardb
  BOOT_MODE:=nor
endef

<<<<<<< HEAD
define Trusted-Firmware-A/lx2160a-rdb
  NAME:=NXP LX2160ARDB
=======
define Package/tfa-lx2160a-rdb
  $(Package/tfa-generic)
  TITLE:=NXP LX2160ARDB Trusted Firmware
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  PLAT:=lx2160ardb
  BOOT_MODE:=flexspi_nor
endef

<<<<<<< HEAD
define Trusted-Firmware-A/lx2160a-rdb-sdboot
  NAME:=NXP LX2160ARDB SD Boot
=======
define Package/tfa-lx2160a-rdb-sdboot
  $(Package/tfa-generic)
  TITLE:=NXP LX2160ARDB SD Boot Trusted Firmware
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  PLAT:=lx2160ardb
  BOOT_MODE:=sd
endef

<<<<<<< HEAD
TFA_TARGETS := \
  ls1012a-frdm \
  ls1012a-rdb \
  ls1012a-frwy-sdboot \
  ls1028a-rdb \
  ls1028a-rdb-sdboot \
=======
define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/build/$(PLAT)/release/bl2_$(BOOT_MODE).pbl \
		$(STAGING_DIR_IMAGE)/fsl_$(BUILD_VARIANT)-bl2.pbl
	$(CP) $(PKG_BUILD_DIR)/build/$(PLAT)/release/fip.bin \
		$(STAGING_DIR_IMAGE)/fsl_$(BUILD_VARIANT)-fip.bin
endef

define Build/Compile
	$(eval $(Package/tfa-$(BUILD_VARIANT))) \
	$(MAKE) -C $(PKG_BUILD_DIR) CROSS_COMPILE=$(TARGET_CROSS) \
		fip pbl PLAT=$(PLAT) BOOT_MODE=$(BOOT_MODE) \
		RCW=$(STAGING_DIR_IMAGE)/fsl_$(BUILD_VARIANT)-rcw.bin \
		BL33=$(STAGING_DIR_IMAGE)/fsl_$(BUILD_VARIANT)-uboot.bin \
		FIPTOOL=$(STAGING_DIR_HOST)/bin/fiptool-layerscape \
		CREATE_PBL=$(STAGING_DIR_HOST)/bin/tfa-create-pbl \
		BYTE_SWAP=$(STAGING_DIR_HOST)/bin/tfa-byte-swap
endef

TFAS := \
  ls1012a-frdm \
  ls1012a-rdb \
  ls1012a-frwy-sdboot \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  ls1043a-rdb \
  ls1043a-rdb-sdboot \
  ls1046a-frwy \
  ls1046a-frwy-sdboot \
  ls1046a-rdb \
  ls1046a-rdb-sdboot \
  ls1088a-rdb \
  ls1088a-rdb-sdboot \
  ls2088a-rdb \
  lx2160a-rdb \
  lx2160a-rdb-sdboot

<<<<<<< HEAD
TFA_MAKE_FLAGS += \
	$(if $(CONFIG_BINUTILS_VERSION_2_37)$(CONFIG_BINUTILS_VERSION_2_38),,LDFLAGS="-no-warn-rwx-segments") \
	fip pbl \
	BOOT_MODE=$(BOOT_MODE) \
	RCW=$(STAGING_DIR_IMAGE)/fsl_$(BUILD_VARIANT)-rcw.bin \
	BL33=$(STAGING_DIR_IMAGE)/fsl_$(BUILD_VARIANT)-uboot.bin \
	FIPTOOL=$(STAGING_DIR_HOST)/bin/fiptool-layerscape \
	CREATE_PBL=$(STAGING_DIR_HOST)/bin/tfa-create-pbl \
	BYTE_SWAP=$(STAGING_DIR_HOST)/bin/tfa-byte-swap

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/build/$(PLAT)/release/bl2_$(BOOT_MODE).pbl \
		$(STAGING_DIR_IMAGE)/fsl_$(BUILD_VARIANT)-bl2.pbl
	$(CP) $(PKG_BUILD_DIR)/build/$(PLAT)/release/fip.bin \
		$(STAGING_DIR_IMAGE)/fsl_$(BUILD_VARIANT)-fip.bin
endef

define Package/trusted-firmware-a/install/default
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage/Trusted-Firmware-A))
=======
$(eval $(call HostBuild))
$(foreach tfa,$(TFAS), \
  $(eval $(call BuildPackage,tfa-$(tfa))) \
)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
