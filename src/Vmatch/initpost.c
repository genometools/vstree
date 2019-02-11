
#include "types.h"
#include "debugdef.h"
#include "fhandledef.h"
#include "procmultiseq.h"

#include "outinfo.h"
#include "vmcldef.h"
#include "markinfo.h"
#include "chaincall.h"
#include "mcldef.h"
#include "mparms.h"

#include "dstrdup.pr"
#include "echomatch.pr"
#include "mlendistrib.pr"
#include "vmcluster.pr"
#include "nomatch.pr"
#include "showmasked.pr"
#include "procfinal.pr"
#include "markmat.pr"
#include "chainvm.pr"
#include "allmclust.pr"

#define DBMARKERMULTISEQ(MARKDB,COMPLETE,VIRTUALMS,QUERYMS)\
        (COMPLETE || MARKDB) ? VIRTUALMS : QUERYMS

static Sint markinfoinitgeneric(BOOL selfmatch,
                                BOOL iscompletematch,
                                Markinfo *markinfo,
                                Nomatch *nomatch,
                                Maskmatch *maskmatch,
                                Uint numofqueryfiles,
                                Outinfo *outinfo,
                                Procmultiseq *procmultiseq)
{
  markinfo->hasnoqueryfiles = (numofqueryfiles == 0) ? True : False;
  if(nomatch->donomatch)
  {
    markinfo->markfields = nomatch->markfields;
  } else
  {
    markinfo->markfields = maskmatch->markfields;
  }
  if(selfmatch)
  {
    initmarktable(&markinfo->markmatchtable,outinfo->outvms);
    if(!markinfo->markfields.markdb && 
       !HASINDEXEDQUERIES(outinfo->outvms))
    {
      if(nomatch->donomatch)
      {
        ERROR0("option -qnomatch requires index containing "
               "query sequences or option -q");
      } else
      {
        ERROR0("option -qmaskmatch requires index containing "
               "query sequences or option -q");
      }
      return (Sint) -1;
    }
  } else
  {
    Multiseq *msref;

    msref = DBMARKERMULTISEQ(markinfo->markfields.markdb,
                             iscompletematch,
                             outinfo->outvms,
                             outinfo->outqms);
    initmarktable(&markinfo->markmatchtable,msref);
  }
  ASSIGNPROCESSMATCH(&procmultiseq->processmatch,markinfo,markmatches);
  return 0;
}

Sint initdatastructuresvmatchgeneric(BOOL selfmatch,
                                     BOOL *processfunctionwasundefined,
                                     BOOL *ownpostprocessing,
                                     Clusterparms *clusterparms,
                                     Sequenceclusterinfo *seqclinfo,
                                     Outinfo *outinfo,
                                     BOOL iscompletematch,
                                     BOOL ispreinfomatch,
                                     Markinfo *markinfo,
                                     Nomatch *nomatch,
                                     Maskmatch *maskmatch,
                                     char *mfargs,
                                     Uint numofqueryfiles,
                                     BOOL domatchbuffering,
                                     Procmultiseq *procmultiseq)
{
  if(procmultiseq->processmatch.processfunction == NULL)
  {
    *processfunctionwasundefined = True;
    if(clusterparms->dbclusterdefined)
    {
      if(initvmcluster(seqclinfo,
                       clusterparms,
                       outinfo,
                       mfargs,
                       outinfo->outvms) != 0)
      {
        return (Sint) -1;
      }
      ASSIGNPROCESSMATCH(&procmultiseq->processmatch,seqclinfo,addvmcluster);
      *ownpostprocessing = True;
    } else
    {
      if(nomatch->donomatch || maskmatch->domaskmatch)
      {
        if(markinfoinitgeneric(selfmatch,
                               iscompletematch,
                               markinfo,
                               nomatch,
                               maskmatch,
                               numofqueryfiles,
                               outinfo,
                               procmultiseq) != 0)
        {
          return (Sint) -2;
        }
        *ownpostprocessing = True;
      } else
      {
        if(ispreinfomatch)
        {
          INITARRAY(&outinfo->matchcounttab,Uint);
          ASSIGNPROCESSMATCH(&procmultiseq->processmatch,
                             outinfo,immediatelycountmatch);
          *ownpostprocessing = True;
        } else
        {
          ASSIGNPROCESSMATCH(&procmultiseq->processmatch,outinfo,
                             immediatelyechothematch);
          if(outinfo->bestinfo.bestflag == Fixednumberofbest ||
             (outinfo->selectbundle != NULL &&
              outinfo->selectbundle->selectmatchFinaltable != NULL) ||
             domatchbuffering) 
          {
            *ownpostprocessing = True;
          } else
          {
            *ownpostprocessing = False;
          }
        }
      }
    }
  } else
  {
    *processfunctionwasundefined = False;
    *ownpostprocessing = False;
  }
  return 0;
}

static Sint markernomatchout(BOOL selfmatch,
                             BOOL iscompletematch,
                             Markinfo *markinfo,
                             Nomatch *nomatch,
                             Outinfo *outinfo,
                             Alphabet *alpha,
                             Showverbose showverbose)
{
  Multiseq *msref;
  Uint posoffset, seqnumoffset, len;

  if(selfmatch)
  {
    msref = outinfo->outvms;
    if(markinfo->markfields.markdb)
    {
      posoffset = 0;
      seqnumoffset = 0;
      len = DATABASELENGTH(outinfo->outvms);
    } else
    {
      posoffset = DATABASELENGTH(outinfo->outvms)+1;
      seqnumoffset = NUMOFDATABASESEQUENCES(outinfo->outvms);
      len = outinfo->outvms->totalquerylength;
    }
  } else
  {
    msref = DBMARKERMULTISEQ(nomatch->markfields.markdb,
                             iscompletematch,
                             outinfo->outvms,
                             outinfo->outqms);
    posoffset = 0;
    seqnumoffset = 0;
    len = msref->totallength;
  }
  if(nomatchsubstringsout(showverbose,
                          msref,
                          alpha,
                          markinfo->markmatchtable,
                          posoffset,
                          seqnumoffset,
                          len,
                          outinfo->outfp,
                          outinfo->showstring,
                          outinfo->showmode,
                          &outinfo->showdesc,
                          nomatch) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

static Sint markermaskmatchout(BOOL selfmatch,
                               BOOL iscompletematch,
                               Markinfo *markinfo,
                               Maskmatch *maskmatch,
                               Outinfo *outinfo,
                               Alphabet *alpha)
{
  if(maskmatch->markfields.markdb)
  {
    if(showmaskedseq(False,
                     outinfo->outfp,
                     outinfo->showstring & MAXLINEWIDTH,
                     NULL,
                     outinfo->outvms,
                     markinfo->markmatchtable,
                     maskmatch->maskchar) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    if(selfmatch)
    {
      NOTIMPLEMENTED; // maskmatch for query sequence in index
    } else
    {
      BOOL transform;
      Multiseq *msref;
  
      msref = DBMARKERMULTISEQ(False,
                               iscompletematch,
                               outinfo->outvms,
                               outinfo->outqms);
      if(msref->originalsequence == NULL)
      {
        msref->originalsequence = msref->sequence;
        transform = True;
      } else
      {
        transform = False;
      }
      if(showmaskedseq(transform,
                       outinfo->outfp,
                       outinfo->showstring & MAXLINEWIDTH,
                       &alpha->characters[0],
                       msref,
                       markinfo->markmatchtable,
                       maskmatch->maskchar) != 0)
      {
        return (Sint) -2;
      }
      if(transform)
      {
        msref->originalsequence = NULL;
      }
    }
  }
  return 0;
}

Sint postprocessvmatchgeneric(BOOL selfmatch,
                              BOOL processfunctionwasundefined,
                              Clusterparms *clusterparms,
                              Chaincallinfo *chaincallinfo,
                              Matchclustercallinfo *matchclustercallinfo,
                              Sequenceclusterinfo *seqclinfo,
                              Outinfo *outinfo,
                              char *mfargs,
                              BOOL domatchbuffering,
                              BOOL iscompletematch,
                              BOOL ispreinfomatch,
                              Markinfo *markinfo,
                              Nomatch *nomatch,
                              Maskmatch *maskmatch,
                              Matchparam *matchparam,
                              Alphabet *alpha,
                              Procmultiseq *procmultiseq,
                              Showverbose showverbose)
{
  if(outinfo->bestinfo.bestflag == Fixednumberofbest)
  {
    if(domatchbuffering)
    {
      storebestmatchlist(&procmultiseq->matchbuffer,
                         &procmultiseq->bestmatchlist);
    } else
    {
      if(showbestmatchlist(outinfo->outvms,
                           outinfo->outqms,
                           &procmultiseq->bestmatchlist,
                           outinfo->sortmode,
                           &procmultiseq->processmatch,
                           showverbose) != 0)
      {
        return (Sint) -1;
      }
    }
  }
  if(chaincallinfo->defined)
  {
    if(vmatchchaining(mfargs,
                      outinfo,
                      procmultiseq->matchbuffer.spaceStoreMatch,
                      procmultiseq->matchbuffer.nextfreeStoreMatch,
                      outinfo->outvms->numofsequences,
                      chaincallinfo,
                      &procmultiseq->processmatch) != 0)
    {
      return (Sint) -2;
    }
  } else
  {
    if(matchclustercallinfo->defined)
    {
      if(genericmatchclustering(matchclustercallinfo,
                                outinfo->outvms,
                                outinfo->outvmsalpha,
                                outinfo->outqms,
                                &procmultiseq->matchbuffer,
                                mfargs) != 0)
      {
        return (Sint) -3;
      }
    }
  }
  if(outinfo->selectbundle != NULL &&
     outinfo->selectbundle->selectmatchFinaltable != NULL)
  {
    ArrayStoreMatch *finaltable
      = outinfo->selectbundle->selectmatchFinaltable(alpha,
                                                     outinfo->outvms,
                                                     outinfo->outqms);
    if(finaltable == NULL)
    {
      ERROR0("selectmatchFinaltable delivers NULL-ptr");
      return (Sint) -3;
    }
    if(iterapplymatchfun(&procmultiseq->processmatch,
                         outinfo->outvms,
                         outinfo->outqms,
                         finaltable) != 0)
    {
      return (Sint) -4;
    }
  }
  if(clusterparms->dbclusterdefined)
  {
    if(processvmcluster(seqclinfo,showverbose) != 0)
    {
      return (Sint) -5;
    }
    freevmcluster(seqclinfo);
  } else
  {
    if(nomatch->donomatch)
    {
      if(markernomatchout(selfmatch,
                          iscompletematch,
                          markinfo,
                          nomatch,
                          outinfo,
                          alpha,
                          showverbose) != 0)
      {
        return (Sint) -6;
      }
      FREESPACE(markinfo->markmatchtable);
    } else
    {
      if(maskmatch->domaskmatch)
      {
        if(markermaskmatchout(selfmatch,
                              iscompletematch,
                              markinfo,
                              maskmatch,
                              outinfo,
                              alpha) != 0)
        {
          return (Sint) -7;
        }
        FREESPACE(markinfo->markmatchtable);
      } else
      {
        if(ispreinfomatch)
        {
          showmatcount(outinfo->outfp,
                       matchparam->userdefinedleastlength,
                       &outinfo->matchcounttab);
          FREEARRAY(&outinfo->matchcounttab,Uint);
        }
      }
    }
  }
  if(processfunctionwasundefined)
  {
    procmultiseq->processmatch.processfunction = NULL;
  }
  return 0;
}
