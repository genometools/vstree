
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "types.h"
#include "debugdef.h"
#include "arraydef.h"
#include "xdropdef.h"
#include "spacedef.h"
#include "minmax.h"
#include "chardef.h"
#include "match.h"

#include "xdrop.pr"

#define SWAPBESTMATCH(BESTMATCH)\
        temp = (BESTMATCH).ivalue;\
        (BESTMATCH).ivalue = (BESTMATCH).jvalue;\
        (BESTMATCH).jvalue = temp

static BOOL acceptmatch(Uint length1,Uint pos1,Uint length2,Uint pos2)
{
  if(pos1 >= pos2)
  {
    return False;
  }
  if(pos1 + length1 - 1 < pos2)  // no overlap
  {
    return True;
  }
  if(pos1 + length1 >= pos2 + length2)  // embedded
  {
    return False;
  }
  // nonoverlap = (pos2 - pos1) + (pos2 + length2) - (pos1 + length1);
  return True;
}

BOOL xdropseedextend(BOOL rcmode,
                     BOOL querycompare,
                     BOOL revmposorder,
                     Xdropscore xdropbelowscore,
                     Match *seed,
                     Xdropscore seedscore,
                     Multiseq *multiseq1,
                     Multiseq *multiseq2,
                     Uint seedlength,
                     Match *ematch)
{
  Uchar *seq2;
  Uint temp, pos1, pos2, length1, length2;
  Sint exti, extj;
  Xdropbest bestmatchleft, bestmatchright;

  DEBUG5(2,"xdropseedextend(rcmode=%s,querycompare=%s,"
         "seed=(l=%lu,p1=%lu,p2=%lu)\n",
          SHOWBOOL(rcmode),
          SHOWBOOL(querycompare),
          (Showuint) seed->length1,
          (Showuint) seed->position1,
          (Showuint) seed->position2);
  if(rcmode)
  {
    seq2 = multiseq2->rcsequence;
  } else
  {
    seq2 = multiseq2->sequence;
  }
  DEBUG1(3,"seed->position1=%lu\n",(Showuint) seed->position1);
  DEBUG1(3,"seed->position2=%lu\n",(Showuint) seed->position2);
  DEBUG1(3,"seed->length=%lu\n",(Showuint) seed->length1);
  if(xdropbelowscore < 0)
  {
    if(!evalhammingxdropleft(&bestmatchleft,
                             multiseq1->sequence,
                             (Sint) seed->position1,
                             seq2,
                             (Sint) seed->position2,
                             -xdropbelowscore,
                             (Sint) seedlength))
    {
      return False;
    }
    evalhammingxdropright(&bestmatchright,
                          multiseq1->sequence+seed->position1 + seed->length1,
                          (Sint) (multiseq1->totallength - 
                                  (seed->position1+seed->length1)),
                          seq2 + seed->position2 + seed->length2,
                          (Sint) (multiseq2->totallength - 
                                  (seed->position2+seed->length2)),
                          -xdropbelowscore);
  } else
  {
    if(seed->position1 == 0 || 
       multiseq1->sequence[seed->position1-1] == SEPARATOR ||
       seed->position2 == 0 || 
       seq2[seed->position2-1] == SEPARATOR)
    {
      bestmatchleft.ivalue = 0;
      bestmatchleft.jvalue = 0;
      bestmatchleft.score = 0;
    } else
    {
      if(revmposorder)
      {
        evaleditxdropleft(&bestmatchleft,
                          seq2,
                          (Sint) seed->position2,
                          multiseq1->sequence,
                          (Sint) seed->position1,
                          xdropbelowscore);
        SWAPBESTMATCH(bestmatchleft);
      } else
      {
        evaleditxdropleft(&bestmatchleft,
                          multiseq1->sequence,
                          (Sint) seed->position1,
                          seq2,
                          (Sint) seed->position2,
                          xdropbelowscore);
      }
    }
    if(seed->position1 + seed->length1 >= multiseq1->totallength ||
       multiseq1->sequence[seed->position1 + seed->length1] == SEPARATOR ||
       seed->position2 + seed->length2 >= multiseq2->totallength ||
       seq2[seed->position2 + seed->length2] == SEPARATOR)
    {
      bestmatchright.ivalue = 0;
      bestmatchright.jvalue = 0;
      bestmatchright.score = 0;
    } else
    {
      if(revmposorder)
      {
        evaleditxdropright(&bestmatchright,
                           seq2 + seed->position2 + seed->length2,
                           (Sint) (multiseq2->totallength - 
                                   (seed->position2+seed->length2)),
                           multiseq1->sequence+seed->position1+seed->length1,
                           (Sint) (multiseq1->totallength - 
                                   (seed->position1+seed->length1)),
                           xdropbelowscore);
        SWAPBESTMATCH(bestmatchright);
      } else
      {
        evaleditxdropright(&bestmatchright,
                           multiseq1->sequence+seed->position1
                                              +seed->length1,
                           (Sint) (multiseq1->totallength - 
                                   (seed->position1+seed->length1)),
                           seq2 + seed->position2 + seed->length2,
                           (Sint) (multiseq2->totallength - 
                                   (seed->position2+seed->length2)),
                           xdropbelowscore);
      }
    }
  }
  DEBUG1(3,"bestmatchleft.ivalue=%ld\n",(Showsint) bestmatchleft.ivalue);
  DEBUG1(3,"bestmatchleft.jvalue=%ld\n",(Showsint) bestmatchleft.jvalue);
  DEBUG1(3,"bestmatchleft.score=%ld\n",(Showsint) bestmatchleft.score);
  DEBUG1(3,"bestmatchright.ivalue=%ld\n",(Showsint) bestmatchright.ivalue);
  DEBUG1(3,"bestmatchright.jvalue=%ld\n",(Showsint) bestmatchright.jvalue);
  DEBUG1(3,"bestmatchright.score=%ld\n",(Showsint) bestmatchright.score);
  pos1 = (Uint) (seed->position1 - bestmatchleft.ivalue);
  pos2 = (Uint) (seed->position2 - bestmatchleft.jvalue);
  exti = (Sint) (bestmatchleft.ivalue + bestmatchright.ivalue);
  extj = (Sint) (bestmatchleft.jvalue + bestmatchright.jvalue);
  if(rcmode || querycompare || pos1 <= pos2)
  {
    length1 = (Uint) (seed->length1 + exti);
    length2 = (Uint) (seed->length1 + extj);
  } else
  {
    temp = pos1;
    pos1 = pos2;
    pos2 = temp;
    length1 = (Uint) (seed->length1 + extj);
    length2 = (Uint) (seed->length1 + exti);
  }
  if(multiseq1->sequence[pos1 + length1 - 1] == SEPARATOR)
  {
    length1--;
  }
  if(multiseq1->sequence[pos1] == SEPARATOR)
  {
    pos1++;
    length1--;
  }
  if(seq2[pos2 + length2 - 1] == SEPARATOR)
  {
    length2--;
  }
  if(seq2[pos2] == SEPARATOR)
  {
    pos2++;
    length2--;
  }
  if(rcmode || querycompare || acceptmatch(length1,pos1,length2,pos2))
  {
    Xdropscore score;

    score = (Xdropscore) (bestmatchleft.score + bestmatchright.score + 
                          seedscore);
    if(xdropbelowscore < 0)
    {
      score = -score;
    }
    ematch->distance = (Sint) EVALSCORE2DISTANCE(score,length1,length2);
    ematch->length1 = length1;
    ematch->length2 = length2;
    ematch->position1 = pos1;
    ematch->position2 = pos2;
    if(querycompare)
    {
      ematch->seqnum2 = seed->seqnum2;
      ematch->relpos2 = seed->relpos2 - (seed->position2 - pos2);
    }
    return True;
  }
  return False;
}
