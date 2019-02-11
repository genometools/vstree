
//\Ignore{

/*
  The following additional features may be implemented:

  -d x:  specify chain depth (min number of fragments in a chain) with 
         value x (integer).   EASY
  -pc x: percentage coverage of the EST (0<x<1). Ask Mohamed
         Ask which model of gap costs he implemented.

  Ideas for improving the running time:
  use special redblacktree functions with inlined compare function
  cmpFragpoint2.

  Use fact that elements are sorted with respect to the start points
  of one dimension. Then they are almost sorted w.r.t to end points.
  To compute the sorting w.r.t. end points, we use insertion sort.
  Hence we can stream both sorted lists and perform the 
  steps of the algorithm as we would merge the two lists.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "arraydef.h"
#include "debugdef.h"
#include "errordef.h"
#include "minmax.h"
#include "chaindef.h"

//}

#define UNDEFPREVIOUS           numofmatches

#define GETSTOREDSTARTPOINT(DIM,IDX)\
        fragmentinfo[IDX].startpos[DIM]
#define GETSTOREDENDPOINT(DIM,IDX)\
        fragmentinfo[IDX].endpos[DIM]

#define INITIALGAP(IDX)\
        fragmentinfo[IDX].initialgap
#define TERMINALGAP(IDX)\
        fragmentinfo[IDX].terminalgap

#define CHECKCHAINSPACE\
        if(lengthofchain >= chain->chainedfragments.allocatedUint)\
        {\
          ALLOCASSIGNSPACE(chain->chainedfragments.spaceUint,\
                           chain->chainedfragments.spaceUint,\
                           Uint,\
                           lengthofchain);\
          chain->chainedfragments.allocatedUint = lengthofchain;\
          chain->chainedfragments.nextfreeUint = 0;\
        }

typedef Chainscoretype (*Chaingapcostfunction)(Fragmentinfo *,Uint,Uint);

typedef struct
{
  Chainscoretype maxscore;
  Uint maxfragnum;
  BOOL defined;
} Maxfragvalue;

static Chainscoretype gapcostL1(Fragmentinfo *fragmentinfo,Uint i,Uint j)
{
  return (Chainscoretype) 
         ((GETSTOREDSTARTPOINT(0,j) - GETSTOREDENDPOINT(0,i)) +
          (GETSTOREDSTARTPOINT(1,j) - GETSTOREDENDPOINT(1,i)));
}

static BOOL colinearfragments(Fragmentinfo *fragmentinfo,Uint i,Uint j)
{
  if(GETSTOREDSTARTPOINT(0, i) < GETSTOREDSTARTPOINT(0, j) &&
     GETSTOREDENDPOINT(0, i)   < GETSTOREDENDPOINT(0, j)   &&
     GETSTOREDSTARTPOINT(1, i) < GETSTOREDSTARTPOINT(1, j) &&
     GETSTOREDENDPOINT(1, i)   < GETSTOREDENDPOINT(1, j))
  {
    return True;
  }
  return False;
}

static void chainingboundarycases(Chainmode *chainmode,
                                  Chain *chain,
                                  Fragmentinfo *fragmentinfo,
                                  Uint numofmatches)
{
  if(numofmatches == 0)
  {
    chain->scoreofchain = 0;
    chain->chainedfragments.nextfreeUint = 0;
  } else
  {
    if(numofmatches == UintConst(1))
    {
      Uint lengthofchain;

      chain->scoreofchain = fragmentinfo[0].weight;
      if(chainmode->chainkind == GLOBALCHAININGWITHGAPCOST)
      {
        chain->scoreofchain -= (INITIALGAP(0) + TERMINALGAP(0));
      }
      lengthofchain = UintConst(1);
      CHECKCHAINSPACE;
      chain->chainedfragments.spaceUint[0] = 0;
      chain->chainedfragments.nextfreeUint = UintConst(1);
    }
  }
}

static BOOL checkmaxgapwidth(Fragmentinfo *fragmentinfo,
                             Uint maxgapwidth,
                             Uint leftfrag,
                             Uint rightfrag)
{
  Uint gapwidth, startpoint, endpoint;

  startpoint = GETSTOREDSTARTPOINT(0,rightfrag);
  endpoint = GETSTOREDENDPOINT(0,leftfrag);
  if(startpoint < endpoint)
  {
    gapwidth = 0;
  } else
  {
    gapwidth = startpoint - endpoint - 1;
  }
  if(gapwidth > maxgapwidth)
  {
    return False;
  } 
  startpoint = GETSTOREDSTARTPOINT(1,rightfrag);
  endpoint = GETSTOREDENDPOINT(1,leftfrag);
  if(startpoint < endpoint)
  {
    gapwidth = 0;
  } else
  {
    gapwidth = startpoint - endpoint - 1;
  }
  if(gapwidth > maxgapwidth)
  {
    return False;
  }
  return True;
}

static void bruteforcechainingscores(Chainmode *chainmode,
                                     Fragmentinfo *fragmentinfo,
                                     Uint *maxchaincounttab,
                                     Uint numofmatches,
                                     Chaingapcostfunction chaingapcostfunction)
{
  Uint previous, leftfrag, rightfrag, maxchaincount;
  Chainscoretype weightright, score;
  Maxfragvalue localmaxfrag;
  BOOL combinable;

  if(numofmatches > UintConst(1))
  {
    fragmentinfo[0].firstinchain = 0;
    fragmentinfo[0].previousinchain = UNDEFPREVIOUS;
    fragmentinfo[0].score = fragmentinfo[0].weight;
    maxchaincounttab[0] = UintConst(1);
    if(chainmode->chainkind == GLOBALCHAININGWITHGAPCOST)
    {
      fragmentinfo[0].score -= (INITIALGAP(0) + TERMINALGAP(0));
    }
    for(rightfrag=UintConst(1); rightfrag<numofmatches; rightfrag++)
    {
      weightright = fragmentinfo[rightfrag].weight;
      localmaxfrag.defined = False;
      localmaxfrag.maxscore = 0;
      localmaxfrag.maxfragnum = 0;
      maxchaincount = 0;
      for(leftfrag=0; leftfrag<rightfrag; leftfrag++)
      {
        if(chainmode->maxgapwidth != 0 && 
           !checkmaxgapwidth(fragmentinfo,
                             chainmode->maxgapwidth,
                             leftfrag,
                             rightfrag))
        {
          combinable = False;
        } else
        {
          if(chainmode->chainkind == GLOBALCHAININGWITHOVERLAPS)
          {
            combinable = colinearfragments(fragmentinfo,leftfrag,rightfrag); 
          } else
          {
            combinable = True;
          }
        }
        if(combinable)
        {
          score = fragmentinfo[leftfrag].score;
          if(chainmode->chainkind == GLOBALCHAINING)
          {
            // process chainkinds without gap costs
            score += weightright;
            previous = leftfrag;
          } else
          {
            score -= chaingapcostfunction(fragmentinfo,leftfrag,rightfrag);
            if(chainmode->chainkind == GLOBALCHAININGWITHGAPCOST)
            {
              score += (weightright + TERMINALGAP(leftfrag)
                                    - TERMINALGAP(rightfrag));
              previous = leftfrag;
            } else
            {
              if(score > 0)
              {
                score += weightright;
                previous = leftfrag;
              } else
              {
                score = weightright;
                previous = UNDEFPREVIOUS;
              }
            } 
          }
          if(!localmaxfrag.defined)
          {
            localmaxfrag.maxscore = score;
            localmaxfrag.maxfragnum = previous;
            localmaxfrag.defined = True;
            maxchaincount = UintConst(1);
          } else
          {
            if(localmaxfrag.maxscore < score)
            {
              localmaxfrag.maxscore = score;
              localmaxfrag.maxfragnum = previous;
              maxchaincount = UintConst(1);
            } else
            {
              if(localmaxfrag.maxscore == score)
              {
                if(previous == UNDEFPREVIOUS)
                {
                  maxchaincount = UintConst(1);
                } else
                {
                  maxchaincount += maxchaincounttab[previous];
                }
              }
            }
          }
        }
      }
      if(localmaxfrag.defined)
      {
        maxchaincounttab[rightfrag] = maxchaincount;
        fragmentinfo[rightfrag].previousinchain = localmaxfrag.maxfragnum;
        if(localmaxfrag.maxfragnum == UNDEFPREVIOUS)
        {
          fragmentinfo[rightfrag].firstinchain = rightfrag;
        } else
        {
          fragmentinfo[rightfrag].firstinchain 
            = fragmentinfo[localmaxfrag.maxfragnum].firstinchain;
        }
        fragmentinfo[rightfrag].score = localmaxfrag.maxscore;
      } else
      {
        maxchaincounttab[rightfrag] = UintConst(1);
        fragmentinfo[rightfrag].previousinchain = UNDEFPREVIOUS;
        fragmentinfo[rightfrag].firstinchain = rightfrag;
        fragmentinfo[rightfrag].score = weightright;
        if(chainmode->chainkind == GLOBALCHAININGWITHGAPCOST)
        {
          fragmentinfo[rightfrag].score 
            -= (INITIALGAP(rightfrag) + TERMINALGAP(rightfrag));
        }
      }
    }
  }
}

Sint bruteforchainingwithcounts(Chainmode *chainmode,
                                Chain *chain,
                                Fragmentinfo *fragmentinfo,
                                Uint numofmatches,
                                Uint presortdim,
                                Showverbose showverbose)
{
  Chaingapcostfunction chaingapcostfunction;

  if(presortdim > UintConst(1))
  {
    ERROR0("fastchaining: presortdim must be 0 or 1");
    return (Sint) -1;
  }
  chaingapcostfunction = gapcostL1;
  if(numofmatches > UintConst(1))
  {
    Uint *maxchaincounttab;

    ALLOCASSIGNSPACE(maxchaincounttab,NULL,Uint,numofmatches);
    if(showverbose != NULL)
    {
      showverbose("compute chain scores");
    }
    bruteforcechainingscores(chainmode,
                             fragmentinfo,
                             maxchaincounttab,
                             numofmatches,
                             chaingapcostfunction);
    if(showverbose != NULL)
    {
      showverbose("retrieve optimal chains");
    }
    /* Now retrieve the information you need by iterating over
       all elements in fragmentinfo. Do this in analogy to  the
       function findmaximalscores as implemented in chain2dim.
       The number of chains with maximal score ending in 
       fragment i can be found in maxchaincounttab[i].
    */
    FREESPACE(maxchaincounttab);
  } else
  {
    chainingboundarycases(chainmode,
                          chain,
                          fragmentinfo,
                          numofmatches);
  }
  return 0;
}
