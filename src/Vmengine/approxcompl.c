
#include "chardef.h"
#include "divmodmul.h"
#include "debugdef.h"

#include "matchstate.h"
#include "pminfo.h"

#include "initcompl.pr"
#include "exactcompl.pr"
#include "splitesaapm.pr"
#include "longestmatch.pr"

Sint edistprocessstartpos(Uint startpos,Uint maxlength,void *info)
{
  PMinfo *pminfo = (PMinfo *) info;
  PairUint longest;

  if(ISLARGEPATTERN8(pminfo->plen))
  {
    longpatternlongestmatch(&longest,
                            pminfo->ecol,
                            pminfo->pattern,
                            pminfo->plen,
                            pminfo->matchstate->virtualtree->multiseq.sequence 
                              + startpos,
                            maxlength);
  } else
  {
    if(ISLARGEPATTERN4(pminfo->plen))
    {
      mediumpatternlongestmatch(&longest,
                                &pminfo->Eqs8[0],
                                pminfo->plen,
                                pminfo->matchstate->virtualtree->multiseq.
                                                                 sequence 
                                 + startpos,
                                maxlength);
    } else
    {
      shortpatternlongestmatch(&longest,
                               &pminfo->Eqs4[0],
                               pminfo->plen,
                               pminfo->matchstate->virtualtree->multiseq.
                                                                sequence 
                                + startpos,
                               maxlength);
    }
  }
  pminfo->match.position1 = startpos;
  pminfo->match.length1 = longest.uint0;
#ifdef DEBUG
  if(longest.uint1 > pminfo->matchstate->matchparam.maxdist.distvalue)
  {
    fprintf(stderr,"illegal distance value %lu\n",(Showuint) longest.uint1);
    exit(EXIT_FAILURE);
  }
#endif
  pminfo->match.distance = (Sint) longest.uint1;
  if(pminfo->matchstate->processfinal(pminfo->matchstate,&pminfo->match) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

Sint hammingprocessstartpos(Uint startpos,Uint mmcount,void *info)
{
  PMinfo *pminfo = (PMinfo *) info;

  pminfo->match.position1 = startpos;
  pminfo->match.length1 = pminfo->plen,
  pminfo->match.distance = -(Sint) mmcount;
  if(pminfo->matchstate->processfinal(pminfo->matchstate,&pminfo->match) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

static Uint approxminscorecompletematchesindexgeneric(BOOL doedist,
                                                      PMinfo *pminfo,
                                                      Uint maximumthreshold)
{
  Sint retval;
  Uint lth, rth, mth, threshold = maximumthreshold+1;

  retval = checkexactcompletematchesindex(pminfo->matchstate->virtualtree,
                                          pminfo->pattern,
                                          pminfo->plen);
  if(retval == SintConst(1))
  {
    DEBUG0(2,"# success with threshold 0\n");
    return 0;
  }
  lth = UintConst(1);
  rth = maximumthreshold;
  while(lth <= rth)
  {
    DEBUG2(2,"# (lth,rth)=(%lu,%lu)\n",(Showuint) lth,(Showuint) rth);
    mth = lth + DIV2(rth - lth + 1);
    DEBUG1(2,"# threshold=%lu\n",(Showuint) mth);
    if(splitesaapm(doedist,
                   NULL,
                   NULL,
                   pminfo->matchstate->virtualtree,
                   &pminfo->Eqs4split[0],
                   mth,
                   pminfo->pattern,
                   pminfo->plen) == SintConst(1))
    {
      threshold = mth;
      rth = mth-1;
    } else
    {
      lth = mth+1;
    }
  }
  return threshold;
}

static Uint edistminscorecompletematchesindex(PMinfo *pminfo,
                                              Uint maximumthreshold)
{
  return approxminscorecompletematchesindexgeneric(True,
                                                   pminfo,
                                                   maximumthreshold);
}

static Uint hammingminscorecompletematchesindex(PMinfo *pminfo,
                                                Uint maximumthreshold)
{
  return approxminscorecompletematchesindexgeneric(False,
                                                   pminfo,
                                                   maximumthreshold);
}

Sint findapproxcompletematchesindex(BOOL doedist,
                                    void *info,
                                    Uint seqnum2,
                                    Uchar *pattern,
                                    Uint plen)
{
  PMinfo pminfo;
  Matchstate *matchstate = (Matchstate *) info;
  Uint approxdemand;

  if(doedist)
  {
    approxdemand = APPROXWITHEQS | APPROXWITHCOLUMNS;
  } else
  {
    approxdemand = 0;
  }
  if(initpminfo(&pminfo,
                doedist ? edistminscorecompletematchesindex
                        : hammingminscorecompletematchesindex,
                approxdemand,
                matchstate,
                pattern,
                seqnum2,
                plen) != 0)
  {
    return (Sint) -1;
  }
  if(pminfo.threshold == 0)
  {
    if(findexactcompletematchesindex(info,
                                     seqnum2,
                                     pattern,
                                     plen) != 0)
    {
      return (Sint) -2;
    }
  } else
  {
    if(pminfo.searchnecessary)
    {
      if(splitesaapm(doedist,
                     doedist ? edistprocessstartpos
                             : hammingprocessstartpos,
                     (void *) &pminfo,
                     matchstate->virtualtree,
                     &pminfo.Eqs4split[0],
                     pminfo.threshold,
                     pminfo.pattern,
                     pminfo.plen) != 0)
      {
        return (Sint) -3;
      }
    }
  }
  if(doedist)
  {
    FREESPACE(pminfo.ecol);
    FREESPACE(pminfo.dcol);
  }
  return 0;
}
