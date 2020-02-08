#!/bin/sh

# set -x

if test $# -ne 1
then
  echo "Usage: $0 <Makedef suffix>"
  exit 1
fi

destfile="Makedef-$1"

if test -f ${destfile}
then
  echo "# link Makedef -> ${destfile}"
else
  echo "${destfile} does not exist"
  exit 1
fi
cmd="ln -sf ${destfile} Makedef"
${cmd}

TMPFILE=makefile.$$

cat << ENDOFMKFILE > ${TMPFILE}
include ./Makedef
cflagsstring:
	@echo \${DEFINECFLAGS}
ENDOFMKFILE

cmd="make -f ${TMPFILE}"
CFLAGS=`${cmd}`

cmd="vmrelease.sh ${CFLAGS} ${CPPFLAGS}"
${cmd} > ${WORKVSTREESRC}/include/vmrelease.h

rm -f ${TMPFILE}
