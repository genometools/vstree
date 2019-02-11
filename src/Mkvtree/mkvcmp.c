#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errordef.h"
#include "debugdef.h"
#include "virtualdef.h"
#include "genfile.h"
#include "fhandledef.h"

#include "accvirt.pr"
#include "readvirt.pr"
#include "filehandle.pr"
#include "safescpy.pr"
#include "alphabet.pr"

static void showonstdout(char *s)
{
  printf("%s\n",s);
}

MAINFUNCTION
{
  Virtualtree virtualtreeram, virtualtreeout, virtualtreemapped;
  DEBUGLEVELSET;

  makeemptyvirtualtree(&virtualtreeram);
  makeemptyvirtualtree(&virtualtreeout);
  makeemptyvirtualtree(&virtualtreemapped);
  if(callmkvtree(argc,argv,True,&virtualtreeout,True,showonstdout) != 0)
  {
    STANDARDMESSAGE;
  }
  printf("virtualtreeout.alphabet:\n");
  showsymbolmap(&virtualtreeout.alpha);
  if(freevirtualtree(&virtualtreeout) != 0)
  {
    STANDARDMESSAGE;
  }
  if(mapvirtualtreeifyoucan(&virtualtreemapped,"all",BASICTABS) != 0)
  {
    STANDARDMESSAGE;
  }
  printf("virtualtreemapped.alphabet:\n");
  showsymbolmap(&virtualtreemapped.alpha);
  if(strcmp(argv[argc-2],"-indexname") != 0 || 
     strcmp(argv[argc-1],"all") != 0)
  {
    fprintf(stderr,"%s: last two arguments must be -indexname all\n",argv[0]);
    return EXIT_FAILURE;
  }
  if(callmkvtree(argc-2,argv,False,&virtualtreeram,True,showonstdout) != 0)
  {
    STANDARDMESSAGE;
  }
  printf("virtualtreeram.alphabet:\n");
  showsymbolmap(&virtualtreeram.alpha);
  compareVirtualtree(&virtualtreeram,&virtualtreemapped);
  if(freevirtualtree(&virtualtreeram) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freevirtualtree(&virtualtreemapped) != 0)
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
