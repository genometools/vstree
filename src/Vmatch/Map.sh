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

usage="Usage: $0 [dna|protein] file1 file2 <parms to vmatch>"
if test $# -le 3
then
  echo ${usage}
  exit 1
fi
alpha=$1
file1=$2
file2=$3
shift
shift
shift
if test "$alpha" = "protein"
then
  mkvopt="-pl -protein"
else
  if test "$alpha" = "dna"
  then
    mkvopt="-pl -dna"
  else
    echo ${usage}
    exit 1
  fi
fi

cmd="mkvtreeall.sh -db $file1 ${mkvopt}"
${cmd}
checkerror

cmd="./vmatch.x $* -q $file2 `basename $file1`"
${cmd} > shitlebrit
checkerror

cat shitlebrit | grep -v '^#' | sort > shit1
grep '^#' shitlebrit

cmd="mkvtreeall.sh -db $file2 ${mkvopt}"
${cmd}
checkerror

cmd="./vmatch.x $* -q `basename $file2` `basename $file1`"
${cmd} > shitlebrit
checkerror

cat shitlebrit | grep -v '^#' | sort > shit2
grep '^#' shitlebrit

cmd="cmp -s shit1 shit2"
${cmd}
checkerror
