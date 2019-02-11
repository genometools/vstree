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
  echo "Usage: $0 <inputfile> [dna|prot|none]"
  exit 1
fi

if test ! -f ./mkvcmp.dbg.x
then
  exit 0
fi

alldemand="-lcp -suf -ois -tis -bwt -bck -sti1 -skp"
for end in ssp llv skp al1 al2 des prj lcp suf tis bwt bck sti sti1
do
  rm -f all.$end
done
case "$2" in
  "dna")  cmd="./mkvcmp.dbg.x -dna -db $1 -pl ${alldemand} -indexname all"
          ${cmd}
          checkerror
          ;;
  "prot") cmd="./mkvcmp.dbg.x -protein -db $1 -pl ${alldemand} -indexname all"
          ${cmd}
          checkerror
          ;;
  "none") cmd="./mkvcmp.dbg.x -db $1 -pl 2 ${alldemand} -indexname all"
          ${cmd}
          checkerror
          ;;
  *)      echo "$0: Illegal second parameter \"$2\""
          exit 1
esac
