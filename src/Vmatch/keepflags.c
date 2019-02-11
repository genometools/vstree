
#include <string.h>
#include "markinfo.h"
#include "errordef.h"
#include "debugdef.h"

Sint parsekeepflags(Markfields *markfields,const char *arg,const char *opt)
{
  if(strcmp(arg,"keepleft") == 0)
  {
    markfields->markleft = False;
  } else
  {
    if(strcmp(arg,"keepright") == 0)
    {
      markfields->markright = False;
    } else
    {
      if(strcmp(arg,"keepleftifsamesequence") == 0)
      {
        markfields->markleftifdifferentsequence = False;
      } else
      {
        if(strcmp(arg,"keeprightifsamesequence") == 0)
        {
          markfields->markrightifdifferentsequence = False;
        } else
        {
          ERROR2("incorrect optional argument \"%s\" to option %s; "
                 "must be one of the following keywords: "
                 "keepleft, keepright, " 
                 "keepleftifsamesequence, keeprightifsamesequence",
                 arg,opt);
          return (Sint) -1;
        }
      }
    }
  }
  return 0;
}

Sint fillkeepflagshelp(char *space,char *msg,char *verb)
{
  Sprintfreturntype start;
  start = sprintf(space,
                  "%s "
                  "containing a match\n"
                  "optional argument:\n"
                  "* keepleft means to %s the left instance\n"
                  "  of a match\n"
                  "* keepright means to %s the right instance\n"
                  "  of a match\n"
                  "* keepleftifsamesequence means to %s the left instance\n"
                  "  of the match if the right instance occurs\n"
                  "  in the same sequence\n"
                  "* keeprightifsamesequence means to %s the right "
                  "instance\n"
                  "  of the match if the left instance occurs\n"
                  "  in the same sequence",msg,verb,verb,verb,verb);
  if(start>=KEEPFLAGSHELPSIZE)
  {
    ERROR1("fillkeepflagshelp: space buffer is too small, "
           "need at least %ld bytes",
           (Showsint) start);
    return (Sint) -1;
  }
  return 0;
}
