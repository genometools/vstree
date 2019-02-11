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

if test $# -ne 4
then
  echo "Usage: $0 <uniquelength> <leastlength> <distance> <inputfile>"
  exit 1
fi

ulen=$1
llen=$2
filename=$4

if test $3 -lt 0
then
  hval=`expr $3 \* -1`
  dist="-h $hval -allmax"
else
  if test $3 -gt 0
  then
    dist="-e $3 -allmax"
  else
    dist=""
  fi
fi
echo "$0 $1 $2 $3 $4"
cmd="mkvtreeall.sh -indexname all -dna -pl -db $filename"
${cmd}
checkerror

MODE="-d -p"

cmd="./vmatch.x ${MODE} -s -dbnomatch $ulen -l $llen $dist all"
${cmd} > .shit
checkerror

cat .shit | grep -v '^#' > query.tmp

sizeofquery=`cat query.tmp | wc -c`

if test ${sizeofquery} -eq 0
then
  exit 0
fi

cmd="./vmatch.x ${MODE} -complete -q query.tmp all"
${cmd} > complete.tmp
checkerror

uniques=`cat query.tmp | grep '^>' | wc -l`
matches=`cat complete.tmp | grep -v '^# args' | wc -l`
if test $uniques -ne $matches
then
  echo "unique=$uniques matches=$matches"
  exit 1
fi

cmd="mkvtreeall.sh -indexname shitindex -db query.tmp -dna -pl"
${cmd}
checkerror

cmd="./vmatch.x ${MODE} -l $llen $dist shitindex"
${cmd} > shit
checkerror

qmatches=`cat shit | grep -v '^#' |\
   grep -v '36     2  1130   P   36     2  1130   0    5.93e-13    72   100.00'\
   | wc -l`
if test $qmatches -ne 0
then
  echo "qmatches=$qmatches"
  exit 1
fi
echo "Okay"
