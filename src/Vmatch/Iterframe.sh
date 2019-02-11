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
echo "----------$0-------------"
seqs=`ls ../testdata/Grumbach[a-z]*.fna`

leastlength=25
for filename in ../testdata/Grumbach/chntxx.fna ../testdata/Grumbach/humdystrop.fna
do
  cmd="./Cmp6fr.sh ${filename} ${leastlength}"
  ${cmd}
  checkerror
  wc tmp1
done

for filename in $seqs
do
  cmd="./Check6fr.sh ${filename} ${leastlength}"
  ${cmd}
  checkerror
done
echo "Test successful at `date`"
