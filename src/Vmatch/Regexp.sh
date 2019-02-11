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

if test $# -eq 1
then
  numofregexp=$1
  numoflines=`expr ${numofregexp} + ${numofregexp}`
  head -n ${numoflines} Regexp.fas > Regexp.tmp
else
  if test $# -eq 0
  then
    cp Regexp.fas Regexp.tmp
  else
    echo "Usage: $0 [numofregexp]"
    exit 1
  fi
fi

cmd="mkvtree.sh -allout -protein -db ${SWK} -pl"
${cmd}
checkerror

cmd="./vmatch.x -q Regexp.tmp -complete -online -v swiss10K"
${cmd} > tmp1
checkerror

cmd="./vmatchselect.x -sort ia tmp1"
${cmd} > tmp
checkerror

#grep -v '^#' tmp > tmp1.sort

#cmd="./vmatch.x -q Regexp.tmp -complete -v swiss10K"
#${cmd} > tmp2
#checkerror
#
#cmd="./vmatchselect.x -sort ia tmp2"
#${cmd} > tmp
#checkerror
#
#grep -v '^#' tmp > tmp2.sort
#cmd="cmp -s tmp1.sort tmp2.sort"
#${cmd}
#checkerror
#wc tmp1.sort
rm -f Regexp.tmp
