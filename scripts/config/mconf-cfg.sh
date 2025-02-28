#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-only

<<<<<<< HEAD
cflags=$1
libs=$2

PKG="ncursesw"
PKG2="ncurses"

if [ -n "$(command -v ${HOSTPKG_CONFIG})" ]; then
	if ${HOSTPKG_CONFIG} --exists $PKG; then
		${HOSTPKG_CONFIG} --cflags ${PKG} > ${cflags}
		${HOSTPKG_CONFIG} --libs ${PKG} > ${libs}
		exit 0
	fi

	if ${HOSTPKG_CONFIG} --exists ${PKG2}; then
		${HOSTPKG_CONFIG} --cflags ${PKG2} > ${cflags}
		${HOSTPKG_CONFIG} --libs ${PKG2} > ${libs}
=======
PKG="ncursesw"
PKG2="ncurses"

if [ -n "$(command -v pkg-config)" ]; then
	if pkg-config --exists $PKG; then
		echo cflags=\"$(pkg-config --cflags $PKG)\"
		echo libs=\"$(pkg-config --libs $PKG)\"
		exit 0
	fi

	if pkg-config --exists $PKG2; then
		echo cflags=\"$(pkg-config --cflags $PKG2)\"
		echo libs=\"$(pkg-config --libs $PKG2)\"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
		exit 0
	fi
fi

# Check the default paths in case pkg-config is not installed.
# (Even if it is installed, some distributions such as openSUSE cannot
# find ncurses by pkg-config.)
if [ -f /usr/include/ncursesw/ncurses.h ]; then
<<<<<<< HEAD
	echo -D_GNU_SOURCE -I/usr/include/ncursesw > ${cflags}
	echo -lncursesw > ${libs}
=======
	echo cflags=\"-D_GNU_SOURCE -I/usr/include/ncursesw\"
	echo libs=\"-lncursesw\"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	exit 0
fi

if [ -f /usr/include/ncurses/ncurses.h ]; then
<<<<<<< HEAD
	echo -D_GNU_SOURCE -I/usr/include/ncurses > ${cflags}
	echo -lncurses > ${libs}
	exit 0
fi

# As a final fallback before giving up, check if $HOSTCC knows of a default
# ncurses installation (e.g. from a vendor-specific sysroot).
if echo '#include <ncurses.h>' | ${HOSTCC} -E - >/dev/null 2>&1; then
	echo -D_GNU_SOURCE > ${cflags}
	echo -lncurses > ${libs}
=======
	echo cflags=\"-D_GNU_SOURCE -I/usr/include/ncurses\"
	echo libs=\"-lncurses\"
	exit 0
fi

if [ -f /usr/include/ncurses.h ]; then
	echo cflags=\"-D_GNU_SOURCE\"
	echo libs=\"-lncurses\"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	exit 0
fi

echo >&2 "*"
echo >&2 "* Unable to find the ncurses package."
echo >&2 "* Install ncurses (ncurses-devel or libncurses-dev"
echo >&2 "* depending on your distribution)."
echo >&2 "*"
<<<<<<< HEAD
echo >&2 "* You may also need to install ${HOSTPKG_CONFIG} to find the"
=======
echo >&2 "* You may also need to install pkg-config to find the"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
echo >&2 "* ncurses installed in a non-default location."
echo >&2 "*"
exit 1
