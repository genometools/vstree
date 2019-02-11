#!/bin/sh

checkerror() {
if test $? -ne 0
then
  echo "failure: ${cmd}"
  exit 1
else
  echo "okay: ${cmd}"
fi
}

if test $# -eq 0
then
  echo "Usage: $0 <vmatch arguments>"
  exit 1
fi
TMPFILE=/tmp/Checkonline.sh-0.$$
TMPFILE1=/tmp/Checkonline.sh-1.$$
TMPFILE2=/tmp/Checkonline.sh-2.$$
#cmd="valgrind.sh --log-file=valfile ./vmatch.x $*"
cmd="./vmatch.x $*"
${cmd} > ${TMPFILE}
checkerror
grep -v '^#' ${TMPFILE} | sort > ${TMPFILE1}
cmd="./vmatch.x ${extratrans} -online $*"
${cmd} > ${TMPFILE}
checkerror
grep -v '^#' ${TMPFILE} | sort > ${TMPFILE2}
cmd="diff ${TMPFILE1} ${TMPFILE2}"
${cmd}
checkerror
rm -f ${TMPFILE} ${TMPFILE1} ${TMPFILE2}
