#!/bin/sh
if test $# -ne 1
then
  echo "Usage: $0 <filename>"
  exit 1
fi
echo "$0 $1"
TMPFILE=`mktemp /tmp/changeaddr.XXXXXX` || exit 1
sed -e 's/Kurtz@TechFak\.Uni-Bielefeld\.DE/kurtz@zbh\.uni-hamburg\.de/g' $1\
      > ${TMPFILE}
mv ${TMPFILE} $1
