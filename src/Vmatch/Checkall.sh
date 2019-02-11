#!/bin/sh

set -x -e

currentdate=`date`

./Iterchecksuper.sh
./Vmcalls.pl VM 11 2 run-dna.query run-dna.ref
./Vmcalls.pl VM 6 1 run-prot.query run-prot.ref
./Vmcalls.pl VM 9 1 run-dna.query run-prot.ref
./Vmcalls.pl VM 15 1 "" run-dna.ref
./Vmcalls.pl OL 9 1 run-dna.query run-prot.ref
./Checkmerge.sh
./Qerror.sh
./Runmcl.sh
./Wessel.sh
./DNAvsProtcheck.sh
./Checksub.sh
./LLNL.sh
./Checktrans.sh
#### Regexp.sh 
./DBnomatch.sh
./Checksonl.sh
./Fmulti.sh
./Showdesc.sh
./Eivind.sh
./String.sh
./Itermap.sh
./Itercomplete.sh
./Detall.sh
./Substr.sh
./Iterqonl.sh
./Iterquery.sh
./Iterunique.sh
./Xdrop.sh
./Iterchain.sh
./Iterapmcheck.sh 10 ../testdata/at1MB
./Iterframe.sh

# the following require other binaries or precomputed indexes.

./CompleteEH.sh
./Checktandem.sh
./Checkcluster.sh
./Large.sh
./Iterest.sh     # depends on estmatch.x
./Iterrep.sh     # depends on oneFE.rand.x, onePE.rand.x and repfind.x
./Iterqueryms.sh  # depends on oneFE.rand.x and rfbrute.x
./Itermum.sh      # depends on mum.x and maxmat3.x
./Iterrepms.sh    # depends on repfind.x and rfbrute.x

echo "Test started at ${currentdate}"
echo "Test successful at `date`"
rm -f .tmp
