
#include "matchtask.h"
#include "mparms.h"
#include "procmultiseq.h"
#include "matchstate.h"
#include "vmotif-data.h"

#include "procfinal.pr"

void vmotifstart(Vmotifdata *vmotifdata,
                 Matchcallinfo *matchcallinfo,
                 Virtualtree *virtualtree)
{
  vmotifdata->numberofqueryfiles = matchcallinfo->numberofqueryfiles;
  vmotifdata->virtualtree = virtualtree;
  vmotifdata->forceonline = (matchcallinfo->matchtask & TASKONLINE) 
                              ? True
                              : False;
  vmotifdata->progname = &matchcallinfo->progname[0];
  vmotifdata->indexname = &matchcallinfo->indexormatchfile[0];
  vmotifdata->queryfiles = &matchcallinfo->queryfiles[0];
  vmotifdata->processfinal = processfinal;
  vmotifdata->showverbose = matchcallinfo->showverbose;
}

Sint vmotifinitMatchstate(Vmotifdata *vmotifdata,
                          Matchcallinfo *matchcallinfo,
                          Virtualtree *virtualtree,
                          Procmultiseq *procmultiseq,
                          Currentdirection currentdirection,
                          BOOL revmposorder,
                          Evalues *evalues,
                          BOOL domatchbuffering)
{
  ALLOCASSIGNSPACE(vmotifdata->voidMatchstate,NULL,Matchstate,UintConst(1));
  if(initMatchstate((Matchstate *) vmotifdata->voidMatchstate,
                    virtualtree,
                    NULL,
                    &matchcallinfo->matchparam,
                    matchcallinfo->outinfo.bestinfo.bestflag,
                    (matchcallinfo->outinfo.showmode & SHOWNOEVALUE) 
                      ? True : False,
                    (matchcallinfo->outinfo.showmode & SHOWSELFPALINDROMIC) 
                      ? True : False,
                    matchcallinfo->outinfo.selectbundle,
                    0,
                    procmultiseq,
                    currentdirection,
                    revmposorder,
                    processfinal,
                    evalues,
                    domatchbuffering) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

Sint vmotifwrap(Vmotifdata *vmotifdata)
{
  FREESPACE(vmotifdata->voidMatchstate);
  return 0;
}
