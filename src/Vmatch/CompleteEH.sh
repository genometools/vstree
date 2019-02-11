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
if test -f "at1MB.prj"
then
  echo "at1MB.prj exists"
else
  cmd="mkvtreeall.sh -db ../testdata/at1MB -dna -pl 7"
  ${cmd}
  checkerror
fi
cmd="./vmatch.x -q Testdir/P1 -complete remred -online -e 3 -s -p -d at1MB"
${cmd} > .shit
checkerror

cat .shit | grep -v '^#' > shit1

cmd="./vmatch.x -q Testdir/P1 -complete remred -online -h 3 -s abbrev -p -d at1MB"
${cmd} > .shit
checkerror

cat .shit| grep -v '^#' >> shit1

cmd="cmp -s shit1 Testdir/EHcheck"
${cmd}
checkerror

echo "okay: $0"
