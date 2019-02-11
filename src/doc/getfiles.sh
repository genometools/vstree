#!/bin/sh
LINK="ln -s"
#LINK="cp"

rm -f EcoliK12 EcoliO157H7 U89959 atEST mgen.fna mpneu.fna

$LINK -f ${BACTERIA}/Ecoli/ecoli.fna EcoliK12
$LINK -f ${BACTERIA}/Ecoli_O157_H7/AE005174.fna EcoliO157H7 
$LINK -f ${U8} U89959
$LINK -f ${AT} atEST
$LINK -f ${DNADIR}/Other/mgen.fna mgen.fna
$LINK -f ${DNADIR}/Other/mpneu.fna mpneu.fna
$LINK -f ${Y3} ychrIII.fna
