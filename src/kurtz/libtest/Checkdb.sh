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
  echo "Usage: $0 genbank-file"
  exit 1
fi

cmd="gb2fasta.sh $1"
${cmd} > shit1
checkerror

cmd="./readdb.x $1"
${cmd} > shit3
checkerror

grep -v '^#' shit3 > shit2
cmd="cmp -s shit[12]"
${cmd}
checkerror
