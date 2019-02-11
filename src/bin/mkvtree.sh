#!/bin/sh

checkerror() {
if test $? -ne 0
then
  echo "failure: ${cmd}"
  exit 1
fi
}

cmd="mkvtree $*"
${cmd}
checkerror
