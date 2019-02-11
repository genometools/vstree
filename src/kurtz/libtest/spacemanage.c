#include <string.h>
#include "types.h"
#include "debugdef.h"
#include "megabytes.h"
#include "spacedef.h"

#define USAGE\
        fprintf(stderr,"Usage: %s <maxnumofblocks> [malloc|viaptr]\n",argv[0]);\
        return EXIT_FAILURE;

static Uint anchor = 0;

typedef enum
{
  Actionmalloc,
  Actionrealloc,
  Actionfree
} Actionkind;

typedef enum
{
  Noallocationcalls,
  Usemalloccalls,
  Usespaceviaptrcalls
} Runkind;

static Uint getnextwithvalue(BOOL nullindex,void **blocks,Uint maxnumofblocks)
{
  Uint trials = 0, randindex = (Uint) maxnumofblocks * drand48();
  
  while(True)
  {
    if(nullindex)
    {
      if(blocks[randindex] == NULL)
      {
        return randindex;
      }
    } else
    {
      if(blocks[randindex] != NULL)
      {
        return randindex;
      }
    }
    if(randindex == maxnumofblocks-1)
    {
      randindex = 0;
    } else
    {
      randindex++;
    }
    trials++;
    if(trials >= maxnumofblocks)
    {
      fprintf(stderr,"Cannot find spaceblock %s NULL\n",
              nullindex ? "==" : "!=");
      exit(EXIT_FAILURE);
    }
  }
}

static void spaceAction(Runkind runkind,
                        Actionkind akind,
                        void **blocks,
                        Uint maxnumofblocks)
{
  BOOL nullindex;
  Uint actionindex;
      Uint allocblocks = 1 + (Uint) 999 * drand48();

  if(akind == Actionmalloc)
  {  
    nullindex = True;
  } else
  {
    nullindex = False;
  }
  actionindex = getnextwithvalue(nullindex,blocks,maxnumofblocks);
  if(akind == Actionfree)
  {
    allocblocks = 0;
  } else
  {
    allocblocks = 1 + (Uint) 999 * drand48();
  }
  if(akind == Actionfree)
  {
    DEBUG0(2,"free");
  } else
  {
    if(akind == Actionmalloc)
    {
      DEBUG0(2,"malloc");
    } else
    {
      DEBUG0(2,"realloc");
    }
  }
  DEBUG2(2," (%lu) bytes at index %lu\n",(Showuint) allocblocks,
                                         (Showuint) actionindex);
  if(akind == Actionfree)
  {
    if(runkind == Noallocationcalls)
    {
      blocks[actionindex] = NULL;
    } else
    {
      if(runkind == Usemalloccalls)
      {
        free(blocks[actionindex]);
        blocks[actionindex] = NULL;
      } else
      {
        FREESPACE(blocks[actionindex]);
      }
    }
  } else
  {
    if(runkind == Noallocationcalls)
    {
      blocks[actionindex] = (void *) &anchor;
    } else
    {
      if(runkind == Usemalloccalls)
      {
        if(akind == Actionmalloc)
        {
          blocks[actionindex] = malloc(sizeof(void *) * allocblocks);
          if(blocks[actionindex] == NULL)
          {
            fprintf(stderr,"malloc (%lu) failed\n",(Showuint) allocblocks);
            exit(EXIT_FAILURE);
          }
        } else
        {
          blocks[actionindex] 
           = realloc(blocks[actionindex],sizeof(void *) * allocblocks);
          if(blocks[actionindex] == NULL)
          {
	    fprintf(stderr,"realloc (%lu) failed\n",(Showuint) allocblocks);
            exit(EXIT_FAILURE);
          }
        }
      } else
      {
        if(akind == Actionmalloc)
        {
          ALLOCASSIGNSPACE(blocks[actionindex],NULL,void *,allocblocks);
        } else
        {
          ALLOCASSIGNSPACE(blocks[actionindex],blocks[actionindex],
                           void *,allocblocks);
        }
      }
    }
  }
}

static void testspacemanage(Uint maxnumofblocks,Runkind runkind)
{
  double randnum, probmalloc = 0.5, probrealloc = 0.3, step = 0.00001;
  void **blocks;
  Uint i, 
       numberofblocks = 0,
       countfree = 0, 
       countmalloc = 0, 
       countrealloc = 0;

  srand48(42349421);
  if(maxnumofblocks <= UintConst(10))
  {
    numberofblocks = UintConst(2);
  } else
  {
    numberofblocks = maxnumofblocks/10;
  }
  blocks = (void **) malloc(sizeof(void *) * (size_t) maxnumofblocks);
  if(blocks == NULL)
  {
    fprintf(stderr,"cannot malloc %lu blocks\n",(Showuint) maxnumofblocks);
    exit(EXIT_FAILURE);
  }
  for(i=0; i<maxnumofblocks; i++)
  {
    blocks[i] = NULL;
  }
  for(i=0; i<numberofblocks; i++)
  {
    spaceAction(runkind,Actionmalloc,blocks,maxnumofblocks);
    countmalloc++;
  }
  while(numberofblocks > 0)
  {
    if(probmalloc == 0.0)
    {
      if(probrealloc > step)
      {
        probrealloc -= step;
      } 
    } else
    {
      if(numberofblocks >= maxnumofblocks)
      {
        probmalloc = 0.0;
        probrealloc = 0.5;
      }
    }
    randnum = drand48();
    if(randnum < probmalloc)
    {
      spaceAction(runkind,Actionmalloc,blocks,maxnumofblocks);
      countmalloc++;
      numberofblocks++;
    } else
    {
      if(randnum < (probmalloc + probrealloc))
      {
        spaceAction(runkind,Actionrealloc,blocks,maxnumofblocks);
        countrealloc++;
      } else
      {
        spaceAction(runkind,Actionfree,blocks,maxnumofblocks);
        countfree++;
        numberofblocks--;
      }
    }
  }
  printf("%lu malloc actions\n",(Showuint) countmalloc);
  printf("%lu realloc actions\n",(Showuint) countrealloc);
  printf("%lu countfree actions\n",(Showuint) countfree);
  free(blocks);
}

MAINFUNCTION
{
  Scaninteger readint;
  Runkind runkind;

  DEBUGLEVELSET;
  if(argc != 3 && argc != 2)
  {
    USAGE;
  }
  if(sscanf(argv[1],"%ld",&readint) != 1 || readint < (Scaninteger) 1)
  {
    USAGE;
  }
  if(argc == 2)
  {
    runkind = Noallocationcalls;
  } else
  {
    if(strcmp(argv[2],"malloc") == 0)
    {
      runkind = Usemalloccalls;
    } else
    {
      if(strcmp(argv[2],"viaptr") == 0)
      {
        runkind = Usespaceviaptrcalls;
      } else
      {
        USAGE;
      }
    }
  }
  testspacemanage((Uint) readint,runkind);
#ifndef NOSPACEBOOKKEEPING
  if(runkind == Usespaceviaptrcalls)
  {
    printf("# overall main memory space peak: %.2f MB\n",
           MEGABYTES(getspacepeak()));
    checkspaceleak();
  }
#endif
  mmcheckspaceleak();
  return EXIT_SUCCESS;
}
