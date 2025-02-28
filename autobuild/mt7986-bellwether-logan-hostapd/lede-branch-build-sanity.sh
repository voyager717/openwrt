#!/bin/bash
source ./autobuild/lede-build-sanity.sh

#get the brach_name
temp=${0%/*}
branch_name=${temp##*/}

rm -rf ${BUILD_DIR}/package/network/services/hostapd
cp -fpR ${BUILD_DIR}/./../mac80211_package/package/network/services/hostapd ${BUILD_DIR}/package/network/services
cp -fpR ${BUILD_DIR}/autobuild/mt7986-bellwether-logan-hostapd/package/network/services/hostapd ${BUILD_DIR}/package/network/services

rm -rf ${BUILD_DIR}/package/libs/libnl-tiny
cp -fpR ${BUILD_DIR}/./../mac80211_package/package/libs/libnl-tiny ${BUILD_DIR}/package/libs

rm -rf ${BUILD_DIR}/package/network/utils/iw
cp -fpR ${BUILD_DIR}/./../mac80211_package/package/network/utils/iw ${BUILD_DIR}/package/network/utils

rm -rf ${BUILD_DIR}/package/network/utils/iwinfo
cp -fpR ${BUILD_DIR}/./../mac80211_package/package/network/utils/iwinfo ${BUILD_DIR}/package/network/utils

rm -rf ${BUILD_DIR}/package/kernel/mac80211
cp -fpR ${BUILD_DIR}/./../mac80211_package/package/kernel/mac80211 ${BUILD_DIR}/package/kernel

# Change wappd branch from master to neptune-dev-nl80211-wk2203
echo "Change wappd branch from master to neptune-dev-nl80211-wk2203"
rm -rf ../app/wlan_daemon/wappd
git clone "https://gerrit.mediatek.inc/neptune/wlan_daemon/wappd" -b neptune-dev-nl80211-wk2203 ../app/wlan_daemon/wappd

#use hostapd master package revision, remove hostapd 2102 patches
find ../mtk-openwrt-feeds/openwrt_patches-21.02 -name "*-2102-hostapd-*.patch" -delete
find ../mtk-openwrt-feeds/openwrt_patches-21.02 -name "*-master-hostapd-*.patch" -delete

#use mt7986_dev2 branch, remove mt76 master patches
find ../mtk-openwrt-feeds/openwrt_patches-21.02 -name "*-master-mt76-*.patch" -delete
#step1 clean
#clean

#do prepare stuff
prepare

echo "# CONFIG_RTL8723BS is not set" >> ./target/linux/mediatek/mt7986/config-5.4
echo "# CONFIG_WILC1000_SDIO is not set" >> ./target/linux/mediatek/mt7986/config-5.4
echo "# CONFIG_WILC1000_SPI is not set" >> ./target/linux/mediatek/mt7986/config-5.4
echo "# CONFIG_PKCS8_PRIVATE_KEY_PARSER is not set" >> ./target/linux/mediatek/mt7986/config-5.4
echo "# CONFIG_PKCS7_TEST_KEY is not set" >> ./target/linux/mediatek/mt7986/config-5.4
echo "# CONFIG_SYSTEM_EXTRA_CERTIFICATE is not set" >> ./target/linux/mediatek/mt7986/config-5.4
echo "# CONFIG_SECONDARY_TRUSTED_KEYRING is not set" >> ./target/linux/mediatek/mt7986/config-5.4

#install mtk feed target
#./scripts/feeds install mtk

#prepare mtk jedi wifi stuff
prepare_mtwifi ${branch_name}

prepare_final ${branch_name}
#step2 build
if [ -z ${1} ]; then
	build ${branch_name} -j1 || [ "$LOCAL" != "1" ]
fi
