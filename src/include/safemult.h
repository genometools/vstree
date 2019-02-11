//\IgnoreLatex{

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "types.h"

//}

/*
  This module defines a functions to check for
  overflows before multiplying values of 
  type \texttt{Safearithtype}. This type has to
  be defined in the file including this header file.

  The following defines the larges possible value of the
  type \texttt{Safearittype}.
*/

#define MAXSAFEARITHMULT    (~((Safearithtype) 0))

#define SAFEARITHMULT(A,B)  safearithmult(__FILE__,(Uint) __LINE__,A,B)

/*
  The following function multiplies two values \texttt{a} and \texttt{b}
  if the product is smaller or equal than the maximal value of
  type \texttt{Safearithtype}. If the product is larger, then an
  error is thrown and the computation stops.
*/

static Safearithtype safearithmult(char *filename, Uint line,
                                   Safearithtype a, Safearithtype b)
{
  if(a == 0 || b == 0)
  {
    return 0;
  }
  if(a >= MAXSAFEARITHMULT/b)
  {
    fprintf(stderr,"(%s,%lu): ",filename,(Showuint) line);
    fprintf(stderr,"overflow while multiplying %lu and %lu\n",
            (Showuint) a,
            (Showuint) b);
    exit(EXIT_FAILURE);
  } 
  return a * b;
}
