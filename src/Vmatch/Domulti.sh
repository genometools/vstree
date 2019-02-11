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

if test $# -ne 0
then
  echo "Usage: $0"
  exit 1
fi
outopt="-bck -dna -tis -bwt -suf -lcp"

cmd="mkvtree.sh -v -indexname atall ${outopt} -pl -db ${IOWADATA}/at0[0-9]"
${cmd}
checkerror

echo "okay: $0"
