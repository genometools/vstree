//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "types.h"
#include "intbits.h"

//}

/*
  This module defines two functions to check for change of values before
  casting a \texttt{Sint} to \texttt{Uint} or a \texttt{Uint} to 
  \texttt{Sint}, respectively.
*/


/*
  The following function casts an \texttt{Uint} \texttt{value} to 
  \texttt{Sint} if the first bit of \texttt{value} is not set, 
  i.e. the cast would not result in a negative value. 
  If the cast would result in a negative value an
  error is thrown and the computation stops.
  These functions should be called via the macros defined in 
  \texttt{safecast.h}.
*/

Sint safecasttoSint(char *filename, Uint line, Uint value)
{
  if(value > EXCEPTFIRSTBIT)
  {
    fprintf(stderr, "(%s,%lu): ",filename,(Showuint) line);
    fprintf(stderr, "illegal cast of Uint to Sint "); 
    fprintf(stderr, "(Uint=%lu)\n" , (Showuint) value);
    exit(EXIT_FAILURE);
  }
  return (Sint) value;
}

/*
  The following function casts an \texttt{Sint} \texttt{value} 
  to \texttt{Uint} if the first bit of \texttt{value} is not set, 
  i.e. the cast would not change
  a negative \textt{Sint} value into a large \texttt{Uint} value.
  If the cast would result in a large value an error is thrown and the 
  computation stops.
*/

Uint safecasttoUint(char *filename, Uint line, Sint value) 
{   
  if(value < 0)
  {
    fprintf(stderr, "(%s,%lu): ",filename,(Showuint) line);
    fprintf(stderr, "illegal cast of Sint to Uint "); 
    fprintf(stderr, "(Sint=%ld)\n" , (Showsint) value);
    exit(EXIT_FAILURE);
  }   
  return (Uint) value;
} 
