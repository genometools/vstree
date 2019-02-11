#!/bin/sh
checkerror() {
  if test $? -ne 0
  then
    echo "failure: ${cmd}"
    exit 1
  fi
}

if test $# -le 1
then
  echo "Usage: $0 <outfile> <list-of-cfiles>"
  exit 1
fi
OUTFILE=$1
shift
LIBCFILES=$*

cmd="which gcc"
${cmd} > /dev/null
checkerror

cmd="make -n cflagsstring"
${cmd} > /dev/null
checkerror

rm -f ${OUTFILE}
touch ${OUTFILE}
cmd="gcc -M -MG -MM -DDEBUG -DCHECK `make cflagsstring` ${LIBCFILES}"
${cmd} > tmp0.mf
checkerror

sed -e 's/\/[_A-Za-z0-9\.\-][_A-Za-z0-9\.\-]*\/vstree\/include\/[_A-Za-z0-9\.\-]*\.h//g' tmp0.mf > tmp1.mf

TMPSEDSCRIPT=/tmp/sedscript.$$
echo "s/\(^[a-zA-Z0-9_\-]*\)\.o:/\1\.dbg.o:/" > ${TMPSEDSCRIPT}
cmd="sed -f ${TMPSEDSCRIPT} tmp1.mf"
${cmd} >  tmp2.mf
checkerror

cat tmp1.mf tmp2.mf |\
    sed -e 's/\/[\/a-z][\/a-z]*zlib\.h//'\
        -e 's/\/[\/a-z][\/a-z]*zconf\.h//'\
         > ${OUTFILE}
rm -f tmp[012].mf ${TMPSEDSCRIPT}
