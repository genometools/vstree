
#include "debugdef.h"
#include "genfile.h"

#include "matchstate.h"

#include "multiseq-adv.pr"
#include "mcontain.pr"

#include "xdropext.pr"
#include "extendgen.pr"

/*
  processing function for finding matches. Applied to all matches found.
*/

Sint processexactquerymatch(void *info,
                            Uint l,
                            Uint i,
                            Uint queryseq,
                            Uint querystart)
{
  Match seed;
  PairUint range;
  Matchstate *matchstate = (Matchstate *) info;

  DEBUG4(3,"processexactquerymatch(l=%lu,i=%lu,queryseq=%lu,querystart=%lu)\n",
            (Showuint) l,
            (Showuint) i,
            (Showuint) queryseq,
            (Showuint) querystart);
  seed.length1 = seed.length2 = l;
  seed.position1 = i;
  if(findboundaries(matchstate->queryinfo->multiseq,queryseq,&range) != 0)
  {
    return (Sint) -1;
  }
  seed.position2 = range.uint0 + querystart;
  seed.seqnum2 = queryseq;
  seed.relpos2 = querystart;
  if(matchstate->showselfpalindromic)
  {
    seed.flag = 0;
  } else
  {
    seed.flag = FLAGQUERY;
  }
  if(matchstate->matchparam.xdropbelowscore == UNDEFXDROPBELOWSCORE)
  {
    if(MPARMEXACTMATCH(&matchstate->matchparam.maxdist))
    {
      seed.distance = 0;
      if(CHECKSHOWPALINDROMIC(matchstate))
      {
        seed.flag |= FLAGPALINDROMIC;
      }
      if(matchstate->showselfpalindromic)
      {
        seed.flag |= FLAGSELFPALINDROMIC;
      }
      if(matchstate->processfinal(info,&seed) != 0)
      {
        return (Sint) -2;
      }
    } else
    {
      Match amatch;
      
      amatch.seqnum2 = UNDEFSEQNUM2(matchstate->queryinfo->multiseq);
      if(matchstate->showselfpalindromic)
      {
        amatch.flag = FLAGSELFPALINDROMIC;
      } else
      {
        amatch.flag = FLAGQUERY;
      }
      if(CHECKSHOWPALINDROMIC(matchstate))
      {
        amatch.flag |= FLAGPALINDROMIC;
      }
      if(matchstate->bestflag == Allmaximalmatches)
      {
        matchstate->seedmstore.nextfreeMatch = 0;
        CALLEXTENSIONFUNCTION(False,&matchstate->seedmstore,True);
        if(applymatchcontainer(&matchstate->allmstore,
                               matchcontainer,
                               &matchstate->seedmstore) != 0)
        {
          return (Sint) -3;
        }
      } else
      {
        CALLEXTENSIONFUNCTION(False,info,False);
      }
    }
  } else
  {
    Match amatch;
  
    seed.distance = 0;
    amatch.flag = SETFLAGXDROP(matchstate->matchparam.xdropbelowscore);
    if(matchstate->showselfpalindromic)
    {
      amatch.flag |= FLAGSELFPALINDROMIC;
    } else
    {
      amatch.flag |= FLAGQUERY;
    }
    if(CHECKSHOWPALINDROMIC(matchstate))
    {
      amatch.flag |= FLAGPALINDROMIC;
    }
    amatch.seqnum2 = UNDEFSEQNUM2(matchstate->queryinfo->multiseq);

    DEBUG1(3,"seedlength = %lu\n",(Showuint) matchstate->matchparam.seedlength);
    if(xdropseedextend(CHECKSHOWPALINDROMIC(matchstate) ?  True : False,
                       True,
                       matchstate->revmposorder,
                       matchstate->matchparam.xdropbelowscore,
                       &seed,
                       (Xdropscore) seed.length1 * MATCHSCORE,
                       &matchstate->virtualtree->multiseq,
                       matchstate->queryinfo->multiseq,
                       matchstate->matchparam.seedlength,
                       &amatch))
    {
      if(matchstate->processfinal(info,&amatch) != 0)
      {
        return (Sint) -3;
      }
    }
  }
  return 0;
}
