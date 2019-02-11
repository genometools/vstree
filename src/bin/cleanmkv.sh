#!/bin/sh
# clean all files comprising the virtual index
SUFFIXES="ssp llv skp al1 al2 des prj lcp suf tis ois bwt bck sds iso sti sti1 cld cld1 crf cfr"

if test $# -ne 1
then
  echo "Usage: $0 indexname"
  exit 1
fi
for suf in $SUFFIXES
do
  rm -f "$1.$suf"
done
