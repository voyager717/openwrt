BOARDNAME:=P1010
<<<<<<< HEAD
KERNEL_IMAGES:=simpleImage.br200-wp simpleImage.tl-wdr4900-v1 simpleImage.ws-ap3715i
=======
FEATURES+=nand
KERNELNAME:=simpleImage.tl-wdr4900-v1
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build firmware images for P1010 based boards.
endef

