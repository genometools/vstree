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

cmd="mkvtreeall.sh -indexname Seq1 -dna -pl 1 -db Testdir/Seq1"
${cmd}
checkerror

for pmode in "" "-p"
do
  for smode in "" "-showdesc 10"
  do
    mode="$pmode $amode $smode"
    cmd="./Cmponl.sh $mode -l 3 -s -q Testdir/Query2 Seq1"
    ${cmd}
    checkerror
  done
done

for pmode in "" "-p"
do
    for dmode in "-h 1" "-e 1"
    do
      for smode in "" "-showdesc 10"
      do
        mode="$pmode $dmode $smode"
        cmd="./Cmponl.sh -allmax $mode -l 4 -s -q Testdir/Query2 Seq1"
        ${cmd}
        checkerror
      done
    done
done

echo "okay: $0"
