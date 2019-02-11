#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "spacedef.h"
#include "failures.h"

/*EE
  The following function makes a copy of a 0-terminated string pointed to by 
  \texttt{source}. 
*/

/*@notnull@*/ char *vm_dynamicstrdup(char *file,Uint linenum,
                                     const char *source)
{
  Uint sourcelength;
  char *dest;

  NOTSUPPOSEDTOBENULL(source);
  sourcelength = (Uint) strlen(source);
  ALLOCASSIGNSPACEGENERIC(file,linenum,dest,NULL,char,sourcelength+1);
  strcpy(dest,source);
  return dest;
}
