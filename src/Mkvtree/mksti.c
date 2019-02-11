#include <stdlib.h>
#include <string.h>
#include "errordef.h"
#include "megabytes.h"
#include "debugdef.h"
#include "virtualdef.h"
#include "fhandledef.h"
#include "spacedef.h"

#include "accvirt.pr"
#include "outindextab.pr"
#include "readvirt.pr"
#include "filehandle.pr"

static Sint makestitab(const char *indexname,Virtualtree *virtualtree)
{
  Uint i;
  
  ALLOCASSIGNSPACE(virtualtree->stitab,NULL,Uint,
                   virtualtree->multiseq.totallength+1);
  for(i=0; i<=virtualtree->multiseq.totallength; i++)
  {
    virtualtree->stitab[virtualtree->suftab[i]] = i;
  }
  if(outindextab(indexname,"sti",
                 (void *) virtualtree->stitab,
                 (Uint) sizeof(Uint),
                 virtualtree->multiseq.totallength+1) != 0)
  {
    return (Sint) -1;
  }
  FREESPACE(virtualtree->stitab);
  return 0;
}

MAINFUNCTION
{
  Virtualtree virtualtree;
  const char *indexname;

  VSTREECHECKARGNUM(2,"indexname");
  DEBUGLEVELSET;
  
  indexname = argv[1];
  if(mapvirtualtreeifyoucan(&virtualtree,indexname,SUFTAB) != 0)
  {
    STANDARDMESSAGE;
  }
  if(makestitab(indexname,&virtualtree) != 0)
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
