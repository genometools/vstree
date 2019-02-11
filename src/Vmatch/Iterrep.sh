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

cmd="----------$0-------------"
cmd="oneFE.rand.x acgt 100000"
${cmd}
checkerror

number=20
plength=30
rm -f tmp1.*

while test $plength -le 31
do
  patternlist=`onePE.rand.x R100000acgt $number $plength` || exit 1
  for p in ${patternlist}
  do
    printf "$p\n"
    printf ">seq\n$p\n" > tmp1

    cmd="./Rep.sh tmp1 4 -1"
    ${cmd}
    checkerror

    cmd="./Rep.sh tmp1 4 1"
    ${cmd}
    checkerror

    cmd="./Rep.sh tmp1 4 -1 palindromic"
    ${cmd}
    checkerror

    cmd="./Rep.sh tmp1 4 1 palindromic"
    ${cmd}
    checkerror

    cmd="./Rep.sh tmp1 6 -2"
    ${cmd}
    checkerror

    cmd="./Rep.sh tmp1 6 2"
    ${cmd}
    checkerror

    cmd="./Rep.sh tmp1 6 -2 palindromic"
    ${cmd}
    checkerror

    cmd="./Rep.sh tmp1 6 2 palindromic"
    ${cmd}
    checkerror

    cmd="./Cnt.sh tmp1 2"
    ${cmd}
    checkerror

    rm -f tmp1.bwt tmp1.prj
  done
  plength=`expr $plength + 1`
done
rm -f R100000acgt
