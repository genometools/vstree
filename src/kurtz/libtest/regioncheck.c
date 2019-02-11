#include <string.h>
#include "types.h"
#include "debugdef.h"
#include "megabytes.h"
#include "spacedef.h"
#include "arraydef.h"
#include "redblackdef.h"

#include "regionsmerger.pr"
#include "redblack.pr"

#define MINREGIONSIZE 2

#define USAGE\
        fprintf(stderr,"Usage: %s <width> <numberofregions>\n",argv[0]);\
        return EXIT_FAILURE;

static Sint echoregion (const Keytype key,
                        VISIT which,
                        /*@unused@ */ Uint depth,
                        /*@unused@ */ void *info)
{
  if (which == postorder || which == leaf)
  {
    PairUint *region = (PairUint *) key;

    printf("(%lu,%lu)",(Showuint) region->uint0,(Showuint) region->uint1);
  }
  return 0;
}

static Sint testregionsmerger (Uint width,
                               Uint numberofregions)
{
  Uint i, regionsize, minbound, maxbound, maxsize, maxregionsize,
       countregions = 0;
  PairUint currentregion;
  ArrayPairUint originalregions;
  BOOL nodecreated;
  void *regiontreeroot = NULL;

  srand48 (42349421);
  ALLOCASSIGNSPACE (originalregions.spacePairUint, NULL, PairUint,
                    numberofregions);
  originalregions.nextfreePairUint = numberofregions;
  originalregions.allocatedPairUint = numberofregions;

  minbound = width;
  maxbound = 0;
  maxsize = 0;
  if(width <= UintConst(100))
  {
    maxregionsize = UintConst(10);
  } else
  {
    if(width <= UintConst(1000))
    {
      maxregionsize = UintConst(20);
    } else
    {
      maxregionsize = UintConst(40);
    }
  }
  for (i = 0; i < numberofregions; i++)
  {
    currentregion.uint0 = (Uint) (width - maxregionsize) * drand48 ();
    regionsize = (Uint) (maxregionsize - MINREGIONSIZE + 1) * drand48 ();
    currentregion.uint1 = currentregion.uint0 + regionsize + MINREGIONSIZE - 1;
    if (regionsize > maxsize)
    {
      maxsize = regionsize;
    }
    if (currentregion.uint0 < minbound)
    {
      minbound = currentregion.uint0;
    }
    if (currentregion.uint1 > maxbound)
    {
      maxbound = currentregion.uint1;
    }
    originalregions.spacePairUint[i] = currentregion;
    DEBUG2 (2, "new region (%lu,%lu)\n", 
            (Showuint) currentregion.uint0,
            (Showuint) currentregion.uint1);
    nodecreated = False;
    if (insertnewregion (&regiontreeroot, 
                         True,
                         &nodecreated, 
                         &currentregion) != 0)
    {
      return (Sint) -1;
    }
    if(nodecreated)
    {
      countregions++;
    }
  }
  printf ("minbound=%lu, maxbound=%lu, regionsize=%lu\n",
          (Showuint) minbound, (Showuint) maxbound, (Showuint) maxsize);
  printf("the intervals=");
  if (redblacktreewalk (regiontreeroot, echoregion, NULL) != 0)
  {
    return (Sint) -1;
  }
  printf("\n");
  if(verifyregiontree(regiontreeroot,
                      width,
                      &originalregions) != 0)
  {
    return (Sint) -2;
  }
  FREEARRAY (&originalregions,PairUint);
  redblacktreedestroy (True, NULL, NULL, regiontreeroot);
  return 0;
}

MAINFUNCTION
{
  Scaninteger readint;
  Uint width, numberofregions;

  DEBUGLEVELSET;
  if (argc != 3)
  {
    USAGE;
  }
  if (sscanf (argv[1], "%ld", &readint) != 1 || readint < (Scaninteger) 1)
  {
    USAGE;
  }
  width = (Uint) readint;
  if (sscanf (argv[2], "%ld", &readint) != 1 || readint < (Scaninteger) 1)
  {
    USAGE;
  }
  numberofregions = (Uint) readint;
  if (testregionsmerger (width, numberofregions) != 0)
  {
    STANDARDMESSAGE;
  }
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak ();
#endif
  mmcheckspaceleak ();
  return EXIT_SUCCESS;
}
