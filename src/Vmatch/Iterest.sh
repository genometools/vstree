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
cmd="mkvtreeall.sh -indexname all -v -pl 8 -dna -db ../testdata/at1MB"
${cmd}
checkerror

cmd="./Est.sh DNA 100 0"
${cmd}
checkerror
cmd="./Est.sh DNA 100 1"
${cmd}
checkerror
cmd="./Est.sh DNA 120 2"
${cmd}
checkerror
cmd="./Est.sh DNA 140 3"
${cmd}
checkerror
cmd="./Est.sh DNA 160 4"
${cmd}
checkerror
cmd="./Est.sh DNA 50 0"
${cmd}
checkerror
cmd="./Est.sh DNA 55 1"
${cmd}
checkerror
cmd="./Est.sh Prot 10 0"
${cmd}
checkerror
cmd="./Est.sh Prot 16 1"
${cmd}
checkerror
cmd="./Est.sh Prot 21 2"
${cmd}
checkerror
cmd="./Est.sh Prot 22 3"
${cmd}
checkerror

echo "okay: $0"
