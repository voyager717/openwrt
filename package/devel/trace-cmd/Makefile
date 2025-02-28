include $(TOPDIR)/rules.mk

PKG_NAME:=trace-cmd
<<<<<<< HEAD
PKG_VERSION:=3.3
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=$(PKG_NAME)-v$(PKG_VERSION)
PKG_SOURCE_URL:=https://git.kernel.org/pub/scm/utils/trace-cmd/trace-cmd
PKG_MIRROR_HASH:=7a4f9c3a18a01012cd76ab9a0a2c4447aed8293d005679d5228ef2aef243445c
=======
PKG_VERSION:=v2.9.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://git.kernel.org/pub/scm/linux/kernel/git/rostedt/trace-cmd.git/snapshot/
PKG_HASH:=9404fc3cf540ea795304608068c9db6cdb38b90584c7f3c43249785913d54b38
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
<<<<<<< HEAD
PKG_BUILD_FLAGS:=no-mips16
=======
PKG_USE_MIPS16:=0
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/trace-cmd
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Linux trace command line utility
<<<<<<< HEAD
  DEPENDS:=+libtracefs +zlib
=======
  DEPENDS:=
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Package/trace-cmd-extra
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Extra plugins for trace-cmd
<<<<<<< HEAD
  DEPENDS:=+libtraceevent-extra
=======
  DEPENDS:=
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

MAKE_FLAGS += \
	NO_PYTHON=1 \
	NO_AUDIT=1 \
<<<<<<< HEAD
	NO_LIBZSTD=1 \
	prefix=/usr

define Package/trace-cmd/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib/traceevent/plugins
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/trace-cmd $(1)/usr/bin/
=======
	prefix=/usr

PLUGINS_DIR := $(PKG_INSTALL_DIR)/usr/lib/traceevent/plugins
PLUGINS_MAIN := function hrtimer mac80211 sched_switch

TARGET_CFLAGS += --std=gnu99 -D_GNU_SOURCE

define Package/trace-cmd/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib/traceevent/plugins
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/trace-cmd $(1)/usr/bin/
	$(CP) \
		$(patsubst %,$(PLUGINS_DIR)/plugin_%.so,$(PLUGINS_MAIN)) \
		$(1)/usr/lib/traceevent/plugins
endef

define Package/trace-cmd-extra/install
	$(INSTALL_DIR) $(1)/usr/lib/traceevent/plugins
	$(CP) \
		$$(patsubst %,$(PLUGINS_DIR)/plugin_%.so, \
			$$(filter-out $(PLUGINS_MAIN), \
				$$(patsubst $(PLUGINS_DIR)/plugin_%.so,%, \
					$$(wildcard $(PLUGINS_DIR)/plugin_*.so)))) \
		$(1)/usr/lib/traceevent/plugins
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

$(eval $(call BuildPackage,trace-cmd))
$(eval $(call BuildPackage,trace-cmd-extra))
