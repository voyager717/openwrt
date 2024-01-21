REQUIRE_IMAGE_METADATA=1
RAMFS_COPY_BIN='fitblk'

platform_get_bootdev() {
	local rootdisk="$(cat /sys/firmware/devicetree/base/chosen/rootdisk)"
	local handle bootdev
	for handle in /sys/class/block/*/of_node/phandle /sys/class/block/*/device/of_node/phandle; do
		[ ! -e "$handle" ] && continue
		if [ "$rootdisk" = "$(cat $handle)" ]; then
			bootdev="${handle%/of_node/phandle}"
			bootdev="${bootdev%/device}"
			bootdev="${bootdev#/sys/class/block/}"
			echo "$bootdev"
			break
		fi
	done
}

platform_do_upgrade() {
	local board=$(board_name)
	local file_type=$(identify $1)

	case "$board" in
	bananapi,bpi-r64)
		[ -e /dev/fit0 ] && fitblk /dev/fit0
		[ -e /dev/fitrw ] && fitblk /dev/fitrw
		bootdev="$(platform_get_bootdev)"
		case "$bootdev" in
		mmcblk*)
			EMMC_KERN_DEV="/dev/$bootdev"
			emmc_do_upgrade "$1"
			;;
		ubiblock*)
			CI_KERNPART="fit"
			nand_do_upgrade "$1"
			;;
		esac
		;;

	buffalo,wsr-2533dhp2|\
	buffalo,wsr-3200ax4s)
		local magic="$(get_magic_long "$1")"

		# use "mtd write" if the magic is "DHP2 (0x44485032)"
		# or "DHP3 (0x44485033)"
		if [ "$magic" = "44485032" -o "$magic" = "44485033" ]; then
			buffalo_upgrade_ubinized "$1"
		else
			CI_KERNPART="firmware"
			nand_do_upgrade "$1"
		fi
		;;
	dlink,eagle-pro-ai-m32-a1|\
	dlink,eagle-pro-ai-r32-a1|\
	elecom,wrc-x3200gst3|\
	mediatek,mt7622-rfb1-ubi|\
	netgear,wax206|\
	totolink,a8000ru|\
	xiaomi,redmi-router-ax6s)
		nand_do_upgrade "$1"
		;;
	linksys,e8450-ubi)
		CI_KERNPART="fit"
		nand_do_upgrade "$1"
		;;
	linksys,e8450)
		if grep -q mtdparts=slave /proc/cmdline; then
			PART_NAME=firmware2
		else
			PART_NAME=firmware1
		fi
		default_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}

PART_NAME=firmware

platform_check_image() {
	local board=$(board_name)
	local magic="$(get_magic_long "$1")"

	[ "$#" -gt 1 ] && return 1

	case "$board" in
	buffalo,wsr-2533dhp2|\
	buffalo,wsr-3200ax4s)
		buffalo_check_image "$board" "$magic" "$1" || return 1
		;;
	dlink,eagle-pro-ai-m32-a1|\
	dlink,eagle-pro-ai-r32-a1|\
	elecom,wrc-x3200gst3|\
	mediatek,mt7622-rfb1-ubi|\
	netgear,wax206|\
	totolink,a8000ru|\
	xiaomi,redmi-router-ax6s)
		nand_do_platform_check "$board" "$1"
		return $?
		;;
	*)
		[ "$magic" != "d00dfeed" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;
	esac

	return 0
}

platform_copy_config() {
	case "$(board_name)" in
	bananapi,bpi-r64)
		if platform_get_bootdev | grep -q mmc; then
			emmc_copy_config
		fi
		;;
	esac
}
