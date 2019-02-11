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
if test $# -eq 0
then
  echo "Usage: $0 <args to vmatch>"
  exit 1
fi

echo "$*"
cmd="./vmatch.x $*"
${cmd} > .shit
checkerror

cat .shit | sort > shit1
cmd="./vmatch.x -online $*"
${cmd} > .shit
checkerror

cat .shit | sed -e 's/-online //' | sort > shit2
cmd="cmp -s shit1 shit2"
${cmd}
checkerror

echo "okay: $0"
