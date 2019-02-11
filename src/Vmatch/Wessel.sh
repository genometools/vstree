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

outopts="-tis -ois -suf -lcp -bwt"
mkvtreeopt="-indexname wessel -db Testdir/Wessel.fna -dna -pl 2 -v ${outopts}"

cmd="mkvtree.sh ${mkvtreeopt}"
${cmd}
checkerror

vmatchopt="-l 50 -seedlength 50 -e 10 -s -allmax wessel" > Testdir/Wessel.test

cmd="./vmatch.x ${vmatchopt}"
${cmd} > .tmp
checkerror

grep -v '^#' .tmp > Testdir/Wessel.test

cmd="cmp -s Testdir/Wessel.test Testdir/Wessel.out"
${cmd}
checkerror

rm -f Testdir/Wessel.test
echo "okay: $0"
