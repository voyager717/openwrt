KERNEL_LOADADDR := 0x80008000
DEVICE_VARS += UBOOT_TARGET UBOOT_OFFSET UBOOT_ENVSIZE

ifneq ($(CONFIG_MTK_BOOT_PARTSIZE),)
BOOTFS_BLOCK_SIZE := 1024
BOOTFS_BLOCKS := $(shell echo $$(($(CONFIG_MTK_BOOT_PARTSIZE)*1024*1024/$(BOOTFS_BLOCK_SIZE))))
endif

define Build/mtk-mmc-img
	rm -f $@.boot
	mkfs.fat -C $@.boot $(BOOTFS_BLOCKS)

	if [ -r $(STAGING_DIR_IMAGE)/$(UBOOT_TARGET)-preloader.bin ]; then \
		./gen_mtk_mmc_img.sh emmc $@.emmc \
			$(STAGING_DIR_IMAGE)/$(UBOOT_TARGET)-preloader.bin; \
		mcopy -i $@.boot $@.emmc ::eMMCboot.bin; \
	fi
	mkenvimage -s $(UBOOT_ENVSIZE) -o $(STAGING_DIR_IMAGE)/$(UBOOT_TARGET)-uboot.env $(UBOOT_TARGET)-uEnv.txt
	mcopy -i $@.boot $(STAGING_DIR_IMAGE)/$(UBOOT_TARGET)-uboot.env ::uboot.env
	mcopy -i $@.boot $(IMAGE_KERNEL) ::uImage
	./gen_mtk_mmc_img.sh sd $@ \
		$(STAGING_DIR_IMAGE)/$(UBOOT_TARGET)-preloader.bin \
		$(STAGING_DIR_IMAGE)/$(UBOOT_TARGET)-u-boot*.bin \
		$(UBOOT_OFFSET) \
		$@.boot \
		$(IMAGE_ROOTFS) \
		$(CONFIG_MTK_BOOT_PARTSIZE) \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE)
endef

define Build/preloader
	$(CP) $(STAGING_DIR_IMAGE)/$1-preloader.bin $@
endef

define Build/scatterfile
	./gen_scatterfile.sh $(subst mt,MT,$(SUBTARGET)) "$1" \
		$(subst -scatter.txt,,$(notdir $@)) "$(DEVICE_TITLE)" > $@
endef

define Device/bpi_bananapi-r2
  DEVICE_VENDOR := Bpi
  DEVICE_MODEL := Banana Pi R2
  DEVICE_DTS := mt7623n-bananapi-bpi-r2
  DEVICE_PACKAGES := kmod-fs-vfat kmod-nls-cp437 kmod-nls-iso8859-1 kmod-mmc \
	mkf2fs e2fsprogs kmod-usb-ohci kmod-usb2 kmod-usb3 kmod-ata-ahci-mtk
  UBOOT_ENVSIZE := 0x2000
  UBOOT_OFFSET := 320k
  UBOOT_TARGET := mt7623n_bpir2
  IMAGES := img.gz
  IMAGE/img.gz := mtk-mmc-img | gzip | append-metadata
  ARTIFACT/preloader.bin := preloader $$(UBOOT_TARGET)
  ARTIFACT/scatter.txt := scatterfile $$(firstword $$(FILESYSTEMS))-$$(firstword $$(IMAGES))
  ARTIFACTS = preloader.bin scatter.txt
  SUPPORTED_DEVICES := bananapi,bpi-r2
endef
TARGET_DEVICES += bpi_bananapi-r2

define Device/unielec_u7623-02-emmc-512m
  DEVICE_VENDOR := UniElec
  DEVICE_MODEL := U7623-02
  DEVICE_VARIANT := eMMC/512MB RAM
  DEVICE_DTS := mt7623a-unielec-u7623-02-emmc-512m
  KERNEL_NAME := zImage
  KERNEL := kernel-bin | append-dtb | uImage none
  KERNEL_INITRAMFS := kernel-bin | append-dtb | uImage none
  DEVICE_PACKAGES := kmod-fs-vfat kmod-nls-cp437 kmod-nls-iso8859-1 kmod-mmc \
	mkf2fs e2fsprogs kmod-usb-ohci kmod-usb2 kmod-usb3 kmod-ata-ahci-mtk
  IMAGES := sysupgrade-emmc.bin.gz
  IMAGE/sysupgrade-emmc.bin.gz := sysupgrade-emmc | gzip | append-metadata
endef
TARGET_DEVICES += unielec_u7623-02-emmc-512m
