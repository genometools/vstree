#!/bin/sh

checkerror() {
if test $? -ne 0
then
  echo "failure: ${cmd}"
  exit 1
fi
}

MKVTREEOPT="-allout $*"
cmd="mkvtree.sh ${MKVTREEOPT}"
${cmd}
checkerror

indexname="`basename $2`"

if test -f ./mksti.x
then
  for prog in mksti mkiso mklsf mkcld
  do
    cmd="../Mkvtree/${prog}.x ${indexname}"
    ${cmd} > .shit
    checkerror
  done
fi
