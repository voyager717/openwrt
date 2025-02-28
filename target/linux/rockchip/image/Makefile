# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

DEVICE_VARS += UBOOT_DEVICE_NAME

define Build/Compile
	$(CP) $(LINUX_DIR)/COPYING $(KDIR)/COPYING.linux
endef

### Image scripts ###
define Build/boot-common
	# This creates a new folder copies the dtb (as rockchip.dtb) 
	# and the kernel image (as kernel.img)
	rm -fR $@.boot
	mkdir -p $@.boot

<<<<<<< HEAD
=======
	$(CP) $(DTS_DIR)/$(DEVICE_DTS).dtb $@.boot/rockchip.dtb
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	$(CP) $(IMAGE_KERNEL) $@.boot/kernel.img
endef

define Build/boot-script
	# Make an U-boot image and copy it to the boot partition
<<<<<<< HEAD
	mkimage -A arm -O linux -T script -C none -a 0 -e 0 -d $(if $(1),$(1),default).bootscript $@.boot/boot.scr
=======
	mkimage -A arm -O linux -T script -C none -a 0 -e 0 -d $(if $(1),$(1),mmc).bootscript $@.boot/boot.scr
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Build/pine64-img
	# Creates the final SD/eMMC images, 
	# combining boot partition, root partition as well as the u-boot bootloader

	# Generate a new partition table in $@ with 32 MiB of 
<<<<<<< HEAD
	# alignment padding for the u-boot-rockchip.bin (idbloader + u-boot) to fit:
=======
	# alignment padding for the idbloader and u-boot to fit:
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	# http://opensource.rock-chips.com/wiki_Boot_option#Boot_flow
	#
	# U-Boot SPL expects the U-Boot ITB to be located at sector 0x4000 (8 MiB) on the MMC storage
	PADDING=1 $(SCRIPT_DIR)/gen_image_generic.sh \
		$@ \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $@.boot \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) $(IMAGE_ROOTFS) \
		32768

<<<<<<< HEAD
	# Copy the u-boot-rockchip.bin to the image at sector 0x40
	dd if="$(STAGING_DIR_IMAGE)"/$(UBOOT_DEVICE_NAME)-u-boot-rockchip.bin of="$@" seek=64 conv=notrunc
=======
	# Copy the idbloader and the u-boot image to the image at sector 0x40 and 0x4000
	dd if="$(STAGING_DIR_IMAGE)"/$(UBOOT_DEVICE_NAME)-idbloader.img of="$@" seek=64 conv=notrunc
	dd if="$(STAGING_DIR_IMAGE)"/$(UBOOT_DEVICE_NAME)-u-boot.itb of="$@" seek=16384 conv=notrunc
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

### Devices ###
define Device/Default
  PROFILES := Default
<<<<<<< HEAD
  KERNEL = kernel-bin | lzma | fit lzma $$(DTS_DIR)/$$(DEVICE_DTS).dtb
  BOOT_SCRIPT :=
  IMAGES := sysupgrade.img.gz
  IMAGE/sysupgrade.img.gz = boot-common | boot-script $$(BOOT_SCRIPT) | pine64-img | gzip | append-metadata
  DEVICE_DTS = rockchip/$$(SOC)-$(lastword $(subst _, ,$(1)))
  UBOOT_DEVICE_NAME = $(lastword $(subst _, ,$(1)))-$$(SOC)
=======
  KERNEL := kernel-bin
  IMAGES := sysupgrade.img.gz
  DEVICE_DTS = rockchip/$$(SOC)-$(lastword $(subst _, ,$(1)))
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

include $(SUBTARGET).mk

$(eval $(call BuildImage))
