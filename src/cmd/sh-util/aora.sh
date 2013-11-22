#!/bin/sh
if test ! -n "$AORA"
then
	echo 'Set $AORA'
	exit 1
fi

export PATH="$AORA/bin:$PATH"

if test $# -eq 0
then
	exec "$SHELL"
else
	exec "$@"
fi
