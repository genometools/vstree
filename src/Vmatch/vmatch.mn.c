
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "debugdef.h"
#include "errordef.h"
#include "virtualdef.h"
#include "megabytes.h"
#include "multidef.h"
#include "fhandledef.h"
#include "optdesc.h"
#include "select.h"
#include "programversion.h"

#include "checkonoff.pr"
#include "clock.pr"
#include "vmatch.pr"
#include "readvirt.pr"
#include "filehandle.pr"

#ifndef NOLICENSEMANAGER
#include "licensemanager.h"
#endif

static void showonstdout(char *s)
{
  printf("# %s\n",s);
}

static Sint swallowmatch(/*@unused@*/ void *showmatchinfo,
                         /*@unused@*/ Multiseq *virtualmultiseq,
                         /*@unused@*/ Multiseq *multiseq,
                         /*@unused@*/ StoreMatch *storematch)
{
  return 0;
}

MAINFUNCTION
{
  Virtualtree virtualtree, 
              queryvirtualtree,
              sixframeofqueryvirtualtree,
              dnavirtualtree;
  Sint retcode;
  BOOL vmatchshowtimespace;
#ifndef NOLICENSEMANAGER
  LmLicense *license;
  if (!(license = lm_license_new_vmatch(argv[0])))
    return EXIT_FAILURE;
#endif

  DEBUGLEVELSET;
#ifndef NOLICENSEMANAGER
  CALLSHOWPROGRAMVERSIONWITHLICENSE("vmatch", license);
#else
  CALLSHOWPROGRAMVERSION("vmatch");
#endif
  retcode = checkenvvaronoff("VMATCHSHOWTIMESPACE");
  if(retcode == (Sint) -1)
  {
    STANDARDMESSAGE;
  }
  vmatchshowtimespace = retcode ? True : False;
  if(vmatchshowtimespace)
  {
    initclock();
  }
  makeemptyvirtualtree(&virtualtree);
  makeemptyvirtualtree(&queryvirtualtree);
  makeemptyvirtualtree(&sixframeofqueryvirtualtree);
  makeemptyvirtualtree(&dnavirtualtree);
  retcode = callvmatch(argc,
                       argv,
                       NULL,
                       NULL,
                       vmatchshowtimespace ? "swallocmatch" : "NULL",
                       vmatchshowtimespace ? swallowmatch : NULL,
                       showonstdout,
                       stdout,
                       NULL,
                       &virtualtree,
                       &queryvirtualtree,
                       &sixframeofqueryvirtualtree,
                       &dnavirtualtree);
  if(retcode == (Sint) -1)
  {
    SIMPLESTANDARDMESSAGE;
  }
  if(retcode < 0)
  {
    STANDARDMESSAGE;
  }
  if(retcode == 0)
  {
    if(wrapvmatch(&virtualtree,
                  &queryvirtualtree,
                  &sixframeofqueryvirtualtree,
                  &dnavirtualtree) != 0)
    {
      STANDARDMESSAGE;
    }
  }
#ifndef NOSPACEBOOKKEEPING
  if(vmatchshowtimespace)
  {
    Uint spacepeak = mmgetspacepeak() + getspacepeak();
    printf("# TIME %s %.2f\n",argv[0],getruntime());
    printf("# SPACE %s %.2f\n",argv[0],MEGABYTES(spacepeak));
  }
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
#ifndef NOLICENSEMANAGER
  lm_license_delete(license);
#endif
  return EXIT_SUCCESS;
}
