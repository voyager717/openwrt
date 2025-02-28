#################################################
# Subtarget generic
#################################################

  # BCM4705 with tg3
define Device/linksys_wrt300n-v1.1
  DEVICE_MODEL := WRT300N
  DEVICE_VARIANT := v1.1
  DEVICE_PACKAGES := kmod-tg3 kmod-b43
  $(Device/linksys)
  DEVICE_ID := EWC2
  VERSION := 1.51.2
<<<<<<< HEAD
  DEFAULT := n
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef
TARGET_DEVICES += linksys_wrt300n-v1.1

define Device/linksys_wrt310n-v1
  DEVICE_MODEL := WRT310N
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-tg3 kmod-b43
  $(Device/linksys)
  DEVICE_ID := 310N
  VERSION := 1.0.10
<<<<<<< HEAD
  DEFAULT := n
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef
TARGET_DEVICES += linksys_wrt310n-v1

define Device/linksys_wrt350n-v1
  DEVICE_MODEL := WRT350N
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-tg3 kmod-b43 $(USB2_PACKAGES)
  $(Device/linksys)
  DEVICE_ID := EWCG
  VERSION := 1.04.1
<<<<<<< HEAD
  DEFAULT := n
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef
TARGET_DEVICES += linksys_wrt350n-v1

define Device/linksys_wrt610n-v1
  DEVICE_MODEL := WRT610N
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-tg3 kmod-b43 $(USB2_PACKAGES)
  $(Device/linksys)
  DEVICE_ID := 610N
  VERSION := 1.0.1
endef
TARGET_DEVICES += linksys_wrt610n-v1

  # BCMA SoC with SSB WiFi
define Device/linksys_wrt610n-v2
  DEVICE_MODEL := WRT610N
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-bgmac kmod-b43 $(USB2_PACKAGES)
  $(Device/linksys)
  DEVICE_ID := 610N
  VERSION := 2.0.0
endef
TARGET_DEVICES += linksys_wrt610n-v2

define Device/linksys_e3000-v1
  DEVICE_MODEL := E3000
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-bgmac kmod-b43 $(USB2_PACKAGES)
  $(Device/linksys)
  DEVICE_ID := 61XN
  VERSION := 1.0.3
endef
TARGET_DEVICES += linksys_e3000-v1

# generic has Ethernet drivers as modules so overwrite standard image
define Device/standard
<<<<<<< HEAD
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := Image with LZMA loader and LZMA compressed kernel
=======
  DEVICE_TITLE := Image with LZMA loader and LZMA compressed kernel
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  DEVICE_PACKAGES := kmod-b44 kmod-bgmac kmod-tg3
endef
TARGET_DEVICES += standard
