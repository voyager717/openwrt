include $(TOPDIR)/rules.mk

PKG_NAME:=uqmi
<<<<<<< HEAD
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/uqmi.git
PKG_SOURCE_DATE:=2024-08-25
PKG_SOURCE_VERSION:=28b48a10dbcd1177095b73c6d8086d10114f49b8
PKG_MIRROR_HASH:=ca4c07775185b873da572d973b9bbce86198d41d921a8d32b990da34e5ffd65d
=======
PKG_RELEASE:=3

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/uqmi.git
PKG_SOURCE_DATE:=2020-11-22
PKG_SOURCE_VERSION:=0a19b5b77140465c29e2afa7d611fe93abc9672f
PKG_MIRROR_HASH:=0a6641f8e167efd21d464b0b2aeb1fec5f974dddcdb8822fbd5d7190d0b741b4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_MAINTAINER:=Matti Laakso <malaakso@elisanet.fi>

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

<<<<<<< HEAD
PKG_FLAGS:=nonshared
PKG_BUILD_FLAGS:=gc-sections
=======
PKG_BUILD_PARALLEL:=1

PKG_FLAGS:=nonshared
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/uqmi
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=WWAN
  DEPENDS:=+libubox +libblobmsg-json +kmod-usb-net +kmod-usb-net-qmi-wwan +wwan
  TITLE:=Control utility for mobile broadband modems
endef

define Package/uqmi/description
  uqmi is a command line tool for controlling mobile broadband modems using
  the QMI-protocol.
endef

TARGET_CFLAGS += \
<<<<<<< HEAD
	-I$(STAGING_DIR)/usr/include \
	-Wno-error=maybe-uninitialized
=======
	-I$(STAGING_DIR)/usr/include -ffunction-sections -fdata-sections

TARGET_LDFLAGS += -Wl,--gc-sections
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

CMAKE_OPTIONS += \
	-DDEBUG=1

define Package/uqmi/install
	$(INSTALL_DIR) $(1)/sbin
<<<<<<< HEAD
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uqmi/uqmi $(1)/sbin/
=======
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uqmi $(1)/sbin/
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,uqmi))
