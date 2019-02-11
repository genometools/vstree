
#include <string.h>
#include <ctype.h>
#include "debugdef.h"
#include "errordef.h"
#include "matchtask.h"
#include "codondef.h"
#include "scoredef.h"
#include "procmultiseq.h"

#include "dstrdup.pr"
#include "detpfxlen.pr"
#include "readvirt.pr"
#include "sixframe.pr"
#include "bestmatch.pr"

#include "substpos.pr"
#include "assigndig.pr"
#include "queryindex.pr"
#include "verifysub.pr"
#include "mapdemand.pr"
#include "runself.pr"
#include "runquery.pr"
#include "initpost.pr"
#include "alphabet.pr"
#include "codon.pr"
#include "evalues.pr"
#include "matchlenparm.pr"
#include "explain.pr"
#include "xmlfunc.pr"
#include "vmotif-start.pr"
#include "cpridx-start.pr"

static Sint constructvirtualforthisquery(Matchcallinfo *matchcallinfo,
                                         Virtualtree *subqueryvirtualtree,
                                         Uint seqnum,
                                         ArrayPairUint *substringpos)
{
  Uint demand = LCPTAB | TISTAB | SUFTAB;
  Sint queryspeedupdemand;

  DEBUG0(2,"demand = LCPTAB | TISTAB | SUFTAB\n");
  queryspeedupdemand 
    = addqueryspeedupdemand(matchcallinfo->matchparam.queryspeedup);
  if(queryspeedupdemand == (Sint) -1)
  {
    return (Sint) -1;
  }
  demand |= ((Uint) queryspeedupdemand);
  if(demand & ISOTAB)
  {
    demand |= STITAB;
  }
  if(matchcallinfo->matchparam.transnum != NOTRANSLATIONSCHEME)
  {
    demand |= OISTAB;
  }
  if(matchcallinfo->showverbose != NULL && substringpos != NULL)
  {
    char sbuf[80+1];
    sprintf(sbuf,"match query number %lu against database substring "
                 "in the range [%lu,%lu]",
            (Showuint) seqnum,
            (Showuint) substringpos->spacePairUint[seqnum].uint0,
            (Showuint) substringpos->spacePairUint[seqnum].uint1);
    matchcallinfo->showverbose(sbuf);
  }
  subqueryvirtualtree->prefixlength 
    = vm_recommendedprefixlength(
         subqueryvirtualtree->alpha.mapsize-1,
         subqueryvirtualtree->multiseq.totallength,
         SIZEOFBCKENTRY);
  subqueryvirtualtree->llvcachemin = subqueryvirtualtree->llvcachemax = NULL;
  if(completevirtualtree(subqueryvirtualtree,
                         demand,
                         matchcallinfo->showverbose) != 0)
  {
    return (Sint) -2;
  }
  if(matchcallinfo->showverbose != NULL)
  {
    if(showvirtualtreestatus(subqueryvirtualtree,"(null)",
                             matchcallinfo->showverbose) != 0)
    {
      return (Sint) -3;
    }
  }
  if(matchcallinfo->fqfsubstringinfo.ffdefined)
  {
    ERROR0("substring of query already specified");
    return (Sint) -4;
  }
  if(substringpos != NULL)
  {
    FQFSETVALUES(&matchcallinfo->fqfsubstringinfo,
                 substringpos->spacePairUint[seqnum].uint0,
                 substringpos->spacePairUint[seqnum].uint1);
  }
  return 0;
}

static Sint applytoeachquery(Virtualtree *subqueryvirtualtree,
                             Uint seqnum,
                             Matchcallinfo *matchcallinfo,
                             ArrayPairUint *substringpos,
                             Queryinfo *virtualqueryinfo,
                             Procmultiseq *procmultiseq,
                             Evalues *evalues)
{
  if(constructvirtualforthisquery(matchcallinfo,
                                  subqueryvirtualtree,
                                  seqnum,
                                  substringpos) != 0)
  {
    return (Sint) -1;
  }
  if(runquerymatches(matchcallinfo,
                     subqueryvirtualtree,
                     seqnum,  // onlinequerynumoffset
                     virtualqueryinfo,
                     procmultiseq,
                     True,
                     evalues) != 0)
  {
    return (Sint) -2;
  }
  matchcallinfo->fqfsubstringinfo.ffdefined = False;
  if(freevirtualtreemain(subqueryvirtualtree) != 0)
  {
    return (Sint) -3;
  }
  return 0;
}

static Sint checkwetherletterscanbemapped(Alphabet *alpha)
{
  Uchar cc;
  Uint i;

  for(i=0; i < alpha->mapsize; i++)
  {
    cc = alpha->characters[i];
    if(!isupper(cc) && !islower(cc) && cc != '*')
    {
      ERROR1("cannot map character %c of the alphabet to lower/upper case: "
             "argument tolower/topupper of option -dbmaskmatch or -qmaskmatch "
             "requires that all alphabet characters are letters",cc);
      return (Sint) -1;
    }
  }
  return 0;
}

static Sint mapvmatchvirtualtree(Matchcallinfo *matchcallinfo,
                                 Virtualtree *virtualtree)
{
  Sint retcode;
  Uint demand;

  retcode = getdemand(matchcallinfo->matchtask,
                      matchcallinfo->showverbose == NULL ? False : True,
                      matchcallinfo->matchparam.repeatgapspec.
                                                lowergapdefined,
                      matchcallinfo->matchclustercallinfo.defined,
                      matchcallinfo->outinfo.showmode,
                      &matchcallinfo->outinfo.showdesc,
                      matchcallinfo->outinfo.showstring,
                      matchcallinfo->numberofqueryfiles,
                      &matchcallinfo->matchparam.maxdist,
                      matchcallinfo->matchparam.xdropbelowscore,
                      matchcallinfo->clusterparms.dbclusterdefined,
                      matchcallinfo->clusterparms.dbclusterfilenameprefix,
                      matchcallinfo->clusterparms.nonredundantfile,
                      &matchcallinfo->maskmatch,
                      &matchcallinfo->nomatch,
                      matchcallinfo->matchparam.queryspeedup);
  if(retcode == (Sint) -1)
  {
    return (Sint) -2;
  }
  demand = (Uint) retcode;
  if(matchcallinfo->vmotifbundle.handle != NULL)
  {
    if(matchcallinfo->vmotifbundle.iface.vpluginadddemand(
                        (void *) &matchcallinfo->vmotifdata) != 0)
    {
      return (Sint) -1;
    }
    demand |= matchcallinfo->vmotifdata.includedemand;
    demand &= ~matchcallinfo->vmotifdata.excludedemand;
  }
  if(matchcallinfo->cpridxpatsearchbundle.handle != NULL)
  {
    if(matchcallinfo->cpridxpatsearchbundle.iface.vpluginadddemand(
                        (void *) &matchcallinfo->cpridxpatsearchdata) != 0)
    {
      return (Sint) -1;
    }
    demand |= matchcallinfo->cpridxpatsearchdata.includedemand;
    demand &= ~matchcallinfo->cpridxpatsearchdata.excludedemand;
  }
  if(mapvirtualtreeifyoucan(virtualtree,
                            matchcallinfo->indexormatchfile,
                            demand) != 0)
  {
    return (Sint) -3;
  }
  assignvirtualdigits(&matchcallinfo->outinfo.digits,&virtualtree->multiseq);
  if(!vm_isdnaalphabet(&virtualtree->alpha))
  {
    if(matchcallinfo->outinfo.showmode & SHOWPALINDROMIC)
    {
      ERROR0("option -p only possible for DNA alphabet: "
             "use option -dna for mkvtree");
      return (Sint) -4;
    }
    if(matchcallinfo->outinfo.showstring & SHOWALIGNABBREVIUB)
    {
      ERROR0("flag \"abbreviub\" for option -s only possible for DNA alphabet");
      return (Sint) -5;
    }
  }
  if(matchcallinfo->maskmatch.domaskmatch)
  {
    if(matchcallinfo->maskmatch.maskchar == MASKTOLOWER ||
       matchcallinfo->maskmatch.maskchar == MASKTOUPPER)
    {
      if(checkwetherletterscanbemapped(&virtualtree->alpha) != 0)
      {
        return (Sint) -6;
      }
    }
  }
  if(matchcallinfo->showverbose != NULL)
  {
    if(showvirtualtreestatus(virtualtree,
                             matchcallinfo->indexormatchfile,
                             matchcallinfo->showverbose) != 0)
    {
      return (Sint) -7;
    }
  }
  return 0;
}

static Sint procquerymatchesonline(Matchcallinfo *matchcallinfo,
                                   Virtualtree *virtualtree,
                                   Virtualtree *queryvirtualtree,
                                   Procmultiseq *procmultiseq,
                                   Evalues *evalues)
{
  Queryinfo virtualqueryinfo;
  ArrayPairUint substringpos;

  if((matchcallinfo->matchtask & TASKMUM) &&
    !(matchcallinfo->matchtask & TASKMUMCANDIDATE) &&
     queryvirtualtree->multiseq.numofsequences > UintConst(1))
  {
    ERROR0("options -mum, -q, and -online can only be combined "
           "if there is exactly one sequence in the query file");
    return (Sint) -1;
  }
  INITARRAY(&substringpos,PairUint);
  if(parsesubstringpos(&queryvirtualtree->multiseq,
                       &substringpos,
                       &virtualtree->multiseq) < 0)
  {
    return (Sint) -2; 
  }
  queryvirtualtree->multiseq.numofqueryfiles = 0;
  queryvirtualtree->multiseq.numofquerysequences = 0;
  queryvirtualtree->multiseq.totalquerylength = 0;
  queryvirtualtree->specialsymbols = virtualtree->specialsymbols;
  copyAlphabet(&queryvirtualtree->alpha,&virtualtree->alpha);
  virtualqueryinfo.multiseq = &virtualtree->multiseq;
  virtualqueryinfo.extended = False;
  virtualqueryinfo.querysubstringoffset = 0;
  if(runforeachquerysubstpos(queryvirtualtree,
                             matchcallinfo,
                             (substringpos.spacePairUint == NULL) 
                                ? NULL
                                : &substringpos,
                             &virtualqueryinfo,
                             procmultiseq,
                             evalues,
                             applytoeachquery) != 0)
  {
    return (Sint) -3;
  }
  FREEARRAY(&substringpos,PairUint);
  return 0;
}

static void initQueryinfooffline(Queryinfo *queryinfo,
                                 Virtualtree *queryvirtualtree,
                                 BOOL didreadqueryfromindex)
{
  queryinfo->multiseq = &queryvirtualtree->multiseq;
  if(didreadqueryfromindex)
  {
    queryinfo->extended = True;
    queryinfo->numofcodes = queryvirtualtree->numofcodes;
    queryinfo->prefixlength = queryvirtualtree->prefixlength;
    queryinfo->suftab = queryvirtualtree->suftab;
    queryinfo->lcptab = queryvirtualtree->lcptab;
    queryinfo->bcktab = queryvirtualtree->bcktab;
  } else
  {
    queryinfo->extended = False;
    queryinfo->numofcodes = 0;
    queryinfo->prefixlength = 0;
    queryinfo->suftab = NULL;
    queryinfo->lcptab = NULL;
    queryinfo->bcktab = NULL;
  }
  queryinfo->querysubstringoffset = 0;
}

Sint arrangevmatchinput(Matchcallinfo *matchcallinfo,
                        Procmultiseq *procmultiseq,
                        Virtualtree *virtualtree,
                        Virtualtree *queryvirtualtree,
                        Virtualtree *sixframeofqueryvirtualtree,
                        Virtualtree *dnavirtualtree,
                        Virtualtree **queryvirtualtreeformatching,
                        BOOL *didreadqueryfromindex)
{
  BOOL storedesc,
       sixframequerymatch,
       maporig;

  if(mapvmatchvirtualtree(matchcallinfo,virtualtree) != 0)
  {
    return (Sint) -1;
  }
  if(matchcallinfo->vmotifbundle.handle != NULL)
  {
    vmotifstart(&matchcallinfo->vmotifdata, matchcallinfo, virtualtree);
  }
  if(matchcallinfo->cpridxpatsearchbundle.handle != NULL)
  {
    if(cpridxpatsearchstart(&matchcallinfo->cpridxpatsearchdata,
                            matchcallinfo,
                            virtualtree) != 0)
    {
      return (Sint) -2;
    }
  }
  if(matchcallinfo->outinfo.showdesc.defined)
  {
    storedesc = True;
  } else
  {
    if(matchcallinfo->matchtask & TASKCOMPLETEMATCH)
    {
      storedesc = False;
    } else
    {
      storedesc = ISTASKONLINE;
    }
  }
  if(matchcallinfo->matchparam.transnum == NOTRANSLATIONSCHEME)
  {
    sixframequerymatch = False;
  } else
  {
    sixframequerymatch = True;
  }
  maporig = decide2maporiginalsequence(&matchcallinfo->maskmatch,
                                       matchcallinfo->
                                         clusterparms.
                                         dbclusterfilenameprefix,
                                       matchcallinfo->
                                         clusterparms.nonredundantfile,
                                       matchcallinfo->outinfo.showstring);
  procmultiseq->supermatchtask = Normalmatch;
  if(initoutinfostruct(&matchcallinfo->outinfo,
                       procmultiseq,
                       matchcallinfo->numberofqueryfiles,
                       &matchcallinfo->indexormatchfile[0],
                       storedesc,
                       sixframequerymatch,
                       &matchcallinfo->vmotifbundle,
                       &matchcallinfo->vmotifdata,
                       matchcallinfo->matchparam.dnasymbolmapdnavsprot,
                       (const char * const*) &matchcallinfo->queryfiles[0],
                       maporig,
                       virtualtree,
                       dnavirtualtree,
                       queryvirtualtree,
                       didreadqueryfromindex,
                       matchcallinfo->showverbose) != 0)
  {
    return (Sint) -3;
  }
  if(matchcallinfo->numberofqueryfiles == 0)
  {
    *queryvirtualtreeformatching = NULL;
    if(virtualtree->sixframeindex != NOTRANSLATIONSCHEME)
    {
      if(matchcallinfo->matchparam.userdefinedleastlength < (Uint) CODONLENGTH)
      {
        ERROR1("for six frame index parameter to option -l must be at least "
               "%lu since it refers to DNA sequence",(Showuint) CODONLENGTH);
        return (Sint) -4;
      }
      matchcallinfo->matchparam.userdefinedleastlength /= CODONLENGTH;
      matchcallinfo->matchparam.transnum = virtualtree->sixframeindex;
      if(checkshowstringmultiple(matchcallinfo->outinfo.showstring) != 0)
      {
        return (Sint) -5;
      }
    }
    procmultiseq->constq6frms = NULL;
  } else
  {
    if(matchcallinfo->matchparam.transnum == NOTRANSLATIONSCHEME)
    {
      *queryvirtualtreeformatching = queryvirtualtree;
      procmultiseq->constq6frms = NULL;
    } else
    {
#ifdef DEBUG
      if(virtualtree->multiseq.originalsequence == NULL)
      {
        virtualtree->multiseq.originalsequence
          = getoriginal(&matchcallinfo->indexormatchfile[0]);
      }
      if(virtualtree->multiseq.originalsequence == NULL)
      {
        return (Sint) -5;
      }
      virtualtree->mapped |= OISTAB;
#endif
      if(matchcallinfo->showverbose != NULL)
      {
        char sbuf[PATH_MAX+9+1];
        sprintf(sbuf,"%s.ois read",&matchcallinfo->indexormatchfile[0]);
        matchcallinfo->showverbose(sbuf);
        sprintf(sbuf,"generating six frame translation "
                     "of DNA query sequence of length %lu; use symbolmap "
                     "\"%s\" and translation scheme \"%s\"",
                     (Showuint) queryvirtualtree->multiseq.totallength,
                     matchcallinfo->matchparam.dnasymbolmapdnavsprot == NULL 
                       ? "dna" :  
                       matchcallinfo->matchparam.dnasymbolmapdnavsprot,
                     transnum2name(matchcallinfo->matchparam.transnum));
        matchcallinfo->showverbose(sbuf);
      }
      if(multisixframetranslateDNA(matchcallinfo->matchparam.transnum,
                                   False,
                                   &sixframeofqueryvirtualtree->multiseq,
                                   &queryvirtualtree->multiseq,
                                   virtualtree->alpha.symbolmap) != 0)
      {
        return (Sint) -6;
      }
      procmultiseq->supermatchtask = Querytranslatedmatch;
      *queryvirtualtreeformatching = sixframeofqueryvirtualtree;
      procmultiseq->constq6frms = &sixframeofqueryvirtualtree->multiseq;
    }
    if(verifysubstringinfo(&matchcallinfo->fqfsubstringinfo,
                           matchcallinfo->outinfo.outqms) != 0)
    {
      return (Sint) -7;
    }
  }
  return 0;
}

static Sint vmatchexplainoutput(Matchcallinfo *matchcallinfo,
                                BOOL sixframeindex)
{
  if(matchcallinfo->outinfo.outfp == stdout &&
     matchcallinfo->showverbose != NULL && 
     !matchcallinfo->clusterparms.dbclusterdefined && 
     !matchcallinfo->nomatch.donomatch &&
     !matchcallinfo->maskmatch.domaskmatch &&
     !(matchcallinfo->matchtask & TASKPREINFO))
  {
    BOOL sixframematch;

    if(matchcallinfo->matchparam.transnum != NOTRANSLATIONSCHEME ||
       sixframeindex)
    {
      sixframematch = True;
    } else
    {
      sixframematch = False;
    }
    if(explainmatchinfofp(False,
                          matchcallinfo->outinfo.outfp,
                          matchcallinfo->numberofqueryfiles,
                          matchcallinfo->outinfo.showmode,
                          &matchcallinfo->outinfo.showdesc,
                          sixframematch) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

Sint procmatch(Matchcallinfo *matchcallinfo,
               Virtualtree *virtualtree,
               Virtualtree *queryvirtualtreeformatching,
               Procmultiseq *procmultiseq,
               BOOL didreadqueryfromindex, 
               const char *functionname,
               void *processinfo,
               Showmatchfuntype showmatchfun)
{
  BOOL selfmatch, 
       processfunctionwasundefined, // the processmatch function was
                                    // originally undefined 
       ownpostprocessing;
  Queryinfo queryinfo;
  Evalues evalues;

  if(vmatchexplainoutput(matchcallinfo,virtualtree->sixframeindex) != 0)
  {
    return (Sint) -2;
  }
  selfmatch = (matchcallinfo->numberofqueryfiles == 0) ? True : False;
  if(!(matchcallinfo->matchtask & TASKCOMPLETEMATCH))
  {
    if(determinematchlengthparam(matchcallinfo->numberofqueryfiles,
                                 &matchcallinfo->matchparam,
                                 matchcallinfo->showverbose) != 0)
    {
      return (Sint) -3;
    }
  }
  if(matchcallinfo->outinfo.bestinfo.bestflag == Fixednumberofbest)
  {
    initbml(&procmultiseq->bestmatchlist,
            matchcallinfo->outinfo.bestinfo.bestnumber);
  } else
  {
    if(DOMATCHBUFFERING(matchcallinfo))
    {
      INITARRAY(&procmultiseq->matchbuffer,StoreMatch);
    }
  }
  inithammingEvalues(&evalues,1.0/(Evaluetype) (virtualtree->alpha.mapsize-1));
  procmultiseq->processmatch.processfunction = showmatchfun;
  procmultiseq->processmatch.processinfo = processinfo;
  ASSIGNDYNAMICSTRDUP(procmultiseq->processmatch.functionname,functionname);
  CALLSELECT(matchcallinfo,
             matchcallinfo->outinfo.outvmsalpha,
             matchcallinfo->outinfo.outvms,
             selectmatchInit,
             matchcallinfo->outinfo.outqms);
  if(matchcallinfo->outinfo.showstring & SHOWVMATCHXML)
  {
    vmatchxmlinit(matchcallinfo->outinfo.outfp,
                  matchcallinfo->outinfo.outvmsalpha,
                  matchcallinfo->outinfo.outvms,
                  matchcallinfo->outinfo.outqms);
  }
  if(initdatastructuresvmatchgeneric(selfmatch,
                                     &processfunctionwasundefined,
                                     &ownpostprocessing,
                                     &matchcallinfo->clusterparms,
                                     &matchcallinfo->seqclinfo,
                                     &matchcallinfo->outinfo,
                                     (matchcallinfo->matchtask & 
                                         TASKCOMPLETEMATCH) ? True: False,
                                     (matchcallinfo->matchtask & TASKPREINFO) 
                                       ? True: False,
                                     &matchcallinfo->markinfo,
                                     &matchcallinfo->nomatch,
                                     &matchcallinfo->maskmatch,
                                     matchcallinfo->mfargs,
                                     matchcallinfo->numberofqueryfiles,
                                     DOMATCHBUFFERING(matchcallinfo),
                                     procmultiseq) != 0)
  {
    return (Sint) -4;
  }
  if(selfmatch)
  {
    if(runselfmatches(matchcallinfo,virtualtree,procmultiseq,&evalues) != 0)
    {
      return (Sint) -5;
    }
  } else
  {
    if(matchcallinfo->matchtask & TASKCOMPLETEMATCH)
    {
      if(matchcallinfo->vmotifbundle.handle != NULL)
      {
        if(matchcallinfo->vmotifbundle.iface.vpluginsearch(
                              &matchcallinfo->vmotifdata) != 0)
        {
          return (Sint) -6;
        }
      } else
      {
        NOTSUPPOSEDTOBENULL(queryvirtualtreeformatching);
        queryinfo.multiseq = &queryvirtualtreeformatching->multiseq;
        queryinfo.extended = False;
        queryinfo.querysubstringoffset = 0;
        if(runcompletematches(matchcallinfo,
                              virtualtree,
                              &queryinfo,
                              procmultiseq,
                              &evalues) != 0)
        {
          return (Sint) -7;
        }
      }
    } else
    {
      if(matchcallinfo->matchtask & TASKONLINE)
      {
        if(procquerymatchesonline(matchcallinfo,
                                  virtualtree,
                                  queryvirtualtreeformatching,
                                  procmultiseq,
                                  &evalues) != 0)
        {
          return (Sint) -8;
        }
      } else
      {
        initQueryinfooffline(&queryinfo,
                             queryvirtualtreeformatching,
                             didreadqueryfromindex);
        if(runquerymatches(matchcallinfo,
                           virtualtree,
                           0,  // onlinequerynumoffset
                           &queryinfo,
                           procmultiseq,
                           False,
                           &evalues) != 0)
        {
          return (Sint) -9;
        }
      }
    }
  }
  if(ownpostprocessing)
  {
    if(postprocessvmatchgeneric(selfmatch,
                                processfunctionwasundefined,
                                &matchcallinfo->clusterparms,
                                &matchcallinfo->chaincallinfo,
                                &matchcallinfo->matchclustercallinfo,
                                &matchcallinfo->seqclinfo,
                                &matchcallinfo->outinfo,
                                matchcallinfo->mfargs,
                                DOMATCHBUFFERING(matchcallinfo),
                                (matchcallinfo->matchtask & TASKCOMPLETEMATCH) 
                                  ? True: False,
                                (matchcallinfo->matchtask & TASKPREINFO) 
                                  ? True: False,
                                &matchcallinfo->markinfo,
                                &matchcallinfo->nomatch,
                                &matchcallinfo->maskmatch,
                                &matchcallinfo->matchparam,
                                &virtualtree->alpha,
                                procmultiseq,
                                matchcallinfo->showverbose) != 0)
    {
      return (Sint) -10;
    }
  }
  ASSIGNPROCESSMATCH(&procmultiseq->processmatch,NULL,NULL);
  CALLSELECT(matchcallinfo,
             matchcallinfo->outinfo.outvmsalpha,
             matchcallinfo->outinfo.outvms,
             selectmatchWrap,
             matchcallinfo->outinfo.outqms);
  if(matchcallinfo->outinfo.showstring & SHOWVMATCHXML)
  {
    vmatchxmlwrap(matchcallinfo->outinfo.outfp);
  }
  freeEvalues(&evalues);
  if(matchcallinfo->outinfo.bestinfo.bestflag == Fixednumberofbest)
  {
    freebestmatchlist(&procmultiseq->bestmatchlist);
  } else
  {
    if(DOMATCHBUFFERING(matchcallinfo))
    {
      FREEARRAY(&procmultiseq->matchbuffer,StoreMatch);
    }
  }
  FREESPACE(procmultiseq->processmatch.functionname);
  if(matchcallinfo->vmotifbundle.handle != NULL)
  {
    if(matchcallinfo->vmotifbundle.iface.vpluginwrap(
                                  (void *) &matchcallinfo->vmotifdata) != 0)
    {
      return (Sint) -11;
    }
  }
  if(matchcallinfo->cpridxpatsearchbundle.handle != NULL)
  {
    if(matchcallinfo->cpridxpatsearchbundle.iface.vpluginwrap(
                            (void *) &matchcallinfo->cpridxpatsearchdata) != 0)
    {
      return (Sint) -13;
    }
  }
  return 0;
}
