
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
#include "alignment.h"
#include "arraydef.h"
#include "errordef.h"
#include "chardef.h"
#include "scoredef.h"
#include "failures.h"
#include "xdropdef.h"

#include "showalign.pr"

//}

/*EE
  The following macro stores the next value \texttt{V} in table
  \(R\). Table \(T\) might be accessed for negative indices. For this
  case, the table stores the value \texttt{-xdropbelowscore}.
  The value of \(S'\) is delivered by the macro \texttt{SPRIME}.
*/

#define RTAB(K) scoretab->spaceXdropscore[currentgen->scoreoffset - \
                                          currentgen->smallestk + (K)]

#define NEXTEOP(EOP)\
        STOREINARRAY(alignment,Editoperation,32,EOP)

#define STOREEDITOP savematchlen = matchlen;\
                    if(matchlen > 0)\
                    {\
                      while(True)\
                      {\
                        if(matchlen > (Sint) MAXIDENTICALLENGTH)\
                        {\
                          NEXTEOP((Editoperation) MAXIDENTICALLENGTH);\
                          matchlen -= (Sint) MAXIDENTICALLENGTH;\
                        } else\
                        {\
                          NEXTEOP((Editoperation) matchlen);\
                          break;\
                        }\
                      }\
                    }

/*
  The greedy algorithm works in generations. For each such generation
  we have to store the following values:
*/

static void xdropgbacktrace(ArrayEditoperation *alignment,
                            Galignbest *galignbest,
                            ArrayAlignmentGeneration *generations,
                            ArrayXdropscore *scoretab,
                            SCORE integermin)
{
  Sint d, k, i = 0, tmpi, savematchlen, matchlen, 
       ilastprocessed = galignbest->ivalue,
       jlastprocessed = galignbest->jvalue;
  AlignmentGeneration *currentgen;
  Editoperation eop;

  DEBUG1(1,"ALIGN2:score=%ld\n",(Showsint) galignbest->score);
  DEBUG2(1,"dbest=%ld,kbest=%ld\n",(Showsint) galignbest->dbest,
                                   (Showsint) galignbest->kbest);
  DEBUG2(1,"ivalue=%ld,jvalue=%ld\n",(Showsint) galignbest->ivalue,
                                     (Showsint) galignbest->jvalue);
  k = galignbest->kbest;
  for(d = galignbest->dbest; d > 0; d--)
  {
    currentgen = generations->spaceAlignmentGeneration + d - 1;
    DEBUG2(3,"R(d=%ld,k=%ld)=",(Showsint) d,(Showsint) k);
    DEBUG1(3,"%ld\n",(Showsint) 
            scoretab->spaceXdropscore[(currentgen+1)->scoreoffset -
                                      (currentgen+1)->smallestk+k]);
    i = MINUSINFINITYSCORE;
    eop = 0;
    if(k >= currentgen->smallestk && k <= currentgen->largestk)
    {
      i = RTAB(k);
      DEBUG3(3,"R(d=%ld,k=%ld)=%ld\n",(Showsint) (d-1),
                                      (Showsint) k,
                                      (Showsint) i);
      if(i != MINUSINFINITYSCORE)
      {
        i++;
      }
      eop = MISMATCHEOP;
    }
    if(k+1 >= currentgen->smallestk && k+1 <= currentgen->largestk)
    {
      tmpi = RTAB(k+1);
      DEBUG3(3,"R(d=%ld,k=%ld)=%ld\n",(Showsint) (d-1),
                                      (Showsint) (k+1),
                                      (Showsint) tmpi);
      if(i < tmpi)
      {
        eop = INSERTIONEOP;
        i = tmpi;
      }
    }
    if(k-1 >= currentgen->smallestk && k-1 <= currentgen->largestk)
    {
      tmpi = RTAB(k-1);
      DEBUG3(3,"R(d=%ld,k=%ld)=%ld\n",(Showsint) (d-1),
                                      (Showsint) (k-1),
                                      (Showsint) tmpi);
      if(tmpi != MINUSINFINITYSCORE)
      {
        tmpi++;
      }
      if(i < tmpi)
      {
        eop = DELETIONEOP;
        i = tmpi;
      }
    }
    switch(eop)
    {
      case MISMATCHEOP:  matchlen = ilastprocessed-i;
                         STOREEDITOP;
                         ilastprocessed -= (savematchlen+1);
                         jlastprocessed -= (savematchlen+1);
                         NEXTEOP(MISMATCHEOP); // mismatch of i-1 and i-k-1
                         break;
      case INSERTIONEOP: matchlen = jlastprocessed - (i-k);
                         STOREEDITOP;
                         ilastprocessed -= savematchlen;
                         jlastprocessed -= (savematchlen+1);
                         galignbest->indelcount++;
                         NEXTEOP(INSERTIONEOP); // insertion of i-k-1
                         k++; 
                         break;
      case DELETIONEOP:  matchlen = ilastprocessed-i;
                         STOREEDITOP;
                         ilastprocessed -= (savematchlen+1);
                         jlastprocessed -= savematchlen;
                         galignbest->indelcount++;
                         NEXTEOP(DELETIONEOP); // deletion of i-1
                         k--; 
                         break;
      default: fprintf(stderr,"Unknown edit operation %lu\n",(Showuint) eop);
               exit(EXIT_FAILURE);
    }
  }
  matchlen = ilastprocessed;
  STOREEDITOP;
}

/*EE
  The following function iterates the computation of an xdrop alignment
  with increasing \texttt{xdrop}-scores.
*/

Uint onexdropalignment2(BOOL forward,ArrayEditoperation *alignment,
                        Uchar *useq,Sint ulen,Uchar *vseq,Sint vlen,
                        Xdropscore xdropbelowscore)
{
  ArrayAlignmentGeneration generations;
  Galignbest galignbest;
  ArrayXdropscore scoretab;
  Xdropscore score;
  Sint i;

  INITARRAY(&generations,AlignmentGeneration);
  INITARRAY(&scoretab,Xdropscore);
  for(score = xdropbelowscore, i = 0; i < (Sint) 5; score++, i++)
  {
    generations.nextfreeAlignmentGeneration = 0;
    scoretab.nextfreeXdropscore = 0;
    DEBUG1(2,"try score %ld\n",(Showsint) score);
    if(forward)
    {
      evalxdroptableright(&galignbest,&generations,&scoretab,useq,ulen,
                          vseq,vlen,score);
    } else
    {
      evalxdroptableleft(&galignbest,&generations,&scoretab,useq,ulen,
                         vseq,vlen,score);

    }
    if(galignbest.ivalue == ulen && galignbest.jvalue == vlen)
    {
      break;
    }
  }
  /* XXX */
  alignment->allocatedEditoperation = (Uint) (2 * galignbest.dbest + 1 + 
                                              (ulen-galignbest.ivalue) +
                                              (vlen-galignbest.jvalue));
  ALLOCASSIGNSPACE(alignment->spaceEditoperation,
                   alignment->spaceEditoperation,
                   Editoperation,
                   alignment->allocatedEditoperation);
  /* XXX */
  galignbest.indelcount = 0;
  DEBUG2(2,"ulen=%ld,vlen=%ld\n",(Showsint) ulen,(Showsint) vlen);
  DEBUG2(2,"ivalue=%ld,jvalue=%ld\n",(Showsint) galignbest.ivalue,
                                     (Showsint) galignbest.jvalue);
  if(galignbest.ivalue != ulen || galignbest.jvalue != vlen)
  {
    DEBUG1(2,"align %ld characters at the end of the first sequence\n",
           (Showsint) (ulen-galignbest.ivalue));
    DEBUG1(2,"align %ld characters at the end of the second sequence\n",
           (Showsint) (vlen-galignbest.jvalue));
    if(forward)
    {
      galignbest.indelcount 
        += onexdropalignment1(True,
                              alignment,
                              useq+galignbest.ivalue,
                              (Uint) (ulen-galignbest.ivalue),
                              vseq+galignbest.jvalue,
                              (Uint) (vlen-galignbest.jvalue));
    } else
    {
      galignbest.indelcount 
        += onexdropalignment1(False,
                              alignment,
                              useq,
                              (Uint) (ulen-galignbest.ivalue),
                              vseq,
                              (Uint) (vlen-galignbest.jvalue));
    }
  }
  xdropgbacktrace(alignment,&galignbest,&generations,&scoretab,-MAX(ulen,vlen));
  DEBUGCODE(1,verifyxdropalignment(__FILE__,(Uint) __LINE__,
                                   alignment->spaceEditoperation,
                                   alignment->nextfreeEditoperation,
                                   (Uint) galignbest.ivalue,
                                   (Uint) galignbest.jvalue,
                                   galignbest.score));
  FREEARRAY(&generations,AlignmentGeneration);
  FREEARRAY(&scoretab,Xdropscore);
  return galignbest.indelcount;
}
