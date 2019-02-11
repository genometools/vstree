
//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include "debugdef.h"
#include "errordef.h"

//}

/*EE
  The following function copies the 0-terminated string pointed to by
  \texttt{source} to the memory area pointed to by \texttt{dest}, provided
  \texttt{source} is shorter than \texttt{maxlen}. If this is not 
  true, then the function returns a negative error code. Otherwise
  the return value is 0.
*/

Sint safestringcopy(char *dest,const char *source,Sint maxlen)
{ 
  Sint slen;

  slen = (Sint) strlen(source);
  if(slen >= maxlen)
  {
    ERROR2("string \"%s\" is too long, cannot copy, maximum length is %ld",
           source,(Showsint) maxlen);
    return (Sint) -1;
  }
  strcpy(dest,source);
  return 0;
}
