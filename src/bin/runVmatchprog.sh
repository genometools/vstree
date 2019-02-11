#!/bin/sh

checkerror() {
if test $? -ne 0
then
  echo "failure: ${cmd}"
  exit 1
fi
}

if test $# -lt 1
then
  echo "Usage: $0 <program to run> <options>"
  exit 1
fi

prog=$1
shift

execfile="`find . -name '*.x' | head -n 1`XXX"
if test $execfile = "XXX"
then
  cmd="${BINDIR}/${prog} $*"
else
  execfile=`echo ${execfile} | sed -e 's/XXX$//'`
  archi=`binarypath.sh ${execfile}`
  cmd="${BINDIRROOT}/${archi}/${prog} $*"
fi
${cmd}
checkerror
