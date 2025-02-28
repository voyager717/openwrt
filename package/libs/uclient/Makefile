include $(TOPDIR)/rules.mk

PKG_NAME:=uclient
PKG_RELEASE=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/uclient.git
<<<<<<< HEAD
PKG_MIRROR_HASH:=13af83ef1db3665d38d3c0cf30b5223216b05eaeca4880d7ef7494610ddd7458
PKG_SOURCE_DATE:=2024-10-22
PKG_SOURCE_VERSION:=88ae8f208dd313f69e268234f7db55956aef1cb9
=======
PKG_MIRROR_HASH:=7c443cac02a734dd312c65618f4de17248d188317f30a9fac192c1503b3d5c05
PKG_SOURCE_DATE:=2021-05-14
PKG_SOURCE_VERSION:=6a6011df3429ffa5958d12b1327eeda4fd9daa47
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
CMAKE_INSTALL:=1

PKG_BUILD_DEPENDS:=ustream-ssl

PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
<<<<<<< HEAD
PKG_CONFIG_DEPENDS := CONFIG_PACKAGE_ucode-mod-uclient
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libuclient
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=HTTP/1.1 client library
  ABI_VERSION:=20201210
  DEPENDS:=+libubox
endef

define Package/uclient-fetch
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Tiny wget replacement using libuclient
  ALTERNATIVES:=200:/usr/bin/wget:/bin/uclient-fetch
  PROVIDES:=wget
  DEPENDS:=+libuclient
endef

<<<<<<< HEAD
define Package/ucode-mod-uclient
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=ucode uclient module
  DEPENDS:=+libucode +libuclient
endef

CMAKE_OPTIONS += \
	-DBUILD_UCODE=$(if $(CONFIG_PACKAGE_ucode-mod-uclient),ON,OFF)

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
define Package/libuclient/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libuclient.so $(1)/usr/lib/
endef

define Package/uclient-fetch/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/uclient-fetch $(1)/bin/
endef

<<<<<<< HEAD
define Package/ucode-mod-uclient/install
	$(INSTALL_DIR) $(1)/usr/lib/ucode
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/ucode/*.so $(1)/usr/lib/ucode
endef

$(eval $(call BuildPackage,libuclient))
$(eval $(call BuildPackage,uclient-fetch))
$(eval $(call BuildPackage,ucode-mod-uclient))
=======
$(eval $(call BuildPackage,libuclient))
$(eval $(call BuildPackage,uclient-fetch))
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
