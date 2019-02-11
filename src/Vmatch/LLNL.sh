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

echo "$0"

query=Testdir/llnlQuery
cmd="mkvtreeall.sh -db Testdir/llnlDB -smap Testdir/TransDNA_CBNP -pl"
${cmd}
checkerror

cmd="./vmatch.x -absolute -d -p -l 20  -qnomatch 1  -q ${query} llnlDB"
${cmd} > .shit
checkerror

cat .shit | grep -v '^#' > tmp1

cmd="./vmatch.x -d -p -l 20 -dbnomatch 1  -q ${query} llnlDB"
${cmd} > .tmp
checkerror

grep -v '^#' .tmp > tmp2

cmd="cmp -s tmp1 Testdir/llnlResult1"
${cmd}
checkerror

cmd="cmp -s tmp2 Testdir/llnlResult2"
${cmd}
checkerror
