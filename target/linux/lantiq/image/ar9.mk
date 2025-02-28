define Device/avm_fritz7312
<<<<<<< HEAD
  $(Device/AVM_preloader)
  DEVICE_MODEL := FRITZ!Box 7312
  SOC := ar9
  IMAGE_SIZE := 15744k
  LOADER_FLASH_OFFS := 0x31000
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-deu-ar9 fritz-tffs -swconfig
=======
  $(Device/AVM)
  DEVICE_MODEL := FRITZ!Box 7312
  SOC := ar9
  IMAGE_SIZE := 15744k
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-basic-wolfssl \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	-swconfig
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef
TARGET_DEVICES += avm_fritz7312

define Device/avm_fritz7320
<<<<<<< HEAD
  $(Device/AVM_preloader)
  DEVICE_MODEL := FRITZ!Box 7320
  DEVICE_ALT0_VENDOR := 1&1
  DEVICE_ALT0_MODEL := HomeServer
  DEVICE_ALT1_VENDOR := AVM
  DEVICE_ALT1_MODEL := Fritz!Box 7330
  SOC := ar9
  IMAGE_SIZE := 15744k
  LOADER_FLASH_OFFS := 0x31000
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-deu-ar9 kmod-usb-dwc2 fritz-tffs -swconfig
=======
  $(Device/AVM)
  DEVICE_MODEL := FRITZ!Box 7320
  DEVICE_ALT0_VENDOR := 1&1
  DEVICE_ALT0_MODEL := HomeServer
  SOC := ar9
  IMAGE_SIZE := 15744k
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-basic-wolfssl \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-usb-dwc2 -swconfig
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  SUPPORTED_DEVICES += FRITZ7320
endef
TARGET_DEVICES += avm_fritz7320

define Device/bt_homehub-v3a
  $(Device/NAND)
<<<<<<< HEAD
  DEVICE_VENDOR := British Telecom (BT)
=======
  DEVICE_VENDOR := British Telecom
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  DEVICE_MODEL := Home Hub 3
  DEVICE_VARIANT := Type A
  BOARD_NAME := BTHOMEHUBV3A
  SOC := ar9
  KERNEL_SIZE := 2048k
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-a kmod-ltq-atm-ar9 \
<<<<<<< HEAD
	kmod-ltq-deu-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
=======
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-basic-wolfssl \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	uboot-envtools
  SUPPORTED_DEVICES += BTHOMEHUBV3A
  DEFAULT := n
endef
TARGET_DEVICES += bt_homehub-v3a

define Device/buffalo_wbmr-hp-g300h-a
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WBMR-HP-G300H
  DEVICE_VARIANT := A
  IMAGE_SIZE := 31488k
  SOC := ar9
  DEVICE_DTS := ar9_buffalo_wbmr-hp-g300h
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-a kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
<<<<<<< HEAD
	kmod-ath9k kmod-owl-loader wpad-basic-mbedtls
=======
	kmod-ath9k kmod-owl-loader wpad-basic-wolfssl
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  SUPPORTED_DEVICES := WBMR buffalo,wbmr-hp-g300h
endef
TARGET_DEVICES += buffalo_wbmr-hp-g300h-a

define Device/buffalo_wbmr-hp-g300h-b
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WBMR-HP-G300H
  DEVICE_VARIANT := B
  IMAGE_SIZE := 31488k
  SOC := ar9
  DEVICE_DTS := ar9_buffalo_wbmr-hp-g300h
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
<<<<<<< HEAD
	kmod-ath9k kmod-owl-loader wpad-basic-mbedtls
=======
	kmod-ath9k kmod-owl-loader wpad-basic-wolfssl
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  SUPPORTED_DEVICES := WBMR buffalo,wbmr-hp-g300h
endef
TARGET_DEVICES += buffalo_wbmr-hp-g300h-b

DGN3500_KERNEL_OFFSET_HEX=0x50000
DGN3500_KERNEL_OFFSET_DEC=327680
define Device/netgear_dgn3500
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := DGN3500
  SOC := ar9
  IMAGE_SIZE := 16000k
<<<<<<< HEAD
  KERNEL := kernel-bin | append-dtb | lzma | loader-kernel | uImage none
  KERNEL_INITRAMFS := $$(KERNEL)
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  IMAGES := \
	sysupgrade-na.bin sysupgrade.bin \
	factory-na.img factory.img
  IMAGE/sysupgrade-na.bin := \
	append-kernel | append-rootfs | dgn3500-sercom-footer 0x0 "NA" | \
<<<<<<< HEAD
	pad-rootfs | check-size | append-metadata
  IMAGE/sysupgrade.bin := \
	append-kernel | append-rootfs | dgn3500-sercom-footer 0x0 "WW" | \
	pad-rootfs | check-size | append-metadata
=======
	pad-rootfs | append-metadata | check-size
  IMAGE/sysupgrade.bin := \
	append-kernel | append-rootfs | dgn3500-sercom-footer 0x0 "WW" | \
	pad-rootfs | append-metadata | check-size
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  IMAGE/factory-na.img := \
	pad-extra $(DGN3500_KERNEL_OFFSET_DEC) | append-kernel | append-rootfs | \
	dgn3500-sercom-footer $(DGN3500_KERNEL_OFFSET_HEX) "NA" | pad-rootfs | \
	check-size 16320k | pad-to 16384k
  IMAGE/factory.img := \
	pad-extra $(DGN3500_KERNEL_OFFSET_DEC) | append-kernel | append-rootfs | \
	dgn3500-sercom-footer $(DGN3500_KERNEL_OFFSET_HEX) "WW" | pad-rootfs | \
	check-size 16320k | pad-to 16384k
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
<<<<<<< HEAD
	kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-a kmod-ltq-atm-ar9 \
	kmod-ltq-deu-ar9 ltq-adsl-app ppp-mod-pppoa
=======
	kmod-ath9k kmod-owl-loader wpad-basic-wolfssl \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-a kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  SUPPORTED_DEVICES += DGN3500
endef
TARGET_DEVICES += netgear_dgn3500

define Device/netgear_dgn3500b
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := DGN3500B
  SOC := ar9
  IMAGE_SIZE := 16000k
<<<<<<< HEAD
  KERNEL := kernel-bin | append-dtb | lzma | loader-kernel | uImage none
  KERNEL_INITRAMFS := $$(KERNEL)
  IMAGES += factory.img
  IMAGE/sysupgrade.bin := \
	append-kernel | append-rootfs | dgn3500-sercom-footer 0x0 "DE" | \
	pad-rootfs | check-size | append-metadata
=======
  IMAGES += factory.img
  IMAGE/sysupgrade.bin := \
	append-kernel | append-rootfs | dgn3500-sercom-footer 0x0 "DE" | \
	pad-rootfs | append-metadata | check-size
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  IMAGE/factory.img := \
	pad-extra $(DGN3500_KERNEL_OFFSET_DEC) | append-kernel | append-rootfs | \
	dgn3500-sercom-footer $(DGN3500_KERNEL_OFFSET_HEX) "DE" | pad-rootfs | \
	check-size 16320k | pad-to 16384k
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
<<<<<<< HEAD
	kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	kmod-ltq-deu-ar9 ltq-adsl-app ppp-mod-pppoa
=======
	kmod-ath9k kmod-owl-loader wpad-basic-wolfssl \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  SUPPORTED_DEVICES += DGN3500B
endef
TARGET_DEVICES += netgear_dgn3500b

define Device/zte_h201l
  DEVICE_VENDOR := ZTE
  DEVICE_MODEL := H201L
  IMAGE_SIZE := 7808k
  SOC := ar9
<<<<<<< HEAD
  DEVICE_PACKAGES := kmod-ath9k-htc wpad-basic-mbedtls \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	kmod-ltq-deu-ar9 ltq-adsl-app ppp-mod-pppoe \
	kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-tapi kmod-ltq-vmmc
  SUPPORTED_DEVICES += H201L
  DEFAULT := n
=======
  DEVICE_PACKAGES := kmod-ath9k-htc wpad-basic-wolfssl \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoe \
	kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-tapi kmod-ltq-vmmc
  SUPPORTED_DEVICES += H201L
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
endef
TARGET_DEVICES += zte_h201l

define Device/zyxel_p-2601hn
<<<<<<< HEAD
  DEVICE_VENDOR := Zyxel
=======
  DEVICE_VENDOR := ZyXEL
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
  DEVICE_MODEL := P-2601HN
  DEVICE_VARIANT := F1/F3
  IMAGE_SIZE := 15616k
  SOC := ar9
<<<<<<< HEAD
  DEVICE_PACKAGES := kmod-rt2800-usb wpad-basic-mbedtls \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	kmod-ltq-deu-ar9 ltq-adsl-app ppp-mod-pppoe \
=======
  DEVICE_PACKAGES := kmod-rt2800-usb wpad-basic-wolfssl \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoe \
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	kmod-usb-dwc2
  SUPPORTED_DEVICES += P2601HNFX
endef
TARGET_DEVICES += zyxel_p-2601hn
