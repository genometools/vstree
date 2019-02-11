
#include <limits.h>
#include <string.h>
#include "debugdef.h"
#include "errordef.h"
#include "virtualdef.h"
#include "absdef.h"
#include "spacedef.h"
#include "minmax.h"
#include "chardef.h"
#include "dpbitvec48.h"
#include "failures.h"
#include "fhandledef.h"
#include "scoredef.h"
#include "vplugin-interface.h"
#include "cpridx-data.h"

#ifdef WITHREGEXP
#include "libautomata.h"
#endif

#ifdef WITHAGREP
#include "agrepdef.h"
#include "agrepexport.h"
#endif

#include "pminfo.h"
#include "matchstate.h"

#include "readvirt.pr"
#include "alphabet.pr"
#include "multiseq.pr"
#include "filehandle.pr"

#include "initmstate.pr"
#include "exactcompl.pr"
#include "hamcompl.pr"
#include "edistcompl.pr"

#define FWRITEPATTERN\
        if(WRITETOFILEHANDLE(pattern,(Uint) sizeof(Uchar),plen,stdout) != 0)\
        {\
          fprintf(stderr,"%s\n",messagespace());\
          exit(EXIT_FAILURE);\
        }

typedef struct
{
  Matchstate matchstate;
  char *indexname;
  BOOL online;
  Vpluginbundle *cpridxpatsearchbundle;
  Cpridxpatsearchdata *cpridxpatsearchdata;
  Uint savedistvalue;
} CompleteMatchstate;

#ifdef WITHREGEXP
typedef struct
{
  Match match;
  Matchstate *matchstate;
} Automatchinfo;

static Sint automatamatch(const Matchreport *matchreport)
{
  Automatchinfo *automatchinfo = (Automatchinfo *) matchreport->user_data;
  automatchinfo->match.position1 = matchreport->matchend - 
                                   matchreport->matchlen + 
                                   1;
  automatchinfo->match.length1 = automatchinfo->match.length2 
                               = matchreport->matchlen;
  if(automatchinfo->matchstate->processfinal(automatchinfo->matchstate,
                                             &automatchinfo->match) != 0)
  {
    return (Sint) -2;
  }
  return 0;
}

static BOOL isSequencepattern(Uchar *pattern,Uint plen)
{
  Uint i;
  Uchar current;

  for(i=0; i<plen; i++)
  {
    current = pattern[i];
    if(current == '[' ||
       current == ']' ||
       current == '#' ||
       current == '.' ||
       current == '^' ||
       current == '+' || /* Change this XXX */
       current == '>' || /* Change this XXX */
       current == '<' || /* Change this XXX */
       current == '{' ||
       current == '}')
    {
      return False;
    }
  }
  return True;
}

#define FINDAUTOMATAMATCHES           findautomatamatchesonline
#define FINDAUTOMATAMATCHESFILETYPE   FILETYPE_FASTA

#include "findautomata.gen"

/*
#undef FINDAUTOMATAMATCHES
#undef FINDAUTOMATAMATCHESFILETYPE

#define FINDAUTOMATAMATCHES           findautomatamatchesoffline
#define FINDAUTOMATAMATCHESFILETYPE   FILETYPE_VTREE

#include "findautomata.gen"
*/

#endif

Sint findexactcompletematchescompressedindex(void *info,
                                             Uint seqnum2,
                                             /*@unused@*/ Uchar *pattern,
                                             Uint plen)
{
  CompleteMatchstate *completematchstate = (CompleteMatchstate *) info;

  completematchstate->cpridxpatsearchdata->pattern = pattern;
  completematchstate->cpridxpatsearchdata->seqnum2 = seqnum2;
  completematchstate->cpridxpatsearchdata->plen = plen;
  if(completematchstate->cpridxpatsearchbundle->iface.vpluginsearch(
                     (void *) completematchstate->cpridxpatsearchdata) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

static Sint decidefcm(void *info,Uint seqnum2,Uchar *pattern,Uint plen)
{
  CompleteMatchstate *completematchstate = (CompleteMatchstate *) info;
  Sint (*fcm)(void *,Uint,Uchar *,Uint) = NULL;
  void *fcmdata;
#ifdef WITHREGEXP
  ArrayUchar *sregexp;
#endif

  fcmdata = &completematchstate->matchstate;
#ifdef WITHREGEXP
  sregexp = &completematchstate->matchstate.queryinfo->multiseq->searchregexp;
  if(sregexp->spaceUchar != NULL && sregexp->spaceUchar[seqnum2])
  {
    if(isSequencepattern(pattern,plen))
    {
      DEBUG0(1,"search pattern with standard method:\"");
      DEBUGCODE(1,FWRITEPATTERN);
      DEBUG0(1,"\"\n");
      if(transformstring(&completematchstate->matchstate.virtualtree->alpha,
                         pattern,plen) != 0)
      {
        return (Sint) -1;
      }
    } else
    {
      DEBUG0(1,"search pattern with non-standard method: \"");
      DEBUGCODE(1,FWRITEPATTERN);
      DEBUG0(1,"\"\n");
/// XXXX
      if(MPARMEXACTMATCH(&completematchstate->matchstate.matchparam.maxdist))
      {
        if(completematchstate->online)
        {
          fcm = findautomatamatchesonline;
        } else
        {
          if(!(completematchstate->matchstate.virtualtree->mapped & SKPTAB) &&
             !(completematchstate->matchstate.virtualtree->constructed & SKPTAB))
          {
            if(mapskptab(completematchstate->matchstate.virtualtree,
                         completematchstate->indexname) != 0)
            {
              return (Sint) -2;
            }
          }
          ERROR0("cannot search for regular expressions "
                 "on enhanced suffix array");
          return (Sint) -3;
          //fcm = findautomatamatchesoffline;
        }
      } else
      {
        if(completematchstate->online)
        {
          NOTIMPLEMENTED; // regular expression with errors online
/*
          fcm = findagrepmatchesonline;
*/
        } else
        {
          ERROR0("use option -online to find regular expression with errors");
          NOTIMPLEMENTED; // regular expression with errors offline
          /*@notreached@*/ return (Sint) -2;
        }
      }
    }
  }
#endif
  if(fcm == NULL)
  {
    if(completematchstate->online)
    {
      if(MPARMEXACTMATCH(&completematchstate->matchstate.matchparam.maxdist))
      {
        fcm = findexactcompletematchesonline;
      } else
      {
        if(MPARMEDISTMATCH(&completematchstate->matchstate.matchparam.maxdist))
        {
          fcm = findedistcompletematchesonline;
        } else
        {
          fcm = findhammingcompletematchesonline;
          fcmdata = completematchstate;
        }
      }
    } else
    {
      if(MPARMEXACTMATCH(&completematchstate->matchstate.matchparam.maxdist))
      {
        if(completematchstate->cpridxpatsearchbundle->handle == NULL)
        {
          fcm = findexactcompletematchesindex;
        } else
        {
          fcm = findexactcompletematchescompressedindex; 
          fcmdata = completematchstate;
        }
      } else
      {
        if(MPARMEDISTMATCH(&completematchstate->matchstate.matchparam.maxdist))
        {
          fcm = findedistcompletematchesindex;
        } else
        {
          fcm = findhammingcompletematchesindex;
        }
      }
    }
  }
  completematchstate->matchstate.matchparam.maxdist.distvalue 
    = completematchstate->savedistvalue;
  if(fcm(fcmdata,
         seqnum2,
         pattern,
         plen) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

Sint findcompletematches(Virtualtree *virtualtree,
                         char *indexormatchfile,
                         Queryinfo *queryinfo,
                         BOOL rcmode,
                         BOOL online,
                         Matchparam *matchparam,
                         Bestflag bestflag,
                         Uint shownoevalue,
                         Uint showselfpalindromic,
                         SelectBundle *selectbundle,
                         void *procmultiseq,
                         Currentdirection currentdirection,
                         Processfinalfunction processfinal,
                         Vpluginbundle *cpridxpatsearchbundle,
                         Cpridxpatsearchdata *cpridxpatsearchdata,
                         Evalues *evalues,
                         BOOL domatchbuffering)
{
  CompleteMatchstate completematchstate;

  completematchstate.online = online;
  completematchstate.indexname = indexormatchfile;
  completematchstate.cpridxpatsearchbundle = cpridxpatsearchbundle;
  completematchstate.cpridxpatsearchdata = cpridxpatsearchdata;
  if(initMatchstate(&completematchstate.matchstate,
                    virtualtree,
                    (void *) queryinfo,
                    matchparam,
                    bestflag,
                    shownoevalue,
                    showselfpalindromic,
                    selectbundle,
                    0, // onlinequerynumoffset
                    procmultiseq,
                    currentdirection,
                    False,
                    processfinal,
                    evalues,
                    domatchbuffering) != 0)
  {
    return (Sint) -1;
  }
  if(cpridxpatsearchbundle->handle == NULL)
  {
    cpridxpatsearchdata->voidMatchstate = NULL;
  } else
  {
    cpridxpatsearchdata->voidMatchstate = &completematchstate.matchstate;
  }
  completematchstate.savedistvalue = matchparam->maxdist.distvalue;
  if(overallsequences(rcmode,
                      queryinfo->multiseq,
                      (void *) &completematchstate,
                      decidefcm) != 0)
  {
    return (Sint) -2;
  }
  return 0;
}
