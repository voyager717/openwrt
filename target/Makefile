# 
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
curdir:=target

$(curdir)/subtargets:=install
<<<<<<< HEAD
$(curdir)/builddirs:=linux sdk imagebuilder toolchain llvm-bpf
$(curdir)/builddirs-default:=linux
$(curdir)/builddirs-install:=\
	linux \
	$(if $(CONFIG_SDK),sdk) \
	$(if $(CONFIG_IB),imagebuilder) \
	$(if $(CONFIG_MAKE_TOOLCHAIN),toolchain) \
	$(if $(CONFIG_SDK_LLVM_BPF),llvm-bpf)
=======
$(curdir)/builddirs:=linux sdk imagebuilder toolchain
$(curdir)/builddirs-default:=linux
$(curdir)/builddirs-install:=linux $(if $(CONFIG_SDK),sdk) $(if $(CONFIG_IB),imagebuilder) $(if $(CONFIG_MAKE_TOOLCHAIN),toolchain)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

$(curdir)/sdk/install:=$(curdir)/linux/install
$(curdir)/imagebuilder/install:=$(curdir)/linux/install

<<<<<<< HEAD
$(curdir)//compile = $(STAGING_DIR)/.prepared $(BIN_DIR)

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
$(eval $(call stampfile,$(curdir),target,prereq,.config))
$(eval $(call stampfile,$(curdir),target,compile,$(TMP_DIR)/.build))
$(eval $(call stampfile,$(curdir),target,install,$(TMP_DIR)/.build))

$($(curdir)/stamp-install): $($(curdir)/stamp-compile) 

$(eval $(call subdir,$(curdir)))
