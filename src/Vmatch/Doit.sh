#!/bin/sh
if test $# -lt 2
then
  echo "Usage: $0 <indexname> <args to mkvtree>"
  exit 1
fi
indexname=$1
shift
outopt="-bck -ois -tis -bwt -suf -lcp"

cmd="mkvtree.sh -v ${outopt} -dna -indexname $indexname -pl -db $*"
${cmd}
checkerror

echo "okay: $0"
