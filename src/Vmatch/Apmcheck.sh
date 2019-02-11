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

if test $# -ne 3
then
  echo "Usage: $0 <edist> <pattern> <indexname>"
  exit 1
fi

edist=$1
pattern=$2
indexname=$3

OUTONLINE=tmp-online.match
OUTOFFLINE=tmp-offline.match
VMATCH="./vmatch.x"

printf ">\n%s\n" $pattern > Pattern.tmp

VMOPTIONS="-complete -e ${edist} -q Pattern.tmp ${indexname}"

cmd="${VMATCH} ${VMOPTIONS}"
${cmd} > ${OUTOFFLINE}
checkerror

cmd="${VMATCH} -online ${VMOPTIONS}"
${cmd} > ${OUTONLINE}
checkerror

cmd="./vmatchselect.x ${OUTONLINE}"
${cmd} > .tmp
checkerror

grep -v '^#' .tmp > ${OUTONLINE}.vms

cmd="./vmatchselect.x ${OUTOFFLINE}"
${cmd} > .tmp
checkerror

grep -v '^#' .tmp > ${OUTOFFLINE}.vms

rm -f .tmp

cmd="diff ${OUTONLINE}.vms ${OUTOFFLINE}.vms"
${cmd}
checkerror

#cmd="apmcontain.x ${OUTONLINE} ${OUTOFFLINE}"
#${cmd}
#checkerror

echo "number of matches is `wc -l ${OUTONLINE}.vms`"

rm -f ${OUTONLINE}.vms ${OUTOFFLINE}.vms ${OUTOFFLINE} ${OUTONLINE}
