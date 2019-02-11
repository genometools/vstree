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

cmd="mkMakedef.pl linux gcc"
${cmd} > Makedef-linux-gcc
checkerror

cmd="mkMakedef.pl linux icc"
${cmd} > Makedef-linux-icc
checkerror

cmd="mkMakedef.pl linux gcc 64"
${cmd} > Makedef-linux-gcc-64
checkerror

cmd="mkMakedef.pl sol2 gcc"
${cmd} > Makedef-sol2-gcc
checkerror

cmd="mkMakedef.pl sol2 gcc 64"
${cmd} > Makedef-sol2-gcc-64
checkerror

cmd="mkMakedef.pl irix gcc"
${cmd} > Makedef-irix-gcc
checkerror

cmd="mkMakedef.pl irix gcc 64"
${cmd} > Makedef-irix-gcc-64
checkerror

cmd="mkMakedef.pl osx gcc"
${cmd} > Makedef-osx-gcc
checkerror

cmd="mkMakedef.pl osx gcc 64"
${cmd} > Makedef-osx-gcc-64
checkerror

cmd="mkMakedef.pl freebsd gcc"
${cmd} > Makedef-freebsd-gcc
checkerror
