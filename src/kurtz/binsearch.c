//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "divmodmul.h"
#include "errordef.h"
#include "failures.h"
#include "debugdef.h"

//}

/*EE
  This file implements some binary search functions over integer arrays
  and over string arrays.
*/

/*EE
  The following function finds smallest integer larger than \texttt{key}
  in the table whose boundaries are marked by \texttt{left} and \texttt{right}.
  The table must be sorted in ascending order.
  The function returns a pointer to this entry. If such an entry does not
  exist, then the return value is \texttt{NULL}.
*/

/*@null@*/ Sint *binsearch(Sint key, Sint *left, Sint *right)
{
  Sint *leftptr, *midptr, *rightptr;
  Uint len;

  NOTSUPPOSEDTOBENULL(left);
  NOTSUPPOSEDTOBENULL(right);
  DEBUG3(3,"binsearch(%ld,%ld,%ld)\n",(Showsint) key,
                                      (Showsint) *left,
                                      (Showsint) *right);
  leftptr = left;
  rightptr = right;
  while (leftptr<=rightptr)
  {
    len = (Uint) (rightptr-leftptr);
    midptr = leftptr + DIV2(len);
    if(key < *midptr)
    {
      rightptr = midptr - 1;
    } else
    {
      if(key > *midptr)
      {
        leftptr = midptr + 1;
      } else
      {
        return midptr;
      }
    }
  }
  return NULL;
}

/*EE
  The following function finds the string \texttt{key} in 
  the table whose boundaries are marked by \texttt{left} and \texttt{right}.
  All strings are 0-terminated and they are lexicographically ordered in
  the table. The function returns a pointer to the table entry.
  If string does occur the the return value is \texttt{NULL}.
*/

/*@null@*/ char **binsearchstring(char *key, char **left, char **right)
{
  char **leftptr = left, **midptr, **rightptr = right;
  Strcmpreturntype cmpval;
  Uint len;

  DEBUG1(3,"binsearchstring(%s) in\n",key);
  while (leftptr<=rightptr)
  {
    len = (Uint) (rightptr-leftptr);
    midptr = leftptr + DIV2(len); // halve len
    DEBUG1(3,"search in (%lu,",(Showuint) (leftptr - left));
    DEBUG2(3,"%lu,%lu)\n",(Showuint) (midptr-left),(Showuint) (rightptr-left));
    cmpval = strcmp(*midptr,key);
    if(cmpval < 0)
    {
      leftptr = midptr + 1;
    } else
    {
      if(cmpval > 0)
      {
        rightptr = midptr-1;
      } else
      {
        return midptr;
      }
    }
  }
  return NULL;
}

/*@null@*/ PairUint *binsearchPairUint(Uint key,PairUint *left,PairUint *right)
{
  PairUint *leftptr = left, *midptr, *rightptr = right;
  Uint len;

  DEBUG1(3,"binsearchPairUint(%lu) in\n",(Showuint) key);
  while (leftptr<=rightptr)
  {
    len = (Uint) (rightptr-leftptr);
    midptr = leftptr + DIV2(len); // halve len
    DEBUG1(3,"search in (%lu,",(Showuint) (leftptr - left));
    DEBUG2(3,"%lu,%lu)\n",(Showuint) (midptr-left),(Showuint) (rightptr-left));
    if(key < midptr->uint0)
    {
      rightptr = midptr-1;
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
  return NULL;
}
