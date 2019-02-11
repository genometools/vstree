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

echo "------------ $0 -----------------"
echo $*
cmd="./vmatch.x $*"
${cmd} > match.tmp
checkerror

head -n 1 match.tmp > shit2

if test -f ./readm.x
then
  cmd="./readm.x match.tmp $*"
  ${cmd} > .shit
  checkerror
  cat .shit | grep -v '^#' >> shit2 
  cmd="diff -q -w match.tmp shit2"
  ${cmd}
  checkerror
fi

echo "okay: $0"
