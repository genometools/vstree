

//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <limits.h>
#include "types.h"
#include "debugdef.h"
#include "errordef.h"
#include "spacedef.h"

/* Assumption: m < n */

#ifdef DEBUG
static void showecol(Uint *ecol,Uint m)
{
  Uint i;

  for(i=0; i<=m; i++)
  {
    printf("%lu",(Showuint) ecol[i]);
    if(i == m)
    {
      (void) putchar('\n');
    } else
    {
      (void) putchar(' ');
    }
  }
}
#endif

//}

/*
  This module implements functions computing unit edit distances.
  The following function computes the unit edit distance of two
  sequences $u$ and $v$ using $O(m)$ space.
*/

static Uint edistunit (Uchar *u, Uint m, Uchar *v, Uint n)
{
  Uint val, we, nw, *ecol, *ecolptr;
  Uchar *uptr, *vptr;

  ALLOCASSIGNSPACE(ecol,NULL,Uint,m+1);
  for(*ecol = 0, ecolptr = ecol+1, uptr = u; uptr < u + m; ecolptr++, uptr++)
  {
    *ecolptr = *(ecolptr-1) + 1;
  }
  for (vptr = v; vptr < v + n; vptr++)
  {
    nw = *ecol;
    *ecol = nw + 1;
    for(ecolptr = ecol+1, uptr = u; uptr < u + m; ecolptr++, uptr++)
    {
      we = *ecolptr;
      *ecolptr = *(ecolptr-1) + 1;
      if(*uptr == *vptr)
      {
        val = nw;
      } else
      {
        val = nw + 1;
      }
      if(val < *ecolptr)
      {
	*ecolptr = val;
      } 
      if((val = we + 1) < *ecolptr)
      {
        *ecolptr = val;
      }
      nw = we;
    }
    DEBUGCODE(3,showecol(ecol,m));
  }
  val = *(ecolptr-1);
  DEBUG1(2,"edist=%lu\n",(Showuint) val);
  FREESPACE(ecol);
  return val;
}

/*EE
  The following function computes the unit edit distance of two
  sequences of length \(m\) and \(n\) using $\min\{m,n\}$.
*/

Uint calledistunit (Uchar *u, Uint m, Uchar *v, Uint n)
{
  if (m < n)
  {
    return edistunit(u,m,v,n);
  } 
  return edistunit(v,n,u,m);
}
