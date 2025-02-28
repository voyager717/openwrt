include $(TOPDIR)/rules.mk

PKG_NAME:=usign
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/usign.git
PKG_SOURCE_DATE:=2020-05-23
PKG_SOURCE_VERSION:=f1f65026a94137c91b5466b149ef3ea3f20091e9
<<<<<<< HEAD
PKG_MIRROR_HASH:=3157ad9e0ebe0512644fbd00191d97a91809f03c3deca67200845a5d07aa721f
CMAKE_INSTALL:=1
PKG_CHECK_FORMAT_SECURITY:=1
PKG_BUILD_FLAGS:=no-mips16 lto
=======
PKG_MIRROR_HASH:=3f6569a5e63fdfd032976ac0f79d736d3935101ac1b97fb370514b013c5e6bb6
CMAKE_INSTALL:=1
PKG_CHECK_FORMAT_SECURITY:=1
PKG_USE_MIPS16:=0
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

HOST_BUILD_PREFIX:=$(STAGING_DIR_HOST)

define Package/usign
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libubox
  TITLE:=OpenWrt signature verification utility
endef

<<<<<<< HEAD
=======
TARGET_CFLAGS += -flto
TARGET_LDFLAGS += -flto=jobserver

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
CMAKE_OPTIONS += \
	-DUSE_LIBUBOX=on

define Package/usign/install
	$(INSTALL_DIR) $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/usign $(1)/usr/bin
	ln -s usign $(1)/usr/bin/signify
endef

$(eval $(call BuildPackage,usign))
$(eval $(call HostBuild))
