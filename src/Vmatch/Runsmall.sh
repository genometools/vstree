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
if test $# -ne 1
then
  echo "Usage: $0 <length>"
  exit 1
fi
if test $1 -ne 0
then
  echo ">" > Tmp3
  cmd="grep -v '^>' ${ECOLI}"
  ${cmd} > .tmp
  checkerror
  cmd="head -c $1 .tmp"
  ${cmd} >> Tmp3
  checkerror
  echo "" >> Tmp3
fi
cmd="mkvtreeall.sh -db Tmp3 -pl 1 -dna"
${cmd}
checkerror

if test $1 -ne 0
then
  echo ">" > Tmp4
  cmd="tail -c $1 ${ECOLI}"
  ${cmd} >> Tmp4
  checkerror
fi

cmd="./vmatch.x -qspeedup 4 -l 4 -q Tmp4 Tmp3"
${cmd}
checkerror
