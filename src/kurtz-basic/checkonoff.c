//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "debugdef.h"
#include "errordef.h"

//}

/*EE
  The following function checks a given environment variable whether
  it is set on or off.
*/

Sint checkenvvaronoff(char *varname)
{
  char *envstring;

  if((envstring = getenv(varname)) != NULL)
  {
    if(strcmp(envstring,"on") == 0)
    {
      return (Sint) 1;
    } else
    {
      if(strcmp(envstring,"off") == 0)
      {
        return 0;
      } 
      ERROR1("environment variable %s must set \"on\" or \"off\"",varname);
      return (Sint) -1;
    }
  }
  return 0;
}
