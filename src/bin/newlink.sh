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

if test $# -ne 1
then
  echo "Usage: $0 <machine/os specification>"
  exit 1
fi
makedeffile="Makedef-$1"

if test -f ${makedeffile}
then
  cmd="ln -sf ${makedeffile} Makedef"
  ${cmd}
  checkerror
  cmd="cd kurtz"
  ${cmd}
  checkerror
  cmd="make release"
  ${cmd}
  checkerror
  cmd="cd .."
  ${cmd}
  checkerror
fi
