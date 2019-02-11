#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "debugdef.h"
#include "dpbitvec48.h"
#include "chardef.h"

typedef signed short Smalllength;

#define UNDEFINDEX      ((Smalllength) (-1))

Uint cmpmaxdistgeneric(DPbitvector4 *Eqs4,Uint ulen,
                       Uchar *vseq,Uint vlen,Uint maxdist)
{
  DPbitvector4 Eq, Xv, Xh, Ph, Mh, Pv, Mv, // as in Myers Paper
               backmask;           // only one bit is on
  Smalllength idx, maxleqk;
  Uchar *vptr;
  Uint score = ulen;

  Pv = (DPbitvector4) ~0;
  Mv = (DPbitvector4) 0;
  maxleqk = (Smalllength) maxdist;
  for(vptr = vseq; vptr < vseq + vlen; vptr++)
  {
    DEBUG1(3,"currentchar %lu\n",(Showuint) *vptr);
    if(*vptr == SEPARATOR)
    {
      fprintf(stderr,"programming error: cannot align sequence "
                     "containing separators\n");
      exit(EXIT_FAILURE);
    }
    Eq = Eqs4[(Uint) *vptr];
    Xv = Eq | Mv;
    Xh = (((Eq & Pv) + Pv) ^ Pv) | Eq;

    Ph = Mv | ~ (Xh | Pv);
    Mh = Pv & Xh;

    Ph = (Ph << 1) | ONEDPbitvector4;
    Pv = (Mh << 1) | ~ (Xv | Ph);
    Mv = Ph & Xv;
    DEBUG1(3,"score %lu\n",(Showuint) score);
    DEBUG1(3,"maxleqk %ld\n",(Showsint) maxleqk);
    backmask = ONEDPbitvector4 << (Uint) maxleqk;  
	       // \(i\)th bit is on iff i=maxleqk
  /*
    Suppose \(i=maxleqk\).
    case (a): Suppose that that the value at index i+1 in the current column 
      is computed along the diagonal or the value along the horizontal
      is decremented. Then it must be \(k\). So the maxleqk-value of the
      current column is maxleqk + 1. 
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
      maxleqk = maxleqk + (Smalllength) 1;
    } else
    {
      if(Ph & backmask)                    // case (b)
      {
	score = maxdist+1;
	maxleqk = UNDEFINDEX;
	for(idx = maxleqk - (Smalllength) 1, backmask >>= 1; 
	    idx >= 0; 
	    idx--, backmask >>= 1)
	{
	  if(Pv & backmask)
	  {
	    score--;
	    if(score <= maxdist)
	    {
	      maxleqk = idx;
	      break;
	    }
	  } else
	  {
	    if(Mv & backmask)
	    {
	      score++;
	    }
	  }
	}
      }
    }
    if(maxleqk == UNDEFINDEX)
    {
      return maxdist+1;
    }
  }
  return score;
}

Uint distboundarycases(BOOL *isboundcase,
                       Uchar *useq,Uint ulen,Uchar *vseq,Uint vlen)
{
  *isboundcase = True;
  if(ulen == 0)
  {
    return vlen;
  }
  if(vlen == 0)
  {
    return ulen;
  }
  if(ulen == UintConst(1) && vlen == UintConst(1))
  {
    if(useq[0] != vseq[0] || ISSPECIAL(useq[0]))
    {
      return UintConst(1);
    }
    return 0;
  }
  *isboundcase = False;
  return 0;
}

Uint cmpmaxdist(Uint mapsize,Uchar *useq,Uint ulen,
                Uchar *vseq,Uint vlen,Uint maxdist)
{
  DPbitvector4 eqsspace[EQSSIZE];
  Uint realdist;
  BOOL isboundcase;

  realdist = distboundarycases(&isboundcase,useq,ulen,vseq,vlen);
  if(isboundcase)
  {
    if(realdist > maxdist)
    {
      printf("cmpmaxdist returns %lu\n",(Showuint) (maxdist+1));
      return maxdist + 1;
    }
    printf("cmpmaxdist returns %lu\n",(Showuint) realdist);
    return realdist;
  }
  if(ulen < vlen)
  {
    getEqs4(eqsspace,mapsize,useq,ulen);
    return cmpmaxdistgeneric(eqsspace,ulen,vseq,vlen,maxdist);
  }
  getEqs4(eqsspace,mapsize,vseq,vlen);
  return cmpmaxdistgeneric(eqsspace,vlen,useq,ulen,maxdist);
}
