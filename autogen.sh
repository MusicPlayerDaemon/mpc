#!/bin/sh
#
echo "Generating build information using aclocal, automake and autoconf"
echo "This may take a while ..."

# Regenerate configuration files
aclocal
libtoolize -f -c
autoheader
automake --foreign --add-missing -c
autoconf

# Run configure for this platform
./configure "$@"
#echo "Now you are ready to run ./configure"

