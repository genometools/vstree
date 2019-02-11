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

if test $# -ne 2
then
  echo "Usage: $0 <patternfile> <textfile>"
  exit 1
fi
patternfile=$1
textfile=$2
indexname=Tmp.index
queryfile=.tmp.fasta
cmd="makefasta.x ${patternfile}"
${cmd} > ${indexname}
checkerror

mkvtreeopt="-db ${indexname} -dna -pl 1 -tis -ois -suf -lcp -bck"
cmd="mkvtree.sh ${mkvtreeopt}"
${cmd}
checkerror

cmd="../Mkvtree/mklsf.x ${indexname}"
${cmd}
checkerror

cmd="makefasta.x ${textfile}"
${cmd} > ${queryfile}
checkerror

vmatchopt="-l 1 -qspeedup 4 -q ${queryfile} ${indexname}"
cmd="./vmatch.x ${vmatchopt}"
${cmd} > .tmp
checkerror

grep -v '^#' .tmp > tmp1

DEBUGLEVEL=2
export DEBUGLEVEL
cmd="mstat.dbg.x ${patternfile} ${textfile}"
${cmd} > .tmp
checkerror

sed -e '/^0 0$/d' -e '/^#/d' -e '/^$/d' .tmp > tmp2
cmd="cmp -s tmp1 tmp2"
${cmd}
checkerror
