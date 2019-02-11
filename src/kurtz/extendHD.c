

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "types.h"
#include "debugdef.h"
#include "minmax.h"
#include "match.h"
#include "chardef.h"

#include "evalues.pr"
#include "extcmp.c"

#define STATICLOOKCELLS 1
#define INCLOOKCELLS    2

static void allocatelooktable(ArrayUint *looktable,
                              Uint *statictable,
                              Uint maxval)
{
  if(maxval >= (Uint) STATICLOOKCELLS)
  {
    Uint extraspace = maxval + 1 - STATICLOOKCELLS;
    
    if(extraspace < (Uint) INCLOOKCELLS)
    {
      extraspace = (Uint) INCLOOKCELLS;
    }
    looktable->allocatedUint = (Uint) STATICLOOKCELLS + extraspace;
    ALLOCASSIGNSPACE(looktable->spaceUint,
                     NULL,
                     Uint,
                     looktable->allocatedUint);
  } else
  {
    looktable->allocatedUint = (Uint) STATICLOOKCELLS;
    looktable->nextfreeUint = 0;
    looktable->spaceUint = statictable;
  }
}

static void wraplooktable(ArrayUint *looktable)
{
  if(looktable->allocatedUint > (Uint) STATICLOOKCELLS)
  {
    FREEARRAY(looktable,Uint);
  }
}

static Uint extendmismatchesleft(Uint *lookleft,
                                 Uint maxdist,
                                 Uint searchlength,
                                 Uchar *seq1,Uint r1,
                                 Uchar *seq2,Uint r2)
{
  Uint h, i1, i2;
  Uchar a, b;

  DEBUG2(3,"extendmismatchesleft(%lu/%lu)\n",(Showuint) r1,(Showuint) r2);
  for(h = 0; h<=maxdist; h++)
  {
    lookleft[h] = 0;
  }
  for(h = UintConst(1), i1 = r1, i2 = r2; /* Nothing */; i1--, i2--)
  {
    a = seq1[i1];
    b = seq2[i2];
    DEBUG4(3,"left: compare seq[%lu]=%lu and seq[%lu]=%lu\n",
           (Showuint) i1,(Showuint) a,(Showuint) i2,(Showuint) b);
    /* The following if statements can be optimized:
       first check for equality, if equal then check if special,
       if special then stop for SEPARATOR */
    if(a == SEPARATOR || b == SEPARATOR)
    {
      lookleft[h] = (Uint) (r1 - i1 + 1);
      DEBUG2(3,"(2) lookleft[%lu]=%lu,",(Showuint) h,(Showuint) lookleft[h]);
      break;
    }
    if(a != b || a == (Uchar) WILDCARD)
    {
      lookleft[h] = (Uint) (r1 - i1 + 1);
      DEBUG2(3,"(1) lookleft[%lu]=%lu,",(Showuint) h,(Showuint) lookleft[h]);
      if(h == maxdist || lookleft[h] - lookleft[h-1] > searchlength)
      {
        DEBUG2(3,"h=maxdist=%lu or seed of length %lu detected\n",
                  (Showuint) maxdist,
                  (Showuint) (lookleft[h]-lookleft[h-1]-1));
        break;
      }
      h++;
    }
    if(i1 == 0 || i2 == 0)
    {
      lookleft[h] = (Uint) (r1 - i1 + 2);
      DEBUG2(3,"(3) lookleft[%lu]=%lu,",(Showuint) h,(Showuint) lookleft[h]);
      break;
    }
  }
  DEBUG1(3,"h=%lu\n",(Showuint) h);
  if(lookleft[h] - lookleft[h-1] > searchlength)
  {
    DEBUG2(3,"h=maxdist=%lu or seed of length %lu detected\n",
              (Showuint) maxdist,
              (Showuint) (lookleft[h]-lookleft[h-1]-1));
    return h-1;
  }
  return h;
}

static Uint extendmismatchesright(Uint *lookright,
                                  Uint maxdist,
                                  Uchar *seq1,
                                  Uint last1,
                                  Uint l1,
                                  Uchar *seq2,
                                  Uint last2,
                                  Uint l2)
{
  Uint h, i1, i2;
  Uchar a, b;

  DEBUG2(3,"extendmismatchesright %lu/%lu\n",(Showuint) l1,(Showuint) l2);
  for(h = 0; h<=maxdist; h++)
  {
    lookright[h] = 0;
  }
  for(h = UintConst(1), i1 = l1, i2 = l2; /* Nothing */; i1++, i2++)
  {
    a = seq1[i1];
    b = seq2[i2];
    DEBUG4(3,"right: compare seq[%lu]=%lu and seq[%lu]=%lu\n",
           (Showuint) i1,(Showuint) a,(Showuint) i2,(Showuint) b);
    if(a == SEPARATOR || b == SEPARATOR)
    {
      lookright[h] = (Uint) (i1 - l1 + 1);
      DEBUG2(3,"(2) lookright[%lu]=%lu,",(Showuint) h,(Showuint) lookright[h]);
      break;
    }
    if(a != b || a == (Uchar) WILDCARD)
    {
      lookright[h] = (Uint) (i1 - l1 + 1);
      DEBUG2(3,"(1) lookright[%lu]=%lu,",(Showuint) h,(Showuint) lookright[h]);
      if(h == maxdist)
      {
        break;
      }
      h++;
    }
    if(i1 == last1 || i2 == last2)
    {
      lookright[h] = (Uint) (i1 - l1 + 2);
      DEBUG2(3,"(3) lookright[%lu]=%lu,",(Showuint) h,(Showuint) lookright[h]);
      break;
    }
  }
  DEBUG1(3,"h=%lu\n",(Showuint) h);
  return h;
}

Sint hammingextend(BOOL rcmode,
                   BOOL querycompare,
                   Evalues *evalues,
                   Uint maxdist,
                   Uint userdefinedleastlength,
                   Uint seedlength,
                   Match *seed,
                   Multiseq *multiseq1,
                   Multiseq *multiseq2,
                   Match *hmatch,
                   void *info,
		   Processfinalfunction processallmax,
		   Processfinalfunction processfinal)
{
  BOOL matchfound = False;
  Uint hleft, hright, remain, dist, lookindex, ll, extlength, r1, r2,
       lookleftspace[STATICLOOKCELLS], 
       lookrightspace[STATICLOOKCELLS];
  Uchar *seq2;
  ArrayUint lookleft, lookright;

  allocatelooktable(&lookleft,&lookleftspace[0],maxdist);
  allocatelooktable(&lookright,&lookrightspace[0],maxdist);
  DEBUG1(3,"userdefinedleastlength=%lu\n",(Showuint) userdefinedleastlength);
  DEBUG1(3,"seedlength=%lu\n",(Showuint) seedlength);
  if(rcmode)
  {
    seq2 = multiseq2->rcsequence;
  } else
  {
    seq2 = multiseq2->sequence;
  }
  if(seed->position1 > UintConst(1) && seed->position2 > UintConst(1))
  {
    if(multiseq1->sequence[seed->position1-1] == SEPARATOR ||
       seq2[seed->position2-1] == SEPARATOR)
    {
      hleft = 0;
      lookleft.spaceUint[0] = 0;
    } else
    {
      hleft = extendmismatchesleft(lookleft.spaceUint,
                                   maxdist,
                                   seedlength,
                                   multiseq1->sequence,
                                   seed->position1-2,
                                   seq2,
                                   seed->position2-2);
    }
  } else
  {
    if(seed->position1 == 0 || seed->position2 == 0 ||
       multiseq1->sequence[seed->position1-1] == SEPARATOR ||
       seq2[seed->position2-1] == SEPARATOR)
    {
      hleft = 0;
      lookleft.spaceUint[0] = 0;
    } else
    {
      hleft = UintConst(1);
      lookleft.spaceUint[0] = 0;
      lookleft.spaceUint[1] = UintConst(1);
    }
  }
  r1 = seed->position1 + seed->length1;
  r2 = seed->position2 + seed->length2;
  if(r1 < multiseq1->totallength - 1 && 
     r2 < multiseq2->totallength - 1)
  {
    if(multiseq1->sequence[r1] == SEPARATOR || seq2[r2] == SEPARATOR)
    {
      hright = 0;
      lookright.spaceUint[0] = 0;
    } else
    {
      hright = extendmismatchesright(lookright.spaceUint,
                                     maxdist,
                                     multiseq1->sequence,
                                     multiseq1->totallength-1,
                                     r1+1,
                                     seq2,
                                     multiseq2->totallength-1,
                                     r2+1);
    }
  } else
  {
    if(r1 >= multiseq1->totallength || r2 >= multiseq2->totallength ||
       multiseq1->sequence[r1] == SEPARATOR ||
       seq2[r2] == SEPARATOR)
    {
      hright = 0;
      lookright.spaceUint[0] = 0;
    } else
    {
      hright = UintConst(1);
      lookright.spaceUint[0] = 0;
      lookright.spaceUint[1] = UintConst(1);
    }
  }
  if(seed->length1 >= userdefinedleastlength)
  {
    remain = 0;
  } else
  {
    remain = userdefinedleastlength - seed->length1;
  }
  matchfound = False;
  DEBUG3(3,"remain=%lu,lookleft[%lu]=%lu,",(Showuint) remain,(Showuint) hleft,
                                           (Showuint) lookleft.spaceUint[hleft]);
  DEBUG2(3,"lookright[%lu]=%lu\n",(Showuint) hright,
                                  (Showuint) lookright.spaceUint[hright]);
  if(lookleft.spaceUint[hleft] + lookright.spaceUint[hright] < remain)
  {
    wraplooktable(&lookleft);
    wraplooktable(&lookright);
    return 0;
  }
  if(hleft + hright < maxdist)
  {
    maxdist = hleft + hright;
  } 
  if(processallmax != NULL)
  {
    //dist = (Sint) maxdist;   XXXXX
    dist = 0;
  } else
  {
    dist = 0;
  }
  for(/* Nothing */ ; dist <= maxdist; dist++)
  {
    DEBUG3(3,"dist=%lu,hleft=%lu,hright=%lu\n",(Showuint) dist,
                                               (Showuint) hleft,
                                               (Showuint) hright);
    if(dist < hright)
    {
      lookindex = 0;
    } else
    {
      lookindex = dist - hright;
    }
    for(/* Nothing */; lookindex<=MIN(dist,hleft); lookindex++)
    {
      DEBUG2(3,"(i,j)=(%lu,%lu)\n",(Showuint) lookindex,
                                   (Showuint) (dist-lookindex));
      ll = lookleft.spaceUint[lookindex];
      extlength = ll + lookright.spaceUint[dist-lookindex];
      if(extlength >= remain)
      {
        if(processallmax != NULL)
        {
          matchfound = True;
          hmatch->distance = -(Sint) dist;
          hmatch->length1 = hmatch->length2 = seed->length1 + extlength;
          hmatch->position1 = seed->position1 - ll;
          hmatch->position2 = seed->position2 - ll;
          if(querycompare)
          {
            hmatch->seqnum2 = seed->seqnum2;
            hmatch->relpos2 = seed->relpos2 - ll;
          }
          if(processallmax(info,hmatch) != 0)
          {
            wraplooktable(&lookleft);
            wraplooktable(&lookright);
            return (Sint) -1;
          }
        } else
        {
          if(!matchfound || 
             cmpmatches(evalues,
                        hmatch->distance,
                        -(Sint) dist,
                        hmatch->length1,
                        seed->length1+extlength,
                        hmatch->position1,
                        seed->position2 - ll) == (Sint) 1)
          {
            matchfound = True;
            hmatch->distance = -(Sint) dist;
            hmatch->length1 = hmatch->length2 = seed->length1 + extlength;
            hmatch->position1 = seed->position1 - ll;
            hmatch->position2 = seed->position2 - ll;
            if(querycompare)
            {
              hmatch->seqnum2 = seed->seqnum2;
              hmatch->relpos2 = seed->relpos2 - ll;
            }
          }
        } 
      }
    }
  }
  if(processallmax == NULL)
  {
    if(matchfound)
    {
      if(processfinal(info,hmatch) != 0)
      {
        wraplooktable(&lookleft);
        wraplooktable(&lookright);
        return (Sint) -2;
      }
    }
  }
  wraplooktable(&lookleft);
  wraplooktable(&lookright);
  return 0;
}
