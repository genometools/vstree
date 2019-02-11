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

echo "------------ $0 -----------------"

ATindex=atindex
SWindex=swindex
cmd="Makeindex.sh ${ATindex} ${SWindex}"
${cmd}
checkerror

cleancl.sh
cmd="./vmatch.x -l 100 -dbcluster 50 50 /tmp/cluster (1,0) -s ${ATindex}"
${cmd} | grep -v '^# args=' > shit1
checkerror

cmd="diff -qw shit1 Testdir/ClusterNum"
${cmd}
checkerror

find /tmp/cluster.*.match -exec Deletepath.sh '{}' \;

find /tmp/cluster.* -exec wc -c '{}' \; | sort -n > shit1

sort -n Testdir/ClusterLS > shit2

cmd="diff -qw shit1 shit2"
${cmd}
checkerror

for filename in `ls /tmp/cluster.*.match`
do
  cmd="./vmatchselect.x -s $filename"
  ${cmd} > /dev/null
  checkerror
done

cmd="./vmatch.x -l 100 ${ATindex}"
${cmd} > tmp.match
checkerror

cmd="./vmatchselect.x -dbcluster 50 50 /tmp/cluster (1,0) -s tmp.match"
${cmd} | grep -v '^# args=' >  .shit
checkerror

cat .shit | grep -v '^#' > shit1
grep -v '^#' Testdir/ClusterNum > shit2

cmd="diff -qw shit1 shit2"
${cmd}
checkerror

find /tmp/cluster.*.match -exec Deletepath.sh '{}' \;

find /tmp/cluster.* -exec wc -c '{}' \; | sort -n > shit3

sort -n Testdir/ClusterLS.sub > shit4

cmd="diff -qw shit3 shit4"
${cmd}
checkerror

for filename in `ls /tmp/cluster.*.match`
do
  cmd="./vmatchselect.x -s $filename"
  ${cmd} > /dev/null
  checkerror
done

echo "okay: $0"
