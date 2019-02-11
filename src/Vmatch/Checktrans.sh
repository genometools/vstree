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

SWS=../testdata/swissSmall

for trans in `ls ../Mkvtree/TRANS/TransProt*`
do
  echo $trans
  cmd="mkvtreeall.sh -indexname sw -db ${SWS} -v -pl -smap $trans"
  ${cmd}
  checkerror
  cmd="./vmatch.x -s -l 20 sw"
  ${cmd} 
  checkerror
done

echo "okay: $0"
