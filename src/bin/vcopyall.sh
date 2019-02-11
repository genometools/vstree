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

if test $# -eq 0
then
  echo "Usage: $0 <file1> <file2> ..."
  exit 1
fi

for filename in $*
do
  vstreepath=`vpathkern.sh $filename`
  cmp -s $vstreepath $filename
  if test $? -ne 0
  then
    echo "$filename modified"
    cmd="cp -f $vstreepath ."
    ${cmd}
    checkerror
  fi
done
