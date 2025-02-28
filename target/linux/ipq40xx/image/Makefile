include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Device/Default
	PROFILES := Default
	KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
<<<<<<< HEAD
	KERNEL_LOADADDR := 0x80208000
	DEVICE_DTS_DIR = $$(DTS_DIR)/qcom
	DEVICE_DTS = $$(SOC)-$(lastword $(subst _, ,$(1)))
	DEVICE_DTS_CONFIG := config@1
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin = sysupgrade-tar | append-metadata
	IMAGE/sysupgrade.bin/squashfs :=
	DEVICE_COMPAT_VERSION := 1.1
	DEVICE_COMPAT_MESSAGE := Config cannot be migrated from swconfig to DSA
=======
	KERNEL_INITRAMFS_PREFIX := $$(IMG_PREFIX)-$(1)-initramfs
	KERNEL_PREFIX := $$(IMAGE_PREFIX)
	KERNEL_LOADADDR := 0x80208000
	DEVICE_DTS = $$(SOC)-$(lastword $(subst _, ,$(1)))
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin = sysupgrade-tar | append-metadata
	IMAGE/sysupgrade.bin/squashfs :=
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef

include $(SUBTARGET).mk

$(eval $(call BuildImage))
