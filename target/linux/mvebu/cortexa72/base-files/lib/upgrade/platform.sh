#
# Copyright (C) 2014-2016 OpenWrt.org
# Copyright (C) 2016 LEDE-Project.org
#

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	case "$(board_name)" in
<<<<<<< HEAD
	globalscale,mochabin|\
	iei,puzzle-m901|\
	iei,puzzle-m902|\
	marvell,armada8040-mcbin-doubleshot|\
	marvell,armada8040-mcbin-singleshot|\
	marvell,armada8040-clearfog-gt-8k|\
	solidrun,clearfog-pro)
		legacy_sdcard_check_image "$1"
=======
	iei,puzzle-m901|\
	iei,puzzle-m902|\
	marvell,armada8040-mcbin-doubleshot|\
	marvell,armada8040-mcbin-singleshot)
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
	iei,puzzle-m901|\
	iei,puzzle-m902)
		platform_do_upgrade_emmc "$1"
		;;
<<<<<<< HEAD
	globalscale,mochabin|\
	marvell,armada8040-mcbin-doubleshot|\
	marvell,armada8040-mcbin-singleshot|\
	marvell,armada8040-clearfog-gt-8k|\
	solidrun,clearfog-pro)
		legacy_sdcard_do_upgrade "$1"
		;;
	mikrotik,rb5009)
		nand_do_upgrade "$1"
=======
	marvell,armada8040-mcbin-doubleshot|\
	marvell,armada8040-mcbin-singleshot)
		platform_do_upgrade_sdcard "$1"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
platform_copy_config() {
	case "$(board_name)" in
<<<<<<< HEAD
	globalscale,mochabin|\
	iei,puzzle-m901|\
	iei,puzzle-m902|\
	marvell,armada8040-mcbin-doubleshot|\
	marvell,armada8040-mcbin-singleshot|\
	marvell,armada8040-clearfog-gt-8k|\
	solidrun,clearfog-pro)
		legacy_sdcard_copy_config
=======
	iei,puzzle-m901|\
	iei,puzzle-m902|\
	marvell,armada8040-mcbin-doubleshot|\
	marvell,armada8040-mcbin-singleshot)
		platform_copy_config_sdcard
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
		;;
	esac
}
