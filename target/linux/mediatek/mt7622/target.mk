ARCH:=aarch64
SUBTARGET:=mt7622
BOARDNAME:=MT7622
CPU_TYPE:=cortex-a53
<<<<<<< HEAD
DEFAULT_PACKAGES += fitblk kmod-mt7622-firmware wpad-basic-mbedtls uboot-envtools
=======
DEFAULT_PACKAGES += kmod-mt7615e kmod-mt7615-firmware wpad-basic-wolfssl
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
KERNELNAME:=Image dtbs

define Target/Description
	Build firmware images for MediaTek MT7622 ARM based boards.
endef
