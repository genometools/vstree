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

if test $# -ne 4
then
  echo "Usage: $0 <qvalue> <edistvalue> <queryfile> <referencefile>"
  exit 1
fi

qvalue=$1
edistvalue=$2
queryfile=$3
referencefile=$4

indexname=`basename ${referencefile}`
tables="-tis -suf -bck -lcp -sti1"

if test -f "${indexname}.prj"
then
  echo "${indexname}.prj exists"
else
  cmd="mkvtree -indexname ${indexname} -db ${referencefile} ${tables} -dna -pl -v"
  ${cmd}
  checkerror
fi

for flag in checkqhit nocheckqhit nocheckleast
do
  cmd="chainqhits.x ${qvalue} ${edistvalue} ${indexname} ${queryfile} ${flag}"
  ${cmd}
  checkerror
done
