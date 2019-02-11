
#include "match.h"
#include "spacedef.h"
#include "debugdef.h"
#include "pminfo.h"

void initcompletematchstruct(Match *match,
                             Uint seqnum2,
                             Uint plen,
                             BOOL ispalindromic)
{
  match->position2 = 0;
  match->relpos2 = 0;
  match->seqnum2 = seqnum2;
  match->length2 = plen;
  match->flag = FLAGQUERY | FLAGCOMPLETEMATCH;
  if(ispalindromic)
  {
    match->flag |= FLAGPALINDROMIC;
  }
}

Sint initpminfo(PMinfo *pminfo,
                Uint (*evalsmallestscore)(PMinfo *,Uint),
                Uint approxdemand,
                Matchstate *matchstate,
                Uchar *pattern,
                Uint seqnum2,
                Uint plen)
{
  pminfo->matchstate = matchstate;
  pminfo->plen = plen;
  pminfo->ecol = NULL;
  pminfo->dcol = NULL;
  pminfo->pattern = pattern;
  pminfo->searchnecessary = True;
  if(approxdemand & APPROXWITHEQS)
  {
    getEqs4(&pminfo->Eqs4[0],(Uint) EQSSIZE,pattern,plen);
    getEqs8(&pminfo->Eqs8[0],(Uint) EQSSIZE,pattern,plen);
  }
  if(approxdemand & APPROXWITHEQSREV)
  {
    getEqsrev4(&pminfo->Eqsrev4[0],(Uint) EQSSIZE,pattern,plen);
    getEqsrev8(&pminfo->Eqsrev8[0],(Uint) EQSSIZE,pattern,plen);
  }
  if(approxdemand & APPROXWITHCOLUMNS)
  {
    ALLOCASSIGNSPACE(pminfo->dcol,NULL,Uint,plen+1);
    ALLOCASSIGNSPACE(pminfo->ecol,NULL,Uint,plen+1);
  }
  if(matchstate->matchparam.maxdist.distinterpretation == Qualpercentaway)
  {
    pminfo->threshold 
      = (plen * matchstate->matchparam.maxdist.distvalue)/UintConst(100);
  } else
  {
    if(matchstate->matchparam.maxdist.distinterpretation == Qualbestof)
    {
      Uint maximalthreshold, minscore;

      if(evalsmallestscore == NULL)
      {
        fprintf(stderr,"program error: function evalsmallestscore undefined\n");
        exit(EXIT_FAILURE);
      }
      maximalthreshold 
        = (plen * matchstate->matchparam.maxdist.distvalue)/UintConst(100);
      minscore = evalsmallestscore(pminfo,maximalthreshold);
      if(minscore <= maximalthreshold)
      {
        matchstate->matchparam.maxdist.distvalue = minscore;
      } else
      {
        pminfo->searchnecessary = False;
      }
    }
    pminfo->threshold = matchstate->matchparam.maxdist.distvalue;
  }
  if(pminfo->searchnecessary)
  {
    initcompletematchstruct(&pminfo->match,
                            seqnum2,
                            plen,
                            CHECKSHOWPALINDROMIC(matchstate) ? True : False);
  }
  return 0;
}
