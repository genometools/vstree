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
  echo "Usage: $0 <maxdepth> <inputfile>"
  exit 1
fi

maxdepth=$1
inputfile=$2

cmd="./mkvtree.x -db ${inputfile} -dna -suf -pl -indexname md-index -maxdepth ${maxdepth}"
${cmd}
checkerror

cmd="./mkvtree.x -db ${inputfile} -dna -suf -pl -indexname realindex"
${cmd}
checkerror

cmd="cmp -s realindex.suf md-index.suf"
${cmd}
checkerror
