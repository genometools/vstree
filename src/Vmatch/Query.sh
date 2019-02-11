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

usage="$0 dna|protein <length> <distance>"

if test $# -ne 3
then
  echo ${usage}
  exit 1
fi
AT=../testdata/at1MB
U8=../testdata/U89959.fna
SW=../testdata/swiss1MB
SIXU8=../testdata/SIXU89959

echo "------------------------------- $0 ---------------------------"
if test "$1" = "dna"
then
  database=${AT}
  query=${U8}
  prefix=8
  smap="-smap ../Mkvtree/TRANS/TransDNA"
  allindex=allDNA
else
  if test "$1" = "protein"
  then
    database=${SW}
    query=${SIXU8}
    prefix=3
    smap="-smap ../Mkvtree/TRANS/TransProt11"
    allindex=allProt
  else
    echo ${usage}
    exit 1
  fi
fi

if test -f ${allindex}.prj
then
  echo "${allindex}.prj exists"
else
  cmd="mkvtreeall.sh -indexname $allindex -v -pl $prefix $smap -db ${database} -q $query"
  ${cmd}
  checkerror
fi

if test -f `basename ${database}`.prj
then
  echo "${database}.prj exists"
else
  cmd="mkvtreeall.sh -db ${database} -v -pl $prefix $smap"
  ${cmd}
  checkerror
fi

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

indexname=`basename ${database}`
cmd="./vmatch.x -l $2 ${dist} ${allindex}"
${cmd} > .shit
checkerror

#./vmatchselect.x -allmax .shit > shit1
#if test $? -ne 0
#then
#  echo "failure: ./vmatchselect.x -allmax shit1"
#  exit 1
#fi
#mv shit1 .shit

cat .shit | grep -v '^#' | sort > shit1
cmd="./vmatch.x -l $2 ${dist} -q $query $indexname"
${cmd} > .shit
checkerror

#./vmatchselect.x -allmax .shit > shit2
#if test $? -ne 0
#then
#  echo "failure: ./vmatchselect.x -allmax shit2"
#  exit 1
#fi 
#mv shit2 .shit

cat .shit | grep -v '^#' | sort > shit2

cmd="diff -q -w shit1 shit2"
${cmd}
checkerror
