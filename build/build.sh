#!/bin/sh
set -e
cd `dirname $0`/..
for i in output/*/build.ninja; do
    ninja -C `dirname "$i"` "$@"
done
