#!/bin/sh
if test $# -eq 0
then
  echo "Usage: $0 <pattern for egre>"
  exit 1
fi
egrep $* mkvcompose.awk data2tex.awk mkvData.sed mkvData.awk mkvdata.sh select.fgrep
