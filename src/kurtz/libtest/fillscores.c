#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "types.h"
#include "alphadef.h"
#include "args.h"
#include "genfile.h"
#include "scoredef.h"
#include "spacedef.h"
#include "debugdef.h"

#include "alphabet.pr"
#include "scorematrix.pr"

MAINFUNCTIONWITHUNUSEDARGUMENTS
{
  Alphabet alpha;
  Scorefunction scorefunction;
  SCORE matchscore = (SCORE) 2,
        mismatchscore = (SCORE) -1,
        indelscore = (SCORE) -2;

  DEBUGLEVELSET;

  assignDNAalphabet(&alpha);
  fillscorematrix(&scorefunction,&alpha,matchscore,mismatchscore,indelscore);
  showblastmatrix(&alpha,&scorefunction.scorematrix);
  FREESPACE(scorefunction.scorematrix.scoretab);
  return EXIT_SUCCESS;
}
