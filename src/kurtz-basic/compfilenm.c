#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "spacedef.h"
#include "failures.h"

/*@notnull@*/ char *composefilename(char *file,
                                    Uint linenum,
                                    const char *filename,
                                    const char *suffix)
{
  Uint i, lenfilename, lensuffix, totalsize;
  char *dest;

  NOTSUPPOSEDTOBENULL(filename);
  lenfilename = (Uint) strlen(filename);
  NOTSUPPOSEDTOBENULL(suffix);
  lensuffix = (Uint) strlen(suffix);
  totalsize = lenfilename+lensuffix+1+1;
  ALLOCASSIGNSPACEGENERIC(file,linenum,dest,NULL,char,totalsize);
  for(i=0; i<lenfilename; i++)
  {
    dest[i] = filename[i];
  }
  dest[lenfilename] = '.';
  for(i=0; i<lensuffix; i++)
  {
    dest[lenfilename+1+i] = suffix[i];
  }
  dest[lenfilename+lensuffix+1] = '\0';
  return dest;
}
