#!/bin/sh
if test $# -eq 0
then
  echo "Usage: $0 [args of grep] pattern"
  exit 1
fi
egrep $* -d skip `${DIRVSTREE}/src/vstree/src/bin/vstreecsrc.sh`
