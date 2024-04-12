#!/bin/sh
USAGE="Usage: $0 <cflagsstring>"
COPYRIGHT="${WORKVSTREESRC}/Copyright"
PROGRAM=VSTREE

if test $# -eq 0
then
  echo ${USAGE}
  exit 1
fi

if test -f ${COPYRIGHT}
then
  cat ${COPYRIGHT}
else
  echo "$0: cannot open file \"${COPYRIGHT}\""
  exit 1
fi

cat << ENDOFRELEASEPRE
#ifndef ${PROGRAM}RELEASE_H
#define ${PROGRAM}RELEASE_H
ENDOFRELEASEPRE

compiledate=`date +%Y-%m-%d`
echo "#define ${PROGRAM}COMPILEDATE "\"${compiledate}\"""

shift # get rid of first argument
echo "#define ${PROGRAM}CFLAGS \"(reproducible build)\""

if [ -n "${SOURCE_DATE_EPOCH}" ]
then
  HOSTNAME="(reproducible build)"
else
  HOSTNAME="`hostname`"
fi

cat << ENDOFRELEASEPOST
#define ${PROGRAM}RELEASEDATE "2007-08-27"
#define ${PROGRAM}VERSION "`cat ${WORKVSTREESRC}/VERSION`"
#define ${PROGRAM}COMPILEHOST "${HOSTNAME}"
#endif
ENDOFRELEASEPOST
