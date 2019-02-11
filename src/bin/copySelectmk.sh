#!/bin/sh
if test $# -ne 1
then
  echo "Usage: $0 directory"
  exit 1
fi
echo ${shareddef}

case "$OSTYPE" in
  "sol2")  sed -f Shared.sed Vmatch/SELECT/makefile > ${OUTMAKE};;
  "linux") cp Vmatch/SELECT/makefile ${OUTMAKE};;
  *)
esac
