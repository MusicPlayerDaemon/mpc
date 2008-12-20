#!/bin/sh
test $# -ne 1 && echo "$0 takes 1 argument" && exit 1
test ! -e "$1" && echo "Argument ($1) needs to be a file" && exit 2
mpc add < "$1"
