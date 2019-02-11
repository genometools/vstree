#!/bin/sh
USAGE="Usage: $0 <program with args>"

if test $# -eq 0
then
  echo $USAGE
  exit 1
fi

echo "Make a choice:"
echo "Calgary Corpus          1"
echo "Canterbury Corpus       2"
echo "Large Canterbury Corpus 3"
echo "Bio Corpus              4"

read inp

case "$inp" in
  "1") echo "You have chosen the Calgary Corpus"
       DIR=${VSTREEDATA}/Calgary
       files="$CALGARY $CALGARYCORPUS";;
  "2") echo "You have chosen the Canterbury Corpus"
       DIR=${VSTREEDATA}/Canterbury
       files="$CANTERBURY";;
  "3") echo "You have chosen the Large Canterbury Corpus"
       DIR=${VSTREEDATA}/Canterbury
       files="$LCANTERBURY";;
  "4") echo "You have chosen the Bio Corpus";;
  *) echo "choice not possible!"
     exit 1;
esac

if test $inp -eq 4
then
  for filename in `ls ${VSTREEDATA}/DNA/*/*.seq`
  do
    echo "$filename"
    $* $filename 
    if test $? -ne 0
    then
      echo " failed"
      exit 1
    fi
  done
else
  for pat in $files
  do
    for filename in `ls $DIR/$pat`
     do
       if test $1 = gzip
       then
          echo "gzip $filename"
          gzip -9 -c $filename > /dev/null
       else
          echo "$1 $filename"
          $* $filename 
          if test $? -ne 0
          then
            echo " failed"
            exit 1
          fi
       fi
     done
  done
fi
