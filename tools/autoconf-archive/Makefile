include $(TOPDIR)/rules.mk

PKG_NAME:=autoconf-archive
<<<<<<< HEAD
PKG_VERSION:=2023.02.20

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/autoconf-archive
PKG_HASH:=71d4048479ae28f1f5794619c3d72df9c01df49b1c628ef85fde37596dc31a33

include $(INCLUDE_DIR)/host-build.mk

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
=======
PKG_VERSION:=2019.01.06

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/autoconf-archive
PKG_HASH:=17195c833098da79de5778ee90948f4c5d90ed1a0cf8391b4ab348e2ec511e3f

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--datarootdir=$(STAGING_DIR_HOST)/share

define Host/Compile
	export SHELL="$(BASH)"; $(MAKE) -C $(HOST_BUILD_DIR)
endef

define Host/Install
	export SHELL="$(BASH)"; $(MAKE) -C $(HOST_BUILD_DIR) install
endef

define Host/Clean
	-export SHELL="$(BASH)"; $(MAKE) -C $(HOST_BUILD_DIR) uninstall
	$(call Host/Clean/Default)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call HostBuild))
