#!/bin/sh
if test $# -ne 1
then
  echo "$0 <texfile>"
  exit 1
fi
texfile=$1
grep '^\\Showheader' ${texfile} | sed -e 's/\\Show[a-z]*{//' -e 's/}.*/.h/' | sort -u
grep '^\\Showcode' ${texfile} | sed -e 's/\\Show[a-z]*{//' -e 's/}.*/.c/' | sort -u
