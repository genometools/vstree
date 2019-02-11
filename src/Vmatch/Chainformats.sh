#!/bin/sh

checkerror() {
if test $? -ne 0
then
  echo "failure: ${cmd}"
  exit 1
fi
}

if test $# -eq 0
then
  echo "Usage: $0 <matchfile>"
  exit 1
fi

# VALGRIND=valgrind.sh 

matchfile=$1
chainopts="-local 10b"

cmd="${VALGRIND} ./chain2dim.x ${chainopts} -silent ${matchfile}"
${cmd} > .tmp1
checkerror

cmd="Vmatchtrans.pl open ${matchfile}"
${cmd} > .shit
checkerror

cmd="${VALGRIND} ./chain2dim.x ${chainopts} -silent .shit"
${cmd} > .tmp2
checkerror

cmd="cmp -s .tmp1 .tmp2"
${cmd}
checkerror

cmd="./vmatchselect.x -sort sa ${matchfile}"
${cmd} > .shit
checkerror

if test -f ./chain2dim.dbg.x
then
  cmd="${VALGRIND} ./chain2dim.dbg.x ${chainopts} -silent .shit"
  ${cmd} > /dev/null
  checkerror
fi

cmd="${VALGRIND} ./chain2dim.x ${chainopts} -silent .shit"
${cmd} > /dev/null
checkerror

rm -f .tmp[12] .shit
