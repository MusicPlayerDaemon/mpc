#!/bin/sh -e
#
# This shell script tests the build of mpc with various compile-time
# options.
#
# Author: Max Kellermann <max@duempel.org>

MAKE="make -j4"
PREFIX=/tmp/mpc

rm -rf $PREFIX

export CFLAGS="-Os"

test -x configure || NOCONFIGURE=1 ./autogen.sh

# all features on
./configure --prefix=$PREFIX/full --enable-debug --enable-werror \
        --enable-iconv
$MAKE clean
$MAKE install

# all features off
./configure --prefix=$PREFIX/full --enable-debug --enable-werror \
        --disable-iconv
$MAKE clean
$MAKE install

# dietlibc
PKG_CONFIG_PATH=/tmp/libmpdclient/diet-notcp/lib/pkgconfig \
	CC="diet -Os gcc -nostdinc" ./configure --prefix=$PREFIX/diet \
	--disable-debug --enable-werror
$MAKE clean
$MAKE install
