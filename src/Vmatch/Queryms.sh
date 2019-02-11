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

usage="$0 <dbfile> <queryfile> <length> <distance> [palindromic]"
mode=""

if test $# -eq 5
then
  case "$5" in 
    "palindromic") mode="-p"
                   ;;
     *)            echo $usage
                   exit 1
  esac
else
  if test $# -ne 4
  then
    echo $usage
    exit 1
  fi
fi

db=$1
query=$2
len=$3
dval=$4

if test -f "`basename ${db}`.bwt"
then
  echo "`basename ${db}.bwt` exists"
else
  cmd="mkvtreeall.sh -db ${db} -pl -dna"
  ${cmd}
  checkerror
fi

if test $dval -lt 0
then
  hval=`expr $dval \* -1`
  vmdist="-h $hval -allmax"
  rfbrutedist="-h $hval"
else
  if test $dval -gt 0
  then
    vmdist="-e $dval -allmax"
    rfbrutedist="-e $dval"
  else
    vmdist=""
    rfbrutedist=""
  fi
fi

noopts="-nodist -noevalue -noscore -noidentity -absolute"
cmd="./vmatch.x $mode ${noopts} -l $len ${vmdist} -q $query ${db}"
${cmd} > .shit
checkerror

cat .shit | grep -v '^#' |\
            sed -e 's/[ ][ ]*/ /g' -e 's/^[ ]//' |\
            sort -u > shit1

cmd="rfbrute.x $mode -l $len ${rfbrutedist} ${db} $query"
${cmd} > .shit
checkerror

cat .shit | sort -u | sed -e 's/F/D/' > shit2

cmd="diff -q -w shit1 shit2"
${cmd}
checkerror
