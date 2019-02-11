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

for size in 10 20 30 40 50 60
do
  cmd="rundict.x ${size} 100"
  ${cmd}
  checkerror
done

for size in 100 1000 1000 10000
do
  for maxnum in 1000 10000 100000
  do
    cmd="rundict.x ${size} ${maxnum}"
    echo "${cmd}"
    ${cmd}
    checkerror
  done
done
