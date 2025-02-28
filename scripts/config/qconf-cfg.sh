#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-only

<<<<<<< HEAD
cflags=$1
libs=$2
bin=$3

PKG5="Qt5Core Qt5Gui Qt5Widgets"
PKG6="Qt6Core Qt6Gui Qt6Widgets"

if [ -z "$(command -v ${HOSTPKG_CONFIG})" ]; then
	echo >&2 "*"
	echo >&2 "* 'make xconfig' requires '${HOSTPKG_CONFIG}'. Please install it."
=======
PKG="Qt5Core Qt5Gui Qt5Widgets"
PKG2="QtCore QtGui"

if [ -z "$(command -v pkg-config)" ]; then
	echo >&2 "*"
	echo >&2 "* 'make xconfig' requires 'pkg-config'. Please install it."
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	echo >&2 "*"
	exit 1
fi

<<<<<<< HEAD
if ${HOSTPKG_CONFIG} --exists $PKG6; then
	${HOSTPKG_CONFIG} --cflags ${PKG6} > ${cflags}
	# Qt6 requires C++17.
	echo -std=c++17 >> ${cflags}
	${HOSTPKG_CONFIG} --libs ${PKG6} > ${libs}
	${HOSTPKG_CONFIG} --variable=libexecdir Qt6Core > ${bin}
	exit 0
fi

if ${HOSTPKG_CONFIG} --exists $PKG5; then
	${HOSTPKG_CONFIG} --cflags ${PKG5} > ${cflags}
	${HOSTPKG_CONFIG} --libs ${PKG5} > ${libs}
	${HOSTPKG_CONFIG} --variable=host_bins Qt5Core > ${bin}
=======
if pkg-config --exists $PKG; then
	echo cflags=\"-std=c++11 -fPIC $(pkg-config --cflags Qt5Core Qt5Gui Qt5Widgets)\"
	echo libs=\"$(pkg-config --libs $PKG)\"
	echo moc=\"$(pkg-config --variable=host_bins Qt5Core)/moc\"
	exit 0
fi

if pkg-config --exists $PKG2; then
	echo cflags=\"$(pkg-config --cflags $PKG2)\"
	echo libs=\"$(pkg-config --libs $PKG2)\"
	echo moc=\"$(pkg-config --variable=moc_location QtCore)\"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	exit 0
fi

echo >&2 "*"
<<<<<<< HEAD
echo >&2 "* Could not find Qt6 or Qt5 via ${HOSTPKG_CONFIG}."
echo >&2 "* Please install Qt6 or Qt5 and make sure it's in PKG_CONFIG_PATH"
echo >&2 "* You need $PKG6 for Qt6"
echo >&2 "* You need $PKG5 for Qt5"
=======
echo >&2 "* Could not find Qt via pkg-config."
echo >&2 "* Please install either Qt 4.8 or 5.x. and make sure it's in PKG_CONFIG_PATH"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
echo >&2 "*"
exit 1
