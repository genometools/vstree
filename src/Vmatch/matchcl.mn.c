
#include "errordef.h"
#include "multidef.h"
#include "debugdef.h"
#include "matchinfo.h"
#include "fhandledef.h"
#include "mparms.h"
#include "optdesc.h"
#include "mcldef.h"
#include "programversion.h"

#include "filehandle.pr"

#include "detmatch.pr"
#include "parsemcl.pr"
#include "allmclust.pr"

static void showonstdout(char *s)
{
  printf("# %s\n",s);
}

MAINFUNCTION
{
  Matchinfo matchinfo;
  Matchclustercallinfo matchclustercallinfo;
  Sint retcode;

  DEBUGLEVELSET;
  CALLSHOWPROGRAMVERSION("matchcluster");
  retcode = parsematchcluster(False,
                              &matchclustercallinfo,
                              argv,
                              argc);
  if(retcode == (Sint) 1)
  {
    return EXIT_SUCCESS;
  }
  if(retcode < 0)
  {
    STANDARDMESSAGE;
  }
  if(determineMatchinfo(NULL,
                        &matchinfo,
                        argc,
                        argv,
                        matchclustercallinfo.matchfile,
                        NULL,
                        NULL,
                        True,
                        showonstdout) != 0)
  {
    STANDARDMESSAGE;
  } 
  if(genericmatchclustering(&matchclustercallinfo,
                            matchinfo.outinfo.outvms,
                            matchinfo.outinfo.outvmsalpha,
                            matchinfo.outinfo.outqms,
                            &matchinfo.matchtab,
                            matchinfo.mfargs) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freeMatchinfo(&matchinfo) != 0)
  {
    STANDARDMESSAGE;
  }
  FREESPACE(matchclustercallinfo.matchfile);
  FREESPACE(matchclustercallinfo.outprefix);
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return EXIT_SUCCESS;
}
