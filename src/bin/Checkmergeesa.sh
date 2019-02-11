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

if test $# -eq 2
then
  program2check=$1
  numofsequences=$2
  minlen=""
else
  if test $# -eq 3
  then
    program2check=$1
    numofsequences=$2
    minlen=$3
  else
    echo "Usage: $0 <program2check> <numofsequences> [minlen]"
    exit 1
  fi
fi

#VALGRIND=valgrind.sh

TMP=tmp.sequence

cmd="mkvtree.sh  -db ${AT} -dna -pl -ois -tis"
${cmd}
checkerror

cmd="../Mkvtree/vseqselect.x -randomnum ${numofsequences} at1MB"
${cmd} > ${TMP}
checkerror

bigindex=bigindex

cmd="mkvtree.sh  -indexname ${bigindex} -db ${TMP} -dna -pl -ois -tis -suf -bck -bwt -lcp"
${cmd}
checkerror

#rm -f tmp-*

cmd="splitmultifasta.pl tmp 60 0 ${TMP}"
${cmd}
checkerror

filelist=`ls tmp-*`

indexlist=""
for filename in ${filelist}
do
  indexname=`echo ${filename} | sed -e 's/tmp/idx/'`
  indexlist="${indexlist} ${indexname}"
  cmd="mkvtree.sh  -indexname ${indexname} -db ${filename} -dna -pl -bck -tis -suf -lcp"
  ${cmd}
  checkerror
done
outindex=outindex

cmd="${program2check} ${minlen} ${outindex} ${bigindex} ${indexlist}"
${cmd}
checkerror

cleanpp.sh
rm -f ${filelist} tmp.sequence
