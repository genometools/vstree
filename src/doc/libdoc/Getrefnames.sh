#!/bin/sh
if test $# -ne 1
then
  echo "Usage: $0 <texfileprefix>"
  exit 1
fi
${AWK} -f Filterref.awk $1.ind | \
        sed -e 's/[{},]//g'        \
            -e 's/\\hyperpage//'   \
            -e '/^T Uint /d'       \
            -e '/^T Sint /d'       \
            -e '/^T Ulong /d'      \
            -e '/^T Uchar /d'      \
            -e '/^T Ushort /d'     \
            -e '/^T String /d'     \
            -e '/^D BOOL /d'       \
            -e '/^D ERROR[0-5] /d' \
            -e '/^D False /d'      \
            -e '/^D True /d'       \
            -e '/^D MAX /d'        \
            -e '/^D MIN /d'        \
            -e '/^D PUSH /d'       \
            -e '/^D TOP /d'        \
            -e '/^D TTAB /d'       \
            -e '/^D LINE /d'       \
            > Refnames
