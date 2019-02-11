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

echo "----------$0-------------"

cleanpp.sh
for len in 15 18 19 20
do
  cmd="./Query.sh dna $len 0"
  ${cmd}
  checkerror
done

for len in 13 12 11 10
do
  cmd="./Query.sh protein $len 0"
  ${cmd}
  checkerror
done

for i in 1 2 3 4 5
do
  dist=`expr $i \* -1`
  cmd="./Query.sh dna 60 $dist"
  ${cmd}
  checkerror
  cmd="./Query.sh dna 60 $i"
  ${cmd}
  checkerror
done

for len in 27 33 40 44
do
  case $len in
    27) edist=1
        hdist=-1;;
    33) edist=2
        hdist=-2;;
    40) edist=3
        hdist=-3;;
    44) edist=4
        hdist=-4;;
  esac
  cmd="./Query.sh dna $len $hdist"
  ${cmd}

  checkerror
  cmd="./Query.sh dna $len $edist"
  ${cmd}

  checkerror
done

cmd="./Query.sh protein 18 -1"
${cmd}
checkerror

cmd="./Query.sh protein 18 1"
${cmd}
checkerror

cmd="./Query.sh protein 19 -2"
${cmd}
checkerror

cmd="./Query.sh protein 19 2"
${cmd}
checkerror

cmd="./Query.sh protein 25 -3"
${cmd}
checkerror

cmd="./Query.sh protein 25 3"
${cmd}
checkerror
