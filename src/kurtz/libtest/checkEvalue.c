#include <stdlib.h>
#include "debugdef.h"
#include "errordef.h"
#include "match.h"
#include "args.h"

#include "evalues.pr"
#include "karlin.pr"

#define USAGE "probmatch kmax"
#define FIRSTARG fprintf(stderr,"%s: first argument must be value between 0.0 and 1.0\n",argv[0])
#define SECONDARG fprintf(stderr,"%s: second argument must be positive integer\n",argv[0])

#define EVALUETAB(I) hpt->evaluetable.spaceEvaluelogtype[(I)]

static Evaluetype lookupEvalue(Evalues *hpt,Sint distance,Sint length)
{
  Sint i;

  i = hpt->evaluelinestart.spaceSint[distance] + length;
  if(i < hpt->evaluelinestart.spaceSint[distance+1] + distance + 2)
  {
    return (Evaluetype) EVALUETAB(i);
  }
  return (Evaluetype) 0.0;
}

static void comparekarlin(double probmatch,Sint kmax)
{
  Evalues ev;
  Sint score, l, k;
  double eval, lambda, K;

  inithammingEvalues(&ev,probmatch);
  for(k=0; k<=kmax; k++)
  {
    incprecomputehammingEvalues(&ev,k);
  }
  if(karlinunitcostpp(&lambda, &K) != 0)
  {
    fprintf(stderr,"%s\n",messagespace());
    exit(EXIT_FAILURE);
  }
  printf("lambda=%.2e, K=%.2e\n",lambda,K);
  lambda=2.05289483e-01;
  K=3.41344176e-02;
  for(k=0; k<=kmax; k++)
  {
    for(l=k+1; /* Nothing */; l++)
    {
      eval = lookupEvalue(&ev,k,l);
      if(eval == 0.0)
      {
        break;
      }
      score = EVALDISTANCE2SCORE(k,l,l);
      printf("%ld %ld %.2e %ld %.6e\n",
              (Showsint) l,
              (Showsint) k,
              eval,
              (Showsint) score,
              significance(lambda,K,(double) (234*234),score));
    }
  }
}

MAINFUNCTION
{
  Evaluetype probmatch;
  Scaninteger kmax;

  DEBUGLEVELSET;
  CHECKARGNUM(3,USAGE);

  if(sscanf(argv[1],"%lf",&probmatch) != 1)
  {
    fprintf(stderr,"Usage: %s %s\n",argv[0],USAGE);
    FIRSTARG;
    return EXIT_FAILURE;
  } 
  if(probmatch < 0.0 || probmatch > 1.0)
  {
    FIRSTARG;
    return EXIT_FAILURE;
  }
  if(sscanf(argv[2],"%ld",&kmax) != 1 || kmax < 0)
  {
    SECONDARG;
    return EXIT_FAILURE;
  }
  checkhamming(probmatch,(Sint) kmax);
  printf("checkhamming okay\n");
  comparekarlin(probmatch,(Sint) kmax);
  printf("comparekarlin okay\n");
  return EXIT_SUCCESS;
}
