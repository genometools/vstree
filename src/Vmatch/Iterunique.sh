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
echo "----------$0-------------"

seqs=`ls ${GRUMBACH}/[a-z]*.fna`
AT=../testdata/at1MB
U8=../testdata/U89959.fna

for filename in $seqs
do
  echo "`basename $filename`"
  cmd="./UniqueDB.sh 200 20 0 $filename "
  ${cmd}
  checkerror
  cmd="./UniqueDB.sh 200 30 -1 $filename "
  ${cmd}
  checkerror
  cmd="./UniqueDB.sh 100 40 -2 $filename "
  ${cmd}
  checkerror
  cmd="./UniqueDB.sh 200 30 1 $filename "
  ${cmd}
  checkerror
  cmd="./UniqueDB.sh 100 40 2 $filename "
  ${cmd}
  checkerror
done

cmd="./UniqueQ2.sh 100 20 0 $AT ${U8}"
${cmd}
checkerror
cmd="./UniqueQ2.sh 100 30 -1 $AT ${U8}"
${cmd}
checkerror
cmd="./UniqueQ2.sh 100 40 -2 $AT ${U8}"
${cmd}
checkerror
cmd="./UniqueQ2.sh 100 30 1 $AT ${U8}"
${cmd}
checkerror
cmd="./UniqueQ2.sh 100 40 2 $AT ${U8}"
${cmd}
checkerror

cmd="./UniqueQ1.sh 100 20 0 $AT ${U8}"
${cmd}
checkerror
cmd="./UniqueQ1.sh 100 30 -1 $AT ${U8}"
${cmd}
checkerror
cmd="./UniqueQ1.sh 100 40 -2 $AT ${U8}"
${cmd}
checkerror
cmd="./UniqueQ1.sh 100 30 1 $AT ${U8}"
${cmd}
checkerror
cmd="./UniqueQ1.sh 100 40 2 $AT ${U8}"
${cmd}
checkerror

