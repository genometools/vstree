#ifndef SARRDEF_H
#define SARRDEF_H
#include <stdio.h>
#include "types.h"
#include "intbits.h"
#include "spacedef.h"

// do not change the following:

#ifdef SUFFIXPTR
typedef Uchar * Suffixptr;
#define ACCESSCHAR(T)   *(T)
#else
typedef Uint Suffixptr;
#define ACCESSCHAR(T)   virtualtree->multiseq.sequence[T]
#endif

DECLAREARRAYSTRUCT(Suffixptr);

/*
  A variable codedistance stores the code and the distance in the following
  bits:
  distance in 4 lower order bits
  code     in the remaining higher order bits
*/

#define PREFIXLENBITS                  4
#define CODEDISTANCE(C,D)              (((C) << PREFIXLENBITS) | (D))
#define GETCODE(V)                     ((V) >> PREFIXLENBITS)

/*
  The following constant is used to state that the prefix length is
  to be deterimined automatically.
*/

#define AUTOPREFIXLENGTH               UintConst(1024)

typedef struct
{
  Suffixptr sentinel,             // AUX
            *sortedsuffixes;      // AUX
  Uint maxbucketsize,             // AUX
       *leftborder,               // AUX
       *specialtable;             // AUX
  ArrayUint arcodedistance;       // AUX
  BOOL storecodes;                // AUX
#ifdef SUFFIXPTR
  Suffixptr compareptr;           // AUX
#endif
#ifdef WITHLCP
  Uint *lcpsubtab;                // AUX
#endif
#ifdef DEBUG
  Uint maxstacksize;              // AUX
#endif
} MKVaux;

#endif
