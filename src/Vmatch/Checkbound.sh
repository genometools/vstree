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

VMATCH="time ./vmatch.x"

if test $# -ne 4
then
  echo "Usage: $0 <minlen> <lower> <upper> <indexname>"
  exit 1
fi

minlen=$1
lower=$2
upper=$3
indexname=$4

FASTBOUNDEDGAPS=off
export FASTBOUNDEDGAPS
echo "############# FASTBOUNDEDGAPS is off #################"
cmd="${VMATCH} -absolute -l ${minlen} ${lower} ${upper} ${indexname}"
${cmd} > .tmp
checkerror

grep -v '^#' .tmp | sort > tmp.vmatfind

FASTBOUNDEDGAPS=on
export FASTBOUNDEDGAPS
echo "############# FASTBOUNDEDGAPS is on #################"
cmd="${VMATCH} -absolute -l ${minlen} ${lower} ${upper} ${indexname}"
${cmd} > .tmp
checkerror

grep -v '^#' .tmp | sort > tmp.vmatbound

cmd="cmp -s tmp.vmatfind tmp.vmatbound"
${cmd}
checkerror

rm -f .tmp tmp.vmatfind tmp.vmatbound
