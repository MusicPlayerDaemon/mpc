#!/bin/sh
set -e

ROOT=/usr/local/i686-w64-mingw32
export PKG_CONFIG_DIR=
export PKG_CONFIG_LIBDIR=${ROOT}/lib/pkgconfig:${ROOT}/share/pkgconfig

exec /usr/bin/pkg-config "$@"
