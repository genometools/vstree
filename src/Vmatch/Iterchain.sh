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
inputfiles=`ls testdata/Ikeda/*.fna | sort -r`
for filename1 in ${inputfiles}
do
  for filename2 in ${inputfiles}
  do
    if test "${filename1}" != "${filename2}"
    then
      echo "${filename1} ${filename2}"
      cmd="./Runchain.sh ${filename1} ${filename2} 17 16 15 14 13"
      ${cmd}
      checkerror
    fi
  done
done
