#!/bin/sh

flags="-I$AORA/include"
test ! -n "$cc" && cc=cc

for i in "$@"
do
	case "$i" in
		*.c)
			$cc -c $flags -o ${i%.c}.o $i || exit 1
			;;
		*)
			flags="$flags $i"
	esac
done
