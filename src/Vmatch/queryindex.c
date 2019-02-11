
#include "types.h"
#include "debugdef.h"
#include "alphadef.h"
#include "outinfo.h"
#include "procmultiseq.h"
#include "vplugin-interface.h"
#include "vmotif-data.h"

#include "dstrdup.pr"
#include "multiseq.pr"
#include "readmulti.pr"
#include "readvirt.pr"
#include "cmpalpha.pr"
#include "alphabet.pr"

#include "assigndig.pr"

#define SETPROCMULTISEQ(PMSCOMP,VAL)\
        if(procmultiseq != NULL)\
        {\
          procmultiseq->PMSCOMP = VAL;\
        }

static Sint readqueryfromindex(BOOL sixframematch,
                               const char *dnasymbolmapdnavsprot,
                               Alphabet *virtualalpha,
                               Virtualtree *queryvirtualtree,
                               Uint querydemand,
                               const char * const*queryfiles,
                               Showverbose showverbose)
{
  if(mapvirtualtreeifyoucan(queryvirtualtree,
                            queryfiles[0],
                            querydemand) != 0)
  {
    return (Sint) -1;
  }
  if(sixframematch)
  {
    if(dnasymbolmapdnavsprot != NULL)
    {
      ERROR1("when mapping the query sequence index \%s\" from file, "
             "the alphabet is fixed; thus you cannot specify a second "
             "argument for option -dnavsprot",
             queryfiles[0]); 
      return (Sint) -2;
    }
    if(!vm_isdnaalphabet(&queryvirtualtree->alpha))
    {
      ERROR1("query sequence index \%s\" must be over a DNA alphabet",
             queryfiles[0]); 
      return (Sint) -3;
    }
  } else
  {
    if(compareAlphabet(virtualalpha,&queryvirtualtree->alpha) != 0)
    {
      return (Sint) -2;
    }
  }
  if(showverbose != NULL)
  {
    if(showvirtualtreestatus(queryvirtualtree,
                             (const char *) queryfiles[0],
                             showverbose) != 0)
    {
      return (Sint) -3;
    }
  }
  return 0;
}

static Sint inputthequeries(const char *dnasymbolmapdnavsprot,
                            Uint numberofqueryfiles,
                            const char * const*queryfiles,
                            BOOL sixframematch,
                            BOOL storedesc,
                            BOOL maporig,
                            Multiseq *querymultiseq,
                            Alphabet *virtualalpha,
                            Showverbose showverbose)
{
  Uint i;

  for(i=0; i<numberofqueryfiles; i++)
  {
    ASSIGNDYNAMICSTRDUP(querymultiseq->allfiles[i].filenamebuf,queryfiles[i]);
  }
  querymultiseq->totalnumoffiles = numberofqueryfiles;
  querymultiseq->numofqueryfiles = 0;
  if(readmultiseq(storedesc,
                  sixframematch,
                  dnasymbolmapdnavsprot,
                  virtualalpha,
                  numberofqueryfiles,
                  querymultiseq,
                  showverbose) != 0)
  {
    return (Sint) -1;
  }
  if(sixframematch || maporig)
  {
    if(readmultiseqagain(&querymultiseq->allfiles[0],
                         numberofqueryfiles,
                         &querymultiseq->originalsequence,
                         showverbose) != 0)
    {
      return (Sint) -3;
    }
  }
  if(showverbose != NULL)
  {
    char sbuf[80+1];
    if(querymultiseq->numofsequences > UintConst(1))
    {
      sprintf(sbuf,"querylength=%lu (including %lu separators)",
                   (Showuint) querymultiseq->totallength,
                   (Showuint) (querymultiseq->numofsequences-1));
    } else
    {
      sprintf(sbuf,"querylength=%lu",(Showuint) querymultiseq->totallength);
    }
    showverbose(sbuf);
  }
  return 0;
}

static Sint maporreadquerysequences(BOOL sixframematch,
                                    const char *dnasymbolmapdnavsprot,
                                    Uint numberofqueryfiles,
                                    const char * const*queryfiles,
                                    BOOL maporig,
                                    BOOL storedesc,
                                    Alphabet *virtualalpha,
                                    Virtualtree *queryvirtualtree,
                                    BOOL *didreadqueryfromindex,
                                    Showverbose showverbose)
{
  if(numberofqueryfiles == UintConst(1) &&
     checkifvstreetabfileexists(queryfiles[0],PROJECTFILESUFFIX))
  {
    Uint localdemand = TISTAB | DESTAB | SSPTAB;

    if(maporig || sixframematch)
    {
      localdemand |= OISTAB;
    }
    if(readqueryfromindex(sixframematch,
                          dnasymbolmapdnavsprot,
                          virtualalpha,
                          queryvirtualtree,
                          localdemand,
                          queryfiles,
                          showverbose) != 0)
    {
      return (Sint) -1;
    }
    *didreadqueryfromindex = True;
  } else
  {
    initmultiseq(&queryvirtualtree->multiseq);
    if(inputthequeries(dnasymbolmapdnavsprot,
                       numberofqueryfiles,
                       queryfiles,
                       sixframematch,
                       storedesc,
                       maporig,
                       &queryvirtualtree->multiseq,
                       virtualalpha,
                       showverbose) != 0)
    {
      return (Sint) -2;
    }
    *didreadqueryfromindex = False;
  }
  return 0;
}

Sint initoutinfostruct(Outinfo *outinfo,
                       Procmultiseq *procmultiseq,
                       Uint numberofqueryfiles,
                       const char *indexormatchfile,
                       BOOL storedesc, 
                       BOOL sixframequerymatch,
                       Vpluginbundle *vmotifbundle,
                       Vmotifdata *vmotifdata, 
                       const char *dnasymbolmapdnavsprot,
                       const char * const*queryfiles,
                       BOOL maporig,
                       Virtualtree *virtualtree,
                       Virtualtree *dnavirtualtree,
                       Virtualtree *queryvirtualtree,
                       BOOL *didreadqueryfromindex,
                       Showverbose showverbose)
{
  SETPROCMULTISEQ(constvms,&virtualtree->multiseq);
  if(numberofqueryfiles == 0)
  {
    outinfo->outqms = NULL;
    SETPROCMULTISEQ(constqms,NULL);
    if(virtualtree->sixframeindex == NOTRANSLATIONSCHEME)
    {
      outinfo->outvms = &virtualtree->multiseq;
      outinfo->outvmsalpha = &virtualtree->alpha;
    } else
    {
      if(prepareforsixframeselfmatch(dnavirtualtree,
                                     indexormatchfile,
                                     maporig,
                                     showverbose) != 0)
      {
        return (Sint) -1;
      }
      outinfo->outvms = &dnavirtualtree->multiseq;
      outinfo->outvmsalpha = &dnavirtualtree->alpha;
      SETPROCMULTISEQ(constvms,&dnavirtualtree->multiseq);
      SETPROCMULTISEQ(supermatchtask,Protprotmatchondna);
    }
  } else
  {
    outinfo->outvms = &virtualtree->multiseq;
    outinfo->outvmsalpha = &virtualtree->alpha;
    if(virtualtree->sixframeindex != NOTRANSLATIONSCHEME)
    {
      ERROR1("six frame match index \"%s\" cannot be used in combination "
             "with option -q",indexormatchfile);
      return (Sint) -3;
    }
    if(vmotifbundle != NULL && vmotifbundle->handle != NULL)
    {
      if(vmotifbundle->iface.vpluginparse((void *) vmotifdata) != 0)
      {
        return (Sint) -4;
      }
    } else
    {
      if(maporreadquerysequences(sixframequerymatch,
                                 dnasymbolmapdnavsprot,
                                 numberofqueryfiles,
                                 queryfiles,
                                 maporig,
                                 storedesc,
                                 &virtualtree->alpha,
                                 queryvirtualtree,
                                 didreadqueryfromindex,
                                 showverbose) != 0)
      {
        return (Sint) -4;
      }
      outinfo->outqms = &queryvirtualtree->multiseq;
      SETPROCMULTISEQ(constqms,&queryvirtualtree->multiseq);
    }
    assignquerydigits(&outinfo->digits,&queryvirtualtree->multiseq);
  }
  return 0;
}
