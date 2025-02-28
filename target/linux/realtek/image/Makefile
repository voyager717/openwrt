# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

<<<<<<< HEAD
KERNEL_LOADADDR = 0x80100000

DEVICE_VARS += \
	BELKIN_HEADER \
	BELKIN_MODEL \
        CAMEO_BOARD_MODEL \
        CAMEO_BOARD_VERSION \
        CAMEO_CUSTOMER_SIGNATURE \
        CAMEO_KERNEL_PART \
        CAMEO_KERNEL_PART_SIZE \
        CAMEO_ROOTFS_PART \
        H3C_DEVICE_ID \
        H3C_PRODUCT_ID \
	LINKSYS_HEADER \
        ZYXEL_VERS

define Build/zyxel-vers
       ( echo VERS;\
       for hw in $(ZYXEL_VERS); do\
=======
KERNEL_LOADADDR = 0x80000000
KERNEL_ENTRY = 0x80000400

DEVICE_VARS += ZYXEL_VERS

define Build/zyxel-vers
       ( echo VERS;\
       for hw in $(1); do\
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
               echo -n "V9.99($$hw.0) | ";\
               date -d @$(SOURCE_DATE_EPOCH) +%m/%d/%Y;\
       done ) >> $@
endef

<<<<<<< HEAD
define Build/cameo-tag
	$(SCRIPT_DIR)/cameo-tag.py $@ $(CAMEO_KERNEL_PART_SIZE)
endef

define Build/cameo-version
	echo -n "OpenWrt" >> $@
	dd if=/dev/zero bs=25 count=1 >> $@
endef

define Build/cameo-headers
        dd if=$@ bs=$(CAMEO_KERNEL_PART_SIZE) count=1 of=$@.kernel_part; \
        dd if=$@ bs=$(CAMEO_KERNEL_PART_SIZE) skip=1 of=$@.rootfs_part; \
        $(SCRIPT_DIR)/cameo-imghdr.py $@.kernel_part $@.kernel_part.hex \
                "$(CAMEO_BOARD_MODEL)" os $(CAMEO_KERNEL_PART) \
                $(CAMEO_CUSTOMER_SIGNATURE) \
                $(CAMEO_BOARD_VERSION) \
                $(KERNEL_LOADADDR); \
        $(SCRIPT_DIR)/cameo-imghdr.py $@.rootfs_part $@.rootfs_part.hex \
                "$(CAMEO_BOARD_MODEL)" squashfs $(CAMEO_ROOTFS_PART) \
                $(CAMEO_CUSTOMER_SIGNATURE) \
                $(CAMEO_BOARD_VERSION); \
        cat $@.kernel_part.hex $@.rootfs_part.hex > $@
endef

define Build/7z
	$(STAGING_DIR_HOST)/bin/7zr a $(@).new -t7z -m0=lzma $(@)
	mv $@.new $@
endef

define Build/h3c-image
	$(STAGING_DIR_HOST)/bin/mkh3cimg \
		-i $(@) \
		-o $(@).new \
		-c 7z \
		-p $(H3C_PRODUCT_ID) \
		-d $(H3C_DEVICE_ID)
	mv $@.new $@
endef

define Build/h3c-vfs
	$(STAGING_DIR_HOST)/bin/mkh3cvfs \
		-i $(@) \
		-o $(@).new \
		-f openwrt-kernel.bin
	mv $@.new $@
endef

define Build/belkin-header
	$(SCRIPT_DIR)/belkin-header.py $(@) $(@).new $(BELKIN_HEADER) ${BELKIN_MODEL}
	mv $@.new $@
endef

define Build/linksys-image
	$(SCRIPT_DIR)/linksys-image.sh $(@) $(@).new $(LINKSYS_MODEL)
	mv $@.new $@
endef

define Device/Default
  PROFILES = Default
  KERNEL := \
	kernel-bin | \
	append-dtb | \
	libdeflate-gzip | \
	uImage gzip
  KERNEL_INITRAMFS := \
	kernel-bin | \
	append-dtb | \
	libdeflate-gzip | \
	uImage gzip
  DEVICE_DTS_DIR := ../dts
  DEVICE_DTS = $$(SOC)_$(1)
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := \
	append-kernel | \
	pad-to 64k | \
	append-rootfs | \
	pad-rootfs | \
	check-size | \
	append-metadata
endef

include $(SUBTARGET).mk
=======
define Device/Default
  PROFILES = Default
  KERNEL := kernel-bin | append-dtb | gzip | uImage gzip
  KERNEL_INITRAMFS := kernel-bin | append-dtb | gzip | uImage gzip
  DEVICE_DTS_DIR := ../dts
  DEVICE_DTS = $$(SOC)_$(1)
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := append-kernel | pad-to 64k | append-rootfs | pad-rootfs | \
	append-metadata | check-size
endef

# "NGE" refers to the uImage magic
define Device/netgear_nge
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | uImage lzma
  SOC := rtl8380
  IMAGE_SIZE := 14848k
  UIMAGE_MAGIC := 0x4e474520
  DEVICE_VENDOR := NETGEAR
endef

define Device/allnet_all-sg8208m
  SOC := rtl8382
  IMAGE_SIZE := 7168k
  DEVICE_VENDOR := ALLNET
  DEVICE_MODEL := ALL-SG8208M
  UIMAGE_MAGIC := 0x00000006
  UIMAGE_NAME := 2.2.2.0
endef
TARGET_DEVICES += allnet_all-sg8208m

define Device/d-link_dgs-1210
  SOC := rtl8382
  IMAGE_SIZE := 13824k
  DEVICE_VENDOR := D-Link
endef

define Device/d-link_dgs-1210-10p
  $(Device/d-link_dgs-1210)
  DEVICE_MODEL := DGS-1210-10P
  DEVICE_PACKAGES += lua-rs232
endef
TARGET_DEVICES += d-link_dgs-1210-10p

define Device/d-link_dgs-1210-16
  $(Device/d-link_dgs-1210)
  DEVICE_MODEL := DGS-1210-16
endef
TARGET_DEVICES += d-link_dgs-1210-16

define Device/d-link_dgs-1210-28
  $(Device/d-link_dgs-1210)
  DEVICE_MODEL := DGS-1210-28
endef
TARGET_DEVICES += d-link_dgs-1210-28

define Device/netgear_gs108t-v3
  $(Device/netgear_nge)
  DEVICE_MODEL := GS108T
  DEVICE_VARIANT := v3
endef
TARGET_DEVICES += netgear_gs108t-v3

define Device/netgear_gs110tpp-v1
  $(Device/netgear_nge)
  DEVICE_MODEL := GS110TPP
  DEVICE_VARIANT := v1
endef
TARGET_DEVICES += netgear_gs110tpp-v1

define Device/zyxel_gs1900
  SOC := rtl8380
  IMAGE_SIZE := 6976k
  DEVICE_VENDOR := ZyXEL
  UIMAGE_MAGIC := 0x83800000
  KERNEL_INITRAMFS := kernel-bin | append-dtb | gzip | zyxel-vers $$$$(ZYXEL_VERS) | \
	uImage gzip
endef

define Device/zyxel_gs1900-10hp
  $(Device/zyxel_gs1900)
  DEVICE_MODEL := GS1900-10HP
  ZYXEL_VERS := AAZI
endef
TARGET_DEVICES += zyxel_gs1900-10hp

define Device/zyxel_gs1900-8
  $(Device/zyxel_gs1900)
  DEVICE_MODEL := GS1900-8
  ZYXEL_VERS := AAHH
endef
TARGET_DEVICES += zyxel_gs1900-8

define Device/zyxel_gs1900-8hp-v1
  $(Device/zyxel_gs1900)
  DEVICE_MODEL := GS1900-8HP
  DEVICE_VARIANT := v1
  ZYXEL_VERS := AAHI
  DEVICE_PACKAGES += lua-rs232
endef
TARGET_DEVICES += zyxel_gs1900-8hp-v1

define Device/zyxel_gs1900-8hp-v2
  $(Device/zyxel_gs1900)
  DEVICE_MODEL := GS1900-8HP
  DEVICE_VARIANT := v2
  ZYXEL_VERS := AAHI
  DEVICE_PACKAGES += lua-rs232
endef
TARGET_DEVICES += zyxel_gs1900-8hp-v2
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

$(eval $(call BuildImage))
