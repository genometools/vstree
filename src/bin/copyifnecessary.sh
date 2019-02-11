#!/bin/sh

checkerror() {
if test $? -ne 0
then
  echo "failure: ${cmd}"
  exit 1
else
  echo "exec ${cmd}"
fi
}

if test $# -ne 2
then
  echo "Usage: $0 <source> <dest>"
  exit 1
fi

sourcefile=$1
destfile=$2

if test ! -f ${sourcefile}
then
  echo "$0: file ${sourcefile} does not exist"
  exit 1
fi

if test -f ${destfile}
then
  cmd="cmp -s ${sourcefile} ${destfile}"
  ${cmd}
  if test $? -eq 0
  then
    copy=0
  else
    copy=1
  fi
else
  copy=1
fi

if test ${copy} -eq 1
then
  cmd="cp -f ${sourcefile} ${destfile}"
  ${cmd}
  checkerror
fi
