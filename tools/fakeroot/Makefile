#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=fakeroot
<<<<<<< HEAD
PKG_VERSION:=1.37
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_SOURCE_URL:=@DEBIAN/pool/main/f/fakeroot
PKG_HASH:=9831cc912bc1da6dadac15699c5a07a82c00d6f0dd5c15ec02e20908dd527d3a
=======
PKG_VERSION:=1.25.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_SOURCE_URL:=http://deb.debian.org/debian/pool/main/f/fakeroot
PKG_HASH:=8e903683357f7f5bcc31b879fd743391ad47691d4be33d24a76be3b6c21e956c
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_VARS += \
	ac_cv_header_sys_capability_h=no \
	ac_cv_func_capset=no \
	CPP="$(HOSTCC) -E"

HOST_CONFIGURE_ARGS += \
	--with-ipc=tcp

<<<<<<< HEAD
define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
$(eval $(call HostBuild))
