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

cmd="../Mkvtree/mkdna6idx.x -indexname dnaidx6fr -db $dnafile -tis -ois"
${cmd} 
checkerror

cmd="./vmatch.x -s 60 -l ${minlength} ${noopt} dnaidx6fr.6fr"
${cmd} > .shit
checkerror

grep '^Sbjct' .shit | awk '{printf(">\n%s\n", $2);}' > shit1

cmd="trdna.pl shit1"
${cmd} > shit1.trans
checkerror

cmd="./Verifytrans.pl shit1.trans"
${cmd}
checkerror
