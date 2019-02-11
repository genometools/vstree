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

#ifndef NOLICENSEMANAGER
#include "licensemanager.h"
#endif

MAINFUNCTION
{
  Virtualtree virtualtree;
#ifndef NOLICENSEMANAGER
  LmLicense *license;
  if (!(license = lm_license_new_vmatch(argv[0])))
    return EXIT_FAILURE;
#endif

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
#ifndef NOLICENSEMANAGER
  lm_license_delete(license);
#endif
  return EXIT_SUCCESS;
}
