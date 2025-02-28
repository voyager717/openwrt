# Copyright (C) 2014 OpenWrt.org
#

. /lib/functions.sh

<<<<<<< HEAD
# 'kernel' partition or UBI volume on NAND contains the kernel
CI_KERNPART="${CI_KERNPART:-kernel}"

# 'ubi' partition on NAND contains UBI
# There are also CI_KERN_UBIPART and CI_ROOT_UBIPART if kernel
# and rootfs are on separated UBIs.
CI_UBIPART="${CI_UBIPART:-ubi}"

# 'rootfs' UBI volume on NAND contains the rootfs
=======
# 'kernel' partition on NAND contains the kernel
CI_KERNPART="${CI_KERNPART:-kernel}"

# 'ubi' partition on NAND contains UBI
CI_UBIPART="${CI_UBIPART:-ubi}"

# 'rootfs' partition on NAND contains the rootfs
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
CI_ROOTPART="${CI_ROOTPART:-rootfs}"

ubi_mknod() {
	local dir="$1"
	local dev="/dev/$(basename $dir)"

	[ -e "$dev" ] && return 0

	local devid="$(cat $dir/dev)"
	local major="${devid%%:*}"
	local minor="${devid##*:}"
	mknod "$dev" c $major $minor
}

nand_find_volume() {
	local ubidevdir ubivoldir
<<<<<<< HEAD
	ubidevdir="/sys/class/ubi/"
=======
	ubidevdir="/sys/devices/virtual/ubi/$1"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	[ ! -d "$ubidevdir" ] && return 1
	for ubivoldir in $ubidevdir/${1}_*; do
		[ ! -d "$ubivoldir" ] && continue
		if [ "$( cat $ubivoldir/name )" = "$2" ]; then
			basename $ubivoldir
			ubi_mknod "$ubivoldir"
			return 0
		fi
	done
}

nand_find_ubi() {
<<<<<<< HEAD
	local ubidevdir ubidev mtdnum cmtdnum
	mtdnum="$( find_mtd_index $1 )"
	[ ! "$mtdnum" ] && return 1
	for ubidevdir in /sys/class/ubi/ubi*; do
		[ ! -e "$ubidevdir/mtd_num" ] && continue
		cmtdnum="$( cat $ubidevdir/mtd_num )"
=======
	local ubidevdir ubidev mtdnum
	mtdnum="$( find_mtd_index $1 )"
	[ ! "$mtdnum" ] && return 1
	for ubidevdir in /sys/devices/virtual/ubi/ubi*; do
		[ ! -d "$ubidevdir" ] && continue
		cmtdnum="$( cat $ubidevdir/mtd_num )"
		[ ! "$mtdnum" ] && continue
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
		if [ "$mtdnum" = "$cmtdnum" ]; then
			ubidev=$( basename $ubidevdir )
			ubi_mknod "$ubidevdir"
			echo $ubidev
			return 0
		fi
	done
}

nand_get_magic_long() {
<<<<<<< HEAD
	($2 < "$1" | dd bs=4 "skip=${3:-0}" count=1 | hexdump -v -n 4 -e '1/1 "%02x"') 2> /dev/null
}

get_magic_long_tar() {
	($2 < "$1" | tar xOf - "$3" | dd bs=4 count=1 | hexdump -v -n 4 -e '1/1 "%02x"') 2> /dev/null
}

identify() {
	identify_magic_long $(nand_get_magic_long "$@")
}

identify_tar() {
	identify_magic_long $(get_magic_long_tar "$@")
}

identify_if_gzip() {
	if [ "$(identify "$1" "cat")" = gzip ]; then echo -n z; fi
}

nand_restore_config() {
	local ubidev=$( nand_find_ubi "${CI_ROOT_UBIPART:-$CI_UBIPART}" )
	local ubivol="$( nand_find_volume $ubidev rootfs_data )"
	if [ ! "$ubivol" ]; then
		ubivol="$( nand_find_volume $ubidev "$CI_ROOTPART" )"
		if [ ! "$ubivol" ]; then
			echo "cannot find ubifs data volume"
			return 1
		fi
	fi
	mkdir /tmp/new_root
	if ! mount -t ubifs /dev/$ubivol /tmp/new_root; then
		echo "cannot mount ubifs volume $ubivol"
		rmdir /tmp/new_root
		return 1
	fi
	if mv "$1" "/tmp/new_root/$BACKUP_FILE"; then
		if umount /tmp/new_root; then
			echo "configuration saved"
			rmdir /tmp/new_root
			return 0
		fi
	else
		umount /tmp/new_root
	fi
	echo "could not save configuration to ubifs volume $ubivol"
	rmdir /tmp/new_root
	return 1
}

nand_remove_ubiblock() {
	local ubivol="$1"

	local ubiblk="ubiblock${ubivol:3}"
	if [ -e "/dev/$ubiblk" ]; then
		umount "/dev/$ubiblk" 2>/dev/null && echo "unmounted /dev/$ubiblk" || :
		if ! ubiblock -r "/dev/$ubivol"; then
			echo "cannot remove $ubiblk"
			return 1
		fi
	fi
}

nand_attach_ubi() {
	local ubipart="$1"
	local has_env="${2:-0}"

	local mtdnum="$( find_mtd_index "$ubipart" )"
	if [ ! "$mtdnum" ]; then
		>&2 echo "cannot find ubi mtd partition $ubipart"
		return 1
	fi

	local ubidev="$( nand_find_ubi "$ubipart" )"
	if [ ! "$ubidev" ]; then
		>&2 ubiattach -m "$mtdnum"
		ubidev="$( nand_find_ubi "$ubipart" )"

		if [ ! "$ubidev" ]; then
			>&2 ubiformat /dev/mtd$mtdnum -y
			>&2 ubiattach -m "$mtdnum"
			ubidev="$( nand_find_ubi "$ubipart" )"

			if [ ! "$ubidev" ]; then
				>&2 echo "cannot attach ubi mtd partition $ubipart"
				return 1
			fi

			if [ "$has_env" -gt 0 ]; then
				>&2 ubimkvol /dev/$ubidev -n 0 -N ubootenv -s 1MiB
				>&2 ubimkvol /dev/$ubidev -n 1 -N ubootenv2 -s 1MiB
			fi
		fi
	fi

	echo "$ubidev"
	return 0
}

nand_detach_ubi() {
	local ubipart="$1"

	local mtdnum="$( find_mtd_index "$ubipart" )"
	if [ ! "$mtdnum" ]; then
		echo "cannot find ubi mtd partition $ubipart"
		return 1
	fi

	local ubidev="$( nand_find_ubi "$ubipart" )"
	if [ "$ubidev" ]; then
		for ubivol in $(find /dev -name "${ubidev}_*" -maxdepth 1 | sort); do
			ubivol="${ubivol:5}"
			nand_remove_ubiblock "$ubivol" || :
			umount "/dev/$ubivol" && echo "unmounted /dev/$ubivol" || :
		done
		if ! ubidetach -m "$mtdnum"; then
			echo "cannot detach ubi mtd partition $ubipart"
			return 1
		fi
	fi
=======
	dd if="$1" skip=$2 bs=4 count=1 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

get_magic_long_tar() {
	( tar xf $1 $2 -O | dd bs=4 count=1 | hexdump -v -n 4 -e '1/1 "%02x"') 2> /dev/null
}

identify_magic() {
	local magic=$1
	case "$magic" in
		"55424923")
			echo "ubi"
			;;
		"31181006")
			echo "ubifs"
			;;
		"68737173")
			echo "squashfs"
			;;
		"d00dfeed")
			echo "fit"
			;;
		"4349"*)
			echo "combined"
			;;
		*)
			echo "unknown $magic"
			;;
	esac
}


identify() {
	identify_magic $(nand_get_magic_long "$1" "${2:-0}")
}

identify_tar() {
	identify_magic $(get_magic_long_tar "$1" "$2")
}

nand_restore_config() {
	sync
	local ubidev=$( nand_find_ubi $CI_UBIPART )
	local ubivol="$( nand_find_volume $ubidev rootfs_data )"
	[ ! "$ubivol" ] &&
		ubivol="$( nand_find_volume $ubidev $CI_ROOTPART )"
	mkdir /tmp/new_root
	if ! mount -t ubifs /dev/$ubivol /tmp/new_root; then
		echo "mounting ubifs $ubivol failed"
		rmdir /tmp/new_root
		return 1
	fi
	mv "$1" "/tmp/new_root/$BACKUP_FILE"
	umount /tmp/new_root
	sync
	rmdir /tmp/new_root
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
}

nand_upgrade_prepare_ubi() {
	local rootfs_length="$1"
	local rootfs_type="$2"
<<<<<<< HEAD
	local rootfs_data_max="$(fw_printenv -n rootfs_data_max 2> /dev/null)"
	[ -n "$rootfs_data_max" ] && rootfs_data_max=$((rootfs_data_max))

	local kernel_length="$3"
	local has_env="${4:-0}"
	local kern_ubidev
	local root_ubidev

	[ -n "$rootfs_length" -o -n "$kernel_length" ] || return 1

	if [ -n "$CI_KERN_UBIPART" -a -n "$CI_ROOT_UBIPART" ]; then
		kern_ubidev="$( nand_attach_ubi "$CI_KERN_UBIPART" "$has_env" )"
		[ -n "$kern_ubidev" ] || return 1
		root_ubidev="$( nand_attach_ubi "$CI_ROOT_UBIPART" )"
		[ -n "$root_ubidev" ] || return 1
	else
		kern_ubidev="$( nand_attach_ubi "$CI_UBIPART" "$has_env" )"
		[ -n "$kern_ubidev" ] || return 1
		root_ubidev="$kern_ubidev"
	fi

	local kern_ubivol="$( nand_find_volume $kern_ubidev "$CI_KERNPART" )"
	local root_ubivol="$( nand_find_volume $root_ubidev "$CI_ROOTPART" )"
	local data_ubivol="$( nand_find_volume $root_ubidev rootfs_data )"
	[ "$root_ubivol" = "$kern_ubivol" ] && root_ubivol=

	# remove ubiblocks
	[ "$kern_ubivol" ] && { nand_remove_ubiblock $kern_ubivol || return 1; }
	[ "$root_ubivol" ] && { nand_remove_ubiblock $root_ubivol || return 1; }
	[ "$data_ubivol" ] && { nand_remove_ubiblock $data_ubivol || return 1; }

	# kill volumes
	[ "$kern_ubivol" ] && ubirmvol /dev/$kern_ubidev -N "$CI_KERNPART" || :
	[ "$root_ubivol" ] && ubirmvol /dev/$root_ubidev -N "$CI_ROOTPART" || :
	[ "$data_ubivol" ] && ubirmvol /dev/$root_ubidev -N rootfs_data || :

	# create kernel vol
	if [ -n "$kernel_length" ]; then
		if ! ubimkvol /dev/$kern_ubidev -N "$CI_KERNPART" -s $kernel_length; then
=======
	local has_kernel="${3:-0}"
	local has_env="${4:-0}"

	local mtdnum="$( find_mtd_index "$CI_UBIPART" )"
	if [ ! "$mtdnum" ]; then
		echo "cannot find ubi mtd partition $CI_UBIPART"
		return 1
	fi

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	if [ ! "$ubidev" ]; then
		ubiattach -m "$mtdnum"
		sync
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	fi

	if [ ! "$ubidev" ]; then
		ubiformat /dev/mtd$mtdnum -y
		ubiattach -m "$mtdnum"
		sync
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
		[ "$has_env" -gt 0 ] && {
			ubimkvol /dev/$ubidev -n 0 -N ubootenv -s 1MiB
			ubimkvol /dev/$ubidev -n 1 -N ubootenv2 -s 1MiB
		}
	fi

	local kern_ubivol="$( nand_find_volume $ubidev $CI_KERNPART )"
	local root_ubivol="$( nand_find_volume $ubidev $CI_ROOTPART )"
	local data_ubivol="$( nand_find_volume $ubidev rootfs_data )"

	# remove ubiblock device of rootfs
	local root_ubiblk="ubiblock${root_ubivol:3}"
	if [ "$root_ubivol" -a -e "/dev/$root_ubiblk" ]; then
		echo "removing $root_ubiblk"
		if ! ubiblock -r /dev/$root_ubivol; then
			echo "cannot remove $root_ubiblk"
			return 1;
		fi
	fi

	# kill volumes
	[ "$kern_ubivol" ] && ubirmvol /dev/$ubidev -N $CI_KERNPART || true
	[ "$root_ubivol" ] && ubirmvol /dev/$ubidev -N $CI_ROOTPART || true
	[ "$data_ubivol" ] && ubirmvol /dev/$ubidev -N rootfs_data || true

	# update kernel
	if [ "$has_kernel" = "1" ]; then
		if ! ubimkvol /dev/$ubidev -N $CI_KERNPART -s $kernel_length; then
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
			echo "cannot create kernel volume"
			return 1;
		fi
	fi

<<<<<<< HEAD
	# create rootfs vol
	if [ -n "$rootfs_length" ]; then
		local rootfs_size_param
		if [ "$rootfs_type" = "ubifs" ]; then
			rootfs_size_param="-m"
		else
			rootfs_size_param="-s $rootfs_length"
		fi
		if ! ubimkvol /dev/$root_ubidev -N "$CI_ROOTPART" $rootfs_size_param; then
			echo "cannot create rootfs volume"
			return 1;
		fi
	fi

	# create rootfs_data vol for non-ubifs rootfs
	if [ "$rootfs_type" != "ubifs" ]; then
		local rootfs_data_size_param="-m"
		if [ -n "$rootfs_data_max" ]; then
			rootfs_data_size_param="-s $rootfs_data_max"
		fi
		if ! ubimkvol /dev/$root_ubidev -N rootfs_data $rootfs_data_size_param; then
			if ! ubimkvol /dev/$root_ubidev -N rootfs_data -m; then
				echo "cannot initialize rootfs_data volume"
				return 1
			fi
		fi
	fi

	return 0
}

# Write the UBI image to MTD ubi partition
nand_upgrade_ubinized() {
	local ubi_file="$1"
	local cmd="$2"

	local ubi_length=$( ($cmd < "$ubi_file" | wc -c) 2> /dev/null)

	nand_detach_ubi "$CI_UBIPART" || return 1

	local mtdnum="$( find_mtd_index "$CI_UBIPART" )"
	$cmd < "$ubi_file" | ubiformat "/dev/mtd$mtdnum" -S "$ubi_length" -y -f - && ubiattach -m "$mtdnum"
}

# Write the UBIFS image to UBI rootfs volume
nand_upgrade_ubifs() {
	local ubifs_file="$1"
	local cmd="$2"

	local ubifs_length=$( ($cmd < "$ubifs_file" | wc -c) 2> /dev/null)

	nand_upgrade_prepare_ubi "$ubifs_length" "ubifs" "" "" || return 1

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	local root_ubivol="$(nand_find_volume $ubidev "$CI_ROOTPART")"
	$cmd < "$ubifs_file" | ubiupdatevol /dev/$root_ubivol -s "$ubifs_length" -
}

# Write the FIT image to UBI kernel volume
nand_upgrade_fit() {
	local fit_file="$1"
	local cmd="$2"

	local fit_length=$( ($cmd < "$fit_file" | wc -c) 2> /dev/null)

	nand_upgrade_prepare_ubi "" "" "$fit_length" "1" || return 1

	local fit_ubidev="$(nand_find_ubi "$CI_UBIPART")"
	local fit_ubivol="$(nand_find_volume $fit_ubidev "$CI_KERNPART")"
	$cmd < "$fit_file" | ubiupdatevol /dev/$fit_ubivol -s "$fit_length" -
}

# Write images in the TAR file to MTD partitions and/or UBI volumes as required
nand_upgrade_tar() {
	local tar_file="$1"
	local cmd="${2:-cat}"
	local jffs2_markers="${CI_JFFS2_CLEAN_MARKERS:-0}"

	# WARNING: This fails if tar contains more than one 'sysupgrade-*' directory.
	local board_dir="$($cmd < "$tar_file" | tar tf - | grep -m 1 '^sysupgrade-.*/$')"
	board_dir="${board_dir%/}"

	local kernel_mtd kernel_length
	if [ "$CI_KERNPART" != "none" ]; then
		kernel_mtd="$(find_mtd_index "$CI_KERNPART")"
		kernel_length=$( ($cmd < "$tar_file" | tar xOf - "$board_dir/kernel" | wc -c) 2> /dev/null)
		[ "$kernel_length" = 0 ] && kernel_length=
	fi
	local rootfs_length=$( ($cmd < "$tar_file" | tar xOf - "$board_dir/root" | wc -c) 2> /dev/null)
	[ "$rootfs_length" = 0 ] && rootfs_length=
	local rootfs_type
	[ "$rootfs_length" ] && rootfs_type="$(identify_tar "$tar_file" "$cmd" "$board_dir/root")"

	local ubi_kernel_length
	if [ "$kernel_length" ]; then
		if [ "$kernel_mtd" ]; then
			# On some devices, the raw kernel and ubi partitions overlap.
			# These devices brick if the kernel partition is erased.
			# Hence only invalidate kernel for now.
			dd if=/dev/zero bs=4096 count=1 2> /dev/null | \
				mtd write - "$CI_KERNPART"
		else
			ubi_kernel_length="$kernel_length"
		fi
	fi

	local has_env=0
	nand_upgrade_prepare_ubi "$rootfs_length" "$rootfs_type" "$ubi_kernel_length" "$has_env" || return 1

	if [ "$rootfs_length" ]; then
		local ubidev="$( nand_find_ubi "${CI_ROOT_UBIPART:-$CI_UBIPART}" )"
		local root_ubivol="$( nand_find_volume $ubidev "$CI_ROOTPART" )"
		$cmd < "$tar_file" | tar xOf - "$board_dir/root" | \
			ubiupdatevol /dev/$root_ubivol -s "$rootfs_length" -
	fi
	if [ "$kernel_length" ]; then
		if [ "$kernel_mtd" ]; then
			if [ "$jffs2_markers" = 1 ]; then
				flash_erase -j "/dev/mtd${kernel_mtd}" 0 0
				$cmd < "$tar_file" | tar xOf - "$board_dir/kernel" | \
					nandwrite "/dev/mtd${kernel_mtd}" -
			else
				$cmd < "$tar_file" | tar xOf - "$board_dir/kernel" | \
					mtd write - "$CI_KERNPART"
			fi
		else
			local ubidev="$( nand_find_ubi "${CI_KERN_UBIPART:-$CI_UBIPART}" )"
			local kern_ubivol="$( nand_find_volume $ubidev "$CI_KERNPART" )"
			$cmd < "$tar_file" | tar xOf - "$board_dir/kernel" | \
				ubiupdatevol /dev/$kern_ubivol -s "$kernel_length" -
		fi
	fi

	return 0
}

nand_verify_if_gzip_file() {
	local file="$1"
	local cmd="$2"

	if [ "$cmd" = zcat ]; then
		echo "verifying compressed sysupgrade file integrity"
		if ! gzip -t "$file"; then
			echo "corrupted compressed sysupgrade file"
			return 1
		fi
	fi
}

nand_verify_tar_file() {
	local file="$1"
	local cmd="$2"

	echo "verifying sysupgrade tar file integrity"
	if ! $cmd < "$file" | tar xOf - > /dev/null; then
		echo "corrupted sysupgrade tar file"
		return 1
	fi
}

nand_do_flash_file() {
	local file="$1"
	local cmd="$2"
	local file_type

	[ -z "$cmd" ] && cmd="$(identify_if_gzip "$file")cat"
	file_type="$(identify "$file" "$cmd" "")"

	[ ! "$(find_mtd_index "$CI_UBIPART")" ] && CI_UBIPART=rootfs

	case "$file_type" in
		"fit")
			nand_verify_if_gzip_file "$file" "$cmd" || return 1
			nand_upgrade_fit "$file" "$cmd"
			;;
		"ubi")
			nand_verify_if_gzip_file "$file" "$cmd" || return 1
			nand_upgrade_ubinized "$file" "$cmd"
			;;
		"ubifs")
			nand_verify_if_gzip_file "$file" "$cmd" || return 1
			nand_upgrade_ubifs "$file" "$cmd"
			;;
		*)
			nand_verify_tar_file "$file" "$cmd" || return 1
			nand_upgrade_tar "$file" "$cmd"
			;;
	esac
}

nand_do_restore_config() {
	local conf_tar="/tmp/sysupgrade.tgz"
	[ ! -f "$conf_tar" ] || nand_restore_config "$conf_tar"
}

# Recognize type of passed file and start the upgrade process
#
# Supported firmware containers:
# 1. Raw file
# 2. Gzip
# 3. Custom (requires passing extracting command)
#
# Supported data formats:
# 1. Tar with kernel/rootfs
# 2. UBI image (built using "ubinized")
# 3. UBIFS image (to update UBI volume with)
# 4. FIT image (to update UBI volume with)
#
# $(1): firmware file path
# $(2): (optional) pipe command to extract firmware
nand_do_upgrade() {
	local file="$1"
	local cmd="$2"

	sync
	nand_do_flash_file "$file" "$cmd" && nand_do_upgrade_success
	nand_do_upgrade_failed
}

nand_do_upgrade_success() {
	if nand_do_restore_config && sync; then
		echo "sysupgrade successful"
		umount -a
		reboot -f
	fi
	nand_do_upgrade_failed
}

nand_do_upgrade_failed() {
	sync
	echo "sysupgrade failed"
	# Should we reboot or bring up some failsafe mode instead?
=======
	# update rootfs
	local root_size_param
	if [ "$rootfs_type" = "ubifs" ]; then
		root_size_param="-m"
	else
		root_size_param="-s $rootfs_length"
	fi
	if ! ubimkvol /dev/$ubidev -N $CI_ROOTPART $root_size_param; then
		echo "cannot create rootfs volume"
		return 1;
	fi

	# create rootfs_data for non-ubifs rootfs
	if [ "$rootfs_type" != "ubifs" ]; then
		if ! ubimkvol /dev/$ubidev -N rootfs_data -m; then
			echo "cannot initialize rootfs_data volume"
			return 1
		fi
	fi
	sync
	return 0
}

nand_do_upgrade_success() {
	local conf_tar="/tmp/sysupgrade.tgz"

	sync
	[ -f "$conf_tar" ] && nand_restore_config "$conf_tar"
	echo "sysupgrade successful"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	umount -a
	reboot -f
}

<<<<<<< HEAD
# Check if passed file is a valid one for NAND sysupgrade.
# Currently it accepts 4 types of files:
# 1) UBI: a ubinized image containing required UBI volumes.
# 2) UBIFS: a UBIFS rootfs volume image.
# 3) FIT: a FIT image containing kernel and rootfs.
# 4) TAR: an archive that includes directory "sysupgrade-${BOARD_NAME}" containing
#         a non-empty "CONTROL" file and required partition and/or volume images.
=======
# Flash the UBI image to MTD partition
nand_upgrade_ubinized() {
	local ubi_file="$1"
	local mtdnum="$(find_mtd_index "$CI_UBIPART")"

	[ ! "$mtdnum" ] && {
		CI_UBIPART="rootfs"
		mtdnum="$(find_mtd_index "$CI_UBIPART")"
	}

	if [ ! "$mtdnum" ]; then
		echo "cannot find mtd device $CI_UBIPART"
		umount -a
		reboot -f
	fi

	local mtddev="/dev/mtd${mtdnum}"
	ubidetach -p "${mtddev}" || true
	sync
	ubiformat "${mtddev}" -y -f "${ubi_file}"
	ubiattach -p "${mtddev}"
	nand_do_upgrade_success
}

# Write the UBIFS image to UBI volume
nand_upgrade_ubifs() {
	local rootfs_length=$( (cat $1 | wc -c) 2> /dev/null)

	nand_upgrade_prepare_ubi "$rootfs_length" "ubifs" "0" "0"

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	local root_ubivol="$(nand_find_volume $ubidev $CI_ROOTPART)"
	ubiupdatevol /dev/$root_ubivol -s $rootfs_length $1

	nand_do_upgrade_success
}

nand_upgrade_tar() {
	local tar_file="$1"
	local kernel_mtd="$(find_mtd_index $CI_KERNPART)"

	local board_dir=$(tar tf $tar_file | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}

	local kernel_length=$( (tar xf $tar_file ${board_dir}/kernel -O | wc -c) 2> /dev/null)
	local rootfs_length=$( (tar xf $tar_file ${board_dir}/root -O | wc -c) 2> /dev/null)

	local rootfs_type="$(identify_tar "$tar_file" ${board_dir}/root)"

	local has_kernel=1
	local has_env=0

	[ "$kernel_length" != 0 -a -n "$kernel_mtd" ] && {
		tar xf $tar_file ${board_dir}/kernel -O | mtd write - $CI_KERNPART
	}
	[ "$kernel_length" = 0 -o ! -z "$kernel_mtd" ] && has_kernel=0

	nand_upgrade_prepare_ubi "$rootfs_length" "$rootfs_type" "$has_kernel" "$has_env"

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	[ "$has_kernel" = "1" ] && {
		local kern_ubivol="$(nand_find_volume $ubidev $CI_KERNPART)"
		tar xf $tar_file ${board_dir}/kernel -O | \
			ubiupdatevol /dev/$kern_ubivol -s $kernel_length -
	}

	local root_ubivol="$(nand_find_volume $ubidev $CI_ROOTPART)"
	tar xf $tar_file ${board_dir}/root -O | \
		ubiupdatevol /dev/$root_ubivol -s $rootfs_length -

	nand_do_upgrade_success
}

# Recognize type of passed file and start the upgrade process
nand_do_upgrade() {
	local file_type=$(identify $1)

	[ ! "$(find_mtd_index "$CI_UBIPART")" ] && CI_UBIPART="rootfs"

	case "$file_type" in
		"ubi")		nand_upgrade_ubinized $1;;
		"ubifs")	nand_upgrade_ubifs $1;;
		*)		nand_upgrade_tar $1;;
	esac
}

# Check if passed file is a valid one for NAND sysupgrade. Currently it accepts
# 3 types of files:
# 1) UBI - should contain an ubinized image, header is checked for the proper
#    MAGIC
# 2) UBIFS - should contain UBIFS partition that will replace "rootfs" volume,
#    header is checked for the proper MAGIC
# 3) TAR - archive has to include "sysupgrade-BOARD" directory with a non-empty
#    "CONTROL" file (at this point its content isn't verified)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
#
# You usually want to call this function in platform_check_image.
#
# $(1): board name, used in case of passing TAR file
# $(2): file to be checked
nand_do_platform_check() {
	local board_name="$1"
<<<<<<< HEAD
	local file="$2"

	local cmd="$(identify_if_gzip "$file")cat"
	local file_type="$(identify "$file" "$cmd" "")"
	local control_length=$( ($cmd < "$file" | tar xOf - "sysupgrade-${board_name//,/_}/CONTROL" | wc -c) 2> /dev/null)

	if [ "$control_length" = 0 ]; then
		control_length=$( ($cmd < "$file" | tar xOf - "sysupgrade-${board_name//_/,}/CONTROL" | wc -c) 2> /dev/null)
	fi

	if [ "$control_length" != 0 ]; then
		nand_verify_tar_file "$file" "$cmd" || return 1
	else
		nand_verify_if_gzip_file "$file" "$cmd" || return 1
		if [ "$file_type" != "fit" -a "$file_type" != "ubi" -a "$file_type" != "ubifs" ]; then
			echo "invalid sysupgrade file"
			return 1
		fi
	fi

	return 0
}
=======
	local tar_file="$2"
	local control_length=$( (tar xf $tar_file sysupgrade-$board_name/CONTROL -O | wc -c) 2> /dev/null)
	local file_type="$(identify $2)"

	[ "$control_length" = 0 -a "$file_type" != "ubi" -a "$file_type" != "ubifs" ] && {
		echo "Invalid sysupgrade file."
		return 1
	}

	return 0
}

dual_boot_upgrade_prepare_ubi() {
	local kernel_vol_name="$1"
	local rootfs_vol_name="$2"
	local kernel_length="$3"
	local rootfs_length="$4"
	local reserve_rootfs_data="$5"

	local mtdnum="$( find_mtd_index "$CI_UBIPART" )"
	if [ ! "$mtdnum" ]; then
		echo "cannot find ubi mtd partition $CI_UBIPART"
		return 1
	fi

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	if [ ! "$ubidev" ]; then
		ubiattach -m "$mtdnum"
		sync
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	fi

	if [ ! "$ubidev" ]; then
		ubiformat /dev/mtd$mtdnum -y
		ubiattach -m "$mtdnum"
		sync
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
		ubimkvol /dev/$ubidev -n 0 -N u-boot-env -s 512KiB
	fi

	local rootfs_data_vol_name=$(cat /sys/module/boot_param/parameters/rootfs_data_part 2>/dev/null)

	local kern_ubivol="$( nand_find_volume $ubidev $kernel_vol_name )"
	local root_ubivol="$( nand_find_volume $ubidev $rootfs_vol_name )"
	local data_ubivol="$( nand_find_volume $ubidev $rootfs_data_vol_name )"

	# remove ubiblock device of rootfs
	local root_ubiblk="ubiblock${root_ubivol:3}"
	if [ "$root_ubivol" -a -e "/dev/$root_ubiblk" ]; then
		echo "removing $root_ubiblk"
		if ! ubiblock -r /dev/$root_ubivol; then
			echo "cannot remove $root_ubiblk"
			return 1;
		fi
	fi

	# kill volumes
	[ "$kern_ubivol" ] && ubirmvol /dev/$ubidev -N $kernel_vol_name || true
	[ "$root_ubivol" ] && ubirmvol /dev/$ubidev -N $rootfs_vol_name || true

	# update kernel
	if ! ubimkvol /dev/$ubidev -N $kernel_vol_name -s $kernel_length; then
		echo "cannot create kernel volume"
		return 1;
	fi

	# update rootfs
	if ! ubimkvol /dev/$ubidev -N $rootfs_vol_name -s $rootfs_length; then
		echo "cannot create rootfs volume"
		return 1;
	fi

	if [ x"${reserve_rootfs_data}" = xY ]; then
		# Do not touch rootfs_data
		sync
		return 0
	fi

	# 'format' rootfs_data volume
	[ "$data_ubivol" ] && {
		local rootfs_data_length=$(cat /sys/class/ubi/$data_ubivol/data_bytes)

		# kill rootfs_data volume
		ubirmvol /dev/$ubidev -N $rootfs_data_vol_name || true

		# update rootfs_data
		if ! ubimkvol /dev/$ubidev -N $rootfs_data_vol_name -s $rootfs_data_length; then
			echo "cannot create $rootfs_data_vol_name volume"
		fi
	}

	sync
	return 0
}

ubi_dual_boot_upgrade_tar() {
	local tar_file="$1"
	local board_dir=$(tar tf ${tar_file} | grep -m 1 '^sysupgrade-.*/$')
	local reserve_rootfs_data=$(cat /sys/module/boot_param/parameters/reserve_rootfs_data 2>/dev/null)
	board_dir=${board_dir%/}

	kernel_vol_name=$(cat /sys/module/boot_param/parameters/upgrade_kernel_part 2>/dev/null)
	[ -z "${kernel_vol_name}" -o $? -ne 0 ] && return 1

	rootfs_vol_name=$(cat /sys/module/boot_param/parameters/upgrade_rootfs_part 2>/dev/null)
	[ -z "${rootfs_vol_name}" -o $? -ne 0 ] && return 1

	local kernel_length=$( (tar xf ${tar_file} ${board_dir}/kernel -O | wc -c) 2> /dev/null)
	local rootfs_length=$( (tar xf ${tar_file} ${board_dir}/root -O | wc -c) 2> /dev/null)

	dual_boot_upgrade_prepare_ubi "${kernel_vol_name}" "${rootfs_vol_name}" \
				      "${kernel_length}" "${rootfs_length}" \
				      "${reserve_rootfs_data}"

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"

	[ "${kernel_length}" != 0 ] && {
		local kern_ubivol="$(nand_find_volume $ubidev ${kernel_vol_name})"
		tar xf ${tar_file} ${board_dir}/kernel -O | \
			ubiupdatevol /dev/${kern_ubivol} -s ${kernel_length} -
	}

	[ "${rootfs_length}" != 0 ] && {
		local root_ubivol="$(nand_find_volume $ubidev ${rootfs_vol_name})"
		tar xf ${tar_file} ${board_dir}/root -O | \
			ubiupdatevol /dev/${root_ubivol} -s ${rootfs_length} -
	}

	upgrade_image_slot=$(cat /sys/module/boot_param/parameters/upgrade_image_slot 2>/dev/null)
	[ -n "${upgrade_image_slot}" ] && {
		v "Set new boot image slot to ${upgrade_image_slot}"
		# Force the creation of fw_printenv.lock
		mkdir -p /var/lock
		touch /var/lock/fw_printenv.lock
		fw_setenv "dual_boot.current_slot" "${upgrade_image_slot}"
		fw_setenv "dual_boot.slot_${upgrade_image_slot}_invalid" "0"
	}

	if [ x"${reserve_rootfs_data}" != xY ]; then
		# do normal upgrade flow
		nand_do_upgrade_success
	fi

	# Do not touch rootfs_data
	sync

	echo "sysupgrade successful"
	umount -a
	reboot -f
}

ubi_do_upgrade() {
	local dual_boot=$(cat /sys/module/boot_param/parameters/dual_boot 2>/dev/null)
	local file_type=$(identify $1)

	if [ x"${dual_boot}" != xY ]; then
		nand_do_upgrade "$1"
		return
	fi

	case "$file_type" in
		"ubi")		v "Unsupported firmware type: ubinized";;
		"ubifs")	v "Unsupported firmware type: ubifs";;
		*)		ubi_dual_boot_upgrade_tar $1;;
	esac
}
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
