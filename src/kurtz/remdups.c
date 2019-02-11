

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include "arraydef.h"

void remdups(ArrayUint *ar)
{
  Uint *storeptr, *readptr;

  if(ar->nextfreeUint > 0)
  {
    for(storeptr = ar->spaceUint, readptr = ar->spaceUint+1;
        readptr < ar->spaceUint + ar->nextfreeUint;
        readptr++)
    {
      if(*storeptr != *readptr)
      {
        storeptr++;
        *storeptr = *readptr;
      }
    }
    ar->nextfreeUint = (Uint) (storeptr - ar->spaceUint + 1);
  }
}
