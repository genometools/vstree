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

cmd="mkvtreeall.sh -indexname atall -dna -pl 7 -db ../testdata/at0[0-9]"
${cmd}
checkerror

cmd="./vmatch.x -l 300 -f atall"
${cmd} > .shit
checkerror

cat .shit | grep -v '^#' |\
            sed -e 's/[A-Za-z0-9\/\.\.]*\(at0[0-9]\)/\1/g' > shit1
cmd="cmp -s shit1 Testdir/atall300"
${cmd}
checkerror

echo "okay: $0"
