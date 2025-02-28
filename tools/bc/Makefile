# 
<<<<<<< HEAD
# Copyright (C) 2013-2022 OpenWrt.org
=======
# Copyright (C) 2013 OpenWrt.org
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=bc
<<<<<<< HEAD
PKG_VERSION:=1.08.1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/bc
PKG_HASH:=515430115b3334c636317503460a0950dff79940aa3259ce2c1aa67c2881d023

PKG_FIXUP := autoreconf
PKG_CPE_ID:=cpe:/a:gnu:bc

include $(INCLUDE_DIR)/host-build.mk

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

=======
PKG_VERSION:=1.06.95

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://alpha.gnu.org/gnu/bc \
	http://gnualpha.uib.no/bc/ \
	http://mirrors.fe.up.pt/pub/gnu-alpha/bc/ \
	http://www.nic.funet.fi/pub/gnu/alpha/gnu/bc/
PKG_HASH:=7ee4abbcfac03d8a6e1a8a3440558a3d239d6b858585063e745c760957725ecc

PKG_FIXUP := autoreconf

include $(INCLUDE_DIR)/host-build.mk

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
$(eval $(call HostBuild))
