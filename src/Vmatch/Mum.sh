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

if test $# -ne 3
then
  echo "Usage: $0 file1 file2 leastlength"
  exit 1
fi
mkvopts="-pl -dna"
database=$1
query=$2
leastlength=$3

cmd="mum.x -p -l $leastlength $database $query"
${cmd} > .shit
checkerror

cat .shit | sed -e 's/[ ][ ]*/ /g' -e '/^#/d' |\
            awk '{printf("%d %d %d\n",$1-1,$2-1,$3);}' |\
            sort > shit1

cmd="mkvtreeall.sh -indexname all ${mkvopts} -db $database -q $query"
${cmd}
checkerror

cmd="./vmatch.x -nodist -noevalue -l $leastlength -mum all"
${cmd} > .shit
checkerror

cat .shit | grep -v '^# args' |\
            awk '{printf("%d %d %d\n",$3,$7,$1);}' |\
            sort > shit2

cmd="cmp -s shit1 shit2"
${cmd}
checkerror

cmd="mkvtreeall.sh -indexname db ${mkvopts} -db $database"
${cmd}
checkerror

cmd="./vmatch.x -nodist -noevalue -l $leastlength -mum -q $query db"
${cmd} > .shit
checkerror

cat .shit | grep -v '^# args' |\
            awk '{printf("%d %d %d\n",$3,$7,$1);}' |\
            sort > shit3

cmd="cmp -s shit2 shit3"
${cmd}
checkerror

wc shit1

cmd="maxmat3.x -mumcand -l $leastlength $database $query"
${cmd} > .shit
checkerror

cat .shit | grep -v '^[#>]' |\
            awk '{printf("%d %d %d\n",$1-1,$2-1,$3);}' |\
            sort > shit2

cmd="./vmatch.x -nodist -noevalue -l $leastlength -mum cand -q $query db"
${cmd} > .shit
checkerror

cat .shit | grep -v '^# args' |\
            awk '{printf("%d %d %d\n",$3,$7,$1);}' |\
            sort > shit3

cmd="cmp -s shit2 shit3"
${cmd}
checkerror
