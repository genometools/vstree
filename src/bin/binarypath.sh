#!/bin/sh

if test $# -ne 1
then
  echo "$0: <binary file>"
  exit 1
fi

inputfile=$1

if test ${CONFIGGUESS} = "sparc-sun-solaris"
then
  sparcidentify.sh ${inputfile}
  exitcode=$?
  if test $exitcode -eq 1
  then 
    exit 1
  else
    if test $exitcode -eq 2
    then
      echo "${CONFIGGUESS}"
    else
      echo "${CONFIGGUESS}/sparcv9"
    fi
  fi
else
  echo "${CONFIGGUESS}"
fi
