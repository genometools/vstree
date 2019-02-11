
#include "matchtask.h"
#include "cpridx-data.h"

#include "procfinal.pr"

Sint cpridxpatsearchstart(Cpridxpatsearchdata *cpridxpatsearchdata,
                          Matchcallinfo *matchcallinfo,
                          Virtualtree *virtualtree)
{
  cpridxpatsearchdata->progname = &matchcallinfo->progname[0];
  cpridxpatsearchdata->indexname = &matchcallinfo->indexormatchfile[0];
  cpridxpatsearchdata->processfinal = processfinal;
  cpridxpatsearchdata->showverbose = matchcallinfo->showverbose;
  cpridxpatsearchdata->virtualtree = virtualtree;
  return 0;
}
