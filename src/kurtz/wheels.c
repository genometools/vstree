
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "errordef.h"

Sint turnwheels(Uint *asizes,Uint numofalphabets,Uint *wheelspace, 
                Sint(*outwheel)(void *,Uint *),void *wheelinfo)
{
  Uint z;
  Sint retcode;

  for(z=0; z<numofalphabets; z++)
  {
    if(asizes[z] == 0)
    {
      return 0;
    }
    wheelspace[z] = 0;
  }
  z = numofalphabets-1;
  while(True)
  {
    retcode = outwheel(wheelinfo,wheelspace);
    if(retcode != 0)
    {
      return retcode;
    }
    while(True)
    {
      wheelspace[z]++;
      if(wheelspace[z] == asizes[z]) 
      {
        wheelspace[z] = 0;
        if(z == 0)
        {
          return 0;
        }
        z--;
      } else 
      {
        z = numofalphabets-1;
        break;
      }
    }
  }
  /*@notreached@*/ return 0;
}
