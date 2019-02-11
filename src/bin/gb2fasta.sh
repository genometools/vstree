#!/bin/sh
if test $# -eq 0
then
  echo "Usage: $0 filenames"
  exit 1
fi
${AWK} -f ${DIRVSTREE}/src/bin/gb2fasta.awk $*
