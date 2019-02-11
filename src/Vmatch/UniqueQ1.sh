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

if test $# -ne 5
then
  echo "Usage: $0 <uniquelength> <leastlength> <distance> <dbfile> <queryfile>"
  exit 1
fi

ulen=$1
llen=$2
distval=$3
dbfile=$4
queryfile=$5

if test $distval -lt 0
then
  hval=`expr $distval \* -1`
  dist="-h $hval"
else
  if test $distval -gt 0
  then
    dist="-e $distval"
  else
    dist=""
  fi
fi
echo "$0: $1 $2 $3 $4 $5"

cmd="mkvtreeall.sh -indexname all -dna -pl 8 -db $dbfile -q $queryfile"
${cmd}
checkerror

cmd="./vmatch.x -s -qnomatch $ulen -l $llen $dist all"
${cmd} > .shit
checkerror

cat .shit | grep -v '^#' > query.tmp

cmd="mkvtreeall.sh -indexname allquery -dna -pl 7 -db $queryfile"
${cmd}
checkerror

cmd="./vmatch.x -complete -q query.tmp allquery"
${cmd} > complete.tmp
checkerror

uniques=`cat query.tmp | grep '^>' | wc -l`
matches=`cat complete.tmp | grep -v '^# args' | wc -l`
if test $uniques -ne $matches
then
  echo "unique=$uniques matches=$matches"
  exit 1
fi

cmd="mkvtreeall.sh -indexname allDB -dna -pl 8 -db $dbfile"
${cmd}
checkerror

cmd="./vmatch.x -l $llen $dist -q query.tmp allDB"
${cmd} > .shit
checkerror

qmatches=`cat .shit | grep -v '^#' | wc -l`
if test $qmatches -ne 0
then
  echo "qmatches=$qmatches"
  exit 1
fi
echo "Okay"
