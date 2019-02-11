#!/bin/sh
if test $# -ne 1
then
  echo "Usage: $0 <sequence file>"
  exit 1
fi
database=$1
mkvtreeopt="-db ${database} -indexname tmp -pl 7 -dna -skp -lcp -suf -tis"
./mkvtree.x ${mkvtreeopt}
if test $? -ne 0
then
  echo "failure: ./mkvtree.x ${mkvtreeopt}"
  exit 1
fi
./mkvtree.x -rev ${mkvtreeopt}
if test $? -ne 0
then
  echo "failure: ./mkvtree.x ${mkvtreeopt}"
  exit 1
fi
./mkcfr.x tmp
