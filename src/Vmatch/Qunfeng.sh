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

VMATCH=./vmatch.x
MKVTREE=mkvtree.sh

printf ">test1\nctgggagcctaaca\n>test2\nCTGGGAGCCTAACA\n" > Qunfeng.query
printf ">DB\nCTGGGAGCCTAACA\n" > Qunfeng.ref

cmd="${MKVTREE} -dna -tis -ois -db Qunfeng.ref"
${cmd}
checkerror

cmd="${VMATCH} -online -qmaskmatch X -q Qunfeng.query -showdesc 60 -l 10 -identity 100 Qunfeng.ref" 
${cmd} > tmp1
checkerror

cmd="${MKVTREE} -dna -allout -pl -db Qunfeng.ref"
${cmd}
checkerror

cmd="${VMATCH} -qmaskmatch X -q Qunfeng.query -showdesc 60 -l 10 -identity 100 Qunfeng.ref"
${cmd} > tmp2
checkerror

cmd="cmp -s tmp1 tmp2"
${cmd}
checkerror

rm -f tmp1 tmp2 Qunfeng.query Qunfeng.ref
