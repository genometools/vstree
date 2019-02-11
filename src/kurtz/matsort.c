
//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "types.h"
#include "spacedef.h"
#include "errordef.h"
#include "debugdef.h"
#include "match.h"

#define NUMSORTMODES\
        ((Uint) (sizeof(comparefunctions)/sizeof(comparefunctions[0])))

/*
  The following macro generates a function returning a non-negative value
  if the component \texttt{StoreP} of the structure referenced by \texttt{p}
  is equal or larger than the same component of the structure referenced
  by \texttt{q}.
*/

#define SORTFUNASCEND(P)\
        static Qsortcomparereturntype P##ascend(StoreMatch *p,StoreMatch *q)\
        {\
          if(p->Store##P == q->Store##P)\
          {\
            return 0;\
          }\
          return (p->Store##P > q->Store##P) ? 1 : -1;\
        }

/*
  The following type stores a comparefunction for the corresponding sort
  mode.
*/

typedef struct 
{
  char *sortmode, 
       *help;
  Qsortcomparereturntype(*compare)(StoreMatch *,StoreMatch *);
} Cmpfunction;

/*
  The following macros generates a function returning a non-negative value
  if the component \texttt{StoreP} of the structure referenced by \texttt{p}
  is equal or smaller than the same component of the structure referenced
  by \texttt{q}.
*/

#define SORTFUNDESCEND(P)\
        static Qsortcomparereturntype P##descend(StoreMatch *p,StoreMatch *q)\
        {\
          if(p->Store##P == q->Store##P)\
          {\
            return 0;\
          }\
          return (p->Store##P > q->Store##P) ? -1 : 1;\
        }


/*
  We define functions for comparing the components 
  \texttt{length1}, \texttt{length2}, 
  \texttt{position2}, \texttt{Evalue}, \texttt{score}, and \texttt{identity}.
*/

 SORTFUNASCEND(length1)
 SORTFUNDESCEND(length1)

 SORTFUNASCEND(position1)
 SORTFUNDESCEND(position1)

 SORTFUNASCEND(position2)
 SORTFUNDESCEND(position2)

 SORTFUNASCEND(Evalue)
 SORTFUNDESCEND(Evalue)

static Qsortcomparereturntype cmpScoregeneric(BOOL ascend,
                                              StoreMatch *p,StoreMatch *q)
{
  Sint scorep = DISTANCE2SCORE(p), 
       scoreq = DISTANCE2SCORE(q); 
  if(scorep == scoreq)
  {
    return 0;
  }
  if(scorep < 0)
  {
    scorep = ABS(scorep);
  }
  if(scoreq < 0)
  {
    scoreq = ABS(scoreq);
  }
  if(ascend)
  {
    return (scorep > scoreq) ? 1 : -1;
  } else
  {
    return (scorep > scoreq) ? -1 : 1;
  }
}

static Qsortcomparereturntype Scoreascend(StoreMatch *p,StoreMatch *q)
{
  return cmpScoregeneric(True,p,q);
}

static Qsortcomparereturntype Scoredescend(StoreMatch *p,StoreMatch *q)
{
  return cmpScoregeneric(False,p,q);
}

static Qsortcomparereturntype cmpIdentitygeneric(BOOL ascend,
                                                 StoreMatch *p,StoreMatch *q)
{
  double identityp, identityq;

  IDENTITY(identityp,p);
  IDENTITY(identityq,q);
  if(identityp == identityq)
  {
    return 0;
  }
  if(identityp < 0.0)
  {
    identityp = -identityp;
  }
  if(identityq < 0.0)
  {
    identityq = -identityq;
  }
  if(ascend)
  {
    return (identityp > identityq) ? 1 : -1;
  } else
  {
    return (identityp > identityq) ? -1 : 1;
  }
}

static Qsortcomparereturntype Identityascend(StoreMatch *p,StoreMatch *q)
{
  return cmpIdentitygeneric(True,p,q);
}

static Qsortcomparereturntype Identitydescend(StoreMatch *p,StoreMatch *q)
{
  return cmpIdentitygeneric(False,p,q);
}

//}

/*
  And store them in a table. Do not change the order before updating
  all calls of sortallmatches accordingly.
*/

static Cmpfunction comparefunctions[] 
   = {{"la",  "ascending order of length", length1ascend},  // 0
      {"ld",  "descending order of length", length1descend},    // 1
      {"ia",  "ascending order of first position", position1ascend}, // 2
      {"id",  "descending order of first position", position1descend},  // 3
      {"ja",  "ascending order of second position", position2ascend},   // 4
      {"jd",  "descending order of second position", position2descend},  // 5
      {"ea",  "ascending order of Evalue", Evalueascend},      // 6
      {"ed",  "descending order of Evalue", Evaluedescend},     // 7
      {"sa",  "ascending order of score", Scoreascend},       // 8
      {"sd",  "descending order of score", Scoredescend},      // 9
      {"ida", "ascending order of identity", Identityascend},   // 10
      {"idd", "descending order of identity", Identitydescend}   // 11
     };

static char sortmodestring[5 * NUMSORTMODES];
static char sortmodehelp[80 * NUMSORTMODES];

/*EE
  The following function checks if the given sorting mode is possible
*/

Sint checksortmode(const char *arg)
{
  Uint i;

  for(i=0; i < NUMSORTMODES; i++)
  {
    if(strcmp(arg,comparefunctions[i].sortmode) == 0)
    {
      return (Sint) i;
    }
  }
  return (Sint) -1;
}

Uint undefsortmode(void)
{
  return (Uint) NUMSORTMODES;
}

char *concatsortmodes(void)
{
  Uint i;
  Sprintfreturntype start = 0;

  for(i=0; i < NUMSORTMODES; i++)
  {
    start += sprintf(sortmodestring+start,"%s",comparefunctions[i].sortmode);
    if(i < NUMSORTMODES - 1)
    {
      start += sprintf(sortmodestring+start,", ");
    }
  }
  return &sortmodestring[0];
}

char *concatsorthelp(void)
{
  Uint i;
  Sprintfreturntype start = 0;

  start += sprintf(sortmodehelp+start,
                   "sort the matches, additional argument is mode\n");

  for(i=0; i < NUMSORTMODES; i++)
  {
    start += sprintf(sortmodehelp+start,"%s: %s",comparefunctions[i].sortmode,
                                                 comparefunctions[i].help);
    if(i < NUMSORTMODES - 1)
    {
      start += sprintf(sortmodehelp+start,"\n");
    }
  }
  return &sortmodehelp[0];
}

/*EE
  The following function sorts an array of matches according to the 
  given sortmode.
*/

void sortallmatches(StoreMatch *storematchtab,
                    Uint numofmatches,
                    Uint sortmode)
{
  if(sortmode >= NUMSORTMODES)
  {
    fprintf(stderr,"sortmode = %lu not allowed\n",(Showuint) sortmode);
    exit(EXIT_FAILURE);
  }
  qsort(storematchtab,
        (size_t) numofmatches,
        sizeof(StoreMatch),
        (Qsortcomparefunction) 
	comparefunctions[sortmode].compare);
}

#define ELEM               Uint
#define QSORTHEADER        static void qsortStorematchgroup(\
                                       StoreMatch *storematch,ELEM *l,ELEM *r)
#define ACCESSELEM(A)      (storematch[*(A)].Storeseqnum2)
#define ELEMGREATER(A,B)   (ACCESSELEM(A) >  ACCESSELEM(B))
#define ELEMGREATEREQ(A,B) (ACCESSELEM(A) >= ACCESSELEM(B))

#include "qsort.gen"

#ifdef DEBUG
static void checkgroupedmatches(Uint *sortedpermutation,
                                StoreMatch *storematchtab,
                                Uint numofmatches)
{
  Uint *pptr;

  for(pptr = sortedpermutation; 
      pptr < sortedpermutation + numofmatches - 1; 
      pptr++)
  {
    if(storematchtab[*pptr].Storeseqnum1 >
       storematchtab[*(pptr+1)].Storeseqnum1)
    {
      fprintf(stderr,"storematchtab[%lu].seqnum1 = %lu > "
                     "%lu = storematchtab[%lu].seqnum1\n",
            (Showuint) (pptr - sortedpermutation),
            (Showuint) storematchtab[*pptr].Storeseqnum1,
            (Showuint) storematchtab[*(pptr+1)].Storeseqnum1,
            (Showuint) (pptr - sortedpermutation + 1));
      exit(EXIT_FAILURE);
    }
    if(storematchtab[*pptr].Storeseqnum1 ==
       storematchtab[*(pptr+1)].Storeseqnum1 &&
       storematchtab[*pptr].Storeseqnum2 >
       storematchtab[*(pptr+1)].Storeseqnum2)
    {
      fprintf(stderr,"storematchtab[%lu].seqnum1 = %lu identical, but "
                     "storematchtab[%lu].seqnum2 = %lu > "
                     "%lu = storematchtab[%lu].seqnum2\n",
            (Showuint) (pptr - sortedpermutation),
            (Showuint) storematchtab[*pptr].Storeseqnum1,
            (Showuint) (pptr - sortedpermutation),
            (Showuint) storematchtab[*pptr].Storeseqnum2,
            (Showuint) storematchtab[*(pptr+1)].Storeseqnum2,
            (Showuint) (pptr - sortedpermutation + 1));
      exit(EXIT_FAILURE);
    }
  }
}
#endif

void groupmatchesbyseqnum(Uint *sortedpermutation,
                          StoreMatch *storematchtab,
                          Uint numofmatches,
                          Uint numofsequences1)
{
  Uint *sequencescounter, i, lastgroupstart;
  StoreMatch *mptr;

  ALLOCASSIGNSPACE(sequencescounter,NULL,Uint,numofsequences1);
  for(i=0; i<numofsequences1; i++)
  {
    sequencescounter[i] = 0;
  }
  for(mptr=storematchtab; mptr<storematchtab + numofmatches; mptr++)
  {
    sequencescounter[mptr->Storeseqnum1]++;
  }
  for(i=UintConst(1); i<numofsequences1; i++)
  {
    sequencescounter[i] += sequencescounter[i-1];
  }
  for(mptr=storematchtab+numofmatches-1; mptr >= storematchtab; mptr--)
  {
    sortedpermutation[--sequencescounter[mptr->Storeseqnum1]] 
      = (Uint) (mptr-storematchtab);
  }
  lastgroupstart = 0;
  for(i=UintConst(1); i<numofmatches; i++)
  {
    if(storematchtab[sortedpermutation[i-1]].Storeseqnum1 !=
       storematchtab[sortedpermutation[i]].Storeseqnum1)
    {
      if(lastgroupstart < i-1)
      {
        qsortStorematchgroup(storematchtab,
                             sortedpermutation+lastgroupstart,
                             sortedpermutation+i-1);
      }
      lastgroupstart = i;
    }
  }
  if(lastgroupstart < numofmatches-1)
  {
    qsortStorematchgroup(storematchtab,
                         sortedpermutation+lastgroupstart,
                         sortedpermutation+numofmatches-1);
  }
#ifdef DEBUG
  checkgroupedmatches(sortedpermutation,storematchtab,numofmatches);
#endif
  FREESPACE(sequencescounter);
}

#undef ELEM
#undef QSORTHEADER
#undef ACCESSELEM
#undef ELEMGREATER
#undef ELEMGREATEREQ

static Sint comparediagonals(StoreMatch *mptra,StoreMatch *mptrb)
{
  if(mptra->idnumber != mptrb->idnumber)
  {
    Sint diaga, diagb;

    diaga = (Sint) mptra->Storeposition2 - (Sint) mptra->Storeposition1;
    diagb = (Sint) mptrb->Storeposition2 - (Sint) mptrb->Storeposition1;
    if(diaga < diagb)
    {
      return (Sint) 1;
    }
    if(diaga > diagb)
    {
      return (Sint) -1;
    }
    if(mptra->Storeposition2 < mptrb->Storeposition2)
    {
      return (Sint) -1;
    }
    if(mptra->Storeposition2 > mptrb->Storeposition2)
    {
      return (Sint) 1;
    }
    fprintf(stderr,"uncomparable matches (%lu,%lu) and (%lu,%lu)\n",
                    (Showuint) mptra->Storeposition1,
                    (Showuint) mptra->Storeposition2,
                    (Showuint) mptrb->Storeposition1,
                    (Showuint) mptrb->Storeposition2);
    exit(EXIT_FAILURE);
  }
  return 0;
}

#define ELEM               StoreMatch
#define QSORTHEADER        void qsortStorematchbydiagonal(ELEM *l,ELEM *r)
#define ELEMGREATER(A,B)   (comparediagonals(A,B) > 0)
#define ELEMGREATEREQ(A,B) (comparediagonals(A,B) >= 0)

#include "qsort.gen"
