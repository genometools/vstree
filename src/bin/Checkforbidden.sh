#!/bin/sh
if test $# -lt 1
then
  echo "Usage: $0 <files to egrep>"
  exit 1
fi
egrep -f ${WORKVSTREE}/src/bin/Forbidden.grep $* | grep -v ':@'
grep -w 'int' $* | grep -v ':@'
