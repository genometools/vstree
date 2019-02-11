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

echo "----------$0-------------"

cmd="mkvtreeall.sh -db Testdir/ACAG.seq -pl -smap Testdir/maskDNA"
${cmd}
checkerror

cmd="./vmatch.x -l 4 -d -q Testdir/ucquery.seq ACAG.seq"
${cmd} > .shit
checkerror

cat .shit | grep -v '^#' > shit1
cmd="cmp -s shit1 Testdir/Eivind.res"
${cmd}
checkerror

cmd="./vmatch.x -l 4 -d -q Testdir/lcquery.seq ACAG.seq"
${cmd} > .shit
checkerror

matches=`cat .shit | wc -l | sed -e 's/ //g'`
if test $matches -ne 1
then
  echo "found some matches: that is not correct"
  exit 1
fi

echo "okay: $0"
