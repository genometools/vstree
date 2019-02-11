
//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include "intbits.h"

//}

/*EE
  This module implements functions for converting integers to
  bitstrings and vice versa. The following function stores the
  bitstring representation of an unsigned integer in a static memory
  area and returns a pointer to this area.
*/

char *intbits2string(Uint bs)
{
  Uint i;
  char *csptr;
  static char cs[INTWORDSIZE+(INTWORDSIZE/CHAR_BIT)-1+1];

  for(csptr=cs, i=0; i < INTWORDSIZE; i++)
  {
     if(ISBITSET(bs,i))
     {
       *csptr = '1';
     } else
     {
       *csptr = '0';
     }
     csptr++;
     if ((i+1) < INTWORDSIZE && (i+1) % CHAR_BIT == 0)
     {
       *csptr = ' ';
       csptr++;
     }
  }
  *csptr = '\0';
  return cs;
}

/*EE
  The following function converts a 0-terminated bitstring 
  into an unsigned integer and returns this.
*/

Uint string2uint(char *cs)
{
  char *csptr;
  Uint len, val = 0;

  for(len = 0, csptr = cs; *csptr != '\0'; csptr++)
  {
    switch(*csptr)
    {
      case '0': val <<= 1; 
                len++; 
                break;
      case '1': val += 1; 
                val <<= 1; 
                len++; 
                break;
      case ' ': break;
      default : 
        fprintf(stderr,"Illegal character %c in binary string\n",*csptr);
        exit(EXIT_FAILURE);
    }
    if(len > INTWORDSIZE)
    {
      fprintf(stderr,"Overflow for %s\n",cs);
      exit(EXIT_FAILURE);
    }
  }
  return val;
}
