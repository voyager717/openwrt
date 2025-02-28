include $(TOPDIR)/rules.mk

PKG_NAME:=adb-enablemodem
<<<<<<< HEAD
PKG_VERSION:=2017.03.05
=======
PKG_VERSION:=2017-03-05
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/adb-enablemodem
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=WWAN
  TITLE:=Enable modem via adb
  DEPENDS:=+adb
endef

define Build/Compile
        true
endef

define Package/adb-enablemodem/install
	$(INSTALL_DIR)	$(1)/etc/init.d
	$(INSTALL_BIN)	./files/adb-enablemodem	$(1)/etc/init.d/adb-enablemodem
endef

$(eval $(call BuildPackage,adb-enablemodem))
