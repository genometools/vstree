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

echo "run rblastm.x"
rm -f tmp1
echo "#  Matrix made by matblas from blosum62.iij" > tmp1
echo "#  * column uses minimum score" >> tmp1
echo "#  BLOSUM Clustered Scoring Matrix in 1/2 Bit Units" >> tmp1
echo "#  Blocks Database = /data/blocks_5.0/blocks.dat" >> tmp1
echo "#  Cluster Percentage: >= 62" >> tmp1
echo "#  Entropy =   0.6979, Expected =  -0.5209" >> tmp1

cmd="./rblastm.x BLOSUM62"
${cmd} >> tmp1
checkerror

cmd="cmp -s tmp1 ${SCOREMATRIXDIR}/BLOSUM62"
${cmd}
checkerror
