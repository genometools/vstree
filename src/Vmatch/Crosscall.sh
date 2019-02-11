#!/bin/sh
SHELLSCRIPTS=`ls *.sh`
for filename in ${SHELLSCRIPTS}
do
  grep -qh ${filename} ${SHELLSCRIPTS}
  if test $? -ne 0
  then
    echo "$filename not called by other shell scripts"
  fi
done
