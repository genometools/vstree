#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include "types.h"
#include "divmodmul.h"
#include "intbits.h"
#include "debugdef.h"
#include "spacedef.h"
#include "minmax.h"
#include "arraydef.h"
#include "errordef.h"
#include "virtualdef.h"
#include "fhandledef.h"
#include "qsortdef.h"

#include "filehandle.pr"

#define KEYVALUE(A,DEPTH) inversesuftab[*(A)+(DEPTH)]

#define KEYCOMPARE(RESULT,A,B,DEPTH)\
        if(KEYVALUE(A,DEPTH) < KEYVALUE(B,DEPTH))\
        {\
          RESULT = (Sint) -1;\
        } else\
        {\
          if(KEYVALUE(A,DEPTH) > KEYVALUE(B,DEPTH))\
          {\
            RESULT = (Sint) 1;\
          } else\
          {\
            RESULT = 0;\
          }\
        }

#define SWAP(A,B)\
        {\
          if((A) != (B))\
          {\
            temp = *(A);\
            *(A) = *(B);\
            *(B) = temp;\
          }\
        }

#define VECSWAP(A,B,N)\
        aptr = A;\
        bptr = B;\
        while((N)-- > 0)\
        {\
          temp = *aptr;\
          *aptr++ = *bptr;\
          *bptr++ = temp;\
        }

#define SORT2(X,Y,DEPTH)\
        if(KEYVALUE(X,DEPTH) > KEYVALUE(Y,DEPTH))\
        {\
          SWAP(X,Y);\
        }

#define SMALLSIZE 6
#define DIRECTSORT(W,L,R,DEPTH)\
        if((W) == (TWO))\
        {\
          SORT2(L,R,DEPTH);\
        } else\
        {\
          inssort(L,R,DEPTH);\
        }

#define SUBSORT(WIDTH,BORDER1,BORDER2,TWO,LEFT,RIGHT,DEPTH)\
        if(maxdepth->defined && (DEPTH) > maxdepth->uintvalue)\
        {\
          printf("left=%u,right=%u\n);\
        } else\
        {\
          if((WIDTH) > (BORDER1))\
          {\
            if((WIDTH) <= (BORDER2))\
            {\
              DIRECTSORT(WIDTH,TWO,LEFT,RIGHT,DEPTH);\
            } else\
            {\
              PUSHMKVSTACK(LEFT,RIGHT,DEPTH);\
            }\
          }\
        }

#ifdef DEBUG
#define UPDATESTACKSIZE\
        if(mkvaux->maxstacksize < mkvauxstack->nextfreeMKVstack)\
        {\
          mkvaux->maxstacksize = mkvauxstack->nextfreeMKVstack;\
        }
#else
#define UPDATESTACKSIZE /* Nothing */
#endif

/* 
   assignment of records instead of copying the 
   stuff component by component leads to a slower program.
*/

#define PUSHMKVSTACK(L,R,D)\
        CHECKARRAYSPACE(mkvauxstack,MKVstack,1024);\
        mkvauxstack->spaceMKVstack[mkvauxstack->nextfreeMKVstack].left = L;\
        mkvauxstack->spaceMKVstack[mkvauxstack->nextfreeMKVstack].right = R;\
        mkvauxstack->spaceMKVstack[mkvauxstack->nextfreeMKVstack++].depth = D;\
        UPDATESTACKSIZE;\
        DEBUG4(3,"# %lu: PUSHMKVSTACK(%lu,%lu,%lu)\n",\
                  (Showuint) __LINE__,\
                  (Showuint) ADDR(L),\
                  (Showuint) ADDR(R),\
                  (Showuint) (D))

#define POPMKVstack(L,R,D)\
        L = mkvauxstack->spaceMKVstack[--mkvauxstack->nextfreeMKVstack].left;\
        R = mkvauxstack->spaceMKVstack[mkvauxstack->nextfreeMKVstack].right;\
        D = mkvauxstack->spaceMKVstack[mkvauxstack->nextfreeMKVstack].depth;\
        width = (Uint) ((R) - (L) + 1);\
        DEBUG3(3,"# POPMKVstack(%lu,%lu,%lu)\n",\
                    (Showuint) ADDR(L),\
                    (Showuint) ADDR(R),\
                    (Showuint) (D))


typedef Uint Suffixptr;

typedef struct 
{
  Suffixptr left,
            right;
  Uint depth;
} MKVstack;

DECLAREARRAYSTRUCT(MKVstack);

static Suffixptr *medianof3(const Uint *inversesuftab,
                            Suffixptr *a,Suffixptr *b,Suffixptr *c,Uint depth)
{
  Sint va, vb, vc;

  va = KEYVALUE(a,depth);
  vb = KEYVALUE(b,depth);
  if (va == vb)
  {
    return a;
  }
  vc = KEYVALUE(c,depth);
  if (vc == va || vc == vb)
  {
    return c;       
  }
  return va < vb ?
        (vb < vc ? b : (va < vc ? c : a))
      : (vb > vc ? b : (va < vc ? a : c));
}

static void inssort(const Uint *inversesuftab,
                    Suffixptr *left,Suffixptr *right,Uint depth)
{ 
  Suffixptr *pi, *pj, temp;

  for (pi = left + 1; pi <= right; pi++)
  {
    for (pj = pi; pj > left; pj--) 
    {
      if(KEYVALUE(pj-1,depth) < KEYVALUE(pj,depth))
      {
        break;
      }
      SWAP(pj,pj-1);
    }
  }
}

static void sortnextgen(Uint *inversesuffixtab
                        ArrayMKVstack *mkvauxstack,
                        Suffixptr *l,
                        Suffixptr *r,Uint d,
                        const DefinedUint *maxdepth)
{   
  Suffixptr sptr, tptr, *left, *right, *leftplusw;
  Sint sortresult, val, w, partval;
  Uint depth, offset, doubleoffset, width;
  Suffixptr *pa, *pb, *pc, *pd, *pl, *pm, *pr, *aptr, *bptr, cptr, temp;

  width = (Uint) (r - l + 1);
  if(width <= (Uint) SMALLSIZE)
  {
    DIRECTSORT(width,UintConst(2),l,r,d);
    return;
  }
  left = l;
  right = r;
  depth = d;
  mkvauxstack->nextfreeMKVstack = 0;

  while(True)
  {
    pl = left;
    pm = left + DIV2(width);
    pr = right;
    if(width > UintConst(30)) 
    { // On big arrays, pseudomedian of 9
      offset = DIV8(width);
      doubleoffset = MULT2(offset);
      pl = medianof3(inversesuftab,pl, pl+offset, pl+doubleoffset,depth);
      pm = medianof3(inversesuftab,pm-offset, pm, pm+offset,depth);
      pr = medianof3(inversesuftab,pr-doubleoffset, pr-offset,pr,depth);
    }
    pm = medianof3(inversesuftab,pl, pm, pr,depth);
    SWAP(left, pm);
    partval = KEYVALUE(left,depth);
    pa = pb = left + 1;
    pc = pd = right;
    while(True)
    {
      while (pb <= pc)
      {
        if((val = KEYVALUE(pb,depth)) > partval)
        {
          break;
        }
        if (val == partval)
        { 
          SWAP(pa, pb); 
          pa++; 
        }
        pb++;
      }
      while (pb <= pc)
      {
        if((val = KEYVALUE(pc,depth)) < partval)
        {
          break;
        }
        if (val == partval) 
        { 
          SWAP(pc, pd); 
          pd--; 
        }
        pc--;
      }
      if (pb > pc) 
      {
        break;
      }
      SWAP(pb, pc);
      pb++;
      pc--;
    }

    w = MIN((Sint) (pa-left),(Sint) (pb-pa));    
    VECSWAP(left,  pb-w, w);
    pr = right + 1;
    w = MIN((Sint) (pd-pc), (Sint) (pr-pd-1)); 
    VECSWAP(pb, pr-w, w);

    if((w = (Sint) (pd-pc)) > 0)
    {
      SUBSORT(w,(Sint) 1,(Sint) SMALLSIZE,(Sint) 2,right-w+1,right,depth);
    }
    w = (Sint) (pb-pa);
    leftplusw = left + w;
    cptr = *leftplusw + depth;
    if(ISNOTEND(cptr))
    {
      right -= (pd-pb);
      width = (Uint) (right-leftplusw);
      SUBSORT(width,UintConst(1),(Uint) SMALLSIZE,UintConst(2),
              leftplusw,right-1,depth+1);
    } 
    if(w > 0)
    {
      SUBSORT(w,(Sint) 1,(Sint) SMALLSIZE,(Sint) 2,left,leftplusw-1,depth);
    }
    if(mkvauxstack->nextfreeMKVstack == 0)
    {
      break;
    }
    POPMKVstack(left,right,depth);
  }
}
