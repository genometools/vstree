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

U8=../testdata/U89959.fna
SIXU8=../testdata/SIXU89959
SW=../testdata/swiss1MB

usage="Usage: $0 [DNA|Prot] length maxdist"
if test $# -ne 3
then
  echo ${usage}
  exit 1
fi

if test "$1" = "DNA"
then
  echo "$0 DNA $2 $3"
  database=../testdata/at1MB
  query=${U8}
  prefix=7
  smap="-smap ../Mkvtree/TRANS/TransDNA"
  allindex=allDNA
else
  if test "$1" = "Prot"
  then
    echo "$0 Prot $2 $3"
    database=${SW}
    query=${SIXU8}
    prefix=3
    smap="-smap ../Mkvtree/TRANS/TransProt11"
    allindex=allProt
  else
    echo ${usage}
    exit 1
  fi
fi

if test -f ${allindex}.prj
then
  echo "${allindex}.prj exists"
else
  cmd="mkvtreeall.sh -indexname $allindex -v -pl $prefix $smap -db ${database} -q ${query}"
  ${cmd}
  checkerror
fi

if test $3 -ne 0
then
  extra="-h $3"
fi

leastlen=$2
cmd="./vmatch.x -best 10000 -d -l $leastlen $extra $allindex"
${cmd} > .shit
checkerror

cat .shit | sed -e '/^#/d' |\
            awk -f Showm.awk |\
            sed -e 's/ 0+/ /' -e 's/+/ /g' |\
            sort > shit1
cmd="estmatch.x $smap -l $leastlen $extra ${database} ${query}"
${cmd} > .shit
checkerror

cat .shit | sed -e '/^#/d' |\
            awk '{print $1 " " $2 " " $4 " " $5 " " $6}' |\
            sort > shit2
cmd="cmp -s shit1 shit2"
${cmd}
checkerror

echo "okay: $0"
