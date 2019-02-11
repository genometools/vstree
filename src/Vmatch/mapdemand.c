
#include "debugdef.h"
#include "errordef.h"
#include "virtualdef.h"
#include "markinfo.h"
#include "matchtask.h"

Sint addqueryspeedupdemand(Uint queryspeedup)
{
  Uint demand;

  switch(queryspeedup)
  {
    case 0:
      demand = BCKTAB;
      DEBUG0(2,"demand = BCKTAB\n");
      break;
    case 1:
      demand = (BCKTAB | ISOTAB);
      DEBUG0(2,"demand |= BCKTAB | ISOTAB\n");
      break;
    case 2:
    case 3:
      demand = STI1TAB | BCKTAB;
      DEBUG0(2,"demand = STI1TAB | BCKTAB\n");
      break;
    case 4:
      demand = LSFTAB | BCKTAB;
      DEBUG0(2,"demand = LSFTAB | BCKTAB\n");
      break;
    case 5:
      demand = 0;
      break;
    default:
      ERROR1("illegal speedup value %lu",(Showuint) queryspeedup);
      return (Sint) -1;
  }
  return (Sint) demand;
}

BOOL decide2maporiginalsequence(Maskmatch *maskmatch,
                                char *dbclusterfilenameprefix,
                                char *nonredundantfile,
                                Uint showstring)
{
  if((maskmatch->domaskmatch && maskmatch->markfields.markdb)
     || dbclusterfilenameprefix != NULL 
     || nonredundantfile != NULL 
     || showstring > 0)
  {
    return True;
  }
  return False;
}

static BOOL postprocesswithtistab(Uint matchtask,
                                  Distancevalue *maxdist,
                                  Xdropscore xdropbelowscore,
                                  Maskmatch *maskmatch,
                                  Nomatch *nomatch,
                                  Uint showmode,
                                  BOOL matchclusterdefined)
{
  if(maskmatch->domaskmatch ||
     nomatch->donomatch ||
     xdropbelowscore != UNDEFXDROPBELOWSCORE ||
     !MPARMEXACTMATCH(maxdist) ||
     (showmode & SHOWPALINDROMIC) ||
     matchclusterdefined)
  {
    return True;
  }
  return False;
}

BOOL decidesimpleselfsearch(Uint matchtask,
                            BOOL dbclusterdefined,
                            BOOL lowergapdefined,
                            BOOL matchclusterdefined,
                            Distancevalue *maxdist,
                            Xdropscore xdropbelowscore,
                            Maskmatch *maskmatch,
                            Nomatch *nomatch,
                            Uint showmode)
{
  if((matchtask & TASKPREINFO) && 
     !(matchtask & TASKTANDEM) &&
     !(matchtask & TASKSUPER) &&
     !(matchtask & TASKMUM) &&
     !(matchtask & TASKMUMCANDIDATE) &&
     !lowergapdefined &&
     !postprocesswithtistab(matchtask,maxdist,xdropbelowscore,maskmatch,
                            nomatch,showmode,matchclusterdefined))
  {
    return True;
  }
  return False;
}

Sint getdemand(Uint matchtask,
               BOOL verbose,
               BOOL lowergapdefined, 
               BOOL matchclusterdefined,
               Uint showmode,
               Showdescinfo *showdesc,
               Uint showstring,
               Uint numberofqueryfiles,
               Distancevalue *maxdist,
               Xdropscore xdropbelowscore,
               BOOL dbclusterdefined,
               char *dbclusterfilenameprefix,
               char *nonredundantfile,
               Maskmatch *maskmatch,
               Nomatch *nomatch,
               Uint queryspeedup)
{
  Uint demand;

  if(numberofqueryfiles == 0)
  {
    if((matchtask & TASKSUPER) &&
       !verbose &&
       !postprocesswithtistab(matchtask,maxdist,xdropbelowscore,maskmatch,
                              nomatch,showmode,
                              matchclusterdefined))
    {
      DEBUG0(2,"demand = BWTTAB | LCPTAB | SUFTAB\n");
      demand = BWTTAB | LCPTAB | SUFTAB | SSPTAB;
    } else
    {
      if(decidesimpleselfsearch(matchtask,
                                dbclusterdefined,
                                lowergapdefined,
                                matchclusterdefined,
                                maxdist,
                                xdropbelowscore,
                                maskmatch,
                                nomatch,
                                showmode))
      {
        demand = BWTTAB | LCPTAB | SSPTAB;
        DEBUG0(2,"demand = BWTTAB | LCPTAB\n");
      } else
      {
        demand = LCPTAB | SUFTAB | TISTAB | SSPTAB;
        DEBUG0(2,"demand = LCPTAB | SUFTAB | TISTAB\n");
      }
    }
    if(showmode & SHOWDIRECT)
    {
      demand |= BWTTAB;
      DEBUG0(2,"demand |= BWTTAB\n");
    }
    if(showmode & SHOWPALINDROMIC)
    {
      Sint queryspeedupdemand = addqueryspeedupdemand(queryspeedup);
      if(queryspeedupdemand == (Sint) -1)
      {
        return (Sint) -1;
      }
      demand |= ((Uint) queryspeedupdemand);
    }
#ifdef DISTRIBUTEDDFS
    demand |= BCKTAB;
#endif
  } else
  {
    if(matchtask & TASKONLINE)
    {
      demand = TISTAB;
      DEBUG0(2,"demand = TISTAB\n");
    } else
    {
      demand = LCPTAB | SUFTAB | TISTAB;
      DEBUG0(2,"demand = LCPTAB | SUFTAB | TISTAB\n");
      if(matchtask & TASKCOMPLETEMATCH)
      {
        demand |= BCKTAB;
        if(!MPARMEXACTMATCH(maxdist))
        {
          demand |= SKPTAB;
          DEBUG0(2,"demand |= SKPTAB\n");
        }
      } else
      {
        Sint queryspeedupdemand = addqueryspeedupdemand(queryspeedup);
        if(queryspeedupdemand == (Sint) -1)
        {
          return (Sint) -1;
        }
        demand |= ((Uint) queryspeedupdemand);
      }
    }
  }
  if(showdesc->defined || nonredundantfile != NULL ||
     (dbclusterdefined && 
      dbclusterfilenameprefix != NULL && 
      showstring > 0))
  {
    demand |= DESTAB;
    DEBUG0(2,"demand |= DESTAB\n");
  }
  if(decide2maporiginalsequence(maskmatch,dbclusterfilenameprefix,
                                nonredundantfile,showstring))
  {
    demand |= OISTAB;
    DEBUG0(2,"demand |= OISTAB\n");
  }
  return (Sint) demand;
}
