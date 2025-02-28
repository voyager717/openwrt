#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mdadm
<<<<<<< HEAD
PKG_VERSION:=4.3
=======
PKG_VERSION:=4.1
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/raid/mdadm
<<<<<<< HEAD
PKG_HASH:=416727ae1f1080ea6e3090cea36dd076826fc369151e36ab736557ba92196f9f
=======
PKG_HASH:=ab7688842908d3583a704d491956f31324c3a5fc9f6a04653cb75d19f1934f4a
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_CPE_ID:=cpe:/a:mdadm_project:mdadm

PKG_BUILD_PARALLEL:=1
<<<<<<< HEAD
PKG_BUILD_FLAGS:=gc-sections
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/package.mk

define Package/mdadm
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Disc
  TITLE:=A tool for managing Soft RAID under Linux
  URL:=https://www.kernel.org/pub/linux/utils/raid/mdadm/
  DEPENDS:=+libpthread +kmod-md-mod +kmod-md-raid0 +kmod-md-raid10 +kmod-md-raid1
endef

define Package/mdadm/description
 A tool for managing Linux Software RAID arrays.
 RAID 0, 1 and 10 support included.
 If you need RAID 4,5 or 6 functionality please
 install kmod-md-raid456 .
endef

define Package/mdadm/conffiles
/etc/config/mdadm
endef

TARGET_CFLAGS += \
<<<<<<< HEAD
=======
	-ffunction-sections -fdata-sections \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	-DHAVE_STDINT_H -DNO_COROSYNC -DNO_DLM -DUSE_PTHREADS \
	-DCONFFILE='\"/var/etc/mdadm.conf\"' \
	-DMAP_DIR='\"/var/run/mdadm\"' \
	-DMDMON_DIR='\"/var/run/mdadm\"' \
<<<<<<< HEAD
	-DFAILED_SLOTS_DIR='\"/var/run/mdadm/failed-slots\"' \
	-DNO_LIBUDEV \
	-D_LARGEFILE64_SOURCE

TARGET_CXFLAGS = -DNO_LIBUDEV

MAKE_FLAGS += \
		CHECK_RUN_DIR=0 \
		CXFLAGS="$(TARGET_CXFLAGS)"
=======
	-DFAILED_SLOTS_DIR='\"/var/run/mdadm/failed-slots\"'

TARGET_LDFLAGS += -Wl,--gc-sections

MAKE_VARS += CHECK_RUN_DIR=0
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Build/Compile
	$(call Build/Compile/Default,mdadm)
endef

define Package/mdadm/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mdadm $(1)/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/mdadm.init $(1)/etc/init.d/mdadm
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/mdadm.config $(1)/etc/config/mdadm
endef

$(eval $(call BuildPackage,mdadm))
