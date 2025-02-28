define Device/mikrotik
	DEVICE_VENDOR := MikroTik
<<<<<<< HEAD
	LOADER_TYPE := elf
	KERNEL_NAME := vmlinuz
	KERNEL := kernel-bin | append-dtb-elf
	KERNEL_INITRAMFS_NAME := vmlinux-initramfs
	KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | loader-kernel
=======
	KERNEL_NAME := vmlinuz
	KERNEL := kernel-bin | append-dtb-elf
	KERNEL_INITRAMFS := kernel-bin | append-dtb-elf
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Device/mikrotik_nor
  $(Device/mikrotik)
<<<<<<< HEAD
  DEVICE_PACKAGES := -yafut
  IMAGE/sysupgrade.bin := append-kernel | yaffs-filesystem -M | \
	pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | \
	check-size | append-metadata
=======
  IMAGE/sysupgrade.bin := append-kernel | kernel2minor -s 1024 -e | \
	pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | \
	append-metadata | check-size
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

define Device/mikrotik_nand
  $(Device/mikrotik)
<<<<<<< HEAD
  IMAGE/sysupgrade.bin = append-kernel | sysupgrade-tar | append-metadata
  DEVICE_COMPAT_MESSAGE := \
       NAND images switched to yafut. If running older image, reinstall from initramfs.
  DEVICE_COMPAT_VERSION := 1.1

=======
  IMAGE/sysupgrade.bin = append-kernel | kernel2minor -s 2048 -e -c | \
	sysupgrade-tar kernel=$$$$@ | append-metadata
  DEVICE_PACKAGES := nand-utils
  DEFAULT := n
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef
