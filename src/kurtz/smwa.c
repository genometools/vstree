
//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "scoredef.h"
#include "spacedef.h"
#include "alignment.h"
#include "debugdef.h"
#include "failures.h"
#include "errordef.h"

#include "showalign.pr"

#define ADDEOP 128

//}

/*EE
  This module implements the Smith-Waterman algorithm. The following
  function computes the Smith-Watermann dynamic programming table
  (referred to by \texttt{swdptable}), given two sequences and a 
  score function.
*/

SCORE smithwatermanDP(DPpoint *swdptable,
                      Scorefunction *scorefunction,
                      Uchar *useq,
                      Uint ulen,
                      Uchar *vseq, 
                      Uint vlen)
{
  SCORE val, maxsimilarity = 0;
  Uchar *uptr, *vptr;
  DPpoint *oldptr, *newptr;

  for(newptr = swdptable; newptr < swdptable+(ulen+1)*(vlen+1); newptr++)
  {
    newptr->similarity = 0;
    newptr->lu = 0;
    newptr->lv = 0;
  }
  for (oldptr = swdptable, newptr = swdptable+(ulen+1), vptr = vseq; 
       vptr < vseq+vlen; vptr++)
  {
    for(oldptr++, newptr++, uptr=useq; uptr < useq+ulen; 
        oldptr++, newptr++, uptr++)
    {
      if((val = (oldptr-1)->similarity + 
                GETSCORE(&scorefunction->scorematrix,*uptr,*vptr))
              >= newptr->similarity)
      {
	newptr->similarity = val;
        newptr->lu = (oldptr-1)->lu+1;
        newptr->lv = (oldptr-1)->lv+1;
      } 
      if ((val = (newptr-1)->similarity + scorefunction->deletionscore)
               > newptr->similarity)
      {
	newptr->similarity = val;
        newptr->lu = (newptr-1)->lu+1;
        newptr->lv = (newptr-1)->lv;
      }
      if ((val = oldptr->similarity + scorefunction->insertionscore) 
               > newptr->similarity)
      {
	newptr->similarity = val;
        newptr->lu = oldptr->lu;
        newptr->lv = oldptr->lv+1;
      }
      if (newptr->similarity > maxsimilarity)
      {
        maxsimilarity = newptr->similarity;
      }
    }
  }
  return maxsimilarity;
}

/*EE
  The following function computes the optimal local similarity score 
  for two sequences. The score is stored in the memory cell
  referred to by \texttt{scol}. \texttt{maxpair} refers to the 
  memory area where the index pair with optimal score is stored.
*/

SCORE localsimilarityscore(SCORE *scol,
                           PairUint *maxpair,
                           Scorefunction *scorefunction,
                           Uchar *useq,
                           Uint ulen,
                           Uchar *vseq,
                           Uint vlen)
{
  SCORE val, we, nw, *scolptr, maximalscore = 0;
  Uchar *uptr, *vptr;

  maxpair->uint0 = maxpair->uint1 = 0;
  for(scolptr = scol; scolptr <= scol + ulen; scolptr++)
  {
    *scolptr = 0;
  }
  for (vptr = vseq; vptr < vseq + vlen; vptr++)
  {
    nw = 0;
    for(scolptr = scol+1, uptr = useq; 
        uptr < useq + ulen; 
        scolptr++, uptr++)
    {
      we = *scolptr;
      *scolptr = *(scolptr-1) + scorefunction->deletionscore;
      if ((val = nw + GETSCORE(&scorefunction->scorematrix,*uptr,*vptr))
               > *scolptr)
      {
        *scolptr = val;
      } 
      if ((val = we + scorefunction->insertionscore) > *scolptr)
      {
        *scolptr = val;
      }
      if(*scolptr < 0)
      {
        *scolptr = 0;
      } else
      {
        if(*scolptr > maximalscore)
        {
	  maximalscore = *scolptr;
          maxpair->uint0 = (Uint) (uptr - useq + 1);
          maxpair->uint1 = (Uint) (vptr - vseq + 1);
	}
      }
      nw = we;
    }
  }
  return maximalscore;
}

/*EE
  The following function computes the optimal local similarity score 
  for two sequences. A reference to the substring with an 
  optimal score is stored in \texttt{maxentry}.
*/

void localsimilarityregion(DPpoint *scol,
                           DPregion *maxentry,
                           Scorefunction *scorefunction,
                           Uchar *useq,
                           Uint ulen,
                           Uchar *vseq,
                           Uint vlen)
{
  SCORE val;
  DPpoint *scolptr, we, nw;
  Uchar *uptr, *vptr;

  maxentry->similarity = 0;
  maxentry->len1 = 0;
  maxentry->len2 = 0;
  maxentry->start1 = 0;
  maxentry->start1 = 0;
  for(scolptr = scol; scolptr <= scol + ulen; scolptr++)
  {
    scolptr->similarity = 0;
    scolptr->lu = scolptr->lv = 0;
  }
  for (vptr = vseq; vptr < vseq + vlen; vptr++)
  {
    nw = *scol;
    for(scolptr = scol+1, uptr = useq; 
        uptr < useq + ulen; 
        scolptr++, uptr++)
    {
      we = *scolptr;
      scolptr->similarity = (scolptr-1)->similarity + 
                            scorefunction->deletionscore;
      scolptr->lu = (scolptr-1)->lu + 1;
      scolptr->lv = (scolptr-1)->lv;
      if ((val = nw.similarity +
                 GETSCORE(&scorefunction->scorematrix,*uptr,*vptr)) 
               > scolptr->similarity)
      {
        scolptr->similarity = val;
        scolptr->lu = nw.lu + 1;
        scolptr->lv = nw.lv + 1;
      } 
      if ((val = we.similarity + scorefunction->insertionscore) 
               > scolptr->similarity)
      {
        scolptr->similarity = val;
        scolptr->lu = we.lu;
        scolptr->lv = we.lv + 1;
      }
      if(scolptr->similarity < 0)
      {
        scolptr->similarity = 0;
        scolptr->lu = scolptr->lv = 0;
      } else
      {
        if(scolptr->similarity > maxentry->similarity)
        {
	  maxentry->similarity = scolptr->similarity;
          maxentry->len1 = scolptr->lu;
          maxentry->len2 = scolptr->lv;
          maxentry->start1 = (Uint) (uptr - useq) - scolptr->lu + 1;
          maxentry->start2 = (Uint) (vptr - vseq) - scolptr->lv + 1;
	}
      }
      nw = we;
    }
  }
}

/*EE
  The following function computes the maximal traceback edges in the
  Smith-Waterman Dynamic programming table.
*/

void maximalDPedges(Retracebits *edges,SCORE *scol,
                    Scorefunction *scorefunction,
                    Uchar *useq,Uint ulen,Uchar *vseq, Uint vlen)
{
  SCORE val, we, nw, *scolptr;
  Uchar *uptr, *vptr;
  Retracebits *eptr;

  eptr = edges;
  *eptr = 0;
  for(*scol = 0, scolptr = scol+1, uptr = useq, eptr++; uptr < useq + ulen; 
      scolptr++, uptr++, eptr++)
  {
    *scolptr = *(scolptr-1) + scorefunction->deletionscore;
    *eptr = DELETIONBIT;
  }
  for (vptr = vseq; vptr < vseq + vlen; vptr++)
  {
    nw = *scol;
    *scol = nw + scorefunction->insertionscore;
    *eptr = INSERTIONBIT;
    for(scolptr = scol+1, uptr = useq, eptr++; uptr < useq + ulen; 
        scolptr++, uptr++, eptr++)
    {
      we = *scolptr;
      *scolptr = *(scolptr-1) + scorefunction->deletionscore;
      *eptr = DELETIONBIT;
      if ((val = nw + GETSCORE(&scorefunction->scorematrix,*uptr,*vptr)) 
               >= *scolptr)
      {
        if (val == *scolptr)
        {
          *eptr = *eptr | REPLACEMENTBIT;
        } else
        {
          *eptr = REPLACEMENTBIT;
        }
        *scolptr = val;
      } 
      if ((val = we + scorefunction->insertionscore) >= *scolptr)
      {
        if (val == *scolptr)
        {
          *eptr = *eptr | INSERTIONBIT;
        } else
        {
          *eptr = INSERTIONBIT;
        }
        *scolptr = val;
      }
      nw = we;
    }
  }
}

/*EE
  The following function computes an optimal local alignment, given
  a table of retrace-bits, computed by the previous function.
*/

void tracebackDPedges(ArrayEditoperation *alignment,Uint ulen,
                      Uint vlen,Retracebits *edges)
{
  Retracebits *eptr = edges + (ulen+1) * (vlen+1) - 1;
  Uint lenid;

  while(True)
  {
    if (*eptr & REPLACEMENTBIT)
    {
      if(alignment->nextfreeEditoperation == 0)
      {
        STOREINARRAY(alignment,Editoperation,ADDEOP,(Editoperation) 1);
      } else
      {
        lenid = alignment->spaceEditoperation[alignment->
                                              nextfreeEditoperation-1]
                & MAXIDENTICALLENGTH;
        if(lenid > 0 && lenid < MAXIDENTICALLENGTH)
        {
          alignment->spaceEditoperation[alignment->nextfreeEditoperation-1]++;
        } else
        {
          STOREINARRAY(alignment,Editoperation,ADDEOP,(Editoperation) 1);
        }
      }
      eptr -= (ulen+2);
    } else
    {
      if (*eptr & DELETIONBIT)
      {
        STOREINARRAY(alignment,Editoperation,ADDEOP,DELETIONEOP);
        eptr--;
      } else
      {
        if (*eptr & INSERTIONBIT)
        {
          STOREINARRAY(alignment,Editoperation,ADDEOP,INSERTIONEOP);
          eptr -= (ulen+1);
        } else
        {
          break;
        }
      } 
    }
  }
}

//\Ignore{

// #define SHOWCHAR(I)  ("ACGT"[I])
#define SHOWCHAR(I)   (Showuint) (I)

#ifdef DEBUG
SCORE evalalignmentscore(Scorefunction *scorefunction,
                         Uchar *useq, Uint ulen, Uchar *vseq, Uint vlen,
                         Editoperation *alignment,Uint numofeops)
{
  Editoperation *alptr;
  SCORE sumscore = 0;
  Editoperation eop;
  Uchar *endptr, *uptr = useq, *vptr = vseq;

  for(alptr = alignment + numofeops - 1; alptr >= alignment; alptr--)
  {
    eop = *alptr;
    if(eop & MAXIDENTICALLENGTH)
    {
      for(endptr = uptr + (eop & MAXIDENTICALLENGTH); uptr < endptr; 
          uptr++, vptr++)
      {
        sumscore += GETSCORE(&scorefunction->scorematrix,*uptr,*vptr);
        DEBUG5(2,"%s u[%lu]=%lu v[%lu]=%lu =>",
             (*uptr == *vptr) ? "match" : "mismatch",
             (Showuint) (uptr-useq),
             SHOWCHAR(*uptr),
             (Showuint) (vptr-vseq),
             SHOWCHAR(*vptr));
        DEBUG2(2," score = %ld, sumscore = %ld\n",
               (Showsint) GETSCORE(&scorefunction->scorematrix,*uptr,*vptr),
               (Showsint) sumscore);
      }
    } else
    {
      if(eop == INSERTIONEOP)
      {
        sumscore += scorefunction->insertionscore;
        DEBUG4(2,"ins v[%lu]=%lu => score = %ld, sumscore = %ld\n",
             (Showuint) (vptr-vseq),
             SHOWCHAR(*vptr),
             (Showsint) scorefunction->insertionscore,
             (Showsint) sumscore);
        vptr++;
      } else
      {
        if(eop == DELETIONEOP)
        {
          sumscore += scorefunction->deletionscore;
          DEBUG4(2,"del u[%lu]=%lu => score = %ld, sumscore = %ld\n",
             (Showuint) (uptr-useq),
             SHOWCHAR(*uptr),
             (Showsint) scorefunction->deletionscore,
             (Showsint) sumscore);
          uptr++;
        }
      }
    }
  }
  if(uptr != useq + ulen)
  {
    fprintf(stderr,"uptr = %lu != %lu = useq + ulen\n",
            (Showuint) (uptr-useq),(Showuint) ulen);
    exit(EXIT_FAILURE);
  }
  if(vptr != vseq + vlen)
  {
    fprintf(stderr,"vptr = %lu != %lu = vseq + vlen\n",
            (Showuint) (vptr-vseq),(Showuint) vlen);
    exit(EXIT_FAILURE);
  }
  return sumscore;
}
#endif

//}

/*EE
  The following function computes an optimal local alignment
  given two sequences and a score function.
*/

SCORE producealignment(ArrayEditoperation *alignment,
                       Retracebits *edges,SCORE *scol,
                       Scorefunction *scorefunction,
                       Uchar *useq,Uint ulen,Uchar *vseq, Uint vlen)
{
#ifdef DEBUG
  SCORE alscore;
#endif

  maximalDPedges(edges,scol,scorefunction,useq,ulen,vseq,vlen);
  tracebackDPedges(alignment,ulen,vlen,edges);
#ifdef DEBUG
  showeditops(alignment->spaceEditoperation,
              alignment->nextfreeEditoperation,
              stdout);
  alscore = evalalignmentscore(scorefunction,useq,ulen,vseq,vlen,
                               alignment->spaceEditoperation,
                               alignment->nextfreeEditoperation);
  return alscore;
#else
  return 0;
#endif
}
