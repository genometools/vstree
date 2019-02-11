#!/bin/sh

checkerror() {
if test $? -ne 0
then
  echo "failure: ${cmd}"
  exit 1
fi
}

USAGE="Usage: $0 <inputfile> [dna|protein]"

if test $# -ne 2
then
  echo ${USAGE}
  exit 1
fi
inputfile=$1
seqtype=$2
if test "${seqtype}" = "dna"
then
  seqopt="-dna"
else
  if test "${seqtype}" = "protein"
  then
    seqopt="-protein"
  else
    echo ${USAGE}
    exit 1
  fi
fi

MKVTREEOPTS="-pl -v -suf -lcp -skp -tis"
IDX=profidx
cmd="mkvtree.sh -db ${inputfile} -indexname ${IDX} ${seqopt} ${MKVTREEOPTS}"
${cmd}
checkerror
cmd="profiler.dbg.x SLABD 10 10 20 ${IDX}"
${cmd}
checkerror
