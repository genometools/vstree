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
  echo "Usage: $0 <minlength>"
  exit 1
fi

minlength=$1

inputindex=../testdata/swiss10K

cmd="mkvtree.sh -indexname proteinseq -db ${inputindex} -protein -allout -pl -v"
${cmd}
checkerror

cmd="head -n 113 ../testdata/at1MB"
${cmd} > tmp0
checkerror

vmatchoptions="-q tmp0 -l ${minlength} -v proteinseq"

cmd="./vmatch.x ${vmatchoptions}"
${cmd}
checkerror

cmd="./vmatch.x -online ${vmatchoptions}"
${cmd}
checkerror
