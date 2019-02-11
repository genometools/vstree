#!/bin/sh
ls *.c > /dev/null
if test $? -eq 1
then
  echo "$0: no C-files available"
  exit 1
fi

if test $# -eq 1
then
  if test $1 == "-local"
  then
    echo "with option -local"
    addoption="-local"
  else
    echo "illegal option $1"
    exit 1
  fi
else
  if test $# -ne 0
  then
    echo "Usage: $0 [-local]"
    exit 1
  fi
fi

if test -f Excludefiles.sed
then
  LIBCFILES=`ls *.c | sed -f Excludefiles.sed`
else
  echo "Excludefiles.sed does not exist"
  LIBCFILES=`ls *.c`
fi

echo "make Filelists.mf"
ls ${LIBCFILES} | mkfilegoals.pl $addoption liblist splintlist prototypelist > Filelists.mf
echo "make Filegoals.mf"
ls ${LIBCFILES} | mkfilegoals.pl $addoption cleanbuild o dbg prepro splint princ > Filegoals.mf
