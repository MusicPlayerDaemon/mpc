#!/bin/sh
#
echo "Generating build information using aclocal, automake and autoconf"
echo "This may take a while ..."

# Touch the timestamps on all the files since CVS messes them up
touch Makefile.am configure.in

rm -f configure
rm -f config.cache
rm -f config.status
rm -rf autom4te*.cache

# Regenerate configuration files
libtoolize -f -c

aclocal
automake --foreign --add-missing -c
autoconf

# Run configure for this platform
./configure $*
#echo "Now you are ready to run ./configure"

