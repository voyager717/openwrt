ARCH:=mipsel
CPU_TYPE:=24kc
SUBTARGET:=le
<<<<<<< HEAD
=======
FEATURES+=source-only
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
BOARDNAME:=Little Endian

define Target/Description
	Build LE firmware images for MIPS Malta CoreLV board running in
	little-endian mode
endef
