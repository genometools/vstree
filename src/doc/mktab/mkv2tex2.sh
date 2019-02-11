#!/bin/sh
touch data
echo "DATE 29.11.2000" >> data
echo "MACHINE SUN400Mhz" >> data
echo "COMPILER gcc" >> data
echo "PROG SUN400Mhz" >> data
echo "ITERATIONS 1" >> data
cat files >> data
mkvdata.sh SUN400Mhz mkv2.res.sun  >> data
${AWK} -f data2tex.awk data > mkv2.tmp.tex
