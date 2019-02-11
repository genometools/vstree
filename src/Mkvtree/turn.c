#include <stdio.h>
#include <string.h>
#include "types.h"
#include "virtualdef.h"
#define ALPHA "abcdefg"

/* 
  The number of wheels turned for an alphabet of size k and prefixlength=l
  is \(\sum_{i=1}^{l}k^{i}\) which can be bound by \(2k^{l}\). Hence the
  number of turned wheels is proportional to the number of words of length
  \(l\) of an alphabet of size \(k\).
*/

static void turningwheels(char *alphabet,Uint prefixlength)
{
  Uint j, lastturned, z = prefixlength-1, w[64+1] = {0};
  Uint turns = 0, asize = (Uint) strlen(alphabet);

  while(True)
  {
    for (j=0; j<prefixlength; j++)
    {
      (void) putchar(ALPHA[w[j]]);
    }
    while(True)
    {
      turns++;
      w[z]++;
      lastturned = z;
      if(w[z] == asize) 
      {
	w[z] = 0;
	if(z == 0)
        {
          printf("\nturns=%lu\n",(Showuint) turns);
	  return;
        }
	z--;
      } else 
      {
	z = prefixlength-1;
	break;
      }
    }
    printf(" lcp=%lu\n",(Showuint) lastturned);
  }
}

MAINFUNCTION
{
  Uint prefixlength;
  char *alphabet;

  VSTREECHECKARGNUM(3,"alphabet prefixlength");

  alphabet = argv[1];
  prefixlength = (Uint) atoi(argv[2]);
  turningwheels(alphabet,prefixlength);
  return EXIT_SUCCESS;
}
