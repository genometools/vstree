#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "chardef.h"
#include "visible.h"
#include "alphadef.h"

Uchar verbosealphachar(const Alphabet *alpha,Uint ind)
{
  if(ind == alpha->mapsize-1)
  {
    return alpha->characters[WILDCARD];
  }
  return alpha->characters[ind];
}

/*EE
  The following function stores the alphabet size and the characters of the
  alphabet into the buffer \texttt{sbuf} and applies the function 
  \texttt{showverbose} to this buffer.
*/

void verbosealphabet(Alphabet *alpha,Showverbose showverbose)
{
  Uchar c;
  char sbuf[4096];
  Uint start, i;

  start = (Uint) sprintf(sbuf,"alphabet \"");
  for(i=0; i < alpha->domainsize; i++)
  {
    c = alpha->mapdomain[i];
    if(INVISIBLE(c))
    {
      start += sprintf(sbuf+start,"\\%lu",(Showuint) c);
    } else
    {
      start += sprintf(sbuf+start,"%c",c);
    }
  }
  start += sprintf(sbuf+start,"\" (size %lu) mapped to \"",
                   (Showuint) alpha->domainsize);
  for(i=0; i < alpha->mapsize; i++)
  {
    c = verbosealphachar(alpha,i);
    if(INVISIBLE(c))
    {
      start += sprintf(sbuf+start,"\\%lu",(Showuint) c);
    } else
    {
      start += sprintf(sbuf+start,"%c",c);
    }
  }
  sprintf(sbuf+start,"\" (size %lu)",(Showuint) alpha->mapsize);
  showverbose(&sbuf[0]);
}
