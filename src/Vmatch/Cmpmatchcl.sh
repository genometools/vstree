#!/bin/sh

set -e -x

checkerror() {
if test $? -ne 0
then
  echo "failure: ${cmd}"
  exit 1
else
  echo "okay: ${cmd}"
fi
}

if test $# -le 2
then
  echo "Usage: $0 <indexname> <vmatch arguments>"
  exit 1
fi

database=$1
shift
vmatchargs=$*

indexname=`basename $database`
if test -f ${indexname}.prj
then
  echo "index ${indexname} exists"
else
  cmd="mkvtree.sh -db ${database} -dna -v -pl -bwt -suf -lcp -tis -ois"
  ${cmd}
  checkerror
fi

cmd="./vmatch.x $vmatchargs ${indexname}"
${cmd} > tmp.match
checkerror

for mode in "gapsize 5000" "overlap 10" "erate 35" 
do
  rm -f clout.[0-9]*.[0-9]*.match
  rm -f vmclout.[0-9]*.[0-9]*.match
  cmd="./matchcluster.x -${mode} -outprefix clout tmp.match"
  ${cmd} 
  checkerror
  cmd="./vmatch.x $vmatchargs -pp matchcluster ${mode} outprefix vmclout ychrIII.fna"
  ${cmd}
  checkerror
  cloutfiles=`ls clout.[0-9]*.[0-9]*.match | wc -l`
  vmcloutfiles=`ls vmclout.[0-9]*.[0-9]*.match | wc -l`
  if test $cloutfiles != $vmcloutfiles
  then
    echo "$cloutfiles != $vmcloutfiles"
    exit 1
  fi
  for filename in `ls clout.[0-9]*.[0-9]*.match`
  do
    if test -f "vm${filename}"
    then
      echo "vm${filename} exists"
      cmd="cmp -s ${filename} vm${filename}"
      ${cmd}
      checkerror
    else
      echo "vm${filename} does not exist"
      exit 1
    fi
  done
done
