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

if test $# -ne 1
then
  echo "Usage: $0 <qvalue>"
  exit 1
fi

qvalue=$1
edistvalue=30

echo "----------$0-------------"
seqs=`ls ${GRUMBACH}/[a-z]*.fna`

for filenameA in $seqs
do
  for filenameB in $seqs
  do
    if test $filenameA != $filenameB
    then
      echo "`basename $filenameA` `basename $filenameB` $leastlength"
      cmd="Checkflychain.sh ${qvalue} ${edistvalue} $filenameA $filenameB"
      ${cmd}
      checkerror
    fi
  done
done
echo "Test successful at `date`"
