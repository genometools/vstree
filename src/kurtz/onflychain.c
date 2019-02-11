#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "debugdef.h"
#include "errordef.h"
#include "minmax.h"
#include "spacedef.h"
#include "failures.h"
#include "redblackdef.h"
#include "onflychaindef.h"
#include "gqueue-if.h"
#include "gqueueproc.h"

#include "redblack.pr"
#include "genericqueue.pr"

#define Ivalue(FRAG) (FRAG)->startpos[0]
#define Jvalue(FRAG) (FRAG)->startpos[1]

#define FRAGDIAGONAL(FRAG) (Sint) (Jvalue(FRAG) - Ivalue(FRAG))

typedef struct
{
  Chainscoretype maxscore;
  Onflyfragment *maxfrag;
  BOOL defined;
} Maxfragvalue;

typedef struct
{
  BOOL chainqhits;
  Uint maxdistance;
  Onflyfragment *rightfrag;
  Maxfragvalue best;
} Currentfraginfo;

typedef struct
{
  Sint lowerdiagonal,
       upperdiagonal;
} Diagonalrange;

/*
  The Chebychev distance for two matches h=(i,j) and h'=(i',j')
  is defined as: 
                 max{|i'-i-length|,|j'-j-length|},
*/

static Uint matchesgapcostCc(const Onflyfragment *leftfrag,
                             const Onflyfragment *rightfrag)
{
  Sint valuefirst, valuesecond;

  valuefirst = (Sint) Ivalue(rightfrag) - 
               (Sint) (Ivalue(leftfrag) + leftfrag->fraglength);
  if(valuefirst < 0)
  {
    // valuefirst = -valuefirst;
    valuefirst = 0;
  }
  valuesecond = (Sint) Jvalue(rightfrag) - 
                (Sint) (Jvalue(leftfrag) + leftfrag->fraglength);
  if(valuesecond < 0)
  {
    //valuesecond = -valuesecond;
    valuesecond = 0;
  }
  return (Uint) MAX(valuefirst,valuesecond);
}

#ifdef DEBUG
static Sint simpleoutputchain(const Onflyfragment *fragptr,
                              /*@unused@*/ void *info)
{
  printf("i=%lu,j=%lu,d=%ld,last=%s,score=%ld, ",
         (Showuint) Ivalue(fragptr),
         (Showuint) Jvalue(fragptr),
         (Showsint) FRAGDIAGONAL(fragptr),
         SHOWBOOL(fragptr->islast),
         (Showsint) fragptr->score);
  if(fragptr->previousinchain == NULL)
  {
    printf("previous = NULL\n");
  } else
  {
    printf("previous = %lu\n",(Showuint) (fragptr->previousinchain->identity));
  }
  return 0;
}

static Sint showallfragments(const Onflyfragment *fragmentstore,
                             Uint numoffragments)
{
  Uint i;

  for(i=0; i<numoffragments; i++)
  {
    printf("frag %lu: ",(Showuint) i);
    if(simpleoutputchain(fragmentstore + i,NULL) != 0)
    {
      return (Sint) -1;
    }
  }
  return 0;
}

static Chainscoretype sumupscores(Onflyfragment *fragptr)
{
  Chainscoretype score;
  
  score = (Chainscoretype) fragptr->fraglength;
  if(fragptr->previousinchain != UNDEFPREVIOUS)
  {
    Onflyfragment *leftfrag, *rightfrag = fragptr;

    for(leftfrag = fragptr->previousinchain; 
        leftfrag != UNDEFPREVIOUS; 
        leftfrag = leftfrag->previousinchain)
    {
      score += (Chainscoretype) leftfrag->fraglength;
      score -= matchesgapcostCc(leftfrag,rightfrag);
      rightfrag = leftfrag;
    }
  }
  return score;
}

static void checkfragmentequality(const Onflyfragment *fptr1,
                                  const Onflyfragment *fptr2,
                                  BOOL checksameprevious)
{
  if(fptr1->score != fptr2->score)
  {
    fprintf(stderr,"fptr1[%lu].score = %ld != %ld = fptr2[%lu].score\n",
                   (Showuint) fptr1->identity,
                   (Showsint) fptr1->score,
                   (Showsint) fptr2->score,
                   (Showuint) fptr2->identity);
    exit(EXIT_FAILURE);
  }
  if(fptr1->previousinchain == NULL &&
     fptr2->previousinchain != NULL)
  {
    fprintf(stderr,"fptr1[%lu].previous = NULL != %lu = fptr2[%lu].previous\n",
                   (Showuint) fptr1->identity,
                   (Showuint) fptr2->previousinchain->identity,
                   (Showuint) fptr2->identity);
    exit(EXIT_FAILURE);
  }
  if(fptr1->previousinchain != NULL &&
     fptr2->previousinchain == NULL)
  {
    fprintf(stderr,"fptr1[%lu].previous = %lu != NULL = fptr2[%lu].previous\n",
                   (Showuint) fptr1->identity,
                   (Showuint) fptr1->previousinchain->identity,
                   (Showuint) fptr2->identity);
    exit(EXIT_FAILURE);
  }
  if(checksameprevious)
  {
    if(fptr1->previousinchain != NULL &&
       fptr2->previousinchain != NULL &&
       fptr1->previousinchain->identity != fptr2->previousinchain->identity)
    {
      fprintf(stderr,"fptr1[%lu].previous = %lu != %lu = fptr2[%lu].previous\n",
                       (Showuint) fptr1->identity,
                       (Showuint) fptr1->previousinchain->identity,
                       (Showuint) fptr2->previousinchain->identity,
                       (Showuint) fptr2->identity);
      exit(EXIT_FAILURE);
    }
  }
}

static void comparefragmentstores(const Onflyfragment *fragstore1,
                                  const Onflyfragment *fragstore2,
                                  Uint numoffragments)
{
  Uint i;

  for(i=0; i<numoffragments; i++)
  {
    checkfragmentequality(fragstore1 + i,fragstore2 + i,True);
  }
}

static Sint checkflychain(Onflyfragment *fragptr,void *info)
{
  Onflyfragment *referencestore = (Onflyfragment *) info;
  Chainscoretype bfscore;

  checkfragmentequality(fragptr,referencestore + fragptr->identity,False);
  bfscore = sumupscores(fragptr);
  if(bfscore != fragptr->score)
  {
    fprintf(stderr,"bfscore = %ld != %ld = fragptr[%lu].score\n",
                     (Showsint) bfscore,
                     (Showsint) fragptr->score,
                     (Showuint) fragptr->identity);
    exit(EXIT_FAILURE);
  }
  return 0;
}
#endif


static BOOL matchesarecompatible(BOOL chainqhits,
                                 Uint maxdistance,
                                 const Onflyfragment *leftfrag,
                                 const Onflyfragment *rightfrag)
{
  if(matchesgapcostCc(leftfrag,rightfrag) <= maxdistance)
  {
    if(Ivalue(leftfrag) + leftfrag->fraglength <= Ivalue(rightfrag) &&
       Jvalue(leftfrag) + leftfrag->fraglength <= Jvalue(rightfrag))
    {
      return True;
    }
    if(chainqhits)
    {
      if(FRAGDIAGONAL(leftfrag) == FRAGDIAGONAL(rightfrag) &&
         Ivalue(leftfrag) < Ivalue(rightfrag))
      {
        return True;
      }
    } 
  }
  return False;
}

static void maintainbestleft(Maxfragvalue *best,
                             Onflyfragment *leftfrag,
                             const Onflyfragment *rightfrag)
{
  Chainscoretype score;

  score = leftfrag->score - matchesgapcostCc(leftfrag,rightfrag);
  if(score > 0)
  {
    score += rightfrag->fraglength;
    if(!best->defined || best->maxscore < score)
    {
      best->maxscore = score;
      best->maxfrag = leftfrag;
      best->defined = True;
    }
  }
}

static void updatecurrentfrag(Onflyfragment *rightfrag,
                              const Maxfragvalue *best)
{
  if(best->defined)
  {
    rightfrag->lengthofchain = best->maxfrag->lengthofchain + UintConst(1);
    rightfrag->firstinchain = best->maxfrag->firstinchain;
    best->maxfrag->islast = False; 
    if(rightfrag->firstinchain->bestchainend == NULL ||
       rightfrag->firstinchain->bestchainend->score < best->maxscore ||
       (rightfrag->firstinchain->bestchainend->score == best->maxscore &&
        rightfrag->firstinchain->bestchainend->lengthofchain 
            < best->maxfrag->lengthofchain))
    {
      rightfrag->firstinchain->bestchainend = rightfrag;
    }
    rightfrag->previousinchain = best->maxfrag;
    rightfrag->score = best->maxscore;
  }
}
    
static BOOL isinJrange(Uint maxdistance,
                       const Onflyfragment *leftfrag,
                       const Onflyfragment *rightfrag)
{
  if(Jvalue(leftfrag) + leftfrag->fraglength + maxdistance + 1 
     < Jvalue(rightfrag))
  {
    return False;
  }
  return True;
}

static void initfragment(Onflyfragment *fptr)
{
  fptr->previousinchain = UNDEFPREVIOUS;
  fptr->score = (Chainscoretype) fptr->fraglength;
  fptr->firstinchain = fptr;
  fptr->bestchainend = NULL;
  fptr->lengthofchain = UintConst(1);
  fptr->islast = True;
}

#ifdef DEBUG

static BOOL isindiagonalrange(Uint maxdistance,
                              const Onflyfragment *leftfrag,
                              const Onflyfragment *rightfrag)
{
  Sint leftdiag, rightdiag;

  leftdiag = FRAGDIAGONAL(leftfrag);
  rightdiag = FRAGDIAGONAL(rightfrag);
  if(leftdiag < rightdiag - (Sint) maxdistance ||
     leftdiag > rightdiag + (Sint) maxdistance)
  {
    return False;
  }
  return True;
}

static void checkfragmentconstraints(Uint maxdistance,
                                     const Onflyfragment *leftfrag,
                                     const Onflyfragment *rightfrag)
{
  if(!isinJrange(maxdistance,leftfrag,rightfrag))
  {
    fprintf(stderr,
            "leftfrag at i=%lu to far apart from rightfrag at i' = %lu\n",
            (Showuint) Ivalue(leftfrag),
            (Showuint) Ivalue(rightfrag));
    exit(EXIT_FAILURE);
  }
  if(!isindiagonalrange(maxdistance,leftfrag,rightfrag))
  {
    fprintf(stderr,
            "leftfrag at i=%lu is on diagonal %ld which is "
            "not in diagonal range [%ld,%ld] for rightfrag at %lu\n",
            (Showuint) Ivalue(leftfrag),
            (Showsint) FRAGDIAGONAL(leftfrag),
            (Showsint) FRAGDIAGONAL(rightfrag) - (Sint) maxdistance,
            (Showsint) FRAGDIAGONAL(rightfrag) + (Sint) maxdistance,
            (Showuint) Ivalue(rightfrag));
    exit(EXIT_FAILURE);
  }
}

static void bruteforcechainingofmatches(BOOL chainqhits,
                                        Uint maxdistance,
                                        Onflyfragment *fragmentstore,
                                        Uint numoffragments)
{
  Onflyfragment *leftfrag, *rightfrag;
  Maxfragvalue best;

  if(numoffragments > 0)
  {
    initfragment(&fragmentstore[0]);
    for(rightfrag = fragmentstore + UintConst(1); 
        rightfrag < fragmentstore + numoffragments; 
        rightfrag++)
    {
      initfragment(rightfrag);
      best.defined = False;
      best.maxscore = 0;
      best.maxfrag = 0;
      for(leftfrag = rightfrag - 1; leftfrag >= fragmentstore; leftfrag--)
      {
        if(matchesarecompatible(chainqhits,
                                maxdistance,
                                leftfrag,
                                rightfrag))
        {
          checkfragmentconstraints(maxdistance,
                                   leftfrag,
                                   rightfrag);
          maintainbestleft(&best,
                           leftfrag,
                           rightfrag);
        }
      }
      updatecurrentfrag(rightfrag,&best);
    }
  }
}

static void chainingofmatches1(BOOL chainqhits,
                               Uint maxdistance,
                               Onflyfragment *fragmentstore,
                               Uint numoffragments)
{
  Onflyfragment *leftfrag, *rightfrag;
  Maxfragvalue best;

  if(numoffragments > 0)
  {
    Uint checkedfrags = 0, candidates1 = 0, candidates2 = 0;

    initfragment(&fragmentstore[0]);
    for(rightfrag = fragmentstore + UintConst(1); 
        rightfrag < fragmentstore + numoffragments; 
        rightfrag++)
    {
      initfragment(rightfrag);
      best.defined = False;
      best.maxscore = 0;
      best.maxfrag = 0;
      for(leftfrag = rightfrag - 1; leftfrag >= fragmentstore; leftfrag--)
      {
        if(!isinJrange(maxdistance,leftfrag,rightfrag))
        {
          checkedfrags += (rightfrag-leftfrag);
          break;
        }
        if(isindiagonalrange(maxdistance,leftfrag,rightfrag))
        {
          candidates1++;
          if(matchesarecompatible(chainqhits,
                                  maxdistance,
                                  leftfrag,
                                  rightfrag))
          {
            candidates2++;
            maintainbestleft(&best,
                             leftfrag,
                             rightfrag);
          }
        }
      }
      updatecurrentfrag(rightfrag,&best);
    }
    printf("average checked: %.2f\n",(double) checkedfrags/numoffragments);
    printf("average candidates1: %.2f\n",(double) candidates1/numoffragments);
    printf("average candidates2: %.2f\n",(double) candidates2/numoffragments);
  }
}
#endif

void initmaintainedfragments(Maintainedfragments *maintainedfragments,
                             BOOL overqueue)
{
  maintainedfragments->lastelements = emptyqueuegeneric();
  INITARRAY(&maintainedfragments->readyforoutput,Onflyfragmentptr);
  maintainedfragments->dictroot = NULL;
  maintainedfragments->overqueue = overqueue;
#ifdef DEBUG
  maintainedfragments->currentidentity = 0;
#endif
}

static Sint combinewithcurrentfragmentinqueue(void *elem,void *info)
{
  Onflyfragment *leftfrag = (Onflyfragment *) elem;
  Currentfraginfo *currentfraginfo = (Currentfraginfo *) info;

  if(matchesarecompatible(currentfraginfo->chainqhits,
                          currentfraginfo->maxdistance,
                          leftfrag,
                          currentfraginfo->rightfrag))
  {
    maintainbestleft(&currentfraginfo->best,
                     leftfrag,
                     currentfraginfo->rightfrag);
  }
  return 0;
}

static Sint combinewithcurrentfragmentintree(Onflyfragment *leftfrag,
                                              void *info)
{
  Currentfraginfo *currentfraginfo = (Currentfraginfo *) info;

  if(matchesarecompatible(currentfraginfo->chainqhits,
                          currentfraginfo->maxdistance,
                          leftfrag,
                          currentfraginfo->rightfrag))
  {
    maintainbestleft(&currentfraginfo->best,
                     leftfrag,
                     currentfraginfo->rightfrag);
  }
  return 0;
}

static Sint applyredblackwalkrange(const Keytype bmkey,
                                   /*@unused@*/ VISIT which,
                                   /*@unused@*/ Uint depth,
                                   void *applyinfo)
{
  return combinewithcurrentfragmentintree((Onflyfragment *) bmkey,
                                          applyinfo);
}

static Sint comparediagonals(const Keytype keya,
                             const Keytype keyb,
                             /*@unused@*/ void *info)
{
  Onflyfragment *frag1 = (Onflyfragment *) keya,
                *frag2 = (Onflyfragment *) keyb;
  if(FRAGDIAGONAL(frag1) < FRAGDIAGONAL(frag2))
  {
    return (Sint) -1;
  }
  if(FRAGDIAGONAL(frag1) > FRAGDIAGONAL(frag2))
  {
    return (Sint) 1;
  }
  if(Jvalue(frag1) < Jvalue(frag2))
  {
    return (Sint) -1;
  }
  if(Jvalue(frag1) > Jvalue(frag2))
  {
    return (Sint) 1;
  }
#ifdef DEBUG
  if(frag1->identity != frag2->identity)
  {
    fprintf(stderr,"same diagonal, same Jvalue, but different identities");
    exit(EXIT_FAILURE);
  }
#endif
  return 0;
}

static BOOL greaterequallowerdiag(const Keytype keyvalue,void *info)
{
  Diagonalrange *diagrange = (Diagonalrange *) info;
  Onflyfragment *fragptr = (Onflyfragment *) keyvalue;

  if(FRAGDIAGONAL(fragptr) >= diagrange->lowerdiagonal)
  {
    return True;
  }
  return False;
}

static BOOL lowerequalupperdiag(const Keytype keyvalue,void *info)
{
  Diagonalrange *diagrange = (Diagonalrange *) info;
  Onflyfragment *fragptr = (Onflyfragment *) keyvalue;

  if(FRAGDIAGONAL(fragptr) <= diagrange->upperdiagonal)
  {
    return True;
  }
  return False;
}

static Sint outputallstackedelements(ArrayOnflyfragmentptr *readyforoutput,
                                     Outflychain outputchain,
                                     void *outinfo)
{
  Onflyfragmentptr *ptr;

  if(readyforoutput->nextfreeOnflyfragmentptr > 0)
  {
    for(ptr = readyforoutput->spaceOnflyfragmentptr + 
              readyforoutput->nextfreeOnflyfragmentptr - 1; 
        ptr >= readyforoutput->spaceOnflyfragmentptr;
        ptr--)
    {
      if((*ptr)->firstinchain->bestchainend == *ptr)
      {
        if(outputchain(*ptr,outinfo) != 0)
        {
          return (Sint) -1;
        }
        DEBUG1(3,"free fragment %lu\n",(Showuint) (*ptr)->identity);
      }
      free(*ptr);
    }
    readyforoutput->nextfreeOnflyfragmentptr = 0;
  }
  return 0;
}

Sint processnewquhit(BOOL chainqhits,
                     Uint maxdistance,
                     Maintainedfragments *maintainedfragments,
                     Uint fraglength,
                     Uint startpos0,
                     Uint startpos1,
                     Outflychain outputchain,
                     void *outinfo)
{
  Onflyfragment *head, *newfragment;
  Currentfraginfo currentfraginfo;
  BOOL nodecreated;

  newfragment = malloc(sizeof(Onflyfragment));
  if(newfragment == NULL)
  {
    ERROR0("cannot allocated space for newfragment");
    return (Sint) -2; 
  }
  newfragment->fraglength = fraglength,
  newfragment->startpos[0] = startpos0;
  newfragment->startpos[1] = startpos1;
  initfragment(newfragment);
#ifdef DEBUG
  newfragment->identity = maintainedfragments->currentidentity++;
  DEBUG1(3,"init fragment %lu\n",(Showuint) newfragment->identity);
#endif
  while(!queueisemptygeneric(maintainedfragments->lastelements))
  {
    head = headofqueuegeneric(maintainedfragments->lastelements);
    if(isinJrange(maxdistance,
                  head,
                  newfragment))
    {
      break;
    }
    if(!maintainedfragments->overqueue)
    {
      if(redblacktreedelete (head,
                             &maintainedfragments->dictroot,
                             comparediagonals,
                             NULL) != 0)
      {
        fprintf(stderr,"cannot head from red black tree\n");
        exit(EXIT_FAILURE);
      }
    }
    // do not free because some others may reference it
    STOREINARRAY(&maintainedfragments->readyforoutput,
                 Onflyfragmentptr,32,head);
    (void) dequeuegeneric(maintainedfragments->lastelements);
    if(queueisemptygeneric(maintainedfragments->lastelements))
    {
      if(outputallstackedelements(&maintainedfragments->readyforoutput,
                                  outputchain,
                                  outinfo) != 0)
      {
        return (Sint) -1;
      }
    }
  }
  currentfraginfo.rightfrag = newfragment;
  currentfraginfo.chainqhits = chainqhits;
  currentfraginfo.maxdistance = maxdistance;
  currentfraginfo.best.defined = False;
  if(maintainedfragments->overqueue)
  {
    if(overallqueuelementsgeneric(maintainedfragments->lastelements,
                                  combinewithcurrentfragmentinqueue,
                                  &currentfraginfo) != 0)
    {
      return (Sint) -1;
    }
  } else
  {
    Diagonalrange diagonalrange;
    Sint diag;

    diag = FRAGDIAGONAL(newfragment);
    diagonalrange.lowerdiagonal = diag - maxdistance;
    diagonalrange.upperdiagonal = diag + maxdistance;
    if(redblacktreewalkrange (maintainedfragments->dictroot,
                              applyredblackwalkrange,
                              &currentfraginfo,
                              greaterequallowerdiag,
                              lowerequalupperdiag,
                              &diagonalrange) != 0)
    {
      return (Sint) -1;
    }
  }
  updatecurrentfrag(newfragment,&currentfraginfo.best);
  enqueuegeneric(maintainedfragments->lastelements,newfragment);
  if(!maintainedfragments->overqueue)
  {
    (void) redblacktreesearch ((Keytype) newfragment,
                               &nodecreated,
                               &maintainedfragments->dictroot,
                               comparediagonals,
                               NULL);
    if(!nodecreated)
    {
      ERROR0("newfragment was not inserted");
      return (Sint) -3;
    }
  }
  return 0;
}

Sint wrapmaintainedfragments(Maintainedfragments *maintainedfragments,
                             Outflychain outputchain,
                             void *outinfo)
{
  Onflyfragment *head;

  while(!queueisemptygeneric(maintainedfragments->lastelements))
  {
    head = dequeuegeneric(maintainedfragments->lastelements);
    STOREINARRAY(&maintainedfragments->readyforoutput,
                 Onflyfragmentptr,32,head);
  }
  if(outputallstackedelements(&maintainedfragments->readyforoutput,
                              outputchain,
                              outinfo) != 0)
  {
    return (Sint) -1;
  }
  if(!maintainedfragments->overqueue)
  {
    redblacktreedestroy(False,NULL,NULL,maintainedfragments->dictroot);
  }
  FREEARRAY(&maintainedfragments->readyforoutput,Onflyfragmentptr);
  free(maintainedfragments->lastelements);
  return 0;
}

#ifdef DEBUG

static Sint chainingofmatches2(BOOL chainqhits,
                               Uint maxdistance,
                               Onflyfragment *fragmentstore,
                               Uint numoffragments,
                               Outflychain outputchain,
                               void *outputinfo)
{
  Maintainedfragments maintainedfragments;
  Onflyfragment *rightfrag;

  initmaintainedfragments(&maintainedfragments,False);
  for(rightfrag = fragmentstore;
      rightfrag < fragmentstore + numoffragments; 
      rightfrag++)
  {
    if(processnewquhit(chainqhits,
                       maxdistance,
                       &maintainedfragments,
                       rightfrag->fraglength,
                       rightfrag->startpos[0],
                       rightfrag->startpos[1],
                       outputchain,
                       outputinfo) != 0)
    {
      return (Sint) -1;
    }
  }
  if(wrapmaintainedfragments(&maintainedfragments,
                             outputchain,
                             outputinfo) != 0)
  {
    return (Sint) -2;
  }
  return 0;
}

static Onflyfragment *makefragmentstorecopy(Onflyfragment *fragmentstore,
                                            Uint numoffragments)
{
  Uint i;
  Onflyfragment *copyfragmentstore;

  ALLOCASSIGNSPACE(copyfragmentstore,NULL,Onflyfragment,numoffragments);
  for(i=0; i<numoffragments; i++)
  {
    copyfragmentstore[i] = fragmentstore[i];
  }
  return copyfragmentstore;
}

#define MAXCHAINEDFRAGMENTS UintConst(5000)

Sint chainingofmatches(BOOL chainqhits,
                       Uint maxdistance,
                       Onflyfragment *fragmentstore,
                       Uint numoffragments)
{
  Onflyfragment *brutestore = NULL;

  if(numoffragments <= MAXCHAINEDFRAGMENTS)
  {
    brutestore = makefragmentstorecopy(fragmentstore,numoffragments);
  }
  printf("# run chainingofmatches1\n");
  chainingofmatches1(chainqhits,
                     maxdistance,
                     fragmentstore,
                     numoffragments);
  DEBUGCODE(2,
  if(showallfragments(fragmentstore,numoffragments) != 0)
  {
    return (Sint) -1;
  });
  if(numoffragments <= MAXCHAINEDFRAGMENTS)
  {
    printf("# run bruteforcechainingofmatches\n");
    bruteforcechainingofmatches(chainqhits,
                                maxdistance,
                                brutestore,
                                numoffragments);
    printf("# check bruteforce\n");
    comparefragmentstores(brutestore,fragmentstore,numoffragments);
    FREESPACE(brutestore);
  }
  printf("# run chainingofmatches2\n");
  if(chainingofmatches2(chainqhits,
                 maxdistance,
                 fragmentstore,
                 numoffragments,
                 checkflychain,
                 fragmentstore) != 0)
  {
    return (Sint) -2;
  }
  return 0;
}

#endif
