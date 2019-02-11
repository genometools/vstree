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

usage="$0 <filename> <length> <distance> [palindromic]"
mode=""
if test $# -eq 4
then
  case "$4" in 
    "palindromic") mode="-p"
                   ;;
     *)            echo $usage
                   exit 1
  esac
else
  if test $# -ne 3
  then
    echo $usage
    exit 1
  fi
fi
if test -f "`basename $1`.bwt"
then
  echo "`basename $1.bwt` exists"
else
  cmd="mkvtreeall.sh -db $1 -pl -dna"
  ${cmd}
  checkerror
fi

if test $3 -lt 0
then
  hval=`expr $3 \* -1`
  vmdist="-h $hval -allmax"
  rfbrutedist="-h $hval"
else
  if test $3 -gt 0
  then
    vmdist="-e $3 -allmax"
    rfbrutedist="-e $3"
  else
    vmdist=""
    rfbrutedist=""
  fi
fi

formatopts="-nodist -noevalue -noscore -noidentity -absolute"
cmd="./vmatch.x $mode ${formatopts} -l $2 $vmdist $1"
${cmd} > .shit
checkerror

cat .shit | grep -v '^#' |\
            sed -e 's/[ ][ ]*/ /g' -e 's/^[ ]//' |\
            sort -u > shit1

cmd="rfbrute.x $mode -l $2 $rfbrutedist $1"
${cmd} > .shit
checkerror

sort -u .shit | sed -e 's/F/D/' > shit2

cmd="cmp -s shit1 shit2"
${cmd}
checkerror
