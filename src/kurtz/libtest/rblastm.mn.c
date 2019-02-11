#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "types.h"
#include "errordef.h"
#include "alphadef.h"
#include "args.h"
#include "genfile.h"
#include "scoredef.h"
#include "debugdef.h"

#include "scorematrix.pr"

MAINFUNCTION
{
  Alphabet alpha;
  Scorematrix scorematrix;

  DEBUGLEVELSET;

  CHECKARGNUM(2,"blastmatrixfile");
  if(readblastmatrix((const char *) argv[1],UCHAR_MAX,
                     True,&alpha,&scorematrix,(SCORE) -4) != 0)
  {
    fprintf(stderr,"%s: %s",argv[1],messagespace());
    return EXIT_FAILURE;
  }
  showblastmatrix(&alpha,&scorematrix);
  freeblastmatrix(&scorematrix);
  return EXIT_SUCCESS;
}
