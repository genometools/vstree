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
DEBUGLEVEL=1
export DEBUGLEVEL

ATK=../testdata/at100K1
AT=../testdata/at1MB
SWK=../testdata/swiss10K
SW=../testdata/swiss1MB
GRUMBACHSEQ=../testdata/Grumbach

if test "XX${ATK}" = "XX"
then
  echo "environment ATK not defined"
  exit 1
fi
if test "XX${AT}" = "XX"
then
  echo "environment AT not defined"
  exit 1
fi
if test "XX${SWK}" = "XX"
then
  echo "environment SWK not defined"
  exit 1
fi
if test "XX${SW}" = "XX"
then
  echo "environment SW not defined"
  exit 1
fi
if test "XX${GRUMBACHSEQ}" = "XX"
then
  echo "environment GRUMBACHSEQ not defined"
  exit 1
fi

for filename in $ATK $AT $SWK $SW
do
  if test -f "$filename"
  then
    echo "file $filename exists"
  else
    echo "file $filename does not exist"
    exit 1
  fi
done

for filename in $GRUMBACHSEQ
do
  if test -d "$filename"
  then
    echo "directory $filename exists"
  else
    echo "directory $filename does not exist"
    exit 1
  fi
done

if test -f ./mkrcidx.dbg.x
then
  cmd="./mkrcidx.dbg.x -indexname at1MBrc -db ${AT}"
  ${cmd}
  checkerror
fi

if test -f ./mkdna6idx.dbg.x
then
  cmd="./mkdna6idx.dbg.x -indexname at1MB6fr -db ${AT}"
  ${cmd}
  checkerror
  cmd="./mkdna6idx.dbg.x -db ${AT} -smap TransProt11"
  ${cmd}
  checkerror
  cmd="./vseqinfo.x at1MB"
  ${cmd}
  checkerror
fi

cmd="./mkvtree.x -indexname alldb -db `ls DATA/DBprot[123]` -tis -ois -protein"
${cmd}
checkerror

cmd="./mkvtree.x -indexname alldbgz -db `ls DATA/DBprot[123].gz` -tis -ois -protein"
${cmd}
checkerror

for suffix in tis ois des sds al1
do
  echo "compare alldb.${suffix} alldbgz.${suffix}"
  cmd="cmp -s alldb.${suffix} alldbgz.${suffix}"
  checkerror
done

echo "-------------------run Callall.sh----------------------"
cmd="Callall.sh"
${cmd}
checkerror

programlist=""
if test -f ./mkvtree.dbg.x
then
  programlist="${programlist} ./mkvtree.dbg.x"
fi

if test -f ./mkvram.x
then
  programlist="${programlist} ./mkvram.x"
fi

for PROG in ${programlist}
do
  cmd="echo \"1\" | ../bin/cmpCal.sh $PROG -pl 1 -db"
  ${cmd}
  checkerror

  cmd="echo \"4\" | ../bin/cmpCal.sh $PROG -dna -pl -db"
  ${cmd}
  checkerror

  cmd="echo \"4\" | ../bin/cmpCal.sh $PROG -dna -pl 1 -db"
  ${cmd}
  checkerror

  echo "---------------$PROG Arabidopsis small----------------------"
  cmd="$PROG -dna -db $ATK"
  ${cmd}
  checkerror

  echo "---------------$PROG Arabidopsis large----------------------"
  cmd="$PROG -smap TransDNA -db $AT"
  ${cmd}
  checkerror

  echo "-------------------$PROG Emblfile----------------------"
  cmd="$PROG -smap TransDNA -db DATA/Emblfile"
  ${cmd}
  checkerror
  
  plen=1
  while test $plen -le 8
  do
    echo "-----------$PROG Arabidopsis -pl $plen large---------------------"
    cmd="$PROG -dna -pl $plen -db $AT"
    ${cmd}
    checkerror

    plen=`expr $plen + 1`
  done

  plen=1
  while test $plen -le 6
  do
    echo "-----------$PROG Arabidopsis -pl $plen small--------------------"
    cmd="$PROG -smap TransDNA -pl $plen -db $ATK"
    ${cmd}
    checkerror

    plen=`expr $plen + 1`
  done

  plen=1
  while test $plen -le 2
  do
    echo "------------$PROG Emblfile----------------------------------------"
    cmd="$PROG -dna -pl $plen -db DATA/Emblfile"
    ${cmd}
    checkerror

    plen=`expr $plen + 1`
  done
  
  echo "--------------$PROG Swissprot--------------------------------------"
  cmd="$PROG -protein -db $SW"
  ${cmd}
  checkerror
  
  plen=1
  while test $plen -le 4
  do
    echo "---------------$PROG Swissprot -pl $plen---------------"
    cmd="$PROG -smap TransProt21 -pl $plen -db $SW"
    ${cmd}
    checkerror
    plen=`expr $plen + 1`
  done

  echo "------------$PROG all from ${GRUMBACHSEQ}---------------"
  cmd="$PROG -pl 8 -dna -db ${GRUMBACHSEQ}/*.seq"
  ${cmd}
  checkerror

  cmd="$PROG -pl -dna -db ${GRUMBACHSEQ}/*.seq"
  ${cmd}
  checkerror
done

for filename in DATA/DBdna1 DATA/DBdna2 DATA/DBdna3 $ATK $AT
do
  echo "------------run CMP.sh $filename dna----------------------"
  cmd="./CMP.sh $filename dna"
  ${cmd}
  checkerror
  if test -f ./mkrcidx.x
  then
    cmd="./mkrcidx.x -db ${filename}"
    ${cmd}
    checkerror
  fi
  cmd="./mkdna6idx.x -db ${filename} -tis -ois -transnum 4 -indexname dna6"
  ${cmd}
  checkerror
done

for filename in DATA/DBprot1 DATA/DBprot2 DATA/DBprot3 $SW $SWK
do
  echo "------------run CMP.sh $filename prot------------------"
  cmd="./CMP.sh $filename prot"
  ${cmd}
  checkerror
done

cmd="./Allwhatsthere.sh"
${cmd}
checkerror
