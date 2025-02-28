# SPDX-License-Identifier: GPL-2.0

include $(TOPDIR)/rules.mk

PKG_VERSION:=2.2
<<<<<<< HEAD
PKG_RELEASE:=2
=======
PKG_RELEASE:=$(AUTORELEASE)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/bcm63xx/atf.git
PKG_SOURCE_DATE:=2021-12-24
PKG_SOURCE_VERSION:=e6d46baf3fae79f693f90bf34f7284c3dfc64aef
<<<<<<< HEAD
PKG_MIRROR_HASH:=5646abc01152210a8181455151f01145001d61b3fb09619b1cee48008e36b0fa

PKG_MAINTAINER:=Rafał Miłecki <rafal@milecki.pl>

include $(INCLUDE_DIR)/kernel.mk
=======
PKG_MIRROR_HASH:=9d5d04f572b1b6ddc6eb3064b9cb09f5fe982e82d350790041d35316349af124

PKG_MAINTAINER:=Rafał Miłecki <rafal@milecki.pl>

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk

define Trusted-Firmware-A/Default
  PLAT:=bcm
  DEFAULT:=y
endef

define Trusted-Firmware-A/bcm4908
  BUILD_TARGET:=bcm4908
  NAME:=BCM4908
  BRCM_CHIP=4908
  TFA_IMAGE:=bl31.bin
endef

TFA_TARGETS:= \
	bcm4908

TFA_MAKE_FLAGS += \
<<<<<<< HEAD
	$(if $(CONFIG_BINUTILS_VERSION_2_37)$(CONFIG_BINUTILS_VERSION_2_38),,LDFLAGS="-no-warn-rwx-segments") \
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	BRCM_CHIP=$(BRCM_CHIP)

define Package/trusted-firmware-a/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/$(PLAT)/release/$(TFA_IMAGE) $(STAGING_DIR_IMAGE)/
endef

$(eval $(call BuildPackage/Trusted-Firmware-A))
