#!/bin/sh
set -e

ROOT=/usr/local/x86_64-w64-mingw32
export PKG_CONFIG_DIR=
export PKG_CONFIG_LIBDIR=${ROOT}/lib/pkgconfig:${ROOT}/share/pkgconfig

exec /usr/bin/pkg-config "$@"
