#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "divmodmul.h"
#include "debugdef.h"
#include "spacedef.h"
#include "errordef.h"
#include "chardef.h"
#include "arraydef.h"
#include "genfile.h"
#include "virtualdef.h"

#include "readgzip.pr"
#include "detnumofcodes.pr"
#include "distri.pr"

#include "mkvaux.h"

#ifdef SUFFIXPTR
#define CHECKTERMINATION if(tptr == virtualtree->multiseq.sequence) break
#else
#define CHECKTERMINATION if(tptr == 0) break
#endif

#ifdef DEBUG
static void bucketstatistic(MKVaux *mkvaux,Uint maximalbsize,Uint numofcodes)
{
  Uint i, bsizemin, bsizemax, bucketsize, *border, witness;
  ArrayUint distribution;

  bsizemin = maximalbsize;
  bsizemax = mkvaux->leftborder[0];
  witness = 0;
  INITARRAY(&distribution,Uint);
  for(border = mkvaux->leftborder+1; 
      border <= mkvaux->leftborder + numofcodes;
      border++)
  {
    bucketsize = *border - *(border-1);
    if(bsizemin > bucketsize)
    {
      bsizemin = bucketsize;
    }
    if(bsizemax < bucketsize)
    {
      bsizemax = bucketsize;
      witness = (Uint) (border - mkvaux->leftborder);
    }
    adddistribution(&distribution,bucketsize);
  }
  printf("# bucket %lu: max bucket size: %lu\n",(Showuint) witness,
                                                (Showuint) bsizemax);
  printf("# min bucket size: %lu\n",(Showuint) bsizemin);
  NOTSUPPOSEDTOBENULL(distribution.spaceUint);
  for(i=0; i<distribution.nextfreeUint; i++)
  {
    if(distribution.spaceUint[i] > 0)
    {
      printf("# bucketsize %lu: %lu times\n",
             (Showuint) i,
             (Showuint) distribution.spaceUint[i]);
    }
  }
  FREEARRAY(&distribution,Uint);
  if(bsizemax != mkvaux->maxbucketsize)
  {
    fprintf(stderr,"Incorrect bucket size: %lu != %lu\n",
             (Showuint) bsizemax,
             (Showuint) mkvaux->maxbucketsize);
    exit(EXIT_FAILURE);
  }
}
#endif

/*
 \texttt{bucketsortsuffixes} sorts all suffixes of the string \(t\) of 
 length \(n\) w.r.t.\ the first prefixlength characters of each suffix. 
*/

Sint bucketsortsuffixes(MKVaux *mkvaux,Virtualtree *virtualtree,
                        Uint numofchars)
{
  Uint shiftmap = MULT2(virtualtree->prefixlength-1), i, mapindex, code, *optr, 
       mappower[UCHAR_MAX+1], specialcountglobal = 0, power;
  Uchar a;
  Sint specialcountlocal = 0;
  ArraySuffixptr arseqptr;
  Suffixptr tptr;

  virtualtree->numofcodes 
   = vm_determinenumofcodes(numofchars,virtualtree->prefixlength);
  power = virtualtree->numofcodes/numofchars;
  ALLOCASSIGNSPACE(mkvaux->leftborder,NULL,Uint,virtualtree->numofcodes+1);
  for(i=0; i<virtualtree->numofcodes; i++)
  {
    mkvaux->leftborder[i] = 0;
  }
  DEBUG2(1,"# numofchars=%lu,power=%lu,",
           (Showuint) numofchars,
           (Showuint) power);
  DEBUG1(1," numofcodes=%lu\n",
           (Showuint) virtualtree->numofcodes);

  INITARRAY(&arseqptr,Suffixptr);
  if(numofchars == UintConst(4))
  {
    specialcountlocal = (Sint) (virtualtree->prefixlength-1);
    for(tptr = mkvaux->sentinel-1; /* Nothing */ ; tptr--)
    {
      if(ISSPECIAL(ACCESSCHAR(tptr)))
      {
        specialcountlocal = (Sint) virtualtree->prefixlength;
      }
      if(specialcountlocal > 0)
      {
        specialcountlocal--;
        specialcountglobal++;
      }
      CHECKTERMINATION;
    }
    code = virtualtree->numofcodes-1;
    mkvaux->leftborder[code]++;
    specialcountlocal = (Sint) (virtualtree->prefixlength-1);
    mkvaux->storecodes 
      = ((specialcountglobal * 2) < virtualtree->numofcodes) ? True : False;
    if(mkvaux->storecodes)
    {
      INITARRAY(&(mkvaux->arcodedistance),Uint);
      STOREINARRAY(&(mkvaux->arcodedistance),Uint,8192,CODEDISTANCE(code,0));
      STOREINARRAY(&arseqptr,Suffixptr,8192,mkvaux->sentinel);
      for(tptr = mkvaux->sentinel-1; /* Nothing */ ; tptr--)
      {
        if(ISSPECIAL(a = ACCESSCHAR(tptr)))
        {
          code = virtualtree->numofcodes-1;
          specialcountlocal = (Sint) virtualtree->prefixlength;
        } else
        {
          code = DIV4(code) | (a << shiftmap);
        }
        mkvaux->leftborder[code]++;
        if(specialcountlocal > 0)
        {
          STOREINARRAY(&(mkvaux->arcodedistance),Uint,8192,
                       CODEDISTANCE(code,virtualtree->prefixlength -
                                         (Uint) specialcountlocal));
          STOREINARRAY(&arseqptr,Suffixptr,8192,tptr);
          specialcountlocal--;
        }
        CHECKTERMINATION;
      }
    } else
    {
      ALLOCASSIGNSPACE(mkvaux->specialtable,NULL,Uint,virtualtree->numofcodes);
      for(i=0; i < virtualtree->numofcodes; i++)
      {
        mkvaux->specialtable[i] = 0;
      }
      mkvaux->specialtable[code]++;
      for(tptr = mkvaux->sentinel-1; /* Nothing */ ; tptr--)
      {
        if(ISSPECIAL(a = ACCESSCHAR(tptr)))
        {
          code = virtualtree->numofcodes-1;
          specialcountlocal = (Sint) virtualtree->prefixlength;
        } else
        {
          code = DIV4(code) | (a << shiftmap);
        }
        mkvaux->leftborder[code]++;
        if(specialcountlocal > 0)
        {
          mkvaux->specialtable[code]++;
          specialcountlocal--;
        }
        CHECKTERMINATION;
      }
    }
  } else
  {
    for(mappower[0] = 0, mapindex=UintConst(1); 
        mapindex<numofchars;
        mapindex++)
    {
      mappower[mapindex] = mappower[mapindex-1] + power;
    }
    code = virtualtree->numofcodes-1;
    mkvaux->leftborder[code]++;
    INITARRAY(&(mkvaux->arcodedistance),Uint);
    STOREINARRAY(&(mkvaux->arcodedistance),Uint,8192,CODEDISTANCE(code,0));
    STOREINARRAY(&arseqptr,Suffixptr,8192,mkvaux->sentinel);
    specialcountlocal = (Sint) (virtualtree->prefixlength-1);
    for(tptr = mkvaux->sentinel-1; /* Nothing */ ; tptr--)
    {
      if(ISSPECIAL(a = ACCESSCHAR(tptr)))
      {
        code = virtualtree->numofcodes-1;
        specialcountlocal = (Sint) virtualtree->prefixlength;
      } else
      {
        code /= numofchars;
        code += mappower[a];
      }
      mkvaux->leftborder[code]++;
      if(specialcountlocal > 0)
      {
        STOREINARRAY(&(mkvaux->arcodedistance),Uint,8192,
                     CODEDISTANCE(code,virtualtree->prefixlength - 
                                       (Uint) specialcountlocal));
        STOREINARRAY(&arseqptr,Suffixptr,8192,tptr);
        specialcountlocal--;
      }
      CHECKTERMINATION;
    }
  }
  mkvaux->maxbucketsize = mkvaux->leftborder[0];
  for(optr = mkvaux->leftborder + 1; 
      optr < mkvaux->leftborder + virtualtree->numofcodes; optr++)
  {
    if(mkvaux->maxbucketsize < *optr)
    {
      mkvaux->maxbucketsize = *optr;
    }
    *optr += *(optr-1);
  }
  mkvaux->leftborder[virtualtree->numofcodes] 
    = virtualtree->multiseq.totallength+1;
  DEBUGCODE(2,bucketstatistic(mkvaux,virtualtree->multiseq.totallength+1,
                              virtualtree->numofcodes));
  if(numofchars != UintConst(4) || mkvaux->storecodes)
  {
    for(i = 0; i < mkvaux->arcodedistance.nextfreeUint; i++)
    {
      NOTSUPPOSEDTOBENULL(arseqptr.spaceSuffixptr);
      mkvaux->sortedsuffixes[--mkvaux->leftborder[GETCODE(mkvaux->arcodedistance.spaceUint[i])]] 
        = arseqptr.spaceSuffixptr[i];
    }
    DEBUG1(1,"# space for artseqptr = %lu bytes\n",
             (Showuint) (sizeof(Suffixptr) * arseqptr.nextfreeSuffixptr));
    FREEARRAY(&arseqptr,Suffixptr);
  } else
  {
    code = virtualtree->numofcodes-1;
    specialcountlocal = (Sint) (virtualtree->prefixlength-1);
    mkvaux->sortedsuffixes[--mkvaux->leftborder[code]] = mkvaux->sentinel;
    for(tptr = mkvaux->sentinel-1; /* Nothing */ ; tptr--)
    {
      if(ISSPECIAL(a = ACCESSCHAR(tptr)))
      {
        code = virtualtree->numofcodes-1;
        specialcountlocal = (Sint) virtualtree->prefixlength;
      } else
      {
        code = DIV4(code) | (a << shiftmap);
      }
      if(specialcountlocal > 0)
      {
        mkvaux->sortedsuffixes[--mkvaux->leftborder[code]] = tptr;
        specialcountlocal--;
      } 
      CHECKTERMINATION;
    } 
  }
  code = virtualtree->numofcodes-1;
  specialcountlocal = (Sint) (virtualtree->prefixlength-1);
  if(numofchars == UintConst(4))
  {
    for(tptr = mkvaux->sentinel-1; /* Nothing */ ; tptr--)
    {
      if(ISSPECIAL(a = ACCESSCHAR(tptr)))
      {
        code = virtualtree->numofcodes-1;
        specialcountlocal = (Sint) virtualtree->prefixlength;
      } else
      {
        code = DIV4(code) | (a << shiftmap);
      }
      if(specialcountlocal > 0)
      {
        specialcountlocal--;
      } else
      {
        mkvaux->sortedsuffixes[--mkvaux->leftborder[code]] = tptr;
      }
      CHECKTERMINATION;
    } 
  } else
  {
    for(tptr = mkvaux->sentinel-1; /* Nothing */ ; tptr--)
    {
      if(ISSPECIAL(a = ACCESSCHAR(tptr)))
      {
        code = virtualtree->numofcodes-1;
        specialcountlocal = (Sint) virtualtree->prefixlength;
      } else
      {
        code /= numofchars;
        code += mappower[a];
      }
      if(specialcountlocal > 0)
      {
        specialcountlocal--;
      } else
      {
        mkvaux->sortedsuffixes[--mkvaux->leftborder[code]] = tptr;
      }
      CHECKTERMINATION;
    } 
  }
  return 0;
}
