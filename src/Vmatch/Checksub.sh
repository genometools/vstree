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

echo "------------ $0 -----------------"

AT=../testdata/at1MB
U8=../testdata/U89959.fna
cmd="mkvtree.sh -allout -v -pl 8 -dna -db ${AT}"
${cmd}
checkerror

for dist in -2 -1 0 1 2
do
  case $dist in
    -2) mode="-h 2 -allmax ";;
    -1) mode="-h 1 -allmax ";;
    0) mode="";;
    1) mode="-e 1 -allmax ";;
    2) mode="-e 2 -allmax ";;
  esac
  cmd="./vmatch.x $mode -l 30 -q ${U8} -noevalue at1MB"
  ${cmd} > .shit
  checkerror

  cat .shit | grep -v '^#' > shit1

  cmd="./vmatch.x $mode -l 30 -q ${U8} (71000,106000) -noevalue at1MB"
  ${cmd} > .shit
  checkerror

  cat .shit | grep -v '^#' > shit2

  cmd="cmp -s shit1 shit2"
  ${cmd}
  checkerror
done

echo "okay: $0"
