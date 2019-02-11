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

U8=../testdata/U89959.fna
GRUMBACHSEQ=../testdata/Grumbach

echo "----------$0-------------"
for filename in `ls ../testdata/at1MB ${GRUMBACHSEQ}/*.seq`
do
  echo ${filename}
  if test ${filename} = "${GRUMBACHSEQ}/mpocpcg.seq"
  then
    echo "skip mpocpcg.seq"
  else
    if test ${filename} = "${GRUMBACHSEQ}/ychrIII.seq"
    then
      echo "skip ychrIII.seq"
    else
      cmd="mkvtreeall.sh -db ${filename} -dna -pl"
      ${cmd}
      checkerror
      cmd="./Qonl.sh `basename ${filename}` ${U8} 20 0"
      ${cmd}
      checkerror
      cmd="./Qonl.sh `basename ${filename}` ${U8} 20 0 palindromic"
      ${cmd}
      checkerror
      cmd="./Qonl.sh `basename ${filename}` ${U8} 30 -1"
      ${cmd}
      checkerror
      cmd="./Qonl.sh `basename ${filename}` ${U8} 30 -1 palindromic"
      ${cmd}
      checkerror
      cmd="./Qonl.sh `basename ${filename}` ${U8} 30 1"
      ${cmd}
      checkerror
      cmd="./Qonl.sh `basename ${filename}` ${U8} 30 1 palindromic"
      ${cmd}
      checkerror
    fi
  fi
done

