#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <limits.h>
#include <stdio.h>
#include "debugdef.h"
#include "spacedef.h"
#include "failures.h"
#include "fhandledef.h"
#include "file.h"

#include "filehandle.pr"
#include "dstrdup.pr"

typedef struct
{
  char *filename;
  FILE *fpin;
} Fopeninfo;

static Sint callfileopen(void *info,char *path)
{
  Fopeninfo *fileopeninfo = (Fopeninfo *) info;
  char tmpfilename[PATH_MAX+1];

  sprintf(tmpfilename,"%s/%s",path,fileopeninfo->filename);
  DEBUG1(2,"try to open \"%s\"\n",tmpfilename);
  fileopeninfo->fpin = CREATEFILEHANDLE(tmpfilename,READMODE);
  if(fileopeninfo->fpin != NULL)
  {
    return (Sint) 1;
  }
  return 0;
}

static Sint evalpathlist(const char *envarname,void *info,
                         Sint(*applypath)(void *,char *))
{
  char *envptr = getenv(envarname);

  if(envptr != NULL)
  {
    char *start, *ptr, *envstring;
    Sint ret;

    ASSIGNDYNAMICSTRDUP(envstring,envptr);
    start = envstring;
    for(ptr = envstring; /* Nothing */ ; ptr++)
    {
      NOTSUPPOSEDTOBENULL(ptr);
      if(*ptr == '\0')
      {
        DEBUG1(2,"# applypath(%s)\n",start);
        if(applypath(info,start) < 0)
        {
          FREESPACE(envstring);
          return (Sint) -1;
        }
        break;
      }
      if(*ptr == PATH_VAR_SEPARATOR)
      {
        *ptr = '\0';
        ret = applypath(info,start);
        start = ptr + 1;
        if(ret < 0)
        {
          FREESPACE(envstring);
          return (Sint) -2;
        } 
        if(ret == (Sint) 1) // stop
        {
          break;
        }
      }
    }
    FREESPACE(envstring);
  }
  return 0;
}

/*@null@*/ FILE *scanpathsforfile(const char *envstring,const char *filename)
{
  Fopeninfo fileopeninfo;

  fileopeninfo.fpin = CREATEFILEHANDLE(filename,READMODE);
  if(fileopeninfo.fpin != NULL)
  {
    return fileopeninfo.fpin;
  }
  if(strchr(filename,PATH_SEPARATOR) != NULL)
  {
    ERROR3("filename \"%s\" contains illegal symbol '%c': the path list "
           "specified by environment variable \"%s\" cannot be searched "
           "for it",filename,PATH_SEPARATOR,envstring);
    return NULL;
  }
  ASSIGNDYNAMICSTRDUP(fileopeninfo.filename,filename);
  if(evalpathlist(envstring,(void *) &fileopeninfo,callfileopen) != 0)
  {
    FREESPACE(fileopeninfo.filename);
    return NULL;
  }
  if(fileopeninfo.fpin == NULL)
  {
    FREESPACE(fileopeninfo.filename);
    ERROR1("cannot find file \"%s\"",filename);
    return NULL;
  }
  FREESPACE(fileopeninfo.filename);
  return fileopeninfo.fpin;
}
