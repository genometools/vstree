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
indexname=ychrIII.fna
Y3=../testdata/Grumbach/ychrIII.fna

cmd="mkvtreeall.sh -db ${Y3} -indexname ${indexname} -dna -pl 7"
${cmd}
checkerror

formatopts="-s -showdesc 10"
cmd="./vmatch.x -online -complete remred -q Testdir/P1 -e 3 ${formatopts} ${indexname}"
${cmd} > .shit
checkerror

cat .shit | grep -v '^#' > shit1

cmd="cmp -s shit1 Testdir/P1.y3.match"
${cmd}
checkerror
