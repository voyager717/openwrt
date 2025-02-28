# 
# Copyright (C) 2006-2014 OpenWrt.org
# Copyright (C) 2016 LEDE Project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/version.mk
include $(INCLUDE_DIR)/download.mk

override MAKEFLAGS=

SDK_NAME:=$(VERSION_DIST_SANITIZED)-sdk-$(if $(CONFIG_VERSION_FILENAMES),$(VERSION_NUMBER)-)$(BOARD)$(if $(SUBTARGET),-$(SUBTARGET))$(if $(GCCV),_gcc-$(GCCV))$(DIR_SUFFIX).$(HOST_OS)-$(HOST_ARCH)
SDK_BUILD_DIR:=$(BUILD_DIR)/$(SDK_NAME)

STAGING_SUBDIR_HOST := staging_dir/host
STAGING_SUBDIR_TARGET := staging_dir/$(TARGET_DIR_NAME)
STAGING_SUBDIR_TOOLCHAIN := staging_dir/toolchain-$(ARCH)$(ARCH_SUFFIX)_gcc-$(GCCV)_$(LIBC)$(if $(CONFIG_arm),_eabi)

BUNDLER_PATH := $(subst $(space),:,$(filter-out $(TOPDIR)/%,$(subst :,$(space),$(PATH))))
BUNDLER_COMMAND := PATH=$(BUNDLER_PATH) $(XARGS) $(SCRIPT_DIR)/bundle-libraries.sh $(SDK_BUILD_DIR)/$(STAGING_SUBDIR_HOST)

EXCLUDE_DIRS:= \
	*/stamp \
	*/stampfiles \
	*/man \
	*/info \
	*/root-* \
<<<<<<< HEAD
	*.install.clean \
	*.install.flags \
	*.install \
=======
	initial \
	*.install.clean \
	*.install.flags \
	*.install \
	*/doc \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	*/share/locale

SDK_DIRS = \
		$(STAGING_SUBDIR_HOST) \
		$(STAGING_SUBDIR_TOOLCHAIN)

GIT_URL:=$(shell git config --get remote.origin.url 2>/dev/null)
GIT_URL:=$(if $(CONFIG_BUILDBOT),$(filter git://% http://% https://%,$(GIT_URL)),$(GIT_URL))
GIT_COMMIT:=$(shell git rev-parse HEAD 2>/dev/null)
GIT_BRANCH:=$(filter-out master HEAD,$(shell git rev-parse --abbrev-ref HEAD 2>/dev/null))
GIT_TAGNAME:=$(shell git show-ref --tags --dereference 2>/dev/null | sed -ne '/^$(GIT_COMMIT) / { s|^.*/||; s|\^.*||; p }')

<<<<<<< HEAD
BASE_FEED:=$(if $(GIT_URL),src-git-full base $(GIT_URL)$(if $(GIT_BRANCH),;$(GIT_BRANCH),$(if $(GIT_TAGNAME),;$(GIT_TAGNAME))))
BASE_FEED:=$(if $(BASE_FEED),$(BASE_FEED),$(shell cd $(TOPDIR); LC_ALL=C git svn info 2>/dev/null | sed -ne 's/^URL: /src-gitsvn base /p'))
BASE_FEED:=$(if $(BASE_FEED),$(BASE_FEED),$(shell cd $(TOPDIR); LC_ALL=C svn info 2>/dev/null | sed -ne 's/^URL: /src-svn base /p'))
BASE_FEED:=$(if $(BASE_FEED),$(BASE_FEED),src-git-full base $(PROJECT_GIT)/openwrt/openwrt.git$(if $(GIT_BRANCH),;$(GIT_BRANCH),$(if $(GIT_TAGNAME),;$(GIT_TAGNAME))))
=======
BASE_FEED:=$(if $(GIT_URL),src-git base $(GIT_URL)$(if $(GIT_BRANCH),;$(GIT_BRANCH),$(if $(GIT_TAGNAME),;$(GIT_TAGNAME))))
BASE_FEED:=$(if $(BASE_FEED),$(BASE_FEED),$(shell cd $(TOPDIR); LC_ALL=C git svn info 2>/dev/null | sed -ne 's/^URL: /src-gitsvn base /p'))
BASE_FEED:=$(if $(BASE_FEED),$(BASE_FEED),$(shell cd $(TOPDIR); LC_ALL=C svn info 2>/dev/null | sed -ne 's/^URL: /src-svn base /p'))
BASE_FEED:=$(if $(BASE_FEED),$(BASE_FEED),src-git base $(PROJECT_GIT)/openwrt/openwrt.git$(if $(GIT_BRANCH),;$(GIT_BRANCH),$(if $(GIT_TAGNAME),;$(GIT_TAGNAME))))
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

KDIR_BASE = $(patsubst $(TOPDIR)/%,%,$(LINUX_DIR))
KDIR_ARCHES = $(LINUX_KARCH)

# arch/arm64/ includes reference files in arch/arm/, so we'll need both
ifeq ($(LINUX_KARCH),arm64)
  KDIR_ARCHES += arm
endif

KERNEL_FILES_ARCH = \
	Makefile* \
	module.lds \
	Kbuild.platforms \
	*/Platform \
	include \
	*/include \
	scripts \
	kernel/asm-offsets.s \
	kernel/module.lds

<<<<<<< HEAD
ifeq ($(LINUX_KARCH),powerpc)
  KERNEL_FILES_ARCH += lib/crtsavres.o
endif

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
KERNEL_FILES_BASE := \
	.config \
	Makefile \
	scripts \
	tools/objtool \
	include \
	Module.symvers \
	modules.builtin \
	$(foreach arch,$(KDIR_ARCHES),$(addprefix arch/$(arch)/,$(KERNEL_FILES_ARCH)))

KERNEL_FILES := $(patsubst $(TOPDIR)/%,%,$(wildcard $(addprefix $(LINUX_DIR)/,$(KERNEL_FILES_BASE))))

# The kernel source hosts various userspace utilities sources.
# These are packaged separately from the kernel and kernel modules.
# The source has to be included here to be buildable by the SDK.
#
USERSPACE_UTILS_FILES := \
	tools/build \
<<<<<<< HEAD
	tools/leds \
	tools/power/cpupower \
	tools/scripts \
	tools/spi \
	tools/thermal/tmon \
	tools/usb/usbip
=======
	tools/scripts \
	tools/usb/usbip \
	tools/spi
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

USERSPACE_FILES := $(patsubst $(TOPDIR)/%,%,$(wildcard $(addprefix $(LINUX_DIR)/,$(USERSPACE_UTILS_FILES))))

all: compile

<<<<<<< HEAD
$(BIN_DIR)/$(SDK_NAME).tar.zst: clean
=======
$(BIN_DIR)/$(SDK_NAME).tar.xz: clean
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	mkdir -p \
		$(SDK_BUILD_DIR)/dl \
		$(SDK_BUILD_DIR)/package \
		$(SDK_BUILD_DIR)/$(STAGING_SUBDIR_TARGET)/include \
		$(SDK_BUILD_DIR)/$(STAGING_SUBDIR_TARGET)/usr/include

	$(CP) -L $(INCLUDE_DIR) $(SCRIPT_DIR) $(SDK_BUILD_DIR)/
	$(TAR) -cf - -C $(TOPDIR) \
		`cd $(TOPDIR); find $(KDIR_BASE)/ -name \*.ko` \
		`cd $(TOPDIR); find $(KDIR_BASE)/firmware/ -newer $(KDIR_BASE)/firmware/Makefile \
			-type f -name '*.bin' -or -name '*.cis' -or -name '*.csp' -or -name '*.dsp' -or -name '*.fw'` \
		$(foreach exclude,$(EXCLUDE_DIRS),--exclude="$(exclude)") \
		$(SDK_DIRS) $(KERNEL_FILES) | \
		$(TAR) -xf - -C $(SDK_BUILD_DIR)

	$(TAR) -cf - -C $(TOPDIR) $(USERSPACE_FILES) | \
		$(TAR) -xf - -C $(SDK_BUILD_DIR)

	(cd $(SDK_BUILD_DIR); find $(STAGING_SUBDIR_HOST)/bin $(STAGING_SUBDIR_HOST)/usr/bin \
<<<<<<< HEAD
		$(STAGING_SUBDIR_HOST)/llvm-bpf/bin $(STAGING_SUBDIR_HOST)/llvm-bpf/libexec \
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
		$(STAGING_SUBDIR_TOOLCHAIN)/bin $(STAGING_SUBDIR_TOOLCHAIN)/*/bin $(STAGING_SUBDIR_TOOLCHAIN)/libexec \
		$(KDIR_BASE) \
		-type f | $(BUNDLER_COMMAND))

	@-( \
		find \
			$(SDK_BUILD_DIR)/$(STAGING_SUBDIR_HOST)/bin \
			$(SDK_BUILD_DIR)/$(STAGING_SUBDIR_HOST)/usr/bin \
<<<<<<< HEAD
			$(SDK_BUILD_DIR)/$(STAGING_SUBDIR_HOST)/llvm-bpf/bin \
			$(SDK_BUILD_DIR)/$(STAGING_SUBDIR_HOST)/llvm-bpf/libexec \
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
			$(SDK_BUILD_DIR)/$(STAGING_SUBDIR_TOOLCHAIN)/bin \
			$(SDK_BUILD_DIR)/$(STAGING_SUBDIR_TOOLCHAIN)/*/bin \
			$(SDK_BUILD_DIR)/$(STAGING_SUBDIR_TOOLCHAIN)/libexec \
			-type f; \
		find \
			$(SDK_BUILD_DIR)/$(STAGING_SUBDIR_HOST)/lib \
<<<<<<< HEAD
			$(SDK_BUILD_DIR)/$(STAGING_SUBDIR_HOST)/llvm-bpf/lib \
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
			$(SDK_BUILD_DIR)/$(STAGING_SUBDIR_HOST)/usr/lib \
			-type f -name \*.so\*; \
	) | xargs strip 2>/dev/null >/dev/null

	mkdir -p $(SDK_BUILD_DIR)/target/linux
	$(CP) $(GENERIC_PLATFORM_DIR) $(PLATFORM_DIR) $(SDK_BUILD_DIR)/target/linux/
	rm -rf \
		$(SDK_BUILD_DIR)/target/linux/*/files* \
		$(SDK_BUILD_DIR)/target/linux/*/patches*
	./convert-config.pl $(TOPDIR)/.config > $(SDK_BUILD_DIR)/Config-build.in
	$(CP) -L \
		$(TOPDIR)/LICENSES \
		$(TOPDIR)/COPYING \
		$(TOPDIR)/rules.mk \
		./files/Config.in \
		./files/Makefile \
		./files/include/prepare.mk \
<<<<<<< HEAD
		./files/README.md \
=======
		./files/README.SDK \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
		$(SDK_BUILD_DIR)/
	mkdir -p $(SDK_BUILD_DIR)/package/kernel
	$(CP) \
		$(TOPDIR)/package/Makefile \
		$(TOPDIR)/package/libs/toolchain \
		$(SDK_BUILD_DIR)/package/
	$(CP) \
		$(TOPDIR)/package/kernel/linux \
		$(SDK_BUILD_DIR)/package/kernel/

	-rm -rf $(SDK_BUILD_DIR)/$(STAGING_SUBDIR_HOST)/.prereq-build
<<<<<<< HEAD
	-rm -rf $(SDK_BUILD_DIR)/$(STAGING_SUBDIR_HOST)/doc
	-rm -rf $(SDK_BUILD_DIR)/$(STAGING_SUBDIR_HOST)/share/doc
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

	-rm -f $(SDK_BUILD_DIR)/feeds.conf.default
	$(if $(BASE_FEED),echo "$(BASE_FEED)" > $(SDK_BUILD_DIR)/feeds.conf.default)
	if [ -f $(TOPDIR)/feeds.conf ]; then \
		cat $(TOPDIR)/feeds.conf >> $(SDK_BUILD_DIR)/feeds.conf.default; \
	else \
		cat $(TOPDIR)/feeds.conf.default >> $(SDK_BUILD_DIR)/feeds.conf.default; \
	fi
	$(SED) 's,^# REVISION:=.*,REVISION:=$(REVISION),g' $(SDK_BUILD_DIR)/include/version.mk
	$(SED) 's,^# SOURCE_DATE_EPOCH:=.*,SOURCE_DATE_EPOCH:=$(SOURCE_DATE_EPOCH),g' $(SDK_BUILD_DIR)/include/version.mk
	$(SED) '/LINUX_VERMAGIC:=/ { s,unknown,$(LINUX_VERMAGIC),g }' $(SDK_BUILD_DIR)/include/kernel.mk
	find $(SDK_BUILD_DIR) -name .git | $(XARGS) rm -rf
	find $(SDK_BUILD_DIR) -name .svn | $(XARGS) rm -rf
	find $(SDK_BUILD_DIR) -name CVS | $(XARGS) rm -rf
	-make -C $(SDK_BUILD_DIR)/scripts/config clean
<<<<<<< HEAD

	(cd $(BUILD_DIR); \
		tar -I '$(STAGING_DIR_HOST)/bin/zstd -T0 --ultra -20' -cf $@ $(SDK_NAME) \
=======
	(cd $(BUILD_DIR); \
		tar -I '$(STAGING_DIR_HOST)/bin/xz -7e -T$(if $(filter 1,$(NPROC)),2,0)' -cf $@ $(SDK_NAME) \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
		--mtime="$(shell date --date=@$(SOURCE_DATE_EPOCH))"; \
	)

download:
prepare:
<<<<<<< HEAD
compile: $(BIN_DIR)/$(SDK_NAME).tar.zst
install: compile

clean:
	rm -rf $(SDK_BUILD_DIR) $(BIN_DIR)/$(SDK_NAME).tar.zst
=======
compile: $(BIN_DIR)/$(SDK_NAME).tar.xz
install: compile

clean:
	rm -rf $(SDK_BUILD_DIR) $(BIN_DIR)/$(SDK_NAME).tar.xz
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
