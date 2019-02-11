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

VMOPTIONS="-l 20 -seedlength 15 -exdrop 2 -q"

key=gremme1

for key in gremme1 gremme2
do
  for mode in -d -p 
  do
    cmd="mkvtree.sh -dna -db Testdir/${key}seq.fna -pl -allout -indexname ${key}"
    ${cmd}
    checkerror
    cmd="./Checkonline.sh ${VMOPTIONS} Testdir/${key}${mode}.fna ${mode} ${key}"
    ${cmd}
    checkerror
  done
done
