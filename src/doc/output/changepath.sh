#!/bin/sh

for filename in `ls *.out`
do
  sed -e 's/\/[A-Za-z0-9][A-Za-z0-9]*\/vstree\/src\/vstree\/src\/doc\///' $filename > .tmp
  mv .tmp $filename
done
