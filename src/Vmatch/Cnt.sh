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
  echo "$0 <filename> <length>"
  exit 1
fi

if test -f "`basename $1`.bwt"
then
  echo "`basename $1.bwt` exists"
else
  mkvtreeall.sh -db $1 -pl 7 -dna
  if test $? -ne 0
  then
    echo "failure: mkvtreeall.sh"
    exit 1
  fi
fi

indexname=`basename $1`
cmd="./vmatch.x -d -i -l $2 $indexname"
${cmd} > .shit
checkerror

cat .shit | grep -v '^# args=' > shit1
cmd="repfind.x -allmax -i -l $2 $1"
${cmd} > .shit
checkerror

cat .shit | grep -v $1 | grep -v '^# allF' | sed -e 's/F //' > shit2
cmd="cmp -s shit1 shit2"
${cmd}
checkerror

echo "okay: $0"
