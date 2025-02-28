# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013-2015 OpenWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

<<<<<<< HEAD
FAT32_BLOCK_SIZE = 1024
FAT32_BLOCKS = $(shell echo $$(($(CONFIG_MXS_SD_BOOT_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))

KERNEL_LOADADDR := 0x40008000

define Build/mxs-ext4-rootfs-with-boot
	rm -rf $(call mkfs_target_dir,$(1))/boot
	mkdir -p $(call mkfs_target_dir,$(1))/boot
	$(CP) --no-preserve=mode $(KDIR)/$(KERNEL_NAME) $(call mkfs_target_dir,$(1))/boot/
	$(foreach dts,$(DEVICE_DTS), \
		$(CP) \
			$(DTS_DIR)/$(dts).dtb \
			$(call mkfs_target_dir,$(1))/boot/; \
	)

	rm -rf $@.rootfs
	$(STAGING_DIR_HOST)/bin/make_ext4fs -L rootfs \
		-l $(ROOTFS_PARTSIZE) -b $(CONFIG_TARGET_EXT4_BLOCKSIZE) \
		$(if $(CONFIG_TARGET_EXT4_RESERVED_PCT),-m $(CONFIG_TARGET_EXT4_RESERVED_PCT)) \
		$(if $(CONFIG_TARGET_EXT4_JOURNAL),,-J) \
		$(if $(SOURCE_DATE_EPOCH),-T $(SOURCE_DATE_EPOCH)) \
		$@.rootfs $(call mkfs_target_dir,$(1))/
	mv $@.rootfs $@
endef

define Build/mxs-sdcard-ext4-ext4
	mv $@ $@.rootfs

	./gen_sdcard_ext4_ext4.sh \
		$@ \
		$(STAGING_DIR_IMAGE)/$(DEVICE_NAME)/u-boot.sb \
		$@.rootfs \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE)
endef

define Build/mxs-sdcard-vfat-ext4
	rm -f $@.boot
	mkfs.fat $@.boot -C $(FAT32_BLOCKS)

	mcopy -i $@.boot $(DTS_DIR)/$(DEVICE_DTS).dtb ::$(DEVICE_DTS).dtb
	mcopy -i $@.boot $(IMAGE_KERNEL) ::uImage

	./gen_sdcard_vfat_ext4.sh \
		$@ \
		$(STAGING_DIR_IMAGE)/$(DEVICE_NAME)/u-boot.sb \
		$@.boot \
		$(IMAGE_ROOTFS) \
		$(CONFIG_MXS_SD_BOOT_PARTSIZE) \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE)
endef

define Device/Default
  PROFILES := Default
  KERNEL_NAME := zImage
  KERNEL := kernel-bin | uImage none
  IMAGES := sdcard.img.gz
  DTS_DIR := $(DTS_DIR)/nxp/mxs
  DEVICE_DTS = $$(SOC)-$(lastword $(subst _, ,$(1)))
endef

define Device/i2se_duckbill
  DEVICE_VENDOR := I2SE
  DEVICE_MODEL := Duckbill
  DEVICE_PACKAGES := -dnsmasq -firewall4 -mtd -nftables -odhcpd-ipv6only -ppp -kmod-nft-offload \
	uboot-envtools kmod-leds-gpio
  SUPPORTED_DEVICES := i2se,duckbill
  SOC := imx28
  KERNEL := kernel-bin
  DEVICE_DTS := imx28-duckbill imx28-duckbill-2 imx28-duckbill-2-485 imx28-duckbill-2-spi
  IMAGE/sdcard.img.gz = mxs-ext4-rootfs-with-boot | mxs-sdcard-ext4-ext4 | append-metadata | gzip
endef
TARGET_DEVICES += i2se_duckbill

define Device/olinuxino_maxi
  DEVICE_VENDOR := Olimex
  DEVICE_MODEL := OLinuXino Maxi
  DEVICE_PACKAGES := kmod-usb-net-smsc95xx kmod-pinctrl-mcp23s08-i2c \
	kmod-pinctrl-mcp23s08-spi kmod-leds-gpio kmod-sound-core
  SUPPORTED_DEVICES := olimex,imx23-olinuxino
  SOC := imx23
  DEVICE_DTS := imx23-olinuxino
  IMAGE/sdcard.img.gz = mxs-sdcard-vfat-ext4 | append-metadata | gzip
endef
TARGET_DEVICES += olinuxino_maxi

define Device/olinuxino_micro
  DEVICE_VENDOR := Olimex
  DEVICE_MODEL := OLinuXino Micro
  DEVICE_PACKAGES := kmod-pinctrl-mcp23s08-spi kmod-pinctrl-mcp23s08-i2c \
	kmod-leds-gpio
  SUPPORTED_DEVICES := olimex,imx23-olinuxino
  SOC := imx23
  DEVICE_DTS := imx23-olinuxino
  IMAGE/sdcard.img.gz = mxs-sdcard-vfat-ext4 | append-metadata | gzip
endef
TARGET_DEVICES += olinuxino_micro
=======
BOARDS:= \
	imx23-olinuxino \
	imx28-duckbill

FAT32_BLOCK_SIZE=1024
FAT32_BLOCKS=$(shell echo $$(($(CONFIG_TARGET_BOOTFS_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))

define Image/BuildKernel
	mkimage -A arm -O linux -T kernel -C none \
		-a 0x40008000 -e 0x40008000 \
		-n 'ARM OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(KDIR)/zImage $(KDIR)/uImage
	cp $(KDIR)/uImage $(BIN_DIR)/$(IMG_PREFIX)-uImage
endef

define Image/InstallKernel
	mkdir -p $(TARGET_DIR)/boot
	cp \
		$(KDIR)/zImage $(KDIR)/uImage \
		$(foreach board,$(BOARDS),$(DTS_DIR)/$(board).dtb) \
		$(TARGET_DIR)/boot/
endef

define Image/Build/SDCard-vfat-ext4
	rm -f $(KDIR)/boot.img
	mkfs.fat $(KDIR)/boot.img -C $(FAT32_BLOCKS)

	mcopy -i $(KDIR)/boot.img $(DTS_DIR)/$(3).dtb ::$(3).dtb
	mcopy -i $(KDIR)/boot.img $(BIN_DIR)/$(IMG_PREFIX)-uImage ::uImage

	./gen_sdcard_vfat_ext4.sh \
		$(BIN_DIR)/$(2) \
		$(STAGING_DIR_IMAGE)/$(4)-u-boot.sb \
		$(KDIR)/boot.img \
		$(KDIR)/root.$(1) \
		$(CONFIG_TARGET_BOOTFS_PARTSIZE) \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE)
	$(call Image/Gzip,$(BIN_DIR)/$(2))
endef

define Image/Build/SDCard-ext4-ext4
	./gen_sdcard_ext4_ext4.sh \
		$(BIN_DIR)/$(2) \
		$(STAGING_DIR_IMAGE)/$(4)-u-boot.sb \
		$(KDIR)/root.$(1) \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE)
	$(call Image/Gzip,$(BIN_DIR)/$(2))
endef

define Image/Build/Profile/olinuxino-maxi
	$(call Image/Build/SDCard-vfat-ext4,$(1),$(2),imx23-olinuxino,mx23_olinuxino)
endef

define Image/Build/Profile/olinuxino-micro
	$(call Image/Build/SDCard-vfat-ext4,$(1),$(2),imx23-olinuxino,mx23_olinuxino)
endef

define Image/Build/Profile/duckbill
	$(call Image/Build/SDCard-ext4-ext4,$(1),$(2),imx28-duckbill,duckbill)
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
	$(call Image/Build/Profile/$(PROFILE),$(1),$(IMG_PREFIX)-$(PROFILE)-sdcard.img)
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) bs=128k conv=sync
	$(call Image/Gzip,$(BIN_DIR)/$(IMG_PREFIX)-root.$(1))
endef
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

$(eval $(call BuildImage))
