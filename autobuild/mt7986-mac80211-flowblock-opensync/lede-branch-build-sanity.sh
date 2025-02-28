#!/bin/bash
source ./autobuild/lede-build-sanity.sh

#get the brach_name
temp=${0%/*}
branch_name=${temp##*/}

rm -rf ${BUILD_DIR}/package/network/services/hostapd
cp -fpR ${BUILD_DIR}/./../mac80211_package/package/network/services/hostapd ${BUILD_DIR}/package/network/services

rm -rf ${BUILD_DIR}/package/libs/libnl-tiny
cp -fpR ${BUILD_DIR}/./../mac80211_package/package/libs/libnl-tiny ${BUILD_DIR}/package/libs

rm -rf ${BUILD_DIR}/package/network/utils/iw
cp -fpR ${BUILD_DIR}/./../mac80211_package/package/network/utils/iw ${BUILD_DIR}/package/network/utils

rm -rf ${BUILD_DIR}/package/network/utils/iwinfo
cp -fpR ${BUILD_DIR}/./../mac80211_package/package/network/utils/iwinfo ${BUILD_DIR}/package/network/utils

rm -rf ${BUILD_DIR}/package/kernel/mac80211
cp -fpR ${BUILD_DIR}/./../mac80211_package/package/kernel/mac80211 ${BUILD_DIR}/package/kernel

#use hostapd master package revision, remove hostapd 2102 patches
find ../mtk-openwrt-feeds/openwrt_patches-21.02 -name "*-2102-hostapd-*.patch" -delete

#use mt76 lastest commit, remove mt76 master patches
find ../mtk-openwrt-feeds/openwrt_patches-21.02 -name "*-master-mt76-*.patch" -delete

#step1 clean
#clean

#do prepare stuff
prepare

#rm patches for flowblock
rm -rf ./target/linux/generic/pending-5.4/64*.patch
rm -rf ./target/linux/generic/hack-5.4/647-netfilter-flow-acct.patch
rm -rf ./target/linux/generic/hack-5.4/650-netfilter-add-xt_OFFLOAD-target.patch
rm -rf ./target/linux/mediatek/patches-5.4/1002-mtkhnat-add-support-for-virtual-interface-acceleration.patch

#hack mt7986 config5.4
echo "CONFIG_PSTORE=y" >> ./target/linux/mediatek/mt7986/config-5.4
echo "CONFIG_NETFILTER=y" >> ./target/linux/mediatek/mt7986/config-5.4
echo "CONFIG_NETFILTER_INGRESS=y" >> ./target/linux/mediatek/mt7986/config-5.4
echo "CONFIG_NETFILTER_NETLINK=y" >> ./target/linux/mediatek/mt7986/config-5.4
echo "CONFIG_NETFILTER_ADVANCED=y" >> ./target/linux/mediatek/mt7986/config-5.4
echo "CONFIG_RELAY=y" >> ./target/linux/mediatek/mt7986/config-5.4

#hack mt76 firmware/eeprom
#===================firmware bin name format=========================
#define MT7915_FIRMWARE_WA		"mediatek/mt7915_wa.bin"
#define MT7915_FIRMWARE_WM		"mediatek/mt7915_wm.bin"
#define MT7915_ROM_PATCH		"mediatek/mt7915_rom_patch.bin"
#define MT7916_FIRMWARE_WA		"mediatek/mt7916_wa.bin"
#define MT7916_FIRMWARE_WM		"mediatek/mt7916_wm.bin"
#define MT7916_ROM_PATCH		"mediatek/mt7916_rom_patch.bin"
#define MT7986_FIRMWARE_WA		"mediatek/mt7986_wa.bin"
#define MT7986_FIRMWARE_WM		"mediatek/mt7986_wm.bin"
#define MT7986_FIRMWARE_WM_MT7975	"mediatek/mt7986_wm_mt7975.bin"
#define MT7986_ROM_PATCH		"mediatek/mt7986_rom_patch.bin"
#define MT7986_ROM_PATCH_MT7975		"mediatek/mt7986_rom_patch_mt7975.bin"
FW_SOURCE_DIR=${BUILD_DIR}/package/kernel/mt76
cp -rf ${FW_SOURCE_DIR}/firmware/mt7986/rebb/7986_WACPU_RAM_CODE_release.bin ${FW_SOURCE_DIR}/src/firmware/mt7986_wa.bin
cp -rf ${FW_SOURCE_DIR}/firmware/mt7986/rebb/WIFI_RAM_CODE_MT7986.bin ${FW_SOURCE_DIR}/src/firmware/mt7986_wm.bin
cp -rf ${FW_SOURCE_DIR}/firmware/mt7986/rebb/mt7986_patch_e1_hdr.bin ${FW_SOURCE_DIR}/src/firmware/mt7986_rom_patch.bin
cp -rf ${FW_SOURCE_DIR}/firmware/mt7986/rebb/WIFI_RAM_CODE_MT7986_MT7975.bin ${FW_SOURCE_DIR}/src/firmware/mt7986_wm_mt7975.bin
cp -rf ${FW_SOURCE_DIR}/firmware/mt7986/rebb/mt7986_patch_e1_hdr_mt7975.bin ${FW_SOURCE_DIR}/src/firmware/mt7986_rom_patch_mt7975.bin
cp -rf ${FW_SOURCE_DIR}/firmware/mt7986/rebb/7986_WOCPU0_RAM_CODE_release.bin ${FW_SOURCE_DIR}/src/firmware/mt7986_wo_0.bin
cp -rf ${FW_SOURCE_DIR}/firmware/mt7986/rebb/7986_WOCPU1_RAM_CODE_release.bin ${FW_SOURCE_DIR}/src/firmware/mt7986_wo_1.bin
cp -rf ${FW_SOURCE_DIR}/firmware/mt7916/rebb/7916_WACPU_RAM_CODE_release.bin ${FW_SOURCE_DIR}/src/firmware/mt7916_wa.bin
cp -rf ${FW_SOURCE_DIR}/firmware/mt7916/rebb/WIFI_RAM_CODE_MT7916.bin ${FW_SOURCE_DIR}/src/firmware/mt7916_wm.bin
cp -rf ${FW_SOURCE_DIR}/firmware/mt7916/rebb/mt7916_patch_e1_hdr.bin ${FW_SOURCE_DIR}/src/firmware/mt7916_rom_patch.bin

#===================eeprom bin name format=========================
#define MT7915_EEPROM_DEFAULT		"mediatek/mt7915_eeprom.bin"
#define MT7915_EEPROM_DEFAULT_DBDC	"mediatek/mt7915_eeprom_dbdc.bin"
#define MT7916_EEPROM_DEFAULT		"mediatek/mt7916_eeprom.bin"
#define MT7986_EEPROM_MT7975_DEFAULT		"mediatek/mt7986_eeprom_mt7975.bin"
#define MT7986_EEPROM_MT7975_DUAL_DEFAULT	"mediatek/mt7986_eeprom_mt7975_dual.bin"
#define MT7986_EEPROM_MT7976_DEFAULT		"mediatek/mt7986_eeprom_mt7976.bin"
#define MT7986_EEPROM_MT7976_DEFAULT_DBDC	"mediatek/mt7986_eeprom_mt7976_dbdc.bin"
#define MT7986_EEPROM_MT7976_DUAL_DEFAULT	"mediatek/mt7986_eeprom_mt7976_dual.bin"
cp -rf ${FW_SOURCE_DIR}/firmware/mt7916/rebb/MT7916_ePAeLNA_EEPROM.bin ${FW_SOURCE_DIR}/src/firmware/mt7916_eeprom.bin
cp -rf ${FW_SOURCE_DIR}/firmware/mt7986/rebb/MT7986_iPAiLNA_EEPROM_AX7800.bin ${FW_SOURCE_DIR}/src/firmware/mt7986_eeprom_mt7975.bin
cp -rf ${FW_SOURCE_DIR}/firmware/mt7986/rebb/MT7986_iPAiLNA_EEPROM_AX6000.bin ${FW_SOURCE_DIR}/src/firmware/mt7986_eeprom_mt7975_dual.bin
cp -rf ${FW_SOURCE_DIR}/firmware/mt7986/rebb/MT7986_ePAeLNA_EEPROM_AX7800.bin ${FW_SOURCE_DIR}/src/firmware/mt7986_eeprom_mt7976.bin
cp -rf ${FW_SOURCE_DIR}/firmware/mt7986/rebb/MT7986_ePAeLNA_EEPROM_ONEADIE_DBDC.bin ${FW_SOURCE_DIR}/src/firmware/mt7986_eeprom_mt7976_dbdc.bin
cp -rf ${FW_SOURCE_DIR}/firmware/mt7986/rebb/MT7986_ePAeLNA_EEPROM_AX6000.bin ${FW_SOURCE_DIR}/src/firmware/mt7986_eeprom_mt7976_dual.bin
cp -fpR ${BUILD_DIR}/autobuild/mt7986-mac80211-flowblock-opensync/package/ ${BUILD_DIR}/

prepare_final ${branch_name}
patch -f -p1 -i ${BUILD_DIR}/autobuild/mt7622-mac80211-opensync/002_merge_hostapd_patch_from_plume.patch
patch -f -p1 -i ${BUILD_DIR}/autobuild/mt7622-mac80211-opensync/003-makefile-ovs-lib-for-opensync.patch
patch -f -p1 -i ../mtk-openwrt-feeds/autobuild_mac80211_release/0001-master-mac80211-generate-hostapd-setting-from-ap-cap.patch
patch -f -p1 -i ../mtk-openwrt-feeds/autobuild_mac80211_release/0002-master-hostapd-makefile-for-utils.patch
#step2 build
if [ -z ${1} ]; then
	build ${branch_name} -j1 || [ "$LOCAL" != "1" ]
fi
