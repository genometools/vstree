
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debugdef.h"
#include "errordef.h"
#include "spacedef.h"
#include "minmax.h"
#include "matchtask.h"
#include "matchinfo.h"
#include "vmcldef.h"
#include "select.h"
#include "intbits.h"
#include "codondef.h"
#include "fhandledef.h"
#include "optdesc.h"
#include "programversion.h"

#include "smcontain.pr"
#include "matsort.pr"
#include "readvirt.pr"
#include "filehandle.pr"

#include "detmatch.pr"
#include "parsevm.pr"
#include "vmcluster.pr"
#include "procargs.pr"
#include "assigndig.pr"
#include "mokay.pr"
#include "echomatch.pr"
#include "explain.pr"

static void showonstdout(char *s)
{
  printf("# %s\n",s);
}

static Sint vmsprocessmatches(Matchinfo *matchinfo,
                              Matchcallinfo *matchcallinfo,
                              Uint howmany)
{
  StoreMatch *smptr;
  Multiseq *outvms = matchinfo->outinfo.outvms,
           *outqms = matchinfo->outinfo.outqms;
  Alphabet *outvmsalpha = matchinfo->outinfo.outvmsalpha;

  for(smptr = matchinfo->matchtab.spaceStoreMatch; 
      smptr < matchinfo->matchtab.spaceStoreMatch + howmany; smptr++)
  {
    if(matchcallinfo->outinfo.selectbundle == NULL ||
       matchcallinfo->outinfo.selectbundle->selectmatch == NULL ||
       matchcallinfo->outinfo.selectbundle->selectmatch(outvmsalpha,
                                                        outvms,
                                                        outqms,
                                                        smptr) == (Sint) 1)
    {
      if(echomatch2file(stdout,
                        False,
                        matchcallinfo->outinfo.showmode,
                        &matchcallinfo->outinfo.showdesc,
                        matchcallinfo->outinfo.showstring,
                        &matchcallinfo->outinfo.digits,
                        outvms,
                        outqms,
                        outvmsalpha,
                        smptr) != 0)
      {
        return (Sint) -1;
      }
    }
  }
  if(matchcallinfo->outinfo.selectbundle != NULL &&
     matchcallinfo->outinfo.selectbundle->selectmatchFinaltable != NULL)
  {
    ArrayStoreMatch *finaltable 
      = matchcallinfo->outinfo.selectbundle
                              ->selectmatchFinaltable(outvmsalpha,
                                                      outvms,outqms);
    if(finaltable == NULL)
    {
      ERROR0("selectmatchFinaltable delivers NULL-ptr");
      return (Sint) -2;
    }
    for(smptr = finaltable->spaceStoreMatch;
        smptr < finaltable->spaceStoreMatch + finaltable->nextfreeStoreMatch; 
        smptr++)
    {
      if(echomatch2file(stdout,
                        False,
                        matchcallinfo->outinfo.showmode,
                        &matchcallinfo->outinfo.showdesc,
                        matchcallinfo->outinfo.showstring,
                        &matchcallinfo->outinfo.digits,
                        outvms,
                        outqms,
                        outvmsalpha,
                        smptr) != 0)
      {
        return (Sint) -3;
      }
    }
  }
  return 0;
}

static Sint vmatchselectcluster(Multiseq *virtualmultiseq,
                                Clusterparms *clusterparms,
                                Outinfo *mcoutinfo,
                                char *args,
                                ArrayStoreMatch *matchtab,
                                Matchparam *mparms)
{
  StoreMatch *mptr;
  Sequenceclusterinfo clusterinfo;

  DEBUGCODE(1,showclusterparms(clusterparms,mparms->userdefinedleastlength,
                               mparms->identity,
                               (Uint) mparms->xdropleastscore));
  if(initvmcluster(&clusterinfo,
                   clusterparms,
                   mcoutinfo,
                   args,
                   virtualmultiseq) != 0)
  {
    return (Sint) -1;
  }
  INITBITTAB(clusterinfo.clusterset.subclusterelems,
             virtualmultiseq->numofsequences);
  for(mptr=matchtab->spaceStoreMatch; 
      mptr < matchtab->spaceStoreMatch + matchtab->nextfreeStoreMatch; mptr++)
  {
    if(clusterinfo.clusterset.subclusterelems != NULL)
    {
      if(!ISIBITSET(clusterinfo.clusterset.subclusterelems,mptr->Storeseqnum1))
      {
        clusterinfo.clusterset.subclusterelemcount++;
        SETIBIT(clusterinfo.clusterset.subclusterelems,mptr->Storeseqnum1);
      }
      if(!ISIBITSET(clusterinfo.clusterset.subclusterelems,mptr->Storeseqnum2))
      {
        clusterinfo.clusterset.subclusterelemcount++;
        SETIBIT(clusterinfo.clusterset.subclusterelems,mptr->Storeseqnum2);
      }
    }
    if(matchokay(mptr->Storelength1,
                 mptr->Storeposition1,
                 mptr->Storelength2,
                 mptr->Storeposition2,
                 mptr->Storedistance,
                 mptr->StoreEvalue,
                 mptr->Storeflag,
                 mparms))
    {
#ifdef DEBUG
      DEBUG1(1,"addvmcluster(match %lu)",
                (Showuint) (mptr - matchtab->spaceStoreMatch));
      if(mparms->userdefinedleastlength > 0)
      {
        DEBUG2(1," length=(%lu,%lu)",
                 (Showuint) mptr->Storelength1,
                 (Showuint) mptr->Storelength2);
      }
      if(mparms->xdropleastscore > 0)
      {
        DEBUG1(1," leastscore=%ld",(Showsint) DISTANCE2SCORE(mptr));
      }
      if(mparms->identity > 0)
      {
        double id;

        IDENTITY(id,mptr);
        DEBUG1(1," identity=%.2f",id);
      } 
      DEBUG0(1,"\n");
#endif
      if(addvmcluster((void *) &clusterinfo,
                      virtualmultiseq,
                      NULL,
                      mptr) != 0)
      {
        return (Sint) -2;
      }
    }
  }
  if(processvmcluster(&clusterinfo,showonstdout) != 0)
  {
    return (Sint) -3;
  }
  FREESPACE(clusterinfo.clusterset.subclusterelems);
  freevmcluster(&clusterinfo);
  return 0;
}

static Sint runvmatsel(Argctype argc,
                       const char **argv,
                       Matchinfo *matchinfo,
                       Matchcallinfo *matchcallinfo)
{
  Uint howmany;

  assignvirtualdigits(&matchcallinfo->outinfo.digits,
                      matchinfo->outinfo.outvms);
  if(matchcallinfo->clusterparms.dbclusterdefined)
  {
    if(vmatchselectcluster(matchinfo->outinfo.outvms,
                           &matchcallinfo->clusterparms,
                           &matchcallinfo->outinfo,
                           matchinfo->mfargs,
                           &matchinfo->matchtab,
                           &matchcallinfo->matchparam) != 0)
    {
      return (Sint) -2;
    }
  } else
  {
    Uint countremoved;
#ifdef DEBUG
    if(checkremoval(&matchinfo->matchtab) != 0)
    {
      return (Sint) -3;
    }
#endif
    countremoved = removecontained(&matchinfo->matchtab);
    if(matchcallinfo->showverbose != NULL)
    {
      char sbuf[80+1];
      sprintf(sbuf,"remove %lu contained matches",(Showuint) countremoved);
      matchcallinfo->showverbose(sbuf);
    }
    if(matchcallinfo->outinfo.sortmode != undefsortmode())
    {
      sortallmatches(matchinfo->matchtab.spaceStoreMatch,
                     matchinfo->matchtab.nextfreeStoreMatch,
                     matchcallinfo->outinfo.sortmode);
    }
    if(showargumentline(matchcallinfo->outinfo.selectbundle == NULL 
                        ? NULL
                        : matchcallinfo->outinfo.selectbundle
                                       ->selectmatchHeader,
                        stdout,
                        matchinfo->mfargs,
                        argc,
                        argv) != 0)
    {
      return (Sint) -4;
    }
    if(matchcallinfo->showverbose != NULL)
    {
      if(explainmatchinfofp(False,
                            stdout,
                            matchinfo->numberofqueryfiles,
                            matchcallinfo->outinfo.showmode,
                            &matchcallinfo->outinfo.showdesc,
                            (matchinfo->transnum != NOTRANSLATIONSCHEME)
                              ? True
                              : False) != 0)
      {
        return (Sint) -5;
      }
    }
    if(matchcallinfo->outinfo.bestinfo.bestflag == Fixednumberofbest)
    {
      howmany = MIN(matchinfo->matchtab.nextfreeStoreMatch,
                    matchcallinfo->outinfo.bestinfo.bestnumber);
    } else
    {
      if(matchcallinfo->outinfo.bestinfo.bestflag == Allbestmatches)
      {
        howmany = matchinfo->matchtab.nextfreeStoreMatch;
      } else
      {
        ERROR1("%s: bestflag = Allmaximalmatches",argv[0]);
        return (Sint) -6;
      }
    }
    CALLSELECT(matchcallinfo,
               matchinfo->outinfo.outvmsalpha,
               matchinfo->outinfo.outvms,
               selectmatchInit,
               matchinfo->outinfo.outqms);
    if(vmsprocessmatches(matchinfo,matchcallinfo,howmany) != 0)
    {
      return (Sint) -4;
    }
    CALLSELECT(matchcallinfo,
               matchinfo->outinfo.outvmsalpha,
               matchinfo->outinfo.outvms,
               selectmatchWrap,
               matchinfo->outinfo.outqms);
  }
  return 0;
}

MAINFUNCTION
{
  Sint pret;
  Matchinfo matchinfo;
  Matchcallinfo matchcallinfo;

  DEBUGLEVELSET;
  CALLSHOWPROGRAMVERSION("vmatchselect");
  pret = parsevmatchargs(False,
                         argc,
                         argv,
                         showonstdout,
                         NULL,
                         NULL,
                         &matchcallinfo);
  if(pret == (Sint) -2)
  {
    SIMPLESTANDARDMESSAGE;
  }
  if(pret < 0)
  {
    STANDARDMESSAGE;
  }
  if(pret == (Sint) 1)
  {
    return EXIT_SUCCESS;
  }
  matchcallinfo.outinfo.outfp = stdout;
  if(determineMatchinfo (NULL,
                         &matchinfo,
                         argc,
                         argv,
                         &matchcallinfo.indexormatchfile[0],
                         (matchcallinfo.clusterparms.dbclusterdefined) 
                          ? NULL
                          : &matchcallinfo.matchparam,
#ifdef VMATCHDB
                          &matchcallinfo.databaseparms,
#endif
#ifndef VMATCHDB
                          NULL,
#endif // VMATCHDB
                          True,
                          matchcallinfo.showverbose) != 0)
  {
    STANDARDMESSAGE;
  }
  if(matchinfo.numberofqueryfiles == 0 &&
     matchinfo.virtualtree.sixframeindex != NOTRANSLATIONSCHEME &&
     checkshowstringmultiple(matchcallinfo.outinfo.showstring) != 0)
  {
    STANDARDMESSAGE;
  }
  if(runvmatsel(argc,argv,&matchinfo,&matchcallinfo) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freeMatchinfo(&matchinfo) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freeMatchcallinfo(&matchcallinfo) != 0)
  {
    STANDARDMESSAGE;
  }
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return EXIT_SUCCESS;
}
