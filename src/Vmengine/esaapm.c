
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "types.h"
#include "debugdef.h"
#include "spacedef.h"
#include "dpbitvec48.h"
#include "chardef.h"
#include "minmax.h"
#include "errordef.h"
#include "arraydef.h"
#include "virtualdef.h"

#define UNDEFINDEX      ((Smalllength) (-1))

#define SETMAXLENGTH\
        maxlength = apminfo.plen + apminfo.threshold;\
        if(maxlength > vlen)\
        {\
          maxlength = vlen;\
        }

#define APMSUCCESS(IDX)\
        if(processapmstartpos == NULL)\
        {\
          return (Sint) 1;\
        } else\
        {\
          if(processapmstartpos(virtualtree->suftab[IDX],\
                                maxlength,\
                                apmoutinfo) != 0)\
          {\
            return (Sint) -1;\
          }\
        }

typedef struct
{
  DPbitvector4 *Eqs4;  // bit vector for reverse order match
  Uint threshold,                 // score threshold
       plen;                      // pattern length
} MyersBVinfo;

typedef signed short Smalllength;

typedef struct
{
  DPbitvector4 Pv,    // the plus-vector for Myers Algorithm
               Mv;    // the minus-vector for Myers Algorithm
  Smalllength maxleqk;// \(\max\{i\in[0,m]\mid D(i)\leq k\}\) where
                      // \(m\) is the length of the pattern, \(k\) is the
                      // distance threshold, and \(D\) is
                      // the current distance column
#ifdef DEBUG
  Uint scorevalue;    // the score for the given depth
#endif
} Column;

/*
  The following function computes 
  \(\max\{i\in[0,m]\mid D(i)\leq k}\) in a brute force way.
  \texttt{threshold} is the maximal distance value \(k\).
  \texttt{plen} is the length \(m\) of the pattern.
  \texttt{plus} is the vector storing where the DP-column is incremented.
  \texttt{minus} is the vector storing where the DP-column is descremented.
*/

#ifdef DEBUG
static void verifycolumnvalues(Uint threshold,
                               Uint plen,
                               Column *col,
                               Uint startscore)
{
  Uint i, score = startscore, minscore;
  DPbitvector4 mask = ONEDPbitvector4;
  Smalllength maxleqkindex;

  if(score <= threshold)
  {
    maxleqkindex = 0;
    minscore = score;
  } else
  {
    maxleqkindex = UNDEFINDEX;
    minscore = 0;
  }
  //printf("%lu",(Showuint) score);
  for(i=UintConst(1); i <= plen; i++, mask <<= 1)
  {
    if(col->Pv & mask)
    {
      score++;
    } else
    {
      if(col->Mv & mask)
      {
        score--;
      }
    }
    //printf(" %lu",(Showuint) score);
    if(score <= threshold)
    {
      maxleqkindex = (Smalllength) i;
      minscore = score;
    }
  }
  //printf("\n");
  //printf("maxleqkindex=%lu\n",(Showuint) maxleqkindex);
  if(maxleqkindex != col->maxleqk)
  {
    fprintf(stderr,"correct maxleqkindex = %hd != %hd = col->maxleqk\n",
                   maxleqkindex,
                   col->maxleqk);
    exit(EXIT_FAILURE);
  }
  if(maxleqkindex != UNDEFINDEX)
  {
    if(minscore != col->scorevalue)
    {
      fprintf(stderr,"correct score = %lu != %lu = col->score\n",
                   (Showuint) minscore,
                   (Showuint) col->scorevalue);
      exit(EXIT_FAILURE);
    }
  }
}
#endif

/*
  The following function implements the bit-vector transformation of 
  Myers Algorithm. In particular, an input column \texttt{incol}
  is transformated into an output column \texttt{outcol} according to
  a character \texttt{currentchar}. The transformation
  cannot be done in-place, since \texttt{incol} maybe used again when
  backtracking with another character. In addition to the vectors 
  \texttt{Pv} and \texttt{Mv} of Myers algorithm we maintain a value
  \texttt{maxleqk} in constant additional time.
  Note another important difference to Myers algorithm:
  The column implicitly computed for the DP does not begin
  with the value 0. Instead, the \(j\)th column begins with value \(j\).
  To achieve this we add an extra statement to the code,
  as marked below: (Stefan: this has to be done).
*/

static void nextEDcolumn(MyersBVinfo *apminfo,
                         Column *outcol,
                         Uchar currentchar,
                         Column *incol)
{
  DPbitvector4 Eq, Xv, Xh, Ph, Mh, // as in Myers Paper
               backmask;           // only one bit is on
  Smalllength idx;                 // a counter
  Uint score;                      // current score

  Eq = apminfo->Eqs4[(Uint) currentchar];
  Xv = Eq | incol->Mv;
  Xh = (((Eq & incol->Pv) + incol->Pv) ^ incol->Pv) | Eq;

  Ph = incol->Mv | ~ (Xh | incol->Pv);
  Mh = incol->Pv & Xh;

  Ph = (Ph << 1) | ONEDPbitvector4;
  outcol->Pv = (Mh << 1) | ~ (Xv | Ph);
  outcol->Mv = Ph & Xv;
  DEBUG1(3,"incol->maxleqk %ld\n",(Showsint) incol->maxleqk);
#ifdef DEBUG
  if((Uint) incol->maxleqk == apminfo->plen)
  {
    fprintf(stderr,"incol->maxleqk = %ld = patternlen not allowed\n",
            (Showsint) apminfo->plen);
    exit(EXIT_FAILURE);
  }
  if(incol->maxleqk == UNDEFINDEX)
  {
    fprintf(stderr,"incol->maxleqk = UNDEFINDEX not allowed\n");
    exit(EXIT_FAILURE);
  }
#endif
  backmask = ONEDPbitvector4 << (Uint) incol->maxleqk;  
             // \(i\)th bit is on iff i=maxleqk
/*
  Suppose \(i=incol->maxleqk\).
  case (a): Suppose that that the value at index i+1 in the current column 
    is computed along the diagonal or the value along the horizontal
    is decremented. Then it must be \(k\). So the maxleqk-value of the
    current column is incol->maxleqk + 1. 
  case (b): 
    Otherwise, we check if the value along the horizontal was incremented. 
    If this is the case, then the score at index \(i+1\) is \(k+1\). We
    check the values at index \(i,i-1,...,0\) starting with \(k+1\),
    and find the first value smaller or equal to \(k\). As soon as we
    have found it we can break. If there is no value smaller or equal
    to \(k\), then maxleqk is undefined.
  case (c):
    If the value along the horizontal is neither incremented nor
    decremented, it did not change, and thus the maxleqk does not change.
*/
  if(Eq & backmask || Mh & backmask)     // case (a)
  {
    outcol->maxleqk = incol->maxleqk + (Smalllength) 1;
#ifdef DEBUG
    outcol->scorevalue = incol->scorevalue;
#endif
  } else
  {
    if(Ph & backmask)                    // case (b)
    {
      score = apminfo->threshold+1;
      outcol->maxleqk = UNDEFINDEX;
      for(idx = incol->maxleqk - (Smalllength) 1, backmask >>= 1; 
          idx >= 0; 
          idx--, backmask >>= 1)
      {
        if(outcol->Pv & backmask)
        {
          score--;
          if(score <= apminfo->threshold)
          {
            outcol->maxleqk = idx;
#ifdef DEBUG
            outcol->scorevalue = score;
#endif
            break;
          }
        } else
        {
          if(outcol->Mv & backmask)
          {
            score++;
          }
        }
      }
    } else                                 // case (c)
    {
      outcol->maxleqk = incol->maxleqk;
#ifdef DEBUG
      outcol->scorevalue = incol->scorevalue;
#endif
    }
  }
}

static Uint evaluateremainingapm(MyersBVinfo *apminfo,
                                 Uchar *vptr,
                                 Column *cstack,
                                 Uint nextdvalue,
                                 Uint maxdvalue)
{
  Uint d;
  Uchar currentchar;

  DEBUG2(3,"evaluateremainingapm(%lu,%lu)\n",(Showuint) nextdvalue,
                                             (Showuint) maxdvalue);
  for(d=nextdvalue; d<=maxdvalue; d++)
  {
    if(cstack[d-1].maxleqk == (Smalllength) apminfo->plen)
    {
      DEBUG3(3,"line %lu: evaluateremainingapm return %ld with score %lu\n",
              (Showuint) __LINE__,
              (Showsint) d-1,
              (Showuint) cstack[d-1].scorevalue);
      return d-1;
    }
    currentchar = vptr[d-1];
    if(currentchar == SEPARATOR)
    {
      return d-1;
    }
    nextEDcolumn(apminfo,
                 cstack + d,
                 vptr[d-1],
                 cstack + d - 1);
#ifdef DEBUG
    verifycolumnvalues(apminfo->threshold,
                       apminfo->plen,
                       cstack + d,
                       d);
#endif
    if(cstack[d].maxleqk == UNDEFINDEX)
    {
      DEBUG3(3,"line %lu: evaluateremainingapm returns %ld with score %lu\n",
                (Showuint) __LINE__,
                (Showsint) (d-1),
                (Showuint) cstack[d-1].scorevalue);
      return d-1;
    }
  }
  DEBUG3(3,"line %lu: evaluateremainingapm returns %ld with score %lu\n",
             (Showuint) __LINE__,
             (Showsint) maxdvalue,
             (Showuint) cstack[maxdvalue].scorevalue);
  return maxdvalue;
}

Sint esaapm(Sint (*processapmstartpos)(Uint,Uint,void *),
            void *apmoutinfo,
            Virtualtree *virtualtree,
            DPbitvector4 *Eqs4,
            Uint plen,
            Uint threshold)
{
  Uint dvalue, idx, vlen, maxlength, lcpvalue, current;
  Uchar *vptr;
  Column cstack[2 * DPWORDSIZE4+1];
  MyersBVinfo apminfo;
 
  apminfo.Eqs4 = Eqs4;
  apminfo.threshold = threshold;
  apminfo.plen = plen;
  cstack[0].Pv = (DPbitvector4) ~0;
  cstack[0].Mv = (DPbitvector4) 0;
  cstack[0].maxleqk = (Smalllength) apminfo.threshold;
#ifdef DEBUG
  cstack[0].scorevalue = apminfo.threshold;
#endif
  vptr = virtualtree->multiseq.sequence + virtualtree->suftab[0];
  vlen = virtualtree->multiseq.totallength - virtualtree->suftab[0];
  SETMAXLENGTH;
  DEBUG1(3,"maxlength=%lu\n",(Showuint) maxlength);
  dvalue = evaluateremainingapm(&apminfo,
                                vptr,
                                &cstack[0],
                                UintConst(1),
                                maxlength);
  if(cstack[dvalue].maxleqk == (Smalllength) apminfo.plen)
  {
    APMSUCCESS(0);
  }
  idx = UintConst(1);
  while(idx < virtualtree->multiseq.totallength)
  {
    DEBUG3(3,"idx=%lu,suf=%lu,lcp=%lu\n",(Showuint) idx,
                                         (Showuint) virtualtree->suftab[idx],
                                         (Showuint) virtualtree->lcptab[idx]);
    vptr = virtualtree->multiseq.sequence + virtualtree->suftab[idx];
    vlen = virtualtree->multiseq.totallength - virtualtree->suftab[idx];
    SETMAXLENGTH;
    DEBUG1(3,"maxlength=%lu\n",(Showuint) maxlength);
    lcpvalue = virtualtree->lcptab[idx];
    if(dvalue >= lcpvalue)
    {
      dvalue = evaluateremainingapm(&apminfo,
                                    vptr,
                                    &cstack[0],
                                    lcpvalue+1,
                                    maxlength);
      DEBUG3(3,"line %lu, dvalue = %lu,maxleqk=%ld\n",
             (Showuint) __LINE__,(Showuint) dvalue,
             (Showsint) cstack[dvalue].maxleqk);
      if(cstack[dvalue].maxleqk == (Smalllength) apminfo.plen)
      {
        APMSUCCESS(idx);
      }
      idx++;
    } else
    {
      current = idx;
      while(True)
      {
        idx = virtualtree->skiptab[idx]+1;
        if(idx >= virtualtree->multiseq.totallength)
        {
          break;
        }
        lcpvalue = virtualtree->lcptab[idx];
        if(dvalue >= lcpvalue)
        {
          break;
        }
      }
      if(cstack[dvalue].maxleqk == (Smalllength) apminfo.plen)
      {
        while(current < idx)
        {
          APMSUCCESS(current);
          current++;
        }
      }
    }
  }
  return 0;
}
