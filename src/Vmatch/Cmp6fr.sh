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
  echo "Usage: $0 <dna-file> <minlength>"
  exit 1
fi

dnafile=$1
minlength=$2
noopt="-absolute -noevalue -noidentity -noscore -nodist"

cmd="mkvtree.sh -indexname dnaidx -db $dnafile -allout -dna -v -pl"
${cmd}
checkerror

cmd="./vmatch.x -l ${minlength} ${noopt} dnaidx"
${cmd} > .shit
checkerror

cat .shit | grep -v '^#' > tmp1

cmd="../Mkvtree/mkdna6idx.x -indexname dnaidx6fr -db $dnafile -tis -ois"
${cmd} 
checkerror

cmd="./vmatch.x -l ${minlength} ${noopt} dnaidx6fr.6fr"
${cmd} > .shit
checkerror

cat .shit | grep -v '^#' > tmp2

cmd="./CheckFrame.pl tmp1 tmp2"
${cmd}
checkerror
