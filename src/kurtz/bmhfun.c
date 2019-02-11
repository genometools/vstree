
//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "types.h"
#include "arraydef.h"
#include "fhandledef.h"

#include "filehandle.pr"

//}

/*EE
  This file implements two functions using the Boyer-Moore Horspool Algorithm
  for exact string searching. \texttt{bmhcount} counts the number of 
  exact string matches of string \(p\) in string \(t\). The length
  of \(p\) is \(m\) and the length of \(t\) is \(n\).
*/

Uint bmhcount(Uchar *p, Uint m, Uchar *t, Uint n)
{
  Uint count = 0, i, j, rmostocc[UCHAR_MAX + 1] = {UintConst(0)};
  Sint k;

  for(i=0; i<m-1; i++)
  {
    rmostocc[(Sint) p[i]] = i+1;
  }
  for(j = 0; j <= n-m; j += m-rmostocc[(Sint) t[j+m-1]])
  {
    for(k=(Sint) m-1; k>=0 && p[k] == t[j+(Uint) k]; k--)
      /* nothing */ ;
    if (k < 0)
    {
      count++;
    }
  }
  return count;
}

/*EE
  \texttt{bmhcheck} takes a list of positions in the array 
  \texttt{result}. The positions must be ordered. The function exits
  with an error code, if the list of matches found by the Boyer Moore
  Horspool Algorithm differs from the \texttt{result}-list.
  The length of \(p\) is \(m\) and the length of \(t\) is \(n\).
*/

void bmhcheck(Uchar *p, Uint m, Uchar *t, Uint n,ArrayUint *result)
{
  Uint r = 0, i, j, rmostocc[UCHAR_MAX +1] = {0};
  Sint k;

  for(i=0; i<m-1; i++)
  {
    rmostocc[(Sint) p[i]] = i+1;
  }
  for(j = 0; j <= n-m; j += m-rmostocc[(Sint) t[j+m-1]])
  {
    for(k=(Sint) (m-1); k>=0 && p[k] == t[j+(Uint) k]; k--)
      /* nothing */ ;
    if (k < 0)
    {
      if(r >= result->nextfreeUint)
      {
        fprintf(stderr,"Pattern \"");
        if(WRITETOFILEHANDLE(p,(Uint) sizeof(Uchar),m,stderr) != 0)
        {
          fprintf(stderr,"%s\n",messagespace());
        } else
        {
          fprintf(stderr,"\": resultlist is too short\n");
        }
        exit(EXIT_FAILURE);
      }
      if(j != result->spaceUint[r])
      {
        fprintf(stderr,"Pattern \"");
        if(WRITETOFILEHANDLE(p,(Uint) sizeof(Uchar),m,stderr) != 0)
        {
          fprintf(stderr,"%s\n",messagespace());
        } else
        {
          fprintf(stderr,"\" %lu: found position %lu != %lu\n",
                  (Showuint) r,
                  (Showuint) j,
                  (Showuint) result->spaceUint[r]);
        }
        exit(EXIT_FAILURE);
      }
      r++;
    }
  }
  if(r != result->nextfreeUint)
  {
    fprintf(stderr,"Pattern \"");
    if(WRITETOFILEHANDLE(p,(Uint) sizeof(Uchar),m,stderr) != 0)
    {
      fprintf(stderr,"%s\n",messagespace());
    } else
    {
       fprintf(stderr,"\" r=%lu != %lu = nextfreeUint\n",
               (Showuint) r,
               (Showuint) result->nextfreeUint);
    }
    exit(EXIT_FAILURE);
  }
}

void bmhout(FILE *fp,Uchar *p, Uint m, Uchar *t, Uint n)
{
  Uint i, j, rmostocc[UCHAR_MAX + 1] = {UintConst(0)};
  Sint k;

  for(i=0; i<m-1; i++)
  {
    rmostocc[(Sint) p[i]] = i+1;
  }
  for(j = 0; j <= n-m; j += m-rmostocc[(Sint) t[j+m-1]])
  {
    for(k=(Sint) m-1; k>=0 && p[k] == t[j+(Uint) k]; k--)
      /* nothing */ ;
    if (k < 0)
    {
      fprintf(fp,"%lu\n",(Showuint) j);
    }
  }
}
