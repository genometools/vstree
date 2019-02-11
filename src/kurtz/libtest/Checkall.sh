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

cmd="strmfna.x 70 ${AT}"
${cmd} > tmp1
checkerror

cmd="cmp -s tmp1 ${AT}"
${cmd}
checkerror

cmd="strmfna.x 64 ${SW}"
${cmd} > tmp1
checkerror

cmd="cmp -s tmp1 ${SW}"
${cmd}
checkerror

cmd="queuetest.x 1000"
${cmd} > tmp4
checkerror

cmd="Checkreadblast.sh"
${cmd}
checkerror

cmd="cmp -s tmp4 queue1000"
${cmd}
checkerror

cmd="checkredrange.x 300 10000"
${cmd}
checkerror

for qvalue in 14 13 12 11
do
  cmd="Iterfly.sh ${qvalue}"
  ${cmd}
  checkerror
done

cmd="mkvtree -indexname ecoli.fna -db ${ECOLI1} -dna -tis"
${cmd}
checkerror

for minlength in 20 30 38 45 87 93 100 150 200 250 300 400 500 600  
do
  for maxlength in 30 38 45 87 93 100 150 200 250 300 400 500 600
  do
    if test ${minlength} -le ${maxlength}
    then
      cmd="checklongdist.x UM ${minlength} ${maxlength} 1000 ecoli.fna"
      ${cmd}
      checkerror
    fi
  done
done

for numofregions in 20 30 40 60 100 200 1000
do
  cmd="regioncheck.x 10000 ${numofregions}"
  ${cmd}
  checkerror
done

FASTAFILE=ATprefix
DEBUGLEVEL=0
export DEBUGLEVEL

ATindex=atindex
SWindex=swindex

cmd="suffixprefix.x 6 ${ATK2}"
${cmd}
checkerror

cmd="Checkcodon.sh"
${cmd}
checkerror

cmd="Makeindex.sh ${ATindex} ${SWindex}"
${cmd}
checkerror

cmd="runVmatchprog.sh mkcld ${ATindex}"
${cmd}
checkerror

cmd="mkcld ${SWindex}"
${cmd}
checkerror

# The following is too slow. That is why we do not execute it.

#cmd="verifycld.x ${ATindex}"
#${cmd}
#checkerror

#cmd="verifycld.x ${SWindex}"
#${cmd}
#checkerror


cmd="Checkdb.sh U89959.gb"
${cmd}
checkerror

cmd="./readdb.x Emblfile"
${cmd} > shit3
checkerror

grep -v '^#' shit3 > shit1

cmd="cmp -s shit1 Emblfile.fna"
${cmd}
checkerror

cmd="./readdb.x U89959.gb"
${cmd} > shit3
checkerror

grep -v '^#' shit3 > shit1

cmd="cmp -s shit1 U89959.fna"
${cmd}
checkerror

cmd="./rmulfast.x 70 $FASTAFILE False"
${cmd} > shit3
checkerror

grep -v '^#' shit3 > shit1

cmd="cmp -s shit1 $FASTAFILE"
${cmd} 
checkerror 

cmd="./addmulti.x shit1"
${cmd} > shit2
checkerror

cmd="cmp -s shit1 shit2"
${cmd}
checkerror

cmd="./checkcluster.x 1000 100"
${cmd}
checkerror

cmd="./checkcluster.x 10000 1000"
${cmd}
checkerror

cmd="./checkcluster.x 10000 5000"
${cmd}
checkerror

cmd="./checkxalign.x -a ab 12"
${cmd}
checkerror

cmd="./checkgalign.x -a ab 12"
${cmd}
checkerror

cmd="./checkEvalue.x 0.25 100"
${cmd}
checkerror

cmd="./checkEvalue.x 0.05 100"
${cmd}
checkerror

cmd="Checkrsym.sh"
${cmd}
checkerror

cmd="checkmygzip.sh ."
${cmd}
checkerror

cmd="runredblack.x"
${cmd}
checkerror

cmd="Rundict.sh"
${cmd}
checkerror

cmd="checkqsort.x MC 1000000 1000000"
${cmd}
checkerror

cmd="checkqsort.x LC 1000000 1000000"
${cmd}
checkerror

cmd="Itercheckmini.sh"
${cmd}
checkerror

echo `date`
