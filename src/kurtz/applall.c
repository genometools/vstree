//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "debugdef.h"

//}

/*EE
  This file implements functions to generate all string of a fixed
  length over a given alphabet. The basic idea for the code
  was developed by Barbara Roth, broth@techfak.uni-bielefeld.de
*/

/*
  We allow to generate strings of the following maximal length.
*/

#define MAXGENLEN 30

/*EE
  The following function enumerates the strings of length \texttt{textlen}
  over alphabet \texttt{alpha}. To each string enumerated, it
  applies the function \texttt{apply}, using \texttt{info} as the
  third argument.
*/

Sint applyall(char *alpha,Uint textlen,void *info,
              Sint (*apply)(Uchar *,Uint,void *))
{
  Uint w[MAXGENLEN+1], z = textlen-1, asize = (Uint) strlen(alpha);
  Uchar text[MAXGENLEN+1];
  Sint i;

  if(textlen > (Uint) MAXGENLEN)
  {
    fprintf(stderr,"textlen = %lu > %lu not allowed\n",
            (Showuint) textlen,(Showuint) MAXGENLEN);
    exit(EXIT_FAILURE);
  }
  for(i=0; i<=(Sint) MAXGENLEN; i++)
  {
    w[i] = UintConst(0);
  }
  text[textlen] = (Uchar) '\0';

  while(True)
  {
    for (i = (Sint) (textlen-1); i>=0; i--)
    {
      text[i] = (Uchar) alpha[w[i]];
    }
    if(apply(text,textlen,info) != 0)
    {
      return (Sint) -1;
    }
    while(True)
    {
      w[z]++;
      if(w[z] == asize) 
      {
        w[z] = 0;
        if(z == 0)
        {
          return 0;
        }
        z--;
      } else 
      {
        z = textlen-1;
        /*@innerbreak@*/ break;
      }
    }
  }
  /*@ignore@*/
  return 0;
  /*@end@*/
}

/*EE
  The following function enumerates the strings of length \texttt{textlen}
  over the alphabet \texttt{[0..asize-1]}. To each string enumerated, it
  applies the function \texttt{apply}, using \texttt{info} as the
  third argument.
*/

Sint applyallasize(Uint asize,Uint textlen,void *info,
                   Sint (*apply)(Uchar *,Uint,void *))
{
  Uchar w[MAXGENLEN+1];
  Uint i, z = textlen-1;

  for(i=0; i<=(Uint) MAXGENLEN; i++)
  {
    w[i] = UintConst(0);
  }
  if(textlen > (Uint) MAXGENLEN)
  {
    fprintf(stderr,"textlen = %lu > %lu not allowed\n",
            (Showuint) textlen,(Showuint) MAXGENLEN);
    exit(EXIT_FAILURE);
  }

  while(True)
  {
    if(apply(w,textlen,info) != 0)
    {
      return (Sint) -1;
    }
    while(True)
    {
      w[z]++;
      if(w[z] == (Uchar) asize)
      {
        w[z] = 0;
        if(z == 0)
        {
          return 0;
        }
        z--;
      } else 
      {
        z = textlen-1;
        /*@innerbreak@*/ break;
      }
    }
  }
  /*@ignore@*/
  return 0;
  /*@end@*/
}
