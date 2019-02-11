
//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "divmodmul.h"
#include "virtualdef.h"
#include "arraydef.h"
#include "debugdef.h"
#include "failures.h"
#include "errordef.h"

#ifdef COUNT
#define COUNTHIT virtualtree->llvcachehit++
#else
#define COUNTHIT /* Nothing */
#endif

#ifdef DEBUG

static PairUint *getexceptionslow(Virtualtree *virtualtree,Uint key)
{
  Uint len;
  PairUint *leftptr, *midptr, *rightptr;

  NOTSUPPOSEDTOBENULL(virtualtree->largelcpvalues.spacePairUint);
  leftptr = virtualtree->largelcpvalues.spacePairUint;
  rightptr = virtualtree->largelcpvalues.spacePairUint +
             virtualtree->largelcpvalues.nextfreePairUint - 1;
  DEBUG3(3,"getexception(%lu,%lu,%lu)\n",(Showuint) key,
                                         (Showuint) leftptr->uint0,
                                         (Showuint) rightptr->uint0);
  while (leftptr<=rightptr)
  {
    len = (Uint) (rightptr-leftptr);
    midptr = leftptr + DIV2(len);
    if(key < midptr->uint0)
    {
      rightptr = midptr - 1;
    } else
    {
      if(key > midptr->uint0)
      {
        leftptr = midptr + 1;
      } else
      {
        return midptr;
      }
    }
  }
  fprintf(stderr,"cannot find %lu in exception table",(Showuint) key);
  exit(EXIT_FAILURE);
}

#endif

//}

/*
  The following function keeps track of 
  an \texttt{llvcache} to store previously extracted values
  from table \texttt{llvtab}. If the sought value \texttt{key} is 
  available in the cache, then it is delivered. Otherwise
  it is extracted from table \texttt{llvtab} by a binary search.
*/

static PairUint *getexceptionfast(Virtualtree *virtualtree,Uint key)
{
  Uint len;
  PairUint *leftptr,
           *midptr, 
           *rightptr;

  NOTSUPPOSEDTOBENULL(virtualtree->largelcpvalues.spacePairUint);
#ifdef COUNT
  virtualtree->callgetexception++;
#endif
  if(virtualtree->llvcachemin != NULL)
  {
    if(virtualtree->llvcachemin->uint0 == key)
    {
      COUNTHIT;
      return virtualtree->llvcachemin;
    } 
    if(virtualtree->llvcachemin->uint0 > key)
    {
      if((virtualtree->llvcachemin-1)->uint0 == key)
      {
        COUNTHIT;
        virtualtree->llvcachemin--;
        return virtualtree->llvcachemin;
      }
      leftptr = virtualtree->largelcpvalues.spacePairUint;
    } else
    {
      leftptr = virtualtree->llvcachemin + 1;
    }
    if(virtualtree->llvcachemax->uint0 == key)
    {
      COUNTHIT;
      return virtualtree->llvcachemax;
    } 
    if(virtualtree->llvcachemax->uint0 < key)
    {
      if((virtualtree->llvcachemax+1)->uint0 == key)
      {
        COUNTHIT;
        virtualtree->llvcachemax++;
        return virtualtree->llvcachemax;
      }
      rightptr = virtualtree->largelcpvalues.spacePairUint + 
                 virtualtree->largelcpvalues.nextfreePairUint - 1;
    } else
    {
      rightptr = virtualtree->llvcachemax - 1;
    }
  } else
  {
    leftptr = virtualtree->largelcpvalues.spacePairUint;
    rightptr = virtualtree->largelcpvalues.spacePairUint + 
               virtualtree->largelcpvalues.nextfreePairUint - 1;
  }
  DEBUG3(3,"getexceptionfast(%lu,%lu,%lu)\n",
                               (Showuint) key,
                               (Showuint) leftptr->uint0,
                               (Showuint) rightptr->uint0);
  while (leftptr<=rightptr)
  {
    len = (Uint) (rightptr-leftptr);
    midptr = leftptr + DIV2(len);
    if(key < midptr->uint0)
    {
      rightptr = midptr - 1;
    } else
    {
      if(key > midptr->uint0)
      {
        leftptr = midptr + 1;
      } else
      {
        virtualtree->llvcachemin = virtualtree->llvcachemax = midptr;
        return midptr;
      }
    }
  }
  fprintf(stderr,"cannot find %lu in exception table",(Showuint) key);
  exit(EXIT_FAILURE);
}

#ifdef DEBUG

PairUint *getexception(Virtualtree *virtualtree,Uint key)
{

  PairUint *ptr1, *ptr2;

  ptr1 = getexceptionslow(virtualtree,key);
  ptr2 = getexceptionfast(virtualtree,key);
  if(ptr1 != ptr2)
  {
    fprintf(stderr,"key=%lu: ptr1=%lu,ptr2=%lu\n",(Showuint) key,
                    (Showuint) (ptr1-virtualtree->largelcpvalues.spacePairUint),
                    (Showuint) (ptr2-virtualtree->largelcpvalues.spacePairUint));
    exit(EXIT_FAILURE);
  }
  return ptr1;
}

#else

/*EE
  The following function directly calls \texttt{getexceptionfast}
  to lookup an lcp-value.
*/

PairUint *getexception(Virtualtree *virtualtree,Uint key)
{
  return getexceptionfast(virtualtree,key);
}

#endif
