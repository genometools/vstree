#!/bin/sh
if test $# -eq 1
then
  inputfile=$1
  subdir=
else
  if test $# -eq 2
  then
    inputfile=$1
    subdir=$2
  else
    echo "Usage: $0 <inputfile> [subdir]"
    exit 1
  fi
fi
FINDCALL="find ${DIRVSTREE}/src/vstree/${subdir} -name ${inputfile}"
numofpaths=`${FINDCALL} | grep -v SELECT | wc -l`
if test ${numofpaths} -eq 0
then
  echo "$0: cannot find path for \"${inputfile}\""
  exit 1
else
  if test ${numofpaths} -gt 1
  then
    echo "$0: more than one path for \"${inputfile}\":"
    ${FINDCALL}
    exit 1
  else
    ${FINDCALL} | grep -v SELECT
  fi
fi
