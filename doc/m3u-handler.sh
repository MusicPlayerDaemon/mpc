#!/bin/sh
mpc clear
cat $1 | mpc add
mpc play
