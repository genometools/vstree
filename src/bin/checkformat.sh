#!/bin/sh
if test $# -ne 1
then
  echo "Usage: $0 (int|long)"
  exit 1
fi
case "$1" in
  "int") formatpat="%[^hl][du]";;
  "long") formatpat="";;
esac
FILELIST="`ls include/*.[ch] kurtz/*.[ch] kurtz/*.gen Mkvtree/*.[ch] Vmatch/*.[ch] Vmatch/*.gen`"
echo "####### search ${formatpat} ####"
grep ${formatpat} ${FILELIST}
echo "####### search for int ####"
grep -wl int ${FILELIST}
