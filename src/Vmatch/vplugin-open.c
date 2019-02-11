
#include <stdio.h>
#include <stdlib.h>
#include "debugdef.h"
#include "dlopen.h"
#include "spacedef.h"
#include "vplugin-interface.h"

#include "dstrdup.pr"

#ifdef _WIN32
#include "windows.h"
#endif

Sint vpluginopen(const char *filename,Vpluginbundle *vplugininfo)
{
  Vplugingetinterface getinterface;
#ifndef _WIN32
  vplugininfo->handle = dlopen(filename,RTLD_LAZY);
  CHECKDLLERROR(vplugininfo->handle,NULL);

/*
  find address of function and data objects
*/

  ASSIGNDYNAMICSTRDUP(vplugininfo->sharedobjectfilename,filename);
  getinterface = (Vplugingetinterface)dlsym(vplugininfo->handle,
                                            VPLUGINGETINTERFACENAME);
  CHECKDLLERROR(getinterface,NULL);
#else
  vplugininfo->handle = LoadLibrary(filename);
  CHECKDLLERROR(vplugininfo->handle,NULL);

/*
  find address of function and data objects
*/

  ASSIGNDYNAMICSTRDUP(vplugininfo->sharedobjectfilename,filename);
  getinterface = (Vplugingetinterface)GetProcAddress(vplugininfo->handle,
                                                     VPLUGINGETINTERFACENAME);
  CHECKDLLERROR(getinterface,NULL);
#endif

  if(getinterface((Uchar) sizeof(void *),
                  (Uchar) sizeof(Vplugininterface),
                  &vplugininfo->iface) != 0)
  {
    ERROR2("entry point \"%s\" not found in \"%s\"",
           VPLUGINGETINTERFACENAME,filename);
    return (Sint) -1;
  }
  DEBUG1(1,"# vpluginopen(%s) successful\n",filename);
  return 0;
}

Sint vpluginclose(Vpluginbundle *vplugininfo)
{
#ifndef _WIN32
  if(vplugininfo->handle != NULL)
  {
    if(dlclose(vplugininfo->handle) != 0)
    {
      ERROR1("error when closing shared library \"%s\"",
             vplugininfo->sharedobjectfilename);
      return (Sint) -1;
    }
    vplugininfo->handle = NULL;
    DEBUG1(1,"# vpluginclose(%s) successful\n",
                vplugininfo->sharedobjectfilename);
    FREESPACE(vplugininfo->sharedobjectfilename);
  }
#endif
  return 0;
}
