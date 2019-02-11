
#include <stdio.h>
#include "match.h"
#include "debugdef.h"
#include "mparms.h"

BOOL matchokay(Uint length1,
               Uint startpos1,
               Uint length2,
               Uint startpos2,
               Sint distance,
               Evaluetype evalue,
               Matchflag flag,
               Matchparam *mp)
{
  if(length1 < mp->userdefinedleastlength ||
     length2 < mp->userdefinedleastlength)
  {
    DEBUG1(2,"length-reject: userdefinedleastlength=%lu, ",
             (Showuint) mp->userdefinedleastlength);
    DEBUG2(2,"length1=%lu, length2=%lu\n",(Showuint) length1,
                                          (Showuint) length2);
    return False;
  }
  if(mp->identity > 0)
  {
    double identity;
    
    EVALIDENTITY(identity,distance,length1,length2);
    if(identity < (double) mp->identity)
    {
      return False;
    }
  }
  if(mp->xdropleastscore != UNDEFXDROPLEASTSCORE)
  {
    Sint score = (Sint) EVALDISTANCE2SCORE(distance,length1,length2);
    
    DEBUG1(2,"xdropleastscore=%ld\n",(Showsint) mp->xdropleastscore);
    if(mp->xdropleastscore >= 0)
    {
      if(score < mp->xdropleastscore)
      {
        DEBUG2(2,"score-reject(1): minscore=%ld, score=%ld\n",
                 (Showsint) mp->xdropleastscore,
                 (Showsint) score);
        return False;
      }
    } else
    {
      if(distance == 0)
      {
        if(score < ABS(mp->xdropleastscore))
        {
          return False;
        }
      } else
      {
        if(score > mp->xdropleastscore)
        {
          DEBUG2(2,"score-reject(2): minscore=%ld, score=%ld\n",
                         (Showsint) mp->xdropleastscore,
                         (Showsint) score);
          return False;
        }
      }
    }
  }
  if(mp->maximumevalue != UNDEFMAXIMUMEVALUE)
  {
    if(evalue > mp->maximumevalue)
    {
      DEBUG2(2,"evalue-reject(2): maximumevalue=%.2e, evalue=%.2e\n",
	       mp->maximumevalue,evalue);
      return False;
    }
  }
  if(mp->repeatgapspec.lowergapdefined)
  {
    Sint gaplength;

    if(startpos1 > startpos2)
    {
      fprintf(stderr,"matchokay: startpos1 = %lu > %lu = startpos2 not "
                     "expected\n",(Showuint) startpos1,(Showuint) startpos2);
      exit(EXIT_FAILURE);
    }
    if (startpos1 == startpos2 && !(flag & FLAGSELFPALINDROMIC))
    {
      fprintf(stderr,"matchokay: startpos1 = %lu == %lu = startpos2 not "
                     "expected for direct matches\n",
                     (Showuint) startpos1,(Showuint) startpos2);
      exit(EXIT_FAILURE);
    }
    if(startpos1 + length1 - 1 > startpos2)
    {
      gaplength = (Sint) (startpos1 + length1 - startpos2);
      gaplength = -gaplength;
    } else
    {
      gaplength = (Sint) (startpos2 - (startpos1 + length1));
    }
    if(gaplength < mp->repeatgapspec.lowergaplength)
    {
      return False;
    }
    if(mp->repeatgapspec.uppergapdefined &&
       gaplength > mp->repeatgapspec.uppergaplength)
    {
      return False;
    }
  }
  return True;
}
