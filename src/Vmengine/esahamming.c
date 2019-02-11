
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "debugdef.h"
#include "spacedef.h"
#include "chardef.h"
#include "errordef.h"
#include "types.h"
#include "virtualdef.h"

#define SETLVALUE\
        if(vlen < plen)\
        {\
          lvalue = vlen-1;\
        } else\
        {\
          lvalue = plen-1;\
        }

#define APMSUCCESS(IDX)\
        DEBUG1(3,"success for index %lu\n",(Showuint) (IDX));\
        if(processapmstartpos == NULL)\
        {\
          FREESPACE(errorstack);\
          return (Sint) 1;\
        } else\
        {\
          if(processapmstartpos(virtualtree->suftab[IDX],\
                                errorstack[plen-1],\
                                apmoutinfo) != 0)\
          {\
            FREESPACE(errorstack);\
            return (Sint) -1;\
          }\
        }

static Sint evaluateremaininghamming(Uchar *pattern,
                                     Uint threshold,
                                     Uchar *vptr,
                                     Uint *errorstack,
                                     Uint startindex,
                                     Uint lvalue)
{
  Sint d;
  Uchar currentchar;
  Uint mmcount;

  if(startindex > lvalue)
  {
    return (Sint) lvalue;
  }
  DEBUG2(3,"evaluateremaininghamming(%lu,%lu)\n",(Showuint) startindex,
                                                 (Showuint) lvalue);
  if(startindex == 0)
  {
    mmcount = 0;
  } else
  {
    mmcount = errorstack[startindex-1];
  }
  for(d=(Sint) startindex; d<=(Sint) lvalue; d++)
  {
    currentchar = vptr[d];
    if(currentchar == SEPARATOR)
    {
      DEBUG1(3,"evaluateremaininghamming returns %ld\n",(Showsint) (d-1));
      return d-1;
    }
    if(currentchar != pattern[d])
    {
      mmcount++;
    }
    if(mmcount > threshold)
    {
      DEBUG1(3,"evaluateremaininghamming returns %ld\n",(Showsint) (d-1));
      return d-1;
    }
    errorstack[d] = mmcount;
    DEBUG2(3,"errorstack[%ld]=%lu\n",(Showsint) d,(Showuint) mmcount);
  }
  DEBUG1(3,"evaluateremaining returns %ld\n",(Showsint) lvalue);
  return (Sint) lvalue;
}

Sint esahamming(Sint (*processapmstartpos)(Uint,Uint,void *),
                void *apmoutinfo,
                Virtualtree *virtualtree,
                Uchar *pattern,
                Uint plen,
                Uint threshold)
{
  Sint dvalue;
  Uint idx, vlen, lvalue, lcpvalue, current, *errorstack;
  Uchar *vptr;
 
  ALLOCASSIGNSPACE(errorstack,NULL,Uint,plen);
  vptr = virtualtree->multiseq.sequence + virtualtree->suftab[0];
  vlen = virtualtree->multiseq.totallength - virtualtree->suftab[0];
  SETLVALUE;
  dvalue = evaluateremaininghamming(pattern,
                                    threshold,
                                    vptr,
                                    errorstack,
                                    0,
                                    lvalue);
  if(dvalue == (Sint) (plen-1))
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
    SETLVALUE;
    lcpvalue = virtualtree->lcptab[idx];
    if(dvalue+1 >= (Sint) lcpvalue)
    {
      dvalue = evaluateremaininghamming(pattern,
                                        threshold,
                                        vptr,
                                        errorstack,
                                        lcpvalue,
                                        lvalue);
      DEBUG2(3,"line %lu, dvalue = %lu\n",
             (Showuint) __LINE__,(Showuint) dvalue);
      if(dvalue == (Sint) (plen-1))
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
        if(dvalue + 1 >= (Sint) lcpvalue)
        {
          break;
        }
      }
      if(dvalue == (Sint) (plen-1))
      {
        while(current < idx)
        {
          APMSUCCESS(current);
          current++;
        }
      }
    }
  }
  FREESPACE(errorstack);
  return 0;
}
