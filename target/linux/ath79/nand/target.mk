BOARDNAME := Generic devices with NAND flash

FEATURES += nand

<<<<<<< HEAD
DEFAULT_PACKAGES += wpad-basic-mbedtls
=======
DEFAULT_PACKAGES += wpad-basic-wolfssl
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Firmware for boards using Qualcomm Atheros, MIPS-based SoCs
	in the ar72xx and subsequent series, with support for NAND flash
endef
