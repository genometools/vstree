#include "spacedef.h"
#include "redblackdef.h"
#include "qsortdef.h"
#include "debugdef.h"
#include "arraydef.h"
#include "args.h"

#include "redblack.pr"
#include "remdups.pr"

typedef struct
{
  void *vroot;
  Uint *numtable, *storerange, *nextfreerange;
  ArrayUint sortednumtable;
  PairUint currentpair;
} Numberinfo;

static Sint cmpNumbers(const Keytype keya,
                       const Keytype keyb,
                       /*@unused@*/ void *info)
{
  Uint *v1 = (Uint *) keya,
       *v2 = (Uint *) keyb;

  if(*v1 < *v2)
  {
    return (Sint) -1;
  }
  if(*v1 > *v2)
  {
    return (Sint) 1;
  }
  return 0;
}

static BOOL greaterequalleft(const Keytype keyvalue,void *info)
{
  PairUint *pair = (PairUint *) info;

  if(*((Uint *) keyvalue) >= pair->uint0)
  {
    return True;
  }
  return False;
}

static BOOL lowerequalright(const Keytype keyvalue,void *info)
{
  PairUint *pair = (PairUint *) info;

  if(*((Uint *) keyvalue) <= pair->uint1)
  {
    return True;
  }
  return False;
}

static void insertthenumbers(Numberinfo *numberinfo,Uint iterations,Uint maxnum)
{
  Uint i;
  BOOL nodecreated;

  ALLOCASSIGNSPACE(numberinfo->numtable,NULL,Uint,iterations);
  ALLOCASSIGNSPACE(numberinfo->sortednumtable.spaceUint,NULL,Uint,iterations);
  ALLOCASSIGNSPACE(numberinfo->storerange,NULL,Uint,iterations);
  numberinfo->vroot = NULL;
  srand48(42349421);
  for(i=0; i<iterations; i++)
  {
    numberinfo->numtable[i] = (Uint) (drand48() * (double) (maxnum+1));
    numberinfo->sortednumtable.spaceUint[i] = numberinfo->numtable[i];
    (void) redblacktreesearch ((Keytype) &numberinfo->numtable[i],
                               &nodecreated,
                               &numberinfo->vroot,
                               cmpNumbers,
                               NULL);
  }
  numberinfo->sortednumtable.nextfreeUint = iterations;
  // treeshape (numberinfo->vroot,0);
  qsortUint(numberinfo->sortednumtable.spaceUint,
            numberinfo->sortednumtable.spaceUint+
            numberinfo->sortednumtable.nextfreeUint-1);
  remdups(&numberinfo->sortednumtable);
}

static Sint applyredblackwalkrange(const Keytype bmkey,
                                   /*@unused@*/ VISIT which,
                                   /*@unused@*/ Uint depth,
                                   void *applyinfo)
{
  Numberinfo *numberinfo = (Numberinfo *) applyinfo;
  Uint *currentvalue = (Uint *) bmkey;

  if(*currentvalue < numberinfo->currentpair.uint0)
  {
    fprintf(stderr,"currentvalue = %lu < %lu = leftrange\n",
                     (Showuint) *currentvalue,
                     (Showuint) numberinfo->currentpair.uint0);
    exit(EXIT_FAILURE);
  }
  if(*currentvalue > numberinfo->currentpair.uint1)
  {
    fprintf(stderr,"currentvalue = %lu > %lu = secondvalue\n",
                     (Showuint) *currentvalue,
                     (Showuint) numberinfo->currentpair.uint0);
    exit(EXIT_FAILURE);
  }
  *(numberinfo->nextfreerange++) = *currentvalue;
  return 0;
}

static Sint checkrange(Numberinfo *numberinfo,Uint left,Uint right)
{
  numberinfo->currentpair.uint0 = left;
  numberinfo->currentpair.uint1 = right;
  numberinfo->nextfreerange = numberinfo->storerange;
  if(redblacktreewalkrange (numberinfo->vroot,
                            applyredblackwalkrange,
                            numberinfo,
                            greaterequalleft,
                            lowerequalright,
                            &numberinfo->currentpair) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

static void shownumbers(Uint *tab,Uint sizetab)
{
  Uint i;

  for(i=0; i<sizetab; i++)
  {
    printf("%lu ",(Showuint) tab[i]);
  }
  printf("\n");
}

static Sint checkallranges(Numberinfo *numberinfo)
{
  Uint i, j, foundrangesize, expectedsize;

  for(i=0; i<numberinfo->sortednumtable.nextfreeUint; i++)
  {
    for(j=i; j<numberinfo->sortednumtable.nextfreeUint; j++)
    {
      if(checkrange(numberinfo,numberinfo->sortednumtable.spaceUint[i],
                               numberinfo->sortednumtable.spaceUint[j]) != 0)
      {
        return (Sint) -1;
      }
      foundrangesize = (Uint) (numberinfo->nextfreerange - 
                               numberinfo->storerange);
      expectedsize = j-i+1;
      if(foundrangesize != expectedsize)
      {
        fprintf(stderr,"foundrangesize = %lu != %lu expectedsize\n",
                        (Showuint) foundrangesize,
                        (Showuint) expectedsize);
        shownumbers(numberinfo->storerange,foundrangesize);
        shownumbers(numberinfo->sortednumtable.spaceUint + i,expectedsize);
        exit(EXIT_FAILURE);
      }
    }
  }
  return 0;
}

MAINFUNCTION
{
  Scaninteger readint;
  Uint iterations, maxnum;
  Numberinfo numberinfo;

  DEBUGLEVELSET;
  CHECKARGNUM(3,"numbers maxnum");
  if(sscanf(argv[1],"%ld",&readint) != 1 || readint < (Scaninteger) 1)
  {
    fprintf(stderr,"%s: illegal first argument\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  iterations = (Uint) readint;
  if(sscanf(argv[2],"%ld",&readint) != 1 || readint < (Scaninteger) 1)
  {
    fprintf(stderr,"%s: illegal second argument\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  maxnum = (Uint) readint;
  insertthenumbers(&numberinfo,iterations,maxnum);
  if(checkallranges(&numberinfo) != 0)
  {
    STANDARDMESSAGE;
  }
  redblacktreedestroy (False,NULL,NULL,numberinfo.vroot);
  FREESPACE(numberinfo.numtable);
  FREEARRAY(&numberinfo.sortednumtable,Uint);
  FREESPACE(numberinfo.storerange);
  checkspaceleak();
  return EXIT_SUCCESS;
}
