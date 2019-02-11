#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

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

#define DEREF(I) ((depth == mersize) ? 0 : *(*(I)+depth))

typedef Uchar * Stringptr;

typedef struct 
{
  Stringptr *left, *right;
  Uint depth;
} Qsortstackelem;

DECLAREARRAYSTRUCT(Qsortstackelem);

#define STRINGCOMPARE(S,T,OFFSET,SC)\
        for (sptr = (S)+(OFFSET), tptr = (T)+(OFFSET); /* Nothing */;\
             sptr++, tptr++)\
        {\
          SC = (Sint) (*sptr - *tptr);\
          if((SC) != 0)\
          {\
            break;\
          }\
        }

#define SORT2(X,Y,D)\
        STRINGCOMPARE(*(X),*(Y),D,sortresult);\
        if(sortresult > 0)\
        {\
          SWAP(X,Y);\
        }

#define LARGESORT(W,L,R,D) inssort(L,R,D)

#define SMALLSIZE 6
#define DIRECTSORT(W,TWO,L,R,D)\
        if((W) == (TWO))\
        {\
          SORT2(L,R,D);\
        } else\
        {\
          LARGESORT(W,L,R,D);\
        }

#define SUBSORT(WIDTH,BORDER1,BORDER2,TWO,LEFT,RIGHT,DEPTH)\
        if((WIDTH) > (BORDER1))\
        {\
          if((WIDTH) <= (BORDER2))\
          {\
            DIRECTSORT(WIDTH,TWO,LEFT,RIGHT,DEPTH);\
          } else\
          {\
            PUSHMKVSTACK(LEFT,RIGHT,DEPTH);\
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

#define SWAP(A,B)\
        {\
          if((A) != (B))\
          {\
            temp = *(A);\
            *(A) = *(B);\
            *(B) = temp;\
          }\
        }

/* 
   assignment of records instead of copying the 
   stuff component by component leads to a slower program.
*/

#define PUSHMKVSTACK(L,R,D)\
        CHECKARRAYSPACE(&qsortstack,Qsortstackelem,1024);\
        qsortstack.spaceQsortstackelem[qsortstack.nextfreeQsortstackelem].left = L;\
        qsortstack.spaceQsortstackelem[qsortstack.nextfreeQsortstackelem].right = R;\
        qsortstack.spaceQsortstackelem[qsortstack.nextfreeQsortstackelem++].depth = D

#define POPQsortstack(L,R,D)\
        NOTSUPPOSEDTOBENULL(qsortstack.spaceQsortstackelem);\
        L = qsortstack.spaceQsortstackelem[--qsortstack.nextfreeQsortstackelem].left;\
        R = qsortstack.spaceQsortstackelem[qsortstack.nextfreeQsortstackelem].right;\
        D = qsortstack.spaceQsortstackelem[qsortstack.nextfreeQsortstackelem].depth;\
        width = (Uint) ((R) - (L) + 1)

/*
  It seems better to compress the lcps directly, rather than compressing
  the differences.
*/

static Stringptr *medianof3(Uint mersize,Stringptr *a,Stringptr *b,
                            Stringptr *c,Uint depth)
{
  Uchar va, vb, vc;

  va = DEREF(a);
  vb = DEREF(b);
  if (va == vb)
  {
    return a;
  }
  if ((vc = DEREF(c)) == va || vc == vb)
  {
    return c;       
  }
  return va < vb ?
        (vb < vc ? b : (va < vc ? c : a))
      : (vb > vc ? b : (va < vc ? a : c));
}

static void inssort(Stringptr *left,Stringptr *right,Uint depth)
{ 
  Stringptr sptr, tptr, *pi, *pj, temp;
  Sint sortresult;


/*
  DEBUG3(3,"# inssort(%lu,%lu,%lu)=",
              (Showuint) ADDR(left),
              (Showuint) ADDR(right),
              (Showuint) depth);
  DEBUGCODE(1,icall++);
  DEBUGCODE(1,ilen+=(right-left+1));
*/
  for (pi = left + 1; pi <= right; pi++)
  {
    for (pj = pi; pj > left; pj--) 
    {
/*
      DEBUG2(3,"# compare suffix %lu and %lu\n",
                (Showuint) PTR2START(*pj),
                (Showuint) PTR2START(*(pj-1)));
*/
      // Inline strcmp: break if *(pj-1) <= *pj
      STRINGCOMPARE(*(pj-1),*pj,depth,sortresult);
      if(sortresult < 0)
      {
        break;
      }
      SWAP(pj,pj-1);
    }
  }
}

void localbese(Uint mersize,Uchar **l,Uchar **r,Uint d)
{   
  Stringptr sptr, tptr, *left, *right, *leftplusw;
  Uchar partval, val;
  Sint sortresult, w;
  Uint depth, offset, doubleoffset, width;
  Stringptr *pa, *pb, *pc, *pd, *pl, *pm, *pr, *aptr, *bptr, temp;
  ArrayQsortstackelem qsortstack;

/*
  DEBUG4(3,"# bese(%lu,%lu,%lu,base=%lu)\n",
              (Showuint) ADDR(l),
              (Showuint) ADDR(r),
              (Showuint) d,
              (Showuint) ADDR(lcpbase));
*/
  
  width = (Uint) (r - l + 1);
  if(width <= (Uint) SMALLSIZE)
  {
    DIRECTSORT(width,UintConst(2),l,r,d);
    return;
  }
  left = l;
  right = r;
  depth = d;
  INITARRAY(&qsortstack,Qsortstackelem);

  while(True)
  {
    pl = left;
    pm = left + DIV2(width);
    pr = right;
    if(width > UintConst(30)) 
    { // On big arrays, pseudomedian of 9
      offset = DIV8(width);
      doubleoffset = MULT2(offset);
      pl = medianof3(mersize,pl, pl+offset, pl+doubleoffset,depth);
      pm = medianof3(mersize,pm-offset, pm, pm+offset,depth);
      pr = medianof3(mersize,pr-doubleoffset, pr-offset,pr,depth);
    }
    pm = medianof3(mersize,pl, pm, pr,depth);
    SWAP(left, pm);
    partval = DEREF(left);
    pa = pb = left + 1;
    pc = pd = right;
    while(True)
    {
      while (pb <= pc)
      {
        if((val = DEREF(pb)) > partval)
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
        if((val = DEREF(pc)) < partval)
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

/*
    DEBUG2(3,"# left=%lu,right=%lu,",
              (Showuint) ADDR(left),
              (Showuint) ADDR(right));
    DEBUG2(3,"a=%lu,b=%lu,",
              (Showuint) ADDR(pa),
              (Showuint) ADDR(pb));
    DEBUG2(3,"c=%lu,d=%lu\n",
              (Showuint) ADDR(pc),
              (Showuint) ADDR(pd));
*/
    if((w = (Sint) (pd-pc)) > 0)
    {
      SUBSORT(w,(Sint) 1,(Sint) SMALLSIZE,(Sint) 2,right-w+1,right,depth);
    }
    w = (Sint) (pb-pa);
    leftplusw = left + w;
    if(depth < mersize)
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
    if(qsortstack.nextfreeQsortstackelem == 0)
    {
      FREEARRAY(&qsortstack,Qsortstackelem);
      break;
    }
    POPQsortstack(left,right,depth);
  }
  FREEARRAY(&qsortstack,Qsortstackelem);
}
