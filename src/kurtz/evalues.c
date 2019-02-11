//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "evaluedef.h"
#include "spacedef.h"
#include "debugdef.h"
#include "minmax.h"
#include "failures.h"

#define MINADDAMOUNT 256

//}

/*
  This file implements functions to compute E-values, as described
  in the following paper:

  Kurtz, S. and Ohlebusch, E. and Stoye, J. and Schleiermacher, C. and 
  Giegerich, R.: Computation and Visualization of Degenerate Repeats 
  in Complete Genomes, in Proc.\ of the International Conference on 
  Intelligent Systems for Molecular Biology, pages 228-238,
  AAAI-Press, 2000

  We print E-values with 2 digit following the '.'
*/

#define FORMATEVALUE "%.2e"

/*
  Given the probability \texttt{P} of a match, the following macro delivers
  the probability that there is no match.
*/

#define PROBNOMATCH(P)        ((Evaluetype) (1.0 - (P)))

/*
  The access to the table of E-values.
*/

#define EVALUETAB(I) hpt->evaluetable.spaceEvaluelogtype[(I)]

/*
  The following defines the maximal exponent of two which we
  want to compute.
*/

#define MAXEXPONENTOF2 100

/*
  The following table stores the average quotient 
  of edit distance Evalues and hamming distance Evalues depending on
  the given distance.
*/

static Evaluetype averagequot[] =
{
  0.0,
  3.97e+00,  // k = 1
  1.28e+01,  // k = 2
  3.26e+01,  // k = 3
  7.60e+01,  // k = 4
  1.71e+02,  // k = 5
  3.77e+02,  // k = 6
  8.22e+02,  // k = 7
  1.78e+03,  // k = 8
  3.91e+03,  // k = 9
  8.50e+03,  // k = 10
  1.76e+04,  // k = 11
  3.78e+04,  // k = 12
  7.98e+04,  // k = 13
  1.66e+05,  // k = 14
  3.58e+05,  // k = 15
  7.44e+05,  // k = 16
  1.52e+06,  // k = 17
  3.20e+06,  // k = 18
  6.40e+06,  // k = 19
  1.31e+07   // k = 20
  /* othervalues:  1.31e+07 * (1 << (k-20)) */
};

BOOL cmpEvaluetype(Evaluetype a,Evaluetype b)
{
  char sa[100], sb[100];
  sprintf(sa,FORMATEVALUE,a);
  sprintf(sb,FORMATEVALUE,b);
  if(strcmp(sa,sb) == 0)
  {
    return True;
  }
  DEBUG2(3,"sa=\"%s\", sb=\"%s\"\n",sa,sb);
  return False;
}

//\Ignore{

#ifdef DEBUG
static Evaluetype binom(Sint l,Sint k)
{
  Evaluetype b = 1.0;
  Sint i;

  if(k == 0)
  {
    return 1.0;
  }
  if(l < k)
  {
    return 0.0;
  }
  for(i=l-k+1; i<=l; i++)
  {
    b *= (Evaluetype) i;
  }
  for(i=(Sint) 1; i<=k; i++)
  {
    b /= (Evaluetype) i;
  }
  return b;
}

static Evaluetype getEvaluebrute(Evaluetype probmatch,Sint l,Sint k)
{
  Evaluetype p;

  p = 1.0;
  p *= binom(l,k);
  p *= pow(probmatch,(Evaluetype) (l-k));
  p *= pow(PROBNOMATCH(probmatch),(Evaluetype) (k+2));
  return p;
}

void debugcheckevalue(Evalues *hpt,Evaluetype probmatch,Evaluetype prob,
                      Sint l,Sint k)
{
  Evaluetype realprob, lookupprob;

  realprob = getEvaluebrute(probmatch,l,k);
  if(!cmpEvaluetype(realprob,prob))
  {
    fprintf(stderr,"(l,k)=(%ld,%ld): prob=",(Showsint) l,(Showsint) k);
    fprintf(stderr,FORMATEVALUE,prob);
    fprintf(stderr," != ");
    fprintf(stderr,FORMATEVALUE,realprob);
    fprintf(stderr," = realprob\n");
    exit(EXIT_FAILURE);
  }
  lookupprob = (Evaluetype) EVALUETAB(hpt->evaluelinestart.spaceSint[k] + l);
  if(!cmpEvaluetype(realprob,lookupprob))
  {
    fprintf(stderr,"(l,k)=(%ld,%ld): lookupprob=",(Showsint) l,(Showsint) k);
    fprintf(stderr,FORMATEVALUE,lookupprob);
    fprintf(stderr," != ");
    fprintf(stderr,FORMATEVALUE,realprob);
    fprintf(stderr," = realprob\n");
    exit(EXIT_FAILURE);
  }
}
#define DEBUGCHECKEVALUE(H,PM,P,L,K)   /*debugcheckevalue(H,PM,P,L,K)*/
#else   /* !DEBUG */
#define DEBUGCHECKEVALUE(H,PM,P,L,K)   /* Nothing */
#endif

//}

/*EE
  The following function precomputes hamming distance E-values according
  to the formulas given in the paper cited above. The precomputation
  must also be done for the edit distance case, since the corresponding 
  E-values are derived from these. \texttt{hpt} is the table to store
  the E-values in, \texttt{probmatch} is the probability of a match,
  and \texttt{kmax} is the maximal integer to compute the tables for.
*/

void precomputehammingEvalues(Evalues *hpt,Evaluetype probmatch,Sint kmax)
{
  Sint l, k;
  Evaluetype prob; 

  DEBUG1(2,"precomputehammingEvalue(kmax=%ld)\n",(Showsint) kmax);
  DEBUG1(2,FORMATEVALUE,probmatch);
  DEBUG0(2,",");
  DEBUG1(2,FORMATEVALUE,PROBNOMATCH(probmatch));
  DEBUG0(2,"\n");
  DEBUG1(2,"kmax=%ld\n",(Showsint) kmax);

  hpt->first = probmatch * (PROBNOMATCH(probmatch) * PROBNOMATCH(probmatch));
  hpt->evaluelinestart.nextfreeSint = 0;
  hpt->evaluelinestart.allocatedSint = (Uint) (kmax+2);
  ALLOCASSIGNSPACE(hpt->evaluelinestart.spaceSint,NULL,Sint,
                   hpt->evaluelinestart.allocatedSint);
  hpt->evaluetable.nextfreeEvaluelogtype = 0;
  hpt->evaluetable.allocatedEvaluelogtype = UintConst(4096); 
  ALLOCASSIGNSPACE(hpt->evaluetable.spaceEvaluelogtype,
                   NULL,Evaluelogtype,hpt->evaluetable.allocatedEvaluelogtype);
  for(k=0; k <= kmax; k++)
  {
#ifdef DEBUG
    if(k >= (Sint) hpt->evaluelinestart.allocatedSint)
    {
      fprintf(stderr,"Error for evaluelinestart\n");
      exit(EXIT_FAILURE);
    }
#endif
    hpt->evaluelinestart.spaceSint[k] 
      = (Sint) hpt->evaluetable.nextfreeEvaluelogtype - (k+1);
    DEBUG2(2,"evaluelinestart[%ld]=%ld\n",
               (Showsint) k,
               (Showsint) hpt->evaluelinestart.spaceSint[k]);
    prob = hpt->first;
    hpt->first *= (((Evaluetype) (k+2)/(k+1)) * PROBNOMATCH(probmatch));
    for(l=k+1; prob > SMALLESTEVALUE; l++)
    {
      DEBUG3(4,"prob[%lu](%ld,%ld)=",
             (Showuint) hpt->evaluetable.nextfreeEvaluelogtype,
             (Showsint) l,
             (Showsint) k);
      DEBUG1(4,FORMATEVALUE,(Evaluelogtype) log(prob));
      DEBUG0(4,"\n");

      CHECKARRAYSPACE(&hpt->evaluetable,Evaluelogtype,MINADDAMOUNT);
      EVALUETAB(hpt->evaluetable.nextfreeEvaluelogtype++) 
        = (Evaluelogtype) prob;
      DEBUGCHECKEVALUE(hpt,probmatch,prob,l,k);
      prob *= ((l+1) * probmatch / (l+1-k));

      DEBUG1(4,"i=%lu: prob=",
              (Showuint) (hpt->evaluetable.nextfreeEvaluelogtype-1));
      DEBUG1(4,FORMATEVALUE,EVALUETAB(hpt->evaluetable.nextfreeEvaluelogtype-1));
      DEBUG0(4," exp(logprob)=");
      DEBUG1(4,FORMATEVALUE,exp((Evaluetype) EVALUETAB(hpt->evaluetable.nextfreeEvaluelogtype-1)));
      DEBUG0(4,"\n");
    }
  }
#ifdef DEBUG
  if(kmax+1 >= (Sint) hpt->evaluelinestart.allocatedSint)
  {
    fprintf(stderr,"not enough allocated: kmax=%ld,allocated=%lu\n",
             (Showsint) kmax, 
             (Showuint) hpt->evaluelinestart.allocatedSint);
    exit(EXIT_FAILURE);
  }
#endif
  hpt->evaluelinestart.nextfreeSint = (Uint) (kmax+1);
  hpt->evaluelinestart.spaceSint[kmax+1] 
    = (Sint) hpt->evaluetable.nextfreeEvaluelogtype - (kmax+1+1);
  DEBUG2(2,"evaluelinestart[%ld]=%ld\n",
          (Showsint) kmax+1,
          (Showsint) hpt->evaluelinestart.spaceSint[kmax+1]);
}

static Evaluetype lookupEvalue(Evalues *hpt,Sint distance,Sint length)
{
  Sint i;

  i = hpt->evaluelinestart.spaceSint[distance] + length;
  if(i < hpt->evaluelinestart.spaceSint[distance+1] + distance + 2)
  {
    return (Evaluetype) EVALUETAB(i);
  }
  return (Evaluetype) 0.0;
}

Evaluetype getEvalue(Evalues *hpt,Evaluetype multiplier,
                     Sint distance,Uint length)
{
  Evaluetype evalue, hequot;
  double powvalue;

  DEBUG0(2,"getEvalue(multiplier=");
  DEBUG1(2,FORMATEVALUE,multiplier);
  DEBUG2(2,",distance=%ld,length=%lu)\n",(Showsint) distance,
                                         (Showuint) length);
  if(distance <= 0)
  {
    evalue = multiplier * lookupEvalue(hpt,-distance,(Sint) length);
  } else
  {
    if(distance > (Sint) 20)
    {
      if((distance - (Sint) 20) > (Sint) MAXEXPONENTOF2)
      {
        evalue = 0.0;
      } else
      {
        powvalue = pow(2.0,(double) (distance-20));
        hequot = 1.31e+07 * powvalue;
        evalue = multiplier * hequot * lookupEvalue(hpt,distance,(Sint) length);
      }
    } else
    {
      hequot = averagequot[distance];
      evalue = multiplier * hequot * lookupEvalue(hpt,distance,(Sint) length);
    }
  }
  DEBUG1(2,FORMATEVALUE,evalue);
  DEBUG0(2,"\n");
  return evalue;
}

void inithammingEvalues(Evalues *hpt,Evaluetype probmatch)
{
  hpt->probmatch = probmatch;
  hpt->first = probmatch * (PROBNOMATCH(probmatch) * PROBNOMATCH(probmatch));
  INITARRAY(&hpt->evaluelinestart,Sint);
  INITARRAY(&hpt->evaluetable,Evaluelogtype);
}

void incprecomputehammingEvalues(Evalues *hpt,Sint kmax)
{
  Sint l, k;
  Evaluetype prob;
  Sint addamount = kmax + 2 - (Sint) hpt->evaluelinestart.nextfreeSint;

  DEBUG1(2,"incprecomputehammingEvalue(%ld)\n",(Showsint) kmax);
  if(addamount < (Sint) MINADDAMOUNT)
  {
    addamount = (Sint) MINADDAMOUNT;
  }
  CHECKARRAYSPACEMULTI(&hpt->evaluelinestart,Sint,(Uint) addamount);
  for(k=(Sint) hpt->evaluelinestart.nextfreeSint; k<=kmax; k++)
  {
#ifdef DEBUG
    if(k >= (Sint) hpt->evaluelinestart.allocatedSint)
    {
      fprintf(stderr,"Error for evaluelinestart\n");
      exit(EXIT_FAILURE);
    }
#endif
    hpt->evaluelinestart.spaceSint[k] 
      = (Sint) hpt->evaluetable.nextfreeEvaluelogtype - (k+1);
    DEBUG2(2,"evaluelinestart[%ld]=%ld\n",
             (Showsint) k,
             (Showsint) hpt->evaluelinestart.spaceSint[k]);
    prob = hpt->first;
    hpt->first *= (((Evaluetype) (k+2)/(k+1)) * PROBNOMATCH(hpt->probmatch));
    for(l=k+1; prob > SMALLESTEVALUE; l++)
    {
      CHECKARRAYSPACE(&hpt->evaluetable,Evaluelogtype,MINADDAMOUNT);
      EVALUETAB(hpt->evaluetable.nextfreeEvaluelogtype++) 
        = (Evaluelogtype) prob;
      DEBUGCHECKEVALUE(hpt,hpt->probmatch,prob,l,k);
      prob *= ((l+1) * hpt->probmatch / (l+1-k));
    }
  }
#ifdef DEBUG
  if(kmax+1 >= (Sint) hpt->evaluelinestart.allocatedSint)
  {
    fprintf(stderr,"not enough allocated: kmax=%ld,allocated=%lu\n",
            (Showsint) kmax,
            (Showuint) hpt->evaluelinestart.allocatedSint);
    exit(EXIT_FAILURE);
  }
#endif
  hpt->evaluelinestart.spaceSint[kmax+1]
    = (Sint) hpt->evaluetable.nextfreeEvaluelogtype - (kmax+1+1);
  hpt->evaluelinestart.nextfreeSint = (Uint) (kmax+1);
}

static Evaluetype inclookupEvalue(Evalues *hpt,Sint distance,Sint length)
{
  Sint i;

  if(distance+1 > (Sint) hpt->evaluelinestart.nextfreeSint)
  {
    incprecomputehammingEvalues(hpt,distance);
  }
  i = hpt->evaluelinestart.spaceSint[distance] + length;
  if(i < hpt->evaluelinestart.spaceSint[distance+1] + distance + 2)
  {
    return (Evaluetype) EVALUETAB(i);
  }
  return (Evaluetype) 0.0;
}

Evaluetype incgetEvalue(Evalues *hpt,Evaluetype multiplier,
                        Sint distance,Uint length)
{
  Evaluetype evalue, hequot;
  double powvalue;

  DEBUG0(2,"incgetEvalue(multiplier=");
  DEBUG1(2,FORMATEVALUE,multiplier);
  DEBUG2(2,",distance=%ld,length=%lu)\n",(Showsint) distance,
                                         (Showuint) length);

  if(distance <= 0)
  {
    evalue = multiplier * inclookupEvalue(hpt,-distance,(Sint) length);
  } else
  {
    if(distance > (Sint) 20)
    {
      if((distance - 20) > (Sint) MAXEXPONENTOF2)
      {
        evalue = 0.0;
      } else
      {
        powvalue = pow(2.0,(double) (distance-20));
        hequot = 1.31e+07 * powvalue;
        evalue = multiplier * hequot * inclookupEvalue(hpt,distance,
                                                       (Sint) length);
      }
    } else
    {
      hequot = averagequot[distance];
      evalue = multiplier * hequot * inclookupEvalue(hpt,distance,
                                                     (Sint) length);
    }
  }
  DEBUG1(2,FORMATEVALUE,evalue);
  DEBUG0(2,"\n");
  return evalue;
}

void freeEvalues(Evalues *hpt)
{
  FREEARRAY((&hpt->evaluelinestart),Sint);
  FREEARRAY((&hpt->evaluetable),Evaluelogtype);
}

#ifdef DEBUG
void checkhamming(Evaluetype probmatch,Sint kmax)
{
  Evalues ev1, ev2, ev3;
  Sint l, k;
  Uint i, checked;
  Evaluetype e1, e3;

  precomputehammingEvalues(&ev1,probmatch,kmax);
  inithammingEvalues(&ev2,probmatch);
  for(k=0; k<=kmax; k++)
  {
    incprecomputehammingEvalues(&ev2,k);
  }
  if(ev1.evaluelinestart.nextfreeSint != ev2.evaluelinestart.nextfreeSint)
  {
    fprintf(stderr,"nextfreeSint: %lu != %lu\n",
                     (Showuint) ev1.evaluelinestart.nextfreeSint,
                     (Showuint) ev2.evaluelinestart.nextfreeSint);
    exit(EXIT_FAILURE);
  }
  for(i=0; i < ev1.evaluelinestart.nextfreeSint; i++)
  {
    if(ev1.evaluelinestart.spaceSint[i] != ev2.evaluelinestart.spaceSint[i])
    {
      fprintf(stderr,"spaceSint[%lu]: %ld != %ld\n",
                      (Showuint) i,
                      (Showsint) ev1.evaluelinestart.spaceSint[i],
                      (Showsint) ev2.evaluelinestart.spaceSint[i]);
      exit(EXIT_FAILURE);
    }
  }
  if(ev1.evaluetable.nextfreeEvaluelogtype != 
     ev2.evaluetable.nextfreeEvaluelogtype)
  {
    fprintf(stderr,"nextfreeEvaluelogtype: %lu != %lu\n",
                         (Showuint) ev1.evaluetable.nextfreeEvaluelogtype,
                         (Showuint) ev2.evaluetable.nextfreeEvaluelogtype);
    exit(EXIT_FAILURE);
  }
  for(i=0; i < ev1.evaluetable.nextfreeEvaluelogtype; i++)
  {
    if(ev1.evaluetable.spaceEvaluelogtype[i] != 
       ev2.evaluetable.spaceEvaluelogtype[i] &&
       !cmpEvaluetype(ev1.evaluetable.spaceEvaluelogtype[i],
                      ev2.evaluetable.spaceEvaluelogtype[i]))
    {
      fprintf(stderr,"spaceSint[%lu]: %.8e != %.8e\n",
                           (Showuint) i,
                           ev1.evaluetable.spaceEvaluelogtype[i],
                           ev2.evaluetable.spaceEvaluelogtype[i]);
      exit(EXIT_FAILURE);
    }
  }
  inithammingEvalues(&ev3,probmatch);
  for(checked=0, k=0; k<=kmax; k++)
  {
    for(l=k+1; /* Nothing */; l++)
    {
      e1 = lookupEvalue(&ev1,k,l);
      e3 = inclookupEvalue(&ev3,k,l);
      if(e1 != e3 && !cmpEvaluetype(e1,e3))
      {
        fprintf(stderr,"checkHamming(l,k)=(%ld,%ld): %.2e != %.2e\n",
                   (Showsint) l,
                   (Showsint) k,e1,e3);
        exit(EXIT_FAILURE);
      }
      if(e1 == 0.0)
      {
        /*@innerbreak@*/ break;
      }
      checked++;
    }
  }
  printf("lookupEvalue is okay: %lu values checked\n",(Showuint) checked);
  checked++;
  freeEvalues(&ev1);
  freeEvalues(&ev2);
  freeEvalues(&ev3);
}
#endif
