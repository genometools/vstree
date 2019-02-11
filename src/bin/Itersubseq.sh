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

if test $# -ne 1 
then
  echo "Usage: $0 <indexname>"
  exit 1
fi
indexname=$1
numofsequences=`grep 'numofsequences=' ${indexname}.prj | sed -e 's/numofsequences=//'`
seqnum=0
while test ${seqnum} -lt ${numofsequences}
do
  cmd="../Mkvtree/vsubseqselect.x -seq 20 ${seqnum} 0 ${indexname}"
  ${cmd}
  checkerror
  seqnum=`expr ${seqnum} + 1`
done
