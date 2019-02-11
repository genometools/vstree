//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "debugdef.h"
#include "spacedef.h"
#include "match.h"
#include "failures.h"

//}

/*
  This file implements functions to store matches in an array and
  check for pairwise containment of matches. 

  The following macro checks for containment of match \texttt{M2} in 
  \texttt{M1}.
*/

#define CONTAINSMATCH(M1,M2)\
        ((M1)->position1 <= (M2)->position1 &&\
         (M2)->position1 + (M2)->length1 <= (M1)->position1 + (M1)->length1 &&\
         (M1)->position2 <= (M2)->position2 &&\
         (M2)->position2 + (M2)->length2 <= (M1)->position2 + (M1)->length2)

/*EE
  The following function maintains the array containing matches.
  If the match is contained in another match already stored, then it 
  is not added to the container. If an already stored match is contained in 
  a match to be added, this match is deleted. The first argument is 
  a \texttt{void} pointer. This allows to use \texttt{matchcontainer} in
  as the second argument to function \texttt{applymatchcontainer} defined 
  below. The return value is always 0.
*/

Sint matchcontainer(void *info,Match *match)
{
  Match *mptr, *mptrend, *mptrnew; 
  BOOL movednew = False;
  ArrayMatch *mstore = (ArrayMatch *) info;

  DEBUG0(1,"put into container\n");
  DEBUG2(1,"matchcontainer: m1=(%lu,%lu), ",(Showuint) match->length1,
                                            (Showuint) match->position1);
  DEBUG2(1,"m2=(%lu,%lu)\n",(Showuint) match->length2,
                            (Showuint) match->position2);
  GETNEXTFREEINARRAY(mptr,mstore,Match,128);
  mptrnew = mptrend = mstore->spaceMatch + mstore->nextfreeMatch - 1;
  ASSIGNMATCH(mptrend,match);
  if(mstore->nextfreeMatch == UintConst(1))
  {
    return 0;
  }
  for(mptr = mstore->spaceMatch; mptr <= mptrend; /* Nothing */)
  {
    if(mptr == mptrnew)
    {
      DEBUG0(1,"mptr=mptrnew=>break\n");
      break;
    }
    if(CONTAINSMATCH(mptr,mptrnew))
    {
      DEBUG0(1,"old contains new => no insertion\n");
      if(!movednew)
      {
        mstore->nextfreeMatch--;
      }
      break;
    }
    if(CONTAINSMATCH(mptrnew,mptr))
    {
      DEBUG0(1," new contains old => replace\n");
      if(mptr != mptrend)
      {
        ASSIGNMATCH(mptr,mptrend);
        if(!movednew)
        {
          movednew = True;
          mptr++;
        }
      }
      mstore->nextfreeMatch--;
      mptrend--;
    } else
    {
      mptr++;
    }
  } 
  return 0;
}

/*EE
  The following function applies the function
  \texttt{processthematch} to all matches stored in the array \texttt{mstore}.
  When \texttt{processthematch} is called, the first argument is the 
  void pointer \texttt{info}. \texttt{applymatchcontainer} returns
  a negative error code if the function \texttt{processthematch} fails.
*/

Sint applymatchcontainer(void *info,Sint (*processthematch)(void *,Match *),
                         ArrayMatch *mstore)
{
  Match *mptr;

  for(mptr = mstore->spaceMatch;
      mptr < mstore->spaceMatch + mstore->nextfreeMatch;
      mptr++)
  {
    if(processthematch(info,mptr) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}
