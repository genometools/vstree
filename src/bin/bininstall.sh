#!/bin/sh

checkerror() {
if test $? -ne 0
then
  echo "failure: ${cmd}"
  exit 1
fi
}

prefix="${WORK}/bin-ops/${CONFIGGUESS}"
for binaryfile in $*
do
  bin/ldd.sh ${binaryfile} > /dev/null
  if test $? -ne 0
  then
    echo "bin/ldd.sh ${binaryfile} fails"
    exit 1
  fi
  if test ${CONFIGGUESS} = "sparc-sun-solaris"
  then
    sparcidentify.sh ${binaryfile}
    exitcode=$?
    if test $exitcode -eq 1
    then 
      exit 1
    else
      if test $exitcode -eq 2
      then
        installpath="${prefix}"
      else
        installpath="${prefix}/sparcv9"
      fi
    fi
  else
    installpath="${prefix}"
  fi
  cmd="mkdir -p ${installpath}"
  ${cmd}
  checkerror
  destfile="`basename ${binaryfile} .x`"
  echo "install ${binaryfile} in ${installpath}/${destfile}"
  cmd="cp -f ${binaryfile} ${installpath}/${destfile}"
  ${cmd}
  checkerror
  chmod g+rx ${installpath}/${destfile}
done
