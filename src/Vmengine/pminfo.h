#ifndef PMINFO_H
#define PMINFO_H

#include "dpbitvec48.h"
#include "match.h"
#include "matchstate.h"

#define APPROXWITHEQS     UintConst(1)
#define APPROXWITHEQSREV  (UintConst(1) << 1)
#define APPROXWITHCOLUMNS (UintConst(1) << 2)

#define PMMS (&pminfo->matchstate->virtualtree->multiseq)

typedef struct
{
  Matchstate *matchstate;
  BOOL searchnecessary;
  Uint *ecol,
       *dcol,
       threshold,
       plen;
  Uchar *pattern;
  Match match;
  DPbitvector4 Eqs4split[EQSSIZE], // for the splitting procedure
               Eqs4[EQSSIZE],      // bit vector for forward order match
               Eqsrev4[EQSSIZE];   // bit vector for reverse order match
  DPbitvector8 Eqs8split[EQSSIZE], // for the splitting procedure
               Eqs8[EQSSIZE],      // bit vector for forward order match
               Eqsrev8[EQSSIZE];
} PMinfo;

#endif
