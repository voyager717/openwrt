KERNEL_LOADADDR := 0x48080000

define Device/mt7981-spim-nor-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-spim-nor-rfb
  DEVICE_DTS := mt7981-spim-nor-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-spim-nor-rfb
endef
TARGET_DEVICES += mt7981-spim-nor-rfb

define Device/mt7981-spim-nand-2500wan-gmac2
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-spim-nand-2500wan-gmac2
  DEVICE_DTS := mt7981-spim-nand-2500wan-gmac2
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-spim-snand-2500wan-gmac2-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-2500wan-gmac2

define Device/mt7981-spim-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-spim-nand-rfb
  DEVICE_DTS := mt7981-spim-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-spim-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-rfb

define Device/mt7981-spim-nand-rfb-z8102ax
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := ZBT-Z8102AX
  DEVICE_DTS := mt7981-spim-nand-rfb-z8102ax
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-spim-snand-rfb-z8102ax
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-rfb-z8102ax

define Device/mt7981-spim-nand-rfb-z8102ax-w
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := ZBT-Z8102AX-W
  DEVICE_DTS := mt7981-spim-nand-rfb-z8102ax
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-spim-snand-rfb-z8102ax-w
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-rfb-z8102ax-w

define Device/mt7981-spim-nand-rfb-z8102ax-dsim
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := ZBT-Z8102AX-DSIM
  DEVICE_DTS := mt7981-spim-nand-rfb-z8102ax
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-spim-snand-rfb-z8102ax-dsim
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-rfb-z8102ax-dsim

define Device/mt7981-spim-nand-rfb-z8102ax
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := ZBT-Z8102AX-CAPI
  DEVICE_DTS := mt7981-spim-nand-rfb-z8102ax
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-spim-snand-rfb-z8102ax
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-rfb-z8102ax

define Device/z8102ax-gsw
  DEVICE_VENDOR := ZbtLink
  DEVICE_MODEL := ZBT-Z8102AX-GSW
  DEVICE_DTS := ZBT-Z8102AX-GSW
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := zbtlink,z8102ax-gsw
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += z8102ax-gsw

define Device/mt7981-spim-nand-rfb-z8103ax
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := ZBT-Z8103AX
  DEVICE_DTS := mt7981-spim-nand-rfb-z8103ax
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-spim-snand-rfb-z8103ax
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-rfb-z8103ax

define Device/z8103ax-pro
  DEVICE_VENDOR := ZbtLink
  DEVICE_MODEL := ZBT-Z8103AX PRO(NAND)
  DEVICE_DTS := ZBT-Z8103AX-PRO
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := zbtlink,z8103ax-pro | mediatek,mt7981-spim-snand-rfb-z8103ax
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += z8103ax-pro

define Device/mt7981-spim-nand-gsw-rfb-z8104ax
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := ZBT-Z8104AX-GSW
  DEVICE_DTS := mt7981-spim-nand-gsw-rfb-z8104ax
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-rfb,ubi
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-gsw-rfb-z8104ax

define Device/mt7981-spim-nand-rfb-z8105ax
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := ZBT-Z8105AX-DSIM
  DEVICE_DTS := mt7981-spim-nand-rfb-z8105ax
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-rfb,ubi
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-rfb-z8105ax

define Device/mt7981-spim-nand-gsw-rfb-z8105ax-dsim
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := ZBT-Z8105AX-DSIM-GSW
  DEVICE_DTS := mt7981-spim-nand-gsw-rfb-z8105ax-dsim
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-rfb,ubi
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-gsw-rfb-z8105ax-dsim

define Device/z8105ax-w-pro
  DEVICE_VENDOR := ZbtLink
  DEVICE_MODEL := ZBT-Z8105AX-W PRO(NAND)
  DEVICE_DTS := ZBT-Z8105AX-W-PRO
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := zbtlink,z8105ax-w-pro | mediatek,mt7981-spim-snand-rfb-z8105ax
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += z8105ax-w-pro

define Device/mt7981-spim-nand-rfb-z8106ax
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := ZBT-Z8106AX-DSIM
  DEVICE_DTS := mt7981-spim-nand-rfb-z8106ax
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-rfb,ubi
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-rfb-z8106ax

define Device/mt7981-spim-nand-rfb-mofi6500
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := ZBT-MOFI6500
  DEVICE_DTS := mt7981-spim-nand-rfb-mofi6500
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-spim-snand-rfb-mofi6500
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-rfb-mofi6500

define Device/z8107ax
  DEVICE_VENDOR := ZbtLink
  DEVICE_MODEL := ZBT-Z8107AX
  DEVICE_DTS := ZBT-Z8107AX
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := zbtlink,z8107ax
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 131072k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += z8107ax

define Device/z8107ax-emmc
  DEVICE_VENDOR := ZbtLink
  DEVICE_MODEL := ZBT-Z8107AX-eMMC
  DEVICE_DTS := ZBT-Z8107AX-eMMC
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := zbtlink,z8107ax-emmc
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 131072k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += z8107ax-emmc

define Device/z8111ax
  DEVICE_VENDOR := ZbtLink
  DEVICE_MODEL := ZBT-Z8111AX(NAND)
  DEVICE_DTS := ZBT-Z8111AX
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := zbtlink,z8111ax | mediatek,mt7981-spim-snand-rfb-z8111ax
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += z8111ax

define Device/mofi6500
  DEVICE_VENDOR := RAVIN
  DEVICE_MODEL := RAVIN-MOFI6500(NAND)
  DEVICE_DTS := RAVIN-MOFI6500
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := ravin,mofi6500
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mofi6500

define Device/mt7981-spim-nand-gsw
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-spim-nand-gsw
  DEVICE_DTS := mt7981-spim-nand-gsw
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-rfb,ubi
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-gsw

define Device/mt7981-emmc-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-emmc-rfb
  DEVICE_DTS := mt7981-emmc-rfb
  SUPPORTED_DEVICES := mediatek,mt7981-emmc-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := mkf2fs e2fsprogs blkid blockdev losetup kmod-fs-ext4 \
		     kmod-mmc kmod-fs-f2fs kmod-fs-vfat kmod-nls-cp437 \
		     kmod-nls-iso8859-1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-emmc-rfb

define Device/mt7981-emmc-rfb-z8102ax
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := ZBT-Z8102AX-eMMC
  DEVICE_DTS := mt7981-emmc-rfb-z8102ax
  SUPPORTED_DEVICES := mediatek,mt7981-emmc-rfb-z8102ax
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := mkf2fs e2fsprogs blkid blockdev losetup kmod-fs-ext4 \
		     kmod-mmc kmod-fs-f2fs kmod-fs-vfat kmod-nls-cp437 \
		     kmod-nls-iso8859-1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-emmc-rfb-z8102ax

define Device/mt7981-sd-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-sd-rfb
  DEVICE_DTS := mt7981-sd-rfb
  SUPPORTED_DEVICES := mediatek,mt7981-sd-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := mkf2fs e2fsprogs blkid blockdev losetup kmod-fs-ext4 \
		     kmod-mmc kmod-fs-f2fs kmod-fs-vfat kmod-nls-cp437 \
		     kmod-nls-iso8859-1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-sd-rfb

define Device/mt7981-snfi-nand-2500wan-p5
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-snfi-nand-2500wan-p5
  DEVICE_DTS := mt7981-snfi-nand-2500wan-p5
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-snfi-snand-pcie-2500wan-p5-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-snfi-nand-2500wan-p5

define Device/mt7981-fpga-spim-nor
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-fpga-spim-nor
  DEVICE_DTS := mt7981-fpga-spim-nor
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-fpga-nor
endef
TARGET_DEVICES += mt7981-fpga-spim-nor

define Device/mt7981-fpga-snfi-nand
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-fpga-snfi-nand
  DEVICE_DTS := mt7981-fpga-snfi-nand
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-fpga-snfi-snand
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-fpga-snfi-nand

define Device/mt7981-fpga-spim-nand
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-fpga-spim-nand
  DEVICE_DTS := mt7981-fpga-spim-nand
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-fpga-spim-snand
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-fpga-spim-nand

define Device/mt7981-fpga-emmc
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-fpga-emmc
  DEVICE_DTS := mt7981-fpga-emmc
  SUPPORTED_DEVICES := mediatek,mt7981-fpga-emmc
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := mkf2fs e2fsprogs blkid blockdev losetup kmod-fs-ext4 \
		     kmod-mmc kmod-fs-f2fs kmod-fs-vfat kmod-nls-cp437 \
		     kmod-nls-iso8859-1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-fpga-emmc

define Device/mt7981-fpga-sd
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-fpga-sd
  DEVICE_DTS := mt7981-fpga-sd
  SUPPORTED_DEVICES := mediatek,mt7981-fpga-sd
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := mkf2fs e2fsprogs blkid blockdev losetup kmod-fs-ext4 \
		     kmod-mmc kmod-fs-f2fs kmod-fs-vfat kmod-nls-cp437 \
		     kmod-nls-iso8859-1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-fpga-sd
