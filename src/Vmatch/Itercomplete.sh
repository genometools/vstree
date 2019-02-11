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

echo "----------$0-------------"

rm -f at1MB.prj
seqs=`ls ../testdata/Grumbach/[a-z]*.fna ../testdata/at1MB`
for filename in $seqs
do
  echo $filename
  cmd="./Complete.sh Testdir/Patterns $filename"
  ${cmd}
  checkerror
done

cmd="./vmatch.x -complete -d -q Testdir/LargePat.test ychrIII.fna"
${cmd} > .shit
checkerror

cat .shit | grep -v '^#' > shit

cmd="cmp -s shit Testdir/LargePat.res"
${cmd}
checkerror
rm -f shit

echo "okay: $0"
