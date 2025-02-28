#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=kernel
PKG_FLAGS:=hold

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/packages
<<<<<<< HEAD
SUBTARGETS = $(sort $(filter-out feeds,$(notdir $(wildcard $(TOPDIR)/target/linux/* $(TOPDIR)/target/linux/feeds/*))))
SUBTARGET_MODULES = $(foreach t,$(SUBTARGETS),$(firstword $(wildcard $(TOPDIR)/target/linux/feeds/$(t)/modules.mk $(TOPDIR)/target/linux/$(t)/modules.mk)))
SCAN_DEPS=modules/*.mk $(SUBTARGET_MODULES) $(TOPDIR)/include/netfilter.mk

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING
=======
SCAN_DEPS=modules/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter.mk

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

export SHELL:=/bin/sh
.ONESHELL:
.SHELLFLAGS = -ec

include $(INCLUDE_DIR)/package.mk

ifeq ($(DUMP),)
<<<<<<< HEAD
  STAMP_BUILT:=$(STAMP_BUILT)_$(shell $(SCRIPT_DIR)/kconfig.pl $(LINUX_DIR)/.config | $(MKHASH) md5)
=======
  STAMP_BUILT:=$(STAMP_BUILT)_$(shell $(SCRIPT_DIR)/kconfig.pl $(LINUX_DIR)/.config | mkhash md5)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  -include $(LINUX_DIR)/.config
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

<<<<<<< HEAD
define Build/Quilt
endef

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
define Build/Compile
endef

define KernelPackage/depends
endef

CONFIG_PACKAGE_kernel=y
define Package/kernel
  SECTION:=sys
  CATEGORY:=Kernel
  DEFAULT:=y
  TITLE:=Virtual kernel package
<<<<<<< HEAD
  VERSION:=$(LINUX_VERSION)~$(LINUX_VERMAGIC)-r$(LINUX_RELEASE)
=======
  VERSION:=$(LINUX_VERSION)-$(LINUX_RELEASE)-$(LINUX_VERMAGIC)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  URL:=http://www.kernel.org/
  PKG_FLAGS:=nonshared
endef

define Package/kernel/install
<<<<<<< HEAD
	$(INSTALL_DIR) $(1)/$(MODULES_SUBDIR)
	$(INSTALL_DATA) $(LINUX_DIR)/modules.builtin $(1)/$(MODULES_SUBDIR)
	$(SED) 's,.*/,,' $(1)/$(MODULES_SUBDIR)/modules.builtin
	strings $(LINUX_DIR)/modules.builtin.modinfo | \
		grep -E -v "\.(file$(if CONFIG_MODULE_STRIPPED,|parmtype))=" | \
		tr '\n' '\0' > $(1)/$(MODULES_SUBDIR)/modules.builtin.modinfo
	echo $(LINUX_VERSION)~$(LINUX_VERMAGIC)-r$(LINUX_RELEASE) > $(TMP_DIR)/kernel.version
=======
  # nothing to do
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Package/kernel/extra_provides
	sed -e 's,.*/,,' $(LINUX_DIR)/modules.builtin;
endef

$(eval $(if $(DUMP),,$(call BuildPackage,kernel)))

include $(sort $(wildcard ./modules/*.mk))
<<<<<<< HEAD
-include $(SUBTARGET_MODULES)
=======
-include $(TOPDIR)/target/linux/*/modules.mk
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
