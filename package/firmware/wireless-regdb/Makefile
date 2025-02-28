include $(TOPDIR)/rules.mk

PKG_NAME:=wireless-regdb
<<<<<<< HEAD
PKG_VERSION:=2025.02.20
PKG_RELEASE:=1
PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/software/network/wireless-regdb/
PKG_HASH:=57f8e7721cf5a880c13ae0c202edbb21092a060d45f9e9c59bcd2a8272bfa456
=======
PKG_VERSION:=2021.08.28
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/software/network/wireless-regdb/
PKG_HASH:=cff370c410d1e6d316ae0a7fa8ac6278fdf1efca5d3d664aca7cfd2aafa54446
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

include $(INCLUDE_DIR)/package.mk

define Package/wireless-regdb
  PKGARCH:=all
  SECTION:=firmware
  CATEGORY:=Firmware
<<<<<<< HEAD
  URL:=https://git.kernel.org/pub/scm/linux/kernel/git/wens/wireless-regdb.git/
=======
  URL:=https://git.kernel.org/pub/scm/linux/kernel/git/sforshee/wireless-regdb.git/
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  TITLE:=Wireless Regulatory Database
endef

define Build/Compile
	$(STAGING_DIR_HOST)/bin/$(PYTHON) $(PKG_BUILD_DIR)/db2fw.py $(PKG_BUILD_DIR)/regulatory.db $(PKG_BUILD_DIR)/db.txt
endef

define Package/wireless-regdb/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(CP) $(PKG_BUILD_DIR)/regulatory.db $(1)/lib/firmware/
endef

$(eval $(call BuildPackage,wireless-regdb))
