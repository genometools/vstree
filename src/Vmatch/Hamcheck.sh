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
  echo "Usage: $0 <hdist> <pattern> <indexname>"
  exit 1
fi

edist=$1
pattern=$2
indexname=$3

OUTONLINE=tmp-online.match
OUTOFFLINE=tmp-offline.match
VMATCH="./vmatch.x"

printf ">\n%s\n" $pattern > Pattern.tmp

VMOPTIONS="-complete -h ${edist} -q Pattern.tmp ${indexname}"

cmd="${VMATCH} ${VMOPTIONS}"
${cmd} > ${OUTOFFLINE}
checkerror

cmd="${VMATCH} -online ${VMOPTIONS}"
${cmd} > ${OUTONLINE}
checkerror

grep -v '^#' ${OUTONLINE} | sort > ${OUTONLINE}.sort
grep -v '^#' ${OUTOFFLINE} | sort > ${OUTOFFLINE}.sort

rm -f .tmp

cmd="diff ${OUTONLINE}.sort ${OUTOFFLINE}.sort"
${cmd}
checkerror

echo "number of matches is `wc -l ${OUTONLINE}.sort`"

rm -f ${OUTONLINE}.sort ${OUTOFFLINE}.sort ${OUTOFFLINE} ${OUTONLINE}
