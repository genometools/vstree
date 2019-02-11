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
for filename in `ls ${IKEDA}/*.seq`
do
  echo $filename
  for d in 0 1 2 3
  do
    l=`expr $d \* 10`
    l=`expr $l + 24`
    if test $d -ne 0
    then
      dist=`expr $d \* -1`
      cmd="Rep.sh $filename $l $dist"
      ${cmd}
      checkerror
      cmd="Rep.sh $filename $l $dist palindromic"
      ${cmd}
      checkerror
      dist=$d
      cmd="Rep.sh $filename $l $dist"
      ${cmd}
      checkerror
      cmd="Rep.sh $filename $l $dist palindromic"
      ${cmd}
      checkerror
    fi
  done
  cmd="Cnt.sh $filename 18"
  ${cmd}
  checkerror
done
