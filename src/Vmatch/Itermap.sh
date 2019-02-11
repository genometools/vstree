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

ATK=../testdata/at100K1
ATK2=../testdata/at100K2
VAC=../testdata/Grumbach/vaccg.fna
CNN=../testdata/Grumbach/chntxx.fna
Y3=../testdata/Grumbach/ychrIII.fna
SWK1=../testdata/sw100K1
SWK2=../testdata/sw100K2

echo "----------$0-------------"
seqs="`ls ${ATK} ${ATK2}` ${VAC} ${CNN} ${Y3}"

filename1="${SWK1}"
filename2="${SWK2}"
echo "`basename $filename1` `basename $filename2`"
cmd="./Map.sh protein $filename1 $filename2 -l 5"
${cmd}
checkerror

#Map.sh protein $filename1 $filename2 -l 5 -online
#checkerror

for filename1 in $seqs
do
  for filename2 in $seqs
  do
    if test $filename1 != $filename2
    then 
      echo "`basename $filename1` `basename $filename2`"
      cmd="./Map.sh dna $filename1 $filename2 -l 18"
      ${cmd}
      checkerror
      #cmd="Map.sh dna $filename1 $filename2 -l 18 -p"
      #${cmd}
      #checkerror
      #cmd="Map.sh dna $filename1 $filename2 -l 18 -online"
      #${cmd}
      #checkerror
      #cmd="Map.sh dna $filename1 $filename2 -l 18 -p -online"
      #${cmd}
      #checkerror
    fi
  done
done
echo "Test successful at `date`"
