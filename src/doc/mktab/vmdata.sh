#!/bin/sh
if test $# -ne 2
then
  echo "Usage: $0 <program> <resultfile>"
  exit 1
fi

${AWK} -f vmData.awk $2 | sed -f vmData.sed | ${AWK} -vprog=$1 -f vmcompose.awk | fgrep -f select.fgrep | grep -v 'FILE' | sort
