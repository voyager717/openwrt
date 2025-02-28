#
# Copyright (C) 2009 OpenWrt.org
#

SUBTARGET:=mt7621
BOARDNAME:=MT7621 based boards
FEATURES+=nand ramdisk rtc usb minor
CPU_TYPE:=24kc
KERNELNAME:=vmlinux vmlinuz
# make Kernel/CopyImage use $LINUX_DIR/vmlinuz
IMAGES_DIR:=../../..

<<<<<<< HEAD
DEFAULT_PACKAGES += wpad-basic-mbedtls uboot-envtools kmod-crypto-hw-eip93
=======
DEFAULT_PACKAGES += wpad-basic-wolfssl
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build firmware images for Ralink MT7621 based boards.
endef

