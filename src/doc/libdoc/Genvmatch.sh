#!/bin/sh
VMATCHDIR="../../Vmatch"
CFILES="detmatch echomatch"
HFILES="matchstate mparms virtualexp matchtask outinfo"

C2LIT="c2lit.x -refnamefile Refnames"
EXPORT="-export"

for filename in $CFILES
do
  if test ! -f ${filename}.inc
  then
    echo "generate ${filename}.inc"
    $C2LIT $EXPORT $VMATCHDIR/${filename}.c | lit2alt.x -C > ${filename}.inc
    else
    if test $VMATCHDIR/${filename}.c -nt ${filename}.inc
    then
      echo "update ${filename}.inc"
      $C2LIT $EXPORT $VMATCHDIR/${filename}.c | lit2alt.x -C > ${filename}.inc
    fi
  fi
done

for filename in $HFILES
do
  if test ! -f ${filename}.inc
  then
    echo "generate ${filename}.inc"
    $C2LIT $VMATCHDIR/${filename}.h | lit2alt.x -C > ${filename}.inc
  else
    if test $VMATCHDIR/${filename}.h -nt ${filename}.inc
    then
      echo "update ${filename}.inc"
      $C2LIT $VMATCHDIR/${filename}.h | lit2alt.x -C > ${filename}.inc
    fi
  fi
done
