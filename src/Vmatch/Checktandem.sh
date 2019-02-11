#!/bin/sh
checkerror() {
  if test $? -ne 0
  then
    echo "failure: ${cmd}"
    exit 1
  else
    echo "Okay"
  fi
}

ATindex=atindex
SWindex=swindex

cmd="Makeindex.sh ${ATindex} ${SWindex}"
${cmd}
checkerror

cmd="./vmatch.x -l 40 -tandem ${ATindex}"
${cmd} | grep -v '^#' > shit
checkerror

cmd="cmp -s shit Testdir/Tandem40AT"
${cmd}
checkerror

echo "okay: $0"
