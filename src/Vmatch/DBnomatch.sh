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

if test -f "at1MB.prj"
then
  echo "index at1MB already exists"
else
  cmd="mkvtree.sh -db ../testdata/at1MB -pl -allout -dna"
  ${cmd}
  checkerror
fi

cmd="./vmatch.x -dbnomatch 100 -l 10 -q Testdir/Q2 -v at1MB"
${cmd} > .tmp
checkerror

cat .tmp | grep -v '^#' > tmp1

cmd="./vmatch.x -dbnomatch 100 -complete -q Testdir/Q2 -v at1MB"
${cmd} > .tmp
checkerror

cat .tmp | grep -v '^#' > tmp2

cmd="cmp -s tmp1 tmp2"
${cmd}
checkerror

echo "okay: $0"
