#!/bin/sh

prefix=/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/staging_dir/host
exec_prefix=/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/staging_dir/host
bindir=${exec_prefix}/bin
libexecpath=${exec_prefix}/libexec/mtd-utils
TESTBINDIR="."

TEST_DIR=$TEST_FILE_SYSTEM_MOUNT_DIR
if test -z "$TEST_DIR";
then
	TEST_DIR="/mnt/test_file_system"
fi

FREESPACE=`$TESTBINDIR/free_space "$TEST_DIR"`

if test -z "$FREESPACE";
then
	echo "Failed to determine free space"
	exit 1
fi

if test -n "$1";
then
	DURATION="-d$1";
else
	DURATION="";
fi

FWRITE00=$TESTBINDIR/fwrite00
RNDWR=$TESTBINDIR/rndwrite00
PDFLUSH=$TESTBINDIR/pdfrun
FSIZE=$(( $FREESPACE/15 ));

$TESTBINDIR/fstest_monitor $DURATION \
"$FWRITE00 -z $FSIZE -n0 -p 300" \
"$FWRITE00 -z $FSIZE -n0 -u" \
"$FWRITE00 -z $FSIZE -n0 -u -c" \
"$FWRITE00 -z $FSIZE -n0 -s -o" \
"$RNDWR -z $FSIZE -n0 -e"

STATUS=$?

rm -rf ${TEST_DIR}/*

exit $STATUS
