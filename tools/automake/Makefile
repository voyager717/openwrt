#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=automake
PKG_CPE_ID:=cpe:/a:gnu:automake
<<<<<<< HEAD
PKG_VERSION:=1.16.5
PKG_API_VERSION:=$(word 2,$(subst ., ,$(PKG_VERSION)))

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/automake
PKG_HASH:=07bd24ad08a64bc17250ce09ec56e921d6343903943e99ccf63bbf0705e34605
=======
PKG_VERSION:=1.15.1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/automake
PKG_HASH:=af6ba39142220687c500f79b4aa2f181d9b24e4f8d8ec497cea4ba26c64bedaf
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
<<<<<<< HEAD
=======
	--datarootdir=$(STAGING_DIR_HOST)/share \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	--disable-silent-rules

HOST_CONFIGURE_VARS += \
	PERL="/usr/bin/env perl" \
	am_cv_prog_PERL_ithreads=no

define Host/Configure
<<<<<<< HEAD
	(cd $(HOST_BUILD_DIR); $(AM_TOOL_PATHS) STAGING_DIR_HOST="" ./bootstrap)
=======
	(cd $(HOST_BUILD_DIR); $(AM_TOOL_PATHS) STAGING_DIR="" ./bootstrap)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	$(call Host/Configure/Default)
endef

define Host/Install
	# remove old automake resources to avoid version conflicts
<<<<<<< HEAD
	$(call Host/Uninstall)
	$(call Host/Compile/Default,install)
	mv $(STAGING_DIR_HOST)/bin/aclocal $(STAGING_DIR_HOST)/bin/aclocal.real
	$(INSTALL_BIN) ./files/aclocal $(STAGING_DIR_HOST)/bin
	( \
		api=$(PKG_API_VERSION); \
		while [ "$$$$api" -ge 11 ]; do \
			ln -sf aclocal "$(STAGING_DIR_HOST)/bin/aclocal-1.$$$$api"; \
			api=$$$$(($$$$api - 1)); \
		done; \
	)
endef

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
	rm -rf $(STAGING_DIR_HOST)/share/aclocal-[0-9]*
	rm -rf $(STAGING_DIR_HOST)/share/automake-[0-9]*
=======
	rm -rf $(STAGING_DIR_HOST)/share/aclocal-[0-9]*
	rm -rf $(STAGING_DIR_HOST)/share/automake-[0-9]*
	$(MAKE) -C $(HOST_BUILD_DIR) install
	mv $(STAGING_DIR_HOST)/bin/aclocal $(STAGING_DIR_HOST)/bin/aclocal.real
	$(INSTALL_BIN) ./files/aclocal $(STAGING_DIR_HOST)/bin
	ln -sf aclocal $(STAGING_DIR_HOST)/bin/aclocal-1.9
	ln -sf aclocal $(STAGING_DIR_HOST)/bin/aclocal-1.10
	ln -sf aclocal $(STAGING_DIR_HOST)/bin/aclocal-1.11
	ln -sf aclocal $(STAGING_DIR_HOST)/bin/aclocal-1.11.6
	ln -sf aclocal $(STAGING_DIR_HOST)/bin/aclocal-1.15
endef

define Host/Clean
	-$(MAKE) -C $(HOST_BUILD_DIR) uninstall
	$(call Host/Clean/Default)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call HostBuild))
