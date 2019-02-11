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
#include "rmnsufinfo.h"
#include "mkvaux.h"

#include "filehandle.pr"
#include "ppsort.pr"
#ifndef SUFFIXPTR
#include "remainsort.pr"
#endif

#define ADDR(I)         ((Uint) ((I) - mkvaux->sortedsuffixes))
#define COMPAREOFFSET   (UCHAR_MAX + 1)

#ifdef SUFFIXPTR
#define UNIQUEINT(P)    ((Sint) ((P) - mkvaux->compareptr))
#else
#define UNIQUEINT(P)    ((Sint) ((P) + COMPAREOFFSET))
#endif

#ifdef SPECIAL
#include "chardef.h"
#define DECLARETMPC Uchar tmpsvar, tmptvar
#define GENDEREF(VAR,A,S)\
        (((A) < mkvaux->sentinel && ISNOTSPECIAL(VAR = ACCESSCHAR(S))) ?\
        ((Sint) VAR) : UNIQUEINT(S))

#define ISNOTEND(P) ((P) < mkvaux->sentinel && ISNOTSPECIAL(ACCESSCHAR(P)))
#define BESESUFFIXSORT  besesuffixsortspecial
#define INITMKVAUX      initmkvauxspecial
#else
#define DECLARETMPC /* Nothing */
#define GENDEREF(VAR,A,S)\
        (((A) < mkvaux->sentinel) ? ((Sint) ACCESSCHAR(S)) : UNIQUEINT(S))
#define ISNOTEND(P) ((P) < mkvaux->sentinel)
#define BESESUFFIXSORT  besesuffixsort
#define INITMKVAUX      initmkvaux
#endif

#define PTR2INT(VAR,I) GENDEREF(VAR,cptr = *(I)+depth,cptr)
#define DEREF(VAR,S)   GENDEREF(VAR,S,S)

#ifdef WITHLCP
#define LCPINDEX(I)      ((Uint) ((I) - lcpbase))
#define EVALLCPLEN(LL,T) LL = (Uint) (tptr - (T))
#define SETLCPGEN(I,V,O)\
        DEBUG4(3,"# %lu: set new lcpsubtab[%lu(%lu)]=%lu ",\
               (Showuint) __LINE__,\
               (Showuint) (I),\
               (Showuint) (I+O),\
               (Showuint) (V));\
        DEBUG2(3," for suffixes %lu and %lu\n",\
               (Showuint) INDEX2START((I)+(O)-1),\
               (Showuint) INDEX2START((I)+(O)));\
        mkvaux->lcpsubtab[I] = V
#else
#define EVALLCPLEN(LL,T)  /* Nothing */
#define SETLCPGEN(I,V,O)  /* Nothing */
#endif

#define SETLCP1(I,V) SETLCPGEN(I,V,(Uint) (lcpbase-mkvaux->sortedsuffixes))
#define SETLCP2(I,V) SETLCPGEN(I,V,mkvaux->leftborder[i])

#define STRINGCOMPARE(S,T,OFFSET,SC,LL)\
        for (sptr = (S)+(OFFSET), tptr = (T)+(OFFSET); /* Nothing */;\
             sptr++, tptr++)\
        {\
          DEBUGCODE(1,charcomp++);\
          SC = DEREF(tmpsvar,sptr) - DEREF(tmptvar,tptr);\
          if((SC) != 0)\
          {\
            EVALLCPLEN(LL,T);\
            break;\
          }\
        }

#define GETLCP(L)\
        for (sptr = *((L)-1), tptr = *(L);\
	     DEREF(tmpsvar,sptr) == DEREF(tmptvar,tptr);\
             sptr++, tptr++)\
              /* Nothing */ ;

#define SORT2(X,Y,D)\
        STRINGCOMPARE(*(X),*(Y),D,sortresult,lcplen);\
        if(sortresult > 0)\
        {\
          SWAP(X,Y);\
        }\
        SETLCP1(LCPINDEX(Y),lcplen)

#undef SPECIALSORT
#ifdef SPECIALSORT
#define SORT3(X,Y,Z,D)\
        Y = X+1;\
        STRINGCOMPARE(*(X),*(Y),D,sortresult,lcplen1);\
        if(sortresult <= 0)\
        {\
          STRINGCOMPARE(*(Y),*(Z),D,sortresult,lcplen2);\
          if(sortresult > 0)\
          {\
            STRINGCOMPARE(*(X),*(Z),D,sortresult,lcplen3);\
            if(sortresult <= 0)\
            {\
              SWAP(Y,Z);\
	      SETLCP1(LCPINDEX(Y),lcplen3);\
	      SETLCP1(LCPINDEX(Z),lcplen2);\
            } else\
            {\
              temp = *(X);\
              *(X) = *(Z);\
              *(Z) = *(Y);\
              *(Y) = temp;\
	      SETLCP1(LCPINDEX(Y),lcplen3);\
	      SETLCP1(LCPINDEX(Z),lcplen1);\
            }\
	  } else\
	  {\
	    SETLCP1(LCPINDEX(Y),lcplen1);\
	    SETLCP1(LCPINDEX(Z),lcplen2);\
          }\
        } else\
        {\
          STRINGCOMPARE(*(X),*(Z),D,sortresult,lcplen2);\
          if(sortresult <= 0)\
          {\
            SWAP(Y,X);\
	    SETLCP1(LCPINDEX(Y),lcplen1);\
	    SETLCP1(LCPINDEX(Z),lcplen2);\
          } else\
          {\
            STRINGCOMPARE(*(Y),*(Z),D,sortresult,lcplen3);\
            if(sortresult >= 0)\
            {\
              SWAP(X,Z);\
	      SETLCP1(LCPINDEX(Y),lcplen3);\
	      SETLCP1(LCPINDEX(Z),lcplen1);\
            } else\
            {\
              temp = *(X);\
              *(X) = *(Y);\
              *(Y) = *(Z);\
              *(Z) = temp;\
	      SETLCP1(LCPINDEX(Y),lcplen3);\
	      SETLCP1(LCPINDEX(Z),lcplen2);\
            }\
          }\
        }

#define LARGESORT(W,L,R,D)\
        if((W) == 3)\
        {\
          SORT3(L,mid,R,D);\
        } else\
        {\
          inssort(mkvaux,virtualtree,L,R,D,lcpbase);\
        }

#else
#define LARGESORT(W,L,R,D) inssort(mkvaux,virtualtree,L,R,D,lcpbase)
#endif

#define SMALLSIZE 6
#define DIRECTSORT(W,TWO,L,R,D)\
        if((W) == (TWO))\
        {\
          SORT2(L,R,D);\
        } else\
        {\
          LARGESORT(W,L,R,D);\
        }

#define SUBSORT2(WIDTH,BORDER1,BORDER2,TWO,LEFT,RIGHT,DEPTH)\
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

#ifndef SUFFIXPTR
#define SUBSORT(WIDTH,BORDER1,BORDER2,TWO,LEFT,RIGHT,DEPTH)\
        if(maxdepth->defined && (DEPTH) >= maxdepth->uintvalue)\
        {\
          addunsortedrange(rmnsufinfo,maxdepth->uintvalue,LEFT,RIGHT);\
        } else\
        {\
          SUBSORT2(WIDTH,BORDER1,BORDER2,TWO,LEFT,RIGHT,DEPTH);\
        }
#else
#define SUBSORT(WIDTH,BORDER1,BORDER2,TWO,LEFT,RIGHT,DEPTH)\
        SUBSORT2(WIDTH,BORDER1,BORDER2,TWO,LEFT,RIGHT,DEPTH)
#endif

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

#define GETDISTANCE(V,PFXLENBITS)     ((V) & MAXVALUEWITHBITS(PFXLENBITS))

#ifdef WITHLCP
#define DOLCPOUT\
        if(outlcpsubtab(demandlcp,virtualtree,lcpfp,mkvaux->lcpsubtab,\
                        (Uint) (right-left),lcpoffset) != 0)\
        {\
          return (Sint) -2;\
        }\
        lcpoffset += (Uint) (right - left)
#else
#define DOLCPOUT /* Nothing */
#endif

typedef struct 
{
  Suffixptr *left, 
            *right;
  Uint depth;
} MKVstack;

DECLAREARRAYSTRUCT(MKVstack);

#ifdef DEBUG

#define UNDEFLCP (virtualtree->multiseq.totallength)

#ifdef SUFFIXPTR
#define INDEX2START(I)  ((Uint) (mkvaux->sortedsuffixes[I] - virtualtree->multiseq.sequence))
#define PTR2START(P)    ((Uint) ((P) - virtualtree->multiseq.sequence))
#else
#define INDEX2START(I)  mkvaux->sortedsuffixes[I]
#define PTR2START(P)    (P)
#endif

static Uint charcomp = 0, icall = 0, ilen = 0;

/*
  It seems better to compress the lcps directly, rather than compressing
  the differences.
*/

static void showsuffix(/*@unused@*/ MKVaux *mkvaux,
                       Virtualtree *virtualtree,Uchar *s)
{
  Uint i = 0;

  while(True)
  {
    if(s + i == 
       virtualtree->multiseq.sequence + virtualtree->multiseq.totallength)
    {
      (void) putchar((Fputcfirstargtype) '~');
      break;
    }
#ifdef SPECIAL
    if(ISSPECIAL(s[i]))
    {
      printf("|%lu",(Showuint) ((s+i) - virtualtree->multiseq.sequence));
      break;
    }
#endif
    if(isprint(s[i]))
    {
      (void) putchar((Fputcfirstargtype) s[i]);
    } else
    {
      printf("\\%lu",(Showuint) s[i]);
    }
    i++;
  }
}

static void showsuffixesrange(/*@unused@*/ MKVaux *mkvaux,
                              /*@unused@*/ Virtualtree *virtualtree,
                              Suffixptr *left,Suffixptr *right)
{
  Suffixptr *p;

  for(p=left; p<=right; p++)
  {
    printf(" %lu",(Showuint) PTR2START(*p));
  }
  (void) putchar('\n');
}

static void checksuftab(MKVaux *mkvaux,Virtualtree *virtualtree)
{
  Uint j, 
       suffixlen, 
       suffixstart1,
       suffixstart2,
       prefixlength, 
       sumprefixlength = 0,
       countsuffixes = 0,
       *tabptr, 
       tabsize = UintConst(1) + 
                 DIVWORDSIZE(virtualtree->multiseq.totallength+1), 
       *occurred;
  Suffixptr ptr1, ptr2;  
  DECLARETMPC;

  ALLOCASSIGNSPACE(occurred,NULL,Uint,tabsize);
  for(tabptr = occurred; tabptr < occurred + tabsize; tabptr++)
  {
    *tabptr = 0;
  }

  for(j=0; j<=virtualtree->multiseq.totallength; j++)
  {
    suffixstart1 = INDEX2START(j);
    if(ISIBITSET(occurred,suffixstart1))
    {
      fprintf(stderr,"suffix %lu already occurred\n",(Showuint) suffixstart1);
      exit(EXIT_FAILURE);
    } 
    SETIBIT(occurred,suffixstart1);
    countsuffixes++;
    DEBUG1(3,"# %lu: ",(Showuint) j);
    DEBUG1(3,"suffix %lu=",(Showuint) suffixstart1);
    DEBUGCODE(4,showsuffix(mkvaux,virtualtree,
                           virtualtree->multiseq.sequence+suffixstart1));
    DEBUG0(3,"\n");
  }
  if(countsuffixes != virtualtree->multiseq.totallength+1)
  {
    fprintf(stderr,"incorrect number %lu of suffixes\n",
            (Showuint) countsuffixes);
    exit(EXIT_FAILURE);
  }

  for(j=UintConst(1); j<=virtualtree->multiseq.totallength; j++)
  {
    suffixstart1 = INDEX2START(j-1);
    suffixstart2 = INDEX2START(j);
    if(suffixstart1 > suffixstart2)
    {
      suffixlen = virtualtree->multiseq.totallength - suffixstart1;
    } else
    {
      suffixlen = virtualtree->multiseq.totallength - suffixstart2;
    }
#ifdef SUFFIXPTR
    for(ptr1 = virtualtree->multiseq.sequence + suffixstart1, 
        ptr2 = virtualtree->multiseq.sequence + suffixstart2;  
        ptr1 < virtualtree->multiseq.sequence + suffixstart1 + suffixlen; 
        ptr1++, ptr2++)
    {
      if(DEREF(tmpsvar,ptr1) != DEREF(tmptvar,ptr2))
      {
        break;
      }
    }
    prefixlength = (Uint) (ptr1-(virtualtree->multiseq.sequence+suffixstart1));
#else
    for(ptr1 = suffixstart1, ptr2 = suffixstart2;  
        ptr1 < suffixstart1 + suffixlen; 
        ptr1++, ptr2++)
    {
      if(DEREF(tmpsvar,ptr1) != DEREF(tmptvar,ptr2))
      {
        break;
      }
    }
    prefixlength = (Uint) (ptr1-suffixstart1);
#endif
    sumprefixlength += prefixlength;
    if(DEREF(tmpsvar,ptr1) > DEREF(tmptvar,ptr2))
    {
      fprintf(stderr,"(1) compare %lu and %lu: ",
              (Showuint) suffixstart1,
              (Showuint) suffixstart2);
      fprintf(stderr,"order incorrect: c1 = ~ > \\%lu = c2\n",
                      (Showuint) ACCESSCHAR(ptr2));
      exit(EXIT_FAILURE);
    }
  }
  printf("# checksuftab okay: %lu checked, ",
           (Showuint) virtualtree->multiseq.totallength);
  printf(" avg prefixlength %.2f\n",
          (double) sumprefixlength/virtualtree->multiseq.totallength);
  FREESPACE(occurred);
}

#ifdef WITHLCP
static Uint checklcpsubtab(MKVaux *mkvaux,Virtualtree *virtualtree,
                           Uint offset,Uint len)
{
  Uint start, j, suffixlen, suffixstart1, suffixstart2, prefixlength, 
       checkcount;
  Suffixptr ptr1, ptr2;  
  DECLARETMPC;

  DEBUG2(3,"# checklcpsubtab(%lu,%lu)\n",(Showuint) offset,(Showuint) len);
  if(offset == 0)
  {
    start = UintConst(1);
    checkcount = (len-1);
  } else
  {
    start = 0;
    checkcount = len;
  }
  for(j=start; j<len; j++)
  {
    if(mkvaux->lcpsubtab[j] == UNDEFLCP)
    {
      fprintf(stderr,"lcp[%lu] not set\n",(Showuint) j);
      exit(EXIT_FAILURE);
    }
  }
  for(j=start; j<len; j++)
  {
    suffixstart1 = INDEX2START(offset+j-1);
    suffixstart2 = INDEX2START(offset+j);
    if(suffixstart1 > suffixstart2)
    {
      suffixlen = virtualtree->multiseq.totallength - suffixstart1;
    } else
    {
      suffixlen = virtualtree->multiseq.totallength - suffixstart2;
    }
#ifdef SUFFIXPTR
    for(ptr1 = virtualtree->multiseq.sequence + suffixstart1, 
        ptr2 = virtualtree->multiseq.sequence + suffixstart2;  
        ptr1 < virtualtree->multiseq.sequence + suffixstart1 + suffixlen; 
        ptr1++, ptr2++)
    {
      if(DEREF(tmpsvar,ptr1) != DEREF(tmptvar,ptr2))
      {
        break;
      }
    }
    prefixlength = (Uint) (ptr1-(virtualtree->multiseq.sequence+suffixstart1));
#else
    for(ptr1 = suffixstart1, ptr2 = suffixstart2;  
        ptr1 < suffixstart1 + suffixlen; 
        ptr1++, ptr2++)
    {
      if(DEREF(tmpsvar,ptr1) != DEREF(tmptvar,ptr2))
      {
        break;
      }
    }
    prefixlength = (Uint) (ptr1-suffixstart1);
#endif
    if(mkvaux->lcpsubtab[j] != prefixlength)
    {
      fprintf(stderr,"entry %lu: Wrong prefixlength=%lu of suffix %lu",
                      (Showuint) j,
                      (Showuint) mkvaux->lcpsubtab[j],
                      (Showuint) suffixstart1);
      fprintf(stderr," and suffix %lu, correct value is %lu\n",
                       (Showuint) suffixstart2,
                       (Showuint) prefixlength);
      exit(EXIT_FAILURE);
    }
  }
  return checkcount;
}
#endif
#endif

#ifdef WITHLCP
static Sint outlcpsubtab(BOOL demandlcp,Virtualtree *virtualtree,
                         FILE *lcpfp,Uint *lcpsubtab,Uint lcpsubtablen,
                         Uint offset)
{
  if(demandlcp)
  {
    Uint value, i;
    Uchar *outspace;
    PairUint *ptr;

    if(lcpfp == NULL)
    {
      outspace = virtualtree->lcptab + offset;
    } else
    {
      outspace = (Uchar *) lcpsubtab;
    }
    for(i=0; i < lcpsubtablen; i++)
    {
      value = lcpsubtab[i];
      if(virtualtree->maxbranchdepth < value)
      {
        virtualtree->maxbranchdepth = value;
      }
      if(value < UCHAR_MAX)
      {
        outspace[i] = (Uchar) value;
      } else
      {
        outspace[i] = UCHAR_MAX;
        GETNEXTFREEINARRAY(ptr,&virtualtree->largelcpvalues,PairUint,4096);
        ptr->uint0 = offset + i;
        ptr->uint1 = value;
      }
    }
    if(lcpfp != NULL)
    {
      if(WRITETOFILEHANDLE(outspace,(Uint) sizeof(Uchar),
                           lcpsubtablen,lcpfp) != 0)
      {
        return (Sint) -1;
      }
    }
  } else
  {
    Uint largevals = 0, *lcpptr;

    for(lcpptr = lcpsubtab; lcpptr < lcpsubtab + lcpsubtablen; lcpptr++)
    {
      if(*lcpptr >= UCHAR_MAX)
      {
        largevals++;
      }
    }
    virtualtree->largelcpvalues.nextfreePairUint += largevals;
  }
  return 0;
}
#endif

void INITMKVAUX(MKVaux *mkvaux,Virtualtree *virtualtree)
{
  Suffixptr tptr;

  ALLOCASSIGNSPACE(mkvaux->sortedsuffixes,NULL,Suffixptr,
                   virtualtree->multiseq.totallength+1);
#ifdef SUFFIXPTR
  {
  Suffixptr *sptr;
  mkvaux->compareptr = virtualtree->multiseq.sequence - COMPAREOFFSET; 
  mkvaux->sentinel 
    = virtualtree->multiseq.sequence + virtualtree->multiseq.totallength;
  for(tptr = virtualtree->multiseq.sequence, 
      sptr = mkvaux->sortedsuffixes; tptr <= mkvaux->sentinel;
      tptr++, sptr++)
  {
    *sptr = tptr;
  }
  }
#else
  mkvaux->sentinel = virtualtree->multiseq.totallength;
  for(tptr = 0; tptr <= mkvaux->sentinel; tptr++)
  {
    mkvaux->sortedsuffixes[tptr] = tptr;
  }
#endif
#ifdef DEBUG
  mkvaux->maxstacksize = 0;
#endif
}

#ifdef WITHLCP
static void initlcpsubtab(MKVaux *mkvaux,Virtualtree *virtualtree,
                          Uint lcpsubtabmaxsize)
{
  ALLOCASSIGNSPACE(mkvaux->lcpsubtab,NULL,Uint,lcpsubtabmaxsize);
#ifdef DEBUG
  {
    Uint *uptr;
    for(uptr = mkvaux->lcpsubtab; uptr < mkvaux->lcpsubtab + lcpsubtabmaxsize; 
        uptr++)
    {
      *uptr = UNDEFLCP;
    }
  }
#endif
  mkvaux->lcpsubtab[0] = 0;
}
#endif

static Suffixptr *medianof3(MKVaux *mkvaux,
                            /*@unused@*/ Virtualtree *virtualtree,
                            Suffixptr *a,Suffixptr *b,Suffixptr *c,Uint depth)
{
  Suffixptr cptr;
  Sint va, vb, vc;
  DECLARETMPC;

  va=PTR2INT(tmpsvar,a);
  vb=PTR2INT(tmptvar,b);
  if (va == vb)
  {
    return a;
  }
  if ((vc=PTR2INT(tmpsvar,c)) == va || vc == vb)
  {
    return c;       
  }
  return va < vb ?
        (vb < vc ? b : (va < vc ? c : a))
      : (vb > vc ? b : (va < vc ? a : c));
}

static void inssort(MKVaux *mkvaux,Virtualtree *virtualtree,Suffixptr *left,
                    Suffixptr *right,Uint depth,Suffixptr *lcpbase)
{ 
  Suffixptr sptr, tptr;
  Suffixptr *pi, *pj, temp;
  Sint sortresult;
#ifdef WITHLCP
  Uint lcplen, lcpindex;
#endif
  DECLARETMPC;

  DEBUG3(3,"# inssort(%lu,%lu,%lu)=",
              (Showuint) ADDR(left),
              (Showuint) ADDR(right),
              (Showuint) depth);
  DEBUGCODE(3,showsuffixesrange(mkvaux,virtualtree,left,right));
  DEBUGCODE(1,icall++);
  DEBUGCODE(1,ilen+=(right-left+1));
  for (pi = left + 1; pi <= right; pi++)
  {
    for (pj = pi; pj > left; pj--) 
    {
      DEBUG2(3,"# compare suffix %lu and %lu\n",
                (Showuint) PTR2START(*pj),
                (Showuint) PTR2START(*(pj-1)));
      // Inline strcmp: break if *(pj-1) <= *pj
      STRINGCOMPARE(*(pj-1),*pj,depth,sortresult,lcplen);
#ifdef WITHLCP
      lcpindex = LCPINDEX(pj);
      if(sortresult > 0 && pj < pi)
      {
        SETLCP1(lcpindex+1,mkvaux->lcpsubtab[lcpindex]);
      }
      SETLCP1(lcpindex,lcplen);
#endif
      if(sortresult < 0)
      {
        break;
      }
      SWAP(pj,pj-1);
    }
  }
}

static void bentleysedgewick(MKVaux *mkvaux,
                             ArrayMKVstack *mkvauxstack,
                             Virtualtree *virtualtree,Suffixptr *l,
                             Suffixptr *r,Uint d,Suffixptr *lcpbase,
                             const DefinedUint *maxdepth,Rmnsufinfo *rmnsufinfo)
{   
  Suffixptr sptr, tptr, *left, *right, *leftplusw;
  Sint sortresult, val, w, partval;
  Uint depth, offset, doubleoffset, width;
  Suffixptr *pa, *pb, *pc, *pd, *pl, *pm, *pr, *aptr, *bptr, cptr, temp;
#ifdef WITHLCP
  Uint lcplen;
#ifdef SPECIALSORT
  Uint lcplen1, lcplen2, lcplen3;
#endif
#endif
#ifdef SPECIALSORT
  Suffixptr *mid;
#endif
  DECLARETMPC;

  DEBUG4(3,"# bentleysedgewick(%lu,%lu,%lu,base=%lu)\n",
              (Showuint) ADDR(l),
              (Showuint) ADDR(r),
              (Showuint) d,
              (Showuint) ADDR(lcpbase));

#ifndef SUFFIXPTR
  if(maxdepth->defined && virtualtree->prefixlength == maxdepth->uintvalue)
  {
    printf("addunsortedrange(%lu,%lu)\n",(Showuint) ADDR(l),
                                         (Showuint) ADDR(r));
    addunsortedrange(rmnsufinfo,maxdepth->uintvalue,l,r);
    return;
  }
#endif
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
      pl = medianof3(mkvaux,virtualtree,pl, pl+offset, pl+doubleoffset,depth);
      pm = medianof3(mkvaux,virtualtree,pm-offset, pm, pm+offset,depth);
      pr = medianof3(mkvaux,virtualtree,pr-doubleoffset, pr-offset,pr,depth);
    }
    pm = medianof3(mkvaux,virtualtree,pl, pm, pr,depth);
    SWAP(left, pm);
    partval = PTR2INT(tmpsvar,left);
    pa = pb = left + 1;
    pc = pd = right;
    while(True)
    {
      while (pb <= pc)
      {
        if((val = PTR2INT(tmpsvar,pb)) > partval)
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
        if((val = PTR2INT(tmpsvar,pc)) < partval)
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

    DEBUG2(3,"# left=%lu,right=%lu,",
              (Showuint) ADDR(left),
              (Showuint) ADDR(right));
    DEBUG2(3,"a=%lu,b=%lu,",
              (Showuint) ADDR(pa),
              (Showuint) ADDR(pb));
    DEBUG2(3,"c=%lu,d=%lu\n",
              (Showuint) ADDR(pc),
              (Showuint) ADDR(pd));
    if((w = (Sint) (pd-pc)) > 0)
    {
      SETLCP1(LCPINDEX(right-w+1),depth);
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
      SETLCP1(LCPINDEX(leftplusw),depth);
      SUBSORT(w,(Sint) 1,(Sint) SMALLSIZE,(Sint) 2,left,leftplusw-1,depth);
    }
    if(mkvauxstack->nextfreeMKVstack == 0)
    {
      break;
    }
    POPMKVstack(left,right,depth);
  }
}

Sint BESESUFFIXSORT(MKVaux *mkvaux,
                    Virtualtree *virtualtree,
                    Uint numofchars,
                    BOOL demandlcp,
                    FILE *lcpfp,
                    Showverbose showverbose,
                    const DefinedUint *maxdepth)
{
  Suffixptr sptr, tptr, *left, *mid, *right;
  Uint i, j, jstart;
  Rmnsufinfo *rmnsufinfo;
  ArrayMKVstack mkvauxstack;// AUX
  
#ifdef DEBUG
#ifdef WITHLCP
  Uint lcpschecked = 0;
#endif
#endif
  DECLARETMPC;

  INITARRAY(&mkvauxstack,MKVstack);
  if(showverbose != NULL)
  {
    showverbose("initializing data structures");
  }
  INITMKVAUX(mkvaux,virtualtree);
#ifndef SUFFIXPTR
  if(maxdepth->defined)
  {
    rmnsufinfo = initRmnsufinfo(mkvaux->sortedsuffixes,
                                virtualtree->multiseq.totallength,
                                virtualtree->multiseq.sequence);
  } else
  {
    rmnsufinfo = NULL;
  }
#else
  rmnsufinfo = NULL;
#endif
  if(virtualtree->prefixlength > 0)
  {
#ifdef WITHLCP
    Uint lcpoffset = 0;
#endif
    if(showverbose != NULL)
    {
      char sbuf[256];
      sprintf(sbuf,"sorting suffixes according to prefix of length %lu",
              (Showuint) virtualtree->prefixlength);
      showverbose(sbuf);
    }
    if(bucketsortsuffixes(mkvaux,virtualtree,numofchars) != 0)
    {
      return (Sint) -1;
    }
    INITARRAY(&virtualtree->largelcpvalues,PairUint);
    virtualtree->maxbranchdepth = 0;
#ifdef WITHLCP
    initlcpsubtab(mkvaux,virtualtree,mkvaux->maxbucketsize);
#endif
    if(showverbose != NULL)
    {
      showverbose("sorting all buckets");
    }
    right = mkvaux->sortedsuffixes + mkvaux->leftborder[0];
    if(numofchars != UintConst(4) || mkvaux->storecodes)
    {
      STOREINARRAY(&(mkvaux->arcodedistance),Uint,4096,
                   CODEDISTANCE(virtualtree->numofcodes,0));
      qsortUint(mkvaux->arcodedistance.spaceUint,
	        mkvaux->arcodedistance.spaceUint+
                      mkvaux->arcodedistance.nextfreeUint-1);
      for(j = 0, i = 0; i < virtualtree->numofcodes; i++)
      {
        left = right;
        right = mkvaux->sortedsuffixes + mkvaux->leftborder[i+1];
        if(left < right)
        {
          for(jstart=j; i == GETCODE(mkvaux->arcodedistance.spaceUint[j]); j++)
            /* Nothing */ ;
          if(left < right - 1)
          {
            mid = right - (j-jstart);
            if(left < mid)
            {
              if(left < mid - 1)
              {
                bentleysedgewick(mkvaux,&mkvauxstack,virtualtree,left,mid - 1,
                                 virtualtree->prefixlength,left,maxdepth,
                                 rmnsufinfo);
              }
#ifdef WITHLCP
              if(left > mkvaux->sortedsuffixes)
              {
                GETLCP(left);
                SETLCP2(0,(Uint) (tptr-*left));
              }
#endif
            }
            if(mid < right)
            {
              if(mid < right - 1)
              {
                bentleysedgewick(mkvaux,&mkvauxstack,virtualtree,mid,right - 1,
                                 GETDISTANCE(mkvaux->arcodedistance.
                                             spaceUint[jstart],PREFIXLENBITS),
                                 left,maxdepth,rmnsufinfo);
              }
#ifdef WITHLCP
              if(mid > mkvaux->sortedsuffixes)
              {
                GETLCP(mid);
                SETLCP2((Uint) (mid-left),(Uint) (tptr-*mid));
              }
#endif
            }
          } else  // bucket size is 1
          {
            if(left > mkvaux->sortedsuffixes)
            {
              GETLCP(left);
              SETLCP2(0,(Uint) (tptr-*left));
            }
          }
#ifdef DEBUG
#ifdef WITHLCP
          lcpschecked += checklcpsubtab(mkvaux,virtualtree,
                                        ADDR(left),(Uint) (right-left));
#endif
#endif
          DOLCPOUT;
        }
      }
    } else
    {
      for(i = 0; i < virtualtree->numofcodes; i++)
      {
        left = right;
        right = mkvaux->sortedsuffixes + mkvaux->leftborder[i+1];
        if(left < right)
        {
          if(left < right - 1)
          {
            mid = right - mkvaux->specialtable[i];
            if(left < mid)
            {
              if(left < mid - 1)
              {
                bentleysedgewick(mkvaux,&mkvauxstack,virtualtree,left,mid - 1,
                                 virtualtree->prefixlength,left,maxdepth,
                                 rmnsufinfo);
              }
#ifdef WITHLCP
              if(left > mkvaux->sortedsuffixes)
              {
                GETLCP(left);
                SETLCP2(0,(Uint) (tptr-*left));
              }
#endif
            }
            if(mid < right)
            {
              if(mid < right - 1)
              {
                bentleysedgewick(mkvaux,&mkvauxstack,virtualtree,mid,
                                 right - 1,0,left,maxdepth,rmnsufinfo);
              }
#ifdef WITHLCP
              if(mid > mkvaux->sortedsuffixes)
              {
                GETLCP(mid);
                SETLCP2((Uint) (mid-left),(Uint) (tptr-*mid));
              }
#endif
            }
          } else  // bucket size is 1
          {
            if(left > mkvaux->sortedsuffixes)
            {
              GETLCP(left);
              SETLCP2(0,(Uint) (tptr-*left));
            }
          }
#ifdef DEBUG
#ifdef WITHLCP
          lcpschecked += checklcpsubtab(mkvaux,virtualtree,
                                        ADDR(left),(Uint) (right-left));
#endif
#endif
          DOLCPOUT;
        }
      }
    }
  } else
  {
    INITARRAY(&virtualtree->largelcpvalues,PairUint);
    virtualtree->maxbranchdepth = 0;
#ifdef WITHLCP
    initlcpsubtab(mkvaux,virtualtree,virtualtree->multiseq.totallength+1);
#endif
    if(showverbose != NULL)
    {
      showverbose("sorting suffixes");
    }
    bentleysedgewick(mkvaux,&mkvauxstack,virtualtree,mkvaux->sortedsuffixes,
                     mkvaux->sortedsuffixes+virtualtree->multiseq.totallength,0,
                     mkvaux->sortedsuffixes,maxdepth,rmnsufinfo);
#ifdef DEBUG
#ifdef WITHLCP
    lcpschecked += checklcpsubtab(mkvaux,virtualtree,0,
                                  virtualtree->multiseq.totallength+1);
#endif
#endif
#ifdef WITHLCP
    if(outlcpsubtab(demandlcp,virtualtree,lcpfp,mkvaux->lcpsubtab,
                    virtualtree->multiseq.totallength+1,0) != 0)
    {
      return (Sint) -2;
    }
#endif
  }
#ifndef SUFFIXPTR
  if(maxdepth->defined)
  {
    wrapRmnsufinfo(&rmnsufinfo,virtualtree->multiseq.totallength);
  }
#endif
#ifdef WITHLCP
#endif
  DEBUGCODE(1,checksuftab(mkvaux,virtualtree));
#ifdef WITHLCP
#ifdef DEBUG
  DEBUG1(1,"# checklcpsubtab: %lu checked\n",(Showuint) lcpschecked);
  if(lcpschecked != virtualtree->multiseq.totallength)
  {
    fprintf(stderr,"Not all suffixes are checked\n");
    exit(EXIT_FAILURE);
  }
#endif
#endif
  DEBUG3(1,"# icall=%lu,ilen=%lu,ilen/icall=%.2f\n",
               (Showuint) icall,(Showuint) ilen,
               (icall == 0) ? 0.0 : (double) ilen/icall);
  DEBUG3(1,"# charcomp=%ld,seqlen=%ld,charcomp/seqlen=%.2f\n",
         (Showsint) charcomp,
         (Showsint) virtualtree->multiseq.totallength,
         (double) charcomp/virtualtree->multiseq.totallength);
  DEBUG1(1,"# maxstacksize=%lu\n",(Showuint) mkvaux->maxstacksize);
  FREEARRAY(&mkvauxstack,MKVstack);
  return 0;
}
