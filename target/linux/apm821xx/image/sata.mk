define Build/hdd-img
	./mbl_gen_hdd_img.sh $@ $@.boot $(IMAGE_ROOTFS) $(CONFIG_TARGET_KERNEL_PARTSIZE) $(CONFIG_TARGET_ROOTFS_PARTSIZE)
endef


define Device/wd_mybooklive
  DEVICE_VENDOR := Western Digital
<<<<<<< HEAD
  DEVICE_MODEL := My Book Live
  DEVICE_ALT0_VENDOR := Western Digital
  DEVICE_ALT0_MODEL := My Book Live Duo
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-ata-dwc kmod-usb-ledtrig-usbport \
	kmod-usb-storage kmod-fs-vfat wpad-basic-mbedtls
  SUPPORTED_DEVICES += mbl wd,mybooklive-duo
  BLOCKSIZE := 1k
  DEVICE_DTC_FLAGS := --pad 4096
  KERNEL := kernel-bin | libdeflate-gzip | uImage gzip
  KERNEL_INITRAMFS := kernel-bin | libdeflate-gzip | MuImage-initramfs gzip
=======
  DEVICE_MODEL := My Book Live Series (Single + Duo)
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport kmod-usb-storage kmod-fs-vfat wpad-basic-wolfssl
  SUPPORTED_DEVICES += mbl wd,mybooklive-duo
  BLOCKSIZE := 1k
  DTB_SIZE := 16384
  KERNEL := kernel-bin | dtb | gzip | uImage gzip
  KERNEL_INITRAMFS := kernel-bin | gzip | dtb | MuImage-initramfs gzip
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  IMAGES := factory.img.gz sysupgrade.img.gz
  ARTIFACTS := apollo3g.dtb
  DEVICE_DTB := apollo3g.dtb
  FILESYSTEMS := ext4 squashfs
<<<<<<< HEAD
  IMAGE/factory.img.gz := boot-script | boot-img | hdd-img | libdeflate-gzip
  IMAGE/sysupgrade.img.gz := boot-script | boot-img | hdd-img | libdeflate-gzip | append-metadata
=======
  IMAGE/factory.img.gz := boot-script | boot-img | hdd-img | gzip
  IMAGE/sysupgrade.img.gz := boot-script | boot-img | hdd-img | gzip | append-metadata
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  ARTIFACT/apollo3g.dtb := export-dtb
endef

TARGET_DEVICES += wd_mybooklive
