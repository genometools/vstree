//\IgnoreLatex{

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "types.h"

//}

/*
  This module defines a functions to check for
  overflows before adding values of 
  type \texttt{Safearithtype}. This type has to
  be defined in the file including this header file.

  The following defines the larges possible value of the
  type \texttt{Safearittype}.
*/

#define MAXSAFEARITHADD    (~((Safearithtype) 0))

#define SAFEARITHADD(A,B)  safearithadd(__FILE__,(Uint) __LINE__,A,B)

/*
  The following function adds two values \texttt{a} and \texttt{b}
  if the sum is smaller or equal than the maximal value of
  type \texttt{Safearithtype}. If the sum is larger, then an
  error is thrown and the computation stops.
*/

static Safearithtype safearithadd(char *filename, Uint line,
                                  Safearithtype a, Safearithtype b)
{
  if(a == 0)
  {
    return b;
  }
  if(b == 0)
  {
    return a;
  }
  if(a >= MAXSAFEARITHADD-b)
  {
    fprintf(stderr,"(%s,%lu): ",filename,(Showuint) line);
    fprintf(stderr,"overflow while adding %lu and %lu\n",
            (Showuint) a,
            (Showuint) b);
    exit(EXIT_FAILURE);
  } 
  return a + b;
}
