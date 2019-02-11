#!/bin/sh
AT=../testdata/at1MB
U8=../testdata/U89959.fna
./Doit.sh 7 u8 ${U8}
if test $? -ne 0
then
  echo "failure: Doit.sh 7 u8 ${U8}"
  exit 1
fi
./Doit.sh 7 at1MB ${AT}
if test $? -ne 0
then
  echo "failure: Doit.sh 7 at1MB ${AT}"
  exit 1
fi
