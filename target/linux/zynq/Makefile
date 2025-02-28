# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2015 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=zynq
BOARDNAME:=Xilinx Zynq 7000 SoCs
FEATURES:=fpu gpio rtc usb usbgadget boot-part rootfs-part squashfs
CPU_TYPE:=cortex-a9
CPU_SUBTYPE:=neon
<<<<<<< HEAD
SUBTARGETS:=generic
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

# future support SUBTARGETS: for both zynq and zynqmp

define Target/Description
	Build firmware image for Zynq 7000 SoC devices.
endef

<<<<<<< HEAD
KERNEL_PATCHVER:=6.6
=======
KERNEL_PATCHVER:=5.4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=Image dtbs

DEFAULT_PACKAGES += uboot-envtools mkf2fs e2fsprogs \
	kmod-usb-storage kmod-fs-msdos

$(eval $(call BuildTarget))
