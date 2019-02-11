#!/bin/sh
if test $# -ne 1
then
  echo "Usage: $0 <matchfile>"
  exit 1
fi

TMPFILE=/tmp/deleteepath.$$
#sed -e 's/\(^# args=[^\/]*\)\/.*\//\1/' $1 > ${TMPFILE}
sed -e 's/\/.*\///' $1 > ${TMPFILE}
mv ${TMPFILE} $1
