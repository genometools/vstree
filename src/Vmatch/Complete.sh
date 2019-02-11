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

if test $# -ne 2
then
  echo "Usage: $0 patternfile filename"
  exit 1
fi

if test -f "`basename $2`.prj"
then
  echo "`basename $2`.prj exists"
else
  cmd="mkvtreeall.sh -db $2 -dna -pl"
  ${cmd}
  checkerror
fi

indexname=`basename $2`

cmd="./vmatch.x -d         -complete -q $1 $indexname" > .shit
${cmd}
checkerror

cat .shit | grep -v '^#' | sort > shit1
cmd="./vmatch.x -d -online -complete -q $1 $indexname" > .shit
${cmd}
checkerror

cat .shit | grep -v '^#' | sort > shit2

cmd="cmp -s shit[12]"
${cmd}
checkerror
rm -f .shit

echo "okay: $0"
