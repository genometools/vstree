#!/bin/sh

if test $# -lt 1
then
  echo "Usage: $0 <file1> .. <filen>"
  exit 1
fi

FIRSTCHOICE=/projects/gi/share/tmpindex
SECONDCHOICE=/local/kurtz/vmindex/32bit
DEFAULTCHOICE=/tmp/vmindex

if test -d ${FIRSTCHOICE}
then
  indexdir=${FIRSTCHOICE}
else
  if test -d ${SECONDCHOICE}
  then
    indexdir=${SECONDCHOICE}
  else
    indexdir=${DEFAULTCHOICE}
  fi
fi

if test ! -d ${indexdir}
then
  mkdir -p ${indexdir}
fi

firstfile=`echo $1 | sed -e 's/^-//'`
indexname="${indexdir}/`basename ${firstfile}`"
shift

for filename in `echo $* | tr ' ' '\n'`
do
  filename=`echo $filename | sed -e 's/^-//'`
  indexname="${indexname}-`basename ${filename}`"
done

echo "${indexname}"
