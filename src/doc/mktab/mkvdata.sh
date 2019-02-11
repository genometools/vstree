#!/bin/sh
if test $# -ne 2
then
  echo "Usage: $0 <program> <resultfile>"
  exit 1
fi

${AWK} -f mkvData.awk $2 | sed -f mkvData.sed | ${AWK} -vprog=$1 -f mkvcompose.awk | fgrep -f select.fgrep | grep -v 'FILE' | sort
