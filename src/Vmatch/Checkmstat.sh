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
echo "------------ $0 -----------------"
seqs=`ls Testdir/BS1000.seq`
patternfile=Tmp.pattern
for plainfile in ${seqs}
do
  headlen=10
  while test ${headlen} -le 100
  do
    echo "mstat.sh ${headlen} ${plainfile}"
    cmd="head -c ${headlen} ${plainfile}"
    ${cmd} > ${patternfile}
    checkerror
    cmd="mstats.sh ${patternfile} ${plainfile}"
    ${cmd}
    checkerror
    headlen=`expr ${headlen} + 10`
  done
done

echo "okay: $0"
