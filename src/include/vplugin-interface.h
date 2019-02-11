
//\IgnoreLatex{

#ifndef VPLUGIN_INTERFACE_H
#define VPLUGIN_INTERFACE_H

#include "types.h"

//}

/*
  This module defines a collection of functions 
*/

#define VPLUGINGETINTERFACENAME "vplugingetinterface"

#define VPLUGINCHECKSIZES(S,I)\
  if((size_t)(S) != sizeof(void *))\
  {\
    ERROR2("Pointer size mismatch (plugin %lu, caller %lu).",\
           (Showuint)sizeof(void *),(Showuint)(S));\
    return -1;\
  }\
  if((size_t)(I) != sizeof(Vplugininterface))\
  {\
    ERROR2("Interface size mismatch (plugin %lu, caller %lu).",\
           (Showuint)sizeof(Vplugininterface),(Showuint)(I));\
    return -1;\
  }

typedef Sint (*Vplugininit)(void *);
typedef Sint (*Vpluginadddemand)(void *);
typedef Sint (*Vpluginparse)(void *);
typedef Sint (*Vpluginsearch)(void *);
typedef Sint (*Vpluginwrap)(void *);

typedef struct
{
  Vplugininit vplugininit; 
  Vpluginadddemand vpluginadddemand; 
  Vpluginparse vpluginparse; 
  Vpluginsearch vpluginsearch;
  Vpluginwrap vpluginwrap;
} Vplugininterface;

typedef struct
{
  void *handle;
  char *sharedobjectfilename;
  Vplugininterface iface;
} Vpluginbundle;

typedef char (*Vplugingetinterface)(Uchar ptrsize, Uchar ifacesize,
                                    Vplugininterface *iface);

//\IgnoreLatex{

#endif

//}
