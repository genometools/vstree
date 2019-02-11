#!/bin/sh
if test ! -f ./allwhatsthere.x
then
  exit 0
fi
for filename in `ls *.prj`
do
  indexname=`basename ${filename} .prj`
  ./allwhatsthere.x ${indexname}
  if test $? -ne 0
  then
    echo "failure: ./allwhatsthere.x ${indexname}"
    exit 1
  fi
done
