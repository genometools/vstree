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

for qspeedup in 5
do
  echo "------------ Checkall.sh ${qspeedup} -----------------"
  QUERYSPEEDUP=${qspeedup}
  export QUERYSPEEDUP
  cmd="Checkall.sh"
  ${cmd}
  checkerror
  cleanpp.sh
done
