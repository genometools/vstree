
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "types.h"
#include "debugdef.h"
#include "errordef.h"
#include "spacedef.h"
#include "megabytes.h"
#include "matchtask.h"
#include "procmultiseq.h"
#include "select.h"

#include "readvirt.pr"

#include "procmatch.pr"
#include "parsevm.pr"
#include "procargs.pr"

#ifndef NOSPACEBOOKKEEPING
static void printspacepeak(Showverbose showverbose,
                           Uint totallength)
{
  if(showverbose != NULL)
  {
    char sbuf[256+1];
    sprintf(sbuf,"overall space peak: main=%.2f MB (%.2f bytes/symbol), secondary=%.2f MB (%.2f bytes/symbol)",
            MEGABYTES(getspacepeak()),
            (double) getspacepeak()/totallength,
            MEGABYTES(mmgetspacepeak()),
            (double) mmgetspacepeak()/totallength);
    showverbose(sbuf);
  }
}
#endif

#ifdef SIXTYFOURBITS
 Sint setlocalpagesize(void);
#endif

Sint callvmatch(Argctype argc,
                const char **argv,
                void *processinfo,
                void initinfo(void *,void *),
                const char *functionname,
                Showmatchfuntype showmatchfun,
                Showverbose showverbose,
                FILE *outfp,
                SelectBundle *precompiledselectbundle,
                Virtualtree *virtualtree,
                Virtualtree *queryvirtualtree,
                Virtualtree *sixframeofqueryvirtualtree,
                Virtualtree *dnavirtualtree)
{
  Matchcallinfo matchcallinfo;
  Procmultiseq procmultiseq;
  Virtualtree *queryvirtualtreeformatching;
  BOOL didreadqueryfromindex;
  Sint pret;

  assert(functionname);

#ifdef SIXTYFOURBITS
  if(setlocalpagesize() != 0)
  {
    return (Sint) -1;
  }
#endif
  pret = parsevmatchargs(True,
                         argc,
                         argv,
                         showverbose,
                         outfp,
                         precompiledselectbundle,
                         &matchcallinfo);
  if(pret == (Sint) -2)
  {
    return (Sint) -1;
  }
  if(pret < 0)
  {
    return (Sint) -2;
  }
  if(pret == (Sint) 1)
  {
    return (Sint) 1;
  }
  if(matchcallinfo.vmotifbundle.handle != NULL)
  {
    if(matchcallinfo.vmotifbundle.iface.vplugininit(&matchcallinfo.vmotifdata)
       != 0)
    {
      return (Sint) -3;
    }
  }
  if(matchcallinfo.cpridxpatsearchbundle.handle != NULL)
  {
    if(matchcallinfo.cpridxpatsearchbundle.iface.vplugininit(
                 &matchcallinfo.cpridxpatsearchdata) != 0)
    {
      return (Sint) -4;
    }
  }
  if(arrangevmatchinput(&matchcallinfo,
                        &procmultiseq,
                        virtualtree,
                        queryvirtualtree,
                        sixframeofqueryvirtualtree,
                        dnavirtualtree,
                        &queryvirtualtreeformatching,
                        &didreadqueryfromindex) != 0)
  {
    return (Sint) -5;
  }
  if(initinfo != NULL)
  {
    initinfo(processinfo,(void *) &matchcallinfo);
  }
  if(procmatch(&matchcallinfo,
               virtualtree,
               queryvirtualtreeformatching,
               &procmultiseq,
               didreadqueryfromindex,
               functionname,
               processinfo,
               showmatchfun) != 0)
  {
    return (Sint) -6;
  }
#ifndef NOSPACEBOOKKEEPING
  if(matchcallinfo.showverbose != NULL)
  {
    printspacepeak(matchcallinfo.showverbose,
                   virtualtree->multiseq.totallength);
  }
#endif
  if(freeMatchcallinfo(&matchcallinfo) != 0)
  {
    return (Sint) -7;
  }
  return 0;
}

Sint wrapvmatch(Virtualtree *virtualtree,
                Virtualtree *queryvirtualtree,
                Virtualtree *sixframeofqueryvirtualtree,
                Virtualtree *dnavirtualtree)
{
  if(freevirtualtree(virtualtree) != 0)
  {
    return (Sint) -1;
  }
  if(freevirtualtree(queryvirtualtree) != 0)
  {
    return (Sint) -2;
  }
  if(freevirtualtree(sixframeofqueryvirtualtree) != 0)
  {
    return (Sint) -1;
  }
  if(freevirtualtree(dnavirtualtree) != 0)
  {
    return (Sint) -2;
  }
  return 0;
}
