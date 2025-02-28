# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=mips64
BOARD:=octeon
BOARDNAME:=Cavium Networks Octeon
FEATURES:=squashfs ramdisk pci usb
CPU_TYPE:=octeonplus

<<<<<<< HEAD
KERNEL_PATCHVER:=6.6
=======
KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build firmware images for Cavium Networks Octeon-based boards.
endef

include $(INCLUDE_DIR)/target.mk

<<<<<<< HEAD
DEFAULT_PACKAGES += mkf2fs e2fsprogs kmod-usb-dwc3-octeon
=======
DEFAULT_PACKAGES += mkf2fs e2fsprogs
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

$(eval $(call BuildTarget))
