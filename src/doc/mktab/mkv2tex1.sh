#!/bin/sh
touch data
echo "DATE 23.11.2000" >> data
echo "MACHINE SUN400Mhz+COMPAQ500Mhz" >> data
echo "COMPILER gcc" >> data
echo "PROG SUN400Mhz" >> data
echo "PROG COMPAQ500Mhz" >> data
echo "ITERATIONS 1" >> data
cat files >> data
mkvdata.sh SUN400Mhz mkv1.res.sun  >> data
mkvdata.sh COMPAQ500Mhz mkv1.res.dec  >> data
${AWK} -f data2tex.awk data > mk1.tmp.tex
