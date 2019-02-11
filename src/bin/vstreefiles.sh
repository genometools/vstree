#!/bin/sh
USAGE="$0 [src]"

if test $# -eq 0
then
  STARTDIR=${DIRVSTREE}
else
  if test $# -eq 1
  then
    if test "$1" = "src"
    then
      STARTDIR=${DIRVSTREE}/src
    else
      echo ${USAGE}
      exit 1
    fi
  else
    echo ${USAGE}
    exit 1
  fi
fi

find ${STARTDIR} -type f -print | egrep -v -f ${DIRVSTREE}/src/bin/vstree.exclude
