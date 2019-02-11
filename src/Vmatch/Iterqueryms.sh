#!/bin/sh

checkerror() {
  if test $? -ne 0
  then
    echo "failure: ${cmd}"
    exit 1
  fi
}

echo "----------$0-------------"
/bin/rm -f tmp1.*
howmany=2
while test $howmany -le 7
do
  echo $howmany
  /bin/rm -f tmp1.*
  mkrandfast.sh $howmany > .tmp
  head -n $howmany .tmp > tmp1
  mkrandfast.sh $howmany > .tmp
  tail -n $howmany .tmp > tmp2
  rm -f .tmp

  cmd="./Queryms.sh tmp1 tmp2 3 0"
  ${cmd}
  checkerror

  cmd="./Queryms.sh tmp1 tmp2 3 0 palindromic"
  ${cmd}
  checkerror

  cmd="./Queryms.sh tmp1 tmp2 4 -1"
  ${cmd}
  checkerror

  cmd="./Queryms.sh tmp1 tmp2 4 -1 palindromic"
  ${cmd}
  checkerror

  cmd="./Queryms.sh tmp1 tmp2 4 1"
  ${cmd}
  checkerror

  cmd="./Queryms.sh tmp1 tmp2 4 1 palindromic"
  ${cmd}
  checkerror

  cmd="./Queryms.sh tmp1 tmp2 6 -2"
  ${cmd}
  checkerror

  cmd="./Queryms.sh tmp1 tmp2 6 -2 palindromic"
  ${cmd}
  checkerror

  cmd="./Queryms.sh tmp1 tmp2 6 2"
  ${cmd}
  checkerror

  cmd="./Queryms.sh tmp1 tmp2 6 2 palindromic"
  ${cmd}
  checkerror

  cmd="./Queryms.sh tmp1 tmp2 8 -3"
  ${cmd}
  checkerror

  cmd="./Queryms.sh tmp1 tmp2 8 -3 palindromic"
  ${cmd}
  checkerror

  cmd="./Queryms.sh tmp1 tmp2 8 3"
  ${cmd}
  checkerror

  cmd="./Queryms.sh tmp1 tmp2 8 3 palindromic"
  ${cmd}
  checkerror

  howmany=`expr $howmany + 2`
done
