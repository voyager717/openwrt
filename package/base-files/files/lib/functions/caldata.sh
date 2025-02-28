# Copyright (C) 2019 OpenWrt.org

. /lib/functions.sh
. /lib/functions/system.sh

caldata_dd() {
	local source=$1
	local target=$2
	local count=$(($3))
	local offset=$(($4))

	dd if=$source of=$target iflag=skip_bytes,fullblock bs=$count skip=$offset count=1 2>/dev/null
	return $?
}

caldata_die() {
	echo "caldata: " "$*"
	exit 1
}

caldata_extract() {
	local part=$1
	local offset=$(($2))
	local count=$(($3))
	local mtd

	mtd=$(find_mtd_chardev $part)
	[ -n "$mtd" ] || caldata_die "no mtd device found for partition $part"

	caldata_dd $mtd /lib/firmware/$FIRMWARE $count $offset || \
		caldata_die "failed to extract calibration data from $mtd"
}

caldata_extract_ubi() {
	local part=$1
	local offset=$(($2))
	local count=$(($3))
	local ubidev
	local ubi

	. /lib/upgrade/nand.sh

	ubidev=$(nand_find_ubi $CI_UBIPART)
	ubi=$(nand_find_volume $ubidev $part)
	[ -n "$ubi" ] || caldata_die "no UBI volume found for $part"

	caldata_dd /dev/$ubi /lib/firmware/$FIRMWARE $count $offset || \
		caldata_die "failed to extract calibration data from $ubi"
}

<<<<<<< HEAD
caldata_extract_mmc() {
	local part=$1
	local offset=$(($2))
	local count=$(($3))
	local mmc_part

	mmc_part=$(find_mmc_part $part)
	[ -n "$mmc_part" ] || caldata_die "no mmc partition found for partition $part"

	caldata_dd $mmc_part /lib/firmware/$FIRMWARE $count $offset || \
		caldata_die "failed to extract calibration data from $mmc_part"
}

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
caldata_extract_reverse() {
	local part=$1
	local offset=$2
	local count=$(($3))
	local mtd
	local reversed
	local caldata

	mtd=$(find_mtd_chardev "$part")
<<<<<<< HEAD
	reversed=$(hexdump -v -s $offset -n $count -e '1/1 "%02x "' $mtd)
=======
	reversed=$(hexdump -v -s $offset -n $count -e '/1 "%02x "' $mtd)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

	for byte in $reversed; do
		caldata="\x${byte}${caldata}"
	done

	printf "%b" "$caldata" > /lib/firmware/$FIRMWARE
}

caldata_from_file() {
	local source=$1
	local offset=$(($2))
	local count=$(($3))
	local target=$4

	[ -n "$target" ] || target=/lib/firmware/$FIRMWARE

	caldata_dd $source $target $count $offset || \
		caldata_die "failed to extract calibration data from $source"
}

caldata_sysfsload_from_file() {
	local source=$1
	local offset=$(($2))
	local count=$(($3))
	local target_dir="/sys/$DEVPATH"
	local target="$target_dir/data"

	[ -d "$target_dir" ] || \
		caldata_die "no sysfs dir to write: $target"

	echo 1 > "$target_dir/loading"
	caldata_dd $source $target $count $offset
	if [ $? != 0 ]; then
		echo 1 > "$target_dir/loading"
		caldata_die "failed to extract calibration data from $source"
	else
		echo 0 > "$target_dir/loading"
	fi
}

caldata_valid() {
	local expected="$1"
	local target=$2

	[ -n "$target" ] || target=/lib/firmware/$FIRMWARE

	magic=$(hexdump -v -n 2 -e '1/1 "%02x"' $target)
	[ "$magic" = "$expected" ]
	return $?
}

<<<<<<< HEAD
caldata_patch_data() {
	local data=$1
	local data_count=$((${#1} / 2))
	[ -n "$2" ] && local data_offset=$(($2))
	[ -n "$3" ] && local chksum_offset=$(($3))
	local target=$4
	local fw_data
	local fw_chksum

	[ -z "$data" -o -z "$data_offset" ] && return

	[ -n "$target" ] || target=/lib/firmware/$FIRMWARE

	fw_data=$(hexdump -v -n $data_count -s $data_offset -e '1/1 "%02x"' $target)

	if [ "$data" != "$fw_data" ]; then

		if [ -n "$chksum_offset" ]; then
			fw_chksum=$(hexdump -v -n 2 -s $chksum_offset -e '1/1 "%02x"' $target)
			fw_chksum=$(xor $fw_chksum $(data_2xor_val $fw_data) $(data_2xor_val $data))

			data_2bin $fw_chksum | \
				dd of=$target conv=notrunc bs=1 seek=$chksum_offset count=2 || \
				caldata_die "failed to write chksum to eeprom file"
		fi

		data_2bin $data | \
			dd of=$target conv=notrunc bs=1 seek=$data_offset count=$data_count || \
			caldata_die "failed to write data to eeprom file"
	fi
=======
caldata_patch_chksum() {
	local mac=$1
	local mac_offset=$(($2))
	local chksum_offset=$(($3))
	local target=$4
	local xor_mac
	local xor_fw_mac
	local xor_fw_chksum

	xor_mac=${mac//:/}
	xor_mac="${xor_mac:0:4} ${xor_mac:4:4} ${xor_mac:8:4}"

	xor_fw_mac=$(hexdump -v -n 6 -s $mac_offset -e '/1 "%02x"' /lib/firmware/$FIRMWARE)
	xor_fw_mac="${xor_fw_mac:0:4} ${xor_fw_mac:4:4} ${xor_fw_mac:8:4}"

	xor_fw_chksum=$(hexdump -v -n 2 -s $chksum_offset -e '/1 "%02x"' /lib/firmware/$FIRMWARE)
	xor_fw_chksum=$(xor $xor_fw_chksum $xor_fw_mac $xor_mac)

	printf "%b" "\x${xor_fw_chksum:0:2}\x${xor_fw_chksum:2:2}" | \
		dd of=$target conv=notrunc bs=1 seek=$chksum_offset count=2
}

caldata_patch_mac() {
	local mac=$1
	local mac_offset=$(($2))
	local chksum_offset=$3
	local target=$4

	[ -z "$mac" -o -z "$mac_offset" ] && return

	[ -n "$target" ] || target=/lib/firmware/$FIRMWARE

	[ -n "$chksum_offset" ] && caldata_patch_chksum "$mac" "$mac_offset" "$chksum_offset" "$target"

	macaddr_2bin $mac | dd of=$target conv=notrunc oflag=seek_bytes bs=6 seek=$mac_offset count=1 || \
		caldata_die "failed to write MAC address to eeprom file"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
}

ath9k_patch_mac() {
	local mac=$1
	local target=$2

<<<<<<< HEAD
	caldata_patch_data "${mac//:/}" 0x2 "" "$target"
=======
	caldata_patch_mac "$mac" 0x2 "" "$target"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
}

ath9k_patch_mac_crc() {
	local mac=$1
	local mac_offset=$2
	local chksum_offset=$((mac_offset - 10))
	local target=$4

<<<<<<< HEAD
	caldata_patch_data "${mac//:/}" "$mac_offset" "$chksum_offset" "$target"
=======
	caldata_patch_mac "$mac" "$mac_offset" "$chksum_offset" "$target"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
}

ath10k_patch_mac() {
	local mac=$1
	local target=$2

<<<<<<< HEAD
	caldata_patch_data "${mac//:/}" 0x6 0x2 "$target"
}

ath11k_patch_mac() {
	local mac=$1
	# mac_id from 0 to 5
	local mac_id=$2
	local target=$3

	[ -z "$mac_id" ] && return

	caldata_patch_data "${mac//:/}" $(printf "0x%x" $(($mac_id * 0x6 + 0xe))) 0xa "$target"
}

ath10k_remove_regdomain() {
	local target=$1

	caldata_patch_data "0000" 0xc 0x2 "$target"
}

ath11k_remove_regdomain() {
	local target=$1
	local regdomain
	local regdomain_data

	regdomain=$(hexdump -v -n 2 -s 0x34 -e '1/1 "%02x"' $target)
	caldata_patch_data "0000" 0x34 0xa "$target"
	
	for offset in 0x450 0x458 0x500 0x5a8; do
		regdomain_data=$(hexdump -v -n 2 -s $offset -e '1/1 "%02x"' $target)

		if [ "$regdomain" == "$regdomain_data" ]; then
			caldata_patch_data "0000" $offset 0xa "$target"
		fi
	done
}

ath11k_set_macflag() {
	local target=$1

	caldata_patch_data "0100" 0x3e 0xa "$target"
=======
	caldata_patch_mac "$mac" 0x6 0x2 "$target"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
}
