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

cleanpp.sh
AT=../testdata/at1MB
U8=../testdata/U89959.fna
SW=../testdata/swiss1MB

cmd="../Mkvtree/mkdna6idx.x  -db ${AT} -indexname at1MBcodon -v"
${cmd}
checkerror

for app in "-hxdrop 1" "-exdrop 1" "-h 1" "-e 1"
do
  for pos in "-absolute"  " "
  do
    cmd="./Checkdet.sh -l 50 ${app} ${pos} at1MBcodon.6fr"
    ${cmd}
    checkerror
  done
done

cmd="mkvtreeall.sh  -db ${SW} -smap TransProt11 -pl"
${cmd}
checkerror

for app in "-hxdrop 1" "-exdrop 1" "-h 1" "-e 1"
do
  for pos in "-absolute"  " "
  do
    cmd="./Checkdet.sh -l 50 -q ${AT} -dnavsprot 1 ${app} ${pos} swiss1MB"
    ${cmd}
    checkerror
  done
done

cmd="./Checkdet.sh -l 50 -q ${AT} -exdrop 1 swiss1MB"
${cmd}
checkerror

cmd="mkvtreeall.sh  -db ${AT} -dna -pl"
${cmd}
checkerror

for query in "-q ${U8}" " "
do
  for app in "-hxdrop 1" "-exdrop 1" "-h 1" "-e 1"
  do
    for pos in "-absolute"  " "
    do
      for palin in "-p" "-p -d"
      do
        cmd="./Checkdet.sh -l 50 ${query} ${app} ${pos} ${ndist} ${palin} at1MB"
        ${cmd}
        checkerror
      done
    done
  done
done

echo "okay: $0"
