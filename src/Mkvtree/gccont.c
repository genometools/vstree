#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "types.h"
#include "errordef.h"

#define TOTALTABWIDTH UintConst(1000)

#define USAGE\
        fprintf(stderr,"Usage: %s <seqlen> A=<probA> ...\n",argv[0]);\
        return EXIT_FAILURE

static Sint checkpercentage(Uint numofchars,Uint *percentages)
{
  Uint i, sum = 0;

  for(i=0; i<numofchars; i++)
  {
    sum += percentages[i];
  }
  if(sum != TOTALTABWIDTH)
  {
    ERROR1("sum of probability is %.2f != 1.0\n",(double) sum/TOTALTABWIDTH);
    return (Sint) -1;
  }
  return 0;
}

static void makepercentagetab(Uint numofchars,Uchar *ptab,Uint *percentages)
{
  Uint i, j, l = 0;

  for(i=0; i<numofchars; i++)
  {
    for(j=0; j < percentages[i]; j++)
    {
      ptab[l++] = (Uchar) i;
    }
  }
}

static Sint makediststring(Uint numofchars,Uchar *alpha,Uint *percentages,
                           Uint seqlen)
{
  Uint i, cc, counttab[UCHAR_MAX+1] = {0};
  Uchar ptab[TOTALTABWIDTH];

  if(checkpercentage(numofchars,percentages) != 0)
  {
    return (Sint) -1;
  }
  makepercentagetab(numofchars,&ptab[0],percentages);
  for(i=UintConst(1); i<=seqlen; i++)
  {
    cc = (Uint) ptab[(Uint) (drand48() * (double) TOTALTABWIDTH)];
    counttab[cc]++;
    (void) putchar((Fputcfirstargtype) alpha[cc]);
    if((i & 63) == 0)
    {
      (void) putchar('\n');
    }
  }
  for(i=0; i < numofchars; i++)
  {
    fprintf(stderr,"#%c %.2f\n",alpha[i],(double) counttab[i]/seqlen);
  }
  (void) putchar('\n');
  return 0;
}

MAINFUNCTION
{
  Uint i, numofchars = 0, seqlen, sumpercent = 0, percentages[UCHAR_MAX+1];
  Scaninteger readint;
  char c;
  BOOL occurred[UCHAR_MAX+1] = {False};
  float readdouble;
  Uchar alphabet[UCHAR_MAX+1];

  if(argc < 2)
  {
    USAGE;
  }
  if(sscanf(argv[1],"%ld",&readint) != 1)
  {
    USAGE;
  }
  if(readint < (Scaninteger) 1)
  {
    fprintf(stderr,"%s: first argument must be positive integer\n",argv[0]);
    return EXIT_FAILURE;
  }
  seqlen = (Uint) readint;
  for(i=UintConst(2); i< (Uint) argc; i++)
  {
    if(sscanf(argv[i],"%c=%f",&c,&readdouble) != 2)
    {
      USAGE;
    }
    if(occurred[(Uint) c])
    {
      fprintf(stderr,"%s: percentage of %c already defined\n",argv[0],c);
      return EXIT_FAILURE;
    }
    occurred[(Uint) c] = True;
    alphabet[numofchars] = (Uchar) c;
    percentages[numofchars] = (Uint) (readdouble * TOTALTABWIDTH);
    numofchars++;
  }
  if(percentages[numofchars-1] == 0)
  {
    for(i=0; i<numofchars-1; i++)
    {
      sumpercent += percentages[i];
    }
    percentages[numofchars-1] = TOTALTABWIDTH - sumpercent;
  }
  printf(">");
  for(i=0; i<numofchars; i++)
  {
    printf("%c=%lu%c",alphabet[i],(Showuint) percentages[i],
               (i < numofchars-1) ? ' ' : '\n');
  }
  if(makediststring(numofchars,alphabet,percentages,seqlen) != 0)
  {
    STANDARDMESSAGE;
  }
  return EXIT_SUCCESS;
}
