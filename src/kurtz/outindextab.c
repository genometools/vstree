
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include "debugdef.h"
#include "errordef.h"
#include "fhandledef.h"

#include "filehandle.pr"

Sint outindextab(const char *indexname,
                 const char *filenamesuffix,
                 void *tab,
                 Uint bytes,
                 Uint len)
{
  char outputfilename[PATH_MAX+4+1];
  FILE *fp;

  sprintf(outputfilename,"%s.%s",indexname,filenamesuffix);
  fp = CREATEFILEHANDLE(outputfilename,WRITEMODE);
  if(fp == NULL)
  {
    return (Sint) -1;
  }
  if(WRITETOFILEHANDLE(tab,bytes,len,fp) != 0)
  {
    return (Sint) -2;
  }
  if(DELETEFILEHANDLE(fp) != 0)
  {
    return (Sint) -3;
  }
  return 0;
}

void outputUintvalue(FILE *fpout,Uint value)
{
  Uint i;

  for(i=0; i < (Uint) sizeof(Uint); i++)
  {
    (void) putc((Fputcfirstargtype) (value & UCHAR_MAX),fpout);
    value >>= 8;
  }
}
