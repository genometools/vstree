#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "debugdef.h"
#include "errordef.h"
#include "alphadef.h"

#include "verbosealpha.pr"

/*
  The following macro compares two integers which are in part of an
  alphabet structure.
*/

#define COMPAREALPHAINT(INT)\
        if(alpha1->INT != alpha2->INT)\
        {\
          ERROR3("different alphabets for virtual trees: %s: %lu != %lu",#INT,\
                   (Showuint) alpha1->INT,\
                   (Showuint) alpha2->INT);\
          return (Sint) -1;\
        }

Sint compareAlphabet(Alphabet *alpha1,Alphabet *alpha2)
{
  Uint i; 
  Uchar cc1, cc2;

  COMPAREALPHAINT(domainsize);
  COMPAREALPHAINT(mapsize);
  COMPAREALPHAINT(mappedwildcards);
  COMPAREALPHAINT(undefsymbol);
  for(i=0; i<alpha1->mapsize; i++)
  {
    cc1 = verbosealphachar(alpha1,i);
    cc2 = verbosealphachar(alpha2,i);
    if(cc1 != cc2)
    {
      ERROR3("different alphabets for virtual trees: char[%lu]: %lu != %lu",
              (Showuint) i,
              (Showuint) cc1,
              (Showuint) cc2);
      return (Sint) -1;
    }
  }
  for(i=0; i<=UCHAR_MAX; i++)
  {
    if(alpha1->symbolmap[i] != alpha2->symbolmap[i])
    {
      ERROR3("different alphabets for virtual trees: symbolmap[%lu]: %lu!=%lu",
             (Showuint) i,
             (Showuint) alpha1->symbolmap[i],
             (Showuint) alpha2->symbolmap[i]);
      return (Sint) -1;
    }
  }
  for(i=0; i<alpha1->domainsize; i++)
  {
    if(alpha1->mapdomain[i] != alpha2->mapdomain[i])
    {
      ERROR3("different alphabets for virtual trees: mapdomain[%lu]: %c!=%c",
             (Showuint) i,
             alpha1->mapdomain[i],
             alpha2->mapdomain[i]);
      return (Sint) -1;
    }
  }
  return 0;
}
