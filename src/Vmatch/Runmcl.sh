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

Y3=../testdata/Grumbach/ychrIII.fna

for len in 100 200 300
do
  for exdrop in 1 2 3 4
  do
    cmd="./Cmpmatchcl.sh ${Y3} -exdrop ${exdrop} -l ${len}"
    ${cmd}
    checkerror
  done
done

cmd="cat clout.[0-9]*.[0-9]*.match"
${cmd} | sed -e 's/[a-zA-Z\/\-]*\/ychrIII.fna$/ychrIII.fna/' > clout-all
checkerror
cmd="cat vmclout.[0-9]*.[0-9]*.match"
${cmd} | sed -e 's/[a-zA-Z\/\-]*\/ychrIII.fna$/ychrIII.fna/' > vmclout-all
checkerror
cmd="gunzip -c Testdir/clout-all.gz"
${cmd} > tmp1
checkerror
cmd="cmp -s clout-all tmp1"
${cmd}
checkerror
cmd="cmp -s vmclout-all tmp1"
${cmd}
checkerror
rm -f clout.[0-9]*.[0-9]*.match clout-all
rm -f vmclout.[0-9]*.[0-9]*.match vmclout-all
