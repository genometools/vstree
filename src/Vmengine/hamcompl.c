
#include "chardef.h"
#include "debugdef.h"
#include "pminfo.h"

#include "initcompl.pr"
#include "approxcompl.pr"

static Sint hammingcompletematchesonline(PMinfo *pminfo)
{
  Uchar *sptr, *qptr, *seqptr1;
  Uint countmm;
  BOOL skipmpositions;

  seqptr1 = PMMS->sequence + PMMS->totallength - pminfo->plen;
  while (seqptr1 >= PMMS->sequence)
  {
    if(*seqptr1 == SEPARATOR)
    {
      seqptr1 -= pminfo->plen;
    } else
    {
      countmm = 0;
      skipmpositions = False;
      for(sptr = seqptr1, qptr = pminfo->pattern; 
          sptr < seqptr1 + pminfo->plen; sptr++, qptr++)
      {
        if(*sptr == SEPARATOR)
        {
          skipmpositions = True;
          break;
        }
        if(*sptr != *qptr)
        {
          countmm++;
          if(countmm > pminfo->threshold)
          {
            break;
          }
        }
      }
      if(!skipmpositions && countmm <= pminfo->threshold)
      {
        pminfo->match.position1 = (Uint) (seqptr1-PMMS->sequence);
        pminfo->match.distance = - (Sint) countmm;
        if(pminfo->matchstate->processfinal(pminfo->matchstate,
                                            &pminfo->match) != 0)
        {
          return (Sint) -1;
        }
      }
      seqptr1--;
    }
  }
  return 0;
}

static Uint hammingminscorematch(PMinfo *pminfo,Uint maximalthreshold)
{
  Uint countmm, minscore = maximalthreshold;
  Uchar *sptr, *qptr, *seqptr1;
  BOOL minscoreupdate = False, skipmpositions;

  seqptr1 = PMMS->sequence + PMMS->totallength - pminfo->plen;
  while (seqptr1 >= PMMS->sequence)
  {
    if(*seqptr1 == SEPARATOR)
    {
      seqptr1 -= pminfo->plen;
    } else
    {
      countmm = 0;
      skipmpositions = False;
      for(sptr = seqptr1, qptr = pminfo->pattern; 
          sptr < seqptr1 + pminfo->plen; sptr++, qptr++)
      {
        if(*sptr == SEPARATOR)
        {
          skipmpositions = True;
          break;
        }
        if(*sptr != *qptr)
        {
          countmm++;
          if(countmm >= minscore)
          {
            break;
          }
        }
      }
      if(skipmpositions)
      {
        seqptr1 -= pminfo->plen;
      } else
      {
        seqptr1--;
        if(countmm <= minscore)
        {
          minscoreupdate = True;
          minscore = countmm;
        }
      }
    }
  }
  if(minscoreupdate)
  {
    return minscore;
  }
  return maximalthreshold+1;
}

Sint findhammingcompletematchesonline(void *info,
                                      Uint seqnum2,
                                      Uchar *pattern,
                                      Uint plen)
{
  Matchstate *matchstate = (Matchstate *) info;
  PMinfo pminfo;
  Uint approxdemand = 0;

  if(initpminfo(&pminfo,
                hammingminscorematch,
                approxdemand,
                matchstate,
                pattern,
                seqnum2,
                plen) != 0)
  {
    return (Sint) -1;
  }
  if(pminfo.searchnecessary)
  {
    pminfo.match.length1 = plen;
    if(hammingcompletematchesonline(&pminfo) != 0)
    {
      return (Sint) -2;
    }
  }
  return 0;
}

Sint findhammingcompletematchesindex(void *info,
                                     Uint seqnum2,
                                     Uchar *pattern,
                                     Uint plen)
{
  return findapproxcompletematchesindex(False,
                                        info,
                                        seqnum2,
                                        pattern,
                                        plen);
}
