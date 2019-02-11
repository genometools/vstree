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
usage="$0 <filename> <length> <distance> [palindromic|both]"
vmode=""
rmode=""
if test $# -eq 4
then
  case "$4" in 
    "palindromic") vmode="-p"
                   rmode="-p"
                   ;;
    "both")        vmode="-d -p"
                   rmode="-f -p"
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

cmd="mkvtreeall.sh -db $1 -pl -dna"
${cmd}
checkerror

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

indexname=`basename $1`
formatopts="-absolute -noscore -noidentity"
cmd="./vmatch.x $vmode ${formatopts} -l $2 $dist $indexname"
${cmd} > .shit
checkerror

cat .shit |\
    grep -v '^#' |\
    sed -e 's/[ ][ ]*/ /g' -e 's/^[ ]//' |\
    sort -u > shit1

cmd="repfind.x $rmode $dist -l $2 $1"
${cmd} > .shit
checkerror

cat .shit |\
    grep -v '^#' |\
    sed -e 's/F/D/' -e 's/[ ][ ]*/ /g' -e 's/^[ ]//' |\
    sort -u > shit2

cmd="diff -w -q shit1 shit2"
${cmd}
checkerror
