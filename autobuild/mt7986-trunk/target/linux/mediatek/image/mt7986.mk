ifdef CONFIG_LINUX_5_4
  KERNEL_LOADADDR := 0x48080000
else
  KERNEL_LOADADDR := 0x48000000
endif

define Device/bananapi_bpi-r3
  DEVICE_VENDOR := Bananapi
  DEVICE_MODEL := BPi-R3
  DEVICE_DTS := mt7986a-rfb
  DEVICE_DTS_OVERLAY := mt7986a-bananapi-bpi-r3		\
			mt7986a-bananapi-bpi-r3-sfp
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  KERNEL = kernel-bin | lzma | \
	fit lzma $$(KDIR)/$$(firstword $$(DEVICE_DTS)).dtb
  KERNEL_INITRAMFS = kernel-bin | lzma | \
	fit lzma $$(KDIR)/$$(firstword $$(DEVICE_DTS)).dtb with-initrd
  DEVICE_PACKAGES := mkf2fs e2fsprogs blkid blockdev losetup kmod-fs-ext4 \
		     kmod-mmc kmod-fs-f2fs kmod-fs-vfat kmod-nls-cp437 \
		     kmod-nls-iso8859-1
  DTC_FLAGS += -@ --space 32768
endef
TARGET_DEVICES += bananapi_bpi-r3

define Device/mt7986a-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-rfb
  DEVICE_DTS := mt7986a-rfb
  DEVICE_DTS_OVERLAY := mt7986a-rfb-spim-nand		\
			mt7986a-rfb-snfi-nand		\
			mt7986a-rfb-spim-nor		\
			mt7986a-rfb-emmc
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  KERNEL = kernel-bin | lzma | \
	fit lzma $$(KDIR)/$$(firstword $$(DEVICE_DTS)).dtb
  KERNEL_INITRAMFS = kernel-bin | lzma | \
	fit lzma $$(KDIR)/$$(firstword $$(DEVICE_DTS)).dtb with-initrd
  DTC_FLAGS += -@ --space 32768
endef
TARGET_DEVICES += mt7986a-rfb

define Device/mt7986b-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986b-rfb
  DEVICE_DTS := mt7986b-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986b-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986b-rfb
