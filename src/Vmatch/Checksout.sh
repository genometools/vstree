#!/bin/sh

checkerror() {
  if test $? -ne 0
  then
    echo "failure: ${cmd}"
    exit 1
  fi
}

echo "------------ $0 -----------------"

cmd="mkvtreeall.sh -indexname Seq1 -dna -pl 1 -db Testdir/Seq1"
${cmd}
checkerror

for mode in h e
do
  cmd="./vmatch.x -allmax -l 3 -$mode 1 -s abbrev                      Seq1"
  ${cmd}
  checkerror
  cmd="./vmatch.x -allmax -l 3 -$mode 1 -s abbrev -p                   Seq1"
  ${cmd}
  checkerror
  cmd="./vmatch.x -allmax -l 3 -$mode 1 -s abbrev    -q Testdir/Query1 Seq1"
  ${cmd}
  checkerror
  cmd="./vmatch.x -allmax -l 3 -$mode 1 -s abbrev -p -q Testdir/Query1 Seq1"
  ${cmd}
  checkerror
  cmd="./vmatch.x         -l 3 -$mode 1 -s abbrev                      Seq1"
  ${cmd}
  checkerror
  cmd="./vmatch.x         -l 3 -$mode 1 -s abbrev -p                   Seq1"
  ${cmd}
  checkerror
  cmd="./vmatch.x         -l 3 -$mode 1 -s abbrev    -q Testdir/Query1 Seq1"
  ${cmd}
  checkerror
  cmd="./vmatch.x         -l 3 -$mode 1 -s abbrev -p -q Testdir/Query1 Seq1"
  ${cmd}
  checkerror
done
rm -f Seq1.*

echo "okay: $0"
