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

if test $# -ne 3
then
  echo "Usage: ${0} <queryfilename> <length> <indexname>"
  exit 1
fi

query=${1}
length=${2}
index=${3}
cmd="./vmatch.x -l ${length} -q ${query} ${index}"
${cmd} > .shit
checkerror

cat .shit | grep -v '^#' | sort -u > tmp0.match

cmd="./vmatch.x -qspeedup 2 -l ${length} -q ${query} ${index}"
${cmd} > .shit
checkerror
cat .shit | grep -v '^#' | sort -u > tmp1.match

cmd="cmp -s tmp[01].match"
${cmd}
checkerror

echo "okay: $0"
