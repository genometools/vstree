#include "args.h"
#include "spacedef.h"
#include "debugdef.h"
#include "virtualdef.h"

#include "readvirt.pr"

static Uint downchecks = 0;
static Uint upchecks = 0;
static Uint nextlIndexchecks = 0;

#ifdef DEBUG
static void showchildtabfield(Uint i,Uchar v)
{
  if(v == UNDEFCHILDVALUE)
  {
    printf("_");
  } else
  {
    if(v == LARGECHILDVALUE)
    {
      printf("L");
    } else
    {
      printf("%lu",(Showuint) (i+v));
    }
  }
}


static void showchildtab(Childinfo *cldtab,Uint totallength)
{
  Uint i;

  for(i=0; i <= totallength; i++)
  {
    printf("cldtab[%lu]=(",(Showuint) i);
    showchildtabfield(i,cldtab[i].up);
    printf(", ");
    showchildtabfield(i,cldtab[i].down);
    printf(", ");
    showchildtabfield(i,cldtab[i].nextlIndex);
    printf(")\n");
  }
}
#endif

static void verifycldtabnextlIndex(Virtualtree *virtualtree)
{
  Uint i, q, lcpi, lcpq;
  PairUint *exception;

  for(i=0; i<virtualtree->multiseq.totallength; i++)
  {
    if(i==0)
    {
      lcpi = 0;
    } else
    {
      if((lcpi = virtualtree->lcptab[i]) == UCHAR_MAX)
      {
        exception = getexception(virtualtree,i);
        lcpi = exception->uint1;
      }
    }
    for(q=i+1; q<=virtualtree->multiseq.totallength; q++)
    {
      if((lcpq = virtualtree->lcptab[q]) == UCHAR_MAX)
      {
        exception = getexception(virtualtree,q);
        lcpq = exception->uint1;
      }
      if(lcpq == lcpi)
      {
        nextlIndexchecks++;
        if(i + virtualtree->cldtab[i].nextlIndex != q)
        {
          if(q <= LARGECHILDVALUE)
          {
            fprintf(stderr,"cldtab[%lu].nextlIndex=%lu!=%lu\n",
                            (Showuint) i,
                            (Showuint) (i+virtualtree->cldtab[i].nextlIndex),
                            (Showuint) q);
            exit(EXIT_FAILURE);
          } 
        }
        break;
      } 
      if(lcpq < lcpi)
      {
        nextlIndexchecks++;
        if(virtualtree->cldtab[i].nextlIndex != UNDEFCHILDVALUE)
        {
          fprintf(stderr,"cldtab[%lu].nextlIndex=%lu != UNDEFCHILDVALUE\n",
                            (Showuint) i,
                            (Showuint) virtualtree->cldtab[i].nextlIndex);
          exit(EXIT_FAILURE);
        }
        break;
      }
    }
  }
}

static void verifycldtabdown(Virtualtree *virtualtree)
{
  Uint i, q, lcpi, lcpq, previousmin, qdown;
  PairUint *exception;

  for(i=0; i<virtualtree->multiseq.totallength; i++)
  {
    if(i==0)
    {
      lcpi = 0;
    } else
    {
      if((lcpi = virtualtree->lcptab[i]) == UCHAR_MAX)
      {
        exception = getexception(virtualtree,i);
        lcpi = exception->uint1;
      }
    }
    previousmin = virtualtree->multiseq.totallength;
    qdown = UNDEFCHILDVALUE;
    for(q=i+1; q<=virtualtree->multiseq.totallength; q++)
    {
      if((lcpq = virtualtree->lcptab[q]) == UCHAR_MAX)
      {
        exception = getexception(virtualtree,q);
        lcpq = exception->uint1;
      }
      if(lcpq > lcpi)
      {
        if(previousmin > lcpq)
        {
          previousmin = lcpq;
          qdown = q;
        }
      } 
      if(lcpq <= lcpi)
      {
        break;
      }
    }
    downchecks++;
    if(qdown <= LARGECHILDVALUE)
    {
      if(virtualtree->cldtab[i].down == UNDEFCHILDVALUE)
      {
        if(qdown != UNDEFCHILDVALUE)
        {
          fprintf(stderr,"cldtab[%lu].down=UNDEFCHILDVALUE!=%lu\n",
                  (Showuint) i,
                  (Showuint) qdown);
          exit(EXIT_FAILURE);
        }
      } else
      {
        if(i + virtualtree->cldtab[i].down != qdown)
        {
          fprintf(stderr,"cldtab[%lu].down=%lu!=%lu\n",
                              (Showuint) i,
                              (Showuint) (i+virtualtree->cldtab[i].down),
                              (Showuint) qdown);
          exit(EXIT_FAILURE);
        }
      }
    }
  }
}

static void verifycldtabup(Virtualtree *virtualtree)
{
  Uint i, q, lcpi, lcpq, previousmin, qup;
  PairUint *exception;

  for(i=0; i<virtualtree->multiseq.totallength; i++)
  {
    if(i==0)
    {
      lcpi = 0;
    } else
    {
      if((lcpi = virtualtree->lcptab[i]) == UCHAR_MAX)
      {
        exception = getexception(virtualtree,i);
        lcpi = exception->uint1;
      }
    }
    if(i==0)
    {
      qup = UNDEFCHILDVALUE;
    } else
    {
      previousmin = virtualtree->multiseq.totallength;
      qup = UNDEFCHILDVALUE;
      for(q=i-1; q>0; q--)
      {
        if((lcpq = virtualtree->lcptab[q]) == UCHAR_MAX)
        {
          exception = getexception(virtualtree,q);
          lcpq = exception->uint1;
        }
        if(lcpq > lcpi)
        {
          if(previousmin >= lcpq)
          {
            qup = q;
          }
        } 
        if(lcpq < lcpi)
        {
          break;
        }
        if(previousmin > lcpq)
        {
          previousmin = lcpq;
        }
      }
    }
    upchecks++;
    if(virtualtree->cldtab[i].up == LARGECHILDVALUE)
    {
      if(i - qup < LARGECHILDVALUE)
      {
        fprintf(stderr,"cldtab[%lu].up=LARGECHILDVALUE!=%lu\n",
                 (Showuint) i,
                 (Showuint) qup);
        exit(EXIT_FAILURE);
      }
    } else
    {
      if(virtualtree->cldtab[i].up == UNDEFCHILDVALUE)
      {
        if(qup != UNDEFCHILDVALUE)
        {
          fprintf(stderr,"cldtab[%lu].up=UNDEFCHILDVALUE!=%lu\n",
                 (Showuint) i,
                 (Showuint) qup);
          exit(EXIT_FAILURE);
        }
      } else
      {
        if(i - virtualtree->cldtab[i].up != qup)
        {
          fprintf(stderr,"cldtab[%lu].up=%lu!=%lu\n",
                              (Showuint) i,
                              (Showuint) (i-virtualtree->cldtab[i].up),
                              (Showuint) qup);
          fprintf(stderr,"tabvalue = %lu\n",
                           (Showuint) virtualtree->cldtab[i].up);
          exit(EXIT_FAILURE);
        }
      }
    }
  }
}

MAINFUNCTION
{
  Virtualtree virtualtree;
  Uint demand;

  CHECKARGNUM(2,"indexname");
  DEBUGLEVELSET;

  demand = LCPTAB | SUFTAB | CLDTAB;
#ifdef DEBUG
  demand |= TISTAB;
#endif
  if(mapvirtualtreeifyoucan(&virtualtree,argv[1],demand) != 0)
  {
    STANDARDMESSAGE;
  }
  verifycldtabnextlIndex(&virtualtree);
  verifycldtabdown(&virtualtree);
  verifycldtabup(&virtualtree);
  DEBUGCODE(2,showchildtab(virtualtree.cldtab,
                           virtualtree.multiseq.totallength));
  if(freevirtualtree(&virtualtree) != 0)
  {
    STANDARDMESSAGE;
  }
  printf("nextlIndexchecks=%lu\n",(Showuint) nextlIndexchecks);
  printf("downchecks=%lu\n",(Showuint) downchecks);
  printf("upchecks=%lu\n",(Showuint) upchecks);
#ifndef NOSPACEBOOKKEEPING
  checkspaceleak();
#endif
  mmcheckspaceleak();
  return EXIT_SUCCESS;
}
