#!/bin/sh
<<<<<<< HEAD
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2024 OpenWrt.org
#
# This script creates a tar file for the Linksys switches of the LGS3xxC/LGS3xxMPC
# series. It contains not only the OpenWrt firmware but additional scripts that
# are needed for the upgrade.
#
# ./linksys-image.py <ImageFile> <ImageFileOut> <LinksysModel>
#
# Known values for LinksysModel are currently
#
# LGS310MPC		60402010
# LGS310C		60402060
# LGS328PC		60401070
# LGS328PC(RTL8218D)	60401080
# LGS310MPCv2		60402090
# LGS328MPC		60412020
# LGS328C		60412040
# LGS328MPCv2		60412060
# LGS352MPC		60422030
# LGS352C		60422050
# LGS352MPCv2		60422070

# The check script that verifies if the images matches the hardware model
gen_imagecheck() {
	echo '#!/bin/sh'
	echo 'if [ "$1" = "'${1}'" ]; then'
	echo 'echo 0'
	echo 'else'
	echo 'echo 1'
	echo 'fi'
}

# Generic attributes
gen_fwinfo() {
	echo 'FW_VERSION=1.01.100\nBOOT_VERSION=01.00.01'
}

# NOR upgrade script. It allows to install OpenWrt only to first partition.
gen_nor_upgrade() {
	echo '#!/bin/sh'
	echo 'flash_bank=65536'
	echo 'filesize=`stat --format=%s ./series_vmlinux.bix`'
	echo 'num_bank=`expr \( ${filesize} + ${flash_bank} - 1 \) / ${flash_bank}`'
	echo 'filesize_bank=`expr ${num_bank} \* ${flash_bank}`'
	echo 'case $1 in'
	echo '1)'
	echo 'mtd_debug erase $2 0 ${filesize_bank} >/dev/null 2>&1'
	echo 'mtd_debug write $2 0 ${filesize} ./series_vmlinux.bix >/dev/null 2>&1'
	echo 'mtd_debug read $2 0 100 image1.img >/dev/null 2>&1'
	echo 'CreateImage -r ./image1.img > /tmp/app/image1.txt'
	echo 'echo 0'
	echo ';;'
	echo '*)'
	echo 'echo 1'
	echo 'esac'
}

# NAND upgrade script. It allows to install OpenWrt only to first partition.
gen_nand_upgrade() {
	echo '#!/bin/sh'
	echo 'case $1 in'
	echo '1)'
	echo 'flash_eraseall $2 >/dev/null 2>&1'
	echo 'nandwrite -p $2 ./series_vmlinux.bix >/dev/null 2>&1'
	echo 'mtd_debug read $2 0 100 image1.img >/dev/null 2>&1'
	echo 'CreateImage -r ./image1.img > /tmp/app/image1.txt'
	echo 'echo 0'
	echo ';;'
	echo '*)'
	echo 'echo 1'
	echo 'esac'
}

tmpdir="$( mktemp -d 2> /dev/null )"
imgdir=$tmpdir/image
mkdir $imgdir

gen_imagecheck $3 > $imgdir/iss_imagecheck.sh
gen_nor_upgrade > $imgdir/iss_imageupgrade.sh
gen_nand_upgrade > $imgdir/iss_nand_imageupgrade.sh
gen_fwinfo > $imgdir/firmware_information.txt

chmod +x $imgdir/iss_imagecheck.sh
chmod +x $imgdir/iss_imageupgrade.sh
chmod +x $imgdir/iss_nand_imageupgrade.sh

cp $1 $imgdir/series_vmlinux.bix

tar cf $2 -C $tmpdir image/

rm -rf $tmpdir
=======
#
# Copyright (C) 2018 Oceanic Systems (UK) Ltd
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# Maintained by: Ryan Pannell <ryan [at] o s u k l .com> <github.com/Escalion>
#
# Write Linksys signature for factory image
# This is appended to the factory image and is tested by the Linksys Upgrader - as observed in civic.
# The footer is 256 bytes. The format is:
#  .LINKSYS.        This is detected by the Linksys upgrader before continuing with upgrade. (9 bytes)
#  <VERSION>        The version number of upgrade. Not checked so use arbitary value (8 bytes)
#  <TYPE>           Model of target device, padded (0x20) to (15 bytes)
#  <CRC>      	    CRC checksum of the image to flash (8 byte)
#  <padding>	    Padding (0x20) (7 bytes)
#  <signature>	    Signature of signer. Not checked so use Arbitary value (16 bytes)
#  <padding>        Padding (0x00) (192 bytes)
#  0x0A		    (1 byte)

## version history
# * version 1: initial commit

set -e

ME="${0##*/}"

usage() {
	echo "Usage: $ME <type> <in filename>"
	[ "$IMG_OUT" ] && rm -f "$IMG_OUT"
	exit 1
}

[ "$#" -lt 3 ] && usage

TYPE=$1

tmpdir="$( mktemp -d 2> /dev/null )"
if [ -z "$tmpdir" ]; then
	# try OSX signature
	tmpdir="$( mktemp -t 'ubitmp' -d )"
fi

if [ -z "$tmpdir" ]; then
	exit 1
fi

trap "rm -rf $tmpdir" EXIT

IMG_TMP_OUT="${tmpdir}/out"

IMG_IN=$2
IMG_OUT="${IMG_IN}.new"

[ ! -f "$IMG_IN" ] && echo "$ME: Not a valid image: $IMG_IN" && usage

dd if="${IMG_IN}" of="${IMG_TMP_OUT}"
CRC=$(printf "%08X" $(dd if="${IMG_IN}" bs=$(stat -c%s "${IMG_IN}") count=1|cksum| cut -d ' ' -f1))

printf ".LINKSYS.01000409%-15s%-8s%-7s%-16s" "${TYPE}" "${CRC}" "" "K0000000F0246434" >> "${IMG_TMP_OUT}"

dd if=/dev/zero bs=1 count=192 conv=notrunc >> "${IMG_TMP_OUT}"

printf '\12' >> "${IMG_TMP_OUT}"

cp "${IMG_TMP_OUT}" "${IMG_OUT}"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
