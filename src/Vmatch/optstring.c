
#include <ctype.h>
#include <string.h>
#include "types.h"
#include "errordef.h"
#include "debugdef.h"
#include "optdesc.h"
#include "outinfo.h"

/*
  The following function parses a single optional argument of option
  -s.
*/

static Uint parseoptstringargs(const char *arg,const char *opt)
{
  if(isdigit((Ctypeargumenttype) arg[0]))
  {
    Scaninteger readint;

    if(sscanf(arg,"%ld",&readint) != 1 
       || readint <= 0 
       || readint > (Scaninteger) MAXLINEWIDTH)
    {
      ERROR3("argument \"%s\" of option %s must be number "
             "in the range [1...%lu]",
             arg,opt,(Showuint) MAXLINEWIDTH);
      return 0;
    }
    return (Uint) readint;
  } 
  if(strcmp(arg,"leftseq") == 0)
  {
    return SHOWPURELEFTSEQ;
  }
  if(strcmp(arg,"rightseq") == 0)
  {
    return SHOWPURERIGHTSEQ;
  }
  if(strcmp(arg,"abbrev") == 0)
  {
    return SHOWALIGNABBREV;
  }
  if(strcmp(arg,"abbreviub") == 0)
  {
    return SHOWALIGNABBREVIUB;
  }
  if(strcmp(arg,"xml") == 0)
  {
    return SHOWVMATCHXML;
  }
  ERROR2("incorrect argument \"%s\" to option %s "
         "must be one of the following keywords: "
         "leftseq, rightseq, abbrev, abbreviub",arg,opt);
  return 0;
}

/*
  The following function parses all optional arguments of option
  -s.
*/

Uint parsesequenceoutparms(Argctype argc,
                           const char * const*argv,
                           Uint *argnum,
                           const char *optname)
{
  Uint showstring;

  if(MOREARGOPTSWITHOUTLAST(*argnum))
  {
    Uint ret;

    showstring = DEFAULTLINEWIDTH;
    (*argnum)++;
    ret = parseoptstringargs(argv[*argnum],optname);
    if(ret == 0)
    {
      return 0;
    }
    if(ret & MAXLINEWIDTH)
    {
      showstring = ret;
    } else
    {
      showstring |= ret;
    }
    if(MOREARGOPTSWITHOUTLAST(*argnum))
    {
      (*argnum)++;
      ret = parseoptstringargs(argv[*argnum],optname);
      if(ret == 0)
      {
        return 0;
      }
      if(ret & MAXLINEWIDTH)
      {
        showstring = (showstring & (SHOWPURELEFTSEQ | SHOWPURERIGHTSEQ)) 
                     | ret;
      } else
      {
        showstring |= ret;
      }
    }
  } else
  {
    showstring = DEFAULTLINEWIDTH;
  }
  return showstring;
}
