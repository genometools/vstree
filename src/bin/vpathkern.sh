#!/bin/sh
if test $# -ne 1
then
  echo "Usage: $0 <inputfile>"
  exit 1
fi
inputfile=$1
vpath.sh ${inputfile} src
