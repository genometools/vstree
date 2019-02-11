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

OUTOPTIONS="-bwt -suf -lcp -ois -tis -skp -bck"

if test $# -ne 2
then
  echo "Usage: $0 <numofpatterns> <inputfile>"
  exit 1
fi

numofpatterns=$1
inputfile=$2

cmd="mkvtree.sh -v -indexname apmindex -db ${inputfile} -pl -dna ${OUTOPTIONS}"
${cmd}
checkerror

${cmd} > .tmp
checkerror

awk '/^[^#]/ {printf(">\n%s\n",$1);}' .tmp > Pattern.tmp

for pattern in `../Mkvtree/vsubseqselect.x -minlength 17 -maxlength 32 -snum ${numofpatterns} apmindex | grep -v '^#'`
do
  echo "$pattern"
  for dist in 1 5p 5b 10p 10b 15p 15b 20p 20b 25b 25p 30p 30b 
  do
    cmd="./Apmcheck.sh ${dist} $pattern apmindex"
    ${cmd}
    checkerror
    cmd="./Hamcheck.sh ${dist} $pattern apmindex"
    ${cmd}
    checkerror
  done
done

#rm -f Pattern.tmp
