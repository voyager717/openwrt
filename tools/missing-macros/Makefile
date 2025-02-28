#
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=missing-macros
<<<<<<< HEAD
PKG_RELEASE:=12
=======
PKG_RELEASE:=11
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/host-build.mk

define Host/Configure
endef

define Host/Compile
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/share/aclocal
<<<<<<< HEAD
	$(INSTALL_DATA) $(HOST_BUILD_DIR)/m4/*.m4 $(STAGING_DIR_HOST)/share/aclocal/
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bin/* $(STAGING_DIR_HOST)/bin/
	$(LN) makeinfo $(STAGING_DIR_HOST)/bin/texi2any
	$(LN) makeinfo $(STAGING_DIR_HOST)/bin/texi2pdf
	$(LN) makeinfo $(STAGING_DIR_HOST)/bin/texi2dvi
	$(LN) makeinfo $(STAGING_DIR_HOST)/bin/pdftexi2dvi
	$(LN) makeinfo $(STAGING_DIR_HOST)/bin/texi2html
=======
	$(INSTALL_DATA) ./src/m4/*.m4 $(STAGING_DIR_HOST)/share/aclocal/
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin
	$(INSTALL_BIN) ./src/bin/* $(STAGING_DIR_HOST)/bin/
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call HostBuild))
