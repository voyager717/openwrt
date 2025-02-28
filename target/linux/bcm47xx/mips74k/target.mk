BOARDNAME:=MIPS 74K
CPU_TYPE:=74kc

<<<<<<< HEAD
DEFAULT_PACKAGES += wpad-basic-mbedtls
=======
DEFAULT_PACKAGES += wpad-basic-wolfssl
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build firmware for Broadcom BCM47xx and BCM53xx devices with
	MIPS 74K CPU.
endef
