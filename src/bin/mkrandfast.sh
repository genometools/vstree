#!/bin/sh

if test $# -ne 1
then
  echo "Usage: $0 <number of sequences>"
  exit 1
fi
i=0
len=10
filelen=`expr 10000 \* $1`
oneFE.rand.x acgt $filelen
seqs=`onePE.rand.x R${filelen}acgt $1 20`
for seq in $seqs
do
  printf ">seq%s\n%*.*s\n" $i $len $len $seq
  len=`expr $len + 1`
  i=`expr $i + 1`
  if test $len -gt 20
  then
    len=10
  fi
done
rm -f R${filelen}acgt
