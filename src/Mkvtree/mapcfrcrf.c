#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "debugdef.h"
#include "errordef.h"
#include "virtualdef.h"
#include "minmax.h"
#include "arraydef.h"
#include "spacedef.h"
#include "fhandledef.h"

#include "accvirt.pr"
#include "readvirt.pr"
#include "safescpy.pr"
#include "filehandle.pr"

/*
  This file contains the function makereversetable for the construction of 
  the reverse tables in the affix affix array data structure
*/

static void showonstdout(char *s)
{
  printf("%s\n",s);
}

MAINFUNCTION
{
  Virtualtree virtualtree;
  char findexname[PATH_MAX+1], 
       rindexname[PATH_MAX+4+1];

  DEBUGLEVELSET;

  VSTREECHECKARGNUM(2,"indexname");
  if(safestringcopy(&findexname[0],argv[1],PATH_MAX) != 0)
  {
    STANDARDMESSAGE;
  }
  if(mapvirtualtreeifyoucan(&virtualtree,&findexname[0],
                            SUFTAB | LCPTAB | SKPTAB | CFRTAB) != 0)
  {
    STANDARDMESSAGE;
  }
  if(showvirtualtreestatus(&virtualtree,&findexname[0],showonstdout) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }

  if(safestringcopy(&rindexname[0],argv[1],PATH_MAX) != 0)
  {
    STANDARDMESSAGE;
  }
  strcat(&rindexname[0],".rev");
  if(mapvirtualtreeifyoucan(&virtualtree,&rindexname[0],
                            SUFTAB | LCPTAB | SKPTAB | CRFTAB) != 0)
  {
    STANDARDMESSAGE;
  }
  if(showvirtualtreestatus(&virtualtree,&rindexname[0],showonstdout) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return EXIT_SUCCESS;
}
