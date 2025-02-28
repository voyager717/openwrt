#!/bin/sh -e
# under GPLv2
# heavily influenced by OpenWrt/LEDE scripts/getver.sh
#                    by coreboot util/genbuild_h/genbuild_h.sh

export LANG=C
export LC_ALL=C
export TZ=UTC

# top directory of the squashfs, used for git detection
TOP="$1"
OUTPUT="$2"
REV=""

# The env SOURCE_DATE_EPOCH might be set from outside

if [ "$#" -gt 2 ] ; then
	echo "$0 [[<topdir>] <outputfile>]"
fi

if [ -z "$TOP" ] ; then
	TOP="$(dirname "$0")/.."
fi

if [ -z "$OUTPUT" ] ; then
	OUTPUT="squashfs-tools/version.h"
fi

our_date() {
	if date --version 2>&1 | grep -q "GNU coreutils"; then
		date -d "@$1" "$2"
	else
		date -r "$1" "$2"
	fi
}

try_version() {
        [ -f version ] && [ -f version.date ] || return 1
        REV="$(cat version)"
        SOURCE_DATE_EPOCH="$(cat version.date)"

        [ -n "$REV" ] && [ -n "$SOURCE_DATE_EPOCH" ]
}

try_git() {
	[ -d .git ] || return 1

	REV="$(git describe --tags --always --dirty 2>/dev/null)"
	SOURCE_DATE_EPOCH="$(git log -1 --format=format:%ct)"

        [ -n "$REV" ] && [ -n "$SOURCE_DATE_EPOCH" ]
}

output_version() {
	echo "Writing $OUTPUT"
	DATE="$(our_date "$SOURCE_DATE_EPOCH" +%Y/%m/%d)"
	cat > "$OUTPUT" <<EOF
#define VERSION_STR "squashfskit-$REV"
#define VERSION_DATE_STR "$DATE"
EOF
}

cd "$TOP"
try_git || try_version || REV="unknown"
[ -z "$SOURCE_DATE_EPOCH" ] && SOURCE_DATE_EPOCH="$(date %s)"
output_version
