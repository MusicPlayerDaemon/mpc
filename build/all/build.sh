#!/bin/sh
set -e
cd `dirname $0`/../..
for i in debug release mini win32 win64; do
    ninja -C "output/$i"
done
