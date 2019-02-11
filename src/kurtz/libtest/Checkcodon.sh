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

cmd="head -n 113 ${AT}"
${cmd} > tmp1
checkerror

cmd="mkvtree -indexname dnaindx -db tmp1 -dna -v -ois -pl"
${cmd}
checkerror

cmd="codon.x noframe 1 dnaindx"
${cmd}
checkerror

cmd="codon.x noframe 2 dnaindx"
${cmd}
checkerror

cmd="codon.x noframe 15 dnaindx"
${cmd}
checkerror
