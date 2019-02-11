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

if test $# -eq 5
then
  case "$5" in 
    "palindromic") mode="-p"
                   ;;
     *)            echo ${usage}
                   exit 1
  esac
else
  if test $# -ne 4
  then
    echo ${usage}
    exit 1
  fi
fi

database=$1
query=$2
len=$3
dval=$4

if test $dval -lt 0
then
  hval=`expr $dval \* -1`
  dstring="-h $hval"
else
  if test $dval -gt 0
  then
    dstring="-e $dval"
  else
    dstring=""
  fi
fi
echo "run $1 $2 $3 $4 $mode"

cmd="./vmatch.x -online $mode -l $len -q $query $dstring ${database}"
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

cmd="./vmatch.x         $mode -l $len -q $query $dstring ${database}"
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
cmd="cmp -s shit[12]"
${cmd} 
checkerror
