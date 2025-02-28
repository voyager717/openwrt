#
# Copyright (C) 2017 Hauke Mehrtens
<<<<<<< HEAD
# Copyright (C) 2021-2023 Daniel Golle
=======
# Copyright (C) 2021 Daniel Golle
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=arm-trusted-firmware-mediatek
<<<<<<< HEAD
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/mtk-openwrt/arm-trusted-firmware.git
PKG_SOURCE_DATE:=2025-02-12
PKG_SOURCE_VERSION:=e090770684e775711a624e68e0b28112227a4c38
PKG_MIRROR_HASH:=1ff9d60f3677aadd9a6716e860985987ba7157bd28ce831112e05c0b3170939a

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk

define Trusted-Firmware-A/Default
  BUILD_TARGET:=mediatek
  TFA_IMAGE:=bl2.img bl31.bin
  HIDDEN:=y
  BOOT_DEVICE:=
  DDR3_FLYBY:=
  DDR_TYPE:=
  NAND_TYPE:=
  BOARD_QFN:=
  DRAM_USE_COMB:=
  RAM_BOOT_UART_DL:=
  USE_UBI:=
=======
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/mtk-openwrt/arm-trusted-firmware.git
PKG_SOURCE_DATE:=2020-11-09
PKG_SOURCE_VERSION:=03017334ccd8c0fac12e7db36749b95b9a7d745f
PKG_MIRROR_HASH:=b211b2f9143d4debc7ad8dc959cb606888af20af790855dd66c87e451b6a1bc7

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

PKG_LICENSE_FILES:=LICENCE.mediatek

BLOBS_TARBALL:=tfa-mtk-files-for-2020-11-09.tgz
BROMIMAGE_EXEC:=bromimage-x64

include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk

PKG_LICENSE+=proprietary

define Download/tfa-files
  URL:=@OPENWRT
  URL_FILE:=$(BLOBS_TARBALL)
  FILE:=$(BLOBS_TARBALL)
  HASH:=689b097e4531d3eeca0c477675ab3dc3cace6ba4ed8a339116a9ede6537839d7
endef

define Download/mt7622-header-emmc
  URL:=https://raw.githubusercontent.com/frank-w/BPI-R64-ATF/a36efa5f7435b8079479d13b562fedc0aa0d42f0
  URL_FILE:=header_emmc.bin
  FILE:=mt7622-header_emmc.bin
  HASH:=0a09c55d90c5fc375c59468a4331555f77cbc3e1dd107ca83b5b8c9625f38d8c
endef

define Download/mt7622-header-sdmmc
  URL:=https://raw.githubusercontent.com/frank-w/BPI-R64-ATF/a36efa5f7435b8079479d13b562fedc0aa0d42f0
  URL_FILE:=header_sdmmc.bin
  FILE:=mt7622-header_sdmmc.bin
  HASH:=242908c04e25289d25ba9fab61a1930425af173051c43d275d1ac9877d6accb1
endef

define Trusted-Firmware-A/Default
  BUILD_TARGET:=mediatek
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  TFA_IMAGE:=bl2.img bl31.bin
  BOOT_DEVICE:=
  DDR_BLOB:=
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Trusted-Firmware-A/mt7622-nor-1ddr
  NAME:=MediaTek MT7622 (SPI-NOR, 1x DDR3)
<<<<<<< HEAD
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=nor
=======
  BOOT_DEVICE:=nor
  DDR_BLOB:=1
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Trusted-Firmware-A/mt7622-nor-2ddr
  NAME:=MediaTek MT7622 (SPI-NOR, 2x DDR3)
<<<<<<< HEAD
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=nor
  DDR3_FLYBY:=1
endef

define Trusted-Firmware-A/mt7622-ram-1ddr
  NAME:=MediaTek MT7622 (RAM, 1x DDR3)
  BOOT_DEVICE:=ram
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  RAM_BOOT_UART_DL:=1
  HIDDEN:=
  DEFAULT:=TARGET_mediatek_mt7622
endef

define Trusted-Firmware-A/mt7622-ram-2ddr
  NAME:=MediaTek MT7622 (RAM, 2x DDR3)
  BOOT_DEVICE:=ram
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  DDR3_FLYBY:=1
  RAM_BOOT_UART_DL:=1
  HIDDEN:=
  DEFAULT:=TARGET_mediatek_mt7622
=======
  BOOT_DEVICE:=nor
  DDR_BLOB:=2
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Trusted-Firmware-A/mt7622-snand-1ddr
  NAME:=MediaTek MT7622 (SPI-NAND, 1x DDR3)
<<<<<<< HEAD
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=snand
endef

define Trusted-Firmware-A/mt7622-snand-ubi-1ddr
  NAME:=MediaTek MT7622 (SPI-NAND using UBI, 1x DDR3)
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=snand
  USE_UBI:=1
endef

define Trusted-Firmware-A/mt7622-snand-2ddr
  NAME:=MediaTek MT7622 (SPI-NAND, 2x DDR3)
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=snand
  DDR3_FLYBY:=1
endef

define Trusted-Firmware-A/mt7622-snand-ubi-2ddr
  NAME:=MediaTek MT7622 (SPI-NAND using UBI, 2x DDR3)
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=snand
  DDR3_FLYBY:=1
  USE_UBI:=1
=======
  BOOT_DEVICE:=snand
  DDR_BLOB:=1
endef

define Trusted-Firmware-A/mt7622-snand-2ddr
  NAME:=MediaTek MT7622 (SPI-SNAND, 2x DDR3)
  BOOT_DEVICE:=snand
  DDR_BLOB:=2
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Trusted-Firmware-A/mt7622-emmc-1ddr
  NAME:=MediaTek MT7622 (eMMC, 1x DDR3)
<<<<<<< HEAD
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=emmc
=======
  BOOT_DEVICE:=emmc
  DDR_BLOB:=1
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Trusted-Firmware-A/mt7622-emmc-2ddr
  NAME:=MediaTek MT7622 (eMMC, 2x DDR3)
<<<<<<< HEAD
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=emmc
  DDR3_FLYBY:=1
endef

define Trusted-Firmware-A/mt7622-sdmmc-1ddr
  NAME:=MediaTek MT7622 (SD card, 1x DDR3)
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=sdmmc
endef

define Trusted-Firmware-A/mt7622-sdmmc-2ddr
  NAME:=MediaTek MT7622 (SD card, 2x DDR3)
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=sdmmc
  DDR3_FLYBY:=1
endef

define Trusted-Firmware-A/mt7981-ram-ddr4
  NAME:=MediaTek MT7981 (RAM, DDR4)
  BOOT_DEVICE:=ram
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7981
  DDR_TYPE:=ddr4
  RAM_BOOT_UART_DL:=1
  HIDDEN:=
  DEFAULT:=TARGET_mediatek_filogic
endef

define Trusted-Firmware-A/mt7981-emmc-ddr4
  NAME:=MediaTek MT7981 (eMMC, DDR4)
  BOOT_DEVICE:=emmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7981
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7981-spim-nand-ddr4
  NAME:=MediaTek MT7981 (SPI-NAND via SPIM, DDR4)
  BOOT_DEVICE:=spim-nand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7981
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7981-nor-ddr3
  NAME:=MediaTek MT7981 (SPI-NOR, DDR3)
  BOOT_DEVICE:=nor
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7981
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7981-ram-ddr3
  NAME:=MediaTek MT7981 (RAM, DDR3)
  BOOT_DEVICE:=ram
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7981
  DDR_TYPE:=ddr3
  RAM_BOOT_UART_DL:=1
  HIDDEN:=
  DEFAULT:=TARGET_mediatek_filogic
endef

define Trusted-Firmware-A/mt7981-nor-ddr4
  NAME:=MediaTek MT7981 (SPI-NOR, DDR4)
  BOOT_DEVICE:=nor
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7981
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7981-emmc-ddr3
  NAME:=MediaTek MT7981 (eMMC, DDR3)
  BOOT_DEVICE:=emmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7981
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7981-sdmmc-ddr3
  NAME:=MediaTek MT7981 (SD card, DDR3)
  BOOT_DEVICE:=sdmmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7981
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7981-snand-ddr3
  NAME:=MediaTek MT7981 (SPI-NAND via SNFI, DDR3)
  BOOT_DEVICE:=snand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7981
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7981-spim-nand-ddr3
  NAME:=MediaTek MT7981 (SPI-NAND via SPIM, DDR3)
  BOOT_DEVICE:=spim-nand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7981
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7986-ram-ddr4
  NAME:=MediaTek MT7986 (RAM, DDR4)
  BOOT_DEVICE:=ram
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr4
  RAM_BOOT_UART_DL:=1
  HIDDEN:=
  DEFAULT:=TARGET_mediatek_filogic
endef

define Trusted-Firmware-A/mt7981-spim-nand-ubi-ddr4
  NAME:=MediaTek MT7981 (SPI-NAND via SPIM, DDR4)
  BOOT_DEVICE:=spim-nand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7981
  DDR_TYPE:=ddr4
  USE_UBI:=1
endef

define Trusted-Firmware-A/mt7986-nor-ddr4
  NAME:=MediaTek MT7986 (SPI-NOR, DDR4)
  BOOT_DEVICE:=nor
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7986-emmc-ddr4
  NAME:=MediaTek MT7986 (eMMC, DDR4)
  BOOT_DEVICE:=emmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7986-sdmmc-ddr4
  NAME:=MediaTek MT7986 (SD card, DDR4)
  BOOT_DEVICE:=sdmmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7986-snand-ddr4
  NAME:=MediaTek MT7986 (SPI-NAND via SNFI, DDR4)
  BOOT_DEVICE:=snand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7986-spim-nand-ddr4
  NAME:=MediaTek MT7986 (SPI-NAND via SPIM, DDR4)
  BOOT_DEVICE:=spim-nand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr4
  NAND_TYPE:=spim:2k+64
endef

define Trusted-Firmware-A/mt7986-spim-nand-ubi-ddr4
  NAME:=MediaTek MT7986 (SPI-NAND via SPIM using UBI, DDR4)
  BOOT_DEVICE:=spim-nand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr4
  NAND_TYPE:=spim:2k+64
  USE_UBI:=1
endef

define Trusted-Firmware-A/mt7986-spim-nand-4k-ddr4
  NAME:=MediaTek MT7986 (SPI-NAND via SPIM, DDR4)
  BOOT_DEVICE:=spim-nand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr4
  NAND_TYPE:=spim:4k+256
endef

define Trusted-Firmware-A/mt7986-ram-ddr3
  NAME:=MediaTek MT7986 (RAM, DDR3)
  BOOT_DEVICE:=ram
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr3
  RAM_BOOT_UART_DL:=1
  HIDDEN:=
  DEFAULT:=TARGET_mediatek_filogic
endef

define Trusted-Firmware-A/mt7986-nor-ddr3
  NAME:=MediaTek MT7986 (SPI-NOR, DDR3)
  BOOT_DEVICE:=nor
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7986-emmc-ddr3
  NAME:=MediaTek MT7986 (eMMC, DDR3)
  BOOT_DEVICE:=emmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7986-sdmmc-ddr3
  NAME:=MediaTek MT7986 (SD card, DDR3)
  BOOT_DEVICE:=sdmmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7986-snand-ddr3
  NAME:=MediaTek MT7986 (SPI-NAND via SNFI, DDR3)
  BOOT_DEVICE:=snand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7986-spim-nand-ddr3
  NAME:=MediaTek MT7986 (SPI-NAND via SPIM, DDR3)
  BOOT_DEVICE:=spim-nand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7986-spim-nand-ubi-ddr3
  NAME:=MediaTek MT7986 (SPI-NAND via SPIM using UBI, DDR3)
  BOOT_DEVICE:=spim-nand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr3
  USE_UBI:=1
endef

define Trusted-Firmware-A/mt7988-nor-ddr3
  NAME:=MediaTek MT7988 (SPI-NOR, DDR3)
  BOOT_DEVICE:=nor
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7988-emmc-ddr3
  NAME:=MediaTek MT7988 (eMMC, DDR3)
  BOOT_DEVICE:=emmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7988-sdmmc-ddr3
  NAME:=MediaTek MT7988 (SD card, DDR3)
  BOOT_DEVICE:=sdmmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7988-snand-ddr3
  NAME:=MediaTek MT7988 (SPI-NAND via SNFI, DDR3)
  BOOT_DEVICE:=snand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7988-spim-nand-ddr3
  NAME:=MediaTek MT7988 (SPI-NAND via SPIM, DDR3)
  BOOT_DEVICE:=spim-nand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7988-nor-ddr4
  NAME:=MediaTek MT7988 (SPI-NOR, DDR4)
  BOOT_DEVICE:=nor
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7988-emmc-ddr4
  NAME:=MediaTek MT7988 (eMMC, DDR4)
  BOOT_DEVICE:=emmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7988-sdmmc-ddr4
  NAME:=MediaTek MT7988 (SD card, DDR4)
  BOOT_DEVICE:=sdmmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7988-snand-ddr4
  NAME:=MediaTek MT7988 (SPI-NAND via SNFI, DDR4)
  BOOT_DEVICE:=snand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7988-spim-nand-ddr4
  NAME:=MediaTek MT7988 (SPI-NAND via SPIM, DDR4)
  BOOT_DEVICE:=spim-nand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7988-ram-comb
  NAME:=MediaTek MT7988 (RAM)
  BOOT_DEVICE:=ram
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DRAM_USE_COMB:=1
  RAM_BOOT_UART_DL:=1
  HIDDEN:=
  DEFAULT:=TARGET_mediatek_filogic
endef

define Trusted-Firmware-A/mt7988-nor-comb
  NAME:=MediaTek MT7988 (SPI-NOR)
  BOOT_DEVICE:=nor
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DRAM_USE_COMB:=1
endef

define Trusted-Firmware-A/mt7988-emmc-comb
  NAME:=MediaTek MT7988 (eMMC)
  BOOT_DEVICE:=emmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DRAM_USE_COMB:=1
endef

define Trusted-Firmware-A/mt7988-sdmmc-comb
  NAME:=MediaTek MT7988 (SD card)
  BOOT_DEVICE:=sdmmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DRAM_USE_COMB:=1
endef

define Trusted-Firmware-A/mt7988-snand-comb
  NAME:=MediaTek MT7988 (SPI-NAND via SNFI)
  BOOT_DEVICE:=snand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DRAM_USE_COMB:=1
endef

define Trusted-Firmware-A/mt7988-snand-ubi-comb
  NAME:=MediaTek MT7988 (SPI-NAND via SNFI, UBI)
  BOOT_DEVICE:=snand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DRAM_USE_COMB:=1
  USE_UBI:=1
endef

define Trusted-Firmware-A/mt7988-spim-nand-comb
  NAME:=MediaTek MT7988 (SPI-NAND via SPIM)
  BOOT_DEVICE:=spim-nand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DRAM_USE_COMB:=1
endef

define Trusted-Firmware-A/mt7988-spim-nand-ubi-comb
  NAME:=MediaTek MT7988 (SPI-NAND via SPIM, UBI)
  BOOT_DEVICE:=spim-nand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7988
  DRAM_USE_COMB:=1
  USE_UBI:=1
endef

TFA_TARGETS:= \
	mt7622-nor-1ddr \
	mt7622-nor-2ddr \
	mt7622-ram-1ddr \
	mt7622-ram-2ddr \
	mt7622-snand-1ddr \
	mt7622-snand-ubi-1ddr \
	mt7622-snand-2ddr \
	mt7622-snand-ubi-2ddr \
	mt7622-emmc-1ddr \
	mt7622-emmc-2ddr \
	mt7622-sdmmc-1ddr \
	mt7622-sdmmc-2ddr \
	mt7981-ram-ddr3 \
	mt7981-emmc-ddr3 \
	mt7981-nor-ddr3 \
	mt7981-nor-ddr4 \
	mt7981-sdmmc-ddr3 \
	mt7981-snand-ddr3 \
	mt7981-spim-nand-ddr3 \
	mt7981-spim-nand-ubi-ddr4 \
	mt7981-ram-ddr4 \
	mt7981-emmc-ddr4 \
	mt7981-spim-nand-ddr4 \
	mt7986-ram-ddr3 \
	mt7986-emmc-ddr3 \
	mt7986-nor-ddr3 \
	mt7986-sdmmc-ddr3 \
	mt7986-snand-ddr3 \
	mt7986-spim-nand-ddr3 \
	mt7986-spim-nand-ubi-ddr3 \
	mt7986-ram-ddr4 \
	mt7986-emmc-ddr4 \
	mt7986-nor-ddr4 \
	mt7986-sdmmc-ddr4 \
	mt7986-snand-ddr4 \
	mt7986-spim-nand-ddr4 \
	mt7986-spim-nand-ubi-ddr4 \
	mt7986-spim-nand-4k-ddr4 \
	mt7988-emmc-ddr3 \
	mt7988-nor-ddr3 \
	mt7988-sdmmc-ddr3 \
	mt7988-snand-ddr3 \
	mt7988-spim-nand-ddr3 \
	mt7988-emmc-ddr4 \
	mt7988-nor-ddr4 \
	mt7988-sdmmc-ddr4 \
	mt7988-snand-ddr4 \
	mt7988-spim-nand-ddr4 \
	mt7988-ram-comb \
	mt7988-emmc-comb \
	mt7988-nor-comb \
	mt7988-sdmmc-comb \
	mt7988-snand-comb \
	mt7988-snand-ubi-comb \
	mt7988-spim-nand-comb \
	mt7988-spim-nand-ubi-comb

TFA_MAKE_FLAGS += \
	BOOT_DEVICE=$(BOOT_DEVICE) \
	USE_MKIMAGE=1 MKIMAGE=$(STAGING_DIR_HOST)/bin/mkimage \
	$(if $(findstring ddr4,$(DDR_TYPE)),DRAM_USE_DDR4=1) \
	$(if $(BOARD_QFN),BOARD_QFN=1,BOARD_BGA=1) \
	$(if $(NAND_TYPE),NAND_TYPE=$(NAND_TYPE)) \
	HAVE_DRAM_OBJ_FILE=yes \
	$(if $(DDR3_FLYBY),DDR3_FLYBY=1) \
	$(if $(DRAM_USE_COMB),DRAM_USE_COMB=1) \
	$(if $(RAM_BOOT_UART_DL),RAM_BOOT_UART_DL=1) \
	$(if $(USE_UBI),UBI=1 $(if $(findstring mt7622,$(PLAT)),OVERRIDE_UBI_START_ADDR=0x80000)) \
	$(if $(USE_UBI),UBI=1 $(if $(findstring mt7981,$(PLAT)),OVERRIDE_UBI_START_ADDR=0x100000)) \
	$(if $(USE_UBI),UBI=1 $(if $(findstring mt7986,$(PLAT)),OVERRIDE_UBI_START_ADDR=0x200000)) \
	$(if $(RAM_BOOT_UART_DL),bl2,all)

define Package/trusted-firmware-a-ram/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/$(PLAT)/release/bl2.bin $(BIN_DIR)/$(BUILD_VARIANT)-bl2.bin
endef
Package/trusted-firmware-a-mt7622-ram-1ddr/install = $(Package/trusted-firmware-a-ram/install)
Package/trusted-firmware-a-mt7622-ram-2ddr/install = $(Package/trusted-firmware-a-ram/install)
Package/trusted-firmware-a-mt7981-ram-ddr3/install = $(Package/trusted-firmware-a-ram/install)
Package/trusted-firmware-a-mt7981-ram-ddr4/install = $(Package/trusted-firmware-a-ram/install)
Package/trusted-firmware-a-mt7986-ram-ddr3/install = $(Package/trusted-firmware-a-ram/install)
Package/trusted-firmware-a-mt7986-ram-ddr4/install = $(Package/trusted-firmware-a-ram/install)
Package/trusted-firmware-a-mt7988-ram-comb/install = $(Package/trusted-firmware-a-ram/install)

define Package/trusted-firmware-a/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/$(PLAT)/release/bl2.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-bl2.img
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/$(PLAT)/release/bl31.bin $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-bl31.bin
=======
  BOOT_DEVICE:=emmc
  DDR_BLOB:=2
endef

define Trusted-Firmware-A/mt7622-sdmmc-1ddr
  NAME:=MediaTek MT7622 (SDcard, 1x DDR3)
  BOOT_DEVICE:=sdmmc
  DDR_BLOB:=1
endef

define Trusted-Firmware-A/mt7622-sdmmc-2ddr
  NAME:=MediaTek MT7622 (SDcard, 2x DDR3)
  BOOT_DEVICE:=sdmmc
  DDR_BLOB:=2
endef

TFA_TARGETS:= \
        mt7622-nor-1ddr \
        mt7622-nor-2ddr \
        mt7622-snand-1ddr \
        mt7622-snand-2ddr \
        mt7622-emmc-1ddr \
        mt7622-emmc-2ddr \
        mt7622-sdmmc-1ddr \
        mt7622-sdmmc-2ddr

TFA_MAKE_FLAGS += BOOT_DEVICE=$(BOOT_DEVICE) all

define Build/Prepare
	$(call Build/Prepare/Default)
ifeq ($(BOOT_DEVICE),emmc)
	$(eval $(call Download,mt7622-header-emmc))
endif
ifeq ($(BOOT_DEVICE),sdmmc)
	$(eval $(call Download,mt7622-header-sdmmc))
endif
	$(eval $(call Download,tfa-files))
# replace 'bromimage' tool by static version
	$(TAR) -vxzf $(DL_DIR)/$(BLOBS_TARBALL) --wildcards \
		-O "*/$(BROMIMAGE_EXEC)" > $(PKG_BUILD_DIR)/tools/mediatek/bromimage/bromimage
	$(TAR) -vxzf $(DL_DIR)/$(BLOBS_TARBALL) --wildcards \
		-C $(PKG_BUILD_DIR) \
		--strip-components=1 */LICENCE.mediatek
endef

define Build/Configure
	$(call Build/Configure/Default)
# replace DRAM calib blobs if needed (variant '2' is shipped upstream)
ifeq ($(DDR_BLOB),1)
	$(TAR) -vxzf $(DL_DIR)/$(BLOBS_TARBALL) --wildcards \
		-C $(PKG_BUILD_DIR)/plat/mediatek/mt7622/drivers/dram/release \
		--strip-components=1 */*.o
endif
endef

define Package/trusted-firmware-a/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/mt7622/release/bl2.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-bl2.img
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/mt7622/release/bl31.bin $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-bl31.bin
ifeq ($(BOOT_DEVICE),emmc)
	$(INSTALL_DATA) $(DL_DIR)/mt7622-header_emmc.bin $(STAGING_DIR_IMAGE)/
endif
ifeq ($(BOOT_DEVICE),sdmmc)
	$(INSTALL_DATA) $(DL_DIR)/mt7622-header_sdmmc.bin $(STAGING_DIR_IMAGE)/
endif
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call BuildPackage/Trusted-Firmware-A))
