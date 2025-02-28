#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openssl
<<<<<<< HEAD
PKG_VERSION:=3.0.16
PKG_RELEASE:=1
PKG_BUILD_FLAGS:=no-mips16 gc-sections no-lto

PKG_BUILD_PARALLEL:=1

PKG_BASE:=$(subst $(space),.,$(wordlist 1,2,$(subst .,$(space),$(PKG_VERSION))))
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:= \
	https://www.openssl.org/source/ \
	https://www.openssl.org/source/old/$(PKG_BASE)/ \
	https://github.com/openssl/openssl/releases/download/$(PKG_NAME)-$(PKG_VERSION)/

PKG_HASH:=57e03c50feab5d31b152af2b764f10379aecd8ee92f16c985983ce4a99f7ef86

PKG_LICENSE:=Apache-2.0
=======
PKG_BASE:=1.1.1
PKG_BUGFIX:=q
PKG_VERSION:=$(PKG_BASE)$(PKG_BUGFIX)
PKG_RELEASE:=1
PKG_USE_MIPS16:=0
ENGINES_DIR=engines-1.1

PKG_BUILD_PARALLEL:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:= \
	http://www.openssl.org/source/ \
	http://www.openssl.org/source/old/$(PKG_BASE)/ \
	http://ftp.fi.muni.cz/pub/openssl/source/ \
	http://ftp.fi.muni.cz/pub/openssl/source/old/$(PKG_BASE)/ \
	ftp://ftp.pca.dfn.de/pub/tools/net/openssl/source/ \
	ftp://ftp.pca.dfn.de/pub/tools/net/openssl/source/old/$(PKG_BASE)/

PKG_HASH:=d7939ce614029cdff0b6c20f0e2e5703158a489a72b2507b8bd51bf8c8fd10ca

PKG_LICENSE:=OpenSSL
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Eneas U de Queiroz <cotequeiroz@gmail.com>
PKG_CPE_ID:=cpe:/a:openssl:openssl
PKG_CONFIG_DEPENDS:= \
	CONFIG_OPENSSL_ENGINE \
	CONFIG_OPENSSL_ENGINE_BUILTIN \
	CONFIG_OPENSSL_ENGINE_BUILTIN_AFALG \
	CONFIG_OPENSSL_ENGINE_BUILTIN_DEVCRYPTO \
	CONFIG_OPENSSL_ENGINE_BUILTIN_PADLOCK \
	CONFIG_OPENSSL_NO_DEPRECATED \
	CONFIG_OPENSSL_OPTIMIZE_SPEED \
	CONFIG_OPENSSL_PREFER_CHACHA_OVER_GCM \
<<<<<<< HEAD
	CONFIG_OPENSSL_SMALL_FOOTPRINT \
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	CONFIG_OPENSSL_WITH_ARIA \
	CONFIG_OPENSSL_WITH_ASM \
	CONFIG_OPENSSL_WITH_ASYNC \
	CONFIG_OPENSSL_WITH_BLAKE2 \
	CONFIG_OPENSSL_WITH_CAMELLIA \
	CONFIG_OPENSSL_WITH_CHACHA_POLY1305 \
	CONFIG_OPENSSL_WITH_CMS \
	CONFIG_OPENSSL_WITH_COMPRESSION \
	CONFIG_OPENSSL_WITH_DTLS \
	CONFIG_OPENSSL_WITH_EC2M \
	CONFIG_OPENSSL_WITH_ERROR_MESSAGES \
<<<<<<< HEAD
=======
	CONFIG_OPENSSL_WITH_GOST \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	CONFIG_OPENSSL_WITH_IDEA \
	CONFIG_OPENSSL_WITH_MDC2 \
	CONFIG_OPENSSL_WITH_NPN \
	CONFIG_OPENSSL_WITH_PSK \
	CONFIG_OPENSSL_WITH_RFC3779 \
	CONFIG_OPENSSL_WITH_SEED \
	CONFIG_OPENSSL_WITH_SM234 \
	CONFIG_OPENSSL_WITH_SRP \
	CONFIG_OPENSSL_WITH_SSE2 \
	CONFIG_OPENSSL_WITH_TLS13 \
	CONFIG_OPENSSL_WITH_WHIRLPOOL

include $(INCLUDE_DIR)/package.mk
<<<<<<< HEAD
include $(INCLUDE_DIR)/openssl-module.mk
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

ifneq ($(CONFIG_CCACHE),)
HOSTCC=$(HOSTCC_NOCACHE)
HOSTCXX=$(HOSTCXX_NOCACHE)
endif

define Package/openssl/Default
  TITLE:=Open source SSL toolkit
<<<<<<< HEAD
  URL:=https://www.openssl.org/
=======
  URL:=http://www.openssl.org/
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  SECTION:=libs
  CATEGORY:=Libraries
endef

define Package/libopenssl/config
source "$(SOURCE)/Config.in"
endef

define Package/openssl/Default/description
The OpenSSL Project is a collaborative effort to develop a robust,
commercial-grade, full-featured, and Open Source toolkit implementing the
Transport Layer Security (TLS) protocol as well as a full-strength
general-purpose cryptography library.
endef

define Package/libopenssl
$(call Package/openssl/Default)
  SUBMENU:=SSL
  DEPENDS:=+OPENSSL_WITH_COMPRESSION:zlib \
	   +OPENSSL_ENGINE_BUILTIN_AFALG:kmod-crypto-user \
	   +OPENSSL_ENGINE_BUILTIN_DEVCRYPTO:kmod-cryptodev \
<<<<<<< HEAD
	   +OPENSSL_ENGINE_BUILTIN_PADLOCK:kmod-crypto-hw-padlock \
	   +(arm||armeb||mips||mipsel||powerpc||arc):libatomic
  TITLE+= (libraries)
  ABI_VERSION:=$(firstword $(subst .,$(space),$(PKG_VERSION)))
=======
	   +OPENSSL_ENGINE_BUILTIN_PADLOCK:kmod-crypto-hw-padlock
  TITLE+= (libraries)
  ABI_VERSION:=1.1
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  MENU:=1
endef

define Package/libopenssl/description
$(call Package/openssl/Default/description)
This package contains the OpenSSL shared libraries, needed by other programs.
endef

define Package/openssl-util
  $(call Package/openssl/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libopenssl +libopenssl-conf
  TITLE+= (utility)
endef

define Package/openssl-util/description
$(call Package/openssl/Default/description)
This package contains the OpenSSL command-line utility.
endef

define Package/libopenssl-conf
  $(call Package/openssl/Default)
  SUBMENU:=SSL
  TITLE:=/etc/ssl/openssl.cnf config file
  DEPENDS:=libopenssl
endef

define Package/libopenssl-conf/conffiles
/etc/ssl/openssl.cnf
<<<<<<< HEAD
$(if $(CONFIG_OPENSSL_ENGINE_BUILTIN_DEVCRYPTO),/etc/ssl/modules.cnf.d/devcrypto.cnf)
$(if $(CONFIG_OPENSSL_ENGINE_BUILTIN_PADLOCK),/etc/ssl/modules.cnf.d/padlock.cnf)
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Package/libopenssl-conf/description
$(call Package/openssl/Default/description)
This package installs the OpenSSL configuration file /etc/ssl/openssl.cnf.
endef

<<<<<<< HEAD
ifneq ($(CONFIG_OPENSSL_ENGINE_BUILTIN_PADLOCK)$(CONFIG_OPENSSL_ENGINE_BUILTIN_DEVCRYPTO),)
define Package/libopenssl-conf/postinst
#!/bin/sh

add_engine_config() {
	if [ -z "$${IPKG_INSTROOT}" ] && uci -q get "openssl.$$1" >/dev/null; then
		[ "$$(uci -q get "openssl.$$1.builtin")" = 1 ] && return
		uci set "openssl.$$1.builtin=1" && uci commit openssl
		return
	fi
}

$(if $(CONFIG_OPENSSL_ENGINE_BUILTIN_DEVCRYPTO),add_engine_config devcrypto)
$(if $(CONFIG_OPENSSL_ENGINE_BUILTIN_PADLOCK),add_engine_config padlock)
endef
endif

$(eval $(call Package/openssl/add-provider,legacy))
define Package/libopenssl-legacy
  $(call Package/openssl/Default)
  $(call Package/openssl/module/Default)
  TITLE:=OpenSSL legacy provider
endef

define Package/libopenssl-legacy/description
The OpenSSL legacy provider supplies OpenSSL implementations of algorithms that
have been deemed legacy. Such algorithms have commonly fallen out of use, have
been deemed insecure by the cryptography community, or something similar.  See
https://www.openssl.org/docs/man3.0/man7/OSSL_PROVIDER-legacy.html
endef

$(eval $(call Package/openssl/add-engine,afalg))
define Package/libopenssl-afalg
  $(call Package/openssl/Default)
  $(call Package/openssl/engine/Default)
  TITLE:=AFALG hardware acceleration engine
  DEPENDS += @KERNEL_AIO +PACKAGE_libopenssl-afalg:kmod-crypto-user \
	     @!OPENSSL_ENGINE_BUILTIN
=======
define Package/libopenssl-afalg
  $(call Package/openssl/Default)
  SUBMENU:=SSL
  TITLE:=AFALG hardware acceleration engine
  DEPENDS:=libopenssl @OPENSSL_ENGINE @KERNEL_AIO \
	   +PACKAGE_libopenssl-afalg:kmod-crypto-user +libopenssl-conf @!OPENSSL_ENGINE_BUILTIN
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Package/libopenssl-afalg/description
This package adds an engine that enables hardware acceleration
through the AF_ALG kernel interface.
<<<<<<< HEAD
See https://www.openssl.org/docs/man3.0/man5/config.html#Engine-Configuration
=======
To use it, you need to configure the engine in /etc/ssl/openssl.cnf
See https://www.openssl.org/docs/man1.1.1/man5/config.html#Engine-Configuration-Module
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
and https://openwrt.org/docs/techref/hardware/cryptographic.hardware.accelerators
The engine_id is "afalg"
endef

<<<<<<< HEAD
$(eval $(call Package/openssl/add-engine,devcrypto))
define Package/libopenssl-devcrypto
  $(call Package/openssl/Default)
  $(call Package/openssl/engine/Default)
  TITLE:=/dev/crypto hardware acceleration engine
  DEPENDS += +PACKAGE_libopenssl-devcrypto:kmod-cryptodev @!OPENSSL_ENGINE_BUILTIN
=======
define Package/libopenssl-devcrypto
  $(call Package/openssl/Default)
  SUBMENU:=SSL
  TITLE:=/dev/crypto hardware acceleration engine
  DEPENDS:=libopenssl @OPENSSL_ENGINE +PACKAGE_libopenssl-devcrypto:kmod-cryptodev +libopenssl-conf \
	   @!OPENSSL_ENGINE_BUILTIN
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Package/libopenssl-devcrypto/description
This package adds an engine that enables hardware acceleration
through the /dev/crypto kernel interface.
<<<<<<< HEAD
See https://www.openssl.org/docs/man3.0/man5/config.html#Engine-Configuration
=======
To use it, you need to configure the engine in /etc/ssl/openssl.cnf
See https://www.openssl.org/docs/man1.1.1/man5/config.html#Engine-Configuration-Module
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
and https://openwrt.org/docs/techref/hardware/cryptographic.hardware.accelerators
The engine_id is "devcrypto"
endef

<<<<<<< HEAD
$(eval $(call Package/openssl/add-engine,padlock))
define Package/libopenssl-padlock
  $(call Package/openssl/Default)
  $(call Package/openssl/engine/Default)
  TITLE:=VIA Padlock hardware acceleration engine
  DEPENDS += @TARGET_x86 +PACKAGE_libopenssl-padlock:kmod-crypto-hw-padlock \
	     @!OPENSSL_ENGINE_BUILTIN
=======
define Package/libopenssl-padlock
  $(call Package/openssl/Default)
  SUBMENU:=SSL
  TITLE:=VIA Padlock hardware acceleration engine
  DEPENDS:=libopenssl @OPENSSL_ENGINE @TARGET_x86 +PACKAGE_libopenssl-padlock:kmod-crypto-hw-padlock \
	   +libopenssl-conf @!OPENSSL_ENGINE_BUILTIN
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Package/libopenssl-padlock/description
This package adds an engine that enables VIA Padlock hardware acceleration.
<<<<<<< HEAD
See https://www.openssl.org/docs/man3.0/man5/config.html#Engine-Configuration
=======
To use it, you need to configure it in /etc/ssl/openssl.cnf.
See https://www.openssl.org/docs/man1.1.1/man5/config.html#Engine-Configuration-Module
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
and https://openwrt.org/docs/techref/hardware/cryptographic.hardware.accelerators
The engine_id is "padlock"
endef

<<<<<<< HEAD
OPENSSL_OPTIONS:= shared no-tests
=======
OPENSSL_OPTIONS:= shared
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

ifndef CONFIG_OPENSSL_WITH_BLAKE2
  OPENSSL_OPTIONS += no-blake2
endif

ifndef CONFIG_OPENSSL_WITH_CHACHA_POLY1305
  OPENSSL_OPTIONS += no-chacha no-poly1305
else
  ifdef CONFIG_OPENSSL_PREFER_CHACHA_OVER_GCM
    OPENSSL_OPTIONS += -DOPENSSL_PREFER_CHACHA_OVER_GCM
  endif
endif

ifndef CONFIG_OPENSSL_WITH_ASYNC
  OPENSSL_OPTIONS += no-async
endif

ifndef CONFIG_OPENSSL_WITH_EC2M
  OPENSSL_OPTIONS += no-ec2m
endif

ifndef CONFIG_OPENSSL_WITH_ERROR_MESSAGES
  OPENSSL_OPTIONS += no-err
endif

ifndef CONFIG_OPENSSL_WITH_TLS13
  OPENSSL_OPTIONS += no-tls1_3
endif

ifndef CONFIG_OPENSSL_WITH_ARIA
  OPENSSL_OPTIONS += no-aria
endif

ifndef CONFIG_OPENSSL_WITH_SM234
  OPENSSL_OPTIONS += no-sm2 no-sm3 no-sm4
endif

ifndef CONFIG_OPENSSL_WITH_CAMELLIA
  OPENSSL_OPTIONS += no-camellia
endif

ifndef CONFIG_OPENSSL_WITH_IDEA
  OPENSSL_OPTIONS += no-idea
endif

ifndef CONFIG_OPENSSL_WITH_SEED
  OPENSSL_OPTIONS += no-seed
endif

ifndef CONFIG_OPENSSL_WITH_MDC2
  OPENSSL_OPTIONS += no-mdc2
endif

ifndef CONFIG_OPENSSL_WITH_WHIRLPOOL
  OPENSSL_OPTIONS += no-whirlpool
endif

ifndef CONFIG_OPENSSL_WITH_CMS
  OPENSSL_OPTIONS += no-cms
endif

ifndef CONFIG_OPENSSL_WITH_RFC3779
  OPENSSL_OPTIONS += no-rfc3779
endif

ifdef CONFIG_OPENSSL_NO_DEPRECATED
  OPENSSL_OPTIONS += no-deprecated
endif

ifeq ($(CONFIG_OPENSSL_OPTIMIZE_SPEED),y)
  TARGET_CFLAGS := $(filter-out -O%,$(TARGET_CFLAGS)) -O3
<<<<<<< HEAD
endif

ifeq ($(CONFIG_OPENSSL_SMALL_FOOTPRINT),y)
=======
else
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  OPENSSL_OPTIONS += -DOPENSSL_SMALL_FOOTPRINT
endif

ifdef CONFIG_OPENSSL_ENGINE
  ifdef CONFIG_OPENSSL_ENGINE_BUILTIN
    OPENSSL_OPTIONS += disable-dynamic-engine
    ifndef CONFIG_OPENSSL_ENGINE_BUILTIN_AFALG
      OPENSSL_OPTIONS += no-afalgeng
    endif
    ifdef CONFIG_OPENSSL_ENGINE_BUILTIN_DEVCRYPTO
      OPENSSL_OPTIONS += enable-devcryptoeng
    endif
    ifndef CONFIG_OPENSSL_ENGINE_BUILTIN_PADLOCK
<<<<<<< HEAD
      OPENSSL_OPTIONS += no-padlockeng
=======
      OPENSSL_OPTIONS += no-hw-padlock
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
    endif
  else
    ifdef CONFIG_PACKAGE_libopenssl-devcrypto
      OPENSSL_OPTIONS += enable-devcryptoeng
    endif
    ifndef CONFIG_PACKAGE_libopenssl-afalg
      OPENSSL_OPTIONS += no-afalgeng
    endif
    ifndef CONFIG_PACKAGE_libopenssl-padlock
<<<<<<< HEAD
      OPENSSL_OPTIONS += no-padlockeng
=======
      OPENSSL_OPTIONS += no-hw-padlock
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
    endif
  endif
else
  OPENSSL_OPTIONS += no-engine
endif

<<<<<<< HEAD
=======
ifndef CONFIG_OPENSSL_WITH_GOST
  OPENSSL_OPTIONS += no-gost
endif

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
ifndef CONFIG_OPENSSL_WITH_DTLS
  OPENSSL_OPTIONS += no-dtls
endif

ifdef CONFIG_OPENSSL_WITH_COMPRESSION
  OPENSSL_OPTIONS += zlib-dynamic
else
  OPENSSL_OPTIONS += no-comp
endif

ifndef CONFIG_OPENSSL_WITH_NPN
  OPENSSL_OPTIONS += no-nextprotoneg
endif

ifndef CONFIG_OPENSSL_WITH_PSK
  OPENSSL_OPTIONS += no-psk
endif

ifndef CONFIG_OPENSSL_WITH_SRP
  OPENSSL_OPTIONS += no-srp
endif

ifndef CONFIG_OPENSSL_WITH_ASM
  OPENSSL_OPTIONS += no-asm
endif

ifdef CONFIG_i386
  ifndef CONFIG_OPENSSL_WITH_SSE2
    OPENSSL_OPTIONS += no-sse2
  endif
endif

OPENSSL_TARGET:=linux-$(call qstrip,$(CONFIG_ARCH))-openwrt

<<<<<<< HEAD
STAMP_CONFIGURED := $(STAMP_CONFIGURED)_$(shell echo $(OPENSSL_OPTIONS) | $(MKHASH) md5)
=======
STAMP_CONFIGURED := $(STAMP_CONFIGURED)_$(shell echo $(OPENSSL_OPTIONS) | mkhash md5)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Build/Configure
	(cd $(PKG_BUILD_DIR); \
		./Configure $(OPENSSL_TARGET) \
			--prefix=/usr \
			--libdir=lib \
			--openssldir=/etc/ssl \
			--cross-compile-prefix="$(TARGET_CROSS)" \
<<<<<<< HEAD
			$(TARGET_CFLAGS) \
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
			$(TARGET_CPPFLAGS) \
			$(TARGET_LDFLAGS) \
			$(OPENSSL_OPTIONS) && \
		{ [ -f $(STAMP_CONFIGURED) ] || make clean; } \
	)
endef

<<<<<<< HEAD
TARGET_CFLAGS += $(FPIC)
=======
TARGET_CFLAGS += $(FPIC) -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		SOURCE_DATE_EPOCH=$(SOURCE_DATE_EPOCH) \
		OPENWRT_OPTIMIZATION_FLAGS="$(TARGET_CFLAGS)" \
		$(OPENSSL_MAKEFLAGS) \
		all
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		$(OPENSSL_MAKEFLAGS) \
		install_sw install_ssldirs
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/openssl $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{crypto,ssl}.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/{openssl,libcrypto,libssl}.pc $(1)/usr/lib/pkgconfig/
	[ -n "$(TARGET_LDFLAGS)" ] && $(SED) 's#$(TARGET_LDFLAGS)##g' $(1)/usr/lib/pkgconfig/{openssl,libcrypto,libssl}.pc || true
endef

define Package/libopenssl/install
	$(INSTALL_DIR) $(1)/etc/ssl/certs
	$(INSTALL_DIR) $(1)/etc/ssl/private
	chmod 0700 $(1)/etc/ssl/private
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libcrypto.so.* $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libssl.so.* $(1)/usr/lib/
	$(if $(CONFIG_OPENSSL_ENGINE),$(INSTALL_DIR) $(1)/usr/lib/$(ENGINES_DIR))
endef

define Package/libopenssl-conf/install
<<<<<<< HEAD
	$(INSTALL_DIR) $(1)/etc/ssl/modules.cnf.d $(1)/etc/config $(1)/etc/init.d
	$(CP) $(PKG_INSTALL_DIR)/etc/ssl/openssl.cnf $(1)/etc/ssl/
	$(INSTALL_BIN) ./files/openssl.init $(1)/etc/init.d/openssl
	$(SED) 's!%ENGINES_DIR%!/usr/lib/$(ENGINES_DIR)!' $(1)/etc/init.d/openssl
	touch $(1)/etc/config/openssl
	$(if $(CONFIG_OPENSSL_ENGINE),,
		$(SED) 's!engines = engines_sect!#&!' $(1)/etc/ssl/openssl.cnf)
	$(if $(CONFIG_OPENSSL_ENGINE_BUILTIN_DEVCRYPTO),
		$(CP) ./files/devcrypto.cnf $(1)/etc/ssl/modules.cnf.d/
		echo -e "config engine 'devcrypto'\n\toption enabled '1'\n\toption builtin '1'" >> $(1)/etc/config/openssl)
	$(if $(CONFIG_OPENSSL_ENGINE_BUILTIN_PADLOCK),
		$(CP) ./files/padlock.cnf $(1)/etc/ssl/modules.cnf.d/
		echo -e "\nconfig engine 'padlock'\n\toption enabled '1'\n\toption builtin '1'" >> $(1)/etc/config/openssl)
=======
	$(INSTALL_DIR) $(1)/etc/ssl
	$(CP) $(PKG_INSTALL_DIR)/etc/ssl/openssl.cnf $(1)/etc/ssl/
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Package/openssl-util/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/openssl $(1)/usr/bin/
endef

<<<<<<< HEAD
=======
define Package/libopenssl-afalg/install
	$(INSTALL_DIR) $(1)/usr/lib/$(ENGINES_DIR)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/$(ENGINES_DIR)/afalg.so $(1)/usr/lib/$(ENGINES_DIR)
endef

define Package/libopenssl-devcrypto/install
	$(INSTALL_DIR) $(1)/usr/lib/$(ENGINES_DIR)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/$(ENGINES_DIR)/devcrypto.so $(1)/usr/lib/$(ENGINES_DIR)
endef

define Package/libopenssl-padlock/install
	$(INSTALL_DIR) $(1)/usr/lib/$(ENGINES_DIR)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/$(ENGINES_DIR)/*padlock.so $(1)/usr/lib/$(ENGINES_DIR)
endef

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
$(eval $(call BuildPackage,libopenssl))
$(eval $(call BuildPackage,libopenssl-conf))
$(eval $(call BuildPackage,libopenssl-afalg))
$(eval $(call BuildPackage,libopenssl-devcrypto))
<<<<<<< HEAD
$(eval $(call BuildPackage,libopenssl-legacy))
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
$(eval $(call BuildPackage,libopenssl-padlock))
$(eval $(call BuildPackage,openssl-util))
