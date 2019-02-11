#include <stdlib.h>
#include <string.h>
#include "errordef.h"
#include "megabytes.h"
#include "debugdef.h"
#include "virtualdef.h"
#include "spacedef.h"
#include "fhandledef.h"

#include "accvirt.pr"
#include "readvirt.pr"
#include "filehandle.pr"

MAINFUNCTION
{
  Virtualtree virtualtree;
  const char *indexname;

  VSTREECHECKARGNUM(2,"indexname");
  DEBUGLEVELSET;
  
  indexname = argv[1];
  if(mapallofvirtualtreewhatsthere(&virtualtree,indexname) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
#ifndef NOSPACEBOOKKEEPING
  printf("# overall space peak: main=%.2f MB (%.2f bytes/symbol), "
         "secondary=%.2f MB (%.2f bytes/symbol)\n",
         MEGABYTES(getspacepeak()),
         (double) getspacepeak()/virtualtree.multiseq.totallength,
	 MEGABYTES(mmgetspacepeak()),
	 (double) mmgetspacepeak()/virtualtree.multiseq.totallength);
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return EXIT_SUCCESS;
}
