BOARDNAME:=Generic
FEATURES+=pcmcia

<<<<<<< HEAD
DEFAULT_PACKAGES += wpad-basic-mbedtls
=======
DEFAULT_PACKAGES += wpad-basic-wolfssl
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build generic firmware for all Broadcom BCM47xx and BCM53xx MIPS
	devices. It runs on both architectures BMIPS3300 and MIPS 74K.
endef
