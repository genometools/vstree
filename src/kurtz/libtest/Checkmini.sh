#!/bin/sh

checkerror() {
if test $? -ne 0
then
  echo "failure: ${cmd}"
  exit 1
fi
}

if test $# -ne 2
then
  echo "Usage: $0 <leastlength> <index>"
  exit 1
fi

leastlength=$1
indexname=$2

cmd="runVmatchprog.sh vmatch -absolute -noevalue -nodist -noidentity -noscore -l ${leastlength} ${indexname}"
${cmd} > tmp2
checkerror

grep -v '^#' tmp2 | ${AWK} '{print $1 " " $2 " " $5}' > tmp1

cmd="vmatch-mini.x ${leastlength} ${indexname}"
${cmd} > tmp3
checkerror

grep -v '^#' tmp3 > tmp2

cmd="cmp -s tmp1 tmp2"
${cmd}
checkerror

cmd="strm-selfcmp.x ${leastlength} ${indexname}"
${cmd} > tmp3
checkerror

cmd="cmp -s tmp2 tmp3"
${cmd}
checkerror
