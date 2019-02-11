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

inputfile=${ECOLI1}

cmd="mkvtree.sh -indexname boundidx -db ${inputfile} -allout -dna -v -pl"
${cmd}
checkerror

for minlen in 15 14 13 12 11 10 9 8 7 6 5
do
  cmd="./Checkbound.sh ${minlen} 1000 1005  boundidx"
  ${cmd}
  checkerror
done
