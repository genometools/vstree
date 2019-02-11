#!/bin/sh

checkerror() {
if test $? -ne 0
then
  echo "failure: ${cmd}"
  exit 1
fi
}

if test $# -ne 1
then
  echo "Usage: $0 <pathend>"
fi

libdir=lib/${CONFIGGUESS}/$1

for archive in `ls ${libdir}/*.a`
do
  installpath="${DIRVSTREE}/${libdir}"
  cmd="mkdir -p ${installpath}"
  ${cmd}
  checkerror
  echo "install `basename ${archive}` in ${installpath}"
  cmd="cp -f $archive ${installpath}"
  ${cmd}
  checkerror
  basefile=`basename ${archive}`
  chmod g+r ${installpath}/${basefile}
done
