include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=bcm63xx-cfe
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/openwrt/bcm63xx-cfe.git
PKG_SOURCE_PROTO:=git
<<<<<<< HEAD
PKG_SOURCE_DATE:=2024-06-25
PKG_SOURCE_VERSION:=6519bd2dde3535cafeea43157755f4dbef2f90c5
PKG_MIRROR_HASH:=db4da580b7a611a2b4ddd4ff812e5f8ddfd9694b6f5fd8246a341e61967c00ef
=======
PKG_SOURCE_DATE:=2021-06-22
PKG_SOURCE_VERSION:=e5050f37150b34deb547b50feccd0e7439cb5bd7
PKG_MIRROR_HASH:=85fed9f4bdf23cf7d33a02f549ffe9073666890f786d5ffa484c0368552b75ae
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/bcm63xx-cfe
  SECTION:=boot
  CATEGORY:=Boot Loaders
<<<<<<< HEAD
  DEPENDS:=@(TARGET_bcm4908||TARGET_bcm63xx||TARGET_bmips)
=======
  DEPENDS:=@(TARGET_bcm4908||TARGET_bcm63xx)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  TITLE:=bcm63xx-cfe
  DEFAULT:=y
endef

define Package/bcm63xx-cfe/description
  CFE RAM binaries for bcm63xx.
endef

define Build/Compile
	true
endef

define Package/bcm63xx-cfe/install
	true
endef

define Build/InstallDev
	rm -rf $(KERNEL_BUILD_DIR)/$(PKG_NAME)
	mkdir -p $(KERNEL_BUILD_DIR)/$(PKG_NAME)
	$(CP) -r $(PKG_BUILD_DIR)/* $(KERNEL_BUILD_DIR)/$(PKG_NAME)
endef

$(eval $(call BuildPackage,bcm63xx-cfe))
