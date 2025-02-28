#
# Copyright (C) 2014-2016 OpenWrt.org
# Copyright (C) 2016 LEDE-Project.org
#

<<<<<<< HEAD
RAMFS_COPY_BIN='fw_printenv fw_setenv mkfs.f2fs fdisk'
=======
RAMFS_COPY_BIN='fw_printenv fw_setenv'
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	case "$(board_name)" in
	glinet,gl-mv1000|\
	globalscale,espressobin|\
	globalscale,espressobin-emmc|\
	globalscale,espressobin-ultra|\
	globalscale,espressobin-v7|\
	globalscale,espressobin-v7-emmc)
<<<<<<< HEAD
		legacy_sdcard_check_image "$1"
=======
		platform_check_image_sdcard "$1"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
		;;
	*)
		return 0
		;;
	esac
}

platform_do_upgrade() {
	case "$(board_name)" in
	glinet,gl-mv1000|\
	globalscale,espressobin|\
	globalscale,espressobin-emmc|\
	globalscale,espressobin-ultra|\
	globalscale,espressobin-v7|\
	globalscale,espressobin-v7-emmc)
<<<<<<< HEAD
		legacy_sdcard_do_upgrade "$1"
		;;
	methode,udpu|\
	methode,edpu)
=======
		platform_do_upgrade_sdcard "$1"
		;;
	methode,udpu)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
		platform_do_upgrade_uDPU "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
platform_copy_config() {
	case "$(board_name)" in
	glinet,gl-mv1000|\
	globalscale,espressobin|\
	globalscale,espressobin-emmc|\
	globalscale,espressobin-ultra|\
	globalscale,espressobin-v7|\
	globalscale,espressobin-v7-emmc)
<<<<<<< HEAD
		legacy_sdcard_copy_config
		;;
	methode,udpu|\
	methode,edpu)
=======
		platform_copy_config_sdcard
		;;
	methode,udpu)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
		platform_copy_config_uDPU
		;;
	esac
}
