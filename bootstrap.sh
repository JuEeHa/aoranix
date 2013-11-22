#!/bin/sh
export AORA=$(readlink -f $(dirname $0))
export PATH="$AORA/bin:$PATH"
mkdir -p $AORA/bin $AORA/lib

cd $AORA/src/cmd/sh-util
for i in c l
do
	install $i.sh $AORA/bin/$i
done

cd ../../libaora
c *.c
l -lo $AORA/lib/libaora.a *.o
rm *.o

cd ../cmd
cp config/nix/b.h config/b.h
sed -i s,AORADIR,"\"$AORA\"", config/b.h
c b.c
l -o $AORA/bin/b b.o
rm b.o config/b.h

cd ..
b
b -i
b -c

echo 'Add `export AORA="'"$AORA"'"` to your .profile and copy/link bin/aora to your PATH'
