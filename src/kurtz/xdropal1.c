
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "spacedef.h"
#include "debugdef.h"
#include "xdropdef.h"
#include "scoredef.h"
#include "alignment.h"

#include "showalign.pr"

/*
  The following type is used when computing optimal alignment 
  w.r.t. to the scores used for the xdrop algorithms.
*/

typedef struct
{
  Xdropscore totalscore,     // the total score of the alignment
             bestscore,      // the best score for all prefixes
             *scol;          // pointer to score table
  Uint ivalue,               // ivalue for best prefix alignment
       jvalue;               // jvalue for best prefix alignment
  Uchar *edges;              // table of bits for backtracing
} Xdropalignspace;           // \Typedef{Xdropalignspace}

/*EE
  The following function computes the DP-matrix for two strings \(u\) and
  \(v\) of length \(m\) and \(n\), respectively. The result is stored
  in the record referred to by \texttt{as}. For the DP-matrix we only store
  one column of distance values at any time. However, we compute an
  \((m+1)\times(n+1)\)-matrix of bytes to facilitate backtracing.
*/

static void minxdropedges(BOOL forward,Xdropalignspace *as,
                          Uchar *useq, Uint ulen, Uchar *vseq, Uint vlen)
{
  Xdropscore tmpval,          // temporary value
             we,              // value in the west
             nw,              // value in the northwest
             score,
             *scolptr;        // points to current value of DP-matrix
  Uint i, j;
  BOOL okay;
  Uchar repbit, *eptr; // pointer to array of bytes

  eptr = as->edges;
  *eptr = (Uchar) 0;

  as->bestscore = 0;
  as->ivalue = as->jvalue = 0;

/*
  Initialize the first column of the matrix
*/
  for(as->scol[0] = 0, scolptr = as->scol+1, i = 0, eptr++; 
      i < ulen; scolptr++, i++, eptr++)
  {
    *scolptr = *(scolptr-1) + INDELSCORE;
    *eptr = DELETIONBIT;
  }

/*
  Compute current column from the previous one.
*/

  for (j = 0; j < vlen; j++)
  {
    nw = as->scol[0];
    as->scol[0] = nw + INDELSCORE;
    *eptr = INSERTIONBIT; 

/*
    Compute new values in place. Therefore store the values
    in the west and the northwest, since the corresponding space is 
    overwritten.
*/

    for(scolptr = as->scol+1, i = 0, eptr++; 
        i < ulen; 
        scolptr++, i++, eptr++)
    {
      we = *scolptr;
      *scolptr = *(scolptr-1) + INDELSCORE;
      *eptr = DELETIONBIT;
      if(forward)
      {
        okay = (useq[i] == vseq[j]) ? True : False;
      } else
      {
        okay = (useq[ulen-1-i] == vseq[vlen-1-j]) ? True : False;
      }
      if(okay)
      {
        score = MATCHSCORE;
        repbit = MATCHBIT;
      } else
      {
        score = MISMATCHSCORE;
        repbit = MISMATCHBIT;
      }
      tmpval = nw + score;
      if (*scolptr == tmpval)
      {
        *eptr |= repbit;      // add REPLACEMENTBIT
      } else
      {
        if(*scolptr < tmpval)
        {
          *eptr = repbit;       // REPLACEMENTBIT is the only bit
          *scolptr = tmpval; // overwrite value with larger one
          
        }
      } 
      tmpval = we + INDELSCORE;
      if (*scolptr == tmpval)
      {
        *eptr |= INSERTIONBIT;       // add INSERTIONBIT
      } else
      {
        if(*scolptr < tmpval)
        { 
          *eptr = INSERTIONBIT;      // INSERTIONBIT is the only bit
          *scolptr = tmpval;
        }
      }
      if(as->bestscore < *scolptr)
      {
        as->bestscore = *scolptr;
        as->ivalue = (Uint) (scolptr - as->scol);
        as->jvalue = j;
      }
      nw = we;
    }
  }
  as->totalscore = as->scol[ulen]; // edit distance is last value in last column
}

/*
  Init the \texttt{Xdropalignspace} record.
*/

static void initxdropalign(Xdropalignspace *as,Uint ulen,Uint vlen)
{
  ALLOCASSIGNSPACE(as->scol,NULL,Xdropscore,ulen+1);
  ALLOCASSIGNSPACE(as->edges,NULL,Uchar,(ulen+1)*(vlen+1));
}

static void freexdropalign(Xdropalignspace *as)
{
  FREESPACE(as->scol);
  FREESPACE(as->edges);
}

/*EE
  Compute one optimal alignment. If there is no direction to
  backtrace, then this must be entry (0,0) in the DP-matrix.
  Hence an optimal alignment is found. The two lines of the 
  alignment are referred to by \texttt{startfirst} and \texttt{startsecond}.
*/

Uint onexdropalignment1(BOOL forward,ArrayEditoperation *alignment,
                        Uchar *useq,Uint ulen,Uchar *vseq, Uint vlen)
{
  Uchar *eptr;
  Editoperation *eopptr;
  Uint lenid, indelcount = 0;
  Xdropalignspace as;

  initxdropalign(&as,ulen,vlen);
  minxdropedges(forward,&as,useq,ulen,vseq,vlen);
  eptr = as.edges+(ulen+1)*as.jvalue+as.ivalue;
  eopptr = alignment->spaceEditoperation - 1;
  while(True)
  {
    if (*eptr & MATCHBIT)
    {
      if(eopptr < alignment->spaceEditoperation)
      {
        eopptr++;  // new edit operation, match of length 1
	*eopptr = (Editoperation) 1;
      } else
      {
        lenid = *eopptr & MAXIDENTICALLENGTH;
        if(lenid > 0 && lenid < MAXIDENTICALLENGTH)
        {
          (*eopptr)++;
        } else
        {
          eopptr++;  // new edit operation
	  *eopptr = (Editoperation) 1;
        }
      } 
      eptr -= (ulen+2);
    } else
    {
      if(*eptr & MISMATCHBIT)
      {
        eopptr++;  // new edit operation
        *eopptr = MISMATCHEOP;
        eptr -= (ulen+2);
      } else
      {
        if (*eptr & INSERTIONBIT)
        {
          eopptr++;  // new edit operation
          *eopptr = INSERTIONEOP;
          indelcount++;
          eptr -= (ulen+1);
        } else
        {
          if (*eptr & DELETIONBIT)
          {
            eopptr++;  // new edit operation
            *eopptr = DELETIONEOP;
            indelcount++;
            eptr--;
          } else
          {
            break;
          }
        }
      }
    }
  }
  freexdropalign(&as);
  alignment->nextfreeEditoperation 
    = (Uint) (eopptr - alignment->spaceEditoperation + 1);
  DEBUGCODE(1,verifyxdropalignment(__FILE__,(Uint) __LINE__,
                                   alignment->spaceEditoperation,
                                   alignment->nextfreeEditoperation,
                                   as.ivalue,as.jvalue,as.bestscore));
  return indelcount;
}
