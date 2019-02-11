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

DEBUGLEVEL=1
export DEBUGLEVEL

programlist="./mkvtree.x"
if test -f ./mkvtree.dbg.x
then
  programlist="${programlist} ./mkvtree.dbg.x"
fi

if test -f ./mkvram.x
then
  programlist="${programlist} ./mkvram.x"
fi

for prog in ${programlist}
do
  cmd="./Mkvcalls.pl $prog DATA/DBdna1 DATA/DBprot1"
  ${cmd}
  checkerror
  cmd="./Mkvcalls.pl $prog DATA/DBdna2 DATA/DBprot2"
  ${cmd}
  checkerror
  cmd="./Mkvcalls.pl $prog DATA/DBdna3 DATA/DBprot3"
  ${cmd}
  checkerror
  cmd="./Mkvcalls.pl $prog DATA/Emblfile DATA/DBprot1"
  ${cmd}
  checkerror
  cmd="./Mkvcalls.pl $prog DATA/GenBankfile DATA/DBprot3"
  ${cmd}
  checkerror
done
