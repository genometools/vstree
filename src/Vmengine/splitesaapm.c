
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "debugdef.h"
#include "spacedef.h"
#include "dpbitvec48.h"
#include "errordef.h"
#include "types.h"
#include "redblackdef.h"
#include "arraydef.h"
#include "chardef.h"

#include "regionsmerger.pr"
#include "redblack.pr"

#include "pminfo.h"

#include "esaapm.pr"
#include "esahamming.pr"
#include "exactcompl.pr"

typedef struct
{
  Virtualtree *virtualtree;
  Uchar *pattern, *patternend;
  DPbitvector4 *Eqs4split;
  Uint regionoffsetend,
    regionoffsetstart,
    patternlength, 
    threshold, 
    dpwork, 
    maxwidthofregiontoverify,
    *dcol;
  BOOL doedist;
  void *apmoutinfo, *regiontreeroot;
  Sint (*processapmstartpos)(Uint,Uint,void *);
#ifdef DEBUG
  ArrayPairUint originalregions;
#endif
} Verifyinfo;

static Sint verifyedistlongmatch(Verifyinfo *verifyinfo, 
                                 PairUint *region,
                                 Uint regionmaxlength)
{
  Uchar *pp, *tptr;
  Uint i, we, nw, *dcolptr, *end;

  for(i = 0; i <= verifyinfo->threshold; i++)
  {
    verifyinfo->dcol[i] = i;
  }
  for(end = verifyinfo->dcol + verifyinfo->threshold + 1,
      tptr = verifyinfo->virtualtree->multiseq.sequence + region->uint1;
      tptr >= verifyinfo->virtualtree->multiseq.sequence + region->uint0;
      tptr--)
  {
    if(*tptr == SEPARATOR)
    {
      for(i = 0; i <= verifyinfo->threshold; i++)
      {
        verifyinfo->dcol[i] = i;
      }
      end = verifyinfo->dcol + verifyinfo->threshold + 1;
    } else
    {
      for(nw = 0, dcolptr = verifyinfo->dcol + 1, pp =
          verifyinfo->patternend; dcolptr < end; dcolptr++, pp--)
      {
        we = *dcolptr;
        if(*tptr == *pp)
        {
          *dcolptr = nw;
        } else
        {
          if(*(dcolptr - 1) < nw)
          {
            *dcolptr = *(dcolptr - 1) + 1;
          } else
          {
            if(we < nw)
            {
              (*dcolptr)++;
            } else
            {
              *dcolptr = nw + 1;
            }
          }
        }
        nw = we;
      }
      if(pp >= verifyinfo->pattern &&
         (*tptr == *pp || verifyinfo->threshold > *(end - 1)))
      {
        *dcolptr = verifyinfo->threshold;
        end++;
      } else
      {
        for(dcolptr = end - 1; *dcolptr > verifyinfo->threshold; dcolptr--)
          /* Nothing */ ;
        end = dcolptr + 1;
      }
    }
    if((Uint) (end - verifyinfo->dcol) == verifyinfo->patternlength + 1)
    {
      if(verifyinfo->processapmstartpos == NULL)
      {
        return (Sint) 1;
      }
      if(verifyinfo->processapmstartpos((Uint)
                                        (tptr -
                                         verifyinfo->virtualtree->multiseq.
                                         sequence),
                                        regionmaxlength,
                                        verifyinfo->apmoutinfo) != 0)
      {
        return (Sint) -1;
      }
    }
  }
  return 0;
}

static Sint verifyedistshortmatch(Verifyinfo *verifyinfo, 
                                  PairUint *region,
                                  Uint regionmaxlength)
{
  DPbitvector4 Pv = (DPbitvector4) ~0,
               Mv = (DPbitvector4) 0,
               Eq,
               Xv,
               Xh,
               Ph,
               Mh,
               Ebit;
  Uint score;
  Uchar *tptr;

  Ebit = (DPbitvector4) (ONEDPbitvector4 << (verifyinfo->patternlength-1));
  score = verifyinfo->patternlength;
  for(tptr = verifyinfo->virtualtree->multiseq.sequence + region->uint1;
      tptr >= verifyinfo->virtualtree->multiseq.sequence + region->uint0;
      tptr--)
  {
    if(*tptr == SEPARATOR)
    {
      Pv = (DPbitvector4) ~0;
      Mv = (DPbitvector4) 0;
      score = verifyinfo->patternlength;
    } else
    {
      Eq = verifyinfo->Eqs4split[(Uint) *tptr];   //  6
      Xv = Eq | Mv;                               //  7
      Xh = (((Eq & Pv) + Pv) ^ Pv) | Eq;          //  8

      Ph = Mv | ~ (Xh | Pv);                      //  9
      Mh = Pv & Xh;                               // 10

      UPDATECOLUMNSCORE(score);

      Ph <<= 1;                                   // 15
      Pv = (Mh << 1) | ~ (Xv | Ph);               // 17
      Mv = Ph & Xv;                               // 18
      if(score <= verifyinfo->threshold)
      {
        if(verifyinfo->processapmstartpos == NULL)
        {
          return (Sint) 1;
        }
        if(verifyinfo->processapmstartpos((Uint)
                                          (tptr -
                                           verifyinfo->virtualtree->multiseq.
                                           sequence), 
                                           regionmaxlength,
                                           verifyinfo->apmoutinfo) != 0)
        {
          return (Sint) -1;
        }
      }
    }
  }
  return 0;
}

static Sint verifyhammingmatch(Verifyinfo *verifyinfo, 
                               PairUint *region)
{
  Uchar *tptr, *qptr, *sptr;
  Uint skipmpositions, countmm;

  for(tptr = verifyinfo->virtualtree->multiseq.sequence + region->uint1 
             - verifyinfo->patternlength + 1;
      tptr >= verifyinfo->virtualtree->multiseq.sequence + region->uint0;
      tptr--)
  {
    if(*tptr == SEPARATOR)
    {
      tptr -= verifyinfo->patternlength;
    } else
    {
      countmm = 0;
      skipmpositions = False;
      for(sptr = tptr, qptr = verifyinfo->pattern;
          sptr < tptr + verifyinfo->patternlength; sptr++, qptr++)
      {
        if(*sptr == SEPARATOR)
        {
          skipmpositions = True;
          break;
        }
        if(*sptr != *qptr)
        {
          countmm++;
          if(countmm > verifyinfo->threshold)
          {
            break;
          }
        }
      }
      if(!skipmpositions && countmm <= verifyinfo->threshold)
      {
        if(verifyinfo->processapmstartpos == NULL)
        {
          return (Sint) 1;
        }
        if(verifyinfo->processapmstartpos((Uint)
                                          (tptr -
                                           verifyinfo->virtualtree->multiseq.
                                           sequence), 
                                           countmm,
                                           verifyinfo->apmoutinfo) != 0)
        {
          return (Sint) -1;
        }
      }
    }
  }
  return 0;
}

static Sint verifytheapproximatematch(Verifyinfo *verifyinfo, 
                                      PairUint *region)
{
  Uint regionmaxlength = verifyinfo->maxwidthofregiontoverify;

  DEBUG2(3, "verify (%lu,%lu)\n",
         (Showuint) region->uint0, (Showuint) region->uint1);
  if(regionmaxlength > (Uint) (region->uint1 - region->uint0 + 1))
  {
    regionmaxlength = (Uint) (region->uint1 - region->uint0 + 1);
  }
  verifyinfo->dpwork += regionmaxlength;
  if(verifyinfo->doedist)
  {
    if(ISLARGEPATTERN4(verifyinfo->patternlength))
    {
      return verifyedistlongmatch(verifyinfo,region,regionmaxlength);
    } else
    {
      return verifyedistshortmatch(verifyinfo,region,regionmaxlength);
    }
  } else
  {
    return verifyhammingmatch(verifyinfo,region);
  }
}

static Sint storeapmposition(Uint startpos,
                             /*@unused@*/ Uint maxlength, 
                             void *info)
{
  Verifyinfo *verifyinfo = (Verifyinfo *) info;
  BOOL nodecreated;
  PairUint region;

  if(verifyinfo->regionoffsetstart > startpos)
  {
    region.uint0 = 0;
  } else
  {
    region.uint0 = startpos - verifyinfo->regionoffsetstart;
  }
  if(verifyinfo->regionoffsetend + startpos - 1 <
     verifyinfo->virtualtree->multiseq.totallength)
  {
    region.uint1 = verifyinfo->regionoffsetend + startpos - 1;
  } else
  {
    region.uint1 = verifyinfo->virtualtree->multiseq.totallength - 1;
  }
#ifdef DEBUG
  STOREINARRAY(&verifyinfo->originalregions, PairUint, 256, region);
#endif
  if(insertnewregion(&verifyinfo->regiontreeroot,
                       True, &nodecreated, &region) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

static Sint verifyapmpositions(const Keytype key, VISIT which,
                               /*@unused@ */ Uint depth,
                               void *info)
{
  if(which == postorder || which == leaf)
  {
    return verifytheapproximatematch((Verifyinfo *) info,(PairUint *) key);
  } else
  {
    return 0;
  }
}

static Uint getoptsplit(BOOL doedist,
                        Uint spliterrorbound,
                        Uint numofchars, 
                        Uint textlen, 
                        Uint patternlength,
                        Uint threshold)
{
  Uint optsplit;

  if(threshold * spliterrorbound >= patternlength)
  {
    optsplit = threshold;
  } else
  {
    double logtextlen, lognumofchars;

    logtextlen = log((double) textlen);
    lognumofchars = log((double) numofchars);
    if(doedist)
    {
      optsplit = (Uint) ((patternlength + threshold) / 
                         (logtextlen / lognumofchars));
    } else
    {
      optsplit = (Uint) (patternlength / (logtextlen / lognumofchars));
    }
    if(optsplit > threshold + 1)
    {
      optsplit = threshold + 1;
    }
  }
  while(patternlength > DPWORDSIZE4 * optsplit)
  {
    optsplit++;
  }
  return optsplit;
}

static Sint processmatchintervalwithstore(void *info,PairUint *leftright)
{
  Verifyinfo *verifyinfo = (Verifyinfo *) info;
  Uint i;
  Sint retcode;

  for(i=leftright->uint0; i<=leftright->uint1; i++)
  {
    retcode = storeapmposition(verifyinfo->virtualtree->suftab[i],
                               verifyinfo->patternlength,
                               info);
    if(retcode < 0 || retcode == SintConst(1))
    {
      return retcode;
    }
  }
  return 0;
}

static Sint realsplitesaapm(Verifyinfo *verifyinfo,
                            Uint splitlen,
                            Uint splitthreshold)
{
  Uint poffset;
  Sint retcode;

  for(poffset = 0; poffset < verifyinfo->patternlength - splitlen + 1; 
      poffset += splitlen)
  {
    if(verifyinfo->doedist)
    {
      verifyinfo->regionoffsetstart = verifyinfo->threshold + poffset;
      verifyinfo->regionoffsetend = verifyinfo->patternlength + 
                                    verifyinfo->threshold - poffset;
    } else
    {
      verifyinfo->regionoffsetstart = poffset;
      verifyinfo->regionoffsetend = verifyinfo->patternlength - poffset;
    }
    if(verifyinfo->threshold == 0)
    {
      if(computeofflineexactmatches(verifyinfo->virtualtree,
                                    verifyinfo->pattern,
                                    verifyinfo->patternlength,
                                    processmatchintervalwithstore,
                                    verifyinfo) != 0)
      {
        return (Sint) -1;
      }
    } else
    {
      if(verifyinfo->doedist)
      {
        getEqs4(verifyinfo->Eqs4split, 
                (Uint) EQSSIZE, 
                verifyinfo->pattern + poffset, 
                splitlen);
        retcode = esaapm(storeapmposition,
                         verifyinfo,
                         verifyinfo->virtualtree,
                         verifyinfo->Eqs4split, 
                         splitlen,
                         splitthreshold);
      } else
      {
        retcode = esahamming(storeapmposition,
                             verifyinfo,
                             verifyinfo->virtualtree,
                             verifyinfo->pattern + poffset,
                             splitlen,
                             splitthreshold);
      }
      if(retcode < 0 || retcode == SintConst(1))
      {
        return retcode;
      }
    }
  }
#ifdef DEBUG
  if(verifyregiontree(verifyinfo->regiontreeroot,
                      verifyinfo->virtualtree->multiseq.totallength,
                      &verifyinfo->originalregions) != 0)
  {
    return (Sint) -3;
  }
#endif
  if(verifyinfo->doedist)
  {
    getEqsrev4(verifyinfo->Eqs4split,
               (Uint) EQSSIZE,
               verifyinfo->pattern,
               verifyinfo->patternlength);
  }
  retcode = redblacktreewalkwithstop(verifyinfo->regiontreeroot,
                                     verifyapmpositions, verifyinfo);
  if(verifyinfo->doedist)
  {
    FREESPACE(verifyinfo->dcol);
  }
#ifdef DEBUG
  FREEARRAY(&verifyinfo->originalregions, PairUint);
#endif
  redblacktreedestroy(True, NULL, NULL, verifyinfo->regiontreeroot);
  DEBUG2(2,"# dpwork = %lu(%.2f)\n",
          (Showuint) verifyinfo->dpwork,
          (double) verifyinfo->dpwork /
                   verifyinfo->virtualtree->multiseq.totallength);
  return retcode;
}

Sint splitesaapm(BOOL doedist,
                 Sint (*processapmstartpos) (Uint, Uint, void *),
                 void *apmoutinfo,
                 Virtualtree *virtualtree,
                 DPbitvector4 *Eqs4split,
                 Uint threshold, 
                 Uchar *pattern, 
                 Uint patternlength)
{
  Verifyinfo verifyinfo;
  Uint splitlen, splitsize, splitthreshold;
  Sint retcode;

  verifyinfo.virtualtree = virtualtree;
  verifyinfo.pattern = pattern;
  verifyinfo.patternend = pattern + patternlength - 1;
  verifyinfo.patternlength = patternlength;
  verifyinfo.threshold = threshold;
  verifyinfo.apmoutinfo = apmoutinfo;
  verifyinfo.processapmstartpos = processapmstartpos;
  verifyinfo.dpwork = 0;
  verifyinfo.Eqs4split = Eqs4split;
  verifyinfo.doedist = doedist;
  if(doedist)
  {
    verifyinfo.maxwidthofregiontoverify = patternlength + threshold;
  } else
  {
    verifyinfo.maxwidthofregiontoverify = patternlength;
  }

  if(threshold >= patternlength)
  {
    ERROR2("threshold=%lu>=%lu=patternlen not allowed",
           (Showuint) threshold, (Showuint) patternlength);
    return (Sint) -1;
  }
  splitsize = getoptsplit(doedist,
                          UintConst(10),
                          virtualtree->alpha.mapsize - 1,
                          virtualtree->multiseq.totallength,
                          patternlength, threshold);
  
  if(splitsize > UintConst(1))
  {
    verifyinfo.regiontreeroot = NULL;
    if(doedist)
    {
      ALLOCASSIGNSPACE(verifyinfo.dcol, NULL, Uint, patternlength + 1);
    }

#ifdef DEBUG
    INITARRAY(&verifyinfo.originalregions, PairUint);
#endif
  }
  splitlen = patternlength / splitsize;
  splitthreshold = threshold / splitsize;
  DEBUG4(2,"# patternlength=%lu, splitsize=%lu, "
           "splitlen=%lu, splitthreshold=%lu\n",
         (Showuint) patternlength,
         (Showuint) splitsize,
         (Showuint) splitlen, (Showuint) splitthreshold);
  if(splitsize == UintConst(1))
  {
    if(doedist)
    {
      getEqs4(Eqs4split,(Uint) EQSSIZE,pattern,patternlength);
      retcode = esaapm(processapmstartpos,
                       apmoutinfo,
                       virtualtree,
                       Eqs4split,
                       patternlength,
                       threshold);
    } else
    {
      retcode = esahamming(processapmstartpos,
                           apmoutinfo,
                           virtualtree,
                           pattern,
                           patternlength,
                           threshold);
    }
  } else
  {
    if(ISLARGEPATTERN4(splitlen))
    {
      NOTIMPLEMENTED; // large pattern after splitting
    }
    retcode = realsplitesaapm(&verifyinfo,
                              splitlen,
                              splitthreshold);
  }
  return retcode;
}
