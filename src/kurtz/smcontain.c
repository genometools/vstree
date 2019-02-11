//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include "types.h"
#include "debugdef.h"
#include "spacedef.h"
#include "intbits.h"
#include "errordef.h"
#include "failures.h"
#include "match.h"

//}

/*
  This file implements functions to store matches in an array and
  check for pairwise containment of matches. 
*/

static Qsortcomparereturntype ordermatchp1l1(StoreMatch *p,StoreMatch *q)
{
  if(p->Storeposition1 == q->Storeposition1)
  {
    if(p->Storelength1 == q->Storelength1)
    {
      return (p->Storeposition2 > q->Storeposition2) ? 1 : -1;
    }
    return (p->Storelength1 > q->Storelength1) ? 1 : -1;
  }
  return (p->Storeposition1 > q->Storeposition1) ? 1 : -1;
}

/*EE
  The following function removes all matches in \texttt{matchstore}
  that are contained in other matches.
*/

Uint removecontained(ArrayStoreMatch *matchstore)
{
  StoreMatch *mptr1, *mptr2;
  Uint idx, idxaux, *reject, countoriginal;

  if(matchstore->nextfreeStoreMatch > 0)
  {
    qsort(matchstore->spaceStoreMatch,(size_t) matchstore->nextfreeStoreMatch,
          sizeof(StoreMatch),
          (Qsortcomparefunction) ordermatchp1l1);
    INITBITTAB(reject,matchstore->nextfreeStoreMatch);
    for(mptr1 = matchstore->spaceStoreMatch;
        mptr1 < matchstore->spaceStoreMatch + matchstore->nextfreeStoreMatch;
        mptr1++)
    {
      idxaux = (Uint) (mptr1 - matchstore->spaceStoreMatch);
      for(mptr2 = mptr1-1;
          mptr2 >= matchstore->spaceStoreMatch && 
          mptr2->Storeposition1 == mptr1->Storeposition1;
          mptr2--)
      {
        if(!ISIBITSET(reject,idxaux) && CONTAINSSTOREMATCH(mptr1,mptr2))
        {
          idx = (Uint) (mptr2 - matchstore->spaceStoreMatch);
          SETIBIT(reject,idx);
        }
      }
      for(mptr2 = mptr1+1;
          mptr2 < matchstore->spaceStoreMatch+matchstore->nextfreeStoreMatch &&
          mptr2->Storeposition1 <= mptr1->Storeposition1+mptr1->Storelength1;
	  mptr2++)
      {
        if(!ISIBITSET(reject,idxaux) && CONTAINSSTOREMATCH(mptr1,mptr2))
        {
          idx = (Uint) (mptr2 - matchstore->spaceStoreMatch);
          SETIBIT(reject,idx);
        }
      }
    }
    mptr2 = matchstore->spaceStoreMatch;
    for(idx = 0; idx < matchstore->nextfreeStoreMatch; idx++)
    {
      if(!ISIBITSET(reject,idx))
      {
        *mptr2++ = matchstore->spaceStoreMatch[idx];
      }
    }
    countoriginal = matchstore->nextfreeStoreMatch;
    matchstore->nextfreeStoreMatch 
      = (Uint) (mptr2 - matchstore->spaceStoreMatch);
    FREESPACE(reject);
    return countoriginal - matchstore->nextfreeStoreMatch;
  }
  return 0;
}


#ifdef DEBUG

/*EE
  The following function maintains the array containing Storematches.
  The function is identical to the function matchcontainer
  which however is implemented on the type match.
  If the match is contained in another match already stored, then it 
  is not added to the container. If an already stored match is contained in 
  a match to be added, this match is deleted. The first argument is 
  a \texttt{void} pointer. This allows to use \texttt{storematchcontainer} in
  as the second argument to function \texttt{applymatchcontainer} defined 
  below. The return value is always 0.
*/

static Sint storematchcontainer(void *info,StoreMatch *match)
{
  StoreMatch *mptr, *mptrend, *mptrnew; 
  BOOL movednew = False;
  ArrayStoreMatch *mstore = (ArrayStoreMatch *) info;

  DEBUG0(1,"put into container\n");
  DEBUG2(1,"storematchcontainer: m1=(%lu,%lu), ",
            (Showuint) match->Storelength1,
            (Showuint) match->Storeposition1);
  DEBUG2(1,"m2=(%lu,%lu)\n",
            (Showuint) match->Storelength2,
            (Showuint) match->Storeposition2);
  GETNEXTFREEINARRAY(mptr,mstore,StoreMatch,128);
  mptrnew = mptrend = mstore->spaceStoreMatch + mstore->nextfreeStoreMatch - 1;
  ASSIGNSTOREMATCH(mptrend,match);
  if(mstore->nextfreeStoreMatch == UintConst(1))
  {
    return 0;
  }
  for(mptr = mstore->spaceStoreMatch; mptr <= mptrend; /* Nothing */)
  {
    if(mptr == mptrnew)
    {
      DEBUG0(1,"mptr=mptrnew=>break\n");
      break;
    }
    if(CONTAINSSTOREMATCH(mptr,mptrnew))
    {
      DEBUG0(1,"old contains new => no insertion\n");
      if(!movednew)
      {
        mstore->nextfreeStoreMatch--;
      }
      break;
    }
    if(CONTAINSSTOREMATCH(mptrnew,mptr))
    {
      DEBUG0(1," new contains old => replace\n");
      if(mptr != mptrend)
      {
        ASSIGNSTOREMATCH(mptr,mptrend);
        if(!movednew)
        {
          movednew = True;
          mptr++;
        }
      }
      mstore->nextfreeStoreMatch--;
      mptrend--;
    } else
    {
      mptr++;
    }
  } 
  return 0;
}

static Sint removecontainedslow(ArrayStoreMatch *mstore)
{
  StoreMatch *smptr;
  ArrayStoreMatch nonmax;
  Uint i;

  INITARRAY(&nonmax,StoreMatch);
  for(smptr = mstore->spaceStoreMatch;
      smptr < mstore->spaceStoreMatch + mstore->nextfreeStoreMatch; smptr++)
  {
    if(storematchcontainer((void *) &nonmax,smptr) != 0)
    {
      return (Sint) -1;
    }
  }
  NOTSUPPOSEDTOBENULL(mstore->spaceStoreMatch);
  if(nonmax.spaceStoreMatch != NULL)
  {
    for(i=0; i< nonmax.nextfreeStoreMatch; i++)
    {
      ASSIGNSTOREMATCH(mstore->spaceStoreMatch + i,nonmax.spaceStoreMatch + i);
    }
  }
  mstore->nextfreeStoreMatch = nonmax.nextfreeStoreMatch;
  FREEARRAY(&nonmax,StoreMatch);
  return 0;
}

#define CHECKUINTVALUE(VAL)\
        if(mptr1->VAL != mptr2->VAL)\
        {\
          fprintf(stderr,"%s: %lu != %lu2\n",#VAL,(Showuint) mptr1->VAL,\
                                                  (Showuint) mptr2->VAL);\
          exit(EXIT_FAILURE);\
        }

Sint checkremoval(ArrayStoreMatch *matchstore)
{
  StoreMatch *mptr1, *mptr2;
  ArrayStoreMatch copymatch;

  ALLOCASSIGNSPACE(copymatch.spaceStoreMatch,
                   NULL,StoreMatch,
                   matchstore->nextfreeStoreMatch);
  copymatch.allocatedStoreMatch = copymatch.nextfreeStoreMatch 
                                = matchstore->nextfreeStoreMatch;
  DEBUG0(3,"all\n");
  for(mptr1 = matchstore->spaceStoreMatch,
      mptr2 = copymatch.spaceStoreMatch;
      mptr1 < matchstore->spaceStoreMatch + matchstore->nextfreeStoreMatch; 
      mptr1++, mptr2++)
  {
    DEBUG4(3,"%lu %lu %lu %lu\n",
            (Showuint) mptr1->Storeposition1,
            (Showuint) (mptr1->Storeposition1 + mptr1->Storelength1 - 1),
            (Showuint) mptr1->Storeposition2,
            (Showuint) (mptr1->Storeposition2 + mptr1->Storelength2 - 1));
    *mptr2 = *mptr1;
  }
  if(removecontainedslow(&copymatch) != 0)
  {
    return (Sint) -2;
  }
  (void) removecontained(matchstore);
  if(copymatch.nextfreeStoreMatch != matchstore->nextfreeStoreMatch)
  {
    fprintf(stderr,"copymatch.size = %lu != %lu = matchstore->size\n",
                    (Showuint) copymatch.nextfreeStoreMatch,
                    (Showuint) matchstore->nextfreeStoreMatch);
    exit(EXIT_FAILURE);
  }
  qsort(copymatch.spaceStoreMatch,(size_t) copymatch.nextfreeStoreMatch,
        sizeof(StoreMatch),
        (Qsortcomparefunction) ordermatchp1l1);
  DEBUG0(3,"fast\n");
  for(mptr1 = matchstore->spaceStoreMatch;
      mptr1 < matchstore->spaceStoreMatch + matchstore->nextfreeStoreMatch; 
      mptr1++)
  {
    DEBUG4(3,"%lu %lu %lu %lu\n",
            (Showuint) mptr1->Storeposition1,
            (Showuint) (mptr1->Storeposition1 + mptr1->Storelength1 - 1),
            (Showuint) mptr1->Storeposition2,
            (Showuint) (mptr1->Storeposition2 + mptr1->Storelength2 - 1));
  }
  DEBUG0(3,"correct\n");
  for(mptr1 = copymatch.spaceStoreMatch;
      mptr1 < copymatch.spaceStoreMatch + copymatch.nextfreeStoreMatch; 
      mptr1++)
  {
    printf("%lu %lu %lu %lu\n",
            (Showuint) mptr1->Storeposition1,
            (Showuint) (mptr1->Storeposition1 + mptr1->Storelength1 - 1),
            (Showuint) mptr1->Storeposition2,
            (Showuint) (mptr1->Storeposition2 + mptr1->Storelength2 - 1));
  }
  for(mptr1 = matchstore->spaceStoreMatch,
      mptr2 = copymatch.spaceStoreMatch;
      mptr1 < matchstore->spaceStoreMatch + matchstore->nextfreeStoreMatch; 
      mptr1++, mptr2++)
  {
    CHECKUINTVALUE(Storeposition1);
    CHECKUINTVALUE(Storelength1);
    CHECKUINTVALUE(Storeposition2);
    CHECKUINTVALUE(Storelength2);
    CHECKUINTVALUE(Storeseqnum1);
    CHECKUINTVALUE(Storerelpos1);
    CHECKUINTVALUE(Storeseqnum2);
    CHECKUINTVALUE(Storerelpos2);
  }
  FREESPACE(copymatch.spaceStoreMatch);
  return 0;
}
#endif
