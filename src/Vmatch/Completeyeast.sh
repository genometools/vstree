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

indexname=yeast1.data

if test $# -ne 1
then
  echo "Usage: $0 <number of patterns>"
  exit 1
fi

if test -f ${indexname}
then
  echo "${indexname} exists"
else
  cmd="makeyeast.sh"
  ${cmd}
  checkerror
fi

if test -f ${indexname}
then
  echo "${indexname}.prj exists"
else
  cmd="mkvtree.sh -db ${indexname} -dna -allout -pl -v"
  ${cmd}
  checkerror
fi

numofpatterns=$1

cmd="../Mkvtree/vsubseqselect.x -snum ${numofpatterns} -minlength 20 -maxlength 64 ${indexname}"
${cmd} > Pattern.list
checkerror

grep -v '^#' Pattern.list | awk '{printf(">\n%s\n", $1);}' > Patternfile

VMATCHSHOWTIMESPACE=on
export VMATCHSHOWTIMESPACE

for percentage in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
do
  if test $percentage -eq 0
  then
    cmd="./vmatch.x -complete -q Patternfile ${indexname}"
    ${cmd}
    checkerror
  else
    for mode in e h
    do
      extra="-${mode} ${percentage}p"
      cmd="./vmatch.x -complete -q Patternfile ${extra} ${indexname}"
      ${cmd}
      checkerror
    done
  fi
done

rm -f Pattern.list Patternfile
