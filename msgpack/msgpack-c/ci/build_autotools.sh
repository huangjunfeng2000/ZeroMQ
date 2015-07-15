#!/bin/sh

./bootstrap
ret=$?
if [ $ret -ne 0 ]
then
    exit $ret
fi

if [ $1 = "cpp11" ]
then
    cpp11="-std=c++11"
else
    cpp11=""
fi

if [ $2 = "32" ]
then
    bit32="-m32"
else
    bit32=""
fi

if [ $3 = "boost" ]
then
    boost="-DMSGPACK_USE_BOOST"
else
    boost=""
fi

./configure CFLAGS="$bit32" CXXFLAGS="$bit32 $cpp11 $boost -I$4"

ret=$?
if [ $ret -ne 0 ]
then
    exit $ret
fi

make

ret=$?
if [ $ret -ne 0 ]
then
    exit $ret
fi

make check

ret=$?
if [ $ret -ne 0 ]
then
    exit $ret
fi

make install DESTDIR=`pwd`/install

ret=$?
if [ $ret -ne 0 ]
then
    exit $ret
fi

exit 0
