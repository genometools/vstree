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

AT=../testdata/at1MB

cmd="mkvtree.sh -v -db ${AT} -dna -pl -allout"
${cmd}
checkerror

TMPFILE1=tmp1.$$
TMPFILE2=tmp2.$$

for len in 300 200 100 80 50 30 20
do
  cmd="./vmatch.x -l ${len} at1MB"
  ${cmd} > ${TMPFILE1}
  checkerror
  grep -v '^#' ${TMPFILE1} | sort > tmp1
  for numproc in 2 3 4 5 6 7 8 9 10 11 12 33 56
  do
    cmd="./vmatch.x -numproc ${numproc} -l ${len} at1MB"
    ${cmd} > ${TMPFILE2}
    checkerror
    grep -v '^#' ${TMPFILE2} | sort > tmp2
    cmd="cmp -s tmp1 tmp2"
    ${cmd}
    checkerror
  done
done
rm -f ${TMPFILE1} ${TMPFILE2}
