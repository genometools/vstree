
#include <string.h>
#include "spacedef.h"
#include "debugdef.h"
#include "errordef.h"
#include "minmax.h"
#include "fhandledef.h"

#include "matchtask.h"
#include "procmultiseq.h"

#include "readmulti.pr"

#include "vmengineexport.h"
#include "echomatch.pr"
#include "nomatch.pr"
#include "showmasked.pr"
#include "markmat.pr"
#include "procfinal.pr"
#include "vmotif-start.pr"

static Uint addsubstringoffset(Uint *savetotallength,
                               Queryinfo *queryinfo,
                               FQFsubstringinfo *fqfsubstringinfo,
                               BOOL rcmode)
{
  Uint seqoffset;

  if(fqfsubstringinfo != NULL && fqfsubstringinfo->ffdefined)
  {
    *savetotallength = queryinfo->multiseq->totallength;
    if(rcmode)
    {
      seqoffset
        = queryinfo->multiseq->totallength - 1 - fqfsubstringinfo->readseqend;
      queryinfo->multiseq->rcsequence += seqoffset;
    } else
    {
      seqoffset = fqfsubstringinfo->readseqstart;
      queryinfo->multiseq->sequence += seqoffset;
    }
    queryinfo->querysubstringoffset = fqfsubstringinfo->readseqstart;
    queryinfo->multiseq->totallength = fqfsubstringinfo->readseqend -
                                       fqfsubstringinfo->readseqstart + 1;
    return seqoffset;
  }
  queryinfo->querysubstringoffset = 0;
  return 0;
}

static void subtractsubstringoffset(Queryinfo *queryinfo,
                                    FQFsubstringinfo *fqfsubstringinfo,
                                    BOOL rcmode,
                                    Uint seqoffset,
                                    Uint savetotallength)
{
  if(fqfsubstringinfo != NULL && fqfsubstringinfo->ffdefined)
  {
    if(rcmode)
    {
      queryinfo->multiseq->rcsequence -= seqoffset;
    } else
    {
      queryinfo->multiseq->sequence -= seqoffset;
    }
    queryinfo->multiseq->totallength = savetotallength;
    queryinfo->querysubstringoffset = 0;
  }
}

static Sint runquerymatchesdirect(BOOL complete,
                                  Matchcallinfo *matchcallinfo,
                                  Virtualtree *virtualtree,
                                  Uint onlinequerynumoffset,
                                  Queryinfo *queryinfo,
                                  Procmultiseq *procmultiseq,
                                  BOOL revmposorder,
                                  Evalues *evalues)
{
  if(complete)
  {
    SHOWVERBOSE(matchcallinfo,"find direct complete matches");
    if(matchcallinfo->vmotifbundle.handle != NULL)
    {
      if(vmotifinitMatchstate(&matchcallinfo->vmotifdata,
                              matchcallinfo,
                              virtualtree,
                              procmultiseq,
                              DirectionForward,
                              revmposorder,
                              evalues,
                              DOMATCHBUFFERING(matchcallinfo)) != 0)
      {
        return (Sint) -1;
      }
    }
    if(findcompletematches(virtualtree,
                           &matchcallinfo->indexormatchfile[0],
                           queryinfo,
                           False,
                           ISTASKONLINE,
                           &matchcallinfo->matchparam,
                           matchcallinfo->outinfo.bestinfo.bestflag,
                           (matchcallinfo->outinfo.showmode & SHOWNOEVALUE)
                              ? True : False,
                           (matchcallinfo->outinfo.showmode 
                                & SHOWSELFPALINDROMIC) ? True : False,
                           matchcallinfo->outinfo.selectbundle,
                           procmultiseq,
                           DirectionForward,
                           processfinal,
                           &matchcallinfo->cpridxpatsearchbundle,
                           &matchcallinfo->cpridxpatsearchdata,
                           evalues,
                           DOMATCHBUFFERING(matchcallinfo)) != 0)
    {
      return (Sint) -2;
    }
    if(matchcallinfo->vmotifbundle.handle != NULL)
    {
       if(vmotifwrap(&matchcallinfo->vmotifdata) != 0)
       {
         return (Sint) -3;
       }
    }
  } else
  {
    Uint seqoffset, savetotallength = 0;

    if(matchcallinfo->matchtask & TASKMUM)
    {
      if(matchcallinfo->matchtask & TASKMUMCANDIDATE)
      {
        SHOWVERBOSE(matchcallinfo,"find direct substring matches against query "
                                  "(maximal unique matches candidates)");
      } else
      {
        SHOWVERBOSE(matchcallinfo,"find direct substring matches against query "
                                  "(maximal unique matches)");
      }
    } else
    {
      SHOWVERBOSE(matchcallinfo,"find direct substring matches against query "
                                "(maximal exact matches)");
    }
    seqoffset = addsubstringoffset(&savetotallength,
                                   queryinfo, 
                                   &matchcallinfo->fqfsubstringinfo, False);
    if(findquerymatches(virtualtree,
                        onlinequerynumoffset,
                        queryinfo,
                        (matchcallinfo->matchtask & TASKMUM) 
                          ? True : False,
                        (matchcallinfo->matchtask & TASKMUMCANDIDATE) 
                          ? True : False,
                        False,//rcmode
                        &matchcallinfo->matchparam,
                        matchcallinfo->outinfo.bestinfo.bestflag,
                           (matchcallinfo->outinfo.showmode & SHOWNOEVALUE)
                              ? True : False,
                           (matchcallinfo->outinfo.showmode
                                & SHOWSELFPALINDROMIC) ? True : False,
                        matchcallinfo->outinfo.selectbundle,
                        procmultiseq,
                        DirectionForward,
                        revmposorder,
                        processfinal,
                        evalues,
                        DOMATCHBUFFERING(matchcallinfo)) != 0)
    {
      return (Sint) -4;
    }
    subtractsubstringoffset(queryinfo,&matchcallinfo->fqfsubstringinfo,False,
                            seqoffset,savetotallength);
  }
  return 0;
}

static Sint runquerymatchespalindromic(BOOL complete,
                                       Matchcallinfo *matchcallinfo,
                                       Virtualtree *virtualtree,
                                       Uint onlinequerynumoffset,
                                       Queryinfo *queryinfo,
                                       Procmultiseq *procmultiseq,
                                       BOOL revmposorder,
                                       Evalues *evalues)
{
  if(queryinfo->multiseq->rcsequence == NULL)
  {
    queryinfo->multiseq->rcsequence = copymultiseqRC(queryinfo->multiseq);
    if(queryinfo->multiseq->rcsequence == NULL)
    {
      return (Sint) -1;
    }
  }
  if(complete)
  {
    SHOWVERBOSE(matchcallinfo,"find palindromic complete matches");
    if(matchcallinfo->vmotifbundle.handle != NULL)
    {
      if(vmotifinitMatchstate(&matchcallinfo->vmotifdata,
                              matchcallinfo,
                              virtualtree,
                              procmultiseq,
                              DirectionReverse,
                              revmposorder,
                              evalues,
                              DOMATCHBUFFERING(matchcallinfo)) != 0)
      {
        return (Sint) -2;
      }
    }
    if(findcompletematches(virtualtree,
                           &matchcallinfo->indexormatchfile[0],
                           queryinfo,
                           True,
                           ISTASKONLINE,
                           &matchcallinfo->matchparam,
                           matchcallinfo->outinfo.bestinfo.bestflag,
                           (matchcallinfo->outinfo.showmode & SHOWNOEVALUE)
                             ? True : False,
                           (matchcallinfo->outinfo.showmode 
                             & SHOWSELFPALINDROMIC) ? True : False,
                           matchcallinfo->outinfo.selectbundle,
                           procmultiseq,
                           DirectionReverse,
                           processfinal,
                           &matchcallinfo->cpridxpatsearchbundle,
                           &matchcallinfo->cpridxpatsearchdata,
                           evalues,
                           DOMATCHBUFFERING(matchcallinfo)) != 0)
    {
      return (Sint) -2;
    }
    if(matchcallinfo->vmotifbundle.handle != NULL)
    {
       if(vmotifwrap(&matchcallinfo->vmotifdata) != 0)
       {
         return (Sint) -3;
       }
    }
  } else
  {
    Uint seqoffset, savetotallength = 0;

    SHOWVERBOSE(matchcallinfo,
                "find palindromic substring matches against query");
    seqoffset = addsubstringoffset(&savetotallength,
                                   queryinfo, 
                                   &matchcallinfo->fqfsubstringinfo, True);
    if(findquerymatches(virtualtree,
                        onlinequerynumoffset,
                        queryinfo,
                        (matchcallinfo->matchtask & TASKMUM) 
                          ? True : False,
                        (matchcallinfo->matchtask & TASKMUMCANDIDATE)
                          ? True : False,
                        True,//rcmode
                        &matchcallinfo->matchparam,
                        matchcallinfo->outinfo.bestinfo.bestflag,
                           (matchcallinfo->outinfo.showmode & SHOWNOEVALUE)
                              ? True : False,
                           (matchcallinfo->outinfo.showmode
                                & SHOWSELFPALINDROMIC) ? True : False,
                        matchcallinfo->outinfo.selectbundle,
                        procmultiseq,
                        DirectionReverse,
                        revmposorder,
                        processfinal,
                        evalues,
                        DOMATCHBUFFERING(matchcallinfo)) != 0)
    {
      return (Sint) -3;
    }
    subtractsubstringoffset(queryinfo,&matchcallinfo->fqfsubstringinfo,True,
                            seqoffset,savetotallength);
  }
  return 0;
}

static Sint runquerymatchesgeneric(BOOL complete,
                                   Virtualtree *virtualtree,
                                   Uint onlinequerynumoffset,
                                   Matchcallinfo *matchcallinfo,
                                   Queryinfo *queryinfo,
                                   Procmultiseq *procmultiseq,
                                   BOOL revmposorder,
                                   Evalues *evalues)
{
  if(matchcallinfo->outinfo.showmode & SHOWDIRECT)
  {
    if(runquerymatchesdirect(complete,
                             matchcallinfo,
                             virtualtree,
                             onlinequerynumoffset,
                             queryinfo,
                             procmultiseq,
                             revmposorder,
                             evalues) != 0)
    {
      return (Sint) -2;
    }
  }
  if(matchcallinfo->outinfo.showmode & SHOWPALINDROMIC)
  {
    if(runquerymatchespalindromic(complete,
                                  matchcallinfo,
                                  virtualtree,
                                  onlinequerynumoffset,
                                  queryinfo,
                                  procmultiseq,
                                  revmposorder,
                                  evalues) != 0)
    {
      return (Sint) -3;
    }
  }
  return 0;
}

Sint runquerymatches(Matchcallinfo *matchcallinfo,
                     Virtualtree *virtualtree,
                     Uint onlinequerynumoffset,
                     Queryinfo *queryinfo,
                     Procmultiseq *procmultiseq,
                     BOOL revmposorder,
                     Evalues *evalues)
{
  return runquerymatchesgeneric(False,
                                virtualtree,
                                onlinequerynumoffset,
                                matchcallinfo,
                                queryinfo,
                                procmultiseq,
                                revmposorder,
                                evalues);
}

Sint runcompletematches(Matchcallinfo *matchcallinfo,
                        Virtualtree *virtualtree,
                        Queryinfo *queryinfo,
                        Procmultiseq *procmultiseq,
                        Evalues *evalues)
{
  return runquerymatchesgeneric(True,
                                virtualtree,
                                0,
                                matchcallinfo,
                                queryinfo,
                                procmultiseq,
                                False, // revmposorder
                                evalues);
}
