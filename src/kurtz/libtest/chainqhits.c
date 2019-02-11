#include <string.h>
#include "types.h"
#include "debugdef.h"
#include "arraydef.h"
#include "genfile.h"
#include "virtualdef.h"
#include "args.h"
#include "megabytes.h"
#include "onflychaindef.h"

#include "readvirt.pr"
#include "multiseq.pr"
#include "multiseq-adv.pr"

#include "produceqhits.pr"
#include "onflychain.pr"

#define ARGLIST "[checkqhit|nocheckqhit|checkleast|nocheckleast]"

typedef struct
{
  BOOL onlyqhits;
  Uint edistvalue;
  Maintainedfragments maintainedfragments;
  ArrayOnflyfragmentptr onflyoutputstore;
} Chainstate;

static Sint storeqhit(void *info,Uint length,Uint ipos,Uint jpos)
{
  ArrayOnflyfragment *qhitstorage = (ArrayOnflyfragment *) info;
  Onflyfragment *qhitptr;

  GETNEXTFREEINARRAY(qhitptr,qhitstorage,Onflyfragment,1024);
  qhitptr->fraglength = length;
  qhitptr->startpos[0] = ipos;
  qhitptr->startpos[1] = jpos;
#ifdef DEBUG
  qhitptr->identity = (Uint) (qhitptr - qhitstorage->spaceOnflyfragment);
#endif
  return 0;
}

static Sint maximalchainout(Onflyfragment *fragptr,void *info)
{
  Chainstate *chainstate = (Chainstate *) info;
  Onflyfragment *ptr;
  Onflyfragmentptr *storeptr;

  printf("chain %lu->%lu: score=%ld,length=%lu: ",
         (Showuint) fragptr->firstinchain->identity,
         (Showuint) fragptr->identity,
         (Showsint) fragptr->score,
         (Showuint) fragptr->lengthofchain);
  for(ptr = fragptr; ptr != UNDEFPREVIOUS; ptr = ptr->previousinchain)
  {
    GETNEXTFREEINARRAY(storeptr,
                       &chainstate->onflyoutputstore,Onflyfragmentptr,32);
    *storeptr = ptr;
  }
  for(storeptr = chainstate->onflyoutputstore.spaceOnflyfragmentptr +
                 chainstate->onflyoutputstore.nextfreeOnflyfragmentptr -1; 
      storeptr >= chainstate->onflyoutputstore.spaceOnflyfragmentptr;
      storeptr--)
  {
    ptr = *storeptr;
    if(chainstate->onlyqhits)
    {
      printf("[%lu,%lu]",
             (Showuint) ptr->startpos[0],
             (Showuint) ptr->startpos[1]);
    } else
    {
      printf("[[%lu..%lu],[%lu..%lu]]",
             (Showuint) ptr->startpos[0],
             (Showuint) (ptr->startpos[0] + ptr->fraglength - 1),
             (Showuint) ptr->startpos[1],
             (Showuint) (ptr->startpos[1] + ptr->fraglength - 1));
    }
  }
  printf("\n");
  chainstate->onflyoutputstore.nextfreeOnflyfragmentptr = 0;
  return 0;
}

static Sint flychain(void *info,Uint length,Uint ipos,Uint jpos)
{
  Chainstate *chainstate = (Chainstate *) info;

  if(processnewquhit(chainstate->onlyqhits,
                     chainstate->edistvalue,
                     &chainstate->maintainedfragments,
                     length,
                     ipos,
                     jpos,
                     maximalchainout,
                     chainstate) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

static void showonstdout(char *s)
{
  printf("# %s\n",s);
}

MAINFUNCTION
{
  Virtualtree virtualtree;
  Multiseq querymultiseq;
  const char *indexname, *queryfile;
  Sint retcode;
  ArrayOnflyfragment fragmentstore;
  Scaninteger readint;
  BOOL withcheck;
  Chainstate chainstate;
  Uint fixedmatchlength;
#ifndef NOSPACEBOOKKEEPING
  Uint peak, mmpeak;
#endif

  CHECKARGNUM(6,"fixedmatchlength edistvalue indexname queryfile " ARGLIST);
  DEBUGLEVELSET;

  if(sscanf(argv[1],"%ld",&readint) != 1 || readint <= 0)
  {
    fprintf(stderr,"%s: illegal first argument \"%s\"\n",argv[0],argv[1]);
    return EXIT_FAILURE;
  }
  fixedmatchlength = (Uint) readint;
  if(sscanf(argv[2],"%ld",&readint) != 1 || readint <= 0)
  {
    fprintf(stderr,"%s: illegal second argument \"%s\"\n",argv[0],argv[1]);
    return EXIT_FAILURE;
  }
  chainstate.edistvalue = (Uint) readint;
  indexname = argv[3];
  queryfile = argv[4];
  if(strcmp(argv[5],"checkqhit") == 0)
  {
    withcheck = True;
    chainstate.onlyqhits = True;
  } else
  {
    if(strcmp(argv[5],"nocheckqhit") == 0)
    {
      withcheck = False;
      chainstate.onlyqhits = True;
    } else
    {
      if(strcmp(argv[5],"checkleast") == 0)
      {
        withcheck = True;
        chainstate.onlyqhits = False;
      } else
      {
        if(strcmp(argv[5],"nocheckleast") == 0)
        {
          withcheck = False;
          chainstate.onlyqhits = False;
        } else
        {
          fprintf(stderr,"%s: last argument must be: %s\n",argv[0],ARGLIST);
          exit(EXIT_FAILURE);
        }
      }
    }
  }
  if(mapvirtualtreeifyoucan(&virtualtree,indexname,
                            TISTAB | BCKTAB | SUFTAB) != 0)
  {
    STANDARDMESSAGE;
  }
  if(fixedmatchlength < virtualtree.prefixlength)
  {
    fprintf(stderr,"%s: fixedmatchlength = %lu must be >= "
                   "prefixlength = %lu\n",
                   argv[0],
                   (Showuint) fixedmatchlength,
                   (Showuint) virtualtree.prefixlength);
    exit(EXIT_FAILURE);
  }
  if(showvirtualtreestatus(&virtualtree,indexname,showonstdout) != 0)
  {
    STANDARDMESSAGE;
  }
  retcode = mapandparsesequencefile(&virtualtree.alpha,
                                    &querymultiseq,
                                    queryfile);
  if(retcode < 0)
  {
    STANDARDMESSAGE;
  }
  if(withcheck)
  {
    INITARRAY(&fragmentstore,Onflyfragment);
  } else
  {
    initmaintainedfragments(&chainstate.maintainedfragments,False);
  }
  INITARRAY(&chainstate.onflyoutputstore,Onflyfragmentptr);
  if(produceqhits(virtualtree.multiseq.sequence,
                  virtualtree.multiseq.totallength,
                  virtualtree.bcktab,
                  virtualtree.suftab,
                  querymultiseq.sequence,
                  querymultiseq.totallength,
                  virtualtree.alpha.mapsize-1,
                  virtualtree.prefixlength,
                  chainstate.onlyqhits,
                  fixedmatchlength,
                  NULL,
                  withcheck ? storeqhit : flychain,
                  withcheck ? (void *) &fragmentstore
                            : (void *) &chainstate) != 0)
  {
    STANDARDMESSAGE;
  }
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
  if(withcheck)
  {
    if(chainingofmatches(chainstate.onlyqhits,
                         chainstate.edistvalue,
                         fragmentstore.spaceOnflyfragment,
                         fragmentstore.nextfreeOnflyfragment) != 0)
    {
      STANDARDMESSAGE;
    }
    FREEARRAY(&fragmentstore,Onflyfragment);
  } else
  {
    if(wrapmaintainedfragments(&chainstate.maintainedfragments,
                               maximalchainout,
                               &chainstate) != 0)
    {
      return (Sint) -2;
    }
  }
  FREEARRAY(&chainstate.onflyoutputstore,Onflyfragmentptr);
  freemultiseq(&querymultiseq);
#ifndef NOSPACEBOOKKEEPING
  printf("# overall space peak: ");
  peak = getspacepeak();
  printf("main: %.2f MB (rate=%.2f), ",MEGABYTES(peak),
                            (double) peak/virtualtree.multiseq.totallength);
  checkspaceleak();
  mmpeak = mmgetspacepeak();
  printf("secondary: %.2f MB (rate=%.2f)\n",MEGABYTES(mmpeak),
                            (double) peak/virtualtree.multiseq.totallength);
#endif
  mmcheckspaceleak();
  return EXIT_SUCCESS;
}
