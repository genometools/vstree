#ifndef DLOPEN_H
#define DLOPEN_H

/* 
   naturally HP-UX 9.05 doesn't have the
   "dl" library all other systems support 
*/

#ifdef HPUX

#include <dl.h>

#define RTLD_NOW        BIND_IMMEDIATE
#define RTLD_LAZY       BIND_DEFERRED
#define dlopen(n, f)    (void *) shl_load((n), (f), 0)
#define dlsym(h, n)     ((shl_findsym((shl_t)(h), (n), TYPE_UNDEFINED,\
                                      (void *) &hpsym) == 0) ? hpsym : NULL)
#define dlclose(h)      shl_unload((shl_t)(h));

#define CHECKDLLERROR(RET,ERRORVAL)\
        if((RET) == (ERRORVAL))\
        {\
          ERROR0("error when handling shared libraries");\
          return (Sint) -1;\
        }

static void *hpsym;             /* makes things non-reentrant, but
                                   HP-UX 9.05 doesn't support threads */

#else

#ifndef _WIN32
#include <dlfcn.h> /* standard dynamic linker functions      */
#endif

#ifndef _WIN32
#define CHECKDLLERROR(RET,ERRORVAL)\
        if((RET) == (ERRORVAL))\
        {\
          ERROR1("%s",dlerror());\
          return (Sint) -1;\
        }
#else
#define CHECKDLLERROR(RET,ERRORVAL)\
        if((RET) == (ERRORVAL))\
        {\
          ERROR1("error when handling shared libraries: 0x%08x",\
          (unsigned int) GetLastError());\
          return (Sint) -1;\
        }
#endif

#endif /* HPUX */

#ifndef RTLD_NOW
#define RTLD_NOW        1       /* systems which don't define this expect
                                   the dlopen() mode argument to be 1 */
#endif /* RTLD_NOW */

#ifndef RTLD_LAZY
#define RTLD_LAZY       1       /* systems which don't define this expect
                                   the dlopen() mode argument to be 1 */
#endif /* RTLD_LAZY */

#include "errordef.h"

#endif
