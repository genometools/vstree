#!/bin/sh

set -e

if test $# -lt 1
then
  echo "Usage: $0 <cfilenamelist>"
  exit 1
fi
cat ../Copyright

cat << ENDOFINCLUDE
#ifndef PROTODEF_H
#define PROTODEF_H

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "frontdef.h"
#include "evaluedef.h"
#include "alphadef.h"
#include "scoredef.h"
#include "optdesc.h"
#include "xdropdef.h"
#include "clusterdef.h"
#include "multidef.h"
#include "virtualdef.h"
#include "vnodedef.h"
#include "match.h"
#include "mumcand.h"
#include "alignment.h"
#include "redblackdef.h"
#include "fhandledef.h"
#include "cmp-tabdef.h"
#include "qualint.h"
#include "skiplist.h"
#include "genfile.h"
#include "galigndef.h"
#include "dpbitvec48.h"
#include "queryext.h"
#include "chaindef.h"
void qsortUint(Uint *l, Uint *r);
void qsortStorematchbydiagonal(StoreMatch *l,StoreMatch *r);
void findmaxprefixlen(Virtualtree *virtualtree,
                      Vnode *vnode,
                      Uchar *query,
                      Uint querylen,
                      PairUint *maxwit);

void findmaxprefixlenstack(Virtualtree *virtualtree,
                                   Vnode *vnode,
                                   Uchar *query,
                                   Uint querylen,
                                   PairUint *maxwit,
                                   ArrayVnode *stack);
ENDOFINCLUDE
skproto $*
echo "#endif"
