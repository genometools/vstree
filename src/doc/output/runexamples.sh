#!/bin/sh
checkerrorcode() {
  if test $? -ne 0
  then
    echo "Error"
    exit 1
  else
    echo "Okay"
  fi
}

VMATCH=../../Vmatch/vmatch.x
MKVTREE=mkvtree
VSI=../../Mkvtree/vseqinfo.x

echo "make mkv1.out"
echo "$ mkvtree -db atEST -dna -pl 7 -allout -v" > mkv1.out
       ${MKVTREE} -db atEST -dna -pl 7 -allout -v >> mkv1.out
checkerrorcode

echo "make mkv2.out"
echo "$ mkvtree -v -pl 8 -db EcoliO157H7 -dna -bck -suf -lcp -tis"  > mkv2.out
       ${MKVTREE} -v -pl 8 -db EcoliO157H7 -dna -bck -suf -lcp -tis  >> mkv2.out
checkerrorcode

echo "make vm1.out"
echo "$ vmatch -v -l 350 atEST" > vm1.out
       ${VMATCH} -v -l 350 atEST >> vm1.out
checkerrorcode

echo "make vm1sel.out"
echo "$ vmatch -v -l 350 -selfun sel392.so atEST" > vm1sel.out
       ${VMATCH} -v -l 350 -selfun ./sel392.so.${CONFIGGUESS} atEST >> vm1sel.out
checkerrorcode

echo "make vm2.out"
echo "$ vmatch -p -l 200 -h 1 -showdesc 10 -s 60 atEST" > vm2.out
       ${VMATCH} -p -l 200 -h 1 -showdesc 10 -s 60 atEST >> vm2.out
checkerrorcode

echo "make vm22.out"
echo "$ vmatch -d -p -l 300 -e 4 -showdesc 10 -best 10 atEST" > vm22.out
       ${VMATCH} -d -p -l 300 -e 4 -showdesc 10 -best 10 atEST >> vm22.out
checkerrorcode

echo "make vm3.out"
echo "$ vmatch -l 250 -e 6 -q U89959 -s 70 atEST"  > vm3.out
       ${VMATCH} -l 250 -e 6 -q U89959 -s 70 atEST  >> vm3.out
checkerrorcode

echo "make vm4.out"
echo "$ vmatch -d -p -online -complete -q ORF -s -showdesc 10 atEST" > vm4.out
       ${VMATCH} -d -p -online -complete -q ORF -s -showdesc 10 atEST >> vm4.out
checkerrorcode

${MKVTREE} -v -dna -pl 7 -bwt -tis -lcp -suf -indexname bac -db mgen.fna -q mpneu.fna
checkerrorcode

echo "make vm5.out"
echo "$ vmatch -mum -l 20 bac" > vm5.out
       ${VMATCH} -mum -l 20 bac >> vm5.out
checkerrorcode

echo "make vm6.out"
echo "$ vmatch -dbnomatch 700 -l 25 -e 1 atEST" > vm6.out
       ${VMATCH} -dbnomatch 700 -l 25 -e 1 atEST >> vm6.out
checkerrorcode

echo "make vm7.out"
echo "$ vmatch -qnomatch 10000 -q EcoliK12 -l 18 EcoliO157H7" > vm7.out
      ${VMATCH}  -qnomatch 10000 -q EcoliK12 -l 18 EcoliO157H7 >> vm7.out
checkerrorcode

echo "make vm8.out"
echo "$ vmatch -dbnomatch 10000 -q EcoliK12 -l 18 EcoliO157H7" > vm8.out
      ${VMATCH}  -dbnomatch 10000 -q EcoliK12 -l 18 EcoliO157H7 >> vm8.out
checkerrorcode

echo "make vseqinfo0.out"
echo "$ vseqinfo atEST | head -n 6 | cut -b 1-73" > vseqinfo0.out
      $VSI atEST | head -n 6 | cut -b 1-73       >> vseqinfo0.out
checkerrorcode

echo "make vseqinfo1.out"
echo "$ vseqinfo bac" > vseqinfo1.out
      $VSI bac       >> vseqinfo1.out
checkerrorcode

for filename in `ls *.out`
do
  sed -e 's/^\/[A-Za-z0-9][A-Za-z0-9]*\/vstree\/src\/vstree\/src\/doc\/output\///' $filename > .tmp
  mv .tmp $filename
done
