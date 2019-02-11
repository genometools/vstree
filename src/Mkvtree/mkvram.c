#include <stdio.h>
#include <stdlib.h>
#include "errordef.h"
#include "debugdef.h"
#include "virtualdef.h"
#include "fhandledef.h"

#include "accvirt.pr"
#include "readvirt.pr"
#include "filehandle.pr"

static void showonstdout(char *s)
{
  printf("%s\n",s);
}

MAINFUNCTION
{
  Virtualtree virtualtree;
  DEBUGLEVELSET;

  makeemptyvirtualtree(&virtualtree);
  if(callmkvtree(argc,argv,False,&virtualtree,True,showonstdout) != 0)
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
