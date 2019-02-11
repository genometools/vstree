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
  echo "Usage: $0 <directory with executables>"
  exit 1
fi
execdir=$1
execfiles=`find ${execdir} -maxdepth 1 -type f ! -name '*\.*' -print`
for filename in ${execfiles}
do
  bname=`basename ${filename}`
  cmd="ln -s ${filename} ${bname}.x"
  ${cmd}
  checkerror
done
