
//\IgnoreLatex{

#ifndef FRONTDEF_H
#define FRONTDEF_H
#include <stdlib.h>
#include "types.h"
#include "arraydef.h"

//}

/*
  This module defines some maros to handle fronts in the 
  threshold sensitive algorithm to compute the unit edit distance.
*/

#define MINUSINFINITYFRONT(GL)  ((GL)->integermin)

#ifdef DEBUG

#define STOREFRONT(GL,F,V)\
        if((V) < 0 && (V) != MINUSINFINITYFRONT(GL))\
        {\
          fprintf(stderr,"frontvalue=%ld\n",(Showsint) V);\
          exit(EXIT_FAILURE);\
        }\
        F = V

#else

#define STOREFRONT(GL,F,V)  F = V

#endif

/*
  The following type specifies a front in the threshold sensitive 
  algorithm to compute the unit edit distance.
*/

typedef struct 
{
  Sint offset,   // absolute base address of the front
       width,    // width of the front
       left;     // left boundary (negative), 
                 // -left is relative address of entry
} Frontspec;     // \Typedef{Frontspec}

DECLAREARRAYSTRUCT(Frontspec);
 
#endif
