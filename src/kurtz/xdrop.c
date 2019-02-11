
//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "spacedef.h"
#include "minmax.h"
#include "debugdef.h"
#include "errordef.h"
#include "chardef.h"
#include "failures.h"
#include "xdropdef.h"

//}

/*EE
  This file implements functions for extending seeds to the left and
  to the right using an xdrop-strategy. That is, the extension computes
  in each step the maximum score \(S_{\max}\) seen so far. If the next 
  score computed is smaller than \(S_{\max}-\texttt{xdropbelowscore}\),
  then the extension is stopped.
*/

/*EE
  The following function applies the Xdrop-strategy allowing only matches
  and mismatches. It extends the two strings \texttt{useq} and \texttt{vseq}
  of length \texttt{ulen} and \texttt{vlen} from left to right, starting
  with the first character of these strings.
*/

void evalhammingxdropright(Xdropbest *bestmatch,Uchar *useq,Sint ulen,
                           Uchar *vseq,Sint vlen,
                           Xdropscore xdropbelowscore)
{
  Uchar a, b, *uptr, *vptr, *bestend = useq - 1;
  Xdropscore totalscore = 0, bestscore = 0;

  for(uptr = useq, vptr = vseq; 
      uptr < useq + ulen && vptr < vseq + vlen; 
      uptr++, vptr++)
  { 
    a = *uptr;
    if(a == SEPARATOR)
    {
      break;
    }
    b = *vptr;
    if(b == SEPARATOR)
    {
      break;
    }
    if(a != b || a == (Uchar) WILDCARD)
    {
      totalscore += MISMATCHSCORE;
    } else
    {
      totalscore += MATCHSCORE;
    }
    if(totalscore < bestscore - xdropbelowscore)
    {
      break;
    }
    if(bestscore < totalscore)
    {
      bestscore = totalscore;
      bestend = uptr;
    }
  }
  bestmatch->ivalue = bestmatch->jvalue = (Uint) (bestend - useq + 1);
  bestmatch->score = bestscore;
}

/*EE
  The following function applies the Xdrop-strategy allowing only matches
  and mismatches. It extends the two strings \texttt{useq} and \texttt{vseq}
  of length \texttt{ulen} and \texttt{vlen} from right to left, starting
  with the last character of these strings. If a region of 
  \texttt{reachlength} consecutive mismatches is found, the function
  returns \texttt{False}. If no such region occurs, then the function
  returns \texttt{True}.
*/

BOOL evalhammingxdropleft(Xdropbest *bestmatch,
                          Uchar *useq,Sint ulen,
                          Uchar *vseq,Sint vlen,
                          Xdropscore xdropbelowscore,
                          Sint reachlength)
{
  Uchar a, b, *uptr, *vptr, *bestend = useq + ulen;
  Sint matchlen = 0;
  Xdropscore totalscore = 0, 
             bestscore = 0;

  for(uptr = useq + ulen - 1, vptr = vseq + vlen - 1; 
      uptr >= useq && vptr >= vseq; 
      uptr--, vptr--)
  { 
    a = *uptr;
    if(a == SEPARATOR)
    {
      break;
    }
    b = *vptr;
    if(b == SEPARATOR)
    {
      break;
    }
    if(a != b || a == (Uchar) WILDCARD)
    {
      matchlen = 0;
      totalscore += MISMATCHSCORE;
    } else
    {
      matchlen++;
      if(matchlen >= reachlength)
      {
        return False;
      }
      totalscore += MATCHSCORE;
    }
    if(totalscore < bestscore - xdropbelowscore)
    {
      break;
    }
    if(bestscore < totalscore)
    {
      bestscore = totalscore;
      bestend = uptr;
    } 
  }
  bestmatch->ivalue = bestmatch->jvalue = (Uint) (useq + ulen - bestend);
  bestmatch->score = bestscore;
  return True;
}

/*
  To allow also insertions and deletions for the extension, we
  apply a greedy algorithm of Miller et. al. 2000.
  This algorithm requires some auxiliary tables. Usually these
  tables are small. So allocate static arrays to store these tables.
  In case more space is needed, the working memory will be 
  dynamically allocated.
*/

#define STATICSCORESPACE   256
#define STATICDISTSPACE    256

/*
  The following macro stores the next value \texttt{V} in table
  \(R\). Table \(T\) might be accessed for negative indices. For this
  case, the table stores the value \texttt{-xdropbelowscore}.
  The value of \(S'\) is delivered by the macro \texttt{SPRIME}.
*/

#define NEWRVALUEMINUSINFINITYSCORE\
        *nextcurrentscore++ = MINUSINFINITYSCORE
#define NEWRVALUE(V)\
        if((V) <= MINUSINFINITYSCORE)\
        {\
          fprintf(stderr,"program error: V=%ld>=%ld=-oo)\n",\
                  (Showsint) (V),(Showsint) MINUSINFINITYSCORE);\
          exit(EXIT_FAILURE);\
        }\
        *nextcurrentscore++ = V

#define STORENEWRVALUE(V)\
        STOREINARRAY(scoretab,Xdropscore,128,V)
#define TTAB(D)\
        (((D) < 0) ? -xdropbelowscore : Ttabptr[D])
#define SPRIME(L)\
        ((L) * HALFMATCHSCORE - dmulti)

#define COMPARESYMBOLSSEP(I,J)\
        USEQ(a,I);\
        if(a == (Uchar) SEPARATOR)\
        {\
          ulen = I;\
          break;\
        }\
        VSEQ(b,J);\
        if(b == (Uchar) SEPARATOR)\
        {\
          vlen = J;\
          break;\
        }\
        if(a != b || a == (Uchar) WILDCARD)\
        {\
          break;\
        }

/*
  The greedy algorithm works in generations. For each such generation
  we have to store the following values:
*/

typedef struct
{
  Sint smallestk,      // smallest value of k
       allocated;      // number of cells allocated
  Xdropscore *scores, // reference to memory area to store the scores
             scorespace[STATICSCORESPACE];  // static area used for most cases
} Generation;

/*
  Since the functions for left and right extension are very similar,
  we only want to have the corresponding source file once. Therefore,
  we use macros to abstract from the differences. The 
  following 7 macros are defined for the left to right extension beginning
  with the first character of the strings under consideration.
*/


//\Ignore{

#define FREEXDROPSPACE\
        if(generation1.allocated > (Sint) STATICSCORESPACE)\
        {\
          FREESPACE(generation1.scores);\
        }\
        if(generation2.allocated > (Sint) STATICSCORESPACE)\
        {\
          FREESPACE(generation2.scores);\
        }\
        if(Ttabsize > (Sint) STATICDISTSPACE)\
        {\
          FREESPACE(Ttabptr);\
        }

//}
    
#define EVALXDROPEDIT     EVALXDROPEDITRIGHT
#define EVALXDROPTABLE    evalxdroptableright
#define USEQ(A,I)         A = useq[I]
#define VSEQ(A,J)         A = vseq[J]

#include "xdrop.gen"

#undef EVALXDROPEDIT
#undef EVALXDROPTABLE
#undef USEQ
#undef VSEQ

/*
  Now we redefine the macros to compute the left to right
  we use macros to abstract from the differences. The 
  following 4 macros are defined for the right to left extension beginning
  with the last character of the strings under consideration.
*/

#define EVALXDROPEDIT     EVALXDROPEDITLEFT
#define EVALXDROPTABLE    evalxdroptableleft
#define USEQ(A,I)         A = useq[ulen-1-(I)]
#define VSEQ(A,J)         A = vseq[vlen-1-(J)]

#include "xdrop.gen"
