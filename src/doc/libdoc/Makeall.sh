#!/bin/sh
checkcmd() {
  if test $? -ne 0
  then
    exit 1
  fi
}

if test $# -ne 1 
then
  echo "$0 <texfileprefix>"
  exit 1
fi

TEXFILEPREFIX=$1
LATEX=pdflatex
Geninc.sh ${TEXFILEPREFIX}
checkcmd
#Genvmatch.sh
#checkcmd
touch ${TEXFILEPREFIX}.ind
$LATEX ${TEXFILEPREFIX}
checkcmd
makeindex -q ${TEXFILEPREFIX}.idx -o ${TEXFILEPREFIX}.ind
checkcmd
$LATEX ${TEXFILEPREFIX}
checkcmd
$LATEX ${TEXFILEPREFIX}
checkcmd
