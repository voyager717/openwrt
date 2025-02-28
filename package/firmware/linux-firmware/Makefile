#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=linux-firmware
<<<<<<< HEAD
PKG_VERSION:=20250211
=======
PKG_VERSION:=20211216
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_RELEASE:=1

PKG_SOURCE_URL:=@KERNEL/linux/kernel/firmware
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
<<<<<<< HEAD
PKG_HASH:=2de1345897bf839d532c5de0fdb348770ca2a5f4edfb21971582597abb45297d
=======
PKG_HASH:=eeddb4e6bef31fd1a3757f12ccc324929bbad97855c0b9ec5ed780f74de1837d
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

SCAN_DEPS = *.mk

include $(INCLUDE_DIR)/package.mk

<<<<<<< HEAD
RSTRIP:=:
STRIP:=:

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
define Package/firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=http://git.kernel.org/cgit/linux/kernel/git/firmware/linux-firmware.git
  TITLE:=$(1)
  DEPENDS:=$(2)
<<<<<<< HEAD
  LICENSE_FILES:=$(3)
  LICENSE:=$(4)
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Build/Compile

endef

include $(wildcard ./*.mk)
#$(eval $(call BuildPackage,linux-firmware))
