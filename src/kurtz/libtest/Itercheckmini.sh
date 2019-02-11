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

PROTFILES="${SWS} ${SWK} ${XCAMP} ${ORF} ${SWK2} ${SWK1} ${SW}"

for filename in ${PROTFILES}
do
  cmd="mkvtree -indexname miniindex -db ${filename} -protein -pl -allout"
  ${cmd}
  checkerror
  leastlength=10
  while test $leastlength -le 20
  do
    cmd="Checkmini.sh ${leastlength} miniindex"
    ${cmd}
    checkerror
    leastlength=`expr ${leastlength} + 1`
  done
done

DNAFILES="${AT} ${ECOLI1} ${MGEN} ${MPNEU} ${U8}"

for filename in ${DNAFILES}
do
  cmd="mkvtree -indexname miniindex -db ${filename} -dna -pl -allout"
  ${cmd}
  checkerror
  leastlength=14
  while test $leastlength -le 25
  do
    cmd="Checkmini.sh ${leastlength} miniindex"
    ${cmd}
    checkerror
    leastlength=`expr ${leastlength} + 1`
  done
done


rm -f tmp1 tmp2
