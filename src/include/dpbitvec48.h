//\IgnoreLatex{

#ifndef DPBITVEC48_H
#define DPBITVEC48_H

#include <stdlib.h>
#include <limits.h>
#include "types.h"

//}

/*
  This file defines some macros related to bitvectors used for dynamic
  programming.
*/

typedef unsigned int DPbitvector4;          // \Typedef{DPbitvector4}

#define DPBYTESINWORD4  4
#define DPBYTESINWORD8  8

#if (LOGWORDSIZE==5)
typedef unsigned long long DPbitvector8;   // \Typedef{DPbitvector8}
#elif (LOGWORDSIZE==6)
typedef unsigned long DPbitvector8;         // \Typedef{DPbitvector8}
#else
#error "illegal value for LOGWORDSIZE: needs to be 5 or 6"
#endif

#define ONEDPbitvector4 ((DPbitvector4) 1)
#define ONEDPbitvector8 ((DPbitvector8) 1)

/*
  The following macro computes the score according to the 
  vertical plus-bitvector \texttt{Ph} and
  the vertical minus-bit vector \texttt{Mh}.
*/

#define UPDATECOLUMNSCORE(SC)\
        if (Ph & Ebit)\
        {\
          (SC)++;\
        } else\
        {\
          if (Mh & Ebit)\
          {\
            (SC)--;\
          }\
        }

/*
  The size of a DPbitvector
*/

#define DPWORDSIZE8            ((Uint) (8 * DPBYTESINWORD8))
#define DPWORDSIZE4            ((Uint) (8 * DPBYTESINWORD4))

/*
  Large patterns cannot be processed directly.
*/

#define ISLARGEPATTERN4(PLEN)   ((PLEN) > DPWORDSIZE4)
#define ISLARGEPATTERN8(PLEN)   ((PLEN) > DPWORDSIZE8)

/*
  DPbitvector with rightmost 1
*/

#define ONEDPbitvector4         ((DPbitvector4) 1)                
#define ONEDPbitvector8         ((DPbitvector8) 1)                

/*
  DPbitvector with leftmost 1
*/

#define LEFTMOSTONEDPbitvector4 (ONEDPbitvector4 << (DPWORDSIZE4-1)) 
#define LEFTMOSTONEDPbitvector8 (ONEDPbitvector8 << (DPWORDSIZE8-1)) 

/*
  Size of the Eq-table
*/

#define EQSSIZE                (UCHAR_MAX+1)                    

#define STATICNUMOFBLOCKS      10
#define STATICMAPSIZE          21

typedef struct
{
  DPbitvector4 Pvmulti4,
               Mvmulti4,
               *eqsmulti4;
} DPbitvectormulti;

typedef struct
{
  Uint numofblocks;
  DPbitvectormulti *eqsmultivector,
                   eqsmultivectorspace[STATICNUMOFBLOCKS];
  DPbitvector4 *eqsmultiflat4,
               eqsmultiflat4space[STATICNUMOFBLOCKS * STATICMAPSIZE],
               eqsspace4[EQSSIZE];
  DPbitvector8 eqsspace8[EQSSIZE];
} DPbitvectorreservoir;

/*
  We add some prototypes of functions to precompute the table 
  \texttt{Eq} in forward or reverse order of the given pattern.
*/

#ifdef __cplusplus
  extern "C" {
#endif

void getEqs4 (DPbitvector4  *Eqs,Uint eqslen,Uchar *u,Uint ulen);
void getEqsrev4 (DPbitvector4  *Eqs,Uint eqslen,Uchar *u,Uint ulen);

void getEqs8 (DPbitvector8  *Eqs,Uint eqslen,Uchar *u,Uint ulen);
void getEqsrev8 (DPbitvector8  *Eqs,Uint eqslen,Uchar *u,Uint ulen);

#ifdef __cplusplus
}
#endif

//\IgnoreLatex{

#endif

//}
