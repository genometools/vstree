
//\IgnoreLatex{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "divmodmul.h"
#include "spacedef.h"
#include "multidef.h"
#include "virtualdef.h"
#include "vnodedef.h"
#include "errordef.h"
#include "debugdef.h"
#include "chardef.h"

//}

/*
  This module contains functions for performing binary searches
  on a suffix interval in a suffix array.

  The following macro delivers the character at position
  \texttt{I}.
*/

#define SEQUENCE(M,I)\
        (((I) == (M)->totallength) ? SEPARATOR : (M)->sequence[I])

/*
  The following function computes the right boundary for character \emph{cc}
  w.r.t.\ a suffix interval given by \(l\), \(r\), and \emph{offset}.
  We use a binary search to find the boundary.
*/

static Uint findright(Multiseq *multiseq,Uint *suftab,
                      Uint offset,Uchar cc,Uint l,Uint r)
{
  Uint pos, mid;
  Uchar midcc;

  DEBUG4(3,"findright(offset=%lu,cc=%lu,l=%lu,r=%lu)\n",(Showuint) offset,
                                                        (Showuint) cc,
                                                        (Showuint) l,
                                                        (Showuint) r);
  while(r > l+1)
  {
    DEBUG2(3,"   check(l=%lu,r=%lu)\n",(Showuint) l,(Showuint) r);
    mid = DIV2(l+r);
    pos = suftab[mid]+offset;
    midcc = SEQUENCE(multiseq,pos);
    if(midcc > cc)
    {
      r = mid;
    } else
    {
      l = mid;
    }
  }
  DEBUG1(3,"result=%lu\n",(Showuint) l);
  return l;
}

/*EE
  The following function computes the lcp value of a suffix interval
  in the suffix array.
*/

Uint binlcpvalue(Multiseq *multiseq,Uint *suftab,Uint offset,Uint l,Uint r)
{
  Uchar *lptr, *rptr;
  
  for(lptr = multiseq->sequence + suftab[l] + offset,
      rptr = multiseq->sequence + suftab[r] + offset; 
      lptr < multiseq->sequence + multiseq->totallength &&
      rptr < multiseq->sequence + multiseq->totallength;
      lptr++, rptr++)
  {
    if(*lptr != *rptr || ISSPECIAL(*lptr) || ISSPECIAL(*rptr))
    {
      break;
    }
  }
  return (Uint) (lptr - (multiseq->sequence + suftab[l]));
}

//\Ignore{

#ifdef DEBUG
#define CHECKVBOUND\
        if(vboundscount >= vboundsize)\
        {\
          fprintf(stderr,"%s, line %lu: "\
                         "vboundscount=%lu >= %lu=vboundsize\n",\
                         __FILE__,(Showuint) __LINE__,\
                         (Showuint) vboundscount,(Showuint) vboundsize);\
          exit(EXIT_FAILURE);\
        }
#else
#define CHECKVBOUND /* Nothing */
#endif

//}

/*EE
  The following function splits a suffix interval given by \(l\), \(r\),
  and \emph{offset}. The result is stored in the Array \texttt{vbounds}.
  The last index in \texttt{vbounds} storing a value is returned.
*/

Uint splitnodewithcharbin(Multiseq *multiseq,Uint *suftab,
                          Vbound *vbounds,Uint vboundsize,
                          Uint lcpvalue,Uint i,Uint j)
{
  Uchar leftcc, rightcc;
  Uint vboundscount = 0, rightbound, leftbound = i;

  DEBUG3(3,"\nsplitnode(lcpvalue=%lu,l=%lu,r=%lu)\n",(Showuint) lcpvalue,
                                                     (Showuint) i,
                                                     (Showuint) j);
  rightcc = SEQUENCE(multiseq,suftab[j]+lcpvalue);
  while(True)
  {
    leftcc = SEQUENCE(multiseq,suftab[leftbound]+lcpvalue);
    DEBUG2(3,"(1) set bound %lu-(%lu,?)\n",(Showuint) leftcc,
                                           (Showuint) leftbound);
    CHECKVBOUND;
    /*printf("(1) vbounds[%lu]=%lu(%ld)\n",(Showuint) vboundscount,
                                           (Showuint) leftbound,
                                           (Showsint) leftcc);*/
    vbounds[vboundscount].bound = leftbound;
    vbounds[vboundscount++].inchar = leftcc;
    if(leftcc == rightcc || ISSPECIAL(leftcc))
    { 
      break;
    }
    rightbound = findright(multiseq,suftab,lcpvalue,leftcc,leftbound,j);
    leftbound = rightbound+1;
  }
  if(ISSPECIAL(leftcc))
  {
    while(leftbound < j)
    {
      CHECKVBOUND;
      /*printf("(2) vbounds[%lu]=%lu(%lu)\n",(Showuint) vboundscount,
                                             (Showuint) leftbound,
                                             (Showsint) SEPARATOR);*/
      vbounds[vboundscount].bound = leftbound+1;
      vbounds[vboundscount++].inchar = SEPARATOR;
      leftbound++;
    }
  }
  CHECKVBOUND;
  //printf("(3) vbounds[%lu]=%lu\n",(Showuint) vboundscount,(Showuint) (j+1));
  vbounds[vboundscount].bound = j+1;
  return vboundscount;
}

/*EE
  The following function splits a suffix interval given by \(l\), \(r\),
  and \emph{offset}. The result is stored in the Array \texttt{vbounds}.
  The last index in \texttt{vbounds} storing a value is returned.
*/

Uint splitnodewithcharbinwithoutspecial(Multiseq *multiseq,Uint *suftab,
                                        Vbound *vbounds,Uint vboundsize,
                                        Uint lcpvalue,Uint i,Uint j)
{
  Uchar leftcc, rightcc;
  Uint vboundscount = 0, rightbound = 0, leftbound = i;

  DEBUG3(3,"\nsplitnode(lcpvalue=%lu,l=%lu,r=%lu)\n",(Showuint) lcpvalue,
                                                     (Showuint) i,
                                                     (Showuint) j);
  rightcc = SEQUENCE(multiseq,suftab[j]+lcpvalue);
  while(True)
  {
    leftcc = SEQUENCE(multiseq,suftab[leftbound]+lcpvalue);
    DEBUG2(3,"(1) set bound %lu-(%lu,?)\n",(Showuint) leftcc,
                                           (Showuint) leftbound);
    CHECKVBOUND;
    /*printf("(1) vbounds[%lu]=%lu(%ld)\n",(Showuint) vboundscount,
                                           (Showuint) leftbound,
                                           (Showsint) leftcc);*/
    if(ISSPECIAL(leftcc))
    {
      vbounds[vboundscount].bound = rightbound+1;
      return vboundscount;
    }
    vbounds[vboundscount].bound = leftbound;
    vbounds[vboundscount++].inchar = leftcc;
    if(leftcc == rightcc)
    { 
      break;
    }
    rightbound = findright(multiseq,suftab,lcpvalue,leftcc,leftbound,j);
    leftbound = rightbound+1;
  }
  CHECKVBOUND;
  //printf("(3) vbounds[%lu]=%lu\n",(Showuint) vboundscount,(Showuint) (j+1));
  vbounds[vboundscount].bound = j+1;
  return vboundscount;
}

/*EE
  The following function computes the child interval of a suffix interval
  for a given character \texttt{cc}. The return value is \texttt{True}
  iff there is such an interval.
*/

BOOL findcharintervalbin(Multiseq *multiseq,Uint *suftab,Vnode *vnode,
                         Uchar cc,Uint lcpvalue,Uint i,Uint j)
{
  Uchar leftcc, rightcc;
  Uint pos, rightbound, leftbound = i;

  DEBUG4(2,"findcharinterval(d=%lu,l=%lu,r=%lu,cc=%lu)",
           (Showuint) lcpvalue,
           (Showuint) i,
           (Showuint) j,
           (Showuint) cc);
  pos = suftab[j]+lcpvalue;
  rightcc = SEQUENCE(multiseq,pos);
  while(True)
  {
    pos = suftab[leftbound]+lcpvalue;
    leftcc = SEQUENCE(multiseq,pos);
    DEBUG2(3,"(1) set bound %lu-(%lu,?)\n",(Showuint) leftcc,
                                           (Showuint) leftbound);
    if(leftcc == rightcc)
    {
      break;
    }
    rightbound = findright(multiseq,suftab,lcpvalue,leftcc,leftbound,j);
    if(leftcc == cc)
    {
      vnode->left = leftbound;
      vnode->right = rightbound; 
      DEBUG2(2,"=>(%lu,%lu)\n",(Showuint) vnode->left,
                               (Showuint) vnode->right);
      return True;
    }
    if(leftcc > cc)
    {
      DEBUG0(2,"=>False\n");
      return False;
    }
    leftbound = rightbound+1;
  }
  if(leftcc == cc)
  {
    vnode->left = leftbound;
    vnode->right = j;
    DEBUG2(2,"=>(%lu,%lu)\n",(Showuint) vnode->left,
                             (Showuint) vnode->right);
    return True;
  }
  DEBUG0(2,"=>False\n");
  return False;
}
