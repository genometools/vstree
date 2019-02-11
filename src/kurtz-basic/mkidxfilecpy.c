#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "fhandledef.h"
#include "spacedef.h"

#include "space.pr"
#include "filehandle.pr"
#include "compfilenm.pr"

Sint makeindexfilecopy(const char *destindex,
                       const char *sourceindex,
                       const char *suffix,
                       Uint64 maxlength)
{
  char *destfilename, *sourcefilename;
  FILE *fpdest, *fpsource;
  Fgetcreturntype cc;
  Uint64 pos;

  destfilename = COMPOSEFILENAME(destindex,suffix);
  fpdest = CREATEFILEHANDLE(destfilename,WRITEMODE);
  if(fpdest == NULL)
  {
    FREESPACE(destfilename);
    return (Sint) -1;
  }
  sourcefilename = COMPOSEFILENAME(sourceindex,suffix);
  printf("# copy %s %s with maxlength=%u\n",sourcefilename,destfilename,
                                            (unsigned int) maxlength);
  fpsource = CREATEFILEHANDLE(sourcefilename,READMODE);
  if(fpsource == NULL)
  {
    FREESPACE(destfilename);
    FREESPACE(sourcefilename);
    return (Sint) -2;
  }
  if(maxlength == 0)
  {
    while((cc = fgetc(fpsource)) != EOF)
    {
      (void) putc(cc,fpdest);
    }
  } else
  {
    for(pos = 0; pos < maxlength; pos++)
    {
      if((cc = fgetc(fpsource)) == EOF)
      {
        break;
      }
      if(cc == 253)
      {
        (void) putc(254,fpdest);
      } else
      {
        (void) putc(cc,fpdest);
      }
    }
  }
  if(DELETEFILEHANDLE(fpdest) != 0)
  {
    FREESPACE(destfilename);
    FREESPACE(sourcefilename);
    return (Sint) -3;
  }
  if(DELETEFILEHANDLE(fpsource) != 0)
  {
    FREESPACE(destfilename);
    FREESPACE(sourcefilename);
    return (Sint) -4;
  }
  FREESPACE(destfilename);
  FREESPACE(sourcefilename);
  return 0;
}
