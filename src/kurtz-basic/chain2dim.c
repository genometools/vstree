
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
#include "intbits.h"
#include "arraydef.h"
#include "redblackdef.h"
#include "divmodmul.h"
#include "debugdef.h"
#include "errordef.h"
#include "minmax.h"
#include "chaindef.h"

#include "redblack.pr"
#include "dictmaxsize.pr"

//}

#ifdef DEBUG
#define ENDPOINTBIT             FIRSTBIT
#define MAKEENDPOINT(FID)       ((FID) | ENDPOINTBIT)
#define ISENDPOINT(FRAG)        (((FRAG)->fpident) & ENDPOINTBIT)
#define FRAGIDENT(FRAG)         (((FRAG)->fpident) & EXCEPTFIRSTBIT)
#else
#define MAKEENDPOINT(FID)       (FID)
#define FRAGIDENT(FRAG)         ((FRAG)->fpident)
#endif

#define UNDEFPREVIOUS           numofmatches

#define GETSTOREDSTARTPOINT(DIM,IDX)\
        fragmentinfo[IDX].startpos[DIM]
#define GETSTOREDENDPOINT(DIM,IDX)\
        fragmentinfo[IDX].endpos[DIM]

#ifdef DEBUG
#define GETSTOREDLENGTH(DIM,IDX)\
        (GETSTOREDENDPOINT(DIM,IDX) - GETSTOREDSTARTPOINT(DIM,IDX))
#endif

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

#ifdef DEBUG
#define CHECKEXPRESSION(EXPR)\
        if(EXPR)\
        {\
          fprintf(stderr,"error %s not supposed\n",#EXPR);\
          exit(EXIT_FAILURE);\
        }
#else
#define CHECKEXPRESSION(EXPR) /* Nothing */
#endif

typedef Chainscoretype (*Chaingapcostfunction)(Fragmentinfo *,Uint,Uint);

typedef struct
{
  Uint fpident,
       fpposition;
} Fragpoint;

#ifdef DEBUG
DECLAREARRAYSTRUCT(Fragpoint);
#endif

typedef struct
{
  void *dictroot;
  Uint *endpointperm;
#ifdef DEBUG
  ArrayFragpoint xfragpoints;
  Uint maxnumofnodes,
       currentnumofnodes;
#endif
} Fragmentstore;

typedef struct
{
  Chainscoretype maxscore;
  Uint maxfragnum;
  BOOL defined;
} Maxfragvalue;

/*
  The component isavailable is used to
  (1) indicate that some score is already stored (when generating the classes)
  (2) indicate that the class representative has not yet been processed
      further (after generation)
*/

typedef struct
{
  BOOL isavailable;
  Sint score;
} Bestofclass;

static BOOL overlappingfragments(Fragmentinfo *fragmentinfo,Uint i,Uint j)
{
  if(GETSTOREDENDPOINT(0,i) >= GETSTOREDSTARTPOINT(0,j))
  {
    return True;
  }
  if(GETSTOREDENDPOINT(1,i) >= GETSTOREDSTARTPOINT(1,j))
  {
    return True;
  }
  return False;
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

#ifdef DEBUG

static BOOL smallerfragments(Fragmentinfo *fragmentinfo,Uint i,Uint j)
{
  if(GETSTOREDENDPOINT(0,i) < GETSTOREDSTARTPOINT(0,j) &&
     GETSTOREDENDPOINT(1,i) < GETSTOREDSTARTPOINT(1,j))
  {
    return True;
  }
  return False;
}

#endif

static Chainscoretype gapcostL1(Fragmentinfo *fragmentinfo,Uint i,Uint j)
{
#ifdef DEBUG
  if(overlappingfragments(fragmentinfo,i,j))
  {
    fprintf(stderr,"gapcostL1 only defined for non overlapping fragments\n");
    exit(EXIT_FAILURE);
  }
  if(!smallerfragments(fragmentinfo,i,j))
  {
    fprintf(stderr,"gapcostL1 only defined for fragment f1 < f2\n");
    exit(EXIT_FAILURE);
  }
#endif
  return (Chainscoretype) 
         ((GETSTOREDSTARTPOINT(0,j) - GETSTOREDENDPOINT(0,i)) +
          (GETSTOREDSTARTPOINT(1,j) - GETSTOREDENDPOINT(1,i)));
}

static Chainscoretype overlapcost(Fragmentinfo *fragmentinfo,Uint i,Uint j)
{
  Uint overlaplength = 0;

#ifdef DEBUG
  if(!colinearfragments(fragmentinfo,i,j))
  {
    fprintf(stderr,"overlapcost only defined for colinear fragments\n");
    exit(EXIT_FAILURE);
  }
#endif

  // add overlap in first dimension
  if(GETSTOREDSTARTPOINT(0, j) <= GETSTOREDENDPOINT(0, i))
  {
    overlaplength += GETSTOREDENDPOINT(0, i) - GETSTOREDSTARTPOINT(0, j) + 1; 
  }

  // add overlap in second dimension
  if(GETSTOREDSTARTPOINT(1, j) <= GETSTOREDENDPOINT(1, i))
  {
    overlaplength += GETSTOREDENDPOINT(1, i) - GETSTOREDSTARTPOINT(1, j) + 1; 
  }

  return (Chainscoretype) overlaplength;
}

/*
  The Chebychev distance for two qhits h=(i,j) and h'=(i',j')
  is defined as:

                      max{|i'-i-q|,|j'-j-q|},

  whereas i+q-1 is the end point in i-dimension of fragment 1 and
  j+q-1 is the end point in j-dimension of fragment 1.
  In using the match specific end points, other than fixed values for q,
  following function generalizes to MEMs as well.
*/

static Chainscoretype gapcostCc(Fragmentinfo *fragmentinfo,Uint i,Uint j)
{
  Uint value1, value2;

#ifdef DEBUG
  if(GETSTOREDENDPOINT(0,i) >= GETSTOREDSTARTPOINT(0,j) || 
     GETSTOREDENDPOINT(1,i) >= GETSTOREDSTARTPOINT(1,j))
  {
    fprintf(stderr, "error: Chebychev distance only defined for two matches) "
                    "(l,i,j) and (l',i',j') iff\n"
                    "i' > i+l-1 && j' > j+l-1");
    exit(EXIT_FAILURE);
  }
#endif

  value1 = GETSTOREDSTARTPOINT(0,j) - GETSTOREDENDPOINT(0,i) - 1,
  value2 = GETSTOREDSTARTPOINT(1,j) - GETSTOREDENDPOINT(1,i) - 1;
  return (Chainscoretype) MAX(value1,value2);
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

static void retracepreviousinchain(Chain *chain,
                                   Fragmentinfo *fragmentinfo,
                                   Uint numofmatches,
                                   Uint retracestart)
{
  Uint fragnum, idx, lengthofchain;

  for(lengthofchain = 0, fragnum = retracestart; 
      fragnum != UNDEFPREVIOUS; lengthofchain++)
  {
    fragnum = fragmentinfo[fragnum].previousinchain;
  }
  CHECKCHAINSPACE
  fragnum = retracestart;
  idx = lengthofchain;
  while(fragnum != UNDEFPREVIOUS)
  {
    CHECKEXPRESSION(idx == 0);
    idx--;
    chain->chainedfragments.spaceUint[idx] = fragnum;
    fragnum = fragmentinfo[fragnum].previousinchain;
  }
  CHECKEXPRESSION(idx != 0);
  chain->chainedfragments.nextfreeUint = lengthofchain;
}

#ifdef DEBUG

static void derivefragpoints(ArrayFragpoint *xfragpoints,
                             Fragmentinfo *fragmentinfo,
                             Uint numofmatches,
                             Uint presortdim)
{
  Uint i;
  Fragpoint *xfpptr;

  xfragpoints->nextfreeFragpoint = xfragpoints->allocatedFragpoint 
                                 = MULT2(numofmatches);
  ALLOCASSIGNSPACE(xfragpoints->spaceFragpoint,NULL,
                   Fragpoint,xfragpoints->allocatedFragpoint);
  xfpptr = xfragpoints->spaceFragpoint;
  for(i=0; i<numofmatches; i++)
  {
    xfpptr->fpident = i;
    xfpptr->fpposition = GETSTOREDSTARTPOINT(presortdim,i);
    xfpptr++;
    xfpptr->fpident = MAKEENDPOINT(i);
    xfpptr->fpposition = GETSTOREDENDPOINT(presortdim,i);
    xfpptr++;
  }
}

/*
  The following function is for the initial fragment
  sorting phase. It compares fragment points (start or endpoints)
  in dimension 1. If a start and an endpoint are identical, 
  then the start point comes before the end point. If two start points
  are identical, then the order is undefined. If two end points 
  are identical, then the order is undefined.
*/

static Qsortcomparereturntype qsortcmpFragpoint (const void *keya,
                                                 const void *keyb)
{
  if(((const Fragpoint *) keya)->fpposition < ((const Fragpoint *) keyb)->fpposition)
  {
    return (Qsortcomparereturntype) -1;
  }
  if(((const Fragpoint *) keya)->fpposition > ((const Fragpoint *) keyb)->fpposition)
  {
    return (Qsortcomparereturntype) 1;
  }
  if(ISENDPOINT((const Fragpoint *) keya))
  {
    if(!ISENDPOINT((const Fragpoint *) keyb))
    {
      return (Qsortcomparereturntype) 1;
    }
  } else
  {
    if(ISENDPOINT((const Fragpoint *) keyb))
    {
      return (Qsortcomparereturntype) -1;
    }
  }
  return 0;
}

static void sortfragpoints(Fragpoint *fragpoints,
                           Uint numfragpoints)
{
  qsort((void *) fragpoints,
        (size_t) numfragpoints,
        sizeof(Fragpoint),
        qsortcmpFragpoint);
}

static Chainscoretype sumofgapcosts(Fragmentinfo *fragmentinfo,
                                    Chain *chain,
                                    Chaingapcostfunction chaingapcostfunction)
{
  Chainscoretype score = 0;
  Uint i;

  for(i=UintConst(1); i<chain->chainedfragments.nextfreeUint; i++)
  {
    score += chaingapcostfunction(fragmentinfo,
                                  chain->chainedfragments.spaceUint[i-1],
                                  chain->chainedfragments.spaceUint[i]);
  }
  return score;
}

static void showthechain(Chainmode *chainmode,
                         Chain *chain,
                         Fragmentinfo *fragmentinfo,
                         Chaingapcostfunction chaingapcostfunction)
{
  Uint i, start, len, currentfrag, previousfrag;

  printf("# lengthofchain=%lu\n",
          (Showuint) chain->chainedfragments.nextfreeUint);
  printf("# score=%ld\n",(Showsint) chain->scoreofchain);
  for(i=0; i<chain->chainedfragments.nextfreeUint; i++)
  {
    currentfrag = chain->chainedfragments.spaceUint[i];
    printf("%lu: id=%lu weight=%ld ",
             (Showuint) i,
             (Showuint) currentfrag,
             (Showsint) fragmentinfo[currentfrag].weight);
    len = GETSTOREDLENGTH(1,currentfrag);
    start = GETSTOREDSTARTPOINT(0,currentfrag);
    printf("%lu (%lu,%lu) ",(Showuint) len,
                            (Showuint) start,
                            (Showuint) (start+len-1));
    start = GETSTOREDSTARTPOINT(1,currentfrag);
    printf("(%lu,%lu)",(Showuint) start,(Showuint) (start+len-1));
    if(chainmode->chainkind != GLOBALCHAINING && 
       chainmode->chainkind != GLOBALCHAININGWITHOVERLAPS)
    {
      if(i == 0)
      {
        if(chainmode->chainkind == GLOBALCHAININGWITHGAPCOST)
        {
          printf(" initalgapcost=%ld", (Showsint) - INITIALGAP(currentfrag));
        }
      } else
      {
        previousfrag = chain->chainedfragments.spaceUint[i-1];
        printf(" gapcost(prev,curr)=%ld",
                (Showsint) chaingapcostfunction(fragmentinfo,
                                                previousfrag,
                                                currentfrag));
      }
    }
    printf("\n");
  }
  if((chainmode->chainkind == GLOBALCHAININGWITHGAPCOST) &&
     chain->chainedfragments.nextfreeUint > 0)
  {
    currentfrag = chain->chainedfragments.
                  spaceUint[chain->chainedfragments.nextfreeUint-1];
    printf("endgapcost=%ld\n",(Showsint) TERMINALGAP(currentfrag));
  }
}

static void showsinglefragpoint(Fragpoint *fpptr)
{
  if(ISENDPOINT(fpptr))
  {
    printf("E ");
  } else
  {
    printf("S ");
  }
  printf("%lu %lu\n",(Showuint) FRAGIDENT(fpptr),
                     (Showuint) fpptr->fpposition);
}

static void showxfragpoints(Fragpoint *fragpoints,
                            Uint numfragpoints)
{
  Fragpoint *fpptr;

  for(fpptr=fragpoints; fpptr < fragpoints + numfragpoints; fpptr++)
  {
    showsinglefragpoint(fpptr);
  }
}

static void checklocalchainconditions(Fragmentinfo *fragmentinfo,
                                      Uint numofmatches,
                                      Chaingapcostfunction chaingapcostfunction,
                                      Chain *chain)
{
  Uint i, firstfrag;
  Chainscoretype sumscore, wgt, gapscore;
 
  if(chain->chainedfragments.nextfreeUint > 0)
  {
    firstfrag = chain->chainedfragments.spaceUint[0];
    for(i=0; i<numofmatches; i++)
    {
      if(smallerfragments(fragmentinfo,i,firstfrag))
      {
        wgt = fragmentinfo[i].weight;
        gapscore = chaingapcostfunction(fragmentinfo,i,firstfrag);
        if(wgt > gapscore)
        {
          fprintf(stderr,"frag %lu/%lu: previous weight %ld>%ld = gapscore\n",
                (Showuint) i,
                (Showuint) firstfrag,
                (Showsint) wgt,
                (Showsint) gapscore);
          exit(EXIT_FAILURE);
        }
      }
    }
    sumscore = fragmentinfo[chain->chainedfragments.spaceUint[0]].weight;
    for(i=UintConst(1); i<chain->chainedfragments.nextfreeUint; i++)
    {
      sumscore += fragmentinfo[chain->chainedfragments.spaceUint[i]].weight;
      gapscore = chaingapcostfunction(fragmentinfo,
                                      chain->chainedfragments.spaceUint[i-1],
                           chain->chainedfragments.spaceUint[i]);
      if(sumscore <= gapscore)
      {
        fprintf(stderr,"frag %lu/%lu: previous score %ld<=%ld = gapscore\n",
                (Showuint) chain->chainedfragments.spaceUint[i-1],
                (Showuint) chain->chainedfragments.spaceUint[i],
                (Showsint) sumscore,
                (Showsint) gapscore);
        exit(EXIT_FAILURE);
      }
      sumscore -= gapscore;
    }
  }
}

static void checkthechain(Chainmode *chainmode,
                          Fragmentinfo *fragmentinfo,
                          Uint numofmatches,
                          Chain *chain,
                          Chaingapcostfunction chaingapcostfunction)
{
  Chainscoretype score;
  Uint i;

  if(chain->chainedfragments.nextfreeUint == 0)
  {
    score = 0;
  } else
  {
    score = fragmentinfo[chain->chainedfragments.spaceUint[0]].weight;
    DEBUG1(3,"init with weight(0)=%ld\n",(Showsint) score);
    if(chainmode->chainkind == GLOBALCHAININGWITHGAPCOST)
    {
      score -= INITIALGAP(chain->chainedfragments.spaceUint[0]);
      DEBUG2(3,"subtract initialgap %ld => score=%ld\n"
            , (Showsint) INITIALGAP(chain->chainedfragments.spaceUint[0])
            , (Showsint) score);
    }
    for(i=UintConst(1); i<chain->chainedfragments.nextfreeUint; i++)
    {
      if(overlappingfragments(fragmentinfo,
                              chain->chainedfragments.spaceUint[i-1],
                              chain->chainedfragments.spaceUint[i]))
      {
        fprintf(stderr,"fragments %lu and %lu overlap\n",
                (Showuint) i,
                (Showuint) (i-1));
        exit(EXIT_FAILURE);
      }
      score += fragmentinfo[chain->chainedfragments.spaceUint[i]].weight;
      DEBUG3(3,"add weight[%lu] = %ld => score=%ld\n", (Showuint) i,
              (Showsint) fragmentinfo[chain->chainedfragments.
                                             spaceUint[i]].weight,
              (Showsint) score);
    }
    if(chainmode->chainkind == GLOBALCHAININGWITHGAPCOST)
    {
      score -= TERMINALGAP(chain->chainedfragments.
                           spaceUint[chain->chainedfragments.nextfreeUint-1]);
      DEBUG2(3,"subtract terminal gap = %ld => %ld\n",
             (Showsint) TERMINALGAP(chain->chainedfragments.
                         spaceUint[chain->chainedfragments.nextfreeUint-1]),
             (Showsint) score);
    }
  }
  if(chainmode->chainkind != GLOBALCHAINING)
  {
    score -= sumofgapcosts(fragmentinfo,chain,chaingapcostfunction);
    DEBUG2(3,"subtract intergaps %ld = %ld\n"
          , (Showsint) sumofgapcosts(fragmentinfo,chain,chaingapcostfunction),
            (Showsint) score);
  }
  if(score != chain->scoreofchain)
  {
    fprintf(stderr,"score = %ld != %ld = scoreofchain\n",
            (Showsint) score,(Showsint) chain->scoreofchain);
    exit(EXIT_FAILURE);
  }
  if(chainmode->chainkind == LOCALCHAININGMAX ||
     chainmode->chainkind == LOCALCHAININGTHRESHOLD ||
     chainmode->chainkind == LOCALCHAININGBEST ||
     chainmode->chainkind == LOCALCHAININGPERCENTAWAY)
  {
    checklocalchainconditions(fragmentinfo,numofmatches,
                              chaingapcostfunction,
                              chain);
  }
}

static Fragmentinfo *copyfragmentinfo(Fragmentinfo *fragmentinfo,
                                      Uint numofmatches)
{
  Fragmentinfo *finfo;
  Uint i;

  ALLOCASSIGNSPACE(finfo,NULL,Fragmentinfo,numofmatches);
  for(i=0; i<numofmatches; i++)
  {
    finfo[i] = fragmentinfo[i];
  }
  return finfo;
}

static void checkallFragmentinfos(BOOL addterminal,
                                  Fragmentinfo *fragmentinfo1,
                                  Fragmentinfo *fragmentinfo2,
                                  Uint numofmatches)
{
  Uint i;
  Chainscoretype tgap;

  for(i=0; i<numofmatches; i++)
  {
    if(addterminal)
    {
      tgap = fragmentinfo1[i].terminalgap;
    } else
    {
      tgap = 0;
    }
    if(fragmentinfo1[i].score - tgap != fragmentinfo2[i].score)
    {
      fprintf(stderr,"%lu, tgap=%ld: score1 = %ld != %ld = score2\n",
                      (Showuint) i,
                      (Showsint) tgap,
                      (Showsint) fragmentinfo1[i].score - tgap,
                      (Showsint) fragmentinfo2[i].score);
      if(fragmentinfo1[i].previousinchain != fragmentinfo2[i].previousinchain)
      {
        Uint previous;
        fprintf(stderr,"%lu, previous = %ld != %ld = score2\n",
                        (Showuint) i,
                        (Showsint) fragmentinfo1[i].previousinchain,
                        (Showsint) fragmentinfo2[i].previousinchain);
        previous = fragmentinfo2[i].previousinchain;
        printf("match %lu is (%lu,%lu) vs (%lu,%lu)\n",
                 (Showuint) i,
                 (Showuint) fragmentinfo2[i].startpos[0],
                 (Showuint) fragmentinfo2[i].endpos[0],
                 (Showuint) fragmentinfo2[i].startpos[1],
                 (Showuint) fragmentinfo2[i].endpos[1]);
        printf("match %lu is (%lu,%lu) vs (%lu,%lu)\n",
                 (Showuint) previous,
                 (Showuint) fragmentinfo2[previous].startpos[0],
                 (Showuint) fragmentinfo2[previous].endpos[0],
                 (Showuint) fragmentinfo2[previous].startpos[1],
                 (Showuint) fragmentinfo2[previous].endpos[1]);
      }
      exit(EXIT_FAILURE);
    }
  }
  printf("checkallFragmentinfos okay\n");
}

typedef struct
{
  Fragpoint *previousfragment;
  Fragmentinfo *fragmentinfo;
  Uint postsortdim;
} Fragwalkinfo;

static Sint checkscoreorder (const Keytype key,
                            VISIT which,
                            /*@unused@ */ Uint depth,
                            void *info)
{
  if(which == postorder || which == leaf)
  {
    Uint prevfpid, currfpid;
    Fragwalkinfo *fragwalkinfo = (Fragwalkinfo *) info;
    Fragmentinfo *fragmentinfo;

    fragmentinfo = fragwalkinfo->fragmentinfo;
    currfpid = FRAGIDENT((Fragpoint *) key);
    printf("currfpid=%lu\n",(Showuint) currfpid);
    if(fragwalkinfo->previousfragment != NULL)
    {
      prevfpid = FRAGIDENT(fragwalkinfo->previousfragment);
      printf("previous(id=%lu)=((%lu,%lu),(%lu,%lu)) score=%ld ",
              (Showuint) prevfpid,
              (Showuint) GETSTOREDSTARTPOINT(0,prevfpid),
              (Showuint) GETSTOREDENDPOINT(0,prevfpid),
              (Showuint) GETSTOREDSTARTPOINT(1,prevfpid),
              (Showuint) GETSTOREDENDPOINT(1,prevfpid),
              (Showsint) fragmentinfo[prevfpid].score);
      printf("current(id=%lu)=((%lu,%lu),(%lu,%lu)) score=%ld\n",
              (Showuint) currfpid,
              (Showuint) GETSTOREDSTARTPOINT(0,currfpid),
              (Showuint) GETSTOREDENDPOINT(0,currfpid),
              (Showuint) GETSTOREDSTARTPOINT(1,currfpid),
              (Showuint) GETSTOREDENDPOINT(1,currfpid),
              (Showsint) fragmentinfo[currfpid].score);
      /* points are sorted by endpoint of postsort dimension */
      if(GETSTOREDENDPOINT(fragwalkinfo->postsortdim,prevfpid) > 
         GETSTOREDENDPOINT(fragwalkinfo->postsortdim,currfpid))
      {
        fprintf(stderr,"ENDPOINT(%lu,%lu) > ENDPOINT(%lu,%lu)\n",
                 (Showuint) fragwalkinfo->postsortdim,
                 (Showuint) prevfpid,
                 (Showuint) fragwalkinfo->postsortdim,
                 (Showuint) currfpid);
        exit(EXIT_FAILURE);
      }
      if(fragmentinfo[prevfpid].score > fragmentinfo[currfpid].score)
      {
        fprintf(stderr,"score[%lu] = %ld > %ld = score[%lu]\n",
                        (Showuint) prevfpid,
                        (Showsint) fragmentinfo[prevfpid].score,
                        (Showsint) fragmentinfo[currfpid].score,
                        (Showuint) currfpid);
        exit(EXIT_FAILURE);
      }
    }
    fragwalkinfo->previousfragment = ((Fragpoint *) key);
  }
  return 0;
}

static Sint redblacktreecheckscoreorder(Uint presortdim,
                                        void *root,
                                        Fragmentinfo *fragmentinfo)
{
  Fragwalkinfo fragwalkinfo;

  printf("run redblacktreecheckscoreorder\n");
  fragwalkinfo.previousfragment = NULL;
  fragwalkinfo.fragmentinfo = fragmentinfo;
  fragwalkinfo.postsortdim = UintConst(1) - presortdim;
  if(redblacktreewalk (root,checkscoreorder, (void *) &fragwalkinfo) != 0)
  {
    return (Sint) -1;
  }
  return 0;
}

#endif

static BOOL checkmaxgapwidth(Fragmentinfo *fragmentinfo,
                             Uint maxgapwidth,
                             Uint leftfrag,
                             Uint rightfrag)
{
  Uint gapwidth, startpoint, endpoint;

  startpoint = GETSTOREDSTARTPOINT(0,rightfrag);
  endpoint = GETSTOREDENDPOINT(0,leftfrag);
  if(startpoint <= endpoint)
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
  if(startpoint <= endpoint)
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
                                     Uint numofmatches,
                                     Chaingapcostfunction chaingapcostfunction)
{
  Uint previous, leftfrag, rightfrag;
  Chainscoretype weightright, score;
  Maxfragvalue localmaxfrag;
  BOOL combinable;

  if(numofmatches > UintConst(1))
  {
    fragmentinfo[0].firstinchain = 0;
    fragmentinfo[0].previousinchain = UNDEFPREVIOUS;
    fragmentinfo[0].score = fragmentinfo[0].weight;
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
            if(overlappingfragments(fragmentinfo,leftfrag,rightfrag))
            {
              combinable = False;
            } else
            {
              combinable = True;
            }
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
          if(!localmaxfrag.defined || localmaxfrag.maxscore < score)
          {
            localmaxfrag.maxscore = score;
            localmaxfrag.maxfragnum = previous;
            localmaxfrag.defined = True;
          } 
        }
      }
      if(localmaxfrag.defined)
      {
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


/*
  The following function compares fragment points. These are 
  end points of fragments in dimension 2.
  If the fragment points are identical, then the order is undefined.
*/

static Sint cmpendFragpoint2(const Keytype keya,
                             const Keytype keyb,
                             /*@unused@*/ void *info)
{
#ifdef DEBUG
  if(!ISENDPOINT((Fragpoint *) keya))
  {
    fprintf(stderr,"keya is not end point\n");
    exit(EXIT_FAILURE);
  }
  if(!ISENDPOINT((Fragpoint *) keyb))
  {
    fprintf(stderr,"keyb is not end point\n");
    exit(EXIT_FAILURE);
  }
#endif
  if(((Fragpoint *) keya)->fpposition < ((Fragpoint *) keyb)->fpposition)
  {
    return (Sint) -1;
  }
  if(((Fragpoint *) keya)->fpposition > ((Fragpoint *) keyb)->fpposition)
  {
    return (Sint) 1;
  }
  if(FRAGIDENT((Fragpoint *) keya) < FRAGIDENT((Fragpoint *) keyb))
  {
    return (Sint) -1;
  }
  if(FRAGIDENT((Fragpoint *) keya) > FRAGIDENT((Fragpoint *) keyb))
  {
    return (Sint) 1;
  }
  return (Sint) 0;
}

static Chainscoretype evalpriority(BOOL addterminal,
                                   Fragmentinfo *fragmentinfo,
                                   Uint fragnum)
{
  if(addterminal)
  {
    return fragmentinfo[fragnum].score - TERMINALGAP(fragnum);
  }
  return fragmentinfo[fragnum].score;
}

static void insertintodict(BOOL addterminal,
                           Fragmentinfo *fragmentinfo,
                           Fragmentstore *fragmentstore,
                           Fragpoint *qfrag2)
{
  Fragpoint *retval2;
  BOOL nodecreated;

  DEBUG2(2,"insert fragment %lu with endpoint %lu\n",
         (Showuint) FRAGIDENT(qfrag2),
         (Showuint) qfrag2->fpposition);
  retval2 = (Fragpoint *) redblacktreesearch ((const Keytype) qfrag2,
                                              &nodecreated,
                                              &fragmentstore->dictroot,
                                              cmpendFragpoint2,
                                              NULL);
  NOTSUPPOSEDTOBENULL(retval2);
  if(nodecreated)
  {
#ifdef DEBUG
    fragmentstore->currentnumofnodes++;
    if(fragmentstore->maxnumofnodes < fragmentstore->currentnumofnodes)
    {
      fragmentstore->maxnumofnodes = fragmentstore->currentnumofnodes;
    }
#endif
  } else
  {
    if(evalpriority(addterminal,fragmentinfo,FRAGIDENT(retval2)) < 
       evalpriority(addterminal,fragmentinfo,FRAGIDENT(qfrag2)))
    {
#ifdef DEBUG
      if(retval2->fpposition != qfrag2->fpposition)
      {
        fprintf(stderr,"retval2->fpposition=%lu != %lu=qfrag2->fpposition\n",
                        (Showuint) retval2->fpposition,
                        (Showuint) qfrag2->fpposition);
        exit(EXIT_FAILURE);
      }
#endif
      retval2->fpident = qfrag2->fpident;
    } 
    free(qfrag2);
  }
  FUNCTIONFINISH;
}

static void activatefragpoint(BOOL addterminal,
                              Fragmentinfo *fragmentinfo,
                              Fragmentstore *fragmentstore,
                              Fragpoint *qfrag2)
{
  Fragpoint *tmp2;
  Chainscoretype qpriority;

  qpriority = evalpriority(addterminal,fragmentinfo,FRAGIDENT(qfrag2));
  DEBUG2(2,"compute previous for fragment %lu with endpoint %lu\n",
          (Showuint) FRAGIDENT(qfrag2),
          (Showuint) qfrag2->fpposition);
  tmp2 = (Fragpoint *) redblacktreepreviousequalkey ((const Keytype) qfrag2,
                                                     fragmentstore->dictroot,
                                                     cmpendFragpoint2,
                                                     NULL);
  if(tmp2 == NULL ||
     qpriority > evalpriority(addterminal,fragmentinfo,FRAGIDENT(tmp2)))
  {
    insertintodict(addterminal,fragmentinfo,fragmentstore,qfrag2);
    while(True)
    {
      DEBUG2(2,"compute next for fragment %lu with endpoint %lu\n",
              (Showuint) FRAGIDENT(qfrag2),
              (Showuint) qfrag2->fpposition);
      tmp2 = (Fragpoint *) redblacktreenextkey ((const Keytype) qfrag2,
                                                fragmentstore->dictroot,
                                                cmpendFragpoint2,
                                                NULL);
      if(tmp2 == NULL || qpriority <= evalpriority(addterminal,
                                                   fragmentinfo,
                                                   FRAGIDENT(tmp2)))
      {
        break;
      }
      DEBUG2(2,"found next fragment %lu with endpoint %lu\n",
             (Showuint) FRAGIDENT(tmp2),
             (Showuint) tmp2->fpposition);
      DEBUG2(2,"delete fragment %lu with endpoint %lu\n",
              (Showuint) FRAGIDENT(tmp2),
              (Showuint) tmp2->fpposition);
      if(redblacktreedelete ((const Keytype) tmp2,
                             &fragmentstore->dictroot,
                             cmpendFragpoint2,
                             NULL) != 0)
      {
        fprintf(stderr,"cannot delete successor node\n");
        exit(EXIT_FAILURE);
      }
      free(tmp2);
#ifdef DEBUG
      fragmentstore->currentnumofnodes--;
#endif
    }
  } else
  {
    free(qfrag2);
  }
  FUNCTIONFINISH;
}

static void evalfragmentscore(Chainmode *chainmode,
                              Fragmentinfo *fragmentinfo,
                              Uint numofmatches,
                              Fragmentstore *fragmentstore,
                              BOOL gapsL1,
                              Uint fragpointident,
                              Uint presortdim)
{
  Uint previous, startpos2;
  Fragpoint *qfrag2;
  Chainscoretype score;

  startpos2 = GETSTOREDSTARTPOINT(UintConst(1)-presortdim,fragpointident);
  if(startpos2 == 0)
  {
    qfrag2 = NULL;
  } else
  {
    Fragpoint keyfrag2;

    keyfrag2.fpposition = startpos2 - 1;  // it is a start position
    keyfrag2.fpident = MAKEENDPOINT(fragpointident); // but considered as endpt
    DEBUG1(2,"RMQ(%lu)\n",(Showuint) keyfrag2.fpposition);
    qfrag2 = (Fragpoint *) redblacktreepreviousequalkey(
                                  (const Keytype) &keyfrag2,
                                  fragmentstore->dictroot,
                                  cmpendFragpoint2,
                                  NULL);
    if(qfrag2 != NULL)
    {
      DEBUG2(2,"retrieved(%lu,%lu)\n",(Showuint) FRAGIDENT(qfrag2),
                                      (Showuint) qfrag2->fpposition);
      if(chainmode->maxgapwidth != 0 &&
         !checkmaxgapwidth(fragmentinfo,
                           chainmode->maxgapwidth,
                           FRAGIDENT(qfrag2),
                           fragpointident))
      {
        qfrag2 = NULL;
      }
    }
  }
  if(qfrag2 == NULL)
  {
    score = fragmentinfo[fragpointident].weight;
    if(chainmode->chainkind == GLOBALCHAININGWITHGAPCOST)
    {
      score -= INITIALGAP(fragpointident);
    }
    DEBUG1(2,"no previous found: add fragment weight %ld\n",(Showsint) score);
    previous = UNDEFPREVIOUS;
  } else
  {
    score = fragmentinfo[FRAGIDENT(qfrag2)].score;
    if(chainmode->chainkind == GLOBALCHAINING)
    {
      score += fragmentinfo[fragpointident].weight;
      previous = FRAGIDENT(qfrag2);
    } else
    {
      Chainscoretype tmpgc;

      if(gapsL1)
      {
        tmpgc = gapcostL1(fragmentinfo,FRAGIDENT(qfrag2),fragpointident);
      } else
      {
        tmpgc = gapcostCc(fragmentinfo,FRAGIDENT(qfrag2),fragpointident);
      }
      if(chainmode->chainkind == GLOBALCHAININGWITHGAPCOST || score > tmpgc)
      {
        score += (fragmentinfo[fragpointident].weight - tmpgc);
        previous = FRAGIDENT(qfrag2);
      } else
      {
        score = fragmentinfo[fragpointident].weight;
        previous = UNDEFPREVIOUS;
      }
    }
  }
  fragmentinfo[fragpointident].score = score;
  fragmentinfo[fragpointident].previousinchain = previous;
  if(previous == UNDEFPREVIOUS)
  {
    fragmentinfo[fragpointident].firstinchain = fragpointident;
  } else
  {
    fragmentinfo[fragpointident].firstinchain 
      = fragmentinfo[previous].firstinchain;
  }
  DEBUG2(2,"scoretable[%lu]=%ld\n",(Showuint) fragpointident,(Showsint) score);
  DEBUG2(2,"previousinchain[%lu]=%lu\n",(Showuint) fragpointident,
                                        (Showuint) previous);
  DEBUG2(2,"firstinchain[%lu]=%lu\n",(Showuint) fragpointident,
                                     (Showuint) fragmentinfo[fragpointident].
                                                firstinchain);
  FUNCTIONFINISH;
}

static BOOL isrightmaximallocalchain(Fragmentinfo *fragmentinfo,
                                     Uint numofmatches,
                                     Uint currentfrag)
{
  if(currentfrag == numofmatches - 1)
  {
    return True;
  }
  if(fragmentinfo[currentfrag+1].previousinchain != currentfrag)
  {
    return True;
  }
  if(fragmentinfo[currentfrag+1].score < fragmentinfo[currentfrag].score)
  {
    return True;
  }
  return False;
}

static void determineequivreps(Bestofclass *chainequivalenceclasses,
                               Fragmentinfo *fragmentinfo,
                               Uint numofmatches)
{
  Uint matchnum;
  Bestofclass *classptr, *classrep;

  for(classptr = chainequivalenceclasses;
      classptr < chainequivalenceclasses + numofmatches;
      classptr++)
  {
    classptr->isavailable = False;
  }
  for(matchnum=0; matchnum<numofmatches; matchnum++)
  {
    if(isrightmaximallocalchain(fragmentinfo,numofmatches,matchnum))
    {
      classrep = chainequivalenceclasses + 
                 fragmentinfo[matchnum].firstinchain;
      if(!classrep->isavailable || 
         classrep->score < fragmentinfo[matchnum].score)
      {
        classrep->score = fragmentinfo[matchnum].score;
        classrep->isavailable = True;
      }
    }
  }
}

static BOOL retrievemaximalscore(Chainscoretype *maxscore,
                                 Chainmode *chainmode,
                                 Fragmentinfo *fragmentinfo,
                                 Uint numofmatches)
{
  Uint matchnum;
  Chainscoretype tgap;
  BOOL maxscoredefined = False;

  *maxscore = 0;
  for(matchnum=0; matchnum<numofmatches; matchnum++)
  {
    if(isrightmaximallocalchain(fragmentinfo,numofmatches,matchnum))
    {
      if(chainmode->chainkind == GLOBALCHAININGWITHGAPCOST)
      {
        tgap = TERMINALGAP(matchnum);
      } else
      {
        tgap = 0;
      }
      if(!maxscoredefined || *maxscore < fragmentinfo[matchnum].score - tgap)
      {
        *maxscore = fragmentinfo[matchnum].score - tgap;
        maxscoredefined = True;
      }
    }
  }
  return maxscoredefined;
}

static Sint comparescores(const Keytype key1,
                          const Keytype key2,
                          /*@unused@*/ void *info)
{
  if(*((Uint *) key1) < *(((Uint *) key2)))
  {
    return (Sint) -1;
  }
  if(*((Uint *) key1) > *(((Uint *) key2)))
  {
    return (Sint) 1;
  }
  return 0;
}

static Sint retrievechainbestscores(BOOL *minscoredefined,
                                    Chainscoretype *minscore,
                                    Fragmentinfo *fragmentinfo,
                                    Uint numofmatches,
                                    Uint howmanybest)
{
  Uint matchnum, fragnum = 0;
  Chainscoretype *scores;
  Dictmaxsize dictbestfragments;
  void *minkey;

  ALLOCASSIGNSPACE(scores,NULL,Chainscoretype,numofmatches);
  initDictmaxsize(&dictbestfragments,howmanybest);
  for(matchnum=0; matchnum<numofmatches; matchnum++)
  {
    if(isrightmaximallocalchain(fragmentinfo,numofmatches,matchnum))
    {
      scores[fragnum] = fragmentinfo[matchnum].score;
      if(insertDictmaxsize(&dictbestfragments,
                           comparescores,
                           NULL,
                           NULL,
                           NULL,
                           (void *) (scores + fragnum)) != 0)
      {
        return (Sint) -1;
      }
      fragnum++;
    }
  }
  if(fragnum == 0)
  {
    *minscoredefined = False;
  } else
  {
    minkey = redblacktreeminimumkey(dictbestfragments.root);
    NOTSUPPOSEDTOBENULL(minkey);
    *minscore = *((Chainscoretype *) minkey);
    *minscoredefined = True;
  }
  redblacktreedestroy (False,NULL,NULL,dictbestfragments.root);
  FREESPACE(scores);
  return 0;
}

static Sint retrievechainthreshold(Chainmode *chainmode,
                                   Fragmentinfo *fragmentinfo,
                                   Uint numofmatches,
                                   Chain *chain,
                                   Chainscoretype minscore,
                                   Chaingapcostfunction chaingapcostfunction,
                                   Bestofclass *chainequivalenceclasses,
                                   Chainprocessor chainprocessor,
                                   void *cpinfo)
{
  Uint matchnum;
  Chainscoretype tgap;
  Bestofclass *classrep;

  for(matchnum=0; matchnum < numofmatches; matchnum++)
  {
    if(isrightmaximallocalchain(fragmentinfo,numofmatches,matchnum))
    {
      if(chainmode->chainkind == GLOBALCHAININGWITHGAPCOST)
      {
        tgap = TERMINALGAP(matchnum);
      } else
      {
        tgap = 0;
      }
      if(fragmentinfo[matchnum].score - tgap >= minscore)
      {
        if(chainequivalenceclasses != NULL)
        {
          classrep = chainequivalenceclasses + 
                     fragmentinfo[matchnum].firstinchain;
          NOTSUPPOSEDTOBENULL(classrep);
          if(classrep->isavailable &&
             classrep->score == fragmentinfo[matchnum].score - tgap)
          {
            chain->scoreofchain = classrep->score;
            classrep->isavailable = False;
            retracepreviousinchain(chain,
                                   fragmentinfo,
                                   numofmatches,
                                   matchnum);
            if(chainprocessor(cpinfo,chain))
            {
              return (Sint) -1;
            }
          }
        } else
        {
          chain->scoreofchain = fragmentinfo[matchnum].score - tgap;
          retracepreviousinchain(chain,
                                 fragmentinfo,
                                 numofmatches,
                                 matchnum);
#ifdef DEBUG
          if(chainmode->chainkind != GLOBALCHAININGWITHOVERLAPS)
          {
            checkthechain(chainmode,fragmentinfo,numofmatches,chain,
                          chaingapcostfunction);
          }
          DEBUGCODE(2,showthechain(chainmode,chain,fragmentinfo,
                                   chaingapcostfunction));
#endif
          if(chainprocessor(cpinfo,chain))
          {
            return (Sint) -1;
          }
        }
      }
    }
  }
  return 0;
}


static Sint comparestartandend(Fragmentinfo *sortedstartpoints,
                               Fragmentinfo *sortedendpoints,
                               Uint presortdim)
{
  if(sortedstartpoints->startpos[presortdim] <
     sortedendpoints->endpos[presortdim])
  {
    return (Sint) -1;
  }
  if(sortedstartpoints->startpos[presortdim] >
     sortedendpoints->endpos[presortdim])
  {
    return (Sint) 1;
  }
  return (Sint) -1;
}

#ifdef DEBUG

/*
static void checkstartpoint(Fragpoint *fragpoints,Uint xidx,Uint startcount)
{
  if(ISENDPOINT(&fragpoints[xidx]))
  {
    fprintf(stderr,"xfrag[%lu] is endpoint\n",(Showuint) xidx);
    exit(EXIT_FAILURE);
  }
  if(FRAGIDENT(&fragpoints[xidx]) != startcount)
  {
    fprintf(stderr,"xfrag[%lu]=%lu != %lu\n",
            (Showuint) xidx,
            (Showuint) FRAGIDENT(&fragpoints[xidx]),
            (Showuint) startcount);
    exit(EXIT_FAILURE);
  }
}

static void checkendpoint(Fragpoint *fragpoints,Uint xidx,Uint endindex)
{
  if(!ISENDPOINT(&fragpoints[xidx]))
  {
    fprintf(stderr,"xfrag[%lu] is not endpoint\n",(Showuint) xidx);
    exit(EXIT_FAILURE);
  }
  if(FRAGIDENT(&fragpoints[xidx]) != endindex)
  {
    fprintf(stderr,"xfrag[%lu]=%lu != %lu\n",
            (Showuint) xidx,
            (Showuint) FRAGIDENT(&fragpoints[xidx]),
            (Showuint) endindex);
    exit(EXIT_FAILURE);
  }
}
*/

#endif

static Fragpoint *makeactivationpoint(Fragmentinfo *fragmentinfo,
                                      Uint fpident,Uint postsortdim)
{
  Fragpoint *fpptr;

  fpptr = malloc(sizeof(Fragpoint));
  if(fpptr == NULL)
  {
    fprintf(stderr,"Cannot allocate space for Fragpoint\n");
    exit(EXIT_FAILURE);
  }
  fpptr->fpident = MAKEENDPOINT(fpident);
  fpptr->fpposition = GETSTOREDENDPOINT(postsortdim,fpident);
  return fpptr;
}

static void mergestartandendpoints(Chainmode *chainmode,
                                   Fragmentinfo *fragmentinfo,
                                   Uint numofmatches,
                                   Fragmentstore *fragmentstore,
                                   BOOL gapsL1,
                                   Uint presortdim)
{
#ifdef DEBUG
  __attribute__ ((unused)) Uint xidx = 0;
#endif
  Uint startcount, endcount;
  BOOL addterminal;
  Uint postsortdim = UintConst(1) - presortdim;

  addterminal = (chainmode->chainkind == GLOBALCHAINING) ? False : True;
  fragmentstore->dictroot = NULL;
  for(startcount = 0, endcount = 0; 
      startcount < numofmatches && endcount < numofmatches; 
      /* Nothing */)
  {
    if(comparestartandend(fragmentinfo + startcount,
                          fragmentinfo + fragmentstore->endpointperm[endcount],
                          presortdim) < 0)
    {
#ifdef DEBUG
  /*
      checkstartpoint(fragmentstore->xfragpoints.spaceFragpoint,
                      xidx,startcount);
  */
#endif
      evalfragmentscore(chainmode,
                        fragmentinfo,
                        numofmatches,
                        fragmentstore,
                        gapsL1,
                        startcount,
                        presortdim);
      startcount++;
    } else
    {
#ifdef DEBUG
  /*
      checkendpoint(fragmentstore->xfragpoints.spaceFragpoint,
                    xidx,fragmentstore->endpointperm[endcount]);
  */
#endif
      activatefragpoint(addterminal,fragmentinfo,fragmentstore,
                        makeactivationpoint(fragmentinfo,
                                            fragmentstore->
                                            endpointperm[endcount],
                                            postsortdim));
      endcount++;
    }
#ifdef DEBUG
    xidx++;
#endif
  }
  while(startcount < numofmatches)
  {
#ifdef DEBUG
  /*
    checkstartpoint(fragmentstore->xfragpoints.spaceFragpoint,
                    xidx,startcount);
  */
#endif
    evalfragmentscore(chainmode,
                      fragmentinfo,
                      numofmatches,
                      fragmentstore,
                      gapsL1,
                      startcount,
                      presortdim);
    startcount++;
#ifdef DEBUG
    xidx++;
#endif
  }
  while(endcount < numofmatches)
  {
#ifdef DEBUG
   /*
      checkendpoint(fragmentstore->xfragpoints.spaceFragpoint,
                    xidx,fragmentstore->endpointperm[endcount]);
   */
#endif
    activatefragpoint(addterminal,fragmentinfo,fragmentstore,
                      makeactivationpoint(fragmentinfo,
                                          fragmentstore->
                                          endpointperm[endcount],
                                          postsortdim));
    endcount++;
#ifdef DEBUG
    xidx++;
#endif
  }
#ifdef DEBUG
  if(chainmode->chainkind == GLOBALCHAINING)
  {
    if(redblacktreecheckscoreorder(presortdim,
                                   fragmentstore->dictroot,
                                   fragmentinfo) != 0)
    {
      fprintf(stderr,"%s\n",messagespace());
      exit(EXIT_FAILURE);
    }
  }
#endif
}

static Sint findmaximalscores(Chainmode *chainmode,
                              Chain *chain,
                              Fragmentinfo *fragmentinfo,
                              Uint numofmatches,
                              Fragmentstore *fragmentstore,
                              Chainprocessor chainprocessor,
                              Chaingapcostfunction chaingapcostfunction,
                              BOOL withequivclasses,
                              void *cpinfo,
                              Showverbose showverbose)
{
  Uint fragnum;
  Chainscoretype minscore = 0;
  Fragpoint *maxpoint;
  BOOL minscoredefined = False;
  Bestofclass *chainequivalenceclasses;

  if(withequivclasses)
  {
    if(chainmode->chainkind == LOCALCHAININGMAX ||
       chainmode->chainkind == LOCALCHAININGTHRESHOLD || 
       chainmode->chainkind == LOCALCHAININGBEST || 
       chainmode->chainkind == LOCALCHAININGPERCENTAWAY)
    {
      ALLOCASSIGNSPACE(chainequivalenceclasses,NULL,Bestofclass,numofmatches);
      determineequivreps(chainequivalenceclasses, fragmentinfo, numofmatches);
    } else
    {
      chainequivalenceclasses = NULL;
    }
  } else
  {
    chainequivalenceclasses = NULL;
  }
  switch(chainmode->chainkind)
  {
    case GLOBALCHAINING:
      maxpoint = redblacktreemaximumkey(fragmentstore->dictroot);
      NOTSUPPOSEDTOBENULL(maxpoint);
      fragnum = FRAGIDENT(maxpoint);
      minscore = fragmentinfo[fragnum].score;
      minscoredefined = True;
      break;
    case GLOBALCHAININGWITHGAPCOST:
    case GLOBALCHAININGWITHOVERLAPS:
    case LOCALCHAININGMAX:
      minscoredefined
        = retrievemaximalscore(&minscore,chainmode,fragmentinfo,numofmatches);
      break;
    case LOCALCHAININGTHRESHOLD:
      minscore = chainmode->minimumscore;
      minscoredefined = True;
      break;
    case LOCALCHAININGBEST:
      if(retrievechainbestscores(&minscoredefined,
                                 &minscore,
                                 fragmentinfo,
                                 numofmatches,
                                 chainmode->howmanybest) != 0)
      {
        return (Sint) -1;
      }
      break;
    case LOCALCHAININGPERCENTAWAY:
      minscoredefined 
        = retrievemaximalscore(&minscore,chainmode,fragmentinfo,numofmatches);
      if(minscoredefined)
      {
        minscore = (Chainscoretype) 
                   ((double) minscore * 
                    (1.0 - (double) chainmode->percentawayfrombest/100.0));
      }
      break;
    default:
      ERROR1("chainkind = %ld not valid\n",(Showsint) chainmode->chainkind);
      return (Sint) -2;
  }
  if(minscoredefined)
  {
    if(showverbose != NULL)
    {
      char vbuf[80+1];
      sprintf(vbuf,"compute optimal %s chains with score >= %ld",
               (chainmode->chainkind == GLOBALCHAINING ||
                chainmode->chainkind == GLOBALCHAININGWITHGAPCOST ||
                chainmode->chainkind == GLOBALCHAININGWITHOVERLAPS)
                ? "global"
                : "local",
               (Showsint) minscore);
      showverbose(vbuf);
    }
    if(retrievechainthreshold(chainmode,
                              fragmentinfo,
                              numofmatches,
                              chain,
                              minscore,
                              chaingapcostfunction,
                              chainequivalenceclasses,
                              chainprocessor,
                              cpinfo) != 0)
    {
      return (Sint) -3;
    }
  } else
  {
    return (Sint) 1;
  }
  if(withequivclasses)
  {
    FREESPACE(chainequivalenceclasses);
  }
  return 0;
}

static void makesortedendpointpermutation(Uint *perm,
                                          Fragmentinfo *fragmentinfo,
                                          Uint numofmatches,
                                          Uint presortdim)
{
  Uint temp, *iptr, *jptr, i;
#ifdef DEBUG
  Uint moves = 0;
#endif

  for (i = 0; i < numofmatches; i++)
  {
    perm[i] = i;
  }
  for (iptr = perm + UintConst(1); iptr < perm + numofmatches; iptr++)
  {
    for (jptr = iptr; jptr > perm; jptr--)
    {
      if(GETSTOREDENDPOINT(presortdim,*(jptr-1)) <=
         GETSTOREDENDPOINT(presortdim,*jptr))
      {
        break;
      }
      temp = *(jptr-1);
      *(jptr-1) = *jptr;
      *jptr = temp;
#ifdef DEBUG
      moves++;
#endif
    }
  }
  DEBUG2(1,"# moves for insertion sort = %lu (%.2f)\n",
            (Showuint) moves,(double) moves/numofmatches);
}

#ifdef DEBUG
static void checksortedstartpoints(Fragmentinfo *fragmentinfo,
                                   Uint numofmatches,
                                   Uint presortdim)
{
  Uint idx;

  for(idx=UintConst(1); idx<numofmatches; idx++)
  {
    if(GETSTOREDSTARTPOINT(presortdim,idx-1) > 
       GETSTOREDSTARTPOINT(presortdim,idx))
    {
      fprintf(stderr,"start(%lu)=%lu > %lu=start(%lu)\n",
                      (Showuint) (idx-1),
                      (Showuint) GETSTOREDSTARTPOINT(presortdim,idx-1),
                      (Showuint) GETSTOREDSTARTPOINT(presortdim,idx),
                      (Showuint) idx);
      exit(EXIT_FAILURE);
    }
  }
}

static void checksortedendpoints(Fragmentinfo *fragmentinfo,
                                 Uint *endpointperm,
                                 Uint numofmatches,
                                 Uint presortdim)
{
  Uint idx;

  for(idx=UintConst(1); idx<numofmatches; idx++)
  {
    if(GETSTOREDENDPOINT(presortdim,endpointperm[idx-1]) > 
       GETSTOREDENDPOINT(presortdim,endpointperm[idx]))
    {
      fprintf(stderr,"end(%lu)=%lu > %lu=end(%lu)\n",
                   (Showuint) endpointperm[idx-1],
                   (Showuint) GETSTOREDENDPOINT(presortdim,endpointperm[idx-1]),
                   (Showuint) GETSTOREDENDPOINT(presortdim,endpointperm[idx]),
                   (Showuint) endpointperm[idx]);
      exit(EXIT_FAILURE);
    }
  }
}
#endif

static void fastchainingscores(Chainmode *chainmode,
                               Fragmentinfo *fragmentinfo,
                               Uint numofmatches,
                               Fragmentstore *fragmentstore,
                               Uint presortdim,
                               BOOL gapsL1)
{
#ifdef DEBUG
  checksortedstartpoints(fragmentinfo,numofmatches,presortdim);
#endif
  ALLOCASSIGNSPACE(fragmentstore->endpointperm,NULL,Uint,numofmatches);
  makesortedendpointpermutation(fragmentstore->endpointperm,
                                fragmentinfo,
                                numofmatches,
                                presortdim);
#ifdef DEBUG
  fragmentstore->maxnumofnodes = fragmentstore->currentnumofnodes = 0;
  checksortedendpoints(fragmentinfo,
                       fragmentstore->endpointperm,
                       numofmatches,
                       presortdim);
  derivefragpoints(&fragmentstore->xfragpoints,
                   fragmentinfo,
                   numofmatches,
                   presortdim);
  sortfragpoints(fragmentstore->xfragpoints.spaceFragpoint,
                 fragmentstore->xfragpoints.nextfreeFragpoint);
  DEBUGCODE(3,showxfragpoints(fragmentstore->xfragpoints.spaceFragpoint,
                              fragmentstore->xfragpoints.nextfreeFragpoint));
#endif
  mergestartandendpoints(chainmode,
                         fragmentinfo,
                         numofmatches,
                         fragmentstore,
                         gapsL1,
                         presortdim);
  DEBUG1(2,"# maxnumofnodes = %lu\n",(Showuint) fragmentstore->maxnumofnodes);
  FREESPACE(fragmentstore->endpointperm);
  FUNCTIONFINISH;
}

/*@null@*/ static Chaingapcostfunction assignchaingapcostfunction(Chainkind 
                                                                  chainkind,
                                                                  BOOL gapsL1)
{
  if(chainkind == GLOBALCHAININGWITHOVERLAPS)
  {
    return overlapcost;
  }
  if(chainkind != GLOBALCHAINING)
  {
    if(gapsL1)
    {
      return gapcostL1;
    }
    return gapcostCc;
  }
  return NULL;
}

/*EE
  The following function implements the different kinds of chaining
  algorithms for global and local chaining. The mode is specified
  by \texttt{chainmode}. There are \texttt{numofmatches} many matches,
  each specified in an array \texttt{fragmentinfo} points to. 
  \texttt{chain} is the array in which a chain is stored. However,
  do not further process chain. Just use 
  INITARRAY(&chain.chainedfragments,Uint) to initialize it before calling
  \textttt{fastchaining} and 
  FREEARRAY(&chain.chainedfragments,Uint) to free the space after calling
  \textttt{fastchaining}. The function \texttt{chainprocessor} processes 
  each chain found, and \texttt{cpinfo} is used as a the first argument 
  in each call to \texttt{chainprocessor}. The function returns 
  0 upon success and otherwise, if an error occurs.
  Finally \texttt{showverbose} is applied to each status message generated
  during the execution of \texttt{fastchaining}. If \texttt{showverbose}
  is \texttt{NULL}, then nothing is generated and shown.
*/

Sint fastchaining(Chainmode *chainmode,
                  Chain *chain,
                  Fragmentinfo *fragmentinfo,
                  Uint numofmatches,
                  BOOL gapsL1,
                  Uint presortdim,
                  BOOL withequivclasses,
                  Chainprocessor chainprocessor,
                  void *cpinfo,
                  Showverbose showverbose)
{
  Sint retcode;
  Chaingapcostfunction chaingapcostfunction;

  if(presortdim > UintConst(1))
  {
    ERROR0("fastchaining: presortdim must be 0 or 1");
    return (Sint) -1;
  }
  chaingapcostfunction 
    = assignchaingapcostfunction(chainmode->chainkind,gapsL1);
  if(numofmatches > UintConst(1))
  {
    Fragmentstore fragmentstore;
    DEBUGDECL(Fragmentinfo *fragmentinfobrute);

    // compute chains
    if(showverbose != NULL)
    {
      showverbose("compute chain scores");
    }
    if(chainmode->chainkind == GLOBALCHAININGWITHOVERLAPS)
    {
      bruteforcechainingscores(chainmode,
                               fragmentinfo,
                               numofmatches,
                               chaingapcostfunction);
    } else
    {
      fastchainingscores(chainmode,
                         fragmentinfo,
                         numofmatches,
                         &fragmentstore,
                         presortdim,
                         gapsL1);
#ifdef DEBUG
      fragmentinfobrute = copyfragmentinfo(fragmentinfo,numofmatches);
      bruteforcechainingscores(chainmode,
                               fragmentinfobrute,
                               numofmatches,
                               chaingapcostfunction);
      checkallFragmentinfos(
             chainmode->chainkind == GLOBALCHAININGWITHGAPCOST ? True : False,
             fragmentinfo,
             fragmentinfobrute,
             numofmatches);
      FREESPACE(fragmentinfobrute);
#endif
    }
    if(showverbose != NULL)
    {
      showverbose("retrieve optimal chains");
    }
    retcode = findmaximalscores(chainmode,
                                chain,
                                fragmentinfo,
                                numofmatches,
                                &fragmentstore,
                                chainprocessor,
                                chaingapcostfunction,
                                withequivclasses,
                                cpinfo,
                                showverbose);
    if(chainmode->chainkind != GLOBALCHAININGWITHOVERLAPS)
    {      
      redblacktreedestroy (True,NULL,NULL,fragmentstore.dictroot);
#ifdef DEBUG
      FREEARRAY(&fragmentstore.xfragpoints,Fragpoint);
#endif
    }
    if(retcode < 0)
    {
      return (Sint) -1;
    }
  } else
  {
    chainingboundarycases(chainmode,
                          chain,
                          fragmentinfo,
                          numofmatches);
    if(chainprocessor(cpinfo,chain))
    {
      return (Sint) -2;
    }
    retcode = 0;
  }
  return retcode;
}
