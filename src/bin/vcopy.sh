#!/bin/sh
if test $# -lt 1
then
  echo "Usage: $0 <inputfiles>"
  exit 1
fi
for filename in $*
do
  if test -e ${filename}
  then
    echo "file ${filename} already exists"
  else
    cp -i `vpathkern.sh ${filename}` .
  fi
done
