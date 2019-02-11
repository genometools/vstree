
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "alphadef.h"
#include "virtualdef.h"
#include "chardef.h"
#include "genfile.h"
#include "vnodedef.h"
#include "fhandledef.h"
#include "debugdef.h"
#include "spacedef.h"
#include "scoredef.h"

#include "scorematrix.pr"
#include "readvirt.pr"
#include "filehandle.pr"
#include "binsplitvn.pr"

/*
  Compute the lcp values on the fly without resorting to large values.
  Add option to compute the best scores.
*/

typedef Sint ProfScore;
#define SHOWProfScore(FP,VAL) fprintf(FP,"%ld",(Showsint) (VAL))

/*
typedef double ProfScore;
#define SHOWProfScore(FP,VAL) fprintf(FP,"%.2f",(double) (VAL))
*/

#define GETPROFSCORE(S,I,J)\
        (S)->scoretab[(S)->multtab[(Sint) I] + (Sint) (J)]

#define MAXNUMOFMATCHES UintConst(1000)

#define RUNSIMPLESEARCH    (UintConst(1))
#define RUNLOOKAHEADSEARCH (UintConst(1) << 1)
#define RUNARRAYSEARCH     (UintConst(1) << 2)
#define RUNARRAYBFSSEARCH  (UintConst(1) << 3)
#define RUNARRAYDFSSEARCH  (UintConst(1) << 4)

#define USAGE\
        fprintf(stderr,"Usage: %s SLABD <profilelength> <numofprofiles> "\
                       "<maxrandscore> <index>\n",argv[0]);\
        return EXIT_FAILURE

#ifdef DEBUG
#define CHECKPREFIXSCORES\
        DEBUGCODE(2,checkprefixscores(prof,\
                                      vptr,\
                                      dvalue,\
                                      prefixscorebrute,\
                                      prefixscore,\
                                      lvalue));
#else
#define CHECKPREFIXSCORES /* Nothing */
#endif

#define DOUBLERUNCHAR\
        fprintf(stderr,"character %c already specified\n",argstring[i]);\
        exit(EXIT_FAILURE);

typedef struct
{
  Uint dimension,
       numofcharacters,
       multtab[UCHAR_MAX+1];            // contains multiples of mapsize
  ProfScore *itmthreshold,              // intermediate thresholds
            maxtotalscore,              // the minimal score
            mintotalscore,              // the maximal score
            *scoretab;                  // scores
} Profilematrix;

#ifdef DEBUG

#define COMPAREFUNCTION       compareProfScores
#define COMPARETYPE           ProfScore
#define COMPAREFORMAT(FP,VAL) SHOWProfScore(FP,VAL)

#include "cmp-gentab.c"

static void compareResults(char *tag,ArrayUint *results1,ArrayUint *results2)
{
  if(results1->nextfreeUint != results2->nextfreeUint)
  {
    fprintf(stderr,"len(%s) %lu != %lu\n",
            tag,(Showuint) results1->nextfreeUint,
                (Showuint) results2->nextfreeUint);
    exit(EXIT_FAILURE);
  }
  if(results1->nextfreeUint > 0)
  {
    NOTSUPPOSEDTOBENULL(results1->spaceUint);
    qsortUint(results1->spaceUint,
              results1->spaceUint + results1->nextfreeUint - 1);
    NOTSUPPOSEDTOBENULL(results2->spaceUint);
    qsortUint(results2->spaceUint,
              results2->spaceUint + results2->nextfreeUint - 1);
  }
  compareUinttab(tag,
                 results1->spaceUint,
                 results2->spaceUint,
                 results1->nextfreeUint);
}

static void showPSSMmatches(ArrayUint *tab)
{
  Uint i;

  for(i=0; i<tab->nextfreeUint; i++)
  {
    printf("sol[%lu]=%lu\n",(Showuint) i,(Showuint) tab->spaceUint[i]);
  }
}

static void showthresholds(ProfScore *itmthreshold,Uint dimension)
{
  Uint i;

  for(i=0; i<dimension; i++)
  {
    printf("itmthreshold[%lu]=",(Showuint) i);
    SHOWProfScore(stdout,itmthreshold[i]);
    printf("\n");
  }
}

static void showProfilematrix(Profilematrix *prof,
                              Uchar *characters)
{
  Uint d, a;

  printf("# %lu x %lu matrix\n",(Showuint) prof->numofcharacters,
                                (Showuint) prof->dimension);
  printf("# mintotalscore=");
  SHOWProfScore(stdout,prof->mintotalscore);
  printf("\n");
  printf("# maxtotalscore=");
  SHOWProfScore(stdout,prof->maxtotalscore);
  printf("\n");
  printf("   ");
  for(a = 0; a < prof->numofcharacters; a++)
  {
    printf("%c",characters[a]);
    printf("%s",(a < prof->numofcharacters - 1) ? "   " : "\n");
  }
  for(d = 0; d < prof->dimension; d++)
  {
    for(a = 0; a < prof->numofcharacters; a++)
    {
      SHOWProfScore(stdout,GETPROFSCORE(prof,a,d));
      printf("%s",(a < prof->numofcharacters - 1) ? " " : " \n");
    } 
  } 
}

#endif

static void simplesearchPSSM(ArrayUint *results,
                             Uchar *dbseq,
                             Uint dblen,
                             Profilematrix *prof,
                             ProfScore minscore)
{
  Uint d;
  Uchar currentchar, *dbseqptr;
  ProfScore score;
 
  results->nextfreeUint = 0;
  for(dbseqptr=dbseq; dbseqptr<=dbseq+dblen-prof->dimension; dbseqptr++)
  {
    score = (ProfScore) 0;
    for(d=0; d<prof->dimension; d++)
    {
      currentchar = *(dbseqptr+d);
      if(ISSPECIAL(currentchar))
      {
        break;
      }
      score += GETPROFSCORE(prof,currentchar,d);
    }
    if(d == prof->dimension && score >= minscore)
    {
      STOREINARRAY(results,Uint,512,(Uint) (dbseqptr-dbseq));
    }
  }
}

static void makeitmthresholds(Profilematrix *prof,
                              ProfScore minscore)
{
  Uint d, a;
  Sint ddown;
  ProfScore partsum, 
            score, 
            *maxscore;

  ALLOCASSIGNSPACE(maxscore,NULL,ProfScore,prof->dimension);
  for(d=0; d<prof->dimension; d++)
  {
    for(a=0; a<prof->numofcharacters; a++)
    {
      score = GETPROFSCORE(prof,a,d);
      if(a == 0 || maxscore[d] < score)
      {
        maxscore[d] = score;
      }
    }
  }
  partsum = (ProfScore) 0;
  NOTSUPPOSEDTOBENULL(prof->itmthreshold);
  for(ddown = (Sint) (prof->dimension-1); ddown>=0; ddown--)
  {
    prof->itmthreshold[ddown] = minscore - partsum;
    partsum += maxscore[ddown];
  }
  FREESPACE(maxscore);
}

static void lookaheadsearchPSSM(ArrayUint *results,
                                Uchar *dbseq,
                                Uint dblen,
                                Profilematrix *prof,
                                ProfScore minscore)
{
  Uint d;
  Uchar currentchar, *dbseqptr;
  ProfScore score;
 
  results->nextfreeUint = 0;
  for(dbseqptr=dbseq; dbseqptr<=dbseq+dblen-prof->dimension; dbseqptr++)
  {
    score = (ProfScore) 0;
    for(d=0; d<prof->dimension; d++)
    {
      currentchar = *(dbseqptr+d);
      if(ISSPECIAL(currentchar))
      {
        break;
      }
      score += GETPROFSCORE(prof,currentchar,d);
      if(score < prof->itmthreshold[d])
      {
        break;
      }
    }
    if(d == prof->dimension && score >= minscore)
    {
      STOREINARRAY(results,Uint,512,(Uint) (dbseqptr-dbseq));
    }
  }
}

static Sint evaluateremaining(Profilematrix *prof,
                              Uchar *vptr,
                              ProfScore *prefixscore,
                              Uint startindex,
                              Uint lvalue)
{
  Sint d;
  ProfScore score;
  Uchar currentchar;

  if(startindex > lvalue)
  {
    return (Sint) lvalue;
  }
  if(startindex == 0)
  {
    score = (ProfScore) 0;
  } else
  {
    score = prefixscore[startindex-1];
  }
  for(d=(Sint) startindex; d<=(Sint) lvalue; d++)
  {
    currentchar = vptr[d];
    if(ISSPECIAL(currentchar))
    {
      DEBUG1(3,"evaluateremaining returns %ld\n",(Showsint) (d-1));
      return d-1;
    }
    score += GETPROFSCORE(prof,currentchar,d);
    if(score < prof->itmthreshold[d])
    {
      DEBUG1(3,"evaluateremaining returns %ld\n",(Showsint) (d-1));
      return d-1;
    }
    prefixscore[d] = score;
  }
  DEBUG1(3,"evaluateremaining return %ld\n",(Showsint) lvalue);
  return (Sint) lvalue;
}

#ifdef DEBUG
static void checkprefixscores(Profilematrix *prof,
                              Uchar *vptr,
                              Sint dvalue,
                              ProfScore *prefixscorebrute,
                              ProfScore *prefixscore,
                              Uint lvalue)
{
  Sint dvaluebrute;

  dvaluebrute = evaluateremaining(prof,
                                  vptr,
                                  prefixscorebrute,
                                  0,
                                  lvalue);
  if(dvaluebrute != dvalue)
  {
    fprintf(stderr,"len(%s) %ld != %ld\n",
                   "dvalue",(Showsint) dvaluebrute,
                            (Showsint) dvalue);
    exit(EXIT_FAILURE);
  }
  if(dvalue >= 0)
  {
    compareProfScores("prefixscore",
                      prefixscorebrute,
                      prefixscore,
                      (Uint) (dvalue+1));
  }
}
#endif

#undef CUTSKIPTAB
#ifdef CUTSKIPTAB
static void cutskiptab(Uchar *skiptab1,Uint *skiptab,Uint len)
{
  Uint value, i;

  for(i=0; i<len; i++)
  {
    value = skiptab[i] + 1 - i;
    if(value > UCHAR_MAX)
    {
      skiptab1[i] = UCHAR_MAX;
    } else
    {
      skiptab1[i] = (Uchar) value;
    }
  }
}
#endif

#define EVALLCPCUT(VIRT,VAR,IND)\
        VAR = (VIRT)->lcptab[IND]

static void arraysearchPSSM(ArrayUint *results,
                            Virtualtree *virtualtree,
                            Profilematrix *prof,
                            /*@unused@*/ Uchar *skiptab1)
{
  Sint dvalue;
  Uint idx, vlen, lvalue, lcpvalue, current;
  Uchar *vptr;
  ProfScore *prefixscore;
  DEBUGDECL(ProfScore *prefixscorebrute);
 
#ifdef DEBUG
  ALLOCASSIGNSPACE(prefixscorebrute,NULL,ProfScore,prof->dimension);
#endif
  ALLOCASSIGNSPACE(prefixscore,NULL,ProfScore,prof->dimension);
  DEBUGCODE(2,showthresholds(prof->itmthreshold,prof->dimension));
  vptr = virtualtree->multiseq.sequence + virtualtree->suftab[0];
  vlen = virtualtree->multiseq.totallength - virtualtree->suftab[0];
  if(vlen < prof->dimension)
  {
    lvalue = vlen-1;
  } else
  {
    lvalue = prof->dimension-1;
  }
  dvalue = evaluateremaining(prof,
                             vptr,
                             prefixscore,
                             0,
                             lvalue);
  CHECKPREFIXSCORES;
  if(dvalue == (Sint) (prof->dimension - 1))
  {
    STOREINARRAY(results,Uint,512,virtualtree->suftab[0]);
  }
  idx=UintConst(1);
  while(idx < virtualtree->multiseq.totallength)
  {
    vptr = virtualtree->multiseq.sequence + virtualtree->suftab[idx];
    vlen = virtualtree->multiseq.totallength - virtualtree->suftab[idx];
    if(vlen < prof->dimension)
    {
      lvalue = vlen-1;
    } else
    {
      lvalue = prof->dimension-1;
    }
    EVALLCPCUT(virtualtree,lcpvalue,idx);
    if(dvalue + 1 >= (Sint) lcpvalue)
    {
      dvalue = evaluateremaining(prof,
                                 vptr,
                                 prefixscore,
                                 lcpvalue,
                                 lvalue);
      CHECKPREFIXSCORES;
      if(dvalue == (Sint) (prof->dimension - 1))
      {
        STOREINARRAY(results,Uint,512,virtualtree->suftab[idx]);
      }
      idx++;
    } else
    {
      current = idx;
      while(True)
      {
#ifdef CUTSKIPTAB
        idx += skiptab1[idx];
#else
        idx = virtualtree->skiptab[idx]+1;
#endif
        if(idx >= virtualtree->multiseq.totallength)
        {
          break;
        }
        EVALLCPCUT(virtualtree,lcpvalue,idx);
        if(dvalue + 1 >= (Sint) lcpvalue)
        {
          break;
        }
      }
      if(dvalue == (Sint) (prof->dimension - 1))
      {
        while(current < idx)
        {
          STOREINARRAY(results,Uint,512,virtualtree->suftab[current]);
          current++;
        }
      }
    }
  }
#ifdef DEBUG
  FREESPACE(prefixscorebrute);
#endif
  FREESPACE(prefixscore);
}

typedef struct
{
  Uint offset, left, right;
  ProfScore score;
} Scoredvnode;

static BOOL evaluateedge(Virtualtree *virtualtree,
                         Profilematrix *prof,
                         ArrayUint *results,
                         Scoredvnode *newnode,
                         Uint l,Uint r,
                         Uint startoffset,
                         ProfScore score)
{
  Uchar *vptr;
  Uint d, endoffset;

  vptr = virtualtree->multiseq.sequence + virtualtree->suftab[l];
  if(l == r)
  {
    endoffset = virtualtree->multiseq.totallength - virtualtree->suftab[l];
  } else
  {
    endoffset = binlcpvalue(&virtualtree->multiseq,
                            virtualtree->suftab,
                            startoffset,l,r);
  }
  if(endoffset > prof->dimension)
  {
    endoffset = prof->dimension;
  }
  for(d=startoffset; d<endoffset; d++)
  {
    if(ISSPECIAL(vptr[d]))
    {
      return False;
    }
    score += GETPROFSCORE(prof,vptr[d],d);
    if(score < prof->itmthreshold[d])
    {
      return False;
    }
  }
  if(l == r)
  {
    if(d == prof->dimension)
    {
      STOREINARRAY(results,Uint,128,virtualtree->suftab[l]);
    } 
    return False;
  } 
  if(d == prof->dimension)
  {
    Uint *sufptr;

    for(sufptr = virtualtree->suftab + l;
        sufptr <= virtualtree->suftab + r; sufptr++)
    {
      STOREINARRAY(results,Uint,128,*sufptr);
    }
    return False;
  }
  newnode->score = score;
  newnode->offset = endoffset;
  newnode->left = l;
  newnode->right = r;
  return True;
}

typedef Scoredvnode Queueelem;

#include "queue.c"

static void arraysearchbfsPSSM(ArrayUint *results,
                               Virtualtree *virtualtree,
                               Profilematrix *prof)
{
  Queue queue;
  Vbound *vbounds, *vboundsptr;
  Scoredvnode scvnode, scvnodenew;
  Uint l, r, vboundmaxsize, vboundscount, countprocessed = 0;
  
  emptyqueue(&queue,UintConst(1024));
  scvnode.score = (ProfScore) 0;
  scvnode.offset = 0;
  scvnode.left = 0;
  scvnode.right = virtualtree->multiseq.totallength;
  enqueue(&queue,scvnode);
  vboundmaxsize = virtualtree->alpha.mapsize-1+1;
  ALLOCASSIGNSPACE(vbounds,NULL,Vbound,vboundmaxsize);
  while(!queueisempty(&queue))
  {
    countprocessed++;
    scvnode = dequeue(&queue);
    //printf("dequeue(offset=%lu)\n",(Showuint) vnode.offset);
    vboundscount = splitnodewithcharbinwithoutspecial(&virtualtree->multiseq,
                                                      virtualtree->suftab,
                                                      vbounds,
                                                      vboundmaxsize,
                                                      scvnode.offset,
                                                      scvnode.left,
                                                      scvnode.right);
    for(vboundsptr = vbounds; vboundsptr < vbounds + vboundscount; 
        vboundsptr++)
    {
      l = vboundsptr->bound;
      r = (vboundsptr+1)->bound-1;
      if(ISSPECIAL(vboundsptr->inchar))
      {
        NOTSUPPOSED;
      }
      DEBUG4(2,"%lu-(%lu,%lu)%c",
                (Showuint) vboundsptr->inchar,(Showuint) l,
                (Showuint) r,(vboundsptr == vbounds + vboundscount - 1) 
                              ? '\n' 
                              : ' ');
      if(l == r)
      {
        (void) evaluateedge(virtualtree,
                            prof,
                            results,
                            NULL,
                            l,r,
                            scvnode.offset,
                            scvnode.score);
      } else
      {
        if(evaluateedge(virtualtree,
                        prof,
                        results,
                        &scvnodenew,
                        l,r,
                        scvnode.offset,
                        scvnode.score))
        {
          enqueue(&queue,scvnodenew);
        } 
      }
    }
  }
  DEBUG1(2,"# processed intervals: %lu\n",(Showuint) countprocessed);
  wrapqueue(&queue);
  FREESPACE(vbounds);
}

typedef Scoredvnode Stackelem;

#include "gstack.c"

static void arraysearchdfsPSSM(ArrayUint *results,
                               Virtualtree *virtualtree,
                               Profilematrix *prof)
{
  Genericstack gstack;
  Vbound *vbounds, *vboundsptr;
  Scoredvnode scvnode, scvnodenew;
  Uint l, r, vboundmaxsize, vboundscount, countprocessed = 0;
  
  emptygenericStack(&gstack,UintConst(1024));
  scvnode.score = (ProfScore) 0;
  scvnode.offset = 0;
  scvnode.left = 0;
  scvnode.right = virtualtree->multiseq.totallength;
  pushGenericstack(&gstack,scvnode);
  vboundmaxsize = virtualtree->alpha.mapsize-1+1;
  ALLOCASSIGNSPACE(vbounds,NULL,Vbound,vboundmaxsize);
  while(!stackisempty(&gstack))
  {
    countprocessed++;
    scvnode = popGenericstack(&gstack);
    vboundscount = splitnodewithcharbinwithoutspecial(&virtualtree->multiseq,
                                                      virtualtree->suftab,
                                                      vbounds,
                                                      vboundmaxsize,
                                                      scvnode.offset,
                                                      scvnode.left,
                                                      scvnode.right);
    for(vboundsptr = vbounds + vboundscount -1; 
        vboundsptr >= vbounds;
        vboundsptr--)
    {
      l = vboundsptr->bound;
      r = (vboundsptr+1)->bound-1;
      if(ISSPECIAL(vboundsptr->inchar))
      {
        NOTSUPPOSED;
      }
      DEBUG4(2,"%lu-(%lu,%lu)%c",
                (Showuint) vboundsptr->inchar,(Showuint) l,
                (Showuint) r,(vboundsptr == vbounds + vboundscount - 1) 
                              ? '\n' 
                              : ' ');
      if(l == r)
      {
        (void) evaluateedge(virtualtree,
                            prof,
                            results,
                            NULL,
                            l,r,
                            scvnode.offset,
                            scvnode.score);
      } else
      {
        if(evaluateedge(virtualtree,
                        prof,
                        results,
                        &scvnodenew,
                        l,r,
                        scvnode.offset,
                        scvnode.score))
        {
          pushGenericstack(&gstack,scvnodenew);
        } 
      }
    }
  }
  DEBUG1(2,"# processed intervals: %lu\n",(Showuint) countprocessed);
  wrapGenericstack(&gstack);
  FREESPACE(vbounds);
}

static void generateprofile(Profilematrix *prof,
                            ProfScore maxrandscore)
{
  Uint a, d;
  ProfScore score, 
            minlinescore = (ProfScore) 0, 
            maxlinescore = (ProfScore) 0;

  prof->maxtotalscore = prof->mintotalscore = (ProfScore) 0;
  DEBUG2(1,"dimension=%lu,numofcharacters=%lu\n",
             (Showuint) prof->dimension,
             (Showuint) prof->numofcharacters);
  for(d=0; d<prof->dimension; d++)
  {
    for(a=0; a<prof->numofcharacters; a++)
    {
      score = (ProfScore) (drand48() * (double) maxrandscore);
      if(a == 0)
      {
        minlinescore = maxlinescore = score;
      } else
      {
        if(minlinescore > score)
        {
          minlinescore = score;
        }
        if(maxlinescore < score)
        {
          maxlinescore = score;
        }
      }
      GETPROFSCORE(prof,a,d) = score;
    }
    prof->maxtotalscore += maxlinescore;
    prof->mintotalscore += minlinescore;
  }
  DEBUG1(1,"maxtotalscore=%ld\n",(Showsint) prof->maxtotalscore);
  DEBUG1(1,"mintotalscore=%ld\n",(Showsint) prof->mintotalscore);
}

static void iterateprofilematching(Uint programs2run,
                                   Virtualtree *virtualtree,
                                   Uint numofprofiles,
                                   Profilematrix *prof,
                                   ProfScore maxrandscore,
                                   Uchar *characters)
{
  Uint idx, numofmatches = 0, comparisons = 0;
  ProfScore minscore;
  ArrayUint sresults, lresults, aresults, bresults, dresults;
#ifdef CUTSKIPTAB
  Uchar *skiptab1;
  ALLOCASSIGNSPACE(skiptab1,NULL,Uchar,virtualtree->multiseq.totallength);
  cutskiptab(skiptab1,virtualtree->skiptab,virtualtree->multiseq.totallength);
#endif
  
  ALLOCASSIGNSPACE(prof->itmthreshold,NULL,ProfScore,prof->dimension);
  INITARRAY(&sresults,Uint);
  INITARRAY(&lresults,Uint);
  INITARRAY(&aresults,Uint);
  INITARRAY(&bresults,Uint);
  INITARRAY(&dresults,Uint);
  if(programs2run == 0)
  {
    fprintf(stderr,"No program to run\n");
    exit(EXIT_FAILURE);
  }
  for(idx=0; idx<numofprofiles; idx++)
  {
    generateprofile(prof,maxrandscore);
    DEBUGCODE(1,showProfilematrix(prof,characters));
    minscore = prof->maxtotalscore;
    while(minscore >= (ProfScore) 0)
    {
      lresults.nextfreeUint = 0;
      sresults.nextfreeUint = 0;
      aresults.nextfreeUint = 0;
      bresults.nextfreeUint = 0;
      dresults.nextfreeUint = 0;
      if(programs2run & RUNSIMPLESEARCH)
      {
        simplesearchPSSM(&sresults,
                         virtualtree->multiseq.sequence,
                         virtualtree->multiseq.totallength,
                         prof,
                         minscore);
        numofmatches = sresults.nextfreeUint;
        DEBUGCODE(3,showPSSMmatches(&sresults));
      }
      makeitmthresholds(prof, minscore);
      if(programs2run & RUNLOOKAHEADSEARCH)
      {
        lookaheadsearchPSSM(&lresults,
                            virtualtree->multiseq.sequence,
                            virtualtree->multiseq.totallength,
                            prof,
                            minscore);
        numofmatches = lresults.nextfreeUint;
#ifdef DEBUG
        if(programs2run & RUNSIMPLESEARCH)
        {
          compareResults("s vs l",&sresults,&lresults);
        }
#endif
      }
      if(programs2run & RUNARRAYSEARCH)
      {
#ifdef CUTSKIPTAB
        arraysearchPSSM(&aresults,virtualtree,prof,skiptab1);
#else
        arraysearchPSSM(&aresults,virtualtree,prof,NULL);
#endif
        numofmatches = aresults.nextfreeUint;
#ifdef DEBUG
        if(programs2run & RUNSIMPLESEARCH)
        {
          compareResults("s vs a",&sresults,&aresults);
        } else
        { 
          if(programs2run & RUNLOOKAHEADSEARCH)
          {
            compareResults("l vs a",&lresults,&aresults);
          }
        } 
#endif
      }
      if(programs2run & RUNARRAYBFSSEARCH)
      {
        arraysearchbfsPSSM(&bresults,virtualtree,prof);
        numofmatches = bresults.nextfreeUint;
#ifdef DEBUG
        if(programs2run & RUNSIMPLESEARCH)
        {
          compareResults("s vs b",&sresults,&bresults);
        } else
        {
          if(programs2run & RUNLOOKAHEADSEARCH)
          {
            compareResults("l vs b",&lresults,&bresults);
          } else
          {
            if(programs2run & RUNARRAYSEARCH)
            {
              compareResults("a vs b",&aresults,&bresults);
            }
          }
        }
#endif
      }
      if(programs2run & RUNARRAYDFSSEARCH)
      {
        arraysearchdfsPSSM(&dresults,virtualtree,prof);
        numofmatches = dresults.nextfreeUint;
#ifdef DEBUG
        if(programs2run & RUNSIMPLESEARCH)
        {
          compareResults("s vs d",&sresults,&dresults);
        } else
        {
          if(programs2run & RUNLOOKAHEADSEARCH)
          {
            compareResults("l vs d",&lresults,&dresults);
          } else
          {
            if(programs2run & RUNARRAYSEARCH)
            {
              compareResults("a vs d",&aresults,&dresults);
            } else
            {
              if(programs2run & RUNARRAYBFSSEARCH)
              {
                compareResults("b vs d",&bresults,&dresults);
              }
            }
          }
        }
#endif
      }

      DEBUG2(2,"minscore = %ld, numofmatches = %lu\n",
              (Showsint) minscore,
              (Showuint) numofmatches);
      if(numofmatches >= MAXNUMOFMATCHES)
      {
        break;
      }
      lresults.nextfreeUint = 0;
      sresults.nextfreeUint = 0;
      aresults.nextfreeUint = 0;
      bresults.nextfreeUint = 0;
      dresults.nextfreeUint = 0;
      minscore--;
      comparisons++;
    }
  }
  FREEARRAY(&sresults,Uint);
  FREEARRAY(&lresults,Uint);
  FREEARRAY(&aresults,Uint);
  FREEARRAY(&bresults,Uint);
  FREEARRAY(&dresults,Uint);
  FREESPACE(prof->itmthreshold);
#ifdef CUTSKIPTAB
  FREESPACE(skiptab1);
#endif
  printf("number of comparisons: %lu\n",(Showuint) comparisons);
}

static Uint analyserunstring(const char *argstring)
{
  Uint i, programs2run = 0;

  for(i=0; argstring[i] != '\0'; i++)
  {
    switch(argstring[i])
    {
      case 'S': if(programs2run & RUNSIMPLESEARCH)
                {
                  DOUBLERUNCHAR;
                }
                programs2run |= RUNSIMPLESEARCH;
                break;
      case 'L': if(programs2run & RUNLOOKAHEADSEARCH)
                {
                  DOUBLERUNCHAR;
                }
                programs2run |= RUNLOOKAHEADSEARCH;
                break;
      case 'A': if(programs2run & RUNARRAYSEARCH)
                {
                  DOUBLERUNCHAR;
                }
                programs2run |= RUNARRAYSEARCH;
                break;
      case 'B': if(programs2run & RUNARRAYBFSSEARCH)
                {
                  DOUBLERUNCHAR;
                }
                programs2run |= RUNARRAYBFSSEARCH;
                break;
      case 'D': if(programs2run & RUNARRAYDFSSEARCH)
                {
                  DOUBLERUNCHAR;
                }
                programs2run |= RUNARRAYDFSSEARCH;
                break;
      default:  fprintf(stderr,"illegal character %c in first argument\n",
                         argstring[i]);
                exit(EXIT_FAILURE);
    }
  }
  return programs2run;
}

static void showverbose(char *s)
{
  printf("%s\n",s);
}

MAINFUNCTION
{
  Scaninteger readint;
  Uint numofprofiles, 
       demand = TISTAB | SUFTAB | LCPTAB | SKPTAB,
       programs2run = 0;
  Sint maxrandscore;
  Profilematrix prof;
  Virtualtree virtualtree;
  const char *indexname;

  DEBUGLEVELSET;
  if(argc != 6)
  {
    USAGE;
  }
  programs2run = analyserunstring(argv[1]);
  if(sscanf(argv[2],"%ld",&readint) != 1 || readint < (Scaninteger) 2)
  {
    USAGE;
  }
  prof.dimension = (Uint) readint;
  if(sscanf(argv[3],"%ld",&readint) != 1 || readint < (Scaninteger) 1)
  {
    USAGE;
  }
  numofprofiles = (Uint) readint;
  if(sscanf(argv[4],"%ld",&readint) != 1 || readint < (Scaninteger) 2)
  {
    USAGE;
  }
  demand = 0;
  if(programs2run & (RUNSIMPLESEARCH | RUNLOOKAHEADSEARCH))
  {
    demand |= TISTAB;
  }
  if(programs2run & RUNARRAYSEARCH)
  {
    demand |= (TISTAB | SUFTAB | LCPTAB | SKPTAB);
  } 
  if(programs2run & (RUNARRAYBFSSEARCH | RUNARRAYDFSSEARCH))
  {
    demand |= (TISTAB | SUFTAB);
  }
  indexname = argv[5];
  if(mapvirtualtreeifyoucan(&virtualtree,indexname,demand) != 0)
  {
    STANDARDMESSAGE;
  }
  if(showvirtualtreestatus(&virtualtree,
                           indexname,
                           showverbose) != 0)
  {
    STANDARDMESSAGE;
  }
  maxrandscore = (Sint) readint;
  prof.numofcharacters = virtualtree.alpha.mapsize-1;
  ALLOCASSIGNSPACE(prof.scoretab,NULL,ProfScore,
                   prof.numofcharacters * prof.dimension);
  initmulttab(prof.multtab,prof.numofcharacters,prof.dimension);
  srand48(42349421);
  iterateprofilematching(programs2run,
                         &virtualtree,
                         numofprofiles,
                         &prof,
                         (ProfScore) maxrandscore,
                         virtualtree.alpha.characters);
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
  FREESPACE(prof.scoretab);
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  checkfilehandles();
  return EXIT_SUCCESS;
}
