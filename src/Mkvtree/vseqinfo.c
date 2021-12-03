#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "debugdef.h"
#include "errordef.h"
#include "virtualdef.h"
#include "fhandledef.h"
#include "genfile.h"

#include "accvirt.pr"
#include "readvirt.pr"
#include "filehandle.pr"
#include "multiseq-adv.pr"

MAINFUNCTION
{
  Virtualtree virtualtree;

  VSTREECHECKARGNUM(2,"indexname");
  DEBUGLEVELSET;
  
  if(mapvirtualtreeifyoucan(&virtualtree,argv[1],DESTAB | SSPTAB) != 0)
  {
    STANDARDMESSAGE;
  }
  if(showdescandlength(&virtualtree.multiseq) != 0)
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
