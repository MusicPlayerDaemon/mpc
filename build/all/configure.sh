#!/bin/sh
set -e
cd `dirname $0`/../..
OPTIONS="--werror"
rm -rf output
meson . output/debug $OPTIONS
meson . output/release $OPTIONS --buildtype release --unity
meson . output/mini $OPTIONS --buildtype minsize --unity -Diconv=false
for i in win32 win64; do
    "./build/$i/configure.sh" $OPTIONS
done
