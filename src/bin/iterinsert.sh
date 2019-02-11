#!/bin/sh

TMPFILE=`mktemp /tmp/insertconfig.XXXXXX`
for filename in `ls *.c`
do
  cat ${filename} | insertconfig.pl > ${TMPFILE}
  mv ${TMPFILE} ${filename}
done
