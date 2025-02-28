#!/bin/sh

<<<<<<< HEAD
set -e -x

if [ $# -ne 5 ]; then
    echo "SYNTAX: $0 <file> <bootfs image> <rootfs image> <bootfs size> <rootfs size>"
    exit 1
fi
=======
set -x
[ $# -eq 5 ] || {
    echo "SYNTAX: $0 <file> <bootfs image> <rootfs image> <bootfs size> <rootfs size>"
    exit 1
}
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

OUTPUT="$1"
BOOTFS="$2"
ROOTFS="$3"
BOOTFSSIZE="$4"
ROOTFSSIZE="$5"

<<<<<<< HEAD
align=4096
head=4
kernel_type=c
rootfs_type=83
sect=63

set $(ptgen -o $OUTPUT -h $head -s $sect -l $align -t $kernel_type -p ${BOOTFSSIZE}M -t $rootfs_type -p ${ROOTFSSIZE}M ${SIGNATURE:+-S 0x$SIGNATURE})

BOOTOFFSET="$(($1 / 512))"
ROOTFSOFFSET="$(($3 / 512))"

dd bs=512 if="$BOOTFS" of="$OUTPUT" seek="$BOOTOFFSET" conv=notrunc
dd bs=512 if="$ROOTFS" of="$OUTPUT" seek="$ROOTFSOFFSET" conv=notrunc
=======
head=4
sect=63

set $(ptgen -o $OUTPUT -h $head -s $sect -l 4096 -t c -p ${BOOTFSSIZE}M -t 83 -p ${ROOTFSSIZE}M)

BOOTOFFSET="$(($1 / 512))"
BOOTSIZE="$(($2 / 512))"
ROOTFSOFFSET="$(($3 / 512))"
ROOTFSSIZE="$(($4 / 512))"

dd bs=512 if="$BOOTFS" of="$OUTPUT" seek="$BOOTOFFSET" conv=notrunc
dd bs=512 if="$ROOTFS" of="$OUTPUT" seek="$ROOTFSOFFSET" conv=notrunc



>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
