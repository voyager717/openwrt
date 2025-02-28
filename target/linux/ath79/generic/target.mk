BOARDNAME:=Generic

<<<<<<< HEAD
DEFAULT_PACKAGES += wpad-basic-mbedtls
=======
DEFAULT_PACKAGES += wpad-basic-wolfssl
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build firmware images for generic Atheros AR71xx/AR913x/AR934x based boards.
endef
