#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "types.h"
#include "virtualdef.h"
#include "debugdef.h"
#include "optdesc.h"
#include "errordef.h"
#include "intbits.h"
#include "fhandledef.h"
#include "programversion.h"

#include "accvirt.pr"
#include "procopt.pr"
#include "readvirt.pr"
#include "filehandle.pr"
#include "safescpy.pr"

#ifndef NOLICENSEMANAGER
#include "licensemanager.h"
#endif

#define BCKTABHORIZONTAL FIRSTBIT

typedef struct
{
  Uint outtables;
  BOOL showstring;
  char indexname[PATH_MAX+1];
} Callinfo;

typedef enum 
{
  OPTSTRING,
  OPTTIS,
  OPTOIS,
  OPTSUF,
  OPTSTI1,
  OPTBWT,
  OPTBCK,
  OPTBCKHORIZONTAL,
#ifdef WITHLCP
  OPTLCP,
  OPTSKP,
#endif
  OPTCFR,
  OPTCRF,
  OPTLSF,
  OPTSTI,
  OPTCLD,
#ifdef WITHLCP
  OPTISO,
#endif
  OPTVMATCHVERSION,
  OPTHELP,
  NUMOFOPTIONS
} Optionnumber;

/*
  The following function initializes the structure Input and 
  some additional values appropriately.
*/

static Sint parseoptions(Callinfo *callinfo,Argctype argc,const char **argv)
{
  Optionnumbertype optval;
  Uint argnum;
  OptionDescription options[NUMOFOPTIONS];

  initoptions(&options[0],(Uint) NUMOFOPTIONS);
  ADDOPTION(OPTSTRING,"-s","output suffixes");
  ADDOPTION(OPTTIS,"-tis","output tistab");
  ADDOPTION(OPTOIS,"-ois","output oistab");
  ADDOPTION(OPTSUF,"-suf","output suftab");
  ADDOPTION(OPTSTI1,"-sti1","output small inverse suftab");
  ADDOPTION(OPTBWT,"-bwt","output bwttab");
  ADDOPTION(OPTBCK,"-bck","output bcktab in vertical mode");
  ADDOPTION(OPTBCKHORIZONTAL,"-bckhz","output bcktab in horizontal mode");
#ifdef WITHLCP
  ADDOPTION(OPTLCP,"-lcp","output lcptab");
  ADDOPTION(OPTSKP,"-skp","output skptab");
  ADDOPTION(OPTISO,"-iso","output isotab");
#endif
  ADDOPTION(OPTCFR,"-cfr","output cfrtab");
  ADDOPTION(OPTCRF,"-crf","output crftab");
  ADDOPTION(OPTLSF,"-lsf","output lsftab");
  ADDOPTION(OPTSTI,"-sti","output inverse suftab");
  ADDOPTION(OPTCLD,"-cld","output cldtab");
  ADDOPTVMATCHVERSION;
  ADDOPTION(OPTHELP,"-help","this option");

  if(argc == 1)
  {
    USAGEOUT;
    return (Sint) -1;
  } 
  callinfo->showstring = False;
  callinfo->outtables = 0;
  for(argnum = UintConst(1); argnum < (Uint) argc && ISOPTION(argv[argnum]); 
      argnum++)
  {
    optval = procoption(options,(Uint) NUMOFOPTIONS,argv[argnum]);
    if(optval < 0)
    {
      return (Sint) -2;
    }
    switch(optval)
    {
      case OPTHELP:
        showoptions(stdout,argv[0],options,(Uint) NUMOFOPTIONS);
        return (Sint) 1;
      case OPTVMATCHVERSION:
        return (Sint) 1;
      case OPTSTRING:
        callinfo->showstring = True;
        break;
      case OPTTIS:
        callinfo->outtables |= TISTAB;
        break;
      case OPTOIS:
        callinfo->outtables |= OISTAB;
        break;
      case OPTSUF:
        callinfo->outtables |= SUFTAB;
        break;
      case OPTSTI:
        callinfo->outtables |= STITAB; 
        break;
      case OPTSTI1:
        callinfo->outtables |= STI1TAB;
        break;
      case OPTCLD:
        callinfo->outtables |= CLDTAB; 
        break;
      case OPTBWT:
        callinfo->outtables |= BWTTAB;
        break;
      case OPTBCK:
        callinfo->outtables |= BCKTAB;
        break;
      case OPTBCKHORIZONTAL:
        callinfo->outtables |= (BCKTAB | BCKTABHORIZONTAL);
        break;
      case OPTCFR:
        callinfo->outtables |= CFRTAB;
        break;
      case OPTCRF:
        callinfo->outtables |= CRFTAB;
        break;
      case OPTLSF:
        callinfo->outtables |= LSFTAB;
        break;
#ifdef WITHLCP
      case OPTLCP:
        callinfo->outtables |= LCPTAB;
        break;
      case OPTSKP:
        callinfo->outtables |= SKPTAB;
        break;
      case OPTISO:
        callinfo->outtables |= ISOTAB;
        break;
#endif
    }
  }
  if(argnum < (Uint) (argc-1))
  {
    ERROR1("superfluous file argument \"%s\"",argv[argc-1]);
    return (Sint) -3;
  }
  if(argnum >= (Uint) argc)
  {
    ERROR0("missing indexname");
    return (Sint) -4;
  }
  if(safestringcopy(&callinfo->indexname[0],argv[argnum],PATH_MAX) != 0)
  {
    return (Sint) -5;
  }
  return 0;
}

MAINFUNCTION
{
  Virtualtree virtualtree;
  Callinfo callinfo;
  Uint demand;
  Sint retval;
#ifndef NOLICENSEMANAGER
  LmLicense *license;
  if (!(license = lm_license_new_vmatch(argv[0])))
    return EXIT_FAILURE;
#endif

  DEBUGLEVELSET;
#ifndef NOLICENSEMANAGER
  CALLSHOWPROGRAMVERSIONWITHLICENSE("vstree2tex", license);
#else
  CALLSHOWPROGRAMVERSION("vstree2tex");
#endif
  retval = parseoptions(&callinfo,argc,argv);
  if(retval == (Sint) -1)
  {
    SIMPLESTANDARDMESSAGE;
  }
  if(retval < 0)
  {
    STANDARDMESSAGE;
  }
  if(retval == (Sint) 1)
  {
    exit(EXIT_SUCCESS);
  }
  demand = callinfo.outtables;
  if(callinfo.showstring)
  {
    demand |= (TISTAB | SUFTAB | LCPTAB);
  }
  if(mapvirtualtreeifyoucan(&virtualtree,&callinfo.indexname[0],demand) != 0)
  {
    STANDARDMESSAGE;
  }
  if(virtual2tex(callinfo.outtables,
                 (callinfo.outtables & BCKTABHORIZONTAL) ? True: False,
                 callinfo.showstring,&virtualtree) != 0)
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
