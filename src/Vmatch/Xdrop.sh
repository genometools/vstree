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
seqs=`ls ${GRUMBACH}/[m-z]*.fna`

for filename in $seqs
do
  cmd="mkvtreeall.sh -db $filename -v -dna -pl"
  ${cmd}
  checkerror

  for xdropbelow in 1 2 3 4 5 6
  do
    for xdropmode in "-exdrop" "-hxdrop"
    do
      for leastscore in 200 300 400 500 600 700 800 900
      do
        for mode in "" "-p"
        do
          vmoptions="$mode $xdropmode $xdropbelow -leastscore $leastscore" 
          cmd="./vmatch.x ${vmoptions} `basename ${filename}`"
          ${cmd} > tmp.match
          checkerror

          cmd="./vmatchselect.x tmp.match"
          ${cmd} > /dev/null
          checkerror

          cmd="./vmatch.x -s ${vmoptions} `basename ${filename}`"
          ${cmd} > /dev/null
          checkerror
        done
      done
    done
  done
done
echo "Test successful at `date`"
