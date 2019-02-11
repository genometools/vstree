#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include "types.h"

BOOL islittleendian(void)
{
  int x = 1;

  if(*(char *) &x == 1)
  {
    return True;
  } else
  {
    return False;
  }
}
