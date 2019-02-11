#!/bin/sh

grep 'DEFINEVMATCHDB=-DVMATCHDB' Vmatch/Makefile > /dev/null
if test $? -eq 0
then
  echo "do not use \"-DVMATCHDB\" in Vmatch/Makefile"
  exit 1
fi

grep '\-DNOSPACEBOOKKEEPING' Makedef > /dev/null
if test $? -eq 1
then
  echo "use \"-DNOSPACEBOOKKEEPING\" in Makedef"
  exit 1
fi


