# Config.mk for ZBT-Z8102AX based on MT7981 chipset

# Define general settings for your device
BOARDNAME := MT7981
CPU_TYPE := cortex-a53
TARGET_SUBTARGET := mt7981
TARGET_PROFILE := DEVICE_zbtlink_zbt-z8102ax

# Define supported devices
SUPPORTED_DEVICES := mediatek,mt7981-rfb

# Kernel settings
KERNEL_NAME := Image
KERNEL_LOADADDR := 0x40008000

# Device DTS files
DEVICE_DTS := mt7981-spim-nand-rfb-z8102ax

# Additional settings based on your requirements
UBINIZE_OPTS := -E 5
BLOCKSIZE := 128k
PAGESIZE := 2048
IMAGE_SIZE := 65536k
KERNEL_IN_UBI := 1

# Define device image configuration
IMAGES += factory.bin
IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata

# Include other necessary files or settings as needed
include $(TOPDIR)/rules.mk
