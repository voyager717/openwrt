include $(TOPDIR)/rules.mk

PKG_NAME:=umbim
<<<<<<< HEAD
PKG_RELEASE:=25

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/umbim.git
PKG_SOURCE_DATE:=2022-08-13
PKG_SOURCE_VERSION:=146bc77c98ace3d1cc672986669650d2e1da71f3
PKG_MIRROR_HASH:=9eae9f191e9c4a8132dba32e356047b8dfd62556f06800f05a283630cfed472a
=======
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/umbim.git
PKG_SOURCE_DATE:=2019-09-11
PKG_SOURCE_VERSION:=184b707ddaa0acee84d02e0ffe599cb8b67782bd
PKG_MIRROR_HASH:=482ff69144f81fafed99035840f5a24e772472f2df2f3ac0219d6de791ac5835
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_MAINTAINER:=John Crispin <john@phrozen.org>

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

define Package/umbim
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=WWAN
  DEPENDS:=+libubox +kmod-usb-net +kmod-usb-net-cdc-mbim +wwan
  TITLE:=Control utility for mobile broadband modems
endef

define Package/umbim/description
  umbim is a command line tool for controlling mobile broadband modems using
  the MBIM-protocol.
endef

TARGET_CFLAGS += \
<<<<<<< HEAD
	-I$(STAGING_DIR)/usr/include
=======
	-I$(STAGING_DIR)/usr/include -ffunction-sections -fdata-sections

TARGET_LDFLAGS += -Wl,--gc-sections
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Package/umbim/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/umbim $(1)/sbin/
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,umbim))
