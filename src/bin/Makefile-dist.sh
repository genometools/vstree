#!/bin/sh

if test $# -ne 1
then
  echo "Usage: $0 <executable>"
  exit 1
fi

cat << TEXT
CC=gcc
LD=\${CC}
LDFLAGS=-m32
LDLIBS=-lm
CFLAGS=-O3 -Wall -Werror -DNOSPACEBOOKKEEPING -m32

SRC=\${wildcard *.c}
OBJ=\${patsubst %.c,%.o,\${SRC}}

${1}:\${OBJ}
	\$(LD) \$(LDFLAGS) \${OBJ} \${LDLIBS} -o \$@

clean:
	rm -f *.o ${1}
TEXT
