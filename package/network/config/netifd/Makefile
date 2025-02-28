include $(TOPDIR)/rules.mk

PKG_NAME:=netifd
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/netifd.git
<<<<<<< HEAD
PKG_SOURCE_DATE:=2024-12-17
PKG_SOURCE_VERSION:=ea01ed41f3212ecbe000422f3c122a01b93fe874
PKG_MIRROR_HASH:=dbaad26c1f9b15d0caff6ccdf80d85b34d96bda72cbd2b1dc188a04136d96c28
=======
PKG_SOURCE_DATE:=2021-10-30
PKG_SOURCE_VERSION:=8f82742ca4f47f459284f3a07323d04da72ea5f6
PKG_MIRROR_HASH:=5e519bb1aec9bb30782213f32f19f12e874c909e42826618dd4332ded816d2fe
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

<<<<<<< HEAD
PKG_BUILD_FLAGS:=lto
=======
PKG_BUILD_PARALLEL:=1
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/netifd
  SECTION:=base
  CATEGORY:=Base system
<<<<<<< HEAD
  DEPENDS:=+libuci +libnl-tiny +libubus +ubus +ubusd +jshn +libubox +libudebug +ucode +ucode-mod-fs
=======
  DEPENDS:=+libuci +libnl-tiny +libubus +ubus +ubusd +jshn +libubox
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  TITLE:=OpenWrt Network Interface Configuration Daemon
endef

define Package/netifd/conffiles
/etc/udhcpc.user
/etc/udhcpc.user.d/
endef

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
<<<<<<< HEAD
	-I$(STAGING_DIR)/usr/include
=======
	-I$(STAGING_DIR)/usr/include \
	-flto

TARGET_LDFLAGS += -flto -fuse-linker-plugin
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

CMAKE_OPTIONS += \
	-DLIBNL_LIBS=-lnl-tiny \
	-DDEBUG=1

define Package/netifd/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/netifd $(1)/sbin/
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/etc/udhcpc.user.d/
<<<<<<< HEAD
	$(CP) \
		$(PKG_BUILD_DIR)/scripts/utils.sh \
		$(PKG_BUILD_DIR)/scripts/netifd-proto.sh \
		$(1)/lib/netifd/
=======
	$(CP) $(PKG_BUILD_DIR)/scripts/* $(1)/lib/netifd/
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call BuildPackage,netifd))
