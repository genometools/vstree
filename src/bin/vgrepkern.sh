#!/bin/sh
if test $# -eq 0
then
  echo "Usage: $0 [args of grep] pattern"
  exit 1
fi
${DIRVSTREE}/src/bin/vstreefiles.sh src | xargs egrep $* -d skip
