#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "types.h"
#include "debugdef.h"
#include "frontdef.h"
#include "arraydef.h"
#include "spacedef.h"
#include "minmax.h"
#include "chardef.h"
#include "match.h"
#include "gextenddef.h"

#include "frontSEP.pr"
#include "evalues.pr"

#include "extcmp.c"

static BOOL acceptmatch(Uint distance,
                        Uint length1,Uint pos1,
                        Uint length2,Uint pos2)
{
  Uint nonoverlap;

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
  nonoverlap = (pos2 - pos1) + (pos2 + length2) - (pos1 + length1);
  if(nonoverlap <= distance)  // non-overlapping part is too small
  {
    return False;
  }
  return True;
}

static Sint extenddifferencesleft(Greedyalignreservoir *extendreservoir,
                                  Uint maxdist,
                                  Uint searchlength,
                                  Uchar *seq1,
                                  Sint r1,
                                  Uchar *seq2,
                                  Sint r2)
{
  return extendedleftSEP(extendreservoir,
                         (Sint) maxdist,
                         seq1,r1,seq2,r2,searchlength);
}

static Sint extenddifferencesright(Greedyalignreservoir *extendreservoir,
                                   Uint maxdist,
                                   Uchar *seq1,
                                   Uint last1,
                                   Uint l1,
                                   Uchar *seq2,
                                   Uint last2,
                                   Uint l2)
{
  return extendedrightSEP(extendreservoir,
                          (Sint) maxdist,
                          seq1+l1,(Sint) (last1-l1+UintConst(1)),
                          seq2+l2,(Sint) (last2-l2+UintConst(1)));
}

Sint editextend(BOOL rcmode,
                BOOL querycompare,
                Evalues *evalues,
                Uint maxdist,
                Uint userdefinedleastlength,
                Uint seedlength,
                Match *seed,
                Multiseq *multiseq1,
                Multiseq *multiseq2,
                Match *ematch,
                void *info,
                Processfinalfunction processallmax,
                Processfinalfunction processfinal)
{
  BOOL nextfront, matchfound = False;
  Uchar *seq2;
  Uint temp, pos1, pos2, length1, length2;
  Sint i, 
      tmpext, maxleftextend, maxrightextend,
      lookindex, dist, remain, hleft, hright, 
      exti, extj, lk, rk, *lfptr, *rfptr;
  Frontspec *lfspec, *rfspec;
  Greedyalignreservoir leftextendreservoir,
                       rightextendreservoir;

  DEBUG1(3,"userdefinedleastlength=%lu\n",(Showuint) userdefinedleastlength);
  DEBUG1(3,"seedlength=%lu\n",(Showuint) seedlength);
  DEBUG3(3,"seed=(len=%lu,pos1=%lu,pos2=%lu\n",
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
  initgreedyextendreservoir(&leftextendreservoir); 
  hleft = extenddifferencesleft(&leftextendreservoir,
                                maxdist,
                                seedlength,
                                multiseq1->sequence,
                                (Sint) seed->position1,
                                seq2,
                                (Sint) seed->position2);
  initgreedyextendreservoir(&rightextendreservoir); 
  hright = extenddifferencesright(&rightextendreservoir,
                                  maxdist,
                                  multiseq1->sequence,
                                  multiseq1->totallength-1,
                                  seed->position1 + seed->length1,
                                  seq2,
                                  multiseq2->totallength-1,
                                  seed->position2 + seed->length2);
  if(seed->length1 >= userdefinedleastlength)
  {
    remain = 0;
  } else
  {
    remain = (Sint) (userdefinedleastlength - seed->length1);
  }
  nextfront = True;
  maxleftextend = maxrightextend = 0;
  for(i=hleft; nextfront && i>=0; i--)
  {
    DEBUG1(3,"left: i=%ld\n",(Showsint) i);
    nextfront = False;
    for(lfspec = leftextendreservoir.frontspecreservoir.spaceFrontspec + i,
        lfptr = leftextendreservoir.frontvaluereservoir.spaceFrontvalue +
                lfspec->offset, lk=lfspec->left;
        lk < lfspec->left + lfspec->width; 
        lk++, lfptr++)
    {
      if(*lfptr < 0)
      {
        nextfront = True;
        DEBUG0(3,"tmpext=-INFTY\n");
      } else
      {
        tmpext = *lfptr + lk;
        DEBUG2(3,"orig = %ld, tmpext=%ld\n",(Showsint) *lfptr,
                                            (Showsint) tmpext);
        if(tmpext > maxleftextend)
        {
          maxleftextend = tmpext;
        }
      }
    }
  }
  nextfront = True;
  for(i=hright; nextfront && i>=0; i--)
  {
    DEBUG1(3,"right: i=%ld\n",(Showsint) i);
    nextfront = False;
    for(rfspec = rightextendreservoir.frontspecreservoir.spaceFrontspec + i,
        rfptr = rightextendreservoir.frontvaluereservoir.spaceFrontvalue +
                rfspec->offset, rk=rfspec->left; 
        rk < rfspec->left + rfspec->width; 
        rk++, rfptr++)
    {
      if(*rfptr < 0)
      {
        nextfront = True;
        DEBUG0(3,"tmpext=-INFTY\n");
      } else
      {
        tmpext = *rfptr + rk;
        DEBUG2(3,"orig = %ld, tmpext=%ld\n",(Showsint) *rfptr,
                                            (Showsint) tmpext);
        if(tmpext > maxrightextend)
        {
          maxrightextend = tmpext;
        } 
      }
    }
  }
  DEBUG2(3,"maxleftendend=%ld,maxrightextend=%ld\n",
            (Showsint) maxleftextend,(Showsint) maxrightextend);
  if(maxleftextend + maxrightextend < remain)
  {
    wrapgreedyextendreservoir(&leftextendreservoir);
    wrapgreedyextendreservoir(&rightextendreservoir);
    return 0;
  }
  if(hleft + hright < (Sint) maxdist)
  {
    maxdist = (Uint) (hleft + hright);
  }
  dist = 0;
  matchfound = False;
  for(/* Nothing */ ; dist<=(Sint) maxdist; dist++)
  {
    DEBUG3(3,"dist=%ld,hleft=%ld,hright=%ld\n",(Showsint) dist,
                                               (Showsint) hleft,
                                               (Showsint) hright);
    if(dist < hright)
    {
      lookindex = 0;
    } else
    {
      lookindex = dist - hright;
    }
    for(/* nothing */; lookindex<=MIN(dist,hleft); lookindex++)
    {
      DEBUG1(3,"lookleft=%ld\n",(Showsint) lookindex);
      DEBUG1(3,"lookright=%ld\n",(Showsint) (dist - lookindex));
      lfspec = leftextendreservoir.frontspecreservoir.spaceFrontspec +
               lookindex;
      rfspec = rightextendreservoir.frontspecreservoir.spaceFrontspec + 
               dist - lookindex;
      for(lfptr = leftextendreservoir.frontvaluereservoir.spaceFrontvalue
                  + lfspec->offset, 
                  lk=lfspec->left; 
          lk < lfspec->left + lfspec->width; lk++, lfptr++)
      {
        if(*lfptr >= 0)
        {
          for(rfptr = rightextendreservoir.frontvaluereservoir.spaceFrontvalue
                      + rfspec->offset, 
              rk=rfspec->left; rk < rfspec->left + rfspec->width; rk++, rfptr++)
          {
            if(*rfptr >= 0)
            {
              exti = *lfptr + *rfptr;
              if(exti >= remain)
              {
                extj = exti + lk + rk;
                if(extj >= remain)
                {
                  DEBUG2(3,"combine (lk=%ld,left=%ld)",(Showsint) lk,
                                                       (Showsint) *lfptr); 
                  DEBUG2(3," (rk=%ld,right=%ld)\n",(Showsint) rk,
                                                   (Showsint) *rfptr); 
                  DEBUG2(3,"(exti=%ld,extj=%ld)\n",(Showsint) exti,
                                                   (Showsint) extj);
                  pos1 = (Uint) (seed->position1 - *lfptr);
                  pos2 = (Uint) (seed->position2 - *lfptr - lk);
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
                  if(rcmode || querycompare || 
                     acceptmatch((Uint) dist,length1,pos1,length2,pos2))
                  {
                    if(processallmax != NULL)
                    {
                      matchfound = True;
                      ematch->distance = (Sint) dist;
                      ematch->length1 = length1;
                      ematch->length2 = length2;
                      ematch->position1 = pos1;
                      ematch->position2 = pos2;
                      if(querycompare)
                      {
                        ematch->seqnum2 = seed->seqnum2;
                        ematch->relpos2 
                          = seed->relpos2 - (seed->position2 - pos2);
                      }
                      if(processallmax(info,ematch) != 0)
                      {
                        wrapgreedyextendreservoir(&leftextendreservoir);
                        wrapgreedyextendreservoir(&rightextendreservoir);
                        return (Sint) -1;
                      }
                    } else
                    {
                      if(!matchfound || 
                          cmpmatches(evalues,
                                     ematch->distance,
                                     (Sint) dist,
                                     MAX(ematch->length1,ematch->length2),
                                     MAX(length1,length2),
                                     ematch->position1,
                                     pos2) == (Sint) 1)
                      {
                        matchfound = True;
                        ematch->distance = dist;
                        ematch->length1 = length1;
                        ematch->length2 = length2;
                        ematch->position1 = pos1;
                        ematch->position2 = pos2;
                        if(querycompare)
                        {
                          ematch->seqnum2 = seed->seqnum2;
                          ematch->relpos2
                            = seed->relpos2 - (seed->position2 - pos2);
                        }
                      }
                    }
                  }
                } 
              }
            }
          }
        }
      }
    }
  }
  if(processallmax == NULL && matchfound)
  {
    if(processfinal(info,ematch) != 0)
    {
      wrapgreedyextendreservoir(&leftextendreservoir);
      wrapgreedyextendreservoir(&rightextendreservoir);
      return (Sint) -1;
    }
  }
  wrapgreedyextendreservoir(&leftextendreservoir);
  wrapgreedyextendreservoir(&rightextendreservoir);
  return 0;
}
