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


if test "${CONFIGGUESS}" = "powerpc-apple-darwin" \
     -o "${CONFIGGUESS}" = "i686-apple-darwin"
then
  cmd="otool -L $*"
else
  cmd="ldd $*"
fi
${cmd}
checkerror
