#!/bin/sh
checkerror() {
  if test $? -ne 0
  then
    echo "failure: ${cmd}"
    exit 1
  fi
}

if test $# -ne 1
then
  echo "Usage: $0 <length>"
  exit 1
fi

cmd="head -n $1 ${ECOLI1}"
${cmd} > ecoli1.fna
checkerror

cmd="mkvtree.sh -db ecoli1.fna -pl -dna -allout"
${cmd}
checkerror

cmd="head -n $1 ${ECOLI2}"
${cmd} > ecoli2.fna
checkerror

cmd="./vmatch.x -l 20 -q ecoli2.fna ecoli1.fna"
${cmd}
checkerror
