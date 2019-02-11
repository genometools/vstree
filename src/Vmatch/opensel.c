
#include <stdio.h>
#include <stdlib.h>
#include "debugdef.h"
#include "dlopen.h"
#include "select.h"

#ifdef _WIN32
#include "windows.h"
#endif

Sint openSelectmatch(const char *filename,SelectBundle *selectbundle)
{
#ifndef _WIN32
  selectbundle->selecthandle = dlopen(filename,RTLD_LAZY);
  CHECKDLLERROR(selectbundle->selecthandle,NULL);

/*
  find address of function and data objects
*/

  selectbundle->selectmatch
    = (Selectmatch) dlsym(selectbundle->selecthandle,"selectmatch");
  CHECKDLLERROR(selectbundle->selectmatch,NULL);
  selectbundle->selectmatchInit
    = (SelectmatchInit) dlsym(selectbundle->selecthandle,"selectmatchInit");
  selectbundle->selectmatchWrap
    = (SelectmatchWrap) dlsym(selectbundle->selecthandle,"selectmatchWrap");
  selectbundle->selectmatchHeader
    = (SelectmatchHeader) dlsym(selectbundle->selecthandle,"selectmatchHeader");
  selectbundle->selectmatchFinaltable
    = (SelectmatchFinaltable) dlsym(selectbundle->selecthandle,
                                    "selectmatchFinaltable");
#else
  selectbundle->selecthandle = LoadLibrary(filename);
  CHECKDLLERROR(selectbundle->selecthandle,NULL);

/*
  find address of function and data objects
*/

  selectbundle->selectmatch
    = (Selectmatch) GetProcAddress(selectbundle->selecthandle,"selectmatch");
  CHECKDLLERROR(selectbundle->selectmatch,NULL);
  selectbundle->selectmatchInit
    = (SelectmatchInit) GetProcAddress(selectbundle->selecthandle,
                                       "selectmatchInit");
  selectbundle->selectmatchWrap
    = (SelectmatchWrap) GetProcAddress(selectbundle->selecthandle,
                                       "selectmatchWrap");
  selectbundle->selectmatchHeader
    = (SelectmatchHeader) GetProcAddress(selectbundle->selecthandle,
                                         "selectmatchHeader");
  selectbundle->selectmatchFinaltable
    = (SelectmatchFinaltable) GetProcAddress(selectbundle->selecthandle,
                                             "selectmatchFinaltable");
#endif
  /*
     Error are not checked, since it is not necessary to define
     these functions.
  */
  return 0;
}

Sint closeSelectmatch(SelectBundle *selectbundle)
{
#ifndef _WIN32
  if(selectbundle != NULL && selectbundle->selecthandle != NULL)
  {
    if(dlclose(selectbundle->selecthandle) != 0)
    {
      ERROR0("error when closing shared library");
      return (Sint) -1;
    }
  }
#endif
  return 0;
}
