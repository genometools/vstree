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

QUERYSPEEDUP=1
export QUERYSPEEDUP

cmd="Checksout.sh"
${cmd} > .shit
checkerror

grep -v '^#' .shit > shit1
QUERYSPEEDUP=2
export QUERYSPEEDUP

cmd="Checksout.sh"
${cmd} > .shit
checkerror

grep -v '^#' > shit2

cmd="diff shit1 shit2"
${cmd}
checkerror
