#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errordef.h"
#include "debugdef.h"
#include "virtualdef.h"
#include "megabytes.h"
#include "fhandledef.h"
#include "optdesc.h"
#include "programversion.h"

#include "accvirt.pr"
#include "readvirt.pr"
#include "filehandle.pr"

#ifndef NOLICENSEMANAGER
#include "licensemanager.h"
#endif

static void showonstdout(char *s)
{
  printf("%s\n",s);
}

#ifndef NOSPACEBOOKKEEPING
static void showspacepeak(Showverbose showverbose,Uint seqlen)
{
  if(showverbose != NULL)
  {
    char sbuf[256];
    sprintf(sbuf,
      "overall space peak: main=%.2f MB (%.2f bytes/symbol), secondary=%.2f MB",
      MEGABYTES(getspacepeak()),
      (double) getspacepeak()/seqlen,
      MEGABYTES(mmgetspacepeak()));
    showverbose(sbuf);
  }
}
#endif

MAINFUNCTION
{
  Virtualtree virtualtree;
  Sint ret;
#ifndef NOSPACEBOOKKEEPING
  Sint i;
#endif
#ifndef NOLICENSEMANAGER
  LmLicense *license;
  if (!(license = lm_license_new_vmatch(argv[0])))
    return EXIT_FAILURE;
#endif

  DEBUGLEVELSET;
#ifndef NOLICENSEMANAGER
  CALLSHOWPROGRAMVERSIONWITHLICENSE("mkvtree", license);
#else
  CALLSHOWPROGRAMVERSION("mkvtree");
#endif
  makeemptyvirtualtree(&virtualtree);
  ret = callmkvtree(argc,argv,True,&virtualtree,True,showonstdout);
  if(ret == (Sint) 1)
  {
    return EXIT_SUCCESS;
  }
  if(ret == (Sint) -1)
  {
    SIMPLESTANDARDMESSAGE;
  }
  if(ret < 0)
  {
    STANDARDMESSAGE;
  }
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
#ifndef NOSPACEBOOKKEEPING
  for(i=(Sint) 1; i<(Sint) argc; i++)
  {
    if(strcmp(argv[i],"-v") == 0)
    {
      showspacepeak(showonstdout,virtualtree.multiseq.totallength);
      break;
    }
  }
  checkspaceleak();
  mmcheckspaceleak();
#endif
  checkfilehandles();
#ifndef NOLICENSEMANAGER
  lm_license_delete(license);
#endif
  return EXIT_SUCCESS;
}
