
//\Ignore{

#ifndef FQFINFO_H
#define FQFINFO_H

#include "types.h"

/*
  set the values of an fqfsubstringinfo
*/

#define FQFSETVALUES(FQF,START,END)\
        (FQF)->ffdefined = True;\
        (FQF)->readseqstart = START;\
        (FQF)->readseqend = END

typedef struct
{
  BOOL ffdefined;
  Uint readseqstart, 
       readseqend;
} FQFsubstringinfo;        // \Typedef{FQFsubstringinfo}

#endif
