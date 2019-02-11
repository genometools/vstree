
#include "types.h"
#include "debugdef.h"
#include "matchtask.h"
#include "procmultiseq.h"
#include "outinfo.h"

#include "readmulti.pr"
#include "checkonoff.pr"

#include "vmengineexport.h"
#include "nomatch.pr"
#include "mapdemand.pr"
#include "procfinal.pr"

 Sint vmatmaxcountgeneric(Virtualtree *virtualtree,
                          Uint numberprocessors,
                          Uint searchlength,
                          ArrayUint *counttab);

static Sint runselfmatchesdirect(Matchcallinfo *matchcallinfo,
                                 Virtualtree *virtualtree,
                                 Procmultiseq *procmultiseq,
                                 Evalues *evalues)
{
  if(matchcallinfo->matchtask & TASKTANDEM)
  {
    SHOWVERBOSE(matchcallinfo,"find right branching tandem repeats");
    if(findtandems(virtualtree,
                   &matchcallinfo->matchparam,
                   matchcallinfo->outinfo.bestinfo.bestflag,
                   (matchcallinfo->outinfo.showmode & SHOWNOEVALUE)
                     ? True : False,
                   (matchcallinfo->outinfo.showmode
                     & SHOWSELFPALINDROMIC) ? True : False,
                   matchcallinfo->outinfo.selectbundle,
                   procmultiseq,
                   processfinal,
                   evalues,
                   DOMATCHBUFFERING(matchcallinfo)) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    Sint mode;

    if(matchcallinfo->matchtask & TASKSUPER)
    {
      SHOWVERBOSE(matchcallinfo,
                  "find direct substring matches (supermaximal repeats)");
      mode = 0; // call findsupermax
    } else
    {
      if(matchcallinfo->matchtask & TASKMUM)
      {
        if(matchcallinfo->matchtask & TASKMUMCANDIDATE)
        {
          ERROR0("option -mum cand also requires option -q");
          return (Sint) -2;
        } else
        {
          SHOWVERBOSE(matchcallinfo,
                     "find direct substring matches (minimal unique matches)");
        }
        mode = (Sint) 1; // call findmaximaluniquematches
      } else
      {
        SHOWVERBOSE(matchcallinfo,
                    "find direct substring matches (repeats)");
        if(matchcallinfo->matchparam.repeatgapspec.lowergapdefined)
        {
          if(checkenvvaronoff("FASTBOUNDEDGAPS"))
          {
            mode = (Sint) 2; // call vmatmaxoutwithgaps
          } else
          {
            mode = (Sint) 3; // call vmatmaxoutgeneric
          }
        } else
        {
          mode = (Sint) 3; // call vmatmaxoutgeneric
        }
      }
    }
    if(decidesimpleselfsearch(matchcallinfo->matchtask,
                              matchcallinfo->clusterparms.dbclusterdefined,
                              matchcallinfo->matchparam.repeatgapspec.
                                             lowergapdefined,
                              matchcallinfo->matchclustercallinfo.defined,
                              &matchcallinfo->matchparam.maxdist,
                              matchcallinfo->matchparam.xdropbelowscore,
                              &matchcallinfo->maskmatch,
                              &matchcallinfo->nomatch,
                              matchcallinfo->outinfo.showmode))
    {
      if(vmatmaxcountgeneric(virtualtree,
                             matchcallinfo->matchparam.numberofprocessors,
                             matchcallinfo->matchparam.userdefinedleastlength,
                             &matchcallinfo->outinfo.matchcounttab) != 0)
      {
        return (Sint) -2;
      }
    } else
    {
      if(findselfmatches(mode,
                         virtualtree,
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
                         evalues,
                         DOMATCHBUFFERING(matchcallinfo)) != 0)
      {
        return (Sint) -3;
      }
    }
  }
  return 0;
}

static Sint runselfmatchespalindromic(Matchcallinfo *matchcallinfo,
                                      Virtualtree *virtualtree,
                                      Procmultiseq *procmultiseq,
                                      Evalues *evalues)
{
  Queryinfo queryinfo;

  if(HASINDEXEDQUERIES(&virtualtree->multiseq))
  {
    ERROR0("option -p for self comparison does not allow "
           "queryfiles in the index");
    return (Sint) -1;
  }
  if(virtualtree->multiseq.rcsequence == NULL)
  {
    virtualtree->multiseq.rcsequence = copymultiseqRC(&virtualtree->multiseq);
    if(virtualtree->multiseq.rcsequence == NULL)
    {
      return (Sint) -2;
    }
  }
  matchcallinfo->outinfo.showmode |= SHOWSELFPALINDROMIC;
  queryinfo.multiseq = &virtualtree->multiseq;
  queryinfo.extended = False;
  queryinfo.querysubstringoffset = 0;
  SHOWVERBOSE(matchcallinfo,
              "find palindromic substring matches");
  if(findquerymatches(virtualtree,
                      0,
                      &queryinfo,
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
                      False,  // revmposorder
                      processfinal,
                      evalues,
                      DOMATCHBUFFERING(matchcallinfo)) != 0)
  {
    return (Sint) -3;
  }
  return 0;
}

Sint runselfmatches(Matchcallinfo *matchcallinfo,
                    Virtualtree *virtualtree,
                    Procmultiseq *procmultiseq,
                    Evalues *evalues)
{
  if(matchcallinfo->outinfo.showmode & SHOWDIRECT)
  {
    if(runselfmatchesdirect(matchcallinfo,
                            virtualtree,
                            procmultiseq,
                            evalues) != 0)
    {
      return (Sint) -1;
    }
  }
  if(matchcallinfo->outinfo.showmode & SHOWPALINDROMIC)
  {
    if(runselfmatchespalindromic(matchcallinfo,
                                 virtualtree,
                                 procmultiseq,
                                 evalues) != 0)
    {
      return (Sint) -2;
    }
  }
  return 0;
}
