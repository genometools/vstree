#!/bin/sh
extraopts=$*
STARTDIR=..
FILELIST=`ls ${STARTDIR}/include/*.[ch]\
             ${STARTDIR}/kurtz/*.c\
             ${STARTDIR}/kurtz/libtest/*.c\
             ${STARTDIR}/kurtz/*.gen\
             ${STARTDIR}/kurtz-basic/*.c\
             ${STARTDIR}/kurtz-basic/*.gen\
             ${STARTDIR}/Vmengine/*.[ch]\
             ${STARTDIR}/Mkvtree/*.[ch]\
             ${STARTDIR}/Vmatch/*.[ch]\
             ${STARTDIR}/Vmengine/*.gen\
             ${STARTDIR}/Vmatch/SELECT/*.c\
             ${STARTDIR}/Multimat/*.[ch]\
             ${STARTDIR}/Multimat/*.gen\
             ${STARTDIR}/fmmatch/*.[ch]\
             ${STARTDIR}/GeneCluster/*.[ch]\
             ${STARTDIR}/Mergesarr/*.[ch]\
             ${STARTDIR}/Minunique/*.[ch]\
             ${STARTDIR}/Raremems/*.[ch]\
             ${STARTDIR}/Smwa/*.[ch]\
             ${STARTDIR}/Swift/*.[ch]\
             ${STARTDIR}/Vmerstat/*.[ch]`
egrep ${extraopts} -f ${WORKVSTREE}/src/bin/Forbidden.grep ${FILELIST}
if test $? -ne 1
then
  echo "forbidden format pattern occurs in file";
  exit 1;
fi
grep -wl 'int' ${FILELIST}
