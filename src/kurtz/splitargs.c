//\Ignore{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errordef.h"
#include "debugdef.h"

//}

/*EE
  The following function splits an argument string. More precisely,
  the function constructs an array of pointers to the first character after
  each sequence of white spaces. The number of arguments is stored in 
  \texttt{args}. The maximal number of arguments is \texttt{maxargc}. 
  \texttt{len} is the length of \texttt{argstring}. If the the number 
  arguments exceeds \texttt{maxargc}, the function return a negative
  error code. Otherwise the return value is 0.
*/

Sint splitargstring(char *argstring,Uint len,
                    Uint maxargc,Argctype *argc,char ** argv)
{
  char *argptr;
  Uint argcount = 0;
  BOOL startarg = True; // if and only at the beginning of argument
  
  for(argptr = argstring; argptr < argstring + len; argptr++)
  {
    if(*argptr == ' ')
    {
      *argptr = '\0';
      startarg = True;
    } else
    {
      if(startarg)
      {
        if(argcount > maxargc)
        {
          ERROR0("argument string too long");
          return (Sint) -1;
        }
        argv[argcount++] = argptr;
        startarg = False;
      } 
    }
  }
  *argc = (Argctype) (argcount+1);
  return 0;
}
