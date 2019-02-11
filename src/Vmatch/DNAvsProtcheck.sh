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

VMATCH="./vmatch.x"
AT=../testdata/at1MB
SW=../testdata/swiss1MB

cmd="cleanpp.sh"
${cmd}
checkerror

cmd="mkvtree.sh -protein -allout -indexname swiss1MB -db ${SW} -pl"
${cmd}
checkerror

cmd="mkvtree.sh -smap TransProt11 -allout -indexname swiss1MBProt11 -db ${SW} -pl"
${cmd}
checkerror

for indexname in swiss1MB swiss1MBProt11
do
  for drop in "-exdrop" "-hxdrop"
  do
    cmd="./vmatch.x -l 50 ${drop} 2 -q ${AT} -s -dnavsprot 1 ${indexname}"
    ${cmd}
    checkerror
    cmd="${VMATCH} -best 100 -l 50 ${drop} 2 -q ${AT} -dnavsprot 1 ${indexname}"
    ${cmd} > tmp1.match
    checkerror
    cmd="./vmatchselect.x -sort ea -s tmp1.match"
    ${cmd} > .shit
    checkerror
    grep -v '^#' .shit > tmp2.align
    cmd="${VMATCH} -best 100 -l 50 ${drop} 2 -q ${AT} -dnavsprot 1 -sort ea -s ${indexname}"
    ${cmd} > .shit
    checkerror
    grep -v '^#' .shit > tmp1.align
    cmd="diff -w tmp1.align tmp2.align"
    ${cmd}
    checkerror
  done
done
rm -f tmp1.align tmp2.align tmp1.match .shit
