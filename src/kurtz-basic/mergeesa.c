#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include "fhandledef.h"
#include "esastream.h"
#include "debugdef.h"

#include "emimergeesa.h"
#include "trieins-def.h"

#include "trieins.pr"
#include "handleesastream.pr"
#include "encodedseq.pr"
#include "alphabet.pr"

 DECLAREREADFUNCTION(Uint);

 DECLAREREADFUNCTION(Uchar);

 DECLAREREADFUNCTION(PairUint);

static void fillandinsert(Trierep *trierep,
                          Uint idx,
                          Uint suftabvalue,
                          Trienode *node,
                          Uint64 ident)
{
  Suffixinfo sinfo;

  sinfo.idx = idx;
  sinfo.startpos = suftabvalue;
#ifdef DEBUG
  sinfo.ident = ident;
#endif
  insertsuffixintotrie(trierep,node,&sinfo);
}


static Sint inputthesequences(Alphabet *alpha,
                              Uint *nextpostable,
                              Esastream *esastreamtable,
                              Encodedsequence *encseqtable,
                              const char **indexnamelist,
                              Uint numofindexes,
                              Uint demand)
{
  Uint idx;
  
  for(idx=0; idx<numofindexes; idx++)
  {
    if(initesastream(&esastreamtable[idx],
                     indexnamelist[idx],
                     demand) != 0)
    {
      return (Sint) -2;
    }
    if(demand & TISTABSTREAM)
    {
      if(initencodedseq(&encseqtable[idx],
                        &esastreamtable[idx].tistabstream,
                        (Uint64) esastreamtable[idx].multiseq.totallength,
                        &esastreamtable[idx].multiseq.specialcharinfo,
                        &esastreamtable[idx].alpha,
                        indexnamelist[idx]) != 0)
      {
        return (Sint) -3;
      }
    }
    nextpostable[idx] = 0;
  }
  copyAlphabet(alpha,&esastreamtable[0].alpha);
  return 0;
}

static Sint insertfirstsuffixes(Trierep *trierep,
                                Uint *nextpostable,
                                Esastream *esastreamtable,
                                Uint numofindexes)
{
  Uint idx, suftabvalue = 0;
  Sint retval;

  for(idx=0; idx<numofindexes; idx++)
  {
    retval = readnextUint(&suftabvalue,&esastreamtable[idx].suftabstream);
    if(retval < 0)
    {
      return (Sint) -1; 
    }
    if(retval == 0)
    {
      ERROREOF("suftab");
      return (Sint) -2;
    }
    nextpostable[idx]++;
    fillandinsert(trierep,
                  idx,
                  suftabvalue,
                  trierep->root,
                  (Uint64) idx);
  }
  return 0;
}

/*@null@*/ static Trienode *findlargestnodeleqlcpvalue(Trienode *smallest,
                                                       Uint lcpvalue)
{
  Trienode *tmp;

  for(tmp = smallest->parent; tmp != NULL; tmp = tmp->parent)
  {
    if(tmp->depth <= lcpvalue)
    {
      return tmp;
    }
  }
  ERROR1("path does not contain a node of depth <= %lu",
          (Showuint) lcpvalue);
  return NULL;
}

Sint stepdeleteandinsertothersuffixes(Emissionmergedesa *emmesa)
{
  Trienode *tmpsmallestleaf, *tmplcpnode;
  PairUint tmpexception;
  Uchar tmpsmalllcpvalue;
  Sint retval;
  Uint tmpsuftabvalue = 0,
       tmpidx, 
       tmplcpvalue,
       tmplastbranchdepth;
  
  for(emmesa->buf.nextstoreidx = 0;
      emmesa->numofentries > 0 &&
      emmesa->buf.nextstoreidx < (Uint) SIZEOFMERGERESULTBUFFER;
      emmesa->buf.nextstoreidx++)
  {
    tmpsmallestleaf = findsmallestnodeintrie(&emmesa->trierep);
    tmplastbranchdepth = tmpsmallestleaf->parent->depth;
    tmpidx = tmpsmallestleaf->suffixinfo.idx;
    emmesa->buf.suftabstore[emmesa->buf.nextstoreidx].idx = tmpidx;
    emmesa->buf.suftabstore[emmesa->buf.nextstoreidx].startpos
      = tmpsmallestleaf->suffixinfo.startpos;
    if(emmesa->nextpostable[tmpidx] > 
       (Uint) ACCESSSEQUENCELENGTH(emmesa->encseqtable + tmpidx))
    {
      deletesmallestpath(tmpsmallestleaf,&emmesa->trierep);
      emmesa->numofentries--;
    } else
    {
      retval = readnextUchar(&tmpsmalllcpvalue,
                             &emmesa->esastreamtable[tmpidx].lcptabstream);
      if(retval < 0)
      {
        return (Sint) -1; 
      }
      if(retval == 0)
      {
        ERROREOF("lcptab");
        return (Sint) -2;
      }
      if(tmpsmalllcpvalue == UCHAR_MAX)
      {
        retval = readnextPairUint(&tmpexception,
                                  &emmesa->esastreamtable[tmpidx].llvtabstream);
        if(retval < 0)
        {
          return (Sint) -3; 
        }
        if(retval == 0)
        {
          ERROREOF("llvtab");
          return (Sint) -3;
        }
        tmplcpvalue = tmpexception.uint1;
      } else
      {
        tmplcpvalue = (Uint) tmpsmalllcpvalue;
      }
      if(tmplcpvalue > tmplastbranchdepth)
      {
        tmplastbranchdepth = tmplcpvalue;
      }
      tmplcpnode = findlargestnodeleqlcpvalue(tmpsmallestleaf,tmplcpvalue);
      retval = readnextUint(&tmpsuftabvalue,
                            &emmesa->esastreamtable[tmpidx].suftabstream);
      if(retval < 0)
      {
        return (Sint) -4;
      }
      if(retval == 0)
      {
        ERROREOF("suftab");
        return (Sint) -5;
      }
      emmesa->nextpostable[tmpidx]++;
      fillandinsert(&emmesa->trierep,
                    tmpidx,
                    tmpsuftabvalue,
                    tmplcpnode,
                    emmesa->ident++);
      tmpsmallestleaf = findsmallestnodeintrie(&emmesa->trierep);
      deletesmallestpath(tmpsmallestleaf,&emmesa->trierep);
    }
    if(emmesa->numofentries > 0)
    {
      emmesa->buf.lcptabstore[emmesa->buf.nextstoreidx]
        = tmplastbranchdepth;
      emmesa->buf.lastpage = False;
    } else
    {
      emmesa->buf.lastpage = True;
    }
  }
  return 0;
}

Sint initEmissionmergedesa(Emissionmergedesa *emmesa,
                           const char **indexnamelist,
                           Uint numofindexes,
                           Uint demand)
{
  emmesa->buf.nextaccessidx = emmesa->buf.nextstoreidx = 0;
  emmesa->numofindexes = numofindexes;
  emmesa->numofentries = numofindexes;
  emmesa->ident = (Uint64) numofindexes;
  ALLOCASSIGNSPACE(emmesa->esastreamtable,NULL,Esastream,numofindexes);
  if(demand & TISTABSTREAM)
  {
    ALLOCASSIGNSPACE(emmesa->encseqtable,NULL,Encodedsequence,numofindexes);
  } else
  {
    emmesa->encseqtable = NULL;
  }
  if(numofindexes > UintConst(1))
  {
    emmesa->trierep.encseqtable = emmesa->encseqtable;
  }
  ALLOCASSIGNSPACE(emmesa->nextpostable,NULL,Uint,numofindexes);
  if(inputthesequences(&emmesa->alpha,
                       emmesa->nextpostable,
                       emmesa->esastreamtable,
                       emmesa->encseqtable,
                       indexnamelist,
                       numofindexes,
                       demand) != 0)
  {
    return (Sint) -1;
  }
  if(numofindexes > UintConst(1))
  {
    inittrienodetable(&emmesa->trierep,numofindexes,numofindexes);
    if(insertfirstsuffixes(&emmesa->trierep,
                           emmesa->nextpostable,
                           emmesa->esastreamtable,numofindexes) != 0)
    {
      return (Sint) -2;
    }
  }
  return 0;
}

Sint wraptEmissionmergedesa(Emissionmergedesa *emmesa)
{
  Uint idx;

  for(idx = 0; idx < emmesa->numofindexes; idx++)
  {
    if(closeesastream(emmesa->esastreamtable + idx) != 0)
    {
      return (Sint) -5;
    }
    if(emmesa->encseqtable != NULL)
    {
      freeEncodedsequence(emmesa->encseqtable + idx);
    }
  }
  FREESPACE(emmesa->encseqtable);
  if(emmesa->numofindexes > UintConst(1))
  {
    freetrierep(&emmesa->trierep);
  }
  FREESPACE(emmesa->esastreamtable);
  FREESPACE(emmesa->nextpostable);
  return 0;
}
