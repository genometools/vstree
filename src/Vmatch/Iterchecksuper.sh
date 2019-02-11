#!/bin/bash

function checkerror()
{
  $1
  if test $? -ne 0
  then
    echo "failure: ${1}"
    exit 1
  else
    echo "okay ${1}"
  fi
}

Y3=../testdata/Grumbach/ychrIII.seq

checkerror "mkvtree.sh -db ${Y3} -dna -v -pl -allout"
checkerror "./Checksuper.sh 15 ychrIII.seq"
