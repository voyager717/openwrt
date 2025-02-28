BOARDNAME:=Devices with small flash
<<<<<<< HEAD
FEATURES += low_mem small_flash

DEFAULT_PACKAGES += wpad-basic-mbedtls
=======
FEATURES += small_flash

DEFAULT_PACKAGES += wpad-basic-wolfssl
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build firmware images for Atheros AR71xx/AR913x/AR934x based boards with small flash
endef
