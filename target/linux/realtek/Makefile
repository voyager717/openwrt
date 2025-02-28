# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

ARCH:=mips
<<<<<<< HEAD
=======
CPU_TYPE:=4kec
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
BOARD:=realtek
BOARDNAME:=Realtek MIPS
DEVICE_TYPE:=basic
FEATURES:=ramdisk squashfs
<<<<<<< HEAD
SUBTARGETS:=rtl838x rtl839x rtl930x rtl931x

KERNEL_PATCHVER:=6.6
=======
SUBTARGETS:=generic

KERNEL_PATCHVER:=5.4
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

define Target/Description
	Build firmware images for Realtek RTL83xx based boards.
endef

include $(INCLUDE_DIR)/target.mk

<<<<<<< HEAD
DEFAULT_PACKAGES += \
	ethtool \
	firewall4 \
	kmod-gpio-button-hotplug \
	odhcp6c \
	uboot-envtools \

=======
FEATURES := $(filter-out mips16,$(FEATURES))

DEFAULT_PACKAGES += uboot-envtools ethtool kmod-gpio-button-hotplug \
	dnsmasq firewall ip6tables iptables odhcp6c odhcpd-ipv6only \
	ip-full ip-bridge tc
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

$(eval $(call BuildTarget))
