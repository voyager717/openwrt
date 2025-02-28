include $(TOPDIR)/rules.mk

PKG_NAME:=ngrokc
PKG_RELEASE:=1
PKG_VERSION:=20170922
PKG_REV:=1fc428f5286064c1d51ec894fecd7e75c2bd0483

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/dosgo/ngrok-c.git
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/ngrokc
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Web Servers/Proxies
	TITLE:=Secure tunnels to localhost(C++ port)
	URL:=https://github.com/dosgo/ngrok-c
	DEPENDS:=+libc +libpthread +libopenssl +libstdcpp
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/Makefile.lede $(PKG_BUILD_DIR)/
endef


MAKE_FLAGS = \
    CXX="$(TARGET_CXX)"

define Build/Compile
	$(call Build/Compile/Default,-f Makefile.lede)
endef

define Package/ngrokc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ngrokc $(1)/usr/bin
endef

$(eval $(call BuildPackage,ngrokc))
