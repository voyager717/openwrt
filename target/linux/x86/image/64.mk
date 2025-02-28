define Device/generic
<<<<<<< HEAD
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := x86/64
  DEVICE_PACKAGES += \
	kmod-amazon-ena kmod-amd-xgbe kmod-bnx2 kmod-dwmac-intel kmod-e1000e kmod-e1000 \
	kmod-forcedeth kmod-fs-vfat kmod-igb kmod-igc kmod-ixgbe kmod-r8169 \
	kmod-tg3 kmod-drm-i915
=======
  DEVICE_TITLE := Generic x86/64
  DEVICE_PACKAGES += kmod-bnx2 kmod-e1000e kmod-e1000 kmod-forcedeth kmod-igb \
	kmod-ixgbe kmod-r8169
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  GRUB2_VARIANT := generic
endef
TARGET_DEVICES += generic
