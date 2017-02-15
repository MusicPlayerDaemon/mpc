#!/bin/sh
set -e
cd `dirname $1`
shift
d=`pwd`
name=`basename $d`
cd ../..
output="output/$name"
rm -rf "$output"
exec meson . "$output" --cross-file "$d/cross-file.txt" "$@"
