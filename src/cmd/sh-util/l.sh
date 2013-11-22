#!/bin/sh

test ! -n "$cc" && cc=cc
lflags="-L$AORA/lib"

ofiles=""
out="a.out"
oflagmode=0
makelib=0
for i in "$@"
do
	if test $oflagmode = 1
	then
		out="$i"
		oflagmode=0
	fi
	
	case "$i" in
		*.o)
			ofiles="$ofiles $i"
			;;
		-L*)
			lflags="$lflags $i"
			;;
		-*)
			for j in $(getopt lo "$i")
			do
				case "$j" in
					-l)
						makelib=1
						;;
					-o)
						oflagmode=1
						;;
				esac
			done
	esac
done


if test $oflagmode = 1
then
	echo "$(basename "$0"): missing filename for '-o'" 1>&2
	exit 1
fi

if test $makelib = 0
then
	libs=""
	for i in $ofiles
	do
		libs="$libs $(nm -B $i | cut -d ' ' -f 3 | grep ^__autolib | sed s/^__autolib_/-l/ | tr '
' ' ')"
	done
	libs="$libs -laora"
	$cc -o "$out" $lflags $ofiles $libs || exit 1
else
	ar r "$out" $ofiles || exit 1
fi
