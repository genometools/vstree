#!/bin/sh

if test $# -ne 2
then
  echo "Usage: $0 <len> <index>"
  exit 1
fi

len=$1
indexname=$2

checkerror() 
{
  if [ "$2" ]
  then
    $1 > $2
  else
    $1
  fi
  if test $? -ne 0
  then
    echo "failure: ${1}"
    exit 1
  else
    echo "okay ${1}"
  fi
}

removehash()
{
  TMPFILE=`mktemp MATCH.XXXXXX || exit 1`
  sed -e '/^\#/d' ${1} > ${TMPFILE}
  mv ${TMPFILE} $1
}

TMPFILE1=`mktemp MATCH.XXXXXX || exit 1`

checkerror "./vmatch.x -l ${len} -s leftseq ${indexname}"  "${TMPFILE1}"
removehash "${TMPFILE1}"
checkerror "mkvtree.sh -indexname maxpairs -db ${TMPFILE1} -tis -lcp -suf -bck -ois -dna -pl"

TMPFILE2=`mktemp SUPERMATCH.XXXXXX || exit 1`

checkerror "./vmatch.x -supermax -l ${len} -s leftseq ${indexname}" "${TMPFILE2}"
removehash "${TMPFILE2}"

checkerror "./vmatch.x -s leftseq -complete -q ${TMPFILE2} -selfun selsuperinc.so maxpairs"

rm -f ${TMPFILE1} ${TMPFILE2}
