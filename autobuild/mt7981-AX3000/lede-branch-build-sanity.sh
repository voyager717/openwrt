#!/bin/bash
source ./autobuild/lede-build-sanity.sh

#get the brach_name
temp=${0%/*}
branch_name=${temp##*/}
#step1 clean
#clean

#step2.1 choose which .config
if [ $1 = "kasan" ]; then
	echo "CONFIG_KERNEL_KASAN=y" >> autobuild/${branch_name}/.config
	echo "CONFIG_KERNEL_KASAN_OUTLINE=y" >> autobuild/${branch_name}/.config
	echo "# CONFIG_PACKAGE_kmod-ufsd_driver is not set" >> autobuild/${branch_name}/.config
fi

#do prepare stuff
prepare

#step2.2 choose which config-5.4
if [ $1 = "kasan" ]; then
	echo "CONFIG_DEBUG_KMEMLEAK=y" >> ./target/linux/mediatek/mt7981/config-5.4
	echo "CONFIG_DEBUG_KMEMLEAK_AUTO_SCAN=y" >> ./target/linux/mediatek/mt7981/config-5.4
	echo "# CONFIG_DEBUG_KMEMLEAK_DEFAULT_OFF is not set" >> ./target/linux/mediatek/mt7981/config-5.4
	echo "CONFIG_DEBUG_KMEMLEAK_MEM_POOL_SIZE=16000" >> ./target/linux/mediatek/mt7981/config-5.4
	echo "CONFIG_DEBUG_KMEMLEAK_TEST=m" >> ./target/linux/mediatek/mt7981/config-5.4
	echo "CONFIG_KALLSYMS=y" >> ./target/linux/mediatek/mt7981/config-5.4
	echo "CONFIG_KASAN=y" >> ./target/linux/mediatek/mt7981/config-5.4
	echo "CONFIG_KASAN_GENERIC=y" >> ./target/linux/mediatek/mt7981/config-5.4
	echo "# CONFIG_KASAN_INLINE is not set" >> ./target/linux/mediatek/mt7981/config-5.4
	echo "CONFIG_KASAN_OUTLINE=y" >> ./target/linux/mediatek/mt7981/config-5.4
	echo "CONFIG_KASAN_SHADOW_OFFSET=0xdfffffd000000000" >> ./target/linux/mediatek/mt7981/config-5.4
	echo "# CONFIG_TEST_KASAN is not set" >> ./target/linux/mediatek/mt7981/config-5.4
	echo "CONFIG_SLUB_DEBUG=y" >> ./target/linux/mediatek/mt7981/config-5.4
fi

#install mtk feed target
#./scripts/feeds install mtk

#prepare mtk jedi wifi stuff
prepare_mtwifi ${branch_name}

prepare_final ${branch_name}

#workaround-add AIROHA_EN8801S_PHY and LEDS_UBNT_LEDBAR on config-5.4 for not to break build code
Fstring=$(grep "CONFIG_AIROHA_EN8801S_PHY" ./target/linux/mediatek/mt7981/config-5.4 | wc -l)
if [ $Fstring == 0 ]; then
	echo "# CONFIG_AIROHA_EN8801S_PHY is not set" >> ./target/linux/mediatek/mt7981/config-5.4
fi

Fstring2=$(grep "CONFIG_LEDS_UBNT_LEDBAR" ./target/linux/mediatek/mt7981/config-5.4 | wc -l)
if [ $Fstring2 == 0 ]; then
	echo "# CONFIG_LEDS_UBNT_LEDBAR is not set" >> ./target/linux/mediatek/mt7981/config-5.4
fi

# To relieve OOM, drop caches after init done.
sed -i "$ i echo 3 > /proc/sys/vm/drop_caches" ${BUILD_DIR}/package/base-files/files/etc/rc.local

#step3 build
if [ -z ${1} ] || [ $1 = "kasan" ]; then
	build ${branch_name} -j1 || [ "$LOCAL" != "1" ]
fi
