# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2019 OpenWrt.org

<<<<<<< HEAD
define KernelPackage/rp1-adc
  SUBMENU:=$(OTHER_MENU)
  TITLE:=RP1 ADC and temperature sensor driver
  KCONFIG:=CONFIG_SENSORS_RP1_ADC
  FILES:=$(LINUX_DIR)/drivers/hwmon/rp1-adc.ko
  AUTOLOAD:=$(call AutoLoad,21,rp1-adc)
  DEPENDS:=@TARGET_bcm27xx_bcm2712
endef

define KernelPackage/rp1-adc/description
  Kernel module for RP1 silicon providing ADC and
  temperature monitoring.
endef

$(eval $(call KernelPackage,rp1-adc))


=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
define KernelPackage/hwmon-raspberrypi
  TITLE:=Raspberry Pi voltage monitor
  KCONFIG:=CONFIG_SENSORS_RASPBERRYPI_HWMON
  FILES:=$(LINUX_DIR)/drivers/hwmon/raspberrypi-hwmon.ko
  AUTOLOAD:=$(call AutoLoad,60,raspberrypi-hwmon)
  $(call AddDepends/hwmon,@TARGET_bcm27xx)
endef

define KernelPackage/hwmon-raspberrypi/description
  Kernel module for voltage sensor on the Raspberry Pi
endef

$(eval $(call KernelPackage,hwmon-raspberrypi))
<<<<<<< HEAD
=======


define KernelPackage/hwmon-rpi-poe-fan
  SUBMENU:=$(HWMON_MENU)
  TITLE:=Raspberry Pi PoE HAT fan
  DEPENDS:=@TARGET_bcm27xx +kmod-hwmon-core
  KCONFIG:=CONFIG_SENSORS_RPI_POE_FAN
  FILES:=$(LINUX_DIR)/drivers/hwmon/rpi-poe-fan.ko
  AUTOLOAD:=$(call AutoProbe,rpi-poe-fan)
endef

define KernelPackage/hwmon-rpi-poe-fan/description
  Raspberry Pi PoE HAT fan driver
endef

$(eval $(call KernelPackage,hwmon-rpi-poe-fan))
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
