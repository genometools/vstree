
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "match.h"
#include "matchtask.h"
#include "errordef.h"
#include "fhandledef.h"
#include "optdesc.h"
#include "debugdef.h"
#include "matchinfo.h"
#include "chaindef.h"
#include "chaincall.h"
#include "programversion.h"

#include "dstrdup.pr"
#include "procopt.pr"
#include "filehandle.pr"

#include "detmatch.pr"
#include "chainvm.pr"
#include "chainof.pr"
#include "procargs.pr"
#include "chncallparse.pr"
#include "echomatch.pr"

#ifndef NOLICENSEMANAGER
#include "licensemanager.h"
#endif

MAINFUNCTION
{
  Chaincallinfo chaincallinfo;
  Sint ret;
#ifndef NOLICENSEMANAGER
  LmLicense *license;
  if (!(license = lm_license_new_vmatch(argv[0])))
    return EXIT_FAILURE;
#endif

  DEBUGLEVELSET;
#ifndef NOLICENSEMANAGER
  CALLSHOWPROGRAMVERSIONWITHLICENSE("chain2dim", license);
#else
  CALLSHOWPROGRAMVERSION("chain2dim");
#endif
  ret = parsechain2dim(False,&chaincallinfo, argv, argc);
  if(ret == (Sint) 1)
  {
    return EXIT_SUCCESS;
  }
  if(ret < 0)
  {
    STANDARDMESSAGE;
  }
  ret = openformatchaining(&chaincallinfo);
  if(ret == (Sint) -2)
  {
    STANDARDMESSAGE;
  }
  if(ret == (Sint) -1)  // input error for open format
  {
    Matchinfo matchinfo;
    Processmatch finalprocessthread;
    BOOL withinputsequences = False;

#ifdef DEBUG
    withinputsequences = True;
#else
    if(extractoption("-thread",argv,argc) >= 0)
    {
      withinputsequences = True;
    }
#endif

    matchinfo.outinfo.showstring = 0;
    if(determineMatchinfo(NULL,
                          &matchinfo,
                          argc,
                          argv, 
                          chaincallinfo.matchfile,
                          NULL,
                          NULL,
                          withinputsequences,
                          chaincallinfo.showverbose) != 0)
    {
      STANDARDMESSAGE;
    }
    matchinfo.outinfo.outfp = stdout;
    if(chaincallinfo.showverbose != NULL)
    {
      char vbuf[PATH_MAX+80+1];
      sprintf(vbuf,"match file \"%s\" (Vmatch format) read",
                    chaincallinfo.matchfile);
      chaincallinfo.showverbose(vbuf);
    }
    finalprocessthread.functionname = NULL;
    ASSIGNPROCESSMATCH(&finalprocessthread,&matchinfo.outinfo,
                       immediatelyechothematch);
    if(vmatchchaining(matchinfo.mfargs,
                      &matchinfo.outinfo,
                      matchinfo.matchtab.spaceStoreMatch,
                      matchinfo.matchtab.nextfreeStoreMatch,
                      matchinfo.virtualtree.multiseq.numofsequences,
                      &chaincallinfo,
                      &finalprocessthread) != 0)
    {
      STANDARDMESSAGE;
    }
    if(freeMatchinfo(&matchinfo) != 0)
    {
      STANDARDMESSAGE;
    }
    FREESPACE(finalprocessthread.functionname);
  }
  FREESPACE(chaincallinfo.outprefix);
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
#ifndef NOLICENSEMANAGER
  lm_license_delete(license);
#endif
  return EXIT_SUCCESS;
}
