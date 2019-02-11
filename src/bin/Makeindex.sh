#!/bin/sh

checkerror() {
  if test $? -ne 0
  then
    echo "failure: ${cmd}"
    exit 1
  else
    echo "okay: ${cmd}"
  fi
}

if test $# -ne 2
then
  echo "Usage: $0 <atindexname> <swindexname>"
  exit 1
fi

ATindex=$1
SWindex=$2
MKVOPTIONS="-bwt -lcp -suf -ois -tis -bck -sti1"
AT=${AT}

if test -f `basename ${ATindex}.prj`
then
  echo "${ATindex}.prj exists"
else
  cmd="mkvtree.sh -indexname ${ATindex} -db ${AT} -v -pl -dna ${MKVOPTIONS}"
  ${cmd}
  checkerror
fi

if test -f `basename ${SWindex}.prj`
then
  echo "${SWindex}.prj exists"
else
  cmd="mkvtree.sh -indexname ${SWindex} -db ${AT} -v -pl -protein ${MKVOPTIONS}"
  ${cmd}
  checkerror
fi

