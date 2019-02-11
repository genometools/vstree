#!/bin/sh

set -e

if test $# -ne 2
then
  echo "Usage: $0 <32|64> <versionnumber>"
  exit 1
fi
wordsize=$1
version=$2

HOMANNDIR=lib-homann

# possibly add --enable-bookkeeping for bookkeeping of space blocks.

cd ${HOMANNDIR}
gzpath="libautompssm-${version}.tar.gz"
#replace the following by a portable call to tar and gzip
tar xvzf $gzpath
subdir=`basename ${gzpath} .tar.gz`

# required for library installation path
PATHEND=`grep 'PATHEND=' ${WORKVSTREESRC}/Makedef | sed -e 's/PATHEND=//'`


OPTIONS="--prefix=${WORKVSTREESRC}
         --libdir=${WORKVSTREESRC}/lib/${CONFIGGUESS}/${PATHEND}
         --with-libkurtz-prefix=${WORKVSTREESRC}
         --with-libkurtz-basename=kurtz-basic
         --disable-matcher-myers"

if test $wordsize -eq 64
then
  OPTIONS="${OPTIONS} CFLAGS=-m64"
else
  OPTIONS="${OPTIONS} CFLAGS=-m32"
fi

# Non debug
BUILD=libautompssm.build
mkdir ${BUILD}
cd ${BUILD}
../${subdir}/configure ${OPTIONS}
make
make install-exec
cp libautomata/libautomata.h ${WORKVSTREESRC}/include
cp libpssm/libpssm.h ${WORKVSTREESRC}/include

# Debug
cd ..
BUILDDBG=libautompssm_debug.build
mkdir ${BUILDDBG}
cd ${BUILDDBG}
../${subdir}/configure ${OPTIONS} --enable-debug
make
make install-exec

# Return
cd ../
rm -rf ${subdir} ${BUILD} ${BUILDDBG}
cd ..
rm -rf share
