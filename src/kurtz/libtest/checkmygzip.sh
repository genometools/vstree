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

if test $# -ne 1
then
  echo "Usage: $0 directory"
  exit 1
fi
directory=$1

for filename in `find ${directory} -type f -print`
do
  echo ${filename}
  gzip -9 -c ${filename} > shit.gz
  cmd="mygzip.x shit.gz"
  ${cmd} > shit
  checkerror

  cmd="cmp -s shit ${filename}"
  ${cmd} 
  checkerror
done
rm -f shit shit.gz
