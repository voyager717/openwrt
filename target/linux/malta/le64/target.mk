ARCH:=mips64el
CPU_TYPE:=mips64r2
SUBTARGET:=le64
<<<<<<< HEAD
=======
FEATURES+=source-only
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
BOARDNAME:=Little Endian (64-bits)

define Target/Description
	Build LE firmware images for MIPS Malta CoreLV board running in
	little-endian and 64-bits mode
endef
