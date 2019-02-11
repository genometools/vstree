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
GRUMBACH=../testdata/Grumbach
seqs=`ls ${GRUMBACH}/[a-z]*.fna`

leastlength=14
for filenameA in $seqs
do
  for filenameB in $seqs
  do
    if test $filenameA != $filenameB
    then 
      echo "`basename $filenameA` `basename $filenameB` $leastlength"
      cmd="./Mum.sh $filenameA $filenameB $leastlength"
      ${cmd}
      checkerror
    fi
  done
done
echo "Test successful at `date`"
