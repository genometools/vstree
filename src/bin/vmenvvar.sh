#!/bin/sh

TMPFILE=/tmp/vmenvvar.$$

cat << HEREDOC > $TMPFILE
QUERYSPEEDUP
MKVTREESMAPDIR
SCOREMATRIXDIR
VMATCHSHOWOPTIONINLATEX
VMATCHSHOWEXCLUDETAB
VMATCHSHOWTIMESPACE
VMATCHSPACEMONITOR
VMATCHPAGESIZELEVEL
DEBUGLEVEL 
DEBUGWHERE 
FASTBOUNDEDGAPS 
VMATCHCOMMENTTOSTDOUT
VMATCHRELATIVEINDEXPATH
LD_LIBRARY_PATH
HEREDOC

for envvar in `cat ${TMPFILE}`
do
  setval="`env | grep ${envvar}`"
  if test $? -eq 0
  then
    echo "${setval}"
  else
    echo "${envvar} is undefined"
  fi
done

rm -f ${TMPFILE}
