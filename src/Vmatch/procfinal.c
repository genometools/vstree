
#include <stdio.h>
#include "types.h"
#include "multidef.h"
#include "debugdef.h"
#include "minmax.h"
#include "match.h"
#include "codondef.h"
#include "genfile.h"

#include "mparms.h"
#include "procmultiseq.h"
#include "matchstate.h"

#include "evalues.pr"
#include "redblack.pr"
#include "codon.pr"

#include "smcontain.pr"
#include "matsort.pr"
#include "bestmatch.pr"
#include "sixframe.pr"
#include "multiseq-adv.pr"
#include "mokay.pr"

/*
static void checkmaximality(Matchstate *matchstate,Match *match)
{
  if(match->position1 > 0 && match->position2 > 0)
  {
    Uchar a, b;
    a = matchstate->virtualtree->multiseq.sequence[match->position1-1]; 
    b = matchstate->virtualtree->multiseq.sequence[match->position2-1];
    if(a == b)
    {
      fprintf(stderr,"not left maximal\n");
      exit(EXIT_FAILURE);
    }
    printf(" a=%lu, b=%lu\n",(Showuint) a,(Showuint) b);
  }
  if(match->position1 + match->length1 < 
     matchstate->virtualtree->multiseq.totallength &&
     match->position2 + match->length2 < 
     matchstate->virtualtree->multiseq.totallength)
  {
    Uchar a, b;
    a = matchstate->virtualtree->multiseq.sequence[match->position1+
                                                   match->length2]; 
    b = matchstate->virtualtree->multiseq.sequence[match->position2+
                                                   match->length2];
    if(a == b)
    {
      fprintf(stderr,"not right maximal\n");
      exit(EXIT_FAILURE);
    }
    printf(" a=%lu, b=%lu\n",(Showuint) a,(Showuint) b);
  }
}
*/

typedef struct
{
  Uint udfsortmode,
       outinfosortmode;
  Processmatch *processmatch;
  Multiseq *constvms,
           *constqms;
  BestMatchlist *bestmatchlistref;
  ArrayStoreMatch matchtab;
} BMcollectvalues;

static Sint fetchpositions(Multiseq *virtualmultiseq,
                           Multiseq *querymultiseq,
                           Match *match,
                           Seqinfo *seqinfo1)
{
  Uint seqstartpos2, // start position of sequence 2
       seqlength2;   // length of sequence 2
/*
  Invariant: The following component of match are defined:
             match->flag
             match->distance
             match->position1
             match->length1
             match->length2
             So match->position2 is the only component not defined.
             if query is not null, then
             seqnum2 and 
             relpos2 also need to be defined.
*/
/*
  Since it might be necessary to output the sequence number and the relative
  positions, we first fetch the sequence information for position1.
*/
  DEBUG2(3,"fetchposition:m1=(%lu,%lu),",
            (Showuint) match->length1,
            (Showuint) match->position1);
  DEBUG2(3,"m2=(%lu,%lu)\n",
            (Showuint) match->length2,
            (Showuint) match->position2);

  if(getseqinfo(virtualmultiseq,seqinfo1,match->position1) != 0)
  {
    return (Sint) -1;
  }
  if(querymultiseq == NULL)
  {
/*
    self comparison => position2 is defined => fetch sequence number and
    relative positions.
*/
    if(match->flag & FLAGREGEXPMATCH)
    {
      seqstartpos2 = 0;
      seqlength2 = 0;
    } else
    {
      Seqinfo seqinfo2;
      if(getseqinfo(virtualmultiseq,&seqinfo2, match->position2) != 0)
      {
        return (Sint) -1;
      }
      seqstartpos2 = seqinfo2.seqstartpos;
      seqlength2 = seqinfo2.seqlength;
      match->seqnum2 = seqinfo2.seqnum;
      match->relpos2 = seqinfo2.relposition;
    }
  } else
  {
/*
    match query against database => position2 is undefined but queryseq
    and query start are defined. So determine the correct values of
    seqinfo2 and from this derive position2.
*/
    PairUint bnds2;

    if(findboundaries(querymultiseq,
                      match->seqnum2,
                      &bnds2) != 0)
    {
      return (Sint) -1;
    }
    seqstartpos2 = bnds2.uint0;
    seqlength2 = bnds2.uint1 - bnds2.uint0 + 1;
    match->position2 = seqstartpos2 + match->relpos2;
    DEBUG1(3,"position2=%lu,",(Showuint) match->position2);
    DEBUG1(3,"seqnum2=%lu,",(Showuint) match->seqnum2);
    DEBUG1(3,"seqlength2=%lu,",(Showuint) seqlength2);
    DEBUG1(3,"seqstartpos2=%lu,",(Showuint) seqstartpos2);
    DEBUG1(3,"seqrelposition2=%lu\n",(Showuint) match->relpos2);
  }
  if(match->flag & FLAGPALINDROMIC)
  {
/*
    adjust position2 and the corresponding relative position
*/
    match->relpos2 = seqlength2 - (match->relpos2+match->length2);
    match->position2 = seqstartpos2 + match->relpos2;
    if(match->flag & FLAGSELFPALINDROMIC)
    {
      if(seqinfo1->seqnum > match->seqnum2 || 
         (seqinfo1->seqnum == match->seqnum2 && 
          seqinfo1->relposition > match->relpos2))
      {
        return (Sint) 1;
      }
    }
  }
  return 0;
}

Sint applymatchfun(Processmatch *processmatch,
                   Multiseq *constvms,
                   Multiseq *constqms,
                   StoreMatch *storematch)
{
#ifdef DEBUG
  if(processmatch->processfunction == NULL)
  {
    fprintf(stderr,"file %s, line %lu: processfunction is NULL\n",
             __FILE__,(Showuint) __LINE__);
    exit(EXIT_FAILURE);
  }
#endif
  DEBUG1(2,"call processmatch=%s\n",processmatch->functionname);
  if(processmatch->processfunction(processmatch->processinfo,
                                   constvms,
                                   constqms,
                                   storematch) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

static Sint assignEvalue(Evalues *evaluetableref,
                         Multiseq *constvms,
                         Multiseq *constqms,
                         StoreMatch *storematch)
{
  Evaluetype multiplier;
  Uint lenmatch;

  if(storematch->Storeflag & FLAGQUERY)
  {
    if(storematch->Storeflag & FLAGCOMPLETEMATCH)
    {
      multiplier = (Evaluetype) constvms->totallength;
    } else
    {
      if(storematch->Storeflag & FLAGSELFPALINDROMIC)
      {
        multiplier = 0.5 * 
                     (Evaluetype) constvms->totallength *
                     (Evaluetype) constqms->totallength;
      } else
      {
        Uint dblen, querylen;
        PairUint range;
        if(findboundaries(constqms, storematch->Storeseqnum2, &range) != 0)
        {
          return (Sint) -1;
        }
        dblen = constvms->totallength;
        querylen = range.uint1 - range.uint0 + 1;
        DEBUG2(3,"dblength = %lu, querylength = %lu\n",(Showuint) dblen
              , (Showuint) querylen);
        multiplier = (Evaluetype) dblen *
                     (Evaluetype) querylen;
      }
    }
  } else
  {
    if(HASINDEXEDQUERIES(constvms))
    {
      multiplier
        = (Evaluetype) DATABASELENGTH(constvms) *
          (Evaluetype) constvms->totalquerylength; // SETMULTIPLIER
    } else
    {
      multiplier
        = 0.5 * 
          (Evaluetype) constvms->totallength *
          (Evaluetype) constvms->totallength;  // SETMULTIPLIER
    }
  }
  if((storematch->Storeflag & FLAGCOMPLETEMATCH) || 
     storematch->Storedistance == 0)
  {
    lenmatch = storematch->Storelength2;
  } else
  {
    lenmatch = MAX(storematch->Storelength1,storematch->Storelength2);
  }
  storematch->StoreEvalue = incgetEvalue(evaluetableref,
                                         multiplier,
                                         storematch->Storedistance,
                                         lenmatch);
  return 0;
}

static Sint dnavsprotfromsixframetooriginalquery(StoreMatch *storematch,
                                                 Uint transnum,
                                                 Multiseq *constqms)
{
  BOOL reversematch;
  Uint dnarelpos, dnastartpos;

  if(sixframeconvertmatch(constqms,
                          &reversematch,
                          &dnarelpos,
                          &dnastartpos,
                          storematch->Storelength2,
                          storematch->Storeseqnum2,
                          storematch->Storerelpos2) != 0)
  {
    return (Sint) -1;
  }
  storematch->Storeflag |= MAKEFLAGCODONMATCH(transnum);
  if(reversematch)
  {
    storematch->Storeflag |= FLAGPPRIGHTREVERSE;
  }
  storematch->Storerelpos2 = dnarelpos;
  storematch->Storeposition2 = dnastartpos + dnarelpos;
  storematch->Storeseqnum2 = storematch->Storeseqnum2/MAXFRAMES;
  storematch->Storelength2 = storematch->Storelength2 * CODONLENGTH;
  return 0;
}

static Sint protvsprotfromsixframetoorigdna(StoreMatch *storematch,
                                            Uint transnum,
                                            Multiseq *constvms)
{
  BOOL reversematch;
  Uint dnarelpos, dnastartpos;

  storematch->Storeflag |= MAKEFLAGCODONMATCH(transnum);
  if(sixframeconvertmatch(constvms,
                          &reversematch,
                          &dnarelpos,
                          &dnastartpos,
                          storematch->Storelength1,
                          storematch->Storeseqnum1,
                          storematch->Storerelpos1) != 0)
  {
    return (Sint) -1;
  }
  if(reversematch)
  {
    storematch->Storeflag |= FLAGPPLEFTREVERSE;
  }
  storematch->Storerelpos1 = dnarelpos;
  storematch->Storeposition1 = dnastartpos + dnarelpos;
  storematch->Storeseqnum1 = storematch->Storeseqnum1/MAXFRAMES;
  storematch->Storelength1 = storematch->Storelength1 * CODONLENGTH;

  if(sixframeconvertmatch(constvms,
                          &reversematch,
                          &dnarelpos,
                          &dnastartpos,
                          storematch->Storelength2,
                          storematch->Storeseqnum2,
                          storematch->Storerelpos2) != 0)
  {
    return (Sint) -2;
  }
  if(reversematch)
  {
    storematch->Storeflag |= FLAGPPRIGHTREVERSE;
  }
  storematch->Storerelpos2 = dnarelpos;
  storematch->Storeposition2 = dnastartpos + dnarelpos;
  storematch->Storeseqnum2 = storematch->Storeseqnum2/MAXFRAMES;
  storematch->Storelength2 = storematch->Storelength2 * CODONLENGTH;
  return 0;
}

#ifdef DEBUG
static Sint checkcodonmatch(StoreMatch *storematch,
                            Supermatchtask supermatchtask,
                            Uint transnum,
                            Multiseq *constvms,
                            Multiseq *constqms,
                            Alphabet *virtualalpha)
{
  if(storematch->Storedistance == 0)
  {
    if(supermatchtask == Querytranslatedmatch)
    {
      if(storematch->Storelength1 * CODONLENGTH != storematch->Storelength2)
      {
        ERROR2("different lengths in exact match: length1*3=%lu != %lu=length2",
                (Showuint) (storematch->Storelength1 * CODONLENGTH),
                (Showuint) storematch->Storelength2);
        return (Sint) -1;
      }
      if(checkproteindnamatch(virtualalpha,
                              transnum,
                              // (storematch->Storeflag & FLAGPALINDROMIC) ?
                              (storematch->Storeflag & FLAGPPRIGHTREVERSE) ?
                                False : True,
                               constvms->originalsequence + 
                                 storematch->Storeposition1,
                               storematch->Storelength1,
                               constqms->originalsequence 
                                 + storematch->Storeposition2,
                               storematch->Storelength2) != 0)
      {
        return (Sint) -2;
      }
      DEBUG0(2,"checkproteindnamatch okay\n");
    } else
    {
      if(storematch->Storelength1 != storematch->Storelength2)
      {
        ERROR2("different lengths in exact match: length1=%lu != %lu=length2",
                (Showuint) storematch->Storelength1,
                (Showuint) storematch->Storelength2);
        return (Sint) -1;
      }
      if(supermatchtask == Protprotmatchondna)
      {
        if(checkprotprotmatchondna(virtualalpha,
                                   transnum,
                                   (storematch->Storeflag & FLAGPPLEFTREVERSE) 
                                     ? False 
                                     : True,
                                   (storematch->Storeflag & FLAGPPRIGHTREVERSE) 
                                     ? False 
                                     : True,
                                   constvms->originalsequence 
                                   + storematch->Storeposition1,
                                   storematch->Storelength1,
                                   constvms->originalsequence 
                                   + storematch->Storeposition2) != 0)
        {
          return (Sint) -1;
        }
        DEBUG0(2,"checkprotprotmatchondna okay\n");
      }
    }
  }
  return 0;
}
#endif

static Sint convertthematch(StoreMatch *storematch,
                            Supermatchtask supermatchtask,
                            Uint querysubstringoffset,
                            BOOL revmposorder,
                            Multiseq *virtualmultiseq,
                            Multiseq *constvms,
                            Multiseq *constqms,
                            Multiseq *constq6frms,
                            Uint transnum,
                            Match *match,
                            Uint seqnum1,
                            Uint relpos1,
                            Uint onlinequerynumoffset)
{
  storematch->Storeflag = match->flag;
  storematch->Storedistance = match->distance;
  if(revmposorder)
  {
    storematch->Storelength2 = match->length1;
    storematch->Storerelpos2 = relpos1;
    storematch->Storeseqnum2 = onlinequerynumoffset;
    if(onlinequerynumoffset == 0)
    {
      storematch->Storeposition2 = relpos1;
    } else
    {
      if(supermatchtask == Querytranslatedmatch)
      {
        storematch->Storeposition2 
          = relpos1 + constq6frms->markpos.spaceUint[onlinequerynumoffset-1]+ 1;
      } else
      {
        storematch->Storeposition2
          = relpos1 + constqms->markpos.spaceUint[onlinequerynumoffset-1] + 1;
      }
    }
    storematch->Storeposition1 = match->position2 + querysubstringoffset;
    storematch->Storelength1 = match->length2;
    storematch->Storeseqnum1 = match->seqnum2;
    storematch->Storerelpos1 = match->relpos2 + querysubstringoffset;
  } else
  {
    storematch->Storeposition1 = match->position1;
    storematch->Storelength1 = match->length1;
    storematch->Storeseqnum1 = seqnum1;
    storematch->Storerelpos1 = relpos1;
    if(match->flag & FLAGQUERY)
    {
      storematch->Storelength2 = match->length2;
      storematch->Storeseqnum2 = match->seqnum2;
      storematch->Storeposition2 = match->position2 + querysubstringoffset;
      storematch->Storerelpos2 = match->relpos2 + querysubstringoffset;
    } else
    {
      storematch->Storelength2 = match->length2;
      if(HASINDEXEDQUERIES(virtualmultiseq))
      {
        storematch->Storeseqnum2
          = match->seqnum2 - NUMOFDATABASESEQUENCES(virtualmultiseq);
        storematch->Storeposition2 
          = match->position2 - DATABASELENGTH(virtualmultiseq) - 1;
      } else
      {
        storematch->Storeseqnum2 = match->seqnum2;
        storematch->Storeposition2 = match->position2;
      }
      storematch->Storerelpos2 = match->relpos2 + querysubstringoffset;
    }
  }
  if(supermatchtask == Querytranslatedmatch)
  {
    if(dnavsprotfromsixframetooriginalquery(storematch,
                                            transnum,
                                            constqms) != 0)  // original dna
    {
      return (Sint) -1;
    }
  } else
  {
    if(supermatchtask == Protprotmatchondna)
    {
      if(protvsprotfromsixframetoorigdna(storematch,
                                         transnum,
                                         constvms) != 0)
      {
        return (Sint) -2;
      }
    }
  }
  return 0;
}

/*
  The following function is called whenever a match ist found,
  namely in the f-modules: fcomplete.c, fquery.c, fself.c, and 
  ftandem.c. It is not called directly in fsuper.c and fmumself.c
  since these are handled in the same way as the functions
  in fself.
  fetchpositions fills and adjusts the remaining components of the 
  match-structure. The matches are then converted using the function
  convertthematch. Then it is checked if the selectionfunction bundle
  is defined. If yes, then the function selectmatch from the 
  function bundle is called. If the return value is 1, then
  the it is checked if the match fullfill all constraints according to
  the parameters given by the user.
*/

Sint processfinal(void *info,Match *match)
{
  Matchstate *matchstate = (Matchstate *) info;

  Seqinfo seqinfo1;
  StoreMatch storematch;
  Sint ret;
  Procmultiseq *procmultiseq;

  ret = fetchpositions(&matchstate->virtualtree->multiseq,
                       (matchstate->queryinfo == NULL) 
                        ? NULL : matchstate->queryinfo->multiseq,
                       match,
                       &seqinfo1);
  if(ret != 0)
  {
    if(ret < 0)
    {
      return (Sint) -1;
    }
    return 0;
  }
  procmultiseq = (Procmultiseq *) matchstate->procmultiseq;
  if(convertthematch(&storematch,
                     procmultiseq->supermatchtask,
                     (matchstate->queryinfo == NULL) 
                       ? 0 : matchstate->queryinfo->querysubstringoffset,
                     matchstate->revmposorder,
                     &matchstate->virtualtree->multiseq,
                     procmultiseq->constvms,
                     procmultiseq->constqms,
                     procmultiseq->constq6frms,
                     matchstate->transnum,
                     match,
                     seqinfo1.seqnum,
                     seqinfo1.relposition,
                     matchstate->onlinequerynumoffset) != 0)
  {
    return (Sint) -2;
  }
#ifdef DEBUG
  if(checkcodonmatch(&storematch,
                     procmultiseq->supermatchtask,
                     matchstate->transnum,
                     procmultiseq->constvms,
                     procmultiseq->constqms,
                     &matchstate->virtualtree->alpha) != 0)
  {
    return (Sint) -3;
  }
#endif
  if(matchstate->shownoevalue)
  {
    storematch.StoreEvalue = 0.0;
  } else
  {
    if(assignEvalue(matchstate->evalues,
                    procmultiseq->constvms,
                    procmultiseq->constqms,
                    &storematch) != 0)
    {
      return (Sint) -3;
    }
  }
  storematch.idnumber = matchstate->currentidnumber++;
  if(matchstate->selectbundle == NULL ||
     matchstate->selectbundle->selectmatch == NULL)
  {
    ret = (Sint) 1;
  } else
  {
    ret = matchstate->selectbundle->selectmatch(
                           &matchstate->virtualtree->alpha,
                           procmultiseq->constvms,
                           procmultiseq->constqms,
                           &storematch);
  }
  if(ret < 0)
  {
    matchstate->currentidnumber--;
    return (Sint) -4;
  }
  if(ret == (Sint) 1 && matchokay(storematch.Storelength1,
                                  storematch.Storeposition1,
                                  storematch.Storelength2,
                                  storematch.Storeposition2,
                                  storematch.Storedistance,
                                  storematch.StoreEvalue,
                                  storematch.Storeflag,
                                  &matchstate->matchparam))
  {
    if(matchstate->bestflag == Fixednumberofbest)
    {
      if(insertintobml(&procmultiseq->bestmatchlist,
                       &storematch) != 0)
      {
        matchstate->currentidnumber--;
        return (Sint) -5;
      }
    } else
    {
      if(matchstate->domatchbuffering)
      {
        STOREINARRAY(&procmultiseq->matchbuffer,
                     StoreMatch,128,storematch);
      } else
      { 
        if(applymatchfun(&procmultiseq->processmatch,
                         procmultiseq->constvms,
                         procmultiseq->constqms,
                         &storematch) != 0)
        {
          matchstate->currentidnumber--;
          return (Sint) -6;
        }
      }
    }
  } else
  {
    matchstate->currentidnumber--;
  }
  return 0;
}

Sint iterapplymatchfun(Processmatch *processmatch,
                       Multiseq *constvms,
                       Multiseq *constqms,
                       ArrayStoreMatch *matchtab)
{
  StoreMatch *mspaceptr;

  for(mspaceptr = matchtab->spaceStoreMatch; 
      mspaceptr < matchtab->spaceStoreMatch + 
                  matchtab->nextfreeStoreMatch;
      mspaceptr++)
  {
    if(applymatchfun(processmatch,
                     constvms,
                     constqms,
                     mspaceptr) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

static Sint collectorprocessStoreMatch(const Keytype bmkey,
                                       VISIT which,
                                       /*@unused@*/ Uint depth,
                                       void *collectinfo)
{
  if(which == postorder || which == leaf)
  {
    BMcollectvalues *bmcollectvalues = (BMcollectvalues *) collectinfo;
    StoreMatch *bmptr, *mspaceptr;

    bmptr = bmcollectvalues->bestmatchlistref->bmreservoir.spaceStoreMatch 
              + (Uint) bmkey;
    if(bmcollectvalues->outinfosortmode == bmcollectvalues->udfsortmode)
    {
      if(applymatchfun(bmcollectvalues->processmatch,
                       bmcollectvalues->constvms,
                       bmcollectvalues->constqms,
                       bmptr) != 0)
      {
        return (Sint) -2;
      }
    } else
    {
      GETNEXTFREEINARRAY(mspaceptr,
                         &bmcollectvalues->matchtab,
                         StoreMatch,
                         1024);
      ASSIGNSTOREMATCH(mspaceptr,bmptr);
    }
  } 
  return 0;
}

Sint showbestmatchlist(Multiseq *constvms,
                       Multiseq *constqms,
                       BestMatchlist *bestmatchlist,
                       Uint outinfosortmode,
                       Processmatch *processmatch,
                       Showverbose showverbose)
{
  BMcollectvalues bmcollectvalues;
    
  bmcollectvalues.udfsortmode = undefsortmode();
  bmcollectvalues.outinfosortmode = outinfosortmode;
  bmcollectvalues.processmatch = processmatch;
  bmcollectvalues.constvms = constvms;
  bmcollectvalues.constqms = constqms;
  bmcollectvalues.bestmatchlistref = bestmatchlist;
  if(outinfosortmode != bmcollectvalues.udfsortmode)
  {
    INITARRAY(&bmcollectvalues.matchtab,StoreMatch);
  }
  if(redblacktreewalkreverseorder(bestmatchlist->bmdict.root,
                                  collectorprocessStoreMatch,
                                  (void *) &bmcollectvalues) != 0)
  {
    return (Sint) -1;
  }
  if(outinfosortmode != bmcollectvalues.udfsortmode)
  {    
    Uint countremoved = removecontained(&bmcollectvalues.matchtab);
    if(showverbose != NULL)
    {
      char sbuf[80+1];
      sprintf(sbuf,"remove %lu contained matches",(Showuint) countremoved);
      showverbose(sbuf);
    }
    if(outinfosortmode != UintConst(2)) // ascending order of first position
    {
      sortallmatches(bmcollectvalues.matchtab.spaceStoreMatch,
                     bmcollectvalues.matchtab.nextfreeStoreMatch,
                     outinfosortmode);
    }
    if(iterapplymatchfun(processmatch,
                         constvms,
                         constqms,
                         &bmcollectvalues.matchtab) != 0)
    {
      return (Sint) -2;
    }
    FREEARRAY(&bmcollectvalues.matchtab,StoreMatch);
  }
  return 0;
}

void storebestmatchlist(ArrayStoreMatch *matchbuffer,
                        BestMatchlist *bestmatchlist)
{
  matchbuffer->spaceStoreMatch = bestmatchlist->bmreservoir.spaceStoreMatch;
  matchbuffer->nextfreeStoreMatch 
    = bestmatchlist->bmreservoir.nextfreeStoreMatch;
  matchbuffer->allocatedStoreMatch 
    = bestmatchlist->bmreservoir.nextfreeStoreMatch;
}
