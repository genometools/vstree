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

Y3=../testdata/Grumbach/ychrIII.fna
AT=../testdata/at1MB
U8=../testdata/U89959.fna

cmd="mkvtreeall.sh -db ${Y3} -v -dna -pl 7"
${cmd}
checkerror

cmd="mkvtreeall.sh -db ${AT} -v -dna -pl 7"
${cmd}
checkerror

for extra in "" "-p" "-absolute" "-p -absolute"
do
  args="${extra} -online -l 15 -noevalue"

  cmd="./vmatch.x ${args} -q Testdir/Emblfile.fna ychrIII.fna"
  ${cmd} > .shit
  checkerror

  cat .shit | grep -v '^#' | sort > shit1

  cmd="./vmatch.x ${args} -q Testdir/Emblfile     ychrIII.fna"
  ${cmd} > .shit
  checkerror

  cat .shit | grep -v '^#' | sort > shit2

  cmd="cmp -s shit1 shit2"
  ${cmd}
  checkerror
done

for extra in "" "-p" "-absolute" "-p -absolute" 
do
  args="${extra} -l 25 -noevalue"

  cmd="./vmatch.x ${args} -q ${U8}                 ychrIII.fna"
  ${cmd} > .shit
  checkerror

  cat .shit | grep -v '^#' | sort > shit1

  cmd="./vmatch.x ${args} -q ${U8} (70000,78000) ychrIII.fna"
  ${cmd} > .shit
  checkerror

  cat .shit | grep -v '^#' > shit2.1

  cmd="./vmatch.x ${args} -q ${U8} (5000,6000)   ychrIII.fna"
  ${cmd} > .shit
  checkerror

  cat .shit | grep -v '^#' > shit2.2
  cat shit2.1 shit2.2 | sort > shit2
  
  cmd="cmp -s shit1 shit2"
  ${cmd} 
  checkerror

  args="${extra} -l 25 -noevalue"
  
  cmd="./vmatch.x ${args} -q ${U8}                  at1MB"
  ${cmd} > .shit
  checkerror

  cat .shit | grep -v '^#' | sort > shit1

  cmd="./vmatch.x ${args} -q ${U8} (71000,106000) at1MB"
  ${cmd} > .shit
  checkerror

  cat .shit | grep -v '^#' | sort > shit2
  
  cmd="cmp -s shit1 shit2"
  ${cmd}
  checkerror
done
rm -f .shit
