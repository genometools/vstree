#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include "types.h"
#include "args.h"
#include "chardef.h"
#include "alphadef.h"
#include "genfile.h"
#include "scoredef.h"
#include "errordef.h"

#include "alphabet.pr"
#include "scorematrix.pr"
#include "cmpalpha.pr"

MAINFUNCTION
{
  Alphabet alpha;
  const char *symbolmapfile;
  BOOL isdna, isprotein, dnamapfile, proteinmapfile;

  CHECKARGNUM(2,"symbolmapfile");
  symbolmapfile = argv[1];
  if(readsymbolmap(&alpha,(Uint) UNDEFCHAR,(Uint) WILDCARD,symbolmapfile) != 0)
  {
    fprintf(stderr,"%s %s: %s\n",argv[0],argv[1],messagespace());
    return EXIT_FAILURE;
  }
  if(strcmp(symbolmapfile,"TransProt20") == 0)
  {
    Alphabet proteinalpha;

    assignProteinalphabet(&proteinalpha);
    if(compareAlphabet(&alpha,&proteinalpha) != 0)
    {
      STANDARDMESSAGE;
    }
  } else
  {
    if(strcmp(symbolmapfile,"TransDNA") == 0)
    {
      Alphabet dnaalphabet;

      assignDNAalphabet(&dnaalphabet);
      if(compareAlphabet(&alpha,&dnaalphabet) != 0)
      {
        STANDARDMESSAGE;
      }
    }
  }
  showsymbolmap(&alpha);
  isdna = vm_isdnaalphabet(&alpha);
  isprotein = vm_isproteinalphabet(&alpha);
  dnamapfile = (strstr(symbolmapfile,"DNA") == NULL) ? False : True;
  proteinmapfile = (strstr(symbolmapfile,"Prot") == NULL) ? False : True;
  if(isdna != dnamapfile)
  {
    fprintf(stderr,"%s: file %s: isdna=%s != %s = dnamapfile\n",
                    argv[0],symbolmapfile,SHOWBOOL(isdna),SHOWBOOL(dnamapfile));
    return EXIT_FAILURE;
  }
  if(isprotein != proteinmapfile)
  {
    fprintf(stderr,"%s: file %s: isdna=%s != %s = dnamapfile\n",
                    argv[0],symbolmapfile,SHOWBOOL(isprotein),
                    SHOWBOOL(proteinmapfile));
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
