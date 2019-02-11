
#ifdef DEBUG

//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "debugdef.h"
#include "dpbitvec48.h"

//}

/*EE
  This file implements some debugging function for the
  algorithms implementing Myers bitvector algorithms for
  approximate matching. They are only defined if the debug compiler
  option is on.
*/

/*
  The following function computes a bitmask to pretty print a bit vector
  in blocks of eight bits.
*/

static DPbitvector4 getshowmask(void)
{
  DPbitvector4 showmask = 0;

  Uint i;
  for(i = UintConst(1); i < (Uint) DPBYTESINWORD; i++)
  {
    showmask = (showmask << 8) | (ONEDPbitvector4 << 8);
  }
  return showmask;
}

/*EE
  The following function shows a bitvector \(v\).
*/

void showbitvector(DPbitvector4 v)
{
  DPbitvector4 leftbit, showmask = getshowmask();
  
  for(leftbit = (DPbitvector4) LEFTMOSTONEDPbitvector4;
      leftbit != 0; 
      leftbit >>= 1)
  {
    (void) putchar((v & leftbit) ? '1' : '0');
    if (leftbit & showmask)
    {
      (void) putchar(' ');
    }
  }
}

/*EE
  The following function shows \texttt{len} bitvectors in table
  \texttt{vs}.
*/

void showallbitvectors(DPbitvector4 *vs,Uint len)
{
  Uint i;

  DEBUG1(2,"Wordsize=%lu\n",(Showuint) DPWORDSIZE4);

  for(i = 0; i < len; i++)
  {
    if(vs[i] != 0)
    {
      printf("vs['%c']=",(char) i);
      showbitvector(vs[i]);
      (void) putchar('\n');
    }
  }
}

/*EE
  The following function shows the last \texttt{plen} bits of a 
  bit vector.
*/

void showbitvectorsuffix(DPbitvector4 v,Uint plen)
{
  DPbitvector4 leftbit, showmask = getshowmask();
  
  for(leftbit = (DPbitvector4) (ONEDPbitvector4 << (plen-1)); 
      leftbit != 0; 
      leftbit >>= 1)
  {
    (void) putchar((v & leftbit) ? '1' : '0');
    if (leftbit & showmask)
    {
      (void) putchar(' ');
    }
  }
}

/*EE
  The following function shows a distance column for a pattern of
  length \texttt{plen} represented by the bitvectors
  \texttt{plus} and \texttt{minus}. The first value of the 
  distance column is \texttt{firstval}.
*/

void showcolumn(Uint firstval,Uint plen,DPbitvector4 plus,DPbitvector4 minus)
{
  Uint i, score = firstval;
  DPbitvector4 mask = ONEDPbitvector4;

  printf("Pv=");
  showbitvectorsuffix(plus,plen);
  printf(", Mv=");
  showbitvectorsuffix(minus,plen);
  printf(", ");
  printf("%lu ",(Showuint) score);
  for(i=0; i < plen; i++, mask <<= 1)
  {
    if(plus & mask)
    {
      score++;
    } else
    {
      if(minus & mask)
      {
        score--;
      }
    }
    printf("%lu",(Showuint) score);
    if(i < plen - 1)
    {
      (void) putchar(' ');
    }
  }
}

#endif  /* Nothing */
