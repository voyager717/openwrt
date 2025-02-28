BOARDNAME := MikroTik devices
FEATURES += minor nand
KERNELNAME := vmlinux vmlinuz
IMAGES_DIR := ../../..

<<<<<<< HEAD
DEFAULT_PACKAGES += wpad-basic-mbedtls yafut
=======
DEFAULT_PACKAGES += wpad-basic-wolfssl
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build firmware images for MikroTik devices based on Qualcomm Atheros
	MIPS SoCs (AR71xx, AR72xx, AR91xx, AR93xx, QCA95xx).
endef
