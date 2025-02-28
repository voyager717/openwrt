#!/bin/sh
#
# Licensed under the terms of the GNU GPL License version 2 or later.
#
# Author: Peter Tyser <ptyser@xes-inc.com>
#
# U-Boot firmware supports the booting of images in the Flattened Image
# Tree (FIT) format.  The FIT format uses a device tree structure to
# describe a kernel image, device tree blob, ramdisk, etc.  This script
# creates an Image Tree Source (.its file) which can be passed to the
# 'mkimage' utility to generate an Image Tree Blob (.itb file).  The .itb
# file can then be booted by U-Boot (or other bootloaders which support
# FIT images).  See doc/uImage.FIT/howto.txt in U-Boot source code for
# additional information on FIT images.
#

usage() {
	printf "Usage: %s -A arch -C comp -a addr -e entry" "$(basename "$0")"
	printf " -v version -k kernel [-D name -n address -d dtb] -o its_file"
<<<<<<< HEAD
=======
	printf " [-s script] [-S key_name_hint] [-r ar_ver] [-R rootfs]"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

	printf "\n\t-A ==> set architecture to 'arch'"
	printf "\n\t-C ==> set compression type 'comp'"
	printf "\n\t-c ==> set config name 'config'"
	printf "\n\t-a ==> set load address to 'addr' (hex)"
	printf "\n\t-e ==> set entry point to 'entry' (hex)"
<<<<<<< HEAD
	printf "\n\t-f ==> set device tree compatible string"
	printf "\n\t-i ==> include initrd Blob 'initrd'"
=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	printf "\n\t-v ==> set kernel version to 'version'"
	printf "\n\t-k ==> include kernel image 'kernel'"
	printf "\n\t-D ==> human friendly Device Tree Blob 'name'"
	printf "\n\t-n ==> fdt unit-address 'address'"
	printf "\n\t-d ==> include Device Tree Blob 'dtb'"
<<<<<<< HEAD
	printf "\n\t-r ==> include RootFS blob 'rootfs'"
	printf "\n\t-H ==> specify hash algo instead of SHA1"
	printf "\n\t-l ==> legacy mode character (@ etc otherwise -)"
	printf "\n\t-o ==> create output file 'its_file'"
	printf "\n\t-O ==> create config with dt overlay 'name:dtb'"
	printf "\n\t-s ==> set FDT load address to 'addr' (hex)"
	printf "\n\t\t(can be specified more than once)\n"
	exit 1
}

REFERENCE_CHAR='-'
FDTNUM=1
ROOTFSNUM=1
INITRDNUM=1
HASH=sha1
LOADABLES=
DTOVERLAY=
DTADDR=

while getopts ":A:a:c:C:D:d:e:f:i:k:l:n:o:O:v:r:s:H:" OPTION
=======
	printf "\n\t-o ==> create output file 'its_file'"
	printf "\n\t-s ==> include u-boot script 'script'"
	printf "\n\t-S ==> add signature at configurations and assign its key_name_hint by 'key_name_hint'"
	printf "\n\t-r ==> set anti-rollback version to 'fw_ar_ver' (dec)"
	printf "\n\t-R ==> specify rootfs file for embedding hash\n"
	exit 1
}

FDTNUM=1

while getopts ":A:a:c:C:D:d:e:k:n:o:v:s:S:r:R:" OPTION
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
do
	case $OPTION in
		A ) ARCH=$OPTARG;;
		a ) LOAD_ADDR=$OPTARG;;
		c ) CONFIG=$OPTARG;;
		C ) COMPRESS=$OPTARG;;
		D ) DEVICE=$OPTARG;;
		d ) DTB=$OPTARG;;
		e ) ENTRY_ADDR=$OPTARG;;
<<<<<<< HEAD
		f ) COMPATIBLE=$OPTARG;;
		i ) INITRD=$OPTARG;;
		k ) KERNEL=$OPTARG;;
		l ) REFERENCE_CHAR=$OPTARG;;
		n ) FDTNUM=$OPTARG;;
		o ) OUTPUT=$OPTARG;;
		O ) DTOVERLAY="$DTOVERLAY ${OPTARG}";;
		r ) ROOTFS=$OPTARG;;
		s ) FDTADDR=$OPTARG;;
		H ) HASH=$OPTARG;;
		v ) VERSION=$OPTARG;;
=======
		k ) KERNEL=$OPTARG;;
		n ) FDTNUM=$OPTARG;;
		o ) OUTPUT=$OPTARG;;
		v ) VERSION=$OPTARG;;
		s ) UBOOT_SCRIPT=$OPTARG;;
		S ) KEY_NAME_HINT=$OPTARG;;
		r ) AR_VER=$OPTARG;;
		R ) ROOTFS_FILE=$OPTARG;;
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
		* ) echo "Invalid option passed to '$0' (options:$*)"
		usage;;
	esac
done

# Make sure user entered all required parameters
if [ -z "${ARCH}" ] || [ -z "${COMPRESS}" ] || [ -z "${LOAD_ADDR}" ] || \
	[ -z "${ENTRY_ADDR}" ] || [ -z "${VERSION}" ] || [ -z "${KERNEL}" ] || \
	[ -z "${OUTPUT}" ] || [ -z "${CONFIG}" ]; then
	usage
fi

ARCH_UPPER=$(echo "$ARCH" | tr '[:lower:]' '[:upper:]')

<<<<<<< HEAD
if [ -n "${COMPATIBLE}" ]; then
	COMPATIBLE_PROP="compatible = \"${COMPATIBLE}\";"
fi

[ "$FDTADDR" ] && {
	DTADDR="$FDTADDR"
}

# Conditionally create fdt information
if [ -n "${DTB}" ]; then
	FDT_NODE="
		fdt${REFERENCE_CHAR}$FDTNUM {
			description = \"${ARCH_UPPER} OpenWrt ${DEVICE} device tree blob\";
			${COMPATIBLE_PROP}
			data = /incbin/(\"${DTB}\");
			type = \"flat_dt\";
			${DTADDR:+load = <${DTADDR}>;}
			arch = \"${ARCH}\";
			compression = \"none\";
			hash${REFERENCE_CHAR}1 {
				algo = \"crc32\";
			};
			hash${REFERENCE_CHAR}2 {
				algo = \"${HASH}\";
			};
		};
"
	FDT_PROP="fdt = \"fdt${REFERENCE_CHAR}$FDTNUM\";"
fi

if [ -n "${INITRD}" ]; then
	INITRD_NODE="
		initrd${REFERENCE_CHAR}$INITRDNUM {
			description = \"${ARCH_UPPER} OpenWrt ${DEVICE} initrd\";
			${COMPATIBLE_PROP}
			data = /incbin/(\"${INITRD}\");
			type = \"ramdisk\";
			arch = \"${ARCH}\";
			os = \"linux\";
			hash${REFERENCE_CHAR}1 {
				algo = \"crc32\";
			};
			hash${REFERENCE_CHAR}2 {
				algo = \"${HASH}\";
			};
		};
"
	INITRD_PROP="ramdisk=\"initrd${REFERENCE_CHAR}${INITRDNUM}\";"
fi


if [ -n "${ROOTFS}" ]; then
	dd if="${ROOTFS}" of="${ROOTFS}.pagesync" bs=4096 conv=sync
	ROOTFS_NODE="
		rootfs${REFERENCE_CHAR}$ROOTFSNUM {
			description = \"${ARCH_UPPER} OpenWrt ${DEVICE} rootfs\";
			${COMPATIBLE_PROP}
			data = /incbin/(\"${ROOTFS}.pagesync\");
			type = \"filesystem\";
			arch = \"${ARCH}\";
			compression = \"none\";
			hash${REFERENCE_CHAR}1 {
				algo = \"crc32\";
			};
			hash${REFERENCE_CHAR}2 {
				algo = \"${HASH}\";
			};
		};
"
	LOADABLES="${LOADABLES:+$LOADABLES, }\"rootfs${REFERENCE_CHAR}${ROOTFSNUM}\""
fi

# add DT overlay blobs
FDTOVERLAY_NODE=""
OVCONFIGS=""
[ "$DTOVERLAY" ] && for overlay in $DTOVERLAY ; do
	overlay_blob=${overlay##*:}
	ovname=${overlay%%:*}
	ovnode="fdt-$ovname"
	ovsize=$(wc -c "$overlay_blob" | awk '{print $1}')
	echo "$ovname ($overlay_blob) : $ovsize" >&2
	FDTOVERLAY_NODE="$FDTOVERLAY_NODE

		$ovnode {
			description = \"${ARCH_UPPER} OpenWrt ${DEVICE} device tree overlay $ovname\";
			${COMPATIBLE_PROP}
			data = /incbin/(\"${overlay_blob}\");
			type = \"flat_dt\";
			arch = \"${ARCH}\";
			compression = \"none\";
			hash${REFERENCE_CHAR}1 {
				algo = \"crc32\";
			};
			hash${REFERENCE_CHAR}2 {
				algo = \"${HASH}\";
			};
		};
"
	OVCONFIGS="$OVCONFIGS

		$ovname {
			description = \"OpenWrt ${DEVICE} overlay $ovname\";
			fdt = \"$ovnode\";
			${COMPATIBLE_PROP}
		};
	"
done
=======
# Conditionally create fdt information
if [ -n "${DTB}" ]; then
	FDT_NODE="
		fdt-$FDTNUM {
			description = \"${ARCH_UPPER} OpenWrt ${DEVICE} device tree blob\";
			data = /incbin/(\"${DTB}\");
			type = \"flat_dt\";
			arch = \"${ARCH}\";
			compression = \"none\";
			hash-1 {
				algo = \"crc32\";
			};
			hash-2 {
				algo = \"sha1\";
			};
		};
"
	FDT_PROP="fdt = \"fdt-$FDTNUM\";"
fi

# Conditionally create rootfs hash information
if [ -f "${ROOTFS_FILE}" ]; then
	ROOTFS_SIZE=$(stat -c %s ${ROOTFS_FILE})

	ROOTFS_SHA1=$(sha1sum ${ROOTFS_FILE} | awk '{print "<0x"substr($0,1,8) " 0x"substr($0,9,8) " 0x"substr($0,17,8) " 0x"substr($0,25,8) " 0x"substr($0,33,8) ">"}')
	ROOTFS_CRC32=$(crc32sum ${ROOTFS_FILE})

	ROOTFS="
	rootfs {
		size = <${ROOTFS_SIZE}>;

		hash-1 {
			value = <0x${ROOTFS_CRC32}>;
			algo = \"crc32\";
		};

		hash-2 {
			value = ${ROOTFS_SHA1};
			algo = \"sha1\";
		};
	};
"
fi

# Conditionally create script information
if [ -n "${UBOOT_SCRIPT}" ]; then
	SCRIPT="\
		script-1 {
			description = \"U-Boot Script\";
			data = /incbin/(\"${UBOOT_SCRIPT}\");
			type = \"script\";
			arch = \"${ARCH}\";
			os = \"linux\";
			load = <0>;
			entry = <0>;
			compression = \"none\";
			hash-1 {
				algo = \"crc32\";
			};
			hash-2 {
				algo = \"sha1\";
			};
		};\
"
	LOADABLES="\
			loadables = \"script-1\";\
"
	SIGN_IMAGES="\
				sign-images = \"fdt\", \"kernel\", \"loadables\";\
"
else
	SIGN_IMAGES="\
				sign-images = \"fdt\", \"kernel\";\
"
fi

# Conditionally create signature information
if [ -n "${KEY_NAME_HINT}" ]; then
	SIGNATURE="\
			signature {
				algo = \"sha1,rsa2048\";
				key-name-hint = \"${KEY_NAME_HINT}\";
${SIGN_IMAGES}
			};\
"
fi

# Conditionally create anti-rollback version information
if [ -n "${AR_VER}" ]; then
	FW_AR_VER="\
			fw_ar_ver = <${AR_VER}>;\
"
fi
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

# Create a default, fully populated DTS file
DATA="/dts-v1/;

/ {
	description = \"${ARCH_UPPER} OpenWrt FIT (Flattened Image Tree)\";
	#address-cells = <1>;

	images {
<<<<<<< HEAD
		kernel${REFERENCE_CHAR}1 {
=======
		kernel-1 {
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
			description = \"${ARCH_UPPER} OpenWrt Linux-${VERSION}\";
			data = /incbin/(\"${KERNEL}\");
			type = \"kernel\";
			arch = \"${ARCH}\";
			os = \"linux\";
			compression = \"${COMPRESS}\";
			load = <${LOAD_ADDR}>;
			entry = <${ENTRY_ADDR}>;
<<<<<<< HEAD
			hash${REFERENCE_CHAR}1 {
				algo = \"crc32\";
			};
			hash${REFERENCE_CHAR}2 {
				algo = \"$HASH\";
			};
		};
${INITRD_NODE}
${FDT_NODE}
${FDTOVERLAY_NODE}
${ROOTFS_NODE}
	};

	configurations {
		default = \"${CONFIG}\";
		${CONFIG} {
			description = \"OpenWrt ${DEVICE}\";
			kernel = \"kernel${REFERENCE_CHAR}1\";
			${FDT_PROP}
			${LOADABLES:+loadables = ${LOADABLES};}
			${COMPATIBLE_PROP}
			${INITRD_PROP}
		};
		${OVCONFIGS}
=======
			hash-1 {
				algo = \"crc32\";
			};
			hash-2 {
				algo = \"sha1\";
			};
		};
${FDT_NODE}
${SCRIPT}
	};

${ROOTFS}

	configurations {
		default = \"${CONFIG}\";
		${CONFIG} {
			description = \"OpenWrt\";
${FW_AR_VER}
${LOADABLES}
			kernel = \"kernel-1\";
			${FDT_PROP}
${SIGNATURE}
		};
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	};
};"

# Write .its file to disk
echo "$DATA" > "${OUTPUT}"
