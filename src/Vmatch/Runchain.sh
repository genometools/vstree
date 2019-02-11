#!/bin/sh

checkerror() {
if test $? -ne 0
then
  echo "failure: ${cmd}"
  exit 1
else
  echo "${cmd}"
fi
}

if test $# -le 2
then
  echo "Usage: $0 file1 file2 minvalue1 minvalue2 ..."
  exit 1 
fi
file1=$1
file2=$2
shift
shift
indexname=chainidx

VMATCH="./vmatch.x"

cmd="mkvtree.sh -indexname ${indexname} -db ${file1} -allout -dna -pl"
${cmd}
checkerror

for length in $*
do
  cmd="${VMATCH} -l ${length} -q ${file2} ${indexname}"
  ${cmd} > tmp.match
  checkerror
  for option in "global" "global gc" "global ov" "local" "local 10b" "local 5p"
  do
    if test -f ./chain2dim.dbg.x
    then
      cmd="./chain2dim.dbg.x -silent -${option} tmp.match"
      ${cmd} 
      checkerror
    fi
    cmd="./chain2dim.x -silent -${option} tmp.match"
    ${cmd} 
    checkerror
    cmd="Chainformats.sh tmp.match"
    ${cmd}
    checkerror
    cmd="${VMATCH} -l ${length} -q ${file2} -pp chain ${option} ${indexname}"
    ${cmd}
    checkerror
  done
done
rm -f tmp.match
