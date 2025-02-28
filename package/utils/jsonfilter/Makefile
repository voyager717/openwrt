include $(TOPDIR)/rules.mk

PKG_NAME:=jsonfilter
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/jsonpath.git
<<<<<<< HEAD
PKG_SOURCE_DATE:=2024-01-23
PKG_SOURCE_VERSION:=594cfa86469c005972ba750614f5b3f1af84d0f6
PKG_MIRROR_HASH:=2f455f04fbfcdb4c81cccd23475b47395f847db44aa4bd9a1007b9aa0ab7fd19
=======
PKG_SOURCE_DATE:=2018-02-04
PKG_SOURCE_VERSION:=c7e938d6582a436dddc938539e72dd1320625c54
PKG_MIRROR_HASH:=0601b4d7aa5ee096e99388a57cb0701673ab58fccd6ed2984a2abbd4f846e045
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
CMAKE_INSTALL:=1

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=ISC

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/jsonfilter
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libubox +libjson-c
  TITLE:=OpenWrt JSON filter utility
<<<<<<< HEAD
  URL:=$(PKG_SOURCE_URL)
=======
  URL:=http://git.openwrt.org/?p=project/jsonpath.git
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Package/jsonfilter/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/jsonpath $(1)/usr/bin/jsonfilter
endef

$(eval $(call BuildPackage,jsonfilter))
