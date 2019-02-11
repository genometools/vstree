#include <stdio.h>
#include "virtualdef.h"
#include "spacedef.h"
#include "debugdef.h"
#include "megabytes.h"
#include "fhandledef.h"

#include "outindextab.pr"
#include "accvirt.pr"
#include "readvirt.pr"
#include "filehandle.pr"
#include "mkcld.pr"

static Sint makecldtabandoutput(const char *indexname,Virtualtree *virtualtree)
{
  ALLOCASSIGNSPACE(virtualtree->cldtab,NULL,Childinfo,
                   virtualtree->multiseq.totallength+1);
  mkcldtabnextlIndex(virtualtree->cldtab,
                     virtualtree->lcptab,
                     &virtualtree->largelcpvalues,
                     virtualtree->multiseq.totallength);
  mkcldtabupdown(virtualtree->cldtab,
                 virtualtree->lcptab,
                 &virtualtree->largelcpvalues,
                 virtualtree->multiseq.totallength);
  
  if(outindextab(indexname,"cld",(void *) virtualtree->cldtab,
                 (Uint) sizeof(Childinfo),
                 virtualtree->multiseq.totallength+1) != 0)
  {
    return (Sint) -1;
  }
  ALLOCASSIGNSPACE(virtualtree->cldtab1,NULL,Uchar,
                  virtualtree->multiseq.totallength+1);
  compresscldtab(virtualtree,virtualtree->cldtab1);
  if(outindextab(indexname,"cld1",(void *) virtualtree->cldtab1,
                 (Uint) sizeof(Uchar),
                 virtualtree->multiseq.totallength+1) != 0)
  {
    return (Sint) -1;
  }
  FREESPACE(virtualtree->cldtab);
  FREESPACE(virtualtree->cldtab1);
  return 0;
}

MAINFUNCTION
{
  Virtualtree virtualtree;
  const char *indexname;

  VSTREECHECKARGNUM(2,"indexname");
  DEBUGLEVELSET;
  
  indexname = argv[1];
  if(mapvirtualtreeifyoucan(&virtualtree,indexname,SUFTAB | LCPTAB) != 0)
  {
    STANDARDMESSAGE;
  }
  if(makecldtabandoutput(indexname,&virtualtree) != 0)
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
