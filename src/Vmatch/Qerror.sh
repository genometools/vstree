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

U8=../testdata/U89959.fna
AT=../testdata/at1MB
query=${U8}
database=${AT}
len=30

cmd="mkvtreeall.sh -indexname all -v -pl -dna -db ${database}"
${cmd}
checkerror

cmd="mkvtreeall.sh -db ${database} -v -pl -dna"
${cmd}
checkerror

cmd="./vmatch.x -l ${len} all"
${cmd} > shit1
checkerror

cmd="./vmatchselect.x -v shit1"
${cmd} > shit2
checkerror

cmd="./vmatch.x -q ${query} -l ${len} all"
${cmd} > shit3
checkerror

cmd="./vmatchselect.x -v shit3"
${cmd} > shit4
checkerror

cmd="mkvtree.sh -indexname allq -tis -ois -db ${query} -v -dna"
${cmd}
checkerror

cmd="./vmatch.x -q allq -l ${len} all"
${cmd} > shit5
checkerror

cmd="./vmatchselect.x -v shit5"
${cmd} > shit6
checkerror
