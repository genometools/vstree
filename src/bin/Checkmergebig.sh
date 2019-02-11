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

if test $# -lt 5
then
  echo "Usage: $0 <program2check> <withminlen> <file1> <file2> ..."
  exit 1
fi

program2check=$1
withminlen=$2
shift
shift
LISTOFFILES=$*

if test ${withminlen} -eq 0
then
  minlen=""
else
  minlen=${withminlen}
fi

bigindex=bigindex
outopt="-bck -tis -suf -lcp"

cmd="mkvtree.sh -indexname ${bigindex} -db ${LISTOFFILES} -v -dna -pl -bwt ${outopt}"
${cmd}
checkerror

idxnum=0
indexlist=""
for filename in ${LISTOFFILES}
do
  cmd="mkvtree.sh -v -indexname merindex${idxnum} -db ${filename} -dna -pl ${outopt}"
  ${cmd}
  checkerror
  indexlist="${indexlist} merindex${idxnum}"
  idxnum=`expr $idxnum + 1`
done

outindex=outindex

cmd="${program2check} ${minlen} ${outindex} ${bigindex} ${indexlist}"
${cmd}
checkerror
