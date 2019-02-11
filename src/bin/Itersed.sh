#!/bin/sh
if test $# -ne 2
then
  echo "Usage: $0 <directory> <sedscriptfile>"
  exit 1
fi
directory=$1
sedscriptfile=$2
case "${directory}" in
  "kurtz")   FILELIST="`ls ${directory}/*.[ch] ${directory}/*.gen`";;
  "Vmatch")  FILELIST="`ls ${directory}/*.[ch] ${directory}/*.gen`";;
  *)         FILELIST="`ls ${directory}/*.[ch]`";;
esac

for filename in ${FILELIST}
do
  echo "${filename}"
  mv ${filename} .tmp
  sed -f ${sedscriptfile} .tmp > ${filename}
done
rm -f .tmp
