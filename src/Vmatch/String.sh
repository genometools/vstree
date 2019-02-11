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

echo "----------$0-------------"

TMPFILE="/tmp/String.sh.$$"
cmd="./Checksout.sh"
${cmd} > ${TMPFILE}
checkerror

TESTFILE="Testdir/Sout1-result"

grep -v '^#' ${TMPFILE} > ${TESTFILE}
rm -f ${TMPFILE}

for filename in `ls Testdir/Sout1.*`
do
  echo "# check ${filename}"
  cmd="cmp -s ${filename} ${TESTFILE}"
  ${cmd}
  if test $? -eq 0
  then
    echo "# ${TESTFILE} = ${filename}"
    rm -f ${TESTFILE}
    exit 0
  fi
done

echo "failure: ${TESTFILE} does not match any file in Testdir/Sout1.*"
exit 1
