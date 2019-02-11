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
/bin/rm -f tmp1.*
howmany=2
while test $howmany -le 8
do
  echo $howmany
  /bin/rm -f tmp1.*
  cmd="mkrandfast.sh $howmany"
  ${cmd} > tmp1
  checkerror

  cmd="./Repms.sh tmp1 3 0"
  ${cmd}
  checkerror

  cmd="./Repms.sh tmp1 3 0 palindromic"
  ${cmd}
  checkerror

  cmd="./Repms.sh tmp1 4 -1"
  ${cmd}
  checkerror

  cmd="./Repms.sh tmp1 4 1"
  ${cmd}
  checkerror

  cmd="./Repms.sh tmp1 4 -1 palindromic"
  ${cmd}
  checkerror

  cmd="./Repms.sh tmp1 4 1 palindromic"
  ${cmd}
  checkerror

  cmd="./Repms.sh tmp1 6 -2"
  ${cmd}
  checkerror

  cmd="./Repms.sh tmp1 6 -2 palindromic"
  ${cmd}
  checkerror

  cmd="./Repms.sh tmp1 6 2"
  ${cmd}
  checkerror

  cmd="./Repms.sh tmp1 6 2 palindromic"
  ${cmd}
  checkerror

  cmd="./Repms.sh tmp1 8 -3"
  ${cmd}
  checkerror

  cmd="./Repms.sh tmp1 8 -3 palindromic"
  ${cmd}
  checkerror

  cmd="./Repms.sh tmp1 8 3"
  ${cmd}
  checkerror

  cmd="./Repms.sh tmp1 8 3 palindromic"
  ${cmd}
  checkerror

  howmany=`expr $howmany + 1`
done
