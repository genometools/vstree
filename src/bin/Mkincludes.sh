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

echo "make Filegoals.mf"
if test -f Excludefiles
then
  if test -d LIB
  then
    LIBCFILES=`ls *.c LIB/*.c | grep -v -f Excludefiles`
  else
    LIBCFILES=`ls *.c | grep -v -f Excludefiles`
  fi
else
  echo "Excludefiles does not exist"
  if test -d LIB
  then
    LIBCFILES=`ls *.c LIB/*.c`
  else
    LIBCFILES=`ls *.c`
  fi
fi
ls ${LIBCFILES} | mkfilegoals.pl $addoption splintlist cleanbuild o dbg so prepro pr splint > Filegoals.mf
