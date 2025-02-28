BOARDNAME:=Devices with NAND flash (Routers)
FEATURES += nand pcie

<<<<<<< HEAD
DEFAULT_PACKAGES += kmod-ath9k swconfig wpad-basic-mbedtls
=======
DEFAULT_PACKAGES += kmod-ath9k swconfig wpad-basic-wolfssl
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build firmware images for APM821XX boards with NAND flash.
	For routers like the MR24 or the WNDR4700.
endef
