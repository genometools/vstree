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


inputfile=../testdata/at1MB
indexname=at1MB
minlen=20
ovl=10
queryfile=../testdata/U89959.fna

MKVOUT="-bwt -lcp -suf -bck -tis -ois -sti1"
MATCHFILE=tmp.match
SF="-selfun mergematches-dbg.so"

cmd="mkvtree.sh -indexname ${indexname} -db ${inputfile} -dna -pl -v ${MKVOUT}"
${cmd}
checkerror

cmd="./vmatch.x -l ${minlen} -v ${indexname}"
${cmd} > ${MATCHFILE}
checkerror

cmd="./vmatchselect.x -v ${SF} ${ovl} ${MATCHFILE}"
${cmd}
checkerror

cmd="./vmatch.x -l ${minlen} -v -q ${queryfile} ${indexname}"
${cmd} > ${MATCHFILE}
checkerror

cmd="./vmatchselect.x -v ${SF} ${ovl} ${MATCHFILE}"
${cmd}
checkerror

cmd="./vmatch.x -l ${minlen} -v ${SF} ${ovl} ${indexname}"
${cmd}
checkerror

cmd="./vmatch.x -l ${minlen} -v ${SF} ${ovl} -q ${queryfile} ${indexname}"
${cmd}
checkerror
