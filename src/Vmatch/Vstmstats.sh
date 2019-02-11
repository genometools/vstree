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

if test $# -ne 2
then
  echo "Usage: $0 <patternfile> <textfile>"
  exit 1
fi
patternfile=$1
textfile=$2
indexname=`basename ${patternfile}`
if test -f "${indexname}.prj"
then
  echo "# index ${indexname} already exists"
else
  mkvtreeopt="-db ${patternfile} -dna -pl 1 -sti1 -tis -ois -suf -lcp -bck"
  cmd="mkvtree.sh ${mkvtreeopt}"
  ${cmd}
  checkerror
  
  cmd="../Mkvtree/mklsf.x ${indexname}"
  ${cmd}
  checkerror
fi

vmatchopt="-l 1 -qspeedup 4 -q ${textfile} ${indexname}"
cmd="./vmatch.x ${vmatchopt}"
time ${cmd}
checkerror
