
#include "chardef.h"
#include "divmodmul.h"
#include "debugdef.h"

#include "matchstate.h"
#include "pminfo.h"

#include "initcompl.pr"
#include "exactcompl.pr"
#include "splitesaapm.pr"
#include "approxcompl.pr"

#define SETMAXLENGTH\
        maxlength = PMMS->totallength - relpos1;\
        if(maxlength > plenplusthreshold)\
        {\
          maxlength = plenplusthreshold;\
        }

/*
  Each position where a match starts may result in several matches
  with less than k or exactly k differences. These immediatly
  follow each other. However, we only want to report one of these
  matches, namely the match with the least number of differences
  to the pattern. Thus we show the matches in a delayed fashion: 
  We check if we have found a match immediately before the 
  current position. If so, then we also check if this match has 
  a larger score than the previous match. If this is the case
  the new match is the better and we keep it.
*/

#define CHECKMATCHPOSITION\
        if(pminfo->matchstate->matchparam.completematchremoveredundancy)\
        {\
          currentpos1 = (Uint) (seqptr1 - PMMS->sequence);\
          if(foundmatch)\
          {\
            if(relpos1 == currentpos1 + 1)\
            {\
              if(score < (Uint) pminfo->match.distance)\
              {\
                relpos1 = currentpos1;\
                pminfo->match.position1 = relpos1;\
                pminfo->match.distance = (Sint) score;\
              } \
            } else\
            {\
              SETMAXLENGTH;\
              if(edistprocessstartpos(relpos1,\
                                      maxlength,\
                                      pminfo) != 0)\
              {\
                return (Sint) -1;\
              }\
              relpos1 = currentpos1;\
              pminfo->match.position1 = relpos1;\
              pminfo->match.distance = (Sint) score;\
            }\
          } else\
          {\
            relpos1 = currentpos1;\
            pminfo->match.position1 = relpos1;\
            pminfo->match.distance = (Sint) score;\
            foundmatch = True;\
          }\
        } else\
        {\
          maxlength = (Uint) (PMMS->sequence + PMMS->totallength - seqptr1);\
          if(maxlength > plenplusthreshold)\
          {\
            maxlength = plenplusthreshold;\
          }\
          if(edistprocessstartpos((Uint) (seqptr1 - PMMS->sequence),\
                                  maxlength,\
                                  pminfo) != 0)\
          {\
            return (Sint) -1;\
          }\
        }

static Sint edistukkonencutoff(PMinfo *pminfo)
{
  Uint i, 
       we, 
       nw, 
       *dcolptr, 
       *end,
       score, 
       relpos1 = 0,
       currentpos1,
       plenplusthreshold,
       maxlength;
  Uchar *seqptr1, *pp;
  BOOL foundmatch = False;

  plenplusthreshold = pminfo->plen + pminfo->threshold;
  score = pminfo->plen;
  for(i=0; i<= pminfo->threshold; i++)
  {
    pminfo->dcol[i] = i;
  }
  for (end=pminfo->dcol+pminfo->threshold+1,
       seqptr1 = PMMS->sequence + PMMS->totallength - 1; 
       seqptr1 >= PMMS->sequence; seqptr1--)
  {
    if(*seqptr1 == SEPARATOR)
    {
      score = pminfo->plen;
      end=pminfo->dcol+pminfo->threshold+1;
      for(i=0; i<= pminfo->threshold; i++)
      {
        pminfo->dcol[i] = i;
      }
    } else
    {
      for(nw = 0, dcolptr = pminfo->dcol+1,
                  pp = pminfo->pattern + pminfo->plen - 1; 
          dcolptr < end;
          dcolptr++, pp--)
      {
        we = *dcolptr;
        if (*seqptr1 == *pp)
        {
          *dcolptr = nw;
        } else
        {
          if (*(dcolptr-1) < nw)
          {
            *dcolptr = *(dcolptr-1)+1;
          } else
          {
            if (we < nw)
            {
              (*dcolptr)++;
            } else
            {
              *dcolptr = nw+1;
            }
          }
        }
        nw = we;
      }
      if (pp >= pminfo->pattern && 
          (*seqptr1 == *pp || pminfo->threshold > *(end-1)))
      {
        *dcolptr = pminfo->threshold;
        end++;
      } else
      {
        for (dcolptr = end-1; 
             *dcolptr > pminfo->threshold; 
             dcolptr--) /*Nothing*/ ;
        end = dcolptr+1;
      }
      if((Uint) (end - pminfo->dcol) == pminfo->plen+1)
      {
        score = *(end-1);
        CHECKMATCHPOSITION;
      }
    }
  }
  if(foundmatch)
  {
    SETMAXLENGTH;
    if(edistprocessstartpos(relpos1,maxlength,pminfo) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

static Uint edistminscoreukkonencutoff(PMinfo *pminfo,Uint maximalthreshold)
{
  Uint i, 
       we, 
       nw, 
       *dcolptr, 
       *end,
       minscore; 
  BOOL minscoreupdate = False;
  Uchar *seqptr1, *pp;

  minscore = maximalthreshold;
  for(i=0; i<= minscore; i++)
  {
    pminfo->dcol[i] = i;
  }
  for (end=pminfo->dcol+minscore+1,
       seqptr1 = PMMS->sequence + PMMS->totallength - 1; 
       seqptr1 >= PMMS->sequence; seqptr1--)
  {
    if(*seqptr1 == SEPARATOR)
    {
      for(i=0; i<= minscore; i++)
      {
        pminfo->dcol[i] = i;
      }
    } else
    {
      for(nw = 0, dcolptr = pminfo->dcol+1,
                  pp = pminfo->pattern + pminfo->plen - 1; 
          dcolptr < end;
          dcolptr++, pp--)
      {
        we = *dcolptr;
        if (*seqptr1 != *pp)
        {
          if (*(dcolptr-1) < nw)
          {
            *dcolptr = *(dcolptr-1)+1;
          } else
          {
            if (we < nw)
            {
              (*dcolptr)++;
            } else
            {
              *dcolptr = nw+1;
            }
          }
        } else
        {
          *dcolptr = nw;
        }
        nw = we;
      }
      if (pp >= pminfo->pattern && (*seqptr1 == *pp || minscore > *(end-1)))
      {
        *dcolptr = minscore;
        end++;
      } else
      {
        for (dcolptr = end-1; 
             *dcolptr > minscore; 
             dcolptr--) /*Nothing*/ ;
        end = dcolptr+1;
      }
      if((Uint) (end - pminfo->dcol) == pminfo->plen+1)
      {
        if(minscore >= *(end-1))
        {
          minscoreupdate = True;
          minscore = *(end-1);
          if(minscore <= UintConst(1))
          {
            break;
          }
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

static Sint edistmyersbitvectorAPM4(PMinfo *pminfo)
{
  DPbitvector4 Pv = (DPbitvector4) ~0, 
               Mv = (DPbitvector4) 0, 
               Eq, 
               Xv, 
               Xh, 
               Ph, 
               Mh, 
               Ebit;
  Uint score, 
       relpos1 = 0, 
       currentpos1, 
       plenplusthreshold,
       maxlength;
  Uchar *seqptr1;
  BOOL foundmatch = False;

  plenplusthreshold = pminfo->plen + pminfo->threshold;
  Ebit = (DPbitvector4) (ONEDPbitvector4 << (pminfo->plen-1));
  score = pminfo->plen;
  for(seqptr1 = PMMS->sequence + PMMS->totallength - 1; 
      seqptr1 >= PMMS->sequence; seqptr1--)
  {
    if(*seqptr1 == SEPARATOR)
    {
      Pv = (DPbitvector4) ~0;
      Mv = (DPbitvector4) 0;
      score = pminfo->plen;
    } else
    {
      Eq = pminfo->Eqsrev4[(Uint) *seqptr1];      //  6
      Xv = Eq | Mv;                               //  7
      Xh = (((Eq & Pv) + Pv) ^ Pv) | Eq;          //  8
  
      Ph = Mv | ~ (Xh | Pv);                      //  9
      Mh = Pv & Xh;                               // 10
  
      UPDATECOLUMNSCORE(score);
  
      Ph <<= 1;                                   // 15
      Pv = (Mh << 1) | ~ (Xv | Ph);               // 17
      Mv = Ph & Xv;                               // 18
      if(score <= pminfo->threshold)
      {
        CHECKMATCHPOSITION;
      }
    }
  }
  if(foundmatch)
  {
    SETMAXLENGTH;
    if(edistprocessstartpos(relpos1,maxlength,pminfo) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

/*
  Implement this and the previous function via templates, once with
  4 and once with 8. XXX
*/

static Sint edistmyersbitvectorAPM8(PMinfo *pminfo)
{
  DPbitvector8 Pv = (DPbitvector8) ~0, 
               Mv = (DPbitvector8) 0, 
               Eq, 
               Xv, 
               Xh, 
               Ph, 
               Mh, 
               Ebit;
  Uint score, 
       relpos1 = 0, 
       currentpos1, 
       plenplusthreshold,
       maxlength;
  Uchar *seqptr1;
  BOOL foundmatch = False;

  plenplusthreshold = pminfo->plen + pminfo->threshold;
  Ebit = (DPbitvector8) (ONEDPbitvector8 << (pminfo->plen-1));
  score = pminfo->plen;
  for(seqptr1 = PMMS->sequence + PMMS->totallength - 1; 
      seqptr1 >= PMMS->sequence; seqptr1--)
  {
    if(*seqptr1 == SEPARATOR)
    {
      Pv = (DPbitvector8) ~0;
      Mv = (DPbitvector8) 0;
      score = pminfo->plen;
    } else
    {
      Eq = pminfo->Eqsrev8[(Uint) *seqptr1];      //  6
      Xv = Eq | Mv;                               //  7
      Xh = (((Eq & Pv) + Pv) ^ Pv) | Eq;          //  8
  
      Ph = Mv | ~ (Xh | Pv);                      //  9
      Mh = Pv & Xh;                               // 10
  
      UPDATECOLUMNSCORE(score);
  
      Ph <<= 1;                                   // 15
      Pv = (Mh << 1) | ~ (Xv | Ph);               // 17
      Mv = Ph & Xv;                               // 18
      if(score <= pminfo->threshold)
      {
        CHECKMATCHPOSITION;
      }
    }
  }
  if(foundmatch)
  {
    SETMAXLENGTH;
    if(edistprocessstartpos(relpos1,maxlength,pminfo) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

static Uint edistminscoremyersbitvectorAPM(PMinfo *pminfo,Uint maximalthreshold)
{
  DPbitvector4 Pv = (DPbitvector4) ~0, 
               Mv = (DPbitvector4) 0, 
               Eq, 
               Xv, 
               Xh, 
               Ph, 
               Mh, 
               Ebit;
  Uint score, minscore;
  Uchar *seqptr1;

  Ebit = (DPbitvector4) (ONEDPbitvector4 << (pminfo->plen-1));
  score = minscore = pminfo->plen;
  for(seqptr1 = PMMS->sequence + PMMS->totallength - 1; 
      seqptr1 >= PMMS->sequence; seqptr1--)
  {
    if(*seqptr1 == SEPARATOR)
    {
      Pv = (DPbitvector4) ~0;
      Mv = (DPbitvector4) 0;
      score = pminfo->plen;
    } else
    {
      Eq = pminfo->Eqsrev4[(Uint) *seqptr1];
      Xv = Eq | Mv;
      Xh = (((Eq & Pv) + Pv) ^ Pv) | Eq;
  
      Ph = Mv | ~ (Xh | Pv);
      Mh = Pv & Xh;
  
      UPDATECOLUMNSCORE(score);
  
      Ph <<= 1;
      Pv = (Mh << 1) | ~ (Xv | Ph);
      Mv = Ph & Xv;
      if(minscore > score)
      {
        minscore = score;
        if(minscore <= UintConst(1))
        {
          break;
        }
      }
    }
  }
  if(minscore <= maximalthreshold)
  {
    DEBUG1(2,"# success with threshold %lu\n",(Showuint) minscore);
    return minscore;
  }
  return maximalthreshold+1;
}

static Uint edistminscorecompletematchesonline(PMinfo *pminfo,
                                               Uint maximalthreshold)
{
  if(checkexactcompletematchesonline(PMMS,
                                     pminfo->pattern,
                                     pminfo->plen))
  {
    DEBUG0(2,"# success with threshold 0\n");
    return 0;
  }
  if(ISLARGEPATTERN4(pminfo->plen))
  {
    return edistminscoreukkonencutoff(pminfo,maximalthreshold);
  } 
  return edistminscoremyersbitvectorAPM(pminfo,maximalthreshold);
}

Sint findedistcompletematchesonline(void *info,
                                    Uint seqnum2,
                                    Uchar *pattern,
                                    Uint plen)
{
  Matchstate *matchstate = (Matchstate *) info;
  PMinfo pminfo;
  Uint approxdemand;

  if(ISLARGEPATTERN8(plen))
  {
    approxdemand = APPROXWITHCOLUMNS;
    if(initpminfo(&pminfo,
                  edistminscorecompletematchesonline,
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
      if(edistukkonencutoff(&pminfo) != 0)
      {
        return (Sint) -2;
      }
    }
    FREESPACE(pminfo.ecol);
    FREESPACE(pminfo.dcol);
  } else
  {
    approxdemand = APPROXWITHEQS | APPROXWITHEQSREV;
    if(initpminfo(&pminfo,
                  edistminscorecompletematchesonline,
                  approxdemand,
                  matchstate,
                  pattern,
                  seqnum2,
                  plen) != 0)
    {
      return (Sint) -3;
    }
    if(pminfo.searchnecessary)
    {
      if((ISLARGEPATTERN4(plen) ? 
            edistmyersbitvectorAPM8 :
            edistmyersbitvectorAPM4) (&pminfo) != 0)
      {
        return (Sint) -4;
      }
    }
  }
  return 0;
}

Sint findedistcompletematchesindex(void *info,
                                   Uint seqnum2,
                                   Uchar *pattern,
                                   Uint plen)
{
  return findapproxcompletematchesindex(True,
                                        info,
                                        seqnum2,
                                        pattern,
                                        plen);
}
