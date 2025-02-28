# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2011 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=i386
BOARD:=x86
BOARDNAME:=x86
<<<<<<< HEAD
FEATURES:=squashfs ext4 vdi vmdk vhdx pcmcia targz fpu boot-part rootfs-part
SUBTARGETS:=generic legacy geode 64

KERNEL_PATCHVER:=6.6
=======
FEATURES:=squashfs ext4 vdi vmdk pcmcia targz fpu boot-part rootfs-part
SUBTARGETS:=generic legacy geode 64

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

KERNELNAME:=bzImage

include $(INCLUDE_DIR)/target.mk

<<<<<<< HEAD
DEFAULT_PACKAGES += \
	partx-utils mkf2fs \
	e2fsprogs kmod-button-hotplug \
	grub2-bios-setup
=======
DEFAULT_PACKAGES += partx-utils mkf2fs e2fsprogs kmod-button-hotplug
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

$(eval $(call BuildTarget))

$(eval $(call $(if $(CONFIG_ISO_IMAGES),SetupHostCommand,Ignore),mkisofs, \
	Please install mkisofs. , \
	mkisofs -v 2>&1 , \
	genisoimage -v 2>&1 | grep genisoimage, \
	xorrisofs -v 2>&1 | grep xorriso \
))
