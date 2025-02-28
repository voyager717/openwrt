ARCH:=mips64
CPU_TYPE:=mips64r2
SUBTARGET:=be64
<<<<<<< HEAD
=======
FEATURES+=source-only
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
BOARDNAME:=Big Endian (64-bits)

define Target/Description
	Build BE firmware images for MIPS Malta CoreLV board running in
	big-endian and 64-bits mode
endef
