#!/bin/sh
touch data
echo "DATE 29.11.2000" >> data
echo "MACHINE SUN400Mhz" >> data
echo "COMPILER gcc" >> data
echo "PROG vmatch20" >> data
echo "PROG vmatch60" >> data
echo "PROG vmatch100" >> data
echo "PROG vmatch150" >> data
echo "PROG vmatch200" >> data
echo "PROG vmatch400" >> data
echo "ITERATIONS 1" >> data
cat files | grep -v 'sprot.fas' >> data
vmdata.sh SUN400Mhz vm.res >> data
${AWK} -f data2tex.awk data > vm.tmp.tex
