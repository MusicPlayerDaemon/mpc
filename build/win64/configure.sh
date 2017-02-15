#!/bin/sh
set -e
exec `dirname $0`/../cross/configure.sh "$0" "$@"
