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
  echo "$0: binary file"
  exit 1
fi

idfile=$1
removetestfile=0
if test -f ${idfile}
then
  file ${idfile} | grep 'archive' > /dev/null
  if test $? -eq 0
  then
    testfile="`ar t ${idfile} | head -n 1`"
    cmd="ar x ${idfile} ${testfile}"
    ${cmd}
    checkerror
    if test -f ${testfile}
    then
      removetestfile=1
    else
      echo "${testfile} is not available"
      exit 1
    fi
  else
    file ${idfile} | grep 'relocatable' > /dev/null
    if test $? -eq 0
    then
      testfile=$idfile
    else
      file ${idfile} | grep 'executable' > /dev/null
      if test $? -eq 0
      then
        testfile=$idfile
      else
         echo "$0: cannot identify architecture from \"${idfile}\""
         exit 1
      fi
    fi
  fi
else
  echo "$0: file \"${idfile}\" does not exist"
  exit 1
fi
filestring="`file ${testfile} | sed -e 's/,//g' -e 's/V9//' | ${AWK} '{print $3 \"-\" $6}'`"
if test ${removetestfile} -eq 1
then
  rm -f ${testfile}
fi
if test ${filestring} = "64-bit-SPARC"
then
  exit 3
else
  if test ${filestring} = "32-bit-SPARC"
  then
    exit 2
  else
    echo "Cannot recognize ${filestring}"
    exit 1
  fi
fi
