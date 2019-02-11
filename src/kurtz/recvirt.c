#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "virtualdef.h"
#include "chardef.h"
#include "debugdef.h"
#include "errordef.h"
#include "spacedef.h"
#include "failures.h"

#include "bwtcode.pr"
#include "mkskip.pr"

/*EE
  The following function recomputes the array \texttt{markpos} of the virtual
  tree by scanning the input sequence and storing the positions where
  the symbol \texttt{SEPARATOR} occurs.
*/

void recomputevirtualtreemarkpos(Virtualtree *virtualtree)
{
  Uint *mptr;
  Uchar *tptr;

  ALLOCASSIGNSPACE(virtualtree->multiseq.markpos.spaceUint,
                   NULL,Uint,virtualtree->multiseq.numofsequences-1);
  mptr = virtualtree->multiseq.markpos.spaceUint;
  for(tptr = virtualtree->multiseq.sequence; 
      tptr < virtualtree->multiseq.sequence + virtualtree->multiseq.totallength;
      tptr++)
  {
    if(*tptr == SEPARATOR)
    {
      *mptr++ = (Uint) (tptr - virtualtree->multiseq.sequence);
    }
  }
}

/*EE
  The following functions \texttt{recomputelcptab} and 
  \texttt{recomputelcptabspecial} recompute the lcp-table of the virtual
  tree. Since both function only differ in the function to compare
  character we use the same program fragment twice and define the 
  differences via the macro \texttt{STRINGCOMPARE}. For 
  \texttt{recomputelcptabspecial} we additionally need two variables
  \texttt{a} and \texttt{b}.
*/

#define RECOMPUTELCPTAB recomputelcptab
#define EXTRAVARS       /* Nothing */
#define STRINGCOMPARE(S,T)\
        (((S) < sentinel && (T) < sentinel && *(S) == *(T)) ? 1 : 0)

#include "lcptab.gen"

#undef RECOMPUTELCPTAB
#undef EXTRAVARS
#undef STRINGCOMPARE

#define RECOMPUTELCPTAB recomputelcptabspecial
#define EXTRAVARS       Uchar a, b
#define STRINGCOMPARE(S,T)\
        (((S) < sentinel && ISNOTSPECIAL(a = *(S)) &&\
          (T) < sentinel && ISNOTSPECIAL(b = *(T)) && a == b) ? 1 : 0)

#include "lcptab.gen"

/*EE
  The following function reconstructs the demanded part
  of the virtual tree from the data available. If this is not possible,
  then the function returns a negative error code.
*/

Sint constructvirtualtreetables(Virtualtree *virtualtree,
                               char *indexname,Uint demand)
{
  if(demand & BWTTAB)
  {
    if(!(virtualtree->mapped & BWTTAB))
    {
      if(!(virtualtree->mapped & SUFTAB) || !(virtualtree->mapped & SUFTAB))
      {
        ERROR2("%s.suf and %s.tis are required to construct bwttab",
               indexname,indexname);
        return (Sint) -1;
      }
      ALLOCASSIGNSPACE(virtualtree->bwttab,NULL,Uchar,
                       virtualtree->multiseq.totallength+1);
      if(encodeburrowswheeler(virtualtree->bwttab,
                              virtualtree->suftab,
                              virtualtree->multiseq.sequence,
                              virtualtree->multiseq.totallength) != 0)
      {
        return (Sint) -2;
      }
      virtualtree->constructed |= BWTTAB;
    }
  }
  if(demand & (TISTAB | SUFTAB))
  {
    if(!(virtualtree->mapped & BWTTAB))
    {
      ERROR1("%s.bwt is needed to construct tis- and suftab",indexname);
      return (Sint) -2;
    }
    if(!virtualtree->longest.defined)
    {
      ERROR0("virtualtree->longest is not defined");
      return (Sint) -2;
    }
    ALLOCASSIGNSPACE(virtualtree->multiseq.sequence,NULL,
                     Uchar,virtualtree->multiseq.totallength+1);
    ALLOCASSIGNSPACE(virtualtree->suftab,NULL,
                     Uint,virtualtree->multiseq.totallength+1);
    virtualtree->constructed |= (TISTAB | SUFTAB);
    if(virtualtree->specialsymbols)
    {
      decodeburrowswheelerspecial(virtualtree->suftab,
                                  virtualtree->multiseq.sequence,
                                  virtualtree->suftab,virtualtree->bwttab,
                                  virtualtree->longest.uintvalue,
                                  virtualtree->multiseq.totallength);
    } else
    {
      decodeburrowswheeler(virtualtree->suftab,virtualtree->multiseq.sequence,
                           virtualtree->suftab,virtualtree->bwttab,
                           virtualtree->longest.uintvalue,
                           virtualtree->multiseq.totallength);
    }
  }
  if(virtualtree->multiseq.numofsequences > UintConst(1) &&
     virtualtree->multiseq.markpos.spaceUint == NULL)
  {
    recomputevirtualtreemarkpos(virtualtree);
    virtualtree->constructed |= SSPTAB;
  }
  if(demand & LCPTAB)
  {
    if(!(virtualtree->constructed & TISTAB) && !(virtualtree->mapped & TISTAB))
    {
      ERROR0("cannot construct lcptable, since tis-table is missing");
      return (Sint) -3;
    }
    if(!(virtualtree->constructed & SUFTAB) && !(virtualtree->mapped & SUFTAB))
    {
      ERROR0("cannot construct lcptable, since suf-table is missing");
      return (Sint) -4;
    }
    if(virtualtree->specialsymbols)
    {
      if(recomputelcptabspecial(virtualtree) != 0)
      {
        return (Sint) -5;
      }
    } else
    {
      if(recomputelcptab(virtualtree) != 0)
      {
        return (Sint) -6;
      }
    }
    virtualtree->constructed |= LCPTAB;
  }
  if(demand & SKPTAB)
  {
    if(!(virtualtree->constructed & LCPTAB) && !(virtualtree->mapped & LCPTAB))
    {
      ERROR0("cannot construct skiptab, since lcp-table is missing");
      return (Sint) -7;
    }
    ALLOCASSIGNSPACE(virtualtree->skiptab,NULL,Uint,
                     virtualtree->multiseq.totallength+1);
    makeskiptable(virtualtree->skiptab,virtualtree->lcptab,
                  &virtualtree->largelcpvalues,
                  virtualtree->multiseq.totallength);
    virtualtree->constructed |= SKPTAB;
  }
  return 0;
}

/*
Sint restorevirtualtree(Virtualtree *virtualtree,char *indexname,Uint demand)
{
  Uint demandcopy = demand;

  if(mapvirtualtreeifyoucan(virtualtree,indexname,&demandcopy) != 0)
  {
    return (Sint) -1;
  }
  if(demandcopy != 0)
  {
    if(constructvirtualtreetables(virtualtree,indexname,demandcopy) != 0)
    {
      return (Sint) -2;
    }
    return (Sint) -2;
  }
  return 0;
}
*/
